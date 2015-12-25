#pragma once
#include <stdlib.h>
#include <stdint.h>

typedef struct {
  void* data;
  size_t length;
  struct buffer_t* next;
} buffer_t;

typedef struct {
  buffer_t* start;
  buffer_t* end;
  size_t length;
} buffer_queue_t;

/**
 * Initializes a buffer queue.
 */
void buffer_queue_init(buffer_queue_t* q);

/**
 * Clears a buffer queue.
 */
void buffer_queue_clear(buffer_queue_t* q);

/**
 * Get the total size in bytes.
 */
size_t buffer_queue_length(buffer_queue_t* q);

/**
 * Enqueues a new buffer into the buffer queue.
 */
void buffer_queue_enqueue(buffer_queue_t* q, void* src, size_t length);

/**
 * Dequeue up to n bytes from a buffer queue and write them to a destination
 * buffer. Returns the number of bytes that were dequeued.
 */
size_t buffer_queue_dequeue(buffer_queue_t* q, void* dest, size_t length);

/**
 * Variant of buffer_queue_dequeue, but it doesn't alter the queue
 */
size_t buffer_queue_top(buffer_queue_t* q, void* dest, size_t length);
