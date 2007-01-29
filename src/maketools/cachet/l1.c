
#include <stdio.h>
#include <stdlib.h>

#include "ctest.h"
#include "l1.h"
#include "accesstime.h"
#include "bits.h"
#include "verbose.h"

int *
GenVectSizeL1 (int cs, cache_ptr Cache)
{
    int *field;
    int cs_elems, act, next;
    int i, bits;

    cs_elems = cs * 1024 / sizeof (int);
    field = heap;

    bits = MaxBit (cs_elems - 1);
    act = 0;
    for (i = 0; i < cs_elems; i++) {
        next = ReverseBits (i + 1, bits);
        field[act] = next;
        act = next;
    }
    MESS (("  trying size %sB.... ", Mem2Str (cs)));
    return (field);
}

int *
GenVectAssocL1 (int steps, cache_ptr Cache)
{
    int *field;
    int cs_elems, act, next;
    int i, stride;

    cs_elems = Cache->cachesize[0] * 1024 / sizeof (int);
    field = heap;

    stride = 2 * cs_elems / steps;
    act = 0;
    for (i = 0; i < steps - 1; i++) {
        next = act + stride;
        field[act] = next;
        act = next;
    }
    field[act] = 0; /* ensure looping! */

    MESS (("  trying assoc %d .... ", steps));

    return (field);
}

int *
GenVectLineL1 (int offset, cache_ptr Cache)
{
    int *field;
    int cs_elems, act, next;
    int off_elems;
    int i, stride;

    cs_elems = Cache->cachesize[0] * 1024 / sizeof (int);
    off_elems = offset / sizeof (int);
    field = heap;

    stride = cs_elems / Cache->associativity[0];
    act = 0;
    for (i = 0; i < Cache->associativity[0] - 1; i++) {
        next = act + stride;
        field[act] = next;
        act = next;
    }
    field[act] = act + stride + off_elems;
    act += stride + off_elems;
    for (i = 0; i < Cache->associativity[0] - 1; i++) {
        next = act + stride;
        field[act] = next;
        act = next;
    }
    field[act] = 0; /* ensure looping! */

    MESS (("  trying cache line size %3d B .... ", offset));

    return (field);
}

int *
GenVectWriteBack (int kind, cache_ptr Cache)
{
    int cs_elems, i, stride, offset;
    int *field;

    cs_elems = Cache->cachesize[0] * 2 * 1024 / sizeof (int);
    field = heap;

    if (kind == 1) {
        offset = 0;
        MESS (("  trying conflicting write ....    "));

    } else {
        offset = Cache->linesize[0];
        MESS (("  trying non-conflicting write ...."));
    }

    stride = cs_elems / (2 * Cache->associativity[0]);
    for (i = 0; i < cs_elems / 2 - stride; i += stride) {
        field[i] = i + 2;          /* write to i+2 */
        field[i + 1] = i + stride; /* jump to i + stride */
    }
    field[i] = i + stride + offset / sizeof (int);
    field[i + 1] = 0; /* ensure looping! */

    return (field);
}

cache_ptr
GetL1Specs (cache_ptr Cache, int accs)
{
    int l1, l1ass, l1ls, l1pol;

    if (Cache->cachesize[0] == 0) {
        MESS ((" checking L1 cache...\n"));
        l1 = FindStep (csmin, csmax, Cache, accs, GenVectSizeL1, MeasureReadLoop);
        MESS ((" L1 size found: %sB\n", Mem2Str (l1)));
        Cache->cachesize[0] = l1;
    } else {
        MESS ((" L1 size preset to %sB\n", Mem2Str (Cache->cachesize[0])));
    }

    if (Cache->associativity[0] == 0) {
        l1ass = FindStep (1, MAXASSOC, Cache, accs, GenVectAssocL1, MeasureReadLoop);
        MESS ((" L1 assoc found: %d\n", l1ass));
        Cache->associativity[0] = l1ass;
    } else {
        MESS ((" L1 assoc preset to %d\n", Cache->associativity[0]));
    }

    if (Cache->linesize[0] == 0) {
        l1ls = 2
               * FindStep (sizeof (int), MAXLINESIZE, Cache, accs, GenVectLineL1,
                           MeasureReadLoop);
        MESS ((" L1 line size found: %d B\n", l1ls));
        Cache->linesize[0] = l1ls;
    } else {
        MESS ((" L1 linesize preset to %d\n", Cache->linesize[0]));
    }

    l1pol = FindStep (1, 2, Cache, accs, GenVectWriteBack, MeasureWriteLoop);
    MESS ((" L1 write back policy found: %s\n\n",
           (l1pol == 1 ? "f (fetch on write)" : "a (write around)")));
    Cache->policy[0] = (l1pol == 1 ? "f" : "a");

    Cache->exists[0] = 1;

    return (Cache);
}
