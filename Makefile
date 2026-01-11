# Makefile for C++ Mutex/Atomic/Lock-Free Demo
# Compatible with both g++ and MSVC (via nmake)

CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -pthread
TARGET_SUFFIX = 

# Detect Windows
ifeq ($(OS),Windows_NT)
    TARGET_SUFFIX = .exe
    # If using MSVC, uncomment below:
    # CXX = cl
    # CXXFLAGS = /std:c++17 /O2 /EHsc
endif

TARGETS = 01_mutex$(TARGET_SUFFIX) \
          02_atomic$(TARGET_SUFFIX) \
          03_atomic_broken$(TARGET_SUFFIX) \
          04_cas_bounded$(TARGET_SUFFIX) \
          05_lockfree_increment$(TARGET_SUFFIX) \
          06_lockfree_stack$(TARGET_SUFFIX) \
          07_producer_consumer$(TARGET_SUFFIX) \
          08_polling_vs_lockfree$(TARGET_SUFFIX) \
          09_cas_with_backoff$(TARGET_SUFFIX) \
          comparison$(TARGET_SUFFIX)

.PHONY: all clean run run-all help

all: $(TARGETS)
	@echo ""
	@echo "✅ All programs built successfully!"
	@echo "Run 'make run' to execute all examples"

01_mutex$(TARGET_SUFFIX): 01_mutex.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

02_atomic$(TARGET_SUFFIX): 02_atomic.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

03_atomic_broken$(TARGET_SUFFIX): 03_atomic_broken.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

04_cas_bounded$(TARGET_SUFFIX): 04_cas_bounded.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

05_lockfree_increment$(TARGET_SUFFIX): 05_lockfree_increment.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

06_lockfree_stack$(TARGET_SUFFIX): 06_lockfree_stack.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

07_producer_consumer$(TARGET_SUFFIX): 07_producer_consumer.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

08_polling_vs_lockfree$(TARGET_SUFFIX): 08_polling_vs_lockfree.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

09_cas_with_backoff$(TARGET_SUFFIX): 09_cas_with_backoff.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

comparison$(TARGET_SUFFIX): comparison.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

# Run the comprehensive comparison
run: comparison$(TARGET_SUFFIX)
	@echo ""
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	@echo " Running Comprehensive Comparison"
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	./comparison$(TARGET_SUFFIX)

# Run all examples individually
run-all: all
	@echo ""
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	@echo " 1/10: Mutex (Baseline)"
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	./01_mutex$(TARGET_SUFFIX)
	@echo ""
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	@echo " 2/10: Atomic"
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	./02_atomic$(TARGET_SUFFIX)
	@echo ""
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	@echo " 3/10: Atomic Broken (Multiple Variables)"
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	./03_atomic_broken$(TARGET_SUFFIX)
	@echo ""
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	@echo " 4/10: CAS Bounded Increment"
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	./04_cas_bounded$(TARGET_SUFFIX)
	@echo ""
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	@echo " 5/10: Lock-Free Increment"
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	./05_lockfree_increment$(TARGET_SUFFIX)
	@echo ""
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	@echo " 6/10: Lock-Free Stack"
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	./06_lockfree_stack$(TARGET_SUFFIX)
	@echo ""
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	@echo " 7/10: Producer-Consumer (Memory Ordering)"
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	./07_producer_consumer$(TARGET_SUFFIX)
	@echo ""
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	@echo " 8/10: Polling vs Lock-Free"
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	./08_polling_vs_lockfree$(TARGET_SUFFIX)
	@echo ""
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	@echo " 9/10: CAS with Backoff"
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	./09_cas_with_backoff$(TARGET_SUFFIX)
	@echo ""
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	@echo " 10/10: Comprehensive Comparison"
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	./comparison$(TARGET_SUFFIX)

clean:
	rm -f $(TARGETS) *.o

help:
	@echo "Available targets:"
	@echo "  make          - Build all programs"
	@echo "  make run      - Run comprehensive comparison"
	@echo "  make run-all  - Run all examples individually"
	@echo "  make clean    - Remove all built files"
	@echo "  make help     - Show this help message"
	@echo ""
	@echo "Individual programs:"
	@echo "  01_mutex               - Mutex baseline"
	@echo "  02_atomic              - Atomic counter"
	@echo "  03_atomic_broken       - Shows atomic limitations"
	@echo "  04_cas_bounded         - CAS with bounded increment"
	@echo "  05_lockfree_increment  - Lock-free CAS loop"
	@echo "  06_lockfree_stack      - Lock-free data structure"
	@echo "  07_producer_consumer   - Memory ordering (acquire/release)"
	@echo "  08_polling_vs_lockfree - Polling vs lock-free distinction"
	@echo "  09_cas_with_backoff    - CAS with exponential backoff"
	@echo "  comparison             - Side-by-side comparison"
