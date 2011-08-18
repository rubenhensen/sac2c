

#include "single_thread_kernels.h"
#include "tree_basic.h"
#include "traverse.h"
#include "str.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "memory.h"
#include "tree_compound.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"
#include "remove_dfms.h"
#include "infer_dfms.h"
#include "namespaces.h"
#include "LookUpTable.h"
#include "free.h"
#include "DupTree.h"

/*
 * INFO structure
 */
struct INFO {
    node *st_kernels;
    namespace_t *ns;
    node *ap;
};

#define INFO_ST_KERNELS(n) (n->st_kernels)
#define INFO_NS(n) (n->ns)
#define INFO_AP(n) (n->ap)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_ST_KERNELS (result) = NULL;
    INFO_NS (result) = NULL;
    INFO_AP (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

node *
STKNLdoSingleThreadKernels (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    /*
     * Infer dataflow masks
     */
    syntax_tree = INFDFMSdoInferDfms (syntax_tree, HIDE_LOCALS_NEVER);

    TRAVpush (TR_stknl);
    syntax_tree = TRAVdo (syntax_tree, arg_info);

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief STKNLmodule( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
STKNLmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_NS (arg_info) = MODULE_NAMESPACE (arg_node);

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief STKNLfundef( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
STKNLfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    } else {
        /* We have reached the end of the N_fundef chain,
         * append the newly created CUDA kernels.*/
        FUNDEF_NEXT (arg_node) = INFO_ST_KERNELS (arg_info);
        INFO_ST_KERNELS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief STKNLassign( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
STKNLassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_AP (arg_info) != NULL) {
        arg_node = FREEdoFreeNode (arg_node);
        ASSIGN_NEXT (INFO_AP (arg_info)) = arg_node;
        arg_node = INFO_AP (arg_info);
        INFO_AP (arg_info) = NULL;
    }
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief STKNLcudast( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
STKNLcudast (node *arg_node, info *arg_info)
{

    lut_t *lut;
    node *args, *rets, *retassign, *st_kernel, *ids, *ap, *vardecs, *assigns;
    dfmask_t *ret_mask, *arg_mask, *local_mask;
    node *region;

    DBUG_ENTER ();

    region = CUDAST_REGION (arg_node);

    ret_mask = DFMgenMaskMinus (BLOCK_OUT_MASK (region), BLOCK_IN_MASK (region));
    arg_mask = DFMgenMaskCopy (BLOCK_IN_MASK (region));
    local_mask = DFMgenMaskCopy (BLOCK_LOCAL_MASK (region));

    lut = LUTgenerateLut ();

    args = DFMUdfm2Args (arg_mask, lut);
    rets = DFMUdfm2Rets (ret_mask);
    vardecs = DFMUdfm2Vardecs (local_mask, lut);

    retassign = TBmakeAssign (TBmakeReturn (DFMUdfm2ReturnExprs (ret_mask, lut)), NULL);

    assigns = TCappendAssign (DUPdoDupTreeLut (BLOCK_ASSIGNS (region), lut), retassign);

    st_kernel = TBmakeFundef (TRAVtmpVarName ("CUDAST"),
                              NSdupNamespace (INFO_NS (arg_info)), rets, args,
                              TBmakeBlock (assigns, vardecs), INFO_ST_KERNELS (arg_info));

    INFO_ST_KERNELS (arg_info) = st_kernel;

    FUNDEF_RETURN (st_kernel) = ASSIGN_STMT (retassign);
    FUNDEF_ISCUDASTGLOBALFUN (st_kernel) = TRUE;

    lut = LUTremoveLut (lut);

    /* Create ap for OUTER context */
    ap = TBmakeAp (st_kernel, DFMUdfm2ApArgs (arg_mask, NULL));
    ids = DFMUdfm2LetIds (ret_mask, NULL);
    INFO_AP (arg_info) = TBmakeAssign (TBmakeLet (ids, ap), NULL);

    while (ids != NULL) {
        AVIS_SSAASSIGN (IDS_AVIS (ids)) = INFO_AP (arg_info);
        ids = IDS_NEXT (ids);
    }

    ret_mask = DFMremoveMask (ret_mask);
    arg_mask = DFMremoveMask (arg_mask);
    local_mask = DFMremoveMask (local_mask);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
