/*
 * $Log$
 * Revision 1.3  2000/07/13 15:25:05  mab
 * added static declaration
 * added APinfer
 *
 * Revision 1.2  2000/07/13 12:08:58  cg
 * Initial version of inference algorithm identical to the standalone
 * implementation developed for PDPTA'2000.
 *
 * Revision 1.1  2000/05/26 13:42:19  sbs
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file: pad_infer.c
 *
 * prefix: API
 *
 * description:
 *
 *   This compiler module infers new shapes for array padding.
 *
 *
 *
 *
 *****************************************************************************/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"

#include "pad_infer.h"
#include "pad_info.h"

#define DIM 3
#define ACCESS 6

/*****************************************************************************
 *
 * function:
 *   void APinfer ()
 *
 * description:
 *   main function for infering new shapes for array padding
 *
 *****************************************************************************/

void
APinfer ()
{

    DBUG_ENTER ("APinfer");

    DBUG_PRINT ("API", ("Array Padding: infering new shapes..."));

    /* to be done */

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * type:  cache_spec_t
 *
 * description
 *
 *   This type is used to represent the external specification of a cache.
 *
 *
 ******************************************************************************/

typedef struct {
    int size;      /* cache size in bytes      */
    int line_size; /* cache line size in bytes */
    int assoc;     /* cache set associativity  */
} cache_spec_t;

/******************************************************************************
 *
 * type:  cache_t
 *
 * description
 *
 *   This type is used as the internal cache specification. In addition to
 *   the external specification some computed figures are stored.
 *
 ******************************************************************************/

typedef struct {
    int assoc;               /* cache set associativity                     */
    int size;                /* cache size in array elements                */
    int line_size;           /* cache line size in array elements           */
    int set_num;             /* number of cache sets                        */
    unsigned int mask;       /* mask used to compute cache set ID           */
    unsigned int inc_shift;  /* shift used to compute cache set incarnation */
    unsigned int line_shift; /* shift used to compute cache set ID          */
} cache_t;

/******************************************************************************
 *
 * type:  cache_util_t
 *
 * description
 *
 *   This type defines one row of the cache utilization table that stores
 *   all intermediate data when computing a padding vector. The cache
 *   utilization table represents the most important data structure in
 *   this compiler module.
 *
 ******************************************************************************/

typedef struct {
    int *access;        /* offset vector                                */
    int offset;         /* offset with respect to array shape           */
    int shifted_offset; /* shifted (non-negative) offset                */
    int set;            /* cache set                                    */
    int sr_conflicts;   /* number of potential spatial reuse conflicts  */
    int sr_minpaddim;   /* minimal padding dimension for spatial reuse  */
    int sr_maxpaddim;   /* maximal padding dimension for spatial reuse  */
    int tr_potflag;     /* flag signaling potential temporal reuse      */
    int tr_conflicts;   /* number of potential temporal reuse conflicts */
    int tr_minpaddim;   /* minimal padding dimension for temporal reuse */
    int tr_maxpaddim;   /* maximal padding dimension for temporal reuse */
} cache_util_t;

/******************************************************************************
 *
 * function:
 *   static void ComputeCache(cache_t *cache, cache_spec_t *cache_spec, int el_size)
 *
 * description
 *
 *   This function transforms an external cache specification into the internal
 *   format used throughout this compiler module.
 *
 ******************************************************************************/

static void
ComputeCache (cache_t *cache, cache_spec_t *cache_spec, int el_size)
{
    unsigned int tmp, cnt;

    cache->assoc = cache_spec->assoc;
    cache->size = cache_spec->size / el_size;
    cache->line_size = cache_spec->line_size / el_size;
    cache->set_num = (cache->size / cache->line_size) / cache->assoc;

    cache->mask = cache->set_num - 1;

    tmp = 1;
    cnt = 0;

    while (tmp < cache->line_size) {
        tmp *= 2;
        cnt++;
    }

    cache->line_shift = cnt;

    tmp = 1;

    while (tmp < cache->set_num) {
        tmp *= 2;
        cnt++;
    }

    cache->inc_shift = cnt;
}

/******************************************************************************
 *
 * function:
 *   static void PrintCache(cache_t *cache)
 *
 * description
 *
 *   This function prints the internal cache representation to stdout.
 *
 *
 ******************************************************************************/

static void
PrintCache (cache_t *cache)
{
    printf (" assoc       :  %d\n", cache->assoc);
    printf (" size        :  %d\n", cache->size);
    printf (" line_size   :  %d\n", cache->line_size);
    printf (" set_num     :  %d\n", cache->set_num);
    printf (" mask        :  %u\n", cache->mask);
    printf (" inc_shift   :  %u\n", cache->inc_shift);
    printf (" line_shift  :  %u\n", cache->line_shift);
}

/******************************************************************************
 *
 * function:
 *
 *
 * description
 *
 *   Some functions for dealing with internal vectors.
 *
 *
 ******************************************************************************/

static void
SetVect (int *pv, int val)
{
    int i;

    for (i = 0; i < DIM; i++) {
        pv[i] = val;
    }
}

static void
CopyVect (int *new, int *old)
{
    int i;

    for (i = 0; i < DIM; i++) {
        new[i] = old[i];
    }
}

static void
AddVect (int *res, int *a, int *b)
{
    int i;

    for (i = 0; i < DIM; i++) {
        res[i] = a[i] + b[i];
    }
}

static void
PrintVect (int *v)
{
    int j;

    printf ("[");
    for (j = 0; j < DIM - 1; j++) {
        printf ("%3d, ", v[j]);
    }
    printf ("%3d]", v[DIM - 1]);
}

/******************************************************************************
 *
 * function:
 *   static void PrintCacheUtil(cache_util_t cache_util[])
 *
 * description
 *
 *   This function prints the cache utilization table to stdout.
 *
 *
 ******************************************************************************/

static void
PrintCacheUtil (cache_util_t cache_util[])
{
    int a;

    for (a = 0; a < ACCESS; a++) {
        PrintVect (cache_util[a].access);
        printf ("  %10d  %10d  %5d  |  %2d  %2d  %2d  |  %2d  %2d  %2d  %2d\n",
                cache_util[a].offset, cache_util[a].shifted_offset, cache_util[a].set,
                cache_util[a].sr_conflicts, cache_util[a].sr_minpaddim,
                cache_util[a].sr_maxpaddim, cache_util[a].tr_potflag,
                cache_util[a].tr_conflicts, cache_util[a].tr_minpaddim,
                cache_util[a].tr_maxpaddim);
    }
}

/******************************************************************************
 *
 * function:
 *   static cache_util_t* InitCacheUtil( cache_util_t *cache_util, int
 *access[ACCESS][DIM])
 *
 * description
 *
 *   This function initializes the cache utilization table.
 *
 *
 ******************************************************************************/

static cache_util_t *
InitCacheUtil (cache_util_t *cache_util, int access[][DIM])
{
    int a;

    for (a = 0; a < ACCESS; a++) {
        cache_util[a].access = access[a];
        cache_util[a].offset = 0;
        cache_util[a].shifted_offset = 0;
        cache_util[a].set = 0;
        cache_util[a].sr_conflicts = 0;
        cache_util[a].sr_minpaddim = 0;
        cache_util[a].tr_potflag = 0;
        cache_util[a].tr_conflicts = 0;
        cache_util[a].tr_minpaddim = 0;
        cache_util[a].tr_maxpaddim = 0;
    }

    return (cache_util);
}

/******************************************************************************
 *
 * function:
 *   static int IsSpatialReuseConflict(cache_util_t *cache_util, cache_t cache,
 *                              int a, int b)
 *
 * description
 *
 *   This function checks whether or not there is a spatial reuse conflict
 *   between two accesses given as indices to the cache utilization table.
 *
 ******************************************************************************/

static int
IsSpatialReuseConflict (cache_util_t *cache_util, cache_t *cache, int a, int b)
{
    int is_conflict = 0;
    int offset_diff, set_diff;

    offset_diff = abs (cache_util[a].shifted_offset - cache_util[b].shifted_offset);

    if (offset_diff >= cache->set_num * cache->line_size) {
        set_diff = abs (cache_util[a].set - cache_util[b].set);
        if ((set_diff < 2) || (set_diff > cache->set_num - 2)) {
            is_conflict = 1;
        }
    }

    return (is_conflict);
}

/******************************************************************************
 *
 * function:
 *   static int IsPotentialTemporalReuse(cache_util_t *cache_util, cache_t cache,
 *                                int a)
 *
 * description
 *
 *   This function checks whether or not there is a potential for temporal
 *   reuse between the given access and its following access.
 *
 ******************************************************************************/

static int
IsPotentialTemporalReuse (cache_util_t *cache_util, cache_t *cache, int a)
{
    int is_reuse = 0;

    if (cache_util[a + 1].shifted_offset - cache_util[a].shifted_offset
        < (cache->set_num - 2) * cache->line_size) {
        is_reuse = 1;
    }

    return (is_reuse);
}

/******************************************************************************
 *
 * function:
 *   static int IsTemporalReuseConflict(cache_util_t *cache_util, cache_t cache,
 *                               int a, int b)
 *
 * description
 *
 *   This function checks whether or not access b destroys the potential
 *   temporal reuse between access a and its following neighbour access.
 *
 *
 ******************************************************************************/

static int
IsTemporalReuseConflict (cache_util_t *cache_util, cache_t *cache, int a, int b)
{
    int is_conflict = 0;
    int offset_diff, set_diff;

    if (cache_util[a].set <= cache_util[a + 1].set) {
        if ((cache_util[b].set > cache_util[a].set)
            && (cache_util[b].set < cache_util[a + 1].set)) {
            is_conflict = 1;
        }
    } else {
        if ((cache_util[b].set > cache_util[a].set)
            || (cache_util[b].set < cache_util[a + 1].set)) {
            is_conflict = 1;
        }
    }

    return (is_conflict);
}

/******************************************************************************
 *
 * function:
 *   static cache_util_t *ComputeAccessData( cache_util_t *cache_util,
 *                                    cache_t *cache, int *shp)
 *
 * description
 *
 *   This function computes the general part of the cache utilization
 *   table.
 *
 *   For each access, the offset vector is linearized with respect to a
 *   given shape vector using the Horner scheme.
 *
 *   Offsets often are negative. Since this makes subsequent computations
 *   more complicated, all offsets are shifted so that the smallest offset
 *   always is 0. This shifting, in contrast, does not cause any trouble
 *   because the whole padding inference scheme only relies on the relative
 *   positioning of accesses in the cache.
 *
 *   The shifted offset is used to compute the cache set of each access.
 *
 *   The idea of the cache incarnation is to virtually arrange an unlimited
 *   number of identical caches one after the other. The incarnation then
 *   represents the number of the cache that would be used to satisfy an
 *   access.
 *
 ******************************************************************************/

static cache_util_t *
ComputeAccessData (cache_util_t *cache_util, cache_t *cache, int *shp)
{
    int offset, i, a;

    for (a = 0; a < ACCESS; a++) {

        offset = cache_util[a].access[0];
        for (i = 1; i < DIM; i++) {
            offset *= shp[i];
            offset += cache_util[a].access[i];
        }

        cache_util[a].offset = offset;

        cache_util[a].shifted_offset = cache_util[a].offset - cache_util[0].offset;

        cache_util[a].set
          = (int)((((unsigned int)cache_util[a].shifted_offset) >> cache->line_shift)
                  & cache->mask);
    }

    return (cache_util);
}

/******************************************************************************
 *
 * function:
 *   static cache_util_t *ComputeSpatialReuse( cache_util_t *cache_util, cache_t *cache)
 *
 * description
 *
 *   This function computes the coloumns of the cache utilization table that
 *   are related to spatial reuse. For each access the number of potential
 *   spatial reuse conflicts is computed along with the minimum and maximum
 *   dimension where padding may solve the problem.
 *
 ******************************************************************************/

static cache_util_t *
ComputeSpatialReuse (cache_util_t *cache_util, cache_t *cache)
{
    int a, i, d, conflicts, minpaddim, maxpaddim;

    for (a = 0; a < ACCESS; a++) {
        conflicts = 0;
        minpaddim = DIM;
        maxpaddim = 0;

        for (i = 0; i < ACCESS; i++) {
            if (IsSpatialReuseConflict (cache_util, cache, a, i)) {
                conflicts++;
                for (d = 0; d < minpaddim; d++) {
                    if (cache_util[a].access[d] != cache_util[i].access[d]) {
                        minpaddim = d;
                        break;
                    }
                }
                for (d = DIM - 2; d > maxpaddim; d--) {
                    if (cache_util[a].access[d] != cache_util[i].access[d]) {
                        maxpaddim = d;
                        break;
                    }
                }
            }
        }

        cache_util[a].sr_conflicts = conflicts;

        if (conflicts == 0) {
            cache_util[a].sr_minpaddim = -1;
            cache_util[a].sr_maxpaddim = -1;
        } else {
            cache_util[a].sr_minpaddim = minpaddim + 1;
            cache_util[a].sr_maxpaddim = maxpaddim + 1;
        }
    }

    return (cache_util);
}

/******************************************************************************
 *
 * function:
 *
 *
 * description
 *
 *
 *
 *
 ******************************************************************************/

static int
ComputeTemporalMinpaddim (cache_util_t *cache_util, int a, int i, int current_minpaddim)
{
    int d, min1, min2, res, h;

    min1 = DIM;
    min2 = DIM;

    for (d = 0; d < DIM; d++) {
        if (cache_util[a].access[d] != cache_util[i].access[d]) {
            min1 = d + 1;
            break;
        }
    }

    for (d = 0; d < DIM; d++) {
        if (cache_util[i].access[d] != cache_util[a + 1].access[d]) {
            min2 = d + 1;
            break;
        }
    }

    if (min1 > min2) {
        h = min1;
        min1 = min2;
        min2 = h;
    }

    /*
     * Now min1 is guaranteed to be less or equal to min2.
     */

    if (min2 <= current_minpaddim) {
        if (current_minpaddim <= cache_util[a].tr_maxpaddim) {
            res = current_minpaddim;
        } else {
            if (min2 <= cache_util[a].tr_maxpaddim) {
                res = min2;
            } else {
                res = min1;
            }
        }
    } else {
        res = current_minpaddim;
    }

    return (res);
}

/******************************************************************************
 *
 * function:
 *   static int ComputeTemporalMaxpaddim(cache_util_t *cache_util, int a)
 *
 * description
 *
 *   This function computes the maximal padding dimension to achieve temporal
 *   reuse between access a and a+1. This is the outermost dimension where
 *   the offset vectors for a and a+1 are different.
 *
 ******************************************************************************/

static int
ComputeTemporalMaxpaddim (cache_util_t *cache_util, int a)
{
    int d;

    for (d = 0; d < DIM; d++) {
        if (cache_util[a].access[d] != cache_util[a + 1].access[d]) {
            break;
        }
    }

    if ((d == 0) && (DIM > 1)) {
        d = 1;
    }

    return (d);
}

/******************************************************************************
 *
 * function:
 *
 *
 * description
 *
 *
 *
 *
 ******************************************************************************/

static cache_util_t *
ComputeTemporalReuse (cache_util_t *cache_util, cache_t *cache)
{
    int a, i, conflicts, minpaddim;

    for (a = 0; a < ACCESS - 1; a++) {

        if (IsPotentialTemporalReuse (cache_util, cache, a)) {
            conflicts = 0;
            minpaddim = DIM;
            cache_util[a].tr_potflag = 1;
            cache_util[a].tr_maxpaddim = ComputeTemporalMaxpaddim (cache_util, a);

            for (i = 0; i < ACCESS; i++) {
                if (IsTemporalReuseConflict (cache_util, cache, a, i)) {
                    conflicts++;
                    minpaddim = ComputeTemporalMinpaddim (cache_util, a, i, minpaddim);
                }
            }

            cache_util[a].tr_conflicts = conflicts;

            if (conflicts > 0) {
                cache_util[a].tr_minpaddim = minpaddim;
            } else {
                cache_util[a].tr_minpaddim = -1;
                cache_util[a].tr_maxpaddim = -1;
            }
        } else {
            cache_util[a].tr_potflag = 0;
            cache_util[a].tr_conflicts = -1;
            cache_util[a].tr_minpaddim = -1;
            cache_util[a].tr_maxpaddim = -1;
        }
    }

    /*
     * Finally, the dummy values for the last row of the cache utilization table have
     * to be set.
     */

    cache_util[a].tr_potflag = -1;
    cache_util[a].tr_conflicts = -1;
    cache_util[a].tr_minpaddim = -1;
    cache_util[a].tr_maxpaddim = -1;

    return (cache_util);
}

/******************************************************************************
 *
 * function:
 *   static int SelectPaddim(int min, int max, int *shp)
 *
 * description
 *
 *   This function selects one dimension from a non-empty range of dimensions
 *   found suitable for padding. It selects the outermost dimension with
 *   maximal shape extent. The first criterion moves padding overhead
 *   from inner loops to outer loops; the second criterion leads to a minimal
 *   increase in memory space needed for the representation of the array.
 *
 ******************************************************************************/

static int
SelectPaddim (int min, int max, int *shp)
{
    int d, res;

    res = min;

    for (d = min + 1; d <= max; d++) {
        if (shp[d] > shp[res]) {
            res = d;
        }
    }

    return (res);
}

/******************************************************************************
 *
 * function:
 *   static int ChoosePaddimForTemporalReuse(cache_util_t *cache_util,
 *                                    cache_t *cache, int *shp)
 *
 * description
 *
 *   This function determines the padding dimension for the elimination of
 *   temporal reuse conflicts. If padding is not useful -1 is returned.
 *
 ******************************************************************************/

static int
ChoosePaddimForTemporalReuse (cache_util_t *cache_util, cache_t *cache, int *shp)
{
    int res, a, minpaddim, maxpaddim, d;

    minpaddim = -1;
    maxpaddim = -1;

    for (a = 0; a < ACCESS - 1; a++) {
        if (cache_util[a].tr_potflag && (cache_util[a].tr_conflicts >= cache->assoc)) {
            /* a real conflict occurs */
            if (cache_util[a].tr_minpaddim <= cache_util[a].tr_maxpaddim) {
                /* padding might help */
                if (minpaddim == -1) {
                    /* no padding dimension determined yet */
                    minpaddim = cache_util[a].tr_minpaddim;
                    maxpaddim = cache_util[a].tr_maxpaddim;
                } else {
                    if ((cache_util[a].tr_minpaddim > minpaddim)
                        && (cache_util[a].tr_minpaddim <= maxpaddim)) {
                        minpaddim = cache_util[a].tr_minpaddim;
                    }
                    if ((cache_util[a].tr_maxpaddim < maxpaddim)
                        && (cache_util[a].tr_maxpaddim >= minpaddim)) {
                        maxpaddim = cache_util[a].tr_maxpaddim;
                    }
                }
            }
        }
    }

    res = SelectPaddim (minpaddim, maxpaddim, shp);

    if (res == 0)
        res = 1;

    return (res);
}

/******************************************************************************
 *
 * function:
 *   static int ChoosePaddimForSpatialReuse(cache_util_t *cache_util,
 *                                   cache_t *cache, int *shp)
 *
 * description
 *
 *   This function determines the padding dimension for the elimination of
 *   spatial reuse conflicts. If padding is not useful -1 is returned.
 *
 ******************************************************************************/

static int
ChoosePaddimForSpatialReuse (cache_util_t *cache_util, cache_t *cache, int *shp)
{
    int res, a, minpaddim, maxpaddim, d;

    minpaddim = -1;
    maxpaddim = -1;

    for (a = 0; a < ACCESS - 1; a++) {
        if (cache_util[a].sr_conflicts >= cache->assoc) {
            /* a real conflict occurs */
            if (cache_util[a].sr_minpaddim <= cache_util[a].sr_maxpaddim) {
                /* padding might help */
                if (minpaddim == -1) {
                    /* no padding dimension determined yet */
                    minpaddim = cache_util[a].sr_minpaddim;
                    maxpaddim = cache_util[a].sr_maxpaddim;
                } else {
                    if ((cache_util[a].sr_minpaddim > minpaddim)
                        && (cache_util[a].sr_minpaddim <= maxpaddim)) {
                        minpaddim = cache_util[a].sr_minpaddim;
                    }
                    if ((cache_util[a].sr_maxpaddim < maxpaddim)
                        && (cache_util[a].sr_maxpaddim >= minpaddim)) {
                        maxpaddim = cache_util[a].sr_maxpaddim;
                    }
                }
            }
        }
    }

    res = SelectPaddim (minpaddim, maxpaddim, shp);

    return (res);
}

/******************************************************************************
 *
 * function:
 *
 *
 * description
 *
 *
 *
 *
 ******************************************************************************/

static void
ComputePadding (cache_spec_t *cache_spec, int dim, int *pv_sr, int *pv_tr, int *shp,
                int el_size, int access[ACCESS][DIM])
{
    cache_t cache;
    cache_util_t cache_util_table[ACCESS];
    cache_util_t *cache_util = cache_util_table;
    int actual_shp[DIM];
    int paddim;
    int pv_sr_unset = 1;
    int *pv;

    /*
     * First of all, the internal cache representation is computed.
     */
    ComputeCache (&cache, cache_spec, el_size);

    DBUG_EXECUTE ("API", PrintCache (&cache); printf ("\n\n"););

    cache_util = InitCacheUtil (cache_util, access);

    pv = pv_sr;

    do {
        AddVect (actual_shp, shp, pv);

        cache_util = ComputeAccessData (cache_util, &cache, actual_shp);
        cache_util = ComputeSpatialReuse (cache_util, &cache);
        cache_util = ComputeTemporalReuse (cache_util, &cache);

        DBUG_EXECUTE ("API", printf ("Current shape :"); PrintVect (actual_shp);
                      printf ("\n\n");

                      PrintCacheUtil (cache_util); printf ("\n\n"););

        paddim = ChoosePaddimForSpatialReuse (cache_util, &cache, actual_shp);

        if (paddim != -1) {
            /*  padding for spatial reuse required */
            pv[paddim] += 1;
        } else {
            if (pv_sr_unset) {
                /*
                 * Maybe we are successful in eliminating all spatial reuse
                 * conflicts, but later on we fail to achieve the same for
                 * temporal reuse conflicts. In this case we may want to
                 * recall the first semi successfull padding vector.
                 */
                pv = pv_tr;
                CopyVect (pv_tr, pv_sr);
                pv_sr_unset = 0;
            }

            paddim = ChoosePaddimForTemporalReuse (cache_util, &cache, actual_shp);

            if (paddim != -1) {
                /*  padding for temporal reuse required */
                pv[paddim] += 1;
            }
        }

    } while (paddim != -1);
}

/******************************************************************************
 *
 * function:
 *   static int ComputePaddingOverhead(int *orig_shape, int *padding)
 *
 * description
 *
 *   This function computes the overhead in memory consumption
 *   caused by a specific padding as percentage of the original
 *   array size.
 *
 ******************************************************************************/

static int
ComputePaddingOverhead (int *orig_shape, int *padding)
{
    int i, orig_size, padding_size, overhead;

    orig_size = 1;
    padding_size = 1;

    for (i = 0; i < DIM; i++) {
        orig_size *= orig_shape[i];
        padding_size *= orig_shape[i] + padding[i];
    }

    overhead = ((padding_size - orig_size) * 100) / orig_size;

    if (overhead * orig_size < (padding_size - orig_size) * 100) {
        overhead++;
    }

    return (overhead);
}

/******************************************************************************
 *
 * function:
 *   static int *SelectRecommendedPadding(int *nopad,
 *                                 int *pv_sr_L1, int *pv_sr_L2,
 *                                 int *pv_tr_L1, int *pv_tr_L2,
 *                                 int threshold)
 *
 * description
 *
 *   This function selects that padding that is mostly recommended
 *   with respect to a given upper limit on additional memory consumption
 *
 ******************************************************************************/

static int *
SelectRecommendedPadding (int *shp, int *nopad, int *pv_sr_L1, int *pv_sr_L2,
                          int *pv_tr_L1, int *pv_tr_L2, int threshold)
{
    int *res;

    if (ComputePaddingOverhead (shp, pv_tr_L2) <= threshold) {
        res = pv_tr_L2;
    } else if (ComputePaddingOverhead (shp, pv_sr_L2) <= threshold) {
        res = pv_sr_L2;
    } else if (ComputePaddingOverhead (shp, pv_tr_L1) <= threshold) {
        res = pv_tr_L1;
    } else if (ComputePaddingOverhead (shp, pv_sr_L1) <= threshold) {
        res = pv_sr_L1;
    } else {
        res = nopad;
    }

    return (res);
}

/******************************************************************************
 *
 * function:
 *   static int main()
 *
 * description
 *
 *   Test setting for padding vector computing.
 *
 *
 ******************************************************************************/

#if 0

static int main()
{
  int i, j;
  int pv_sr_L1[DIM], pv_sr_L2[DIM], pv_tr_L1[DIM], pv_tr_L2[DIM];
  int shp[DIM], nopad[DIM];
  int overhead_threshold = 20;
  int *recommended_pv;
  
  int access[ACCESS][DIM] = { { -1,  0,  0},
                              {  0, -1,  0},
                              {  0,  0, -1},
                              {  0,  0,  1},
                              {  0,  1,  0},
                              {  1,  0,  0}};
  
  cache_spec_t cache[] = { 
    { 16*1024, 32, 1},
    {1024*1024, 64, 1
    }
  };
  
  
  for (i=250; i<=262; i+=2 ) {
    for (j=0; j<DIM; j++) {
      shp[j] = i;
    }
    
    SetVect(nopad, 0);
    SetVect(pv_sr_L1, 0);
    SetVect(pv_sr_L2, 0);
    SetVect(pv_tr_L1, 0);
    SetVect(pv_tr_L2, 0);
  
    ComputePadding( &(cache[0]), DIM, pv_sr_L1, pv_tr_L1, shp, 8, access);
  
    CopyVect(pv_sr_L2, pv_tr_L1);
    
    ComputePadding( &(cache[1]), DIM, pv_sr_L2, pv_tr_L2, shp, 8, access);
  
 DBUG_EXECUTE("API",      
     
	      printf("Original shape                       :  ");
	      PrintVect(shp);
	      printf("\n");
	      
	      printf("L1 Padding vector for spatial reuse  :  ");
	      PrintVect(pv_sr_L1);
	      printf("    Overhead: <= %4d%%\n", ComputePaddingOverhead(shp, pv_sr_L1));
	      
	      printf("L1 Padding vector for temporal reuse :  ");
	      PrintVect(pv_tr_L1);
	      printf("    Overhead: <= %4d%%\n", ComputePaddingOverhead(shp, pv_tr_L1));
	      
	      printf("L2 Padding vector for spatial reuse  :  ");
	      PrintVect(pv_sr_L2);
	      printf("    Overhead: <= %4d%%\n", ComputePaddingOverhead(shp, pv_sr_L2));
	      
	      printf("L2 Padding vector for temporal reuse :  ");
	      PrintVect(pv_tr_L2);
	      printf("    Overhead: <= %4d%%\n", ComputePaddingOverhead(shp, pv_tr_L2));
	      
	      printf("Recommended padding vector           :  ");
	      recommended_pv = SelectRecommendedPadding(shp, nopad,
							pv_sr_L1, pv_sr_L2,
							pv_tr_L1, pv_tr_L2,
							overhead_threshold);
	      
	      PrintVect(recommended_pv);
	      printf("    Overhead: <= %4d%%\n", ComputePaddingOverhead(shp, recommended_pv));
	      
	      printf("\n\n============================================================\n\n\n");
	      );

    /*
#else
    printf("#elif N==%d\n", i);
    printf("#define PAD ");
    PrintVect(SelectRecommendedPadding(shp, nopad,
                                       pv_sr_L1, pv_sr_L2,
                                       pv_tr_L1, pv_tr_L2,
                                       overhead_threshold));    
    printf("\n");
#endif
    */    
  }
  
  return(0);
}

#endif
