/*****************************************************************************
 *
 * $Id$
 *
 * file: lw3.c
 *
 * prefix: LW3
 *
 * Description:
 *
 *   This traversal lifts with3 bodies into functions
 *
 * usage of arg_info (INFO_...):
 *
 *    ...THREADS    pointer to a chain of thread fundefs to be added to the
 *                  syntax tree
 *    ...THREADNO   number of thread functions created
 *    ...NS         namespace to create thread functions in
 *
 *****************************************************************************/

#include "lift_with3_bodies.h"
#include "dbug.h"
#include "memory.h"
#include "LookUpTable.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"
#include "tree_basic.h"
#include "str.h"
#include "str_buffer.h"
#include "DupTree.h"
#include "namespaces.h"
#include "node_basic.h"
#include "traverse.h"
#include "cleanup_decls.h"
#include "remove_dfms.h"
#include "tree_compound.h"
#include "infer_dfms.h"
#include "free.h"

/*
 * INFO structure
 */
struct INFO {
    node *threads;
    int threadno;
    namespace_t *ns;
};

/*
 * INFO macros
 */
#define INFO_THREADS(n) ((n)->threads)
#define INFO_THREADNO(n) ((n)->threadno)
#define INFO_NS(n) ((n)->ns)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_THREADS (result) = NULL;
    INFO_THREADNO (result) = 0;
    INFO_NS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!-- ****************************************************************** -->
 * @fn char *CreateThreadFunName(info *arg_info)
 *
 * @brief Creates a name for a new thread function
 *
 * @param arg_info info structure to access global counter of thread functions
 *
 * @return the newly created name
 ******************************************************************************/
static char *
CreateThreadFunName (info *arg_info)
{
    str_buf *buffer;
    char *name;

    DBUG_ENTER ("CreateThreadFunName");

    buffer = SBUFcreate (16);

    buffer = SBUFprintf (buffer, "threadFun_%i", INFO_THREADNO (arg_info));
    INFO_THREADNO (arg_info)++;

    name = SBUF2str (buffer);
    buffer = SBUFfree (buffer);

    DBUG_RETURN (name);
}

/** <!-- ****************************************************************** -->
 * @fn node *CreateThreadFunction(node *block, info *arg_info)
 *
 * @brief Lifts the body argument into a thread function.
 *
 * @param block    block to be lifted into the function
 * @param index    avis of index variable of this range
 * @param arg_info info structure to store new function
 *
 * @return N_ap node for outer context to call the newly created thread
 *         function
 ******************************************************************************/
static node *
CreateThreadFunction (node *block, node *index, info *arg_info)
{
    lut_t *lut;
    node *args, *rets, *retassign, *threadfun, *ap, *vardecs, *assigns;
    node *innerindex;
    char *funName;
    dfmask_t *ret_mask, *arg_mask, *local_mask;

    DBUG_ENTER ("CreateThreadFunction");

    lut = LUTgenerateLut ();

    /*
     * We have to massage the masks slightly to make the index an
     * artificial local variable that we will later explicitly
     * tag as an index. We can do so, as we no longer are in SSA
     * form, so no actual defining assignment is needed.
     */
    ret_mask = DFMgenMaskMinus (BLOCK_OUT_MASK (block), BLOCK_IN_MASK (block));
    arg_mask = DFMgenMaskCopy (BLOCK_IN_MASK (block));
    DFMsetMaskEntryClear (arg_mask, NULL, index);
    local_mask = DFMgenMaskCopy (BLOCK_LOCAL_MASK (block));
    DFMsetMaskEntrySet (local_mask, NULL, index);

    args = DFMUdfm2Args (arg_mask, lut);
    rets = DFMUdfm2Rets (ret_mask);
    vardecs = DFMUdfm2Vardecs (local_mask, lut);

    retassign = TBmakeAssign (TBmakeReturn (DFMUdfm2ReturnExprs (ret_mask, lut)), NULL);

    assigns = TCappendAssign (DUPdoDupTreeLut (BLOCK_INSTR (block), lut), retassign);

    funName = CreateThreadFunName (arg_info);
    threadfun = TBmakeFundef (funName, NSdupNamespace (INFO_NS (arg_info)), rets, args,
                              TBmakeBlock (assigns, vardecs), INFO_THREADS (arg_info));
    INFO_THREADS (arg_info) = threadfun;

    FUNDEF_RETURN (threadfun) = ASSIGN_INSTR (retassign);
    FUNDEF_ISTHREADFUN (threadfun) = TRUE;

    /*
     * tag the index variable
     */
    innerindex = LUTsearchInLutPp (lut, index);
    if (innerindex != index) {
        AVIS_ISTHREADINDEX (innerindex) = TRUE;
    }

    lut = LUTremoveLut (lut);

    ap = TBmakeAp (threadfun, DFMUdfm2ApArgs (arg_mask, NULL));

    ret_mask = DFMremoveMask (ret_mask);
    arg_mask = DFMremoveMask (arg_mask);
    local_mask = DFMremoveMask (local_mask);

    DBUG_RETURN (ap);
}

/******************************************************************************
 *
 * function:
 *   node *LW3module( node *syntax_tree)
 *
 * description:
 *
 *
 *****************************************************************************/
node *
LW3module (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("LW3module");

    DBUG_ASSERT ((MODULE_THREADFUNS (arg_node) == NULL),
                 "Thread functions are already in this module");

    INFO_NS (arg_info) = MODULE_NAMESPACE (arg_node);

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    MODULE_THREADFUNS (arg_node) = INFO_THREADS (arg_info);

    INFO_THREADS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *LW3range( node *arg_node, info *arg_info)
 *
 * @brief Triggers the lifting of the body into a thread function.
 *
 * @param arg_node N_range node
 * @param arg_info info structure
 *
 * @return N_range node with lifted body
 ******************************************************************************/
node *
LW3range (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("LW3range");

    /*
     * we perform the transformation depth first / bottom up
     */
    RANGE_BODY (arg_node) = TRAVdo (RANGE_BODY (arg_node), arg_info);

    RANGE_RESULTS (arg_node)
      = CreateThreadFunction (RANGE_BODY (arg_node), ID_AVIS (RANGE_INDEX (arg_node)),
                              arg_info);

    RANGE_BODY (arg_node) = TBmakeBlock (TBmakeEmpty (), NULL);

    RANGE_NEXT (arg_node) = TRAVopt (RANGE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *LW3doLiftWith3( node *syntax_tree)
 *
 * @brief Lifts out the bodies of with loop3s into functions.
 *
 * @param syntax_tree N_module node of current syntax tree
 *
 * @return module with all with3 bodies lifted into thread functions.
 ******************************************************************************/
node *
LW3doLiftWith3 (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("LW3doLiftWith3");

    /*
     * Infer dataflow masks
     */
    syntax_tree = INFDFMSdoInferDfms (syntax_tree, HIDE_LOCALS_WITH3);

    info = MakeInfo ();

    TRAVpush (TR_lw3);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    /*
     * clean up tree
     */
    syntax_tree = CUDdoCleanupDecls (syntax_tree);
    syntax_tree = RDFMSdoRemoveDfms (syntax_tree);

    DBUG_RETURN (syntax_tree);
}
