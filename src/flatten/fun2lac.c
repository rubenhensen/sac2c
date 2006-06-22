/*****************************************************************************
 *
 * $Id$
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
 *   After a large-scale re-organization of this phase, loop and cond
 *   functions are merely transformed into loops and conditionals while
 *   inlining is postponed to a separate application of function inlining.
 *
 *****************************************************************************/

#include "dbug.h"
#include "types.h"
#include "new_types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "internal_lib.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "print.h"
#include "globals.h"

/*
 * INFO structure
 */
struct INFO {
    int dummy;
};

/*
 * INFO macros
 */

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    DBUG_RETURN (info);
}

#ifndef DBUG_OFF

/******************************************************************************
 *
 * function:
 *   bool IsRecursiveCall( node *assign, node *fundef)
 *
 * description:
 *
 *
 ******************************************************************************/

static bool
IsRecursiveCall (node *assign, node *fundef)
{
    bool res;
    node *instr, *expr;

    DBUG_ENTER ("IsRecursiveCall");

    DBUG_ASSERT ((NODE_TYPE (assign) == N_assign), "illegal parameter found!");
    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef), "illegal parameter found!");

    instr = ASSIGN_INSTR (assign);

    if (NODE_TYPE (instr) == N_let) {
        expr = LET_EXPR (instr);

        if ((NODE_TYPE (expr) == N_ap) && (AP_FUNDEF (expr) == fundef)) {
            res = TRUE;
        } else {
            res = FALSE;
        }
    } else {
        res = FALSE;
    }

    DBUG_RETURN (res);
}

#endif

/******************************************************************************
 *
 * Function:
 *   bool ArgIsInternalArg( node *ext_arg, node *int_args)
 *
 * Description:
 *
 *
 ******************************************************************************/

static bool
ArgIsInternalArg (node *ext_arg, node *int_args)
{
    node *int_expr;
    bool found = FALSE;

    DBUG_ENTER ("ArgIsInternalArg");

    DBUG_ASSERT ((NODE_TYPE (ext_arg) == N_arg), "illegal parameter found!");

    if (int_args == NULL) {
        found = FALSE;
    } else {
        DBUG_ASSERT ((NODE_TYPE (int_args) == N_exprs), "illegal parameter found!");
        int_expr = EXPRS_EXPR (int_args);

        if ((NODE_TYPE (int_expr) == N_id) && (ID_DECL (int_expr) == ext_arg)) {
            found = TRUE;
        } else {
            found = ArgIsInternalArg (ext_arg, EXPRS_NEXT (int_args));
        }
    }

    DBUG_RETURN (found);
}

/******************************************************************************
 *
 * Function:
 *   bool ArgIsReturnValue( node *ext_arg, node *ret_args)
 *
 * Description:
 *
 *
 ******************************************************************************/

static bool
ArgIsReturnValue (node *ext_arg, node *ret_args)
{
    bool is_ret;

    DBUG_ENTER ("ArgIsReturnValue");

    DBUG_ASSERT ((NODE_TYPE (ext_arg) == N_arg), "illegal parameter found!");

    if (ret_args == NULL) {
        is_ret = FALSE;
    } else {
        DBUG_ASSERT ((NODE_TYPE (ret_args) == N_exprs), "illegal parameter found!");
        if (ID_DECL (EXPRS_EXPR (ret_args)) == ext_arg) {
            is_ret = TRUE;
        } else {
            is_ret = ArgIsReturnValue (ext_arg, EXPRS_NEXT (ret_args));
        }
    }

    DBUG_RETURN (is_ret);
}

/******************************************************************************
 *
 * Function:
 *   void BuildRenamingAssignsForDo( node **vardecs,
 *                                   node **ass1, node **ass2, node **ass3,
 *                                   node *ext_args, node *int_args,
 *                                   node *ret_args)
 *
 * Description:
 *   This function returns a N_vardec chain and three N_assign node chains.
 *
 *   If the names of the actual parameters found in the recursive call of a
 *   loop-function ('int_args') differ from the names of the formal parameters
 *   ('ext_args') appropriate renaming assignments are build:
 *
 *     ... Do( a_1, ...)                          tmp_a_1 = a_1;      // [1]
 *     {                                          ...
 *                                                do {
 *       <ass>                                      a_1 = tmp_a_1;    // [2]
 *       if (<cond>) {                              ...
 *         r_1, ... = Do( A_1, ...);     --->       <ass>
 *       }                                          tmp_a_1 = A_1;    // [3]
 *                                                  ...
 *       return( r_1, ...);                       }
 *     }                                          while (<cond>);
 *
 *   In some situations it is possible to insert just *one* assignment instead
 *   of three for a parameter at position 'i':
 *   If, for a given 'i' with (A_i != a_i), no one of the following conditions
 *   is hold, it is possible to assign (a_i = A_i) directly (at position [3]):
 *      [a]   exists j: ((j > i) && (A_j == a_i))
 *      [b]   exists j: (r_j == a_i)
 *   ([a] means, that a argument is shifted to a position with greater index;
 *    [b] means, that a parameter is a return value.)
 *
 *   Example:
 *
 *     ... Do( a, b, c, d, e, f)
 *     {
 *       <ass>
 *       if (<cond>) {
 *         d, f = Do( b, B, C, a, e, f);
 *       }
 *       return( c, f);
 *     }
 *
 *   --->
 *                                           (implemented)
 *     wrong:             correct:           better:            even better:
 *     ------             --------           -------            ------------
 *
 *                        tmp_a = a;         tmp_a = a;
 *                        tmp_b = b;
 *                        tmp_c = c;         tmp_c = c;         tmp_c = c;
 *                        tmp_d = d;
 *     do {               do {               do {               do {
 *                          a = tmp_a;         a = tmp_a;
 *                          b = tmp_b;
 *                          c = tmp_c;         c = tmp_c;         c = tmp_c;
 *                          d = tmp_d;
 *       <ass>              <ass>              <ass>              <ass>
 *                                                                tmp_a = a;
 *       a = b; // [a]      tmp_a = b;         tmp_a = b;         a = b;
 *       b = B;             tmp_b = B;         b = B;             b = B;
 *       c = C; // [b]      tmp_c = C;         tmp_c = C;         tmp_c = C;
 *       d = a;             tmp_d = a;         d = a;             d = tmp_a;
 *     }                  }                  }                  }
 *     while (<cond>);    while (<cond>);    while (<cond>);    while (<cond>);
 *     ... c ...          ... c ...          ... c ...          ... c ...
 *
 *  Even better solution would be (depicted in right-most row):
 *    Replace [a] by [a']: Insert two assignments if an argument is shifted to a
 *    position with greater index. Note that the first assignment (tmp_a = a) is
 *    needed only once if 'a' is used at multiple argument positions.
 *
 ******************************************************************************/

static void
BuildRenamingAssignsForDo (node **vardecs, node **ass1, node **ass2, node **ass3,
                           node *ext_args, node *int_args, node *ret_args)
{
    node *int_expr;
    char *new_name;

    DBUG_ENTER ("BuildRenamingAssignsForDo");

    DBUG_ASSERT (((vardecs != NULL)
                  && (((*vardecs) == NULL) || (NODE_TYPE ((*vardecs)) == N_vardec))),
                 "no vardecs found!");
    DBUG_ASSERT (((ass1 != NULL)
                  && (((*ass1) == NULL) || (NODE_TYPE ((*ass1)) == N_assign))),
                 "illegal parameter found!");
    DBUG_ASSERT (((ass2 != NULL)
                  && (((*ass2) == NULL) || (NODE_TYPE ((*ass2)) == N_assign))),
                 "illegal parameter found!");
    DBUG_ASSERT (((ass3 != NULL)
                  && (((*ass3) == NULL) || (NODE_TYPE ((*ass3)) == N_assign))),
                 "illegal parameter found!");
    DBUG_ASSERT (((ret_args == NULL) || (NODE_TYPE (ret_args) == N_exprs)),
                 "illegal parameter found!");

    if (ext_args != NULL) {
        BuildRenamingAssignsForDo (vardecs, ass1, ass2, ass3, ARG_NEXT (ext_args),
                                   EXPRS_NEXT (int_args), ret_args);

        DBUG_ASSERT ((int_args != NULL), "inconsistent LAC-signature found");
        DBUG_ASSERT ((NODE_TYPE (ext_args) == N_arg), "illegal parameter found!");
        DBUG_ASSERT ((NODE_TYPE (int_args) == N_exprs), "illegal parameter found!");

        int_expr = EXPRS_EXPR (int_args);

        DBUG_ASSERT ((ID_DECL (int_expr) != NULL), "vardec not found!");

        if ((NODE_TYPE (int_expr) != N_id) || (ext_args != ID_DECL (int_expr))) {
            if (ArgIsReturnValue (ext_args, ret_args)
                || ArgIsInternalArg (ext_args, int_args)) {
                /*
                 * build a fresh vardec (tmp_a_i)
                 */
                new_name = ILIBtmpVarName (ARG_NAME (ext_args));
                (*vardecs) = TBmakeVardec (TBmakeAvis (new_name, TYcopyType (AVIS_TYPE (
                                                                   ARG_AVIS (ext_args)))),
                                           *vardecs);
                VARDEC_TYPE ((*vardecs)) = DUPdupAllTypes (ARG_TYPE (ext_args));

                /*
                 * tmp_a_i = a_i;
                 */
                *ass1 = TBmakeAssign (TBmakeLet (TBmakeIds (DECL_AVIS (*vardecs), NULL),
                                                 TBmakeId (ARG_AVIS (ext_args))),
                                      *ass1);

                /*
                 * a_i = tmp_a_i;
                 */
                *ass2 = TBmakeAssign (TBmakeLet (TBmakeIds (DECL_AVIS (ext_args), NULL),
                                                 TBmakeId (VARDEC_AVIS (*vardecs))),
                                      *ass2);

                /*
                 * tmp_a_i = A_i;
                 */
                *ass3 = TBmakeAssign (TBmakeLet (TBmakeIds (DECL_AVIS (*vardecs), NULL),
                                                 DUPdoDupNode (int_expr)),
                                      *ass3);
            } else {
                /*
                 * a_i = A_i;
                 */
                *ass3 = TBmakeAssign (TBmakeLet (TBmakeIds (DECL_AVIS (ext_args), NULL),
                                                 DUPdoDupNode (int_expr)),
                                      *ass3);
            }
        }
    } else {
        DBUG_ASSERT ((int_args == NULL), "inconsistent LAC-signature found");
    }

    DBUG_VOID_RETURN;
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
ReturnVarsAreIdentical (node *ext_rets, node *int_rets)
{
    bool ok = TRUE;

    DBUG_ENTER ("ReturnVarsAreIdentical");

    DBUG_ASSERT (((ext_rets == NULL) || (NODE_TYPE (ext_rets) == N_exprs)),
                 "illegal parameter found!");

    DBUG_ASSERT ((TCcountExprs (ext_rets) == TCcountIds (int_rets)),
                 "inconsistent LAC-signature found");

    while (ext_rets != NULL) {
        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (ext_rets)) == N_id),
                     "return value of special LaC function must be a N_id node!");
        DBUG_ASSERT (((ID_DECL (EXPRS_EXPR (ext_rets)) != NULL)
                      && (IDS_DECL (int_rets) != NULL)),
                     "vardec not found!");

        if (ID_DECL (EXPRS_EXPR (ext_rets)) != IDS_DECL (int_rets)) {
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
 *       tmp_a_i = a_i;
 *       do {
 *         a_i = tmp_a_i;
 *         <ass>
 *         tmp_a_i = A_i;
 *       } while (<pred>);
 *
 *       return( r_1, r_2, ...);
 *     }
 *
 *   In some situations it is possible to insert just *one* assignment instead
 *   of three for a parameter at position 'i' (see comment in the header of
 *   BuildRenamingAssignsForDo())
 *
 *
 *   To prepare correct inlining of the function, the type of the concrete
 *   parameters to the loop function must be downgraded to the type
 *   of the formal loop arguments.
 *
 *
 *   Important: The following will only happen AFTER EMRefcounting!
 *              EMReferenceCounting inserts adjust_rc-operations
 *              in both, THEN- and ELSE-Branch of the conditional
 *              These operations are extracted from the conditional
 *              before transformation starts. Later, they are reintroduced
 *              Into the do-loop in the following way:
 *
 *     ... DoLoopFun( args)
 *     {
 *       <ass>
 *       if (<pred>) {
 *         dec_rc( B\A);
 *         res_1 = DoLoopFun( A );
 *       }
 *       else {
 *         dec_rc( A\B);
 *       }
 *       return( B);
 *     }
 *
 *     will be transformed into:
 *
 *     goto LABEL:
 *     do
 *     {
 *       dec_rc(B\A );
 *     LABEL:
 *       <ass>;
 *     }
 *     while(<pred>);
 *     dec_rc(A/B );
 *
 *     The string LABEL is stored in DO_LABEL, while the block containing
 *     dec_rc(B/A ) is stores in DO_SKIP
 *
 *     !!!!! There is no lac2fun transformation for this (yet) !!!!!
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
    node *ass1 = NULL;
    node *ass2 = NULL;
    node *ass3 = NULL;
    node *skip = NULL;
    node *epilogue = NULL;

    DBUG_ENTER ("TransformIntoDoLoop");

    assigns = FUNDEF_INSTR (fundef);

    if (NODE_TYPE (ASSIGN_INSTR (assigns)) == N_cond) {
        cond_assign = assigns;
        loop_body = NULL;
    } else {
        tmp = assigns;
        while ((NODE_TYPE (ASSIGN_INSTR (ASSIGN_NEXT (tmp))) != N_cond)
               && (NODE_TYPE (ASSIGN_INSTR (ASSIGN_NEXT (tmp))) != N_return)) {
            tmp = ASSIGN_NEXT (tmp);
            DBUG_ASSERT ((tmp != NULL), "recursive call of do-loop function not found");
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
        if (global.compiler_phase >= PH_memory) {
            /*
             * Adjust_RC Operations in the THEN-Tree must be moved
             * in to the Do-Loops SKIP-Block
             */
            while (
              (BLOCK_INSTR (COND_THEN (cond)) != NULL)
              && (NODE_TYPE (BLOCK_INSTR (COND_THEN (cond))) == N_assign)
              && (NODE_TYPE (ASSIGN_RHS (BLOCK_INSTR (COND_THEN (cond)))) == N_prf)
              && ((PRF_PRF (ASSIGN_RHS (BLOCK_INSTR (COND_THEN (cond)))) == F_inc_rc)
                  || (PRF_PRF (ASSIGN_RHS (BLOCK_INSTR (COND_THEN (cond)))) == F_dec_rc)
                  || (PRF_PRF (ASSIGN_RHS (BLOCK_INSTR (COND_THEN (cond)))) == F_free))) {
                tmp = ASSIGN_NEXT (BLOCK_INSTR (COND_THEN (cond)));
                ASSIGN_NEXT (BLOCK_INSTR (COND_THEN (cond))) = NULL;
                skip = TCappendAssign (skip, BLOCK_INSTR (COND_THEN (cond)));
                BLOCK_INSTR (COND_THEN (cond)) = tmp;
            }

            /*
             * Adjust_RC Operations in the ELSE-Tree must be moved
             * after the do-loop, they are put into epilogue
             */
            while (
              (BLOCK_INSTR (COND_ELSE (cond)) != NULL)
              && (NODE_TYPE (BLOCK_INSTR (COND_ELSE (cond))) == N_assign)
              && (NODE_TYPE (ASSIGN_RHS (BLOCK_INSTR (COND_ELSE (cond)))) == N_prf)
              && ((PRF_PRF (ASSIGN_RHS (BLOCK_INSTR (COND_ELSE (cond)))) == F_inc_rc)
                  || (PRF_PRF (ASSIGN_RHS (BLOCK_INSTR (COND_ELSE (cond)))) == F_dec_rc)
                  || (PRF_PRF (ASSIGN_RHS (BLOCK_INSTR (COND_ELSE (cond)))) == F_free))) {
                tmp = ASSIGN_NEXT (BLOCK_INSTR (COND_ELSE (cond)));
                ASSIGN_NEXT (BLOCK_INSTR (COND_ELSE (cond))) = NULL;
                epilogue = TCappendAssign (epilogue, BLOCK_INSTR (COND_ELSE (cond)));
                BLOCK_INSTR (COND_ELSE (cond)) = tmp;
            }

            if (BLOCK_INSTR (COND_ELSE (cond)) == NULL)
                BLOCK_INSTR (COND_ELSE (cond)) = TBmakeEmpty ();
        }

        ret = ASSIGN_INSTR (ASSIGN_NEXT (cond_assign));

        DBUG_ASSERT ((NODE_TYPE (cond) == N_cond),
                     "Illegal node type in conditional position.");
        DBUG_ASSERT ((NODE_TYPE (BLOCK_INSTR (COND_ELSE (cond))) == N_empty),
                     "else part of conditional in do-loop must be empty");
        DBUG_ASSERT ((NODE_TYPE (ret) == N_return),
                     "after conditional in do-loop funtion no assignments are"
                     " allowed");

        loop_pred = COND_COND (cond);
        COND_COND (cond) = TBmakeBool (FALSE);

        /*
         * cond_assign: if (false) { ... = DoLoopFun(...); }  return(...);
         * cond:        if (false) { ... = DoLoopFun(...); }
         * ret:         return(...);
         * loop_pred:   <pred>
         * loop_body:   <ass>
         */

        int_call = BLOCK_INSTR (COND_THEN (cond));
        while ((int_call != NULL)
               && (NODE_TYPE (ASSIGN_INSTR (int_call)) == N_annotate)) {
            int_call = ASSIGN_NEXT (int_call);
        }
        DBUG_ASSERT ((IsRecursiveCall (int_call, fundef)),
                     "recursive call of do-loop function must be the only"
                     " assignment in the conditional");

        DBUG_ASSERT ((ReturnVarsAreIdentical (RETURN_EXPRS (ret), ASSIGN_LHS (int_call))),
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

        BuildRenamingAssignsForDo (&(FUNDEF_VARDEC (fundef)), &ass1, &ass2, &ass3,
                                   FUNDEF_ARGS (fundef), AP_ARGS (ASSIGN_RHS (int_call)),
                                   RETURN_EXPRS (ret));

        loop_body = TCappendAssign (ass2, TCappendAssign (loop_body, ass3));

        if (loop_body == NULL) {
            loop_body = TBmakeEmpty ();
        }
        loop_body = TBmakeBlock (loop_body, NULL);

        /*
         * cond_assign: if (false) { ... = DoLoopFun(...); }  return(...);
         * cond:        if (false) { ... = DoLoopFun(...); }
         * ret:         return(...);
         * loop_pred:   <pred>
         * loop_body:   { a_i = tmp_a_i; <ass> tmp_a_i = A_i; }
         * int_call:    ... = DoLoopFun(...);
         */

        /* replace cond by do-loop */
        ASSIGN_INSTR (cond_assign) = FREEdoFreeTree (ASSIGN_INSTR (cond_assign));
        ASSIGN_INSTR (cond_assign) = TBmakeDo (loop_pred, loop_body);

        if (skip == NULL) {
            skip = TBmakeEmpty ();
        }
        DO_SKIP (ASSIGN_INSTR (cond_assign)) = TBmakeBlock (skip, NULL);
        DO_LABEL (ASSIGN_INSTR (cond_assign)) = ILIBtmpVar ();
    }

    /* Insert epilogue */
    epilogue = TCappendAssign (epilogue, ASSIGN_NEXT (cond_assign));
    ASSIGN_NEXT (cond_assign) = epilogue;

    FUNDEF_INSTR (fundef) = TCappendAssign (ass1, cond_assign);

    FUNDEF_ISDOFUN (fundef) = FALSE;
    FUNDEF_ISLACINLINE (fundef) = TRUE;

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

    FUNDEF_ISCONDFUN (fundef) = FALSE;
    FUNDEF_ISLACINLINE (fundef) = TRUE;

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   node *FUN2LACfundef( node *arg_node, info *arg_info)
 *
 * description:
 *   This function traverses the function body of any function that does NOT
 *   represent an abstracted conditional or loop.
 *
 ******************************************************************************/

node *
F2Lfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("F2Lfundef");

    if (FUNDEF_ISDOFUN (arg_node)) {
        arg_node = TransformIntoDoLoop (arg_node);
    } else if (FUNDEF_ISCONDFUN (arg_node)) {
        arg_node = TransformIntoCond (arg_node);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *FUN2LACmodul( node *arg_node, info *arg_info)
 *
 * description:
 *   This function traverses all function definitions under a N_modul
 *   node.
 *
 ******************************************************************************/

node *
F2Lmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FUN2LACmodul");

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
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
F2LdoFun2Lac (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("F2LdoFun2Lac");

    info = MakeInfo ();

    TRAVpush (TR_f2l);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
