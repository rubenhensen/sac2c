/*
 *
 * $Log$
 * Revision 3.4  2001/04/02 15:24:30  dkr
 * macros FUNDEF_IS_... used
 *
 * Revision 3.3  2001/03/22 19:20:54  dkr
 * include of tree.h eliminated
 *
 * Revision 3.2  2001/03/21 14:03:41  nmw
 * when used in ssa-form return renamings are moved out of
 * do/while loops in the calling context
 * THIS HAS NOT BEEN TESTED !!!
 *
 * Revision 3.1  2000/11/20 17:59:21  sacbase
 * new release made
 *
 * Revision 1.4  2000/11/14 13:19:01  dkr
 * no '... might be used uninitialized' warnings anymore
 *
 * Revision 1.3  2000/02/18 14:03:55  cg
 * Added reconversion of while- and do-loops. Do-loops are yet untested
 * due to missing corresponding capability of lac2fun.c.
 *
 * Revision 1.2  2000/02/18 10:49:47  cg
 * All initial bugs fixed; this version successfully reconverts
 * conditionals.
 *
 * Revision 1.1  2000/02/17 16:15:25  cg
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   fun2lac.c
 *
 * prefix: FUN2LAC
 *
 * description:
 *   This compiler module implements the reconversion of conditionals and
 *   loops from their true functional representation generated by routines
 *   found in lac2fun.c into an explicit representation suitable for code
 *   generation.
 *
 *****************************************************************************/

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "traverse.h"
#include "free.h"
#include "dbug.h"
#include "print.h"
#include "Inline.h"

/******************************************************************************
 *
 * function:
 *   int IsRecursiveCall( node *assign, node *fundef)
 *
 * description:
 *
 *
 ******************************************************************************/

static int
IsRecursiveCall (node *assign, node *fundef)
{
    int res;
    node *instr, *expr;

    DBUG_ENTER ("IsRecursiveCall");

    DBUG_ASSERT ((NODE_TYPE (assign) == N_assign),
                 "Wrong 1st argument to IsRecursiveCall().");
    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "Wrong 2nd argument to IsRecursiveCall().");

    instr = ASSIGN_INSTR (assign);

    if (NODE_TYPE (instr) == N_let) {
        expr = LET_EXPR (instr);

        if ((NODE_TYPE (expr) == N_ap) && (AP_FUNDEF (expr) == fundef)) {
            res = 1;
        } else {
            res = 0;
        }
    } else {
        res = 0;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *ReplaceAssignmentByWhileLoop( node *assign,
 *                                       node *fundef, node *inl_code)
 *
 * description:
 *   This function replaces the given assignment containing the application
 *   of a while-loop function by the (transformed) inlined body of the
 *   corresponding function definition.
 *
 *   Basically the following transformation is performed:
 *
 *     if (<pred>) {
 *       <statements>
 *       <pre-renamings>
 *       ... = WhileLoopFun(...);
 *       <post-renamings>
 *     }
 *     else {
 *     }
 *     <return-renamings>
 *
 *   is being transformed into:
 *
 *     while (<pred>) {
 *       <statements>
 *       <pre-renamings>
 *     }
 *     <post-renamings>
 *     <return-renamings>
 *
 ******************************************************************************/

static node *
ReplaceAssignmentByWhileLoop (node *assign, node *fundef, node *inl_code)
{
    node *cond_assign, *cond;
    node *loop_body, *loop_suffix, *loop_pred;
    node *int_call;
    node *new_assign;
    node *tmp;

    DBUG_ENTER ("ReplaceAssignmentByWhileLoop");

    cond_assign = inl_code;
    inl_code = NULL;
    cond = ASSIGN_INSTR (cond_assign);

    /*
     * cond_assign: if (<pred>) { ... } else { } <return-renamings>
     * cond:        if (<pred>) { ... } else { }
     */

    DBUG_ASSERT ((NODE_TYPE (cond) == N_cond),
                 "Illegal structure of while-loop function.");
    DBUG_ASSERT ((NODE_TYPE (BLOCK_INSTR (COND_ELSE (cond))) == N_empty),
                 "else part of conditional in while-loop must be empty");

    loop_pred = COND_COND (cond);
    COND_COND (cond) = MakeBool (FALSE);

    /*
     * cond_assign: if (false) { ... } else { } <return-renamings>
     * cond:        if (false) { <statements> <pre-renamings>
     *                           ... = WhileLoopFun(...); <post-renamings> }
     *                    else { }
     * loop_pred:   <pred>
     */

    tmp = BLOCK_INSTR (COND_THEN (cond));
    if (IsRecursiveCall (tmp, fundef)) {
        loop_body = MakeBlock (MakeEmpty (), NULL);
    } else {
        while (!IsRecursiveCall (ASSIGN_NEXT (tmp), fundef)) {
            tmp = ASSIGN_NEXT (tmp);
        }
        loop_body = MakeBlock (BLOCK_INSTR (COND_THEN (cond)), NULL);
        BLOCK_INSTR (COND_THEN (cond)) = ASSIGN_NEXT (tmp);
        ASSIGN_NEXT (tmp) = NULL;
        tmp = BLOCK_INSTR (COND_THEN (cond));
    }
    int_call = tmp;

    /*
     * cond_assign: if (false) { ... } else { } <return-renamings>
     * cond:        if (false) { ... = WhileLoopFun(...); <post-renamings> }
     *                    else { }
     * loop_pred:   <pred>
     * loop_body:   { <statements> <pre-renamings> }
     * int_call:    ... = WhileLoopFun(...); <post-renamings>
     */

    loop_suffix = AppendAssign (ASSIGN_NEXT (int_call), ASSIGN_NEXT (cond_assign));
    /* preserve assignments from being freed */
    ASSIGN_NEXT (int_call) = NULL;
    ASSIGN_NEXT (cond_assign) = NULL;

    /*
     * cond_assign: if (false) { ... = WhileLoopFun(...); } else { }
     * cond:        if (false) { ... = WhileLoopFun(...); } else { }
     * loop_pred:   <pred>
     * loop_body:   { <statements> <pre-renamings> }
     * loop_suffix: <post-renamings> <return-renamings>
     * int_call:    ... = WhileLoopFun(...);
     */

    loop_suffix = AppendAssign (loop_suffix, ASSIGN_NEXT (assign));
    ASSIGN_NEXT (assign) = NULL;

    /* free current assignment */
    FreeTree (assign);
    FreeTree (cond_assign);

    new_assign = MakeAssign (MakeWhile (loop_pred, loop_body), loop_suffix);

    DBUG_RETURN (new_assign);
}

/******************************************************************************
 *
 * function:
 *   node *ReplaceAssignmentByDoLoop( node *assign,
 *                                    node *fundef, node *inl_code)
 *
 * description:
 *   This function replaces the given assignment containing the application
 *   of a do-loop function by the (transformed) inlined body of the
 *   corresponding function definition.
 *
 *   Basically the following transformation is performed:
 *
 *     <statements>
 *     if (<pred>) {
 *       <pre-renamings>
 *       ... = DoLoopFun(...);
 *       <post-renamings>
 *     }
 *     else {
 *     }
 *     <return-renamings>
 *
 *   is being transformed into:
 *
 *     do {
 *       <statements>
 *       <pre-renamings>
 *     } while (<pred>);
 *     <post-renamings>
 *     <return-renamings>
 *
 ******************************************************************************/

static node *
ReplaceAssignmentByDoLoop (node *assign, node *fundef, node *inl_code)
{
    node *cond_assign, *cond;
    node *loop_body, *loop_pred, *loop_suffix;
    node *int_call;
    node *new_assign;
    node *tmp;

    DBUG_ENTER ("ReplaceAssignmentByDoLoop");

    if (NODE_TYPE (ASSIGN_INSTR (inl_code)) == N_cond) {
        cond_assign = inl_code;
        loop_body = MakeBlock (NULL, NULL);
    } else {
        tmp = inl_code;
        while (NODE_TYPE (ASSIGN_INSTR (ASSIGN_NEXT (tmp))) != N_cond) {
            tmp = ASSIGN_NEXT (tmp);
        }
        cond_assign = ASSIGN_NEXT (tmp);
        ASSIGN_NEXT (tmp) = NULL;
        loop_body = MakeBlock (inl_code, NULL);
    }
    cond = ASSIGN_INSTR (cond_assign);
    inl_code = NULL;

    /*
     * cond_assign: if (<pred>) { ... } else { } <return-renamings>
     * cond:        if (<pred>) { ... } else { }
     * loop_body:   { <statements> }
     */

    DBUG_ASSERT ((NODE_TYPE (cond) == N_cond),
                 "Illegal node type in conditional position.");
    DBUG_ASSERT ((NODE_TYPE (BLOCK_INSTR (COND_ELSE (cond))) == N_empty),
                 "else part of conditional in do-loop must be empty");

    loop_pred = COND_COND (cond);
    COND_COND (cond) = MakeBool (FALSE);

    /*
     * cond_assign: if (false) { ... } else { } <return-renamings>
     * cond:        if (false) { ... } else { }
     * loop_pred:   <pred>
     * loop_body:   { <statements> }
     */

    tmp = BLOCK_INSTR (COND_THEN (cond));
    if (!IsRecursiveCall (tmp, fundef)) {
        BLOCK_INSTR (loop_body) = AppendAssign (BLOCK_INSTR (loop_body), tmp);

        while (!IsRecursiveCall (ASSIGN_NEXT (tmp), fundef)) {
            tmp = ASSIGN_NEXT (tmp);
        }
        BLOCK_INSTR (COND_THEN (cond)) = ASSIGN_NEXT (tmp);
        ASSIGN_NEXT (tmp) = NULL;
        tmp = BLOCK_INSTR (COND_THEN (cond));
    }
    int_call = tmp;

    /*
     * cond_assign: if (false) { ... = DoLoopFun(...); <post-renamings> } else { }
     *                <return-renamings>
     * cond:        if (false) { ... = DoLoopFun(...); <post-renamings> } else { }
     * loop_pred:   <pred>
     * loop_body:   { <statements> <pre-renamings> }
     * int_call:    ... = DoLoopFun(...); <post-renamings>
     */

    loop_suffix = AppendAssign (ASSIGN_NEXT (int_call), ASSIGN_NEXT (cond_assign));
    /* preserve assignments from being freed */
    ASSIGN_NEXT (int_call) = NULL;
    ASSIGN_NEXT (cond_assign) = NULL;

    /*
     * cond_assign: if (false) { ... = DoLoopFun(...); } else { }
     * cond:        if (false) { ... = DoLoopFun(...); } else { }
     * loop_pred:   <pred>
     * loop_body:   { <statements> <pre-renamings> }
     * loop_suffix: <post-renamings> <return-renamings>
     * int_call:    ... = DoLoopFun(...);
     */

    loop_suffix = AppendAssign (loop_suffix, ASSIGN_NEXT (assign));
    ASSIGN_NEXT (assign) = NULL;

    /* free current assignment */
    FreeTree (assign);
    FreeTree (cond_assign);

    new_assign = MakeAssign (MakeDo (loop_pred, loop_body), loop_suffix);

    DBUG_RETURN (new_assign);
}

/******************************************************************************
 *
 * function:
 *   node *ReplaceAssignmentByCond( node *assign,
 *                                  node *fundef, node *inl_code)
 *
 * description:
 *   This function replaces the given assignment containing the application
 *   of a conditional function by the inlined body of the corresponding
 *   function definition.
 *
 ******************************************************************************/

static node *
ReplaceAssignmentByCond (node *assign, node *fundef, node *inl_code)
{
    DBUG_ENTER ("ReplaceAssignmentByCond");

    FreeNode (assign);
    inl_code = AppendAssign (inl_code, assign);

    DBUG_RETURN (inl_code);
}

/******************************************************************************
 *
 * function:
 *   node *FUN2LACap( node *arg_node, node *arg_info)
 *
 * description:
 *   If the function applied is a cond or loop function, then the identifiers
 *   in the corresponding function definition are adjusted in order to allow
 *   naive inlining during the bottom-up traversal.
 *
 ******************************************************************************/

node *
FUN2LACap (node *arg_node, node *arg_info)
{
    node *fundef;
    node *inl_code;

    DBUG_ENTER ("FUN2LACap");

    fundef = AP_FUNDEF (arg_node);

    if (FUNDEF_IS_LACFUN (fundef)) {
        inl_code = InlineSingleApplication (INFO_F2L_LET (arg_info),
                                            INFO_F2L_FUNDEF (arg_info), INL_NAIVE);

        DBUG_EXECUTE ("F2L", Print (inl_code););

        INFO_F2L_INLINED (arg_info) = inl_code;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *FUN2LAClet( node *arg_node, node *arg_info)
 *
 * description:
 *   The main purpose of this function is to store the let variables in the
 *   arg_info node in order to have them available if the right hand side
 *   contains an application of a cond or loop function whose identifiers
 *   have to be adjusted to the calling context.
 *
 ******************************************************************************/

node *
FUN2LAClet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("FUN2LAClet");

    INFO_F2L_LET (arg_info) = arg_node;

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *FUN2LACassign( node *arg_node, node *arg_info)
 *
 * description:
 *   The instruction behind the assignment is traversed. Iff it represents the
 *   application of a cond or loop function, this application is replaced by
 *   the inlined code of the cond or loop function, respectively.
 *   Prior to this, the identifiers of the function to be inlined have already
 *   been adjusted in a way that allows rather naive inlining.
 *
 ******************************************************************************/

node *
FUN2LACassign (node *arg_node, node *arg_info)
{
    node *inl_code;
    node *fundef;

    DBUG_ENTER ("FUN2LACassign");

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    inl_code = INFO_F2L_INLINED (arg_info);
    if (inl_code != NULL) {
        DBUG_ASSERT ((NODE_TYPE (LET_EXPR (INFO_F2L_LET (arg_info))) == N_ap),
                     "no N_ap node found!");
        fundef = AP_FUNDEF (LET_EXPR (INFO_F2L_LET (arg_info)));

        switch (FUNDEF_STATUS (fundef)) {
        case ST_condfun:
            DBUG_PRINT ("F2L", ("Naive inlining of conditional function %s.\n",
                                ItemName (fundef)));

            arg_node = ReplaceAssignmentByCond (arg_node, fundef, inl_code);
            break;

        case ST_whilefun:
            DBUG_PRINT ("F2L", ("Naive inlining of while-loop function %s.\n",
                                ItemName (fundef)));

            arg_node = ReplaceAssignmentByWhileLoop (arg_node, fundef, inl_code);
            break;

        case ST_dofun:
            DBUG_PRINT ("F2L",
                        ("Naive inlining of do-loop function %s.\n", ItemName (fundef)));

            arg_node = ReplaceAssignmentByDoLoop (arg_node, fundef, inl_code);
            break;

        default:
            DBUG_ASSERT (0, "Illegal status of abstracted cond or loop fun");
        }

        INFO_F2L_FUNDEF (arg_info) = NULL;
        INFO_F2L_INLINED (arg_info) = NULL;

        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *FUN2LACblock( node *arg_node, node *arg_info)
 *
 * description:
 *   This function initiates the traversal of the instruction chain
 *   of an assignment block. The vardecs are not traversed.
 *
 ******************************************************************************/

node *
FUN2LACblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("FUN2LACblock");

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *FUN2LACfundef( node *arg_node, node *arg_info)
 *
 * description:
 *   This function traverses the function body of any function that does NOT
 *   represent an abstracted conditional or loop.
 *
 ******************************************************************************/

node *
FUN2LACfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("FUN2LACfundef");

    INFO_F2L_FUNDEF (arg_info) = arg_node;

    if ((!FUNDEF_IS_LACFUN (arg_node)) && (FUNDEF_BODY (arg_node) != NULL)) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    if (FUNDEF_IS_LACFUN (arg_node)) {
        /*
         * At this point, only the prototype of the conditional or loop
         * function remains in the fundef chain.
         */
        arg_node = FreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *FUN2LACmodul( node *arg_node, node *arg_info)
 *
 * description:
 *   This function traverses all function definitions under an N_modul
 *   node.
 *
 ******************************************************************************/

node *
FUN2LACmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("FUN2LACmodul");

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *Fun2Lac( node *syntax_tree)
 *
 * description:
 *   This function initiates the tree traversal process for reconverting
 *   functional representations of loops and conditionals into their
 *   explicit representations.
 *
 ******************************************************************************/

node *
Fun2Lac (node *syntax_tree)
{
    node *info_node;

    DBUG_ENTER ("Fun2Lac");

    act_tab = fun2lac_tab;
    info_node = MakeInfo ();

    syntax_tree = Trav (syntax_tree, info_node);

    FreeNode (info_node);

    DBUG_RETURN (syntax_tree);
}
