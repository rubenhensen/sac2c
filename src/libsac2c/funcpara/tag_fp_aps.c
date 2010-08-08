/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup tff tag fundef nodes if they contain a spawn statement
 *
 * @ingroup fp
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file tag_fp_fundefs.c
 *
 * Prefix: MSS
 *
 *****************************************************************************/
#include "tag_fp_fundefs.h"

#include "dbug.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "memory.h"
#include "traverse.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "constants.h"
#include "type_utils.h"

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *TFFdoTagFPAps( node *argnode)
 *
 *****************************************************************************/

node *
TFFdoTagFPAps (node *argnode)
{
    DBUG_ENTER ("TFFdoTagFPAps");
    DBUG_PRINT ("TFF", ("Tagging FP Ap nodes"));

    TRAVpush (TR_tfa);
    argnode = TRAVdo (argnode, NULL);
    TRAVpop ();

    DBUG_RETURN (argnode);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *TFAap(node *arg_node, info *arg_info)
 *
 * @brief Check to see if AP points to a function that contains spawns
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
TFAap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TFFap");
    DBUG_PRINT ("TFA", ("Traversing Ap node"));

    AP_TOSPAWN (arg_node) = FUNDEF_CONTAINSSPAWN (AP_FUNDEF (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Add Sync -->
 *****************************************************************************/
