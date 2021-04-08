#include "circular_buffer.h"

int cb_init(circular_buffer *cb, size_t capacity){
    cb->buffer = malloc(capacity);
    if (cb->buffer == NULL){
        return CB_MEMORY_ERROR;
    }
    cb->sidx = 0;
    cb->eidx = 0;
    cb->full = 0;
    cb->max_cap = capacity;
    return CB_SUCCESS;
}

long cb_free_cp(circular_buffer *cb, unsigned char lock){
    long free_cp = -1;
    if (lock){
        pthread_mutex_lock(&(cb->lock));
    }

    if (cb->sidx > cb->eidx){
        free_cp = cb->sidx - cb->eidx;
    }
    else if (cb->sidx < cb->eidx){
        free_cp = cb->max_cap - (cb->eidx - cb->sidx);
    }
    else{
        if (cb->full){
            free_cp = 0;
        }
        else{
            free_cp = cb->max_cap;
        }
    }
    if (lock){
        pthread_mutex_unlock(&(cb->lock));
    }
    return free_cp;
}

int cb_push_back(circular_buffer *cb, const void *buf, unsigned int in_sz){
    size_t free_cp = cb_free_cp(cb, 1);
    if (in_sz > free_cp){
        return CB_OVERFLOW_ERROR;
    }

    pthread_mutex_lock(&(cb->lock));
    if (cb->sidx <= cb->eidx){ // 000111100
        size_t tail_cp = cb->max_cap - cb->eidx;
        if (in_sz <= tail_cp){
            memcpy(cb->buffer + cb->eidx, buf, in_sz);
            cb->eidx += in_sz;
            if (cb->eidx == cb->max_cap){
                cb->eidx = 0;
            }
        }else{
            memcpy(cb->buffer + cb->eidx, buf, tail_cp);
            memcpy(cb->buffer, buf + tail_cp, in_sz - tail_cp);
            cb->eidx = in_sz - tail_cp;
        }
    }else{  // 111000011
        memcpy(cb->buffer + cb->eidx, buf, in_sz);
        cb->eidx += in_sz;
    }

    if(free_cp == in_sz){
        cb->full = 1;
    }
    pthread_mutex_unlock(&(cb->lock));
    return CB_SUCCESS;
}

long cb_pop_front(circular_buffer *cb, void *buf, unsigned int max_sz){
    long osz = -1; // no data in the buffer
    if(cb_free_cp(cb, 1) == cb->max_cap){
        return osz;
    }

    pthread_mutex_lock(&(cb->lock));
    if (cb->sidx < cb->eidx){ // 000111100
        osz = MIN(max_sz, cb->max_cap - cb_free_cp(cb, 0));
        memcpy(buf, cb->buffer + cb->sidx, osz);
        cb->sidx += osz;
    }
    else{ // 111000011 or 111111111
        osz = MIN(max_sz, cb->max_cap - cb->sidx);
        memcpy(buf, cb->buffer + cb->sidx, osz);
        if (osz < max_sz){ // 111000000 or 1111110000
            memcpy(buf+osz, cb->buffer, MIN(max_sz - osz, cb->eidx));
            cb->sidx = MIN(max_sz - osz, cb->eidx);
            osz += MIN(max_sz - osz, cb->eidx);
        }else{
            cb->sidx += osz;
        }
        if (cb->sidx == cb->eidx){
            cb->full = 0;
        }
    }
    pthread_mutex_unlock(&(cb->lock));

    return osz;
}