#include "multi_operator_set_expressions.h"
#include "traverse.h"

#define DBUG_PREFIX "MOSE"
#include "debug.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "memory.h"
#include "str.h"

/**
 * @file multi_operator_set_expressions.c
 *
 * This traversal has two tasks:
 * 1) It identifies set expressions that are supposed to return more or less
 *    values than exactly one, and
 * 2) It changes SETWL_EXPR from being just an expression into being a chain
 *    of N_exprs nodes of identifiers that are potentially preceded by 
 *    assignments in SETWL_ASSIGN.
 *
 * The key challenge is that this phase is called prior to flattening.
 * Since it is not known what return-arity function symbols have, we need
 * to use the number of LHS variables as tell-tale for the expected number
 * of results for a set expression. For example, we re-write
 *
 *   a, b = {[i] -> foo (i) | [i] < [10]};    into
 *
 *   a, b = {[i] -> let
 *                     t1, t2 = foo (i);
 *                  in (t1, t2)  | [i] < [10]};
 *
 *   NB: the let-notation is NOT supported in general, we just use it to
 *   show the code that sits in SETWL_ASSIGNS.
 *
 * Similarly, we re-write
 *
 *   a = {[i] -> foo (i) | [i] < [10]};    into
 *
 *   a = {[i] -> let
 *                  t1 = foo (i);
 *               in (t1)  | [i] < [10]};
 *
 * and
 *
 *   {[i] -> foo (i) | [i] < [10]};    into
 *
 *   {[i] -> let
 *              foo (i);
 *           in ()  | [i] < [10]};
 *
 * The latter makes sense for side-effecting functions such as print.
 *
 * The central question here is: can we always expect an explicit 
 * assignment to exist? or can we put a set-exprssion anywhere else in
 * expression position, where we do expect more than one expression?
 * Luckily, ATM (2024), the ONLY place where this is possible is the
 * body of a set expression itself! And there, we always expect the 
 * number of expressions that is required by the surrounding set-
 * expression!
 * In all other cases, we always expect exactly 1 return value!
 *
 * This allows for a straight forward implementation, where we 
 * keep the number of expected return values in INFO_MOSE_NUM_EXPRS.
 * We modify this iff we find an N_let with an N_setwl on the RHS,
 * and set it back as soon as we return from the RHS traversal.
 */

/* INFO structure */
struct INFO {
    int num_exprs;
};

/* access macros */
#define INFO_MOSE_NUM_EXPRS(n) ((n)->num_exprs)

/**
 * builds an info structure.
 *
 * @return new info structure
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_MOSE_NUM_EXPRS (result) = 1;

    DBUG_RETURN (result);
}

/**
 * frees an info structure.
 *
 * @param info the info structure to free
 */
static info *
FreeInfo (info *arg_info)
{
    DBUG_ENTER ();

    arg_info = MEMfree (arg_info);

    DBUG_RETURN (arg_info);
}


/**
 * hook to start the handle dots traversal of the AST.
 *
 * @param arg_node current AST
 * @result transformed AST without dots and dot constructs
 */
node *
MOSEdoMultiOperatorSetExpressions (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    TRAVpush (TR_mose);

    arg_node = TRAVdo (arg_node, arg_info);

    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    CTIabortOnError ();

    DBUG_RETURN (arg_node);
}


/*
 * Count the LHS iff the RHS is an N_setwl.
 * Put the finding into arg_info.
 *
 * @param arg_node current node of the AST
 * @param arg_info info node
 * @return current node of the AST
 */
node *
MOSElet (node *arg_node, info *arg_info)
{
    int old_count;
    DBUG_ENTER ();

    old_count = INFO_MOSE_NUM_EXPRS (arg_info);
    if (NODE_TYPE (LET_EXPR (arg_node)) == N_setwl) {
        INFO_MOSE_NUM_EXPRS (arg_info) = TCcountSpids (LET_IDS (arg_node));
        DBUG_PRINT ("expecting %d returns from set-expression in line %zu",
                    INFO_MOSE_NUM_EXPRS (arg_info), NODE_LINE (arg_node));
    }
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    INFO_MOSE_NUM_EXPRS (arg_info) = old_count;
    
    DBUG_RETURN (arg_node);
}


/**
 * @param arg_node current node of the ast
 * @param arg_info info node
 * @return transformed AST
 */
node *
MOSEsetwl (node *arg_node, info *arg_info)
{
    int i;
    int count;
    char *tmp;
    node *exprs;
    node *spids;
    DBUG_ENTER ();

    DBUG_ASSERT (SETWL_ASSIGNS (arg_node) == NULL, "expected NULL SETWL_ASSIGNS!");

    /*
     * No need to traverse SETWL_VEC as there can be no TCs!
     */
    /*
     * There can be TCs in the generator. However, these can only be in a context
     * that expects exactly 1 value.
     */
    count = INFO_MOSE_NUM_EXPRS (arg_info);
    INFO_MOSE_NUM_EXPRS (arg_info) = 1;
    SETWL_GENERATOR (arg_node) = TRAVdo (SETWL_GENERATOR (arg_node), arg_info);
    INFO_MOSE_NUM_EXPRS (arg_info) = count;

    /* 
     * As mentioned above, for nested set-expressions we need to carry over the 
     * expected number of return values, for example:
     *    a, b = { [i] -> {[j] -> foo(2,j) | [j] <[5] } | [i] < [10] };
     * turns into:
     *     a, b = { [i] -> let
     *                       t1, t2 = { [j] -> let
     *                                           t3, t4 = foo( 2, j)
     *                                         in (t3, t4) | [j] < [ 5 ] };
     *                     in (t1, t2) | [i] < [ 10 ] };
     * Consequently, we traverse the expression keeping the INFO_MOSE_NUM_EXPRS
     * identical to the actual one!
     */
    SETWL_EXPR (arg_node) = TRAVdo (SETWL_EXPR (arg_node), arg_info);

    /*
     * Finally, we standardise SETWL_EXPR into an N_exprs chain and add 
     * SETWL_ASSIGNS if needed.
     */
    exprs = NULL;
    spids = NULL;
    for (i=0; i< INFO_MOSE_NUM_EXPRS (arg_info); i++) {
        tmp = TRAVtmpVar ();
        exprs = TBmakeExprs ( TBmakeSpid (NULL, tmp), exprs);
        spids = TBmakeSpids ( STRcpy (tmp), spids);
    }
    SETWL_ASSIGNS (arg_node)
        = TBmakeAssign (TBmakeLet (spids, SETWL_EXPR (arg_node)), NULL);
    SETWL_EXPR (arg_node) = exprs;

    SETWL_NEXT (arg_node) = TRAVopt (SETWL_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
