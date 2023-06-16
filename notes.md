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