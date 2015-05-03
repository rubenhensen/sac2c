/*****************************************************************************
 *
 * file:   distmem.h
 *
 * prefix: SAC_DISTMEM_ (no tracing) / SAC_DISTMEM_TR_ (tracing)
 *
 * description:
 *
 *****************************************************************************/

#ifndef _SAC_DISTMEM_H_
#define _SAC_DISTMEM_H_

/*****************************************************************************/

#if SAC_DO_DISTMEM

SAC_C_EXTERN_VAR int SAC_DISTMEM_rank;

SAC_C_EXTERN_VAR int SAC_DISTMEM_size;

/* No tracing */

void SAC_DISTMEM_Init (int argc, char *argv[]);

void SAC_DISTMEM_Barrier ();

void SAC_DISTMEM_Exit ();

/* Tracing */

void SAC_DISTMEM_TR_Init (int argc, char *argv[]);

void SAC_DISTMEM_TR_Barrier ();

void SAC_DISTMEM_TR_Exit ();

/* Macros to hide function calls */

#if SAC_DO_TRACE && SAC_DO_TRACE_DISTMEM

#define SAC_DISTMEM_INIT() SAC_DISTMEM_TR_Init (__argc, __argv);

#define SAC_DISTMEM_BARRIER() SAC_DISTMEM_TR_Barrier ();

#define SAC_DISTMEM_EXIT() SAC_DISTMEM_TR_Exit ();

#else /* SAC_DO_TRACE && SAC_DO_TRACE_DISTMEM */

#define SAC_DISTMEM_INIT() SAC_DISTMEM_Init (__argc, __argv);

#define SAC_DISTMEM_BARRIER() SAC_DISTMEM_Barrier ();

#define SAC_DISTMEM_EXIT() SAC_DISTMEM_Exit ();

#endif /* SAC_DO_TRACE && SAC_DO_TRACE_DISTMEM */

#else /* SAC_DO_DISTMEM */

/* Dummy definitions */

#define SAC_DISTMEM_INIT()

#define SAC_DISTMEM_BARRIER()

#define SAC_DISTMEM_EXIT()

#endif /* SAC_DO_DISTMEM */

#endif /* _SAC_DISTMEM_H */

/*****************************************************************************/
