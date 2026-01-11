#include <mutex>
#include <thread>
#include <vector>
#include <iostream>
#include <chrono>

std::mutex mtx;
int counter = 0;

void worker_mutex() {
    for (int i = 0; i < 1'000'000; ++i) {
        std::lock_guard<std::mutex> lock(mtx);
        ++counter;
        // Only one thread can enter here at a time
        // Other threads are blocked (possibly context-switched)
    }
}

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i)
        threads.emplace_back(worker_mutex);
    for (auto& t : threads)
        t.join();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "=== MUTEX (Blocking) ===" << "\n";
    std::cout << "Final counter: " << counter << "\n";
    std::cout << "Time taken: " << duration.count() << " ms\n";
    std::cout << "\n";
    std::cout << "✅ Correct\n";
    std::cout << "❌ Blocking under contention\n";
    std::cout << "❌ Can involve OS scheduling under contention\n";
    std::cout << "❌ Overkill for a single integer in a hot path\n";
    std::cout << "\n";
    std::cout << "Nuance: Uncontended mutexes can be quite fast.\n";
    std::cout << "The pain shows up when the lock becomes contended\n";
    std::cout << "and the critical section is tiny.\n";
    
    return 0;
}
