#include <stdio.h>

#include "extend_lac_funs.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "DupTree.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "namespaces.h"
#include "deserialize.h"
#include "new_types.h"

#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse_helper.h"
#include "globals.h"

/*
 * OPEN PROBLEMS:
 *
 * I) not yet solved:
 *
 * II) to be fixed here:
 *
 * III) to be fixed somewhere else:
 *
 */

/**
 * This phase ....
 *
 */

/*
 * usages of 'arg_info':
 *
 *   INFO_VARDECS     - vardec chain of freshly generated vardecs
 *   INFO_ASSIGNS     - assignment chain of freshly generated assignments
 */

/**
 * INFO structure
 */
struct INFO {
    node *vardecs;
    node *assigns;
};

/**
 * INFO macros
 */
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_ASSIGNS(n) ((n)->assigns)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_VARDECS (result) = NULL;
    INFO_ASSIGNS (result) = NULL;

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
 * @fn node * SearchPredicate( node *ap)
 *
 *****************************************************************************/

static node *
SearchPredicate (node *ap)
{
    node *pred_avis, *formal_args, *act_args, *assign;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (AP_FUNDEF (ap)) == N_fundef,
                 "AP_FUNDEF does not point to a fundef node");
    DBUG_ASSERT (FUNDEF_ISCONDFUN (AP_FUNDEF (ap)) || FUNDEF_ISLOOPFUN (AP_FUNDEF (ap)),
                 "AP_FUNDEF does not point to a LaC fun fundef node");
    DBUG_ASSERT (FUNDEF_BODY (AP_FUNDEF (ap)) != NULL,
                 "AP_FUNDEF points to a fundef node without body");
    DBUG_ASSERT (NODE_TYPE (FUNDEF_BODY (AP_FUNDEF (ap))) == N_block,
                 "AP_FUNDEF does not point to a fundef with a block node");

    assign = BLOCK_ASSIGNS (FUNDEF_BODY (AP_FUNDEF (ap)));

    pred_avis = ID_AVIS (COND_COND (ASSIGN_STMT (assign)));

    formal_args = FUNDEF_ARGS (AP_FUNDEF (ap));
    act_args = AP_ARGS (ap);

    while ((formal_args != NULL) && (ARG_AVIS (formal_args) != pred_avis)) {
        formal_args = ARG_NEXT (formal_args);
        act_args = EXPRS_NEXT (act_args);
    }

    DBUG_RETURN (ID_AVIS (EXPRS_EXPR (act_args)));
}

/** <!--********************************************************************-->
 *
 * @fn node * CreateTmpVar( char *name, info *arg_info)
 *
 *****************************************************************************/

static node *
CreateTmpVar (char *name, info *arg_info)
{
    node *avis;

    DBUG_ENTER ();

    avis = TBmakeAvis (TRAVtmpVarName (name), TYmakeAUD (TYmakeSimpleType (T_unknown)));
    INFO_VARDECS (arg_info) = TBmakeVardec (avis, INFO_VARDECS (arg_info));

    DBUG_RETURN (avis);
}

/** <!--********************************************************************-->
 *
 * @fn node * CreateLetAssign( node *avis, node *rhs, node *next)
 *
 *****************************************************************************/

static node *
CreateLetAssign (node *avis, node *rhs, node *next)
{
    DBUG_ENTER ();

    DBUG_RETURN (TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), rhs), next));
}

/** <!--********************************************************************-->
 *
 * @fn node *CreateLacFunCallAssignments( node *ap, node *pred_avis,
 *            node *result_avis, node *shape_avis, node *idx_avis,
 *            info *arg_info)
 *
 *****************************************************************************/

static node *
CreateLacFunCallAssignments (node *ap, node *pred_avis, node *result_avis,
                             node *shape_avis, node *idx_avis, info *arg_info)
{
    node *assigns = NULL;
    node *exprs, *arg_avis, *new_arg_avis, *new_arg_expr, *loc_args;
    node *arg, *args = NULL, *act_args = NULL;
    node *fundef, *result_expr;
    DBUG_ENTER ();

    /**
     * First, we compute all new_args:
     *
     * iff shape != NULL we create:
     *
     *    arg1' = adjustLacFunParams( pred, arg1, idx);
     *    ...
     *    argn' = adjustLacFunParams( pred, argn, idx);
     *
     * otherwise, we create:
     *
     *    arg1' = adjustLacFunParamsReshape( pred, arg1, idx, shape);
     *    ...
     *    argn' = adjustLacFunParamsReshape( pred, argn, idx, shape);
     *
     * For the predicate, we generate:
     *
     *    pred' = _sel_( idx, pred);
     */
    exprs = AP_ARGS (ap);

    while (exprs != NULL) {
        arg_avis = ID_AVIS (EXPRS_EXPR (exprs));

        new_arg_avis = CreateTmpVar ("arg", arg_info);

        if (arg_avis == pred_avis) {
            new_arg_expr
              = TCmakePrf2 (F_sel_VxA, TBmakeId (idx_avis), TBmakeId (pred_avis));
            assigns = CreateLetAssign (new_arg_avis, new_arg_expr, assigns);

        } else {
            if (shape_avis != NULL) {
                loc_args
                  = TBmakeExprs (TBmakeId (pred_avis),
                                 TBmakeExprs (TBmakeId (arg_avis),
                                              TBmakeExprs (TBmakeId (idx_avis),
                                                           TBmakeExprs (TBmakeId (
                                                                          shape_avis),
                                                                        NULL))));
                fundef = DSdispatchFunCall (NSgetNamespace (global.preludename),
                                            "adjustLacFunParamsReshape", loc_args);
                DBUG_ASSERT (fundef != NULL, "%s::adjustLacFunParamsReshape not found",
                             global.preludename);

                assigns = CreateLetAssign (new_arg_avis, fundef, assigns);
            } else {
                loc_args
                  = TBmakeExprs (TBmakeId (pred_avis),
                                 TBmakeExprs (TBmakeId (arg_avis),
                                              TBmakeExprs (TBmakeId (idx_avis), NULL)));
                fundef = DSdispatchFunCall (NSgetNamespace (global.preludename),
                                            "adjustLacFunParams", loc_args);
                DBUG_ASSERT (fundef != NULL, "%s::adjustLacFunParams not found",
                             global.preludename);

                assigns = CreateLetAssign (new_arg_avis, fundef, assigns);
            }
        }

        arg = TBmakeExprs (TBmakeId (new_arg_avis), NULL);

        if (args == NULL) {
            args = arg;
            act_args = args;
        } else {
            EXPRS_NEXT (act_args) = arg;
            act_args = arg;
        }

        exprs = EXPRS_NEXT (exprs);
    }

    /**
     * Eventually, we create the LaCfun call:
     *
     * result = LaCfun( pred', arg1', ..., argn');
     */
    result_expr = DUPdoDupTree (ap);
    AP_ARGS (result_expr) = FREEdoFreeTree (AP_ARGS (result_expr));
    AP_ARGS (result_expr) = args;

    assigns = TCappendAssign (assigns, CreateLetAssign (result_avis, result_expr, NULL));

    DBUG_RETURN (assigns);
}

/** <!--********************************************************************-->
 *
 * @fn node *CreateWithLoop( node *ap, info *arg_info)
 *
 *****************************************************************************/

static node *
CreateWithLoop (node *ap, info *arg_info)
{
    node *with, *pred_avis, *shape_avis, *idx_avis, *idx_expr, *shape_expr;
    node *default_avis, *default_expr_ass, *iv_avis, *code_avis, *code_expr;
    node *code_expr_ass;

    DBUG_ENTER ();

    pred_avis = SearchPredicate (ap);

    /**
     *  First we compute a few tmps needed throughout:
     *
     *  shape = _shape_( pred);
     *  idx = 0*shape;
     */
    shape_avis = CreateTmpVar ("shape", arg_info);
    idx_avis = CreateTmpVar ("idx", arg_info);

    shape_expr = TCmakePrf1 (F_shape_A, TBmakeId (pred_avis));
    idx_expr = TCmakePrf2 (F_mul_SxV, TBmakeNum (0), TBmakeId (shape_avis));

    INFO_ASSIGNS (arg_info)
      = CreateLetAssign (idx_avis, idx_expr, INFO_ASSIGNS (arg_info));
    INFO_ASSIGNS (arg_info)
      = CreateLetAssign (shape_avis, shape_expr, INFO_ASSIGNS (arg_info));

    /**
     * Now, we create the assignment chain that computes the default expression:
     *
     * arg1' = adjustLacFunParams( pred, arg1, idx);
     * ...
     * argn' = adjustLacFunParams( pred, argn, idx);
     * pred' = _sel_( idx, p);
     * default = LaCfun( pred', arg1', ..., argn');
     */
    default_avis = CreateTmpVar ("default", arg_info);
    default_expr_ass = CreateLacFunCallAssignments (ap, pred_avis, default_avis, NULL,
                                                    idx_avis, arg_info);

    INFO_ASSIGNS (arg_info) = TCappendAssign (INFO_ASSIGNS (arg_info), default_expr_ass);

    /**
     * Similarily, we create the body of the WL:
     *
     * arg1' = adjustLacFunParamsReshape( pred, arg1, iv, shape);
     * ...
     * argn' = adjustLacFunParamsReshape( pred, argn, iv, shape);
     * pred' = _sel_( iv, p);
     * code = LaCfun( pred', arg1', ..., argn');
     */

    iv_avis = CreateTmpVar ("iv", arg_info);
    code_avis = CreateTmpVar ("code", arg_info);
    code_expr_ass = CreateLacFunCallAssignments (ap, pred_avis, code_avis, shape_avis,
                                                 iv_avis, arg_info);

    /**
     * Eventually, we create the WL:
     *
     * with {
     *   ( idx <= iv < shape) : {
     *     <code_expr_ass>          // see above!
     *   } : code;
     * } : genarray( shape, default);
     */
    code_expr = TBmakeCode (TBmakeBlock (code_expr_ass, NULL),
                            TBmakeExprs (TBmakeId (code_avis), NULL));
    CODE_USED (code_expr)++;

    with
      = TBmakeWith (TBmakePart (code_expr, TBmakeWithid (TBmakeIds (iv_avis, NULL), NULL),
                                TBmakeGenerator (F_wl_le, F_wl_lt, TBmakeId (idx_avis),
                                                 TBmakeId (shape_avis), NULL, NULL)),
                    code_expr,
                    TBmakeGenarray (TBmakeId (shape_avis), TBmakeId (default_avis)));
    DBUG_RETURN (with);
}

/** <!--********************************************************************-->
 *
 * @fn node *ELFdoExtendLacFuns( node *arg_node)
 *
 *****************************************************************************/

node *
ELFdoExtendLacFuns (node *arg_node)
{
    info *info_node;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_module,
                 "EBTdoEliminateBottomTypes can be called on N_module only!");

    TRAVpush (TR_elf);

    info_node = MakeInfo ();
    DSinitDeserialize (arg_node);
    arg_node = TRAVdo (arg_node, info_node);
    DSfinishDeserialize (arg_node);
    info_node = FreeInfo (info_node);

    TRAVpop ();

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ELFfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @return
 *
 *****************************************************************************/
node *
ELFfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (!NSequals (FUNDEF_NS (arg_node), NSgetNamespace (global.preludename))) {
        FUNDEF_BODY (arg_node) = TRAVopt(FUNDEF_BODY (arg_node), arg_info);

        if (INFO_VARDECS (arg_info) != NULL) {
            BLOCK_VARDECS (FUNDEF_BODY (arg_node))
              = TCappendVardec (INFO_VARDECS (arg_info),
                                BLOCK_VARDECS (FUNDEF_BODY (arg_node)));
            INFO_VARDECS (arg_info) = NULL;
        }
    }

    FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ELFassign( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @return
 *
 *****************************************************************************/
node *
ELFassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt(ASSIGN_NEXT (arg_node), arg_info);
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    if (INFO_ASSIGNS (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_ASSIGNS (arg_info), arg_node);
        INFO_ASSIGNS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ELFap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @return
 *
 *****************************************************************************/
node *
ELFap (node *arg_node, info *arg_info)
{
    node *with;

    DBUG_ENTER ();

    if (FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node))) {
        with = CreateWithLoop (arg_node, arg_info);
        arg_node = FREEdoFreeTree (arg_node);
        arg_node = with;
    } else if (FUNDEF_ISLOOPFUN (AP_FUNDEF (arg_node))
               && !AP_ISRECURSIVEDOFUNCALL (arg_node)) {
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
