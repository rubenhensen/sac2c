/* *
 * $Log$
 * Revision 1.9  2005/02/15 18:43:40  mwe
 * completely new implementation
 *
 * Revision 1.8  2004/11/24 12:05:40  mwe
 * changed signature of TBmakeLet
 *
 * Revision 1.7  2004/11/23 20:29:02  khf
 * SacDevCamp04: Compiles
 *
 * Revision 1.6  2004/11/10 18:27:29  mwe
 * code for type upgrade added
 * use ntype-structure instead of type-structure
 * new code deactivated by MWE_NTYPE_READY
 *
 * Revision 1.5  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 1.4  2004/07/07 15:57:05  mwe
 * former log-messages added
 *
 *
 *
 * revision 1.3    locked by: mwe;
 * date: 2004/07/07 15:43:36;  author: mwe;  state: Exp;  lines: +2 -6
 * last changes undone (all changes connected to new type representation with ntype*)
 * ----------------------------
 * revision 1.2
 * date: 2004/06/10 14:43:06;  author: mwe;  state: Exp;  lines: +6 -2
 * usage of ntype* instead of type added
 * ----------------------------
 * revision 1.1
 * date: 2003/04/26 20:58:44;  author: mwe;  state: Exp;
 * Initial revision
 */

#include <stdio.h>
#include <stdlib.h>

#include "tree_basic.h"
#include "node_basic.h"
#include "globals.h"
#include "tree_compound.h"
#include "traverse.h"
#include "new_types.h"
#include "dbug.h"
#include "internal_lib.h"
#include "optimize.h"
#include "free.h"
#include "DataFlowMask.h"
#include "DupTree.h"
#include "SSATransform.h"

#include "UndoElimSubDiv.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *postassign;
    node *let;
    bool topdown;
};

/*
 * INFO macros
 */
#define INFO_UESD_FUNDEF(n) (n->fundef)
#define INFO_UESD_POSTASSIGN(n) (n->postassign)
#define INFO_UESD_LET(n) (n->let)
#define INFO_UESD_TOPDOWN(n) (n->topdown)
/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_UESD_FUNDEF (result) = NULL;
    INFO_UESD_POSTASSIGN (result) = NULL;
    INFO_UESD_LET (result) = NULL;
    INFO_UESD_TOPDOWN (result) = TRUE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

static node *
CheckId (node *id, prf op)
{
    node *result = NULL;

    DBUG_ENTER ("CheckId");

    if (AVIS_SSAASSIGN (ID_AVIS (id)) != NULL) {

        node *assign = AVIS_SSAASSIGN (ID_AVIS (id));
        prf prfop;
        bool correctop = FALSE;

        switch (op) {
        case F_add_SxS:
        case F_add_AxS:
        case F_add_SxA:
        case F_add_AxA:
            prfop = F_esd_neg;
            correctop = TRUE;
            break;
        case F_mul_SxS:
        case F_mul_AxS:
        case F_mul_SxA:
        case F_mul_AxA:
            prfop = F_esd_rec;
            correctop = TRUE;
            break;
        default:
            correctop = FALSE;
            break;
        }

        if ((correctop) && (N_let == NODE_TYPE (ASSIGN_INSTR (assign)))
            && (N_prf == NODE_TYPE (LET_EXPR (ASSIGN_INSTR (assign))))) {

            if (PRF_PRF (LET_EXPR (ASSIGN_INSTR (assign))) == prfop) {
                result = PRF_ARGS (LET_EXPR (ASSIGN_INSTR (assign)));
            }
        }
    }

    DBUG_RETURN (result);
}

static prf
TogglePrf (prf op)
{
    prf result;
    DBUG_ENTER ("TogglePrf");

    if (F_add_SxS == op) {
        result = F_sub_SxS;
    }
    if (F_add_SxA == op) {
        result = F_sub_SxA;
    }
    if (F_add_AxS == op) {
        result = F_sub_AxS;
    }
    if (F_add_AxA == op) {
        result = F_sub_AxA;
    }

    if (F_mul_SxS == op) {
        result = F_div_SxS;
    }
    if (F_mul_SxA == op) {
        result = F_div_SxA;
    }
    if (F_mul_AxS == op) {
        result = F_div_AxS;
    }
    if (F_mul_AxA == op) {
        result = F_div_AxA;
    }

    DBUG_RETURN (result);
}

node *
UESDdoUndoElimSubDiv (node *fundef)
{
    info *info;

    DBUG_ENTER ("UESDdoUndoElimSubDiv");

    if (fundef != NULL) {
        DBUG_PRINT ("OPT",
                    ("starting undo elim sub div in function %s", FUNDEF_NAME (fundef)));

        info = MakeInfo ();

        INFO_UESD_FUNDEF (info) = fundef;

        TRAVpush (TR_uesd);
        FUNDEF_BODY (fundef) = TRAVdo (FUNDEF_BODY (fundef), info);
        TRAVpop ();

        info = FreeInfo (info);
    }

    DBUG_RETURN (fundef);
}

node *
UESDblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("UESDblock");

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
UESDassign (node *arg_node, info *arg_info)
{
    node *postassign = NULL;

    DBUG_ENTER ("UESDassign");

    INFO_UESD_TOPDOWN (arg_info) = TRUE;
    INFO_UESD_POSTASSIGN (arg_info) = NULL;

    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }

    postassign = INFO_UESD_POSTASSIGN (arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    INFO_UESD_TOPDOWN (arg_info) = FALSE;

    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }

    if (postassign != NULL) {
        ASSIGN_NEXT (postassign) = arg_node;
        arg_node = postassign;
    }

    DBUG_RETURN (arg_node);
}

node *
UESDlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("UESDlet");

    INFO_UESD_LET (arg_info) = arg_node;

    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
UESDprf (node *arg_node, info *arg_info)
{
    prf op, newop;
    node *id1, *id2;
    bool add = FALSE;
    ntype *type;

    DBUG_ENTER ("UESDprf");

    op = PRF_PRF (arg_node);

    if (INFO_UESD_TOPDOWN (arg_info)) {
        switch (op) {

        case F_add_SxS:
        case F_add_AxS:
        case F_add_SxA:
        case F_add_AxA:
            add = TRUE;

        case F_mul_SxS:
        case F_mul_AxS:
        case F_mul_SxA:
        case F_mul_AxA:
            /*
             * handel add and div seperatly
             */
            id1 = CheckId (EXPRS_EXPR (PRF_ARGS (arg_node)), op);
            id2 = CheckId (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))), op);

            if ((id1 == NULL) && (id2 == NULL)) {
                /*
                 * nothing to do
                 */
            } else if ((id1 == NULL) && (id2 != NULL)) {
                EXPRS_NEXT (PRF_ARGS (arg_node))
                  = FREEdoFreeNode (EXPRS_NEXT (PRF_ARGS (arg_node)));
                EXPRS_NEXT (PRF_ARGS (arg_node)) = DUPdoDupTree (id2);
                PRF_PRF (arg_node) = TogglePrf (op);

            } else if ((id1 != NULL) && (id2 == NULL)) {
                node *tmp = EXPRS_NEXT (PRF_ARGS (arg_node));
                EXPRS_NEXT (PRF_ARGS (arg_node)) = NULL;
                PRF_ARGS (arg_node) = FREEdoFreeNode (PRF_ARGS (arg_node));
                PRF_ARGS (arg_node) = tmp;
                EXPRS_NEXT (PRF_ARGS (arg_node)) = DUPdoDupTree (id1);
                PRF_PRF (arg_node) = TogglePrf (op);
            } else if ((id1 != NULL) && (id2 != NULL)) {
                node *avis;
                node *tmp = DUPdoDupTree (id1);
                EXPRS_NEXT (tmp) = DUPdoDupTree (id2);
                tmp = TBmakePrf (op, tmp);
                avis = TBmakeAvis (ILIBtmpVar (),
                                   TYcopyType (AVIS_TYPE (
                                     IDS_AVIS (LET_IDS (INFO_UESD_LET (arg_info))))));
                BLOCK_VARDEC (FUNDEF_BODY (INFO_UESD_FUNDEF (arg_info)))
                  = TBmakeVardec (avis, BLOCK_VARDEC (
                                          FUNDEF_BODY (INFO_UESD_FUNDEF (arg_info))));

                AVIS_DECL (avis)
                  = BLOCK_VARDEC (FUNDEF_BODY (INFO_UESD_FUNDEF (arg_info)));

                tmp = TCmakeAssignLet (avis, tmp);

                /*
                 * change current prf
                 */
                PRF_ARGS (arg_node) = FREEdoFreeTree (PRF_ARGS (arg_node));
                PRF_ARGS (arg_node)
                  = TBmakeExprs (DUPdupIdsId (LET_IDS (ASSIGN_INSTR (tmp))), NULL);

                if (add) {
                    PRF_PRF (arg_node) = F_esd_neg;
                } else {
                    PRF_PRF (arg_node) = F_esd_rec;
                }

                INFO_UESD_POSTASSIGN (arg_info) = tmp;
            }

            break;

        default:
            break;
        }
    } else {

        /*
         * bottom-up traversal
         */

        type = TYcopyType (AVIS_TYPE (IDS_AVIS (LET_IDS (INFO_UESD_LET (arg_info)))));

        if (PRF_PRF (arg_node) == F_esd_neg) {

            if (TYisAUD (type)) {

                newop = F_sub_SxA;
            } else if ((TYisAUDGZ (type)) || (TYgetDim (type) > 0)) {
                newop = F_sub_SxA;
            } else {
                newop = F_sub_SxS;
            }
            PRF_PRF (arg_node) = newop;
            if (((TYisArray (type)) && (T_int == TYgetSimpleType (TYgetScalar (type))))
                || ((!TYisArray (type)) && (T_int == TYgetSimpleType (type)))) {
                PRF_ARGS (arg_node) = TBmakeExprs (TBmakeNum (0), PRF_ARGS (arg_node));
            } else if (((TYisArray (type))
                        && (T_float == TYgetSimpleType (TYgetScalar (type))))
                       || ((!TYisArray (type)) && (T_float == TYgetSimpleType (type)))) {
                PRF_ARGS (arg_node)
                  = TBmakeExprs (TBmakeFloat (0.0), PRF_ARGS (arg_node));
            } else if (((TYisArray (type))
                        && (T_double == TYgetSimpleType (TYgetScalar (type))))
                       || ((!TYisArray (type)) && (T_double == TYgetSimpleType (type)))) {
                PRF_ARGS (arg_node)
                  = TBmakeExprs (TBmakeDouble (0.0), PRF_ARGS (arg_node));
            } else {
                DBUG_ASSERT ((FALSE), "unexpected simpletype");
            }
        }

        if (PRF_PRF (arg_node) == F_esd_rec) {

            if (TYisAUD (type)) {

                newop = F_div_SxA;
            } else if ((TYisAUDGZ (type)) || (TYgetDim (type) > 0)) {
                newop = F_div_SxA;
            } else {
                newop = F_div_SxS;
            }

            PRF_PRF (arg_node) = newop;
            if (((TYisArray (type)) && (T_int == TYgetSimpleType (TYgetScalar (type))))
                || ((!TYisArray (type)) && (T_int == TYgetSimpleType (type)))) {
                PRF_ARGS (arg_node) = TBmakeExprs (TBmakeNum (1), PRF_ARGS (arg_node));
            } else if (((TYisArray (type))
                        && (T_float == TYgetSimpleType (TYgetScalar (type))))
                       || ((!TYisArray (type)) && (T_float == TYgetSimpleType (type)))) {
                PRF_ARGS (arg_node)
                  = TBmakeExprs (TBmakeFloat (1.0), PRF_ARGS (arg_node));
            } else if (((TYisArray (type))
                        && (T_double == TYgetSimpleType (TYgetScalar (type))))
                       || ((!TYisArray (type)) && (T_double == TYgetSimpleType (type)))) {
                PRF_ARGS (arg_node)
                  = TBmakeExprs (TBmakeDouble (1.0), PRF_ARGS (arg_node));
            } else {
                DBUG_ASSERT ((FALSE), "unexpected simpletype");
            }
        }
    }

    DBUG_RETURN (arg_node);
}
