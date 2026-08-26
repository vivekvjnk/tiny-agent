#include <stdbool.h>
#include <stdint.h>
#include <stdatomic.h>

// 1. Capacity MUST be a power of 2 for bitwise masking (& (RING_BUFFER_SIZE - 1))
#define RING_BUFFER_SIZE 8U 
#define RING_BUFFER_MASK (RING_BUFFER_SIZE - 1U)

static uint32_t g_circular_queue[RING_BUFFER_SIZE];

// 2. Use standard atomic uint types initialized to zero
static atomic_uint g_tail = ATOMIC_VAR_INIT(0);
static atomic_uint g_head = ATOMIC_VAR_INIT(0);

/**
 * publish_data - Producer Enqueue
 * Only called by Producer thread/ISR
 */
bool publish_data(uint32_t data) {
    // 1. Load current tail (local thread owns tail, can use relaxed)
    uint32_t current_tail = atomic_load_explicit(&g_tail, memory_order_relaxed);
    
    // 2. Acquire load head to see where the consumer is up to
    uint32_t current_head = atomic_load_explicit(&g_head, memory_order_acquire);

    // 3. Queue is FULL if (tail - head) == capacity
    if ((current_tail - current_head) >= RING_BUFFER_SIZE) {
        return false; // Buffer Full
    }

    // 4. Write data to slot using bitwise mask
    g_circular_queue[current_tail & RING_BUFFER_MASK] = data;

    // 5. Release store: Publish updated tail to consumer core
    // Guarantees data store above is committed BEFORE tail update is visible
    atomic_store_explicit(&g_tail, current_tail + 1U, memory_order_release);
    
    return true;
}

/**
 * consume_data - Consumer Dequeue
 * Only called by Consumer thread/ISR
 */
bool consume_data(uint32_t *out_data) {
    if (!out_data) return false;

    // 1. Load current head (local thread owns head, can use relaxed)
    uint32_t current_head = atomic_load_explicit(&g_head, memory_order_relaxed);

    // 2. Acquire load tail to see how far the producer has written
    uint32_t current_tail = atomic_load_explicit(&g_tail, memory_order_acquire);

    // 3. Queue is EMPTY if tail == head
    if (current_tail == current_head) {
        return false; // Buffer Empty
    }

    // 4. Read data from slot using bitwise mask
    *out_data = g_circular_queue[current_head & RING_BUFFER_MASK];

    // 5. Release store: Publish updated head back to producer core
    // Guarantees data read above finishes BEFORE head update is published
    atomic_store_explicit(&g_head, current_head + 1U, memory_order_release);

    return true;
}