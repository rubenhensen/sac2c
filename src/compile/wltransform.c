/*
 *
 * $Log$
 * Revision 2.9  2000/01/24 19:35:43  dkr
 * disjointness check added
 *
 * Revision 2.8  1999/12/01 15:20:29  dkr
 * oops, the DBUG_ASSERT in WLTRALet was too restrictiv ...
 *
 * Revision 2.7  1999/12/01 14:14:09  dkr
 * DBUG_ASSERTs in function WLTRALet() added
 *
 * Revision 2.6  1999/11/30 20:30:22  dkr
 * finally I have done it 8-)))
 * The "must-resign" problem is fixed now!
 * Therefore the cube calculation should work in general now!
 *
 * Moreover, some bugs have been fixed, e.g. in NormalizeStride_1() ...
 *
 * Revision 2.4  1999/11/30 10:07:35  dkr
 * fixed a bug in AdjustBounds()
 * code brushed
 * some comments added
 * some functions renamed
 *
 * Revision 2.3  1999/11/16 15:23:19  dkr
 * fixed a bug:
 *   blocking for var.segs. not yet implemented
 *   therefore WLSEG_BV and WLSEG_UBV are set to (1,1,1,...) in this case
 *   to prevent the building of an ADJUST_OFFSET-ICM.
 *
 * Revision 2.2  1999/08/10 15:43:11  dkr
 * bug in cube generation fixed
 *
 * Revision 2.1  1999/02/23 12:43:00  sacbase
 * new release made
 *
 * Revision 1.38  1999/01/19 10:24:41  sbs
 * unused var unroll in InferInnerStep eliminated
 *
 * Revision 1.37  1998/11/08 14:32:55  dkr
 * WLTRANcode:
 *   NCODE_CBLOCK and NCODE_CEXPR should be never NULL!
 *   (if so, we will get an assert now :-)
 *
 * Revision 1.36  1998/10/24 18:32:57  dkr
 * Fixed a bug in InferInnerStep
 *
 * Revision 1.35  1998/08/14 22:16:45  dkr
 * bug fixed in IsHom():
 *   ASSERT-condition now correct
 *
 * Revision 1.34  1998/08/13 22:20:53  dkr
 * InferMaxHomDim finished
 *
 * Revision 1.33  1998/08/11 18:17:34  dkr
 * InferMaxHomDim() added (not yet completed :-((
 *
 * Revision 1.32  1998/08/11 14:46:04  dkr
 * support for N_WLsegVar added (not yet completed)
 * WL..._INNERSTEP infered
 *
 * Revision 1.31  1998/08/11 00:10:46  dkr
 * assert-text in IntersectOutline changed
 *
 * Revision 1.30  1998/08/07 18:03:53  dkr
 * a bug in CheckParams fixed
 *
 * Revision 1.27  1998/08/06 01:16:12  dkr
 * fixed some minor bugs
 *
 * Revision 1.26  1998/07/08 13:02:00  dkr
 * added ; behind DBUG_VOID_RETURN
 *
 * Revision 1.25  1998/06/16 13:59:02  dkr
 * fixed a bug:
 *   call of NormalizeWL now with correct WLSEG_IDX_MAX
 *
 * Revision 1.24  1998/06/09 16:47:24  dkr
 * IDX_MIN, IDX_MAX now segment-specific
 *
 * Revision 1.23  1998/06/03 13:48:08  dkr
 * added some comments for ComputeOneCube and its helper-routines
 *
 * Revision 1.22  1998/05/29 00:06:38  dkr
 * fixed a bug with unrolling
 *
 * Revision 1.21  1998/05/25 13:15:11  dkr
 * ASSERTs about wrong arguments in wlcomp-pragmas are now ABORT-messages
 *
 * Revision 1.20  1998/05/24 21:15:39  dkr
 * fixed a bug in ComputeIndexMinMax
 *
 * Revision 1.19  1998/05/24 00:41:16  dkr
 * fixed some minor bugs
 * code templates are not used anymore
 *
 * Revision 1.18  1998/05/17 02:06:33  dkr
 * added assertion in IntersectOutline()
 *
 * Revision 1.17  1998/05/17 00:11:20  dkr
 * WLGRID_CEXPR_TEMPLATE is now WLGRID_CODE_TEMPLATE
 *
 * Revision 1.16  1998/05/16 19:51:36  dkr
 * fixed minor bugs in ComputeOneCube
 * (0->1) grids are always N_WLgrid-nodes now!
 *
 * Revision 1.15  1998/05/15 23:45:20  dkr
 * removed a unused var and a unused param in NormalizeWL()
 *
 * Revision 1.14  1998/05/15 23:02:31  dkr
 * fixed some minor bugs
 *
 * Revision 1.13  1998/05/15 16:05:06  dkr
 * fixed a bug in ComputeOneCube, ...
 *
 * Revision 1.12  1998/05/15 15:10:56  dkr
 * changed ComputeOneCube (finished :)
 *
 * Revision 1.11  1998/05/14 21:35:45  dkr
 * changed ComputeOneCube (not finished ...)
 *
 * Revision 1.10  1998/05/12 22:42:54  dkr
 * added attributes NWITH2_DIM, NWITH2_IDX_MIN, NWITH2_IDX_MAX
 * added ComputeIndexMinMax()
 *
 * Revision 1.9  1998/05/12 15:00:48  dkr
 * renamed ..._RC_IDS to ..._DEC_RC_IDS
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
 */

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
     for each dimension (d) bv0_d, bv1_d, ... and ubv_d must be multiples of
     sv_d.
     However, for the first components of bv0, bv1, ... and ubv is also the
     value 1 allowed: in this case no blocking is performed for these
     dimensions.

     If bv = (1, ..., 1, ?, gt, ..., gt) is hold --- gt means that bv_d is
     greater or equal the segment width ---, this is equivalent to
     bv = (1, ..., 1), provided that this is compatible to the value of ubv.
     Nevertheless, this simplification is *not* carried out by the compiler yet!

     -> set of segments (segment := a cube without a grid)

  In the example: We choose the complete shape as a segment and
                  bv0 = (180,156), ubv = (1,6) --- note that sv = (9,3).


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
    each dimension: Each group formed after splitting a single dimension contains
    exactly the projections that must be joined in the merging step.
    Nevertheless it is easier and clearer to separate these two steps from each
    other: On the one hand the merging can be performed *after* the blocking step
    only (for the reasons see step 4.), on the other hand the splitting is
    carried out best just *before* the blocking, because while doing so, some
    grids might move (see example) --- i.e. the contents of a block would change!

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

  The blocking is performed that early because the blocking changes the iteration
  order on the array. That means, all the following steps --- in perticulary
  merging and optimize --- must know the concrete blocking.
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
        0000->1500, block[1] 156:  // ... the blocks of the first cube.
               op1                 // Here we must define what has to happen ...
                                   // ... inside the block.
    0000->0500, block[0] 180:
        1500->4000, block[1] 156:
               op2

    0500->3000, block[0] 180:
        0000->1500, block[1] 156:
               op3

    0500->3000, block[0] 180:
        1500->4000, block[1] 156:
               op4

    3000->4000, block[0] 180:
        0000->1000, block[1] 156:
               op5

    3000->4000, block[0] 180:
        1000->4000, block[1] 156:
               op6

    At the leafs of this blocking-tree we now add the contents of the blocks:

    0000->0500, block[0] 180:
        0000->1500, block[1] 156:
               0->180, step[0] 1
                            0->1: 0->156, step[1] 1
                                               0->1: e1

    0000->0500, block[0] 180:
        1500->4000, block[1] 156:
               0->180, step[0] 9
                            0->2: 0->156, step[1] 1
                                               0->1: e2
                            2->9: 0->156, step[1] 1
                                               0->1: e1

    0500->3000, block[0] 180:
        0000->1500, block[1] 156:
               0->180, step[0] 1
                            0->1: 0->156, step[1] 1
                                               0->1: e2

    0500->3000, block[0] 180:
        1500->4000, block[1] 156:
               0->180, step[0] 9
                            0->4: 0->156, step[1] 1
                                               0->1: e1
                            4->6: 0->156, step[1] 1
                                               0->1: e2
                            6->9: 0->156, step[1] 1
                                               0->1: e1

    3000->4000, block[0] 180:
        0000->1000, block[1] 156:
               0->180, step[0] 1
                            0->1: 0->156, step[1] 1
                                               0->1: e3

    3000->4000, block[0] 180:
        1000->4000, block[1] 156:
               0->180, step[0] 1
                            0->1: 0->156, step[1] 3
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

  In the example with bv = (1,156):

    0000->0500, step[0] 1
                     0->1:    0->1500, block[1] 156:
                                             0->156, step[1] 1
                                                          0->1: e1
    0000->0500, step[0] 9
                     0->2: 1500->4000, block[1] 156:
                                             0->156, step[1] 1
                                                          0->1: e2
                     2->9: 1500->4000, block[1] 156:
                                             0->156, step[1] 1
                                                          0->1: e1
    0500->3000, step[0] 1
                     0->1:    0->1500, block[1] 156:
                                             0->156, step[1] 1
                                                          0->1: e2
    0500->3000, step[0] 9
                     0->4: 1500->4000, block[1] 156:
                                             0->156, step[1] 1
                                                          0->1: e1
                     4->6: 1500->4000, block[1] 156:
                                             0->156, step[1] 1
                                                          0->1: e2
                     6->9: 1500->4000, block[1] 156:
                                             0->156, step[1] 1
                                                          0->1: e1
    3000->4000, step[0] 1
                     0->1:    0->1000, block[1] 156:
                                             0->156, step[1] 1
                                                          0->1: e3
    3000->4000, step[0] 1
                     0->1: 1000->4000, block[1] 156:
                                             0->156, step[1] 3
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
  for the example with bv = (180,156):

    000->050, block[0] 180:
        000->150, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, step[1] 1
                                              0->1: e1
    000->050, block[0] 180:
        150->400, block[1] 156:
              0->180, step[0] 9
                           0->2: 0->156, step[1] 1
                                              0->1: e2
                           2->9: 0->156, step[1] 1
                                              0->1: e1
    050->300, block[0] 180:
        000->150, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, step[1] 1
                                              0->1: e2
    050->300, block[0] 180:
        150->400, block[1] 156:
              0->180, step[0] 9
                           0->4: 0->156, step[1] 1
                                              0->1: e1
                           4->6: 0->156, step[1] 1
                                              0->1: e2
                           6->9: 0->156, step[1] 1
                                              0->1: e1
    300->400, block[0] 180:
        000->100, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, step[1] 1
                                              0->1: e3
    300->400, block[0] 180:
        100->400, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, step[1] 3
                                              0->1: e3
                                              1->3: e4

  Note: instead of transforming to
          0->50, block[0] 50: ...
        at first the original values are preserved because these are needed
        for the tree optimization (see step 7.) !!
        However, the corrected blocking sizes are needed not until the
        fitting (see step 9.).

  With bv = (1,156) --- because of ubv = (1,6) this is *not* equivalent to (1,1)
  --- we would get:

      0-> 50, step[0] 1
                   0->1:   0->150, block[1] 156:
                                         0->156, step[1] 1
                                                      0->1: e1
      0-> 50, step[0] 9
                   0->2: 150->400, block[1] 156:
                                         0->156, step[1] 1
                                                      0->1: e2
                   2->9: 150->400, block[1] 156:
                                         0->156, step[1] 1
                                                      0->1: e1
     50->300, step[0] 1
                   0->1:   0->150, block[1] 156:
                                         0->156, step[1] 1
                                                      0->1: e2
     50->300, step[0] 9
                   0->4: 150->400, block[1] 156:
                                         0->156, step[1] 1
                                                      0->1: e1
                   4->6: 150->400, block[1] 156:
                                         0->156, step[1] 1
                                                      0->1: e2
                   6->9: 150->400, block[1] 156:
                                         0->156, step[1] 1
                                                      0->1: e1
    300->400, step[0] 1
                   0->1:   0->100, block[1] 156:
                                         0->156, step[1] 1
                                                      0->1: e3
    300->400, step[0] 1
                   0->1: 100->400, block[1] 156:
                                         0->156, step[1] 3
                                                      0->1: e3
                                                      1->3: e4


5.) Unrolling-Blocking (without fitting) according to the values in ubv
    ------------------

    On each block an additional blocking for each dimension with (ubv_d > 1)
    is performed.
    This blocking differs from the conventional hierarchical blocking in
    the sense, that possibly a fitting is carried out (see step 8.)

  In the example with bv = (180,156):

    000->050, block[0] 180:
        000->150, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
    000->050, block[0] 180:
        150->400, block[1] 156:
              0->180, step[0] 9
                           0->2: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           2->9: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
    050->300, block[0] 180:
        000->150, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
    050->300, block[0] 180:
        150->400, block[1] 156:
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
    300->400, block[0] 180:
        000->100, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e3
    300->400, block[0] 180:
        100->400, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, ublock[1] 6:
                                                0->6, step[1] 3
                                                           0->1: e3
                                                           1->3: e4

  In the example with bv = (1,156):

      0-> 50, step[0] 1
                   0->1:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
      0-> 50, step[0] 9
                   0->2: 150->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                   2->9: 150->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
     50->300, step[0] 1
                   0->1:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
     50->300, step[0] 9
                   0->4: 150->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                   4->6: 150->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                   6->9: 150->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
    300->400, step[0] 1
                   0->1:   0->100, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e3
    300->400, step[0] 1
                   0->1: 100->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 3
                                                                   0->1: e3
                                                                   1->3: e4


6.) Cube Merging (Makes cubes with identical subtrees compatible and joins them)
    ------------

    -> The tree forms in each dimension a partition of the relevant index-vector-set
       projection.

  In the example with bv = (180,156):

    000->050, block[0] 180:
        000->150, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
        150->400, block[1] 156:
              0->180, step[0] 9
                           0->2: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           2->9: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
    050->300, block[0] 180:
        000->150, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
        150->400, block[1] 156:
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
    300->400, block[0] 180:
        000->100, block[1] 156:
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

  In the example with bv = (1,156):

      0-> 50, step[0] 9
                   0->2:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         150->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                   2->9:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         150->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
     50->300, step[0] 9
                   0->4:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                   4->6:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                   6->9:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
    300->400, step[0] 1
                   0->1:   0->100, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e3
                         100->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 3
                                                                   0->1: e3
                                                                   1->3: e4


7.) Tree Optimization (Joins identical subtrees)
    -----------------

    Projections with consecutive index ranges and identical operations (subtrees)
    are joined.

  In the example with bv = (1,156):

      0-> 50, step[0] 9
               ...
                   2->9:   0->150, block[1] 156: ... tree_1 ...

                         150->400, block[1] 156: ... tree_1 ...
                        Fortunately we still ^ have the same values overhere!

    is transformed to

      0-> 50, step[0] 9
               ...
                   2->9:   0->400, block[1] 156: ... tree_1 ...
                        Now this value makes ^ sense again!

  Altogether:

      0-> 50, step[0] 9
                   0->2:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         150->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                   2->9:   0->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
     50->300, step[0] 9
                   0->4:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                   4->6:   0->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                   6->9:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
    300->400, step[0] 1
                   0->1:   0->100, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e3
                         100->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 3
                                                                   0->1: e3
                                                                   1->3: e4


8.) Projection Fitting (Removes incomplete periods at the tail)
    ------------------
    (^ after the optimization we have in general no cubes anymore ...)

    The boundaries of the most outer node in each dimension is adjusted to
    the number of unrolled elements (= max( ubv_d, step)).

  In the example with bv = (180,156):

    000->045, block[0] 180:
        000->150, block[1] 156:
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

  In the example with bv = (1,156):

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

  In the example bv = (180,156):

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

  In the example with bv = (1,156):

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
               step      (int)                // blocking-factor
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

#include "tree.h"
#include "free.h"

#include "internal_lib.h"
#include "traverse.h"
#include "DataFlowMask.h"
#include "optimize.h"
#include "wlpragma_funs.h"

#include "DupTree.h"
#include "dbug.h"

/*
 * here we store the lineno of the current with-loop
 *  (for creating error-messages ...)
 */
static int line;

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
            comp = DupNode (node);                                                       \
            break;                                                                       \
        case N_exprs:                                                                    \
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (node)) == N_num),                       \
                         "wrong node type found");                                       \
            comp = MakeNum (NUM_VAL (EXPRS_EXPR (node)));                                \
            node = EXPRS_NEXT (node);                                                    \
            break;                                                                       \
        default:                                                                         \
            DBUG_ASSERT ((0), "wrong node type found");                                  \
        }                                                                                \
    } else {                                                                             \
        comp = MakeNum (1);                                                              \
    }

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

/******************************************************************************
 ******************************************************************************
 **
 ** general purpose functions
 **
 **/

/******************************************************************************
 *
 * function:
 *   int CompareWLnode( node *node1, node *node2, int outline)
 *
 * description:
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
            DBUG_ASSERT ((grid1 != NULL), "no grid1 for comparison found");
            grid2 = WLSTRIDE_CONTENTS (node2);
            DBUG_ASSERT ((grid2 != NULL), "no grid2 for comparison found");

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
 * function:
 *   node *NormalizeStride_1( node *stride)
 *
 * description:
 *   Returns the IN THE FIRST DIMENSION normalized N_WLstride-node 'stride'.
 *   a possibly present next node in 'stride' is ignored.
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
 *     * (step < bound2 - bound1) or (step == 1),
 *     * (step < grid_b2 - grid_b1) or (step == 1),
 *     * grid_b2 <= step,
 *     * bound1 minimized,
 *     * bound2 maximized.
 *
 ******************************************************************************/

node *
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
     * and therefore not yet implemented ...
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
    DBUG_ASSERT ((0 <= bound1), "given stride has illegal lower bound (<= 0)");
    DBUG_ASSERT ((bound1 < bound2),
                 "given stride is empty (lower bound >= upper bound)!");
    DBUG_ASSERT ((0 < step), "given step is illegal (<= 0)");
    DBUG_ASSERT ((0 <= grid_b1), "given grid has illegal lower bound (<= 0)");
    DBUG_ASSERT ((grid_b1 < grid_b2),
                 "given grid is empty (lower bound >= upper bound)!");

    /*
     * if (bound2 - bound1 <= step), we can set (step = 1)
     */
    if (bound2 - bound1 <= step) {
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
    DBUG_ASSERT ((0 <= bound1), "modified stride has illegal lower bound (<= 0)");
    DBUG_ASSERT ((bound1 < bound2),
                 "modified stride is empty (lower bound >= upper bound)!");
    DBUG_ASSERT ((0 < step), "modified step is illegal (<= 0)");
    DBUG_ASSERT ((0 <= grid_b1), "modified grid has illegal lower bound (<= 0)");
    DBUG_ASSERT ((grid_b1 < grid_b2),
                 "modified grid is empty (lower bound >= upper bound)!");

    /*
     * maximize the outline:
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
 *   int IndexHeadStride( node *stride)
 *
 * description:
 *   returns the index position of the first element of 'stride'.
 *
 * remark:
 *   the grids of the stride must be sorted in ascending order with respect
 *   to their lower bounds, because this routine examines only the *first*
 *   grid!
 *
 ******************************************************************************/

int
IndexHeadStride (node *stride)
{
    int result;

    DBUG_ENTER ("IndexHeadStride");

    DBUG_ASSERT ((NODE_TYPE (stride) == N_WLstride),
                 "given node is not a constant stride!");
    DBUG_ASSERT ((WLSTRIDE_BOUND1 (stride) < WLSTRIDE_BOUND2 (stride)),
                 "given stride is empty (lower bound >= upper bound)!");

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
    node *grid;
    int bound2, grid_b1, result;

    DBUG_ENTER ("IndexRearStride");

    DBUG_ASSERT ((NODE_TYPE (stride) == N_WLstride),
                 "given node is not a constant stride!");
    DBUG_ASSERT ((WLSTRIDE_BOUND1 (stride) < WLSTRIDE_BOUND2 (stride)),
                 "given stride is empty (lower bound >= upper bound)!");

    grid = WLSTRIDE_CONTENTS (stride);
    bound2 = WLSTRIDE_BOUND2 (stride);

    DBUG_ASSERT ((NODE_TYPE (grid) == N_WLgrid), "given stride contains no grid!");

    grid_b1 = WLGRID_BOUND1 (grid);

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

/******************************************************************************
 *
 * function:
 *   int GetMaxUnroll( node *nodes, int dim)
 *
 * description:
 *   returns the maximally number elements that must be unrolled
 *     in dimension 'dim' of the WL-tree 'nodes'.
 *
 *   we must search for the first N_WLublock- or N_WLstride-node in each
 *     leaf of the 'nodes'-tree and get the step of this node.
 *
 ******************************************************************************/

int
GetMaxUnroll (node *nodes, int dim)
{
    int unroll = 1;

    DBUG_ENTER ("GetMaxUnroll");

    if (nodes != NULL) {
        unroll = GetMaxUnroll (WLNODE_NEXT (nodes), dim);

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
                unroll = MAX (unroll, GetMaxUnroll (WLBLOCK_NEXTDIM (nodes), dim));
                unroll = MAX (unroll, GetMaxUnroll (WLBLOCK_CONTENTS (nodes), dim));
                break;

            case N_WLstride:
                unroll = MAX (unroll, GetMaxUnroll (WLSTRIDE_CONTENTS (nodes), dim));
                break;

            case N_WLgrid:
                unroll = MAX (unroll, GetMaxUnroll (WLBLOCK_NEXTDIM (nodes), dim));
                break;

            default:
                DBUG_ASSERT ((0), "wrong node type");
            }
        }
    }

    DBUG_RETURN (unroll);
}

/**
 **
 ** general purpose functions
 **
 ******************************************************************************
 ******************************************************************************/

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
    node *parts_stride, *stride, *new_stride, *new_grid, *last_grid, *gen, *bound1,
      *bound2, *step, *width, *curr_bound1, *curr_bound2, *curr_step, *curr_width;
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
                DBUG_ASSERT ((bound1 != NULL), "bound1 incomplete");
                DBUG_ASSERT ((bound2 != NULL), "bound2 incomplete");

                curr_step_ = (step != NULL) ? NUM_VAL (EXPRS_EXPR (step)) : 1;
                curr_width_ = (width != NULL) ? NUM_VAL (EXPRS_EXPR (width)) : 1;

                /* build N_WLstride-node of current dimension */
                new_stride = MakeWLstride (0, dim, NUM_VAL (EXPRS_EXPR (bound1)),
                                           NUM_VAL (EXPRS_EXPR (bound2)), curr_step_, 0,
                                           MakeWLgrid (0, dim, 0, curr_width_, 0, NULL,
                                                       NULL, NULL),
                                           NULL);

                /* the PART-information is needed by 'IntersectStrideWithOutline' */
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

            if ((NODE_TYPE (curr_width) == N_num) && (NUM_VAL (curr_width) == 1)) {
                /*
                 * If we have found a (0 -> 1) grid, we can build a N_WLgrid- instead of
                 *  a N_WLgridVar-node.
                 *
                 * CAUTION: We must *not* build a N_WLgrid-node for constant grids in
                 * general!! An example:
                 *
                 *            0 -> b step 4
                 *                   0 -> 3: op0
                 *                   3 -> 4: op1
                 *
                 *          For N_WLgrid-nodes we create code, that executes
                 * unconditionally the whole grid. If the grid is not a (0 -> 1) grid,
                 * this can be dangerous: Let b=10, then we must cut off the grids in the
                 * last loop-pass, because the step (4) is not a divisor of the
                 * stride-width (b=10). For constant strides this is done statically in
                 * 'wltransform: fit'. But for grids != (0 -> 1), belonging to a
                 * WLstriVar-node, we must do this at runtime. Therefore these grids
                 * *must* be N_WLgridVar-nodes.
                 */
                new_grid = MakeWLgrid (0, dim, 0, 1, 0, NULL, NULL, NULL);
                curr_width = FreeNode (curr_width);
            } else {
                new_grid
                  = MakeWLgridVar (0, dim, MakeNum (0), curr_width, NULL, NULL, NULL);
            }

            /* build N_WLstriVar-node of current dimension */
            new_stride = MakeWLstriVar (0, dim, curr_bound1, curr_bound2, curr_step,
                                        new_grid, NULL);

            /* append 'new_stride' to 'parts_stride' */
            if (dim == 0) {
                parts_stride = new_stride;
            } else {
                WLGRIDVAR_NEXTDIM (last_grid) = new_stride;
            }
            last_grid = WLSTRIVAR_CONTENTS (new_stride);
        }

        if (NODE_TYPE (last_grid) == N_WLgrid) {
            WLGRID_CODE (last_grid) = NPART_CODE (parts);
        } else {
            WLGRIDVAR_CODE (last_grid) = NPART_CODE (parts);
        }
        NCODE_USED (NPART_CODE (parts))++;
    }

    DBUG_RETURN (parts_stride);
}

/******************************************************************************
 ******************************************************************************
 **
 ** functions for CheckDisjointness()
 **
 **/

/******************************************************************************
 *
 * function:
 *   int StridesNotDisjoint_OneDim( int lb1, int ub1, int step1, int width1,
 *                                  int lb2, int ub2, int step2, int width2)
 *
 * description:
 *   checks whether the given strides are disjoint.
 *
 *   stride1: lb1 <= iv1 < ub1 STEP step1 WIDTH width1
 *   stride2: lb2 <= iv1 < ub2 STEP step2 WIDTH width2
 *
 *   return value: 0 - disjoint
 *                 1 - not disjoint
 *
 ******************************************************************************/

int
StridesNotDisjoint_OneDim (int lb1, int ub1, int step1, int width1, int lb2, int ub2,
                           int step2, int width2)
{
    int ub, lb, step;
#ifdef DO_NOT_USE_MOD
    int inc1, inc2, iv1, iv2;
#else
    int iv;
#endif
    int not_disjoint = 0;

    DBUG_ENTER ("StridesNotDisjoint_OneDim");

    lb = MAX (lb1, lb2);
    ub = MIN (ub1, ub2);
    step = lcm (step1, step2);
    ub = MIN (ub, lb + step);

#ifdef DO_NOT_USE_MOD
    inc1 = (lb - lb1) % step1;
    if (inc1 < width1) {
        iv1 = lb;
    } else {
        iv1 = lb + step1 - inc1;
        inc1 = 0;
    }

    inc2 = (lb - lb2) % step2;
    if (inc2 < width2) {
        iv2 = lb;
    } else {
        iv2 = lb + step2 - inc2;
        inc2 = 0;
    }

    while ((iv1 < ub) && (iv2 < ub)) {
        if (iv1 == iv2) {
            not_disjoint = 1;
            break;
        }
        if (iv1 < iv2) {
            if (inc1 + 1 < width1) {
                iv1++;
                inc1++;
            } else {
                iv1 += step1;
                inc1 = 0;
            }
        } else {
            if (inc2 + 1 < width2) {
                iv2++;
                inc2++;
            } else {
                iv2 += step2;
                inc2 = 0;
            }
        }
    }
#else
    for (iv = lb; iv < ub; iv++) {
        if (((iv - lb1) % step1 < width1) && ((iv - lb2) % step2 < width2)) {
            not_disjoint = 1;
            break;
        }
    }
#endif

    DBUG_RETURN (not_disjoint);
}

/******************************************************************************
 *
 * function:
 *   int StridesNotDisjoint_AllDims( node *stride1, node *stride2)
 *
 * description:
 *   checks whether the given strides are disjoint (in at least one dimension).
 *
 *   return value: 0 - disjoint
 *                 1 - not disjoint
 *
 ******************************************************************************/

int
StridesNotDisjoint_AllDims (node *stride1, node *stride2)
{
    int not_disjoint = 1;
    node *grid1, *grid2;

    DBUG_ENTER ("StridesNotDisjoint_AllDims");

    while (stride1 != NULL) {
        DBUG_ASSERT ((stride2 != NULL), "stride1 contains more dimensions than stride2");
        grid1 = WLSTRIDE_CONTENTS (stride1);
        grid2 = WLSTRIDE_CONTENTS (stride2);
        DBUG_ASSERT (((grid1 != NULL) && (grid2 != NULL)),
                     "stride with missing grid found");

        if (!StridesNotDisjoint_OneDim (WLSTRIDE_BOUND1 (stride1) + WLGRID_BOUND1 (grid1),
                                        WLSTRIDE_BOUND2 (stride1),
                                        WLSTRIDE_STEP (stride1),
                                        WLGRID_BOUND2 (grid1) - WLGRID_BOUND1 (grid1),
                                        WLSTRIDE_BOUND1 (stride2) + WLGRID_BOUND1 (grid2),
                                        WLSTRIDE_BOUND2 (stride2),
                                        WLSTRIDE_STEP (stride2),
                                        WLGRID_BOUND2 (grid2) - WLGRID_BOUND1 (grid2))) {
            not_disjoint = 0;
            break;
        }

        stride1 = WLGRID_NEXTDIM (grid1);
        stride2 = WLGRID_NEXTDIM (grid2);
    }

    DBUG_RETURN (not_disjoint);
}

/**
 **
 ** functions for CheckDisjointness()
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *   int CheckDisjointness( node *strides)
 *
 * description:
 *   checks whether all strides are pairwise disjoint.
 *
 *   return value: 0 - disjoint
 *                 1 - not disjoint
 *
 ******************************************************************************/

int
CheckDisjointness (node *strides)
{
    node *stride2;
    int not_disjoint = 0;

    DBUG_ENTER ("CheckDisjointness");

    while (strides != NULL) {
        stride2 = WLSTRIDE_NEXT (strides);
        while (stride2 != NULL) {
            if (StridesNotDisjoint_AllDims (strides, stride2)) {
                not_disjoint = 1;
                goto ret;
            }
            stride2 = WLSTRIDE_NEXT (stride2);
        }
        strides = WLSTRIDE_NEXT (strides);
    }

ret:
    DBUG_RETURN (not_disjoint);
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
    node *segs;
    char *fun_names;

    DBUG_ENTER ("SetSegs");

    /*
     * create default configuration
     */
    segs = All (NULL, NULL, cubes, dims, line);
    /*
     * also possible:
     *   segs = Cubes( NULL, NULL, cubes, dims, line);
     * (it might be a good idea to introduce a sac2c-flag for this ...)
     */

    /*
     * create pragma-dependent configuration
     */
    if (pragma != NULL) {
        aps = PRAGMA_WLCOMP_APS (pragma);
        while (aps != NULL) {

#define WLP(fun, str)                                                                    \
    if (strcmp (AP_NAME (EXPRS_EXPR (aps)), str) == 0) {                                 \
        segs = fun (segs, AP_ARGS (EXPRS_EXPR (aps)), cubes, dims, line);                \
    } else
#include "wlpragma_funs.mac"
#undef WLP
            {
                fun_names =
#define WLP(fun, str) " "##str
#include "wlpragma_funs.mac"
#undef WLP
                  ;

                ABORT (line, ("Illegal function name in wlcomp-pragma found."
                              " Currently supported functions are:"
                              "%s",
                              fun_names));
            }

            aps = EXPRS_NEXT (aps);
        }
    }

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * function:
 *   void CheckParams( node *seg)
 *
 * description:
 *   checks whether the parameter of the segment 'seg' are legal.
 *
 ******************************************************************************/

void
CheckParams (node *seg)
{
    int last, first_block, d, j;

    DBUG_ENTER (" CheckParams");

    /* test, whether (bv0 >= bv1 >= bv2 >= ... >= 1), (ubv >= 1) */
    for (d = 0; d < WLSEGX_DIMS (seg); d++) {
        j = WLSEGX_BLOCKS (seg) - 1;
        if ((WLSEGX_BV (seg, j))[d] < 1) {
            ABORT (line, ("Blocking step (%i) is smaller than 1."
                          " Please check parameters of functions in wlcomp-pragma",
                          (WLSEGX_BV (seg, j))[d]));
        }
        last = (WLSEGX_BV (seg, j))[d];
        for (; j >= 0; j--) {
            if ((WLSEGX_BV (seg, j))[d] < last) {
                ABORT (line, ("Inner Blocking step (%i) is smaller than outer one (%i)."
                              " Please check parameters of functions in wlcomp-pragma",
                              (WLSEGX_BV (seg, j))[d], last));
            }
            last = (WLSEGX_BV (seg, j))[d];
        }

        if ((WLSEGX_UBV (seg))[d] < 1) {
            ABORT (line, ("Unrolling-blocking step (%i) is smaller than 1."
                          " Please check parameters of functions in wlcomp-pragma",
                          (WLSEGX_UBV (seg))[d]));
        }
    }

    /*
     * check bv:
     *
     * checks for all bv (bv0, bv1, bv2, ...):
     *  exists k: (forall (d < k): bv_d = 1) and
     *            (forall (d >= k): bv_d >= max(sv_j, ubv_j))
     */
    first_block = 0;
    for (j = 0; j < WLSEGX_BLOCKS (seg); j++) {
        /* goto first dim with (bv_d > 1) */
        d = 0;
        while ((d < WLSEGX_DIMS (seg)) && ((WLSEGX_BV (seg, j))[d] == 1)) {
            d++;
        }

        if (d < WLSEGX_DIMS (seg)) {
            first_block = d;
        }
        for (; d < WLSEGX_DIMS (seg); d++) {
            if ((WLSEGX_BV (seg, j))[d]
                < MAX ((WLSEGX_SV (seg))[d], (WLSEGX_UBV (seg)[d]))) {
                ABORT (line, ("Blocking step (%i) is smaller than stride step (%i),"
                              " unrolling-blocking step (%i) respectively. "
                              "Please check parameters of functions in wlcomp-pragma",
                              (WLSEGX_BV (seg, j))[d], (WLSEGX_SV (seg))[d],
                              (WLSEGX_UBV (seg))[d]));
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
    while ((d < WLSEGX_DIMS (seg)) && ((WLSEGX_UBV (seg))[d] == 1)) {
        d++;
    }

    if (first_block > d) {
        ABORT (line, ("Unrolling-blocking step (%i) is greater than"
                      " most inner blocking step (%i). "
                      "Please check parameters of functions in wlcomp-pragma",
                      (WLSEGX_UBV (seg))[d], (WLSEGX_BV (seg, j))[first_block]));
    }

    for (; d < WLSEGX_DIMS (seg); d++) {
        if ((WLSEGX_UBV (seg))[d] % (WLSEGX_SV (seg))[d] != 0) {
            ABORT (line,
                   ("Stride step (%i) is not a divisor of unrolling-blocking step (%i). "
                    "Please check parameters of functions in wlcomp-pragma",
                    (WLSEGX_SV (seg))[d], (WLSEGX_UBV (seg))[d]));
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 ******************************************************************************
 **
 ** functions for SplitWL()
 **
 **/

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

    DBUG_VOID_RETURN;
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

    if (strides != NULL) {
        if (NODE_TYPE (strides) == N_WLstride) {

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
                            fixpoint = 0; /* no, not a fixpoint yet :( */
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

        } else {
            DBUG_ASSERT ((NODE_TYPE (strides) == N_WLstriVar),
                         "stride with wrong node type found");
            /*
             * nothing to do for non-constant strides
             */
        }
    }

    DBUG_RETURN (strides);
}

/**
 **
 ** functions for SplitWL()
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 ******************************************************************************
 **
 ** functions for BlockWL()
 **
 **/

/******************************************************************************
 *
 * function:
 *   node *BlockStride( node *stride, long *bv, int unroll)
 *
 * description:
 *   returns 'stride' with corrected bounds, blocking levels and
 *     unrolling-flag.
 *   this function is needed after a blocking.
 *
 ******************************************************************************/

node *
BlockStride (node *stride, long *bv, int unroll)
{
    node *curr_stride, *curr_grid, *grids;

    DBUG_ENTER ("BlockStride");

    if (stride != NULL) {

        DBUG_ASSERT ((NODE_TYPE (stride)), "no N_WLstride node found");

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

                        block
                          = MakeWLblock (level, WLSTRIDE_DIM (contents),
                                         WLSTRIDE_BOUND1 (contents),
                                         WLSTRIDE_BOUND2 (contents),
                                         bv[WLSTRIDE_DIM (contents)], NULL, NULL, NULL);

                        /*
                         * unrolling-blocking wanted?
                         */
                        if (unroll > 0) {
                            NODE_TYPE (block) = N_WLublock;
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

        case N_WLstriVar:
            /*
             * not yet implemented :-(
             */
            for (d = 0; d < dims; d++) {
                bv[d] = 1; /* no blocking -> reset blocking vector */
            }
            break;

        default:
            DBUG_ASSERT ((0), "wrong node type");
        }
    }

    DBUG_RETURN (stride);
}

/**
 **
 ** functions for BlockWL()
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 ******************************************************************************
 **
 ** functions for MergeWL()
 **
 **/

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

    DBUG_VOID_RETURN;
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
                            fixpoint = 0;
                        }
                        if (i_grid2 != NULL) {
                            new_grids = InsertWLnodes (new_grids, i_grid2);
                            WLGRID_MODIFIED (grid2) = new_grids;
                            fixpoint = 0;
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

        case N_WLgridVar:
            /*
             * nothing to do for non-constant strides
             */
            break;

        case N_WLstriVar:
            /*
             * nothing to do for non-constant strides
             */
            break;

        default:
            DBUG_ASSERT ((0), "wrong node type");
        }

        node1 = WLNODE_NEXT (node1);
    }

    DBUG_RETURN (nodes);
}

/**
 **
 ** functions for MergeWL()
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 ******************************************************************************
 **
 ** functions for OptWL()
 **
 **/

/******************************************************************************
 *
 * function:
 *   int IsEqualWLnodes( node *tree1, node *tree2)
 *
 * description:
 *   returns 1 if the N_WL...-trees 'tree1' and 'tree2' are equal.
 *   returns 0 otherwise.
 *
 *   remark: we can not use CompareWLnodes() here, because that function only
 *           compares the first level of dimensions.
 *           In contrast here we must compare the *whole* trees --- all block
 *           levels, and even the code ...
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
        equal = ((tree1 == NULL) && (tree2 == NULL));
    }

    DBUG_RETURN (equal);
}

/******************************************************************************
 *
 * function:
 *   node *OptWL( node *nodes)
 *
 * description:
 *   returns the optimized N_WL...-tree 'nodes'.
 *
 ******************************************************************************/

node *
OptWL (node *nodes)
{
    node *next, *grids, *comp1, *comp2;
    int offset;

    DBUG_ENTER ("OptWL");

    if (nodes != NULL) {

        if (NODE_TYPE (nodes) != N_WLstriVar) {

            /*
             * optimize the next node
             */
            next = WLNODE_NEXT (nodes) = OptWL (WLNODE_NEXT (nodes));

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
                    comp1 = WLBLOCK_NEXTDIM (nodes) = OptWL (WLBLOCK_NEXTDIM (nodes));
                    if (next != NULL) {
                        comp2 = WLBLOCK_NEXTDIM (next);
                    }
                } else {
                    /*
                     * compare CONTENTS (NEXTDIM is NULL)
                     */
                    comp1 = WLBLOCK_CONTENTS (nodes) = OptWL (WLBLOCK_CONTENTS (nodes));
                    if (next != NULL) {
                        comp2 = WLBLOCK_CONTENTS (next);
                    }
                }
                break;

            case N_WLstride:
                comp1 = WLSTRIDE_CONTENTS (nodes) = OptWL (WLSTRIDE_CONTENTS (nodes));
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
                    comp1 = WLGRID_NEXTDIM (nodes) = OptWL (WLGRID_NEXTDIM (nodes));
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
                    if (((comp1 != NULL) && (NODE_TYPE (comp1) != N_Ncode))
                          ? IsEqualWLnodes (comp1, comp2)
                          : (comp1 == comp2)) {
                        /* concate 'nodes' and 'next' */
                        WLNODE_BOUND2 (nodes) = WLNODE_BOUND2 (next);
                        /* free useless data in 'next' */
                        WLNODE_NEXT (nodes) = FreeNode (WLNODE_NEXT (nodes));
                    }
                }
            }

        } else {
            /*
             * nothing to do for non-constant strides
             */
        }
    }

    DBUG_RETURN (nodes);
}

/**
 **
 ** functions for OptWL()
 **
 ******************************************************************************
 ******************************************************************************/

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

    if (nodes != NULL) {

        if (NODE_TYPE (nodes) != N_WLstriVar) {
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
                            unroll = GetMaxUnroll (WLBLOCK_NEXTDIM (tmp), curr_dim);
                        } else {
                            unroll = GetMaxUnroll (WLBLOCK_CONTENTS (tmp), curr_dim);
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
        } else {
            /*
             * nothing to do for non-constant strides
             */
        }
    }

    DBUG_RETURN (nodes);
}

/******************************************************************************
 ******************************************************************************
 **
 ** functions for NormWL()
 **
 **/

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
 *   node *NormWL( node *nodes, int *idx_max)
 *
 * description:
 *   returns the normalized N_WL...-tree 'nodes'.
 *   'idx_max' is the supremum of the index-vector.
 *
 ******************************************************************************/

node *
NormWL (node *nodes, int *idx_max)
{
    DBUG_ENTER ("NormWL");

    if (NODE_TYPE (nodes) != N_WLstriVar) {
        nodes = NormalizeWLnodes (nodes, idx_max);
    } else {
        /*
         * nothing to do for non-constant strides
         */
    }

    DBUG_RETURN (nodes);
}

/**
 **
 ** functions for NormWL()
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 ******************************************************************************
 **
 **   functions for ComputeOneCube()
 **
 **/

/******************************************************************************
 *
 * function:
 *   node *GenerateCompleteGrid( node *stride_var)
 *
 * description:
 *   Supplements missings parts of the grid in 'stride_var'.
 *
 *   Example (with shape [300,300]):
 *
 *         60 -> 200 step[0] 50
 *                     40 -> 50:  60 -> 200 step[1] 50
 *                                            40 -> 50: op0
 *
 *   =>>
 *
 *        100 -> 200 step[0] 50
 *                      0 -> 10: 100 -> 200 step[1] 50
 *                                             0 -> 10: op0
 *                                            10 -> 50: noop
 *                     10 -> 50: noop
 *
 *   This function is called by 'ComputeOneCube'.
 *
 ******************************************************************************/

node *
GenerateCompleteGrid (node *stride_var)
{
    node *grid_var;

    DBUG_ENTER ("GenerateCompleteGrid");

    if (stride_var != NULL) {

        grid_var = WLSTRIANY_CONTENTS (stride_var);

        if (NODE_TYPE (stride_var) == N_WLstride) {
            DBUG_ASSERT ((NODE_TYPE (grid_var) == N_WLgrid), "wrong node type found");

            /*
             * is the grid incomplete?
             */
            if (WLGRID_BOUND2 (grid_var) - WLGRID_BOUND1 (grid_var)
                < WLSTRIDE_STEP (stride_var)) {
                WLSTRIDE_BOUND1 (stride_var) += WLGRID_BOUND1 (grid_var);
                WLGRID_BOUND2 (grid_var) -= WLGRID_BOUND1 (grid_var);
                WLGRID_BOUND1 (grid_var) = 0;

                WLGRID_NEXT (grid_var)
                  = MakeWLgrid (0, WLGRID_DIM (grid_var), WLGRID_BOUND2 (grid_var),
                                WLSTRIDE_STEP (stride_var), 0, NULL, NULL, NULL);
            }

            /*
             * next dim
             */
            WLGRID_NEXTDIM (grid_var) = GenerateCompleteGrid (WLGRID_NEXTDIM (grid_var));

        } else { /* NODE_TYPE( stride_var) == N_WLstriVar */

            /*
             * CAUTION: the grid can be a N_WLgrid node!!!
             */

            if (NODE_TYPE (grid_var) == N_WLgrid) {
                DBUG_ASSERT ((WLGRID_BOUND1 (grid_var) == 0), "bound1 not zero");

                /*
                 * is the grid incomplete?
                 */
                if ((NODE_TYPE (WLSTRIVAR_STEP (stride_var)) != N_num)
                    || (WLGRID_BOUND2 (grid_var)
                        < NUM_VAL (WLSTRIVAR_STEP (stride_var)))) {

                    WLGRID_NEXT (grid_var)
                      = MakeWLgridVar (WLGRID_LEVEL (grid_var), WLGRID_DIM (grid_var),
                                       MakeNum (WLGRID_BOUND2 (grid_var)),
                                       DupNode (WLSTRIVAR_STEP (stride_var)), NULL, NULL,
                                       NULL);
                }

                /*
                 * next dim
                 */
                WLGRID_NEXTDIM (grid_var)
                  = GenerateCompleteGrid (WLGRID_NEXTDIM (grid_var));

            } else { /* NODE_TYPE( grid_var) == N_WLgridVar */
                DBUG_ASSERT (((NODE_TYPE (WLGRIDVAR_BOUND1 (grid_var)) == N_num)
                              && (NUM_VAL (WLGRIDVAR_BOUND1 (grid_var)) == 0)),
                             "bound1 not zero");

                /*
                 * is the grid incomplete?
                 */
                if ((NODE_TYPE (WLGRIDVAR_BOUND2 (grid_var)) != N_num)
                    || (NODE_TYPE (WLSTRIVAR_STEP (stride_var)) != N_num)
                    || (NUM_VAL (WLGRIDVAR_BOUND2 (grid_var))
                        < NUM_VAL (WLSTRIVAR_STEP (stride_var)))) {

                    WLGRIDVAR_NEXT (grid_var)
                      = MakeWLgridVar (WLGRIDVAR_LEVEL (grid_var),
                                       WLGRIDVAR_DIM (grid_var),
                                       DupNode (WLGRIDVAR_BOUND2 (grid_var)),
                                       DupNode (WLSTRIVAR_STEP (stride_var)), NULL, NULL,
                                       NULL);
                }

                /*
                 * next dim
                 */
                WLGRIDVAR_NEXTDIM (grid_var)
                  = GenerateCompleteGrid (WLGRIDVAR_NEXTDIM (grid_var));
            }
        }
    }

    DBUG_RETURN (stride_var);
}

/******************************************************************************
 *
 * function:
 *   node *GenerateShapeStrides( int dim, int dims, shpseg* shape)
 *
 * description:
 *   Returns strides/grids of the size found in 'shape'.
 *
 *   This function is called by 'GenerateCompleteDomain',
 *    'GenerateCompleteDomainVar'.
 *
 ******************************************************************************/

node *
GenerateShapeStrides (int dim, int dims, shpseg *shape)
{
    node *new_grid, *strides = NULL;

    DBUG_ENTER ("GenerateShapeStrides");

    if (dim < dims) {
        new_grid = MakeWLgrid (0, dim, 0, 1, 0,
                               GenerateShapeStrides (dim + 1, dims, shape), NULL, NULL);
        strides
          = MakeWLstride (0, dim, 0, SHPSEG_SHAPE (shape, dim), 1, 0, new_grid, NULL);
    }

    DBUG_RETURN (strides);
}

/******************************************************************************
 *
 * function:
 *   node *GenerateCompleteDomain( node *strides,
 *                                 int dims, shpseg *shape)
 *
 * description:
 *   Supplements strides/grids for the complement of 'stride'.
 *
 *   For constant strides we must *not* optimize and merge strides, because
 *   'BlockWL()' can not handle them!! We must create simple cubes instead.
 *   Example (with shape [10,10]):
 *
 *       5 -> 10 step[0] 2
 *                  0 -> 1: 0 -> 5  step[1] 1
 *                                     0 -> 1: op
 *
 *     is *not* converted into (the following is not a cube!!)
 *
 *       0 ->  5 step[0] 1
 *                  0 -> 1: 0 -> 10 step[1] 1
 *                                     0 -> 1: init/copy/noop
 *       5 -> 10 step[0] 2
 *                  0 -> 1: 0 -> 5  step[1] 1
 *                                     0 -> 1: op
 *                          5 -> 10 step[1] 1
 *                                     0 -> 1: init/copy/noop
 *                  1 -> 2: 0 -> 10 step[1] 1
 *                                     0 -> 1: init/copy/noop
 *
 *     but into
 *
 *       0 ->  5 step[0] 1
 *                  0 -> 1: 0 -> 10 step[1] 1
 *                                     0 -> 1: init/copy/noop
 *       5 -> 10 step[0] 2
 *                  0 -> 1: 0 -> 5  step[1] 1
 *                                     0 -> 1: op
 *                  1 -> 2: 0 -> 5  step[1] 1
 *                                     0 -> 1: init/copy/noop
 *       5 -> 10 step[0] 1
 *                  0 -> 1: 5 -> 10 step[1] 1
 *                                     0 -> 1: init/copy/noop
 *
 *   This function is called by 'ComputeOneCube'.
 *
 ******************************************************************************/

node *
GenerateCompleteDomain (node *strides, int dims, shpseg *shape)
{
    node *new_strides, *comp_strides, *stride, *grid, *comp_stride, *comp_grid,
      *last_comp_grid, *new_stride, *new_grid, *dup_strides, *last_dup_grid, *next_dim;

    DBUG_ENTER ("GenerateCompleteDomain");

    DBUG_ASSERT ((shape != NULL), "no shape found");

    /*
     * we duplicate 'strides'
     *  -> later on we use this to generate complement strides
     */
    DBUG_ASSERT ((WLSTRIDE_NEXT (strides) == NULL), "more than one stride found");
    comp_strides = DupNode (strides);
    /*
     * in the duplicated chain we set all steps to '1'
     */
    comp_stride = comp_strides;
    while (comp_stride != NULL) {
        WLSTRIDE_STEP (comp_stride) = 1;
        comp_grid = WLSTRIDE_CONTENTS (comp_stride);
        WLSTRIDE_BOUND1 (comp_stride) += WLGRID_BOUND1 (comp_grid);
        WLGRID_BOUND1 (comp_grid) = 0;
        WLGRID_BOUND2 (comp_grid) = 1;
        comp_stride = WLGRID_NEXTDIM (comp_grid);
    }
    /*
     * this chain is base for complements.
     *  -> we must remove the code.
     */
    WLGRID_CODE (comp_grid) = NULL;

    new_strides = NULL;
    stride = strides;
    comp_stride = comp_strides;
    last_comp_grid = NULL;
    while (stride != NULL) {
        DBUG_ASSERT ((NODE_TYPE (stride) == N_WLstride), "no constant stride found");

        grid = WLSTRIDE_CONTENTS (stride);
        comp_grid = WLSTRIDE_CONTENTS (comp_stride);
        DBUG_ASSERT ((NODE_TYPE (grid) == N_WLgrid), "no constant grid found");

        /*
         * normalize the bounds
         */
        WLSTRIDE_BOUND1 (stride) += WLGRID_BOUND1 (grid);
        WLGRID_BOUND2 (grid) -= WLGRID_BOUND1 (grid);
        WLGRID_BOUND1 (grid) = 0;

        /*
         * insert lower part of complement
         */
        if (WLSTRIDE_BOUND2 (stride) < SHPSEG_SHAPE (shape, WLSTRIDE_DIM (stride))) {
            if (last_comp_grid != NULL) {
                /*
                 * duplicate 'comp_strides' from root til 'last_comp_grid'.
                 */
                next_dim = WLGRID_NEXTDIM (last_comp_grid);
                WLGRID_NEXTDIM (last_comp_grid) = NULL;
                dup_strides = DupNode (comp_strides);
                WLGRID_NEXTDIM (last_comp_grid) = next_dim;
                /*
                 * go to duplicated 'last_comp_grid'
                 */
                last_dup_grid = WLSTRIDE_CONTENTS (dup_strides);
                while (WLGRID_NEXTDIM (last_dup_grid) != NULL) {
                    last_dup_grid = WLSTRIDE_CONTENTS (WLGRID_NEXTDIM (last_dup_grid));
                }
            }

            /*
             * generate new stride/grid
             */
            new_grid
              = MakeWLgrid (0, WLGRID_DIM (grid), 0, 1, 0,
                            GenerateShapeStrides (WLGRID_DIM (grid) + 1, dims, shape),
                            NULL, NULL);
            new_stride = MakeWLstride (0, WLSTRIDE_DIM (stride), WLSTRIDE_BOUND2 (stride),
                                       SHPSEG_SHAPE (shape, WLSTRIDE_DIM (stride)), 1, 0,
                                       new_grid, NULL);

            /*
             * append new stride/grid to duplicated 'comp_strides'
             */
            if (last_comp_grid != NULL) {
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
        if (WLSTRIDE_BOUND1 (stride) > 0) {
            if (last_comp_grid != NULL) {
                /*
                 * duplicate 'comp_strides' from root til 'last_comp_grid'.
                 */
                next_dim = WLGRID_NEXTDIM (last_comp_grid);
                WLGRID_NEXTDIM (last_comp_grid) = NULL;
                dup_strides = DupNode (comp_strides);
                WLGRID_NEXTDIM (last_comp_grid) = next_dim;
                /*
                 * go to duplicated 'last_comp_grid'
                 */
                last_dup_grid = WLSTRIDE_CONTENTS (dup_strides);
                while (WLGRID_NEXTDIM (last_dup_grid) != NULL) {
                    last_dup_grid = WLSTRIDE_CONTENTS (WLGRID_NEXTDIM (last_dup_grid));
                }
            }

            /*
             * generate new stride/grid
             */
            new_grid
              = MakeWLgrid (0, WLGRID_DIM (grid), 0, 1, 0,
                            GenerateShapeStrides (WLGRID_DIM (grid) + 1, dims, shape),
                            NULL, NULL);
            new_stride = MakeWLstride (0, WLSTRIDE_DIM (stride), 0,
                                       WLSTRIDE_BOUND1 (stride), 1, 0, new_grid, NULL);

            /*
             * append new stride/grid to duplicated 'comp_strides'
             */
            if (last_comp_grid != NULL) {
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
        if (WLGRID_BOUND2 (grid) - WLGRID_BOUND1 (grid) < WLSTRIDE_STEP (stride)) {
            WLGRID_NEXT (grid)
              = MakeWLgrid (0, WLGRID_DIM (grid), WLGRID_BOUND2 (grid),
                            WLSTRIDE_STEP (stride), 0,
                            DupNode (WLGRID_NEXTDIM (comp_grid)), NULL, NULL);
        }

        /*
         * next dim
         */
        stride = WLGRID_NEXTDIM (grid);
        comp_stride = WLGRID_NEXTDIM (comp_grid);
        last_comp_grid = comp_grid;
    }

    /*
     * insert completed stride/grid into 'new_strides'
     */
    new_strides = InsertWLnodes (new_strides, strides);

    /*
     * the copy of 'strides' is useless now
     */
    comp_strides = FreeTree (comp_strides);

    DBUG_RETURN (new_strides);
}

/******************************************************************************
 *
 * function:
 *   node *GenerateCompleteDomainVar( node *stride_var, int dims, shpseg *shape)
 *
 * description:
 *   Supplements strides/grids for the complement of 'stride_var'.
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
 *                                     0 -> 1: init/copy/noop
 *       5 -> 10 step[0] 2
 *                  0 -> 1: 0 -> a  step[1] 1
 *                                     0 -> 1: init/copy/noop
 *                          a -> b  step[1] 1
 *                                     0 -> 1: op
 *                          b -> 10 step[1] 1
 *                                     0 -> 1: init/copy/noop
 *                  1 -> 2: 0 -> 10 step[1] 1
 *                                     0 -> 1: init/copy/noop
 *
 *   This function is called by 'ComputeOneCube'.
 *
 ******************************************************************************/

node *
GenerateCompleteDomainVar (node *stride_var, int dims, shpseg *shape)
{
    node *grid_var, *new_grid;

    DBUG_ENTER ("GenerateCompleteDomainVar");

    DBUG_ASSERT ((shape != NULL), "no shape found");

    if (stride_var != NULL) {
        DBUG_ASSERT ((NODE_TYPE (stride_var) == N_WLstriVar), "no variable stride found");

        grid_var = WLSTRIANY_CONTENTS (stride_var);
        /*
         * CAUTION: the grid can be a N_WLgrid node!!!
         */

        if (NODE_TYPE (grid_var) == N_WLgrid) {

            /*
             * is the grid incomplete?
             */
            if ((NODE_TYPE (WLSTRIVAR_STEP (stride_var)) != N_num)
                || (WLGRID_BOUND2 (grid_var) < NUM_VAL (WLSTRIVAR_STEP (stride_var)))) {
                WLGRID_NEXT (grid_var)
                  = MakeWLgridVar (WLGRID_LEVEL (grid_var), WLGRID_DIM (grid_var),
                                   MakeNum (WLGRID_BOUND2 (grid_var)),
                                   DupNode (WLSTRIVAR_STEP (stride_var)),
                                   GenerateShapeStrides (WLGRID_DIM (grid_var) + 1, dims,
                                                         shape),
                                   NULL, NULL);
            }

            /*
             * next dim
             */
            WLGRID_NEXTDIM (grid_var)
              = GenerateCompleteDomainVar (WLGRID_NEXTDIM (grid_var), dims, shape);

            /*
             * append lower part of complement
             */
            if ((NODE_TYPE (WLSTRIVAR_BOUND2 (stride_var)) != N_num)
                || (NUM_VAL (WLSTRIVAR_BOUND2 (stride_var))
                    < SHPSEG_SHAPE (shape, WLSTRIVAR_DIM (stride_var)))) {
                new_grid = MakeWLgrid (0, WLGRID_DIM (grid_var), 0, 1, 0,
                                       GenerateShapeStrides (WLGRID_DIM (grid_var) + 1,
                                                             dims, shape),
                                       NULL, NULL);
                WLSTRIVAR_NEXT (stride_var)
                  = MakeWLstriVar (WLSTRIVAR_LEVEL (stride_var),
                                   WLSTRIVAR_DIM (stride_var),
                                   DupNode (WLSTRIVAR_BOUND2 (stride_var)),
                                   MakeNum (
                                     SHPSEG_SHAPE (shape, WLSTRIVAR_DIM (stride_var))),
                                   MakeNum (1), new_grid, NULL);
            }

            /*
             * insert upper part of complement
             */
            if ((NODE_TYPE (WLSTRIVAR_BOUND1 (stride_var)) != N_num)
                || (NUM_VAL (WLSTRIVAR_BOUND1 (stride_var)) > 0)) {
                new_grid = MakeWLgrid (0, WLGRID_DIM (grid_var), 0, 1, 0,
                                       GenerateShapeStrides (WLGRID_DIM (grid_var) + 1,
                                                             dims, shape),
                                       NULL, NULL);
                stride_var = MakeWLstriVar (WLSTRIVAR_LEVEL (stride_var),
                                            WLSTRIVAR_DIM (stride_var), MakeNum (0),
                                            DupNode (WLSTRIVAR_BOUND1 (stride_var)),
                                            MakeNum (1), new_grid, stride_var);
            }

        } else { /* NODE_TYPE( grid_var) == N_WLgridVar */

            /*
             * is the grid incomplete?
             */
            if ((NODE_TYPE (WLGRIDVAR_BOUND2 (grid_var)) != N_num)
                || (NODE_TYPE (WLSTRIVAR_STEP (stride_var)) != N_num)
                || (NUM_VAL (WLGRIDVAR_BOUND2 (grid_var))
                    < NUM_VAL (WLSTRIVAR_STEP (stride_var)))) {
                WLGRIDVAR_NEXT (grid_var)
                  = MakeWLgridVar (WLGRIDVAR_LEVEL (grid_var), WLGRIDVAR_DIM (grid_var),
                                   DupNode (WLGRIDVAR_BOUND2 (grid_var)),
                                   DupNode (WLSTRIVAR_STEP (stride_var)),
                                   GenerateShapeStrides (WLGRIDVAR_DIM (grid_var) + 1,
                                                         dims, shape),
                                   NULL, NULL);
            }

            /*
             * next dim
             */
            WLGRIDVAR_NEXTDIM (grid_var)
              = GenerateCompleteDomainVar (WLGRIDVAR_NEXTDIM (grid_var), dims, shape);

            /*
             * append lower part of complement
             */
            if ((NODE_TYPE (WLSTRIVAR_BOUND2 (stride_var)) != N_num)
                || (NUM_VAL (WLSTRIVAR_BOUND2 (stride_var))
                    < SHPSEG_SHAPE (shape, WLSTRIVAR_DIM (stride_var)))) {
                new_grid = MakeWLgrid (0, WLGRIDVAR_DIM (grid_var), 0, 1, 0,
                                       GenerateShapeStrides (WLGRIDVAR_DIM (grid_var) + 1,
                                                             dims, shape),
                                       NULL, NULL);
                WLSTRIVAR_NEXT (stride_var)
                  = MakeWLstriVar (WLSTRIVAR_LEVEL (stride_var),
                                   WLSTRIVAR_DIM (stride_var),
                                   DupNode (WLSTRIVAR_BOUND2 (stride_var)),
                                   MakeNum (
                                     SHPSEG_SHAPE (shape, WLSTRIVAR_DIM (stride_var))),
                                   MakeNum (1), new_grid, NULL);
            }

            /*
             * insert upper part of complement
             */
            if ((NODE_TYPE (WLSTRIVAR_BOUND1 (stride_var)) != N_num)
                || (NUM_VAL (WLSTRIVAR_BOUND1 (stride_var)) > 0)) {
                new_grid = MakeWLgrid (0, WLGRIDVAR_DIM (grid_var), 0, 1, 0,
                                       GenerateShapeStrides (WLGRIDVAR_DIM (grid_var) + 1,
                                                             dims, shape),
                                       NULL, NULL);
                stride_var = MakeWLstriVar (WLSTRIVAR_LEVEL (stride_var),
                                            WLSTRIVAR_DIM (stride_var), MakeNum (0),
                                            DupNode (WLSTRIVAR_BOUND1 (stride_var)),
                                            MakeNum (1), new_grid, stride_var);
            }
        }
    }

    DBUG_RETURN (stride_var);
}

/******************************************************************************
 *
 * function:
 *   node* ComputeOneCube( node *stride_var, WithOpType wltype,
 *                         int dims, shpseg *shape)
 *
 * description:
 *   If the with-loop contains one part/generator only, we must supplement
 *   new generators for the complement.
 *
 * remark:
 *   The new generators contain no pointer to a code-block. We inspect the
 *   type of the with-loop (WO_genarray, WO_modarray, WO_fold...) to decide
 *   whether we must ...
 *     ... initialize the array-part with 0 (WO_genarray -> 'init'),
 *     ... copy the source-array (WO_modarray -> 'copy'),
 *     ... do nothing (WO_fold -> 'noop').
 *
 ******************************************************************************/

node *
ComputeOneCube (node *stride_var, WithOpType wltype, int dims, shpseg *shape)
{
    DBUG_ENTER ("ComputeOneCube");

    if ((wltype == WO_genarray) || (wltype == WO_modarray)) {
        if (NODE_TYPE (stride_var) == N_WLstriVar) {
            stride_var = GenerateCompleteDomainVar (stride_var, dims, shape);
        } else { /* N_WLstride */
            stride_var = GenerateCompleteDomain (stride_var, dims, shape);
        }
    } else { /* WO_fold... */
        stride_var = GenerateCompleteGrid (stride_var);
    }

    DBUG_RETURN (stride_var);
}

/**
 **
 ** functions for ComputeOneCube()
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 ******************************************************************************
 **
 **   functions for ComputeCubes()
 **
 **/

/******************************************************************************
 *
 * function:
 *   int TestAndDivideStrides( node *stride1, node *stride2,
 *                             node **divided_stridea, node **divided_strideb)
 *
 * description:
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

int
TestAndDivideStrides (node *stride1, node *stride2, node **divided_stridea,
                      node **divided_strideb)
{
    node *grid1, *grid2, *trav_d_stride1a, *trav_d_stride1b, *trav_d_stride2a,
      *trav_d_stride2b, *divided_stride1a, *divided_stride1b, *divided_stride2a,
      *divided_stride2b;
    int bound11, bound21, grid1_b1, grid1_b2, bound12, bound22, grid2_b1, grid2_b2, head1,
      rear1, head2, rear2, i_bound1, i_bound2, i_offset1, i_offset2, offset;
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
             * Note: (i_offset_1 > grid1_b1) means, that the stride1 must be split
             *        in two parts to fit the new upper bound in the current dim.
             *       Then the *projections* of stride1, stride2 can not be disjoint,
             *        therefore stride1, stride2 must have disjoint outlines!!!
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
            } else {
                if (i_bound1 + grid2_b1 - i_offset2 >= i_bound2) {
                    /*
                     * the intersection of 'stride2' with the outline of 'stride1' is
                     * empty
                     *   -> dividing stride2
                     */
                    WLSTRIDE_BOUND2 (trav_d_stride2a) = bound11;

                    WLSTRIDE_BOUND1 (trav_d_stride2b) = bound21;
                    offset
                      = GridOffset (bound21, bound12, WLSTRIDE_STEP (stride2), grid2_b2);
                    DBUG_ASSERT ((offset <= grid2_b1), "offset is inconsistant");
                    WLGRID_BOUND1 (WLSTRIDE_CONTENTS (trav_d_stride2b)) -= offset;
                    WLGRID_BOUND2 (WLSTRIDE_CONTENTS (trav_d_stride2b)) -= offset;

                    result = 2;
                    trav_d_stride2a = NormalizeStride_1 (trav_d_stride2a);
                    trav_d_stride2b = NormalizeStride_1 (trav_d_stride2b);
                }
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

        /* next dim */
        stride1 = WLGRID_NEXTDIM (grid1);
        stride2 = WLGRID_NEXTDIM (grid2);
        trav_d_stride1a = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (trav_d_stride1a));
        trav_d_stride1b = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (trav_d_stride1b));
        trav_d_stride2a = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (trav_d_stride2a));
        trav_d_stride2b = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (trav_d_stride2b));
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
 * function:
 *   int IntersectStrideWithOutline( node *stride1, node *stride2,
 *                                   node **i_stride1, node **i_stride2)
 *
 * description:
 *   returns in 'i_stride1' and 'i_stride2' the part of 'stride1', 'stride2'
 *     respectively that lies in a common cube.
 *   possibly present next nodes in 'stride1' or 'stride2' are ignored.
 *   the return value is 1 if and only if the intersection is non-empty.
 *
 *   if interested in the return value only, call this function with
 *     ('i_stride1' == NULL), ('i_stride2' == NULL).
 *
 ******************************************************************************/

int
IntersectStrideWithOutline (node *stride1, node *stride2, node **i_stride1,
                            node **i_stride2)
{
    node *grid1, *grid2, *trav_i_stride1, *trav_i_stride2;
    int bound11, bound21, grid1_b1, grid1_b2, bound12, bound22, grid2_b1, grid2_b2, head1,
      rear1, head2, rear2, i_bound1, i_bound2, i_offset1, i_offset2;
    int result = 1;

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

        if (/* are the projection outlines of 'stride1' and 'stride2' not disjoint? */
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
             * Note: (i_offset_1 > grid1_b1) means, that the stride1 must be split
             *        in two parts to fit the new upper bound in the current dim.
             *       Then the *projections* of stride1, stride2 can not be disjoint,
             *        therefore stride1, stride2 must have disjoint outlines!!!
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
                 * The following parts of the 'cube generation' can not handle this case!
                 * Therefore we will stop here!!
                 *
                 * remark: If this assertion fails, there is a bug in the 'first step'
                 *         of 'ComputeCubes()' !!!
                 */

                DBUG_ASSERT ((0),
                             ("must resign:"
                              " intersection of outline(stride1) and outline(stride2) is "
                              "non-empty,"
                              " while intersection of outline(stride1) and stride2, or"
                              " intersection of stride1 and outline(stride2) is empty "
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
            result = 0;

            /* we can give up here */
            break;
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
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   int IsSubsetStride( node *stride1, node *stride2)
 *
 * description:
 *   determines, whether stride2 is a subset of stride1 (return value 1) or
 *   stride1 is a subset of stride2 (return value 2) or nothing of these
 *   (return value 0).
 *
 *   this function is used in EleminateDuplicatesAndAdjustBounds().
 *
 ******************************************************************************/

int
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
 * function:
 *   int AdjustBounds( node **stride1, node **stride2)
 *
 * description:
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

int
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
 * function:
 *   node* EleminateDuplicatesAndAdjustBounds( node *strides)
 *
 * description:
 *   When ComputeCubes() splits the strides by using IntersectStrideWithOutline()
 *   this may create non-disjoint index vector sets A and B ...
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
 *     128->306 step 9, 7->9: op2 <- intersection of 'op2' and outline('op3')  (*1*)
 *     137->306 step 9, 7->9: op2 <- intersection of 'op2' and outline('op4')  (*2*)
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

node *
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
    node *new_strides, *stride1, *stride2, *i_stride1, *i_stride2, *divided_stridea,
      *divided_strideb, *remain, *last_remain, *last_stride1, *tmp;
    int fixpoint, res;

    DBUG_ENTER ("ComputeCubes");

    DBUG_ASSERT ((NODE_TYPE (strides) == N_WLstride), "wrong node type found");

    /*
     * first step:
     * -----------
     *
     * divide strides
     *  -> for every two strides (stride1, stride2) the following property is hold:
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
        fixpoint = 1;
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
                    fixpoint = 0;
                    WLSTRIDE_MODIFIED (stride1) = divided_stridea;
                } else {
                    if (res == 2) {
                        fixpoint = 0;
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
        fixpoint = 1;
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
                        fixpoint = 0;
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
                        fixpoint = 0;
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
 **   functions for ComputeCubes()
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 ******************************************************************************
 **
 **   functions for InferParams()
 **
 **/

/******************************************************************************
 *
 * function:
 *   node *InferInnerStep( node *nodes, int curr_dim, int dims)
 *
 * description:
 *   Infers for each outer WLblock-, WLublock- or WLstride-node the
 *   the unroll-information (max(ublock,step)) and stores it in INNERSTEP.
 *
 ******************************************************************************/

node *
InferInnerStep (node *nodes, int curr_dim, int dims)
{
    node *grids, *tmp;

    DBUG_ENTER ("InferInnerStep");

    if (curr_dim < dims) {
        /*
         * traverse the whole chain
         */
        tmp = nodes;
        while (tmp != NULL) {

            switch (NODE_TYPE (tmp)) {
            case N_WLblock:
                if (WLBLOCK_NEXTDIM (tmp) != NULL) {
                    /*
                     * inspect next dimension;
                     *   compute unrolling information and store it in WLBLOCK_INNERSTEP;
                     */
                    DBUG_ASSERT ((WLBLOCK_CONTENTS (tmp) == NULL),
                                 "next blocking *and* inner of block found");
                    WLBLOCK_NEXTDIM (tmp)
                      = InferInnerStep (WLBLOCK_NEXTDIM (tmp), curr_dim + 1, dims);

                    WLBLOCK_INNERSTEP (tmp)
                      = GetMaxUnroll (WLBLOCK_NEXTDIM (tmp), curr_dim);
                } else {
                    /*
                     * compute unrolling information and store it in WLBLOCK_INNERSTEP.
                     */
                    DBUG_ASSERT ((WLBLOCK_CONTENTS (tmp) != NULL),
                                 "inner of block not found");

                    WLBLOCK_INNERSTEP (tmp)
                      = GetMaxUnroll (WLBLOCK_CONTENTS (tmp), curr_dim);
                }
                break;

            case N_WLublock:
                if (WLUBLOCK_NEXTDIM (tmp) != NULL) {
                    /*
                     * inspect next dimension
                     */
                    DBUG_ASSERT ((WLUBLOCK_CONTENTS (tmp) == NULL),
                                 "next ublocking *and* inner of ublock found");
                    WLUBLOCK_NEXTDIM (tmp)
                      = InferInnerStep (WLUBLOCK_NEXTDIM (tmp), curr_dim + 1, dims);
                }

                /*
                 * compute unroll-information and store it in WLUBLOCK_INNERSTEP
                 */
                WLUBLOCK_INNERSTEP (tmp) = WLUBLOCK_STEP (tmp);
                break;

            case N_WLstride:
                grids = WLSTRIDE_CONTENTS (tmp);
                if (curr_dim < dims - 1) {
                    /*
                     * inspect all grids in next dimension;
                     *   get unrolling information
                     */
                    while (grids != NULL) {
                        WLGRID_NEXTDIM (grids)
                          = InferInnerStep (WLGRID_NEXTDIM (grids), curr_dim + 1, dims);
                        grids = WLGRID_NEXT (grids);
                    }
                }

                /*
                 * get unroll-information and store it in WL..._INNERSTEP
                 */
                WLSTRIDE_INNERSTEP (tmp) = WLSTRIDE_STEP (tmp);
                break;

            case N_WLstriVar: /* non-constant stride */
            case N_WLgridVar: /* non-constant grids */
                /*
                 * not yet implemented :-(
                 */
                break;

            default:
                DBUG_ASSERT ((0), "wrong node type");
            }
            tmp = WLNODE_NEXT (tmp);
        }
    }

    DBUG_RETURN (nodes);
}

/******************************************************************************
 *
 * function:
 *   int IsHom( node *wlnode, int sv_d)
 *
 * description:
 *   Infers, whether 'wlnode' has a homogenous grid or not.
 *
 ******************************************************************************/

int
IsHom (node *wlnode, int sv_d)
{
    int width;

    DBUG_ENTER ("IsHom");

    width = WLNODE_BOUND2 (wlnode) - WLNODE_BOUND1 (wlnode);
    DBUG_ASSERT ((WLNODE_INNERSTEP (wlnode) > 0), "illegal INNERSTEP found");
    DBUG_ASSERT ((sv_d % WLNODE_INNERSTEP (wlnode) == 0),
                 "INNERSTEP is not a divisor of SV");
    DBUG_RETURN (((width % sv_d) == 0));
}

/******************************************************************************
 *
 * function:
 *   int InferMaxHomDim( node *wlnode, int *sv, int max_hom_dim)
 *
 * description:
 *   Infers the maximal homogenous dimension of the given wlnode-tree 'wlnode'.
 *
 ******************************************************************************/

int
InferMaxHomDim (node *wlnode, long *sv, int max_hom_dim)
{
    node *grid;

    DBUG_ENTER ("InferMaxHomDim");

    if (wlnode != NULL) {

        if (IsHom (wlnode, sv[WLNODE_DIM (wlnode)])) {
            max_hom_dim = InferMaxHomDim (WLNODE_NEXT (wlnode), sv, max_hom_dim);

            if (max_hom_dim > WLNODE_DIM (wlnode)) {
                switch (NODE_TYPE (wlnode)) {
                case N_WLblock:
                    /* here is no break missing */
                case N_WLublock:
                    max_hom_dim
                      = InferMaxHomDim (WLNODE_NEXTDIM (wlnode), sv, max_hom_dim);
                    break;

                case N_WLstride:
                    grid = WLSTRIDE_CONTENTS (wlnode);
                    do {
                        max_hom_dim
                          = InferMaxHomDim (WLGRID_NEXTDIM (grid), sv, max_hom_dim);
                        grid = WLGRID_NEXT (grid);
                    } while ((grid != NULL) && (max_hom_dim > WLSTRIDE_DIM (wlnode)));
                    break;

                case N_WLstriVar:
                    /*
                     * not yet implemented :(
                     */
                    break;

                default:
                    DBUG_ASSERT ((0), "wrong node type found");
                }
            }
        } else {
            max_hom_dim = WLNODE_DIM (wlnode) - 1;
        }
    }

    DBUG_RETURN (max_hom_dim);
}

/******************************************************************************
 *
 * function:
 *   node *InferParams( node *seg)
 *
 * description:
 *   infers WLSEGX_MAXHOMDIM for the given segment 'seg' and WL..._INNERSTEP
 *   for all contained WLblock-, WLublock-, WLstride-nodes with
 *   (WL..._LEVEL == 0).
 *
 ******************************************************************************/

node *
InferParams (node *seg)
{
    DBUG_ENTER ("InferParams");

    WLSEGX_CONTENTS (seg) = InferInnerStep (WLSEGX_CONTENTS (seg), 0, WLSEGX_DIMS (seg));

    WLSEGX_MAXHOMDIM (seg)
      = InferMaxHomDim (WLSEGX_CONTENTS (seg), WLSEGX_SV (seg), WLSEGX_DIMS (seg) - 1);

    DBUG_RETURN (seg);
}

/**
 **
 **   functions for InferParams()
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *   node *WLTRANwith( node *arg_node, node *arg_info)
 *
 * description:
 *   transforms with-loop (N_Nwith-node) into new representation (N_Nwith2).
 *
 * remark:
 *   'INFO_WL_SHPSEG( arg_info)' points to the shape-segs of 'LET_IDS'.
 *
 ******************************************************************************/

node *
WLTRANwith (node *arg_node, node *arg_info)
{
    node *new_node, *strides, *cubes, *segs, *seg;
    int dims, b;
    enum {
        WL_PH_conv,
        WL_PH_cubes,
        WL_PH_segs,
        WL_PH_split,
        WL_PH_block,
        WL_PH_ublock,
        WL_PH_merge,
        WL_PH_opt,
        WL_PH_fit,
        WL_PH_norm
    } WL_break_after;

    DBUG_ENTER ("WLTRANwith");

    /*
     * store the lineno of the current with-loop
     *  (for generation of error-messages)
     */
    line = NODE_LINE (arg_node);

    /* analyse 'break_specifier' */
    WL_break_after = WL_PH_norm;
    if (break_after == PH_wltrans) {
        if (strcmp (break_specifier, "conv") == 0) {
            WL_break_after = WL_PH_conv;
        } else {
            if (strcmp (break_specifier, "cubes") == 0) {
                WL_break_after = WL_PH_cubes;
            } else {
                if (strcmp (break_specifier, "segs") == 0) {
                    WL_break_after = WL_PH_segs;
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

    /*
     * convert parts of with-loop into new format
     */
    DBUG_EXECUTE ("WLprec", NOTE (("step 0.1: converting parts to strides\n")));
    strides = Parts2Strides (NWITH_PART (arg_node), dims);

    /*
     * consistence check: ensures that the strides are pairwise disjoint
     */
    DBUG_EXECUTE ("WLprec", NOTE (("step 0.2: checking disjointness of strides\n")));
#ifndef DBUG_OFF
    if (CheckDisjointness (strides) != 0) {
        DBUG_ASSERT ((0),
                     "Consistence check failed: Not all strides are pairwise disjoint!\n"
                     "This is probably due to an error during with-loop-folding.");
    }
#endif

    if (WL_break_after >= WL_PH_cubes) {
        /*
         * build the cubes
         */
        DBUG_EXECUTE ("WLprec", NOTE (("step 1: cube-building\n")));
        if (NPART_NEXT (NWITH_PART (arg_node)) == NULL) {
            /*
             * we have one part only.
             *  -> the index-range of the generator is possibly a *proper* subset of
             *     the index-vector-space.
             *  -> the generator params are possibly vars.
             */
            cubes = ComputeOneCube (strides, NWITH2_TYPE (new_node), dims,
                                    INFO_WL_SHPSEG (arg_info));
        } else {
            /*
             * we have multiple parts.
             *  -> the index-ranges of the generators partitionize the index-vector-space.
             *  -> the generator params are constant.
             *
             * remark: for the time being these assertions are not a restriction, because
             *         in a SAC-source we can specifiy one part only.
             *         Therefore multiple parts are generated exclusiv by WLF, and these
             *         multiple parts meet the above conditions.
             */
            cubes = ComputeCubes (strides);
        }

        /*
         * full-featured wltransformation
         */

        if (WL_break_after >= WL_PH_segs) {
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
                /* check params of segment */
                CheckParams (seg);

                /* splitting */
                if (WL_break_after >= WL_PH_split) {
                    DBUG_EXECUTE ("WLprec", NOTE (("step 3: splitting\n")));
                    WLSEGX_CONTENTS (seg) = SplitWL (WLSEGX_CONTENTS (seg));
                }

                /* hierarchical blocking */
                if (WL_break_after >= WL_PH_block) {
                    DBUG_EXECUTE ("WLprec", NOTE (("step 4: hierarchical blocking\n")));
                    for (b = 0; b < WLSEGX_BLOCKS (seg); b++) {
                        DBUG_EXECUTE ("WLprec",
                                      NOTE (
                                        ("step 4.%d: hierarchical blocking (level %d)\n",
                                         b + 1, b)));
                        WLSEGX_CONTENTS (seg)
                          = BlockWL (WLSEGX_CONTENTS (seg), dims, WLSEGX_BV (seg, b), 0);
                    }
                }

                /* unrolling-blocking */
                if (WL_break_after >= WL_PH_ublock) {
                    DBUG_EXECUTE ("WLprec", NOTE (("step 5: unrolling-blocking\n")));
                    WLSEGX_CONTENTS (seg)
                      = BlockWL (WLSEGX_CONTENTS (seg), dims, WLSEGX_UBV (seg), 1);
                }

                /* merging */
                if (WL_break_after >= WL_PH_merge) {
                    DBUG_EXECUTE ("WLprec", NOTE (("step 6: merging\n")));
                    WLSEGX_CONTENTS (seg) = MergeWL (WLSEGX_CONTENTS (seg));
                }

                /* optimization */
                if (WL_break_after >= WL_PH_opt) {
                    DBUG_EXECUTE ("WLprec", NOTE (("step 7: optimization\n")));
                    WLSEGX_CONTENTS (seg) = OptWL (WLSEGX_CONTENTS (seg));
                }

                /* fitting */
                if (WL_break_after >= WL_PH_fit) {
                    DBUG_EXECUTE ("WLprec", NOTE (("step 8: fitting\n")));
                    WLSEGX_CONTENTS (seg) = FitWL (WLSEGX_CONTENTS (seg), 0, dims);
                }

                /* normalization */
                if (WL_break_after >= WL_PH_norm) {
                    DBUG_EXECUTE ("WLprec", NOTE (("step 9: normalization\n")));
                    WLSEGX_CONTENTS (seg)
                      = NormWL (WLSEGX_CONTENTS (seg), WLSEGX_IDX_MAX (seg));
                }

#if 0
        /*
         * -> cg ???
         */

        /* infer WLSEGX_MAXHOMDIM and WL..._INNERSTEP */
        seg = InferParams( seg);
#endif

                seg = WLSEG_NEXT (seg);
            }
        } else {
            /*
             * we want to stop after cube-building.
             *  -> build one segment containing all cubes.
             */
            segs = All (NULL, NULL, cubes, dims, line);
        }
    } else {
        /*
         * we want to stop after converting.
         *  -> build one segment containing the strides.
         */
        segs = All (NULL, NULL, strides, dims, line);
    }

    NWITH2_SEGS (new_node) = segs;

    /*
     * old WL-representation is no longer needed
     */
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
        NCODE_CEXPR (code) = Trav (NCODE_CEXPR (code), arg_info);
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
 *   node *WLTRAFundef(node *arg_node, node *arg_info)
 *
 * description:
 *   'INFO_WL_SHPSEG( arg_info)' points to the shape-segs of 'LET_IDS'
 *   (needed for 'WLTRANwith').
 *
 ******************************************************************************/

node *
WLTRALet (node *arg_node, node *arg_info)
{
    shpseg *tmp;

    DBUG_ENTER ("WLTRALet");

    tmp = INFO_WL_SHPSEG (arg_info);

    DBUG_ASSERT ((LET_VARDEC (arg_node) != NULL), "vardec of let-variable not found!");
    DBUG_ASSERT (((NODE_TYPE (LET_VARDEC (arg_node)) == N_vardec)
                  || (NODE_TYPE (LET_VARDEC (arg_node)) == N_arg)),
                 "vardec-node of let-variable has wrong type!");
    INFO_WL_SHPSEG (arg_info) = VARDEC_OR_ARG_SHPSEG (LET_VARDEC (arg_node));

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    INFO_WL_SHPSEG (arg_info) = tmp;

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
