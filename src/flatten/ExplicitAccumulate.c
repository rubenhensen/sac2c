/*
 *
 * $Log$
 * Revision 1.1  2004/07/21 12:35:31  khf
 * Initial revision
 *
 *
 *
 */

#define NEW_INFO

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "new_types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "free.h"
#include "DupTree.h"
#include "Error.h"
#include "globals.h"
#include "dbug.h"
#include "traverse.h"
#include "constants.h"
#include "ExplicitAccumulate.h"

/**
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *wl;
    ids *ids;
};

#define INFO_EA_FUNDEF(n) (n->fundef)
#define INFO_EA_WL(n) (n->wl)
#define INFO_EA_LHS_IDS(n) (n->ids)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_EA_FUNDEF (result) = NULL;
    INFO_EA_WL (result) = NULL;
    INFO_EA_LHS_IDS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *MakeAccuAssign( node *fundef, ids *lhs_ids, ids *idx_vec,
 *                           node *neutral)
 *
 *   @brief
 *
 *   @param  node *expr  :  expr
 *           node *f_def :  N_fundef
 *   @return node *      :  a chained list of N_assign nodes
 ******************************************************************************/
static node *
MakeAccuAssign (node *fundef, ids *lhs_ids, ids *idx_vec, node *neutral)
{
    node *vardec, *tmp, *nassign;
    ids *_ids;
    char *nvarname;

    DBUG_ENTER ("MakeAccuAssign");

    DBUG_ASSERT (((neutral != NULL) && (NODE_TYPE (neutral) == N_id)),
                 "neutral not found or no id");

    nvarname = TmpVarName (IDS_NAME (lhs_ids));
    _ids = MakeIds (nvarname, NULL, ST_regular);
    vardec = MakeVardec (StringCopy (nvarname), DupOneTypes (IDS_TYPE (lhs_ids)), NULL);
    AVIS_TYPE (VARDEC_AVIS (vardec)) = TYCopyType (AVIS_TYPE (IDS_AVIS (lhs_ids)));

    IDS_VARDEC (_ids) = vardec;
    IDS_AVIS (_ids) = VARDEC_AVIS (vardec);

    fundef = AddVardecs (fundef, vardec);

    /* F_accu( <idx-varname>, <neutral-element-of-foldfun>) */
    tmp = MakePrf2 (F_accu, DupIds_Id (idx_vec), DupNode (neutral));

    nassign = MakeAssign (MakeLet (tmp, _ids), NULL);

    /* set correct backref to defining assignment */
    AVIS_SSAASSIGN (IDS_AVIS (_ids)) = nassign;

    DBUG_RETURN (nassign);
}

/** <!--********************************************************************-->
 *
 * @fn node *MakeFoldFunAssign( node *fundef, node *withop, ids *accu_ids,
 *                              node *cexpr);
 *
 *   @brief
 *
 *   @param  node *expr  :  expr
 *           node *f_def :  N_fundef
 *   @return node *      :  a chained list of N_assign nodes
 ******************************************************************************/
static node *
MakeFoldFunAssign (node *fundef, node *withop, ids *accu_ids, node *cexpr)
{
    node *vardec, *funap, *nassign;
    ids *_ids;
    char *nvarname;

    DBUG_ENTER ("MakeFoldFunAssign");

    /*
     * create a function application of the form:
     *    <res> = <fun>( <acc>, <cexpr>);
     * where
     *    <acc>   is the accumulator variable
     *    <fun>   is the name of the (artificially introduced) folding-fun
     *              (can be found via 'NWITHOP_FUN')
     *    <cexpr> is the expression in the operation part
     */

    DBUG_ASSERT ((NODE_TYPE (cexpr) == N_id), "CEXPR must be a N_id node!");

    nvarname = TmpVarName (ID_NAME (cexpr));
    _ids = MakeIds (nvarname, NULL, ST_regular);
    vardec = MakeVardec (StringCopy (nvarname), DupOneTypes (IDS_TYPE (accu_ids)), NULL);
    AVIS_TYPE (VARDEC_AVIS (vardec)) = TYCopyType (AVIS_TYPE (IDS_AVIS (accu_ids)));

    IDS_VARDEC (_ids) = vardec;
    IDS_AVIS (_ids) = VARDEC_AVIS (vardec);

    fundef = AddVardecs (fundef, vardec);

    funap = MakeAp2 (StringCopy (NWITHOP_FUN (withop)), NWITHOP_MOD (withop),
                     DupIds_Id (accu_ids), DupNode (cexpr));

    AP_FUNDEF (funap) = NWITHOP_FUNDEF (withop);

    nassign = MakeAssign (MakeLet (funap, _ids), NULL);

    /* set correct backref to defining assignment */
    AVIS_SSAASSIGN (IDS_AVIS (_ids)) = nassign;

    DBUG_RETURN (nassign);
}

/** <!--********************************************************************-->
 *
 * @fn node *EAmodul(node *arg_node, info *arg_info)
 *
 *   @brief traverses function definitions only!
 *
 *   @param  node *arg_node:  N_modul
 *           info *arg_info:  N_info
 *   @return node *        :  N_modul
 ******************************************************************************/

node *
EAmodul (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EAmodul");

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EAfundef(node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param  node *arg_node:  N_fundef
 *           info *arg_info:  N_info
 *   @return node *        :  N_fundef
 ******************************************************************************/

node *
EAfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EAfundef");

    INFO_EA_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node)) {
        FUNDEF_INSTR (arg_node) = Trav (FUNDEF_INSTR (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EAlet(node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param  node *arg_node:  N_let
 *           info *arg_info:  N_info
 *   @return node *        :  N_let
 ******************************************************************************/

node *
EAlet (node *arg_node, info *arg_info)
{
    ids *tmp;

    DBUG_ENTER ("EAlet");

    tmp = INFO_EA_LHS_IDS (arg_info);
    INFO_EA_LHS_IDS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    INFO_EA_LHS_IDS (arg_info) = tmp;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EANwith(node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param  node *arg_node:  N_Nwith
 *           info *arg_info:  N_info
 *   @return node *        :  N_Nwith
 ******************************************************************************/

node *
EANwith (node *arg_node, info *arg_info)
{
    node *tmp;
    DBUG_ENTER ("EANwith");

    if (NWITH_IS_FOLD (arg_node) && (NWITH_CODE (arg_node) != NULL)) {

        tmp = INFO_EA_WL (arg_info);
        INFO_EA_WL (arg_info) = arg_node; /* store the current node for later */

        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

        INFO_EA_WL (arg_info) = tmp;

    } /* else nothing to do */

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EANcode(node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param  node *arg_node:  N_Ncode
 *           info *arg_info:  N_info
 *   @return node *        :  N_Ncode
 ******************************************************************************/

node *
EANcode (node *arg_node, info *arg_info)
{
    node *wl, *nblock, *accuassign, *ffassign, *nassign, *nid;

    DBUG_ENTER ("EANcode");

    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);

    wl = INFO_EA_WL (arg_info);
    nblock = NCODE_CBLOCK (arg_node);

    accuassign = MakeAccuAssign (INFO_EA_FUNDEF (arg_info), INFO_EA_LHS_IDS (arg_info),
                                 NWITH_VEC (wl), NWITH_NEUTRAL (wl));
    ffassign = MakeFoldFunAssign (INFO_EA_FUNDEF (arg_info), NWITH_WITHOP (wl),
                                  ASSIGN_LHS (accuassign), NWITH_CEXPR (wl));

    nassign = AppendAssign (BLOCK_INSTR (nblock), ffassign);

    ASSIGN_NEXT (accuassign) = nassign;

    BLOCK_INSTR (nblock) = accuassign;

    /* replace CEXPR */
    nid = DupIds_Id (ASSIGN_LHS (ffassign));

    NWITH_CEXPR (wl) = FreeNode (NWITH_CEXPR (wl));
    NWITH_CEXPR (wl) = nid;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ExplicitAccumulate( node *arg_node)
 *
 *   @brief
 *
 *   @param  node *arg_node:  the whole syntax tree
 *   @return node *        :  the transformed syntax tree
 ******************************************************************************/

node *
ExplicitAccumulate (node *arg_node)
{
    funtab *tmp_tab;
    info *arg_info;

    DBUG_ENTER ("ExplicitAccumulate");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_modul),
                 "ExplicitAccumulate not started with modul node");

    DBUG_PRINT ("EA", ("starting ExplicitAccumulation"));

    arg_info = MakeInfo ();

    tmp_tab = act_tab;
    act_tab = ea_tab;

    arg_node = Trav (arg_node, arg_info);

    arg_info = FreeInfo (arg_info);
    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}
