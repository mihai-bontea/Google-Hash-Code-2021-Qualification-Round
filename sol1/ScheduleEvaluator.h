#pragma once
#include <set>
#include <queue>
#include <vector>
#include <numeric>
#include <unordered_map>

#include "Data.h"

using MinHeap = std::priority_queue<std::pair<int,int>,
        std::vector<std::pair<int,int>>,
        decltype([](const std::pair<int,int>& a, const std::pair<int,int>& b) {
            return a.first > b.first;
        })>;

class ScheduleEvaluator
{
private:
    const Data& data;
    const Schedule& schedule;
    // intersection -> {(green_light_start, green_light_end)} in the same street order as Schedule
    std::unordered_map<int, std::vector<std::pair<int,int>>> streets_green_intervals;

    std::vector<std::queue<std::pair<int, int>>> street_to_queue;
    MinHeap events;

    void precompute_street_green_intervals()
    {
        int time = 0;
        for (const auto& [intersection_id, street_schedule] : schedule)
        {
            auto& intervals = streets_green_intervals[intersection_id];
            for (const auto [street_id, green_light_time] : street_schedule)
            {
                intervals.emplace_back(time, time + green_light_time - 1);
                time += green_light_time;
            }
        }
    }

    // Modify, nu mai face event pt secunda 0 ca n-are sens
    void insert_starting_streets()
    {
        // step 1: group cars by street
        // step 2: using get_next_green_light_time and incrementng time, handle each street fully before moving on
        // push new events to heap and to street_to_queue

        for (int car_id = 0; car_id < data.nr_cars; ++car_id)
        {
            const int starting_street = data.car_paths[car_id].front();
            // Cars start at the end of the first street
            street_to_queue[starting_street].emplace(car_id, 0);
            events.emplace(0, starting_street);
        }
    }

    int get_next_green_light_time(int street_id, int current_time)
    {
        const int intersection_id = std::get<0>(data.street_info.at(street_id));

        const auto& street_schedule = schedule.find(intersection_id)->second;
        auto it = std::find_if(street_schedule.begin(), street_schedule.end(),
                               [street_id](const std::pair<int,int>& p) { return p.first == street_id; });
        const int street_pos_in_sched = std::distance(street_schedule.begin(), it);

        auto& green_intervals = streets_green_intervals[intersection_id];
        const int streets_at_int = green_intervals.size();
        const int cycle_length = green_intervals[streets_at_int - 1].second + 1;
        const int sec_in_cycle = current_time % cycle_length;

        const auto [desired_start, desired_end] = green_intervals[street_pos_in_sched];

        if (desired_start <= sec_in_cycle && sec_in_cycle <= desired_end)
            return current_time;
        else if (desired_end < sec_in_cycle)
            return current_time + cycle_length - sec_in_cycle + desired_start;
        else
            return desired_start - sec_in_cycle;
    }

    std::pair<int, std::set<int>> get_street_ids_with_next_events()
    {
        const int soonest_event_time = events.top().first;
        std::set<int> affected_street_ids;
        while (!events.empty() && events.top().first == soonest_event_time)
        {
            affected_street_ids.insert(events.top().second);
            events.pop();
        }
        return {soonest_event_time, affected_street_ids};
    }
public:

    explicit ScheduleEvaluator(const Data& data, const Schedule& schedule)
    : data(data)
    , schedule(schedule)
    {
        street_to_queue.reserve(data.nr_streets);

        precompute_street_green_intervals();
        insert_starting_streets();
    }

    int simulate()
    {
        std::vector<int> car_id_to_street_pos(data.nr_cars);

        int t = 0;
        while (t <= data.simulation_seconds)
        {
            const auto [soonest_event_time, affected_street_ids] = get_street_ids_with_next_events(events);


            for (const auto& [street_name, _] : data.street_usage)
            {
                auto& queue = street_to_queue[street_name];
                if (queue.empty())
                    continue;

                const auto [car_id, entry_second] = queue.front();

                const int street_len = std::get<2>(data.street_info.at(street_name));
                if (t - entry_second >= street_len)
                {
                    // car can go through intersection IF it got green light
                    bool green_light = true;
                    if (green_light)
                    {
                        // push into the queue for the next street
                        queue.pop();
                    }
                }
            }
            // Pass time to next event
            // a) a car reaches the end of a street
            // b) a street with a car at the end gets green light
            // min priority_queue care tine secundele pana la urmatoru event
            // cand pushezi o masina pe o strada, bagi secunda cand ajunge la final in PQ
            // cand o masina la final de strada are rosu, pushezi momentu cand o sa aiba verde?
        }
    }
};