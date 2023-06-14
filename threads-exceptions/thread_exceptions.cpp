#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using namespace std::literals;

template <typename T>
struct ThreadResult
{
    T value;
    std::exception_ptr eptr;

    T& get()
    {
        if (eptr)
        {
            std::rethrow_exception(eptr);
        }

        return value;
    }
};

void background_work(size_t id, const std::string& text, ThreadResult<char>& result)
{
    std::cout << "bw#" << id << " has started..." << std::endl;

    for (const auto& c : text)
    {
        std::cout << "bw#" << id << ": " << c << std::endl;

        std::this_thread::sleep_for(100ms);
    }

    try
    {
        result.value = text.at(5); // potential exception
    }
    catch (...)
    {
        result.eptr = std::current_exception();
    }

    std::cout << "bw#" << id << " is finished..." << std::endl;
}

int main()
{
    std::cout << "No of cores: " << std::max(1u, std::thread::hardware_concurrency()) << "\n";

    std::cout << "Main thread starts..." << std::endl;
    const std::string text = "Hello Threads";

    std::vector<ThreadResult<char>> results(2);

    std::thread thd_1{&background_work, 1, "multithreading", std::ref(results[0])}; // OK
    std::thread thd_2{&background_work, 2, "abc", std::ref(results[1])};            // Failure
    thd_1.join();
    thd_2.join();

    for (auto& result : results)
    {
        try
        {
            auto& r = result.get();
            std::cout << "result: " << r << "\n";
        }
        catch (const std::out_of_range& e)
        {
            // exception handling
            std::cout << "Caught an exception: " << e.what() << "\n";
        }
        catch (const std::exception& e)
        {
            // exception handling
            std::cout << "Caught an exception: " << e.what() << "\n";
        }
    }
}