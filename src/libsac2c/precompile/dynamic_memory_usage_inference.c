/** <!--********************************************************************-->
 *
 * @defgroup temp Traversal template
 *
 * <pre>
 * Property                                | should be | y/n |  who  |  when
 * =============================================================================
 * can be called on N_module               |   -----   |  y  |       |
 * can be called on N_fundef               |   -----   |  y  |       |
 * expects LaC funs                        |   -----   |  n  |       |
 * follows N_ap to LaC funs                |   -----   |  n  |       |
 * =============================================================================
 * deals with GLF properly                 |    yes    | yes |       |
 * =============================================================================
 * is aware of potential SAA annotations   |    yes    | yes |       |
 * utilises SAA annotations                |   -----   |  no |       |
 * =============================================================================
 * tolerates flattened N_array             |    yes    | yes |       |
 * tolerates flattened Generators          |    yes    | yes |       |
 * tolerates flattened operation parts     |    yes    | yes |       |
 * tolerates different generator variables
 *           in individual WL partitions   |    yes    | yes |       |
 * =============================================================================
 * tolerates multi-operator WLs            |    yes    | yes |       |
 * =============================================================================
 * </pre>
 *
 * Find any function that does not need to perform dynamic memory allocation
 *
 * @ingroup tt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file dynamic_memory_usage_inference.c
 *
 * Prefix: DMUI
 *
 *****************************************************************************/
#include "dynamic_memory_usage_inference.h"

/*
 * Other includes go here
 */

#define DBUG_PREFIX "DMUI"
#include "debug.h"

#include "traverse.h"
#include "tree_basic.h"
#include "memory.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    bool needsDynamicMemory;
};

/**
 * A template entry in the template info structure
 */
#define INFO_NEEDS_DYNAMIC_MEMORY(n) (n->needsDynamicMemory)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_NEEDS_DYNAMIC_MEMORY (result) = FALSE;

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
 * @fn node *DMUIdoDynamicMemoryUsageInference( node *syntax_tree)
 *
 *****************************************************************************/
node *
DMUIdoDynamicMemoryUsageInference (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    DBUG_PRINT ("Starting dynamic memory usage inference traversal.");

    TRAVpush (TR_dmui);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_PRINT ("Dynamic memory usage inference traversal complete.");

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
 * @fn node *DMUIfundef(node *arg_node, info *arg_info)
 *
 * @brief Stack info and copy result of trav into fundef node
 *
 *****************************************************************************/
node *
DMUIfundef (node *arg_node, info *arg_info)
{
    info *stack;
    DBUG_ENTER ();

    stack = arg_info;
    arg_info = MakeInfo ();

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_PRINT ("%s needs dynamic memory: %s", FUNDEF_NAME (arg_node),
                INFO_NEEDS_DYNAMIC_MEMORY (arg_info) ? "yes" : "no");
    FUNDEF_NEEDSDYNAMICMEMORY (arg_node) = INFO_NEEDS_DYNAMIC_MEMORY (arg_info);

    arg_info = stack;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DMUIprf(node *arg_node, info *arg_info)
 *
 * @brief Does this prf perform a dynamic memory allocation?
 *
 *****************************************************************************/
node *
DMUIprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (PRF_PRF (arg_node)) {
    case F_alloc:
    case F_alloc_or_reuse:
    case F_alloc_or_reshape:
    case F_alloc_or_resize:
#if 0
    /*
     * Below may be needed for non mutc backends
     */
  case F_device2host:
  case F_host2device:
  case F_device2device:
#endif
        INFO_NEEDS_DYNAMIC_MEMORY (arg_info) = TRUE;
        break;
    default:
        break; /* Do nothing */
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/

#undef DBUG_PREFIX
