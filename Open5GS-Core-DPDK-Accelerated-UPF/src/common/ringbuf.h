#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct ringbuf ringbuf_t;

ringbuf_t* ringbuf_create(size_t cap);
void       ringbuf_free(ringbuf_t* rb);

bool       ringbuf_push(ringbuf_t* rb, const void* data, size_t len);
bool       ringbuf_pop (ringbuf_t* rb, void* out, size_t* inout_len);

size_t     ringbuf_size(const ringbuf_t* rb);
size_t     ringbuf_capacity(const ringbuf_t* rb);
