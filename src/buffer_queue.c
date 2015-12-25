#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "buffer_queue.h"

void buffer_queue_init(buffer_queue_t* q) {
  q->start  = NULL;
  q->end    = NULL;
  q->length = 0;
}

void buffer_queue_clear(buffer_queue_t* q) {
  buffer_t* buffer = q->start;

  for (; buffer != NULL; buffer = (buffer_t*) buffer->next) {
    q->start = (buffer_t*) buffer->next;
    free(buffer->data);
    free(buffer);
    buffer = q->start;
  }

  q->end    = NULL;
  q->length = 0;
}

void buffer_queue_enqueue(buffer_queue_t* q, void* src, size_t length) {
  buffer_t* buffer = malloc(sizeof(buffer_t));
  buffer->data   = src;
  buffer->length = length;
  buffer->next   = NULL;

  if (q->start == NULL) {
    q->start = buffer;
    q->end   = buffer;
  } else {
    q->end->next = (struct buffer_t*) buffer;
    q->end       = buffer;
  }

  q->length += length;
}

size_t buffer_queue_dequeue(buffer_queue_t* q, void* dest, size_t length) {
  buffer_t *buffer = q->start;
  size_t bytes_read = 0;

  // Consume entire buffers for as long as possible
  while (buffer != NULL && buffer->length <= length) {
    memcpy(dest, buffer->data, buffer->length);

    dest        = (uint8_t *) dest + buffer->length;
    length     -= buffer->length;
    bytes_read += buffer->length;

    q->start = (buffer_t*) buffer->next;
    if (NULL == q->start) q->end = NULL;
    q->length -= buffer->length;

    free(buffer->data);
    free(buffer);

    buffer = q->start;
  }

  // If no data in buffer anymore
  if (buffer == NULL)
    return bytes_read;

  // Consume parts of a buffer
  memcpy(dest, buffer->data, length);
  memmove(buffer->data, (uint8_t *) buffer->data + length, buffer->length - length);
  buffer->data = realloc(buffer->data, buffer->length - length);
  q->length -= length;

  return bytes_read + length;
}
