/* *
 * $Log$
 * Revision 1.5  2003/02/15 16:48:00  mwe
 * bugs removed
 * chenged assignment for INFO_DL_TYPES (because former assignment was not up to date)
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
#define INFO_DL_CURRENTASSIGN(n) (n->node[2])
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
 *   store types-node as shape for new types-nodes
 *   reset nodelists
 *   traverse through block-nodes
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
            /*   INFO_DL_TYPE(arg_info) = VARDEC_TYPE( BLOCK_VARDEC(arg_node));*/
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

            dl_expr = dl_expr + 1;

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

            INFO_DL_CURRENTASSIGN (arg_info) = arg_node;

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
    /* union für 'prf', 'nutzerdefinierte fkt' */
    union operator;

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

node *
DLPrfOrAp (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("DLprfOrap");

    if (((NODE_TYPE (arg_node) == N_prf) && (NODE_TYPE (PRF_ARGS (arg_node)) == N_exprs))
        || ((NODE_TYPE (arg_node) == N_ap) && (AP_ARGS (arg_node) != NULL)
            && (NODE_TYPE (AP_ARGS (arg_node)) == N_exprs))) {

        INFO_DL_MAINOPERATOR (arg_info) = arg_node;

        if (CheckOperator (INFO_DL_MAINOPERATOR (arg_info), arg_info)) {

            INFO_DL_OPTLIST (arg_info) = NULL;
            INFO_DL_NONOPTLIST (arg_info) = NULL;

            arg_info = SearchMostFrequentNode (arg_node, arg_info);

            if ((INFO_DL_OCCURENCEOFMOSTFREQUENTNODE (arg_info) > 1)
                && ((!enforce_ieee) || (INFO_DL_IEEEFLAG (arg_info) == 0))) {

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

node *
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
 *   All nodes TravElems terminates for, are collected in arg_info with
 *   function AddNode (argument '1': constant node ; '0' no constant node)
 *
 ****************************************************************************/

node *
SearchTravElems (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("SearchTravElems");

    if (DLIsConstant (EXPRS_EXPR (arg_node), arg_info)) {

        /*
         * constant argument reached
         */
        arg_info = RegisterNode (EXPRS_EXPR (arg_node), arg_info);

    } else if (DLReachedArgument (EXPRS_EXPR (arg_node))
               || DLReachedDefinition (EXPRS_EXPR (arg_node))
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
         * second operator reached - save second operator, traverse
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
         * still main operator or second operator - traverse
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

bool
DLIsConstant (node *arg_node, node *arg_info)
{
    bool is_constant;

    DBUG_ENTER ("DLIsConstant");

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

bool
IsSupportedOperator (node *arg_node, node *arg_info)
{

    bool support;
    node *operator;

    DBUG_ENTER ("UnsupportedOperator");
    support = FALSE;

    operator= GetUsedOperator (arg_node, arg_info);

    support = CheckOperator (operator, arg_info );

    DBUG_RETURN (support);
}

bool
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

node *
GetUsedOperator (node *arg_node, node *arg_info)
{

    node *operator;

    DBUG_ENTER ("GetUsedOperator");

    operator= LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (arg_node))));

    DBUG_RETURN (operator);
}

bool
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

int
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

bool
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

bool
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
 *   bool DLReachedArgument(node *arg_node)
 *
 * description:
 *   returns TRUE if the arg_node is an argument defined outside current
 *   block node
 *
 ****************************************************************************/

bool
DLReachedArgument (node *arg_node)
{

    bool reached;
    DBUG_ENTER ("DLReachedArgument");

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
 *   bool DLReachedDefinition(node *arg_node)
 *
 * description:
 *   returns TRUE if the definition of the arg_node contain no prf-node
 *
 ****************************************************************************/

bool
DLReachedDefinition (node *arg_node)
{

    bool reached;
    DBUG_ENTER ("DLReachedDefinition");

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

node *
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

node *
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

nodelist *
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

            if ((NODE_TYPE (operator) == NODE_TYPE (current_operator))
                && (NODE_TYPE (arg_node) == NODE_TYPE (current_node))) {

                if ((NODE_TYPE (operator) == N_prf)
                    && (PRF_PRF (operator) == PRF_PRF (current_operator)))
                    equal_op = TRUE;

                if ((NODE_TYPE (operator) == N_ap)
                    && (AP_FUNDEF (operator) == AP_FUNDEF (current_operator)))
                    equal_op = TRUE;

                if (equal_op) {

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

node *
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

node *
RegisterMultipleUsableNodes (node *arg_info)
{

    nodelist *currentlist, *tmplist;
    node *currentnode;

    DBUG_ENTER ("RegisterMultipleUsableNodes");

    currentlist = INFO_DL_COUNTLIST (arg_info);

    while (currentlist != NULL) {

        if (IsSameOperator (INFO_DL_MAINOPERATOR (arg_info),
                            DL_NODELIST_OPERATOR (currentlist))) {

            currentnode = NODELIST_NODE (currentlist);

            tmplist = INFO_DL_COUNTLIST (arg_info);

            while (tmplist != NULL) {

                if (tmplist != currentlist) {

                    if (IsIdenticalNode (NODELIST_NODE (tmplist), currentnode)) {

                        /*
                         * different, but identical node found
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

nodelist *
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

node *
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
 *   All nodes TravElems terminates for, are collected in arg_info with
 *   function AddNode (argument '1': constant node ; '0' no constant node)
 *
 ****************************************************************************/

node *
OptTravElems (node *arg_node, node *arg_info)
{

    nodelist *list;

    DBUG_ENTER ("OptTravElems");

    if (DLIsConstant (EXPRS_EXPR (arg_node), arg_info)
        || DLReachedArgument (EXPRS_EXPR (arg_node))
        || DLReachedDefinition (EXPRS_EXPR (arg_node))
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
         * second operator reached - save second operator, traverse
         */

        INFO_DL_STATUSSECONDOPERATOR (arg_info) = 1;

        /*
         * vorher bnur arg_node als Argument! eine ebene zu früh
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
             * no MOSTFREQUENTNODE found - add node to NONOPTLIST
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
         * second operator - register 'source'-node, traverse, remove 'source'-node
         */

        ASSIGN_STATUS (AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node)))) = 0;

        /*
         * vorher nur arg_node als Argument
         */

        list = MakeNodelistNode (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                                   AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node)))))),
                                 INFO_DL_TMPLIST (arg_info));
        INFO_DL_TMPLIST (arg_info) = list;

        arg_info = OptTravElems (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                                   AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node)))))),
                                 arg_info);
        arg_info = OptTravElems (EXPRS_NEXT (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                                   AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node))))))),
                                 arg_info);

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

node *
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

bool
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

node *
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
     *     - remove MOSTFREQUENTNODE, change prf/ap to let-node
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

            new_son = GetNeutralElement (
              INFO_DL_SECONDOPERATOR (arg_info)); /* new_son is N_expr-node */
            new_son = MakeAssignLetNodeFromCurrentNode (new_son, arg_info);

            DL_EXPRS_FLAG (top_elem) = 0;

        } else {
            /*
             * one of the nodes son is MFN
             * remove MFN
             */
            /*
             *   segmentation fault - wenn top_elem eine Konstante/definition ist
             *      first_arg =  EXPRS_EXPR(PRF_ARGS( LET_EXPR( ASSIGN_INSTR(
             * AVIS_SSAASSIGN ( ID_AVIS( EXPRS_EXPR(top_elem))))))); second_arg =
             * EXPRS_EXPR(EXPRS_NEXT( PRF_ARGS( LET_EXPR( ASSIGN_INSTR( AVIS_SSAASSIGN (
             * ID_AVIS( EXPRS_EXPR(top_elem))))))));
             */

            first_arg = EXPRS_EXPR (top_elem);
            second_arg = EXPRS_EXPR (EXPRS_NEXT (top_elem));

            if (IsIdenticalNode (first_arg, INFO_DL_MOSTFREQUENTNODE (arg_info))) {

                new_son = DupTree (second_arg);
                new_son = MakeAssignLetNodeFromCurrentNode (new_son, arg_info);

            } else if (IsIdenticalNode (second_arg,
                                        INFO_DL_MOSTFREQUENTNODE (arg_info))) {

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
                 * get both arguments of current_element
                 * replace old_son with new_son
                 * make new assign nodes with both arguments and SECONDOPERATOR
                 * save new_son in correct list
                 * new assign node is new_son again
                 */
                current_elem = NODELIST_NODE (tmp_list);
                second_argnode = NULL;

                if (!DLIsConstant (EXPRS_EXPR (current_elem), arg_info)
                    && !DLReachedArgument (EXPRS_EXPR (current_elem))
                    && !DLReachedDefinition (EXPRS_EXPR (current_elem))) {

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
                } else if (!DLIsConstant (EXPRS_EXPR (EXPRS_NEXT (current_elem)),
                                          arg_info)
                           && !DLReachedArgument (EXPRS_EXPR (EXPRS_NEXT (current_elem)))
                           && !DLReachedDefinition (
                                EXPRS_EXPR (EXPRS_NEXT (current_elem)))) {

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
                 * create new id-node for new_son and create new exprs-node with result
                 */

                new_parent = MakeExprsNodeFromAssignNode (new_son);

                EXPRS_NEXT (new_parent) = second_argnode;

                /*
                 * save new_son in correct list
                 */

                arg_info = AddNodeToOptimizedNodes (new_son, arg_info);

                /*
                 * create prf/ap-node and then assign-node
                 */

                new_parent
                  = MakeOperatorNode (new_parent, INFO_DL_SECONDOPERATOR (arg_info));
                new_parent = MakeAssignLetNodeFromCurrentNode (new_parent, arg_info);

                old_son = current_elem;
                new_son = new_parent;

                tmp_list = NODELIST_NEXT (tmp_list);
            }
        }
        /*
         * no more parent nodes exist
         * only need to save new_son in list for later usage
         */

        NODELIST_NODE (list) = new_son;

        list = NODELIST_NEXT (list);
    }

    DBUG_RETURN (arg_info);
}

node *
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

node *
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

node *
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
 *   node *MakeAssignLetNodeFromCurrentNode( node *newnode , node *arg_info)
 *
 * description:
 *   This function create a new assign-node with the exprs-node as an
 *   argument. The correct vardec-root-node is provided
 *   by the arg_info-node.
 *
 ****************************************************************************/

node *
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
 *   node *MakeExprsNodeFromAssignNode(node *elem1)
 *
 * description:
 *   The assign-nodes get argument of new id-node. This id-node
 *   gets an argument of a new exprs-node.
 *
 ****************************************************************************/

node *
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

node *
CreateAssignNodes (node *arg_info)
{

    nodelist *list; /*, *lastnodelistnode, *tmplist;
    node *node1, *node2, *newnode;

    int counter, number_of_nodes;*/

    DBUG_ENTER ("CreateAssignNodes");

    list = INFO_DL_NONOPTLIST (arg_info);

    if (list != NULL) {
        /*
         * lastnodelistnode = list;
         *
         *while (lastnodelistnode != NULL){
         *
         *  node1 = NODELIST_NODE(lastnodelistnode);
         *  node2 = EXPRS_NEXT(node1);
         *  if (node2 != NULL){
         *
         *tmplist = MakeNodelistNode(node2);
         *
         *
         *  }
         *
         *  lastnodelistnode = NODELIST_NEXT(lastnodelistnode);
         *
         *}*/

        arg_info
          = MakeAllNodelistnodesToAssignNodes (INFO_DL_NONOPTLIST (arg_info), arg_info);

        /*   lastnodelistnode = list;

        counter = 0;

        while (lastnodelistnode != NULL){

          lastnodelistnode = NODELIST_NEXT(lastnodelistnode);
          counter = counter + 1;

        }

        number_of_nodes = counter;

        for( counter = 1; (2*counter) <= number_of_nodes; counter = counter + 1 ){

          node1 = NODELIST_NODE(list);
          node2 = NODELIST_NODE(NODELIST_NEXT(list));

          node1 = MakeExprsNodeFromAssignNode(node1);
          newnode = MakeExprsNodeFromAssignNode(node2);
          EXPRS_NEXT(newnode) = node1;
          newnode = MakeOperatorNode(newnode, INFO_DL_MAINOPERATOR(arg_info));
          newnode = MakeAssignLetNodeFromCurrentNode(newnode, arg_info);

          AddToOptimizedNodes(node1, arg_info);
          AddToOptimizedNodes(node1, arg_info);

          NODELIST_NODE(list) = newnode;
          NODELIST_NODE(NODELIST_NEXT(list)) = NULL;
          tmplist = NODELIST_NEXT(list);
          NODELIST_NEXT(list) = NODELIST_NEXT(NODELIST_NEXT(list));
          NODELIST_NEXT(tmplist) = NULL;
          tmplist = DeleteNodelist(tmplist);

          list = NODELIST_NEXT(NODELIST_NEXT(list));

        }

        if (div(number_of_nodes,2).rem == 1){

          node1 = NODELIST_NODE(list);

          newnode = MakeExprsNodeFromAssignNode(node1);
          newnode = MakeAssignLetNodeFromCurrentNode(newnode, arg_info);

          AddToOptimizedNodes(node1, arg_info);

          NODELIST_NODE(list) = newnode;

          }*/
    }

    DBUG_RETURN (arg_info);
}

node *
MakeAllNodelistnodesToAssignNodes (nodelist *list, node *arg_info)
{

    node *node1, *newnode;

    DBUG_ENTER ("MakeAllNodelistnodesToAssignNodes");

    while (list != NULL) {

        if (NODE_TYPE (NODELIST_NODE (list)) != N_assign) {

            node1 = (NODELIST_NODE (list));

            DBUG_ASSERT ((DLIsConstant (node1, arg_info) || (N_id == NODE_TYPE (node1))),
                         "Unexpected node! No supported EXPR_EXPRS result");

            newnode = DupTree (node1);
            newnode = MakeAssignLetNodeFromCurrentNode (newnode, arg_info);

            NODELIST_NODE (list) = newnode;
        }

        list = NODELIST_NEXT (list);
    }

    DBUG_RETURN (arg_info);
}

nodelist *
CommitAssignNodes (nodelist *list, node *arg_info)
{

    nodelist *lastnodelistnode, *tmplist, *startlist;
    node *node1, *node2, *newnode, *tmpnode;

    DBUG_ENTER ("CommitAssignNodes");

    startlist = list;

    if (list != NULL) {

        lastnodelistnode = list;

        while (NODELIST_NEXT (lastnodelistnode) != NULL) {

            lastnodelistnode = NODELIST_NEXT (lastnodelistnode);
        }

        while (list != lastnodelistnode) {

            node1 = NODELIST_NODE (list);
            node2 = NODELIST_NODE (NODELIST_NEXT (list));

            tmpnode = MakeExprsNodeFromAssignNode (node1);
            newnode = MakeExprsNodeFromAssignNode (node2);
            EXPRS_NEXT (newnode) = tmpnode;
            newnode = MakeOperatorNode (newnode, INFO_DL_MAINOPERATOR (arg_info));
            newnode = MakeAssignLetNodeFromCurrentNode (newnode, arg_info);

            arg_info = AddToOptimizedNodes (node1, arg_info);
            arg_info = AddToOptimizedNodes (node2, arg_info);

            tmplist = MakeNodelistNode (newnode, NULL);
            NODELIST_NEXT (lastnodelistnode) = tmplist;
            lastnodelistnode = NODELIST_NEXT (lastnodelistnode);

            /*
             *      tmplist = NODELIST_NEXT(list);
             *      NODELIST_NEXT(list) = NODELIST_NEXT(NODELIST_NEXT(list));
             *      NODELIST_NODE(tmplist) = NULL;
             *      NODELIST_NEXT(tmplist) = NULL;
             *      tmplist = DeleteNodelist(tmplist);
             */

            list = NODELIST_NEXT (NODELIST_NEXT (list));
        }

        while (startlist != lastnodelistnode) {

            tmplist = startlist;
            startlist = NODELIST_NEXT (startlist);

            NODELIST_NEXT (tmplist) = NULL;
            tmplist = DeleteNodelist (tmplist);
        }
    }

    DBUG_RETURN (startlist);
}

node *
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

    arg_info = AddToOptimizedNodes (tmpnode, arg_info);

    NODELIST_NODE (INFO_DL_OPTLIST (arg_info)) = newnode;

    DBUG_RETURN (arg_info);
}

node *
IntegrateResults (node *arg_info)
{

    node *original, *old_nonoptnode, *old_optnode, *new_optnode, *new_nonoptnode,
      *oldargs;

    DBUG_ENTER ("IntegrateResults");

    original = INFO_DL_LETNODE (arg_info);
    old_optnode = NODELIST_NODE (INFO_DL_OPTLIST (arg_info));
    new_optnode = MakeExprsNodeFromAssignNode (old_optnode);
    arg_info = AddToOptimizedNodes (old_optnode, arg_info);

    new_nonoptnode = NULL;
    if (NULL != INFO_DL_NONOPTLIST (arg_info)) {
        old_nonoptnode = NODELIST_NODE (INFO_DL_NONOPTLIST (arg_info));
        new_nonoptnode = MakeExprsNodeFromAssignNode (old_nonoptnode);
        arg_info = AddToOptimizedNodes (old_nonoptnode, arg_info);

        oldargs = AP_OR_PRF_ARGS (LET_EXPR (original));

        original = LET_EXPR (original);
        EXPRS_NEXT (new_optnode) = new_nonoptnode;
        if (NODE_TYPE (original) == N_ap)
            AP_ARGS (original) = new_optnode;
        else
            PRF_ARGS (original) = new_optnode;

    } else {

        oldargs = LET_EXPR (original);
        LET_EXPR (original) = EXPRS_EXPR (new_optnode);
    }
    FreeTree (oldargs);

    DBUG_RETURN (arg_info);
}

node *
AddToOptimizedNodes (node *node1, node *arg_info)
{

    nodelist *new;

    DBUG_ENTER ("AddToOptimizedNodes");

    new = MakeNodelistNode (node1, NULL);

    NODELIST_NEXT (new) = INFO_DL_OPTIMIZEDNODES (arg_info);
    INFO_DL_OPTIMIZEDNODES (arg_info) = new;

    DBUG_RETURN (arg_info);
}

node *
GetNeutralElement (node *op)
{

    node *neutral_elem;

    DBUG_ENTER ("GetNeutralElement");

    neutral_elem = NULL;

    if (NODE_TYPE (op) == N_prf) {

        if (PRF_PRF (op) == F_add_SxS) {

            neutral_elem = MakeNum (0);

        } else if (PRF_PRF (op) == F_mul_SxS) {

            neutral_elem = MakeNum (1);
            /*neutral_elem = MakeExprs(neutral_elem, NULL);*/

        } else if (PRF_PRF (op) == F_and) {

            neutral_elem = MakeBool (TRUE);
            /*neutral_elem = MakeExprs(neutral_elem, NULL);*/

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

bool
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
