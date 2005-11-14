/* *
 * $Log$
 *
 */

/**<!--******************************************************************-->
 *
 * @file type_upgrade.c
 *
 * This file implements functionality of type upgrade (infer types of lhs
 * dependent on rhs), reverse type upgrade (infer types of rhs dependent
 * on lhs), function dispatch (removes calls of wrapper functions) and
 * function specialization (create more special function instances).
 *
 **************************************************************************/

#include "tree_basic.h"
#include "traverse.h"
#include "node_basic.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "dbug.h"
#include "internal_lib.h"
#include "free.h"
#include "DupTree.h"
#include "globals.h"
#include "type_utils.h"
#include "ct_with.h"
#include "type_errors.h"
#include "ct_prf.h"
#include "ct_with.h"
#include "ct_fun.h"
#include "constants.h"
#include "shape.h"
#include "ct_basic.h"
#include "tree_compound.h"
#include "ctinfo.h"

#include "wlselcount.h"

/*
 * info structure
 */
struct INFO {
    bool wlfunapps;
    bool iswlcode;
    int wlselsmax;
    int wlselcmax;
    int wlsels;
};

#define INFO_WLSELSMAX(n) (n->wlselsmax)
#define INFO_WLSELCMAX(n) (n->wlselcmax)
#define INFO_WLSELS(n) (n->wlsels)
#define INFO_WLFUNAPPS(n) (n->wlfunapps)
#define INFO_ISWLCODE(n) (n->iswlcode)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");
    result = ILIBmalloc (sizeof (info));

    INFO_WLSELSMAX (result) = 0;
    INFO_WLSELCMAX (result) = 0;
    INFO_WLSELS (result) = 0;
    INFO_WLFUNAPPS (result) = 0;
    INFO_ISWLCODE (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

node *
WLSELCdoWithloopSelectionCount (node *fundef)
{
    info *arg_info;

    DBUG_ENTER ("WLSELCdoWithloopSelection");

    arg_info = MakeInfo ();

    TRAVpush (TR_wlselc);
    fundef = TRAVdo (fundef, arg_info);
    TRAVpop ();

    fundef = FREEdoFreeTree (fundef);

    DBUG_RETURN (fundef);
}

/**<!--*************************************************************-->
 *
 * @fn node *WLSELCfundef(node *arg_node, info *arg_info)
 *
 * @brief: handles fundef nodes
 *
 *  <+long description+>
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ********************************************************************/
node *
WLSELCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSELCfundef");

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**<!--*************************************************************-->
 *
 * @fn node *WLSELCblock(node *arg_node, info *arg_info)
 *
 * @brief: handles block nodes
 *
 *  <+long description+>
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ********************************************************************/
node *
WLSELCblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSELCblock");

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**<!--*************************************************************-->
 *
 * @fn node *WLSELCwith(node *arg_node, info *arg_info)
 *
 * @brief: handles with nodes
 *
 *  <+long description+>
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ********************************************************************/
node *
WLSELCwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSELCwith");

    INFO_WLSELSMAX (arg_info) = 0;

    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    WITH_SELMAX (arg_node) = INFO_WLSELCMAX (arg_info);

    DBUG_RETURN (arg_node);
}

/**<!--*************************************************************-->
 *
 * @fn node *WLSELCcode(node *arg_node, info *arg_info)
 *
 * @brief: handles code nodes
 *
 *  <+long description+>
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ********************************************************************/
node *
WLSELCcode (node *arg_node, info *arg_info)
{
    bool iswlcode_old;

    DBUG_ENTER ("WLSELCcode");

    INFO_WLSELS (arg_info) = 0;
    INFO_WLFUNAPPS (arg_info) = FALSE;

    iswlcode_old = INFO_ISWLCODE (arg_info);
    INFO_ISWLCODE (arg_info) = TRUE;

    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);

    if (INFO_WLFUNAPPS (arg_info)) {
        INFO_WLSELCMAX (arg_info) = -1;
    } else {
        INFO_WLSELSMAX (arg_info) = (INFO_WLSELS (arg_info) < INFO_WLSELSMAX (arg_info))
                                      ? (INFO_WLSELSMAX (arg_info))
                                      : (INFO_WLSELS (arg_info));
    }

    INFO_ISWLCODE (arg_info) = iswlcode_old;

    CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);

    if ((NULL != CODE_NEXT (arg_node)) && (INFO_WLSELSMAX (arg_info) != -1)) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**<!--*************************************************************-->
 *
 * @fn node *WLSELCprf(node *arg_node, info *arg_info)
 *
 * @brief: handles prf nodes
 *
 *  <+long description+>
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ********************************************************************/
node *
WLSELCprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSELCprf");

    if (NULL != PRF_ARGS (arg_node)) {
        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
    }

    if ((INFO_ISWLCODE (arg_info)) && (F_sel == PRF_PRF (arg_node))) {
        INFO_WLSELS (arg_info) = INFO_WLSELS (arg_info) + 1;
    }

    DBUG_RETURN (arg_node);
}

/**<!--*************************************************************-->
 *
 * @fn node *WLSELCap(node *arg_node, info *arg_info)
 *
 * @brief: <+short description+>
 *
 *  <+long description+>
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ********************************************************************/
node *
WLSELCap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSELCap");

    if (NULL != AP_ARGS (arg_node)) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    }

    if ((INFO_ISWLCODE (arg_info))) {
        INFO_WLFUNAPPS (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}
