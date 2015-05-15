/*****************************************************************************
 *
 * file:   distmem.h
 *
 * prefix: SAC_DISTMEM_ (no tracing) / SAC_DISTMEM_TR_ (tracing)
 *
 * description: The header file declares the communication library independent
 *              interface of libsacdistmem. From outside libsacdistmem use
 *              the provided macros and do not call the functions directly.
 *              This way you do not need to check whether the distributed
 *              memory backend is actually used.
 *
 *****************************************************************************/

#ifndef _SAC_DISTMEM_H_
#define _SAC_DISTMEM_H_

/*****************************************************************************/

/* Rank of this node
 * We always declare this variable for tracing purposes. */
SAC_C_EXTERN_VAR size_t SAC_DISTMEM_rank;

/* For tracing purposes. */
#define SAC_DISTMEM_RANK_UNDEFINED SIZE_MAX

#if SAC_DO_DISTMEM

/* Minimum elements per node so that array is distributed. */
#define SAC_DISTMEM_MIN_ELEMS_PER_NODE 10

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

SAC_C_EXTERN_VAR unsigned long SAC_DISTMEM_TR_num_ptr_calcs;

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
 * @fn size_t SAC_DISTMEM_DetDim0Start( size_t dim0_size)
 *
 *   @brief   Determines first index of first dimension which is owned by this node.
 *
 *            This node owns all indices of the first dimension of the array for which
 *            index >= start.
 *
 *   @param dim0_size  size of the first dimension of the array
 *   @return           first index owned by this node
 *
 ******************************************************************************/

size_t SAC_DISTMEM_DetDim0Start (size_t dim0_size);

/** <!--********************************************************************-->
 *
 * @fn size_t SAC_DISTMEM_DetDim0Stop( size_t dim0_size, size_t *start, size_t *stop)
 *
 *   @brief   Determines the last index + 1 of the first dimension which is owned by this
 *node.
 *
 *            This node owns all indices of the first dimension of the array for which
 *            index < stop.
 *
 *   @param dim0_size   size of the first dimension of the array
 *   @return            last index owned by this node + 1
 *
 ******************************************************************************/

size_t SAC_DISTMEM_DetDim0Stop (size_t dim0_size);

/** <!--********************************************************************-->
 *
 * @fn void *SAC_DISTMEM_Malloc( size_t b, uintptr_t *offset)
 *
 *   @brief   Allocates b bytes of memory in the shared segment.
 *
 *   @param b         number of bytes to allocate
 *   @param offset    is set to the offset within the shared segment
 *   @return          pointer to allocated memory
 *
 ******************************************************************************/

void *SAC_DISTMEM_Malloc (size_t b, uintptr_t *offset);

/******************************************
 * Tracing declarations
 *******************************************/

void SAC_DISTMEM_TR_Init (int argc, char *argv[]);

void SAC_DISTMEM_TR_Setup (size_t maxmem_mb);

void SAC_DISTMEM_TR_InvalEntireCache (void);

void SAC_DISTMEM_TR_InvalCache (uintptr_t arr_offset, size_t b);

void SAC_DISTMEM_TR_Barrier (void);

void SAC_DISTMEM_TR_Exit (void);

bool SAC_DISTMEM_TR_DetDoDistrArr (size_t total_elems, size_t dim0_size);

size_t SAC_DISTMEM_TR_DetMaxElemsPerNode (size_t total_elems, size_t dim0_size);

size_t SAC_DISTMEM_TR_DetDim0Start (size_t dim0_size);

size_t SAC_DISTMEM_TR_DetDim0Stop (size_t dim0_size);

void *SAC_DISTMEM_TR_Malloc (size_t b, uintptr_t *offset);

/******************************************
 * Macros for accessing distributed
 * array elements
 *******************************************/

/** <!--********************************************************************-->
 *
 * @fn _SAC_DISTMEM_ELEM_POINTER(arr_offset, elem_type, elems_first_nodes, elem_index)
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

#define _SAC_DISTMEM_TR_ELEM_POINTER(arr_offset, elem_type, elems_first_nodes,           \
                                     elem_index)                                         \
    (SAC_DISTMEM_TR_num_ptr_calcs++,                                                     \
     SAC_TR_DISTMEM_PRINT (                                                              \
       "Retrieving pointer for element owned by node %zd (segment starting at: %p), "    \
       "offset within segment: %zd, element address: %p",                                \
       elem_index / elems_first_nodes,                                                   \
       (elem_type *)((uintptr_t)                                                         \
                       SAC_DISTMEM_local_seg_ptrs[elem_index / elems_first_nodes]        \
                     + arr_offset),                                                      \
       elem_index % elems_first_nodes,                                                   \
       _SAC_DISTMEM_ELEM_POINTER (arr_offset, elem_type, elems_first_nodes,              \
                                  elem_index)),                                          \
     _SAC_DISTMEM_ELEM_POINTER (arr_offset, elem_type, elems_first_nodes, elem_index))

/** <!--********************************************************************-->
 *
 * @fn _SAC_DISTMEM_RECALC_INDEX(elems_first_nodes, elem_index)
 *
 *   @brief   Returns the last index that is owned by the same node as elem_index.
 *
 *   @param elems_first_nodes   number of elements owned by all but the last node
 *   @param elem_index          index of the requested array element
 *   @return                    last index owned by the same node as elem_index
 *
 ******************************************************************************/
#define _SAC_DISTMEM_RECALC_INDEX(elems_first_nodes, elem_index)                         \
    elem_index + (elems_first_nodes - elem_index % elems_first_nodes);

#define _SAC_DISTMEM_TR_RECALC_INDEX(elems_first_nodes, elem_index)                      \
    _SAC_DISTMEM_RECALC_INDEX (elems_first_nodes, elem_index);

/******************************************
 * Macros to hide trace settings
 *******************************************/

#if SAC_DO_TRACE && SAC_DO_TRACE_DISTMEM

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

#define SAC_DISTEM_DET_DIM0_START(dim0_size) SAC_DISTMEM_TR_DetDim0Start (dim0_size);

#define SAC_DISTEM_DET_DIM0_STOP(dim0_size) SAC_DISTMEM_TR_DetDim0Stop (dim0_size);

#define SAC_DISTMEM_MALLOC(b, offset) SAC_DISTMEM_TR_Malloc (b, offset);

#define SAC_DISTMEM_ELEM_POINTER(arr_offset, elem_type, elems_first_nodes, elem_index)   \
    _SAC_DISTMEM_TR_ELEM_POINTER (arr_offset, elem_type, elems_first_nodes, elem_index)

#define SAC_DISTMEM_RECALC_INDEX(elems_first_nodes, elem_index)                          \
    _SAC_DISTMEM_TR_RECALC_INDEX (elems_first_nodes, elem_index);

#else /* SAC_DO_TRACE && SAC_DO_TRACE_DISTMEM */

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

#define SAC_DISTEM_DET_DIM0_START(dim0_size) SAC_DISTMEM_DetDim0Start (dim0_size);

#define SAC_DISTEM_DET_DIM0_STOP(dim0_size) SAC_DISTMEM_DetDim0Stop (dim0_size);

#define SAC_DISTMEM_MALLOC(b, offset) SAC_DISTMEM_Malloc (b, offset);

#define SAC_DISTMEM_ELEM_POINTER(arr_offset, elem_type, elems_first_nodes, elem_index)   \
    _SAC_DISTMEM_ELEM_POINTER (arr_offset, elem_type, elems_first_nodes, elem_index)

#define SAC_DISTMEM_RECALC_INDEX(elems_first_nodes, elem_index)                          \
    _SAC_DISTMEM_RECALC_INDEX (elems_first_nodes, elem_index);

#endif /* SAC_DO_TRACE && SAC_DO_TRACE_DISTMEM */

#else /* SAC_DO_DISTMEM */

/******************************************
 * Dummy definitions for when the
 * distributed memory backend is not used.
 *******************************************/

#define SAC_DISTMEM_INIT()

#define SAC_DISTMEM_SETUP()

#define SAC_DISTMEM_INVAL_ENTIRE_CACHE()

#define SAC_DISTMEM_INVAL_CACHE(arr_offset, b)

#define SAC_DISTMEM_BARRIER()

#define SAC_DISTMEM_EXIT()

/* TODO: Prelude needs this */
#define SAC_DISTMEM_ELEM_POINTER(arr_offset, elem_type, elems_first_nodes, elem_index) 0

/*
 * We do not define the following macros because they
 * should never be called when the distributed memory
 * backend is not used:
 *
 *  SAC_DISTMEM_TR_DET_DO_DISTR_ARR
 *  SAC_DISTMEM_DET_MAX_ELEMS_PER_NODE
 *  SAC_DISTEM_DET_DIM0_START_STOP
 *  SAC_DISTMEM_MALLOC
 *  SAC_DISTMEM_ELEM_POINTER
 *  SAC_DISTMEM_RECALC_INDEX
 *
 */

#endif /* SAC_DO_DISTMEM */

#endif /* _SAC_DISTMEM_H */
