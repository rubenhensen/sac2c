
#include "cuda_data_reuse.h"

#include <stdlib.h>

#include "tree_basic.h"
#include "tree_compound.h"
#include "globals.h"
#include "memory.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "print.h"
#include "new_types.h"
#include "shape.h"
#include "types.h"
#include "constants.h"
#include "pattern_match.h"
#include "pattern_match_attribs.h"
#include "cuda_utils.h"
#include "LookUpTable.h"
#include "int_matrix.h"
#include "types.h"
#include "cuda_utils.h"
#include "create_loop_fun.h"
#include "create_cond_fun.h"

typedef struct CUIDX_SET_T {
    node *tx;
    node *ty;
    node *bx;
    node *by;
} cuidx_set_t;

#define CIS_TX(n) (n->tx)
#define CIS_TY(n) (n->ty)
#define CIS_BX(n) (n->bx)
#define CIS_BY(n) (n->by)

typedef struct RANGE_INFO_T {
    node *range;
    bool toplevel;
    struct RANGE_INFO_T *prev;
    struct RANGE_INFO_T *next;
} range_info_t;

typedef struct RANGE_SET_T {
    range_info_t *last_blocked_range;
    range_info_t *last_nonblocked_range;
    range_info_t *blocked_ranges;
    range_info_t *nonblocked_ranges;
    size_t blocked_ranges_count;
    int nonblocked_ranges_count;
    struct RANGE_SET_T *prev;
    struct RANGE_SET_T *next;
} range_set_t;

#define RI_RANGE(n) (n->range)
#define RI_TOPLEVEL(n) (n->toplevel)
#define RI_PREV(n) (n->prev)
#define RI_NEXT(n) (n->next)

#define RS_LAST_BLOCKED_RANGE(n) (n->last_blocked_range)
#define RS_LAST_NONBLOCKED_RANGE(n) (n->last_nonblocked_range)
#define RS_BLOCKED_RANGES(n) (n->blocked_ranges)
#define RS_NONBLOCKED_RANGES(n) (n->nonblocked_ranges)
#define RS_BLOCKED_RANGES_CNT(n) (n->blocked_ranges_count)
#define RS_NONBLOCKED_RANGES_CNT(n) (n->nonblocked_ranges_count)

#if 0
//currently not used....
#define RS_RANGES_CNT(n) (((n->blocked_ranges_count) + (n->nonblocked_ranges_count)))
#endif

#define RS_PREV(n) (n->prev)
#define RS_NEXT(n) (n->next)

/* First set, i.e. the one at the bottom of the stack */
static range_set_t *first_range_set = NULL;
static int range_set_cnt = 0;

/** <!--********************************************************************-->
 *
 *             Functions for Range Info
 *
 *****************************************************************************/

static range_info_t *
CreateRangeInfo (node *range, bool toplevel)
{
    range_info_t *res;

    DBUG_ENTER ();

    res = (range_info_t *)MEMmalloc (sizeof (range_info_t));

    RI_RANGE (res) = range;
    RI_TOPLEVEL (res) = toplevel;
    RI_PREV (res) = NULL;
    RI_NEXT (res) = NULL;

    DBUG_RETURN (res);
}

static range_info_t *
FreeRangeInfo (range_info_t *info)
{
    DBUG_ENTER ();

    if (info == NULL) {
        return info;
    } else {
        RI_NEXT (info) = FreeRangeInfo (RI_NEXT (info));
        info = MEMfree (info);
        info = NULL;
    }

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 *             Functions for Range Set
 *
 *****************************************************************************/

static range_set_t *
EnqRangeInfo (range_set_t *set, node *range, bool toplevel)
{
    range_info_t *info, *next, *dummy;

    DBUG_ENTER ();

    info = CreateRangeInfo (range, toplevel);

    if (RANGE_ISBLOCKED (range)) {
        if (RS_LAST_BLOCKED_RANGE (set) == NULL) {
            RS_BLOCKED_RANGES (set) = info;
        } else {
            RI_NEXT (RS_LAST_BLOCKED_RANGE (set)) = info;
            RI_PREV (info) = RS_LAST_BLOCKED_RANGE (set);
        }
        RS_BLOCKED_RANGES_CNT (set)++;
        RS_LAST_BLOCKED_RANGE (set) = info;

        /****** Pay attention to the following code ******/

        if (RS_NONBLOCKED_RANGES (set) != NULL
            && RI_TOPLEVEL (RS_NONBLOCKED_RANGES (set))) {
            RI_PREV (RS_BLOCKED_RANGES (set)) = RS_NONBLOCKED_RANGES (set);
            next = RI_NEXT (RS_NONBLOCKED_RANGES (set));
            RI_NEXT (RS_NONBLOCKED_RANGES (set)) = RS_BLOCKED_RANGES (set);
            RS_BLOCKED_RANGES (set) = RS_NONBLOCKED_RANGES (set);
            dummy = CreateRangeInfo (TBmakeRange (NULL, NULL, NULL, NULL, NULL, NULL,
                                                  NULL, NULL),
                                     FALSE);
            RI_NEXT (dummy) = next;
            RI_PREV (next) = dummy;
            RS_NONBLOCKED_RANGES (set) = dummy;
            RS_BLOCKED_RANGES_CNT (set)++;
        }
        /*************************************************/
    } else {
        if (RS_LAST_NONBLOCKED_RANGE (set) == NULL) {
            RS_NONBLOCKED_RANGES (set) = info;
        } else {
            RI_NEXT (RS_LAST_NONBLOCKED_RANGE (set)) = info;
            RI_PREV (info) = RS_LAST_NONBLOCKED_RANGE (set);
        }
        RS_NONBLOCKED_RANGES_CNT (set)++;
        RS_LAST_NONBLOCKED_RANGE (set) = info;
    }

    DBUG_RETURN (set);
}

static range_set_t *
DeqRangeInfo (range_set_t *set, node *range)
{
    range_info_t *info;

    DBUG_ENTER ();

    if (RS_LAST_BLOCKED_RANGE (set) != NULL
        && range == RI_RANGE (RS_LAST_BLOCKED_RANGE (set))) {
        info = RS_LAST_BLOCKED_RANGE (set);
        RS_LAST_BLOCKED_RANGE (set) = RI_PREV (info);
        if (RS_LAST_BLOCKED_RANGE (set) != NULL) {
            RI_NEXT (RS_LAST_BLOCKED_RANGE (set)) = NULL;
        } else {
            RS_BLOCKED_RANGES (set) = NULL;
        }
        RS_BLOCKED_RANGES_CNT (set)--;
        info = MEMfree (info);
    } else if (RS_LAST_NONBLOCKED_RANGE (set) != NULL
               && range == RI_RANGE (RS_LAST_NONBLOCKED_RANGE (set))) {
        info = RS_LAST_NONBLOCKED_RANGE (set);
        RS_LAST_NONBLOCKED_RANGE (set) = RI_PREV (info);
        if (RS_LAST_NONBLOCKED_RANGE (set) != NULL) {
            RI_NEXT (RS_LAST_NONBLOCKED_RANGE (set)) = NULL;
        } else {
            RS_NONBLOCKED_RANGES (set) = NULL;
        }
        RS_NONBLOCKED_RANGES_CNT (set)--;
        info = MEMfree (info);
    } else {
        DBUG_UNREACHABLE ("N_range in neither blocked nor nonblocked ranges!");
    }

    DBUG_RETURN (set);
}

static range_set_t *
CreateRangeSet (void)
{
    range_set_t *res;

    DBUG_ENTER ();

    res = (range_set_t *)MEMmalloc (sizeof (range_set_t));

    RS_LAST_BLOCKED_RANGE (res) = NULL;
    RS_LAST_NONBLOCKED_RANGE (res) = NULL;
    RS_BLOCKED_RANGES (res) = NULL;
    RS_NONBLOCKED_RANGES (res) = NULL;
    RS_BLOCKED_RANGES_CNT (res) = 0;
    RS_NONBLOCKED_RANGES_CNT (res) = 0;
    RS_PREV (res) = NULL;
    RS_NEXT (res) = NULL;

    DBUG_RETURN (res);
}

static range_set_t *
PushRangeSet (range_set_t *sets)
{
    range_set_t *new_set;

    DBUG_ENTER ();

    new_set = CreateRangeSet ();

    if (sets == NULL) {
        sets = new_set;
        /* global variables recording the range set stack info */
        first_range_set = sets;
        range_set_cnt++;
    } else {
        RS_NEXT (new_set) = sets;
        RS_PREV (sets) = new_set;
        sets = new_set;
    }

    DBUG_RETURN (sets);
}

static range_set_t *
PopRangeSet (range_set_t *sets)
{
    range_set_t *popped_set;

    DBUG_ENTER ();

    if (sets != NULL) {
        popped_set = sets;
        sets = RS_NEXT (sets);
        if (sets != NULL) {
            RS_PREV (sets) = NULL;
        }
        RS_NEXT (popped_set) = NULL;

        RS_BLOCKED_RANGES (popped_set) = FreeRangeInfo (RS_BLOCKED_RANGES (popped_set));
        RS_NONBLOCKED_RANGES (popped_set)
          = FreeRangeInfo (RS_NONBLOCKED_RANGES (popped_set));
        popped_set = MEMfree (popped_set);

        range_set_cnt--;
        if (range_set_cnt == 0) {
            first_range_set = NULL;
            sets = NULL;
        }
    }

    DBUG_RETURN (sets);
}

/*
static void PrintSpaces(int num)
{
  int i;

  DBUG_ENTER( "PrintSpaces");

  for( i = 0; i < num; i++) {
    DBUG_PRINT( "CUDR", ("  "));
  }

  DBUG_VOID_RETURN;
}

static void PrintRangeSet( range_set_t *sets, int indent)
{
  range_info_t *blocked_ranges;
  range_info_t *nonblocked_ranges;

  DBUG_ENTER( "PrintRangeSet");

  blocked_ranges = RS_BLOCKED_RANGES( sets);
  nonblocked_ranges = RS_NONBLOCKED_RANGES( sets);

  PrintSpaces( indent);
  DBUG_PRINT( "CUDR", ("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"));

  PrintSpaces( indent);
  if( RS_LAST_BLOCKED_RANGE( sets) != NULL) {
    DBUG_PRINT( "CUDR", ("Last Blocked Range: %s[toplevel:%d]\n",
                IDS_NAME( RANGE_INDEX( RI_RANGE( RS_LAST_BLOCKED_RANGE( sets)))),
                RI_TOPLEVEL( RS_LAST_BLOCKED_RANGE( sets))));
  }
  else {
    DBUG_PRINT( "CUDR", ("Last Blocked Range: NULL\n"));
  }


  PrintSpaces( indent);
  if( RS_LAST_NONBLOCKED_RANGE( sets) != NULL) {
    DBUG_PRINT( "CUDR", ("Last Nonblocked Range: %s[toplevel:%d]\n",
                IDS_NAME( RANGE_INDEX( RI_RANGE( RS_LAST_NONBLOCKED_RANGE( sets)))),
                RI_TOPLEVEL( RS_LAST_NONBLOCKED_RANGE( sets))));
  }
  else {
    DBUG_PRINT( "CUDR", ("Last Nonblocked Range: NULL\n"));
  }

  PrintSpaces( indent);
  DBUG_PRINT( "CUDR", ("Blocked Ranges[%d]: ", RS_BLOCKED_RANGES_CNT( sets)));
  while( blocked_ranges != NULL) {
    DBUG_PRINT( "CUDR", ("(Index:%s) ", IDS_NAME( RANGE_INDEX( RI_RANGE(
blocked_ranges))))); blocked_ranges = RI_NEXT( blocked_ranges);
  }
  DBUG_PRINT("CUDR", ("\n"));

  PrintSpaces( indent);
  DBUG_PRINT( "CUDR", ("Nonblocked Ranges[%d]: ", RS_NONBLOCKED_RANGES_CNT( sets)));
  while( nonblocked_ranges != NULL) {
    if( RANGE_INDEX( RI_RANGE( nonblocked_ranges)) == NULL) {
      DBUG_PRINT( "CUDR", ("(Index:Dummy) "));
    }
    else {
      DBUG_PRINT( "CUDR", ("(Index:%s) ", IDS_NAME( RANGE_INDEX( RI_RANGE(
nonblocked_ranges)))));
    }
    nonblocked_ranges = RI_NEXT( nonblocked_ranges);
  }
  DBUG_PRINT( "CUDR", ("\n"));

  PrintSpaces( indent);
  DBUG_PRINT("CUDR", ("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"));

  DBUG_VOID_RETURN;
}
*/

typedef struct RANGE_PAIR_T {
    node *outer;
    node *inner;
    size_t outerlevel;
    struct RANGE_PAIR_T *next;
} range_pair_t;

#define RP_OUTER(n) (n->outer)
#define RP_INNER(n) (n->inner)
#define RP_OUTERLEVEL(n) (n->outerlevel)
#define RP_NEXT(n) (n->next)

typedef struct SHARED_GLOBAL_INFO_T {
    node *shridx_cal; /* assignments for shared memory index calculation */
    node *glbidx_cal; /* assignments for new global memory index calculation */
    node *shravis;
    node *glbavis;
    range_pair_t *range_pairs;
    struct SHARED_GLOBAL_INFO_T *next;
} shared_global_info_t;

#define SG_INFO_SHRIDX_CAL(n) (n->shridx_cal)
#define SG_INFO_GLBIDX_CAL(n) (n->glbidx_cal)
#define SG_INFO_SHRAVIS(n) (n->shravis)
#define SG_INFO_GLBAVIS(n) (n->glbavis)
#define SG_INFO_RANGE_PAIRS(n) (n->range_pairs)
#define SG_INFO_NEXT(n) (n->next)

/** <!--********************************************************************-->
 *
 *             Functions for Range Pair
 *
 *****************************************************************************/

static range_pair_t *
FreeRangePair (range_pair_t *pairs)
{
    DBUG_ENTER ();

    if (pairs != NULL) {
        if (RP_NEXT (pairs) != NULL) {
            RP_NEXT (pairs) = FreeRangePair (RP_NEXT (pairs));
        }

        pairs = MEMfree (pairs);
        pairs = NULL;
    }

    DBUG_RETURN (pairs);
}

static range_pair_t *
GetNthRangePair (size_t nth)
{
    range_pair_t *pair = NULL;
    range_set_t *sets;
    range_info_t *blocked, *nonblocked;
    size_t cnt = 1, old_nth;

    DBUG_ENTER ();

    old_nth = nth;
    sets = first_range_set;

    while (sets != NULL) {
        if (nth > RS_BLOCKED_RANGES_CNT (sets)) {
            nth -= RS_BLOCKED_RANGES_CNT (sets);
        } else {
            blocked = RS_BLOCKED_RANGES (sets);
            nonblocked = RS_NONBLOCKED_RANGES (sets);
            while (cnt < nth) {
                DBUG_ASSERT (blocked != NULL, "Blocked range list is NULL!");
                DBUG_ASSERT (nonblocked != NULL, "Nonblocked range list is NULL!");
                blocked = RI_NEXT (blocked);
                nonblocked = RI_NEXT (RI_NEXT (nonblocked));
                cnt++;
            }
            pair = (range_pair_t *)MEMmalloc (sizeof (range_pair_t));
            RP_OUTER (pair) = RI_RANGE (blocked);
            RP_INNER (pair) = RI_RANGE (nonblocked);
            RP_OUTERLEVEL (pair) = old_nth;
            RP_NEXT (pair) = NULL;
            break;
        }
        sets = RS_PREV (sets);
    }

    DBUG_RETURN (pair);
}

/** <!--********************************************************************-->
 *
 *             Functions for Shared GLobal Info
 *
 *****************************************************************************/

static shared_global_info_t *
CreateSharedGlobalInfo (shared_global_info_t **sg_infos)
{
    shared_global_info_t *res, *tmp;

    DBUG_ENTER ();

    /* Append new sg_info to the prev_sg_infos */
    res = (shared_global_info_t *)MEMmalloc (sizeof (shared_global_info_t));

    SG_INFO_SHRIDX_CAL (res) = NULL;
    SG_INFO_GLBIDX_CAL (res) = NULL;
    SG_INFO_SHRAVIS (res) = NULL;
    SG_INFO_GLBAVIS (res) = NULL;
    SG_INFO_RANGE_PAIRS (res) = NULL;
    SG_INFO_NEXT (res) = NULL;

    if (*sg_infos != NULL) {
        tmp = *sg_infos;
        while (SG_INFO_NEXT (tmp) != NULL) {
            tmp = SG_INFO_NEXT (tmp);
        }
        SG_INFO_NEXT (tmp) = res;
    } else {
        *sg_infos = res;
    }

    DBUG_RETURN (res);
}

static shared_global_info_t *
FreeSharedGlobalInfo (shared_global_info_t *sg_infos)
{
    range_pair_t *pairs;

    DBUG_ENTER ();

    if (sg_infos != NULL) {
        if (SG_INFO_NEXT (sg_infos) != NULL) {
            SG_INFO_NEXT (sg_infos) = FreeSharedGlobalInfo (SG_INFO_NEXT (sg_infos));
        }

        pairs = SG_INFO_RANGE_PAIRS (sg_infos);
        pairs = FreeRangePair (pairs);
        sg_infos = MEMfree (sg_infos);
        sg_infos = NULL;
    }

    DBUG_RETURN (sg_infos);
}

/*
 * INFO structure
 */
struct INFO {
    size_t level;
    node *fundef;
    node *lastassign;
    range_set_t *range_sets;
    node *with3;
    size_t cuwldim;
    cuidx_set_t *cis;
    node *preassigns;
    node *g2s_assigns;
    node *condfuns;
    bool fromap;
    node *cuwlpart;
};

#define INFO_LEVEL(n) (n->level)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LASTASSIGN(n) (n->lastassign)
#define INFO_RANGE_SETS(n) (n->range_sets)
#define INFO_WITH3(n) (n->with3)
#define INFO_CUWLDIM(n) (n->cuwldim)
#define INFO_CIS(n) (n->cis)
#define INFO_PREASSIGNS(n) (n->preassigns)
#define INFO_G2S_ASSIGNS(n) (n->g2s_assigns)
#define INFO_LACFUNS(n) (n->condfuns)
#define INFO_FROMAP(n) (n->fromap)
#define INFO_CUWLPART(n) (n->cuwlpart)

/** <!--********************************************************************-->
 *
 *             Functions for arg_info
 *
 *****************************************************************************/
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_LEVEL (result) = 0;
    INFO_FUNDEF (result) = NULL;
    INFO_LASTASSIGN (result) = NULL;
    INFO_RANGE_SETS (result) = NULL;
    INFO_WITH3 (result) = NULL;
    INFO_CUWLDIM (result) = 0;
    INFO_CIS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_G2S_ASSIGNS (result) = NULL;
    INFO_LACFUNS (result) = NULL;
    INFO_FROMAP (result) = FALSE;
    INFO_CUWLPART (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 *             Internal Helper Functions
 *
 *****************************************************************************/

static node *
CreatePrfOrConst (bool isprf, char *name, simpletype sty, shape *shp, prf pfun,
                  node *args, node **vardecs_p, node **assigns_p)
{
    node *avis = NULL, *new_assign;

    DBUG_ENTER ();

    if (name != NULL) {
        avis
          = TBmakeAvis (TRAVtmpVarName (name), TYmakeAKS (TYmakeSimpleType (sty), shp));

        *vardecs_p = TBmakeVardec (avis, *vardecs_p);
    }

    new_assign = TBmakeAssign (TBmakeLet ((avis == NULL) ? avis : TBmakeIds (avis, NULL),
                                          (isprf) ? TBmakePrf (pfun, args) : args),
                               NULL);

    if (avis != NULL) {
        AVIS_SSAASSIGN (avis) = new_assign;
    }

    if (*assigns_p == NULL) {
        *assigns_p = new_assign;
    } else {
        *assigns_p = TCappendAssign (*assigns_p, new_assign);
    }

    DBUG_RETURN (avis);
}
/*
      sg_info = ComputeIndexInternal( TRUE, "loop_glb",
                                      TBmakeId( IDS_AVIS( RANGE_INDEX( RP_OUTER( pair)))),
                                      TBmakeNum( CUIDX_COEFFICIENT( idx)),
                                      FALSE, NULL, TRUE, sg_info, arg_info);
*/
/* This function works out the subscript computations for the
 * new shared memory access and the global memory access.
 * This includes the assignments of subscript computation and
 * the final output avis(s) from the assignment chains. All
 * are stored in the shared_global_info struct sg_info. Note
 * that for shared memory, they will be directly used later in
 * ComputeSharedMemoryOffset. However, for global memory, this
 * only provides the base and more computation might be needed,
 * for example, when the thread block size is small than the
 * shared meory size. See function InsertGlobal2Shared. */
static shared_global_info_t *
ComputeIndexInternal (bool global, /* for global or shared memory access? */
                      char *postfix, node *idx, node *coefficient, bool needsub,
                      node *operand, bool prf, shared_global_info_t *sg_info,
                      info *arg_info)
{
    node *avis, *args, *vardecs = NULL, *assigns = NULL;

    DBUG_ENTER ();

    /* Whether this idx need to substract a value first before it
     * it's mulitplied by the coefficient. This is the case for
     * threadIdx.x and threaddx.y in global memory access. If
     * this is TRUE, then the argument "operand" denotes the
     * amount to be substracted. */
    if (needsub) {
        args = TBmakeExprs (idx, TBmakeExprs (operand, NULL));
        avis = CreatePrfOrConst (TRUE, postfix, T_int, SHmakeShape (0), F_sub_SxS, args,
                                 &vardecs, &assigns);

        idx = TBmakeId (avis);
    }

    /* When "idx" is a constant, we pass NULL "idx" */
    if (idx != NULL) {
        args = TBmakeExprs (coefficient, TBmakeExprs (idx, NULL));
    } else {
        args = coefficient;
    }

    /* Multiply the index by the coefficient */
    avis = CreatePrfOrConst (prf, postfix, T_int, SHmakeShape (0), F_mul_SxS, args,
                             &vardecs, &assigns);

    /* Are we computing the index for global or shared memory access? */
    if (global) {
        if (SG_INFO_GLBAVIS (sg_info) != NULL) {
            args = TBmakeExprs (TBmakeId (avis),
                                TBmakeExprs (TBmakeId (SG_INFO_GLBAVIS (sg_info)), NULL));
            avis = CreatePrfOrConst (TRUE, postfix, T_int, SHmakeShape (0), F_add_SxS,
                                     args, &vardecs, &assigns);

            SG_INFO_GLBAVIS (sg_info) = avis;
            assigns = TCappendAssign (SG_INFO_GLBIDX_CAL (sg_info), assigns);
        }
        SG_INFO_GLBAVIS (sg_info) = avis;
        SG_INFO_GLBIDX_CAL (sg_info) = assigns;
    } else {
        if (SG_INFO_SHRAVIS (sg_info) != NULL) {
            args = TBmakeExprs (TBmakeId (avis),
                                TBmakeExprs (TBmakeId (SG_INFO_SHRAVIS (sg_info)), NULL));
            avis = CreatePrfOrConst (TRUE, postfix, T_int, SHmakeShape (0), F_add_SxS,
                                     args, &vardecs, &assigns);

            SG_INFO_SHRAVIS (sg_info) = avis;
            assigns = TCappendAssign (SG_INFO_SHRIDX_CAL (sg_info), assigns);
        }
        SG_INFO_SHRAVIS (sg_info) = avis;
        SG_INFO_SHRIDX_CAL (sg_info) = assigns;
    }

    FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
      = TCappendVardec (FUNDEF_VARDECS (INFO_FUNDEF (arg_info)), vardecs);

    DBUG_RETURN (sg_info);
}

static shared_global_info_t *
ComputeIndex (shared_global_info_t *sg_info, cuda_index_t *idx, info *arg_info)
{
    range_pair_t *pair = NULL;

    DBUG_ENTER ();

    switch (CUIDX_TYPE (idx)) {
    case IDX_CONSTANT:
        /* For non-zero constant apprearing in global index, we create
         * an assignment "var_const = NUM;". For shared memory
         * index, we don't need to do anything */
        if (CUIDX_COEFFICIENT (idx) != 0) {
            sg_info = ComputeIndexInternal (TRUE, "const", NULL,
                                            TBmakeNum (CUIDX_COEFFICIENT (idx)), FALSE,
                                            NULL, FALSE, sg_info, arg_info);
        }
        break;
    case IDX_EXTID:
        /* For external id apprearing in global index, we create
         * an assignment "var_extid = _mul_SxS_( id, coe);". For shared memory
         * index, we don't need to do anything */
        sg_info = ComputeIndexInternal (TRUE, "extid_glb", TBmakeId (CUIDX_ID (idx)),
                                        TBmakeNum (CUIDX_COEFFICIENT (idx)), FALSE, NULL,
                                        TRUE, sg_info, arg_info);

        sg_info = ComputeIndexInternal (FALSE, "extid_shr", NULL, TBmakeNum (0), FALSE,
                                        NULL, FALSE, sg_info, arg_info);
        break;
    case IDX_THREADIDX_X:
        /* Assignments for global memory index calculation */
        sg_info = ComputeIndexInternal (TRUE, "tx_glb", TBmakeId (CUIDX_ID (idx)),
                                        TBmakeNum (CUIDX_COEFFICIENT (idx)), TRUE,
                                        TBmakeId (CIS_TX (INFO_CIS (arg_info))), TRUE,
                                        sg_info, arg_info);

        /* Assignments for shared memory index calculation */
        sg_info = ComputeIndexInternal (FALSE, "tx_shr",
                                        TBmakeId (CIS_TX (INFO_CIS (arg_info))),
                                        TBmakeNum (CUIDX_COEFFICIENT (idx)), FALSE, NULL,
                                        TRUE, sg_info, arg_info);

        break;
    case IDX_THREADIDX_Y:
        /* Assignments for global memory index calculation */
        sg_info = ComputeIndexInternal (TRUE, "ty_glb", TBmakeId (CUIDX_ID (idx)),
                                        TBmakeNum (CUIDX_COEFFICIENT (idx)), TRUE,
                                        TBmakeId (CIS_TY (INFO_CIS (arg_info))), TRUE,
                                        sg_info, arg_info);

        /* Assignments for shared memory index calculation */
        sg_info = ComputeIndexInternal (FALSE, "ty_shr",
                                        TBmakeId (CIS_TY (INFO_CIS (arg_info))),
                                        TBmakeNum (CUIDX_COEFFICIENT (idx)), FALSE, NULL,
                                        TRUE, sg_info, arg_info);

        break;
    case IDX_LOOPIDX:
        pair = GetNthRangePair (CUIDX_LOOPLEVEL (idx) - INFO_CUWLDIM (arg_info));
        DBUG_ASSERT (pair != NULL, "Range pair is NULL!");
        RP_NEXT (pair) = SG_INFO_RANGE_PAIRS (sg_info);
        SG_INFO_RANGE_PAIRS (sg_info) = pair;

        if (CUIDX_COEFFICIENT (idx) > 0) {
            /* Assignments for global memory index calculation */
            sg_info
              = ComputeIndexInternal (TRUE, "loop_glb",
                                      TBmakeId (IDS_AVIS (RANGE_INDEX (RP_OUTER (pair)))),
                                      TBmakeNum (CUIDX_COEFFICIENT (idx)), FALSE, NULL,
                                      TRUE, sg_info, arg_info);

            /* Assignments for shared memory index calculation */
            sg_info
              = ComputeIndexInternal (FALSE, "loop_shr",
                                      TBmakeId (IDS_AVIS (RANGE_INDEX (RP_INNER (pair)))),
                                      TBmakeNum (CUIDX_COEFFICIENT (idx)), FALSE, NULL,
                                      TRUE, sg_info, arg_info);
        } else {
            /* Assignments for global memory index calculation */
            sg_info
              = ComputeIndexInternal (TRUE, "loop_glb_neg",
                                      TBmakeId (IDS_AVIS (RANGE_INDEX (RP_OUTER (pair)))),
                                      TBmakeNum (CUIDX_COEFFICIENT (idx)), TRUE,
                                      TBmakeNum (
                                        -NUM_VAL (RANGE_UPPERBOUND (RP_INNER (pair)))),
                                      TRUE, sg_info, arg_info);

            /* Assignments for shared memory index calculation */
            sg_info
              = ComputeIndexInternal (FALSE, "loop_shr_neg",
                                      TBmakeId (IDS_AVIS (RANGE_INDEX (RP_INNER (pair)))),
                                      TBmakeNum (CUIDX_COEFFICIENT (idx)), TRUE,
                                      TBmakeNum (
                                        NUM_VAL (RANGE_UPPERBOUND (RP_INNER (pair)))),
                                      TRUE, sg_info, arg_info);
        }
        break;
    default:
        DBUG_UNREACHABLE ("Unknown index type found!");
        break;
    }

    DBUG_RETURN (sg_info);
}

static node *
CreateCudaIndexInitCode (node *part, info *arg_info)
{
    size_t dim;
    cuidx_set_t *cis;
    node *assigns = NULL, *vardecs = NULL;

    DBUG_ENTER ();

    cis = (cuidx_set_t *)MEMmalloc (sizeof (cuidx_set_t));
    dim = TCcountIds (PART_IDS (part));

    CIS_TX (cis) = CreatePrfOrConst (TRUE, "tx", T_int, SHmakeShape (0),
                                     F_cuda_threadIdx_x, NULL, &vardecs, &assigns);

    CIS_BX (cis) = CreatePrfOrConst (TRUE, "b_dim_x", T_int, SHmakeShape (0),
                                     F_cuda_blockDim_x, NULL, &vardecs, &assigns);

    if (dim == 2) {
        CIS_TY (cis) = CreatePrfOrConst (TRUE, "ty", T_int, SHmakeShape (0),
                                         F_cuda_threadIdx_y, NULL, &vardecs, &assigns);

        CIS_BY (cis) = CreatePrfOrConst (TRUE, "b_dim_y", T_int, SHmakeShape (0),
                                         F_cuda_blockDim_y, NULL, &vardecs, &assigns);
    }

    FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
      = TCappendVardec (FUNDEF_VARDECS (INFO_FUNDEF (arg_info)), vardecs);

    INFO_CIS (arg_info) = cis;

    DBUG_RETURN (assigns);
}

static range_pair_t *
GetInnermostRangePair (shared_global_info_t *sg_info)
{
    range_pair_t *pairs, *innermost = NULL;
    ssize_t level = -1;

    DBUG_ENTER ();

    while (sg_info != NULL) {
        pairs = SG_INFO_RANGE_PAIRS (sg_info);

        while (pairs != NULL) {
            if ((ssize_t)RP_OUTERLEVEL (pairs) > level) {
                innermost = pairs;
                level = (ssize_t)RP_OUTERLEVEL (pairs);
            }
            pairs = RP_NEXT (pairs);
        }
        sg_info = SG_INFO_NEXT (sg_info);
    }

    DBUG_RETURN (innermost);
}

static void
InsertGlobal2Shared (shared_global_info_t *sg_info, cuda_access_info_t *access_info,
                     info *arg_info)
{
    node *array_elems;
    node *shr_args, *glb_args, *shr_args_tmp, *glb_args_tmp, *args = NULL;
    node *vardecs = NULL, *avis;
    node *assigns = NULL;
    node *shr_idx, *glb_idx, *val;
    simpletype sty;
    range_pair_t *innermost;
    node *in_shared_array = NULL, *out_shared_array = NULL;
    node *cond = NULL, *predicate = NULL, *iterator = NULL, *loop_bound = NULL;
    node *tb_shape_elems = NULL;

    DBUG_ENTER ();

    array_elems = ARRAY_AELEMS (CUAI_SHARRAYSHP_LOG (access_info));
    tb_shape_elems = ARRAY_AELEMS (PART_THREADBLOCKSHAPE (INFO_CUWLPART (arg_info)));

    if (CUAI_DIM (access_info) == 1) {      /* If the accessed array is 1D */
        if (INFO_CUWLDIM (arg_info) == 1) { /* If the CUDA withloop is 1D */
            int x, tb_x;

            tb_x = NUM_VAL (EXPRS_EXPR1 (tb_shape_elems));

            in_shared_array = CUAI_SHARRAY (access_info);

            assigns = SG_INFO_GLBIDX_CAL (sg_info);

            for (x = 0; x < NUM_VAL (EXPRS_EXPR1 (array_elems)); x += tb_x) {
                /* Argument list for F_idxs2offset to compute the actual offsets for
                 * both shared and global memory accesses. Here we start with the
                 * respective shape first */
                shr_args
                  = TBmakeExprs (DUPdoDupNode (CUAI_SHARRAYSHP_PHY (access_info)), NULL);
                glb_args = TBmakeExprs (DUPdoDupNode (CUAI_ARRAYSHP (access_info)), NULL);

                if (!CUAI_ISCONSTANT (access_info, 0)) {
                    /* Create Ashr[tx+0], Ashr[tx+1*blockDim.x], Ashr[tx+2*blockDim.x]
                     * ...*/
                    args = TBmakeExprs (TBmakeId (CIS_TX (INFO_CIS (arg_info))),
                                        TBmakeExprs (TBmakeNum (x), NULL));
                    avis = CreatePrfOrConst (TRUE, "tx_shr_g2s", T_int, SHmakeShape (0),
                                             F_add_SxS, args, &vardecs, &assigns);

                    shr_args
                      = TCcombineExprs (shr_args, TBmakeExprs (TBmakeId (avis), NULL));

                    /* Create Aglb[base+tx+0], Aglb[base+tx+1*blockDim.x],
                     * Aglb[base+tx+2*blockDim.x] ...*/
                    args = TBmakeExprs (TBmakeId (CIS_TX (INFO_CIS (arg_info))),
                                        TBmakeExprs (TBmakeId (SG_INFO_GLBAVIS (sg_info)),
                                                     NULL));
                    avis = CreatePrfOrConst (TRUE, "tx_glb_g2s", T_int, SHmakeShape (0),
                                             F_add_SxS, args, &vardecs, &assigns);

                    args
                      = TBmakeExprs (TBmakeId (avis), TBmakeExprs (TBmakeNum (x), NULL));
                    avis = CreatePrfOrConst (TRUE, "tx_glb_g2s", T_int, SHmakeShape (0),
                                             F_add_SxS, args, &vardecs, &assigns);

                    glb_args
                      = TCcombineExprs (glb_args, TBmakeExprs (TBmakeId (avis), NULL));
                } else {
                    /* What to do if the access has CONSTANT subscript */
                }

                /* Create global2shared assignment i.e. idx_modarray_AxSxS */
                glb_idx = CreatePrfOrConst (TRUE, "idx_glb_g2s", T_int, SHmakeShape (0),
                                            F_idxs2offset, glb_args, &vardecs, &assigns);

                args
                  = TBmakeExprs (TBmakeId (glb_idx),
                                 TBmakeExprs (TBmakeId (CUAI_ARRAY (access_info)), NULL));

                sty = CUd2hSimpleTypeConversion (
                  TYgetSimpleType (TYgetScalar (AVIS_TYPE (CUAI_ARRAY (access_info)))));

                val = CreatePrfOrConst (TRUE, "val_glb_g2s", sty, SHmakeShape (0),
                                        F_idx_sel, args, &vardecs, &assigns);

                shr_idx = CreatePrfOrConst (TRUE, "idx_shr_g2s", T_int, SHmakeShape (0),
                                            F_idxs2offset, shr_args, &vardecs, &assigns);

                args = TBmakeExprs (TBmakeId (CUAI_SHARRAY (access_info)),
                                    TBmakeExprs (TBmakeId (shr_idx),
                                                 TBmakeExprs (TBmakeId (val), NULL)));
                CUAI_SHARRAY (access_info)
                  = CreatePrfOrConst (TRUE, "shmem",
                                      TYgetSimpleType (TYgetScalar (
                                        AVIS_TYPE (CUAI_SHARRAY (access_info)))),
                                      SHarray2Shape (CUAI_SHARRAYSHP_PHY (access_info)),
                                      F_idx_modarray_AxSxS, args, &vardecs, &assigns);

                if (tb_x > NUM_VAL (EXPRS_EXPR1 (array_elems))) {
                    args = TBmakeExprs (TBmakeId (CIS_TX (INFO_CIS (arg_info))),
                                        TBmakeExprs (TBmakeNum (NUM_VAL (
                                                       EXPRS_EXPR1 (array_elems))),
                                                     NULL));

                    cond = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0),
                                             F_lt_SxS, args, &vardecs, &predicate);
                }
            }
            FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
              = TCappendVardec (FUNDEF_VARDECS (INFO_FUNDEF (arg_info)), vardecs);
        } else if (INFO_CUWLDIM (arg_info) == 2) {
            /* TODO */
        } else {
            /* Do nothing */
        }

    } else if (CUAI_DIM (access_info) == 2) { /* If the accessed array is 2D */
        if (INFO_CUWLDIM (arg_info) == 1) {   /* If the CUDA WL is 1D */
            node *loop_assigns = NULL;        /* Assigns of the loop body */
            int tb_x, arr_shp_y, arr_shp_x;

            /* tb_x is the size of the thread block along the X
             * dimension. arr_shp_y and arr_shp_x represent the
             * logical shape of the shared array */
            tb_x = NUM_VAL (EXPRS_EXPR1 (tb_shape_elems));
            arr_shp_y = NUM_VAL (EXPRS_EXPR1 (array_elems));
            arr_shp_x = NUM_VAL (EXPRS_EXPR2 (array_elems));

            in_shared_array = CUAI_SHARRAY (access_info);

            /* !!! For the time being, we assume that size of
             * X dimension of the thread block is always larger
             * than the X dimension of the shared memory. Howver,
             * this is no always true and need to be generalised
             * in the future !!!*/
            DBUG_ASSERT (arr_shp_x <= tb_x,
                         "Size of X dimension is lager than thread block size!");

            /* Since the array is 2D, we know there are only two assignment
             * chains computing the base subscript expressions */
            assigns = TCappendAssign (SG_INFO_GLBIDX_CAL (sg_info),
                                      SG_INFO_GLBIDX_CAL (SG_INFO_NEXT (sg_info)));

            /* Because the shr_args will be used in F_idxs2offset to compute
             * the actual address within physical memory, we need to use the
             * physical shape of the shared memory */
            shr_args
              = TBmakeExprs (DUPdoDupNode (CUAI_SHARRAYSHP_PHY (access_info)), NULL);
            glb_args = TBmakeExprs (DUPdoDupNode (CUAI_ARRAYSHP (access_info)), NULL);

            if (tb_x % arr_shp_x == 0 && arr_shp_y % ((int)(tb_x / arr_shp_x)) == 0) {
                printf ("hahah, in then branh!\n");

                int load_block_size = arr_shp_y / (tb_x / arr_shp_x);

                args = TBmakeExprs (TBmakeId (CIS_TX (INFO_CIS (arg_info))),
                                    TBmakeExprs (TBmakeNum (arr_shp_x), NULL));
                avis = CreatePrfOrConst (TRUE, "ld_blk_idx", T_int, SHmakeShape (0),
                                         F_div_SxS, args, &vardecs, &assigns);

                args = TBmakeExprs (TBmakeId (avis),
                                    TBmakeExprs (TBmakeNum (load_block_size), NULL));
                iterator = CreatePrfOrConst (TRUE, "iterator", T_int, SHmakeShape (0),
                                             F_mul_SxS, args, &vardecs, &assigns);

                args = TBmakeExprs (TBmakeId (iterator),
                                    TBmakeExprs (TBmakeNum (load_block_size), NULL));
                loop_bound = CreatePrfOrConst (TRUE, "loop_bound", T_int, SHmakeShape (0),
                                               F_add_SxS, args, &vardecs, &assigns);

                /* Y dimension shared memory */
                args
                  = TBmakeExprs (TBmakeId (iterator), TBmakeExprs (TBmakeNum (0), NULL));
                avis = CreatePrfOrConst (TRUE, "ty_shr_g2s", T_int, SHmakeShape (0),
                                         F_add_SxS, args, &vardecs, &loop_assigns);
                shr_args = TCcombineExprs (shr_args, TBmakeExprs (TBmakeId (avis), NULL));

                /* X dimension shared memory */
                args = TBmakeExprs (TBmakeId (CIS_TX (INFO_CIS (arg_info))),
                                    TBmakeExprs (TBmakeNum (arr_shp_x), NULL));
                avis = CreatePrfOrConst (TRUE, "tx_shr_g2s", T_int, SHmakeShape (0),
                                         F_mod_SxS, args, &vardecs, &loop_assigns);
                shr_args = TCcombineExprs (shr_args, TBmakeExprs (TBmakeId (avis), NULL));

                /* Y dimension global memory */
                args = TBmakeExprs (TBmakeId (iterator),
                                    TBmakeExprs (TBmakeId (SG_INFO_GLBAVIS (sg_info)),
                                                 NULL));
                avis = CreatePrfOrConst (TRUE, "ty_glb_g2s", T_int, SHmakeShape (0),
                                         F_add_SxS, args, &vardecs, &loop_assigns);

                glb_args = TCcombineExprs (glb_args, TBmakeExprs (TBmakeId (avis), NULL));

                /* X dimension global memory */
                args = TBmakeExprs (TBmakeId (CIS_TX (INFO_CIS (arg_info))),
                                    TBmakeExprs (TBmakeNum (arr_shp_x), NULL));
                avis = CreatePrfOrConst (TRUE, "tx_glb_g2s", T_int, SHmakeShape (0),
                                         F_mod_SxS, args, &vardecs, &loop_assigns);

                args
                  = TBmakeExprs (TBmakeId (avis), TBmakeExprs (TBmakeId (SG_INFO_GLBAVIS (
                                                                 SG_INFO_NEXT (sg_info))),
                                                               NULL));
                avis = CreatePrfOrConst (TRUE, "tx_glb_g2s", T_int, SHmakeShape (0),
                                         F_add_SxS, args, &vardecs, &loop_assigns);

                glb_args = TCcombineExprs (glb_args, TBmakeExprs (TBmakeId (avis), NULL));
            } else {
                printf ("hahah, in else branh!\n");
                /* Initialise loop iterator to 0 and set loop bound to
                 * size of the Y dimension of the share memory array */
                iterator
                  = CreatePrfOrConst (FALSE, "iterator", T_int, SHmakeShape (0),
                                      F_add_SxS, TBmakeNum (0), &vardecs, &assigns);
                /*
                        loop_bound = TBmakeNum( arr_shp_y);
                */
                loop_bound = CreatePrfOrConst (FALSE, "loop_bound", T_int,
                                               SHmakeShape (0), F_add_SxS,
                                               TBmakeNum (arr_shp_y), &vardecs, &assigns);

                /* Y dimension shared memory */
                args
                  = TBmakeExprs (TBmakeId (iterator), TBmakeExprs (TBmakeNum (0), NULL));
                avis = CreatePrfOrConst (TRUE, "ty_shr_g2s", T_int, SHmakeShape (0),
                                         F_add_SxS, args, &vardecs, &loop_assigns);
                shr_args = TCcombineExprs (shr_args, TBmakeExprs (TBmakeId (avis), NULL));

                /* X dimension shared memory */
                args = TBmakeExprs (TBmakeId (CIS_TX (INFO_CIS (arg_info))),
                                    TBmakeExprs (TBmakeNum (0), NULL));
                avis = CreatePrfOrConst (TRUE, "tx_shr_g2s", T_int, SHmakeShape (0),
                                         F_add_SxS, args, &vardecs, &loop_assigns);
                shr_args = TCcombineExprs (shr_args, TBmakeExprs (TBmakeId (avis), NULL));

                /* Y dimension global memory */
                args = TBmakeExprs (TBmakeId (iterator),
                                    TBmakeExprs (TBmakeId (SG_INFO_GLBAVIS (sg_info)),
                                                 NULL));
                avis = CreatePrfOrConst (TRUE, "ty_glb_g2s", T_int, SHmakeShape (0),
                                         F_add_SxS, args, &vardecs, &loop_assigns);

                glb_args = TCcombineExprs (glb_args, TBmakeExprs (TBmakeId (avis), NULL));

                /* X dimension global memory */
                args = TBmakeExprs (TBmakeId (CIS_TX (INFO_CIS (arg_info))),
                                    TBmakeExprs (TBmakeId (SG_INFO_GLBAVIS (
                                                   SG_INFO_NEXT (sg_info))),
                                                 NULL));
                avis = CreatePrfOrConst (TRUE, "tx_glb_g2s", T_int, SHmakeShape (0),
                                         F_add_SxS, args, &vardecs, &loop_assigns);

                glb_args = TCcombineExprs (glb_args, TBmakeExprs (TBmakeId (avis), NULL));

                /* Create a conditional so that only threads with threadIdx.x
                 * less than the size of the X dimension of the shared memory
                 * array will participate in loading data */
                args = TBmakeExprs (TBmakeId (CIS_TX (INFO_CIS (arg_info))),
                                    TBmakeExprs (TBmakeNum (arr_shp_x), NULL));

                cond = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0), F_lt_SxS,
                                         args, &vardecs, &predicate);
            }

            /* Create global2shared loading instruction i.e. idx_modarray_AxSxS */
            glb_idx = CreatePrfOrConst (TRUE, "idx_glb_g2s", T_int, SHmakeShape (0),
                                        F_idxs2offset, glb_args, &vardecs, &loop_assigns);

            args = TBmakeExprs (TBmakeId (glb_idx),
                                TBmakeExprs (TBmakeId (CUAI_ARRAY (access_info)), NULL));

            sty = CUd2hSimpleTypeConversion (
              TYgetSimpleType (TYgetScalar (AVIS_TYPE (CUAI_ARRAY (access_info)))));

            val = CreatePrfOrConst (TRUE, "val_glb_g2s", sty, SHmakeShape (0), F_idx_sel,
                                    args, &vardecs, &loop_assigns);

            shr_idx = CreatePrfOrConst (TRUE, "idx_shr_g2s", T_int, SHmakeShape (0),
                                        F_idxs2offset, shr_args, &vardecs, &loop_assigns);

            args = TBmakeExprs (TBmakeId (CUAI_SHARRAY (access_info)),
                                TBmakeExprs (TBmakeId (shr_idx),
                                             TBmakeExprs (TBmakeId (val), NULL)));
            CUAI_SHARRAY (access_info)
              = CreatePrfOrConst (TRUE, "shmem",
                                  TYgetSimpleType (
                                    TYgetScalar (AVIS_TYPE (CUAI_SHARRAY (access_info)))),
                                  SHarray2Shape (CUAI_SHARRAYSHP_PHY (access_info)),
                                  F_idx_modarray_AxSxS, args, &vardecs, &loop_assigns);

            /* We have the add the new vardecs to the fundef here
             * because  function CLACFdoCreateLacFun need to know
             * all the vardecs to work properly. */
            FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
              = TCappendVardec (FUNDEF_VARDECS (INFO_FUNDEF (arg_info)), vardecs);

            /* After this, loop_assigns is the function application of a loop function */
            /*
                  loop_assigns = CLACFdoCreateLacFun( FALSE, INFO_FUNDEF( arg_info),
               loop_assigns, NULL, iterator, loop_bound, in_shared_array, CUAI_SHARRAY(
               access_info), &INFO_LACFUNS( arg_info));
            */
            loop_assigns
              = CLFdoCreateLoopFun (INFO_FUNDEF (arg_info), loop_assigns, iterator,
                                    loop_bound, in_shared_array,
                                    CUAI_SHARRAY (access_info), &INFO_LACFUNS (arg_info));

            /* This is the final assignment chain used to create the conditional function
             */
            assigns = TCappendAssign (assigns, loop_assigns);
        } else if (INFO_CUWLDIM (arg_info) == 2) { /* If the CUDA WL is 2D */
            int x, y, tb_x, tb_y;

            tb_y = NUM_VAL (EXPRS_EXPR1 (tb_shape_elems));
            tb_x = NUM_VAL (EXPRS_EXPR2 (tb_shape_elems));

            in_shared_array = CUAI_SHARRAY (access_info);

            /* Since the array is 2D, we know there are only two assignment
             * chains computing the base subscript expressions*/
            assigns = TCappendAssign (SG_INFO_GLBIDX_CAL (sg_info),
                                      SG_INFO_GLBIDX_CAL (SG_INFO_NEXT (sg_info)));

            for (y = 0; y < NUM_VAL (EXPRS_EXPR1 (array_elems)); y += tb_y) {
                /* Argument list for F_idxs2offset to compute the actual offsets for
                 * both shared and global memory accesses. Here we start with the
                 * respective shape first */
                shr_args
                  = TBmakeExprs (DUPdoDupNode (CUAI_SHARRAYSHP_PHY (access_info)), NULL);
                glb_args = TBmakeExprs (DUPdoDupNode (CUAI_ARRAYSHP (access_info)), NULL);

                if (!CUAI_ISCONSTANT (access_info, 0)) {
                    /* Create Ashr[ty+0][...], Ashr[ty+16][...], Ashr[ty+32][...] ...*/
                    args = TBmakeExprs (TBmakeId (CIS_TY (INFO_CIS (arg_info))),
                                        TBmakeExprs (TBmakeNum (y), NULL));
                    avis = CreatePrfOrConst (TRUE, "ty_shr_g2s", T_int, SHmakeShape (0),
                                             F_add_SxS, args, &vardecs, &assigns);

                    shr_args
                      = TCcombineExprs (shr_args, TBmakeExprs (TBmakeId (avis), NULL));

                    /* Create Aglb[base+ty+0][...], Aglb[base+ty+16][...],
                     * Aglb[base+ty+32][...] ...*/
                    args = TBmakeExprs (TBmakeId (CIS_TY (INFO_CIS (arg_info))),
                                        TBmakeExprs (TBmakeId (SG_INFO_GLBAVIS (sg_info)),
                                                     NULL));
                    avis = CreatePrfOrConst (TRUE, "ty_glb_g2s", T_int, SHmakeShape (0),
                                             F_add_SxS, args, &vardecs, &assigns);

                    args
                      = TBmakeExprs (TBmakeId (avis), TBmakeExprs (TBmakeNum (y), NULL));
                    avis = CreatePrfOrConst (TRUE, "ty_glb_g2s", T_int, SHmakeShape (0),
                                             F_add_SxS, args, &vardecs, &assigns);

                    glb_args
                      = TCcombineExprs (glb_args, TBmakeExprs (TBmakeId (avis), NULL));
                } else {
                    avis
                      = CreatePrfOrConst (FALSE, "ty_shr_g2s", T_int, SHmakeShape (0),
                                          F_add_SxS, TBmakeNum (0), &vardecs, &assigns);

                    shr_args
                      = TCcombineExprs (shr_args, TBmakeExprs (TBmakeId (avis), NULL));

                    glb_args
                      = TCcombineExprs (glb_args,
                                        TBmakeExprs (TBmakeId (SG_INFO_GLBAVIS (sg_info)),
                                                     NULL));

                    args = TBmakeExprs (TBmakeId (CIS_TY (INFO_CIS (arg_info))),
                                        TBmakeExprs (TBmakeNum (0), NULL));

                    cond = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0),
                                             F_eq_SxS, args, &vardecs, &predicate);
                }

                for (x = 0; x < NUM_VAL (EXPRS_EXPR2 (array_elems)); x += tb_x) {
                    shr_args_tmp = DUPdoDupTree (shr_args);
                    glb_args_tmp = DUPdoDupTree (glb_args);

                    if (!CUAI_ISCONSTANT (access_info, 1)) {
                        /* Create Ashr[...][tx+0], Ashr[...][tx+32], Ashr[...][tx+64]
                         * ...*/
                        args = TBmakeExprs (TBmakeId (CIS_TX (INFO_CIS (arg_info))),
                                            TBmakeExprs (TBmakeNum (x), NULL));
                        avis
                          = CreatePrfOrConst (TRUE, "tx_shr_g2s", T_int, SHmakeShape (0),
                                              F_add_SxS, args, &vardecs, &assigns);

                        shr_args_tmp
                          = TCcombineExprs (shr_args_tmp,
                                            TBmakeExprs (TBmakeId (avis), NULL));

                        /* Create Aglb[...][base+tx+0], Aglb[...][base+tx+32],
                         * Aglb[][base+tx+64] ...*/
                        args = TBmakeExprs (TBmakeId (CIS_TX (INFO_CIS (arg_info))),
                                            TBmakeExprs (TBmakeId (SG_INFO_GLBAVIS (
                                                           SG_INFO_NEXT (sg_info))),
                                                         NULL));
                        avis
                          = CreatePrfOrConst (TRUE, "tx_glb_g2s", T_int, SHmakeShape (0),
                                              F_add_SxS, args, &vardecs, &assigns);

                        args = TBmakeExprs (TBmakeId (avis),
                                            TBmakeExprs (TBmakeNum (x), NULL));
                        avis
                          = CreatePrfOrConst (TRUE, "tx_glb_g2s", T_int, SHmakeShape (0),
                                              F_add_SxS, args, &vardecs, &assigns);

                        glb_args_tmp
                          = TCcombineExprs (glb_args_tmp,
                                            TBmakeExprs (TBmakeId (avis), NULL));
                    } else {
                        avis = CreatePrfOrConst (FALSE, "tx_shr_g2s", T_int,
                                                 SHmakeShape (0), F_add_SxS,
                                                 TBmakeNum (0), &vardecs, &assigns);

                        shr_args_tmp
                          = TCcombineExprs (shr_args_tmp,
                                            TBmakeExprs (TBmakeId (avis), NULL));

                        glb_args_tmp
                          = TCcombineExprs (glb_args_tmp,
                                            TBmakeExprs (TBmakeId (SG_INFO_GLBAVIS (
                                                           SG_INFO_NEXT (sg_info))),
                                                         NULL));

                        args = TBmakeExprs (TBmakeId (CIS_TX (INFO_CIS (arg_info))),
                                            TBmakeExprs (TBmakeNum (0), NULL));

                        cond = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0),
                                                 F_eq_SxS, args, &vardecs, &predicate);
                    }

                    /* Create global2shared assignment i.e. idx_modarray_AxSxS */
                    glb_idx = CreatePrfOrConst (TRUE, "idx_glb_g2s", T_int,
                                                SHmakeShape (0), F_idxs2offset,
                                                glb_args_tmp, &vardecs, &assigns);

                    args = TBmakeExprs (TBmakeId (glb_idx),
                                        TBmakeExprs (TBmakeId (CUAI_ARRAY (access_info)),
                                                     NULL));

                    sty = CUd2hSimpleTypeConversion (TYgetSimpleType (
                      TYgetScalar (AVIS_TYPE (CUAI_ARRAY (access_info)))));

                    val = CreatePrfOrConst (TRUE, "val_glb_g2s", sty, SHmakeShape (0),
                                            F_idx_sel, args, &vardecs, &assigns);

                    shr_idx = CreatePrfOrConst (TRUE, "idx_shr_g2s", T_int,
                                                SHmakeShape (0), F_idxs2offset,
                                                shr_args_tmp, &vardecs, &assigns);

                    args = TBmakeExprs (TBmakeId (CUAI_SHARRAY (access_info)),
                                        TBmakeExprs (TBmakeId (shr_idx),
                                                     TBmakeExprs (TBmakeId (val), NULL)));
                    CUAI_SHARRAY (access_info)
                      = CreatePrfOrConst (TRUE, "shmem",
                                          TYgetSimpleType (TYgetScalar (
                                            AVIS_TYPE (CUAI_SHARRAY (access_info)))),
                                          SHarray2Shape (
                                            CUAI_SHARRAYSHP_PHY (access_info)),
                                          F_idx_modarray_AxSxS, args, &vardecs, &assigns);
                }
                shr_args = FREEdoFreeTree (shr_args);
                glb_args = FREEdoFreeTree (glb_args);
            }
            FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
              = TCappendVardec (FUNDEF_VARDECS (INFO_FUNDEF (arg_info)), vardecs);
        } else {
            /* Do nothing */
        }
    } else {
        DBUG_UNREACHABLE ("Reuse array with unsupported dimension!");
    }

    vardecs = NULL;

    if (predicate != NULL) {
        out_shared_array = CUAI_SHARRAY (access_info);

        /* After this, assigns is the function application of a conditional function */
        /*
            assigns = CLACFdoCreateLacFun( TRUE, INFO_FUNDEF( arg_info), assigns, cond,
           NULL, NULL, in_shared_array, out_shared_array, &INFO_LACFUNS( arg_info));
        */
        assigns = CCFdoCreateCondFun (INFO_FUNDEF (arg_info), assigns, NULL,
                                      cond, /* Null 'else assigns' */
                                      in_shared_array, out_shared_array, NULL,
                                      &INFO_LACFUNS (arg_info));

        assigns = TCappendAssign (predicate, assigns);
    }

    /* Create syncthreads after loading data from global memory to shared memory */
    args = TBmakeExprs (TBmakeId (CUAI_SHARRAY (access_info)), NULL);

    CUAI_SHARRAY (access_info)
      = CreatePrfOrConst (TRUE, "shmem",
                          TYgetSimpleType (
                            TYgetScalar (AVIS_TYPE (CUAI_SHARRAY (access_info)))),
                          SHarray2Shape (CUAI_SHARRAYSHP_PHY (access_info)),
                          F_syncthreads, args, &vardecs, &assigns);

    FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
      = TCappendVardec (FUNDEF_VARDECS (INFO_FUNDEF (arg_info)), vardecs);

    innermost = GetInnermostRangePair (sg_info);

    if (innermost != NULL) {
        RANGE_G2SINSTRS (RP_OUTER (innermost))
          = TCappendAssign (assigns, RANGE_G2SINSTRS (RP_OUTER (innermost)));

        /* The inner loop of the blocked pair will be unrolled by the CUDA
         * compiler. Note that this might increase the registers used by
         * each thread and thus potentially decrease thread level parallelism,
         * so it's a better idea to make this a compiler option which
         * can be switched on and off for experimental purpose. Also, if this option
         * is switched on, we should aslo set the maxrregcount option when compiling
         * with nvcc to pervent excessive register usage. This max register count
         * should be set to 20. */
        RANGE_NEEDCUDAUNROLL (RP_INNER (innermost)) = TRUE;
    } else {
        INFO_G2S_ASSIGNS (arg_info)
          = TCappendAssign (assigns, INFO_G2S_ASSIGNS (arg_info));
    }

    DBUG_RETURN ();
}

/* This function concatenate all subscript computation assignments
 * for a shared memory access in sg_info all together and add to the
 * end of the chain a F_idxs2offset to compute the actual offset to
 * access the shared memory. This will be used in the F_idx_sel
 * assignment to select the element from the shared memory. This
 * chain of assignments will be stored in INFO_PREASSIGNS and
 * preappended to the F_idx_sel assignment */
static node *
ComputeSharedMemoryOffset (shared_global_info_t *sg_info, cuda_access_info_t *access_info,
                           info *arg_info)
{
    node *idx_avis, *new_ass, *args = NULL, *assigns = NULL;

    DBUG_ENTER ();

    args = TBmakeExprs (DUPdoDupNode (CUAI_SHARRAYSHP_PHY (access_info)), NULL);

    while (sg_info != NULL) {
        args = TCcombineExprs (args,
                               TBmakeExprs (TBmakeId (SG_INFO_SHRAVIS (sg_info)), NULL));
        assigns = TCappendAssign (assigns, SG_INFO_SHRIDX_CAL (sg_info));
        sg_info = SG_INFO_NEXT (sg_info);
    }

    idx_avis = TBmakeAvis (TRAVtmpVarName ("idx_shr"),
                           TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));

    new_ass = TBmakeAssign (TBmakeLet (TBmakeIds (idx_avis, NULL),
                                       TBmakePrf (F_idxs2offset, args)),
                            NULL);

    INFO_PREASSIGNS (arg_info) = TCappendAssign (assigns, new_ass);

    FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
      = TCappendVardec (FUNDEF_VARDECS (INFO_FUNDEF (arg_info)),
                        TBmakeVardec (idx_avis, NULL));

    FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
      = TCappendVardec (FUNDEF_VARDECS (INFO_FUNDEF (arg_info)),
                        TBmakeVardec (CUAI_SHARRAY (access_info), NULL));

    DBUG_RETURN (idx_avis);
}

/** <!--********************************************************************-->
 *
 * @fn node *ATravAssign(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
static node *
ATravAssign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (ASSIGN_ACCESS_INFO (arg_node) != NULL) {
        ASSIGN_ACCESS_INFO (arg_node)
          = TBfreeCudaAccessInfo (ASSIGN_ACCESS_INFO (arg_node));
        ASSIGN_ACCESS_INFO (arg_node) = NULL;
    }

    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUDRdoCudaDaraReuse( node *syntax_tree)
 *
 * @brief
 *
 *****************************************************************************/
node *
CUDRdoCudaDataReuse (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();
    TRAVpush (TR_cudr);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUDRmodule( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CUDRmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    if (INFO_LACFUNS (arg_info) != NULL) {
        MODULE_FUNS (arg_node)
          = TCappendFundef (MODULE_FUNS (arg_node), INFO_LACFUNS (arg_info));
        INFO_LACFUNS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUDRfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CUDRfundef (node *arg_node, info *arg_info)
{
    node *old_fundef;

    DBUG_ENTER ();

    if (INFO_FROMAP (arg_info)) {
        old_fundef = INFO_FUNDEF (arg_info);
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = old_fundef;
    } else {
        if (FUNDEF_ISLACFUN (arg_node)) {
            FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
        } else {
            INFO_FUNDEF (arg_info) = arg_node;
            FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
            INFO_FUNDEF (arg_info) = NULL;
            FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUDRap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CUDRap (node *arg_node, info *arg_info)
{
    node *fundef;
    bool old_fromap;

    DBUG_ENTER ();

    fundef = AP_FUNDEF (arg_node);

    if (fundef != NULL && FUNDEF_ISLACFUN (fundef) &&
        /* INFO_LEVEL( arg_info) > 0 && */
        fundef != INFO_FUNDEF (arg_info)) {
        old_fromap = INFO_FROMAP (arg_info);
        INFO_FROMAP (arg_info) = TRUE;
        AP_FUNDEF (arg_node) = TRAVopt (AP_FUNDEF (arg_node), arg_info);
        INFO_FROMAP (arg_info) = old_fromap;
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUDRassign( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CUDRassign (node *arg_node, info *arg_info)
{
    node *old_lastassign, *tmp;

    DBUG_ENTER ();

    old_lastassign = INFO_LASTASSIGN (arg_info);
    INFO_LASTASSIGN (arg_info) = arg_node;

    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);

    tmp = arg_node;
    if (INFO_PREASSIGNS (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    INFO_LASTASSIGN (arg_info) = old_lastassign;

    ASSIGN_NEXT (tmp) = TRAVopt (ASSIGN_NEXT (tmp), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUDRwith( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CUDRwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (WITH_CUDARIZABLE (arg_node)) {
        INFO_CUWLDIM (arg_info) = TCcountIds (WITH_IDS (arg_node));
        WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);

        /************ Anonymous Traversal ************/
        anontrav_t atrav[2] = {{N_assign, &ATravAssign}, {(nodetype)0, NULL}};

        TRAVpushAnonymous (atrav, &TRAVsons);
        WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), NULL);
        TRAVpop ();
        /*********************************************/
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUDRpart( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CUDRpart (node *arg_node, info *arg_info)
{
    size_t dim;
    node *init_assigns;

    DBUG_ENTER ();

    dim = TCcountIds (PART_IDS (arg_node));

    if (BLOCK_ASSIGNS (PART_CBLOCK (arg_node)) != NULL) {
        init_assigns = CreateCudaIndexInitCode (arg_node, arg_info);

        /* Note here that we don't need to stack N_part because there will
         * not be any partitions within an N_part */
        INFO_CUWLPART (arg_info) = arg_node;
        INFO_LEVEL (arg_info) += dim;
        PART_CODE (arg_node) = TRAVopt (PART_CODE (arg_node), arg_info);
        INFO_LEVEL (arg_info) -= dim;
        INFO_CUWLPART (arg_info) = NULL;

        if (INFO_G2S_ASSIGNS (arg_info) != NULL) {
            BLOCK_ASSIGNS (PART_CBLOCK (arg_node))
              = TCappendAssign (INFO_G2S_ASSIGNS (arg_info),
                                BLOCK_ASSIGNS (PART_CBLOCK (arg_node)));
            INFO_G2S_ASSIGNS (arg_info) = NULL;
        }

        BLOCK_ASSIGNS (PART_CBLOCK (arg_node))
          = TCappendAssign (init_assigns, BLOCK_ASSIGNS (PART_CBLOCK (arg_node)));

        /* This struct is created in CreateCudaIndexInitCode and freed here */
        INFO_CIS (arg_info) = MEMfree (INFO_CIS (arg_info));
    }

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUDRwith3( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CUDRwith3 (node *arg_node, info *arg_info)
{
    node *old_with3;

    DBUG_ENTER ();

    /* Everytime we enter a top level with3, we push a range set */
    if (WITH3_ISTOPLEVEL (arg_node)) {
        INFO_RANGE_SETS (arg_info) = PushRangeSet (INFO_RANGE_SETS (arg_info));
    }

    old_with3 = INFO_WITH3 (arg_info);
    INFO_WITH3 (arg_info) = arg_node;
    WITH3_RANGES (arg_node) = TRAVopt (WITH3_RANGES (arg_node), arg_info);
    INFO_WITH3 (arg_info) = old_with3;

    /* Everytime we leave a top level with3, we pop a range set */
    if (WITH3_ISTOPLEVEL (arg_node)) {
        INFO_RANGE_SETS (arg_info) = PopRangeSet (INFO_RANGE_SETS (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUDRrange( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CUDRrange (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* We only traverse the range if it is NOT for fitting */
    if (!RANGE_ISFITTING (arg_node)) {
        INFO_RANGE_SETS (arg_info)
          = EnqRangeInfo (INFO_RANGE_SETS (arg_info), arg_node,
                          WITH3_ISTOPLEVEL (INFO_WITH3 (arg_info)));

        INFO_LEVEL (arg_info)++;

        /*
            PrintSpaces( INFO_LEVEL( arg_info));
            DBUG_PRINT( "CUDR", ("Entering range [index:%s level:%d blocked:%d]\n",
                        IDS_NAME( RANGE_INDEX( arg_node)),
                        INFO_LEVEL( arg_info), RANGE_ISBLOCKED( arg_node)));
            PrintRangeSet( INFO_RANGE_SETS( arg_info), INFO_LEVEL( arg_info));
        */

        RANGE_G2SINSTRS (arg_node) = NULL;

        RANGE_BODY (arg_node) = TRAVopt (RANGE_BODY (arg_node), arg_info);

        if (RANGE_G2SINSTRS (arg_node) != NULL) {
            BLOCK_ASSIGNS (RANGE_BODY (arg_node))
              = TCappendAssign (RANGE_G2SINSTRS (arg_node),
                                BLOCK_ASSIGNS (RANGE_BODY (arg_node)));
            RANGE_G2SINSTRS (arg_node) = NULL;
        }

        /*
            PrintSpaces( INFO_LEVEL( arg_info));
            DBUG_PRINT( "CUDR", ("Leaving range [index:%s level:%d blocked:%d]\n",
                        IDS_NAME( RANGE_INDEX( arg_node)),
                        INFO_LEVEL( arg_info), RANGE_ISBLOCKED( arg_node)));
        */

        INFO_LEVEL (arg_info)--;

        INFO_RANGE_SETS (arg_info) = DeqRangeInfo (INFO_RANGE_SETS (arg_info), arg_node);
    }

    RANGE_NEXT (arg_node) = TRAVopt (RANGE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUDRcode( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CUDRcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUDRprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CUDRprf (node *arg_node, info *arg_info)
{
    int i;
    shared_global_info_t *sg_info = NULL, *sg_info_tmp;
    node *shr_idx;
    cuda_index_t *cuidx;

    DBUG_ENTER ();

    /* If we are in cuda withloop */
    if (INFO_LEVEL (arg_info) > 0) {
        if (PRF_PRF (arg_node) == F_idx_sel) {
            cuda_access_info_t *access_info;

            node *idx = PRF_ARG1 (arg_node);
            node *arr = PRF_ARG2 (arg_node);

            DBUG_ASSERT (NODE_TYPE (idx) == N_id,
                         "Non-id node found in the first argument of idx_sel!");
            DBUG_ASSERT (NODE_TYPE (arr) == N_id,
                         "Non-id node found in the second argument of idx_sel!");

            access_info = ASSIGN_ACCESS_INFO (INFO_LASTASSIGN (arg_info));

            /* If this idx_sel accesses an array with reuse opportunity */
            if (access_info != NULL) {
                for (i = 0; i < CUAI_DIM (access_info); i++) {
                    /* Here sg_info is a chain of shared_global_info structs and
                     * sg_info_tmp is the newly created shared_global_info struct. */
                    sg_info_tmp = CreateSharedGlobalInfo (&sg_info);
                    cuidx = CUAI_INDICES (access_info, i);
                    while (cuidx != NULL) {
                        sg_info_tmp = ComputeIndex (sg_info_tmp, cuidx, arg_info);
                        cuidx = CUIDX_NEXT (cuidx);
                    }
                }

                shr_idx = ComputeSharedMemoryOffset (sg_info, access_info, arg_info);
                InsertGlobal2Shared (sg_info, access_info, arg_info);

                /*
                 * F_idx_sel( A_dev, offset_host)
                 * -->
                 * F_idx_sel( A_shmem, offset_shmem)
                 */
                ID_AVIS (idx) = shr_idx;
                ID_AVIS (arr) = CUAI_SHARRAY (access_info);

                /* Clean up */
                sg_info = FreeSharedGlobalInfo (sg_info);
            }
        }
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
