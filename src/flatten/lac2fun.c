/*
 *
 * $Log$
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

#include "tree.h"
#include "traverse.h"
#include "free.h"
#include "dbug.h"
#include "DupTree.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"

#define MAIN_HAS_NO_MODNAME

/*
 *
 * functions for the first traversal(s) [fixpoint interation]
 *
 */

#define UPDATE(old, new)                                                                 \
    if (old != NULL) {                                                                   \
        old = DFMRemoveMask (old);                                                       \
    }                                                                                    \
    old = new;

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
 *   void DefinedIds( ids *ids_, node *arg_info)
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
        INFO_LAC2FUN_IN (arg_info)
          = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_LAC2FUN_FUNDEF (arg_info)));
    }
    if (*out != NULL) {
        INFO_LAC2FUN_OUT (arg_info) = DFMGenMaskCopy (*out);
    } else {
        INFO_LAC2FUN_OUT (arg_info)
          = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_LAC2FUN_FUNDEF (arg_info)));
    }
    INFO_LAC2FUN_LOCAL (arg_info)
      = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_LAC2FUN_FUNDEF (arg_info)));

    /*
     * traverse then/else-block and loop-block respectively
     */
    switch (type) {
    case N_cond:
        COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);

        in_then = INFO_LAC2FUN_IN (arg_info);
        out_then = INFO_LAC2FUN_OUT (arg_info);
        local_then = INFO_LAC2FUN_LOCAL (arg_info);

        /*
         * setup in-, out-, local-masks for else-block
         */
        INFO_LAC2FUN_IN (arg_info)
          = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_LAC2FUN_FUNDEF (arg_info)));
        INFO_LAC2FUN_OUT (arg_info)
          = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_LAC2FUN_FUNDEF (arg_info)));
        INFO_LAC2FUN_LOCAL (arg_info)
          = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_LAC2FUN_FUNDEF (arg_info)));

        COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);

        in_else = INFO_LAC2FUN_IN (arg_info);
        out_else = INFO_LAC2FUN_OUT (arg_info);
        local_else = INFO_LAC2FUN_LOCAL (arg_info);
        break;

    case N_while:
        WHILE_BODY (arg_node) = Trav (WHILE_BODY (arg_node), arg_info);
        break;

    case N_do:
        DO_BODY (arg_node) = Trav (DO_BODY (arg_node), arg_info);
        break;

    default:
        DBUG_ASSERT ((0), "Only conditionals or loops can be lifted!");
        break;
    }

    /*
     * restore old needed-mask
     */
    INFO_LAC2FUN_NEEDED (arg_info) = DFMRemoveMask (INFO_LAC2FUN_NEEDED (arg_info));
    INFO_LAC2FUN_NEEDED (arg_info) = old_needed;

    /*
     * calculate new in-, out-, local-masks and traverse condition
     */
    switch (type) {
    case N_cond:
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

    case N_do:;

        /*
         * traverse condition
         */
        DO_COND (arg_node) = Trav (DO_COND (arg_node), arg_info);
        break;

    default:
        DBUG_ASSERT ((0), "Only conditionals or loops can be lifted!");
        break;
    }

    /*
     * store the infered in-, out-, local-masks
     * detect whether fixpoint-property is hold or not
     */
    if (type == N_cond) {
        /* condition */
        UPDATE ((*in), (INFO_LAC2FUN_IN (arg_info)));
        UPDATE ((*out), (INFO_LAC2FUN_OUT (arg_info)));
        UPDATE ((*local), (INFO_LAC2FUN_LOCAL (arg_info)));
    } else {
        /* loop */
        COMPARE_AND_UPDATE ((*in), (INFO_LAC2FUN_IN (arg_info)));
        COMPARE_AND_UPDATE ((*out), (INFO_LAC2FUN_OUT (arg_info)));
        COMPARE_AND_UPDATE ((*local), (INFO_LAC2FUN_LOCAL (arg_info)));
    }

    /*
     * restore and update old in-, out-, local-masks
     */
    INFO_LAC2FUN_IN (arg_info) = old_in;
    INFO_LAC2FUN_OUT (arg_info) = old_out;
    INFO_LAC2FUN_LOCAL (arg_info) = old_local;
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
 *   The body is traversed until the signature of the contained conditions and
 *   loops remains unchanged (fixpoint iteration).
 *
 ******************************************************************************/

node *
L2F_INFERfundef (node *arg_node, node *arg_info)
{
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

        do {
            INFO_LAC2FUN_ISFIX (arg_info) = 1;

            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

            DFMSetMaskClear (INFO_LAC2FUN_IN (arg_info));
            DFMSetMaskClear (INFO_LAC2FUN_OUT (arg_info));
            DFMSetMaskClear (INFO_LAC2FUN_LOCAL (arg_info));
            DFMSetMaskClear (INFO_LAC2FUN_NEEDED (arg_info));
        } while (INFO_LAC2FUN_ISFIX (arg_info) != 1);

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
    ids *_ids;

    DBUG_ENTER ("L2F_INFERlet");

    _ids = LET_IDS (arg_node);
    DefinedIds (_ids, arg_info);

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

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
    ids *_ids;

    DBUG_ENTER ("L2F_INFERwithid");

    _ids = NWITHID_VEC (arg_node);
    DefinedIds (_ids, arg_info);
    _ids = NWITHID_IDS (arg_node);
    DefinedIds (_ids, arg_info);

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
 *   node *MakeDummyFundef( char *funname, char *modname, status status,
 *                          node *instr, node *funcall_let,
 *                          DFMmask_t in, DFMmask_t out, DFMmask_t local)
 *
 * description:
 *   Creates the fundef-node of a dummy function.
 *
 ******************************************************************************/

static node *
MakeDummyFundef (char *funname, char *modname, statustype status, node *instr,
                 node *funcall_let, DFMmask_t in, DFMmask_t out, DFMmask_t local)
{
    lut_t *lut;
    DFMmask_t tmp_mask;
    node *ret, *args, *vardecs, *fundef, *assigns, *new_body, *let, *tmp;

    DBUG_ENTER ("MakeDummyFundef");

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

    ret = MakeAssign (MakeReturn (DFM2Exprs (out, lut)), NULL);

    fundef = MakeFundef (StringCopy (funname), StringCopy (modname), DFM2Types (out),
                         args, NULL, /* the block is not complete yet */
                         NULL);
    FUNDEF_STATUS (fundef) = status;
    FUNDEF_RETURN (fundef) = ASSIGN_INSTR (ret);

    switch (status) {
    case ST_condfun:
        assigns = MakeAssign (DupTreeLUT (instr, NULL, lut), ret);
        break;

    case ST_whilefun:
        new_body = DupTreeLUT (WHILE_BODY (instr), NULL, lut);

        /*
         * append call of loop-dummy-function to body.
         */
        tmp = BLOCK_INSTR (new_body);
        if (tmp != NULL) {
            while (ASSIGN_NEXT (tmp) != NULL) {
                tmp = ASSIGN_NEXT (tmp);
            }
            let = DupTreeLUT (funcall_let, NULL, lut);
            AP_FUNDEF (LET_EXPR (let)) = fundef;
            ASSIGN_NEXT (tmp) = MakeAssign (let, NULL);
        }

        assigns = MakeAssign (MakeCond (DupTreeLUT (WHILE_COND (instr), NULL, lut),
                                        new_body, MakeBlock (MakeEmpty (), NULL)),
                              ret);
        break;

    case ST_dofun:
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

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   node *MakeDummyFunLet( char *funname,
 *                          DFMmask_t in, DFMmask_t out)
 *
 * description:
 *   Creates the let node containing the call of a dummy function.
 *
 ******************************************************************************/

static node *
MakeDummyFunLet (char *funname, char *modname, DFMmask_t in, DFMmask_t out)
{
    node *let;

    DBUG_ENTER ("MakeDummyFunLet");

    let = MakeLet (MakeAp (StringCopy (funname), StringCopy (modname),
                           DFM2Exprs (in, NULL)),
                   DFM2Ids (out, NULL));

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
#ifdef MAIN_HAS_NO_MODNAME
    if (modname == NULL) {
        /* okay, we are in the main() function ... */
        modname = "_MAIN";
    }
#endif
    DBUG_ASSERT ((modname != NULL), "modul name for LAC function is NULL!");
    let = MakeDummyFunLet (funname, modname, in, out);

    /*
     * build new dummy function
     */
    fundef = MakeDummyFundef (funname, modname, status, arg_node, let, in, out, local);

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

#if 1
    arg_node = DoLifting ("While", ST_whilefun, WHILE_IN_MASK (arg_node),
                          WHILE_OUT_MASK (arg_node), WHILE_LOCAL_MASK (arg_node),
                          arg_node, arg_info);
#endif

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

#if 0
  arg_node = DoLifting( "Do", ST_dofun,
                        DO_IN_MASK( arg_node), DO_OUT_MASK( arg_node),
                        DO_LOCAL_MASK( arg_node), arg_node, arg_info);
#endif

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

    DBUG_RETURN (syntax_tree);
}
