/* $Id$ */

/******************************************************************************
 *
 * Things done during this traversal:
 *   - Shared N_code blocks are duplicated so that ALL CODE_USED counters are 1!
 *     This is required for the code generation as the generated code may
 *     contain labels which may not be copied! (Earlier, we simply created
 *     the ICMs and duplicated those which led to bug #231!)
 *
 ******************************************************************************/

#include "resolve_code_sharing.h"
#include "tree_basic.h"
#include "DupTree.h"

#define DBUG_PREFIX "RCS"
#include "debug.h"

#include "traverse.h"

node *
RCSdoResolveCodeSharing (node *arg_node)
{
    DBUG_ENTER ();

    TRAVpush (TR_rcs);

    arg_node = TRAVdo (arg_node, NULL);

    TRAVpop ();

    DBUG_RETURN (arg_node);
}

static node *
ResolvePotentialSharing (node *code)
{
    node *new_code, *next_code;

    DBUG_ENTER ();

    if (code != NULL) {
        if (CODE_USED (code) == 0) {
            CODE_USED (code) = 1;
            DBUG_PRINT ("CODE_USED( %p) = 1", code);

            new_code = code;
        } else {
            /**
             * Duplicate this very code:
             */
            next_code = CODE_NEXT (code);
            CODE_NEXT (code) = NULL;
            new_code = DUPdoDupTree (code);
            /**
             * Node here, that DUPcode always
             * initializes CODE_USED with 0!!!!
             * Therefore, we have to explicitly set
             * it to 1!
             */
            CODE_USED (new_code) = 1;
            /**
             * and plug it into the code chain:
             */
            CODE_NEXT (code) = new_code;
            CODE_NEXT (new_code) = next_code;
            DBUG_PRINT ("duplicated %p into %p", code, new_code);
        }
    } else {
        new_code = code;
    }

    DBUG_RETURN (new_code);
}

/** <!--********************************************************************-->
 *
 * @fn node *RCSwith( node *arg_node, info *arg_info)
 *
 * @brief ensures the order of traversal!
 *
 *****************************************************************************/
node *
RCSwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /**
     * First, we reset all USED counters!
     */
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    /**
     * Then, we resolve the sharing while traversing the parts!
     */
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RCSwith2( node *arg_node, info *arg_info)
 *
 * @brief  ensures the order of traversal!
 *
 *****************************************************************************/
node *
RCSwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /**
     * First, we reset all USED counters!
     */
    WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);

    /**
     * Then, we resolve the sharing while traversing the segments!
     */
    WITH2_SEGS (arg_node) = TRAVdo (WITH2_SEGS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RCScode( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
RCScode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    CODE_USED (arg_node) = 0;

    DBUG_PRINT ("CODE_USED( %p) = 0", arg_node);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RCSpart( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
RCSpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    PART_CODE (arg_node) = ResolvePotentialSharing (PART_CODE (arg_node));

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RCSwlgrid( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
RCSwlgrid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    WLGRID_CODE (arg_node) = ResolvePotentialSharing (WLGRID_CODE (arg_node));

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
