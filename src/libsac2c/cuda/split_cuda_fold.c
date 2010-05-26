
#include "split_cuda_fold.h"

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

typedef enum { trav_normal, trav_genarray, trav_fold } travmode_t;

/*
 * INFO structure
 */
struct INFO {
    node *lhs;
    node *preassign;
    node *fundef;
    node *shape;
    node *array;
    node *neutral;
    node *withids;
    node *withvec;
    travmode_t travmode;
};

/*
 * INFO macros
 */
#define INFO_LHS(n) (n->lhs)
#define INFO_PREASSIGN(n) (n->preassign)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_SHAPE(n) (n->shape)
#define INFO_ARRAY(n) (n->array)
#define INFO_NEUTRAL(n) (n->neutral)
#define INFO_WITHIDS(n) (n->withids)
#define INFO_WITHVEC(n) (n->withids)
#define INFO_TRAVMODE(n) (n->travmode)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_LHS (result) = NULL;
    INFO_PREASSIGN (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_SHAPE (result) = NULL;
    INFO_ARRAY (result) = NULL;
    INFO_NEUTRAL (result) = NULL;
    INFO_WITHIDS (result) = NULL;
    INFO_WITHVEC (result) = NULL;
    INFO_TRAVMODE (result) = trav_normal;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

node *
SCUFdoSplitCudaFold (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("SCUFdoSplitCudaFold");

    info = MakeInfo ();
    TRAVpush (TR_scuf);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

node *
SCUFfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCUFfundef");

    INFO_FUNDEF (arg_info) = arg_node;

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
SCUFassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCUFassign");

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_PREASSIGN (arg_info) != NULL && INFO_TRAVMODE (arg_info) == trav_normal) {
        arg_node = TCappendAssign (INFO_PREASSIGN (arg_info), arg_node);
        INFO_PREASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

node *
SCUFlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCUFlet");

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
SCUFwith (node *arg_node, info *arg_info)
{
    node *new_with, *wlidx;

    DBUG_ENTER ("SCUFwith");

    if (WITH_CUDARIZABLE (arg_node) && NODE_TYPE (WITH_WITHOP (arg_node)) == N_fold) {
        /* Traverse the partition and the opertaion to collect
         * information needed to build a new genarray withloop */
        WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
        WITH_WITHOP (arg_node) = TRAVopt (WITH_WITHOP (arg_node), arg_info);

        /* Create a new genarray withloop */
        new_with = DUPdoDupNode (arg_node);
        WITH_WITHOP (new_with) = FREEdoFreeNode (WITH_WITHOP (new_with));
        WITH_WITHOP (new_with)
          = TBmakeGenarray (INFO_SHAPE (arg_info), INFO_NEUTRAL (arg_info));

        wlidx = TBmakeAvis (TRAVtmpVarName ("wlidx"),
                            TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = TCappendVardec (FUNDEF_VARDEC (INFO_FUNDEF (arg_info)),
                            TBmakeVardec (wlidx, NULL));
        GENARRAY_IDX (WITH_WITHOP (new_with)) = wlidx;
        WITHID_IDXS (WITH_WITHID (new_with)) = TBmakeIds (wlidx, NULL);

        /* Start traversing N_code of the new genarray withloop */
        INFO_TRAVMODE (arg_info) = trav_genarray;
        WITH_CODE (new_with) = TRAVopt (WITH_CODE (new_with), arg_info);
        INFO_TRAVMODE (arg_info) = trav_normal;

        /* Start traversing N_code of the old fold withloop */
        INFO_TRAVMODE (arg_info) = trav_fold;
        INFO_WITHIDS (arg_info) = WITH_IDS (arg_node);
        INFO_WITHVEC (arg_info) = WITH_VEC (arg_node);
        WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);
        INFO_TRAVMODE (arg_info) = trav_normal;

        INFO_PREASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (INFO_ARRAY (arg_info), new_with), NULL);
        AVIS_SSAASSIGN (IDS_AVIS (INFO_ARRAY (arg_info))) = INFO_PREASSIGN (arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
SCUFcode (node *arg_node, info *arg_info)
{
    node *accu_avis, *offset_avis, *elem_avis, *op_avis;
    node *accu_ass, *offset_ass, *elem_ass, *op_ass;
    node *cexpr_avis;
    prf op;

    DBUG_ENTER ("SCUFcode");

    if (INFO_TRAVMODE (arg_info) == trav_genarray) {
        CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);
    } else if (INFO_TRAVMODE (arg_info) == trav_fold) {
        BLOCK_INSTR (CODE_CBLOCK (arg_node))
          = FREEdoFreeTree (BLOCK_INSTR (CODE_CBLOCK (arg_node)));

        /* Get the reduction operation of this fold withloop */
        cexpr_avis = ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (arg_node)));
        op = PRF_PRF (LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (cexpr_avis))));
        CODE_CEXPR (arg_node) = FREEdoFreeNode (CODE_CEXPR (arg_node));

        /* tmp1 = accu(iv); */
        accu_avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (AVIS_TYPE (cexpr_avis)));
        accu_ass
          = TBmakeAssign (TBmakeLet (TBmakeIds (accu_avis, NULL),
                                     TCmakePrf1 (F_accu, TBmakeId (IDS_AVIS (
                                                           INFO_WITHVEC (arg_info))))),
                          NULL);
        AVIS_SSAASSIGN (accu_avis) = accu_ass;
        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = TCappendVardec (FUNDEF_VARDEC (INFO_FUNDEF (arg_info)),
                            TBmakeVardec (accu_avis, NULL));

        /* tmp2 = idxs2offset([...], i, j, k...); */
        offset_avis = TBmakeAvis (TRAVtmpVar (),
                                  TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
        offset_ass
          = TBmakeAssign (TBmakeLet (TBmakeIds (offset_avis, NULL),
                                     TBmakePrf (F_idxs2offset,
                                                TBmakeExprs (DUPdoDupNode (
                                                               INFO_SHAPE (arg_info)),
                                                             TCids2Exprs (INFO_WITHIDS (
                                                               arg_info))))),
                          NULL);
        AVIS_SSAASSIGN (offset_avis) = offset_ass;
        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = TCappendVardec (FUNDEF_VARDEC (INFO_FUNDEF (arg_info)),
                            TBmakeVardec (offset_avis, NULL));

        printf ("after offset\n");

        /* tmp3 = idx_sel( offset, array); */
        elem_avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (AVIS_TYPE (cexpr_avis)));
        elem_ass = TBmakeAssign (TBmakeLet (TBmakeIds (elem_avis, NULL),
                                            TCmakePrf2 (F_idx_sel, TBmakeId (offset_avis),
                                                        TBmakeId (IDS_AVIS (
                                                          INFO_ARRAY (arg_info))))),
                                 NULL);
        AVIS_SSAASSIGN (elem_avis) = elem_ass;
        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = TCappendVardec (FUNDEF_VARDEC (INFO_FUNDEF (arg_info)),
                            TBmakeVardec (elem_avis, NULL));

        printf ("after elem\n");
        /* tmp4 = op( arg_1, arg_2); */
        op_avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (AVIS_TYPE (cexpr_avis)));
        op_ass = TBmakeAssign (TBmakeLet (TBmakeIds (op_avis, NULL),
                                          TCmakePrf2 (op, TBmakeId (accu_avis),
                                                      TBmakeId (elem_avis))),
                               NULL);
        AVIS_SSAASSIGN (op_avis) = op_ass;
        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = TCappendVardec (FUNDEF_VARDEC (INFO_FUNDEF (arg_info)),
                            TBmakeVardec (op_avis, NULL));
        printf ("after op\n");
        CODE_CEXPR (arg_node) = TBmakeExprs (TBmakeId (op_avis), NULL);
        BLOCK_INSTR (CODE_CBLOCK (arg_node))
          = TCappendAssign (accu_ass,
                            TCappendAssign (offset_ass,
                                            TCappendAssign (elem_ass,
                                                            TCappendAssign (op_ass,
                                                                            NULL))));
    } else {
        DBUG_ASSERT ((0), "Wrong traverse mode in SCUFcode!");
    }

    DBUG_RETURN (arg_node);
}

node *
SCUFpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCUFpart");

    DBUG_ASSERT ((PART_NEXT (arg_node)) == NULL,
                 "Found fold withloop with more than one partition!");

    PART_GENERATOR (arg_node) = TRAVopt (PART_GENERATOR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
SCUFgenerator (node *arg_node, info *arg_info)
{
    node *avis, *shape_expr;
    ntype *type = NULL;

    DBUG_ENTER ("SCUFgenerator");

    shape_expr = DUPdoDupNode (GENERATOR_BOUND2 (arg_node));

    if (NODE_TYPE (shape_expr) == N_array) {
        if (COisConstant (shape_expr)) {
            type = TYmakeAKS (TYmakeSimpleType (TYgetSimpleType (
                                TYgetScalar (IDS_NTYPE (INFO_LHS (arg_info))))),
                              SHarray2Shape (shape_expr));
        }
    } else if (NODE_TYPE (shape_expr) == N_id) {
        DBUG_ASSERT ((0), "We are not supporting N_id bound2 yet!");
    } else {
        DBUG_ASSERT ((0), "Bound2 is of wrong node type!");
    }

    avis = TBmakeAvis (TRAVtmpVar (), type);

    FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
      = TCappendVardec (FUNDEF_VARDEC (INFO_FUNDEF (arg_info)),
                        TBmakeVardec (avis, NULL));

    INFO_ARRAY (arg_info) = TBmakeIds (avis, NULL);
    INFO_SHAPE (arg_info) = shape_expr;

    DBUG_RETURN (arg_node);
}

node *
SCUFfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCUFfold");

    INFO_NEUTRAL (arg_info) = DUPdoDupNode (FOLD_NEUTRAL (arg_node));

    DBUG_RETURN (arg_node);
}

node *
SCUFprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCUFprf");

    if (INFO_TRAVMODE (arg_info) == trav_genarray) {
        if (PRF_PRF (arg_node) == F_accu) {
            arg_node = FREEdoFreeNode (arg_node);
            arg_node = DUPdoDupNode (INFO_NEUTRAL (arg_info));
        }
    } else if (INFO_TRAVMODE (arg_info) == trav_fold) {
    } else {
        /* Do nothing */
    }

    DBUG_RETURN (arg_node);
}
