#include <mutex>
#include <atomic>
#include <thread>
#include <vector>
#include <iostream>
#include <chrono>
#include <iomanip>

// All three approaches in one file for direct comparison

const int ITERATIONS = 1'000'000;
const int NUM_THREADS = 4;

// ============ MUTEX VERSION ============
std::mutex mtx;
int counter_mutex = 0;

void worker_mutex() {
    for (int i = 0; i < ITERATIONS; ++i) {
        std::lock_guard<std::mutex> lock(mtx);
        ++counter_mutex;
    }
}

long long benchmark_mutex() {
    counter_mutex = 0;
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i)
        threads.emplace_back(worker_mutex);
    for (auto& t : threads)
        t.join();
    
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

// ============ ATOMIC VERSION ============
std::atomic<int> counter_atomic{0};

void worker_atomic() {
    for (int i = 0; i < ITERATIONS; ++i) {
        counter_atomic.fetch_add(1, std::memory_order_relaxed);
    }
}

long long benchmark_atomic() {
    counter_atomic = 0;
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i)
        threads.emplace_back(worker_atomic);
    for (auto& t : threads)
        t.join();
    
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

// ============ LOCK-FREE CAS VERSION ============
std::atomic<int> counter_lockfree{0};

void lock_free_increment() {
    int old = counter_lockfree.load(std::memory_order_relaxed);
    while (!counter_lockfree.compare_exchange_weak(
        old, old + 1,
        std::memory_order_relaxed)) {
    }
}

void worker_lockfree() {
    for (int i = 0; i < ITERATIONS; ++i) {
        lock_free_increment();
    }
}

long long benchmark_lockfree() {
    counter_lockfree = 0;
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i)
        threads.emplace_back(worker_lockfree);
    for (auto& t : threads)
        t.join();
    
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

// ============ MAIN ============
int main() {
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  C++ Synchronization: Mutex vs Atomic vs Lock-Free         ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n\n";
    
    std::cout << "Configuration:\n";
    std::cout << "  Threads: " << NUM_THREADS << "\n";
    std::cout << "  Iterations per thread: " << ITERATIONS << "\n";
    std::cout << "  Total operations: " << (ITERATIONS * NUM_THREADS) << "\n\n";
    
    std::cout << "Running benchmarks...\n\n";
    
    // Benchmark all three
    long long time_mutex = benchmark_mutex();
    long long time_atomic = benchmark_atomic();
    long long time_lockfree = benchmark_lockfree();
    
    // Results
    std::cout << "┌─────────────────────┬──────────────┬────────────┬────────────┐\n";
    std::cout << "│ Approach            │ Time (ms)    │ Final      │ Speedup    │\n";
    std::cout << "├─────────────────────┼──────────────┼────────────┼────────────┤\n";
    
    std::cout << "│ Mutex (blocking)    │ " 
              << std::setw(12) << time_mutex << " │ "
              << std::setw(10) << counter_mutex << " │ "
              << std::setw(10) << "1.00x" << " │\n";
    
    std::cout << "│ Atomic (primitive)  │ " 
              << std::setw(12) << time_atomic << " │ "
              << std::setw(10) << counter_atomic.load() << " │"
              << std::setw(10) << std::fixed << std::setprecision(2) 
              << ((double)time_mutex / time_atomic) << "x" << " │\n";
    
    std::cout << "│ CAS Loop (algorithm)│ "
              << std::setw(12) << time_lockfree << " │ "
              << std::setw(10) << counter_lockfree.load() << " │"
              << std::setw(10) << std::fixed << std::setprecision(2)
              << ((double)time_mutex / time_lockfree) << "x" << " │\n";
    
    std::cout << "└─────────────────────┴──────────────┴────────────┴────────────┘\n\n";
    
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "Lock-Free Terminology Clarification:\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "• Atomic primitive  = Lock-free at HARDWARE level\n";
    std::cout << "                      (single CPU instruction, no mutex)\n";
    std::cout << "• CAS loop/algorithm = Lock-free at ALGORITHM level\n";
    std::cout << "                       (retry-based, progress guarantee)\n";
    std::cout << "\nBoth are \"lock-free\" but at different abstraction levels!\n";
    std::cout << "\nKey Observations:\n";
    std::cout << "• Atomic is typically 2-10x faster than Mutex for simple counters\n";
    std::cout << "• CAS loop is SLOWER than built-in atomic primitive\n";
    std::cout << "  (we're reimplementing what fetch_add already does!)\n";
    std::cout << "• Mutex includes OS overhead under contention (context switches, blocking)\n";
    std::cout << "• Atomic uses CPU-level instructions (no blocking)\n";
    std::cout << "• Performance varies by contention level and CPU architecture\n\n";
    
    std::cout << "Decision Tree:\n";
    std::cout << "  Q: Does shared state span multiple variables?\n";
    std::cout << "     Yes → Use Mutex\n";
    std::cout << "     No (single variable) →\n";
    std::cout << "        Q: Does std::atomic provide the operation?\n";
    std::cout << "           Yes → Use std::atomic\n";
    std::cout << "           No →\n";
    std::cout << "              Q: Is this a proven hot path (profiled)?\n";
    std::cout << "                 No  → Use Mutex (simpler, safer)\n";
    std::cout << "                 Yes → CAS-based lock-free (with backoff + testing)\n\n";
    
    std::cout << "Remember:\n";
    std::cout << "• Mutex protects multi-variable invariants \u2192 default choice\n";
    std::cout << "• Atomics are perfect for single-variable state\n";
    std::cout << "• Lock-free is a latency optimization, not a throughput guarantee\n";
    std::cout << "• Under contention, lock-free often relocates contention from OS\n";
    std::cout << "  into the cache-coherence fabric (not always better!)\n";
    
    return 0;
}
