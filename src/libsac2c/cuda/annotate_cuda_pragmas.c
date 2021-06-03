/*****************************************************************************
 *
 * @defgroup
 *
 * description:
 *
 *   Niek will explain :-)
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file annotate_cuda_pragmas.c
 *
 * Prefix: ACP
 *
 *****************************************************************************/
#include "annotate_cuda_pragmas.h"

#include <stdlib.h>
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"
#include "globals.h"
#include "traverse.h"

#define DBUG_PREFIX "ACP"
#include "debug.h"


/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *pragma;
};

#define INFO_PRAGMA(n) n->pragma

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));
    INFO_PRAGMA (result) = NULL;

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
 * @fn node *ACPdoAnnotateCUDAPragmas( node *syntax_tree)
 *
 *****************************************************************************/
node *
ACPdoAnnotateCUDAPragmas (node *syntax_tree)
{
    info *info;
    DBUG_ENTER ();

    info = MakeInfo ();
    TRAVpush (TR_acp);
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
 * @name Static helper functions
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
 * @fn node *ACPpart (node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
ACPwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (WITH_CUDARIZABLE (arg_node)) {
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    } else {
        WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ACPpart (node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
ACPpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (PART_PRAGMA (arg_node) == NULL) {
        PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);
        DBUG_ASSERT (INFO_PRAGMA (arg_info)!=NULL, "failed to generate pragma "
                                                   "for partition");
        PART_PRAGMA (arg_node) = INFO_PRAGMA (arg_info);
        INFO_PRAGMA (arg_info) = NULL;
    }

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node* ACPgenerator (node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
ACPgenerator (node *arg_node, info *arg_info)
{
    node *lower_bound, *upper_bound;
    node *pragma;

    DBUG_ENTER ();

    lower_bound = GENERATOR_BOUND1 (arg_node);
    upper_bound = GENERATOR_BOUND2 (arg_node);

    pragma = TBmakePragma();
    PRAGMA_GPUKERNEL_APS (pragma) = TBmakeSpap (
                                      TBmakeSpid (NULL, STRcpy("BOO")),
                                      NULL);

    INFO_PRAGMA (arg_info) = pragma;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/

#undef DBUG_PREFIX
