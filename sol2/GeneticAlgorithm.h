#pragma once
#include <vector>
#include <random>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <unordered_set>
#include <unordered_map>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "Data.h"
#include "ScheduleEvaluator.h"
#include "MutationGenerator.h"

struct GAConfig
{
    int population_size = 200;
    int max_generations = 2000;
    int tournament_size = 3;
    double elitism_fraction = 0.1;          // top fraction kept unchanged
    double crossover_rate = 0.8;             // probability of crossover (vs. cloning a parent)
    double mutation_rate = 0.5;              // probability that a child gets mutated

    // Mutation-related
    double per_intersection_mut_rate = 0.05;
    double add_missing_intersection_rate = 0.05;
    int max_green_duration = 8;

    int time_budget_ms = 0;                  // 0 = use max_generations; >0 = run until time elapsed
    int log_every = 2000;                       // log every N generations
    int stagnation_limit = 50;               // gens without improvement before bumping mutation
    unsigned seed = 0;                       // 0 = random
};

class GeneticAlgorithm
{
private:
    const Data& data;
    GAConfig config;
    MutationGenerator mutator;

    // Per-thread RNG (avoids contention)
    std::vector<std::mt19937> rngs;

    struct Individual
    {
        Schedule schedule;
        unsigned long long score = 0;
        bool evaluated = false;
    };

    std::vector<Individual> population;
    Individual best_ever;

    double current_mutation_rate;
    int gens_since_improvement = 0;

    std::mt19937& thread_rng()
    {
#ifdef _OPENMP
        return rngs[omp_get_thread_num()];
#else
        return rngs[0];
#endif
    }

    Schedule make_random_schedule(std::mt19937& rng)
    {
        Schedule s;
        const auto& intersections = mutator.get_intersections_with_streets();
        s.reserve(intersections.size());
        std::uniform_int_distribution<int> dur_dist(1, config.max_green_duration);

        for (int int_id : intersections)
        {
            const auto& streets = mutator.get_usable_streets(int_id);
            std::vector<std::pair<int,int>> sched;
            sched.reserve(streets.size());

            std::vector<int> shuffled = streets;
            std::shuffle(shuffled.begin(), shuffled.end(), rng);
            for (int s_id : shuffled)
                sched.emplace_back(s_id, dur_dist(rng));

            s[int_id] = std::move(sched);
        }
        return s;
    }

    Schedule make_trivial_schedule()
    {
        Schedule s;
        const auto& intersections = mutator.get_intersections_with_streets();
        s.reserve(intersections.size());
        for (int int_id : intersections)
        {
            const auto& streets = mutator.get_usable_streets(int_id);
            std::vector<std::pair<int,int>> sched;
            sched.reserve(streets.size());
            for (int s_id : streets)
                sched.emplace_back(s_id, 1);
            s[int_id] = std::move(sched);
        }
        return s;
    }

    Schedule mutate_copy_of_trivial(std::mt19937& rng)
    {
        Schedule s = make_trivial_schedule();
        std::uniform_real_distribution<double> uni(0.0, 1.0);
        std::uniform_int_distribution<int> dur_dist(1, config.max_green_duration);
        for (auto& [id, sched] : s)
        {
            if (uni(rng) < 0.3)
                std::shuffle(sched.begin(), sched.end(), rng);
            if (uni(rng) < 0.3 && !sched.empty())
            {
                std::uniform_int_distribution<int> pos(0, (int)sched.size() - 1);
                sched[pos(rng)].second = dur_dist(rng);
            }
        }
        return s;
    }

    // Per-intersection uniform crossover
    Schedule crossover(const Schedule& a, const Schedule& b, std::mt19937& rng)
    {
        Schedule child;
        child.reserve(std::max(a.size(), b.size()));
        std::uniform_int_distribution<int> coin(0, 1);

        std::unordered_set<int> all_ids;
        all_ids.reserve(a.size() + b.size());
        for (const auto& [id, _] : a) all_ids.insert(id);
        for (const auto& [id, _] : b) all_ids.insert(id);

        for (int id : all_ids)
        {
            const auto a_it = a.find(id);
            const auto b_it = b.find(id);

            if (a_it != a.end() && b_it != b.end())
                child[id] = coin(rng) ? a_it->second : b_it->second;
            else if (a_it != a.end())
            {
                if (coin(rng)) child[id] = a_it->second;
            }
            else
            {
                if (coin(rng)) child[id] = b_it->second;
            }
        }
        return child;
    }

    int tournament_select(std::mt19937& rng)
    {
        std::uniform_int_distribution<int> pick(0, (int)population.size() - 1);
        int best_idx = pick(rng);
        for (int i = 1; i < config.tournament_size; ++i)
        {
            int idx = pick(rng);
            if (population[idx].score > population[best_idx].score)
                best_idx = idx;
        }
        return best_idx;
    }

    // Build a single offspring (used inside parallel region)
    Individual make_offspring(std::mt19937& rng)
    {
        std::uniform_real_distribution<double> uni(0.0, 1.0);

        const int parent_a_idx = tournament_select(rng);
        const int parent_b_idx = tournament_select(rng);

        Individual child;
        if (uni(rng) < config.crossover_rate)
        {
            child.schedule = crossover(
                    population[parent_a_idx].schedule,
                    population[parent_b_idx].schedule,
                    rng);
        }
        else
        {
            child.schedule = (uni(rng) < 0.5)
                             ? population[parent_a_idx].schedule
                             : population[parent_b_idx].schedule;
        }

        if (uni(rng) < current_mutation_rate)
            mutator.mutate(child.schedule, rng);

        child.evaluated = false;
        return child;
    }

    void evaluate_population_parallel()
    {
        const int n = (int)population.size();
#pragma omp parallel for schedule(dynamic, 1)
        for (int i = 0; i < n; ++i)
        {
            if (population[i].evaluated)
                continue;
            ScheduleEvaluator ev(data, population[i].schedule);
            population[i].score = ev.simulate();
            population[i].evaluated = true;
        }
    }

    // Build offspring in parallel and evaluate them
    void produce_and_evaluate_offspring(std::vector<Individual>& next_gen, int target_size)
    {
        const int elite_count = (int)next_gen.size();
        const int offspring_count = target_size - elite_count;
        if (offspring_count <= 0) return;

        // Pre-size so each thread can write directly to its slot
        next_gen.resize(target_size);

        // Parallel offspring generation: crossover + mutation
#pragma omp parallel for schedule(dynamic, 4)
        for (int i = 0; i < offspring_count; ++i)
        {
            auto& rng = thread_rng();
            next_gen[elite_count + i] = make_offspring(rng);
        }

        // Parallel evaluation of the new offspring (elites are already evaluated)
#pragma omp parallel for schedule(dynamic, 1)
        for (int i = elite_count; i < target_size; ++i)
        {
            if (next_gen[i].evaluated)
                continue;
            ScheduleEvaluator ev(data, next_gen[i].schedule);
            next_gen[i].score = ev.simulate();
            next_gen[i].evaluated = true;
        }
    }

    double avg_score() const
    {
        unsigned long long sum = 0;
        for (const auto& ind : population) sum += ind.score;
        return (double)sum / population.size();
    }

    static MutationConfig make_mutation_config(const GAConfig& ga_cfg)
    {
        MutationConfig mc;
        mc.per_intersection_mut_rate = ga_cfg.per_intersection_mut_rate;
        mc.add_missing_intersection_rate = ga_cfg.add_missing_intersection_rate;
        mc.max_green_duration = ga_cfg.max_green_duration;
        return mc;
    }

public:
    explicit GeneticAlgorithm(const Data& data, const GAConfig& config = {})
            : data(data)
            , config(config)
            , mutator(data, make_mutation_config(config))
            , current_mutation_rate(config.mutation_rate)
    {
        int num_threads = 1;
#ifdef _OPENMP
        num_threads = omp_get_max_threads();
#endif
        rngs.reserve(num_threads);
        std::mt19937 seed_gen(config.seed ? config.seed : std::random_device{}());
        for (int i = 0; i < num_threads; ++i)
            rngs.emplace_back(seed_gen());
    }

    Schedule run(bool run_for_30_minutes = false)
    {
        // If requested, override the time budget to 30 minutes and let
        // generations effectively be unlimited so the loop is bounded by time.
        if (run_for_30_minutes)
        {
            config.time_budget_ms = 30 * 60 * 1000; // 30 minutes in ms
            config.max_generations = std::numeric_limits<int>::max();
        }

        const auto t_start = std::chrono::steady_clock::now();

        population.clear();
        population.reserve(config.population_size);

        // Seed: ~5% trivial copies, ~20% mutated trivial copies, rest random
        const int trivial_seeds = std::max(1, config.population_size / 20);
        const int mutated_trivial_seeds = std::max(1, config.population_size / 5);

        for (int i = 0; i < trivial_seeds; ++i)
            population.push_back({make_trivial_schedule(), 0, false});
        for (int i = 0; i < mutated_trivial_seeds; ++i)
            population.push_back({mutate_copy_of_trivial(rngs[0]), 0, false});
        while ((int)population.size() < config.population_size)
            population.push_back({make_random_schedule(rngs[0]), 0, false});

        evaluate_population_parallel();

        best_ever = *std::max_element(population.begin(), population.end(),
                                      [](const Individual& a, const Individual& b) { return a.score < b.score; });

        std::cout << "Gen 0: best=" << best_ever.score
                  << " avg=" << (long long)avg_score() << '\n';

        const int elite_count = std::max(1, (int)(config.elitism_fraction * config.population_size));

        for (int gen = 1; gen <= config.max_generations; ++gen)
        {
            if (config.time_budget_ms > 0)
            {
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - t_start).count();
                if (elapsed >= config.time_budget_ms)
                {
                    std::cout << "Time budget reached at gen " << gen << '\n';
                    break;
                }
            }

            std::sort(population.begin(), population.end(),
                      [](const Individual& a, const Individual& b) { return a.score > b.score; });

            std::vector<Individual> next_gen;
            next_gen.reserve(config.population_size);

            // Elites first (kept unchanged, already evaluated)
            for (int i = 0; i < elite_count; ++i)
                next_gen.push_back(population[i]);

            // Generate and evaluate offspring in parallel
            produce_and_evaluate_offspring(next_gen, config.population_size);

            population = std::move(next_gen);

            const auto& gen_best = *std::max_element(population.begin(), population.end(),
                                                     [](const Individual& a, const Individual& b) { return a.score < b.score; });

            if (gen_best.score > best_ever.score)
            {
                best_ever = gen_best;
                gens_since_improvement = 0;
                current_mutation_rate = config.mutation_rate;
            }
            else
            {
                gens_since_improvement++;
                if (gens_since_improvement > 0 && gens_since_improvement % config.stagnation_limit == 0)
                {
                    if (gens_since_improvement > 1000)
                    {
                        std::cout << "Long stagnation period, shutting down simulation\n";
                        break;
                    }

                    current_mutation_rate = std::min(1.0, current_mutation_rate * 1.5);
                    // Stagnation detected, increase mutation rate
                }
            }

            if (gen % config.log_every == 0)
            {
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - t_start).count();
                std::cout << "Gen " << gen
                          << ": best=" << best_ever.score
                          << " gen_best=" << gen_best.score
                          << " avg=" << (long long)avg_score()
                          << " mut_rate=" << current_mutation_rate
                          << " (" << elapsed << "ms)\n";
            }
        }

        auto total_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - t_start).count();
        std::cout << "GA complete. Best score: " << best_ever.score
                  << " in " << total_elapsed << "ms\n";

        return best_ever.schedule;
    }

    unsigned long long best_score() const { return best_ever.score; }
};