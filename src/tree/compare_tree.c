/*
 *
 * $Log$
 * Revision 1.9  2001/05/17 11:15:47  dkr
 * FREE(info) replaced by FreeTree(info)
 *
 * Revision 1.8  2001/04/30 12:24:58  nmw
 * comments corrected
 *
 * Revision 1.7  2001/03/29 08:45:37  nmw
 * tabs2spaces done
 *
 * Revision 1.6  2001/03/22 20:02:03  dkr
 * include of tree.h eliminated
 *
 * Revision 1.5  2001/03/20 14:22:50  nmw
 * CMPTarray added, checks for equal types, too
 *
 * Revision 1.4  2001/03/12 17:19:51  nmw
 * weak compare for NULL terminated chains fixed
 *
 * Revision 1.3  2001/03/07 15:56:45  nmw
 * compare tree implemented (for simple expressions used by SSACSE)
 *
 * Revision 1.2  2001/03/07 10:03:40  nmw
 * first implementation
 *
 * Revision 1.1  2001/03/06 13:16:50  nmw
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   compare_tree.c
 *
 * prefix: CMPT
 *
 * description:
 *   this module implements a literal tree compare for two given parts of
 *   the ast. it compares for equal structre, identifiers and values.
 *   this modules is used by SSACSE to find common subexpressions.
 *
 *****************************************************************************/

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "dbug.h"
#include "traverse.h"
#include "free.h"
#include "compare_tree.h"
#include "typecheck.h"

static ids *TravIDS (ids *arg_ids, node *arg_info);
static ids *CMPTids (ids *arg_ids, node *arg_info);

/*
 * flag = CMP_TEST( flag, testcond ):
 * checks condition testcond (like DBUG_ASSERT) and sets CMPT_NEQ if failed
 * else preseves flag
 */
#define CMPT_TEST(flag, testcond)                                                        \
    ((((flag) == CMPT_EQ) && (!(testcond))) ? CMPT_NEQ : (flag))

/******************************************************************************
 *
 * function:
 *   node* CMPTnum(node *arg_node, node *arg_info)
 *
 * description:
 *   compares value of num node
 *
 ******************************************************************************/
node *
CMPTnum (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTnum");

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   NUM_VAL (arg_node) == NUM_VAL (INFO_CMPT_TREE (arg_info)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTchar(node *arg_node, node *arg_info)
 *
 * description:
 *   compares value of char node
 *
 ******************************************************************************/
node *
CMPTchar (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTchar");

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   CHAR_VAL (arg_node) == CHAR_VAL (INFO_CMPT_TREE (arg_info)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTbool(node *arg_node, node *arg_info)
 *
 * description:
 *   compares value of bool node
 *
 ******************************************************************************/
node *
CMPTbool (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTbool");

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   BOOL_VAL (arg_node) == BOOL_VAL (INFO_CMPT_TREE (arg_info)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTstr(node *arg_node, node *arg_info)
 *
 * description:
 *   compares string of str node
 *
 ******************************************************************************/
node *
CMPTstr (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTstr");

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   strcmp (STR_STRING (arg_node), STR_STRING (INFO_CMPT_TREE (arg_info)))
                     == 0);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTid(node *arg_node, node *arg_info)
 *
 * description:
 *   compares avis link of id node
 *
 ******************************************************************************/
node *
CMPTid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTid");

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   ID_AVIS (arg_node) == ID_AVIS (INFO_CMPT_TREE (arg_info)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTfloat(node *arg_node, node *arg_info)
 *
 * description:
 *   compares value of float node
 *
 ******************************************************************************/
node *
CMPTfloat (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTfloat");

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   FLOAT_VAL (arg_node) == FLOAT_VAL (INFO_CMPT_TREE (arg_info)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTdouble(node *arg_node, node *arg_info)
 *
 * description:
 *   compares value of double node
 *
 ******************************************************************************/
node *
CMPTdouble (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTdouble");

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   DOUBLE_VAL (arg_node) == DOUBLE_VAL (INFO_CMPT_TREE (arg_info)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTarray(node *arg_node, node *arg_info)
 *
 * description:
 *   compares two arrays
 *
 ******************************************************************************/
node *
CMPTarray (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTarray");

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   CmpTypes (ARRAY_TYPE (arg_node),
                             ARRAY_TYPE (INFO_CMPT_TREE (arg_info)))
                     == 1);

    /* traverse ArrayElements (the real son) */
    arg_node = CMPTTravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTlet(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses IDS chain and expr
 *   the ugly node* cast is needed to use INFO_CMPT_TREE for ids structures.
 *
 ******************************************************************************/
node *
CMPTlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTlet");

    /* traverse ids-chain */
    if ((INFO_CMPT_EQFLAG (arg_info) == CMPT_EQ) && (LET_IDS (arg_node) != NULL)) {
        INFO_CMPT_TREE (arg_info) = (node *)(LET_IDS (INFO_CMPT_TREE (arg_info)));
        LET_IDS (arg_node) = TravIDS (LET_IDS (arg_node), arg_info);
    }

    /* traverse expr (the real son) */
    arg_node = CMPTTravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTprf(node *arg_node, node *arg_info)
 *
 * description:
 *   check for equal primitive function and equal args.
 *
 ******************************************************************************/
node *
CMPTprf (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTprf");

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   PRF_PRF (arg_node) == PRF_PRF (INFO_CMPT_TREE (arg_info)));

    /* traverse args (the real son) */
    arg_node = CMPTTravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTap(node *arg_node, node *arg_info)
 *
 * description:
 *   check for equal called function and equal args
 *
 ******************************************************************************/
node *
CMPTap (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTap");

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   AP_FUNDEF (arg_node) == AP_FUNDEF (INFO_CMPT_TREE (arg_info)));

    /* traverse args (the real son) */
    arg_node = CMPTTravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTNwithid(node *arg_node, node *arg_info)
 *
 * description:
 *   traverse ids chains (VEC and IDS)
 *
 ******************************************************************************/
node *
CMPTNwithid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTNwithid");
    /* traverse IDS-chain */
    if ((INFO_CMPT_EQFLAG (arg_info) == CMPT_EQ) && (NWITHID_IDS (arg_node) != NULL)) {
        INFO_CMPT_TREE (arg_info) = (node *)(NWITHID_IDS (INFO_CMPT_TREE (arg_info)));
        NWITHID_IDS (arg_node) = TravIDS (NWITHID_IDS (arg_node), arg_info);
    }

    /* traverse VEC-chain */
    if ((INFO_CMPT_EQFLAG (arg_info) == CMPT_EQ) && (NWITHID_VEC (arg_node) != NULL)) {
        INFO_CMPT_TREE (arg_info) = (node *)(NWITHID_VEC (INFO_CMPT_TREE (arg_info)));
        NWITHID_VEC (arg_node) = TravIDS (NWITHID_VEC (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTNgenerator(node *arg_node, node *arg_info)
 *
 * description:
 *   checks all attributes and traverses sons
 *
 ******************************************************************************/
node *
CMPTNgenerator (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTNgenerator");

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   NGEN_OP1 (arg_node) == NGEN_OP1 (INFO_CMPT_TREE (arg_info)));

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   NGEN_OP2 (arg_node) == NGEN_OP2 (INFO_CMPT_TREE (arg_info)));

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   NGEN_OP1_ORIG (arg_node) == NGEN_OP1_ORIG (INFO_CMPT_TREE (arg_info)));

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   NGEN_OP2_ORIG (arg_node) == NGEN_OP2_ORIG (INFO_CMPT_TREE (arg_info)));

    /* traverse all sons */
    arg_node = CMPTTravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTNwithop(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTNwithop (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTNwithop");

    /* compare attributes */
    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   NWITHOP_TYPE (arg_node) == NWITHOP_TYPE (INFO_CMPT_TREE (arg_info)));

    if (NWITHOP_TYPE (arg_node) == WO_foldprf) {
        INFO_CMPT_EQFLAG (arg_info)
          = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                       NWITHOP_PRF (arg_node) == NWITHOP_PRF (INFO_CMPT_TREE (arg_info)));
    } else if (NWITHOP_TYPE (arg_node) == WO_foldfun) {
        INFO_CMPT_EQFLAG (arg_info)
          = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                       NWITHOP_FUNDEF (arg_node)
                         == NWITHOP_FUNDEF (INFO_CMPT_TREE (arg_info)));
    }

    /* traverse all sons */
    arg_node = CMPTTravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTNcode(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTNcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTNcode");

    /* compare attributes */
    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   NCODE_USED (arg_node) == NCODE_USED (INFO_CMPT_TREE (arg_info)));

    /* traverse all sons */
    arg_node = CMPTTravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTunknown(node *arg_node, node *arg_info)
 *
 * description:
 *   dummy function for all node types with missing implementation.
 *   sets compare-result to CMPT_NEQ.
 *
 ******************************************************************************/
node *
CMPTunknown (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTunknown");

    INFO_CMPT_EQFLAG (arg_info) = CMPT_NEQ;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   static ids *CMPTids(ids *arg_ids, node *arg_info)
 *
 * description:
 *   there are some ugly casts between node* and ids* to handle ids chain
 *   with usual INFO_CMPT_TREE atribute.
 *
 ******************************************************************************/
static ids *
CMPTids (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("CMPTids");

    /* only one of the sons is NULL, so tree1 and tree2 cannot be equal */
    if (((arg_ids == NULL) || (INFO_CMPT_TREE (arg_info) == NULL))
        && (arg_ids != (ids *)(INFO_CMPT_TREE (arg_info)))) {
        INFO_CMPT_EQFLAG (arg_info) = CMPT_NEQ;
    }

    if ((arg_ids != NULL) && (INFO_CMPT_TREE (arg_info) != NULL)
        && (INFO_CMPT_EQFLAG (arg_info) == CMPT_EQ)) {

        INFO_CMPT_EQFLAG (arg_info)
          = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                       IDS_AVIS (arg_ids)
                         == IDS_AVIS (((ids *)INFO_CMPT_TREE (arg_info))));

        if (IDS_NEXT (arg_ids) != NULL) {
            INFO_CMPT_TREE (arg_info)
              = (node *)(IDS_NEXT (((ids *)INFO_CMPT_TREE (arg_info))));
            IDS_NEXT (arg_ids) = TravIDS (IDS_NEXT (arg_ids), arg_info);
        }
    }
    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   ids *TravIDS(ids *arg_ids, node *arg_info)
 *
 * description:
 *   similar implementation of trav mechanism as used for nodes
 *   here used for ids.
 *
 ******************************************************************************/
static ids *
TravIDS (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("TravIDS");

    DBUG_ASSERT (arg_ids != NULL, "traversal in NULL ids");
    arg_ids = CMPTids (arg_ids, arg_info);

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   node *CMPTTravSons(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses all sons of the given node and sets the corresponding
 *   son in the second tree stored in INFO_CMPT_TREE. If the second tree
 *   attribute.
 *   if only one attribute of the both trees is NULL,
 *   set INFO_CMPT_EQFLAG to CMPT_NEQ.
 *   if this flag is set to CMPT_NEQ the traversal is stopped.
 *
 ******************************************************************************/
node *
CMPTTravSons (node *arg_node, node *arg_info)
{
    node *arg_node2;
    int i;

    DBUG_ENTER ("CMPTTravSons");

    arg_node2 = INFO_CMPT_TREE (arg_info);

    for (i = 0; i < nnode[NODE_TYPE (arg_node)]; i++) {
        /* process every son of node */
        if (INFO_CMPT_EQFLAG (arg_info) == CMPT_EQ) {
            if (arg_node->node[i] != NULL) {
                /* start traversal of son */
                INFO_CMPT_TREE (arg_info) = arg_node2->node[i];
                arg_node->node[i] = Trav (arg_node->node[i], arg_info);
            } else {
                /* check, if second tree is also NULL */
                if (arg_node2->node[i] != NULL) {
                    INFO_CMPT_EQFLAG (arg_info) = CMPT_NEQ;
                }
            }
        } else {
            /* stop further traversals */
            i = nnode[NODE_TYPE (arg_node)] + 1;
        }
    }

    INFO_CMPT_TREE (arg_info) = arg_node2;
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTnodeType(node *arg_node, node *arg_info)
 *
 * description:
 *   This PRE-Traversal function checks both trees for equal node types
 *   and NULL values
 *
 ******************************************************************************/
node *
CMPTnodeType (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTnodeType");

    /* only one of the sons is NULL, so tree1 and tree2 cannot be equal */
    if (((arg_node == NULL) || (INFO_CMPT_TREE (arg_info) == NULL))
        && (arg_node != INFO_CMPT_TREE (arg_info))) {
        INFO_CMPT_EQFLAG (arg_info) = CMPT_NEQ;
    }

    /* check for equal node type (if tree1 and tree2 != NULL)*/
    if ((arg_node != NULL) && (INFO_CMPT_TREE (arg_info) != NULL)
        && (NODE_TYPE (arg_node) != NODE_TYPE (INFO_CMPT_TREE (arg_info)))) {
        DBUG_PRINT ("CMPT", ("comparing nodetype %s and %s", NODE_TEXT (arg_node),
                             NODE_TEXT (INFO_CMPT_TREE (arg_info))));

        INFO_CMPT_EQFLAG (arg_info) = CMPT_NEQ;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   cmptree_t CompareTree(node* tree1, node *tree2)
 *
 * description:
 *   starts traversal of tree1 to compare it with tree2.
 *   the result is
 *        CMPT_EQ: tree1 == tree2 (means same operations, values and variables)
 *       CMPT_NEQ: tree1 != tree2 (at least one difference between the trees)
 *
 ******************************************************************************/
cmptree_t
CompareTree (node *tree1, node *tree2)
{
    cmptree_t result;
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("CompareTree");

    if ((tree1 == NULL) || (tree2 == NULL)) {
        /* NULL pointer handling */
        if (tree1 == tree2) {
            /* NULL == NULL is EQ */
            result = CMPT_EQ;
        } else {
            result = CMPT_NEQ;
        }
    } else {

        arg_info = MakeInfo ();

        DBUG_PRINT ("CMPT", ("starting tree compare (%s, %s)", NODE_TEXT (tree1),
                             NODE_TEXT (tree2)));

        /* start traversal with CMPT_EQ as result */
        INFO_CMPT_EQFLAG (arg_info) = CMPT_EQ;
        INFO_CMPT_TREE (arg_info) = tree2;

        old_tab = act_tab;
        act_tab = cmptree_tab;

        tree1 = Trav (tree1, arg_info);

        /* save result */
        result = INFO_CMPT_EQFLAG (arg_info);

        act_tab = old_tab;
        arg_info = FreeTree (arg_info);
    }

    DBUG_RETURN (result);
}
