#include <map>
#include <array>
#include <iostream>

#include "Data.h"

class BasicSolver
{
private:
    const Data &data;
public:
    explicit BasicSolver(const Data &data)
    : data(data)
    {}

    std::map<int, std::vector<std::pair<std::string,int>>> solve()
    {
        std::map<int, std::vector<std::pair<std::string,int>>> schedule;
        for (const auto& [street_name, _] : data.street_usage)
        {
            int intersection = std::get<1>(data.street_info.at(street_name));
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
        Data data(in_prefix + input_file);
        BasicSolver solver(data);
        const auto result = solver.solve();

        const auto out_filename = out_prefix + input_file.substr(0, (input_file.find('.'))) + ".out";
        Data::write_to_file(out_filename, result);
    }
    return 0;
}
