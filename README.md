# C++ Synchronization: Mutex / Atomic / Lock-Free (CAS)

**C++ Series 05 | Mutex / Atomic / Lock-Free (CAS)** <br>
*Hands-on snippets for replacing mutex with atomic — and where that replacement stops making sense* <br>
https://medium.com/@yc-kuo/c-series-05-mutex-atomic-lock-free-cas-de7f6d3b7997

Hands-on code examples demonstrating when to use **mutex**, **atomic**, and **lock-free** synchronization in C++. This repository accompanies the Medium article with runnable benchmarks so you can see the real performance differences on your own hardware.

## 🎯 What This Repo Teaches

This is about **choosing the right tool** and understanding the real boundary lines between:
- **Mutex/Semaphore** - blocking, general-purpose invariants
- **Atomic** - CPU-level primitives, non-blocking for single locations
- **Lock-free (CAS loops)** - algorithmic, retry-based, complexity tax included

## 🚀 Quick Start

```bash
# Build everything
make

# Run comprehensive comparison
make run

# Run all 10 examples individually
make run-all
```

## 📊 Decision Tree (Put This in Your Brain First)

Use this in order. **Stop as soon as you hit "yes".**

```
Q1: Is there shared state?
 └─ No → No synchronization needed

Q2: Does the shared state span multiple variables / invariants?
 └─ Yes → Mutex (default)
 └─ No (single variable) →
      Q3: Does std::atomic already provide the operation?
        └─ Yes → std::atomic (no CAS loop)
        └─ No →
            Q4: Is this a proven hot path (profiled)?
              └─ No  → Mutex (simpler, safer)
              └─ Yes → CAS-based lock-free (with backoff + heavy testing)
```

**If you reach "lock-free" without a profiler, you probably made a mistake.**

## 📂 Files

| File | What It Demonstrates |
|------|---------------------|
| `01_mutex.cpp` | Mutex baseline - blocking under contention |
| `02_atomic.cpp` | Atomic counter - lock-free primitive |
| `03_atomic_broken.cpp` | Why atomics fail for multiple variables |
| `04_cas_bounded.cpp` | CAS loop for custom logic (bounded increment) |
| `05_lockfree_increment.cpp` | Reimplementing increment via CAS (slower!) |
| `06_lockfree_stack.cpp` | Lock-free data structure with CAS |
| `07_producer_consumer.cpp` | **Memory ordering: acquire/release vs relaxed** |
| `08_polling_vs_lockfree.cpp` | **Polling vs lock-free: they look similar but aren't** |
| `09_cas_with_backoff.cpp` | **CAS with exponential backoff to reduce contention** |
| `comparison.cpp` | Side-by-side benchmark of all three approaches |

## 🔬 Key Concepts

### 1. When to Use Mutex
- Multi-variable invariants
- Pointers, structure topology
- Correctness first

```cpp
std::lock_guard<std::mutex> lock(mtx);
++counter;  // Protected
```

**Nuance:** Uncontended mutexes can be quite fast. The pain shows up when the lock becomes contended and the critical section is tiny.

### 2. When to Use Atomic
- Single variable
- Operation already exists (fetch_add, store, load, exchange)
- Counters, flags, stats, reference counts

```cpp
counter.fetch_add(1, std::memory_order_relaxed);
```

**Why not just `++counter`?**
- `++counter` defaults to `memory_order_seq_cst` (strictest/slowest)
- `fetch_add` lets you specify memory order explicitly for better performance

**Why `memory_order_relaxed` works for counters:**
1. `fetch_add` is already atomic (no lost increments)
2. Incrementing is commutative (order doesn't matter)
3. We only care about final total after `join()`
4. `thread.join()` provides sufficient synchronization
5. Using `seq_cst` would add ~10-50% overhead for zero benefit

### 3. When to Use CAS Loops
- Custom atomic operations not provided by `std::atomic`
- Example: "increment only if below threshold"

```cpp
int old = value.load();
while (old < 100 && !value.compare_exchange_weak(old, old + 1)) {
    // Retry - 'old' updated on failure
}
```

**This is no longer a primitive - it's an algorithm.**

### 4. Memory Ordering

#### Quick Reference
- **relaxed**: No ordering guarantees, just atomicity (counters, stats)
- **acquire**: Load ensures subsequent reads see effects (reading flags/pointers)
- **release**: Store ensures previous writes visible before this (publishing data)
- **acq_rel**: Both acquire + release (read-modify-write)
- **seq_cst**: Strictest, global ordering (default for `++`, rarely needed)

#### Producer-Consumer Pattern
```cpp
int data = 0;  // Non-atomic
std::atomic<bool> ready{false};

// Producer
data = 42;                                      // 1. Setup
ready.store(true, std::memory_order_release);   // 2. Signal

// Consumer
while (!ready.load(std::memory_order_acquire)) {}  // 3. Wait
assert(data == 42);  // ✅ Guaranteed to see 42
```

**What goes wrong with `relaxed`:**
```cpp
// ❌ BROKEN
ready.store(true, std::memory_order_relaxed);   // No sync guarantee
while (!ready.load(std::memory_order_relaxed)) {}
assert(data == 42);  // ⚠️ MIGHT FAIL - might see data == 0
```

### 5. Polling vs Lock-Free

They **look similar** but are fundamentally different:

| Aspect | Polling | Lock-Free CAS |
|--------|---------|---------------|
| What it is | A waiting strategy | An algorithmic synchronization method |
| Operation | Repeated `load()` until ready | Retry CAS until update succeeds |
| Writes | Often none (read-only) | Yes (attempting updates) |
| Progress | None (could spin forever) | System-wide progress guaranteed |
| Under contention | Burns CPU (reads) | Burns CPU + cache thrashing (writes) |
| Best use | Short waits, handoff flags | Hot paths where locking kills perf |

**Why they feel the same:** Both involve spinning/retrying.

**Why they're different:** CAS failures are MUCH more expensive than loads because they fight for cache-line ownership.

### 6. CAS with Backoff

**Backoff** = adding deliberate delays between failed CAS retries to reduce cache line contention.

```cpp
int backoff = 1;
while (!value.compare_exchange_weak(old, new_val)) {
    for (int i = 0; i < backoff; ++i) {
        _mm_pause();  // x86 pause instruction
    }
    backoff = std::min(backoff * 2, 64);  // Exponential backoff
}
```

**When to use backoff:**
- ✓ High contention (many threads)
- ✓ NUMA systems (remote cache access is expensive)
- ✓ When throughput > latency matters

**When not to use backoff:**
- ✗ Ultra-low latency requirements
- ✗ Low contention (overhead not worth it)


## � Expected Results

On a typical 4-core system:

| Approach | Relative Speed | Use Case |
|----------|---------------|----------|
| Mutex | 1.0x (baseline) | Multi-variable protection |
| Atomic | **2-10x faster** | Single-variable updates |
| Lock-Free CAS | Similar to atomic | Only when profiling proves it's needed |

**Key Insight:** Lock-free CAS reimplementing `fetch_add` is often **slower** than the built-in atomic because you're reimplementing what the CPU already provides!

## 🧪 Cache Coherence Reality

### Mutex
- Lock variable ping-pongs mainly on lock/unlock
- Losers can park/sleep → reduces coherence pressure
- 👉 Good when work inside the lock is non-trivial

### Atomic
- Every `fetch_add` forces exclusive ownership of cache line
- Cache-line ping-pong
- 👉 Great for low contention; scales poorly under heavy load

### CAS Loop
- Loads + CAS attempts + failed retries
- Worst cache thrashing under contention
- 👉 Cache coherence traffic explodes

**Key insight:** Lock-free often relocates contention from the OS into the cache-coherence fabric. You didn't remove contention - you relocated it.

## ⚠️ Why Most Lock-Free Code Is Slower Than Mutexes

Under Summary (Systems-Engineer Takeaway)

| Approach | Use When |
|----------|----------|
| **Mutex** | Multi-variable invariants, pointers, structure topology. **Correctness first.** |
| **Atomic** | Counters, flags, stats, reference counts. Cheap, simple, predictable. |
| **Lock-free** | Only when profiling proves mutex contention or latency is the bottleneck. |

### Remember:
- Mutex protects invariants across multiple variables → **default choice**
- Atomics are perfect for single-variable shared state
- CAS loops appear only when you invent new atomic behavior
- Polling is a waiting strategy; lock-free is a progress property
- Replacing mutex → atomic is already a **meaningful win**
- Use lock-free with discretion (complexity tax is real)

## 🧪 Experiment Ideas

Try modifying the examples:

1. **Change thread count** - See how contention affects each method
2. **Change iteration count** - Observe scaling behavior
3. **Add work inside critical section** - See when mutex becomes better
4. **Try different memory orders** - Experiment with `memory_order_relaxed` vs `memory_order_seq_cst`
### Decision Tree

```
Q: Is there shared state?
└─ No → No synchronization needed

Q: Is it a single variable?
└─ No → Use Mutex
└─ Yes → Does std::atomic provide the operation?
    └─ Yes → Use std::atomic
    └─ No → Is this a proven hot path?
        └─ No → Use Mutex (simpler)
        └─ Yes → Use CAS / Lock-free
```
References

1. 王健偉 (2020)。C++ 新經典。北京：清華大學。
2. [C++11中的内存模型下篇 - C++11支持的几种内存模型](https://www.codedump.info/post/20191214-cxx11-memory-model-2/)
3. [C++中的memory order](https://edward852.github.io/post/cpp%E4%B8%AD%E7%9A%84memory_order/)
4. [std::memory_order - cppreference](https://en.cppreference.com/w/cpp/atomic/memory_order)
5. [C++ Object Initialization](https://www.cooldoger.com/2023/11/c-object-initialization.html)

## 📄 License

MIT License - Feel free to use for learning and teaching!

---

**Found this useful?** Read the full article on Medium for deeper explanations and systems-level insights. Run the benchmarks on your own hardware and see the numbers for yourself!
⚠️ Important Notes

- Lock-free is **NOT always faster** than mutex
- Under high contention, mutex can win due to sleeping vs spinning
- Cache coherence traffic is the real cost
- Profile before optimizing!

## 📖 Further Reading

- C++ Concurrency in Action (Anthony Williams)
- [C++ Reference: std::atomic](https://en.cppreference.com/w/cpp/atomic/atomic)
- [C++ Reference: std::mutex](https://en.cppreference.com/w/cpp/thread/mutex)

## 📄 License

MIT License - Feel free to use for learning and teaching!

---

**Remember:** Most production systems should:
- Use mutexes for structure
- Use atomics for counters/flags  
- Use lock-free only in proven hot paths with heavy testing
