/* *
 * $Log$
 * Revision 1.16  2003/02/09 18:06:06  mwe
 * bug removed: now optimization don't start, if only constant
 * nodes are available
 *
 * Revision 1.15  2002/11/13 16:36:18  mwe
 * unused variable removed
 *
 * Revision 1.14  2002/11/04 09:41:55  mwe
 * completed comments
 *
 * Revision 1.13  2002/11/02 20:59:44  mwe
 * removed bug - store pointer to block node in function ALblock again
 *
 * Revision 1.12  2002/10/25 16:02:55  mwe
 * implement usage of enforce_ieee-option
 *
 * Revision 1.11  2002/10/23 15:34:25  mwe
 * remove bug - store pointer to block node in function AssociativeLaw() and not in
 *ALblock()
 *
 * Revision 1.10  2002/10/16 11:47:43  mwe
 * reduce optimization expenditure
 * prevent useless optimization of unused nodes
 *
 * Revision 1.9  2002/10/14 16:15:43  mwe
 * F_add_AxA changed to F_add_SxS
 *
 * Revision 1.8  2002/10/14 12:10:30  mwe
 * al_expr-counter activated
 *
 * Revision 1.7  2002/10/10 09:25:06  mwe
 * debugging all functions
 * new functions added
 * seems to be the first correct working version
 *
 *
 *
 *----------------------------
 *revision 1.6
 *date: 2002/09/11 23:16:17;  author: dkr;  state: Exp;  lines: +2 -2
 *prf_node_info.mac modified
 *----------------------------
 *revision 1.5
 *date: 2002/08/25 14:23:12;  author: mwe;  state: Exp;  lines: +503 -69
 *elemination of warnings
 *changings in many functions
 *----------------------------
 *revision 1.4
 *date: 2002/07/21 21:46:40;  author: mwe;  state: Exp;  lines: +32 -16
 *editing ALassign
 *----------------------------
 *revision 1.3
 *date: 2002/07/21 15:56:44;  author: mwe;  state: Exp;  lines: +338 -28
 *editing CreateNAssignNodes and CommitNAssignNodes
 *----------------------------
 *revision 1.2
 *date: 2002/07/20 14:23:53;  author: mwe;  state: Exp;  lines: +427 -2
 *adding first functions
 *----------------------------
 *revision 1.1
 *date: 2002/06/07 17:38:35;  author: mwe;  state: Exp;
 *Initial revision
 *=============================================================================
 *
 */

/*****************************************************************************
 *
 *
 * Files: AssociativeLaw.c, AssociativeLaw.h
 * ******
 *
 * Prefix: AL
 * *******
 *
 * description:
 * ************
 *
 * AssociativeLaw is an algebraic optimization technique using ssa-form.
 *
 * AssociativeLaw is searching the code for separate constant nodes connected
 * by the same associative and commutative primitive operation. If separate
 * constant nodes are found, all these constant and variable nodes are
 * rearranged in that way, so that all constant nodes stand next together.
 * So these constants can be summarized by constant folding.
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
 * associative and commutative primitive operations:
 *   - F_add_SxS
 *   - F_mul_SxS
 *   - F_max
 *   - F_min
 *   - F_and
 *   - F_or
 *
 *
 * order of one optimization cycle:
 * ********************************
 *
 * The optimization AL starts for all fundef nodes. Then the recusive traversal
 * through the block node down to the last assign-node - the return node starts.
 * Then the return node is ignored and the optimization of the last assign-node
 * starts. First then primitive operation is checked. Is the primitive associative
 * and commutative the function 'travElems' traverse the arguments (both
 * exprs nodes) by recursion. The recursion terminates and add the termination
 * nodes to a nodelist when:
 *   i. the next primitive is not the same
 *  ii. the argument is a constant value
 * iii. the argument is defined outside the current block
 *
 * There are seperate nodelists for constant nodes and non-constant nodes. In the
 * nodelist are only pointer to the nodes. The nodes stay unchanged.
 * Only if more than one constant node and at least one variable node are collected,
 * it is neccessary to proceed the optimization.
 * First all collected nodes are duplicated. Then first all constant nodes get
 * compendious to assign nodes and then all variable nodes.
 * These assign nodes also get compendious until only one assign node remain.
 * This node replace the assign-node, the optimization was running for.
 * All new assign nodes are collected in a nodelist, which is stored in arg_info.
 * After terminatin of the current recursion-step of assign node-traversal, these
 * new assign nodes are included in the AST.
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
 *   a1 = 1 + arg1;
 *   a2 = 3 + a1;
 *   m1 = 2 * arg1;
 *   m2 = 4 * a2;
 *   m3 = m1 * m2;
 *
 *-------------------------------------------------------------------
 * initialization:
 *
 *   unused nodes: m3, m2, m1, a2, a1
 *
 *-------------------------------------------------------------------
 * 1. optimization step:   ** optimizing m3 **
 *
 *   CONSTANTLIST: 2, 4
 *   VARIABLELIST: arg1, a2
 *
 *   used nodes: m2, m1
 *
 *   al1 = 2 * 4;
 *   al2 = a2 * arg1;
 *   m3 = al1 * al2;
 *
 *  ==> unused nodes: a2, a1
 *
 *-------------------------------------------------------------------
 * 2. optimization step:   ** optimizing a2 **
 *
 *   CONSTANTLIST: 1, 3
 *   VARIABLELIST: arg1
 *
 *   used nodes: a2
 *
 *   al3 = 1 + 3;
 *   a2 = al3 + arg1;
 *
 *-------------------------------------------------------------------
 * result:
 *
 *   a1 = 1 + arg1;
 *   al3 = 1 + 3;
 *   a2 = al3 + arg1;
 *   m1 = 2 * a1;
 *   m2 = 4 * a2;
 *   al1 = 2 * 4;
 *   al2 = a2 * arg1;
 *   m3 = 8 * al2;
 *
 *  ==> after execution of ConstantFolding and DeadCodeRemoval:
 *
 *   a2 = 4 + arg1;
 *   al2 = a2 * arg1;
 *   m3 = 8 * al2;
 *
 *------------------------------------------------------------------
 *------------------------------------------------------------------
 *
 *  !!! IMPORTANT !!!
 * AssociativeLaw could cause much deadcode. So it is important to
 * run DeadCodeRemoval subsequent to AssociativeLaw. So following optimization
 * techniques aren't working on dead code and the memory usage does not
 * explode.
 *
 *
 * usage of arg_info and accessmacros:
 * ***********************************
 *
 *  -info2:      CONSTANTLIST
 *                  nodelist* used to store unused constant nodes
 *  -info3:      VARIABLELIST
 *                  nodelist* used to store unused variable nodes
 *  -dfmask[2]:  OPTCONSTANTLIST
 *                  nodelist* used to store edited nodes from CONSTANTLIST
 *  -dfmask[1]:  OPTVARIABLELIST
 *                  nodelist* used to store edited nodes from VARIABLELIST
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
 *  -node[2]:    CURRENTASSIGN
 *                  used while traversal through definitions of the
 *                  arguments of exprs-nodes. Store pointer to the
 *                  current assign node. If current exprs nodes
 *                  could be used in optimization process,
 *                  the assign node is marked as used.
 *
 *
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

#include "AssociativeLaw.h"

/*****************************************************************************
 *
 * function:
 *   node *AssociativeLaw(node *arg_node)
 *
 * description:
 *   is called from optimize.c
 *
 *****************************************************************************/

node *
AssociativeLaw (node *arg_node, node *a)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("AssociativeLaw");

    if (arg_node != NULL) {
        arg_info = MakeInfo ();

        old_tab = act_tab;
        act_tab = al_tab;

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
ALblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ALblock");

    if (BLOCK_INSTR (arg_node) != NULL) {
        /*
         * store pointer on actual N_block-node for append of new N_vardec nodes
         */
        if (BLOCK_VARDEC (arg_node) != NULL) {
            INFO_AL_TYPE (arg_info) = VARDEC_TYPE (BLOCK_VARDEC (arg_node));
            INFO_AL_BLOCKNODE (arg_info) = arg_node;
        }

        INFO_AL_OPTCONSTANTLIST (arg_info) = NULL;
        INFO_AL_OPTVARIABLELIST (arg_info) = NULL;

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
ALassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ALassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {

        ASSIGN_STATUS (arg_node) = 1;

        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);

        if (ContainOptInformation (arg_info) == 1) {

            nodelist *nodelist1;
            node *old_succ, *akt_nassign;

            /*
             * increase optimization counter
             * include new N_assign nodes from arg_info behind actual N_assign node
             * modify old successor
             * reset arg_info nodes
             */

            al_expr++;

            /*
             * store next N_assign node
             */
            old_succ = ASSIGN_NEXT (arg_node);

            /*
             * pointer on the last included N_assign node
             */
            akt_nassign = arg_node;

            /*
             * include N_assign-nodes created from constant nodes
             */
            nodelist1 = INFO_AL_OPTCONSTANTLIST (arg_info);

            if (nodelist1 != NULL) {
                while (NODELIST_NEXT (nodelist1) != NULL) {
                    ASSIGN_NEXT (akt_nassign) = NODELIST_NODE (nodelist1);
                    akt_nassign = ASSIGN_NEXT (akt_nassign);
                    nodelist1 = NODELIST_NEXT (nodelist1);
                }
                ASSIGN_NEXT (akt_nassign) = NODELIST_NODE (nodelist1);
                akt_nassign = ASSIGN_NEXT (akt_nassign);
            }

            /*
             * include N_assign-nodes created from 'variable' nodes
             */
            nodelist1 = INFO_AL_OPTVARIABLELIST (arg_info);

            while (NODELIST_NEXT (nodelist1) != NULL) {
                ASSIGN_NEXT (akt_nassign) = NODELIST_NODE (nodelist1);
                akt_nassign = ASSIGN_NEXT (akt_nassign);
                nodelist1 = NODELIST_NEXT (nodelist1);
            }

            ASSIGN_NEXT (akt_nassign) = NODELIST_NODE (nodelist1);
            akt_nassign = ASSIGN_NEXT (akt_nassign);

            /*
             * connect last included element with stored N_assign node
             */
            ASSIGN_NEXT (akt_nassign) = old_succ;

            /*
             * reset nodelists in arg_info
             */

            (INFO_AL_OPTVARIABLELIST (arg_info))
              = RemoveNodelistNodes (INFO_AL_OPTVARIABLELIST (arg_info));
            (INFO_AL_OPTCONSTANTLIST (arg_info))
              = RemoveNodelistNodes (INFO_AL_OPTCONSTANTLIST (arg_info));

            FreeNodelist (INFO_AL_OPTVARIABLELIST (arg_info));
            FreeNodelist (INFO_AL_OPTCONSTANTLIST (arg_info));
            (INFO_AL_CONSTANTLIST (arg_info)) = NULL;
            (INFO_AL_VARIABLELIST (arg_info)) = NULL;
            INFO_AL_OPTCONSTANTLIST (arg_info) = NULL;
            (INFO_AL_OPTVARIABLELIST (arg_info)) = NULL;
            INFO_AL_NUMBEROFVARIABLES (arg_info) = 0;
            INFO_AL_NUMBEROFCONSTANTS (arg_info) = 0;
        }
    }

    /*
     * traverse in N_let-node
     */

    if ((ASSIGN_INSTR (arg_node) != NULL) && (ASSIGN_STATUS (arg_node) == 1)) {

        INFO_AL_CURRENTASSIGN (arg_info) = arg_node;

        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
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
ALlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ALlet");
    if (LET_EXPR (arg_node) != NULL) {
        INFO_AL_LETNODE (arg_info) = arg_node;
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
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
ALprf (node *arg_node, node *arg_info)
{
    int anz_const;
    int anz_all;
    nodetype nodetype;

    DBUG_ENTER ("AssociativeLawOptimize");

    if (NODE_TYPE (PRF_ARGS (arg_node)) == N_exprs) {
        if (IsAssociativeAndCommutative (arg_node) == 1) {

            INFO_AL_NUMBEROFVARIABLES (arg_info) = 0;
            INFO_AL_NUMBEROFCONSTANTS (arg_info) = 0;
            INFO_AL_CONSTANTLIST (arg_info) = NULL;
            INFO_AL_VARIABLELIST (arg_info) = NULL;
            INFO_AL_CURRENTPRF (arg_info) = PRF_PRF (arg_node);

            /*
             * Traverse through the definitions of arg_node and collect nodes
             */

            arg_info = TravElems (PRF_ARGS (arg_node), arg_info);
            arg_info = TravElems (EXPRS_NEXT (PRF_ARGS (arg_node)), arg_info);

            anz_const = INFO_AL_NUMBEROFCONSTANTS (arg_info);
            anz_all = INFO_AL_NUMBEROFVARIABLES (arg_info) + anz_const;

            /*
             * neccessary to optimize?
             *
             * (anz_const < anz_all): important, because if no variable node is
             * available, the 'CommitNAssignNodes' will crash. If only constant
             * nodes are available, so 'constant folding' is deactivated and this
             * optimization make no sense
             */
            if ((anz_const > 1) && (anz_all > 2) && (anz_const < anz_all)) {

                nodetype = NODE_TYPE (NODELIST_NODE (INFO_AL_CONSTANTLIST (arg_info)));
                if (!(enforce_ieee)
                    || ((nodetype != N_float) && (nodetype != N_double))) {

                    /*
                     * start optimization
                     */
                    CreateNAssignNodes (arg_info);

                    anz_const = INFO_AL_NUMBEROFCONSTANTS (arg_info);

                    if (anz_const > 0) {
                        CommitNAssignNodes (arg_info);
                    } else {
                        INFO_AL_NUMBEROFVARIABLES (arg_info) = 2;
                        INFO_AL_NUMBEROFCONSTANTS (arg_info) = 2;
                    }
                } else {
                    /*
                     * nothing to optimize
                     */
                    FreeNodelist (INFO_AL_CONSTANTLIST (arg_info));
                    FreeNodelist (INFO_AL_VARIABLELIST (arg_info));
                    INFO_AL_NUMBEROFVARIABLES (arg_info) = 0;
                    INFO_AL_NUMBEROFCONSTANTS (arg_info) = 0;
                }
            } else {
                /*
                 * nothing to optimize
                 */
                FreeNodelist (INFO_AL_CONSTANTLIST (arg_info));
                FreeNodelist (INFO_AL_VARIABLELIST (arg_info));
                INFO_AL_NUMBEROFVARIABLES (arg_info) = 0;
                INFO_AL_NUMBEROFCONSTANTS (arg_info) = 0;
            }
        }
    }
    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   IsAssociativeAndCommutative(node *arg_node)
 *
 * description:
 *   returns '1' if the primitive operation is associative and commutative
 *
 ****************************************************************************/

int
IsAssociativeAndCommutative (node *arg_node)
{

    int return_bool;
    DBUG_ENTER ("IsAssociativeAndCommutative");
    switch (PRF_PRF (arg_node)) {
    case F_add_SxS:
    case F_mul_SxS:
    case F_max:
    case F_min:
    case F_and:
    case F_or:
        return_bool = 1;
        break;

    default:
        return_bool = 0;
    }

    DBUG_RETURN (return_bool);
}

/*****************************************************************************
 *
 * function:
 *   TravElems(node *arg_node, node *arg_info)
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
TravElems (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TravElems");

    if (IsConstant (EXPRS_EXPR (arg_node))) {

        arg_info = AddNode (arg_node, arg_info, 1);

    } else {

        if (OtherPrfOp (arg_node, arg_info) || ReachedArgument (EXPRS_EXPR (arg_node))
            || ReachedDefinition (EXPRS_EXPR (arg_node))) {

            arg_info = AddNode (arg_node, arg_info, 0);

        } else {

            ASSIGN_STATUS (INFO_AL_CURRENTASSIGN (arg_info)) = 0;
            INFO_AL_CURRENTASSIGN (arg_info)
              = AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node)));

            arg_info = TravElems (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                                    AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node)))))),
                                  arg_info);
            arg_info = TravElems (EXPRS_NEXT (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                                    AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node))))))),
                                  arg_info);
        }
    }

    DBUG_RETURN (arg_info);
}

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

int
ReachedArgument (node *arg_node)
{

    int return_bool;
    DBUG_ENTER ("ReachedArgument");

    if (NODE_TYPE (arg_node) == N_id) {
        if (AVIS_SSAASSIGN (ID_AVIS (arg_node)) == NULL)
            return_bool = 1;
        else
            return_bool = 0;
    } else
        return_bool = 0;

    DBUG_RETURN (return_bool);
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

int
ReachedDefinition (node *arg_node)
{

    int return_bool;
    DBUG_ENTER ("ReachedDefinition");

    if (NODE_TYPE (arg_node) == N_id) {
        if (AVIS_SSAASSIGN (ID_AVIS (arg_node)) == NULL)
            return_bool = 0;
        else if (NODE_TYPE (LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (arg_node)))))
                 != N_prf)
            return_bool = 1;
        else
            return_bool = 0;
    } else
        return_bool = 0;

    DBUG_RETURN (return_bool);
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

int
IsConstant (node *arg_node)
{

    int return_bool;
    DBUG_ENTER ("IsConstant");

    switch (NODE_TYPE (arg_node)) {
    case N_num:
    case N_double:
    case N_float:
    case N_bool:
    case N_char:
        return_bool = 1;
        break;

    default:
        return_bool = 0;
    }

    DBUG_RETURN (return_bool);
}

/*****************************************************************************
 *
 * function:
 *   node *AddNode(node *arg_node, node *arg_info, int constant)
 *
 * description:
 *   Add the arg_node to the node-list in arg_info
 *   If the int-argument is '1', the arg_node contains a constant value
 *   If the int-argument is '0', the arg_node contains no constant value
 *   Increase the corresponding counter
 *   arg_node is added to the corresponding list
 *
 ****************************************************************************/

node *
AddNode (node *arg_node, node *arg_info, int constant)
{
    nodelist *newnodelistnode = MakeNodelistNode (EXPRS_EXPR (arg_node), NULL);

    DBUG_ENTER ("AddNode");

    if (constant == 1) {
        INFO_AL_NUMBEROFCONSTANTS (arg_info) = INFO_AL_NUMBEROFCONSTANTS (arg_info) + 1;
    }

    else {
        INFO_AL_NUMBEROFVARIABLES (arg_info) = INFO_AL_NUMBEROFVARIABLES (arg_info) + 1;
    }

    if (constant == 1) {
        NODELIST_NEXT (newnodelistnode) = (INFO_AL_CONSTANTLIST (arg_info));
        INFO_AL_CONSTANTLIST (arg_info) = newnodelistnode;
    } else {
        NODELIST_NEXT (newnodelistnode) = ((nodelist *)(INFO_AL_VARIABLELIST (arg_info)));
        INFO_AL_VARIABLELIST (arg_info) = newnodelistnode;
    }

    DBUG_RETURN (arg_info);
}

/*****************************************************************************
 *
 * function:
 *   int OtherPrfOp(node *arg_node, node *arg_info)
 *
 * description:
 *   Thje arg_node is an exprs-node
 *   Returns '1' if the argument of the exprs-node is an id-node and the
 *   used primitive operation in its definition is not the same as saved
 *   in arg_info
 *
 ****************************************************************************/

int
OtherPrfOp (node *arg_node, node *arg_info)
{
    int otherOp;
    prf otherPrf;
    DBUG_ENTER ("OtherPrfOp");
    if (NODE_TYPE (EXPRS_EXPR (arg_node)) == N_id) {
        if (AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node))) == NULL)
            otherOp = 0;
        else {
            otherPrf = PRF_PRF (
              LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node))))));

            if (INFO_AL_CURRENTPRF (arg_info) == otherPrf)
                otherOp = 0;
            else
                otherOp = 1;
        }
    } else
        otherOp = 0;

    DBUG_RETURN (otherOp);
}

/*****************************************************************************
 *
 * function:
 *   CreateNAssignNodes(node *arg_info)
 *
 * description:
 *   Replace all nodes in CONSTANTLIST by copies and imbed them in exprs-nodes
 *   Create assign-nodes from two adjacent exprs-nodes
 *   If an odd number of nodes is in list, create with last created
 *   assign-node and last exprs-node new assign-node (and store
 *   'old' assign_node in OPTCONSTANTLIST to avoid double usage
 *
 *   The same procedure for nodes stored in VARIABLELIST
 *   additional:
 *     If there is only one variable node in list it is neccessary
 *     to check how much constant nodes are in list:
 *       - remain only one assign node: create new prf-node with
 *           the both last nodes as arguments and set it behind stored
 *           let node and stop this optimization step
 *       - remain more than one assign node: create new assign node
 *           with sole variable expr-node and last 'constant' assign-node
 *           and store new assign node in VARIABLELIST instead of expr-node
 *
 *   Remove NODELIST_NODES with NULL-pointer as arguments
 *
 ****************************************************************************/

node *
CreateNAssignNodes (node *arg_info)
{
    int count;
    node *listElem, *newnode;
    nodelist *currentElem, *secElem;

    DBUG_ENTER ("CreateNAssignNodes");

    /*
     * create assign nodes with node of CONSTANTLIST
     */

    currentElem = INFO_AL_CONSTANTLIST (arg_info); /*Pointer on first list element*/
    secElem = NULL;

    currentElem = CreateExprsNodes (currentElem, INFO_AL_NUMBEROFCONSTANTS (arg_info));

    currentElem = INFO_AL_CONSTANTLIST (arg_info); /*pointer on first list element*/
    for (count = 0; count < (div (INFO_AL_NUMBEROFCONSTANTS (arg_info), 2).quot);
         count++) {
        currentElem = CreateAssignNodesFromExprsNodes (currentElem, arg_info);
        currentElem = NODELIST_NEXT (currentElem);
    }

    /*
     * If CONSTANTLIST has an odd number of list elements:
     */
    if (div (INFO_AL_NUMBEROFCONSTANTS (arg_info), 2).rem == 1) {

        currentElem = INFO_AL_CONSTANTLIST (arg_info); /*Pointer on first list element*/
        while (NODELIST_NEXT (currentElem) != NULL) {
            secElem = currentElem;
            currentElem = NODELIST_NEXT (currentElem);
        }

        newnode = CreateAssignNodeFromAssignAndExprsNode (currentElem, secElem, arg_info);

        (INFO_AL_OPTCONSTANTLIST (arg_info))
          = MakeNodelistNode (NODELIST_NODE (secElem), NULL);
        NODELIST_NEXT ((INFO_AL_OPTCONSTANTLIST (arg_info))) = NULL;

        FreeNodelist (currentElem);

        NODELIST_NODE (secElem) = newnode; /* store new node */
        NODELIST_NEXT (secElem) = NULL;    /* remove old N_expr-node */
    }

    /*
     * constant nodes ready
     * create assign nodes with nodes of VARIABLELIST
     */

    currentElem = INFO_AL_VARIABLELIST (arg_info); /*Pointer on first list element*/
    currentElem = CreateExprsNodes (currentElem, INFO_AL_NUMBEROFVARIABLES (arg_info));

    currentElem = INFO_AL_VARIABLELIST (arg_info); /*Pointer on first list element*/

    /*
     * check number of list elements
     */

    if ((INFO_AL_NUMBEROFVARIABLES (arg_info) == 1)
        && (INFO_AL_NUMBEROFCONSTANTS (arg_info) <= 3)) {

        secElem
          = INFO_AL_CONSTANTLIST (arg_info); /*Pointer on first constant list element*/
        while (NODELIST_NEXT (secElem) != NULL)
            secElem = NODELIST_NEXT (secElem);

        newnode = MakeExprsNodeFromExprsAndAssignNode (secElem, currentElem);

        listElem = PRF_ARGS (LET_EXPR (INFO_AL_LETNODE (arg_info)));

        /* free listElem */

        INFO_AL_OPTVARIABLELIST (arg_info)
          = MakeNodelistNode (NODELIST_NODE (secElem), NULL);
        NODELIST_NEXT ((INFO_AL_OPTVARIABLELIST (arg_info))) = NULL;

        PRF_ARGS (LET_EXPR (INFO_AL_LETNODE (arg_info))) = newnode;
        /* prohibits functioncall of 'CommitNassignNodes()' */
        INFO_AL_NUMBEROFCONSTANTS (arg_info) = 0;
        INFO_AL_NUMBEROFVARIABLES (arg_info) = 0;
    } else {

        if (INFO_AL_NUMBEROFVARIABLES (arg_info) == 1) {

            secElem = INFO_AL_CONSTANTLIST (
              arg_info); /*Pointer on first constant list element*/
            while (NODELIST_NEXT (secElem) != NULL)
                secElem = NODELIST_NEXT (secElem);

            newnode = MakeExprsNodeFromExprsAndAssignNode (secElem, currentElem);
            newnode = MakeAssignNodeFromExprsNode (newnode, arg_info);

            if (INFO_AL_OPTCONSTANTLIST (arg_info) != NULL) {
                NODELIST_NEXT ((INFO_AL_OPTCONSTANTLIST (arg_info)))
                  = MakeNodelistNode (NODELIST_NODE (secElem), NULL);
                NODELIST_NEXT (NODELIST_NEXT ((INFO_AL_OPTCONSTANTLIST (arg_info))))
                  = NULL;
            } else {
                (INFO_AL_OPTCONSTANTLIST (arg_info))
                  = MakeNodelistNode (NODELIST_NODE (secElem), NULL);
                ;
                NODELIST_NEXT ((INFO_AL_OPTCONSTANTLIST (arg_info))) = NULL;
            }
            NODELIST_NODE (secElem) = NULL; /* store new node */ /* vorher currentElem */
            /*NODELIST_NEXT(secElem) = NULL;*/ /* remove old N_expr-node */
            NODELIST_NODE (currentElem) = newnode;
        } else {

            for (count = 0; count < (div (INFO_AL_NUMBEROFVARIABLES (arg_info), 2).quot);
                 count++) {
                currentElem = CreateAssignNodesFromExprsNodes (currentElem, arg_info);
                currentElem = NODELIST_NEXT (currentElem);
            }
            /*
             * If VARIABLELIST has odd number of list elements
             */
            if (div (INFO_AL_NUMBEROFVARIABLES (arg_info), 2).rem == 1) {

                currentElem
                  = INFO_AL_VARIABLELIST (arg_info); /*Pointer on first list element*/
                while (NODELIST_NEXT (currentElem) != NULL) {
                    secElem = currentElem;
                    currentElem = NODELIST_NEXT (currentElem);
                }

                newnode = CreateAssignNodeFromAssignAndExprsNode (currentElem, secElem,
                                                                  arg_info);

                (INFO_AL_OPTVARIABLELIST (arg_info))
                  = MakeNodelistNode (NODELIST_NODE (secElem), NULL);
                NODELIST_NEXT ((INFO_AL_OPTVARIABLELIST (arg_info))) = NULL;

                FreeNodelist (currentElem);

                NODELIST_NODE (secElem) = newnode; /* store new node */
                NODELIST_NEXT (secElem) = NULL;    /* remove old N_expr-node */
            }
        }
    }

    /*
     *  remove NULL-pointer-nodes
     */

    INFO_AL_CONSTANTLIST (arg_info)
      = RemoveNullpointerNodes ((INFO_AL_CONSTANTLIST (arg_info)));

    INFO_AL_VARIABLELIST (arg_info)
      = RemoveNullpointerNodes ((INFO_AL_VARIABLELIST (arg_info)));

    DBUG_RETURN (arg_info);
}

/*****************************************************************************
 *
 * function:
 *   node *CreateAssignNodeFromAssignAndExprsNode(nodelist *currentElem,
 *                                       nodelist *secElem, node* arg_info)
 *
 * description:
 *    This function creates a new assign-node with another assign-node and
 *    an exprs-node as an argument.
 *
 ****************************************************************************/

node *
CreateAssignNodeFromAssignAndExprsNode (nodelist *currentElem, nodelist *secElem,
                                        node *arg_info)
{

    node *newnode;

    DBUG_ENTER ("CreateAssignNodeFromAssignAndExprsNode");

    newnode = MakeExprsNodeFromExprsAndAssignNode (secElem, currentElem);
    newnode = MakeAssignNodeFromExprsNode (newnode, arg_info);

    DBUG_RETURN (newnode);
}

/*****************************************************************************
 *
 * function:
 *   node *CreateExprsNode(nodelist *currentElem, int number_of_list_elements)
 *
 * description:
 *   aktelem contains 'number_of_list_elements' list elements.
 *   These list elements are constant nodes or id nodes.
 *   The list elements will be duplicated and the new nodes will replace the
 *   original nodes.
 *
 ****************************************************************************/

nodelist *
CreateExprsNodes (nodelist *currentElem, int number_of_list_elements)
{
    int count;
    node *listElem;

    DBUG_ENTER ("CreateExprsNodes");

    for (count = 0; count < number_of_list_elements; count++) {
        listElem
          = NODELIST_NODE (currentElem); /*pointer on actual node stored in nodelist*/
        listElem = DupTree (listElem);   /*pointer on duplicated node */
        listElem = MakeExprs (listElem, NULL); /* create N_expr-node */
        NODELIST_NODE (currentElem) = listElem;
        currentElem = NODELIST_NEXT (currentElem);
    }
    /* all elements of the nodelist are N_expr nodes*/

    DBUG_RETURN (currentElem);
}

/*****************************************************************************
 *
 * function:
 *   nodelist *CreateAssignNodesFromExprsNodes( nodelist *currentElem,
 *                                              node *arg_info )
 *
 * description:
 *   The nodelist currentElem contains exprs-nodes. The first and the second
 *   node in list will be connected to one assign-node.
 *   After that, the two exprs-nodes will be replaced by the new
 *   assign node.
 *
 ****************************************************************************/

nodelist *
CreateAssignNodesFromExprsNodes (nodelist *currentElem, node *arg_info)
{

    nodelist *secElem;
    node *listElem, *listElem2;

    DBUG_ENTER ("CreateAssignNodesFromExprsNodes");

    secElem = NODELIST_NEXT (currentElem); /*pointer on next list element*/
    listElem = NODELIST_NODE (currentElem);
    listElem2 = NODELIST_NODE (secElem);

    EXPRS_NEXT (listElem) = listElem2; /*concatenate first with second N_expr-node*/
    listElem = MakeAssignNodeFromExprsNode (listElem, arg_info);

    NODELIST_NODE (currentElem) = listElem;
    NODELIST_NEXT (currentElem) = NODELIST_NEXT (secElem); /* remove second node*/
    secElem = currentElem; /* store pointer on last element */

    DBUG_RETURN (currentElem);
}

/*****************************************************************************
 *
 * function:
 *   CommitNAssignNodes(node *arg_info)
 *
 * description:
 *   When this function is called, all nodes in the VARIABLELIST and
 *   CONSTANTLIST are duplicated and processed to assign-nodes.
 *   Now, first the first and second assign-node in CONSTANTLIST get
 *   arguments of a new assign-node, which get the last element of
 *   CONSTANTLIST. The first and second element are removed from the list
 *   and added to the OPTCONSTANTLIST. This procedure is repeated until
 *   only one assign-node is in CONSTANTLIST.
 *   This node and the first node of the VARIABLELIST are handled the same
 *   way. Both nodes are removed from their lists and inserted in
 *   OPTVARIABLELIST.
 *   Now, this procedure works the same way for VARIABLELIST. The last both
 *   assign-nodes become arguments of an prf-node, which replace the
 *   old prf-node of the 'starting assign-node'.
 *
 ****************************************************************************/

node *
CommitNAssignNodes (node *arg_info)
{
    node *newnode, *elem1, *elem2;
    nodelist *lastListElem, *aktListElem, *constlist, *varlist;

    DBUG_ENTER ("CommitNAssignNodes");

    constlist = INFO_AL_OPTCONSTANTLIST (arg_info);
    if (constlist != NULL) {
        while (NODELIST_NEXT (constlist) != NULL)
            constlist = NODELIST_NEXT (constlist);
    }

    varlist = INFO_AL_OPTVARIABLELIST (arg_info);
    if (varlist != NULL) {
        while (NODELIST_NEXT (varlist) != NULL)
            varlist = NODELIST_NEXT (varlist);
    }

    aktListElem = INFO_AL_CONSTANTLIST (arg_info);
    lastListElem = aktListElem;

    while (NODELIST_NEXT (lastListElem) != NULL) {
        lastListElem = NODELIST_NEXT (lastListElem);
    }

    while (aktListElem != lastListElem) {
        elem1 = NODELIST_NODE (aktListElem);

        if (constlist != NULL) {
            NODELIST_NEXT (constlist) = aktListElem;
            constlist = NODELIST_NEXT (constlist);
        } else {
            INFO_AL_OPTCONSTANTLIST (arg_info) = aktListElem;
            constlist = INFO_AL_OPTCONSTANTLIST (arg_info);
        }

        aktListElem = NODELIST_NEXT (aktListElem);
        elem2 = NODELIST_NODE (aktListElem);

        NODELIST_NEXT (constlist) = aktListElem;
        constlist = NODELIST_NEXT (constlist);

        newnode = MakeExprsNodeFromAssignNodes (elem1, elem2);
        newnode = MakeAssignNodeFromExprsNode (newnode, arg_info);

        NODELIST_NEXT (lastListElem) = MakeNodelistNode (newnode, NULL);
        lastListElem = NODELIST_NEXT (lastListElem);
        aktListElem = NODELIST_NEXT (aktListElem);
    }

    /* constant nodes ready */

    /* connect last constant node with first variable node */

    elem1 = NODELIST_NODE (lastListElem);

    if (constlist != NULL) {
        NODELIST_NEXT (constlist) = lastListElem;
        constlist = NODELIST_NEXT (constlist);
    } else {
        INFO_AL_OPTCONSTANTLIST (arg_info) = lastListElem;
        constlist = INFO_AL_OPTCONSTANTLIST (arg_info);
    }
    NODELIST_NEXT (constlist) = NULL;

    aktListElem = INFO_AL_VARIABLELIST (arg_info);
    lastListElem = aktListElem;
    while (NODELIST_NEXT (lastListElem) != NULL) {
        lastListElem = NODELIST_NEXT (lastListElem);
    }

    if (aktListElem != lastListElem) {

        elem2 = NODELIST_NODE (aktListElem);

        if (varlist != NULL) {
            NODELIST_NEXT (varlist) = aktListElem;
            varlist = NODELIST_NEXT (varlist);
        } else {
            INFO_AL_OPTVARIABLELIST (arg_info) = aktListElem;
            varlist = INFO_AL_OPTVARIABLELIST (arg_info);
        }

        aktListElem = NODELIST_NEXT (aktListElem);

        newnode = MakeExprsNodeFromAssignNodes (elem1, elem2);
        newnode = MakeAssignNodeFromExprsNode (newnode, arg_info);

        NODELIST_NEXT (lastListElem) = MakeNodelistNode (newnode, NULL);
        lastListElem = NODELIST_NEXT (lastListElem);

        while (NODELIST_NEXT (aktListElem) != lastListElem) {
            elem1 = NODELIST_NODE (aktListElem);

            NODELIST_NEXT (varlist) = aktListElem;
            varlist = NODELIST_NEXT (varlist);

            aktListElem = NODELIST_NEXT (aktListElem);
            elem2 = NODELIST_NODE (aktListElem);

            NODELIST_NEXT (varlist) = aktListElem;
            varlist = NODELIST_NEXT (varlist);

            newnode = MakeExprsNodeFromAssignNodes (elem1, elem2);
            newnode = MakeAssignNodeFromExprsNode (newnode, arg_info);

            NODELIST_NEXT (lastListElem) = MakeNodelistNode (newnode, NULL);
            lastListElem = NODELIST_NEXT (lastListElem);

            aktListElem = NODELIST_NEXT (aktListElem);
        }

        /* variable nodes ready  */

        /* connect last two N_assign nodes of variable list */

        elem1 = NODELIST_NODE (aktListElem);
        elem2 = NODELIST_NODE (lastListElem);

        NODELIST_NEXT (varlist) = aktListElem;
        varlist = NODELIST_NEXT (varlist);
        NODELIST_NEXT (varlist) = lastListElem;
        varlist = NODELIST_NEXT (varlist);
        NODELIST_NEXT (varlist) = NULL;

        newnode = MakeExprsNodeFromAssignNodes (elem1, elem2);

        elem1 = PRF_ARGS (LET_EXPR (INFO_AL_LETNODE (arg_info)));

        FreeNode (elem1);

        PRF_ARGS (LET_EXPR (INFO_AL_LETNODE (arg_info))) = newnode;

    } else {

        elem2 = NODELIST_NODE (lastListElem);

        if (varlist != NULL)
            NODELIST_NEXT (varlist) = lastListElem;
        else {
            INFO_AL_OPTVARIABLELIST (arg_info) = lastListElem;
            varlist = INFO_AL_OPTVARIABLELIST (arg_info);
        }

        if (NODELIST_NEXT (varlist) != NULL) {
            varlist = NODELIST_NEXT (varlist);
        }
        NODELIST_NEXT (varlist) = aktListElem;
        varlist = NODELIST_NEXT (varlist);
        NODELIST_NEXT (varlist) = NULL;

        newnode = MakeExprsNodeFromAssignNodes (elem1, elem2);

        elem1 = PRF_ARGS (LET_EXPR (INFO_AL_LETNODE (arg_info)));

        FreeNode (elem1);

        PRF_ARGS (LET_EXPR (INFO_AL_LETNODE (arg_info))) = newnode;
    }
    NODELIST_NEXT (varlist) = NULL;

    DBUG_RETURN (arg_info);
}

/*****************************************************************************
 *
 * function:
 *   ContainOptInformation(node *arg_info)
 *
 * description:
 *   returns 1 (true) if the arg_info-node contains N_assign-nodes to include
 *   in the tree
 *
 ****************************************************************************/

int
ContainOptInformation (node *arg_info)
{
    DBUG_ENTER ("ContainOptInformation");

    if ((INFO_AL_NUMBEROFCONSTANTS (arg_info) > 1)
        && (INFO_AL_NUMBEROFCONSTANTS (arg_info) + INFO_AL_NUMBEROFVARIABLES (arg_info)
            > 3))
        DBUG_RETURN (1);
    else
        DBUG_RETURN (0);
}

/*****************************************************************************
 *
 * function:
 *   nodelist *RemoveNodelistNodes( nodelist* current_list )
 *
 * description:
 *   set all NODELIST_NODE to NULL
 *   These function remove all connected nodes. This circumvent deleting
 *   of the NODELIST_NODE-elements when FREE_NODELIST is executed.
 *
 ****************************************************************************/

nodelist *
RemoveNodelistNodes (nodelist *current_list)
{

    nodelist *temp = current_list;
    DBUG_ENTER ("RemoveNodelistNodes");

    if (temp != NULL) {
        while (NODELIST_NEXT (temp) != NULL) {
            NODELIST_NODE (temp) = NULL;
            temp = NODELIST_NEXT (temp);
        }
        NODELIST_NODE (temp) = NULL;
    }

    DBUG_RETURN (current_list);
}

/*****************************************************************************
 *
 * function:
 *   nodelist *RemoveNullpointerNodes( nodelist *source  )
 *
 * description:
 *   While the optimization process it is possible, that some NULL-Pointer
 *   in the NODELIST_NODE-pointer can accure. This can cause exceptions,
 *   so these NULL-Pointer-elements have to be removed.
 *   The following function do so.
 *
 ****************************************************************************/

nodelist *
RemoveNullpointerNodes (nodelist *source)
{

    nodelist *currentElem, *secElem;

    DBUG_ENTER ("RemoveNullpointerNodes");

    currentElem = source; /*Pointer on first list element*/
    ;
    secElem = NULL;
    while ((currentElem != NULL) && (NODELIST_NEXT (currentElem) != NULL)) {
        if (NODELIST_NODE (currentElem) == NULL) {
            if (secElem != NULL) {
                NODELIST_NEXT (secElem) = NODELIST_NEXT (currentElem);
                /*
                 * remove currentElem
                 */

                NODELIST_NEXT (currentElem) = NULL;
                FreeNodelist (currentElem);

                currentElem = secElem;
            } else {
                if (NODELIST_NEXT (currentElem) != NULL) {

                    secElem = currentElem;

                    source = NODELIST_NEXT (currentElem);

                    NODELIST_NEXT (secElem) = NULL;
                    FreeNodelist (secElem);
                    secElem = NULL;
                    currentElem = source;
                } else {
                    FreeNodelist (currentElem);
                    source = NULL;
                    break;
                }
            }
        }
        secElem = currentElem;
        currentElem = NODELIST_NEXT (currentElem);
    }

    if ((currentElem != NULL) && (NODELIST_NODE (currentElem) == NULL)) {
        FreeNodelist (currentElem);
        if (secElem != NULL)
            NODELIST_NEXT (secElem) = NULL;
    }

    DBUG_RETURN (source);
}

/*****************************************************************************
 *
 * function:
 *   node *MakeExprsNodeFromExprsAndAssignNode(nodelist *assignnode,
 *                                             nodelist *exprsnode )
 *
 * description:
 *   One NODELIST_NODE contain an assign-node and the other an exprs-node.
 *   The assign-node becomes the argument of a new id node.
 *   This id node and the exprs node are connected to a new exprs-node.
 *
 ****************************************************************************/

node *
MakeExprsNodeFromExprsAndAssignNode (nodelist *assignnode, nodelist *exprsnode)
{

    node *listElem, *listElem2, *newnode;
    char *newname1, *newname2, *mod, *mod2;
    statustype status;

    DBUG_ENTER ("MakeExprsNodeFromExprsAndAssignNode");

    listElem = NODELIST_NODE (assignnode); /* pointer on last constant N_assign-node */
    listElem2 = NODELIST_NODE (exprsnode); /* pointer on node in list */

    newname2 = NULL;
    newname1 = IDS_NAME (LET_IDS (ASSIGN_INSTR (listElem)));
    DBUG_ASSERT ((newname1 != NULL),
                 "Unexpected error, existing variable has no name: IDS_NAME == NULL");
    newname2 = StringCopy (newname1);
    mod = IDS_MOD (LET_IDS (ASSIGN_INSTR (listElem)));

    if (mod != NULL)
        mod2 = StringCopy (mod);
    else
        mod2 = NULL;

    status = IDS_ATTRIB (LET_IDS (ASSIGN_INSTR (listElem)));

    newnode = MakeId (newname2, mod2, status);
    newnode = MakeExprs (newnode, listElem2); /* create new N_Expr-node */

    ID_AVIS (EXPRS_EXPR (newnode)) = IDS_AVIS (LET_IDS (ASSIGN_INSTR (listElem)));

    ID_VARDEC (EXPRS_EXPR (newnode)) = IDS_VARDEC (LET_IDS (ASSIGN_INSTR (listElem)));

    DBUG_RETURN (newnode);
}

/*****************************************************************************
 *
 * function:
 *   node *MakeAssignNodeFromExprsNode( node *newnode , node *arg_info)
 *
 * description:
 *   This function create a new assign-node with the exprs-node as an
 *   argument. The correct primitive and vardec-root-node are provided
 *   by the arg_info-node.
 *
 ****************************************************************************/

node *
MakeAssignNodeFromExprsNode (node *newnode, node *arg_info)
{

    node *newvardec;
    types *type;
    char *newname1, *newname2;

    DBUG_ENTER ("MakeAssignNodeFromExprsNode");
    newnode = MakePrf (INFO_AL_CURRENTPRF (arg_info), newnode);

    type = MakeTypes (TYPES_BASETYPE ((INFO_AL_TYPE (arg_info))),
                      TYPES_DIM ((INFO_AL_TYPE (arg_info))), NULL, NULL, NULL);

    newname1 = TmpVar ();

    newvardec
      = MakeVardec (newname1, type, (BLOCK_VARDEC (INFO_AL_BLOCKNODE (arg_info))));

    BLOCK_VARDEC (INFO_AL_BLOCKNODE (arg_info)) = newvardec;

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
 *   node *MakeExprsNodeFromAssignNodes(node *elem1, node *elem2)
 *
 * description:
 *   Both assign-nodes get arguments of new id-nodes. These id-nodes
 *   get arguments of a new exprs-node.
 *
 ****************************************************************************/

node *
MakeExprsNodeFromAssignNodes (node *elem1, node *elem2)
{

    node *newnode;
    statustype status1, status2;
    char *varname1, *newname1, *varmod1, *newmod1, *varname2, *newname2, *varmod2,
      *newmod2;

    DBUG_ENTER ("MakeExprsNodeFromAssignNodes");

    newname1 = NULL;
    newname2 = NULL;
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

    varname2 = IDS_NAME (LET_IDS (ASSIGN_INSTR (elem2)));
    DBUG_ASSERT ((varname2 != NULL),
                 "Unexpected error, existing variable has no name: IDS_NAME == NULL");
    newname2 = StringCopy (varname2);
    varmod2 = IDS_MOD (LET_IDS (ASSIGN_INSTR (elem2)));
    if (varmod2 != NULL)
        newmod2 = StringCopy (varmod2);
    else
        newmod2 = NULL;
    status2 = IDS_ATTRIB (LET_IDS (ASSIGN_INSTR (elem2)));

    newnode = MakeExprs (MakeId (newname1, newmod1, status1),
                         MakeExprs (MakeId (newname2, newmod2, status2), NULL));

    ID_AVIS (EXPRS_EXPR (newnode)) = IDS_AVIS (LET_IDS (ASSIGN_INSTR (elem1)));
    ID_AVIS (EXPRS_EXPR (EXPRS_NEXT (newnode)))
      = IDS_AVIS (LET_IDS (ASSIGN_INSTR (elem2)));

    ID_VARDEC (EXPRS_EXPR (newnode)) = IDS_VARDEC (LET_IDS (ASSIGN_INSTR (elem1)));
    ID_VARDEC (EXPRS_EXPR (EXPRS_NEXT (newnode)))
      = IDS_VARDEC (LET_IDS (ASSIGN_INSTR (elem2)));

    DBUG_RETURN (newnode);
}
