/* *
 * $Log$
 * Revision 1.10  2004/10/21 17:21:34  sah
 * ElimSubDiv is limited on types that do support
 * the optimisation. Now, ElimSubDiv optimises only
 * those functions it really can optimise ;)
 *
 * Revision 1.9  2004/10/19 15:29:12  sah
 * the negative One in GF(2) is 1 ;)
 *
 * Revision 1.8  2004/10/19 14:38:52  sah
 * Added support for T_bool types...
 *
 * Revision 1.7  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 1.6  2004/07/07 15:57:05  mwe
 * former log-messages added
 *
 *
 *
 *----------------------------
 *revision 1.5    locked by: mwe;
 *date: 2004/07/07 15:43:36;  author: mwe;  state: Exp;  lines: +6 -16
 *last changes undone (all changes connected to new type representation with ntype*)
 *----------------------------
 *revision 1.4
 *date: 2004/06/10 15:08:46;  author: mwe;  state: Exp;  lines: +1 -4
 *unused variables removed
 *----------------------------
 *revision 1.3
 *date: 2004/06/10 14:43:06;  author: mwe;  state: Exp;  lines: +15 -2
 *usage of ntype* instead of type added
 *----------------------------
 *revision 1.2
 *date: 2004/02/06 14:19:33;  author: mwe;  state: Exp;  lines: +0 -1
 *remove ASSIGN2
 *----------------------------
 *revision 1.1
 *date: 2003/04/26 20:54:17;  author: mwe;  state: Exp;
 *Initial revision
 */

#define NEW_INFO

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

#include "ElimSubDiv.h"

/*
 * INFO structure
 */
struct INFO {
    types *type;
    node *blocknode;
    node *letnode;
    node *newnode;
    int counter;
};

/*
 * INFO macros
 */
#define INFO_ESD_TYPE(n) (n->type)
#define INFO_ESD_BLOCKNODE(n) (n->blocknode)
#define INFO_ESD_LETNODE(n) (n->letnode)
#define INFO_ESD_NEWNODE(n) (n->newnode)
#define INFO_ESD_COUNTER(n) (n->counter)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_ESD_TYPE (result) = NULL;
    INFO_ESD_BLOCKNODE (result) = NULL;
    INFO_ESD_LETNODE (result) = NULL;
    INFO_ESD_NEWNODE (result) = NULL;
    INFO_ESD_COUNTER (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
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
 *   node *MakeAssignLetNodeFromCurrentNode( node *newnode , info *arg_info)
 *
 * description:
 *   This function create a new assign-node with the exprs-node as an
 *   argument. The correct vardec-root-node is provided
 *   by the arg_info-node.
 *
 ****************************************************************************/

static node *
MakeAssignNode (node *newnode, info *arg_info)
{

    node *newvardec, *newshpseg;
    types *type;
    char *newname1, *newname2, *newname, *newmod;
    shpseg *shp;
    node *shpnode;
    int shpint;

    DBUG_ENTER ("MakeAssignNode");

    /*
     * try to investigate the correct BASETYPE
     */

    if (TYPES_NAME (INFO_ESD_TYPE (arg_info)) != NULL)
        newname = StringCopy (TYPES_NAME (INFO_ESD_TYPE (arg_info)));
    if (TYPES_MOD (INFO_ESD_TYPE (arg_info)) != NULL)
        newmod = StringCopy (TYPES_MOD (INFO_ESD_TYPE (arg_info)));

    if ((PRF_PRF (newnode) == F_mul_SxS) || (PRF_PRF (newnode) == F_div_SxS)) {
        type
          = MakeTypes (TYPES_BASETYPE ((INFO_ESD_TYPE (arg_info))), 0, NULL, NULL, NULL);
    } else {
        newshpseg = NULL;
        shpint = TYPES_DIM ((INFO_ESD_TYPE (arg_info)));
        shpnode = Shpseg2Array (TYPES_SHPSEG (INFO_ESD_TYPE (arg_info)), shpint);
        shp = Array2Shpseg (shpnode, &shpint);

        type = VARDEC_TYPE (ID_VARDEC (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (newnode)))));
        type = MakeTypes (TYPES_BASETYPE ((INFO_ESD_TYPE (arg_info))),
                          TYPES_DIM ((INFO_ESD_TYPE (arg_info))), shp, NULL, NULL);
    }

    newname1 = TmpVar ();

    newvardec
      = MakeVardec (newname1, type, (BLOCK_VARDEC (INFO_ESD_BLOCKNODE (arg_info))));

    BLOCK_VARDEC (INFO_ESD_BLOCKNODE (arg_info)) = newvardec;

    newname2 = StringCopy (newname1);
    newnode = MakeAssignLet (newname2, newvardec, newnode);

    VARDEC_OBJDEF (newvardec) = newnode;
    AVIS_SSAASSIGN (VARDEC_AVIS (newvardec)) = newnode;

    DBUG_RETURN (newnode);
}

static node *
CreateNegOne (info *arg_info)
{

    node *newnode;

    DBUG_ENTER ("CreateNegOne");

    newnode = NULL;

    if (TYPES_BASETYPE (INFO_ESD_TYPE (arg_info)) == T_int) {

        newnode = MakeNum (-1);

    } else if (TYPES_BASETYPE (INFO_ESD_TYPE (arg_info)) == T_double) {

        newnode = MakeDouble (-1.0);

    } else if (TYPES_BASETYPE (INFO_ESD_TYPE (arg_info)) == T_float) {

        newnode = MakeFloat (-1.0f);

    } else if (TYPES_BASETYPE (INFO_ESD_TYPE (arg_info)) == T_bool) {
        newnode = MakeBool (TRUE);
    } else {
        DBUG_ASSERT (FALSE, "Unexpected BASETYPE!");
    }

    newnode = MakeExprs (newnode, NULL);

    DBUG_RETURN (newnode);
}

static void
CreateNegative (node *arg, info *arg_info, int flag)
{

    node *node1, *newnode;

    DBUG_ENTER ("CreateNegative");

    node1 = MakeExprs (arg, NULL);
    newnode = CreateNegOne (arg_info);

    EXPRS_NEXT (newnode) = node1;

    if (flag == 0)
        newnode = MakePrf (F_mul_SxS, newnode);
    else
        newnode = MakePrf (F_mul_SxA, newnode);

    newnode = MakeAssignNode (newnode, arg_info);

    INFO_ESD_NEWNODE (arg_info) = newnode;

    DBUG_VOID_RETURN;
}

static node *
CreateOne (info *arg_info)
{

    node *newnode;

    DBUG_ENTER ("CreateOne");

    newnode = NULL;

    if (TYPES_BASETYPE (INFO_ESD_TYPE (arg_info)) == T_int) {

        newnode = MakeNum (1);

    } else if (TYPES_BASETYPE (INFO_ESD_TYPE (arg_info)) == T_double) {

        newnode = MakeDouble (1.0);

    } else if (TYPES_BASETYPE (INFO_ESD_TYPE (arg_info)) == T_float) {

        newnode = MakeFloat (1.0f);

    } else if (TYPES_BASETYPE (INFO_ESD_TYPE (arg_info)) == T_bool) {

        newnode = MakeBool (TRUE);

    } else {
        DBUG_ASSERT (FALSE, "Unexpected BASETYPE!");
    }

    newnode = MakeExprs (newnode, NULL);

    DBUG_RETURN (newnode);
}

static void
CreateInverse (node *arg, info *arg_info, int flag)
{

    node *node1, *newnode;

    DBUG_ENTER ("CreateInverse");

    node1 = MakeExprs (arg, NULL);
    newnode = CreateOne (arg_info);

    EXPRS_NEXT (newnode) = node1;

    if (flag == 0)
        newnode = MakePrf (F_div_SxS, newnode);
    else
        newnode = MakePrf (F_div_SxA, newnode);

    newnode = MakeAssignNode (newnode, arg_info);

    INFO_ESD_NEWNODE (arg_info) = newnode;

    DBUG_VOID_RETURN;
}

node *
ElimSubDiv (node *arg_node)
{
    info *info;
    funtab *old_tab;

    DBUG_ENTER ("ElimSubDiv");

    if (arg_node != NULL) {
        DBUG_PRINT ("OPT",
                    ("starting elim sub div in function %s", FUNDEF_NAME (arg_node)));

        info = MakeInfo ();

        old_tab = act_tab;
        act_tab = esd_tab;

        arg_node = Trav (arg_node, info);

        act_tab = old_tab;

        info = FreeInfo (info);
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   ALblock(node *arg_node, info *arg_info)
 *
 * description:
 *   store block-node for access to vardec-nodes
 *   store types-node as shape for new types-nodes
 *   reset nodelists
 *   traverse through block-nodes
 ****************************************************************************/

node *
ESDblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ESDblock");

    if (BLOCK_INSTR (arg_node) != NULL) {

        INFO_ESD_COUNTER (arg_info) = 0;
        /*
         * store pointer on actual N_block-node for append of new N_vardec nodes
         */
        if (BLOCK_VARDEC (arg_node) != NULL) {
            INFO_ESD_TYPE (arg_info) = VARDEC_TYPE (BLOCK_VARDEC (arg_node));
            INFO_ESD_BLOCKNODE (arg_info) = arg_node;
        }

        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    if (INFO_ESD_NEWNODE (arg_info) != NULL) {

        ASSIGN_NEXT (INFO_ESD_NEWNODE (arg_info)) = BLOCK_INSTR (arg_node);
        BLOCK_INSTR (arg_node) = INFO_ESD_NEWNODE (arg_info);
    }
    INFO_ESD_NEWNODE (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   ALassign(node *arg_node, info *arg_info)
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
ESDassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ESDassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {

        ASSIGN_STATUS (arg_node) = 1;

        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);

        if (INFO_ESD_NEWNODE (arg_info) != NULL) {

            ASSIGN_NEXT (INFO_ESD_NEWNODE (arg_info)) = ASSIGN_NEXT (arg_node);
            ASSIGN_NEXT (arg_node) = INFO_ESD_NEWNODE (arg_info);
        }

        INFO_ESD_NEWNODE (arg_info) = NULL;

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
 *   ALlet(node *arg_node, info *arg_info)
 *
 * description:
 *   store current let-node to include last created new primitive-node
 *   traverse N_let-nodes
 *
 ****************************************************************************/

node *
ESDlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ESDlet");
    if (LET_EXPR (arg_node) != NULL) {

        INFO_ESD_LETNODE (arg_info) = arg_node;
        if ((LET_IDS (arg_node) != NULL) && (IDS_AVIS (LET_IDS (arg_node)) != NULL)) {
            INFO_ESD_TYPE (arg_info)
              = VARDEC_OR_ARG_TYPE (AVIS_VARDECORARG (IDS_AVIS (LET_IDS (arg_node))));

            if ((!enforce_ieee)
                || ((TYPES_BASETYPE (INFO_ESD_TYPE (arg_info)) != T_double)
                    && (TYPES_BASETYPE (INFO_ESD_TYPE (arg_info)) != T_float)))

                LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   ALprf(node *arg_node, info *arg_info)
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
ESDprf (node *arg_node, info *arg_info)
{

    node *newnode;

    DBUG_ENTER ("ESDprf");

    if ((NODE_TYPE (PRF_ARGS (arg_node)) == N_exprs)
        && (INFO_ESD_COUNTER (arg_info) < 20000)) {

        if ((PRF_PRF (arg_node) == F_sub_SxS)
            && ((TYPES_BASETYPE (INFO_ESD_TYPE (arg_info)) == T_double)
                || (TYPES_BASETYPE (INFO_ESD_TYPE (arg_info)) == T_float)
                || (TYPES_BASETYPE (INFO_ESD_TYPE (arg_info)) == T_int)
                || (TYPES_BASETYPE (INFO_ESD_TYPE (arg_info)) == T_bool))) {

            PRF_PRF (arg_node) = F_add_SxS;
            CreateNegative (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))), arg_info, 0);

            newnode = MakeExprsNodeFromAssignNode (INFO_ESD_NEWNODE (arg_info));

            EXPRS_NEXT (PRF_ARGS (arg_node)) = newnode;

            INFO_ESD_COUNTER (arg_info) = INFO_ESD_COUNTER (arg_info) + 0;
        }

        if ((PRF_PRF (arg_node) == F_div_SxS)
            && (NODE_TYPE (EXPRS_EXPR (PRF_ARGS (arg_node))) == N_id)
            && ((TYPES_BASETYPE (INFO_ESD_TYPE (arg_info)) == T_double)
                || (TYPES_BASETYPE (INFO_ESD_TYPE (arg_info)) == T_float))) {

            PRF_PRF (arg_node) = F_mul_SxS;
            CreateInverse (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))), arg_info, 0);

            newnode = MakeExprsNodeFromAssignNode (INFO_ESD_NEWNODE (arg_info));

            EXPRS_NEXT (PRF_ARGS (arg_node)) = newnode;

            INFO_ESD_COUNTER (arg_info) = INFO_ESD_COUNTER (arg_info) + 0;
        }

        if (PRF_PRF (arg_node) == F_sub_AxS) {
            PRF_PRF (arg_node) = F_add_AxS;
            CreateNegative (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))), arg_info, 0);

            newnode = MakeExprsNodeFromAssignNode (INFO_ESD_NEWNODE (arg_info));

            EXPRS_NEXT (PRF_ARGS (arg_node)) = newnode;
        }

        if ((PRF_PRF (arg_node) == F_div_AxS)
            && (TYPES_BASETYPE (INFO_ESD_TYPE (arg_info)) != T_int)) {

            PRF_PRF (arg_node) = F_mul_AxS;
            CreateInverse (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))), arg_info, 0);

            newnode = MakeExprsNodeFromAssignNode (INFO_ESD_NEWNODE (arg_info));

            EXPRS_NEXT (PRF_ARGS (arg_node)) = newnode;
        }

        if ((PRF_PRF (arg_node) == F_div_SxA)
            && (NODE_TYPE (EXPRS_EXPR (PRF_ARGS (arg_node))) == N_id)
            && (TYPES_BASETYPE (INFO_ESD_TYPE (arg_info)) != T_int)) {

            PRF_PRF (arg_node) = F_mul_SxA;
            CreateInverse (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))), arg_info, 1);

            newnode = MakeExprsNodeFromAssignNode (INFO_ESD_NEWNODE (arg_info));

            EXPRS_NEXT (PRF_ARGS (arg_node)) = newnode;
        }

        if ((PRF_PRF (arg_node) == F_div_AxA)
            && (TYPES_BASETYPE (INFO_ESD_TYPE (arg_info)) != T_int)) {

            PRF_PRF (arg_node) = F_mul_AxA;
            CreateInverse (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))), arg_info, 1);

            newnode = MakeExprsNodeFromAssignNode (INFO_ESD_NEWNODE (arg_info));

            EXPRS_NEXT (PRF_ARGS (arg_node)) = newnode;
        }

        if (PRF_PRF (arg_node) == F_sub_AxA) {

            PRF_PRF (arg_node) = F_add_AxA;
            CreateNegative (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))), arg_info, 1);

            newnode = MakeExprsNodeFromAssignNode (INFO_ESD_NEWNODE (arg_info));

            EXPRS_NEXT (PRF_ARGS (arg_node)) = newnode;
        }

        if (PRF_PRF (arg_node) == F_sub_SxA) {

            PRF_PRF (arg_node) = F_add_SxA;
            CreateNegative (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))), arg_info, 1);

            newnode = MakeExprsNodeFromAssignNode (INFO_ESD_NEWNODE (arg_info));

            EXPRS_NEXT (PRF_ARGS (arg_node)) = newnode;
        }
    }
    DBUG_RETURN (arg_node);
}
