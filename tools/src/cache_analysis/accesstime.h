

#ifndef _accesstime_h
#define _accesstime_h

#include <sys/time.h>
#include <sys/resource.h>

typedef struct rusage stimer_t;
typedef int *(*genvectfun_ptr) (int, cache_ptr);
typedef double (*measurefun_ptr) (int, int *);

extern stimer_t start_timer;
extern stimer_t stop_timer;

/*****************************************************************************/

#define START_CLOCK() getrusage (RUSAGE_SELF, &start_timer)
#define STOP_CLOCK() getrusage (RUSAGE_SELF, &stop_timer)

/*****************************************************************************/

extern double CalcTime (double divisor);
extern char *Mem2Str (int cs);
extern char *Time2Str (double utime);

extern int CheckLoopRange (int loops, int maxchecks);
extern double MeasureNoopLoop (int iter, int *arr);
extern double MeasureReadLoop (int iter, int *arr);
extern double MeasureWriteLoop (int iter, int *arr);

extern int FindStep (int initial, int max, cache_ptr Cache, int accs,
                     genvectfun_ptr GenVectFun, measurefun_ptr MeasureFun);

/*****************************************************************************/

#endif
