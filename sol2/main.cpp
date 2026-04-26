#include <array>
#include <iostream>

#include "Data.h"
#include "GeneticAlgorithm.h"

int main()
{
#ifdef _OPENMP
    std::cout << "OpenMP version: " << _OPENMP << '\n';
    std::cout << "Max threads: " << omp_get_max_threads() << '\n';
    std::cout << "Num procs: " << omp_get_num_procs() << '\n';
#else
    std::cout << "OpenMP NOT defined\n";
#endif

    const std::string in_prefix = "../../input_files/";
    const std::string out_prefix = "../../output_files/sol2/";
    const std::array<std::string, 6> input_files = {"a_example.in", "b_ocean.in", "c_checkmate.in",
                                                    "d_daily_commute.in", "e_etoile.in", "f_forever_jammed.in"};

    for (const auto& input_file : input_files)
    {
//        if (input_file != "d_daily_commute.in")
//            continue;

        std::cout << "Now working on " << input_file << std::endl;
        Data data(in_prefix + input_file);

        GeneticAlgorithm genetic_algorithm(data);
        const auto best_schedule = genetic_algorithm.run();

        const auto out_filename = out_prefix + input_file.substr(0, (input_file.find('.'))) + ".out";
        data.write_to_file(out_filename, best_schedule);
    }
    return 0;
}
