#include <atomic>
#include <thread>
#include <vector>
#include <iostream>
#include <chrono>

// Lock-free stack: demonstrating CAS with pointers
// This is where CAS is actually necessary

struct Node {
    int value;
    Node* next;
    Node(int v) : value(v), next(nullptr) {}
};

class LockFreeStack {
private:
    std::atomic<Node*> head{nullptr};
    
public:
    void push(int value) {
        Node* new_node = new Node(value);
        Node* old_head = head.load(std::memory_order_relaxed);
        do {
            new_node->next = old_head;
            // Multiple steps:
            // 1. Read current head
            // 2. Link new node to it
            // 3. Try to update head atomically
            // CAS checks whether head is still unchanged
            // Retry if another thread modified it
        } while (!head.compare_exchange_weak(old_head, new_node,
                                             std::memory_order_release,
                                             std::memory_order_relaxed));
    }
    
    bool pop(int& value) {
        Node* old_head = head.load(std::memory_order_relaxed);
        while (old_head != nullptr) {
            Node* next = old_head->next;
            if (head.compare_exchange_weak(old_head, next,
                                           std::memory_order_acquire,
                                           std::memory_order_relaxed)) {
                value = old_head->value;
                delete old_head;
                return true;
            }
        }
        return false;
    }
    
    ~LockFreeStack() {
        int dummy;
        while (pop(dummy)) {}
    }
};

LockFreeStack stack;

void worker_push() {
    for (int i = 0; i < 100'000; ++i) {
        stack.push(i);
    }
}

void worker_pop() {
    int value;
    for (int i = 0; i < 100'000; ++i) {
        stack.pop(value);
    }
}

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    // 2 pushers, 2 poppers
    for (int i = 0; i < 2; ++i)
        threads.emplace_back(worker_push);
    for (int i = 0; i < 2; ++i)
        threads.emplace_back(worker_pop);
    
    for (auto& t : threads)
        t.join();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "=== LOCK-FREE STACK: Data Structure ===" << "\n";
    std::cout << "Time taken: " << duration.count() << " ms\n";
    std::cout << "\n";
    std::cout << "✔ No mutex needed\n";
    std::cout << "✔ System-wide progress guarantee\n";
    std::cout << "❌ Complex correctness reasoning\n";
    std::cout << "❌ Memory reclamation issues (simplified here with delete)\n";
    std::cout << "❌ ABA problem potential (not handled in this simple version)\n";
    std::cout << "❌ Requires understanding of memory ordering\n";
    std::cout << "\n";
    std::cout << "Note: This is lock-free construction, not just 'using atomics'.\n";
    std::cout << "Real production code needs hazard pointers or epoch-based reclamation.\n";
    
    return 0;
}
