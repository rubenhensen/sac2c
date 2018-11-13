

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "split_partitions.h"

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
#include "constants.h"
#include "types.h"

typedef struct seg {
    int offset;
    int extent;
    struct seg *next;
} seg_t;

typedef struct partition {
    size_t segs_cnt;
    int extents[3];
    seg_t *segs[3];
} partition_t;

#define MAX_BLOCK_THREADS 512
#define OPTIMAL_SEG_EXTENT_3D 256
#define OPTIMAL_SEG_EXTENT_4D 16
#define OPTIMAL_SEG_EXTENT_5D 8

static const int optimal_seg_extents[3]
  = {OPTIMAL_SEG_EXTENT_3D, OPTIMAL_SEG_EXTENT_4D, OPTIMAL_SEG_EXTENT_5D};

#define SET_DIM(lb, ub, i, seg_iter)                                                     \
    NUM_VAL (EXPRS_EXPR##i (ARRAY_AELEMS (lb))) = SEG_OFFSET (seg_iter);                 \
    NUM_VAL (EXPRS_EXPR##i (ARRAY_AELEMS (ub)))                                          \
      = SEG_OFFSET (seg_iter) + SEG_EXTENT (seg_iter);

#define OPTIMAL_SEG_EXTENT(dims) (optimal_seg_extents[dims - 3])

#define SEG_OFFSET(s) (s->offset)
#define SEG_EXTENT(s) (s->extent)
#define SEG_NEXT(s) (s->next)

#define PARTITION_SEGS_CNT(p) (p->segs_cnt)
#define PARTITION_SEG(p, i) (p->segs[i])
#define PARTITION_EXTENT(p, i) (p->extents[i])

/**
 * INFO structure
 */
struct INFO {
    node *part;
    node *new_parts;
    size_t wl_dim;
};

/**
 * INFO macros
 */
#define INFO_PART(n) (n->part)
#define INFO_NEW_PARTS(n) (n->new_parts)
#define INFO_WL_DIM(n) (n->wl_dim)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_PART (result) = NULL;
    INFO_NEW_PARTS (result) = NULL;
    INFO_WL_DIM (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--**********************************************************************
 *
 * @fn void CheckGeneratorHelper( node *bound, **bound_array)
 *
 *
 * @param
 *
 * @return
 *
 *****************************************************************************/
static node *
CheckAndGetBound (node *bound)
{
    pattern *pat;
    node *array = NULL;

    DBUG_ENTER ();

    pat = PMarray (1, PMAgetNode (&array), 1, PMskip (0));

    if (PMmatchFlat (pat, bound)) {
        /* Since we are after constant propogation, we expect
         * the bounds of an AKS N_with (which is also a cudarizable
         * N_with) to be constant arrays */
        DBUG_ASSERT (COisConstant (array),
                     "N_gnerator must be contain only constant N_array!");
    } else {
        DBUG_UNREACHABLE ("Non constant N_array node found in N_generator!");
    }

    pat = PMfree (pat);

    DBUG_RETURN (array);
}

/** <!--**********************************************************************
 *
 * @fn seg_t *MakeSeg( seg_t *seg, int offset, int extent)
 *
 *
 * @param
 *
 * @return
 *
 *****************************************************************************/
static seg_t *
MakeSeg (seg_t *seg, int offset, int extent)
{
    seg_t *new_seg;

    DBUG_ENTER ();

    new_seg = (seg_t *)MEMmalloc (sizeof (seg_t));

    new_seg->offset = offset;
    new_seg->extent = extent;
    new_seg->next = seg;

    DBUG_RETURN (new_seg);
}

/** <!--**********************************************************************
 *
 * @fn seg_t *FreeSeg( seg_t *seg)
 *
 *
 * @param
 *
 * @return
 *
 *****************************************************************************/
static seg_t *
FreeSeg (seg_t *seg)
{
    DBUG_ENTER ();

    if (seg != NULL) {
        if (SEG_NEXT (seg) != NULL) {
            SEG_NEXT (seg) = FreeSeg (SEG_NEXT (seg));
        }
        seg = MEMfree (seg);
    }

    DBUG_RETURN (seg);
}

/** <!--**********************************************************************
 *
 * @fn partition_t *MakePartition( int segs_cnt)
 *
 *
 * @param
 *
 * @return
 *
 *****************************************************************************/
static partition_t *
MakePartition (size_t segs_cnt)
{
    partition_t *new_part;

    DBUG_ENTER ();

    new_part = (partition_t *)MEMmalloc (sizeof (partition_t));
    // FIXME grzegorz: may be good to use macros here to be consistent 
    PARTITION_SEGS_CNT (new_part) = segs_cnt;

    new_part->segs[0] = NULL;
    new_part->segs[1] = NULL;
    new_part->segs[2] = NULL;
    new_part->extents[0] = 0;
    new_part->extents[1] = 0;
    new_part->extents[2] = 0;

    DBUG_RETURN (new_part);
}

/** <!--**********************************************************************
 *
 * @fn partition_t *FreePartition( partition *part)
 *
 *
 * @param
 *
 * @return
 *
 *****************************************************************************/
static partition_t *
FreePartition (partition_t *part)
{
    DBUG_ENTER ();

    if (part != NULL) {
        size_t i = 0;
        while (i < PARTITION_SEGS_CNT (part)) {
            part->segs[i] = FreeSeg (part->segs[i]);
            i++;
        }
        part = MEMfree (part);
    }

    DBUG_RETURN (part);
}

/** <!--**********************************************************************
 *
 * @fn bool PartitionNeedsSplit( partition_t *part)
 *
 *
 * @param
 *
 * @return
 *
 *****************************************************************************/
static bool
PartitionNeedsSplit (partition_t *part)
{
    int total_volume = 1;
    size_t i = 0;
    bool res;

    DBUG_ENTER ();

    while (i < PARTITION_SEGS_CNT (part)) {
        total_volume *= PARTITION_EXTENT (part, i);
        i++;
    }

    res = (total_volume > MAX_BLOCK_THREADS);

    DBUG_RETURN (res);
}

/** <!--**********************************************************************
 *
 * @fn partition_t *CreatePartitionsAndSegs( node *lb, node *ub, int dims)
 *
 *
 * @param
 *
 * @return
 *
 *****************************************************************************/
static partition_t *
CreatePartitionsAndSegs (node *lb, node *ub, node *step, node *width, size_t dims)
{
    partition_t *part;
    seg_t *segs = NULL;
    node *lb_rem_dims = NULL, *ub_rem_dims = NULL;
    node *step_rem_dims = NULL, *width_rem_dims = NULL;
    int opt_seg_ext;
    int lb_num, ub_num, step_num; // width_num;
    bool has_step_width = FALSE;

    DBUG_ENTER ();

    part = MakePartition (dims - 2);

    opt_seg_ext = OPTIMAL_SEG_EXTENT (dims);

    lb_rem_dims = EXPRS_EXPRS3 (ARRAY_AELEMS (lb));
    ub_rem_dims = EXPRS_EXPRS3 (ARRAY_AELEMS (ub));

    if (step != NULL) {
        has_step_width = TRUE;
        step_rem_dims = EXPRS_EXPRS3 (ARRAY_AELEMS (step));
        width_rem_dims = EXPRS_EXPRS3 (ARRAY_AELEMS (width));
    }

    int i = 0;
    while (lb_rem_dims != NULL) {
        DBUG_ASSERT (ub_rem_dims != NULL,
                     "Lower bound and upper bound have different number of elements!");

        if (has_step_width) {
            DBUG_ASSERT ((step_rem_dims != NULL && width_rem_dims != NULL),
                         "Step and width have different number of elements as bounds!");
        }

        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (lb_rem_dims)) == N_num
                      && NODE_TYPE (EXPRS_EXPR (ub_rem_dims)) == N_num),
                     "Non constant found in the elements of lower or upper bounds!");

        lb_num = NUM_VAL (EXPRS_EXPR (lb_rem_dims));
        ub_num = NUM_VAL (EXPRS_EXPR (ub_rem_dims));

        if (has_step_width) {
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (step_rem_dims)) == N_num
                          && NODE_TYPE (EXPRS_EXPR (width_rem_dims)) == N_num),
                         "Non constant found in the elements of step or width!");
            step_num = NUM_VAL (EXPRS_EXPR (step_rem_dims));
            // width_num = NUM_VAL( EXPRS_EXPR( width_rem_dims));
            opt_seg_ext = ((int)(opt_seg_ext / step_num)) * step_num;
        }

        PARTITION_EXTENT (part, i) = ub_num - lb_num;

        int extent;
        while (lb_num < ub_num) {
            extent = ((lb_num + opt_seg_ext) > ub_num ? (ub_num - lb_num) : opt_seg_ext);
            segs = MakeSeg (segs, lb_num, extent);
            lb_num += extent;
        }

        PARTITION_SEG (part, i) = segs;
        segs = NULL;

        lb_rem_dims = EXPRS_NEXT (lb_rem_dims);
        ub_rem_dims = EXPRS_NEXT (ub_rem_dims);
        if (has_step_width) {
            step_rem_dims = EXPRS_NEXT (step_rem_dims);
            width_rem_dims = EXPRS_NEXT (width_rem_dims);
        }

        i++;
    }
    DBUG_RETURN (part);
}

/** <!--**********************************************************************
 *
 * @fn void CreateWithloopPartitionsHelper( node *lb, node *ub, info* arg_info)
 *
 *
 * @param
 *
 * @return
 *
 *****************************************************************************/
static void
CreateWithloopPartitionsHelper (node *lb, node *ub, node *step, node *width,
                                info *arg_info)
{
    node *new_generator, *new_withid, *new_partition, *old_partition;

    DBUG_ENTER ();

    old_partition = INFO_PART (arg_info);

    new_generator = TBmakeGenerator (F_wl_le, F_wl_lt, lb, ub, step, width);
    new_withid = DUPdoDupNode (PART_WITHID (old_partition));
    new_partition = TBmakePart (PART_CODE (old_partition), new_withid, new_generator);
    CODE_USED (PART_CODE (old_partition))++;

    PART_NEXT (new_partition) = INFO_NEW_PARTS (arg_info);
    INFO_NEW_PARTS (arg_info) = new_partition;

    DBUG_RETURN ();
}

/** <!--**********************************************************************
 *
 * @fn void CreateWithloopPartitions( node *lb_array, node *ub_array,
 *                                    partition_t *part, info* arg_info)
 *
 *
 * @param
 *
 * @return
 *
 *****************************************************************************/
static void
CreateWithloopPartitions (node *lb_array, node *ub_array, node *step_array,
                          node *width_array, partition_t *part, info *arg_info)
{
    node *lb, *ub, *step = NULL, *width = NULL;
    seg_t *seg1, *seg2, *seg3;
    seg_t *seg1_iter, *seg2_iter, *seg3_iter;

    DBUG_ENTER ();

    if (PARTITION_SEGS_CNT (part) == 1) { /* For 3D N_with */
        seg1 = PARTITION_SEG (part, 0);

        DBUG_ASSERT (seg1 != NULL, "Found partition with NULL segment!");

        seg1_iter = seg1;
        while (seg1_iter != NULL) {
            lb = DUPdoDupNode (lb_array);
            ub = DUPdoDupNode (ub_array);

            if (step_array != NULL) {
                step = DUPdoDupNode (step_array);
                width = DUPdoDupNode (width_array);
            }

            SET_DIM (lb, ub, 3, seg1_iter)

            CreateWithloopPartitionsHelper (lb, ub, step, width, arg_info);

            seg1_iter = SEG_NEXT (seg1_iter);
        }
    } else if (PARTITION_SEGS_CNT (part) == 2) { /* For 4D N_with */
        seg1 = PARTITION_SEG (part, 0);
        seg2 = PARTITION_SEG (part, 1);

        DBUG_ASSERT ((seg1 != NULL && seg2 != NULL),
                     "Found partition with NULL segment!");

        seg1_iter = seg1;
        while (seg1_iter != NULL) {
            seg2_iter = seg2;
            while (seg2_iter != NULL) {
                lb = DUPdoDupNode (lb_array);
                ub = DUPdoDupNode (ub_array);
                if (step_array != NULL) {
                    step = DUPdoDupNode (step_array);
                    width = DUPdoDupNode (width_array);
                }

                SET_DIM (lb, ub, 3, seg1_iter)
                SET_DIM (lb, ub, 4, seg2_iter)

                CreateWithloopPartitionsHelper (lb, ub, step, width, arg_info);

                seg2_iter = SEG_NEXT (seg2_iter);
            }
            seg1_iter = SEG_NEXT (seg1_iter);
        }
    } else if (PARTITION_SEGS_CNT (part) == 3) { /* For 5D N_with */
        seg1 = PARTITION_SEG (part, 0);
        seg2 = PARTITION_SEG (part, 1);
        seg3 = PARTITION_SEG (part, 2);

        DBUG_ASSERT ((seg1 != NULL && seg2 != NULL && seg3 != NULL),
                     "Found partition with NULL segment!");

        seg1_iter = seg1;
        while (seg1_iter != NULL) {
            seg2_iter = seg2;
            while (seg2_iter != NULL) {
                seg3_iter = seg3;
                while (seg3_iter != NULL) {
                    lb = DUPdoDupNode (lb_array);
                    ub = DUPdoDupNode (ub_array);
                    if (step_array != NULL) {
                        step = DUPdoDupNode (step_array);
                        width = DUPdoDupNode (width_array);
                    }

                    SET_DIM (lb, ub, 3, seg1_iter)
                    SET_DIM (lb, ub, 4, seg2_iter)
                    SET_DIM (lb, ub, 5, seg3_iter)

                    CreateWithloopPartitionsHelper (lb, ub, step, width, arg_info);

                    seg3_iter = SEG_NEXT (seg3_iter);
                }
                seg2_iter = SEG_NEXT (seg2_iter);
            }
            seg1_iter = SEG_NEXT (seg1_iter);
        }
    } else {
        DBUG_UNREACHABLE ("Wrong number of segments!");
    }

    DBUG_RETURN ();
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

    DBUG_ENTER ();

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
    DBUG_ENTER ();

    if (WITH_CUDARIZABLE (arg_node)) {
        INFO_WL_DIM (arg_info) = TCcountIds (WITH_IDS (arg_node));
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
    DBUG_ENTER ();

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    if (INFO_WL_DIM (arg_info) >= 3) {
        INFO_PART (arg_info) = arg_node;
        INFO_NEW_PARTS (arg_info) = NULL;
        PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);
        INFO_PART (arg_info) = NULL;

        if (INFO_NEW_PARTS (arg_info) != NULL) {
            arg_node = FREEdoFreeNode (arg_node);
            arg_node = INFO_NEW_PARTS (arg_info);
            INFO_NEW_PARTS (arg_info) = NULL;
        }
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
    partition_t *part;

    DBUG_ENTER ();

    DBUG_ASSERT (INFO_WL_DIM (arg_info) <= 5,
                 "N_with with dimension larger than 5 found!");

    lb = GENERATOR_BOUND1 (arg_node);
    ub = GENERATOR_BOUND2 (arg_node);
    step = GENERATOR_STEP (arg_node);
    width = GENERATOR_WIDTH (arg_node);

    /* After call to the checkGenerator, lb_array, ub_array, step_array
     * and width_array are all assumed to be constant arrays (Since)
     * the N_with is a AKS N_with */
    lb_array = CheckAndGetBound (lb);
    ub_array = CheckAndGetBound (ub);

    /* We only have to manipulate the lower bound and the upper bound */
    if (step == NULL) {
        DBUG_ASSERT (width == NULL, "step is NULL while width is not NULL!");

        part = CreatePartitionsAndSegs (lb_array, ub_array, NULL, NULL,
                                        INFO_WL_DIM (arg_info));

        if (PartitionNeedsSplit (part)) {
            CreateWithloopPartitions (lb_array, ub_array, NULL, NULL, part, arg_info);
        }
        part = FreePartition (part);
    } else {
        DBUG_ASSERT (width != NULL, "Found step but width is NULL!s");
        /* Ooops, there's step and width, more
         * sophisticated analysis is required */
        step_array = CheckAndGetBound (step);
        width_array = CheckAndGetBound (width);

        part = CreatePartitionsAndSegs (lb_array, ub_array, step_array, width_array,
                                        INFO_WL_DIM (arg_info));

        if (PartitionNeedsSplit (part)) {
            CreateWithloopPartitions (lb_array, ub_array, step_array, width_array, part,
                                      arg_info);
        }
        part = FreePartition (part);
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
