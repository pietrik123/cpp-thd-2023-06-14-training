#include <cassert>
#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>

using namespace std::literals;

int calculate_square(int x)
{
    std::cout << "Starting calculation for " << x << " in " << std::this_thread::get_id() << std::endl;

    std::random_device rd;
    std::uniform_int_distribution<> distr(100, 5000);

    std::this_thread::sleep_for(std::chrono::milliseconds(distr(rd)));

    if (x % 3 == 0)
        throw std::runtime_error("Error#3");

    return x * x;
}

void save_to_file(const std::string& filename)
{
    std::cout << "Saving to file: " << filename << std::endl;

    std::this_thread::sleep_for(3s);

    std::cout << "File saved: " << filename << std::endl;
}

int main()
{
    std::future<int> f1 = std::async(std::launch::deferred, calculate_square, 11);
    std::future<int> f2 = std::async(std::launch::async, calculate_square, 13);
    std::future<int> f3 = std::async(std::launch::async, [] { return calculate_square(30); });
    std::future<void> fsave = std::async(std::launch::async, save_to_file, "data.txt");

    //assert(f2.wait_for(1ms) == std::future_status::deferred);

    // while (f1.wait_for(50ms) != std::future_status::ready)
    // {
    //     std::cout << ".";
    //     std::cout.flush();
    // }
    // std::cout << std::endl;

    std::vector<std::future<int>> vec_future_squares;
    vec_future_squares.push_back(std::move(f1));
    vec_future_squares.push_back(std::move(f2));
    vec_future_squares.push_back(std::move(f3));

    for (auto& f_square : vec_future_squares)
    {
        try
        {
            int result = f_square.get();
            std::cout << "result: " << result << "\n";
        }
        catch (const std::runtime_error& e)
        {
            std::cout << "Caught an exception: " << e.what() << "\n";
        }
    }

    try
    {
        vec_future_squares[0].get(); // throws future_error
    }
    catch(const std::future_error& e)
    {
        std::cout << "Many calls to get() not allowed...\n";
    }
    

    std::shared_future shared_f_save = fsave.share();

    std::thread thd1{[shared_f_save] {
        //... create dest directory
        shared_f_save.wait();
        //... copy to destination
    }};

    std::thread thd2{[shared_f_save] {
        //... create dest directory
        shared_f_save.wait();
        //... copy to destination
    }};

    thd1.join();
    thd2.join();
}
