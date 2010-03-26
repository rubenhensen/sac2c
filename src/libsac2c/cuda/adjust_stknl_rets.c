/**
 *
 * @defgroup crece Create Cells
 * @ingroup muth
 *
 * @brief creates initial cells
 * @{
 **/

/** <!--********************************************************************-->
 *
 * @file cuda_create_cells.c
 *
 * prefix: CRECE
 *
 * description:
 *   creates a seperate cell around each first assignment of a CELLID, which
 *   is MUTH_EXCLUSIVE, MUTH_SINGLE or MUTH_MULTI tagged
 *
 *****************************************************************************/

#include "adjust_stknl_rets.h"
#include "tree_basic.h"
#include "traverse.h"
#include "str.h"
#include "dbug.h"
#include "memory.h"
#include "tree_compound.h"
#include "LookUpTable.h"
#include "new_types.h"

/*
 * INFO structure
 */
struct INFO {
    lut_t *lut;
    node *letids;
    bool from_ap;
    node *apids;
    node *apargs;
    node *fundef;
    int lsnum;

    lut_t *at_lut;
    node *at_letids;
};

#define INFO_LUT(n) (n->lut)
#define INFO_LETIDS(n) (n->letids)
#define INFO_FROM_AP(n) (n->from_ap)
#define INFO_APIDS(n) (n->apids)
#define INFO_APARGS(n) (n->apargs)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LSNUM(n) (n->lsnum)

#define INFO_AT_LUT(n) ((n)->at_lut)
#define INFO_AT_LETIDS(n) (n->at_letids)
/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_LUT (result) = NULL;
    INFO_LETIDS (result) = NULL;
    INFO_FROM_AP (result) = FALSE;
    INFO_APIDS (result) = NULL;
    INFO_APARGS (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_LSNUM (result) = 1;

    INFO_AT_LUT (result) = LUTgenerateLut ();
    INFO_AT_LETIDS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *GetApArgFromFundefArg(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
static node *
GetApArgFromFundefArg (node *fundef_arg, node *fundef_args, node *ap_args)
{
    DBUG_ENTER ("GetApArgFromFundefArg");

    while (fundef_args != NULL && ap_args != NULL) {
        if (fundef_args == fundef_arg)
            break;
        fundef_args = ARG_NEXT (fundef_args);
        ap_args = EXPRS_NEXT (ap_args);
    }

    DBUG_RETURN (ap_args);
}

/** <!--********************************************************************-->
 *
 * @fn node *GetDeclFromRet(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
static node *
GetDeclFromRet (node *avis, info *arg_info)
{
    lut_t *lut;
    node *rhs_avis;

    DBUG_ENTER ("GetDeclFromRet");

    lut = INFO_AT_LUT (arg_info);

    rhs_avis = LUTsearchInLutPp (lut, avis);

    printf ("hshsha: %s\n", AVIS_NAME (avis));

    while (NODE_TYPE (rhs_avis) != N_empty) {

        printf ("jajjajajajajajaj\n");

        DBUG_ASSERT (rhs_avis != avis, "Avis not found in LUT!");
        avis = rhs_avis;
        rhs_avis = LUTsearchInLutPp (lut, avis);
    }

    DBUG_ASSERT (NODE_TYPE (AVIS_DECL (avis)) == N_arg,
                 "Declaration of avis is not N_arg!");

    DBUG_RETURN (AVIS_DECL (avis));
}

/** <!--********************************************************************-->
 *
 * @fn node *ATravFundef(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
static node *
ATravFundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ATravFundef");

    DBUG_ASSERT (FUNDEF_ISCUDASTGLOBALFUN (arg_node),
                 "N_fundef must be a cudast function!");

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ATravAssign(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
static node *
ATravAssign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ATravAssign");

    /* Bottom up traversal */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ATravLet(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
static node *
ATravLet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ATravLet");

    INFO_AT_LETIDS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);
    INFO_AT_LETIDS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ATravFundef(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
static node *
ATravId (node *arg_node, info *arg_info)
{
    node *letids;
    node *search_res;

    DBUG_ENTER ("ATravId");

    letids = INFO_AT_LETIDS (arg_info);
    if (letids != NULL) {
        search_res = LUTsearchInLutPp (INFO_AT_LUT (arg_info), IDS_AVIS (letids));
        if (ID_AVIS (arg_node) != IDS_AVIS (letids)
            && TYeqTypes (AVIS_TYPE (ID_AVIS (arg_node)), AVIS_TYPE (IDS_AVIS (letids)))
            && NODE_TYPE (search_res) == N_empty) {
            printf ("Inserting pair: %s->%s\n", AVIS_NAME (IDS_AVIS (letids)),
                    AVIS_NAME (ID_AVIS (arg_node)));
            INFO_AT_LUT (arg_info)
              = LUTupdateLutP (INFO_AT_LUT (arg_info), IDS_AVIS (letids),
                               ID_AVIS (arg_node), NULL);
            printf ("Inserting pair: %s->N_empty\n", AVIS_NAME (ID_AVIS (arg_node)));
            INFO_AT_LUT (arg_info)
              = LUTinsertIntoLutP (INFO_AT_LUT (arg_info), ID_AVIS (arg_node),
                                   TBmakeEmpty ());
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUASRdoAdjustStknlRets(node *syntax_tree)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CUASRdoAdjustStknlRets (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ("CUASRdoAdjustStknlRets");

    arg_info = MakeInfo ();

    TRAVpush (TR_cuasr);

    syntax_tree = TRAVdo (syntax_tree, arg_info);

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUASRmodule(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CUASRmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUASRmodule");

    INFO_LUT (arg_info) = LUTgenerateLut ();
    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);
    INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUASRassign(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CUASRassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUASRassign");

    ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUASRlet(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CUASRlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUASRlet");

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);
    INFO_LETIDS (arg_info) = NULL;
    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUASRfundef(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CUASRfundef (node *arg_node, info *arg_info)
{
    node *old_fundef;

    DBUG_ENTER ("CUASRfundef");

    old_fundef = INFO_FUNDEF (arg_info);

    if (!FUNDEF_ISCUDASTGLOBALFUN (arg_node)) {
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = old_fundef;
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    } else {
        if (INFO_FROM_AP (arg_info)) {
            INFO_LSNUM (arg_info) = 1;
            FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
            INFO_LSNUM (arg_info) = 1;
            INFO_FUNDEF (arg_info) = arg_node;
            FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
            INFO_FUNDEF (arg_info) = old_fundef;
        } else {
            FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUASRarg(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CUASRarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUASRarg");

    ARG_HASLINKSIGNINFO (arg_node) = TRUE;
    ARG_LINKSIGN (arg_node) = INFO_LSNUM (arg_info);

    INFO_LSNUM (arg_info) += 1;

    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUASRap(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CUASRap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUASRap");

    if (AP_FUNDEF (arg_node) != NULL && FUNDEF_ISCUDASTGLOBALFUN (AP_FUNDEF (arg_node))) {
        INFO_FROM_AP (arg_info) = TRUE;
        INFO_APIDS (arg_info) = INFO_LETIDS (arg_info);
        INFO_APARGS (arg_info) = AP_ARGS (arg_node);
        AP_FUNDEF (arg_node) = TRAVopt (AP_FUNDEF (arg_node), arg_info);
        INFO_APARGS (arg_info) = NULL;
        INFO_APIDS (arg_info) = NULL;
        INFO_FROM_AP (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUASRreturn(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CUASRreturn (node *arg_node, info *arg_info)
{
    node *fundef, *ap_arg, *ap_args, *fundef_args, *apids, *rets;
    node *fundef_rets;
    node *ret_id;
    node *old_avis;

    info *anon_info;

    DBUG_ENTER ("CUASRreturn");

    fundef = INFO_FUNDEF (arg_info);

    if (FUNDEF_ISCUDASTGLOBALFUN (fundef)) {
        ap_args = INFO_APARGS (arg_info);
        fundef_args = FUNDEF_ARGS (fundef);
        fundef_rets = FUNDEF_RETS (fundef);
        apids = INFO_APIDS (arg_info);
        rets = RETURN_EXPRS (arg_node);

        while (rets != NULL) {
            ret_id = EXPRS_EXPR (rets);

            DBUG_ASSERT (NODE_TYPE (ret_id) == N_id, "Non N_id node found in N_return!");
            DBUG_ASSERT (!TYisScalar (AVIS_TYPE (ID_AVIS (ret_id))),
                         "Scalar found in N_return!");

            /************ Anonymous Traversal ************/
            anontrav_t atrav[6]
              = {{N_fundef, &ATravFundef}, {N_assign, &ATravAssign}, {N_id, &ATravId},
                 {N_let, &ATravLet},       {N_return, &TRAVnone},    {0, NULL}};

            TRAVpushAnonymous (atrav, &TRAVsons);

            anon_info = MakeInfo ();

            INFO_AT_LUT (anon_info)
              = LUTinsertIntoLutP (INFO_AT_LUT (anon_info), ID_AVIS (ret_id),
                                   TBmakeEmpty ());

            fundef = TRAVdo (fundef, anon_info);
            /*********************************************/

            node *ret_decl = GetDeclFromRet (ID_AVIS (ret_id), anon_info);

            /* if( NODE_TYPE( AVIS_DECL( ID_AVIS( ret_id))) == N_arg) {
              ap_arg = GetApArgFromFundefArg( AVIS_DECL( ID_AVIS( ret_id)),
                                              fundef_args, ap_args);
             */
            ap_arg = GetApArgFromFundefArg (ret_decl, fundef_args, ap_args);

            RET_HASLINKSIGNINFO (fundef_rets) = TRUE;
            RET_LINKSIGN (fundef_rets) = ARG_LINKSIGN (ret_decl);

            old_avis = IDS_AVIS (apids);
            IDS_AVIS (apids) = ID_AVIS (EXPRS_EXPR (ap_arg));

            INFO_LUT (arg_info)
              = LUTinsertIntoLutP (INFO_LUT (arg_info), old_avis, IDS_AVIS (apids));
            /* } */

            rets = EXPRS_NEXT (rets);
            apids = IDS_NEXT (apids);
            fundef_rets = RET_NEXT (fundef_rets);

            /************ Anonymous Traversal ************/
            anon_info = FreeInfo (anon_info);
            TRAVpop ();
            /*********************************************/
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUASRid(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CUASRid (node *arg_node, info *arg_info)
{
    node *new_avis;

    DBUG_ENTER ("CUASRid");

    new_avis = LUTsearchInLutPp (INFO_LUT (arg_info), ID_AVIS (arg_node));

    if (new_avis != ID_AVIS (arg_node)) {
        ID_AVIS (arg_node) = new_avis;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUASRids(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CUASRids (node *arg_node, info *arg_info)
{
    node *new_avis;

    DBUG_ENTER ("CUASRids");

    new_avis = LUTsearchInLutPp (INFO_LUT (arg_info), IDS_AVIS (arg_node));

    if (new_avis != IDS_AVIS (arg_node)) {
        IDS_AVIS (arg_node) = new_avis;
    }

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}
