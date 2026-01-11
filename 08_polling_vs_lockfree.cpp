#include <atomic>
#include <thread>
#include <iostream>
#include <chrono>
#include <vector>

// Demonstrating the difference between POLLING and LOCK-FREE

// ============ POLLING: Read-only waiting ============
std::atomic<bool> ready_flag{false};

void polling_consumer() {
    int spin_count = 0;
    while (!ready_flag.load(std::memory_order_acquire)) {
        // This is POLLING: repeatedly checking a condition
        // Read-only operation, no writes
        ++spin_count;
        std::this_thread::yield();
    }
    std::cout << "Polling consumer: spun " << spin_count << " times\n";
}

void polling_producer() {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ready_flag.store(true, std::memory_order_release);
}

// ============ LOCK-FREE: CAS with retries ============
std::atomic<int> counter{0};

void lockfree_worker() {
    for (int i = 0; i < 10000; ++i) {
        int old = counter.load(std::memory_order_relaxed);
        while (!counter.compare_exchange_weak(
            old, old + 1,
            std::memory_order_relaxed,
            std::memory_order_relaxed)) {
            // This LOOKS like polling, but it's different:
            // - Each retry involves a WRITE attempt (CAS)
            // - Fights for cache-line ownership
            // - Much more expensive under contention
        }
    }
}

int main() {
    std::cout << "╔══════════════════════════════════════════════════════╗\n";
    std::cout << "║  POLLING vs LOCK-FREE: They Look Similar But Aren't ║\n";
    std::cout << "╚══════════════════════════════════════════════════════╝\n\n";
    
    // Demonstration 1: Polling
    std::cout << "─── Demonstration 1: POLLING (read-only waiting) ───\n";
    ready_flag.store(false);
    
    auto start1 = std::chrono::high_resolution_clock::now();
    std::thread t1(polling_producer);
    std::thread t2(polling_consumer);
    t1.join();
    t2.join();
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);
    
    std::cout << "Time: " << duration1.count() << " ms\n\n";
    
    // Demonstration 2: Lock-free CAS
    std::cout << "─── Demonstration 2: LOCK-FREE CAS (write attempts) ───\n";
    counter.store(0);
    
    auto start2 = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(lockfree_worker);
    }
    for (auto& t : threads) {
        t.join();
    }
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2);
    
    std::cout << "Final counter: " << counter.load() << "\n";
    std::cout << "Time: " << duration2.count() << " ms\n\n";
    
    // Comparison table
    std::cout << "┌─────────────────┬──────────────────────────────┬─────────────────────────────────────┐\n";
    std::cout << "│ Aspect          │ POLLING                      │ LOCK-FREE CAS                       │\n";
    std::cout << "├─────────────────┼──────────────────────────────┼─────────────────────────────────────┤\n";
    std::cout << "│ What it is      │ A waiting strategy           │ An algorithmic synchronization      │\n";
    std::cout << "│ Operation       │ Repeated load() until ready  │ Retry CAS until update succeeds     │\n";
    std::cout << "│ Writes involved │ Often none (read-only)       │ Yes (attempting updates)            │\n";
    std::cout << "│ Progress        │ None (could spin forever)    │ System-wide progress guaranteed     │\n";
    std::cout << "│ Under contention│ Burns CPU (reads)            │ Burns CPU + cache thrashing (writes)│\n";
    std::cout << "│ Best use        │ Short waits, handoff flags   │ Hot paths where locking kills perf  │\n";
    std::cout << "└─────────────────┴──────────────────────────────┴─────────────────────────────────────┘\n\n";
    
    std::cout << "Why They Feel The Same:\n";
    std::cout << "• Both involve spinning/retrying\n";
    std::cout << "• Both can burn CPU under long waits\n\n";
    
    std::cout << "Why They're Different:\n";
    std::cout << "• Polling = read-only, waiting for condition\n";
    std::cout << "• Lock-free CAS = write attempts, guarantees progress\n";
    std::cout << "• CAS failures are MUCH more expensive (cache-line ownership fights)\n";
    
    return 0;
}
