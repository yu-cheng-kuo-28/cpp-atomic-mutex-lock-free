#include <atomic>
#include <thread>
#include <iostream>
#include <chrono>

// ❌ This demonstrates why atomics don't protect invariants across multiple variables

std::atomic<int> x{0};
std::atomic<int> y{0};

void writer_thread() {
    for (int i = 0; i < 100'000; ++i) {
        x.store(1, std::memory_order_relaxed);
        y.store(1, std::memory_order_relaxed);
        // No atomicity across (x, y) as a group
        x.store(0, std::memory_order_relaxed);
        y.store(0, std::memory_order_relaxed);
    }
}

void reader_thread(int& inconsistent_count) {
    for (int i = 0; i < 100'000; ++i) {
        int val_x = x.load(std::memory_order_relaxed);
        int val_y = y.load(std::memory_order_relaxed);
        
        // Check for inconsistent state: one is 1, the other is 0
        if ((val_x == 1 && val_y == 0) || (val_x == 0 && val_y == 1)) {
            ++inconsistent_count;
        }
    }
}

int main() {
    std::cout << "=== ATOMIC BROKEN: Multiple Variables ===" << "\n";
    std::cout << "Demonstrating that atomics don't protect invariants across variables\n\n";
    
    int inconsistent_observations = 0;
    
    std::thread writer(writer_thread);
    std::thread reader(reader_thread, std::ref(inconsistent_observations));
    
    writer.join();
    reader.join();
    
    std::cout << "Inconsistent observations: " << inconsistent_observations << "\n";
    std::cout << "\n❌ Another thread may observe: y == 1, x == 0\n";
    std::cout << "❌ If multiple variables must change together, you want a mutex.\n";
    
    return 0;
}
