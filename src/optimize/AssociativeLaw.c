/* *
 * $Log$
 * Revision 1.21  2003/07/31 16:36:39  mwe
 * some changes regarding to array support
 *
 * Revision 1.20  2003/04/23 19:53:44  mwe
 * small changes in code
 *
 * Revision 1.19  2003/04/10 21:26:06  mwe
 * implement support for arrays
 *
 * Revision 1.18  2003/02/24 17:40:36  mwe
 * declare local used functions as static
 *
 * Revision 1.17  2003/02/15 16:45:38  mwe
 * changed assignment for INFO_AL_TYPES
 *
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
 *  -dfmask[3]:  ARRAYLIST
 *                  nodelist* used to store unused array nodes
 *  -dfmask[4]:  OPTARRAYLIST
 *                  nodelist* used to store edited nodes from ARRAYLIST
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

#define INFO_AL_OPTLIST(n) (nodelist *)(n->dfmask[4])
#define INFO_AL_ARRAYLIST(n) (nodelist *)(n->dfmask[3])
#define INFO_AL_CONSTARRAYLIST(n) (nodelist *)(n->dfmask[5])

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
 *   IsAssociativeAndCommutative(node *arg_node)
 *
 * description:
 *   returns '1' if the primitive operation is associative and commutative
 *
 ****************************************************************************/

static int
IsAssociativeAndCommutative (node *arg_node)
{

    int return_bool;
    DBUG_ENTER ("IsAssociativeAndCommutative");
    switch (PRF_PRF (arg_node)) {
    case F_add_SxS:
    case F_mul_SxS:

    case F_add_AxS:
    case F_mul_AxS:
    case F_add_SxA:
    case F_mul_SxA:
    case F_add_AxA:
    case F_mul_AxA:

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
 *   int ReachedArgument(node *arg_node)
 *
 * description:
 *   returns '1' if the arg_node is an argument defined outside current
 *   block node
 *
 ****************************************************************************/

static int
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

static int
ReachedDefinition (node *arg_node)
{

    int return_bool;
    DBUG_ENTER ("ReachedDefinition");

    if (NODE_TYPE (arg_node) == N_id) {
        if (AVIS_SSAASSIGN (ID_AVIS (arg_node)) == NULL)
            return_bool = 0;
        else if ((NODE_TYPE (
                    LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (arg_node)))))
                  != N_prf))
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

static int
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

static bool
IsConstantArray (node *arg_node)
{

    bool result;
    DBUG_ENTER ("IsConstantArray");

    switch (NODE_TYPE (arg_node)) {
    case N_array:
        result = TRUE;
        break;

    default:
        result = FALSE;
    }

    DBUG_RETURN (result);
}

/*****************************************************************************
 *
 * function:
 *   node *AddNode(node *arg_node, node *arg_info, int nodetype)
 *
 * description:
 *   Add the arg_node to the node-list in arg_info
 *   If the int-argument is '1', the arg_node contains a constant scalar value
 *   If the int-argument is '0', the arg_node contains no constant scalar value
 *   If the int-argument is '2', the arg_node contains an array value
 *   If the int-argument is '3', the arg_node contains an constant array value
 *
 *   Increase the corresponding counter
 *   arg_node is added to the corresponding list
 *
 ****************************************************************************/

static node *
AddNode (node *arg_node, node *arg_info, int nodetype)
{
    nodelist *newnodelistnode = MakeNodelistNode (EXPRS_EXPR (arg_node), NULL);

    DBUG_ENTER ("AddNode");

    if ((nodetype == 1) || (nodetype == 3)) {
        INFO_AL_NUMBEROFCONSTANTS (arg_info) = INFO_AL_NUMBEROFCONSTANTS (arg_info) + 1;
    }

    else {
        INFO_AL_NUMBEROFVARIABLES (arg_info) = INFO_AL_NUMBEROFVARIABLES (arg_info) + 1;
    }

    if (nodetype == 1) {
        NODELIST_NEXT (newnodelistnode) = (INFO_AL_CONSTANTLIST (arg_info));
        INFO_AL_CONSTANTLIST (arg_info) = newnodelistnode;
    }

    else if (nodetype == 2) {

        NODELIST_NEXT (newnodelistnode) = (INFO_AL_ARRAYLIST (arg_info));
        INFO_AL_ARRAYLIST (arg_info) = newnodelistnode;

    }

    else if (nodetype == 3) {

        NODELIST_NEXT (newnodelistnode) = (INFO_AL_CONSTARRAYLIST (arg_info));
        INFO_AL_CONSTARRAYLIST (arg_info) = newnodelistnode;

    }

    else {
        NODELIST_NEXT (newnodelistnode) = (INFO_AL_VARIABLELIST (arg_info));
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
 *   used primitive operation in its definition is not the same (or similar)
 *   as saved in arg_info
 *
 ****************************************************************************/

static int
OtherPrfOp (node *arg_node, node *arg_info)
{
    int otherOp;
    prf otherPrf;
    DBUG_ENTER ("OtherPrfOp");
    if (NODE_TYPE (EXPRS_EXPR (arg_node)) == N_id) {
        if ((AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node))) == NULL)
            || (NODE_TYPE (LET_EXPR (
                  ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node))))))
                != N_prf))
            otherOp = 0;
        else {
            otherPrf = PRF_PRF (
              LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node))))));

            switch (INFO_AL_CURRENTPRF (arg_info)) {
            case F_add_SxS:
            case F_add_AxS:
            case F_add_SxA:
            case F_add_AxA: {
                if ((otherPrf == F_add_SxS) || (otherPrf == F_add_AxS)
                    || (otherPrf == F_add_SxA) || (otherPrf == F_add_AxA))
                    otherOp = 0;
                else
                    otherOp = 1;
                break;
            } break;
            case F_mul_SxS:
            case F_mul_AxS:
            case F_mul_SxA:
            case F_mul_AxA: {
                if ((otherPrf == F_mul_SxS) || (otherPrf == F_mul_AxS)
                    || (otherPrf == F_mul_SxA) || (otherPrf == F_mul_AxA))
                    otherOp = 0;
                else
                    otherOp = 1;
                break;
            } break;
            default: {
                if (INFO_AL_CURRENTPRF (arg_info) == otherPrf)
                    otherOp = 0;
                else
                    otherOp = 1;
            }
            }
        }
    } else
        otherOp = 0;

    DBUG_RETURN (otherOp);
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

static nodelist *
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
 *   node *MakeAssignNodeFromExprsNode( node *newnode , node *arg_info)
 *
 * description:
 *   This function create a new assign-node with the current node as an
 *   argument. The correct vardec-root-node is provided
 *   by the arg_info-node.
 *
 ****************************************************************************/

static node *
MakeAssignNodeFromCurrentNode (node *newnode, node *arg_info, int dim)
{

    node *newvardec;
    types *type;
    char *newname1, *newname2;

    shpseg *shp;
    node *shpnode;
    int shpint;

    DBUG_ENTER ("MakeAssignNodeFromCurrentNode");

    if (NODE_TYPE (newnode) == N_id) {

        shpint = TYPES_DIM ((INFO_AL_TYPE (arg_info)));
        shpnode = Shpseg2Array (TYPES_SHPSEG (INFO_AL_TYPE (arg_info)), shpint);
        shp = Array2Shpseg (shpnode, &shpint);

        type = VARDEC_TYPE (ID_VARDEC (newnode));
        type = MakeTypes (TYPES_BASETYPE (type), TYPES_DIM (type), shp, NULL, NULL);

    } else {

        if (dim > 0) {

            shpint = TYPES_DIM ((INFO_AL_TYPE (arg_info)));
            shpnode = Shpseg2Array (TYPES_SHPSEG (INFO_AL_TYPE (arg_info)), shpint);
            shp = Array2Shpseg (shpnode, &shpint);
            type = MakeTypes (TYPES_BASETYPE ((INFO_AL_TYPE (arg_info))),
                              TYPES_DIM ((INFO_AL_TYPE (arg_info))), shp, NULL, NULL);
        } else {

            type = MakeTypes (TYPES_BASETYPE ((INFO_AL_TYPE (arg_info))), 0, NULL, NULL,
                              NULL);
        }
    }

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
 *   node *MakeAssignNodeFromExprsNode( node *newnode , node *arg_info)
 *
 * description:
 *   This function create a new assign-node with the exprs-node as an
 *   argument. The correct primitive and vardec-root-node are provided
 *   by the arg_info-node.
 *
 ****************************************************************************/

static node *
MakeAssignNodeFromExprsNode (node *newnode, node *arg_info, int dim)
{
    DBUG_ENTER ("MakeAssignNodeFromExprsNode");
    newnode = MakePrf (INFO_AL_CURRENTPRF (arg_info), newnode);

    newnode = MakeAssignNodeFromCurrentNode (newnode, arg_info, dim);

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

static node *
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

static node *
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

static node *
CommitAssignNodes (nodelist *currentList, node *arg_info, int dim)
{
    node *newnode, *elem1, *elem2;
    nodelist *lastListElem;

    DBUG_ENTER ("CommitAssignNodes");

    lastListElem = currentList;

    if (currentList != NULL) {

        while (NODELIST_NEXT (lastListElem) != NULL)
            lastListElem = NODELIST_NEXT (lastListElem);

        while (NODELIST_NEXT (currentList) != NULL) {

            elem1 = NODELIST_NODE (currentList);
            currentList = NODELIST_NEXT (currentList);
            elem2 = NODELIST_NODE (currentList);

            newnode = MakeExprsNodeFromAssignNodes (elem1, elem2);
            newnode = MakeAssignNodeFromExprsNode (newnode, arg_info, dim);

            NODELIST_NEXT (lastListElem) = MakeNodelistNode (newnode, NULL);
            lastListElem = NODELIST_NEXT (lastListElem);
            currentList = NODELIST_NEXT (currentList);
        }
    }

    DBUG_RETURN (arg_info);
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

static node *
CreateAssignNodes (nodelist *currentList, node *arg_info, int dim)
{

    node *listElem;
    DBUG_ENTER ("CreateAssignNodes");

    /*
     * create assign nodes with nodes of currentList
     */

    while (currentList != NULL) {

        listElem = NODELIST_NODE (currentList);
        listElem = DupTree (listElem);
        listElem = MakeAssignNodeFromCurrentNode (listElem, arg_info, dim);
        NODELIST_NODE (currentList) = listElem;
        currentList = NODELIST_NEXT (currentList);
    }

    DBUG_RETURN (arg_info);
}

static node *
CreateOptlist (node *arg_info)
{

    nodelist *currentList, *next;

    DBUG_ENTER ("CreateOptlist");

    currentList = INFO_AL_CONSTANTLIST (arg_info);

    if (currentList != NULL) {
        while (NODELIST_NEXT (currentList) != NULL) {

            next = NODELIST_NEXT (currentList);
            NODELIST_NEXT (currentList) = INFO_AL_OPTLIST (arg_info);
            INFO_AL_OPTLIST (arg_info) = currentList;

            INFO_AL_CONSTANTLIST (arg_info) = next;
            currentList = INFO_AL_CONSTANTLIST (arg_info);
        }
    }

    currentList = INFO_AL_ARRAYLIST (arg_info);

    if (currentList != NULL) {
        while (NODELIST_NEXT (currentList) != NULL) {

            next = NODELIST_NEXT (currentList);
            NODELIST_NEXT (currentList) = INFO_AL_OPTLIST (arg_info);
            INFO_AL_OPTLIST (arg_info) = currentList;

            INFO_AL_ARRAYLIST (arg_info) = next;
            currentList = INFO_AL_ARRAYLIST (arg_info);
        }
    }

    currentList = INFO_AL_CONSTARRAYLIST (arg_info);

    if (currentList != NULL) {
        while (NODELIST_NEXT (currentList) != NULL) {

            next = NODELIST_NEXT (currentList);
            NODELIST_NEXT (currentList) = INFO_AL_OPTLIST (arg_info);
            INFO_AL_OPTLIST (arg_info) = currentList;

            INFO_AL_CONSTARRAYLIST (arg_info) = next;
            currentList = INFO_AL_CONSTARRAYLIST (arg_info);
        }
    }

    currentList = INFO_AL_VARIABLELIST (arg_info);

    if (currentList != NULL) {
        while (NODELIST_NEXT (currentList) != NULL) {

            next = NODELIST_NEXT (currentList);
            NODELIST_NEXT (currentList) = INFO_AL_OPTLIST (arg_info);
            INFO_AL_OPTLIST (arg_info) = currentList;

            INFO_AL_VARIABLELIST (arg_info) = next;
            currentList = INFO_AL_VARIABLELIST (arg_info);
        }
    }

    DBUG_RETURN (arg_info);
}

static bool
ContainsAnArray (node *expr)
{

    bool result = FALSE;

    DBUG_ENTER ("ContainsAnArray");

    if (TYPES_DIM (VARDEC_TYPE (ID_VARDEC (EXPRS_EXPR (expr)))) > 0)
        result = TRUE;

    DBUG_RETURN (result);
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
 *   function AddNode (argument '1': constant node ; '0' no constant node;
 *                              '2': array)
 *
 ****************************************************************************/

static node *
TravElems (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TravElems");

    if (IsConstant (EXPRS_EXPR (arg_node))) {

        arg_info = AddNode (arg_node, arg_info, 1);

    } else {

        if (IsConstantArray (EXPRS_EXPR (arg_node))) {

            arg_info = AddNode (arg_node, arg_info, 3);

        } else {

            if (OtherPrfOp (arg_node, arg_info) || ReachedArgument (EXPRS_EXPR (arg_node))
                || ReachedDefinition (EXPRS_EXPR (arg_node))) {

                /*
                 *
                 */
                if (ContainsAnArray (arg_node)) {

                    int a, b;

                    a = ReachedArgument (EXPRS_EXPR (arg_node));
                    b = OtherPrfOp (arg_node, arg_info);

                    if ((a == 0) && (b == 0)
                        && (NODE_TYPE (LET_EXPR (ASSIGN_INSTR (
                              AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node))))))
                            == N_array)) {

                        node *tmp;
                        tmp = LET_EXPR (ASSIGN_INSTR (
                          AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node)))));
                        tmp = MakeExprs (tmp, NULL);
                        arg_info = AddNode (tmp, arg_info, 3);
                    } else
                        arg_info = AddNode (arg_node, arg_info, 2);
                } else
                    arg_info = AddNode (arg_node, arg_info, 0);

                /*
                 *
                 */
            } else {

                ASSIGN_STATUS (INFO_AL_CURRENTASSIGN (arg_info)) = 0;
                INFO_AL_CURRENTASSIGN (arg_info)
                  = AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node)));

                arg_info = TravElems (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (
                                        ID_AVIS (EXPRS_EXPR (arg_node)))))),
                                      arg_info);
                arg_info
                  = TravElems (EXPRS_NEXT (PRF_ARGS (LET_EXPR (ASSIGN_INSTR (
                                 AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (arg_node))))))),
                               arg_info);
            }
        }
    }

    DBUG_RETURN (arg_info);
}

static prf
GetOperator (prf op, int flag)
{

    prf newop;

    DBUG_ENTER ("GetOperator");

    newop = F_add_SxS;

    switch (op) {

    case F_add_SxS:
    case F_add_AxS:
    case F_add_SxA:
    case F_add_AxA: {
        if (flag == 0)
            newop = F_add_SxS;
        if (flag == 1)
            newop = F_add_SxA;
        if (flag == 2)
            newop = F_add_AxS;
        if (flag == 3)
            newop = F_add_AxA;
        break;
    }

    case F_mul_SxS:
    case F_mul_AxS:
    case F_mul_SxA:
    case F_mul_AxA: {
        if (flag == 0)
            newop = F_mul_SxS;
        if (flag == 1)
            newop = F_mul_SxA;
        if (flag == 2)
            newop = F_mul_AxS;
        if (flag == 3)
            newop = F_mul_AxA;
        break;
    }
    default:
        newop = op;
    }

    DBUG_RETURN (newop);
}

static node *
JoinResults (nodelist *nodelist1, nodelist *nodelist2, node *arg_info, int flag)
{

    node *tmp;

    DBUG_ENTER ("JoinResults");

    if (nodelist1 != NULL) {
        if (nodelist2 != NULL) {

            tmp = MakeExprsNodeFromAssignNodes (NODELIST_NODE (nodelist1),
                                                NODELIST_NODE (nodelist2));
            tmp = MakePrf (GetOperator (INFO_AL_CURRENTPRF (arg_info), flag), tmp);
            tmp = MakeAssignNodeFromCurrentNode (tmp, arg_info, flag);

        } else {

            tmp = MakeExprsNodeFromExprsAndAssignNode (nodelist1,
                                                       MakeNodelistNode (NULL, NULL));
            tmp = MakeAssignNodeFromCurrentNode (EXPRS_EXPR (tmp), arg_info, -1);
        }
    } else if (nodelist2 != NULL) {

        tmp = MakeExprsNodeFromExprsAndAssignNode (nodelist2,
                                                   MakeNodelistNode (NULL, NULL));
        tmp = MakeAssignNodeFromCurrentNode (EXPRS_EXPR (tmp), arg_info, -1);
    } else {

        tmp = NULL;
    }

    DBUG_RETURN (tmp);
}

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

    ASSIGN_STATUS (arg_node) = 1;

    if (ASSIGN_NEXT (arg_node) != NULL) {

        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);

        if (INFO_AL_OPTLIST (arg_info) != NULL) {

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
            nodelist1 = INFO_AL_OPTLIST (arg_info);

            while (nodelist1 != NULL) {
                ASSIGN_NEXT (akt_nassign) = NODELIST_NODE (nodelist1);
                ASSIGN_NEXT (ASSIGN_NEXT (akt_nassign)) = old_succ;
                old_succ = ASSIGN_NEXT (arg_node);
                nodelist1 = NODELIST_NEXT (nodelist1);
            }

            /*
             * reset nodelists in arg_info
             */

            (INFO_AL_OPTLIST (arg_info))
              = RemoveNodelistNodes (INFO_AL_OPTLIST (arg_info));

            FreeNodelist (INFO_AL_OPTLIST (arg_info));
            (INFO_AL_CONSTANTLIST (arg_info)) = NULL;
            (INFO_AL_VARIABLELIST (arg_info)) = NULL;
            INFO_AL_ARRAYLIST (arg_info) = NULL;
            INFO_AL_OPTLIST (arg_info) = NULL;
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
        if ((LET_IDS (arg_node) != NULL) && (IDS_AVIS (LET_IDS (arg_node)) != NULL))
            INFO_AL_TYPE (arg_info)
              = VARDEC_OR_ARG_TYPE (AVIS_VARDECORARG (IDS_AVIS (LET_IDS (arg_node))));

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
    node *nodetype;
    node *node1, *node2;
    nodelist *tmp;
    nodelist *tmp1;

    DBUG_ENTER ("AssociativeLawOptimize");

    if (NODE_TYPE (PRF_ARGS (arg_node)) == N_exprs) {
        if (IsAssociativeAndCommutative (arg_node) == 1) {

            INFO_AL_NUMBEROFVARIABLES (arg_info) = 0;
            INFO_AL_NUMBEROFCONSTANTS (arg_info) = 0;
            INFO_AL_CONSTANTLIST (arg_info) = NULL;
            INFO_AL_VARIABLELIST (arg_info) = NULL;
            INFO_AL_ARRAYLIST (arg_info) = NULL;
            INFO_AL_CONSTARRAYLIST (arg_info) = NULL;
            INFO_AL_OPTLIST (arg_info) = NULL;
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

                nodetype = IDS_VARDEC (LET_IDS (INFO_AL_LETNODE (arg_info)));

                if (!(enforce_ieee)
                    || ((TYPES_BASETYPE (VARDEC_TYPE (nodetype)) != T_float)
                        && (TYPES_BASETYPE (VARDEC_TYPE (nodetype)) != T_double))) {

                    /*
                     * start optimization
                     */
                    arg_info
                      = CreateAssignNodes (INFO_AL_CONSTANTLIST (arg_info), arg_info, 0);
                    arg_info
                      = CreateAssignNodes (INFO_AL_ARRAYLIST (arg_info), arg_info, 1);
                    arg_info
                      = CreateAssignNodes (INFO_AL_VARIABLELIST (arg_info), arg_info, 0);
                    arg_info = CreateAssignNodes (INFO_AL_CONSTARRAYLIST (arg_info),
                                                  arg_info, 1);

                    arg_info
                      = CommitAssignNodes (INFO_AL_CONSTANTLIST (arg_info), arg_info, 0);
                    arg_info
                      = CommitAssignNodes (INFO_AL_ARRAYLIST (arg_info), arg_info, 1);
                    arg_info
                      = CommitAssignNodes (INFO_AL_VARIABLELIST (arg_info), arg_info, 0);
                    arg_info = CommitAssignNodes (INFO_AL_CONSTARRAYLIST (arg_info),
                                                  arg_info, 1);

                    arg_info = CreateOptlist (arg_info);

                    node1 = JoinResults (INFO_AL_CONSTANTLIST (arg_info),
                                         INFO_AL_CONSTARRAYLIST (arg_info), arg_info, 1);
                    if (INFO_AL_CONSTANTLIST (arg_info) != NULL) {

                        tmp = INFO_AL_CONSTANTLIST (arg_info);
                        INFO_AL_CONSTANTLIST (arg_info) = INFO_AL_OPTLIST (arg_info);
                        NODELIST_NEXT (tmp) = INFO_AL_CONSTANTLIST (arg_info);
                        INFO_AL_OPTLIST (arg_info) = tmp;
                    }
                    if (INFO_AL_CONSTARRAYLIST (arg_info) != NULL) {

                        tmp = INFO_AL_CONSTARRAYLIST (arg_info);
                        INFO_AL_CONSTARRAYLIST (arg_info) = INFO_AL_OPTLIST (arg_info);
                        NODELIST_NEXT (tmp) = INFO_AL_CONSTARRAYLIST (arg_info);
                        INFO_AL_OPTLIST (arg_info) = tmp;
                    }
                    node2 = JoinResults (INFO_AL_VARIABLELIST (arg_info),
                                         INFO_AL_ARRAYLIST (arg_info), arg_info, 1);
                    if (INFO_AL_VARIABLELIST (arg_info) != NULL) {

                        tmp = INFO_AL_VARIABLELIST (arg_info);
                        INFO_AL_VARIABLELIST (arg_info) = INFO_AL_OPTLIST (arg_info);
                        NODELIST_NEXT (tmp) = INFO_AL_VARIABLELIST (arg_info);
                        INFO_AL_OPTLIST (arg_info) = tmp;
                    }
                    if (INFO_AL_ARRAYLIST (arg_info) != NULL) {

                        tmp = INFO_AL_ARRAYLIST (arg_info);
                        INFO_AL_ARRAYLIST (arg_info) = INFO_AL_OPTLIST (arg_info);
                        NODELIST_NEXT (tmp) = INFO_AL_ARRAYLIST (arg_info);
                        INFO_AL_OPTLIST (arg_info) = tmp;
                    }

                    if (node1 != NULL) {
                        if (node2 != NULL) {

                            int dim, tmp;

                            tmp = TYPES_DIM (
                              VARDEC_TYPE (IDS_VARDEC (LET_IDS (ASSIGN_INSTR (node1)))));
                            if (tmp == 0) {
                                if (TYPES_DIM (VARDEC_TYPE (
                                      IDS_VARDEC (LET_IDS (ASSIGN_INSTR (node2)))))
                                    == 0)
                                    dim = 0;
                                else
                                    dim = 1;
                            } else if (TYPES_DIM (VARDEC_TYPE (
                                         IDS_VARDEC (LET_IDS (ASSIGN_INSTR (node2)))))
                                       == 0)
                                dim = 2;
                            else
                                dim = 3;

                            INFO_AL_CONSTANTLIST (arg_info)
                              = MakeNodelistNode (node1, MakeNodelistNode (node2, NULL));
                            tmp1 = INFO_AL_CONSTANTLIST (arg_info);
                            INFO_AL_CONSTANTLIST (arg_info) = INFO_AL_OPTLIST (arg_info);
                            NODELIST_NEXT (NODELIST_NEXT (tmp1))
                              = INFO_AL_CONSTANTLIST (arg_info);
                            INFO_AL_OPTLIST (arg_info) = tmp1;
                            node1 = MakeExprsNodeFromAssignNodes (node1, node2);
                            node2 = PRF_ARGS (arg_node);
                            PRF_ARGS (arg_node) = node1;

                            PRF_PRF (arg_node) = GetOperator (PRF_PRF (arg_node), dim);
                        } else {
                            INFO_AL_CONSTANTLIST (arg_info)
                              = MakeNodelistNode (node1, NULL);
                            tmp1 = INFO_AL_CONSTANTLIST (arg_info);
                            INFO_AL_CONSTANTLIST (arg_info) = INFO_AL_OPTLIST (arg_info);
                            NODELIST_NEXT (tmp1) = INFO_AL_CONSTANTLIST (arg_info);
                            INFO_AL_OPTLIST (arg_info) = tmp1;
                            node1 = MakeExprsNodeFromExprsAndAssignNode (
                              MakeNodelistNode (node1, NULL),
                              MakeNodelistNode (NULL, NULL));
                            node2 = arg_node;
                            arg_node = node1;
                        }
                    } else {
                        INFO_AL_CONSTANTLIST (arg_info) = MakeNodelistNode (node2, NULL);
                        tmp1 = INFO_AL_CONSTANTLIST (arg_info);
                        INFO_AL_CONSTANTLIST (arg_info) = INFO_AL_OPTLIST (arg_info);
                        NODELIST_NEXT (tmp1) = INFO_AL_CONSTANTLIST (arg_info);
                        INFO_AL_OPTLIST (arg_info) = tmp1;
                        node1
                          = MakeExprsNodeFromExprsAndAssignNode (MakeNodelistNode (node2,
                                                                                   NULL),
                                                                 MakeNodelistNode (NULL,
                                                                                   NULL));
                        node2 = arg_node;
                        arg_node = node1;
                    }
                    Free (node2);

                } else {
                    /*
                     * nothing to optimize
                     */
                    FreeNodelist (INFO_AL_CONSTANTLIST (arg_info));
                    FreeNodelist (INFO_AL_VARIABLELIST (arg_info));
                    FreeNodelist (INFO_AL_CONSTARRAYLIST (arg_info));
                    FreeNodelist (INFO_AL_ARRAYLIST (arg_info));
                    INFO_AL_NUMBEROFVARIABLES (arg_info) = 0;
                    INFO_AL_NUMBEROFCONSTANTS (arg_info) = 0;
                }
            } else {
                /*
                 * nothing to optimize
                 */
                FreeNodelist (INFO_AL_CONSTANTLIST (arg_info));
                FreeNodelist (INFO_AL_VARIABLELIST (arg_info));
                FreeNodelist (INFO_AL_CONSTARRAYLIST (arg_info));
                FreeNodelist (INFO_AL_ARRAYLIST (arg_info));
                INFO_AL_NUMBEROFVARIABLES (arg_info) = 0;
                INFO_AL_NUMBEROFCONSTANTS (arg_info) = 0;
            }
        }
    }
    DBUG_RETURN (arg_node);
}
