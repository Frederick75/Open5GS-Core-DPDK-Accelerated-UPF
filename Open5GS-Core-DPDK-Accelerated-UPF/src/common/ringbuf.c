#include "common/ringbuf.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

struct ringbuf {
  uint8_t* buf;
  size_t cap;
  size_t head;
  size_t tail;
  size_t size;
  pthread_mutex_t mu;
};

ringbuf_t* ringbuf_create(size_t cap){
  ringbuf_t* rb = calloc(1, sizeof(*rb));
  if (!rb) return NULL;
  rb->buf = calloc(1, cap);
  if (!rb->buf){ free(rb); return NULL; }
  rb->cap = cap;
  pthread_mutex_init(&rb->mu, NULL);
  return rb;
}

void ringbuf_free(ringbuf_t* rb){
  if (!rb) return;
  pthread_mutex_destroy(&rb->mu);
  free(rb->buf);
  free(rb);
}

size_t ringbuf_size(const ringbuf_t* rb){ return rb ? rb->size : 0; }
size_t ringbuf_capacity(const ringbuf_t* rb){ return rb ? rb->cap : 0; }

static size_t min_sz(size_t a, size_t b){ return a < b ? a : b; }

bool ringbuf_push(ringbuf_t* rb, const void* data, size_t len){
  if (!rb || !data || len == 0) return false;
  pthread_mutex_lock(&rb->mu);
  if (len > (rb->cap - rb->size)){ pthread_mutex_unlock(&rb->mu); return false; }

  size_t first = min_sz(len, rb->cap - rb->tail);
  memcpy(rb->buf + rb->tail, data, first);
  memcpy(rb->buf, (const uint8_t*)data + first, len - first);

  rb->tail = (rb->tail + len) % rb->cap;
  rb->size += len;
  pthread_mutex_unlock(&rb->mu);
  return true;
}

bool ringbuf_pop(ringbuf_t* rb, void* out, size_t* inout_len){
  if (!rb || !out || !inout_len) return false;
  pthread_mutex_lock(&rb->mu);
  size_t want = *inout_len;
  if (want == 0 || rb->size < want){ pthread_mutex_unlock(&rb->mu); return false; }

  size_t first = min_sz(want, rb->cap - rb->head);
  memcpy(out, rb->buf + rb->head, first);
  memcpy((uint8_t*)out + first, rb->buf, want - first);

  rb->head = (rb->head + want) % rb->cap;
  rb->size -= want;
  pthread_mutex_unlock(&rb->mu);
  return true;
}
