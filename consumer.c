/**
 * Consumer Program for Producer-Consumer Problem
 * Course: Operating Systems - Programming Assignment #1
 * 
 * This program consumes items from a shared table (buffer)
 * Uses POSIX semaphores for synchronization and shared memory for IPC
 * 
 * Compilation: gcc consumer.c -pthread -lrt -o consumer
 * Execution: ./consumer &
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>

#define TABLE_SIZE 2
#define SHM_NAME "/producer_consumer_shm"
#define SEM_EMPTY "/sem_empty"
#define SEM_FULL "/sem_full"
#define SEM_MUTEX "/sem_mutex"
#define NUM_ITEMS_TO_CONSUME 10

// Shared memory structure (must match producer)
typedef struct {
    int table[TABLE_SIZE];
    int in;
    int out;
    int count;
} SharedTable;

// Global variables
SharedTable *shared_table;
sem_t *empty, *full, *mutex;

void* consumer_thread(void* arg) {
    int thread_id = *(int*)arg;
    int item;
    
    for (int i = 0; i < NUM_ITEMS_TO_CONSUME; i++) {
        printf("[Consumer %d] Attempting to consume item\n", thread_id);
        
        // Wait if table is empty
        sem_wait(full);
        
        // Acquire mutex for critical section
        sem_wait(mutex);
        
        // Critical Section: Remove item from table
        item = shared_table->table[shared_table->out];
        printf("[Consumer %d] Retrieved item %d from position %d\n", 
               thread_id, item, shared_table->out);
        shared_table->table[shared_table->out] = 0; // Clear the slot
        shared_table->out = (shared_table->out + 1) % TABLE_SIZE;
        shared_table->count--;
        printf("[Consumer %d] Table now has %d items\n", thread_id, shared_table->count);
        
        // Release mutex
        sem_post(mutex);
        
        // Signal that table has empty slot
        sem_post(empty);
        
        // Simulate consumption time
        usleep(rand() % 1500000); // Sleep 0-1.5 seconds
    }
    
    printf("[Consumer %d] Finished consuming\n", thread_id);
    return NULL;
}

int main() {
    int shm_fd;
    pthread_t consumer1, consumer2;
    int id1 = 1, id2 = 2;
    
    // Seed random number generator
    srand(time(NULL) + 1000); // Different seed from producer
    
    // Allow time for producer to set up (in real scenario)
    sleep(1);
    
    // Open existing shared memory
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        printf("Note: Make sure producer is running first\n");
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
    
    // Open existing semaphores
    empty = sem_open(SEM_EMPTY, 0);
    if (empty == SEM_FAILED) {
        perror("sem_open empty");
        printf("Note: Make sure producer is running first\n");
        exit(1);
    }
    
    full = sem_open(SEM_FULL, 0);
    if (full == SEM_FAILED) {
        perror("sem_open full");
        exit(1);
    }
    
    mutex = sem_open(SEM_MUTEX, 0);
    if (mutex == SEM_FAILED) {
        perror("sem_open mutex");
        exit(1);
    }
    
    printf("========== CONSUMER PROGRAM STARTED ==========\n");
    printf("Table size: %d\n", TABLE_SIZE);
    printf("Current items in table: %d\n", shared_table->count);
    printf("Creating consumer threads...\n\n");
    
    // Create consumer threads
    if (pthread_create(&consumer1, NULL, consumer_thread, &id1) != 0) {
        perror("pthread_create");
        exit(1);
    }
    
    if (pthread_create(&consumer2, NULL, consumer_thread, &id2) != 0) {
        perror("pthread_create");
        exit(1);
    }
    
    // Wait for threads to complete
    pthread_join(consumer1, NULL);
    pthread_join(consumer2, NULL);
    
    printf("\n========== CONSUMER PROGRAM COMPLETED ==========\n");
    
    // Cleanup - close and unlink everything
    printf("Cleaning up shared resources...\n");
    
    // Close semaphores
    sem_close(empty);
    sem_close(full);
    sem_close(mutex);
    
    // Unlink semaphores (remove from system)
    sem_unlink(SEM_EMPTY);
    sem_unlink(SEM_FULL);
    sem_unlink(SEM_MUTEX);
    
    // Unmap and unlink shared memory
    munmap(shared_table, sizeof(SharedTable));
    shm_unlink(SHM_NAME);
    
    printf("Cleanup completed\n");
    
    return 0;
}
