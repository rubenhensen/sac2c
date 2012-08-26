/** <!--********************************************************************-->
 *
 * @defgroup Introduce Availability Loops
 *
 *
 *   This module inserts a loop around CUDA with-loops, to launch the kernel
 *   on a subset of the scheduled area until the whole schedule is complete.
 *   The subset is determined by the local availability of the data, which is
 *   computed from the indexing offsets calculated in IMAL.
 *
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file introduce_availability_loop.c
 *
 * Prefix: IAL
 *
 *****************************************************************************/
#include "introduce_availability_loop.h"

/*
 * Other includes go here
 */
#include <stdlib.h>
#include "tree_basic.h"
#include "tree_compound.h"
#include "memory.h"

#define DBUG_PREFIX "IAL"
#include "debug.h"

#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "new_types.h"
#include "shape.h"
#include "LookUpTable.h"
#include "DupTree.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    bool in_cuda_block;
    node *avail_start;
    node *avail_stop;
    node *preassigns;
    node *genassigns;
    node *prememtran;
    lut_t *memory_transfers;
    node *wl;
    int dim;
    bool bound1;
    node *device_number;
};

/*
 * INFO_FUNDEF          The N_fundef node we are currently traversing.
 *
 * INFO_INCUDABLOCK     Whether we are currently traversing the CUDA branch of
 *                      the SPMD conditional.
 *
 * INFO_AVAILSTART      N_avis for the index of the start of the available
 *                      section
 *
 * INFO_AVAILSTOP       N_avis for the index of the stop of the available
 *                      section
 *
 * INFO_PREASSIGNS      N_assign chain to be prepended to the availability loop
 *
 * INFO_GENASSIGNS      N_assign chain of flattened generators, to be prepended
 *                      to the with-loop.
 *
 * INFO_PREMEMTRAN      N_assign chain to be preppended to the memory transfers
 *                      inside the loop
 *
 * INFO_MEMTRAN         Lookup table storing pairs N_avis->NULL where each
 *                      N_avis is a destination array for a with-loop operation.
 *
 * INFO_WL              N_avis of the with-loop, for the scheduler N_prf.
 *
 * INFO_DIM             The dimension of the generator bounds being flattened.
 *
 * INFO_BOUND1          Whether this is the first generator boundary.
 *
 * INFO_DEVICENUMBER    The avis of the device number variable.
 *
 */

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_INCUDABLOCK(n) (n->in_cuda_block)
#define INFO_AVAILSTART(n) (n->avail_start)
#define INFO_AVAILSTOP(n) (n->avail_stop)
#define INFO_PREASSIGNS(n) (n->preassigns)
#define INFO_GENASSIGNS(n) (n->genassigns)
#define INFO_PREMEMTRAN(n) (n->prememtran)
#define INFO_MEMTRAN(n) (n->memory_transfers)
#define INFO_WL(n) (n->wl)
#define INFO_DIM(n) (n->dim)
#define INFO_BOUND1(n) (n->bound1)
#define INFO_DEVICENUMBER(n) (n->device_number)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_INCUDABLOCK (result) = FALSE;
    INFO_AVAILSTART (result) = NULL;
    INFO_AVAILSTOP (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_GENASSIGNS (result) = NULL;
    INFO_PREMEMTRAN (result) = NULL;
    INFO_MEMTRAN (result) = NULL;
    INFO_WL (result) = NULL;
    INFO_DIM (result) = 0;
    INFO_BOUND1 (result) = FALSE;
    INFO_DEVICENUMBER (result) = NULL;

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
 * @fn node *IALdoIntroduceAvailabilityLoops( node *syntax_tree)
 *
 *****************************************************************************/
node *
IALdoIntroduceAvailabilityLoops (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_ial);
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
 * @name Static helper functions
 * @{
 *
 *****************************************************************************/

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
 * @fn node *IALfundef( node *arg_node, info *arg_info)
 *
 * @brief Skip non-SPMD functions
 *
 *****************************************************************************/
node *
IALfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISSPMDFUN (arg_node)) {
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = NULL;
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IALassign( node *arg_node, info *arg_info)
 *
 * @brief  Introduce availability loop
 *
 *****************************************************************************/
node *
IALassign (node *arg_node, info *arg_info)
{
    node *schedule_start0, *schedule_stop0, *avail_start, *avail_stop, *prf_node;
    node *loop_block, *res, *vardecs, *rhs, *stop_var, *exprs, *cont_stop;
    prf rhs_prf;
    nodetype node_type;

    DBUG_ENTER ();

    if (INFO_INCUDABLOCK (arg_info)) {
        /* we are traversing the CUDA branch of the SPMD. The goal now is to move
         * the transfers for the destination memory OUT of the loop and check the
         * availability of the others. */
        DBUG_ASSERT (NODE_TYPE (ASSIGN_STMT (arg_node)) == N_let,
                     "All the statements in the CUDA branch of a SPMD should be N_let!");

        /* We do a bottom-up traversal, as
         * the with-loop contains the information of the destination memory but it
         * comes after the <dist2device> transfers. */
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

        rhs = ASSIGN_RHS (arg_node);
        node_type = NODE_TYPE (rhs);
        if (node_type == N_prf) {

            rhs_prf = PRF_PRF (rhs);
            if (LUTsearchInLutPp (INFO_MEMTRAN (arg_info),
                                  IDS_AVIS (ASSIGN_LHS (arg_node)))
                  == NULL
                || rhs_prf == F_dist2device_abs) {
                /* if this is a transfer for one of the memory destinations, move it to
                 * the preassignments on info. */
                INFO_PREASSIGNS (arg_info)
                  = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
                res = ASSIGN_NEXT (arg_node);
                ASSIGN_NEXT (arg_node) = NULL;
            } else if (rhs_prf == F_dist2device_rel) {
                /*
                 * create check for availability of this array
                 */

                /* create variable for result of availability check */
                stop_var
                  = TBmakeAvis (TRAVtmpVarName ("stop"),
                                TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
                vardecs = FUNDEF_VARDECS (INFO_FUNDEF (arg_info));
                FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
                  = TBmakeVardec (stop_var, vardecs);

                /* create dist_cont_block prf for this transfer, copy the arguments
                 from the dist2conc */
                exprs = TBmakeExprs (TBmakeId (INFO_AVAILSTART (arg_info)),
                                     TBmakeExprs (TBmakeId (INFO_AVAILSTOP (arg_info)),
                                                  NULL));
                cont_stop
                  = TBmakePrf (F_dist_cont_block,
                               TCappendExprs (DUPdoDupTree (PRF_ARGS (rhs)), exprs));
                INFO_PREMEMTRAN (arg_info)
                  = TBmakeAssign (TBmakeLet (TBmakeIds (stop_var, NULL), cont_stop),
                                  INFO_PREMEMTRAN (arg_info));

                /* change dist2conc to dist2device prf with availability check */
                PRF_PRF (rhs) = F_dist2dev_avail;
                PRF_ARGS (rhs) = TCappendExprs (PRF_ARGS (rhs), DUPdoDupTree (exprs));

                res = arg_node;
            } else {
                DBUG_ASSERT (rhs_prf == F_device2dist,
                             "Invalid prf found in CUDA SPMD branch!");
                /* Insert availability start and stop IDs in the prf's arguments */
                ID_AVIS (PRF_ARG3 (rhs)) = INFO_AVAILSTART (arg_info);
                ID_AVIS (PRF_ARG4 (rhs)) = INFO_AVAILSTOP (arg_info);
                res = arg_node;
            }
        } else {
            /* When we reach the with-loop we traverse it to flatten the generators
             and we move subsequent assignments (F_device2dist) to outside the loop.*/
            DBUG_ASSERT (node_type == N_with, "RHS of N_let in CUDA branch of SPMD must "
                                              "be either N_prf or N_with!");

            INFO_WL (arg_info) = IDS_AVIS (ASSIGN_LHS (arg_node));
            ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

            res = TCappendAssign (INFO_GENASSIGNS (arg_info), arg_node);
            INFO_GENASSIGNS (arg_info) = NULL;
        }

    } else {
        ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

        if (INFO_INCUDABLOCK (arg_info)) {
            /* if by traversing the statement we turned on this flag, we just started
             traversing the CUDA branch of the SPMD. We create the loop here.*/

            /* Initialize Lut*/
            INFO_MEMTRAN (arg_info) = LUTgenerateLut ();

            /* create scheduler and availability indexes */
            schedule_start0
              = TBmakeAvis (TRAVtmpVarName ("schedule_start0"),
                            TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
            schedule_stop0
              = TBmakeAvis (TRAVtmpVarName ("schedule_stop0"),
                            TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
            avail_start
              = TBmakeAvis (TRAVtmpVarName ("availstart"),
                            TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
            INFO_AVAILSTART (arg_info) = avail_start;
            avail_stop
              = TBmakeAvis (TRAVtmpVarName ("availstop"),
                            TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
            INFO_AVAILSTOP (arg_info) = avail_stop;
            stop_var
              = TBmakeAvis (TRAVtmpVarName ("stop"),
                            TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0)));
            /* add vardecs for new vars */
            vardecs = FUNDEF_VARDECS (INFO_FUNDEF (arg_info));
            vardecs = TBmakeVardec (avail_start, TBmakeVardec (avail_stop, vardecs));
            vardecs
              = TBmakeVardec (schedule_start0, TBmakeVardec (schedule_stop0, vardecs));
            FUNDEF_VARDECS (INFO_FUNDEF (arg_info)) = TBmakeVardec (stop_var, vardecs);

            /* create inside of availability loop */
            loop_block
              = TBmakeAssign (TBmakeLet (TBmakeIds (stop_var, NULL),
                                         TCmakePrf2 (F_lt_SxS, TBmakeId (avail_stop),
                                                     TBmakeId (schedule_stop0))),
                              NULL);
            loop_block
              = TCappendAssign (TRAVdo (ASSIGN_NEXT (arg_node), arg_info), loop_block);
            loop_block = TCappendAssign (INFO_PREMEMTRAN (arg_info), loop_block);
            loop_block = TBmakeAssign (TBmakeLet (TBmakeIds (avail_stop, NULL),
                                                  TBmakeId (schedule_stop0)),
                                       loop_block);
            loop_block = TBmakeAssign (TBmakeLet (TBmakeIds (avail_start, NULL),
                                                  TBmakeId (avail_stop)),
                                       loop_block);
            rhs = TCmakePrf1 (F_cuda_get_stream, TBmakeId (INFO_DEVICENUMBER (arg_info)));
            loop_block = TBmakeAssign (TBmakeLet (NULL, rhs), loop_block);
            loop_block = TBmakeBlock (loop_block, NULL);
            BLOCK_ISMTPARALLELBRANCH (loop_block) = TRUE;

            /* create loop and surrounding assignments */
            prf_node
              = TCmakePrf1 (F_cuda_device_sync, TBmakeId (INFO_DEVICENUMBER (arg_info)));
            res = TBmakeAssign (TBmakeLet (NULL, prf_node), NULL);

            res = TBmakeAssign (TBmakeDo (TBmakeId (stop_var), loop_block), res);
            res = TBmakeAssign (TBmakeLet (TBmakeIds (avail_stop, NULL),
                                           TBmakeId (schedule_start0)),
                                res);
            res = TBmakeAssign (TBmakeLet (TBmakeIds (avail_start, NULL),
                                           TBmakeId (schedule_start0)),
                                res);
            res = TCappendAssign (INFO_PREASSIGNS (arg_info), res);
            res = TBmakeAssign (TBmakeLet (TBmakeIds (schedule_stop0, NULL),
                                           TCmakePrf2 (F_sched_stop,
                                                       TBmakeId (INFO_WL (arg_info)),
                                                       TBmakeNum (0))),
                                res);
            res = TBmakeAssign (TBmakeLet (TBmakeIds (schedule_start0, NULL),
                                           TCmakePrf2 (F_sched_start,
                                                       TBmakeId (INFO_WL (arg_info)),
                                                       TBmakeNum (0))),
                                res);

            /* cleanup */
            ASSIGN_NEXT (arg_node) = res;
            res = arg_node;
            INFO_INCUDABLOCK (arg_info) = FALSE;
            INFO_MEMTRAN (arg_info) = LUTremoveLut (INFO_MEMTRAN (arg_info));
            INFO_WL (arg_info) = NULL;
            INFO_PREASSIGNS (arg_info) = NULL;
            INFO_PREMEMTRAN (arg_info) = NULL;
        } else {
            ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
            res = arg_node;
        }
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *IALprf( node *arg_node, info *arg_info)
 *
 * @brief Save avis of the cuda device variable.
 *
 *****************************************************************************/
node *
IALprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (PRF_PRF (arg_node) == F_cuda_set_device) {
        INFO_DEVICENUMBER (arg_info) = ID_AVIS (PRF_ARG1 (arg_node));
        INFO_INCUDABLOCK (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IALwith( node *arg_node, info *arg_info)
 *
 * @brief Traverse operations to save destination arrays in table and traverse
 *        partitions to flatten generators.
 *
 *****************************************************************************/
node *
IALwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IALpart( node *arg_node, info *arg_info)
 *
 * @brief Traverse generators and next partition only.
 *
 *****************************************************************************/
node *
IALpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);
    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IALgenerator( node *arg_node, info *arg_info)
 *
 * @brief Flatten generators.
 *
 *****************************************************************************/
node *
IALgenerator (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_DIM (arg_info) = 0;
    INFO_BOUND1 (arg_info) = TRUE;
    GENERATOR_BOUND1 (arg_node) = TRAVdo (GENERATOR_BOUND1 (arg_node), arg_info);
    INFO_DIM (arg_info) = 0;
    INFO_BOUND1 (arg_info) = FALSE;
    GENERATOR_BOUND2 (arg_node) = TRAVdo (GENERATOR_BOUND2 (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IALexprs( node *arg_node, info *arg_info)
 *
 * @brief Flatten generators.
 *
 *****************************************************************************/
node *
IALexprs (node *arg_node, info *arg_info)
{
    node *sched_avis, *vardecs, *genassigns, *bound_avis, *assign;
    prf sched_prf, intersect;

    DBUG_ENTER ();

    /*
     * if we are inside a CUDA SPMD branch, we are traversing the generator bounds
     */
    if (INFO_INCUDABLOCK (arg_info)) {
        vardecs = FUNDEF_VARDECS (INFO_FUNDEF (arg_info));
        genassigns = INFO_GENASSIGNS (arg_info);

        /* for the first dimension, we get the bounds from the availability
         * variables. For the others, we query the scheduler directly.
         */
        if (INFO_DIM (arg_info) == 0) {
            sched_avis = (INFO_BOUND1 (arg_info) ? INFO_AVAILSTART (arg_info)
                                                 : INFO_AVAILSTOP (arg_info));
        } else {
            sched_avis
              = TBmakeAvis (TRAVtmpVarName ("schedule"),
                            TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
            vardecs = TBmakeVardec (sched_avis, vardecs);

            sched_prf = (INFO_BOUND1 (arg_info) ? F_sched_start : F_sched_stop);

            assign
              = TBmakeAssign (TBmakeLet (TBmakeIds (sched_avis, NULL),
                                         TCmakePrf2 (sched_prf,
                                                     TBmakeId (INFO_WL (arg_info)),
                                                     TBmakeNum (INFO_DIM (arg_info)))),
                              NULL);
            genassigns = TCappendAssign (genassigns, assign);
        }

        /* create new bound variable and predicate */
        bound_avis = TBmakeAvis (TRAVtmpVarName ("bound"),
                                 TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
        FUNDEF_VARDECS (INFO_FUNDEF (arg_info)) = TBmakeVardec (bound_avis, vardecs);

        /* new bound is the maximum or minimum between the current bound and the
         * scheduled/available bounds. These assignments perform the calculations.*/
        intersect = (INFO_BOUND1 (arg_info) ? F_max_SxS : F_min_SxS);
        assign = TBmakeAssign (TBmakeLet (TBmakeIds (bound_avis, NULL),
                                          TCmakePrf2 (intersect, EXPRS_EXPR (arg_node),
                                                      TBmakeId (sched_avis))),
                               NULL);
        INFO_GENASSIGNS (arg_info) = TCappendAssign (genassigns, assign);
        EXPRS_EXPR (arg_node) = TBmakeId (bound_avis);

        INFO_DIM (arg_info)++;
        EXPRS_NEXT (arg_node) = TRAVopt (EXPRS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IALgenarray( node *arg_node, info *arg_info)
 *
 * @brief Save memory array into LUT and traverse next op.
 *
 *****************************************************************************/
node *
IALgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_MEMTRAN (arg_info) = LUTinsertIntoLutP (INFO_MEMTRAN (arg_info),
                                                 ID_AVIS (GENARRAY_MEM (arg_node)), NULL);
    GENARRAY_NEXT (arg_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IALmodarray( node *arg_node, info *arg_info)
 *
 * @brief Save memory array into LUT and traverse next op.
 *
 *****************************************************************************/
node *
IALmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_MEMTRAN (arg_info) = LUTinsertIntoLutP (INFO_MEMTRAN (arg_info),
                                                 ID_AVIS (MODARRAY_MEM (arg_node)), NULL);
    MODARRAY_NEXT (arg_node) = TRAVopt (MODARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/

#undef DBUG_PREFIX
