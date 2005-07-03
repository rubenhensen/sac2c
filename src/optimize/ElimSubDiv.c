/* *
 * $Log$
 * Revision 1.17  2005/07/03 17:11:43  ktr
 * Some codebrushing. IMHO code needs complete rewrite
 *
 * Revision 1.16  2005/04/13 15:26:45  ktr
 * fundef traversal now checks whether function body is NULL
 *
 * Revision 1.15  2005/02/18 22:20:17  mwe
 * debug message removed
 *
 * Revision 1.14  2005/02/16 14:11:09  mwe
 * some doxygen comments added
 *
 * Revision 1.13  2005/02/15 18:42:59  mwe
 * complete new implementation
 *
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

/**
 *
 * @file ElimSubDiv.c
 *
 * @brief replaces substration and division by introducing special
 * primitive negation and reciprocal operator (F_esd_neg, F_esd_rec)
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "new_types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "internal_lib.h"
#include "optimize.h"
#include "free.h"
#include "DataFlowMask.h"
#include "DupTree.h"
#include "SSATransform.h"
#include "constants.h"
#include "shape.h"

#include "ElimSubDiv.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *newassign;
};

/*
 * INFO macros
 */
#define INFO_ESD_FUNDEF(n) (n->fundef)
#define INFO_ESD_NEWASSIGN(n) (n->newassign)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_ESD_FUNDEF (result) = NULL;
    INFO_ESD_NEWASSIGN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/**<!--**************************************************************-->
 *
 * @fn static prf TogglePrf(prf op)
 *
 * @brief returns opposite primitive operator of op
 *
 * @param op primitive operator
 *
 * @return opposite primitive operator of op
 *
 **********************************************************************/
static prf
TogglePrf (prf op)
{
    prf result;
    DBUG_ENTER ("TogglePrf");

    switch (op) {
    case F_sub_SxS:
        result = F_add_SxS;
        break;

    case F_sub_SxA:
        result = F_add_SxA;
        break;

    case F_sub_AxS:
        result = F_add_AxS;
        break;

    case F_sub_AxA:
        result = F_add_AxA;
        break;

    case F_div_SxS:
        result = F_mul_SxS;
        break;

    case F_div_SxA:
        result = F_mul_SxA;
        break;

    case F_div_AxS:
        result = F_mul_AxS;
        break;

    case F_div_AxA:
        result = F_mul_AxA;
        break;

    default:
        DBUG_ASSERT ((0), "Illegal argument prf!");
    }

    DBUG_RETURN (result);
}

/**<!--*************************************************************-->
 *
 * @fn static ntype *GetTypeOfExpr(node *expr)
 *
 * @brief returns the type of param expr
 *
 * @param expr N_expr node
 *
 * @return type of param expr
 *
 **********************************************************************/
static ntype *
GetTypeOfExpr (node *expr)
{
    ntype *type;
    constant *cv;

    DBUG_ENTER ("GetTypeOfExpr");

    switch (NODE_TYPE (EXPRS_EXPR (expr))) {

    case N_id:
        if (TYisAKV (AVIS_TYPE (ID_AVIS (EXPRS_EXPR (expr))))) {
            type = TYmakeAKS (TYcopyType (
                                TYgetScalar (AVIS_TYPE (ID_AVIS (EXPRS_EXPR (expr))))),
                              SHcopyShape (
                                TYgetShape (AVIS_TYPE (ID_AVIS (EXPRS_EXPR (expr))))));
        } else {
            type = TYcopyType (AVIS_TYPE (ID_AVIS (EXPRS_EXPR (expr))));
        }
        break;
    case N_num:

        cv = COaST2Constant (expr);
        if (cv == NULL) {
            type = TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0));
        } else {
            type = TYmakeAKV (TYmakeSimpleType (T_int), cv);
        }
        break;
    case N_float:

        cv = COaST2Constant (expr);
        if (cv == NULL) {
            type = TYmakeAKS (TYmakeSimpleType (T_float), SHcreateShape (0));
        } else {
            type = TYmakeAKV (TYmakeSimpleType (T_float), cv);
        }
        break;
    case N_double:
        cv = COaST2Constant (expr);
        if (cv == NULL) {
            type = TYmakeAKS (TYmakeSimpleType (T_double), SHcreateShape (0));
        } else {
            type = TYmakeAKV (TYmakeSimpleType (T_double), cv);
        }
        break;
    case N_bool:
        cv = COaST2Constant (expr);
        if (cv == NULL) {
            type = TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0));
        } else {
            type = TYmakeAKV (TYmakeSimpleType (T_bool), cv);
        }
        break;
    case N_char:
        cv = COaST2Constant (expr);
        if (cv == NULL) {
            type = TYmakeAKS (TYmakeSimpleType (T_char), SHcreateShape (0));
        } else {
            type = TYmakeAKV (TYmakeSimpleType (T_char), cv);
        }
        break;
    default:
        type = NULL;
        DBUG_ASSERT ((FALSE), "unexpected node type found");
    }

    DBUG_RETURN (type);
}

/**<!--**************************************************************-->
 *
 * @fn node *ESDdoElimSubDiv(node *fundef)
 *
 * @brief start function for traversal
 *
 * @param fundef fundef node
 *
 * @return
 *
 **********************************************************************/
node *
ESDdoElimSubDiv (node *fundef)
{
    info *info;

    DBUG_ENTER ("ESDdoElimSubDiv");

    if (fundef != NULL) {
        DBUG_PRINT ("OPT",
                    ("starting elim sub div in function %s", FUNDEF_NAME (fundef)));

        info = MakeInfo ();

        INFO_ESD_FUNDEF (info) = fundef;

        TRAVpush (TR_esd);
        if (FUNDEF_BODY (fundef) != NULL) {
            FUNDEF_BODY (fundef) = TRAVdo (FUNDEF_BODY (fundef), info);
        }
        TRAVpop ();

        info = FreeInfo (info);
    }

    DBUG_RETURN (fundef);
}

/**<!--***************************************************************-->
 *
 * @fn ESDblock(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 **********************************************************************/
node *
ESDblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ESDblock");

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**<!--***************************************************************-->
 *
 * @fn ESDassign(node *arg_node, info *arg_info)
 *
 * @brief traverses in instructions and inserts new assignments
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 **********************************************************************/
node *
ESDassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ESDassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    INFO_ESD_NEWASSIGN (arg_info) = NULL;

    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }

    if (INFO_ESD_NEWASSIGN (arg_info) != NULL) {
        /*
         * insert new assignment
         */
        ASSIGN_NEXT (INFO_ESD_NEWASSIGN (arg_info)) = arg_node;
        arg_node = INFO_ESD_NEWASSIGN (arg_info);

        INFO_ESD_NEWASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/**<!--*************************************************************-->
 *
 * @fn ESDlet(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 **********************************************************************/
node *
ESDlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ESDlet");

    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**<!--**************************************************************-->
 *
 * @fn ESDprf(node *arg_node, info *arg_info)
 *
 * @brief removes substartion and division, introduces negation
 *        and reciprocal instead
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 **********************************************************************/
node *
ESDprf (node *arg_node, info *arg_info)
{
    prf op = F_noop;
    node *newnode = NULL;

    DBUG_ENTER ("ESDprf");

    INFO_ESD_NEWASSIGN (arg_info) = NULL;

    switch (PRF_PRF (arg_node)) {

    case F_sub_SxS:
    case F_sub_AxS:
    case F_sub_SxA:
    case F_sub_AxA:
        op = F_esd_neg;
        break;
    case F_div_SxS:
    case F_div_AxS:
    case F_div_SxA:
    case F_div_AxA:
        op = F_esd_rec;
        break;
    default:
        break;
    }

    if (op != F_noop) {
        node *avis, *vardec;

        /*
         * create new assign
         */
        avis
          = TBmakeAvis (ILIBtmpVar (), GetTypeOfExpr (EXPRS_NEXT (PRF_ARGS (arg_node))));
        vardec = TBmakeVardec (avis, NULL);
        newnode = TBmakePrf (op, EXPRS_NEXT (PRF_ARGS (arg_node)));
        newnode = TCmakeAssignLet (avis, newnode);
        AVIS_SSAASSIGN (avis) = newnode;

        /*
         * use new assign
         */
        EXPRS_NEXT (PRF_ARGS (arg_node)) = TBmakeExprs (TBmakeId (avis), NULL);
        PRF_PRF (arg_node) = TogglePrf (PRF_PRF (arg_node));

        /*
         * insert vardec
         */
        VARDEC_NEXT (vardec) = FUNDEF_VARDEC (INFO_ESD_FUNDEF (arg_info));
        FUNDEF_VARDEC (INFO_ESD_FUNDEF (arg_info)) = vardec;

        /*
         * save newnode
         */
        INFO_ESD_NEWASSIGN (arg_info) = newnode;
    }

    DBUG_RETURN (arg_node);
}
