/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup rmo Remove unused vardecs
 *
 * To prevent superfluous vardecs hanging around in the AST, this traversal
 * quickly eliminates all vardecs no IDS exists for.
 *
 * @ingroup opt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file remove_vardecs.c
 *
 * Prefix: RMV
 *
 *****************************************************************************/
#include "remove_vardecs.h"

#include "dbug.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "str.h"
#include "memory.h"
#include "traverse.h"
#include "free.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    enum { TM_init, TM_delete } travmode;
};

#define INFO_TRAVMODE(n) ((n)->travmode)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_TRAVMODE (result) = TM_init;

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
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *RMVdoRemoveVardecsOneFundef( node *fundef)
 *
 *****************************************************************************/
node *
RMVdoRemoveVardecsOneFundef (node *fundef)
{
    info *info;

    DBUG_ENTER ("RMVdoRemoveVardecsOneFundef");

    info = MakeInfo ();

    TRAVpush (TR_rmv);
    fundef = TRAVdo (fundef, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (fundef);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *RMVfundef(node *arg_node, info *arg_info)
 *
 * @brief Performs a traversal of the fundef chain entering the body
 *
 *****************************************************************************/
node *
RMVfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RMVfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RMVblock( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
RMVblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RMVblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        INFO_TRAVMODE (arg_info) = TM_init;
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    if (BLOCK_VARDEC (arg_node) != NULL) {
        INFO_TRAVMODE (arg_info) = TM_delete;
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RMVvardec( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
RMVvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RMVvardec");

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    switch (INFO_TRAVMODE (arg_info)) {
    case TM_init:
        AVIS_ISDEAD (VARDEC_AVIS (arg_node)) = TRUE;
        break;

    case TM_delete:
        if (AVIS_ISDEAD (VARDEC_AVIS (arg_node))) {
            arg_node = FREEdoFreeNode (arg_node);
        }
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RMVids( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
RMVids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RMVids");

    AVIS_ISDEAD (IDS_AVIS (arg_node)) = FALSE;

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Remove Vardecs -->
 *****************************************************************************/
