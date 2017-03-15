/** <!--********************************************************************-->
 *
 * @defgroup plur polyhedral loop unrolling analysis
 *
 *  Overview: This traversal attempts to set FUNDEF_LOOPCOUNT for
 *            LOOPFUNs, using polyhedral analysis. LUR checks the value of
 *            FUNDEF_LOOPCOUNT, and will perform the actual code
 *            unrolling if it deems the value appropriate.
 *
 *            NB. The loop count is one less than the number of
 *            NB. iterations, because sac2c uses tail recursion:
 *            NB  One iteration has already been performed,
 *            NB. so we have to compensate for that.
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file polyhedral_loop_unrolling.c
 *
 * Prefix: PLUR
 *
 *****************************************************************************/

#include "globals.h"

#define DBUG_PREFIX "PLUR"
#include <stdlib.h>
#include <sys/wait.h>
#include "debug.h"
#include "memory.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "pattern_match.h"
#include "constants.h"
#include "shape.h"
#include "type_utils.h"
#include "new_types.h"
#include "constants.h"
#include "new_typecheck.h"
#include "narray_utilities.h"
#include "DupTree.h"
#include "print.h"
#include "system.h"
#include "filemgr.h"
#include "sys/param.h"
#include "polyhedral_utilities.h"
#include "polyhedral_setup.h"
#include "polyhedral_loop_unrolling.h"
#include "polyhedral_defs.h"
#include "LookUpTable.h"
#include "polyhedral_guard_optimization.h"

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    lut_t *varlut;
    node *lacfun;
    node *nassign;
};

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_VARLUT(n) ((n)->varlut)
#define INFO_LACFUN(n) ((n)->lacfun)
#define INFO_NASSIGN(n) ((n)->nassign)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_VARLUT (result) = NULL;
    INFO_LACFUN (result) = NULL;
    INFO_NASSIGN (result) = NULL;

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

/** <!--********************************************************************-->
 *
 * @name Static helper functions
 * @{
 *
 *****************************************************************************/

/** <!--*******************************************************************-->
 *
 * @fn node *PLURap( node *arg_node, info *arg_info)
 *
 * @brief: If this is a non-recursive call of a LACFUN,
 *         set FUNDEF_CALLAP to point to this N_ap's N_assign node,
 *         and FUNDEF_CALLERFUNDEF to pint to this N_ap's fundef node,
 *         then traverse the LACFUN.
 *
 *****************************************************************************/
node *
PLURap (node *arg_node, info *arg_info)
{
    node *lacfundef;
    node *newfundef;

    DBUG_ENTER ();

    lacfundef = AP_FUNDEF (arg_node);
    if ((FUNDEF_ISLACFUN (lacfundef)) &&         // Ignore call to non-lacfun
        (NULL != INFO_LACFUN (arg_info)) &&      // Ignore vanilla traversal
        (lacfundef != INFO_FUNDEF (arg_info))) { // Ignore recursive call
        DBUG_PRINT ("Found LACFUN: %s non-recursive call from: %s",
                    FUNDEF_NAME (lacfundef), FUNDEF_NAME (INFO_FUNDEF (arg_info)));

        // Traverse into the LACFUN
        INFO_LACFUN (arg_info) = lacfundef; // The called lacfun
        FUNDEF_CALLAP (lacfundef) = INFO_NASSIGN (arg_info);
        FUNDEF_CALLERFUNDEF (lacfundef) = INFO_FUNDEF (arg_info);
        newfundef = TRAVdo (lacfundef, arg_info);
        DBUG_ASSERT (newfundef = lacfundef,
                     "Did not expect N_fundef of LACFUN to change");
        INFO_LACFUN (arg_info) = NULL; // Back to normal traversal
        FUNDEF_CALLAP (lacfundef) = NULL;
        FUNDEF_CALLERFUNDEF (lacfundef) = NULL;
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *PLURfundef( node *arg_node, info *arg_info)
 *
 * @brief Traverse LOOPFUNs only.
 *
 *****************************************************************************/
node *
PLURfundef (node *arg_node, info *arg_info)
{
    node *fundefold;
    int lc = UNR_NONE;

    DBUG_ENTER ();

    fundefold = INFO_FUNDEF (arg_info);
    INFO_FUNDEF (arg_info) = arg_node;

    if ((!FUNDEF_ISWRAPPERFUN (arg_node)) &&    // Ignore wrappers
        (arg_node != INFO_LACFUN (arg_info)) && // Ignore recursive call
        (FUNDEF_ISLOOPFUN (arg_node))) {        // loopfuns only
        DBUG_PRINT ("Starting to traverse LOOPFUN %s", FUNDEF_NAME (arg_node));
        lc = PHUTgetLoopCount (arg_node, INFO_VARLUT (arg_info));
        if (UNR_NONE != lc) {
            DBUG_PRINT ("Loop count for LOOPFUN %s was %d, is now %d",
                        FUNDEF_NAME (INFO_FUNDEF (arg_info)), FUNDEF_LOOPCOUNT (arg_node),
                        lc);
            FUNDEF_LOOPCOUNT (arg_node) = lc;
            global.optcounters.plur_expr++;
        }
    }

    PHUTclearAvisIslAttributes (INFO_VARLUT (arg_info));
    DBUG_PRINT ("Removing content from VARLUT");
    LUTremoveContentLut (INFO_VARLUT (arg_info));
    INFO_FUNDEF (arg_info) = fundefold;
    DBUG_PRINT ("leaving function %s", FUNDEF_NAME (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *PLURlet( node *arg_node, info *arg_info)
 *
 *
 *****************************************************************************/
node *
PLURlet (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *PLURassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
PLURassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_NASSIGN (arg_info) = arg_node;
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    INFO_NASSIGN (arg_info) = NULL;

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *PLURpart( node *arg_node, info *arg_info)
 *
 * @brief Mark the WITHID_VEC, WITHID_IDS, and WITHID_IDXS of this N_part
 *        with the N_part address when we enter, and NULL them on the way out.
 *
 *        These values are used by PHUT to locate GENERATOR_BOUND values
 *        for the WL variables. Those bounds are then used to create
 *        polyhedral inequalities for the WITHID variables.
 *
 *****************************************************************************/
node *
PLURpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    arg_node = POLYSsetClearAvisPart (arg_node, arg_node);

    CODE_CBLOCK (PART_CODE (arg_node))
      = TRAVopt (CODE_CBLOCK (PART_CODE (arg_node)), arg_info);

    arg_node = POLYSsetClearAvisPart (arg_node, NULL);

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PLURdoPolyhedralLoopUnrolling( node *arg_node)
 *
 *   @brief
 *   @param arg_node
 *   @return modified AST.
 *
 *****************************************************************************/
node *
PLURdoPolyhedralLoopUnrolling (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    INFO_VARLUT (arg_info) = LUTgenerateLut ();
    TRAVpush (TR_plur);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();
    INFO_VARLUT (arg_info) = LUTremoveLut (INFO_VARLUT (arg_info));

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
