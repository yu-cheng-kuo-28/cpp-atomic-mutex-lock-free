# Repository Refinement Summary

## What Was Updated

This repository has been refined to match the comprehensive Medium article "C++ Series 05: Mutex / Atomic / Lock-Free (CAS)".

## New Files Added (3)

1. **07_producer_consumer.cpp** - Demonstrates memory ordering (acquire/release vs relaxed)
   - Shows why acquire/release pairs are necessary for producer-consumer patterns
   - Demonstrates what goes wrong with memory_order_relaxed
   - Includes clear explanations of synchronizes-with relationships

2. **08_polling_vs_lockfree.cpp** - Clarifies the distinction between polling and lock-free
   - Polling = waiting strategy (read-only)
   - Lock-free = algorithmic synchronization (write attempts)
   - Shows why they look similar but have different costs

3. **09_cas_with_backoff.cpp** - Shows exponential backoff strategy
   - Demonstrates how backoff reduces cache line contention
   - Compares CAS with and without backoff
   - Explains when to use backoff and when not to

## Existing Files Updated (7)

1. **01_mutex.cpp**
   - Added nuance about uncontended mutexes being fast
   - Emphasized that pain shows up under contention with tiny critical sections

2. **02_atomic.cpp**
   - Added explanation of why fetch_add() instead of ++counter
   - Detailed explanation of why memory_order_relaxed works for counters
   - Emphasized this is using CPU primitives, not "doing lock-free programming"

3. **04_cas_bounded.cpp**
   - Added explanation of compare_exchange_weak vs strong
   - Emphasized "this is an algorithm, not a primitive"

4. **05_lockfree_increment.cpp**
   - Added note that this reimplements what fetch_add already does (slower!)

5. **06_lockfree_stack.cpp**
   - Better comments about CAS retry logic
   - Added notes about real production requirements (hazard pointers, etc.)

6. **comparison.cpp**
   - Updated output with decision tree
   - Better messaging about when to use each approach
   - Emphasized lock-free relocates contention, doesn't remove it

7. **Makefile**
   - Updated to include 3 new programs (now 10 total)
   - Updated help text
   - Updated run-all to show all 10 examples

8. **README.md** - Comprehensive rewrite
   - Added decision tree front and center
   - Added detailed memory ordering section with examples
   - Added polling vs lock-free explanation
   - Added CAS with backoff explanation
   - Added cache coherence reality section
   - Added "why lock-free is slower" section
   - Better structured with systems-engineer perspective
   - Added all references from the article

## Key Concepts Now Covered

### Memory Ordering
- Quick reference table (relaxed, acquire, release, acq_rel, seq_cst)
- Producer-consumer pattern with correct acquire/release
- What goes wrong with relaxed
- Practical examples with explanations

### Polling vs Lock-Free
- Clear distinction between waiting strategy vs algorithmic synchronization
- Side-by-side comparison table
- Cost model differences

### CAS with Backoff
- Exponential backoff implementation
- When to use and when not to use
- Performance comparison with and without backoff

### Decision Tree
- Placed prominently at the beginning
- Clear step-by-step guidance
- "Stop as soon as you hit yes" approach

### Cache Coherence Reality
- How each approach affects cache lines
- Why lock-free relocates contention
- Why lock-free is often slower under contention

## Build and Run

```bash
# Build all 10 examples
make

# Run comprehensive comparison
make run

# Run all examples individually
make run-all
```

## Article Alignment

The repository now fully aligns with the Medium article structure:
1. Decision tree first (mental model)
2. Basic examples (mutex, atomic, broken atomic)
3. CAS loops (bounded, increment)
4. Lock-free data structures
5. Memory ordering (producer-consumer)
6. Polling vs lock-free distinction
7. Performance optimizations (backoff)
8. Cache coherence reality
9. Why lock-free is often slower

All code examples from the article are now runnable with timing measurements and detailed explanations.
