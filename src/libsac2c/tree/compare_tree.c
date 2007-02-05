/*****************************************************************************
 *
 * $Id$
 *
 * prefix: CMPT
 *
 * description:
 *   this module implements a literal tree compare for two given parts of
 *   the ast. it compares for equal structre, identifiers and values.
 *   this modules is used by SSACSE to find common subexpressions.
 *
 *****************************************************************************/

#include "compare_tree.h"

#include <string.h>

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse_helper.h"
#include "new_types.h"
#include "internal_lib.h"
#include "str.h"
#include "memory.h"
#include "dbug.h"
#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "shape.h"

/*
 * INFO structure
 */
struct INFO {
    cmptree_t eqflag;
    node *tree;
    node *ids;
};

/*
 * INFO macros
 */
#define INFO_EQFLAG(n) (n->eqflag)
#define INFO_TREE(n) (n->tree)
#define INFO_IDS(n) (n->ids)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_EQFLAG (result) = CMPT_UKNWN;
    INFO_TREE (result) = NULL;
    INFO_IDS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

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
 *   node *TravLocal(node *arg_node, info *arg_info)
 *
 * description:
 *   traverses all sons of the given node and sets the corresponding
 *   son in the second tree stored in INFO_TREE. If the second tree
 *   attribute.
 *   if only one attribute of the both trees is NULL,
 *   set INFO_EQFLAG to CMPT_NEQ.
 *   if this flag is set to CMPT_NEQ the traversal is stopped.
 *
 ******************************************************************************/
static node *
TravLocal (node *arg_node, info *arg_info)
{
    node *arg_node2;
    int i;

    DBUG_ENTER ("TravLocal");

    arg_node2 = INFO_TREE (arg_info);

    if (arg_node == NULL) {
        if (arg_node2 == NULL) {
            INFO_EQFLAG (arg_info) = CMPT_EQ;
        } else {
            INFO_EQFLAG (arg_info) = CMPT_NEQ;
        }
    } else {
        if (arg_node2 == NULL) {
            INFO_EQFLAG (arg_info) = CMPT_NEQ;
        } else {
            for (i = 0; i < TRAVnumSons (arg_node); i++) {
                /* process every son of node */
                if (INFO_EQFLAG (arg_info) == CMPT_EQ) {
                    if (TRAVgetSon (i, arg_node) != NULL) {
                        /* start traversal of son */
                        INFO_TREE (arg_info) = TRAVgetSon (i, arg_node2);
                        if (INFO_TREE (arg_info) != NULL) {
                            TRAVdo (TRAVgetSon (i, arg_node), arg_info);
                        } else {
                            INFO_EQFLAG (arg_info) = CMPT_NEQ;
                        }
                    } else {
                        /* check, if second tree is also NULL */
                        if (TRAVgetSon (i, arg_node2) != NULL) {
                            INFO_EQFLAG (arg_info) = CMPT_NEQ;
                        }
                    }
                } else {
                    /* stop further traversals */
                    i = TRAVnumSons (arg_node) + 1;
                }
            }
        }
    }

    INFO_TREE (arg_info) = arg_node2;
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTnum(node *arg_node, info *arg_info)
 *
 * description:
 *   compares value of num node
 *
 ******************************************************************************/
node *
CMPTnum (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMPTnum");

    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info),
                   NUM_VAL (arg_node) == NUM_VAL (INFO_TREE (arg_info)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTchar(node *arg_node, info *arg_info)
 *
 * description:
 *   compares value of char node
 *
 ******************************************************************************/
node *
CMPTchar (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMPTchar");

    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info),
                   CHAR_VAL (arg_node) == CHAR_VAL (INFO_TREE (arg_info)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTbool(node *arg_node, info *arg_info)
 *
 * description:
 *   compares value of bool node
 *
 ******************************************************************************/
node *
CMPTbool (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMPTbool");

    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info),
                   BOOL_VAL (arg_node) == BOOL_VAL (INFO_TREE (arg_info)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTfloat(node *arg_node, info *arg_info)
 *
 * description:
 *   compares value of float node
 *
 ******************************************************************************/
node *
CMPTfloat (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMPTfloat");

    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info),
                   FLOAT_VAL (arg_node) == FLOAT_VAL (INFO_TREE (arg_info)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTdouble(node *arg_node, info *arg_info)
 *
 * description:
 *   compares value of double node
 *
 ******************************************************************************/
node *
CMPTdouble (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMPTdouble");

    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info),
                   DOUBLE_VAL (arg_node) == DOUBLE_VAL (INFO_TREE (arg_info)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTstr(node *arg_node, info *arg_info)
 *
 * description:
 *   compares string of str node
 *
 ******************************************************************************/
node *
CMPTstr (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMPTstr");

    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info),
                   strcmp (STR_STRING (arg_node), STR_STRING (INFO_TREE (arg_info)))
                     == 0);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CMPTtype( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
CMPTtype (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMPTtype");

    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info),
                   TYcmpTypes (TYPE_TYPE (arg_node), TYPE_TYPE (INFO_TREE (arg_info)))
                     == TY_eq);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTid(node *arg_node, info *arg_info)
 *
 * description:
 *   compares avis link of id node
 *
 ******************************************************************************/
node *
CMPTid (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ("CMPTid");

    if (INFO_EQFLAG (arg_info) == CMPT_EQ) {
        avis = ID_AVIS (INFO_TREE (arg_info));

        INFO_EQFLAG (arg_info)
          = CMPT_TEST (INFO_EQFLAG (arg_info),
                       ID_AVIS (arg_node)
                         == (AVIS_ALT (avis) != NULL ? AVIS_ALT (avis) : avis));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CMPTids( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
CMPTids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMPTids");

    if (INFO_EQFLAG (arg_info) == CMPT_EQ) {
        AVIS_ALT (IDS_AVIS (INFO_TREE (arg_info))) = IDS_AVIS (arg_node);
        INFO_IDS (arg_info)
          = TBmakeIds (IDS_AVIS (INFO_TREE (arg_info)), INFO_IDS (arg_info));
        arg_node = TravLocal (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTarray(node *arg_node, info *arg_info)
 *
 * description:
 *   compares two arrays
 *
 ******************************************************************************/
node *
CMPTarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMPTarray");

    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info),
                   TYeqTypes (ARRAY_ELEMTYPE (arg_node),
                              ARRAY_ELEMTYPE (INFO_TREE (arg_info)))
                     && SHcompareShapes (ARRAY_SHAPE (arg_node),
                                         ARRAY_SHAPE (INFO_TREE (arg_info))));

    /* traverse ArrayElements (the real son) */
    arg_node = TravLocal (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTprf(node *arg_node, info *arg_info)
 *
 * description:
 *   check for equal primitive function and equal args.
 *
 ******************************************************************************/
node *
CMPTprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMPTprf");

    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info),
                   PRF_PRF (arg_node) == PRF_PRF (INFO_TREE (arg_info)));

    /* traverse args (the real son) */
    arg_node = TravLocal (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTap(node *arg_node, info *arg_info)
 *
 * description:
 *   check for equal called function and equal args
 *
 ******************************************************************************/
node *
CMPTap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMPTap");

    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info),
                   AP_FUNDEF (arg_node) == AP_FUNDEF (INFO_TREE (arg_info)));

    /* traverse args (the real son) */
    arg_node = TravLocal (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTgenerator(node *arg_node, info *arg_info)
 *
 * description:
 *   checks all attributes and traverses sons
 *
 ******************************************************************************/
node *
CMPTgenerator (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMPTgenerator");

    /* compare attributes */
    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info),
                   GENERATOR_OP1 (arg_node) == GENERATOR_OP1 (INFO_TREE (arg_info)));

    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info),
                   GENERATOR_OP2 (arg_node) == GENERATOR_OP2 (INFO_TREE (arg_info)));

    INFO_EQFLAG (arg_info) = CMPT_TEST (INFO_EQFLAG (arg_info),
                                        GENERATOR_OP1_ORIG (arg_node)
                                          == GENERATOR_OP1_ORIG (INFO_TREE (arg_info)));

    INFO_EQFLAG (arg_info) = CMPT_TEST (INFO_EQFLAG (arg_info),
                                        GENERATOR_OP2_ORIG (arg_node)
                                          == GENERATOR_OP2_ORIG (INFO_TREE (arg_info)));

    /* traverse all sons */
    arg_node = TravLocal (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTfold(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMPTfold");

    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info),
                   FOLD_FUNDEF (arg_node) == FOLD_FUNDEF (INFO_TREE (arg_info)));

    /* traverse all sons */
    arg_node = TravLocal (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTunknown(node *arg_node, info *arg_info)
 *
 * description:
 *   dummy function for all node types with missing implementation.
 *   sets compare-result to CMPT_NEQ.
 *
 ******************************************************************************/
node *
CMPTunknown (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMPTunknown");

    DBUG_EXECUTE ("CMPT", CTInote ("Unknown node type %s encountered in compare tree "
                                   "traversal!",
                                   NODE_TEXT (arg_node)););
    INFO_EQFLAG (arg_info) = CMPT_NEQ;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CMPT<basic>(node *arg_node, info *arg_info)
 *
 *****************************************************************************/
#define CMPTBASIC(name)                                                                  \
    node *CMPT##name (node *arg_node, info *arg_info)                                    \
    {                                                                                    \
        DBUG_ENTER ("CMPT" #name);                                                       \
                                                                                         \
        arg_node = TravLocal (arg_node, arg_info);                                       \
                                                                                         \
        DBUG_RETURN (arg_node);                                                          \
    }

CMPTBASIC (block)
CMPTBASIC (do)
CMPTBASIC (return )
CMPTBASIC (assign)
CMPTBASIC (let)
CMPTBASIC (cond)
CMPTBASIC (empty)
CMPTBASIC (exprs)
CMPTBASIC (funcond)
CMPTBASIC (with)
CMPTBASIC (part)
CMPTBASIC (withid)
CMPTBASIC (code)
CMPTBASIC (genarray)
CMPTBASIC (modarray)

/******************************************************************************
 *
 * function:
 *   node* CMPTnodeType(node *arg_node, info *arg_info)
 *
 * description:
 *   This PRE-Traversal function checks both trees for equal node types
 *   and NULL values
 *
 ******************************************************************************/
node *
CMPTnodeType (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMPTnodeType");

    /* only one of the sons is NULL, so tree1 and tree2 cannot be equal */
    if (((arg_node == NULL) || (INFO_TREE (arg_info) == NULL))
        && (arg_node != INFO_TREE (arg_info))) {
        INFO_EQFLAG (arg_info) = CMPT_NEQ;
    }

    /* check for equal node type (if tree1 and tree2 != NULL)*/
    if ((arg_node != NULL) && (INFO_TREE (arg_info) != NULL)
        && (NODE_TYPE (arg_node) != NODE_TYPE (INFO_TREE (arg_info)))) {

        INFO_EQFLAG (arg_info) = CMPT_NEQ;
    }

    DBUG_RETURN (arg_node);
}

/** <!--*********************************************************************-->
 *
 * function:
 *   cmptree_t CMPTdoCompareTree(node* tree1, node *tree2)
 *
 * @param tree1 First tree
 * @param tree2 Second tree
 *
 * @return  the result is
 *        CMPT_EQ: tree1 == tree2 (means same operations and values)
 *       CMPT_NEQ: tree1 != tree2 (at least one difference between the trees)
 *
 ******************************************************************************/
cmptree_t
CMPTdoCompareTree (node *tree1, node *tree2)
{
    cmptree_t result;
    info *arg_info;
    node *ids;

    DBUG_ENTER ("CMPTdoCompareTree");

    arg_info = MakeInfo ();

    /* start traversal with CMPT_EQ as result */
    INFO_EQFLAG (arg_info) = CMPT_EQ;
    INFO_TREE (arg_info) = tree2;

    TRAVpush (TR_cmpt);
    tree1 = TRAVdo (tree1, arg_info);
    TRAVpop ();

    /* save result */
    result = INFO_EQFLAG (arg_info);

    /*
     * Reset AVIS_ALT of all modified avis nodes
     */
    ids = INFO_IDS (arg_info);
    while (ids != NULL) {
        AVIS_ALT (IDS_AVIS (ids)) = NULL;
        ids = IDS_NEXT (ids);
    }
    if (INFO_IDS (arg_info) != NULL) {
        INFO_IDS (arg_info) = FREEdoFreeTree (INFO_IDS (arg_info));
    }

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (result);
}
