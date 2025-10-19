# Technical Documentation

## 1. System Architecture

### Overview
The Producer-Consumer system consists of two separate processes that communicate through shared memory and synchronize using POSIX semaphores. This design demonstrates true inter-process communication (IPC) rather than just inter-thread communication.

### Architecture Diagram
```
┌─────────────────┐          ┌─────────────────┐
│  Producer       │          │  Consumer       │
│  Process        │          │  Process        │
│  ┌───────────┐  │          │  ┌───────────┐  │
│  │ Thread 1  │  │          │  │ Thread 1  │  │
│  └───────────┘  │          │  └───────────┘  │
│  ┌───────────┐  │          │  ┌───────────┐  │
│  │ Thread 2  │  │          │  │ Thread 2  │  │
│  └───────────┘  │          │  └───────────┘  │
└────────┬────────┘          └────────┬────────┘
         │                            │
         ↓                            ↓
    ┌────────────────────────────────────┐
    │         Shared Memory              │
    │  ┌────────────────────────────┐   │
    │  │   Table (Circular Buffer)   │   │
    │  │   [Item1] [Item2]           │   │
    │  └────────────────────────────┘   │
    └────────────────────────────────────┘
         ↑            ↑            ↑
    ┌────────┐   ┌────────┐   ┌────────┐
    │ Empty  │   │  Full  │   │ Mutex  │
    │  Sem   │   │  Sem   │   │  Sem   │
    └────────┘   └────────┘   └────────┘
```

## 2. Implementation Details

### 2.1 Shared Memory Structure
```c
typedef struct {
    int table[TABLE_SIZE];  // Circular buffer for items
    int in;                 // Index for next insertion
    int out;                // Index for next removal
    int count;              // Current number of items
} SharedTable;
```

**Key Points:**
- The table is implemented as a circular buffer
- `in` and `out` indices wrap around using modulo operation
- `count` provides quick access to current table occupancy

### 2.2 Semaphore Usage

#### Empty Semaphore
- **Purpose**: Tracks the number of empty slots in the table
- **Initial Value**: TABLE_SIZE (2)
- **Producer waits** on this when table is full
- **Consumer signals** this after removing an item

#### Full Semaphore
- **Purpose**: Tracks the number of filled slots in the table
- **Initial Value**: 0
- **Consumer waits** on this when table is empty
- **Producer signals** this after adding an item

#### Mutex Semaphore
- **Purpose**: Ensures mutual exclusion for critical sections
- **Initial Value**: 1
- **Both processes** acquire before accessing shared table
- **Both processes** release after completing table operation

### 2.3 Process Synchronization Flow

#### Producer Thread Operation
```
1. Generate item
2. sem_wait(empty)       // Wait for empty slot
3. sem_wait(mutex)       // Enter critical section
4. Add item to table
5. Update indices and count
6. sem_post(mutex)       // Exit critical section
7. sem_post(full)        // Signal item available
8. Sleep (simulate production time)
9. Repeat
```

#### Consumer Thread Operation
```
1. sem_wait(full)        // Wait for available item
2. sem_wait(mutex)       // Enter critical section
3. Remove item from table
4. Update indices and count
5. sem_post(mutex)       // Exit critical section
6. sem_post(empty)       // Signal empty slot
7. Process item
8. Sleep (simulate consumption time)
9. Repeat
```

## 3. Critical Section Management

### What's Protected
The critical section includes all operations that modify the shared table:
- Reading/Writing to table array
- Updating `in` and `out` indices
- Modifying the `count` variable

### Why It's Necessary
Without proper mutual exclusion, race conditions could occur:
- Two producers might write to the same slot
- A consumer might read partially written data
- Index variables could become corrupted
- Count could become inconsistent with actual items

## 4. POSIX API Functions Used

### Shared Memory Functions
- `shm_open()`: Creates/opens a shared memory object
- `ftruncate()`: Sets the size of shared memory
- `mmap()`: Maps shared memory into process address space
- `munmap()`: Unmaps shared memory
- `shm_unlink()`: Removes shared memory object

### Semaphore Functions
- `sem_open()`: Creates/opens a named semaphore
- `sem_wait()`: Decrements semaphore (blocking)
- `sem_post()`: Increments semaphore
- `sem_close()`: Closes semaphore
- `sem_unlink()`: Removes named semaphore

### Thread Functions
- `pthread_create()`: Creates a new thread
- `pthread_join()`: Waits for thread completion

## 5. Design Decisions

### Why Separate Processes Instead of Threads?
- Demonstrates true IPC mechanisms
- More realistic for distributed systems
- Shows how unrelated processes can communicate
- Better isolation and fault tolerance

### Why Named Semaphores?
- Can be shared across processes
- Persistent across program runs
- Easier debugging (visible in /dev/shm/)

### Why Circular Buffer?
- Efficient use of fixed memory
- Simple index management
- No need for dynamic memory allocation
- Natural fit for producer-consumer pattern

## 6. Synchronization Guarantees

### Safety Properties
1. **Mutual Exclusion**: At most one thread accesses the table at any time
2. **Bounded Buffer**: Table never exceeds capacity
3. **Data Integrity**: No corrupted or partially written items

### Liveness Properties
1. **Progress**: If table not full, producers can eventually produce
2. **No Starvation**: All threads get fair chance to access table
3. **Deadlock Freedom**: Proper semaphore ordering prevents deadlock

## 7. Performance Considerations

### Optimizations
- Circular buffer eliminates need for shifting elements
- Named semaphores reduce overhead vs. other IPC methods
- Multiple threads increase throughput

### Bottlenecks
- Mutex creates serialization point
- Context switching overhead with multiple threads
- Shared memory access may cause cache invalidation

## 8. Error Handling

### Checked Conditions
- Shared memory creation/opening failures
- Semaphore creation/opening failures
- Thread creation failures
- Memory mapping failures

### Recovery Strategies
- Clear error messages with `perror()`
- Cleanup of partial resources
- Graceful termination on critical errors

## 9. Testing Strategies

### Unit Testing
- Test with single producer, single consumer
- Test with multiple producers/consumers
- Test with different production/consumption rates

### Stress Testing
- Run for extended periods
- Vary TABLE_SIZE
- Test with asymmetric thread counts

### Edge Cases
- Producer much faster than consumer
- Consumer much faster than producer
- Rapid start/stop cycles
- System resource exhaustion

## 10. Known Limitations

1. **Fixed Buffer Size**: TABLE_SIZE is compile-time constant
2. **No Priority**: All threads have equal priority
3. **No Item Validation**: No checksums or validity checks
4. **Limited Error Recovery**: Critical errors cause termination
5. **Platform Specific**: Requires POSIX-compliant system

## 11. Possible Enhancements

1. **Dynamic Buffer Sizing**: Adjust table size at runtime
2. **Priority Queue**: Implement priority-based consumption
3. **Multiple Tables**: Separate tables for different item types
4. **Network Distribution**: Extend to work across network
5. **Persistence**: Save table state to disk
6. **Statistics**: Track production/consumption rates
7. **Graceful Shutdown**: Signal handling for clean termination

## 12. Compilation Flags Explanation

- `-pthread`: Links pthread library for thread support
- `-lrt`: Links real-time library for shared memory
- `-o`: Specifies output file name

## 13. System Requirements Details

### Minimum Requirements
- Linux kernel 2.6.32 or later
- GCC 4.8 or later
- POSIX.1-2001 compliance
- 64MB RAM
- 1MB disk space

### Recommended Environment
- Ubuntu 20.04 LTS or later
- GCC 9.0 or later
- 256MB RAM
- Multi-core processor for true parallelism
