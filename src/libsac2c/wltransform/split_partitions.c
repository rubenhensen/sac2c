
#include "dbug.h"

#include "globals.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "DupTree.h"
#include "free.h"
#include "namespaces.h"
#include "new_types.h"
#include "shape.h"
#include "LookUpTable.h"
#include "pattern_match.h"
#include "dbug.h"
#include "constants.h"
#include "types.h"

#include "split_partitions.h"

#define MAX_BLOCK_THREADS 512
#define OPTIMAL_BLOCK_THREADS 256

/**
 * INFO structure
 */
struct INFO {
    node *part;
    node *new_parts;
};

/**
 * INFO macros
 */
#define INFO_PART(n) (n->part)
#define INFO_NEW_PARTS(n) (n->new_parts)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_PART (result) = NULL;
    INFO_NEW_PARTS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

static node *
getArrayLastElement (node *array)
{
    node *last_elem;
    node *array_elems;
    int elem_count;

    DBUG_ENTER ("getArrayLastElement");

    array_elems = ARRAY_AELEMS (array);
    elem_count = TCcountExprs (array_elems);

    last_elem = TCgetNthExprsExpr (elem_count - 1, array_elems);

    DBUG_RETURN (last_elem);
}

static void
checkGenerator (node *lb, node *ub, node *step, node *width, node **lb_array,
                node **ub_array, node **step_array, node **width_array)
{
    pattern *pat;
    node *array;

    DBUG_ENTER ("checkGenerator");

    pat = PMarray (1, PMAgetNode (&array), 1, PMskip (0));

    if (PMmatchFlat (pat, lb)) {
        *lb_array = array;
        /* Since we are after constant propogation, we expect
         * the bounds of an AKS N_with (which is also a cudarizable
         * N_with) to be constant arrays */
        DBUG_ASSERT ((COisConstant (*lb_array)), "Lower bound must be constant!");
    } else {
        DBUG_ASSERT (FALSE, ("Non N_array node found for lower bound!"));
    }

    if (PMmatchFlat (pat, ub)) {
        *ub_array = array;
        DBUG_ASSERT ((COisConstant (*ub_array)), "Upper bound must be constant!");
    } else {
        DBUG_ASSERT (FALSE, ("Non N_array node found for upper bound!"));
    }

    if (step != NULL) {
        if (PMmatchFlat (pat, step)) {
            *step_array = array;
            DBUG_ASSERT ((COisConstant (*step_array)), "Step must be constant!");
        } else {
            DBUG_ASSERT (FALSE, ("Non N_array node found for step!"));
        }
    }

    if (width != NULL) {
        if (PMmatchFlat (pat, width)) {
            *width_array = array;
            DBUG_ASSERT ((COisConstant (*width_array)), "Width must be constant!");
        } else {
            DBUG_ASSERT (FALSE, ("Non N_array node found for width!"));
        }
    }

    DBUG_VOID_RETURN;
}

/** <!--**********************************************************************
 *
 * @fn node *SPTNdoSplitPartitions( node *syntax_tree)
 *
 *
 * @param
 *
 * @return
 *
 *****************************************************************************/
node *
SPTNdoSplitPartitions (node *arg_node)
{
    info *info_node;

    DBUG_ENTER ("SPTNdoSplitPartitions");

    info_node = MakeInfo ();

    TRAVpush (TR_sptn);
    arg_node = TRAVdo (arg_node, info_node);
    TRAVpop ();

    info_node = FreeInfo (info_node);

    DBUG_RETURN (arg_node);
}

/** <!--**********************************************************************
 *
 * @fn node *SPTNwith( node *arg_node, info *arg_info)
 *
 *
 * @param
 *
 * @return
 *
 *****************************************************************************/
node *
SPTNwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPTNwith");

    if (WITH_CUDARIZABLE (arg_node)) {
        WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--**********************************************************************
 *
 * @fn node *SPTNpart( node *arg_node, info *arg_info)
 *
 *
 * @param
 *
 * @return
 *
 *****************************************************************************/
node *
SPTNpart (node *arg_node, info *arg_info)
{
    node *with_ids;

    DBUG_ENTER ("SPTNpart");

    with_ids = WITHID_IDS (PART_WITHID (arg_node));

    /* For the experiment, we only look at partition with
     * dimentionality 3 */
    if (TCcountIds (with_ids) == 3) {
        INFO_PART (arg_info) = arg_node;
        PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);
        INFO_PART (arg_info) = NULL;
    }

    if (PART_NEXT (arg_node) != NULL) {
        PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
    } else {
        /* Append new partitions to the N_with */
        PART_NEXT (arg_node) = INFO_NEW_PARTS (arg_info);
        INFO_NEW_PARTS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--**********************************************************************
 *
 * @fn node *SPTNgenerator( node *arg_node, info *arg_info)
 *
 *
 * @param
 *
 * @return
 *
 *****************************************************************************/
node *
SPTNgenerator (node *arg_node, info *arg_info)
{
    node *lb, *ub, *step, *width;
    node *lb_array, *ub_array, *step_array, *width_array;
    node *last_lb_elem, *last_ub_elem;
    int trip_count, partition_count, remaining_trip_count;
    int next_part_lb, i;
    node *old_partition, *new_withid, *new_generator, *new_partition;

    DBUG_ENTER ("SPTNgenerator");

    lb = GENERATOR_BOUND1 (arg_node);
    ub = GENERATOR_BOUND2 (arg_node);
    step = GENERATOR_STEP (arg_node);
    width = GENERATOR_WIDTH (arg_node);

    checkGenerator (lb, ub, step, width, &lb_array, &ub_array, &step_array, &width_array);

    /* We only have to manipulate the lower bound and the upper bound */
    if (step == NULL) {
        DBUG_ASSERT ((width == NULL), "step is NULL while width is not NULL!");

        /* We find the last element in the bound array.
         * This is the dimention we want to check against
         * (only in case of 3D N_with) */
        last_lb_elem = getArrayLastElement (lb_array);
        last_ub_elem = getArrayLastElement (ub_array);

        DBUG_ASSERT ((NODE_TYPE (last_lb_elem) == N_num
                      && NODE_TYPE (last_ub_elem) == N_num),
                     "Non constant found in the last element of bounds!");

        /* We need to split the partition if the trip count of the last
         * dimention is greater than MAX_BLOCK_THREADS (512) */
        trip_count = NUM_VAL (last_ub_elem) - NUM_VAL (last_lb_elem);

        if (trip_count > MAX_BLOCK_THREADS) {
            /* Get the orginal N_part that this N_generator belongs to */
            old_partition = INFO_PART (arg_info);

            remaining_trip_count = trip_count % OPTIMAL_BLOCK_THREADS;
            partition_count
              = ((remaining_trip_count == 0) ? trip_count / OPTIMAL_BLOCK_THREADS
                                             : trip_count / OPTIMAL_BLOCK_THREADS + 1);

            NUM_VAL (last_ub_elem) = NUM_VAL (last_lb_elem) + OPTIMAL_BLOCK_THREADS;
            next_part_lb = NUM_VAL (last_ub_elem);

            /* The reason we start from 1 here is because the orginal
             * partiton is 'reused' as one of the partitions */
            i = 1;
            while (i < partition_count) {
                lb_array = DUPdoDupNode (lb_array);
                ub_array = DUPdoDupNode (ub_array);
                last_lb_elem = getArrayLastElement (lb_array);
                last_ub_elem = getArrayLastElement (ub_array);

                NUM_VAL (last_lb_elem) = next_part_lb;
                if (i == partition_count - 1 && remaining_trip_count != 0) {
                    NUM_VAL (last_ub_elem) = next_part_lb + remaining_trip_count;
                } else {
                    NUM_VAL (last_ub_elem) = next_part_lb + OPTIMAL_BLOCK_THREADS;
                }
                next_part_lb = NUM_VAL (last_ub_elem);

                new_generator
                  = TBmakeGenerator (F_wl_le, F_wl_lt, lb_array, ub_array, NULL, NULL);
                new_withid = DUPdoDupNode (PART_WITHID (old_partition));
                new_partition
                  = TBmakePart (PART_CODE (old_partition), new_withid, new_generator);
                CODE_USED (PART_CODE (old_partition))++;

                PART_NEXT (new_partition) = INFO_NEW_PARTS (arg_info);
                INFO_NEW_PARTS (arg_info) = new_partition;

                i++;
            }
        }
    } else {
        /* Ooops, there's step and width, more
         * sophisticated analysis is required */
    }

    DBUG_RETURN (arg_node);
}
