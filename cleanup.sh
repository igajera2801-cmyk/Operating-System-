#!/bin/bash

# Cleanup script for Producer-Consumer programs
# Removes shared memory and semaphores

echo "Cleaning up Producer-Consumer resources..."

# Kill any running instances
echo -n "Stopping any running programs... "
killall producer 2>/dev/null
killall consumer 2>/dev/null
echo "✓"

# Remove shared memory
echo -n "Removing shared memory... "
rm -f /dev/shm/producer_consumer_shm 2>/dev/null
echo "✓"

# Remove semaphores
echo -n "Removing semaphores... "
rm -f /dev/shm/sem_empty 2>/dev/null
rm -f /dev/shm/sem_full 2>/dev/null
rm -f /dev/shm/sem_mutex 2>/dev/null
echo "✓"

# Check if cleanup was successful
if [ ! -f /dev/shm/producer_consumer_shm ] && \
   [ ! -f /dev/shm/sem_empty ] && \
   [ ! -f /dev/shm/sem_full ] && \
   [ ! -f /dev/shm/sem_mutex ]; then
    echo ""
    echo "✓ All resources cleaned successfully!"
else
    echo ""
    echo "⚠ Some resources may still exist. Check /dev/shm/"
fi
