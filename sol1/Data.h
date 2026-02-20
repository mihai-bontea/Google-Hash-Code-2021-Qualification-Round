#pragma once
#include <vector>
#include <fstream>
#include <unordered_map>

struct Data
{
    int simulation_seconds, nr_intersections, nr_streets, nr_cars, score_per_car;
    std::unordered_map<std::string, std::tuple<int,int,int>> street_info;
    std::vector<std::vector<std::string>> incoming, car_paths;
    std::unordered_map<std::string, int> street_usage;

    explicit Data(const std::string& filename)
    {
        std::ifstream fin(filename);

        fin >> simulation_seconds >> nr_intersections >> nr_streets >> nr_cars >> score_per_car;
        incoming.resize(nr_intersections);
        car_paths.resize(nr_cars);

        for (int i = 0; i < nr_streets; ++i)
        {
            int start_int, end_int, length_in_sec;
            std::string street_name;
            fin >> start_int >> end_int >> street_name >> length_in_sec;
            street_info[street_name] = {start_int, end_int, length_in_sec};
            incoming[end_int].push_back(street_name);
        }

        for (int i = 0; i < nr_cars; ++i)
        {
            int nr_streets_in_path;
            std::string street;

            fin >> nr_streets_in_path;
            while (nr_streets_in_path--)
            {
                fin >> street;
                street_usage[street]++;
                car_paths[i].push_back(street);
            }
        }
    }

    static void write_to_file(const std::string& filename, const std::map<int, std::vector<std::pair<std::string,int>>>& schedule)
    {
        std::ofstream fout(filename);
        fout << schedule.size() << '\n';
        for (const auto& [intersection_id, schedule_for_int] : schedule)
        {
            fout << intersection_id << '\n' << schedule_for_int.size() << '\n';
            for (const auto& [street_name, seconds_green] : schedule_for_int)
                fout << street_name << " " << seconds_green << '\n';
        }
    }
};