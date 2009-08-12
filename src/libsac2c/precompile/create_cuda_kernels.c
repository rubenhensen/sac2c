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

static int ids_dim = 0;

/**
 * INFO structure
 */

struct INFO {
    node *cudakernels;
    node *cudaaps;
    node *letids;
    node *fundef;
    lut_t *wllut;
    lut_t *partlut;
    node *withopargs;
    node *withopparams;
    node *args;
    node *params;
    node *vardecs;
    node *rets;
    node *retexprs;
    bool collect;
    bool withid;
    bool operator;
    bool mem;
    int ls_num;
    node *allocassigns;
    node *freeassigns;
    bool is_wlids;
    node *wlids;
    bool is_wlidxs;
    node *prfwlids;
    node *prfwlidxs;
    node *prfgridblock;
    bool hasstepwidth;
};

/**
 * INFO macros
 */

#define INFO_CUDAKERNELS(n) (n->cudakernels)
#define INFO_CUDAAPS(n) (n->cudaaps)
#define INFO_LETIDS(n) (n->letids)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_WLLUT(n) (n->wllut)
#define INFO_PARTLUT(n) (n->partlut)
#define INFO_ARGS(n) (n->args)
#define INFO_PARAMS(n) (n->params)
#define INFO_WITHOPARGS(n) (n->withopargs)
#define INFO_WITHOPPARAMS(n) (n->withopparams)
#define INFO_VARDECS(n) (n->vardecs)
#define INFO_RETS(n) (n->rets)
#define INFO_RETEXPRS(n) (n->retexprs)
#define INFO_COLLECT(n) (n->collect)
#define INFO_WITHID(n) (n->withid)
#define INFO_OPERATOR(n) (n->operator)
#define INFO_MEM(n) (n->mem)
#define INFO_LS_NUM(n) (n->ls_num)
#define INFO_ALLOCASSIGNS(n) (n->allocassigns)
#define INFO_FREEASSIGNS(n) (n->freeassigns)
#define INFO_IS_WLIDS(n) (n->is_wlids)
#define INFO_WLIDS(n) (n->wlids)
#define INFO_IS_WLIDXS(n) (n->is_wlidxs)
#define INFO_PRFWLIDS(n) (n->prfwlids)
#define INFO_PRFWLIDXS(n) (n->prfwlidxs)
#define INFO_PRFGRIDBLOCK(n) (n->prfgridblock)
#define INFO_HASSTEPWIDTH(n) (n->hasstepwidth)

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
    INFO_WLLUT (result) = NULL;
    INFO_PARTLUT (result) = NULL;
    INFO_WITHOPARGS (result) = NULL;
    INFO_WITHOPPARAMS (result) = NULL;
    INFO_ARGS (result) = NULL;
    INFO_PARAMS (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_RETS (result) = NULL;
    INFO_RETEXPRS (result) = NULL;
    INFO_COLLECT (result) = FALSE;
    INFO_WITHID (result) = FALSE;
    INFO_OPERATOR (result) = FALSE;
    INFO_MEM (result) = FALSE;
    INFO_LS_NUM (result) = 1;
    INFO_ALLOCASSIGNS (result) = NULL;
    INFO_FREEASSIGNS (result) = NULL;
    INFO_IS_WLIDS (result) = FALSE;
    INFO_WLIDS (result) = NULL;
    INFO_IS_WLIDXS (result) = FALSE;
    INFO_PRFWLIDS (result) = NULL;
    INFO_PRFWLIDXS (result) = NULL;
    INFO_PRFGRIDBLOCK (result) = NULL;
    INFO_HASSTEPWIDTH (result) = FALSE;

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
 * @brief SetLinksignInfo( node *withopargs,
 *                         node *args,
 *                         info *arg_info)
 *
 * @param
 * @param
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
static void
SetLinksignInfo (node *withopargs, node *args, info *arg_info)
{
    node *_withopargs;
    node *_args;

    _withopargs = withopargs;
    _args = args;

    while (_withopargs != NULL) {
        if (!ARG_HASLINKSIGNINFO (_withopargs)) {
            ARG_LINKSIGN (_withopargs) = INFO_LS_NUM (arg_info);
            ARG_HASLINKSIGNINFO (_withopargs) = TRUE;
            INFO_LS_NUM (arg_info) += 1;
        }
        _withopargs = ARG_NEXT (_withopargs);
    }

    while (_args != NULL) {
        if (!ARG_HASLINKSIGNINFO (_args)) {
            ARG_LINKSIGN (_args) = INFO_LS_NUM (arg_info);
            ARG_HASLINKSIGNINFO (_args) = TRUE;
            INFO_LS_NUM (arg_info) += 1;
        }
        _args = ARG_NEXT (_args);
    }
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
    node *cuda_kerneldef, *fundef, *body, *retexprs, *vardecs;
    node *args, *rets, *retur, *dupargs;
    node *allocassigns, *freeassigns;
    node *prfwlids, *prfwlidxs;

    DBUG_ENTER ("CreateCudaKernelDdef");

    fundef = INFO_FUNDEF (arg_info);

    retexprs = INFO_RETEXPRS (arg_info);

    vardecs = INFO_VARDECS (arg_info);
    INFO_VARDECS (arg_info) = NULL;

    rets = INFO_RETS (arg_info);

    dupargs = DUPdoDupTree (INFO_WITHOPARGS (arg_info));
    SetLinksignInfo (dupargs, INFO_ARGS (arg_info), arg_info);
    args = TCappendArgs (dupargs, INFO_ARGS (arg_info));
    INFO_ARGS (arg_info) = NULL;

    allocassigns = INFO_ALLOCASSIGNS (arg_info);
    INFO_ALLOCASSIGNS (arg_info) = NULL;

    prfwlids = INFO_PRFWLIDS (arg_info);
    INFO_PRFWLIDS (arg_info) = NULL;

    prfwlidxs = INFO_PRFWLIDXS (arg_info);

    /* Set the DECL of the 2nd F_cuda_wlidxs argument to the First function arg */
    AVIS_DECL (ID_AVIS (PRF_ARG2 (LET_EXPR (ASSIGN_INSTR (prfwlidxs))))) = dupargs;

    INFO_PRFWLIDXS (arg_info) = NULL;

    freeassigns = INFO_FREEASSIGNS (arg_info);
    INFO_FREEASSIGNS (arg_info) = NULL;

    retur = TBmakeReturn (DUPdoDupTree (retexprs));

    body = DUPdoDupTree (CODE_CBLOCK (code));

    BLOCK_INSTR (body) = TCappendAssign (
      TCappendAssign (TCappendAssign (TCappendAssign (TCappendAssign (allocassigns,
                                                                      prfwlids),
                                                      prfwlidxs),
                                      BLOCK_INSTR (body)),
                      freeassigns),
      TBmakeAssign (retur, NULL));

    BLOCK_VARDEC (body) = TCappendVardec (vardecs, BLOCK_VARDEC (body));

    cuda_kerneldef
      = TBmakeFundef (TRAVtmpVarName ("CUDA"), NSdupNamespace (FUNDEF_NS (fundef)),
                      DUPdoDupTree (rets), args, body, NULL);

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

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

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

    if (!FUNDEF_ISCUDAGLOBALFUN (arg_node) && (FUNDEF_BODY (arg_node) != NULL)) {
        /*
         * Only Non CUDA function may contain withloops which we want to lift.
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
         * We have reached the end of the FUNDEF chain. We add the new CUDA functions
         * constructed meanwhile and stored in the info structure to the end and stop
         * the traversal.
         */
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
    node *tmp;

    next = ASSIGN_NEXT (arg_node);

    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }

    if (!INFO_COLLECT (arg_info) && INFO_CUDAAPS (arg_info) != NULL) {
        arg_node = FREEdoFreeNode (arg_node);

        ret_node = INFO_CUDAAPS (arg_info);
        tmp = INFO_CUDAAPS (arg_info);
        while (ASSIGN_NEXT (tmp) != NULL) {
            tmp = ASSIGN_NEXT (tmp);
        }
        ASSIGN_NEXT (tmp) = next;
        INFO_CUDAAPS (arg_info) = NULL;
        ASSIGN_NEXT (tmp) = TRAVopt (ASSIGN_NEXT (tmp), arg_info);
    } else {
        ret_node = arg_node;
        ASSIGN_NEXT (ret_node) = TRAVopt (ASSIGN_NEXT (ret_node), arg_info);
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

    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

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
    node *avis, *new_avis, *ids, *dim, *shape, *alloc, *free;
    node *prf_wlids, *prf_wlidxs;

    lut_t *current_lut;

    DBUG_ENTER ("CUKNLid");

    avis = ID_AVIS (arg_node);
    new_avis = NULL;
    ids = NULL;
    dim = NULL;
    shape = NULL;
    alloc = NULL;
    free = NULL;

    DBUG_PRINT ("CUKNL", ("ENTER id %s", ID_NAME (arg_node)));

    if (INFO_OPERATOR (arg_info)) {
        current_lut = INFO_WLLUT (arg_info);
    } else {
        current_lut = INFO_PARTLUT (arg_info);
    }

    if (INFO_COLLECT (arg_info)) {
        if (LUTsearchInLutPp (current_lut, avis) == avis) {
            DBUG_PRINT ("CUKNL", ("  Not handled before..."));
            new_avis = DUPdoDupNode (avis);

            if (!INFO_WITHID (arg_info)) {
                if (INFO_OPERATOR (arg_info)) {
                    /* args and params from the operator: mem, default, shape etc etc */
                    INFO_WITHOPARGS (arg_info)
                      = TBmakeArg (new_avis, INFO_WITHOPARGS (arg_info));
                    INFO_WITHOPPARAMS (arg_info)
                      = TBmakeExprs (TBmakeId (avis), INFO_WITHOPPARAMS (arg_info));
                    if (INFO_MEM (arg_info)) {
                        ARG_LINKSIGN (INFO_WITHOPARGS (arg_info))
                          = INFO_LS_NUM (arg_info);
                        ARG_HASLINKSIGNINFO (INFO_WITHOPARGS (arg_info)) = TRUE;
                    }
                } else {
                    INFO_ARGS (arg_info) = TBmakeArg (new_avis, INFO_ARGS (arg_info));
                    INFO_PARAMS (arg_info)
                      = TBmakeExprs (TBmakeId (avis), INFO_PARAMS (arg_info));
                }
            }
            current_lut = LUTinsertIntoLutP (current_lut, avis, new_avis);
        }

        if (INFO_WITHID (arg_info)) {
            DBUG_PRINT ("CUKNL", ("...is Withid-id"));
            if (new_avis == NULL) {
                new_avis = LUTsearchInLutPp (current_lut, avis);
            } else {
                INFO_VARDECS (arg_info)
                  = TBmakeVardec (new_avis, INFO_VARDECS (arg_info));
            }

            ids = TBmakeIds (new_avis, NULL);

            if (TUdimKnown (AVIS_TYPE (new_avis))) {
                dim = TBmakeNum (TYgetDim (AVIS_TYPE (new_avis)));
            }

            if (TUshapeKnown (AVIS_TYPE (new_avis))) {
                shape = SHshape2Array (TYgetShape (AVIS_TYPE (new_avis)));
            }

            /* Create F_alloc and F_free for N_withid->ids and N_withid->idxs */
            alloc = TCmakePrf3 (F_alloc, TBmakeNum (1), dim, shape);
            INFO_ALLOCASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (ids, alloc), INFO_ALLOCASSIGNS (arg_info));

            free = TCmakePrf1 (F_free, TBmakeId (new_avis));
            INFO_FREEASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (NULL, free), INFO_FREEASSIGNS (arg_info));

            /* Create F_cuda_wlids and F_cuda_wlidxs for N_withid->ids and N_withid->idxs
             */
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
                exprs = TCappendExprs (DUPdoDupTree (INFO_WLIDS (arg_info)), exprs);
                exprs
                  = TBmakeExprs (TBmakeNum (TCcountExprs (INFO_WLIDS (arg_info))), exprs);
                exprs = TBmakeExprs (TBmakeId (IDS_AVIS (INFO_LETIDS (arg_info))), exprs);
                exprs = TBmakeExprs (TBmakeId (new_avis), exprs);
                prf_wlidxs = TBmakePrf (F_cuda_wlidxs, exprs);
                INFO_PRFWLIDXS (arg_info) = TBmakeAssign (TBmakeLet (NULL, prf_wlidxs),
                                                          INFO_PRFWLIDXS (arg_info));
            }
        }

        if (INFO_OPERATOR (arg_info)) {
            INFO_WLLUT (arg_info) = current_lut;
        } else {
            INFO_PARTLUT (arg_info) = current_lut;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUKNLids(node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
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
        if (LUTsearchInLutPp (INFO_PARTLUT (arg_info), avis) == avis) {
            new_avis = DUPdoDupNode (avis);
            INFO_VARDECS (arg_info) = TBmakeVardec (new_avis, INFO_VARDECS (arg_info));
            INFO_PARTLUT (arg_info)
              = LUTinsertIntoLutP (INFO_PARTLUT (arg_info), avis, new_avis);
            DBUG_PRINT ("CUKNL", (">>> ids %s added to LUT", IDS_NAME (arg_node)));
        }
    }

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

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
        /*
         * Start collecting data flow information
         */
        INFO_COLLECT (arg_info) = TRUE;

        INFO_WLLUT (arg_info) = LUTgenerateLut ();

        INFO_OPERATOR (arg_info) = TRUE;
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
        INFO_OPERATOR (arg_info) = FALSE;

        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

        INFO_WLLUT (arg_info) = LUTremoveLut (INFO_WLLUT (arg_info));
        INFO_WITHOPARGS (arg_info) = FREEdoFreeTree (INFO_WITHOPARGS (arg_info));
        INFO_WITHOPPARAMS (arg_info) = FREEdoFreeTree (INFO_WITHOPPARAMS (arg_info));
        INFO_RETS (arg_info) = FREEdoFreeTree (INFO_RETS (arg_info));
        INFO_RETEXPRS (arg_info) = FREEdoFreeTree (INFO_RETEXPRS (arg_info));
        INFO_LS_NUM (arg_info) = 1;

        INFO_COLLECT (arg_info) = FALSE;
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
    node *avis, *old_ids;

    DBUG_ENTER ("CUKNLgenarray");

    if (GENARRAY_DEFAULT (arg_node) != NULL) {
        GENARRAY_DEFAULT (arg_node) = TRAVdo (GENARRAY_DEFAULT (arg_node), arg_info);
    }

    /* commet for now and see what happends */
    if (GENARRAY_SUB (arg_node) != NULL) {
        // GENARRAY_SUB( arg_node) = TRAVdo( GENARRAY_SUB( arg_node), arg_info);
    }

    if (GENARRAY_MEM (arg_node) != NULL) {
        INFO_MEM (arg_info) = TRUE;
        GENARRAY_MEM (arg_node) = TRAVdo (GENARRAY_MEM (arg_node), arg_info);
        INFO_MEM (arg_info) = FALSE;
    }

    avis = IDS_AVIS (INFO_LETIDS (arg_info));

    /* the memval in withop is the return value */
    INFO_RETS (arg_info)
      = TCappendRet (TBmakeRet (TYeliminateAKV (AVIS_TYPE (avis)), NULL),
                     INFO_RETS (arg_info));

    RET_LINKSIGN (INFO_RETS (arg_info)) = INFO_LS_NUM (arg_info);
    RET_HASLINKSIGNINFO (INFO_RETS (arg_info)) = TRUE;
    INFO_LS_NUM (arg_info) += 1;

    INFO_RETEXPRS (arg_info)
      = TCappendExprs (TBmakeExprs (TBmakeId (avis), NULL), INFO_RETEXPRS (arg_info));

    old_ids = INFO_LETIDS (arg_info);
    if (GENARRAY_NEXT (arg_node) != NULL) {
        INFO_LETIDS (arg_info) = IDS_NEXT (INFO_LETIDS (arg_info));
        DBUG_ASSERT (INFO_LETIDS (arg_info) != NULL, "#ids != #genarrays!");
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }
    INFO_LETIDS (arg_info) = old_ids;

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
    node *avis, *old_ids;

    DBUG_ENTER ("CUKNLmodarray");

    if (MODARRAY_SUB (arg_node) != NULL) {
        MODARRAY_SUB (arg_node) = TRAVdo (MODARRAY_SUB (arg_node), arg_info);
    }

    if (MODARRAY_MEM (arg_node) != NULL) {
        INFO_MEM (arg_info) = TRUE;
        MODARRAY_MEM (arg_node) = TRAVdo (MODARRAY_MEM (arg_node), arg_info);
        INFO_MEM (arg_info) = FALSE;
    }

    avis = IDS_AVIS (INFO_LETIDS (arg_info));

    /* the memval in withop is the return value */
    INFO_RETS (arg_info)
      = TCappendRet (TBmakeRet (TYeliminateAKV (AVIS_TYPE (avis)), NULL),
                     INFO_RETS (arg_info));

    RET_LINKSIGN (INFO_RETS (arg_info)) = INFO_LS_NUM (arg_info);
    RET_HASLINKSIGNINFO (INFO_RETS (arg_info)) = TRUE;
    INFO_LS_NUM (arg_info) += 1;

    INFO_RETEXPRS (arg_info)
      = TCappendExprs (TBmakeExprs (TBmakeId (avis), NULL), INFO_RETEXPRS (arg_info));

    old_ids = INFO_LETIDS (arg_info);
    if (MODARRAY_NEXT (arg_node) != NULL) {
        INFO_LETIDS (arg_info) = IDS_NEXT (INFO_LETIDS (arg_info));
        DBUG_ASSERT (INFO_LETIDS (arg_info) != NULL, "#ids != #modarrays!");
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }
    INFO_LETIDS (arg_info) = old_ids;

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
    node *avis, *new_avis;

    DBUG_ENTER ("CUKNLwithid");

    INFO_WITHID (arg_info) = TRUE;
    INFO_WLIDS (arg_info) = WITHID_IDS (arg_node);

    /* WITHID_VEC should be treated seperately, because we don't want
     * to allocate memory within CUDA kernels for the index vector
     */
    if (WITHID_VEC (arg_node) != NULL) {
        avis = ID_AVIS (WITHID_VEC (arg_node));
        new_avis = DUPdoDupNode (avis);
        INFO_PARTLUT (arg_info)
          = LUTinsertIntoLutP (INFO_PARTLUT (arg_info), avis, new_avis);
    }

    /* traverse order of WLids and WLidxs must not be changed because WLidxs need WLids */
    if (WITHID_IDS (arg_node) != NULL) {
        ids_dim = 0;
        INFO_IS_WLIDS (arg_info) = TRUE;
        WITHID_IDS (arg_node) = TRAVdo (WITHID_IDS (arg_node), arg_info);
        INFO_IS_WLIDS (arg_info) = FALSE;
        ids_dim = 0;
    }

    if (WITHID_IDXS (arg_node) != NULL) {
        INFO_IS_WLIDXS (arg_info) = TRUE;
        WITHID_IDXS (arg_node) = TRAVdo (WITHID_IDXS (arg_node), arg_info);
        INFO_IS_WLIDXS (arg_info) = FALSE;
    }

    INFO_WITHID (arg_info) = FALSE;
    INFO_WLIDS (arg_info) = NULL;

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

    if (CODE_CBLOCK (arg_node) != NULL) {
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
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
    DBUG_ENTER ("CUKNLpart");

    node *cuda_kernel;
    node *old_ids;
    node *cuda_funap = NULL;
    int old_ls_num;

    /* initially, each partion's LUT is a duplicate of the withop's LUT */
    INFO_PARTLUT (arg_info) = LUTduplicateLut (INFO_WLLUT (arg_info));

    old_ids = INFO_LETIDS (arg_info);
    old_ls_num = INFO_LS_NUM (arg_info);

    if (PART_WITHID (arg_node) != NULL) {
        PART_WITHID (arg_node) = TRAVdo (PART_WITHID (arg_node), arg_info);
    }

    if (PART_CODE (arg_node) != NULL) {
        PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);
    }

    if (PART_GENERATOR (arg_node) != NULL) {
        PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);
    }

    INFO_LETIDS (arg_info) = old_ids;

    cuda_kernel = CreateCudaKernelDef (PART_CODE (arg_node), arg_info);
    FUNDEF_NEXT (cuda_kernel) = INFO_CUDAKERNELS (arg_info);
    INFO_CUDAKERNELS (arg_info) = cuda_kernel;

    cuda_funap
      = TBmakeAssign (TBmakeLet (DUPdoDupTree (INFO_LETIDS (arg_info)),
                                 TBmakeAp (cuda_kernel,
                                           TCappendExprs (DUPdoDupTree (
                                                            INFO_WITHOPPARAMS (arg_info)),
                                                          INFO_PARAMS (arg_info)))),
                      NULL);

    INFO_CUDAAPS (arg_info)
      = TCappendAssign (INFO_PRFGRIDBLOCK (arg_info),
                        TCappendAssign (cuda_funap, INFO_CUDAAPS (arg_info)));

    INFO_ARGS (arg_info) = NULL;
    INFO_PARAMS (arg_info) = NULL;
    INFO_PRFGRIDBLOCK (arg_info) = NULL;
    INFO_PARTLUT (arg_info) = LUTremoveLut (INFO_PARTLUT (arg_info));

    INFO_LS_NUM (arg_info) = old_ls_num;

    if (PART_NEXT (arg_node) != NULL) {
        PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
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

    node *lower_bound_elements, *upper_bound_elements;
    node *step_elements, *width_elements;
    node *avis, *new_avis;
    node *gridblock_exprs = NULL;
    char *bound_name;
    char *step_name, *width_name;
    int dim = 0;

    lower_bound_elements = ARRAY_AELEMS (GENERATOR_BOUND1 (arg_node));
    upper_bound_elements = ARRAY_AELEMS (GENERATOR_BOUND2 (arg_node));

    while (lower_bound_elements != NULL && upper_bound_elements != NULL) {
        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (upper_bound_elements)) == N_id),
                     "Upper bound should be an array of N_id nodes");
        avis = ID_AVIS (EXPRS_EXPR (upper_bound_elements));
        INFO_PARAMS (arg_info) = TBmakeExprs (TBmakeId (avis), INFO_PARAMS (arg_info));
        gridblock_exprs = TBmakeExprs (TBmakeId (avis), gridblock_exprs);
        new_avis = DUPdoDupNode (avis);
        AVIS_NAME (new_avis) = MEMfree (AVIS_NAME (new_avis));
        bound_name = (char *)MEMmalloc (sizeof (char) * (STRlen ("_ub_") + 2));
        sprintf (bound_name, "%s%d", "_ub_", dim);
        AVIS_NAME (new_avis) = bound_name;
        INFO_ARGS (arg_info) = TBmakeArg (new_avis, INFO_ARGS (arg_info));
        AVIS_DECL (new_avis) = INFO_ARGS (arg_info);
        ARG_LINKSIGN (INFO_ARGS (arg_info)) = INFO_LS_NUM (arg_info);
        ARG_HASLINKSIGNINFO (INFO_ARGS (arg_info)) = TRUE;
        INFO_LS_NUM (arg_info) += 1;

        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (lower_bound_elements)) == N_id),
                     "Lower bound should be an array of N_id nodes");
        avis = ID_AVIS (EXPRS_EXPR (lower_bound_elements));
        INFO_PARAMS (arg_info) = TBmakeExprs (TBmakeId (avis), INFO_PARAMS (arg_info));
        gridblock_exprs = TBmakeExprs (TBmakeId (avis), gridblock_exprs);
        new_avis = DUPdoDupNode (avis);
        AVIS_NAME (new_avis) = MEMfree (AVIS_NAME (new_avis));
        bound_name = (char *)MEMmalloc (sizeof (char) * (STRlen ("_lb_") + 2));
        sprintf (bound_name, "%s%d", "_lb_", dim);
        AVIS_NAME (new_avis) = bound_name;
        INFO_ARGS (arg_info) = TBmakeArg (new_avis, INFO_ARGS (arg_info));
        AVIS_DECL (new_avis) = INFO_ARGS (arg_info);
        ARG_LINKSIGN (INFO_ARGS (arg_info)) = INFO_LS_NUM (arg_info);
        ARG_HASLINKSIGNINFO (INFO_ARGS (arg_info)) = TRUE;
        INFO_LS_NUM (arg_info) += 1;

        lower_bound_elements = EXPRS_NEXT (lower_bound_elements);
        upper_bound_elements = EXPRS_NEXT (upper_bound_elements);
        dim++;
    }

    INFO_PRFGRIDBLOCK (arg_info)
      = TBmakeAssign (TBmakeLet (NULL, TBmakePrf (F_cuda_grid_block, gridblock_exprs)),
                      NULL);

    dim = 0;

    if (GENERATOR_STEP (arg_node) != NULL && GENERATOR_WIDTH (arg_node) != NULL) {
        INFO_HASSTEPWIDTH (arg_info) = TRUE;

        step_elements = ARRAY_AELEMS (GENERATOR_STEP (arg_node));
        width_elements = ARRAY_AELEMS (GENERATOR_WIDTH (arg_node));

        while (step_elements != NULL && width_elements != NULL) {
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (step_elements)) == N_id),
                         "Step should be an array of N_id nodes");
            avis = ID_AVIS (EXPRS_EXPR (step_elements));
            INFO_PARAMS (arg_info)
              = TBmakeExprs (TBmakeId (avis), INFO_PARAMS (arg_info));
            new_avis = DUPdoDupNode (avis);
            AVIS_NAME (new_avis) = MEMfree (AVIS_NAME (new_avis));
            step_name = (char *)MEMmalloc (sizeof (char) * (STRlen ("_step_") + 2));
            sprintf (step_name, "%s%d", "_step_", dim);
            AVIS_NAME (new_avis) = step_name;
            INFO_ARGS (arg_info) = TBmakeArg (new_avis, INFO_ARGS (arg_info));
            AVIS_DECL (new_avis) = INFO_ARGS (arg_info);
            ARG_LINKSIGN (INFO_ARGS (arg_info)) = INFO_LS_NUM (arg_info);
            ARG_HASLINKSIGNINFO (INFO_ARGS (arg_info)) = TRUE;
            INFO_LS_NUM (arg_info) += 1;

            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (width_elements)) == N_id),
                         "Width should be an array of N_id nodes");
            avis = ID_AVIS (EXPRS_EXPR (width_elements));
            INFO_PARAMS (arg_info)
              = TBmakeExprs (TBmakeId (avis), INFO_PARAMS (arg_info));
            new_avis = DUPdoDupNode (avis);
            AVIS_NAME (new_avis) = MEMfree (AVIS_NAME (new_avis));
            width_name = (char *)MEMmalloc (sizeof (char) * (STRlen ("_width_") + 2));
            sprintf (width_name, "%s%d", "_width_", dim);
            AVIS_NAME (new_avis) = width_name;
            INFO_ARGS (arg_info) = TBmakeArg (new_avis, INFO_ARGS (arg_info));
            AVIS_DECL (new_avis) = INFO_ARGS (arg_info);
            ARG_LINKSIGN (INFO_ARGS (arg_info)) = INFO_LS_NUM (arg_info);
            ARG_HASLINKSIGNINFO (INFO_ARGS (arg_info)) = TRUE;
            INFO_LS_NUM (arg_info) += 1;

            step_elements = EXPRS_NEXT (step_elements);
            width_elements = EXPRS_NEXT (width_elements);
            dim++;
        }
    }

    DBUG_RETURN (arg_node);
}

/*
static node *HandleBoundExprs(node *bound_exprs, info *arg_info)
{
  node *avis, *new_avis;

  DBUG_ENTER("HandleBoundExprs");

  if( EXPRS_NEXT( bound_exprs) != NULL) {
    EXPRS_NEXT( bound_exprs) = HandleBoundExprs( EXPRS_NEXT( bound_exprs), arg_info);
  }
    DBUG_ASSERT( ( NODE_TYPE( EXPRS_EXPR( bound_exprs)) == N_id),
                   "Bound elements should be an array of N_id nodes");
    avis = ID_AVIS( EXPRS_EXPR( bound_exprs));
    INFO_PARAMS(arg_info) = TBmakeExprs( TBmakeId( avis),
                                           INFO_PARAMS( arg_info));
    new_avis = DUPdoDupNode( avis);
    INFO_ARGS(arg_info) = TBmakeArg( new_avis, INFO_ARGS( arg_info));
    AVIS_DECL( new_avis) = INFO_ARGS(arg_info);
    ARG_LINKSIGN( INFO_ARGS(arg_info)) = INFO_LS_NUM( arg_info);
    ARG_HASLINKSIGNINFO( INFO_ARGS(arg_info)) = TRUE;
    INFO_LS_NUM( arg_info) += 1;


  DBUG_RETURN( bound_exprs);
}

node *CUKNLgenerator( node *arg_node, info *arg_info)
{
  DBUG_ENTER("CUKNLgenerator");

  node *lower_bound_elements, *upper_bound_elements;

  lower_bound_elements = ARRAY_AELEMS( GENERATOR_BOUND1( arg_node));
  upper_bound_elements = ARRAY_AELEMS( GENERATOR_BOUND2( arg_node));

  upper_bound_elements = HandleBoundExprs(upper_bound_elements, arg_info);
  lower_bound_elements = HandleBoundExprs(lower_bound_elements, arg_info);

  DBUG_RETURN( arg_node);
}
*/

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUKNLwith2( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUKNLwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUKNLwith2");

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
    node *ret_node = NULL;
    node *args;

    DBUG_ENTER ("CUKNLprf");

    if (INFO_COLLECT (arg_info)) {
        switch (PRF_PRF (arg_node)) {
        case F_wl_assign:
            PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
            args
              = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                             TBmakeExprs (DUPdoDupNode (PRF_ARG2 (arg_node)),
                                          TBmakeExprs (DUPdoDupNode (PRF_ARG4 (arg_node)),
                                                       NULL)));
            ret_node = TBmakePrf (F_cuda_wl_assign, args);
            arg_node = FREEdoFreeTree (arg_node);
            break;
        case F_suballoc:
            PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
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
