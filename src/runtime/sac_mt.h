/*
 *
 * $Log$
 * Revision 1.2  1998/05/07 11:15:41  cg
 * minor errors removed
 *
 * Revision 1.1  1998/05/07 08:38:05  cg
 * Initial revision
 *
 *
 *
 */

/*****************************************************************************
 *
 * file:   sac_mt.h
 *
 * prefix: SAC_MT_
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It is the major header file of the implementation of the multi-threading
 *   facilities.
 *
 *
 *
 *****************************************************************************/

#ifndef SAC_MT_H

#define SAC_MT_H

/*****************************************************************************/

#if SAC_DO_MULTITHREAD

/***
 ***   Definitions and declarations for the multi-threaded runtime system
 ***/

#include <pthread.h>

/*
 * Definition of synchronisation barrier data type.
 */

typedef union {
    union {
        int result_int;
        float result_float;
        double result_double;
        char result_char;
        void *result_array;
        void *result_hidden;
    } b[SAC_SET_MAX_SYNC_FOLD + 1];
} SAC_MT_barrier_dummy_t;

#define SAC_MT_MAX(a, b) ((a) > (b) ? (a) : (b))

#define SAC_MT_CACHE_LINE_MAX()                                                          \
    SAC_MT_MAX (SAC_SET_CACHE_1_LINE,                                                    \
                SAC_MT_MAX (SAC_SET_CACHE_2_LINE, SAC_MT_MAX (SAC_SET_CACHE_3_LINE, 1)))

#define SAC_MT_BARRIER_OFFSET()                                                          \
    ((SAC_MT_CACHE_LINE_MAX () >= sizeof (SAC_MT_barrier_dummy_t))                       \
       ? SAC_MT_CACHE_LINE_MAX ()                                                        \
       : (SAC_MT_CACHE_LINE_MAX ()                                                       \
          * (((sizeof (SAC_MT_barrier_dummy_t) - 1) / SAC_MT_CACHE_LINE_MAX ()) + 1)))

typedef union {
    char dummy[SAC_MT_BARRIER_OFFSET ()];
    union {
        int result_int;
        float result_float;
        double result_double;
        char result_char;
        void *result_array;
        void *result_hidden;
    } b[SAC_SET_MAX_SYNC_FOLD + 1];
} SAC_MT_barrier_t;

/*
 *  Basic access macros for synchronisation barrier.
 */

#define SAC_MT_BARRIER_READY(barrier, n) (barrier[n].b[0].result_int)

#define SAC_MT_BARRIER_RESULT(barrier, n, m, type) (barrier[n].b[m].result_##type)

/*
 *  Advanced access macros for synchronisation barrier.
 */

#define SAC_MT_CLEAR_BARRIER(n) SAC_MT_BARRIER_READY (SAC_MT_barrier, n) = 0;

#define SAC_MT_SET_BARRIER(n, m) SAC_MT_BARRIER_READY (SAC_MT_barrier, n) = m;

#define SAC_MT_CHECK_BARRIER(n) (SAC_MT_BARRIER_READY (SAC_MT_barrier, n))

#define SAC_MT_SET_BARRIER_RESULT(n, m, type, res)                                       \
    {                                                                                    \
        SAC_MT_BARRIER_RESULT (SAC_MT_barrier, n, m, type) = res;                        \
        SAC_MT_BARRIER_READY (SAC_MT_barrier, n) = m;                                    \
    }

#define SAC_MT_GET_BARRIER_RESULT(n, m, type)                                            \
    (SAC_MT_BARRIER_RESULT (SAC_MT_barrier, n, m, type))

/*
 *  Definition of macro implemented ICMs for global symbol definitions
 */

#define SAC_MT_DEFINE() SAC_MT_DEFINE_BARRIER ()

#if SAC_MT_CACHE_LINE_MAX()

#define SAC_MT_DEFINE_BARRIER()                                                          \
    SAC_MT_barrier_t SAC_MT_barrier_space[SAC_SET_THREADS_MAX + 1];

#else /* SAC_MT_CACHE_LINE_MAX() */

#define SAC_MT_DEFINE_BARRIER()                                                          \
    SAC_MT_barrier_t SAC_MT_barrier_space[SAC_SET_THREADS_MAX];

#endif /* SAC_MT_CACHE_LINE_MAX() */

#define SAC_MT_DEFINE_ARG_BUFFER_BEGIN() static union {

#define SAC_MT_DEFINE_ARG_BUFFER_END()                                                   \
    }                                                                                    \
    SAC_MT_argument_buffer;

#define SAC_MT_DEFINE_ARG_BUFFER_ENTRY_BEGIN(name) struct {

#define SAC_MT_DEFINE_ARG_BUFFER_ENTRY_END(name)                                         \
    }                                                                                    \
    name;

#define SAC_MT_DEFINE_ARG_BUFFER_ENTRY_ITEM(type, item) type name;

/*
 *  Definition of macro implemented ICMs for setting up the multi-threaded
 *  runtime system.
 */

#define SAC_MT_SETUP()                                                                   \
    {                                                                                    \
        SAC_MT_SETUP_BARRIER ();                                                         \
        SAC_MT_SETUP_NUMTHREADS ();                                                      \
        SAC_MT_SETUP_MASTERCLASS ();                                                     \
        SAC_MT_SETUP_PTHREAD ();                                                         \
        SAC_MT_SETUP_MASTER ();                                                          \
    }

#define SAC_MT_SETUP_PTHREAD()                                                           \
    {                                                                                    \
        pthread_attr_init (&SAC_MT_thread_attribs);                                      \
        pthread_attr_setscope (&SAC_MT_thread_attribs, PTHREAD_SCOPE_SYSTEM);            \
        pthread_attr_setdetachstate (&SAC_MT_thread_attribs, PTHREAD_CREATE_DETACHED);   \
        pthread_create (NULL, &SAC_MT_thread_attribs,                                    \
                        (void *(*)(void *))SAC_MT_ThreadControl, NULL);                  \
    }

#if SAC_MT_CACHE_LINE_MAX()

#define SAC_MT_SETUP_BARRIER()                                                           \
    {                                                                                    \
        SAC_MT_barrier = (SAC_MT_barrier_t *)((char *)(SAC_MT_barrier_space + 1)         \
                                              - ((unsigned long int)SAC_MT_barrier_space \
                                                 % SAC_MT_BARRIER_OFFSET ()));           \
    }

#else /* SAC_MT_CACHE_LINE_MAX() */

#define SAC_MT_SETUP_BARRIER()
{
    SAC_MT_barrier = SAC_MT_barrier_space;
}

#endif /* SAC_MT_CACHE_LINE_MAX() */

/*
 *  Definition of macro implemented ICMs for managing multi-threaded execution.
 */

#define SAC_MT_ENDIF_SEQUENTIAL()

/*
 *  Definition of macro implemented ICMs for handling of spmd-function
 */

#define SAC_MT_SPMD_START(name)                                                          \
    {                                                                                    \
        SAC_MT_worker_flag = 1 - SAC_MT_worker_flag;                                     \
        name (0, SAC_MT_MASTERCLASS (), 0);                                              \
    }

#define SAC_MT_SPMD_FUN_ARGLIST()                                                        \
    unsigned int SAC_MT_mythread, unsigned int SAC_MT_myworkerclass,                     \
      unsigned int SAC_MT_wait_flag,

#define SAC_MT_SPMD_FUN_RETURN() return (SAC_MT_wait_flag);

#define SAC_MT_MYTHREAD() SAC_MT_mythread

#define SAC_MT_MYWORKERCLASS() SAC_MT_myworkerclass

#define SAC_MT_SPMD_FUN_PARAM(name, type, arg) type arg = SAC_MT_argument_buffer.name.arg;

#define SAC_MT_SPMD_FUN_ARG(name, arg) SAC_MT_argument_buffer.name.arg = arg;

/*
 *  Definition of macro implemented ICMs for synchronisation
 */

#define SAC_MT_SYNC_NONFOLD()                                                            \
    {                                                                                    \
        unsigned int i;                                                                  \
                                                                                         \
        for (i = 1; i <= SAC_MT_MYWORKERCLASS (); i <<= 1) {                             \
            while (!SAC_MT_CHECK_BARRIER (SAC_MT_MYTHREAD () + i))                       \
                ;                                                                        \
        }                                                                                \
                                                                                         \
        SAC_MT_SET_BARRIER (SAC_MT_MYTHREAD (), 1);                                      \
                                                                                         \
        for (i = 1; i <= SAC_MT_MYWORKERCLASS (); i <<= 1) {                             \
            SAC_MT_CLEAR_BARRIER (SAC_MT_MYTHREAD () + i);                               \
        }                                                                                \
    }

#define SAC_MT_SYNC_ONEFOLD(type, accu_var, tmp_var, foldop)                             \
    {                                                                                    \
        if (!SAC_MT_MYWORKERCLASS ()) {                                                  \
            SAC_MT_SET_BARRIER_RESULT (SAC_MT_MYTHREAD, 1, type, accu_var);              \
        } else {                                                                         \
            unsigned int SAC_MT_ready_count = SAC_MT_MYWORKERCLASS () >> 1;              \
            unsigned int SAC_MT_son_id;                                                  \
            unsigned int SAC_MT_i;                                                       \
                                                                                         \
            for (;;) {                                                                   \
                SAC_MT_i = SAC_MT_MYWORKERCLASS ();                                      \
                                                                                         \
                do {                                                                     \
                    SAC_MT_son_id = SAC_MT_MYTHREAD () + SAC_MT_i;                       \
                                                                                         \
                    if (SAC_MT_CHECK_BARRIER (SAC_MT_son_id)) {                          \
                        SAC_MT_CLEAR_BARRIER (SAC_MT_son_id);                            \
                        tmp_var = SAC_MT_GET_BARRIER_RESULT (SAC_MT_son_id, 1, type);    \
                                                                                         \
                        foldop;                                                          \
                                                                                         \
                        if (!SAC_MT_ready_count) {                                       \
                            SAC_MT_SET_BARRIER_RESULT (SAC_MT_MYTHREAD (), 1, type,      \
                                                       accu_var);                        \
                            goto SAC_MT_ENDSYNC;                                         \
                        }                                                                \
                        SAC_MT_ready_count >>= 1;                                        \
                    }                                                                    \
                    SAC_MT_i >>= 1;                                                      \
                } while (SAC_MT_i);                                                      \
            }                                                                            \
        }                                                                                \
    SAC_MT_ENDSYNC:                                                                      \
    }

/*
 *  Declarations of global variables and functions defined in libsac_mt.c
 */

extern pthread_attr_t SAC_MT_thread_attribs;

extern SAC_MT_barrier_t *SAC_MT_barrier;

extern volatile unsigned int SAC_MT_worker_flag;

extern unsigned int SAC_MT_not_yet_parallel;

extern unsigned int SAC_MT_masterclass;

extern unsigned int SAC_MT_threads;

extern volatile unsigned int(SAC_MT_spmd_function) (unsigned int, unsigned int,
                                                    unsigned int);

extern void SAC_MT_ThreadControl (void *arg);

/*****************************************************************************/

#if SAC_DO_THREADS_STATIC

/***
 ***   Definitions and declarations specific to the case where the exact number
 ***   of threads is knwon statically.
 ***/

#define SAC_MT_THREADS() SAC_SET_THREADS

#define SAC_MT_MASTERCLASS() SAC_SET_MASTERCLASS

#define SAC_MT_SETUP_NUMTHREADS()                                                        \
    {                                                                                    \
        SAC_MT_threads = SAC_SET_THREADS;                                                \
    }

#define SAC_MT_SETUP_MASTERCLASS()                                                       \
    {                                                                                    \
        SAC_MT_masterclass = SAC_SET_MASTERCLASS;                                        \
    }

#define SAC_MT_SETUP_MASTER()                                                            \
    {                                                                                    \
        unsigned int i;                                                                  \
                                                                                         \
        for (i = 1; i <= SAC_SET_MASTERCLASS; i <<= 1) {                                 \
            SAC_MT_CLEAR_BARRIER (i);                                                    \
        }                                                                                \
    }

/*****************************************************************************/

#else /* SAC_DO_THREADS_STATIC */

/***
 ***   Definitions and declarations specific to the case where the exact number
 ***   of threads is determined dynamically.
 ***/

#define SAC_MT_THREADS() SAC_MT_threads

#define SAC_MT_MASTERCLASS() SAC_MT_masterclass

#define SAC_MT_SETUP_NUMTHREADS()                                                        \
    {                                                                                    \
        unsigned int i;                                                                  \
                                                                                         \
        for (i = 1; i < __argc; i++) {                                                   \
            if (0 == strncmp (__argv[i], "-threads=", 9)) {                              \
                SAC_MT_threads = atoi (__argv[i] + 9);                                   \
                if ((SAC_MT_threads > 0)                                                 \
                    && (SAC_MT_threads < (SAC_SET_THREADS_MAX + 1))) {                   \
                    break;                                                               \
                }                                                                        \
                                                                                         \
                SAC_RuntimeError ("Number of threads exceeds legal range (1 to %d)",     \
                                  SAC_SET_THREADS_MAX);                                  \
            }                                                                            \
        }                                                                                \
    }

#define SAC_MT_SETUP_MASTERCLASS()                                                       \
    {                                                                                    \
        for (SAC_MT_masterclass = 1; SAC_MT_masterclass < SAC_MT_threads;                \
             SAC_MT_masterclass <<= 1) {                                                 \
            SAC_MT_CLEAR_BARRIER (SAC_MT_masterclass);                                   \
        }                                                                                \
                                                                                         \
        SAC_MT_masterclass >>= 1;                                                        \
    }

#define SAC_MT_SETUP_MASTER()

#endif /* SAC_DO_THREADS_STATIC */

/*****************************************************************************/

#else /* SAC_DO_MULTITHREAD */

/***
 ***   Definitions and declarations for sequential execution (dummies)
 ***/

#define SAC_MT_SETUP()

#define SAC_MT_DEFINE()

#define SAC_MT_DEFINE_ARG_BUFFER_BEGIN()

#define SAC_MT_DEFINE_ARG_BUFFER_END()

#define SAC_MT_DEFINE_ARG_BUFFER_ENTRY_BEGIN(name)

#define SAC_MT_DEFINE_ARG_BUFFER_ENTRY_END(name)

#define SAC_MT_DEFINE_ARG_BUFFER_ENTRY_ITEM(type, item)

/*****************************************************************************/

#endif /* SAC_DO_MULTITHREAD */

#endif /* SAC_MT_H */
