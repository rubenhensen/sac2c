/*
 *
 * $Log$
 * Revision 3.10  2001/05/10 11:54:36  dkr
 * another bug in BuildRenamingAssigns() fixed:
 * assignments are wrapped by conditional now if needed
 *
 * Revision 3.9  2001/05/09 14:22:30  dkr
 * bug in BuildRenamingAssigns() fixed:
 * variables are swap correctly now if needed
 *
 * Revision 3.8  2001/05/07 14:22:41  nmw
 * BuildRenamingAssigns processes all args now :-)
 *
 * Revision 3.7  2001/04/26 13:26:29  dkr
 * BuildRenamingAssigns() and ReturnVarsAreIdentical() added
 *
 * Revision 3.6  2001/04/26 01:43:44  dkr
 * - InlineSingleApplication() used for inlining now :-)
 * - Probably the transformation scheme is too regide yet, especially for
 *   SSA :-((
 *   But okay ... let's give it a try ...
 *
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

#include "dbug.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
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

    DBUG_ASSERT ((NODE_TYPE (assign) == N_assign), "illegal parameter found!");
    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef), "illegal parameter found!");

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
 * Function:
 *   node *GetRenamingExpr( node **vardecs, node **assigns,
 *                          node *ext_args, node *int_args)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
GetRenamingExpr (node **vardecs, node **assigns, node *ext_args, node *int_args)
{
    char *tmp_name;
    node *int_expr;
    node *tmp_ext_args, *tmp_int_args;
    node *assign;
    node *expr_node = NULL;

    DBUG_ENTER ("GetRenamingId");

    DBUG_ASSERT (((vardecs != NULL) && ((*vardecs) != NULL)
                  && (NODE_TYPE ((*vardecs)) == N_vardec)),
                 "no vardecs found!");
    DBUG_ASSERT (((assigns != NULL)
                  && (((*assigns) == NULL) || (NODE_TYPE ((*assigns)) == N_assign))),
                 "illegal parameter found!");
    DBUG_ASSERT (((ext_args == NULL) || (NODE_TYPE (ext_args) == N_arg)),
                 "illegal parameter found!");
    DBUG_ASSERT (((int_args == NULL) || (NODE_TYPE (int_args) == N_exprs)),
                 "illegal parameter found!");

    int_expr = EXPRS_EXPR (int_args);
    tmp_ext_args = ext_args;
    tmp_int_args = int_args;
    while (tmp_ext_args != NULL) {
        DBUG_ASSERT (((ID_VARDEC (int_expr) != NULL)
                      && (ID_VARDEC (EXPRS_EXPR (tmp_int_args)) != NULL)),
                     "vardec not found!");

        if (((NODE_TYPE (EXPRS_EXPR (tmp_int_args)) != N_id)
             || (tmp_ext_args != ID_VARDEC (EXPRS_EXPR (tmp_int_args))))
            && (NODE_TYPE (int_expr) == N_id) && (tmp_ext_args == ID_VARDEC (int_expr))) {
            tmp_name = TmpVarName (ID_NAME (int_expr));
            (*vardecs) = MakeVardec (tmp_name, DupTypes (ID_TYPE (int_expr)), *vardecs);

            expr_node = MakeId_Copy (tmp_name);
            ID_VARDEC (expr_node) = (*vardecs);

            assign = MakeAssignLet (StringCopy (tmp_name), *vardecs,
                                    DupNode (EXPRS_EXPR (int_args)));
            ASSIGN_NEXT (assign) = (*assigns);
            (*assigns) = assign;
            break;
        }

        tmp_ext_args = ARG_NEXT (tmp_ext_args);
        tmp_int_args = EXPRS_NEXT (tmp_int_args);
    }

    if (expr_node == NULL) {
        expr_node = DupNode (EXPRS_EXPR (int_args));
    }

    DBUG_RETURN (expr_node);
}

/******************************************************************************
 *
 * Function:
 *   bool RenamingConflictsWithReturn( node *arg, node *ret_args)
 *
 * Description:
 *
 *
 ******************************************************************************/

bool
RenamingConflictsWithReturn (node *arg, node *ret_args)
{
    bool confl = FALSE;

    DBUG_ENTER ("RenamingConflictsWithReturn");

    DBUG_ASSERT ((NODE_TYPE (arg) == N_arg), "illegal parameter found!");
    DBUG_ASSERT (((ret_args == NULL) || (NODE_TYPE (ret_args) == N_exprs)),
                 "illegal parameter found!");

    while (ret_args != NULL) {
        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (ret_args)) == N_id),
                     "return value of special LaC function must be a N_id node!");
        DBUG_ASSERT ((ID_VARDEC (EXPRS_EXPR (ret_args)) != NULL), "vardec not found!");

        if (ID_VARDEC (EXPRS_EXPR (ret_args)) == arg) {
            confl = TRUE;
            break;
        }

        ret_args = EXPRS_NEXT (ret_args);
    }

    DBUG_RETURN (confl);
}

/******************************************************************************
 *
 * Function:
 *   node *BuildRenamingAssigns( node **vardecs,
 *                               node *ext_args, node *int_args,
 *                               node *pred, node *ret_args)
 *
 * Description:
 *   This function returns a N_assign node chain.
 *   If the names of the actual parameters found in the recursive call of a
 *   loop-function ('int_args') differ from the names of the formal parameters
 *   ('ext_args') appropriate renaming assignments are build:
 *
 *     ... Loop( a_1, a_2, ...)                           loop {
 *     {                                                    <ass_1>
 *       <ass_1>                                            <ass_2>
 *       if (<cond>) {
 *         <ass_2>                                 --->     a_1 = A_1;
 *         r_1, r_2, ... = Loop( A_1, A_2, ...);            a_2 = A_2;
 *       }                                                  ...
 *       return( r_1, r_2, ...);                          }
 *     }
 *
 *   The assignments are ordered with descending 'i'.
 *   An assignment is build iff (A_i != a_i) is hold.
 *
 *   *** CAUTION ***
 *   If, for a given 'i' with (A_i != a_i), one of the following conditions
 *   is hold, we must *not* assign (a_i = A_i) directly:
 *      [1]  exists j: ((j > i) && (A_j != a_j) && (A_i == a_j))
 *      [2]  exists j: (r_j == a_i)
 *
 *   Example:
 *
 *     ... Do( a, b, c, d, e, f)
 *     {
 *       <ass>
 *       if (<cond>) {
 *         d, f = Do( b, a, C, D, e, f);
 *       }
 *       return( d, f);
 *     }
 *
 *   --->
 *
 *     do {                               do {
 *       <ass>                              <ass>
 *
 *                                          tmp_b = b;
 *
 *       d = D;  // case [2]                if (<cond>) {
 *                                            d = D;
 *                                          }
 *       c = C;                             c = C;
 *       b = a;                             b = a;
 *       a = b;  // case [1]                a = tmp_b;
 *     }                                  }
 *     while (<cond>);                    while (<cond>);
 *     ... d ...                          ... d ...
 *
 ******************************************************************************/

static node *
BuildRenamingAssigns (node **vardecs, node *ext_args, node *int_args, node *pred,
                      node *ret_args)
{
    node *assign;
    node *cond_block;
    node *assigns = NULL;
    node *tmp_assigns = NULL;

    DBUG_ENTER ("BuildRenamingAssigns");

    DBUG_ASSERT (((vardecs != NULL) && ((*vardecs) != NULL)
                  && (NODE_TYPE ((*vardecs)) == N_vardec)),
                 "no vardecs found!");
    DBUG_ASSERT (((ext_args == NULL) || (NODE_TYPE (ext_args) == N_arg)),
                 "illegal parameter found!");
    DBUG_ASSERT (((int_args == NULL) || (NODE_TYPE (int_args) == N_exprs)),
                 "illegal parameter found!");
    DBUG_ASSERT ((pred != NULL), "illegal parameter found!");
    DBUG_ASSERT (((ret_args == NULL) || (NODE_TYPE (ret_args) == N_exprs)),
                 "illegal parameter found!");

    DBUG_ASSERT ((CountArgs (ext_args) == CountExprs (int_args)),
                 "inconsistent LAC-signature found");

    while (ext_args != NULL) {
        DBUG_ASSERT ((ID_VARDEC (EXPRS_EXPR (int_args)) != NULL), "vardec not found!");

        if ((NODE_TYPE (EXPRS_EXPR (int_args)) != N_id)
            || (ext_args != ID_VARDEC (EXPRS_EXPR (int_args)))) {
            assign = MakeAssignLet (StringCopy (ARG_NAME (ext_args)), ext_args,
                                    /* here, case [1] is handled: */
                                    GetRenamingExpr (vardecs, &tmp_assigns, ext_args,
                                                     int_args));

            /* here, case [2] is handled: */
            if (RenamingConflictsWithReturn (ext_args, ret_args)) {
                if ((assigns != NULL) && (NODE_TYPE (ASSIGN_INSTR (assigns)) == N_cond)) {
                    /*
                     * head of assignment chain is already a conditional
                     *  -> insert new assingment at head of then-block
                     */
                    cond_block = COND_THEN (ASSIGN_INSTR (assigns));
                    ASSIGN_NEXT (assign) = BLOCK_INSTR (cond_block);
                    BLOCK_INSTR (cond_block) = assign;
                } else {
                    assigns
                      = MakeAssign (MakeCond (DupTree (pred), MakeBlock (assign, NULL),
                                              MakeBlock (MakeEmpty (), NULL)),
                                    assigns);
                }
            } else {
                ASSIGN_NEXT (assign) = assigns;
                assigns = assign;
            }
        }

        ext_args = ARG_NEXT (ext_args);
        int_args = EXPRS_NEXT (int_args);
    }

    assigns = AppendAssign (tmp_assigns, assigns);

    DBUG_RETURN (assigns);
}

#ifndef DBUG_OFF
/******************************************************************************
 *
 * Function:
 *   bool ReturnVarsAreIdentical( node *ext_rets, ids *int_rets)
 *
 * Description:
 *
 *
 ******************************************************************************/

static bool
ReturnVarsAreIdentical (node *ext_rets, ids *int_rets)
{
    bool ok = TRUE;

    DBUG_ENTER ("ReturnVarsAreIdentical");

    DBUG_ASSERT (((ext_rets == NULL) || (NODE_TYPE (ext_rets) == N_exprs)),
                 "illegal parameter found!");

    DBUG_ASSERT ((CountExprs (ext_rets) == CountIds (int_rets)),
                 "inconsistent LAC-signature found");

    while (ext_rets != NULL) {
        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (ext_rets)) == N_id),
                     "return value of special LaC function must be a N_id node!");
        DBUG_ASSERT (((ID_VARDEC (EXPRS_EXPR (ext_rets)) != NULL)
                      && (IDS_VARDEC (int_rets) != NULL)),
                     "vardec not found!");

        if (ID_VARDEC (EXPRS_EXPR (ext_rets)) != IDS_VARDEC (int_rets)) {
            ok = FALSE;
            break;
        }

        ext_rets = EXPRS_NEXT (ext_rets);
        int_rets = IDS_NEXT (int_rets);
    }

    DBUG_RETURN (ok);
}
#endif

/******************************************************************************
 *
 * function:
 *   node *TransformIntoWhileLoop( node *fundef)
 *
 * description:
 *   This function replaces the given assignment containing the application
 *   of a while-loop function by the (transformed) inlined body of the
 *   corresponding function definition.
 *
 *   Basically the following transformation is performed:
 *
 *     ... WhileLoopFun( a_1, a_2, ...)
 *     {
 *       <vardecs>
 *
 *       if (<pred>) {
 *         <ass>
 *         r_1, r_2, ... = WhileLoopFun( A_1, A_2, ...);
 *       }
 *       else {
 *       }
 *
 *       return( r_1, r_2, ...);
 *     }
 *
 *   is being transformed into:
 *
 *     ... WhileLoopFun( a_1, a_2, ...)
 *     {
 *       <vardecs>
 *
 *       while (<pred>) {
 *         <ass>
 *         a_i = A_i;
 *       }
 *
 *       return( r_1, r_2, ...);
 *     }
 *
 ******************************************************************************/

static node *
TransformIntoWhileLoop (node *fundef)
{
    node *cond_assign, *cond;
    node *loop_body, *loop_pred;
    node *int_call, *ret;
    node *tmp;

    DBUG_ENTER ("TransformIntoWhileLoop");

    /*
     * CAUTION:
     * In case of (FUNDEF_USED > 1) the 'fundef' may have been transformed into
     * a while-loop already!
     */
    if (NODE_TYPE (ASSIGN_INSTR (FUNDEF_INSTR (fundef))) != N_while) {
        cond_assign = FUNDEF_INSTR (fundef);
        cond = ASSIGN_INSTR (cond_assign);

        /*
         * cond_assign: if (<pred>) { ... }  return(...);
         * cond:        if (<pred>) { ... }
         */

        if (NODE_TYPE (cond) != N_return) {
            ret = ASSIGN_INSTR (ASSIGN_NEXT (cond_assign));

            DBUG_ASSERT ((NODE_TYPE (cond) == N_cond),
                         "Illegal structure of while-loop function.");
            DBUG_ASSERT ((NODE_TYPE (BLOCK_INSTR (COND_ELSE (cond))) == N_empty),
                         "else part of conditional in while-loop function must be"
                         " empty");
            DBUG_ASSERT ((NODE_TYPE (ret) == N_return),
                         "after conditional in while-loop funtion no assignments are"
                         " allowed");

            loop_pred = COND_COND (cond);
            COND_COND (cond) = MakeBool (FALSE);

            /*
             * cond_assign: if (false) { ... }  return(...);
             * cond:        if (false) { <ass>  ... = WhileLoopFun(...); }
             * ret:         return(...);
             * loop_pred:   <pred>
             */

            tmp = BLOCK_INSTR (COND_THEN (cond));
            if (IsRecursiveCall (tmp, fundef)) {
                loop_body = NULL;
            } else {
                while (!IsRecursiveCall (ASSIGN_NEXT (tmp), fundef)) {
                    tmp = ASSIGN_NEXT (tmp);
                    DBUG_ASSERT ((tmp != NULL),
                                 "recursive call of while-loop function not found");
                }
                loop_body = BLOCK_INSTR (COND_THEN (cond));
                BLOCK_INSTR (COND_THEN (cond)) = ASSIGN_NEXT (tmp);
                ASSIGN_NEXT (tmp) = NULL;
                tmp = BLOCK_INSTR (COND_THEN (cond));
            }
            int_call = tmp;

            DBUG_ASSERT ((ReturnVarsAreIdentical (RETURN_EXPRS (ret),
                                                  ASSIGN_LHS (int_call))),
                         "vars on LHS of recursive call and in return statement"
                         " are not identical!");

            /*
             * cond_assign: if (false) { ... }  return(...);
             * cond:        if (false) { ... = WhileLoopFun(...); }
             * ret:         return(...);
             * loop_pred:   <pred>
             * loop_body:   <ass>
             * int_call:    ... = WhileLoopFun(...);
             */

            loop_body
              = AppendAssign (loop_body,
                              BuildRenamingAssigns (&(FUNDEF_VARDEC (fundef)),
                                                    FUNDEF_ARGS (fundef),
                                                    AP_ARGS (ASSIGN_RHS (int_call)),
                                                    loop_pred, RETURN_EXPRS (ret)));

            if (loop_body == NULL) {
                loop_body = MakeEmpty ();
            }
            loop_body = MakeBlock (loop_body, NULL);

            /*
             * cond_assign: if (false) { ... }  return(...);
             * cond:        if (false) { ... = WhileLoopFun(...); }
             * ret:         return(...);
             * loop_pred:   <pred>
             * loop_body:   { <ass> a_i = A_i; }
             * int_call:    ... = WhileLoopFun(...);
             */

            DBUG_ASSERT ((ASSIGN_NEXT (int_call) == NULL),
                         "recursive call of while-loop funtion must be the last"
                         " assignments of the conditional");

            /*
             * cond_assign: if (false) { ... }  return(...);
             * cond:        if (false) { ... = WhileLoopFun(...); }
             * ret:         return(...);
             * loop_pred:   <pred>
             * loop_body:   { <ass> a_i = A_i; }
             * int_call:    ... = WhileLoopFun(...);
             */

            /* replace cond by while-loop */
            ASSIGN_INSTR (cond_assign) = FreeTree (ASSIGN_INSTR (cond_assign));
            ASSIGN_INSTR (cond_assign) = MakeWhile (loop_pred, loop_body);
        }
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   node *TransformIntoDoLoop( node *fundef)
 *
 * description:
 *   This function replaces the given assignment containing the application
 *   of a do-loop function by the (transformed) inlined body of the
 *   corresponding function definition.
 *
 *   Basically the following transformation is performed:
 *
 *     ... DoLoopFun( a_1, a_2, ...)
 *     {
 *       <vardecs>
 *
 *       <ass>
 *       if (<pred>) {
 *         r_1, r_2, ... = DoLoopFun( A_1, A_2, ...);
 *       }
 *       else {
 *       }
 *
 *       return( r_1, r_2, ...);
 *     }
 *
 *   is being transformed into:
 *
 *     ... DoLoopFun( a_1, a_2, ...)
 *     {
 *       <vardecs>
 *
 *       do {
 *         <ass>
 *         a_i = A_i;
 *       } while (<pred>);
 *
 *       return( r_1, r_2, ...);
 *     }
 *
 ******************************************************************************/

static node *
TransformIntoDoLoop (node *fundef)
{
    node *assigns;
    node *cond_assign, *cond;
    node *loop_body, *loop_pred;
    node *int_call, *ret;
    node *tmp;

    DBUG_ENTER ("TransformIntoDoLoop");

    /*
     * CAUTION:
     * In case of (FUNDEF_USED > 1) the 'fundef' may have been transformed into
     * a do-loop already!
     */
    if (NODE_TYPE (ASSIGN_INSTR (FUNDEF_INSTR (fundef))) != N_do) {
        assigns = FUNDEF_INSTR (fundef);

        if (NODE_TYPE (ASSIGN_INSTR (assigns)) == N_cond) {
            cond_assign = assigns;
            loop_body = NULL;
        } else {
            tmp = assigns;
            while ((NODE_TYPE (ASSIGN_INSTR (ASSIGN_NEXT (tmp))) != N_cond)
                   && (NODE_TYPE (ASSIGN_INSTR (ASSIGN_NEXT (tmp))) != N_return)) {
                tmp = ASSIGN_NEXT (tmp);
                DBUG_ASSERT ((tmp != NULL),
                             "recursive call of do-loop function not found");
            }
            cond_assign = ASSIGN_NEXT (tmp);
            ASSIGN_NEXT (tmp) = NULL;
            loop_body = assigns;
        }
        cond = ASSIGN_INSTR (cond_assign);

        /*
         * cond_assign: if (<pred>) { ... }  return(...);
         * cond:        if (<pred>) { ... }
         * loop_body:   <ass>
         */

        if (NODE_TYPE (cond) != N_return) {
            ret = ASSIGN_INSTR (ASSIGN_NEXT (cond_assign));

            DBUG_ASSERT ((NODE_TYPE (cond) == N_cond),
                         "Illegal node type in conditional position.");
            DBUG_ASSERT ((NODE_TYPE (BLOCK_INSTR (COND_ELSE (cond))) == N_empty),
                         "else part of conditional in do-loop must be empty");
            DBUG_ASSERT ((NODE_TYPE (ret) == N_return),
                         "after conditional in do-loop funtion no assignments are"
                         " allowed");

            loop_pred = COND_COND (cond);
            COND_COND (cond) = MakeBool (FALSE);

            /*
             * cond_assign: if (false) { ... = DoLoopFun(...); }  return(...);
             * cond:        if (false) { ... = DoLoopFun(...); }
             * ret:         return(...);
             * loop_pred:   <pred>
             * loop_body:   <ass>
             */

            int_call = BLOCK_INSTR (COND_THEN (cond));
            DBUG_ASSERT ((IsRecursiveCall (int_call, fundef)
                          && (ASSIGN_NEXT (int_call) == NULL)),
                         "recursive call of do-loop function must be the only"
                         " assignment in the conditional");

            DBUG_ASSERT ((ReturnVarsAreIdentical (RETURN_EXPRS (ret),
                                                  ASSIGN_LHS (int_call))),
                         "vars on LHS of recursive call and in return statement"
                         " are not identical!");

            /*
             * cond_assign: if (false) { ... = DoLoopFun(...); }  return(...);
             * cond:        if (false) { ... = DoLoopFun(...); }
             * ret:         return(...);
             * loop_pred:   <pred>
             * loop_body:   <ass>
             * int_call:    ... = DoLoopFun(...);
             */

            loop_body
              = AppendAssign (loop_body,
                              BuildRenamingAssigns (&(FUNDEF_VARDEC (fundef)),
                                                    FUNDEF_ARGS (fundef),
                                                    AP_ARGS (ASSIGN_RHS (int_call)),
                                                    loop_pred, RETURN_EXPRS (ret)));

            if (loop_body == NULL) {
                loop_body = MakeEmpty ();
            }
            loop_body = MakeBlock (loop_body, NULL);

            /*
             * cond_assign: if (false) { ... = DoLoopFun(...); }  return(...);
             * cond:        if (false) { ... = DoLoopFun(...); }
             * ret:         return(...);
             * loop_pred:   <pred>
             * loop_body:   { <ass> a_i = A_i; }
             * int_call:    ... = DoLoopFun(...);
             */

            /* replace cond by do-loop */
            ASSIGN_INSTR (cond_assign) = FreeTree (ASSIGN_INSTR (cond_assign));
            ASSIGN_INSTR (cond_assign) = MakeDo (loop_pred, loop_body);
            FUNDEF_INSTR (fundef) = cond_assign;
        }
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   node *TransformIntoCond( node *fundef)
 *
 * description:
 *
 *
 ******************************************************************************/

static node *
TransformIntoCond (node *fundef)
{
    DBUG_ENTER ("TransformIntoCond");

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   node *FUN2LACap( node *arg_node, node *arg_info)
 *
 * description:
 *   If the function applied is a cond or loop function, then the function
 *   body is re-transformed into a cond or loop and inlined afterwards.
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
        switch (FUNDEF_STATUS (fundef)) {
        case ST_condfun:
            DBUG_PRINT ("F2L", ("Naive inlining of conditional function %s.\n",
                                ItemName (fundef)));

            fundef = TransformIntoCond (fundef);
            break;

        case ST_whilefun:
            DBUG_PRINT ("F2L", ("Naive inlining of while-loop function %s.\n",
                                ItemName (fundef)));

            fundef = TransformIntoWhileLoop (fundef);
            break;

        case ST_dofun:
            DBUG_PRINT ("F2L",
                        ("Naive inlining of do-loop function %s.\n", ItemName (fundef)));

            fundef = TransformIntoDoLoop (fundef);
            break;

        default:
            DBUG_ASSERT (0, "Illegal status of abstracted cond or loop fun");
        }

        DBUG_EXECUTE ("F2L", PrintNode (fundef););

        inl_code
          = InlineSingleApplication (INFO_F2L_LET (arg_info), INFO_F2L_FUNDEF (arg_info));

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
 *
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
 *
 ******************************************************************************/

node *
FUN2LACassign (node *arg_node, node *arg_info)
{
    node *inl_code;

    DBUG_ENTER ("FUN2LACassign");

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    inl_code = INFO_F2L_INLINED (arg_info);
    if (inl_code != NULL) {
        INFO_F2L_INLINED (arg_info) = NULL;

        arg_node = AppendAssign (inl_code, FreeNode (arg_node));

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

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *FUN2LACmodul( node *arg_node, node *arg_info)
 *
 * description:
 *   This function traverses all function definitions under a N_modul
 *   node.
 *
 ******************************************************************************/

node *
FUN2LACmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("FUN2LACmodul");

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);

        /*
         * After inlining the special LaC functions and removing the external
         * reference (N_ap node) of it, all LaC functions should be zombies
         * with (FUNDEF_USED == 0), now.
         */
        arg_node = RemoveAllZombies (arg_node);
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
