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
#include "constants.h"
#include "pattern_match.h"

/*
 * This phase converts With-Loop index variables back into
 * non-SSA form. That is, the iv and other generator variables
 * for each partition in a WL are renamed to match the names
 * in the first partition of the WL.
 *
 * This phase ALSO unflattens WL bounds, so that they
 * are (mostly?) N_array nodes, rather than the N_id nodes
 * that point to those N_array nodes.
 *
 * This renaming is required by the code generator,
 * and other phases that run after the optimizers.
 *
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
            DBUG_PRINT ("UFL", ("Unflattening function: %s", FUNDEF_NAME (arg_node)));
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
#ifdef FIXME
        I think the WITHID_IDXS varbs should remain name - invariant over SSA,
          because they represent FOR
            - loop index vars that are shared by all partitions of the WL.

              curidxs
          = WITHID_IDXS (arg_node);
        curidxsp0 = WITHID_IDXS (INFO_WITHID (arg_info));
        while (NULL != curidxs) {
            AVIS_SUBST (IDS_AVIS (curidxs)) = IDS_AVIS (curidxsp0);
            DBUG_PRINT ("UFL", ("Marking WITHID_IDXS %s to be replaced by %s",
                                AVIS_NAME (IDS_AVIS (curidxs)),
                                AVIS_NAME (AVIS_SUBST (IDS_AVIS (curidxs)))));
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
 *   node *UFLgenerator( node *arg_node, info *arg_info)
 *
 * description:
 *   unflattens one WL generator:
 *    Case 1: Replaces N_id nodes in BOUNDs by the N_array nodes they point to.
 *    Case 2: If N_id nodes do not point to N_arrays, but are AKV,
 *            then build an N_array containing the constant.
 *    Case 3: N_array nodes: These are not supposed to exist any more.
 *
 *    The latter can occur when the bound is a lac-fn argument.
 *
 * TODO: Perhaps should also handle STEP and WIDTH?
 *
 ******************************************************************************/

node *
UFLgenerator (node *arg_node, info *arg_info)
{
    node *lb = NULL;
    node *ub = NULL;
    constant *lbfs = NULL;
    constant *ubfs = NULL;

    DBUG_ENTER ("UFLgenerator");

#ifdef REALLYFLATTENED // as of 2009-01-22, this is still being violated
    DBUG_ASSERT (N_id == NODE_TYPE (GENERATOR_BOUND1 (arg_node)),
                 "UFLgenerator expected N_id node for GENERATOR_BOUND1");
    DBUG_ASSERT (N_id == NODE_TYPE (GENERATOR_BOUND2 (arg_node)),
                 "UFLgenerator expected N_id node for GENERATOR_BOUND2");
#endif // REALLYFLATTENED

    if (NULL != arg_node) {
        if (PM (PMarray (&lbfs, &lb, GENERATOR_BOUND1 (arg_node)))) {
            COfreeConstant (lbfs);
            lb = DUPdoDupTree (lb);
            FREEdoFreeTree (GENERATOR_BOUND1 (arg_node));
            GENERATOR_BOUND1 (arg_node) = lb;
        } else {
            if (PM (PMconst (&lbfs, &lb, GENERATOR_BOUND1 (arg_node)))) {
                FREEdoFreeTree (GENERATOR_BOUND1 (arg_node));
                GENERATOR_BOUND1 (arg_node) = COconstant2AST (lbfs);
                lbfs = COfreeConstant (lbfs);
            }
        }

        if ((PM (PMarray (&ubfs, &ub, GENERATOR_BOUND2 (arg_node))))) {
            COfreeConstant (ubfs);
            ub = DUPdoDupTree (ub);
            FREEdoFreeTree (GENERATOR_BOUND2 (arg_node));
            GENERATOR_BOUND2 (arg_node) = ub;
        } else {
            if (PM (PMconst (&ubfs, &ub, GENERATOR_BOUND2 (arg_node)))) {
                FREEdoFreeTree (GENERATOR_BOUND2 (arg_node));
                GENERATOR_BOUND2 (arg_node) = COconstant2AST (ubfs);
                lbfs = COfreeConstant (ubfs);
            }
        }
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
        PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);
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

    arg_info = MakeInfo ();

    if (NULL != arg_node) {
        if (NULL != WITH_PART (arg_node)) {
            WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

            /* Now, rename the withids in code fragments */
            WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        }
    }
    arg_info = FreeInfo (arg_info);

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
    info *arg_info = NULL;

    DBUG_ENTER ("UFLdoUnflattenWLGenerators");

    TRAVpush (TR_ufl);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    DBUG_RETURN (arg_node);
}
