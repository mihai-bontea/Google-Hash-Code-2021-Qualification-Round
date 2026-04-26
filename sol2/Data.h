#pragma once
#include <map>
#include <vector>
#include <fstream>
#include <unordered_map>

#define MAX_DURATION 8072

// intersection -> {street_id : green_light_seconds}
using Schedule = std::unordered_map<int, std::vector<std::pair<int,int>>>;

struct Data
{
    int simulation_seconds, nr_intersections, nr_streets, nr_cars, score_per_car;

    // street_id -> (start_intersection, end_intersection)
    std::vector<std::pair<int, int>> street_intersect;
    std::vector<int> street_to_length;

    // intersection -> list of incoming street_ids
    std::vector<std::vector<int>> incoming;

    // CSR representation of car paths:
    //   car_paths_data: concatenated path street_ids for all cars
    //   car_paths_offsets: car_paths_offsets[car_id] = start index of car_id's path in car_paths_data
    //   car_id's path length = car_paths_offsets[car_id + 1] - car_paths_offsets[car_id]
    std::vector<int> car_paths_data;
    std::vector<int> car_paths_offsets;

    // street_id -> number of cars using it
    std::vector<int> street_usage;

    // Only used for producing the output file; working on ints is faster than on string
    std::vector<std::string> street_id_to_name;

    inline int car_path_length(int car_id) const noexcept
    {
        return car_paths_offsets[car_id + 1] - car_paths_offsets[car_id];
    }

    inline int car_path_at(int car_id, int pos) const noexcept
    {
        return car_paths_data[car_paths_offsets[car_id] + pos];
    }

    inline const int* car_path_begin(int car_id) const noexcept
    {
        return &car_paths_data[car_paths_offsets[car_id]];
    }

    inline int car_path_first(int car_id) const noexcept
    {
        return car_paths_data[car_paths_offsets[car_id]];
    }

    explicit Data(const std::string& filename)
    {
        std::ifstream fin(filename);

        fin >> simulation_seconds >> nr_intersections >> nr_streets >> nr_cars >> score_per_car;

        incoming.resize(nr_intersections);
        street_intersect.resize(nr_streets);
        street_to_length.resize(nr_streets);
        street_id_to_name.resize(nr_streets);
        street_usage.resize(nr_streets, 0);

        std::unordered_map<std::string, int> street_name_to_id;
        for (int i = 0; i < nr_streets; ++i)
        {
            int start_int, end_int, length_in_sec;
            std::string street_name;
            fin >> start_int >> end_int >> street_name >> length_in_sec;

            street_name_to_id[street_name] = i;
            street_id_to_name[i] = street_name;
            street_intersect[i] = {start_int, end_int};
            street_to_length[i] = length_in_sec;
            incoming[end_int].push_back(i);
        }

        car_paths_offsets.resize(nr_cars + 1);
        car_paths_offsets[0] = 0;

        std::vector<std::vector<int>> tmp_paths(nr_cars);
        int total_path_length = 0;

        for (int i = 0; i < nr_cars; ++i)
        {
            int nr_streets_in_path;
            std::string street_name;

            fin >> nr_streets_in_path;
            tmp_paths[i].reserve(nr_streets_in_path);
            total_path_length += nr_streets_in_path;

            while (nr_streets_in_path--)
            {
                fin >> street_name;
                const int street_id = street_name_to_id[street_name];

                // No need to schedule the last intersection
                if (nr_streets_in_path)
                    street_usage[street_id]++;
                tmp_paths[i].push_back(street_id);
            }
        }

        // Flatten into CSR
        car_paths_data.resize(total_path_length);
        int offset = 0;
        for (int i = 0; i < nr_cars; ++i)
        {
            car_paths_offsets[i] = offset;
            std::copy(tmp_paths[i].begin(), tmp_paths[i].end(), car_paths_data.begin() + offset);
            offset += (int)tmp_paths[i].size();
        }
        car_paths_offsets[nr_cars] = offset;
    }

    void write_to_file(const std::string& filename, const Schedule& schedule) const
    {
        std::ofstream fout(filename);
        fout << schedule.size() << '\n';
        for (const auto& [intersection_id, schedule_for_int] : schedule)
        {
            fout << intersection_id << '\n' << schedule_for_int.size() << '\n';
            for (const auto& [street_id, seconds_green] : schedule_for_int)
                fout << street_id_to_name[street_id] << ' ' << seconds_green << '\n';
        }
    }
};