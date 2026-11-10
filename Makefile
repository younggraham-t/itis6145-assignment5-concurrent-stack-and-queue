CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread -fopenmp
BUILD_DIR = build
TARGET = $(BUILD_DIR)/main

SRCS = main.cpp stack.cpp stack_factory.cpp queue.cpp queue_factory.cpp
OBJS = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(SRCS))

# Optional flags for single 'make run'
type ?=
TYPE ?= $(type)
impl ?=
IMPL ?= $(impl)
workload ?=
WORKLOAD ?= $(workload)
threads ?=
THREADS_FLAG ?= $(threads)
ops ?=
OPS_FLAG ?= $(ops)

# ==============================================================================
# Assignment 5 Experimental Benchmark Sweep Configurations
# ==============================================================================
# C.1 Thread scaling: 1, 2, 4, 8, 16, 32
THREADS ?= 1 2 4 8 16 32

# Operations per test (default 1,000,000 as per PDF)
OPS ?= 1000000

# Stack configurations:
# Impls: mutex (A0 baseline), treiber (A1), elimination (A2)
# Workloads: push_heavy (80/20), balanced (50/50), pop_heavy (20/80)
STACK_IMPLS ?= mutex treiber elimination
STACK_WORKLOADS ?= push_heavy balanced pop_heavy

# Queue configurations:
# Impls: mutex (B0 baseline), ms (B1 Michael-Scott)
# Workloads: mp_mc (MPMC), pc_split (Producer-Consumer split)
QUEUE_IMPLS ?= mutex ms
QUEUE_WORKLOADS ?= mp_mc pc_split

# Output CSV file for benchmark data (throughput and runtime analysis)
CSV_OUT ?= benchmark_results.csv

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: $(TARGET)
	@CMD="./$(TARGET)"; \
	if [ -n "$(TYPE)" ]; then CMD="$$CMD --type $(TYPE)"; fi; \
	if [ -n "$(IMPL)" ]; then CMD="$$CMD --impl $(IMPL)"; fi; \
	if [ -n "$(WORKLOAD)" ]; then CMD="$$CMD --workload $(WORKLOAD)"; fi; \
	if [ -n "$(THREADS_FLAG)" ]; then CMD="$$CMD --threads $(THREADS_FLAG)"; fi; \
	if [ -n "$(OPS_FLAG)" ]; then CMD="$$CMD --ops $(OPS_FLAG)"; fi; \
	$$CMD

# Initialize CSV output file with header
init-csv:
	@echo "type,impl,workload,threads,ops,time_ms,throughput_ops_sec" > $(CSV_OUT)
	@echo "Initialized benchmark results file: $(CSV_OUT)"

# Sweep all stack configurations
sweep-stack: $(TARGET)
	@echo "=========================================================="
	@echo " Starting Stack Sweep (Impls: $(STACK_IMPLS))"
	@echo " Ops: $(OPS) | Threads: $(THREADS)"
	@echo "=========================================================="
	@for impl in $(STACK_IMPLS); do \
		for wl in $(STACK_WORKLOADS); do \
			for t in $(THREADS); do \
				printf "[RUN] Stack | %-11s | %-10s | %2d threads | %d ops ... " "$$impl" "$$wl" "$$t" "$(OPS)"; \
				OUTPUT=$$(./$(TARGET) --type stack --impl $$impl --workload $$wl --threads $$t --ops $(OPS) 2>&1); \
				TIME=$$(echo "$$OUTPUT" | grep "Time Elapsed" | awk '{print $$NF}'); \
				if [ -n "$$TIME" ]; then \
					THROUGHPUT=$$(awk -v ops="$(OPS)" -v t="$$TIME" 'BEGIN { if (t > 0) printf "%.2f", (ops * 1000.0) / t; else print "0" }'); \
					echo "$$TIME ms ($$THROUGHPUT ops/sec)"; \
					echo "stack,$$impl,$$wl,$$t,$(OPS),$$TIME,$$THROUGHPUT" >> $(CSV_OUT); \
				else \
					echo "FAILED"; \
					echo "stack,$$impl,$$wl,$$t,$(OPS),FAILED,0" >> $(CSV_OUT); \
				fi; \
			done; \
		done; \
	done

# Sweep all queue configurations
sweep-queue: $(TARGET)
	@echo "=========================================================="
	@echo " Starting Queue Sweep (Impls: $(QUEUE_IMPLS))"
	@echo " Ops: $(OPS) | Threads: $(THREADS)"
	@echo "=========================================================="
	@for impl in $(QUEUE_IMPLS); do \
		for wl in $(QUEUE_WORKLOADS); do \
			for t in $(THREADS); do \
				printf "[RUN] Queue | %-11s | %-10s | %2d threads | %d ops ... " "$$impl" "$$wl" "$$t" "$(OPS)"; \
				OUTPUT=$$(./$(TARGET) --type queue --impl $$impl --workload $$wl --threads $$t --ops $(OPS) 2>&1); \
				TIME=$$(echo "$$OUTPUT" | grep "Time Elapsed" | awk '{print $$NF}'); \
				if [ -n "$$TIME" ]; then \
					THROUGHPUT=$$(awk -v ops="$(OPS)" -v t="$$TIME" 'BEGIN { if (t > 0) printf "%.2f", (ops * 1000.0) / t; else print "0" }'); \
					echo "$$TIME ms ($$THROUGHPUT ops/sec)"; \
					echo "queue,$$impl,$$wl,$$t,$(OPS),$$TIME,$$THROUGHPUT" >> $(CSV_OUT); \
				else \
					echo "FAILED"; \
					echo "queue,$$impl,$$wl,$$t,$(OPS),FAILED,0" >> $(CSV_OUT); \
				fi; \
			done; \
		done; \
	done

# Full benchmark sweep across all assignment configurations
sweep: $(TARGET) init-csv sweep-stack sweep-queue
	@echo "=========================================================="
	@echo " Full Benchmark Sweep Complete!"
	@echo " Results saved to: $(CSV_OUT)"
	@echo "=========================================================="

# Quick sweep (10,000 ops, subset of threads) for quick verification
sweep-quick:
	$(MAKE) sweep OPS=10000 THREADS="1 4 8" CSV_OUT="quick_results.csv"

clean:
	rm -rf $(BUILD_DIR) $(CSV_OUT) quick_results.csv

.PHONY: all clean run sweep sweep-stack sweep-queue sweep-quick init-csv
