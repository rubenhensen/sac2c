/* This is an alternative implementation of the distributed variables, which
 * focused on providing sparse allocation of arrays. The idea turned out to be
 * too cumbersome to implement and not very good performance wise, with very
 * little gain, especially if you don't have many GPUs on your machine.
 *
 * It currently does not work, the code below is for reference only.
 */

#ifndef _SAC_DIST_VAR_MEMSAVE_H_
#define _SAC_DIST_VAR_MEMSAVE_H_

#ifdef __CUDACC__

#ifdef DIST_MEMSAVE

/*
 * dist var stuff
 */

enum block_state { Invalid = 0, Shared = 1, Owned = 2 };

typedef struct block_allocation {
    char *pointer; // array offsets in bytes
    unsigned int start;
    unsigned int stop;
    block_state *state;
    struct block_allocation *next;
} * block_allocation_t;

typedef struct dist_var {
    unsigned int n_blocks;
    size_t block_size;
    int *owner;
    block_allocation_t *allocations;
} * dist_var_t;

extern unsigned int SAC_MT_DEVICES;
extern unsigned int SAC_CUDA_DEVICES;
extern double SAC_CUDA_SHARE;

/*
 * memfuncs
 */

// allocate dist var
dist_var_t dist_var_alloc (unsigned int n_blocks, int total_size, size_t unit_size);

// free dist var
dist_var_t dist_var_free (dist_var_t dist_array);

/*
 * type conversions
 */

// updates distributed variable structure
dist_var_t host2dist_st (dist_var_t dist_array, unsigned int block_start,
                         unsigned int block_stop);
dist_var_t host2dist_spmd (dist_var_t dist_array, unsigned int block_start,
                           unsigned int block_stop);
dist_var_t dev2dist_spmd (dist_var_t dist_array, void *array, unsigned int block_start,
                          unsigned int block_stop, int device);

// assigns section of distributed array to device, returns pointer for section
void *dist2conc (dist_var_t dist_array, unsigned int block_start, unsigned int block_stop,
                 int device, bool set_owner, cudaStream_t *stream);

/* others */
// return index of last block between [block_start->block_stop] with same state as
// block_start
unsigned int dist_end_contiguous_block (dist_var_t dist_array, unsigned int block_start,
                                        unsigned int block_stop, int device);

#ifdef DEBUG_DIST
#define debug(s, ...) printf (s, ##__VA_ARGS__)
#define checkBounds                                                                      \
    if (block_start > dist_array->n_blocks) {                                            \
        printf ("starting block %d bigger than total of %d!\n", block_start,             \
                dist_array->n_blocks);                                                   \
        exit (-1);                                                                       \
    }                                                                                    \
    if (block_stop > (dist_array->n_blocks)) {                                           \
        printf ("ending block %d bigger than total of %d!\n", block_stop,                \
                dist_array->n_blocks);                                                   \
        exit (-1);                                                                       \
    }
#else
#define debug(s, ...)
#define checkBounds
#endif

/*
 * dist var stuff
 */
#include <stdlib.h>
#include <stdio.h>

unsigned int SAC_MT_DEVICES;
unsigned int SAC_CUDA_DEVICES;
double SAC_CUDA_SHARE;

#include "dist_var.h"
#include "alloc_cache.h"

/*
 * memfuncs
 */
dist_var_t
dist_var_alloc (unsigned int n_blocks, int total_size, size_t unit_size)
{
    dist_var_t result = (dist_var_t)malloc (sizeof (dist_var));

    result->n_blocks = n_blocks;
    result->block_size = total_size * unit_size / n_blocks;
    int *owner = (int *)malloc (SAC_MT_DEVICES * n_blocks * sizeof (int));
    for (unsigned int i = 0; i < n_blocks; i++)
        owner[i] = -1;
    result->owner = owner;
    result->allocations
      = (block_allocation_t *)calloc (SAC_MT_DEVICES, sizeof (block_allocation_t));

    debug ("Allocating host-side array\n");
    block_allocation_t alloc = (block_allocation_t)malloc (sizeof (block_allocation));
    alloc->start = 0;
    alloc->stop = n_blocks;
    alloc->next = NULL;
    alloc->pointer = (char *)malloc (total_size * unit_size);
    alloc->state = (block_state *)calloc (n_blocks, sizeof (block_state));
    result->allocations[0] = alloc;

    debug ("New dist var with %d blocks of size: %zu\n", n_blocks, result->block_size);

    return result;
}

dist_var_t
dist_var_free (dist_var_t dist_array)
{
    debug ("Freeing dist_array %p\n", dist_array);

    size_t block_size = dist_array->block_size;
    block_allocation_t *allocations = dist_array->allocations;

    free (dist_array->owner);

    // unswitched host
    for (block_allocation_t alloc = allocations[0]; alloc != NULL;) {
        block_allocation_t temp;

        free (alloc->pointer);
        free (alloc->state);

        temp = alloc->next;
        free (alloc);
        alloc = temp;
    }
    for (unsigned int i = 1; i < SAC_MT_DEVICES; i++) {
        for (block_allocation_t alloc = allocations[i]; alloc != NULL;) {
            block_allocation_t temp;
            unsigned int alloc_start = alloc->start;

            cache_free (alloc->pointer + alloc_start * block_size, i - 1);
            free (alloc->state + alloc_start);

            temp = alloc->next;
            free (alloc);
            alloc = temp;
        }
    }
    free (dist_array->allocations);
    free (dist_array);

    return dist_array;
}

/*type conversions*/
dist_var_t
host2dist_st (dist_var_t dist_array, unsigned int block_start, unsigned int block_stop)
{
    debug ("host2dist_st(dist %p, start %d, stop %d)\n", dist_array, block_start,
           block_stop);

    block_state *state = dist_array->allocations[0]->state;
    block_allocation_t alloc;

    // unswitch host case
    for (unsigned int j = block_start; j < block_stop; j++) {
        state[j] = Owned;
    }

    for (unsigned int i = 1; i < SAC_MT_DEVICES; i++) {
        // look for allocations starting before or at block_start
        for (alloc = dist_array->allocations[i];
             alloc != NULL && alloc->start < block_stop; alloc = alloc->next) {
            state = alloc->state;
            unsigned int stop = min (alloc->stop, block_stop);
            // mark as invalid
            for (unsigned int j = block_start; j < stop; j++)
                state[j] = Invalid;
        }
    }

    debug ("host2dist_st(dist %p, start %d, stop %d) = %p\n", dist_array, block_start,
           block_stop, dist_array);

    return dist_array;
}

dist_var_t
host2dist_spmd (dist_var_t dist_array, unsigned int block_start, unsigned int block_stop)
{
    debug ("host2dist_spmd(dist %p, start %d, stop %d)\n", dist_array, block_start,
           block_stop);

    // mark as owned
    block_state *state = dist_array->allocations[0]->state;
    for (unsigned int i = block_start; i < block_stop; i++) {
        state[i] = Owned;
    }
    debug ("host2dist_spmd(dist %p, start %d, stop %d) = %p\n", dist_array, block_start,
           block_stop, dist_array);

    return dist_array;
}

dist_var_t
dev2dist_spmd (dist_var_t dist_array, void *array, unsigned int block_start,
               unsigned int block_stop, int device)
{
    debug ("dev2dist_spmd(dist %p, array %p, start %d, stop %d, device %d)\n", dist_array,
           array, block_start, block_stop, device);

    char *pointer = (char *)array;
    block_allocation_t alloc;
    block_state *state;

    // find allocations including determined blocks.
    // unswitch owned allocation block
    for (alloc = dist_array->allocations[device];
         alloc != NULL && alloc->pointer != pointer; alloc = alloc->next) {
        state = alloc->state;
        for (unsigned int i = alloc->start; i < alloc->stop; i++)
            state[i] = Invalid;
    }
    for (; alloc != NULL && alloc->pointer == pointer; alloc = alloc->next) {
        unsigned int i;
        state = alloc->state;

        // unswitch owned block range
        for (i = alloc->start; i < block_start; i++) {
            state[i] = Invalid;
        }
        for (; i < block_stop; i++) {
            state[i] = Owned;
        }
        for (; i < alloc->stop; i++) {
            state[i] = Invalid;
        }
    }
    for (; alloc != NULL; alloc = alloc->next) {
        state = alloc->state;
        for (unsigned int i = alloc->start; i < alloc->stop; i++)
            state[i] = Invalid;
    }

    // invalidate on host
    state = dist_array->allocations[0]->state;
    for (unsigned int i = block_start; i < block_stop; i++) {
        state[i] = Invalid;
    }
    debug ("dev2dist_spmd(dist %p, array %p, start %d, stop %d, device %d) = %p\n",
           dist_array, array, block_start, block_stop, device, dist_array);

    return dist_array;
}

unsigned int
dist_end_contiguous_block (dist_var_t dist_array, unsigned int block_start,
                           unsigned int block_stop, int device)
{

    debug ("dist_end_contiguous_block(dist %p, start %d, stop %d, device %d)\n",
           (void *)dist_array, block_start, block_stop, device);

    checkBounds

      block_allocation_t alloc;
    unsigned int res;

    if (block_start < block_stop) {

        // find allocation that contains block_start
        for (alloc = dist_array->allocations[device];
             alloc != NULL && alloc->stop <= block_start; alloc = alloc->next)
            ;

        // we reached the end of allocation list without finding it
        // [block_start->block_stop] not allocated
        if (alloc == NULL) {
            debug (
              "Device %d: reached the end of allocation list without finding blocks\n",
              device);
            res = block_stop;
        }
        // or we found an allocation starting after block_start, it is not allocated.
        // either next allocation starts before block_stop,
        // or nothing is allocated until block_stop.
        else if (alloc->start > block_start) {
            debug ("Device %d: found an allocation starting after block_start at %d\n",
                   device, alloc->start);
            res = min (alloc->start, block_stop);
        }
        // or we found an allocation starting at/before block_start.
        // either this allocation ends before block_stop,
        // or it ends at/after block_stop.
        else {
            debug (
              "Device %d: found an allocation starting at/before block_start at %d\n",
              device, alloc->start);
            unsigned int max = min (alloc->stop, block_stop);
            // check local state
            if (alloc->state[block_start] == Invalid)
                for (res = block_start + 1; res < max && alloc->state[res] == Invalid;
                     res++)
                    ;
            else
                for (res = block_start + 1; res < max && alloc->state[res] != Invalid;
                     res++)
                    ;
        }
    } else { // kernel with empty area
        res = block_stop;
    }

    debug ("dist_end_contiguous_block(dist %p, start %d, stop %d, device %d) = %d\n",
           (void *)dist_array, block_start, block_stop, device, res);
    return res;
}

void *
dist2conc (dist_var_t dist_array, unsigned int block_start, unsigned int block_stop,
           int device, bool set_owner, cudaStream_t *stream)
{

    debug ("dist2conc(dist %p, start %d, stop %d, device %d, owner? %d, stream %p)\n",
           (void *)dist_array, block_start, block_stop, device, set_owner, stream);

    char *array_p;
    size_t block_size = dist_array->block_size;
    int *owner = dist_array->owner;
    block_allocation_t alloc;

    checkBounds

      /* find/allocate destination array */

      /* find place for this block:
        start looking from the start.
        We stop looking when the block starts after block_start (one too far)
        or when the block stops at/after block_stop (correct block)
       */
      for (alloc = dist_array->allocations[device];
           alloc != NULL && alloc->start <= block_start && alloc->stop < block_stop;
           alloc = alloc->next);

    // if not the right block
    if (alloc == NULL || alloc->start > block_start) {
        debug ("Device %d: Right block not allocated\n", device);
        // there is no allocation at or after block_start
        if (alloc == NULL) {
            debug ("Device %d: No allocations starting after block_start\n", device);
            block_allocation_t *alloc_p;
            // find last pointer
            for (alloc_p = &(dist_array->allocations[device]); *alloc_p != NULL;
                 alloc_p = &(*alloc_p)->next)
                ;
            // create block_allocation
            alloc = (block_allocation_t)malloc (sizeof (block_allocation));
            *alloc_p = alloc;
            alloc->next = NULL;
        }
        // if we are too far, create a new block_allocation at the correct spot
        else if (alloc->start > block_start) {
            debug ("Device %d: One allocation after block_start\n", device);
            // create next block_allocation from current
            block_allocation_t next
              = (block_allocation_t)malloc (sizeof (block_allocation));
            memcpy (next, alloc, sizeof (block_allocation));
            /* change current */
            alloc->next = next;
        }

        alloc->start = block_start;
        alloc->stop = block_stop;
        alloc->state
          = (block_state *)calloc ((block_stop - block_start), sizeof (block_state))
            - block_start;

        // allocate
        size_t size_alloc = (block_stop - block_start) * block_size;
        if (device == 0) { /* host initialization */
            debug ("Device %d: allocating host-side array\n", device);
            array_p = (char *)malloc (size_alloc) - alloc->start * block_size;
        } else { /* cuda allocation */
            debug ("Device %d: allocating cuda array\n", device);
            array_p
              = (char *)cache_malloc (size_alloc, device - 1) - alloc->start * block_size;
        }
        alloc->pointer = array_p;
    } else {
        array_p = alloc->pointer;
    }
    debug ("Device %d: Will store at %p\n", device, array_p);

    // for each block requested:
    for (unsigned int i = block_start; i < block_stop;) {
        unsigned int k;
        block_state *state = alloc->state;

        debug ("Device %d checking block %d: ", device, i);
        debug ("local state %d, ", state[i]);
        debug ("owner %d\n", owner[i]);
        // if the block is not in the destination array and is elsewhere
        if (state[i] == Invalid && owner[i] != -1) {
            int source = owner[i];

            // find more contiguous not valid blocks
            for (k = i + 1; k < block_stop && state[k] == Invalid && owner[k] == source;
                 k++)
                ;

            debug ("Device %d Missing blocks %d to %d, will copy from %d\n", device, i, k,
                   source);

            // the blocks may be spread across different allocations on source, loop until
            // we get all of them
            while (i < k) {
                unsigned int j;
                char *source_p, *destination_p;
                block_allocation_t alloc_src;

                // find allocation including block i
                for (alloc_src = dist_array->allocations[source];
                     alloc_src->stop < i && alloc_src->state[i] != Invalid;
                     alloc_src = alloc_src->next)
                    ;
                block_state *state_src = alloc_src->state;
                // find how many contiguous blocks are available in found allocation
                for (j = i + 1; j < k && state_src[j] != Invalid; j++)
                    ;
                // get fake pointer
                source_p = alloc_src->pointer;

                size_t offset = i * block_size;
                size_t size = (j - i) * block_size;
                source_p += offset;
                destination_p = array_p + offset;

                debug ("Device %d: offset %zu, size %zu\n", device, offset, size);
                if (device == 0) { /* cuda to host */
                    debug ("Device %d: copying from cuda %p to host %p\n", device,
                           source_p, destination_p);
                    cudaSetDevice (source - 1);
                    cudaMemcpy (destination_p, source_p, size, cudaMemcpyDeviceToHost);
                } else if (source == 0) { /* host to cuda */
                    debug ("Device %d: copying from host %p to cuda %p\n", device,
                           source_p, destination_p);
                    cudaSetDevice (device - 1);
                    if (stream) {
                        cudaMemcpyAsync (destination_p, source_p, size,
                                         cudaMemcpyHostToDevice, *stream);
                    } else {
                        cudaMemcpy (destination_p, source_p, size,
                                    cudaMemcpyHostToDevice);
                    }
                } else { /* cuda to cuda */
                    debug ("Device %d: copying from cuda %p to cuda %p\n", device,
                           source_p, destination_p);
                    if (stream) {
                        cudaMemcpyPeerAsync (destination_p, device - 1, source_p,
                                             source - 1, size, *stream);
                    } else {
                        cudaMemcpyPeer (destination_p, device - 1, source_p, source - 1,
                                        size);
                    }
                }

                // mark blocks in block_allocation
                for (unsigned int m = i; m < j; m++) {
                    state[m] = Shared;
                }

                debug ("Device %d copied blocks %d to %d\n", device, i, j);

                i = j;
            }

        } else {
            // find more contiguous valid blocks
            for (k = i + 1; k < block_stop && (owner[i] == -1 || state[i] != Invalid);
                 k++)
                ;

            debug ("Device %d skipped blocks %d to %d\n", device, i, k);
            i = k;
        }
        debug ("Device %d check done until block %d\n", device, i);
    }

    // mark as owner
    if (set_owner)
        for (unsigned int i = block_start; i < block_stop; i++)
            owner[i] = device;

    debug (
      "dist2conc(dist %p, start %d, stop %d, device %d, owner? %d, stream %p) = %p\n",
      (void *)dist_array, block_start, block_stop, device, set_owner, stream, array_p);

    return array_p;
}

#endif
#endif
#endif
