# Makefile for Producer-Consumer Problem
# Compile with: make all
# Clean with: make clean
# Run with: make run

CC = gcc
CFLAGS = -Wall -Wextra -pthread
LDFLAGS = -lrt -pthread
TARGETS = producer consumer

# Default target
all: $(TARGETS)

# Compile producer
producer: producer.c
	$(CC) $(CFLAGS) producer.c $(LDFLAGS) -o producer
	@echo "✓ Producer compiled successfully"

# Compile consumer
consumer: consumer.c
	$(CC) $(CFLAGS) consumer.c $(LDFLAGS) -o consumer
	@echo "✓ Consumer compiled successfully"

# Run both programs
run: all
	@echo "Starting Producer-Consumer programs..."
	@echo "========================================="
	./producer & ./consumer &
	@echo "Programs started in background"
	@echo "Use 'ps aux | grep -E \"producer|consumer\"' to monitor"
	@echo "Use 'make stop' to terminate programs"

# Run producer only
run-producer: producer
	./producer

# Run consumer only
run-consumer: consumer
	./consumer

# Stop running programs
stop:
	@echo "Stopping programs..."
	@-killall producer 2>/dev/null || true
	@-killall consumer 2>/dev/null || true
	@echo "✓ Programs stopped"

# Clean compiled files and shared memory
clean:
	@echo "Cleaning up..."
	@rm -f $(TARGETS)
	@rm -f *.o
	@echo "✓ Executables removed"
	@make clean-ipc
	@echo "✓ Cleanup complete"

# Clean IPC resources only
clean-ipc:
	@echo "Cleaning IPC resources..."
	@rm -f /dev/shm/producer_consumer_shm 2>/dev/null || true
	@rm -f /dev/shm/sem_empty 2>/dev/null || true
	@rm -f /dev/shm/sem_full 2>/dev/null || true
	@rm -f /dev/shm/sem_mutex 2>/dev/null || true
	@echo "✓ Shared memory and semaphores cleaned"

# Test compilation
test-compile: clean all
	@echo "✓ Compilation test passed"

# Run a simple test
test: all
	@echo "Running basic test..."
	@echo "========================================="
	@timeout 5 bash -c './producer & ./consumer &' || true
	@make clean-ipc
	@echo "========================================="
	@echo "✓ Basic test complete"

# Show help
help:
	@echo "Producer-Consumer Makefile Commands:"
	@echo "  make all         - Compile both programs"
	@echo "  make producer    - Compile producer only"
	@echo "  make consumer    - Compile consumer only"
	@echo "  make run         - Compile and run both programs"
	@echo "  make run-producer- Run producer only"
	@echo "  make run-consumer- Run consumer only"
	@echo "  make stop        - Stop running programs"
	@echo "  make clean       - Remove executables and IPC resources"
	@echo "  make clean-ipc   - Remove IPC resources only"
	@echo "  make test        - Run basic test"
	@echo "  make help        - Show this help message"

# Check system compatibility
check-system:
	@echo "Checking system compatibility..."
	@echo -n "GCC version: "
	@gcc --version | head -1
	@echo -n "pthread support: "
	@ldconfig -p | grep pthread > /dev/null && echo "✓ Available" || echo "✗ Not found"
	@echo -n "RT library: "
	@ldconfig -p | grep librt > /dev/null && echo "✓ Available" || echo "✗ Not found"
	@echo -n "Shared memory: "
	@ls /dev/shm > /dev/null 2>&1 && echo "✓ Available" || echo "✗ Not found"

.PHONY: all clean run run-producer run-consumer stop clean-ipc test test-compile help check-system
