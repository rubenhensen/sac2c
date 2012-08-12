#ifndef _SAC_DIST_VAR_NO_MEMSAVE_H_
#define _SAC_DIST_VAR_NO_MEMSAVE_H_

#ifdef __CUDACC__

#ifndef DIST_MEMSAVE

/*
 * dist var stuff
 */

enum block_state { Invalid = 0, Shared = 1, Owned = 2 };

typedef struct dist_var {
    unsigned int n_blocks;
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
dist_var_t host2dist_st (dist_var_t dist_array, void *array);
dist_var_t host2dist_spmd (dist_var_t dist_array, void *array);
dist_var_t dev2dist_spmd (dist_var_t dist_array, void *array, int device);

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

    debug ("Allocating host-side array\n");
#ifdef DEBUG_DIST
    result->allocations[0] = (char *)calloc (total_size, unit_size);
#else
    result->allocations[0] = (char *)malloc (total_size * unit_size);
#endif

    debug ("New dist var with %d blocks of size: %zu\n", n_blocks, result->block_size);

    return result;
}

dist_var_t
dist_var_free (dist_var_t dist_array)
{
    debug ("Freeing dist_array\n");

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
host2dist_st (dist_var_t dist_array, void *array)
{
    debug ("host2dist_st(dist %p, array %p)\n", dist_array, array);

    /* cast block_state array to matrix */
    block_state (*state)[SAC_MT_DEVICES][dist_array->n_blocks]
      = (block_state (*)[SAC_MT_DEVICES][dist_array->n_blocks]) (dist_array->state);

    for (int i = 0; i < (int)SAC_MT_DEVICES; i++) {
        for (unsigned int j = 0; j < dist_array->n_blocks; j++) {
            if (dist_array->owner[j] == i)
                (*state)[i][j] = Owned;
            else
                (*state)[i][j] = Invalid;
        }
    }

    debug ("host2dist_st(dist %p, array %p) = %p\n", dist_array, array, dist_array);

    return dist_array;
}

dist_var_t
host2dist_spmd (dist_var_t dist_array, void *array)
{
    debug ("host2dist_spmd(dist %p)\n", dist_array);

    /* cast block_state array to matrix */
    block_state (*state)[SAC_MT_DEVICES][dist_array->n_blocks]
      = (block_state (*)[SAC_MT_DEVICES][dist_array->n_blocks]) (dist_array->state);
    unsigned int i;

    for (i = 0; i < dist_array->n_blocks; i++) {
        if (dist_array->owner[i] == 0)
            (*state)[0][i] = Owned;
        else
            (*state)[0][i] = Invalid;
    }

    debug ("host2dist_spmd(dist %p) = %p\n", dist_array);

    return dist_array;
}

dist_var_t
dev2dist_spmd (dist_var_t dist_array, void *array, int device)
{
    debug ("dev2dist_spmd(dist %p, array %p, device %d)\n", dist_array, array, device);

    /* cast block_state array to matrix */
    block_state (*state)[SAC_MT_DEVICES][dist_array->n_blocks]
      = (block_state (*)[SAC_MT_DEVICES][dist_array->n_blocks]) (dist_array->state);
    unsigned int i;

    for (i = 0; i < dist_array->n_blocks; i++) {
        if (dist_array->owner[i] == device)
            (*state)[device][i] = Owned;
        else
            (*state)[device][i] = Invalid;
    }

    debug ("dev2dist_spmd(dist %p, array %p, device %d) = %p\n", dist_array, array,
           device, dist_array);

    return dist_array;
}

unsigned int
dist_end_contiguous_block (dist_var_t dist_array, unsigned int block_start,
                           unsigned int block_stop, int device)
{

    debug ("dist_end_contiguous_block(dist %p, start %d, stop %d, device %d)\n",
           (void *)dist_array, block_start, block_stop, device);

    checkBounds

      /* cast block_state array to matrix */
      block_state (*state)[SAC_MT_DEVICES][dist_array->n_blocks]
      = (block_state (*)[SAC_MT_DEVICES][dist_array->n_blocks]) (dist_array->state);
    unsigned int res;

    // find more contiguous blocks
    if ((*state)[device][block_start] == Invalid) {
        for (res = block_start + 1; res < block_stop && (*state)[device][res] == Invalid;
             res++)
            ;

        debug ("Device %d has invalid blocks from %d to %d\n", device, block_start, res);
    } else {
        for (res = block_start + 1; res < block_stop && (*state)[device][res] != Invalid;
             res++)
            ;

        debug ("Device %d has valid blocks from %d to %d\n", device, block_start, res);
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
    /* cast block_state array to matrix */
    block_state (*state)[SAC_MT_DEVICES][dist_array->n_blocks]
      = (block_state (*)[SAC_MT_DEVICES][dist_array->n_blocks]) (dist_array->state);
    char **allocations = dist_array->allocations;

    checkBounds

      /* find/allocate destination array */

      if (allocations[device] == NULL)
    {
        debug ("Device %d: Array not allocated\n", device);
        size_t size_alloc = block_size * dist_array->n_blocks;
        // allocate
        if (device == 0) { /* host initialization */
            debug ("Device %d: allocating host-side array\n", device);
            array_p = (char *)malloc (size_alloc);
        } else { /* cuda allocation */
            debug ("Device %d: allocating cuda array\n", device);
            array_p = (char *)cache_malloc (size_alloc, device - 1);
        }
        allocations[device] = array_p;
    }
    else
    {
        array_p = allocations[device];
    }
    debug ("Device %d: Will store at %p\n", device, array_p);

    // for each block requested:
    for (unsigned int i = block_start; i < block_stop;) {
        unsigned int k;

        debug ("Device %d checking block %d: ", device, i);
        debug ("local state %d, ", (*state)[device][i]);
        debug ("owner %d\n", owner[i]);
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

                debug ("Device %d Missing blocks %d to %d, will copy from %d\n", device,
                       i, k, source);

                source_p = allocations[source];

                size_t offset = i * block_size;
                size_t size = (k - i) * block_size;
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

                // mark_blocks
                for (unsigned int m = i; m < k; m++) {
                    (*state)[device][m] = Shared;
                }

                debug ("Device %d copied blocks %d to %d\n", device, i, k);
            } else {
                debug ("Device %d: blocks %d to %d not computed yet, skipping\n", device,
                       i, k);
            }

        } else {
            // find more contiguous valid blocks
            for (k = i + 1; k < block_stop && (*state)[device][k] != Invalid; k++)
                ;

            debug ("Device %d skipped blocks %d to %d\n", device, i, k);
        }
        i = k;

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
