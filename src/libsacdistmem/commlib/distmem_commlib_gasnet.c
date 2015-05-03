/*****************************************************************************
 *
 * file:   distmem_commlib_gasnet.c
 *
 * prefix: SAC_DISTMEM_COMMLIB
 *
 * description:
 *
 *****************************************************************************/

#include "config.h"

/* By default, we do not use tracing. */
#ifndef COMPILE_TRACE
#define COMPILE_TRACE 0
#endif /* !defined(COMPILE_TRACE) */

/* Silence empty source file warning */
static UNUSED int SAC_DISTMEM_COMMLIB_GASNET_dummy;

#ifdef COMPILE_DISTMEM_GASNET

// TODO: This is temporary until we actually check the availability.
#define ENABLE_DISTMEM_GASNET 1

#if ENABLE_DISTMEM_GASNET

#include <gasnet.h>

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

#define GASNET_SAFE(fncall)                                                              \
    {                                                                                    \
        int retval;                                                                      \
        if ((retval = fncall) != GASNET_OK) {                                            \
            SAC_RuntimeError ("Error calling: %s\n"                                      \
                              " at: %s:%i\n"                                             \
                              " error: %s (%s)\n",                                       \
                              #fncall, __FILE__, __LINE__, gasnet_ErrorName (retval),    \
                              gasnet_ErrorDesc (retval));                                \
            SAC_DISTMEM_Exit ();                                                         \
        }                                                                                \
    }

#if COMPILE_TRACE
void
SAC_DISTMEM_COMMLIB_TR_Init (int argc, char *argv[])
{
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_COMMLIB_Init (int argc, char *argv[])
{
#endif /* COMPILE_TRACE */
    GASNET_SAFE (gasnet_init (&argc, &argv));
    SAC_DISTMEM_rank = gasnet_mynode ();
    SAC_DISTMEM_size = gasnet_nodes ();

    size_t segsz = GASNET_PAGESIZE * 1;
    size_t heapsz = 0;

    GASNET_SAFE (gasnet_attach (NULL, 0, segsz * SAC_DISTMEM_size, heapsz));
}

#if COMPILE_TRACE
void
SAC_DISTMEM_COMMLIB_TR_Barrier ()
{
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_COMMLIB_Barrier ()
{
#endif /* COMPILE_TRACE */
    gasnet_barrier_notify (0, GASNET_BARRIERFLAG_ANONYMOUS);
    GASNET_SAFE (gasnet_barrier_wait (0, GASNET_BARRIERFLAG_ANONYMOUS));
}

#if COMPILE_TRACE
void
SAC_DISTMEM_COMMLIB_TR_Exit ()
{
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_COMMLIB_Exit ()
{
#endif /* COMPILE_TRACE */
    gasnet_exit (0);
}

#endif /* ENABLE_DISTMEM_GASNET */

#endif /* defined(COMPILE_DISTMEM_GASNET) */
