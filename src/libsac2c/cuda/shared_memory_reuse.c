
#include "shared_memory_reuse.h"

#include <stdlib.h>

#include "tree_basic.h"
#include "tree_compound.h"
#include "globals.h"
#include "memory.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "print.h"
#include "new_types.h"
#include "shape.h"
#include "types.h"
#include "constants.h"
#include "pattern_match.h"
#include "pattern_match_attribs.h"
#include "cuda_utils.h"
#include "LookUpTable.h"

typedef enum { trav_normal, trav_update } travmode_t;

/*
 * INFO structure
 */
struct INFO {
    int level;
    node *withids;
    node *fundef;
    travmode_t travmode;
    rc_t *rc;
    lut_t *lut;
    node **array_p;
};

#define INFO_LEVEL(n) (n->level)
#define INFO_WITHIDS(n) (n->withids)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_TRAVMODE(n) (n->travmode)
#define INFO_RC(n) (n->rc)
#define INFO_LUT(n) (n->lut)
#define INFO_ARRAY_P(n) (n->array_p)

/*
 * INFO macros
 */

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_LEVEL (result) = 0;
    INFO_WITHIDS (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_TRAVMODE (result) = trav_normal;
    INFO_RC (result) = NULL;
    INFO_LUT (result) = NULL;
    INFO_ARRAY_P (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

static rc_t *
FreeAllRcs (rc_t *rcs)
{
    DBUG_ENTER ();

    DBUG_ASSERT (rcs != NULL, "RC to be freed is NULL!");

    if (RC_NEXT (rcs) != NULL) {
        RC_NEXT (rcs) = FreeAllRcs (RC_NEXT (rcs));
    }

    if (RC_ARRAYSHP (rcs) != NULL) {
        RC_ARRAYSHP (rcs) = MEMfree (RC_ARRAYSHP (rcs));
    }

    if (RC_SHARRAYSHP (rcs) != NULL) {
        RC_SHARRAYSHP (rcs) = MEMfree (RC_SHARRAYSHP (rcs));
    }

    rcs = MEMfree (rcs);

    DBUG_RETURN ((rc_t *)NULL);
}

static node *
CreateShmemBoundaryLoadPrf (node *cond, node *shmem, rc_t *rc, size_t dim, node *shmem_ids,
                            node *withids, node *offsets, node **vardecs_p,
                            node **assigns_p)
{
    node *A_shmem, *A_shmem_old, *new_assign;

    DBUG_ENTER ();

    A_shmem_old = shmem;
    A_shmem = DUPdoDupNode (shmem);
    AVIS_NAME (A_shmem) = MEMfree (AVIS_NAME (A_shmem));
    AVIS_NAME (A_shmem) = TRAVtmpVarName ("shmem");
    *vardecs_p = TBmakeVardec (A_shmem, *vardecs_p);

    new_assign = TBmakeAssign (
      TBmakeLet (
        TBmakeIds (A_shmem, NULL),
        TBmakePrf (
          F_shmem_boundary_load,
          TBmakeExprs (
            TBmakeId (cond),
            TBmakeExprs (
              TBmakeId (A_shmem_old),
              TBmakeExprs (
                DUPdoDupNode (RC_SHARRAYSHP (rc)),
                TBmakeExprs (TBmakeId (RC_ARRAY (rc)),
                             TBmakeExprs (DUPdoDupNode (RC_ARRAYSHP (rc)),
                                          TBmakeExprs (TBmakeNum (dim),
                                                       TCappendExprs (
                                                         shmem_ids,
                                                         TCappendExprs (
                                                           TCids2Exprs (withids),
                                                           TCappendExprs (offsets,
                                                                          NULL))))))))))),
      NULL);

    AVIS_SSAASSIGN (A_shmem) = new_assign;
    *assigns_p = TCappendAssign (*assigns_p, new_assign);

    DBUG_RETURN (A_shmem);
}

static node *
CreatePrf (char *name, simpletype sty, shape *shp, prf pfun, node *args, node **vardecs_p,
           node **assigns_p)
{
    node *avis = NULL, *new_assign;

    DBUG_ENTER ();

    if (name != NULL) {
        avis
          = TBmakeAvis (TRAVtmpVarName (name), TYmakeAKS (TYmakeSimpleType (sty), shp));

        *vardecs_p = TBmakeVardec (avis, *vardecs_p);
    }

    new_assign = TBmakeAssign (TBmakeLet ((avis == NULL) ? avis : TBmakeIds (avis, NULL),
                                          TBmakePrf (pfun, args)),
                               NULL);

    if (avis != NULL) {
        AVIS_SSAASSIGN (avis) = new_assign;
    }

    if (*assigns_p == NULL) {
        *assigns_p = new_assign;
    } else {
        *assigns_p = TCappendAssign (*assigns_p, new_assign);
    }

    DBUG_RETURN (avis);
}

static node *
CreateSharedMemoryAccessCode (rc_t *rc, info *arg_info)
{
    size_t dim;
    node *assigns = NULL, *vardecs = NULL;
    node *tx, *ty, *tx_shmem, *ty_shmem, *b_dim_x, *b_dim_y, *b_dim_x_m1, *b_dim_y_m1,
      *idx_shmem, *idx_dev, *elem, *A_shmem, *withids;
    node *args;
    node *cond_0, *cond_1, *cond;
    node *shmem_ids, *offsets;
    simpletype sty;

    DBUG_ENTER ();

    dim = RC_DIM (rc);
    withids = INFO_WITHIDS (arg_info);

    if (dim == 1) {
    } else if (dim == 2) {
        /*
         *  ty = cuda_threadIdx( 2, 0);
         *  tx = cuda_threadIdx( 2, 1);
         *  b_dim_y = cuda_blockDim( 2, 0);
         *  b_dim_x = cuda_blockDim( 2, 1);
         */
        ty = CreatePrf ("ty", T_int, SHmakeShape (0), F_cuda_threadIdx_y, NULL, &vardecs,
                        &assigns);

        tx = CreatePrf ("tx", T_int, SHmakeShape (0), F_cuda_threadIdx_x, NULL, &vardecs,
                        &assigns);

        b_dim_y = CreatePrf ("b_dim_y", T_int, SHmakeShape (0), F_cuda_blockDim_y, NULL,
                             &vardecs, &assigns);

        b_dim_x = CreatePrf ("b_dim_x", T_int, SHmakeShape (0), F_cuda_blockDim_x, NULL,
                             &vardecs, &assigns);

        /*
         *  ty_shmem = add_SxS( ty, offset);
         *  tx_shmem = add_SxS( tx, offset);
         */
        args = TBmakeExprs (TBmakeId (ty),
                            TBmakeExprs (TBmakeNum (RC_NEGOFFSET (rc, 0)), NULL));
        ty_shmem = CreatePrf ("ty_shmem", T_int, SHmakeShape (0), F_add_SxS, args,
                              &vardecs, &assigns);

        args = TBmakeExprs (TBmakeId (tx),
                            TBmakeExprs (TBmakeNum (RC_NEGOFFSET (rc, 1)), NULL));
        tx_shmem = CreatePrf ("tx_shmem", T_int, SHmakeShape (0), F_add_SxS, args,
                              &vardecs, &assigns);

        shmem_ids
          = TBmakeExprs (TBmakeId (ty_shmem), TBmakeExprs (TBmakeId (tx_shmem), NULL));

        INFO_LUT (arg_info)
          = LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (withids), ty_shmem);
        INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info),
                                                 IDS_AVIS (IDS_NEXT (withids)), tx_shmem);

        /*
         *  idx_shmem = idxs2offset( [18, 18], ty_shmem, tx_shmem);
         *  idx_dev = idxs2offset( [1024, 1024], eat1, eat2);
         */
        args = TBmakeExprs (DUPdoDupNode (RC_SHARRAYSHP (rc)), DUPdoDupTree (shmem_ids));
        idx_shmem = CreatePrf ("shmemidx", T_int, SHmakeShape (0), F_idxs2offset, args,
                               &vardecs, &assigns);

        args = TBmakeExprs (DUPdoDupNode (RC_ARRAYSHP (rc)), TCids2Exprs (withids));
        idx_dev = CreatePrf ("devidx", T_int, SHmakeShape (0), F_idxs2offset, args,
                             &vardecs, &assigns);

        /*
         *  elem = idx_sel( idx_dev, A);
         */
        args = TBmakeExprs (TBmakeId (idx_dev),
                            TBmakeExprs (TBmakeId (RC_ARRAY (rc)), NULL));
        sty = CUd2hSimpleTypeConversion (
          TYgetSimpleType (TYgetScalar (AVIS_TYPE (RC_ARRAY (rc)))));
        elem
          = CreatePrf ("elem", sty, SHmakeShape (0), F_idx_sel, args, &vardecs, &assigns);

        /*
         *  A_shmem = idx_modarray_AxSxS( A_shmem, idx_shmem, elem);
         */
        args = TBmakeExprs (TBmakeId (RC_SHARRAY (rc)),
                            TBmakeExprs (TBmakeId (idx_shmem),
                                         TBmakeExprs (TBmakeId (elem), NULL)));
        A_shmem = CreatePrf ("shmem",
                             TYgetSimpleType (TYgetScalar (AVIS_TYPE (RC_SHARRAY (rc)))),
                             SHarray2Shape (RC_SHARRAYSHP (rc)), F_idx_modarray_AxSxS,
                             args, &vardecs, &assigns);

        /*
         *  cond = eq_SxS( ty, 0);
         *  A_shmem = shmem_boundary_load( cond, A_shmem, [18, 18], A, [1024, 1024],
         *                                 2, ty_shmem, tx_shmem, eat1, eat2, off1, off2);
         */
        args = TBmakeExprs (TBmakeId (ty), TBmakeExprs (TBmakeNum (0), NULL));
        cond_0 = CreatePrf ("cond", T_bool, SHmakeShape (0), F_eq_SxS, args, &vardecs,
                            &assigns);
        offsets = TBmakeExprs (TBmakeNum (-RC_NEGOFFSET (rc, 0)),
                               TBmakeExprs (TBmakeNum (0), NULL));
        A_shmem = CreateShmemBoundaryLoadPrf (cond_0, A_shmem, rc, dim,
                                              DUPdoDupTree (shmem_ids), withids, offsets,
                                              &vardecs, &assigns);

        /*
         *  cond = eq_SxS( tx, 0);
         *  A_shmem = shmem_boundary_load( cond, A_shmem, [18, 18], A, [1024, 1024],
         *                                 2, ty_shmem, tx_shmem, eat1, eat2, off1, off2);
         */
        args = TBmakeExprs (TBmakeId (tx), TBmakeExprs (TBmakeNum (0), NULL));
        cond_0 = CreatePrf ("cond", T_bool, SHmakeShape (0), F_eq_SxS, args, &vardecs,
                            &assigns);
        offsets = TBmakeExprs (TBmakeNum (0),
                               TBmakeExprs (TBmakeNum (-RC_NEGOFFSET (rc, 1)), NULL));
        A_shmem = CreateShmemBoundaryLoadPrf (cond_0, A_shmem, rc, dim,
                                              DUPdoDupTree (shmem_ids), withids, offsets,
                                              &vardecs, &assigns);

        /*
         *  b_dim_y_m1 = sub_SxS( b_dim_y, 1);
         *  cond_0 = eq_SxS( ty, b_dim_y_m1);
         *  cond_1 = shmem_boundary_check( 0, eat0, 1);
         *  cond = or_SxS( cond_0, cond_1);
         *  A_shmem = shmem_boundary_load( cond, A_shmem, [18, 18], A, [1024, 1024],
         *                                 2, ty_shmem, tx_shmem, eat1, eat2, off1, off2);
         */
        args = TBmakeExprs (TBmakeId (b_dim_y), TBmakeExprs (TBmakeNum (1), NULL));
        b_dim_y_m1 = CreatePrf ("b_dim_y_m1", T_int, SHmakeShape (0), F_sub_SxS, args,
                                &vardecs, &assigns);
        args = TBmakeExprs (TBmakeId (ty), TBmakeExprs (TBmakeId (b_dim_y_m1), NULL));
        cond_0 = CreatePrf ("cond", T_bool, SHmakeShape (0), F_eq_SxS, args, &vardecs,
                            &assigns);

        args
          = TBmakeExprs (TBmakeNum (0), TBmakeExprs (TBmakeId (IDS_AVIS (withids)),
                                                     TBmakeExprs (TBmakeNum (1), NULL)));

        cond_1 = CreatePrf ("cond", T_bool, SHmakeShape (0), F_shmem_boundary_check, args,
                            &vardecs, &assigns);

        args = TBmakeExprs (TBmakeId (cond_0), TBmakeExprs (TBmakeId (cond_1), NULL));

        cond = CreatePrf ("cond", T_bool, SHmakeShape (0), F_or_SxS, args, &vardecs,
                          &assigns);

        offsets = TBmakeExprs (TBmakeNum (RC_POSOFFSET (rc, 0)),
                               TBmakeExprs (TBmakeNum (0), NULL));
        A_shmem
          = CreateShmemBoundaryLoadPrf (cond, A_shmem, rc, dim, DUPdoDupTree (shmem_ids),
                                        withids, offsets, &vardecs, &assigns);

        /*
         *  b_dim_x_m1 = sub_SxS( b_dim_x, 1);
         *  cond_0 = eq_SxS( tx, b_dim_x_m1);
         *  cond_1 = shmem_boundary_check( 1, eat1, 1);
         *  cond = or_SxS( cond_0, cond_1);
         *  A_shmem = shmem_boundary_load( cond, A_shmem, [18, 18], A, [1024, 1024],
         *                                 2, ty_shmem, tx_shmem, eat1, eat2, off1, off2);
         */
        args = TBmakeExprs (TBmakeId (b_dim_x), TBmakeExprs (TBmakeNum (1), NULL));
        b_dim_x_m1 = CreatePrf ("b_dim_x_m1", T_int, SHmakeShape (0), F_sub_SxS, args,
                                &vardecs, &assigns);

        args = TBmakeExprs (TBmakeId (tx), TBmakeExprs (TBmakeId (b_dim_x_m1), NULL));
        cond_0 = CreatePrf ("cond", T_bool, SHmakeShape (0), F_eq_SxS, args, &vardecs,
                            &assigns);

        args = TBmakeExprs (TBmakeNum (1),
                            TBmakeExprs (TBmakeId (IDS_AVIS (IDS_NEXT (withids))),
                                         TBmakeExprs (TBmakeNum (1), NULL)));
        cond_1 = CreatePrf ("cond", T_bool, SHmakeShape (0), F_shmem_boundary_check, args,
                            &vardecs, &assigns);

        args = TBmakeExprs (TBmakeId (cond_0), TBmakeExprs (TBmakeId (cond_1), NULL));

        cond = CreatePrf ("cond", T_bool, SHmakeShape (0), F_or_SxS, args, &vardecs,
                          &assigns);

        offsets = TBmakeExprs (TBmakeNum (0),
                               TBmakeExprs (TBmakeNum (RC_POSOFFSET (rc, 1)), NULL));
        A_shmem
          = CreateShmemBoundaryLoadPrf (cond, A_shmem, rc, dim, DUPdoDupTree (shmem_ids),
                                        withids, offsets, &vardecs, &assigns);

        RC_SHARRAY (rc) = A_shmem;

        // shmem_ids = FREEdoFreeTree( shmem_ids);
        FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
          = TCappendVardec (FUNDEF_VARDECS (INFO_FUNDEF (arg_info)), vardecs);
    } else {
    }

    DBUG_RETURN (assigns);
}

/** <!--********************************************************************-->
 *
 * @fn node *SHMEMdoSharedMemoryReuse( node *syntax_tree)
 *
 * @brief
 *
 *****************************************************************************/
node *
SHMEMdoSharedMemoryReuse (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();
    TRAVpush (TR_shmem);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *SHMEMfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
SHMEMfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SHMEMassign( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
SHMEMassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_TRAVMODE (arg_info) == trav_normal) {
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SHMEMwith( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
SHMEMwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (WITH_CUDARIZABLE (arg_node)) {
        INFO_LEVEL (arg_info)++;
        WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
        INFO_LEVEL (arg_info)--;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SHMEMpart( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
SHMEMpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_WITHIDS (arg_info) = PART_IDS (arg_node);
    PART_CODE (arg_node) = TRAVopt (PART_CODE (arg_node), arg_info);

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SHMEMcode( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
SHMEMcode (node *arg_node, info *arg_info)
{
    rc_t *rcs;
    node *assigns = NULL, *all_new_assigns = NULL;
    node *sync_ids = NULL, *sync_exprs = NULL, *avis;
    node *sync_assign;

    DBUG_ENTER ();

    if (CODE_IRA_INFO (arg_node) != NULL) {
        if (CODE_IRA_RCCOUNT (arg_node) > 0) {
            rcs = CODE_IRA_RCS (arg_node);
            while (rcs != NULL) {
                if (RC_REUSABLE (rcs)) {
                    INFO_LUT (arg_info) = LUTgenerateLut ();
                    assigns = CreateSharedMemoryAccessCode (rcs, arg_info);
                    DBUG_ASSERT (assigns != NULL,
                                 "Found null assign chain for reuse candidate!");

                    sync_exprs = TBmakeExprs (TBmakeId (RC_SHARRAY (rcs)), NULL);
                    avis = TBmakeAvis (TRAVtmpVarName ("shmem"),
                                       TYcopyType (AVIS_TYPE (RC_SHARRAY (rcs))));
                    FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
                      = TBmakeVardec (avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));
                    RC_SHARRAY (rcs) = avis;
                    sync_ids = TBmakeIds (avis, NULL);

                    sync_assign
                      = TBmakeAssign (TBmakeLet (sync_ids,
                                                 TBmakePrf (F_syncthreads, sync_exprs)),
                                      NULL);

                    AVIS_SSAASSIGN (IDS_AVIS (sync_ids)) = sync_assign;

                    all_new_assigns
                      = TCappendAssign (TCappendAssign (assigns, sync_assign),
                                        all_new_assigns);

                    /* Traverse the code with the current reuse candidate */
                    INFO_RC (arg_info) = rcs;
                    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);
                    INFO_RC (arg_info) = NULL;

                    INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));
                }
                rcs = RC_NEXT (rcs);
            }

            CODE_CBLOCK_ASSIGNS (arg_node)
              = TCappendAssign (all_new_assigns, CODE_CBLOCK_ASSIGNS (arg_node));
        }

        CODE_IRA_RCS (arg_node) = FreeAllRcs (CODE_IRA_RCS (arg_node));
        CODE_IRA_INFO (arg_node) = MEMfree (CODE_IRA_INFO (arg_node));
        CODE_IRA_INFO (arg_node) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SHMEMprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
SHMEMprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* If we are in cuda withloop */
    if (INFO_LEVEL (arg_info) > 0) {
        if (PRF_PRF (arg_node) == F_idx_sel) {
            node *idx = PRF_ARG1 (arg_node);
            node *arr = PRF_ARG2 (arg_node);

            DBUG_ASSERT (NODE_TYPE (idx) == N_id,
                         "Non-id node found in the first argument of idx_sel!");
            DBUG_ASSERT (NODE_TYPE (arr) == N_id,
                         "Non-id node found in the second argument of idx_sel!");

            if (ID_AVIS (arr) == RC_ARRAY (INFO_RC (arg_info))) {
                INFO_TRAVMODE (arg_info) = trav_update;
                INFO_ARRAY_P (arg_info) = &arr;
                ID_SSAASSIGN (idx) = TRAVopt (ID_SSAASSIGN (idx), arg_info);
                INFO_ARRAY_P (arg_info) = NULL;
                INFO_TRAVMODE (arg_info) = trav_normal;
            }
        } else if (PRF_PRF (arg_node) == F_idxs2offset
                   && INFO_TRAVMODE (arg_info) == trav_update) {
            pattern *pat1, *pat2, *pat3;
            node *id, *ids, *withids, *avis;
            rc_t *rc;
            int off;
            bool reusable = TRUE;
            nodelist *nl = NULL;

            rc = INFO_RC (arg_info);
            DBUG_ASSERT (rc != NULL, "Null reuse candidate found!");

            pat1 = PMprf (1, PMAisPrf (F_sub_SxS), 2, PMvar (1, PMAgetNode (&id), 0),
                          PMint (1, PMAgetIVal (&off), 0));
            pat2 = PMprf (1, PMAisPrf (F_add_SxS), 2, PMvar (1, PMAgetNode (&id), 0),
                          PMint (1, PMAgetIVal (&off), 0));
            pat3 = PMprf (1, PMAisPrf (F_add_SxS), 2, PMvar (1, PMAgetIVal (&off), 0),
                          PMint (1, PMAgetNode (&id), 0));

            ids = PRF_EXPRS2 (arg_node);
            withids = INFO_WITHIDS (arg_info);

            /* We currenly only look at the case when the dim of
             * the array being inferred is same as the dim of the
             * surrounding withloop. */
            if (TCcountExprs (ids) == TCcountIds (withids)) {
                while (ids != NULL && withids != NULL) {
                    if (PMmatchFlat (pat1, EXPRS_EXPR (ids))
                        && ID_AVIS (id) == IDS_AVIS (withids)) {
                        nl = TCnodeListAppend (nl, id, NULL);
                    } else if ((PMmatchFlat (pat2, EXPRS_EXPR (ids))
                                || PMmatchFlat (pat3, EXPRS_EXPR (ids)))
                               && ID_AVIS (id) == IDS_AVIS (withids)) {
                        nl = TCnodeListAppend (nl, id, NULL);
                    } else if (ID_AVIS (EXPRS_EXPR (ids)) == IDS_AVIS (withids)) {
                        nl = TCnodeListAppend (nl, EXPRS_EXPR (ids), NULL);
                    } else {
                        reusable = FALSE;
                        break;
                    }
                    ids = EXPRS_NEXT (ids);
                    withids = IDS_NEXT (withids);
                }

                if (reusable) {
                    while (nl != NULL) {
                        id = NODELIST_NODE (nl);
                        DBUG_ASSERT (NODE_TYPE (id) == N_id,
                                     "Non N_id node found in nodelist!");

                        avis
                          = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), ID_AVIS (id));
                        if (avis != ID_AVIS (id)) {
                            ID_AVIS (id) = avis;
                        }
                        nl = NODELIST_NEXT (nl);
                    }
                    EXPRS_EXPR (PRF_EXPRS1 (arg_node))
                      = FREEdoFreeNode (EXPRS_EXPR (PRF_EXPRS1 (arg_node)));
                    EXPRS_EXPR (PRF_EXPRS1 (arg_node))
                      = DUPdoDupNode (RC_SHARRAYSHP (rc));
                    ID_AVIS (*(INFO_ARRAY_P (arg_info))) = RC_SHARRAY (rc);
                }
            }

            pat1 = PMfree (pat1);
            pat2 = PMfree (pat2);
            pat3 = PMfree (pat3);
        } else {
        }
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
