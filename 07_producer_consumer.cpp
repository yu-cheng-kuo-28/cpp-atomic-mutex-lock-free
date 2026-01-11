#include <atomic>
#include <thread>
#include <iostream>
#include <chrono>
#include <cassert>

// Producer-Consumer Example: Why acquire/release matters
// This demonstrates the synchronizes-with relationship

int data = 0;  // Non-atomic shared data
std::atomic<bool> ready{false};

// Producer thread
void producer() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    data = 42;                                      // 1. Setup work
    ready.store(true, std::memory_order_release);   // 2. Signal "done"
    // release ensures all writes BEFORE this are visible to threads that acquire
}

// Consumer thread with acquire
void consumer_correct() {
    while (!ready.load(std::memory_order_acquire)) {  // 3. Wait for signal
        // acquire ensures we see all writes that happened BEFORE the release
        std::this_thread::yield();
    }
    // 4. Use the data (guaranteed to see 42)
    std::cout << "Consumer (acquire): data = " << data << " ✅\n";
    assert(data == 42);
}

// Consumer thread with relaxed (BROKEN)
void consumer_broken() {
    while (!ready.load(std::memory_order_relaxed)) {
        std::this_thread::yield();
    }
    // ⚠️ MIGHT see stale data! No synchronization guarantee
    std::cout << "Consumer (relaxed): data = " << data << " ⚠️ (might be 0 or 42)\n";
    // In practice, often works due to timing, but NOT guaranteed
}

int main() {
    std::cout << "=== PRODUCER-CONSUMER: Memory Ordering ===" << "\n\n";
    
    // Test 1: Correct usage (acquire/release pair)
    std::cout << "Test 1: acquire/release pair\n";
    data = 0;
    ready.store(false);
    
    std::thread t1(producer);
    std::thread t2(consumer_correct);
    t1.join();
    t2.join();
    
    std::cout << "\nTest 2: relaxed (broken - no guarantee)\n";
    data = 0;
    ready.store(false);
    
    std::thread t3(producer);
    std::thread t4(consumer_broken);
    t3.join();
    t4.join();
    
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "Key Points:\n";
    std::cout << "• memory_order_acquire on load ensures:\n";
    std::cout << "  - All memory operations AFTER this load cannot be reordered BEFORE it\n";
    std::cout << "  - You see all writes that happened BEFORE the matching release store\n\n";
    std::cout << "• memory_order_release on store ensures:\n";
    std::cout << "  - All memory operations BEFORE this store cannot be reordered AFTER it\n";
    std::cout << "  - Subsequent acquire loads will see these writes\n\n";
    std::cout << "• memory_order_relaxed provides:\n";
    std::cout << "  - Only atomicity of the operation itself\n";
    std::cout << "  - NO ordering guarantees with other memory operations\n";
    std::cout << "  - Can lead to observing inconsistent state\n";
    
    return 0;
}
