#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <thread>
#include <vector>

using namespace std::literals;

namespace Atomics
{

    std::atomic<bool> global_flag{false};

    class Data
    {
        std::vector<int> data_;
        std::atomic<bool> is_data_ready_ = false;

    public:
        void read()
        {
            std::cout << "Start reading..." << std::endl;
            data_.resize(100);

            std::random_device rnd;
            std::generate(begin(data_), end(data_), [&rnd] { return rnd() % 1000; });
            std::this_thread::sleep_for(2s);
            std::cout << "End reading..." << std::endl;

            global_flag = true; // global_flag.store(true);

            // is_data_ready_ = true;
            is_data_ready_.store(true, std::memory_order_release); // one-way barrier
        }

        void process(int id)
        {
            while (!is_data_ready_.load(std::memory_order_acquire)) { } // busy wait // one-way barrier
            long sum = std::accumulate(begin(data_), end(data_), 0L);

            std::cout << "Id: " << id << "; Sum: " << sum << std::endl;
        }
    };
} // namespace Atomics

int main()
{
    {
        using namespace Atomics;
        
        Data data;
        std::thread thd_producer{[&data] {
            data.read();
        }};

        std::thread thd_consumer_1{[&data] {
            data.process(1);
        }};
        std::thread thd_consumer_2{[&data] {
            data.process(2);
        }};

        std::cout << global_flag.load() << "\n";

        thd_producer.join();
        thd_consumer_1.join();
        thd_consumer_2.join();
    }

    std::cout << "END of main..." << std::endl;
}
