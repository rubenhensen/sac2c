/*
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:43:17  sacbase
 * new release made
 *
 * Revision 1.1  1999/01/15 15:31:06  cg
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   tile_size_inference.c
 *
 * prefix: TSI
 *
 * description:
 *
 *   This compiler module realizes an inference scheme for the selection
 *   of appropriate tile sizes. This is used by the code generation in
 *   order to create tiled target code.
 *
 *
 *****************************************************************************/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"

/******************************************************************************
 *
 * function:
 *   node *TileSizeInference(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   This function initiates the tile size inference scheme, i.e.
 *   act_tab is set to tsi_tab and the traversal mechanism is started.
 *   Just as the other optimization schemes, tile size selection is performed
 *   on single function definitions rather than on the entire syntax tree.
 *
 ******************************************************************************/

node *
TileSizeInference (node *arg_node, node *arg_info)
{
    funptr *tmp_tab;

    DBUG_ENTER ("TileSizeInference");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "Tile size selection not initiated on N_fundef level");

    tmp_tab = act_tab;
    act_tab = tsi_tab;

    arg_node = Trav (arg_node, NULL);

    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   nums *NumVect2NumsList(int coeff, node *exprs)
 *
 * description:
 *
 *   This function converts an N_exprs list from an N_array node into a list
 *   of type nums, i.e. the elements of a constant int vector are transformed
 *   into a format suitable to build a shape segment. During the format conversion
 *   each element is multiplied by <coeff>.
 *
 * remark:
 *
 *   Here, a recursive implementation is used although recrsion is not without
 *   problems concerning the stack size and the performance. However, a recursive
 *   implementation where the order of the list is untouched is much easier
 *   than an iterative one and this function is only used for index vectors,
 *   i.e. the lists are quite short.
 *
 ******************************************************************************/

static nums *
NumVect2NumsList (int coeff, node *exprs)
{
    nums *tmp;

    DBUG_ENTER ("NumVect2NumsList");

    DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs),
                 "Illegal node type in call to function NumVect2NumsList()");

    if (exprs == NULL) {
        tmp = NULL;
    } else {
        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (exprs)) == N_num),
                     "Illegal expression in call to function NumVect2NumsList()");

        tmp = MakeNums (coeff * NUM_VAL (EXPRS_EXPR (exprs)),
                        NumVect2NumsList (coeff, EXPRS_NEXT (exprs)));
    }

    DBUG_RETURN (tmp);
}

/******************************************************************************
 *
 * function:
 *   int IsIndexVect(types *type)
 *
 * description:
 *
 *   This function classifies certain types as "index vectors", i.e. small
 *   integer vectors on which primitive arithmetic operations shall not harm
 *   tile size inference.
 *
 ******************************************************************************/

static int
IsIndexVect (types *type)
{
    int res = 0;

    DBUG_ENTER ("IsIndexVect");

    if ((TYPES_BASETYPE (type) == T_int) && (TYPES_DIM (type) == 1)
        && (TYPES_SHAPE (type, 0) < SHP_SEG_SIZE)) {
        res = 1;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   access_t *SearchAccess(node *arg_info)
 *
 * description:
 *
 *   This function traverses all accesses stored in the given arg_info node.
 *   It returns the first one whose index vector is exactly the same variable
 *   that is instantiated in the last let-expression. Additionally, the access
 *   class still must be ACL_unknown.
 *
 ******************************************************************************/

static access_t *
SearchAccess (node *arg_info)
{
    access_t *access;

    DBUG_ENTER ("SearchAccess");

    access = INFO_TSI_ACCESS (arg_info);

    while (access != NULL) {
        if ((ACCESS_IV (access) == IDS_VARDEC (INFO_TSI_LASTLETIDS (arg_info)))
            && (ACCESS_CLASS (access) == ACL_unknown)) {
            break;
        }
        access = ACCESS_NEXT (access);
    }

    DBUG_RETURN (access);
}

/******************************************************************************
 *
 * function:
 *   node *TSIfundef(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_fundef node.
 *
 *   The traversal is limited to the function body, arguments and remaining
 *   functions are not traversed.
 *
 ******************************************************************************/

node *
TSIfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TSIfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        arg_node = Trav (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TSIblock(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_block node.
 *
 *   The traversal is limited to the assignments chain, variable declarations
 *   are not traversed.
 *
 *
 ******************************************************************************/

node *
TSIblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TSIblock");

    if (BLOCK_INSTR (arg_node) != NULL) {
        arg_node = Trav (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TSInwith(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_Nwith node.
 *
 *   Here, the arg_info node is created and initialized. Everywhere beyond
 *   an N_Nwith node, arg_info is not NULL. See general remarks about usage
 *   of arg_info in this compiler module.
 *
 *   In the case of a nested with-loop, the old arg_info is stored and the
 *   whole game is restarted. Afterwards, the outer with-loops features bit
 *   mask is set accordingly.
 *
 ******************************************************************************/

node *
TSInwith (node *arg_node, node *arg_info)
{
    node *old_arg_info;

    DBUG_ENTER ("TSInwith");

    old_arg_info = arg_info;
    /*
     * Store old arg_info for the case of nested with-loops.
     */

    arg_info = MakeInfo ();
    INFO_TSI_ACCESS (arg_info) = NULL;
    INFO_TSI_INDEXVAR (arg_info) = IDS_VARDEC (NWITH_VEC (arg_node));
    INFO_TSI_FEATURE (arg_info) = FEATURE_NONE;
    INFO_TSI_WOTYPE (arg_info) = NWITH_TYPE (arg_node);
    INFO_TSI_BELOWAP (arg_info) = 0;

    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

    FREE (arg_info);

    arg_info = old_arg_info;

    if (arg_info != NULL) {
        INFO_TSI_FEATURE (arg_info) = INFO_TSI_FEATURE (arg_info) | FEATURE_WL;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TSIncode(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_Ncode node.
 *
 *   The code block of the first operator is traversed, the information
 *   gathered is stored in this node, and the traversal continues with the
 *   following operator.
 *
 ******************************************************************************/

node *
TSIncode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TSIncode");

    if (NCODE_CBLOCK (arg_node) != NULL) {
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    }

    NCODE_ACCESS (arg_node) = INFO_TSI_ACCESS (arg_info);
    INFO_TSI_ACCESS (arg_info) = NULL;

    NCODE_FEATURE (arg_node) = INFO_TSI_FEATURE (arg_info);
    INFO_TSI_FEATURE (arg_info) = FEATURE_NONE;

    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TSIassign(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_assign node.
 *
 *   This function just realizes a post-order traversal of the code.
 *
 ******************************************************************************/

node *
TSIassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TSIassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_ASSERT ((ASSIGN_INSTR (arg_node) != NULL), "N_assign node without instruction.");

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TSIwhile(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_while node.
 *
 *   If a with-loop operator contains a sequential loop (for/do/while), then
 *   tile size inference has lost. The only exception is the case where the
 *   loop does not contain any array operations. The function detects exactly
 *   this and sets the features bit mask accordingly.
 *
 ******************************************************************************/

node *
TSIwhile (node *arg_node, node *arg_info)
{
    access_t *old_access;
    feature_t old_feature;

    DBUG_ENTER ("TSIwhile");

    if (arg_info != NULL) {
        old_access = INFO_TSI_ACCESS (arg_info);
        old_feature = INFO_TSI_FEATURE (arg_info);
        INFO_TSI_ACCESS (arg_info) = NULL;
        INFO_TSI_FEATURE (arg_info) = FEATURE_NONE;
    }

    WHILE_BODY (arg_node) = Trav (WHILE_BODY (arg_node), arg_info);

    if (arg_info != NULL) {
        if ((INFO_TSI_ACCESS (arg_info) == NULL)
            && (INFO_TSI_FEATURE (arg_info) == FEATURE_NONE)) {
            /*
             * Nothing harmful happened within the loop.
             */
            INFO_TSI_ACCESS (arg_info) = old_access;
            INFO_TSI_FEATURE (arg_info) = old_feature;
        } else {
            /*
             * Something harmful happened within the loop.
             */
            FreeAllAccess (INFO_TSI_ACCESS (arg_info));
            INFO_TSI_ACCESS (arg_info) = old_access;
            INFO_TSI_FEATURE (arg_info)
              = INFO_TSI_FEATURE (arg_info) | FEATURE_LOOP | old_feature;
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TSIdo(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_do node.
 *
 *   This function is equivalent to TSIwhile for while loops. The kind of loop
 *   makes no difference for the purpose of tile size inference.
 *
 ******************************************************************************/

node *
TSIdo (node *arg_node, node *arg_info)
{
    access_t *old_access;
    feature_t old_feature;

    DBUG_ENTER ("TSIdo");

    if (arg_info != NULL) {
        old_access = INFO_TSI_ACCESS (arg_info);
        old_feature = INFO_TSI_FEATURE (arg_info);
        INFO_TSI_ACCESS (arg_info) = NULL;
        INFO_TSI_FEATURE (arg_info) = FEATURE_NONE;
    }

    DO_BODY (arg_node) = Trav (DO_BODY (arg_node), arg_info);

    if (arg_info != NULL) {
        if ((INFO_TSI_ACCESS (arg_info) == NULL)
            && (INFO_TSI_FEATURE (arg_info) == FEATURE_NONE)) {
            /*
             * Nothing harmful happened within the loop.
             */
            INFO_TSI_ACCESS (arg_info) = old_access;
            INFO_TSI_FEATURE (arg_info) = old_feature;
        } else {
            /*
             * Anything harmful happened within the loop.
             */
            FreeAllAccess (INFO_TSI_ACCESS (arg_info));
            INFO_TSI_ACCESS (arg_info) = old_access;
            INFO_TSI_FEATURE (arg_info)
              = INFO_TSI_FEATURE (arg_info) | FEATURE_LOOP | old_feature;
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TSIcond(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

node *
TSIcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TSIcond");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TSIlet(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_let node.
 *
 *   This function mainly sets the LASTLETIDS entry of the arg_info node
 *   correctly and traverses the left hand side of the let.
 *   Afterwards, it is checked whether any of the variables set by this let,
 *   has been used as an index vector in a psi operation and the respective
 *   access is still classified ACL_unknown. In this case, the access is
 *   re-classified as ACL_irregular. Any "regular" access would have already
 *   been handled during the traversal of the left hand side.
 *
 ******************************************************************************/

node *
TSIlet (node *arg_node, node *arg_info)
{
    ids *var;
    access_t *access;

    DBUG_ENTER ("TSIlet");

    if (arg_info != NULL) {
        /*
         * Here, we are beyond a with-loop.
         */
        INFO_TSI_LASTLETIDS (arg_info) = LET_IDS (arg_node);
    }

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    if (arg_info != NULL) {
        /*
         * Here, we are beyond a with-loop.
         */
        INFO_TSI_LASTLETIDS (arg_info) = NULL;

        var = LET_IDS (arg_node);

        while (var != NULL) {
            access = INFO_TSI_ACCESS (arg_info);
            while (access != NULL) {
                if ((IDS_VARDEC (var) == ACCESS_IV (access))
                    && (ACCESS_CLASS (access) == ACL_unknown)) {
                    ACCESS_CLASS (access) = ACL_irregular;
                }
                access = ACCESS_NEXT (access);
            }
            var = IDS_NEXT (var);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TSIprf(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_prf node.
 *
 *   This function does most of the code analysis. See specific functions
 *   for more information.
 *
 ******************************************************************************/

node *
TSIprf (node *arg_node, node *arg_info)
{
    int i;
    nums *offset;
    access_t *access;

    DBUG_ENTER ("TSIprf");

    if (arg_info != NULL) {
        /*
         * Here, we are beyond a with-loop.
         */
        switch (PRF_PRF (arg_node)) {
        case F_psi:
            if (IDS_DIM (INFO_TSI_LASTLETIDS (arg_info)) == SCALAR) {
                DBUG_ASSERT ((NODE_TYPE (PRF_ARG1 (arg_node)) == N_id),
                             "1st arg of psi is not variable");
                DBUG_ASSERT ((NODE_TYPE (PRF_ARG2 (arg_node)) == N_id),
                             "2nd arg of psi is not variable");

                INFO_TSI_ACCESS (arg_info)
                  = MakeAccess (ID_VARDEC (PRF_ARG2 (arg_node)),
                                ID_VARDEC (PRF_ARG1 (arg_node)), ACL_unknown, NULL,
                                INFO_TSI_ACCESS (arg_info));

                if (ACCESS_IV (INFO_TSI_ACCESS (arg_info))
                    == INFO_TSI_INDEXVAR (arg_info)) {
                    /*
                     * The array is accessed by the index vector  of the surrounding
                     * with-loop.
                     */
                    ACCESS_CLASS (INFO_TSI_ACCESS (arg_info)) = ACL_offset;

                    DBUG_ASSERT ((ID_DIM (PRF_ARG2 (arg_node)) > 0),
                                 "Unknown dimension for 2nd arg of psi");

                    offset = NULL;
                    for (i = 0; i < ID_DIM (PRF_ARG2 (arg_node)); i++) {
                        offset = MakeNums (0, offset);
                    }

                    ACCESS_OFFSET (INFO_TSI_ACCESS (arg_info)) = MakeShpseg (offset);
                } else {
                    if (ID_VAL (PRF_ARG1 (arg_node)) != NULL) {
                        ACCESS_CLASS (INFO_TSI_ACCESS (arg_info)) = ACL_const;

                        DBUG_ASSERT ((NODE_TYPE (ID_VAL (PRF_ARG1 (arg_node)))
                                      == N_array),
                                     "propagated constant vector is not of node type "
                                     "N_array");

                        ACCESS_OFFSET (INFO_TSI_ACCESS (arg_info)) = MakeShpseg (
                          NumVect2NumsList (1,
                                            ARRAY_AELEMS (ID_VAL (PRF_ARG1 (arg_node)))));
                    }
                }
            } else {
                INFO_TSI_FEATURE (arg_info) = INFO_TSI_FEATURE (arg_info) | FEATURE_APSI;
            }
            break;

        case F_take:
            INFO_TSI_FEATURE (arg_info) = INFO_TSI_FEATURE (arg_info) | FEATURE_TAKE;
            break;

        case F_drop:
            INFO_TSI_FEATURE (arg_info) = INFO_TSI_FEATURE (arg_info) | FEATURE_DROP;
            break;

        case F_cat:
            INFO_TSI_FEATURE (arg_info) = INFO_TSI_FEATURE (arg_info) | FEATURE_CAT;
            break;

        case F_rotate:
            INFO_TSI_FEATURE (arg_info) = INFO_TSI_FEATURE (arg_info) | FEATURE_ROT;
            break;

        case F_modarray:
            INFO_TSI_FEATURE (arg_info) = INFO_TSI_FEATURE (arg_info) | FEATURE_MODA;
            break;

        case F_add_SxA:
            access = SearchAccess (arg_info);
            if (access != NULL) {
                if ((NODE_TYPE (PRF_ARG1 (arg_node)) == N_num)
                    && (ID_VARDEC (PRF_ARG2 (arg_node))
                        == INFO_TSI_INDEXVAR (arg_info))) {
                    ACCESS_CLASS (access) = ACL_offset;
                    offset = NULL;
                    for (i = 0; i < VARDEC_OR_ARG_DIM (ACCESS_ARRAY (access)); i++) {
                        offset = MakeNums (NUM_VAL (PRF_ARG1 (arg_node)), offset);
                    }

                    ACCESS_OFFSET (INFO_TSI_ACCESS (arg_info)) = MakeShpseg (offset);
                }
            } else {
                if (!IsIndexVect (IDS_TYPE (INFO_TSI_LASTLETIDS (arg_info)))) {
                    INFO_TSI_FEATURE (arg_info)
                      = INFO_TSI_FEATURE (arg_info) | FEATURE_AARI;
                }
            }
            break;

        case F_add_AxS:
            access = SearchAccess (arg_info);
            if (access != NULL) {
                if ((NODE_TYPE (PRF_ARG2 (arg_node)) == N_num)
                    && (ID_VARDEC (PRF_ARG1 (arg_node))
                        == INFO_TSI_INDEXVAR (arg_info))) {
                    ACCESS_CLASS (access) = ACL_offset;
                    offset = NULL;
                    for (i = 0; i < VARDEC_OR_ARG_DIM (ACCESS_ARRAY (access)); i++) {
                        offset = MakeNums (NUM_VAL (PRF_ARG2 (arg_node)), offset);
                    }

                    ACCESS_OFFSET (INFO_TSI_ACCESS (arg_info)) = MakeShpseg (offset);
                }
            } else {
                if (!IsIndexVect (IDS_TYPE (INFO_TSI_LASTLETIDS (arg_info)))) {
                    INFO_TSI_FEATURE (arg_info)
                      = INFO_TSI_FEATURE (arg_info) | FEATURE_AARI;
                }
            }
            break;

        case F_add_AxA:
            access = SearchAccess (arg_info);
            if (access != NULL) {
                if ((ID_VAL (PRF_ARG1 (arg_node)) != NULL)
                    && (ID_VARDEC (PRF_ARG2 (arg_node))
                        == INFO_TSI_INDEXVAR (arg_info))) {
                    ACCESS_CLASS (access) = ACL_offset;

                    DBUG_ASSERT ((NODE_TYPE (ID_VAL (PRF_ARG1 (arg_node))) == N_array),
                                 "propagated constant vector is not of node type "
                                 "N_array");

                    ACCESS_OFFSET (INFO_TSI_ACCESS (arg_info)) = MakeShpseg (
                      NumVect2NumsList (1, ARRAY_AELEMS (ID_VAL (PRF_ARG1 (arg_node)))));
                } else {
                    if ((ID_VAL (PRF_ARG2 (arg_node)) != NULL)
                        && (ID_VARDEC (PRF_ARG1 (arg_node))
                            == INFO_TSI_INDEXVAR (arg_info))) {
                        ACCESS_CLASS (access) = ACL_offset;

                        DBUG_ASSERT ((NODE_TYPE (ID_VAL (PRF_ARG1 (arg_node)))
                                      == N_array),
                                     "propagated constant vector is not of node type "
                                     "N_array");

                        ACCESS_OFFSET (INFO_TSI_ACCESS (arg_info)) = MakeShpseg (
                          NumVect2NumsList (1,
                                            ARRAY_AELEMS (ID_VAL (PRF_ARG1 (arg_node)))));
                    }
                }
            } else {
                if (!IsIndexVect (IDS_TYPE (INFO_TSI_LASTLETIDS (arg_info)))) {
                    INFO_TSI_FEATURE (arg_info)
                      = INFO_TSI_FEATURE (arg_info) | FEATURE_AARI;
                }
            }
            break;

        case F_sub_AxS:
            access = SearchAccess (arg_info);
            if (access != NULL) {
                if ((NODE_TYPE (PRF_ARG2 (arg_node)) == N_num)
                    && (ID_VARDEC (PRF_ARG1 (arg_node))
                        == INFO_TSI_INDEXVAR (arg_info))) {
                    ACCESS_CLASS (access) = ACL_offset;
                    offset = NULL;
                    for (i = 0; i < VARDEC_OR_ARG_DIM (ACCESS_ARRAY (access)); i++) {
                        offset = MakeNums (-1 * NUM_VAL (PRF_ARG2 (arg_node)), offset);
                    }

                    ACCESS_OFFSET (INFO_TSI_ACCESS (arg_info)) = MakeShpseg (offset);
                }
            } else {
                if (!IsIndexVect (IDS_TYPE (INFO_TSI_LASTLETIDS (arg_info)))) {
                    INFO_TSI_FEATURE (arg_info)
                      = INFO_TSI_FEATURE (arg_info) | FEATURE_AARI;
                }
            }
            break;

        case F_sub_AxA:
            access = SearchAccess (arg_info);
            if (access != NULL) {
                if ((ID_VAL (PRF_ARG2 (arg_node)) != NULL)
                    && (ID_VARDEC (PRF_ARG1 (arg_node))
                        == INFO_TSI_INDEXVAR (arg_info))) {
                    ACCESS_CLASS (access) = ACL_offset;

                    DBUG_ASSERT ((NODE_TYPE (ID_VAL (PRF_ARG1 (arg_node))) == N_array),
                                 "propagated constant vector is not of node type "
                                 "N_array");

                    ACCESS_OFFSET (INFO_TSI_ACCESS (arg_info)) = MakeShpseg (
                      NumVect2NumsList (-1, ARRAY_AELEMS (ID_VAL (PRF_ARG1 (arg_node)))));
                }
            } else {
                if (!IsIndexVect (IDS_TYPE (INFO_TSI_LASTLETIDS (arg_info)))) {
                    INFO_TSI_FEATURE (arg_info)
                      = INFO_TSI_FEATURE (arg_info) | FEATURE_AARI;
                }
            }
            break;

        case F_sub_SxA:
        case F_mul_SxA:
        case F_mul_AxS:
        case F_mul_AxA:
        case F_div_SxA:
        case F_div_AxS:
        case F_div_AxA:
            if (!IsIndexVect (IDS_TYPE (INFO_TSI_LASTLETIDS (arg_info)))) {
                INFO_TSI_FEATURE (arg_info) = INFO_TSI_FEATURE (arg_info) | FEATURE_AARI;
            }
            break;

        case F_idx_psi:
        case F_idx_modarray:
            /*
             * These functions are only introduced by index vector elimination,
             * however tile size inference must always be applied before index
             * vector elimination.
             */
            DBUG_ASSERT (1, "primitive function idx_psi or idx_modarray found during "
                            "tile size selection");
            break;

        default:
            /*
             * Do nothing !
             *
             * All other primitive functions do not deal with arrays.
             */
            break;
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TSIap(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_ap node.
 *
 *   This function detects function applications within with-loop operators
 *   and sets the features bit mask accordingly. However, functions which do
 *   not deal with arrays are no problem for tile size inference. We could
 *   investigate this by an inter-functional analysis. But for now, we only check
 *   whether one of the arguments or one of the return values is of an array
 *   type.
 *
 ******************************************************************************/

node *
TSIap (node *arg_node, node *arg_info)
{
    ids *ret_var;

    DBUG_ENTER ("TSIap");

    if (arg_info != NULL) {
        /*
         * Here, we are beyond a with-loop.
         */

        ret_var = INFO_TSI_LASTLETIDS (arg_info);

        while (ret_var != NULL) {
            if (IDS_DIM (ret_var) != SCALAR) {
                INFO_TSI_FEATURE (arg_info) = INFO_TSI_FEATURE (arg_info) | FEATURE_AP;
                break;
            }
            ret_var = IDS_NEXT (ret_var);
        }

        if (!(INFO_TSI_FEATURE (arg_info) & FEATURE_AP)) {
            /*
             * FEATURE_AP has not been set yet.
             */
            if (AP_ARGS (arg_node) != NULL) {
                INFO_TSI_BELOWAP (arg_info) = 1;
                AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
                INFO_TSI_BELOWAP (arg_info) = 0;
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TSIid(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_id node.
 *
 *   This function applies only beyond an N_ap node within a with-loop
 *   operator. If the function argument is not of scalar type the AP bit
 *   of the features bit mask is set.
 *
 *
 ******************************************************************************/

node *
TSIid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TSIid");

    if ((arg_info != NULL) && (INFO_TSI_BELOWAP (arg_info))) {
        if (ID_DIM (arg_node) != SCALAR) {
            INFO_TSI_FEATURE (arg_info) = INFO_TSI_FEATURE (arg_info) | FEATURE_AP;
        }
    }

    DBUG_RETURN (arg_node);
}
