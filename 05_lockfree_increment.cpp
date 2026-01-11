#include <atomic>
#include <thread>
#include <vector>
#include <iostream>
#include <chrono>

// Lock-free increment using CAS loop
// This is what happens when you implement your own atomic operation

std::atomic<int> counter{0};

void lock_free_increment() {
    int old = counter.load(std::memory_order_relaxed);
    while (!counter.compare_exchange_weak(
        old, old + 1,
        std::memory_order_relaxed,
        std::memory_order_relaxed)) {
        // Retry
        // No blocking
        // System-wide progress is guaranteed (lock-free property)
        //
        // But note: we're reimplementing what fetch_add() already does!
        // This is SLOWER than using the built-in atomic primitive
    }
}

void worker_lockfree() {
    for (int i = 0; i < 1'000'000; ++i) {
        lock_free_increment();
    }
}

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i)
        threads.emplace_back(worker_lockfree);
    for (auto& t : threads)
        t.join();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "=== LOCK-FREE ALGORITHM (CAS Loop) ===" << "\n";
    std::cout << "Final counter: " << counter << "\n";
    std::cout << "Time taken: " << duration.count() << " ms\n";
    std::cout << "\n";
    std::cout << "✔ No blocking\n";
    std::cout << "✔ System-wide progress guarantee (lock-free property)\n";
    std::cout << "❌ Easy to get almost right\n";
    std::cout << "❌ Hard to maintain\n";
    std::cout << "\n";
    std::cout << "Important distinction:\n";
    std::cout << "• Atomic primitives (fetch_add) = lock-free at HARDWARE level\n";
    std::cout << "• CAS loops (this code) = lock-free at ALGORITHM level\n";
    std::cout << "• This is SLOWER than fetch_add() - we're reimplementing it!\n";
    
    return 0;
}
