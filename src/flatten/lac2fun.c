/*
 *
 * $Log$
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

#define DEFINED_VARS(ids, arg_info)                                                      \
    while (ids != NULL) {                                                                \
        DefinedVar (NULL, IDS_VARDEC (ids), INFO_LAC2FUN_NEEDED (arg_info),              \
                    &(INFO_LAC2FUN_IN (arg_info)), &(INFO_LAC2FUN_OUT (arg_info)),       \
                    &(INFO_LAC2FUN_LOCAL (arg_info)));                                   \
        ids = IDS_NEXT (ids);                                                            \
    }

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
 *   void DefinedVar( char *id, node *decl,
 *                    DFMmask_t needed,
 *                    DFMmask_t *in, DFMmask_t *out, DFMmask_t *local)
 *
 * description:
 *   Updates the given in-, out-, local-masks according to a defining occurence
 *   of a variable. The variable can either be specified directly (id) or as
 *   a pointer to the declaration (decl). The unused parameter has to be NULL.
 *
 ******************************************************************************/

static void
DefinedVar (char *id, node *decl, DFMmask_t needed, DFMmask_t *in, DFMmask_t *out,
            DFMmask_t *local)
{
    DBUG_ENTER ("DefinedVar");

    if ((NODE_TYPE (decl) == N_vardec) || (NODE_TYPE (decl) == N_arg)) {
        DFMSetMaskEntryClear (*in, id, decl);
        if (DFMTestMaskEntry (needed, id, decl)) {
            DFMSetMaskEntrySet (*out, id, decl);
        }
        DFMSetMaskEntrySet (*local, id, decl);
    } else {
        DBUG_ASSERT ((NODE_TYPE (decl) == N_objdef),
                     "declaration is neither a N_arg/N_vardec-node nor a N_objdef-node");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void UsedVar( char *id, node *decl,
 *                 DFMmask_t *in, DFMmask_t *out, DFMmask_t *local)
 *
 * description:
 *   Updates the given in-, out-, local-masks according to a non-defining
 *   occurence of a variable. The variable can either be specified directly (id)
 *   or as a pointer to the declaration (decl). The unused parameter has to be
 *   NULL.
 *
 ******************************************************************************/

static void
UsedVar (char *id, node *decl, DFMmask_t *in, DFMmask_t *local)
{
    DBUG_ENTER ("UsedVar");

    if ((NODE_TYPE (decl) == N_vardec) || (NODE_TYPE (decl) == N_arg)) {
        DFMSetMaskEntrySet (*in, id, decl);
        DFMSetMaskEntryClear (*local, id, decl);
    } else {
        DBUG_ASSERT ((NODE_TYPE (decl) == N_objdef),
                     "declaration is neither a N_arg/N_vardec-node nor a N_objdef-node");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void AdjustNeeded( DFMmask_t *needed,
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
AdjustNeeded (DFMmask_t *needed, DFMmask_t in, DFMmask_t out)
{
    DBUG_ENTER ("AdjustNeeded");

    DFMSetMaskMinus (needed, out);
    DFMSetMaskOr (needed, in);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   node *DoLifting( node *arg_node, node *arg_info)
 *
 * description:
 *   This function carries out the lifting of a conditional or loop
 *   (N_cond, N_while, or N_do).
 *
 ******************************************************************************/

static node *
DoLifting (node *arg_node, node *arg_info)
{
    DFMmask_t old_needed, old_in, old_out, old_local;
    DFMmask_t in_then, out_then, local_then;
    DFMmask_t in_else, out_else, local_else;
    DFMmask_t tmp;
    char *funname, *modname;
    node *fundef, *let;
    statustype status;
    nodetype type = NODE_TYPE (arg_node);

    DBUG_ENTER ("DoLifting");

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
    INFO_LAC2FUN_IN (arg_info) = DFMGenMaskClear (INFO_LAC2FUN_DFMBASE (arg_info));
    INFO_LAC2FUN_OUT (arg_info) = DFMGenMaskClear (INFO_LAC2FUN_DFMBASE (arg_info));
    INFO_LAC2FUN_LOCAL (arg_info) = DFMGenMaskClear (INFO_LAC2FUN_DFMBASE (arg_info));

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
        INFO_LAC2FUN_IN (arg_info) = DFMGenMaskClear (INFO_LAC2FUN_DFMBASE (arg_info));
        INFO_LAC2FUN_OUT (arg_info) = DFMGenMaskClear (INFO_LAC2FUN_DFMBASE (arg_info));
        INFO_LAC2FUN_LOCAL (arg_info) = DFMGenMaskClear (INFO_LAC2FUN_DFMBASE (arg_info));

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
         * >> CAUTION <<
         * In case of nested loops some vars might be out-vars although they are
         * infered as local-vars of the loop body only:
         *
         *   a = 1;
         *   while (...) {
         *     ... a ...
         *     a = 2;
         *     while (...) {  <--- 'a' is infered as local- but *not* as out-var :-(
         *       a = 3;
         *     }
         *     <--- here is in fact an implicit 'return( a)' for the outer loop.
         *   }
         *
         * The easy solution for this problem: All local-vars are assumed to be
         * out-vars as well. In a following step we shall remove all superfluous
         * parameters of the loop-dummy-function later on.
         *
         * out' = out u local */
        DFMSetMaskOr (INFO_LAC2FUN_OUT (arg_info), INFO_LAC2FUN_LOCAL (arg_info));
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
     * build call of the new dummy function
     */
    switch (type) {
    case N_cond:
        funname = GetDummyFunName ("Cond");
        break;
    case N_while:
        funname = GetDummyFunName ("While");
        break;
    case N_do:
        funname = GetDummyFunName ("Do");
        break;
    default:
        DBUG_ASSERT ((0), "Only conditionals or loops can be lifted!");
        break;
    }
    modname = FUNDEF_MOD (INFO_LAC2FUN_FUNDEF (arg_info));
#ifdef MAIN_HAS_NO_MODNAME
    if (modname == NULL) {
        /* okay, we are in the main() function ... */
        modname = "_MAIN";
    }
#endif
    DBUG_ASSERT ((modname != NULL), "modul name for LAC function is NULL!");
    let = MakeDummyFunLet (funname, modname, INFO_LAC2FUN_IN (arg_info),
                           INFO_LAC2FUN_OUT (arg_info));

    /*
     * build new dummy function
     */
    switch (type) {
    case N_cond:
        status = ST_condfun;
        break;
    case N_while:
        status = ST_whilefun;
        break;
    case N_do:
        status = ST_dofun;
        break;
    default:
        DBUG_ASSERT ((0), "Only conditionals or loops can be lifted!");
        break;
    }
    fundef = MakeDummyFundef (funname, modname, status, arg_node, let,
                              INFO_LAC2FUN_IN (arg_info), INFO_LAC2FUN_OUT (arg_info),
                              INFO_LAC2FUN_LOCAL (arg_info));

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
    arg_node = FreeTree (arg_node);
    arg_node = let;
    INFO_LAC2FUN_ISTRANS (arg_info) = 1;

    /*
     * restore old in-, out-, local-masks
     * traverse new let-assignment
     */
    INFO_LAC2FUN_IN (arg_info) = DFMRemoveMask (INFO_LAC2FUN_IN (arg_info));
    INFO_LAC2FUN_IN (arg_info) = old_in;
    INFO_LAC2FUN_OUT (arg_info) = DFMRemoveMask (INFO_LAC2FUN_OUT (arg_info));
    INFO_LAC2FUN_OUT (arg_info) = old_out;
    INFO_LAC2FUN_LOCAL (arg_info) = DFMRemoveMask (INFO_LAC2FUN_LOCAL (arg_info));
    INFO_LAC2FUN_LOCAL (arg_info) = old_local;
    arg_node = Trav (arg_node, arg_info);

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
 *   node *LAC2FUNfundef( node *arg_node, node *arg_info)
 *
 * description:
 *   All DFM-masks needed during traversal of the fundef are build before
 *   and removed afterwards.
 *
 ******************************************************************************/

node *
LAC2FUNfundef (node *arg_node, node *arg_info)
{
    node *ret, *tmp;

    DBUG_ENTER ("LAC2FUNfundef");

    INFO_LAC2FUN_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_LAC2FUN_FUNS (arg_info) = NULL;

        INFO_LAC2FUN_DFMBASE (arg_info)
          = DFMGenMaskBase (FUNDEF_ARGS (arg_node), (FUNDEF_BODY (arg_node) != NULL)
                                                      ? FUNDEF_VARDEC (arg_node)
                                                      : NULL);

        INFO_LAC2FUN_IN (arg_info) = DFMGenMaskClear (INFO_LAC2FUN_DFMBASE (arg_info));
        INFO_LAC2FUN_OUT (arg_info) = DFMGenMaskClear (INFO_LAC2FUN_DFMBASE (arg_info));
        INFO_LAC2FUN_LOCAL (arg_info) = DFMGenMaskClear (INFO_LAC2FUN_DFMBASE (arg_info));
        INFO_LAC2FUN_NEEDED (arg_info)
          = DFMGenMaskClear (INFO_LAC2FUN_DFMBASE (arg_info));
        INFO_LAC2FUN_ISTRANS (arg_info) = 0;

        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

        INFO_LAC2FUN_IN (arg_info) = DFMRemoveMask (INFO_LAC2FUN_IN (arg_info));
        INFO_LAC2FUN_OUT (arg_info) = DFMRemoveMask (INFO_LAC2FUN_OUT (arg_info));
        INFO_LAC2FUN_LOCAL (arg_info) = DFMRemoveMask (INFO_LAC2FUN_LOCAL (arg_info));
        INFO_LAC2FUN_NEEDED (arg_info) = DFMRemoveMask (INFO_LAC2FUN_NEEDED (arg_info));

        INFO_LAC2FUN_DFMBASE (arg_info)
          = DFMRemoveMaskBase (INFO_LAC2FUN_DFMBASE (arg_info));

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
 *   node *LAC2FUNassign( node *arg_node, node *arg_info)
 *
 * description:
 *   Bottom-up-traversal of the assignments.
 *   INFO_LAC2FUN_ISTRANS indicates whether an assignment has been transformed
 *   into a function call or not.
 *   If (INFO_LAC2FUN_ISTRANS > 0) is hold, the assign-node has been modificated
 *   and must be correctly inserted into the AST.
 *
 ******************************************************************************/

node *
LAC2FUNassign (node *arg_node, node *arg_info)
{
    node *assign_next;

    DBUG_ENTER ("LAC2FUNassign");

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
 *   node *LAC2FUNlet( node *arg_node, node *arg_info)
 *
 * description:
 *   Every left hand side variable is marked as 'defined' in the in-, out-,
 *   local-masks of the current block.
 *
 ******************************************************************************/

node *
LAC2FUNlet (node *arg_node, node *arg_info)
{
    ids *_ids;

    DBUG_ENTER ("LAC2FUNlet");

    _ids = LET_IDS (arg_node);
    DEFINED_VARS (_ids, arg_info);

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *LAC2FUNid( node *arg_node, node *arg_info)
 *
 * description:
 *   Every right hand side variable is marked as 'used' in the in-, out-,
 *   local-masks of the current block.
 *
 ******************************************************************************/

node *
LAC2FUNid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("LAC2FUNid");

    UsedVar (NULL, ID_VARDEC (arg_node), &(INFO_LAC2FUN_IN (arg_info)),
             &(INFO_LAC2FUN_LOCAL (arg_info)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *LAC2FUNwithid( node *arg_node, node *arg_info)
 *
 * description:
 *   Every index variable is marked as 'defined' in the in-, out-,
 *   local-masks of the current block.
 *
 ******************************************************************************/

node *
LAC2FUNwithid (node *arg_node, node *arg_info)
{
    ids *_ids;

    DBUG_ENTER ("LAC2FUNwithid");

    _ids = NWITHID_VEC (arg_node);
    DEFINED_VARS (_ids, arg_info);
    _ids = NWITHID_IDS (arg_node);
    DEFINED_VARS (_ids, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *LAC2FUNcode( node *arg_node, node *arg_info)
 *
 * description:
 *   In order to get a correct bottom-up-traversal, the code-expr must be
 *   traversed *before* the code-block!
 *
 ******************************************************************************/

node *
LAC2FUNcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("LAC2FUNcode");

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
 *   node *LAC2FUNwith( node *arg_node, node *arg_info)
 *
 * description:
 *   In order to infer the withid-ids as local-vars, withop and code must be
 *   traversed *before* the withid (contained in part)!
 *
 ******************************************************************************/

node *
LAC2FUNwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("LAC2FUNwith");

    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);
    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *LAC2FUNwith2( node *arg_node, node *arg_info)
 *
 * description:
 *   In order to infer the withid-ids as local-vars, the withop, segments and
 *   code must be traversed *before* the withid!
 *
 ******************************************************************************/

node *
LAC2FUNwith2 (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("LAC2FUNwith2");

    NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);
    NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);
    NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
    NWITH2_WITHID (arg_node) = Trav (NWITH2_WITHID (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *LAC2FUNcond( node *arg_node, node *arg_info)
 *
 * description:
 *   Inferes the in-, out-, local-parameters of the conditional, lifts the
 *   conditional and inserts an equivalent function call instead.
 *
 ******************************************************************************/

node *
LAC2FUNcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("LAC2FUNcond");

    arg_node = DoLifting (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *LAC2FUNwhile( node *arg_node, node *arg_info)
 *
 * description:
 *   Inferes the in-, out-, local-parameters of the while-loop, lifts the loop
 *   and inserts an equivalent function call instead.
 *
 ******************************************************************************/

node *
LAC2FUNwhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("LAC2FUNwhile");

    arg_node = DoLifting (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *LAC2FUNdo( node *arg_node, node *arg_info)
 *
 * description:
 *   Inferes the in-, out-, local-parameters of the do-loop, lifts the loop and
 *   inserts an equivalent function call instead.
 *
 ******************************************************************************/

node *
LAC2FUNdo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("LAC2FUNdo");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *LaC2Fun( node *syntax_tree)
 *
 * description:
 *   Converts all loops and conditions into (annotated) functions.
 *
 ******************************************************************************/

node *
LaC2Fun (node *syntax_tree)
{
    node *info_node;

    DBUG_ENTER ("LaC2Fun");

    act_tab = lac2fun_tab;
    info_node = MakeInfo ();
    syntax_tree = Trav (syntax_tree, info_node);

    DBUG_RETURN (syntax_tree);
}
