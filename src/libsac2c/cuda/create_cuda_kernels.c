/** <!--********************************************************************-->
 *
 * @defgroup Create CUDA kernel functions
 *
 *
 *
 *   This module creates CUDA kernel functions from cudarizable N_with
 *   nodes. Each partition is created as a CUDA kernel and the code of
 *   partition becomes the body the kernel function. For exmaple:
 *
 *   in_var1 = ...
 *   in_var2 = ...
 *   a_mem = with
 *           {
 *             [l0,l1,l2] <= iv = [eat1, eat2, eat3](IDXS:_wlidx) < [u0,u1,u2]
 *             {
 *               ... = var1;
 *               local_var1 = ...;
 *               ...
 *               res1 = ...;
 *             }:res1;
 *             [l3,l4,l5] <= iv = [eat1, eat2, eat3](IDXS:_wlidx) < [u3,u4,u5]
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
 *   _cuda_grid_block_( u0, u1, u2, l0, l1, l2);
 *   a_mem = CUDA_kernel1( a_mem, l0, l1, l2, u0, u1, u2, var1);
 *   _cuda_grid_block_( u0, u1, u2, l0, l1, l2);
 *   a_mem = CUDA_kernel2( a_mem, l3, l4, l5, u3, u4, u5, var2);
 *
 *   dev[*] CUDA_kernel1( a_mem, l0, l1, l2, u0, u1, u2, var1)
 *   {
 *     _cuda_wlids_( eat1);
 *     _cuda_wlids_( eat2);
 *     _cuda_wlids_( eat3);
 *     _cuda_wlidxs_( _wlidx, a_mem, eat1, eat2, eat3);
 *
 *     ... = var1;
 *     local_var1 = ...;
 *     ...
 *     res1 = ...;
 *     return a_mem;
 *   }
 *
 *   dev[*] CUDA_kernel2( a_mem, l3, l4, l5, u3, u4, u5, var2)
 *   {
 *     _cuda_wlids_( eat1);
 *     _cuda_wlids_( eat2);
 *     _cuda_wlids_( eat3);
 *     _cuda_wlidxs_( _wlidx, a_mem, eat1, eat2, eat3);
 *
 *     ... = var2;
 *     local_var2 = ...;
 *     ...
 *     res2 = ...;
 *     return a_mem;
 *   }
 *
 *   Note that in the trasformed code, serveral primitives are also
 *   inserted. F_cuda_grid_block inserted before CUDA kernel applicatino
 *   is used to create the CUDA kernel configuration parameters in
 *   the backend. F_cuda_wlids and F_cuda_wlidxs inserted at the beginning
 *   of each CUDA kernel are use to generate the correct index value
 *   in the backend.
 *
 * @ingroup
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file create_cuda_kernels.c
 *
 * Prefix: CUKNL
 *
 *****************************************************************************/
#include "create_cuda_kernels.h"

#define DBUG_PREFIX "CUKNL"
#include "debug.h"

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
#include "cuda_utils.h"
#include "kernel_post_processing.h"
#include "adjust_shmem_access.h"
#include "expand_shmem_boundary_load.h"
#include "globals.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
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
    bool collect :1;
    node *allocassigns;
    node *freeassigns;
    node *prfwlids;
    node *prfwlidxs;
    node *prfgridblock;
    bool hasstepwidth :1;
    node *part;
    node *d2dsource;
    node *d2dtransfer;
    lut_t *lut;
    int ls_num;
    bool lift_done :1;
    node *with;
    bool in_withop :1;
    node *withop;
    bool trav_mem :1;
    bool suballoc_rhs :1;
    node *suballoc_lhs;
    bool in_cuda_partition :1;
    node *part_tbshp;
    node *pragma;
};

#define INFO_CUDAKERNELS(n) (n->cudakernels)   // collects the new kernel fundefs
#define INFO_CUDAAPS(n) (n->cudaaps)
#define INFO_LETIDS(n) (n->letids)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_ARGS(n) (n->args)
#define INFO_PARAMS(n) (n->params)
#define INFO_VARDECS(n) (n->vardecs)
#define INFO_RETS(n) (n->rets)
#define INFO_RETEXPRS(n) (n->retexprs)
#define INFO_IN_CUDA_WL(n) (n->collect)        // are we inside a to-be-lifted WL?
#define INFO_ALLOCASSIGNS(n) (n->allocassigns)
#define INFO_FREEASSIGNS(n) (n->freeassigns)
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
#define INFO_TRAVMEM(n) (n->trav_mem)
#define INFO_SUBALLOC_RHS(n) (n->suballoc_rhs)
#define INFO_SUBALLOC_LHS(n) (n->suballoc_lhs)
#define INFO_IN_CUDA_PARTITION(n) (n->in_cuda_partition)
#define INFO_WITH(n) (n->with)
#define INFO_PART_TBSHP(n) (n->part_tbshp)
#define INFO_PRAGMA(n) (n->pragma)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_CUDAKERNELS (result) = NULL;
    INFO_CUDAAPS (result) = NULL;
    INFO_LETIDS (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_ARGS (result) = NULL;
    INFO_PARAMS (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_RETS (result) = NULL;
    INFO_RETEXPRS (result) = NULL;
    INFO_IN_CUDA_WL (result) = FALSE;
    INFO_ALLOCASSIGNS (result) = NULL;
    INFO_FREEASSIGNS (result) = NULL;
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
    INFO_TRAVMEM (result) = FALSE;
    INFO_SUBALLOC_RHS (result) = FALSE;
    INFO_SUBALLOC_LHS (result) = FALSE;
    INFO_IN_CUDA_PARTITION (result) = FALSE;
    INFO_WITH (result) = NULL;
    INFO_PART_TBSHP (result) = NULL;
    INFO_PRAGMA (result) = NULL;

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
 * @fn node *
 *
 *****************************************************************************/
node *
CUKNLdoCreateCudaKernels (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module, "Illegal argument node!");

    syntax_tree = ASHAdoAdjustShmemAccess (syntax_tree);

    info = MakeInfo ();

    TRAVpush (TR_cuknl);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    syntax_tree = KPPdoKernelPostProcessing (syntax_tree);
    syntax_tree = ESBLdoExpandShmemBoundaryLoad (syntax_tree);

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
 *
 * @fn static void  SetLinksignInfo( node *args, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
static void
SetLinksignInfo (node *args, info *arg_info)
{
    node *iter;

    DBUG_ENTER ();

    iter = args;

    while (iter != NULL) {
        if (!ARG_HASLINKSIGNINFO (iter)) {
            ARG_LINKSIGN (iter) = INFO_LS_NUM (arg_info);
            ARG_HASLINKSIGNINFO (iter) = TRUE;
            INFO_LS_NUM (arg_info) += 1;
        }
        iter = ARG_NEXT (iter);
    }

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn static node *HandleBoundStepWidthExprs( node *expr,
 *                                             node **gridblock_exprs,
 *                                             char *name,
 *                                             info *arg_info)
 *
 * @param expr the AST of one of the four generator expressions
 * @param gridblock_exprs location of an N_exprs chain of nodes that serves
 *                        as parameters to F_cuda_grid_block, if NULL,
 *                        the parameter is not to be insert!
 * @param name string prefix, one of ("_lb_", "_ub_", "_step_", "_width")
 * @param arg_info
 *
 * @brief 1) assembles arguments to F_cuda_grid_block in *gridblock_exprs,
 *           PROVIDED gridblock_exprs is not NULL
 *        2) generates N_exprs-chain for kernel function call => INFO_PARAMS
 *        3) generates N_arg-chain for kernel function definition => INFO_ARGS
 *
 *****************************************************************************/
static node *
HandleBoundStepWidthExprs (node *expr, node **gridblock_exprs, char *name, info *arg_info)
{
    node *elements;
    node *avis, *new_avis;
    char *bound_name;
    int dim = 0;
    bool insert_in_grid_block;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (expr) == N_array, "Expr in not a N_array!");

    /* If the expression list passed in is lower/bound elements,
     * we need to update 'gridblock_exprs'
     */
    insert_in_grid_block = (gridblock_exprs != NULL);
    elements = ARRAY_AELEMS (expr);

    while (elements != NULL) {
        DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (elements)) == N_id,
                     "Should be an array of N_id nodes");

        if (INFO_IN_CUDA_PARTITION (arg_info)) {
            EXPRS_EXPR (elements) = TRAVopt (EXPRS_EXPR (elements), arg_info);
        } else {
            avis = ID_AVIS (EXPRS_EXPR (elements));

            // add old LUSW component to the parameters in kernel call:
            INFO_PARAMS (arg_info)
              = TBmakeExprs (TBmakeId (avis), INFO_PARAMS (arg_info));

            // construct fresh N_arg for the kernel function:
            new_avis = DUPdoDupNode (avis);
            AVIS_NAME (new_avis) = MEMfree (AVIS_NAME (new_avis));
            bound_name = (char *)MEMmalloc (sizeof (char) * (STRlen (name) + 2));
            sprintf (bound_name, "%s%d", name, dim);
            AVIS_NAME (new_avis) = bound_name;
            INFO_ARGS (arg_info) = TBmakeArg (new_avis, INFO_ARGS (arg_info));

            if (insert_in_grid_block) {
                // insert LUSW component into F_cuda_grid_block arguments:
                (*gridblock_exprs) = TBmakeExprs (TBmakeId (avis), (*gridblock_exprs));
            }
        }
        elements = EXPRS_NEXT (elements);
        dim++;
    }

    DBUG_RETURN (expr);
}

/** <!--********************************************************************-->
 *
 * @fn static node* CreateCudaKernelDef( node *code, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
static node *
CreateCudaKernelDef (node *code, info *arg_info)
{
    node *cuda_kerneldef, *body, *vardecs, *args, *retur;
    node *allocassigns, *freeassigns;
    node *prfwlids, *prfwlidxs;

    DBUG_ENTER ();

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

    BLOCK_ASSIGNS (body) = TCappendAssign (
      TCappendAssign (TCappendAssign (TCappendAssign (TCappendAssign (allocassigns,
                                                                      prfwlids),
                                                      prfwlidxs),
                                      BLOCK_ASSIGNS (body)),
                      freeassigns),
      TBmakeAssign (retur, NULL));

    BLOCK_VARDECS (body) = TCappendVardec (vardecs, BLOCK_VARDECS (body));

    cuda_kerneldef = TBmakeFundef (TRAVtmpVarName ("CUDA"),
                                   NSdupNamespace (FUNDEF_NS (INFO_FUNDEF (arg_info))),
                                   INFO_RETS (arg_info), args, body, NULL);

    FUNDEF_ISCUDAGLOBALFUN (cuda_kerneldef) = TRUE;
    FUNDEF_HASSTEPWIDTHARGS (cuda_kerneldef) = INFO_HASSTEPWIDTH (arg_info);
    INFO_HASSTEPWIDTH (arg_info) = FALSE;
    FUNDEF_RETURN (cuda_kerneldef) = retur;

    INFO_CUDAKERNELS (arg_info)
      = TCappendFundef (cuda_kerneldef, INFO_CUDAKERNELS (arg_info));

    DBUG_RETURN (INFO_CUDAKERNELS (arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn void CreateAllocAndFree( node *avis, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
static void
CreateAllocAndFree (node *avis, info *arg_info)
{
    node *alloc, *free, *dim, *shape;

    DBUG_ENTER ();

    /*
      if( TUdimKnown( AVIS_TYPE( avis))) {
        dim = TBmakeNum( TYgetDim( AVIS_TYPE( avis)));
      }

      if( TUshapeKnown( AVIS_TYPE( avis))) {
        shape = SHshape2Array( TYgetShape( AVIS_TYPE( avis)));
      }
    */

    DBUG_ASSERT (TUdimKnown (AVIS_TYPE (avis)), "Dimension is not known!");
    dim = TBmakeNum (TYgetDim (AVIS_TYPE (avis)));

    DBUG_ASSERT (TUdimKnown (AVIS_TYPE (avis)), "Shape is not known!");
    shape = SHshape2Array (TYgetShape (AVIS_TYPE (avis)));

    /* Create F_alloc and F_free for N_withid->ids and N_withid->idxs */
    alloc = TCmakePrf3 (F_alloc, TBmakeNum (1), dim, shape);
    INFO_ALLOCASSIGNS (arg_info)
      = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), alloc),
                      INFO_ALLOCASSIGNS (arg_info));

    free = TCmakePrf1 (F_free, TBmakeId (avis));
    INFO_FREEASSIGNS (arg_info)
      = TBmakeAssign (TBmakeLet (NULL, free), INFO_FREEASSIGNS (arg_info));

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn node *PreprocessWithid( node *id, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
static node *
PreprocessWithid (node *id, info *arg_info)
{
    node *avis, *new_avis;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (id) == N_id,
                 "Non N_id node found in N_withid->ids or N_withid->idxs!");
    avis = ID_AVIS (id);
    new_avis = DUPdoDupNode (avis);
    INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), avis, new_avis);

    INFO_VARDECS (arg_info) = TBmakeVardec (new_avis, INFO_VARDECS (arg_info));

    DBUG_RETURN (new_avis);
}

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
 * @fn node *CUKNLfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CUKNLfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (!FUNDEF_ISCUDAGLOBALFUN (arg_node)) {
        /* Only Non CUDA kernels may contain cudarizable N_with */
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = NULL;
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
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
 * @fn node *CUKNLassign( node *arg_node, info *arg_info)
 *
 * @brief  For cudarizable N_with, remove the old N_assign and insert
 *         the newly created CUDA kernel N_aps into the assign chain.
 *
 *****************************************************************************/
node *
CUKNLassign (node *arg_node, info *arg_info)
{
    node *next, *new_lastassign;

    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);

    /* If we are not in collect mode and the RHS has been
     * lifted as a chain of CUDA kernel applications */
    if (!INFO_IN_CUDA_WL (arg_info) && INFO_LIFTDONE (arg_info)) {
        /* Free the current N_assign and store the
         * next N_assign in 'next' */
        next = FREEdoFreeNode (arg_node);

        /* If there is a <device2device> to be prepended */
        if (INFO_D2DTRANSFER (arg_info) != NULL) {
            arg_node
              = TCappendAssign (INFO_D2DTRANSFER (arg_info), INFO_CUDAAPS (arg_info));
        } else {
            arg_node = INFO_CUDAAPS (arg_info);
        }

        new_lastassign = arg_node;
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
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUKNLdo( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CUKNLdo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* We do not traverse SKIP son */
    DO_BODY (arg_node) = TRAVopt (DO_BODY (arg_node), arg_info);
    DO_COND (arg_node) = TRAVopt (DO_COND (arg_node), arg_info);
    DO_SKIP (arg_node) = TRAVopt (DO_SKIP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUKNLlet( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CUKNLlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    INFO_LETIDS (arg_info) = NULL;

    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUKNLwith( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CUKNLwith (node *arg_node, info *arg_info)
{
    node *old_with, *old_pragma;
    DBUG_ENTER ();

    if (WITH_CUDARIZABLE (arg_node)) {
        /*
         * push new arg_info stuff:
         */
        INFO_IN_CUDA_WL (arg_info) = TRUE; // signal cudarisation!
        old_pragma = INFO_PRAGMA (arg_info);
        INFO_PRAGMA (arg_info) = WITH_PRAGMA (arg_node);
        INFO_WITHOP (arg_info) = WITH_WITHOP (arg_node);
        old_with = INFO_WITH (arg_info);
        INFO_WITH (arg_info) = arg_node;

        WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);

        /*
         * restore old arg_info stuff:
         */
        INFO_WITH (arg_info) = old_with;
        INFO_PRAGMA (arg_info) = old_pragma;
        INFO_WITHOP (arg_info) = NULL;
        INFO_IN_CUDA_WL (arg_info) = FALSE; // cudarisation done!

        /*
         * Indicate to N_assign that a chain of CUDA kernel N_aps
         * has been created and needs to be inserted into the AST
         */
        INFO_LIFTDONE (arg_info) = TRUE;

    } else if (INFO_IN_CUDA_PARTITION (arg_info)) {

        old_with = INFO_WITH (arg_info);
        INFO_WITH (arg_info) = arg_node;
        WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
        INFO_WITH (arg_info) = old_with;
        // WITH_CODE( arg_node) = TRAVopt( WITH_CODE( arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUKNLwith2( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CUKNLwith2 (node *arg_node, info *arg_info)
{
    node *old_with;

    DBUG_ENTER ();

    old_with = INFO_WITH (arg_info);
    INFO_WITH (arg_info) = arg_node;

    WITH2_WITHID (arg_node) = TRAVdo (WITH2_WITHID (arg_node), arg_info);
    WITH2_SEGS (arg_node) = TRAVdo (WITH2_SEGS (arg_node), arg_info);
    WITH2_CODE (arg_node) = TRAVopt (WITH2_CODE (arg_node), arg_info);
    WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);

    INFO_WITH (arg_info) = old_with;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUKNLpart( node *arg_node, info *arg_info)
 *
 * @brief Traverse each N_part (including the withop of the enclosing N_with)
 *        and create both CUDA kernel and kernel application for it.
 *
 *****************************************************************************/
node *
CUKNLpart (node *arg_node, info *arg_info)
{
    node *cuda_kernel, *cuda_funap;
    node *old_ids, *dup_code;

    DBUG_ENTER ();

    if (INFO_IN_CUDA_WL (arg_info)) {
        INFO_PART (arg_info) = arg_node;
        /* For each cudarizable partition, we create a CUDA kernel */
        /*
            if( ( !global.optimize.dopra && PART_CUDARIZABLE( arg_node)) ||
                ( global.optimize.dopra && !PART_ISCOPY( arg_node) && PART_CUDARIZABLE(
           arg_node))) {
        */
        if ((!WITH_HASRC (INFO_WITH (arg_info)) || !PART_ISCOPY (arg_node))
            && PART_CUDARIZABLE (arg_node)) {
            /* We create a lookup table for the traversal of each partition */
            INFO_LUT (arg_info) = LUTgenerateLut ();

            /* Since each CUDA kernel created from an N_part may
             * potentially need the sons of withop as arguments and
             * it's created independently, traversal of each N_part
             * must also traverse the withop associated with the
             * N_with. */
            INFO_INWITHOP (arg_info) = TRUE;
            INFO_WITHOP (arg_info) = TRAVopt (INFO_WITHOP (arg_info), arg_info);
            INFO_INWITHOP (arg_info) = FALSE;

            old_ids = INFO_LETIDS (arg_info);

            /********* Begin traversal of N_part Sons/Attributes *********/

            PART_WITHID (arg_node) = TRAVopt (PART_WITHID (arg_node), arg_info);

            /* Since each CUDA kernel contains the code originally in each
             * N_part and since N_code can be shared between more than one
             * N_part, we duplicate it before traversing into it */
            dup_code = DUPdoDupNode (PART_CODE (arg_node));
            INFO_IN_CUDA_PARTITION (arg_info) = TRUE;
            dup_code = TRAVopt (dup_code, arg_info);
            INFO_IN_CUDA_PARTITION (arg_info) = FALSE;

            INFO_PART_TBSHP (arg_info) = PART_THREADBLOCKSHAPE (arg_node);
            PART_GENERATOR (arg_node) = TRAVopt (PART_GENERATOR (arg_node), arg_info);
            INFO_PART_TBSHP (arg_info) = NULL;

            /********** End traversal of N_part Sons/Attributes **********/

            INFO_LETIDS (arg_info) = old_ids;

            /****** Begin creating CUDA kernel and its application ******/

            cuda_kernel = CreateCudaKernelDef (dup_code, arg_info);

            cuda_funap
              = TBmakeAssign (TBmakeLet (DUPdoDupTree (INFO_LETIDS (arg_info)),
                                         TBmakeAp (cuda_kernel, INFO_PARAMS (arg_info))),
                              NULL);

            /* Each CUDA kernel N_ap is preceeded by a primitive F_grid_block.
             * This is used in the later code generation to create the correct
             * CUDA configuration parameters, i.e. shape of grid and block. */
            INFO_CUDAAPS (arg_info)
              = TCappendAssign (INFO_PRFGRIDBLOCK (arg_info),
                                TCappendAssign (cuda_funap, INFO_CUDAAPS (arg_info)));

            /******* End creating CUDA kernel and its application *******/

            /* Clean up */
            INFO_ARGS (arg_info) = NULL;
            INFO_PARAMS (arg_info) = NULL;
            INFO_RETS (arg_info) = NULL;
            INFO_RETEXPRS (arg_info) = NULL;
            INFO_PRFGRIDBLOCK (arg_info) = NULL;
            INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));
        } else if (INFO_IN_CUDA_PARTITION (arg_info)) {
            PART_WITHID (arg_node) = TRAVopt (PART_WITHID (arg_node), arg_info);
            PART_GENERATOR (arg_node) = TRAVopt (PART_GENERATOR (arg_node), arg_info);
            PART_CODE (arg_node) = TRAVopt (PART_CODE (arg_node), arg_info);
        } else {
            /* For non-cudarizable partition, we traverse its code
             * and create a <device2device>. Note that we only traverse
             * the first non-cudarizable partition encountered since
             * for all non-cudarizable partition, only one <device2device>
             * is needed. */
            if (PART_CODE (arg_node) != NULL && INFO_D2DTRANSFER (arg_info) == NULL) {
                old_ids = INFO_LETIDS (arg_info);
                // PART_CODE( arg_node) = TRAVopt( PART_CODE( arg_node), arg_info);
                INFO_LETIDS (arg_info) = old_ids;
            }
        }
        PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUKNLgenarray( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CUKNLgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_IN_CUDA_WL (arg_info)) {
        if (INFO_IN_CUDA_PARTITION (arg_info)) {
            /* Shape needs to be traversed as well. */
            GENARRAY_SHAPE (arg_node) = TRAVopt (GENARRAY_SHAPE (arg_node), arg_info);
            GENARRAY_MEM (arg_node) = TRAVopt (GENARRAY_MEM (arg_node), arg_info);
            GENARRAY_DEFAULT (arg_node) = TRAVopt (GENARRAY_DEFAULT (arg_node), arg_info);
            GENARRAY_IDX (arg_node)
              = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), GENARRAY_IDX (arg_node));
        } else {
            GENARRAY_DEFAULT (arg_node) = TRAVopt (GENARRAY_DEFAULT (arg_node), arg_info);
            INFO_TRAVMEM (arg_info) = TRUE;
            GENARRAY_MEM (arg_node) = TRAVopt (GENARRAY_MEM (arg_node), arg_info);
            INFO_TRAVMEM (arg_info) = FALSE;
        }
        GENARRAY_NEXT (arg_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUKNLmodarray( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CUKNLmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_IN_CUDA_WL (arg_info)) {
        if (INFO_IN_CUDA_PARTITION (arg_info)) {
            MODARRAY_MEM (arg_node) = TRAVopt (MODARRAY_MEM (arg_node), arg_info);
            MODARRAY_IDX (arg_node)
              = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), MODARRAY_IDX (arg_node));
        } else {
            INFO_TRAVMEM (arg_info) = TRUE;
            MODARRAY_MEM (arg_node) = TRAVopt (MODARRAY_MEM (arg_node), arg_info);
            INFO_TRAVMEM (arg_info) = FALSE;
        }
        MODARRAY_NEXT (arg_node) = TRAVopt (MODARRAY_NEXT (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUKNLfold( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CUKNLfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_IN_CUDA_WL (arg_info)) {
        if (INFO_IN_CUDA_PARTITION (arg_info)) {
            /* Shape needs to be traversed as well. */
            FOLD_NEUTRAL (arg_node) = TRAVopt (FOLD_NEUTRAL (arg_node), arg_info);
            FOLD_INITIAL (arg_node) = TRAVopt (FOLD_INITIAL (arg_node), arg_info);
            FOLD_PARTIALMEM (arg_node) = TRAVopt (FOLD_PARTIALMEM (arg_node), arg_info);
        } else {
            FOLD_NEUTRAL (arg_node) = TRAVopt (FOLD_NEUTRAL (arg_node), arg_info);
            FOLD_INITIAL (arg_node) = TRAVopt (FOLD_INITIAL (arg_node), arg_info);
            INFO_TRAVMEM (arg_info) = TRUE;
            FOLD_PARTIALMEM (arg_node) = TRAVopt (FOLD_PARTIALMEM (arg_node), arg_info);
            INFO_TRAVMEM (arg_info) = FALSE;
        }
        FOLD_NEXT (arg_node) = TRAVopt (FOLD_NEXT (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUKNLwithid( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CUKNLwithid (node *arg_node, info *arg_info)
{
    node *wlids, *wlidxs, *wlvec, *id, *withop;
    node *new_wlids = NULL;
    node *new_avis, *iv_new_avis;
    int dim, iter = 0;
    node *prf_wlids, *prf_wlidxs, *prf_wlidxs_args = NULL;

    DBUG_ENTER ();

    wlids = WITHID_IDS (arg_node);
    wlidxs = WITHID_IDXS (arg_node);
    wlvec = WITHID_VEC (arg_node);
    withop = INFO_WITHOP (arg_info);

    if (INFO_IN_CUDA_WL (arg_info)) {

        if (INFO_IN_CUDA_PARTITION (arg_info)) {
            WITHID_IDS (arg_node) = TRAVopt (WITHID_IDS (arg_node), arg_info);
            WITHID_IDXS (arg_node) = TRAVopt (WITHID_IDXS (arg_node), arg_info);
            if (LUTsearchInLutPp (INFO_LUT (arg_info), ID_AVIS (WITHID_VEC (arg_node)))
                == ID_AVIS (WITHID_VEC (arg_node))) {
                /* We need to create N_vardec for the iv of the inner N_with2. However
                 * Due to the potential absence of alloc of iv, we need to deal with it
                 * specially here. */
                new_avis = PreprocessWithid (WITHID_VEC (arg_node), arg_info);
                ID_AVIS (WITHID_VEC (arg_node)) = new_avis;
            }
        } else {

            DBUG_ASSERT (NODE_TYPE (wlvec) == N_id,
                         "Non N_id node found in N_withid->vec!");
            /* Get the dimentionality of the N_with */
            dim = SHgetExtent (TYgetShape (AVIS_TYPE (ID_AVIS (wlvec))), 0);

            /* In the kernel, we statically create an array to store the
             * index vector of a withloop, e.g. int[3] iv;  */
            iv_new_avis = PreprocessWithid (WITHID_VEC (arg_node), arg_info);

            while (wlids != NULL) {
                id = EXPRS_EXPR (wlids);
                new_avis = PreprocessWithid (id, arg_info);
                CreateAllocAndFree (new_avis, arg_info);

                /* Create primitives F_cuda_wlids */
                prf_wlids = TCmakePrf3 (F_cuda_wlids, TBmakeNum (iter), TBmakeNum (dim),
                                        TBmakeId (iv_new_avis));
                INFO_PRFWLIDS (arg_info)
                  = TBmakeAssign (TBmakeLet (TBmakeIds (new_avis, NULL), prf_wlids),
                                  INFO_PRFWLIDS (arg_info));

                /* Build an N_exprs containing all N_id in N_withid->ids,
                 * each N_ids contain the new avis. This N_exprs will be
                 * used to construct primitive F_cuda_wlidxs (See below) */
                new_wlids
                  = TCappendExprs (new_wlids, TBmakeExprs (TBmakeId (new_avis), NULL));
                iter++;
                wlids = EXPRS_NEXT (wlids);
            }

            while (wlidxs != NULL && withop != NULL) {
                id = EXPRS_EXPR (wlidxs);
                new_avis = PreprocessWithid (id, arg_info);
                CreateAllocAndFree (new_avis, arg_info);

                node *mem_id = WITHOP_MEM (withop);
                DBUG_ASSERT (NODE_TYPE (mem_id) == N_id,
                             "Non N_id node found in withop->mem");
                node *mem_avis = ID_AVIS (mem_id);
                node *new_mem_avis
                  = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), mem_avis);
                DBUG_ASSERT (new_mem_avis != mem_avis,
                             "Withop->mem has not been traversed before!");

                if (TYisAKS (AVIS_TYPE (new_mem_avis))) {
                    prf_wlidxs_args = TBmakeExprs (SHshape2Array (TYgetShape (
                                                     AVIS_TYPE (new_mem_avis))),
                                                   DUPdoDupTree (new_wlids));

                    /* Create primitives F_array_idxs2offset */
                    prf_wlidxs = TBmakePrf (F_idxs2offset, prf_wlidxs_args);
                } else {
                    prf_wlidxs_args
                      = TBmakeExprs (TBmakeId (new_mem_avis), DUPdoDupTree (new_wlids));

                    /* Create primitives F_array_idxs2offset */
                    prf_wlidxs = TBmakePrf (F_array_idxs2offset, prf_wlidxs_args);
                }

                INFO_PRFWLIDXS (arg_info)
                  = TBmakeAssign (TBmakeLet (TBmakeIds (new_avis, NULL), prf_wlidxs),
                                  INFO_PRFWLIDXS (arg_info));

                wlidxs = EXPRS_NEXT (wlidxs);
                withop = WITHOP_NEXT (withop);
                DBUG_ASSERT (((wlidxs == NULL && withop == NULL)
                              || (wlidxs != NULL && withop != NULL)),
                             "#withop != #N_withid->wlidxs!");
            }
            new_wlids = FREEdoFreeTree (new_wlids);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUKNLcode( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CUKNLcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* We only traverse N_code if we are in collect mode */
    // if( INFO_IN_CUDA_WL( arg_info)) {
    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);
    //}

    if (INFO_IN_CUDA_PARTITION (arg_info)) {
        CODE_CEXPRS (arg_node) = TRAVopt (CODE_CEXPRS (arg_node), arg_info);
    }

    if (NODE_TYPE (INFO_WITH (arg_info)) == N_with2) {
        CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUKNLgenerator( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CUKNLgenerator (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    node *lower_bound, *upper_bound;
    node *step, *width;
    node *gridblock_exprs = NULL;

    if (INFO_IN_CUDA_WL (arg_info)) {
        lower_bound = GENERATOR_BOUND1 (arg_node);
        upper_bound = GENERATOR_BOUND2 (arg_node);

        HandleBoundStepWidthExprs (lower_bound, &gridblock_exprs, "_lb_", arg_info);
        HandleBoundStepWidthExprs (upper_bound, &gridblock_exprs, "_ub_", arg_info);

        if (!INFO_IN_CUDA_PARTITION (arg_info)) {
#if 0
            /*
             * IMHO (sbs) this injects the block values!
             */
            if (INFO_PART_TBSHP (arg_info) != NULL) {
                gridblock_exprs
                  = TCappendExprs (gridblock_exprs, DUPdoDupTree (ARRAY_AELEMS (
                                                      INFO_PART_TBSHP (arg_info))));
            }
#endif

            if (INFO_PRAGMA (arg_info) != NULL) {
                gridblock_exprs = TBmakeExprs (DUPdoDupTree (INFO_PRAGMA (arg_info)),
                                               gridblock_exprs);
            }
            INFO_PRFGRIDBLOCK (arg_info)
              = TBmakeAssign (TBmakeLet (NULL,
                                         TBmakePrf (F_cuda_thread_space, gridblock_exprs)),
                              NULL);
        }

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
 * @fn node *CUKNLid( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CUKNLid (node *arg_node, info *arg_info)
{
    node *avis, *new_avis;

    DBUG_ENTER ();

    avis = ID_AVIS (arg_node);

    new_avis = NULL;

    DBUG_PRINT ("ENTER id %s", ID_NAME (arg_node));

    if (INFO_IN_CUDA_WL (arg_info)) {
        if (LUTsearchInLutPp (INFO_LUT (arg_info), avis) == avis
            && !CUisShmemTypeNew (AVIS_TYPE (avis))) {
            new_avis = DUPdoDupNode (avis);
            INFO_ARGS (arg_info) = TBmakeArg (new_avis, INFO_ARGS (arg_info));
            INFO_PARAMS (arg_info)
              = TBmakeExprs (TBmakeId (avis), INFO_PARAMS (arg_info));

            if (INFO_INWITHOP (arg_info)) {
                ARG_LINKSIGN (INFO_ARGS (arg_info)) = INFO_LS_NUM (arg_info);
                ARG_HASLINKSIGNINFO (INFO_ARGS (arg_info)) = TRUE;

                /* If we are traversing withop->Mem, create N_ret/N_return
                 * and set the linksign properly */
                if (INFO_TRAVMEM (arg_info)) {
                    /* The type of 'Mem' in withop is the return type */
                    INFO_RETS (arg_info)
                      = TCappendRet (TBmakeRet (TYeliminateAKV (AVIS_TYPE (new_avis)),
                                                NULL),
                                     INFO_RETS (arg_info));

                    /* Set the correct linksign value for N_ret */
                    RET_LINKSIGN (INFO_RETS (arg_info)) = INFO_LS_NUM (arg_info);
                    RET_HASLINKSIGNINFO (INFO_RETS (arg_info)) = TRUE;

                    /* Create a return N_id */
                    INFO_RETEXPRS (arg_info)
                      = TCappendExprs (TBmakeExprs (TBmakeId (new_avis), NULL),
                                       INFO_RETEXPRS (arg_info));
                }
                INFO_LS_NUM (arg_info) += 1;
            } else {
                /* We only set the new avis if the N_id is NOT in
                 * N_genarray/N_modarray.  */
                ID_AVIS (arg_node) = new_avis;
            }

            INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), avis, new_avis);
        } else {
            ID_AVIS (arg_node) = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), avis);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUKNLids( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CUKNLids (node *arg_node, info *arg_info)
{
    node *avis, *new_avis;

    DBUG_ENTER ();

    avis = IDS_AVIS (arg_node);

    DBUG_PRINT ("ENTER ids %s", IDS_NAME (arg_node));

    if (INFO_IN_CUDA_WL (arg_info)
        && (PART_CUDARIZABLE (INFO_PART (arg_info))
            || INFO_IN_CUDA_PARTITION (arg_info))) {
        /* Not come across before */
        if (LUTsearchInLutPp (INFO_LUT (arg_info), avis) == avis) {
            new_avis = DUPdoDupNode (avis);
            if (INFO_SUBALLOC_RHS (arg_info)) {
                if (!CUisDeviceTypeNew (AVIS_TYPE (new_avis))) {
                    ntype *scalar_type = TYgetScalar (AVIS_TYPE (new_avis));
                    simpletype sty = TYgetSimpleType (scalar_type);
                    scalar_type
                      = TYsetSimpleType (scalar_type, CUh2dSimpleTypeConversion (sty));
                }
                INFO_SUBALLOC_RHS (arg_info) = FALSE;
            }

            INFO_VARDECS (arg_info) = TBmakeVardec (new_avis, INFO_VARDECS (arg_info));
            AVIS_DECL (new_avis) = INFO_VARDECS (arg_info);
            INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), avis, new_avis);
            DBUG_PRINT (">>> ids %s added to LUT", IDS_NAME (arg_node));
        }
        IDS_AVIS (arg_node) = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), avis);
    }

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUKNLprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CUKNLprf (node *arg_node, info *arg_info)
{
    node *args, *ret_node;

    DBUG_ENTER ();

    if (INFO_IN_CUDA_WL (arg_info)) {
        switch (PRF_PRF (arg_node)) {
        case F_wl_assign:
            if (PART_CUDARIZABLE (INFO_PART (arg_info))
                || INFO_IN_CUDA_PARTITION (arg_info)) {
                /* Replace the F_wlassign by a new F_cuda_wlassign which
                 * doesn't contain N_withid->vec */
                if (PART_INNERWLIDXASSIGN (INFO_PART (arg_info)) != NULL) {
                    ID_AVIS (PRF_ARG4 (arg_node)) = IDS_AVIS (
                      ASSIGN_LHS (PART_INNERWLIDXASSIGN (INFO_PART (arg_info))));
                }

                node *mem_id = DUPdoDupNode (PRF_ARG2 (arg_node));
                args
                  = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                                 TBmakeExprs (mem_id, TBmakeExprs (DUPdoDupNode (
                                                                     PRF_ARG4 (arg_node)),
                                                                   NULL)));
                IDS_AVIS (INFO_LETIDS (arg_info)) = ID_AVIS (mem_id);
                ret_node = TBmakePrf (F_cuda_wl_assign, args);
                arg_node = FREEdoFreeTree (arg_node);
                PRF_ARGS (ret_node) = TRAVopt (PRF_ARGS (ret_node), arg_info);
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
            if (PART_CUDARIZABLE (INFO_PART (arg_info))
                || INFO_IN_CUDA_PARTITION (arg_info)) {
                PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
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
        case F_suballoc:
            PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
            INFO_SUBALLOC_RHS (arg_info) = TRUE;
            INFO_SUBALLOC_LHS (arg_info) = INFO_LETIDS (arg_info);
            ret_node = arg_node;
            break;
        default:
            PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
            ret_node = arg_node;
            break;
        }
    } else {
        ret_node = arg_node;
    }

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/

#undef DBUG_PREFIX
