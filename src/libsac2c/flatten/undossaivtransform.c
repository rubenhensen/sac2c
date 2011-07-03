/*
 * $Id: undossaivtransform.c 15674 2009-09-24 15:10:11Z rbe $
 */

#include <stdio.h>

#include "globals.h"

#define DBUG_PREFIX "USSAI"
#include "debug.h"

#include "types.h"
#include "str.h"
#include "memory.h"
#include "tree_basic.h"
#include "node_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "ctinfo.h"
#include "undossaivtransform.h"
#include "constants.h"
#include "pattern_match.h"

/*
 * This phase converts With-Loop index variables back into
 * non-SSA form. That is, the iv and other generator variables
 * for each partition in a WL are renamed to match the names
 * in the first partition of the WL.
 *
 * This renaming is required by the code generator,
 * and perhaps by other phases that run after the optimizers.
 *
 *  2009-07-15: This code is a placeholder - code was never completely
 *  tested.
 */

/**
 * INFO structure
 */
struct INFO {
    node *withid; /* Pointer to withid for first partition */
};

/**
 * INFO macros
 */
#define INFO_WITHID(n) (n->withid)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_WITHID (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * function:
 *  node *USSAIfundef(node *arg_node, info *arg_info)
 *
 * description:
 *  If we are running in ssaiv mode for WL WITHIDS,
 *  rename them back to non-SSA form so that remaining
 *  phases that depend on name-sharing among partitions
 *  can operate properly.
 *
 ******************************************************************************/

node *
USSAIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NULL != FUNDEF_BODY (arg_node)) {
        DBUG_PRINT ("Unflattening function: %s", FUNDEF_NAME (arg_node));
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    /*
     * Proceed with the next function...
     */
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *USSAIwithid( node *arg_node, info *arg_info)
 *
 * description:
 *   If this is the first partition in the WL, save its
 *   withid as the source for renaming other partitions' withids.
 *
 ******************************************************************************/

node *
USSAIwithid (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

#ifdef ssaiv
    node *curidxsp0;
    node *curidxs;

    if ssaiv ever gets working, you might want to reuse/review some of this code.

  In particular, it renames iv -> iv, and then deletes the vardec
  for iv. This is not desirable.

  if ( NULL == INFO_WITHID( arg_info))
        {
            /* First partition */
            INFO_WITHID (arg_info) = arg_node;
        }
    else {
        /* Set up renames for remaining partitions */
        if (NULL != WITHID_VEC (arg_node)) {
            AVIS_SUBST (IDS_AVIS (WITHID_VEC (arg_node)))
              = IDS_AVIS (WITHID_VEC (INFO_WITHID (arg_info)));
            DBUG_PRINT ("Marking WITHID_VEC %s to be replaced by %s",
                        AVIS_NAME (IDS_AVIS (WITHID_VEC (arg_node))),
                        AVIS_NAME (AVIS_SUBST (IDS_AVIS (WITHID_VEC (arg_node)))));
        }

        curidxs = WITHID_IDS (arg_node);
        curidxsp0 = WITHID_IDS (INFO_WITHID (arg_info));
        while (NULL != curidxs) {
            AVIS_SUBST (IDS_AVIS (curidxs)) = IDS_AVIS (curidxsp0);
            DBUG_PRINT ("Marking WITHID_IDS %s to be replaced by %s",
                        AVIS_NAME (IDS_AVIS (curidxs)),
                        AVIS_NAME (AVIS_SUBST (IDS_AVIS (curidxs))));
            curidxs = IDS_NEXT (curidxs);
            curidxsp0 = IDS_NEXT (curidxsp0);
        }
#ifdef FIXME
        I think the WITHID_IDXS varbs should remain name - invariant over SSA,
          because they represent FOR
            - loop index vars that are shared by all partitions of the WL.

              curidxs
          = WITHID_IDXS (arg_node);
        curidxsp0 = WITHID_IDXS (INFO_WITHID (arg_info));
        while (NULL != curidxs) {
            AVIS_SUBST (IDS_AVIS (curidxs)) = IDS_AVIS (curidxsp0);
            DBUG_PRINT ("Marking WITHID_IDXS %s to be replaced by %s",
                        AVIS_NAME (IDS_AVIS (curidxs)),
                        AVIS_NAME (AVIS_SUBST (IDS_AVIS (curidxs))));
            curidxs = IDS_NEXT (curidxs);
            curidxsp0 = IDS_NEXT (curidxsp0);
        }
#endif // FIXME

        /* Now, replace this partition's withid elements
         * by the partition zero withid
         */
        if (NULL != WITHID_VEC (arg_node)) {
            FREEdoFreeTree (WITHID_VEC (arg_node));
            WITHID_VEC (arg_node) = DUPdoDupTree (WITHID_VEC (INFO_WITHID (arg_info)));
        }

        if (NULL != WITHID_IDS (arg_node)) {
            FREEdoFreeTree (WITHID_IDS (arg_node));
            WITHID_IDS (arg_node) = DUPdoDupTree (WITHID_IDS (INFO_WITHID (arg_info)));
        }

#ifdef FIXME
        See above

          if (NULL != WITHID_IDXS (arg_node))
        {
            FREEdoFreeTree (WITHID_IDXS (arg_node));
            WITHID_IDXS (arg_node) = DUPdoDupTree (WITHID_IDXS (INFO_WITHID (arg_info)));
        }
#endif // FIXME
    }

#endif // ssaiv

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *USSAIblock( node *arg_node, info *arg_info)
 *
 * description:
 *   Discard no-longer-referenced vardecs
 *   Bottom-up traversal simplifies chaining
 *
 ******************************************************************************/

node *
USSAIblock (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    /* Mark unreferenced variables; look for WL generators  */
    BLOCK_INSTR (arg_node) = TRAVopt (BLOCK_INSTR (arg_node), arg_info);

#ifdef ssaiv
    /* Delete unreferenced vardecs */
    BLOCK_VARDEC (arg_node) = TRAVopt (BLOCK_VARDEC (arg_node), arg_info);
#endif //  ssaiv

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *USSAIvardec( node *arg_node, info *arg_info)
 *
 * description:
 *   Discard no-longer-referenced vardecs
 *   Bottom-up traversal simplifies chaining
 *
 ******************************************************************************/

node *
USSAIvardec (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

#ifdef ssaiv
    VARDEC_NEXT (arg_node) = TRAVopt (VARDEC_NEXT (arg_node), arg_info);

    if ((NULL == VARDEC_AVIS (arg_node))
        || (NULL != AVIS_SUBST (VARDEC_AVIS (arg_node)))) {
        DBUG_PRINT ("Deleting vardec for %s", AVIS_NAME (VARDEC_AVIS (arg_node)));
        arg_node = FREEdoFreeNode (arg_node);
    }
#endif //  ssaiv

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *USSAIid( node *arg_node, info *arg_info)
 *
 * description:
 *   Possibly rename one name in a withid.
 *
 ******************************************************************************/

node *
USSAIid (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

#ifdef ssaiv
    if (NULL != AVIS_SUBST (ID_AVIS (arg_node))) {
        DBUG_PRINT ("Renaming %s to %s", AVIS_NAME (ID_AVIS (arg_node)),
                    AVIS_NAME (AVIS_SUBST (ID_AVIS (arg_node))));
        ID_AVIS (arg_node) = AVIS_SUBST (ID_AVIS (arg_node));
    }

#endif // ssaiv
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *USSAIpart( node *arg_node, info *arg_info)
 *
 * description:
 *   unflattens all N_part nodes
 *
 * remark:
 *   withid are all renamed to match those in the first
 *   partition.
 *
 ******************************************************************************/

node *
USSAIpart (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    if (NULL != arg_node) {
        PART_WITHID (arg_node) = TRAVdo (PART_WITHID (arg_node), arg_info);
        PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);
        PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *USSAIwith( node *arg_node, info *arg_info)
 *
 * Description:
 *  Rename all partition withids to match those of the
 *  first partition.
 *
 ******************************************************************************/

node *
USSAIwith (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);

    /* Now, rename the withids in code fragments */
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *USSAIdoUndoSSAivTransform(node *arg_node)
 *
 * description:
 *   converts WL generators back to non-SSA form, to keep the
 *   back-end happy.
 *
 ******************************************************************************/

node *
USSAIdoUndoSSAivTransform (node *arg_node)
{
    info *arg_info = NULL;

    DBUG_ENTER ();

    TRAVpush (TR_ussai);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
