
/**<!--******************************************************************-->
 *
 * $Id$
 *
 * @file wlselcount.c
 *
 * This traversal infers two WIth-Loop attributes:
 *
 *        WITH_SELMAX and WITH_CONTAINSFUNAPS
 *
 * They are typically used as a crude cost estimate for Wls.
 *
 * WITH_SELMAX is the maximum number of statically identifyable
 * selections (F_sel_VxA_) performed at each iteration of
 * the With-Loop.
 *
 * A few examples:
 *
 *  a = with {
 *        ( a <= iv < b) : ... x[y] ... x[z]....k[p]...;
 *      } : modarray(...);
 *
 *  yields 3
 *
 *  a = with {
 *        ( c <= iv < d) : ... x[z]....k[p]...;
 *        ( a <= iv < b) : ... x[y] ... x[z]....k[p]...;
 *      } : modarray(...);
 *
 *  yields 3
 *
 *  a = with {
 *        ( a <= iv < b) : ... x[y] ... x[z]....k[p]...;
 *        ( c <= iv < d) {
 *          A = with {
 *                ( c <= iv < d) : ... x[z]....k[p]...;
 *                ( a <= iv < b) : ... x[y] ... x[z]....k[p]...;
 *              } : modarray(...);
 *        } : A + x[iv] ;
 *        ( e <= iv < f) {
 *          A = with {
 *                ( c <= iv < d) : ... x[z]....k[p]...;
 *              } : modarray(...);
 *        } : A;
 *      } : modarray(...);
 *
 *  yields 4 (second partition)
 *
 * WITH_CONTAINSFUNAPS is just a boolean flag indicating whether
 * ANY partition contains ANY call to a user defined function.
 *
 **************************************************************************/

#include "tree_basic.h"
#include "traverse.h"
#include "node_basic.h"

#define DBUG_PREFIX "WLSELC"
#include "debug.h"

#include "str.h"
#include "memory.h"
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
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();
    result = MEMmalloc (sizeof (info));

    INFO_WLSELSMAX (result) = 0;
    INFO_WLSELS (result) = 0;
    INFO_WLFUNAPPS (result) = FALSE;
    INFO_ISWLCODE (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

#define MAX(A, B) ((A) < (B) ? B : A)

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

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef,
                 "WLSELCdoWithloopSelection called on non N_fundef node!");
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
    DBUG_ENTER ();

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

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
    DBUG_ENTER ();

    old_funapps = INFO_WLFUNAPPS (arg_info);
    INFO_WLFUNAPPS (arg_info) = FALSE;

    /**
     * reset counter for max-value
     */
    old = INFO_WLSELSMAX (arg_info);
    INFO_WLSELSMAX (arg_info) = 0;

    DBUG_PRINT ("> analysing With-Loop in line %d", NODE_LINE (arg_node));
    /**
     * traverse into with-loop
     */
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    WITH_CONTAINSFUNAPS (arg_node) = INFO_WLFUNAPPS (arg_info);
    DBUG_PRINT ("  containsFunAps flag set to %s",
                (WITH_CONTAINSFUNAPS (arg_node) ? "true" : "false"));
    WITH_SELMAX (arg_node) = INFO_WLSELSMAX (arg_info);
    DBUG_PRINT ("  selmax counter set to %d", WITH_SELMAX (arg_node));

    INFO_WLSELSMAX (arg_info) = old;
    INFO_WLFUNAPPS (arg_info) = old_funapps;

    if (INFO_ISWLCODE (arg_info)) {
        INFO_WLSELS (arg_info) += WITH_SELMAX (arg_node);
        INFO_WLFUNAPPS (arg_info)
          = INFO_WLFUNAPPS (arg_info) || WITH_CONTAINSFUNAPS (arg_node);
    }

    DBUG_PRINT ("< done with With-Loop in line %d", NODE_LINE (arg_node));

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
    DBUG_ENTER ();

    /**
     * reset 'local' counters
     */
    old_wlsels = INFO_WLSELS (arg_info);
    old_iswlcode = INFO_ISWLCODE (arg_info);
    INFO_WLSELS (arg_info) = 0;

    /**
     * traverse into cblock
     * manipulate 'local' counters
     */
    INFO_ISWLCODE (arg_info) = TRUE;
    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    INFO_ISWLCODE (arg_info) = FALSE;

    /**
     * set max-counter according to values of 'local' counters
     */
    INFO_WLSELSMAX (arg_info) = MAX (INFO_WLSELS (arg_info), INFO_WLSELSMAX (arg_info));

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
    DBUG_ENTER ();

    if ((INFO_ISWLCODE (arg_info))
        && ((F_sel_VxA == PRF_PRF (arg_node)) || (F_idx_sel == PRF_PRF (arg_node)))) {
        INFO_WLSELS (arg_info)++;
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
    DBUG_ENTER ();

    if ((INFO_ISWLCODE (arg_info))) {
        INFO_WLFUNAPPS (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
