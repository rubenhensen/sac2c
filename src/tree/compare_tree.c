/*
 *
 * $Log$
 * Revision 1.17  2005/07/19 22:46:45  sbs
 * fixed problem with NULL arg_node2
 *
 * Revision 1.16  2004/12/08 18:02:40  ktr
 * removed ARRAY_TYPE/ARRAY_NTYPE
 *
 * Revision 1.15  2004/11/26 13:12:18  khf
 * SacDevCamp04: COMPILES!!
 *
 * Revision 1.14  2004/11/21 11:22:03  sah
 * removed some old ast infos
 *
 * Revision 1.13  2004/08/01 18:44:21  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 1.12  2003/11/18 17:12:58  dkr
 * no changes done
 *
 * Revision 1.11  2003/01/28 18:16:22  ktr
 * CompareTreeLUT added to compare_tree
 *
 * Revision 1.10  2001/05/22 14:57:37  nmw
 * bug fixed in CMPwithid(), destroys sometimes arg_info :-(
 *
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
 *   WARNING: THIS MODULE USES NEW TYPES ONLY!!
 *
 *****************************************************************************/

#include "compare_tree.h"

#include <string.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse_helper.h"
#include "new_types.h"
#include "internal_lib.h"
#include "dbug.h"
#include "traverse.h"
#include "free.h"
#include "shape.h"

/*
 * INFO structure
 */
struct INFO {
    cmptree_t eqflag;
    node *tree;
    lut_t *lut;
    int writelut;
};

/*
 * INFO macros
 */
#define INFO_CMPT_EQFLAG(n) (n->eqflag)
#define INFO_CMPT_TREE(n) (n->tree)
#define INFO_CMPT_LUT(n) (n->lut)
#define INFO_CMPT_WRITELUT(n) (n->writelut)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_CMPT_EQFLAG (result) = CMPT_UKNWN;
    INFO_CMPT_TREE (result) = NULL;
    INFO_CMPT_LUT (result) = NULL;
    INFO_CMPT_WRITELUT (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

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
 *   son in the second tree stored in INFO_CMPT_TREE. If the second tree
 *   attribute.
 *   if only one attribute of the both trees is NULL,
 *   set INFO_CMPT_EQFLAG to CMPT_NEQ.
 *   if this flag is set to CMPT_NEQ the traversal is stopped.
 *
 ******************************************************************************/
static node *
TravLocal (node *arg_node, info *arg_info)
{
    node *arg_node2;
    int i;

    DBUG_ENTER ("TravLocal");

    arg_node2 = INFO_CMPT_TREE (arg_info);

    if (arg_node == NULL) {
        if (arg_node2 == NULL) {
            INFO_CMPT_EQFLAG (arg_info) = CMPT_EQ;
        } else {
            INFO_CMPT_EQFLAG (arg_info) = CMPT_NEQ;
        }
    } else {
        if (arg_node2 == NULL) {
            INFO_CMPT_EQFLAG (arg_info) = CMPT_NEQ;
        } else {

            for (i = 0; i < TRAVnumSons (arg_node); i++) {
                /* process every son of node */
                if (INFO_CMPT_EQFLAG (arg_info) == CMPT_EQ) {
                    if (TRAVgetSon (i, arg_node) != NULL) {
                        /* start traversal of son */
                        INFO_CMPT_TREE (arg_info) = TRAVgetSon (i, arg_node2);
                        if (INFO_CMPT_TREE (arg_info) != NULL) {
                            TRAVdo (TRAVgetSon (i, arg_node), arg_info);
                        } else {
                            INFO_CMPT_EQFLAG (arg_info) = CMPT_NEQ;
                        }
                    } else {
                        /* check, if second tree is also NULL */
                        if (TRAVgetSon (i, arg_node2) != NULL) {
                            INFO_CMPT_EQFLAG (arg_info) = CMPT_NEQ;
                        }
                    }
                } else {
                    /* stop further traversals */
                    i = TRAVnumSons (arg_node) + 1;
                }
            }
        }
    }

    INFO_CMPT_TREE (arg_info) = arg_node2;
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

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   NUM_VAL (arg_node) == NUM_VAL (INFO_CMPT_TREE (arg_info)));

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

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   CHAR_VAL (arg_node) == CHAR_VAL (INFO_CMPT_TREE (arg_info)));

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

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   BOOL_VAL (arg_node) == BOOL_VAL (INFO_CMPT_TREE (arg_info)));

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

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   strcmp (STR_STRING (arg_node), STR_STRING (INFO_CMPT_TREE (arg_info)))
                     == 0);

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
    DBUG_ENTER ("CMPTid");

    if (INFO_CMPT_LUT (arg_info) == NULL) {
        INFO_CMPT_EQFLAG (arg_info)
          = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                       ID_AVIS (arg_node) == ID_AVIS (INFO_CMPT_TREE (arg_info)));
    } else {
        if (INFO_CMPT_WRITELUT (arg_info) == FALSE) {
            INFO_CMPT_EQFLAG (arg_info)
              = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                           ID_AVIS (arg_node)
                             == LUTsearchInLutPp (INFO_CMPT_LUT (arg_info),
                                                  ID_AVIS (INFO_CMPT_TREE (arg_info))));
        } else {
            LUTinsertIntoLutP (INFO_CMPT_LUT (arg_info),
                               ID_AVIS (INFO_CMPT_TREE (arg_info)), ID_AVIS (arg_node));
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CMPTids(node *arg_ids, info *arg_info)
 *
 * description:
 *   there are some ugly casts between node* and ids* to handle ids chain
 *   with usual INFO_CMPT_TREE atribute.
 *
 ******************************************************************************/
node *
CMPTids (node *arg_ids, info *arg_info)
{
    DBUG_ENTER ("CMPTids");

    /* only one of the sons is NULL, so tree1 and tree2 cannot be equal */
    if (((arg_ids == NULL) || (INFO_CMPT_TREE (arg_info) == NULL))
        && (arg_ids != (INFO_CMPT_TREE (arg_info)))) {
        INFO_CMPT_EQFLAG (arg_info) = CMPT_NEQ;
    }

    if ((arg_ids != NULL) && (INFO_CMPT_TREE (arg_info) != NULL)
        && (INFO_CMPT_EQFLAG (arg_info) == CMPT_EQ)) {

        if (INFO_CMPT_LUT (arg_info) == NULL) {
            INFO_CMPT_EQFLAG (arg_info)
              = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                           IDS_AVIS (arg_ids) == IDS_AVIS ((INFO_CMPT_TREE (arg_info))));
        } else {
            if (INFO_CMPT_WRITELUT (arg_info) == FALSE) {
                INFO_CMPT_EQFLAG (arg_info)
                  = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                               IDS_AVIS (arg_ids)
                                 == LUTsearchInLutPp (INFO_CMPT_LUT (arg_info),
                                                      IDS_AVIS (
                                                        (INFO_CMPT_TREE (arg_info)))));
            } else {
                LUTinsertIntoLutP (INFO_CMPT_LUT (arg_info),
                                   IDS_AVIS ((INFO_CMPT_TREE (arg_info))),
                                   IDS_AVIS (arg_ids));
            }
        }
        if (IDS_NEXT (arg_ids) != NULL) {
            INFO_CMPT_TREE (arg_info) = (IDS_NEXT ((INFO_CMPT_TREE (arg_info))));
            IDS_NEXT (arg_ids) = TravLocal (IDS_NEXT (arg_ids), arg_info);
        }
    }
    DBUG_RETURN (arg_ids);
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

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   FLOAT_VAL (arg_node) == FLOAT_VAL (INFO_CMPT_TREE (arg_info)));

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

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   DOUBLE_VAL (arg_node) == DOUBLE_VAL (INFO_CMPT_TREE (arg_info)));

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

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   SHcompareShapes (ARRAY_SHAPE (arg_node),
                                    ARRAY_SHAPE (INFO_CMPT_TREE (arg_info))));

    /* traverse ArrayElements (the real son) */
    arg_node = TravLocal (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTlet(node *arg_node, info *arg_info)
 *
 * description:
 *   traverses IDS chain and expr
 *   the ugly node* cast is needed to use INFO_CMPT_TREE for ids structures.
 *
 ******************************************************************************/
node *
CMPTlet (node *arg_node, info *arg_info)
{
    node *let_node2;

    DBUG_ENTER ("CMPTlet");

    /* save current compare node */
    let_node2 = INFO_CMPT_TREE (arg_info);

    INFO_CMPT_WRITELUT (arg_info) = TRUE;
    /* traverse ids-chain */
    if ((INFO_CMPT_EQFLAG (arg_info) == CMPT_EQ) && (LET_IDS (arg_node) != NULL)) {
        INFO_CMPT_TREE (arg_info) = (LET_IDS (INFO_CMPT_TREE (arg_info)));
        LET_IDS (arg_node) = TravLocal (LET_IDS (arg_node), arg_info);
    }

    INFO_CMPT_WRITELUT (arg_info) = FALSE;

    /* restore current compare node */
    INFO_CMPT_TREE (arg_info) = let_node2;

    /* traverse expr (the real son) */
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

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   PRF_PRF (arg_node) == PRF_PRF (INFO_CMPT_TREE (arg_info)));

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

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   AP_FUNDEF (arg_node) == AP_FUNDEF (INFO_CMPT_TREE (arg_info)));

    /* traverse args (the real son) */
    arg_node = TravLocal (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTwithid(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse ids chains (VEC and IDS)
 *
 ******************************************************************************/
node *
CMPTwithid (node *arg_node, info *arg_info)
{
    node *withid_node2;

    DBUG_ENTER ("CMPTwithid");

    /* save current compare node */
    withid_node2 = INFO_CMPT_TREE (arg_info);

    /* traverse IDS-chain */
    if ((INFO_CMPT_EQFLAG (arg_info) == CMPT_EQ) && (WITHID_IDS (arg_node) != NULL)) {
        INFO_CMPT_TREE (arg_info) = WITHID_IDS (INFO_CMPT_TREE (arg_info));
        WITHID_IDS (arg_node) = TravLocal (WITHID_IDS (arg_node), arg_info);
    }

    /* restore current compare node */
    INFO_CMPT_TREE (arg_info) = withid_node2;

    /* traverse VEC-chain */
    if ((INFO_CMPT_EQFLAG (arg_info) == CMPT_EQ) && (WITHID_VEC (arg_node) != NULL)) {
        INFO_CMPT_TREE (arg_info) = WITHID_VEC (INFO_CMPT_TREE (arg_info));
        WITHID_VEC (arg_node) = TravLocal (WITHID_VEC (arg_node), arg_info);
    }

    /* restore current compare node */
    INFO_CMPT_TREE (arg_info) = withid_node2;

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

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   GENERATOR_OP1 (arg_node) == GENERATOR_OP1 (INFO_CMPT_TREE (arg_info)));

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   GENERATOR_OP2 (arg_node) == GENERATOR_OP2 (INFO_CMPT_TREE (arg_info)));

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   GENERATOR_OP1_ORIG (arg_node)
                     == GENERATOR_OP1_ORIG (INFO_CMPT_TREE (arg_info)));

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   GENERATOR_OP2_ORIG (arg_node)
                     == GENERATOR_OP2_ORIG (INFO_CMPT_TREE (arg_info)));

    /* traverse all sons */
    arg_node = TravLocal (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTgenarray(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMPTgenarray");

    /* compare attributes */
    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   NODE_TYPE (arg_node) == NODE_TYPE (INFO_CMPT_TREE (arg_info)));

    /* traverse all sons */
    arg_node = TravLocal (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTmodarray(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMPTmodarray");

    /* compare attributes */
    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   NODE_TYPE (arg_node) == NODE_TYPE (INFO_CMPT_TREE (arg_info)));

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

    /* compare attributes */
    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   NODE_TYPE (arg_node) == NODE_TYPE (INFO_CMPT_TREE (arg_info)));

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   FOLD_PRF (arg_node) == FOLD_PRF (INFO_CMPT_TREE (arg_info)));

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   FOLD_FUNDEF (arg_node) == FOLD_FUNDEF (INFO_CMPT_TREE (arg_info)));

    /* traverse all sons */
    arg_node = TravLocal (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTcode(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMPTNcode");

    /* compare attributes */
    if (INFO_CMPT_LUT (arg_info) == NULL)
        INFO_CMPT_EQFLAG (arg_info)
          = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                       CODE_USED (arg_node) == CODE_USED (INFO_CMPT_TREE (arg_info)));

    /* traverse all sons */
    arg_node = TravLocal (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTglobobj(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

extern node *
CMPTglobobj (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CMPTgloboj");

    INFO_CMPT_EQFLAG (arg_info)
      = CMPT_TEST (INFO_CMPT_EQFLAG (arg_info),
                   GLOBOBJ_OBJDEF (arg_node)
                     == GLOBOBJ_OBJDEF (INFO_CMPT_TREE (arg_info)));

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

    INFO_CMPT_EQFLAG (arg_info) = CMPT_NEQ;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTblock(node *arg_node, info *arg_info)
 *
 * description: call of local traverse mechanism
 *
 *
 ******************************************************************************/

extern node *
CMPTblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMPTblock");

    arg_node = TravLocal (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTassign(node *arg_node, info *arg_info)
 *
 * description: call of local traverse mechanism
 *
 *
 ******************************************************************************/

extern node *
CMPTassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMPTassign");

    arg_node = TravLocal (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTreturn(node *arg_node, info *arg_info)
 *
 * description: call of local traverse mechanism
 *
 *
 ******************************************************************************/

extern node *
CMPTreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMPTreturn");

    arg_node = TravLocal (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTcond(node *arg_node, info *arg_info)
 *
 * description: call of local traverse mechanism
 *
 *
 ******************************************************************************/

extern node *
CMPTcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMPTcond");

    arg_node = TravLocal (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPwith(node *arg_node, info *arg_info)
 *
 * description: call of local traverse mechanism
 *
 *
 ******************************************************************************/

extern node *
CMPTwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMPTwith");

    arg_node = TravLocal (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTpart(node *arg_node, info *arg_info)
 *
 * description: call of local traverse mechanism
 *
 *
 ******************************************************************************/

extern node *
CMPTpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMPTpart");

    arg_node = TravLocal (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTdo(node *arg_node, info *arg_info)
 *
 * description: call of local traverse mechanism
 *
 *
 ******************************************************************************/

extern node *
CMPTdo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMPTdo");

    arg_node = TravLocal (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTexprs(node *arg_node, info *arg_info)
 *
 * description: call of local traverse mechanism
 *
 *
 ******************************************************************************/

extern node *
CMPTexprs (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMPTexprs");

    arg_node = TravLocal (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTempty(node *arg_node, info *arg_info)
 *
 * description: call of local traverse mechanism
 *
 *
 ******************************************************************************/

extern node *
CMPTempty (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMPTempty");

    arg_node = TravLocal (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

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
 *   cmptree_t CMPTdoCompareTree(node* tree1, node *tree2)
 *
 * description:
 *   starts traversal of tree1 to compare it with tree2.
 *   the result is
 *        CMPT_EQ: tree1 == tree2 (means same operations, values and variables)
 *       CMPT_NEQ: tree1 != tree2 (at least one difference between the trees)
 *
 ******************************************************************************/
cmptree_t
CMPTdoCompareTree (node *tree1, node *tree2)
{
    cmptree_t result;
    info *arg_info;

    DBUG_ENTER ("CMPTdoCompareTree");

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

        /***************/
        INFO_CMPT_LUT (arg_info) = NULL;
        INFO_CMPT_WRITELUT (arg_info) = FALSE;
        /***************/

        TRAVpush (TR_cmpt);
        tree1 = TRAVdo (tree1, arg_info);
        TRAVpop ();

        /* save result */
        result = INFO_CMPT_EQFLAG (arg_info);

        arg_info = FreeInfo (arg_info);
    }

    DBUG_RETURN (result);
}

/**
 ******************************************************************************
 *
 * starts traversal of tree 1 to compare it with tree 2 taking a LUT
 * into account.
 * This allows to detect structural equality despite the occurence of
 * different ID(s) nodes. Optionally the third parameter can be used to
 * insert a priori knowledge into the traversal.
 *
 * @param tree1 First tree
 * @param tree2 Second tree
 * @param lut A LUT specifying a prioro knowledge about id-substitutions.
 * Usally you might want to pass a fresh LUT.
 *
 * @return  the result is
 *        CMPT_EQ: tree1 == tree2 (means same operations and values)
 *       CMPT_NEQ: tree1 != tree2 (at least one difference between the trees)
 *
 ******************************************************************************
 */
cmptree_t
CMPTdoCompareTreeLUT (node *tree1, node *tree2, lut_t *lut)
{
    cmptree_t result;
    info *arg_info;

    DBUG_ENTER ("CMPTdoCompareTreeLUT");

    DBUG_ASSERT (lut != NULL, "lut == NULL!");

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

        INFO_CMPT_LUT (arg_info) = lut;
        INFO_CMPT_WRITELUT (arg_info) = FALSE;

        TRAVpush (TR_cmpt);
        tree1 = TRAVdo (tree1, arg_info);
        TRAVpop ();

        /* save result */
        result = INFO_CMPT_EQFLAG (arg_info);

        arg_info = FreeInfo (arg_info);
    }

    DBUG_RETURN (result);
}
