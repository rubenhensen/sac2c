/*
 * $Id$
 */

/**
 *
 * @file ExplicitAccumulate.c
 *
 * In this traversal fold functions of fold withloops
 * are become explicit and a new function F_accu is inserted
 * in code.
 *
 * Ex.:
 *    A = with(iv)
 *          gen:{ val = ...;
 *              }: val
 *        fold( op, n);
 *
 * is transformed into
 *
 *    A = with(iv)
 *          gen:{ acc   = accu( iv);
 *                val = ...;
 *                res = op( acc, val);
 *              }: res
 *        fold( op, n);
 *
 * The function F_accu is used to get the correct
 * accumulation value but is only pseudo syntax and is kicked
 * off the code in compile. The only argument is the index vector
 * of the surrounding withloop to disable LIR.
 *
 *
 * Furthermore, we make the fix-break comparison (if present) explicit.
 * I.e., we introduce a variable new_s of type bool[] which replaces
 * the CODE_GUARD value. It is computed by using sacprelude::eq
 *
 * Ex.:
 *    A = with(iv)
 *          gen:{ val = ...;
 *              }: val break s;
 *        fold( op, n);
 *
 * is transformed into
 *
 *    A = with(iv)
 *          gen:{ acc   = accu( iv);
 *                val = ...;
 *                res = op( acc, val);
 *                new_s = sacprelude::eq( res, s);
 *              }: res break new_s;
 *        fold( op, n );
 *
 * NB: All this needs to happen correctly even if the fold operator is
 * "surrounded" by propagate operators! Therefore we have to be carefull
 * when dealing with FOLD_LHS and FOLD_CEXPR! (cf.bug 376!)
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "new_types.h"
#include "tree_basic.h"
#include "node_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "DupTree.h"
#include "globals.h"

#define DBUG_PREFIX "EA"
#include "debug.h"

#include "traverse.h"
#include "constants.h"
#include "shape.h"
#include "deserialize.h"
#include "namespaces.h"
#include "ExplicitAccumulate.h"

/**
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *wl;
    node *fold;
    node *fold_ids;
    node *accu;
    node *guard;
    node *cexprs;
    node *expr;
    node *ids;
};

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_WL(n) (n->wl)
#define INFO_FOLD(n) (n->fold)
#define INFO_FOLD_LHS(n) (n->fold_ids)
#define INFO_FOLD_ACCU(n) (n->accu)
#define INFO_FOLD_GUARD(n) (n->guard)
#define INFO_FOLD_CEXPR(n) (n->expr)
#define INFO_CEXPRS(n) (n->cexprs)
#define INFO_LHS_IDS(n) (n->ids)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_WL (result) = NULL;
    INFO_FOLD (result) = NULL;
    INFO_FOLD_LHS (result) = NULL;
    INFO_FOLD_ACCU (result) = NULL;
    INFO_FOLD_GUARD (result) = NULL;
    INFO_FOLD_CEXPR (result) = NULL;
    INFO_LHS_IDS (result) = NULL;

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
 * @fn node *MakeAccuAssign( node *code, info *arg_info)
 *
 *   @brief creates new assignment containing prf F_accu
 *
 *   @param  node *code     :  code block
 *           info *arg_info :  context info
 *   @return node *         :  updated code node
 ******************************************************************************/
static node *
MakeAccuAssign (node *code, info *arg_info)
{
    node *lhs_ids, *avis, *assign;

    DBUG_ENTER ();

    /* grab assign and get rid of N_empty nodes if one is there */
    assign = BLOCK_INSTR (CODE_CBLOCK (code));
    if (NODE_TYPE (assign) == N_empty) {
        assign = FREEdoFreeNode (assign);
    }

    /* create avis */
    lhs_ids = INFO_FOLD_LHS (arg_info);
    INFO_FOLD_LHS (arg_info) = NULL;
    avis = TBmakeAvis (TRAVtmpVarName (IDS_NAME (lhs_ids)),
                       TYeliminateAKV (AVIS_TYPE (IDS_AVIS (lhs_ids))));
    INFO_FOLD_ACCU (arg_info) = avis;

    /* insert vardec */
    FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
      = TBmakeVardec (avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

    /* create <avis> = F_accu( <idx-varname>) */

    assign = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL),
                                      TCmakePrf1 (F_accu, DUPdupIdsId (WITH_VEC (
                                                            INFO_WL (arg_info))))),
                           assign);

    /* set correct backref to defining assignment */
    AVIS_SSAASSIGN (avis) = assign;

    /* put back assign */
    BLOCK_INSTR (CODE_CBLOCK (code)) = assign;

    DBUG_RETURN (code);
}

/** <!--********************************************************************-->
 *
 * @fn node *MakeFoldFunAssign( info *arg_info);
 *
 *   @brief  creates new assignment containing fold function of withloop
 *
 *   @param  info *arg_info  :  fold WL context
 *   @return node *          :  new N_assign node
 ******************************************************************************/
static node *
MakeFoldFunAssign (info *arg_info)
{
    node *old_cexpr_id, *avis, *assign, *args, *eq_funap, *fixassign;

    DBUG_ENTER ();

    /*
     * create a function application of the form:
     *    <cexpr'> = <fun>( <acc>, <cexpr>);
     * where
     *    <acc>   is the accumulator variable
     *    <fun>   is the name of the (artificially introduced) folding-fun
     *    <cexpr> is the expression in the operation part
     */

    /* create new cexpr id: */
    old_cexpr_id = EXPRS_EXPR1 (INFO_FOLD_CEXPR (arg_info));
    avis = TBmakeAvis (TRAVtmpVarName (ID_NAME (old_cexpr_id)),
                       TYcopyType (AVIS_TYPE (INFO_FOLD_ACCU (arg_info))));

    /* replace old_cexpr_id with new one: */
    EXPRS_EXPR1 (INFO_FOLD_CEXPR (arg_info)) = TBmakeId (avis);

    /* insert vardec */
    FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
      = TBmakeVardec (avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

    /* create <avis> = <fun>( <accu>, old_cexpr_id); */

    assign = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL),
                                      TCmakeAp2 (FOLD_FUNDEF (INFO_FOLD (arg_info)),
                                                 TBmakeId (INFO_FOLD_ACCU (arg_info)),
                                                 old_cexpr_id)),
                           NULL);

    /* set correct backref to defining assignment */
    AVIS_SSAASSIGN (avis) = assign;

    if (INFO_FOLD_GUARD (arg_info) != NULL) {
        /*
         * create an assignment <fixbool> = sacprelude::eq( <avis>, fixval);
         */
        args
          = TBmakeExprs (TBmakeId (avis),
                         TBmakeExprs (DUPdoDupNode (INFO_FOLD_GUARD (arg_info)), NULL));

        eq_funap = DSdispatchFunCall (NSgetNamespace (global.preludename), "eq", args);
        DBUG_ASSERT (eq_funap != NULL, "%s::eq not found", global.preludename);

        avis = TBmakeAvis (TRAVtmpVarName (ID_NAME (INFO_FOLD_GUARD (arg_info))),
                           TYmakeAKS (TYmakeSimpleType (T_bool), SHmakeShape (0)));

        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = TBmakeVardec (avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

        fixassign = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), eq_funap), NULL);

        AVIS_SSAASSIGN (avis) = fixassign;
        ASSIGN_NEXT (assign) = fixassign;

        EXPRS_NEXT (INFO_FOLD_CEXPR (arg_info))
          = TBmakeExprs (TBmakeId (avis), EXPRS_NEXT (INFO_FOLD_CEXPR (arg_info)));
    }

    DBUG_RETURN (assign);
}

/**
 *
 *  TRAVERSAL FUNCTIONS
 *
 * @{
 ****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *EAmodule(node *arg_node, info *arg_info)
 *
 *   @brief traverses function definitions only!
 *
 *   @param  node *arg_node:  N_module
 *           info *arg_info:  info
 *   @return node *        :  N_module
 ******************************************************************************/

node *
EAmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DSinitDeserialize (arg_node);
    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }
    DSfinishDeserialize (arg_node);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EAfundef(node *arg_node, info *arg_info)
 *
 *   @brief  Traverses FUNDEF body
 *
 *   @param  node *arg_node:  N_fundef
 *           info *arg_info:  info
 *   @return node *        :  N_fundef
 ******************************************************************************/

node *
EAfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node)) {
        FUNDEF_INSTR (arg_node) = TRAVdo (FUNDEF_INSTR (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EAassign(node *arg_node, info *arg_info)
 *
 *   @brief  Traverses in expression
 *
 *   @param  node *arg_node:  N_assign
 *           info *arg_info:  info
 *   @return node *        :  N_assign
 ******************************************************************************/

node *
EAassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    } else {
        if (INFO_FOLD_ACCU (arg_info) != NULL) {
            ASSIGN_NEXT (arg_node) = MakeFoldFunAssign (arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EAlet(node *arg_node, info *arg_info)
 *
 *   @brief  Traverses in expression
 *
 *   @param  node *arg_node:  N_let
 *           info *arg_info:  info
 *   @return node *        :  N_let
 ******************************************************************************/

node *
EAlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS_IDS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EAwith(node *arg_node, info *arg_info)
 *
 *   @brief  if current WL is a fold WL modify code.
 *
 *   @param  node *arg_node:  N_with
 *           info *arg_info:  N_info
 *   @return node *        :  N_with
 ******************************************************************************/

node *
EAwith (node *arg_node, info *arg_info)
{
    info *tmp;

    DBUG_ENTER ();

    /* stack arg_info */
    tmp = arg_info;
    arg_info = MakeInfo ();
    INFO_FUNDEF (arg_info) = INFO_FUNDEF (tmp);
    INFO_LHS_IDS (arg_info) = INFO_LHS_IDS (tmp);
    INFO_WL (arg_info) = arg_node;
    INFO_CEXPRS (arg_info) = CODE_CEXPRS (WITH_CODE (arg_node));

    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    INFO_CEXPRS (arg_info) = NULL;

    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    if (INFO_FOLD_GUARD (arg_info) != NULL) {
        INFO_FOLD_GUARD (arg_info) = FREEdoFreeNode (INFO_FOLD_GUARD (arg_info));
    }

    /* pop arg_info */
    arg_info = FreeInfo (arg_info);
    arg_info = tmp;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EApropagate(node *arg_node, info *arg_info)
 *
 *   @brief  modify code.
 *
 *   @param  node *arg_node:  N_propagate
 *           info *arg_info:  N_info
 *   @return node *        :  N_propagate
 ******************************************************************************/

node *
EApropagate (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_CEXPRS (arg_info) = EXPRS_NEXT (INFO_CEXPRS (arg_info));
    INFO_LHS_IDS (arg_info) = IDS_NEXT (INFO_LHS_IDS (arg_info));
    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EAfold(node *arg_node, info *arg_info)
 *
 *   @brief  modify code.
 *
 *   @param  node *arg_node:  N_fold
 *           info *arg_info:  N_info
 *   @return node *        :  N_fold
 ******************************************************************************/

node *
EAfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Fold WL found, inserting F_Accu...");

    INFO_FOLD (arg_info) = arg_node;
    INFO_FOLD_LHS (arg_info) = INFO_LHS_IDS (arg_info);
    INFO_FOLD_CEXPR (arg_info) = INFO_CEXPRS (arg_info);

    if (FOLD_GUARD (arg_node) != NULL) {
        node *avis;
        node *brk;
        node *ids;

        /*
         * Transcribe Guard into info node and remove guard from fold
         */
        INFO_FOLD_GUARD (arg_info) = FOLD_GUARD (arg_node);
        FOLD_GUARD (arg_node) = NULL;

        /*
         * Append break withop
         */
        brk = TBmakeBreak ();
        BREAK_NEXT (brk) = FOLD_NEXT (arg_node);
        FOLD_NEXT (arg_node) = brk;

        /*
         * Append IDS to LHS of assignment
         */
        avis = TBmakeAvis (TRAVtmpVarName (ID_NAME (INFO_FOLD_GUARD (arg_info))),
                           TYmakeAKS (TYmakeSimpleType (T_bool), SHmakeShape (0)));

        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = TBmakeVardec (avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

        AVIS_SSAASSIGN (avis) = AVIS_SSAASSIGN (IDS_AVIS (INFO_LHS_IDS (arg_info)));

        ids = TBmakeIds (avis, NULL);
        IDS_NEXT (ids) = IDS_NEXT (INFO_LHS_IDS (arg_info));
        IDS_NEXT (INFO_LHS_IDS (arg_info)) = ids;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EAcode(node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param  node *arg_node:  N_code
 *           info *arg_info:  N_info
 *   @return node *        :  N_code
 ******************************************************************************/

node *
EAcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_FOLD_LHS (arg_info) != NULL) {
        arg_node = MakeAccuAssign (arg_node, arg_info);
    }

    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);

    DBUG_ASSERT (CODE_NEXT (arg_node) == NULL, "cannot handle multi generator WLs");

    DBUG_RETURN (arg_node);
}

/**
 * @}
 */

/** <!--********************************************************************-->
 *
 * @fn node *ExplicitAccumulate( node *arg_node)
 *
 *   @brief  Starting function of ExplicitAccumulate traversal
 *
 *   @param  node *arg_node:  the whole syntax tree
 *   @return node *        :  the transformed syntax tree
 ******************************************************************************/

node *
EAdoExplicitAccumulate (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_module,
                 "ExplicitAccumulate not started with module node");

    DBUG_PRINT ("starting ExplicitAccumulation");

    arg_info = MakeInfo ();

    TRAVpush (TR_ea);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
