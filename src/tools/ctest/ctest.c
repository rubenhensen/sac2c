
#include <stdio.h>

#include "getoptions.h"
#include "verbose.h"
#include "ctest.h"
#include "accesstime.h"
#include "l1.h"
#include "l2.h"

/***************************  global variables  ******************************/

int mintime;
int csmin;
int csmax;
int filter;
int filter_size;
int filter_assoc;

int *heap;

/*****************************************************************************
 *
 *  function:
 *    void SetDefaults()
 *
 *  description:
 *    This function initializes all global variables with the default values.
 */

void
SetDefaults (cache_ptr Cache)
{
    int i;

    verbose = 3;

    for (i = 0; i < 3; i++) {
        Cache->exists[i] = 0;
        Cache->cachesize[i] = 0;
        Cache->linesize[i] = 0;
        Cache->associativity[i] = 0;
        Cache->policy[i] = "d";
    }

    mintime = 1;
    csmin = 4;
    csmax = 32768;
    filter = 20;
    filter_size = 50;
    filter_assoc = 40;

    heap = (int *)malloc (csmax * 2048 + MAXLINESIZE);
}

/*****************************************************************************
 *
 *  function:
 *    void PrintCache( cache_ptr Cache)
 *
 *  description:
 *    prints the cache settings found in a sac2crc suited form.
 */

void
PrintCache (cache_ptr Cache)
{
    int i;

    for (i = 0; i < 3; i++) {
        if (Cache->exists[i] != 0) {
            printf ("CACHE%d_SIZE     := %d\n", i + 1, Cache->cachesize[i]);
            if (Cache->linesize[i] != 0) {
                printf ("CACHE%d_LINE     := %d\n", i + 1, Cache->linesize[i]);
            }
            if (Cache->associativity[i] != 0) {
                printf ("CACHE%d_ASSOC    := %d\n", i + 1, Cache->associativity[i]);
            }
            if (strcmp (Cache->policy[i], "d") != 0) {
                printf ("CACHE%d_WRITEPOL := %s\n", i + 1, Cache->policy[i]);
            }
            printf ("\n");
        }
    }
}

/*****************************************************************************
 *
 *  function:
 *    void Usage()
 *
 *  description:
 *    prints usage info to stdout.
 */

#define HEADER "   "
void
Usage (void)
{
    printf ("ctest\n");
    printf (HEADER "-h              print this help mesage.\n");
    printf (HEADER "-mintime <num>  minimum runtime for each measurement"
                   " [10th of a sec] (default: %d).\n",
            mintime);
    printf (HEADER "-csmin <num>    minimum cache size to be looked for"
                   " [kB] (default: %d).\n",
            csmin);
    printf (HEADER "-csmax <num>    maximum cache size to be looked for"
                   " [kB] (default: %d).\n",
            csmax);
    printf (HEADER "-v <num>        verbose-level [0-3 0:'results only']"
                   " (default: %d).\n",
            verbose);
    printf (HEADER
            "-fs <num>       sets the tolerance for finding L2 size to <num> per cent."
            " (default: %d)\n",
            filter_size);
    printf (HEADER
            "-fa <num>       sets the tolerance for finding L2 associativity to <num>"
            " per cent. (default: %d)\n\n",
            filter_assoc);
    printf (HEADER "-l1s <num>      Preset L1 size to <num> kB.\n");
    printf (HEADER "-l1a <num>      Preset L1 associativity to <num>.\n");
    printf (HEADER "-l1ls <num>     Preset L1 line size to <num> B.\n\n");

    printf (HEADER "-l2s <num>      Preset L2 size to <num> kB.\n");
    printf (HEADER "-l2a <num>      Preset L2 associativity to <num>.\n");
    printf (HEADER "-l2ls <num>     Preset L2 line size to <num> B.\n");
}

/*****************************************************************************/

int
main (int argc, char *argv[])
{
    int accs;
    cache_t Cache, *Cache_p;

    Cache_p = &Cache;
    SetDefaults (Cache_p);

    ARGS_BEGIN (argc, argv);
    ARGS_FLAG ("h", Usage (); exit (0));
    ARGS_OPTION ("v", ARG_INT (verbose));
    ARGS_OPTION ("fs", ARG_INT (filter_size));
    ARGS_OPTION ("fa", ARG_INT (filter_assoc));

    ARGS_OPTION ("l1s", ARG_INT (Cache.cachesize[0]));
    ARGS_OPTION ("l1a", ARG_INT (Cache.associativity[0]));
    ARGS_OPTION ("l1ls", ARG_INT (Cache.linesize[0]));

    ARGS_OPTION ("l2s", ARG_INT (Cache.cachesize[1]));
    ARGS_OPTION ("l2a", ARG_INT (Cache.associativity[1]));
    ARGS_OPTION ("l2ls", ARG_INT (Cache.linesize[1]));

    ARGS_OPTION ("mintime", ARG_INT (mintime));
    ARGS_OPTION ("csmin", ARG_INT (csmin));
    ARGS_OPTION ("csmax", ARG_INT (csmax));
    ARGS_END ();

    MESS_EXT (("heap allocated at %08x!\n", heap));
    while (((long)heap & (MAXLINESIZE - 1)) != 0) { /* align heap to MAXLINESIZE! */
        heap++;
    }
    MESS_EXT (("heap aligned to   %08x!\n", heap));

    MESS_EXT (("adr of            main is %08x!\n", main));
    MESS_EXT (("adr of MeasureReadLoop is %08x!\n", MeasureReadLoop));

    MESS (("\nCalibrating loop sizes:\n"));
    accs = CheckLoopRange (MINLOOPS, 4);
    MESS ((" loop size chosen: %s\n", Mem2Str (accs / 1024)));

    MESS (("\nLooking for number of cachelevels and cachesizes:\n"));
    Cache_p = GetL1Specs (Cache_p, accs);
    Cache_p = GetL2Specs (Cache_p, accs);

    MESS (("\nSettings found:\n\n"));
    PrintCache (Cache_p);

    return (0);
}
