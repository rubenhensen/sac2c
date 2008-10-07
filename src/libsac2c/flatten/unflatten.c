/*
 * $Id: unflatten.c 15674 2009-09-24 15:10:11Z rbe $
 */

#include <stdio.h>

#include "globals.h"
#include "dbug.h"
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
#include "unflatten.h"

/*
 * This phase converts With-Loop index variables back into
 * non-SSA form. That is, the iv and other generator variables
 * for each partition in a WL are renamed to match the names
 * in the first partition of the WL.
 *
 * This renaming is required by the code generator,
 * and perhaps by other phases.
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

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_WITHID (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * function:
 *  node *UFLfundef(node *arg_node, info *arg_info)
 *
 * description:
 *  If we are running in SSA mode for WL generators,
 *  rename them back to non-SSA form so that remaining
 *  phases that depend on name-sharing among partitions
 *  can operate properly.
 *
 *  This function does NOT, at present, remove dead variables,
 *  as a DCR call follows immediately. If you want to use
 *  this phase elsewhere, it might be worthwhile to enhance
 *  this code to remove the dead varbs.
 *
 ******************************************************************************/

node *
UFLfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("UFLfundef");

    if (global.ssaiv) {
        if (NULL != FUNDEF_BODY (arg_node)) {
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        }

        /*
         * Proceed with the next function...
         */
        if (NULL != FUNDEF_NEXT (arg_node)) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *UFLwithid( node *arg_node, info *arg_info)
 *
 * description:
 *   If this is the first partition in the WL, save its
 *   withid as the source for renaming other partitions' withids.
 *
 ******************************************************************************/

node *
UFLwithid (node *arg_node, info *arg_info)
{
    node *curidxsp0;
    node *curidxs;

    DBUG_ENTER ("UFLwithid");

    if (NULL == INFO_WITHID (arg_info)) {
        /* First partition */
        INFO_WITHID (arg_info) = arg_node;
    } else {
        /* Set up renames for remaining partitions */
        if (NULL != WITHID_VEC (arg_node)) {
            AVIS_SUBST (IDS_AVIS (WITHID_VEC (arg_node)))
              = IDS_AVIS (WITHID_VEC (INFO_WITHID (arg_info)));
            DBUG_PRINT ("UFL",
                        ("Marking WITHID_VEC %s to be replaced by %s",
                         AVIS_NAME (IDS_AVIS (WITHID_VEC (arg_node))),
                         AVIS_NAME (AVIS_SUBST (IDS_AVIS (WITHID_VEC (arg_node))))));
        }

        curidxs = WITHID_IDS (arg_node);
        curidxsp0 = WITHID_IDS (INFO_WITHID (arg_info));
        while (NULL != curidxs) {
            AVIS_SUBST (IDS_AVIS (curidxs)) = IDS_AVIS (curidxsp0);
            DBUG_PRINT ("UFL", ("Marking WITHID_IDS %s to be replaced by %s",
                                AVIS_NAME (IDS_AVIS (curidxs)),
                                AVIS_NAME (AVIS_SUBST (IDS_AVIS (curidxs)))));
            curidxs = IDS_NEXT (curidxs);
            curidxsp0 = IDS_NEXT (curidxsp0);
        }

        /* Now, replace this partition's withid by the partition zero withid */
        if (NULL != WITHID_VEC (arg_node)) {
            FREEdoFreeTree (WITHID_VEC (arg_node));
            WITHID_VEC (arg_node) = DUPdoDupTree (WITHID_VEC (INFO_WITHID (arg_info)));
        }

        if (NULL != WITHID_IDS (arg_node)) {
            FREEdoFreeTree (WITHID_IDS (arg_node));
            WITHID_IDS (arg_node) = DUPdoDupTree (WITHID_IDS (INFO_WITHID (arg_info)));
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *UFLblock( node *arg_node, info *arg_info)
 *
 * description:
 *   Discard no-longer-referenced vardecs
 *   Bottom-up traversal simplifies chaining
 *
 ******************************************************************************/

node *
UFLblock (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("UFLblock");

    /* Mark unreferenced variables */
    if (NULL != BLOCK_INSTR (arg_node)) {
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    /* Delete unreferenced vardecs */
    if (NULL != BLOCK_VARDEC (arg_node)) {
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *UFLvardec( node *arg_node, info *arg_info)
 *
 * description:
 *   Discard no-longer-referenced vardecs
 *   Bottom-up traversal simplifies chaining
 *
 ******************************************************************************/

node *
UFLvardec (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("UFLvardec");
    if (NULL != VARDEC_NEXT (arg_node)) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    if ((NULL == VARDEC_AVIS (arg_node))
        || (NULL != AVIS_SUBST (VARDEC_AVIS (arg_node)))) {
        DBUG_PRINT ("UFL",
                    ("Deleting vardec for %s", AVIS_NAME (VARDEC_AVIS (arg_node))));
        arg_node = FREEdoFreeNode (arg_node);
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *UFLid( node *arg_node, info *arg_info)
 *
 * description:
 *   Possibly rename one name in a withid.
 *
 ******************************************************************************/

node *
UFLid (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("UFLid");

    if (NULL != AVIS_SUBST (ID_AVIS (arg_node))) {
        DBUG_PRINT ("UFL", ("Renaming %s to %s", AVIS_NAME (ID_AVIS (arg_node)),
                            AVIS_NAME (AVIS_SUBST (ID_AVIS (arg_node)))));
        ID_AVIS (arg_node) = AVIS_SUBST (ID_AVIS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *UFLpart( node *arg_node, info *arg_info)
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
UFLpart (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("UFLpart");

    if (NULL != arg_node) {
        PART_WITHID (arg_node) = TRAVdo (PART_WITHID (arg_node), arg_info);
        if (NULL != PART_NEXT (arg_node)) {
            PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *UFLwith( node *arg_node, info *arg_info)
 *
 * Description:
 *  Rename all partition withids to match those of the
 *  first partition.
 *
 ******************************************************************************/

node *
UFLwith (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("UFLwith");

    if (NULL != arg_node) {
        if (NULL != PART_NEXT (WITH_PART (arg_node))) {
            /* Set up the various AVIS_SUBST fields for next traversal */
            WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

            /* Now, rename the withids in code fragments */
            WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        }
    }
    INFO_WITHID (arg_info) = NULL; /* done with this WL */

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *UFLdoUnflattenWLGenerators(node *arg_node)
 *
 * description:
 *   converts WL generators back to non-SSA form, to keep the
 *   back-end happy.
 *
 ******************************************************************************/

node *
UFLdoUnflattenWLGenerators (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("UFLdoUnflattenWLGenerators");

    arg_info = MakeInfo ();

    TRAVpush (TR_ufl);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}
