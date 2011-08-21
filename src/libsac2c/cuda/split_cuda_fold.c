
#include "split_cuda_fold.h"

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
#include "remove_unused_lac.h"

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
    node *foldwl;
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
#define INFO_FOLDWL(n) (n->foldwl)
#define INFO_TRAVMODE(n) (n->travmode)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_LHS (result) = NULL;
    INFO_PREASSIGN (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_SHAPE (result) = NULL;
    INFO_ARRAY (result) = NULL;
    INFO_NEUTRAL (result) = NULL;
    INFO_FOLDWL (result) = NULL;
    INFO_TRAVMODE (result) = trav_normal;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

static node *
AppendVardec (node *fundef, node *avis)
{
    DBUG_ENTER ();

    FUNDEF_VARDECS (fundef)
      = TCappendVardec (FUNDEF_VARDECS (fundef), TBmakeVardec (avis, NULL));

    DBUG_RETURN (fundef);
}

node *
SCUFdoSplitCudaFold (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();
    TRAVpush (TR_scuf);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

    syntax_tree = RLACdoRemoveUnusedLac (syntax_tree);

    DBUG_RETURN (syntax_tree);
}

node *
SCUFfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* Bug found when compiling mandelbrot. We should
     * not traverse into sticky functions */
    if (!FUNDEF_ISSTICKY (arg_node)) {
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    }
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
SCUFassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_PREASSIGN (arg_info) != NULL && INFO_TRAVMODE (arg_info) == trav_normal) {
        arg_node = TCappendAssign (INFO_PREASSIGN (arg_info), arg_node);
        INFO_PREASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

node *
SCUFlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
SCUFwith (node *arg_node, info *arg_info)
{
    node *new_with, *wlidx;

    DBUG_ENTER ();

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
        INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), wlidx);
        GENARRAY_IDX (WITH_WITHOP (new_with)) = wlidx;
        WITHID_IDXS (WITH_WITHID (new_with)) = TBmakeIds (wlidx, NULL);

        /* Start traversing N_code of the new genarray withloop */
        INFO_TRAVMODE (arg_info) = trav_genarray;
        WITH_CODE (new_with) = TRAVopt (WITH_CODE (new_with), arg_info);
        INFO_TRAVMODE (arg_info) = trav_normal;

        /* Start traversing N_code of the old fold withloop */
        INFO_TRAVMODE (arg_info) = trav_fold;
        /* update iv and wlids in the first partition */
        WITH_WITHID (arg_node) = TRAVopt (WITH_WITHID (arg_node), arg_info);
        INFO_FOLDWL (arg_info) = arg_node;
        WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
        INFO_FOLDWL (arg_info) = NULL;
        INFO_TRAVMODE (arg_info) = trav_normal;

        WITH_CUDARIZABLE (arg_node) = FALSE;

        INFO_PREASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (INFO_ARRAY (arg_info), new_with), NULL);
        AVIS_SSAASSIGN (IDS_AVIS (INFO_ARRAY (arg_info))) = INFO_PREASSIGN (arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
SCUFcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (INFO_TRAVMODE (arg_info) == trav_genarray,
                 "Wrong traverse mode in SCUFcode!");

    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);
    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
SCUFpart (node *arg_node, info *arg_info)
{
    node *accu_avis, *offset_avis, *elem_avis, *op_avis;
    node *accu_ass, *offset_ass, *elem_ass, *op_ass;
    node *cexpr_avis;
    ntype *cexpr_type;
    node *new_code, *code_instr;
    prf op;

    DBUG_ENTER ();

    if (INFO_TRAVMODE (arg_info) == trav_normal) {
        if (PART_NEXT (arg_node) == NULL) {
            PART_GENERATOR (arg_node) = TRAVopt (PART_GENERATOR (arg_node), arg_info);
        } else {
            PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
        }
    } else if (INFO_TRAVMODE (arg_info) == trav_fold) {
        /* Get the reduction operation of this fold withloop */
        cexpr_avis = ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (PART_CODE (arg_node))));
        cexpr_type = AVIS_TYPE (cexpr_avis);
        op = PRF_PRF (LET_EXPR (ASSIGN_STMT (AVIS_SSAASSIGN (cexpr_avis))));

        if (PART_NEXT (arg_node) != NULL) {
            PART_NEXT (arg_node) = FREEdoFreeTree (PART_NEXT (arg_node));
            PART_NEXT (arg_node) = NULL;
        }
        PART_CODE (arg_node) = FREEdoFreeTree (PART_CODE (arg_node));

        PART_BOUND2 (arg_node) = FREEdoFreeNode (PART_BOUND2 (arg_node));
        PART_BOUND2 (arg_node) = DUPdoDupNode (INFO_SHAPE (arg_info));

        /* tmp1 = accu(iv); */
        accu_avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (cexpr_type));
        accu_ass = TBmakeAssign (TBmakeLet (TBmakeIds (accu_avis, NULL),
                                            TCmakePrf1 (F_accu, TBmakeId (IDS_AVIS (
                                                                  PART_VEC (arg_node))))),
                                 NULL);
        AVIS_SSAASSIGN (accu_avis) = accu_ass;
        INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), accu_avis);

        /* tmp2 = idxs2offset([...], i, j, k...); */
        offset_avis = TBmakeAvis (TRAVtmpVar (),
                                  TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
        offset_ass
          = TBmakeAssign (TBmakeLet (TBmakeIds (offset_avis, NULL),
                                     TBmakePrf (F_idxs2offset,
                                                TBmakeExprs (DUPdoDupNode (
                                                               INFO_SHAPE (arg_info)),
                                                             TCids2Exprs (
                                                               PART_IDS (arg_node))))),
                          NULL);
        AVIS_SSAASSIGN (offset_avis) = offset_ass;
        INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), offset_avis);

        /* tmp3 = idx_sel( offset, array); */
        elem_avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (cexpr_type));
        elem_ass = TBmakeAssign (TBmakeLet (TBmakeIds (elem_avis, NULL),
                                            TCmakePrf2 (F_idx_sel, TBmakeId (offset_avis),
                                                        TBmakeId (IDS_AVIS (
                                                          INFO_ARRAY (arg_info))))),
                                 NULL);
        AVIS_SSAASSIGN (elem_avis) = elem_ass;
        INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), elem_avis);

        /* tmp4 = op( arg_1, arg_2); */
        op_avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (cexpr_type));
        op_ass = TBmakeAssign (TBmakeLet (TBmakeIds (op_avis, NULL),
                                          TCmakePrf2 (op, TBmakeId (accu_avis),
                                                      TBmakeId (elem_avis))),
                               NULL);
        AVIS_SSAASSIGN (op_avis) = op_ass;
        INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), op_avis);

        /* Update the new partition */
        code_instr
          = TCappendAssign (accu_ass,
                            TCappendAssign (offset_ass,
                                            TCappendAssign (elem_ass,
                                                            TCappendAssign (op_ass,
                                                                            NULL))));
        new_code = TBmakeCode (TBmakeBlock (code_instr, NULL),
                               TBmakeExprs (TBmakeId (op_avis), NULL));
        CODE_USED (new_code) = 1;
        CODE_NEXT (new_code) = NULL;
        PART_CODE (arg_node) = new_code;
        WITH_CODE (INFO_FOLDWL (arg_info)) = new_code;
    } else {
        DBUG_ASSERT (0, "Wrong traverse mode in SCUFpart!");
    }

    DBUG_RETURN (arg_node);
}

node *
SCUFwithid (node *arg_node, info *arg_info)
{
    node *new_avis, *withids;
    ;

    DBUG_ENTER ();

    if (INFO_TRAVMODE (arg_info) == trav_fold) {
        new_avis = TBmakeAvis (TRAVtmpVarName ("iv"),
                               TYcopyType (IDS_NTYPE (WITHID_VEC (arg_node))));
        IDS_AVIS (WITHID_VEC (arg_node)) = new_avis;
        INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), new_avis);

        withids = WITHID_IDS (arg_node);
        while (withids != NULL) {
            new_avis
              = TBmakeAvis (TRAVtmpVarName ("ids"), TYcopyType (IDS_NTYPE (withids)));
            INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), new_avis);
            IDS_AVIS (withids) = new_avis;
            withids = IDS_NEXT (withids);
        }
    }

    DBUG_RETURN (arg_node);
}

node *
SCUFgenerator (node *arg_node, info *arg_info)
{
    node *avis, *shape_expr;
    ntype *type = NULL;

    DBUG_ENTER ();

    shape_expr = DUPdoDupNode (GENERATOR_BOUND2 (arg_node));

    if (NODE_TYPE (shape_expr) == N_array) {
        if (COisConstant (shape_expr)) {
            type = TYmakeAKS (TYmakeSimpleType (TYgetSimpleType (
                                TYgetScalar (IDS_NTYPE (INFO_LHS (arg_info))))),
                              SHarray2Shape (shape_expr));
        }
    } else if (NODE_TYPE (shape_expr) == N_id) {
        DBUG_ASSERT (0, "We are not supporting N_id bound2 yet!");
    } else {
        DBUG_ASSERT (0, "Bound2 is of wrong node type!");
    }

    avis = TBmakeAvis (TRAVtmpVar (), type);

    INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), avis);

    INFO_ARRAY (arg_info) = TBmakeIds (avis, NULL);
    INFO_SHAPE (arg_info) = shape_expr;

    DBUG_RETURN (arg_node);
}

node *
SCUFfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_NEUTRAL (arg_info) = DUPdoDupNode (FOLD_NEUTRAL (arg_node));

    DBUG_RETURN (arg_node);
}

node *
SCUFprf (node *arg_node, info *arg_info)
{
    // ntype *type;

    DBUG_ENTER ();

    if (INFO_TRAVMODE (arg_info) == trav_genarray) {
        if (PRF_PRF (arg_node) == F_accu) {
            arg_node = FREEdoFreeNode (arg_node);
            arg_node = DUPdoDupNode (INFO_NEUTRAL (arg_info));
            /*
            type = IDS_NTYPE( INFO_LHS( arg_info));
            arg_node = COconstant2AST(
                         COmakeZero( TYgetSimpleType( TYgetScalar( type)),
                                      TYgetShape( type)));
            */
        }
    } else if (INFO_TRAVMODE (arg_info) == trav_fold) {
    } else {
        /* Do nothing */
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
