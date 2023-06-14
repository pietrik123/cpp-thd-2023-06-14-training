#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <stop_token>

using namespace std::literals;

void background_work(std::stop_token final_stopper, std::stop_token stopper, size_t id, const std::string& text, std::chrono::milliseconds delay)
{
    std::cout << "bw#" << id << " has started..." << std::endl;

    uint32_t index = 0;
    const uint32_t size = text.size();
    while(true)
    {
        uint32_t i = (index++) % size;
        std::cout << "bw#" << id << ": " << text.at(i) << std::endl;

        if (stopper.stop_requested())
        {
            std::cout << "bw#" << id << " was cancelled..." << std::endl;
            return;
        }

        std::this_thread::sleep_for(delay);

        if (final_stopper.stop_requested())
        {
            std::cout << "bw#" << id << " was cancelled by jthread destructor..." << std::endl;
            return;
        }

    }

    std::cout << "bw#" << id << " is finished..." << std::endl;
}

int main()
{
    std::cout << "Main thread starts..." << std::endl;
    const std::string text = "Hello Threads";

    std::stop_source stop_src;
    
    // {
    //     auto my_token = stop_src.get_token();
    //     std::jthread thd_1{&background_work, 1, "TEXT", 200ms};
    //     std::jthread thd_2{&background_work, 2, "text", 50ms};

    //     std::this_thread::sleep_for(1s);
    // } // implicit join

    //stop_src.request_stop();

    {
        std::jthread thd_1;
        std::stop_source stop_src2;

        auto my_token = stop_src2.get_token();
        thd_1 = std::jthread{&background_work, my_token, 1, "TEXT", 200ms};
        std::jthread thd_2{&background_work, std::stop_token{}, 2, "text", 50ms};

        std::this_thread::sleep_for(1s);
        stop_src2.request_stop();
        std::this_thread::sleep_for(1s);
    } // implicit join
    // ~stop_src2

    
    std::cout << "Main thread ends..." << std::endl;
}
