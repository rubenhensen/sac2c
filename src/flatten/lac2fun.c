/*
 *
 * $Log$
 * Revision 1.24  2000/10/17 17:03:43  dkr
 * define-flag MAIN_HAS_NO_MODNAME inverted
 *
 * Revision 1.23  2000/10/17 16:50:54  dkr
 * _MAIN replaced by macro MAIN_MOD_NAME
 *
 * Revision 1.22  2000/06/23 15:01:55  dkr
 * signature of DupTreeLUT changed
 *
 * Revision 1.21  2000/05/25 17:19:13  dkr
 * header added
 *
 * Revision 1.20  2000/03/24 15:21:25  dkr
 * fixed a bug in L2F_INFERap: varargs are handle correctly now
 *
 * Revision 1.19  2000/03/24 00:51:56  dkr
 * handling of reference parameters corrected
 *
 * Revision 1.18  2000/03/21 14:53:31  dkr
 * ASSERT added: For the time being Lac2fun() can be used after type
 * checking only
 *
 * Revision 1.17  2000/03/17 18:30:36  dkr
 * type lut_t* replaced by LUT_t
 *
 * Revision 1.16  2000/03/17 16:34:28  dkr
 * some superfluous local vars eliminated
 *
 * Revision 1.15  2000/03/17 16:00:49  dkr
 * include of cleanup_decls.h added
 *
 * Revision 1.14  2000/03/17 15:59:18  dkr
 * added call of CleanupDecls()
 *
 * Revision 1.13  2000/02/24 16:53:28  dkr
 * fixed a bug in InferMasks:
 * in case of do-loops the condition must be traversed *before* the body
 *
 * Revision 1.12  2000/02/24 15:06:07  dkr
 * some comments and dbug-output added
 *
 * Revision 1.11  2000/02/24 01:27:55  dkr
 * lac2fun completed now 8-))
 *
 * Revision 1.10  2000/02/24 00:27:55  dkr
 * lac2fun works now correct for conditionals and while-loops 8-))
 *
 * Revision 1.9  2000/02/17 11:36:07  dkr
 * FUNDEF_LAC_LET removed
 *
 * Revision 1.8  2000/02/09 16:38:00  dkr
 * Workaround for main function: For the time being the main function
 * has an empty module name. Therefore the module name for LAC functions
 * must be set by hand if the function is lifted from main().
 *
 * Revision 1.7  2000/02/09 15:06:05  dkr
 * the parts of a with-loop are traversed in the correct order now
 * the modul name of the lifted fundef is set correctly now
 *
 * Revision 1.6  2000/02/09 09:59:16  dkr
 * FUNDEF_LAC_LET added
 * global objects are handled correctly now
 *
 * Revision 1.5  2000/02/08 16:40:33  dkr
 * LAC2FUNwith() and LAC2FUNwith2() added
 *
 * Revision 1.4  2000/02/08 15:14:32  dkr
 * LAC2FUNwithid added
 * some bugs fixed
 *
 * Revision 1.3  2000/02/08 10:17:02  dkr
 * wrong include instruction removed
 *
 * Revision 1.2  2000/02/03 17:29:23  dkr
 * conditions are lifted now correctly :)
 *
 * Revision 1.1  2000/01/21 12:48:59  dkr
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   lac2fun.c
 *
 * prefix: L2F_INFER, L2F_LIFT
 *
 * description:
 *
 *   This compiler module implements the conversion of conditionals and
 *   loops into their true functional representation.
 *
 * usage of arg_info (INFO_LAC2FUN_...):
 *
 *   during INFER-phase:
 *
 *     ...FUNDEF   pointer to the current fundef
 *
 *     ...NEEDED   DFmask: vars needed in outer blocks
 *     ...IN       DFmask: vars used before eventually defined (in the current block)
 *     ...OUT      DFmask: vars defined an needed in outer blocks
 *     ...LOCAL    DFmask: vars defined before eventually used
 *     (Note: Each var occuring in a block is either IN-var or (exclusive!) LOCAL-var)
 *
 *     ...ISFIX    flag: fixpoint reached?
 *
 *   during LIFT-phase:
 *
 *     ...FUNDEF   pointer to the current fundef
 *
 *     ...ISTRANS  flag: has the current assignment been modified?
 *     ...FUNS     chain of newly generated dummy functions
 *
 *****************************************************************************/

#include "tree.h"
#include "traverse.h"
#include "free.h"
#include "dbug.h"
#include "DupTree.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"
#include "cleanup_decls.h"

/*
 *
 * functions for the first traversal(s) [fixpoint interation]
 *
 */

/*
 * The current value of the DFmask 'old' is freed and subsequently the
 * value of 'new' is assigned to 'old'.
 */
#define UPDATE(old, new)                                                                 \
    if (old != NULL) {                                                                   \
        old = DFMRemoveMask (old);                                                       \
    }                                                                                    \
    old = new;

/*
 * Before UPDATE() is called, the two DFmasks 'old' and 'new' are compared.
 * If they differ the flag INFO_LAC2FUN_ISFIX is unset to indicate that
 * another iteration is needed (fixpoint not yet reached).
 */
#define COMPARE_AND_UPDATE(old, new)                                                     \
    if (old != NULL) {                                                                   \
        if (DFMTestMask (old) != DFMTest2Masks (old, new)) {                             \
            /* 'old' and 'new' differs */                                                \
            INFO_LAC2FUN_ISFIX (arg_info) = 0;                                           \
        }                                                                                \
    } else {                                                                             \
        INFO_LAC2FUN_ISFIX (arg_info) = 0;                                               \
    }                                                                                    \
    UPDATE (old, new);

/*
 * compound macro
 */
#define INFO_DFMBASE(arg_info) FUNDEF_DFM_BASE (INFO_LAC2FUN_FUNDEF (arg_info))

/******************************************************************************
 *
 * function:
 *   void DefinedVar( node *decl,
 *                    DFMmask_t needed,
 *                    DFMmask_t in, DFMmask_t out, DFMmask_t local)
 *
 * description:
 *   Updates the given in-, out-, local-masks according to a defining occurence
 *   of a variable. The variable can either be specified directly (id) or as
 *   a pointer to the declaration (decl). The unused parameter has to be NULL.
 *
 ******************************************************************************/

static void
DefinedVar (node *decl, DFMmask_t needed, DFMmask_t in, DFMmask_t out, DFMmask_t local)
{
    DBUG_ENTER ("DefinedVar");

    DBUG_ASSERT ((decl != NULL),
                 "Variable declaration missing!"
                 "For the time being Lac2fun() can be used after type checking only!");

    if ((NODE_TYPE (decl) != N_vardec) && (NODE_TYPE (decl) != N_arg)) {
        DBUG_ASSERT ((NODE_TYPE (decl) == N_objdef),
                     "declaration is neither a N_arg/N_vardec-node nor a N_objdef-node");
    } else {
        DFMSetMaskEntryClear (in, NULL, decl);
        if (DFMTestMaskEntry (needed, NULL, decl)) {
            DFMSetMaskEntrySet (out, NULL, decl);
        }
        DFMSetMaskEntrySet (local, NULL, decl);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void DefinedIds( ids *_ids, node *arg_info)
 *
 * description:
 *   Calls 'DefinedVar()' for each ids of the given ids-chain.
 *
 ******************************************************************************/

static void
DefinedIds (ids *_ids, node *arg_info)
{
    DBUG_ENTER ("DefinedIds");

    while (_ids != NULL) {
        DefinedVar (IDS_VARDEC (_ids), INFO_LAC2FUN_NEEDED (arg_info),
                    INFO_LAC2FUN_IN (arg_info), INFO_LAC2FUN_OUT (arg_info),
                    INFO_LAC2FUN_LOCAL (arg_info));
        _ids = IDS_NEXT (_ids);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void DefinedMask( DFMmask_t mask, node *arg_info)
 *
 * description:
 *   Calls 'DefinedVar()' for each variable set in the given mask.
 *
 ******************************************************************************/

static void
DefinedMask (DFMmask_t mask, node *arg_info)
{
    node *decl;

    DBUG_ENTER ("DefinedMask");

    decl = DFMGetMaskEntryDeclSet (mask);
    while (decl != NULL) {
        DefinedVar (decl, INFO_LAC2FUN_NEEDED (arg_info), INFO_LAC2FUN_IN (arg_info),
                    INFO_LAC2FUN_OUT (arg_info), INFO_LAC2FUN_LOCAL (arg_info));
        decl = DFMGetMaskEntryDeclSet (NULL);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void UsedVar( node *decl,
 *                 DFMmask_t in, DFMmask_t local)
 *
 * description:
 *   Updates the given in-, out-, local-masks according to a non-defining
 *   occurence of a variable. The variable can either be specified directly (id)
 *   or as a pointer to the declaration (decl). The unused parameter has to be
 *   NULL.
 *
 ******************************************************************************/

static void
UsedVar (node *decl, DFMmask_t in, DFMmask_t local)
{
    DBUG_ENTER ("UsedVar");

    DBUG_ASSERT ((decl != NULL),
                 "Variable declaration missing! "
                 "For the time being Lac2fun() can be used after type checking only!");

    if ((NODE_TYPE (decl) != N_vardec) && (NODE_TYPE (decl) != N_arg)) {
        DBUG_ASSERT ((NODE_TYPE (decl) == N_objdef),
                     "declaration is neither a N_arg/N_vardec-node nor a N_objdef-node");
    } else {
        DFMSetMaskEntrySet (in, NULL, decl);
        DFMSetMaskEntryClear (local, NULL, decl);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void UsedMask( DFMmask_t mask, node *arg_info)
 *
 * description:
 *   Calls 'UsedVar()' for each variable set in the given mask.
 *
 ******************************************************************************/

static void
UsedMask (DFMmask_t mask, node *arg_info)
{
    node *decl;

    DBUG_ENTER ("UsedMask");

    decl = DFMGetMaskEntryDeclSet (mask);
    while (decl != NULL) {
        UsedVar (decl, INFO_LAC2FUN_IN (arg_info), INFO_LAC2FUN_LOCAL (arg_info));
        decl = DFMGetMaskEntryDeclSet (NULL);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void AdjustNeeded( DFMmask_t needed,
 *                      DFMmask_t in, DFMmask_t out)
 *
 * description:
 *   Updates the needed-mask (contains all vars, that are needed in outer
 *   blocks) according to the in- and out-mask of the current block.
 *   This function is needed to calculate the new needed-mask before entering
 *   a new block.
 *
 ******************************************************************************/

static void
AdjustNeeded (DFMmask_t needed, DFMmask_t in, DFMmask_t out)
{
    DBUG_ENTER ("AdjustNeeded");

    DFMSetMaskMinus (needed, out);
    DFMSetMaskOr (needed, in);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   node *InferMasks( DFMmask_t *in, DFMmask_t *out, DFMmask_t *local,
 *                     node *arg_node, node *arg_info)
 *
 * description:
 *   Inferes the in-, out-, local-vars of a conditional or loop.
 *
 ******************************************************************************/

static node *
InferMasks (DFMmask_t *in, DFMmask_t *out, DFMmask_t *local, node *arg_node,
            node *arg_info)
{
    DFMmask_t old_needed, old_in, old_out, old_local;
    DFMmask_t in_then, out_then, local_then;
    DFMmask_t in_else, out_else, local_else;
    DFMmask_t tmp;
    nodetype type = NODE_TYPE (arg_node);

    DBUG_ENTER ("InferMasks");

    /*
     * save old masks
     */
    old_needed = INFO_LAC2FUN_NEEDED (arg_info);
    old_in = INFO_LAC2FUN_IN (arg_info);
    old_out = INFO_LAC2FUN_OUT (arg_info);
    old_local = INFO_LAC2FUN_LOCAL (arg_info);

    /*
     * setup needed-masks for then/else-block and loop-block respectively
     */
    INFO_LAC2FUN_NEEDED (arg_info) = DFMGenMaskCopy (old_needed);
    AdjustNeeded (INFO_LAC2FUN_NEEDED (arg_info), old_in, old_out);

    /*
     * setup in-, out-, local-masks for then-block and loop-block respectively
     */
    if (*in != NULL) {
        INFO_LAC2FUN_IN (arg_info) = DFMGenMaskCopy (*in);
    } else {
        INFO_LAC2FUN_IN (arg_info) = DFMGenMaskClear (INFO_DFMBASE (arg_info));
    }
    if (*out != NULL) {
        INFO_LAC2FUN_OUT (arg_info) = DFMGenMaskCopy (*out);
    } else {
        INFO_LAC2FUN_OUT (arg_info) = DFMGenMaskClear (INFO_DFMBASE (arg_info));
    }
    INFO_LAC2FUN_LOCAL (arg_info) = DFMGenMaskClear (INFO_DFMBASE (arg_info));

    switch (type) {
    case N_cond:
        /*
         * traverse then-block
         */
        COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);

        in_then = INFO_LAC2FUN_IN (arg_info);
        out_then = INFO_LAC2FUN_OUT (arg_info);
        local_then = INFO_LAC2FUN_LOCAL (arg_info);

        /*
         * setup in-, out-, local-masks for else-block
         */
        INFO_LAC2FUN_IN (arg_info) = DFMGenMaskClear (INFO_DFMBASE (arg_info));
        INFO_LAC2FUN_OUT (arg_info) = DFMGenMaskClear (INFO_DFMBASE (arg_info));
        INFO_LAC2FUN_LOCAL (arg_info) = DFMGenMaskClear (INFO_DFMBASE (arg_info));

        COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);

        in_else = INFO_LAC2FUN_IN (arg_info);
        out_else = INFO_LAC2FUN_OUT (arg_info);
        local_else = INFO_LAC2FUN_LOCAL (arg_info);

        /*
         * calculate new in-, out-, local-masks and traverse condition
         */
        /* in = in_then u in_else u (out_then \ out_else) u (out_else \ out_then) */
        INFO_LAC2FUN_IN (arg_info) = DFMGenMaskMinus (out_then, out_else);
        tmp = DFMGenMaskMinus (out_else, out_then);
        DFMSetMaskOr (INFO_LAC2FUN_IN (arg_info), tmp);
        DFMSetMaskOr (INFO_LAC2FUN_IN (arg_info), in_then);
        DFMSetMaskOr (INFO_LAC2FUN_IN (arg_info), in_else);
        /* out = out_then u out_else */
        INFO_LAC2FUN_OUT (arg_info) = DFMGenMaskOr (out_then, out_else);
        /* local = (local_then u local_else) \ in */
        INFO_LAC2FUN_LOCAL (arg_info) = DFMGenMaskOr (local_then, local_else);
        DFMSetMaskMinus (INFO_LAC2FUN_LOCAL (arg_info), INFO_LAC2FUN_IN (arg_info));

        /*
         * traverse condition
         */
        COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);
        break;

    case N_while:
        /*
         * traverse body
         */
        WHILE_BODY (arg_node) = Trav (WHILE_BODY (arg_node), arg_info);

        /*
         * calculate new in-, out-, local-masks and traverse condition
         */
        /*
         * a = 1;
         * b = 1;
         * c = 1;
         * while (...) {  <--- 'a' is in- and out-var, 'b' is local- and out-var,
         *   ... a ...         'c' is in-var,          'd' is local-var.
         *   a = 2;
         *   b = 2;
         *   ... c ...
         *   c = 2;
         *   d = 1;
         * }
         * ... a ... b ...
         *
         * All vars that are out-vars of the loop body have to be arguments of the
         * loop-dummy-function as well. (Even if they are local-vars!) This is
         * important in the case that the loop body is not executed at all.
         *
         * in' = in u out' */
        DFMSetMaskOr (INFO_LAC2FUN_IN (arg_info), INFO_LAC2FUN_OUT (arg_info));
        /* local' = local \ in' */
        DFMSetMaskMinus (INFO_LAC2FUN_LOCAL (arg_info), INFO_LAC2FUN_IN (arg_info));

        /*
         * traverse condition
         */
        WHILE_COND (arg_node) = Trav (WHILE_COND (arg_node), arg_info);
        break;

    case N_do:
        /*
         * traverse condition
         */
        DO_COND (arg_node) = Trav (DO_COND (arg_node), arg_info);

        /*
         * traverse body
         */
        DO_BODY (arg_node) = Trav (DO_BODY (arg_node), arg_info);

        /*
         * calculate new in-, out-, local-masks and traverse condition
         */
        /* there is no need to adjust the in-, out-, local-masks ... */
        break;

    default:
        DBUG_ASSERT ((0), "Only conditionals or loops are transformed into functions!");
        break;
    }

    /*
     * store the infered in-, out-, local-masks and
     * detect whether the fixpoint-property is hold or not
     */
    if (type == N_cond) {
        /*
         * conditional: no fixpoint iteration needed!
         */
        UPDATE ((*in), (INFO_LAC2FUN_IN (arg_info)));
        UPDATE ((*out), (INFO_LAC2FUN_OUT (arg_info)));
        UPDATE ((*local), (INFO_LAC2FUN_LOCAL (arg_info)));
    } else {
        /*
         * loop
         */
        COMPARE_AND_UPDATE ((*in), (INFO_LAC2FUN_IN (arg_info)));
        COMPARE_AND_UPDATE ((*out), (INFO_LAC2FUN_OUT (arg_info)));
        COMPARE_AND_UPDATE ((*local), (INFO_LAC2FUN_LOCAL (arg_info)));
    }

    DBUG_PRINT ("LAC2FUN", ("signature of %s: ", NODE_TEXT (arg_node)));
    DBUG_EXECUTE ("LAC2FUN", fprintf (stderr, "    in-vars: ");
                  DFMPrintMask (stderr, "%s ", *in); fprintf (stderr, "\n    out-vars: ");
                  DFMPrintMask (outfile, "%s ", *out);
                  fprintf (stderr, "\n    local-vars: ");
                  DFMPrintMask (outfile, "%s ", *local); fprintf (stderr, "\n\n"););

    /*
     * restore old needed-mask
     */
    INFO_LAC2FUN_NEEDED (arg_info) = DFMRemoveMask (INFO_LAC2FUN_NEEDED (arg_info));
    INFO_LAC2FUN_NEEDED (arg_info) = old_needed;

    /*
     * restore old in-, out-, local-masks
     */
    INFO_LAC2FUN_IN (arg_info) = old_in;
    INFO_LAC2FUN_OUT (arg_info) = old_out;
    INFO_LAC2FUN_LOCAL (arg_info) = old_local;
    /*
     * The whole conditional/loop is replaced later on by a single function call
     *     ...out-vars... dummy-fun( ...in-vars... );
     * Therefore we must adjust the current in-, out-mask accordingly.
     */
    DefinedMask (*out, arg_info);
    UsedMask (*in, arg_info);

    /*
     * remove all the temporarily needed masks
     */
    if (type == N_cond) {
        tmp = DFMRemoveMask (tmp);
        in_then = DFMRemoveMask (in_then);
        out_then = DFMRemoveMask (out_then);
        local_then = DFMRemoveMask (local_then);
        in_else = DFMRemoveMask (in_else);
        out_else = DFMRemoveMask (out_else);
        local_else = DFMRemoveMask (local_else);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *L2F_INFERfundef( node *arg_node, node *arg_info)
 *
 * description:
 *   All DFM-masks needed during traversal of the body are build before
 *   and removed afterwards.
 *   The formal arguments are traversed to take reference parameters into
 *   account.
 *   The body is traversed until the signature of the contained conditions and
 *   loops remains unchanged (fixpoint iteration).
 *
 ******************************************************************************/

node *
L2F_INFERfundef (node *arg_node, node *arg_info)
{
#ifndef DBUG_OFF
    int cnt = 0;
#endif

    DBUG_ENTER ("L2F_INFERfundef");

    INFO_LAC2FUN_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        if (FUNDEF_DFM_BASE (arg_node) == NULL) {
            FUNDEF_DFM_BASE (arg_node)
              = DFMGenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));
        }

        INFO_LAC2FUN_IN (arg_info) = DFMGenMaskClear (FUNDEF_DFM_BASE (arg_node));
        INFO_LAC2FUN_OUT (arg_info) = DFMGenMaskClear (FUNDEF_DFM_BASE (arg_node));
        INFO_LAC2FUN_LOCAL (arg_info) = DFMGenMaskClear (FUNDEF_DFM_BASE (arg_node));
        INFO_LAC2FUN_NEEDED (arg_info) = DFMGenMaskClear (FUNDEF_DFM_BASE (arg_node));

        DBUG_PRINT ("LAC2FUN", ("Infering signatures of conditionals/loops in %s():\n",
                                FUNDEF_NAME (arg_node)));

        /*
         * search in formal args for reference parameters
         *  -> adjust INFO_LAC2FUN_IN accordingly (resolve the reference parameters)
         */
        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
        }

        do {
            DBUG_EXECUTE ("LAC2FUN", cnt++;);
            DBUG_PRINT ("LAC2FUN", ("fixpoint iteration --- loop %i", cnt));
            INFO_LAC2FUN_ISFIX (arg_info) = 1;

            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

            DFMSetMaskClear (INFO_LAC2FUN_IN (arg_info));
            DFMSetMaskClear (INFO_LAC2FUN_OUT (arg_info));
            DFMSetMaskClear (INFO_LAC2FUN_LOCAL (arg_info));
            DFMSetMaskClear (INFO_LAC2FUN_NEEDED (arg_info));
        } while (INFO_LAC2FUN_ISFIX (arg_info) != 1);

        DBUG_PRINT ("LAC2FUN",
                    ("%s() finished after %i iterations\n", FUNDEF_NAME (arg_node), cnt));

        INFO_LAC2FUN_IN (arg_info) = DFMRemoveMask (INFO_LAC2FUN_IN (arg_info));
        INFO_LAC2FUN_OUT (arg_info) = DFMRemoveMask (INFO_LAC2FUN_OUT (arg_info));
        INFO_LAC2FUN_LOCAL (arg_info) = DFMRemoveMask (INFO_LAC2FUN_LOCAL (arg_info));
        INFO_LAC2FUN_NEEDED (arg_info) = DFMRemoveMask (INFO_LAC2FUN_NEEDED (arg_info));
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *L2F_INFERarg( node *arg_node, node *arg_info)
 *
 * description:
 *   Searches for reference parameters and marks them in INFO_LAC2FUN_IN.
 *
 ******************************************************************************/

node *
L2F_INFERarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("L2F_INFERarg");

    if ((ARG_ATTRIB (arg_node) == ST_reference)
        || (ARG_ATTRIB (arg_node) == ST_readonly_reference)) {

        DBUG_PRINT ("LAC2FUN",
                    ("Reference parameter: .. %s( .. %s .. ) { .. }",
                     FUNDEF_NAME (INFO_LAC2FUN_FUNDEF (arg_info)), ARG_NAME (arg_node)));

        DFMSetMaskEntrySet (INFO_LAC2FUN_IN (arg_info), NULL, arg_node);
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *L2F_INFERassign( node *arg_node, node *arg_info)
 *
 * description:
 *   Bottom-up-traversal of the assignments.
 *
 ******************************************************************************/

node *
L2F_INFERassign (node *arg_node, node *arg_info)
{
    node *assign_next;

    DBUG_ENTER ("L2F_INFERassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        assign_next = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *L2F_INFERlet( node *arg_node, node *arg_info)
 *
 * description:
 *   Every left hand side variable is marked as 'defined' in the in-, out-,
 *   local-masks of the current block.
 *
 ******************************************************************************/

node *
L2F_INFERlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("L2F_INFERlet");

    DefinedIds (LET_IDS (arg_node), arg_info);

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *L2F_INFERap( node *arg_node, node *arg_info)
 *
 * description:
 *   Searches for reference parameters and marks them as 'defined vars'.
 *
 ******************************************************************************/

node *
L2F_INFERap (node *arg_node, node *arg_info)
{
    node *fundef_args, *ap_args;

    DBUG_ENTER ("L2F_INFERap");

    /*
     * search for reference parameters and mark them as 'defined vars'
     * (resolve them explicitly)
     */
    DBUG_ASSERT ((AP_FUNDEF (arg_node) != NULL),
                 "Application with missing pointer to fundef found!");
    /*
     * traverse the formal (fundef_args) and current (ap_args) parameters
     */
    fundef_args = FUNDEF_ARGS (AP_FUNDEF (arg_node));
    ap_args = AP_ARGS (arg_node);
    while (ap_args != NULL) {
        if ((ARG_ATTRIB (fundef_args) == ST_reference)
            || (ARG_ATTRIB (fundef_args) == ST_readonly_reference)) {
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (ap_args)) == N_id),
                         "Reference parameter must be a N_id node!");

            DBUG_PRINT ("LAC2FUN",
                        ("Reference parameter (in %s()):  %s( .. %s .. )",
                         FUNDEF_NAME (INFO_LAC2FUN_FUNDEF (arg_info)),
                         FUNDEF_NAME (AP_FUNDEF (arg_node)), EXPRS_EXPR (ap_args)));

            DefinedVar (ID_VARDEC (EXPRS_EXPR (ap_args)), INFO_LAC2FUN_NEEDED (arg_info),
                        INFO_LAC2FUN_IN (arg_info), INFO_LAC2FUN_OUT (arg_info),
                        INFO_LAC2FUN_LOCAL (arg_info));
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
 *   node *L2F_INFERid( node *arg_node, node *arg_info)
 *
 * description:
 *   Every right hand side variable is marked as 'used' in the in-, out-,
 *   local-masks of the current block.
 *
 ******************************************************************************/

node *
L2F_INFERid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("L2F_INFERid");

    UsedVar (ID_VARDEC (arg_node), INFO_LAC2FUN_IN (arg_info),
             INFO_LAC2FUN_LOCAL (arg_info));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *L2F_INFERwithid( node *arg_node, node *arg_info)
 *
 * description:
 *   Every index variable is marked as 'defined' in the in-, out-,
 *   local-masks of the current block.
 *
 ******************************************************************************/

node *
L2F_INFERwithid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("L2F_INFERwithid");

    DefinedIds (NWITHID_VEC (arg_node), arg_info);
    DefinedIds (NWITHID_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *L2F_INFERcode( node *arg_node, node *arg_info)
 *
 * description:
 *   In order to get a correct bottom-up-traversal, the code-expr must be
 *   traversed *before* the code-block!
 *
 ******************************************************************************/

node *
L2F_INFERcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("L2F_INFERcode");

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
 *   node *L2F_INFERwith( node *arg_node, node *arg_info)
 *
 * description:
 *   In order to infer the withid-ids as local-vars, withop and code must be
 *   traversed *before* the withid (contained in part)!
 *
 ******************************************************************************/

node *
L2F_INFERwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("L2F_INFERwith");

    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);
    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *L2F_INFERwith2( node *arg_node, node *arg_info)
 *
 * description:
 *   In order to infer the withid-ids as local-vars, the withop, segments and
 *   code must be traversed *before* the withid!
 *
 ******************************************************************************/

node *
L2F_INFERwith2 (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("L2F_INFERwith2");

    NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);
    NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);
    NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
    NWITH2_WITHID (arg_node) = Trav (NWITH2_WITHID (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *L2F_INFERcond( node *arg_node, node *arg_info)
 *
 * description:
 *   Inferes the in-, out-, local-parameters of the conditional.
 *
 ******************************************************************************/

node *
L2F_INFERcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("L2F_INFERcond");

    arg_node = InferMasks (&(COND_IN_MASK (arg_node)), &(COND_OUT_MASK (arg_node)),
                           &(COND_LOCAL_MASK (arg_node)), arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *L2F_INFERwhile( node *arg_node, node *arg_info)
 *
 * description:
 *   Inferes the in-, out-, local-parameters of the while-loop.
 *
 ******************************************************************************/

node *
L2F_INFERwhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("L2F_INFERwhile");

    arg_node = InferMasks (&(WHILE_IN_MASK (arg_node)), &(WHILE_OUT_MASK (arg_node)),
                           &(WHILE_LOCAL_MASK (arg_node)), arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *L2F_INFERdo( node *arg_node, node *arg_info)
 *
 * description:
 *   Inferes the in-, out-, local-parameters of the do-loop.
 *
 ******************************************************************************/

node *
L2F_INFERdo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("L2F_INFERdo");

    arg_node = InferMasks (&(DO_IN_MASK (arg_node)), &(DO_OUT_MASK (arg_node)),
                           &(DO_LOCAL_MASK (arg_node)), arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/*
 *
 * functions for the second traversal
 *
 */

/******************************************************************************
 *
 * function:
 *   char *GetDummyFunName( char *suffix)
 *
 * description:
 *   Creates a new name for a dummy function. 'suffix' should be one of the
 *   strings "Cond", "Do" or "While".
 *
 ******************************************************************************/

static char *
GetDummyFunName (char *suffix)
{
#define NAMLEN 100
    static int number = 0;
    static char funname[NAMLEN];

    DBUG_ENTER ("GetDummyFunName");

    DBUG_ASSERT (((strlen (suffix) + number / 10 + 4) <= NAMLEN),
                 "name of dummy function too long");
    sprintf (funname, "__%s%i", suffix, number);
    number++;

    DBUG_RETURN (funname);
}

/******************************************************************************
 *
 * function:
 *   node *MakeL2fFundef( char *funname, char *modname, status status,
 *                        node *instr, node *funcall_let,
 *                        DFMmask_t in, DFMmask_t out, DFMmask_t local)
 *
 * description:
 *   Creates the fundef-node of a dummy function.
 *
 ******************************************************************************/

static node *
MakeL2fFundef (char *funname, char *modname, statustype status, node *instr,
               node *funcall_let, DFMmask_t in, DFMmask_t out, DFMmask_t local)
{
    LUT_t lut;
    DFMmask_t tmp_mask;
    node *args, *vardecs, *ret, *fundef, *assigns, *new_body, *let, *tmp;

    DBUG_ENTER ("MakeL2fFundef");

    /*
     * Create a new LUT and store the old/new args and vardecs in it.
     * This is done to generate the right references in the function body.
     */
    lut = GenerateLUT ();
    args = DFM2Args (in, lut);
    tmp_mask = DFMGenMaskMinus (out, in);
    DFMSetMaskOr (tmp_mask, local);
    vardecs = DFM2Vardecs (tmp_mask, lut);
    tmp_mask = DFMRemoveMask (tmp_mask);

    /*
     * Convert parameters from call-by-reference into call-by-value
     *  because they have already been resolved!
     */
    tmp = args;
    while (tmp != NULL) {
        if ((ARG_ATTRIB (tmp) == ST_reference)
            || (ARG_ATTRIB (tmp) == ST_readonly_reference)) {
            ARG_ATTRIB (tmp) = ST_unique;

            DBUG_PRINT ("LAC2FUN", ("ATTRIB[ .. %s( .. %s .. ) { .. } ]: "
                                    " ST_..reference -> ST_unique",
                                    funname, ARG_NAME (tmp)));
        }
        tmp = ARG_NEXT (tmp);
    }

    ret = MakeAssign (MakeReturn (DFM2Exprs (out, lut)), NULL);

    /*
     * All return ids with attrib 'ST_was_reference' must have the status
     *  'ST_artificial'
     */
    tmp = RETURN_EXPRS (ASSIGN_INSTR (ret));
    while (tmp != NULL) {
        if (ID_ATTRIB (EXPRS_EXPR (tmp)) == ST_was_reference) {
            ID_ATTRIB (EXPRS_EXPR (tmp)) = ST_unique;
            ID_STATUS (EXPRS_EXPR (tmp)) = ST_artificial;

            DBUG_PRINT ("LAC2FUN", ("%s():  ATTRIB/STATUS[ return( %s) ] "
                                    " .. -> ST_unique/ST_artificial",
                                    funname, ID_NAME (EXPRS_EXPR (tmp))));
        }
        tmp = EXPRS_NEXT (tmp);
    }

    /*
     * All args with attrib 'ST_was_reference' which are no out-vars must have
     *  the attrib 'ST_unique' instead.
     *
     * Example:
     *
     *    IntStack fun( IntStack &stack)
     *    {
     *      new_stack = create_stack();
     *      _flat_3 = !( is_empty( stack));
     *      if (_flat_3) {
     *        push( new_stack, top( stack));
     *      }
     *      stack = create_stack();
     *      return (new_stack);
     *    }
     *
     * With resolved reference parameters:
     *
     *    IntStack, IntStack fun( IntStack stack)
     *    {
     *      new_stack = create_stack();
     *      _flat_3 = !( is_empty( stack));
     *      if (_flat_3) {
     *        new_stack = push( new_stack, top( stack));
     *      }
     *      stack = create_stack();
     *      return (stack, new_stack);
     *    }
     *
     * After L2F transformation:
     *
     *    IntStack __Cond1( IntStack new_stack, bool _flat_3, IntStack:IntStack stack)
     *    {
     *      if (_flat_3) {
     *        new_stack = push( new_stack, top( stack));
     *      }
     *      return (new_stack);         // 'stack' is not needed in outer block!!
     *    }
     *
     *    IntStack, IntStack fun( IntStack stack)
     *    {
     *      new_stack = create_stack();
     *      _flat_3 = !( is_empty( stack));
     *      new_stack = __Cond1( new_stack, _flat_3, stack);
     *      stack = create_stack();     // redefinition of 'stack'
     *      return (stack, new_stack);
     *    }
     *
     * Although 'stack' was marked as 'ST_was_reference' in function 'fun' that is
     * no longer true in the context of the dummy function '__Cond1' because 'stack'
     * is not an out-var of this function!!
     */
    tmp = args;
    while (tmp != NULL) {
        if (ARG_ATTRIB (tmp) == ST_was_reference) {
            /*
             * CAUTION: the arg-node is a new one not contained in the relevant
             * DFM-base! Therefore we must search for ARG_NAME instead of the pointer
             * itself!
             */
            if (!DFMTestMaskEntry (out, ARG_NAME (tmp), NULL)) {
                ARG_ATTRIB (tmp) = ST_unique;
            }

            DBUG_PRINT ("LAC2FUN", ("ATTRIB[ .. %s( .. %s ..) { .. } ]: "
                                    " ST_was_reference -> ST_unique",
                                    funname, ARG_NAME (tmp)));
        }
        tmp = ARG_NEXT (tmp);
    }

    fundef
      = MakeFundef (StringCopy (funname), StringCopy (modname), DFM2ReturnTypes (out),
                    args, NULL, /* the block is not complete yet */
                    NULL);
    FUNDEF_STATUS (fundef) = status;
    FUNDEF_RETURN (fundef) = ASSIGN_INSTR (ret);

    switch (status) {
    case ST_condfun:
        assigns = MakeAssign (DupTreeLUT (instr, lut), ret);
        break;

    case ST_whilefun:
        new_body = DupTreeLUT (WHILE_BODY (instr), lut);

        /*
         * append call of loop-dummy-function to body.
         */
        tmp = BLOCK_INSTR (new_body);
        if (tmp != NULL) {
            while (ASSIGN_NEXT (tmp) != NULL) {
                tmp = ASSIGN_NEXT (tmp);
            }
            let = DupTreeLUT (funcall_let, lut);
            AP_FUNDEF (LET_EXPR (let)) = fundef;
            ASSIGN_NEXT (tmp) = MakeAssign (let, NULL);
        }

        assigns = MakeAssign (MakeCond (DupTreeLUT (WHILE_COND (instr), lut), new_body,
                                        MakeBlock (MakeEmpty (), NULL)),
                              ret);
        break;

    case ST_dofun:
        assigns = DupTreeLUT (BLOCK_INSTR (WHILE_BODY (instr)), lut);

        /*
         * append conditional with call of loop-dummy-function to assignments.
         */
        tmp = assigns;
        if (tmp != NULL) {
            while (ASSIGN_NEXT (tmp) != NULL) {
                tmp = ASSIGN_NEXT (tmp);
            }
            let = DupTreeLUT (funcall_let, lut);
            AP_FUNDEF (LET_EXPR (let)) = fundef;
            ASSIGN_NEXT (tmp)
              = MakeAssign (MakeCond (DupTreeLUT (WHILE_COND (instr), lut),
                                      MakeBlock (MakeAssign (let, NULL), NULL),
                                      MakeBlock (MakeEmpty (), NULL)),
                            ret);
        }
        break;

    default:
        assigns = NULL;
        break;
    }
    DBUG_ASSERT ((assigns != NULL), "wrong status -> no assigns created");

    /*
     * now we can add the body to the fundef
     */
    FUNDEF_BODY (fundef) = MakeBlock (assigns, vardecs);

    lut = RemoveLUT (lut);

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   node *MakeL2fFunLet( char *funname,
 *                        DFMmask_t in, DFMmask_t out)
 *
 * description:
 *   Creates the let node containing the call of a dummy function.
 *
 ******************************************************************************/

static node *
MakeL2fFunLet (char *funname, char *modname, DFMmask_t in, DFMmask_t out)
{
    node *let;
    ids *tmp;

    DBUG_ENTER ("MakeL2fFunLet");

    let = MakeLet (MakeAp (StringCopy (funname), StringCopy (modname),
                           DFM2Exprs (in, NULL)),
                   DFM2Ids (out, NULL));

    /*
     * All left hand side ids with attrib 'ST_was_reference' must have the status
     *  'ST_artificial'
     */
    tmp = LET_IDS (let);
    while (tmp != NULL) {
        if (IDS_ATTRIB (tmp) == ST_was_reference) {
            IDS_ATTRIB (tmp) = ST_unique;
            IDS_STATUS (tmp) = ST_artificial;

            DBUG_PRINT ("LAC2FUN", ("ATTRIB/STATUS[ %s = %s( .. ) ] "
                                    " .. -> ST_unique/ST_artificial",
                                    IDS_NAME (tmp), funname));
        }
        tmp = IDS_NEXT (tmp);
    }

    DBUG_RETURN (let);
}

/******************************************************************************
 *
 * function:
 *   node *DoLifting( char *prefix, statustype status,
 *                    DFMmask_t in, DFMmask_t out, DFMmask_t local,
 *                    node *arg_node, node *arg_info)
 *
 * description:
 *   This function carries out the lifting of a conditional or loop.
 *
 ******************************************************************************/

static node *
DoLifting (char *prefix, statustype status, DFMmask_t in, DFMmask_t out, DFMmask_t local,
           node *arg_node, node *arg_info)
{
    char *funname, *modname;
    node *fundef, *let;

    DBUG_ENTER ("DoLifting");

    /*
     * build call of the new dummy function
     */
    funname = GetDummyFunName (prefix);
    modname = FUNDEF_MOD (INFO_LAC2FUN_FUNDEF (arg_info));
#ifndef MAIN_HAS_MODNAME
    if (modname == NULL) {
        /* okay, we are in the main() function ... */
        modname = MAIN_MOD_NAME;
    }
#endif
    DBUG_ASSERT ((modname != NULL), "modul name for LAC function is NULL!");
    let = MakeL2fFunLet (funname, modname, in, out);

    /*
     * build new dummy function
     */
    fundef = MakeL2fFundef (funname, modname, status, arg_node, let, in, out, local);

    /*
     * set back-references let <-> fundef
     */
    AP_FUNDEF (LET_EXPR (let)) = fundef;

    /*
     * insert new dummy function into INFO_LAC2FUN_FUNS
     */
    FUNDEF_NEXT (fundef) = INFO_LAC2FUN_FUNS (arg_info);
    INFO_LAC2FUN_FUNS (arg_info) = fundef;

    /*
     * replace the instruction by a call of the new dummy function
     */
    in = DFMRemoveMask (in);
    out = DFMRemoveMask (out);
    local = DFMRemoveMask (local);
    arg_node = FreeTree (arg_node);
    arg_node = let;
    INFO_LAC2FUN_ISTRANS (arg_info) = 1;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *L2F_LIFTfundef( node *arg_node, node *arg_info)
 *
 * description:
 *   All dummy fundefs created during traversal of the body are inserted
 *   into the AST.
 *
 ******************************************************************************/

node *
L2F_LIFTfundef (node *arg_node, node *arg_info)
{
    node *ret, *tmp;

    DBUG_ENTER ("L2F_LIFTfundef");

    INFO_LAC2FUN_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_LAC2FUN_FUNS (arg_info) = NULL;
        INFO_LAC2FUN_ISTRANS (arg_info) = 0;

        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

        /*
         * insert dummy fundefs into the AST
         */
        tmp = INFO_LAC2FUN_FUNS (arg_info);
        if (tmp != NULL) {
            while (FUNDEF_NEXT (tmp) != NULL) {
                tmp = FUNDEF_NEXT (tmp);
            }
            FUNDEF_NEXT (tmp) = arg_node;
            ret = INFO_LAC2FUN_FUNS (arg_info);
            INFO_LAC2FUN_FUNS (arg_info) = NULL;
        } else {
            ret = arg_node;
        }
    } else {
        ret = arg_node;
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * function:
 *   node *L2F_LIFTassign( node *arg_node, node *arg_info)
 *
 * description:
 *   Bottom-up-traversal of the assignments.
 *   INFO_LAC2FUN_ISTRANS indicates whether an assignment has been transformed
 *   into a function call or not. If (INFO_LAC2FUN_ISTRANS > 0) is hold, the
 *   assign-node has been modificated and must be correctly inserted into the
 *   AST.
 *
 ******************************************************************************/

node *
L2F_LIFTassign (node *arg_node, node *arg_info)
{
    node *assign_next;

    DBUG_ENTER ("L2F_LIFTassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        assign_next = Trav (ASSIGN_NEXT (arg_node), arg_info);
        if (INFO_LAC2FUN_ISTRANS (arg_info)) {
            ASSIGN_NEXT (assign_next) = ASSIGN_NEXT (ASSIGN_NEXT (arg_node));
            ASSIGN_NEXT (arg_node) = assign_next;
            INFO_LAC2FUN_ISTRANS (arg_info) = 0;
        }
    }

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *L2F_LIFTcond( node *arg_node, node *arg_info)
 *
 * description:
 *   Lifts the conditional and inserts an equivalent function call instead.
 *
 ******************************************************************************/

node *
L2F_LIFTcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("L2F_LIFTcond");

    COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);
    COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);

    arg_node
      = DoLifting ("Cond", ST_condfun, COND_IN_MASK (arg_node), COND_OUT_MASK (arg_node),
                   COND_LOCAL_MASK (arg_node), arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *L2F_LIFTwhile( node *arg_node, node *arg_info)
 *
 * description:
 *   Lifts the while-loop and inserts an equivalent function call instead.
 *
 ******************************************************************************/

node *
L2F_LIFTwhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("LAC2FUNwhile");

    WHILE_BODY (arg_node) = Trav (WHILE_BODY (arg_node), arg_info);

    arg_node = DoLifting ("While", ST_whilefun, WHILE_IN_MASK (arg_node),
                          WHILE_OUT_MASK (arg_node), WHILE_LOCAL_MASK (arg_node),
                          arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *L2F_LIFTdo( node *arg_node, node *arg_info)
 *
 * description:
 *   Lifts the do-loop and inserts an equivalent function call instead.
 *
 ******************************************************************************/

node *
L2F_LIFTdo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("L2F_LIFTdo");

    DO_BODY (arg_node) = Trav (DO_BODY (arg_node), arg_info);

    arg_node = DoLifting ("Do", ST_dofun, DO_IN_MASK (arg_node), DO_OUT_MASK (arg_node),
                          DO_LOCAL_MASK (arg_node), arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *Lac2Fun( node *syntax_tree)
 *
 * description:
 *   Converts all loops and conditions into (annotated) functions.
 *
 *   First traversal(s):
 *     Infers the in-, out- and local-masks of each conditional or loop
 *     (via fixpoint iteration).
 *   Second traversal:
 *     Lifts each conditional or loop.
 *
 ******************************************************************************/

node *
Lac2Fun (node *syntax_tree)
{
    node *info_node;

    DBUG_ENTER ("Lac2Fun");

    info_node = MakeInfo ();

    act_tab = l2f_infer_tab;
    syntax_tree = Trav (syntax_tree, info_node);

    act_tab = l2f_lift_tab;
    syntax_tree = Trav (syntax_tree, info_node);

    info_node = FreeNode (info_node);

    /*
     * cleanup declarations (remove unused vardecs, ...)
     */
    syntax_tree = CleanupDecls (syntax_tree);

    DBUG_RETURN (syntax_tree);
}
