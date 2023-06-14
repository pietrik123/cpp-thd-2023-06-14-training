#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "joining_thread.hpp"

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

struct X
{
    void f(int arg) const {}
};

void may_throw()
{
    throw std::runtime_error("Error#13");
}

std::atomic<int> create_counter()
{
    return std::atomic<int>{0};  // prvalue - RVO
}

std::thread create_thread(const std::string& text)
{
    static int id_gen = 665;
    //return std::thread{&background_work, ++id_gen, text, 100ms}; // prvalue - RVO
    return std::thread{[text] { background_work(++id_gen, text, 100ms); }}; // capture by value is safer - avoid dangling references!!!

    // std::thread thd{&background_work, ++id_gen, text, 100ms};
    // return thd; // lvalue - probably NRVO
}

int main()
{
    std::cout << "Main thread starts..." << std::endl;
    const std::string text = "Hello Threads";

    std::thread thd_empty;
    thd_empty = std::thread{&background_work, 0, "no-longer-empty", 200ms};

    std::cout << "thd_empty.id = " << thd_empty.get_id() << "\n";

    // Option 1 - function
    std::thread thd_1{&background_work, 1, "THREADING", 200ms};
    std::thread thd_2{&background_work, 2, "multi", 150ms};

    // Option 2 - functor
    BackgroundWork bw{3, "BACKGROUND_DWORK"}; // bw - lvalue
    std::thread thd_3{std::cref(bw)};
    std::thread thd_4{BackgroundWork(4, "bw4"), 300ms}; // BackgroundWork(4, "bw4") is rvalue -> move semantics

    // Option 3 - lambda expression
    const int id = 5;
    std::thread thd_5{[id, &text](){ background_work(id, text, 400ms); }}; 

    X obj;
    auto functor_f = std::mem_fn(&X::f);
    functor_f(obj, 42); // obj.f()
    std::thread thd_6{functor_f, std::ref(obj), 42};
    std::thread thd_7{[obj = std::move(obj)] { obj.f(42); }};

    //may_throw();
    std::thread thd_8 = create_thread("THD8");

    thd_1.detach();
    std::vector<std::thread> thds;
    thds.push_back(std::move(thd_empty));
    thds.push_back(std::move(thd_2));
    thds.push_back(std::move(thd_3));
    thds.push_back(std::move(thd_4));
    thds.push_back(std::move(thd_5));
    thds.push_back(std::move(thd_6));
    thds.push_back(std::move(thd_7));
    thds.push_back(std::move(thd_8));

    for(auto& thd : thds)
    {
        if (thd.joinable())
            thd.join();
    }
    
    /////////////////////////////////////////////////////////////
    
    const std::vector source = {1, 2, 3, 4, 5, 6};
    std::vector<int> target_1(source.size());
    std::vector<int> target_2(source.size());

    {
        ext::joining_thread thd_copy_1{[&source, &target_1] { std::copy(source.begin(), source.end(), target_1.begin()); } };
        ext::joining_thread thd_copy_2{[&source, &target_2] { std::copy(source.begin(), source.end(), target_2.begin()); } };
    }
    
    for(const auto& item : target_1)
        std::cout << item << " ";
    std::cout << "\n";

    for(const auto& item : target_2)
        std::cout << item << " ";
    std::cout << "\n";

    std::cout << "Main thread ends..." << std::endl;
}
