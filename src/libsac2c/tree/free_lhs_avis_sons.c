/**
 *
 * This file frees N_avis son nodes associated with the lhs
 * nodes defined in the arg_node block.
 */

#include "tree_basic.h"
#include "tree_compound.h"
#include "types.h"
#include "new_types.h"

#define DBUG_PREFIX "FLAS"
#include "debug.h"

#include "free.h"
#include "traverse.h"

/** <!--********************************************************************-->
 *
 * @fn node FreeLhsAvisSons(node *codes)
 *
 *   @brief Free the N_avis son nodes of all LHS nodes defined in
 *          arg_node.
 *
 *          This situation arises when we have something like a
 *          WL partition of this nature:
 *
 *          with {
 *            ( lb <= iv < ub)
 *            ...
 *            k = 3;
 *            min = lb + k;
 *            stuff = iv + k;
 *          }
 *
 *          IVEXP introduced the min = lb + k;
 *          then sets AVIS_MIN( stuff) = TBmakeId( min);
 *
 *         When the WL code block is freed, the AVIS_MIN( stuff)
 *         setting remains unchanged, even though min no longer exists.
 *
 *         This is a design problem associated with having son nodes
 *         on N_avis entries, and I do not have any good ideas on
 *         how to resolve it cleanly.
 *
 *         It may be necessary to introduce calls to this function
 *         from other places in the compiler, until such time as we
 *         have a clean solution to the problem.
 *
 *
 *   @param  arg_node: N_code node
 *   @return modified N_code node
 *
 ******************************************************************************/
node *
FLASfreeLhsAvisSons (node *arg_node)
{
    node *z;

    DBUG_ENTER ();

    TRAVpush (TR_flas);
    arg_node = TRAVdo (arg_node, NULL);
    TRAVpop ();

    z = arg_node;

    DBUG_RETURN (z);
}

node *
FLASids (node *arg_node, info *arg_info)
{
    node *z;
    node *avis;

    DBUG_ENTER ();

    z = arg_node;

    avis = IDS_AVIS (arg_node);
    DBUG_PRINT ("Looking at avis for %s", AVIS_NAME (avis));

    if (NULL != AVIS_DIM (avis)) {
        AVIS_DIM (avis) = FREEdoFreeNode (AVIS_DIM (avis));
    }

    if (NULL != AVIS_SHAPE (avis)) {
        AVIS_SHAPE (avis) = FREEdoFreeNode (AVIS_SHAPE (avis));
    }

    if (NULL != AVIS_MIN (avis)) {
        AVIS_MIN (avis) = FREEdoFreeNode (AVIS_MIN (avis));
    }

    if (NULL != AVIS_MAX (avis)) {
        AVIS_MAX (avis) = FREEdoFreeNode (AVIS_MAX (avis));
    }

    if (NULL != AVIS_SCALARS (avis)) {
        AVIS_SCALARS (avis) = FREEdoFreeNode (AVIS_SCALARS (avis));
    }

    DBUG_RETURN (z);
}

#undef DBUG_PREFIX
