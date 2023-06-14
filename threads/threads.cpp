#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using namespace std::literals;

void background_work(size_t id, const std::string& text, std::chrono::milliseconds delay)
{
    std::cout << "bw#" << id << " has started..." << std::endl;

    for (const auto& c : text)
    {
        std::cout << "bw#" << id << ": " << c << std::endl;

        std::this_thread::sleep_for(delay);
    }

    std::cout << "bw#" << id << " is finished..." << std::endl;
}

class BackgroundWork
{
    const int id_;
    const std::string text_;

public:
    BackgroundWork(int id, std::string text)
        : id_{id}
        , text_{std::move(text)}
    {
    }

    void operator()(std::chrono::milliseconds delay = 100ms) const
    {
        std::cout << "BW#" << id_ << " has started..." << std::endl;

        for (const auto& c : text_)
        {
            std::cout << "BW#" << id_ << ": " << c << std::endl;

            std::this_thread::sleep_for(delay);
        }

        std::cout << "BW#" << id_ << " is finished..." << std::endl;
    }
};

int main()
{
    std::cout << "Main thread starts..." << std::endl;
    const std::string text = "Hello Threads";

    std::thread thd_empty;

    std::cout << "thd_empty.id = " << thd_empty.get_id() << "\n";

    // Option 1 - function
    std::thread thd_1{&background_work, 1, "THREADING", 200ms};
    std::thread thd_2{&background_work, 2, "multi", 150ms};

    // Option 2 - functor
    BackgroundWork bw{3, "BACKGROUN_DWORK"}; // bw - lvalue
    std::thread thd_3{std::cref(bw)};
    std::thread thd_4{BackgroundWork(4, "bw4"), 300ms}; // BackgroundWork(4, "bw4") is rvalue -> move semantics

    // Option 3 - lambda expression
    const int id = 5;
    const std::string text = "TEXT";
    std::thread thd_5{[id, &text](){ background_work(id, text, 400ms); }}; 

    thd_1.join();
    thd_2.join();
    thd_3.join();
    thd_4.join();
    thd_5.join();

    std::cout << "Main thread ends..." << std::endl;
}
