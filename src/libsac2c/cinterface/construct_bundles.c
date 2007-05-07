/* $Id$ */

/** <!--********************************************************************-->
 *
 * @defgroup cbl Construct Bundles template
 *
 * Module description goes here.
 *
 * @ingroup cbl
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file construct_bundles.c
 *
 * Prefix: CBL
 *
 *****************************************************************************/

#include "construct_bundles.h"

#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "memory.h"
#include "str.h"
#include "namespaces.h"
#include "ctinfo.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *bundles;
};

#define INFO_BUNDLES(n) (n->bundles)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_BUNDLES (result) = NULL;

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
 * @fn node *CBLdoConstructBundles( node *syntax_tree)
 *
 *****************************************************************************/
node *
CBLdoConstructBundles (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("CBLdoConstructBundles");

    info = MakeInfo ();

    TRAVpush (TR_cbl);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
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
 *
 * @fn node *InsertIntoBundles( node *fundef, int arity, node *bundles)
 *
 * @brief Inserts the given fundef into the appropriate bundle in bundles.
 *        If no such bundle exists, it will be created.
 *
 *****************************************************************************/
static node *
InsertIntoBundles (node *fundef, int arity, node *bundles)
{
    DBUG_ENTER ("InsertIntoBundles");

    DBUG_ASSERT ((FUNDEF_NEXT (fundef) == NULL),
                 "FUNDEF_NEXT needs to be NULL before InsertIntoBundles is called!");

    if (bundles == NULL) {
        bundles
          = TBmakeFunbundle (STRcpy (FUNDEF_NAME (fundef)),
                             NSdupNamespace (FUNDEF_NS (fundef)), arity, fundef, NULL);
    } else {
        if ((arity == FUNBUNDLE_ARITY (bundles))
            && NSequals (FUNDEF_NS (fundef), FUNBUNDLE_NS (bundles))
            && STReq (FUNDEF_NAME (fundef), FUNBUNDLE_NAME (bundles))) {
            FUNBUNDLE_FUNDEF (bundles)
              = TCappendFundef (FUNBUNDLE_FUNDEF (bundles), fundef);
        } else {
            FUNBUNDLE_NEXT (bundles)
              = InsertIntoBundles (fundef, arity, FUNBUNDLE_NEXT (bundles));
        }
    }

    DBUG_RETURN (bundles);
}

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
 * @fn node *CBLfundef(node *arg_node, info *arg_info)
 *
 * @brief Traverses the fundef chain and adds all wrappers to a matching
 *        function bundle.
 *
 *****************************************************************************/
node *
CBLfundef (node *arg_node, info *arg_info)
{
    node *temp;
    int arity;

    DBUG_ENTER ("CBLfundef");

    if (FUNDEF_ISWRAPPERFUN (arg_node)) {
        temp = arg_node;
        arg_node = FUNDEF_NEXT (arg_node);
        FUNDEF_NEXT (temp) = NULL;

        arity = TCcountArgs (FUNDEF_ARGS (temp));

        DBUG_PRINT ("CBL",
                    ("Adding function %s (%d) to bundle.", CTIitemName (temp), arity));

        INFO_BUNDLES (arg_info)
          = InsertIntoBundles (temp, arity, INFO_BUNDLES (arg_info));
    }

    if (arg_node == NULL) {
        arg_node = INFO_BUNDLES (arg_info);
        INFO_BUNDLES (arg_info) = NULL;
    } else {
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        } else {
            FUNDEF_NEXT (arg_node) = INFO_BUNDLES (arg_info);
            INFO_BUNDLES (arg_info) = NULL;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CBLmodule(node *arg_node, info *arg_info)
 *
 * @brief Traverses only the fundefs chain, as all others cannot contain
 *        wrappers.
 *
 *****************************************************************************/
node *
CBLmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CBLmodule");

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal Construct Bundles -->
 *****************************************************************************/
