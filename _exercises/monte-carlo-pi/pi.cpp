#include <atomic>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>
#include <thread>

using namespace std;

/*******************************************************
 * https://academo.org/demos/estimating-pi-monte-carlo
 * *****************************************************/

int main()
{
    std::cout << "No of cores: " << std::max(1u, std::thread::hardware_concurrency()) << "\n";

    const long N = 100'000'000;

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    {
        //////////////////////////////////////////////////////////////////////////////
        // single thread

        cout << "Pi calculation started!" << endl;
        const auto start = chrono::high_resolution_clock::now();

        long hits = 0;

        for (long n = 0; n < N; ++n)
        {
            double x = rand() / static_cast<double>(RAND_MAX);
            double y = rand() / static_cast<double>(RAND_MAX);
            if (x * x + y * y < 1)
                hits++;
        }

        const double pi = static_cast<double>(hits) / N * 4;

        const auto end = chrono::high_resolution_clock::now();
        const auto elapsed_time = chrono::duration_cast<chrono::milliseconds>(end - start).count();

        cout << "Pi (sync) = " << pi << endl;
        cout << "Elapsed = " << elapsed_time << "ms" << endl;
    }

    //////////////////////////////////////////////////////////////////////////////

    {
        const auto numOfCores = std::max(1u, std::thread::hardware_concurrency());

        const auto n_per_thread = N / numOfCores;

        std::vector<std::thread> threads;
        std::vector<long> hitsVect(numOfCores);

        auto calcFunction = [n_per_thread](long& counter) {
            for (long n = 0; n < n_per_thread; ++n)
            {
                double x = rand() / static_cast<double>(RAND_MAX);
                double y = rand() / static_cast<double>(RAND_MAX);
                if (x * x + y * y < 1)
                    counter++;
            }
        };

        auto start = chrono::high_resolution_clock::now();

        for (auto i = 0; i < numOfCores; i++)
        {
            threads.emplace_back(calcFunction, std::ref(hitsVect[i]));
        }

        for (auto& th : threads)
        {
            th.join();
        }

        long hits = 0;

        for (const auto& item : hitsVect)
        {
            hits += item;
        }

        const double pi = static_cast<double>(hits) / N * 4;

        auto end = chrono::high_resolution_clock::now();

        auto elapsed_time = chrono::duration_cast<chrono::milliseconds>(end - start).count();

        cout << "Pi (multi) = " << pi << endl;
        cout << "Elapsed = " << elapsed_time << "ms" << endl;
    }
}
