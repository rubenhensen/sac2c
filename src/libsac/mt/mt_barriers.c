/*
 * global variables used for the pthread barriers.
 * FIXME: these should be prefixed with SACxxx
 */

#include <pthread.h>
#include "mt_barriers.h"

int mutex_nr_threads;
int mutex_thread_count;
pthread_mutex_t mutex_sacred;
pthread_mutex_t mutex_lock_sacred;
pthread_mutex_t mutex_barrier;

pthread_cond_t cond_barrier;
pthread_mutex_t cond_mutex;

#ifndef __APPLE__
pthread_barrier_t pthread_barrier;
#endif
