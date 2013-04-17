/** <!--********************************************************************-->
 *
 * @defgroup cspf Create Spawn Functions
 *
 * <pre>
 * Property                                | should be | y/n |  who  |  when
 * =============================================================================
 * can be called on N_module               |   -----   |  y  |  sah  | 02/27/11
 * can be called on N_fundef               |   -----   |  n  |  sah  | 02/27/11
 * expects LaC funs                        |   -----   |  y  |  sah  | 02/27/11
 * follows N_ap to LaC funs                |   -----   |  n  |  sah  | 02/27/11
 * =============================================================================
 * deals with GLF properly                 |    yes    |  y  |  sah  | 02/27/11
 * =============================================================================
 * is aware of potential SAA annotations   |    yes    |  y  |  sah  | 02/27/11
 * utilises SAA annotations                |   -----   |  n  |  sah  | 02/27/11
 * =============================================================================
 * tolerates flattened N_array             |    yes    |  y  |  sah  | 02/27/11
 * tolerates flattened Generators          |    yes    |  y  |  sah  | 02/27/11
 * tolerates flattened operation parts     |    yes    |  y  |  sah  | 02/27/11
 * tolerates different generator variables
 *           in individual WL partitions   |    yes    |  y  |  sah  | 02/27/11
 * =============================================================================
 * tolerates multi-operator WLs            |    yes    |  y  |  sah  | 02/27/11
 * =============================================================================
 * </pre>
 *
 * This module creates local copies of all spawned functions. This is
 * necessary so that these functions can be compiled differently in the
 * backend.
 *
 * @ingroup cspf
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file create_spawn_funs.c
 *
 * Prefix: CSPF
 *
 *****************************************************************************/
#include "create_spawn_funs.h"

/*
 * Other includes go here
 */

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "traverse.h"
#include "tree_basic.h"
#include "memory.h"
#include "namespaces.h"
#include "deserialize.h"
#include "DupTree.h"
#include "new_types.h"
#include "type_utils.h"
#include "tree_compound.h"
#include "elim_alpha_types.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *spawnstore;
};

/**
 * A template entry in the template info structure
 */
#define INFO_SPAWNSTORE(n) ((n)->spawnstore)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_SPAWNSTORE (result) = NULL;

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
 * @fn node *CSPFdoCreateSpawnFunctions( node *syntax_tree)
 *
 *****************************************************************************/
node *
CSPFdoCreateSpawnFunctions (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_cspf);
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
 * @fn node *LocalizeAndMakeSpawnFun(node *arg_node)
 *
 * @brief A dummy static helper functions used only in your traversal
 *
 *****************************************************************************/
static node *
LocalizeAndMakeSpawnFun (node *arg_node, info *arg_info)
{
    node *temp, *result;
    ntype *wtype = NULL;

    DBUG_ENTER ();

    if (FUNDEF_SPAWNFUN (arg_node) == NULL) {
        temp = FUNDEF_NEXT (arg_node);
        FUNDEF_NEXT (arg_node) = NULL;
        result = DUPdoDupTree (arg_node);
        if (FUNDEF_ISWRAPPERFUN (arg_node)) {
            if (FUNDEF_IMPL (arg_node) != NULL) {
                FUNDEF_IMPL (arg_node)
                  = LocalizeAndMakeSpawnFun (FUNDEF_IMPL (arg_node), arg_info);
            } else {
                wtype = TYmapFunctionInstances (FUNDEF_WRAPPERTYPE (result),
                                                LocalizeAndMakeSpawnFun, arg_info);
                FUNDEF_WRAPPERTYPE (result) = TUrebuildWrapperTypeAlphaFix (wtype);
                wtype = TYfreeType (wtype);
            }
        } else if (FUNDEF_BODY (result) == NULL) {
            FUNDEF_BODY (result) = DSloadFunctionBody (arg_node);
            DBUG_ASSERT (FUNDEF_BODY (result) != NULL, "function body went missing");
        }
        if (FUNDEF_SYMBOLNAME (result) != NULL) {
            FUNDEF_SYMBOLNAME (result) = MEMfree (FUNDEF_SYMBOLNAME (result));
        }
        if (!FUNDEF_ISLOCAL (result)) {
            FUNDEF_ISLOCAL (result) = TRUE;
            FUNDEF_WASUSED (result) = FALSE;
            FUNDEF_WASIMPORTED (result) = FALSE;
            FUNDEF_NS (result) = NSfreeNamespace (FUNDEF_NS (result));
            FUNDEF_NS (result) = NSbuildView (FUNDEF_NS (arg_node));
        }
        FUNDEF_ISEXPORTED (result) = FALSE;
        FUNDEF_ISPROVIDED (result) = FALSE;
        FUNDEF_ISSPAWNFUN (result) = TRUE;
        FUNDEF_ISTHREADFUN (result) = TRUE;

        FUNDEF_SPAWNFUN (arg_node) = result;
        FUNDEF_NEXT (arg_node) = temp;

        FUNDEF_NEXT (result) = INFO_SPAWNSTORE (arg_info);
        INFO_SPAWNSTORE (arg_info) = result;
    }

    DBUG_RETURN (FUNDEF_SPAWNFUN (arg_node));
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
 * @fn node *CSPFmodule(node *arg_node, info *arg_info)
 *
 * @brief Traverses the fundef chain to pick up all spawn operations and
 *        create their target functions
 *
 *****************************************************************************/
node *
CSPFmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DSinitDeserialize (arg_node);

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    if (INFO_SPAWNSTORE (arg_info) != NULL) {
        MODULE_FUNS (arg_node)
          = TCappendFundef (INFO_SPAWNSTORE (arg_info), MODULE_FUNS (arg_node));
        INFO_SPAWNSTORE (arg_info) = NULL;
        arg_node = EATdoEliminateAlphaTypes (arg_node);
    }

    DSfinishDeserialize (arg_node);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CSPFap(node *arg_node, info *arg_info)
 *
 * @brief Inspects the AP node whether it is a spawn operation and creates
 *        the spawn target if it does not exist, yet.
 *
 *****************************************************************************/
node *
CSPFap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (AP_ISSPAWNED (arg_node)) {
        AP_FUNDEF (arg_node) = LocalizeAndMakeSpawnFun (AP_FUNDEF (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Create Spawn Functions template -->
 *****************************************************************************/

#undef DBUG_PREFIX
