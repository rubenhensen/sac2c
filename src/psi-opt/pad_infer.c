/*
 * $Log$
 * Revision 1.5  2000/08/03 15:33:42  mab
 * completed implementation of inference algorithm
 * (conversion functions not yet supported)
 *
 * Revision 1.4  2000/07/21 14:42:18  mab
 * completed APinfer, added SortAccesses
 *
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

/* @@@
 - Aufruf mit Wurzelknoten
 - nach RemoveUnsupportedShapes
 - Konvertier-Funktionen (pad, unpad) fuer alle verbliebenen Shapes erzeugen
*/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "globals.h"
#include "DupTree.h"
#include "Error.h"
#include "resource.h"

#include "pad_infer.h"
#include "pad_info.h"
#include "pad.h"

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

typedef struct c_u_t {
    shpseg *access;     /* offset vector                                */
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

#define TYP_IFsize(size) size
static int ctype_size[] = {
#include "type_info.mac"
};

/******************************************************************************
 *
 * function:
 *   static void ComputeCache(cache_t *cache, int size, int line_size, int assoc, int
 *el_size)
 *
 * description
 *
 *   This function transforms an external cache specification into the internal
 *   format used throughout this compiler module.
 *
 ******************************************************************************/

static void
ComputeCache (cache_t *cache, int size, int line_size, int assoc, int el_size)
{
    unsigned int tmp, cnt;

    DBUG_ENTER ("ComputeCache");

    cache->assoc = assoc;
    cache->size = size / el_size;
    cache->line_size = line_size / el_size;
    cache->set_num = (size / line_size) / assoc;

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

    DBUG_VOID_RETURN;
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

    DBUG_ENTER ("PrintCache");

    APprintDiag (" assoc       :  %d\n", cache->assoc);
    APprintDiag (" size        :  %d\n", cache->size);
    APprintDiag (" line_size   :  %d\n", cache->line_size);
    APprintDiag (" set_num     :  %d\n", cache->set_num);
    APprintDiag (" mask        :  %u\n", cache->mask);
    APprintDiag (" inc_shift   :  %u\n", cache->inc_shift);
    APprintDiag (" line_shift  :  %u\n\n", cache->line_shift);

    DBUG_VOID_RETURN;
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
SetVect (int dim, shpseg *pv, int val)
{
    int i;

    DBUG_ENTER ("SetVect");

    DBUG_ASSERT ((dim <= SHP_SEG_SIZE), " dimension out of range in SetVect()!");

    for (i = 0; i < dim; i++) {
        SHPSEG_SHAPE (pv, i) = val;
    }

    DBUG_VOID_RETURN;
}

static void
CopyVect (int dim, shpseg *new, shpseg *old)
{
    int i;

    DBUG_ENTER ("CopyVect");

    DBUG_ASSERT ((dim <= SHP_SEG_SIZE), " dimension out of range in CopyVect()!");

    for (i = 0; i < dim; i++) {
        SHPSEG_SHAPE (new, i) = SHPSEG_SHAPE (old, i);
    }

    DBUG_VOID_RETURN;
}

static void
AddVect (int dim, shpseg *res, shpseg *a, shpseg *b)
{
    int i;

    DBUG_ENTER ("AddVect");

    DBUG_ASSERT ((dim <= SHP_SEG_SIZE), " dimension out of range in AddVect()!");

    for (i = 0; i < dim; i++) {
        SHPSEG_SHAPE (res, i) = SHPSEG_SHAPE (a, i) + SHPSEG_SHAPE (b, i);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   static void PrintCacheUtil(int dim, int rows, cache_util_t* cache_util)
 *
 * description
 *
 *   This function prints the cache utilization table to stdout.
 *
 *
 ******************************************************************************/

static void
PrintCacheUtil (int dim, int rows, cache_util_t *cache_util)
{

    int i;

    DBUG_ENTER ("PrintCacheUtil");

    APprintDiag ("cache "
                 "utilisation\n(access,offs,shoffs,set|srconfl,srmindim,srmaxdim|trflag,"
                 "trconfl,trmindim,trmaxdim)\n");

    for (i = 0; i < rows; i++) {

        PIprintShpSeg (dim, cache_util[i].access);
        APprintDiag ("  %10d  %10d  %5d  |  %2d  %2d  %2d  |  %2d  %2d  %2d  %2d\n",
                     cache_util[i].offset, cache_util[i].shifted_offset,
                     cache_util[i].set, cache_util[i].sr_conflicts,
                     cache_util[i].sr_minpaddim, cache_util[i].sr_maxpaddim,
                     cache_util[i].tr_potflag, cache_util[i].tr_conflicts,
                     cache_util[i].tr_minpaddim, cache_util[i].tr_maxpaddim);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   static int InitCacheUtil( cache_util_t** cache_util, pattern_t* pattern,
 *array_type_t* array)
 *
 * description
 *
 *   This function generates and initializes the cache utilization table.
 *   It returns the number of rows and the table itself.
 *
 *
 ******************************************************************************/

static int
InitCacheUtil (cache_util_t **cache_util, pattern_t *pattern, array_type_t *array)
{
    int rows;
    int i;
    pattern_t *pt_ptr;

    DBUG_ENTER ("InitCacheUtil");

    APprintDiag ("initialize cache utilisation (read access patterns):\n");

    rows = 0;
    pt_ptr = pattern;
    while (pt_ptr != NULL) {
        rows++;
        pt_ptr = PIgetNextPattern (pt_ptr);
    }
    (*cache_util) = (cache_util_t *)MALLOC (rows * sizeof (cache_util_t));

    pt_ptr = pattern;
    for (i = 0; i < rows; i++) {
        PIprintPatternElement (array, pt_ptr);
        (*cache_util)[i].access = PIgetPatternShape (pt_ptr);
        (*cache_util)[i].offset = 0;
        (*cache_util)[i].shifted_offset = 0;
        (*cache_util)[i].set = 0;
        (*cache_util)[i].sr_conflicts = 0;
        (*cache_util)[i].sr_minpaddim = 0;
        (*cache_util)[i].sr_maxpaddim = 0;
        (*cache_util)[i].tr_potflag = 0;
        (*cache_util)[i].tr_conflicts = 0;
        (*cache_util)[i].tr_minpaddim = 0;
        (*cache_util)[i].tr_maxpaddim = 0;
        pt_ptr = PIgetNextPattern (pt_ptr);
    }

    DBUG_RETURN (rows);
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

    DBUG_ENTER ("IsSpatialReuseConflict");

    offset_diff = abs (cache_util[a].shifted_offset - cache_util[b].shifted_offset);

    if (offset_diff >= cache->set_num * cache->line_size) {
        set_diff = abs (cache_util[a].set - cache_util[b].set);
        if ((set_diff < 2) || (set_diff > cache->set_num - 2)) {
            is_conflict = 1;
        }
    }

    DBUG_RETURN (is_conflict);
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

    DBUG_ENTER ("IsPotentialTemopralReuseConflict");

    if (cache_util[a + 1].shifted_offset - cache_util[a].shifted_offset
        < (cache->set_num - 2) * cache->line_size) {
        is_reuse = 1;
    }

    DBUG_RETURN (is_reuse);
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

    DBUG_ENTER ("IsTemporalReuseConflict");

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

    DBUG_RETURN (is_conflict);
}

/******************************************************************************
 *
 * function:
 *   static cache_util_t *ComputeAccessData(int rows, cache_util_t *cache_util,
 *                                    cache_t *cache, shpseg* shape)
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
ComputeAccessData (int rows, cache_util_t *cache_util, cache_t *cache, int dim,
                   shpseg *shape)
{

    int a;

    DBUG_ENTER ("ComputeAccessData");

    for (a = 0; a < rows; a++) {

        cache_util[a].offset = PIlinearizeVector (dim, shape, cache_util[a].access);

        cache_util[a].shifted_offset = cache_util[a].offset - cache_util[0].offset;

        cache_util[a].set
          = (int)((((unsigned int)cache_util[a].shifted_offset) >> cache->line_shift)
                  & cache->mask);
    }

    DBUG_RETURN (cache_util);
}

/******************************************************************************
 *
 * function:
 *   static cache_util_t *ComputeSpatialReuse(int rows, cache_util_t *cache_util, cache_t
 **cache)
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
ComputeSpatialReuse (int rows, cache_util_t *cache_util, cache_t *cache, int dim)
{

    int a, i, d, conflicts, minpaddim, maxpaddim;

    DBUG_ENTER ("ComputeSpatialReuse");

    for (a = 0; a < rows; a++) {
        conflicts = 0;
        minpaddim = dim;
        maxpaddim = 0;

        for (i = 0; i < rows; i++) {
            if (IsSpatialReuseConflict (cache_util, cache, a, i)) {
                conflicts++;
                for (d = 0; d < minpaddim; d++) {
                    if (SHPSEG_SHAPE (cache_util[a].access, d)
                        != SHPSEG_SHAPE (cache_util[i].access, d)) {
                        minpaddim = d;
                        break;
                    }
                }
                for (d = dim - 2; d > maxpaddim; d--) {
                    if (SHPSEG_SHAPE (cache_util[a].access, d)
                        != SHPSEG_SHAPE (cache_util[i].access, d)) {
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

    DBUG_RETURN (cache_util);
}

/******************************************************************************
 *
 * function:
 *   static int ComputeTemporalMinpaddim(int rows, cache_util_t *cache_util,
 *                                int a, int i, int current_minpaddim, int dim)
 *
 * description
 *
 *
 *
 *
 ******************************************************************************/

static int
ComputeTemporalMinpaddim (int rows, cache_util_t *cache_util, int a, int i,
                          int current_minpaddim, int dim)
{

    int d, min1, min2, res, h;

    DBUG_ENTER ("ComputeTemporalMinpaddim");

    min1 = dim;
    min2 = dim;

    for (d = 0; d < dim; d++) {
        if (SHPSEG_SHAPE (cache_util[a].access, d)
            != SHPSEG_SHAPE (cache_util[i].access, d)) {
            min1 = d + 1;
            break;
        }
    }

    for (d = 0; d < dim; d++) {
        if (SHPSEG_SHAPE (cache_util[i].access, d)
            != SHPSEG_SHAPE (cache_util[a + 1].access, d)) {
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

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   static int ComputeTemporalMaxpaddim(cache_util_t *cache_util, int a, int dim)
 *
 * description
 *
 *   This function computes the maximal padding dimension to achieve temporal
 *   reuse between access a and a+1. This is the outermost dimension where
 *   the offset vectors for a and a+1 are different.
 *
 ******************************************************************************/

static int
ComputeTemporalMaxpaddim (cache_util_t *cache_util, int a, int dim)
{

    int d;

    DBUG_ENTER ("ComputeTemporalMaxpaddim");

    for (d = 0; d < dim; d++) {
        if (SHPSEG_SHAPE (cache_util[a].access, d)
            != SHPSEG_SHAPE (cache_util[a + 1].access, d)) {
            break;
        }
    }

    if ((d == 0) && (dim > 1)) {
        d = 1;
    }

    DBUG_RETURN (d);
}

/******************************************************************************
 *
 * function:
 *   static cache_util_t *ComputeTemporalReuse(int rows, cache_util_t *cache_util, cache_t
 **cache, int dim)
 *
 * description
 *
 *
 *
 *
 ******************************************************************************/

static cache_util_t *
ComputeTemporalReuse (int rows, cache_util_t *cache_util, cache_t *cache, int dim)
{

    int a, i, conflicts, minpaddim;

    DBUG_ENTER ("ComputeTemporalReuse");

    for (a = 0; a < rows - 1; a++) {

        if (IsPotentialTemporalReuse (cache_util, cache, a)) {
            conflicts = 0;
            minpaddim = dim;
            cache_util[a].tr_potflag = 1;
            cache_util[a].tr_maxpaddim = ComputeTemporalMaxpaddim (cache_util, a, dim);

            for (i = 0; i < rows; i++) {
                if (IsTemporalReuseConflict (cache_util, cache, a, i)) {
                    conflicts++;
                    minpaddim
                      = ComputeTemporalMinpaddim (rows, cache_util, a, i, minpaddim, dim);
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

    DBUG_RETURN (cache_util);
}

/******************************************************************************
 *
 * function:
 *   static int SelectPaddim(int min, int max, shpseg* shape)
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
SelectPaddim (int min, int max, shpseg *shape)
{

    int d, res;

    DBUG_ENTER ("SelectPaddim");

    res = min;

    for (d = min + 1; d <= max; d++) {
        if (SHPSEG_SHAPE (shape, d) > SHPSEG_SHAPE (shape, res)) {
            res = d;
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   static int ChoosePaddimForTemporalReuse(cache_util_t *cache_util,
 *                                    cache_t *cache, shpseg* shape)
 *
 * description
 *
 *   This function determines the padding dimension for the elimination of
 *   temporal reuse conflicts. If padding is not useful -1 is returned.
 *
 ******************************************************************************/

static int
ChoosePaddimForTemporalReuse (int rows, cache_util_t *cache_util, cache_t *cache,
                              shpseg *shape)
{

    int res, a, minpaddim, maxpaddim;

    DBUG_ENTER ("ChoosePaddimForTemporalReuse");

    minpaddim = -1;
    maxpaddim = -1;

    for (a = 0; a < rows - 1; a++) {
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

    res = SelectPaddim (minpaddim, maxpaddim, shape);

    if (res == 0)
        res = 1;

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   static int ChoosePaddimForSpatialReuse(int rows, cache_util_t *cache_util,
 *                                   cache_t *cache, shpseg* shape)
 *
 * description
 *
 *   This function determines the padding dimension for the elimination of
 *   spatial reuse conflicts. If padding is not useful -1 is returned.
 *
 ******************************************************************************/

static int
ChoosePaddimForSpatialReuse (int rows, cache_util_t *cache_util, cache_t *cache,
                             shpseg *shape)
{

    int res, a, minpaddim, maxpaddim;

    DBUG_ENTER ("ChoosePaddimForSpatialReuse");

    minpaddim = -1;
    maxpaddim = -1;

    for (a = 0; a < rows - 1; a++) {
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

    res = SelectPaddim (minpaddim, maxpaddim, shape);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   static void ComputePadding( int size, int line_size, int assoc, int dim,
 *                               shpseg* pv_sr, shpseg* pv_tr, shpseg* shape,
 *                               int el_size, pattern_t* pattern, array_type_t* array)
 *
 * description
 *
 *
 *
 *
 ******************************************************************************/

static void
ComputePadding (int size, int line_size, int assoc, int dim, shpseg *pv_sr, shpseg *pv_tr,
                shpseg *shape, int el_size, pattern_t *pattern, array_type_t *array)
{

    cache_t cache;
    cache_util_t *cache_util;
    shpseg *actual_shape;
    int paddim;
    int pv_sr_unset = 1;
    shpseg *pv;
    int rows;

    DBUG_ENTER ("ComputePadding");

    /*
     * First of all, the internal cache representation is computed.
     */
    ComputeCache (&cache, size, line_size, assoc, el_size);

    PrintCache (&cache);

    rows = InitCacheUtil (&cache_util, pattern, array);

    pv = pv_sr;

    actual_shape = MakeShpseg (NULL);

    do {
        AddVect (dim, actual_shape, shape, pv);

        cache_util = ComputeAccessData (rows, cache_util, &cache, dim, actual_shape);
        cache_util = ComputeSpatialReuse (rows, cache_util, &cache, dim);
        cache_util = ComputeTemporalReuse (rows, cache_util, &cache, dim);

        APprintDiag ("\nCurrent shape :");
        PIprintShpSeg (dim, actual_shape);
        APprintDiag ("\n\n");

        PrintCacheUtil (dim, rows, cache_util);
        APprintDiag ("\n\n");

        paddim = ChoosePaddimForSpatialReuse (rows, cache_util, &cache, shape);

        if (paddim != -1) {
            /*  padding for spatial reuse required */
            SHPSEG_SHAPE (pv, paddim) += 1;
        } else {
            if (pv_sr_unset) {
                /*
                 * Maybe we are successful in eliminating all spatial reuse
                 * conflicts, but later on we fail to achieve the same for
                 * temporal reuse conflicts. In this case we may want to
                 * recall the first semi successfull padding vector.
                 */
                pv = pv_tr;
                CopyVect (dim, pv_tr, pv_sr);
                pv_sr_unset = 0;
            }

            paddim = ChoosePaddimForTemporalReuse (rows, cache_util, &cache, shape);

            if (paddim != -1) {
                /*  padding for temporal reuse required */
                SHPSEG_SHAPE (pv, paddim) += 1;
            }
        }

    } while (paddim != -1);

    FreeShpseg (actual_shape);
    FREE (cache_util);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   static int ComputePaddingOverhead(int dim, shpseg* orig_shape, shpseg* padding)
 *
 * description
 *
 *   This function computes the overhead in memory consumption
 *   caused by a specific padding as percentage of the original
 *   array size.
 *
 ******************************************************************************/

static int
ComputePaddingOverhead (int dim, shpseg *orig_shape, shpseg *padding)
{

    int i, orig_size, padding_size, overhead;

    DBUG_ENTER ("ComputePaddingOverhead");

    orig_size = 1;
    padding_size = 1;

    for (i = 0; i < dim; i++) {
        orig_size *= SHPSEG_SHAPE (orig_shape, i);
        padding_size *= SHPSEG_SHAPE (orig_shape, i) + SHPSEG_SHAPE (padding, i);
    }

    overhead = ((padding_size - orig_size) * 100) / orig_size;

    if (overhead * orig_size < (padding_size - orig_size) * 100) {
        overhead++;
    }

    DBUG_RETURN (overhead);
}

/******************************************************************************
 *
 * function:
 *   static shpseg *SelectRecommendedPadding(shpseg* shape, shpseg* nopad,
 *                       shpseg* pv_sr_L1, shpseg* pv_sr_L2, shpseg* pv_sr_L3,
 *                       shpseg* pv_tr_L1, shpseg* pv_tr_L2, shpseg* pv_tr_L3,
 *                       int threshold)
 *
 * description
 *
 *   This function selects that padding that is mostly recommended
 *   with respect to a given upper limit on additional memory consumption
 *
 ******************************************************************************/

static shpseg *
SelectRecommendedPadding (int dim, shpseg *shape, shpseg *nopad, shpseg *pv_sr_L1,
                          shpseg *pv_sr_L2, shpseg *pv_sr_L3, shpseg *pv_tr_L1,
                          shpseg *pv_tr_L2, shpseg *pv_tr_L3, int threshold)
{

    shpseg *res;

    DBUG_ENTER ("SelectRecommendedPadding");

    res = nopad;

    if ((ComputePaddingOverhead (dim, shape, pv_sr_L1) <= threshold)
        && (PIlinearizeVector (dim, shape, pv_sr_L1)
            >= PIlinearizeVector (dim, shape, res))) {
        res = pv_sr_L1;
    }
    if ((ComputePaddingOverhead (dim, shape, pv_tr_L1) <= threshold)
        && (PIlinearizeVector (dim, shape, pv_tr_L1)
            >= PIlinearizeVector (dim, shape, res))) {
        res = pv_tr_L1;
    }
    if ((ComputePaddingOverhead (dim, shape, pv_sr_L2) <= threshold)
        && (PIlinearizeVector (dim, shape, pv_sr_L2)
            >= PIlinearizeVector (dim, shape, res))) {
        res = pv_sr_L2;
    }
    if ((ComputePaddingOverhead (dim, shape, pv_tr_L2) <= threshold)
        && (PIlinearizeVector (dim, shape, pv_tr_L2)
            >= PIlinearizeVector (dim, shape, res))) {
        res = pv_tr_L2;
    }
    if ((ComputePaddingOverhead (dim, shape, pv_sr_L3) <= threshold)
        && (PIlinearizeVector (dim, shape, pv_sr_L3)
            >= PIlinearizeVector (dim, shape, res))) {
        res = pv_sr_L3;
    }
    if ((ComputePaddingOverhead (dim, shape, pv_tr_L3) <= threshold)
        && (PIlinearizeVector (dim, shape, pv_tr_L3)
            >= PIlinearizeVector (dim, shape, res))) {
        res = pv_tr_L3;
    }

    DBUG_RETURN (res);
}

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

    shpseg *pv_sr_L1;
    shpseg *pv_sr_L2;
    shpseg *pv_sr_L3;

    shpseg *pv_tr_L1;
    shpseg *pv_tr_L2;
    shpseg *pv_tr_L3;

    shpseg *nopad;

    int overhead_threshold = 20;
    shpseg *recommended_pv;

    simpletype type;
    int dim;
    shpseg *shape;
    int element_size;

    array_type_t *at_ptr;
    conflict_group_t *cg_ptr;
    pattern_t *pt_ptr;

    shpseg *new_shape;

    DBUG_ENTER ("APinfer");

    DBUG_PRINT ("API", ("Array Padding: infering new shapes..."));

    /* clean up collected information */

    PItidyAccessPattern ();

    PIprintAccessPatterns ();

    /* init additional data structures */

    pv_sr_L1 = MakeShpseg (NULL);
    pv_sr_L2 = MakeShpseg (NULL);
    pv_sr_L3 = MakeShpseg (NULL);

    pv_tr_L1 = MakeShpseg (NULL);
    pv_tr_L2 = MakeShpseg (NULL);
    pv_tr_L3 = MakeShpseg (NULL);

    nopad = MakeShpseg (NULL);

    /* for every array type... */
    at_ptr = PIgetFirstArrayType ();
    while (at_ptr != NULL) {

        APprintDiag ("\n\nInfering new shape for array type:\n");
        PIprintArrayTypeElement (at_ptr);

        dim = PIgetArrayTypeDim (at_ptr);
        shape = DupShpSeg (PIgetArrayTypeShape (at_ptr));
        type = PIgetArrayTypeBasetype (at_ptr);
        element_size = ctype_size[type];

        SetVect (dim, pv_sr_L1, 0);
        SetVect (dim, pv_sr_L2, 0);
        SetVect (dim, pv_sr_L3, 0);
        SetVect (dim, pv_tr_L1, 0);
        SetVect (dim, pv_tr_L2, 0);
        SetVect (dim, pv_tr_L3, 0);
        SetVect (dim, nopad, 0);

        /* get access patterns for each conflict group */
        cg_ptr = PIgetFirstConflictGroup (at_ptr);
        while (cg_ptr != NULL) {

            APprintDiag (
              "\n\nInfering new shape for single conflict group of array type:\n");
            PIprintConflictGroupElement (at_ptr, cg_ptr);

            /* for every access pattern infer new shape */
            pt_ptr = PIgetFirstPattern (cg_ptr);

            /* check, if first level cache is specified */
            if (config.cache1_size > 0) {

                APprintDiag ("using 1st level cache:\n");

                /* first level cache */
                ComputePadding (config.cache1_size * 1024, config.cache1_line,
                                config.cache1_assoc, dim, pv_sr_L1, pv_tr_L1, shape,
                                element_size, pt_ptr, at_ptr);

                /* check, if second level cache is specified */
                if (config.cache2_size > 0) {

                    APprintDiag ("using 2nd level cache:\n");

                    CopyVect (dim, pv_sr_L2, pv_tr_L1);

                    /* second level cache */
                    ComputePadding (config.cache2_size * 1024, config.cache2_line,
                                    config.cache2_assoc, dim, pv_sr_L2, pv_tr_L2, shape,
                                    element_size, pt_ptr, at_ptr);

                    /* check, if third level cache is specified */
                    if (config.cache3_size > 0) {

                        APprintDiag ("using 3rd level cache:\n");

                        CopyVect (dim, pv_sr_L3, pv_tr_L2);

                        /* third level cache */
                        ComputePadding (config.cache3_size * 1024, config.cache3_line,
                                        config.cache3_assoc, dim, pv_sr_L3, pv_tr_L3,
                                        shape, element_size, pt_ptr, at_ptr);
                    }
                }
            }

            /* recommended padding for this conflict group */
            recommended_pv
              = SelectRecommendedPadding (dim, shape, nopad, pv_sr_L1, pv_sr_L2, pv_sr_L3,
                                          pv_tr_L1, pv_tr_L2, pv_tr_L3,
                                          overhead_threshold);

            APprintDiag ("Original shape                       :  ");
            PIprintShpSeg (dim, shape);
            APprintDiag ("\n");

            if (config.cache1_size > 0) {

                APprintDiag ("L1 Padding vector for spatial reuse  :  ");
                PIprintShpSeg (dim, pv_sr_L1);
                APprintDiag ("    Overhead: <= %4d%%\n",
                             ComputePaddingOverhead (dim, shape, pv_sr_L1));

                APprintDiag ("L1 Padding vector for temporal reuse :  ");
                PIprintShpSeg (dim, pv_tr_L1);
                APprintDiag ("    Overhead: <= %4d%%\n",
                             ComputePaddingOverhead (dim, shape, pv_tr_L1));
                if (config.cache2_size > 0) {

                    APprintDiag ("L2 Padding vector for spatial reuse  :  ");
                    PIprintShpSeg (dim, pv_sr_L2);
                    APprintDiag ("    Overhead: <= %4d%%\n",
                                 ComputePaddingOverhead (dim, shape, pv_sr_L2));

                    APprintDiag ("L2 Padding vector for temporal reuse :  ");
                    PIprintShpSeg (dim, pv_tr_L2);
                    APprintDiag ("    Overhead: <= %4d%%\n",
                                 ComputePaddingOverhead (dim, shape, pv_tr_L2));

                    if (config.cache3_size > 0) {

                        APprintDiag ("L3 Padding vector for spatial reuse  :  ");
                        PIprintShpSeg (dim, pv_sr_L2);
                        APprintDiag ("    Overhead: <= %4d%%\n",
                                     ComputePaddingOverhead (dim, shape, pv_sr_L3));

                        APprintDiag ("L3 Padding vector for temporal reuse :  ");
                        PIprintShpSeg (dim, pv_tr_L2);
                        APprintDiag ("    Overhead: <= %4d%%\n",
                                     ComputePaddingOverhead (dim, shape, pv_tr_L3));
                    }
                }
            }

            APprintDiag ("Recommended padding vector           :  ");
            PIprintShpSeg (dim, recommended_pv);
            APprintDiag ("    Overhead: <= %4d%%\n\n",
                         ComputePaddingOverhead (dim, shape, recommended_pv));

            /* start with recommended padding vector for next conflict group for same
             * array type */
            CopyVect (dim, pv_sr_L1, recommended_pv);
            recommended_pv = pv_sr_L1;
            SetVect (dim, pv_sr_L2, 0);
            SetVect (dim, pv_sr_L3, 0);
            SetVect (dim, pv_tr_L1, 0);
            SetVect (dim, pv_tr_L2, 0);
            SetVect (dim, pv_tr_L3, 0);

            cg_ptr = PIgetNextConflictGroup (cg_ptr);
        }

        /* if shape needs padding, add to pad_info */
        new_shape = MakeShpseg (NULL);
        AddVect (dim, new_shape, shape, recommended_pv);
        if (!EqualShpseg (dim, shape, new_shape)) {

            PIaddInferredShape (type, dim, shape, new_shape);

        } else {

            FreeShpseg (shape);
            FreeShpseg (new_shape);
        }

        /* infer padding for next array type */

        at_ptr = PIgetNextArrayType (at_ptr);
    }

    FreeShpseg (pv_sr_L1);
    FreeShpseg (pv_sr_L2);
    FreeShpseg (pv_sr_L3);

    FreeShpseg (pv_tr_L1);
    FreeShpseg (pv_tr_L2);
    FreeShpseg (pv_tr_L3);

    FreeShpseg (nopad);

    PIprintPadInfo ();

    PIremoveUnsupportedShapes ();

    PIprintPadInfo ();

    DBUG_VOID_RETURN;
}
