/*****************************************************************************
 *
 * file:   distmem_commlib_gpi.c
 *
 * prefix: SAC_DISTMEM_COMMLIB
 *
 * description: This is the GPI specific implementation of distmem_commlib.h.
 *
 * GPI-2 is the implementation of the GASPI standard.
 * See http://http://www.gaspi.de and http://www.gpi-site.com
 *
 *****************************************************************************/

#include "config.h"

/* By default, we do not use tracing. */
#ifndef COMPILE_TRACE
#define COMPILE_TRACE 0
#endif /* !defined(COMPILE_TRACE) */

/* Silence empty source file warning */
static UNUSED int SAC_DISTMEM_COMMLIB_GPI_dummy;

#ifdef COMPILE_DISTMEM_GPI

#if ENABLE_DISTMEM_GPI

/* For GPI-2 (GASPI standard implementation) */
#include <GASPI.h>
/*
 * Important: when using GPI, do not print anything at other
 * nodes than 0 when no error occurs and debug is disabled.
 * Printing to stderr from other ranks than 0 ends up in
 * stdout too.
 */

/* For NULL */
#include <stdlib.h>
/* For mmap */
#include <sys/mman.h>
/* For errno */
#include <errno.h>

#if COMPILE_TRACE
#define SAC_DO_TRACE 1
#define SAC_DO_TRACE_DISTMEM 1
#else /* COMPILE_TRACE */
#define SAC_DO_TRACE 0
#define SAC_DO_TRACE_DISTMEM 0
#endif /* COMPILE_TRACE */
#define SAC_DO_DISTMEM 1
#define SAC_DO_DISTMEM_GPI 1

#include "sac.h"

#undef SAC_DO_TRACE
#undef SAC_DO_TRACE_DISTMEM
#undef SAC_DO_DISTMEM
#undef SAC_DO_DISTMEM_GPI

#include "distmem_commlib.h"

/*
 * Takes care of error handling for GPI functions that
 * return a success value.
 */
#define GPI_SAFE(fncall)                                                                 \
    {                                                                                    \
        const gaspi_return_t retval = fncall;                                            \
        if (retval != GASPI_SUCCESS) {                                                   \
            SAC_RuntimeError ("Error during GPI call from: %s\n"                         \
                              " at %s:%i\n"                                              \
                              " error: %d",                                              \
                              #fncall, __FILE__, __LINE__, retval);                      \
        }                                                                                \
    }

#define SEGID_SHARED_SEG 0
#define SEGID_LOCAL_CACHE 1

static gaspi_segment_id_t
get_segid (gaspi_rank_t at, int segid_type)
{
    return at * 2 + segid_type;
}

static char *
get_seg (gaspi_rank_t at, int segid_type)
{
    void *seg = NULL;
    GPI_SAFE (gaspi_segment_ptr (get_segid (at, segid_type), &seg));
    return seg;
}

static char *
get_local_shared_seg (void)
{
    return get_seg (SAC_DISTMEM_rank, SEGID_SHARED_SEG);
}

#if COMPILE_TRACE
void
SAC_DISTMEM_COMMLIB_TR_Init (int argc, char *argv[])
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_COMMLIB_Init (int argc, char *argv[])
#endif /* COMPILE_TRACE */
{
    gaspi_rank_t rank;
    gaspi_rank_t size;

    GPI_SAFE (gaspi_proc_init (GASPI_BLOCK));
    GPI_SAFE (gaspi_proc_rank (&rank));
    GPI_SAFE (gaspi_proc_num (&size));

    SAC_DISTMEM_rank = rank;
    SAC_DISTMEM_size = size;
}

#if COMPILE_TRACE
void
SAC_DISTMEM_COMMLIB_TR_Setup (size_t maxmem, bool alloc_cache_outside_dsm)
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_COMMLIB_Setup (size_t maxmem, bool alloc_cache_outside_dsm)
#endif /* COMPILE_TRACE */
{
    /* When GPI is used as a communication library, the cache has to be allocated
     * within the DSM segment. We can, therefore, not use the maximum amount of memory
     * for the shared segment. */
    SAC_DISTMEM_segsz = maxmem / SAC_DISTMEM_size;

    /*
     * Divide by and multiply with the page size because the segment size has to be a
     * multiple of the page size.
     */
    SAC_DISTMEM_segsz = SAC_DISTMEM_segsz / SAC_DISTMEM_pagesz * SAC_DISTMEM_pagesz;

    gaspi_number_t segment_max;
    GPI_SAFE (gaspi_segment_max (&segment_max));
    if (segment_max < SAC_DISTMEM_size + 1) {
        SAC_RuntimeError ("GPI does support at most %d segments but %d were requested. "
                          "Try to run the program with fewer nodes.",
                          segment_max, SAC_DISTMEM_size + 1);
    }

    /* Allocate local cache. */
    GPI_SAFE (gaspi_segment_alloc (get_segid (SAC_DISTMEM_rank, SEGID_LOCAL_CACHE),
                                   SAC_DISTMEM_segsz * (SAC_DISTMEM_size - 1),
                                   GASPI_ALLOC_DEFAULT));

    /* For each node ... */
    for (size_t i = 0; i < SAC_DISTMEM_size; i++) {
        /* Allocate and register shared segment. */
        GPI_SAFE (gaspi_segment_create (get_segid (i, SEGID_SHARED_SEG),
                                        SAC_DISTMEM_segsz, GASPI_GROUP_ALL, GASPI_BLOCK,
                                        GASPI_ALLOC_DEFAULT));
    }

    SAC_DISTMEM_shared_seg_ptr = get_local_shared_seg ();
    SAC_DISTMEM_cache_ptr = get_seg (SAC_DISTMEM_rank, SEGID_LOCAL_CACHE);

    if ((uintptr_t)SAC_DISTMEM_cache_ptr % SAC_DISTMEM_pagesz != 0) {
        SAC_RuntimeError ("GPI cache is not page-aligned.");
    }

    if ((uintptr_t)SAC_DISTMEM_shared_seg_ptr % SAC_DISTMEM_pagesz != 0) {
        SAC_RuntimeError ("GPI shared segment is not page-aligned.");
    }
}

#if COMPILE_TRACE
void
SAC_DISTMEM_COMMLIB_TR_Barrier ()
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_COMMLIB_Barrier ()
#endif /* COMPILE_TRACE */
{
    GPI_SAFE (gaspi_barrier (GASPI_GROUP_ALL, GASPI_BLOCK));
}

#if COMPILE_TRACE
void
SAC_DISTMEM_COMMLIB_TR_Exit (int exit_code)
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_COMMLIB_Exit (int exit_code)
#endif /* COMPILE_TRACE */
{
    GPI_SAFE (gaspi_proc_term (GASPI_BLOCK));
}

#if COMPILE_TRACE
void
SAC_DISTMEM_COMMLIB_TR_Abort (int exit_code)
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_COMMLIB_Abort (int exit_code)
#endif /* COMPILE_TRACE */
{
    GPI_SAFE (gaspi_proc_term (GASPI_BLOCK));
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
    const gaspi_queue_id_t queue_id = 0;
    uintptr_t local_offset = (uintptr_t)local_page_ptr - (uintptr_t)SAC_DISTMEM_cache_ptr;

    SAC_TR_DISTMEM_PRINT (
      "GPI get from node %d, local segment: %d, local offset: %" PRIuPTR
      ", remote seg: %d, remote offset: %zd\n",
      owner_rank, get_segid (SAC_DISTMEM_rank, SEGID_LOCAL_CACHE), local_offset,
      get_segid (owner_rank, SEGID_SHARED_SEG), remote_page_index * SAC_DISTMEM_pagesz,
      SAC_DISTMEM_pagesz);

    GPI_SAFE (gaspi_read (get_segid (SAC_DISTMEM_rank, SEGID_LOCAL_CACHE), local_offset,
                          owner_rank, get_segid (owner_rank, SEGID_SHARED_SEG),
                          remote_page_index * SAC_DISTMEM_pagesz, SAC_DISTMEM_pagesz,
                          queue_id, GASPI_BLOCK));
    GPI_SAFE (gaspi_wait (queue_id, GASPI_BLOCK));
}

#endif /* ENABLE_DISTMEM_GPI */

#endif /* defined(COMPILE_DISTMEM_GPI) */
