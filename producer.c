/**
 * Producer Program for Producer-Consumer Problem
 * Course: Operating Systems - Programming Assignment #1
 * 
 * This program creates items and places them on a shared table (buffer)
 * Uses POSIX semaphores for synchronization and shared memory for IPC
 * 
 * Compilation: gcc producer.c -pthread -lrt -o producer
 * Execution: ./producer &
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define TABLE_SIZE 2
#define SHM_NAME "/producer_consumer_shm"
#define SEM_EMPTY "/sem_empty"
#define SEM_FULL "/sem_full"
#define SEM_MUTEX "/sem_mutex"
#define NUM_ITEMS_TO_PRODUCE 10

// Shared memory structure
typedef struct {
    int table[TABLE_SIZE];
    int in;
    int out;
    int count;
} SharedTable;

// Global variables
SharedTable *shared_table;
sem_t *empty, *full, *mutex;

void* producer_thread(void* arg) {
    int thread_id = *(int*)arg;
    int item;
    
    for (int i = 0; i < NUM_ITEMS_TO_PRODUCE; i++) {
        // Generate random item
        item = (thread_id * 1000) + i + 1;
        
        printf("[Producer %d] Attempting to produce item %d\n", thread_id, item);
        
        // Wait if table is full
        sem_wait(empty);
        
        // Acquire mutex for critical section
        sem_wait(mutex);
        
        // Critical Section: Add item to table
        shared_table->table[shared_table->in] = item;
        printf("[Producer %d] Placed item %d at position %d\n", 
               thread_id, item, shared_table->in);
        shared_table->in = (shared_table->in + 1) % TABLE_SIZE;
        shared_table->count++;
        printf("[Producer %d] Table now has %d items\n", thread_id, shared_table->count);
        
        // Release mutex
        sem_post(mutex);
        
        // Signal that table has item
        sem_post(full);
        
        // Simulate production time
        usleep(rand() % 1000000); // Sleep 0-1 second
    }
    
    printf("[Producer %d] Finished producing\n", thread_id);
    return NULL;
}

int main() {
    int shm_fd;
    pthread_t producer1, producer2;
    int id1 = 1, id2 = 2;
    
    // Seed random number generator
    srand(time(NULL));
    
    // Create or open shared memory
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    
    // Set the size of shared memory
    if (ftruncate(shm_fd, sizeof(SharedTable)) == -1) {
        perror("ftruncate");
        exit(1);
    }
    
    // Map shared memory
    shared_table = (SharedTable*)mmap(NULL, sizeof(SharedTable),
                                      PROT_READ | PROT_WRITE, MAP_SHARED,
                                      shm_fd, 0);
    if (shared_table == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    
    // Initialize shared table
    shared_table->in = 0;
    shared_table->out = 0;
    shared_table->count = 0;
    memset(shared_table->table, 0, sizeof(shared_table->table));
    
    // Create semaphores
    sem_unlink(SEM_EMPTY);  // Remove if exists
    sem_unlink(SEM_FULL);
    sem_unlink(SEM_MUTEX);
    
    empty = sem_open(SEM_EMPTY, O_CREAT | O_EXCL, 0666, TABLE_SIZE);
    if (empty == SEM_FAILED) {
        perror("sem_open empty");
        exit(1);
    }
    
    full = sem_open(SEM_FULL, O_CREAT | O_EXCL, 0666, 0);
    if (full == SEM_FAILED) {
        perror("sem_open full");
        exit(1);
    }
    
    mutex = sem_open(SEM_MUTEX, O_CREAT | O_EXCL, 0666, 1);
    if (mutex == SEM_FAILED) {
        perror("sem_open mutex");
        exit(1);
    }
    
    printf("========== PRODUCER PROGRAM STARTED ==========\n");
    printf("Table size: %d\n", TABLE_SIZE);
    printf("Creating producer threads...\n\n");
    
    // Create producer threads
    if (pthread_create(&producer1, NULL, producer_thread, &id1) != 0) {
        perror("pthread_create");
        exit(1);
    }
    
    if (pthread_create(&producer2, NULL, producer_thread, &id2) != 0) {
        perror("pthread_create");
        exit(1);
    }
    
    // Wait for threads to complete
    pthread_join(producer1, NULL);
    pthread_join(producer2, NULL);
    
    printf("\n========== PRODUCER PROGRAM COMPLETED ==========\n");
    printf("Note: Semaphores and shared memory remain for consumer\n");
    
    // Close semaphores (but don't unlink - consumer needs them)
    sem_close(empty);
    sem_close(full);
    sem_close(mutex);
    
    // Keep shared memory mapped (consumer needs it)
    
    return 0;
}
