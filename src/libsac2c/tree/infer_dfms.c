/*****************************************************************************
 *
 * file:   infer_dfms.c
 *
 * prefix: INFDFMS
 *
 * description:
 *
 *   This compiler module implements the inference of the data flow masks
 *   and is used by LAC2Fun, Refcounting, SPMD, ...
 *   The data flow masks are bitmasks that are attached to
 *   - conditionals   (N_cond)
 *   - loops          (N-do)
 *   - and with-loops (N_with / N_with2 / N_with3)
 *   - blocks         (N-block)
 *   They signal those variables that are relatively free
 *   (IN-mask), local to the compound node (LOCAL-mask), or exported from
 *   the compound node (OUT-mask).
 *
 *   There are 2 possible entry calls to this traversal:
 *
 *   - INFDFMSdoInferDfms :
 *       - may be called on fundefs and modules only! If called on a fundef
 *         node, only that fundef and corresponding loop/cond funs will
 *         be processed.
 *       - It ensures the existance of a mask base at the fundef(s)
 *         AND it allocates 3 masks at each N_cond, N_do, N_with, N_with2
 *         and N_with3 node
 *
 *   - INFDFMSdoInferInDfmAssignChain:
 *       - may be called on any N_assign node!
 *       - CAUTION: does NOT do fix-point iteration! Hence, it refuses to
 *         traverse N_do nodes!!
 *       - In contrast to INFDFMSdoInferDfms, it DOES NOT allocate any masks
 *         at any node.
 *       - It returns (a copy of ) the final IN_MASK.
 *
 * usage of arg_info (INFO_...):
 *
 *   ...FUNDEF   pointer to the current fundef
 *
 *   ...NEEDED   DFmask: vars needed in outer blocks
 *   ...IN       DFmask: vars used before eventually defined
 *                       (in the current block)
 *   ...OUT      DFmask: vars defined in the current block and needed in
 *                       outer blocks
 *   ...LOCAL    DFmask: vars defined before eventually used
 *                       (in the current block)
 *   (Note: Each var occuring in a block is either IN-var or (exclusive!)
 *          LOCAL-var)
 *
 *   ...ISFIX          flag: fixpoint reached?
 *   ...FIRST          flag: first traversal?
 *   ...HIDELOC        bit field: steers hiding of local vars
 *   ...ATTACHATTRIBS  flag: attach dfms to cond/do/withX nodes
 *   ...ONEFUNDEF      flag: only infer for the current fundef
 *
 *****************************************************************************/

#include "infer_dfms.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "node_basic.h"
#include "traverse.h"
#include "free.h"

#define DBUG_PREFIX "INFDFMS"
#include "debug.h"

#include "DupTree.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"
#include "type_utils.h"
#include "globals.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    dfmask_t *in;
    dfmask_t *out;
    dfmask_t *local;
    dfmask_t *needed;
    bool isfix;
    bool first;
    int hideloc;
    bool attachattribs;
    bool onefundef;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_IN(n) ((n)->in)
#define INFO_OUT(n) ((n)->out)
#define INFO_LOCAL(n) ((n)->local)
#define INFO_NEEDED(n) ((n)->needed)
#define INFO_ISFIX(n) ((n)->isfix)
#define INFO_FIRST(n) ((n)->first)
#define INFO_HIDELOC(n) ((n)->hideloc)
#define INFO_ATTACHATTRIBS(n) ((n)->attachattribs)
#define INFO_ONEFUNDEF(n) ((n)->onefundef)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_IN (result) = NULL;
    INFO_OUT (result) = NULL;
    INFO_LOCAL (result) = NULL;
    INFO_NEEDED (result) = NULL;
    INFO_ISFIX (result) = FALSE;
    INFO_FIRST (result) = FALSE;
    INFO_HIDELOC (result) = 0;
    INFO_ATTACHATTRIBS (result) = FALSE;
    INFO_ONEFUNDEF (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/*
 * macros for testing HIDE_LOCALS for a given node
 */
#define TEST_BIT(bf, bit) ((bf & bit) != 0)

#define TEST_HIDE_LOCALS(bf, arg_node)                                                   \
    ((NODE_TYPE (arg_node) == N_do)                                                      \
       ? TEST_BIT (bf, HIDE_LOCALS_DO)                                                   \
       : ((NODE_TYPE (arg_node) == N_while)                                              \
            ? TEST_BIT (bf, HIDE_LOCALS_WHILE)                                           \
            : ((NODE_TYPE (arg_node) == N_cond)                                          \
                 ? TEST_BIT (bf, HIDE_LOCALS_COND)                                       \
                 : ((NODE_TYPE (arg_node) == N_with)                                     \
                      ? TEST_BIT (bf, HIDE_LOCALS_WITH)                                  \
                      : ((NODE_TYPE (arg_node) == N_with2)                               \
                           ? TEST_BIT (bf, HIDE_LOCALS_WITH2)                            \
                           : ((NODE_TYPE (arg_node) == N_with3)                          \
                                ? TEST_BIT (bf, HIDE_LOCALS_WITH3)                       \
                                : ((NODE_TYPE (arg_node) == N_block)                     \
                                     ? TEST_BIT (bf, HIDE_LOCALS_BLOCK)                  \
                                     : FALSE)))))))

/*
 * The current value of the DFmask 'old' is freed and subsequently the
 * value of 'new' is assigned to 'old'.
 */
#define UPDATE(old, new)                                                                 \
    if ((old) != NULL) {                                                                 \
        (old) = DFMremoveMask (old);                                                     \
    }                                                                                    \
    (old) = (new);

/*
 * Before UPDATE() is called, the two DFmasks 'old' and 'new' are compared.
 * If they differ the flag INFO_ISFIX is unset to indicate that
 * another iteration is needed (fixpoint not yet reached).
 */
#define COMPARE_AND_UPDATE(old, new, arg_info)                                           \
    if ((old) != NULL) {                                                                 \
        if (DFMtestMask (old) + DFMtestMask (new) != 2 * DFMtest2Masks (old, new)) {     \
            /* 'old' and 'new' differs */                                                \
            INFO_ISFIX (arg_info) = FALSE;                                               \
        }                                                                                \
    } else {                                                                             \
        INFO_ISFIX (arg_info) = FALSE;                                                   \
    }                                                                                    \
    UPDATE (old, new);

/*
 * compound macro
 */
#define INFO_DFM_BASE(arg_info) FUNDEF_DFM_BASE (INFO_FUNDEF (arg_info))

#ifndef DBUG_OFF
/******************************************************************************
 *
 * Function:
 *   void DbugPrintMask( dfmask_t* dfm)
 *
 * Description:
 *
 *
 ******************************************************************************/
static void
DbugPrintMask (char *dfm_str, dfmask_t *dfm)
{
    DBUG_ENTER ();

    fprintf (stderr, "%s<" F_PTR ">: ", dfm_str, (void *)dfm);
    if (dfm != NULL) {
        DFMprintMask (stderr, "%s ", dfm);
    } else {
        fprintf (stderr, "NULL");
    }
    fprintf (stderr, "\n");

    DBUG_RETURN ();
}
#endif

#ifndef DBUG_OFF
/******************************************************************************
 *
 * Function:
 *   void DbugPrintSignature( char *node_str,
 *                            dfmask_t* in, dfmask_t* out, dfmask_t* local)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
DbugPrintSignature (const char *node_str, dfmask_t *in, dfmask_t *out, dfmask_t *local)
{
    DBUG_ENTER ();

    fprintf (stderr, "\n------------------------------------------\n");
    fprintf (stderr, "Signature of %s:\n", node_str);

    DbugPrintMask ("   in   ", in);
    DbugPrintMask ("   out  ", out);
    DbugPrintMask ("   local", local);

    fprintf (stderr, "------------------------------------------\n\n");

    DBUG_RETURN ();
}
#endif

#ifndef DBUG_OFF
/******************************************************************************
 *
 * Function:
 *   void DbugPrintMasks( info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
DbugPrintMasks (info *arg_info)
{
    DBUG_ENTER ();

    fprintf (stderr, "  ->\n");

    DbugPrintMask ("   in    ", INFO_IN (arg_info));
    DbugPrintMask ("   out   ", INFO_OUT (arg_info));
    DbugPrintMask ("   local ", INFO_LOCAL (arg_info));
    DbugPrintMask ("   needed", INFO_NEEDED (arg_info));

    DBUG_RETURN ();
}
#endif

/******************************************************************************
 *
 * function:
 *   info *UsedVar( info *arg_info, node *decl)
 *
 * description:
 *   Updates the masks of 'arg_info' according to a non-defining occurence of
 *   a variable. The variable is specified as a pointer to the declaration.
 *
 ******************************************************************************/

static info *
UsedVar (info *arg_info, node *avis)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Updating mask for N_avis %s...", AVIS_NAME (avis));

    DBUG_ASSERT (avis != NULL,
                 "Variable declaration missing! "
                 "For the time being Lac2fun() can be used after type checking"
                 " only!");

    DBUG_ASSERT (N_avis == NODE_TYPE (avis), "avis expected");

    DFMsetMaskEntrySet (INFO_IN (arg_info), NULL, avis);
    DFMsetMaskEntryClear (INFO_LOCAL (arg_info), NULL, avis);

    DBUG_RETURN (arg_info);
}

#if 0

/******************************************************************************
 *
 * function:
 *   info *UsedId( info *arg_info, node *arg_id)
 *
 * description:
 *   Calls 'UsedVar()' for the given id-node.
 *
 ******************************************************************************/

static
info *UsedId( info *arg_info, node *arg_id)
{
  DBUG_ENTER ();

  DBUG_ASSERT (NODE_TYPE( arg_id) == N_id, "no N_id node found!");

  arg_info = UsedVar( arg_info, ID_AVIS( arg_id));

  DBUG_RETURN (arg_info);
}

#endif

/******************************************************************************
 *
 * function:
 *   info *UsedMask( info *arg_info, dfmask_t* mask)
 *
 * description:
 *   Calls 'UsedVar()' for each variable set in the given mask.
 *
 *  TODO:
 *
 ******************************************************************************/

static info *
UsedMask (info *arg_info, dfmask_t *mask)
{
    node *avis;

    DBUG_ENTER ();

    DBUG_PRINT ("Updating mask...");

    avis = DFMgetMaskEntryAvisSet (mask);
    while (avis != NULL) {
        arg_info = UsedVar (arg_info, avis);
        avis = DFMgetMaskEntryAvisSet (NULL);
    }

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * function:
 *   info *DefinedVar( info *arg_info, node *decl)
 *
 * description:
 *   Updates the masks of 'arg_info' according to a defining occurence of a
 *   variable. The variable is specified as a pointer to the declaration.
 *
 ******************************************************************************/

static info *
DefinedVar (info *arg_info, node *avis)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Updating mask for N_avis %s...", AVIS_NAME (avis));

    DBUG_ASSERT (avis != NULL,
                 "Variable declaration missing! "
                 "For the time being Lac2fun() can be used after type checking"
                 " only!");

    DBUG_ASSERT (N_avis == NODE_TYPE (avis), "avis expected!");

    if ((NODE_TYPE (AVIS_DECL (avis)) == N_arg) && (ARG_ISREFERENCE (AVIS_DECL (avis)))) {
        /*
         * reference parameter found  ->  handle as occurance on RHS
         * (reference parameters should *not* be marked as out-parameters,
         * but as reference-in-parameters only!!)
         */
        arg_info = UsedVar (arg_info, avis);
    } else {
        DFMsetMaskEntryClear (INFO_IN (arg_info), NULL, avis);
        if (DFMtestMaskEntry (INFO_NEEDED (arg_info), NULL, avis)) {
            DFMsetMaskEntrySet (INFO_OUT (arg_info), NULL, avis);
        }
        DFMsetMaskEntrySet (INFO_LOCAL (arg_info), NULL, avis);
    }

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * function:
 *   info *DefinedIds( info *arg_info, node *arg_ids)
 *
 * description:
 *   Calls 'DefinedVar()' for each ids of the given ids-chain.
 *
 ******************************************************************************/

static info *
DefinedIds (info *arg_info, node *arg_ids)
{
    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_ids) == N_ids, "no N_ids node found");

    arg_info = DefinedVar (arg_info, IDS_AVIS (arg_ids));

    DBUG_RETURN (arg_info);
}

#if 0

/******************************************************************************
 *
 * function:
 *   info *DefinedId( info *arg_info, node *arg_id)
 *
 * description:
 *   Calls 'DefinedVar()' for the given id-node.
 *
 ******************************************************************************/

static
info *DefinedId( info *arg_info, node *arg_id)
{
  DBUG_ENTER ();

  DBUG_ASSERT (NODE_TYPE( arg_id) == N_id, "no N_id node found!");

  arg_info = DefinedVar( arg_info, ID_AVIS( arg_id));

  DBUG_RETURN (arg_info);
}

#endif

/******************************************************************************
 *
 * function:
 *   info *DefinedMask( info *arg_info, dfmask_t* mask)
 *
 * description:
 *   Calls 'DefinedVar()' for each variable set in the given mask.
 *
 ******************************************************************************/

static info *
DefinedMask (info *arg_info, dfmask_t *mask)
{
    node *avis;

    DBUG_ENTER ();

    DBUG_PRINT ("Updating mask...");

    avis = DFMgetMaskEntryAvisSet (mask);
    while (avis != NULL) {
        arg_info = DefinedVar (arg_info, avis);
        avis = DFMgetMaskEntryAvisSet (NULL);
    }

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * function:
 *   dfmask_t* AdjustNeededMasks( dfmask_t* needed,
 *                                dfmask_t* in, dfmask_t* out)
 *
 * description:
 *   Updates the given needed-mask (contains all vars, that are needed in
 *   outer blocks) according to the in- and out-mask of the current block.
 *   This function is needed to calculate the new needed-mask before entering
 *   a new block.
 *
 ******************************************************************************/

static dfmask_t *
AdjustNeededMasks (dfmask_t *needed, dfmask_t *in, dfmask_t *out)
{
    DBUG_ENTER ();

    DFMsetMaskMinus (needed, out);
    DFMsetMaskOr (needed, in);

    DBUG_RETURN (needed);
}

/******************************************************************************
 *
 * Function:
 *   info *GenerateClearMasks( info *arg_info);
 *
 * Description:
 *   Generates fresh masks in 'arg_info' when starting this phase.
 *     in = empty
 *     out = empty
 *     local = empty
 *     needed = empty
 *
 ******************************************************************************/

static info *
GenerateClearMasks (info *arg_info)
{
    DBUG_ENTER ();

    INFO_IN (arg_info) = DFMgenMaskClear (INFO_DFM_BASE (arg_info));
    INFO_OUT (arg_info) = DFMgenMaskClear (INFO_DFM_BASE (arg_info));
    INFO_LOCAL (arg_info) = DFMgenMaskClear (INFO_DFM_BASE (arg_info));
    INFO_NEEDED (arg_info) = DFMgenMaskClear (INFO_DFM_BASE (arg_info));

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * Function:
 *   info *GenerateMasks( info *arg_info,
 *                        dfmask_t* in, dfmask_t* out, dfmask_t* needed)
 *
 * Description:
 *   Generates fresh masks in 'arg_info' for a newly entered block
 *   according to the given masks of the outer block:
 *     in' = empty
 *     out' = empty
 *     local' = empty
 *     needed' = (needed \ out) u in   .
 *
 ******************************************************************************/

static info *
GenerateMasks (info *arg_info, dfmask_t *in, dfmask_t *out, dfmask_t *needed)
{
    DBUG_ENTER ();

    INFO_IN (arg_info) = DFMgenMaskClear (INFO_DFM_BASE (arg_info));
    INFO_OUT (arg_info) = DFMgenMaskClear (INFO_DFM_BASE (arg_info));
    INFO_LOCAL (arg_info) = DFMgenMaskClear (INFO_DFM_BASE (arg_info));
    INFO_NEEDED (arg_info) = DFMgenMaskCopy (needed);

    INFO_NEEDED (arg_info) = AdjustNeededMasks (INFO_NEEDED (arg_info), in, out);

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * Function:
 *   info *RemoveMasks( info *arg_info);
 *
 * Description:
 *   Removes all masks in 'arg_info'.
 *
 ******************************************************************************/

static info *
RemoveMasks (info *arg_info)
{
    DBUG_ENTER ();

    INFO_IN (arg_info) = DFMremoveMask (INFO_IN (arg_info));
    INFO_OUT (arg_info) = DFMremoveMask (INFO_OUT (arg_info));
    INFO_LOCAL (arg_info) = DFMremoveMask (INFO_LOCAL (arg_info));
    INFO_NEEDED (arg_info) = DFMremoveMask (INFO_NEEDED (arg_info));

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * Function:
 *   node *EnsureDFMbase( node *fundef);
 *
 * Description:
 *   Makes sure a correct DFM_BASE is attached to fundef.
 *
 ******************************************************************************/

static node *
EnsureDFMbase (node *fundef)
{
    dfmask_base_t *old_dfm_base;

    DBUG_ENTER ();

    old_dfm_base = FUNDEF_DFM_BASE (fundef);
    if (old_dfm_base == NULL) {
        FUNDEF_DFM_BASE (fundef)
          = DFMgenMaskBase (FUNDEF_ARGS (fundef), FUNDEF_VARDECS (fundef));

        DBUG_PRINT ("no DFM base found -> created (" F_PTR ")",
                    (void *)FUNDEF_DFM_BASE (fundef));
    } else {
        FUNDEF_DFM_BASE (fundef) = DFMupdateMaskBase (old_dfm_base, FUNDEF_ARGS (fundef),
                                                      FUNDEF_VARDECS (fundef));

        DBUG_ASSERT (FUNDEF_DFM_BASE (fundef) == old_dfm_base,
                     "address of DFM base has changed during update!");

        DBUG_PRINT ("DFM base found -> updated (" F_PTR ")",
                    (void *)FUNDEF_DFM_BASE (fundef));
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 ******************************************************************************
 *
 * AdjustXXX functions
 *
 * These functions do the actual mask manipulations that are required for
 * computing the masks of all 4 possible compound operations:
 * - N_cond
 * - N_do
 * - N_with
 * - N_with2
 * None of these operations does FREE or ALLOCATE any masks!!!
 * They all inspect their arguments and destructively modify the masks
 * in arg_info!
 */

/******************************************************************************
 *
 * function:
 *   info *AdjustMasksWith_Pre( info *arg_info, node *arg_node)
 *
 * description:
 *   Adjusts the masks for a newly entered with-loop according to the old
 *   masks (the masks that have been infered during the previous iteration).
 *
 *   *** in-, out-, local- mask ***
 *   left as is
 *
 *   *** needed-mask ***
 *   Stricly speaking, the scope of all vars defined within a with-loop
 *   is restricted to the with-loop itself:
 *
 *     val = 1;
 *     with (...) {
 *       val = 2;
 *     }
 *     genarray( ...)
 *     ... val ...         <---  here, 'val' still contains the value 1
 *
 *   That means, we have to clear the needed-mask!
 *
 ******************************************************************************/

static info *
AdjustMasksWith_Pre (info *arg_info, node *arg_node)
{
    DBUG_ENTER ();

    DBUG_ASSERT (((NODE_TYPE (arg_node) == N_with) || (NODE_TYPE (arg_node) == N_with2)
                  || (NODE_TYPE (arg_node) == N_with3)),
                 "wrong node type found!");

    DFMsetMaskClear (INFO_NEEDED (arg_info));

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * function:
 *   info *AdjustMasksWith_Post( info *arg_info)
 *
 * description:
 *   Adjusts the masks after traversal of a with-loop:
 *
 *   All masks are left as is.
 *
 ******************************************************************************/

static info *
AdjustMasksWith_Post (info *arg_info)
{
    DBUG_ENTER ();

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * Function:
 *   info *AdjustMasksCond_Pre( info *arg_info, node *arg_node)
 *
 * Description:
 *   Adjusts the masks for a newly entered conditional according to the old
 *   masks (the masks that have been infered during the previous iteration):
 *
 *   All masks are left as is.
 *
 ******************************************************************************/
static info *
AdjustMasksCond_Pre (info *arg_info, node *arg_node)
{
    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_cond, "wrong node type found!");

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * Function:
 *   info *AdjustMasksCond_Post( info *arg_info,
 *                dfmask_t* in_then, dfmask_t* out_then, dfmask_t* local_then)
 *
 * Description:
 *   Adjusts the masks after traversal of a conditional:
 *
 *   We have:
 *     in    = in_then + in_else + (out_then \ out_else) + (out_else \ out_then)
 *     out   = out_then u out_else
 *     local = (local_then u local_else) \ in
 *
 *   Furthermore, arg_info contains the else masks, i.e.,
 *     in_else    == INFO_IN( arg_info)
 *     out_else   == INFO_OUT( arg_info)
 *     local_else == INFO_LOCAL( arg_info)
 *
 ******************************************************************************/
static info *
AdjustMasksCond_Post (info *arg_info, dfmask_t *in_then, dfmask_t *out_then,
                      dfmask_t *local_then)
{
    dfmask_t *in_else, *out_else, *local_else;
    dfmask_t *tmp1, *tmp2;

    DBUG_ENTER ();

    in_else = INFO_IN (arg_info);
    out_else = INFO_OUT (arg_info);
    local_else = INFO_LOCAL (arg_info);

    /**
     * calculate new in-, out-, local-masks:
     *
     *
     * change in_else:
     */
    DFMsetMaskOr (in_else, in_then);
    tmp1 = DFMgenMaskMinus (out_then, out_else);
    tmp2 = DFMgenMaskMinus (out_else, out_then);
    DFMsetMaskOr (in_else, tmp1);
    DFMsetMaskOr (in_else, tmp2);

    /**
     * change out_else:
     */
    DFMsetMaskOr (out_else, out_then);

    /**
     * change local_else:
     */
    DFMsetMaskOr (local_else, local_then);
    DFMsetMaskMinus (local_else, in_else);

    tmp1 = DFMremoveMask (tmp1);
    tmp2 = DFMremoveMask (tmp2);

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * Function:
 *   info *AdjustMasksDo_Pre( info *arg_info, node *arg_node)
 *
 * Description:
 *   Adjusts the masks for a newly entered do-loop according to the old
 *   masks (the masks that have been infered during the previous iteration):
 *
 *   The conditional containing the recursive call of the do-function must
 *   be taken into account:
 *     <assignments>
 *     if (...) {
 *       out = DoFun( in);
 *     }
 *     return( out);
 *
 ******************************************************************************/
static info *
AdjustMasksDo_Pre (info *arg_info, node *arg_node)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Adjust mask for do-loop...");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_do, "wrong node type found!");

    /*
     * out = DoFun( in);
     */
    arg_info = DefinedMask (arg_info, DO_OUT_MASK (arg_node));
    arg_info = UsedMask (arg_info, DO_IN_MASK (arg_node));

    /*
     * if (...) { out = DoFun( in); }
     *
     * Note, that we have a conditional here:
     *   in  = in_t u in_e u (out_t \ out_e) u (out_e \ out_t) = in_t u out_t
     *   out = out_t u out_e = out_t
     *   local = (local_t u local_e) \ in = local_t \ out_t
     */
    DFMsetMaskOr (INFO_IN (arg_info), DO_OUT_MASK (arg_node));
    DFMsetMaskMinus (INFO_LOCAL (arg_info), DO_OUT_MASK (arg_node));

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * Function:
 *   info *AdjustMasksDo_Post( info *arg_info)
 *
 * Description:
 *   Adjusts the masks after traversal of a do-loop:
 *
 *   All masks are left as is.
 *
 ******************************************************************************/
static info *
AdjustMasksDo_Post (info *arg_info)
{
    DBUG_ENTER ();

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * function:
 *   info *AdjustMasksBlock_Pre( info *arg_info, node *arg_node)
 *
 * description:
 *   Adjusts the masks before traversal of a block:
 *
 *   All masks are left as is.
 *
 ******************************************************************************/

static info *
AdjustMasksBlock_Pre (info *arg_info, node *arg_node)
{
    DBUG_ENTER ();

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * function:
 *   info *AdjustMasksBlock_Post( info *arg_info)
 *
 * description:
 *   Adjusts the masks after traversal of a block:
 *
 *   All masks are left as is.
 *
 *   Scoping is handled by the surrounding construct.
 *
 ******************************************************************************/

static info *
AdjustMasksBlock_Post (info *arg_info)
{
    DBUG_ENTER ();

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 ******************************************************************************
 *
 * InferMasksXXX functions
 *
 * These functions do handle all 6 possible compound operations:
 * - N_cond
 * - N_do
 * - N_with
 * - N_with2
 * - N_with3
 * - N_block
 * All these functions do compute 4 new (freshly allocated) masks
 * in arg_info which contain the IN, OUT, LOCAL, and NEEDED masks
 * for the compound node given.
 * They do NOT preserve the masks that are in arg_info upon calling them.
 * This needs to be taken care of by the calling function, i.e.
 * by InferMasks!
 *
 */

/******************************************************************************
 *
 * Function:
 *   node *InferMasksWith( node *arg_node, info *arg_info)
 *
 * Description:
 *   In order to infer the withid-ids as local-vars, withop and code must be
 *   traversed *before* the withid (contained in part)!
 *   create 4 fresh masks in arg_info that contain the results of traversing
 *   this N_with
 *
 ******************************************************************************/
static node *
InferMasksWith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * setup masks
     */
    arg_info = GenerateMasks (arg_info, INFO_IN (arg_info), INFO_OUT (arg_info),
                              INFO_NEEDED (arg_info));

    /*
     * adjust masks (part 1)
     */
    arg_info = AdjustMasksWith_Pre (arg_info, arg_node);

    /*
     * traverse sons
     */

    DBUG_EXECUTE (fprintf (stderr, ">>>  %s entered", NODE_TEXT (arg_node));
                  DbugPrintMasks (arg_info););

    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    DBUG_EXECUTE (fprintf (stderr, "<<<  %s finished\n", NODE_TEXT (arg_node)));

    /*
     * adjust masks (part 2)
     */
    arg_info = AdjustMasksWith_Post (arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *InferMasksWith2( node *arg_node, info *arg_info)
 *
 * Description:
 *   In order to infer the withid-ids as local-vars, the withop, segments and
 *   code must be traversed *before* the withid!
 *   create 4 fresh masks in arg_info that contain the results of traversing
 *   this N_with2
 *
 ******************************************************************************/
static node *
InferMasksWith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * setup masks
     */
    arg_info = GenerateMasks (arg_info, INFO_IN (arg_info), INFO_OUT (arg_info),
                              INFO_NEEDED (arg_info));

    /*
     * adjust masks (part 1)
     */
    arg_info = AdjustMasksWith_Pre (arg_info, arg_node);

    /*
     * traverse sons
     */

    DBUG_EXECUTE (fprintf (stderr, ">>>  %s entered", NODE_TEXT (arg_node));
                  DbugPrintMasks (arg_info));

    WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
    WITH2_SEGS (arg_node) = TRAVdo (WITH2_SEGS (arg_node), arg_info);

    if (WITH2_CODE (arg_node) != NULL) {
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
    }

    WITH2_WITHID (arg_node) = TRAVdo (WITH2_WITHID (arg_node), arg_info);

    DBUG_EXECUTE (fprintf (stderr, "<<<  %s finished\n", NODE_TEXT (arg_node)));

    /*
     * adjust masks (part 2)
     */
    arg_info = AdjustMasksWith_Post (arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *InferMasksWith3( node *arg_node, info *arg_info)
 *
 * Description:
 *   create 3 fresh masks in arg_info that contain the results of traversing
 *   this N_with3
 *
 ******************************************************************************/
static node *
InferMasksWith3 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * setup masks
     */
    arg_info = GenerateMasks (arg_info, INFO_IN (arg_info), INFO_OUT (arg_info),
                              INFO_NEEDED (arg_info));

    /*
     * adjust masks (part 1)
     */
    arg_info = AdjustMasksWith_Pre (arg_info, arg_node);

    /*
     * traverse sons
     */

    DBUG_EXECUTE (fprintf (stderr, ">>>  %s entered", NODE_TEXT (arg_node));
                  DbugPrintMasks (arg_info));

    WITH3_OPERATIONS (arg_node) = TRAVdo (WITH3_OPERATIONS (arg_node), arg_info);
    WITH3_RANGES (arg_node) = TRAVopt (WITH3_RANGES (arg_node), arg_info);

    DBUG_EXECUTE (fprintf (stderr, "<<<  %s finished\n", NODE_TEXT (arg_node)));

    /*
     * adjust masks (part 2)
     */
    arg_info = AdjustMasksWith_Post (arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *InferMasksCond( node *arg_node, info *arg_info)
 *
 * Description:
 *   create 4 fresh masks in arg_info that contain the results of traversing
 *   this N_cond
 *
 ******************************************************************************/

static node *
InferMasksCond (node *arg_node, info *arg_info)
{
    dfmask_t *old_in, *old_out, *old_needed;
    dfmask_t *in_then, *out_then, *local_then;

    DBUG_ENTER ();

    old_in = INFO_IN (arg_info);
    old_out = INFO_OUT (arg_info);
    old_needed = INFO_NEEDED (arg_info);

    /*
     * setup masks for then-block
     */
    arg_info = GenerateMasks (arg_info, old_in, old_out, old_needed);

    /*
     * adjust masks (part 1) for then-block
     */
    arg_info = AdjustMasksCond_Pre (arg_info, arg_node);

    /*
     * traverse then-block
     */

    DBUG_EXECUTE (fprintf (stderr, ">>>  then-block of %s entered", NODE_TEXT (arg_node));
                  DbugPrintMasks (arg_info));

    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);

    DBUG_EXECUTE (
      fprintf (stderr, "<<<  then-block of %s finished\n", NODE_TEXT (arg_node)));

    in_then = INFO_IN (arg_info);
    out_then = INFO_OUT (arg_info);
    local_then = INFO_LOCAL (arg_info);

    INFO_NEEDED (arg_info) = DFMremoveMask (INFO_NEEDED (arg_info));
    /*
     * setup masks for else-block
     */
    arg_info = GenerateMasks (arg_info, old_in, old_out, old_needed);

    /*
     * adjust masks (part 1) for else-block
     */
    arg_info = AdjustMasksCond_Pre (arg_info, arg_node);

    /*
     * traverse else-block
     */

    DBUG_EXECUTE (fprintf (stderr, ">>>  else-block of %s entered", NODE_TEXT (arg_node));
                  DbugPrintMasks (arg_info));

    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    DBUG_EXECUTE (
      fprintf (stderr, "<<<  else-block of %s finished\n", NODE_TEXT (arg_node)));

    /*
     * adjust masks (part 2)
     */
    arg_info = AdjustMasksCond_Post (arg_info, in_then, out_then, local_then);

    in_then = DFMremoveMask (in_then);
    out_then = DFMremoveMask (out_then);
    local_then = DFMremoveMask (local_then);

    /*
     * traverse condition
     */
    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *InferMasksDo( node *arg_node, info *arg_info)
 *
 * Description:
 *   BODY must be traversed after COND!
 *   create 4 fresh masks in arg_info that contain the results of traversing
 *   this N_do
 *
 ******************************************************************************/
static node *
InferMasksDo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * setup masks
     */
    arg_info = GenerateMasks (arg_info, INFO_IN (arg_info), INFO_OUT (arg_info),
                              INFO_NEEDED (arg_info));

    /*
     * adjust masks (part 1)
     */
    arg_info = AdjustMasksDo_Pre (arg_info, arg_node);

    /*
     * traverse sons
     */

    DBUG_EXECUTE (fprintf (stderr, ">>>  %s entered", NODE_TEXT (arg_node));
                  DbugPrintMasks (arg_info));

    DO_COND (arg_node) = TRAVdo (DO_COND (arg_node), arg_info);
    DO_BODY (arg_node) = TRAVdo (DO_BODY (arg_node), arg_info);

    DBUG_EXECUTE (fprintf (stderr, "<<<  %s finished\n", NODE_TEXT (arg_node)));

    /*
     * adjust masks (part 2)
     */
    arg_info = AdjustMasksDo_Post (arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *InferMasksBlock( node *arg_node, info *arg_info)
 *
 * Description:
 *   create 3 fresh masks in arg_info that contain the results of traversing
 *   this N_block
 *
 ******************************************************************************/
static node *
InferMasksBlock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * setup masks
     */
    arg_info = GenerateMasks (arg_info, INFO_IN (arg_info), INFO_OUT (arg_info),
                              INFO_NEEDED (arg_info));

    /*
     * adjust masks (part 1)
     */
    arg_info = AdjustMasksBlock_Pre (arg_info, arg_node);

    /*
     * traverse sons
     */

    DBUG_EXECUTE (fprintf (stderr, ">>>  %s entered", NODE_TEXT (arg_node));
                  DbugPrintMasks (arg_info));

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    DBUG_EXECUTE (fprintf (stderr, "<<<  %s finished\n", NODE_TEXT (arg_node)));

    /*
     * adjust masks (part 2)
     */
    arg_info = AdjustMasksBlock_Post (arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *InferMasks( dfmask_t* *in, dfmask_t* *out, dfmask_t* *local,
 *                     node *arg_node, info *arg_info,
 *                     node *(InferMasksFun)( node *, node *),
 *                     bool do_fixpoint_iter)
 *
 * description:
 *   Infers the in-, out-, local-vars of a conditional or loop.
 *
 ******************************************************************************/

static node *
InferMasks (dfmask_t **in, dfmask_t **out, dfmask_t **local, node *arg_node,
            info *arg_info, node *(InferMasksFun) (node *, info *), bool do_fixpoint_iter)
{
    dfmask_t *old_needed, *old_in, *old_out, *old_local;
    dfmask_t *new_in, *new_out, *new_local;

    DBUG_ENTER ();

    DBUG_PRINT ("Infer masks for cond/loop-fun...");

    if (INFO_ATTACHATTRIBS (arg_info) && INFO_FIRST (arg_info)) {
        /*
         * first traversal
         *  -> init the given in/out/local-masks!!!!
         */
        if ((*in) != NULL) {
            (*in) = DFMremoveMask ((*in));
        }
        (*in) = DFMgenMaskClear (INFO_DFM_BASE (arg_info));

        if ((*out) != NULL) {
            (*out) = DFMremoveMask ((*out));
        }
        (*out) = DFMgenMaskClear (INFO_DFM_BASE (arg_info));

        if ((*local) != NULL) {
            (*local) = DFMremoveMask ((*local));
        }
        (*local) = DFMgenMaskClear (INFO_DFM_BASE (arg_info));
    }

    /*
     * save old masks
     */
    old_needed = INFO_NEEDED (arg_info);
    old_in = INFO_IN (arg_info);
    old_out = INFO_OUT (arg_info);
    old_local = INFO_LOCAL (arg_info);

    /*
     * infer new masks. All these functions create 4 new masks
     * in arg_info!
     */
    arg_node = InferMasksFun (arg_node, arg_info);

    new_in = INFO_IN (arg_info);
    new_out = INFO_OUT (arg_info);
    new_local = INFO_LOCAL (arg_info);

    if (INFO_ATTACHATTRIBS (arg_info)) {
        /*
         * store the infered in-, out-, local-masks and
         * detect whether the fixpoint-property is hold or not
         */
        if (do_fixpoint_iter) {
            COMPARE_AND_UPDATE (*in, new_in, arg_info);
            COMPARE_AND_UPDATE (*out, new_out, arg_info);
            COMPARE_AND_UPDATE (*local, new_local, arg_info);
        } else {
            /*
             * no fixpoint iteration needed
             */
            UPDATE (*in, new_in);
            UPDATE (*out, new_out);
            UPDATE (*local, new_local);
        }
    }

#ifndef DBUG_OFF
    DBUG_EXECUTE (DbugPrintSignature (NODE_TEXT (arg_node), new_in, new_out, new_local));
#endif

    /*
     * update old local-mask
     */
    if (TEST_HIDE_LOCALS (INFO_HIDELOC (arg_info), arg_node)) {
        /*
         * we have to hide the local vars!!
         */
        DBUG_PRINT_TAG ("INFDFMS_ALL", "local vars of node %s are hidden!!!",
                        NODE_TEXT (arg_node));
    } else {
        DBUG_PRINT_TAG ("INFDFMS_ALL", "local vars of node %s are not hidden.",
                        NODE_TEXT (arg_node));
        DFMsetMaskOr (old_local, new_local);
    }

    /*
     * restore old needed-mask
     */
    INFO_NEEDED (arg_info) = DFMremoveMask (INFO_NEEDED (arg_info));
    INFO_NEEDED (arg_info) = old_needed;

    /*
     * restore old in-, out-, local-mask
     */
    INFO_IN (arg_info) = old_in;
    INFO_OUT (arg_info) = old_out;
    INFO_LOCAL (arg_info) = old_local;

    /*
     * The whole conditional/loop can be represented by a single function call
     *     ...out-vars... dummy-fun( ...in-vars... )
     * Therefore we must adjust the current masks accordingly.
     *
     * Note, that the local-mask has been updated already!!!!
     */
    arg_info = DefinedMask (arg_info, new_out);
    arg_info = UsedMask (arg_info, new_in);

    if (!INFO_ATTACHATTRIBS (arg_info)) {
        /**
         * the inferred masks are no longer needed as they have not been
         * attached to the arg_node. Hence, we have to remove them.
         */
        new_in = DFMremoveMask (new_in);
        new_out = DFMremoveMask (new_out);
        new_local = DFMremoveMask (new_local);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INFDFMSfundef( node *arg_node, info *arg_info)
 *
 * description:
 *   All DFM-masks needed during traversal of the body are build before
 *   and removed afterwards.
 *   The formal arguments are traversed to take reference parameters into
 *   account.
 *   The body is traversed until the signature of the contained conditions and
 *   loops remains unchanged (fixpoint iteration).
 *
 * remark:
 *   Fixpoint iteration is needed for nested loops:
 *
 *     a = 1;
 *     do {         <- loop1
 *       b = a;
 *       do {       <- loop2
 *         a = 2;
 *       }
 *     }
 *     ... b ...
 *
 *   To determine the signature of loop1 the signature of loop2 is needed
 *   and vise versa!!!
 *
 ******************************************************************************/

node *
INFDFMSfundef (node *arg_node, info *arg_info)
{
#ifndef DBUG_OFF
    int cnt = 0;
#endif

    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_EXECUTE (
          fprintf (stderr, ">>>>>>>  function %s():\n", FUNDEF_NAME (arg_node)));

        arg_node = EnsureDFMbase (arg_node);

        arg_info = GenerateClearMasks (arg_info);
        INFO_FIRST (arg_info) = TRUE;

        do {
            DBUG_EXECUTE (
              cnt++; fprintf (stderr, "\n>>>>>  fixpoint iteration --- loop %i\n", cnt));
            INFO_ISFIX (arg_info) = TRUE;

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

            DFMsetMaskClear (INFO_IN (arg_info));
            DFMsetMaskClear (INFO_OUT (arg_info));
            DFMsetMaskClear (INFO_LOCAL (arg_info));
            DFMsetMaskClear (INFO_NEEDED (arg_info));
            INFO_FIRST (arg_info) = FALSE;
        } while (!INFO_ISFIX (arg_info));

        DBUG_EXECUTE (fprintf (stderr, "<<<<<<<  %s(): finished after %i iterations\n",
                               FUNDEF_NAME (arg_node), cnt));

        arg_info = RemoveMasks (arg_info);
    }

    if (!INFO_ONEFUNDEF (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INFDFMSarg( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
INFDFMSarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INFDFMSassign( node *arg_node, info *arg_info)
 *
 * description:
 *   Bottom-up-traversal of the assignments.
 *
 ******************************************************************************/

node *
INFDFMSassign (node *arg_node, info *arg_info)
{
    node *assign_next;

    DBUG_ENTER ();

    if (ASSIGN_NEXT (arg_node) != NULL) {
        assign_next = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INFDFMSlet( node *arg_node, info *arg_info)
 *
 * description:
 *   Every left hand side variable is marked as 'defined' in the in-, out-,
 *   local-masks of the current block.
 *
 ******************************************************************************/

node *
INFDFMSlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_EXECUTE_TAG ("INFDFMS_ALL",
                      fprintf (stderr, "%s traversed (", NODE_TEXT (arg_node));
                      {
                          node *let_ids = LET_IDS (arg_node);

                          if (let_ids != NULL) {
                              while (let_ids != NULL) {
                                  fprintf (stderr, "%s", IDS_NAME (let_ids));
                                  let_ids = IDS_NEXT (let_ids);
                                  if (let_ids != NULL) {
                                      fprintf (stderr, " ,");
                                  }
                              }
                              fprintf (stderr, " = ");
                          }
                          fprintf (stderr, "[%s])", NODE_TEXT (LET_EXPR (arg_node)));
                      } DbugPrintMasks (arg_info));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INFDFMSap( node *arg_node, info *arg_info)
 *
 * description:
 *   Searches for reference parameters and marks them as 'defined vars'.
 *
 ******************************************************************************/

node *
INFDFMSap (node *arg_node, info *arg_info)
{
    node *fundef_args, *ap_args, *decl;

    DBUG_ENTER ();

    /*
     * search for reference parameters and mark them as 'defined vars'
     * (resolve them explicitly)
     */
    DBUG_ASSERT (AP_FUNDEF (arg_node) != NULL, "AP_FUNDEF not found!");

    /* traverse the formal (fundef_args) and current (ap_args) parameters */
    fundef_args = FUNDEF_ARGS (AP_FUNDEF (arg_node));
    ap_args = AP_ARGS (arg_node);
    while ((ap_args != NULL) && (fundef_args != NULL)) {
        if (NODE_TYPE (EXPRS_EXPR (ap_args)) == N_globobj) {
            /* CAJ
             * How should objects be handled, when lifting?
             */
        } else {
            if ((ARG_ISREFERENCE (fundef_args))) {
                node *avis;
                DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (ap_args)) == N_id,
                             "Reference parameter must be a N_id node!");

                decl = ID_DECL (EXPRS_EXPR (ap_args));
                avis = ID_AVIS (EXPRS_EXPR (ap_args));
                if ((NODE_TYPE (decl) == N_arg) && ((ARG_ISREFERENCE (decl)))) {
                    /*
                     * argument is used as reference parameter of the application,
                     * but its declaration is already a reference parameter, too
                     *   -> do *not* mask as defined variable
                     *      (it is no out-var but a reference-in-var!)
                     */
                    DBUG_PRINT ("N_ap in %s() with reference as ref. parameter:"
                                "  %s( .. %s .. )",
                                FUNDEF_NAME (INFO_FUNDEF (arg_info)),
                                FUNDEF_NAME (AP_FUNDEF (arg_node)),
                                ID_NAME (EXPRS_EXPR (ap_args)));
                } else {
                    /*
                     * argument (which declaration is not a reference parameter) is
                     * used as reference parameter of the application
                     *   -> mark as defined variable (must be a out-var as well)
                     */
                    DBUG_PRINT ("N_ap in %s() with non-reference as ref. parameter:"
                                "  %s( .. %s .. )",
                                FUNDEF_NAME (INFO_FUNDEF (arg_info)),
                                FUNDEF_NAME (AP_FUNDEF (arg_node)),
                                ID_NAME (EXPRS_EXPR (ap_args)));
                    arg_info = DefinedVar (arg_info, avis);
                }
            }
        }

        fundef_args = ARG_NEXT (fundef_args);

        ap_args = EXPRS_NEXT (ap_args);
    }

    /*
     * traverse the arguments -> mark them as 'used vars'
     */
    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INFDFMSid( node *arg_node, info *arg_info)
 *
 * description:
 *   Every right hand side variable is marked as 'used' in the in-, out-,
 *   local-masks of the current block.
 *
 ******************************************************************************/

node *
INFDFMSid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("inspecting N_id %s...", ID_NAME (arg_node));
    arg_info = UsedVar (arg_info, ID_AVIS (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INFDFMSids( node *arg_node, info *arg_info)
 *
 * description:
 *   Every right hand side variable is marked as 'used' in the in-, out-,
 *   local-masks of the current block.
 *
 ******************************************************************************/

node *
INFDFMSids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    arg_info = DefinedIds (arg_info, arg_node);

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INFDFMSwithx( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
INFDFMSwith2 (node *arg_node, info *arg_info)
{
    dfmask_t *out;

    DBUG_ENTER ();
    DBUG_PRINT ("inferring masks for N_with2...");

    arg_node = InferMasks (&WITH2_IN_MASK (arg_node), &WITH2_OUT_MASK (arg_node),
                           &WITH2_LOCAL_MASK (arg_node), arg_node, arg_info,
                           InferMasksWith2, FALSE);

    DBUG_PRINT ("done inferring masks...");

    out = WITH2_OUT_MASK (arg_node);
    DBUG_ASSERT (((out == NULL) || (DFMgetMaskEntryAvisSet (out) == NULL)),
                 "with2 loop with out-vars found!");

    DBUG_RETURN (arg_node);
}

node *
INFDFMSwith (node *arg_node, info *arg_info)
{
    dfmask_t *out;

    DBUG_ENTER ();
    DBUG_PRINT ("inferring masks for N_with...");

    arg_node = InferMasks (&WITH_IN_MASK (arg_node), &WITH_OUT_MASK (arg_node),
                           &WITH_LOCAL_MASK (arg_node), arg_node, arg_info,
                           InferMasksWith, FALSE);

    DBUG_PRINT ("done inferring masks...");

    out = WITH_OUT_MASK (arg_node);
    DBUG_ASSERT (((out == NULL) || (DFMgetMaskEntryAvisSet (out) == NULL)),
                 "with loop with out-vars found!");

    DBUG_RETURN (arg_node);
}

node *
INFDFMSwith3 (node *arg_node, info *arg_info)
{
    dfmask_t *out;

    DBUG_ENTER ();
    DBUG_PRINT ("inferring masks for N_with3...");

    arg_node = InferMasks (&(WITH3_IN_MASK (arg_node)), &(WITH3_OUT_MASK (arg_node)),
                           &(WITH3_LOCAL_MASK (arg_node)), arg_node, arg_info,
                           InferMasksWith3, FALSE);

    DBUG_PRINT ("done inferring masks...");

    out = WITH3_OUT_MASK (arg_node);
    DBUG_ASSERT (((out == NULL) || (DFMgetMaskEntryAvisSet (out) == NULL)),
                 "with3 loop with out-vars found!");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INFDFMScode( node *arg_node, info *arg_info)
 *
 * description:
 *   In order to get a correct bottom-up-traversal, the code-expr must be
 *   traversed *before* the code-block!
 *
 ******************************************************************************/

node *
INFDFMScode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);
    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);

    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *INFDFMSrange( node *arg_node, info *arg_info)
 *
 * @brief Ensures a proper bottom-up traversal of the results and body of the
 *        range.
 *
 * @param arg_node N_range node
 * @param arg_info info structure
 *
 * @return N_range node
 ******************************************************************************/
node *
INFDFMSrange (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    RANGE_RESULTS (arg_node) = TRAVopt (RANGE_RESULTS (arg_node), arg_info);
    RANGE_BODY (arg_node) = TRAVdo (RANGE_BODY (arg_node), arg_info);

    RANGE_INDEX (arg_node) = TRAVdo (RANGE_INDEX (arg_node), arg_info);

    RANGE_LOWERBOUND (arg_node) = TRAVdo (RANGE_LOWERBOUND (arg_node), arg_info);
    RANGE_UPPERBOUND (arg_node) = TRAVdo (RANGE_UPPERBOUND (arg_node), arg_info);
    RANGE_CHUNKSIZE (arg_node) = TRAVopt (RANGE_CHUNKSIZE (arg_node), arg_info);

    RANGE_NEXT (arg_node) = TRAVopt (RANGE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INFDFMScond( node *arg_node, info *arg_info)
 *
 * description:
 *   Inferes the in-, out-, local-parameters of the conditional.
 *
 ******************************************************************************/

node *
INFDFMScond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    arg_node = InferMasks (&(COND_IN_MASK (arg_node)), &(COND_OUT_MASK (arg_node)),
                           &(COND_LOCAL_MASK (arg_node)), arg_node, arg_info,
                           InferMasksCond, FALSE);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INFDFMSdo( node *arg_node, info *arg_info)
 *
 * description:
 *   Inferes the in-, out-, local-parameters of the do-loop.
 *
 ******************************************************************************/

node *
INFDFMSdo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (INFO_ATTACHATTRIBS (arg_info),
                 "trying to traverse N_do node while being called via"
                 " INFDFMSdoInferInDfmAssignChain");

    arg_node
      = InferMasks (&(DO_IN_MASK (arg_node)), &(DO_OUT_MASK (arg_node)),
                    &(DO_LOCAL_MASK (arg_node)), arg_node, arg_info, InferMasksDo, TRUE);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *INFDFMSblock( node *arg_node, info *arg_info)
 *
 * @brief Infers the in, out and local parameters of a block.
 *
 * @param arg_node N_block node
 * @param arg_info info structure
 *
 * @return N_block node with updated masks
 ******************************************************************************/
node *
INFDFMSblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    arg_node = InferMasks (&(BLOCK_IN_MASK (arg_node)), &(BLOCK_OUT_MASK (arg_node)),
                           &(BLOCK_LOCAL_MASK (arg_node)), arg_node, arg_info,
                           InferMasksBlock, FALSE);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *InferDFMs( node *syntax_tree, int hide_locals)
 *
 * description:
 *   Infers the in-, out- and local-masks of each conditional or loop
 *   (via fixpoint iteration).
 *
 *   'hide_locals' is a bit field. It defines which node types are hiding
 *   their local variables during inference:
 *
 *     while (...) {
 *       ...
 *       while (...) {    <--- 'a' is local in respect to the inner while-loop
 *         a = 1;
 *         ... a ...
 *       }
 *       ...
 *     }
 *
 *   The question is: Should 'a' be a local variable for the outer loop, too?
 *   In most cases the answer would be 'yes', but e.g. during the inference for
 *   lac2fun this is not the correct behaviour, because after lifting the inner
 *   while-loop their local vars are invisible for the outer loop.
 *
 ******************************************************************************/

node *
INFDFMSdoInferDfms (node *syntax_tree, int hide_locals)
{
    info *info_node;

    DBUG_ENTER ();

    DBUG_ASSERT (((NODE_TYPE (syntax_tree) == N_module)
                  || (NODE_TYPE (syntax_tree) == N_fundef)),
                 "argument of InferDFMs() must be a N_modul or a N_fundef node!");

    info_node = MakeInfo ();
    INFO_HIDELOC (info_node) = hide_locals;
    INFO_ONEFUNDEF (info_node) = (NODE_TYPE (syntax_tree) == N_fundef);

    /**
     * indicate that we do want to attach attribs to all N_cond, N_do,
     * N_with and N_with2 nodes.
     */
    INFO_ATTACHATTRIBS (info_node) = TRUE;

    TRAVpush (TR_infdfms);
    syntax_tree = TRAVdo (syntax_tree, info_node);
    TRAVpop ();

    info_node = FreeInfo (info_node);

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * function:
 *   dfmask_t* InferInDFMAssignChain
 *
 * description:
 *   Infers the inmask of a given assignment chain which is located in
 *   a given function
 *
 *****************************************************************************/

dfmask_t *
INFDFMSdoInferInDfmAssignChain (node *assign, node *fundef)
{
    info *info;
    dfmask_t *res;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (assign) == N_assign,
                 "argument of InferInDFMAssignChain() must be a N_assign node!");
    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef,
                 "second argument of InferInDFMAssignChain() must be a N_fundef");

    fundef = EnsureDFMbase (fundef);

    info = MakeInfo ();
    INFO_HIDELOC (info) = HIDE_LOCALS_NEVER;
    INFO_FUNDEF (info) = fundef;
    INFO_FIRST (info) = TRUE;
    info = GenerateClearMasks (info);

    /**
     * indicate that we do NOT want to attach attributes:
     */
    INFO_ATTACHATTRIBS (info) = FALSE;

    TRAVpush (TR_infdfms);
    assign = TRAVdo (assign, info);
    TRAVpop ();

    res = DFMgenMaskCopy (INFO_IN (info));
    DFMsetMaskMinus (res, INFO_LOCAL (info));

    info = RemoveMasks (info);
    info = FreeInfo (info);

    DBUG_EXECUTE_TAG ("INFDFMS_AC", DFMprintMask (0, " %s ", res));

    DBUG_RETURN (res);
}

#undef DBUG_PREFIX
