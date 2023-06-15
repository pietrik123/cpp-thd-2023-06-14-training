#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>

using namespace std::literals;

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
        return std::lock_guard{mtx_value};
    }
};

void run(int& value, std::mutex& mtx_value)
{
    for(int i = 0; i < 10'000'000; ++i)
    {       
        {  ///////// CS - start
            std::lock_guard lk{mtx_value};
            ++value;
        }  ///////// CS - end
    }
}

void run(SynchronizedValue<int>& counter)
{
    for(int i = 0; i < 10'000'000; ++i)
    {       
        {  ///////// CS - start
            std::lock_guard lk{counter};
            ++counter.value;
        }  ///////// CS - end

        {  ///////// CS - start
            auto lk = counter.with_lock();
            ++counter.value;
        }  ///////// CS - end
    }
}

int main()
{
    std::cout << "Main thread starts..." << std::endl;

    int counter = 0;
    std::mutex mtx_counter;

    SynchronizedValue<int> synced_counter{};
    
    std::thread thd_1{[&counter, &mtx_counter] { run(counter, mtx_counter); }};
    std::thread thd_2{[&counter, &mtx_counter] { run(counter, mtx_counter); }};

    std::thread thd_3{[&synced_counter] { run(synced_counter); }};
    std::thread thd_4{[&synced_counter] { run(synced_counter); }};
    
    thd_1.join();
    thd_2.join();
    thd_3.join();
    thd_4.join();

    std::cout << "counter: " << counter << "\n";
    std::cout << "synced_counter: " << synced_counter.value << "\n";

    std::cout << "Main thread ends..." << std::endl;
}
