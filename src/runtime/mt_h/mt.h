/*
 * $Id: mt.h 16792 2010-03-29 09:22:23Z cg $
 */

/*****************************************************************************
 *
 * file:   mt.h
 *
 * prefix: SAC_MT_
 *
 * description:
 *
 *   It is the major header file of the implementation of the common multi-threading
 *   facilities, both for OpenMP solution and Pthread solution.
 *
 *****************************************************************************/

#ifndef _SAC_MT_H_
#define _SAC_MT_H_

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif /* SAC_C_EXTERN */

#ifndef SAC_SIMD_COMPILATION

/*****************************************************************************/

#if SAC_DO_MULTITHREAD

/***
 ***   Definitions and declarations for common the multi-threaded runtime system
 ***/

#ifndef SAC_SET_CACHE_1_LINE
#define SAC_SET_CACHE_1_LINE 0
#endif

#ifndef SAC_SET_CACHE_2_LINE
#define SAC_SET_CACHE_2_LINE 0
#endif

#ifndef SAC_SET_CACHE_3_LINE
#define SAC_SET_CACHE_3_LINE 0
#endif

/*
 * Macros for cache line adjustment of data structure
 */

#define SAC_MT_CACHE_LINE_MAX()                                                          \
    SAC_MAX (SAC_SET_CACHE_1_LINE,                                                       \
             SAC_MAX (SAC_SET_CACHE_2_LINE, SAC_MAX (SAC_SET_CACHE_3_LINE, 1)))

#include <pthread.h>
#include <alloca.h>

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199506L
#endif

#ifndef _REENTRANT
#define _REENTRANT
#endif

#ifndef memset
extern void *memset (void *s, int c, size_t n);
#endif

#ifndef memcpy
extern void *memcpy (void *dest, const void *src, size_t n);
#endif

/*
 * We need the above extern declarations here rather than including
 * the corresponding header files because the further declarations in
 * string.h conflict with SAC generated headers in the SAC string module.
 *
 * The check for a previous definition of a macro of equal name are required
 * for operating systems that try to overload memset or memcpy, e.g. MAC OS.
 */

/*****************************************************************************/

/*****************************************************************************/

/*
 * Definition of macros implementing a general locking mechanism
 */

#define SAC_MT_DEFINE_LOCK(name) pthread_mutex_t name = PTHREAD_MUTEX_INITIALIZER;

#define SAC_MT_DECLARE_LOCK(name) SAC_C_EXTERN pthread_mutex_t name;

#define SAC_MT_ACQUIRE_LOCK(name) pthread_mutex_lock (&name);

#define SAC_MT_RELEASE_LOCK(name) pthread_mutex_unlock (&name);

/*****************************************************************************/

/*
 * Macros for implementing the SPMD frame used to move data into SPMD function
 *
 * SAC MT FRAME, used to pass in and inout arguments to SPMD functions.
 * Defined as a union of structs, one struct for every SPMD function.
 */

#define SAC_MT_MYTHREAD() SAC_MT_mythread

#define SAC_MT_MYTHREAD_PARAM() const unsigned int SAC_MT_mythread

#if SAC_DO_THREADS_STATIC

/***
 ***   Definitions and declarations specific to the case where the exact number
 ***   of threads is known statically.
 ***/

#define SAC_MT_THREADS() SAC_SET_THREADS

#else /* SAC_DO_THREADS_STATIC */

/***
 ***   Definitions and declarations specific to the case where the exact number
 ***   of threads is determined dynamically.
 ***/

#define SAC_MT_THREADS() SAC_MT_threads

#endif /* SAC_DO_THREADS_STATIC */

SAC_C_EXTERN unsigned int SAC_MT_threads;
SAC_C_EXTERN volatile unsigned int SAC_MT_not_yet_parallel;
SAC_C_EXTERN pthread_key_t SAC_MT_threadid_key;

SAC_C_EXTERN void SAC_COMMON_MT_SetupInitial (int argc, char *argv[],
                                              unsigned int num_threads,
                                              unsigned int max_threads);
/*
 * REMARK:
 *
 * no volatile for the function return value here, as volatile has
 * no effect for rvalues! And a function return value is a rvalue.
 */

/*****************************************************************************/

#else /* SAC_DO_MULTITHREAD */

SAC_C_EXTERN volatile unsigned int SAC_MT_not_yet_parallel;

/***
 ***   Definitions and declarations for sequential execution (dummies)
 ***/

#define SAC_MT_THREADS() 1

#define SAC_MT_MYTHREAD() 0

#define SAC_MT_SETUP()
#define SAC_MT_SETUP_INITIAL()

#define SAC_MT_DEFINE()

#define SAC_MT_DEFINE_LOCK(name)

#define SAC_MT_DECLARE_LOCK(name)

#define SAC_MT_ACQUIRE_LOCK(name)

#define SAC_MT_RELEASE_LOCK(name)

#define SAC_ND_PROP_OBJ_IN()

#define SAC_ND_PROP_OBJ_OUT()

#define SAC_ND_PROP_OBJ_UNBOX(unboxed, boxed)

#define SAC_ND_PROP_OBJ_BOX(boxed, unboxed)                                              \
    SAC_ND_A_FIELD (boxed) = SAC_ND_A_FIELD (unboxed);

/*****************************************************************************/

#endif /* SAC_DO_MULTITHREAD */

#endif /* SAC_SIMD_COMPILATION */

#endif /* _SAC_MT_H_ */
