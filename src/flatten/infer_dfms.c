/*
 *
 * $Log$
 * Revision 1.1  2000/12/06 19:57:52  dkr
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   infer_dfm.c
 *
 * prefix: INFDFMS
 *
 * description:
 *
 *   This compiler module implements the conversion of conditionals and
 *   loops into their true functional representation.
 *
 * usage of arg_info (INFO_INFDFMS_...):
 *
 *   ...FUNDEF   pointer to the current fundef
 *
 *   ...NEEDED   DFmask: vars needed in outer blocks
 *   ...IN       DFmask: vars used before eventually defined (in the current block)
 *   ...OUT      DFmask: vars defined an needed in outer blocks
 *   ...LOCAL    DFmask: vars defined before eventually used
 *   (Note: Each var occuring in a block is either IN-var or (exclusive!) LOCAL-var)
 *
 *   ...ISFIX    flag: fixpoint reached?
 *
 *****************************************************************************/

#include "tree.h"
#include "traverse.h"
#include "free.h"
#include "dbug.h"
#include "DupTree.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"
#include "infer_dfms.h"

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
 * If they differ the flag INFO_INFDFMS_ISFIX is unset to indicate that
 * another iteration is needed (fixpoint not yet reached).
 */
#define COMPARE_AND_UPDATE(old, new)                                                     \
    if (old != NULL) {                                                                   \
        if (DFMTestMask (old) != DFMTest2Masks (old, new)) {                             \
            /* 'old' and 'new' differs */                                                \
            INFO_INFDFMS_ISFIX (arg_info) = 0;                                           \
        }                                                                                \
    } else {                                                                             \
        INFO_INFDFMS_ISFIX (arg_info) = 0;                                               \
    }                                                                                    \
    UPDATE (old, new);

/*
 * compound macro
 */
#define INFO_DFMBASE(arg_info) FUNDEF_DFM_BASE (INFO_INFDFMS_FUNDEF (arg_info))

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
        DefinedVar (IDS_VARDEC (_ids), INFO_INFDFMS_NEEDED (arg_info),
                    INFO_INFDFMS_IN (arg_info), INFO_INFDFMS_OUT (arg_info),
                    INFO_INFDFMS_LOCAL (arg_info));
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
        DefinedVar (decl, INFO_INFDFMS_NEEDED (arg_info), INFO_INFDFMS_IN (arg_info),
                    INFO_INFDFMS_OUT (arg_info), INFO_INFDFMS_LOCAL (arg_info));
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
        UsedVar (decl, INFO_INFDFMS_IN (arg_info), INFO_INFDFMS_LOCAL (arg_info));
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
 * remarks:
 *   The initializations of in_then, out_then, local_then, in_else, out_else,
 *   local_else, and tmp are required to avoid a compiler warning
 *   "...might be used uninitialized" only. However, they should do no harm
 *   either 8-) since type is assigned only once, and these vars are referred
 *   to iff (type == N_cond) holds.
 *
 ******************************************************************************/

static node *
InferMasks (DFMmask_t *in, DFMmask_t *out, DFMmask_t *local, node *arg_node,
            node *arg_info)
{
    DFMmask_t old_needed, old_in, old_out, old_local;
    DFMmask_t in_then = NULL, out_then = NULL, local_then = NULL;
    DFMmask_t in_else = NULL, out_else = NULL, local_else = NULL;
    DFMmask_t tmp = NULL;
    nodetype type = NODE_TYPE (arg_node);

    DBUG_ENTER ("InferMasks");

    /*
     * save old masks
     */
    old_needed = INFO_INFDFMS_NEEDED (arg_info);
    old_in = INFO_INFDFMS_IN (arg_info);
    old_out = INFO_INFDFMS_OUT (arg_info);
    old_local = INFO_INFDFMS_LOCAL (arg_info);

    /*
     * setup needed-masks for then/else-block and loop-block respectively
     */
    INFO_INFDFMS_NEEDED (arg_info) = DFMGenMaskCopy (old_needed);
    AdjustNeeded (INFO_INFDFMS_NEEDED (arg_info), old_in, old_out);

    /*
     * setup in-, out-, local-masks for then-block and loop-block respectively
     */
    if (*in != NULL) {
        INFO_INFDFMS_IN (arg_info) = DFMGenMaskCopy (*in);
    } else {
        INFO_INFDFMS_IN (arg_info) = DFMGenMaskClear (INFO_DFMBASE (arg_info));
    }
    if (*out != NULL) {
        INFO_INFDFMS_OUT (arg_info) = DFMGenMaskCopy (*out);
    } else {
        INFO_INFDFMS_OUT (arg_info) = DFMGenMaskClear (INFO_DFMBASE (arg_info));
    }
    INFO_INFDFMS_LOCAL (arg_info) = DFMGenMaskClear (INFO_DFMBASE (arg_info));

    switch (type) {
    case N_cond:
        /*
         * traverse then-block
         */
        COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);

        in_then = INFO_INFDFMS_IN (arg_info);
        out_then = INFO_INFDFMS_OUT (arg_info);
        local_then = INFO_INFDFMS_LOCAL (arg_info);

        /*
         * setup in-, out-, local-masks for else-block
         */
        INFO_INFDFMS_IN (arg_info) = DFMGenMaskClear (INFO_DFMBASE (arg_info));
        INFO_INFDFMS_OUT (arg_info) = DFMGenMaskClear (INFO_DFMBASE (arg_info));
        INFO_INFDFMS_LOCAL (arg_info) = DFMGenMaskClear (INFO_DFMBASE (arg_info));

        COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);

        in_else = INFO_INFDFMS_IN (arg_info);
        out_else = INFO_INFDFMS_OUT (arg_info);
        local_else = INFO_INFDFMS_LOCAL (arg_info);

        /*
         * calculate new in-, out-, local-masks and traverse condition
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
        DFMSetMaskOr (INFO_INFDFMS_IN (arg_info), INFO_INFDFMS_OUT (arg_info));
        /* local' = local \ in' */
        DFMSetMaskMinus (INFO_INFDFMS_LOCAL (arg_info), INFO_INFDFMS_IN (arg_info));

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
        UPDATE ((*in), (INFO_INFDFMS_IN (arg_info)));
        UPDATE ((*out), (INFO_INFDFMS_OUT (arg_info)));
        UPDATE ((*local), (INFO_INFDFMS_LOCAL (arg_info)));
    } else {
        /*
         * loop
         */
        COMPARE_AND_UPDATE ((*in), (INFO_INFDFMS_IN (arg_info)));
        COMPARE_AND_UPDATE ((*out), (INFO_INFDFMS_OUT (arg_info)));
        COMPARE_AND_UPDATE ((*local), (INFO_INFDFMS_LOCAL (arg_info)));
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
    INFO_INFDFMS_NEEDED (arg_info) = DFMRemoveMask (INFO_INFDFMS_NEEDED (arg_info));
    INFO_INFDFMS_NEEDED (arg_info) = old_needed;

    /*
     * restore old in-, out-, local-masks
     */
    INFO_INFDFMS_IN (arg_info) = old_in;
    INFO_INFDFMS_OUT (arg_info) = old_out;
    INFO_INFDFMS_LOCAL (arg_info) = old_local;
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
 *   node *INFDFMSfundef( node *arg_node, node *arg_info)
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
INFDFMSfundef (node *arg_node, node *arg_info)
{
#ifndef DBUG_OFF
    int cnt = 0;
#endif

    DBUG_ENTER ("INFDFMSfundef");

    INFO_INFDFMS_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        if (FUNDEF_DFM_BASE (arg_node) == NULL) {
            FUNDEF_DFM_BASE (arg_node)
              = DFMGenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));
        }

        INFO_INFDFMS_IN (arg_info) = DFMGenMaskClear (FUNDEF_DFM_BASE (arg_node));
        INFO_INFDFMS_OUT (arg_info) = DFMGenMaskClear (FUNDEF_DFM_BASE (arg_node));
        INFO_INFDFMS_LOCAL (arg_info) = DFMGenMaskClear (FUNDEF_DFM_BASE (arg_node));
        INFO_INFDFMS_NEEDED (arg_info) = DFMGenMaskClear (FUNDEF_DFM_BASE (arg_node));

        DBUG_PRINT ("LAC2FUN", ("Infering signatures of conditionals/loops in %s():\n",
                                FUNDEF_NAME (arg_node)));

        /*
         * search in formal args for reference parameters
         *  -> adjust INFO_INFDFMS_IN accordingly (resolve the reference parameters)
         */
        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
        }

        do {
            DBUG_EXECUTE ("LAC2FUN", cnt++;);
            DBUG_PRINT ("LAC2FUN", ("fixpoint iteration --- loop %i", cnt));
            INFO_INFDFMS_ISFIX (arg_info) = 1;

            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

            DFMSetMaskClear (INFO_INFDFMS_IN (arg_info));
            DFMSetMaskClear (INFO_INFDFMS_OUT (arg_info));
            DFMSetMaskClear (INFO_INFDFMS_LOCAL (arg_info));
            DFMSetMaskClear (INFO_INFDFMS_NEEDED (arg_info));
        } while (INFO_INFDFMS_ISFIX (arg_info) != 1);

        DBUG_PRINT ("LAC2FUN",
                    ("%s() finished after %i iterations\n", FUNDEF_NAME (arg_node), cnt));

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
 *   node *INFDFMSarg( node *arg_node, node *arg_info)
 *
 * description:
 *   Searches for reference parameters and marks them in INFO_INFDFMS_IN.
 *
 ******************************************************************************/

node *
INFDFMSarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("INFDFMSarg");

    if ((ARG_ATTRIB (arg_node) == ST_reference)
        || (ARG_ATTRIB (arg_node) == ST_readonly_reference)) {

        DBUG_PRINT ("LAC2FUN",
                    ("Reference parameter: .. %s( .. %s .. ) { .. }",
                     FUNDEF_NAME (INFO_INFDFMS_FUNDEF (arg_info)), ARG_NAME (arg_node)));

        DFMSetMaskEntrySet (INFO_INFDFMS_IN (arg_info), NULL, arg_node);
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INFDFMSassign( node *arg_node, node *arg_info)
 *
 * description:
 *   Bottom-up-traversal of the assignments.
 *
 ******************************************************************************/

node *
INFDFMSassign (node *arg_node, node *arg_info)
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
 *   node *INFDFMSlet( node *arg_node, node *arg_info)
 *
 * description:
 *   Every left hand side variable is marked as 'defined' in the in-, out-,
 *   local-masks of the current block.
 *
 ******************************************************************************/

node *
INFDFMSlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("INFDFMSlet");

    DefinedIds (LET_IDS (arg_node), arg_info);

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INFDFMSap( node *arg_node, node *arg_info)
 *
 * description:
 *   Searches for reference parameters and marks them as 'defined vars'.
 *
 ******************************************************************************/

node *
INFDFMSap (node *arg_node, node *arg_info)
{
    node *fundef_args, *ap_args;

    DBUG_ENTER ("INFDFMSap");

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
                         FUNDEF_NAME (INFO_INFDFMS_FUNDEF (arg_info)),
                         FUNDEF_NAME (AP_FUNDEF (arg_node)), EXPRS_EXPR (ap_args)));

            DefinedVar (ID_VARDEC (EXPRS_EXPR (ap_args)), INFO_INFDFMS_NEEDED (arg_info),
                        INFO_INFDFMS_IN (arg_info), INFO_INFDFMS_OUT (arg_info),
                        INFO_INFDFMS_LOCAL (arg_info));
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
 *   node *INFDFMSid( node *arg_node, node *arg_info)
 *
 * description:
 *   Every right hand side variable is marked as 'used' in the in-, out-,
 *   local-masks of the current block.
 *
 ******************************************************************************/

node *
INFDFMSid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("INFDFMSid");

    UsedVar (ID_VARDEC (arg_node), INFO_INFDFMS_IN (arg_info),
             INFO_INFDFMS_LOCAL (arg_info));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INFDFMSwithid( node *arg_node, node *arg_info)
 *
 * description:
 *   Every index variable is marked as 'defined' in the in-, out-,
 *   local-masks of the current block.
 *
 ******************************************************************************/

node *
INFDFMSwithid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("INFDFMSwithid");

    DefinedIds (NWITHID_VEC (arg_node), arg_info);
    DefinedIds (NWITHID_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INFDFMScode( node *arg_node, node *arg_info)
 *
 * description:
 *   In order to get a correct bottom-up-traversal, the code-expr must be
 *   traversed *before* the code-block!
 *
 ******************************************************************************/

node *
INFDFMScode (node *arg_node, node *arg_info)
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
 *   node *INFDFMSwith( node *arg_node, node *arg_info)
 *
 * description:
 *   In order to infer the withid-ids as local-vars, withop and code must be
 *   traversed *before* the withid (contained in part)!
 *
 ******************************************************************************/

node *
INFDFMSwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("INFDFMSwith");

    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);
    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INFDFMSwith2( node *arg_node, node *arg_info)
 *
 * description:
 *   In order to infer the withid-ids as local-vars, the withop, segments and
 *   code must be traversed *before* the withid!
 *
 ******************************************************************************/

node *
INFDFMSwith2 (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("INFDFMSwith2");

    NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);
    NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);

    if (NWITH2_CODE (arg_node) != NULL) {
        NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
    }

    NWITH2_WITHID (arg_node) = Trav (NWITH2_WITHID (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INFDFMScond( node *arg_node, node *arg_info)
 *
 * description:
 *   Inferes the in-, out-, local-parameters of the conditional.
 *
 ******************************************************************************/

node *
INFDFMScond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("INFDFMScond");

    arg_node = InferMasks (&(COND_IN_MASK (arg_node)), &(COND_OUT_MASK (arg_node)),
                           &(COND_LOCAL_MASK (arg_node)), arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INFDFMSwhile( node *arg_node, node *arg_info)
 *
 * description:
 *   Inferes the in-, out-, local-parameters of the while-loop.
 *
 ******************************************************************************/

node *
INFDFMSwhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("INFDFMSwhile");

    arg_node = InferMasks (&(WHILE_IN_MASK (arg_node)), &(WHILE_OUT_MASK (arg_node)),
                           &(WHILE_LOCAL_MASK (arg_node)), arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INFDFMSdo( node *arg_node, node *arg_info)
 *
 * description:
 *   Inferes the in-, out-, local-parameters of the do-loop.
 *
 ******************************************************************************/

node *
INFDFMSdo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("INFDFMSdo");

    arg_node = InferMasks (&(DO_IN_MASK (arg_node)), &(DO_OUT_MASK (arg_node)),
                           &(DO_LOCAL_MASK (arg_node)), arg_node, arg_info);

    DBUG_RETURN (arg_node);
}
