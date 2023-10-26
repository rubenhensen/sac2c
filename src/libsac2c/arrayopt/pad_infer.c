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
 *****************************************************************************/

/* @@@
 - Aufruf mit Wurzelknoten
 - nach RemoveUnsupportedShapes
 - Konvertier-Funktionen (pad, unpad) fuer alle verbliebenen Shapes erzeugen
*/

#define DBUG_PREFIX "API"
#include "debug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "globals.h"
#include "DupTree.h"
#include "resource.h"
#include "convert.h"
#include "str.h"
#include "memory.h"

#include "pad_infer.h"
#include "pad_info.h"
#include "pad.h"

#define VERY_LARGE_NUMBER 10000000

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
    shape *access;      /* offset vector                                */
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
 *   static
 *   cache_t *CreateCacheSpec(int size, int line_size,
 *                            int assoc, int el_size)
 *
 * description
 *
 *   This function transforms an external cache specification into the internal
 *   format used throughout cache inference. In particular, this format stores
 *   all sizes with respect to array elements of a given basetype.
 *
 ******************************************************************************/

static cache_t *
CreateCacheSpec (int size, int line_size, int assoc, int el_size)
{
    unsigned int tmp, cnt;
    cache_t *cache;

    DBUG_ENTER ();

    if (size == 0) {
        cache = NULL;
    } else {

        cache = (cache_t *)MEMmalloc (sizeof (cache_t));

        cache->assoc = assoc;
        cache->size = size / el_size;
        cache->line_size = line_size / el_size;
        cache->set_num = (size / line_size) / assoc;

        cache->mask = (unsigned int) cache->set_num - 1;

        tmp = 1;
        cnt = 0;

        while (tmp < (unsigned int)(cache->line_size)) {
            tmp *= 2;
            cnt++;
        }

        cache->line_shift = cnt;

        tmp = 1;

        while (tmp < (unsigned int)(cache->set_num)) {
            tmp *= 2;
            cnt++;
        }

        cache->inc_shift = cnt;
    }

    DBUG_RETURN (cache);
}

/******************************************************************************
 *
 * function:
 *   static void PrintCacheSpec(int level, cache_t *cache)
 *
 * description
 *
 *   This function prints the internal cache representation to diagnostic
 *   output.
 *
 ******************************************************************************/

static void
PrintCacheSpec (int level, cache_t *cache)
{

    DBUG_ENTER ();

    APprintDiag ("\n L%d cache:", level);

    if (cache == NULL) {
        APprintDiag (" none\n\n");
    } else {
        APprintDiag ("\n\n");

        APprintDiag ("  cache size       :  %d\n", cache->size);
        APprintDiag ("  cache line size  :  %d\n", cache->line_size);
        APprintDiag ("  associativity    :  %d\n", cache->assoc);
        APprintDiag ("  cache sets       :  %d\n", cache->set_num);
        APprintDiag ("  mask             :  %u\n", cache->mask);
        APprintDiag ("  inc shift        :  %u\n", cache->inc_shift);
        APprintDiag ("  line shift       :  %u\n\n", cache->line_shift);
    }

    DBUG_RETURN ();
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
SetVect (int dim, shape *pv, int val)
{
    int i;

    DBUG_ENTER ();

    DBUG_ASSERT (dim <= SHP_SEG_SIZE, " dimension out of range in SetVect()!");

    for (i = 0; i < dim; i++) {
        SHsetExtent (pv, i, val);
    }

    DBUG_RETURN ();
}

static void
CopyVect (int dim, shape *xnew, shape *old)
{
    int i;

    DBUG_ENTER ();

    DBUG_ASSERT (dim <= SHgetDim (xnew),
                 " dimension out of range in CopyVect()!");

    for (i = 0; i < dim; i++) {
        SHsetExtent (xnew, i, SHgetExtent (old, i));
    }

    DBUG_RETURN ();
}

static void
AddVect (int dim, shape *res, shape *a, shape *b)
{
    int i;

    DBUG_ENTER ();

    DBUG_ASSERT (dim <= SHgetDim (res), " dimension out of range in AddVect()!");

    for (i = 0; i < dim; i++) {
        SHsetExtent (res, i, SHgetExtent (a, i) + SHgetExtent (b, i));
    }

    DBUG_RETURN ();
}

static int
EqualVect (int dim, shape *a, shape *b)
{
    int i;
    int equal = 1;

    DBUG_ENTER ();

    DBUG_ASSERT (dim <= SHgetDim (a), " dimension out of range in EqualVect()!");
    DBUG_ASSERT (dim <= SHgetDim (b), " dimension out of range in EqualVect()!");

    for (i = 0; i < dim; i++) {
        if (SHgetExtent (a, i) != SHgetExtent (b, i)) {
            equal = 0;
            break;
        }
    }

    DBUG_RETURN (equal);
}

/******************************************************************************
 *
 * function:
 *   static void PrintCacheUtil(int dim, unsigned int rows, cache_util_t* cache_util)
 *
 * description
 *
 *   This function prints the cache utilization table to diagnostic output.
 *
 *
 ******************************************************************************/

static void
PrintCacheUtil (int dim, unsigned int rows, cache_util_t *cache_util)
{
    unsigned int i;

    DBUG_ENTER ();

    APprintDiag ("Cache Utilisation Table:\n"
                 "(access,offs,shoffs,set|srconfl,srmindim,srmaxdim"
                 "|trflag,trconfl,trmindim,trmaxdim)\n");

    for (i = 0; i < rows; i++) {

        PIprintShpSeg (dim, cache_util[i].access);
        APprintDiag ("  %10d  %10d  %5d  |  %2d  %2d  %2d  |  %2d  %2d  %2d  %2d\n",
                     cache_util[i].offset, cache_util[i].shifted_offset,
                     cache_util[i].set, cache_util[i].sr_conflicts,
                     cache_util[i].sr_minpaddim, cache_util[i].sr_maxpaddim,
                     cache_util[i].tr_potflag, cache_util[i].tr_conflicts,
                     cache_util[i].tr_minpaddim, cache_util[i].tr_maxpaddim);
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   static int InitCacheUtil( cache_util_t** cache_util,
 *                             pattern_t* pattern,
 *                             array_type_t* array)
 *
 * description
 *
 *   This function generates and initializes the cache utilization table.
 *   It returns the number of rows and the table itself.
 *
 *
 ******************************************************************************/

static unsigned int
InitCacheUtil (cache_util_t **cache_util, pattern_t *pattern, array_type_t *array)
{
    unsigned int rows;
    unsigned int i;
    pattern_t *pt_ptr;

    DBUG_ENTER ();

    APprintDiag ("initialize cache utilisation (read access patterns):\n");

    rows = 0;
    pt_ptr = pattern;
    while (pt_ptr != NULL) {
        rows++;
        pt_ptr = PIgetNextPattern (pt_ptr);
    }
    (*cache_util) = (cache_util_t *)MEMmalloc (rows * sizeof (cache_util_t));

    pt_ptr = pattern;
    for (i = 0; i < rows; i++) {
        PIprintPatternElement (array, pt_ptr);
        /* attention: PIgetPatternShape stores only the pointer to shape in .access
         *            it espacially does not make a copy!
         */
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
 *   static int IsSpatialReuseConflict(cache_util_t *cache_util,
 *                                     cache_t cache,
 *                                     int a, int b)
 *
 * description
 *
 *   This function checks whether or not there is a spatial reuse conflict
 *   between two accesses given as indices to the cache utilization table.
 *
 ******************************************************************************/

static int
IsSpatialReuseConflict (cache_util_t *cache_util, cache_t *cache, unsigned int a, unsigned int b)
{
    int is_conflict = 0;
    int offset_diff, set_diff;

    DBUG_ENTER ();

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
 *   static int IsPotentialTemporalReuse(cache_util_t *cache_util,
 *                                       cache_t cache,
 *                                       unsigned int a)
 *
 * description
 *
 *   This function checks whether or not there is a potential for temporal
 *   reuse between the given access and its following access.
 *
 ******************************************************************************/

static int
IsPotentialTemporalReuse (cache_util_t *cache_util, cache_t *cache, unsigned int a)
{
    int is_reuse = 0;

    DBUG_ENTER ();

    if (cache_util[a + 1].shifted_offset - cache_util[a].shifted_offset
        < (cache->set_num - 2) * cache->line_size) {
        is_reuse = 1;
    }

    DBUG_RETURN (is_reuse);
}

/******************************************************************************
 *
 * function:
 *   static int IsTemporalReuseConflict(cache_util_t *cache_util,
 *                                      cache_t cache,
 *                                      unsigned int a, unsigned int b)
 *
 * description
 *
 *   This function checks whether or not access b destroys the potential
 *   temporal reuse between access a and its following neighbour access.
 *
 *
 ******************************************************************************/

static int
IsTemporalReuseConflict (cache_util_t *cache_util, cache_t *cache, unsigned int a, unsigned int b)
{
    int is_conflict = 0;

    DBUG_ENTER ();

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
 *   static cache_util_t *ComputeAccessData(int rows,
 *                                          cache_util_t *cache_util,
 *                                          cache_t *cache,
 *                                          shape* shp)
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
ComputeAccessData (unsigned int rows, cache_util_t *cache_util, cache_t *cache, int dim,
                   shape *shp)
{
    unsigned int a;

    DBUG_ENTER ();

    for (a = 0; a < rows; a++) {

        cache_util[a].offset = PIlinearizeVector (dim, shp, cache_util[a].access);

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
 *   static cache_util_t *ComputeSpatialReuse(unsigned int rows,
 *                                            cache_util_t *cache_util,
 *                                            cache_t *cache)
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
ComputeSpatialReuse (unsigned int rows, cache_util_t *cache_util, cache_t *cache, int dim)
{
    unsigned int a, i;
    int d, conflicts, minpaddim, maxpaddim;

    DBUG_ENTER ();

    for (a = 0; a < rows; a++) {
        conflicts = 0;
        minpaddim = dim;
        maxpaddim = 0;

        for (i = 0; i < rows; i++) {
            if (IsSpatialReuseConflict (cache_util, cache, a, i)) {
                conflicts++;
                for (d = 0; d < minpaddim; d++) {
                    if (SHgetExtent (cache_util[a].access, d)
                        != SHgetExtent (cache_util[i].access, d)) {
                        minpaddim = d;
                        break;
                    }
                }
                for (d = dim - 2; d > maxpaddim; d--) {
                    if (SHgetExtent (cache_util[a].access, d)
                        != SHgetExtent (cache_util[i].access, d)) {
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
            /*
             * The following solution produces a non-terminating compiler run for pde1
             * and the problem size 512^3.
             *  cache_util[a].sr_minpaddim = minpaddim + 1;
             *  cache_util[a].sr_maxpaddim = maxpaddim + 1;
             *
             * The following represents nothin but a quick and dirty hack to overcome the
             * above problem. Unfortunately, this solution results in completely different
             * padding recommendations as the previous one.
             *  cache_util[a].sr_minpaddim = dim - 1;
             *  cache_util[a].sr_maxpaddim = dim - 1;
             */
            cache_util[a].sr_minpaddim = minpaddim + 1;
            cache_util[a].sr_maxpaddim = dim - 1;
        }
    }

    DBUG_RETURN (cache_util);
}

/******************************************************************************
 *
 * function:
 *   static int ComputeTemporalMinpaddim(unsigned int rows,
 *                                       cache_util_t *cache_util,
 *                                       int a, int i,
 *                                       int current_minpaddim, int dim)
 *
 * description
 *
 *
 *
 *
 ******************************************************************************/

static int
ComputeTemporalMinpaddim (unsigned int rows, cache_util_t *cache_util, unsigned int a, unsigned int i,
                          int current_minpaddim, int dim)
{
    int d, min1, min2, res, h;

    DBUG_ENTER ();

    min1 = dim;
    min2 = dim;

    for (d = 0; d < dim; d++) {
        if (SHgetExtent (cache_util[a].access, d)
            != SHgetExtent (cache_util[i].access, d)) {
            min1 = d + 1;
            break;
        }
    }

    for (d = 0; d < dim; d++) {
        if (SHgetExtent (cache_util[i].access, d)
            != SHgetExtent (cache_util[a + 1].access, d)) {
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
 *   static int ComputeTemporalMaxpaddim(cache_util_t *cache_util,
 *                                       unsigned int a, int dim)
 *
 * description
 *
 *   This function computes the maximal padding dimension to achieve temporal
 *   reuse between access a and a+1. This is the outermost dimension where
 *   the offset vectors for a and a+1 are different.
 *
 ******************************************************************************/

static int
ComputeTemporalMaxpaddim (cache_util_t *cache_util, unsigned int a, int dim)
{

    int d;

    DBUG_ENTER ();

    for (d = 0; d < dim; d++) {
        if (SHgetExtent (cache_util[a].access, d)
            != SHgetExtent (cache_util[a + 1].access, d)) {
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
 *   static cache_util_t *ComputeTemporalReuse(unsigned int rows,
 *                                             cache_util_t *cache_util,
 *                                             cache_t *cache,
 *                                             int dim)
 *
 * description
 *
 *
 *
 *
 ******************************************************************************/

static cache_util_t *
ComputeTemporalReuse (unsigned int rows, cache_util_t *cache_util, cache_t *cache, int dim)
{
    unsigned int a, i;
    int conflicts, minpaddim;

    DBUG_ENTER ();

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

#if 0

/******************************************************************************
 *
 * function:
 *   static int SelectPaddim(int min, int max, shape* shp)
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

static int SelectPaddim(int min, int max, shape* shp)
{
  int d, res;

  DBUG_ENTER ();

  res = min;

  for (d=min+1; d<=max; d++) {
    if (SHgetExtent(shp,d) > SHgetExtent(shp,res)) {
      res = d;
    }
  }

  DBUG_RETURN (res);
}


/******************************************************************************
 *
 * function:
 *   static int ChoosePaddimForTemporalReuse(int rows,
 *                                           cache_util_t *cache_util,
 *                                           cache_t *cache,
 *                                           shape* shp)
 *
 * description
 *
 *   This function determines the padding dimension for the elimination of
 *   temporal reuse conflicts. If padding is not useful -1 is returned.
 *
 ******************************************************************************/

static int ChoosePaddimForTemporalReuse(int rows,
                                        cache_util_t *cache_util,
                                        cache_t *cache,
                                        shape* shp) {

  int res, a, minpaddim, maxpaddim;

  DBUG_ENTER ();

  minpaddim = -1;
  maxpaddim = -1;

  for (a=0; a<rows-1; a++) {
    if (cache_util[a].tr_potflag
        && (cache_util[a].tr_conflicts >= cache->assoc)) {
      /* a real conflict occurs */
      if (cache_util[a].tr_minpaddim <= cache_util[a].tr_maxpaddim) {
        /* padding might help */
        if (minpaddim == -1) {
          /* no padding dimension determined yet */
          minpaddim = cache_util[a].tr_minpaddim;
          maxpaddim = cache_util[a].tr_maxpaddim;
        }
        else {
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

  res = SelectPaddim(minpaddim, maxpaddim, shp);

  if (res==0) res=1;

  DBUG_RETURN (res);
}


/******************************************************************************
 *
 * function:
 *   static int ChoosePaddimForSpatialReuse(int rows,
 *                                          cache_util_t *cache_util,
 *                                          cache_t *cache,
 *                                          shape* shp)
 *
 * description
 *
 *   This function determines the padding dimension for the elimination of
 *   spatial reuse conflicts. If padding is not useful -1 is returned.
 *
 ******************************************************************************/

static int ChoosePaddimForSpatialReuse(int rows,
                                       cache_util_t *cache_util,
                                       cache_t *cache,
                                       shape* shp)
{
  int res, a, minpaddim, maxpaddim;

  DBUG_ENTER ();

  minpaddim = -1;
  maxpaddim = -1;

  for (a=0; a<rows-1; a++) {
    if (cache_util[a].sr_conflicts >= cache->assoc) {
      /* a real conflict occurs */
      if (cache_util[a].sr_minpaddim <= cache_util[a].sr_maxpaddim) {
        /* padding might help */
        if (minpaddim == -1) {
          /* no padding dimension determined yet */
          minpaddim = cache_util[a].sr_minpaddim;
          maxpaddim = cache_util[a].sr_maxpaddim;
        }
        else {
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

  res = SelectPaddim(minpaddim, maxpaddim, shp);

  DBUG_RETURN (res);
}

#endif /*  0  */

/******************************************************************************
 *
 * function:
 *   static int
 *   ComputeNumSpatialReuseConflicts( unsigned int rows, cache_util_t *cache_util)
 *
 * description
 *
 *   This function analyzes the cache utilization table and yields the total
 *   number of spatial reuse conflicts.
 *
 ******************************************************************************/

static int
ComputeNumSpatialReuseConflicts (unsigned int rows, cache_util_t *cache_util)
{
    unsigned int a;
    int res;

    DBUG_ENTER ();

    res = 0;

    for (a = 0; a < rows; a++) {
        res += cache_util[a].sr_conflicts;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   static int
 *   ComputeNumTemporalReuseConflicts( unsigned int rows, cache_util_t *cache_util)
 *
 * description
 *
 *   This function analyzes the cache utilization table and yields the total
 *   number of temporal reuse conflicts.
 *
 ******************************************************************************/

static int
ComputeNumTemporalReuseConflicts (unsigned int rows, cache_util_t *cache_util)
{
    unsigned int a;
    int res;

    DBUG_ENTER ();

    res = 0;

    for (a = 0; a < rows - 1; a++) {
        if (cache_util[a].tr_potflag) {
            /*
             * A spatial reuse conflict only exists where temporal reuse is
             * possible due to cache capacity constraints.
             */
            res += cache_util[a].tr_conflicts;
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   static int
 *   ComputeSpatialReuseMinPadDim( int dim, unsigned int rows, cache_util_t *cache_util);
 *
 * description
 *
 *   This function computes the maximum over all spatial reuse minimum
 *   padding dimension entries in the given cache utilization table.
 *
 ******************************************************************************/

static int
ComputeSpatialReuseMinPadDim (int dim, unsigned int rows, cache_util_t *cache_util)
{
    int min_paddim;
    unsigned int a;

    DBUG_ENTER ();

    min_paddim = -1;

    for (a = 0; a < rows; a++) {
        if (cache_util[a].sr_conflicts > 0) {
            if (cache_util[a].sr_minpaddim > min_paddim) {
                min_paddim = cache_util[a].sr_minpaddim;
            }
        }
    }

    DBUG_RETURN (min_paddim);
}

/******************************************************************************
 *
 * function:
 *   static int
 *   ComputeSpatialReuseMaxPadDim( int dim, unsigned int rows, cache_util_t *cache_util);
 *
 * description
 *
 *   This function computes the minimum over all spatial reuse maximum
 *   padding dimension entries in the given cache utilization table.
 *
 ******************************************************************************/

static int
ComputeSpatialReuseMaxPadDim (int dim, unsigned int rows, cache_util_t *cache_util)
{
    int max_paddim;
    unsigned int a;

    DBUG_ENTER ();

    max_paddim = dim;

    for (a = 0; a < rows; a++) {
        if (cache_util[a].sr_conflicts > 0) {
            if (cache_util[a].sr_maxpaddim < max_paddim) {
                max_paddim = cache_util[a].sr_maxpaddim;
            }
        }
    }

    DBUG_RETURN (max_paddim);
}

/******************************************************************************
 *
 * function:
 *   static int
 *   ComputeTemporalReuseMinPadDim( int dim, unsigned int rows, cache_util_t *cache_util);
 *
 * description
 *
 *   This function computes the maximum over all relevant temporal reuse
 *   minimum padding dimension entries in the given cache utilization table.
 *
 ******************************************************************************/

static int
ComputeTemporalReuseMinPadDim (int dim, unsigned int rows, cache_util_t *cache_util)
{
    int min_paddim;
    unsigned int a;

    DBUG_ENTER ();

    min_paddim = 0;

    for (a = 0; a < rows - 1; a++) {
        if (cache_util[a].tr_potflag) {
            if (cache_util[a].tr_minpaddim > min_paddim) {
                min_paddim = cache_util[a].tr_minpaddim;
            }
        }
    }

    DBUG_RETURN (min_paddim);
}

/******************************************************************************
 *
 * function:
 *   static int
 *   ComputeTemporalReuseMaxPadDim( int dim, unsigned int rows, cache_util_t *cache_util);
 *
 * description
 *
 *   This function computes the minimum over all temporal reuse maximum
 *   padding dimension entries in the given cache utilization table.
 *
 ******************************************************************************/

static int
ComputeTemporalReuseMaxPadDim (int dim, unsigned int rows, cache_util_t *cache_util)
{
    int max_paddim;
    unsigned int a;

    DBUG_ENTER ();

    max_paddim = dim;

    for (a = 0; a < rows - 1; a++) {
        if (cache_util[a].tr_potflag) {
            if (cache_util[a].sr_maxpaddim < max_paddim) {
                max_paddim = cache_util[a].sr_maxpaddim;
            }
        }
    }

    DBUG_RETURN (max_paddim);
}

/******************************************************************************
 *
 * function:
 *   static shape *
 *   UpdatePaddingVectorForSpatialReuse(unsigned int rows, cache_util_t *cache_util,
 *                                      int dim, shape *shp, shape *pv)
 *
 * description
 *
 *   This function tries to update the given padding vector pv for spatial
 *   reuse, i.e., it computes the next potential padding vector to be evaluated.
 *   If all potential padding vectors with respect to the resource allocation
 *   overhead limit have been tested, NULL is returned.
 *
 ******************************************************************************/

static shape *
UpdatePaddingVectorForSpatialReuse (unsigned int rows, cache_util_t *cache_util, int dim,
                                    shape *shp, shape *pv)
{
    shape *res = NULL;
    int min_paddim, max_paddim, current_paddim;

    DBUG_ENTER ();

    /*
     * Identify potential padding dimensions.
     */
    min_paddim = ComputeSpatialReuseMinPadDim (dim, rows, cache_util);
    max_paddim = ComputeSpatialReuseMaxPadDim (dim, rows, cache_util);

    current_paddim = min_paddim;

    do {
        /*
         * Update padding in current padding dimension.
         */
        SHsetExtent (pv, current_paddim, SHgetExtent (pv, current_paddim)+1);

        if (PIpaddingOverhead (dim, shp, pv) <= global.padding_overhead_limit) {
            res = pv;
            break;
        }

        /*
         * Current padding dimension exhausted, switch to next dimension.
         */
        SHsetExtent (pv, current_paddim, 0);
        current_paddim += 1;

        if (current_paddim > max_paddim) {
            /*
             * Maximum padding dimension already reached.
             */
            res = NULL;
        }
    } while (current_paddim <= max_paddim);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   static shape *
 *   UpdatePaddingVectorForTemporalReuse(unsigned int rows, cache_util_t *cache_util,
 *                                       int dim, shape *shp, shape *pv)
 *
 * description
 *
 *   This function tries to update the given padding vector pv for spatial
 *   reuse, i.e., it computes the next potential padding vector to be evaluated.
 *   If all potential padding vectors with respect to the resource allocation
 *   overhead limit have been tested, NULL is returned.
 *
 ******************************************************************************/

static shape *
UpdatePaddingVectorForTemporalReuse (unsigned int rows, cache_util_t *cache_util, int dim,
                                     shape *shp, shape *pv)
{
    shape *res = NULL;
    int min_paddim, max_paddim, current_paddim;

    DBUG_ENTER ();

    /*
     * Identify potential padding dimensions.
     */
    min_paddim = ComputeTemporalReuseMinPadDim (dim, rows, cache_util);
    max_paddim = ComputeTemporalReuseMaxPadDim (dim, rows, cache_util);

    /*
     * Note here that the following might hold:
     *
     *  min_paddim > max_paddim
     *
     * Nevertheless, we try to pad in min_paddim in this case.
     */

    current_paddim = min_paddim;

    do {
        /*
         * Update padding in current padding dimension.
         */
        SHsetExtent (pv, current_paddim, SHgetExtent (pv, current_paddim)+1);

        if (PIpaddingOverhead (dim, shp, pv) <= global.padding_overhead_limit) {
            res = pv;
            break;
        }

        /*
         * Current padding dimension exhausted, switch to next dimension.
         */
        SHsetExtent (pv, current_paddim, 0);
        current_paddim += 1;

        if (current_paddim > max_paddim) {
            /*
             * Maximum padding dimension already reached.
             */
            res = NULL;
        }
    } while (current_paddim <= max_paddim);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    static int
 *    EvaluatePadding( int *ret,
 *                     int dim, cache_t *cache,
 *                     unsigned int rows, cache_util_t *cache_util,
 *                     shape* shp,
 *                     shape* pv)
 *
 * description
 *
 *   This function evaluates the effectiveness of a given padding.
 *   It yields the number of spatial reuse conflicts and, in the location
 *   indicated by the first argument, the number of temporal reuse conflicts.
 *
 ******************************************************************************/

static int
EvaluatePadding (int *ret, int dim, cache_t *cache, unsigned int rows, cache_util_t *cache_util,
                 shape *shp, shape *pv)
{
    shape *actual_shape;
    int num_sr_conflicts;
    int num_tr_conflicts;

    DBUG_ENTER ();

    if (pv == NULL) {
        num_sr_conflicts = VERY_LARGE_NUMBER;
        num_tr_conflicts = VERY_LARGE_NUMBER;
    } else {

        /*
         * Compute actual array shape including padding.
         */
        actual_shape = SHmakeShape (dim);
        AddVect (dim, actual_shape, shp, pv);

        /*
         * Compute cache utilization table.
         */
        cache_util = ComputeAccessData (rows, cache_util, cache, dim, actual_shape);
        cache_util = ComputeSpatialReuse (rows, cache_util, cache, dim);
        cache_util = ComputeTemporalReuse (rows, cache_util, cache, dim);

        /*
         * Evaluate numbers of spatial and temporal reuse conflicts.
         */
        num_sr_conflicts = ComputeNumSpatialReuseConflicts (rows, cache_util);
        num_tr_conflicts = ComputeNumTemporalReuseConflicts (rows, cache_util);

        /*
         * Free local resources.
         */
        SHfreeShape (actual_shape);
    }

    *ret = num_tr_conflicts;

    DBUG_RETURN (num_sr_conflicts);
}

/******************************************************************************
 *
 * function:
 *   static shape *
 *   ComputePaddingForSpatialReuse( int dim, cache_t *cache,
 *                                  unsigned int rows, cache_util_t *cache_util,
 *                                  shape* shp,
 *                                  shape* pv)
 *
 * description
 *
 *
 *
 *
 ******************************************************************************/

static shape *
ComputePaddingForSpatialReuse (int dim, cache_t *cache, unsigned int rows,
                               cache_util_t *cache_util, shape *shp, shape *pv)
{
    shape *actual_shape;
    shape *pv_opt, *new_pv = NULL;
    int min_sr_conflicts;
    int num_sr_conflicts;

    DBUG_ENTER ();

    actual_shape = SHmakeShape (dim);
    pv_opt = SHmakeShape (dim);
    min_sr_conflicts = VERY_LARGE_NUMBER;

    do {
        /*
         * Compute actual array shape including padding.
         */
        AddVect (dim, actual_shape, shp, pv);

        /*
         * Compute cache utilization table.
         */
        cache_util = ComputeAccessData (rows, cache_util, cache, dim, actual_shape);
        cache_util = ComputeSpatialReuse (rows, cache_util, cache, dim);
        cache_util = ComputeTemporalReuse (rows, cache_util, cache, dim);

        /*
         * Produce diagnostic output.
         */
        APprintDiag ("\nCurrent state :  ");
        PIprintShpSeg (dim, shp);
        APprintDiag (" + ");
        PIprintShpSeg (dim, pv);
        APprintDiag (" -> ");
        PIprintShpSeg (dim, actual_shape);
        APprintDiag ("\nCurrent overhead :  <= %d%%\n\n",
                     PIpaddingOverhead (dim, shp, pv));

        PrintCacheUtil (dim, rows, cache_util);
        APprintDiag ("\n\n");

        /*
         * Evaluate numbers of spatial reuse conflicts.
         */
        num_sr_conflicts = ComputeNumSpatialReuseConflicts (rows, cache_util);

        if (num_sr_conflicts < min_sr_conflicts) {
            /*
             * The current padding reduces the number of spatial reuse conflicts.
             */
            if (min_sr_conflicts != VERY_LARGE_NUMBER) {
                APprintDiag (
                  "Current padding reduces spatial reuse conflicts: %d -> %d !\n",
                  min_sr_conflicts, num_sr_conflicts);
            } else {
                APprintDiag ("Current number of spatial reuse conflicts : %d\n",
                             num_sr_conflicts);
            }

            min_sr_conflicts = num_sr_conflicts;
            CopyVect (dim, pv_opt, pv);
        } else {
            APprintDiag ("Current number of spatial reuse conflicts : %d\n",
                         num_sr_conflicts);
        }

        if (num_sr_conflicts > 0) {
            /*
             * There are still spatial reuse conflicts. So, let's try some more
             * padding.
             */
            new_pv
              = UpdatePaddingVectorForSpatialReuse (rows, cache_util, dim, shp, pv);

            if (new_pv == NULL) {
                /*
                 * Padding limit reached, no further padding allowed.
                 * So, take best padding found so far.
                 */
                CopyVect (dim, pv, pv_opt);
                num_sr_conflicts = min_sr_conflicts;

                APprintDiag ("Padding overhead constraint of %d%% exhausted.\n",
                             global.padding_overhead_limit);
                APprintDiag ("Returning to padding vector ");
                PIprintShpSeg (dim, pv);
                APprintDiag (" .\n"
                             "This padding implies %d spatial reuse conflicts.\n\n",
                             num_sr_conflicts);
            } else {
                /*
                 * Additional padding still possible. So, let's go for it.
                 */
                pv = new_pv;
            }
        }

        /*
         * Repeat until either all spatial reuse conflicts are solved or
         * the maximum amount of padding allowed is reached.
         */
    } while ((num_sr_conflicts > 0) && (new_pv != NULL));

    /*
     * Free local resources.
     */
    SHfreeShape (actual_shape);
    SHfreeShape (pv_opt);

    DBUG_RETURN (pv);
}

/******************************************************************************
 *
 * function:
 *   static shape *
 *   ComputePaddingForTemporalReuse( int dim, cache_t *cache,
 *                                   unsigned int rows, cache_util_t *cache_util,
 *                                   shape* shp,
 *                                   shape* pv)
 *
 * description
 *
 *
 *
 *
 ******************************************************************************/

static shape *
ComputePaddingForTemporalReuse (int dim, cache_t *cache, unsigned int rows,
                                cache_util_t *cache_util, shape *shp, shape *pv)
{
    shape *actual_shape;
    shape *pv_opt;
    shape *new_pv = NULL;
    int min_sr_conflicts;
    int min_tr_conflicts;
    int num_sr_conflicts;
    int num_tr_conflicts;

    DBUG_ENTER ();

    actual_shape = SHmakeShape (dim);
    pv_opt = SHmakeShape (dim);
    min_sr_conflicts = VERY_LARGE_NUMBER;
    min_tr_conflicts = VERY_LARGE_NUMBER;

    do {
        /*
         * Compute actual array shape including padding.
         */
        AddVect (dim, actual_shape, shp, pv);

        /*
         * Compute cache utilization table.
         */
        cache_util = ComputeAccessData (rows, cache_util, cache, dim, actual_shape);
        cache_util = ComputeSpatialReuse (rows, cache_util, cache, dim);
        cache_util = ComputeTemporalReuse (rows, cache_util, cache, dim);

        /*
         * Produce diagnostic output.
         */
        APprintDiag ("\nCurrent state :  ");
        PIprintShpSeg (dim, shp);
        APprintDiag (" + ");
        PIprintShpSeg (dim, pv);
        APprintDiag (" -> ");
        PIprintShpSeg (dim, actual_shape);
        APprintDiag ("\nCurrent overhead :  <= %d%%\n\n",
                     PIpaddingOverhead (dim, shp, pv));

        PrintCacheUtil (dim, rows, cache_util);
        APprintDiag ("\n\n");

        /*
         * Evaluate numbers of spatial and temporal reuse conflicts.
         */
        num_sr_conflicts = ComputeNumSpatialReuseConflicts (rows, cache_util);
        num_tr_conflicts = ComputeNumTemporalReuseConflicts (rows, cache_util);

        if (num_sr_conflicts < min_sr_conflicts) {
            /*
             * The current padding reduces the number of spatial reuse conflicts.
             * This is for initialization only.
             */
            min_sr_conflicts = num_sr_conflicts;
            min_tr_conflicts = num_tr_conflicts;
            CopyVect (dim, pv_opt, pv);
            APprintDiag ("Current number of spatial reuse conflicts :  %d\n",
                         num_sr_conflicts);
            APprintDiag ("Current number of temporal reuse conflicts : %d\n",
                         num_tr_conflicts);
        } else {
            if (num_sr_conflicts == min_sr_conflicts) {
                /*
                 * Padding for temporal reuse is only pursued as long as the number
                 * of spatial reuse conflicts does not grow again because spatial
                 * reuse conflicts are generally much more severe than temporal
                 * reuse conflicts.
                 */

                if (num_tr_conflicts < min_tr_conflicts) {
                    /*
                     * The current padding does successfully reduce the number of
                     * temporal reuse conflicts.
                     */
                    APprintDiag (
                      "Current padding reduces temporal reuse conflicts: %d -> %d !\n",
                      min_tr_conflicts, num_tr_conflicts);
                    min_tr_conflicts = num_tr_conflicts;
                    CopyVect (dim, pv_opt, pv);
                } else {
                    APprintDiag ("Current number of temporal reuse conflicts : %d\n",
                                 num_tr_conflicts);
                }

                if (num_tr_conflicts > 0) {
                    /*
                     * There are still temporal reuse conflicts.
                     * So, let's try some more padding.
                     */
                    new_pv = UpdatePaddingVectorForTemporalReuse (rows, cache_util, dim,
                                                                  shp, pv);

                    if (new_pv == NULL) {
                        /*
                         * Padding limit reached, no further padding allowed.
                         * So, take best padding found so far.
                         */
                        CopyVect (dim, pv, pv_opt);
                        num_tr_conflicts = min_tr_conflicts;

                        APprintDiag ("Padding overhead constraint of %d%% exhausted.\n",
                                     global.padding_overhead_limit);
                        APprintDiag ("Returning to padding vector ");
                        PIprintShpSeg (dim, pv);
                        APprintDiag (" .\n"
                                     "This padding implies %d temporal reuse "
                                     "conflicts.\n\n",
                                     num_tr_conflicts);
                    } else {
                        /*
                         * Additional padding still possible. So, let's go for it.
                         */
                        pv = new_pv;
                    }
                }
            } else {
                /*
                 * The current padding increases the number of spatial reuse
                 * conflicts again. Therefore, we stop at this point and take
                 * the best padding found so far.
                 */
                CopyVect (dim, pv, pv_opt);
                num_tr_conflicts = min_tr_conflicts;

                APprintDiag (
                  "Current padding increases spatial reuses conflicts: %d -> %d !\n",
                  min_sr_conflicts, num_sr_conflicts);
                APprintDiag ("Returning to padding vector ");
                PIprintShpSeg (dim, pv);
                APprintDiag (" .\n"
                             "This padding implies %d temporal reuse conflicts.\n\n",
                             num_tr_conflicts);
            }
        }

        /*
         * Repeat until either all spatial reuse conflicts are solved or
         * the maximum amount of padding allowed is reached.
         */
    } while ((num_tr_conflicts > 0) && (new_pv != NULL));

    /*
     * Free local resources.
     */
    SHfreeShape (actual_shape);
    SHfreeShape (pv_opt);

    DBUG_RETURN (pv);
}

/******************************************************************************
 *
 * function:
 *   static shape *
 *   ComputePadding( cache_t *cache_L1,  cache_t *cache_L1,  cache_t *cache_L1,
 *                   int dim, shape* shp, shape *padding,
 *                   pattern_t *pattern, array_type_t *array)
 *
 * description
 *
 *
 *
 *
 ******************************************************************************/

static shape *
ComputePadding (cache_t *cache_L1, cache_t *cache_L2, cache_t *cache_L3, int dim,
                shape *shp, shape *padding, pattern_t *pattern, array_type_t *array)
{
    cache_util_t *cache_util;
    unsigned int rows;
    shape *padding_keep;
    shape *padding_store;
    int num_sr_conflicts_P1;
    int num_sr_conflicts_P2;
    int num_tr_conflicts_P1;
    int num_tr_conflicts_P2;

    DBUG_ENTER ();

    rows = InitCacheUtil (&cache_util, pattern, array);

    padding_keep = SHmakeShape (dim);
    padding_store = SHmakeShape (dim);

    if (cache_L1 != NULL) {

        APprintDiag ("\n----------------------------------------------------\n"
                     "Inferring padding vector with respect to L1 cache :\n"
                     "----------------------------------------------------\n\n");

        padding = ComputePaddingForSpatialReuse (dim, cache_L1, rows, cache_util, shp,
                                                 padding);

        padding = ComputePaddingForTemporalReuse (dim, cache_L1, rows, cache_util, shp,
                                                  padding);

        if (cache_L2 != NULL) {
            /*
             * Store padding inferred so far.
             */
            CopyVect (dim, padding_keep, padding);

            APprintDiag ("\nRecommended padding for L1 cache :  ");
            PIprintShpSeg (dim, padding);
            APprintDiag ("\n\n");

            APprintDiag ("\n----------------------------------------------------\n"
                         "Inferring padding vector with respect to L2 cache :\n"
                         "----------------------------------------------------\n\n");

            do {
                CopyVect (dim, padding_store, padding);

                padding = ComputePaddingForSpatialReuse (dim, cache_L2, rows, cache_util,
                                                         shp, padding);

                padding = ComputePaddingForTemporalReuse (dim, cache_L2, rows, cache_util,
                                                          shp, padding);

                if (EqualVect (dim, padding, padding_store)) {
                    break;
                }

                padding = ComputePaddingForSpatialReuse (dim, cache_L1, rows, cache_util,
                                                         shp, padding);

                padding = ComputePaddingForTemporalReuse (dim, cache_L1, rows, cache_util,
                                                          shp, padding);
            } while (!EqualVect (dim, padding, padding_store));

            APprintDiag ("\nRecommended padding for L2 cache :  ");
            PIprintShpSeg (dim, padding);
            APprintDiag ("\n\n");

            /*
             * Evaluate both padding candidates.
             */
            num_sr_conflicts_P1 = EvaluatePadding (&num_tr_conflicts_P1, dim, cache_L1,
                                                   rows, cache_util, shp, padding_keep);
            num_sr_conflicts_P2 = EvaluatePadding (&num_tr_conflicts_P2, dim, cache_L1,
                                                   rows, cache_util, shp, padding);

            if ((num_sr_conflicts_P1 < num_sr_conflicts_P2)
                || (num_tr_conflicts_P1 < num_tr_conflicts_P2)) {
                /*
                 * The current padding is not as good for L1 cache as the
                 * first one.
                 */
                CopyVect (dim, padding, padding_keep);

                APprintDiag ("Current padding increases conflicts in L1 cache !\n");
                APprintDiag ("Returning to padding vector ");
                PIprintShpSeg (dim, padding);
                APprintDiag (" .\n");
            }

            if (cache_L3 != NULL) {
                /*
                 * Store padding inferred so far.
                 */
                CopyVect (dim, padding_keep, padding);

                APprintDiag ("\n----------------------------------------------------\n"
                             "Inferring padding vector with respect to L3 cache :\n"
                             "----------------------------------------------------\n\n");

                do {
                    CopyVect (dim, padding_store, padding);

                    padding = ComputePaddingForSpatialReuse (dim, cache_L3, rows,
                                                             cache_util, shp, padding);

                    padding = ComputePaddingForTemporalReuse (dim, cache_L3, rows,
                                                              cache_util, shp, padding);

                    if (EqualVect (dim, padding, padding_store)) {
                        break;
                    }

                    padding = ComputePaddingForSpatialReuse (dim, cache_L2, rows,
                                                             cache_util, shp, padding);

                    padding = ComputePaddingForTemporalReuse (dim, cache_L2, rows,
                                                              cache_util, shp, padding);

                    padding = ComputePaddingForSpatialReuse (dim, cache_L1, rows,
                                                             cache_util, shp, padding);

                    padding = ComputePaddingForTemporalReuse (dim, cache_L1, rows,
                                                              cache_util, shp, padding);

                } while (!EqualVect (dim, padding, padding_store));

                APprintDiag ("\nRecommended padding for L3 cache :  ");
                PIprintShpSeg (dim, padding);
                APprintDiag ("\n\n");

                /*
                 * Evaluate both padding candidates with respect to L1 cache.
                 */
                num_sr_conflicts_P1
                  = EvaluatePadding (&num_tr_conflicts_P1, dim, cache_L1, rows,
                                     cache_util, shp, padding_keep);
                num_sr_conflicts_P2
                  = EvaluatePadding (&num_tr_conflicts_P2, dim, cache_L1, rows,
                                     cache_util, shp, padding);

                if ((num_sr_conflicts_P1 < num_sr_conflicts_P2)
                    || (num_tr_conflicts_P1 < num_tr_conflicts_P2)) {
                    /*
                     * The current padding is not as good for L1 cache as the
                     * first one.
                     */
                    CopyVect (dim, padding, padding_keep);

                    APprintDiag ("Current padding increases conflicts in L1 cache !\n");
                    APprintDiag ("Returning to padding vector ");
                    PIprintShpSeg (dim, padding);
                    APprintDiag (" .\n");
                } else {
                    /*
                     * Evaluate both padding candidates with respect to L2 cache.
                     */
                    num_sr_conflicts_P1
                      = EvaluatePadding (&num_tr_conflicts_P1, dim, cache_L2, rows,
                                         cache_util, shp, padding_keep);
                    num_sr_conflicts_P2
                      = EvaluatePadding (&num_tr_conflicts_P2, dim, cache_L2, rows,
                                         cache_util, shp, padding);

                    if ((num_sr_conflicts_P1 < num_sr_conflicts_P2)
                        || (num_tr_conflicts_P1 < num_tr_conflicts_P2)) {
                        /*
                         * The current padding is not as good for L2 cache as the
                         * first one.
                         */
                        CopyVect (dim, padding, padding_keep);

                        APprintDiag (
                          "Current padding increases conflicts in L2 cache !\n");
                        APprintDiag ("Returning to padding vector ");
                        PIprintShpSeg (dim, padding);
                        APprintDiag (" .\n");
                    }
                }
            }
        }
    }

    /*
     * Free local resources.
     */
    cache_util = MEMfree (cache_util);
    padding_keep = MEMfree (padding_keep);
    padding_store = MEMfree (padding_store);

    DBUG_RETURN (padding);
}

/*****************************************************************************
 *
 * function:
 *   void APinfer (void)
 *
 * description:
 *
 *   main function for infering new shapes for array padding
 *
 *****************************************************************************/

void
APinfer (void)
{
    shape *padding;
    shape *initial_padding;

    cache_t *cache_L1;
    cache_t *cache_L2;
    cache_t *cache_L3;

    simpletype type;
    int dim;
    shape *shp;
    int element_size;

    array_type_t *at_ptr;
    conflict_group_t *cg_ptr;
    pattern_t *pt_ptr;

    shape *new_shape;

    DBUG_ENTER ();

    DBUG_PRINT ("Array Padding: infering new shapes...");

    /*
     * Clean up collected information.
     */
    PItidyAccessPattern ();
    PIprintAccessPatterns ();


    /* for every array type... */
    at_ptr = PIgetFirstArrayType ();
    while (at_ptr != NULL) {

        APprintDiag (
          "\n\n**********************************************************************\n"
          "**********************************************************************\n\n"
          "Inferring recommended padding for array type:\n");
        PIprintArrayTypeElement (at_ptr);

        /*
         * Extract information concerning array data type.
         */
        dim = PIgetArrayTypeDim (at_ptr);
        /*
         * Init padding vectors.
         */
        padding = SHmakeShape (dim);
        initial_padding = SHmakeShape (dim);

        shp = SHcopyShape (PIgetArrayTypeShape (at_ptr));
        type = PIgetArrayTypeBasetype (at_ptr);
        element_size = ctype_size[type];

        /*
         * Create concise cache specifications.
         */
        cache_L1
          = CreateCacheSpec (global.config.cache1_size * 1024, global.config.cache1_line,
                             global.config.cache1_assoc, element_size);

        cache_L2
          = CreateCacheSpec (global.config.cache2_size * 1024, global.config.cache2_line,
                             global.config.cache2_assoc, element_size);

        cache_L3
          = CreateCacheSpec (global.config.cache3_size * 1024, global.config.cache3_line,
                             global.config.cache3_assoc, element_size);

        APprintDiag ("\nInternal cache specification (sizes in array elements) :\n");

        PrintCacheSpec (1, cache_L1);
        PrintCacheSpec (2, cache_L2);
        PrintCacheSpec (3, cache_L3);

        /*
         * Reset padding vector.
         */
        SetVect (dim, padding, 0);

        /*
         * Get access patterns for each conflict group
         */
        cg_ptr = PIgetFirstConflictGroup (at_ptr);
        while (cg_ptr != NULL) {

            APprintDiag (
              "\n************************************************************\n"
              "Inferring recommended padding for given conflict group:\n\n");
            PIprintConflictGroupElement (at_ptr, cg_ptr);

            /* for every access pattern infer new shape */
            /* Why is there no loop across the access patterns of a conflict group? */

            pt_ptr = PIgetFirstPattern (cg_ptr);

            CopyVect (dim, initial_padding, padding);

            padding = ComputePadding (cache_L1, cache_L2, cache_L3, dim, shp, padding,
                                      pt_ptr, at_ptr);

            APprintDiag ("\nOriginal shape vector       :  ");
            PIprintShpSeg (dim, shp);

            APprintDiag ("\nInitial padding vector      :  ");
            PIprintShpSeg (dim, initial_padding);

            APprintDiag ("\nRecommended padding vector  :  ");
            PIprintShpSeg (dim, padding);

            APprintDiag ("\nMemory allocation overhead  :  <= %d%%\n\n",
                         PIpaddingOverhead (dim, shp, padding));

            cg_ptr = PIgetNextConflictGroup (cg_ptr);
        }

        /*
         * Padding inference finished for one array type.
         */

        new_shape = SHmakeShape (dim);
        AddVect (dim, new_shape, shp, padding);

        APprintDiag (
          "\n*****************************************************************\n"
          "*\n"
          "* Final padding recommendation for array type:\n"
          "*\n"
          "*  Original array type         :  %s",
          CVbasetype2String (type));
        PIprintShpSeg (dim, shp);

        APprintDiag ("\n"
                     "*  Recommended padding vector  :  ");
        PIprintShpSeg (dim, padding);

        APprintDiag ("\n"
                     "*  Resulting array type        :  %s",
                     CVbasetype2String (type));
        PIprintShpSeg (dim, new_shape);

        APprintDiag ("\n"
                     "*  Memory allocation overhead  :  <= %d%%",
                     PIpaddingOverhead (dim, shp, padding));

        APprintDiag (
          "\n*\n"
          "*****************************************************************\n\n");

        /*
         * If current array type needs padding, add to pad_info.
         */

        if (EqualVect (dim, shp, new_shape)) {
            /*
             * No padding recommended.
             */
            SHfreeShape (shp);
            SHfreeShape (new_shape);
        } else {
            PIaddInferredShape (type, dim, shp, new_shape, SHcopyShape (padding));
        }

        /*
         * Remove array type specific internal cache specifications.
         */
        if (cache_L1 != NULL) {
            cache_L1 = MEMfree (cache_L1);
        }

        if (cache_L2 != NULL) {
            cache_L2 = MEMfree (cache_L2);
        }

        if (cache_L3 != NULL) {
            cache_L3 = MEMfree (cache_L3);
        }

        padding = SHfreeShape (padding);
        initial_padding = SHfreeShape (initial_padding);

        /*
         * Infer padding for next array type.
         */
        at_ptr = PIgetNextArrayType (at_ptr);
    }

    PIprintPadInfo ();

    PIremoveUnsupportedShapes ();

    PIprintPadInfo ();

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
