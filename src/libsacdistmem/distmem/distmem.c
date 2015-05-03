/*****************************************************************************
 *
 * file:   distmem.c
 *
 * prefix: SAC_DISTMEM_ (no tracing) / SAC_DISTMEM_TR_ (tracing)
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

#ifdef COMPILE_DISTMEM

// TODO: This is temporary until we actually check the availability.
#define ENABLE_DISTMEM 1

#if ENABLE_DISTMEM

#if COMPILE_TRACE
#define SAC_DO_TRACE 1
#define SAC_DO_TRACE_DISTMEM 1
#else /* COMPILE_TRACE */
#define SAC_DO_TRACE 0
#define SAC_DO_TRACE_DISTMEM 0
#endif /* COMPILE_TRACE */
#define SAC_DO_DISTMEM 1

#include "sac.h"

#undef SAC_DO_TRACE
#undef SAC_DO_TRACE_DISTMEM
#undef SAC_DO_DISTMEM

#include "distmem_commlib.h"

#if !COMPILE_TRACE
/*
 * If we compile for distmem_trace.o, we don't need the global variables since
 * these always remain in distmem_notrace.o.
 */

int SAC_DISTMEM_rank;

int SAC_DISTMEM_size;
#endif

#if COMPILE_TRACE
void
SAC_DISTMEM_TR_Init (int argc, char *argv[])
{
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_Init (int argc, char *argv[])
{
#endif /* COMPILE_TRACE */
    SAC_TR_DISTMEM_PRINT (("Initializing communication library.\n"));
    SAC_DISTMEM_COMMLIB_INIT (argc, argv);
    SAC_TR_DISTMEM_PRINT (("Rank: %d, size: %d\n", SAC_DISTMEM_rank, SAC_DISTMEM_size));
}

#if COMPILE_TRACE
void
SAC_DISTMEM_TR_Barrier ()
{
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_Barrier ()
{
#endif /* COMPILE_TRACE */
    SAC_TR_DISTMEM_PRINT (("%d before barrier.\n", SAC_DISTMEM_rank));
    SAC_DISTMEM_COMMLIB_BARRIER ();
    SAC_TR_DISTMEM_PRINT (("%d after barrier.\n", SAC_DISTMEM_rank));
}

#if COMPILE_TRACE
void
SAC_DISTMEM_TR_Exit ()
{
#else  /* COMPILE_TRACE */
void
SAC_DISTMEM_Exit ()
{
#endif /* COMPILE_TRACE */
    SAC_TR_DISTMEM_PRINT (("%d exiting communication library.\n", SAC_DISTMEM_rank));
    SAC_DISTMEM_COMMLIB_EXIT ();
}

#endif /* ENABLE_DISTMEM */

#endif /* defined(COMPILE_DISTMEM) */
