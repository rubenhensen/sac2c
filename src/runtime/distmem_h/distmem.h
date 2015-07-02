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

/* If not equal to SAC_DISTMEM_TRACE_PROFILE_RANK_ANY, only produce
 * trace output for this rank.
 * We always declare this variable for tracing purposes. */
SAC_C_EXTERN_VAR int SAC_DISTMEM_trace_profile_rank;

/** <!--********************************************************************-->
 *
 * @fn void SAC_DISTMEM_Exit( int exit_code)
 *
 *   @brief    Shuts down the dsm system.
 *
 *             It is recommended to still call exit() afterwards but do not
 *             rely on it being called.
 *             This is also declared when the distributed memory backend is not
 *             used so that it can be used from SAC_RuntimeError.
 *
 *  @param exit_code    exit code
 *
 ******************************************************************************/

void SAC_DISTMEM_Exit (int exit_code);

/* For tracing purposes we define the following macros also when
 * the distributed memory backend is not used. */

#define SAC_DISTMEM_RANK_UNDEFINED SIZE_MAX
#define SAC_DISTMEM_TRACE_PROFILE_RANK_ANY -1

/* Master node. Only this nodes executes functions with side-effects. */
#define SAC_DISTMEM_RANK_MASTER 0

#if SAC_DO_DISTMEM

/*
 * We need the below extern declarations of memset here rather than including
 * the corresponding header files because the further declarations in
 * string.h conflict with SAC-generated headers in the SAC string module.
 *
 * The check for a previous definition of a macro of equal name are required
 * for operating systems that try to overload memset or memcpy, e.g. MAC OS.
 *
 * These extern declarations were copied from mt.h because they are also needed
 * for the distributed memory backend and the distributed memory backend is
 * currently not compatible with multi-threading.
 */

#ifndef memset
extern void *memset (void *s, int c, size_t n);
#endif

#ifndef memcpy
extern void *memcpy (void *dest, const void *src, size_t n);
#endif

/* Type for the execution mode */
typedef enum SAC_DISTMEM_exec_mode_enum {
    SAC_DISTMEM_exec_mode_sync, /* All nodes are executing the same code. TODO: Rename to
                                   replicated */
    SAC_DISTMEM_exec_mode_dist, /* Each node is executing its share of work. */
    SAC_DISTMEM_exec_mode_side_effects_outer, /* The master node is executing the
                                                 most-outer function with side-effects. */
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

/* Flag that indicates whether allocations in the DSM segment are
 * currently allowed. */
SAC_C_EXTERN_VAR bool SAC_DISTMEM_are_dsm_allocs_allowed;

/* Flag that indicates whether writes to distributed arrays are
 * currently allowed. */
SAC_C_EXTERN_VAR bool SAC_DISTMEM_are_dist_writes_allowed;

/* Flag that indicates whether writes into the local dsm cache are
 * currently allowed. */
SAC_C_EXTERN_VAR bool SAC_DISTMEM_are_cache_writes_allowed;

/* Current execution mode. */
SAC_C_EXTERN_VAR SAC_DISTMEM_exec_mode_t SAC_DISTMEM_exec_mode;

/* Minimum number of array elements per node such that an array gets distributed */
SAC_C_EXTERN_VAR size_t SAC_DISTMEM_min_elems_per_node;

/******************************************
 * Global variables used for
 * tracing/profiling
 *******************************************/

/* Number of invalidated pages */
SAC_C_EXTERN_VAR unsigned long SAC_DISTMEM_TR_num_inval_pages;

/* Number of segfaults = page fetches */
SAC_C_EXTERN_VAR unsigned long SAC_DISTMEM_TR_num_segfaults;

/* Number of pointer calculations */
SAC_C_EXTERN_VAR unsigned long SAC_DISTMEM_TR_num_ptr_calcs;

/* Number of avoided pointer calculations for local writes */
SAC_C_EXTERN_VAR unsigned long SAC_DISTMEM_TR_num_avoided_ptr_calcs_local_writes;

/* Number of avoided pointer calculations for local reads */
SAC_C_EXTERN_VAR unsigned long SAC_DISTMEM_TR_num_avoided_ptr_calcs_local_reads;

/* Number of avoided pointer calculations for remote reads */
SAC_C_EXTERN_VAR unsigned long SAC_DISTMEM_TR_num_avoided_ptr_calcs_remote_reads;

/* Number of barriers */
SAC_C_EXTERN_VAR unsigned long SAC_DISTMEM_TR_num_barriers;

/******************************************
 * Global variables used for
 * runtime checks
 *******************************************/

/* Upper limit (non-inclusive) for valid cache pointers. */
SAC_C_EXTERN_VAR uintptr_t SAC_DISTMEM_CH_max_valid_cache_ptr;

/* Upper limit (non-inclusive) for valid pointers into the local shared segment. */
SAC_C_EXTERN_VAR uintptr_t SAC_DISTMEM_CH_max_valid_write_ptr;

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
 * @fn void SAC_DISTMEM_Setup( size_t maxmem_mb, size_t min_elems_per_node, int
 *trace_profile_rank)
 *
 *   @brief    Sets up the dsm systemn so that it is ready for use.
 *
 *             Tries to reserve maxmem_mb MB of memory for the dsm system.
 *             If less than the requested memory is available, a warning
 *             is printed.
 *             Initializes the private heap manager.
 *
 *   @param maxmem_mb             amount of memory to use for the dsm system in MB
 *   @param min_elems_per_node    minimum number of elements per node such that
 *                                an array gets distributed
 *   @param trace_profile_rank    If not equal to SAC_DISTMEM_TRACE_PROFILE_RANK_ANY, only
 *                                produce trace output for the provided rank.
 *
 ******************************************************************************/

void SAC_DISTMEM_Setup (size_t maxmem_mb, size_t min_elems_per_node,
                        int trace_profile_rank);

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
 * @fn SAC_DISTMEM_InvalCacheOfNode(uintptr_t arr_offset, size_t node, size_t b)
 *
 *   @brief   Invalidates part of the cache of a specific node.
 *
 ******************************************************************************/

void SAC_DISTMEM_InvalCacheOfNode (uintptr_t arr_offset, size_t node, size_t b);

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

/******************************************
 * Tracing declarations
 *******************************************/

void SAC_DISTMEM_TR_Init (int argc, char *argv[]);

void SAC_DISTMEM_TR_Setup (size_t maxmem_mb, size_t min_elems_per_node,
                           int trace_profile_rank);

void SAC_DISTMEM_TR_InvalEntireCache (void);

void SAC_DISTMEM_TR_InvalCache (uintptr_t arr_offset, size_t b);

void SAC_DISTMEM_TR_InvalCacheOfNode (uintptr_t arr_offset, size_t node, size_t b);

void SAC_DISTMEM_TR_Barrier (void);

void SAC_DISTMEM_TR_AllowDistWrites (void);

void SAC_DISTMEM_TR_ForbidDistWrites (void);

void SAC_DISTMEM_TR_Exit (int exit_code);

bool SAC_DISTMEM_TR_DetDoDistrArr (size_t total_elems, size_t dim0_size);

size_t SAC_DISTMEM_TR_DetMaxElemsPerNode (size_t total_elems, size_t dim0_size);

size_t SAC_DISTMEM_TR_DetDim0Start (size_t dim0_size, size_t start, size_t stop);

size_t SAC_DISTMEM_TR_DetDim0Stop (size_t dim0_size, size_t start, size_t stop);

void SAC_DISTMEM_TR_IncNumPtrCalcs (void);

/******************************************
 * Profiling declarations
 *******************************************/

void SAC_DISTMEM_PR_Init (int argc, char *argv[]);

void SAC_DISTMEM_PR_Setup (size_t maxmem_mb, size_t min_elems_per_node,
                           int trace_profile_rank);

void SAC_DISTMEM_PR_InvalEntireCache (void);

void SAC_DISTMEM_PR_InvalCache (uintptr_t arr_offset, size_t b);

void SAC_DISTMEM_PR_InvalCacheOfNode (uintptr_t arr_offset, size_t node, size_t b);

void SAC_DISTMEM_PR_Barrier (void);

void SAC_DISTMEM_PR_AllowDistWrites (void);

void SAC_DISTMEM_PR_ForbidDistWrites (void);

void SAC_DISTMEM_PR_Exit (int exit_code);

bool SAC_DISTMEM_PR_DetDoDistrArr (size_t total_elems, size_t dim0_size);

size_t SAC_DISTMEM_PR_DetMaxElemsPerNode (size_t total_elems, size_t dim0_size);

size_t SAC_DISTMEM_PR_DetDim0Start (size_t dim0_size, size_t start, size_t stop);

size_t SAC_DISTMEM_PR_DetDim0Stop (size_t dim0_size, size_t start, size_t stop);

/******************************************
 * Functions that are implemented as
 * macros.
 *******************************************/

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_DET_LOCAL_FROM( var_NT)
 *
 *   @brief   Determines the first array element that is local to this node.
 *
 *   @param  var_NT       array
 *
 *   @return              index of the first array element that is local to this node
 *
 ******************************************************************************/

#define SAC_DISTMEM_DET_LOCAL_FROM(var_NT)                                               \
    (SAC_ND_A_IS_DIST (var_NT)                                                           \
       ? (SAC_TR_DISTMEM_PRINT_EXPR ("The first local element of array %s is %d.",       \
                                     NT_STR (var_NT),                                    \
                                     SAC_ND_A_FIRST_ELEMS (var_NT) * SAC_DISTMEM_rank)   \
            SAC_ND_A_FIRST_ELEMS (var_NT)                                                \
          * SAC_DISTMEM_rank)                                                            \
       : 0)

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_DET_LOCAL_TO( var_NT)
 *
 *   @brief   Determines the last array element that is local to this node.
 *
 *   @param  var_NT       array
 *
 *   @return              index of the last array element that is local to this node
 *
 ******************************************************************************/

#define SAC_DISTMEM_DET_LOCAL_TO(var_NT)                                                 \
    (SAC_ND_A_IS_DIST (var_NT)                                                           \
       ? (SAC_TR_DISTMEM_PRINT_EXPR ("The last local element of array %s is %d.",        \
                                     NT_STR (var_NT),                                    \
                                     SAC_ND_A_FIRST_ELEMS (var_NT)                       \
                                         * (SAC_DISTMEM_rank + 1)                        \
                                       - 1) SAC_ND_A_FIRST_ELEMS (var_NT)                \
            * (SAC_DISTMEM_rank + 1)                                                     \
          - 1)                                                                           \
       : (SAC_ND_A_SIZE (var_NT) - 1))

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_RECHECK_AKS_IS_DIST( var_NT)
 *
 *   @brief   Rechecks at allocation time of an AKS array if it should still be
 *            distributed and updates the mirror accordingly.
 *
 *            This check is necessary because the execution mode may have changed
 *            since the declaration of the array.
 *
 *   @param  var_NT       the array in question
 *
 ******************************************************************************/

#define SAC_DISTMEM_RECHECK_AKS_IS_DIST(var_NT)                                          \
    SAC_TR_DISTMEM_PRINT ("Rechecking if array %s is distributed, was: %d, is: %d.",     \
                          NT_STR (var_NT), SAC_ND_A_IS_DIST (var_NT),                    \
                          SAC_ND_A_IS_DIST (var_NT)                                      \
                            && SAC_DISTMEM_exec_mode == SAC_DISTMEM_exec_mode_sync);     \
    SAC_ND_A_MIRROR_IS_DIST (var_NT)                                                     \
      = SAC_ND_A_IS_DIST (var_NT)                                                        \
        && SAC_DISTMEM_exec_mode == SAC_DISTMEM_exec_mode_sync;

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_DET_OFFS( addr)
 *
 *   @brief   Calculates the offset of an address within the shared segment of this node.
 *
 ******************************************************************************/

#define SAC_DISTMEM_DET_OFFS(addr)                                                       \
    (SAC_TR_DISTMEM_PRINT_EXPR ("Offset of address %p in shared segment is %" PRIuPTR    \
                                ".",                                                     \
                                addr,                                                    \
                                (uintptr_t)addr                                          \
                                  - (uintptr_t)SAC_DISTMEM_shared_seg_ptr) (uintptr_t)   \
       addr                                                                              \
     - (uintptr_t)SAC_DISTMEM_shared_seg_ptr)

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_MOVE_PTR_FOR_WRITES( ptr)
 *
 *   @brief   Moves the pointer of a distributed array so that it can be treated like a
 *            pointer to a non-DSM array for write accesses.
 *
 *   @param  ptr          pointer to the start of the local portion of the array
 *   @param  first_elems  number of elements all nodes but the last node own
 *
 ******************************************************************************/

#define SAC_DISTMEM_MOVE_PTR_FOR_WRITES(addr, first_elems)                               \
    SAC_TR_DISTMEM_PRINT ("Moving pointer from %p to %p (first elems: %zd).", addr,      \
                          addr - first_elems * SAC_DISTMEM_rank, first_elems);           \
    addr -= first_elems * SAC_DISTMEM_rank;

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_GET_PTR_FOR_FREE( ptr)
 *
 *   @brief   Undoes SAC_DISTMEM_MOVE_PTR_FOR_WRITES for free.
 *
 *   @param  ptr          pointer
 *
 *   @return              pointer to the start of the local portion of the array
 *
 ******************************************************************************/

#define SAC_DISTMEM_GET_PTR_FOR_FREE(addr, first_elems)                                  \
    (SAC_TR_DISTMEM_PRINT_EXPR (                                                         \
       "Getting pointer for free from %p: %p (first elems: %zd).", addr,                 \
       addr + first_elems * SAC_DISTMEM_rank, first_elems) addr                          \
     + first_elems * SAC_DISTMEM_rank)

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_SWITCH_TO_DIST_EXEC()
 *
 *   @brief   Switches to distributed execution mode.
 *
 *            In distributed execution mode, each node is working on its share of a
 *with-loop. If another with-loop is encountered while in distributed execution mode, it
 *will not be distributed. Allocations in the DSM segment are forbidden in distributed
 *execution mode.
 *
 ******************************************************************************/

#define SAC_DISTMEM_SWITCH_TO_DIST_EXEC()                                                \
    SAC_TR_DISTMEM_PRINT ("Switching to distributed execution mode.");                   \
    SAC_DISTMEM_CHECK_IS_SWITCH_TO_DIST_EXEC_ALLOWED ()                                  \
    SAC_PF_BEGIN_EXEC_DIST ()                                                            \
    SAC_DISTMEM_exec_mode = SAC_DISTMEM_exec_mode_dist;                                  \
    SAC_DISTMEM_FORBID_DSM_ALLOCS ();

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_SWITCH_TO_SYNC_EXEC()
 *
 *   @brief   Switches to replicated execution mode.
 *
 *            In replicated execution mode, all nodes are executing the same program in
 *parallel. If the previous execution mode was distributed, allows allocations in the DSM
 *segment again.
 *
 ******************************************************************************/

#define SAC_DISTMEM_SWITCH_TO_SYNC_EXEC()                                                \
    if (SAC_DISTMEM_exec_mode == SAC_DISTMEM_exec_mode_dist) {                           \
        SAC_DISTMEM_ALLOW_DSM_ALLOCS ();                                                 \
    }                                                                                    \
    SAC_TR_DISTMEM_PRINT ("Switching to replicated execution mode.");                    \
    SAC_DISTMEM_CHECK_IS_SWITCH_TO_SYNC_EXEC_ALLOWED ()                                  \
    SAC_PF_END_EXEC_MODE ()                                                              \
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
    SAC_DISTMEM_CHECK_IS_SWITCH_TO_SIDE_EFFECTS_EXEC_ALLOWED ()                          \
    SAC_DISTMEM_exec_mode = SAC_DISTMEM_exec_mode_side_effects;

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_SWITCH_TO_SIDE_EFFECTS_OUTER_EXEC()
 *
 *   @brief   Switches to side effects outer execution mode.
 *
 *            In addition to the effects of the side effects exeuction mode,
 *            DSM variables are by default allocated in the DSM memory.
 *
 ******************************************************************************/

#define SAC_DISTMEM_SWITCH_TO_SIDE_EFFECTS_OUTER_EXEC()                                  \
    SAC_TR_DISTMEM_PRINT ("Switching to side effects outer execution mode.");            \
    SAC_DISTMEM_CHECK_IS_SWITCH_TO_SIDE_EFFECTS_OUTER_EXEC_ALLOWED ()                    \
    SAC_PF_BEGIN_EXEC_SIDE_EFFECTS ()                                                    \
    SAC_DISTMEM_exec_mode = SAC_DISTMEM_exec_mode_side_effects_outer;

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_DET_ALLOC_DSM_IN_DSM()
 *
 *   @brief   Determines whether a DSM variable needs to be allocated in DSM memory by
 *default.
 *
 *            It would be more efficient to immediately allocate variables in DSM memory
 *in outer execution mode. However, that is not possible because we need to guarantee the
 *exact same order of DSM memory allocations at all nodes.
 *
 ******************************************************************************/

#define SAC_DISTMEM_DET_ALLOC_DSM_IN_DSM() FALSE

/** <!--********************************************************************-->
 *
 * @fn _SAC_DISTMEM_ELEM_POINTER( arr_offset, elem_type, node, elem_index)
 *
 *   @brief   Returns a pointer to the requested array element assuming that the array is
 *DSM (i.e. not distributed but allocated in DSM memory).
 *
 *   @param arr_offset          offset of the array within the shared segment
 *   @param elem_type           element type
 *   @param node                node on which the element is located
 *   @param elem_index          index of the requested array element (node-local)
 *   @return                    pointer to requested array element
 *
 ******************************************************************************/

#define SAC_DISTMEM_DSM_ELEM_POINTER(arr_offset, elem_type, node, elem_index)            \
    ((elem_type *)((uintptr_t)SAC_DISTMEM_local_seg_ptrs[node] + arr_offset) + elem_index)

/** <!--********************************************************************-->
 *
 * @fn _SAC_DISTMEM_ELEM_POINTER( arr_offset, elem_type, elems_first_nodes, elem_index)
 *
 *   @brief   Returns a pointer to the requested array element assumg that the array is
 *distributed.
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
                SAC_TR_DISTMEM_PRINT ("  No need to touch cache of node %zd.", rank);    \
                continue;                                                                \
            }                                                                            \
            SAC_TR_DISTMEM_PRINT (                                                       \
              "  Assuring that elements %zd to %zd are in the cache of node %zd.",       \
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
                SAC_TR_DISTMEM_PRINT ("  Touching %p in cache of node %zd.", start_ptr,  \
                                      rank);                                             \
                *((volatile elem_type *)start_ptr);                                      \
                start_ptr = (elem_type *)((uintptr_t)start_ptr + SAC_DISTMEM_pagesz);    \
            }                                                                            \
            SAC_TR_DISTMEM_PRINT ("  Touching %p in cache of node %zd.", end_ptr, rank); \
            *((volatile elem_type *)end_ptr);                                            \
        }                                                                                \
    }

/*
 * Macros for the generation of variable names
 * for the broadcast operations.
 */

#define _BC_DIMSIZE_VAR(value_NT) CAT12 (NT_NAME (value_NT), __bc_dimsize_var)

#define _BC_DIMSIZE_OFFS(value_NT) CAT12 (NT_NAME (value_NT), __bc_dimsize_offs)

#define _BC_VAR(value_NT) CAT12 (NT_NAME (value_NT), __bc_var)

#define _BC_OFFS(value_NT) CAT12 (NT_NAME (value_NT), __bc_offs)

#define _BC_DESC_VAR(value_NT) CAT12 (NT_NAME (value_NT), __bc_desc_var)

#define _BC_DESC_OFFS(value_NT) CAT12 (NT_NAME (value_NT), __bc_desc_offs)

/** <!--********************************************************************-->
 *
 * @fn _SAC_DISTMEM_BROADCAST_WITH_DESC_INIT_COMMON( value_type, value_NT)
 *
 *   @brief   Common part of the initialization of a broadcast operation (master ->
 *workers) for both master and worker nodes.
 *
 *            This macro is for AKD and AKU arrays; the dimension, the size and the
 *descriptor are broadcast as well.
 *
 *   @param value_type    C-type of the value
 *   @param value_NT      at the master: source variable
 *                        at a worker: target variable
 *
 ******************************************************************************/

#define _SAC_DISTMEM_BROADCAST_WITH_DESC_INIT_COMMON(value_type, value_NT)               \
    int *_BC_DIMSIZE_VAR (value_NT);                                                     \
    value_type *_BC_VAR (value_NT);                                                      \
    SAC_ND_DESC_TYPE (value_NT) _BC_DESC_VAR (value_NT);                                 \
    uintptr_t _BC_DIMSIZE_OFFS (value_NT);                                               \
    uintptr_t _BC_OFFS (value_NT);                                                       \
    uintptr_t _BC_DESC_OFFS (value_NT);                                                  \
    SAC_TR_DISTMEM_PRINT (                                                               \
      "Initializing broadcast with descriptor of variable %s (type: %s).",               \
      NT_STR (value_NT), #value_type);                                                   \
    SAC_DISTMEM_HM_MALLOC (_BC_DIMSIZE_VAR (value_NT), 2 * sizeof (int), int)            \
    SAC_TR_DISTMEM_PRINT ("BC dimsize starting at: %p (offset: %" PRIuPTR ")",           \
                          _BC_DIMSIZE_VAR (value_NT), _BC_DIMSIZE_OFFS (value_NT));      \
    _BC_DIMSIZE_OFFS (value_NT) = SAC_DISTMEM_DET_OFFS (_BC_DIMSIZE_VAR (value_NT));

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_BROADCAST_WITH_DESC_INIT_MASTER( value_type, value_NT)
 *
 *   @brief   Initializes a broadcast operation (master -> workers) at the master node.
 *
 *            This macro is for AKD and AKU arrays; the dimension, the size and the
 *descriptor are broadcast as well.
 *
 *            A barrier is required after this macro. The barrier is not included in this
 *macro for performance reasons. If multiple values are broadcast, a single barrier after
 *all calls to this macro suffices.
 *
 *   @param value_type    C-type of the value
 *   @param value_NT      source variable
 *
 ******************************************************************************/

#define SAC_DISTMEM_BROADCAST_WITH_DESC_INIT_MASTER(value_type, value_NT)                \
    _SAC_DISTMEM_BROADCAST_WITH_DESC_INIT_COMMON (value_type, value_NT)                  \
    _BC_DIMSIZE_VAR (value_NT)[0] = SAC_A_DIM_BEFORE_UPDATE_MIRROR (value_NT);           \
    _BC_DIMSIZE_VAR (value_NT)[1] = SAC_ND_A_DESC_SIZE (value_NT);                       \
    SAC_TR_DISTMEM_PRINT ("BC dim: %d, size: %d", _BC_DIMSIZE_VAR (value_NT)[0],         \
                          _BC_DIMSIZE_VAR (value_NT)[1]);                                \
    SAC_DISTMEM_HM_MALLOC_FIXED_SIZE_WITH_DESC (_BC_VAR (value_NT),                      \
                                                _BC_DESC_VAR (value_NT),                 \
                                                SAC_ND_A_DESC_SIZE (value_NT)            \
                                                  * sizeof (value_type),                 \
                                                SAC_A_DIM_BEFORE_UPDATE_MIRROR (         \
                                                  value_NT),                             \
                                                value_type,                              \
                                                SAC_ND_DESC_BASETYPE (value_NT));        \
    _BC_OFFS (value_NT) = SAC_DISTMEM_DET_OFFS (_BC_VAR (value_NT));                     \
    SAC_TR_DISTMEM_PRINT ("BC data starting at: %p (offset: %" PRIuPTR ")",              \
                          _BC_VAR (value_NT), _BC_OFFS (value_NT));                      \
    _BC_DESC_OFFS (value_NT) = SAC_DISTMEM_DET_OFFS (_BC_DESC_VAR (value_NT));           \
    SAC_TR_DISTMEM_PRINT ("BC descriptor starting at: %p (offset: %" PRIuPTR ")",        \
                          _BC_DESC_VAR (value_NT), _BC_DESC_OFFS (value_NT));            \
    SAC_TR_DISTMEM_PRINT ("BC copying %d B of descriptor from %p.",                      \
                          BYTE_SIZE_OF_DESC (SAC_ND_A_DIM (value_NT)),                   \
                          _BC_DESC_VAR (value_NT));                                      \
    memcpy (_BC_DESC_VAR (value_NT), SAC_ND_A_DESC (value_NT),                           \
            BYTE_SIZE_OF_DESC (SAC_A_DIM_BEFORE_UPDATE_MIRROR (value_NT)));              \
    SAC_TR_DISTMEM_PRINT ("BC copying %d B of data from %p.",                            \
                          SAC_ND_A_SIZE (value_NT) * sizeof (value_type),                \
                          SAC_ND_A_FIELD (value_NT));                                    \
    memcpy (_BC_VAR (value_NT), SAC_ND_A_FIELD (value_NT),                               \
            SAC_ND_A_DESC_SIZE (value_NT) * sizeof (value_type));

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_BROADCAST_WITH_DESC_INIT_WORKER( value_type, value_NT)
 *
 *   @brief   Initializes a broadcast operation (master -> workers) at the worker node.
 *
 *            This macro is for AKD and AKU arrays; the dimension, the size and the
 *descriptor are broadcast as well.
 *
 *            A barrier is required after this macro. The barrier is not included in this
 *macro for performance reasons. If multiple values are broadcast, a single barrier after
 *all calls to this macro suffices.
 *
 *   @param value_type    C-type of the value
 *   @param value_NT      target variable
 *
 ******************************************************************************/

#define SAC_DISTMEM_BROADCAST_WITH_DESC_INIT_WORKER(value_type, value_NT)                \
    _SAC_DISTMEM_BROADCAST_WITH_DESC_INIT_COMMON (value_type, value_NT)                  \
    SAC_DISTMEM_INVAL_CACHE_OF_NODE (_BC_DIMSIZE_OFFS (value_NT),                        \
                                     SAC_DISTMEM_RANK_MASTER, 2 * sizeof (int));         \
    _BC_VAR (value_NT) = NULL;

/** <!--********************************************************************-->
 *
 * @fn _SAC_DISTMEM_BROADCAST_INIT_COMMON( value_type, value_NT)
 *
 *   @brief   Common part of the initialization of a broadcast operation (master ->
 *worker) for both master and worker nodes.
 *
 *            This macro is for AKS arrays; the dimension, the size and the descriptor
 *            are not broadcast.
 *
 *   @param value_type    C-type of the value
 *   @param value_NT      at the master: source variable
 *                        at a worker: target variable
 *
 ******************************************************************************/

#define _SAC_DISTMEM_BROADCAST_INIT_COMMON(value_type, value_NT)                         \
    value_type *_BC_VAR (value_NT);                                                      \
    uintptr_t _BC_OFFS (value_NT);                                                       \
    SAC_TR_DISTMEM_PRINT ("Initializing broadcast of variable %s (type: %s).",           \
                          NT_STR (value_NT), #value_type);

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_BROADCAST_INIT_MASTER( value_type, value_NT)
 *
 *   @brief   Initializes a broadcast operation (master -> workers) at the master node.
 *
 *            This macro is for AKS arrays; the dimension, the size and the descriptor
 *            are not broadcast.
 *
 *            A barrier is required after this macro. The barrier is not included in this
 *macro for performance reasons. If multiple values are broadcast, a single barrier after
 *all calls to this macro suffices.
 *
 *   @param value_type    C-type of the value
 *   @param value_NT      source variable
 *
 ******************************************************************************/

#define SAC_DISTMEM_BROADCAST_INIT_MASTER(value_type, value_NT)                          \
    _SAC_DISTMEM_BROADCAST_INIT_COMMON (value_type, value_NT)                            \
    SAC_DISTMEM_HM_MALLOC_FIXED_SIZE (_BC_VAR (value_NT),                                \
                                      SAC_ND_A_SIZE (value_NT) * sizeof (value_type),    \
                                      value_type);                                       \
    _BC_OFFS (value_NT) = SAC_DISTMEM_DET_OFFS (_BC_VAR (value_NT));                     \
    SAC_TR_DISTMEM_PRINT ("BC data starting at: %p (offset: %" PRIuPTR ")",              \
                          _BC_VAR (value_NT), _BC_OFFS (value_NT));                      \
    SAC_TR_DISTMEM_PRINT ("BC copying %d B of data from %p.",                            \
                          SAC_ND_A_SIZE (value_NT) * sizeof (value_type),                \
                          SAC_ND_A_FIELD (value_NT));                                    \
    memcpy (_BC_VAR (value_NT), SAC_ND_A_FIELD (value_NT),                               \
            SAC_ND_A_SIZE (value_NT) * sizeof (value_type));

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_BROADCAST_INIT_WORKER( value_type, value_NT)
 *
 *   @brief   Initializes a broadcast operation (master -> workers) at a worker node.
 *
 *            This macro is for AKS arrays; the dimension, the size and the descriptor
 *            are not broadcast.
 *
 *            A barrier is required after this macro. The barrier is not included in this
 *macro for performance reasons. If multiple values are broadcast, a single barrier after
 *all calls to this macro suffices.
 *
 *   @param value_type    C-type of the value
 *   @param value_NT      target variable
 *
 ******************************************************************************/

#define SAC_DISTMEM_BROADCAST_INIT_WORKER(value_type, value_NT)                          \
    _SAC_DISTMEM_BROADCAST_INIT_COMMON (value_type, value_NT)

/** <!--********************************************************************-->
 *
 * @fn _SAC_DISTMEM_BROADCAST_SCL_INIT_COMMON( value_type, value_NT)
 *
 *   @brief   Common part of the initialization of a broadcast operation (master ->
 *worker) for both master and worker nodes.
 *
 *            This macro is for SCL; the dimension, the size and the descriptor
 *            are not broadcast.
 *
 *   @param value_type    C-type of the value
 *   @param value_NT      at the master: source variable
 *                        at a worker: target variable
 *
 ******************************************************************************/

#define _SAC_DISTMEM_BROADCAST_SCL_INIT_COMMON(value_type, value_NT)                     \
    value_type *_BC_VAR (value_NT);                                                      \
    uintptr_t _BC_OFFS (value_NT);                                                       \
    SAC_TR_DISTMEM_PRINT ("Initializing scalar broadcast of variable %s (type: %s).",    \
                          NT_STR (value_NT), #value_type);

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_BROADCAST_SCL_INIT_MASTER( value_type, value_NT)
 *
 *   @brief   Initializes a broadcast operation (master -> workers) at the master node.
 *
 *            This macro is for SCL; the dimension, the size and the descriptor
 *            are not broadcast.
 *
 *            A barrier is required after this macro. The barrier is not included in this
 *macro for performance reasons. If multiple values are broadcast, a single barrier after
 *all calls to this macro suffices.
 *
 *   @param value_type    C-type of the value
 *   @param value_NT      source variable
 *
 ******************************************************************************/

#define SAC_DISTMEM_BROADCAST_SCL_INIT_MASTER(value_type, value_NT)                      \
    _SAC_DISTMEM_BROADCAST_INIT_COMMON (value_type, value_NT)                            \
    SAC_DISTMEM_HM_MALLOC_FIXED_SIZE (_BC_VAR (value_NT), sizeof (value_type),           \
                                      value_type);                                       \
    _BC_OFFS (value_NT) = SAC_DISTMEM_DET_OFFS (_BC_VAR (value_NT));                     \
    SAC_TR_DISTMEM_PRINT ("BC data starting at: %p (offset: %" PRIuPTR ")",              \
                          _BC_VAR (value_NT), _BC_OFFS (value_NT));                      \
    SAC_TR_DISTMEM_PRINT ("BC copying %d B of data from %p.", sizeof (value_type),       \
                          &SAC_ND_A_FIELD (value_NT));                                   \
    memcpy (_BC_VAR (value_NT), &SAC_ND_A_FIELD (value_NT), sizeof (value_type));

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_BROADCAST_SCL_INIT_WORKER( value_type, value_NT)
 *
 *   @brief   Initializes a broadcast operation (master -> workers) at a worker node.
 *
 *            This macro is for SCL; the dimension, the size and the descriptor
 *            are not broadcast.
 *
 *            A barrier is required after this macro. The barrier is not included in this
 *macro for performance reasons. If multiple values are broadcast, a single barrier after
 *all calls to this macro suffices.
 *
 *   @param value_type    C-type of the value
 *   @param value_NT      target variable
 *
 ******************************************************************************/

#define SAC_DISTMEM_BROADCAST_SCL_INIT_WORKER(value_type, value_NT)                      \
    _SAC_DISTMEM_BROADCAST_INIT_COMMON (value_type, value_NT)

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_BROADCAST_WITH_DESC_STEP1_WORKER( value_type, value_NT)
 *
 *   @brief   First step to finalize a broadcast operation (master -> workers) at a worker
 *node.
 *
 *            This macro is for AKD and AKU arrays; the dimension, the size and the
 *descriptor are broadcast as well.
 *
 *            The finalize operation has been split into two operations to reduce
 *communication if multiple values are broadcast. Otherwise, the same memory pages may be
 *invalidated and fetched multiple times.
 *
 *            Important: Interfering accesses to the DSM segment and local cache
 *            are not allowed during the whole broadcast operation!
 *
 *            Note: This macro is called in between of the function application and the
 *update of the mirror. Since it is used for AKD and AUD arrays, (some of) the mirror
 *variables have not yet been initialized. We need to initialize them before calling
 *SAC_ND_ALLOC_BEGIN, however. After the descriptor has been allocated we copy the values
 *we received from the master node.
 *
 *   @param value_type    C-type of the value
 *   @param value_NT      target variable
 *
 ******************************************************************************/

#define SAC_DISTMEM_BROADCAST_WITH_DESC_STEP1_WORKER(value_type, value_NT)               \
    SAC_TR_DISTMEM_PRINT ("Finalizing broadcast with descriptor "                        \
                          "of variable %s (type: %s), step 1.",                          \
                          NT_STR (value_NT), #value_type);                               \
    SAC_UPDATE_A_MIRROR_DIM (value_NT,                                                   \
                             *SAC_DISTMEM_DSM_ELEM_POINTER (_BC_DIMSIZE_OFFS (value_NT), \
                                                            int,                         \
                                                            SAC_DISTMEM_RANK_MASTER,     \
                                                            0));                         \
    SAC_ND_A_MIRROR_SIZE (value_NT)                                                      \
      = *SAC_DISTMEM_DSM_ELEM_POINTER (_BC_DIMSIZE_OFFS (value_NT), int,                 \
                                       SAC_DISTMEM_RANK_MASTER, 1);                      \
    SAC_TR_DISTMEM_PRINT ("BC dim: %d, size: %d", _BC_DIMSIZE_VAR (value_NT)[0],         \
                          _BC_DIMSIZE_VAR (value_NT)[1]);                                \
    SAC_DISTMEM_HM_MALLOC_FIXED_SIZE_WITH_DESC (_BC_VAR (value_NT),                      \
                                                _BC_DESC_VAR (value_NT),                 \
                                                SAC_ND_A_SIZE (value_NT)                 \
                                                  * sizeof (value_type),                 \
                                                SAC_ND_A_DIM (value_NT), value_type,     \
                                                SAC_ND_DESC_BASETYPE (value_NT));        \
    _BC_OFFS (value_NT) = SAC_DISTMEM_DET_OFFS (_BC_VAR (value_NT));                     \
    SAC_TR_DISTMEM_PRINT ("BC data starting at: %p (offset: %" PRIuPTR ")",              \
                          _BC_VAR (value_NT), _BC_OFFS (value_NT));                      \
    _BC_DESC_OFFS (value_NT) = SAC_DISTMEM_DET_OFFS (_BC_DESC_VAR (value_NT));           \
    SAC_TR_DISTMEM_PRINT ("BC descriptor starting at: %p (offset: %" PRIuPTR ")",        \
                          _BC_DESC_VAR (value_NT), _BC_DESC_OFFS (value_NT));            \
    SAC_DISTMEM_INVAL_CACHE_OF_NODE (_BC_OFFS (value_NT), SAC_DISTMEM_RANK_MASTER,       \
                                     SAC_ND_A_SIZE (value_NT) * sizeof (value_type));    \
    SAC_DISTMEM_INVAL_CACHE_OF_NODE (_BC_DESC_OFFS (value_NT), SAC_DISTMEM_RANK_MASTER,  \
                                     BYTE_SIZE_OF_DESC (SAC_ND_A_DIM (value_NT)));

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_BROADCAST_WITH_DESC_STEP2_WORKER( value_type, value_NT)
 *
 *   @brief   Second step to finalize a broadcast operation (master -> workers) at a
 *worker node.
 *
 *            This macro is for AKD and AKU arrays; the dimension, the size and the
 *descriptor are broadcast as well.
 *
 *            The finalize operation has been split into two operations to reduce
 *communication if multiple values are broadcast. Otherwise, the same memory pages may be
 *invalidated and fetched multiple times.
 *
 *            Important: Interfering accesses to the DSM segment and local cache
 *            are not allowed during the whole broadcast operation!
 *
 *            Note: This macro is called in between of the function application and the
 *update of the mirror. Since it is used for AKD and AUD arrays, (some of) the mirror
 *variables have not yet been initialized. We need to initialize them before calling
 *SAC_ND_ALLOC_BEGIN, however. After the descriptor has been allocated we copy the values
 *we received from the master node.
 *
 *   @param value_type    C-type of the value
 *   @param value_NT      target variable
 *
 ******************************************************************************/

#define SAC_DISTMEM_BROADCAST_WITH_DESC_STEP2_WORKER(value_type, value_NT)               \
    SAC_TR_DISTMEM_PRINT ("Finalizing broadcast with descriptor "                        \
                          "of variable %s (type: %s), step 2.",                          \
                          NT_STR (value_NT), #value_type);                               \
    SAC_ND_ALLOC_BEGIN (value_NT, 1, SAC_ND_A_DIM (value_NT), value_type)                \
    SAC_TR_DISTMEM_PRINT ("BC copying %d B of descriptor to %p.",                        \
                          BYTE_SIZE_OF_DESC (SAC_ND_A_DIM (value_NT)),                   \
                          SAC_ND_A_DESC (value_NT));                                     \
    memcpy (SAC_ND_A_DESC (value_NT), _BC_DESC_VAR (value_NT),                           \
            BYTE_SIZE_OF_DESC (SAC_ND_A_DIM (value_NT)));                                \
    SAC_ND_ALLOC_END (value_NT, 1, SAC_ND_A_DIM (value_NT), value_type)                  \
    SAC_TR_DISTMEM_PRINT ("BC copying %d B of data to %p.",                              \
                          SAC_ND_A_SIZE (value_NT) * sizeof (value_type),                \
                          SAC_ND_A_FIELD (value_NT));                                    \
    memcpy (SAC_ND_A_FIELD (value_NT),                                                   \
            SAC_DISTMEM_DSM_ELEM_POINTER (_BC_OFFS (value_NT), value_type,               \
                                          SAC_DISTMEM_RANK_MASTER, 0),                   \
            SAC_ND_A_SIZE (value_NT) * sizeof (value_type));

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_BROADCAST_STEP1_WORKER( value_type, value_NT)
 *
 *   @brief   First step to finalize a broadcast operation (master -> workers) at a worker
 *node.
 *
 *            This macro is for AKS arrays; the dimension, the size and the descriptor
 *            are not broadcast.
 *
 *            The finalize operation has been split into two operations to reduce
 *communication if multiple values are broadcast. Otherwise, the same memory pages may be
 *invalidated and fetched multiple times.
 *
 *            Important: Interfering accesses to the DSM segment and local cache
 *            are not allowed during the whole broadcast operation!
 *
 *   @param value_type    C-type of the value
 *   @param value_NT      target variable
 *
 ******************************************************************************/

#define SAC_DISTMEM_BROADCAST_STEP1_WORKER(value_type, value_NT)                         \
    SAC_TR_DISTMEM_PRINT ("Finalizing broadcast "                                        \
                          "of variable %s (type: %s), step 1",                           \
                          NT_STR (value_NT), #value_type);                               \
    SAC_DISTMEM_HM_MALLOC_FIXED_SIZE (_BC_VAR (value_NT),                                \
                                      SAC_ND_A_SIZE (value_NT) * sizeof (value_type),    \
                                      value_type);                                       \
    _BC_OFFS (value_NT) = SAC_DISTMEM_DET_OFFS (_BC_VAR (value_NT));                     \
    SAC_TR_DISTMEM_PRINT ("BC data starting at: %p (offset: %" PRIuPTR ")",              \
                          _BC_VAR (value_NT), _BC_OFFS (value_NT));                      \
    SAC_DISTMEM_INVAL_CACHE_OF_NODE (_BC_OFFS (value_NT), SAC_DISTMEM_RANK_MASTER,       \
                                     SAC_ND_A_SIZE (value_NT) * sizeof (value_type));

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_BROADCAST_STEP2_WORKER( value_type, value_NT)
 *
 *   @brief   Second step to finalize a broadcast operation (master -> workers) at a
 *worker node.
 *
 *            This macro is for AKS arrays; the dimension, the size and the descriptor
 *            are not broadcast.
 *
 *            The finalize operation has been split into two operations to reduce
 *communication if multiple values are broadcast. Otherwise, the same memory pages may be
 *invalidated and fetched multiple times.
 *
 *            Important: Interfering accesses to the DSM segment and local cache
 *            are not allowed during the whole broadcast operation!
 *
 *   @param value_type    C-type of the value
 *   @param value_NT      target variable
 *
 ******************************************************************************/

#define SAC_DISTMEM_BROADCAST_STEP2_WORKER(value_type, value_NT)                         \
    SAC_TR_DISTMEM_PRINT ("Finalizing broadcast "                                        \
                          "of variable %s (type: %s), step 2",                           \
                          NT_STR (value_NT), #value_type);                               \
    SAC_ND_ALLOC_BEGIN (value_NT, 1, SAC_ND_A_DIM (value_NT), value_type)                \
    SAC_ND_ALLOC_END (value_NT, 1, SAC_ND_A_DIM (value_NT), value_type)                  \
    SAC_TR_DISTMEM_PRINT ("BC copying %d B of data to %p.",                              \
                          SAC_ND_A_SIZE (value_NT) * sizeof (value_type),                \
                          SAC_ND_A_FIELD (value_NT));                                    \
    memcpy (SAC_ND_A_FIELD (value_NT),                                                   \
            SAC_DISTMEM_DSM_ELEM_POINTER (_BC_OFFS (value_NT), value_type,               \
                                          SAC_DISTMEM_RANK_MASTER, 0),                   \
            SAC_ND_A_SIZE (value_NT) * sizeof (value_type));

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_BROADCAST_SCL_STEP1_WORKER( value_type, value_NT)
 *
 *   @brief   First step to finalize a broadcast operation (master -> workers) at a worker
 *node.
 *
 *            This macro is for SCL; the dimension, the size and the descriptor
 *            are not broadcast.
 *
 *            The finalize operation has been split into two operations to reduce
 *communication if multiple values are broadcast. Otherwise, the same memory pages may be
 *invalidated and fetched multiple times.
 *
 *            Important: Interfering accesses to the DSM segment and local cache
 *            are not allowed during the whole broadcast operation!
 *
 *   @param value_type    C-type of the value
 *   @param value_NT      target variable
 *
 ******************************************************************************/

#define SAC_DISTMEM_BROADCAST_SCL_STEP1_WORKER(value_type, value_NT)                     \
    SAC_TR_DISTMEM_PRINT ("Finalizing scalar broadcast "                                 \
                          "of variable %s (type: %s), step 1",                           \
                          NT_STR (value_NT), #value_type);                               \
    SAC_DISTMEM_HM_MALLOC_FIXED_SIZE (_BC_VAR (value_NT), sizeof (value_type),           \
                                      value_type);                                       \
    _BC_OFFS (value_NT) = SAC_DISTMEM_DET_OFFS (_BC_VAR (value_NT));                     \
    SAC_TR_DISTMEM_PRINT ("BC data starting at: %p (offset: %" PRIuPTR ")",              \
                          _BC_VAR (value_NT), _BC_OFFS (value_NT));                      \
    SAC_DISTMEM_INVAL_CACHE_OF_NODE (_BC_OFFS (value_NT), SAC_DISTMEM_RANK_MASTER,       \
                                     sizeof (value_type));

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_BROADCAST_SCL_STEP2_WORKER( value_type, value_NT)
 *
 *   @brief   Second step to finalize a broadcast operation (master -> workers) at a
 *worker node.
 *
 *            This macro is for SCL; the dimension, the size and the descriptor
 *            are not broadcast.
 *
 *            The finalize operation has been split into two operations to reduce
 *communication if multiple values are broadcast. Otherwise, the same memory pages may be
 *invalidated and fetched multiple times.
 *
 *            Important: Interfering accesses to the DSM segment and local cache
 *            are not allowed during the whole broadcast operation!
 *
 *   @param value_type    C-type of the value
 *   @param value_NT      target variable
 *
 ******************************************************************************/

#define SAC_DISTMEM_BROADCAST_SCL_STEP2_WORKER(value_type, value_NT)                     \
    SAC_TR_DISTMEM_PRINT ("Finalizing scalar broadcast "                                 \
                          "of variable %s (type: %s), step 2",                           \
                          NT_STR (value_NT), #value_type);                               \
    SAC_TR_DISTMEM_PRINT ("BC copying %d B of data to %p.", sizeof (value_type),         \
                          &SAC_ND_A_FIELD (value_NT));                                   \
    memcpy (&SAC_ND_A_FIELD (value_NT),                                                  \
            SAC_DISTMEM_DSM_ELEM_POINTER (_BC_OFFS (value_NT), value_type,               \
                                          SAC_DISTMEM_RANK_MASTER, 0),                   \
            sizeof (value_type));

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_BROADCAST_WITH_DESC_FREE_MEM_COMMON( value_type, value_NT)
 *
 *   @brief   Frees the memory that was allocated during the broadcast operation at both
 *master and workers.
 *
 *            This macro is for AKD and AKU arrays; the dimension, the size and the
 *descriptor were broadcast as well.
 *
 *            A barrier is required before this macro. The barrier is not included in this
 *macro for performance reasons. If multiple values are broadcast, a single barrier before
 *all calls to this macro suffices.
 *
 *   @param value_type    C-type of the value
 *   @param value_NT      at the master: source variable
 *                        at a worker: target variable
 *
 ******************************************************************************/

#define SAC_DISTMEM_BROADCAST_WITH_DESC_FREE_MEM_COMMON(value_type, value_NT)            \
    SAC_TR_DISTMEM_PRINT ("Freeing memory after broadcast with "                         \
                          "descriptor of variable %s of type %s.",                       \
                          NT_STR (value_NT), #value_type);                               \
    SAC_DISTMEM_HM_FREE (_BC_VAR (value_NT))                                             \
    SAC_DISTMEM_HM_FREE (_BC_DIMSIZE_VAR (value_NT))                                     \
    SAC_TR_DISTMEM_PRINT ("Done with broadcast with "                                    \
                          "descriptor of variable %s of type %s.",                       \
                          NT_STR (value_NT), #value_type);

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_BROADCAST_FREE_MEM_COMMON( value_type, value_NT)
 *
 *   @brief   Frees the memory that was allocated during the broadcast operation at both
 *worker and master.
 *
 *            This macro is for AKS arrays; the dimension, the size and the descriptor
 *            were not broadcast.
 *
 *            A barrier is required before this macro. The barrier is not included in this
 *macro for performance reasons. If multiple values are broadcast, a single barrier before
 *all calls to this macro suffices.
 *
 *   @param value_type    C-type of the value
 *   @param value_NT      at the master: source variable
 *                        at a worker: target variable
 *
 ******************************************************************************/

#define SAC_DISTMEM_BROADCAST_FREE_MEM_COMMON(value_type, value_NT)                      \
    SAC_TR_DISTMEM_PRINT ("Freeing memory after broadcast of variable %s of type %s.",   \
                          NT_STR (value_NT), #value_type);                               \
    SAC_DISTMEM_HM_FREE (_BC_VAR (value_NT))                                             \
    SAC_TR_DISTMEM_PRINT ("Done with broadcast of variable %s of type %s.",              \
                          NT_STR (value_NT), #value_type);

/** <!--********************************************************************-->
 *
 * @fn SAC_DISTMEM_BROADCAST_SCL_FREE_MEM_COMMON( value_type, value_NT)
 *
 *   @brief   Frees the memory that was allocated during the broadcast operation at both
 *worker and master.
 *
 *            This macro is for SCL; the dimension, the size and the descriptor
 *            were not broadcast.
 *
 *            A barrier is required before this macro. The barrier is not included in this
 *macro for performance reasons. If multiple values are broadcast, a single barrier before
 *all calls to this macro suffices.
 *
 *   @param value_type    C-type of the value
 *   @param value_NT      at the master: source variable
 *                        at a worker: target variable
 *
 ******************************************************************************/

#define SAC_DISTMEM_BROADCAST_SCL_FREE_MEM_COMMON(value_type, value_NT)                  \
    SAC_TR_DISTMEM_PRINT (                                                               \
      "Freeing memory after scalar broadcast of variable %s of type %s.",                \
      NT_STR (value_NT), #value_type);                                                   \
    SAC_DISTMEM_HM_FREE (_BC_VAR (value_NT))                                             \
    SAC_TR_DISTMEM_PRINT ("Done with broadcast of variable %s of type %s.",              \
                          NT_STR (value_NT), #value_type);

/******************************************
 * Runtime check macros
 *******************************************/

#if SAC_DO_CHECK_DISTMEM

/** <!--********************************************************************-->
 *
 * @fn void SAC_DISTMEM_ALLOW_DSM_ALLOCS( void)
 *
 *   @brief    Allows allocations in the DSM segment.
 *
 ******************************************************************************/

#define SAC_DISTMEM_ALLOW_DSM_ALLOCS()                                                   \
    {                                                                                    \
        SAC_TR_DISTMEM_PRINT ("Allowing allocations in the DSM segment.");               \
        SAC_DISTMEM_are_dsm_allocs_allowed = TRUE;                                       \
    }

/** <!--********************************************************************-->
 *
 * @fn void SAC_DISTMEM_FORBID_DSM_ALLOCS( void)
 *
 *   @brief    Forbids allocations in the DSM segment.
 *
 ******************************************************************************/

#define SAC_DISTMEM_FORBID_DSM_ALLOCS()                                                  \
    {                                                                                    \
        SAC_TR_DISTMEM_PRINT ("Forbidding allocations in the DSM segment.");             \
        SAC_DISTMEM_are_dsm_allocs_allowed = FALSE;                                      \
    }

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
 *              - ptr must not lie after the end of the local shared segment.
 *
 *   @return        TRUE if ptr is valid, FALSE otherwise
 *
 ******************************************************************************/

#define SAC_DISTMEM_IS_VALID_WRITE_PTR(ptr)                                              \
    (((uintptr_t)ptr < (uintptr_t)SAC_DISTMEM_shared_seg_ptr                             \
      || (uintptr_t)ptr > SAC_DISTMEM_CH_max_valid_write_ptr)                            \
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
 *
 *   @return        TRUE if ptr is valid, FALSE otherwise
 *
 ******************************************************************************/

#define SAC_DISTMEM_IS_VALID_CACHE_PTR(ptr)                                              \
    (((uintptr_t)ptr < (uintptr_t)SAC_DISTMEM_cache_ptr                                  \
      || (uintptr_t)ptr > SAC_DISTMEM_CH_max_valid_cache_ptr)                            \
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

/** <!--********************************************************************-->
 *
 * @fn void SAC_DISTMEM_IS_NON_DIST_PTR( ptr)
 *
 *   @brief    Determines whether ptr is a non-DSM pointer.
 *
 *             This macro is used when the runtime checks are activated to check
 *             that no distributed arrays are used as if they were non-distributed.
 *
 *   @return        TRUE if ptr is a non-DSM pointer, FALSE otherwise
 *
 ******************************************************************************/

#define SAC_DISTMEM_IS_NON_DIST_PTR(ptr)                                                 \
    ((!SAC_DISTMEM_IS_VALID_WRITE_PTR (ptr) && !SAC_DISTMEM_IS_VALID_CACHE_PTR (ptr))    \
       ? TRUE                                                                            \
       : FALSE)

#else /* SAC_DO_CHECK_DISTMEM */

#define SAC_DISTMEM_ALLOW_DSM_ALLOCS()

#define SAC_DISTMEM_FORBID_DSM_ALLOCS()

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

#define SAC_DISTMEM_SETUP()                                                              \
    SAC_DISTMEM_TR_Setup (SAC_SET_DISTMEM_MAX_MEMORY_MB,                                 \
                          SAC_SET_DISTMEM_MIN_ELEMS_PER_NODE,                            \
                          SAC_SET_DISTMEM_TRACE_PROFILE_NODE);

#define SAC_DISTMEM_INVAL_ENTIRE_CACHE() SAC_DISTMEM_TR_InvalEntireCache ();

#define SAC_DISTMEM_INVAL_CACHE(arr_offset, b) SAC_DISTMEM_TR_InvalCache (arr_offset, b);

#define SAC_DISTMEM_INVAL_CACHE_OF_NODE(arr_offset, node, b)                             \
    SAC_DISTMEM_TR_InvalCacheOfNode (arr_offset, node, b);

#define SAC_DISTMEM_BARRIER()                                                            \
    SAC_PF_BEGIN_BARRIER ()                                                              \
    SAC_DISTMEM_TR_Barrier ();                                                           \
    SAC_PF_END_BARRIER ()

#define SAC_DISTMEM_EXIT(exit_code) SAC_DISTMEM_TR_Exit (exit_code);

#define SAC_DISTMEM_DET_DO_DISTR_ARR(total_elems, dim0_size)                             \
    SAC_DISTMEM_TR_DetDoDistrArr (total_elems, dim0_size);

#define SAC_DISTMEM_DET_MAX_ELEMS_PER_NODE(total_elems, dim0_size)                       \
    SAC_DISTMEM_TR_DetMaxElemsPerNode (total_elems, dim0_size);

#define SAC_DISTEM_DET_DIM0_START(dim0_size, start_range, stop_range)                    \
    SAC_DISTMEM_TR_DetDim0Start (dim0_size, start_range, stop_range);

#define SAC_DISTEM_DET_DIM0_STOP(dim0_size, start, stop)                                 \
    SAC_DISTMEM_TR_DetDim0Stop (dim0_size, start, stop);

#define SAC_DISTMEM_ELEM_POINTER(arr_offset, elem_type, elems_first_nodes, elem_index)   \
    _SAC_DISTMEM_TR_ELEM_POINTER (arr_offset, elem_type, elems_first_nodes, elem_index)

#define SAC_DISTMEM_AVOIDED_PTR_CALC_LOCAL_WRITE_EXPR()                                  \
    SAC_TR_DISTMEM_AA_PRINT ("Avoided pointer calculation for local write.")             \
    SAC_DISTMEM_TR_num_avoided_ptr_calcs_local_writes++,

#define SAC_DISTMEM_AVOIDED_PTR_CALC_LOCAL_READ_EXPR(var_NT)                             \
    (SAC_ND_A_IS_DIST (var_NT)                                                           \
       ? SAC_TR_DISTMEM_AA_PRINT ("Avoided pointer calculation for local read.")         \
           SAC_DISTMEM_TR_num_avoided_ptr_calcs_local_reads++                            \
       : 0),

#define SAC_DISTMEM_AVOIDED_PTR_CALC_REMOTE_READ_EXPR()                                  \
    SAC_TR_DISTMEM_AA_PRINT ("Avoided pointer calculation for remote read.")             \
    SAC_DISTMEM_TR_num_avoided_ptr_calcs_remote_reads++,

#elif SAC_DO_PROFILE_DISTMEM

#define SAC_DISTMEM_INIT() SAC_DISTMEM_PR_Init (__argc, __argv);

#define SAC_DISTMEM_SETUP()                                                              \
    SAC_DISTMEM_PR_Setup (SAC_SET_DISTMEM_MAX_MEMORY_MB,                                 \
                          SAC_SET_DISTMEM_MIN_ELEMS_PER_NODE,                            \
                          SAC_SET_DISTMEM_TRACE_PROFILE_NODE);

#define SAC_DISTMEM_INVAL_ENTIRE_CACHE() SAC_DISTMEM_PR_InvalEntireCache ();

#define SAC_DISTMEM_INVAL_CACHE(arr_offset, b) SAC_DISTMEM_PR_InvalCache (arr_offset, b);

#define SAC_DISTMEM_INVAL_CACHE_OF_NODE(arr_offset, node, b)                             \
    SAC_DISTMEM_PR_InvalCacheOfNode (arr_offset, node, b);

#define SAC_DISTMEM_BARRIER()                                                            \
    SAC_PF_BEGIN_BARRIER ()                                                              \
    SAC_DISTMEM_PR_Barrier ();                                                           \
    SAC_PF_END_BARRIER ()

#define SAC_DISTMEM_EXIT(exit_code) SAC_DISTMEM_PR_Exit (exit_code);

#define SAC_DISTMEM_DET_DO_DISTR_ARR(total_elems, dim0_size)                             \
    SAC_DISTMEM_PR_DetDoDistrArr (total_elems, dim0_size);

#define SAC_DISTMEM_DET_MAX_ELEMS_PER_NODE(total_elems, dim0_size)                       \
    SAC_DISTMEM_PR_DetMaxElemsPerNode (total_elems, dim0_size);

#define SAC_DISTEM_DET_DIM0_START(dim0_size, start_range, stop_range)                    \
    SAC_DISTMEM_PR_DetDim0Start (dim0_size, start_range, stop_range);

#define SAC_DISTEM_DET_DIM0_STOP(dim0_size, start, stop)                                 \
    SAC_DISTMEM_PR_DetDim0Stop (dim0_size, start, stop);

#define SAC_DISTMEM_ELEM_POINTER(arr_offset, elem_type, elems_first_nodes, elem_index)   \
    _SAC_DISTMEM_PR_ELEM_POINTER (arr_offset, elem_type, elems_first_nodes, elem_index)

#define SAC_DISTMEM_AVOIDED_PTR_CALC_LOCAL_WRITE_EXPR()                                  \
    SAC_DISTMEM_TR_num_avoided_ptr_calcs_local_writes++,

#define SAC_DISTMEM_AVOIDED_PTR_CALC_LOCAL_READ_EXPR(var_NT)                             \
    (SAC_ND_A_IS_DIST (var_NT) ? SAC_DISTMEM_TR_num_avoided_ptr_calcs_local_reads++ : 0),

#define SAC_DISTMEM_AVOIDED_PTR_CALC_REMOTE_READ_EXPR()                                  \
    SAC_DISTMEM_TR_num_avoided_ptr_calcs_remote_reads++,

#else

#define SAC_DISTMEM_INIT() SAC_DISTMEM_Init (__argc, __argv);

#define SAC_DISTMEM_SETUP()                                                              \
    SAC_DISTMEM_Setup (SAC_SET_DISTMEM_MAX_MEMORY_MB,                                    \
                       SAC_SET_DISTMEM_MIN_ELEMS_PER_NODE,                               \
                       SAC_SET_DISTMEM_TRACE_PROFILE_NODE);

#define SAC_DISTMEM_INVAL_ENTIRE_CACHE() SAC_DISTMEM_InvalEntireCache ();

#define SAC_DISTMEM_INVAL_CACHE(arr_offset, b) SAC_DISTMEM_InvalCache (arr_offset, b);

#define SAC_DISTMEM_INVAL_CACHE_OF_NODE(arr_offset, node, b)                             \
    SAC_DISTMEM_InvalCacheOfNode (arr_offset, node, b);

#define SAC_DISTMEM_BARRIER() SAC_DISTMEM_Barrier ();

#define SAC_DISTMEM_EXIT(exit_code) SAC_DISTMEM_Exit (exit_code);

#define SAC_DISTMEM_DET_DO_DISTR_ARR(total_elems, dim0_size)                             \
    SAC_DISTMEM_DetDoDistrArr (total_elems, dim0_size);

#define SAC_DISTMEM_DET_MAX_ELEMS_PER_NODE(total_elems, dim0_size)                       \
    SAC_DISTMEM_DetMaxElemsPerNode (total_elems, dim0_size);

#define SAC_DISTEM_DET_DIM0_START(dim0_size, start_range, stop_range)                    \
    SAC_DISTMEM_DetDim0Start (dim0_size, start_range, stop_range);

#define SAC_DISTEM_DET_DIM0_STOP(dim0_size, start_range, stop_range)                     \
    SAC_DISTMEM_DetDim0Stop (dim0_size, start_range, stop_range);

#define SAC_DISTMEM_ELEM_POINTER(arr_offset, elem_type, elems_first_nodes, elem_index)   \
    _SAC_DISTMEM_ELEM_POINTER (arr_offset, elem_type, elems_first_nodes, elem_index)

#define SAC_DISTMEM_AVOIDED_PTR_CALC_LOCAL_WRITE_EXPR()

#define SAC_DISTMEM_AVOIDED_PTR_CALC_LOCAL_READ_EXPR(var_NT)

#define SAC_DISTMEM_AVOIDED_PTR_CALC_REMOTE_READ_EXPR()

#endif

#else /* SAC_DO_DISTMEM */

/******************************************
 * Dummy definitions for when the
 * distributed memory backend is not used.
 *******************************************/

#define SAC_DISTMEM_EXIT(exit_code) SAC_DISTMEM_Exit (exit_code);

#define SAC_DISTMEM_BARRIER()

/*
 * We do not define the other macros because they
 * should never be called when the distributed memory
 * backend is not used.
 */

#endif /* SAC_DO_DISTMEM */

#endif /* _SAC_DISTMEM_H */
