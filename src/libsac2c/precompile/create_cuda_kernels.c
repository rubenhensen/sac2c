/*****************************************************************************
 *
 * $Id: create_spmd_funs.c 15846 2008-11-04 00:29:36Z cg $
 *
 * file:   create_spmd_funs.c
 *
 * prefix: MTSPMDF
 *
 * description:
 *
 *   We traverse each ST function body and look for with-loops to be
 *   parallelised. Each such with-loop is then lifted into a separate
 *   function definition. These newly created fundefs are named and tagged
 *   SPMD functions. These functions will later implement switching from
 *   single-threaded execution to multithreaded execution and vice versa.
 *
 *   The necessary information to create a fully-fledged function definition,
 *   e.g. parameter names and types, return values and types, local variables
 *   and their types, is gathered during a full traversal of each with-loop
 *   tagged for multithreaded execution.
 *
 *   The newly generated functions are inserted at the end of the fundef
 *   chain.
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

/**
 * INFO structure
 */

struct INFO {
    node *cudakernels;
    node *cudaaps;
    node *letids;

    node *fundef;
    lut_t *lut;
    node *args;
    node *params;
    node *vardecs;
    node *rets;
    node *retexprs;
    node *allocassigns;
    node *freeassigns;
    bool collect;
    bool lift;
    bool withid;
};

/**
 * INFO macros
 */

#define INFO_CUDAKERNELS(n) (n->cudakernels)
#define INFO_CUDAAPS(n) (n->cudaaps)
#define INFO_LETIDS(n) (n->letids)

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LUT(n) (n->lut)
#define INFO_ARGS(n) (n->args)
#define INFO_PARAMS(n) (n->params)
#define INFO_VARDECS(n) (n->vardecs)
#define INFO_RETS(n) (n->rets)
#define INFO_RETEXPRS(n) (n->retexprs)
#define INFO_ALLOCASSIGNS(n) (n->allocassigns)
#define INFO_FREEASSIGNS(n) (n->freeassigns)
#define INFO_COLLECT(n) (n->collect)
#define INFO_LIFT(n) (n->lift)
#define INFO_WITHID(n) (n->withid)

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
    INFO_LUT (result) = NULL;
    INFO_ARGS (result) = NULL;
    INFO_PARAMS (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_RETS (result) = NULL;
    INFO_RETEXPRS (result) = NULL;
    INFO_ALLOCASSIGNS (result) = NULL;
    INFO_FREEASSIGNS (result) = NULL;
    INFO_COLLECT (result) = FALSE;
    INFO_LIFT (result) = FALSE;
    INFO_WITHID (result) = FALSE;

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
 * @fn static node *CreateSpmdFundef( node *arg_node, info *arg_info)
 *
 * @brief generates SPMD fundef from data gathered in the info node during
 *   preceding traversal of MT-tagged with-loop
 *
 *****************************************************************************/

/*
static
node *CreateCudaKernelDef( node *arg_node, info *arg_info)
{
  node *cuda_hernekdef, *fundef, *body, *withlet, *retexprs, *vardecs;
  node *allocassigns, *freeassigns, *assigns, *args, *rets, *retur;

  DBUG_ENTER("CreateCudaKernelDdef");

  DBUG_ASSERT( NODE_TYPE( arg_node) == N_let,
                   "CreateCudaKernelDdef() called with illegal node type.");

  fundef = INFO_FUNDEF(arg_info);

  retexprs = INFO_RETEXPRS( arg_info);
  INFO_RETEXPRS( arg_info) = NULL;

  vardecs = INFO_VARDECS( arg_info);
  INFO_VARDECS( arg_info) = NULL;


  allocassigns = INFO_ALLOCASSIGNS( arg_info);
  INFO_ALLOCASSIGNS( arg_info) = NULL;

  freeassigns = INFO_FREEASSIGNS( arg_info);
  INFO_FREEASSIGNS( arg_info) = NULL;

  rets = INFO_RETS(arg_info);
  INFO_RETS(arg_info) = NULL;


  args = INFO_ARGS(arg_info);
  INFO_ARGS(arg_info) = NULL;


  withlet = DUPdoDupTreeLut( arg_node, INFO_LUT( arg_info));
  INFO_LUT( arg_info) = LUTremoveContentLut( INFO_LUT( arg_info));

  retur = TBmakeReturn( retexprs);

  assigns = TCappendAssign( allocassigns,
                            TCappendAssign( TBmakeAssign( withlet, NULL),
                                             TCappendAssign( freeassigns,
                                                             TBmakeAssign( retur,
                                                                           NULL))));
  body = TBmakeBlock( assigns, vardecs);

  cuda_hernekdef = TBmakeFundef( TRAVtmpVarName( FUNDEF_NAME( fundef)),
                              NSdupNamespace( FUNDEF_NS( fundef)),
                              NULL,
                              args,
                              NULL,
                              NULL);

  FUNDEF_ISCUDAGLOBALFUN( cuda_hernekdef) = TRUE;
  FUNDEF_RETURN( spmd_fundef) = retur;

  DBUG_RETURN(cuda_hernekdef);
}
*/

static node *
CreateCudaKernelDef (node *code, info *arg_info)
{
    node *cuda_kerneldef, *fundef, *body, *retexprs, *vardecs;
    node *args, *rets, *retur;

    DBUG_ENTER ("CreateCudaKernelDdef");

    fundef = INFO_FUNDEF (arg_info);

    retexprs = INFO_RETEXPRS (arg_info);
    INFO_RETEXPRS (arg_info) = NULL;

    vardecs = INFO_VARDECS (arg_info);
    INFO_VARDECS (arg_info) = NULL;

    rets = INFO_RETS (arg_info);
    INFO_RETS (arg_info) = NULL;

    args = INFO_ARGS (arg_info);
    INFO_ARGS (arg_info) = NULL;

    // kernel_code = DUPdoDupTreeLut( code, INFO_LUT( arg_info));
    // INFO_LUT( arg_info) = LUTremoveContentLut( INFO_LUT( arg_info));

    retur = TBmakeReturn (retexprs);

    body = DUPdoDupTree (CODE_CBLOCK (code));

    cuda_kerneldef
      = TBmakeFundef (TRAVtmpVarName (FUNDEF_NAME (fundef)),
                      NSdupNamespace (FUNDEF_NS (fundef)), rets, args, body, NULL);

    FUNDEF_ISCUDAGLOBALFUN (cuda_kerneldef) = TRUE;
    FUNDEF_RETURN (cuda_kerneldef) = retur;

    DBUG_RETURN (cuda_kerneldef);
}

/******************************************************************************
 *
 * @fn MTSPMDFdoCreateSpmdFuns( node *syntax_tree)
 *
 *  @brief initiates traversal for creating SPMD functions
 *
 *  @param syntax_tree
 *
 *  @return syntax_tree
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
 * @fn node *MTSPMDFmodule( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
CUKNLmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUKNLmodule");

    INFO_LUT (arg_info) = LUTgenerateLut ();

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTSPMDFfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
CUKNLfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUKNLfundef");

    if (!FUNDEF_ISCUDAGLOBALFUN (arg_node) && (FUNDEF_BODY (arg_node) != NULL)) {
        /*
         * Only ST funs may contain parallel with-loops.
         * Hence, we constrain our search accordingly.
         */
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = NULL;
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    } else {
        /*
         * We have reached the end of the FUNDEF chain. We add the new SPMD functions
         * constructed meanwhile and stored in the info structure to the end and stop
         * the traversal.
         */
        FUNDEF_NEXT (arg_node) = INFO_CUDAKERNELS (arg_info);
        INFO_CUDAKERNELS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

node *
CUKNLassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUKNLassign");

    node *ret_node = NULL;
    node *next = NULL;
    node *tmp;

    next = ASSIGN_NEXT (arg_node);

    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }

    if (INFO_CUDAAPS (arg_info) != NULL) {
        ret_node = INFO_CUDAAPS (arg_info);
        tmp = INFO_CUDAAPS (arg_info);
        while (ASSIGN_NEXT (tmp) != NULL) {
            tmp = ASSIGN_NEXT (tmp);
        }
        ASSIGN_NEXT (tmp) = next INFO_CUDAAPS (arg_info) = NULL;
        if (ASSIGN_NEXT (tmp) != NULL) {
            ASSIGN_NEXT (tmp) = TRAVdo (ASSIGN_NEXT (tmp), arg_info);
        }
    } else {
        ret_node = arg_node;
        if (ASSIGN_NEXT (arg_node) != NULL) {
            ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTSPMDFlet( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
CUKNLlet (node *arg_node, info *arg_info)
{
    // node *spmd_fundef, *spmd_ap;

    DBUG_ENTER ("CUKNLlet");

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    if (INFO_LIFT (arg_info)) {

        arg_node = FREEdoFreeTree (arg_node);
        /*
            spmd_fundef = CreateSpmdFundef( arg_node, arg_info);
            FUNDEF_NEXT( spmd_fundef) = INFO_CUDAKERNELS( arg_info);
            INFO_CUDAKERNELS( arg_info) = spmd_fundef;

            cuda_kernel_ap = TBmakeAp( spmd_fundef, INFO_PARAMS( arg_info));
            INFO_PARAMS( arg_info) = NULL;

            LET_EXPR( arg_node) = FREEdoFreeTree( LET_EXPR( arg_node));
            LET_EXPR( arg_node) = cuda_kernel_ap;
        */
        INFO_LIFT (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *MTSPMDFid(node *arg_node, info *arg_info)
 *
 *    @brief
 *
 *    @param arg_node
 *    @param arg_info
 *
 *    @return
 ******************************************************************************/

node *
CUKNLid (node *arg_node, info *arg_info)
{
    node *avis, *new_avis, *ids, *dim, *shape, *alloc, *free;

    DBUG_ENTER ("CUKNLid");

    avis = ID_AVIS (arg_node);
    new_avis = NULL;

    ids = NULL;
    dim = NULL;
    shape = NULL;
    alloc = NULL;
    free = NULL;

    DBUG_PRINT ("CUKNL", ("ENTER id %s", ID_NAME (arg_node)));

    if (INFO_COLLECT (arg_info)) {
        if (LUTsearchInLutPp (INFO_LUT (arg_info), avis) == avis) {
            DBUG_PRINT ("CUKNL", ("  Not handled before..."));
            new_avis = DUPdoDupNode (avis);

            // if(!INFO_WITHID(arg_info)) {
            INFO_ARGS (arg_info) = TBmakeArg (new_avis, INFO_ARGS (arg_info));
            INFO_PARAMS (arg_info)
              = TBmakeExprs (TBmakeId (avis), INFO_PARAMS (arg_info));
            //}
            INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), avis, new_avis);
        }

        /*
            if (INFO_WITHID(arg_info)) {
              DBUG_PRINT("MTSPMDF", ("...is Withid-id"));
              if (new_avis == NULL) {
                new_avis = LUTsearchInLutPp(INFO_LUT(arg_info), avis);
              }
              else {
                INFO_VARDECS(arg_info) = TBmakeVardec(new_avis, INFO_VARDECS(arg_info));
              }

              ids = TBmakeIds(new_avis, NULL);

              if(TUdimKnown(AVIS_TYPE(new_avis))) {
                dim = TBmakeNum(TYgetDim(AVIS_TYPE(new_avis)));
              }

              if(TUshapeKnown(AVIS_TYPE(new_avis))) {
                shape = SHshape2Array(TYgetShape(AVIS_TYPE(new_avis)));
              }

              alloc = TCmakePrf3(F_alloc, TBmakeNum(1), dim, shape);

              INFO_ALLOCASSIGNS(arg_info) = TBmakeAssign(TBmakeLet(ids, alloc),
                                                         INFO_ALLOCASSIGNS(arg_info));

              free = TCmakePrf1(F_free, TBmakeId(new_avis));
              INFO_FREEASSIGNS(arg_info) = TBmakeAssign(TBmakeLet(NULL, free),
                                                        INFO_FREEASSIGNS(arg_info));
            }
        */
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *MTSPMDFids(node *arg_node, info *arg_info)
 *
 *    @brief
 *
 *    @param arg_node
 *    @param arg_info
 *
 *    @return
 ******************************************************************************/

node *
CUKNLids (node *arg_node, info *arg_info)
{
    node *avis;
    node *new_avis;

    DBUG_ENTER ("CUKNLids");

    avis = IDS_AVIS (arg_node);
    new_avis = NULL;

    DBUG_PRINT ("CUKNL", ("ENTER ids %s", IDS_NAME (arg_node)));

    if (INFO_COLLECT (arg_info)) {
        if (LUTsearchInLutPp (INFO_LUT (arg_info), avis) == avis) {
            new_avis = DUPdoDupNode (avis);
            INFO_VARDECS (arg_info) = TBmakeVardec (new_avis, INFO_VARDECS (arg_info));
            INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), avis, new_avis);
            DBUG_PRINT ("CUKNL", (">>> ids %s added to LUT", IDS_NAME (arg_node)));
        }
    } else {
        /*
            if (INFO_LIFT(arg_info)) {

              //If INFO_LIFT is true, we are on the LHS of the assignment which has the
              //MT-Withloop on the RHS.

              new_avis = LUTsearchInLutPp(INFO_LUT(arg_info), avis);

              if (new_avis == avis) {
                new_avis = DUPdoDupNode(avis);
                INFO_LUT(arg_info) = LUTinsertIntoLutP(INFO_LUT(arg_info),
                                                       avis, new_avis);
                INFO_VARDECS(arg_info) = TBmakeVardec(new_avis,
                                                      INFO_VARDECS(arg_info));
              }

              INFO_RETS(arg_info) =
                TCappendRet(INFO_RETS(arg_info),
                            TBmakeRet(TYeliminateAKV(AVIS_TYPE(new_avis)), NULL));

              INFO_RETEXPRS(arg_info) =
                TCappendExprs(INFO_RETEXPRS(arg_info),
                              TBmakeExprs(TBmakeId( new_avis), NULL));
            }
        */
    }

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *MTSPMDFwith2( node *arg_node, info *arg_info)
 *
 * description:
 *   lifts a parallelised with-loop into a function.
 *
 ******************************************************************************/

node *
CUKNLwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUKNLwith");

    if (WITH_CUDARIZABLE (arg_node)) {
        /*
         * Start collecting data flow information
         */
        INFO_COLLECT (arg_info) = TRUE;

        WITH_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

        INFO_COLLECT (arg_info) = FALSE;
        /*
         * Stop collecting data flow information
         */
        INFO_LIFT (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *MTSPMDFwithid( node *arg_node, info *arg_info)
 *
 *    @brief traversal function for N_withid node
 *      We memoise the fact that we now traverse a withid to do the right things
 *      when encountering id and ids nodes.
 *
 *    @param arg_node
 *    @param arg_info
 *
 *    @return arg_node
 *
 ******************************************************************************/

node *
CUKNLwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUKNLwithid");

    INFO_WITHID (arg_info) = TRUE;

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_WITHID (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

node *
CUKNLgenarray (node *arg_node, info *arg_info)
{
    node *shp;
    node *shp_elements;
    node *new_avis, *avis;

    DBUG_ENTER ("CUKNLgenarray");

    shp = GENARRAY_SHAPE (arg_node);

    if (NODE_TYPE (shp) == N_num) {
        avis = TBmakeAvis ("shp", TYmakeSimpleType (T_int));
        INFO_ARGS (arg_info) = TBmakeArg (avis, INFO_ARGS (arg_info));
        INFO_PARAMS (arg_info) = TBmakeExprs (shp, INFO_PARAMS (arg_info));

    } else if (NODE_TYPE (shp) == N_array) {
        shp_elements = ARRAY_AELEMS (shp);

        int count = 0;
        while (shp_elements != NULL) {
            switch (NODE_TYPE (EXPRS_EXPR (shp_elements))) {
            case N_num:
                INFO_PARAMS (arg_info)
                  = TBmakeExprs (EXPRS_EXPR (shp_elements), INFO_PARAMS (arg_info));

                char *num_str = STRitoa (count);
                char *name_str = STRcat ("shp", num_str);
                num_str = MEMfree (num_str);
                avis = TBmakeAvis (name_str, TYmakeSimpleType (T_int));
                num_str = MEMfree (name_str);
                INFO_ARGS (arg_info) = TBmakeArg (avis, INFO_ARGS (arg_info));
                break;
            case N_id:
                avis = ID_AVIS (EXPRS_EXPR (shp_elements));
                INFO_PARAMS (arg_info)
                  = TBmakeExprs (TBmakeId (avis), INFO_PARAMS (arg_info));
                new_avis = DUPdoDupNode (avis);
                INFO_ARGS (arg_info) = TBmakeArg (new_avis, INFO_ARGS (arg_info));
            default:
                break;
            }
            shp_elements = EXPRS_NEXT (shp_elements);
            count++;
        }
    }

    if (GENARRAY_DEFAULT (arg_node) != NULL)
        GENARRAY_DEFAULT (arg_node) = TRAVdo (GENARRAY_DEFAULT (arg_node), arg_info);

    if (GENARRAY_MEM (arg_node) != NULL)
        GENARRAY_MEM (arg_node) = TRAVdo (GENARRAY_MEM (arg_node), arg_info);

    if (GENARRAY_SUB (arg_node) != NULL)
        GENARRAY_SUB (arg_node) = TRAVdo (GENARRAY_SUB (arg_node), arg_info);

    if (GENARRAY_NEXT (arg_node) != NULL)
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
CUKNLcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUKNLcode");

    if (CODE_CBLOCK (arg_node) != NULL)
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
CUKNLpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUKNLpart");

    node *cuda_kernel;
    node *ids;

    if (PART_CODE (arg_node) != NULL)
        PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);

    if (PART_GENERATOR (arg_node) != NULL)
        PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);

    cuda_kernel = CreateCudaKernelDef (PART_CODE (arg_node), arg_info);
    FUNDEF_NEXT (cuda_kernel) = INFO_CUDAKERNELS (arg_info);
    INFO_CUDAKERNELS (arg_info) = cuda_kernel;

    ids = INFO_LETIDS (arg_info);
    INFO_CUDAAPS (arg_info)
      = TBmakeAssign (TBmakeLet (TBmakeIds (IDS_AVIS (ids), NULL),
                                 TBmakeAp (cuda_kernel, INFO_ARGS (arg_info))),
                      INFO_CUDAAPS (arg_info));

    if (PART_NEXT (arg_node) != NULL)
        PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
CUKNLgenerator (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUKNLgenerator");

    node *lower_bound, *upper_bound;
    node *lower_bound_elements, *upper_bound_elements;
    node *avis, *new_avis;

    lower_bound = GENERATOR_BOUND1 (arg_node);
    upper_bound = GENERATOR_BOUND2 (arg_node);

    if (NODE_TYPE (lower_bound) == N_num && NODE_TYPE (upper_bound) == N_num) {
        avis = TBmakeAvis ("ub", TYmakeSimpleType (T_int));
        INFO_ARGS (arg_info) = TBmakeArg (avis, INFO_ARGS (arg_info));
        avis = TBmakeAvis ("lb", TYmakeSimpleType (T_int));
        INFO_ARGS (arg_info) = TBmakeArg (avis, INFO_ARGS (arg_info));

        INFO_PARAMS (arg_info) = TBmakeExprs (upper_bound, INFO_PARAMS (arg_info));
        INFO_PARAMS (arg_info) = TBmakeExprs (lower_bound, INFO_PARAMS (arg_info));
    } else if (NODE_TYPE (lower_bound) == N_array && NODE_TYPE (upper_bound) == N_array) {
        lower_bound_elements = ARRAY_AELEMS (lower_bound);
        upper_bound_elements = ARRAY_AELEMS (upper_bound);

        int count = 0;
        while (lower_bound_elements != NULL && upper_bound_elements != NULL) {
            switch (NODE_TYPE (EXPRS_EXPR (upper_bound_elements))) {
            case N_num:
                INFO_PARAMS (arg_info) = TBmakeExprs (EXPRS_EXPR (upper_bound_elements),
                                                      INFO_PARAMS (arg_info));

                char *num_str = STRitoa (count);
                char *name_str = STRcat ("ub", num_str);
                num_str = MEMfree (num_str);
                avis = TBmakeAvis (name_str, TYmakeSimpleType (T_int));
                num_str = MEMfree (name_str);
                INFO_ARGS (arg_info) = TBmakeArg (avis, INFO_ARGS (arg_info));
                break;
            case N_id:
                avis = ID_AVIS (EXPRS_EXPR (upper_bound_elements));
                INFO_PARAMS (arg_info)
                  = TBmakeExprs (TBmakeId (avis), INFO_PARAMS (arg_info));
                new_avis = DUPdoDupNode (avis);
                INFO_ARGS (arg_info) = TBmakeArg (new_avis, INFO_ARGS (arg_info));
            default:
                break;
            }
            switch (NODE_TYPE (EXPRS_EXPR (lower_bound_elements))) {
            case N_num:
                INFO_PARAMS (arg_info) = TBmakeExprs (EXPRS_EXPR (lower_bound_elements),
                                                      INFO_PARAMS (arg_info));

                char *num_str = STRitoa (count);
                char *name_str = STRcat ("lb", num_str);
                num_str = MEMfree (num_str);
                avis = TBmakeAvis (name_str, TYmakeSimpleType (T_int));
                num_str = MEMfree (name_str);
                INFO_ARGS (arg_info) = TBmakeArg (avis, INFO_ARGS (arg_info));
                break;
            case N_id:
                avis = ID_AVIS (EXPRS_EXPR (lower_bound_elements));
                INFO_PARAMS (arg_info)
                  = TBmakeExprs (TBmakeId (avis), INFO_PARAMS (arg_info));
                new_avis = DUPdoDupNode (avis);
                INFO_ARGS (arg_info) = TBmakeArg (new_avis, INFO_ARGS (arg_info));
            default:
                break;
            }

            lower_bound_elements = EXPRS_NEXT (lower_bound_elements);
            upper_bound_elements = EXPRS_NEXT (upper_bound_elements);
            count++;
        }
    }

    DBUG_RETURN (arg_node);
}
