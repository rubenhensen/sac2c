#ifndef _SAC_ALLOCCACHE_H_
#define _SAC_ALLOCCACHE_H_

#include "libsac/essentials/message.h"

#ifdef __CUDACC__

#include <cuda_runtime.h>

void cache_init ();

void *cache_malloc (size_t size, int device);

void cache_free (void *pointer, int device);

cudaStream_t *cache_stream_create (int device);

void cache_stream_destroy (int device);

void cache_end ();

#ifdef DEBUG_CACHE
#define debug(s, ...) printf (s, ##__VA_ARGS__)
#else
#define debug(s, ...)
#endif

typedef struct cache_alloc {
    void *pointer;
    size_t size;
    struct cache_alloc *next;
    bool inUse;
} * cache_alloc_t;

typedef struct cache_stream {
    cudaStream_t stream;
    struct cache_stream *next;
    bool inUse;
} * cache_stream_t;

static cache_alloc_t *cacheAlloc;
static cache_stream_t *cacheStream;

extern unsigned int SAC_CUDA_DEVICES;

/* initiate caches, allocate initial arrays */
void
cache_init ()
{
    cacheAlloc = (cache_alloc_t *)calloc (SAC_CUDA_DEVICES, sizeof (cache_alloc_t));
    cacheStream = (cache_stream_t *)calloc (SAC_CUDA_DEVICES, sizeof (cache_stream_t));
}

/* end cache, free everything */
void
cache_end ()
{
    for (unsigned int i = 0; i < SAC_CUDA_DEVICES; i++) {
        cudaSetDevice (i);
        for (cache_alloc_t elem = cacheAlloc[i]; elem != NULL;) {
            cache_alloc_t temp;
            cudaFree (elem->pointer);
            temp = elem;
            elem = elem->next;
            free (temp);
        }
        for (cache_stream_t elem = cacheStream[i]; elem != NULL;) {
            cache_stream_t temp;
            cudaStreamDestroy (elem->stream);
            temp = elem;
            elem = elem->next;
            free (temp);
        }
    }
    free (cacheAlloc);
    free (cacheStream);
}

/* return pointer for array of some size on some device, either from cache or
 freshly malloc'd */
void *
cache_malloc (size_t size, int device)
{
    int res;
    cache_alloc_t newElem;
    cache_alloc_t *last;

    debug ("Looking for array of size %zu in cache for device %d\n", size, device);
    /* first check cache */
    for (cache_alloc_t elem = cacheAlloc[device]; elem != NULL; elem = elem->next) {
        if (elem->size == size && !(elem->inUse)) {
            debug ("Found array in cache\n");
            elem->inUse = true;
            return elem->pointer;
        }
    }

    debug ("Array not found, allocating\n");
    /* if not found, malloc */
    newElem = (cache_alloc_t)malloc (sizeof (struct cache_alloc));
    cudaSetDevice (device);

    if ((res = cudaMalloc (&(newElem->pointer), size)) != cudaSuccess) {
        debug ("Allocation failed, freeing cache\n");
        /* allocation failed, free cache */
        for (cache_alloc_t *elem = &(cacheAlloc[device]); *elem != NULL;) {
            if (!((*elem)->inUse)) {
                cache_alloc_t temp;
                cudaFree ((*elem)->pointer);
                temp = *elem;
                elem = &((*elem)->next);
                free (temp);
            } else
                elem = &((*elem)->next);
        }
        cacheAlloc[device] = NULL;
        if ((res = cudaMalloc (&(newElem->pointer), size)) != cudaSuccess) {
            SAC_RuntimeError ("Error allocating %zu bytes on CUDA device %d: %d\n", size,
                              device, res);
            exit (-1);
        }
    }
    newElem->size = size;
    newElem->next = NULL;
    newElem->inUse = true;

    for (last = &(cacheAlloc[device]); *last != NULL; last = &((*last)->next))
        ;
    *last = newElem;

    debug ("Allocated %p on device %d\n", newElem->pointer, device);

    return newElem->pointer;
}

/* marks given pointer as no longer being used, keeps it alloc'd */
void
cache_free (void *pointer, int device)
{
    debug ("Freeing %p on device %d\n", pointer, device);
    for (cache_alloc_t elem = cacheAlloc[device]; elem != NULL; elem = elem->next) {
        debug ("Found %p on device %d == %p ?: %d\n", elem->pointer, device, pointer,
               elem->pointer == pointer);
        if (elem->pointer == pointer) {
            elem->inUse = false;
            return;
        }
    }
    SAC_RuntimeError ("Pointer %p being freed not found on cache for device %d\n",
                      pointer, device);
    exit (-1);
}

/* returns pointer for a stream, either from cache or freshly created */
cudaStream_t *
cache_stream_create (int device)
{
    cache_stream_t newElem;
    cache_stream_t *last;

    /* first check cache */
    for (cache_stream_t elem = cacheStream[device]; elem != NULL; elem = elem->next) {
        if (!(elem->inUse)) {
            debug ("Found stream in cache\n");
            elem->inUse = true;
            return &(elem->stream);
        }
    }

    debug ("Stream not found, allocating\n");
    /* if not found, malloc */
    newElem = (cache_stream_t)malloc (sizeof (struct cache_stream));
    cudaSetDevice (device);
    cudaStreamCreate (&(newElem->stream));
    newElem->next = NULL;
    newElem->inUse = true;

    for (last = &(cacheStream[device]); *last != NULL; last = &((*last)->next))
        ;
    *last = newElem;

    return &(newElem->stream);
}

/* marks given pointer as no longer being used, keeps it alloc'd */
void
cache_stream_destroy (int device)
{
    for (cache_stream_t elem = cacheStream[device]; elem != NULL; elem = elem->next) {
        elem->inUse = false;
    }
    return;
}

#endif
#endif
