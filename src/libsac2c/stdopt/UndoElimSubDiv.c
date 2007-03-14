/*
 * $Id$
 */

/**
 *
 * @file UndoElimSubDiv.c
 *
 * @brief Replaces all occurences of primitive negation and reciprocal operators
 *        introduced by ElimSubDiv with substraction and division.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "tree_basic.h"
#include "node_basic.h"
#include "globals.h"
#include "tree_compound.h"
#include "traverse.h"
#include "new_types.h"
#include "dbug.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "DataFlowMask.h"
#include "DupTree.h"
#include "SSATransform.h"

#include "UndoElimSubDiv.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *postassign;
    node *let;
    bool topdown;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_POSTASSIGN(n) (n->postassign)
#define INFO_LET(n) (n->let)
#define INFO_TOPDOWN(n) (n->topdown)
/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_POSTASSIGN (result) = NULL;
    INFO_LET (result) = NULL;
    INFO_TOPDOWN (result) = TRUE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/**<!--****************************************************************-->
 *
 * @fn static node *CheckExpr(node *expr, prf op)
 *
 * @brief checks if definiton of argument fullfils special conditions
 *        (is an assignment with  primitive of type esd_rec or esd_neg)
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
    node *result = NULL;

    DBUG_ENTER ("CheckExpr");

    if ((N_id == NODE_TYPE (expr)) && (AVIS_SSAASSIGN (ID_AVIS (expr)) != NULL)) {

        node *assign = AVIS_SSAASSIGN (ID_AVIS (expr));
        prf prfop = F_noop;

        switch (op) {
        case F_add_SxS:
        case F_add_AxS:
        case F_add_SxA:
        case F_add_AxA:
            prfop = F_esd_neg;
            break;
        case F_mul_SxS:
        case F_mul_AxS:
        case F_mul_SxA:
        case F_mul_AxA:
            prfop = F_esd_rec;
            break;
        default:
            break;
        }

        if ((prfop != F_noop) && (N_let == NODE_TYPE (ASSIGN_INSTR (assign)))
            && (N_prf == NODE_TYPE (LET_EXPR (ASSIGN_INSTR (assign))))) {

            if (PRF_PRF (LET_EXPR (ASSIGN_INSTR (assign))) == prfop) {
                result = PRF_ARGS (LET_EXPR (ASSIGN_INSTR (assign)));
            }
        }
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
    prf result = F_noop;
    DBUG_ENTER ("TogglePrf");

    switch (op) {
    case F_add_SxS:
        result = F_sub_SxS;
        break;

    case F_add_SxA:
        result = F_sub_SxA;
        break;

    case F_add_AxS:
        result = F_sub_AxS;
        break;

    case F_add_AxA:
        result = F_sub_AxA;
        break;

    case F_mul_SxS:
        result = F_div_SxS;
        break;

    case F_mul_SxA:
        result = F_div_SxA;
        break;

    case F_mul_AxS:
        result = F_div_AxS;
        break;

    case F_mul_AxA:
        result = F_div_AxA;
        break;

    default:
        DBUG_ASSERT ((0), "Illegal argument prf!");
    }

    DBUG_RETURN (result);
}

/**<!--****************************************************************-->
 *
 * @fn node *UESDdoUndoElimSubDiv(node *module)
 *
 * @brief startimng function of UndoElimSubDiv
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

    DBUG_ENTER ("UESDdoUndoElimSubDiv");

    info = MakeInfo ();

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
    DBUG_ENTER ("UESDfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
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
    DBUG_ENTER ("UESDblock");

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

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
    node *postassign = NULL;

    DBUG_ENTER ("UESDassign");

    INFO_TOPDOWN (arg_info) = TRUE;
    INFO_POSTASSIGN (arg_info) = NULL;

    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }

    postassign = INFO_POSTASSIGN (arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    INFO_TOPDOWN (arg_info) = FALSE;

    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }

    if (postassign != NULL) {
        ASSIGN_NEXT (postassign) = arg_node;
        arg_node = postassign;
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
    DBUG_ENTER ("UESDlet");

    INFO_LET (arg_info) = arg_node;

    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

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
    prf op, newop;
    node *id1, *id2;
    bool add = FALSE;
    ntype *type;

    DBUG_ENTER ("UESDprf");

    op = PRF_PRF (arg_node);

    if (INFO_TOPDOWN (arg_info)) {
        switch (op) {

        case F_add_SxS:
        case F_add_AxS:
        case F_add_SxA:
        case F_add_AxA:
            add = TRUE;

        case F_mul_SxS:
        case F_mul_AxS:
        case F_mul_SxA:
        case F_mul_AxA:
            /*
             * handel add and div seperatly
             */
            id1 = CheckExpr (EXPRS_EXPR (PRF_ARGS (arg_node)), op);
            id2 = CheckExpr (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))), op);

            if ((id1 == NULL) && (id2 == NULL)) {
                /*
                 * nothing to do
                 */
            } else if ((id1 == NULL) && (id2 != NULL)) {
                EXPRS_NEXT (PRF_ARGS (arg_node))
                  = FREEdoFreeNode (EXPRS_NEXT (PRF_ARGS (arg_node)));
                EXPRS_NEXT (PRF_ARGS (arg_node)) = DUPdoDupTree (id2);
                PRF_PRF (arg_node) = TogglePrf (op);

            } else if ((id1 != NULL) && (id2 == NULL)) {
                node *tmp = EXPRS_NEXT (PRF_ARGS (arg_node));
                EXPRS_NEXT (PRF_ARGS (arg_node)) = NULL;
                PRF_ARGS (arg_node) = FREEdoFreeNode (PRF_ARGS (arg_node));
                PRF_ARGS (arg_node) = tmp;
                EXPRS_NEXT (PRF_ARGS (arg_node)) = DUPdoDupTree (id1);
                PRF_PRF (arg_node) = TogglePrf (op);
            } else if ((id1 != NULL) && (id2 != NULL)) {
                node *avis;
                node *tmp = DUPdoDupTree (id1);
                EXPRS_NEXT (tmp) = DUPdoDupTree (id2);
                tmp = TBmakePrf (op, tmp);

                avis
                  = TBmakeAvis (TRAVtmpVar (),
                                TYcopyType (IDS_NTYPE (LET_IDS (INFO_LET (arg_info)))));
                FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
                  = TBmakeVardec (avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

                tmp = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), tmp), NULL);

                /*
                 * change current prf
                 */
                PRF_ARGS (arg_node) = FREEdoFreeTree (PRF_ARGS (arg_node));
                PRF_ARGS (arg_node)
                  = TBmakeExprs (TBmakeId (IDS_AVIS (LET_IDS (ASSIGN_INSTR (tmp)))),
                                 NULL);

                if (add) {
                    PRF_PRF (arg_node) = F_esd_neg;
                } else {
                    PRF_PRF (arg_node) = F_esd_rec;
                }

                INFO_POSTASSIGN (arg_info) = tmp;
            }

            break;
        default:
            break;
        }
    } else {

        /*
         * bottom-up traversal
         */

        type = TYcopyType (AVIS_TYPE (IDS_AVIS (LET_IDS (INFO_LET (arg_info)))));

        if (PRF_PRF (arg_node) == F_esd_neg) {

            if (TYisAUD (type)) {

                newop = F_sub_SxA;
            } else if ((TYisAUDGZ (type)) || (TYgetDim (type) > 0)) {
                newop = F_sub_SxA;
            } else {
                newop = F_sub_SxS;
            }
            PRF_PRF (arg_node) = newop;
            if (((TYisArray (type)) && (T_int == TYgetSimpleType (TYgetScalar (type))))
                || ((!TYisArray (type)) && (T_int == TYgetSimpleType (type)))) {
                PRF_ARGS (arg_node) = TBmakeExprs (TBmakeNum (0), PRF_ARGS (arg_node));
            } else if (((TYisArray (type))
                        && (T_float == TYgetSimpleType (TYgetScalar (type))))
                       || ((!TYisArray (type)) && (T_float == TYgetSimpleType (type)))) {
                PRF_ARGS (arg_node)
                  = TBmakeExprs (TBmakeFloat (0.0), PRF_ARGS (arg_node));
            } else if (((TYisArray (type))
                        && (T_double == TYgetSimpleType (TYgetScalar (type))))
                       || ((!TYisArray (type)) && (T_double == TYgetSimpleType (type)))) {
                PRF_ARGS (arg_node)
                  = TBmakeExprs (TBmakeDouble (0.0), PRF_ARGS (arg_node));
            } else {
                DBUG_ASSERT ((FALSE), "unexpected simpletype");
            }
        }

        if (PRF_PRF (arg_node) == F_esd_rec) {

            if (TYisAUD (type)) {

                newop = F_div_SxA;
            } else if ((TYisAUDGZ (type)) || (TYgetDim (type) > 0)) {
                newop = F_div_SxA;
            } else {
                newop = F_div_SxS;
            }

            PRF_PRF (arg_node) = newop;
            if (((TYisArray (type)) && (T_int == TYgetSimpleType (TYgetScalar (type))))
                || ((!TYisArray (type)) && (T_int == TYgetSimpleType (type)))) {
                PRF_ARGS (arg_node) = TBmakeExprs (TBmakeNum (1), PRF_ARGS (arg_node));
            } else if (((TYisArray (type))
                        && (T_float == TYgetSimpleType (TYgetScalar (type))))
                       || ((!TYisArray (type)) && (T_float == TYgetSimpleType (type)))) {
                PRF_ARGS (arg_node)
                  = TBmakeExprs (TBmakeFloat (1.0), PRF_ARGS (arg_node));
            } else if (((TYisArray (type))
                        && (T_double == TYgetSimpleType (TYgetScalar (type))))
                       || ((!TYisArray (type)) && (T_double == TYgetSimpleType (type)))) {
                PRF_ARGS (arg_node)
                  = TBmakeExprs (TBmakeDouble (1.0), PRF_ARGS (arg_node));
            } else {
                DBUG_ASSERT ((FALSE), "unexpected simpletype");
            }
        }
    }

    DBUG_RETURN (arg_node);
}