
#include <stdio.h>
#include <stdlib.h>

#include "ctest.h"
#include "l2.h"
#include "accesstime.h"
#include "bits.h"
#include "verbose.h"

int *
GenVectSizeL2 (int cs, cache_ptr Cache)
{
    int cs_elems, l1_elems, l1_assoc, bits, act, next, i;
    int *field;
    int stride;

    cs_elems = cs * 1024 / sizeof (int);
    l1_elems = Cache->cachesize[0] * 1024 / sizeof (int);
    l1_assoc = Cache->associativity[0];
    field = heap;

    bits = MaxBit (cs_elems - 1);
    act = 0;
    next = ReverseBits (1, bits);
    stride = Cache->linesize[0] / sizeof (int);

    for (i = 0; i < cs_elems / 2; i += stride) {
        field[act + i] = next + i;
        field[next + i] = i + stride;
    }

    field[next + i - stride] = 0;

    MESS (("  trying size %sB.... ", Mem2Str (cs)));

    return (field);
}

int *
GenVectAssocL2 (int steps, cache_ptr Cache)
{
    int i, j, big_stride, small_stride;
    int *field;
    int cs_elems;
    int act, next;

    cs_elems = Cache->cachesize[1] * 1024 / sizeof (int);
    field = heap;

    MESS (("  trying assoc %d .... ", steps));

    if (steps == 1) {
        big_stride = cs_elems / 2;
        steps = 2;
    } else {
        big_stride = 2 * cs_elems / steps;
    }
    small_stride = Cache->linesize[0] / sizeof (int);
    act = 0;

    for (i = 0; i < cs_elems / (steps * small_stride); i++) {
        for (j = 0; j < steps - 1; j++) {
            next = act + big_stride;
            field[act] = next;
            act = next;
        }
        next = act - (steps - 1) * big_stride + small_stride;
        field[act] = next;
        act = next;
    }
    field[act - small_stride + (steps - 1) * big_stride] = 0; /* ensure looping! */

    return (field);
}

cache_ptr
GetL2Specs (cache_ptr Cache, int accs)
{
    int l2, l2ass;

    MESS (("Please note, that the numbers for L2 cache heavily\n"));
    MESS (("depend on the mapping of virtual to physical addresses!\n"));
    MESS (("Therefore, the cache parameters found may differ from the\n"));
    MESS (("parameters of the cache actually used. However, the memory\n"));
    MESS (("access pattern used for the measurements simulate array\n"));
    MESS (("traversals similar to the code generated by sac2c.\n\n"));
    MESS (("To obtain best results you should run this program several times\n"));
    MESS (("and compare its findings. It might even be necessary to vary\n"));
    MESS (("the filter settings ( see 'ctest -h' for help) to obtain good\n"));
    MESS (("results!\n\n"));

    filter = filter_size;

    if (Cache->cachesize[1] == 0) {
        MESS ((" checking L2 cache...\n"));
        l2 = FindStep (2 * Cache->cachesize[0], csmax, Cache, accs, GenVectSizeL2,
                       MeasureReadLoop);

        if (l2 == csmax) {
            MESS (("\nNo level 2 cache found! You may either increase cmax\n"));
            MESS (("or decrease fs (see 'ctest -h')\n\n"));
        } else {
            Cache->exists[1] = 1;
            MESS ((" L2 size found: %sB\n", Mem2Str (l2)));
            Cache->cachesize[1] = l2;
        }

    } else {
        Cache->exists[1] = 1;
        MESS ((" L2 size preset to %sB\n", Mem2Str (Cache->cachesize[1])));
    }

    if (Cache->exists[1] != 0) {
        if (Cache->associativity[1] == 0) {
            filter = filter_assoc;
            l2ass = FindStep (1, MAXASSOC, Cache, accs, GenVectAssocL2, MeasureReadLoop);
            MESS ((" L2 assoc found: %d\n", l2ass));
            Cache->associativity[1] = l2ass;
        } else {
            MESS ((" L2 assoc preset to %d\n", Cache->associativity[1]));
        }
    }

    return (Cache);
}