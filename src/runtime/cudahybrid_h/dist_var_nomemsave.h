#ifndef _SAC_DIST_VAR_NO_MEMSAVE_H_
#define _SAC_DIST_VAR_NO_MEMSAVE_H_

#ifdef __CUDACC__

#ifndef DIST_MEMSAVE

/*
 * dist var stuff
 */

enum block_state { Invalid = 0, Shared = 1, Owned = 2 };

typedef struct dist_var {
    int n_blocks;
    size_t block_size;
    block_state *state;
    int *owner;
    char **allocations; // pointers in bytes
} * dist_var_t;

unsigned int SAC_MT_DEVICES;
unsigned int SAC_CUDA_DEVICES;

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
dist_var_t conc2dist (dist_var_t dist_array, int block_start, int block_stop, int device);

// assigns section of distributed array to device, returns pointer for section
void *dist2conc (dist_var_t dist_array, int block_start, int block_stop, int device,
                 bool set_owner, cudaStream_t *stream);

/* others */
// return index of last block between [block_start->block_stop] with same state as
// block_start
unsigned int dist_end_contiguous_block (dist_var_t dist_array, int block_start,
                                        int block_stop, int device);

#ifdef DEBUG_DIST
#define debug_dist(s, ...) SAC_Print (s, ##__VA_ARGS__)
#else
#define debug_dist(s, ...)
#endif

#define checkBounds()                                                                    \
    if (block_start >= dist_array->n_blocks) {                                           \
        block_start = dist_array->n_blocks - 1;                                          \
    } else if (block_start < 0) {                                                        \
        block_start = 0;                                                                 \
    }                                                                                    \
    if (block_stop > (dist_array->n_blocks)) {                                           \
        block_stop = dist_array->n_blocks;                                               \
    }

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
    result->allocations = (char **)calloc (SAC_MT_DEVICES, sizeof (char *));
    result->state
      = (block_state *)calloc (SAC_MT_DEVICES * n_blocks, sizeof (block_state));

    debug_dist ("Allocating host-side array\n");
#ifdef DEBUG_DIST
    result->allocations[0] = (char *)calloc (total_size, unit_size);
#else
    result->allocations[0] = (char *)malloc (total_size * unit_size);
#endif

    debug_dist ("New dist var with %d blocks of size: %zu\n", n_blocks,
                result->block_size);

    return result;
}

dist_var_t
dist_var_free (dist_var_t dist_array)
{
    debug_dist ("Freeing dist_array\n");

    free (dist_array->owner);
    free (dist_array->allocations[0]);
    for (unsigned int i = 1; i < SAC_MT_DEVICES; i++) {
        if (dist_array->allocations[i] != NULL)
            cache_free (dist_array->allocations[i], i - 1);
    }
    free (dist_array->allocations);
    free (dist_array->state);
    free (dist_array);

    return dist_array;
}

/*type conversions*/
dist_var_t
conc2dist (dist_var_t dist_array, int block_start, int block_stop, int device)
{
    debug_dist ("conc2dist(dist %p, start %d, stop %d, device %d)\n", dist_array,
                block_start, block_stop, device);

    /* cast block_state array to matrix */
    block_state (*state)[SAC_MT_DEVICES][dist_array->n_blocks]
      = (block_state (*)[SAC_MT_DEVICES][dist_array->n_blocks]) (dist_array->state);
    int i;

    checkBounds ();

    for (i = 0; i < device; i++) {
        for (int j = block_start; j < block_stop; j++) {
            (*state)[i][j] = Invalid;
        }
    }
    for (int j = block_start; j < block_stop; j++) {
        (*state)[device][j] = Owned;
    }
    for (i = device + 1; i < (int)SAC_MT_DEVICES; i++) {
        for (int j = block_start; j < block_stop; j++) {
            (*state)[i][j] = Invalid;
        }
    }

    debug_dist ("conc2dist(dist %p, start %d, stop %d, device %d) = %p\n", dist_array,
                block_start, block_stop, device, dist_array);

    return dist_array;
}

unsigned int
dist_end_contiguous_block (dist_var_t dist_array, int block_start, int block_stop,
                           int device)
{

    debug_dist ("dist_end_contiguous_block(dist %p, start %d, stop %d, device %d)\n",
                (void *)dist_array, block_start, block_stop, device);

    checkBounds ();

    /* cast block_state array to matrix */
    block_state (*state)[SAC_MT_DEVICES][dist_array->n_blocks]
      = (block_state (*)[SAC_MT_DEVICES][dist_array->n_blocks]) (dist_array->state);
    int res;

    // find more contiguous blocks
    if ((*state)[device][block_start] == Invalid) {
        for (res = block_start + 1; res < block_stop && (*state)[device][res] == Invalid;
             res++)
            ;

        debug_dist ("Device %d has invalid blocks from %d to %d\n", device, block_start,
                    res);
    } else {
        for (res = block_start + 1; res < block_stop && (*state)[device][res] != Invalid;
             res++)
            ;

        debug_dist ("Device %d has valid blocks from %d to %d\n", device, block_start,
                    res);
    }

    debug_dist ("dist_end_contiguous_block(dist %p, start %d, stop %d, device %d) = %d\n",
                (void *)dist_array, block_start, block_stop, device, res);
    return res;
}

void *
dist2conc (dist_var_t dist_array, int block_start, int block_stop, int device,
           bool set_owner, cudaStream_t *stream)
{

    debug_dist (
      "dist2conc(dist %p, start %d, stop %d, device %d, owner? %d, stream %p)\n",
      (void *)dist_array, block_start, block_stop, device, set_owner, stream);

    char *array_p;
    size_t block_size = dist_array->block_size;
    int *owner = dist_array->owner;
    /* cast block_state array to matrix */
    block_state (*state)[SAC_MT_DEVICES][dist_array->n_blocks]
      = (block_state (*)[SAC_MT_DEVICES][dist_array->n_blocks]) (dist_array->state);
    char **allocations = dist_array->allocations;

    checkBounds ();

    /* find/allocate destination array */

    if (allocations[device] == NULL) {
        debug_dist ("Device %d: Array not allocated\n", device);
        size_t size_alloc = block_size * dist_array->n_blocks;
        // allocate
        if (device == 0) { /* host initialization */
            debug_dist ("Device %d: allocating host-side array\n", device);
            array_p = (char *)malloc (size_alloc);
        } else { /* cuda allocation */
            debug_dist ("Device %d: allocating cuda array\n", device);
            array_p = (char *)cache_malloc (size_alloc, device - 1);
        }
        allocations[device] = array_p;
        debug_dist ("Device %d: Will store from %p to %p\n", device, array_p,
                    array_p + size_alloc);
    } else {
        array_p = allocations[device];
        debug_dist ("Device %d: Will store at %p\n", device, array_p);
    }

    // for each block requested:
    for (int i = block_start; i < block_stop;) {
        int k;

        debug_dist ("Device %d checking block %d: ", device, i);
        debug_dist ("local state %d, ", (*state)[device][i]);
        debug_dist ("owner %d\n", owner[i]);
        // if the block is not in the destination array
        if ((*state)[device][i] == Invalid) {
            char *source_p, *destination_p;
            int source = owner[i];

            // find more contiguous not valid blocks
            for (k = i + 1;
                 k < block_stop && (*state)[device][k] == Invalid && owner[k] == source;
                 k++)
                ;

            if (source > -1) {

                debug_dist ("Device %d Missing blocks %d to %d, will copy from %d\n",
                            device, i, k, source);

                source_p = allocations[source];

                size_t offset = i * block_size;
                size_t size = (k - i) * block_size;
                source_p += offset;
                destination_p = array_p + offset;

                debug_dist ("Device %d: offset %zu, size %zu\n", device, offset, size);
                if (device == 0) { /* cuda to host */
                    debug_dist ("Device %d: copying from cuda %p to host %p\n", device,
                                source_p, destination_p);
                    cudaSetDevice (source - 1);
                    cudaMemcpy (destination_p, source_p, size, cudaMemcpyDeviceToHost);
                } else if (source == 0) { /* host to cuda */
                    debug_dist ("Device %d: copying from host %p to cuda %p\n", device,
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
                    debug_dist ("Device %d: copying from cuda %p to cuda %p\n", device,
                                source_p, destination_p);
                    if (stream) {
                        cudaMemcpyPeerAsync (destination_p, device - 1, source_p,
                                             source - 1, size, *stream);
                    } else {
                        cudaMemcpyPeer (destination_p, device - 1, source_p, source - 1,
                                        size);
                    }
                }

                // mark_blocks
                for (int m = i; m < k; m++) {
                    (*state)[device][m] = Shared;
                }

                debug_dist ("Device %d copied blocks %d to %d\n", device, i, k);
            } else {
                debug_dist ("Device %d: blocks %d to %d not computed yet, skipping\n",
                            device, i, k);
            }

        } else {
            // find more contiguous valid blocks
            for (k = i + 1; k < block_stop && (*state)[device][k] != Invalid; k++)
                ;

            debug_dist ("Device %d skipped blocks %d to %d\n", device, i, k);
        }
        i = k;

        debug_dist ("Device %d check done until block %d\n", device, i);
    }

    // mark as owner
    if (set_owner)
        for (int i = block_start; i < block_stop; i++)
            owner[i] = device;

    debug_dist (
      "dist2conc(dist %p, start %d, stop %d, device %d, owner? %d, stream %p) = %p\n",
      (void *)dist_array, block_start, block_stop, device, set_owner, stream, array_p);

    return array_p;
}

#endif
#endif
#endif
