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

#if SAC_DO_DISTMEM

/******************************************
 * Global variables
 *******************************************/

/* Rank of this node */
SAC_C_EXTERN_VAR int SAC_DISTMEM_rank;

/* Number of nodes */
SAC_C_EXTERN_VAR int SAC_DISTMEM_size;

/* System page size */
SAC_C_EXTERN_VAR size_t SAC_DISTMEM_pagesz;

/* Segment size (applies to local shared segment and local
 * caches for segments owned by other nodes) */
SAC_C_EXTERN_VAR size_t SAC_DISTMEM_segsz;

/* Pointer to start of local shared segment */
SAC_C_EXTERN_VAR void *SAC_DISTMEM_shared_seg_ptr;

/* Pointer to start of local caches */
SAC_C_EXTERN_VAR void *SAC_DISTMEM_cache_ptr;

/* Pointers to the local shared segment (at index SAC_DISTMEM_rank)
 * and to the local caches of segments owned by other nodes
 * (at other indices) */
SAC_C_EXTERN_VAR void **SAC_DISTMEM_local_seg_ptrs;

/* Offset to free memory within shared segment */
SAC_C_EXTERN_VAR size_t SAC_DISTMEM_seg_free_offs;

/******************************************
 * No tracing declarations
 *******************************************/

void SAC_DISTMEM_Init (int argc, char *argv[]);

void SAC_DISTMEM_Setup (size_t maxmem_mb);

void SAC_DISTMEM_InvalCache (void);

void SAC_DISTMEM_Barrier (void);

void SAC_DISTMEM_Exit (void);

/******************************************
 * Tracing declarations
 *******************************************/

void SAC_DISTMEM_TR_Init (int argc, char *argv[]);

void SAC_DISTMEM_TR_Setup (size_t maxmem_mb);

void SAC_DISTMEM_TR_InvalCache (void);

void SAC_DISTMEM_TR_Barrier (void);

void SAC_DISTMEM_TR_Exit (void);

/******************************************
 * Macros to hide function calls
 *******************************************/

#if SAC_DO_TRACE && SAC_DO_TRACE_DISTMEM

#define SAC_DISTMEM_INIT() SAC_DISTMEM_TR_Init (__argc, __argv);

#define SAC_DISTMEM_SETUP() SAC_DISTMEM_TR_Setup (SAC_SET_DISTMEM_MAX_MEMORY_MB);

#define SAC_DISTMEM_INVAL_CACHE() SAC_DISTMEM_TR_InvalCache ();

#define SAC_DISTMEM_BARRIER() SAC_DISTMEM_TR_Barrier ();

#define SAC_DISTMEM_EXIT() SAC_DISTMEM_TR_Exit ();

#else /* SAC_DO_TRACE && SAC_DO_TRACE_DISTMEM */

#define SAC_DISTMEM_INIT() SAC_DISTMEM_Init (__argc, __argv);

#define SAC_DISTMEM_SETUP() SAC_DISTMEM_Setup (SAC_SET_DISTMEM_MAX_MEMORY_MB);

#define SAC_DISTMEM_INVAL_CACHE() SAC_DISTMEM_InvalCache ();

#define SAC_DISTMEM_BARRIER() SAC_DISTMEM_Barrier ();

#define SAC_DISTMEM_EXIT() SAC_DISTMEM_Exit ();

#endif /* SAC_DO_TRACE && SAC_DO_TRACE_DISTMEM */

#else /* SAC_DO_DISTMEM */

/******************************************
 * Dummy definitions for when the
 * distributed memory backend is not used.
 *******************************************/

#define SAC_DISTMEM_INIT()

#define SAC_DISTMEM_SETUP()

#define SAC_DISTMEM_INVAL_CACHE()

#define SAC_DISTMEM_BARRIER()

#define SAC_DISTMEM_EXIT()

#endif /* SAC_DO_DISTMEM */

#endif /* _SAC_DISTMEM_H */
