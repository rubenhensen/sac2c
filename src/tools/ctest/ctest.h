

#ifndef _ctest_h
#define _ctest_h

typedef struct CACHE_T {
    int exists[3];
    int cachesize[3];
    int linesize[3];
    int associativity[3];
    const char *policy[3];
} cache_t, *cache_ptr;

extern int mintime;
extern int csmin;
extern int csmax;
extern int verbose;
extern int filter;
extern int filter_size;
extern int filter_assoc;

extern int *heap;

/*****************************************************************************/

#define MINLOOPS 16384
#define MAXASSOC 256
#define MAXLINESIZE 256

#define START_CLOCK() getrusage (RUSAGE_SELF, &start_timer)
#define STOP_CLOCK() getrusage (RUSAGE_SELF, &stop_timer)
#define MIN(a, b) ((a < b) ? (a) : (b))
#define MAX(a, b) ((a > b) ? (a) : (b))
#define ABS(val) (((val) < 0) ? (-(val)) : (val))

#endif
