# Lock-Free: Primitive vs Algorithm

## The Terminology Confusion

"Lock-free" means different things at different abstraction levels, which causes confusion.

## Two Distinct Concepts

### 1️⃣ Lock-Free Primitive (Hardware Level)

**Definition:** A single atomic operation implemented directly by the CPU hardware without using locks.

**Examples:**
```cpp
std::atomic<int> counter{0};
counter.fetch_add(1);        // ✅ Lock-free primitive
counter.store(42);           // ✅ Lock-free primitive
counter.load();              // ✅ Lock-free primitive
counter.exchange(10);        // ✅ Lock-free primitive
```

**Characteristics:**
- Single CPU instruction
- No mutex/lock involved
- Hardware-level atomicity
- Fixed operation (add, store, load, exchange, etc.)
- O(1) guaranteed
- You're **using** lock-free primitives

**You can check if atomic types are lock-free:**
```cpp
std::atomic<int> x;
if (x.is_lock_free()) {
    // True on most modern architectures
}
```

---

### 2️⃣ Lock-Free Algorithm (Algorithm Level)

**Definition:** An algorithm that guarantees system-wide progress even if individual threads are delayed. Built using CAS loops and atomic operations.

**Examples:**
```cpp
// Lock-free bounded increment
void bounded_increment(std::atomic<int>& v) {
    int old = v.load();
    while (old < 100 && !v.compare_exchange_weak(old, old + 1)) {
        // Retry - lock-free algorithm
    }
}

// Lock-free stack
void push(Node* node) {
    Node* old_head = head.load();
    do {
        node->next = old_head;
    } while (!head.compare_exchange_weak(old_head, node));
}
```

**Characteristics:**
- Multiple steps with retry loops
- Uses CAS (compare-and-swap) operations
- Progress guarantee: at least one thread makes progress
- Complex correctness reasoning required
- May retry many times under contention
- You're **implementing** lock-free algorithms

---

## Visual Comparison

```
┌─────────────────────────────────────────────────────────────────┐
│                    LOCK-FREE SPECTRUM                           │
├─────────────────────────┬───────────────────────────────────────┤
│ Lock-Free Primitive     │ Lock-Free Algorithm                   │
├─────────────────────────┼───────────────────────────────────────┤
│ Hardware level          │ Algorithm level                       │
│ Single atomic operation │ Multiple operations + retry logic     │
│ fetch_add(), store()    │ CAS loops, complex data structures    │
│ Fixed operations        │ Custom atomic behaviors               │
│ Simple to use           │ Complex to implement correctly        │
│ Fast (1 instruction)    │ Can be slow (many retries)            │
│ Use std::atomic         │ Build with compare_exchange_weak      │
└─────────────────────────┴───────────────────────────────────────┘
```

---

## Key Insight

**Both are "lock-free" but answering different questions:**

| Question | Lock-Free Primitive | Lock-Free Algorithm |
|----------|---------------------|---------------------|
| "Does it use locks?" | No (hardware atomic) | No (CAS-based) |
| "What level?" | Hardware/CPU | Algorithm/Software |
| "Complexity?" | Use built-in operations | Implement complex logic |
| "When to use?" | Single-variable updates | Custom atomic operations |

---

## Common Confusion Examples

### ❌ Confusing Statement:
"Atomics are lock-free, so I should use CAS loops everywhere."

### ✅ Correct Understanding:
- Atomic **primitives** like `fetch_add()` are lock-free at the hardware level
- You DON'T need CAS loops if a primitive already exists
- CAS loops are for **building** lock-free **algorithms** when no primitive exists

---

### ❌ Confusing Statement:
"Lock-free is always faster than mutexes."

### ✅ Correct Understanding:
- Lock-free **primitives** (fetch_add) are usually faster than mutexes for simple operations
- Lock-free **algorithms** (CAS loops) can be SLOWER than mutexes under contention
- It depends on the workload and contention level

---

## When Do You Need Each?

### Use Lock-Free Primitives (fetch_add, store, load)
```cpp
// ✅ Perfect use case
std::atomic<int> request_count{0};
request_count.fetch_add(1);  // Just increment

std::atomic<bool> shutdown{false};
shutdown.store(true);  // Just set flag
```

**When:** The operation already exists as a built-in atomic.

---

### Use Lock-Free Algorithms (CAS loops)
```cpp
// ✅ No primitive exists for "increment if < 100"
void bounded_increment(std::atomic<int>& v) {
    int old = v.load();
    while (old < 100 && !v.compare_exchange_weak(old, old + 1)) {
        // Custom logic requires CAS loop
    }
}
```

**When:** 
- Building custom atomic operations
- Implementing lock-free data structures
- Proven hot path (profiled!)
- You understand the complexity tax

---

## Progress Guarantees

Both primitives and algorithms can be "lock-free" in the formal sense:

| Property | Definition |
|----------|------------|
| **Lock-free** | System-wide progress: at least one thread makes progress |
| **Wait-free** | Per-thread progress: every thread makes progress in bounded steps |

Most atomic primitives are lock-free or wait-free.
Most lock-free algorithms are only lock-free (not wait-free).

---

## Summary Table

| Aspect | Lock-Free Primitive | Lock-Free Algorithm |
|--------|---------------------|---------------------|
| **Abstraction** | Hardware | Software/Algorithm |
| **Example** | `fetch_add(1)` | `while (!CAS()) {}` |
| **Complexity** | Simple (use it) | Complex (build it) |
| **Performance** | Fast (1 instruction) | Varies (many retries) |
| **Correctness** | Guaranteed by hardware | Your responsibility |
| **When** | Operation exists | Custom logic needed |
| **Level** | Using lock-free | Implementing lock-free |

---

## The Bottom Line

When people say "use atomics, they're lock-free":
- They usually mean lock-free **primitives** (hardware level)
- They DON'T mean you should implement lock-free **algorithms** (software level)

When people say "lock-free programming":
- They usually mean implementing lock-free **algorithms** with CAS loops
- This is hard, error-prone, and only worth it in proven hot paths

**Both are "lock-free", but at different levels of abstraction.**
