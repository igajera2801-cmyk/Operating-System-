#!/bin/bash

# Test script for Producer-Consumer Problem
# This script compiles and runs the programs, capturing output

echo "======================================"
echo "Producer-Consumer Test Script"
echo "======================================"
echo ""

# Clean up any existing resources
echo "Cleaning up previous resources..."
rm -f /dev/shm/producer_consumer_shm 2>/dev/null
rm -f /dev/shm/sem_* 2>/dev/null
echo "✓ Cleanup complete"
echo ""

# Compile programs
echo "Compiling programs..."
gcc producer.c -pthread -lrt -o producer
if [ $? -eq 0 ]; then
    echo "✓ Producer compiled successfully"
else
    echo "✗ Producer compilation failed"
    exit 1
fi

gcc consumer.c -pthread -lrt -o consumer
if [ $? -eq 0 ]; then
    echo "✓ Consumer compiled successfully"
else
    echo "✗ Consumer compilation failed"
    exit 1
fi
echo ""

# Run test
echo "Starting test run..."
echo "======================================"
echo ""

# Start producer in background and capture output
./producer > producer_output.txt 2>&1 &
PRODUCER_PID=$!
echo "Producer started (PID: $PRODUCER_PID)"

# Small delay to let producer initialize
sleep 0.5

# Start consumer in background and capture output
./consumer > consumer_output.txt 2>&1 &
CONSUMER_PID=$!
echo "Consumer started (PID: $CONSUMER_PID)"

# Wait for both to complete (with timeout)
echo ""
echo "Waiting for programs to complete..."

# Wait up to 10 seconds for completion
TIMEOUT=10
ELAPSED=0
while [ $ELAPSED -lt $TIMEOUT ]; do
    if ! ps -p $PRODUCER_PID > /dev/null 2>&1 && ! ps -p $CONSUMER_PID > /dev/null 2>&1; then
        echo "✓ Both programs completed"
        break
    fi
    sleep 1
    ELAPSED=$((ELAPSED + 1))
    echo -n "."
done
echo ""

# Force kill if still running
if ps -p $PRODUCER_PID > /dev/null 2>&1; then
    kill $PRODUCER_PID 2>/dev/null
    echo "Producer terminated (timeout)"
fi
if ps -p $CONSUMER_PID > /dev/null 2>&1; then
    kill $CONSUMER_PID 2>/dev/null
    echo "Consumer terminated (timeout)"
fi

echo ""
echo "======================================"
echo "Test Results:"
echo "======================================"
echo ""

echo "--- Producer Output ---"
cat producer_output.txt
echo ""

echo "--- Consumer Output ---"
cat consumer_output.txt
echo ""

# Save combined output
echo "======================================"
echo "Saving test results to examples/test_output.txt"
{
    echo "Test run at: $(date)"
    echo ""
    echo "=== PRODUCER OUTPUT ==="
    cat producer_output.txt
    echo ""
    echo "=== CONSUMER OUTPUT ==="
    cat consumer_output.txt
} > examples/test_output.txt

# Cleanup
rm -f producer_output.txt consumer_output.txt
rm -f /dev/shm/producer_consumer_shm 2>/dev/null
rm -f /dev/shm/sem_* 2>/dev/null

echo "✓ Test complete"
echo "======================================"
