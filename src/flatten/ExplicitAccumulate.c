/*
 *
 * $Log$
 * Revision 1.6  2004/11/24 12:47:20  khf
 * replaced WITH_CEXPR
 *
 * Revision 1.5  2004/11/23 21:50:47  khf
 * SacDefCamp04: Compiles
 *
 * Revision 1.4  2004/11/11 19:01:51  khf
 * fixed bug 83
 *
 * Revision 1.3  2004/08/09 13:14:17  khf
 * some comments added
 *
 * Revision 1.2  2004/07/23 13:35:46  khf
 * F_accu contains no longer the neutral elements of fold operators
 *
 * Revision 1.1  2004/07/21 12:35:31  khf
 * Initial revision
 *
 *
 *
 */

/**
 *
 * @file ExplicitAccumulate.c
 *
 * In this traversal fold functions of fold withloops
 * are become explicit and a new function F_accu is inserted
 * in code.
 *
 * Ex.:
 *    A = with(iv)
 *          gen:{ val = ...;
 *              }: val
 *        fold( op, n);
 *
 * is transformed into
 *
 *    A = with(iv)
 *          gen:{ acc   = accu( iv);
 *                val = ...;
 *                res = op( acc, val);
 *              }: res
 *        fold( op, n);
 *
 * The function F_accu is used to get the correct
 * accumulation value but is only pseudo syntax and is kicked
 * off the code in compile. The only argument is the index vector
 * of the surrounding withloop to disable LIR.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "new_types.h"
#include "tree_basic.h"
#include "node_basic.h"
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
    node *ids;
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

    result = ILIBmalloc (sizeof (info));

    INFO_EA_FUNDEF (result) = NULL;
    INFO_EA_WL (result) = NULL;
    INFO_EA_LHS_IDS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *MakeAccuAssign( node *fundef, node *lhs_ids, node *idx_vec)
 *
 *   @brief creates new assignment containing prf F_accu
 *
 *   @param  node *fundef  :  N_fundef of current function
 *           node *lhs_ids :  LHS N_ids of current withloop
 *           node *idx_vex :  index vector of current withloop
 *   @return node *        :  new N_assign node
 ******************************************************************************/
static node *
MakeAccuAssign (node *fundef, node *lhs_ids, node *idx_vec)
{
    node *vardec, *tmp, *nassign, *_ids;
    char *nvarname;

    DBUG_ENTER ("MakeAccuAssign");

    nvarname = ILIBtmpVarName (IDS_NAME (lhs_ids));
    _ids = TBmakeIds (TBmakeAvis (nvarname, TYcopyType (AVIS_TYPE (IDS_AVIS (lhs_ids)))),
                      NULL);

    vardec = TBmakeVardec (IDS_AVIS (_ids), NULL);

    fundef = TCaddVardecs (fundef, vardec);

    /* F_accu( <idx-varname>) */
    tmp = TCmakePrf1 (F_accu, DUPdupIdsId (idx_vec));

    nassign = TBmakeAssign (TBmakeLet (tmp, _ids), NULL);

    /* set correct backref to defining assignment */
    AVIS_SSAASSIGN (IDS_AVIS (_ids)) = nassign;

    DBUG_RETURN (nassign);
}

/** <!--********************************************************************-->
 *
 * @fn node *MakeFoldFunAssign( node *fundef, node *withop, node *accu_ids,
 *                              node *cexpr);
 *
 *   @brief  creates new assignment containing fold function of withloop
 *
 *   @param  node *fundef  :  N_fundef of current function
 *           node *withop  :  N_withop of current withloop
 *           node *accu_ids:  lhs of accu assign
 *           node *cexpr   :  cexpr of current ncode
 *   @return node *        :  new N_assign node
 ******************************************************************************/
static node *
MakeFoldFunAssign (node *fundef, node *withop, node *accu_ids, node *cexpr)
{
    node *vardec, *funap, *nassign, *_ids;
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
    DBUG_ASSERT ((NODE_TYPE (withop) == N_fold), "withop must be a N_fold node!");

    nvarname = ILIBtmpVarName (ID_NAME (cexpr));
    _ids = TBmakeIds (TBmakeAvis (nvarname, TYcopyType (AVIS_TYPE (IDS_AVIS (accu_ids)))),
                      NULL);

    vardec = TBmakeVardec (IDS_AVIS (_ids), NULL);

    fundef = TCaddVardecs (fundef, vardec);

    funap = TCmakeAp2 (ILIBstringCopy (FOLD_FUN (withop)), FOLD_MOD (withop),
                       DUPdupIdsId (accu_ids), DUPdoDupNode (cexpr));

    AP_FUNDEF (funap) = FOLD_FUNDEF (withop);

    nassign = TBmakeAssign (TBmakeLet (funap, _ids), NULL);

    /* set correct backref to defining assignment */
    AVIS_SSAASSIGN (IDS_AVIS (_ids)) = nassign;

    DBUG_RETURN (nassign);
}

/** <!--********************************************************************-->
 *
 * @fn node *InsertAccuPrf( node *ncode, info *arg_info);
 *
 *   @brief
 *
 *   @param  node *ncode    :  N_code of current withloop
 *           node *arg_info :  N_info
 *   @return node *         :  modified N_code
 ******************************************************************************/
static node *
InsertAccuPrf (node *ncode, info *arg_info)
{
    node *wl, *nblock, *accuassign, *ffassign, *nassign, *nid;

    DBUG_ENTER ("InsertAccuPrf");

    wl = INFO_EA_WL (arg_info);
    nblock = CODE_CBLOCK (ncode);

    /* <acc> = F_accu( <idx-varname>); */
    accuassign = MakeAccuAssign (INFO_EA_FUNDEF (arg_info), INFO_EA_LHS_IDS (arg_info),
                                 WITH_VEC (wl));

    /*   <res> = <fun>( <acc>, <cexpr>); */
    ffassign = MakeFoldFunAssign (INFO_EA_FUNDEF (arg_info), WITH_WITHOP (wl),
                                  ASSIGN_LHS (accuassign), EXPRS_EXPR (WITH_CEXPRS (wl)));

    nassign = TCappendAssign (BLOCK_INSTR (nblock), ffassign);

    ASSIGN_NEXT (accuassign) = nassign;

    BLOCK_INSTR (nblock) = accuassign;

    /* replace CEXPR */
    nid = DUPdupIdsId (ASSIGN_LHS (ffassign));

    EXPRS_EXPR (WITH_CEXPRS (wl)) = FREEdoFreeNode (EXPRS_EXPR (WITH_CEXPRS (wl)));
    EXPRS_EXPR (WITH_CEXPRS (wl)) = nid;

    DBUG_RETURN (ncode);
}

/**
 *
 *  TRAVERSAL FUNCTIONS
 *
 * @{
 ****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *EAmodule(node *arg_node, info *arg_info)
 *
 *   @brief traverses function definitions only!
 *
 *   @param  node *arg_node:  N_module
 *           info *arg_info:  info
 *   @return node *        :  N_module
 ******************************************************************************/

node *
EAmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EAmodule");

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EAfundef(node *arg_node, info *arg_info)
 *
 *   @brief  Traverses FUNDEF body
 *
 *   @param  node *arg_node:  N_fundef
 *           info *arg_info:  info
 *   @return node *        :  N_fundef
 ******************************************************************************/

node *
EAfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EAfundef");

    INFO_EA_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node)) {
        FUNDEF_INSTR (arg_node) = TRAVdo (FUNDEF_INSTR (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EAlet(node *arg_node, info *arg_info)
 *
 *   @brief  Traverses in expression
 *
 *   @param  node *arg_node:  N_let
 *           info *arg_info:  info
 *   @return node *        :  N_let
 ******************************************************************************/

node *
EAlet (node *arg_node, info *arg_info)
{
    node *tmp;

    DBUG_ENTER ("EAlet");

    tmp = INFO_EA_LHS_IDS (arg_info);
    INFO_EA_LHS_IDS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    INFO_EA_LHS_IDS (arg_info) = tmp;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EAwith(node *arg_node, info *arg_info)
 *
 *   @brief  if current WL is a fold WL modify code.
 *
 *   @param  node *arg_node:  N_with
 *           info *arg_info:  N_info
 *   @return node *        :  N_with
 ******************************************************************************/

node *
EAwith (node *arg_node, info *arg_info)
{
    info *tmp;

    DBUG_ENTER ("EAwith");

    /* stack arg_info */
    tmp = arg_info;
    arg_info = MakeInfo ();
    INFO_EA_FUNDEF (arg_info) = INFO_EA_FUNDEF (tmp);

    /* modify bottom up */
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    /* pop arg_info */
    arg_info = FreeInfo (arg_info);
    arg_info = tmp;

    if (NODE_TYPE (WITH_WITHOP (arg_node)) == N_fold) {

        DBUG_PRINT ("EA", ("Fold WL found, inserting F_Accu..."));

        INFO_EA_WL (arg_info) = arg_node; /* store the current node for later */

        DBUG_ASSERT ((CODE_NEXT (WITH_CODE (arg_node)) == NULL),
                     "Withloop has more than one N_code!");
        WITH_CODE (arg_node) = InsertAccuPrf (WITH_CODE (arg_node), arg_info);

        DBUG_PRINT ("EA", (" inserting complete"));

    } /* else nothing to do */

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EAcode(node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param  node *arg_node:  N_code
 *           info *arg_info:  N_info
 *   @return node *        :  N_code
 ******************************************************************************/

node *
EAcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EAcode");

    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @}
 */

/** <!--********************************************************************-->
 *
 * @fn node *ExplicitAccumulate( node *arg_node)
 *
 *   @brief  Starting function of ExplicitAccumulate traversal
 *
 *   @param  node *arg_node:  the whole syntax tree
 *   @return node *        :  the transformed syntax tree
 ******************************************************************************/

node *
EAdoExplicitAccumulate (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("EAdoExplicitAccumulate");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module),
                 "ExplicitAccumulate not started with module node");

    DBUG_PRINT ("EA", ("starting ExplicitAccumulation"));

    arg_info = MakeInfo ();

    TRAVpush (TR_ea);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}
