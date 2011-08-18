/**
 *
 * @defgroup
 * @ingroup
 *
 * @brief
 * @{
 **/

/** <!--********************************************************************-->
 *
 * @file adjust_stknl_rets.c
 *
 * prefix: CUASR
 *
 * description:
 *
 *****************************************************************************/

#include "adjust_stknl_rets.h"
#include "tree_basic.h"
#include "traverse.h"
#include "str.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "memory.h"
#include "tree_compound.h"
#include "LookUpTable.h"
#include "new_types.h"
#include "type_utils.h"
#include "free.h"

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

    /* Used by anonymous traversal */
    node *at_avis;
    node *at_letids;
};

#define INFO_LUT(n) (n->lut)
#define INFO_LETIDS(n) (n->letids)
#define INFO_FROMAP(n) (n->from_ap)
#define INFO_APIDS(n) (n->apids)
#define INFO_APARGS(n) (n->apargs)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LSNUM(n) (n->lsnum)

#define INFO_AT_AVIS(n) (n->at_avis)
#define INFO_AT_LETIDS(n) (n->at_letids)
/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_LUT (result) = NULL;
    INFO_LETIDS (result) = NULL;
    INFO_FROMAP (result) = FALSE;
    INFO_APIDS (result) = NULL;
    INFO_APARGS (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_LSNUM (result) = 1;

    INFO_AT_AVIS (result) = NULL;
    INFO_AT_LETIDS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

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
GetApArgFromFundefArg (node *arg, node *fundef_args, node *ap_args)
{
    DBUG_ENTER ();

    DBUG_ASSERT (TCcountArgs (fundef_args) == TCcountExprs (ap_args),
                 "Number of arguments and paramenters mismatch!");

    while (fundef_args != NULL) {
        if (fundef_args == arg)
            break;
        fundef_args = ARG_NEXT (fundef_args);
        ap_args = EXPRS_NEXT (ap_args);
    }

    DBUG_RETURN (ap_args);
}

/** <!--********************************************************************-->
 *
 * @fn node *GetRetByLinksign(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
static node *
GetRetByLinksign (node *rets, int linksign)
{
    DBUG_ENTER ();

    while (rets != NULL) {
        if (RET_HASLINKSIGNINFO (rets) && RET_LINKSIGN (rets) == linksign) {
            break;
        }
        rets = RET_NEXT (rets);
    }

    DBUG_RETURN (rets);
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
    DBUG_ENTER ();

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
    DBUG_ENTER ();

    /* Bottom up traversal */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ATravLet(node *arg_node, info *arg_info)
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
static node *
ATravLet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (LET_IDS (arg_node) != NULL
        && IDS_AVIS (LET_IDS (arg_node)) == INFO_AT_AVIS (arg_info)) {
        INFO_AT_LETIDS (arg_info) = LET_IDS (arg_node);
        LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ATravId(node *arg_node, info *arg_info)
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

    DBUG_ENTER ();

    letids = INFO_AT_LETIDS (arg_info);

    if (letids != NULL && TYeqTypes (ID_NTYPE (arg_node), IDS_NTYPE (letids))) {
        INFO_AT_AVIS (arg_info) = ID_AVIS (arg_node);
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

    DBUG_ENTER ();

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
    DBUG_ENTER ();

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
    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);
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
    DBUG_ENTER ();

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

    DBUG_ENTER ();

    old_fundef = INFO_FUNDEF (arg_info);

    if (!FUNDEF_ISCUDASTGLOBALFUN (arg_node)) {
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = old_fundef;
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    } else {
        if (INFO_FROMAP (arg_info)) {
            INFO_LSNUM (arg_info) = 1;
            FUNDEF_RETS (arg_node) = TRAVopt (FUNDEF_RETS (arg_node), arg_info);
            FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
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
 * @fn node *CUASRret(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CUASRret (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    RET_HASLINKSIGNINFO (arg_node) = FALSE;
    RET_LINKSIGN (arg_node) = INFO_LSNUM (arg_info);

    INFO_LSNUM (arg_info) += 1;

    RET_NEXT (arg_node) = TRAVopt (RET_NEXT (arg_node), arg_info);

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
    DBUG_ENTER ();

    ARG_HASLINKSIGNINFO (arg_node) = FALSE;
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
    DBUG_ENTER ();

    AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);

    /* We traverse CUDA ST global function inline */
    if (AP_FUNDEF (arg_node) != NULL && FUNDEF_ISCUDASTGLOBALFUN (AP_FUNDEF (arg_node))) {
        INFO_FROMAP (arg_info) = TRUE;
        INFO_APIDS (arg_info) = INFO_LETIDS (arg_info);
        INFO_APARGS (arg_info) = AP_ARGS (arg_node);
        AP_FUNDEF (arg_node) = TRAVopt (AP_FUNDEF (arg_node), arg_info);
        INFO_APARGS (arg_info) = NULL;
        INFO_APIDS (arg_info) = NULL;
        INFO_FROMAP (arg_info) = FALSE;
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
    node *rets_prev, *apids_prev, *fundef_rets_prev;

    info *anon_info;

    DBUG_ENTER ();

    fundef = INFO_FUNDEF (arg_info);

    if (FUNDEF_ISCUDASTGLOBALFUN (fundef)) {
        ap_args = INFO_APARGS (arg_info);
        fundef_args = FUNDEF_ARGS (fundef);
        fundef_rets = FUNDEF_RETS (fundef);
        apids = INFO_APIDS (arg_info);
        rets = RETURN_EXPRS (arg_node);

        while (rets != NULL) {
            ret_id = EXPRS_EXPR (rets);

            DBUG_ASSERT (!TUisScalar (AVIS_TYPE (ID_AVIS (ret_id))),
                         "Scalar found in N_return!");

            /************ Anonymous Traversal ************/
            anontrav_t atrav[6]
              = {{N_fundef, &ATravFundef}, {N_assign, &ATravAssign}, {N_let, &ATravLet},
                 {N_id, &ATravId},         {N_return, &TRAVnone},    {0, NULL}};

            TRAVpushAnonymous (atrav, &TRAVsons);

            anon_info = MakeInfo ();

            INFO_AT_AVIS (anon_info) = ID_AVIS (ret_id);
            fundef = TRAVdo (fundef, anon_info);
            /*********************************************/

            node *decl = AVIS_DECL (INFO_AT_AVIS (anon_info));

            DBUG_ASSERT (NODE_TYPE (decl) == N_arg, "Declaration of avis is not N_arg!");

            /* Note that this block of code has been moved here just
             * to remove the warning message in the masterrun. However,
             * I don't it's the right place to put the code. Further
             * investigation is required */
            rets_prev = rets;
            apids_prev = apids;
            fundef_rets_prev = fundef_rets;
            /*******************************************************/

            if (!ARG_HASLINKSIGNINFO (decl)) {
                ARG_HASLINKSIGNINFO (decl) = TRUE;
                RET_HASLINKSIGNINFO (fundef_rets) = TRUE;
                RET_LINKSIGN (fundef_rets) = ARG_LINKSIGN (decl);

                printf ("[%s] setting linksign of ret to %d\n",
                        FUNDEF_NAME (INFO_FUNDEF (arg_info)), RET_LINKSIGN (fundef_rets));

                ap_arg = GetApArgFromFundefArg (decl, fundef_args, ap_args);

                old_avis = IDS_AVIS (apids);
                IDS_AVIS (apids) = ID_AVIS (EXPRS_EXPR (ap_arg));

                INFO_LUT (arg_info)
                  = LUTinsertIntoLutP (INFO_LUT (arg_info), old_avis, IDS_AVIS (apids));

                /* This block of code has been moved to the begining of the if
                 * rets_prev = rets;
                 * apids_prev = apids;
                 * fundef_rets_prev = fundef_rets
                 */

                rets = EXPRS_NEXT (rets);
                apids = IDS_NEXT (apids);
                fundef_rets = RET_NEXT (fundef_rets);
            } else {
                node *tmp_ret
                  = GetRetByLinksign (FUNDEF_RETS (fundef), ARG_LINKSIGN (decl));

                DBUG_ASSERT (tmp_ret != NULL,
                             "Found linksigned N_arg with no corresponding N_ret!");

                ARG_LINKSIGN (decl)--;
                RET_LINKSIGN (tmp_ret)--;

                ap_arg = GetApArgFromFundefArg (decl, fundef_args, ap_args);

                old_avis = IDS_AVIS (apids);

                INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), old_avis,
                                                         ID_AVIS (EXPRS_EXPR (ap_arg)));

                rets = EXPRS_NEXT (rets_prev) = FREEdoFreeNode (rets);
                apids = IDS_NEXT (apids_prev) = FREEdoFreeNode (apids);
                fundef_rets = RET_NEXT (fundef_rets_prev) = FREEdoFreeNode (fundef_rets);
            }

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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

    new_avis = LUTsearchInLutPp (INFO_LUT (arg_info), IDS_AVIS (arg_node));

    if (new_avis != IDS_AVIS (arg_node)) {
        IDS_AVIS (arg_node) = new_avis;
    }

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
