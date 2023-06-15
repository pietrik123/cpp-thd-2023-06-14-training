#include <atomic>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>
#include <thread>

using namespace std::literals;

/*******************************************************
 * https://academo.org/demos/estimating-pi-monte-carlo
 ******************************************************/

int main()
{
    std::cout << "No of cores: " << std::max(1u, std::thread::hardware_concurrency()) << "\n";

    const uintmax_t N = 100'000'000;

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    {
        //////////////////////////////////////////////////////////////////////////////
        // single thread

        std::cout << "Pi calculation started!" << std::endl;
        const auto start = std::chrono::high_resolution_clock::now();

        uintmax_t hits = 0;

        for (uintmax_t n = 0; n < N; ++n)
        {
            double x = rand() / static_cast<double>(RAND_MAX);
            double y = rand() / static_cast<double>(RAND_MAX);
            if (x * x + y * y < 1)
                hits++;
        }

        const double pi = static_cast<double>(hits) / N * 4;

        const auto end = std::chrono::high_resolution_clock::now();
        const auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        std::cout << "Pi (sync) = " << pi << std::endl;
        std::cout << "Elapsed = " << elapsed_time << "ms" << std::endl;
    }

    std::cout << "///////////////////////////////////////////////////" << std::endl;

    {
        const auto no_of_cores = std::max(1u, std::thread::hardware_concurrency());

        const auto n_per_thread = N / no_of_cores;

        std::vector<std::thread> threads;
        std::vector<uintmax_t> hits_per_thread(no_of_cores);

        auto calc_hits = [n_per_thread](uintmax_t& counter) {
            for (uintmax_t n = 0; n < n_per_thread; ++n)
            {
                double x = rand() / static_cast<double>(RAND_MAX);
                double y = rand() / static_cast<double>(RAND_MAX);
                if (x * x + y * y < 1)
                    counter++;
            }
        };

        auto start = std::chrono::high_resolution_clock::now();

        for (auto i = 0; i < no_of_cores; i++)
        {
            threads.emplace_back(calc_hits, std::ref(hits_per_thread[i]));
        }

        for (auto& th : threads)
        {
            th.join();
        }

        uintmax_t hits = 0;

        for (const auto& item : hits_per_thread)
        {
            hits += item;
        }

        const double pi = static_cast<double>(hits) / N * 4;

        auto end = std::chrono::high_resolution_clock::now();

        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        std::cout << "Pi (multi) = " << pi << std::endl;
        std::cout << "Elapsed = " << elapsed_time << "ms" << std::endl;
    }
}
