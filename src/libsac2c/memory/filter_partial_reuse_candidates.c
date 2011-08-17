/*
 * $Id: trav_template.c 15657 2007-11-13 13:57:30Z cg $
 */

/** <!--********************************************************************-->
 *
 * @defgroup fprc Filter Partial Reuse Candidates
 *
 * This module filters out invalid partial reuse candidates. A candidate is
 * invalid, if
 *
 * a) it is used inside the withloop for something other than an idx_sel
 *    operation. The original wrci phase has already checked that the
 *    candidate is only used in the body and not later and that all selections
 *    only use it with the plain index vector. However, ive might not have been
 *    able to remove the offset computation in which case the shape of the
 *    candidate might still be required. This, however, prohibits the reuse
 *    of the descriptor.
 *
 * b) the reuse candidate is a prefix of the result. This ensures that the
 *    memcopy operation, which is performed if resizing the reuse candidate
 *    cannot be done, has no adverse effect on performance.
 *
 * TODO: B NOT NOT CHECKED AT THE MOMENT!
 *
 * @ingroup fprc
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file filter_partial_reuse_candidates.c
 *
 * Prefix: FPRC
 *
 *****************************************************************************/
#include "filter_partial_reuse_candidates.h"

/*
 * Other includes go here
 */

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "traverse.h"
#include "tree_basic.h"
#include "memory.h"
#include "free.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *temp;
};

/**
 * A template entry in the template info structure
 */
#define INFO_TEMP(n) (n->temp)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_TEMP (result) = NULL;

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
 * @fn node *FPRCdoFilterPartialReuseCandidates( node *syntax_tree)
 *
 *****************************************************************************/
node *
FPRCdoFilterPartialReuseCandidates (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_fprc);
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
 * @fn node *FilterPRC(node *arg_node)
 *
 * @brief Removes all id nodes where IsUsed is set to true.
 *
 *****************************************************************************/
static node *
FilterPRC (node *arg_node)
{
    DBUG_ENTER ();

    if (arg_node != NULL) {
        EXPRS_NEXT (arg_node) = FilterPRC (EXPRS_NEXT (arg_node));

#if 0
    if (AVIS_ISUSED( ID_AVIS( EXPRS_EXPR( arg_node)))) {
      arg_node = FREEdoFreeNode( arg_node);
    }
#endif
    }

    DBUG_RETURN (arg_node);
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
 * @fn node *FPRCblock(node *arg_node, info *arg_info)
 *
 * @brief traverses the vardecs to reset the IsUsed info, then the body and then
 *        the vardecs again.
 *
 *****************************************************************************/
node *
FPRCblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    BLOCK_VARDECS (arg_node) = TRAVopt (BLOCK_VARDECS (arg_node), arg_info);

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    BLOCK_VARDECS (arg_node) = TRAVopt (BLOCK_VARDECS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FPRCavis(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
FPRCavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    AVIS_ISUSED (arg_node) = FALSE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FPRCassign(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
FPRCassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FPRCwith(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
FPRCwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);
    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
    WITH_WITHOP (arg_node) = TRAVopt (WITH_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FPRCfundef(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
FPRCgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    GENARRAY_RC (arg_node) = TRAVopt (GENARRAY_RC (arg_node), arg_info);

    GENARRAY_PRC (arg_node) = FilterPRC (GENARRAY_PRC (arg_node));

    GENARRAY_NEXT (arg_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FPRCid(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
FPRCid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    AVIS_ISUSED (ID_AVIS (arg_node)) = TRUE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FPRCprf(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
FPRCprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (PRF_PRF (arg_node)) {
    case F_idx_sel:
        break;

    default:
        PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Filter Partial Reuse Candidates -->
 *****************************************************************************/

#undef DBUG_PREFIX
