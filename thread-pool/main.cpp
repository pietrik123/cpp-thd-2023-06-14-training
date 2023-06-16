#include "thread_safe_queue.hpp"

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

using Task = std::function<void()>;
// using Task = folly::Function<void()>;

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

    template <typename F>
    auto submit(F&& f)
    {
        using ResultT = decltype(f());
        auto pt = std::make_shared<std::packaged_task<ResultT()>>(std::forward<F>(f));
        auto f_result = pt->get_future();

        tsq_.push([pt]() mutable { (*pt)(); });

        return f_result;
    }

    ~ThreadPool()
    {
        for (size_t i = 0; i < threads_.size(); ++i)
        {
            submit([this] { keep_running = false; });
        }

        for (auto& thd : threads_)
        {
            thd.join();
        }
    }

private:
    std::vector<std::thread> threads_;
    ThreadSafeQueue<Task> tsq_;
    std::atomic<bool> keep_running = true;

    void run()
    {
        while (keep_running)
        {
            Task t;
            tsq_.pop(t);
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

        std::future<int> f = thd_pool.submit([] { return calculate_square(13); });

        for (int i = 3; i < 20; ++i)
            thd_pool.submit([i]() { background_work(i, "Text#" + std::to_string(i), 150ms); });

        std::cout << "13 * 13 = " << f.get() << std::endl;
    }

    std::cout << "Main thread ends..." << std::endl;
}
