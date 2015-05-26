/*****************************************************************************
 *
 * file:   distmem_commlib_gasnet.c
 *
 * prefix: SAC_DISTMEM_COMMLIB
 *
 * description: This is the GASNet specific implementation of distmem_commlib.h.
 *
 *****************************************************************************/

#include "config.h"

/* By default, we do not use tracing. */
#ifndef COMPILE_TRACE
#define COMPILE_TRACE 0
#endif /* !defined(COMPILE_TRACE) */

/*
 * Silence empty source file warning.
 *
 * Why? This source file is also compiled (empty)
 * when GASNet is disabled or when the code for
 * a different communication library is compiled.
 */
static UNUSED int SAC_DISTMEM_COMMLIB_GASNET_dummy;

#ifdef COMPILE_DISTMEM_GASNET

#if ENABLE_DISTMEM_GASNET

/* Do not show some warnings for the GASNet header for GCC (>= 4.6). */
#ifdef __GNUC__
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
#endif
#include <gasnet.h>
#ifdef __GNUC__
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
#pragma GCC diagnostic pop
#endif
#endif

#if COMPILE_TRACE
#define SAC_DO_TRACE 1
#define SAC_DO_TRACE_DISTMEM 1
#else /* COMPILE_TRACE */
#define SAC_DO_TRACE 0
#define SAC_DO_TRACE_DISTMEM 0
#endif /* COMPILE_TRACE */
#define SAC_DO_DISTMEM 1
#define SAC_DO_DISTMEM_GASNET 1

#include "sac.h"

#undef SAC_DO_TRACE
#undef SAC_DO_TRACE_DISTMEM
#undef SAC_DO_DISTMEM
#undef SAC_DO_DISTMEM_GASNET

#include "distmem_commlib.h"

/*
 * Takes care of error handling for GASNet functions that
 * return a success value.
 */
#define GASNET_SAFE(fncall)                                                              \
    {                                                                                    \
        int retval;                                                                      \
        if ((retval = fncall) != GASNET_OK) {                                            \
            SAC_RuntimeError ("Error calling: %s\n"                                      \
                              " at: %s:%i\n"                                             \
                              " error: %s (%s)\n",                                       \
                              #fncall, __FILE__, __LINE__, gasnet_ErrorName (retval),    \
                              gasnet_ErrorDesc (retval));                                \
        }                                                                                \
    }

/* Holds the shared segment base addresses and sizes of all nodes. */
static gasnet_seginfo_t *seg_info;

#if COMPILE_TRACE
void
SAC_DISTMEM_COMMLIB_TR_Init (int argc, char *argv[])
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_COMMLIB_Init (int argc, char *argv[])
#endif /* COMPILE_TRACE */
{
    GASNET_SAFE (gasnet_init (&argc, &argv));
}

#if COMPILE_TRACE
void
SAC_DISTMEM_COMMLIB_TR_Setup (size_t maxmem)
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_COMMLIB_Setup (size_t maxmem)
#endif /* COMPILE_TRACE */
{
    size_t i;

    /* Print GASNet configuration. */
    SAC_TR_DISTMEM_PRINT (GASNET_CONFIG_STRING);

    SAC_DISTMEM_rank = gasnet_mynode ();
    SAC_DISTMEM_size = gasnet_nodes ();
    // TODO: Check the size again the specified size?

    if (SAC_DISTMEM_pagesz != GASNET_PAGESIZE) {
        SAC_RuntimeError (
          "System page size (%zd) and GASNet page (%zd) size do not match.",
          SAC_DISTMEM_pagesz, GASNET_PAGESIZE);
    }

    /* Global minimum of optimistic maximum shared segment size */
    size_t max_global_segsz = gasnet_getMaxGlobalSegmentSize ();

    /*
     * Determine minimum of:
     *  - user-specified maximum memory
     *  - and global minimum of optimistic maximum shared segment size
     */
    size_t requested_segsize = SAC_MIN (maxmem, max_global_segsz);
    size_t heapsz = 0;
    GASNET_SAFE (gasnet_attach (NULL, 0, requested_segsize, heapsz));

    seg_info = (gasnet_seginfo_t *)malloc (SAC_DISTMEM_size * sizeof (gasnet_seginfo_t));
    GASNET_SAFE (gasnet_getSegmentInfo (seg_info, SAC_DISTMEM_size));

    /*
     * Check the minimum allocated segment size because less than the requested amount
     * may have been allocated at some nodes.
     */
    size_t min_global_segsz = max_global_segsz;
    for (i = 0; i < SAC_DISTMEM_size; i++) {
        min_global_segsz = SAC_MIN (min_global_segsz, seg_info[i].size);
    }

    if (maxmem != min_global_segsz) {
        SAC_RuntimeWarning (
          "Registered less than requested memory: (%zd) MB rather than (%zd) MB",
          min_global_segsz / 1024 / 1024, maxmem / 1024 / 1024);
    }

    /*
     * Divide by and multiply with the page size because the segment size has to be a
     * multiple of the page size.
     */
    SAC_DISTMEM_segsz
      = min_global_segsz / SAC_DISTMEM_size / SAC_DISTMEM_pagesz * SAC_DISTMEM_pagesz;
    SAC_DISTMEM_shared_seg_ptr = seg_info[SAC_DISTMEM_rank].addr;

    /*
           * With GASNet the cache needs to lie within the segment.
           * When previously pinned pages are protected, this leads to an error:
           * FATAL ERROR: ibv_reg_mr failed in firehose_move_callback errno=14 (Bad
       address)

           * From readme:
           * In a GASNET_SEGMENT_FAST configuration, the GASNet
           * segment is registered (pinned) with the HCA at initialization time,
           * because pinning is required for RDMA.  However, GASNet allows for
           * local addresses (source of a PUT or destination of a GET) to lie
           * outside of the GASNet segment.  So, to perform RDMA GETs and PUTs,
           * ibv-conduit must either copy out-of-segment transfers though
           * preregistered bounce buffers, or dynamically register memory.  By
           * default firehose is used to manage registration of out-of-segment
           * memory.
           */
    SAC_DISTMEM_cache_ptr
      = (void *)((uintptr_t)SAC_DISTMEM_shared_seg_ptr + SAC_DISTMEM_segsz);
}

#if COMPILE_TRACE
void
SAC_DISTMEM_COMMLIB_TR_Barrier (void)
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_COMMLIB_Barrier (void)
#endif /* COMPILE_TRACE */
{
    gasnet_barrier_notify (0, GASNET_BARRIERFLAG_ANONYMOUS);
    GASNET_SAFE (gasnet_barrier_wait (0, GASNET_BARRIERFLAG_ANONYMOUS));
}

#if COMPILE_TRACE
void
SAC_DISTMEM_COMMLIB_TR_LoadPage (void *local_page_ptr, size_t owner_rank,
                                 size_t remote_page_index)
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_COMMLIB_LoadPage (void *local_page_ptr, size_t owner_rank,
                              size_t remote_page_index)
#endif /* COMPILE_TRACE */
{
    SAC_TR_DISTMEM_PRINT ("GASNet get from remote address: %p",
                          (void *)((uintptr_t)seg_info[owner_rank].addr
                                   + remote_page_index * SAC_DISTMEM_pagesz));
    gasnet_get (local_page_ptr, owner_rank,
                (void *)((uintptr_t)seg_info[owner_rank].addr
                         + remote_page_index * SAC_DISTMEM_pagesz),
                SAC_DISTMEM_pagesz);
}

#if COMPILE_TRACE
void
SAC_DISTMEM_COMMLIB_TR_Exit (void)
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_COMMLIB_Exit (void)
#endif /* COMPILE_TRACE */
{
    gasnet_exit (0);
}

#endif /* ENABLE_DISTMEM_GASNET */

#endif /* defined(COMPILE_DISTMEM_GASNET) */
