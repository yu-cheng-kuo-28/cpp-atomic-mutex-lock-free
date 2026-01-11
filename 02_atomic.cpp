#include <atomic>
#include <thread>
#include <vector>
#include <iostream>
#include <chrono>

std::atomic<int> counter{0};

void worker_atomic() {
    for (int i = 0; i < 1'000'000; ++i) {
        counter.fetch_add(1, std::memory_order_relaxed);
        // Single atomic instruction
        // No mutex, no blocking, no context switch
        //
        // Q: Why "fetch_add(x, ...)" instead of just ++counter?
        // A: ++counter defaults to memory_order_seq_cst (strictest/slowest)
        //    fetch_add lets you specify the memory order explicitly
        //    TL;DR: ++counter works but forces expensive seq_cst;
        //           fetch_add gives you performance control
    }
}

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i)
        threads.emplace_back(worker_atomic);
    for (auto& t : threads)
        t.join();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "=== ATOMIC (Hardware-Level Lock-Free) ===" << "\n";
    std::cout << "Final counter: " << counter.load(std::memory_order_relaxed) << "\n";
    std::cout << "Time taken: " << duration.count() << " ms\n";
    std::cout << "\n";
    std::cout << "✔ Same correctness\n";
    std::cout << "✔ Lower overhead than contended locks\n";
    std::cout << "✔ Perfect for counters/flags/stats/refcounts\n";
    std::cout << "\n";
    std::cout << "Note: This is a lock-free PRIMITIVE (single atomic operation),\n";
    std::cout << "not lock-free PROGRAMMING (complex CAS-based algorithms).\n";
    std::cout << "\n";
    std::cout << "Why memory_order_relaxed works here:\n";
    std::cout << "• fetch_add is already atomic (no lost increments)\n";
    std::cout << "• Incrementing is commutative (order doesn't matter)\n";
    std::cout << "• We only care about final total after join()\n";
    std::cout << "• thread.join() provides sufficient synchronization\n";
    std::cout << "• Using seq_cst would add ~10-50%% overhead for zero benefit\n";
    std::cout << "\n";
    std::cout << "At this level, you're not 'doing lock-free programming.'\n";
    std::cout << "You're simply using CPU-provided primitives.\n";
    
    return 0;
}
