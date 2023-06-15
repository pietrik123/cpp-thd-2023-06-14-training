#include <atomic>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>
#include <thread>
#include <list>

using namespace std::literals;

//inline constexpr size_t hardware_destructive_interference_size = 64U;

/*******************************************************
 * https://academo.org/demos/estimating-pi-monte-carlo
 ******************************************************/

void calc_hits(uintmax_t n_per_thread, uintmax_t& counter)
{
    size_t seed = std::hash<std::thread::id>{}(std::this_thread::get_id());
    std::mt19937_64 rnd_gen(seed);
    std::uniform_real_distribution<double> rnd(0, 1.0);

    for (uintmax_t n = 0; n < n_per_thread; ++n)
    {
        double x = rnd(rnd_gen);
        double y = rnd(rnd_gen);

        if (x * x + y * y < 1)
            counter++;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void calc_hits_with_local_counter(uintmax_t n_per_thread, uintmax_t& counter)
{
    uintmax_t local_hits{};
    size_t seed = std::hash<std::thread::id>{}(std::this_thread::get_id());
    std::mt19937_64 rnd_gen(seed);
    std::uniform_real_distribution<double> rnd(0, 1.0);

    for (uintmax_t n = 0; n < n_per_thread; ++n)
    {
        double x = rnd(rnd_gen);
        double y = rnd(rnd_gen);
        
        if (x * x + y * y < 1)
            local_hits++; // local update in hot loop
    }

    counter += local_hits; // update global state
}

////////////////////////////////////////////////////////////////////////////////////////////////////

struct Hits
{
    alignas(std::hardware_destructive_interference_size) uintmax_t value;
};

void calc_hits_with_padding(const uintmax_t count, Hits& hits)
{
    std::mt19937_64 rnd_gen(std::hash<std::thread::id>{}(std::this_thread::get_id()));
    std::uniform_real_distribution<double> rnd(0, 1.0);

    for (uintmax_t n = 0; n < count; ++n)
    {
        double x = rnd(rnd_gen);
        double y = rnd(rnd_gen);
        if (x * x + y * y < 1)
            hits.value++;
    }
}

template <typename T, typename TMutex = std::mutex>
struct SynchronizedValue
{
    T value;
    TMutex mtx_value;

    void lock()
    {
        mtx_value.lock();
    }

    void unlock()
    {
        mtx_value.unlock();
    }

    std::lock_guard<TMutex> with_lock() // since C++17
    {
        return std::lock_guard{mtx_value}; // prvalue
    }
};

void calc_hits_with_mutex(const uintmax_t count, SynchronizedValue<uintmax_t>& hits)
{
    std::mt19937_64 rnd_gen(std::hash<std::thread::id>{}(std::this_thread::get_id()));
    std::uniform_real_distribution<double> rnd(0, 1.0);

    uintmax_t local_hits{};
    for (uintmax_t n = 0; n < count; ++n)
    {
        double x = rnd(rnd_gen);
        double y = rnd(rnd_gen);
        if (x * x + y * y < 1)
        {            
            local_hits++;
        }
    }

    {
        auto lck = hits.with_lock();
        hits.value += local_hits;
    }
}

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

        calc_hits(N, hits);

        const double pi = static_cast<double>(hits) / N * 4;

        const auto end = std::chrono::high_resolution_clock::now();
        const auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        std::cout << "Pi (sync) = " << pi << std::endl;
        std::cout << "Elapsed = " << elapsed_time << "ms" << std::endl;
    }

    std::cout << "///////////////////////////////////////////////////" << std::endl;

    {
        const auto no_of_cores = std::max(1u, std::thread::hardware_concurrency());

        const uintmax_t n_per_thread = N / no_of_cores;

        std::vector<std::thread> threads;
        std::vector<uintmax_t> hits_per_thread(no_of_cores);

        auto start = std::chrono::high_resolution_clock::now();

        for (auto i = 0; i < no_of_cores; i++)
        {
            threads.emplace_back(&calc_hits, n_per_thread, std::ref(hits_per_thread[i]));
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

    std::cout << "///////////////////////////////////////////////////" << std::endl;

    {
        const auto no_of_cores = std::max(1u, std::thread::hardware_concurrency());

        const uintmax_t n_per_thread = N / no_of_cores;

        std::vector<std::thread> threads;
        std::vector<uintmax_t> hits_per_thread(no_of_cores);

        auto start = std::chrono::high_resolution_clock::now();

        for (auto i = 0; i < no_of_cores; i++)
        {
            threads.emplace_back(&calc_hits_with_local_counter, n_per_thread, std::ref(hits_per_thread[i]));
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

        std::cout << "Pi (multi - local counter) = " << pi << std::endl;
        std::cout << "Elapsed = " << elapsed_time << "ms" << std::endl;
    }

    std::cout << "///////////////////////////////////////////////////" << std::endl;

    {
        const auto no_of_cores = std::max(1u, std::thread::hardware_concurrency());

        const uintmax_t n_per_thread = N / no_of_cores;

        std::vector<std::thread> threads;
        std::vector<Hits> hits_per_thread(no_of_cores);

        auto start = std::chrono::high_resolution_clock::now();

        for (auto i = 0; i < no_of_cores; i++)
        {
            threads.emplace_back(&calc_hits_with_padding, n_per_thread, std::ref(hits_per_thread[i]));
        }

        for (auto& th : threads)
        {
            th.join();
        }

        uintmax_t hits = 0;

        for (const auto& partial_hits : hits_per_thread)
        {
            hits += partial_hits.value;
        }

        const double pi = static_cast<double>(hits) / N * 4;

        auto end = std::chrono::high_resolution_clock::now();

        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        std::cout << "Pi (multi - padding) = " << pi << std::endl;
        std::cout << "Elapsed = " << elapsed_time << "ms" << std::endl;
    }

    std::cout << "///////////////////////////////////////////////////" << std::endl;

    {
        const auto no_of_cores = std::max(1u, std::thread::hardware_concurrency());

        const uintmax_t n_per_thread = N / no_of_cores;

        std::vector<std::thread> threads;
        SynchronizedValue<uintmax_t> hits{};

        auto start = std::chrono::high_resolution_clock::now();

        for (auto i = 0; i < no_of_cores; i++)
        {
            threads.emplace_back(&calc_hits_with_mutex, n_per_thread, std::ref(hits));
        }

        for (auto& th : threads)
        {
            th.join();
        }

        const double pi = static_cast<double>(hits.value) / N * 4;

        auto end = std::chrono::high_resolution_clock::now();

        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        std::cout << "Pi (mutex) = " << pi << std::endl;
        std::cout << "Elapsed = " << elapsed_time << "ms" << std::endl;
    }
}
