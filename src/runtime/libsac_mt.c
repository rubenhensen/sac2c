/*****************************************************************************
 *
 * file:
 *
 * prefix:
 *
 * description:
 *
 *
 *
 *
 *
 *
 *****************************************************************************/

#include <pthread.h>

/*
 * Definition of global variables.
 */

pthread_attr_t SAC_MT_thread_attribs;

void *SAC_MT_barrier;

volatile unsigned int SAC_MT_worker_flag = 0;

unsigned int SAC_MT_not_yet_parallel = 1;

unsigned int SAC_MT_masterclass;

unsigned int SAC_MT_threads;

volatile unsigned int(SAC_MT_spmd_function) (unsigned int, unsigned int, unsigned int);

/*
 * Definition of thread control functions.
 */

static void
ThreadControl (void *arg)
{
    unsigned int wait_flag = 0;
    unsigned int i;
    unsigned int my_worker_class = ((unsigned int)arg) >> 17;
    const unsigned int my_thread_id = ((unsigned int)arg) & 0xFFFF;

    while (my_thread_id + my_worker_class >= SAC_MT_threads) {
        my_worker_class >>= 1;
    }

    for (i = my_worker_class; i; i >>= 1) {

        pthread_create (NULL, &SAC_MT_thread_attribs, (void *(*)(void *))ThreadControl,
                        (void *)((i << 16) + (my_thread_id + i)));
    }

    for (;;) {
        while (wait_flag == SAC_MT_worker_flag)
            ;
        wait_flag = SAC_MT_worker_flag;

        wait_flag = SAC_MT_spmd_function (my_thread_id, my_worker_class, wait_flag);
    }
}

void
SAC_MT_ThreadControl (void *arg)
{
    unsigned int wait_flag = 0;
    unsigned int i;

    for (i = SAC_MT_masterclass; i > 1; i >>= 1) {

        pthread_create (NULL, &SAC_MT_thread_attribs, (void *(*)(void *))ThreadControl,
                        (void *)((i << 16) + i));
    }

    for (;;) {
        while (wait_flag == SAC_MT_worker_flag)
            ;
        wait_flag = SAC_MT_worker_flag;

        wait_flag = SAC_MT_spmd_function (1, 0, wait_flag);
    }
}
