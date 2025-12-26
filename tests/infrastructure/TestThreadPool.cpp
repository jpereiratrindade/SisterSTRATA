#include <iostream>
#include <vector>
#include <chrono>
#include <cassert>
#include "infrastructure/threading/ThreadPool.hpp"

void test_simple_task() {
    Infrastructure::Threading::ThreadPool pool(2);
    auto fut = pool.submit([]{ return 42; });
    assert(fut.get() == 42);
    std::cout << "Simple task passed!" << std::endl;
}

void test_parallel_execution() {
    Infrastructure::Threading::ThreadPool pool(4);
    std::mutex mtx;
    std::vector<int> results;
    std::vector<std::future<void>> futures;

    for(int i=0; i<10; ++i) {
        futures.emplace_back(pool.submit([&mtx, &results, i]{
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            std::lock_guard<std::mutex> lock(mtx);
            results.push_back(i);
        }));
    }

    for(auto& f : futures) f.get();

    assert(results.size() == 10);
    std::cout << "Parallel execution passed!" << std::endl;
}

int main() {
    test_simple_task();
    test_parallel_execution();
    return 0;
}
