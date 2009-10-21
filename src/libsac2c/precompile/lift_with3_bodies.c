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
 *   This traversal lifts with3 bodies into thread functions
 *
 *
 *
 *  res = with3{
 *     ( lb <= i < ub) {
 *       <code1>
 *     } : id;
 *     ...
 *   } : withop;
 *
 *
 *  type( id) threadFun0(FV(<code1>)\{i}){
 *   index i;
 *   <code1>
 *   return( id);
 *  }
 *
 *  res = with3 {
 *    ( lb <= i < ub) : threadFun0(FV(<code1>)\{i});
 *    ...
 *  } : withop;
 *
 *  withop can be modarray genarray.
 *
 *  The function FV is..
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
#include "new_types.h"

/*
 * INFO structure
 */
struct INFO {
    node *threads;
    int threadno;
    namespace_t *ns;
    node *fundef;
    node *vardecs;
    node *lhs;
    node *withops;
    lut_t *withops_ids;
    int ranges;

    lut_t *at_lut;
    lut_t *at_init_lut;
    node *at_exprs_ids;
};

/*
 * INFO macros
 */
#define INFO_THREADS(n) ((n)->threads)
#define INFO_THREADNO(n) ((n)->threadno)
#define INFO_NS(n) ((n)->ns)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_WITHOPS(n) ((n)->withops)
#define INFO_WITHOPS_IDS(n) ((n)->withops_ids)
#define INFO_RANGES(n) ((n)->ranges)
/* Special meaning for anon traversal */
#define INFO_AT_LUT(n) ((n)->at_lut)
#define INFO_AT_INIT_LUT(n) ((n)->at_init_lut)
#define INFO_AT_EXPRS_IDS(n) ((n)->at_exprs_ids)

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
    INFO_FUNDEF (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_WITHOPS (result) = NULL;
    INFO_WITHOPS_IDS (result) = LUTgenerateLut ();
    INFO_RANGES (result) = 0;

    INFO_AT_LUT (result) = LUTgenerateLut ();
    INFO_AT_INIT_LUT (result) = LUTgenerateLut ();
    INFO_AT_EXPRS_IDS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    DBUG_ASSERT ((INFO_AT_EXPRS_IDS (info) == NULL),
                 "Leaking memory in AT_EXPRS_IDS chain");

    INFO_AT_LUT (info) = LUTremoveLut (INFO_AT_LUT (info));
    INFO_AT_INIT_LUT (info) = LUTremoveLut (INFO_AT_INIT_LUT (info));

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!-- ****************************************************************** -->
 * @fn node *ATravFundef( node *arg_node, info *arg_info)
 *
 * @brief Insert any generated vardecs into ast.
 *****************************************************************************/
static node *
ATravFundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ATravFundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        FUNDEF_VARDEC (arg_node)
          = TCappendVardec (FUNDEF_VARDEC (arg_node), INFO_VARDECS (arg_info));

        INFO_VARDECS (arg_info) = NULL;
    }

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *Exprs2IdsWhenFold( node *exprs, node *ops)
 *
 * @brief Convert a chain of exprs->id into a chain of ids removing all exprs
 *        that are not associated with a fold
 *
 *        expr -> id -> avis1             fold
 *         |                               |
 *         +-> expr -> id -> avis2         +-> genarray
 *              |                             |
 *              +-> expr -> id ->avis3        +-> fold
 *
 *        becomes:
 *
 *        ids -> avis1
 *         |
 *         +-> ids -> avis3
 *
 *        NOTE: avis3 is discarded as it is not associated with a fold
 *
 * @param exprs Chain of exprs where each expr is an Id
 * @param ops   Chain of withops
 *
 * @return Chain of ids
 *****************************************************************************/
static node *
Exprs2IdsWhenFold (node *exprs, node *ops, lut_t *lut)
{
    node *ids = NULL;
    node *next;
    DBUG_ENTER ("Exprs2IdsWhenFold");

    if (exprs != NULL) {
        DBUG_ASSERT ((ops != NULL), "Results and withops have different lengths");
        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (exprs)) == N_id),
                     "Expected an id in result");

        next = Exprs2IdsWhenFold (EXPRS_NEXT (exprs), WITHOP_NEXT (ops), lut);

        if (NODE_TYPE (ops) == N_fold) {
            ids = TBmakeIds (ID_AVIS (EXPRS_EXPR (exprs)), next);
            lut = LUTinsertIntoLutP (lut, IDS_AVIS (ids), ops);
        }
    }
    /*
     * ops may be longer as there may be more non folds
     * should this be checked for?
     */

    DBUG_RETURN (ids);
}

/** <!-- ****************************************************************** -->
 * @fn node *PairWithopsIds( node *arg_node, info *arg_info)
 *
 * @brief Pair withops with ids in to a lut
 *        withop -> ids
 *****************************************************************************/
static lut_t *
PairWithopsIds (lut_t *lut, node *withops, node *ids)
{
    DBUG_ENTER ("PairWithopsIds");

    if (withops != NULL) {
        DBUG_ASSERT ((ids != NULL), "Less ids than withops");
        lut = LUTinsertIntoLutP (lut, withops, ids);

        lut = PairWithopsIds (lut, WITHOP_NEXT (withops), IDS_NEXT (ids));
    }

    DBUG_RETURN (lut);
}
/** <!-- ****************************************************************** -->
 * @fn node *ATravWith3( node *arg_node, info *arg_info)
 *
 * @brief Save withops into info
 *****************************************************************************/
static node *
ATravWith3 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ATravWith3");

    INFO_WITHOPS (arg_info) = WITH3_OPERATIONS (arg_node);

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_WITHOPS_IDS (arg_info)
      = PairWithopsIds (INFO_WITHOPS_IDS (arg_info), WITH3_OPERATIONS (arg_node),
                        INFO_LHS (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *ATravRange( node *arg_node, info *arg_info)
 *
 * @brief Save fold results of this range into info as a chain of ids
 *****************************************************************************/
static node *
ATravRange (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ATravRange");

    INFO_AT_EXPRS_IDS (arg_info)
      = Exprs2IdsWhenFold (RANGE_RESULTS (arg_node), INFO_WITHOPS (arg_info),
                           INFO_AT_INIT_LUT (arg_info));

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn char *IdsIdsToshareds( node *ids, node *ids2, lut_t *lut)
 *
 * @brief For each ids ids2 pair create a shared with the same type.
 *        Save the new avis( ids) and avis( ids2) into the lut pointing to
 *        the new shared avis.
 *
 * @param ids  Chain of ids' that are to be assisted with shareds
 * @param ids2 Chain of ids' that are to be assisted with shareds
 *             also results of range.
 * @param lut  Look up table to store mappings of avis -> avis ( shared) in
 *
 * @return Chain of vardecs for new shareds.
 *****************************************************************************/
static node *
IdsIdsToShareds (node *ids, node *ids2, lut_t *lut, lut_t *init_lut)
{
    node *avis;
    node *vardec = NULL;
    ntype *type;
    node *fold;
    DBUG_ENTER ("IdsIdsToShareds");

    if (ids != NULL) {
        DBUG_ASSERT ((ids2 != NULL), "Expected two lists of the same length");

        DBUG_ASSERT ((TYeqTypes (AVIS_TYPE (IDS_AVIS (ids)),
                                 AVIS_TYPE (IDS_AVIS (ids2)))),
                     "Expected both types to be equal");

        vardec = IdsIdsToShareds (IDS_NEXT (ids), IDS_NEXT (ids2), lut, init_lut);

        type = TYcopyType (AVIS_TYPE (IDS_AVIS (ids)));
        type = TYsetMutcScope (type, MUTC_SHARED);
        avis = TBmakeAvis (TRAVtmpVar (), type);
        vardec = TBmakeVardec (avis, vardec);

        fold = LUTsearchInLutPp (init_lut, IDS_AVIS (ids2));
        DBUG_ASSERT ((fold != NULL), "Lost information about fold");

        AVIS_WITH3FOLD (avis) = fold;

        lut = LUTinsertIntoLutP (lut, IDS_AVIS (ids), avis);
        lut = LUTinsertIntoLutP (lut, IDS_AVIS (ids2), avis);
    } else {
        DBUG_ASSERT ((ids2 == NULL), "Expected two lists of the same length");
    }

    DBUG_RETURN (vardec);
}

/** <!-- ****************************************************************** -->
 * @fn node *ATravPrfAccu( node *arg_node, info *arg_info)
 *
 * @brief We have found an accu using this accus lhs and the fold results of
 *        this with loop create the shareds that we need.
 *****************************************************************************/
static node *
ATravPrfAccu (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ATravPrfAccu");

    INFO_VARDECS (arg_info)
      = TCappendVardec (INFO_VARDECS (arg_info),
                        IdsIdsToShareds (INFO_LHS (arg_info),
                                         INFO_AT_EXPRS_IDS (arg_info),
                                         INFO_AT_LUT (arg_info),
                                         INFO_AT_INIT_LUT (arg_info)));

    INFO_AT_EXPRS_IDS (arg_info) = FREEdoFreeTree (INFO_AT_EXPRS_IDS (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *ATravPrfSyncIn( node *arg_node, info *arg_info)
 *
 * @brief Add shared variable to syncIn as 2nd argument
 *****************************************************************************/
static node *
ATravPrfSyncIn (node *arg_node, info *arg_info)
{
    node *avis;
    DBUG_ENTER ("ATravPrfSyncIn");

    DBUG_ASSERT ((TCcountExprs (PRF_ARGS (arg_node)) == 1),
                 "Expected syncin to have one argument");

    avis = LUTsearchInLutPp (INFO_AT_LUT (arg_info), ID_AVIS (PRF_ARG1 (arg_node)));

    DBUG_ASSERT ((avis != NULL), "Could not create shared for syncIn");

    PRF_ARGS (arg_node)
      = TCappendExprs (PRF_ARGS (arg_node), TBmakeExprs (TBmakeId (avis), NULL));

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *ATravPrfSyncOut( node *arg_node, info *arg_info)
 *
 * @brief Add shared variable to syncOut as 2nd argument
 *****************************************************************************/
static node *
ATravPrfSyncOut (node *arg_node, info *arg_info)
{
    node *avis;
    DBUG_ENTER ("ATravPrfSyncOut");

    DBUG_ASSERT ((TCcountExprs (PRF_ARGS (arg_node)) == 1),
                 "Expected syncout to have one argument");

    avis = LUTsearchInLutPp (INFO_AT_LUT (arg_info), IDS_AVIS (INFO_LHS (arg_info)));

    DBUG_ASSERT ((avis != NULL), "Could not create a shared for syncout");

    PRF_ARGS (arg_node)
      = TCappendExprs (PRF_ARGS (arg_node), TBmakeExprs (TBmakeId (avis), NULL));
    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *ATravPrf( node *arg_node, info *arg_info)
 *
 * @brief Handle the three different prfs that are intresting when adding syncs
 *        accu
 *        syncIn
 *        syncOut
 *****************************************************************************/
static node *
ATravPrf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ATravPrf");
    switch (PRF_PRF (arg_node)) {
    case F_accu:
        ATravPrfAccu (arg_node, arg_info);
        break;
    case F_syncin:
        ATravPrfSyncIn (arg_node, arg_info);
        break;
    case F_syncout:
        ATravPrfSyncOut (arg_node, arg_info);
        break;
    default:
        arg_node = TRAVcont (arg_node, arg_info);
        break;
    }
    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *ATravLet( node *arg_node, info *arg_info)
 *
 * @brief Rember the lhs of the current let
 *****************************************************************************/
static node *
ATravLet (node *arg_node, info *arg_info)
{
    node *stack;
    DBUG_ENTER ("ATravLet");

    stack = INFO_LHS (arg_info);

    INFO_LHS (arg_info) = LET_IDS (arg_node);

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_LHS (arg_info) = stack;

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *addShareds( node *syntax_tree, info *arg_info)
 *
 * @brief Add shared varables to the syntax tree for sync ins and sync outs.
 *
 * @return syntax tree with sharded variables added
 *****************************************************************************/
static node *
addShareds (node *syntax_tree, info *arg_info)
{
    anontrav_t atrav[6]
      = {{N_prf, &ATravPrf}, {N_range, &ATravRange}, {N_fundef, &ATravFundef},
         {N_let, &ATravLet}, {N_with3, &ATravWith3}, {0, NULL}};
    info *anon_info;

    DBUG_ENTER ("addShareds");

    /* Need to pass fundef nodes to add vardecs */
    DBUG_ASSERT (((NODE_TYPE (syntax_tree) == N_module)
                  || (NODE_TYPE (syntax_tree) == N_fundef)),
                 "addShareds can only be run on module or fundef");

    TRAVpushAnonymous (atrav, &TRAVsons);

    anon_info = MakeInfo ();
    INFO_WITHOPS_IDS (anon_info) = INFO_WITHOPS_IDS (arg_info);
    syntax_tree = TRAVdo (syntax_tree, anon_info);
    INFO_WITHOPS_IDS (arg_info) = INFO_WITHOPS_IDS (anon_info);
    anon_info = FreeInfo (anon_info);

    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}

/** <!-- ****************************************************************** -->
 * @fn char *CreateThreadFunName(info *arg_info)
 *
 * @brief Creates a name for a new thread function
 *
 * @param arg_info info structure to access global counter of thread functions
 *
 * @return the newly created name
 *****************************************************************************/
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

static node *
InitFolds (node *exprs, bool first, lut_t *lut)
{
    DBUG_ENTER ("InitFolds");

    if (exprs != NULL) {
        node *fold;

        if (EXPRS_NEXT (exprs) != NULL) {
            EXPRS_NEXT (exprs) = InitFolds (EXPRS_NEXT (exprs), first, lut);
        }

        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (exprs)) == N_id), "Expected N_id");

        fold = AVIS_WITH3FOLD (ID_AVIS (EXPRS_EXPR (exprs)));

        if (fold != NULL) {
            if (first) {
                if (FOLD_INITIAL (fold) == NULL) {
                    ID_AVIS (EXPRS_EXPR (exprs)) = ID_AVIS (FOLD_NEUTRAL (fold));
                } else {
                    ID_AVIS (EXPRS_EXPR (exprs)) = ID_AVIS (FOLD_INITIAL (fold));
                }
            } else {
                ID_AVIS (EXPRS_EXPR (exprs)) = IDS_AVIS (LUTsearchInLutPp (lut, fold));
            }
        }
    }

    DBUG_RETURN (exprs);
}

/** <!-- ****************************************************************** -->
 * @fn node *CreateThreadFunction(node *block, node *index, info *arg_info)
 *
 * @brief Lifts the body argument into a thread function which is prepended
 *        to INFO_THREADFUNS to be added to module node. Change variables
 *        so that they have new avis as they are now in a different context.
 *
 * @param block      block to be lifted into the function ( consumed)
 * @param results    results of the range                 ( consumed)
 * @param index      avis of index variable of this range
 * @param fistRanges is this the first range in a with3 loop?
 * @param arg_info   info structure to store new function
 *
 * @return N_ap node to be used as new result of the range for use in the
 *              outer context.
 ******************************************************************************/
static node *
CreateThreadFunction (node *block, node *results, node *index, bool firstRange,
                      info *arg_info)
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
     * tag as an index. We can do so, as we are no longer in SSA
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
    FUNDEF_WASWITH3BODY (threadfun) = TRUE;
    /*
     * tag the index variable
     */
    innerindex = LUTsearchInLutPp (lut, index);
    if (innerindex != index) {
        AVIS_ISTHREADINDEX (innerindex) = TRUE;
    }

    lut = LUTremoveLut (lut);

    /* Create ap for OUTER context */
    ap = TBmakeAp (threadfun, InitFolds (DFMUdfm2ApArgs (arg_mask, NULL), firstRange,
                                         INFO_WITHOPS_IDS (arg_info)));

    ret_mask = DFMremoveMask (ret_mask);
    arg_mask = DFMremoveMask (arg_mask);
    local_mask = DFMremoveMask (local_mask);

    block = FREEdoFreeTree (block);
    results = FREEdoFreeTree (results);

    DBUG_RETURN (ap);
}

/** <!-- ****************************************************************** -->
 * @fn node *LW3doLiftWith3( node *syntax_tree)
 *
 * @brief Lifts out the bodies of with3s into functions.
 *
 * @param syntax_tree N_module node of current syntax tree
 *
 * @return module with all with3 bodies lifted into thread functions.
 *****************************************************************************/
node *
LW3doLiftWith3 (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("LW3doLiftWith3");

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) == N_module),
                 "LW3 Called with non module node");

    info = MakeInfo ();
    syntax_tree = addShareds (syntax_tree, info);

    /*
     * Infer dataflow masks
     */
    syntax_tree = INFDFMSdoInferDfms (syntax_tree, HIDE_LOCALS_WITH3);

    TRAVpush (TR_lw3);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    /*
     * Clean up tree
     *   Remove unneeded vardecs as some vardecs may nolonger be needed as they
     *     were in the lifted code.
     *   Remove dataflow mask
     */
    syntax_tree = CUDdoCleanupDecls (syntax_tree);
    syntax_tree = RDFMSdoRemoveDfms (syntax_tree);

    DBUG_RETURN (syntax_tree);
}

/** <!-- ****************************************************************** -->
 * @fn node *LW3module( node *arg_node, info *arg_info)
 *
 * @brief Find out the namespace of this module and save in info struct.
 *        After traver...
 *
 * @param arg_node module node
 * @param arg_info info structure
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

node *
LW3fundef (node *arg_node, info *arg_info)
{
    node *stack;
    DBUG_ENTER ("LW3fundef");

    stack = INFO_FUNDEF (arg_info);

    INFO_FUNDEF (arg_info) = arg_node;

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_FUNDEF (arg_info) = stack;

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
 *****************************************************************************/
node *
LW3range (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("LW3range");

    INFO_RANGES (arg_info)++;

    /*
     * we perform the transformation depth first / bottom up
     */
    RANGE_BODY (arg_node) = TRAVdo (RANGE_BODY (arg_node), arg_info);

    /* Re compute dfm to account for init of folds */
    INFO_FUNDEF (arg_info)
      = INFDFMSdoInferDfms (RDFMSdoRemoveDfms (INFO_FUNDEF (arg_info)),
                            HIDE_LOCALS_WITH3);

    RANGE_RESULTS (arg_node)
      = CreateThreadFunction (RANGE_BODY (arg_node), RANGE_RESULTS (arg_node),
                              ID_AVIS (RANGE_INDEX (arg_node)),
                              (INFO_RANGES (arg_info) == 1), arg_info);

    RANGE_BODY (arg_node) = TBmakeBlock (TBmakeEmpty (), NULL);

    RANGE_NEXT (arg_node) = TRAVopt (RANGE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *LW3range( node *arg_node, info *arg_info)
 *
 * @brief Stack the count of number of ranges.
 *****************************************************************************/
node *
LW3with3 (node *arg_node, info *arg_info)
{
    int ranges;
    DBUG_ENTER ("LW3with3");

    ranges = INFO_RANGES (arg_info);
    INFO_RANGES (arg_info) = 0;
    arg_node = TRAVcont (arg_node, arg_info);
    INFO_RANGES (arg_info) = ranges;

    DBUG_RETURN (arg_node);
}
