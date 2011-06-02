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
#include "dbug.h"
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
#include "create_lac_fun.h"

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

typedef enum { y_reduction, x_reduction } reduction_kind;

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
    int dim;
    int innerdim;
    node *fundef;
    node *lastassign;
    node *partialarr;
    node *postassign;
    node *foldfundef;
    node *neutral;
    prf foldop;
    node *cexpr;
    cuidx_set_t *cis;
    node *shrarray;
    node *lacfuns;

    node *at_ids;
    node *at_vec;
    node *at_bound1;
    node *at_bound2;
    node *at_genwidth;
    node *at_code;
    node *at_assign;
    node *at_vecassigns;
    int at_innerdim;
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
#define INFO_SHRARRAY(n) (n->shrarray)
#define INFO_LACFUNS(n) (n->lacfuns)

#define INFO_AT_INNERWITHIDS(n) (n->at_ids)
#define INFO_AT_INNERWITHVEC(n) (n->at_vec)
#define INFO_AT_INNERWITHBOUND1(n) (n->at_bound1)
#define INFO_AT_INNERWITHBOUND2(n) (n->at_bound2)
#define INFO_AT_INNERWITHGENWIDTH(n) (n->at_genwidth)
#define INFO_AT_INNERWITHCODE(n) (n->at_code)
#define INFO_AT_INNERWITHASSIGN(n) (n->at_assign)
#define INFO_AT_VECASSIGNS(n) (n->at_vecassigns)
#define INFO_AT_INNERDIM(n) (n->at_innerdim)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

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
    INFO_SHRARRAY (result) = NULL;
    INFO_LACFUNS (result) = NULL;

    INFO_AT_INNERWITHIDS (result) = NULL;
    INFO_AT_INNERWITHVEC (result) = NULL;
    INFO_AT_INNERWITHBOUND1 (result) = NULL;
    INFO_AT_INNERWITHBOUND2 (result) = NULL;
    INFO_AT_INNERWITHGENWIDTH (result) = NULL;
    INFO_AT_INNERWITHCODE (result) = NULL;
    INFO_AT_INNERWITHASSIGN (result) = NULL;
    INFO_AT_VECASSIGNS (result) = NULL;
    INFO_AT_INNERDIM (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

static node *
AppendVardec (node *fundef, node *avis)
{
    DBUG_ENTER ("AppendVardec");

    FUNDEF_VARDEC (fundef)
      = TCappendVardec (FUNDEF_VARDEC (fundef), TBmakeVardec (avis, NULL));

    DBUG_RETURN (fundef);
}

static node *
CreatePrfOrConst (bool isprf, char *name, simpletype sty, shape *shp, prf pfun,
                  node *args, node **vardecs_p, node **assigns_p)
{
    node *avis = NULL, *new_assign;

    DBUG_ENTER ("CreatePrfOrConst");

    if (name != NULL) {
        avis
          = TBmakeAvis (TRAVtmpVarName (name), TYmakeAKS (TYmakeSimpleType (sty), shp));

        *vardecs_p = TBmakeVardec (avis, *vardecs_p);
    }

    new_assign = TBmakeAssign (TBmakeLet ((avis == NULL) ? avis : TBmakeIds (avis, NULL),
                                          (isprf) ? TBmakePrf (pfun, args) : args),
                               NULL);

    if (avis != NULL) {
        AVIS_SSAASSIGN (avis) = new_assign;
    }

    if (&assigns_p == NULL) {
        *assigns_p = new_assign;
    } else {
        *assigns_p = TCappendAssign (*assigns_p, new_assign);
    }

    DBUG_RETURN (avis);
}

static node *
CreateCudaIndexInitCode (node *part, info *arg_info)
{
    int dim;
    cuidx_set_t *cis;
    node *assigns = NULL, *vardecs = NULL;

    DBUG_ENTER ("CreateCudaIndexInitCode");

    cis = MEMmalloc (sizeof (cuidx_set_t));
    dim = TCcountIds (PART_IDS (part));

    CIS_TX (cis) = CreatePrfOrConst (TRUE, "tx", T_int, SHmakeShape (0),
                                     F_cuda_threadIdx_x, NULL, &vardecs, &assigns);

    CIS_BX (cis) = CreatePrfOrConst (TRUE, "bx", T_int, SHmakeShape (0),
                                     F_cuda_blockIdx_x, NULL, &vardecs, &assigns);

    if (dim == 2) {
        CIS_TY (cis) = CreatePrfOrConst (TRUE, "ty", T_int, SHmakeShape (0),
                                         F_cuda_threadIdx_y, NULL, &vardecs, &assigns);

        CIS_BY (cis) = CreatePrfOrConst (TRUE, "by", T_int, SHmakeShape (0),
                                         F_cuda_blockIdx_y, NULL, &vardecs, &assigns);
    }

    FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
      = TCappendVardec (FUNDEF_VARDEC (INFO_FUNDEF (arg_info)), vardecs);

    INFO_CIS (arg_info) = cis;

    DBUG_RETURN (assigns);
}

static node *
BuildLoadAssigns (node *part, info *arg_info)
{
    node *assigns = NULL, *vardecs = NULL;
    node *avis, *args;
    node *shmem;

    DBUG_ENTER ("BuildLoadAssigns");

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
        DBUG_ASSERT ((0), "Only 1D and 2D fold withloop is supported!");
    }

    /* Create  i.e. idx_modarray_AxSxS */
    avis = CreatePrfOrConst (TRUE, "idx_shr", T_int, SHmakeShape (0), F_idxs2offset, args,
                             &vardecs, &assigns);

    /* Create shared memory */
    shmem = TBmakeAvis (TRAVtmpVarName ("shmem"),
                        TYmakeAKS (TYmakeSimpleType (
                                     CUh2shSimpleTypeConversion (TYgetSimpleType (
                                       TYgetScalar (AVIS_TYPE (INFO_CEXPR (arg_info)))))),
                                   SHarray2Shape (PART_THREADBLOCKSHAPE (part))));

    FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
      = TBmakeVardec (shmem, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

    args
      = TBmakeExprs (TBmakeId (shmem),
                     TBmakeExprs (TBmakeId (avis),
                                  TBmakeExprs (TBmakeId (INFO_CEXPR (arg_info)), NULL)));
    /* idx_modarray */
    avis = CreatePrfOrConst (TRUE, "shmem",
                             TYgetSimpleType (TYgetScalar (AVIS_TYPE (shmem))),
                             SHarray2Shape (PART_THREADBLOCKSHAPE (part)),
                             F_idx_modarray_AxSxS, args, &vardecs, &assigns);

    args = TBmakeExprs (TBmakeId (avis), NULL);

    /* syncthreads */
    avis = CreatePrfOrConst (TRUE, "shmem",
                             TYgetSimpleType (TYgetScalar (AVIS_TYPE (shmem))),
                             SHarray2Shape (PART_THREADBLOCKSHAPE (part)), F_syncthreads,
                             args, &vardecs, &assigns);

    INFO_SHRARRAY (arg_info) = avis;

    FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
      = TCappendVardec (FUNDEF_VARDEC (INFO_FUNDEF (arg_info)), vardecs);

    DBUG_RETURN (assigns);
}

static node *
BuildReduceAssignsInternal (reduction_kind kind, int partshp, int partialshp,
                            int shmemshp, int remain, node *part, info *arg_info)
{
    node *assigns = NULL, *vardecs = NULL, *args, *avis;
    simpletype sty;

    node *loop_assigns = NULL, *predicate = NULL, *iterator;

    node *loop_bound, *cond;
    node *idx1, *idx2, *val1, *val2, *val3;
    node *in_shared_array, *out_shared_array;
    node *cond_predicate;
    node *tx_zero, *ty_zero;

    DBUG_ENTER ("BuildReduceAssignsInternal");

    sty = TYgetSimpleType (TYgetScalar (AVIS_TYPE (INFO_SHRARRAY (arg_info))));

    in_shared_array = INFO_SHRARRAY (arg_info);
    /* i = 1; */
    iterator = CreatePrfOrConst (FALSE, "iterator", T_int, SHmakeShape (0), F_add_SxS,
                                 TBmakeNum (1), &vardecs, &assigns);

    /* Test if the length of the Y dimension is evenly divisible by
     * the lenght of the Y dimension shared memory. If not, we need
     * to generate the loop bound in way so that the last thread block
     * does not access elements off boundary in the shared memory.
     * If yes, then the loop bound is simply the lengh of the Y dimesnion
     * of the shared memory.
     */
    if (remain != 0) {
        node *bound1, *bound2, *bound;
        bound1 = CreatePrfOrConst (FALSE, "loop_bound", T_int, SHmakeShape (0), F_unknown,
                                   TBmakeNum (shmemshp), &vardecs, &assigns);

        bound2 = CreatePrfOrConst (FALSE, "loop_bound", T_int, SHmakeShape (0), F_unknown,
                                   TBmakeNum (remain), &vardecs, &assigns);

        bound = CreatePrfOrConst (FALSE, "bound", T_int, SHmakeShape (0), F_unknown,
                                  TBmakeNum (partialshp - 1), &vardecs, &assigns);

        if (kind == y_reduction) {
            args = TBmakeExprs (TBmakeId (CIS_BY (INFO_CIS (arg_info))),
                                TBmakeExprs (TBmakeId (bound), NULL));
        } else if (kind == x_reduction) {
            args = TBmakeExprs (TBmakeId (CIS_BX (INFO_CIS (arg_info))),
                                TBmakeExprs (TBmakeId (bound), NULL));
        } else {
            DBUG_ASSERT ((0), "Reduction in unknown dimension!");
        }

        cond = CreatePrfOrConst (TRUE, "cond1", T_bool, SHmakeShape (0), F_eq_SxS, args,
                                 &vardecs, &assigns);

        args = TBmakeExprs (TBmakeId (cond),
                            TBmakeExprs (TBmakeId (bound2),
                                         TBmakeExprs (TBmakeId (bound1), NULL)));

        loop_bound = CreatePrfOrConst (TRUE, "loop_bound", T_int, SHmakeShape (0), F_cond,
                                       args, &vardecs, &assigns);
    } else {
        loop_bound
          = CreatePrfOrConst (FALSE, "loop_bound", T_int, SHmakeShape (0), F_unknown,
                              TBmakeNum (shmemshp), &vardecs, &assigns);
    }

    /* Body of the loop. The loop is executed by all threads in a block with
     * ty == 0. Each thread executes a loop to reduce all elements from [0][tx]
     * to [loop_bound][tx] into [0][tx]. This is the firsdt step of folding
     * for a two dimension array. */
    avis = CreatePrfOrConst (FALSE, "const", T_int, SHmakeShape (0), F_unknown,
                             TBmakeNum (0), &vardecs, &loop_assigns);

    if (kind == y_reduction) {
        args = TBmakeExprs (DUPdoDupNode (PART_THREADBLOCKSHAPE (part)),
                            TBmakeExprs (TBmakeId (avis),
                                         TBmakeExprs (TBmakeId (
                                                        CIS_TX (INFO_CIS (arg_info))),
                                                      NULL)));
    } else if (kind == x_reduction) {
        if (INFO_DIM (arg_info) == 1) {
            args = TBmakeExprs (DUPdoDupNode (PART_THREADBLOCKSHAPE (part)),
                                TBmakeExprs (TBmakeId (avis), NULL));
        } else if (INFO_DIM (arg_info) == 2) {
            args = TBmakeExprs (DUPdoDupNode (PART_THREADBLOCKSHAPE (part)),
                                TBmakeExprs (TBmakeId (avis),
                                             TBmakeExprs (TBmakeId (avis), NULL)));
        } else {
        }
    }

    idx1 = CreatePrfOrConst (TRUE, "idx", T_int, SHmakeShape (0), F_idxs2offset, args,
                             &vardecs, &loop_assigns);

    args = TBmakeExprs (TBmakeId (idx1),
                        TBmakeExprs (TBmakeId (INFO_SHRARRAY (arg_info)), NULL));

    val1 = CreatePrfOrConst (TRUE, "val", sty, SHmakeShape (0), F_idx_sel, args, &vardecs,
                             &loop_assigns);

    if (kind == y_reduction) {
        args = TBmakeExprs (DUPdoDupNode (PART_THREADBLOCKSHAPE (part)),
                            TBmakeExprs (TBmakeId (iterator),
                                         TBmakeExprs (TBmakeId (
                                                        CIS_TX (INFO_CIS (arg_info))),
                                                      NULL)));

    } else if (kind == x_reduction) {
        if (INFO_DIM (arg_info) == 1) {
            args = TBmakeExprs (DUPdoDupNode (PART_THREADBLOCKSHAPE (part)),
                                TBmakeExprs (TBmakeId (iterator), NULL));
        } else if (INFO_DIM (arg_info) == 2) {
            args = TBmakeExprs (DUPdoDupNode (PART_THREADBLOCKSHAPE (part)),
                                TBmakeExprs (TBmakeId (avis),
                                             TBmakeExprs (TBmakeId (iterator), NULL)));
        } else {
        }
    }

    idx2 = CreatePrfOrConst (TRUE, "idx", T_int, SHmakeShape (0), F_idxs2offset, args,
                             &vardecs, &loop_assigns);

    args = TBmakeExprs (TBmakeId (idx2),
                        TBmakeExprs (TBmakeId (INFO_SHRARRAY (arg_info)), NULL));

    val2 = CreatePrfOrConst (TRUE, "val", sty, SHmakeShape (0), F_idx_sel, args, &vardecs,
                             &loop_assigns);

    args = TBmakeExprs (TBmakeId (val1), TBmakeExprs (TBmakeId (val2), NULL));

    val3 = CreatePrfOrConst (TRUE, "val", sty, SHmakeShape (0), INFO_FOLDOP (arg_info),
                             args, &vardecs, &loop_assigns);

    args
      = TBmakeExprs (TBmakeId (INFO_SHRARRAY (arg_info)),
                     TBmakeExprs (TBmakeId (idx1), TBmakeExprs (TBmakeId (val3), NULL)));

    INFO_SHRARRAY (arg_info)
      = CreatePrfOrConst (TRUE, "shmem", sty,
                          SHarray2Shape (PART_THREADBLOCKSHAPE (part)),
                          F_idx_modarray_AxSxS, args, &vardecs, &loop_assigns);

    /* This is actually the condition to allow only threads with ty == 0
     * the execute the reduction loop. We put it here because the new
     * variables related to the condition need to be added to the function
     * vardec list before CLACdoCreateLacFun for the conditional can be called.
     */
    if (kind == y_reduction) {
        args = TBmakeExprs (TBmakeId (CIS_TY (INFO_CIS (arg_info))),
                            TBmakeExprs (TBmakeNum (0), NULL));

        cond_predicate = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0),
                                           F_eq_SxS, args, &vardecs, &predicate);
    } else if (kind == x_reduction) {
        if (INFO_DIM (arg_info) == 1) {
            args = TBmakeExprs (TBmakeId (CIS_TX (INFO_CIS (arg_info))),
                                TBmakeExprs (TBmakeNum (0), NULL));

            cond_predicate = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0),
                                               F_eq_SxS, args, &vardecs, &predicate);
        } else if (INFO_DIM (arg_info) == 2) {
            args = TBmakeExprs (TBmakeId (CIS_TY (INFO_CIS (arg_info))),
                                TBmakeExprs (TBmakeNum (0), NULL));

            ty_zero = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0), F_eq_SxS,
                                        args, &vardecs, &predicate);

            args = TBmakeExprs (TBmakeId (CIS_TX (INFO_CIS (arg_info))),
                                TBmakeExprs (TBmakeNum (0), NULL));

            tx_zero = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0), F_eq_SxS,
                                        args, &vardecs, &predicate);

            args
              = TBmakeExprs (TBmakeId (tx_zero), TBmakeExprs (TBmakeId (ty_zero), NULL));
            cond_predicate = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0),
                                               F_and_SxS, args, &vardecs, &predicate);
        } else {
        }
    }

    FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
      = TCappendVardec (FUNDEF_VARDEC (INFO_FUNDEF (arg_info)), vardecs);

    vardecs = NULL;

    /* After this, loop_assigns is the function application of a loop function */
    loop_assigns
      = CLACFdoCreateLacFun (FALSE, INFO_FUNDEF (arg_info), loop_assigns, NULL, iterator,
                             loop_bound, in_shared_array, INFO_SHRARRAY (arg_info),
                             &INFO_LACFUNS (arg_info));

    /* This is the final assignment chain used to create the conditional function */
    assigns = TCappendAssign (assigns, loop_assigns);
    loop_assigns = NULL;

    /* Here we create the conditional function. Its body is contained in "assigns" */
    out_shared_array = INFO_SHRARRAY (arg_info);

    /* After this, assigns is the function application of a conditional function */
    assigns = CLACFdoCreateLacFun (TRUE, INFO_FUNDEF (arg_info), assigns, cond_predicate,
                                   NULL, NULL, in_shared_array, out_shared_array,
                                   &INFO_LACFUNS (arg_info));

    /* Join the condition testing and the conditional function together */
    assigns = TCappendAssign (predicate, assigns);

    args = TBmakeExprs (TBmakeId (INFO_SHRARRAY (arg_info)), NULL);

    /* Synchronising the threads in the block after the conditional */
    avis = CreatePrfOrConst (TRUE, "shmem", sty,
                             SHarray2Shape (PART_THREADBLOCKSHAPE (part)), F_syncthreads,
                             args, &vardecs, &assigns);

    INFO_SHRARRAY (arg_info) = avis;

    FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
      = TCappendVardec (FUNDEF_VARDEC (INFO_FUNDEF (arg_info)), vardecs);

    DBUG_RETURN (assigns);
}

static node *
BuildReduceAssigns (node *part, info *arg_info)
{
    int partshp_x, partshp_y, partialshp_y, partialshp_x, shmemshp_x, shmemshp_y,
      remain_x, remain_y;

    node *assigns, *y_reduction_assigns = NULL, *x_reduction_assigns = NULL;

    DBUG_ENTER ("BuildReduceAssigns");

    if (INFO_DIM (arg_info) == 1) {
        partshp_x = NUM_VAL (EXPRS_EXPR1 (ARRAY_AELEMS (INFO_PARTSHP (arg_info))));
        partialshp_x = NUM_VAL (EXPRS_EXPR1 (ARRAY_AELEMS (INFO_PARTIALSHP (arg_info))));
        shmemshp_x = NUM_VAL (EXPRS_EXPR1 (ARRAY_AELEMS (PART_THREADBLOCKSHAPE (part))));

        remain_x = partshp_x % shmemshp_x;

        assigns = BuildReduceAssignsInternal (x_reduction, partshp_x, partialshp_x,
                                              shmemshp_x, remain_x, part, arg_info);
    } else if (INFO_DIM (arg_info) == 2) {

        partshp_y = NUM_VAL (EXPRS_EXPR1 (ARRAY_AELEMS (INFO_PARTSHP (arg_info))));
        partshp_x = NUM_VAL (EXPRS_EXPR2 (ARRAY_AELEMS (INFO_PARTSHP (arg_info))));
        partialshp_y = NUM_VAL (EXPRS_EXPR1 (ARRAY_AELEMS (INFO_PARTIALSHP (arg_info))));
        partialshp_x = NUM_VAL (EXPRS_EXPR2 (ARRAY_AELEMS (INFO_PARTIALSHP (arg_info))));
        shmemshp_y = NUM_VAL (EXPRS_EXPR1 (ARRAY_AELEMS (PART_THREADBLOCKSHAPE (part))));
        shmemshp_x = NUM_VAL (EXPRS_EXPR2 (ARRAY_AELEMS (PART_THREADBLOCKSHAPE (part))));

        remain_y = partshp_y % shmemshp_y;
        remain_x = partshp_x % shmemshp_x;

        y_reduction_assigns
          = BuildReduceAssignsInternal (y_reduction, partshp_y, partialshp_y, shmemshp_y,
                                        remain_y, part, arg_info);

        if (INFO_INNERDIM (arg_info) == 0) {
            x_reduction_assigns
              = BuildReduceAssignsInternal (x_reduction, partshp_x, partialshp_x,
                                            shmemshp_x, remain_x, part, arg_info);
        }

        assigns = TCappendAssign (y_reduction_assigns, x_reduction_assigns);
    } else {
        DBUG_ASSERT ((0), "High dimension is not suppored yet!");
    }

    DBUG_RETURN (assigns);
}

static node *
BuildStoreAssigns (node *part, info *arg_info)
{
    node *vardecs = NULL, *assigns = NULL;
    node *args, *avis, *tx_zero, *ty_zero, *cnst_zero;
    simpletype sty;
    node *target_idx;

    DBUG_ENTER ("BuildStoreAssigns");

    sty = TYgetSimpleType (TYgetScalar (AVIS_TYPE (INFO_SHRARRAY (arg_info))));

    /* Here we generate code so that only thread with tx == 0 and ty == 0
     * selects the first element in the shared memory (final reduction result)
     */
    if (INFO_DIM (arg_info) == 1) {
        args = TBmakeExprs (DUPdoDupNode (INFO_PARTIALSHP (arg_info)),
                            TBmakeExprs (TBmakeId (CIS_BX (INFO_CIS (arg_info))), NULL));

        target_idx = CreatePrfOrConst (TRUE, "idx", T_int, SHmakeShape (0), F_idxs2offset,
                                       args, &vardecs, &assigns);

        args = TBmakeExprs (TBmakeId (CIS_TX (INFO_CIS (arg_info))),
                            TBmakeExprs (TBmakeNum (0), NULL));
        avis = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0), F_eq_SxS, args,
                                 &vardecs, &assigns);

        args = TBmakeExprs (TBmakeId (avis),
                            TBmakeExprs (TBmakeNum (0),
                                         TBmakeExprs (TBmakeId (INFO_SHRARRAY (arg_info)),
                                                      TBmakeExprs (TBmakeId (target_idx),
                                                                   NULL))));

        avis = CreatePrfOrConst (TRUE, "res", sty, SHmakeShape (0), F_cond_wl_assign,
                                 args, &vardecs, &assigns);
    } else if (INFO_DIM (arg_info) == 2) {
        if (INFO_INNERDIM (arg_info) == 0) {
            args = TBmakeExprs (DUPdoDupNode (INFO_PARTIALSHP (arg_info)),
                                TBmakeExprs (TBmakeId (CIS_BY (INFO_CIS (arg_info))),
                                             TBmakeExprs (TBmakeId (
                                                            CIS_BX (INFO_CIS (arg_info))),
                                                          NULL)));
        } else {
            args = TBmakeExprs (DUPdoDupNode (INFO_PARTIALSHP (arg_info)),
                                TBmakeExprs (TBmakeId (CIS_BY (INFO_CIS (arg_info))),
                                             TBmakeExprs (TBmakeId (IDS_AVIS (IDS_NEXT (
                                                            INFO_WITHIDS (arg_info)))),
                                                          NULL)));
        }

        target_idx = CreatePrfOrConst (TRUE, "idx", T_int, SHmakeShape (0), F_idxs2offset,
                                       args, &vardecs, &assigns);

        if (INFO_INNERDIM (arg_info) == 0) {
            args = TBmakeExprs (TBmakeId (CIS_TX (INFO_CIS (arg_info))),
                                TBmakeExprs (TBmakeNum (0), NULL));
            tx_zero = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0), F_eq_SxS,
                                        args, &vardecs, &assigns);

            args = TBmakeExprs (TBmakeId (CIS_TY (INFO_CIS (arg_info))),
                                TBmakeExprs (TBmakeNum (0), NULL));
            ty_zero = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0), F_eq_SxS,
                                        args, &vardecs, &assigns);

            args
              = TBmakeExprs (TBmakeId (tx_zero), TBmakeExprs (TBmakeId (ty_zero), NULL));
            avis = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0), F_and_SxS,
                                     args, &vardecs, &assigns);

            cnst_zero = CreatePrfOrConst (FALSE, "zero", T_int, SHmakeShape (0),
                                          F_unknown, TBmakeNum (0), &vardecs, &assigns);

            args
              = TBmakeExprs (TBmakeId (avis),
                             TBmakeExprs (TBmakeId (cnst_zero),
                                          TBmakeExprs (TBmakeId (
                                                         INFO_SHRARRAY (arg_info)),
                                                       TBmakeExprs (TBmakeId (target_idx),
                                                                    NULL))));
        } else {
            args = TBmakeExprs (TBmakeId (CIS_TY (INFO_CIS (arg_info))),
                                TBmakeExprs (TBmakeNum (0), NULL));
            avis = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0), F_eq_SxS,
                                     args, &vardecs, &assigns);

            args
              = TBmakeExprs (TBmakeId (avis),
                             TBmakeExprs (TBmakeId (CIS_TX (INFO_CIS (arg_info))),
                                          TBmakeExprs (TBmakeId (
                                                         INFO_SHRARRAY (arg_info)),
                                                       TBmakeExprs (TBmakeId (target_idx),
                                                                    NULL))));
        }

        avis = CreatePrfOrConst (TRUE, "res", sty, SHmakeShape (0), F_cond_wl_assign,
                                 args, &vardecs, &assigns);
    }

    FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
      = TCappendVardec (FUNDEF_VARDEC (INFO_FUNDEF (arg_info)), vardecs);

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
 * @fn node *ACUWLdoAnnotateCUDAWL( node *syntax_tree)
 *
 *****************************************************************************/
node *
PFDdoPartialFold (node *syntax_tree)
{
    info *info;
    DBUG_ENTER ("PFDdoPartialFold");

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
    DBUG_ENTER ("PFDmodule");

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

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
    DBUG_ENTER ("PFDfundef");

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

    DBUG_ENTER ("PFDassign");

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    old_lastassign = INFO_LASTASSIGN (arg_info);
    INFO_LASTASSIGN (arg_info) = arg_node;
    ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);
    INFO_LASTASSIGN (arg_info) = old_lastassign;

    if (INFO_POSTASSIGN (arg_info) != NULL && INFO_LEVEL (arg_info) == 0) {
        ASSIGN_NEXT (arg_node)
          = TCappendAssign (INFO_POSTASSIGN (arg_info), ASSIGN_NEXT (arg_node));
        INFO_POSTASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ACUWLlet( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
PFDlet (node *arg_node, info *arg_info)
{
    node *old_lhs;

    DBUG_ENTER ("PFDlet");

    old_lhs = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);
    INFO_LHS (arg_info) = old_lhs;

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

    DBUG_ENTER ("PFDgenerator");

    bound1 = GENERATOR_BOUND1 (arg_node);
    bound2 = GENERATOR_BOUND2 (arg_node);

    DBUG_ASSERT (COisConstant (bound1), "Bound1 is not constant");
    DBUG_ASSERT (COisConstant (bound2), "Bound2 is not constant");

    INFO_PARTSHP (arg_info)
      = COconstant2AST (COsub (COaST2Constant (bound2), COaST2Constant (bound1), NULL));

    DBUG_ASSERT (NODE_TYPE (INFO_PARTSHP (arg_info)) = N_array,
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
    DBUG_ENTER ("PFDwithid");

    INFO_DIM (arg_info) = TCcountIds (WITHID_IDS (arg_node));
    INFO_WITHIDS (arg_info) = WITHID_IDS (arg_node);

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
    node *cexpr_avis;

    DBUG_ENTER ("PFDcode");

    /* collect the type of the result and the folding operation */
    cexpr_avis = ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (arg_node)));
    INFO_CEXPR (arg_info) = cexpr_avis;
    INFO_FOLDOP (arg_info)
      = PRF_PRF (LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (cexpr_avis))));

    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

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
    int reduce_dim, dim, tbshp_len, partshp_len, partialshp_len;
    node *tbshp_elems = NULL, *partshp_elems = NULL, *partialshp_elems = NULL;
    node *init_assigns, *load_assigns, *reduce_assigns, *store_assigns;

    DBUG_ENTER ("PFDpart");

    DBUG_ASSERT ((PART_NEXT (arg_node) == NULL),
                 "Found fold withloop with more than one partition!");

    /* Collect the dimension of this part and store it in INFO_DIM */
    PART_WITHID (arg_node) = TRAVopt (PART_WITHID (arg_node), arg_info);

    /* Work out the shape of this part as the different between upper
     * bound and lower bound and store it in INFO_PARTSHP */
    PART_GENERATOR (arg_node) = TRAVopt (PART_GENERATOR (arg_node), arg_info);

    /* Based on the code cexpr, work out the folding operation of this
     * fold withloop and store it in INFO_FOLDOP */
    PART_CODE (arg_node) = TRAVopt (PART_CODE (arg_node), arg_info);

    tbshp_elems = ARRAY_AELEMS (PART_THREADBLOCKSHAPE (arg_node));
    partshp_elems = ARRAY_AELEMS (INFO_PARTSHP (arg_info));

    reduce_dim = INFO_DIM (arg_info) - INFO_INNERDIM (arg_info);
    dim = 0;
    while (dim < INFO_DIM (arg_info)) {
        DBUG_ASSERT ((tbshp_elems != NULL), "Thread shape is NULL!");
        DBUG_ASSERT ((partshp_elems != NULL), "Partition shape is NULL!");

        tbshp_len = NUM_VAL (EXPRS_EXPR (tbshp_elems));
        partshp_len = NUM_VAL (EXPRS_EXPR (partshp_elems));

        if (dim < reduce_dim) {
            if (partshp_len % tbshp_len == 0) {
                partialshp_len = partshp_len / tbshp_len;
            } else {
                partialshp_len = partshp_len / tbshp_len + 1;
            }
        } else {
            partialshp_len = partshp_len;
        }

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

    INFO_PARTIALSHP (arg_info)
      = TBmakeArray (TYmakeSimpleType (T_int), SHcreateShape (1, INFO_DIM (arg_info)),
                     partialshp_elems);

    /*****************************************************************************/

    init_assigns = CreateCudaIndexInitCode (arg_node, arg_info);

    BLOCK_INSTR (PART_CBLOCK (arg_node))
      = TCappendAssign (init_assigns, BLOCK_INSTR (PART_CBLOCK (arg_node)));

    load_assigns = BuildLoadAssigns (arg_node, arg_info);

    BLOCK_INSTR (PART_CBLOCK (arg_node))
      = TCappendAssign (BLOCK_INSTR (PART_CBLOCK (arg_node)), load_assigns);

    reduce_assigns = BuildReduceAssigns (arg_node, arg_info);

    BLOCK_INSTR (PART_CBLOCK (arg_node))
      = TCappendAssign (BLOCK_INSTR (PART_CBLOCK (arg_node)), reduce_assigns);

    store_assigns = BuildStoreAssigns (arg_node, arg_info);

    BLOCK_INSTR (PART_CBLOCK (arg_node))
      = TCappendAssign (BLOCK_INSTR (PART_CBLOCK (arg_node)), store_assigns);

    /* This struct is created in CreateCudaIndexInitCode and freed here */
    INFO_CIS (arg_info) = MEMfree (INFO_CIS (arg_info));

    INFO_PARTSHP (arg_info) = FREEdoFreeTree (INFO_PARTSHP (arg_info));

    DBUG_RETURN (arg_node);
}

static node *
BuildFoldWithloop (node *old_foldwl, info *arg_info)
{
    int dim;
    node *new_foldwl, *avis;
    node *part, *new_code, *withop;
    node *withid, *generator;
    node *vec, *ids = NULL;
    node *bound1, *bound2;

    node *accu_avis, *offset_avis, *elem_avis, *op_avis;
    node *accu_ass, *offset_ass, *elem_ass, *op_ass;
    ntype *cexpr_type;
    node *code_instr;
    prf op;

    DBUG_ENTER ("BuildFoldWithloop");

    if (INFO_INNERDIM (arg_info) == 0) {
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
    } else {
        int outer_dim, inner_dim;
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
        printf ("Number of inner ub elems %d\n", TCcountExprs (partialshp_elems));

        inner_ub_elems = TCgetNthExprs (outer_dim, partialshp_elems);
        EXPRS_NEXT (TCgetNthExprs (outer_dim - 1, partialshp_elems)) = NULL;
        outer_ub_elems = partialshp_elems;

        printf ("Number of inner ub elems %d\n", TCcountExprs (inner_ub_elems));

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

        cexpr_type = TYmakeAKS (TYmakeSimpleType (T_int), SHarray2Shape (inner_bound2));

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

        /* new genarray withloop */
        new_genarraywl = TBmakeWith (part, new_code, withop);
        WITH_PARTS (new_genarraywl) = 1;

        /************* End of Creating Inner Genarray Withloop **************/

        op_avis = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
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
    DBUG_ENTER ("ATravWithid");

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

    FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
      = TBmakeVardec (IDS_AVIS (WITHID_VEC (arg_node)),
                      FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

    DBUG_RETURN (arg_node);
}

node *
ATravGenerator (node *arg_node, info *arg_info)
{
    node *outer_b1, *outer_b2, *inner_b1, *inner_b2;
    node *outer_genwidth, *inner_genwidth;

    DBUG_ENTER ("ATravGenerator");

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

    DBUG_ENTER ("ATravCode");

    assigns = CODE_CBLOCK_INSTR (arg_node);

    DBUG_ASSERT (assigns != NULL, "Fold withloop body is empty!");

    prev = assigns;
    next = prev;

    while (next != NULL && next != INFO_AT_INNERWITHASSIGN (arg_info)) {
        prev = next;
        next = ASSIGN_NEXT (next);
    }

    DBUG_ASSERT (next != NULL, "Inner withloop is not found!");

    ASSIGN_NEXT (prev) = INFO_AT_INNERWITHCODE (arg_info);

    CODE_CBLOCK_INSTR (arg_node)
      = TCappendAssign (INFO_AT_VECASSIGNS (arg_info), assigns);

    DBUG_RETURN (arg_node);
}

node *
ATravPart (node *arg_node, info *arg_info)
{
    node *lhs_avis, *cexpr, *ssa_assign, *inner_wl;
    int cat_dim;

    DBUG_ENTER ("ATravPart");

    DBUG_ASSERT ((PART_NEXT (arg_node) == NULL),
                 "Found fold withloop with more than one partition!");

    lhs_avis = IDS_AVIS (INFO_LHS (arg_info));

    cexpr = EXPRS_EXPR (PART_CEXPRS (arg_node));
    DBUG_ASSERT (NODE_TYPE (cexpr) == N_id, "Non N_id cexpr found!");
    ssa_assign = AVIS_SSAASSIGN (ID_AVIS (cexpr));

    INFO_AT_INNERWITHASSIGN (arg_info) = ssa_assign;
    inner_wl = ASSIGN_RHS (ssa_assign);

    cat_dim = TCcountIds (PART_IDS (arg_node)) + TYgetDim (AVIS_TYPE (ID_AVIS (cexpr)));

    /* After scalarizing, we expect the outer fold withloop
     * to be at most 3D */
    if (!TUisScalar (AVIS_TYPE (ID_AVIS (cexpr))) && cat_dim <= 3 && cat_dim >= 1
        && /* The concatenated dimension is 1 <= dim <= 3 */
        NODE_TYPE (ASSIGN_INSTR (ssa_assign)) == N_let
        && /* The result is deinfed by a wl */
        NODE_TYPE (inner_wl) == N_with && PART_NEXT (WITH_PART (inner_wl)) == NULL
        && /* The defining wl has only one partition */
        (NODE_TYPE (WITH_WITHOP (inner_wl)) == N_genarray
         || NODE_TYPE (WITH_WITHOP (inner_wl)) == N_modarray)) {
        CHANGED = 1;
        INFO_AT_INNERDIM (arg_info) = TYgetDim (AVIS_TYPE (ID_AVIS (cexpr)));

        INFO_AT_VECASSIGNS (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (IDS_AVIS (WITH_VEC (inner_wl)), NULL),
                                     TBmakeArray (TYmakeSimpleType (T_int),
                                                  SHcreateShape (1, TCcountIds (WITH_IDS (
                                                                      inner_wl))),
                                                  TCconvertIds2Exprs (
                                                    WITH_IDS (inner_wl)))),
                          INFO_AT_VECASSIGNS (arg_info));

        AVIS_SSAASSIGN (IDS_AVIS (WITH_VEC (inner_wl))) = INFO_AT_VECASSIGNS (arg_info);

        INFO_AT_INNERWITHIDS (arg_info) = WITH_IDS (inner_wl);
        INFO_AT_INNERWITHVEC (arg_info) = WITH_VEC (inner_wl);
        INFO_AT_INNERWITHBOUND1 (arg_info) = WITH_BOUND1 (inner_wl);
        INFO_AT_INNERWITHBOUND2 (arg_info) = WITH_BOUND2 (inner_wl);
        INFO_AT_INNERWITHGENWIDTH (arg_info) = WITH_GENWIDTH (inner_wl);
        INFO_AT_INNERWITHCODE (arg_info) = BLOCK_INSTR (WITH_CBLOCK (inner_wl));

        PART_WITHID (arg_node) = TRAVopt (PART_WITHID (arg_node), arg_info);
        PART_GENERATOR (arg_node) = TRAVopt (PART_GENERATOR (arg_node), arg_info);
        PART_CODE (arg_node) = TRAVopt (PART_CODE (arg_node), arg_info);

        /* Set the cexpr to be the cexpr of the original inner withloop */
        ID_AVIS (EXPRS_EXPR (PART_CEXPRS (arg_node))) = ID_AVIS (WITH_CEXPR (inner_wl));

        if (cat_dim == 2) {
            PART_THREADBLOCKSHAPE (arg_node)
              = FREEdoFreeNode (PART_THREADBLOCKSHAPE (arg_node));

            /* Now we need a 2D thread block */
            PART_THREADBLOCKSHAPE (arg_node)
              = TBmakeArray (TYmakeSimpleType (T_int),
                             SHcreateShape (1, TCcountIds (PART_IDS (arg_node))),
                             TBmakeExprs (TBmakeNum (global.cuda_2d_block_y),
                                          TBmakeExprs (TBmakeNum (global.cuda_2d_block_x),
                                                       NULL)));
        }
    }

    DBUG_RETURN (arg_node);
}

node *
ATravWith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ATravPart");

    if (WITH_CUDARIZABLE (arg_node) && NODE_TYPE (WITH_WITHOP (arg_node)) == N_fold) {
        WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
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

    DBUG_ENTER ("PFDwith");

    /************ Anonymous Traversal ************/
    anontrav_t atrav[6]
      = {{N_with, &ATravWith},           {N_part, &ATravPart},     {N_code, &ATravCode},
         {N_generator, &ATravGenerator}, {N_withid, &ATravWithid}, {0, NULL}};

    /* Here we travverse the fold withloop until no
     * changes have happened. This is because the inner
     * withloop defining the result can itself contain
     * withloops so we need to scalarized those withloops
     * as well once we have scalarized the outer withloop.
     * However, there is a problem here: The scalarization
     * of outer withloop is committed before we traverse
     * into any inner withloops and if we later found that
     * those inner withloops cannot be scalarized then it
     * will impossible to undo the previous scalarization.
     * This leaves an intermediate withloop which cannot
     * be recognized by other compile phases. So the solution
     * is to perform a thorough check first to confirm that
     * the withloop can indeed be scalarized before we actually
     * transform it. But this has not been implemented yet (TODO)  */
    if (CHANGED == 1) {
        CHANGED = 0;

        TRAVpushAnonymous (atrav, &TRAVsons);

        anon_info = MakeInfo ();
        INFO_LHS (anon_info) = INFO_LHS (arg_info);
        INFO_FUNDEF (anon_info) = INFO_FUNDEF (arg_info);

        arg_node = TRAVdo (arg_node, anon_info);

        INFO_INNERDIM (arg_info) += INFO_AT_INNERDIM (anon_info);

        anon_info = FreeInfo (anon_info);
        TRAVpop ();
    }
    /*********************************************/

    if (WITH_CUDARIZABLE (arg_node) && NODE_TYPE (WITH_WITHOP (arg_node)) == N_fold) {

        DBUG_ASSERT (FOLD_ISPARTIALFOLD (WITH_WITHOP (arg_node)),
                     "CUDA fold withloop must be partial folding!");

        avis = IDS_AVIS (INFO_LHS (arg_info));

        /* The withop has to be traversed before part. Because we need the
         * neutral element during the code traversal (i.e. to replace the
         * rhs of F_accu with the neutral element). */
        INFO_LEVEL (arg_info)++;
        WITH_WITHOP (arg_node) = TRAVopt (WITH_WITHOP (arg_node), arg_info);
        WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
        INFO_LEVEL (arg_info)--;

        /******* Create a new variable for the partial folding result ******/

        new_avis = DUPdoDupNode (avis);
        AVIS_SSAASSIGN (new_avis) = INFO_LASTASSIGN (arg_info);
        AVIS_NAME (new_avis) = MEMfree (AVIS_NAME (new_avis));
        AVIS_NAME (new_avis) = TRAVtmpVar ();
        sty = TYgetSimpleType (TYgetScalar (AVIS_TYPE (new_avis)));
        AVIS_TYPE (new_avis) = TYfreeType (AVIS_TYPE (new_avis));
        AVIS_TYPE (new_avis) = TYmakeAKS (TYmakeSimpleType (sty),
                                          SHarray2Shape (INFO_PARTIALSHP (arg_info)));

        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = TCappendVardec (FUNDEF_VARDEC (INFO_FUNDEF (arg_info)),
                            TBmakeVardec (new_avis, NULL));

        IDS_AVIS (INFO_LHS (arg_info)) = new_avis;
        INFO_PARTIALARR (arg_info) = new_avis;

        /*******************************************************************/

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
    DBUG_ENTER ("PFDfold");

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
    DBUG_ENTER ("PFDprf");

    if (INFO_LEVEL (arg_info) == 1) {
        if (PRF_PRF (arg_node) == F_accu) {
            arg_node = FREEdoFreeNode (arg_node);
            arg_node = DUPdoDupNode (INFO_NEUTRAL (arg_info));
        }
    } else {
        PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
