#pragma once
#include <vector>
#include <random>
#include <algorithm>
#include <unordered_set>

#include "Data.h"

struct MutationConfig
{
    // Probability that any given intersection in the schedule gets mutated
    double per_intersection_mut_rate = 0.05;

    // Probability of trying to add a missing intersection (per call to mutate)
    double add_missing_intersection_rate = 0.05;

    int max_green_duration = 8;

    // Type weights (must sum to 100)
    int weight_swap = 50;        // swap two streets within an intersection
    int weight_adjust_dur = 30;  // adjust green duration by +-delta
    int weight_shuffle = 10;     // shuffle entire street order
    int weight_add_remove = 10;  // add or remove a street within an intersection
};

class MutationGenerator
{
private:
    const Data& data;
    MutationConfig config;

    // Precomputed: intersections that have at least one usable street
    std::vector<std::vector<int>> usable_streets_per_intersection;
    std::vector<int> intersections_with_streets;

    void compute_usable_streets()
    {
        usable_streets_per_intersection.resize(data.nr_intersections);
        for (int street_id = 0; street_id < data.nr_streets; ++street_id)
        {
            if (!data.street_usage[street_id])
                continue;
            const int intersection = data.street_intersect[street_id].second;
            usable_streets_per_intersection[intersection].push_back(street_id);
        }
        for (int i = 0; i < data.nr_intersections; ++i)
            if (!usable_streets_per_intersection[i].empty())
                intersections_with_streets.push_back(i);
    }

    void mutate_swap(std::vector<std::pair<int,int>>& sched, std::mt19937& rng)
    {
        if (sched.size() < 2) return;
        std::uniform_int_distribution<int> pos(0, (int)sched.size() - 1);
        int i = pos(rng), j = pos(rng);
        std::swap(sched[i], sched[j]);
    }

    void mutate_adjust_duration(std::vector<std::pair<int,int>>& sched, std::mt19937& rng)
    {
        if (sched.empty()) return;
        std::uniform_int_distribution<int> dur_delta_dist(-2, 2);
        std::uniform_int_distribution<int> pos(0, (int)sched.size() - 1);

        const int street_pos = pos(rng);
        int new_dur = sched[street_pos].second + dur_delta_dist(rng);

        if (new_dur < 1)
            new_dur = 1;
        if (new_dur > data.simulation_seconds)
            new_dur = data.simulation_seconds;

        sched[street_pos].second = new_dur;
    }

    void mutate_shuffle(std::vector<std::pair<int,int>>& sched, std::mt19937& rng)
    {
        std::shuffle(sched.begin(), sched.end(), rng);
    }

    void mutate_add_remove(int intersection_id, std::vector<std::pair<int,int>>& sched, std::mt19937& rng)
    {
        const auto& usable = usable_streets_per_intersection[intersection_id];
        if (usable.empty())
            return;

        std::uniform_int_distribution<int> coin(0, 1);
        std::uniform_int_distribution<int> dur_dist(1, config.max_green_duration);

        if (coin(rng) && sched.size() < usable.size())
        {
            // Add a street that's not currently scheduled
            std::unordered_set<int> present;
            present.reserve(sched.size());
            for (const auto& [s_id, _] : sched) present.insert(s_id);

            std::vector<int> missing;
            missing.reserve(usable.size() - sched.size());
            for (int s_id : usable)
                if (!present.count(s_id)) missing.push_back(s_id);

            if (!missing.empty())
            {
                std::uniform_int_distribution<int> pick(0, (int)missing.size() - 1);
                sched.emplace_back(missing[pick(rng)], dur_dist(rng));
            }
        }
        else if (sched.size() > 1)
        {
            std::uniform_int_distribution<int> pos(0, (int)sched.size() - 1);
            sched.erase(sched.begin() + pos(rng));
        }
    }

    // Adds a randomly chosen missing intersection (one not currently in the schedule)
    void try_add_missing_intersection(Schedule& s, std::mt19937& rng)
    {
        // Find intersections not in the schedule
        std::vector<int> missing;
        missing.reserve(intersections_with_streets.size());
        for (int int_id : intersections_with_streets)
            if (!s.contains(int_id))
                missing.push_back(int_id);

        if (missing.empty()) return;

        std::uniform_int_distribution<int> pick(0, (int)missing.size() - 1);
        const int chosen = missing[pick(rng)];

        // Build a randomized schedule for it
        const auto& streets = usable_streets_per_intersection[chosen];
        std::vector<int> shuffled = streets;
        std::shuffle(shuffled.begin(), shuffled.end(), rng);

        std::vector<std::pair<int,int>> sched;
        sched.reserve(shuffled.size());
        std::uniform_int_distribution<int> dur_dist(1, config.max_green_duration);
        for (int s_id : shuffled)
            sched.emplace_back(s_id, dur_dist(rng));

        s[chosen] = std::move(sched);
    }

public:
    explicit MutationGenerator(const Data& data, const MutationConfig& config = {})
            : data(data)
            , config(config)
    {
        compute_usable_streets();
    }

    void mutate(Schedule& s, std::mt19937& rng)
    {
        std::uniform_real_distribution<double> uni(0.0, 1.0);

        const int total_weight = config.weight_swap + config.weight_adjust_dur
                                 + config.weight_shuffle + config.weight_add_remove;
        std::uniform_int_distribution<int> type_dist(0, total_weight - 1);

        // Mutate intersections that are already in the schedule
        for (auto& [id, sched] : s)
        {
            if (uni(rng) >= config.per_intersection_mut_rate)
                continue;

            const int type = type_dist(rng);

            int cumulative = config.weight_swap;
            // Mutation type 1: Swap two streets in the same intersection
            if (type < cumulative)
            {
                mutate_swap(sched, rng);
                continue;
            }
            cumulative += config.weight_adjust_dur;
            // Mutation type 2: Adjust green duration for one street
            if (type < cumulative)
            {
                mutate_adjust_duration(sched, rng);
                continue;
            }
            cumulative += config.weight_shuffle;
            // Mutation type 3: Completely shuffle the street order
            if (type < cumulative)
            {
                mutate_shuffle(sched, rng);
                continue;
            }
            // Otherwise: add or remove a street
            mutate_add_remove(id, sched, rng);
        }

        // Independent chance to add a missing intersection back into the schedule.
        if (uni(rng) < config.add_missing_intersection_rate)
            try_add_missing_intersection(s, rng);
    }

    inline const std::vector<int>& get_intersections_with_streets() const noexcept
    {
        return intersections_with_streets;
    }

    inline const std::vector<int>& get_usable_streets(int intersection_id) const noexcept
    {
        return usable_streets_per_intersection[intersection_id];
    }
};