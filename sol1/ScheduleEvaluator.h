#pragma once
#include <set>
#include <queue>
#include <vector>
#include <numeric>

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

    void insert_starting_streets(std::vector<std::queue<std::pair<int, int>>>& street_to_queue, MinHeap& events)
    {
        for (int car_id = 0; car_id < data.nr_cars; ++car_id)
        {
            const int starting_street = data.car_paths[car_id].front();
            // Cars start at the end of the first street
            street_to_queue[starting_street].emplace(car_id, 0);
            events.emplace(0, starting_street);
        }
    }

    std::pair<int, std::set<int>> get_street_ids_with_next_events(MinHeap& events)
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

    int get_waiting_time(int current_time, int street_id, int pos_in_queue, const Schedule& schedule)
    {
        const int intersection_id = std::get<0>(data.street_info.at(street_id));

        // Always red light for this intersection
        const auto& int_schedule_it = schedule.find(intersection_id);
        if (int_schedule_it == schedule.end())
            return -1;

        const auto& int_schedule = int_schedule_it->second;
        int cycle_length = std::accumulate(int_schedule.begin(), int_schedule.end(), 0,
                                            [](int acc, const std::pair<int,int>& p) { return acc + p.second; });



    }
public:

    explicit ScheduleEvaluator(const Data& data)
    : data(data)
    {}

    int simulate(const Schedule& schedule)
    {
        MinHeap events;
        std::vector<int> car_id_to_street_pos(data.nr_cars);
        std::vector<std::queue<std::pair<int, int>>> street_to_queue(data.nr_streets);

        insert_starting_streets(street_to_queue, events);

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