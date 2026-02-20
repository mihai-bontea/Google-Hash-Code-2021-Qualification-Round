#include <map>
#include <array>
#include <numeric>
#include <iostream>
#include <algorithm>

#include "Data.h"
#include "MergeFindSet.h"

class BasicSolver
{
private:
    const Data &data;
public:
    explicit BasicSolver(const Data &data)
    : data(data)
    {}

    double get_avg_frequency() const
    {
        std::vector<int> intersection_frequency(data.nr_intersections, 0);
        for (const auto& car_path : data.car_paths)
        {
            for (int i = 0; i < car_path.size() - 1; ++i)
            {
                const auto &street_name = car_path[i];
                const int intersection = std::get<1>(data.street_info.at(street_name));
                intersection_frequency[intersection]++;
            }
        }
        const int active = (int)std::count_if(intersection_frequency.begin(), intersection_frequency.end(), [](int f){ return f > 0; });
        return std::accumulate(intersection_frequency.begin(), intersection_frequency.end(), 0) / (double)active;
    }

    int count_connected_components() const
    {
        MergeFindSet mfs(data.nr_intersections);

        for (const auto& car_path : data.car_paths)
            for (int i = 0; i + 1 < (int)car_path.size(); ++i)
            {
                int from = std::get<0>(data.street_info.at(car_path[i]));
                int to   = std::get<1>(data.street_info.at(car_path[i]));
                if (!mfs.are_same_set(from + 1, to + 1))
                    mfs.merge_sets(from + 1, to + 1);
            }

        int components = 0;
        for (int i = 0; i < data.nr_intersections; ++i)
            if (mfs.find(i + 1) == (unsigned)(i + 1))
                components++;

        return components;
    }

    std::map<int, std::vector<std::pair<std::string,int>>> solve()
    {
        std::map<int, std::vector<std::pair<std::string,int>>> schedule;
        for (const auto& [street_name, _] : data.street_usage)
        {
            const int intersection = std::get<1>(data.street_info.at(street_name));
            schedule[intersection].emplace_back(street_name, 1);
        }

        return schedule;
    }
};

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
        // These two metrics show the extent to which obtaining the schedule can be parallelized
        // (How much overlap is there between the car paths?)
        std::cout << "The average active intersection influences " << solver.get_avg_frequency() << " cars on average\n";
        std::cout << "The car paths form " << solver.count_connected_components() << " independent components\n\n";

        const auto result = solver.solve();

        const auto out_filename = out_prefix + input_file.substr(0, (input_file.find('.'))) + ".out";
        Data::write_to_file(out_filename, result);
    }
    return 0;
}
