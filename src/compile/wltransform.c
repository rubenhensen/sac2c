/*
 *
 * $Log$
 * Revision 3.72  2003/03/13 00:13:35  dkr
 * comment in GetWlShape() added
 *
 * Revision 3.71  2003/03/12 23:34:07  dkr
 * SSA transform removed...
 *
 * Revision 3.70  2003/03/12 17:28:47  dkr
 * SSA form used if flag -ssa is set
 *
 * Revision 3.69  2003/03/05 15:33:09  dkr
 * bug in GetWlShape() fixed:
 * works correcly for nested type definitions (user defined types...) now
 *
 * Revision 3.68  2002/10/30 14:12:58  dkr
 * -enforceIEEE for WLs implemented
 *
 * Revision 3.67  2002/10/29 19:06:49  dkr
 * message of DBUG_PRINT modified
 *
 * Revision 3.66  2002/10/11 16:27:41  dkr
 * bug in Parts2Strides fixed
 *
 * Revision 3.65  2002/10/08 15:43:16  dkr
 * EmptyParts2StridesOrExpr(): default brance in switch contruct added in
 * order to please the cc.
 *
 * Revision 3.64  2002/10/07 23:35:29  dkr
 * some bugs fixed
 *
 * Revision 3.62  2002/08/15 11:58:23  dkr
 * EmptyParts2Expr(): no cc warning anymore
 *
 * Revision 3.61  2002/08/09 12:44:46  dkr
 * minor changes done
 *
 * Revision 3.60  2002/08/09 12:38:36  dkr
 * - more code brushing done
 * - INFO_WL_TYPES move from tree_basic.h to wltransform.c
 *
 * Revision 3.59  2002/08/08 12:34:58  dkr
 * code brushed, some DBUG_ASSERTs added for AKD, AUD
 *
 * Revision 3.58  2002/08/07 13:06:34  dkr
 * DBUG_ASSERT added
 *
 * Revision 3.57  2002/07/15 15:04:02  dkr
 * grrrr... modification of last revision undone
 *
 * Revision 3.54  2001/11/22 08:41:17  sbs
 * CheckWithids is only compiled in the dbug version since
 * it is used from DBUG_ASSERT only!
 *
 * Revision 3.53  2001/11/14 16:02:00  dkr
 * fixed a bug in OptWL()
 *
 * Revision 3.52  2001/06/28 09:30:42  sbs
 * catenation in WP macro def eliminated
 *
 * Revision 3.50  2001/05/17 12:03:24  dkr
 * FREE eliminated
 *
 * Revision 3.49  2001/05/03 17:31:44  dkr
 * MAXHOMDIM replaced by HOMSV
 *
 * Revision 3.48  2001/04/26 18:17:43  dkr
 * fixed a bug in WLTRAlet: LET_IDS may be NULL ...
 *
 * Revision 3.47  2001/04/10 13:17:22  dkr
 * file description modified
 *
 * Revision 3.46  2001/04/03 18:00:06  dkr
 * signature of IsHomSV modified
 *
 * Revision 3.45  2001/04/03 17:52:21  dkr
 * calculation of WLSEG_SV corrected (BV is also taken into account now)
 *
 * Revision 3.44  2001/04/03 10:47:08  dkr
 * minor changes in ComputeIndexMinMax() done
 *
 * Revision 3.42  2001/04/02 17:06:42  dkr
 * illegal wlcomp-pragma functions produce a warning now instead of an
 * error
 *
 * Revision 3.41  2001/04/02 16:11:06  dkr
 * Support for new with-loop bounds format added:
 * besides '[1,2,3]' and 'A', now also '[a,b,c]' works :-)
 *
 * Revision 3.40  2001/04/02 11:43:32  dkr
 * include of wl_bounds.h added
 *
 * Revision 3.39  2001/03/29 01:34:25  dkr
 * WLSEGVAR_IDX_MIN, WLSEGVAR_IDX_MAX are now a node vector :-)
 *
 * Revision 3.38  2001/03/22 19:19:02  dkr
 * include of tree.h eliminated
 *
 * Revision 3.37  2001/03/20 15:32:20  ben
 * prefix WLCOMP_ for wlcomp-functions added
 *
 * Revision 3.36  2001/03/20 15:26:11  dkr
 * WLSEG_HOMSV removed (WLSEG_SV used instead)
 *
 * Revision 3.35  2001/03/05 17:01:45  dkr
 * message text of DBUG_ASSERT about CheckWithids() modified
 *
 * Revision 3.34  2001/03/05 16:42:27  dkr
 * no macros NWITH???_IS_FOLD used
 *
 * Revision 3.33  2001/03/05 15:00:00  dkr
 * NCODE_NO no longer used here (but in print.c only)
 *
 * Revision 3.32  2001/02/22 10:47:18  sbs
 * compiler warning in GenerateCompleteDomain eliminated by
 * initializing act_compl_grid to NULL.
 *
 * Revision 3.31  2001/02/12 17:54:35  dkr
 * fixed a bug in GenerateCompleteDomain()
 *
 * Revision 3.30  2001/02/09 10:51:21  dkr
 * fixed a bug in InferFitted
 *
 * Revision 3.29  2001/02/08 16:30:13  dkr
 * warning about uninitialized variable eliminated
 *
 * Revision 3.28  2001/02/07 20:16:07  dkr
 * InsertNoopNodes(): NOOP optimization added
 *
 * Revision 3.27  2001/02/06 18:21:39  dkr
 * fixed a bug in Parts2Strides()
 *
 * Revision 3.26  2001/02/06 01:48:34  dkr
 * NOOP nodes added
 *
 * Revision 3.22  2001/01/29 19:21:21  dkr
 * fixed a bug in FitWL
 *
 * Revision 3.21  2001/01/29 18:34:51  dkr
 * some superfluous attributes of N_WLsegVar removed
 *
 * Revision 3.20  2001/01/25 14:35:47  dkr
 * InferSchedulingParams() renamed into InferSegsSchedulingParams().
 * InferSegParams() renamed into InferSegsParams().
 * signature of NormWL() modified.
 * NormWL() can handle also N_WLxblock-nodes now.
 *
 * Revision 3.19  2001/01/25 12:07:08  dkr
 * ResetBV() added
 *
 * Revision 3.18  2001/01/25 00:40:31  dkr
 * fixed a bug in WLTRAwith:
 * split, block, merge, opt, fit, norm are skipped for var. segments now
 *
 * Revision 3.17  2001/01/24 23:36:22  dkr
 * signature of MakeWLgridVar, MakeWLgrid, MakeWLseg, MakeWLsegVar
 * modified.
 * GenerateCompleteDomain() brushed.
 * WLGRIDX_FITTED used.
 *
 * Revision 3.16  2001/01/22 13:47:06  dkr
 * DBUG string WLprec renamed into WLtrans
 *
 * Revision 3.15  2001/01/19 11:57:40  dkr
 * usage of NodeOrInt_GetNameOrVal() modified
 *
 * Revision 3.12  2001/01/10 18:44:27  dkr
 * function ComputeIndexMinMax modified
 *
 * Revision 3.11  2001/01/10 11:44:18  dkr
 * WLBLOCKX_... macros used
 *
 * Revision 3.10  2001/01/09 20:00:10  dkr
 * code brushed
 * support for naive compilation extended
 * support for AKDs added (not finished yet)
 *
 * Revision 3.9  2001/01/09 17:28:36  dkr
 * N_WLstriVar renamed into N_WLstrideVar
 *
 * Revision 3.8  2001/01/08 16:11:57  dkr
 * support for naive compilation of with-loops added (not finished yet)
 *
 * Revision 3.7  2001/01/08 13:40:34  dkr
 * functions ExtractAplPragma... moved from wltransform.c to
 * wlpragma_funs.c
 *
 * Revision 3.6  2000/12/12 13:08:37  dkr
 * fixed in bug in WLTRAwith:
 * NWITH_OUT_MASK no longer accidentially removed
 *
 * Revision 3.5  2000/12/12 11:43:45  dkr
 * NWITH2_IN renamed into NWITH2_IN_MASK
 * NWITH2_INOUT removed
 *
 * Revision 3.2  2000/11/27 21:07:35  cg
 * APL entry of wlcomp pragma is extracted from wlcomp chain and
 * added to new Nwith2 node for further processing in compile.c.
 * Basic consistency checks and some transformations are applied
 * to parameters.
 *
 * Revision 3.1  2000/11/20 18:01:31  sacbase
 * new release made
 *
 * [...]
 *
 */

/******************************************************************************
 *
 * file: wltransform.c
 *
 * description:
 *
 * This module implements the transformation of the with-loops from the
 * frontend representation (N_Nwith) into the backend representation (N_Nwith2).
 *
 * *** CAUTION ***
 * For a successful transformation the AST has to meet some requirements:
 *   - For all N_Ngenerator nodes of a with-loop is hold:
 *       - OP1 equals <= and OP2 equals <.
 *       - BOUND1, BOUND2, STEP, WIDTH are not NULL.
 *       - BOUND1, BOUND2, STEP, WIDTH are N_id nodes, N_array nodes containing
 *         N_id nodes or N_array nodes containing N_num nodes
 *         (A, [a,b,c] or [1,2,3]).
 *   - For all N_withid nodes of a single with-loop is hold:
 *       - the same VEC and IDS names are used.
 *
 ******************************************************************************/

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "traverse.h"
#include "free.h"
#include "dbug.h"
#include "DupTree.h"
#include "DataFlowMask.h"
#include "print.h"
#include "NameTuplesUtils.h"
#include "constants.h"
#include "wl_bounds.h"
#include "wlpragma_funs.h"
#include "wltransform.h"

/*
 * access macros for 'arg_info'
 */
#define INFO_WL_TYPES(n) (n->info.types)

/*
 * here we store the lineno of the current with-loop
 *  (for creating error-messages ...)
 */
static int line;

/*****************************************************************************


Transformation N_Nwith -> N_Nwith2:
===================================


Example:
--------

  [  0,  0] -> [ 50,150] step [1,1] width [1,1]: e1
  [  0,150] -> [300,400] step [9,1] width [2,1]: e2
  [  2,150] -> [300,400] step [9,1] width [7,1]: e1
  [ 50,  0] -> [300,150] step [1,1] width [1,1]: e2
  [300,  0] -> [400,100] step [1,1] width [1,1]: e3
  [300,100] -> [400,400] step [1,3] width [1,1]: e3
  [300,101] -> [400,400] step [1,3] width [1,2]: e4


1.) Cube Calculation (Calculates the set of cubes)
--------------------

    -> set of cubes

  In the example:

      0-> 50, step[0] 1
                   0->1:   0->150, step[1] 1
                                        0->1: e1
      0->300, step[0] 9
                   0->2: 150->400, step[1] 1
                                        0->1: e2
                   2->9: 150->400, step[1] 1
                                        0->1: e1
     50->300, step[0] 1
                   0->1:   0->150, step[1] 1
                                        0->1: e2
    300->400, step[0] 1
                   0->1:   0->100, step[1] 1
                                        0->1: e3
    300->400, step[0] 1
                   0->1: 100->400, step[1] 3
                                        0->1: e3
                                        1->3: e4


2a.) Selection of segments and bv0, bv1, ... (blocking vektors),
     ---------------------     ubv (unrolling-blocking vector)

     Let sv be the global step vector of a segment S --- i.e. sv is the lcm of
     all steps from cubes for which the intersection with S is non-empty. Then
     for each dimension (d) ubv_d must be multiples of sv_d.
     (Later on during the compilation process [fitting phase] bv0_d, bv1_d, ...
     will be modified in a way that they are multiples of sv_d, too.)
     However, for the first components of bv0, bv1, ... and ubv also the
     value 1 is allowed: in this case no blocking is performed for these
     dimensions.

     If bv = (1, ..., 1, ?, gt, ..., gt) is hold --- gt means that bv_d is
     greater or equal the segment width ---, this is equivalent to
     bv = (1, ..., 1), provided that this is compatible to the value of ubv.
     Nevertheless, this simplification is *not* carried out by the compiler yet!

     -> set of segments (segment := a cube without a grid)

  In the example: We choose the complete shape as a segment and
                  bv0 = (180,158), ubv = (1,6) --- note that sv = (9,3).


2b.) Adjust the cubes to the segments
     --------------------------------

     -> every segment is decomposed into a set of cubes.

  In the example: For the chosen segment we get exactly the same cube set as
                  found in section 1.)


For every segment the following steps must be performed:


3.) Cube Splitting (Cuts the projections)
    --------------

    First the spitting is performed with all cubes in the 0th dimension: The
    cubes are splitted in the 0th dimension till the projections of its bounds
    in the 0th dimension are pairwise disjoint or identical.
    Afterwards the cubes are arranged in groups with identical 0-projections.
    With each of this groups the splitting is performed in the 1st dimension ...
    and so on.

    The splitting is a preparation for the merging (see step 6). At first glance
    it seems to be a good idea to perform splitting and merging alternating on
    each dimension: Each group formed after splitting a single dimension
    contains exactly the projections that must be joined in the merging step.
    Nevertheless it is easier and clearer to separate these two steps from each
    other: On the one hand the merging can be performed *after* the blocking
    step only (for the reasons see step 4.), on the other hand the splitting is
    carried out best just *before* the blocking, because while doing so, some
    grids might move (see example) --- i.e. the contents of a block would
    change!

  In the example:

      0-> 50, step[0] 1
                   0->1:   0->150, step[1] 1
                                        0->1: e1
      0-> 50, step[0] 9
                   0->2: 150->400, step[1] 1
                                        0->1: e2
                   2->9: 150->400, step[1] 1
                                        0->1: e1
     50->300, step[0] 1
                   0->1:   0->150, step[1] 1
                                        0->1: e2
     50->300, step[0] 9
                   0->4: 150->400, step[1] 1
                                        0->1: e1
                   4->6: 150->400, step[1] 1
                                        0->1: e2
                   6->9: 150->400, step[1] 1
                                        0->1: e1
    300->400, step[0] 1
                   0->1:   0->100, step[1] 1
                                        0->1: e3
    300->400, step[0] 1
                   0->1: 100->400, step[1] 3
                                        0->1: e3
                                        1->3: e4


4.) Blocking (without fitting) according to the values in bv
    --------

  The blocking is performed that early because the blocking changes the
  iteration order on the array. That means, all the following steps --- in
  perticulary merging and optimize --- must know the concrete blocking.
  For example: Without blocking the e1-cube is given a step of 9 while merging
               with the e2/e3-cube in the 0th dimension. Afterwards in the
               optimization phase the (0->2)-branches of the two parts can be
               joined.
               With blocking both things do not happen because the two parts
               lie in different blocks! Compare this with the example further
               down.

  The blocking is represented in the tree analogeous to cubes --- we must simply
  put a analogeous structured block hierarchie in front.

  In the example (bounds x10 for more realistic results):

    At the beginning of a new cube we need always a new block. Therefore for
    each cube a separate blocking-stride is needed:

    0000->0500, block[0] 180:      // This is the system of coordinates for ...
        0000->1500, block[1] 158:  // ... the blocks of the first cube.
               op1                 // Here we must define what has to happen ...
                                   // ... inside the block.
    0000->0500, block[0] 180:
        1500->4000, block[1] 158:
               op2

    0500->3000, block[0] 180:
        0000->1500, block[1] 158:
               op3

    0500->3000, block[0] 180:
        1500->4000, block[1] 158:
               op4

    3000->4000, block[0] 180:
        0000->1000, block[1] 158:
               op5

    3000->4000, block[0] 180:
        1000->4000, block[1] 158:
               op6

    At the leafs of this blocking-tree we now add the contents of the blocks:

    0000->0500, block[0] 180:
        0000->1500, block[1] 158:
               0->180, step[0] 1
                            0->1: 0->158, step[1] 1
                                               0->1: e1

    0000->0500, block[0] 180:
        1500->4000, block[1] 158:
               0->180, step[0] 9
                            0->2: 0->158, step[1] 1
                                               0->1: e2
                            2->9: 0->158, step[1] 1
                                               0->1: e1

    0500->3000, block[0] 180:
        0000->1500, block[1] 158:
               0->180, step[0] 1
                            0->1: 0->158, step[1] 1
                                               0->1: e2

    0500->3000, block[0] 180:
        1500->4000, block[1] 158:
               0->180, step[0] 9
                            0->4: 0->158, step[1] 1
                                               0->1: e1
                            4->6: 0->158, step[1] 1
                                               0->1: e2
                            6->9: 0->158, step[1] 1
                                               0->1: e1

    3000->4000, block[0] 180:
        0000->1000, block[1] 158:
               0->180, step[0] 1
                            0->1: 0->158, step[1] 1
                                               0->1: e3

    3000->4000, block[0] 180:
        1000->4000, block[1] 158:
               0->180, step[0] 1
                            0->1: 0->158, step[1] 3
                                               0->1: e3
                                               1->3: e4

  This scheme can be expanded analogeous for hierarchical blocking with any
  number of levels.

  To suppress blocking in all or just some dimensions it is not adequate, at
  least in case of (sv_d > 1), to insert a blocking-stride with blocking-step 1.
  If doing so, possibly existing grids would not fit in the relevant branch.
  It is rather necessary to omit the block[d]-node and just insert the relevant
  step[d]-branch directly. The blocking hierarchie for dimension (d+1) follows
  as usual.

  In the example with bv = (1,158):

    0000->0500, step[0] 1
                     0->1:    0->1500, block[1] 158:
                                             0->158, step[1] 1
                                                          0->1: e1
    0000->0500, step[0] 9
                     0->2: 1500->4000, block[1] 158:
                                             0->158, step[1] 1
                                                          0->1: e2
                     2->9: 1500->4000, block[1] 158:
                                             0->158, step[1] 1
                                                          0->1: e1
    0500->3000, step[0] 1
                     0->1:    0->1500, block[1] 158:
                                             0->158, step[1] 1
                                                          0->1: e2
    0500->3000, step[0] 9
                     0->4: 1500->4000, block[1] 158:
                                             0->158, step[1] 1
                                                          0->1: e1
                     4->6: 1500->4000, block[1] 158:
                                             0->158, step[1] 1
                                                          0->1: e2
                     6->9: 1500->4000, block[1] 158:
                                             0->158, step[1] 1
                                                          0->1: e1
    3000->4000, step[0] 1
                     0->1:    0->1000, block[1] 158:
                                             0->158, step[1] 1
                                                          0->1: e3
    3000->4000, step[0] 1
                     0->1: 1000->4000, block[1] 158:
                                             0->158, step[1] 3
                                                          0->1: e3
                                                          1->3: e4

  Using this method blocking (bv_d > 1) and non-blocking (bv_d = 1) can
  be used alternately for all dimensions and levels (hierarchical blocking).
  With the chosen tree representation the following transformation steps
  6.) to 9.) can be described and performed in an (quite) easy and clear way:
  It is only necessary to compare nodes and there subtrees; possibly some
  subtrees have to be copied, erased oder moved, but these operations have
  to be done on local scope only, irrespective of the concrete position in the
  tree (-> context free). This is hold even for the final code generation phase.

  Using the original values (to get somewhat pathological results) we get
  for the example with bv = (180,158):

    000->050, block[0] 180:
        000->150, block[1] 158:
              0->180, step[0] 1
                           0->1: 0->158, step[1] 1
                                              0->1: e1
    000->050, block[0] 180:
        150->400, block[1] 158:
              0->180, step[0] 9
                           0->2: 0->158, step[1] 1
                                              0->1: e2
                           2->9: 0->158, step[1] 1
                                              0->1: e1
    050->300, block[0] 180:
        000->150, block[1] 158:
              0->180, step[0] 1
                           0->1: 0->158, step[1] 1
                                              0->1: e2
    050->300, block[0] 180:
        150->400, block[1] 158:
              0->180, step[0] 9
                           0->4: 0->158, step[1] 1
                                              0->1: e1
                           4->6: 0->158, step[1] 1
                                              0->1: e2
                           6->9: 0->158, step[1] 1
                                              0->1: e1
    300->400, block[0] 180:
        000->100, block[1] 158:
              0->180, step[0] 1
                           0->1: 0->158, step[1] 1
                                              0->1: e3
    300->400, block[0] 180:
        100->400, block[1] 158:
              0->180, step[0] 1
                           0->1: 0->158, step[1] 3
                                              0->1: e3
                                              1->3: e4

  Note: instead of transforming to
          0->50, block[0] 50: ...
        at first the original values are preserved because these are needed
        for the tree optimization (see step 7.) !!
        However, the corrected blocking sizes are needed not until the
        fitting (see step 9.).

  With bv = (1,158) --- because of ubv = (1,6) this is *not* equivalent to (1,1)
  --- we would get:

      0-> 50, step[0] 1
                   0->1:   0->150, block[1] 158:
                                         0->158, step[1] 1
                                                      0->1: e1
      0-> 50, step[0] 9
                   0->2: 150->400, block[1] 158:
                                         0->158, step[1] 1
                                                      0->1: e2
                   2->9: 150->400, block[1] 158:
                                         0->158, step[1] 1
                                                      0->1: e1
     50->300, step[0] 1
                   0->1:   0->150, block[1] 158:
                                         0->158, step[1] 1
                                                      0->1: e2
     50->300, step[0] 9
                   0->4: 150->400, block[1] 158:
                                         0->158, step[1] 1
                                                      0->1: e1
                   4->6: 150->400, block[1] 158:
                                         0->158, step[1] 1
                                                      0->1: e2
                   6->9: 150->400, block[1] 158:
                                         0->158, step[1] 1
                                                      0->1: e1
    300->400, step[0] 1
                   0->1:   0->100, block[1] 158:
                                         0->158, step[1] 1
                                                      0->1: e3
    300->400, step[0] 1
                   0->1: 100->400, block[1] 158:
                                         0->158, step[1] 3
                                                      0->1: e3
                                                      1->3: e4


5.) Unrolling-Blocking (without fitting) according to the values in ubv
    ------------------

    On each block an additional blocking for each dimension with (ubv_d > 1)
    is performed.
    This blocking differs from the conventional hierarchical blocking in
    the sense, that possibly a fitting is carried out (see step 8.)

  In the example with bv = (180,158):

    000->050, block[0] 180:
        000->150, block[1] 158:
              0->180, step[0] 1
                           0->1: 0->158, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
    000->050, block[0] 180:
        150->400, block[1] 158:
              0->180, step[0] 9
                           0->2: 0->158, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           2->9: 0->158, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
    050->300, block[0] 180:
        000->150, block[1] 158:
              0->180, step[0] 1
                           0->1: 0->158, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
    050->300, block[0] 180:
        150->400, block[1] 158:
              0->180, step[0] 9
                           0->4: 0->158, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
                           4->6: 0->158, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           6->9: 0->158, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
    300->400, block[0] 180:
        000->100, block[1] 158:
              0->180, step[0] 1
                           0->1: 0->158, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e3
    300->400, block[0] 180:
        100->400, block[1] 158:
              0->180, step[0] 1
                           0->1: 0->158, ublock[1] 6:
                                                0->6, step[1] 3
                                                           0->1: e3
                                                           1->3: e4

  In the example with bv = (1,158):

      0-> 50, step[0] 1
                   0->1:   0->150, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
      0-> 50, step[0] 9
                   0->2: 150->400, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                   2->9: 150->400, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
     50->300, step[0] 1
                   0->1:   0->150, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
     50->300, step[0] 9
                   0->4: 150->400, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                   4->6: 150->400, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                   6->9: 150->400, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
    300->400, step[0] 1
                   0->1:   0->100, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e3
    300->400, step[0] 1
                   0->1: 100->400, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 3
                                                                   0->1: e3
                                                                   1->3: e4


6.) Cube Merging (Makes cubes with identical subtrees compatible and joins them)
    ------------

    -> The tree forms in each dimension a partition of the relevant
       index-vector-set projection.

  In the example with bv = (180,158):

    000->050, block[0] 180:
        000->150, block[1] 158:
              0->180, step[0] 1
                           0->1: 0->158, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
        150->400, block[1] 158:
              0->180, step[0] 9
                           0->2: 0->158, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           2->9: 0->158, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
    050->300, block[0] 180:
        000->150, block[1] 158:
              0->180, step[0] 1
                           0->1: 0->158, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
        150->400, block[1] 158:
              0->180, step[0] 9
                           0->4: 0->158, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
                           4->6: 0->158, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           6->9: 0->158, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
    300->400, block[0] 180:
        000->100, block[1] 158:
              0->180, step[0] 1
                           0->1: 0->158, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e3
        100->400, block[1] 158:
              0->180, step[0] 1
                           0->1: 0->158, ublock[1] 6:
                                                0->6, step[1] 3
                                                           0->1: e3
                                                           1->3: e4

  In the example with bv = (1,158):

      0-> 50, step[0] 9
                   0->2:   0->150, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         150->400, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                   2->9:   0->150, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         150->400, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
     50->300, step[0] 9
                   0->4:   0->150, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->400, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                   4->6:   0->150, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->400, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                   6->9:   0->150, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->400, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
    300->400, step[0] 1
                   0->1:   0->100, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e3
                         100->400, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 3
                                                                   0->1: e3
                                                                   1->3: e4


7.) Tree Optimization (Joins identical subtrees)
    -----------------

    Projections with consecutive index ranges and identical operations
    (subtrees) are joined.

  In the example with bv = (1,158):

      0-> 50, step[0] 9
               ...
                   2->9:   0->150, block[1] 158: ... tree_1 ...

                         150->400, block[1] 158: ... tree_1 ...
                        Fortunately we still ^ have the same values overhere!

    is transformed to

      0-> 50, step[0] 9
               ...
                   2->9:   0->400, block[1] 158: ... tree_1 ...
                        Now this value makes ^ sense again!

  Altogether:

      0-> 50, step[0] 9
                   0->2:   0->150, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         150->400, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                   2->9:   0->400, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
     50->300, step[0] 9
                   0->4:   0->150, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->400, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                   4->6:   0->400, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                   6->9:   0->150, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->400, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
    300->400, step[0] 1
                   0->1:   0->100, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e3
                         100->400, block[1] 158:
                                         0->158, ublock[1] 6:
                                                        0->6, step[1] 3
                                                                   0->1: e3
                                                                   1->3: e4


8.) Projection Fitting (Removes incomplete periods at the tail and adjusts ...
    ------------------                                   ... block sizes)
    (^ after the optimization we have in general no cubes anymore ...)

    In each subtree and each dimension the boundaries of the most outer node
    as well as each block size are adjusted to the number of unrolled elements
    (= max( ubv_d, step)).
    The adjustment of the block size is crucial to preserve the periodicity
    of the block content!!!

  In the example with bv = (180,158):

    000->045, block[0] 180:      // bounds: (050 - 000 = 50) % (step[0] = 9) = 5
        000->150, block[1] 156:  // block size: 158 % (ublock[1] = 6)  =  2
              0->180, step[0] 1
                           0->1: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
        150->396, block[1] 156:
              0->180, step[0] 9
                           0->2: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           2->9: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
        396->400, block[1] 156:
              0->180, step[0] 9
                           0->2: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           2->9: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
    045->050, block[0] 180:
        000->150, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->150, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
        150->396, block[1] 156:
              0->180, step[0] 5
                           0->2: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           2->5: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
        396->400, block[1] 156:
              0->180, step[0] 5
                           0->2: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           2->5: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
    050->293, block[0] 180:
        000->150, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
        150->396, block[1] 156:
              0->180, step[0] 9
                           0->4: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
                           4->6: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           6->9: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
        396->400, block[1] 156:
              0->180, step[0] 9
                           0->4: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
                           4->6: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           6->9: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
    293->300, block[0] 180:
        000->150, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
        150->396, block[1] 156:
              0->180, step[0] 7
                           0->4: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
                           4->6: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           6->7: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
        396->400, block[1] 156:
              0->180, step[0] 7
                           0->4: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
                           4->6: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           6->7: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
    300->400, block[0] 180:
        000-> 96, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e3
        096->100, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e3
        100->400, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, ublock[1] 6:
                                                0->6, step[1] 3
                                                           0->1: e3
                                                           1->3: e4

  In the example with bv = (1,158):

      0-> 45, step[0] 9
                   0->2:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         150->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         396->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                   2->9:   0->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         396->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
     45-> 50, step[0] 5
                   0->2:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         150->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         396->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                   2->5:   0->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         396->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
     50->293, step[0] 9
                   0->4:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         396->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                   4->6:   0->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         396->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                   6->9:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         396->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
    293->300, step[0] 7
                   0->4:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         396->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                   4->6:   0->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         396->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                   6->7:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         396->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
    300->400, step[0] 1
                   0->1:   0-> 96, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e3
                          96->100, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e3
                         100->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 3
                                                                   0->1: e3
                                                                   1->3: e4


9.) Adjust block sizes to real projection sizes
    -------------------------------------------

  In the example bv = (180,158):

    000->045, block[0]  45:
        000->150, block[1] 150:
              0-> 45, step[0] 1
                           0->1: 0->150, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
        150->396, block[1] 156:
              0-> 45, step[0] 9
                           0->2: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           2->9: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
        396->400, block[1]   4:
              0-> 45, step[0] 9
                           0->2: 0->  4, ublock[1] 4:
                                                0->4, step[1] 1
                                                           0->1: e2
                           2->9: 0->  4, ublock[1] 4:
                                                0->4, step[1] 1
                                                           0->1: e1
    045->050, block[0]   5:
        000->150, block[1] 150:
              0->  5, step[0] 1
                           0->1: 0->150, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
        150->396, block[1] 156:
              0->  5, step[0] 5
                           0->2: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           2->5: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
        396->400, block[1]   4:
              0->  5, step[0] 5
                           0->2: 0->  4, ublock[1] 4:
                                                0->4, step[1] 1
                                                           0->1: e2
                           2->5: 0->  4, ublock[1] 4:
                                                0->4, step[1] 1
                                                           0->1: e1
    050->293, block[0] 180:
        000->150, block[1] 150:
              0->180, step[0] 1
                           0->1: 0->150, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
        150->396, block[1] 156:
              0->180, step[0] 9
                           0->4: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
                           4->6: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           6->9: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
        396->400, block[1]   4:
              0->180, step[0] 9
                           0->4: 0->  4, ublock[1] 4:
                                                0->4, step[1] 1
                                                           0->1: e1
                           4->6: 0->  4, ublock[1] 4:
                                                0->4, step[1] 1
                                                           0->1: e2
                           6->9: 0->  4, ublock[1] 4:
                                                0->4, step[1] 1
                                                           0->1: e1
    293->300, block[0]   7:
        000->150, block[1] 150:
              0->  7, step[0] 1
                           0->1: 0->150, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
        150->396, block[1] 156:
              0->  7, step[0] 7
                           0->4: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
                           4->6: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           6->7: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
        396->400, block[1]   4:
              0->  7, step[0] 7
                           0->4: 0->  4, ublock[1] 4:
                                                0->4, step[1] 1
                                                           0->1: e1
                           4->6: 0->  4, ublock[1] 4:
                                                0->4, step[1] 1
                                                           0->1: e2
                           6->7: 0->  4, ublock[1] 4:
                                                0->4, step[1] 1
                                                           0->1: e1
    300->400, block[0] 100:
        000-> 96, block[1]  96:
              0->100, step[0] 1
                           0->1: 0-> 96, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e3
        096->100, block[1]   4:
              0->100, step[0] 1
                           0->1: 0->  4, ublock[1] 4:
                                                0->4, step[1] 1
                                                           0->1: e3
        100->400, block[1] 156:
              0->100, step[0] 1
                           0->1: 0->156, ublock[1] 6:
                                                0->6, step[1] 3
                                                           0->1: e3
                                                           1->3: e4

  In the example with bv = (1,158):

      0-> 45, step[0] 9
                   0->2:   0->150, block[1] 150:
                                         0->150, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         150->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         396->400, block[1]   4:
                                         0->  4, ublock[1] 4:
                                                        0->4, step[1] 1
                                                                   0->1: e2
                   2->9:   0->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         396->400, block[1]   4:
                                         0->  4, ublock[1] 4:
                                                        0->4, step[1] 1
                                                                   0->1: e1
     45-> 50, step[0] 5
                   0->2:   0->150, block[1] 150:
                                         0->150, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         150->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         396->400, block[1]   4:
                                         0->  4, ublock[1] 4:
                                                        0->4, step[1] 1
                                                                   0->1: e2
                   2->5:   0->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         396->400, block[1]   4:
                                         0->  4, ublock[1] 4:
                                                        0->4, step[1] 1
                                                                   0->1: e1
     50->293, step[0] 9
                   0->4:   0->150, block[1] 150:
                                         0->150, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         396->400, block[1]   4:
                                         0->  4, ublock[1] 4:
                                                        0->4, step[1] 1
                                                                   0->1: e1
                   4->6:   0->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         396->400, block[1]   4:
                                         0->  4, ublock[1] 4:
                                                        0->4, step[1] 1
                                                                   0->1: e2
                   6->9:   0->150, block[1] 150:
                                         0->150, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         396->400, block[1]   4:
                                         0->  4, ublock[1] 4:
                                                        0->4, step[1] 1
                                                                   0->1: e1
    293->300, step[0] 7
                   0->4:   0->150, block[1] 150:
                                         0->150, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         396->400, block[1]   4:
                                         0->  4, ublock[1] 4:
                                                        0->4, step[1] 1
                                                                   0->1: e1
                   4->6:   0->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         396->400, block[1]   4:
                                         0->  4, ublock[1] 4:
                                                        0->4, step[1] 1
                                                                   0->1: e2
                   6->7:   0->150, block[1] 150:
                                         0->150, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         396->400, block[1]   4:
                                         0->  4, ublock[1] 4:
                                                        0->4, step[1] 1
                                                                   0->1: e1
    300->400, step[0] 1
                   0->1:   0-> 96, block[1]  96:
                                         0-> 96, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e3
                          96->100, block[1]   4:
                                         0->  4, ublock[1] 4:
                                                        0->4, step[1] 1
                                                                   0->1: e3
                         100->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 3
                                                                   0->1: e3
                                                                   1->3: e4



Internal representation in the abstract syntax tree:
====================================================


  Most of the nodes have an attribute 'level'. This attribute indicates how
  many ancestors in the tree concern the dimension 'dim', too.


  node type:   attribute (type of the attribute)
  ----------------------------------------------

    WLseg:     contents  (WLblock, WLublock, WLstride)
               next      (WLseg)


    WLblock:   level     (int)
               dim       (int)
               bound1    (int)
               bound2    (int)
               step      (int)                // block size
               nextdim   (WLblock)            // blocking-node of next dim
               contents  (WLublock, WLstride) // op. of interior of block
               next      (WLblock)            // next blocking this dim


    WLublock:  level     (int)
               dim       (int)
               bound1    (int)
               bound2    (int)
               step      (int)
               nextdim   (WLublock)           // only one of this ...
               contents  (WLstride)           // ... nodes is != NULL
               next      (WLublock)


    WLstride:  level     (int)
               dim       (int)
               bound1    (int)
               bound2    (int)
               step      (int)
               unrolling (bool)      // is unrolling wanted? ...
                                     // ... e. g. because of a WLublock-node
               contents  (WLgrid)    // description of the inner grids
               next      (WLstride)  // next stride this dim


    WLgrid:    dim       (int)
               bound1    (int)                         // == offset
               bound2    (int)                         // == offset + width
               unrolling (bool)
               nextdim   (WLblock, WLublock, WLstride) // only one of this ..
               code      (WLcode)                      // .. nodes is != NULL
               next      (WLgrid)


    WLcode:    cblock    (block)
               cexpr     ("expr")


*****************************************************************************/

/*
 * these macros are used in 'CompareWLnode' for compare purpose
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

/* forward declaration */
static int CompareWLnode (node *node1, node *node2, int outline);

/**
 ** external functions
 **/

/******************************************************************************
 *
 * Function:
 *   bool AllStridesAreConstant( node *strides,
 *                               bool trav_cont, bool trav_nextdim)
 *
 * Description:
 *   Infers, whether all the strides of the given chain are constant.
 *
 ******************************************************************************/

bool
AllStridesAreConstant (node *wlnode, bool trav_cont, bool trav_nextdim)
{
    bool all_const = TRUE;

    DBUG_ENTER ("AllStridesAreConstant");

    if (wlnode != NULL) {
        switch (NODE_TYPE (wlnode)) {
        case N_WLstride:
            if (WLSTRIDE_BOUND2 (wlnode) >= 0) {
                all_const = (((!trav_cont)
                              || AllStridesAreConstant (WLSTRIDE_CONTENTS (wlnode),
                                                        trav_cont, trav_nextdim))
                             && AllStridesAreConstant (WLSTRIDE_NEXT (wlnode), trav_cont,
                                                       trav_nextdim));
            } else {
                DBUG_ASSERT ((WLSTRIDE_BOUND2 (wlnode) == IDX_SHAPE),
                             "illegal WLSTRIDE_BOUND2 found!");
                all_const = FALSE;
            }
            break;

        case N_WLgrid:
            all_const = (((!trav_nextdim)
                          || AllStridesAreConstant (WLGRID_NEXTDIM (wlnode), trav_cont,
                                                    trav_nextdim))
                         && AllStridesAreConstant (WLGRID_NEXT (wlnode), trav_cont,
                                                   trav_nextdim));
            break;

        case N_WLstrideVar:
            /* here is no break missing! */
        case N_WLgridVar:
            all_const = FALSE;
            break;

        default:
            DBUG_ASSERT ((0), "illegal stride/grid node found!");
            break;
        }
    }

    DBUG_RETURN (all_const);
}

/******************************************************************************
 *
 * Function:
 *   node *InsertWLnodes( node *nodes, node *insert_nodes)
 *
 * Description:
 *   inserts all elements of the chain 'insert_nodes' into the sorted chain
 *     'nodes'.
 *   all elements of 'insert_nodes' that exist already in 'nodes' are freed.
 *   the function 'CompareWLnode' is used to sort the elements.
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
 * Function:
 *   int GridOffset( int new_bound1,
 *                   int bound1, int step, int grid_b2)
 *
 * Description:
 *   computes a offset for a grid relating to 'new_bound1':
 *     what happens to the bounds of a grid if 'new_bound1' is the new
 *     upper bound for the accessory stride?
 *
 *   if (offset <= grid_b1) the new bounds of the grid are
 *       "(grid_b1 - offset) -> (grid_b2 - offset)"
 *   if (offset > grid_b1) the grid must be devided in two parts:
 *       "(grid_b1 - offset + step) -> step" and
 *       "0 -> (grid_b2 - offset)" !!
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

/**
 ** static functions
 **/

/******************************************************************************
 ******************************************************************************
 **
 **  general purpose functions
 **
 **/

/******************************************************************************
 *
 * Function:
 *   int CompareWLnode( node *node1, node *node2, int outline)
 *
 * Description:
 *   this function defines the sort order for InsertWLnodes:
 *   compares the N_WL...-nodes 'node1' and 'node2' IN ALL DIMS.
 *   possibly present next nodes in 'node1' or 'node2' are ignored.
 *
 *   if (outline > 0) ALL GRID DATA IS IGNORED!!!
 *   (this feature is used by 'ComputeCubes', to determine whether two strides
 *    lie in the same cube or not)
 *
 *   return: -2 => outline('node1') < outline('node2')
 *           -1 => outline('node1') = outline('node2'), 'node1' < 'node2'
 *            0 => 'node1' = 'node2'
 *            1 => outline('node1') = outline('node2'), 'node1' > 'node2'
 *            2 => outline('node1') > outline('node2')
 *
 ******************************************************************************/

static int
CompareWLnode (node *node1, node *node2, int outline)
{
    node *grid1, *grid2;
    int result, grid_result;

    DBUG_ENTER ("CompareWLnode");

    if ((node1 == NULL) || (node2 == NULL)) {
        if ((node1 == NULL) && (node2 == NULL)) {
            result = 0;
        } else {
            result = (node2 == NULL) ? 2 : (-2);
        }
    } else {
        DBUG_ASSERT ((NODE_TYPE (node1) == NODE_TYPE (node2)),
                     "can not compare objects of different type");

        if ((NODE_TYPE (node1) == N_WLstrideVar) || (NODE_TYPE (node1) == N_WLgridVar)) {
            /*
             * we can not compare var. strides or grids
             */
            result = 2;
        } else {
            /* compare the bounds first */
            COMP_BEGIN (WLNODE_BOUND1 (node1), WLNODE_BOUND1 (node2), result, 2)
            COMP_BEGIN (WLNODE_BOUND2 (node1), WLNODE_BOUND2 (node2), result, 2)

            switch (NODE_TYPE (node1)) {
            case N_WLblock:
                /* here is no break missing! */
            case N_WLublock:
                /* compare next dim */
                result = CompareWLnode (WLXBLOCK_NEXTDIM (node1),
                                        WLXBLOCK_NEXTDIM (node2), outline);
                break;

            case N_WLstride:
                grid1 = WLSTRIDE_CONTENTS (node1);
                DBUG_ASSERT ((grid1 != NULL), "no grid1 for comparison found");
                grid2 = WLSTRIDE_CONTENTS (node2);
                DBUG_ASSERT ((grid2 != NULL), "no grid2 for comparison found");

                if (outline) {
                    /* compare outlines only -> skip grid */
                    result = CompareWLnode (WLGRID_NEXTDIM (grid1),
                                            WLGRID_NEXTDIM (grid2), outline);
                } else {
                    /*
                     * compare grid, but leave 'result' untouched
                     *   until later dimensions are checked!
                     */
                    COMP_BEGIN (WLGRID_BOUND1 (grid1), WLGRID_BOUND1 (grid2), grid_result,
                                1)
                    COMP_BEGIN (WLGRID_BOUND2 (grid1), WLGRID_BOUND2 (grid2), grid_result,
                                1)
                    grid_result = 0;
                    COMP_END
                    COMP_END

                    /* compare later dimensions */
                    result = CompareWLnode (WLGRID_NEXTDIM (grid1),
                                            WLGRID_NEXTDIM (grid2), outline);

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
                result = CompareWLnode (WLGRID_NEXTDIM (node1), WLGRID_NEXTDIM (node2),
                                        outline);
                break;

            default:
                result = 0;
                DBUG_ASSERT ((0), "wrong node type");
                break;
            }

            COMP_END
            COMP_END
        }
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * Function:
 *   void CheckStride( int bound1, int bound2, int step,
 *                     int grid_b1, int grid_b2,
 *                     bool should_be_non_empty)
 *
 * Description:
 *   Assures that the given stride parameters are legal.
 *   The parameter 'should_be_non_empty' triggers whether empty strides are
 *   allowed or not.
 *
 ******************************************************************************/

static void
CheckStride (int bound1, int bound2, int step, int grid_b1, int grid_b2,
             bool should_be_non_empty)
{
    DBUG_ENTER ("CheckStride");

    DBUG_ASSERT ((0 <= bound1), "given stride has illegal lower bound (<= 0)");
    if (should_be_non_empty) {
        DBUG_ASSERT ((bound1 < bound2),
                     "given stride is empty (lower bound >= upper bound)!");
    }
    DBUG_ASSERT ((0 < step), "given step is illegal (<= 0)");
    DBUG_ASSERT ((0 <= grid_b1), "given grid has illegal lower bound (<= 0)");
    DBUG_ASSERT ((grid_b1 < grid_b2),
                 "given grid is empty (lower bound >= upper bound)!");
    DBUG_ASSERT ((grid_b2 <= step), "given grid has illegal upper bound (> step)!");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   node *NormalizeStride_1( node *stride)
 *
 * Description:
 *   Normalizes the given stride in the FIRST dimension and returns it.
 *   'stride' has to be non-empty!!
 *
 *   This normalization has two major goals:
 *     * every stride has a unambiguous form
 *        -> two strides represent the same index set
 *           if and only if there attribute values are equal.
 *     * maximize the outline of strides
 *        -> two strides of the same cube do not split each other in several
 *           parts when intersected
 *
 *   A stride is normalized, iff
 *     * (step < bound2 - bound1 - grid_b1)  or  (step == 1),
 *     * (step < grid_b2 - grid_b1)  or  (step == 1),
 *     * grid_b2 <= step,
 *     * bound1 minimized,
 *     * bound2 maximized.
 *
 ******************************************************************************/

static node *
NormalizeStride_1 (node *stride)
{
    node *grid;
    int bound1, bound2, step, grid_b1, grid_b2, new_bound1, new_bound2;

    DBUG_ENTER ("NormalizeStride_1");

    DBUG_ASSERT ((NODE_TYPE (stride) == N_WLstride),
                 "given node is not a constant stride!");

    grid = WLSTRIDE_CONTENTS (stride);
    DBUG_ASSERT ((grid != NULL), "given stride contains no grid!");
    /*
     * For the time being support for multiple grids is not needed
     * and therefore not implemented yet ...
     */
    DBUG_ASSERT ((WLGRID_NEXT (grid) == NULL),
                 "given stride contains more than one grid!");

    bound1 = WLSTRIDE_BOUND1 (stride);
    bound2 = WLSTRIDE_BOUND2 (stride);
    step = WLSTRIDE_STEP (stride);
    grid_b1 = WLGRID_BOUND1 (grid);
    grid_b2 = WLGRID_BOUND2 (grid);

    /*
     * assure, that the stride is legal!
     */
    CheckStride (bound1, bound2, step, grid_b1, grid_b2, TRUE);

    /*
     * if (bound2 - bound1 - grid_b1 <= step), we can set (step = 1)
     */
    if (bound2 - bound1 - grid_b1 <= step) {
        bound2 = MIN (bound2, bound1 + grid_b2);
        bound1 += grid_b1;
        grid_b1 = 0;
        grid_b2 = 1;
        step = 1;
    }

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
     * assure, that the stride is still legal!
     */
    CheckStride (bound1, bound2, step, grid_b1, grid_b2, TRUE);

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
 * Function:
 *   node *NormalizeAllStrides( node *strides)
 *
 * Description:
 *   Normalizes all the given strides in all dimensions.
 *
 ******************************************************************************/

static node *
NormalizeAllStrides (node *strides)
{
    node *grid;

    DBUG_ENTER ("NormalizeAllStrides");

    if (strides != NULL) {
        if (NODE_TYPE (strides) == N_WLstride) {
            strides = NormalizeStride_1 (strides);
            grid = WLSTRIDE_CONTENTS (strides);
            DBUG_ASSERT ((WLGRID_NEXT (grid) == NULL), "multiple grids found!");
            WLGRID_NEXTDIM (grid) = NormalizeAllStrides (WLGRID_NEXTDIM (grid));
            WLSTRIDE_NEXT (strides) = NormalizeAllStrides (WLSTRIDE_NEXT (strides));
        } else {
            DBUG_ASSERT ((NODE_TYPE (strides) == N_WLstrideVar), "illegal stride found!");
        }
    }

    DBUG_RETURN (strides);
}

/******************************************************************************
 *
 * Function:
 *   node *NormalizeGrids( node *stride)
 *
 * Description:
 *   If the grids contained in the given stride have an offset, remove it:
 *
 *       5 -> 15 step 5                  7 -> 15 step 5
 *               2 -> 3: ...     ==>             0 -> 1: ...
 *               4 -> 5: ...                     2 -> 3: ...
 *
 ******************************************************************************/

node *
NormalizeGrids (node *stride)
{
    node *grids;
    int offset;

    DBUG_ENTER ("NormalizeGrids");

    DBUG_ASSERT (((NODE_TYPE (stride) == N_WLstride)
                  || (NODE_TYPE (stride) == N_WLstrideVar)),
                 "illegal stride found!");

    if (NODE_TYPE (stride) == N_WLstride) {
        grids = WLSTRIDE_CONTENTS (stride);
        DBUG_ASSERT ((grids != NULL), "no grid found");
        offset = WLGRID_BOUND1 (grids);

        if (offset > 0) {
            WLSTRIDE_BOUND1 (stride) += offset;
            do {
                DBUG_ASSERT ((NODE_TYPE (grids) == N_WLgrid), "var. grid found!");

                WLGRID_BOUND1 (grids) -= offset;
                WLGRID_BOUND2 (grids) -= offset;
                grids = WLGRID_NEXT (grids);
            } while (grids != NULL);
        }
    }

    DBUG_RETURN (stride);
}

/******************************************************************************
 *
 * Function:
 *   int IndexHeadStride( node *stride)
 *
 * Description:
 *   returns the index position of the first element of 'stride'.
 *
 * Remark:
 *   the grids of the stride must be sorted in ascending order with respect
 *   to their lower bounds, because this routine examines only the *first*
 *   grid!
 *
 ******************************************************************************/

static int
IndexHeadStride (node *stride)
{
    int result;
    int bound1, bound2;

    DBUG_ENTER ("IndexHeadStride");

    DBUG_ASSERT ((NODE_TYPE (stride) == N_WLstride),
                 "given node is not a constant stride!");

    bound1 = WLSTRIDE_BOUND1 (stride);
    bound2 = WLSTRIDE_BOUND2 (stride);
    DBUG_ASSERT ((bound1 < bound2),
                 "given stride is empty (lower bound >= upper bound)!");

    result = bound1 + WLGRID_BOUND1 (WLSTRIDE_CONTENTS (stride));

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * Function:
 *   int IndexRearStride( node *stride)
 *
 * Description:
 *   returns the index position '+1' of the last element of 'stride'
 *
 ******************************************************************************/

static int
IndexRearStride (node *stride)
{
    node *grid;
    int bound1, bound2, grid_b1, result;

    DBUG_ENTER ("IndexRearStride");

    DBUG_ASSERT ((NODE_TYPE (stride) == N_WLstride),
                 "given node is not a constant stride!");

    bound1 = WLSTRIDE_BOUND1 (stride);
    bound2 = WLSTRIDE_BOUND2 (stride);
    DBUG_ASSERT ((bound1 < bound2),
                 "given stride is empty (lower bound >= upper bound)!");

    grid = WLSTRIDE_CONTENTS (stride);
    DBUG_ASSERT ((NODE_TYPE (grid) == N_WLgrid), "given stride contains no grid!");

    grid_b1 = WLGRID_BOUND1 (grid);

    /* search last grid (there will we find the last element!) */
    while (WLGRID_NEXT (grid) != NULL) {
        grid = WLGRID_NEXT (grid);
    }

    result = bound2
             - MAX (0, ((bound2 - bound1 - grid_b1 - 1) % WLSTRIDE_STEP (stride)) + 1
                         - (WLGRID_BOUND2 (grid) - grid_b1));

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * Function:
 *   int GetLcmUnroll( node *nodes, int dim, bool include_blocks)
 *
 * Description:
 *   Returns the maximal number of elements that must be unrolled in dimension
 *   'dim' of the WL-tree 'nodes'.
 *   We must search for the first N_WLublock- or N_WLstride-node in each leaf
 *   of the 'nodes'-tree and calculate the lcm of their steps.
 *
 *   'include_blocks' denotes whether N_WLblock nodes are considered to be
 *   undivisible or not.
 *   For the time being the backend (WLSEG_SV -> multi-threading) needs
 *   (include_blocks = TRUE) !!!
 *
 ******************************************************************************/

static int
GetLcmUnroll (node *nodes, int dim, bool include_blocks)
{
    int unroll = 1;

    DBUG_ENTER ("GetLcmUnroll");

    if (nodes != NULL) {
        unroll = GetLcmUnroll (WLNODE_NEXT (nodes), dim, include_blocks);

        if ((WLNODE_DIM (nodes) == dim)
            && (((NODE_TYPE (nodes) == N_WLblock) && (include_blocks))
                || (NODE_TYPE (nodes) == N_WLublock)
                || (NODE_TYPE (nodes) == N_WLstride))) {
            /*
             * we have found a node with unrolling information
             */
            unroll = lcm (unroll, WLNODE_STEP (nodes));
        } else {
            /*
             * search in whole tree for nodes with unrolling information
             */
            switch (NODE_TYPE (nodes)) {
            case N_WLblock:
                /* here is no break missing! */
            case N_WLublock:
                unroll = lcm (unroll, GetLcmUnroll (WLXBLOCK_NEXTDIM (nodes), dim,
                                                    include_blocks));
                unroll = lcm (unroll, GetLcmUnroll (WLXBLOCK_CONTENTS (nodes), dim,
                                                    include_blocks));
                break;

            case N_WLstride:
                unroll = lcm (unroll, GetLcmUnroll (WLSTRIDE_CONTENTS (nodes), dim,
                                                    include_blocks));
                break;

            case N_WLgrid:
                unroll = lcm (unroll,
                              GetLcmUnroll (WLGRID_NEXTDIM (nodes), dim, include_blocks));
                break;

            default:
                DBUG_ASSERT ((0), "wrong node type");
                break;
            }
        }
    }

    DBUG_RETURN (unroll);
}

/******************************************************************************
 *
 * Function:
 *   node *GenerateShapeStrides( int dim, int dims, shpseg* shape)
 *
 * Description:
 *   Returns strides/grids of the size found in 'shape'.
 *
 ******************************************************************************/

static node *
GenerateShapeStrides (int dim, int dims, shpseg *shape)
{
    node *new_grid;
    node *strides = NULL;

    DBUG_ENTER ("GenerateShapeStrides");

    if (dim < dims) {
        new_grid = MakeWLgrid (0, dim, 0, 1, FALSE,
                               GenerateShapeStrides (dim + 1, dims, shape), NULL, NULL);

        strides = MakeWLstride (0, dim, 0, GET_SHAPE_IDX (SHPSEG_ELEMS (shape), dim), 1,
                                FALSE, new_grid, NULL);
    }

    DBUG_RETURN (strides);
}

/**
 **
 **  general purpose functions
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 ******************************************************************************
 **
 **  functions for NodeOrInt, NameOrVal
 **
 **/

/******************************************************************************
 *
 * Function:
 *   node *GenerateNodeForGap( node *wlnode)
 *                             nodetype nt1, void *pnode1,
 *                             nodetype nt2, void *pnode2,
 *                             bool is_noop)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
GenerateNodeForGap (node *wlnode, nodetype nt1, void *pnode1, nodetype nt2, void *pnode2,
                    bool is_noop)
{
    int val1, val2;
    bool is_const;
    node *gap_node = NULL;

    DBUG_ENTER ("GenerateNodeForGap");

    DBUG_ASSERT ((wlnode != NULL), "no WL node found!");

    if (!NodeOrInt_Eq (nt1, pnode1, nt2, pnode2, IDX_SHAPE)) {
        NodeOrInt_GetNameOrVal (NULL, &val1, nt1, pnode1);
        NodeOrInt_GetNameOrVal (NULL, &val2, nt2, pnode2);
        is_const = ((val1 != IDX_OTHER) && (val2 != IDX_OTHER));

        switch (NODE_TYPE (wlnode)) {
        case N_WLblock:
            DBUG_ASSERT ((is_const), "non-constant block bounds found!");
            gap_node = MakeWLblock (WLNODE_LEVEL (wlnode), WLNODE_DIM (wlnode), val1,
                                    val2, 1, NULL, NULL, NULL);
            break;

        case N_WLublock:
            DBUG_ASSERT ((is_const), "non-constant block bounds found!");
            gap_node = MakeWLublock (WLNODE_LEVEL (wlnode), WLNODE_DIM (wlnode), val1,
                                     val2, 1, NULL, NULL, NULL);
            break;

        case N_WLstride:
            /* here is no break missing */
        case N_WLstrideVar:
            if (is_const) {
                gap_node = MakeWLstride (WLNODE_LEVEL (wlnode), WLNODE_DIM (wlnode), val1,
                                         val2, 1, FALSE, NULL, NULL);
            } else {
                gap_node = MakeWLstrideVar (WLNODE_LEVEL (wlnode), WLNODE_DIM (wlnode),
                                            NodeOrInt_MakeNode (nt1, pnode1),
                                            NodeOrInt_MakeNode (nt2, pnode2), MakeNum (1),
                                            NULL, NULL);
            }
            break;

        case N_WLgrid:
            /* here is no break missing */
        case N_WLgridVar:
            if (is_const) {
                gap_node = MakeWLgrid (WLNODE_LEVEL (wlnode), WLNODE_DIM (wlnode), val1,
                                       val2, FALSE, NULL, NULL, NULL);
            } else {
                gap_node
                  = MakeWLgridVar (WLNODE_LEVEL (wlnode), WLNODE_DIM (wlnode),
                                   NodeOrInt_MakeNode (nt1, pnode1),
                                   NodeOrInt_MakeNode (nt2, pnode2), NULL, NULL, NULL);
            }
            WLGRIDX_NOOP (gap_node) = is_noop;
            break;

        default:
            DBUG_ASSERT ((0), "illegal node type found!");
            break;
        }
    }

    DBUG_RETURN (gap_node);
}

/******************************************************************************
 *
 * Function:
 *   node *FillGapPred( node **new_node,   // a return value!!
 *                      node *wlnode,
 *                      nodetype nt1, void *pnode1,
 *                      nodetype nt2, void *pnode2,
 *                      bool is_noop)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
FillGapPred (node **new_node, /* a return value!! */
             node *wlnode, nodetype nt1, void *pnode1, nodetype nt2, void *pnode2,
             bool is_noop)
{
    node *gap_node;

    DBUG_ENTER ("FillGapPred");

    DBUG_ASSERT ((wlnode != NULL), "no WL node found!");

    gap_node = GenerateNodeForGap (wlnode, nt1, pnode1, nt2, pnode2, is_noop);

    if (gap_node != NULL) {
        WLNODE_NEXT (gap_node) = wlnode;
        wlnode = gap_node;
    }

    if (new_node != NULL) {
        (*new_node) = gap_node;
    }

    DBUG_RETURN (wlnode);
}

/******************************************************************************
 *
 * Function:
 *   node *FillGapSucc( node **new_node,   // a return value!!
 *                      node *wlnode,
 *                      nodetype nt1, void *pnode1,
 *                      nodetype nt2, void *pnode2,
 *                      bool is_noop)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
FillGapSucc (node **new_node, /* a return value!! */
             node *wlnode, nodetype nt1, void *pnode1, nodetype nt2, void *pnode2,
             bool is_noop)
{
    node *gap_node;

    DBUG_ENTER ("FillGapSucc");

    DBUG_ASSERT ((wlnode != NULL), "no WL node found!");

    gap_node = GenerateNodeForGap (wlnode, nt1, pnode1, nt2, pnode2, is_noop);

    if (gap_node != NULL) {
        WLNODE_NEXT (gap_node) = WLNODE_NEXT (wlnode);
        WLNODE_NEXT (wlnode) = gap_node;
    }

    if (new_node != NULL) {
        (*new_node) = gap_node;
    }

    DBUG_RETURN (wlnode);
}

/******************************************************************************
 *
 * Function:
 *   bool CheckWithids( node *part)
 *
 * Description:
 *   Checks whether NWITHID_VEC, NWITHID_IDS of all parts contain the same
 *   names.
 *
 ******************************************************************************/

#ifndef DBUG_OFF

/*
 * This function is called from within DBUG_ASSERTS only!
 */

static bool
CheckWithids (node *part)
{
    node *tmp;
    ids *_ids_part, *_ids_tmp;
    bool res = TRUE;

    DBUG_ENTER ("CheckWithids");

    DBUG_ASSERT ((NODE_TYPE (part) == N_Npart), "CheckWithids() needs a N_Npart node!");

    if (part != NULL) {
        tmp = NPART_NEXT (part);

        /*
         * compares each part (tmp) with the first one (part)
         */
        while (res && (tmp != NULL)) {
            /*
             * compares VEC
             */
            if (IDS_VARDEC (NPART_VEC (part)) != IDS_VARDEC (NPART_VEC (tmp))) {
                res = FALSE;
            } else {
                /*
                 * compares each ids-entry in IDS
                 */
                _ids_part = NPART_IDS (part);
                _ids_tmp = NPART_IDS (tmp);
                while (res && (_ids_part != NULL) && (_ids_tmp != NULL)) {
                    if (IDS_VARDEC (_ids_part) != IDS_VARDEC (_ids_tmp)) {
                        res = FALSE;
                    }
                    _ids_part = IDS_NEXT (_ids_part);
                    _ids_tmp = IDS_NEXT (_ids_tmp);
                }

                /*
                 * are some ids-entries left?
                 *  -> dimensionality differs -> error
                 */
                if ((_ids_part != NULL) || (_ids_tmp != NULL)) {
                    res = FALSE;
                }
            }

            tmp = NPART_NEXT (tmp);
        }
    }

    DBUG_RETURN (res);
}

#endif /* ! DBUG_OFF */

/**
 **
 **  functions for NodeOrInt, NameOrVal
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 *
 * Function:
 *   shpseg *GetWlShape( node *wl, int dims, types *wl_type)
 *
 * Description:
 *
 *
 ******************************************************************************/

shpseg *
GetWlShape (node *wl, int dims, types *wl_type)
{
    node *shp_node;
    int check_dims, i;
    shpseg *shape = NULL;

    DBUG_ENTER ("GetWlShape");

    switch (NWITH_TYPE (wl)) {
    case WO_genarray:
        shp_node = NWITH_SHAPE (wl);
        if (NODE_TYPE (shp_node) == N_array) {
            shape = MakeShpseg (NULL);
            shp_node = ARRAY_AELEMS (shp_node);
            for (i = 0; i < dims; i++) {
                DBUG_ASSERT ((shp_node != NULL),
                             "genarray with-loop:"
                             " size of index vector > dimension of WL shape");
                DBUG_ASSERT (((shp_node != NULL)
                              && (NODE_TYPE (EXPRS_EXPR (shp_node)) == N_num)),
                             "illegal shape found!");
                SHPSEG_SHAPE (shape, i) = NUM_VAL (EXPRS_EXPR (shp_node));
                shp_node = EXPRS_NEXT (shp_node);
            }
            DBUG_ASSERT ((shp_node == NULL),
                         "genarray with-loop:"
                         " size of index vector < dimension of WL shape");
        } else {
            DBUG_ASSERT ((NODE_TYPE (shp_node) == N_id),
                         "NWITH_SHAPE is neither N_array nor N_id");
            /*
             * handling of constant N_id nodes is missing here
             *
             * For the time being constant N_id nodes are substituted into
             * the N_Nwithop node during WLT (SSAWLT.c) ...
             * Note here, that it is not possible to use the SSA form in this
             * phase yet:
             *   - The previous phase RC can not handle SSA.
             *   - This phase can not handle SSA either since all NCODE_CEXPR
             *     have to have identical names especially in fold-WLs...
             */
        }
        break;

    case WO_modarray:
        shape = Type2Shpseg (wl_type, &check_dims);
        DBUG_ASSERT ((check_dims >= dims),
                     "modarray with-loop:"
                     " size of index vector > dimension of target array");
        break;

    case WO_foldfun:
        /* here is no break missing! */
    case WO_foldprf:
        break;
    }

    DBUG_RETURN (shape);
}

/******************************************************************************
 ******************************************************************************
 **
 **  functions for Parts2Strides()
 **
 **/

/******************************************************************************
 *
 * Function:
 *   bool IsEmptyStride_1( node *stride)
 *
 * Description:
 *   Checks whether the FIRST dimension of the given stride is empty or not.
 *
 ******************************************************************************/

static bool
IsEmptyStride_1 (node *stride)
{
    node *grid;
    int bound1, bound2, step, grid_b1, grid_b2;
    bool is_empty;

    DBUG_ENTER ("IsEmptyStride_1");

    DBUG_ASSERT ((NODE_TYPE (stride) == N_WLstride),
                 "given node is not a constant stride!");

    grid = WLSTRIDE_CONTENTS (stride);
    DBUG_ASSERT ((grid != NULL), "given stride contains no grid!");
    /*
     * For the time being support for multiple grids is not needed
     * and therefore not implemented yet ...
     */
    DBUG_ASSERT ((WLGRID_NEXT (grid) == NULL),
                 "given stride contains more than one grid!");

    bound1 = WLSTRIDE_BOUND1 (stride);
    bound2 = WLSTRIDE_BOUND2 (stride);
    step = WLSTRIDE_STEP (stride);
    grid_b1 = WLGRID_BOUND1 (grid);
    grid_b2 = WLGRID_BOUND2 (grid);

    CheckStride (bound1, bound2, step, grid_b1, grid_b2, FALSE);

    is_empty = (bound1 >= bound2);

    DBUG_RETURN (is_empty);
}

/******************************************************************************
 *
 * Function:
 *   node *ToFirstComponent( node *array)
 *
 * Description:
 *   returns 'ARRAY_AELEMS(array)' if 'array' is of type N_array, and 'array'
 *   otherwise.
 *
 ******************************************************************************/

static node *
ToFirstComponent (node *array)
{
    node *comp;

    DBUG_ENTER ("ToFirstComponent");

    if (array != NULL) {
        if (NODE_TYPE (array) == N_array) {
            comp = ARRAY_AELEMS (array);
        } else {
            DBUG_ASSERT ((NODE_TYPE (array) == N_id), "wrong node type found!");
            comp = array;
        }
    } else {
        comp = NULL;
    }

    DBUG_RETURN (comp);
}

/******************************************************************************
 *
 * Function:
 *   node *ToNextComponent( node *aelems)
 *
 * Description:
 *   returns the next component of 'aelems'.
 *
 ******************************************************************************/

static node *
ToNextComponent (node *aelems)
{
    node *comp;

    DBUG_ENTER ("ToNextComponent");

    if (aelems != NULL) {
        if (NODE_TYPE (aelems) == N_exprs) {
            comp = EXPRS_NEXT (aelems);
        } else {
            comp = aelems;
        }
    } else {
        comp = NULL;
    }

    DBUG_RETURN (comp);
}

/******************************************************************************
 *
 * Function:
 *   int CurrentComponent_GetInt( node *aelems)
 *
 * Description:
 *   Returns the current int-component of 'aelems'.
 *
 ******************************************************************************/

static int
CurrentComponent_GetInt (node *aelems)
{
    int comp;

    DBUG_ENTER ("CurrentComponent_GetInt");

    if (aelems != NULL) {
        switch (NODE_TYPE (aelems)) {
        case N_id:
            comp = IDX_OTHER;
            break;

        case N_exprs:
            if (NODE_TYPE (EXPRS_EXPR (aelems)) == N_num) {
                comp = NUM_VAL (EXPRS_EXPR (aelems));
            } else {
                DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (aelems)) == N_id),
                             "wrong node type found");
                comp = IDX_OTHER;
            }
            break;

        default:
            DBUG_ASSERT ((0), "wrong node type found");
            comp = IDX_OTHER;
            break;
        }
    } else {
        comp = 1;
    }

    DBUG_RETURN (comp);
}

/******************************************************************************
 *
 * Function:
 *   bool CurrentComponent_IsInt( node *aelems)
 *
 * Description:
 *
 *
 ******************************************************************************/

static bool
CurrentComponent_IsInt (node *aelems)
{
    bool res;

    DBUG_ENTER ("CurrentComponent_IsInt");

    if (aelems != NULL) {
        switch (NODE_TYPE (aelems)) {
        case N_id:
            res = FALSE;
            break;

        case N_exprs:
            if (NODE_TYPE (EXPRS_EXPR (aelems)) == N_num) {
                res = TRUE;
            } else {
                DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (aelems)) == N_id),
                             "wrong node type found");
                res = FALSE;
            }
            break;

        default:
            DBUG_ASSERT ((0), "wrong node type found");
            res = FALSE;
            break;
        }
    } else {
        res = TRUE;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * Function:
 *   node *CurrentComponent_GetNode( node *aelems)
 *
 * Description:
 *   Returns the current component of 'aelems' as a N_num (int-value exists)
 *   or N_id (no int-value exists) node.
 *
 ******************************************************************************/

static node *
CurrentComponent_GetNode (node *aelems)
{
    node *comp;
    int comp_int;

    DBUG_ENTER ("CurrentComponent_GetNode");

    comp_int = CurrentComponent_GetInt (aelems);

    if (IDX_IS_NUM (comp_int)) {
        comp = MakeNum (comp_int);
    } else {
        if (NODE_TYPE (aelems) == N_exprs) {
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (aelems)) == N_id),
                         "wrong node type found");
            comp = DupNode (EXPRS_EXPR (aelems));
        } else {
            DBUG_ASSERT ((NODE_TYPE (aelems) == N_id), "wrong node type found!");
            comp = DupNode (aelems);
        }
    }

    DBUG_RETURN (comp);
}

/******************************************************************************
 *
 * Function:
 *   node* Parts2Strides( node *parts, int dims, shpseg *shape)
 *
 * Description:
 *   Converts a N_Npart-chain ('parts') into a N_WLstride-chain (return).
 *   If 'shape' != NULL out-of-bounds values are adjusted accordingly.
 *
 ******************************************************************************/

static node *
Parts2Strides (node *parts, int dims, shpseg *shape)
{
    node *parts_stride, *stride, *new_stride, *new_grid;
    node *gen, *code;
    node *bound1, *bound2, *step, *width;
    int dim;
    bool is_empty;
    node *last_grid = NULL;

    DBUG_ENTER ("Parts2Strides");

    parts_stride = NULL;
    while (parts != NULL) {
        code = NPART_CODE (parts);
        /*
         * the N_Ncode node is reused for the new with-loop representation
         * therefore we have to remove it from 'parts'
         */
        NPART_CODE (parts) = NULL;

        stride = NULL;

        gen = NPART_GEN (parts);
        DBUG_ASSERT ((NGEN_OP1 (gen) == F_le), "op1 in generator is not <=");
        DBUG_ASSERT ((NGEN_OP2 (gen) == F_lt), "op2 in generator is not <");

        /* get components of current generator */
        bound1 = ToFirstComponent (NGEN_BOUND1 (gen));
        bound2 = ToFirstComponent (NGEN_BOUND2 (gen));
        step = ToFirstComponent (NGEN_STEP (gen));
        width = ToFirstComponent (NGEN_WIDTH (gen));

        is_empty = FALSE;
        new_grid = NULL;

        for (dim = 0; dim < dims; dim++) {
            DBUG_ASSERT ((bound1 != NULL), "bound1 incomplete");
            DBUG_ASSERT ((bound2 != NULL), "bound2 incomplete");

            if ((width == NULL) || CurrentComponent_IsInt (width)) {
                /*
                 * width is constant
                 */
                new_grid = MakeWLgrid (0, dim, 0, CurrentComponent_GetInt (width), FALSE,
                                       NULL, NULL, NULL);
            } else {
                /*
                 * width is not constant
                 */
                new_grid
                  = MakeWLgridVar (0, dim, MakeNum (0), CurrentComponent_GetNode (width),
                                   NULL, NULL, NULL);
            }

            if (CurrentComponent_IsInt (bound1) && CurrentComponent_IsInt (bound2)
                && ((step == NULL) || CurrentComponent_IsInt (step))) {
                /*
                 * all stride parameters are constant
                 */

                /* build N_WLstride-node of current dimension */
                new_stride
                  = MakeWLstride (0, dim, CurrentComponent_GetInt (bound1),
                                  (shape != NULL) ? MIN (CurrentComponent_GetInt (bound2),
                                                         SHPSEG_SHAPE (shape, dim))
                                                  : CurrentComponent_GetInt (bound2),
                                  CurrentComponent_GetInt (step), FALSE, new_grid, NULL);

                /* the PART-information is needed by 'IntersectStrideWithOutline' */
                WLSTRIDE_PART (new_stride) = parts;

                is_empty = IsEmptyStride_1 (new_stride);
            } else {
                /*
                 * not all stride parameters are constant
                 */

                /* build N_WLstrideVar-node of current dimension */
                new_stride
                  = MakeWLstrideVar (0, dim, CurrentComponent_GetNode (bound1),
                                     CurrentComponent_GetNode (bound2),
                                     CurrentComponent_GetNode (step), new_grid, NULL);
            }

            if (is_empty) {
                new_stride = FreeTree (new_stride);
                break;
            } else {
                /* append 'new_stride' to 'stride' */
                if (dim == 0) {
                    stride = new_stride;
                } else {
                    WLGRIDX_NEXTDIM (last_grid) = new_stride;
                }
                last_grid = new_grid;
            }

            /* go to next dim */
            bound1 = ToNextComponent (bound1);
            bound2 = ToNextComponent (bound2);
            step = ToNextComponent (step);
            width = ToNextComponent (width);
        }

        if (!is_empty) {
            WLGRIDX_CODE (new_grid) = code;
            NCODE_USED (code)++;
            WLGRIDX_NOOP (new_grid) = NCODE_AP_DUMMY_CODE (code);
            parts_stride = InsertWLnodes (parts_stride, stride);
        }

        parts = NPART_NEXT (parts);
    }
    while (parts != NULL)
        ;

    DBUG_RETURN (parts_stride);
}

/**
 **
 **  functions for Parts2Strides()
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 *
 * Function:
 *   node *EmptyParts2StridesOrExpr( node **wl, int dims, shpseg *shape)
 *
 * Description:
 *   This function handles the case in which all parts are empty.
 *
 ******************************************************************************/

static node *
EmptyParts2StridesOrExpr (node **wl, int dims, shpseg *shape)
{
    node *strides;
    node *tmp;

    DBUG_ENTER ("EmptyParts2StridesOrExpr");

    DBUG_EXECUTE ("WLtrans", NOTE (("  all parts of WL are empty!")));

    switch (NWITH2_TYPE ((*wl))) {
    case WO_genarray:
#if 0
      /*
       * empty genarray with-loops are not allowed!!!
       */
      ERROR( line, ("genarray-with-loop with empty index vector set found"));
#else
        if (
#if 0
          GetShpsegLength( dims, shape) > 0
#else
          TRUE
#endif
        ) {
            /*
             * result array of genarray-with-loop is non-empty
             *  -> generate a single stride over the whole domain
             */
            strides = GenerateShapeStrides (0, dims, shape);
        } else {
            /*
             * result array of genarray-with-loop is empty
             *  -> replace with-loop by empty array
             */
            node *cexpr;
            simpletype btype;

            tmp = *wl;

            cexpr = NWITH2_CEXPR ((*wl));
            DBUG_ASSERT ((NODE_TYPE (cexpr) == N_id), "CEXPR must be a N_id node!");
            btype = TYPES_BASETYPE (ID_TYPE (cexpr));
            DBUG_ASSERT ((btype != T_user), "Array elements of genarray-with-loop have a "
                                            "user-defined type!");
            *wl = CreateZeroVector (0, btype);
            strides = NULL;

            tmp = FreeNode (tmp);
        }
#endif
        break;

    case WO_modarray:
        tmp = *wl;

        /*
         * replace '*wl'
         */
        *wl = NWITH2_ARRAY ((*wl));
        strides = NULL;

        NWITH2_ARRAY (tmp) = NULL;
        tmp = FreeNode (tmp);
        break;

    case WO_foldfun:
        /* here is no break missing! */
    case WO_foldprf:
        tmp = *wl;

        /*
         * replace '*wl'
         */
        *wl = NWITH2_NEUTRAL ((*wl));
        strides = NULL;

        NWITH2_NEUTRAL (tmp) = NULL;
        tmp = FreeNode (tmp);
        break;

    default:
        strides = NULL;
    }

    DBUG_RETURN (strides);
}

#ifndef DBUG_OFF
/******************************************************************************
 ******************************************************************************
 **
 **  functions for CheckDisjointness()
 **
 **/

/******************************************************************************
 *
 * Function:
 *   bool StridesDisjoint_OneDim( int lb1, int ub1, int step1, int width1,
 *                                int lb2, int ub2, int step2, int width2)
 *
 * Description:
 *   Checks whether the given strides are disjoint.
 *
 *   stride1: lb1 <= iv1 < ub1 STEP step1 WIDTH width1
 *   stride2: lb2 <= iv1 < ub2 STEP step2 WIDTH width2
 *
 *   return value: 0 - not disjoint
 *                 1 - disjoint
 *
 ******************************************************************************/

static bool
StridesDisjoint_OneDim (int lb1, int ub1, int step1, int width1, int lb2, int ub2,
                        int step2, int width2)
{
    int ub, lb, step;
    int iv;
    bool disjoint = TRUE;

    DBUG_ENTER ("StridesDisjoint_OneDim");

    lb = MAX (lb1, lb2);
    ub = MIN (ub1, ub2);
    step = lcm (step1, step2);
    ub = MIN (ub, lb + step);

    for (iv = lb; iv < ub; iv++) {
        if (((iv - lb1) % step1 < width1) && ((iv - lb2) % step2 < width2)) {
            disjoint = FALSE;
            break;
        }
    }

    DBUG_RETURN (disjoint);
}

/******************************************************************************
 *
 * Function:
 *   bool StridesDisjoint_AllDims( node *stride1, node *stride2)
 *
 * Description:
 *   checks whether the given strides are disjoint (in at least one dimension).
 *
 *   return value: 0 - not disjoint
 *                 1 - disjoint
 *
 ******************************************************************************/

static bool
StridesDisjoint_AllDims (node *stride1, node *stride2)
{
    int disjoint = FALSE;
    node *grid1, *grid2;

    DBUG_ENTER ("StridesDisjoint_AllDims");

    if ((NODE_TYPE (stride1) == N_WLstrideVar)
        || (NODE_TYPE (stride2) == N_WLstrideVar)) {
        /*
         * disjointness check for var. strides not implemented yet
         */
        disjoint = TRUE;
    } else {
        while (stride1 != NULL) {
            DBUG_ASSERT ((stride2 != NULL),
                         "stride1 contains more dimensions than stride2");
            grid1 = WLSTRIDE_CONTENTS (stride1);
            grid2 = WLSTRIDE_CONTENTS (stride2);
            DBUG_ASSERT (((grid1 != NULL) && (grid2 != NULL)),
                         "stride with missing grid found");

            if (StridesDisjoint_OneDim (WLSTRIDE_BOUND1 (stride1) + WLGRID_BOUND1 (grid1),
                                        WLSTRIDE_BOUND2 (stride1),
                                        WLSTRIDE_STEP (stride1),
                                        WLGRID_BOUND2 (grid1) - WLGRID_BOUND1 (grid1),
                                        WLSTRIDE_BOUND1 (stride2) + WLGRID_BOUND1 (grid2),
                                        WLSTRIDE_BOUND2 (stride2),
                                        WLSTRIDE_STEP (stride2),
                                        WLGRID_BOUND2 (grid2) - WLGRID_BOUND1 (grid2))) {
                disjoint = TRUE;
                break;
            }

            stride1 = WLGRID_NEXTDIM (grid1);
            stride2 = WLGRID_NEXTDIM (grid2);
        }
    }

    DBUG_RETURN (disjoint);
}

/******************************************************************************
 *
 * Function:
 *   bool CheckDisjointness( node *strides)
 *
 * Description:
 *   checks whether all strides are pairwise disjoint.
 *
 *   return value: 0 - not disjoint
 *                 1 - disjoint
 *
 ******************************************************************************/

static bool
CheckDisjointness (node *strides)
{
    node *stride2;
    int disjoint = TRUE;

    DBUG_ENTER ("CheckDisjointness");

    while (strides != NULL) {
        stride2 = WLSTRIDEX_NEXT (strides);
        while (stride2 != NULL) {
            if (!StridesDisjoint_AllDims (strides, stride2)) {
                disjoint = FALSE;
                goto ret;
            }
            stride2 = WLSTRIDEX_NEXT (stride2);
        }
        strides = WLSTRIDEX_NEXT (strides);
    }

ret:
    DBUG_RETURN (disjoint);
}

/**
 **
 **  functions for CheckDisjointness()
 **
 ******************************************************************************
 ******************************************************************************/
#endif

/******************************************************************************
 ******************************************************************************
 **
 **  functions for ComputeCubes()
 **
 **/

/******************************************************************************
 *
 * Function:
 *   int TestAndDivideStrides( node *stride1, node *stride2,
 *                             node **divided_stridea, node **divided_strideb)
 *
 * Description:
 *   this function divides 'stride1' or 'stride2' if necessary, and returns
 *     the two yielded strides in 'divided_stridea', 'divided_strideb'.
 *   possibly present next nodes in 'stride1' or 'stride2' are ignored.
 *
 *   return value: 0 - no strides has been divided
 *                     ('divided_stride?' should be NULL in this case)
 *                 1 - 'stride1' has been divided because of 'stride2'
 *                 2 - 'stride2' has been divided because of 'stride1'
 *   (note, that the cases 1 and 2 are mutually exclusive!)
 *
 ******************************************************************************/

static int
TestAndDivideStrides (node *stride1, node *stride2, node **divided_stridea,
                      node **divided_strideb)
{
    node *grid1, *grid2;
    node *trav_d_stride1a, *trav_d_stride1b, *trav_d_stride2a, *trav_d_stride2b;
    node *divided_stride1a, *divided_stride1b, *divided_stride2a, *divided_stride2b;
    int bound11, bound21, grid1_b1, grid1_b2, bound12, bound22, grid2_b1, grid2_b2;
    int head1, rear1, head2, rear2;
    int i_bound1, i_bound2, i_offset1, i_offset2, offset;
    int result = 0;

    DBUG_ENTER ("TestAndDivideStrides");

    DBUG_ASSERT (((divided_stridea != NULL) && (divided_strideb != NULL)),
                 "a reference node pointer of TestAndDivideStrides() is NULL!");

    *divided_stridea = *divided_strideb = NULL;

    trav_d_stride1a = divided_stride1a = DupNode (stride1);
    trav_d_stride1b = divided_stride1b = DupNode (stride1);
    trav_d_stride2a = divided_stride2a = DupNode (stride2);
    trav_d_stride2b = divided_stride2b = DupNode (stride2);

    while (stride1 != NULL) {
        DBUG_ASSERT ((stride2 != NULL), "missing dim in second stride!");

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

        if (/* are the outlines of 'stride1' and 'stride2' not disjoint? */
            (head1 < rear2) && (head2 < rear1) &&

            /* are 'stride1' and 'stride2' descended from different Npart-nodes? */
            (WLSTRIDE_PART (stride1) != WLSTRIDE_PART (stride2)) &&
            /*
             * Note: if stride1 and stride are descended from the same Npart-node
             *        they must have disjoint outlines!!!
             */

            /* are the grids compatible? */
            (i_offset1 <= grid1_b1) && (i_offset2 <= grid2_b1)
            /*
             * Note:
             * (i_offset_1 > grid1_b1) means, that the stride1 must be split in
             * two parts to fit the new upper bound in the current dim.
             * Then the *projections* of stride1, stride2 can not be disjoint,
             * therefore stride1, stride2 must have disjoint outlines!!!
             */
        ) {

            /*
             * Now we assume, that the outlines of 'stride1' and 'stride2' are not
             * disjoint!
             * (Nevertheless in following dimensions we may infer, that this
             * property is *not* hold  ->  we will dump the collected data then.)
             */

            if (i_bound1 + grid1_b1 - i_offset1 >= i_bound2) {
                /*
                 * the intersection of 'stride1' with the outline of 'stride2' is empty
                 *  -> dividing stride1
                 */
                DBUG_ASSERT ((i_bound1 + grid2_b1 - i_offset2 < i_bound2),
                             "the intersection of stride1 with the outline of stride2"
                             " as well as"
                             " the intersection of stride2 with the outline of stride1"
                             " are empty!");
                WLSTRIDE_BOUND2 (trav_d_stride1a) = bound12;

                WLSTRIDE_BOUND1 (trav_d_stride1b) = bound22;
                offset = GridOffset (bound22, bound11, WLSTRIDE_STEP (stride1), grid1_b2);
                DBUG_ASSERT ((offset <= grid1_b1), "offset is inconsistant");
                WLGRID_BOUND1 (WLSTRIDE_CONTENTS (trav_d_stride1b)) -= offset;
                WLGRID_BOUND2 (WLSTRIDE_CONTENTS (trav_d_stride1b)) -= offset;

                result = 1;
                trav_d_stride1a = NormalizeStride_1 (trav_d_stride1a);
                trav_d_stride1b = NormalizeStride_1 (trav_d_stride1b);

                /* we can stop here */
                break;
            } else if (i_bound1 + grid2_b1 - i_offset2 >= i_bound2) {
                /*
                 * the intersection of 'stride2' with the outline of 'stride1' is empty
                 *   -> dividing stride2
                 */
                WLSTRIDE_BOUND2 (trav_d_stride2a) = bound11;

                WLSTRIDE_BOUND1 (trav_d_stride2b) = bound21;
                offset = GridOffset (bound21, bound12, WLSTRIDE_STEP (stride2), grid2_b2);
                DBUG_ASSERT ((offset <= grid2_b1), "offset is inconsistant");
                WLGRID_BOUND1 (WLSTRIDE_CONTENTS (trav_d_stride2b)) -= offset;
                WLGRID_BOUND2 (WLSTRIDE_CONTENTS (trav_d_stride2b)) -= offset;

                result = 2;
                trav_d_stride2a = NormalizeStride_1 (trav_d_stride2a);
                trav_d_stride2b = NormalizeStride_1 (trav_d_stride2b);
                trav_d_stride1b = NormalizeStride_1 (trav_d_stride1b);

                /* we can stop here */
                break;
            } else {
                /* next dim */
                stride1 = WLGRID_NEXTDIM (grid1);
                stride2 = WLGRID_NEXTDIM (grid2);
                trav_d_stride1a = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (trav_d_stride1a));
                trav_d_stride1b = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (trav_d_stride1b));
                trav_d_stride2a = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (trav_d_stride2a));
                trav_d_stride2b = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (trav_d_stride2b));
            }
        } else {
            /*
             * the outlines are disjoint
             *  -> free the useless data in 'divided_stride?'
             */
            divided_stride1a = FreeTree (divided_stride1a);
            divided_stride1b = FreeTree (divided_stride1b);
            divided_stride2a = FreeTree (divided_stride2a);
            divided_stride2b = FreeTree (divided_stride2b);
            *divided_stridea = *divided_strideb = NULL;
            result = 0;

            /* we can give up here */
            break;
        }
    }

    if (result != 0) {
        if (result == 1) {
            *divided_stridea = divided_stride1a;
            *divided_strideb = divided_stride1b;
        } else {
            *divided_stridea = divided_stride2a;
            *divided_strideb = divided_stride2b;
        }
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * Function:
 *   bool IntersectStrideWithOutline( node *stride1, node *stride2,
 *                                    node **i_stride1, node **i_stride2)
 *
 * Description:
 *   returns in 'i_stride1' and 'i_stride2' the part of 'stride1', 'stride2'
 *     respectively that lies in a common cube.
 *   possibly present next nodes in 'stride1' or 'stride2' are ignored.
 *   the return value is 1 if and only if the intersection is non-empty.
 *
 *   if interested in the return value only, call this function with
 *     ('i_stride1' == NULL), ('i_stride2' == NULL).
 *
 ******************************************************************************/

static bool
IntersectStrideWithOutline (node *stride1, node *stride2, node **i_stride1,
                            node **i_stride2)
{
    node *grid1, *grid2;
    int bound11, bound21, grid1_b1, grid1_b2, bound12, bound22, grid2_b1, grid2_b2, head1,
      rear1, head2, rear2, i_bound1, i_bound2, i_offset1, i_offset2;
    node *trav_i_stride1 = NULL;
    node *trav_i_stride2 = NULL;
    bool result = TRUE;

    DBUG_ENTER ("IntersectStrideWithOutline");

    if (i_stride1 != NULL) {
        trav_i_stride1 = *i_stride1 = DupNode (stride1);
    }
    if (i_stride2 != NULL) {
        trav_i_stride2 = *i_stride2 = DupNode (stride2);
    }

    while (stride1 != NULL) {
        DBUG_ASSERT ((stride2 != NULL), "missing dim in second stride!");

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

        if (/*
             * are the projection outlines of 'stride1' and 'stride2' not disjoint?
             */
            (head1 < rear2) && (head2 < rear1) &&

            /* are 'stride1' and 'stride2' descended from different Npart-nodes? */
            (WLSTRIDE_PART (stride1) != WLSTRIDE_PART (stride2)) &&
            /*
             * Note: if stride1 and stride are descended from the same Npart-node
             *       we can deduce that they have disjoint outlines!
             */

            /* are the grids compatible? */
            (i_offset1 <= grid1_b1) && (i_offset2 <= grid2_b1)
            /*
             * Note:
             * (i_offset_1 > grid1_b1) means, that the stride1 must be split in
             * two parts to fit the new upper bound in the current dim.
             * Then the *projections* of stride1, stride2 can not be disjoint,
             * therefore stride1, stride2 must have disjoint outlines!!!
             */
        ) {

            /*
             * Now we assume, that the outlines of 'stride1' and 'stride2' are not
             * disjoint!
             * (Nevertheless in following dimensions we may infer, that this
             * property is *not* hold  ->  we will dump the collected data then.)
             */

            if (
              /* is intersection of 'stride1' with the outline of 'stride2' empty? */
              (i_bound1 + grid1_b1 - i_offset1 >= i_bound2) ||
              /* is intersection of 'stride2' with the outline of 'stride1' empty? */
              (i_bound1 + grid2_b1 - i_offset2 >= i_bound2)) {

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
                 *
                 * The following parts of the 'cube generation' can not handle this
                 * case! Therefore we will stop here!!
                 *
                 * Remark: If this assertion fails, there is a bug in the 'first step'
                 *         of 'ComputeCubes()' !!!
                 */

                DBUG_ASSERT ((0),
                             ("must resign:"
                              " intersection of outline(stride1) and outline(stride2) is"
                              " non-empty, while intersection of outline(stride1) and "
                              "stride2,"
                              " or intersection of stride1 and outline(stride2) is empty "
                              ":-("));
            }

            /* intersect 'stride1' with the outline of 'stride2' */
            if (i_stride1 != NULL) {
                WLSTRIDE_BOUND1 (trav_i_stride1) = i_bound1;
                WLSTRIDE_BOUND2 (trav_i_stride1) = i_bound2;
                WLGRID_BOUND1 (WLSTRIDE_CONTENTS (trav_i_stride1)) = grid1_b1 - i_offset1;
                WLGRID_BOUND2 (WLSTRIDE_CONTENTS (trav_i_stride1)) = grid1_b2 - i_offset1;
                trav_i_stride1 = NormalizeStride_1 (trav_i_stride1);
            }

            /* intersect 'stride2' with the outline of 'stride1' */
            if (i_stride2 != NULL) {
                WLSTRIDE_BOUND1 (trav_i_stride2) = i_bound1;
                WLSTRIDE_BOUND2 (trav_i_stride2) = i_bound2;
                WLGRID_BOUND1 (WLSTRIDE_CONTENTS (trav_i_stride2)) = grid2_b1 - i_offset2;
                WLGRID_BOUND2 (WLSTRIDE_CONTENTS (trav_i_stride2)) = grid2_b2 - i_offset2;
                trav_i_stride2 = NormalizeStride_1 (trav_i_stride2);
            }

            /* next dim */
            stride1 = WLGRID_NEXTDIM (grid1);
            stride2 = WLGRID_NEXTDIM (grid2);
            if (i_stride1 != NULL) {
                trav_i_stride1 = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (trav_i_stride1));
            }
            if (i_stride2 != NULL) {
                trav_i_stride2 = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (trav_i_stride2));
            }
        } else {
            /*
             * the outlines are disjoint
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
            result = FALSE;

            /* we can give up here */
            break;
        }
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * Function:
 *   int IsSubsetStride( node *stride1, node *stride2)
 *
 * Description:
 *   determines whether stride2 is a subset of stride1 (return value 1) or
 *   stride1 is a subset of stride2 (return value 2) or nothing of these
 *   (return value 0).
 *
 *   this function is used in EleminateDuplicatesAndAdjustBounds().
 *
 ******************************************************************************/

static int
IsSubsetStride (node *stride1, node *stride2)
{
    node *new_stride1, *new_stride2;
    int bound11, bound21, bound12, bound22;
    int res = 0;

    DBUG_ENTER ("IsSubsetStride");

    DBUG_ASSERT (((NODE_TYPE (stride1) == N_WLstride)
                  && (NODE_TYPE (stride2) == N_WLstride)),
                 "call by reference params are NULL");

    if (WLSTRIDE_PART (stride1) == WLSTRIDE_PART (stride2)) {
        new_stride1 = stride1;
        new_stride2 = stride2;

        while (new_stride1 != NULL) {
            res = -1;
            DBUG_ASSERT ((new_stride2 != NULL), "dim not found");

            bound11 = WLSTRIDE_BOUND1 (new_stride1);
            bound21 = WLSTRIDE_BOUND2 (new_stride1);
            bound12 = WLSTRIDE_BOUND1 (new_stride2);
            bound22 = WLSTRIDE_BOUND2 (new_stride2);

            if ((bound12 >= bound11) && (bound22 <= bound21)) {
                /* stride2 is subset of stride1 */
                if ((bound12 > bound11) || (bound22 < bound21)) {
                    /* stride2 is *proper* subset of stride1 */
                    res = 1;
                }
            } else {
                if ((bound11 >= bound12) && (bound21 <= bound22)) {
                    /* stride1 is subset of stride2 */
                    if ((bound11 > bound12) || (bound21 < bound22)) {
                        /* stride1 is *proper* subset of stride2 */
                        res = 2;
                    }
                } else {
                    res = 0;
                    break;
                }
            }
            new_stride1 = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (new_stride1));
            new_stride2 = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (new_stride2));
        }
    }

    if (res == -1) {
        /*
         * the two strides are equal -> remove one of them
         */
        res = 1;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * Function:
 *   int AdjustBounds( node **stride1, node **stride2)
 *
 * Description:
 *   adjusts the bounds of one of two strides (stride1, stride2) taken from the
 *   same WL-part to get disjoint strides.
 *   returns 0 if stride1 and stride2 are left unchanged, 1 if stride1 has
 *   been adjusted, 2 if stride2 has been adjusted.
 *
 *   example:
 *     0->7  step 3, 1->3: op2
 *     3->16 step 3, 1->3: op2
 *   is transformed into
 *     0->4  step 3, 1->3: op2
 *     3->16 step 3, 1->3: op2
 *
 *   this function is used in EleminateDuplicatesAndAdjustBounds().
 *
 ******************************************************************************/

static int
AdjustBounds (node **stride1, node **stride2)
{
    node *new_stride1, *new_stride2;
    int bound11, bound21, bound12, bound22;
    int res = 0;

    DBUG_ENTER ("AdjustBounds");

    DBUG_ASSERT (((stride1 != NULL) && (stride2 != NULL)),
                 "call by reference parameters are NULL");

    if (WLSTRIDE_PART (*stride1) == WLSTRIDE_PART (*stride2)) {
        /*
         * transformation needed for strides taken from the same WL-part only
         */
        new_stride1 = *stride1;
        new_stride2 = *stride2;

        while (new_stride1 != NULL) {
            DBUG_ASSERT ((new_stride2 != NULL), "dim of stride not found");

            bound11 = WLSTRIDE_BOUND1 (new_stride1);
            bound21 = WLSTRIDE_BOUND2 (new_stride1);
            bound12 = WLSTRIDE_BOUND1 (new_stride2);
            bound22 = WLSTRIDE_BOUND2 (new_stride2);

            if (bound21 < bound22) {
                DBUG_ASSERT ((bound11 < bound12), "the two strides are not disjoint");
                if (IndexRearStride (new_stride1) > bound12) { /* bound21 > bound12 */
                    res = 1;
                    WLSTRIDE_BOUND2 (new_stride1) = bound12;
                    new_stride1 = NormalizeStride_1 (new_stride1);
                }
                break;
            } else {
                if (bound21 > bound22) {
                    DBUG_ASSERT ((bound12 < bound11), "the two strides are not disjoint");
                    if (IndexRearStride (new_stride2) > bound11) { /* bound22 > bound11 */
                        res = 2;
                        WLSTRIDE_BOUND2 (new_stride2) = bound11;
                        new_stride2 = NormalizeStride_1 (new_stride2);
                    }
                    break;
                } else {
                    new_stride1 = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (new_stride1));
                    new_stride2 = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (new_stride2));
                }
            }
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * Function:
 *   node* EleminateDuplicatesAndAdjustBounds( node *strides)
 *
 * Description:
 *   When ComputeCubes() splits the strides by using
 *   IntersectStrideWithOutline() this may create non-disjoint
 *   index vector sets A and B ...
 *
 *   a) ... where A is a subset of B. Example:
 *
 *       0->140 step 9, 2->9: op1
 *       0->306 step 9, 0->2: op2
 *     135->302 step 9, 5->9: op3
 *     140->300 step 9, 6->9: op4
 *    ----------------------- after first round with IntersectStrideWithOutline:
 *       0->140 step 9, 2->9: op1
 *       0->144 step 9, 0->2: op2 <- intersection of 'op2' and outline('op1')
 *     128->306 step 9, 7->9: op2 <- inters. of 'op2' and outline('op3')  (*1*)
 *     137->306 step 9, 7->9: op2 <- inters. of 'op2' and outline('op4')  (*2*)
 *     135->302 step 9, 5->9: op3
 *     140->300 step 9, 6->9: op4
 *
 *     (*2*) is a subset of (*1*), therefore (*1*) should be removed!
 *
 *   b) ... where A and B are no subsets of each other. Example:
 *
 *     0->6  step 3, 0->1: op1
 *     0->16 step 3, 1->3: op2
 *     4->16 step 3, 2->3: op3
 *    ----------------------- after first round with IntersectStrideWithOutline:
 *     0->7  step 3, 1->3: op2  <- intersection of 'op2' and outline('op1')
 *     3->16 step 3, 1->3: op2  <- intersection of 'op2' and outline('op3')
 *
 *     These two strides are **not** disjoint!!!
 *     But they are part of the same Npart!!
 *
 *   Therefore first whole strides are removed (a) and then bounds are adjusted
 *   to get disjoint index vector sets (b).
 *
 ******************************************************************************/

static node *
EleminateDuplicatesAndAdjustBounds (node *strides)
{
    node *stride1, *prev_stride1, *stride2, *prev_stride2;
    int res;

    DBUG_ENTER ("EleminateDuplicatesAndAdjustBounds");

    /*
     * first step: remove redundant strides
     */
    stride1 = strides;
    prev_stride1 = NULL;
    while (stride1 != NULL) {

        stride2 = WLSTRIDE_NEXT (stride1);
        prev_stride2 = stride1;
        while (stride2 != NULL) {

            res = IsSubsetStride (stride1, stride2);
            if (res == 1) {
                /*
                 * stride2 is subset of stride1
                 *   -> remove 'stride1' from chain
                 */
                if (prev_stride1 == NULL) {
                    strides = WLSTRIDE_NEXT (strides);
                    stride1 = FreeNode (stride1);
                } else {
                    stride1 = WLSTRIDE_NEXT (prev_stride1)
                      = FreeNode (WLSTRIDE_NEXT (prev_stride1));
                }
                stride2 = WLSTRIDE_NEXT (stride1);
                prev_stride2 = stride1;
            } else {
                if (res == 2) {
                    /*
                     * stride1 is subset of stride2
                     *   -> remove 'stride2' from chain
                     */
                    stride2 = WLSTRIDE_NEXT (prev_stride2)
                      = FreeNode (WLSTRIDE_NEXT (prev_stride2));
                } else {
                    DBUG_ASSERT ((res == 0), "unknown value returned");
                    prev_stride2 = stride2;
                    stride2 = WLSTRIDE_NEXT (stride2);
                }
            }
        }

        prev_stride1 = stride1;
        stride1 = WLSTRIDE_NEXT (stride1);
    }

    /*
     * second step: adjust bounds of non-disjoint strides
     */
    stride1 = strides;
    while (stride1 != NULL) {
        stride2 = WLSTRIDE_NEXT (stride1);
        while (stride2 != NULL) {
            AdjustBounds (&stride1, &stride2);
            stride2 = WLSTRIDE_NEXT (stride2);
        }
        stride1 = WLSTRIDE_NEXT (stride1);
    }

    DBUG_RETURN (strides);
}

static node *MergeWL (node *nodes); /* forward declaration */

/******************************************************************************
 *
 * Function:
 *   node *ComputeCubes( node *strides)
 *
 * Description:
 *   returns the set of cubes as a N_WLstride-chain
 *
 ******************************************************************************/

static node *
ComputeCubes (node *strides)
{
    node *new_strides, *stride1, *stride2, *i_stride1, *i_stride2, *divided_stridea,
      *divided_strideb, *remain, *last_remain, *last_stride1, *tmp;
    int res;
    bool fixpoint;

    DBUG_ENTER ("ComputeCubes");

    DBUG_ASSERT ((NODE_TYPE (strides) == N_WLstride), "wrong node type found");

    /*
     * first step:
     * -----------
     *
     * divide strides
     *  -> for every two strides (stride1, stride2) the following property is
     *     hold:
     *     if outline(stride1) and outline(stride2) are not disjoint,
     *     then the intersection of stride1 with outline(stride2) is non-empty!
     *  (this property is exploited in the second step)
     *
     * this property is *not* hold in general!!!!
     * example:
     *
     *   stride1: 0->5 step 2          stride2: 2->3 step 1
     *                   1->2: ...                     0->1: ...
     *
     * The outlines of these two strides are not disjoint.
     * Nevertheless the intersection of stride1 with outline(stride2) is empty:
     *
     *            2->3 step 2
     *                   1->2: ???
     *
     ************
     *
     * Attention:
     *   In general a single round (test-and-divide all pairs of strides) is
     *   *not* sufficient! Example:
     *
     *     0->4 step 2            0: ( op1 )
     *            0->1: op1       1: ( op2 )
     *     0->5 step 3            2: ( op1 )
     *            1->2: op2       3: ( op3 )
     *     3->4 step 1:           4: ( op2 )
     *            0->1: op3       5:
     *
     *   ---> in first round: split op2 because of op3 --->
     *
     *     0->4 step 2            0: ( op1  )
     *            0->1: op1       1: ( op2a )
     *     1->2 step 1            2: ( op1  )
     *            0->1: op2a      3: ( op3  )
     *     3->4 step 1:           4: ( op2b )
     *            0->1: op3       5:
     *     4->5 step 1
     *            0->1: op2b
     *
     *   ---> in second round: split op1 because of op2a --->
     *
     *     0->1 step 1            0: ( op1a )
     *            0->1: op1a      1: ( op2a )
     *     1->2 step 1            2: ( op1b )
     *            0->1: op2a      3: ( op3  )
     *     2->3 step 1:           4: ( op2b )
     *            0->1: op1b      5:
     *     3->4 step 1:
     *            0->1: op3
     *     4->5 step 1
     *            0->1: op2b
     *
     *  Therefore fixpoint-iteration is needed!
     */
    do {
        fixpoint = TRUE;
        new_strides = NULL;

        /* check WLSTRIDE_MODIFIED */
        stride1 = strides;
        while (stride1 != NULL) {
            DBUG_ASSERT ((WLSTRIDE_MODIFIED (stride1) == NULL), "stride was modified");
            stride1 = WLSTRIDE_NEXT (stride1);
        }

        /* test-and-divide the elements of 'strides' pairwise */
        stride1 = strides;
        while (stride1 != NULL) {
            stride2 = WLSTRIDE_NEXT (stride1);
            while (stride2 != NULL) {
                res = TestAndDivideStrides (stride1, stride2, &divided_stridea,
                                            &divided_strideb);

                if (res == 1) {
                    fixpoint = FALSE;
                    WLSTRIDE_MODIFIED (stride1) = divided_stridea;
                } else {
                    if (res == 2) {
                        fixpoint = FALSE;
                        WLSTRIDE_MODIFIED (stride2) = divided_stridea;
                    } else {
                        DBUG_ASSERT (((divided_stridea == NULL)
                                      && (divided_strideb == NULL)),
                                     "results of TestAndDivideStrides() are "
                                     "inconsistent!");
                    }
                }

                new_strides = InsertWLnodes (new_strides, divided_stridea);
                new_strides = InsertWLnodes (new_strides, divided_strideb);
                stride2 = WLSTRIDE_NEXT (stride2);
            }

            /* was 'stride1' not modified? */
            if (WLSTRIDE_MODIFIED (stride1) == NULL) {
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
     * second step:
     * ------------
     *
     * create disjoint outlines
     *  -> every stride lies in one and only one cube
     */
    do {
        fixpoint = TRUE;
        new_strides = NULL;

        /* check WLSTRIDE_MODIFIED */
        stride1 = strides;
        while (stride1 != NULL) {
            DBUG_ASSERT ((WLSTRIDE_MODIFIED (stride1) == NULL), "stride was modified");
            stride1 = WLSTRIDE_NEXT (stride1);
        }

        /* intersect the elements of 'strides' pairwise */
        stride1 = strides;
        while (stride1 != NULL) {
            stride2 = WLSTRIDE_NEXT (stride1);
            while (stride2 != NULL) {
                /* intersect 'stride1' with outline of 'stride2' and vise versa */
                IntersectStrideWithOutline (stride1, stride2, &i_stride1, &i_stride2);

                if (i_stride1 != NULL) {
                    if (CompareWLnode (stride1, i_stride1, 1) != 0) {
                        fixpoint = FALSE;
                        WLSTRIDE_MODIFIED (stride1) = i_stride1;
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
                        fixpoint = FALSE;
                        WLSTRIDE_MODIFIED (stride2) = i_stride2;
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
            if (WLSTRIDE_MODIFIED (stride1) == NULL) {
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

        strides = EleminateDuplicatesAndAdjustBounds (new_strides);
    } while (!fixpoint);

    /*
     * third step:
     * -----------
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
            if (IntersectStrideWithOutline (stride1, stride2, NULL, NULL)) {
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

/**
 **
 **  functions for ComputeCubes()
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 ******************************************************************************
 **
 **  functions for BuildCubes()
 **
 **/

/******************************************************************************
 *
 * Function:
 *   node *GenerateCompleteDomain( node *stride, int dims, shpseg *shape)
 *
 * Description:
 *   Supplements strides/grids for the complement of N_WLstride node 'stride'.
 *
 *   For constant strides we must *not* optimize and merge strides, because
 *   'BlockWL()' can not handle them!! We must create simple cubes instead.
 *   Example (with shape [10,10]):
 *
 *       5 -> 10 step[0] 2
 *                  0 -> 1: 0 -> 5  step[1] 1
 *                                     0 -> 1: op
 *
 *     is *not* converted into
 *
 *       0 ->  5 step[0] 1
 *                  0 -> 1: 0 -> 10 step[1] 1
 *                                     0 -> 1: init/copy
 *       5 -> 10 step[0] 2
 *                  0 -> 1: 0 -> 5  step[1] 1
 *                                     0 -> 1: op
 *                          5 -> 10 step[1] 1
 *                                     0 -> 1: init/copy
 *                  1 -> 2: 0 -> 10 step[1] 1
 *                                     0 -> 1: init/copy
 *
 *     but into
 *
 *       0 ->  5 step[0] 1
 *                  0 -> 1: 0 -> 10 step[1] 1
 *                                     0 -> 1: init/copy
 *       5 -> 10 step[0] 2
 *                  0 -> 1: 0 -> 5  step[1] 1
 *                                     0 -> 1: op
 *                  1 -> 2: 0 -> 5  step[1] 1
 *                                     0 -> 1: init/copy
 *       5 -> 10 step[0] 1
 *                  0 -> 1: 5 -> 10 step[1] 1
 *                                     0 -> 1: init/copy
 *
 *   This function is called by 'ComputeOneCube'.
 *
 ******************************************************************************/

static node *
GenerateCompleteDomain (node *stride, int dims, shpseg *shape)
{
    node *new_strides, *new_stride, *new_grid;
    node *next_dim;
    node *compl_stride;
    node *act_stride, *act_grid;
    node *act_compl_stride, *last_compl_grid;
    int lb;
    node *act_compl_grid = NULL;
    node *dup_strides = NULL;
    node *last_dup_grid = NULL;

    DBUG_ENTER ("GenerateCompleteDomain");

    DBUG_ASSERT ((stride != NULL), "no stride found");
    DBUG_ASSERT ((NODE_TYPE (stride) == N_WLstride), "var. stride found");
    DBUG_ASSERT ((WLSTRIDE_NEXT (stride) == NULL), "more than one stride found");
    DBUG_ASSERT ((shape != NULL), "no shape information found!");

    /*
     * we duplicate 'strides'
     *  -> later on we use this to generate complement strides
     */
    compl_stride = DupNode (stride);
    /*
     * set all steps to '1' and remove the code in the duplicated chain
     */
    act_compl_stride = compl_stride;
    while (act_compl_stride != NULL) {
        act_compl_stride = NormalizeGrids (act_compl_stride);
        WLSTRIDE_STEP (act_compl_stride) = 1;
        act_compl_grid = WLSTRIDE_CONTENTS (act_compl_stride);
        WLGRID_BOUND2 (act_compl_grid) = 1;
        act_compl_stride = WLGRID_NEXTDIM (act_compl_grid);
    }
    WLGRID_CODE (act_compl_grid) = NULL;

    new_strides = NULL;
    act_stride = stride;
    act_compl_stride = compl_stride;
    last_compl_grid = NULL;
    while (act_stride != NULL) {
        act_grid = WLSTRIDE_CONTENTS (act_stride);
        act_compl_grid = WLSTRIDE_CONTENTS (act_compl_stride);
        DBUG_ASSERT ((NODE_TYPE (act_stride) == N_WLstride), "var. stride found");
        DBUG_ASSERT ((NODE_TYPE (act_grid) == N_WLgrid), "var. grid found");

        /*
         * normalize the bounds
         */
        act_stride = NormalizeGrids (act_stride);

        /*
         * insert lower part of complement
         */
        new_stride
          = GenerateNodeForGap (act_stride, N_WLstride, &(WLSTRIDE_BOUND2 (act_stride)),
                                N_WLstride,
                                &(SHPSEG_SHAPE (shape, WLSTRIDE_DIM (act_stride))),
                                FALSE);
        if (new_stride != NULL) {
            WLSTRIDE_CONTENTS (new_stride)
              = MakeWLgrid (0, WLGRID_DIM (act_grid), 0, 1, FALSE,
                            GenerateShapeStrides (WLGRID_DIM (act_grid) + 1, dims, shape),
                            NULL, NULL);

            if (last_compl_grid != NULL) {
                /*
                 * duplicate 'compl_stride' from root til 'last_compl_grid'.
                 */
                next_dim = WLGRID_NEXTDIM (last_compl_grid);
                WLGRID_NEXTDIM (last_compl_grid) = NULL;
                dup_strides = DupNode (compl_stride);
                WLGRID_NEXTDIM (last_compl_grid) = next_dim;
                /*
                 * go to duplicated 'last_compl_grid'
                 */
                last_dup_grid = WLSTRIDE_CONTENTS (dup_strides);
                while (WLGRID_NEXTDIM (last_dup_grid) != NULL) {
                    last_dup_grid = WLSTRIDE_CONTENTS (WLGRID_NEXTDIM (last_dup_grid));
                }
            }

            /*
             * append new stride/grid to duplicated 'compl_stride'
             */
            if (last_compl_grid != NULL) {
                WLGRID_NEXTDIM (last_dup_grid) = new_stride;
            } else {
                dup_strides = new_stride;
            }

            /*
             * insert 'dup_strides' into 'new_strides'
             */
            new_strides = InsertWLnodes (new_strides, dup_strides);
        }

        /*
         * insert upper part of complement
         */
        lb = 0;
        new_stride = GenerateNodeForGap (act_stride, N_WLstride, &lb, N_WLstride,
                                         &(WLSTRIDE_BOUND1 (act_stride)), FALSE);
        if (new_stride != NULL) {
            WLSTRIDE_CONTENTS (new_stride)
              = MakeWLgrid (0, WLGRID_DIM (act_grid), 0, 1, FALSE,
                            GenerateShapeStrides (WLGRID_DIM (act_grid) + 1, dims, shape),
                            NULL, NULL);

            if (last_compl_grid != NULL) {
                /*
                 * duplicate 'compl_stride' from root til 'last_compl_grid'.
                 */
                next_dim = WLGRID_NEXTDIM (last_compl_grid);
                WLGRID_NEXTDIM (last_compl_grid) = NULL;
                dup_strides = DupNode (compl_stride);
                WLGRID_NEXTDIM (last_compl_grid) = next_dim;
                /*
                 * go to duplicated 'last_compl_grid'
                 */
                last_dup_grid = WLSTRIDE_CONTENTS (dup_strides);
                while (WLGRID_NEXTDIM (last_dup_grid) != NULL) {
                    last_dup_grid = WLSTRIDE_CONTENTS (WLGRID_NEXTDIM (last_dup_grid));
                }
            }

            /*
             * append new stride/grid to duplicated 'compl_stride'
             */
            if (last_compl_grid != NULL) {
                WLGRID_NEXTDIM (last_dup_grid) = new_stride;
            } else {
                dup_strides = new_stride;
            }

            /*
             * insert 'dup_strides' into 'new_strides'
             */
            new_strides = InsertWLnodes (new_strides, dup_strides);
        }

        /*
         * is the grid incomplete?
         */
        lb = WLGRID_BOUND2 (act_grid) - WLGRID_BOUND1 (act_grid);
        act_grid = FillGapSucc (&new_grid, act_grid, N_WLgrid, &lb, N_WLstride,
                                &(WLSTRIDE_STEP (act_stride)), FALSE);
        if (new_grid != NULL) {
            WLGRID_NEXTDIM (new_grid) = DupNode (WLGRID_NEXTDIM (act_compl_grid));
        }

        /*
         * next dim
         */
        act_stride = WLGRID_NEXTDIM (act_grid);
        act_compl_stride = WLGRID_NEXTDIM (act_compl_grid);
        last_compl_grid = act_compl_grid;
    }

    /*
     * insert completed stride/grid into 'new_strides'
     */
    new_strides = InsertWLnodes (new_strides, stride);

    /*
     * the copy of 'strides' is useless now
     */
    compl_stride = FreeTree (compl_stride);

    DBUG_RETURN (new_strides);
}

/******************************************************************************
 *
 * Function:
 *   node *GenerateCompleteDomainVar( node *stride, int dims, shpseg *shape)
 *
 * Description:
 *   Supplements strides/grids for the complement of N_WLstrideVar node
 *   'stride'.
 *
 *   For variable strides we do not call 'SplitWL()', 'MergeWL()', 'OptWL()',
 *   ... therefore we must create optimized and merged strides/grids.
 *   (This means, we actually do not create a cube!!!)
 *   Example (with shape [10,10]):
 *
 *       5 -> 10 step[0] 2
 *                  0 -> 1: a -> b  step[1] 1
 *                                     0 -> 1: op
 *
 *     is converted into
 *
 *       0 ->  5 step[0] 1
 *                  0 -> 1: 0 -> 10 step[1] 1
 *                                     0 -> 1: init/copy
 *       5 -> 10 step[0] 2
 *                  0 -> 1: 0 -> a  step[1] 1
 *                                     0 -> 1: init/copy
 *                          a -> b  step[1] 1
 *                                     0 -> 1: op
 *                          b -> 10 step[1] 1
 *                                     0 -> 1: init/copy
 *                  1 -> 2: 0 -> 10 step[1] 1
 *                                     0 -> 1: init/copy
 *
 *   This function is called by 'ComputeOneCube'.
 *
 ******************************************************************************/

static node *
GenerateCompleteDomainVar (node *stride, int dims, shpseg *shape)
{
    node *grid;
    int lb, shp_idx;
    node *new_node;

    DBUG_ENTER ("GenerateCompleteDomainVar");

    if (stride != NULL) {
        grid = WLSTRIDEX_CONTENTS (stride);
        /*
         * CAUTION: the grid can be a N_WLgrid *or* N_WLgridVar node!!
         */

        /*
         * complete grid if needed
         */
        grid = FillGapSucc (&new_node, grid, NODE_TYPE (grid),
                            WLGRIDX_GET_ADDR (grid, BOUND2), NODE_TYPE (stride),
                            WLSTRIDEX_GET_ADDR (stride, STEP), FALSE);
        if (new_node != NULL) {
            WLGRIDX_NEXTDIM (new_node)
              = GenerateShapeStrides (WLGRID_DIM (grid) + 1, dims, shape);
        }

        /*
         * next dim
         */
        WLGRIDX_NEXTDIM (grid)
          = GenerateCompleteDomainVar (WLGRIDX_NEXTDIM (grid), dims, shape);

        /*
         * append lower part of complement
         */
        shp_idx = GET_SHAPE_IDX (SHPSEG_ELEMS (shape), WLSTRIDEX_DIM (stride));
        stride = FillGapSucc (&new_node, stride, NODE_TYPE (stride),
                              WLSTRIDEX_GET_ADDR (stride, BOUND2), N_WLstride, &shp_idx,
                              FALSE);
        if (new_node != NULL) {
            WLSTRIDEX_CONTENTS (new_node)
              = MakeWLgrid (0, WLSTRIDEX_DIM (stride), 0, 1, FALSE,
                            GenerateShapeStrides (WLSTRIDEX_DIM (stride) + 1, dims,
                                                  shape),
                            NULL, NULL);
        }

        /*
         * insert upper part of complement
         */
        lb = 0;
        stride = FillGapPred (&new_node, stride, N_WLstride, &lb, NODE_TYPE (stride),
                              WLSTRIDEX_GET_ADDR (stride, BOUND1), FALSE);
        if (new_node != NULL) {
            WLSTRIDEX_CONTENTS (new_node)
              = MakeWLgrid (0, WLSTRIDEX_DIM (stride), 0, 1, FALSE,
                            GenerateShapeStrides (WLSTRIDEX_DIM (stride) + 1, dims,
                                                  shape),
                            NULL, NULL);
        }
    }

    DBUG_RETURN (stride);
}

/******************************************************************************
 *
 * Function:
 *   node *BuildCubes( node *strides, node *wl2, int dims, shpseg *shape,
 *                     bool *do_naive_comp)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
BuildCubes (node *strides, node *wl2, int dims, shpseg *shape, bool *do_naive_comp)
{
    bool all_const;
    node *cubes = NULL;

    DBUG_ENTER ("BuildCubes");

    all_const = AllStridesAreConstant (strides, TRUE, TRUE);
    DBUG_EXECUTE ("WLtrans", NOTE ((all_const ? "  constant-bounds with-loop: TRUE"
                                              : "  constant-bounds with-loop: FALSE"));
                  NOTE (((shape != NULL) ? "  known-shape with-loop: TRUE"
                                         : "  known-shape with-loop: FALSE (dim = %d)",
                         dims));
                  NOTE (((WLSTRIDEX_NEXT (strides) != NULL)
                           ? "  multi-generator with-loop: TRUE"
                           : "  multi-generator with-loop: FALSE")););

    if (WLSTRIDEX_NEXT (strides) == NULL) {
        /*
         * we have a single stride (generator)
         *  -> the index-range of the stride is possibly a *proper* subset
         *     of the index-vector-space.
         */
        if (!NWITH2_IS_FOLD (wl2)) {
            /* no fold with-loop  ->  add missing indices (init/copy) */
            if (all_const && (shape != NULL)) {
                cubes = GenerateCompleteDomain (strides, dims, shape);
            } else {
                cubes = GenerateCompleteDomainVar (strides, dims, shape);

                /*
                 * the generated cubes are already splitted and merged
                 *  -> no naive compilation possible anymore ...
                 */
                if (*do_naive_comp) {
                    *do_naive_comp = FALSE;
                    WARN (line, ("wlcomp-pragma function Naive() ignored for"
                                 " single-generator with-loops"));
                }
            }
        } else {
            /* fold with-loop  ->  the missing indices represent 'noop' */
            cubes = strides;
        }
    } else {
        if (!all_const) {
            /*
             * multiple strides, which are not all constant
             *  -> just naive compilation possible for the time being :-(
             */
            *do_naive_comp = TRUE;
            WARN (line, ("naive compilation of multi-generator with-loop activated"));

            cubes = strides;
        } else {
            if (*do_naive_comp) {
                /*
                 * this is a trick in order to put each stride in a separate
                 * segment later on
                 */
                cubes = strides;
            } else {
                strides = NormalizeAllStrides (strides);
                cubes = ComputeCubes (strides);
            }
        }
    }

    DBUG_RETURN (cubes);
}

/**
 **
 **  functions for BuildCubes()
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 *
 * Function:
 *   node *SetSegs( node *pragma, node *cubes, int dims, bool fold_float)
 *
 * Description:
 *   returns chain of segments (based on the calculated cubes 'cubes')
 *
 ******************************************************************************/

static node *
SetSegs (node *pragma, node *cubes, int dims, bool fold_float)
{
    node *aps;
    node *segs;
    char *fun_names;

    DBUG_ENTER ("SetSegs");

    /*
     * create default configuration.
     * also possible:
     *   segs = WLCOMP_Cubes( NULL, NULL, cubes, dims, line);
     */
    segs = WLCOMP_All (NULL, NULL, cubes, dims, line);

    /*
     * create pragma-dependent configuration
     */
    if (pragma != NULL) {
        aps = PRAGMA_WLCOMP_APS (pragma);
        while (aps != NULL) {

#define WLP(fun, str, ieee)                                                              \
    if (strcmp (AP_NAME (EXPRS_EXPR (aps)), str) == 0) {                                 \
        if ((!fold_float) || (!enforce_ieee) || ieee) {                                  \
            segs = fun (segs, AP_ARGS (EXPRS_EXPR (aps)), cubes, dims, line);            \
        } else {                                                                         \
            WARN (line, ("Function %s of wlcomp-pragma ignored in order"                 \
                         " to meet the IEEE-754 standard",                               \
                         AP_NAME (EXPRS_EXPR (aps))));                                   \
        }                                                                                \
    } else
#include "wlpragma_funs.mac"
#undef WLP
            {
                fun_names =
#define WLP(fun, str, ieee) str
#include "wlpragma_funs.mac"
#undef WLP
                  ;

                WARN (line, ("Illegal function name %s in wlcomp-pragma found."
                             " Currently supported functions are:%s",
                             AP_NAME (EXPRS_EXPR (aps)), fun_names));
            }

            aps = EXPRS_NEXT (aps);
        }
    }

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * Function:
 *   void CheckParams( node *seg)
 *
 * Description:
 *   checks whether the parameter of the segment 'seg' are legal.
 *
 ******************************************************************************/

static void
CheckParams (node *seg)
{
    int last, first_block, d, b;

    DBUG_ENTER ("CheckParams");

    if (NODE_TYPE (seg) == N_WLseg) {

        DBUG_ASSERT ((WLSEG_IDX_MIN (seg) != NULL), "WLSEG_IDX_MIN not found!");
        DBUG_ASSERT ((WLSEG_IDX_MAX (seg) != NULL), "WLSEG_IDX_MAX not found!");

        DBUG_ASSERT ((WLSEG_SV (seg) != NULL), "WLSEG_SV not found!");
        DBUG_ASSERT ((WLSEG_UBV (seg) != NULL), "WLSEG_UBV not found!");
        for (b = 0; b < WLSEG_BLOCKS (seg); b++) {
            DBUG_ASSERT ((WLSEG_BV (seg, b) != NULL), "WLSEG_BV not found!");
        }

        for (d = 0; d < WLSEG_DIMS (seg); d++) {
            DBUG_ASSERT (((WLSEG_SV (seg))[d] >= 1), "illegal WLSEG_SV value found!");
        }

        /* check whether (bv0 >= bv1 >= bv2 >= ... >= 1), (ubv >= 1) */
        for (d = 0; d < WLSEG_DIMS (seg); d++) {
            b = WLSEG_BLOCKS (seg) - 1;
            if ((WLSEG_BV (seg, b))[d] < 1) {
                ABORT (line, ("Blocking step (%i) is smaller than 1."
                              " Please check parameters of functions in wlcomp-pragma",
                              (WLSEG_BV (seg, b))[d]));
            }
            last = (WLSEG_BV (seg, b))[d];
            for (; b >= 0; b--) {
                if ((WLSEG_BV (seg, b))[d] < last) {
                    ABORT (line, ("Inner Blocking step (%i) is smaller than outer one"
                                  " (%i). Please check parameters of functions in"
                                  " wlcomp-pragma",
                                  (WLSEG_BV (seg, b))[d], last));
                }
                last = (WLSEG_BV (seg, b))[d];
            }

            if ((WLSEG_UBV (seg))[d] < 1) {
                ABORT (line, ("Unrolling-blocking step (%i) is smaller than 1."
                              " Please check parameters of functions in wlcomp-pragma",
                              (WLSEG_UBV (seg))[d]));
            }
        }

        /*
         * check bv:
         *
         * checks for all bv (bv0, bv1, bv2, ...):
         *  exists k: (forall (d < k): bv_d = 1) and
         *            (forall (d >= k): bv_d >= max(sv_b, ubv_b))
         */
        first_block = 0;
        for (b = 0; b < WLSEG_BLOCKS (seg); b++) {
            /* goto first dim with (bv_d > 1) */
            d = 0;
            while ((d < WLSEG_DIMS (seg)) && ((WLSEG_BV (seg, b))[d] == 1)) {
                d++;
            }

            if (d < WLSEG_DIMS (seg)) {
                first_block = d;
            }
            for (; d < WLSEG_DIMS (seg); d++) {
                if ((WLSEG_BV (seg, b))[d]
                    < MAX ((WLSEG_SV (seg))[d], (WLSEG_UBV (seg)[d]))) {
                    ABORT (line, ("Blocking step (%i) is greater than 1 but smaller than"
                                  " stride step (%i) or unrolling-blocking step (%i)"
                                  " respectively. "
                                  "Please check parameters of functions in wlcomp-pragma",
                                  (WLSEG_BV (seg, b))[d], (WLSEG_SV (seg))[d],
                                  (WLSEG_UBV (seg))[d]));
                }
            }
        }

        /*
         * check ubv:
         *
         * checks for ubv:
         *  - exists k: (forall (d < k): ubv_d = 1) and
         *              (forall (d >= k): sv_d | ubv_d)
         *  - ubv <= bv, for most inner bv with (bv != 1)
         *    (we must prevent e.g. bv = (1,40), ubv = (2,2), sv = (2,2),
         *     but this is allowed: bv = (1,1),  ubv = (2,2), sv = (2,2))
         */

        /* goto first dim with (ubv_d > 1) */
        d = 0;
        while ((d < WLSEG_DIMS (seg)) && ((WLSEG_UBV (seg))[d] == 1)) {
            d++;
        }

        if (first_block > d) {
            ABORT (line, ("Unrolling-blocking step (%i) is greater than"
                          " most inner blocking step (%i). "
                          "Please check parameters of functions in wlcomp-pragma",
                          (WLSEG_UBV (seg))[d], (WLSEG_BV (seg, b))[first_block]));
        }

        for (; d < WLSEG_DIMS (seg); d++) {
            if ((WLSEG_UBV (seg))[d] % (WLSEG_SV (seg))[d] != 0) {
                ABORT (line, ("Unrolling-blocking step (%i) is not a multiple of"
                              " stride step (%i). "
                              "Please check parameters of functions in wlcomp-pragma",
                              (WLSEG_UBV (seg))[d], (WLSEG_SV (seg))[d]));
            }
        }
    } else {
        DBUG_ASSERT ((WLSEGVAR_IDX_MIN (seg) != NULL), "WLSEGVAR_IDX_MIN not found!");
        DBUG_ASSERT ((WLSEGVAR_IDX_MAX (seg) != NULL), "WLSEGVAR_IDX_MAX not found!");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 ******************************************************************************
 **
 **  functions for SplitWL()
 **
 **/

/******************************************************************************
 *
 * Function:
 *   node *NewBoundsStride( node *stride, int dim,
 *                          int new_bound1, int new_bound2)
 *
 * Description:
 *   returns modified 'stride':
 *     all strides in dimension ("current dimension" + 'dim') are new bounds
 *     given ('bound1', 'bound2').
 *
 ******************************************************************************/

static node *
NewBoundsStride (node *stride, int dim, int new_bound1, int new_bound2)
{
    node *grids, *new_grids, *grid, *grid2;
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

                /* extract current grid from chain -> single grid in 'grid' */
                grid = grids;
                grids = WLGRID_NEXT (grids);
                WLGRID_NEXT (grid) = NULL;

                if (offset <= grid_b1) {
                    /*
                     * grid is still in one pice :)
                     */

                    WLGRID_BOUND1 (grid) = grid_b1 - offset;
                    WLGRID_BOUND2 (grid) = grid_b2 - offset;

                    /* insert changed grid into 'new_grids' */
                    new_grids = InsertWLnodes (new_grids, grid);
                } else {
                    /*
                     * the grid is split into two parts :(
                     */

                    /* first part: recycle old grid */
                    WLGRID_BOUND1 (grid) = grid_b1 - offset + step;
                    WLGRID_BOUND2 (grid) = step;
                    /* second part: duplicate old grid first */
                    grid2 = DupNode (grid);
                    WLGRID_BOUND1 (grid2) = 0;
                    WLGRID_BOUND2 (grid2) = grid_b2 - offset;
                    /* concate the two grids */
                    WLGRID_NEXT (grid2) = grid;

                    /* insert them into 'new_grids' */
                    new_grids = InsertWLnodes (new_grids, grid2);
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
 * Function:
 *   void SplitStride( node *stride1, node *stride2
 *                     node **s_stride1, node **s_stride2)
 *
 * Description:
 *   returns in 's_stride1', 's_stride2' the splitted stride 'stride1',
 *     'stride2' respectively.
 *   returns NULL if there is nothing to split.
 *
 ******************************************************************************/

static void
SplitStride (node *stride1, node *stride2, node **s_stride1, node **s_stride2)
{
    node *tmp1, *tmp2;
    int i_bound1, i_bound2, dim;

    DBUG_ENTER ("SplitStride");

    tmp1 = stride1;
    tmp2 = stride2;

    *s_stride1 = *s_stride2 = NULL;

    DBUG_ASSERT (((NODE_TYPE (stride1) == N_WLstride)
                  && (NODE_TYPE (stride2) == N_WLstride)),
                 "no N_WLstride nodes found");

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

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   node *SplitWL( node *strides)
 *
 * Description:
 *   returns the splitted stride-tree 'strides'.
 *
 ******************************************************************************/

static node *
SplitWL (node *strides)
{
    node *stride1, *stride2, *split_stride1, *split_stride2, *new_strides, *tmp;
    bool fixpoint;

    DBUG_ENTER ("SplitWL");

    if (strides != NULL) {
        /*
         * the outline of each stride is intersected with all the other ones.
         * this is done until no new intersections are generated (fixpoint).
         */
        do {
            DBUG_ASSERT ((NODE_TYPE (strides) == N_WLstride),
                         "SplitWL() for var. segments not implemented yet!");

            fixpoint = TRUE;    /* initialize 'fixpoint' */
            new_strides = NULL; /* here we collect the new stride-set */

            /* check WLSTRIDE_MODIFIED */
            stride1 = strides;
            while (stride1 != NULL) {
                DBUG_ASSERT ((WLSTRIDE_MODIFIED (stride1) == NULL),
                             "stride was modified");
                stride1 = WLSTRIDE_NEXT (stride1);
            }

            /*
             * split pairwise
             */
            stride1 = strides;
            while (stride1 != NULL) {
                stride2 = WLSTRIDE_NEXT (stride1);
                while (stride2 != NULL) {
                    SplitStride (stride1, stride2, &split_stride1, &split_stride2);
                    if (split_stride1 != NULL) {
                        DBUG_ASSERT ((split_stride2 != NULL), "wrong splitting");
                        fixpoint = FALSE; /* no, not a fixpoint yet :( */
                        WLSTRIDE_MODIFIED (stride1) = WLSTRIDE_MODIFIED (stride2)
                          = stride1;
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
                if (WLSTRIDE_MODIFIED (stride1) == NULL) {
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
    }

    DBUG_RETURN (strides);
}

/**
 **
 **  functions for SplitWL()
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 ******************************************************************************
 **
 **  functions for BlockWL()
 **
 **/

/******************************************************************************
 *
 * Function:
 *   node *BlockStride( node *stride, int *bv, bool unroll)
 *
 * Description:
 *   returns 'stride' with corrected bounds, blocking levels and
 *     unrolling-flag.
 *   this function is needed after a blocking.
 *
 ******************************************************************************/

static node *
BlockStride (node *stride, int *bv, bool unroll)
{
    node *curr_stride, *curr_grid, *grids;

    DBUG_ENTER ("BlockStride");

    if (stride != NULL) {

        DBUG_ASSERT ((NODE_TYPE (stride) == N_WLstride), "no N_WLstride node found");

        curr_stride = stride;
        do {
            /* correct blocking level and unrolling-flag */
            WLSTRIDE_LEVEL (curr_stride)++;
            WLSTRIDE_UNROLLING (curr_stride) = unroll;
            grids = WLSTRIDE_CONTENTS (curr_stride);
            do {
                WLGRID_LEVEL (grids)++;
                WLGRID_UNROLLING (grids) = unroll;
                grids = WLGRID_NEXT (grids);
            } while (grids != NULL);

            /* fit bounds of stride to blocking step */
            WLSTRIDE_BOUND1 (curr_stride) = 0;
            WLSTRIDE_BOUND2 (curr_stride) = bv[WLSTRIDE_DIM (curr_stride)];

            /*
             * involve all grids of current dimension
             */
            curr_grid = WLSTRIDE_CONTENTS (curr_stride);
            do {
                WLGRID_NEXTDIM (curr_grid)
                  = BlockStride (WLGRID_NEXTDIM (curr_grid), bv, unroll);
                curr_grid = WLGRID_NEXT (curr_grid);
            } while (curr_grid != NULL);

            curr_stride = WLSTRIDE_NEXT (curr_stride);
        } while (curr_stride != NULL);
    }

    DBUG_RETURN (stride);
}

/******************************************************************************
 *
 * Function:
 *   node *BlockWL( node *stride, int dims, int *bv, bool unroll)
 *
 * Description:
 *   returns with blocking-vector 'bv' blocked 'stride'.
 *   'dims' is the number of dimensions in 'stride'.
 *
 *   when called multiple times in a row, this function even realizes
 *     hierarchical blocking!! (top-down: coarse blocking first!)
 *
 *   ('unroll' > 0): performs unrolling-blocking --- allowed only once after
 *     all the convential blocking!
 *
 ******************************************************************************/

static node *
BlockWL (node *stride, int dims, int *bv, bool unroll)
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
             *
             * unrolling-blocking is allowed only once after all conventional
             *  blocking!!
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
                             "wrong bv-value found");
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
                                     "no stride found");
                        DBUG_ASSERT (((d == WLSTRIDE_DIM (curr_stride))
                                      || (WLSTRIDE_NEXT (contents) == NULL)),
                                     "more than one stride found");

                        /*
                         * unrolling-blocking wanted?
                         */
                        if (unroll > 0) {
                            block = MakeWLublock (level, WLSTRIDE_DIM (contents),
                                                  WLSTRIDE_BOUND1 (contents),
                                                  WLSTRIDE_BOUND2 (contents),
                                                  bv[WLSTRIDE_DIM (contents)], NULL, NULL,
                                                  NULL);
                        } else {
                            block = MakeWLblock (level, WLSTRIDE_DIM (contents),
                                                 WLSTRIDE_BOUND1 (contents),
                                                 WLSTRIDE_BOUND2 (contents),
                                                 bv[WLSTRIDE_DIM (contents)], NULL, NULL,
                                                 NULL);
                        }

                        if (lastdim != NULL) {
                            /*
                             * not first blocking dim
                             *  -> append at block node of last dim
                             */
                            WLBLOCK_NEXTDIM (lastdim) = block;
                        } else {
                            /*
                             * current dim is first blocking dim
                             */
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
                      = BlockStride (WLBLOCK_CONTENTS (lastdim), bv, unroll);
                }
            }
            break;

        default:
            DBUG_ASSERT ((0), "wrong node type");
            break;
        }
    }

    DBUG_RETURN (stride);
}

/**
 **
 **  functions for BlockWL()
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 ******************************************************************************
 **
 **  functions for MergeWL()
 **
 **/

/******************************************************************************
 *
 * Function:
 *   node *NewStepGrids( node *grids, int step, int new_step, int offset)
 *
 * Description:
 *   returns a modified 'grids' chain:
 *     - the bounds of the grids are modified (relating to 'offset')
 *     - the step of the grids is now 'step'
 *        -> possibly the grids must be duplicated
 *
 ******************************************************************************/

static node *
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
 * Function:
 *   node *IntersectGrid( node *grid1, node *grid2, int step,
 *                        node **i_grid1, node **i_grid2)
 *
 * Description:
 *   returns in 'i_grid1', 'i_grid2' the intersection of 'grid1' and 'grid2'.
 *   both grids must have the same step ('step').
 *
 *   returns NULL if the intersection is equal to the original grid!!
 *
 ******************************************************************************/

static void
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

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   node *MergeWL( node *nodes)
 *
 * Description:
 *   returns the merged chain 'nodes'.
 *   if necessary (e.g. if called from 'ComputeCubes') the bounds of the
 *     chain-elements are adjusted.
 *
 ******************************************************************************/

static node *
MergeWL (node *nodes)
{
    node *node1, *grids, *new_grids, *grid1, *grid2, *i_grid1, *i_grid2, *tmp;
    int bound1, bound2, step, rear1, count, i;
    bool fixpoint;

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
                fixpoint = TRUE;
                new_grids = NULL;

                /* check WLGRID_MODIFIED */
                grid1 = grids;
                while (grid1 != NULL) {
                    DBUG_ASSERT ((WLGRID_MODIFIED (grid1) == NULL), "grid was modified");
                    grid1 = WLGRID_NEXT (grid1);
                }

                grid1 = grids;
                while (grid1 != NULL) {
                    grid2 = WLGRID_NEXT (grid1);
                    while (grid2 != NULL) {
                        IntersectGrid (grid1, grid2, step, &i_grid1, &i_grid2);
                        if (i_grid1 != NULL) {
                            new_grids = InsertWLnodes (new_grids, i_grid1);
                            WLGRID_MODIFIED (grid1) = new_grids;
                            fixpoint = FALSE;
                        }
                        if (i_grid2 != NULL) {
                            new_grids = InsertWLnodes (new_grids, i_grid2);
                            WLGRID_MODIFIED (grid2) = new_grids;
                            fixpoint = FALSE;
                        }
                        grid2 = WLGRID_NEXT (grid2);
                    }

                    /* was 'grid1' not modified? */
                    if (WLGRID_MODIFIED (grid1) == NULL) {
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
            break;
        }

        node1 = WLNODE_NEXT (node1);
    }

    DBUG_RETURN (nodes);
}

/**
 **
 **  functions for MergeWL()
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 ******************************************************************************
 **
 **  functions for OptWL()
 **
 **/

/******************************************************************************
 *
 * Function:
 *   bool CompareWLtrees( node *tree1, node *tree2)
 *
 * Description:
 *   returns 1 if the N_WL...-trees 'tree1' and 'tree2' are equal.
 *   returns 0 otherwise.
 *
 *   remark: we can not use CompareWLnodes() here, because that function only
 *           compares the first level of dimensions.
 *           In contrast here we must compare the *whole* trees --- all block
 *           levels, and even the code ...
 *
 ******************************************************************************/

static bool
CompareWLtrees (node *tree1, node *tree2)
{
    node *tmp1, *tmp2;
    bool equal = TRUE;

    DBUG_ENTER ("CompareWLtrees");

    if ((tree1 != NULL) && (tree2 != NULL)) {

        DBUG_ASSERT ((NODE_TYPE (tree1) == NODE_TYPE (tree2)),
                     "can not compare objects of different type");

        /*
         * compare the whole chains
         */
        tmp1 = tree1;
        tmp2 = tree2;
        do {
            DBUG_ASSERT ((tmp2 != NULL), "trees differ in length");

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
                    if (WLNODE_NEXTDIM (tmp1) != NULL) {
                        /*
                         * compare NEXTDIM (CONTENTS is NULL)
                         */
                        DBUG_ASSERT ((WLXBLOCK_CONTENTS (tmp1) == NULL),
                                     "data in NEXTDIM *and* CONTENTS found");
                        DBUG_ASSERT ((WLXBLOCK_CONTENTS (tmp2) == NULL),
                                     "data in NEXTDIM *and* CONTENTS found");
                        equal = CompareWLtrees (WLXBLOCK_NEXTDIM (tmp1),
                                                WLXBLOCK_NEXTDIM (tmp2));
                    } else {
                        /*
                         * compare CONTENTS (NEXTDIM is NULL)
                         */
                        equal = CompareWLtrees (WLXBLOCK_CONTENTS (tmp1),
                                                WLXBLOCK_CONTENTS (tmp2));
                    }
                    break;

                case N_WLstride:
                    equal = CompareWLtrees (WLSTRIDE_CONTENTS (tmp1),
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
                          = CompareWLtrees (WLGRID_NEXTDIM (tmp1), WLGRID_NEXTDIM (tmp2));
                    } else {
                        /*
                         * compare CODE (NEXTDIM is NULL)
                         */
                        equal = (WLGRID_CODE (tmp1) == WLGRID_CODE (tmp2));
                    }
                    break;

                default:
                    DBUG_ASSERT ((0), "wrong node type");
                    break;
                }
            } else {
                equal = FALSE;
            }

            tmp1 = WLNODE_NEXT (tmp1);
            tmp2 = WLNODE_NEXT (tmp2);
        } while (equal && (tmp1 != NULL));

    } else {
        equal = ((tree1 == NULL) && (tree2 == NULL));
    }

    DBUG_RETURN (equal);
}

/******************************************************************************
 *
 * Function:
 *   node *OptWL( node *nodes)
 *
 * Description:
 *   Returns the optimized N_WL...-tree 'nodes'.
 *
 ******************************************************************************/

static node *
OptWL (node *nodes)
{
    node *next;
    node *cont1, *cont2;
    node *nextdim1, *nextdim2;

    DBUG_ENTER ("OptWL");

    if (nodes != NULL) {
        /*
         * optimize the next node
         */
        next = WLNODE_NEXT (nodes) = OptWL (WLNODE_NEXT (nodes));
        cont2 = nextdim2 = NULL;

        /*
         * optimize the type-specific sons
         *
         * save in 'cont?', 'nextdim?' the sons of 'nodes', 'next' respectively.
         */
        switch (NODE_TYPE (nodes)) {
        case N_WLblock:
            /* here is no break missing! */
        case N_WLublock:
            cont1 = WLXBLOCK_CONTENTS (nodes) = OptWL (WLXBLOCK_CONTENTS (nodes));
            nextdim1 = WLXBLOCK_NEXTDIM (nodes) = OptWL (WLXBLOCK_NEXTDIM (nodes));
            if (next != NULL) {
                cont2 = WLXBLOCK_CONTENTS (next);
                nextdim2 = WLXBLOCK_NEXTDIM (next);
            }
            break;

        case N_WLstride:
            cont1 = WLSTRIDE_CONTENTS (nodes) = OptWL (WLSTRIDE_CONTENTS (nodes));
            nextdim1 = NULL;
            if (next != NULL) {
                cont2 = WLSTRIDE_CONTENTS (next);
            }

            /*
             * if the grids contained in the stride have an offset
             * (the first grid does not begin at index 0), remove this offset.
             */
            nodes = NormalizeGrids (nodes);

            /*
             * if the first (and only) grid fills the whole step range
             *   set upper bound of this grid and step to 1
             */
            if ((WLGRID_BOUND1 (cont1) == 0)
                && (WLGRID_BOUND2 (cont1) == WLSTRIDE_STEP (nodes))) {
                WLGRID_BOUND2 (cont1) = WLSTRIDE_STEP (nodes) = 1;
            }
            break;

        case N_WLgrid:
            cont1 = WLGRID_CODE (nodes);
            nextdim1 = WLGRID_NEXTDIM (nodes) = OptWL (WLGRID_NEXTDIM (nodes));
            if (next != NULL) {
                cont2 = WLGRID_CODE (next);
                nextdim2 = WLGRID_NEXTDIM (next);
            }
            break;

        default:
            cont1 = nextdim1 = NULL;
            DBUG_ASSERT ((0), "wrong node type");
            break;
        }

        /*
         * if 'cont1'/'cont2' and 'nextdim1'/'nextdim2' are equal subtrees
         *   we can concate 'nodes' and 'next'
         */
        if (next != NULL) {
            if ((WLNODE_STEP (nodes) == WLNODE_STEP (next))
                && ((NODE_TYPE (nodes) != N_WLstride) ||
                    /*
                     * For strides we must check whether the grids are compatible or not.
                     * The following strides CAN NOT be concatenated:
                     *    0->5  step 3:
                     *            0->1: R1
                     *            1->3: R2
                     *    5->10 step 3:
                     *            0->1: R1
                     *            1->3: R2
                     * The following strides CAN be concatenated:
                     *    0->6  step 3:
                     *            0->1: R1
                     *            1->3: R2
                     *    6->10 step 3:
                     *            0->1: R1
                     *            1->3: R2
                     * The following strides COULD be concatenated but the algorithm
                     * does not detect this yet:
                     *    0->5  step 3:
                     *            0->1: R1
                     *            1->3: R2
                     *    5->10 step 3:
                     *            0->1: R2
                     *            1->2: R1
                     *            2->3: R2
                     */
                    (((WLNODE_BOUND2 (nodes) - WLNODE_BOUND1 (nodes))
                      % WLNODE_STEP (nodes))
                     == 0))
                && (WLNODE_BOUND2 (nodes) == WLNODE_BOUND1 (next))) {
                if ((((cont1 != NULL) && (NODE_TYPE (cont1) != N_Ncode))
                       ? CompareWLtrees (cont1, cont2)
                       : (cont1 == cont2))
                    && (CompareWLtrees (nextdim1, nextdim2))) {
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

/**
 **
 **  functions for OptWL()
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 ******************************************************************************
 **
 **  functions for FitWL()
 **
 **/

/******************************************************************************
 *
 * Function:
 *   int AdjustBlockSize( int old_bv, int unroll, bool warn)
 *
 * Description:
 *
 *
 ******************************************************************************/

static int
AdjustBlockSize (int old_bv, int unroll, bool warn)
{
    int mod, new_bv;

    DBUG_ENTER ("AdjustBlockSize");

    mod = old_bv % unroll;
    if ((old_bv > 1) && (mod != 0)) {
        if (mod <= unroll / 2) {
            new_bv = old_bv - mod;
        } else {
            new_bv = old_bv + unroll - mod;
        }

        DBUG_ASSERT ((new_bv % unroll == 0), "adjustment of block size wrong!");
    } else {
        new_bv = old_bv;
    }

    if (warn && (old_bv != new_bv)) {
        WARN (line, ("Block size adjusted: %i instead of %i", new_bv, old_bv));
    }

    DBUG_RETURN (new_bv);
}

/******************************************************************************
 *
 * Function:
 *   node *FitNode( node *wlnode, int unroll)
 *
 * Description:
 *   Checks whether the extent (difference of upper and lower bound) of
 *   'wlnode' is a multiple of 'unroll'. If not, the incomplete periode at the
 *   tail is split.
 *
 ******************************************************************************/

static node *
FitNode (node *wlnode, int unroll)
{
    int bnd1, bnd2, width, remain;
    node *new_wlnode;

    DBUG_ENTER ("FitNode");

    DBUG_ASSERT ((wlnode != NULL), "no node found!");

    if (unroll > 0) {
        NodeOrInt_GetNameOrVal (NULL, &bnd1, NODE_TYPE (wlnode),
                                WLNODE_GET_ADDR (wlnode, BOUND1));

        NodeOrInt_GetNameOrVal (NULL, &bnd2, NODE_TYPE (wlnode),
                                WLNODE_GET_ADDR (wlnode, BOUND2));

        if ((bnd1 >= 0) && (bnd2 >= 0)) {
            width = bnd2 - bnd1;
            remain = width % unroll;
            if ((remain > 0) && (width > remain)) {
                /*
                 * incomplete periode found -> split
                 */
                new_wlnode = DupNode (wlnode);

                DBUG_ASSERT (((NODE_TYPE (wlnode) != N_WLstrideVar)
                              && (NODE_TYPE (wlnode) != N_WLgridVar)),
                             "illegal node found!");

                WLNODE_BOUND2 (wlnode) = WLNODE_BOUND1 (new_wlnode)
                  = WLNODE_BOUND2 (wlnode) - remain;

                WLNODE_NEXT (new_wlnode) = WLNODE_NEXT (wlnode);
                WLNODE_NEXT (wlnode) = new_wlnode;
            }
        }
    }

    DBUG_RETURN (wlnode);
}

/******************************************************************************
 *
 * Function:
 *   node *FitWL( node *wlnode)
 *
 * Description:
 *   Returns the fitted N_WL...-tree 'nodes'.
 *
 ******************************************************************************/

static node *
FitWL (node *wlnode)
{
    node *grids;
    int unroll;

    DBUG_ENTER ("FitWL");

    if (wlnode != NULL) {
        switch (NODE_TYPE (wlnode)) {
        case N_WLblock:
            if (WLBLOCK_NEXTDIM (wlnode) != NULL) {
                /*
                 * fit in next dimension; compute unrolling information
                 */
                DBUG_ASSERT ((WLBLOCK_CONTENTS (wlnode) == NULL),
                             "Sons CONTENTS and NEXTDIM of WLblock are used "
                             "simultaneous!");

                WLBLOCK_NEXTDIM (wlnode) = FitWL (WLBLOCK_NEXTDIM (wlnode));

                unroll
                  = GetLcmUnroll (WLBLOCK_NEXTDIM (wlnode), WLBLOCK_DIM (wlnode), FALSE);
            } else {
                /*
                 * fit contents of block; compute unrolling information
                 */
                DBUG_ASSERT ((WLBLOCK_NEXTDIM (wlnode) == NULL), "Sons CONTENTS and "
                                                                 "NEXTDIM of WLblock are "
                                                                 "used simultaneous!");

                WLBLOCK_CONTENTS (wlnode) = FitWL (WLBLOCK_CONTENTS (wlnode));

                unroll
                  = GetLcmUnroll (WLBLOCK_CONTENTS (wlnode), WLBLOCK_DIM (wlnode), FALSE);
            }

            /*
             * adjust block size
             * (block size must be a multiple of 'unroll')
             */
            WLBLOCK_STEP (wlnode) = AdjustBlockSize (WLBLOCK_STEP (wlnode), unroll, TRUE);
            /*
             * the upper bound of the related ublock/stride-nodes in the contents
             * of the block is corrected later on. (just a few lines ahead ...)
             */
            break;

        case N_WLublock:
            if (WLUBLOCK_NEXTDIM (wlnode) != NULL) {
                /*
                 * fit in next dimension
                 */
                DBUG_ASSERT ((WLUBLOCK_CONTENTS (wlnode) == NULL),
                             "Sons CONTENTS and NEXTDIM of WLublock are used "
                             "simultaneous!");

                WLUBLOCK_NEXTDIM (wlnode) = FitWL (WLUBLOCK_NEXTDIM (wlnode));
            } else {
                /*
                 * fit contents of block
                 */
                DBUG_ASSERT ((WLUBLOCK_NEXTDIM (wlnode) == NULL),
                             "Sons CONTENTS and NEXTDIM of WLublock are used "
                             "simultaneous!");

                WLUBLOCK_CONTENTS (wlnode) = FitWL (WLUBLOCK_CONTENTS (wlnode));
            }

            unroll = WLUBLOCK_STEP (wlnode);
            break;

        case N_WLstride:
            /* here is no break missing! */
        case N_WLstrideVar:
            grids = WLSTRIDEX_CONTENTS (wlnode);

            /*
             * fit for all grids in next dimension;
             */
            while (grids != NULL) {
                WLGRIDX_NEXTDIM (grids) = FitWL (WLGRIDX_NEXTDIM (grids));
                grids = WLGRIDX_NEXT (grids);
            }

            NodeOrInt_GetNameOrVal (NULL, &unroll, NODE_TYPE (wlnode),
                                    WLSTRIDEX_GET_ADDR (wlnode, STEP));
            break;

        default:
            unroll = 0;
            DBUG_ASSERT ((0), "wrong node type");
            break;
        }

        if (WLNODE_LEVEL (wlnode) == 0) { /* outer most node? */
            /*
             * Fit the outer most node of the current dimension:
             *   Split a uncompleted periode at the end of index range
             */
            wlnode = FitNode (wlnode, unroll);
        } else {
            if (NODE_TYPE (wlnode) != N_WLblock) {
                /*
                 * We have a inner ublock- or stride-node, therefore we are inside of a
                 * block or unrolling-block. That means, the lower bound must be equal
                 * to 0 and the upper bound should be a multiple of the step.
                 * If the latter is not hold, this node corresponds with a block-node
                 * whose block size had been adjusted
                 *   -> we must fathom this adjustment here!
                 */
                DBUG_ASSERT ((WLNODE_BOUND1 (wlnode) == 0),
                             "lower bound of inner node is != 0");
                WLNODE_BOUND2 (wlnode)
                  = AdjustBlockSize (WLNODE_BOUND2 (wlnode), WLNODE_STEP (wlnode), FALSE);
            }
        }

        WLNODE_NEXT (wlnode) = FitWL (WLNODE_NEXT (wlnode));
    }

    DBUG_RETURN (wlnode);
}

/**
 **
 **  functions for FitWL()
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 ******************************************************************************
 **
 **  functions for NormWL()
 **
 **/

/******************************************************************************
 *
 * Function:
 *   node *DoNormalize( node *nodes, int *width)
 *
 * Description:
 *   Returns the normalized N_WL...-tree 'nodes'.
 *   'width' is an array with one component for each dimension initially
 *     containing the supremum of the index vector set. This array is modified
 *     during calculation (the width of the index ranges is stored there).
 *
 * Remark:
 *   During normalization another fitting might be needed. Example:
 *   Before 'fitting phase':
 *     (0 -> 23), ublock0[0] 4:
 *       (0 -> 4), step1[0] 2
 *         (0 -> 1): op_1
 *         (1 -> 2): op_0
 *   After 'fitting phase':
 *     (0 -> 20), ublock0[0] 4:
 *       ...
 *     (20 -> 23), ublock0[0] 4:
 *       (0 -> 4), step1[0] 2
 *         (0 -> 1): op_1
 *         (1 -> 2): op_0
 *   After naive 'normalization' without fitting:
 *     (0 -> 20), ublock0[0] 4:
 *       ...
 *     (20 -> 23), ublock0[0] 3:
 *       (0 -> 3), step1[0] 2    // CAUTION: the stride extent (3) is ...
 *         (0 -> 1): op_1        // ... not a multiple of the step (2) anymore!
 *         (1 -> 2): op_0
 *   Another fitting is needed:
 *     (0 -> 20), ublock0[0] 4:
 *       ...
 *     (20 -> 23), ublock0[0] 3:
 *       (0 -> 2), step1[0] 2
 *         (0 -> 1): op_1
 *         (1 -> 2): op_0
 *       (2 -> 3), step1[0] 1
 *         (0 -> 1): op_1
 *
 ******************************************************************************/

static node *
DoNormalize (node *nodes, int *width)
{
    node *node;
    int curr_width;

    DBUG_ENTER ("DoNormalize");

    if (nodes != NULL) {
        /*
         * backup width of current dim
         */
        curr_width = width[WLNODE_DIM (nodes)];

        node = nodes;
        do {
            /*
             * adjust upper bound
             */
            DBUG_ASSERT ((WLNODE_BOUND1 (node) < curr_width), "wrong bounds found");
            WLNODE_BOUND2 (node) = MIN (WLNODE_BOUND2 (node), curr_width);

            /*
             * remove nodes whose index ranges lies outside the current block
             */
            while ((WLNODE_NEXT (node) != NULL)
                   && (WLNODE_BOUND1 (WLNODE_NEXT (node)) >= curr_width)) {
                WLNODE_NEXT (node) = FreeNode (WLNODE_NEXT (node));
            }

            /*
             * perform another fitting if the current node is a stride-node and
             * the extent is not a multiple of the step.
             */
            if ((NODE_TYPE (node) == N_WLstride)
                && ((WLNODE_BOUND2 (node) - WLNODE_BOUND1 (node)) % WLNODE_STEP (node)
                    != 0)) {
                node = FitNode (node, WLNODE_STEP (node));
            }

            /* take next node */
            node = WLNODE_NEXT (node);
        } while (node != NULL);

        node = nodes;
        do {
            /*
             * save width of current index range; adjust step
             */
            width[WLNODE_DIM (node)] = WLNODE_BOUND2 (node) - WLNODE_BOUND1 (node);
            WLNODE_STEP (node) = MIN (WLNODE_STEP (node), width[WLNODE_DIM (node)]);

            /*
             * normalize the type-specific sons
             */
            switch (NODE_TYPE (node)) {
            case N_WLblock:
                /* here is no break missing! */
            case N_WLublock:
                WLXBLOCK_NEXTDIM (node) = DoNormalize (WLXBLOCK_NEXTDIM (node), width);
                WLXBLOCK_CONTENTS (node) = DoNormalize (WLXBLOCK_CONTENTS (node), width);
                break;

            case N_WLstride:
                WLSTRIDE_CONTENTS (node) = DoNormalize (WLSTRIDE_CONTENTS (node), width);
                break;

            case N_WLgrid:
                WLGRID_NEXTDIM (node) = DoNormalize (WLGRID_NEXTDIM (node), width);
                break;

            default:
                DBUG_ASSERT ((0), "wrong node type");
                break;
            }

            /* take next node */
            node = WLNODE_NEXT (node);
        } while (node != NULL);

        /*
         * restore width of current dim
         */
        width[WLNODE_DIM (nodes)] = curr_width;
    }

    DBUG_RETURN (nodes);
}

/******************************************************************************
 *
 * Function:
 *   node *NormWL( int dims, shpseg *shape, int *idx_max, node *nodes)
 *
 * Description:
 *   Returns the normalized N_WL...-tree 'nodes'.
 *
 ******************************************************************************/

static node *
NormWL (int dims, shpseg *shape, int *idx_max, node *nodes)
{
    int d;
    int *width = NULL;

    DBUG_ENTER ("NormWL");

    MALLOC_VECT (width, dims, int);
    for (d = 0; d < dims; d++) {
        if (idx_max[d] == IDX_SHAPE) {
            DBUG_ASSERT ((shape != NULL), "no shape found!");
            width[d] = SHPSEG_SHAPE (shape, d);
        } else {
            width[d] = idx_max[d];
        }
    }

    nodes = DoNormalize (nodes, width);

    width = Free (width);

    DBUG_RETURN (nodes);
}

/**
 **
 **  functions for NormWL()
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 ******************************************************************************
 **
 **  functions for InsertNoopGrids(), InsertNoopNodes()
 **
 **/

/******************************************************************************
 *
 * Function:
 *   node *InsertNoopGrids( node *stride)
 *
 * Description:
 *   Inserts NOOP N_WLgrid...-nodes for gaps found in the grids of the given
 *   strides.
 *
 *   Example for gaps in grids:
 *       90 -> 200 step[0] 50
 *                   10 -> 20: ... tree1 ...
 *                   30 -> 40: ... tree2 ...
 *
 *   =>>
 *
 *      100 -> 200 step[0] 50
 *                    0 -> 10: ... tree1 ...
 *                   10 -> 20: noop
 *                   20 -> 30: ... tree2 ...
 *                   30 -> 50: noop
 *
 ******************************************************************************/

static node *
InsertNoopGrids (node *stride)
{
    node *grid, *grid_next;

    DBUG_ENTER ("InsertNoopGrids");

    if (stride != NULL) {
        DBUG_ASSERT (((NODE_TYPE (stride) == N_WLstride)
                      || (NODE_TYPE (stride) == N_WLstrideVar)),
                     "illegal stride found!");

        grid = WLSTRIDEX_CONTENTS (stride);
        DBUG_ASSERT ((grid != NULL), "no grid found!");

        if (NODE_TYPE (stride) == N_WLstride) {
            DBUG_ASSERT ((NODE_TYPE (grid) == N_WLgrid), "wrong node type found");

            /*
             * lower bound of first grid >0 ??
             */
            stride = NormalizeGrids (stride);
        } else {
            DBUG_ASSERT ((NodeOrInt_IntEq (NODE_TYPE (grid),
                                           WLGRIDX_GET_ADDR (grid, BOUND1), 0,
                                           IDX_SHAPE)),
                         "lower bound of first grid != 0");
        }

        /*
         * fill the gaps between the grids of the current dim
         */
        while (WLGRIDX_NEXT (grid) != NULL) {
            grid_next = WLGRIDX_NEXT (grid);

            grid = FillGapSucc (NULL, grid, NODE_TYPE (grid),
                                WLGRIDX_GET_ADDR (grid, BOUND2), NODE_TYPE (grid_next),
                                WLGRIDX_GET_ADDR (grid_next, BOUND1), TRUE);

            /*
             * next dim
             */
            WLGRIDX_NEXTDIM (grid) = InsertNoopGrids (WLGRIDX_NEXTDIM (grid));

            grid = grid_next;
        }

        /*
         * fill the gap after the last grid of the current dim
         */
        grid = FillGapSucc (NULL, grid, NODE_TYPE (grid), WLGRIDX_GET_ADDR (grid, BOUND2),
                            NODE_TYPE (stride), WLSTRIDEX_GET_ADDR (stride, STEP), TRUE);

        /*
         * next dim
         */
        WLGRIDX_NEXTDIM (grid) = InsertNoopGrids (WLGRIDX_NEXTDIM (grid));

        /*
         * next
         */
        WLSTRIDEX_NEXT (stride) = InsertNoopGrids (WLSTRIDEX_NEXT (stride));
    }

    DBUG_RETURN (stride);
}

/******************************************************************************
 *
 * Function:
 *   node *InsertNoopNode( node *wlnode)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
InsertNoopNode (node *wlnode)
{
    node *next;

    DBUG_ENTER ("InsertNoopNode");

    DBUG_ASSERT ((wlnode != NULL), "no WL node found!");

    next = WLNODE_NEXT (wlnode);

    if (next != NULL) {
        wlnode = FillGapSucc (NULL, wlnode, NODE_TYPE (wlnode),
                              WLNODE_GET_ADDR (wlnode, BOUND2), NODE_TYPE (next),
                              WLNODE_GET_ADDR (next, BOUND1), TRUE);
    }

    DBUG_RETURN (wlnode);
}

/******************************************************************************
 *
 * Function:
 *   bool InsertNoopNodes( node *wlnode)
 *
 * Description:
 *   Inserts NOOP-N_WLnodes for gaps found in 'wlnode'.
 *   Sons, which are complete NOOP trees, are freed.
 *
 ******************************************************************************/

static bool
InsertNoopNodes (node *wlnode)
{
    bool is_noop;

    DBUG_ENTER ("InsertNoopNodes");

    if (wlnode != NULL) {
        switch (NODE_TYPE (wlnode)) {
        case N_WLblock:
            /* here is no break missing */
        case N_WLublock:
            is_noop = InsertNoopNodes (WLXBLOCK_CONTENTS (wlnode));
            is_noop &= InsertNoopNodes (WLXBLOCK_NEXTDIM (wlnode));
            if (is_noop) {
                WLXBLOCK_CONTENTS (wlnode) = FreeTree (WLXBLOCK_CONTENTS (wlnode));
                WLXBLOCK_NEXTDIM (wlnode) = FreeTree (WLXBLOCK_NEXTDIM (wlnode));
            }

            is_noop &= InsertNoopNodes (WLXBLOCK_NEXT (wlnode));

            wlnode = InsertNoopNode (wlnode);
            break;

        case N_WLstride:
            /* here is no break missing */
        case N_WLstrideVar:
            is_noop = InsertNoopNodes (WLSTRIDEX_CONTENTS (wlnode));
            if (is_noop) {
                WLSTRIDEX_CONTENTS (wlnode) = FreeTree (WLSTRIDEX_CONTENTS (wlnode));
            }

            is_noop &= InsertNoopNodes (WLSTRIDEX_NEXT (wlnode));

            wlnode = InsertNoopNode (wlnode);
            break;

        case N_WLgrid:
            /* here is no break missing */
        case N_WLgridVar:
            if (WLGRIDX_NEXTDIM (wlnode) != NULL) {
                is_noop = InsertNoopNodes (WLGRIDX_NEXTDIM (wlnode));
                if (is_noop) {
                    WLGRIDX_NEXTDIM (wlnode) = FreeTree (WLGRIDX_NEXTDIM (wlnode));
                    WLGRIDX_NOOP (wlnode) = TRUE;
                }
            }

            is_noop = WLGRIDX_NOOP (wlnode);
            is_noop &= InsertNoopNodes (WLGRIDX_NEXT (wlnode));

            /*
             * no gaps in grids left after call of InsertNoopGrids() !!!
             *
            wlnode = InsertNoopNode( wlnode);
             */
            break;

        default:
            DBUG_ASSERT ((0), "illegal node type found!");
            is_noop = FALSE;
            break;
        }
    } else {
        is_noop = TRUE;
    }

    DBUG_RETURN (is_noop);
}

/**
 **
 **  functions for InsertNoopGrids(), InsertNoopNodes()
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 ******************************************************************************
 **
 **  functions for InferSegsParams_Pre()
 **
 **/

/******************************************************************************
 *
 * function:
 *   void ComputeIndexMinMax( node *wlseg, shpseg *shape, node *wlnode)
 *
 * description:
 *   Computes the minimum and maximum of the index-vector found in 'wlnode'.
 *
 * CAUTION:
 *   This routine fails if a segment has a variable IDX_MAX and contains more
 *   than one variable upper bound (e.g. multi-generator fold with-loop with
 *   more than one variable upper bound):
 *
 *     with(iv)
 *       a -> b step 1 ...
 *       c -> d step 1 ...
 *     fold( ...)
 *
 *   In this example  [ IDX_MIN, IDX_MAX ]  is set to  [ a, b ]  , but the
 *   correct result is  [ MIN( a, c), MAX( b, d) ]  .
 *
 *     with(iv)
 *       a -> b step 1 ...
 *       b -> c step 1 ...
 *     fold( ...)
 *
 *   In this example  [ IDX_MIN, IDX_MAX ]  is set to  [ a, b ]  , but the
 *   correct result is  [ a, c ]  .
 *
 *   Even worse: Although a wrong result is returned no error message is given!
 *
 ******************************************************************************/

static void
ComputeIndexMinMax (node *wlseg, shpseg *shape, node *wlnode)
{
    nodetype nt;
    void *p_min, *p_max;
    void *p_idx_min, *p_idx_max;
    int dim, shp_idx;

    DBUG_ENTER ("ComputeIndexMinMax");

    if (wlnode != NULL) {
        nt = NODE_TYPE (wlnode);
        switch (nt) {
        case N_WLstride:
            /* here is no break missing! */
        case N_WLstrideVar:
            ComputeIndexMinMax (wlseg, shape, WLSTRIDEX_CONTENTS (wlnode));
            ComputeIndexMinMax (wlseg, shape, WLSTRIDEX_NEXT (wlnode));

            dim = WLSTRIDEX_DIM (wlnode);
            p_min = WLSTRIDEX_GET_ADDR (wlnode, BOUND1);
            p_max = WLSTRIDEX_GET_ADDR (wlnode, BOUND2);
            break;

        case N_WLblock:
            /* here is no break missing! */
        case N_WLublock:
            ComputeIndexMinMax (wlseg, shape, WLXBLOCK_NEXTDIM (wlnode));
            ComputeIndexMinMax (wlseg, shape, WLXBLOCK_NEXT (wlnode));

            dim = WLXBLOCK_DIM (wlnode);
            p_min = WLSTRIDEX_GET_ADDR (wlnode, BOUND1);
            p_max = WLSTRIDEX_GET_ADDR (wlnode, BOUND2);
            break;

        case N_WLgrid:
            /* here is no break missing! */
        case N_WLgridVar:
            ComputeIndexMinMax (wlseg, shape, WLGRIDX_NEXTDIM (wlnode));
            ComputeIndexMinMax (wlseg, shape, WLGRIDX_NEXT (wlnode));

            /*
             * skip adjustment of 'idx_min', 'idx_max'
             */
            dim = (-1);
            p_min = p_max = 0;
            break;

        default:
            DBUG_ASSERT ((0), "illegal node type found!");
            dim = (-1);
            p_min = p_max = 0;
            break;
        }

        if (dim >= 0) {
            p_idx_min = WLSEGX_IDX_GET_ADDR (wlseg, IDX_MIN, dim);
            p_idx_max = WLSEGX_IDX_GET_ADDR (wlseg, IDX_MAX, dim);
            shp_idx = GET_SHAPE_IDX (SHPSEG_ELEMS (shape), dim);

            if (NodeOrInt_Le (nt, p_min, NODE_TYPE (wlseg), p_idx_min, shp_idx)) {
                NodeOrInt_SetNodeOrInt (NODE_TYPE (wlseg), p_idx_min, nt, p_min);
            }
            if (NodeOrInt_Le (NODE_TYPE (wlseg), p_idx_max, nt, p_max, shp_idx)) {
                NodeOrInt_SetNodeOrInt (NODE_TYPE (wlseg), p_idx_max, nt, p_max);
            }
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   node* InferSegsParams_Pre( node *segs, shpseg *shape)
 *
 * Description:
 *   Infers the temporary attribute IDX_MIN, IDX_MAX, SV of all the given
 *   segments.
 *
 ******************************************************************************/

static node *
InferSegsParams_Pre (node *segs, shpseg *shape)
{
    int d;

    DBUG_ENTER ("InferSegsParams_Pre");

    if (segs != NULL) {
        DBUG_ASSERT (((NODE_TYPE (segs) == N_WLseg) || (NODE_TYPE (segs) == N_WLsegVar)),
                     "no segment found!");

        DBUG_EXECUTE (
          "WLtrans", fprintf (stderr, "InferSegsParams_Pre: ");
          fprintf (stderr, "SHAPE = "); if (shape != NULL) {
              PRINT_VECT (stderr, SHPSEG_ELEMS (shape), WLSEGX_DIMS (segs), "%i");
          } else { fprintf (stderr, "NULL"); });

        if (NODE_TYPE (segs) == N_WLseg) {

            /**********************
             *  IDX_MIN, IDX_MAX  *
             **********************/

            MALLOC_INIT_VECT (WLSEG_IDX_MIN (segs), WLSEG_DIMS (segs), int, IDX_SHAPE);
            MALLOC_INIT_VECT (WLSEG_IDX_MAX (segs), WLSEG_DIMS (segs), int, 0);

            /*
             * compute the infimum and supremum of the index-vector.
             */
            ComputeIndexMinMax (segs, shape, WLSEG_CONTENTS (segs));

            /*******************
             *       SV        *
             *******************/

            MALLOC_VECT (WLSEG_SV (segs), WLSEG_DIMS (segs), int);
            for (d = 0; d < WLSEG_DIMS (segs); d++) {
                (WLSEG_SV (segs))[d] = GetLcmUnroll (WLSEG_CONTENTS (segs), d, FALSE);
            }

            DBUG_EXECUTE ("WLtrans", fprintf (stderr, ", WLSEG_IDX_MIN = ");
                          WLSEG_IDX_PRINT (stderr, segs, IDX_MIN);
                          fprintf (stderr, ", WLSEG_IDX_MAX = ");
                          WLSEG_IDX_PRINT (stderr, segs, IDX_MAX);
                          fprintf (stderr, ", WLSEG_SV = ");
                          PRINT_VECT (stderr, WLSEG_SV (segs), WLSEG_DIMS (segs), "%i");
                          fprintf (stderr, "\n"););

        } else {

            /**********************
             *  IDX_MIN, IDX_MAX  *
             **********************/

            MALLOC_INIT_VECT (WLSEGVAR_IDX_MIN (segs), WLSEGVAR_DIMS (segs), node *,
                              MakeNum (IDX_SHAPE));
            MALLOC_INIT_VECT (WLSEGVAR_IDX_MAX (segs), WLSEGVAR_DIMS (segs), node *,
                              MakeNum (0));

            /*
             * compute the infimum and supremum of the index-vector.
             */
            ComputeIndexMinMax (segs, shape, WLSEGVAR_CONTENTS (segs));

            DBUG_EXECUTE ("WLtrans", fprintf (stderr, ", WLSEGVAR_IDX_MIN = ");
                          WLSEGVAR_IDX_PRINT (stderr, segs, IDX_MIN);
                          fprintf (stderr, ", WLSEGVAR_IDX_MAX = ");
                          WLSEGVAR_IDX_PRINT (stderr, segs, IDX_MAX);
                          fprintf (stderr, "\n"););
        }

        WLSEGX_NEXT (segs) = InferSegsParams_Pre (WLSEGX_NEXT (segs), shape);
    }

    DBUG_RETURN (segs);
}

/**
 **
 **  functions for InferSegsParams_Pre()
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 ******************************************************************************
 **
 **  functions for InferSegsParams_Post()
 **
 **/

/******************************************************************************
 *
 * Function:
 *   bool IsHomSV( node *nodes, int dim, int sv, bool include_blocks)
 *
 * Description:
 *   Infers for the given wlnode tree whether in dimension 'dim' the extend
 *   of each top-level block-, ublock- or stride- node is a multiple of 'sv'.
 *
 *   'include_blocks' denotes whether N_WLblock nodes are considered to be
 *   undivisible or not.
 *   For the time being the backend needs (include_blocks = TRUE) !!!
 *
 ******************************************************************************/

static bool
IsHomSV (node *nodes, int dim, int sv, bool include_blocks)
{
    int ishom = TRUE;

    DBUG_ENTER ("IsHomSV");

    if (nodes != NULL) {
        ishom = IsHomSV (WLNODE_NEXT (nodes), dim, sv, include_blocks);

        if ((WLNODE_DIM (nodes) == dim)
            && (((NODE_TYPE (nodes) == N_WLblock) && (include_blocks))
                || (NODE_TYPE (nodes) == N_WLublock)
                || (NODE_TYPE (nodes) == N_WLstride))) {
            /*
             * we have found a relevant node
             */
            ishom &= ((WLNODE_BOUND2 (nodes) - WLNODE_BOUND1 (nodes)) % sv == 0);
        } else {
            /*
             * search in whole tree for relevant nodes
             */
            switch (NODE_TYPE (nodes)) {
            case N_WLblock:
                /* here is no break missing! */
            case N_WLublock:
                ishom &= IsHomSV (WLXBLOCK_NEXTDIM (nodes), dim, sv, include_blocks);
                ishom &= IsHomSV (WLXBLOCK_CONTENTS (nodes), dim, sv, include_blocks);
                break;

            case N_WLstride:
                ishom &= IsHomSV (WLSTRIDE_CONTENTS (nodes), dim, sv, include_blocks);
                break;

            case N_WLgrid:
                ishom &= IsHomSV (WLGRID_NEXTDIM (nodes), dim, sv, include_blocks);
                break;

            default:
                DBUG_ASSERT ((0), "wrong node type");
                break;
            }
        }
    }

    DBUG_RETURN (ishom);
}

/******************************************************************************
 *
 * Function:
 *   node* InferSegsParams_Post( node *segs)
 *
 * Description:
 *   Infers the temporary attributes SV and HOMSV of all the given segments.
 *
 ******************************************************************************/

static node *
InferSegsParams_Post (node *segs)
{
    int sv, d;

    DBUG_ENTER ("InferSegsParams_Post");

    if (segs != NULL) {
        DBUG_ASSERT (((NODE_TYPE (segs) == N_WLseg) || (NODE_TYPE (segs) == N_WLsegVar)),
                     "no segment found!");

        DBUG_EXECUTE ("WLtrans", fprintf (stderr, "InferSegsParams_Post: "););

        if (NODE_TYPE (segs) == N_WLseg) {

            /***************
             *  SV, HOMSV  *
             ***************/

            MALLOC_VECT (WLSEG_HOMSV (segs), WLSEG_DIMS (segs), int);
            for (d = 0; d < WLSEG_DIMS (segs); d++) {
                /*
                 * We must recalculate SV here because the with-loop transformations
                 * (especially the fitting) probabily have modified the layout!
                 * Moreover we have to consider blocks as undivisible here!
                 * (I.e. For the time being due to backend-limitations only complete
                 * blocks can be scheduled (multi-threading) !!!)
                 */
                sv = GetLcmUnroll (WLSEG_CONTENTS (segs), d, TRUE);
                (WLSEG_SV (segs))[d] = sv;
                if (IsHomSV (WLSEG_CONTENTS (segs), d, sv, TRUE)) {
                    (WLSEG_HOMSV (segs))[d] = sv;
                } else {
                    (WLSEG_HOMSV (segs))[d] = 0;
                }
            }

            DBUG_EXECUTE ("WLtrans", fprintf (stderr, "WLSEG_SV = ");
                          PRINT_VECT (stderr, WLSEG_SV (segs), WLSEG_DIMS (segs), "%i");
                          fprintf (stderr, ", WLSEG_HOMSV = ");
                          PRINT_HOMSV (stderr, WLSEG_HOMSV (segs), WLSEG_DIMS (segs));
                          fprintf (stderr, "\n"););

        } else {

            /*
             * nothing to do :-)
             */

            DBUG_EXECUTE ("WLtrans", fprintf (stderr, "---"); fprintf (stderr, "\n"););
        }

        WLSEGX_NEXT (segs) = InferSegsParams_Post (WLSEGX_NEXT (segs));
    }

    DBUG_RETURN (segs);
}

/**
 **
 **  functions for InferSegsParams_Post()
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 *
 * Function:
 *   node *InferFitted( node *wlnode)
 *
 * Description:
 *   Traverses the whole tree 'wlnode' and sets the FITTED flag of all grids
 *   correctly.
 *
 ******************************************************************************/

static node *
InferFitted (node *wlnode)
{
    node *grids;
    int bnd1, bnd2, step, width, remain, g_bnd1, g_bnd2;

    DBUG_ENTER ("InferFitted");

    if (wlnode != NULL) {
        WLNODE_NEXT (wlnode) = InferFitted (WLNODE_NEXT (wlnode));

        switch (NODE_TYPE (wlnode)) {
        case N_WLblock:
            /* here is no break missing */
        case N_WLublock:
            WLXBLOCK_NEXTDIM (wlnode) = InferFitted (WLXBLOCK_NEXTDIM (wlnode));
            WLXBLOCK_CONTENTS (wlnode) = InferFitted (WLXBLOCK_CONTENTS (wlnode));
            break;

        case N_WLstrideVar:
            /* here is no break missing */
        case N_WLstride:
            NodeOrInt_GetNameOrVal (NULL, &bnd1, NODE_TYPE (wlnode),
                                    WLSTRIDEX_GET_ADDR (wlnode, BOUND1));
            NodeOrInt_GetNameOrVal (NULL, &bnd2, NODE_TYPE (wlnode),
                                    WLSTRIDEX_GET_ADDR (wlnode, BOUND2));
            NodeOrInt_GetNameOrVal (NULL, &step, NODE_TYPE (wlnode),
                                    WLSTRIDEX_GET_ADDR (wlnode, STEP));
            width = bnd2 - bnd1;
            remain = width % step;

            grids = WLSTRIDEX_CONTENTS (wlnode);
            while (grids != NULL) {
                NodeOrInt_GetNameOrVal (NULL, &g_bnd1, NODE_TYPE (grids),
                                        WLGRIDX_GET_ADDR (grids, BOUND1));
                NodeOrInt_GetNameOrVal (NULL, &g_bnd2, NODE_TYPE (grids),
                                        WLGRIDX_GET_ADDR (grids, BOUND2));

                if ((g_bnd2 == 1)
                    || ((bnd1 >= 0) && (bnd2 >= 0) && (g_bnd1 >= 0) && (g_bnd2 >= 0)
                        && ((remain == 0) || (g_bnd2 <= remain)))) {
                    WLGRIDX_FITTED (grids) = TRUE;
                }

                WLGRIDX_NEXTDIM (grids) = InferFitted (WLGRIDX_NEXTDIM (grids));
                grids = WLGRIDX_NEXT (grids);
            }
            break;

        default:
            DBUG_ASSERT ((0), "wrong node type found!");
            break;
        }
    }

    DBUG_RETURN (wlnode);
}

/******************************************************************************
 *
 * Function:
 *   node *ProcessSegments( node *segs, int dims, shpseg *shape,
 *                          bool do_naive_comp)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
ProcessSegments (node *segs, int dims, shpseg *shape, bool do_naive_comp)
{
    node *seg;

    DBUG_ENTER ("ProcessSegments");

    /* compute SEGX_IDX_MIN, SEGX_IDX_MAX and SEG_SV */
    segs = InferSegsParams_Pre (segs, shape);

    seg = segs;
    while (seg != NULL) {
        DBUG_EXECUTE ("WLtrans", NOTE ((">>> entering segment")));

        /* check params of segment */
        CheckParams (seg);

        /*
         * splitting
         */
        if ((NODE_TYPE (seg) == N_WLseg) && (!do_naive_comp)) {
            DBUG_EXECUTE ("WLtrans", NOTE (("step 5: split")));
            WLSEG_CONTENTS (seg) = SplitWL (WLSEG_CONTENTS (seg));
        }
        if ((break_after == PH_wltrans) && (!strcmp (break_specifier, "split"))) {
            goto DONE;
        }

        /*
         * hierarchical blocking
         */
        if ((NODE_TYPE (seg) == N_WLseg) && (!do_naive_comp)) {
            int b;
            DBUG_EXECUTE ("WLtrans", NOTE (("step 6: hierarchical blocking")));
            for (b = 0; b < WLSEG_BLOCKS (seg); b++) {
                DBUG_EXECUTE ("WLtrans",
                              NOTE (("step 6.%d: hierarchical blocking (level %d)", b + 1,
                                     b)));
                WLSEG_CONTENTS (seg)
                  = BlockWL (WLSEG_CONTENTS (seg), dims, WLSEG_BV (seg, b), FALSE);
            }
        }
        if ((break_after == PH_wltrans) && (!strcmp (break_specifier, "block"))) {
            goto DONE;
        }

        /*
         * unrolling-blocking
         */
        if ((NODE_TYPE (seg) == N_WLseg) && (!do_naive_comp)) {
            DBUG_EXECUTE ("WLtrans", NOTE (("step 7: unrolling-blocking")));
            WLSEG_CONTENTS (seg)
              = BlockWL (WLSEG_CONTENTS (seg), dims, WLSEG_UBV (seg), TRUE);
        }
        if ((break_after == PH_wltrans) && (!strcmp (break_specifier, "ublock"))) {
            goto DONE;
        }

        /*
         * merging
         */
        if ((NODE_TYPE (seg) == N_WLseg) && (!do_naive_comp)) {
            DBUG_EXECUTE ("WLtrans", NOTE (("step 8: merge")));
            WLSEG_CONTENTS (seg) = MergeWL (WLSEG_CONTENTS (seg));
        }
        if ((break_after == PH_wltrans) && (!strcmp (break_specifier, "merge"))) {
            goto DONE;
        }

        /*
         * optimization
         */
        if ((NODE_TYPE (seg) == N_WLseg) && (!do_naive_comp)) {
            DBUG_EXECUTE ("WLtrans", NOTE (("step 9: optimize")));
            WLSEG_CONTENTS (seg) = OptWL (WLSEG_CONTENTS (seg));
        }
        if ((break_after == PH_wltrans) && (!strcmp (break_specifier, "opt"))) {
            goto DONE;
        }

        /*
         * fitting
         */
        if ((NODE_TYPE (seg) == N_WLseg) && (!do_naive_comp)) {
            DBUG_EXECUTE ("WLtrans", NOTE (("step 10: fit")));
            WLSEG_CONTENTS (seg) = FitWL (WLSEG_CONTENTS (seg));
        }
        if ((break_after == PH_wltrans) && (!strcmp (break_specifier, "fit"))) {
            goto DONE;
        }

        /*
         * normalization
         */
        if ((NODE_TYPE (seg) == N_WLseg) && (!do_naive_comp)) {
            DBUG_EXECUTE ("WLtrans", NOTE (("step 11: normalize")));
            WLSEG_CONTENTS (seg)
              = NormWL (dims, shape, WLSEG_IDX_MAX (seg), WLSEG_CONTENTS (seg));
        }
        if ((break_after == PH_wltrans) && (!strcmp (break_specifier, "norm"))) {
            goto DONE;
        }

        /*
         * fill all gaps
         */
        DBUG_EXECUTE ("WLtrans", NOTE (("step 12: fill gaps (all)")));
        InsertNoopNodes (WLSEGX_CONTENTS (seg));
        if ((break_after == PH_wltrans) && (!strcmp (break_specifier, "fill2"))) {
            goto DONE;
        }

    DONE:
        /* compute GRIDX_FITTED */
        WLSEGX_CONTENTS (seg) = InferFitted (WLSEGX_CONTENTS (seg));

        DBUG_EXECUTE ("WLtrans", NOTE (("<<< leaving segment")));

        seg = WLSEGX_NEXT (seg);
    }

    /* recompute SEG_SV and compute SEG_HOMSV */
    segs = InferSegsParams_Post (segs);

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * Function:
 *   node *ConvertWith( node *wl, int dims)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
ConvertWith (node *wl, int dims)
{
    node *new_node;

    DBUG_ENTER ("ConvertWith");

    new_node
      = MakeNWith2 (NWITH_WITHID (wl), NULL, NWITH_CODE (wl), NWITH_WITHOP (wl), dims);

    /* extract array-placement pragmas */
    NWITH2_PRAGMA (new_node) = ExtractAplPragma (NWITH_PRAGMA (wl), line);
    if ((NWITH2_PRAGMA (new_node) != NULL) && (NWITH_IS_FOLD (wl))) {
        /* no array placement for fold with-loops */
        NWITH2_PRAGMA (new_node) = FreeTree (NWITH2_PRAGMA (new_node));
    }

    NWITH2_OFFSET_NEEDED (new_node)
      = ((NWITH_TYPE (wl) == WO_genarray) || (NWITH_TYPE (wl) == WO_modarray));

    NWITH2_DEC_RC_IDS (new_node) = NWITH_DEC_RC_IDS (wl);
    NWITH2_IN_MASK (new_node) = NWITH_IN_MASK (wl);
    NWITH2_OUT_MASK (new_node) = NWITH_OUT_MASK (wl);
    NWITH2_LOCAL_MASK (new_node) = NWITH_LOCAL_MASK (wl);

    /*
     * withid, code, withop and IN/INOUT/OUT/LOCAL are reused for the
     *  Nwith2-tree.
     * Because of that, these parts are cut off from the old nwith-tree,
     *  before freeing it.
     */
    NPART_WITHID (NWITH_PART (wl)) = NULL;
    NWITH_CODE (wl) = NULL;
    NWITH_WITHOP (wl) = NULL;
    NWITH_DEC_RC_IDS (wl) = NULL;
    NWITH_IN_MASK (wl) = NULL;
    NWITH_OUT_MASK (wl) = NULL;
    NWITH_LOCAL_MASK (wl) = NULL;

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * Function:
 *   node *WLTRAwith( node *arg_node, node *arg_info)
 *
 * Description:
 *   transforms with-loop (N_Nwith-node) into new representation (N_Nwith2).
 *
 * Remark:
 *   'INFO_WL_TYPES( arg_info)' points to the type of the (first) let-ids.
 *
 ******************************************************************************/

node *
WLTRAwith (node *arg_node, node *arg_info)
{
    types *idx_type;
    shape_class_t idx_sc;
    node *new_node = NULL;

    DBUG_ENTER ("WLTRAwith");

    DBUG_EXECUTE ("WLtrans", NOTE ((">>> >>> entering with-loop")));

    /* store the lineno of the current wl (for generation of error-messages) */
    line = NODE_LINE (arg_node);

    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

    idx_type = IDS_TYPE (NWITH_VEC (arg_node));
    idx_sc = GetShapeClassFromTypes (idx_type);
    if (idx_sc != C_aks) {
        DBUG_ASSERT ((idx_sc != C_scl), "scalar index vector found!");

        /*
         * index vector is AKD or AUD
         *   -> we have to create totally different C code
         *   -> we leave the N_Nwith node untouched
         */
        DBUG_EXECUTE ("WLtrans",
                      NOTE (("with-loop with non-AKS withid found (line %d)", line)););
        new_node = arg_node;
    } else {
        node *strides;
        int idx_size;
        shpseg *wl_shp;

        DBUG_EXECUTE ("WLtrans",
                      NOTE (("with-loop with AKS withid found (line %d)", line)););

        /*
         * check whether NWITHID_VEC, NWITHID_IDS of all parts have identical
         * names
         */
        DBUG_ASSERT ((CheckWithids (NWITH_PART (arg_node))),
                     "Not all N_Nwithid nodes of the with-loop have identical"
                     " names!\n"
                     "This is probably due to an error during with-loop-folding.");

        /* get number of dims of with-loop index range */
        idx_size = TYPES_SHAPE (idx_type, 0);
        wl_shp = GetWlShape (arg_node, idx_size, INFO_WL_TYPES (arg_info));

        /*
         * convert parts of with-loop into new format
         */
        DBUG_EXECUTE ("WLtrans", NOTE (("step 1.1: convert parts into strides")));
        strides = Parts2Strides (NWITH_PART (arg_node), idx_size, wl_shp);

        /*
         * consistence check: ensures that the strides are pairwise disjoint
         */
        DBUG_EXECUTE ("WLtrans", NOTE (("step 1.2: check disjointness of strides")));
        DBUG_ASSERT ((CheckDisjointness (strides)),
                     "Consistence check failed:"
                     " Not all strides are pairwise disjoint!\n"
                     "This is probably due to an error during with-loop-folding.");

        new_node = ConvertWith (arg_node, idx_size);

        if (strides == NULL) {
            /* all parts are empty  ->  set 'strides' or 'new_node' */
            strides = EmptyParts2StridesOrExpr (&new_node, idx_size, wl_shp);
        }

        if (strides != NULL) {
            node *cubes = NULL;
            node *segs = NULL;
            bool do_naive_comp = ExtractNaiveCompPragma (NWITH_PRAGMA (arg_node), line);
            if ((break_after == PH_wltrans) && (!strcmp (break_specifier, "conv"))) {
                goto DONE;
            }

            /*
             * build the cubes
             */
            DBUG_EXECUTE ("WLtrans", NOTE (("step 2: build cubes")));
            cubes = BuildCubes (strides, new_node, idx_size, wl_shp, &do_naive_comp);
            if ((break_after == PH_wltrans) && (!strcmp (break_specifier, "cubes"))) {
                goto DONE;
            }

            /*
             * normalize grids and fill gaps
             */
            DBUG_EXECUTE ("WLtrans", NOTE (("step 3: fill gaps (grids)")));
            cubes = InsertNoopGrids (cubes);
            if ((break_after == PH_wltrans) && (!strcmp (break_specifier, "fill1"))) {
                goto DONE;
            }

            DBUG_EXECUTE ("WLtrans", NOTE (("step 4: choose segments")));
            if (do_naive_comp) {
                /* naive compilation  ->  put each stride in a separate segment */
                segs = WLCOMP_Cubes (NULL, NULL, cubes, idx_size, line);
            } else {
                simpletype wl_btype = GetBasetype (INFO_WL_TYPES (arg_info));
                bool fold_float = (NWITH2_IS_FOLD (new_node)
                                   && ((wl_btype == T_float) || (wl_btype == T_double)));
                segs = SetSegs (NWITH_PRAGMA (arg_node), cubes, idx_size, fold_float);
            }
            if ((break_after == PH_wltrans) && (!strcmp (break_specifier, "segs"))) {
                goto DONE;
            }

            /*
             * do all the segment-wise transformation stuff (step 4 -- 11)
             */
            segs = ProcessSegments (segs, idx_size, wl_shp, do_naive_comp);

        DONE:
            if (segs == NULL) {
                segs = WLCOMP_All (NULL, NULL, (cubes == NULL) ? strides : cubes,
                                   idx_size, line);
            }

            /* free temporary data */
            if (cubes != NULL) {
                cubes = FreeTree (cubes);
            }

            NWITH2_SEGS (new_node) = segs;
        }

        /* old WL-representation is no longer needed */
        arg_node = FreeTree (arg_node);
    }

    DBUG_EXECUTE ("WLtrans", NOTE (("<<< <<< leaving with-loop")));

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * Function:
 *   node *WLTRAcode( node *arg_node, node *arg_info)
 *
 * Description:
 *   checks NCODE_USED
 *
 ******************************************************************************/

node *
WLTRAcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLTRAcode");

    DBUG_ASSERT ((NCODE_USED (arg_node) >= 0), "illegal NCODE_USED value!");

    if (NCODE_CBLOCK (arg_node) != NULL) {
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    }

    if (NCODE_CEXPR (arg_node) != NULL) {
        NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);
    }

    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *WLTRAlet( node *arg_node, node *arg_info)
 *
 * Description:
 *   'INFO_WL_TYPES( arg_info)' points to the type of the (first) let-ids.
 *
 ******************************************************************************/

node *
WLTRAlet (node *arg_node, node *arg_info)
{
    types *tmp;

    DBUG_ENTER ("WLTRAlet");

    tmp = INFO_WL_TYPES (arg_info);

    if (LET_IDS (arg_node) != NULL) {
        DBUG_ASSERT ((LET_VARDEC (arg_node) != NULL),
                     "vardec of let-variable not found!");

        DBUG_ASSERT (((NODE_TYPE (LET_VARDEC (arg_node)) == N_vardec)
                      || (NODE_TYPE (LET_VARDEC (arg_node)) == N_arg)),
                     "vardec-node of let-variable has wrong type!");

        INFO_WL_TYPES (arg_info) = LET_TYPE (arg_node);
    } else {
        INFO_WL_TYPES (arg_info) = NULL;
    }

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    INFO_WL_TYPES (arg_info) = tmp;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *WlTransform( node *syntax_tree)
 *
 * Description:
 *   Transforms all N_Nwith nodes into N_Nwith2 nodes.
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

    info = FreeTree (info);

    DBUG_RETURN (syntax_tree);
}
