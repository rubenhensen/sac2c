#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "internal_lib.h"
#include "optimize.h"
#include "free.h"
#include "DataFlowMask.h"
#include "DupTree.h"
#include "SSATransform.h"

#include "UndoElimSubDiv.h"

/*****************************************************************************
 *
 * function:
 *   int ReachedArgument(node *arg_node)
 *
 * description:
 *   returns '1' if the arg_node is an argument defined outside current
 *   block node
 *
 ****************************************************************************/

static bool
ReachedArgument (node *arg_node)
{

    bool result;
    DBUG_ENTER ("ReachedArgument");

    if (NODE_TYPE (arg_node) == N_id) {
        if (AVIS_SSAASSIGN (ID_AVIS (arg_node)) == NULL)
            result = TRUE;
        else
            result = FALSE;
    } else
        result = FALSE;

    DBUG_RETURN (result);
}

/*****************************************************************************
 *
 * function:
 *   int ReachedDefinition(node *arg_node)
 *
 * description:
 *   returns '1' if the definition of the arg_node contain no prf-node
 *
 ****************************************************************************/

static bool
ReachedDefinition (node *arg_node)
{

    int result;
    DBUG_ENTER ("ReachedDefinition");

    if (NODE_TYPE (arg_node) == N_id) {
        if (AVIS_SSAASSIGN (ID_AVIS (arg_node)) == NULL)
            result = FALSE;
        else if (NODE_TYPE (LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (arg_node)))))
                 != N_prf)
            result = TRUE;
        else
            result = FALSE;
    } else
        result = FALSE;

    DBUG_RETURN (result);
}

/*****************************************************************************
 *
 * function:
 *   int IsConstant(node *arg_node)
 *
 * description:
 *   returns '1' if the arg_node is a constant value
 *
 ****************************************************************************/

static bool
IsConstant (node *arg_node)
{

    int result;
    DBUG_ENTER ("IsConstant");

    switch (NODE_TYPE (arg_node)) {
    case N_num:
    case N_double:
    case N_float:
    case N_bool:
    case N_char:
        result = TRUE;
        break;

    default:
        result = FALSE;
    }

    DBUG_RETURN (result);
}

static bool
IsTraverseable (node *arg)
{

    bool result;

    DBUG_ENTER ("IsTraverseable");

    result = TRUE;

    if (IsConstant (arg) || ReachedDefinition (arg) || ReachedArgument (arg))
        result = FALSE;

    DBUG_RETURN (result);
}

static bool
IsUndoCase (prf primf, node *tmp, node *tmp2)
{

    bool result;

    DBUG_ENTER ("IsUndoCase");

    result = FALSE;

    if ((PRF_PRF (tmp) == F_mul_SxS) && (primf == F_add_SxS)) {

        if ((NODE_TYPE (tmp2) != N_id)) {

            if (((NODE_TYPE (tmp2) == N_num) && (NUM_VAL (tmp2) == -1))
                || ((NODE_TYPE (tmp2) == N_float) && (FLOAT_VAL (tmp2) == -1.0))
                || ((NODE_TYPE (tmp2) == N_double) && (DOUBLE_VAL (tmp2) == -1.0)))
                result = TRUE;
        }
    }

    if ((PRF_PRF (tmp) == F_div_SxS) && (primf == F_mul_SxS)) {

        if ((NODE_TYPE (tmp2) != N_id)) {

            if (((NODE_TYPE (tmp2) == N_num) && (NUM_VAL (tmp2) == 1))
                || ((NODE_TYPE (tmp2) == N_float) && (FLOAT_VAL (tmp2) == 1.0))
                || ((NODE_TYPE (tmp2) == N_double) && (DOUBLE_VAL (tmp2) == 1.0)))
                result = TRUE;
        }
    }

    DBUG_RETURN (result);
}

node *
UndoElimSubDiv (node *arg_node, node *a)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("UndoEleminateSD");

    if (arg_node != NULL) {
        arg_info = MakeInfo ();

        old_tab = act_tab;
        act_tab = uesd_tab;

        arg_node = Trav (arg_node, arg_info);

        act_tab = old_tab;

        arg_info = FreeTree (arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   ALblock(node *arg_node, node *arg_info)
 *
 * description:
 *   store block-node for access to vardec-nodes
 *   store types-node as shape for new types-nodes
 *   reset nodelists
 *   traverse through block-nodes
 ****************************************************************************/

node *
UESDblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ESDblock");

    if (BLOCK_INSTR (arg_node) != NULL) {
        /*
         * store pointer on actual N_block-node for append of new N_vardec nodes
         */

        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   ALassign(node *arg_node, node *arg_info)
 *
 * description:
 *   Set flag ASSIGN_STATUS to mark unused nodes in optimization-process
 *   Traverse through assign-nodes
 *   If previous assign-node was optimized, increase optimization counter
 *   and include new nodes
 *   Free all used nodelists
 *   If current assign node was unused during this optimization-cycle
 *   traverse in let node
 *
 ****************************************************************************/

node *
UESDassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ESDassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {

        ASSIGN_STATUS (arg_node) = 1;

        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);

        /*
         * traverse in N_let-node
         */

        if ((ASSIGN_INSTR (arg_node) != NULL) && (ASSIGN_STATUS (arg_node) == 1)) {

            ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
        }
    }
    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   ALlet(node *arg_node, node *arg_info)
 *
 * description:
 *   store current let-node to include last created new primitive-node
 *   traverse N_let-nodes
 *
 ****************************************************************************/

node *
UESDlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("UESDlet");
    if (LET_EXPR (arg_node) != NULL) {

        if ((LET_IDS (arg_node) != NULL) && (IDS_AVIS (LET_IDS (arg_node)) != NULL)) {
            types *tmp
              = VARDEC_OR_ARG_TYPE (AVIS_VARDECORARG (IDS_AVIS (LET_IDS (arg_node))));
            if ((!enforce_ieee)
                || ((TYPES_BASETYPE (tmp) != T_double)
                    && (TYPES_BASETYPE (tmp) != T_float)))

                LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   ALprf(node *arg_node, node *arg_info)
 *
 * description:
 *   If primitive is associative and commutative start optimization-routines:
 *     Reset all needed variables and lists
 *     Traverse recursively through arguments of the primitive node
 *     and collect all constant nodes and indissoluble argument nodes
 *     If there are enough collected nodes, create new optimized structure
 *     The optimized structure is stored in arg_info and will be included
 *     in the previous incarnation of ALassign()
 *     Free nodelists
 *
 ****************************************************************************/

node *
UESDprf (node *arg_node, node *arg_info)
{

    node *newnode, *tmp;

    DBUG_ENTER ("AssociativeLawOptimize");

    if (NODE_TYPE (PRF_ARGS (arg_node)) == N_exprs) {

        if ((PRF_PRF (arg_node) == F_add_SxS) || (PRF_PRF (arg_node) == F_add_AxS)
            || (PRF_PRF (arg_node) == F_add_SxA) || (PRF_PRF (arg_node) == F_add_AxA)) {

            if (IsTraverseable (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))))) {

                tmp = EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node)));
                tmp = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (tmp))));
                if (IsUndoCase (F_add_SxS, tmp, EXPRS_EXPR (PRF_ARGS (tmp)))) {

                    if (PRF_PRF (arg_node) == F_add_SxS)
                        PRF_PRF (arg_node) = F_sub_SxS;
                    if (PRF_PRF (arg_node) == F_add_AxS)
                        PRF_PRF (arg_node) = F_sub_AxS;
                    if (PRF_PRF (arg_node) == F_add_SxA)
                        PRF_PRF (arg_node) = F_sub_SxA;
                    if (PRF_PRF (arg_node) == F_add_AxA)
                        PRF_PRF (arg_node) = F_sub_AxA;

                    newnode = DupTree (EXPRS_EXPR (
                      EXPRS_NEXT (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (
                        ID_AVIS (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node)))))))))));

                    EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))) = newnode;

                } else if ((NULL != EXPRS_NEXT (PRF_ARGS (tmp)))
                           && (IsUndoCase (F_add_SxS, tmp,
                                           EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (tmp)))))) {

                    if (PRF_PRF (arg_node) == F_add_SxS)
                        PRF_PRF (arg_node) = F_sub_SxS;
                    if (PRF_PRF (arg_node) == F_add_AxS)
                        PRF_PRF (arg_node) = F_sub_AxS;
                    if (PRF_PRF (arg_node) == F_add_SxA)
                        PRF_PRF (arg_node) = F_sub_SxA;
                    if (PRF_PRF (arg_node) == F_add_AxA)
                        PRF_PRF (arg_node) = F_sub_AxA;

                    newnode = DupTree (
                      EXPRS_EXPR (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (
                        ID_AVIS (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))))))))));

                    EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))) = newnode;
                }

            } else if (IsTraverseable (EXPRS_EXPR (PRF_ARGS (arg_node)))) {

                tmp = EXPRS_EXPR (PRF_ARGS (arg_node));
                tmp = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (tmp))));
                if (IsUndoCase (F_add_SxS, tmp, EXPRS_EXPR (PRF_ARGS (tmp)))) {

                    if (PRF_PRF (arg_node) == F_add_SxS)
                        PRF_PRF (arg_node) = F_sub_SxS;
                    if (PRF_PRF (arg_node) == F_add_AxS)
                        PRF_PRF (arg_node) = F_sub_SxA;
                    if (PRF_PRF (arg_node) == F_add_SxA)
                        PRF_PRF (arg_node) = F_sub_AxS;
                    if (PRF_PRF (arg_node) == F_add_AxA)
                        PRF_PRF (arg_node) = F_sub_AxA;

                    newnode = DupTree (
                      EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                        AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (PRF_ARGS (arg_node))))))))));

                    EXPRS_EXPR (PRF_ARGS (arg_node))
                      = EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node)));

                    EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))) = newnode;

                } else if ((NULL != EXPRS_NEXT (PRF_ARGS (tmp)))
                           && (IsUndoCase (F_add_SxS, tmp,
                                           EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (tmp)))))) {

                    if (PRF_PRF (arg_node) == F_add_SxS)
                        PRF_PRF (arg_node) = F_sub_SxS;
                    if (PRF_PRF (arg_node) == F_add_AxS)
                        PRF_PRF (arg_node) = F_sub_SxA;
                    if (PRF_PRF (arg_node) == F_add_SxA)
                        PRF_PRF (arg_node) = F_sub_AxS;
                    if (PRF_PRF (arg_node) == F_add_AxA)
                        PRF_PRF (arg_node) = F_sub_AxA;

                    newnode = DupTree (EXPRS_EXPR (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                      AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (PRF_ARGS (arg_node)))))))));

                    EXPRS_EXPR (PRF_ARGS (arg_node))
                      = EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node)));

                    EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))) = newnode;
                }
            }
        }

        if ((PRF_PRF (arg_node) == F_mul_SxS) || (PRF_PRF (arg_node) == F_mul_AxS)
            || (PRF_PRF (arg_node) == F_mul_SxA) || (PRF_PRF (arg_node) == F_mul_AxA)) {

            if (IsTraverseable (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))))) {

                tmp = EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node)));
                tmp = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (tmp))));
                if (IsUndoCase (F_mul_SxS, tmp, EXPRS_EXPR (PRF_ARGS (tmp)))) {

                    if (PRF_PRF (arg_node) == F_mul_SxS)
                        PRF_PRF (arg_node) = F_div_SxS;
                    if (PRF_PRF (arg_node) == F_mul_AxS)
                        PRF_PRF (arg_node) = F_div_AxS;
                    if (PRF_PRF (arg_node) == F_mul_SxA)
                        PRF_PRF (arg_node) = F_div_SxA;
                    if (PRF_PRF (arg_node) == F_mul_AxA)
                        PRF_PRF (arg_node) = F_div_AxA;

                    newnode = DupTree (EXPRS_EXPR (
                      EXPRS_NEXT (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (
                        ID_AVIS (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node)))))))))));

                    EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))) = newnode;
                }

            } else if (IsTraverseable (EXPRS_EXPR (PRF_ARGS (arg_node)))) {

                tmp = EXPRS_EXPR (PRF_ARGS (arg_node));
                tmp = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (tmp))));
                if (IsUndoCase (F_mul_SxS, tmp, EXPRS_EXPR (PRF_ARGS (tmp)))) {

                    if (PRF_PRF (arg_node) == F_mul_SxS)
                        PRF_PRF (arg_node) = F_div_SxS;
                    if (PRF_PRF (arg_node) == F_mul_AxS)
                        PRF_PRF (arg_node) = F_div_AxS;
                    if (PRF_PRF (arg_node) == F_mul_SxA)
                        PRF_PRF (arg_node) = F_div_SxA;
                    if (PRF_PRF (arg_node) == F_mul_AxA)
                        PRF_PRF (arg_node) = F_div_AxA;

                    newnode = DupTree (
                      EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                        AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (PRF_ARGS (arg_node))))))))));

                    EXPRS_EXPR (PRF_ARGS (arg_node))
                      = EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node)));

                    EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))) = newnode;
                }
            }
        }
    }
    DBUG_RETURN (arg_node);
}
