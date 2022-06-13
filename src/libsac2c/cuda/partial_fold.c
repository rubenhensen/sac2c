/*****************************************************************************
 *
 * @defgroup
 *
 *
 *
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file partial_fold.c
 *
 * Prefix: PFD
 *
 *****************************************************************************/
#include "partial_fold.h"

#include <stdlib.h>
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "globals.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "namespaces.h"
#include "new_types.h"
#include "DupTree.h"
#include "type_utils.h"
#include "shape.h"
#include "constants.h"
#include "cuda_utils.h"
#include "create_loop_fun.h"
#include "create_cond_fun.h"

typedef struct CUIDX_SET_T {
    node *tx;
    node *ty;
    node *bx;
    node *by;
} cuidx_set_t;

#define CIS_TX(n) (n->tx)
#define CIS_TY(n) (n->ty)
#define CIS_BX(n) (n->bx)
#define CIS_BY(n) (n->by)

typedef struct CUDIM_SET_T {
    node *bdim_x;
    node *bdim_y;
    node *gdim_x;
    node *gdim_y;
} cudim_set_t;

#define CDS_BDIM_X(n) (n->bdim_x)
#define CDS_BDIM_Y(n) (n->bdim_y)
#define CDS_GDIM_X(n) (n->gdim_x)
#define CDS_GDIM_Y(n) (n->gdim_y)

typedef enum {
    y_reduction,
    x_reduction,
} reduction_kind;

typedef enum { def_scalar, def_withloop, def_array } res_def;

static int CHANGED = 1;

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *lhs;
    int level;
    node *partshp;
    node *partialshp;
    node *withids;
    size_t dim;
    size_t innerdim;
    node *fundef;
    node *lastassign;
    node *partialarr;
    node *postassign;
    node *foldfundef;
    node *neutral;
    prf foldop;
    node *cexpr;
    cuidx_set_t *cis;
    cudim_set_t *cds;
    node *shrarray;
    node *lacfuns;
    node *arrayelems;
    res_def resdef;

    /* Field for anonmous travesal */
    node *at_ids;
    node *at_vec;
    node *at_bound1;
    node *at_bound2;
    node *at_genwidth;
    node *at_code;
    node *at_assign;
    node *at_vecassigns;
    size_t at_innerdim;
    node *at_arrayelems;
    res_def at_resdef;
};

#define INFO_LHS(n) (n->lhs)
#define INFO_LEVEL(n) (n->level)
#define INFO_PARTSHP(n) (n->partshp)
#define INFO_PARTIALSHP(n) (n->partialshp)
#define INFO_WITHIDS(n) (n->withids)
#define INFO_DIM(n) (n->dim)
#define INFO_INNERDIM(n) (n->innerdim)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LASTASSIGN(n) (n->lastassign)
#define INFO_PARTIALARR(n) (n->partialarr)
#define INFO_POSTASSIGN(n) (n->postassign)
#define INFO_FOLDFUNDEF(n) (n->foldfundef)
#define INFO_NEUTRAL(n) (n->neutral)
#define INFO_FOLDOP(n) (n->foldop)
#define INFO_CEXPR(n) (n->cexpr)
#define INFO_CIS(n) (n->cis)
#define INFO_CDS(n) (n->cds)
#define INFO_SHRARRAY(n) (n->shrarray)
#define INFO_LACFUNS(n) (n->lacfuns)
#define INFO_ARRAYELEMS(n) (n->arrayelems)
#define INFO_RESDEF(n) (n->resdef)

#define INFO_AT_INNERWITHIDS(n) (n->at_ids)
#define INFO_AT_INNERWITHVEC(n) (n->at_vec)
#define INFO_AT_INNERWITHBOUND1(n) (n->at_bound1)
#define INFO_AT_INNERWITHBOUND2(n) (n->at_bound2)
#define INFO_AT_INNERWITHGENWIDTH(n) (n->at_genwidth)
#define INFO_AT_INNERWITHCODE(n) (n->at_code)
#define INFO_AT_INNERWITHASSIGN(n) (n->at_assign)
#define INFO_AT_VECASSIGNS(n) (n->at_vecassigns)
#define INFO_AT_INNERDIM(n) (n->at_innerdim)

#define INFO_AT_ARRAYELEMS(n) (n->at_arrayelems)
#define INFO_AT_RESDEF(n) (n->at_resdef)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_LHS (result) = NULL;
    INFO_LEVEL (result) = 0;
    INFO_PARTSHP (result) = NULL;
    INFO_PARTIALSHP (result) = NULL;
    INFO_WITHIDS (result) = NULL;
    INFO_DIM (result) = 0;
    INFO_INNERDIM (result) = 0;
    INFO_FUNDEF (result) = NULL;
    INFO_LASTASSIGN (result) = NULL;
    INFO_PARTIALARR (result) = NULL;
    INFO_POSTASSIGN (result) = NULL;
    INFO_FOLDFUNDEF (result) = NULL;
    INFO_NEUTRAL (result) = NULL;
    INFO_FOLDOP (result) = F_unknown;
    INFO_CEXPR (result) = NULL;
    INFO_CIS (result) = NULL;
    INFO_CDS (result) = NULL;
    INFO_SHRARRAY (result) = NULL;
    INFO_LACFUNS (result) = NULL;
    INFO_ARRAYELEMS (result) = NULL;
    INFO_RESDEF (result) = def_scalar;

    INFO_AT_INNERWITHIDS (result) = NULL;
    INFO_AT_INNERWITHVEC (result) = NULL;
    INFO_AT_INNERWITHBOUND1 (result) = NULL;
    INFO_AT_INNERWITHBOUND2 (result) = NULL;
    INFO_AT_INNERWITHGENWIDTH (result) = NULL;
    INFO_AT_INNERWITHCODE (result) = NULL;
    INFO_AT_INNERWITHASSIGN (result) = NULL;
    INFO_AT_VECASSIGNS (result) = NULL;
    INFO_AT_INNERDIM (result) = 0;

    INFO_AT_ARRAYELEMS (result) = NULL;
    INFO_AT_RESDEF (result) = def_scalar;

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

static node *
AppendVardec (node *fundef, node *avis)
{
    DBUG_ENTER ();

    FUNDEF_VARDECS (fundef)
      = TCappendVardec (FUNDEF_VARDECS (fundef), TBmakeVardec (avis, NULL));

    DBUG_RETURN (fundef);
}

static node *
CreatePrfOrConst (bool isprf, char *name, simpletype sty, shape *shp, prf pfun,
                  node *args, node **assigns_p, node *fundef)
{
    node *avis = NULL, *new_assign;

    DBUG_ENTER ();

    if (name != NULL) {
        avis
          = TBmakeAvis (TRAVtmpVarName (name), TYmakeAKS (TYmakeSimpleType (sty), shp));

        //*vardecs_p = TBmakeVardec( avis, *vardecs_p);
        FUNDEF_VARDECS (fundef) = TBmakeVardec (avis, FUNDEF_VARDECS (fundef));
    }

    new_assign = TBmakeAssign (TBmakeLet ((avis == NULL) ? avis : TBmakeIds (avis, NULL),
                                          (isprf) ? TBmakePrf (pfun, args) : args),
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
BuildInitializationAssigns (node *part, info *arg_info)
{
    size_t dim;
    cuidx_set_t *cis;
    cudim_set_t *cds;
    node *assigns = NULL, *fundef;

    DBUG_ENTER ();

    fundef = INFO_FUNDEF (arg_info);
    cis = (cuidx_set_t *)MEMmalloc (sizeof (cuidx_set_t));
    cds = (cudim_set_t *)MEMmalloc (sizeof (cudim_set_t));
    dim = TCcountIds (PART_IDS (part));

    CIS_TX (cis) = CreatePrfOrConst (TRUE, "tx", T_int, SHmakeShape (0),
                                     F_cuda_threadIdx_x, NULL, &assigns, fundef);

    CIS_BX (cis) = CreatePrfOrConst (TRUE, "bx", T_int, SHmakeShape (0),
                                     F_cuda_blockIdx_x, NULL, &assigns, fundef);

    CDS_BDIM_X (cds) = CreatePrfOrConst (TRUE, "bdim_x", T_int, SHmakeShape (0),
                                         F_cuda_blockDim_x, NULL, &assigns, fundef);

    CDS_GDIM_X (cds) = CreatePrfOrConst (TRUE, "gdim_x", T_int, SHmakeShape (0),
                                         F_cuda_gridDim_x, NULL, &assigns, fundef);

    if (dim == 2) {
        CIS_TY (cis) = CreatePrfOrConst (TRUE, "ty", T_int, SHmakeShape (0),
                                         F_cuda_threadIdx_y, NULL, &assigns, fundef);

        CIS_BY (cis) = CreatePrfOrConst (TRUE, "by", T_int, SHmakeShape (0),
                                         F_cuda_blockIdx_y, NULL, &assigns, fundef);

        CDS_BDIM_Y (cds) = CreatePrfOrConst (TRUE, "bdim_y", T_int, SHmakeShape (0),
                                             F_cuda_blockDim_y, NULL, &assigns, fundef);

        CDS_GDIM_Y (cds) = CreatePrfOrConst (TRUE, "gdim_y", T_int, SHmakeShape (0),
                                             F_cuda_gridDim_y, NULL, &assigns, fundef);
    }

    /* TODO: what happen for higer dimensionality, such as 3D? */

    INFO_CIS (arg_info) = cis;
    INFO_CDS (arg_info) = cds;

    DBUG_RETURN (assigns);
}

static node *
BuildLoadAssigns (node *part, info *arg_info)
{
    node *assigns = NULL, *fundef;
    node *avis, *args = NULL;
    node *shmem, *shmem_shp_array;
    shape *inner_shp;
    int inner_size;

    DBUG_ENTER ();

    fundef = INFO_FUNDEF (arg_info);

    /* Create an array to represent the shape of the shared memory array */
    if (INFO_RESDEF (arg_info) == def_array) {
        /*
         * TODO: What if the shared memory array size is larger than
         * the total amount available?
         */
        inner_shp = TYgetShape (AVIS_TYPE (INFO_CEXPR (arg_info)));

        shmem_shp_array = TCmakeIntVector (
          TCappendExprs (DUPdoDupTree (ARRAY_AELEMS (PART_THREADBLOCKSHAPE (part))),
                         SHshape2Exprs (inner_shp)));

        /* Number of elements of the inner array */
        inner_size = SHgetUnrLen (inner_shp);
    } else {
        /* TODO: We expect scalar INFO_CEXPR here, but can it also be an array? */
        shmem_shp_array = DUPdoDupNode (PART_THREADBLOCKSHAPE (part));
        inner_size = 0;
    }

    if (INFO_DIM (arg_info) == 1) {
        args = TBmakeExprs (DUPdoDupNode (PART_THREADBLOCKSHAPE (part)),
                            TBmakeExprs (TBmakeId (CIS_TX (INFO_CIS (arg_info))), NULL));
    } else if (INFO_DIM (arg_info) == 2) {
        args = TBmakeExprs (DUPdoDupNode (PART_THREADBLOCKSHAPE (part)),
                            TBmakeExprs (TBmakeId (CIS_TY (INFO_CIS (arg_info))),
                                         TBmakeExprs (TBmakeId (
                                                        CIS_TX (INFO_CIS (arg_info))),
                                                      NULL)));
    } else {
        /* TODO: should 3D, 4D and 5D be supported? */
        DBUG_ASSERT ((0), "Reduction is only supported for 1D and 2D folds!");
    }

    /*
     * Copmute the offset of each thread within the thread block via F_idxs2offet.
     *   1D array: offset = _idxs2offset( PART_THREADBLOCKSHAPE, tx);
     *   2D array: offset = _idxs2offset( PART_THREADBLOCKSHAPE, ty, tx);
     */
    avis = CreatePrfOrConst (TRUE, "idx_shr", T_int, SHmakeShape (0), F_idxs2offset, args,
                             &assigns, fundef);

    /*
     * If each individual element is itself an array, the offset will need to be
     * multiplied by the size of the array.
     */
    if (inner_size != 0) {
        args = TBmakeExprs (TBmakeId (avis), TBmakeExprs (TBmakeNum (inner_size), NULL));
        avis = CreatePrfOrConst (TRUE, "idx_shr", T_int, SHmakeShape (0), F_mul_SxS, args,
                                 &assigns, fundef);
    }

    /* Create shared memory */
    shmem = TBmakeAvis (TRAVtmpVarName ("shmem"),
                        TYmakeAKS (TYmakeSimpleType (
                                     CUh2shSimpleTypeConversion (TYgetSimpleType (
                                       TYgetScalar (AVIS_TYPE (INFO_CEXPR (arg_info)))))),
                                   SHarray2Shape (shmem_shp_array)));

    FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
      = TBmakeVardec (shmem, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

    /* Arguments for primitives idx_modarray_AxSxS or idx_modarray_AxSxA */
    args
      = TBmakeExprs (TBmakeId (shmem),
                     TBmakeExprs (TBmakeId (avis),
                                  TBmakeExprs (TBmakeId (INFO_CEXPR (arg_info)), NULL)));

    /*
     * Create idx_modarray_AxSxA or idx_modarray_AxSxS depending on whether
     * the inner result is an array or a scalar. This
     * essentially load the WITH-loop result INFO_CEXPR into the shared memory.
     */
    if (inner_size != 0) {
        /* Inner result is an array */
        avis = CreatePrfOrConst (TRUE, "shmem",
                                 TYgetSimpleType (TYgetScalar (AVIS_TYPE (shmem))),
                                 SHarray2Shape (shmem_shp_array), F_idx_modarray_AxSxA,
                                 args, &assigns, fundef);
    } else {
        /* Inner result is a scalar */
        avis = CreatePrfOrConst (TRUE, "shmem",
                                 TYgetSimpleType (TYgetScalar (AVIS_TYPE (shmem))),
                                 SHarray2Shape (shmem_shp_array), F_idx_modarray_AxSxS,
                                 args, &assigns, fundef);
    }

    args = TBmakeExprs (TBmakeId (avis), NULL);

    /* Synchronize the threads after each load */
    avis = CreatePrfOrConst (TRUE, "shmem",
                             TYgetSimpleType (TYgetScalar (AVIS_TYPE (shmem))),
                             SHarray2Shape (shmem_shp_array), F_syncthreads, args,
                             &assigns, fundef);

    /*
     * Store the shared memory avis (after load) into info structure so that
     * it can be accessed later by other functions (e.g. reduction and store)
     */
    INFO_SHRARRAY (arg_info) = avis;

    shmem_shp_array = FREEdoFreeNode (shmem_shp_array);

    DBUG_RETURN (assigns);
}

static node *
BuildReduceAssignsInternal (reduction_kind kind, int partshp, int partialshp,
                            int shmemshp, int remain, int inner_offset, node *part,
                            info *arg_info)
{
    simpletype sty;
    node *assigns = NULL, *fundef, *args = NULL, *avis;
    node *predicate = NULL, *iterator;
    node *loop_bound = NULL, *cond = NULL;
    node *idx1, *idx2, *val1, *val2, *val3;

    node *in_shared_array, *out_shared_array;
    node *out_shared_arrays[2];
    node *loop_assigns[2] = {NULL, NULL};
    node *loop_aps[2] = {NULL, NULL};

    node *reduction_assign = NULL;
    node *cond_predicate = NULL;
    node *tx_zero, *ty_zero;
    node *zero_indices = NULL;
    node *shmem_len, *remain_len = NULL, *partial_bound;
    node *shape_array;
    int iter, loops_required = 0;
    size_t i;

    DBUG_ENTER ();

    sty = TYgetSimpleType (TYgetScalar (AVIS_TYPE (INFO_SHRARRAY (arg_info))));

    in_shared_array = INFO_SHRARRAY (arg_info);
    fundef = INFO_FUNDEF (arg_info);

    /*
     * shape_array will be used as the first argument to all the F_idx2offset
     * primitives created in this function. It is essentially the shaped of the
     * shared memory. Note that for all the following uses, this variable should be
     * copied with DUPdoDupNode and when this function finishes, it should be
     * freed.
     */
    shape_array = SHshape2Array (TYgetShape (AVIS_TYPE (INFO_SHRARRAY (arg_info))));

    /* Loop iterator, initialized to 1 */
    iterator = CreatePrfOrConst (FALSE, "iterator", T_int, SHmakeShape (0), F_add_SxS,
                                 TBmakeNum (1), &assigns, fundef);

    /*
     * Test if the length of the partition is evenly divisible by
     * the length of the the shared memory along the same dimension. If not, we need
     * to generate the loop bound in way so that the last thread block
     * does not access out-of-bound elements in the shared memory.
     * If it is evenly dividible, then the loop bound is simply the length of the
     * of corresponding shared memory dimension.
     */
    shmem_len = CreatePrfOrConst (FALSE, "shmem_len", T_int, SHmakeShape (0), F_unknown,
                                  TBmakeNum (shmemshp), &assigns, fundef);

    if (remain != 0) {
        /* For 1D fold WITH-loop, we do not create two loops for reduction
         * as the loop will not be unrolled anyway */
        loops_required = (INFO_DIM (arg_info) == 1 ? 1 : 2);

        remain_len = CreatePrfOrConst (FALSE, "remain_len", T_int, SHmakeShape (0),
                                       F_unknown, TBmakeNum (remain), &assigns, fundef);

        if (kind == y_reduction) {
            args = TBmakeExprs (TBmakeId (CDS_GDIM_Y (INFO_CDS (arg_info))),
                                TBmakeExprs (TBmakeNum (1), NULL));
        } else if (kind == x_reduction) {
            args = TBmakeExprs (TBmakeId (CDS_GDIM_X (INFO_CDS (arg_info))),
                                TBmakeExprs (TBmakeNum (1), NULL));
        } else {
            DBUG_UNREACHABLE ("Reduction in unknown dimension!");
        }

        partial_bound = CreatePrfOrConst (TRUE, "partial_bound", T_int, SHmakeShape (0),
                                          F_sub_SxS, args, &assigns, fundef);

        if (kind == y_reduction) {
            args = TBmakeExprs (TBmakeId (CIS_BY (INFO_CIS (arg_info))),
                                TBmakeExprs (TBmakeId (partial_bound), NULL));
        } else if (kind == x_reduction) {
            args = TBmakeExprs (TBmakeId (CIS_BX (INFO_CIS (arg_info))),
                                TBmakeExprs (TBmakeId (partial_bound), NULL));
        } else {
            DBUG_UNREACHABLE ("Reduction in unknown dimension!");
        }

        /* Is the thread block the last one in the dimension? */
        cond = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0), F_neq_SxS, args,
                                 &assigns, fundef);

        /*
         * The F_cond primitive evaluates its condition (i.e. 1st argument) and
         * the result is the 2nd argument if the condition is true and 3rd argument
         * if the condition is false. So here we check the index of the thread
         * block (either X or Y dimension) and if it is NOT the last block in that
         * dimension, we set the loop bound to shmem_len. Otherwise, we set it
         * to remain_len.
         */
        if (INFO_DIM (arg_info) == 1) {
            args = TBmakeExprs (TBmakeId (cond),
                                TBmakeExprs (TBmakeId (shmem_len),
                                             TBmakeExprs (TBmakeId (remain_len), NULL)));

            loop_bound = CreatePrfOrConst (TRUE, "loop_bound", T_int, SHmakeShape (0),
                                           F_cond, args, &assigns, fundef);
        }
    } else {
        loops_required = 1;
    }

    for (iter = 0; iter < loops_required; iter++) {

        /* Now we construct the loop body which performs the actual reduction.
         * Depending on the dimension, reduction is carried out as following:
         *   1) y_reduction
         *        All threads in a block with ty == 0 executes the loop in parallel
         *        to reduce all elements from [0][tx] to [loop_bound][tx] into [0][tx].
         *        This is the first step to fold a two dimensional array.
         *   2) x_reduction
         *        The only thread in a block with tx == 0 executes the loop
         *        to reduce all elements from [0][0] to [0][loop_bound] into [0][0].
         *        This is the second step to fold a two dimensional array.
         */
        avis = CreatePrfOrConst (FALSE, "const", T_int, SHmakeShape (0), F_unknown,
                                 TBmakeNum (0), &loop_assigns[iter], fundef);

        if (kind == y_reduction) {
            /* [0, tx] */
            args = TBmakeExprs (DUPdoDupNode (shape_array),
                                TBmakeExprs (TBmakeId (avis),
                                             TBmakeExprs (TBmakeId (
                                                            CIS_TX (INFO_CIS (arg_info))),
                                                          NULL)));
        } else if (kind == x_reduction) {
            if (INFO_DIM (arg_info) == 1) {
                /* [0] */
                args = TBmakeExprs (DUPdoDupNode (shape_array),
                                    TBmakeExprs (TBmakeId (avis), NULL));
            } else if (INFO_DIM (arg_info) == 2) {
                /* [0, 0] */
                args = TBmakeExprs (DUPdoDupNode (shape_array),
                                    TBmakeExprs (TBmakeId (avis),
                                                 TBmakeExprs (TBmakeId (avis), NULL)));
            } else {
                DBUG_ASSERT ((0), "Dimension not supported!");
            }
        }

        /* If the inner element is an array instead of a scalar, we need
         * to first compute the initial offset of the element and then
         * add the offset within that array to the initial offset. Here
         * we first construct an exprs list with N 0's where N is the
         * dimentionalities of the inner element. This exprs list will
         * be appended to the args computed previously. The final exprs
         * list will then be used as argument to F_idxs2offset to compute
         * the initia offset. Finally, inner_offset will be added to the
         * initial offset to get the offset of the scalar element
         */
        /* When the inner element is defined by a withloop, we do NOT need
         * to add zeros to the list of indices. */
        if (INFO_RESDEF (arg_info) == def_array) {
            for (i = 0; i < INFO_INNERDIM (arg_info); i++) {
                avis
                  = CreatePrfOrConst (FALSE, "const", T_int, SHmakeShape (0), F_unknown,
                                      TBmakeNum (0), &loop_assigns[iter], fundef);
                if (zero_indices == NULL) {
                    zero_indices = TBmakeExprs (TBmakeId (avis), NULL);
                } else {
                    zero_indices
                      = TCappendExprs (zero_indices, TBmakeExprs (TBmakeId (avis), NULL));
                }
            }
            args = TCappendExprs (args, zero_indices);
        }

        /*
         * The following idx_sel selects the initial element to start the reduction.
         * for 1D array, this 1st element is at [0] and for 2D array this element
         * is at either [0][tx] or [0][0] depending on whether it is Y reduction
         * or X reduction
         */
        idx1 = CreatePrfOrConst (TRUE, "offset", T_int, SHmakeShape (0), F_idxs2offset,
                                 args, &loop_assigns[iter], fundef);

        args
          = TBmakeExprs (TBmakeId (idx1), TBmakeExprs (TBmakeNum (inner_offset), NULL));

        idx1 = CreatePrfOrConst (TRUE, "offset", T_int, SHmakeShape (0), F_add_SxS, args,
                                 &loop_assigns[iter], fundef);

        args = TBmakeExprs (TBmakeId (idx1),
                            TBmakeExprs (TBmakeId (INFO_SHRARRAY (arg_info)), NULL));

        /* Select from shared memory */
        val1 = CreatePrfOrConst (TRUE, "val", sty, SHmakeShape (0), F_idx_sel, args,
                                 &loop_assigns[iter], fundef);

        if (kind == y_reduction) {
            args = TBmakeExprs (DUPdoDupNode (shape_array),
                                TBmakeExprs (TBmakeId (iterator),
                                             TBmakeExprs (TBmakeId (
                                                            CIS_TX (INFO_CIS (arg_info))),
                                                          NULL)));
        } else if (kind == x_reduction) {
            if (INFO_DIM (arg_info) == 1) {
                args = TBmakeExprs (DUPdoDupNode (shape_array),
                                    TBmakeExprs (TBmakeId (iterator), NULL));
            } else if (INFO_DIM (arg_info) == 2) {
                args
                  = TBmakeExprs (DUPdoDupNode (shape_array),
                                 TBmakeExprs (TBmakeId (avis),
                                              TBmakeExprs (TBmakeId (iterator), NULL)));
            } else {
                DBUG_UNREACHABLE ("Dimension not supported!");
            }
        }

        if (zero_indices != NULL) {
            args = TCappendExprs (args, DUPdoDupTree (zero_indices));
        }

        /*
         * The following idx_sel selects an element to perform the reduction.
         * for 1D array, this element is at [loop_iteratior] and for 2D array
         * this element is at either [loop_iterator][tx] or [0][loop_iterator]
         * depending on whether it is Y reduction or X reduction
         */
        idx2 = CreatePrfOrConst (TRUE, "offset", T_int, SHmakeShape (0), F_idxs2offset,
                                 args, &loop_assigns[iter], fundef);

        args
          = TBmakeExprs (TBmakeId (idx2), TBmakeExprs (TBmakeNum (inner_offset), NULL));

        idx2 = CreatePrfOrConst (TRUE, "offset", T_int, SHmakeShape (0), F_add_SxS, args,
                                 &loop_assigns[iter], fundef);

        args = TBmakeExprs (TBmakeId (idx2),
                            TBmakeExprs (TBmakeId (INFO_SHRARRAY (arg_info)), NULL));

        val2 = CreatePrfOrConst (TRUE, "val", sty, SHmakeShape (0), F_idx_sel, args,
                                 &loop_assigns[iter], fundef);

        /*
         * Now we perform the fold operation on val1 and val2 to get val3.
         * The fold operation is stored in INFO_FOLDOP
         */
        args = TBmakeExprs (TBmakeId (val1), TBmakeExprs (TBmakeId (val2), NULL));

        val3
          = CreatePrfOrConst (TRUE, "val", sty, SHmakeShape (0), INFO_FOLDOP (arg_info),
                              args, &loop_assigns[iter], fundef);

        /*
         * Next the partial folding result is stored in the initial position
         * in the shared memory to be ready for the next iteration. The initial
         * position can be either [0](1D, X reduction), [0][tx](2D, Y reduction)
         * or [0][0](2D, X reduction).
         * TODO:
         *   This approach might be very inefficient since we are storing the
         *   partial result back into shared memory during each iteration instead
         *   of keeping it in a register. Not sure whether the nvcc compiler will
         *   dectect this and optimize it. If not, we will need to do it here.
         */
        args = TBmakeExprs (TBmakeId (INFO_SHRARRAY (arg_info)),
                            TBmakeExprs (TBmakeId (idx1),
                                         TBmakeExprs (TBmakeId (val3), NULL)));

        out_shared_arrays[iter]
          = CreatePrfOrConst (TRUE, "shmem", sty,
                              SHcopyShape (
                                TYgetShape (AVIS_TYPE (INFO_SHRARRAY (arg_info)))),
                              F_idx_modarray_AxSxS, args, &loop_assigns[iter], fundef);
    }

    /*
     * The following computes the condition to only allow certain threads to
     * perform the reduction. Note that we put it here because all the new
     * variables created here need to be added to the function
     * variable declaration list before the CLACdoCreateLacFun can be called.
     */
    if (kind == y_reduction) {
        /* if( ty == 0) */
        args = TBmakeExprs (TBmakeId (CIS_TY (INFO_CIS (arg_info))),
                            TBmakeExprs (TBmakeNum (0), NULL));

        cond_predicate = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0),
                                           F_eq_SxS, args, &predicate, fundef);
    } else if (kind == x_reduction) {
        if (INFO_DIM (arg_info) == 1) {
            /* if( tx == 0) */
            args = TBmakeExprs (TBmakeId (CIS_TX (INFO_CIS (arg_info))),
                                TBmakeExprs (TBmakeNum (0), NULL));

            cond_predicate = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0),
                                               F_eq_SxS, args, &predicate, fundef);
        } else if (INFO_DIM (arg_info) == 2) {
            /* if( ty == 0 && tx == 0) */
            args = TBmakeExprs (TBmakeId (CIS_TY (INFO_CIS (arg_info))),
                                TBmakeExprs (TBmakeNum (0), NULL));

            ty_zero = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0), F_eq_SxS,
                                        args, &predicate, fundef);

            args = TBmakeExprs (TBmakeId (CIS_TX (INFO_CIS (arg_info))),
                                TBmakeExprs (TBmakeNum (0), NULL));

            tx_zero = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0), F_eq_SxS,
                                        args, &predicate, fundef);

            args
              = TBmakeExprs (TBmakeId (tx_zero), TBmakeExprs (TBmakeId (ty_zero), NULL));
            cond_predicate = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0),
                                               F_and_SxS, args, &predicate, fundef);
        } else {
            DBUG_UNREACHABLE ("Dimension not supported!");
        }
    }

    INFO_SHRARRAY (arg_info) = out_shared_arrays[0];

    /* After this, "loop_aps[i]" is a loop function application */
    if (loops_required == 1) {
        /* If remainder is 0, just one loop is required */
        reduction_assign
          = CLFdoCreateLoopFun (INFO_FUNDEF (arg_info), loop_assigns[0], iterator,
                                (INFO_DIM (arg_info) == 1 ? loop_bound : shmem_len),
                                in_shared_array, out_shared_arrays[0],
                                &INFO_LACFUNS (arg_info));
    } else if (loops_required == 2) {
        loop_aps[0] = CLFdoCreateLoopFun (INFO_FUNDEF (arg_info), loop_assigns[0],
                                          iterator, shmem_len, in_shared_array,
                                          out_shared_arrays[0], &INFO_LACFUNS (arg_info));
        /*
         * If the remainder is 1, no reduction is needed. However,
         * since loops in SAC are do/while loop, which means that at least one
         * iteration will be executed and this will cause out-of-bound data to
         * be accessed. To guard against this case, we need to prevent the loop
         * from being exectued at all if the remainder is 1. After this, "assigns"
         * will be the final assignment chain used to create the conditional
         * function (perform X or Y reduction based on thread indices).
         */
        if (remain == 1) {
            /*
             * If remainder is 1, the loop will only be executed for thread block
             * which are not the last one. The condition "cond" is computed earlier
             * to check whether a thread block is the last one or not
             */
            reduction_assign
              = CCFdoCreateCondFun (INFO_FUNDEF (arg_info), loop_aps[0], NULL, cond,
                                    in_shared_array, out_shared_arrays[0], NULL,
                                    &INFO_LACFUNS (arg_info));
            loop_assigns[1] = FREEdoFreeTree (loop_assigns[1]);
        } else {
            loop_aps[1]
              = CLFdoCreateLoopFun (INFO_FUNDEF (arg_info), loop_assigns[1], iterator,
                                    remain_len, in_shared_array, out_shared_arrays[1],
                                    &INFO_LACFUNS (arg_info));

            reduction_assign
              = CCFdoCreateCondFun (INFO_FUNDEF (arg_info), loop_aps[0], loop_aps[1],
                                    cond, in_shared_array, out_shared_arrays[0],
                                    out_shared_arrays[1], &INFO_LACFUNS (arg_info));
        }
    } else {
        DBUG_ASSERT ((0), "No more than two loops will be needed!");
    }

    /* Chain up the assigments */
    assigns = TCappendAssign (assigns, reduction_assign);

    /* Now we create the conditional function with body "assigns" */
    out_shared_array = INFO_SHRARRAY (arg_info);

    /* After this, "assigns" is the function application of the conditional function */
    assigns = CCFdoCreateCondFun (INFO_FUNDEF (arg_info), assigns, NULL, cond_predicate,
                                  in_shared_array, out_shared_array, NULL,
                                  &INFO_LACFUNS (arg_info));

    /* Concatenate the condition evaluation assignments and the conditional function
     * application*/
    assigns = TCappendAssign (predicate, assigns);

    args = TBmakeExprs (TBmakeId (INFO_SHRARRAY (arg_info)), NULL);

    /* Synchronising the threads in the block after the conditional */
    avis
      = CreatePrfOrConst (TRUE, "shmem", sty,
                          SHcopyShape (TYgetShape (AVIS_TYPE (INFO_SHRARRAY (arg_info)))),
                          F_syncthreads, args, &assigns, fundef);

    INFO_SHRARRAY (arg_info) = avis;

    shape_array = FREEdoFreeNode (shape_array);

    DBUG_RETURN (assigns);
}

static node *
BuildReduceAssigns (node *part, info *arg_info)
{
    int partshp_x, partshp_y, partialshp_y, partialshp_x, shmemshp_x, shmemshp_y,
      remain_x, remain_y;
    int inner_size, i;
    node *assigns = NULL, *y_reduction_assigns = NULL, *x_reduction_assigns = NULL;

    DBUG_ENTER ();

    /* Outer most fold WITH-loop is 1-dimensional */
    if (INFO_DIM (arg_info) == 1) {
        /* Shape of the partition */
        partshp_x = NUM_VAL (EXPRS_EXPR1 (ARRAY_AELEMS (INFO_PARTSHP (arg_info))));
        /* Shape of the partial result array */
        partialshp_x = NUM_VAL (EXPRS_EXPR1 (ARRAY_AELEMS (INFO_PARTIALSHP (arg_info))));
        /* Shape of the partition thread block */
        shmemshp_x = NUM_VAL (EXPRS_EXPR1 (ARRAY_AELEMS (PART_THREADBLOCKSHAPE (part))));

        remain_x = partshp_x % shmemshp_x;

        if (INFO_RESDEF (arg_info) == def_array) {
            /*
             * Each loop iteration builds up the code to reduce all elements
             * of the inner array at one position
             */
            inner_size = SHgetUnrLen (TYgetShape (AVIS_TYPE (INFO_CEXPR (arg_info))));
            for (i = 0; i < inner_size; i++) {
                if (assigns == NULL) {
                    assigns = BuildReduceAssignsInternal (x_reduction, partshp_x,
                                                          partialshp_x, shmemshp_x,
                                                          remain_x, i, part, arg_info);
                } else {
                    assigns
                      = TCappendAssign (assigns,
                                        BuildReduceAssignsInternal (x_reduction,
                                                                    partshp_x,
                                                                    partialshp_x,
                                                                    shmemshp_x, remain_x,
                                                                    i, part, arg_info));
                }
            }
        } else {
            /* Assume inner elements are scalars */
            assigns
              = BuildReduceAssignsInternal (x_reduction, partshp_x, partialshp_x,
                                            shmemshp_x, remain_x, 0, part, arg_info);
        }
    } else if (INFO_DIM (arg_info) == 2) {

        partshp_y = NUM_VAL (EXPRS_EXPR1 (ARRAY_AELEMS (INFO_PARTSHP (arg_info))));
        partshp_x = NUM_VAL (EXPRS_EXPR2 (ARRAY_AELEMS (INFO_PARTSHP (arg_info))));
        partialshp_y = NUM_VAL (EXPRS_EXPR1 (ARRAY_AELEMS (INFO_PARTIALSHP (arg_info))));
        partialshp_x = NUM_VAL (EXPRS_EXPR2 (ARRAY_AELEMS (INFO_PARTIALSHP (arg_info))));
        shmemshp_y = NUM_VAL (EXPRS_EXPR1 (ARRAY_AELEMS (PART_THREADBLOCKSHAPE (part))));
        shmemshp_x = NUM_VAL (EXPRS_EXPR2 (ARRAY_AELEMS (PART_THREADBLOCKSHAPE (part))));

        remain_y = partshp_y % shmemshp_y;
        remain_x = partshp_x % shmemshp_x;

        if (INFO_RESDEF (arg_info) == def_array) {
            /* TODO: Similar to the 1D case */
            assigns = NULL;
        } else {
            y_reduction_assigns
              = BuildReduceAssignsInternal (y_reduction, partshp_y, partialshp_y,
                                            shmemshp_y, remain_y, 0, part, arg_info);

            if (INFO_INNERDIM (arg_info) == 0) {
                x_reduction_assigns
                  = BuildReduceAssignsInternal (x_reduction, partshp_x, partialshp_x,
                                                shmemshp_x, remain_x, 0, part, arg_info);
            }

            assigns = TCappendAssign (y_reduction_assigns, x_reduction_assigns);
        }
    } else {
        DBUG_UNREACHABLE ("Reduction is only supported for 1D and 2D folds!");
    }

    DBUG_RETURN (assigns);
}

static node *
BuildStoreAssigns (node *part, info *arg_info)
{
    node *fundef, *assigns = NULL;
    node *args = NULL, *avis = NULL, *tx_zero, *ty_zero, *cnst;
    simpletype sty;
    node *idx, *target_idx;
    int i, inner_size;
    node *last_arg, *vec_avis;
    node *cond;

    DBUG_ENTER ();

    sty = TYgetSimpleType (TYgetScalar (AVIS_TYPE (INFO_SHRARRAY (arg_info))));
    vec_avis = IDS_AVIS (PART_VEC (part));
    fundef = INFO_FUNDEF (arg_info);

    if (INFO_DIM (arg_info) == 1) {
        if (INFO_RESDEF (arg_info) == def_array) {
            inner_size = SHgetUnrLen (TYgetShape (AVIS_TYPE (INFO_CEXPR (arg_info))));

            args
              = TBmakeExprs (TCmakeIntVector (
                               TBmakeExprs (DUPdoDupNode (EXPRS_EXPR1 (
                                              ARRAY_AELEMS (INFO_PARTIALSHP (arg_info)))),
                                            NULL)),
                             TBmakeExprs (TBmakeId (CIS_BX (INFO_CIS (arg_info))), NULL));

            idx = CreatePrfOrConst (TRUE, "offset", T_int, SHmakeShape (0), F_idxs2offset,
                                    args, &assigns, fundef);

            args
              = TBmakeExprs (TBmakeId (idx), TBmakeExprs (TBmakeNum (inner_size), NULL));

            idx = CreatePrfOrConst (TRUE, "offset", T_int, SHmakeShape (0), F_mul_SxS,
                                    args, &assigns, fundef);

            /* if( tx == 0) */
            args = TBmakeExprs (TBmakeId (CIS_TX (INFO_CIS (arg_info))),
                                TBmakeExprs (TBmakeNum (0), NULL));

            cond = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0), F_eq_SxS,
                                     args, &assigns, fundef);

            /* For each inner array element, we perform a store */
            last_arg = vec_avis;
            for (i = 0; i < inner_size; i++) {
                cnst = CreatePrfOrConst (FALSE, "cnst", T_int, SHmakeShape (0), F_unknown,
                                         TBmakeNum (i), &assigns, fundef);

                args = TBmakeExprs (TBmakeId (idx), TBmakeExprs (TBmakeNum (i), NULL));

                target_idx = CreatePrfOrConst (TRUE, "offset", T_int, SHmakeShape (0),
                                               F_add_SxS, args, &assigns, fundef);

                args = TBmakeExprs (
                  TBmakeId (cond),
                  TBmakeExprs (TBmakeId (cnst),
                               TBmakeExprs (TBmakeId (INFO_SHRARRAY (arg_info)),
                                            TBmakeExprs (TBmakeId (target_idx),
                                                         TBmakeExprs (TBmakeId (last_arg),
                                                                      NULL)))));

                avis = CreatePrfOrConst (TRUE, "res", sty, SHmakeShape (0),
                                         F_cond_wl_assign, args, &assigns, fundef);

                last_arg = avis;
            }
        } else if (INFO_RESDEF (arg_info) == def_scalar) {
            /*
             * If we are in the else branch, the result must be defined as a
             * a scalar and cannot be a withloop. This is because if it it is
             * defined by a withloop we would have scalarized it and then
             * INOF_DIM must be at least 2 and cannot be 1. The following code
             * generate the following code sequence:
             *
             * partition: [0] <= iv=[eat] < [ub]
             *
             * idx = F_idxs2offset( [partial_shp], blockIdx.x);
             * cond = F_eq_SxS( threadIdx.x, 0);
             * cnst = 0;
             * res = F_cond_wl_assign( cond, cnst, shmem, idx);
             */
            args
              = TBmakeExprs (DUPdoDupNode (INFO_PARTIALSHP (arg_info)),
                             TBmakeExprs (TBmakeId (CIS_BX (INFO_CIS (arg_info))), NULL));

            target_idx = CreatePrfOrConst (TRUE, "offset", T_int, SHmakeShape (0),
                                           F_idxs2offset, args, &assigns, fundef);

            args = TBmakeExprs (TBmakeId (CIS_TX (INFO_CIS (arg_info))),
                                TBmakeExprs (TBmakeNum (0), NULL));
            avis = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0), F_eq_SxS,
                                     args, &assigns, fundef);

            cnst = CreatePrfOrConst (FALSE, "cnst", T_int, SHmakeShape (0), F_unknown,
                                     TBmakeNum (0), &assigns, fundef);

            args = TBmakeExprs (
              TBmakeId (avis),
              TBmakeExprs (TBmakeId (cnst),
                           TBmakeExprs (TBmakeId (INFO_SHRARRAY (arg_info)),
                                        TBmakeExprs (TBmakeId (target_idx),
                                                     TBmakeExprs (TBmakeId (vec_avis),
                                                                  NULL)))));

            avis = CreatePrfOrConst (TRUE, "res", sty, SHmakeShape (0), F_cond_wl_assign,
                                     args, &assigns, fundef);
        } else {
            DBUG_ASSERT ((0), "Result has wrong defined type!");
        }
    } else if (INFO_DIM (arg_info) == 2) {

        if (INFO_RESDEF (arg_info) == def_scalar) {
            /*
             * partition: ( [0,0] <= iv=[eat1,eat2] < [ub0,ub1])
             *
             * idx = F_idxs2offset( [partial_shp], blockIdx.y, blockIdx.x);
             */
            args = TBmakeExprs (DUPdoDupNode (INFO_PARTIALSHP (arg_info)),
                                TBmakeExprs (TBmakeId (CIS_BY (INFO_CIS (arg_info))),
                                             TBmakeExprs (TBmakeId (
                                                            CIS_BX (INFO_CIS (arg_info))),
                                                          NULL)));
            target_idx = CreatePrfOrConst (TRUE, "offset", T_int, SHmakeShape (0),
                                           F_idxs2offset, args, &assigns, fundef);

            /*
             * cond1 = F_eq_SxS( threadIdx.x, 0);
             * cond2 = F_eq_SxS( threadIdx.y, 0);
             * cond3 = F_and_SxS( cond1, cond2);
             * cnst = 0;
             * res = F_cond_wl_assign( cond3, cnst, shmem, idx);
             */
            args = TBmakeExprs (TBmakeId (CIS_TX (INFO_CIS (arg_info))),
                                TBmakeExprs (TBmakeNum (0), NULL));
            tx_zero = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0), F_eq_SxS,
                                        args, &assigns, fundef);

            args = TBmakeExprs (TBmakeId (CIS_TY (INFO_CIS (arg_info))),
                                TBmakeExprs (TBmakeNum (0), NULL));
            ty_zero = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0), F_eq_SxS,
                                        args, &assigns, fundef);

            args
              = TBmakeExprs (TBmakeId (tx_zero), TBmakeExprs (TBmakeId (ty_zero), NULL));
            avis = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0), F_and_SxS,
                                     args, &assigns, fundef);

            cnst = CreatePrfOrConst (FALSE, "cnst", T_int, SHmakeShape (0), F_unknown,
                                     TBmakeNum (0), &assigns, fundef);

            args = TBmakeExprs (
              TBmakeId (avis),
              TBmakeExprs (TBmakeId (cnst),
                           TBmakeExprs (TBmakeId (INFO_SHRARRAY (arg_info)),
                                        TBmakeExprs (TBmakeId (target_idx),
                                                     TBmakeExprs (TBmakeId (vec_avis),
                                                                  NULL)))));
        } else if (INFO_RESDEF (arg_info) == def_withloop) {
            /*
             * partition: ( [0,0] <= iv=[eat1,eat2] < [ub0,ub1])
             *
             * idx = F_idxs2offset( [partial_shp], blockIdx.y, eat2);
             */
            args = TBmakeExprs (DUPdoDupNode (INFO_PARTIALSHP (arg_info)),
                                TBmakeExprs (TBmakeId (CIS_BY (INFO_CIS (arg_info))),
                                             TBmakeExprs (TBmakeId (IDS_AVIS (IDS_NEXT (
                                                            INFO_WITHIDS (arg_info)))),
                                                          NULL)));
            target_idx = CreatePrfOrConst (TRUE, "offset", T_int, SHmakeShape (0),
                                           F_idxs2offset, args, &assigns, fundef);

            /*
             * cond = F_eq_SxS( threadIdx.y, 0);
             * res = F_cond_wl_assign( cond, threadIdx.x, shmem, idx);
             */
            args = TBmakeExprs (TBmakeId (CIS_TY (INFO_CIS (arg_info))),
                                TBmakeExprs (TBmakeNum (0), NULL));
            avis = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0), F_eq_SxS,
                                     args, &assigns, fundef);

            args = TBmakeExprs (
              TBmakeId (avis),
              TBmakeExprs (TBmakeId (CIS_TX (INFO_CIS (arg_info))),
                           TBmakeExprs (TBmakeId (INFO_SHRARRAY (arg_info)),
                                        TBmakeExprs (TBmakeId (target_idx),
                                                     TBmakeExprs (TBmakeId (vec_avis),
                                                                  NULL)))));
        } else if (INFO_RESDEF (arg_info) == def_array) {
            /* TODO */
        } else {
            DBUG_UNREACHABLE ("Unknow result definition assignment!");
        }

        /*
         * Primitive of the form:
         *    res = F_cond_wl_assign( cond, shmemidx, shmem, devidx, [devmem])
         * will be compiled into:
         *
         * if( cond) {
         *   devmem[devidx] = shmem[shmemidx];
         * }
         *
         * Note that devmem is has not been added in this phase. It will be
         * added in memory management and it denotes the memory space where
         * the partial folding result will be stored.
         */
        avis = CreatePrfOrConst (TRUE, "res", sty, SHmakeShape (0), F_cond_wl_assign,
                                 args, &assigns, fundef);
    }

    ID_AVIS (EXPRS_EXPR (PART_CEXPRS (part))) = avis;

    DBUG_RETURN (assigns);
}

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *PFDdoPartialFold( node *syntax_tree)
 *
 *****************************************************************************/
node *
PFDdoPartialFold (node *syntax_tree)
{
    info *info;
    DBUG_ENTER ();

    info = MakeInfo ();
    TRAVpush (TR_pfd);
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
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *PFDmodule( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
PFDmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    /* Append all newly created lac functions to the module */
    if (INFO_LACFUNS (arg_info) != NULL) {
        MODULE_FUNS (arg_node)
          = TCappendFundef (MODULE_FUNS (arg_node), INFO_LACFUNS (arg_info));
        INFO_LACFUNS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PFDfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
PFDfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PFDassign( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
PFDassign (node *arg_node, info *arg_info)
{
    node *old_lastassign;

    DBUG_ENTER ();

    /* Bottom up traversal, easier to append new assignments */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    /* LASTASSIGN stores the currently enclosing assignment */
    old_lastassign = INFO_LASTASSIGN (arg_info);
    INFO_LASTASSIGN (arg_info) = arg_node;
    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);
    INFO_LASTASSIGN (arg_info) = old_lastassign;

    /* If we are not in any withloop */
    if (INFO_POSTASSIGN (arg_info) != NULL && INFO_LEVEL (arg_info) == 0) {
        ASSIGN_NEXT (arg_node)
          = TCappendAssign (INFO_POSTASSIGN (arg_info), ASSIGN_NEXT (arg_node));
        INFO_POSTASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PFDlet( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
PFDlet (node *arg_node, info *arg_info)
{
    node *old_lhs;

    DBUG_ENTER ();

    old_lhs = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);
    INFO_LHS (arg_info) = old_lhs;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PFDcode( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
PFDcode (node *arg_node, info *arg_info)
{
    node *rhs, *cexpr_avis, *first_elem;

    DBUG_ENTER ();

    /* collect the type of the result and the folding operation */
    cexpr_avis = ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (arg_node)));
    INFO_CEXPR (arg_info) = cexpr_avis;

    rhs = ASSIGN_RHS (AVIS_SSAASSIGN (cexpr_avis));

    /*
     * The only case cexpr is NOT a scalar is when it's been
     * defined by an array. Note that if it's defined by a
     * genarray/modarray, it will be scalarized and the result
     * will be either a scalar or an array
     */
    if (TUisScalar (AVIS_TYPE (cexpr_avis))) {
        INFO_FOLDOP (arg_info) = PRF_PRF (rhs);
    } else {
        DBUG_ASSERT (NODE_TYPE (rhs) == N_array,
                     "Non-scalar result is not defined as an array!");

        first_elem = EXPRS_EXPR1 (ARRAY_AELEMS (rhs));

        DBUG_ASSERT (NODE_TYPE (first_elem) == N_id,
                     "First array element is not an N_id!");

        /*
         * If the result is defined by an array, we need to find out the
         * operation of the fold withloop by following the SSA
         * assign of its elements. Here we choose to start tracing back
         * from the first array elemnt althought any array element will do.
         */
        INFO_FOLDOP (arg_info)
          = PRF_PRF (ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (first_elem))));
    }

    /* TODO: Do we need to traverse the code? */
    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PFDgenerator( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
PFDgenerator (node *arg_node, info *arg_info)
{
    node *bound1;
    node *bound2;

    DBUG_ENTER ();

    bound1 = GENERATOR_BOUND1 (arg_node);
    bound2 = GENERATOR_BOUND2 (arg_node);

    /* We expect both bounds to be constants */
    DBUG_ASSERT (COisConstant (bound1), "Bound1 is not constant");
    DBUG_ASSERT (COisConstant (bound2), "Bound2 is not constant");

    INFO_PARTSHP (arg_info)
      = COconstant2AST (COsub (COaST2Constant (bound2), COaST2Constant (bound1), NULL));

    DBUG_ASSERT (NODE_TYPE (INFO_PARTSHP (arg_info)) == N_array,
                 "Partition shape is not an array!");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PFDwithid( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
PFDwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* Work out the dimensionality of the partition and
     * remember the scalarized partition indices */
    INFO_DIM (arg_info) = TCcountIds (WITHID_IDS (arg_node));
    INFO_WITHIDS (arg_info) = WITHID_IDS (arg_node);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PFDpart( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
PFDpart (node *arg_node, info *arg_info)
{
    int tbshp_len, partshp_len, partialshp_len;
    size_t reduce_dim = 0, dim;
    node *tbshp_elems = NULL, *partshp_elems = NULL, *partialshp_elems = NULL;
    node *init_assigns, *load_assigns, *reduce_assigns, *store_assigns;

    DBUG_ENTER ();

    DBUG_ASSERT (PART_NEXT (arg_node) == NULL,
                 "Found fold WITH-loop with more than one partition!");

    /*
     * Work out the dimensionality of this part and store it in INFO_DIM.
     * Also stores the scalarized WITH-loop indices in INFO_WITHIDS
     */
    PART_WITHID (arg_node) = TRAVopt (PART_WITHID (arg_node), arg_info);

    /*
     * Work out the shape of this part as the difference between upper
     * bound and lower bound and store it in INFO_PARTSHP
     */
    PART_GENERATOR (arg_node) = TRAVopt (PART_GENERATOR (arg_node), arg_info);

    /*
     * Work out the folding operation (i.e. primitive operation) of this fold withloop
     * from the code cexpr and store it in INFO_FOLDOP. Also traverse into
     * the code block.
     */
    PART_CODE (arg_node) = TRAVopt (PART_CODE (arg_node), arg_info);

    /*
     * Array elements of the thread block configuration. In cases where WITH-loop
     * scalarization has been performed, this is changed accordingly.
     */
    tbshp_elems = ARRAY_AELEMS (PART_THREADBLOCKSHAPE (arg_node));

    /* Array elements of the partition shape */
    partshp_elems = ARRAY_AELEMS (INFO_PARTSHP (arg_info));

    /*
     * reduce_dim is actually the dimensions we would like the to reduce along.
     * When it comes to create CUDA threads for the fold withloop, these are
     * also the dimensions that we will create threads for.
     * If the fold with loop result is defined by a WITH-loop, then INFO_DIM is
     * the sum of the dim of the original outer fold WITH-loop and the original
     * inner genarray/modarray WITH-loop (because of WITH-loop scalarization)
     * TODO:
     *   What if the fold withloop contains an inner genarray whose result is
     *   defined by an array? Is this def_array or def_withloop?
     */
    if (INFO_RESDEF (arg_info) == def_scalar || INFO_RESDEF (arg_info) == def_array) {
        /* No WITH-loop scalarization, all orginal fold WITH-loop
         * dimensions are fodable. */
        reduce_dim = INFO_DIM (arg_info);
    } else if (INFO_RESDEF (arg_info) == def_withloop) {
        /* WITH-loop scalarization, only original fold WITH-loop
         * dimensions are foldable but not the newly added inner
         * WITH-loop dimensions */
        reduce_dim = INFO_DIM (arg_info) - INFO_INNERDIM (arg_info);
    } else {
        DBUG_ASSERT ((0), "Unknown defining assignment type!");
    }

    /*
     * This loop constructs the array elements (i.e. partialshp_elems) used
     * create the shape of the partial folded array.
     */
    dim = 0;
    while (dim < INFO_DIM (arg_info)) {
        DBUG_ASSERT (tbshp_elems != NULL, "Thread shape is NULL!");
        DBUG_ASSERT (partshp_elems != NULL, "Partition shape is NULL!");

        /* tbshp_len and partshp_len must be of equal length */
        tbshp_len = NUM_VAL (EXPRS_EXPR (tbshp_elems));
        partshp_len = NUM_VAL (EXPRS_EXPR (partshp_elems));

        /*
         * If the dimesnion is the one we would like to perform reduction
         * along, the length of the corresponding partial array dimesnion
         * is partshp_len/tbshp_len. For example, if the partition is of
         * shape [128,512] and the thread block shape is [16,16] and the
         * result is scalar defined, then the partial array shape will be
         * [8,32]
         */
        if (dim < reduce_dim) {
            if (partshp_len % tbshp_len == 0) {
                partialshp_len = partshp_len / tbshp_len;
            } else {
                partialshp_len = partshp_len / tbshp_len + 1;
            }
        } else {
            /*
             * If the partition shape is [128,512,64] and the last dimension
             * is a result of the scalarization, then the partial array shape
             * is [8,32,64] given a thread block [16,16]
             */
            partialshp_len = partshp_len;
        }

        /* Create an N_exprs */
        if (partialshp_elems == NULL) {
            partialshp_elems = TBmakeExprs (TBmakeNum (partialshp_len), NULL);
        } else {
            partialshp_elems
              = TCappendExprs (partialshp_elems,
                               TBmakeExprs (TBmakeNum (partialshp_len), NULL));
        }

        tbshp_elems = EXPRS_NEXT (tbshp_elems);
        partshp_elems = EXPRS_NEXT (partshp_elems);

        dim++;
    }

    /* Now we create the N_array to represent the shape of the partial array */
    INFO_PARTIALSHP (arg_info)
      = TBmakeArray (TYmakeSimpleType (T_int), SHcreateShape (1, INFO_DIM (arg_info)),
                     partialshp_elems);

    /*
     * If the result is defined by an N_array, dimensions of the array itself
     * are not appended to partialshp_elems in the previous loop. To construct
     * a correct shape for the partial array, we need to append the inner
     * dimensions to partialshp_elems here.
     */
    if (INFO_RESDEF (arg_info) == def_array) {
        ARRAY_AELEMS (INFO_PARTIALSHP (arg_info))
          = TCappendExprs (ARRAY_AELEMS (INFO_PARTIALSHP (arg_info)),
                           SHshape2Exprs (
                             TYgetShape (AVIS_TYPE (INFO_CEXPR (arg_info)))));
        ARRAY_FRAMESHAPE (INFO_PARTIALSHP (arg_info))
          = SHfreeShape (ARRAY_FRAMESHAPE (INFO_PARTIALSHP (arg_info)));
        ARRAY_FRAMESHAPE (INFO_PARTIALSHP (arg_info))
          = SHcreateShape (1, INFO_DIM (arg_info) + INFO_INNERDIM (arg_info));
    }

    /*****************************************************************************/

    /* Create assignments for CUDA index initialization */
    init_assigns = BuildInitializationAssigns (arg_node, arg_info);

    BLOCK_ASSIGNS (PART_CBLOCK (arg_node))
      = TCappendAssign (init_assigns, BLOCK_ASSIGNS (PART_CBLOCK (arg_node)));

    /* Create assignments for loading data into shared memory */
    load_assigns = BuildLoadAssigns (arg_node, arg_info);

    BLOCK_ASSIGNS (PART_CBLOCK (arg_node))
      = TCappendAssign (BLOCK_ASSIGNS (PART_CBLOCK (arg_node)), load_assigns);

    /* Create assignments for performing the actual reduction */
    reduce_assigns = BuildReduceAssigns (arg_node, arg_info);

    BLOCK_ASSIGNS (PART_CBLOCK (arg_node))
      = TCappendAssign (BLOCK_ASSIGNS (PART_CBLOCK (arg_node)), reduce_assigns);

    /* Create assignments for storing the reduction result from
     * shared memory to the partial array in global memory */
    store_assigns = BuildStoreAssigns (arg_node, arg_info);

    BLOCK_ASSIGNS (PART_CBLOCK (arg_node))
      = TCappendAssign (BLOCK_ASSIGNS (PART_CBLOCK (arg_node)), store_assigns);

    /* This struct is created in BuildInitializationAssigns and freed here */
    INFO_CIS (arg_info) = MEMfree (INFO_CIS (arg_info));
    INFO_PARTSHP (arg_info) = FREEdoFreeTree (INFO_PARTSHP (arg_info));

    DBUG_RETURN (arg_node);
}

static node *
BuildFoldWithloop (node *old_foldwl, info *arg_info)
{
    size_t dim;
    node *new_foldwl = NULL, *avis;
    node *part, *new_code, *withop;
    node *withid, *generator;
    node *vec, *ids = NULL;
    node *bound1, *bound2;
    node *accu_avis, *offset_avis, *elem_avis, *op_avis;
    node *accu_ass, *offset_ass, *elem_ass, *op_ass;
    ntype *cexpr_type;
    node *code_instr;
    prf op;

    DBUG_ENTER ();

    if (INFO_RESDEF (arg_info) == def_scalar) {
        /* iv */
        avis = TBmakeAvis (TRAVtmpVarName ("iv"),
                           TYmakeAKS (TYmakeSimpleType (T_int),
                                      SHcreateShape (1, INFO_DIM (arg_info))));
        vec = TBmakeIds (avis, NULL);
        INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), avis);

        /* ids */
        dim = 0;
        while (dim < INFO_DIM (arg_info)) {
            avis = TBmakeAvis (TRAVtmpVarName ("ids"),
                               TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));

            if (ids == NULL) {
                ids = TBmakeIds (avis, NULL);
            } else {
                ids = TCappendIds (ids, TBmakeIds (avis, NULL));
            }
            INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), avis);
            dim++;
        }

        /* withid */
        withid = TBmakeWithid (vec, ids);

        /* bound1 & bound2 */
        bound1 = SHshape2Array (SHmakeShape (INFO_DIM (arg_info)));
        bound2 = DUPdoDupNode (INFO_PARTIALSHP (arg_info));

        /* generator */
        generator = TBmakeGenerator (F_wl_le, F_wl_lt, bound1, bound2, NULL, NULL);

        /* Set genwidth */
        GENERATOR_GENWIDTH (generator) = DUPdoDupNode (INFO_PARTIALSHP (arg_info));

        /* code */
        cexpr_type = AVIS_TYPE (INFO_CEXPR (arg_info));
        op = INFO_FOLDOP (arg_info);

        /* tmp1 = accu(iv); */
        accu_avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (cexpr_type));
        accu_ass
          = TBmakeAssign (TBmakeLet (TBmakeIds (accu_avis, NULL),
                                     TCmakePrf1 (F_accu, TBmakeId (IDS_AVIS (vec)))),
                          NULL);
        AVIS_SSAASSIGN (accu_avis) = accu_ass;
        INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), accu_avis);

        /* tmp2 = idxs2offset([...], i, j, k...); */
        offset_avis = TBmakeAvis (TRAVtmpVar (),
                                  TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
        offset_ass
          = TBmakeAssign (TBmakeLet (TBmakeIds (offset_avis, NULL),
                                     TBmakePrf (F_idxs2offset,
                                                TBmakeExprs (DUPdoDupNode (
                                                               INFO_PARTIALSHP (
                                                                 arg_info)),
                                                             TCids2Exprs (ids)))),
                          NULL);
        AVIS_SSAASSIGN (offset_avis) = offset_ass;
        INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), offset_avis);

        /* tmp3 = idx_sel( offset, array); */
        elem_avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (cexpr_type));
        elem_ass
          = TBmakeAssign (TBmakeLet (TBmakeIds (elem_avis, NULL),
                                     TCmakePrf2 (F_idx_sel, TBmakeId (offset_avis),
                                                 TBmakeId (INFO_PARTIALARR (arg_info)))),
                          NULL);
        AVIS_SSAASSIGN (elem_avis) = elem_ass;
        INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), elem_avis);

        /* tmp4 = op( arg_1, arg_2); */
        op_avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (cexpr_type));
        op_ass = TBmakeAssign (TBmakeLet (TBmakeIds (op_avis, NULL),
                                          TCmakePrf2 (op, TBmakeId (accu_avis),
                                                      TBmakeId (elem_avis))),
                               NULL);
        AVIS_SSAASSIGN (op_avis) = op_ass;
        INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), op_avis);

        /* Update the new partition */
        code_instr
          = TCappendAssign (accu_ass,
                            TCappendAssign (offset_ass,
                                            TCappendAssign (elem_ass,
                                                            TCappendAssign (op_ass,
                                                                            NULL))));
        new_code = TBmakeCode (TBmakeBlock (code_instr, NULL),
                               TBmakeExprs (TBmakeId (op_avis), NULL));
        CODE_USED (new_code) = 1;
        CODE_NEXT (new_code) = NULL;

        /* Create new partition */
        part = TBmakePart (new_code, withid, generator);

        /* fold withop */
        withop = TBmakeFold (INFO_FOLDFUNDEF (arg_info),
                             DUPdoDupNode (INFO_NEUTRAL (arg_info)));

        /* new fold withloop */
        new_foldwl = TBmakeWith (part, new_code, withop);
        WITH_PARTS (new_foldwl) = 1;
        WITH_REFERENCED (new_foldwl) = WITH_REFERENCED (old_foldwl);
        WITH_ISFOLDABLE (new_foldwl) = WITH_ISFOLDABLE (old_foldwl);
    } else if (INFO_RESDEF (arg_info) == def_array) {
        node *outer_ub_elems = NULL, *partialshp_elems;
        int inner_size, i;
        node *args, *assigns = NULL, *fundef;
        node *elem_avis1, *elem_avis2, *res_elems = NULL;

        fundef = INFO_FUNDEF (arg_info);
        partialshp_elems = ARRAY_AELEMS (INFO_PARTIALSHP (arg_info));

        /* iv */
        avis = TBmakeAvis (TRAVtmpVarName ("iv"),
                           TYmakeAKS (TYmakeSimpleType (T_int),
                                      SHcreateShape (1, INFO_DIM (arg_info))));
        vec = TBmakeIds (avis, NULL);
        INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), avis);

        /* ids */
        dim = 0;
        while (dim < INFO_DIM (arg_info)) {
            avis = TBmakeAvis (TRAVtmpVarName ("ids"),
                               TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
            INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), avis);

            if (ids == NULL) {
                ids = TBmakeIds (avis, NULL);
            } else {
                ids = TCappendIds (ids, TBmakeIds (avis, NULL));
            }

            if (outer_ub_elems == NULL) {
                outer_ub_elems
                  = TBmakeExprs (DUPdoDupNode (EXPRS_EXPR (partialshp_elems)), NULL);
            } else {
                outer_ub_elems
                  = TCappendExprs (outer_ub_elems,
                                   TBmakeExprs (EXPRS_EXPR (partialshp_elems), NULL));
            }

            dim++;
            partialshp_elems = EXPRS_NEXT (partialshp_elems);
        }

        /* withid */
        withid = TBmakeWithid (vec, ids);

        /* bound1 & bound2 */
        bound1 = TCcreateZeroVector (INFO_DIM (arg_info), T_int);
        bound2 = TCmakeIntVector (outer_ub_elems);

        /* generator */
        generator = TBmakeGenerator (F_wl_le, F_wl_lt, bound1, bound2, NULL, NULL);

        /* Set genwidth */
        GENERATOR_GENWIDTH (generator) = DUPdoDupNode (bound2);

        /* code */
        cexpr_type = AVIS_TYPE (INFO_CEXPR (arg_info));
        op = INFO_FOLDOP (arg_info);

        /* tmp1 = accu(iv); */
        accu_avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (cexpr_type));
        accu_ass
          = TBmakeAssign (TBmakeLet (TBmakeIds (accu_avis, NULL),
                                     TCmakePrf1 (F_accu, TBmakeId (IDS_AVIS (vec)))),
                          NULL);
        AVIS_SSAASSIGN (accu_avis) = accu_ass;
        INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), accu_avis);

        inner_size = SHgetUnrLen (TYgetShape (cexpr_type));

        for (i = 0; i < inner_size; i++) {
            /* idx = idxs2offset([...], i, j, k...); */
            args = TBmakeExprs (TCmakeIntVector (
                                  TCappendExprs (DUPdoDupTree (outer_ub_elems),
                                                 TBmakeExprs (TBmakeNum (inner_size),
                                                              NULL))),
                                TCappendExprs (TCids2Exprs (ids),
                                               TBmakeExprs (TBmakeNum (i), NULL)));

            offset_avis = CreatePrfOrConst (TRUE, "offset", T_int, SHmakeShape (0),
                                            F_idxs2offset, args, &assigns, fundef);

            /* elem1 = idx_sel( idx, array); */
            args
              = TBmakeExprs (TBmakeId (offset_avis),
                             TBmakeExprs (TBmakeId (INFO_PARTIALARR (arg_info)), NULL));

            elem_avis1
              = CreatePrfOrConst (TRUE, "elem",
                                  TYgetSimpleType (TYgetScalar (cexpr_type)),
                                  SHmakeShape (0), F_idx_sel, args, &assigns, fundef);

            /* elem2 = idx_sel( idx, array); */
            args = TBmakeExprs (TBmakeNum (i), TBmakeExprs (TBmakeId (accu_avis), NULL));

            elem_avis2
              = CreatePrfOrConst (TRUE, "elem",
                                  TYgetSimpleType (TYgetScalar (cexpr_type)),
                                  SHmakeShape (0), F_idx_sel, args, &assigns, fundef);

            /* opres = op( elem1, elem2); */
            args = TBmakeExprs (TBmakeId (elem_avis1),
                                TBmakeExprs (TBmakeId (elem_avis2), NULL));

            op_avis = CreatePrfOrConst (TRUE, "opres",
                                        TYgetSimpleType (TYgetScalar (cexpr_type)),
                                        SHmakeShape (0), op, args, &assigns, fundef);

            if (res_elems == NULL) {
                res_elems = TBmakeExprs (TBmakeId (op_avis), NULL);
            } else {
                res_elems
                  = TCappendExprs (res_elems, TBmakeExprs (TBmakeId (op_avis), NULL));
            }
        }

        op_avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (cexpr_type));
        op_ass
          = TBmakeAssign (TBmakeLet (TBmakeIds (op_avis, NULL),
                                     TBmakeArray (TYcopyType (TYgetScalar (cexpr_type)),
                                                  SHcopyShape (TYgetShape (cexpr_type)),
                                                  res_elems)),
                          NULL);
        AVIS_SSAASSIGN (op_avis) = op_ass;
        INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), op_avis);

        /* Update the new partition */
        code_instr = TCappendAssign (accu_ass, TCappendAssign (assigns, op_ass));
        new_code = TBmakeCode (TBmakeBlock (code_instr, NULL),
                               TBmakeExprs (TBmakeId (op_avis), NULL));
        CODE_USED (new_code) = 1;
        CODE_NEXT (new_code) = NULL;

        /* Create new partition */
        part = TBmakePart (new_code, withid, generator);

        /* fold withop */
        withop = TBmakeFold (INFO_FOLDFUNDEF (arg_info),
                             DUPdoDupNode (INFO_NEUTRAL (arg_info)));

        /* new fold withloop */
        new_foldwl = TBmakeWith (part, new_code, withop);
        WITH_PARTS (new_foldwl) = 1;
        WITH_REFERENCED (new_foldwl) = WITH_REFERENCED (old_foldwl);
        WITH_ISFOLDABLE (new_foldwl) = WITH_ISFOLDABLE (old_foldwl);
    } else if (INFO_RESDEF (arg_info) == def_withloop) {
        size_t outer_dim, inner_dim;
        node *partialshp_elems, *outer_ub_elems = NULL, *inner_ub_elems = NULL;
        node *outer_bound1, *inner_bound1, *outer_bound2, *inner_bound2;
        node *outer_withid, *inner_withid, *outer_generator, *inner_generator;
        node *new_genarraywl;
        node *outer_ids, *inner_ids;
        node *elem_avis1, *elem_ass1, *elem_avis2, *elem_ass2;
        node *offset_avis1, *offset_ass1, *offset_avis2, *offset_ass2;
        node *wlidx;

        inner_dim = INFO_INNERDIM (arg_info);
        outer_dim = INFO_DIM (arg_info) - INFO_INNERDIM (arg_info);

        partialshp_elems = DUPdoDupTree (ARRAY_AELEMS (INFO_PARTIALSHP (arg_info)));

        inner_ub_elems = TCgetNthExprs (outer_dim, partialshp_elems);
        EXPRS_NEXT (TCgetNthExprs (outer_dim - 1, partialshp_elems)) = NULL;
        outer_ub_elems = partialshp_elems;

        /************* Creating Outer Fold Withloop **************/

        /* iv */
        avis
          = TBmakeAvis (TRAVtmpVarName ("iv"), TYmakeAKS (TYmakeSimpleType (T_int),
                                                          SHcreateShape (1, outer_dim)));
        vec = TBmakeIds (avis, NULL);
        INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), avis);

        /* ids for outer withloop */
        dim = 0;
        outer_ids = NULL;
        while (dim < outer_dim) {
            avis = TBmakeAvis (TRAVtmpVarName ("ids"),
                               TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));

            if (outer_ids == NULL) {
                outer_ids = TBmakeIds (avis, NULL);
            } else {
                outer_ids = TCappendIds (outer_ids, TBmakeIds (avis, NULL));
            }
            INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), avis);
            dim++;
        }

        /* withid */
        outer_withid = TBmakeWithid (vec, outer_ids);

        /* bound1 & bound2  for outer withloop */
        outer_bound1 = SHshape2Array (SHmakeShape (outer_dim));
        outer_bound2 = TBmakeArray (TYmakeSimpleType (T_int),
                                    SHcreateShape (1, outer_dim), outer_ub_elems);

        /* generator */
        outer_generator
          = TBmakeGenerator (F_wl_le, F_wl_lt, outer_bound1, outer_bound2, NULL, NULL);

        /* Set genwidth */
        GENERATOR_GENWIDTH (outer_generator) = DUPdoDupNode (outer_bound2);

        /* code */
        inner_bound1 = SHshape2Array (SHmakeShape (inner_dim));
        inner_bound2 = TBmakeArray (TYmakeSimpleType (T_int),
                                    SHcreateShape (1, inner_dim), inner_ub_elems);

        cexpr_type
          = TYmakeAKS (TYcopyType (TYgetScalar (AVIS_TYPE (INFO_CEXPR (arg_info)))),
                       SHarray2Shape (inner_bound2));

        /* tmp1 = accu(iv); */
        accu_avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (cexpr_type));
        accu_ass
          = TBmakeAssign (TBmakeLet (TBmakeIds (accu_avis, NULL),
                                     TCmakePrf1 (F_accu, TBmakeId (IDS_AVIS (vec)))),
                          NULL);
        AVIS_SSAASSIGN (accu_avis) = accu_ass;
        INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), accu_avis);

        /************* Start of Creating Inner Genarray Withloop **************/

        op = INFO_FOLDOP (arg_info);

        /* iv */
        avis
          = TBmakeAvis (TRAVtmpVarName ("iv"), TYmakeAKS (TYmakeSimpleType (T_int),
                                                          SHcreateShape (1, inner_dim)));
        vec = TBmakeIds (avis, NULL);
        INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), avis);

        /* ids for inner withloop */
        dim = 0;
        inner_ids = NULL;
        while (dim < inner_dim) {
            avis = TBmakeAvis (TRAVtmpVarName ("ids"),
                               TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));

            if (inner_ids == NULL) {
                inner_ids = TBmakeIds (avis, NULL);
            } else {
                inner_ids = TCappendIds (inner_ids, TBmakeIds (avis, NULL));
            }
            INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), avis);
            dim++;
        }

        /* withid */
        inner_withid = TBmakeWithid (vec, inner_ids);

        /* generator */
        inner_generator
          = TBmakeGenerator (F_wl_le, F_wl_lt, inner_bound1, inner_bound2, NULL, NULL);

        /* Set genwidth */
        GENERATOR_GENWIDTH (inner_generator) = DUPdoDupNode (inner_bound2);

        /* code */
        cexpr_type = AVIS_TYPE (INFO_CEXPR (arg_info));

        /* offset = idxs2offset([...], i, j, k...); */
        offset_avis1 = TBmakeAvis (TRAVtmpVar (),
                                   TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
        offset_ass1
          = TBmakeAssign (TBmakeLet (TBmakeIds (offset_avis1, NULL),
                                     TBmakePrf (F_idxs2offset,
                                                TBmakeExprs (DUPdoDupNode (inner_bound2),
                                                             TCids2Exprs (inner_ids)))),
                          NULL);
        AVIS_SSAASSIGN (offset_avis1) = offset_ass1;
        INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), offset_avis1);

        /* elem = idx_sel( offset, array); */
        elem_avis1 = TBmakeAvis (TRAVtmpVar (), TYcopyType (cexpr_type));
        elem_ass1
          = TBmakeAssign (TBmakeLet (TBmakeIds (elem_avis1, NULL),
                                     TCmakePrf2 (F_idx_sel, TBmakeId (offset_avis1),
                                                 TBmakeId (accu_avis))),
                          NULL);
        AVIS_SSAASSIGN (elem_avis1) = elem_ass1;
        INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), elem_avis1);

        /* offset = idxs2offset([...], i, j, k...); */
        offset_avis2 = TBmakeAvis (TRAVtmpVar (),
                                   TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
        offset_ass2 = TBmakeAssign (
          TBmakeLet (TBmakeIds (offset_avis2, NULL),
                     TBmakePrf (F_idxs2offset,
                                TBmakeExprs (DUPdoDupNode (INFO_PARTIALSHP (arg_info)),
                                             TCids2Exprs (
                                               TCappendIds (DUPdoDupTree (outer_ids),
                                                            DUPdoDupTree (inner_ids)))))),
          NULL);
        AVIS_SSAASSIGN (offset_avis2) = offset_ass2;
        INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), offset_avis2);

        /* elem = idx_sel( offset, array); */
        elem_avis2 = TBmakeAvis (TRAVtmpVar (), TYcopyType (cexpr_type));
        elem_ass2
          = TBmakeAssign (TBmakeLet (TBmakeIds (elem_avis2, NULL),
                                     TCmakePrf2 (F_idx_sel, TBmakeId (offset_avis2),
                                                 TBmakeId (INFO_PARTIALARR (arg_info)))),
                          NULL);
        AVIS_SSAASSIGN (elem_avis2) = elem_ass2;
        INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), elem_avis2);

        /* res = op( arg_1, arg_2); */
        op_avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (cexpr_type));
        op_ass = TBmakeAssign (TBmakeLet (TBmakeIds (op_avis, NULL),
                                          TCmakePrf2 (op, TBmakeId (elem_avis1),
                                                      TBmakeId (elem_avis2))),
                               NULL);
        AVIS_SSAASSIGN (op_avis) = op_ass;
        INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), op_avis);

        /* Update the new partition */
        code_instr = TCappendAssign (
          offset_ass1,
          TCappendAssign (elem_ass1,
                          TCappendAssign (offset_ass2,
                                          TCappendAssign (elem_ass2,
                                                          TCappendAssign (op_ass,
                                                                          NULL)))));
        new_code = TBmakeCode (TBmakeBlock (code_instr, NULL),
                               TBmakeExprs (TBmakeId (op_avis), NULL));
        CODE_USED (new_code) = 1;
        CODE_NEXT (new_code) = NULL;

        /* Create new partition */
        part = TBmakePart (new_code, inner_withid, inner_generator);

        /* genarrau withop */
        wlidx = TBmakeAvis (TRAVtmpVarName ("wlidx"),
                            TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
        INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), wlidx);
        withop = TBmakeGenarray (DUPdoDupNode (inner_bound2), NULL);
        GENARRAY_IDX (withop) = wlidx;
        WITHID_IDXS (inner_withid) = TBmakeIds (wlidx, NULL);

        /* new genarray withloop */
        new_genarraywl = TBmakeWith (part, new_code, withop);
        WITH_PARTS (new_genarraywl) = 1;

        /************* End of Creating Inner Genarray Withloop **************/

        op_avis
          = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYcopyType (TYgetScalar (
                                                    AVIS_TYPE (INFO_CEXPR (arg_info)))),
                                                  SHarray2Shape (inner_bound2)));

        op_ass
          = TBmakeAssign (TBmakeLet (TBmakeIds (op_avis, NULL), new_genarraywl), NULL);
        AVIS_SSAASSIGN (op_avis) = op_ass;
        INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), op_avis);

        /* Update the new partition */
        code_instr = TCappendAssign (accu_ass, op_ass);
        new_code = TBmakeCode (TBmakeBlock (code_instr, NULL),
                               TBmakeExprs (TBmakeId (op_avis), NULL));
        CODE_USED (new_code) = 1;
        CODE_NEXT (new_code) = NULL;

        /* Create new partition */
        part = TBmakePart (new_code, outer_withid, outer_generator);

        /* fold withop */
        withop = TBmakeFold (INFO_FOLDFUNDEF (arg_info),
                             DUPdoDupNode (INFO_NEUTRAL (arg_info)));

        /* new fold withloop */
        new_foldwl = TBmakeWith (part, new_code, withop);
        WITH_PARTS (new_foldwl) = 1;
        WITH_REFERENCED (new_foldwl) = WITH_REFERENCED (old_foldwl);
        WITH_ISFOLDABLE (new_foldwl) = WITH_ISFOLDABLE (old_foldwl);
    } else {
        DBUG_UNREACHABLE ("Unknow result definition assignment!");
    }

    /* Cleanup arg_info */
    INFO_PARTIALARR (arg_info) = NULL;
    INFO_FOLDFUNDEF (arg_info) = NULL;
    INFO_FOLDOP (arg_info) = F_unknown;
    INFO_NEUTRAL (arg_info) = NULL;
    INFO_CEXPR (arg_info) = NULL;
    INFO_DIM (arg_info) = 0;

    DBUG_RETURN (new_foldwl);
}

/********************************************************************************************************************/

node *
ATravWithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_AT_VECASSIGNS (arg_info)
      = TBmakeAssign (TBmakeLet (TBmakeIds (IDS_AVIS (WITHID_VEC (arg_node)), NULL),
                                 TBmakeArray (TYmakeSimpleType (T_int),
                                              SHcreateShape (1, TCcountIds (
                                                                  WITHID_IDS (arg_node))),
                                              TCconvertIds2Exprs (
                                                WITHID_IDS (arg_node)))),
                      INFO_AT_VECASSIGNS (arg_info));

    AVIS_SSAASSIGN (IDS_AVIS (WITHID_VEC (arg_node))) = INFO_AT_VECASSIGNS (arg_info);

    /* New WITHID_IDS is a concatenation of the ids from outer
     * wl and those from inner wl */
    WITHID_IDS (arg_node) = TCappendIds (WITHID_IDS (arg_node),
                                         DUPdoDupTree (INFO_AT_INNERWITHIDS (arg_info)));

    IDS_AVIS (WITHID_VEC (arg_node))
      = TBmakeAvis (TRAVtmpVarName ("iv"),
                    TYmakeAKS (TYmakeSimpleType (T_int),
                               SHcreateShape (1, TCcountIds (WITHID_IDS (arg_node)))));

    FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
      = TBmakeVardec (IDS_AVIS (WITHID_VEC (arg_node)),
                      FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

    DBUG_RETURN (arg_node);
}

node *
ATravGenerator (node *arg_node, info *arg_info)
{
    node *outer_b1, *outer_b2, *inner_b1, *inner_b2;
    node *outer_genwidth, *inner_genwidth;

    DBUG_ENTER ();

    outer_b1 = GENERATOR_BOUND1 (arg_node);
    outer_b2 = GENERATOR_BOUND2 (arg_node);
    outer_genwidth = GENERATOR_GENWIDTH (arg_node);
    inner_b1 = INFO_AT_INNERWITHBOUND1 (arg_info);
    inner_b2 = INFO_AT_INNERWITHBOUND2 (arg_info);
    inner_genwidth = INFO_AT_INNERWITHGENWIDTH (arg_info);

    DBUG_ASSERT (COisConstant (outer_b1), "Outer bound1 is not constant!");
    DBUG_ASSERT (COisConstant (outer_b2), "Outer bound2 is not constant!");
    DBUG_ASSERT (COisConstant (outer_genwidth), "Outer genwidth is not constant!");
    DBUG_ASSERT (COisConstant (inner_b1), "Inner bound1 is not constant!");
    DBUG_ASSERT (COisConstant (inner_b2), "Inner bound2 is not constant!");
    DBUG_ASSERT (COisConstant (inner_genwidth), "Inner genwidth is not constant!");

    GENERATOR_BOUND1 (arg_node) = COconstant2AST (
      COcat (COaST2Constant (outer_b1), COaST2Constant (inner_b1), NULL));

    GENERATOR_BOUND2 (arg_node) = COconstant2AST (
      COcat (COaST2Constant (outer_b2), COaST2Constant (inner_b2), NULL));

    GENERATOR_GENWIDTH (arg_node) = COconstant2AST (
      COcat (COaST2Constant (outer_genwidth), COaST2Constant (inner_genwidth), NULL));

    DBUG_RETURN (arg_node);
}

node *
ATravCode (node *arg_node, info *arg_info)
{
    node *assigns, *prev, *next;

    DBUG_ENTER ();

    assigns = CODE_CBLOCK_ASSIGNS (arg_node);

    DBUG_ASSERT (assigns != NULL, "Fold withloop body is empty!");

    prev = assigns;
    next = prev;

    while (next != NULL && next != INFO_AT_INNERWITHASSIGN (arg_info)) {
        prev = next;
        next = ASSIGN_NEXT (next);
    }

    DBUG_ASSERT (next != NULL, "Inner withloop is not found!");

    ASSIGN_NEXT (prev) = INFO_AT_INNERWITHCODE (arg_info);

    CODE_CBLOCK_ASSIGNS (arg_node)
      = TCappendAssign (INFO_AT_VECASSIGNS (arg_info), assigns);

    DBUG_RETURN (arg_node);
}

node *
ATravPart (node *arg_node, info *arg_info)
{
    node *cexpr, *ssa_assign, *defining_rhs;
    size_t cat_dim;

    DBUG_ENTER ();

    /* All fold withloops are assumed to have only one partition */
    DBUG_ASSERT (PART_NEXT (arg_node) == NULL,
                 "Found fold withloop with more than one partition!");

    cexpr = EXPRS_EXPR (PART_CEXPRS (arg_node));

    /* This is the defining assignment of the result elment of
     * the outer fold withloop */
    ssa_assign = AVIS_SSAASSIGN (ID_AVIS (cexpr));
    INFO_AT_INNERWITHASSIGN (arg_info) = ssa_assign;
    defining_rhs = ASSIGN_RHS (ssa_assign);

    /* The sum of the dimensionality of the outer fold withloop and the inner
     * genarray/modarray withloop */
    cat_dim = TCcountIds (PART_IDS (arg_node)) + TYgetDim (AVIS_TYPE (ID_AVIS (cexpr)));

    /* After scalarizing, we expect the outer fold withloop
     * to be at most 3D */
    if (!TUisScalar (ID_NTYPE (cexpr)) && cat_dim <= 3 && cat_dim >= 1
        && /* The concatenated dimension is 1 <= dim <= 3 */
        NODE_TYPE (ASSIGN_STMT (ssa_assign)) == N_let) {
        if (NODE_TYPE (defining_rhs) == N_with
            && PART_NEXT (WITH_PART (defining_rhs)) == NULL
            && /* The defining withloop must have only one partition */
            (NODE_TYPE (WITH_WITHOP (defining_rhs)) == N_genarray
             || NODE_TYPE (WITH_WITHOP (defining_rhs)) == N_modarray)) {
            /* All conditions are met, we can scalarize this withloop */
            CHANGED = 1;

            INFO_AT_INNERDIM (arg_info) = TYgetDim (ID_NTYPE (cexpr));

            /* Create assignments of the form: iv = [x,y,z...]; */
            INFO_AT_VECASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (IDS_AVIS (WITH_VEC (defining_rhs)),
                                                    NULL),
                                         TBmakeArray (TYmakeSimpleType (T_int),
                                                      SHcreateShape (1,
                                                                     TCcountIds (
                                                                       WITH_IDS (
                                                                         defining_rhs))),
                                                      TCconvertIds2Exprs (
                                                        WITH_IDS (defining_rhs)))),
                              INFO_AT_VECASSIGNS (arg_info));

            AVIS_SSAASSIGN (IDS_AVIS (WITH_VEC (defining_rhs)))
              = INFO_AT_VECASSIGNS (arg_info);

            INFO_AT_INNERWITHIDS (arg_info) = WITH_IDS (defining_rhs);
            INFO_AT_INNERWITHVEC (arg_info) = WITH_VEC (defining_rhs);
            INFO_AT_INNERWITHBOUND1 (arg_info) = WITH_BOUND1 (defining_rhs);
            INFO_AT_INNERWITHBOUND2 (arg_info) = WITH_BOUND2 (defining_rhs);
            INFO_AT_INNERWITHGENWIDTH (arg_info) = WITH_GENWIDTH (defining_rhs);
            INFO_AT_INNERWITHCODE (arg_info) = BLOCK_ASSIGNS (WITH_CBLOCK (defining_rhs));

            PART_WITHID (arg_node) = TRAVopt (PART_WITHID (arg_node), arg_info);
            PART_GENERATOR (arg_node) = TRAVopt (PART_GENERATOR (arg_node), arg_info);
            PART_CODE (arg_node) = TRAVopt (PART_CODE (arg_node), arg_info);

            /* Set the cexpr to be the cexpr of the original inner withloop */
            ID_AVIS (EXPRS_EXPR (PART_CEXPRS (arg_node)))
              = ID_AVIS (WITH_CEXPR (defining_rhs));

            if (cat_dim == 2) {
                PART_THREADBLOCKSHAPE (arg_node)
                  = FREEdoFreeNode (PART_THREADBLOCKSHAPE (arg_node));

                /* Now we need a 2D thread block */
                PART_THREADBLOCKSHAPE (arg_node)
                  = TBmakeArray (TYmakeSimpleType (T_int),
                                 SHcreateShape (1, TCcountIds (PART_IDS (arg_node))),
                                 TBmakeExprs (TBmakeNum (
                                                global.config.cuda_2d_block_y),
                                              TBmakeExprs (TBmakeNum (global.config.cuda_2d_block_x),
                                                           NULL)));
            }
            INFO_AT_RESDEF (arg_info) = def_withloop;
        } else if (NODE_TYPE (defining_rhs) == N_array) {
            INFO_AT_INNERDIM (arg_info) = TYgetDim (ID_NTYPE (cexpr));
            INFO_AT_ARRAYELEMS (arg_info) = ARRAY_AELEMS (defining_rhs);
            INFO_AT_RESDEF (arg_info) = def_array;
        }
    }

    DBUG_RETURN (arg_node);
}

/********************************************************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *PFDwith( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
PFDwith (node *arg_node, info *arg_info)
{
    node *avis, *new_avis;
    simpletype sty;
    node *new_fold;
    info *anon_info;

    DBUG_ENTER ();

    /************ Anonymous Traversal ************/
    anontrav_t atrav[5] = {{N_part, &ATravPart},
                           {N_code, &ATravCode},
                           {N_generator, &ATravGenerator},
                           {N_withid, &ATravWithid},
                           {(nodetype)0, NULL}};

    /* Here we traverse the fold withloop and try to scalarize it until no
     * more scalarization can be done, i.e. CHANGED equals to 0. The reason we need to
     * do it iteratively is because the inner
     * withloop defining the result can itself contain
     * withloops so we need to scalarized those withloops
     * as well after we have scalarized the outer withloop.
     * However, there is a problem here: The scalarization
     * of outer withloop is committed before we traverse
     * into any inner withloops and if we later found that
     * those inner withloops cannot be scalarized then it
     * will impossible to undo the previous scalarization.
     * This leaves an intermediate withloop which cannot
     * be recognized by other compile phases. So the solution
     * is to perform a thorough check first to confirm that
     * the withloop can indeed be scalarized before we actually
     * transform it. But this has not been implemented yet.
     *
     * After this anonymous traversal, onlt one of the following condtions is true:
     *   1) The fold withloop has not been changed at all as its result is
     *      already a scalar.
     *   2) The fold withloop has not been changed at all as its result is
     *      defined by an array.
     *   3) The fold withloop has been scalarized and the new reuslt is now
     *      either a scalar or an array.
     * The fold withloop may potentially been re-tagged as not cudarizable after the
     * check.
     *
     * TODO:
     *   This anonymous traversal to check for legality of scalarization
     *   and actually perform the scalarization is NOT robust. A much
     *   better approach would be to have separate compiler passes to
     *   perform the the actual check and transformation similar to those
     *   found in the withloop scalarization. This means that we can reuse
     *   some part of the implementation of the withloop scalarization. And
     *   this should replace this anonymous traversal here.
     */

    if (WITH_CUDARIZABLE (arg_node) && NODE_TYPE (WITH_WITHOP (arg_node)) == N_fold) {
        if (CHANGED == 1) {
            CHANGED = 0;

            TRAVpushAnonymous (atrav, &TRAVsons);

            anon_info = MakeInfo ();
            INFO_LHS (anon_info) = INFO_LHS (arg_info);
            INFO_FUNDEF (anon_info) = INFO_FUNDEF (arg_info);

            WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), anon_info);

            /* Dimension of the inner result */
            INFO_INNERDIM (arg_info) += INFO_AT_INNERDIM (anon_info);
            /* However the result of this withloop is defined? def_scalar,
             * def_array or def_withloop? */
            INFO_RESDEF (arg_info) = INFO_AT_RESDEF (anon_info);
            /* In case that the result of the withloop is defined by an array,
             * here are the individual array elements */
            INFO_ARRAYELEMS (arg_info) = INFO_AT_ARRAYELEMS (anon_info);

            anon_info = FreeInfo (anon_info);
            TRAVpop ();
        }
    }
    /*********************************************/

    CHANGED = 1;

    if (WITH_CUDARIZABLE (arg_node) && NODE_TYPE (WITH_WITHOP (arg_node)) == N_fold) {

        DBUG_ASSERT (FOLD_ISPARTIALFOLD (WITH_WITHOP (arg_node)),
                     "CUDA fold withloop must be partial folding!");

        /*
         * The withop has to be traversed before part. Because we need the
         * neutral element during the code traversal (i.e. to replace the
         * rhs of F_accu with the neutral element). Traversal of this withop
         * will remember both the folding function and the neutral element
         * in INFO_FOLDFUN and INFO_NEUTRAL respectively.
         * The traversal of the partition will build all the instructions for:
         *
         *   1) Load data into shared memory.
         *   2) Perform reduction with data in the shared memory.
         *   3) Store reduction result in shared memory back to global memory.
         */
        INFO_LEVEL (arg_info)++;
        WITH_WITHOP (arg_node) = TRAVopt (WITH_WITHOP (arg_node), arg_info);
        WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
        INFO_LEVEL (arg_info)--;

        /******* Create a new variable (new_avis) for the partial folding result ******/
        avis = IDS_AVIS (INFO_LHS (arg_info));
        new_avis = DUPdoDupNode (avis);
        AVIS_SSAASSIGN (new_avis) = INFO_LASTASSIGN (arg_info);
        AVIS_NAME (new_avis) = MEMfree (AVIS_NAME (new_avis));
        AVIS_NAME (new_avis) = TRAVtmpVar ();
        sty = TYgetSimpleType (TYgetScalar (AVIS_TYPE (new_avis)));
        AVIS_TYPE (new_avis) = TYfreeType (AVIS_TYPE (new_avis));

        /* Create a new type with the partial shape */
        AVIS_TYPE (new_avis) = TYmakeAKS (TYmakeSimpleType (sty),
                                          SHarray2Shape (INFO_PARTIALSHP (arg_info)));

        FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
          = TCappendVardec (FUNDEF_VARDECS (INFO_FUNDEF (arg_info)),
                            TBmakeVardec (new_avis, NULL));

        IDS_AVIS (INFO_LHS (arg_info)) = new_avis;
        INFO_PARTIALARR (arg_info) = new_avis;

        /*******************************************************************/

        /*
         * Build a new fold withloop which will be executed on the host and folds
         * the partial array generated from the previous CUDA fold withloop
         */
        new_fold = BuildFoldWithloop (arg_node, arg_info);

        INFO_PARTIALSHP (arg_info) = FREEdoFreeNode (INFO_PARTIALSHP (arg_info));

        INFO_POSTASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), new_fold), NULL);
        AVIS_SSAASSIGN (avis) = INFO_POSTASSIGN (arg_info);
    }

    INFO_INNERDIM (arg_info) = 0;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PFDfold( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
PFDfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* Remember the folding function and its neutral element */
    INFO_FOLDFUNDEF (arg_info) = FOLD_FUNDEF (arg_node);
    INFO_NEUTRAL (arg_info) = FOLD_NEUTRAL (arg_node);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PFDprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
PFDprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_LEVEL (arg_info) == 1) {
        if (PRF_PRF (arg_node) == F_accu) {
            /*
             * Replace the F_accu assignment of the outermost fold withloop by an
             * assignment of the neutral element.
             */
            arg_node = FREEdoFreeNode (arg_node);
            arg_node = DUPdoDupNode (INFO_NEUTRAL (arg_info));
        }
    } else {
        PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
