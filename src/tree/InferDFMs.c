/*
 *
 * $Log$
 * Revision 1.30  2004/11/17 09:01:31  ktr
 * added InferInDFMAssignChain
 *
 * Revision 1.29  2004/08/01 15:43:46  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 1.28  2004/03/09 23:57:59  dkrHH
 * old backend removed
 *
 * Revision 1.27  2002/10/16 12:11:50  dkr
 * code for RESOLVE_REFERENCES removed:
 * reference parameters are considered in-parameters only (not
 * out-params!)
 *
 * Revision 1.26  2002/10/16 11:57:00  dkr
 * cpp flag RESOLVE_REFERENCES added
 *
 * Revision 1.25  2002/09/06 10:16:48  dkr
 * fixed a bug in handling of IDXS2OFFSET icm
 *
 * Revision 1.24  2002/09/06 09:37:30  dkr
 * ND_IDXS2OFFSET added
 *
 * Revision 1.23  2002/07/24 15:06:36  dkr
 * INFDFMSicm() simplified
 *
 * Revision 1.22  2002/07/12 17:00:46  dkr
 * INFDFMSicm(): modifications for new backend done
 *
 * Revision 1.20  2002/04/16 18:27:15  dkr
 * INFO_DFMBASE renamed into INFO_DFM_BASE
 *
 * Revision 1.19  2002/02/22 13:23:43  dkr
 * INFDFMSwithx() modified: & is used correctly now
 *
 * Revision 1.18  2002/02/22 12:32:40  sbs
 * some comments added
 *
 * Revision 1.17  2002/02/21 12:56:14  dkr
 * comment corrected
 *
 * Revision 1.16  2001/05/08 20:12:59  dkr
 * - fixed a bug in inference strategy for while-loops
 * - code re-organized and brushed
 *
 * Revision 1.15  2001/05/08 15:51:15  dkr
 * more debug output added
 *
 * Revision 1.14  2001/04/23 15:10:15  dkr
 * InferDFMs: DBUG_ASSERT added
 *
 * Revision 1.13  2001/04/23 13:38:39  dkr
 * minor changes in DbugPrintSignature() done
 *
 * Revision 1.12  2001/04/20 12:57:52  dkr
 * AdjustNeededWith() added:
 * All non-unique objects are removed from the needed-mask.
 *
 * Revision 1.10  2001/04/19 15:37:36  dkr
 * fixpoint iteration works correctly now
 * (bug in COMPARE_AND_UPDATE fixed)
 *
 * Revision 1.9  2001/04/19 10:09:26  dkr
 * INFDFMSwith(), INFDFMSwith2() replaced by INFDFMSwithx()
 *
 * Revision 1.8  2001/04/19 07:40:33  dkr
 * macro F_PTR used as format string for pointers
 *
 * Revision 1.7  2001/04/04 19:40:16  dkr
 * warning about out-vars in with-loop replaced by a DBUG_PRINT
 *
 * Revision 1.5  2001/03/22 20:01:20  dkr
 * include of tree.h eliminated
 *
 * Revision 1.4  2001/02/13 16:12:15  dkr
 * 'act_tab' is stacked now :-)
 *
 * Revision 1.3  2001/02/12 21:22:31  dkr
 * INFO_INFDFMS_FIRST added.
 * bug fixed: all DFMs in the AST are initialized correctly now
 *
 * Revision 1.2  2001/02/12 18:30:08  dkr
 * fixed a bug in INFDFMSfundef:
 * Now, DFMUpdateMaskBase() is called in order to get definitely correct
 * masks ...
 *
 * Revision 1.1  2000/12/15 18:29:31  dkr
 * Initial revision
 *
 * Revision 1.4  2000/12/15 11:05:02  dkr
 * mechanism for hiding of local variables added
 *
 * Revision 1.3  2000/12/12 11:37:39  dkr
 * INFDFMSicm added
 *
 * Revision 1.2  2000/12/08 10:28:00  dkr
 * function InferDFMs added
 *
 * Revision 1.1  2000/12/06 19:57:52  dkr
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   InferDFMs.c
 *
 * prefix: INFDFMS
 *
 * description:
 *
 *   This compiler module implements the inference of the data flow masks
 *   and is used by LAC2Fun, Refcounting, SPMD, ...
 *   The data flow masks are bitmasks that are attached to conditionals
 *   and loop nodes. They signal those variables that are relatively free
 *   (IN-mask), local to the compound node (LOCAL-mask), or exported from
 *   the compound node (OUT-mask).
 *
 * usage of arg_info (INFO_INFDFMS_...):
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
 *   ...ISFIX    flag: fixpoint reached?
 *   ...FIRST    flag: first traversal?
 *   ...HIDELOC  bit field: steers hiding of local vars
 *
 *****************************************************************************/

#define NEW_INFO

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "traverse.h"
#include "free.h"
#include "dbug.h"
#include "DupTree.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"
#include "InferDFMs.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    DFMmask_t in;
    DFMmask_t out;
    DFMmask_t local;
    DFMmask_t needed;
    bool isfix;
    bool first;
    int hideloc;
};

/*
 * INFO macros
 */
#define INFO_INFDFMS_FUNDEF(n) (n->fundef)
#define INFO_INFDFMS_IN(n) (n->in)
#define INFO_INFDFMS_OUT(n) (n->out)
#define INFO_INFDFMS_LOCAL(n) (n->local)
#define INFO_INFDFMS_NEEDED(n) (n->needed)
#define INFO_INFDFMS_ISFIX(n) (n->isfix)
#define INFO_INFDFMS_FIRST(n) (n->first)
#define INFO_INFDFMS_HIDELOC(n) (n->hideloc)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_INFDFMS_FUNDEF (result) = NULL;
    INFO_INFDFMS_IN (result) = NULL;
    INFO_INFDFMS_OUT (result) = NULL;
    INFO_INFDFMS_LOCAL (result) = NULL;
    INFO_INFDFMS_NEEDED (result) = NULL;
    INFO_INFDFMS_ISFIX (result) = FALSE;
    INFO_INFDFMS_FIRST (result) = FALSE;
    INFO_INFDFMS_HIDELOC (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

/*
 * The current value of the DFmask 'old' is freed and subsequently the
 * value of 'new' is assigned to 'old'.
 */
#define UPDATE(old, new)                                                                 \
    if ((old) != NULL) {                                                                 \
        (old) = DFMRemoveMask (old);                                                     \
    }                                                                                    \
    (old) = (new);

/*
 * Before UPDATE() is called, the two DFmasks 'old' and 'new' are compared.
 * If they differ the flag INFO_INFDFMS_ISFIX is unset to indicate that
 * another iteration is needed (fixpoint not yet reached).
 */
#define COMPARE_AND_UPDATE(old, new, arg_info)                                           \
    if ((old) != NULL) {                                                                 \
        if (DFMTestMask (old) + DFMTestMask (new) != 2 * DFMTest2Masks (old, new)) {     \
            /* 'old' and 'new' differs */                                                \
            INFO_INFDFMS_ISFIX (arg_info) = FALSE;                                       \
        }                                                                                \
    } else {                                                                             \
        INFO_INFDFMS_ISFIX (arg_info) = FALSE;                                           \
    }                                                                                    \
    UPDATE (old, new);

/*
 * compound macro
 */
#define INFO_DFM_BASE(arg_info) FUNDEF_DFM_BASE (INFO_INFDFMS_FUNDEF (arg_info))

#ifndef DBUG_OFF
/******************************************************************************
 *
 * Function:
 *   void DbugPrintMask( DFMmask_t dfm)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
DbugPrintMask (char *dfm_str, DFMmask_t dfm)
{
    DBUG_ENTER ("DbugPrintMask");

    fprintf (stderr, "%s<" F_PTR ">: ", dfm_str, dfm);
    if (dfm != NULL) {
        DFMPrintMask (stderr, "%s ", dfm);
    } else {
        fprintf (stderr, "NULL");
    }
    fprintf (stderr, "\n");

    DBUG_VOID_RETURN;
}
#endif

#ifndef DBUG_OFF
/******************************************************************************
 *
 * Function:
 *   void DbugPrintSignature( char *node_str,
 *                            DFMmask_t in, DFMmask_t out, DFMmask_t local)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
DbugPrintSignature (char *node_str, DFMmask_t in, DFMmask_t out, DFMmask_t local)
{
    DBUG_ENTER ("DbugPrintSignature");

    fprintf (stderr, "\n------------------------------------------\n");
    fprintf (stderr, "Signature of %s:\n", node_str);

    DbugPrintMask ("   in   ", in);
    DbugPrintMask ("   out  ", out);
    DbugPrintMask ("   local", local);

    fprintf (stderr, "------------------------------------------\n\n");

    DBUG_VOID_RETURN;
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
    DBUG_ENTER ("DbugPrintMasks");

    fprintf (stderr, "  ->\n");

    DbugPrintMask ("   in    ", INFO_INFDFMS_IN (arg_info));
    DbugPrintMask ("   out   ", INFO_INFDFMS_OUT (arg_info));
    DbugPrintMask ("   local ", INFO_INFDFMS_LOCAL (arg_info));
    DbugPrintMask ("   needed", INFO_INFDFMS_NEEDED (arg_info));

    DBUG_VOID_RETURN;
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
UsedVar (info *arg_info, node *decl)
{
    DBUG_ENTER ("UsedVar");

    DBUG_ASSERT ((decl != NULL),
                 "Variable declaration missing! "
                 "For the time being Lac2fun() can be used after type checking"
                 " only!");

    if ((NODE_TYPE (decl) != N_vardec) && (NODE_TYPE (decl) != N_arg)) {
        DBUG_ASSERT ((NODE_TYPE (decl) == N_objdef),
                     "declaration is neither a N_arg/N_vardec node nor a N_objdef"
                     " node");
    } else {
        DFMSetMaskEntrySet (INFO_INFDFMS_IN (arg_info), NULL, decl);
        DFMSetMaskEntryClear (INFO_INFDFMS_LOCAL (arg_info), NULL, decl);
    }

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * function:
 *   info *UsedId( info *arg_info, node *arg_id)
 *
 * description:
 *   Calls 'UsedVar()' for the given id-node.
 *
 ******************************************************************************/

static info *
UsedId (info *arg_info, node *arg_id)
{
    DBUG_ENTER ("UsedId");

    DBUG_ASSERT ((NODE_TYPE (arg_id) == N_id), "no N_id node found!");

    arg_info = UsedVar (arg_info, ID_VARDEC (arg_id));

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * function:
 *   info *UsedMask( info *arg_info, DFMmask_t mask)
 *
 * description:
 *   Calls 'UsedVar()' for each variable set in the given mask.
 *
 ******************************************************************************/

static info *
UsedMask (info *arg_info, DFMmask_t mask)
{
    node *decl;

    DBUG_ENTER ("UsedMask");

    decl = DFMGetMaskEntryDeclSet (mask);
    while (decl != NULL) {
        arg_info = UsedVar (arg_info, decl);
        decl = DFMGetMaskEntryDeclSet (NULL);
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
DefinedVar (info *arg_info, node *decl)
{
    DBUG_ENTER ("DefinedVar");

    DBUG_ASSERT ((decl != NULL),
                 "Variable declaration missing! "
                 "For the time being Lac2fun() can be used after type checking"
                 " only!");

    if ((NODE_TYPE (decl) != N_vardec) && (NODE_TYPE (decl) != N_arg)) {
        DBUG_ASSERT ((NODE_TYPE (decl) == N_objdef),
                     "Declaration is neither a N_arg/N_vardec node nor a N_objdef"
                     " node");
    } else {
        if ((NODE_TYPE (decl) == N_arg)
            && ((ARG_ATTRIB (decl) == ST_reference)
                || (ARG_ATTRIB (decl) == ST_readonly_reference))) {
            /*
             * reference parameter found  ->  handle as occurance on RHS
             * (reference parameters should *not* be marked as out-parameters,
             * but as reference-in-parameters only!!)
             */
            arg_info = UsedVar (arg_info, decl);
        } else {
            DFMSetMaskEntryClear (INFO_INFDFMS_IN (arg_info), NULL, decl);
            if (DFMTestMaskEntry (INFO_INFDFMS_NEEDED (arg_info), NULL, decl)) {
                DFMSetMaskEntrySet (INFO_INFDFMS_OUT (arg_info), NULL, decl);
            }
            DFMSetMaskEntrySet (INFO_INFDFMS_LOCAL (arg_info), NULL, decl);
        }
    }

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * function:
 *   info *DefinedIds( info *arg_info, ids *arg_ids)
 *
 * description:
 *   Calls 'DefinedVar()' for each ids of the given ids-chain.
 *
 ******************************************************************************/

static info *
DefinedIds (info *arg_info, ids *arg_ids)
{
    DBUG_ENTER ("DefinedIds");

    while (arg_ids != NULL) {
        arg_info = DefinedVar (arg_info, IDS_VARDEC (arg_ids));
        arg_ids = IDS_NEXT (arg_ids);
    }

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * function:
 *   info *DefinedId( info *arg_info, node *arg_id)
 *
 * description:
 *   Calls 'DefinedVar()' for the given id-node.
 *
 ******************************************************************************/

static info *
DefinedId (info *arg_info, node *arg_id)
{
    DBUG_ENTER ("DefinedId");

    DBUG_ASSERT ((NODE_TYPE (arg_id) == N_id), "no N_id node found!");

    arg_info = DefinedVar (arg_info, ID_VARDEC (arg_id));

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * function:
 *   info *DefinedMask( info *arg_info, DFMmask_t mask)
 *
 * description:
 *   Calls 'DefinedVar()' for each variable set in the given mask.
 *
 ******************************************************************************/

static info *
DefinedMask (info *arg_info, DFMmask_t mask)
{
    node *decl;

    DBUG_ENTER ("DefinedMask");

    decl = DFMGetMaskEntryDeclSet (mask);
    while (decl != NULL) {
        arg_info = DefinedVar (arg_info, decl);
        decl = DFMGetMaskEntryDeclSet (NULL);
    }

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * function:
 *   DFMmask_t AdjustNeededMasks( DFMmask_t needed,
 *                                DFMmask_t in, DFMmask_t out)
 *
 * description:
 *   Updates the given needed-mask (contains all vars, that are needed in
 *   outer blocks) according to the in- and out-mask of the current block.
 *   This function is needed to calculate the new needed-mask before entering
 *   a new block.
 *
 ******************************************************************************/

static DFMmask_t
AdjustNeededMasks (DFMmask_t needed, DFMmask_t in, DFMmask_t out)
{
    DBUG_ENTER ("AdjustNeededMasks");

    DFMSetMaskMinus (needed, out);
    DFMSetMaskOr (needed, in);

    DBUG_RETURN (needed);
}

/******************************************************************************
 *
 * Function:
 *   info *GenerateMasks( info *arg_info,
 *                        DFMmask_t in, DFMmask_t out, DFMmask_t needed)
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
GenerateMasks (info *arg_info, DFMmask_t in, DFMmask_t out, DFMmask_t needed)
{
    DBUG_ENTER ("GenerateMasks");

    INFO_INFDFMS_IN (arg_info) = DFMGenMaskClear (INFO_DFM_BASE (arg_info));
    INFO_INFDFMS_OUT (arg_info) = DFMGenMaskClear (INFO_DFM_BASE (arg_info));
    INFO_INFDFMS_LOCAL (arg_info) = DFMGenMaskClear (INFO_DFM_BASE (arg_info));
    INFO_INFDFMS_NEEDED (arg_info) = DFMGenMaskCopy (needed);

    INFO_INFDFMS_NEEDED (arg_info)
      = AdjustNeededMasks (INFO_INFDFMS_NEEDED (arg_info), in, out);

    DBUG_RETURN (arg_info);
}

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
 *   BUT, in case of global objects this behaviour is NOT wanted.
 *   Therefore, during the flattening phase all non-global vars defined
 *   within a with-loop are renamed:
 *
 *     val = 1;
 *     with (...) {
 *       _val = val;
 *       _val = 2;
 *     }
 *     genarray( ...)
 *     ... val ...         <---  here, 'val' still contains the value 1
 *
 *   Because of that, we can leave all global objects in the needed-mask
 *   in order to detect OUT-global-vars :-)
 *
 ******************************************************************************/

static info *
AdjustMasksWith_Pre (info *arg_info, node *arg_node)
{
    node *decl;

    DBUG_ENTER ("AdjustMasksWith_Pre");

    DBUG_ASSERT (((NODE_TYPE (arg_node) == N_Nwith)
                  || (NODE_TYPE (arg_node) == N_Nwith2)),
                 "wrong node type found!");

    decl = DFMGetMaskEntryDeclSet (INFO_INFDFMS_NEEDED (arg_info));
    while (decl != NULL) {
        /*
         * no unique object -> clear entry in mask
         */
        if (!IsUnique (VARDEC_OR_ARG_TYPE (decl))) {
            DFMSetMaskEntryClear (INFO_INFDFMS_NEEDED (arg_info), NULL, decl);
        }

        decl = DFMGetMaskEntryDeclSet (NULL);
    }

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
    DBUG_ENTER ("AdjustMasksWith_Post");

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

info *
AdjustMasksCond_Pre (info *arg_info, node *arg_node)
{
    DBUG_ENTER ("AdjustMasksCond_Pre");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_cond), "wrong node type found!");

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * Function:
 *   info *AdjustMasksCond_Post( info *arg_info,
 *                DFMmask_t in_then, DFMmask_t out_then, DFMmask_t local_then,
 *                DFMmask_t in_else, DFMmask_t out_else, DFMmask_t local_else)
 *
 * Description:
 *   Adjusts the masks after traversal of a conditional:
 *
 *   All masks are left as is.
 *
 ******************************************************************************/

info *
AdjustMasksCond_Post (info *arg_info, DFMmask_t in_then, DFMmask_t out_then,
                      DFMmask_t local_then, DFMmask_t in_else, DFMmask_t out_else,
                      DFMmask_t local_else)
{
    DFMmask_t tmp;

    DBUG_ENTER ("AdjustMasksCond_Post");

    /*
     * calculate new in-, out-, local-masks
     */
    /* in = in_then u in_else u (out_then \ out_else) u (out_else \ out_then) */
    INFO_INFDFMS_IN (arg_info) = DFMGenMaskMinus (out_then, out_else);
    tmp = DFMGenMaskMinus (out_else, out_then);
    DFMSetMaskOr (INFO_INFDFMS_IN (arg_info), tmp);
    DFMSetMaskOr (INFO_INFDFMS_IN (arg_info), in_then);
    DFMSetMaskOr (INFO_INFDFMS_IN (arg_info), in_else);
    /* out = out_then u out_else */
    INFO_INFDFMS_OUT (arg_info) = DFMGenMaskOr (out_then, out_else);
    /* local = (local_then u local_else) \ in */
    INFO_INFDFMS_LOCAL (arg_info) = DFMGenMaskOr (local_then, local_else);
    DFMSetMaskMinus (INFO_INFDFMS_LOCAL (arg_info), INFO_INFDFMS_IN (arg_info));

    tmp = DFMRemoveMask (tmp);
    in_then = DFMRemoveMask (in_then);
    out_then = DFMRemoveMask (out_then);
    local_then = DFMRemoveMask (local_then);
    in_else = DFMRemoveMask (in_else);
    out_else = DFMRemoveMask (out_else);
    local_else = DFMRemoveMask (local_else);

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * Function:
 *   info *AdjustMasksWhile_Pre( info *arg_info, node *arg_node)
 *
 * Description:
 *   Adjusts the masks for a newly entered while-loop according to the old
 *   masks (the masks that have been infered during the previous iteration):
 *
 *   The recursive call of the while-function must be taken into account:
 *     if (...) {
 *       <assignments>
 *       out = WhileFun( in);
 *     }
 *     return( out);
 *
 *   Note, that the conditional is handle *after*  <assignments>  has been
 *   traversed (see AdjustMaskWhile_Post()).
 *
 ******************************************************************************/

info *
AdjustMasksWhile_Pre (info *arg_info, node *arg_node)
{
    DBUG_ENTER ("AdjustMasksWhile_Pre");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_while), "wrong node type found!");

    /*
     * out = WhileFun( in);
     */
    arg_info = DefinedMask (arg_info, WHILE_OUT_MASK (arg_node));
    arg_info = UsedMask (arg_info, WHILE_IN_MASK (arg_node));

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * Function:
 *   info *AdjustMasksWhile_Post( info *arg_info)
 *
 * Description:
 *   Adjusts the masks after traversal of a while-loop:
 *
 *   The conditional of the while-function must be taken into account:
 *     if (...) {
 *       <assignments>
 *       out = WhileFun( in);
 *     }
 *     return( out);
 *
 ******************************************************************************/

info *
AdjustMasksWhile_Post (info *arg_info)
{
    DBUG_ENTER ("AdjustMasksWhile_Post");

    /*
     * Note, that we have a conditional here:
     *   in  = in_t u in_e u (out_t \ out_e) u (out_e \ out_t) = in_t u out_t
     *   out = out_t u out_e = out_t
     *   local = (local_t u local_e) \ in = local_t \ out_t
     */
    DFMSetMaskOr (INFO_INFDFMS_IN (arg_info), INFO_INFDFMS_OUT (arg_info));
    DFMSetMaskMinus (INFO_INFDFMS_LOCAL (arg_info), INFO_INFDFMS_OUT (arg_info));

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

info *
AdjustMasksDo_Pre (info *arg_info, node *arg_node)
{
    DBUG_ENTER ("AdjustMasksDo_Pre");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_do), "wrong node type found!");

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
    DFMSetMaskOr (INFO_INFDFMS_IN (arg_info), DO_OUT_MASK (arg_node));
    DFMSetMaskMinus (INFO_INFDFMS_LOCAL (arg_info), DO_OUT_MASK (arg_node));

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

info *
AdjustMasksDo_Post (info *arg_info)
{
    DBUG_ENTER ("AdjustMasksDo_Post");

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * Function:
 *   node *InferMasksWith( node *arg_node, info *arg_info)
 *
 * Description:
 *   In order to infer the withid-ids as local-vars, withop and code must be
 *   traversed *before* the withid (contained in part)!
 *
 ******************************************************************************/

node *
InferMasksWith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("InferMasksWith");

    /*
     * setup masks
     */
    arg_info
      = GenerateMasks (arg_info, INFO_INFDFMS_IN (arg_info), INFO_INFDFMS_OUT (arg_info),
                       INFO_INFDFMS_NEEDED (arg_info));

    /*
     * adjust masks (part 1)
     */
    arg_info = AdjustMasksWith_Pre (arg_info, arg_node);

    /*
     * traverse sons
     */

    DBUG_EXECUTE ("INFDFMS",
                  fprintf (stderr, ">>>  %s entered", mdb_nodetype[NODE_TYPE (arg_node)]);
                  DbugPrintMasks (arg_info););

    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);
    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);

    DBUG_EXECUTE ("INFDFMS", fprintf (stderr, "<<<  %s finished\n",
                                      mdb_nodetype[NODE_TYPE (arg_node)]););

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
 *
 ******************************************************************************/

node *
InferMasksWith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("InferMasksWith2");

    /*
     * setup masks
     */
    arg_info
      = GenerateMasks (arg_info, INFO_INFDFMS_IN (arg_info), INFO_INFDFMS_OUT (arg_info),
                       INFO_INFDFMS_NEEDED (arg_info));

    /*
     * adjust masks (part 1)
     */
    arg_info = AdjustMasksWith_Pre (arg_info, arg_node);

    /*
     * traverse sons
     */

    DBUG_EXECUTE ("INFDFMS",
                  fprintf (stderr, ">>>  %s entered", mdb_nodetype[NODE_TYPE (arg_node)]);
                  DbugPrintMasks (arg_info););

    NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);
    NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);

    if (NWITH2_CODE (arg_node) != NULL) {
        NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
    }

    NWITH2_WITHID (arg_node) = Trav (NWITH2_WITHID (arg_node), arg_info);

    DBUG_EXECUTE ("INFDFMS", fprintf (stderr, "<<<  %s finished\n",
                                      mdb_nodetype[NODE_TYPE (arg_node)]););

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
 *
 *
 ******************************************************************************/

static node *
InferMasksCond (node *arg_node, info *arg_info)
{
    DFMmask_t old_in, old_out, old_needed;
    DFMmask_t in_then, out_then, local_then;
    DFMmask_t in_else, out_else, local_else;

    DBUG_ENTER ("InferMasksCond");

    old_in = INFO_INFDFMS_IN (arg_info);
    old_out = INFO_INFDFMS_OUT (arg_info);
    old_needed = INFO_INFDFMS_NEEDED (arg_info);

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

    DBUG_EXECUTE ("INFDFMS", fprintf (stderr, ">>>  then-block of %s entered",
                                      mdb_nodetype[NODE_TYPE (arg_node)]);
                  DbugPrintMasks (arg_info););

    COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);

    DBUG_EXECUTE ("INFDFMS", fprintf (stderr, "<<<  then-block of %s finished\n",
                                      mdb_nodetype[NODE_TYPE (arg_node)]););

    in_then = INFO_INFDFMS_IN (arg_info);
    out_then = INFO_INFDFMS_OUT (arg_info);
    local_then = INFO_INFDFMS_LOCAL (arg_info);

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

    DBUG_EXECUTE ("INFDFMS", fprintf (stderr, ">>>  else-block of %s entered",
                                      mdb_nodetype[NODE_TYPE (arg_node)]);
                  DbugPrintMasks (arg_info););

    COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);

    DBUG_EXECUTE ("INFDFMS", fprintf (stderr, "<<<  else-block of %s finished\n",
                                      mdb_nodetype[NODE_TYPE (arg_node)]););

    in_else = INFO_INFDFMS_IN (arg_info);
    out_else = INFO_INFDFMS_OUT (arg_info);
    local_else = INFO_INFDFMS_LOCAL (arg_info);

    /*
     * adjust masks (part 2)
     */
    arg_info = AdjustMasksCond_Post (arg_info, in_then, out_then, local_then, in_else,
                                     out_else, local_else);

    /*
     * traverse condition
     */
    COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *InferMasksWhile( node *arg_node, info *arg_info)
 *
 * Description:
 *   BODY must be traversed before COND!
 *
 ******************************************************************************/

node *
InferMasksWhile (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("InferMasksWhile");

    /*
     * setup masks
     */
    arg_info
      = GenerateMasks (arg_info, INFO_INFDFMS_IN (arg_info), INFO_INFDFMS_OUT (arg_info),
                       INFO_INFDFMS_NEEDED (arg_info));

    /*
     * adjust masks (part 1)
     */
    arg_info = AdjustMasksWhile_Pre (arg_info, arg_node);

    /*
     * traverse sons
     */

    DBUG_EXECUTE ("INFDFMS",
                  fprintf (stderr, ">>>  %s entered", mdb_nodetype[NODE_TYPE (arg_node)]);
                  DbugPrintMasks (arg_info););

    WHILE_BODY (arg_node) = Trav (WHILE_BODY (arg_node), arg_info);
    WHILE_COND (arg_node) = Trav (WHILE_COND (arg_node), arg_info);

    DBUG_EXECUTE ("INFDFMS", fprintf (stderr, "<<<  %s finished\n",
                                      mdb_nodetype[NODE_TYPE (arg_node)]););

    /*
     * adjust masks (part 2)
     */
    arg_info = AdjustMasksWhile_Post (arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *InferMasksDo( node *arg_node, info *arg_info)
 *
 * Description:
 *   BODY must be traversed after COND!
 *
 ******************************************************************************/

node *
InferMasksDo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("InferMasksDo");

    /*
     * setup masks
     */
    arg_info
      = GenerateMasks (arg_info, INFO_INFDFMS_IN (arg_info), INFO_INFDFMS_OUT (arg_info),
                       INFO_INFDFMS_NEEDED (arg_info));

    /*
     * adjust masks (part 1)
     */
    arg_info = AdjustMasksDo_Pre (arg_info, arg_node);

    /*
     * traverse sons
     */

    DBUG_EXECUTE ("INFDFMS",
                  fprintf (stderr, ">>>  %s entered", mdb_nodetype[NODE_TYPE (arg_node)]);
                  DbugPrintMasks (arg_info););

    DO_COND (arg_node) = Trav (DO_COND (arg_node), arg_info);
    DO_BODY (arg_node) = Trav (DO_BODY (arg_node), arg_info);

    DBUG_EXECUTE ("INFDFMS", fprintf (stderr, "<<<  %s finished\n",
                                      mdb_nodetype[NODE_TYPE (arg_node)]););

    /*
     * adjust masks (part 2)
     */
    arg_info = AdjustMasksDo_Post (arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *InferMasks( DFMmask_t *in, DFMmask_t *out, DFMmask_t *local,
 *                     node *arg_node, info *arg_info,
 *                     node *(InferMasksFun)( node *, node *),
 *                     bool do_fixpoint_iter)
 *
 * description:
 *   Infers the in-, out-, local-vars of a conditional or loop.
 *
 ******************************************************************************/

static node *
InferMasks (DFMmask_t *in, DFMmask_t *out, DFMmask_t *local, node *arg_node,
            info *arg_info, node *(InferMasksFun) (node *, info *), bool do_fixpoint_iter)
{
    DFMmask_t old_needed, old_in, old_out, old_local;

    DBUG_ENTER ("InferMasks");

    if (INFO_INFDFMS_FIRST (arg_info)) {
        /*
         * first traversal
         *  -> init the given in/out/local-masks!!!!
         */
        (*in) = DFMGenMaskClear (INFO_DFM_BASE (arg_info));
        (*out) = DFMGenMaskClear (INFO_DFM_BASE (arg_info));
        (*local) = DFMGenMaskClear (INFO_DFM_BASE (arg_info));
    }

    /*
     * save old masks
     */
    old_needed = INFO_INFDFMS_NEEDED (arg_info);
    old_in = INFO_INFDFMS_IN (arg_info);
    old_out = INFO_INFDFMS_OUT (arg_info);
    old_local = INFO_INFDFMS_LOCAL (arg_info);

    /*
     * infer new masks
     */
    arg_node = InferMasksFun (arg_node, arg_info);

    /*
     * store the infered in-, out-, local-masks and
     * detect whether the fixpoint-property is hold or not
     */
    if (do_fixpoint_iter) {
        COMPARE_AND_UPDATE (*in, INFO_INFDFMS_IN (arg_info), arg_info);
        COMPARE_AND_UPDATE (*out, INFO_INFDFMS_OUT (arg_info), arg_info);
        COMPARE_AND_UPDATE (*local, INFO_INFDFMS_LOCAL (arg_info), arg_info);
    } else {
        /*
         * no fixpoint iteration needed
         */
        UPDATE (*in, INFO_INFDFMS_IN (arg_info));
        UPDATE (*out, INFO_INFDFMS_OUT (arg_info));
        UPDATE (*local, INFO_INFDFMS_LOCAL (arg_info));
    }

    DBUG_EXECUTE ("INFDFMS",
                  DbugPrintSignature (NODE_TEXT (arg_node), *in, *out, *local););

    /*
     * update old local-mask
     */
    if (TEST_HIDE_LOCALS (INFO_INFDFMS_HIDELOC (arg_info), arg_node)) {
        /*
         * we have to hide the local vars!!
         */
        DBUG_PRINT ("INFDFMS_ALL",
                    ("local vars of node %s are hid!!!", NODE_TEXT (arg_node)));
    } else {
        DBUG_PRINT ("INFDFMS_ALL",
                    ("local vars of node %s are not hid.", NODE_TEXT (arg_node)));
        DFMSetMaskOr (old_local, *local);
    }

    /*
     * restore old needed-mask
     */
    INFO_INFDFMS_NEEDED (arg_info) = DFMRemoveMask (INFO_INFDFMS_NEEDED (arg_info));
    INFO_INFDFMS_NEEDED (arg_info) = old_needed;

    /*
     * restore old in-, out-, local-mask
     */
    INFO_INFDFMS_IN (arg_info) = old_in;
    INFO_INFDFMS_OUT (arg_info) = old_out;
    INFO_INFDFMS_LOCAL (arg_info) = old_local;

    /*
     * The whole conditional/loop can be represented by a single function call
     *     ...out-vars... dummy-fun( ...in-vars... )
     * Therefore we must adjust the current masks accordingly.
     *
     * Note, that the local-mask must has been updated already!!!!
     */
    arg_info = DefinedMask (arg_info, *out);
    arg_info = UsedMask (arg_info, *in);

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
    DFMmask_base_t old_dfm_base;
#ifndef DBUG_OFF
    int cnt = 0;
#endif

    DBUG_ENTER ("INFDFMSfundef");

    INFO_INFDFMS_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_EXECUTE ("INFDFMS", fprintf (stderr, ">>>>>>>  function %s():\n",
                                          FUNDEF_NAME (arg_node)););

        old_dfm_base = FUNDEF_DFM_BASE (arg_node);
        if (old_dfm_base == NULL) {
            FUNDEF_DFM_BASE (arg_node)
              = DFMGenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));

            DBUG_PRINT ("INFDFMS_ALL", ("no DFM base found -> created (" F_PTR ")",
                                        FUNDEF_DFM_BASE (arg_node)));
        } else {
            FUNDEF_DFM_BASE (arg_node)
              = DFMUpdateMaskBase (old_dfm_base, FUNDEF_ARGS (arg_node),
                                   FUNDEF_VARDEC (arg_node));

            DBUG_ASSERT ((FUNDEF_DFM_BASE (arg_node) == old_dfm_base),
                         "address of DFM base has changed during update!");

            DBUG_PRINT ("INFDFMS_ALL", ("DFM base found -> updated (" F_PTR ")",
                                        FUNDEF_DFM_BASE (arg_node)));
        }

        INFO_INFDFMS_IN (arg_info) = DFMGenMaskClear (FUNDEF_DFM_BASE (arg_node));
        INFO_INFDFMS_OUT (arg_info) = DFMGenMaskClear (FUNDEF_DFM_BASE (arg_node));
        INFO_INFDFMS_LOCAL (arg_info) = DFMGenMaskClear (FUNDEF_DFM_BASE (arg_node));
        INFO_INFDFMS_NEEDED (arg_info) = DFMGenMaskClear (FUNDEF_DFM_BASE (arg_node));

        INFO_INFDFMS_FIRST (arg_info) = TRUE;
        do {
            DBUG_EXECUTE ("INFDFMS", cnt++;
                          fprintf (stderr, "\n>>>>>  fixpoint iteration --- loop %i\n",
                                   cnt););
            INFO_INFDFMS_ISFIX (arg_info) = TRUE;

            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

            DFMSetMaskClear (INFO_INFDFMS_IN (arg_info));
            DFMSetMaskClear (INFO_INFDFMS_OUT (arg_info));
            DFMSetMaskClear (INFO_INFDFMS_LOCAL (arg_info));
            DFMSetMaskClear (INFO_INFDFMS_NEEDED (arg_info));
            INFO_INFDFMS_FIRST (arg_info) = FALSE;
        } while (!INFO_INFDFMS_ISFIX (arg_info));

        DBUG_EXECUTE ("INFDFMS",
                      fprintf (stderr, "<<<<<<<  %s(): finished after %i iterations\n",
                               FUNDEF_NAME (arg_node), cnt););

        INFO_INFDFMS_IN (arg_info) = DFMRemoveMask (INFO_INFDFMS_IN (arg_info));
        INFO_INFDFMS_OUT (arg_info) = DFMRemoveMask (INFO_INFDFMS_OUT (arg_info));
        INFO_INFDFMS_LOCAL (arg_info) = DFMRemoveMask (INFO_INFDFMS_LOCAL (arg_info));
        INFO_INFDFMS_NEEDED (arg_info) = DFMRemoveMask (INFO_INFDFMS_NEEDED (arg_info));
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
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
    DBUG_ENTER ("INFDFMSarg");

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

    DBUG_ENTER ("INFDFMSassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        assign_next = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

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
    DBUG_ENTER ("INFDFMSlet");

    arg_info = DefinedIds (arg_info, LET_IDS (arg_node));

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    DBUG_EXECUTE ("INFDFMS_ALL",
                  fprintf (stderr, "%s traversed (", mdb_nodetype[NODE_TYPE (arg_node)]);
                  {
                      ids *let_ids = LET_IDS (arg_node);

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
                      fprintf (stderr, "[%s])",
                               mdb_nodetype[NODE_TYPE (LET_EXPR (arg_node))]);
                  } DbugPrintMasks (arg_info););

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

    DBUG_ENTER ("INFDFMSap");

    /*
     * search for reference parameters and mark them as 'defined vars'
     * (resolve them explicitly)
     */
    DBUG_ASSERT ((AP_FUNDEF (arg_node) != NULL), "AP_FUNDEF not found!");

    /* traverse the formal (fundef_args) and current (ap_args) parameters */
    fundef_args = FUNDEF_ARGS (AP_FUNDEF (arg_node));
    ap_args = AP_ARGS (arg_node);
    while (ap_args != NULL) {
        if ((ARG_ATTRIB (fundef_args) == ST_reference)
            || (ARG_ATTRIB (fundef_args) == ST_readonly_reference)) {
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (ap_args)) == N_id),
                         "Reference parameter must be a N_id node!");

            decl = ID_VARDEC (EXPRS_EXPR (ap_args));
            if ((NODE_TYPE (decl) == N_arg)
                && ((ARG_ATTRIB (decl) == ST_reference)
                    || (ARG_ATTRIB (decl) == ST_readonly_reference))) {
                /*
                 * argument is used as reference parameter of the application,
                 * but its declaration is already a reference parameter, too
                 *   -> do *not* mask as defined variable
                 *      (it is no out-var but a reference-in-var!)
                 */
                DBUG_PRINT ("INFDFMS", ("N_ap in %s() with reference as ref. parameter:"
                                        "  %s( .. %s .. )",
                                        FUNDEF_NAME (INFO_INFDFMS_FUNDEF (arg_info)),
                                        FUNDEF_NAME (AP_FUNDEF (arg_node)),
                                        ID_NAME (EXPRS_EXPR (ap_args))));
            } else {
                /*
                 * argument (which declaration is not a reference parameter) is
                 * used as reference parameter of the application
                 *   -> mark as defined variable (must be a out-var as well)
                 */
                DBUG_PRINT ("INFDFMS",
                            ("N_ap in %s() with non-reference as ref. parameter:"
                             "  %s( .. %s .. )",
                             FUNDEF_NAME (INFO_INFDFMS_FUNDEF (arg_info)),
                             FUNDEF_NAME (AP_FUNDEF (arg_node)),
                             ID_NAME (EXPRS_EXPR (ap_args))));
                arg_info = DefinedVar (arg_info, decl);
            }
        }

        if (ARG_BASETYPE (fundef_args) != T_dots) {
            /*
             * This is a function with variable parameter list. Example:
             *   extern void printf( string FORMAT, ... ) { ... }
             *   ...
             *   printf( "%i %i", a_int, b_int)
             */
            fundef_args = ARG_NEXT (fundef_args);
        }
        ap_args = EXPRS_NEXT (ap_args);
    }
    if (fundef_args != NULL) {
        DBUG_ASSERT ((ARG_BASETYPE (fundef_args) == T_dots),
                     "Partial function application found!");
    }

    /*
     * traverse the arguments -> mark them as 'used vars'
     */
    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
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
    DBUG_ENTER ("INFDFMSid");

    arg_info = UsedVar (arg_info, ID_VARDEC (arg_node));

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
INFDFMSwithx (node *arg_node, info *arg_info)
{
    DFMmask_t nwith_in, nwith_out, nwith_local;
    DFMmask_t out;

    DBUG_ENTER ("INFDFMSwithx");

    nwith_in = NWITH_OR_NWITH2_IN_MASK (arg_node);
    nwith_out = NWITH_OR_NWITH2_OUT_MASK (arg_node);
    nwith_local = NWITH_OR_NWITH2_LOCAL_MASK (arg_node);

    arg_node
      = InferMasks (&nwith_in, &nwith_out, &nwith_local, arg_node, arg_info,
                    (NODE_TYPE (arg_node) == N_Nwith) ? InferMasksWith : InferMasksWith2,
                    FALSE);

    L_NWITH_OR_NWITH2_IN_MASK (arg_node, nwith_in);
    L_NWITH_OR_NWITH2_OUT_MASK (arg_node, nwith_out);
    L_NWITH_OR_NWITH2_LOCAL_MASK (arg_node, nwith_local);

    /*
     * A with-loop indeed *can* have out-vars:
     * Some global objects might be modified within the with-loop. This is legal
     * in SAC although it can cause non-deterministic results!
     * Therfore, we print a warning message here.
     */
    out = NWITH_OR_NWITH2_OUT_MASK (arg_node);
    if ((out != NULL) && (DFMGetMaskEntryDeclSet (out) != NULL)) {
#if 0
    WARN( NODE_LINE( arg_node), ("with-loop with out-vars detected"));
#else
        DBUG_PRINT ("INFDFMS", ("with-loop with out-vars detected!"));
#endif
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INFDFMSwithid( node *arg_node, info *arg_info)
 *
 * description:
 *   Every index variable is marked as 'defined' in the in-, out-,
 *   local-masks of the current block.
 *
 ******************************************************************************/

node *
INFDFMSwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("INFDFMSwithid");

    arg_info = DefinedIds (arg_info, NWITHID_VEC (arg_node));
    arg_info = DefinedIds (arg_info, NWITHID_IDS (arg_node));

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
    DBUG_ENTER ("INFDFMScode");

    NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);
    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

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
    DBUG_ENTER ("INFDFMScond");

    arg_node = InferMasks (&(COND_IN_MASK (arg_node)), &(COND_OUT_MASK (arg_node)),
                           &(COND_LOCAL_MASK (arg_node)), arg_node, arg_info,
                           InferMasksCond, FALSE);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INFDFMSwhile( node *arg_node, info *arg_info)
 *
 * description:
 *   Inferes the in-, out-, local-parameters of the while-loop.
 *
 ******************************************************************************/

node *
INFDFMSwhile (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("INFDFMSwhile");

    arg_node = InferMasks (&(WHILE_IN_MASK (arg_node)), &(WHILE_OUT_MASK (arg_node)),
                           &(WHILE_LOCAL_MASK (arg_node)), arg_node, arg_info,
                           InferMasksWhile, TRUE);

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
    DBUG_ENTER ("INFDFMSdo");

    arg_node
      = InferMasks (&(DO_IN_MASK (arg_node)), &(DO_OUT_MASK (arg_node)),
                    &(DO_LOCAL_MASK (arg_node)), arg_node, arg_info, InferMasksDo, TRUE);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *INFDFMSicm( node *arg_node, info *arg_info)
 *
 * Description:
 *   ICMs must be handled indiviually in order to prevent the introduction of
 *   wrong data dependencies.
 *
 ******************************************************************************/

node *
INFDFMSicm (node *arg_node, info *arg_info)
{
    char *name;

    DBUG_ENTER ("INFDFMSicm");

    DBUG_PRINT ("INFDFMS", ("icm %s found", ICM_NAME (arg_node)));

    name = ICM_NAME (arg_node);
    if (strstr (name, "USE_GENVAR_OFFSET") != NULL) {
        /*
         * USE_GENVAR_OFFSET( off_nt, wl_nt):
         *   off_nt = wl_nt__destptr;
         *
         * def: 1st arg
         * use: ---
         */
        arg_info = DefinedId (arg_info, ICM_ARG1 (arg_node));
    } else if (strstr (name, "VECT2OFFSET") != NULL) {
        /*
         * VECT2OFFSET( off_nt, ., from_nt, ., ...):
         *   off_nt = ... from_nt ...;
         *
         * def: 1st arg
         * use: 3nd arg
         */
        arg_info = DefinedId (arg_info, ICM_ARG1 (arg_node));
        arg_info = UsedId (arg_info, ICM_ARG3 (arg_node));
    } else if (strstr (name, "IDXS2OFFSET") != NULL) {
        /*
         * IDXS2OFFSET( off_nt, i, idxs_nt, ., ...):
         *   off_nt = ... idxs_nt[0] ... idxs_nt[i-1] ...;
         *
         * def: 1st arg
         * use: 3nd arg (var-arg!!)
         */
        node *exprs;
        int cnt;

        arg_info = DefinedId (arg_info, ICM_ARG1 (arg_node));
        DBUG_ASSERT (((NODE_TYPE (ICM_ARG2 (arg_node)) == N_num)
                      && (NUM_VAL (ICM_ARG2 (arg_node)) >= 0)),
                     "illegal counter for var-arg of IDXS2OFFSET found!");
        cnt = NUM_VAL (ICM_ARG2 (arg_node));
        exprs = ICM_EXPRS3 (arg_node);
        while (cnt > 0) {
            DBUG_ASSERT ((exprs != NULL), "var-arg of IDXSOFFSET is inconsistant!");
            arg_info = UsedId (arg_info, EXPRS_EXPR (exprs));
            cnt--;
            exprs = EXPRS_NEXT (exprs);
        }
        DBUG_ASSERT (((exprs != NULL) && (NODE_TYPE (EXPRS_EXPR (exprs)) == N_num)),
                     "var-arg of IDXSOFFSET is inconsistant!");
    } else {
        DBUG_ASSERT ((0), "unknown ICM found!");
    }

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
InferDFMs (node *syntax_tree, int hide_locals)
{
    info *info_node;
    funtab *old_funtab;

    DBUG_ENTER ("InferDFMs");

    DBUG_ASSERT (((NODE_TYPE (syntax_tree) == N_modul)
                  || (NODE_TYPE (syntax_tree) == N_fundef)),
                 "argument of InferDFMs() must be a N_modul or a N_fundef node!");

    info_node = MakeInfo ();
    INFO_INFDFMS_HIDELOC (info_node) = hide_locals;

    old_funtab = act_tab;
    act_tab = infdfms_tab;
    syntax_tree = Trav (syntax_tree, info_node);
    act_tab = old_funtab;

    info_node = FreeInfo (info_node);

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * function:
 *   DFMmask_t InferInDFMAssignChain
 *
 * description:
 *   Infers the inmask of a given assignment chain which is located in
 *   a given function
 *
 *****************************************************************************/
DFMmask_t
InferInDFMAssignChain (node *assign, node *fundef)
{
    info *info;
    funtab *old_funtab;
    DFMmask_t res;
    DFMmask_base_t old_dfm_base;

    DBUG_ENTER ("InferInDFMAssignChain");

    DBUG_ASSERT ((NODE_TYPE (assign) == N_assign),
                 "argument of InferInDFMAssignChain() must be a N_assign node!");
    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "second argument of InferInDFMAssignChain() must be a N_fundef");

    info = MakeInfo ();
    INFO_INFDFMS_HIDELOC (info) = HIDE_LOCALS_NEVER;
    INFO_INFDFMS_FUNDEF (info) = fundef;

    old_dfm_base = FUNDEF_DFM_BASE (fundef);
    if (old_dfm_base == NULL) {
        FUNDEF_DFM_BASE (fundef)
          = DFMGenMaskBase (FUNDEF_ARGS (fundef), FUNDEF_VARDEC (fundef));

        DBUG_PRINT ("INFDFMS_ALL", ("no DFM base found -> created (" F_PTR ")",
                                    FUNDEF_DFM_BASE (fundef)));
    } else {
        FUNDEF_DFM_BASE (fundef) = DFMUpdateMaskBase (old_dfm_base, FUNDEF_ARGS (fundef),
                                                      FUNDEF_VARDEC (fundef));

        DBUG_ASSERT ((FUNDEF_DFM_BASE (fundef) == old_dfm_base),
                     "address of DFM base has changed during update!");

        DBUG_PRINT ("INFDFMS_ALL",
                    ("DFM base found -> updated (" F_PTR ")", FUNDEF_DFM_BASE (fundef)));
    }

    INFO_INFDFMS_IN (info) = DFMGenMaskClear (FUNDEF_DFM_BASE (fundef));
    INFO_INFDFMS_OUT (info) = DFMGenMaskClear (FUNDEF_DFM_BASE (fundef));
    INFO_INFDFMS_LOCAL (info) = DFMGenMaskClear (FUNDEF_DFM_BASE (fundef));
    INFO_INFDFMS_NEEDED (info) = DFMGenMaskClear (FUNDEF_DFM_BASE (fundef));

    INFO_INFDFMS_FIRST (info) = TRUE;

    old_funtab = act_tab;
    act_tab = infdfms_tab;
    assign = Trav (assign, info);
    act_tab = old_funtab;

    res = DFMGenMaskCopy (INFO_INFDFMS_IN (info));
    DFMSetMaskMinus (res, INFO_INFDFMS_LOCAL (info));

    INFO_INFDFMS_IN (info) = DFMRemoveMask (INFO_INFDFMS_IN (info));
    INFO_INFDFMS_OUT (info) = DFMRemoveMask (INFO_INFDFMS_OUT (info));
    INFO_INFDFMS_LOCAL (info) = DFMRemoveMask (INFO_INFDFMS_LOCAL (info));
    INFO_INFDFMS_NEEDED (info) = DFMRemoveMask (INFO_INFDFMS_NEEDED (info));

    info = FreeInfo (info);

    DBUG_EXECUTE ("INFDFMS_AC", DFMPrintMask (0, " %s ", res););

    DBUG_RETURN (res);
}
