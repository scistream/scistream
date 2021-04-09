#ifndef CIRCULAR_BUF_H
#define CIRCULAR_BUF_H

#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#define CB_SUCCESS           0  /* CB operation was successful */
#define CB_MEMORY_ERROR      1  /* Failed to allocate memory */
#define CB_OVERFLOW_ERROR    2  /* CB is full. Cannot push more items. */
#define CB_EMPTY_ERROR       3  /* CB is empty. Cannot pop more items. */

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

typedef struct circular_buffer {
    void *buffer;
    size_t sidx;
    size_t eidx;    // no valid data at this index
    size_t max_cap;
    unsigned char full; // a better way to distinguish sidx==eidx? either full or empty
    pthread_mutex_t lock;
} circular_buffer;

int cb_init(circular_buffer *cb, size_t capacity);
long cb_free_cp(circular_buffer *cb, unsigned char lock);
int cb_push_back(circular_buffer *cb, const void *buf, unsigned int in_sz);
long cb_pop_front(circular_buffer *cb, void *buf, unsigned int max_sz);
void print_cb_status(circular_buffer *cb);

#endif