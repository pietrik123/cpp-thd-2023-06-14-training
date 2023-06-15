#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <random>

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


int slots[64];
std::shared_mutex mtx_slots;

void reader()
{
    std::mt19937_64 rnd_gen(std::hash<std::thread::id>{}(std::this_thread::get_id()));
    uint32_t index = rnd_gen() % 64;

    {
        std::shared_lock<std::shared_mutex> lock(mtx_slots); // non-exclusive lock
        std::cout << "I am in CS - reader: " << std::this_thread::get_id() << "\n"; 
        int value = slots[index];
        std::this_thread::sleep_for(3s);
    }
}

void writer()
{
    std::mt19937_64 rnd_gen(std::hash<std::thread::id>{}(std::this_thread::get_id()));
    uint32_t index = rnd_gen() % 64;

    std::lock_guard<std::shared_mutex> lock(mtx_slots); // exclusive lock
    std::cout << "I am in CS - writer: " << std::this_thread::get_id() << "\n"; 
    slots[index] = 42;    
}

void readers_writers()
{
    std::thread thd_reader_1{reader};
    std::thread thd_reader_2{reader};
    std::thread thd_writer{writer};

    thd_reader_1.join();
    thd_reader_2.join();
    thd_writer.join();
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

    std::cout << "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\" << std::endl;

    readers_writers();

    std::cout << "Main thread ends..." << std::endl;
}
