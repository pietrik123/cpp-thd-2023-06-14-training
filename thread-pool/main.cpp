#include "thread_safe_queue.hpp"

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

using Task = std::function<void()>;

class ThreadPool
{
    static constexpr nullptr_t end_of_work{};
public:
    explicit ThreadPool(uint32_t size)
        : threads_(size)
    {
        for (auto& thd : threads_)
        {
            thd = std::thread([this] { run(); });
        }
    }

    ~ThreadPool()
    {
        // for(size_t i = 0; i < threads_.size(); ++i)
        // {
        //     submit([this] { keep_running = false; });
        // }

        for (size_t i = 0; i < threads_.size(); ++i)
        {
            tsq_.push(end_of_work);
        }

        for (auto& thd : threads_)
        {
            thd.join();
        }
    }

    void submit(Task tsk)
    {
        if (!tsk)
            throw std::invalid_argument("Empty function not allowed");
        
        tsq_.push(tsk);
    }

private:    
    std::vector<std::thread> threads_;
    ThreadSafeQueue<Task> tsq_;
    // std::atomic<bool> keep_running = true;

    void run()
    {
        // while (keep_running)
        // {
        //     Task t;
        //     tsq_.pop(t);
        //     t();
        // }

        while (true)
        {
            Task t;
            tsq_.pop(t);
            
            if (t == end_of_work)
                return;
            
            t();
        }
    }
};

int main()
{
    std::cout << "Main thread starts..." << std::endl;
    const std::string text = "Hello Threads";

    const size_t no_of_cores = std::max(1u, std::thread::hardware_concurrency());

    {
        ThreadPool thd_pool(no_of_cores);

        thd_pool.submit([] { background_work(1, "Text#1", 200ms); });
        thd_pool.submit([] { background_work(2, "Text#2", 200ms); });

        for (int i = 3; i < 20; ++i)
            thd_pool.submit([i]() { background_work(i, "Text#" + std::to_string(i), 150ms); });
    }

    std::cout << "Main thread ends..." << std::endl;
}
