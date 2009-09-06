/*****************************************************************************
 *
 * file:   create_cuda_kernels.c
 *
 * prefix: CUKNL
 *
 * description:
 *   This module creates CUDA kernel functions from N_with nodes. Each
 *   partition of N_with is created as a CUDA kernel and the code of
 *   partition becomes the body the kernel function. For exmaple:
 *
 *   in_var1 = ...
 *   in_var2 = ...
 *   a_mem = with
 *           {
 *             [l0,l1,l2] <= iv < [u0,u1,u2]
 *             {
 *               ... = var1;
 *               local_var1 = ...;
 *               ...
 *               res1 = ...;
 *             }:res1;
 *             [l3,l4,l5] <= iv < [u3,u4,u5]
 *             {
 *               ... = var2;
 *               local_var2 = ...;
 *               ...
 *               res2 = ...;
 *             }:res2;
 *           } : genarray(shp, a_mem);
 *
 *    ==>
 *
 *   in_var1 = ...
 *   in_var2 = ...
 *   a_mem = CUDA_kenerl1( a_mem, l0, l1, l2, u0, u1, u2, var1);
 *   a_mem = CUDA_kenerl2( a_mem, l3, l4, l5, u3, u4, u5, var2);
 *
 *   void CUDA_kenerl1( a_mem, l0, l1, l2, u0, u1, u2, var1)
 *   {
 *     ... = var1;
 *     local_var1 = ...;
 *     ...
 *     res1 = ...;
 *     return a_mem;
 *   }
 *
 *   void CUDA_kenerl2( a_mem, l3, l4, l5, u3, u4, u5, var2)
 *   {
 *     ... = var2;
 *     local_var2 = ...;
 *     ...
 *     res2 = ...;
 *     return a_mem;
 *   }
 *
 *
 *****************************************************************************/

#include "create_cuda_kernels.h"

#include "dbug.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "DupTree.h"
#include "free.h"
#include "memory.h"
#include "LookUpTable.h"
#include "namespaces.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"
#include "str.h"

static int ids_dim = 0;

/**
 * INFO structure
 */

struct INFO {
    node *cudakernels;
    node *cudaaps;
    node *letids;
    node *fundef;
    node *args;
    node *params;
    node *vardecs;
    node *rets;
    node *retexprs;
    bool collect;
    node *allocassigns;
    node *freeassigns;
    bool is_wlids;
    node *wlids;
    bool is_wlidxs;
    node *prfwlids;
    node *prfwlidxs;
    node *prfgridblock;
    bool hasstepwidth;
    node *part;
    node *d2dsource;
    node *d2dtransfer;
    lut_t *lut;
    int ls_num;
    bool lift_done;
    bool in_withop;
    node *withop;
};

/**
 * INFO macros
 */

#define INFO_CUDAKERNELS(n) (n->cudakernels)
#define INFO_CUDAAPS(n) (n->cudaaps)
#define INFO_LETIDS(n) (n->letids)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_ARGS(n) (n->args)
#define INFO_PARAMS(n) (n->params)
#define INFO_VARDECS(n) (n->vardecs)
#define INFO_RETS(n) (n->rets)
#define INFO_RETEXPRS(n) (n->retexprs)
#define INFO_COLLECT(n) (n->collect)
#define INFO_ALLOCASSIGNS(n) (n->allocassigns)
#define INFO_FREEASSIGNS(n) (n->freeassigns)
#define INFO_IS_WLIDS(n) (n->is_wlids)
#define INFO_WLIDS(n) (n->wlids)
#define INFO_IS_WLIDXS(n) (n->is_wlidxs)
#define INFO_PRFWLIDS(n) (n->prfwlids)
#define INFO_PRFWLIDXS(n) (n->prfwlidxs)
#define INFO_PRFGRIDBLOCK(n) (n->prfgridblock)
#define INFO_HASSTEPWIDTH(n) (n->hasstepwidth)
#define INFO_PART(n) (n->part)
#define INFO_D2DSOURCE(n) (n->d2dsource)
#define INFO_D2DTRANSFER(n) (n->d2dtransfer)
#define INFO_LUT(n) (n->lut)
#define INFO_LS_NUM(n) (n->ls_num)
#define INFO_LIFTDONE(n) (n->lift_done)
#define INFO_INWITHOP(n) (n->in_withop)
#define INFO_WITHOP(n) (n->withop)

/**
 * INFO functions
 */

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_CUDAKERNELS (result) = NULL;
    INFO_CUDAAPS (result) = NULL;
    INFO_LETIDS (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_ARGS (result) = NULL;
    INFO_PARAMS (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_RETS (result) = NULL;
    INFO_RETEXPRS (result) = NULL;
    INFO_COLLECT (result) = FALSE;
    INFO_ALLOCASSIGNS (result) = NULL;
    INFO_FREEASSIGNS (result) = NULL;
    INFO_IS_WLIDS (result) = FALSE;
    INFO_WLIDS (result) = NULL;
    INFO_IS_WLIDXS (result) = FALSE;
    INFO_PRFWLIDS (result) = NULL;
    INFO_PRFWLIDXS (result) = NULL;
    INFO_PRFGRIDBLOCK (result) = NULL;
    INFO_HASSTEPWIDTH (result) = FALSE;
    INFO_PART (result) = NULL;
    INFO_D2DSOURCE (result) = NULL;
    INFO_D2DTRANSFER (result) = NULL;
    INFO_LUT (result) = NULL;
    INFO_LS_NUM (result) = 1;
    INFO_LIFTDONE (result) = FALSE;
    INFO_INWITHOP (result) = FALSE;
    INFO_WITHOP (result) = NULL;

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
 *
 * @fn
 *
 * @brief static void  SetLinksignInfo( node *args, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
static void
SetLinksignInfo (node *args, info *arg_info)
{
    node *iter;

    DBUG_ENTER ("SetLinksignInfo");

    iter = args;

    while (iter != NULL) {
        if (!ARG_HASLINKSIGNINFO (iter)) {
            ARG_LINKSIGN (iter) = INFO_LS_NUM (arg_info);
            ARG_HASLINKSIGNINFO (iter) = TRUE;
            INFO_LS_NUM (arg_info) += 1;
        }
        iter = ARG_NEXT (iter);
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief static node *HandleBoundStepWidthExprs( node *expr,
 *                                                node **gridblock_exprs,
 *                                                char *name,
 *                                                info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
static node *
HandleBoundStepWidthExprs (node *expr, node **gridblock_exprs, char *name, info *arg_info)
{
    node *elements;
    node *avis, *new_avis;
    char *bound_name;
    int dim = 0;
    bool is_bound;

    DBUG_ENTER ("HandleBoundStepWidthExprs");

    DBUG_ASSERT (NODE_TYPE (expr) == N_array, "Expr in not a N_array!");

    /* If the expression list passed in is lower/bound elements,
     * we need to update 'gridblock_exprs' */
    is_bound = (gridblock_exprs != NULL);
    elements = ARRAY_AELEMS (expr);

    while (elements != NULL) {
        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (elements)) == N_id),
                     "Should be an array of N_id nodes");
        avis = ID_AVIS (EXPRS_EXPR (elements));
        INFO_PARAMS (arg_info) = TBmakeExprs (TBmakeId (avis), INFO_PARAMS (arg_info));
        new_avis = DUPdoDupNode (avis);
        AVIS_NAME (new_avis) = MEMfree (AVIS_NAME (new_avis));
        bound_name = (char *)MEMmalloc (sizeof (char) * (STRlen (name) + 2));
        sprintf (bound_name, "%s%d", name, dim);
        AVIS_NAME (new_avis) = bound_name;
        INFO_ARGS (arg_info) = TBmakeArg (new_avis, INFO_ARGS (arg_info));

        if (is_bound) {
            (*gridblock_exprs) = TBmakeExprs (TBmakeId (avis), (*gridblock_exprs));
        }
        elements = EXPRS_NEXT (elements);
        dim++;
    }

    DBUG_RETURN (expr);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief static node* CreateCudaKernelDef( node *code, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
static node *
CreateCudaKernelDef (node *code, info *arg_info)
{
    node *cuda_kerneldef, *body, *vardecs, *args, *retur;
    node *allocassigns, *freeassigns;
    node *prfwlids, *prfwlidxs;

    DBUG_ENTER ("CreateCudaKernelDdef");

    args = INFO_ARGS (arg_info);
    SetLinksignInfo (args, arg_info);
    INFO_ARGS (arg_info) = NULL;

    vardecs = INFO_VARDECS (arg_info);
    INFO_VARDECS (arg_info) = NULL;

    allocassigns = INFO_ALLOCASSIGNS (arg_info);
    INFO_ALLOCASSIGNS (arg_info) = NULL;

    prfwlids = INFO_PRFWLIDS (arg_info);
    INFO_PRFWLIDS (arg_info) = NULL;

    prfwlidxs = INFO_PRFWLIDXS (arg_info);
    INFO_PRFWLIDXS (arg_info) = NULL;

    freeassigns = INFO_FREEASSIGNS (arg_info);
    INFO_FREEASSIGNS (arg_info) = NULL;

    retur = TBmakeReturn (INFO_RETEXPRS (arg_info));

    body = CODE_CBLOCK (code);

    BLOCK_INSTR (body) = TCappendAssign (
      TCappendAssign (TCappendAssign (TCappendAssign (TCappendAssign (allocassigns,
                                                                      prfwlids),
                                                      prfwlidxs),
                                      BLOCK_INSTR (body)),
                      freeassigns),
      TBmakeAssign (retur, NULL));

    BLOCK_VARDEC (body) = TCappendVardec (vardecs, BLOCK_VARDEC (body));

    cuda_kerneldef = TBmakeFundef (TRAVtmpVarName ("CUDA"),
                                   NSdupNamespace (FUNDEF_NS (INFO_FUNDEF (arg_info))),
                                   INFO_RETS (arg_info), args, body, NULL);

    FUNDEF_ISCUDAGLOBALFUN (cuda_kerneldef) = TRUE;
    FUNDEF_HASSTEPWIDTHARGS (cuda_kerneldef) = INFO_HASSTEPWIDTH (arg_info);
    INFO_HASSTEPWIDTH (arg_info) = FALSE;
    FUNDEF_RETURN (cuda_kerneldef) = retur;

    DBUG_RETURN (cuda_kerneldef);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUKNLdoCreateCudaKernels( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUKNLdoCreateCudaKernels (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("CUKNLdoCreateCudaKernels");

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module, "Illegal argument node!");

    info = MakeInfo ();

    TRAVpush (TR_cuknl);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUKNLmodule( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUKNLmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUKNLmodule");

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUKNLfundef( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUKNLfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUKNLfundef");

    if (!FUNDEF_ISCUDAGLOBALFUN (arg_node)) {
        /* Only Non CUDA function may contain N_with to be lifted. */
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = NULL;
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    } else {
        /* We have reached the end of the N_fundef chain,
         * append the newly created CUDA kernels.*/
        FUNDEF_NEXT (arg_node) = INFO_CUDAKERNELS (arg_info);
        INFO_CUDAKERNELS (arg_info) = NULL;
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUKNLassign( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUKNLassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUKNLassign");

    node *ret_node = NULL;
    node *next = NULL;
    node *new_lastassign;

    next = ASSIGN_NEXT (arg_node);

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    /* If we are not in collect mode and the RHS has been
     * lifted as a chain of CUDA kernel applications */
    if (!INFO_COLLECT (arg_info) && INFO_LIFTDONE (arg_info)) {
        /* Free the current N_assign */
        arg_node = FREEdoFreeNode (arg_node);

        /* If there is a <device2device> to be prepended */
        if (INFO_D2DTRANSFER (arg_info) != NULL) {
            ret_node
              = TCappendAssign (INFO_D2DTRANSFER (arg_info), INFO_CUDAAPS (arg_info));
        } else {
            ret_node = INFO_CUDAAPS (arg_info);
        }

        new_lastassign = ret_node;
        while (ASSIGN_NEXT (new_lastassign) != NULL) {
            new_lastassign = ASSIGN_NEXT (new_lastassign);
        }
        ASSIGN_NEXT (new_lastassign) = next;

        INFO_CUDAAPS (arg_info) = NULL;
        INFO_D2DTRANSFER (arg_info) = NULL;
        INFO_LIFTDONE (arg_info) = FALSE;

        ASSIGN_NEXT (new_lastassign) = TRAVopt (ASSIGN_NEXT (new_lastassign), arg_info);
    } else {
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
        ret_node = arg_node;
    }

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUKNLlet( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUKNLlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUKNLlet");

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);
    INFO_LETIDS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUKNLid(node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUKNLid (node *arg_node, info *arg_info)
{
    node *avis, *new_avis, *dim, *shape;
    node *alloc, *free;
    node *prf_wlids, *prf_wlidxs;
    bool in_withid;

    DBUG_ENTER ("CUKNLid");

    avis = ID_AVIS (arg_node);
    in_withid = (INFO_IS_WLIDS (arg_info) || INFO_IS_WLIDXS (arg_info));

    new_avis = NULL;
    dim = NULL;
    shape = NULL;
    alloc = NULL;
    free = NULL;

    DBUG_PRINT ("CUKNL", ("ENTER id %s", ID_NAME (arg_node)));

    if (INFO_COLLECT (arg_info)) {
        if (LUTsearchInLutPp (INFO_LUT (arg_info), avis) == avis) {
            new_avis = DUPdoDupNode (avis);

            /* If the N_id is not in N_withid, we create
             * N_fundef/N_ap arguments from it. */
            if (!in_withid) {
                INFO_ARGS (arg_info) = TBmakeArg (new_avis, INFO_ARGS (arg_info));
                INFO_PARAMS (arg_info)
                  = TBmakeExprs (TBmakeId (avis), INFO_PARAMS (arg_info));

                if (INFO_INWITHOP (arg_info)) {
                    ARG_LINKSIGN (INFO_ARGS (arg_info)) = INFO_LS_NUM (arg_info);
                    ARG_HASLINKSIGNINFO (INFO_ARGS (arg_info)) = TRUE;
                    INFO_LS_NUM (arg_info) += 1;
                } else {
                    /* We only set the new avis if the N_id is
                     * NOT in N_genarray/N_modarray */
                    ID_AVIS (arg_node) = new_avis;
                }
            }

            INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), avis, new_avis);
        } else {
            ID_AVIS (arg_node) = LUTsearchInLutPp (INFO_LUT (arg_info), avis);
        }

        if (in_withid) {
            INFO_VARDECS (arg_info) = TBmakeVardec (new_avis, INFO_VARDECS (arg_info));

            if (TUdimKnown (AVIS_TYPE (new_avis))) {
                dim = TBmakeNum (TYgetDim (AVIS_TYPE (new_avis)));
            }

            if (TUshapeKnown (AVIS_TYPE (new_avis))) {
                shape = SHshape2Array (TYgetShape (AVIS_TYPE (new_avis)));
            }

            /* Create F_alloc and F_free for N_withid->ids and N_withid->idxs */
            alloc = TCmakePrf3 (F_alloc, TBmakeNum (1), dim, shape);
            INFO_ALLOCASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (new_avis, NULL), alloc),
                              INFO_ALLOCASSIGNS (arg_info));

            free = TCmakePrf1 (F_free, TBmakeId (new_avis));
            INFO_FREEASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (NULL, free), INFO_FREEASSIGNS (arg_info));

            /* Create F_cuda_wlids and F_cuda_wlidxs */
            if (INFO_IS_WLIDS (arg_info)) {
                prf_wlids
                  = TCmakePrf3 (F_cuda_wlids, TBmakeId (new_avis), TBmakeNum (ids_dim),
                                TBmakeNum (TCcountExprs (INFO_WLIDS (arg_info))));
                INFO_PRFWLIDS (arg_info)
                  = TBmakeAssign (TBmakeLet (NULL, prf_wlids), INFO_PRFWLIDS (arg_info));
                ids_dim++;
            }

            if (INFO_IS_WLIDXS (arg_info)) {
                node *exprs = NULL;
                /* The avis of the returned device variable */
                node *dev_avis = ARG_AVIS (INFO_ARGS (arg_info));
                node *wlids = INFO_WLIDS (arg_info);
                node *wlids_avis;
                while (wlids != NULL) {
                    DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (wlids)) == N_id,
                                 "N_withid->ids is not a list of N_id nodes!");
                    wlids_avis = LUTsearchInLutPp (INFO_LUT (arg_info),
                                                   ID_AVIS (EXPRS_EXPR (wlids)));
                    exprs
                      = TCappendExprs (exprs, TBmakeExprs (TBmakeId (wlids_avis), NULL));
                    wlids = EXPRS_NEXT (wlids);
                }
                exprs
                  = TBmakeExprs (TBmakeNum (TCcountExprs (INFO_WLIDS (arg_info))), exprs);
                exprs = TBmakeExprs (TBmakeId (dev_avis), exprs);
                exprs = TBmakeExprs (TBmakeId (new_avis), exprs);
                prf_wlidxs = TBmakePrf (F_cuda_wlidxs, exprs);
                INFO_PRFWLIDXS (arg_info) = TBmakeAssign (TBmakeLet (NULL, prf_wlidxs),
                                                          INFO_PRFWLIDXS (arg_info));
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUKNLids( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUKNLids (node *arg_node, info *arg_info)
{
    node *avis, *new_avis;

    DBUG_ENTER ("CUKNLids");

    avis = IDS_AVIS (arg_node);

    DBUG_PRINT ("CUKNL", ("ENTER ids %s", IDS_NAME (arg_node)));

    if (INFO_COLLECT (arg_info) && PART_CUDARIZABLE (INFO_PART (arg_info))) {
        /* Not come across before */
        if (LUTsearchInLutPp (INFO_LUT (arg_info), avis) == avis) {
            new_avis = DUPdoDupNode (avis);
            INFO_VARDECS (arg_info) = TBmakeVardec (new_avis, INFO_VARDECS (arg_info));
            AVIS_DECL (new_avis) = INFO_VARDECS (arg_info);
            INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), avis, new_avis);
            DBUG_PRINT ("CUKNL", (">>> ids %s added to LUT", IDS_NAME (arg_node)));
        }
        IDS_AVIS (arg_node) = LUTsearchInLutPp (INFO_LUT (arg_info), avis);
    }

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUKNLwith( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUKNLwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUKNLwith");

    if (WITH_CUDARIZABLE (arg_node)) {
        /* Start collecting data flow information */
        INFO_COLLECT (arg_info) = TRUE;
        INFO_WITHOP (arg_info) = WITH_WITHOP (arg_node);
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
        INFO_COLLECT (arg_info) = FALSE;
        INFO_LIFTDONE (arg_info) = TRUE;
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUKNLgenarray( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUKNLgenarray (node *arg_node, info *arg_info)
{
    node *avis, *new_avis;

    DBUG_ENTER ("CUKNLgenarray");

    if (INFO_COLLECT (arg_info)) {
        GENARRAY_DEFAULT (arg_node) = TRAVopt (GENARRAY_DEFAULT (arg_node), arg_info);
        GENARRAY_MEM (arg_node) = TRAVopt (GENARRAY_MEM (arg_node), arg_info);

        avis = ID_AVIS (GENARRAY_MEM (arg_node));

        /* The memval in N_genarray is the return value */
        INFO_RETS (arg_info)
          = TCappendRet (TBmakeRet (TYeliminateAKV (AVIS_TYPE (avis)), NULL),
                         INFO_RETS (arg_info));

        /* Set the linksign to the linksign of the return memval */
        RET_LINKSIGN (INFO_RETS (arg_info)) = ARG_LINKSIGN (INFO_ARGS (arg_info));
        RET_HASLINKSIGNINFO (INFO_RETS (arg_info)) = TRUE;

        /* Set the declaration of the return N_id to the memval argument */
        new_avis = DUPdoDupNode (avis);
        AVIS_DECL (new_avis) = INFO_ARGS (arg_info);
        INFO_RETEXPRS (arg_info) = TCappendExprs (TBmakeExprs (TBmakeId (new_avis), NULL),
                                                  INFO_RETEXPRS (arg_info));

        GENARRAY_NEXT (arg_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUKNLmodarray( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUKNLmodarray (node *arg_node, info *arg_info)
{
    node *avis, *new_avis;

    DBUG_ENTER ("CUKNLmodarray");

    if (INFO_COLLECT (arg_info)) {
        MODARRAY_SUB (arg_node) = TRAVopt (MODARRAY_SUB (arg_node), arg_info);
        MODARRAY_MEM (arg_node) = TRAVopt (MODARRAY_MEM (arg_node), arg_info);

        avis = ID_AVIS (MODARRAY_MEM (arg_node));

        /* The memval in N_modarray is the return value */
        INFO_RETS (arg_info)
          = TCappendRet (TBmakeRet (TYeliminateAKV (AVIS_TYPE (DUPdoDupNode (avis))),
                                    NULL),
                         INFO_RETS (arg_info));

        /* Set the linksign to the linksign of the return memval */
        RET_LINKSIGN (INFO_RETS (arg_info)) = ARG_LINKSIGN (INFO_ARGS (arg_info));
        RET_HASLINKSIGNINFO (INFO_RETS (arg_info)) = TRUE;

        /* Set the declaration of the return N_id to the memval argument */
        new_avis = DUPdoDupNode (avis);
        AVIS_DECL (new_avis) = INFO_ARGS (arg_info);
        INFO_RETEXPRS (arg_info) = TCappendExprs (TBmakeExprs (TBmakeId (new_avis), NULL),
                                                  INFO_RETEXPRS (arg_info));

        MODARRAY_NEXT (arg_node) = TRAVopt (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUKNLwithid( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUKNLwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUKNLwithid");

    if (INFO_COLLECT (arg_info)) {
        INFO_WLIDS (arg_info) = WITHID_IDS (arg_node);

        /* Traverse order of N_withid->ids and N_with->idxs must
         * not be changed because N_with->idxs need N_withid->ids */
        ids_dim = 0;
        INFO_IS_WLIDS (arg_info) = TRUE;
        WITHID_IDS (arg_node) = TRAVopt (WITHID_IDS (arg_node), arg_info);
        INFO_IS_WLIDS (arg_info) = FALSE;

        INFO_IS_WLIDXS (arg_info) = TRUE;
        WITHID_IDXS (arg_node) = TRAVopt (WITHID_IDXS (arg_node), arg_info);
        INFO_IS_WLIDXS (arg_info) = FALSE;

        INFO_WLIDS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUKNLcode( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUKNLcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUKNLcode");

    if (INFO_COLLECT (arg_info)) {
        CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUKNLpart( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUKNLpart (node *arg_node, info *arg_info)
{
    node *cuda_kernel, *cuda_funap;
    node *old_ids, *code;

    DBUG_ENTER ("CUKNLpart");

    if (INFO_COLLECT (arg_info)) {
        INFO_PART (arg_info) = arg_node;
        /* For each cudarizable partition, we create a CUDA kernel */
        if (PART_CUDARIZABLE (arg_node)) {
            INFO_LUT (arg_info) = LUTgenerateLut ();

            INFO_INWITHOP (arg_info) = TRUE;
            INFO_WITHOP (arg_info) = TRAVdo (INFO_WITHOP (arg_info), arg_info);
            INFO_INWITHOP (arg_info) = FALSE;

            old_ids = INFO_LETIDS (arg_info);

            PART_WITHID (arg_node) = TRAVopt (PART_WITHID (arg_node), arg_info);
            code = DUPdoDupNode (PART_CODE (arg_node));
            code = TRAVopt (code, arg_info);
            PART_GENERATOR (arg_node) = TRAVopt (PART_GENERATOR (arg_node), arg_info);

            INFO_LETIDS (arg_info) = old_ids;

            cuda_kernel = CreateCudaKernelDef (code, arg_info);
            FUNDEF_NEXT (cuda_kernel) = INFO_CUDAKERNELS (arg_info);
            INFO_CUDAKERNELS (arg_info) = cuda_kernel;

            cuda_funap
              = TBmakeAssign (TBmakeLet (DUPdoDupTree (INFO_LETIDS (arg_info)),
                                         TBmakeAp (cuda_kernel, INFO_PARAMS (arg_info))),
                              NULL);
            INFO_CUDAAPS (arg_info)
              = TCappendAssign (INFO_PRFGRIDBLOCK (arg_info),
                                TCappendAssign (cuda_funap, INFO_CUDAAPS (arg_info)));

            INFO_ARGS (arg_info) = NULL;
            INFO_PARAMS (arg_info) = NULL;
            INFO_RETS (arg_info) = NULL;
            INFO_RETEXPRS (arg_info) = NULL;
            INFO_PRFGRIDBLOCK (arg_info) = NULL;
            INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));
        } else {
            /* For non-cudarizable partition, we traverse its code
             * and create a <device2device>. Note that we only traverse
             * the first non-cudarizable partition encountered since
             * for all non-cudarizable partition, only one <device2device>
             * is needed. */
            if (PART_CODE (arg_node) != NULL && INFO_D2DTRANSFER (arg_info) == NULL) {
                old_ids = INFO_LETIDS (arg_info);
                PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);
                INFO_LETIDS (arg_info) = old_ids;
            }
        }
        PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUKNLgenerator( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUKNLgenerator (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUKNLgenerator");

    node *lower_bound, *upper_bound;
    node *step, *width;
    node *gridblock_exprs = NULL;

    if (INFO_COLLECT (arg_info)) {
        lower_bound = GENERATOR_BOUND1 (arg_node);
        upper_bound = GENERATOR_BOUND2 (arg_node);

        HandleBoundStepWidthExprs (lower_bound, &gridblock_exprs, "_lb_", arg_info);
        HandleBoundStepWidthExprs (upper_bound, &gridblock_exprs, "_ub_", arg_info);

        INFO_PRFGRIDBLOCK (arg_info)
          = TBmakeAssign (TBmakeLet (NULL,
                                     TBmakePrf (F_cuda_grid_block, gridblock_exprs)),
                          NULL);

        step = GENERATOR_STEP (arg_node);
        width = GENERATOR_WIDTH (arg_node);
        if (step != NULL && width != NULL) {
            INFO_HASSTEPWIDTH (arg_info) = TRUE;
            HandleBoundStepWidthExprs (step, NULL, "_step_", arg_info);
            HandleBoundStepWidthExprs (width, NULL, "_width_", arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUKNLprf( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUKNLprf (node *arg_node, info *arg_info)
{
    node *args, *ret_node;

    DBUG_ENTER ("CUKNLprf");

    if (INFO_COLLECT (arg_info)) {
        switch (PRF_PRF (arg_node)) {
        case F_wl_assign:
            if (PART_CUDARIZABLE (INFO_PART (arg_info))) {
                /* Replace the F_wlassign by a new F_cuda_wlassign which
                 * doesn't contain N_withid->vec */
                args = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                                    TBmakeExprs (DUPdoDupNode (PRF_ARG2 (arg_node)),
                                                 TBmakeExprs (DUPdoDupNode (
                                                                PRF_ARG4 (arg_node)),
                                                              NULL)));
                ret_node = TBmakePrf (F_cuda_wl_assign, args);
                arg_node = FREEdoFreeTree (arg_node);
                PRF_ARGS (ret_node) = TRAVdo (PRF_ARGS (ret_node), arg_info);
            } else {
                /* We only create a <device2device> primitive if this is the
                 * first non-cudarizable partition we come across. */
                if (INFO_D2DTRANSFER (arg_info) == NULL) {
                    INFO_D2DTRANSFER (arg_info)
                      = TBmakeAssign (TBmakeLet (TBmakeIds (ID_AVIS (PRF_ARG2 (arg_node)),
                                                            NULL),
                                                 TCmakePrf1 (F_device2device,
                                                             TBmakeId (INFO_D2DSOURCE (
                                                               arg_info)))),
                                      NULL);
                }
                ret_node = arg_node;
            }
            break;
        case F_idx_sel:
            if (PART_CUDARIZABLE (INFO_PART (arg_info))) {
                PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
            } else {
                /* If the N_part is not cudarizable, no CUDA kernel needs
                 * to be created. This partition simply copies array elements
                 * from a source array to a destination array. Here we store
                 * the N_avis of the source array and this will be used later
                 * to create the <device2device>. */
                INFO_D2DSOURCE (arg_info) = ID_AVIS (PRF_ARG2 (arg_node));
            }
            ret_node = arg_node;
            break;
        default:
            PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
            ret_node = arg_node;
            break;
        }
    } else {
        ret_node = arg_node;
    }

    DBUG_RETURN (ret_node);
}
