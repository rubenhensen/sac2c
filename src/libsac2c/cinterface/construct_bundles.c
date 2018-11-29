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

#define DBUG_PREFIX "CBL"
#include "debug.h"

#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "memory.h"
#include "str.h"
#include "str_buffer.h"
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
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_BUNDLES (result) = NULL;

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

    DBUG_ENTER ();

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

static char *
GenerateFunbundleName (char *name, namespace_t *ns, size_t arity)
{
    char *result, *safens, *safename;
    str_buf *buffer;

    DBUG_ENTER ();

    buffer = SBUFcreate (128);

    safename = STRreplaceSpecialCharacters (name);
    safens = STRreplaceSpecialCharacters (NSgetName (ns));

    buffer = SBUFprintf (buffer, "%s__%s%d", safens, safename, arity);
    result = SBUF2str (buffer);

    DBUG_PRINT ("generated name '%s'", result);

    buffer = SBUFfree (buffer);

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn node *InsertIntoBundles( node *fundef, int arity, node *bundles)
 *
 * @brief Inserts the given fundef into the appropriate bundle in bundles.
 *        If no such bundle exists, it will be created.
 *
 *****************************************************************************/
static node *
InsertIntoBundles (node *fundef, size_t arity, node *bundles)
{
    DBUG_ENTER ();

    DBUG_ASSERT (FUNDEF_NEXT (fundef) == NULL,
                 "FUNDEF_NEXT needs to be NULL before InsertIntoBundles is called!");

    if (bundles == NULL) {
        bundles = TBmakeFunbundle (STRcpy (FUNDEF_NAME (fundef)),
                                   NSdupNamespace (FUNDEF_NS (fundef)),
                                   GenerateFunbundleName (FUNDEF_NAME (fundef),
                                                          FUNDEF_NS (fundef), arity),
                                   arity, fundef, NULL);
        FUNBUNDLE_ISXTBUNDLE (bundles) = FUNDEF_ISXTFUN (fundef);
        FUNBUNDLE_ISSTBUNDLE (bundles) = FUNDEF_ISSTFUN (fundef);
        DBUG_PRINT ("Funbundle created: %s.\n", FUNBUNDLE_NAME (bundles));
    } else {
        if ((arity == FUNBUNDLE_ARITY (bundles)) &&
            // NSequals( FUNDEF_NS( fundef), FUNBUNDLE_NS( bundles)) &&
            STReq (NSgetName (FUNDEF_NS (fundef)), NSgetName (FUNBUNDLE_NS (bundles)))
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
    node *old_node = NULL;
    size_t arity;

    DBUG_ENTER ();

    if (FUNDEF_ISWRAPPERFUN (arg_node) && !FUNDEF_ISSACARGCONVERSION (arg_node)) {
        if (!FUNDEF_HASDOTRETS (arg_node) && !FUNDEF_HASDOTARGS (arg_node)) {
            old_node = arg_node;
            arg_node = FUNDEF_NEXT (arg_node);
            FUNDEF_NEXT (old_node) = NULL;

            arity = TCcountArgs (FUNDEF_ARGS (old_node));

            DBUG_PRINT ("Adding function %s (%zu) to bundle.", CTIitemName (old_node),
                        arity);

            INFO_BUNDLES (arg_info)
              = InsertIntoBundles (old_node, arity, INFO_BUNDLES (arg_info));
        } else {
            CTIwarn ("%s is not exported as it uses varargs.", CTIitemName (arg_node));
        }
    }

    if (old_node != NULL) {
        /*
         * we moved a fundef into a bundle so the current
         * node actually already is the next node
         */
        if (arg_node == NULL) {
            arg_node = INFO_BUNDLES (arg_info);
            INFO_BUNDLES (arg_info) = NULL;
        } else {
            arg_node = TRAVdo (arg_node, arg_info);
        }
    } else {
        /*
         * we did not do anything, so go on as usual
         */
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
    DBUG_ENTER ();

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

#undef DBUG_PREFIX
