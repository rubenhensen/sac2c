/*****************************************************************************
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

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse_helper.h"
#include "new_types.h"
#include "str.h"
#include "memory.h"
#include "namespaces.h"

#define DBUG_PREFIX "CMPT"
#include "debug.h"

#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "shape.h"
#include "globals.h"
#include "pattern_match.h"
#include "type_utils.h"

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
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_EQFLAG (result) = CMPT_UKNWN;
    INFO_TREE (result) = NULL;
    INFO_IDS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

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

    DBUG_ENTER ();

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
 *   node* CMPTnumbyte(node *arg_node, info *arg_info)
 *
 * description:
 *   compares value of numbyte node
 *
 ******************************************************************************/
node *
CMPTnumbyte (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info),
                   NUMBYTE_VAL (arg_node) == NUMBYTE_VAL (INFO_TREE (arg_info)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTnumshort(node *arg_node, info *arg_info)
 *
 * description:
 *   compares value of numshort node
 *
 ******************************************************************************/
node *
CMPTnumshort (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info),
                   NUMSHORT_VAL (arg_node) == NUMSHORT_VAL (INFO_TREE (arg_info)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTnumint(node *arg_node, info *arg_info)
 *
 * description:
 *   compares value of numint node
 *
 ******************************************************************************/
node *
CMPTnumint (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info),
                   NUMINT_VAL (arg_node) == NUMINT_VAL (INFO_TREE (arg_info)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTnumlong(node *arg_node, info *arg_info)
 *
 * description:
 *   compares value of numlong node
 *
 ******************************************************************************/
node *
CMPTnumlong (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info),
                   NUMLONG_VAL (arg_node) == NUMLONG_VAL (INFO_TREE (arg_info)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTnumlonglong(node *arg_node, info *arg_info)
 *
 * description:
 *   compares value of numlonglong node
 *
 ******************************************************************************/
node *
CMPTnumlonglong (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info),
                   NUMLONGLONG_VAL (arg_node) == NUMLONGLONG_VAL (INFO_TREE (arg_info)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTnumubyte(node *arg_node, info *arg_info)
 *
 * description:
 *   compares value of numubyte node
 *
 ******************************************************************************/
node *
CMPTnumubyte (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info),
                   NUMUBYTE_VAL (arg_node) == NUMUBYTE_VAL (INFO_TREE (arg_info)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTnumushort(node *arg_node, info *arg_info)
 *
 * description:
 *   compares value of numushort node
 *
 ******************************************************************************/
node *
CMPTnumushort (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info),
                   NUMUSHORT_VAL (arg_node) == NUMUSHORT_VAL (INFO_TREE (arg_info)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTnumuint(node *arg_node, info *arg_info)
 *
 * description:
 *   compares value of numuint node
 *
 ******************************************************************************/
node *
CMPTnumuint (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info),
                   NUMUINT_VAL (arg_node) == NUMUINT_VAL (INFO_TREE (arg_info)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTnumulong(node *arg_node, info *arg_info)
 *
 * description:
 *   compares value of numulong node
 *
 ******************************************************************************/
node *
CMPTnumulong (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info),
                   NUMULONG_VAL (arg_node) == NUMULONG_VAL (INFO_TREE (arg_info)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTnumulonglong(node *arg_node, info *arg_info)
 *
 * description:
 *   compares value of numulonglong node
 *
 ******************************************************************************/
node *
CMPTnumulonglong (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info), NUMULONGLONG_VAL (arg_node)
                                             == NUMULONGLONG_VAL (INFO_TREE (arg_info)));

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
    DBUG_ENTER ();

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
    DBUG_ENTER ();

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
    DBUG_ENTER ();

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
    DBUG_ENTER ();

    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info),
                   FLOAT_VAL (arg_node) == FLOAT_VAL (INFO_TREE (arg_info)));

    DBUG_RETURN (arg_node);
}

node *
CMPTfloatvec (node *arg_node, info *arg_info)
{
    floatvec a, b;
    DBUG_ENTER ();

    a = FLOATVEC_VAL (arg_node);
    b = FLOATVEC_VAL (INFO_TREE (arg_info));

    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info), memcmp (&a, &b, sizeof (floatvec)) == 0);

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
    DBUG_ENTER ();

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
    DBUG_ENTER ();

    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info),
                   STReq (STR_STRING (arg_node), STR_STRING (INFO_TREE (arg_info))));

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
    DBUG_ENTER ();

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

    DBUG_ENTER ();

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
 *   node* CMPTspid(node *arg_node, info *arg_info)
 *
 * description:
 *   compares string of spid node
 *
 ******************************************************************************/
node *
CMPTspid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info),
                   STReq (SPID_NAME (arg_node), SPID_NAME (INFO_TREE (arg_info)))
                   && NSequals (SPID_NS (arg_node), SPID_NS (INFO_TREE (arg_info))));

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
    DBUG_ENTER ();

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
 *   static node* HeterogeneousArrayCompare(node *arg_node, info *arg_info)
 *
 * description:
 *   Compares two N_array nodes whose elements are all simple scalars
 *   constants.
 *   This assumes a non-homogeneous representation of such N_array nodes,
 *   where one node may have:
 *     one = 1;
 *     [ one, 2, 3];
 *   and the other has:
 *     [ 1, 2, 3];
 *
 *   If a decision is made to completely flatten N_array element nodes,
 *   then this code should be eliminated.
 *
 *
 ******************************************************************************/
static node *
HeterogeneousArrayCompare (node *arg_node, info *arg_info)
{
    node *n1;
    node *n2;
    node *c1;
    node *c2;
    node *oldn2;
    pattern *pat1;
    pattern *pat2;

    DBUG_ENTER ();

    if (TUisScalar (ARRAY_ELEMTYPE (arg_node)) && TYisAKV (ARRAY_ELEMTYPE (arg_node))
        && TYisAKV (ARRAY_ELEMTYPE (INFO_TREE (arg_info)))) {

        pat1 = PMconst (1, PMAgetNode (&c1));
        pat2 = PMconst (1, PMAgetNode (&c2));
        n1 = ARRAY_AELEMS (arg_node);
        n2 = ARRAY_AELEMS (INFO_TREE (arg_info));
        while ((NULL != n1) && (PMmatchFlat (pat1, ARRAY_AELEMS (n1)))
               && (PMmatchFlat (pat2, ARRAY_AELEMS (n2)))) {

            oldn2 = INFO_TREE (arg_info);
            INFO_TREE (arg_info) = c2;
            n1 = TravLocal (c1, arg_info);
            INFO_TREE (arg_info) = oldn2;

            n1 = EXPRS_NEXT (n1);
            n2 = EXPRS_NEXT (n2);
        }
        PMfree (pat1);
        PMfree (pat2);

    } else {
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
    DBUG_ENTER ();

    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info),
                   TYeqTypes (ARRAY_ELEMTYPE (arg_node),
                              ARRAY_ELEMTYPE (INFO_TREE (arg_info)))
                     && SHcompareShapes (ARRAY_FRAMESHAPE (arg_node),
                                         ARRAY_FRAMESHAPE (INFO_TREE (arg_info))));

    /* traverse ArrayElements (the real son) */
    arg_node = HeterogeneousArrayCompare (arg_node, arg_info);

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
    DBUG_ENTER ();

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
    DBUG_ENTER ();

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
    DBUG_ENTER ();

    /* compare attributes */
    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info),
                   GENERATOR_OP1 (arg_node) == GENERATOR_OP1 (INFO_TREE (arg_info)));

    INFO_EQFLAG (arg_info)
      = CMPT_TEST (INFO_EQFLAG (arg_info),
                   GENERATOR_OP2 (arg_node) == GENERATOR_OP2 (INFO_TREE (arg_info)));

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
    DBUG_ENTER ();

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
    DBUG_ENTER ();

    DBUG_EXECUTE (CTInote (EMPTY_LOC, "Unknown node type %s encountered in compare tree "
                           "traversal!",
                           NODE_TEXT (arg_node)));
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
        DBUG_ENTER ();                                                                   \
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
    DBUG_ENTER ();

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

    DBUG_ENTER ();

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
    INFO_IDS (arg_info) = FREEoptFreeTree(INFO_IDS (arg_info));

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (result);
}

#undef DBUG_PREFIX
