/*
 * $Id$
 */

/**
 *
 * @file UndoElimSubDiv.c
 *
 * @brief Replaces all occurences of primitive negation and reciprocal operators
 *        introduced by ElimSubDiv with subtraction and division.
 *
 * More precisely, we look for the following patterns:
 *
 *   a + -b  =>  a - b
 *  -a +  b  =>  b - a
 *  -a + -b => -a - b
 *
 *   a * /b  =>  a / b
 *  /a *  b  =>  b / a
 *  /a * /b =>  /a / b
 *
 *  /a      =>  1 / a
 *
 *  Whereas all occurrences of F_reciproc are eliminated, we leave F_neg
 *  in the code because it is supported by the backend.
 *
 * FIXME: Let's assume that it is now the future, and remove this stuff.
 *
 *  Since both neg and reciproc are standard prfs, why do we remove them
 *  at all during the optimisation cycle rather than keeping them until
 *  the end? The main reason is that legacy optimisation, e.g. with-loop
 *  folding or any form of array indexing analysis, look for patterns
 *  like iv - x, but hardly for iv + -x. If this changes in the future
 *  or is shown to be irrelevant, then we could move UndoElimSubDiv to
 *  the end of the optimisation phase.
 */

#include <stdio.h>
#include <stdlib.h>

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "tree_basic.h"
#include "node_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "new_types.h"
#include "shape.h"
#include "memory.h"
#include "free.h"
#include "DupTree.h"

#include "UndoElimSubDiv.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    bool onefundef;
    node *preassign;
    node *lhs;
    bool topdown;
};

/*
 * INFO macros
 */
#define INFO_ONEFUNDEF(n) (n->onefundef)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_PREASSIGN(n) (n->preassign)
#define INFO_LHS(n) (n->lhs)
#define INFO_TOPDOWN(n) (n->topdown)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_PREASSIGN (result) = NULL;
    INFO_LHS (result) = NULL;

    INFO_TOPDOWN (result) = TRUE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/**<!--****************************************************************-->
 *
 * @fn static node *CheckExpr(node *expr, prf op)
 *
 * @brief checks if definiton of argument fullfils special conditions
 *        (is an assignment with primitive neg or reciproc
 *
 * @param expr argument of primitive operation
 * @param op current prf
 *
 * @return
 *
 ************************************************************************/
static node *
CheckExpr (node *expr, prf op)
{
    node *result;
    prf prfop1, prfop2;

    DBUG_ENTER ();

    if ((N_id == NODE_TYPE (expr)) && (AVIS_SSAASSIGN (ID_AVIS (expr)) != NULL)) {

        node *assign = AVIS_SSAASSIGN (ID_AVIS (expr));

        switch (op) {
        case F_add_SxS:
        case F_add_VxS:
        case F_add_SxV:
        case F_add_VxV:
            prfop1 = F_neg_S;
            prfop2 = F_neg_V;
            break;
        case F_mul_SxS:
        case F_mul_VxS:
        case F_mul_SxV:
        case F_mul_VxV:
            prfop1 = F_reciproc_S;
            prfop2 = F_reciproc_V;
            break;
        default:
            prfop1 = F_unknown;
            prfop2 = F_unknown;
        }

        if ((prfop1 != F_unknown) && (N_let == NODE_TYPE (ASSIGN_INSTR (assign)))
            && (N_prf == NODE_TYPE (LET_EXPR (ASSIGN_INSTR (assign))))
            && ((PRF_PRF (LET_EXPR (ASSIGN_INSTR (assign))) == prfop1)
                || (PRF_PRF (LET_EXPR (ASSIGN_INSTR (assign))) == prfop2))) {
            result = PRF_ARG1 (LET_EXPR (ASSIGN_INSTR (assign)));
        } else {
            result = NULL;
        }
    } else {
        result = NULL;
    }

    DBUG_RETURN (result);
}

/**<!--****************************************************************-->
 *
 * @fn static prf TogglePrf(prf op)
 *
 * @brief returns opposite primitive operator of op
 *
 * @param op primitive operator
 *
 * @return opposite primitive operator of op
 *
 ************************************************************************/
static prf
TogglePrf (prf op)
{
    prf result = F_unknown;

    DBUG_ENTER ();

    switch (op) {
    case F_add_SxS:
        result = F_sub_SxS;
        break;

    case F_add_SxV:
        result = F_sub_SxV;
        break;

    case F_add_VxS:
        result = F_sub_VxS;
        break;

    case F_add_VxV:
        result = F_sub_VxV;
        break;

    case F_mul_SxS:
        result = F_div_SxS;
        break;

    case F_mul_SxV:
        result = F_div_SxV;
        break;

    case F_mul_VxS:
        result = F_div_VxS;
        break;

    case F_mul_VxV:
        result = F_div_VxV;
        break;

    default:
        DBUG_ASSERT (0, "Illegal argument prf!");
    }

    DBUG_RETURN (result);
}

/**<!--****************************************************************-->
 *
 * @fn static prf TogglePrfSwap(prf op)
 *
 * @brief returns opposite primitive operator of op taking into account
 *        that also the order of arguments is swapped.
 *
 * @param op primitive operator
 *
 * @return opposite primitive operator of op
 *
 ************************************************************************/
static prf
TogglePrfSwap (prf op)
{
    prf result = F_unknown;

    DBUG_ENTER ();

    switch (op) {
    case F_add_SxS:
        result = F_sub_SxS;
        break;

    case F_add_SxV:
        result = F_sub_VxS;
        break;

    case F_add_VxS:
        result = F_sub_SxV;
        break;

    case F_add_VxV:
        result = F_sub_VxV;
        break;

    case F_mul_SxS:
        result = F_div_SxS;
        break;

    case F_mul_SxV:
        result = F_div_VxS;
        break;

    case F_mul_VxS:
        result = F_div_SxV;
        break;

    case F_mul_VxV:
        result = F_div_VxV;
        break;

    default:
        DBUG_ASSERT (0, "Illegal argument prf!");
    }

    DBUG_RETURN (result);
}

/**<!--****************************************************************-->
 *
 * @fn node *UESDdoUndoElimSubDivModule(node *arg_node)
 *
 * @brief starting function of UndoElimSubDiv for modules
 *
 * @param fundef
 *
 * @return
 *
 ************************************************************************/
node *
UESDdoUndoElimSubDivModule (node *arg_node)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_module, "WLI called on non-N_module node");

    INFO_ONEFUNDEF (info) = FALSE;

    TRAVpush (TR_uesd);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

/**<!--****************************************************************-->
 *
 * @fn node *UESDdoUndoElimSubDiv(node *arg_node)
 *
 * @brief starting function of UndoElimSubDiv
 *
 * @param fundef
 *
 * @return
 *
 ************************************************************************/
node *
UESDdoUndoElimSubDiv (node *arg_node)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef, "WLI called on nonN_fundef node");

    INFO_ONEFUNDEF (info) = TRUE;

    TRAVpush (TR_uesd);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *UESDfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
UESDfundef (node *arg_node, info *arg_info)
{
    bool old_onefundef;

    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    old_onefundef = INFO_ONEFUNDEF (arg_info);
    INFO_ONEFUNDEF (arg_info) = FALSE;
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    INFO_ONEFUNDEF (arg_info) = old_onefundef;

    if (!INFO_ONEFUNDEF (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**<!--****************************************************************-->
 *
 * @fn UESDblock(node *arg_node, info *arg_info)
 *
 * @brief traverses in instructions
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 ************************************************************************/
node *
UESDblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    BLOCK_INSTR (arg_node) = TRAVopt (BLOCK_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**<!--****************************************************************-->
 *
 * @fn UESDassign(node *arg_node, info *arg_info)
 *
 * @brief traverse in instruction and insert new assignments
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 ************************************************************************/
node *
UESDassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_TOPDOWN (arg_info) = TRUE;

    ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    INFO_TOPDOWN (arg_info) = FALSE;

    ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_PREASSIGN (arg_info) != NULL) {
        ASSIGN_NEXT (INFO_PREASSIGN (arg_info)) = arg_node;
        arg_node = INFO_PREASSIGN (arg_info);
        INFO_PREASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/**<!--****************************************************************-->
 *
 * @fn UESDlet(node *arg_node, info *arg_info)
 *
 * @brief traverse in expr
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 ************************************************************************/
node *
UESDlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);

    INFO_LHS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/**<!--****************************************************************-->
 *
 * @fn UESDprf(node *arg_node, info *arg_info)
 *
 * @brief replace negation and reciprocal operator
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 ************************************************************************/

node *
UESDprf (node *arg_node, info *arg_info)
{
    prf op;
    node *id1, *id2;

    DBUG_ENTER ();

    op = PRF_PRF (arg_node);

    if (INFO_TOPDOWN (arg_info)) {
        /*
         * top-down traversal
         */

        switch (op) {
        case F_add_SxS:
        case F_add_VxS:
        case F_add_SxV:
        case F_add_VxV:
        case F_mul_SxS:
        case F_mul_VxS:
        case F_mul_SxV:
        case F_mul_VxV:
            id1 = CheckExpr (EXPRS_EXPR (PRF_ARGS (arg_node)), op);
            id2 = CheckExpr (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))), op);

            if (id2 != NULL) {
                PRF_ARG2 (arg_node) = FREEdoFreeTree (PRF_ARG2 (arg_node));
                PRF_ARG2 (arg_node) = DUPdoDupTree (id2);
                PRF_PRF (arg_node) = TogglePrf (op);
            } else if (id1 != NULL) {
                PRF_ARG1 (arg_node) = FREEdoFreeTree (PRF_ARG1 (arg_node));
                PRF_ARG1 (arg_node) = PRF_ARG2 (arg_node);
                PRF_ARG2 (arg_node) = DUPdoDupTree (id1);
                PRF_PRF (arg_node) = TogglePrfSwap (op);
            }
        default:
            break;
        }
    } else {
        /*
         * bottom-up traversal
         */
        simpletype stype;
        node *avis, *exp;

        if ((op == F_reciproc_S) || (op == F_reciproc_V)) {
            stype = TYgetSimpleType (
              TYgetScalar (AVIS_TYPE (IDS_AVIS (INFO_LHS (arg_info)))));
            avis = TBmakeAvis (TRAVtmpVar (),
                               TYmakeAKS (TYmakeSimpleType (stype), SHmakeShape (0)));
            PRF_ARGS (arg_node) = TBmakeExprs (TBmakeId (avis), PRF_ARGS (arg_node));
            PRF_PRF (arg_node) = (op == F_reciproc_S) ? F_div_SxS : F_div_SxV;

            switch (stype) {
            case T_float:
                exp = TBmakeFloat (1.0);
                break;
            case T_double:
                exp = TBmakeDouble (1.0);
                break;
            default:
                exp = NULL;
                DBUG_ASSERT (FALSE, "We should never reach here.");
            }
            INFO_PREASSIGN (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), exp), NULL);
            FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
              = TBmakeVardec (avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));
        }
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
