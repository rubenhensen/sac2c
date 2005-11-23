
/**<!--******************************************************************-->
 *
 * $Id$
 *
 * @file wlselcount.c
 *
 *
 **************************************************************************/

#include "tree_basic.h"
#include "traverse.h"
#include "node_basic.h"
#include "dbug.h"
#include "internal_lib.h"
#include "free.h"
#include "tree_compound.h"

#include "wlselcount.h"

/*
 * info structure
 */
struct INFO {
    bool wlfunapps;
    bool iswlcode;
    int wlselsmax;
    int wlsels;
};

#define INFO_WLSELSMAX(n) (n->wlselsmax)
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

/**<!--*************************************************************-->
 *
 * @fn node *WLSELCdoWithloopSelectionCount(node *fundef)
 *
 * @brief: starting point of traversal
 *
 *  <+long description+>
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ********************************************************************/
node *
WLSELCdoWithloopSelectionCount (node *fundef)
{
    info *arg_info;

    DBUG_ENTER ("WLSELCdoWithloopSelection");

    arg_info = MakeInfo ();

    TRAVpush (TR_wlselc);
    fundef = TRAVdo (fundef, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

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

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

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
    int old;
    bool old_funapps;
    DBUG_ENTER ("WLSELCwith");

    old_funapps = INFO_WLFUNAPPS (arg_info);
    INFO_WLFUNAPPS (arg_info) = FALSE;

    /**
     * reset counter for max-value
     */
    old = INFO_WLSELSMAX (arg_info);
    INFO_WLSELSMAX (arg_info) = 0;

    /**
     * traverse into with-loop
     */

    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    /**
     * assign max-value to with-loop
     */
    if (INFO_WLFUNAPPS (arg_info)) {
        INFO_WLSELSMAX (arg_info) = -1;
    }
    WITH_SELMAX (arg_node) = INFO_WLSELSMAX (arg_info);
    INFO_WLSELSMAX (arg_info) = old;
    if (WITH_SELMAX (arg_node) >= 0) {
        INFO_WLSELS (arg_info) = INFO_WLSELS (arg_info) + WITH_SELMAX (arg_node);
    } else {
        INFO_WLSELS (arg_info) = -1;
    }

    INFO_WLFUNAPPS (arg_info) = old_funapps;
    if (INFO_ISWLCODE (arg_info)) {
        INFO_WLFUNAPPS (arg_info) = TRUE;
    }

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
    int old_wlsels;
    bool old_iswlcode;
    DBUG_ENTER ("WLSELCcode");

    /**
     * reset 'local' counters
     */
    old_wlsels = INFO_WLSELS (arg_info);
    old_iswlcode = INFO_ISWLCODE (arg_info);
    INFO_WLSELS (arg_info) = 0;
    INFO_ISWLCODE (arg_info) = TRUE;

    /**
     * traverse into cblock
     * manipulate 'local' counters
     */
    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);

    /**
     * set max-counter according to values of 'local' counters
     */
    INFO_WLSELSMAX (arg_info) = (INFO_WLSELS (arg_info) < INFO_WLSELSMAX (arg_info))
                                  ? (INFO_WLSELSMAX (arg_info))
                                  : (INFO_WLSELS (arg_info));

    INFO_ISWLCODE (arg_info) = FALSE;

    /**
     * traverse into other sub-structures
     */

    INFO_WLSELS (arg_info) = old_wlsels;
    INFO_ISWLCODE (arg_info) = old_iswlcode;

    if ((NULL != CODE_NEXT (arg_node))) {
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

    if ((INFO_ISWLCODE (arg_info))) {
        INFO_WLFUNAPPS (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}
