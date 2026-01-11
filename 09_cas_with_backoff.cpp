#include <atomic>
#include <thread>
#include <vector>
#include <iostream>
#include <chrono>
#include <iomanip>

// Demonstrating CAS with and without backoff
// Backoff = adding deliberate delays between failed CAS retries to reduce cache line contention

std::atomic<int> counter_no_backoff{0};
std::atomic<int> counter_with_backoff{0};
std::atomic<int> failed_cas_count_no_backoff{0};
std::atomic<int> failed_cas_count_with_backoff{0};

const int ITERATIONS = 100000;
const int NUM_THREADS = 4;

// CAS without backoff (naive)
void worker_no_backoff() {
    for (int i = 0; i < ITERATIONS; ++i) {
        int old = counter_no_backoff.load(std::memory_order_relaxed);
        while (!counter_no_backoff.compare_exchange_weak(
            old, old + 1,
            std::memory_order_relaxed,
            std::memory_order_relaxed)) {
            // Immediate retry - hammers the cache line
            failed_cas_count_no_backoff.fetch_add(1, std::memory_order_relaxed);
        }
    }
}

// CAS with exponential backoff
void worker_with_backoff() {
    for (int i = 0; i < ITERATIONS; ++i) {
        int old = counter_with_backoff.load(std::memory_order_relaxed);
        int backoff = 1;
        while (!counter_with_backoff.compare_exchange_weak(
            old, old + 1,
            std::memory_order_relaxed,
            std::memory_order_relaxed)) {
            // Backoff strategy: pause briefly, then increase delay
            for (int j = 0; j < backoff; ++j) {
                #if defined(_MSC_VER)
                    _mm_pause();  // x86 intrinsic
                #elif defined(__GNUC__) || defined(__clang__)
                    __builtin_ia32_pause();  // GCC/Clang intrinsic
                #else
                    std::this_thread::yield();
                #endif
            }
            backoff = std::min(backoff * 2, 64);  // Exponential backoff, cap at 64
            failed_cas_count_with_backoff.fetch_add(1, std::memory_order_relaxed);
        }
    }
}

int main() {
    std::cout << "╔════════════════════════════════════════════════════╗\n";
    std::cout << "║  CAS with Backoff: Reducing Cache Line Contention  ║\n";
    std::cout << "╚════════════════════════════════════════════════════╝\n\n";
    
    std::cout << "Configuration:\n";
    std::cout << "  Threads: " << NUM_THREADS << "\n";
    std::cout << "  Iterations per thread: " << ITERATIONS << "\n\n";
    
    // Test 1: No backoff
    std::cout << "Running CAS WITHOUT backoff...\n";
    counter_no_backoff.store(0);
    failed_cas_count_no_backoff.store(0);
    
    auto start1 = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> threads1;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads1.emplace_back(worker_no_backoff);
    }
    for (auto& t : threads1) {
        t.join();
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);
    
    // Test 2: With backoff
    std::cout << "Running CAS WITH backoff...\n\n";
    counter_with_backoff.store(0);
    failed_cas_count_with_backoff.store(0);
    
    auto start2 = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> threads2;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads2.emplace_back(worker_with_backoff);
    }
    for (auto& t : threads2) {
        t.join();
    }
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2);
    
    // Results
    std::cout << "┌────────────────────┬─────────────┬──────────────┬─────────────────┐\n";
    std::cout << "│ Strategy           │ Time (ms)   │ Final Count  │ Failed CAS      │\n";
    std::cout << "├────────────────────┼─────────────┼──────────────┼─────────────────┤\n";
    std::cout << "│ No Backoff         │ " 
              << std::setw(11) << duration1.count() << " │ "
              << std::setw(12) << counter_no_backoff.load() << " │ "
              << std::setw(15) << failed_cas_count_no_backoff.load() << " │\n";
    std::cout << "│ With Backoff       │ " 
              << std::setw(11) << duration2.count() << " │ "
              << std::setw(12) << counter_with_backoff.load() << " │ "
              << std::setw(15) << failed_cas_count_with_backoff.load() << " │\n";
    std::cout << "└────────────────────┴─────────────┴──────────────┴─────────────────┘\n\n";
    
    std::cout << "Key Observations:\n";
    std::cout << "• Without backoff: Immediate retries hammer the cache line\n";
    std::cout << "  → More contention, more coherence traffic\n";
    std::cout << "  → Can be faster on low thread counts but scales poorly\n\n";
    
    std::cout << "• With backoff: Brief pauses reduce cache line ping-pong\n";
    std::cout << "  → Less coherence traffic on the memory bus\n";
    std::cout << "  → Better scaling with more threads\n";
    std::cout << "  → Trade: slight latency for better throughput\n\n";
    
    std::cout << "When to use backoff:\n";
    std::cout << "✓ High contention scenarios (many threads)\n";
    std::cout << "✓ NUMA systems (remote cache line access is expensive)\n";
    std::cout << "✓ When throughput > latency matters\n\n";
    
    std::cout << "When not to use backoff:\n";
    std::cout << "✗ Ultra-low latency requirements\n";
    std::cout << "✗ Low contention (overhead not worth it)\n";
    std::cout << "✗ When a built-in atomic already exists (use that instead!)\n";
    
    return 0;
}
