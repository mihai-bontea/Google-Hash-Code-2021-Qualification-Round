#include <array>
#include <chrono>
#include <iostream>

#include "Data.h"
#include "MergeFindSet.h"
#include "ScheduleEvaluator.h"

class BasicSolver
{
private:
    const Data &data;
public:
    explicit BasicSolver(const Data &data)
    : data(data)
    {}

    Schedule solve()
    {
        Schedule schedule;
        for (int street_id = 0; street_id < data.nr_streets; ++ street_id)
        {
            if (!data.street_usage[street_id])
                continue;

            const int intersection = std::get<1>(data.street_intersect.at(street_id));
            schedule[intersection].emplace_back(street_id, 1);
        }

        return schedule;
    }
};

void test_get_next_green_light1()
{
    const std::string in_prefix = "../../test_input_files/";
    const std::string input_file = "test1.in";

    Data data(in_prefix + input_file);
    // Create a basic schedule(each required street receives 1 second of green, in lexicographic order)
    Schedule schedule;
    schedule[1].emplace_back(0, 1);
    schedule[1].emplace_back(1, 1);
    schedule[1].emplace_back(3, 1);
    schedule[4].emplace_back(2, 1);

    ScheduleEvaluator sched_ev(data, schedule);

    // Street C is the only street at intersection 4 that should ever have green light
    // so get_next_green_light(C, local_time) should always equal local_time
    for (int i = 0; i < data.simulation_seconds; ++i)
        assert(sched_ev.get_next_green_light_time(2, i) == i);

    // get_next_green_light_time at time 0
    int next_green_light_time = sched_ev.get_next_green_light_time(0, 0);
    assert(next_green_light_time == 0);

    next_green_light_time = sched_ev.get_next_green_light_time(1, 0);
    assert(next_green_light_time == 1);

    next_green_light_time = sched_ev.get_next_green_light_time(3, 0);
    assert(next_green_light_time == 2);

    // get_next_Green_light_time at next cycle start
    next_green_light_time = sched_ev.get_next_green_light_time(0, 3);
    assert(next_green_light_time == 3);

    next_green_light_time = sched_ev.get_next_green_light_time(1, 3);
    assert(next_green_light_time == 4);

    next_green_light_time = sched_ev.get_next_green_light_time(3, 3);
    assert(next_green_light_time == 5);

    // get_next_Green_light_time at next cycle end
    next_green_light_time = sched_ev.get_next_green_light_time(0, 5);
    assert(next_green_light_time == 6);

    next_green_light_time = sched_ev.get_next_green_light_time(1, 5);
    assert(next_green_light_time == 7);

    next_green_light_time = sched_ev.get_next_green_light_time(3, 5);
    assert(next_green_light_time == 5);
}

void test_score1()
{
    const std::string in_prefix = "../../test_input_files/";
    const std::string input_file = "test1.in";

    Data data(in_prefix + input_file);
    // Create a basic schedule(each required street receives 1 second of green, in lexicographic order)
    Schedule schedule;
    schedule[1].emplace_back(0, 1);
    schedule[1].emplace_back(1, 1);
    schedule[1].emplace_back(3, 1);
    schedule[4].emplace_back(2, 1);

    ScheduleEvaluator sched_ev(data, schedule);
    const auto score = sched_ev.simulate();
    // All 3 cars reach the end, with 5, 6, 7 seconds to spare(3 * 100 + 18)
    assert(score == 318);
}

void test_score2()
{
    const std::string in_prefix = "../../test_input_files/";
    const std::string input_file = "test2.in";

    Data data(in_prefix + input_file);
    // Create a basic schedule(each required street receives 1 second of green, in lexicographic order)
    Schedule schedule;
    schedule[1].emplace_back(0, 1);
    schedule[2].emplace_back(1, 1);
    schedule[3].emplace_back(2, 1);

    ScheduleEvaluator sched_ev(data, schedule);
    const auto score = sched_ev.simulate();
    // 2 cars out of 4 reach the end on time, one with 1 second to spare(2 * 100 + 1)
    assert(score == 201);
}

int main()
{
    const std::string in_prefix = "../../input_files/";
    const std::string out_prefix = "../../output_files/sol1/";
    const std::array<std::string, 6> input_files = {"a_example.in", "b_ocean.in", "c_checkmate.in",
                                                    "d_daily_commute.in", "e_etoile.in", "f_forever_jammed.in"};

    for (const auto& input_file : input_files)
    {
        std::cout << "Now working on " << input_file << std::endl;
        Data data(in_prefix + input_file);
        BasicSolver solver(data);

        const auto schedule = solver.solve();

        auto t_start = std::chrono::high_resolution_clock::now();
        ScheduleEvaluator sched_ev(data, schedule);
        auto t_construct = std::chrono::high_resolution_clock::now();
        auto score = sched_ev.simulate();
        auto t_simulate = std::chrono::high_resolution_clock::now();

        auto construct_us = std::chrono::duration_cast<std::chrono::microseconds>(t_construct - t_start).count();
        auto simulate_us = std::chrono::duration_cast<std::chrono::microseconds>(t_simulate - t_construct).count();
        auto total_us = construct_us + simulate_us;

        std::cout << "Score = " << score << '\n';
        std::cout << "  Constructor: " << construct_us << " us\n";
        std::cout << "  Simulate:    " << simulate_us << " us\n";
        std::cout << "  Total eval:  " << total_us << " us ("
                  << total_us / 1000.0 << " ms)\n\n";

        const auto out_filename = out_prefix + input_file.substr(0, (input_file.find('.'))) + ".out";
        data.write_to_file(out_filename, schedule);
    }
    return 0;
}
