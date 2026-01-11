#include <atomic>
#include <thread>
#include <vector>
#include <iostream>
#include <chrono>

// CAS loop example: "increment if below a threshold"
// No built-in atomic provides this

std::atomic<int> counter{0};

void bounded_increment(std::atomic<int>& v) {
    int old = v.load(std::memory_order_relaxed);
    while (old < 100 &&
           !v.compare_exchange_weak(
               old, old + 1,
               std::memory_order_relaxed,  // success
               std::memory_order_relaxed   // failure
           )) {
        // retry with updated 'old'
        // On failure, 'old' is automatically updated with current value
        //
        // compare_exchange_weak vs strong:
        // - weak: Can fail spuriously (must use in loop), faster per attempt
        // - strong: Only fails if values don't match, slightly slower
        //
        // This is no longer a primitive - it's an algorithm
    }
}

void worker_cas() {
    for (int i = 0; i < 1'000'000; ++i) {
        bounded_increment(counter);
    }
}

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i)
        threads.emplace_back(worker_cas);
    for (auto& t : threads)
        t.join();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "=== CAS LOOP: Bounded Increment ===" << "\n";
    std::cout << "Final counter: " << counter << " (max 100)\n";
    std::cout << "Time taken: " << duration.count() << " ms\n";
    std::cout << "\n";
    std::cout << "✔ Custom atomic operation\n";
    std::cout << "✔ No built-in primitive for 'increment if < threshold'\n";
    std::cout << "❌ More complex than simple atomic\n";
    std::cout << "❌ This is an algorithm, not a primitive\n";
    std::cout << "\n";
    std::cout << "Note: Counter capped at 100, so most operations after that are no-ops.\n";
    std::cout << "This demonstrates when CAS is necessary: custom atomic logic.\n";
    
    return 0;
}
