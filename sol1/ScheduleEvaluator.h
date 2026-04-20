#pragma once
#include <set>
#include <queue>
#include <vector>
#include <numeric>
#include <cassert>
#include <unordered_map>

#include "Data.h"
#include "BucketQueue.h"

using MinHeap = std::priority_queue<std::pair<int,int>,
        std::vector<std::pair<int,int>>,
        decltype([](const std::pair<int,int>& a, const std::pair<int,int>& b) {
            return a.first > b.first;
        })>;

struct StreetGreenInfo {
    int green_start;  // offset within cycle
    int green_end;    // offset within cycle
    int cycle_length; // total cycle length at this intersection
};

struct ScheduleEvaluator
{
    const Data& data;
    const Schedule& schedule;
    // intersection -> {(green_light_start, green_light_end)} in the same street order as Schedule
    std::unordered_map<int, std::vector<std::pair<int,int>>> streets_green_intervals;

    std::vector<int> street_id_to_local_time, street_pos_in_sched;

    std::vector<int> car_id_to_street_pos;

    std::vector<StreetGreenInfo> street_green_info;

    // street_id -> {(time, car_id)}
//    std::vector<MinHeap> street_to_queue;
//    MinHeap events;
    BucketQueue<std::pair<int, int>> events;

    unsigned long long total_score;

    void precompute_street_green_intervals()
    {
        for (const auto& [intersection_id, street_schedule] : schedule)
        {
            int time = 0;
            auto& intervals = streets_green_intervals[intersection_id];
            for (const auto [street_id, green_light_time] : street_schedule)
            {
                intervals.emplace_back(time, time + green_light_time - 1);
                time += green_light_time;
            }
        }
    }

    void precompute_street_pos_in_sched()
    {
        std::fill(street_pos_in_sched.begin(), street_pos_in_sched.end(), -1);

        for (const auto& [intersection_id, street_schedule] : schedule)
        {
            for (int pos = 0; pos < (int)street_schedule.size(); ++pos)
            {
                street_pos_in_sched[street_schedule[pos].first] = pos;
            }
        }
    }

    void precompute_street_green_info()
    {
        street_green_info.assign(data.nr_streets, {-1, -1, -1});

        for (const auto& [intersection_id, street_schedule] : schedule)
        {
            int time = 0;
            for (const auto& [street_id, green_duration] : street_schedule)
            {
                street_green_info[street_id] = {time, time + green_duration - 1, -1};
                time += green_duration;
            }
            // Now set cycle_length for all streets at this intersection
            for (const auto& [street_id, green_duration] : street_schedule)
                street_green_info[street_id].cycle_length = time;
        }
    }

    inline bool is_street_scheduled(int street_id) const
    {
        return street_pos_in_sched[street_id] != -1;
    }

    inline int get_next_street_for_car(int car_id) const
    {
        const int pos_in_path = car_id_to_street_pos[car_id] + 1;
        return data.car_paths[car_id][pos_in_path];
    }

    void insert_starting_streets()
    {
        for (int car_id = 0; car_id < data.nr_cars; ++car_id)
        {
            // Cars start at the end of the first street
            const int starting_street_id = data.car_paths[car_id].front();

            // Since initially multiple cars can be at the end of the 1st street at the exact same time, we need to manually
            // update the local time for each street
            const int local_time = street_id_to_local_time[starting_street_id];
            const int next_green_light_time = get_next_green_light_time(starting_street_id, local_time);
            assert(next_green_light_time >= 0);

            const int next_street_id = get_next_street_for_car(car_id);

            // Time until next event(when car reaches the end of the next street)
            const int next_street_length = std::get<2>(data.street_info.at(next_street_id));
            const int time_until_next_event = next_green_light_time + next_street_length;

            // Update the events queues
//            street_to_queue[next_street_id].emplace(time_until_next_event, car_id);
//            events.emplace(time_until_next_event, next_street_id);
            events.insert({next_street_id, car_id}, time_until_next_event);

            // Update the time for the current street(i.e. how much did the car wait for the green light?)
            street_id_to_local_time[starting_street_id] = next_green_light_time + 1;
            car_id_to_street_pos[car_id]++;
        }
    }

    int get_next_green_light_time(int street_id, int current_time)
    {
        const auto& info = street_green_info[street_id];
        const int sec_in_cycle = current_time % info.cycle_length;

        if (sec_in_cycle >= info.green_start && sec_in_cycle <= info.green_end)
            return current_time;
        if (sec_in_cycle > info.green_end)
            return current_time + info.cycle_length - sec_in_cycle + info.green_start;
        return current_time + info.green_start - sec_in_cycle;
    }

//    std::pair<int, std::set<int>> get_street_ids_with_next_events()
//    {
//        const int soonest_event_time = events.top().first;
//        std::set<int> affected_street_ids;
//        while (!events.empty() && events.top().first == soonest_event_time)
//        {
//            affected_street_ids.insert(events.top().second);
//            events.pop();
//        }
//        return {soonest_event_time, affected_street_ids};
//    }

    inline bool is_last_street(int car_id) const
    {
        return car_id_to_street_pos[car_id] == (data.car_paths[car_id].size() - 1);
    }

    inline void update_score(int current_time)
    {
        if (current_time <= data.simulation_seconds)
            total_score += data.score_per_car + (data.simulation_seconds - current_time);
    }

    explicit ScheduleEvaluator(const Data& data, const Schedule& schedule)
    : data(data)
    , total_score(0)
    , car_id_to_street_pos(data.nr_cars)
//    , street_to_queue(data.nr_streets)
    , street_id_to_local_time(data.nr_streets)
    , street_pos_in_sched(data.nr_streets)
    , street_green_info(data.nr_streets)
    , schedule(schedule)
    {
//        street_to_queue.reserve(data.nr_streets);

        precompute_street_green_intervals();
        precompute_street_pos_in_sched();
        precompute_street_green_info();
        insert_starting_streets();
    }

    unsigned long long simulate()
    {
        // Simulate while there are events left and time is not exceeded
        while (!events.empty())
        {
//            const auto [soonest_event_time, affected_street_ids] = get_street_ids_with_next_events();
            const int soonest_event_time = events.find_next_non_empty(0);
            if (soonest_event_time > data.simulation_seconds)
                break;

            const auto soonest_events = events.extract_bucket(soonest_event_time);
//            for (int street_id : affected_street_ids)
            for (const auto& [street_id, car_id] : soonest_events)
            {
//                const auto [time, car_id] = street_to_queue[street_id].top();
//                assert(time == soonest_event_time);

//                street_to_queue[street_id].pop();
                // Two events for the same street should never happen at the same time
                // If last street in path, update score and skip the event updating
                if (is_last_street(car_id))
                {
                    update_score(soonest_event_time);
                    continue;
                }

                // If the schedule doesn't include this street, no need to process further
                if (!is_street_scheduled(street_id))
                    continue;

                // Despite the fact that a car may reach end of street at 'time', we also need 'local_time' in case
                // it has to wait after other cars already queued at the street's end
                // This is only relevant when wanting to pass through the intersection(not on last street in path)
                const int local_time = std::max(soonest_event_time, street_id_to_local_time[street_id]);

                const int next_green_light_time = get_next_green_light_time(street_id, local_time);
                const int next_street_id = get_next_street_for_car(car_id);

                // Time until next event(when car reaches the end of the next street)
                const int next_street_length = std::get<2>(data.street_info.at(next_street_id));
                const int time_until_next_event = next_green_light_time + next_street_length;

                // Update the events queues
//                street_to_queue[next_street_id].emplace(time_until_next_event, car_id);
//                events.emplace(time_until_next_event, next_street_id);
                events.insert({next_street_id, car_id}, time_until_next_event);

                // Update the time for the current street(i.e. how much did the car wait for the green light?)
                street_id_to_local_time[street_id] = next_green_light_time + 1;
                car_id_to_street_pos[car_id]++;
            }
        }
        return total_score;
    }
};