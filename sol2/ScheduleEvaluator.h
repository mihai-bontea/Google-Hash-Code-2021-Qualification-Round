#pragma once
#include <set>
#include <queue>
#include <vector>
#include <numeric>
#include <cassert>
#include <unordered_map>

#include "Data.h"
#include "BucketQueue.h"

struct StreetGreenInfo {
    int green_start;  // offset within cycle
    int green_end;    // offset within cycle
    int cycle_length; // total cycle length at this intersection
};

struct ScheduleEvaluator
{
    const Data& data;
    const Schedule& schedule;

    std::vector<int> street_id_to_local_time;

    std::vector<int> car_id_to_street_pos;

    std::vector<StreetGreenInfo> street_green_info;

    BucketQueue<std::pair<int, int>> events;

    unsigned long long total_score;

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

    inline bool is_street_scheduled(int street_id) const noexcept
    {
        return street_green_info[street_id].cycle_length != -1;
    }

    inline int get_next_street_for_car(int car_id) const noexcept
    {
        const int pos_in_path = car_id_to_street_pos[car_id] + 1;
        return data.car_path_at(car_id, pos_in_path);
    }

    void insert_starting_streets()
    {
        for (int car_id = 0; car_id < data.nr_cars; ++car_id)
        {
            // Cars start at the end of the first street
            const int starting_street_id = data.car_path_first(car_id);

            // If the schedule doesn't include this street, no need to process further
            if (!is_street_scheduled(starting_street_id))
                continue;

            // Since initially multiple cars can be at the end of the 1st street at the exact same time, we need to manually
            // update the local time for each street
            const int local_time = street_id_to_local_time[starting_street_id];
            const int next_green_light_time = get_next_green_light_time(starting_street_id, local_time);
            assert(next_green_light_time >= 0);

            const int next_street_id = get_next_street_for_car(car_id);

            // Time until next event(when car reaches the end of the next street)
            const int time_until_next_event = next_green_light_time + data.street_to_length[next_street_id];

            // Update the events queues
            if (time_until_next_event <= data.simulation_seconds)
                events.insert({next_street_id, car_id}, time_until_next_event);

            // Update the time for the current street(i.e. how much did the car wait for the green light?)
            street_id_to_local_time[starting_street_id] = next_green_light_time + 1;
            car_id_to_street_pos[car_id]++;
        }
    }

    int get_next_green_light_time(int street_id, int current_time) const noexcept
    {
        const auto& info = street_green_info[street_id];
        const int sec_in_cycle = current_time % info.cycle_length;

        if (sec_in_cycle >= info.green_start && sec_in_cycle <= info.green_end)
            return current_time;
        if (sec_in_cycle > info.green_end)
            return current_time + info.cycle_length - sec_in_cycle + info.green_start;
        return current_time + info.green_start - sec_in_cycle;
    }

    inline bool is_last_street(int car_id) const noexcept
    {
        return car_id_to_street_pos[car_id] == (data.car_path_length(car_id) - 1);
    }

    inline void update_score(int current_time) noexcept
    {
        if (current_time <= data.simulation_seconds)
            total_score += data.score_per_car + (data.simulation_seconds - current_time);
    }

    explicit ScheduleEvaluator(const Data& data, const Schedule& schedule)
            : data(data)
            , total_score(0)
            , car_id_to_street_pos(data.nr_cars)
            , street_id_to_local_time(data.nr_streets)
            , street_green_info(data.nr_streets)
            , schedule(schedule)
    {
        precompute_street_green_info();
        insert_starting_streets();
    }

    unsigned long long simulate()
    {
        // Simulate while there are events left and time is not exceeded
        int prev_time = 0;
        while (!events.empty())
        {
            const int soonest_event_time = events.find_next_non_empty(prev_time);
            if (soonest_event_time > data.simulation_seconds)
                break;

            prev_time = soonest_event_time;

            const auto soonest_events = events.extract_bucket(soonest_event_time);
            for (const auto& [street_id, car_id] : soonest_events)
            {
                // Two events for the same street should never happen at the same time
                // If last street in path, update score and skip the event updating
                if (is_last_street(car_id)) [[unlikely]]
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
                const int time_until_next_event = next_green_light_time + data.street_to_length[next_street_id];

                // Update the events queues
                if (time_until_next_event <= data.simulation_seconds)
                    events.insert({next_street_id, car_id}, time_until_next_event);

                // Update the time for the current street(i.e. how much did the car wait for the green light?)
                street_id_to_local_time[street_id] = next_green_light_time + 1;
                car_id_to_street_pos[car_id]++;
            }
        }
        return total_score;
    }
};