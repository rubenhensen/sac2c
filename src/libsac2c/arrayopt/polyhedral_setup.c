/**
 *
 * @file Polyhedral Analysis Setup
 *
 * This traversal initializes several fields:
 *
 *    AVIS_NPART
 *    FUNDEF_LOOPCOUNT
 *    AVIS_ISLTREE
 *
 * These are needed by polyhedral-based optimizations.
 *
 */

#define DBUG_PREFIX "POLYS"
#include "debug.h"

#include "types.h"
#include "DupTree.h"
#include "free.h"
#include "new_types.h"
#include "constants.h"
#include "globals.h"
#include "memory.h"
#include "shape.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "polyhedral_setup.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    node *lhs;
    node *preassigns;
    node *with;
    node *nassign;
    node *lacfun;
    bool issetup;
};

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_WITH(n) ((n)->with)
#define INFO_NASSIGN(n) ((n)->nassign)
#define INFO_LACFUN(n) ((n)->lacfun)
#define INFO_ISSETUP(n) ((n)->issetup)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_WITH (result) = NULL;
    INFO_NASSIGN (result) = NULL;
    INFO_LACFUN (result) = NULL;
    INFO_ISSETUP (result) = FALSE;

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
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--*******************************************************************-->
 *
 * @fn void *
 *
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper functions
 * @{
 *
 *****************************************************************************/

/** <!--*******************************************************************-->
 *
 * @fn node *POLYSfundef( node *arg_node, info *arg_info)
 *
 * @brief Traverse this function only. Do not traverse any LACFUN, unless
 *        we come via POLYSap.
 *
 *****************************************************************************/
node *
POLYSfundef (node *arg_node, info *arg_info)
{
    node *fundefold;

    DBUG_ENTER ();

    fundefold = INFO_FUNDEF (arg_info);
    INFO_FUNDEF (arg_info) = arg_node;

    if (!FUNDEF_ISWRAPPERFUN (arg_node)) {
        if ((!FUNDEF_ISLACFUN (arg_node)) || (arg_node == INFO_LACFUN (arg_info))) {
            /* Vanilla traversal or LACFUN via POLYSap */
            DBUG_PRINT ("Starting to traverse %s %s",
                        (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                        FUNDEF_NAME (arg_node));
            FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        }
    }

    INFO_FUNDEF (arg_info) = fundefold;
    DBUG_PRINT ("leaving function %s", FUNDEF_NAME (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *POLYSpart( node *arg_node, info *arg_info)
 *
 * @brief Mark the WITHID_VEC, WITHID_IDS, and WITHID_IDXS of this N_part
 *        with the N_part address when we are doing setup,
 *        and NULL them when we are doing tear down.
 *
 *        These values are used by PHUT to locate GENERATOR_BOUND values
 *        for the WL variables. Those bounds are then used to create
 *        polyhedral inequalities for the WITHID variables.
 *
 *****************************************************************************/
node *
POLYSpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_ISSETUP (arg_info)) {
        arg_node = POLYSsetClearAvisPart (arg_node, arg_node);
    }

    CODE_CBLOCK (PART_CODE (arg_node))
      = TRAVopt (CODE_CBLOCK (PART_CODE (arg_node)), arg_info);

    if (!INFO_ISSETUP (arg_info)) {
        arg_node = POLYSsetClearAvisPart (arg_node, NULL);
    }

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *POLYSwith( node *arg_node, info *arg_info)
 *
 * @brief Traverse with-loop partitions. Rebuild WITH_CODE
 *        We disconnect the current WITH_CODE chain.
 *
 *****************************************************************************/
node *
POLYSwith (node *arg_node, info *arg_info)
{
    node *lastwith;

    DBUG_ENTER ();

    lastwith = INFO_WITH (arg_info);
    INFO_WITH (arg_info) = arg_node;

    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    INFO_WITH (arg_info) = lastwith;

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *POLYSassign( node *arg_node, info *arg_info)
 *
 *
 *****************************************************************************/
node *
POLYSassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_NASSIGN (arg_info) = arg_node;
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    INFO_NASSIGN (arg_info) = NULL;

    if (INFO_PREASSIGNS (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *POLYSap( node *arg_node, info *arg_info)
 *
 * @brief: If this is a non-recursive call of a LACFUN,
 *         traverse the LACFUN.
 *
 *****************************************************************************/
node *
POLYSap (node *arg_node, info *arg_info)
{
    node *lacfundef;
    node *newfundef;

    DBUG_ENTER ();

    lacfundef = AP_FUNDEF (arg_node);
    if ((FUNDEF_ISLACFUN (lacfundef)) &&         /* Ignore call to non-lacfun */
        (lacfundef != INFO_FUNDEF (arg_info))) { /* Ignore recursive call */
        DBUG_PRINT ("Found LACFUN: %s non-recursive call from: %s",
                    FUNDEF_NAME (lacfundef), FUNDEF_NAME (INFO_FUNDEF (arg_info)));

        POLYSsetClearCallAp (lacfundef, INFO_FUNDEF (arg_info), INFO_NASSIGN (arg_info),
                             INFO_ISSETUP (arg_info));

        // Traverse into the LACFUN
        INFO_LACFUN (arg_info) = lacfundef;
        newfundef = TRAVdo (lacfundef, arg_info);
        DBUG_ASSERT (newfundef == lacfundef,
                     "Did not expect N_fundef of LACFUN to change");
        INFO_LACFUN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *POLYSlet( node *arg_node, info *arg_info)
 *
 *
 *****************************************************************************/
node *
POLYSlet (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    INFO_LHS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *POLYSprf( node *arg_node, info *arg_info)
 *
 * @brief:
 *
 * @result:
 *
 *****************************************************************************/
node *
POLYSprf (node *arg_node, info *arg_info)
{
    node *res;

    DBUG_ENTER ();

    res = arg_node;

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *POLYSdoPolyhedralSetup( node *arg_node)
 *
 *   @brief
 *   @param arg_node
 *   @return modified AST.
 *
 *****************************************************************************/
node *
POLYSdoPolyhedralSetup (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();
    INFO_ISSETUP (arg_info) = TRUE;

    TRAVpush (TR_polys);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *POLYSdoPolyhedralTearDown( node *arg_node)
 *
 *   @brief
 *   @param arg_node
 *   @return modified AST.
 *
 *****************************************************************************/
node *
POLYSdoPolyhedralTearDown (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();
    INFO_ISSETUP (arg_info) = FALSE;

    TRAVpush (TR_polys);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *POLYSsetClearAvisPart( node *arg_node, node *val)
 *
 * @brief Set or clear AVIS_NPART attributes for arg_node's WITHID_IDS,
 *        WITHID_VEC, and WITHID_IDXS to val.
 *
 * @param arg_node: The N_part of a WL
 * @param val: Either NULL or the address of the N_part.
 *
 * @return arg_node, unchanged. Side effects on the relevant N_avis nodes.
 *
 * NB. Since WITHID elements are NOT SSA,
 *     elements in multiple partitions will share
 *     the same N_AVIS nodes. This means that AVIS_NPART MUST be set on
 *     a traversal into a partition, and cleared on exiting from that partition.
 *     Hence, the DBUG_ASSERT statements below, in case you were not
 *     paying attention.
 *
 *     The above paragraph is probably no longer correct, now that we operate
 *     in -dossawl mode. However, I am not sure how/if we can maintain
 *     AVIS_NPART even in SSA mode.
 *
 ******************************************************************************/
node *
POLYSsetClearAvisPart (node *arg_node, node *val)
{
    node *ids;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_part, "Expected N_part node");

    AVIS_NPART (IDS_AVIS (WITHID_VEC (PART_WITHID (arg_node)))) = val;

    ids = WITHID_IDS (PART_WITHID (arg_node));
    while (NULL != ids) {
        AVIS_NPART (IDS_AVIS (ids)) = val;
        ids = IDS_NEXT (ids);
    }

    ids = WITHID_IDXS (PART_WITHID (arg_node));
    while (NULL != ids) {
        AVIS_NPART (IDS_AVIS (ids)) = val;
        ids = IDS_NEXT (ids);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node node *POLYSsetClearCallAp()
 *
 * @brief Set or clear FUNDEF_CALLAP in a LACFUN
 *        "nassign" should be set to the external function's N_assign node
 *        for the LACFUN's N_ap call, when the caller is being traversed,
 *        and NULLed afterward. The LACFUN can then be called to do its thing,
 *        and when that completes, it is nulled by the caller.
 *        FUNDEF_CALLAP and FUNDEF_CALLERFUNDEF are used within a LACFUN
 *        to find the argument list and fundef entry of its calling function.
 *
 *        This could have been done inline, but this way everbody uses
 *        the same code.
 *
 * @param arg_node: The N_fundef of a LACFUN.
 * @param callerfundef: The N_fundef of the caller function of the LACFUN
 * @param nassign: The N_assign node of the caller's N_ap of this LACFUN
 * @param setclear: TRUE for set; FALSE for clear
 *
 * @return none
 *
 *
 ******************************************************************************/
void
POLYSsetClearCallAp (node *arg_node, node *callerfundef, node *nassign, bool setclear)
{

    DBUG_ENTER ();

    FUNDEF_CALLAP (arg_node) = setclear ? nassign : NULL;
    FUNDEF_CALLERFUNDEF (arg_node) = setclear ? callerfundef : NULL;

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
