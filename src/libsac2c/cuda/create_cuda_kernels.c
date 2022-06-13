/** <!--********************************************************************-->
 *
 * @defgroup Create CUDA kernel functions
 *
 *
 *
 *   This module creates CUDA kernel functions from cudarizable partititons
 *   (PART_CUDARIZABLE) of curaziable with loops (WITH_CUDARIZABLE).
 *   The with-loop is being replaced by a sequence of kernel function 
 *   calls that are preceded by by a call to the primitive function
 *   _cuda_thread_space_ which indicates how to create the thread-space
 *   from the generator bounds.
 *
 *   _cuda_thread_space_ takes a gpukernel pragma as first argument, followed
 *   by all generator bounds (lb, ub, step, width). In case step and width are
 *   not present, N_num(1) nodes will be injected.
 *   It is assumed here that all cudarizable WL partitions do have gpukernel
 *   pragmas already attached to them. These either need to be annotated by the
 *   programmer or they are inferred by some heuristics in a traversal named
 *   IGKP (infer_gpukernel_pragmas).
 *
 *   For exmaple:
 *
 *   int    l0;
 *   int{5} l1;
 *   int    l2;
 *   int{4} u0;
 *   int    u1;
 *   int    u2;
 *   ...
 *   var1 = ...
 *   a_mem = with {
 *             ( [l0,l1,l2] <= iv = [eat1, eat2, eat3](IDXS:_wlidx) < [u0,u1,u2])
 *             #pragma gpukernel ShiftLB ( Gen)
 *             {
 *               ... = var1;
 *               local_var1 = ...;
 *               ...
 *               res1 = _wl_assign_(res1, a_mem, iv, _wlidx);
 *             }:res1;
 *           } : genarray(shp, a_mem);
 *
 *    ==>
 *
 *   var1 = ...
 *   _cuda_thread_space_( ShiftLB ( Gen), l0, 5, l2, 4, u1, u2, 1, 1, 1, 1, 1, 1);
 *                                        \-------/  \-------/  \-----/  \-----/
 *   a_mem = CUDA_kernel( a_mem, l0, l1, l2, u0, u1, u2, var1);
 *
 *
 *
 *   dev[*] CUDA_kernel( a_mem, l0, l1, l2, u0, u1, u2, var1)
 *   {
 *     _cuda_index_space_( ShiftLB ( Gen), iv,  l0,    5,   l2,
 *                                               4,   u1,   u2,
 *                                               1,    1,    1,
 *                                               1,    1,    1,
 *                                            eat1, eat2, eat3);
 *     _wlidx = _idxs2offset_( <const-shp>, eat1, eat2, eat3);  // in case of AKS!
 *     _wlidx = _array_idxs2offset_( shp, eat1, eat2, eat3);    // in case of AKD!
 *
 *     ... = var1;
 *     local_var1 = ...;
 *     ...
 *     a_mem = _cuda_wl_assign_(res1, a_mem, _wlidx);
 *     return a_mem;
 *   }
 *
 *
 * 
 * implementation:
 *   Once a cudarizable partition has been identified, we need to lift out the
 *   corresponding code into the body of a new kernel function. In CUKNLpart,
 *   we use a copy of the corresponding N_code as a starting point for this function.
 *
 *   While we traverse this new body, we identify relatively free variables (N_id)
 *   and locally defined ones (N_ids).
 *   For all these variables, upon first encounter we create new, identically name
 *   N_avis nodes and keep track of this through a LUT (old avis, new avis).
 *   -> For the relatively free vars (N_id), we then create N_arg and N_exprs chains
 *   (INFO_ARGS, INFO_PARAMS) for creating a matching pair a function signature
 *   and a function call.
 *   -> For the local vars (N_ids), we create N_vardec nodes (INFO_VARDECS).
 *   The handling of these variables is implemented in
 *      ProcessRelFreeVariable (avis, name, arg_info), and 
 *      ProcessLocalVariable (avis, name, arg_info)
 *   
 *   We change all links to the old N_avis nodes into the new ones. This is correct
 *   as we are traversing the copy of the code which will constitute the body of the
 *   kernel!
 *
 *   Before the aforementioned traversal of the body in spe, CUKNLpart actually 
 *   traverses N_withop and N_withid, thereafter the N_generator.
 *
 *   During N_withop, we make identify the returned memory (INFO_TRAVMEM)
 *   which triggers in CUKNLid the creation of return expressions for the kernel
 *   function (INFO_RETEXPRS) and the N_rets as well (INFO_RETS).
 *   This is implemented in 
 *      CreateRetsAndRetexprs (avis, arg_info)
 *
 *   During N_withids, we take care of the WITHID_VEC, WITHID_IDS, and the 
 *   WITHID_IDXS. We treat all of these as local variables, but we additionally
 *   insert allocations (INFO_ALLOCASSIGNS) and free (INFO_FREEASSIGNS) operations.
 *   This is implemented in
 *      CreateAllocAndFree (avis, arg_info)
 *   We also pu WITHID_VEC as exprs chain into INFO_INDEXSPACE
 *   and the WITHID_IDS as exprs chain in INFO_WLIDS.
 *   Finally, we generate the prf for computing the wlidx. We use a copy of
 *   INFO_WLIDS for the arguments and store the prf call in INFO_PRFWLIDXS.
 *   
 *   After the traversal of the body in spe, we traverse the N_generator.
 *   While doing so, we collect the bounds as exprs chains and append them
 *   to INFO_THREADSPACE and to INFO_INDEXSPACE. This is implemented in 
 *      HandleBoundStepWidthExprs (array, dims, name, arg_info)
 *   Thereafter, we assemble the prf calls _cuda_thread_space_ and
 *   _cuda_index_space_ in INFO_THREADSPACE and INFO_INDEXSPACE.
 *
 *   Now, we have components together. For the new kernel fun, we have:
 *      INFO_ARGS          - contains the function arguments
 *      INFO_RETS          - contains the function return types
 *      INFO_VARDECS       - contains the local variable declarations
 *      INFO_ALLOCASSIGNS  - contains local allocations for withid vars
 *      INFO_INDEXSPACE    - contains the call to _cuda_index_space_
 *      INFO_PRFWLIDXS     - contains the assignment to the wlidx
 *      INFO_FREEASSIGNS   - contains local free operations for the withid vars
 *      INFO_RETEXPRS      - contains the return expressions for the kernel fun
 *   For the kernel launch, we have:
 *      INFO_THREADSPACE   - contains the call to _cuda_thread_space_
 *      INFO_PARAMS        - contains the parameters of thekernel function call.
 *
 *
 *   In CIKNLpart, we eventually make the assembly of the kernel function happen
 *   and collect the kernels in INFO_CUDAKERNELS.
 *   This is implemented in:
 *      CreateCudaKernelDef (code, arg_info)
 *   Finally, we create the assignments that are part of the replacement for the
 *   cudarised with-loop from INFO_THREADSPACE and INFO_PARAMS and store them in
 *   INFO_CUDAAPS:
 *      
 *   _cuda_thread_space_( <gpukernel-pragma>, <lb0>...<lbn>, <ub0>...<ubn>,
 *                                            <s0> ... <sn>, <w0> ... <wn>);
 *   <mem> = CUDA_kernel( <mem>, <lb0>...<lbn>, <ub0>...<ubn>, <rfv0>...<rfvm> );
 *
 *   One tricky aspect is that we allow some with-loops (even in N_with2 form)
 *   to live *inside* cudarizable with loops. Therefore, we have to carefully
 *   keep track of what we do when dealing with with-loop components. Key for this
 *   are three context indicators:
 *
 *   INFO_IN_CUDA_WL         indicates that we are within an N_with that is
 *                           marked as CUDARIZABLE.
 *   INFO_IN_CUDA_PARTITION  indicates that we are traversing the duplicate
 *                           of the partittion that is currently being lifted!
 *   INFO_TRAVMEM            indicates that the N_id we are dealing with is
 *                           the variable pointing to the memory of the WL!
 *
 *   The typical programming pattern inside the WL components is:
 *      INFO_IN_CUDA_WL
 *      &&   ! INFO_IN_CUDA_PARTITION   => we need to do the above changes!
 *      INFO_IN_CUDA_WL
 *      &&     INFO_IN_CUDA_PARTITION   => we only deal with the identification
 *                                         of additional LUT entries!
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
#include "print.h"
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
    bool hasstepwidth :1;
    node *part;
    node *d2dsource;
    node *d2dtransfer;
    lut_t *lut;
    node *with;
    bool in_withop :1;
    node *withop;
    bool trav_mem :1;
    bool suballoc_rhs :1;
    node *suballoc_lhs;
    bool in_cuda_partition :1;
    node *pragma;
    node *replassigns;
    node *threadspace;
    node *indexspace;
};

// kernel function components:
#define INFO_ARGS(n) (n->args)
#define INFO_RETS(n) (n->rets)
#define INFO_VARDECS(n) (n->vardecs)
#define INFO_ALLOCASSIGNS(n) (n->allocassigns)
#define INFO_WLIDS(n) (n->prfwlids)             // helper only!
#define INFO_INDEXSPACE(n) (n->indexspace)
#define INFO_PRFWLIDXS(n) (n->prfwlidxs)
#define INFO_FREEASSIGNS(n) (n->freeassigns)
#define INFO_RETEXPRS(n) (n->retexprs)
// collects the new kernel fundefs:
#define INFO_CUDAKERNELS(n) (n->cudakernels)

// kernel launch components:
#define INFO_THREADSPACE(n) (n->threadspace)
#define INFO_PARAMS(n) (n->params)
// collects the kernel launches:
#define INFO_CUDAAPS(n) (n->cudaaps)
// signal replacement to N_assign
#define INFO_REPLACE_ASSIGNS(n) (n->replassigns)

// context indicators
#define INFO_IN_CUDA_WL(n) (n->collect)
#define INFO_IN_CUDA_PARTITION(n) (n->in_cuda_partition)
#define INFO_TRAVMEM(n) (n->trav_mem)

// context carriers:
#define INFO_LETIDS(n) (n->letids)  // current LHS N_ids
#define INFO_FUNDEF(n) (n->fundef)  // current N_fundef
#define INFO_WITH(n) (n->with)      // current N_with
#define INFO_WITHOP(n) (n->withop)  // withop of current WL
#define INFO_PART(n) (n->part)      // current N_part
#define INFO_PRAGMA(n) (n->pragma)  // gpukernel pragma of current WL

// needed for the currently not working device2device copies:
#define INFO_D2DSOURCE(n) (n->d2dsource)
#define INFO_D2DTRANSFER(n) (n->d2dtransfer)


// helpers:
#define INFO_LUT(n) (n->lut)
#define INFO_HASSTEPWIDTH(n) (n->hasstepwidth)

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
    INFO_WLIDS (result) = NULL;
    INFO_THREADSPACE (result) = NULL;
    INFO_INDEXSPACE (result) = NULL;
    INFO_PRFWLIDXS (result) = NULL;
    INFO_HASSTEPWIDTH (result) = FALSE;
    INFO_PART (result) = NULL;
    INFO_D2DSOURCE (result) = NULL;
    INFO_D2DTRANSFER (result) = NULL;
    INFO_LUT (result) = NULL;
    INFO_WITHOP (result) = NULL;
    INFO_TRAVMEM (result) = FALSE;
    INFO_IN_CUDA_PARTITION (result) = FALSE;
    INFO_WITH (result) = NULL;
    INFO_PRAGMA (result) = NULL;
    INFO_REPLACE_ASSIGNS (result) = NULL;

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
 * @fn node *CUKNLdoCreateCudaKernels (node *syntax_tree)
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
 * @fn node *PreprocessLocalVariable (node *avis, char *new_name, info *arg_info)
 *
 * @brief if the avis is not yet in the LUT, we create a new one whose type
 *        is switched from host type to device type. This avis is memorised
 *        as replacement for the old one in the LUT and we create an N_vardec
 *        for it which is inserted into INFO_VARDECS.
 *
 *        In any way, we return the associated avis.
 *
 *****************************************************************************/
static node *
ProcessLocalVariable (node *avis, char *new_name, info *arg_info)
{
    node *new_avis;
    ntype *type;
    DBUG_ENTER ();

    new_avis = LUTsearchInLutPp (INFO_LUT (arg_info), avis);
    if (new_avis == avis) {
        new_avis = DUPdoDupNode (avis);
        type = AVIS_TYPE (new_avis);
        if (!CUisDeviceTypeNew (type)) {
            AVIS_TYPE (new_avis) = CUconvertHostToDeviceType (type);
            type = TYfreeType (type);
        }
        if (new_name != NULL) {
            AVIS_NAME (new_avis) = MEMfree (AVIS_NAME (new_avis));
            AVIS_NAME (new_avis) = new_name;
        }
        INFO_VARDECS (arg_info) = TBmakeVardec (new_avis,
                                                INFO_VARDECS (arg_info));
        AVIS_DECL (new_avis) = INFO_VARDECS (arg_info);
        INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info),
                                                 avis, new_avis);
        DBUG_PRINT ("  >>> local variable %s added to LUT", AVIS_NAME (avis));
    }

    DBUG_RETURN (new_avis);
}

/** <!--********************************************************************-->
 *
 * @fn node *PreprocessRelFreeVariable (node *avis, char *new_name, info *arg_info)
 *
 * @brief if the avis is not yet in the LUT, we create a new one whose type
 *        is switched from host type to device type. This avis is memorised
 *        as replacement for the old one in the LUT and we create an N_arg
 *        for it which is inserted into INFO_ARGS. We also create an N_id
 *        for the old avis and insert it into INFO_PARAMS.
 *
 *        In any way, we return the associated avis.
 *
 *****************************************************************************/
static node *
ProcessRelFreeVariable (node *avis, char *new_name, info *arg_info)
{
    node *new_avis;
    ntype *type;
    DBUG_ENTER ();

    new_avis = LUTsearchInLutPp (INFO_LUT (arg_info), avis);
    if (new_avis == avis) {
        new_avis = DUPdoDupNode (avis);
        type = AVIS_TYPE (new_avis);
        if (!CUisDeviceTypeNew (type)) {
            AVIS_TYPE (new_avis) = CUconvertHostToDeviceType (type);
            type = TYfreeType (type);
        }
        if (new_name != NULL) {
            AVIS_NAME (new_avis) = MEMfree (AVIS_NAME (new_avis));
            AVIS_NAME (new_avis) = new_name;
        }
        INFO_ARGS (arg_info) = TBmakeArg (new_avis,
                                          INFO_ARGS (arg_info));
        AVIS_DECL (new_avis) = INFO_ARGS (arg_info);
        INFO_PARAMS (arg_info) = TBmakeExprs (TBmakeId (avis),
                                              INFO_PARAMS (arg_info));
        INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info),
                                                 avis, new_avis);
        DBUG_PRINT ("  >>> rel-free variable %s added to LUT", AVIS_NAME (avis));
    }

    DBUG_RETURN (new_avis);
}

/** <!--********************************************************************-->
 *
 * @fn static node *HandleBoundStepWidthExprs( node *array,
 *                                             size_t dims,
 *                                             char *name,
 *                                             info *arg_info)
 *
 * @param array the AST of one of the four generator expressions
 * @param dims number of elemens that should be in the bound
 * @param name string prefix, one of ("_lb_", "_ub_", "_step_", "_width")
 * @param arg_info
 *
 * @brief 1) assembles arguments to F_thread_space in INFO_THREADSPACE
 *           and for F_index_space in INFO_INDEXSPACE. If array == NULL,
 *           dims many 1s are being inserted.
 *        2) generates N_exprs-chain for kernel function call => INFO_PARAMS
 *        3) generates N_arg-chain for kernel function definition => INFO_ARGS
 *
 *****************************************************************************/
static void
HandleBoundStepWidthExprs (node *array, size_t dims, char *name, info *arg_info)
{
    node *elements;
    node *avis, *new_avis;
    char *bound_name;
    size_t dim = 0;
    int val;

    DBUG_ENTER ();

    if (array == NULL) {
        /* we are dealing with empty step or width */
        for( dim=0; dim<dims; dim++) {
            INFO_THREADSPACE (arg_info) =
                TCappendExprs (INFO_THREADSPACE (arg_info),
                               TBmakeExprs (TBmakeNum (1), NULL));

            INFO_INDEXSPACE (arg_info) =
                TCappendExprs (INFO_INDEXSPACE (arg_info),
                               TBmakeExprs (TBmakeNum (1), NULL));
        }

    } else {
        DBUG_ASSERT (NODE_TYPE (array) == N_array,
                     "generator expr is not an N_array!");
        elements = ARRAY_AELEMS (array);

        while (elements != NULL) {
            if (NODE_TYPE (EXPRS_EXPR (elements)) == N_id) {
                avis = ID_AVIS (EXPRS_EXPR (elements));
                bound_name = (char *)MEMmalloc (sizeof (char)
                                                * (STRlen (name) + 3));
                sprintf (bound_name, "%s%02zu", name, dim);
                new_avis = ProcessRelFreeVariable (avis, bound_name, arg_info);
    
                INFO_THREADSPACE (arg_info) =
                    TCappendExprs (INFO_THREADSPACE (arg_info),
                                   TBmakeExprs (TBmakeId (avis),
                                                NULL));
                INFO_INDEXSPACE (arg_info) =
                    TCappendExprs (INFO_INDEXSPACE (arg_info),
                                   TBmakeExprs (TBmakeId (new_avis),
                                                NULL));
            } else {
                DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (elements)) == N_num,
                    "generator bound is not an array of N_id or N_num nodes");
    
                val = NUM_VAL (EXPRS_EXPR (elements));
                INFO_THREADSPACE (arg_info) = 
                    TCappendExprs (INFO_THREADSPACE (arg_info),
                                   TBmakeExprs ( TBmakeNum (val), NULL));
                INFO_INDEXSPACE (arg_info) =
                    TCappendExprs (INFO_INDEXSPACE (arg_info),
                                   TBmakeExprs ( TBmakeNum (val), NULL));
            }

            elements = EXPRS_NEXT (elements);
            dim++;
        }
    }

    DBUG_RETURN ();
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
    node *cuda_kerneldef, *block, *assigns, *args, *ret;

    DBUG_ENTER ();

    /*
     * we build the assignment chain top down for readability
     * even if this is slightlyy less efficient!
     */
    assigns = INFO_ALLOCASSIGNS (arg_info);
    INFO_ALLOCASSIGNS (arg_info) = NULL;

    assigns = TCappendAssign (assigns, INFO_INDEXSPACE (arg_info));
    INFO_INDEXSPACE (arg_info) = NULL;

    assigns = TCappendAssign (assigns, INFO_PRFWLIDXS (arg_info));
    INFO_PRFWLIDXS (arg_info) = NULL;

    block = CODE_CBLOCK (code);
    assigns = TCappendAssign (assigns, BLOCK_ASSIGNS (block));
    BLOCK_ASSIGNS (block) = assigns;

    assigns = TCappendAssign (assigns, INFO_FREEASSIGNS (arg_info));
    INFO_FREEASSIGNS (arg_info) = NULL;

    ret = TBmakeReturn (INFO_RETEXPRS (arg_info));
    INFO_RETEXPRS (arg_info) = NULL;
    assigns = TCappendAssign (assigns, TBmakeAssign (ret, NULL));

    DBUG_ASSERT (BLOCK_VARDECS (block) == NULL, "vardecs in N_code block found!");
    BLOCK_VARDECS (block) = INFO_VARDECS (arg_info);
    INFO_VARDECS (arg_info) = NULL;

    args = INFO_ARGS (arg_info);
    // SetLinksignInfo (args, arg_info);
    INFO_ARGS (arg_info) = NULL;

    cuda_kerneldef = TBmakeFundef (
                         TRAVtmpVarName ("CUDA"),
                         NSdupNamespace (FUNDEF_NS (INFO_FUNDEF (arg_info))),
                         INFO_RETS (arg_info),
                         args,
                         block,
                         NULL);
    INFO_RETS (arg_info) = NULL;
    CODE_CBLOCK (code) = NULL;

    FUNDEF_ISCUDAGLOBALFUN (cuda_kerneldef) = TRUE;
    FUNDEF_HASSTEPWIDTHARGS (cuda_kerneldef) = INFO_HASSTEPWIDTH (arg_info);
    INFO_HASSTEPWIDTH (arg_info) = FALSE;
    FUNDEF_RETURN (cuda_kerneldef) = ret;

    INFO_CUDAKERNELS (arg_info) = TCappendFundef (cuda_kerneldef,
                                                  INFO_CUDAKERNELS (arg_info));

    DBUG_ASSERT (CODE_NEXT (code) == NULL, "code arg in CreateCudaKernelDef"
                                           " has NEXT!");
    code = FREEdoFreeTree (code);

    DBUG_PRINT ("  => new kernel function:");
    DBUG_EXECUTE ( PRTdoPrintNodeFile (stderr, cuda_kerneldef););

    DBUG_RETURN (cuda_kerneldef);
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

    DBUG_ASSERT (TUdimKnown (AVIS_TYPE (avis)), "Dimension is not known!");
    dim = TBmakeNum (TYgetDim (AVIS_TYPE (avis)));

    DBUG_ASSERT (TUshapeKnown (AVIS_TYPE (avis)), "Shape is not known!");
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
 * @fn void CreateRetsAndRetexprs( node *avis, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
static void
CreateRetsAndRetexprs (node *avis, info *arg_info)
{
    DBUG_ENTER ();

    INFO_RETS (arg_info) = TCappendRet (
                               TBmakeRet (TYeliminateAKV (
                                              AVIS_TYPE (avis)),
                                          NULL),
                               INFO_RETS (arg_info));

    INFO_RETEXPRS (arg_info) = TCappendExprs (
                                   TBmakeExprs (TBmakeId (avis), NULL),
                                   INFO_RETEXPRS (arg_info));

    DBUG_RETURN ();
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
 * @fn node *CUKNLassign( node *arg_node, info *arg_info)
 *
 * @brief  For cudarizable N_with, remove the old N_assign and insert
 *         the newly created CUDA kernel N_aps into the assign chain.
 *
 *****************************************************************************/
node *
CUKNLassign (node *arg_node, info *arg_info)
{
    node *replacement = NULL;
    DBUG_ENTER ();

    /* We do need to traverse top-down as out local/rel-free variable
     * detection for identifying the kernel arguments relies on it!
     * However, in case we replace the entire RHS, we need to make sure
     * that we continue with the correct node! Therefore, we delay
     * the actual replacement until *after* traversing the remainder.
     * surely, we could stack INFO_REPLACE_ASSIGNS instead, but that
     * seems to be even more overhead.
     */
    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);
    if (INFO_REPLACE_ASSIGNS (arg_info) != NULL) {
        DBUG_PRINT ("=> WL replacement code:");
        DBUG_EXECUTE ( PRTdoPrintFile (stderr,
                                       INFO_REPLACE_ASSIGNS (arg_info)););
        
        replacement = INFO_REPLACE_ASSIGNS (arg_info);
        INFO_REPLACE_ASSIGNS (arg_info) = NULL;
    }

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    if (replacement != NULL)
        arg_node = TCappendAssign (replacement, FREEdoFreeNode (arg_node));

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
    node *old_letids;
    DBUG_ENTER ();

    old_letids = INFO_LETIDS (arg_info);
    INFO_LETIDS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    INFO_LETIDS (arg_info) = old_letids;

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
    node *old_with;
    DBUG_ENTER ();

    if (WITH_CUDARIZABLE (arg_node)) {
        /*
         * push new arg_info stuff:
         */
        DBUG_PRINT ("start cudarizing with-loop");
        INFO_IN_CUDA_WL (arg_info) = TRUE; // signal cudarisation!
        INFO_WITHOP (arg_info) = WITH_WITHOP (arg_node);
        old_with = INFO_WITH (arg_info);
        INFO_WITH (arg_info) = arg_node;

        WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);

        /*
         * restore old arg_info stuff:
         */
        INFO_WITH (arg_info) = old_with;
        INFO_WITHOP (arg_info) = NULL;
        INFO_IN_CUDA_WL (arg_info) = FALSE; // cudarisation done!

        /*
         * Indicate to N_assign that a chain of CUDA kernel N_aps
         * has been created and needs to be inserted into the AST
         */
        INFO_REPLACE_ASSIGNS (arg_info)
           = TCappendAssign (INFO_D2DTRANSFER (arg_info), INFO_CUDAAPS (arg_info));
        INFO_D2DTRANSFER (arg_info)  = NULL;
        INFO_CUDAAPS (arg_info)  = NULL;
        DBUG_PRINT ("done cudarizing with-loop");

    } else if (INFO_IN_CUDA_PARTITION (arg_info)) {

        old_with = INFO_WITH (arg_info);
        INFO_WITH (arg_info) = arg_node;
        arg_node = TRAVcont (arg_node, arg_info);
        INFO_WITH (arg_info) = old_with;
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

    DBUG_ENTER ();

    arg_node = TRAVcont (arg_node, arg_info);

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
    node *old_pragma;
    node *cuda_kernel, *cuda_funap;
    node *dup_code;

    DBUG_ENTER ();

    if (INFO_IN_CUDA_WL (arg_info)) {
        INFO_PART (arg_info) = arg_node;
        if (WITH_HASRC (INFO_WITH (arg_info)) && PART_ISCOPY (arg_node)) {
            DBUG_PRINT ("  copy partition => dismissing!");
        } else if (PART_CUDARIZABLE (arg_node)) {
            DBUG_PRINT ("  start cudarizing partition");
            /* We create a lookup table for the traversal of each partition */
            INFO_LUT (arg_info) = LUTgenerateLut ();

            /* Since each CUDA kernel created from an N_part may
             * potentially need the sons of withop as arguments and
             * it's created independently, traversal of each N_part
             * must also traverse the withop associated with the
             * N_with. */
            DBUG_PRINT ("    traversing withop:");
            INFO_WITHOP (arg_info) = TRAVopt (INFO_WITHOP (arg_info), arg_info);

            /********* Begin traversal of N_part Sons/Attributes *********/

            DBUG_PRINT ("    traversing withid:");
            PART_WITHID (arg_node) = TRAVopt (PART_WITHID (arg_node), arg_info);

            /* Since each CUDA kernel contains the code originally in each
             * N_part and since N_code can be shared between more than one
             * N_part, we duplicate it before traversing into it */
            dup_code = DUPdoDupNode (PART_CODE (arg_node));
            INFO_IN_CUDA_PARTITION (arg_info) = TRUE;
            DBUG_PRINT ("    traversing body:");
            dup_code = TRAVopt (dup_code, arg_info);
            INFO_IN_CUDA_PARTITION (arg_info) = FALSE;

            DBUG_PRINT ("    traversing generator:");
     
            old_pragma = INFO_PRAGMA (arg_info);
            INFO_PRAGMA (arg_info) = PART_PRAGMA (arg_node);
            PART_GENERATOR (arg_node) = TRAVopt (PART_GENERATOR (arg_node), arg_info);
            INFO_PRAGMA (arg_info) = old_pragma;

            /********** End traversal of N_part Sons/Attributes **********/

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
              = TCappendAssign (INFO_THREADSPACE (arg_info),
                                TCappendAssign (cuda_funap, INFO_CUDAAPS (arg_info)));

            /******* End creating CUDA kernel and its application *******/

            /* Clean up */
            INFO_ARGS (arg_info) = NULL;
            INFO_PARAMS (arg_info) = NULL;
            INFO_RETS (arg_info) = NULL;
            INFO_RETEXPRS (arg_info) = NULL;
            INFO_THREADSPACE (arg_info) = NULL;
            INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));
            DBUG_PRINT ("  done cudarizing partition");

        } else if (INFO_IN_CUDA_PARTITION (arg_info)) {
            DBUG_PRINT ("  traversing inner partition");

            PART_WITHID (arg_node) = TRAVopt (PART_WITHID (arg_node), arg_info);
            PART_GENERATOR (arg_node) = TRAVopt (PART_GENERATOR (arg_node), arg_info);

        } else {
            DBUG_ASSERT ((0==1), "   device2decvice not yet properly supported!");
            /* For non-cudarizable partition, we traverse its code
             * and create a <device2device>. Note that we only traverse
             * the first non-cudarizable partition encountered since
             * for all non-cudarizable partition, only one <device2device>
             * is needed. */
            if (PART_CODE (arg_node) != NULL && INFO_D2DTRANSFER (arg_info) == NULL) {
                // PART_CODE( arg_node) = TRAVopt( PART_CODE( arg_node), arg_info);
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
            /* This is an inner WL! Shape needs to be traversed as well. */
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
            /* This is an inner WL! */
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
            /* This is an inner WL! Shape needs to be traversed as well. */
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
    node *new_avis, *iv_new_avis;
    node *prf_wlidxs, *prf_wlidxs_args = NULL;

    DBUG_ENTER ();

    wlids = WITHID_IDS (arg_node);
    wlidxs = WITHID_IDXS (arg_node);
    wlvec = WITHID_VEC (arg_node);
    withop = INFO_WITHOP (arg_info);

    if (INFO_IN_CUDA_WL (arg_info)) {

        DBUG_ASSERT (NODE_TYPE (wlvec) == N_id,
                     "Non N_id node found in N_withid->vec!");
        iv_new_avis = ProcessLocalVariable (ID_AVIS (WITHID_VEC (arg_node)),
                                            NULL, arg_info);
        CreateAllocAndFree (iv_new_avis, arg_info);
        if (INFO_IN_CUDA_PARTITION (arg_info)) {
            ID_AVIS (WITHID_VEC (arg_node)) = iv_new_avis;
        } else {
            INFO_INDEXSPACE (arg_info) = TBmakeExprs (TBmakeId (iv_new_avis),
                                                      NULL);
        }

        while (wlids != NULL) {
            id = EXPRS_EXPR (wlids);
            new_avis = ProcessLocalVariable (ID_AVIS (id), NULL, arg_info);
            CreateAllocAndFree (new_avis, arg_info);

            if (INFO_IN_CUDA_PARTITION (arg_info)) {
                ID_AVIS (id) = new_avis;
            } else {
                INFO_WLIDS (arg_info) = TCappendExprs (
                                            INFO_WLIDS (arg_info),
                                            TBmakeExprs (TBmakeId (new_avis),
                                                         NULL));
            }
            wlids = EXPRS_NEXT (wlids);
        }

        while (wlidxs != NULL && withop != NULL) {
            id = EXPRS_EXPR (wlidxs);
            new_avis = ProcessLocalVariable (ID_AVIS (id), NULL, arg_info);

            if (INFO_IN_CUDA_PARTITION (arg_info)) {
                ID_AVIS (id) = new_avis;
            } else {
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
                                                   DUPdoDupTree (INFO_WLIDS (arg_info)));

                    /* Create primitives F_idxs2offset */
                    prf_wlidxs = TBmakePrf (F_idxs2offset, prf_wlidxs_args);
                } else {
                    prf_wlidxs_args
                      = TBmakeExprs (TBmakeId (new_mem_avis),
                                     DUPdoDupTree (INFO_WLIDS (arg_info)));

                    /* Create primitives F_array_idxs2offset */
                    prf_wlidxs = TBmakePrf (F_array_idxs2offset, prf_wlidxs_args);
                }

                INFO_PRFWLIDXS (arg_info)
                  = TBmakeAssign (TBmakeLet (TBmakeIds (new_avis, NULL), prf_wlidxs),
                                  INFO_PRFWLIDXS (arg_info));
            }

            wlidxs = EXPRS_NEXT (wlidxs);
            withop = WITHOP_NEXT (withop);
            DBUG_ASSERT (((wlidxs == NULL && withop == NULL)
                          || (wlidxs != NULL && withop != NULL)),
                         "#withop != #N_withid->wlidxs!");
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

    arg_node = TRAVcont (arg_node, arg_info);

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
    size_t dims;

    if (INFO_IN_CUDA_WL (arg_info)) {
        if (INFO_IN_CUDA_PARTITION (arg_info)) {
            GENERATOR_BOUND1 (arg_node) = TRAVdo (GENERATOR_BOUND1 (arg_node),
                                                  arg_info);
            GENERATOR_BOUND2 (arg_node) = TRAVdo (GENERATOR_BOUND2 (arg_node),
                                                  arg_info);
            GENERATOR_STEP (arg_node) = TRAVopt (GENERATOR_STEP (arg_node),
                                                 arg_info);
            GENERATOR_WIDTH (arg_node) = TRAVopt (GENERATOR_WIDTH (arg_node),
                                                  arg_info);
        } else {
            dims = TCcountExprs (ARRAY_AELEMS (GENERATOR_BOUND1 (arg_node)));
            HandleBoundStepWidthExprs (GENERATOR_BOUND1 (arg_node),
                                       dims, "_lb_", arg_info);
            HandleBoundStepWidthExprs (GENERATOR_BOUND2 (arg_node),
                                       dims, "_ub_", arg_info);

            HandleBoundStepWidthExprs (GENERATOR_STEP (arg_node),
                                       dims, "_step_", arg_info);
            HandleBoundStepWidthExprs (GENERATOR_WIDTH (arg_node),
                                       dims, "_width_", arg_info);

            DBUG_ASSERT (INFO_PRAGMA (arg_info) != NULL, "missing gpukernel pragma");
            INFO_THREADSPACE (arg_info) =
                TBmakeAssign (
                    TBmakeLet (
                        NULL,
                        TBmakePrf (
                            F_cuda_thread_space,
                            TBmakeExprs (
                                DUPdoDupTree (INFO_PRAGMA (arg_info)),
                                INFO_THREADSPACE (arg_info)))),
                    NULL);
            DBUG_PRINT_TAG ("CUKNL_EXT", "  => assembled _cuda_thread_space_:");
            DBUG_EXECUTE_TAG ("CUKNL_EXT", PRTdoPrintFile (stderr,
                                               INFO_THREADSPACE (arg_info)));

            INFO_INDEXSPACE (arg_info) = TCappendExprs (INFO_INDEXSPACE (arg_info),
                                                        INFO_WLIDS (arg_info));
            INFO_WLIDS (arg_info) = NULL;
            INFO_INDEXSPACE (arg_info) =
                TBmakeAssign (
                    TBmakeLet (
                        NULL,
                        TBmakePrf (
                            F_cuda_index_space,
                            TBmakeExprs (
                                DUPdoDupTree (INFO_PRAGMA (arg_info)),
                                INFO_INDEXSPACE (arg_info)))),
                    NULL);
            DBUG_PRINT_TAG ("CUKNL_EXT", "  => assembled _cuda_index_space_:");
            DBUG_EXECUTE_TAG ("CUKNL_EXT", PRTdoPrintFile (stderr,
                                               INFO_INDEXSPACE (arg_info)));


            if (GENERATOR_STEP (arg_node) != NULL
                && GENERATOR_WIDTH (arg_node) != NULL) {
                INFO_HASSTEPWIDTH (arg_info) = TRUE;
            }
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
    node *new_avis;

    DBUG_ENTER ();

    if (INFO_IN_CUDA_WL (arg_info)) {
        DBUG_PRINT ("      processing id %s", ID_NAME (arg_node));
        new_avis = ProcessRelFreeVariable (ID_AVIS (arg_node), NULL, arg_info);

        if (INFO_IN_CUDA_PARTITION (arg_info)) {
            ID_AVIS (arg_node) = new_avis;
        } else if (INFO_TRAVMEM (arg_info)) {
            CreateRetsAndRetexprs (new_avis, arg_info);
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
    DBUG_ENTER ();


    if (INFO_IN_CUDA_WL (arg_info)
        && (PART_CUDARIZABLE (INFO_PART (arg_info))
            || INFO_IN_CUDA_PARTITION (arg_info))) {
        DBUG_PRINT ("      processing ids %s", IDS_NAME (arg_node));
        IDS_AVIS (arg_node) = ProcessLocalVariable (IDS_AVIS (arg_node),
                                                    NULL, arg_info);
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
