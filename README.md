# Producer-Consumer Problem Implementation

## Project Overview
This project implements the classic Producer-Consumer problem using POSIX threads, semaphores, and shared memory in C. The solution demonstrates inter-process communication (IPC) and synchronization between two separate processes.

## Problem Description
- **Producers** generate items and place them on a shared table (buffer)
- **Consumers** remove and consume items from the table
- The table can hold a maximum of 2 items at any time
- When the table is full, producers must wait
- When the table is empty, consumers must wait
- Proper synchronization ensures mutual exclusion and prevents race conditions

## Files Included
- `producer.c` - Producer program that generates items and places them on the shared table
- `consumer.c` - Consumer program that retrieves and consumes items from the shared table
- `README.md` - This documentation file
- `DOCUMENTATION.md` - Detailed technical documentation
- `examples/` - Directory containing example outputs and test results
- `Makefile` - Compilation automation script

## Requirements
- Linux/Unix operating system (Ubuntu 20.04+ recommended)
- GCC compiler with pthread support
- POSIX semaphore and shared memory support

## Installation & Setup

### For Linux Users
No additional setup required. The system should have all necessary libraries.

### For Windows/Mac Users
Install VirtualBox or Docker with a Linux distribution:
1. Download VirtualBox: https://www.virtualbox.org/
2. Install Ubuntu 20.04 or later
3. Install build essentials: `sudo apt-get install build-essential`

## Compilation Instructions

### Manual Compilation
```bash
# Compile the producer program
gcc producer.c -pthread -lrt -o producer

# Compile the consumer program
gcc consumer.c -pthread -lrt -o consumer
```

### Using Makefile
```bash
# Compile both programs
make all

# Clean compiled files
make clean
```

## Execution Instructions

### Running Both Programs Concurrently
```bash
./producer & ./consumer &
```

### Running Programs Separately (for testing)
```bash
# Terminal 1 - Start producer first
./producer

# Terminal 2 - Start consumer (within a few seconds)
./consumer
```

### Monitoring Execution
```bash
# View running processes
ps aux | grep -E "producer|consumer"

# Kill processes if needed
killall producer consumer
```

## Key Components

### 1. Shared Memory
- Used to create the shared table accessible by both processes
- Implemented using POSIX shared memory (`shm_open`, `mmap`)
- Table structure contains buffer array, indices, and count

### 2. Semaphores
- **Empty Semaphore**: Tracks empty slots (initialized to TABLE_SIZE)
- **Full Semaphore**: Tracks filled slots (initialized to 0)
- **Mutex Semaphore**: Ensures mutual exclusion (initialized to 1)

### 3. Threads
- Each program creates 2 threads for parallel execution
- Producer threads generate different item ranges
- Consumer threads compete for available items

### 4. Mutual Exclusion
- Critical sections protected by mutex semaphore
- Ensures only one thread accesses the table at a time
- Prevents race conditions and data corruption

## Expected Output
The programs will display:
- Producer placing items on the table
- Consumer retrieving items from the table
- Current table status after each operation
- Thread synchronization in action

## Troubleshooting

### Common Issues and Solutions

1. **"shm_open: Permission denied"**
   - Solution: Run with proper permissions or use sudo
   - Clear old semaphores: `rm /dev/shm/*`

2. **"sem_open: File exists"**
   - Solution: Previous run didn't clean up properly
   - Fix: `./cleanup.sh` or manually remove semaphores

3. **Programs hang or deadlock**
   - Ensure producer starts before consumer
   - Check if semaphores are properly initialized
   - Verify TABLE_SIZE matches in both programs

4. **Compilation errors**
   - Ensure `-pthread -lrt` flags are included
   - Check GCC version: `gcc --version` (should be 7.0+)

## Testing
Run the provided test script:
```bash
./run_tests.sh
```

## Performance Notes
- Production/consumption delays are randomized for realistic simulation
- Adjust `NUM_ITEMS_TO_PRODUCE` and `NUM_ITEMS_TO_CONSUME` for different test scenarios
- Monitor with `htop` or `top` for resource usage

## Cleanup
The consumer program automatically cleans up shared resources when it completes. If manual cleanup is needed:
```bash
# Remove shared memory
rm /dev/shm/producer_consumer_shm

# Remove semaphores
rm /dev/shm/sem_*
```

## Author
[Your Name]
[Your Email]
[Date]

## License
This project is created for educational purposes as part of Operating Systems coursework.
