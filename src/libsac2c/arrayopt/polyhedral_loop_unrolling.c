/** <!--********************************************************************-->
 *
 * @defgroup plur polyhedral loop unrolling analysis
 *
 *  Overview: This traversal attempts to set FUNDEF_LOOPCOUNT for
 *            LOOPFUNs, using polyhedral analysis. LUR checks the value of
 *            FUNDEF_LOOPCOUNT, and will perform the actual code
 *            unrolling if it deems the value appropriate.
 *
 *            NB. The loop count is reduced by one. Because
 *            NB. sac2c uses tail recursion, one iteration has
 *            NB. already been performed, so we have to compensate
 *            NB. for that.
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
};

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_VARLUT(n) ((n)->varlut)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_VARLUT (result) = NULL;

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

    if ((!FUNDEF_ISWRAPPERFUN (arg_node)) && (FUNDEF_ISLOOPFUN (arg_node))) {
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

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

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
