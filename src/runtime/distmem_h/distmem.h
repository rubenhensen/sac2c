/*****************************************************************************
 *
 * file:   distmem.h
 *
 * prefix: SAC_DISTMEM_ (no tracing) / SAC_DISTMEM_TR_ (tracing)
 *         SAC_DISTMEM_PR_ (profiling)
 *
 * description: The header file declares the communication library independent
 *              interface of libsacdistmem. From outside libsacdistmem use
 *              the provided macros and do not call the functions directly.
 *              This way you do not need to check whether the distributed
 *              memory backend is actually used (for some functions) and
 *              whether the tracing/profiling version needs to be used.
 *
 *****************************************************************************/

#ifndef _SAC_DISTMEM_H_
#define _SAC_DISTMEM_H_

/*****************************************************************************/

#include <inttypes.h>

/* Rank of this node
 * We always declare this variable for tracing purposes. */
SAC_C_EXTERN_VAR size_t SAC_DISTMEM_rank;

/* For tracing purposes. */
#define SAC_DISTMEM_RANK_UNDEFINED SIZE_MAX

#if SAC_DO_DISTMEM

/* Master node. Only this nodes executes functions with side-effects. */
#define SAC_DISTMEM_RANK_MASTER 0

/* Minimum elements per node so that array is distributed. */
#define SAC_DISTMEM_MIN_ELEMS_PER_NODE 10

/* Type for the execution mode */
typedef enum SAC_DISTMEM_exec_mode_enum {
    SAC_DISTMEM_exec_mode_sync,        /* All nodes are executing the same code. */
    SAC_DISTMEM_exec_mode_dist,        /* Each node is executing its share of work. */
    SAC_DISTMEM_exec_mode_side_effects /* The master node is executing a function with
                                          side-effects. */
} SAC_DISTMEM_exec_mode_t;

/******************************************
 * Global variables
 *******************************************/

/* Number of nodes */
SAC_C_EXTERN_VAR size_t SAC_DISTMEM_size;

/* System page size */
SAC_C_EXTERN_VAR size_t SAC_DISTMEM_pagesz;

/* Segment size (applies to local shared segment and local
 * caches for segments owned by other nodes) */
SAC_C_EXTERN_VAR uintptr_t SAC_DISTMEM_segsz;

/* Pointer to start of local shared segment */
SAC_C_EXTERN_VAR void *SAC_DISTMEM_shared_seg_ptr;

/* Pointer to start of local caches */
SAC_C_EXTERN_VAR void *SAC_DISTMEM_cache_ptr;

/* Pointers to the local shared segment (at index SAC_DISTMEM_rank)
 * and to the local caches of segments owned by other nodes
 * (at other indices) */
SAC_C_EXTERN_VAR void **SAC_DISTMEM_local_seg_ptrs;

/* Offset to free memory within shared segment */
SAC_C_EXTERN_VAR uintptr_t SAC_DISTMEM_seg_free_offs;

/* Flag that indicates whether writes to distributed arrays are
 * currently allowed. */
SAC_C_EXTERN_VAR bool SAC_DISTMEM_are_dist_writes_allowed;

/* Flag that indicates whether writes into the local dsm cache are
 * currently allowed. */
SAC_C_EXTERN_VAR bool SAC_DISTMEM_are_cache_writes_allowed;

/* Current execution mode. */
SAC_C_EXTERN_VAR SAC_DISTMEM_exec_mode_t SAC_DISTMEM_exec_mode;

/******************************************
 * Global variables used for
 * tracing/profiling
 *******************************************/

/* Number of distributed arrays */
SAC_C_EXTERN_VAR unsigned long SAC_DISTMEM_TR_num_arrays;

/* Number of invalidated pages */
SAC_C_EXTERN_VAR unsigned long SAC_DISTMEM_TR_num_inval_pages;

/* Number of segfaults = page fetches */
SAC_C_EXTERN_VAR unsigned long SAC_DISTMEM_TR_num_segfaults;

/* Number of pointer calculations */
SAC_C_EXTERN_VAR unsigned long SAC_DISTMEM_TR_num_ptr_calcs;

/* Number of barriers */
SAC_C_EXTERN_VAR unsigned long SAC_DISTMEM_TR_num_barriers;

/******************************************
 * No tracing declarations
 *******************************************/

/** <!--********************************************************************-->
 *
 * @fn void SAC_DISTMEM_Init( int argc, char *argv[])
 *
 *   @brief   Initializes the dsm system. It cannot be used yet though.
 *
 *            Has to be called before any other operation.
 *            The dsm system cannot be used yet after this
 *            operation but other operations (e.g. parameter handling)
 *            are permitted then.
 *
 *   @param argc      argc as passed to main()
 *   @param argv      argv as passed to main()
 *
 ******************************************************************************/

void SAC_DISTMEM_Init (int argc, char *argv[]);

/** <!--********************************************************************-->
 *
 * @fn void SAC_DISTMEM_Setup( size_t maxmem_mb)
 *
 *   @brief    Sets up the dsm systemn so that it is ready for use.
 *
 *             Tries to reserve maxmem_mb MB of memory for the dsm system.
 *             If less than the requested memory is available, a warning
 *             is printed.
 *
 *   @param maxmem_mb      amount of memory to use for the dsm system in MB
 *
 ******************************************************************************/

void SAC_DISTMEM_Setup (size_t maxmem_mb);

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_InvalEntireCache( void)
 *
 *   @brief   Invalidates the entire cache.
 *
 ******************************************************************************/

void SAC_DISTMEM_InvalEntireCache (void);

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_InvalCache(uintptr_t arr_offset, size_t b)
 *
 *   @brief   Invalidates part of the cache.
 *
 ******************************************************************************/

void SAC_DISTMEM_InvalCache (uintptr_t arr_offset, size_t b);

/** <!--********************************************************************-->
 *
 * @fn void SAC_DISTMEM_Barrier( void)
 *
 *   @brief    Performs a barrier operation.
 *
 ******************************************************************************/

void SAC_DISTMEM_Barrier (void);

/** <!--********************************************************************-->
 *
 * @fn void SAC_DISTMEM_Exit( void)
 *
 *   @brief    Shuts down the dsm system.
 *
 *             It is recommended to still call exit() afterwards but do not
 *             rely on it being called.
 *
 ******************************************************************************/

void SAC_DISTMEM_Exit (void);

/** <!--********************************************************************-->
 *
 * @fn size_t SAC_DISTMEM_DetDoDistributeArray( size_t total_elems, size_t dim0_size)
 *
 *   @brief   Determines whether an array of the given size should be distributed.
 *
 *   @param total_elems   total number of array elements
 *   @param dim0_size     size of the first dimension of the array
 *   @return              TRUE if the array should be distributed, FALSE otherwise
 *
 ******************************************************************************/

bool SAC_DISTMEM_DetDoDistrArr (size_t total_elems, size_t dim0_size);

/** <!--********************************************************************-->
 *
 * @fn size_t SAC_DISTMEM_DetMaxElemsPerNode( size_t total_elems, size_t dim0_size)
 *
 *   @brief   Determines the maximum number of array elements owned by each node.
 *
 *            Determines the number of array elements owned by each of the
 *            first (SAC_DISTMEM_size - 1) nodes. The last node owns the
 *            remaining array elements.
 *
 *   @param total_elems   total number of array elements
 *   @param dim0_size     size of the first dimension of the array
 *   @return              maximum number of array elements owned by each node
 *
 ******************************************************************************/

size_t SAC_DISTMEM_DetMaxElemsPerNode (size_t total_elems, size_t dim0_size);

/** <!--********************************************************************-->
 *
 * @fn size_t SAC_DISTMEM_DetDim0Start( size_t dim0_size, size_t start_range, size_t
 *stop_range)
 *
 *   @brief   Determines the first index within the range [start_range_range, stop) that
 *is owned by this node.
 *
 *   @param dim0_size     size of the first dimension of the array
 *   @param start_range   beginning of index range
 *   @param stop_range    end of index range
 *   @return              first index owned by this node or 0 if none
 *
 ******************************************************************************/

size_t SAC_DISTMEM_DetDim0Start (size_t dim0_size, size_t start_range, size_t stop_range);

/** <!--********************************************************************-->
 *
 * @fn size_t SAC_DISTMEM_DetDim0Stop( size_t dim0_size, size_t start_range, size_t
 *stop_range)
 *
 *   @brief   Determines the last index within the range [start_range, stop_range) that is
 *owned by this node.
 *
 *   @param dim0_size     size of the first dimension of the array
 *   @param start_range   beginning of index range
 *   @param stop_range    end of index range
 *   @return              last index owned by this node + 1 or 0 if none
 *
 ******************************************************************************/

size_t SAC_DISTMEM_DetDim0Stop (size_t dim0_size, size_t start_range, size_t stop_range);

/** <!--********************************************************************-->
 *
 * @fn void *SAC_DISTMEM_Malloc( size_t num_elems, size_t elem_size, uintptr_t *offset)
 *
 *   @brief   Allocates num_elems * elems_size B of memory in the shared segment.
 *
 *            Also takes care of memory alignment.
 *
 *   @param num_elems number of elements (per node)
 *   @param elem_size element size in B
 *   @param offset    is set to the offset within the shared segment
 *   @return          pointer to allocated memory
 *
 ******************************************************************************/

void *SAC_DISTMEM_Malloc (size_t num_elems, size_t elem_size, uintptr_t *offset);

/******************************************
 * Tracing declarations
 *******************************************/

void SAC_DISTMEM_TR_Init (int argc, char *argv[]);

void SAC_DISTMEM_TR_Setup (size_t maxmem_mb);

void SAC_DISTMEM_TR_InvalEntireCache (void);

void SAC_DISTMEM_TR_InvalCache (uintptr_t arr_offset, size_t b);

void SAC_DISTMEM_TR_Barrier (void);

void SAC_DISTMEM_TR_AllowDistWrites (void);

void SAC_DISTMEM_TR_ForbidDistWrites (void);

void SAC_DISTMEM_TR_Exit (void);

bool SAC_DISTMEM_TR_DetDoDistrArr (size_t total_elems, size_t dim0_size);

size_t SAC_DISTMEM_TR_DetMaxElemsPerNode (size_t total_elems, size_t dim0_size);

size_t SAC_DISTMEM_TR_DetDim0Start (size_t dim0_size, size_t start, size_t stop);

size_t SAC_DISTMEM_TR_DetDim0Stop (size_t dim0_size, size_t start, size_t stop);

void *SAC_DISTMEM_TR_Malloc (size_t num_elems, size_t elem_size, uintptr_t *offset);

void SAC_DISTMEM_TR_IncNumPtrCalcs (void);

/******************************************
 * Profiling declarations
 *******************************************/

void SAC_DISTMEM_PR_Init (int argc, char *argv[]);

void SAC_DISTMEM_PR_Setup (size_t maxmem_mb);

void SAC_DISTMEM_PR_InvalEntireCache (void);

void SAC_DISTMEM_PR_InvalCache (uintptr_t arr_offset, size_t b);

void SAC_DISTMEM_PR_Barrier (void);

void SAC_DISTMEM_PR_AllowDistWrites (void);

void SAC_DISTMEM_PR_ForbidDistWrites (void);

void SAC_DISTMEM_PR_Exit (void);

bool SAC_DISTMEM_PR_DetDoDistrArr (size_t total_elems, size_t dim0_size);

size_t SAC_DISTMEM_PR_DetMaxElemsPerNode (size_t total_elems, size_t dim0_size);

size_t SAC_DISTMEM_PR_DetDim0Start (size_t dim0_size, size_t start, size_t stop);

size_t SAC_DISTMEM_PR_DetDim0Stop (size_t dim0_size, size_t start, size_t stop);

void *SAC_DISTMEM_PR_Malloc (size_t num_elems, size_t elem_size, uintptr_t *offset);

/******************************************
 * Functions that are implemented as
 * macros.
 *******************************************/

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_SWITCH_TO_DISTR_EXEC()
 *
 *   @brief   Switches to distributed execution mode.
 *
 *            In distributed execution mode, each node is working on its share of a
 *with-loop. If another with-loop is encountered while in distributed execution mode, it
 *will not be distributed.
 *
 ******************************************************************************/

#define SAC_DISTMEM_SWITCH_TO_DISTR_EXEC()                                               \
    SAC_TR_DISTMEM_PRINT ("Switching to distributed execution mode.");                   \
    SAC_DISTMEM_exec_mode = SAC_DISTMEM_exec_mode_dist;

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_SWITCH_TO_SYNC_EXEC()
 *
 *   @brief   Switches to synchronous execution mode.
 *
 *            In synchronous execution mode, all nodes are executing the same program in
 *parallel.
 *
 ******************************************************************************/

#define SAC_DISTMEM_SWITCH_TO_SYNC_EXEC()                                                \
    SAC_TR_DISTMEM_PRINT ("Switching to synchronous execution mode.");                   \
    SAC_DISTMEM_exec_mode = SAC_DISTMEM_exec_mode_sync;

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_SWITCH_TO_SIDE_EFFECTS_EXEC()
 *
 *   @brief   Switches to side effects execution mode.
 *
 *            In side effects execution mode, the master is executing a function
 *application with side effects. If the master encounters a with-loop while in side
 *effects execution mode, it will not be distributed. If the master encounters a barrier
 *while in side effects execution mode, it will be ignored.
 *
 ******************************************************************************/

#define SAC_DISTMEM_SWITCH_TO_SIDE_EFFECTS_EXEC()                                        \
    SAC_TR_DISTMEM_PRINT ("Switching to side effects execution mode.");                  \
    SAC_DISTMEM_exec_mode = SAC_DISTMEM_exec_mode_side_effects;

/** <!--********************************************************************-->
 *
 * @fn _SAC_DISTMEM_ELEM_POINTER( arr_offset, elem_type, elems_first_nodes, elem_index)
 *
 *   @brief   Returns a pointer to the requested array element.
 *
 *   @param arr_offset          offset of the array within the shared segment
 *   @param elem_type           element type
 *   @param elems_first_nodes   number of elements owned by all but the last node
 *   @param elem_index          index of the requested array element
 *   @return                    pointer to requested array element
 *
 ******************************************************************************/

#define _SAC_DISTMEM_ELEM_POINTER(arr_offset, elem_type, elems_first_nodes, elem_index)  \
    ((elem_type *)((uintptr_t)SAC_DISTMEM_local_seg_ptrs[elem_index / elems_first_nodes] \
                   + arr_offset)                                                         \
     + elem_index % elems_first_nodes)

#define _SAC_DISTMEM_PR_ELEM_POINTER(arr_offset, elem_type, elems_first_nodes,           \
                                     elem_index)                                         \
    (SAC_DISTMEM_TR_IncNumPtrCalcs (),                                                   \
     _SAC_DISTMEM_ELEM_POINTER (arr_offset, elem_type, elems_first_nodes, elem_index))

/*
 * We use SAC_TR_DISTMEM_AA_PRINT rather than SAC_TR_DISTMEM_EXPR because
 * we only want to print this when tracing for the distributed memory backend
 * AND tracing of array accesses is activated.
 * Tracing every pointer calculation clutters the trace output and this way it can
 * be switched off easily.
 */
#define _SAC_DISTMEM_TR_ELEM_POINTER(arr_offset, elem_type, elems_first_nodes,           \
                                     elem_index)                                         \
    ((elems_first_nodes == 0)                                                            \
       ? (SAC_RuntimeError (                                                             \
            "Invalid arguments for retrieving DSM pointer: element %zd, array offset: "  \
            "%p, elems_first_nodes: %zd",                                                \
            elem_index, arr_offset, elems_first_nodes),                                  \
          (elem_type *)NULL)                                                             \
       : (SAC_DISTMEM_TR_IncNumPtrCalcs (),                                              \
          SAC_TR_DISTMEM_AA_PRINT (                                                      \
            "Retrieving pointer for element %zd owned by node %zd, segment "             \
            "starting at: %p, offset within segment: %" PRIuPTR ", array starting "      \
            "at: %p, elems first nodes: %zd, offset within array: %zd, "                 \
            "element address: %p",                                                       \
            elem_index, elem_index / elems_first_nodes,                                  \
            (elem_type *)((                                                              \
              uintptr_t)SAC_DISTMEM_local_seg_ptrs[elem_index / elems_first_nodes]),     \
            arr_offset,                                                                  \
            (elem_type *)((uintptr_t)                                                    \
                            SAC_DISTMEM_local_seg_ptrs[elem_index / elems_first_nodes]   \
                          + arr_offset),                                                 \
            elems_first_nodes, elem_index % elems_first_nodes,                           \
            _SAC_DISTMEM_ELEM_POINTER (arr_offset, elem_type, elems_first_nodes,         \
                                       elem_index))                                      \
            _SAC_DISTMEM_ELEM_POINTER (arr_offset, elem_type, elems_first_nodes,         \
                                       elem_index)))

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_RECALC_INDEX( elems_first_nodes, elem_index)
 *
 *   @brief   Returns the last index that is owned by the same node as elem_index.
 *
 *   @param elems_first_nodes   number of elements owned by all but the last node
 *   @param elem_index          index of the requested array element
 *   @return                    last index owned by the same node as elem_index
 *
 ******************************************************************************/

#define SAC_DISTMEM_RECALC_INDEX(elems_first_nodes, elem_index)                          \
    elem_index + (elems_first_nodes - elem_index % elems_first_nodes);

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_ASSURE_IN_CACHE( arr_offset, elem_type, elems_first_nodes, elem_index,
 *num_elems)
 *
 *   @brief   Assures that num_elems elements of the given array have been loaded into the
 *local cache.
 *
 *   @param arr_offset          offset of the array within the shared segment
 *   @param elem_type           element type
 *   @param elems_first_nodes   number of elements owned by all but the last node
 *   @param elem_index          index of the first array element that is required to be in
 *the local cache
 *   @param num_elems           number of elements that are required to be in the local
 *cache
 *
 ******************************************************************************/

#define SAC_DISTMEM_ASSURE_IN_CACHE(arr_offset, elem_type, elems_first_nodes,            \
                                    elem_index, num_elems)                               \
    {                                                                                    \
        SAC_TR_DISTMEM_PRINT (                                                           \
          "Assuring that %d elements from index %d are in the local cache.", num_elems,  \
          elem_index);                                                                   \
        size_t rank;                                                                     \
        for (rank = 0; rank < SAC_DISTMEM_size; rank++) {                                \
            if (rank == SAC_DISTMEM_rank) {                                              \
                continue;                                                                \
            }                                                                            \
            size_t first_elem_at_rank = SAC_MAX (elems_first_nodes * rank, elem_index);  \
            size_t last_elem_at_rank = SAC_MIN (elems_first_nodes * (rank + 1) - 1,      \
                                                elem_index + num_elems - 1);             \
            if (last_elem_at_rank < first_elem_at_rank) {                                \
                SAC_TR_DISTMEM_PRINT ("\t No need to touch cache of node %zd.", rank);   \
                continue;                                                                \
            }                                                                            \
            SAC_TR_DISTMEM_PRINT (                                                       \
              "\t Assuring that elements %zd to %zd are in the cache of node %zd.",      \
              first_elem_at_rank, last_elem_at_rank, rank);                              \
            void *start_ptr                                                              \
              = SAC_DISTMEM_ELEM_POINTER (arr_offset, elem_type, elems_first_nodes,      \
                                          first_elem_at_rank);                           \
            void *end_ptr                                                                \
              = SAC_DISTMEM_ELEM_POINTER (arr_offset, elem_type, elems_first_nodes,      \
                                          last_elem_at_rank);                            \
            SAC_DISTMEM_CHECK_POINTER_VALID_FOR_READ (start_ptr);                        \
            SAC_DISTMEM_CHECK_POINTER_VALID_FOR_READ (end_ptr);                          \
            while ((uintptr_t)start_ptr < (uintptr_t)end_ptr) {                          \
                SAC_TR_DISTMEM_PRINT ("\t Touching %p in cache of node %zd.", start_ptr, \
                                      rank);                                             \
                *((volatile elem_type *)start_ptr);                                      \
                start_ptr = (elem_type *)((uintptr_t)start_ptr + SAC_DISTMEM_pagesz);    \
            }                                                                            \
            SAC_TR_DISTMEM_PRINT ("\t Touching %p in cache of node %zd.", end_ptr,       \
                                  rank);                                                 \
            *((volatile elem_type *)end_ptr);                                            \
        }                                                                                \
    }

/** <!--********************************************************************-->
 *
 * @fn SAC_MASTER_BROADCAST_INIT( value_type, value_NT)
 *
 *   @brief   Initializes a broadcasts of value_NT from the master node to all other
 *nodes.
 *
 *   @param value_type    type of the values
 *   @param value_NT      at the master: variable to be broadcasted
 *                        at a worker: variable where the result should be stored in
 *
 ******************************************************************************/

#define SAC_MASTER_BROADCAST_INIT(value_type, value_NT)                                  \
    uintptr_t CAT12 (NT_NAME (value_NT), __broadcast_offset);                            \
    SAC_DISTMEM_MALLOC (SAC_ND_A_SIZE (value_NT), sizeof (value_type),                   \
                        &CAT12 (NT_NAME (value_NT), __broadcast_offset));                \
    if (SAC_DISTMEM_rank == SAC_DISTMEM_RANK_MASTER) {                                   \
        for (int i = 0; i < SAC_ND_A_SIZE (value_NT); i++) {                             \
            *SAC_DISTMEM_ELEM_POINTER (CAT12 (NT_NAME (value_NT), __broadcast_offset),   \
                                       value_type, SAC_ND_A_SIZE (value_NT), i)          \
              = SAC_ND_READ (value_NT, i);                                               \
        }                                                                                \
    } else {                                                                             \
        SAC_DISTMEM_INVAL_CACHE (CAT12 (NT_NAME (value_NT), __broadcast_offset),         \
                                 SAC_ND_A_SIZE (value_NT) * sizeof (value_type));        \
    }

/** <!--********************************************************************-->
 *
 * @fn SAC_MASTER_BROADCAST_FINALIZE( value_type, value_NT)
 *
 *   @brief   Finalizes a broadcast of value_NT from the master node to all other nodes.
 *
 *            A barrier is required between SAC_MASTER_BROADCAST_INIT and this function.
 *            The barrier is not included in this function for performance reasons.
 *            If multiple values are broadcasted, a single barrier between
 *            the calls to SAC_MASTER_BROADCAST_INIT and SAC_MASTER_BROADCAST_FINALIZE
 *suffices. Accesses to the local cache are not allowed between SAC_MASTER_BROADCAST_INIT
 *and this function!
 *
 *   @param value_type    type of the values
 *   @param value_NT      at the master: variable to be broadcasted
 *                        at a worker: variable where the result should be stored in
 *
 ******************************************************************************/

#define SAC_MASTER_BROADCAST_FINALIZE(value_type, value_NT)                              \
    if (SAC_DISTMEM_rank != SAC_DISTMEM_RANK_MASTER) {                                   \
        for (int i = 0; i < SAC_ND_A_SIZE (value_NT); i++) {                             \
            SAC_ND_WRITE (value_NT, i)                                                   \
              = *SAC_DISTMEM_ELEM_POINTER (CAT12 (NT_NAME (value_NT),                    \
                                                  __broadcast_offset),                   \
                                           value_type, SAC_ND_A_SIZE (value_NT), i);     \
        }                                                                                \
    }                                                                                    \
    /* TODO: Free memory! This would require a barrier too. */

/******************************************
 * Runtime check macros
 *******************************************/

#if SAC_DO_CHECK_DISTMEM

/** <!--********************************************************************-->
 *
 * @fn void SAC_DISTMEM_ALLOW_DIST_WRITES( void)
 *
 *   @brief    Allows write accesses to distributed arrays (but not directly into the
 *local cache).
 *
 *             If the distributed memory runtime checks are active,
 *             also sets the SAC_DISTMEM_are_dist_writes_allowed flag to TRUE.
 *
 ******************************************************************************/

#define SAC_DISTMEM_ALLOW_DIST_WRITES()                                                  \
    {                                                                                    \
        SAC_TR_DISTMEM_PRINT ("Allowing write accesses to distributed arrays.");         \
        SAC_DISTMEM_are_dist_writes_allowed = TRUE;                                      \
    }

/** <!--********************************************************************-->
 *
 * @fn void SAC_DISTMEM_FORBID_DIST_WRITES( void)
 *
 *   @brief    Forbids write accesses to distributed arrays (but not directly into the
 *local cache).
 *
 *             If the distributed memory runtime checks are active,
 *             also sets the SAC_DISTMEM_are_dist_writes_allowed flag to FALSE.
 *
 ******************************************************************************/

#define SAC_DISTMEM_FORBID_DIST_WRITES()                                                 \
    {                                                                                    \
        SAC_TR_DISTMEM_PRINT ("Forbidding write accesses to distributed arrays.");       \
        SAC_DISTMEM_are_dist_writes_allowed = FALSE;                                     \
    }

/** <!--********************************************************************-->
 *
 * @fn void SAC_DISTMEM_ALLOW_CACHE_WRITES( void)
 *
 *   @brief    Allows write accesses to distributed arrays (also directly into the local
 *cache).
 *
 *             In any case, performs a barrier operation.
 *             If the distributed memory runtime checks are active,
 *             also sets the SAC_DISTMEM_are_dist_writes_allowed and
 *             SAC_DISTMEM_are_cache_writes_allowed flags to TRUE.
 *
 ******************************************************************************/

#define SAC_DISTMEM_ALLOW_CACHE_WRITES()                                                 \
    {                                                                                    \
        SAC_TR_DISTMEM_PRINT ("Allowing write accesses to distributed "                  \
                              "arrays (also directly into the local cache).");           \
        SAC_DISTMEM_are_dist_writes_allowed = TRUE;                                      \
        SAC_DISTMEM_are_cache_writes_allowed = TRUE;                                     \
    }

/** <!--********************************************************************-->
 *
 * @fn void SAC_DISTMEM_FORBID_CACHE_WRITES( void)
 *
 *   @brief    Forbids write accesses to distributed arrays (also directly into the local
 *cache).
 *
 *             In any case, performs a barrier operation.
 *             If the distributed memory runtime checks are active,
 *             also sets the SAC_DISTMEM_are_dist_writes_allowed and
 *             SAC_DISTMEM_are_cache_writes_allowed flags to FALSE.
 *
 ******************************************************************************/

#define SAC_DISTMEM_FORBID_CACHE_WRITES()                                                \
    {                                                                                    \
        SAC_TR_DISTMEM_PRINT ("Forbidding write accesses to distributed "                \
                              "arrays (also directly into the local cache");             \
        SAC_DISTMEM_are_dist_writes_allowed = FALSE;                                     \
        SAC_DISTMEM_are_cache_writes_allowed = FALSE;                                    \
    }

/** <!--********************************************************************-->
 *
 * @fn void SAC_DISTMEM_IS_VALID_WRITE_PTR( ptr)
 *
 *   @brief    Determines whether ptr is a valid pointer for a write access to a
 *distributed array.
 *
 *             The pointer is valid for a write access if it lies within the local
 *             shared segment (not within the local cache).
 *
 *             Performs the following checks:
 *              - ptr must not lie before the begin of the local shared segment.
 *              - ptr must not lie after the end of the currently allocated area
 *                within the local shared segment.
 *
 *   @return        TRUE if ptr is valid, FALSE otherwise
 *
 ******************************************************************************/

#define SAC_DISTMEM_IS_VALID_WRITE_PTR(ptr)                                              \
    (((uintptr_t)ptr < (uintptr_t)SAC_DISTMEM_shared_seg_ptr                             \
      || (uintptr_t)ptr                                                                  \
           > (uintptr_t)SAC_DISTMEM_shared_seg_ptr + SAC_DISTMEM_seg_free_offs)          \
       ? FALSE                                                                           \
       : TRUE)

/** <!--********************************************************************-->
 *
 * @fn void SAC_DISTMEM_IS_VALID_CACHE_PTR( ptr)
 *
 *   @brief    Determines whether ptr is a valid pointer within the local dsm cache.
 *
 *             Performs the following checks:
 *              - ptr must not lie before the begin of the overall cache memory segment.
 *              - ptr must not lie after the end of the overall cache memory segment.
 *              - ptr must not lie after the end of the currently allocated area within
 *                a node's segment.
 *
 *   @return        TRUE if ptr is valid, FALSE otherwise
 *
 ******************************************************************************/

#define SAC_DISTMEM_IS_VALID_CACHE_PTR(ptr)                                              \
    (((uintptr_t)ptr < (uintptr_t)SAC_DISTMEM_cache_ptr                                  \
      || (uintptr_t)ptr > (uintptr_t)SAC_DISTMEM_cache_ptr                               \
                            + SAC_DISTMEM_segsz * (SAC_DISTMEM_size - 1)                 \
      || ((uintptr_t)ptr - (uintptr_t)SAC_DISTMEM_cache_ptr))                            \
           % SAC_DISTMEM_segsz                                                           \
         > SAC_DISTMEM_seg_free_offs                                                     \
       ? FALSE                                                                           \
       : TRUE)

/** <!--********************************************************************-->
 *
 * @fn void SAC_DISTMEM_IS_VALID_READ_PTR( ptr)
 *
 *   @brief    Determines whether ptr is a valid pointer for a read access to a
 *distributed array.
 *
 *             The pointer is valid for reading if it either lies within the (currently
 *allocated portion of) the local dsm cache or it lies within the (currently allocated
 *portion of) the local shared segment.
 *
 *   @return        TRUE if ptr is valid, FALSE otherwise
 *
 ******************************************************************************/

#define SAC_DISTMEM_IS_VALID_READ_PTR(ptr)                                               \
    ((SAC_DISTMEM_IS_VALID_WRITE_PTR (ptr) || SAC_DISTMEM_IS_VALID_CACHE_PTR (ptr))      \
       ? TRUE                                                                            \
       : FALSE)

#else /* SAC_DO_CHECK_DISTMEM */

#define SAC_DISTMEM_ALLOW_DIST_WRITES()

#define SAC_DISTMEM_FORBID_DIST_WRITES()

#define SAC_DISTMEM_ALLOW_CACHE_WRITES()

#define SAC_DISTMEM_FORBID_CACHE_WRITES()

#endif /* SAC_DO_CHECK_DISTMEM */

/******************************************
 * Macros to hide trace settings
 *******************************************/

#if SAC_DO_TRACE_DISTMEM

#define SAC_DISTMEM_INIT() SAC_DISTMEM_TR_Init (__argc, __argv);

#define SAC_DISTMEM_SETUP() SAC_DISTMEM_TR_Setup (SAC_SET_DISTMEM_MAX_MEMORY_MB);

#define SAC_DISTMEM_INVAL_ENTIRE_CACHE() SAC_DISTMEM_TR_InvalEntireCache ();

#define SAC_DISTMEM_INVAL_CACHE(arr_offset, b) SAC_DISTMEM_TR_InvalCache (arr_offset, b);

#define SAC_DISTMEM_BARRIER() SAC_DISTMEM_TR_Barrier ();

#define SAC_DISTMEM_EXIT() SAC_DISTMEM_TR_Exit ();

#define SAC_DISTMEM_DET_DO_DISTR_ARR(total_elems, dim0_size)                             \
    SAC_DISTMEM_TR_DetDoDistrArr (total_elems, dim0_size);

#define SAC_DISTMEM_DET_MAX_ELEMS_PER_NODE(total_elems, dim0_size)                       \
    SAC_DISTMEM_TR_DetMaxElemsPerNode (total_elems, dim0_size);

#define SAC_DISTEM_DET_DIM0_START(dim0_size, start_range, stop_range)                    \
    SAC_DISTMEM_TR_DetDim0Start (dim0_size, start_range, stop_range);

#define SAC_DISTEM_DET_DIM0_STOP(dim0_size, start, stop)                                 \
    SAC_DISTMEM_TR_DetDim0Stop (dim0_size, start, stop);

#define SAC_DISTMEM_MALLOC(num_elems, elem_size, offset)                                 \
    SAC_DISTMEM_TR_Malloc (num_elems, elem_size, offset);

#define SAC_DISTMEM_ELEM_POINTER(arr_offset, elem_type, elems_first_nodes, elem_index)   \
    _SAC_DISTMEM_TR_ELEM_POINTER (arr_offset, elem_type, elems_first_nodes, elem_index)

#elif SAC_DO_PROFILE_DISTMEM

#define SAC_DISTMEM_INIT() SAC_DISTMEM_PR_Init (__argc, __argv);

#define SAC_DISTMEM_SETUP() SAC_DISTMEM_PR_Setup (SAC_SET_DISTMEM_MAX_MEMORY_MB);

#define SAC_DISTMEM_INVAL_ENTIRE_CACHE() SAC_DISTMEM_PR_InvalEntireCache ();

#define SAC_DISTMEM_INVAL_CACHE(arr_offset, b) SAC_DISTMEM_PR_InvalCache (arr_offset, b);

#define SAC_DISTMEM_BARRIER() SAC_DISTMEM_PR_Barrier ();

#define SAC_DISTMEM_EXIT() SAC_DISTMEM_PR_Exit ();

#define SAC_DISTMEM_DET_DO_DISTR_ARR(total_elems, dim0_size)                             \
    SAC_DISTMEM_PR_DetDoDistrArr (total_elems, dim0_size);

#define SAC_DISTMEM_DET_MAX_ELEMS_PER_NODE(total_elems, dim0_size)                       \
    SAC_DISTMEM_PR_DetMaxElemsPerNode (total_elems, dim0_size);

#define SAC_DISTEM_DET_DIM0_START(dim0_size, start_range, stop_range)                    \
    SAC_DISTMEM_PR_DetDim0Start (dim0_size, start_range, stop_range);

#define SAC_DISTEM_DET_DIM0_STOP(dim0_size, start, stop)                                 \
    SAC_DISTMEM_PR_DetDim0Stop (dim0_size, start, stop);

#define SAC_DISTMEM_MALLOC(num_elems, elem_size, offset)                                 \
    SAC_DISTMEM_PR_Malloc (num_elems, elem_size, offset);

#define SAC_DISTMEM_ELEM_POINTER(arr_offset, elem_type, elems_first_nodes, elem_index)   \
    _SAC_DISTMEM_PR_ELEM_POINTER (arr_offset, elem_type, elems_first_nodes, elem_index)

#else

#define SAC_DISTMEM_INIT() SAC_DISTMEM_Init (__argc, __argv);

#define SAC_DISTMEM_SETUP() SAC_DISTMEM_Setup (SAC_SET_DISTMEM_MAX_MEMORY_MB);

#define SAC_DISTMEM_INVAL_ENTIRE_CACHE() SAC_DISTMEM_InvalEntireCache ();

#define SAC_DISTMEM_INVAL_CACHE(arr_offset, b) SAC_DISTMEM_InvalCache (arr_offset, b);

#define SAC_DISTMEM_BARRIER() SAC_DISTMEM_Barrier ();

#define SAC_DISTMEM_EXIT() SAC_DISTMEM_Exit ();

#define SAC_DISTMEM_DET_DO_DISTR_ARR(total_elems, dim0_size)                             \
    SAC_DISTMEM_DetDoDistrArr (total_elems, dim0_size);

#define SAC_DISTMEM_DET_MAX_ELEMS_PER_NODE(total_elems, dim0_size)                       \
    SAC_DISTMEM_DetMaxElemsPerNode (total_elems, dim0_size);

#define SAC_DISTEM_DET_DIM0_START(dim0_size, start_range, stop_range)                    \
    SAC_DISTMEM_DetDim0Start (dim0_size, start_range, stop_range);

#define SAC_DISTEM_DET_DIM0_STOP(dim0_size, start_range, stop_range)                     \
    SAC_DISTMEM_DetDim0Stop (dim0_size, start_range, stop_range);

#define SAC_DISTMEM_MALLOC(num_elems, elem_size, offset)                                 \
    SAC_DISTMEM_Malloc (num_elems, elem_size, offset);

#define SAC_DISTMEM_ELEM_POINTER(arr_offset, elem_type, elems_first_nodes, elem_index)   \
    _SAC_DISTMEM_ELEM_POINTER (arr_offset, elem_type, elems_first_nodes, elem_index)

#endif

#else /* SAC_DO_DISTMEM */

/******************************************
 * Dummy definitions for when the
 * distributed memory backend is not used.
 *******************************************/

#define SAC_DISTMEM_INIT()

#define SAC_DISTMEM_SETUP()

#define SAC_DISTMEM_EXIT()

#if 0

// TODO: verify sequential works and remove

/* TODO: We probably don't need this when the distmem backend is not used. */
#define SAC_DISTMEM_INVAL_ENTIRE_CACHE()

/* TODO: We probably don't need this when the distmem backend is not used. */
#define SAC_DISTMEM_INVAL_CACHE(arr_offset, b)

/* TODO: We probably don't need this when the distmem backend is not used. */
#define SAC_DISTMEM_BARRIER()

/* TODO: Prelude needs this. Really? */
#define SAC_DISTMEM_ELEM_POINTER(arr_offset, elem_type, elems_first_nodes, elem_index) 0

#endif

/*
 * We do not define the other macros because they
 * should never be called when the distributed memory
 * backend is not used.
 */

#endif /* SAC_DO_DISTMEM */

#endif /* _SAC_DISTMEM_H */
