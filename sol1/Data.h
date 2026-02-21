#pragma once
#include <map>
#include <vector>
#include <fstream>
#include <unordered_map>

// intersection -> {street_id : green_light_seconds}
using Schedule = std::map<int, std::vector<std::pair<int,int>>>;

struct Data
{
    int simulation_seconds, nr_intersections, nr_streets, nr_cars, score_per_car;

    // street_id -> (start_intersection, end_intersection, length)
    std::vector<std::tuple<int,int,int>> street_info;

    // intersection -> list of incoming street_ids
    std::vector<std::vector<int>> incoming;

    // car -> list of street_ids in path
    std::vector<std::vector<int>> car_paths;

    // street_id -> number of cars using it
    std::vector<int> street_usage;

    // Only used for producing the output file; working on ints is faster than on string
    std::vector<std::string> street_id_to_name;

    explicit Data(const std::string& filename)
    {
        std::ifstream fin(filename);

        fin >> simulation_seconds >> nr_intersections >> nr_streets >> nr_cars >> score_per_car;

        incoming.resize(nr_intersections);
        car_paths.resize(nr_cars);
        street_info.resize(nr_streets);
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
            street_info[i] = {start_int, end_int, length_in_sec};
            incoming[end_int].push_back(i);
        }

        for (int i = 0; i < nr_cars; ++i)
        {
            int nr_streets_in_path;
            std::string street_name;

            fin >> nr_streets_in_path;
            while (nr_streets_in_path--)
            {
                fin >> street_name;
                int id = street_name_to_id[street_name];
                street_usage[id]++;
                car_paths[i].push_back(id);
            }
        }
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