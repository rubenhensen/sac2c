/*
 *
 * $Log$
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
 *
 *
 */

/*****************************************************************************
 *
 * file:   fun2lac.c
 *
 * prefix: FUN2LAC
 *
 * description:
 *
 *   This compiler module implements the reconversion of conditionals and
 *   loops from their true functional representation generated by routines
 *   found in lac2fun.c into an explicit representation suitable for code
 *   generation.
 *
 *****************************************************************************/

#include "tree.h"
#include "traverse.h"
#include "free.h"
#include "dbug.h"

#include "adjust_ids.h"

/******************************************************************************
 *
 * function:
 *
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *
 *
 * description:
 *
 *
 *
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
 *   node *MoveVardecs(node *source, node *dest)
 *
 * description:
 *
 *   This function moves the vardec nodes from a conditional or loop function
 *   to the corresponding function definition where the conditional or loop
 *   function is applied.
 *
 ******************************************************************************/

static node *
MoveVardecs (node *source, node *dest)
{
    node *tmp;

    DBUG_ENTER ("MoveVardecs");

    tmp = BLOCK_VARDEC (source);

    if (tmp != NULL) {
        while (VARDEC_NEXT (tmp) != NULL) {
            tmp = VARDEC_NEXT (tmp);
        }
        VARDEC_NEXT (tmp) = BLOCK_VARDEC (dest);
        BLOCK_VARDEC (dest) = BLOCK_VARDEC (source);
        BLOCK_VARDEC (source) = NULL;
    }

    DBUG_RETURN (dest);
}

/******************************************************************************
 *
 * function:
 *   node *ReplaceAssignmentByWhileLoopFun(node *assign, node *fundef)
 *
 * description:
 *
 *   This function replaces the given assignment containing the application
 *   of a while-loop function by the transformed body of the corresponding
 *   function definition.
 *
 *   Basically the following transformation is performed:
 *
 *   ... WhileLoopFun(...)
 *   {
 *     if (<pred>) {
 *       <statements>;
 *       <pre-renamings>;
 *       ... = WhileLoopFun(...);
 *       <post-renamings>;
 *     }
 *     else {
 *       <anything>;
 *     }
 *     <return-renamings>;
 *     return(...);
 *   }
 *
 *   ...;
 *   ... = WhileLoopFun(...);
 *   ...;
 *
 *
 *   is being transformed into:
 *
 *   ...;
 *   while (<pred>) {
 *     <statements>;
 *     <pre-renamings>;
 *   }
 *   <post-renamings>;
 *   ...;
 *
 ******************************************************************************/

static node *
ReplaceAssignmentByWhileLoopFun (node *assign, node *fundef)
{
    node *tmp, *cond, *loop_body, *loop_suffix, *loop_pred, *new_assign;

    DBUG_ENTER ("ReplaceAssignmentByWhileLoopFun");

    cond = ASSIGN_INSTR (BLOCK_INSTR (FUNDEF_BODY (fundef)));

    DBUG_ASSERT ((NODE_TYPE (cond) == N_cond),
                 "Illegal structure of while-loop function.");

    loop_pred = COND_COND (cond);
    COND_COND (cond) = MakeBool (0);

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

    /*
     * Here tmp points to the recursive function application.
     */

    loop_suffix = AppendAssign (ASSIGN_NEXT (tmp), ASSIGN_NEXT (assign));
    ASSIGN_NEXT (tmp) = NULL;
    ASSIGN_NEXT (assign) = NULL;

    FreeTree (assign);
    /* free current assignment */

    new_assign = MakeAssign (MakeWhile (loop_pred, loop_body), loop_suffix);

    DBUG_RETURN (new_assign);
}

/******************************************************************************
 *
 * function:
 *   node *ReplaceAssignmentByDoLoopFun(node *assign, node *fundef)
 *
 * description:
 *
 *   This function replaces the given assignment containing the application
 *   of a do-loop function by the transformed body of the corresponding
 *   function definition.
 *
 *   Basically the following transformation is performed:
 *
 *   ... DoLoopFun(...)
 *   {
 *     <statements>;
 *     if (<pred>) {
 *       <pre-renamings>;
 *       ... = DoLoopFun(...);
 *       <post-renamings>;
 *     }
 *     else {
 *       <anything>;
 *     }
 *     <return-renamings>;
 *     return(...);
 *   }
 *
 *   ...;
 *   ... = DoLoopFun(...);
 *   ...;
 *
 *
 *   is being transformed into:
 *
 *   ...;
 *   do {
 *     <statements>;
 *     <pre-renamings>;
 *   } while (<pred>);
 *   <post-renamings>;
 *   ...;
 *
 ******************************************************************************/

static node *
ReplaceAssignmentByDoLoopFun (node *assign, node *fundef)
{
    node *tmp, *cond, *loop_body, *loop_suffix, *loop_pred, *new_assign;

    DBUG_ENTER ("ReplaceAssignmentByDoLoopFun");

    tmp = BLOCK_INSTR (FUNDEF_BODY (fundef));

    if (NODE_TYPE (ASSIGN_INSTR (tmp)) == N_cond) {
        loop_body = MakeBlock (NULL, NULL);
    } else {
        while (NODE_TYPE (ASSIGN_INSTR (ASSIGN_NEXT (tmp))) != N_cond) {
            tmp = ASSIGN_NEXT (tmp);
        }
        loop_body = MakeBlock (BLOCK_INSTR (FUNDEF_BODY (fundef)), NULL);
        BLOCK_INSTR (FUNDEF_BODY (fundef)) = ASSIGN_NEXT (tmp);
        ASSIGN_NEXT (tmp) = NULL;
        tmp = BLOCK_INSTR (FUNDEF_BODY (fundef));
    }

    /*
     * Variable tmp now points to the conditional.
     */

    cond = ASSIGN_INSTR (tmp);

    DBUG_ASSERT ((NODE_TYPE (cond) == N_cond),
                 "Illegal node type in conditional position.");

    loop_pred = COND_COND (cond);
    COND_COND (cond) = MakeBool (0);

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

    /*
     * Here tmp points to the recursive function application.
     */

    loop_suffix = AppendAssign (ASSIGN_NEXT (tmp), ASSIGN_NEXT (assign));
    ASSIGN_NEXT (tmp) = NULL;
    ASSIGN_NEXT (assign) = NULL;

    FreeTree (assign);
    /* free current assignment */

    new_assign = MakeAssign (MakeDo (loop_pred, loop_body), loop_suffix);

    DBUG_RETURN (new_assign);
}

/******************************************************************************
 *
 * function:
 *   node *ReplaceAssignmentByCondFun(node *assign, node *fundef)
 *
 * description:
 *
 *   This function replaces the given assignment containing the application
 *   of a conditional function by the transformed body of the corresponding
 *   function definition.
 *
 *   Basically the following transformation is performed:
 *
 *   ... CondFun(...)
 *   {
 *     if (<pred>) {
 *       <then-statements>;
 *     }
 *     else {
 *       <else-statements>;
 *     }
 *     <return-renamings>;
 *     return(...);
 *   }
 *
 *   ...;
 *   ... =CondFun(...);
 *   ...;
 *
 *
 *   is being transformed into:
 *
 *   ...;
 *   if (<pred>) {
 *     <then-statements>;
 *   }
 *   else {
 *     <else-statements>;
 *   }
 *   <return-renamings>;
 *   ...;
 *
 ******************************************************************************/

static node *
ReplaceAssignmentByCondFun (node *assign, node *fundef)
{
    node *assign_chain, *tmp;

    DBUG_ENTER ("ReplaceAssignmentByCondFun");

    assign_chain = BLOCK_INSTR (FUNDEF_BODY (fundef));

    tmp = assign_chain;

    while (NODE_TYPE (ASSIGN_INSTR (ASSIGN_NEXT (tmp))) != N_return) {
        /* find last assignment before return statement */
        tmp = ASSIGN_NEXT (tmp);
    }

    BLOCK_INSTR (FUNDEF_BODY (fundef)) = ASSIGN_NEXT (tmp);
    /* Remove all assignments but the return statement from function definition */

    ASSIGN_NEXT (tmp) = ASSIGN_NEXT (assign);
    /* append following assignments to the assignment chain of the conditional
       function */

    FUNDEF_BODY (fundef) = FreeTree (FUNDEF_BODY (fundef));
    /* free remaining body of conditional function */

    ASSIGN_NEXT (assign) = NULL;
    FreeTree (assign);
    /* free current assignment */

    DBUG_RETURN (assign_chain);
}

/******************************************************************************
 *
 * function:
 *   node *FUN2LACap( node *arg_node, node *arg_info)
 *
 * description:
 *
 *   If the function applied is a cond or loop function, then the identifiers
 *   in the corresponding function definition are adjusted in order to allow
 *   naive inlining during the bottom-up traversal.
 *
 ******************************************************************************/

node *
FUN2LACap (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("FUN2LACap");

    if ((FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_condfun)
        || (FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_whilefun)
        || (FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_dofun)) {

        AdjustIdentifiers (AP_FUNDEF (arg_node), INFO_FUN2LAC_LET (arg_info));
        INFO_FUN2LAC_FUNDEF (arg_info) = AP_FUNDEF (arg_node);
        /*
         * The fundef node of an applied conditional or loop function is stored
         * in arg_info. This allows to memory the interesting case during the
         * following bottom-up traversal.
         */
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *FUN2LAClet( node *arg_node, node *arg_info)
 *
 * description:
 *
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

    INFO_FUN2LAC_LET (arg_info) = arg_node;

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    INFO_FUN2LAC_LET (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *FUN2LACassign( node *arg_node, node *arg_info)
 *
 * description:
 *
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
    DBUG_ENTER ("FUN2LACassign");

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_FUN2LAC_FUNDEF (arg_info) != NULL) {
        INFO_FUN2LAC_FUNBLOCK (arg_info)
          = MoveVardecs (FUNDEF_BODY (INFO_FUN2LAC_FUNDEF (arg_info)),
                         INFO_FUN2LAC_FUNBLOCK (arg_info));

        switch (FUNDEF_STATUS (INFO_FUN2LAC_FUNDEF (arg_info))) {
        case ST_condfun: {
            DBUG_PRINT ("FUN2LAC", ("Naive inlining of conditional function %s.\n",
                                    ItemName (INFO_FUN2LAC_FUNDEF (arg_info))));

            arg_node
              = ReplaceAssignmentByCondFun (arg_node, INFO_FUN2LAC_FUNDEF (arg_info));
            break;
        }
        case ST_whilefun: {
            DBUG_PRINT ("FUN2LAC", ("Naive inlining of while-loop function %s.\n",
                                    ItemName (INFO_FUN2LAC_FUNDEF (arg_info))));

            arg_node = ReplaceAssignmentByWhileLoopFun (arg_node,
                                                        INFO_FUN2LAC_FUNDEF (arg_info));
            break;
        }
        case ST_dofun: {
            DBUG_PRINT ("FUN2LAC", ("Naive inlining of do-loop function %s.\n",
                                    ItemName (INFO_FUN2LAC_FUNDEF (arg_info))));

            arg_node
              = ReplaceAssignmentByDoLoopFun (arg_node, INFO_FUN2LAC_FUNDEF (arg_info));
            break;
        }
        default: {
            DBUG_ASSERT (0, "Illegal status of abstracted cond or loop fun");
        }
        }

        INFO_FUN2LAC_FUNDEF (arg_info) = NULL;

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
 *
 *   This function initiates the traversal of the instruction chain
 *   of an assignment block. The vardecs are not traversed.
 *
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
 *
 *   This function traverses the function body of any function that does NOT
 *   represent an abstracted conditional or loop.
 *
 ******************************************************************************/

node *
FUN2LACfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("FUN2LACfundef");

    if ((FUNDEF_STATUS (arg_node) != ST_condfun)
        && (FUNDEF_STATUS (arg_node) != ST_whilefun)
        && (FUNDEF_STATUS (arg_node) != ST_dofun) && (FUNDEF_BODY (arg_node) != NULL)) {
        INFO_FUN2LAC_FUNBLOCK (arg_info) = FUNDEF_BODY (arg_node);
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUN2LAC_FUNBLOCK (arg_info) = NULL;
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    if ((FUNDEF_STATUS (arg_node) == ST_condfun)
        || (FUNDEF_STATUS (arg_node) == ST_whilefun)
        || (FUNDEF_STATUS (arg_node) == ST_dofun)) {
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
 *
 *   This function traverses all function definitions under an N_modul
 *   node.
 *
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
 *
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
