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
    int dim;
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
};

#define INFO_LHS(n) (n->lhs)
#define INFO_LEVEL(n) (n->level)
#define INFO_PARTSHP(n) (n->partshp)
#define INFO_PARTIALSHP(n) (n->partialshp)
#define INFO_DIM(n) (n->dim)
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
    INFO_DIM (result) = 0;
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

    args = TBmakeExprs (DUPdoDupNode (PART_THREADBLOCKSHAPE (part)),
                        TBmakeExprs (TBmakeId (CIS_TY (INFO_CIS (arg_info))),
                                     TBmakeExprs (TBmakeId (CIS_TX (INFO_CIS (arg_info))),
                                                  NULL)));

    /* Create  i.e. idx_modarray_AxSxS */
    avis = CreatePrfOrConst (TRUE, "idx_shr", T_int, SHmakeShape (0), F_idxs2offset, args,
                             &vardecs, &assigns);

    /* Create shared memory */
    shmem = TBmakeAvis (TRAVtmpVarName ("shmem"),
                        TYmakeAKS (TYmakeSimpleType (
                                     CUh2shSimpleTypeConversion (TYgetSimpleType (
                                       TYgetScalar (AVIS_TYPE (INFO_CEXPR (arg_info)))))),
                                   SHarray2Shape (PART_THREADBLOCKSHAPE (part))));

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
BuildReduceAssigns (node *part, info *arg_info)
{
    node *assigns = NULL, *vardecs = NULL, *args, *avis;
    simpletype sty;

    node *loop_assigns = NULL, *predicate = NULL, *iterator;

    int partshp_x, partshp_y, partialshp_y, partialshp_x, shmemshp_x, shmemshp_y,
      remain_x, remain_y;
    node *loop_bound, *cond;
    node *idx1, *idx2, *val1, *val2, *val3;
    node *in_shared_array, *out_shared_array;
    node *cond_predicate;
    node *tx_zero, *ty_zero;

    DBUG_ENTER ("BuildReduceAssigns");

    sty = TYgetSimpleType (TYgetScalar (AVIS_TYPE (INFO_SHRARRAY (arg_info))));

    if (INFO_DIM (arg_info) == 1) {
    } else if (INFO_DIM (arg_info) == 2) {
        in_shared_array = INFO_SHRARRAY (arg_info);

        /* i = 1; */
        iterator = CreatePrfOrConst (FALSE, "iterator", T_int, SHmakeShape (0), F_add_SxS,
                                     TBmakeNum (1), &vardecs, &assigns);

        partshp_y = NUM_VAL (EXPRS_EXPR1 (ARRAY_AELEMS (INFO_PARTSHP (arg_info))));
        partshp_x = NUM_VAL (EXPRS_EXPR2 (ARRAY_AELEMS (INFO_PARTSHP (arg_info))));
        partialshp_y = NUM_VAL (EXPRS_EXPR1 (ARRAY_AELEMS (INFO_PARTIALSHP (arg_info))));
        partialshp_x = NUM_VAL (EXPRS_EXPR2 (ARRAY_AELEMS (INFO_PARTIALSHP (arg_info))));
        shmemshp_y = NUM_VAL (EXPRS_EXPR1 (ARRAY_AELEMS (PART_THREADBLOCKSHAPE (part))));
        shmemshp_x = NUM_VAL (EXPRS_EXPR2 (ARRAY_AELEMS (PART_THREADBLOCKSHAPE (part))));

        remain_y = partshp_y % shmemshp_y;
        remain_x = partshp_x % shmemshp_x;

        if (remain_y != 0) {
            node *bound1, *bound2, *by_bound;
            bound1
              = CreatePrfOrConst (FALSE, "loop_bound", T_int, SHmakeShape (0), F_unknown,
                                  TBmakeNum (shmemshp_y), &vardecs, &assigns);

            bound2
              = CreatePrfOrConst (FALSE, "loop_bound", T_int, SHmakeShape (0), F_unknown,
                                  TBmakeNum (remain_y), &vardecs, &assigns);

            by_bound
              = CreatePrfOrConst (FALSE, "by_bound", T_int, SHmakeShape (0), F_unknown,
                                  TBmakeNum (partialshp_y - 1), &vardecs, &assigns);

            args = TBmakeExprs (TBmakeId (CIS_BY (INFO_CIS (arg_info))),
                                TBmakeExprs (TBmakeId (by_bound), NULL));

            cond = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0), F_eq_SxS,
                                     args, &vardecs, &assigns);

            args = TBmakeExprs (TBmakeId (cond),
                                TBmakeExprs (TBmakeId (bound2),
                                             TBmakeExprs (TBmakeId (bound1), NULL)));

            loop_bound = CreatePrfOrConst (TRUE, "loop_bound", T_int, SHmakeShape (0),
                                           F_cond, args, &vardecs, &assigns);
        } else {
            loop_bound
              = CreatePrfOrConst (FALSE, "loop_bound", T_int, SHmakeShape (0), F_unknown,
                                  TBmakeNum (shmemshp_y), &vardecs, &assigns);
        }

        /* Loop body */
        avis = CreatePrfOrConst (FALSE, "const", T_int, SHmakeShape (0), F_unknown,
                                 TBmakeNum (0), &vardecs, &loop_assigns);

        args = TBmakeExprs (DUPdoDupNode (PART_THREADBLOCKSHAPE (part)),
                            TBmakeExprs (TBmakeId (avis),
                                         TBmakeExprs (TBmakeId (
                                                        CIS_TX (INFO_CIS (arg_info))),
                                                      NULL)));

        idx1 = CreatePrfOrConst (TRUE, "idx", T_int, SHmakeShape (0), F_idxs2offset, args,
                                 &vardecs, &loop_assigns);

        args = TBmakeExprs (TBmakeId (idx1),
                            TBmakeExprs (TBmakeId (INFO_SHRARRAY (arg_info)), NULL));

        val1 = CreatePrfOrConst (TRUE, "val", sty, SHmakeShape (0), F_idx_sel, args,
                                 &vardecs, &loop_assigns);

        args = TBmakeExprs (DUPdoDupNode (PART_THREADBLOCKSHAPE (part)),
                            TBmakeExprs (TBmakeId (iterator),
                                         TBmakeExprs (TBmakeId (
                                                        CIS_TX (INFO_CIS (arg_info))),
                                                      NULL)));

        idx2 = CreatePrfOrConst (TRUE, "idx", T_int, SHmakeShape (0), F_idxs2offset, args,
                                 &vardecs, &loop_assigns);

        args = TBmakeExprs (TBmakeId (idx2),
                            TBmakeExprs (TBmakeId (INFO_SHRARRAY (arg_info)), NULL));

        val2 = CreatePrfOrConst (TRUE, "val", sty, SHmakeShape (0), F_idx_sel, args,
                                 &vardecs, &loop_assigns);

        args = TBmakeExprs (TBmakeId (val1), TBmakeExprs (TBmakeId (val2), NULL));

        val3 = CreatePrfOrConst (TRUE, "val", sty, SHmakeShape (0),
                                 INFO_FOLDOP (arg_info), args, &vardecs, &loop_assigns);

        args = TBmakeExprs (TBmakeId (INFO_SHRARRAY (arg_info)),
                            TBmakeExprs (TBmakeId (idx1),
                                         TBmakeExprs (TBmakeId (val3), NULL)));

        INFO_SHRARRAY (arg_info)
          = CreatePrfOrConst (TRUE, "shmem", sty,
                              SHarray2Shape (PART_THREADBLOCKSHAPE (part)),
                              F_idx_modarray_AxSxS, args, &vardecs, &loop_assigns);

        args = TBmakeExprs (TBmakeId (CIS_TY (INFO_CIS (arg_info))),
                            TBmakeExprs (TBmakeNum (0), NULL));

        cond_predicate = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0),
                                           F_eq_SxS, args, &vardecs, &predicate);

        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = TCappendVardec (FUNDEF_VARDEC (INFO_FUNDEF (arg_info)), vardecs);

        vardecs = NULL;

        /* After this, loop_assigns is the function application of a loop function */
        loop_assigns
          = CLACFdoCreateLacFun (FALSE, INFO_FUNDEF (arg_info), loop_assigns, NULL,
                                 iterator, loop_bound, in_shared_array,
                                 INFO_SHRARRAY (arg_info), &INFO_LACFUNS (arg_info));

        /* This is the final assignment chain used to create the conditional function */
        assigns = TCappendAssign (assigns, loop_assigns);

        /* Create conditional */
        out_shared_array = INFO_SHRARRAY (arg_info);

        /* After this, assigns is the function application of a conditional function */
        assigns = CLACFdoCreateLacFun (TRUE, INFO_FUNDEF (arg_info), assigns,
                                       cond_predicate, NULL, NULL, in_shared_array,
                                       out_shared_array, &INFO_LACFUNS (arg_info));

        assigns = TCappendAssign (predicate, assigns);

        /* Thread at tx=0 && ty=0 select the final reduction result */
        args = TBmakeExprs (TBmakeId (CIS_TX (INFO_CIS (arg_info))),
                            TBmakeExprs (TBmakeNum (0), NULL));
        tx_zero = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0), F_eq_SxS, args,
                                    &vardecs, &assigns);

        args = TBmakeExprs (TBmakeId (CIS_TY (INFO_CIS (arg_info))),
                            TBmakeExprs (TBmakeNum (0), NULL));
        ty_zero = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0), F_eq_SxS, args,
                                    &vardecs, &assigns);

        args = TBmakeExprs (TBmakeId (tx_zero), TBmakeExprs (TBmakeId (ty_zero), NULL));
        avis = CreatePrfOrConst (TRUE, "cond", T_bool, SHmakeShape (0), F_and_SxS, args,
                                 &vardecs, &assigns);

    } else {
        DBUG_ASSERT ((0), "High dimension is not suppored yet!");
    }

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
    int dim, tbshp_len, partshp_len, partialshp_len;
    node *tbshp_elems = NULL, *partshp_elems = NULL, *partialshp_elems = NULL;

    node *init_assigns, *load_assigns, *reduce_assigns;

    DBUG_ENTER ("PFDpart");

    DBUG_ASSERT ((PART_NEXT (arg_node) == NULL),
                 "Found fold withloop with more than one partition!");

    PART_WITHID (arg_node) = TRAVopt (PART_WITHID (arg_node), arg_info);
    PART_GENERATOR (arg_node) = TRAVopt (PART_GENERATOR (arg_node), arg_info);
    PART_CODE (arg_node) = TRAVopt (PART_CODE (arg_node), arg_info);

    tbshp_elems = ARRAY_AELEMS (PART_THREADBLOCKSHAPE (arg_node));
    partshp_elems = ARRAY_AELEMS (INFO_PARTSHP (arg_info));

    dim = 0;
    while (dim < INFO_DIM (arg_info)) {
        DBUG_ASSERT ((tbshp_elems != NULL), "Thread shape is NULL!");
        DBUG_ASSERT ((partshp_elems != NULL), "Partition shape is NULL!");

        tbshp_len = NUM_VAL (EXPRS_EXPR (tbshp_elems));
        partshp_len = NUM_VAL (EXPRS_EXPR (partshp_elems));

        if (partshp_len % tbshp_len == 0) {
            partialshp_len = partshp_len / tbshp_len;
        } else {
            partialshp_len = partshp_len / tbshp_len + 1;
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
    accu_ass = TBmakeAssign (TBmakeLet (TBmakeIds (accu_avis, NULL),
                                        TCmakePrf1 (F_accu, TBmakeId (IDS_AVIS (vec)))),
                             NULL);
    AVIS_SSAASSIGN (accu_avis) = accu_ass;
    INFO_FUNDEF (arg_info) = AppendVardec (INFO_FUNDEF (arg_info), accu_avis);

    /* tmp2 = idxs2offset([...], i, j, k...); */
    offset_avis
      = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
    offset_ass
      = TBmakeAssign (TBmakeLet (TBmakeIds (offset_avis, NULL),
                                 TBmakePrf (F_idxs2offset,
                                            TBmakeExprs (DUPdoDupNode (
                                                           INFO_PARTIALSHP (arg_info)),
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
                                                        TCappendAssign (op_ass, NULL))));
    new_code = TBmakeCode (TBmakeBlock (code_instr, NULL),
                           TBmakeExprs (TBmakeId (op_avis), NULL));
    CODE_USED (new_code) = 1;
    CODE_NEXT (new_code) = NULL;

    /* Create new partition */
    part = TBmakePart (new_code, withid, generator);

    /* fold withop */
    withop
      = TBmakeFold (INFO_FOLDFUNDEF (arg_info), DUPdoDupNode (INFO_NEUTRAL (arg_info)));

    /* new fold withloop */
    new_foldwl = TBmakeWith (part, new_code, withop);
    WITH_PARTS (new_foldwl) = 1;
    WITH_REFERENCED (new_foldwl) = WITH_REFERENCED (old_foldwl);
    WITH_ISFOLDABLE (new_foldwl) = WITH_ISFOLDABLE (old_foldwl);

    /* Cleanup arg_info */
    INFO_PARTIALARR (arg_info) = NULL;
    INFO_FOLDFUNDEF (arg_info) = NULL;
    INFO_FOLDOP (arg_info) = F_unknown;
    INFO_NEUTRAL (arg_info) = NULL;
    INFO_CEXPR (arg_info) = NULL;
    INFO_DIM (arg_info) = 0;

    DBUG_RETURN (new_foldwl);
}

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

    DBUG_ENTER ("PFDwith");

    avis = IDS_AVIS (INFO_LHS (arg_info));

    if (WITH_CUDARIZABLE (arg_node) && NODE_TYPE (WITH_WITHOP (arg_node)) == N_fold) {

        DBUG_ASSERT (FOLD_ISPARTIALFOLD (WITH_WITHOP (arg_node)),
                     "CUDA fold withloop must be partial folding!");

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
