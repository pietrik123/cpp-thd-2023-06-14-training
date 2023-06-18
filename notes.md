# C++ multithreading training notes

My notes from the programming training led by Infotraining / **Krystian Piękoś**.

## Modern C++ thread module

 [48aeb282](https://github.com/pietrik123/cpp-thd-2023-06-14-training/commit/48aeb282aaa9e3284685f24bec0d0db9da16b894) :
* thread object creation, passing various types of callbacks: functions, functor, lambdas
* use `std::ref()` wrapper to pass arguments to callback function when creating thread object
* remember to call `th.join()` in parent thread to wait for child thread finish. `std::jthread` is C++20 joining thread - no need to call `join()`.

[892c58a7](https://github.com/pietrik123/cpp-thd-2023-06-14-training/commit/892c58a7bda0759e11b505f5d51cfc8295bd88b4) : 
* create empty thread
* avoid passing references in lambdas capture lists because of dangling ref. problem. That is: the passed object may end life in the caller's thread , earlier than the child thread finishes. Try to pass by value or do `std::move()`
* `detach()` separates real thread from the `std::thread` object. After detach you cannot call `join()`

[139bd24b](https://github.com/pietrik123/cpp-thd-2023-06-14-training/commit/139bd24b433aef219b1eb501d93b4c61d08c0ce0) :
* when exception happens in the child thread and it is not handled inside the thread's function, program terminates.
* it would be usually better to handle exception in the parent thread. So we may use `std::exception_ptr` and `std::rethrow_exception` in the child thread. After that catching and handling in parent thread is possible.

### Thread excercise

Calculate value of PI number based on Monte-Carlo method.

[59a8643b2d](https://github.com/pietrik123/cpp-thd-2023-06-14-training/commit/59a8643b2da56afce9b782c63ba8f2d062e6f8b4)
* try to speed up calculation and use multiple hardware threads
* `std::thread::hardware_concurrency()` returns number of hardware threads
* since `rand()` is not thread safe it surprisingly takes more time to calculate in the multiple thread calc variant. It depends on the compiler - how `rand()` is implemented.

[9b861eff](https://github.com/pietrik123/cpp-thd-2023-06-14-training/commit/9b861eff7190b66ae2b4f8e0e0b708c85d4e6e33) :
* update Monte-Carlo excercise, use thread-safe version of rand
* use `alignas(hardware_constructive_interference)` to force operation in other cache lines of the proc

## Thread synchronization

### Mutex

[d66939a5](https://github.com/pietrik123/cpp-thd-2023-06-14-training/commit/d66939a57116647dd967ce9ba8118d36c3c0efc4) :
* mutex, lock and unlock
* critical section (code region protected by mutex or sth else) should be as small as possible

[15e2db06](https://github.com/pietrik123/cpp-thd-2023-06-14-training/commit/15e2db06dd5722fa8a85fbd79899e09abcbc90c8) : 
* use RAII wrapper for `std::mutex` -> `std::lock_guard`
* synchronized data structure implementation

[79c5f55a](https://github.com/pietrik123/cpp-thd-2023-06-14-training/commit/79c5f55afa69aea36d26d87d168e7a92936d76ef)
* use mutex in example with PI calculation

[15e2db06](https://github.com/pietrik123/cpp-thd-2023-06-14-training/commit/15e2db06dd5722fa8a85fbd79899e09abcbc90c8) :
* use synchronized value structure in the PI calculation excercies

### Thread sync excercise

Synchronized bank account

[ee52122b](https://github.com/pietrik123/cpp-thd-2023-06-14-training/commit/ee52122b0f74b93cd7328637b681dbebb70fedc4)

* deadlock problem - 2 threads waiting on each other due to order of locks on mutexes
* solve deadlock using `std::unique_lock` with `defer_lock` parameter
* solve deadlock using `std::scoped_lock` (deadlock free technique available since C++17)

### Shared mutex

[497696c2](https://github.com/pietrik123/cpp-thd-2023-06-14-training/commit/497696c2b5362b4f353cb26a1e3304d62efb879b) : 
* use shared mutex to allow multiple readers and exclusive writers

## Event based synchronization

* atomics
* `condition_variable`

### Atomic

`std::atomic<>` wrapper does not require mutex, the date is protected automatically

Two types of waiting:
* busy wait (while loop)
* idle wait - go into sleep mode (condition_variable)

[a80b7a0b29](https://github.com/pietrik123/cpp-thd-2023-06-14-training/commit/a80b7a0b29d0b828a1534f61b7bbcaf8fbcab560) : 
* use atomic variable in PI calc exercise

### Condition variable

`condition_variable` :
* used to synchronize threads
* `wait()` to block a thread waiting for specific event
* `notify_one()` / `notify_all()` to unblock one or multiple threads
* spurious wakups - sleeping thread can wake up without a reason in some conditions (OS dependent, can be avoided using construction with additional variable `cv.wait(uniqueLockObj, []() { return isDataReadyl;} )`

[06e2fa5dd7](https://github.com/pietrik123/cpp-thd-2023-06-14-training/commit/06e2fa5dd769bfa52537750634112628455cab3e) :
* data preparation / process example - one thread prepares a data and the other processes it
* processing threads wait for data ready (wait)
* preparing thread notifies about the data ready state

### Excercise - thread safe queue

[658c7d3d](https://github.com/pietrik123/cpp-thd-2023-06-14-training/commit/658c7d3d40abb35d210df732d7060b72bfcd35f8) :
* create a thread-safe queue as template class (mutex + condition_variable)

[5505ea75](https://github.com/pietrik123/cpp-thd-2023-06-14-training/commit/5505ea75d03a400437a29e3a784e849dc5ced365)
* implement thread pool using thread-safe queue from previous
* the pool has a fixed number of threads
* the pool has `submit()` function which accepts `Task t` parameter
* the pool queues tasks and assigns worker thread, when a job is done, the thread is freed waiting for other tasks

## Future

Future
* Calling `std::async()` returns a `std::future` object.
```
auto futureObj = std::async(launchtype::async, function);
``` 
* the function is executed in a separate thread
* `std::future` has `get()` method which blocks the calling thread until job in caller thread is done. when job is done it returns the result

[660a4b24](https://github.com/pietrik123/cpp-thd-2023-06-14-training/commit/660a4b24c60d96a9efb8f77dd16acc35463f0769) :
* futures

[4e2039f3](https://github.com/pietrik123/cpp-thd-2023-06-14-training/commit/4e2039f342a280f55ceb4bed0535125d67a75bf6)
* use `std::promise` - promise is fulfilled in another thread (set value). promise has `get_future()` object that returns `std::future` object, that can block the parent thread using `get()` or `wait()` (until promise is fulfilled)

[6bd23b31](https://github.com/pietrik123/cpp-thd-2023-06-14-training/commit/6bd23b3133e3a695ce95f3f94a1c03cec020b0c5)
* use futures in PI calculation exercise