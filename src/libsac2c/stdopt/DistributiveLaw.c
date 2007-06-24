/*
 * $Id$
 */

#define DL_NODELIST_OPERATOR(n) ((node *)NODELIST_ATTRIB2 (n))
#define DL_NODELIST_PARENTNODES(n) ((nodelist *)NODELIST_ATTRIB2 (n))

/*****************************************************************************
 *
 *
 * Files: DistributiveLaw.c, DistributiveLaw.h
 * ******
 *
 * Prefix: DL
 * *******
 *
 * description:
 * ************
 *
 * DistributiveLaw is an algebraic optimization technique using ssa-form.
 *
 * DistributiveLaw tries to find a commonly used node in conjunction with
 * the same supported(!) operation in an expression.
 * This node is extracted from the subexpression it is used in. First the
 * remaining subexpressions are evaluated, summarized and after that the
 * extracted node is added to the result.
 * Simple example: (a * b) + (a * c)  =>   a * (b + c)
 *
 *
 * Explaining of the meaning of some used expressions:
 * ***************************************************
 *
 * variable nodes:
 *   - N_id
 *
 * constant nodes:
 *   - N_num
 *   - N_bool
 *   - N_float
 *   - N_double
 *   - N_char
 *
 * Supported operations:
 *   this are the following associative and commutative primitive operations:
 *     - F_add_SxS
 *     - F_mul_SxS
 *     - F_max_SxS
 *     - F_min
 *     - F_and_SxS
 *     - F_or_SxS
 *
 *
 * order of one optimization cycle:
 * ********************************
 *
 * The optimization DL starts for all fundef nodes. Then the recusive traversal
 * through the block node down to the last assign-node - the return node starts.
 * Then the return node is ignored and the optimization of the last assign-node
 * starts. First then prf/ap-node is checked. Is the prf/ap-node is supported
 * the function 'travElems' traverse the arguments (both exprs nodes) by
 * recursion. The recursion terminates and count every different termination
 * nodes in conjunction to the used prf/ap-node to a nodelist when:
 *   i. the next primitive is not supported
 *  ii. the argument is a constant value
 * iii. the argument is defined outside the current block
 *  iv. after reaching of a second supported prf/ap-node a third one is reached
 * The traversal continues if a second operator is reached and the distributive
 * law is applicable to first and second prf/ap-node. This node is memorized
 * and traversal continues until a different prf/ap-node is reached.
 *
 * After collecting all termination-nodes in conjunction with the prf/ap-nodes,
 * which are reachable from the start-node, the most frequent node is
 * investigated. This node will be extracted in the following.
 * Now the traversal through the the definition of the start-node is repeated
 * once again with two changes:
 * 1) Only the investigated operator is traversed in!
 * 2) If a termination node is reached it is checked if it is the
 *    MOSTFREQUENTNODE in conjuntion with the correct operation.
 *    If yes: The node and all nodes from first occurence of the operator
 *            to this node are stored in OPTLIST and every new occurence
 *            of MOSTFREQUENTNODE in this subtree is ignored.
 *    If no: The traversal is continued. If no MOSTFREQUENTNODE is found in
 *           the whole subtree of the first occurence of the investigated operator
 *           the root of the subtree is stored in NONOPTLIST.
 * All traversed assign-nodes are marked as used and will be ignored as
 * later start-nodes of this optimization in this cycle.
 *
 * After that all collected MOSTFREQUENTNODES in OPTLIST are removed or if
 * possible and neccessary ( ((2 * a) + a)  =>   (a * (2 + 1)) ) replaced by
 * the neutral element of the investigated operator.
 * Then all nodes in OPTLIST are concatenates with the starting-operator until only
 * one top node remains. This node is concatenated with the MOSTFREQUENTNODE
 * with the investigated operator.
 * No all NONOPTLIST-nodes are concatenated until only one node remains.
 * This node and the result node of OPTLIST replace the exprs-nodes
 * of the starting node.
 * After termination of the current recursion step all in former steps
 * manipulated and created nodes are included in the AST before the next
 * optimization call of another assign-node.
 *
 * Then optimization of the next assign node starts and work the same way...
 *
 *
 *------------------------------------------------------------------
 *------------------------------------------------------------------
 *
 * Example:
 * ********
 *
 *  a, b, c are arguments!
 *
 * ((2 * a) * c) + (3 * b) + a
 *
 *   m1 = 2 * a;
 *   m2 = m1 * c;
 *   m3 = 3 * b;
 *   a1 = m1 + m3;
 *   a2 = a1 + a;
 *
 *-------------------------------------------------------------------
 * initialization:
 *
 *   unused nodes: m3, m2, m1, a2, a1
 *
 *-------------------------------------------------------------------
 * 1. optimization step:   ** optimizing a2 **
 *
 *   investigate MOSTFREQUENTNODE:
 *
 *   COUNTLIST: (2,*):1, (c,*):1, (a,*):1, (b,*):1, (a,+):1
 *              - neutral element of * is known
 *              - (a,+):1 can be added to (a,*):1  =>   (a,*):2
 *
 *   MOSTFREQUENTNODE: a
 *   MAINOPERATOR:     +
 *   SECONDOPERATOR:   *
 *
 *   create OPTLIST and NONOPTLIST:
 *     {structure of one OPTLIST-nodelistelement: (X,[Y])
 *      X: single value  or two values containing MOSTFREQUENTNODE
 *      Y: nodes on the traversal-path to this node from begining
 *         of subtree of SECONDOPERATOR}
 *
 *   OPTLIST: ((2,a),[(m1,c)]), ((a),[])
 *   NONOPTLIST: (3,b)
 *
 *   remove MOSTFREQUENTNODE:
 *
 *   OPTLIST: ((2),[m1,c]), ((1),[])
 *
 *   concatenate nodes:
 *
 *   dl1 = 2
 *   dl2 = dl1 * c
 *   dl3 = dl2 + 1
 *
 *   dl4 = a * dl3
 *
 *   dl5 = 3 * b
 *
 *
 *  ==> unused nodes: -  => no further optimization this cycle
 *
 *-------------------------------------------------------------------
 * result:
 *
 *   m1 = 2 * a;
 *   m2 = m1 * c;
 *   m3 = 3 * b;
 *   a1 = m1 + m3;
 *   dl1 = 2
 *   dl2 = dl1 * c
 *   dl3 = dl2 + 1
 *   dl4 = a * dl3
 *   dl5 = 3 * b
 *   a2 = dl4 + dl5;
 *
 *
 *  ==> after execution of ConstantFolding and DeadCodeRemoval:
 *
 *   dl2 = 2 * c
 *   dl3 = dl2 + 1
 *   dl4 = a * dl3
 *   dl5 = 3 * b
 *   a2 = dl4 + dl5;
 *
 *   ==>   (((2 * c) + 1) * a) + (3 * b)
 *
 *------------------------------------------------------------------
 *------------------------------------------------------------------
 *
 *  !!! IMPORTANT !!!
 * DistributiveLaw could cause much deadcode. So it is important to
 * run DeadCodeRemoval subsequent to AssociativeLaw. So following optimization
 * techniques aren't working on dead code and the memory usage does not
 * explode.
 *
 *
 * usage of arg_info and accessmacros:
 * ***********************************
 *
 *  -info2:      COUNTLIST
 *                  nodelist used to collect all termination nodes to
 *                  investigate the MOSTFREQUENTNODE
 *  -info2:      OPTLIST
 *                  nodelist* used to store nodes containing MOSTFREQENTNODE
 *  -info3:      NONOPTLIST
 *                  nodelist* used to store root of subtrees without
 *                  MOSTFREQUENTNODE
 *  -dfmask[2]:  TMPLIST
 *                  nodelist* used to store path used in the subtree
 *  -dfmask[1]:  OPTIMIZEDNODES
 *                  nodelist* used to store already manipulated nodes
 *  -dfmask[0]:  TYPE
 *                  stored types-node at beginnig of optimization process.
 *                  Is used to create new types-nodes
 *  -counter:    NUMBEROFCONSTANTS
 *                  counter for the number of elements in CONSTANTLIST
 *  -varno:      NUMBEROFVARIABLES
 *                  counter for the number of elements in VARIABLELIST
 *  -node[0]:    BLOCKNODE
 *                  pointer on the current outer block node.
 *                  Is used to insert new vardec nodes
 *  -info.prf:   CURRENTPRF
 *                  store the primitive operation used in current node
 *                  the optimization is running for.
 *  -node[1]:    LETNODE
 *                  pointer on the let node of the current assign-node
 *                  the optimization is running for.
 *  -node[3]:    MOSTFREQUENTNODE
 *                  pointer to the most frequent node (this one to extract)
 *  -refcnt:     OCCURENCEOFMOSTFREQUENTNODE
 *                  counter how often MOSTFREQUENTNODE was found
 *  -node[4]:    MAINOPERATOR
 *                  pointer to operator used at starting node
 *  -node[5]:    SECONDOPERATOR
 *                  pointer to operator used in conjunction with
 *                  MOSTFREQUENTNODE
 *  -flag        STATUSSECONDOPERATOR
 *                  0: SECONDOPERATOR not reached till now
 *                  1: SECONDOPERATOR reached
 *                  2: MOSTFREQUENTNODE was found, ignore remaining subtree
 *  -varno       IEEEFLAG
 *                  is set while first traversal if float/double's are used
 *
 *
 * usage of exprs and accessmacros:
 * ***********************************
 *
 *  -flag        EXPRS_FLAG
 *                  is set when a single MOSTFREQUENTNODE is stored
 *                  (this mean: in conjunction with MAINIOPERATOR)
 *
 *
 * usage of nodelist and accessmacros:
 * ***********************************
 *
 *  -NODELIST_ATTRIB2   NODELIST_OPERATOR
 *                        used to store in COUNTLIST the used operator
 *  -NODELIST_ATTRIB2   NODELIST_PARENTNODES
 *                        used in OPTLIST to store the path to
 *                        MOSTFREQUENTNODE
 *  -NODELIST_STATUS    is set to 1 in COUNTLIST when one MOSTFREQUENTNODE
 *                      was found in the current subtree
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "globals.h"
#include "new_types.h"

#include "traverse.h"
#include "dbug.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "DupTree.h"

#include "DistributiveLaw.h"

/*
 * INFO structure
 */
struct INFO {
    types *type;
    node *blocknode;
    nodelist *optlist;
    nodelist *nonoptlist;
    nodelist *optimizednodes;
    node *letnode;
    node *mostfreqnode;
    int occmostfreqnode;
    node *mainoperator;
    node *secoperator;
    int statussecoperator;
    nodelist *countlist;
    nodelist *tmplist;
    int ieeeflag;
    ntype *newtype;
};

/*
 * INFO macros
 */
#define INFO_TYPE(n) (n->type)
#define INFO_BLOCKNODE(n) (n->blocknode)
#define INFO_OPTLIST(n) (n->optlist)
#define INFO_NONOPTLIST(n) (n->nonoptlist)
#define INFO_OPTIMIZEDNODES(n) (n->optimizednodes)
#define INFO_LETNODE(n) (n->letnode)
#define INFO_MOSTFREQUENTNODE(n) (n->mostfreqnode)
#define INFO_OCCURENCEOFMOSTFREQUENTNODE(n) (n->occmostfreqnode)
#define INFO_MAINOPERATOR(n) (n->mainoperator)
#define INFO_STATUSSECONDOPERATOR(n) (n->statussecoperator)
#define INFO_SECONDOPERATOR(n) (n->secoperator)
#define INFO_COUNTLIST(n) (n->countlist)
#define INFO_TMPLIST(n) (n->tmplist)
#define INFO_IEEEFLAG(n) (n->ieeeflag)
#define INFO_NTYPE(n) (n->newtype)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_TYPE (result) = NULL;
    INFO_BLOCKNODE (result) = NULL;
    INFO_OPTLIST (result) = NULL;
    INFO_NONOPTLIST (result) = NULL;
    INFO_OPTIMIZEDNODES (result) = NULL;
    INFO_LETNODE (result) = NULL;
    INFO_MOSTFREQUENTNODE (result) = NULL;
    INFO_OCCURENCEOFMOSTFREQUENTNODE (result) = 0;
    INFO_MAINOPERATOR (result) = NULL;
    INFO_STATUSSECONDOPERATOR (result) = 0;
    INFO_SECONDOPERATOR (result) = NULL;
    INFO_COUNTLIST (result) = NULL;
    INFO_TMPLIST (result) = NULL;
    INFO_IEEEFLAG (result) = 0;
    INFO_NTYPE (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/*****************************************************************************
 *
 * function:
 *   node *GetNeutralElement(node *op, info *arg_info)
 *
 * description:
 *   returns new exprs-node, with neutral element of the primitive
 *   operation 'op'. The neutral element is of correct basetype.
 *
 ****************************************************************************/

static node *
GetNeutralElement (node *op, info *arg_info)
{
    node *neutral_elem = NULL;
    simpletype basetype;

    DBUG_ENTER ("GetNeutralElement");

    DBUG_ASSERT (NODE_TYPE (op) == N_prf, "Only N_prf node are supported!");

    DBUG_ASSERT (TYisArray (INFO_NTYPE (arg_info)), "Non-array type encountered!");
    basetype = TYgetSimpleType (TYgetScalar (INFO_NTYPE (arg_info)));

    switch (PRF_PRF (op)) {
    case F_add_SxS:
    case F_add_VxS:
    case F_add_SxV:
    case F_add_VxV:
        switch (basetype) {
        case T_int:
            neutral_elem = TBmakeNum (0);
            break;
        case T_float:
            neutral_elem = TBmakeFloat (0.0f);
            break;
        case T_double:
            neutral_elem = TBmakeDouble (0.0);
            break;
        default:
            break;
        }
        break;

    case F_mul_SxS:
    case F_mul_VxS:
    case F_mul_SxV:
    case F_mul_VxV:
        switch (basetype) {
        case T_int:
            neutral_elem = TBmakeNum (1);
            break;
        case T_float:
            neutral_elem = TBmakeFloat (1.0f);
            break;
        case T_double:
            neutral_elem = TBmakeDouble (1.0);
            break;
        default:
            break;
        }
        break;

    case F_and_SxS:
        neutral_elem = TBmakeBool (TRUE);
        break;

    case F_or_SxS:
#if 0
    /* 
     * commented out as it was not implemented in original implementation
     */
     neutral_elem = TBmakeBool( FALSE);
     break;
#endif

    default:
        DBUG_ASSERT (FALSE, "Not supported primitive operation!"
                            "No neutral element known!");
    }

    DBUG_RETURN (neutral_elem);
}

/*****************************************************************************
 *
 * function:
 *   bool ExistKnownNeutralElement(node *op)
 *
 * description:
 *   returns TRUE, if a neutral element exist for prf/ap 'op';
 *   otherwise FALSE.
 *
 ****************************************************************************/

static bool
ExistKnownNeutralElement (node *op)
{

    bool ret;

    DBUG_ENTER ("ExistKnownNeutralElement");

    switch (NODE_TYPE (op)) {
    case N_prf:
        switch (PRF_PRF (op)) {
        case F_add_SxS:
        case F_add_SxV:
        case F_add_VxS:
        case F_add_VxV:
        case F_mul_SxS:
        case F_mul_SxV:
        case F_mul_VxS:
        case F_mul_VxV:
        case F_and_SxS:
        case F_and_SxV:
        case F_and_VxS:
        case F_and_VxV:
        case F_or_SxS:
        case F_or_SxV:
        case F_or_VxS:
        case F_or_VxV:
            ret = TRUE;
            break;
        default:
            ret = FALSE;
        }
    case N_ap:
        ret = FALSE;
        break;
    default:
        DBUG_ASSERT (FALSE, "Unexpected node! Only N_prf/N_ap nodes accepted!");
        ret = FALSE;
    }

    DBUG_RETURN (ret);
}

/*****************************************************************************
 *
 * function:
 *   bool IsConstant(node *arg_node, info *arg_info)
 *
 * description:
 *   returns TRUE, if arg_node is a constant node, otherwise FALSE.
 *   If the node contains a double or float value, the IEEEFLAG is set, so
 *   this optimization don't continue later, if the enforceIEEE-flag is set!
 *
 ****************************************************************************/

static bool
IsConstant (node *arg_node, info *arg_info)
{
    bool is_constant;

    DBUG_ENTER ("IsConstant");

    is_constant = FALSE;

    switch (NODE_TYPE (arg_node)) {
    case N_float:
    case N_double:
        INFO_IEEEFLAG (arg_info) = 1;
    case N_bool:
    case N_num:
    case N_array:
    case N_char:
        is_constant = TRUE;
        break;
    default:
        is_constant = FALSE;
    }
    DBUG_RETURN (is_constant);
}

/*****************************************************************************
 *
 * function:
 *   bool CheckOperator(node *arg_node, info *arg_info)
 *
 * description:
 *   Returns TRUE if 'operator' is supported. This means, 'operator' has to
 *   be associative, commutative.
 *
 * notes:
 *   Until now the function differentiate between prf and ap nodes.
 *   Primitive functions are given. If later user-defined-functions are
 *   also supported in this optimization, they can be added here or the
 *   method call can be placed here!
 *
 ****************************************************************************/

static bool
CheckOperator (node *operator, info *arg_info )
{

    bool support;

    DBUG_ENTER ("CheckOperator");

    support = FALSE;

    if (NODE_TYPE (operator) == N_prf) {

        switch (PRF_PRF (operator)) {
        case F_add_SxS:
        case F_add_VxS:
        case F_add_SxV:
        case F_add_VxV:
        case F_mul_SxS:
        case F_mul_VxS:
        case F_mul_SxV:
        case F_mul_VxV:
        case F_and_SxS:
        case F_and_SxV:
        case F_and_VxS:
        case F_and_VxV:
        case F_or_SxS:
        case F_or_SxV:
        case F_or_VxS:
        case F_or_VxV:
        case F_max_SxS:
        case F_max_SxV:
        case F_max_VxS:
        case F_max_VxV:
        case F_min_SxS:
        case F_min_SxV:
        case F_min_VxS:
        case F_min_VxV:
            support = TRUE;
            break;
        default:
            support = FALSE;
        }
    } else if (NODE_TYPE (operator) == N_ap) {

    } else {
        DBUG_ASSERT (FALSE, "Unexpected operator detected");
    }

    DBUG_RETURN (support);
}

static node *
GetUsedOperator (node *arg_node, info *arg_info)
{

    node *operator;

    DBUG_ENTER ("GetUsedOperator");

    operator= LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (arg_node))));

    DBUG_RETURN (operator);
}

/*****************************************************************************
 *
 * function:
 *   bool IsSupportedOperator(node *arg_node, info *arg_info)
 *
 * description:
 *   returns TRUE, if the used operator in the definition of arg_node
 *   fulfil the conditions of 'CheckOperator'.
 *
 ****************************************************************************/

static bool
IsSupportedOperator (node *arg_node, info *arg_info)
{

    bool support;
    node *operator;

    DBUG_ENTER ("IsSupportedOperator");
    support = FALSE;

    operator= GetUsedOperator (arg_node, arg_info);

    support = CheckOperator (operator, arg_info );

    DBUG_RETURN (support);
}

/*****************************************************************************
 *
 * function:
 *   int GetPriority(node *operator)
 *
 * description:
 *   Returns the priority of the 'operator'.
 *
 * notes:
 *   VERY UGLY FUNCTION!!!!
 *   The priotities are choosed by best conscience. Should be removed after
 *   a better algorithm to investigate the validity of distributive law for
 *   a pair of 'operator' is known.
 *
 ****************************************************************************/

static int
GetPriority (node *operator)
{

    int priority;

    DBUG_ENTER ("GetPriority");

    if (NODE_TYPE (operator) == N_prf) {

        switch (PRF_PRF (operator)) {
        case F_add_SxS:
        case F_add_VxS:
        case F_add_VxV:
        case F_add_SxV:
            priority = 6;
            break;
        case F_mul_SxS:
        case F_mul_VxS:
        case F_mul_VxV:
        case F_mul_SxV:
            priority = 7;
            break;
        case F_max_SxS:
        case F_max_SxV:
        case F_max_VxS:
        case F_max_VxV:
        case F_min_SxS:
        case F_min_SxV:
        case F_min_VxS:
        case F_min_VxV:
            priority = 5;
            break;
        case F_and_SxS:
        case F_and_SxV:
        case F_and_VxS:
        case F_and_VxV:
            priority = 5;
            break;
        case F_or_SxS:
        case F_or_SxV:
        case F_or_VxS:
        case F_or_VxV:
            priority = 4;
            break;
        default:
            DBUG_ASSERT (FALSE, "Unexpected primitive operation");
            priority = 0;
        }

    } else if (NODE_TYPE (operator) == N_ap) {

        priority = 0;

    } else {

        DBUG_ASSERT (FALSE, "Wrong node! N_prf or N_ap expected!");
        priority = 0;
    }

    DBUG_RETURN (priority);
}

/*****************************************************************************
 *
 * function:
 *   bool IsIdenticalNode(node *node1, node *node2)
 *
 * description:
 *   returns TRUE, if both arguments refer to the same source-node or if
 *   they are constant and have the same values!
 *
 ****************************************************************************/

static bool
IsIdenticalNode (node *node1, node *node2)
{

    bool result;

    DBUG_ENTER ("IsIdenticalNode");

    result = FALSE;

    if (NODE_TYPE (node1) == NODE_TYPE (node2)) {

        if (((NODE_TYPE (node1)) == N_id) && (ID_AVIS (node1) == ID_AVIS (node2)))
            result = TRUE;

        if (((NODE_TYPE (node1)) == N_num) && (NUM_VAL (node1) == NUM_VAL (node2)))
            result = TRUE;

        if (((NODE_TYPE (node1)) == N_char) && (CHAR_VAL (node1) == CHAR_VAL (node2)))
            result = TRUE;

        if (((NODE_TYPE (node1)) == N_float) && (FLOAT_VAL (node1) == FLOAT_VAL (node2)))
            result = TRUE;

        if (((NODE_TYPE (node1)) == N_double)
            && (DOUBLE_VAL (node1) == DOUBLE_VAL (node2)))
            result = TRUE;

        if (((NODE_TYPE (node1)) == N_bool) && (BOOL_VAL (node1) == BOOL_VAL (node2)))
            result = TRUE;
    }

    DBUG_RETURN (result);
}

/*****************************************************************************
 *
 * function:
 *   bool IsValidSecondOperator(node *arg_node, info *arg_info)
 *
 * description:
 *   returns TRUE, if the the used operator in the definition of arg_node
 *   is supported and his priority is higher than the priority of
 *   MAINOPERATOR.
 *
 ****************************************************************************/

static bool
IsValidSecondOperator (node *arg_node, info *arg_info)
{

    bool valid;
    int priority_firstop;
    int priority_secondop;
    node *secondop;
    node *firstop;

    DBUG_ENTER ("IsValidSecondOperator");

    valid = FALSE;

    if (IsSupportedOperator (arg_node, arg_info)) {

        firstop = INFO_MAINOPERATOR (arg_info);
        secondop = GetUsedOperator (arg_node, arg_info);

        priority_firstop = GetPriority (firstop);
        priority_secondop = GetPriority (secondop);

        if (priority_secondop > priority_firstop)
            valid = TRUE;
    }
    DBUG_RETURN (valid);
}

/*****************************************************************************
 *
 * function:
 *   bool IsSameOperator(node *firstop, node *secondop)
 *
 * description:
 *   returns TRUE, both operators are equal.
 *
 ****************************************************************************/

static bool
IsSameOperator (node *firstop, node *secondop)
{

    bool is_same;

    DBUG_ENTER ("IsSameOperator");
    is_same = FALSE;

    if (NODE_TYPE (firstop) == NODE_TYPE (secondop)) {

        if (NODE_TYPE (firstop) == N_prf) {

            switch (PRF_PRF (firstop)) {
            case F_add_SxS:
            case F_add_VxS:
            case F_add_SxV:
            case F_add_VxV: {
                switch (PRF_PRF (secondop)) {
                case F_add_SxS:
                case F_add_VxS:
                case F_add_SxV:
                case F_add_VxV: {
                    is_same = TRUE;
                } break;
                default:
                    is_same = FALSE;
                }
            } break;
            case F_mul_SxS:
            case F_mul_VxS:
            case F_mul_SxV:
            case F_mul_VxV: {
                switch (PRF_PRF (secondop)) {
                case F_mul_SxS:
                case F_mul_VxS:
                case F_mul_SxV:
                case F_mul_VxV: {
                    is_same = TRUE;
                } break;
                default:
                    is_same = FALSE;
                }
            } break;
            default: {
                if (PRF_PRF (firstop) == PRF_PRF (secondop))
                    is_same = TRUE;
            }
            }
        } else if (NODE_TYPE (firstop) == N_ap) {

            if (AP_FUNDEF (firstop) == AP_FUNDEF (secondop))
                is_same = TRUE;

        } else {

            DBUG_ASSERT (FALSE, "Wrong node! N_prf or N_ap expected!");
        }
    }

    DBUG_RETURN (is_same);
}

/*****************************************************************************
 *
 * function:
 *   bool IsThirdOperatorReached(node *arg_node, info *arg_info)
 *
 * description:
 *   returns TRUE if the operator used in the definition of arg_node is not
 *   the same operator as SECONDOPERATOR.
 *
 ****************************************************************************/

static bool
IsThirdOperatorReached (node *arg_node, info *arg_info)
{

    bool is_thirdop;
    node *operator;

    DBUG_ENTER ("IsThirdOperatorReached");

    is_thirdop = FALSE;

    if ((INFO_STATUSSECONDOPERATOR (arg_info) != 0)) {

        operator= GetUsedOperator (arg_node, arg_info);

        is_thirdop = !(IsSameOperator (operator, INFO_SECONDOPERATOR (arg_info)));
    }
    DBUG_RETURN (is_thirdop);
}

/*****************************************************************************
 *
 * function:
 *   bool ReachedArgument(node *arg_node)
 *
 * description:
 *   returns TRUE if the arg_node is an argument defined outside current
 *   block node
 *
 ****************************************************************************/

static bool
ReachedArgument (node *arg_node)
{

    bool reached;
    DBUG_ENTER ("ReachedArgument");

    if (NODE_TYPE (arg_node) == N_id) {
        if (AVIS_SSAASSIGN (ID_AVIS (arg_node)) == NULL)
            reached = TRUE;
        else
            reached = FALSE;
    } else
        reached = FALSE;

    DBUG_RETURN (reached);
}

/*****************************************************************************
 *
 * function:
 *   bool ReachedDefinition(node *arg_node)
 *
 * description:
 *   returns TRUE if the definition of the arg_node contain no prf-node
 *
 ****************************************************************************/

static bool
ReachedDefinition (node *arg_node)
{

    bool reached;
    DBUG_ENTER ("ReachedDefinition");

    if (NODE_TYPE (arg_node) == N_id) {
        if (AVIS_SSAASSIGN (ID_AVIS (arg_node)) == NULL) {
            reached = FALSE;
        } else {
            if (NODE_TYPE (LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (arg_node)))))
                != N_prf) {
                reached = TRUE;
            } else {
                reached = FALSE;
            }
        }
    } else {
        reached = FALSE;
    }

    DBUG_RETURN (reached);
}

/*****************************************************************************
 *
 * function:
 *   node *ResetFlags(info *arg_info)
 *
 * description:
 *   All NODELIST_FLAGS in COUNTLIST are set to 0.
 *
 ****************************************************************************/

static void
ResetFlags (info *arg_info)
{

    nodelist *list;

    DBUG_ENTER ("ResetFlags");

    list = INFO_COUNTLIST (arg_info);

    while (list != NULL) {

        NODELIST_STATUS (list) = 0;
        list = NODELIST_NEXT (list);
    }
    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   node *RegisterMultipleUsableNodes(info *arg_info)
 *
 * description:
 *   If a collected node x in the COUNTLIST is directly connected with the
 *   MAINOPERATOR and it also exist node x with SECONDOPERATOR with a known
 *   neutral element in COUNTLIST, so the number of collected nodes for x with
 *   MAINOPERATOR can be added to the node x with SECONDOPERATOR.
 *
 * example:
 *     f = (a * b) + (a * c) + a
 *
 *      => COUNTLIST: (node,operator): count
 *                    (a,*): 2
 *                    (a,+): 1
 *                    (b,*): 1
 *                    (c,*): 1
 *
 *      => MAINOPERATOR: +
 *      => neutral element of * known?: yes
 *
 *      => new COUNTLIST:
 *                    (a,*): 3
 *                    (b,*): 1
 *                    (c,*): 1
 *
 *      => advantage: all possible optimization cases are considered!
 *
 *
 ****************************************************************************/

static void
RegisterMultipleUsableNodes (info *arg_info)
{

    nodelist *currentlist, *tmplist;
    node *currentnode;

    DBUG_ENTER ("RegisterMultipleUsableNodes");

    currentlist = INFO_COUNTLIST (arg_info);

    /*
     * traverse whole COUNTLIST
     */
    while (currentlist != NULL) {

        /*
         * compare operator in COUNTLIST with MAINOPERATOR
         */
        if (IsSameOperator (INFO_MAINOPERATOR (arg_info),
                            DL_NODELIST_OPERATOR (currentlist))) {

            currentnode = NODELIST_NODE (currentlist);

            tmplist = INFO_COUNTLIST (arg_info);

            /*
             * traverse whole COUNTLIST again
             */
            while (tmplist != NULL) {

                if (tmplist != currentlist) {

                    if (IsIdenticalNode (NODELIST_NODE (tmplist), currentnode)) {

                        /*
                         * different operator, but identical node found
                         */

                        if (ExistKnownNeutralElement (DL_NODELIST_OPERATOR (tmplist))) {

                            NODELIST_INT (tmplist)
                              = NODELIST_INT (tmplist) + NODELIST_INT (currentlist);
                        }
                    }
                }

                tmplist = NODELIST_NEXT (tmplist);
            }
        }

        currentlist = NODELIST_NEXT (currentlist);
    }

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   node *AddNodeToOptimizedNodes(node *arg, info *arg_info)
 *
 * description:
 *   The function create a new NodelistNode with 'arg' as NODELIST_NODE
 *   and add it to OPTIMIZEDNODES.
 *
 ****************************************************************************/

static void
AddNodeToOptimizedNodes (node *arg, info *arg_info)
{
    nodelist *newlist;

    DBUG_ENTER ("AddNodeToOptimizedNodes");

    newlist = TBmakeNodelistNode (arg, NULL);

    if (INFO_OPTIMIZEDNODES (arg_info) == NULL) {

        INFO_OPTIMIZEDNODES (arg_info) = newlist;

    } else {

        NODELIST_NEXT (newlist) = INFO_OPTIMIZEDNODES (arg_info);
        INFO_OPTIMIZEDNODES (arg_info) = newlist;
    }

    DBUG_VOID_RETURN;
}

static bool
IsAnArray (node *expr, info *arg_info)
{

    bool result;

    DBUG_ENTER ("IsAnArray");

    result = TRUE;

    if (IsConstant (expr, arg_info))
        result = FALSE;
    else if (N_id == NODE_TYPE (expr)) {

        result = TYisArray (AVIS_TYPE (ID_AVIS (expr)));
    } else {
        DBUG_ASSERT ((FALSE), "Unexpected EXPRS_EXPR node!");
    }

    DBUG_RETURN (result);
}

/*****************************************************************************
 *
 * function:
 *   node *MakeOperatorNode(node *exprs, node *op)
 *
 * description:
 *   This function creates and returns a new prf or ap node with the operator
 *   'op' and with exprs as the arguments.
 *   'optype' shows which operands are scalar or arrays!
 *   If 'optype' is 3: VxV
 *   If 'optype' is 2: VxS
 *   If 'optype' is 1: SxV
 *   If 'optype' is 0: SxS
 *
 ****************************************************************************/

static node *
MakeOperatorNode (node *exprs, node *op, int optype)
{

    node *new_op_node;

    DBUG_ENTER ("MakeOperatorNode");

    new_op_node = NULL;

    if (NODE_TYPE (op) == N_prf) {

        if ((PRF_PRF (op) == F_add_SxS) || (PRF_PRF (op) == F_add_SxV)
            || (PRF_PRF (op) == F_add_VxS) || (PRF_PRF (op) == F_add_VxV)) {

            if (optype == 0) {
                new_op_node = TBmakePrf (F_add_SxS, exprs);
            }
            if (optype == 1) {
                new_op_node = TBmakePrf (F_add_SxV, exprs);
            }
            if (optype == 2) {
                new_op_node = TBmakePrf (F_add_VxS, exprs);
            }
            if (optype == 3) {
                new_op_node = TBmakePrf (F_add_VxV, exprs);
            }
        } else if ((PRF_PRF (op) == F_mul_SxS) || (PRF_PRF (op) == F_mul_SxV)
                   || (PRF_PRF (op) == F_mul_VxS) || (PRF_PRF (op) == F_mul_VxV)) {

            if (optype == 0) {
                new_op_node = TBmakePrf (F_mul_SxS, exprs);
            }
            if (optype == 1) {
                new_op_node = TBmakePrf (F_mul_SxV, exprs);
            }
            if (optype == 2) {
                new_op_node = TBmakePrf (F_mul_VxS, exprs);
            }
            if (optype == 3) {
                new_op_node = TBmakePrf (F_mul_VxV, exprs);
            }
        } else {
            new_op_node = TBmakePrf (PRF_PRF (op), exprs);
        }
    } else if (NODE_TYPE (op) == N_ap) {

        new_op_node = TBmakeAp (AP_FUNDEF (op), exprs);
    } else {

        DBUG_ASSERT (FALSE, "Unexpected node! Only N_prf or N_ap node are excepted!");
    }

    DBUG_RETURN (new_op_node);
}

/*****************************************************************************
 *
 * function:
 *   node *CreateExprsNode(node *currentElem)
 *
 * description:
 *   The currentElem is a constant nodes or id node.
 *   The currentElem will be duplicated and the new nodes will replace the
 *   original node.
 *
 ****************************************************************************/

static node *
MakeExprNodes (node *currentElem)
{
    DBUG_ENTER ("MakeExprNodes");

    currentElem = DUPdoDupTree (currentElem);      /*pointer on duplicated node */
    currentElem = TBmakeExprs (currentElem, NULL); /* create N_expr-node */

    DBUG_RETURN (currentElem);
}

/*****************************************************************************
 *
 * function:
 *   node *MakeExprsNodeFromAssignNode(node *elem1)
 *
 * description:
 *   The assign-nodes get argument of new id-node. This id-node
 *   gets an argument of a new exprs-node.
 *
 ****************************************************************************/

static node *
MakeExprsNodeFromAssignNode (node *elem1)
{

    node *newnode;

    DBUG_ENTER ("MakeExprsNodeFromAssignNode");

    newnode = TBmakeExprs (DUPdupIdsId (LET_IDS (ASSIGN_INSTR (elem1))), NULL);

    DBUG_RETURN (newnode);
}

/*****************************************************************************
 *
 * function:
 *   node *MakeAssignLetNodeFromCurrentNode( node *newnode , info *arg_info)
 *
 * description:
 *   This function create a new assign-node with the newnode as an
 *   argument. The correct vardec-root-node is provided
 *   by the arg_info-node.
 *   If 'flag' is 0: newnode contains scalar values
 *   If 'flag' is 1: newnode contains an array
 *
 ****************************************************************************/

static node *
MakeAssignLetNodeFromCurrentNode (node *newnode, info *arg_info, int flag)
{

    node *newvardec, *newavis;
    ntype *type;

    DBUG_ENTER ("MakeAssignNodeFromExprsNode");

    type = TYcopyType (INFO_NTYPE (arg_info));

    newavis = TBmakeAvis (TRAVtmpVar (), type);

    newvardec = TBmakeVardec (newavis, (BLOCK_VARDEC (INFO_BLOCKNODE (arg_info))));

    BLOCK_VARDEC (INFO_BLOCKNODE (arg_info)) = newvardec;

    newnode = TBmakeAssign (TBmakeLet (TBmakeIds (newavis, NULL), newnode), NULL);

    AVIS_SSAASSIGN (newavis) = newnode;

    DBUG_RETURN (newnode);
}

/*****************************************************************************
 *
 * function:
 *   node *MakeAllNodelistnodesToAssignNodes( nodelist* list, info *arg_info)
 *
 * description:
 *   All NODELIST_NODES in list are replaced by an assign-node, with the
 *   original node as an argument. The original nodes have to be
 *   id-nodes or other constant nodes!
 *
 ****************************************************************************/

static void
MakeAllNodelistnodesToAssignNodes (nodelist *list, info *arg_info)
{

    node *node1, *newnode;

    DBUG_ENTER ("MakeAllNodelistnodesToAssignNodes");

    while (list != NULL) {

        if (NODE_TYPE (NODELIST_NODE (list)) != N_assign) {

            node1 = (NODELIST_NODE (list));

            DBUG_ASSERT ((IsConstant (node1, arg_info) || (N_id == NODE_TYPE (node1))),
                         "Unexpected node! No supported EXPR_EXPRS result");

            newnode = DUPdoDupTree (node1);

            if (IsAnArray (newnode, arg_info)) {
                newnode = MakeAssignLetNodeFromCurrentNode (newnode, arg_info, 1);
            } else {
                newnode = MakeAssignLetNodeFromCurrentNode (newnode, arg_info, 0);
            }
            NODELIST_NODE (list) = newnode;
        }
        list = NODELIST_NEXT (list);
    }

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   node *DeleteNodelist( nodelist* list)
 *
 * description:
 *   All attributes of list are removed from nodelist - but not deleted.
 *   But all allocated memory from nodelist-nodes is freed.
 *
 ****************************************************************************/

static nodelist *
DeleteNodelist (nodelist *list)
{

    nodelist *tmp;

    DBUG_ENTER ("DeleteNodelist");

    tmp = list;

    while (tmp != NULL) {

        NODELIST_NODE (tmp) = NULL;
        NODELIST_ATTRIB2 (tmp) = NULL;
        tmp = FREEfreeNodelistNode (tmp);
    }

    DBUG_RETURN (tmp);
}

/*****************************************************************************
 *
 * function:
 *   node *CreateAssignNodes(info *arg_info)
 *
 * description:
 *   All NODELIST_NODE's of NONOPTLIST are expanded to assign-nodes
 *
 ****************************************************************************/

static void
CreateAssignNodes (info *arg_info)
{

    nodelist *list;
    DBUG_ENTER ("CreateAssignNodes");

    list = INFO_NONOPTLIST (arg_info);

    if (list != NULL) {

        MakeAllNodelistnodesToAssignNodes (INFO_NONOPTLIST (arg_info), arg_info);
    }
    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   nodelist* CommitAssignNodes(nodelist *list, info *arg_info)
 *
 * description:
 *   all NODELIST_NODE's in list are assign-nodes. Every two assign nodes
 *   becomes argument of a new assign nodes (with MAINOPERATOR) and the new
 *   node is append to the list. The two argument-nodes are added to
 *   OPTIMIZEDNODES. The execution terminates if one node remain.
 *
 *   example:
 *      nodes in nodelist: |a, b, c, d
 *      1. step:  e = a op b    => new nodelist: a, b, |c, d, e
 *      2. step:  f = c op d    => new nodelist: a, b, c, d, |e, f
 *      3. step:  g = e op f    => new nodelist: a, b, c, d, e, f, |g
 *
 ****************************************************************************/

static nodelist *
CommitAssignNodes (nodelist *list, info *arg_info)
{

    nodelist *lastnodelistnode, *tmplist, *startlist;
    node *node1, *node2, *newnode, *tmpnode;

    DBUG_ENTER ("CommitAssignNodes");

    startlist = list;

    /*
     * check if list is empty
     */
    if (list != NULL) {

        lastnodelistnode = list;

        /*
         * find last node in list
         */
        while (NODELIST_NEXT (lastnodelistnode) != NULL) {

            lastnodelistnode = NODELIST_NEXT (lastnodelistnode);
        }
        /*
         * Create a new assign node with MAINOPERATOR and
         * the two nodes of the next two NODELIST_NODE's as arguments.
         * The new node is append at the end of the list and get the
         * new lastnodelistnode. The two nodes used as arguments are append
         * to OPTIMIZEDNODES.
         */
        while (list != lastnodelistnode) {

            /*
             * get both arguments
             */
            node1 = NODELIST_NODE (list);
            node2 = NODELIST_NODE (NODELIST_NEXT (list));

            /*
             * create new assign node
             */
            tmpnode = MakeExprsNodeFromAssignNode (node1);
            newnode = MakeExprsNodeFromAssignNode (node2);
            EXPRS_NEXT (newnode) = tmpnode;

            if (IsAnArray (EXPRS_EXPR (newnode), arg_info))
                if (IsAnArray (EXPRS_EXPR (tmpnode), arg_info)) {

                    newnode = MakeOperatorNode (newnode, INFO_MAINOPERATOR (arg_info), 3);
                    newnode = MakeAssignLetNodeFromCurrentNode (newnode, arg_info, 1);

                } else {

                    newnode = MakeOperatorNode (newnode, INFO_MAINOPERATOR (arg_info), 2);
                    newnode = MakeAssignLetNodeFromCurrentNode (newnode, arg_info, 1);
                }
            else if (IsAnArray (EXPRS_EXPR (tmpnode), arg_info)) {

                newnode = MakeOperatorNode (newnode, INFO_MAINOPERATOR (arg_info), 1);
                newnode = MakeAssignLetNodeFromCurrentNode (newnode, arg_info, 1);
            } else {

                newnode = MakeOperatorNode (newnode, INFO_MAINOPERATOR (arg_info), 0);
                newnode = MakeAssignLetNodeFromCurrentNode (newnode, arg_info, 0);
            }

            /*
             * add used arguments to OPTIMIZEDNODES
             */
            AddNodeToOptimizedNodes (node1, arg_info);
            AddNodeToOptimizedNodes (node2, arg_info);

            /*
             * append new node on list
             */
            tmplist = TBmakeNodelistNode (newnode, NULL);
            NODELIST_NEXT (lastnodelistnode) = tmplist;
            lastnodelistnode = NODELIST_NEXT (lastnodelistnode);

            list = NODELIST_NEXT (NODELIST_NEXT (list));
        }

        /*
         * delete nodelist - except last element
         * last element remains as first element
         */
        while (startlist != lastnodelistnode) {

            tmplist = startlist;
            startlist = NODELIST_NEXT (startlist);

            NODELIST_NEXT (tmplist) = NULL;
            tmplist = DeleteNodelist (tmplist);
        }
    }
    DBUG_RETURN (startlist);
}

/*****************************************************************************
 *
 * function:
 *   node *IncludeMostFrequentNode(info *arg_info)
 *
 * description:
 *   connect MOSTFREQUENNODE with the remaining node in OPTLIST with operator
 *   SECONDOPERATOR.
 *   Add last node from OPTLIST to OPTIMIZEDNODE's.
 *   The new node remains in OPTLIST.
 *
 ****************************************************************************/

static void
IncludeMostFrequentNode (info *arg_info)
{

    node *node1, *node2, *newnode, *tmpnode;

    DBUG_ENTER ("IncludeMostFrequentNode");

    node1 = INFO_MOSTFREQUENTNODE (arg_info);
    node1 = TBmakeExprs (node1, NULL);

    tmpnode = NODELIST_NODE (INFO_OPTLIST (arg_info));
    node2 = MakeExprsNodeFromAssignNode (tmpnode);
    EXPRS_NEXT (node2) = node1;

    if (IsAnArray (EXPRS_EXPR (node1), arg_info))
        if (IsAnArray (EXPRS_EXPR (node2), arg_info)) {

            newnode = MakeOperatorNode (node2, INFO_SECONDOPERATOR (arg_info), 3);
            newnode = MakeAssignLetNodeFromCurrentNode (newnode, arg_info, 1);
        } else {

            newnode = MakeOperatorNode (node2, INFO_SECONDOPERATOR (arg_info), 1);
            newnode = MakeAssignLetNodeFromCurrentNode (newnode, arg_info, 1);
        }
    else

      if (IsAnArray (EXPRS_EXPR (node2), arg_info)) {

        newnode = MakeOperatorNode (node2, INFO_SECONDOPERATOR (arg_info), 2);
        newnode = MakeAssignLetNodeFromCurrentNode (newnode, arg_info, 1);
    } else {

        newnode = MakeOperatorNode (node2, INFO_SECONDOPERATOR (arg_info), 0);
        newnode = MakeAssignLetNodeFromCurrentNode (newnode, arg_info, 0);
    }
    AddNodeToOptimizedNodes (tmpnode, arg_info);

    NODELIST_NODE (INFO_OPTLIST (arg_info)) = newnode;

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   node *IntegrateResults(info *arg_info)
 *
 * description:
 *   First the remaining assign-node in OPTLIST is the source for a new
 *   exprs-node and than added to OPTIMIZEDNODES.
 *   If in NONOPTLIST exist a remaining node the same procedure is done and
 *   the two new exprs-nodes become the new arguments of the operator of the
 *   starting-node of this optimization-cycle.
 *   If no node in NONOPTLIST exists, than the opeartor node is removed
 *   and replaced by the new id node.
 *   So the begining of the optimized structure is included in the tree.
 *
 ****************************************************************************/

static void
IntegrateResults (info *arg_info)
{
    node *original, *old_nonoptnode, *old_optnode, *new_optnode, *new_nonoptnode,
      *oldargs;

    DBUG_ENTER ("IntegrateResults");

    /*
     * create exprs node from remaining node in OPTLIST
     */
    original = INFO_LETNODE (arg_info);
    old_optnode = NODELIST_NODE (INFO_OPTLIST (arg_info));
    new_optnode = MakeExprsNodeFromAssignNode (old_optnode);
    AddNodeToOptimizedNodes (old_optnode, arg_info);

    new_nonoptnode = NULL;

    /*
     * check if there is a remaining node in NONOPTLIST
     */
    if (NULL != INFO_NONOPTLIST (arg_info)) {

        /*
         * create exprs node from remaining node in NONOPTLIST
         */
        old_nonoptnode = NODELIST_NODE (INFO_NONOPTLIST (arg_info));
        new_nonoptnode = MakeExprsNodeFromAssignNode (old_nonoptnode);
        AddNodeToOptimizedNodes (old_nonoptnode, arg_info);

        oldargs = AP_OR_PRF_ARGS (LET_EXPR (original));

        /*
         * include new two exprs nodes as arguments of operator
         * in the starting node of optimization-cycle
         */
        original = LET_EXPR (original);
        EXPRS_NEXT (new_optnode) = new_nonoptnode;
        if (NODE_TYPE (original) == N_ap)
            AP_ARGS (original) = new_optnode;
        else {
            PRF_ARGS (original) = new_optnode;

            if (IsAnArray (EXPRS_EXPR (new_optnode), arg_info)) {
                if (IsAnArray (EXPRS_EXPR (new_nonoptnode), arg_info)) {
                    if ((PRF_PRF (original) == F_add_SxS)
                        || (PRF_PRF (original) == F_add_VxS)
                        || (PRF_PRF (original) == F_add_SxV)) {
                        PRF_PRF (original) = F_add_VxV;
                    }
                    if ((PRF_PRF (original) == F_mul_SxS)
                        || (PRF_PRF (original) == F_mul_VxS)
                        || (PRF_PRF (original) == F_mul_SxV)) {
                        PRF_PRF (original) = F_mul_VxV;
                    }
                } else {
                    if ((PRF_PRF (original) == F_add_SxS)
                        || (PRF_PRF (original) == F_add_VxV)
                        || (PRF_PRF (original) == F_add_SxV)) {
                        PRF_PRF (original) = F_add_VxS;
                    }
                    if ((PRF_PRF (original) == F_mul_SxS)
                        || (PRF_PRF (original) == F_mul_VxV)
                        || (PRF_PRF (original) == F_mul_SxV)) {
                        PRF_PRF (original) = F_mul_VxS;
                    }
                }
            } else {
                if (IsAnArray (EXPRS_EXPR (new_nonoptnode), arg_info)) {
                    if ((PRF_PRF (original) == F_add_SxS)
                        || (PRF_PRF (original) == F_add_VxS)
                        || (PRF_PRF (original) == F_add_VxV)) {
                        PRF_PRF (original) = F_add_SxV;
                    }
                    if ((PRF_PRF (original) == F_mul_SxS)
                        || (PRF_PRF (original) == F_mul_VxS)
                        || (PRF_PRF (original) == F_mul_VxV)) {
                        PRF_PRF (original) = F_mul_SxV;
                    }
                } else {
                    if ((PRF_PRF (original) == F_add_VxV)
                        || (PRF_PRF (original) == F_add_VxV)
                        || (PRF_PRF (original) == F_add_SxV)) {
                        PRF_PRF (original) = F_add_SxS;
                    }
                    if ((PRF_PRF (original) == F_mul_VxV)
                        || (PRF_PRF (original) == F_mul_VxV)
                        || (PRF_PRF (original) == F_mul_SxV)) {
                        PRF_PRF (original) = F_mul_SxS;
                    }
                }
            }
        }
    } else {

        /*
         * only one node to include - remove operator node
         */
        oldargs = LET_EXPR (original);
        LET_EXPR (original) = EXPRS_EXPR (new_optnode);
    }
    FREEdoFreeTree (oldargs);

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   node *CheckNode(node *arg_node, info *arg_info)
 *
 * description:
 *   This function check if arg_node is equal to MOSTFREQUENTNODE.
 *   The head of TMPLIST contains arg_node. The head of TMPLIST is added
 *   to OPTLIST, all other nodes in TMPLIST are added PARENTNODES,
 *   which is another argument of the former head of TMPLIST.
 *   PARENTNODES cotains the nodes which are traversed from the first
 *   occurence of SECONDOPERATOR till arg_node is reached.
 *
 ****************************************************************************/

static void
CheckNode (node *arg_node, info *arg_info)
{

    nodelist *head;

    DBUG_ENTER ("CheckNode");

    if (IsIdenticalNode (arg_node, INFO_MOSTFREQUENTNODE (arg_info))) {

        /*
         * one MOSTFREQUENTNODE found
         * add first node in TMPLIST to OPTLIST
         * add other nodes to 'attrib2' of first node in a nodelist
         */

        /*
         * head is node with MOSTFREQUENTNODE, remove node from TMPLIST
         */
        head = INFO_TMPLIST (arg_info);
        INFO_TMPLIST (arg_info) = NODELIST_NEXT (INFO_TMPLIST (arg_info));

        /*
         * add following nodes to PARENTNODES of head-node
         * DL_NODELIST_PARENTNODES( head) = INFO_TMPLIST( arg_info);
         */
        NODELIST_ATTRIB2 (head) = (node *)INFO_TMPLIST (arg_info);

        /*
         * add head to OPTLIST
         */
        NODELIST_NEXT (head) = INFO_OPTLIST (arg_info);
        INFO_OPTLIST (arg_info) = head;

        /*
         * prevent more registration of nodes,
         * because MOSTFREQUENTNODE is found already
         */
        INFO_STATUSSECONDOPERATOR (arg_info) = 2;
    }

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   node *RemoveMostFrequentNode(info *arg_info)
 *
 * description:
 *   All NODELIST_NODE's in OPTLIST contain the MOSTFREQUENTNODE.
 *   This function remove the MOSTFREQUENTNODE from NODELIST_NODE's.
 *   If the MOSTFREQUENTNODE is a argument of SECONDOPERATOR, SECONOPERATOR
 *   is removed and only the other argument remains.
 *   If the MOSTFREQUENTNODE is a argument of MAINOPERATOR, MOSTFREQUENTNODE
 *   is replaced by neutral element of SECONDOPERATOR (if known)!
 *   If there are existing PARENTNODES, they are all duplicated. The new
 *   ones are updated to the former changes. So there exist a duplicated
 *   structure of the original, but without MOSTFREQUENTNODE!
 *
 ****************************************************************************/

static void
RemoveMostFrequentNode (info *arg_info)
{

    nodelist *list, *parent_list, *tmp_list;
    node *top_elem, *current_elem, *first_arg, *second_arg, *new_son, *old_son;
    node *new_parent, *second_argnode;

    DBUG_ENTER ("RemoveMostFrequentNode");

    /*
     * duplicate MOSTFREQUENTNODE
     * all top-nodes in OPTLIST contain the MOSTFREQUENTNODE
     * for all top-nodes:
     *     - copy top-node
     *     - remove MOSTFREQUENTNODE, remove prf/ap node
     *       (only let-node -> id-node  remains)
     *     - copy all parent-nodes and update their arguments with the new copies
     */

    new_son = NULL;

    current_elem = INFO_MOSTFREQUENTNODE (arg_info);
    current_elem = DUPdoDupTree (current_elem);
    INFO_MOSTFREQUENTNODE (arg_info) = current_elem;
    current_elem = NULL;

    list = INFO_OPTLIST (arg_info);

    while (list != NULL) {

        top_elem = NODELIST_NODE (list);

        if ((IsIdenticalNode (INFO_MOSTFREQUENTNODE (arg_info), top_elem))
            && (!(NODE_TYPE (top_elem) == N_exprs) || (EXPRS_NEXT (top_elem) == NULL))) {
            /*
             * constant node - MFN is argument of MAINOPERATOR
             * replace MFN with neutral element of MAINOPERATOR
             */

            new_son = GetNeutralElement (INFO_SECONDOPERATOR (arg_info), arg_info);
            new_son = MakeAssignLetNodeFromCurrentNode (new_son, arg_info, 0);
        } else {

            /*
             * one of the nodes son is MFN
             * remove MFN
             */
            first_arg = EXPRS_EXPR (top_elem);
            second_arg = EXPRS_EXPR (EXPRS_NEXT (top_elem));

            /*
             * is first_arg MOSTFREQUENTNODE?
             */
            if (IsIdenticalNode (first_arg, INFO_MOSTFREQUENTNODE (arg_info))) {

                new_son = DUPdoDupTree (second_arg);

                if (IsAnArray (EXPRS_EXPR (EXPRS_NEXT (top_elem)), arg_info))
                    new_son = MakeAssignLetNodeFromCurrentNode (new_son, arg_info, 1);
                else
                    new_son = MakeAssignLetNodeFromCurrentNode (new_son, arg_info, 0);
            }
            /*
             * is second_arg MOSTFREQUENTNODE?
             */
            else if (IsIdenticalNode (second_arg, INFO_MOSTFREQUENTNODE (arg_info))) {

                new_son = DUPdoDupTree (first_arg);

                if (IsAnArray (EXPRS_EXPR (top_elem), arg_info))
                    new_son = MakeAssignLetNodeFromCurrentNode (new_son, arg_info, 1);
                else
                    new_son = MakeAssignLetNodeFromCurrentNode (new_son, arg_info, 0);
            } else {
                DBUG_ASSERT (0 == 1, "No MOSTFREQUENTNODE found, invalid construction of "
                                     "OPTLIST");
            }

            old_son = top_elem;

            parent_list = DL_NODELIST_PARENTNODES (list);

            tmp_list = parent_list;

            while (tmp_list != NULL) {

                /*
                 * at least one listelement in tmp_list
                 * get both arguments of current_element (NODELIST_NODE of head of
                 * tmp_list) replace old_son with new_son make new assign nodes with both
                 * arguments and SECONDOPERATOR save new_son in correct list new assign
                 * node is new_son again
                 */
                current_elem = NODELIST_NODE (tmp_list);
                second_argnode = NULL;

                /*
                 * find original of in last step updated node
                 * memorize the other(!) argument as second_argnode
                 */
                if (!IsConstant (EXPRS_EXPR (current_elem), arg_info)
                    && !ReachedArgument (EXPRS_EXPR (current_elem))
                    && !ReachedDefinition (EXPRS_EXPR (current_elem))
                    && ((IsSameOperator (INFO_SECONDOPERATOR (arg_info),
                                         LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (
                                           ID_AVIS (EXPRS_EXPR (current_elem)))))))
                        || (IsSameOperator (INFO_MAINOPERATOR (arg_info),
                                            LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (
                                              ID_AVIS (EXPRS_EXPR (current_elem))))))))) {

                    first_arg = PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                      AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (current_elem))))));
                    second_arg = EXPRS_NEXT (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                      AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (current_elem)))))));

                    if (IsIdenticalNode (EXPRS_EXPR (first_arg), EXPRS_EXPR (old_son))) {

                        second_argnode
                          = MakeExprNodes (EXPRS_EXPR (EXPRS_NEXT (current_elem)));

                    } else if (IsIdenticalNode (EXPRS_EXPR (second_arg),
                                                EXPRS_EXPR (old_son))) {

                        second_argnode
                          = MakeExprNodes (EXPRS_EXPR (EXPRS_NEXT (current_elem)));
                    }
                }

                else if (!IsConstant (EXPRS_EXPR (EXPRS_NEXT (current_elem)), arg_info)
                         && !ReachedArgument (EXPRS_EXPR (EXPRS_NEXT (current_elem)))
                         && !ReachedDefinition (EXPRS_EXPR (EXPRS_NEXT (current_elem)))) {

                    first_arg = PRF_ARGS (LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (
                      ID_AVIS (EXPRS_EXPR (EXPRS_NEXT (current_elem)))))));
                    second_arg
                      = EXPRS_NEXT (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (
                        ID_AVIS (EXPRS_EXPR (EXPRS_NEXT (current_elem))))))));

                    if (IsIdenticalNode (EXPRS_EXPR (first_arg), EXPRS_EXPR (old_son))) {

                        second_argnode = MakeExprNodes (EXPRS_EXPR (current_elem));

                    } else if (IsIdenticalNode (EXPRS_EXPR (second_arg),
                                                EXPRS_EXPR (old_son))) {

                        second_argnode = MakeExprNodes (EXPRS_EXPR (current_elem));
                    }
                }

                else {
                    DBUG_ASSERT (FALSE, "Unexpected construction in PARENTNODELIST, no "
                                        "old_son found");
                }

                /*
                 * create new exprs-node from new_son
                 * second_argnode becomes second argument
                 */

                new_parent = MakeExprsNodeFromAssignNode (new_son);

                EXPRS_NEXT (new_parent) = second_argnode;

                /*
                 * save new_son in correct list (OPTLIST)
                 */

                AddNodeToOptimizedNodes (new_son, arg_info);

                /*
                 * create prf/ap-node and then assign-node
                 */

                if (IsAnArray (EXPRS_EXPR (new_parent), arg_info))
                    if (IsAnArray (EXPRS_EXPR (EXPRS_NEXT (new_parent)), arg_info)) {

                        new_parent = MakeOperatorNode (new_parent,
                                                       INFO_SECONDOPERATOR (arg_info), 3);
                        new_parent
                          = MakeAssignLetNodeFromCurrentNode (new_parent, arg_info, 1);

                    } else {

                        new_parent = MakeOperatorNode (new_parent,
                                                       INFO_SECONDOPERATOR (arg_info), 2);
                        new_parent
                          = MakeAssignLetNodeFromCurrentNode (new_parent, arg_info, 1);
                    }
                else if (IsAnArray (EXPRS_EXPR (EXPRS_NEXT (new_parent)), arg_info)) {

                    new_parent
                      = MakeOperatorNode (new_parent, INFO_SECONDOPERATOR (arg_info), 1);
                    new_parent
                      = MakeAssignLetNodeFromCurrentNode (new_parent, arg_info, 1);

                } else {

                    new_parent
                      = MakeOperatorNode (new_parent, INFO_SECONDOPERATOR (arg_info), 0);
                    new_parent
                      = MakeAssignLetNodeFromCurrentNode (new_parent, arg_info, 0);
                }

                /*
                 * prepare for new loop-cycle
                 */
                old_son = current_elem;
                new_son = new_parent;

                tmp_list = NODELIST_NEXT (tmp_list);
            }
        }
        /*
         * no more parent nodes exist
         * only have to save new_son in list for later usage
         */

        NODELIST_NODE (list) = new_son;

        list = NODELIST_NEXT (list);
    }

    DBUG_VOID_RETURN;
}

static bool
IsMainOrSecondOperator (node *id, info *arg_info)
{

    bool result = FALSE;
    node *operator;

    DBUG_ENTER ("IsMainOrSecondOperator");

    operator= GetUsedOperator (id, arg_info);

    if ((IsSameOperator (operator, INFO_SECONDOPERATOR (arg_info)))
        || (IsSameOperator (operator, INFO_MAINOPERATOR (arg_info))))
        result = TRUE;

    DBUG_RETURN (result);
}

/*****************************************************************************
 *
 * function:
 *   OptTravElems(node *arg_node, info *arg_info)
 *
 * description:
 *   The arg_node is an exprs-node
 *   The function terminates, when
 *     (a) the expr-node contain an constant node
 *     (b) in the definition of the argument of the expr-node
 *         (i) another primitive is used
 *         (ii) no primitive node is used
 *     (c) a variable is reached, which is defined outside the actual N_block
 *     (d) no associative/commutative operator is reached
 *     (e) the third operator is reached
 *
 *   After the second operator is reached all nodes which are travesed are
 *   memorized, so if the MOSTFREQUENTNODE is found the path to reach this
 *   node from the first occurence of the second operator is reconstructable!
 *   This nodes are memorized while the traversal in TMPLIST and after the
 *   MOSTFREQUENTNODE is found they are stored in OPTLIST.
 *   If no MOSTFREQUENTNODE could be found in a subtree (root: the first
 *   occurence of the second operator) the arguments of the root are stored
 *   in NONOPTLIST.
 *
 ****************************************************************************/

static info *
OptTravElems (node *arg_node, info *arg_info)
{

    nodelist *list;

    DBUG_ENTER ("OptTravElems");

    if (IsConstant (EXPRS_EXPR (arg_node), arg_info)
        || ReachedArgument (EXPRS_EXPR (arg_node))
        || ReachedDefinition (EXPRS_EXPR (arg_node))
        || (!IsSupportedOperator (EXPRS_EXPR (arg_node), arg_info))
        || IsThirdOperatorReached (EXPRS_EXPR (arg_node), arg_info)
        || (!IsMainOrSecondOperator (EXPRS_EXPR (arg_node), arg_info))) {

        /*
         * termination-condition reached
         * check if MOSTFREQUENTNODE is not found till now but SECONDOPERATOR reached
         */
        if (INFO_STATUSSECONDOPERATOR (arg_info) == 1) {
            CheckNode (EXPRS_EXPR (arg_node), arg_info);
        }

        /*
         * if node is a single node connected with MAINOPERATOR
         */
        if (INFO_STATUSSECONDOPERATOR (arg_info) == 0) {

            nodelist *newlist;
            node *node_tmp;

            node_tmp = EXPRS_EXPR (arg_node);

            newlist = TBmakeNodelistNode (node_tmp, NULL);

            /*
             * check if arg_node is identical to MOSTFREQUENTNODE
             */
            if (IsIdenticalNode (EXPRS_EXPR (arg_node),
                                 INFO_MOSTFREQUENTNODE (arg_info))) {

                if (ExistKnownNeutralElement (INFO_SECONDOPERATOR (arg_info))) {

                    /*
                     * node is MFN and neutral element is known
                     */
                    NODELIST_NEXT (newlist) = INFO_OPTLIST (arg_info);
                    INFO_OPTLIST (arg_info) = newlist;
                }
            } else {

                /*
                 * node is not MFN or no known neutral Element: add to NONOPTLIST
                 */
                NODELIST_NEXT (newlist) = INFO_NONOPTLIST (arg_info);
                INFO_NONOPTLIST (arg_info) = newlist;
            }
        }
    } else if ((INFO_STATUSSECONDOPERATOR (arg_info) == 0)
               && (IsSameOperator (INFO_SECONDOPERATOR (arg_info),
                                   GetUsedOperator (EXPRS_EXPR (arg_node), arg_info)))) {

        /*
         * second operator reached for first time
         *    1.) save second operator
         *    2.) register new(!) arguments in TMPLIST
         *    3.) traverse to new arguments
         *    4.) Add TMPLIST to NONOPTLIST if neccessary
         */

        INFO_STATUSSECONDOPERATOR (arg_info) = 1;

        /*
         * save nodes which will be traversed in TMPLIST
         * (neccessary to memorize the PARENTNODES of possible MOSTFREQUENTNODES)
         */
        list = TBmakeNodelistNode (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                                     AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node)))))),
                                   NULL);
        INFO_TMPLIST (arg_info) = list;

        ASSIGN_ISUNUSED (AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node)))) = FALSE;

        arg_info = OptTravElems (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                                   AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node)))))),
                                 arg_info);

        arg_info = OptTravElems (EXPRS_NEXT (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                                   AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node))))))),
                                 arg_info);

        if (INFO_STATUSSECONDOPERATOR (arg_info) == 1) {

            node *newnode, *node1, *node2;
            nodelist *newlist;
            /*
             * no MOSTFREQUENTNODE found
             *
             * duplicate both exprs-nodes in TMPLIST
             * add result to NONOPTLIST
             * clear TMPLIST
             */

            node1 = NODELIST_NODE (INFO_TMPLIST (arg_info));
            node2 = EXPRS_EXPR (EXPRS_NEXT (node1));
            node1 = EXPRS_EXPR (node1);

            node1 = MakeExprNodes (node1);
            node2 = MakeExprNodes (node2);

            newnode = node1;
            EXPRS_NEXT (newnode) = node2;

            if (IsAnArray (EXPRS_EXPR (node1), arg_info))
                if (IsAnArray (EXPRS_EXPR (node2), arg_info)) {

                    newnode
                      = MakeOperatorNode (newnode, INFO_SECONDOPERATOR (arg_info), 3);
                    newnode = MakeAssignLetNodeFromCurrentNode (newnode, arg_info, 1);

                } else {

                    newnode
                      = MakeOperatorNode (newnode, INFO_SECONDOPERATOR (arg_info), 2);
                    newnode = MakeAssignLetNodeFromCurrentNode (newnode, arg_info, 1);
                }
            else if (IsAnArray (EXPRS_EXPR (node2), arg_info)) {

                newnode = MakeOperatorNode (newnode, INFO_SECONDOPERATOR (arg_info), 1);
                newnode = MakeAssignLetNodeFromCurrentNode (newnode, arg_info, 1);

            } else {

                newnode = MakeOperatorNode (newnode, INFO_SECONDOPERATOR (arg_info), 0);
                newnode = MakeAssignLetNodeFromCurrentNode (newnode, arg_info, 0);
            }

            newlist = TBmakeNodelistNode (newnode, NULL);

            NODELIST_NEXT (newlist) = INFO_NONOPTLIST (arg_info);
            INFO_NONOPTLIST (arg_info) = newlist;
            INFO_TMPLIST (arg_info) = NULL;
        }

        INFO_STATUSSECONDOPERATOR (arg_info) = 0;

    } else if ((INFO_STATUSSECONDOPERATOR (arg_info) == 1)
               && IsSameOperator (INFO_SECONDOPERATOR (arg_info),
                                  GetUsedOperator (EXPRS_EXPR (arg_node), arg_info))) {

        /*
         * second operator reached again -
         *   1.) register new(!) arguments in TMPLIST
         *   2.) traverse to new arguments,
         *   3.) remove traversed arguments from TMPLIST
         */

        /*
         * set flag to prevent traversed and used node
         * to be optimized again later as start-node of a optimization-cycle
         */
        ASSIGN_ISUNUSED (AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node)))) = FALSE;

        /*
         * register new(!) arguments in TMPLIST
         */
        list = TBmakeNodelistNode (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                                     AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node)))))),
                                   INFO_TMPLIST (arg_info));
        INFO_TMPLIST (arg_info) = list;

        /*
         * traverse in new arguments
         */
        arg_info = OptTravElems (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                                   AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node)))))),
                                 arg_info);
        arg_info = OptTravElems (EXPRS_NEXT (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                                   AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node))))))),
                                 arg_info);

        /*
         * remove traversed arguments if neccessary
         * (if no MOSTFREQUENTNODE is found till now)
         */
        if (INFO_STATUSSECONDOPERATOR (arg_info) == 1) {
            list = INFO_TMPLIST (arg_info);
            INFO_TMPLIST (arg_info) = NODELIST_NEXT (INFO_TMPLIST (arg_info));
            NODELIST_NODE (list) = NULL;
            NODELIST_NEXT (list) = NULL;
            MEMfree (list);
        }
    } else {

        /*
         * main operator - traverse
         * or MOSTFREQUENTNODE already found - traverse till end
         */

        ASSIGN_ISUNUSED (AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node)))) = FALSE;

        arg_info = OptTravElems (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                                   AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node)))))),
                                 arg_info);
        arg_info = OptTravElems (EXPRS_NEXT (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                                   AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node))))))),
                                 arg_info);
    }

    DBUG_RETURN (arg_info);
}

/*****************************************************************************
 *
 * function:
 *   node *CreateOptLists(node *arg_node, info *arg_info)
 *
 * description:
 *   Starting function for investigate the OPTLIST and NONOPTLIST
 *   in relation to MOSTFREQUENTNODE.
 *
 *****************************************************************************/

static void
CreateOptLists (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CreateOptLists");

    INFO_OPTLIST (arg_info) = NULL;
    INFO_NONOPTLIST (arg_info) = NULL;

    arg_info = OptTravElems (PRF_ARGS (arg_node), arg_info);
    arg_info = OptTravElems (EXPRS_NEXT (PRF_ARGS (arg_node)), arg_info);

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   nodelist* FindNodeInList(nodelist* list, node *arg_node, node *operator)
 *
 * description:
 *   Returns the position in nodelist where NODELIST_NODE is equal to arg_node
 *   and operator is equal to OPERATOR.
 *   If no such combination is in list, so NULL is returned.
 *
 *****************************************************************************/

static nodelist *
FindNodeInList (nodelist *list, node *arg_node, node *operator)
{

    nodelist *result;
    node *current_node;
    node *current_operator;
    bool result_found, equal_op;

    DBUG_ENTER ("FindNodeInList");

    equal_op = FALSE;

    result = NULL;
    result_found = FALSE;

    if (list != NULL) {

        while ((list != NULL) && (!result_found)) {

            current_node = NODELIST_NODE (list);
            current_operator = DL_NODELIST_OPERATOR (list);

            result_found = FALSE;

            equal_op = FALSE;

            /*
             * are operator-type and argument-type equal?
             */
            if ((NODE_TYPE (operator) == NODE_TYPE (current_operator))
                && (NODE_TYPE (arg_node) == NODE_TYPE (current_node))) {

                /*
                 * are both operation the same?
                 */

                equal_op = IsSameOperator (operator, current_operator );

                if (equal_op) {

                    /*
                     * are both arguments equal?
                     */
                    result_found = IsIdenticalNode (arg_node, current_node);
                }
            }
            if (result_found)
                result = list;

            list = NODELIST_NEXT (list);
        }
    }

    DBUG_RETURN (result);
}

/*****************************************************************************
 *
 * function:
 *   node *FindAndSetMostFrequentNode(info *arg_info)
 *
 * description:
 *   Add memorized nodes connected with MAINOOERATOR in COUNTLIST to nodes
 *   connected with a operator for which the neutral element is known.
 *   Than the most frequent node is investigated and saved in
 *   MOSTFREQUENTNODE
 *
 *****************************************************************************/

static void
FindAndSetMostFrequentNode (info *arg_info)
{

    int maxOccurence;
    nodelist *list;

    DBUG_ENTER ("FindAndSetMostFrequentNode");

    RegisterMultipleUsableNodes (arg_info);

    maxOccurence = 0;
    list = INFO_COUNTLIST (arg_info);

    while (list != NULL) {

        if ((NODELIST_INT (list) > maxOccurence)
            && (!IsSameOperator (DL_NODELIST_OPERATOR (list),
                                 INFO_MAINOPERATOR (arg_info)))) {

            maxOccurence = NODELIST_INT (list);

            INFO_SECONDOPERATOR (arg_info) = DL_NODELIST_OPERATOR (list);
            INFO_MOSTFREQUENTNODE (arg_info) = NODELIST_NODE (list);
            INFO_OCCURENCEOFMOSTFREQUENTNODE (arg_info) = maxOccurence;
        }
        list = NODELIST_NEXT (list);
    }
    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   node *RegisterNode(node *arg_node, info *arg_info)
 *
 * description:
 *   If arg_node with the correct operator exists in COUNTLIST,
 *   the counter is increased and the flag to prevent multiple counting
 *   is set. Else a new listelement is added to COUNTLIST with counter == 1.
 *
 *****************************************************************************/

static void
RegisterNode (node *arg_node, info *arg_info)
{

    node *lastOperator;
    nodelist *node_in_list;
    nodelist *newnodelistnode;

    DBUG_ENTER ("RegisterNode");

    if (INFO_STATUSSECONDOPERATOR (arg_info) == 1)
        lastOperator = INFO_SECONDOPERATOR (arg_info);
    else
        lastOperator = INFO_MAINOPERATOR (arg_info);

    node_in_list = FindNodeInList (INFO_COUNTLIST (arg_info), arg_node, lastOperator);

    if (node_in_list == NULL) {

        /*
         * node with operator is not in COUNTLIST
         * create new nodelistnode, becomes (after all) first element of COUNTLIST
         * insert arg_node, operator and initilize counter
         */

        newnodelistnode = TBmakeNodelistNode (arg_node, NULL);

        NODELIST_NODE (newnodelistnode) = arg_node;

        /*
         * DL_NODELIST_OPERATOR(newnodelistnode) = lastOperator;
         */
        NODELIST_ATTRIB2 (newnodelistnode) = lastOperator;

        NODELIST_STATUS (newnodelistnode) = 1;

        NODELIST_INT (newnodelistnode) = 1;

        NODELIST_NEXT (newnodelistnode) = INFO_COUNTLIST (arg_info);
        INFO_COUNTLIST (arg_info) = newnodelistnode;

    } else {

        /*
         * node with operator is in COUNTLIST
         * only increase counter
         */
        if (NODELIST_STATUS (node_in_list) == 0)
            NODELIST_INT (node_in_list) = NODELIST_INT (node_in_list) + 1;
        NODELIST_STATUS (node_in_list) = 1;
    }
    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   SearchTravElems(node *arg_node, info *arg_info)
 *
 * description:
 *   The arg_node is an exprs-node
 *   The function terminates, when
 *     (a) the expr-node contain an constant node
 *     (b) in the definition of the argument of the expr-node
 *         (i) another primitive is used
 *         (ii) no primitive node is used
 *     (c) a variable is reached, which is defined outside the actual N_block
 *     (d) no associative/commutative operator is reached
 *     (e) the third operator is reached
 *
 *   Every termination node is registered to COUNTLIST. So the amount of
 *   different argument/operation - combination is counted.
 *
 ****************************************************************************/

static void
SearchTravElems (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("SearchTravElems");

    if (IsConstant (EXPRS_EXPR (arg_node), arg_info)) {

        /*
         * constant argument reached
         */
        RegisterNode (EXPRS_EXPR (arg_node), arg_info);

    } else if (ReachedArgument (EXPRS_EXPR (arg_node))
               || ReachedDefinition (EXPRS_EXPR (arg_node))
               || (!IsSupportedOperator (EXPRS_EXPR (arg_node), arg_info))
               || IsThirdOperatorReached (EXPRS_EXPR (arg_node), arg_info)
               || ((IsSupportedOperator (EXPRS_EXPR (arg_node), arg_info))
                   && (!IsValidSecondOperator (EXPRS_EXPR (arg_node), arg_info))
                   && (!IsSameOperator (INFO_MAINOPERATOR (arg_info),
                                        GetUsedOperator (EXPRS_EXPR (arg_node),
                                                         arg_info))))) {

        /*
         * other termination-condition reached
         */
        RegisterNode (EXPRS_EXPR (arg_node), arg_info);

    } else if ((IsSupportedOperator (EXPRS_EXPR (arg_node), arg_info))
               && (INFO_STATUSSECONDOPERATOR (arg_info) == 0)
               && IsValidSecondOperator (EXPRS_EXPR (arg_node), arg_info)) {

        /*
         * second operator reached - save second operator and traverse
         */

        ResetFlags (arg_info);

        INFO_SECONDOPERATOR (arg_info)
          = GetUsedOperator (EXPRS_EXPR (arg_node), arg_info);
        INFO_STATUSSECONDOPERATOR (arg_info) = 1;

        SearchTravElems (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                           AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node)))))),
                         arg_info);
        SearchTravElems (EXPRS_NEXT (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                           AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node))))))),
                         arg_info);

        INFO_SECONDOPERATOR (arg_info) = NULL;
        INFO_STATUSSECONDOPERATOR (arg_info) = 0;

        ResetFlags (arg_info);
    } else {

        /*
         * still main operator or second operator: traverse
         */
        SearchTravElems (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                           AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node)))))),
                         arg_info);
        SearchTravElems (EXPRS_NEXT (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                           AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node))))))),
                         arg_info);
    }

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   node *SearchMostFrequentNode(node *arg_node, info *arg_info)
 *
 * description:
 *   Starting function for investigate the MOSTFREQUENTNODE.
 *   Both exprs-nodes are traversed and every termination-node is
 *   memorized (with used operator) and his occurences counted.
 *   After the this traversal the created COUNTLIST is evaluated and the
 *   MOSTFREQUENTNODE is set.
 *
 *****************************************************************************/

static void
SearchMostFrequentNode (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("SearchMostFrequentNode");

    INFO_COUNTLIST (arg_info) = NULL;

    /*
     * Traverse through the definitions of arg_node and collect nodes
     */

    INFO_MAINOPERATOR (arg_info) = arg_node;

    SearchTravElems (PRF_ARGS (arg_node), arg_info);
    SearchTravElems (EXPRS_NEXT (PRF_ARGS (arg_node)), arg_info);

    /*
     * Now, all untraversable nodes are added to COUNTLIST.
     * Next step is to find the most frequent node in list
     * and modify SECONDOPERATOR in arg_info with the operation of the 'winner-node'
     */

    FindAndSetMostFrequentNode (arg_info);

    DeleteNodelist (INFO_COUNTLIST (arg_info));
    INFO_COUNTLIST (arg_info) = NULL;

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   node *DistributiveLaw(node *arg_node)
 *
 * description:
 *   is called from optimize.c
 *
 *****************************************************************************/

node *
DLdoDistributiveLaw (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("DistributiveLaw");

    if (arg_node != NULL) {
        DBUG_PRINT ("OPT",
                    ("starting distributive law in function %s", FUNDEF_NAME (arg_node)));

        arg_info = MakeInfo ();

        TRAVpush (TR_dl);
        arg_node = TRAVdo (arg_node, arg_info);
        TRAVpop ();

        INFO_NTYPE (arg_info) = NULL;

        arg_info = FreeInfo (arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   DLblock(node *arg_node, info *arg_info)
 *
 * description:
 *   store block-node for access to vardec-nodes
 *   reset nodelists
 *   traverse through block-nodes
 *
 ****************************************************************************/

node *
DLblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DLblock");

    if (BLOCK_INSTR (arg_node) != NULL) {
        /*
         * store pointer on actual N_block-node for append of new N_vardec nodes
         */
        if (BLOCK_VARDEC (arg_node) != NULL) {
            INFO_BLOCKNODE (arg_info) = arg_node;
        }

        INFO_OPTLIST (arg_info) = NULL;
        INFO_NONOPTLIST (arg_info) = NULL;

        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   DLassign(node *arg_node, info *arg_info)
 *
 * description:
 *   Set flag ASSIGN_ISUNUSED to mark unused nodes in optimization-process
 *   Traverse through assign-nodes
 *   If previous assign-node was optimized, increase optimization counter
 *   and include new nodes
 *   Free all used nodelists
 *   If current assign node was unused during this optimization-cycle
 *   traverse in let node
 *
 ****************************************************************************/

node *
DLassign (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("DLassign");

    ASSIGN_ISUNUSED (arg_node) = TRUE;

    if (ASSIGN_NEXT (arg_node) != NULL) {

        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);

        if (INFO_OPTIMIZEDNODES (arg_info) != NULL) {

            node *assign_next;
            nodelist *tmplist;

            /*
             * increase optimization counter
             */

            /*dl_expr = dl_expr + 1;*/

            /*
             * insert OPTIMIZEDNODES in tree in front of 'source'-node
             */

            assign_next = ASSIGN_NEXT (arg_node);

            while (INFO_OPTIMIZEDNODES (arg_info) != NULL) {

                ASSIGN_NEXT (NODELIST_NODE (INFO_OPTIMIZEDNODES (arg_info)))
                  = assign_next;
                assign_next = NODELIST_NODE (INFO_OPTIMIZEDNODES (arg_info));

                tmplist = INFO_OPTIMIZEDNODES (arg_info);

                INFO_OPTIMIZEDNODES (arg_info) = NODELIST_NEXT (tmplist);

                NODELIST_NEXT (tmplist) = NULL;
                NODELIST_NODE (tmplist) = NULL;
                tmplist = DeleteNodelist (tmplist);
            }

            INFO_OPTIMIZEDNODES (arg_info) = NULL;

            ASSIGN_NEXT (arg_node) = assign_next;
        }
        /*
         * traverse in N_let-node
         */
    }
    if ((ASSIGN_INSTR (arg_node) != NULL) && (ASSIGN_ISUNUSED (arg_node) == TRUE)) {

        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   DLlet(node *arg_node, info *arg_info)
 *
 * description:
 *   store current let-node to include last created new primitive-node
 *   traverse N_let-nodes
 *
 ****************************************************************************/

node *
DLlet (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("DLlet");
    if (LET_EXPR (arg_node) != NULL) {

        INFO_LETNODE (arg_info) = arg_node;
        INFO_IEEEFLAG (arg_info) = 0;

        if ((LET_IDS (arg_node) != NULL) && (IDS_AVIS (LET_IDS (arg_node)) != NULL)) {
            INFO_NTYPE (arg_info) = AVIS_TYPE (IDS_AVIS (LET_IDS (arg_node)));
        }
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

        /*
         * if optimization was neccessary:
         * both remaining nodes get arguments of start-assign-node
         */
        if ((INFO_OCCURENCEOFMOSTFREQUENTNODE (arg_info) > 1)
            && ((!global.enforce_ieee) || (INFO_IEEEFLAG (arg_info) == 0))) {
            IntegrateResults (arg_info);
        }
        INFO_OCCURENCEOFMOSTFREQUENTNODE (arg_info) = 0;
    }
    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *DLPrfOrAp(node *arg_node, info *arg_info)
 *
 * description:
 *   Starting point of one optimization-cycle.
 *   First the operator the operator is checked, if the operator is supported,
 *   after that the MOSTFREQUENTNODE is investigated and then the
 *   optimization starts with reconstructing the syntax-tree.
 *
 ****************************************************************************/

node *
DLap (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("DLap");

    if (((NODE_TYPE (arg_node) == N_ap) && (AP_ARGS (arg_node) != NULL)
         && (NODE_TYPE (AP_ARGS (arg_node)) == N_exprs))) {

        /*
         * memorize MAINOPERATOR
         */
        INFO_MAINOPERATOR (arg_info) = arg_node;

        /*
         * is MAINOPERATOR supported?
         */
        if (CheckOperator (INFO_MAINOPERATOR (arg_info), arg_info)) {

            INFO_OPTLIST (arg_info) = NULL;
            INFO_NONOPTLIST (arg_info) = NULL;

            /*
             * investigate the most frequent node
             */
            SearchMostFrequentNode (arg_node, arg_info);

            /*
             * Exist an optimization case?
             */
            if ((INFO_OCCURENCEOFMOSTFREQUENTNODE (arg_info) > 1)
                && ((!global.enforce_ieee) || (INFO_IEEEFLAG (arg_info) == 0))) {

                /*
                 * increase optimization counter
                 */

                global.optcounters.dl_expr = global.optcounters.dl_expr
                                             + INFO_OCCURENCEOFMOSTFREQUENTNODE (arg_info)
                                             - 1;

                /*
                 * Create OPTLIST and NONOPTLIST in regard to MOSTFREQUENTNODE
                 * nodes in OPTLIST contain (somewhere) in their definition the
                 * MOSTFREQENTNODE all other nodes are collected in NONOPTLIST
                 */

                CreateOptLists (arg_node, arg_info);

                /*
                 * OPTIMIZATION
                 *
                 * duplicate MFN and remove then all MFN from OPTLIST
                 */
                RemoveMostFrequentNode (arg_info);

                /*
                 * remove all original nodes from NONOPTLIST by creating
                 * new assign nodes with original nodes as arguments
                 */
                CreateAssignNodes (arg_info);

                /*
                 * connect node in OPTLIST / NONOPTLIST till only one assign-node remain
                 */
                INFO_NONOPTLIST (arg_info)
                  = CommitAssignNodes (INFO_NONOPTLIST (arg_info), arg_info);
                INFO_OPTLIST (arg_info)
                  = CommitAssignNodes (INFO_OPTLIST (arg_info), arg_info);

                /*
                 * connect MFN with remaining node in OPTLIST
                 */
                IncludeMostFrequentNode (arg_info);
            }
        }

        INFO_MAINOPERATOR (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *DLprf(node *arg_node, info *arg_info)
 *
 * description:
 *   Starting point of one optimization-cycle.
 *   First the operator the operator is checked, if the operator is supported,
 *   after that the MOSTFREQUENTNODE is investigated and then the
 *   optimization starts with reconstructing the syntax-tree.
 *
 ****************************************************************************/

node *
DLprf (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("DLprf");

    if (((NODE_TYPE (arg_node) == N_prf)
         && (NODE_TYPE (PRF_ARGS (arg_node)) == N_exprs))) {

        /*
         * memorize MAINOPERATOR
         */
        INFO_MAINOPERATOR (arg_info) = arg_node;

        /*
         * is MAINOPERATOR supported?
         */
        if (CheckOperator (INFO_MAINOPERATOR (arg_info), arg_info)) {

            INFO_OPTLIST (arg_info) = NULL;
            INFO_NONOPTLIST (arg_info) = NULL;

            /*
             * investigate the most frequent node
             */
            SearchMostFrequentNode (arg_node, arg_info);

            /*
             * Exist an optimization case?
             */
            if ((INFO_OCCURENCEOFMOSTFREQUENTNODE (arg_info) > 1)
                && ((!global.enforce_ieee) || (INFO_IEEEFLAG (arg_info) == 0))) {

                /*
                 * increase optimization counter
                 */

                global.optcounters.dl_expr = global.optcounters.dl_expr
                                             + INFO_OCCURENCEOFMOSTFREQUENTNODE (arg_info)
                                             - 1;

                /*
                 * Create OPTLIST and NONOPTLIST in regard to MOSTFREQUENTNODE
                 * nodes in OPTLIST contain (somewhere) in their definition the
                 * MOSTFREQENTNODE all other nodes are collected in NONOPTLIST
                 */

                CreateOptLists (arg_node, arg_info);

                /*
                 * OPTIMIZATION
                 *
                 * duplicate MFN and remove then all MFN from OPTLIST
                 */
                RemoveMostFrequentNode (arg_info);

                /*
                 * remove all original nodes from NONOPTLIST by creating
                 * new assign nodes with original nodes as arguments
                 */
                CreateAssignNodes (arg_info);

                /*
                 * connect node in OPTLIST / NONOPTLIST till only one assign-node remain
                 */
                INFO_NONOPTLIST (arg_info)
                  = CommitAssignNodes (INFO_NONOPTLIST (arg_info), arg_info);
                INFO_OPTLIST (arg_info)
                  = CommitAssignNodes (INFO_OPTLIST (arg_info), arg_info);

                /*
                 * connect MFN with remaining node in OPTLIST
                 */
                IncludeMostFrequentNode (arg_info);
            }
        }

        INFO_MAINOPERATOR (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}
