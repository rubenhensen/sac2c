
#include "cuda_data_reuse.h"

#include <stdlib.h>

#include "tree_basic.h"
#include "tree_compound.h"
#include "globals.h"
#include "memory.h"
#include "dbug.h"
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
#include "matrix.h"
#include "types.h"

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
    int blocked_ranges_count;
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
#define RS_RANGES_CNT(n) (((n->blocked_ranges_count) + (n->nonblocked_ranges_count)))
#define RS_PREV(n) (n->prev)
#define RS_NEXT(n) (n->next)

/* First set, i.e. the one at the bottom of the stack */
static range_set_t *first_range_set = NULL;
static int range_set_cnt = 0;

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

/*
 * INFO structure
 */
struct INFO {
    int level;
    node *fundef;
    node *lastassign;
    range_set_t *range_sets;
    node *with3;
    int cuwldim;
    cuidx_set_t *cis;
    node *preassigns;
};

#define INFO_LEVEL(n) (n->level)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LASTASSIGN(n) (n->lastassign)
#define INFO_RANGE_SETS(n) (n->range_sets)
#define INFO_WITH3(n) (n->with3)
#define INFO_CUWLDIM(n) (n->cuwldim)
#define INFO_CIS(n) (n->cis)
#define INFO_PREASSIGNS(n) (n->preassigns)

typedef struct RANGE_PAIR_T {
    node *outer;
    node *inner;
    int outerlevel;
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

static shared_global_info_t *
CreateSharedGlobalInfo (shared_global_info_t *prev_sg_infos)
{
    shared_global_info_t *res, *tmp;

    DBUG_ENTER ("CreateSharedGlobalInfo");

    /* Append new sg_info to the prev_sg_infos */
    res = MEMmalloc (sizeof (shared_global_info_t));

    SG_INFO_SHRIDX_CAL (res) = NULL;
    SG_INFO_GLBIDX_CAL (res) = NULL;
    SG_INFO_SHRAVIS (res) = NULL;
    SG_INFO_GLBAVIS (res) = NULL;
    SG_INFO_RANGE_PAIRS (res) = NULL;
    SG_INFO_NEXT (res) = NULL;

    if (prev_sg_infos != NULL) {
        tmp = prev_sg_infos;
        while (SG_INFO_NEXT (tmp) != NULL) {
            tmp = SG_INFO_NEXT (tmp);
        }
        SG_INFO_NEXT (tmp) = res;
        res = prev_sg_infos;
    }

    DBUG_RETURN (res);
}

static node *
CreatePrf (char *name, simpletype sty, shape *shp, prf pfun, node *args, node **vardecs_p,
           node **assigns_p)
{
    node *avis = NULL, *new_assign;

    DBUG_ENTER ("CreatePrf");

    if (name != NULL) {
        avis
          = TBmakeAvis (TRAVtmpVarName (name), TYmakeAKS (TYmakeSimpleType (sty), shp));

        *vardecs_p = TBmakeVardec (avis, *vardecs_p);
    }

    new_assign = TBmakeAssign (TBmakeLet ((avis == NULL) ? avis : TBmakeIds (avis, NULL),
                                          TBmakePrf (pfun, args)),
                               NULL);

    if (avis != NULL) {
        AVIS_SSAASSIGN (avis) = new_assign;
    }

    if (&assigns_p == NULL) {
        *assigns_p = new_assign;
    } else {
        *assigns_p = TCappendAssign (*assigns_p, new_assign);
    }

    DBUG_RETURN (avis);
}

static range_pair_t *
GetNthRangePair (int nth)
{
    range_pair_t *pair = NULL;
    range_set_t *sets;
    range_info_t *blocked, *nonblocked;
    int cnt = 1, old_nth;

    DBUG_ENTER ("GetNthRangePair");

    old_nth = nth;
    sets = first_range_set;

    while (sets != NULL) {
        if (nth > RS_BLOCKED_RANGES_CNT (sets)) {
            nth -= RS_BLOCKED_RANGES_CNT (sets);
        } else {
            blocked = RS_BLOCKED_RANGES (sets);
            nonblocked = RS_NONBLOCKED_RANGES (sets);
            while (cnt <= nth) {
                DBUG_ASSERT ((blocked != NULL), "Blocked range list is NULL!");
                DBUG_ASSERT ((nonblocked != NULL), "Nonblocked range list is NULL!");
                blocked = RI_NEXT (blocked);
                nonblocked = RI_NEXT (RI_NEXT (nonblocked));
                cnt++;
            }
            pair = MEMmalloc (sizeof (range_pair_t));
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

static shared_global_info_t *
ComputeIndex (shared_global_info_t *sg_info, index_t *idx, info *arg_info)
{
    node *args, *vardecs = NULL;
    node *avis1, *avis2, *avis3, *assign1 = NULL, *assign2 = NULL, *assign3 = NULL;
    range_pair_t *pair = NULL;

    DBUG_ENTER ("ComputeIndex");

    switch (INDEX_TYPE (idx)) {
    case IDX_CONSTANT:
        /* For constant apprearing in global index, we create
         * an assignment "var_const = NUM;". For shared memory
         * index, we don't need to do anything */
        avis1 = TBmakeAvis (TRAVtmpVarName ("const"),
                            TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));

        assign1 = TBmakeAssign (TBmakeLet (TBmakeIds (avis1, NULL),
                                           TBmakeNum (INDEX_COEFFICIENT (idx))),
                                NULL);

        vardecs = TBmakeVardec (avis1, NULL);
        AVIS_SSAASSIGN (avis1) = assign1;

        if (SG_INFO_GLBAVIS (sg_info) != NULL) {
            args = TBmakeExprs (TBmakeId (avis1),
                                TBmakeExprs (TBmakeId (SG_INFO_GLBAVIS (sg_info)), NULL));
            avis2 = CreatePrf ("const", T_int, SHmakeShape (0), F_add_SxS, args, &vardecs,
                               &assign2);

            ASSIGN_NEXT (assign1) = assign2;

            SG_INFO_GLBAVIS (sg_info) = avis2;
            SG_INFO_GLBIDX_CAL (sg_info)
              = TCappendAssign (SG_INFO_GLBIDX_CAL (sg_info), assign1);
        } else {
            SG_INFO_GLBAVIS (sg_info) = avis1;
            SG_INFO_GLBIDX_CAL (sg_info) = assign1;
        }

        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = TCappendVardec (FUNDEF_VARDEC (INFO_FUNDEF (arg_info)), vardecs);

        break;

    case IDX_EXTID:
        /* For external id apprearing in global index, we create
         * an assignment "var_extid = _mul_SxS_( id, coe);". For shared memory
         * index, we don't need to do anything */
        args = TBmakeExprs (TBmakeId (INDEX_ID (idx)),
                            TBmakeExprs (TBmakeNum (INDEX_COEFFICIENT (idx)), NULL));
        avis1 = CreatePrf ("extid", T_int, SHmakeShape (0), F_mul_SxS, args, &vardecs,
                           &assign1);

        if (SG_INFO_GLBAVIS (sg_info) != NULL) {
            args = TBmakeExprs (TBmakeId (avis1),
                                TBmakeExprs (TBmakeId (SG_INFO_GLBAVIS (sg_info)), NULL));
            avis2 = CreatePrf ("extid", T_int, SHmakeShape (0), F_add_SxS, args, &vardecs,
                               &assign2);

            ASSIGN_NEXT (assign1) = assign2;

            SG_INFO_GLBAVIS (sg_info) = avis2;
            SG_INFO_GLBIDX_CAL (sg_info)
              = TCappendAssign (SG_INFO_GLBIDX_CAL (sg_info), assign1);
        } else {
            SG_INFO_GLBAVIS (sg_info) = avis1;
            SG_INFO_GLBIDX_CAL (sg_info) = assign1;
        }

        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = TCappendVardec (FUNDEF_VARDEC (INFO_FUNDEF (arg_info)), vardecs);

        break;

    case IDX_THREADIDX_X:

        /* Assignments for global memory index calculation */
        args = TBmakeExprs (TBmakeId (INDEX_ID (idx)),
                            TBmakeExprs (TBmakeId (CIS_TX (INFO_CIS (arg_info))), NULL));
        avis1 = CreatePrf ("tx_glb", T_int, SHmakeShape (0), F_sub_SxS, args, &vardecs,
                           &assign1);

        args = TBmakeExprs (TBmakeId (avis1),
                            TBmakeExprs (TBmakeNum (INDEX_COEFFICIENT (idx)), NULL));
        avis2 = CreatePrf ("tx_glb", T_int, SHmakeShape (0), F_mul_SxS, args, &vardecs,
                           &assign2);

        ASSIGN_NEXT (assign1) = assign2;

        if (SG_INFO_GLBAVIS (sg_info) != NULL) {
            args = TBmakeExprs (TBmakeId (avis2),
                                TBmakeExprs (TBmakeId (SG_INFO_GLBAVIS (sg_info)), NULL));
            avis3 = CreatePrf ("tx_glb", T_int, SHmakeShape (0), F_add_SxS, args,
                               &vardecs, &assign3);

            ASSIGN_NEXT (assign2) = assign3;

            SG_INFO_GLBAVIS (sg_info) = avis3;
            SG_INFO_GLBIDX_CAL (sg_info)
              = TCappendAssign (SG_INFO_GLBIDX_CAL (sg_info), assign1);
        } else {
            SG_INFO_GLBAVIS (sg_info) = avis2;
            SG_INFO_GLBIDX_CAL (sg_info) = assign1;
        }
        /********************************************/

        /* Assignments for shared memory index calculation */
        args = TBmakeExprs (TBmakeId (TBmakeId (CIS_TX (INFO_CIS (arg_info)))),
                            TBmakeExprs (TBmakeNum (INDEX_COEFFICIENT (idx)), NULL));
        avis1 = CreatePrf ("tx_shr", T_int, SHmakeShape (0), F_mul_SxS, args, &vardecs,
                           &assign1);

        if (SG_INFO_SHRAVIS (sg_info) != NULL) {
            args = TBmakeExprs (TBmakeId (avis1),
                                TBmakeExprs (TBmakeId (SG_INFO_SHRAVIS (sg_info)), NULL));
            avis2 = CreatePrf ("tx_shr", T_int, SHmakeShape (0), F_add_SxS, args,
                               &vardecs, &assign2);

            ASSIGN_NEXT (assign1) = assign2;

            SG_INFO_SHRAVIS (sg_info) = avis2;
            SG_INFO_SHRIDX_CAL (sg_info)
              = TCappendAssign (SG_INFO_SHRIDX_CAL (sg_info), assign1);
        } else {
            SG_INFO_SHRAVIS (sg_info) = avis1;
            SG_INFO_SHRIDX_CAL (sg_info) = assign1;
        }
        /********************************************/

        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = TCappendVardec (FUNDEF_VARDEC (INFO_FUNDEF (arg_info)), vardecs);

        break;

    case IDX_THREADIDX_Y:

        /* Assignments for global memory index calculation */
        args = TBmakeExprs (TBmakeId (INDEX_ID (idx)),
                            TBmakeExprs (TBmakeId (CIS_TY (INFO_CIS (arg_info))), NULL));
        avis1 = CreatePrf ("ty_glb", T_int, SHmakeShape (0), F_sub_SxS, args, &vardecs,
                           &assign1);

        args = TBmakeExprs (TBmakeId (avis1),
                            TBmakeExprs (TBmakeNum (INDEX_COEFFICIENT (idx)), NULL));
        avis2 = CreatePrf ("ty_glb", T_int, SHmakeShape (0), F_mul_SxS, args, &vardecs,
                           &assign2);

        ASSIGN_NEXT (assign1) = assign2;

        if (SG_INFO_GLBAVIS (sg_info) != NULL) {
            args = TBmakeExprs (TBmakeId (avis2),
                                TBmakeExprs (TBmakeId (SG_INFO_GLBAVIS (sg_info)), NULL));
            avis3 = CreatePrf ("ty_glb", T_int, SHmakeShape (0), F_add_SxS, args,
                               &vardecs, &assign3);

            ASSIGN_NEXT (assign2) = assign3;

            SG_INFO_GLBAVIS (sg_info) = avis3;
            SG_INFO_GLBIDX_CAL (sg_info)
              = TCappendAssign (SG_INFO_GLBIDX_CAL (sg_info), assign1);
        } else {
            SG_INFO_GLBAVIS (sg_info) = avis2;
            SG_INFO_GLBIDX_CAL (sg_info) = assign1;
        }
        /********************************************/

        /* Assignments for shared memory index calculation */
        args = TBmakeExprs (TBmakeId (TBmakeId (CIS_TY (INFO_CIS (arg_info)))),
                            TBmakeExprs (TBmakeNum (INDEX_COEFFICIENT (idx)), NULL));
        avis1 = CreatePrf ("ty_shr", T_int, SHmakeShape (0), F_mul_SxS, args, &vardecs,
                           &assign1);

        if (SG_INFO_SHRAVIS (sg_info) != NULL) {
            args = TBmakeExprs (TBmakeId (avis1),
                                TBmakeExprs (TBmakeId (SG_INFO_SHRAVIS (sg_info)), NULL));
            avis2 = CreatePrf ("ty_shr", T_int, SHmakeShape (0), F_add_SxS, args,
                               &vardecs, &assign2);

            ASSIGN_NEXT (assign1) = assign2;

            SG_INFO_SHRAVIS (sg_info) = avis2;
            SG_INFO_SHRIDX_CAL (sg_info)
              = TCappendAssign (SG_INFO_SHRIDX_CAL (sg_info), assign1);
        } else {
            SG_INFO_SHRAVIS (sg_info) = avis1;
            SG_INFO_SHRIDX_CAL (sg_info) = assign1;
        }
        /********************************************/

        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = TCappendVardec (FUNDEF_VARDEC (INFO_FUNDEF (arg_info)), vardecs);

        break;
    case IDX_LOOPIDX:
        pair = GetNthRangePair (INDEX_LOOPLEVEL (idx) - INFO_CUWLDIM (arg_info));
        DBUG_ASSERT ((pair != NULL), "Range pair is NULL!");
        RP_NEXT (pair) = SG_INFO_RANGE_PAIRS (sg_info);
        SG_INFO_RANGE_PAIRS (sg_info) = pair;

        /* Assignments for global memory index calculation */
        args = TBmakeExprs (TBmakeId (RP_OUTER (pair)),
                            TBmakeExprs (TBmakeNum (INDEX_COEFFICIENT (idx)), NULL));
        avis1 = CreatePrf ("loop_glb", T_int, SHmakeShape (0), F_mul_SxS, args, &vardecs,
                           &assign1);

        if (SG_INFO_GLBAVIS (sg_info) != NULL) {
            args = TBmakeExprs (TBmakeId (avis1),
                                TBmakeExprs (TBmakeId (SG_INFO_GLBAVIS (sg_info)), NULL));
            avis2 = CreatePrf ("loop_glb", T_int, SHmakeShape (0), F_add_SxS, args,
                               &vardecs, &assign2);

            ASSIGN_NEXT (assign1) = assign2;

            SG_INFO_GLBAVIS (sg_info) = avis2;
            SG_INFO_GLBIDX_CAL (sg_info)
              = TCappendAssign (SG_INFO_GLBIDX_CAL (sg_info), assign1);
        } else {
            SG_INFO_GLBAVIS (sg_info) = avis1;
            SG_INFO_GLBIDX_CAL (sg_info) = assign1;
        }
        /********************************************/

        /* Assignments for shared memory index calculation */
        args = TBmakeExprs (TBmakeId (RP_INNER (pair)),
                            TBmakeExprs (TBmakeNum (INDEX_COEFFICIENT (idx)), NULL));
        avis1 = CreatePrf ("loop_shr", T_int, SHmakeShape (0), F_mul_SxS, args, &vardecs,
                           &assign1);

        if (SG_INFO_SHRAVIS (sg_info) != NULL) {
            args = TBmakeExprs (TBmakeId (avis1),
                                TBmakeExprs (TBmakeId (SG_INFO_SHRAVIS (sg_info)), NULL));
            avis2 = CreatePrf ("loop_shr", T_int, SHmakeShape (0), F_add_SxS, args,
                               &vardecs, &assign2);

            ASSIGN_NEXT (assign1) = assign2;

            SG_INFO_SHRAVIS (sg_info) = avis2;
            SG_INFO_SHRIDX_CAL (sg_info)
              = TCappendAssign (SG_INFO_SHRIDX_CAL (sg_info), assign1);
        } else {
            SG_INFO_SHRAVIS (sg_info) = avis1;
            SG_INFO_SHRIDX_CAL (sg_info) = assign1;
        }
        /********************************************/

        break;
    default:
        DBUG_ASSERT ((0), "Unknown index type found!");
        break;
    }

    DBUG_RETURN (sg_info);
}

/*
 * INFO macros
 */

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_LEVEL (result) = 0;
    INFO_FUNDEF (result) = NULL;
    INFO_LASTASSIGN (result) = NULL;
    INFO_RANGE_SETS (result) = NULL;
    INFO_WITH3 (result) = NULL;
    INFO_CUWLDIM (result) = 0;
    INFO_CIS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

static range_info_t *
CreateRangeInfo (node *range, bool toplevel)
{
    range_info_t *res;

    DBUG_ENTER ("CreateRangeInfo");

    res = MEMmalloc (sizeof (range_info_t));

    RI_RANGE (res) = range;
    RI_TOPLEVEL (res) = toplevel;
    RI_PREV (res) = NULL;
    RI_NEXT (res) = NULL;

    DBUG_RETURN (res);
}

static range_set_t *
CreateRangeSet ()
{
    range_set_t *res;

    DBUG_ENTER ("CreateRangeSet");

    res = MEMmalloc (sizeof (range_set_t));

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
EnqRangeInfo (range_set_t *set, node *range, bool toplevel)
{
    range_info_t *info, *next, *dummy;

    DBUG_ENTER ("EnqRangeInfo");

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

    DBUG_ENTER ("DeqRangeInfo");

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
        DBUG_ASSERT ((0), "N_range in neither blocked nor nonblocked ranges!");
    }

    DBUG_RETURN (set);
}

static range_info_t *
FreeRangeInfo (range_info_t *info)
{
    DBUG_ENTER ("FreeRangeInfo");

    if (info == NULL) {
        return info;
    } else {
        RI_NEXT (info) = FreeRangeInfo (RI_NEXT (info));
        info = MEMfree (info);
        info = NULL;
    }

    DBUG_RETURN (info);
}

static range_set_t *
PushRangeSet (range_set_t *sets)
{
    range_set_t *new_set;

    DBUG_ENTER ("PushRangeSet");

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

    DBUG_ENTER ("PopRangeSet");

    if (sets != NULL) {
        popped_set = sets;
        sets = RS_NEXT (sets);
        RS_PREV (sets) = NULL;
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

static void
PrintSpaces (int num)
{
    int i;

    DBUG_ENTER ("PrintSpaces");

    for (i = 0; i < num; i++) {
        printf ("  ");
    }

    DBUG_VOID_RETURN;
}

static void
PrintRangeSet (range_set_t *sets, int indent)
{
    range_info_t *blocked_ranges;
    range_info_t *nonblocked_ranges;

    DBUG_ENTER ("PrintRangeSet");

    blocked_ranges = RS_BLOCKED_RANGES (sets);
    nonblocked_ranges = RS_NONBLOCKED_RANGES (sets);

    PrintSpaces (indent);
    printf ("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

    PrintSpaces (indent);
    if (RS_LAST_BLOCKED_RANGE (sets) != NULL) {
        printf ("Last Blocked Range: %s[toplevel:%d]\n",
                IDS_NAME (RANGE_INDEX (RI_RANGE (RS_LAST_BLOCKED_RANGE (sets)))),
                RI_TOPLEVEL (RS_LAST_BLOCKED_RANGE (sets)));
    } else {
        printf ("Last Blocked Range: NULL\n");
    }

    PrintSpaces (indent);
    if (RS_LAST_NONBLOCKED_RANGE (sets) != NULL) {
        printf ("Last Nonblocked Range: %s[toplevel:%d]\n",
                IDS_NAME (RANGE_INDEX (RI_RANGE (RS_LAST_NONBLOCKED_RANGE (sets)))),
                RI_TOPLEVEL (RS_LAST_NONBLOCKED_RANGE (sets)));
    } else {
        printf ("Last Nonblocked Range: NULL\n");
    }

    PrintSpaces (indent);
    printf ("Blocked Ranges[%d]: ", RS_BLOCKED_RANGES_CNT (sets));
    while (blocked_ranges != NULL) {
        printf ("(Index:%s) ", IDS_NAME (RANGE_INDEX (RI_RANGE (blocked_ranges))));
        blocked_ranges = RI_NEXT (blocked_ranges);
    }
    printf ("\n");

    PrintSpaces (indent);
    printf ("Nonblocked Ranges[%d]: ", RS_NONBLOCKED_RANGES_CNT (sets));
    while (nonblocked_ranges != NULL) {
        if (RANGE_INDEX (RI_RANGE (nonblocked_ranges)) == NULL) {
            printf ("(Index:Dummy) ");
        } else {
            printf ("(Index:%s) ", IDS_NAME (RANGE_INDEX (RI_RANGE (nonblocked_ranges))));
        }
        nonblocked_ranges = RI_NEXT (nonblocked_ranges);
    }
    printf ("\n");

    PrintSpaces (indent);
    printf ("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

    DBUG_VOID_RETURN;
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

    DBUG_ENTER ("CUDRdoCudaDataReuse");

    info = MakeInfo ();
    TRAVpush (TR_cudr);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
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
    DBUG_ENTER ("CUDRfundef");

    INFO_FUNDEF (arg_info) = arg_node;
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

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

    DBUG_ENTER ("CUDRassign");

    old_lastassign = INFO_LASTASSIGN (arg_info);
    INFO_LASTASSIGN (arg_info) = arg_node;

    ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);

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
    DBUG_ENTER ("CUDRwith");

    if (WITH_CUDARIZABLE (arg_node)) {
        INFO_CUWLDIM (arg_info) = TCcountIds (WITH_IDS (arg_node));
        WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

static void
CreateCudaIndexInitCode (node *part, info *arg_info)
{
    int dim;
    cuidx_set_t *cis;
    node *assigns = NULL, *vardecs = NULL;

    DBUG_ENTER ("CreateCudaIndexInitCode");

    cis = MEMmalloc (sizeof (cuidx_set_t));
    dim = TCcountIds (PART_IDS (part));

    CIS_TX (cis) = CreatePrf ("tx", T_int, SHmakeShape (0), F_cuda_threadIdx_x, NULL,
                              &vardecs, &assigns);
    AVIS_SSAASSIGN (CIS_TX (cis)) = assigns;

    CIS_BX (cis) = CreatePrf ("b_dim_x", T_int, SHmakeShape (0), F_cuda_blockDim_x, NULL,
                              &vardecs, &assigns);
    AVIS_SSAASSIGN (CIS_BX (cis)) = assigns;

    if (dim == 2) {
        CIS_TY (cis) = CreatePrf ("ty", T_int, SHmakeShape (0), F_cuda_threadIdx_y, NULL,
                                  &vardecs, &assigns);
        AVIS_SSAASSIGN (CIS_TY (cis)) = assigns;

        CIS_BY (cis) = CreatePrf ("b_dim_y", T_int, SHmakeShape (0), F_cuda_blockDim_y,
                                  NULL, &vardecs, &assigns);
        AVIS_SSAASSIGN (CIS_BY (cis)) = assigns;
    }

    FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
      = TCappendVardec (FUNDEF_VARDEC (INFO_FUNDEF (arg_info)), vardecs);

    BLOCK_INSTR (PART_CBLOCK (part))
      = TCappendAssign (assigns, BLOCK_INSTR (PART_CBLOCK (part)));

    INFO_CIS (arg_info) = cis;

    DBUG_VOID_RETURN;
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
    int dim;

    DBUG_ENTER ("CUDRpart");

    dim = TCcountIds (PART_IDS (arg_node));

    if (NODE_TYPE (BLOCK_INSTR (PART_CBLOCK (arg_node))) != N_empty) {
        CreateCudaIndexInitCode (arg_node, arg_info);

        INFO_LEVEL (arg_info) += dim;
        PART_CODE (arg_node) = TRAVopt (PART_CODE (arg_node), arg_info);
        INFO_LEVEL (arg_info) -= dim;
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

    DBUG_ENTER ("CUDRwith3");

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
    DBUG_ENTER ("CUDRrange");

    /* We only traverse the range if it is NOT for fitting */
    if (!RANGE_ISFITTING (arg_node)) {
        INFO_RANGE_SETS (arg_info)
          = EnqRangeInfo (INFO_RANGE_SETS (arg_info), arg_node,
                          WITH3_ISTOPLEVEL (INFO_WITH3 (arg_info)));

        INFO_LEVEL (arg_info)++;

#if 1
        PrintSpaces (INFO_LEVEL (arg_info));
        printf ("Entering range [index:%s level:%d blocked:%d]\n",
                IDS_NAME (RANGE_INDEX (arg_node)), INFO_LEVEL (arg_info),
                RANGE_ISBLOCKED (arg_node));
        PrintRangeSet (INFO_RANGE_SETS (arg_info), INFO_LEVEL (arg_info));
#endif

        RANGE_BODY (arg_node) = TRAVopt (RANGE_BODY (arg_node), arg_info);

#if 1
        PrintSpaces (INFO_LEVEL (arg_info));
        printf ("Leaving range [index:%s level:%d blocked:%d]\n",
                IDS_NAME (RANGE_INDEX (arg_node)), INFO_LEVEL (arg_info),
                RANGE_ISBLOCKED (arg_node));
#endif

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
    DBUG_ENTER ("CUDRcode");

    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

static node *
ComputeSharedMemoryIndex (shared_global_info_t *sg_info, cuda_access_info_t *access_info)
{
    node *new_ass, *args = NULL, *assigns = NULL;

    DBUG_ENTER ("ComputeSharedMemoryIndex");

    args = TBmakeExprs (DUPdoDupNode (CUAI_SHARRAYSHP (access_info)), NULL);

    while (sg_info != NULL) {
        args = TCcombineExprs (args, TBmakeId (SG_INFO_SHRAVIS (sg_info)));
        assigns = TCappendAssign (assigns, SG_INFO_SHRIDX_CAL (sg_info));
        sg_info = SG_INFO_NEXT (sg_info);
    }

    new_ass = TBmakeAssign (TBmakePrf (F_idxs2offset, args), NULL);
    assigns = TCappendAssign (assigns, new_ass);

    DBUG_RETURN (assigns);
}

/*
static range_pair_t *GetInnermostRangePair( range_pair_t *range_pairs)
{
  range_pair_t *innermost = NULL;
  int level = -1;

  DBUG_ENTER("GetInnermostRangePair");

  innermost = range_pairs;

  while( range_pairs != NULL) {
    if( RP_OUTERLEVEL( range_pairs) > level) {
      innermost = range_pairs;
      level = RP_OUTERLEVEL( range_pairs);
    }
    range_pairs = RP_NEXT( range_pairs);
  }

  DBUG_RETURN( innermost);
}
*/

/*
typedef struct SHARED_GLOBAL_INFO_T {
  node *shridx_cal;
  node *glbidx_cal;
  node *shravis;
  node *glbavis;
  range_pair_t *range_pairs;
  struct SHARED_GLOBAL_INFO_T *next;
} shared_global_info_t;
*/
/*
static node*
ComputeGlobalMemoryIndex( shared_global_info_t *sg_info,
                          cuda_access_info_t *access_info,
                          int dim)
{
  node *new_ass, *args = NULL, *assigns = NULL;

  DBUG_ENTER("ComputeGlobalMemoryIndex");

  args = TBmakeExprs( DUPdoDupNode( CUAI_SHARRAYSHP( access_info)), NULL);

  while( sg_info != NULL) {
    args = TCcombineExprs( args, TBmakeId( SG_INFO_SHRAVIS( sg_info)));
    assigns = TCappendAssign( assigns, SG_INFO_SHRIDX_CAL( sg_info));
    sg_info = SG_INFO_NEXT( sg_info);
  }

  new_ass = TBmakeAssign( TBmakePrf( F_idxs2offset, args), NULL);
  assigns = TCappendAssign( assigns, new_ass);

  DBUG_RETURN( assigns);
}
*/

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
    shared_global_info_t *sg_info = NULL;

    DBUG_ENTER ("CUDRprf");

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
                /* Add declaration for shared memory */
                FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
                  = TCappendVardec (FUNDEF_VARDEC (INFO_FUNDEF (arg_info)),
                                    TBmakeVardec (CUAI_SHARRAY (access_info), NULL));

                index_t *idx;
                i = 0;
                for (i = 0; i < CUAI_DIM (access_info); i++) {
                    sg_info = CreateSharedGlobalInfo (sg_info);
                    idx = CUAI_INDICES (access_info, i);
                    while (idx != NULL) {
                        sg_info = ComputeIndex (sg_info, idx, arg_info);
                        idx = INDEX_NEXT (idx);
                    }
                }

                INFO_PREASSIGNS (arg_info)
                  = ComputeSharedMemoryIndex (sg_info, access_info);
            }
        }
    }

    DBUG_RETURN (arg_node);
}
