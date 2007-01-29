
#include <stdio.h>
#include <stdlib.h>

#include "ctest.h"
#include "accesstime.h"
#include "verbose.h"

/***************************  global variables  ******************************/

stimer_t start_timer;
stimer_t stop_timer;

/*****************************************************************************
 *
 *  function:
 *    double CalcTime(double divisor)
 *
 *  description:
 *    This function results the used time in microseconds of a program segment
 *    which has to be framed by the macros START_CLOCK() and STOP_CLOCK()
 *    divided by the number of accesses (given as divisor).
 *    The macros START_CLOCK() and STOP_CLOCK() are using the global variable
 *    start_timer.
 */
double
CalcTime (double divisor)
{
    double ut1, ut2, st1, st2, result;

    ut1 = (double)start_timer.ru_utime.tv_usec;
    st1 = (double)start_timer.ru_utime.tv_sec;
    ut2 = (double)stop_timer.ru_utime.tv_usec;
    st2 = (double)stop_timer.ru_utime.tv_sec;
    result = ((ut2 - ut1) + 1000000 * (st2 - st1)) / (divisor);

    return (result);
}

/*****************************************************************************
 *
 *  function:
 *    char* Mem2Str( int cs)
 *
 *  description:
 *    input: cachesize or memorysize in kilobytes.
 *    The result of this function is a string representing a memorysize.
 *    If the the input is less than 1024 the memorysize will be resulted with
 *    a following "kB" otherwise the memorysize divided by 1024 will be
 *    resulted with a following "MB".
 */
char *
Mem2Str (int cs)
{
    char *buf;

    buf = (char *)malloc (10 * sizeof (char));
    if (cs < 1024) {
        sprintf (buf, "%3d k", cs);
    } else {
        sprintf (buf, "%3d M", (cs / 1024));
    }

    return (buf);
}

/*******************************************************************************
 *  function:
 *    char* Time2Str( double utime)
 *
 *  description:
 *    input: time in microseconds.
 *    The format of the output string is: <time> <unit> where <unit> is one of
 *    "nsec", "micsec", "msec", and "sec".
 */
char *
Time2Str (double utime)
{
    char *pref[] = {"n", "mic", "m", ""};
    char *buf;
    int counter = 0;

    utime *= 1000;
    buf = (char *)malloc (20 * sizeof (char));
    while (utime > 1000) {
        utime *= 0.001;
        counter++;
    }
    sprintf (buf, "%6.2f %ssec", utime, pref[counter]);

    return (buf);
}

/******************************************************************************
 *  function:
 *    int CheckLoopRange(int loops, int maxchecks)
 *
 *  description:
 *    loops is the initial guess for the number of loops to be taken.
 *    maxchecks gives the maximum number of re-tries allowed!
 *
 *******************************************************************************/
int
CheckLoopRange (int loops, int maxchecks)
{
    int i, err;
    int res;
    double utime = 0;
    double stime;
    double mint, maxt;

    if (maxchecks > 0) {
        res = 0;

        MESS ((" checking %s accesses.... ", Mem2Str (loops / 1024)));
        err = START_CLOCK ();
        for (i = 0; i < loops; i++) {
            res++;
        }
        err = STOP_CLOCK ();

        utime = CalcTime (1);
        MESS (("  %s\n", Time2Str (utime)));

        if (utime < 1000) {
            loops = CheckLoopRange (loops * 1024, maxchecks - 1);
        } else {
            stime = utime / 1000000;
            mint = (double)mintime / 10;
            maxt = 2.2 * (double)mint;
            if ((stime < mint) || (stime > maxt)) {
                while (stime < mint) {
                    stime *= 2;
                    loops *= 2;
                }
                while (stime > maxt) {
                    stime /= 2;
                    loops /= 2;
                }
                loops = CheckLoopRange (loops, maxchecks - 1);
            }
        }
    }

    return (loops);
}

/*****************************************************************************
 *
 *  function:
 *    double MeasureNoopLoop( int iter, int * arr )
 *
 *  description:
 *    reads arr[0] iter times. The time required is returned.
 *
 *****************************************************************************/

double
MeasureNoopLoop (int iter, int *arr)
{
    register int i;
    register int idx = 0;

    START_CLOCK ();
    for (i = 0; i < iter; i++) {
        idx = 1;
    }
    STOP_CLOCK ();
    return (CalcTime (iter));
}

/*****************************************************************************
 *
 *  function:
 *    double MeasureReadLoop( int iter, int * arr )
 *
 *  description:
 *    iter times reads from arr[idx] where to read the next index idx.
 *    This allows the content of arr to stear the order in which the
 *    elements of arr are read! Since iter may exceed size(arr) it should
 *    be made sure, that the chain of indices loops!
 *    The time required is returned.
 *
 *****************************************************************************/

double
MeasureReadLoop (int iter, int *arr)
{
    register int i;
    register int idx = 0;

    START_CLOCK ();
    for (i = 0; i < iter; i += 3) {
        idx = arr[idx];
        idx = arr[idx];
        idx = arr[idx];
    }
    STOP_CLOCK ();

    return (CalcTime (iter));
}

/*****************************************************************************
 *
 *  function:
 *    double MeasureWriteLoop( int iter, int * arr )
 *
 *  description:
 *    iter times reads from arr[idx] where to read the next index idx.
 *    This allows the content of arr to stear the order in which the
 *    elements of arr are read! Since iter may exceed size(arr) it should
 *    be made sure, that the chain of indices loops!
 *    The time required is returned.
 *
 *****************************************************************************/

double
MeasureWriteLoop (int iter, int *arr)
{
    register int i;
    register int idx = 0;

    START_CLOCK ();
    for (i = 0; i < iter; i += 3) {
        arr[arr[idx++]] = 1;
        idx = arr[idx];
    }
    STOP_CLOCK ();

    return (CalcTime (iter));
}

/*****************************************************************************
 *
 *  function:
 *    int FindStep( int initial, int max, cache_ptr Cache, int accs,
 *                  genvectfun_ptr GenVectFun, measurefun_ptr MeasureFun)
 *
 *  description:
 *
 *****************************************************************************/

int
FindStep (int initial, int max, cache_ptr Cache, int accs, genvectfun_ptr GenVectFun,
          measurefun_ptr MeasureFun)
{
    int test;
    double time, oldtime;
    int *cache;

    /*
     * First, we make a dummy measurement in order to initialize time / oldtime
     */
    cache = GenVectFun (initial, Cache);
    time = MeasureFun (accs, cache);

    oldtime = time;
    test = initial;

    while ((test <= max)
           && ((time / oldtime < 1.0 + filter / 100.0)
               && (oldtime / time < 1.0 + filter / 100.0))) {
        oldtime = time;

        if (test != initial) {
            cache = GenVectFun (test, Cache);
        }
        time = MeasureFun (accs, cache);

        MESS (("  %s\n", Time2Str (time)));
        test *= 2;
    }
    if ((time / oldtime >= 1.0 + filter / 100.0)
        || (oldtime / time >= 1.0 + filter / 100.0))
        return (test / 4);
    else
        return (test / 2);
}
