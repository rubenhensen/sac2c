/*
 *
 * $Log$
 * Revision 1.10  1998/05/12 22:42:54  dkr
 * added attributes NWITH2_DIM, NWITH2_IDX_MIN, NWITH2_IDX_MAX
 * added ComputeIndexMinMax()
 *
 * Revision 1.9  1998/05/12 15:00:48  dkr
 * renamed ???_RC_IDS to ???_DEC_RC_IDS
 *
 * Revision 1.8  1998/05/08 15:45:29  dkr
 * fixed a bug in cube-generation:
 *   pathologic grids are eleminated now :)
 *
 * Revision 1.7  1998/05/08 00:46:09  dkr
 * added some attributes to N_Nwith/N_Nwith2
 *
 * Revision 1.6  1998/05/07 10:14:36  dkr
 * inference of NWITH2_IN/INOUT/OUT/LOCAL moved to refcount
 *
 * Revision 1.5  1998/05/06 14:38:16  dkr
 * inference of NWITH_IN/INOUT/OUT/LOCAL moved to refcount
 *
 * Revision 1.4  1998/05/02 17:47:00  dkr
 * added new attributes to N_Nwith2
 *
 * Revision 1.3  1998/04/29 20:09:25  dkr
 * added a comment
 *
 * Revision 1.2  1998/04/29 17:26:36  dkr
 * added a comment
 *
 * Revision 1.1  1998/04/29 17:17:15  dkr
 * Initial revision
 *
 *
 */

#include "tree.h"
#include "free.h"

#include "internal_lib.h"
#include "traverse.h"
#include "DataFlowMask.h"
#include "optimize.h"
#include "wlpragma_funs.h"

#include "DupTree.h"
#include "dbug.h"

#include <limits.h> /* INT_MAX */

/*
 * these macros are used in 'Parts2Strides' to manage
 *   non-constant generator params
 */

#define TO_FIRST_COMPONENT(node)                                                         \
    if (NODE_TYPE (node) == N_array) {                                                   \
        node = ARRAY_AELEMS (node);                                                      \
    }

#define GET_CURRENT_COMPONENT(node, comp)                                                \
    if (node != NULL) {                                                                  \
        switch (NODE_TYPE (node)) {                                                      \
        case N_id:                                                                       \
            /* here is no break missing!! */                                             \
        case N_num:                                                                      \
            comp = DupTree (node, NULL);                                                 \
            break;                                                                       \
        case N_exprs:                                                                    \
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (node)) == N_num),                       \
                         "wrong node type found");                                       \
            comp = MakeNum (NUM_VAL (EXPRS_EXPR (node)));                                \
            node = EXPRS_EXPR (node);                                                    \
            break;                                                                       \
        default:                                                                         \
            DBUG_ASSERT ((0), "wrong node type found");                                  \
        }                                                                                \
    } else {                                                                             \
        comp = MakeNum (1);                                                              \
    }

/*
 * these macros are used in 'CompareWlnode' for compare purpose
 */

#define COMP_BEGIN(a, b, result, inc)                                                    \
    if (a > b) {                                                                         \
        result = inc;                                                                    \
    } else {                                                                             \
        if (a < b) {                                                                     \
            result = -inc;                                                               \
        } else {

#define COMP_END                                                                         \
    }                                                                                    \
    }

/******************************************************************************
 *
 * function:
 *   void ComputeIndexMinMax( int *idx_min, int *idx_max,
 *                            int dims, node *strides)
 *
 * description:
 *   Computes the minimum and maximum of the index-vector found in 'strides'.
 *
 ******************************************************************************/

void
ComputeIndexMinMax (int *idx_min, int *idx_max, int dims, node *strides)
{
    int d;

    DBUG_ENTER (" ComputeIndexMinMax");

    for (d = 0; d < dims; d++) {

        DBUG_ASSERT (((strides != NULL) && (NODE_TYPE (strides) == N_WLstride)),
                     "no stride found");

        idx_min[d] = WLSTRIDE_BOUND1 (strides);

        while (WLSTRIDE_NEXT (strides) != NULL) {
            strides = WLSTRIDE_NEXT (strides);
        }
        idx_max[d] = WLSTRIDE_BOUND2 (strides);

        strides = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (strides));
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   int CompareWLnode( node *node1, node *node2, int outline)
 *
 * description:
 *   compares the N_WL...-nodes 'node1' and 'node2' IN ALL DIMS.
 *   possibly present next nodes in 'node1' or 'node2' are ignored.
 *
 *   if (outline > 0) ALL GRID DATA IS IGNORED!!!
 *   (this feature is used by 'ComputeCubes', to determine whether two strides
 *    lie in the same cube or not)
 *
 *   this function definies the sort order for InsertWLnodes.
 *
 *   return: -2 => outline('node1') < outline('node2')
 *           -1 => outline('node1') = outline('node2'), 'node1' < 'node2'
 *            0 => 'node1' = 'node2'
 *            1 => outline('node1') = outline('node2'), 'node1' > 'node2'
 *            2 => outline('node1') > outline('node2')
 *
 ******************************************************************************/

int
CompareWLnode (node *node1, node *node2, int outline)
{
    node *grid1, *grid2;
    int result, grid_result;

    DBUG_ENTER ("CompareWLnode");

    if ((node1 != NULL) && (node2 != NULL)) {

        DBUG_ASSERT ((NODE_TYPE (node1) == NODE_TYPE (node2)),
                     "can not compare objects of different type");

        /* compare the bounds first */
        COMP_BEGIN (WLNODE_BOUND1 (node1), WLNODE_BOUND1 (node2), result, 2)
        COMP_BEGIN (WLNODE_BOUND2 (node1), WLNODE_BOUND2 (node2), result, 2)

        switch (NODE_TYPE (node1)) {

        case N_WLblock:
            /* here is no break missing! */
        case N_WLublock:

            /* compare next dim */
            result
              = CompareWLnode (WLNODE_NEXTDIM (node1), WLNODE_NEXTDIM (node2), outline);
            break;

        case N_WLstride:

            grid1 = WLSTRIDE_CONTENTS (node1);
            DBUG_ASSERT ((grid1 != NULL), "no grid found");
            grid2 = WLSTRIDE_CONTENTS (node2);
            DBUG_ASSERT ((grid2 != NULL), "no grid found");

            if (outline) {
                /* compare outlines only -> skip grid */
                result = CompareWLnode (WLGRID_NEXTDIM (grid1), WLGRID_NEXTDIM (grid2),
                                        outline);
            } else {
                /*
                 * compare grid, but leave 'result' untouched
                 *   until later dimensions are checked!
                 */
                COMP_BEGIN (WLGRID_BOUND1 (grid1), WLGRID_BOUND1 (grid2), grid_result, 1)
                COMP_BEGIN (WLGRID_BOUND2 (grid1), WLGRID_BOUND2 (grid2), grid_result, 1)
                grid_result = 0;
                COMP_END
                COMP_END

                /* compare later dimensions */
                result = CompareWLnode (WLGRID_NEXTDIM (grid1), WLGRID_NEXTDIM (grid2),
                                        outline);

                /*
                 * the 'grid_result' value is important
                 *   only if the outlines are equal
                 */
                if (abs (result) != 2) {
                    result = grid_result;
                }
            }

            break;

        case N_WLgrid:

            result
              = CompareWLnode (WLGRID_NEXTDIM (node1), WLGRID_NEXTDIM (node2), outline);
            break;

        default:

            DBUG_ASSERT ((0), "wrong node type");
        }

        COMP_END
        COMP_END

    } else {

        if ((node1 == NULL) && (node2 == NULL)) {
            result = 0;
        } else {
            result = (node2 == NULL) ? 2 : (-2);
        }
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *InsertWLnodes( node *nodes, node *insert_nodes)
 *
 * description:
 *   inserts all elements of the chain 'insert_nodes' into the sorted chain
 *     'nodes'.
 *   all elements of 'insert_nodes' that exist already in 'nodes' are freed.
 *   uses function 'CompareWLnode' to sort the elements.
 *
 *   insert_nodes: (unsorted) chain of N_WL...-nodes
 *   nodes:        sorted chain of N_WL...-nodes
 *   return:       sorted chain of N_WL...-nodes containing all the data of
 *                   'nodes' and 'insert_nodes'
 *
 ******************************************************************************/

node *
InsertWLnodes (node *nodes, node *insert_nodes)
{
    node *tmp, *insert_here;
    int compare;

    DBUG_ENTER ("InsertWLnodes");

    /*
     * insert all elements of 'insert_nodes' in 'nodes'
     */
    while (insert_nodes != NULL) {

        /* compare the first element to insert with the first element in 'nodes' */
        compare = CompareWLnode (insert_nodes, nodes, 0);

        if ((nodes == NULL) || (compare < 0)) {
            /* insert current element of 'insert_nodes' at head of 'nodes' */
            tmp = insert_nodes;
            insert_nodes = WLNODE_NEXT (insert_nodes);
            WLNODE_NEXT (tmp) = nodes;
            nodes = tmp;
        } else {

            /* search for insert-position in 'nodes' */
            insert_here = nodes;
            while ((compare > 0) && (WLNODE_NEXT (insert_here) != NULL)) {
                compare = CompareWLnode (insert_nodes, WLNODE_NEXT (insert_here), 0);

                if (compare > 0) {
                    insert_here = WLNODE_NEXT (insert_here);
                }
            }

            if (compare == 0) {
                /* current element of 'insert_nodes' exists already -> free it */
                insert_nodes = FreeNode (insert_nodes);
            } else {
                /* insert current element of 'insert_nodes' after the found position */
                tmp = insert_nodes;
                insert_nodes = WLNODE_NEXT (insert_nodes);
                WLNODE_NEXT (tmp) = WLNODE_NEXT (insert_here);
                WLNODE_NEXT (insert_here) = tmp;
            }
        }
    }

    DBUG_RETURN (nodes);
}

/******************************************************************************
 *
 * function:
 *   node *NormalizeStride_1( node *stride)
 *
 * description:
 *   returns the IN THE FIRST DIMENSION normalized N_WLstride-node 'stride'.
 *   a possibly present next node in 'stride' is ignored.
 *
 *   this normalization has two major goals:
 *     * every stride has a unambiguous form
 *        -> two strides represent the same index set
 *           if and only if there attribute values are equal.
 *     * maximize the outline of strides
 *        -> two strides of the same cube do not split each other in several
 *           parts when intersected
 *
 ******************************************************************************/

node *
NormalizeStride_1 (node *stride)
{
    node *grid;
    int bound1, bound2, step, grid_b1, grid_b2, new_bound1, new_bound2;

    DBUG_ENTER ("NormalizeStride_1");

    grid = WLSTRIDE_CONTENTS (stride);
    DBUG_ASSERT ((grid != NULL), "grid not found");
    DBUG_ASSERT ((WLGRID_NEXT (grid) == NULL), "more than one grid found");

    bound1 = WLSTRIDE_BOUND1 (stride);
    bound2 = WLSTRIDE_BOUND2 (stride);
    step = WLSTRIDE_STEP (stride);
    grid_b1 = WLGRID_BOUND1 (grid);
    grid_b2 = WLGRID_BOUND2 (grid);

    /*
     * assure: ([grid_b1; grid_b2] < [0; step]) or (grid_b2 = step = 1);
     * in other terms: (width < step) or (width = step = 1)
     */

    if (grid_b2 > step) {
        grid_b2 = step;
    }
    if ((step > 1) && (grid_b1 == 0) && (grid_b2 == step)) {
        grid_b2 = step = 1;
    }

    /*
     * if (bound2 - bound1 <= step), we set (step = 1) to avoid pathologic cases!!!
     */

    if (bound2 - bound1 <= step) {
        bound2 = bound1 + grid_b2;
        bound1 += grid_b1;
        grid_b1 = 0;
        grid_b2 = 1;
        step = 1;
    }

    /*
     * maximize the outline
     */

    /* calculate minimum for 'bound1' */
    new_bound1 = bound1 - (step - grid_b2);
    new_bound1 = MAX (0, new_bound1);

    /* calculate maximum for 'bound2' */
    new_bound2 = ((bound2 - bound1 - grid_b1) % step >= grid_b2 - grid_b1)
                   ? (bound2 + step - ((bound2 - bound1 - grid_b1) % step))
                   : (bound2);

    WLSTRIDE_BOUND1 (stride) = new_bound1;
    WLSTRIDE_BOUND2 (stride) = new_bound2;
    WLSTRIDE_STEP (stride) = step;
    WLGRID_BOUND1 (grid) = grid_b1 + (bound1 - new_bound1);
    WLGRID_BOUND2 (grid) = grid_b2 + (bound1 - new_bound1);

    DBUG_RETURN (stride);
}

/******************************************************************************
 *
 * function:
 *   node* Parts2Strides( node *parts, int dims)
 *
 * description:
 *   converts a N_Npart-chain ('parts') into a N_WLstride-chain (return).
 *   'dims' is the number of dimensions.
 *
 ******************************************************************************/

node *
Parts2Strides (node *parts, int dims)
{
    node *parts_stride, *stride, *new_stride, *last_grid, *gen, *bound1, *bound2, *step,
      *width, *curr_bound1, *curr_bound2, *curr_step, *curr_width;
    int dim, curr_step_, curr_width_;

    DBUG_ENTER ("Parts2Strides");

    parts_stride = NULL;

    gen = NPART_GEN (parts);
    bound1 = NGEN_BOUND1 (gen);
    bound2 = NGEN_BOUND2 (gen);
    step = NGEN_STEP (gen);
    width = NGEN_WIDTH (gen);

    /*
     * check, if params of generator are constant
     */
    if ((NODE_TYPE (bound1) == N_array) && (NODE_TYPE (bound2) == N_array)
        && ((step == NULL) || (NODE_TYPE (step) == N_array))
        && ((width == NULL) || (NODE_TYPE (width) == N_array))) {

        /*
         * the generator parameters are constant
         *  -> the params of *all* generators are constant (see WLF)
         */

        while (parts != NULL) {
            stride = NULL;

            gen = NPART_GEN (parts);
            DBUG_ASSERT ((NGEN_OP1 (gen) == F_le), "op1 in generator is not <=");
            DBUG_ASSERT ((NGEN_OP2 (gen) == F_lt), "op2 in generator is not <");

            /* get components of current generator */
            bound1 = ARRAY_AELEMS (NGEN_BOUND1 (gen));
            bound2 = ARRAY_AELEMS (NGEN_BOUND2 (gen));
            step = (NGEN_STEP (gen) != NULL) ? ARRAY_AELEMS (NGEN_STEP (gen)) : NULL;
            width = (NGEN_WIDTH (gen) != NULL) ? ARRAY_AELEMS (NGEN_WIDTH (gen)) : NULL;

            for (dim = 0; dim < dims; dim++) {
                DBUG_ASSERT ((bound1 != NULL), "bound1 not complete");
                DBUG_ASSERT ((bound2 != NULL), "bound2 not complete");

                curr_step_ = (step != NULL) ? NUM_VAL (EXPRS_EXPR (step)) : 1;
                curr_width_ = (width != NULL) ? NUM_VAL (EXPRS_EXPR (width)) : 1;

                /* build N_WLstride-node of current dimension */
                new_stride = MakeWLstride (0, dim, NUM_VAL (EXPRS_EXPR (bound1)),
                                           NUM_VAL (EXPRS_EXPR (bound2)), curr_step_, 0,
                                           MakeWLgrid (0, dim, 0, curr_width_, 0, NULL,
                                                       NULL, NULL),
                                           NULL);

                /* the PART-information is needed by 'IntersectOutline' */
                WLSTRIDE_PART (new_stride) = parts;

                new_stride = NormalizeStride_1 (new_stride);

                /* append 'new_stride' to 'stride' */
                if (dim == 0) {
                    stride = new_stride;
                } else {
                    WLGRID_NEXTDIM (last_grid) = new_stride;
                }
                last_grid = WLSTRIDE_CONTENTS (new_stride);

                /* go to next dim */
                bound1 = EXPRS_NEXT (bound1);
                bound2 = EXPRS_NEXT (bound2);
                if (step != NULL) {
                    step = EXPRS_NEXT (step);
                }
                if (width != NULL) {
                    width = EXPRS_NEXT (width);
                }
            }

            WLGRID_CODE (last_grid) = NPART_CODE (parts);
            NCODE_USED (NPART_CODE (parts))++;
            parts_stride = InsertWLnodes (parts_stride, stride);

            parts = NPART_NEXT (parts);
        }
        while (parts != NULL)
            ;

    } else {

        /*
         * not all generator parameters are constant
         */

        DBUG_ASSERT ((NPART_NEXT (parts) == NULL), "more than one part found");

        TO_FIRST_COMPONENT (bound1)
        TO_FIRST_COMPONENT (bound2)
        if (step != NULL) {
            TO_FIRST_COMPONENT (step)
        }
        if (width != NULL) {
            TO_FIRST_COMPONENT (width)
        }

        for (dim = 0; dim < dims; dim++) {
            /*
             * components of current dim
             */
            GET_CURRENT_COMPONENT (bound1, curr_bound1)
            GET_CURRENT_COMPONENT (bound2, curr_bound2)
            GET_CURRENT_COMPONENT (step, curr_step)
            GET_CURRENT_COMPONENT (width, curr_width)

            /* build N_WLstriVar-node of current dimension */
            new_stride = MakeWLstriVar (dim, curr_bound1, curr_bound2, curr_step,
                                        MakeWLgridVar (dim, MakeNum (0), curr_width, NULL,
                                                       NULL, NULL),
                                        NULL);

            /* append 'new_stride' to 'parts_stride' */
            if (dim == 0) {
                parts_stride = new_stride;
            } else {
                WLGRIDVAR_NEXTDIM (last_grid) = new_stride;
            }
            last_grid = WLSTRIVAR_CONTENTS (new_stride);
        }

        WLGRIDVAR_CODE (last_grid) = NPART_CODE (parts);
        NCODE_USED (NPART_CODE (parts))++;
    }

    DBUG_RETURN (parts_stride);
}

/******************************************************************************
 *
 * function:
 *   int IndexHeadStride( node *stride)
 *
 * description:
 *   returns the index position of the first element of 'stride'
 *
 ******************************************************************************/

int
IndexHeadStride (node *stride)
{
    int result;

    DBUG_ENTER ("IndexHeadStride");

    result = WLSTRIDE_BOUND1 (stride) + WLGRID_BOUND1 (WLSTRIDE_CONTENTS (stride));

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   int IndexRearStride( node *stride)
 *
 * description:
 *   returns the index position '+1' of the last element of 'stride'
 *
 ******************************************************************************/

int
IndexRearStride (node *stride)
{
    node *grid = WLSTRIDE_CONTENTS (stride);
    int bound2 = WLSTRIDE_BOUND2 (stride);
    int grid_b1 = WLGRID_BOUND1 (grid);
    int result;

    DBUG_ENTER ("IndexRearStride");

    /* search last grid (there will we find the last element!) */
    while (WLGRID_NEXT (grid) != NULL) {
        grid = WLGRID_NEXT (grid);
    }

    result = bound2
             - MAX (0, ((bound2 - WLSTRIDE_BOUND1 (stride) - grid_b1 - 1)
                        % WLSTRIDE_STEP (stride))
                         + 1 - (WLGRID_BOUND2 (grid) - grid_b1));

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   int GridOffset( int new_bound1,
 *                   int bound1, int step, int grid_b2)
 *
 * description:
 *   computes a offset for a grid relating to 'new_bound1':
 *     what happens to the bounds of a grid if 'new_bound1' is the new
 *     upper bound for the accessory stride?
 *     the new bounds of the grid are:
 *         grid_b1 - offset, grid_b2 - offset
 *
 *   CAUTION: if (offset > grid_b1) the grid must be devided in two parts:
 *              "(grid_b1 - offset + step) -> step" and
 *              "0 -> (grid_b2 - offset)" !!
 *
 ******************************************************************************/

int
GridOffset (int new_bound1, int bound1, int step, int grid_b2)
{
    int offset;

    DBUG_ENTER ("GridOffset");

    offset = (new_bound1 - bound1) % step;

    if (offset >= grid_b2) {
        offset -= step;
    }

    DBUG_RETURN (offset);
}

/******************************************************************************
 *
 * function:
 *   int IntersectOutline( node *stride1, node *stride2,
 *                         node **i_stride1, node **i_stride2)
 *
 * description:
 *   returns in 'i_stride1' and 'i_stride2' the part of 'stride1', 'stride2'
 *     respectively that lies in a common cube.
 *   possibly present next nodes in 'stride1' or 'stride2' are ignored.
 *   the return value is 1 if and only if the intersection is non-empty.
 *
 *   if we are interested in the return value only, we can call this
 *     function with ('stride1' == NULL), ('stride2' == NULL).
 *
 ******************************************************************************/

int
IntersectOutline (node *stride1, node *stride2, node **i_stride1, node **i_stride2)
{
    node *grid1, *grid2, *new_i_stride1, *new_i_stride2;
    int bound11, bound21, grid1_b1, grid1_b2, bound12, bound22, grid2_b1, grid2_b2, head1,
      rear1, head2, rear2, i_bound1, i_bound2, i_offset1, i_offset2;
    int flag = 0;
    int result = 1;

    DBUG_ENTER ("IntersectOutline");

    if (i_stride1 != NULL) {
        new_i_stride1 = *i_stride1 = DupNode (stride1);
    }
    if (i_stride2 != NULL) {
        new_i_stride2 = *i_stride2 = DupNode (stride2);
    }

    while (stride1 != NULL) {
        DBUG_ASSERT ((stride2 != NULL), "dim not found");

        DBUG_ASSERT ((WLSTRIDE_PART (stride1) != NULL), "no part found");
        DBUG_ASSERT ((WLSTRIDE_PART (stride2) != NULL), "no part found");

        grid1 = WLSTRIDE_CONTENTS (stride1);
        DBUG_ASSERT ((grid1 != NULL), "grid not found");
        DBUG_ASSERT ((WLGRID_NEXT (grid1) == NULL), "more than one grid found");
        grid2 = WLSTRIDE_CONTENTS (stride2);
        DBUG_ASSERT ((grid2 != NULL), "grid not found");
        DBUG_ASSERT ((WLGRID_NEXT (grid2) == NULL), "more than one grid found");

        bound11 = WLSTRIDE_BOUND1 (stride1);
        bound21 = WLSTRIDE_BOUND2 (stride1);
        grid1_b1 = WLGRID_BOUND1 (grid1);
        grid1_b2 = WLGRID_BOUND2 (grid1);

        bound12 = WLSTRIDE_BOUND1 (stride2);
        bound22 = WLSTRIDE_BOUND2 (stride2);
        grid2_b1 = WLGRID_BOUND1 (grid2);
        grid2_b2 = WLGRID_BOUND2 (grid2);

        head1 = IndexHeadStride (stride1);
        rear1 = IndexRearStride (stride1);
        head2 = IndexHeadStride (stride2);
        rear2 = IndexRearStride (stride2);

        i_bound1 = MAX (bound11, bound12);
        i_bound2 = MIN (bound21, bound22);

        i_offset1 = GridOffset (i_bound1, bound11, WLSTRIDE_STEP (stride1), grid1_b2);
        i_offset2 = GridOffset (i_bound1, bound12, WLSTRIDE_STEP (stride2), grid2_b2);

        if (/* are the outlines of 'stride1' and 'stride2' not disjunkt? */
            (head1 < rear2) && (head2 < rear1) &&

            /* are the grids compatible? */
            (i_offset1 <= grid1_b1) && (i_offset2 <= grid2_b1) &&
            /*
             * Note: (i_offset_1 < grid1_b1) means, that the grid1 must be split
             *        in two parts to fit the new upper bound in the current dim.
             *       Then the *projections* of grid1, grid2 can not be disjunct,
             *        therefore grid1, grid2 must have disjunct outlines!!!
             */

            /* is intersection of 'stride1' with the outline of 'stride2' not empty? */
            (i_bound1 + grid1_b1 - i_offset1 < i_bound2) &&
            /* is intersection of 'stride2' with the outline of 'stride1' not empty? */
            (i_bound1 + grid2_b1 - i_offset2 < i_bound2)
            /*
             * example:
             *
             *   stride1: 0->5 step 2          stride2: 2->3 step 1
             *                   1->2: ...                     0->1: ...
             *
             * Here we must notice, that the intersection of 'stride1' with the
             *  outline of 'stride2' is empty:
             *
             *            2->3 step 2
             *                   1->2: ...     !!!!!!!!!!!!
             */

        ) {

            if ((WLSTRIDE_PART (stride1) == WLSTRIDE_PART (stride2)) &&
                /* are 'stride1' and 'stride2' descended from the same Npart? */
                (!flag)) {
                /* we should deal with this exception only once !! */

                /*
                 * example:
                 *
                 *  0->6  step 3, 0->1: op1
                 *  0->16 step 3, 1->3: op2
                 *  4->20 step 3, 2->3: op3
                 * ------------------------- after first round with IntersectOutline:
                 *  0->7  step 3, 1->3: op2  <- intersection of 'op2' and outline('op1')
                 *  3->16 step 3, 1->3: op2  <- intersection of 'op2' and outline('op3')
                 *
                 *  these two strides are **not** disjunkt!!!
                 *  but they are part of the same Npart!!
                 */

                flag = 1; /* skip this exception handling in later dimensions! */

                /*
                 * modify the bounds of the first stride,
                 * so that the new outlines are disjunct
                 */
                if (WLSTRIDE_BOUND2 (stride1) < WLSTRIDE_BOUND2 (stride2)) {
                    if (i_stride1 != NULL) {
                        WLSTRIDE_BOUND2 (new_i_stride1) = i_bound1;
                        new_i_stride1 = NormalizeStride_1 (new_i_stride1);
                    }
                } else {
                    if (i_stride2 != NULL) {
                        WLSTRIDE_BOUND2 (new_i_stride2) = i_bound1;
                        new_i_stride2 = NormalizeStride_1 (new_i_stride2);
                    }
                }

            } else {

                if (i_stride1 != NULL) {
                    /* intersect 'stride1' with the outline of 'stride2' */
                    WLSTRIDE_BOUND1 (new_i_stride1) = i_bound1;
                    WLSTRIDE_BOUND2 (new_i_stride1) = i_bound2;
                    WLGRID_BOUND1 (WLSTRIDE_CONTENTS (new_i_stride1))
                      = grid1_b1 - i_offset1;
                    WLGRID_BOUND2 (WLSTRIDE_CONTENTS (new_i_stride1))
                      = grid1_b2 - i_offset1;
                    new_i_stride1 = NormalizeStride_1 (new_i_stride1);
                }

                if (i_stride2 != NULL) {
                    /* intersect 'stride2' with the outline of 'stride1' */
                    WLSTRIDE_BOUND1 (new_i_stride2) = i_bound1;
                    WLSTRIDE_BOUND2 (new_i_stride2) = i_bound2;
                    WLGRID_BOUND1 (WLSTRIDE_CONTENTS (new_i_stride2))
                      = grid2_b1 - i_offset2;
                    WLGRID_BOUND2 (WLSTRIDE_CONTENTS (new_i_stride2))
                      = grid2_b2 - i_offset2;
                    new_i_stride2 = NormalizeStride_1 (new_i_stride2);
                }
            }

        } else {
            /*
             * intersection is empty
             *  -> free the useless data in 'i_stride1', 'i_stride2'
             */
            if (i_stride1 != NULL) {
                if (*i_stride1 != NULL) {
                    *i_stride1 = FreeTree (*i_stride1);
                }
            }
            if (i_stride2 != NULL) {
                if (*i_stride2 != NULL) {
                    *i_stride2 = FreeTree (*i_stride2);
                }
            }
            result = 0;

            /* we can give up here */
            break;
        }

        /* next dim */
        stride1 = WLGRID_NEXTDIM (grid1);
        stride2 = WLGRID_NEXTDIM (grid2);
        if (i_stride1 != NULL) {
            new_i_stride1 = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (new_i_stride1));
        }
        if (i_stride2 != NULL) {
            new_i_stride2 = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (new_i_stride2));
        }
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *SetSegs( node *pragma, node *cubes, int dims)
 *
 * description:
 *   returns chain of segments (based on the calculated cubes 'cubes')
 *
 ******************************************************************************/

node *
SetSegs (node *pragma, node *cubes, int dims)
{
    node *aps;
    node *segs = NULL;

    DBUG_ENTER ("SetSegs");

    /*
     * create default configuration
     */
#if 1 /* -> sac2c flag!?! */
    segs = All (segs, NULL, cubes, dims);
#else
    segs = Cubes (segs, NULL, cubes, dims);
#endif

    /*
     * create pragma-dependent configuration
     */
    if (pragma != NULL) {
        aps = PRAGMA_WLCOMP_APS (pragma);
        while (aps != NULL) {

#define WLP(fun, str)                                                                    \
    if (strcmp (AP_NAME (EXPRS_EXPR (aps)), str) == 0) {                                 \
        segs = fun (segs, AP_ARGS (EXPRS_EXPR (aps)), cubes, dims);                      \
    } else

#include "wlpragma_funs.mac"
            DBUG_ASSERT ((0), "wrong function name in wlcomp-pragma");

#undef WLP

            aps = EXPRS_NEXT (aps);
        }
    }

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * function:
 *   node *NewBoundsStride( node *stride, int dim,
 *                          int new_bound1, int new_bound2)
 *
 * description:
 *   returns modified 'stride':
 *     all strides in dimension "current dimension"+'dim' are new bounds
 *     given ('bound1', 'bound2').
 *
 ******************************************************************************/

node *
NewBoundsStride (node *stride, int dim, int new_bound1, int new_bound2)
{
    node *grids, *new_grids, *tmp, *tmp2;
    int bound1, step, grid_b1, grid_b2, offset;

    DBUG_ENTER ("NewBoundsStride");

    grids = WLSTRIDE_CONTENTS (stride);

    if (dim == 0) {
        /*
         * arrived at the correct dimension
         *  -> set new bounds
         *  -> correct the grids if necessary
         */

        bound1 = WLSTRIDE_BOUND1 (stride);
        if (bound1 != new_bound1) {
            /*
             * correct the grids
             */

            step = WLSTRIDE_STEP (stride);
            new_grids = NULL;
            do {
                grid_b1 = WLGRID_BOUND1 (grids);
                grid_b2 = WLGRID_BOUND2 (grids);

                offset = GridOffset (new_bound1, bound1, step, grid_b2);

                /* extract current grid from chain -> single grid in 'tmp' */
                tmp = grids;
                grids = WLGRID_NEXT (grids);
                WLGRID_NEXT (tmp) = NULL;

                if (offset <= grid_b1) {
                    /*
                     * grid is still in one pice :)
                     */

                    WLGRID_BOUND1 (tmp) = grid_b1 - offset;
                    WLGRID_BOUND2 (tmp) = grid_b2 - offset;

                    /* insert changed grid into 'new_grids' */
                    new_grids = InsertWLnodes (new_grids, tmp);
                } else {
                    /*
                     * the grid is split into two parts :(
                     */

                    /* first part: recycle old grid */
                    WLGRID_BOUND1 (tmp) = grid_b1 - offset + step;
                    WLGRID_BOUND2 (tmp) = step;
                    /* second part: duplicate old grid first */
                    tmp2 = DupNode (tmp);
                    WLGRID_BOUND1 (tmp2) = 0;
                    WLGRID_BOUND2 (tmp2) = grid_b2 - offset;
                    /* concate the two grids */
                    WLGRID_NEXT (tmp2) = tmp;

                    /* insert them into 'new_grids' */
                    new_grids = InsertWLnodes (new_grids, tmp2);
                }
            } while (grids != NULL);

            WLSTRIDE_CONTENTS (stride) = new_grids;
            WLSTRIDE_BOUND1 (stride) = new_bound1;
        }

        WLSTRIDE_BOUND2 (stride) = new_bound2;

    } else {
        /*
         * involve all grids of current dimension
         */

        do {
            WLGRID_NEXTDIM (grids)
              = NewBoundsStride (WLGRID_NEXTDIM (grids), dim - 1, new_bound1, new_bound2);
            grids = WLGRID_NEXT (grids);
        } while (grids != NULL);
    }

    DBUG_RETURN (stride);
}

/******************************************************************************
 *
 * function:
 *   void SplitStride( node *stride1, node *stride2
 *                     node **s_stride1, node **s_stride2)
 *
 * description:
 *   returns in 's_stride1', 's_stride2' the splitted stride 'stride1',
 *     'stride2' respectively.
 *   returns NULL if there is nothing to split.
 *
 ******************************************************************************/

void
SplitStride (node *stride1, node *stride2, node **s_stride1, node **s_stride2)
{
    node *tmp1, *tmp2;
    int i_bound1, i_bound2, dim;

    DBUG_ENTER ("SplitStride");

    tmp1 = stride1;
    tmp2 = stride2;

    *s_stride1 = *s_stride2 = NULL;

    /*
     * in which dimension is splitting needed?
     *
     * search for the first dim,
     * in which the bounds of 'stride1' and 'stride2' are not equal
     */
    dim = 0;
    while ((tmp1 != NULL) && (tmp2 != NULL)
           && (WLSTRIDE_BOUND1 (tmp1) == WLSTRIDE_BOUND1 (tmp2))
           && (WLSTRIDE_BOUND2 (tmp1) == WLSTRIDE_BOUND2 (tmp2))) {
        /*
         * we can take the first grid only,
         * because the stride-bounds are equal in all grids!!
         */
        tmp1 = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (tmp1));
        tmp2 = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (tmp2));
        dim++;
    }

    if ((tmp1 != NULL) && (tmp2 != NULL)) { /* is there anything to split? */
        /* compute bounds of intersection */
        i_bound1 = MAX (WLSTRIDE_BOUND1 (tmp1), WLSTRIDE_BOUND1 (tmp2));
        i_bound2 = MIN (WLSTRIDE_BOUND2 (tmp1), WLSTRIDE_BOUND2 (tmp2));

        if (i_bound1 < i_bound2) { /* is intersection non-empty? */
            *s_stride1 = DupNode (stride1);
            *s_stride2 = DupNode (stride2);

            /*
             * propagate the new bounds in dimension 'dim'
             * over the whole tree of 'stride1', 'stride2' respectively
             */
            *s_stride1 = NewBoundsStride (*s_stride1, dim, i_bound1, i_bound2);
            *s_stride2 = NewBoundsStride (*s_stride2, dim, i_bound1, i_bound2);
        }
    }

    DBUG_VOID_RETURN
}

/******************************************************************************
 *
 * function:
 *   node *SplitWL( node *strides)
 *
 * description:
 *   returns the splitted stride-tree 'strides'.
 *
 ******************************************************************************/

node *
SplitWL (node *strides)
{
    node *stride1, *stride2, *split_stride1, *split_stride2, *new_strides, *tmp;
    int fixpoint;

    DBUG_ENTER ("SplitWL");

    DBUG_ASSERT ((NODE_TYPE (strides) == N_WLstride), "wrong node type found");

    /*
     * the outline of each stride is intersected with all the other ones.
     * this is done until no new intersections are generated (fixpoint).
     */
    do {
        fixpoint = 1;       /* initialize 'fixpoint' */
        new_strides = NULL; /* here we collect the new stride-set */

        /* check WLSTRIDE_MODIFIED */
        stride1 = strides;
        while (stride1 != NULL) {
            DBUG_ASSERT ((WLSTRIDE_MODIFIED (stride1) == 0), "stride was modified");
            stride1 = WLSTRIDE_NEXT (stride1);
        }

        /*
         * split in pairs
         */
        stride1 = strides;
        while (stride1 != NULL) {

            stride2 = WLSTRIDE_NEXT (stride1);
            while (stride2 != NULL) {

                SplitStride (stride1, stride2, &split_stride1, &split_stride2);
                if (split_stride1 != NULL) {
                    DBUG_ASSERT ((split_stride2 != NULL), "wrong splitting");
                    fixpoint = 0; /* no, not a fixpoint yet :( */
                    WLSTRIDE_MODIFIED (stride1) = WLSTRIDE_MODIFIED (stride2) = 1;
                    new_strides = InsertWLnodes (new_strides, split_stride1);
                    new_strides = InsertWLnodes (new_strides, split_stride2);
                } else {
                    DBUG_ASSERT ((split_stride2 == NULL), "wrong splitting");
                }

                stride2 = WLSTRIDE_NEXT (stride2);
            }

            /*
             * was 'stride1' not modified?
             *  -> it is a part of the result
             */
            if (WLSTRIDE_MODIFIED (stride1) == 0) {
                /* insert 'stride1' in 'new_strides' */
                tmp = stride1;
                stride1 = WLSTRIDE_NEXT (stride1);
                WLSTRIDE_NEXT (tmp) = NULL;
                new_strides = InsertWLnodes (new_strides, tmp);
            } else {
                /*
                 * 'stride1' was modified, hence no part of the result.
                 *  -> is no longer needed
                 */
                stride1 = FreeNode (stride1);
                /* 'stride1' points to his successor now! */
            }
        }

        strides = new_strides;
    } while (!fixpoint); /* fixpoint found? */

    DBUG_RETURN (strides);
}

/******************************************************************************
 *
 * function:
 *   node *BlockStride( node *stride, long *bv)
 *
 * description:
 *   returns 'stride' with corrected bounds, blocking levels and
 *     unrolling-flag.
 *   this function is needed after a blocking.
 *
 ******************************************************************************/

node *
BlockStride (node *stride, long *bv)
{
    node *curr_stride, *curr_grid, *grids;

    DBUG_ENTER ("BlockStride");

    if (stride != NULL) {

        DBUG_ASSERT ((NODE_TYPE (stride)), "no N_WLstride node found");

        curr_stride = stride;
        do {

            /* correct blocking level and unrolling-flag */
            WLSTRIDE_LEVEL (curr_stride)++;
            WLSTRIDE_UNROLLING (curr_stride) = 1; /* we want to unroll this stride */
            grids = WLSTRIDE_CONTENTS (curr_stride);
            do {
                WLGRID_LEVEL (grids)++;
                WLGRID_UNROLLING (grids) = 1; /* we want to unroll this grid */
                grids = WLGRID_NEXT (grids);
            } while (grids != NULL);

            /* fit bounds of stride to blocking step */
            DBUG_ASSERT ((bv[WLSTRIDE_DIM (curr_stride)] > 1), "wrong bv value found");
            DBUG_ASSERT ((bv[WLSTRIDE_DIM (curr_stride)] >= WLSTRIDE_STEP (curr_stride)),
                         "blocking step (>1) is smaller than stride step");
            WLSTRIDE_BOUND1 (curr_stride) = 0;
            WLSTRIDE_BOUND2 (curr_stride) = bv[WLSTRIDE_DIM (curr_stride)];

            /*
             * involve all grids of current dimension
             */
            curr_grid = WLSTRIDE_CONTENTS (curr_stride);
            do {
                WLGRID_NEXTDIM (curr_grid) = BlockStride (WLGRID_NEXTDIM (curr_grid), bv);
                curr_grid = WLGRID_NEXT (curr_grid);
            } while (curr_grid != NULL);

            curr_stride = WLSTRIDE_NEXT (curr_stride);
        } while (curr_stride != NULL);
    }

    DBUG_RETURN (stride);
}

/******************************************************************************
 *
 * function:
 *   node *BlockWL( node *stride, int dims, long *bv, int unroll)
 *
 * description:
 *   returns with blocking-vector 'bv' blocked 'stride'.
 *   'dims' is the number of dimensions in 'stride'.
 *
 *   when called multiple times in a row, this function even realizes
 *     hierarchical blocking!! (top-down: coarse blocking first!)
 *
 *   ('unroll' > 0) means unrolling-blocking --- allowed only once after all
 *     convential blocking!
 *
 ******************************************************************************/

node *
BlockWL (node *stride, int dims, long *bv, int unroll)
{
    node *curr_block, *curr_dim, *curr_stride, *curr_grid, *contents, *lastdim,
      *last_block, *block;
    int level, d;

    DBUG_ENTER ("BlockWL");

    if (stride != NULL) {

        switch (NODE_TYPE (stride)) {

        case N_WLblock:
            /*
             * block found -> hierarchical blocking
             */

            curr_block = stride;
            while (curr_block != NULL) {

                /* go to contents of block -> skip all found block nodes */
                curr_dim = curr_block;
                DBUG_ASSERT ((NODE_TYPE (curr_dim) == N_WLblock), "no block found");
                while (WLBLOCK_NEXTDIM (curr_dim) != NULL) {
                    curr_dim = WLBLOCK_NEXTDIM (curr_dim);
                    DBUG_ASSERT ((NODE_TYPE (curr_dim) == N_WLblock), "no block found");
                }

                /* block contents of found block */
                WLBLOCK_CONTENTS (curr_dim)
                  = BlockWL (WLBLOCK_CONTENTS (curr_dim), dims, bv, unroll);

                curr_block = WLBLOCK_NEXT (curr_block);
            }
            break;

        case N_WLublock:
            /*
             * ublock found ?!?! -> error
             */

            /*
             * unrolling-blocking is allowed only once
             * after all conventional blocking!!
             */
            DBUG_ASSERT ((0), "data of unrolling-blocking found while blocking");
            break;

        case N_WLstride:
            /*
             * unblocked stride found
             */

            level = WLSTRIDE_LEVEL (stride);

            last_block = NULL;
            curr_stride = stride;
            while (curr_stride != NULL) {

                DBUG_ASSERT ((bv[WLSTRIDE_DIM (curr_stride)] >= 1),
                             "wrong bv value found");

                if (bv[WLSTRIDE_DIM (curr_stride)] == 1) {
                    /*
                     * no blocking -> go to next dim
                     */
                    curr_grid = WLSTRIDE_CONTENTS (curr_stride);
                    do {
                        WLGRID_NEXTDIM (curr_grid)
                          = BlockWL (WLGRID_NEXTDIM (curr_grid), dims, bv, unroll);

                        curr_grid = WLGRID_NEXT (curr_grid);
                    } while (curr_grid != NULL);

                    curr_stride = WLSTRIDE_NEXT (curr_stride);
                } else {
                    /*
                     * blocking -> create a N_WLblock (N_WLublock respectively) node
                     *   for each following dim
                     */
                    contents = curr_stride;
                    lastdim = NULL;
                    for (d = WLSTRIDE_DIM (curr_stride); d < dims; d++) {
                        DBUG_ASSERT ((NODE_TYPE (contents) == N_WLstride),
                                     "wrong hierarchical blocking");

                        block
                          = MakeWLblock (level, WLSTRIDE_DIM (contents),
                                         WLSTRIDE_BOUND1 (contents),
                                         WLSTRIDE_BOUND2 (contents),
                                         bv[WLSTRIDE_DIM (contents)], NULL, NULL, NULL);

                        if (unroll > 0) { /* unrolling-blocking wanted? */
                            NODE_TYPE (block) = N_WLublock;
                        }

                        if (lastdim != NULL) {
                            /*
                             * not first blocking dim
                             *  -> append at block node of last dim
                             */
                            WLBLOCK_NEXTDIM (lastdim) = block;
                        } else {
                            /* current dim is first blocking dim */
                            if (last_block != NULL) {
                                /* append to last block */
                                WLBLOCK_NEXT (last_block) = block;
                            } else {
                                /* this is the first block */
                                stride = block;
                            }
                            last_block = block;
                        }
                        lastdim = block;

                        DBUG_ASSERT ((WLSTRIDE_CONTENTS (contents) != NULL),
                                     "no grid found");
                        contents = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (contents));
                    }

                    /*
                     * now the block nodes are complete
                     *  -> append contents of block
                     */
                    DBUG_ASSERT ((lastdim != NULL), "block node of last dim not found");
                    WLBLOCK_CONTENTS (lastdim) = curr_stride;
                    curr_stride = WLSTRIDE_NEXT (curr_stride);
                    /* successor is in next block -> no 'next' anymore! */
                    WLSTRIDE_NEXT (WLBLOCK_CONTENTS (lastdim)) = NULL;
                    /* correct the bounds and blocking level in contents of block */
                    WLBLOCK_CONTENTS (lastdim)
                      = BlockStride (WLBLOCK_CONTENTS (lastdim), bv);
                }
            }
            break;

        default:

            DBUG_ASSERT ((0), "wrong node type");
        }
    }

    DBUG_RETURN (stride);
}

/******************************************************************************
 *
 * function:
 *   node *NewStepGrids( node *grids, int step, int new_step, int offset)
 *
 * description:
 *   returns a modified 'grids' chain:
 *     - the bounds of the grids are modified (relating to 'offset')
 *     - the step of the grids is now 'step'
 *        -> possibly the grids must be duplicated
 *
 ******************************************************************************/

node *
NewStepGrids (node *grids, int step, int new_step, int offset)
{
    node *last_old, *last, *new_grid, *tmp;
    int i, div;

    DBUG_ENTER ("NewStepGrids");

    DBUG_ASSERT ((new_step % step == 0), "wrong new step");

    if (step == 1) {
        DBUG_ASSERT ((WLGRID_BOUND1 (grids) == 0), "grid has wrong lower bound");
        DBUG_ASSERT ((WLGRID_NEXT (grids) == NULL), "grid has wrong bounds");
        WLGRID_BOUND2 (grids) = new_step;
    } else {
        div = new_step / step;

        /*
         * adjust bounds (relating to 'offset')
         *
         * search for last grid -> save it in 'last_old'
         */
        WLGRID_BOUND1 (grids) -= offset;
        WLGRID_BOUND2 (grids) -= offset;
        last_old = grids;
        while (WLGRID_NEXT (last_old) != NULL) {
            last_old = WLGRID_NEXT (last_old);
            WLGRID_BOUND1 (last_old) -= offset;
            WLGRID_BOUND2 (last_old) -= offset;
        }

        if (div > 1) {
            /*
             * duplicate all grids ('div' -1) times
             */
            last = last_old;
            for (i = 1; i < div; i++) {
                tmp = grids;
                do {
                    /* duplicate current grid */
                    new_grid = DupNode (tmp);
                    WLGRID_BOUND1 (new_grid) = WLGRID_BOUND1 (new_grid) + i * step;
                    WLGRID_BOUND2 (new_grid) = WLGRID_BOUND2 (new_grid) + i * step;

                    last = WLGRID_NEXT (last) = new_grid;
                } while (tmp != last_old);
            }
        }
    }

    DBUG_RETURN (grids);
}

/******************************************************************************
 *
 * function:
 *   node *IntersectGrid( node *grid1, node *grid2, int step,
 *                        node **i_grid1, node **i_grid2)
 *
 * description:
 *   returns in 'i_grid1', 'i_grid2' the intersection of 'grid1' and 'grid2'.
 *   both grids must have the same step ('step').
 *
 *   returns NULL if the intersection is equal to the original grid!!
 *
 ******************************************************************************/

void
IntersectGrid (node *grid1, node *grid2, int step, node **i_grid1, node **i_grid2)
{
    int bound11, bound21, bound12, bound22, i_bound1, i_bound2;

    DBUG_ENTER ("IntersectGrid");

    *i_grid1 = *i_grid2 = NULL;

    bound11 = WLGRID_BOUND1 (grid1);
    bound21 = WLGRID_BOUND2 (grid1);

    bound12 = WLGRID_BOUND1 (grid2);
    bound22 = WLGRID_BOUND2 (grid2);

    /* compute bounds of intersection */
    i_bound1 = MAX (bound11, bound12);
    i_bound2 = MIN (bound21, bound22);

    if (i_bound1 < i_bound2) { /* is intersection non-empty? */

        if ((i_bound1 != bound11) || (i_bound2 != bound21)) {
            *i_grid1 = DupNode (grid1);
            WLGRID_BOUND1 ((*i_grid1)) = i_bound1;
            WLGRID_BOUND2 ((*i_grid1)) = i_bound2;
        }

        if ((i_bound1 != bound12) || (i_bound2 != bound22)) {
            *i_grid2 = DupNode (grid2);
            WLGRID_BOUND1 ((*i_grid2)) = i_bound1;
            WLGRID_BOUND2 ((*i_grid2)) = i_bound2;
        }
    }

    DBUG_VOID_RETURN
}

/******************************************************************************
 *
 * function:
 *   node *MergeWL( node *nodes)
 *
 * description:
 *   returns the merged chain 'nodes'.
 *   if necessary (e.g. if called from 'ComputeCubes') the bounds of the
 *     chain-elements are adjusted.
 *
 ******************************************************************************/

node *
MergeWL (node *nodes)
{
    node *node1, *grids, *new_grids, *grid1, *grid2, *i_grid1, *i_grid2, *tmp;
    int bound1, bound2, step, rear1, count, fixpoint, i;

    DBUG_ENTER ("MergeWL");

    node1 = nodes;
    while (node1 != NULL) {

        /*
         * get all nodes with same bounds as 'node1'
         *
         * (because of the sort order these nodes are
         * located directly after 'node1' in the chain)
         */

        switch (NODE_TYPE (node1)) {

        case N_WLblock:
            /* here is no break missing! */
        case N_WLublock:
            /* here is no break missing! */
        case N_WLgrid:

            while ((WLNODE_NEXT (node1) != NULL)
                   && (WLNODE_BOUND1 (node1) == WLNODE_BOUND1 (WLNODE_NEXT (node1)))) {

                DBUG_ASSERT ((WLNODE_BOUND2 (node1)
                              == WLNODE_BOUND2 (WLNODE_NEXT (node1))),
                             "wrong bounds found");
                DBUG_ASSERT ((WLNODE_NEXTDIM (node1) != NULL), "dim not found");
                DBUG_ASSERT ((WLNODE_NEXTDIM (WLNODE_NEXT (node1)) != NULL),
                             "dim not found");

                /*
                 * merge 'node1' with his successor
                 */
                WLNODE_NEXTDIM (node1)
                  = InsertWLnodes (WLNODE_NEXTDIM (node1),
                                   WLNODE_NEXTDIM (WLNODE_NEXT (node1)));

                /* the remaining block node is useless now */
                WLNODE_NEXTDIM (WLNODE_NEXT (node1)) = NULL;
                WLNODE_NEXT (node1) = FreeNode (WLNODE_NEXT (node1));
                /* 'WLNODE_NEXT( node1)' points to his successor now */

                /* merge next dimension */
                WLNODE_NEXTDIM (node1) = MergeWL (WLNODE_NEXTDIM (node1));
            }
            break;

        case N_WLstride:

            /*
             * compute new bounds and step
             *             ^^^^^^
             * CAUTION: when called by 'ComputeCubes' the bounds are not equal!!
             */
            rear1 = IndexRearStride (node1);
            bound1 = WLSTRIDE_BOUND1 (node1);
            bound2 = WLSTRIDE_BOUND2 (node1);
            step = WLSTRIDE_STEP (node1);
            count = 0;
            tmp = WLSTRIDE_NEXT (node1);
            while ((tmp != NULL) && (IndexHeadStride (tmp) < rear1)) {
                /* compute new bounds */
                bound1 = MAX (bound1, WLSTRIDE_BOUND1 (tmp));
                bound2 = MIN (bound2, WLSTRIDE_BOUND2 (tmp));
                /* compute new step */
                step = lcm (step, WLSTRIDE_STEP (tmp));
                /* count the number of found dimensions for next traversal */
                count++;
                tmp = WLSTRIDE_NEXT (tmp);
            }

            /*
             * fit all grids to new step and collect them in 'grids'
             */
            grids = NewStepGrids (WLSTRIDE_CONTENTS (node1), WLSTRIDE_STEP (node1), step,
                                  bound1 - WLSTRIDE_BOUND1 (node1));
            for (i = 0; i < count; i++) {
                grids
                  = InsertWLnodes (grids,
                                   NewStepGrids (WLSTRIDE_CONTENTS (
                                                   WLSTRIDE_NEXT (node1)),
                                                 WLSTRIDE_STEP (WLSTRIDE_NEXT (node1)),
                                                 step,
                                                 bound1
                                                   - WLSTRIDE_BOUND1 (
                                                       WLSTRIDE_NEXT (node1))));

                /* the remaining block node is useless now */
                WLSTRIDE_CONTENTS (WLSTRIDE_NEXT (node1)) = NULL;
                WLSTRIDE_NEXT (node1) = FreeNode (WLSTRIDE_NEXT (node1));
                /* 'WLSTRIDE_NEXT( node1)' points to his successor now */
            }

            /*
             * intersect all grids with each other
             *   until fixpoint is reached.
             */
            do {
                fixpoint = 1;
                new_grids = NULL;

                /* check WLGRID_MODIFIED */
                grid1 = grids;
                while (grid1 != NULL) {
                    DBUG_ASSERT ((WLGRID_MODIFIED (grid1) == 0), "grid was modified");
                    grid1 = WLGRID_NEXT (grid1);
                }

                grid1 = grids;
                while (grid1 != NULL) {

                    grid2 = WLGRID_NEXT (grid1);
                    while (grid2 != NULL) {
                        IntersectGrid (grid1, grid2, step, &i_grid1, &i_grid2);
                        if (i_grid1 != NULL) {
                            new_grids = InsertWLnodes (new_grids, i_grid1);
                            WLGRID_MODIFIED (grid1) = 1;
                            fixpoint = 0;
                        }
                        if (i_grid2 != NULL) {
                            new_grids = InsertWLnodes (new_grids, i_grid2);
                            WLGRID_MODIFIED (grid2) = 1;
                            fixpoint = 0;
                        }

                        grid2 = WLGRID_NEXT (grid2);
                    }

                    /* was 'grid1' not modified? */
                    if (WLGRID_MODIFIED (grid1) == 0) {
                        /* insert 'grid1' in 'new_grids' */
                        tmp = grid1;
                        grid1 = WLGRID_NEXT (grid1);
                        WLGRID_NEXT (tmp) = NULL;
                        new_grids = InsertWLnodes (new_grids, tmp);
                    } else {
                        /* 'grid1' is no longer needed */
                        grid1 = FreeNode (grid1);
                        /* 'grid1' points to his successor now! */
                    }
                }

                grids = new_grids;
            } while (!fixpoint);

            /*
             * merge the grids
             */
            WLSTRIDE_BOUND1 (node1) = bound1;
            WLSTRIDE_BOUND2 (node1) = bound2;
            WLSTRIDE_STEP (node1) = step;
            WLSTRIDE_CONTENTS (node1) = MergeWL (grids);
            break;

        default:

            DBUG_ASSERT ((0), "wrong node type");
        }

        node1 = WLNODE_NEXT (node1);
    }

    DBUG_RETURN (nodes);
}

/******************************************************************************
 *
 * function:
 *   int IsEqualWLnodes( node *tree1, node *tree2)
 *
 * description:
 *   returns 1 if the N_WL...-trees 'tree1' and 'tree2' are equal.
 *   returns 0 otherwise.
 *
 *   remark: we can not use 'CompareWlnodes' here, because this function only
 *           compares the first level of dimensions.
 *           but here we must compare the *whole* trees --- all block levels,
 *           and even the code ...
 *
 ******************************************************************************/

int
IsEqualWLnodes (node *tree1, node *tree2)
{
    node *tmp1, *tmp2;
    int equal = 1;

    DBUG_ENTER ("IsEqualWLnodes");

    if ((tree1 != NULL) && (tree2 != NULL)) {

        DBUG_ASSERT ((NODE_TYPE (tree1) == NODE_TYPE (tree2)),
                     "can not compare objects of different type");

        /*
         * compare the whole chains
         */
        tmp1 = tree1;
        tmp2 = tree2;
        do {

            /*
             * compare type-independent data
             */
            if ((WLNODE_BOUND1 (tmp1) == WLNODE_BOUND1 (tmp2))
                && (WLNODE_BOUND2 (tmp1) == WLNODE_BOUND2 (tmp2))
                && (WLNODE_STEP (tmp1) == WLNODE_STEP (tmp2))) {

                /*
                 * compare type-specific data
                 */
                switch (NODE_TYPE (tmp1)) {

                case N_WLblock:
                    /* here is no break missing! */
                case N_WLublock:

                    /*
                     * CAUTION: to prevent nice ;-) bugs in this code fragment
                     *          WLBLOCK_CONTENTS and WLUBLOCK_CONTENTS must be
                     *          equivalent (as currently realized in tree_basic.h)
                     */
                    if (WLNODE_NEXTDIM (tmp1) != NULL) {
                        /*
                         * compare NEXTDIM (CONTENTS is NULL)
                         */
                        DBUG_ASSERT ((WLBLOCK_CONTENTS (tmp1) == NULL),
                                     "data in NEXTDIM *and* CONTENTS found");
                        DBUG_ASSERT ((WLBLOCK_CONTENTS (tmp2) == NULL),
                                     "data in NEXTDIM *and* CONTENTS found");
                        equal
                          = IsEqualWLnodes (WLNODE_NEXTDIM (tmp1), WLNODE_NEXTDIM (tmp2));
                    } else {
                        /*
                         * compare CONTENTS (NEXTDIM is NULL)
                         */
                        equal = IsEqualWLnodes (WLBLOCK_CONTENTS (tmp1),
                                                WLBLOCK_CONTENTS (tmp2));
                    }
                    break;

                case N_WLstride:

                    equal = IsEqualWLnodes (WLSTRIDE_CONTENTS (tmp1),
                                            WLSTRIDE_CONTENTS (tmp2));
                    break;

                case N_WLgrid:

                    if (WLGRID_NEXTDIM (tmp1) != NULL) {
                        /*
                         * compare NEXTDIM (CODE is NULL)
                         */
                        DBUG_ASSERT ((WLGRID_CODE (tmp1) == NULL),
                                     "data in NEXTDIM *and* CODE found");
                        DBUG_ASSERT ((WLGRID_CODE (tmp2) == NULL),
                                     "data in NEXTDIM *and* CODE found");
                        equal
                          = IsEqualWLnodes (WLGRID_NEXTDIM (tmp1), WLGRID_NEXTDIM (tmp2));
                    } else {
                        /*
                         * compare CODE (NEXTDIM is NULL)
                         */
                        equal = (WLGRID_CODE (tmp1) == WLGRID_CODE (tmp2));
                    }
                    break;

                default:

                    DBUG_ASSERT ((0), "wrong node type");
                }

            } else {
                equal = 0;
            }

            tmp1 = WLNODE_NEXT (tmp1);
            tmp2 = WLNODE_NEXT (tmp2);
        } while (equal && (tmp1 != NULL));

    } else {
        DBUG_ASSERT (((tree1 == NULL) && (tree2 == NULL)),
                     "trees differ in their depths");
        equal = 1;
    }

    DBUG_RETURN (equal);
}

/******************************************************************************
 *
 * function:
 *   node *OptimizeWL( node *nodes)
 *
 * description:
 *   returns the optimized N_WL...-tree 'nodes'.
 *
 ******************************************************************************/

node *
OptimizeWL (node *nodes)
{
    node *next, *grids, *comp1, *comp2;
    int offset;

    DBUG_ENTER ("OptimizeWL");

    if (nodes != NULL) {

        /*
         * optimize the next node
         */
        next = WLNODE_NEXT (nodes) = OptimizeWL (WLNODE_NEXT (nodes));

        /*
         * optimize the type-specific sons
         *
         * save in 'comp1', 'comp2' the son of 'nodes', 'next' respectively.
         */
        switch (NODE_TYPE (nodes)) {

        case N_WLblock:
            /* here is no break missing! */
        case N_WLublock:

            if (WLBLOCK_NEXTDIM (nodes) != NULL) {
                /*
                 * compare NEXTDIM (CONTENTS is NULL)
                 */
                DBUG_ASSERT ((WLBLOCK_CONTENTS (nodes) == NULL),
                             "data in NEXTDIM *and* CONTENTS found");
                comp1 = WLBLOCK_NEXTDIM (nodes) = OptimizeWL (WLBLOCK_NEXTDIM (nodes));
                if (next != NULL) {
                    comp2 = WLBLOCK_NEXTDIM (next);
                }
            } else {
                /*
                 * compare CONTENTS (NEXTDIM is NULL)
                 */
                comp1 = WLBLOCK_CONTENTS (nodes) = OptimizeWL (WLBLOCK_CONTENTS (nodes));
                if (next != NULL) {
                    comp2 = WLBLOCK_CONTENTS (next);
                }
            }
            break;

        case N_WLstride:

            comp1 = WLSTRIDE_CONTENTS (nodes) = OptimizeWL (WLSTRIDE_CONTENTS (nodes));
            if (next != NULL) {
                comp2 = WLSTRIDE_CONTENTS (next);
            }

            /*
             * if the grids contained in the stride have an offset
             * (the first grid does not begin at index 0), remove this offset.
             */
            grids = comp1;
            DBUG_ASSERT ((grids != NULL), "no grid found");
            offset = WLGRID_BOUND1 (grids);
            WLSTRIDE_BOUND1 (nodes) += offset;
            if (offset > 0) {
                do {
                    WLGRID_BOUND1 (grids) -= offset;
                    WLGRID_BOUND2 (grids) -= offset;
                    grids = WLGRID_NEXT (grids);
                } while (grids != NULL);
            }

            /*
             * if the first (and only) grid fills the whole step range
             *   set upper bound of this grid and step to 1
             */
            DBUG_ASSERT ((comp1 != NULL), "no grid found");
            if ((WLGRID_BOUND1 (comp1) == 0)
                && (WLGRID_BOUND2 (comp1) == WLSTRIDE_STEP (nodes))) {
                WLGRID_BOUND2 (comp1) = WLSTRIDE_STEP (nodes) = 1;
            }
            break;

        case N_WLgrid:

            if (WLGRID_NEXTDIM (nodes) != NULL) {
                /*
                 * compare NEXTDIM (CODE is NULL)
                 */
                DBUG_ASSERT ((WLGRID_CODE (nodes) == NULL),
                             "data in NEXTDIM *and* CODE found");
                comp1 = WLGRID_NEXTDIM (nodes) = OptimizeWL (WLGRID_NEXTDIM (nodes));
                if (next != NULL) {
                    comp2 = WLGRID_NEXTDIM (next);
                }
            } else {
                /*
                 * compare CODE (NEXTDIM is NULL)
                 */
                comp1 = WLGRID_CODE (nodes);
                if (next != NULL) {
                    comp2 = WLGRID_CODE (next);
                }
            }
            break;

        default:

            DBUG_ASSERT ((0), "wrong node type");
        }

        /*
         * if 'comp1' and 'comp2' are equal subtrees
         *   we can concate 'nodes' and 'next'
         */
        if (next != NULL) {
            if ((WLNODE_STEP (nodes) == WLNODE_STEP (next))
                && (WLNODE_BOUND2 (nodes) == WLNODE_BOUND1 (next))) {
                if ((NODE_TYPE (comp1) != N_Ncode) ? IsEqualWLnodes (comp1, comp2)
                                                   : (comp1 == comp2)) {
                    /* concate 'nodes' and 'next' */
                    WLNODE_BOUND2 (nodes) = WLNODE_BOUND2 (next);
                    /* free useless data in 'next' */
                    WLNODE_NEXT (nodes) = FreeNode (WLNODE_NEXT (nodes));
                }
            }
        }
    }

    DBUG_RETURN (nodes);
}

/******************************************************************************
 *
 * function:
 *   int GetMaxUnroll( node *nodes, int unroll, int dim)
 *
 * description:
 *   returns the maximally number elements that must be unrolled
 *     in dimension 'dim' of N_WL...-tree 'nodes'.
 *   'unroll' is the initial value for the computation (normally 1).
 *
 *   we must search for the first N_WLublock- or N_WLstride-node in each
 *     leaf of the 'nodes'-tree and get the step of this node.
 *
 ******************************************************************************/

int
GetMaxUnroll (node *nodes, int unroll, int dim)
{
    DBUG_ENTER ("GetMaxUnroll");

    if (nodes != NULL) {

        unroll = GetMaxUnroll (WLNODE_NEXT (nodes), unroll, dim);

        if ((WLNODE_DIM (nodes) == dim)
            && ((NODE_TYPE (nodes) == N_WLublock) || (NODE_TYPE (nodes) == N_WLstride))) {

            /*
             * we have found a node with unrolling information
             */
            unroll = MAX (unroll, WLNODE_STEP (nodes));

        } else {

            /*
             * search in whole tree for nodes with unrolling information
             */
            switch (NODE_TYPE (nodes)) {

            case N_WLblock:
                /* here is no break missing! */
            case N_WLublock:

                unroll = GetMaxUnroll (WLBLOCK_NEXTDIM (nodes), unroll, dim);
                unroll = GetMaxUnroll (WLBLOCK_CONTENTS (nodes), unroll, dim);
                break;

            case N_WLstride:

                unroll = GetMaxUnroll (WLSTRIDE_CONTENTS (nodes), unroll, dim);
                break;

            case N_WLgrid:

                unroll = GetMaxUnroll (WLBLOCK_NEXTDIM (nodes), unroll, dim);
                break;

            default:

                DBUG_ASSERT ((0), "wrong node type");
            }
        }
    }

    DBUG_RETURN (unroll);
}

/******************************************************************************
 *
 * function:
 *   node *FitWL( node *nodes, int curr_dim, int dims)
 *
 * description:
 *   returns the fitted N_WL...-tree 'nodes'.
 *   the tree is fitted in the dimension from 'curr_dim' till ('dims'-1).
 *
 ******************************************************************************/

node *
FitWL (node *nodes, int curr_dim, int dims)
{
    node *new_node, *grids, *tmp;
    int unroll, remain, width;

    DBUG_ENTER ("FitWL");

    if (curr_dim < dims) {

        /*
         * traverse the whole chain
         */
        tmp = nodes;
        while (tmp != NULL) {

            switch (NODE_TYPE (tmp)) {

            case N_WLblock:

                if (curr_dim < dims - 1) {
                    /*
                     * fit in next dimension;
                     *   compute unrolling information
                     */
                    WLBLOCK_NEXTDIM (tmp)
                      = FitWL (WLBLOCK_NEXTDIM (tmp), curr_dim + 1, dims);
                    unroll = GetMaxUnroll (WLBLOCK_NEXTDIM (tmp), 1, curr_dim);
                } else {
                    unroll = GetMaxUnroll (WLBLOCK_CONTENTS (tmp), 1, curr_dim);
                }
                break;

            case N_WLublock:

                if (curr_dim < dims - 1) {
                    /*
                     * fit in next dimension;
                     *   get unrolling information
                     */
                    WLBLOCK_NEXTDIM (tmp)
                      = FitWL (WLBLOCK_NEXTDIM (tmp), curr_dim + 1, dims);
                }
                unroll = WLUBLOCK_STEP (tmp);
                break;

            case N_WLstride:

                grids = WLSTRIDE_CONTENTS (tmp);
                if (curr_dim < dims - 1) {
                    /*
                     * fit for all grids in next dimension;
                     *   get unrolling information
                     */
                    while (grids != NULL) {
                        WLGRID_NEXTDIM (grids)
                          = FitWL (WLGRID_NEXTDIM (grids), curr_dim + 1, dims);
                        grids = WLGRID_NEXT (grids);
                    }
                }
                unroll = WLSTRIDE_STEP (tmp);
                break;

            default:

                DBUG_ASSERT ((0), "wrong node type");
            }

            /*
             * fit current dimension:
             *   split a uncompleted periode at the end of index range
             */
            width = WLNODE_BOUND2 (tmp) - WLNODE_BOUND1 (tmp);
            remain = width % unroll;
            if ((remain > 0) && (width > remain)) {
                /*
                 *  uncompleted periode found -> split
                 */
                new_node = DupNode (tmp);
                WLNODE_BOUND2 (new_node) = WLNODE_BOUND2 (tmp);
                WLNODE_BOUND2 (tmp) = WLNODE_BOUND1 (new_node)
                  = WLNODE_BOUND2 (tmp) - remain;
                WLNODE_NEXT (new_node) = WLNODE_NEXT (tmp);
                WLNODE_NEXT (tmp) = new_node;
                tmp = new_node;
            }

            tmp = WLNODE_NEXT (tmp);
        }
    }

    DBUG_RETURN (nodes);
}

/******************************************************************************
 *
 * function:
 *   node *NormalizeWLnodes( node *nodes, int *width)
 *
 * description:
 *   returns the normalized N_WL...-tree 'nodes'.
 *   'width' is an array with one component for each dimension;
 *     here we save the width of the index ranges
 *
 ******************************************************************************/

node *
NormalizeWLnodes (node *nodes, int *width)
{
    node *tmp;
    int curr_width;

    DBUG_ENTER ("NormalizeWLnodes");

    if (nodes != NULL) {

        /*
         * backup width of current dim
         */
        curr_width = width[WLNODE_DIM (nodes)];

        tmp = nodes;
        do {

            /*
             * adjust upper bound
             */
            DBUG_ASSERT ((WLNODE_BOUND1 (tmp) < curr_width), "wrong bounds found");
            WLNODE_BOUND2 (tmp) = MIN (WLNODE_BOUND2 (tmp), curr_width);

            /*
             * remove nodes whose index ranges lies outside the current block
             */
            while ((WLNODE_NEXT (tmp) != NULL)
                   && (WLNODE_BOUND1 (WLNODE_NEXT (tmp)) >= curr_width)) {
                WLNODE_NEXT (tmp) = FreeNode (WLNODE_NEXT (tmp));
            }

            /* take next node */
            tmp = WLNODE_NEXT (tmp);
        } while (tmp != NULL);

        tmp = nodes;
        do {

            /*
             * save width of current index range; adjust step
             */
            width[WLNODE_DIM (tmp)] = WLNODE_BOUND2 (tmp) - WLNODE_BOUND1 (tmp);
            WLNODE_STEP (tmp) = MIN (WLNODE_STEP (tmp), width[WLNODE_DIM (tmp)]);

            /*
             * normalize the type-specific sons
             */
            switch (NODE_TYPE (tmp)) {

            case N_WLblock:
                /* here is no break missing! */
            case N_WLublock:

                WLBLOCK_NEXTDIM (tmp) = NormalizeWLnodes (WLBLOCK_NEXTDIM (tmp), width);
                WLBLOCK_CONTENTS (tmp) = NormalizeWLnodes (WLBLOCK_CONTENTS (tmp), width);
                break;

            case N_WLstride:

                WLSTRIDE_CONTENTS (tmp)
                  = NormalizeWLnodes (WLSTRIDE_CONTENTS (tmp), width);
                break;

            case N_WLgrid:

                WLGRID_NEXTDIM (tmp) = NormalizeWLnodes (WLGRID_NEXTDIM (tmp), width);
                break;

            default:

                DBUG_ASSERT ((0), "wrong node type");
            }

            /* take next node */
            tmp = WLNODE_NEXT (tmp);
        } while (tmp != NULL);

        /*
         * restore width of current dim
         */
        width[WLNODE_DIM (nodes)] = curr_width;
    }

    DBUG_RETURN (nodes);
}

/******************************************************************************
 *
 * function:
 *   node *NormalizeWL( node *nodes, int dims)
 *
 * description:
 *   returns the normalized N_WL...-tree 'nodes'.
 *   'dims' is the number of dimension in 'nodes'.
 *
 ******************************************************************************/

node *
NormalizeWL (node *nodes, int dims)
{
    int *width;
    int d;

    DBUG_ENTER ("NormalizeWL");

    /*
     * initialize 'width' with the maximum value for int
     */
    width = (int *)MALLOC (dims * sizeof (int));
    for (d = 0; d < dims; d++) {
        width[d] = INT_MAX;
    }

    nodes = NormalizeWLnodes (nodes, width);

    FREE (width);

    DBUG_RETURN (nodes);
}

/******************************************************************************
 *
 * function:
 *   node *DupOutline( node *stride_var)
 *
 * description:
 *   Duplicates the outline of 'stride_var' (N_WLstride, N_WLstriVar):
 *      a -> b step s:                a+c -> b step 1:
 *             c -> d: op      ->>             0 -> 1: op
 *
 *   This function is called by 'ComputeOneCube'.
 *
 ******************************************************************************/

node *
DupOutline (node *stride_var)
{
    node *grid_var, *new_stride_var = NULL;

    DBUG_ENTER ("DupOutline");

    if (stride_var != NULL) {
        if (NODE_TYPE (stride_var) == N_WLstride) {

            grid_var = WLSTRIDE_CONTENTS (stride_var);

            new_stride_var
              = MakeWLstride (WLSTRIDE_LEVEL (stride_var), WLSTRIDE_DIM (stride_var),
                              WLSTRIDE_BOUND1 (stride_var) + WLGRID_BOUND1 (grid_var),
                              WLSTRIDE_BOUND2 (stride_var), 1,
                              WLSTRIDE_UNROLLING (stride_var),
                              WLSTRIDE_CONTENTS (stride_var), NULL);
            WLSTRIDE_PART (new_stride_var) = WLSTRIDE_PART (stride_var);

            WLSTRIDE_CONTENTS (new_stride_var)
              = MakeWLgrid (WLGRID_LEVEL (grid_var), WLGRID_DIM (grid_var), 0, 1,
                            WLGRID_UNROLLING (grid_var), NULL, NULL, NULL);
            WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (new_stride_var))
              = DupOutline (WLGRID_NEXTDIM (grid_var));

        } else {
        }
    }

    DBUG_RETURN (new_stride_var);
}

/******************************************************************************
 *
 * function:
 *   node* ComputeOneCube( node *stride_var)
 *
 * description:
 *   In with-loops, which have one part/generator only, this generator
 *   possibly is not a complete cube but a grid (e.g. fold with generator
 *   (step < width)).
 *   The compilation-scheme can not handle these incomplete grids, therefore
 *   this function supplements missings parts of the grid to get a complete
 *   cube:
 *         60 -> 200 step 50                  100 -> 200 step 50
 *                  40 -> 50: op0     =>>                0 -> 10: op0
 *                                                      40 -> 50: noop
 *
 ******************************************************************************/

node *
ComputeOneCube (node *stride_var)
{
    node *grid_var;

    DBUG_ENTER ("ComputeOneCube");

    if (stride_var != NULL) {
        if (NODE_TYPE (stride_var) == N_WLstride) {

            grid_var = WLSTRIDE_CONTENTS (stride_var);
            if (WLGRID_BOUND1 (grid_var) > 0) {

                WLSTRIDE_BOUND1 (stride_var) += WLGRID_BOUND1 (grid_var);
                WLGRID_BOUND2 (grid_var) -= WLGRID_BOUND1 (grid_var);
                WLGRID_BOUND1 (grid_var) = 0;

                WLGRID_NEXT (grid_var)
                  = MakeWLgrid (WLGRID_LEVEL (grid_var), WLGRID_DIM (grid_var),
                                WLGRID_BOUND2 (grid_var), WLSTRIDE_STEP (stride_var),
                                WLGRID_UNROLLING (grid_var),
                                DupOutline (WLGRID_NEXTDIM (grid_var)), NULL, NULL);
            }
            WLGRID_NEXTDIM (grid_var) = ComputeOneCube (WLGRID_NEXTDIM (grid_var));

        } else {

            grid_var = WLSTRIVAR_CONTENTS (stride_var);
#if 0
      if (WLGRIDVAR_BOUND1( grid_var) > 0) {
      
        WLSTRIVAR_BOUND1( stride_var) += WLGRIDVAR_BOUND1( grid_var);
        WLGRIDVAR_BOUND2( grid_var) -= WLGRIDVAR_BOUND1( grid_var);
        WLGRIDVAR_BOUND1( grid_var) = 0;

        WLGRIDVAR_NEXT( grid_var)
          = MakeWLgrid( WLGRIDVAR_LEVEL( grid_var),
                        WLGRIDVAR_DIM( grid_var),
                        WLGRIDVAR_BOUND2( grid_var),
                        WLSTRIVAR_STEP( stride_var),
                        WLGRIDVAR_UNROLLING( grid_var),
                        DupOutline( WLGRIDVAR_NEXTDIM( grid_var)),
                        NULL,
                        NULL);
      }
      WLGRIDVAR_NEXTDIM( grid_var)
        = ComputeOneCube( WLGRIDVAR_NEXTDIM( grid_var));
#endif
        }
    }

    DBUG_RETURN (stride_var);
}

/******************************************************************************
 *
 * function:
 *   node *ComputeCubes( node *strides)
 *
 * description:
 *   returns the set of cubes as a N_WLstride-chain
 *
 ******************************************************************************/

node *
ComputeCubes (node *strides)
{
    node *new_strides, *stride1, *stride2, *i_stride1, *i_stride2, *remain, *last_remain,
      *last_stride1, *tmp;
    int fixpoint;

    DBUG_ENTER ("ComputeCubes");

    DBUG_ASSERT ((NODE_TYPE (strides) == N_WLstride), "wrong node type found");

    /*
     * first step:
     *
     * if a stride contains
     */

    /*
     * second step:
     *
     * create disjunct outlines
     *  -> every stride lies in one and only one cube
     */
    do {
        fixpoint = 1;
        new_strides = NULL;

        /* check WLSTRIDE_MODIFIED */
        stride1 = strides;
        while (stride1 != NULL) {
            DBUG_ASSERT ((WLSTRIDE_MODIFIED (stride1) == 0), "stride was modified");
            stride1 = WLSTRIDE_NEXT (stride1);
        }

        /* intersect the elements of 'strides' in pairs */
        stride1 = strides;
        while (stride1 != NULL) {

            stride2 = WLSTRIDE_NEXT (stride1);
            while (stride2 != NULL) {

                /* intersect outlines of 'stride1' and 'stride2' */
                IntersectOutline (stride1, stride2, &i_stride1, &i_stride2);

                if (i_stride1 != NULL) {
                    if (CompareWLnode (stride1, i_stride1, 1) != 0) {
                        fixpoint = 0;
                        WLSTRIDE_MODIFIED (stride1) = 1;
                        new_strides = InsertWLnodes (new_strides, i_stride1);
                    } else {
                        /*
                         * 'stride1' and 'i_stride1' are equal
                         *  -> free 'i_stride1'
                         */
                        i_stride1 = FreeTree (i_stride1);
                    }
                }

                if (i_stride2 != NULL) {
                    if (CompareWLnode (stride2, i_stride2, 1) != 0) {
                        fixpoint = 0;
                        WLSTRIDE_MODIFIED (stride2) = 1;
                        new_strides = InsertWLnodes (new_strides, i_stride2);
                    } else {
                        /*
                         * 'stride2' and 'i_stride2' are equal
                         *  -> free 'i_stride2'
                         */
                        i_stride2 = FreeTree (i_stride2);
                    }
                }

                stride2 = WLSTRIDE_NEXT (stride2);
            }

            /* was 'stride1' not modified? */
            if (WLSTRIDE_MODIFIED (stride1) == 0) {
                /* insert 'stride1' in 'new_strides' */
                tmp = stride1;
                stride1 = WLSTRIDE_NEXT (stride1);
                WLSTRIDE_NEXT (tmp) = NULL;
                new_strides = InsertWLnodes (new_strides, tmp);
            } else {
                /* 'stride1' is no longer needed */
                stride1 = FreeNode (stride1);
                /* 'stride1' points to his successor now! */
            }
        }

        strides = new_strides;
    } while (!fixpoint);

    /*
     * third step:
     *
     * merge the strides of each cube
     */
    stride1 = strides;
    while (stride1 != NULL) {

        /*
         * collect all strides, that lie in the same cube as 'stride1'.
         * 'remain' collects the remaining strides.
         */
        stride2 = WLSTRIDE_NEXT (stride1);
        last_stride1 = NULL;
        remain = last_remain = NULL;
        while (stride2 != NULL) {

            /* lie 'stride1' and 'stride2' in the same cube? */
            if (IntersectOutline (stride1, stride2, NULL, NULL)) {
                /*
                 * 'stride1' and 'stride2' lie in the same cube
                 *  -> append 'stride2' to the 'stride1'-chain
                 */
                if (last_stride1 == NULL) {
                    WLSTRIDE_NEXT (stride1) = stride2;
                } else {
                    WLSTRIDE_NEXT (last_stride1) = stride2;
                }
                last_stride1 = stride2;
            } else {
                /*
                 * 'stride2' lies not in the same cube as 'stride1'
                 *  -> append 'stride2' to to 'remain'-chain
                 */
                if (remain == NULL) {
                    remain = stride2;
                } else {
                    WLSTRIDE_NEXT (last_remain) = stride2;
                }
                last_remain = stride2;
            }

            stride2 = WLSTRIDE_NEXT (stride2);
        }

        /*
         * merge the 'stride1'-chain
         */
        if (last_stride1 != NULL) {
            WLSTRIDE_NEXT (last_stride1) = NULL;
            stride1 = MergeWL (stride1);
        }

        if (strides == NULL) {
            strides = stride1;
        }

        WLSTRIDE_NEXT (stride1) = remain;
        if (last_remain != NULL) {
            WLSTRIDE_NEXT (last_remain) = NULL;
        }
        stride1 = remain;
    }
    strides = new_strides;

    DBUG_RETURN (strides);
}

/******************************************************************************
 *
 * function:
 *   node *WLTRANwith( node *arg_node, node *arg_info)
 *
 * description:
 *   precompilation of new with-loop (N_Nwith-node)
 *
 ******************************************************************************/

node *
WLTRANwith (node *arg_node, node *arg_info)
{
    node *new_node, *strides, *cubes, *segs, *seg;
    int dims, b;
    enum {
        WL_PH_cube,
        WL_PH_seg,
        WL_PH_split,
        WL_PH_block,
        WL_PH_ublock,
        WL_PH_merge,
        WL_PH_opt,
        WL_PH_fit,
        WL_PH_norm
    } WL_break_after;

    DBUG_ENTER ("WLTRANwith");

    /* analyse 'break_specifier' */
    WL_break_after = WL_PH_norm;
    if (break_after == PH_wltrans) {
        if (strcmp (break_specifier, "cubes") == 0) {
            WL_break_after = WL_PH_cube;
        } else {
            if (strcmp (break_specifier, "segs") == 0) {
                WL_break_after = WL_PH_seg;
            } else {
                if (strcmp (break_specifier, "split") == 0) {
                    WL_break_after = WL_PH_split;
                } else {
                    if (strcmp (break_specifier, "block") == 0) {
                        WL_break_after = WL_PH_block;
                    } else {
                        if (strcmp (break_specifier, "ublock") == 0) {
                            WL_break_after = WL_PH_ublock;
                        } else {
                            if (strcmp (break_specifier, "merge") == 0) {
                                WL_break_after = WL_PH_merge;
                            } else {
                                if (strcmp (break_specifier, "opt") == 0) {
                                    WL_break_after = WL_PH_opt;
                                } else {
                                    if (strcmp (break_specifier, "fit") == 0) {
                                        WL_break_after = WL_PH_fit;
                                    } else {
                                        if (strcmp (break_specifier, "norm") == 0) {
                                            WL_break_after = WL_PH_norm;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

    /*
     * get number of dims of with-loop index range
     */
    dims = IDS_SHAPE (NWITHID_VEC (NPART_WITHID (NWITH_PART (arg_node))), 0);

    new_node = MakeNWith2 (NPART_WITHID (NWITH_PART (arg_node)), NULL,
                           NWITH_CODE (arg_node), NWITH_WITHOP (arg_node), dims);

    NWITH2_DEC_RC_IDS (new_node) = NWITH_DEC_RC_IDS (arg_node);
    NWITH2_IN (new_node) = NWITH_IN (arg_node);
    NWITH2_INOUT (new_node) = NWITH_INOUT (arg_node);
    NWITH2_OUT (new_node) = NWITH_OUT (arg_node);
    NWITH2_LOCAL (new_node) = NWITH_LOCAL (arg_node);

    /*
     * withid, code, withop and IN/INOUT/OUT/LOCAL are overtaken to the Nwith2-tree
     *  without a change.
     * Because of that, these parts are cut off from the old nwith-tree,
     *  before freeing it.
     */

    NPART_WITHID (NWITH_PART (arg_node)) = NULL;
    NWITH_CODE (arg_node) = NULL;
    NWITH_WITHOP (arg_node) = NULL;

    NWITH_DEC_RC_IDS (arg_node) = NULL;
    NWITH_IN (arg_node) = NULL;
    NWITH_INOUT (arg_node) = NULL;
    NWITH_OUT (arg_node) = NULL;
    NWITH_LOCAL (arg_node) = NULL;

    DBUG_EXECUTE ("WLprec", NOTE (("step 0: converting parts to strides\n")));
    strides = Parts2Strides (NWITH_PART (arg_node), dims);

    /*
     * the params of all generators are constant
     */

    DBUG_EXECUTE ("WLprec", NOTE (("step 1: cube-building\n")));
    if (NPART_NEXT (NWITH_PART (arg_node)) == NULL) {
        cubes = ComputeOneCube (strides);
    } else {
        cubes = ComputeCubes (strides);
    }
    if ((WL_break_after == WL_PH_cube) || (NODE_TYPE (strides) != N_WLstride)) {
        /*
         * build one segment containing all the cubes
         */
        segs = MakeWLseg (dims, cubes, NULL);
    }

    if (NODE_TYPE (strides) == N_WLstride) {

        ComputeIndexMinMax (NWITH2_IDX_MIN (new_node), NWITH2_IDX_MAX (new_node),
                            NWITH2_DIMS (new_node), strides);

        if (WL_break_after >= WL_PH_seg) {
            DBUG_EXECUTE ("WLprec", NOTE (("step 2: choice of segments\n")));
            segs = SetSegs (NWITH_PRAGMA (arg_node), cubes, dims);
            /* free temporary data */
            if (NWITH_PRAGMA (arg_node) != NULL) {
                NWITH_PRAGMA (arg_node) = FreeTree (NWITH_PRAGMA (arg_node));
            }
            if (cubes != NULL) {
                cubes = FreeTree (cubes);
            }

            seg = segs;
            while (seg != NULL) {
                /* splitting */
                if (WL_break_after >= WL_PH_split) {
                    DBUG_EXECUTE ("WLprec", NOTE (("step 3: splitting\n")));
                    WLSEG_CONTENTS (seg) = SplitWL (WLSEG_CONTENTS (seg));
                }

                /* hierarchical blocking */
                if (WL_break_after >= WL_PH_block) {
                    DBUG_EXECUTE ("WLprec", NOTE (("step 4: hierarchical blocking\n")));
                    for (b = 0; b < WLSEG_BLOCKS (seg); b++) {
                        DBUG_EXECUTE ("WLprec",
                                      NOTE (
                                        ("step 4.%d: hierarchical blocking (level %d)\n",
                                         b + 1, b)));
                        WLSEG_CONTENTS (seg)
                          = BlockWL (WLSEG_CONTENTS (seg), dims, WLSEG_BV (seg, b), 0);
                    }
                }

                /* unrolling-blocking */
                if (WL_break_after >= WL_PH_ublock) {
                    DBUG_EXECUTE ("WLprec", NOTE (("step 5: unrolling-blocking\n")));
                    WLSEG_CONTENTS (seg)
                      = BlockWL (WLSEG_CONTENTS (seg), dims, WLSEG_UBV (seg), 1);
                }

                /* merging */
                if (WL_break_after >= WL_PH_merge) {
                    DBUG_EXECUTE ("WLprec", NOTE (("step 6: merging\n")));
                    WLSEG_CONTENTS (seg) = MergeWL (WLSEG_CONTENTS (seg));
                }

                /* optimization */
                if (WL_break_after >= WL_PH_opt) {
                    DBUG_EXECUTE ("WLprec", NOTE (("step 7: optimization\n")));
                    WLSEG_CONTENTS (seg) = OptimizeWL (WLSEG_CONTENTS (seg));
                }

                /* fitting */
                if (WL_break_after >= WL_PH_fit) {
                    DBUG_EXECUTE ("WLprec", NOTE (("step 8: fitting\n")));
                    WLSEG_CONTENTS (seg) = FitWL (WLSEG_CONTENTS (seg), 0, dims);
                }

                /* normalization */
                if (WL_break_after >= WL_PH_norm) {
                    DBUG_EXECUTE ("WLprec", NOTE (("step 9: normalization\n")));
                    WLSEG_CONTENTS (seg) = NormalizeWL (WLSEG_CONTENTS (seg), dims);
                }

                seg = WLSEG_NEXT (seg);
            }
        }
    }

    NWITH2_SEGS (new_node) = segs;

    arg_node = FreeTree (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLTRANcode( node *arg_node, node *arg_info)
 *
 * description:
 *   precompilation of Ncode-nodes.
 *
 * remarks:
 *   - CODE_NO is set in the whole Ncode-chain
 *
 ******************************************************************************/

node *
WLTRANcode (node *arg_node, node *arg_info)
{
    node *code;
    int no = 0;

    DBUG_ENTER ("WLTRANcode");

    code = arg_node;
    while (code != NULL) {
        NCODE_NO (code) = no;

        if (NCODE_CBLOCK (code) != NULL) {
            NCODE_CBLOCK (code) = Trav (NCODE_CBLOCK (code), arg_info);
        }
        if (NCODE_CEXPR (code) != NULL) {
            NCODE_CEXPR (code) = Trav (NCODE_CEXPR (code), arg_info);
        }

        no++;
        code = NCODE_NEXT (code);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLTRAFundef(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses sons.
 *
 ******************************************************************************/

node *
WLTRAFundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLTRAFundef");

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }
    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }
    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WlTransform( node *syntax_tree)
 *
 * description:
 *   In this compiler phase all N_Nwith nodes are transformed in N_Nwith2
 *   nodes.
 *
 ******************************************************************************/

node *
WlTransform (node *syntax_tree)
{
    node *info;

    DBUG_ENTER ("WlTransform");

    info = MakeInfo ();

    act_tab = wltrans_tab;
    syntax_tree = Trav (syntax_tree, info);

    FREE (info);

    DBUG_RETURN (syntax_tree);
}
