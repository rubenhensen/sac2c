/* *
 * $Log$
 * Revision 1.7  2003/04/10 21:26:06  mwe
 * bug in RemoveMostFrequentNode removed
 *
 * Revision 1.6  2003/02/24 17:40:36  mwe
 * declare local used functions as static
 * bug in GetNeutralElement removed
 *
 * Revision 1.5  2003/02/15 16:48:00  mwe
 * bugs removed
 * changed assignment for INFO_DL_TYPES (because former assignment was not up to date)
 *
 * Revision 1.4  2003/02/13 20:15:43  mwe
 * Warnings removed
 *
 * Revision 1.3  2003/02/10 18:01:30  mwe
 * removed bugs
 * enforce_ieee support added
 *
 * Revision 1.2  2003/02/09 22:31:24  mwe
 * removed bugs
 *
 * Revision 1.1  2003/02/08 16:08:02  mwe
 * Initial revision
 *
 *
 */

/**************************
 *
 *  Zugriffmakros:
 *
 * INFO_DL_TYPE          :  Quelle für weitere type-nodes
 * INFO_DL_BLOCKNODE     :  Zugriff auf Blocknode (AVIS)
 * INFO_DL_OPTLIST       :  sammeln der Knoten mit Opt-Fälle
 * INFO_DL_NONOPTLIST    :  sammeln von Knoten ohne Opt-Fälle
 * INFO_DL_CURRENTASSIGN :  Startknoten der aktuellen Optimierung
 * INFO_DL_LETNODE       :  let-node zum Einfügen der optim. Knoten
 * INFO_DL_MAINOPERATOR  :  äussere Operation
 *
 ********************/

#define INFO_DL_TYPE(n) ((types *)(n->dfmask[0]))
#define INFO_DL_BLOCKNODE(n) (n->node[0])
#define INFO_DL_OPTLIST(n) ((nodelist *)(n->info2))
#define INFO_DL_NONOPTLIST(n) ((nodelist *)(n->info3))
#define INFO_DL_OPTIMIZEDNODES(n) ((nodelist *)(n->dfmask[1]))
#define INFO_DL_LETNODE(n) (n->node[1])
#define INFO_DL_MOSTFREQUENTNODE(n) (n->node[3])
#define INFO_DL_OCCURENCEOFMOSTFREQUENTNODE(n) (n->refcnt)
#define INFO_DL_MAINOPERATOR(n) (n->node[4])
#define INFO_DL_STATUSSECONDOPERATOR(n) (n->flag)
#define INFO_DL_SECONDOPERATOR(n) (n->node[5])
#define INFO_DL_COUNTLIST(n) ((nodelist *)(n->info2))
#define INFO_DL_TMPLIST(n) ((nodelist *)(n->dfmask[2]))
#define INFO_DL_IEEEFLAG(n) (n->varno)

#define DL_NODELIST_OPERATOR(n) ((node *)NODELIST_ATTRIB2 (n))
#define DL_NODELIST_PARENTNODES(n) ((nodelist *)NODELIST_ATTRIB2 (n))
#define DL_NODELIST_FLAGS(n) (NODELIST_STATUS (n))
#define DL_EXPRS_FLAG(n) (n->flag)

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
 *     - F_max
 *     - F_min
 *     - F_and
 *     - F_or
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
 *  -NODELIST_FLAGS     NODELIST_STATUS
 *                        is set in COUNTLIST when one MOSTFREQUENTNODE
 *                        was found in the current subtree
 *****************************************************************************/

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

#include "DistributiveLaw.h"

/*****************************************************************************
 *
 * function:
 *   node* GetNeutralElement(node *op, node *arg_info)
 *
 * description:
 *   returns new exprs-node, with neutral element of the primitive
 *   operation 'op'. The neutral element is of correct basetype.
 *
 ****************************************************************************/

static node *
GetNeutralElement (node *op, node *arg_info)
{

    node *neutral_elem;

    DBUG_ENTER ("GetNeutralElement");

    neutral_elem = NULL;

    /* is 'op' a supported node-type? */
    if (NODE_TYPE (op) == N_prf) {

        /* is 'op' a supported operator? */
        if (PRF_PRF (op) == F_add_SxS) {

            /* choose correct basetype of the neutral element */
            if (TYPES_BASETYPE (INFO_DL_TYPE (arg_info)) == T_int) {

                neutral_elem = MakeNum (0);
            }

            if (TYPES_BASETYPE (INFO_DL_TYPE (arg_info)) == T_float) {

                neutral_elem = MakeFloat (0.0f);
            }
            if (TYPES_BASETYPE (INFO_DL_TYPE (arg_info)) == T_double) {

                neutral_elem = MakeDouble (0.0);
            }

        }

        /* is 'op' a supported operator? */
        else if (PRF_PRF (op) == F_mul_SxS) {

            /* choose correct basetype of the neutral element */
            if (TYPES_BASETYPE (INFO_DL_TYPE (arg_info)) == T_int) {

                neutral_elem = MakeNum (1);
            }

            if (TYPES_BASETYPE (INFO_DL_TYPE (arg_info)) == T_float) {

                neutral_elem = MakeFloat (1.0f);
            }
            if (TYPES_BASETYPE (INFO_DL_TYPE (arg_info)) == T_double) {

                neutral_elem = MakeDouble (1.0);
            }

        }

        /* is 'op' a supported operator? */
        else if (PRF_PRF (op) == F_and) {

            neutral_elem = MakeBool (TRUE);

        } else {

            DBUG_ASSERT (FALSE,
                         "Not supported primitive operation! No neutral element known!");
        }

    } else if (NODE_TYPE (op) == N_ap) {

        DBUG_ASSERT (FALSE, "No N_ap nodes supported! No neutral element known!");

    } else {

        DBUG_ASSERT (FALSE, "Unexpected node! Only N_prf/N_ap nodes accepted!");
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

    DBUG_ENTER ("");

    ret = FALSE;

    if (NODE_TYPE (op) == N_prf) {

        if (PRF_PRF (op) == F_add_SxS) {

            ret = TRUE;

        } else if (PRF_PRF (op) == F_mul_SxS) {

            ret = TRUE;

        } else if (PRF_PRF (op) == F_and) {

            ret = TRUE;
        }

    } else if (NODE_TYPE (op) == N_ap) {

        ret = FALSE;

    } else {

        DBUG_ASSERT (FALSE, "Unexpected node! Only N_prf/N_ap nodes accepted!");
    }

    DBUG_RETURN (ret);
}

/*****************************************************************************
 *
 * function:
 *   bool IsConstant(node *arg_node, node* arg_info)
 *
 * description:
 *   returns TRUE, if arg_node is a constant node, otherwise FALSE.
 *   If the node contains a double or float value, the IEEEFLAG is set, so
 *   this optimization don't continue later, if the enforceIEEE-flag is set!
 *
 ****************************************************************************/

static bool
IsConstant (node *arg_node, node *arg_info)
{
    bool is_constant;

    DBUG_ENTER ("IsConstant");

    is_constant = FALSE;

    switch (NODE_TYPE (arg_node)) {
    case N_float:
    case N_double:
        INFO_DL_IEEEFLAG (arg_info) = 1;
    case N_bool:
    case N_num:
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
 *   bool CheckOperator(node *arg_node, node* arg_info)
 *
 * description:
 *   Returns TRUE if 'operator' is supported. This means, 'operator' has to
 *   be associative, commutative.
 *
 * notes:
 *   Till now the function differentiate between prf and ap nodes.
 *   Primitive functions are given. If later user-defined-functions are
 *   also supported in this optimization, they can be added here or the
 *   method call can be placed here!
 *
 ****************************************************************************/

static bool
CheckOperator (node *operator, node *arg_info )
{

    bool support;

    DBUG_ENTER ("CheckOperator");

    support = FALSE;

    if (NODE_TYPE (operator) == N_prf) {

        switch (PRF_PRF (operator)) {
        case F_add_SxS:
        case F_mul_SxS:
        case F_and:
        case F_or:
        case F_max:
        case F_min:
            support = TRUE;
            break;
        default:
            support = FALSE;
        }
    } else if (NODE_TYPE (operator) == N_ap) {

    } else {
        DBUG_ASSERT ((1 == 0), "Unexpected operator detected");
    }

    DBUG_RETURN (support);
}

static node *
GetUsedOperator (node *arg_node, node *arg_info)
{

    node *operator;

    DBUG_ENTER ("GetUsedOperator");

    operator= LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (arg_node))));

    DBUG_RETURN (operator);
}

/*****************************************************************************
 *
 * function:
 *   bool IsSupportedOperator(node *arg_node, node* arg_info)
 *
 * description:
 *   returns TRUE, if the used operator in the definition of arg_node
 *   fulfil the conditions of 'CheckOperator'.
 *
 ****************************************************************************/

static bool
IsSupportedOperator (node *arg_node, node *arg_info)
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
            priority = 6;
            break;
        case F_mul_SxS:
            priority = 7;
            break;
        case F_min:
            priority = 5;
            break;
        case F_max:
            priority = 5;
            break;
        case F_and:
            priority = 5;
            break;
        case F_or:
            priority = 4;
            break;
        default:
            DBUG_ASSERT ((1 == 0), "Unexpected primitive operation");
            priority = 0;
        }

    } else if (NODE_TYPE (operator) == N_ap) {

        priority = 0;

    } else {

        DBUG_ASSERT ((1 == 0), "Wrong node! N_prf or N_ap expected!");
        priority = 0;
    }

    DBUG_RETURN (priority);
}

/*****************************************************************************
 *
 * function:
 *   bool IsIdenticalNode(node *node1, node* node2)
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
 *   bool IsValidSecondOperator(node *arg_node, node* arg_info)
 *
 * description:
 *   returns TRUE, if the the used operator in the definition of arg_node
 *   is supported and his priority is higher than the priority of
 *   MAINOPERATOR.
 *
 ****************************************************************************/

static bool
IsValidSecondOperator (node *arg_node, node *arg_info)
{

    bool valid;
    int priority_firstop;
    int priority_secondop;
    node *secondop;
    node *firstop;

    DBUG_ENTER ("IsValidSecondOperator");

    valid = FALSE;

    if (IsSupportedOperator (arg_node, arg_info)) {

        firstop = INFO_DL_MAINOPERATOR (arg_info);
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
 *   bool IsSameOperator(node *firstop, node* secondop)
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

            if (PRF_PRF (firstop) == PRF_PRF (secondop))
                is_same = TRUE;

        } else if (NODE_TYPE (firstop) == N_ap) {

            if (AP_FUNDEF (firstop) == AP_FUNDEF (secondop))
                is_same = TRUE;

        } else {

            DBUG_ASSERT ((1 == 0), "Wrong node! N_prf or N_ap expected!");
        }
    }

    DBUG_RETURN (is_same);
}

/*****************************************************************************
 *
 * function:
 *   bool IsThirdOperatorReached(node *arg_node, node *arg_info)
 *
 * description:
 *   returns TRUE if the operator used in the definition of arg_node is not
 *   the same operator as SECONDOPERATOR.
 *
 ****************************************************************************/

static bool
IsThirdOperatorReached (node *arg_node, node *arg_info)
{

    bool is_thirdop;
    node *operator;

    DBUG_ENTER ("IsThirdOperatorReached");

    is_thirdop = FALSE;

    if ((INFO_DL_STATUSSECONDOPERATOR (arg_info) != 0)) {

        operator= GetUsedOperator (arg_node, arg_info);

        is_thirdop = !(IsSameOperator (operator, INFO_DL_SECONDOPERATOR (arg_info)));
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
        if (AVIS_SSAASSIGN (ID_AVIS (arg_node)) == NULL)
            reached = FALSE;
        else if (NODE_TYPE (LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (arg_node)))))
                 != N_prf)
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
 *   node* ResetFlags(node *arg_info)
 *
 * description:
 *   All NODELIST_FLAGS in COUNTLIST are set to 0.
 *
 ****************************************************************************/

static node *
ResetFlags (node *arg_info)
{

    nodelist *list;

    DBUG_ENTER ("ResetFlags");

    list = INFO_DL_COUNTLIST (arg_info);

    while (list != NULL) {

        DL_NODELIST_FLAGS (list) = 0;
        list = NODELIST_NEXT (list);
    }
    DBUG_RETURN (arg_info);
}

/*****************************************************************************
 *
 * function:
 *   node* RegisterMultipleUsableNodes(node *arg_info)
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

static node *
RegisterMultipleUsableNodes (node *arg_info)
{

    nodelist *currentlist, *tmplist;
    node *currentnode;

    DBUG_ENTER ("RegisterMultipleUsableNodes");

    currentlist = INFO_DL_COUNTLIST (arg_info);

    /*
     * traverse whole COUNTLIST
     */
    while (currentlist != NULL) {

        /*
         * compare operator in COUNTLIST withe MAINOPERATOR
         */
        if (IsSameOperator (INFO_DL_MAINOPERATOR (arg_info),
                            DL_NODELIST_OPERATOR (currentlist))) {

            currentnode = NODELIST_NODE (currentlist);

            tmplist = INFO_DL_COUNTLIST (arg_info);

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

    DBUG_RETURN (arg_info);
}

/*****************************************************************************
 *
 * function:
 *   node* AddNodeToOptimizedNodes(node* arg, node* arg_info)
 *
 * description:
 *   The function create a new NodelistNode with 'arg' as NODELIST_NODE
 *   and add it to OPTIMIZEDNODES.
 *
 ****************************************************************************/

static node *
AddNodeToOptimizedNodes (node *arg, node *arg_info)
{

    nodelist *newlist;

    DBUG_ENTER ("AddNodeToOptimizedNodes");

    newlist = MakeNodelistNode (arg, NULL);

    if (INFO_DL_OPTIMIZEDNODES (arg_info) == NULL) {

        INFO_DL_OPTIMIZEDNODES (arg_info) = newlist;

    } else {

        NODELIST_NEXT (newlist) = INFO_DL_OPTIMIZEDNODES (arg_info);
        INFO_DL_OPTIMIZEDNODES (arg_info) = newlist;
    }

    DBUG_RETURN (arg_info);
}

/*****************************************************************************
 *
 * function:
 *   node* MakeOperatorNode(node* exprs, node* op)
 *
 * description:
 *   This function creates and returns a new prf or ap node with the operator
 *   'op' and with exprs as the arguments.
 *
 ****************************************************************************/

static node *
MakeOperatorNode (node *exprs, node *op)
{

    node *new_op_node;
    char *apname, *apmod;

    DBUG_ENTER ("MakeOperatorNode");

    new_op_node = NULL;

    if (NODE_TYPE (op) == N_prf) {

        new_op_node = MakePrf (PRF_PRF (op), exprs);

    } else if (NODE_TYPE (op) == N_ap) {

        apname = StringCopy (AP_NAME (op));
        apmod = StringCopy (AP_MOD (op));

        new_op_node = MakeAp (apname, apmod, exprs);
        ID_AVIS (new_op_node) = ID_AVIS (op);
        ID_VARDEC (new_op_node) = ID_VARDEC (op);
        ID_STATUS (new_op_node) = ID_STATUS (op);
        ID_DEF (new_op_node) = ID_DEF (op);
    } else {

        DBUG_ASSERT ((1 == 0), "Unexpected node! Only N_prf or N_ap node are excepted!");
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

    currentElem = DupTree (currentElem);         /*pointer on duplicated node */
    currentElem = MakeExprs (currentElem, NULL); /* create N_expr-node */

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
    statustype status1;
    char *varname1, *newname1, *varmod1, *newmod1;

    DBUG_ENTER ("MakeExprsNodeFromAssignNode");

    newname1 = NULL;

    varname1 = IDS_NAME (LET_IDS (ASSIGN_INSTR (elem1)));
    DBUG_ASSERT ((varname1 != NULL),
                 "Unexpected error, existing variable has no name: IDS_NAME == NULL");
    newname1 = StringCopy (varname1);
    varmod1 = IDS_MOD (LET_IDS (ASSIGN_INSTR (elem1)));
    if (varmod1 != NULL)
        newmod1 = StringCopy (varmod1);
    else
        newmod1 = NULL;
    status1 = IDS_ATTRIB (LET_IDS (ASSIGN_INSTR (elem1)));

    newnode = MakeExprs (MakeId (newname1, newmod1, status1), NULL);

    ID_AVIS (EXPRS_EXPR (newnode)) = IDS_AVIS (LET_IDS (ASSIGN_INSTR (elem1)));

    ID_VARDEC (EXPRS_EXPR (newnode)) = IDS_VARDEC (LET_IDS (ASSIGN_INSTR (elem1)));

    DBUG_RETURN (newnode);
}

/*****************************************************************************
 *
 * function:
 *   node *MakeAssignLetNodeFromCurrentNode( node *newnode , node *arg_info)
 *
 * description:
 *   This function create a new assign-node with the newnode as an
 *   argument. The correct vardec-root-node is provided
 *   by the arg_info-node.
 *
 ****************************************************************************/

static node *
MakeAssignLetNodeFromCurrentNode (node *newnode, node *arg_info)
{

    node *newvardec;
    types *type;
    char *newname1, *newname2;

    DBUG_ENTER ("MakeAssignNodeFromExprsNode");

    type = MakeTypes (TYPES_BASETYPE ((INFO_DL_TYPE (arg_info))),
                      TYPES_DIM ((INFO_DL_TYPE (arg_info))), NULL, NULL, NULL);

    newname1 = TmpVar ();

    newvardec
      = MakeVardec (newname1, type, (BLOCK_VARDEC (INFO_DL_BLOCKNODE (arg_info))));

    BLOCK_VARDEC (INFO_DL_BLOCKNODE (arg_info)) = newvardec;

    newname2 = StringCopy (newname1);
    newnode = MakeAssignLet (newname2, newvardec, newnode);

    VARDEC_OBJDEF (newvardec) = newnode;
    AVIS_SSAASSIGN (VARDEC_AVIS (newvardec)) = newnode;
    AVIS_SSAASSIGN2 (VARDEC_AVIS (newvardec)) = newnode;

    DBUG_RETURN (newnode);
}

/*****************************************************************************
 *
 * function:
 *   node *MakeAllNodelistnodesToAssignNodes( nodelist* list, node *arg_info)
 *
 * description:
 *   All NODELIST_NODES in list are replaced by an assign-node, with the
 *   original node as an argument. The original nodes have to be
 *   id-nodes or other constant nodes!
 *
 ****************************************************************************/

static node *
MakeAllNodelistnodesToAssignNodes (nodelist *list, node *arg_info)
{

    node *node1, *newnode;

    DBUG_ENTER ("MakeAllNodelistnodesToAssignNodes");

    while (list != NULL) {

        if (NODE_TYPE (NODELIST_NODE (list)) != N_assign) {

            node1 = (NODELIST_NODE (list));

            DBUG_ASSERT ((IsConstant (node1, arg_info) || (N_id == NODE_TYPE (node1))),
                         "Unexpected node! No supported EXPR_EXPRS result");

            newnode = DupTree (node1);
            newnode = MakeAssignLetNodeFromCurrentNode (newnode, arg_info);

            NODELIST_NODE (list) = newnode;
        }

        list = NODELIST_NEXT (list);
    }

    DBUG_RETURN (arg_info);
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
        tmp = FreeNodelistNode (tmp);
    }

    DBUG_RETURN (tmp);
}

/*****************************************************************************
 *
 * function:
 *   node* CreateAssignNodes(node* arg_info)
 *
 * description:
 *   All NODELIST_NODE's of NONOPTLIST are expanded to assign-nodes
 *
 ****************************************************************************/

static node *
CreateAssignNodes (node *arg_info)
{

    nodelist *list;
    DBUG_ENTER ("CreateAssignNodes");

    list = INFO_DL_NONOPTLIST (arg_info);

    if (list != NULL) {

        arg_info
          = MakeAllNodelistnodesToAssignNodes (INFO_DL_NONOPTLIST (arg_info), arg_info);
    }

    DBUG_RETURN (arg_info);
}

/*****************************************************************************
 *
 * function:
 *   nodelist* CommitAssignNodes(nodelist *list, node* arg_info)
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
CommitAssignNodes (nodelist *list, node *arg_info)
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
            newnode = MakeOperatorNode (newnode, INFO_DL_MAINOPERATOR (arg_info));
            newnode = MakeAssignLetNodeFromCurrentNode (newnode, arg_info);

            /*
             * add used arguments to OPTIMIZEDNODES
             */
            arg_info = AddNodeToOptimizedNodes (node1, arg_info);
            arg_info = AddNodeToOptimizedNodes (node2, arg_info);

            /*
             * append new node on list
             */
            tmplist = MakeNodelistNode (newnode, NULL);
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
 *   node* IncludeMostFrequentNode(node* arg_info)
 *
 * description:
 *   connect MOSTFREQUENNODE with the remaining node in OPTLIST with operator
 *   SECONDOPERATOR.
 *   Add last node from OPTLIST to OPTIMIZEDNODE's.
 *   The new node remains in OPTLIST.
 *
 ****************************************************************************/

static node *
IncludeMostFrequentNode (node *arg_info)
{

    node *node1, *node2, *newnode, *tmpnode;

    DBUG_ENTER ("IncludeMostFrequentNode");

    node1 = INFO_DL_MOSTFREQUENTNODE (arg_info);
    node1 = MakeExprs (node1, NULL);

    tmpnode = NODELIST_NODE (INFO_DL_OPTLIST (arg_info));
    node2 = MakeExprsNodeFromAssignNode (tmpnode);
    EXPRS_NEXT (node2) = node1;

    newnode = MakeOperatorNode (node2, INFO_DL_SECONDOPERATOR (arg_info));
    newnode = MakeAssignLetNodeFromCurrentNode (newnode, arg_info);

    arg_info = AddNodeToOptimizedNodes (tmpnode, arg_info);

    NODELIST_NODE (INFO_DL_OPTLIST (arg_info)) = newnode;

    DBUG_RETURN (arg_info);
}

/*****************************************************************************
 *
 * function:
 *   node* IntegrateResults(node* arg_info)
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

static node *
IntegrateResults (node *arg_info)
{

    node *original, *old_nonoptnode, *old_optnode, *new_optnode, *new_nonoptnode,
      *oldargs;

    DBUG_ENTER ("IntegrateResults");

    /*
     * create exprs node from remaining node in OPTLIST
     */
    original = INFO_DL_LETNODE (arg_info);
    old_optnode = NODELIST_NODE (INFO_DL_OPTLIST (arg_info));
    new_optnode = MakeExprsNodeFromAssignNode (old_optnode);
    arg_info = AddNodeToOptimizedNodes (old_optnode, arg_info);

    new_nonoptnode = NULL;

    /*
     * check if there is a remaining node in NONOPTLIST
     */
    if (NULL != INFO_DL_NONOPTLIST (arg_info)) {

        /*
         * create exprs node from remaining node in NONOPTLIST
         */
        old_nonoptnode = NODELIST_NODE (INFO_DL_NONOPTLIST (arg_info));
        new_nonoptnode = MakeExprsNodeFromAssignNode (old_nonoptnode);
        arg_info = AddNodeToOptimizedNodes (old_nonoptnode, arg_info);

        oldargs = AP_OR_PRF_ARGS (LET_EXPR (original));

        /*
         * include new two exprs nodes as arguments of operator
         * in the starting node of optimization-cycle
         */
        original = LET_EXPR (original);
        EXPRS_NEXT (new_optnode) = new_nonoptnode;
        if (NODE_TYPE (original) == N_ap)
            AP_ARGS (original) = new_optnode;
        else
            PRF_ARGS (original) = new_optnode;

    } else {

        /*
         * only one node to include - remove operator node
         */
        oldargs = LET_EXPR (original);
        LET_EXPR (original) = EXPRS_EXPR (new_optnode);
    }
    FreeTree (oldargs);

    DBUG_RETURN (arg_info);
}

/*****************************************************************************
 *
 * function:
 *   node* CheckNode(node* arg_node, node* arg_info)
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

static node *
CheckNode (node *arg_node, node *arg_info)
{

    nodelist *head;

    DBUG_ENTER ("CheckNode");

    if (IsIdenticalNode (arg_node, INFO_DL_MOSTFREQUENTNODE (arg_info))) {

        /*
         * one MOSTFREQUENTNODE found
         * add first node in TMPLIST to OPTLIST
         * add other nodes to 'attrib2' of first node in a nodelist
         */

        /*
         * head is node with MOSTFREQUENTNODE, remove node from TMPLIST
         */
        head = INFO_DL_TMPLIST (arg_info);
        INFO_DL_TMPLIST (arg_info) = NODELIST_NEXT (INFO_DL_TMPLIST (arg_info));

        /*
         * add following nodes to PARENTNODES of head-node
         */
        DL_NODELIST_PARENTNODES (head) = INFO_DL_TMPLIST (arg_info);

        /*
         * add head to OPTLIST
         */
        NODELIST_NEXT (head) = INFO_DL_OPTLIST (arg_info);
        INFO_DL_OPTLIST (arg_info) = head;

        /*
         * prevent more registration of nodes,
         * because MOSTFREQUENTNODE is found already
         */
        INFO_DL_STATUSSECONDOPERATOR (arg_info) = 2;
    }

    DBUG_RETURN (arg_info);
}

/*****************************************************************************
 *
 * function:
 *   node* RemoveMostFrequentNode(node* arg_info)
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

static node *
RemoveMostFrequentNode (node *arg_info)
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

    current_elem = INFO_DL_MOSTFREQUENTNODE (arg_info);
    current_elem = DupTree (current_elem);
    INFO_DL_MOSTFREQUENTNODE (arg_info) = current_elem;
    current_elem = NULL;

    list = INFO_DL_OPTLIST (arg_info);

    while (list != NULL) {

        top_elem = NODELIST_NODE (list);

        if (DL_EXPRS_FLAG (top_elem) == 1) {

            /*
             * constant node - MFN is argument of MAINOPERATOR
             * replace MFN with neutral element of MAINOPERATOR
             */

            new_son = GetNeutralElement (INFO_DL_SECONDOPERATOR (arg_info), arg_info);
            new_son = MakeAssignLetNodeFromCurrentNode (new_son, arg_info);

            DL_EXPRS_FLAG (top_elem) = 0;

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
            if (IsIdenticalNode (first_arg, INFO_DL_MOSTFREQUENTNODE (arg_info))) {

                new_son = DupTree (second_arg);
                new_son = MakeAssignLetNodeFromCurrentNode (new_son, arg_info);

            }
            /*
             * is second_arg MOSTFREQUENTNODE?
             */
            else if (IsIdenticalNode (second_arg, INFO_DL_MOSTFREQUENTNODE (arg_info))) {

                new_son = DupTree (first_arg);
                new_son = MakeAssignLetNodeFromCurrentNode (new_son, arg_info);

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
                    && ((IsSameOperator (INFO_DL_SECONDOPERATOR (arg_info),
                                         LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (
                                           ID_AVIS (EXPRS_EXPR (current_elem)))))))
                        || (IsSameOperator (INFO_DL_MAINOPERATOR (arg_info),
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

                arg_info = AddNodeToOptimizedNodes (new_son, arg_info);

                /*
                 * create prf/ap-node and then assign-node
                 */

                new_parent
                  = MakeOperatorNode (new_parent, INFO_DL_SECONDOPERATOR (arg_info));
                new_parent = MakeAssignLetNodeFromCurrentNode (new_parent, arg_info);

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

    DBUG_RETURN (arg_info);
}

/*****************************************************************************
 *
 * function:
 *   OptTravElems(node *arg_node, node *arg_info)
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

static node *
OptTravElems (node *arg_node, node *arg_info)
{

    nodelist *list;

    DBUG_ENTER ("OptTravElems");

    if (IsConstant (EXPRS_EXPR (arg_node), arg_info)
        || ReachedArgument (EXPRS_EXPR (arg_node))
        || ReachedDefinition (EXPRS_EXPR (arg_node))
        || (!IsSupportedOperator (EXPRS_EXPR (arg_node), arg_info))
        || IsThirdOperatorReached (EXPRS_EXPR (arg_node), arg_info)) {

        /*
         * termination-condition reached
         * check if MOSTFREQUENTNODE is not found till now but SECONDOPERATOR reached
         */
        if (INFO_DL_STATUSSECONDOPERATOR (arg_info) == 1)
            arg_info = CheckNode (EXPRS_EXPR (arg_node), arg_info);

        /*
         * if node is a single node connected with MAINOPERATOR
         */
        if (INFO_DL_STATUSSECONDOPERATOR (arg_info) == 0) {

            nodelist *newlist;
            node *node_tmp;

            node_tmp = EXPRS_EXPR (arg_node);
            DL_EXPRS_FLAG (node_tmp) = 1;
            newlist = MakeNodelistNode (node_tmp, NULL);

            /*
             * check if arg_node is identical to MOSTFREQUENTNODE
             */
            if (IsIdenticalNode (EXPRS_EXPR (arg_node),
                                 INFO_DL_MOSTFREQUENTNODE (arg_info))) {

                if (ExistKnownNeutralElement (INFO_DL_SECONDOPERATOR (arg_info))) {

                    /*
                     * node is MFN and neutral element is known
                     */
                    NODELIST_NEXT (newlist) = INFO_DL_OPTLIST (arg_info);
                    INFO_DL_OPTLIST (arg_info) = newlist;
                }

            } else {

                /*
                 * node is not MFN or no known neutral Element: add to NONOPTLIST
                 */
                NODELIST_NEXT (newlist) = INFO_DL_NONOPTLIST (arg_info);
                INFO_DL_NONOPTLIST (arg_info) = newlist;
            }
        }

    } else if ((INFO_DL_STATUSSECONDOPERATOR (arg_info) == 0)
               && (IsSameOperator (INFO_DL_SECONDOPERATOR (arg_info),
                                   GetUsedOperator (EXPRS_EXPR (arg_node), arg_info)))) {

        /*
         * second operator reached for first time
         *    1.) save second operator
         *    2.) register new(!) arguments in TMPLIST
         *    3.) traverse to new arguments
         *    4.) Add TMPLIST to NONOPTLIST if neccessary
         */

        INFO_DL_STATUSSECONDOPERATOR (arg_info) = 1;

        /*
         * save nodes which will be traversed in TMPLIST
         * (neccessary to memorize the PARENTNODES of possible MOSTFREQUENTNODES)
         */
        list = MakeNodelistNode (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                                   AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node)))))),
                                 NULL);
        INFO_DL_TMPLIST (arg_info) = list;

        ASSIGN_STATUS (AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node)))) = 0;

        arg_info = OptTravElems (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                                   AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node)))))),
                                 arg_info);

        arg_info = OptTravElems (EXPRS_NEXT (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                                   AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node))))))),
                                 arg_info);

        if (INFO_DL_STATUSSECONDOPERATOR (arg_info) == 1) {

            node *newnode, *node1, *node2;
            nodelist *newlist;
            /*
             * no MOSTFREQUENTNODE found
             *
             * duplicate both exprs-nodes in TMPLIST
             * add result to NONOPTLIST
             * clear TMPLIST
             */

            node1 = NODELIST_NODE (INFO_DL_TMPLIST (arg_info));
            node2 = EXPRS_EXPR (EXPRS_NEXT (node1));
            node1 = EXPRS_EXPR (node1);

            node1 = MakeExprNodes (node1);
            node2 = MakeExprNodes (node2);

            newnode = node1;
            EXPRS_NEXT (newnode) = node2;

            newnode = MakeOperatorNode (newnode, INFO_DL_SECONDOPERATOR (arg_info));
            newnode = MakeAssignLetNodeFromCurrentNode (newnode, arg_info);

            newlist = MakeNodelistNode (newnode, NULL);

            NODELIST_NEXT (newlist) = INFO_DL_NONOPTLIST (arg_info);
            INFO_DL_NONOPTLIST (arg_info) = newlist;
            INFO_DL_TMPLIST (arg_info) = NULL;
        }

        INFO_DL_STATUSSECONDOPERATOR (arg_info) = 0;

    } else if ((INFO_DL_STATUSSECONDOPERATOR (arg_info) == 1)
               && IsSameOperator (INFO_DL_SECONDOPERATOR (arg_info),
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
        ASSIGN_STATUS (AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node)))) = 0;

        /*
         * register new(!) arguments in TMPLIST
         */
        list = MakeNodelistNode (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                                   AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node)))))),
                                 INFO_DL_TMPLIST (arg_info));
        INFO_DL_TMPLIST (arg_info) = list;

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
        if (INFO_DL_STATUSSECONDOPERATOR (arg_info) == 1) {
            list = INFO_DL_TMPLIST (arg_info);
            INFO_DL_TMPLIST (arg_info) = NODELIST_NEXT (INFO_DL_TMPLIST (arg_info));
            NODELIST_NODE (list) = NULL;
            NODELIST_NEXT (list) = NULL;
            Free (list);
        }
    } else {

        /*
         * main operator - traverse
         * or MOSTFREQUENTNODE already found - traverse till end
         */

        ASSIGN_STATUS (AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node)))) = 0;

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
 *   node *CreateOptLists(node* arg_node, node* arg_info)
 *
 * description:
 *   Starting function for investigate the OPTLIST and NONOPTLIST
 *   in relation to MOSTFREQUENTNODE.
 *
 *****************************************************************************/

static node *
CreateOptLists (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("CreateOptLists");

    INFO_DL_OPTLIST (arg_info) = NULL;
    INFO_DL_NONOPTLIST (arg_info) = NULL;

    arg_info = OptTravElems (PRF_ARGS (arg_node), arg_info);
    arg_info = OptTravElems (EXPRS_NEXT (PRF_ARGS (arg_node)), arg_info);

    DBUG_RETURN (arg_info);
}

/*****************************************************************************
 *
 * function:
 *   nodelist* FindNodeInList(nodelist* list, node* arg_node, node* operator)
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
                if ((NODE_TYPE (operator) == N_prf)
                    && (PRF_PRF (operator) == PRF_PRF (current_operator)))
                    equal_op = TRUE;

                if ((NODE_TYPE (operator) == N_ap)
                    && (AP_FUNDEF (operator) == AP_FUNDEF (current_operator)))
                    equal_op = TRUE;

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
 *   node* FindAndSetMostFrequentNode(node* arg_info)
 *
 * description:
 *   Add memorized nodes connected with MAINOOERATOR in COUNTLIST to nodes
 *   connected with a operator for which the neutral element is known.
 *   Than the most frequent node is investigated and saved in
 *   MOSTFREQUENTNODE
 *
 *****************************************************************************/

static node *
FindAndSetMostFrequentNode (node *arg_info)
{

    int maxOccurence;
    nodelist *list;

    DBUG_ENTER ("FindAndSetMostFrequentNode");

    arg_info = RegisterMultipleUsableNodes (arg_info);

    maxOccurence = 0;
    list = INFO_DL_COUNTLIST (arg_info);

    while (list != NULL) {

        if ((NODELIST_INT (list) > maxOccurence)
            && (!IsSameOperator (DL_NODELIST_OPERATOR (list),
                                 INFO_DL_MAINOPERATOR (arg_info)))) {

            maxOccurence = NODELIST_INT (list);

            INFO_DL_SECONDOPERATOR (arg_info) = DL_NODELIST_OPERATOR (list);
            INFO_DL_MOSTFREQUENTNODE (arg_info) = NODELIST_NODE (list);
            INFO_DL_OCCURENCEOFMOSTFREQUENTNODE (arg_info) = maxOccurence;
        }

        list = NODELIST_NEXT (list);
    }

    DBUG_RETURN (arg_info);
}

/*****************************************************************************
 *
 * function:
 *   node *RegisterNode(node *arg_node, node *arg_info)
 *
 * description:
 *   If arg_node with the correct operator exists in COUNTLIST,
 *   the counter is increased and the flag to prevent multiple counting
 *   is set. Else a new listelement is added to COUNTLIST with counter == 1.
 *
 *****************************************************************************/

static node *
RegisterNode (node *arg_node, node *arg_info)
{

    node *lastOperator;
    nodelist *node_in_list;
    nodelist *newnodelistnode;

    DBUG_ENTER ("RegisterNode");

    if (INFO_DL_STATUSSECONDOPERATOR (arg_info) == 1)
        lastOperator = INFO_DL_SECONDOPERATOR (arg_info);
    else
        lastOperator = INFO_DL_MAINOPERATOR (arg_info);

    node_in_list = FindNodeInList (INFO_DL_COUNTLIST (arg_info), arg_node, lastOperator);

    if (node_in_list == NULL) {

        /*
         * node with operator is not in COUNTLIST
         * create new nodelistnode, becomes (after all) first element of COUNTLIST
         * insert arg_node, operator and initilize counter
         */

        newnodelistnode = MakeNodelistNode (arg_node, NULL);

        NODELIST_NODE (newnodelistnode) = arg_node;

        DL_NODELIST_OPERATOR (newnodelistnode) = lastOperator;
        DL_NODELIST_FLAGS (newnodelistnode) = 1;

        NODELIST_INT (newnodelistnode) = 1;

        NODELIST_NEXT (newnodelistnode) = INFO_DL_COUNTLIST (arg_info);
        INFO_DL_COUNTLIST (arg_info) = newnodelistnode;

    } else {

        /*
         * node with operator is in COUNTLIST
         * only increase counter
         */
        if (DL_NODELIST_FLAGS (node_in_list) == 0)
            NODELIST_INT (node_in_list) = NODELIST_INT (node_in_list) + 1;
        DL_NODELIST_FLAGS (node_in_list) = 1;
    }

    DBUG_RETURN (arg_info);
}

/*****************************************************************************
 *
 * function:
 *   SearchTravElems(node *arg_node, node *arg_info)
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

static node *
SearchTravElems (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("SearchTravElems");

    if (IsConstant (EXPRS_EXPR (arg_node), arg_info)) {

        /*
         * constant argument reached
         */
        arg_info = RegisterNode (EXPRS_EXPR (arg_node), arg_info);

    } else if (ReachedArgument (EXPRS_EXPR (arg_node))
               || ReachedDefinition (EXPRS_EXPR (arg_node))
               || (!IsSupportedOperator (EXPRS_EXPR (arg_node), arg_info))
               || IsThirdOperatorReached (EXPRS_EXPR (arg_node), arg_info)) {

        /*
         * other termination-condition reached
         */
        arg_info = RegisterNode (EXPRS_EXPR (arg_node), arg_info);

    } else if ((IsSupportedOperator (EXPRS_EXPR (arg_node), arg_info))
               && (INFO_DL_STATUSSECONDOPERATOR (arg_info) == 0)
               && IsValidSecondOperator (EXPRS_EXPR (arg_node), arg_info)) {

        /*
         * second operator reached - save second operator and traverse
         */

        arg_info = ResetFlags (arg_info);

        INFO_DL_SECONDOPERATOR (arg_info)
          = GetUsedOperator (EXPRS_EXPR (arg_node), arg_info);
        INFO_DL_STATUSSECONDOPERATOR (arg_info) = 1;

        arg_info = SearchTravElems (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                                      AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node)))))),
                                    arg_info);
        arg_info
          = SearchTravElems (EXPRS_NEXT (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                               AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node))))))),
                             arg_info);

        INFO_DL_SECONDOPERATOR (arg_info) = NULL;
        INFO_DL_STATUSSECONDOPERATOR (arg_info) = 0;

        arg_info = ResetFlags (arg_info);
    } else {

        /*
         * still main operator or second operator: traverse
         */
        arg_info = SearchTravElems (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                                      AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node)))))),
                                    arg_info);
        arg_info
          = SearchTravElems (EXPRS_NEXT (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                               AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node))))))),
                             arg_info);
    }

    DBUG_RETURN (arg_info);
}

/*****************************************************************************
 *
 * function:
 *   node *SearchMostFrequentNode(node* arg_node, node* arg_info)
 *
 * description:
 *   Starting function for investigate the MOSTFREQUENTNODE.
 *   Both exprs-nodes are traversed and every termination-node is
 *   memorized (with used operator) and his occurences counted.
 *   After the this traversal the created COUNTLIST is evaluated and the
 *   MOSTFREQUENTNODE is set.
 *
 *****************************************************************************/

static node *
SearchMostFrequentNode (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("SearchMostFrequentNode");

    INFO_DL_COUNTLIST (arg_info) = NULL;

    /*
     * Traverse through the definitions of arg_node and collect nodes
     */

    INFO_DL_MAINOPERATOR (arg_info) = arg_node;

    arg_info = SearchTravElems (PRF_ARGS (arg_node), arg_info);
    arg_info = SearchTravElems (EXPRS_NEXT (PRF_ARGS (arg_node)), arg_info);

    /*
     * Now, all untraversable nodes are added to COUNTLIST.
     * Next step is to find the most frequent node in list
     * and modify SECONDOPERATOR in arg_info with the operation of the 'winner-node'
     */

    arg_info = FindAndSetMostFrequentNode (arg_info);

    DeleteNodelist (INFO_DL_COUNTLIST (arg_info));
    INFO_DL_COUNTLIST (arg_info) = NULL;

    DBUG_RETURN (arg_info);
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
DistributiveLaw (node *arg_node, node *a)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("DistributiveLaw");

    if (arg_node != NULL) {
        arg_info = MakeInfo ();

        old_tab = act_tab;
        act_tab = dl_tab;

        arg_node = Trav (arg_node, arg_info);

        act_tab = old_tab;

        arg_info = FreeTree (arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   DLblock(node *arg_node, node *arg_info)
 *
 * description:
 *   store block-node for access to vardec-nodes
 *   reset nodelists
 *   traverse through block-nodes
 *
 ****************************************************************************/

node *
DLblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DLblock");

    if (BLOCK_INSTR (arg_node) != NULL) {
        /*
         * store pointer on actual N_block-node for append of new N_vardec nodes
         */
        if (BLOCK_VARDEC (arg_node) != NULL) {
            INFO_DL_BLOCKNODE (arg_info) = arg_node;
        }

        INFO_DL_OPTLIST (arg_info) = NULL;
        INFO_DL_NONOPTLIST (arg_info) = NULL;

        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   DLassign(node *arg_node, node *arg_info)
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
DLassign (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("DLassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {

        ASSIGN_STATUS (arg_node) = 1;

        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);

        if (INFO_DL_OPTIMIZEDNODES (arg_info) != NULL) {

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

            while (INFO_DL_OPTIMIZEDNODES (arg_info) != NULL) {

                ASSIGN_NEXT (NODELIST_NODE (INFO_DL_OPTIMIZEDNODES (arg_info)))
                  = assign_next;
                assign_next = NODELIST_NODE (INFO_DL_OPTIMIZEDNODES (arg_info));

                tmplist = INFO_DL_OPTIMIZEDNODES (arg_info);

                INFO_DL_OPTIMIZEDNODES (arg_info) = NODELIST_NEXT (tmplist);

                NODELIST_NEXT (tmplist) = NULL;
                NODELIST_NODE (tmplist) = NULL;
                tmplist = DeleteNodelist (tmplist);
            }

            INFO_DL_OPTIMIZEDNODES (arg_info) = NULL;

            ASSIGN_NEXT (arg_node) = assign_next;
        }
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
 *   DLlet(node *arg_node, node *arg_info)
 *
 * description:
 *   store current let-node to include last created new primitive-node
 *   traverse N_let-nodes
 *
 ****************************************************************************/

node *
DLlet (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("DLlet");
    if (LET_EXPR (arg_node) != NULL) {

        INFO_DL_LETNODE (arg_info) = arg_node;
        INFO_DL_IEEEFLAG (arg_info) = 0;

        if ((LET_IDS (arg_node) != NULL) && (IDS_AVIS (LET_IDS (arg_node)) != NULL))
            INFO_DL_TYPE (arg_info)
              = VARDEC_OR_ARG_TYPE (AVIS_VARDECORARG (IDS_AVIS (LET_IDS (arg_node))));

        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

        /*
         * if optimization was neccessary:
         * both remaining nodes get arguments of start-assign-node
         */
        if ((INFO_DL_OCCURENCEOFMOSTFREQUENTNODE (arg_info) > 1)
            && ((!enforce_ieee) || (INFO_DL_IEEEFLAG (arg_info) == 0)))
            arg_info = IntegrateResults (arg_info);

        INFO_DL_OCCURENCEOFMOSTFREQUENTNODE (arg_info) = 0;
    }
    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *DLPrfOrAp(node* arg_node, node* arg_info)
 *
 * description:
 *   Starting point of one optimization-cycle.
 *   First the operator the operator is checked, if the operator is supported,
 *   after that the MOSTFREQUENTNODE is investigated and then the
 *   optimization starts with reconstructing the syntax-tree.
 *
 ****************************************************************************/

node *
DLPrfOrAp (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("DLprfOrap");

    if (((NODE_TYPE (arg_node) == N_prf) && (NODE_TYPE (PRF_ARGS (arg_node)) == N_exprs))
        || ((NODE_TYPE (arg_node) == N_ap) && (AP_ARGS (arg_node) != NULL)
            && (NODE_TYPE (AP_ARGS (arg_node)) == N_exprs))) {

        /*
         * memorize MAINOPERATOR
         */
        INFO_DL_MAINOPERATOR (arg_info) = arg_node;

        /*
         * is MAINOPERATOR supported?
         */
        if (CheckOperator (INFO_DL_MAINOPERATOR (arg_info), arg_info)) {

            INFO_DL_OPTLIST (arg_info) = NULL;
            INFO_DL_NONOPTLIST (arg_info) = NULL;

            /*
             * investigate the most frequent node
             */
            arg_info = SearchMostFrequentNode (arg_node, arg_info);

            /*
             * Exist an optimization case?
             */
            if ((INFO_DL_OCCURENCEOFMOSTFREQUENTNODE (arg_info) > 1)
                && ((!enforce_ieee) || (INFO_DL_IEEEFLAG (arg_info) == 0))) {

                /*
                 * increase optimization counter
                 */

                dl_expr = dl_expr + INFO_DL_OCCURENCEOFMOSTFREQUENTNODE (arg_info) - 1;

                /*
                 * Create OPTLIST and NONOPTLIST in regard to MOSTFREQUENTNODE
                 * nodes in OPTLIST contain (somewhere) in their definition the
                 * MOSTFREQENTNODE all other nodes are collected in NONOPTLIST
                 */

                arg_info = CreateOptLists (arg_node, arg_info);

                /*
                 * OPTIMIZATION
                 *
                 * duplicate MFN and remove then all MFN from OPTLIST
                 */
                arg_info = RemoveMostFrequentNode (arg_info);

                /*
                 * remove all original nodes from NONOPTLIST by creating
                 * new assign nodes with original nodes as arguments
                 */
                arg_info = CreateAssignNodes (arg_info);

                /*
                 * connect node in OPTLIST / NONOPTLIST till only one assign-node remain
                 */
                INFO_DL_NONOPTLIST (arg_info)
                  = CommitAssignNodes (INFO_DL_NONOPTLIST (arg_info), arg_info);
                INFO_DL_OPTLIST (arg_info)
                  = CommitAssignNodes (INFO_DL_OPTLIST (arg_info), arg_info);

                /*
                 * connect MFN with remaining node in OPTLIST
                 */
                arg_info = IncludeMostFrequentNode (arg_info);
            }
        }

        INFO_DL_MAINOPERATOR (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}
