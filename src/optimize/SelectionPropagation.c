#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "dbug.h"
#include "globals.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "SelectionPropagation.h"
#include "optimize.h"
#include "print.h"

#define INFO_SP_FLAGS(n) (n->flag)
#define INFO_SP_POSSIBLE(n) ((bool)(n->counter))
#define INFO_SP_FUNDEF(n) (n->node[0])

#define SP_DIRDOWN 1
#define SP_MAPOP 2
#define SP_SEL 4

#define SP_IMPOSSIBLE -1
#define SP_POSSIBLE 0

/* internal functions for traversing ids like nodes */
static ids *TravLeftIDS (ids *arg_ids, node *arg_info);
static ids *SPleftids (ids *arg_ids, node *arg_info);
static ids *TravRightIDS (ids *arg_ids, node *arg_info);
static ids *SPrightids (ids *arg_ids, node *arg_info);

bool
isMapOp (prf Prf)
{
    return ((Prf == F_add_AxS) || (Prf == F_add_SxA) || (Prf == F_add_AxA)
            || (Prf == F_sub_AxS) || (Prf == F_sub_SxA) || (Prf == F_sub_AxA)
            || (Prf == F_mul_AxS) || (Prf == F_mul_SxA) || (Prf == F_mul_AxA)
            || (Prf == F_div_AxS) || (Prf == F_div_SxA) || (Prf == F_div_AxA));
}

bool
isLeftArray (prf Prf)
{
    return ((Prf == F_add_AxS) || (Prf == F_add_AxA) || (Prf == F_sub_AxS)
            || (Prf == F_sub_AxA) || (Prf == F_mul_AxS) || (Prf == F_mul_AxA)
            || (Prf == F_div_AxS) || (Prf == F_div_AxA));
}

bool
isRightArray (prf Prf)
{
    return ((Prf == F_add_SxA) || (Prf == F_add_AxA) || (Prf == F_sub_SxA)
            || (Prf == F_sub_AxA) || (Prf == F_mul_SxA) || (Prf == F_mul_AxA)
            || (Prf == F_div_SxA) || (Prf == F_div_AxA));
}

prf
getScalarPrf (prf Prf)
{
    switch (Prf) {
    case F_add_AxS:
    case F_add_SxA:
    case F_add_AxA:
        return F_add_SxS;

    case F_sub_AxS:
    case F_sub_SxA:
    case F_sub_AxA:
        return F_sub_SxS;

    case F_mul_AxS:
    case F_mul_SxA:
    case F_mul_AxA:
        return F_mul_SxS;

    case F_div_AxS:
    case F_div_SxA:
    case F_div_AxA:
        return F_div_SxS;

    default:
        printf ("%s", prf_string[Prf]);
        DBUG_ASSERT (FALSE, "Invalid PRF");
    }
}

node *
selectOrIdentity (node *arg, node *index, bool isArray, types *t, node *arg_info)
{
    node *vardec = NULL;
    node *tmp;

    vardec = MakeVardec (TmpVar (), DupOneTypes (t), NULL);

    AddVardecs (INFO_SP_FUNDEF (arg_info), vardec);

    tmp = MakeAssignLet (StringCopy (VARDEC_NAME (vardec)), vardec,
                         isArray ? MakePrf2 (F_sel, DupNode (index), DupNode (arg))
                                 : DupNode (arg));

    AVIS_SSAASSIGN (VARDEC_AVIS (vardec)) = tmp;

    return tmp;
}

node *
propagateSelection (node *arg_node, node *arg_info)
{

    node *origop;
    node *left_sel;
    node *right_sel;
    node *lid;
    node *rid;

    DBUG_ENTER ("propagateSelection");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_assign, "node type must be N_assing");
    DBUG_ASSERT (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_let, "node type must be N_let");
    DBUG_ASSERT (((NODE_TYPE (ASSIGN_RHS (arg_node)) == N_prf)
                  && (PRF_PRF (ASSIGN_RHS (arg_node)) == F_sel)),
                 "No selection found!!");
    DBUG_ASSERT (ID_SSAASSIGN (PRF_ARG2 (ASSIGN_RHS (arg_node))) != NULL,
                 "ID must be defined in an N_assign-node");
    DBUG_ASSERT (NODE_TYPE (ASSIGN_RHS (ID_SSAASSIGN (PRF_ARG2 (ASSIGN_RHS (arg_node)))))
                   == N_prf,
                 "ID should reference an prf.");

    origop = ASSIGN_RHS (ID_SSAASSIGN (PRF_ARG2 (ASSIGN_RHS (arg_node))));

    left_sel = selectOrIdentity (PRF_ARG1 (origop), PRF_ARG1 (ASSIGN_RHS (arg_node)),
                                 isLeftArray (PRF_PRF (origop)),
                                 IDS_TYPE (ASSIGN_LHS (arg_node)), arg_info);

    right_sel = selectOrIdentity (PRF_ARG2 (origop), PRF_ARG1 (ASSIGN_RHS (arg_node)),
                                  isRightArray (PRF_PRF (origop)),
                                  IDS_TYPE (ASSIGN_LHS (arg_node)), arg_info);

    FreeNode (ASSIGN_RHS (arg_node));

    lid = MakeId (StringCopy (IDS_NAME (ASSIGN_LHS (left_sel))),
                  StringCopy (IDS_MOD (ASSIGN_LHS (left_sel))), ST_regular);

    ID_VARDEC (lid) = IDS_VARDEC (ASSIGN_LHS (left_sel));
    ID_AVIS (lid) = IDS_AVIS (ASSIGN_LHS (left_sel));

    rid = MakeId (StringCopy (IDS_NAME (ASSIGN_LHS (right_sel))),
                  StringCopy (IDS_MOD (ASSIGN_LHS (right_sel))), ST_regular);

    ID_VARDEC (rid) = IDS_VARDEC (ASSIGN_LHS (right_sel));
    ID_AVIS (rid) = IDS_AVIS (ASSIGN_LHS (right_sel));

    ASSIGN_RHS (arg_node) = MakePrf2 (getScalarPrf (PRF_PRF (origop)), lid, rid);

    ASSIGN_NEXT (left_sel) = right_sel;
    ASSIGN_NEXT (right_sel) = arg_node;

    sp_expr++;

    return left_sel;
}

/******************************************************************************
 *
 * function:
 *   node *SPwith(node *arg_node , node *arg_info)
 *
 * description:
 *   traverses withop, code and partitions of withloop
 *
 *****************************************************************************/
node *
SPwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPwith");

    INFO_SP_POSSIBLE (arg_info) = FALSE;

    /* traverse withop */
    if (NWITH_WITHOP (arg_node) != NULL) {
        NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);
    }

    /* traverse part */
    if (NWITH_PART (arg_node) != NULL) {
        NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
    }

    /* traverse code */
    if (NWITH_CODE (arg_node) != NULL) {
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    }

    INFO_SP_POSSIBLE (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPpart(node *arg_node , node *arg_info)
 *
 * description:
 *   traverses generator, withid and next part
 *
 *****************************************************************************/
node *
SPpart (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPpart");

    INFO_SP_POSSIBLE (arg_info) = FALSE;

    /* traverse generator */
    if (NPART_GEN (arg_node) != NULL) {
        NPART_GEN (arg_node) = Trav (NPART_GEN (arg_node), arg_info);
    }

    /* traverse withid */
    if (NPART_WITHID (arg_node) != NULL) {
        NPART_WITHID (arg_node) = Trav (NPART_WITHID (arg_node), arg_info);
    }

    /* traverse next part */
    if (NPART_NEXT (arg_node) != NULL) {
        NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPcode(node *arg_node , node *arg_info)
 *
 * description:
 *   traverses expr, block and next in this order
 *
 *****************************************************************************/
node *
SPcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPcode");

    INFO_SP_POSSIBLE (arg_info) = FALSE;

    /* traverse expression */
    if (NCODE_CEXPR (arg_node) != NULL) {
        NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);
    }

    /* traverse code block */
    if (NCODE_CBLOCK (arg_node) != NULL) {
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    }

    /* traverse expression */
    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SParg(node *arg_node , node *arg_info)
 *
 * description:
 *   removes all args from function signature that have not been used in the
 *   function.
 *
 *
 *****************************************************************************/
node *
SParg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SParg");

    /* traverse to next arg */
    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    AVIS_SELPROP (ARG_AVIS (arg_node)) = SP_IMPOSSIBLE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPap(node *arg_node , node *arg_info)
 *
 * description:
 *
 *****************************************************************************/
node *
SPap (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("SPap");

    INFO_SP_POSSIBLE (arg_info) = FALSE;

    if (INFO_SP_FLAGS (arg_info) & SP_DIRDOWN) {
        if (AP_ARGS (arg_node) != NULL) {
            AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPprf(node *arg_node , node *arg_info)
 *
 * description:
 *
 *****************************************************************************/
node *
SPprf (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPprf");

    if (PRF_PRF (arg_node) == F_sel) {

        INFO_SP_FLAGS (arg_info) |= SP_SEL;

        if (!(INFO_SP_FLAGS (arg_info) & SP_DIRDOWN)) {
            INFO_SP_POSSIBLE (arg_info)
              = ((ID_AVIS (PRF_ARG2 (arg_node)) != NULL)
                 && (AVIS_SSAASSIGN (ID_AVIS (PRF_ARG2 (arg_node))) != NULL)
                 && (NODE_TYPE (
                       ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (PRF_ARG2 (arg_node)))))
                     == N_prf)
                 && (isMapOp (PRF_PRF (
                      ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (PRF_ARG2 (arg_node)))))))
                 && (AVIS_SELPROP (ID_AVIS (PRF_ARG2 (arg_node))) == SP_POSSIBLE));
        }

    } else {
        if (isMapOp (PRF_PRF (arg_node))) {
            INFO_SP_FLAGS (arg_info) |= SP_MAPOP;

            if (INFO_SP_FLAGS (arg_info) & SP_DIRDOWN) {
                PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
            }
        } else {
            INFO_SP_POSSIBLE (arg_info) = FALSE;

            if (INFO_SP_FLAGS (arg_info) & SP_DIRDOWN) {
                if (PRF_ARGS (arg_node) != NULL) {
                    PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
                }
            }
        }
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPlet(node *arg_node , node *arg_info)
 *
 * description:
 *
 *****************************************************************************/
node *
SPlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPlet");

    DBUG_ASSERT ((LET_EXPR (arg_node) != NULL), "let without expression");

    if ((INFO_SP_FLAGS (arg_info) & SP_DIRDOWN)
        && (NODE_TYPE (LET_EXPR (arg_node)) == N_id)) {
        INFO_SP_POSSIBLE (arg_info) = FALSE;
    }

    /* traverse right side of let */
    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    /* traverse left side identifier */
    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TravLeftIDS (LET_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPassign(node *arg_node , node *arg_info)
 *
 * description:
 *
 *****************************************************************************/
node *
SPassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPassign");

    INFO_SP_FLAGS (arg_info) = SP_DIRDOWN;

    INFO_SP_POSSIBLE (arg_info) = TRUE;

    /* traverse instruction */
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    INFO_SP_FLAGS (arg_info) = 0;
    INFO_SP_POSSIBLE (arg_info) = FALSE;

    /* traverse instruction */
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_SP_POSSIBLE (arg_info)) {
        /* Transform! */
        arg_node = propagateSelection (arg_node, arg_info);
        INFO_SP_POSSIBLE (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPreturn(node *arg_node , node *arg_info)
 *
 * description:
 *   starts traversal of return expressions to mark them as needed.
 *
 *****************************************************************************/
node *
SPreturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPreturn");

    INFO_SP_POSSIBLE (arg_info) = FALSE;

    if (RETURN_EXPRS (arg_node) != NULL) {
        RETURN_EXPRS (arg_node) = Trav (RETURN_EXPRS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPid(node *arg_node , node *arg_info)
 *
 * description:
 *   "traverses" the contained ids structure.
 *
 *****************************************************************************/
node *
SPid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPid");

    DBUG_ASSERT ((ID_IDS (arg_node) != NULL), "id without ids");
    ID_IDS (arg_node) = TravRightIDS (ID_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   static ids *SPleftids(ids *arg_ids, node *arg_info)
 *
 * description:
 *
 *****************************************************************************/
static ids *
SPleftids (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("SPleftids");

    if (INFO_SP_FLAGS (arg_info) & SP_DIRDOWN) {
        if (INFO_SP_POSSIBLE (arg_info) == FALSE) {
            AVIS_SELPROP (IDS_AVIS (arg_ids)) = SP_IMPOSSIBLE;
        } else if (INFO_SP_FLAGS (arg_info) & SP_MAPOP) {
            AVIS_SELPROP (IDS_AVIS (arg_ids)) = SP_POSSIBLE;
        }
    }

    /* traverse next ids in ids chain */
    if (IDS_NEXT (arg_ids) != NULL) {
        IDS_NEXT (arg_ids) = TravLeftIDS (IDS_NEXT (arg_ids), arg_info);
    }

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   static ids *SPrightids(ids *arg_ids, node *arg_info)
 *
 * description:
 *
 *****************************************************************************/
static ids *
SPrightids (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("SPrightids");

    if (INFO_SP_FLAGS (arg_info) & SP_DIRDOWN) {
        if (INFO_SP_POSSIBLE (arg_info) == FALSE) {
            AVIS_SELPROP (IDS_AVIS (arg_ids)) = SP_IMPOSSIBLE;
        } else if (INFO_SP_FLAGS (arg_info) & SP_MAPOP) {
            AVIS_SELPROP (IDS_AVIS (arg_ids)) = SP_IMPOSSIBLE;
        }
    }

    /* traverse next ids in ids chain */
    if (IDS_NEXT (arg_ids) != NULL) {
        IDS_NEXT (arg_ids) = TravRightIDS (IDS_NEXT (arg_ids), arg_info);
    }

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   static ids *TravLeftIDS(ids *arg_ids, node *arg_info)
 *
 * description:
 *  implements a similar traversal mechanism like Trav() for IDS chains.
 *  Here for LHS identifier.
 *
 *****************************************************************************/
static ids *
TravLeftIDS (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("TravLeftIDS");

    DBUG_ASSERT (arg_ids != NULL, "traversal in NULL ids");
    arg_ids = SPleftids (arg_ids, arg_info);

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   static ids *TravRightIDS(ids *arg_ids, node *arg_info)
 *
 * description:
 *  implements a similar traversal mechanism like Trav() for IDS chains.
 *  Here for RHS identifier.
 *
 *****************************************************************************/
static ids *
TravRightIDS (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("TravRightIDS");

    DBUG_ASSERT (arg_ids != NULL, "traversal in NULL ids");
    arg_ids = SPrightids (arg_ids, arg_info);

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   node *SelectionPropagation(node *fundef, node *modul)
 *
 * description:
 *   starting point of SelectionPropagation for SSA form.
 *   Starting fundef must not be a special fundef (do, while, cond) created by
 *   lac2fun transformation.
 *
 *****************************************************************************/
node *
SelectionPropagation (node *fundef, node *modul)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("SelectionPropagation");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "SelectionPropagation called for non-fundef node");

    DBUG_PRINT ("OPT", ("starting selection propagation (ssa) in function %s",
                        FUNDEF_NAME (fundef)));

    /* do not start traversal in special functions */
    arg_info = MakeInfo ();

    INFO_SP_FUNDEF (arg_info) = fundef;
    INFO_SP_POSSIBLE (arg_info) = TRUE;
    INFO_SP_FLAGS (arg_info) = SP_DIRDOWN;

    old_tab = act_tab;
    act_tab = sp_tab;

    fundef = Trav (fundef, arg_info);

    act_tab = old_tab;

    arg_info = FreeTree (arg_info);

    DBUG_RETURN (fundef);
}
