/* *
 * $Log$
 * Revision 1.12  2004/11/24 12:05:40  mwe
 * changed signature of TBmakeLet
 *
 * Revision 1.11  2004/11/10 18:27:29  mwe
 * code for type upgrade added
 * use ntype-structure instead of type-structure
 * new code deactivated by MWE_NTYPE_READY
 *
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

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "new_types.h"
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
    ntype *newtype;
    node *blocknode;
    node *letnode;
    node *newnode;
    int counter;
};

/*
 * INFO macros
 */
#define INFO_ESD_TYPE(n) (n->type)
#define INFO_ESD_NTYPE(n) ((ntype *)(n->newtype))
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

    result = ILIBmalloc (sizeof (info));

    INFO_ESD_TYPE (result) = NULL;
    INFO_ESD_NTYPE (result) = NULL;
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

    info = ILIBfree (info);

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

    DBUG_ENTER ("MakeExprsNodeFromAssignNode");

    newnode = TBmakeExprs (DUPdupIdsId (elem1), NULL);

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

    node *newavis, *newvardec;

    DBUG_ENTER ("MakeAssignNode");

    /*
     * try to investigate the correct BASETYPE
     */

    newavis = TBmakeAvis (ILIBtmpVar (), TYcopyType (INFO_ESD_NTYPE (arg_info)));
    newvardec = TBmakeVardec (newavis, BLOCK_VARDEC (INFO_ESD_BLOCKNODE (arg_info)));
    BLOCK_VARDEC (INFO_ESD_BLOCKNODE (arg_info)) = newvardec;

    newnode = TBmakeAssign (TBmakeLet (TBmakeIds (newavis, NULL), newnode), NULL);
    ;

    DBUG_RETURN (newnode);
}

static node *
CreateNegOne (info *arg_info)
{

    node *newnode;
    simpletype basetype;

    DBUG_ENTER ("CreateNegOne");

    newnode = NULL;

    if (TYisArray (INFO_ESD_NTYPE (arg_info))) {

        basetype = TYgetSimpleType (TYgetScalar (INFO_ESD_NTYPE (arg_info)));
    } else if (TYisSimple (INFO_ESD_NTYPE (arg_info))) {

        basetype = TYgetSimpleType (INFO_ESD_NTYPE (arg_info));
    }

    if ((TYisArray (INFO_ESD_NTYPE (arg_info)))
        || (TYisSimple (INFO_ESD_NTYPE (arg_info)))) {

        if (basetype == T_int) {

            newnode = TBmakeNum (-1);

        } else if (basetype == T_double) {

            newnode = TBmakeDouble (-1.0);

        } else if (basetype == T_float) {

            newnode = TBmakeFloat (-1.0f);

        } else if (basetype == T_bool) {
            newnode = TBmakeBool (TRUE);
        } else {
            DBUG_ASSERT (FALSE, "Unexpected BASETYPE!");
        }
    }

    newnode = TBmakeExprs (newnode, NULL);

    DBUG_RETURN (newnode);
}

static void
CreateNegative (node *arg, info *arg_info, bool flag)
{

    node *node1, *newnode;

    DBUG_ENTER ("CreateNegative");

    node1 = TBmakeExprs (arg, NULL);
    newnode = CreateNegOne (arg_info);

    EXPRS_NEXT (newnode) = node1;

    if (flag)
        newnode = TBmakePrf (F_mul_SxA, newnode);
    else
        newnode = TBmakePrf (F_mul_SxS, newnode);

    newnode = MakeAssignNode (newnode, arg_info);

    INFO_ESD_NEWNODE (arg_info) = newnode;

    DBUG_VOID_RETURN;
}

static node *
CreateOne (info *arg_info)
{

    node *newnode;
    simpletype basetype;

    DBUG_ENTER ("CreateOne");

    newnode = NULL;

    if (TYisArray (INFO_ESD_NTYPE (arg_info))) {

        basetype = TYgetSimpleType (TYgetScalar (INFO_ESD_NTYPE (arg_info)));
    } else if (TYisSimple (INFO_ESD_NTYPE (arg_info))) {

        basetype = TYgetSimpleType (INFO_ESD_NTYPE (arg_info));
    }

    if ((TYisArray (INFO_ESD_NTYPE (arg_info)))
        || (TYisSimple (INFO_ESD_NTYPE (arg_info)))) {

        if (basetype == T_int) {

            newnode = TBmakeNum (1);

        } else if (basetype == T_double) {

            newnode = TBmakeDouble (1.0);

        } else if (basetype == T_float) {

            newnode = TBmakeFloat (1.0f);

        } else if (basetype == T_bool) {

            newnode = TBmakeBool (TRUE);

        } else {
            DBUG_ASSERT (FALSE, "Unexpected BASETYPE!");
        }
    }

    newnode = TBmakeExprs (newnode, NULL);

    DBUG_RETURN (newnode);
}

static void
CreateInverse (node *arg, info *arg_info, bool flag)
{

    node *node1, *newnode;

    DBUG_ENTER ("CreateInverse");

    node1 = TBmakeExprs (arg, NULL);
    newnode = CreateOne (arg_info);

    EXPRS_NEXT (newnode) = node1;

    if (flag)
        newnode = TBmakePrf (F_div_SxA, newnode);
    else
        newnode = TBmakePrf (F_div_SxS, newnode);

    newnode = MakeAssignNode (newnode, arg_info);

    INFO_ESD_NEWNODE (arg_info) = newnode;

    DBUG_VOID_RETURN;
}

node *
ESDdoElimSubDiv (node *arg_node)
{
    info *info;

    DBUG_ENTER ("ESDdoElimSubDiv");

    if (arg_node != NULL) {
        DBUG_PRINT ("OPT",
                    ("starting elim sub div in function %s", FUNDEF_NAME (arg_node)));

        info = MakeInfo ();
        TRAVpush (TR_esd);
        arg_node = TRAVdo (arg_node, info);
        TRAVpop ();

        info = FreeInfo (info);
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   ESDblock(node *arg_node, info *arg_info)
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
            INFO_ESD_NTYPE (arg_info) = AVIS_TYPE (VARDEC_AVIS (BLOCK_VARDEC (arg_node)));
            INFO_ESD_BLOCKNODE (arg_info) = arg_node;
        }

        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
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
 *   ESDassign(node *arg_node, info *arg_info)
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

        ASSIGN_ISUNUSED (arg_node) = TRUE;

        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);

        if (INFO_ESD_NEWNODE (arg_info) != NULL) {

            ASSIGN_NEXT (INFO_ESD_NEWNODE (arg_info)) = ASSIGN_NEXT (arg_node);
            ASSIGN_NEXT (arg_node) = INFO_ESD_NEWNODE (arg_info);
        }

        INFO_ESD_NEWNODE (arg_info) = NULL;

        /*
         * traverse in N_let-node
         */

        if ((ASSIGN_INSTR (arg_node) != NULL) && (ASSIGN_ISUNUSED (arg_node))) {

            ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
        }
    }
    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   ESDlet(node *arg_node, info *arg_info)
 *
 * description:
 *   store current let-node to include last created new primitive-node
 *   traverse N_let-nodes
 *
 ****************************************************************************/

node *
ESDlet (node *arg_node, info *arg_info)
{

    ntype *tmp;

    DBUG_ENTER ("ESDlet");
    if (LET_EXPR (arg_node) != NULL) {

        INFO_ESD_LETNODE (arg_info) = arg_node;
        if ((LET_IDS (arg_node) != NULL) && (IDS_AVIS (LET_IDS (arg_node)) != NULL)) {
            INFO_ESD_NTYPE (arg_info) = AVIS_TYPE (IDS_AVIS (LET_IDS (arg_node)));

            tmp = AVIS_TYPE (IDS_AVIS (LET_IDS (arg_node)));

            if (TYisArray (tmp)) {
                tmp = TYgetScalar (tmp);
            }

            if (TYisSimple (tmp)
                && ((!global.enforce_ieee)
                    || ((TYgetSimpleType (tmp) != T_double)
                        && (TYgetSimpleType (tmp) != T_float)))) {

                LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   ESDprf(node *arg_node, info *arg_info)
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
    simpletype basetype;

    DBUG_ENTER ("ESDprf");

    if ((NODE_TYPE (PRF_ARGS (arg_node)) == N_exprs)
        && (INFO_ESD_COUNTER (arg_info) < 20000)) {

        if (TYisArray (INFO_ESD_NTYPE (arg_info))) {

            basetype = TYgetSimpleType (TYgetScalar (INFO_ESD_NTYPE (arg_info)));

        } else if (TYisSimple (INFO_ESD_NTYPE (arg_info))) {

            basetype = TYgetSimpleType (INFO_ESD_NTYPE (arg_info));
        }

        if ((TYisArray (INFO_ESD_NTYPE (arg_info)))
            || (TYisSimple (INFO_ESD_NTYPE (arg_info)))) {
            if ((PRF_PRF (arg_node) == F_sub_SxS)
                && ((basetype == T_double) || (basetype == T_float) || (basetype == T_int)
                    || (basetype == T_bool))) {

                PRF_PRF (arg_node) = F_add_SxS;
                CreateNegative (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))), arg_info,
                                FALSE);

                newnode = MakeExprsNodeFromAssignNode (INFO_ESD_NEWNODE (arg_info));

                EXPRS_NEXT (PRF_ARGS (arg_node)) = newnode;

                INFO_ESD_COUNTER (arg_info) = INFO_ESD_COUNTER (arg_info) + 0;
            }
        }

        if ((PRF_PRF (arg_node) == F_div_SxS)
            && (NODE_TYPE (EXPRS_EXPR (PRF_ARGS (arg_node))) == N_id)
            && ((basetype == T_double) || (basetype == T_float))) {

            PRF_PRF (arg_node) = F_mul_SxS;
            CreateInverse (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))), arg_info,
                           FALSE);

            newnode = MakeExprsNodeFromAssignNode (INFO_ESD_NEWNODE (arg_info));

            EXPRS_NEXT (PRF_ARGS (arg_node)) = newnode;

            INFO_ESD_COUNTER (arg_info) = INFO_ESD_COUNTER (arg_info) + 0;
        }

        if (PRF_PRF (arg_node) == F_sub_AxS) {
            PRF_PRF (arg_node) = F_add_AxS;
            CreateNegative (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))), arg_info,
                            FALSE);

            newnode = MakeExprsNodeFromAssignNode (INFO_ESD_NEWNODE (arg_info));

            EXPRS_NEXT (PRF_ARGS (arg_node)) = newnode;
        }

        if ((PRF_PRF (arg_node) == F_div_AxS) && (basetype != T_int)) {

            PRF_PRF (arg_node) = F_mul_AxS;
            CreateInverse (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))), arg_info,
                           FALSE);

            newnode = MakeExprsNodeFromAssignNode (INFO_ESD_NEWNODE (arg_info));

            EXPRS_NEXT (PRF_ARGS (arg_node)) = newnode;
        }

        if ((PRF_PRF (arg_node) == F_div_SxA)
            && (NODE_TYPE (EXPRS_EXPR (PRF_ARGS (arg_node))) == N_id)
            && (basetype != T_int)) {

            PRF_PRF (arg_node) = F_mul_SxA;
            CreateInverse (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))), arg_info, TRUE);

            newnode = MakeExprsNodeFromAssignNode (INFO_ESD_NEWNODE (arg_info));

            EXPRS_NEXT (PRF_ARGS (arg_node)) = newnode;
        }

        if ((PRF_PRF (arg_node) == F_div_AxA) && (basetype != T_int)) {

            PRF_PRF (arg_node) = F_mul_AxA;
            CreateInverse (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))), arg_info, TRUE);

            newnode = MakeExprsNodeFromAssignNode (INFO_ESD_NEWNODE (arg_info));

            EXPRS_NEXT (PRF_ARGS (arg_node)) = newnode;
        }

        if (PRF_PRF (arg_node) == F_sub_AxA) {

            PRF_PRF (arg_node) = F_add_AxA;
            CreateNegative (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))), arg_info,
                            TRUE);

            newnode = MakeExprsNodeFromAssignNode (INFO_ESD_NEWNODE (arg_info));

            EXPRS_NEXT (PRF_ARGS (arg_node)) = newnode;
        }

        if (PRF_PRF (arg_node) == F_sub_SxA) {

            PRF_PRF (arg_node) = F_add_SxA;
            CreateNegative (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))), arg_info,
                            TRUE);

            newnode = MakeExprsNodeFromAssignNode (INFO_ESD_NEWNODE (arg_info));

            EXPRS_NEXT (PRF_ARGS (arg_node)) = newnode;
        }
    }
    DBUG_RETURN (arg_node);
}
