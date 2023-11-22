#include <stdio.h>

#include "globals.h"

#define DBUG_PREFIX "FLATTEN"
#include "debug.h"

#include "types.h"
#include "str.h"
#include "memory.h"
#include "tree_basic.h"
#include "node_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "ctinfo.h"
#include "handle_mops.h"
#include "while2do.h"
#include "handle_condexpr.h"
#include "namespaces.h"

#include "flatten.h"

/*
 * OPEN PROBLEMS:
 *
 * I) not yet solved:
 *
 * II) to be fixed here:
 *
 * III) to be fixed somewhere else:
 */

/*
 * arg_info in this file:
 * CONTEXT    : context of this arg_node, e.g. within a condition. FltnExprs
 *              decides to abstract or not abstract the node.
 *              legal values are (enum type contextflag):
 *                CT_normal,
 *                CT_ap,
 *                CT_array,
 *                CT_return,
 *
 * LASTASSIGN:  Every FltnAssign replaces node[0] with arg_node so that other
 *              functions may place instructions IN FRONT of that assignment.
 *              FltnAssign returns this node[0]
 *
 * FINALASSIGN: Every FltnBlock resets node[2] to NULL.
 *              Every FltnAssign replaces node[2] with arg_node if
 *              ASSIGN_NEXT(arg_node) equals NULL.
 *
 */

/**
 * INFO structure
 */
struct INFO {
    int context;
    node *lastassign;
    node *lastwlblock;
    node *finalassign;
};

/**
 * INFO macros
 */
#define INFO_FLAT_CONTEXT(n) (n->context)
#define INFO_FLAT_LASTASSIGN(n) (n->lastassign)
#define INFO_FLAT_FINALASSIGN(n) (n->finalassign)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FLAT_CONTEXT (result) = 0;
    INFO_FLAT_LASTASSIGN (result) = NULL;
    INFO_FLAT_FINALASSIGN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/*
 * NODE_IS_ID is a helper macro which returns true if the corresponding
 * node is an identifier, i.e. an N_id node or an N_spid node
 */
#define NODE_IS_ID(n) ((NODE_TYPE (n) == N_id) || (NODE_TYPE (n) == N_spid))

/******************************************************************************
 *
 * function:
 *  node *Abstract( node *arg_node, info *arg_info)
 *
 * description:
 *   - gets an expression <expr> to be abstracted out as argument
 *   - creates an assignment of the form:
 *        __flat_<n> = <expr>;
 *     end prepands it to LASTASSIGN from arg_info
 *   - returns a freshly created N_id node holding  __flat_<n>
 *
 ******************************************************************************/

static node *
Abstract (node *arg_node, info *arg_info)
{
    char *tmp;
    node *res, *ids;

    DBUG_ENTER ();

    tmp = TRAVtmpVar ();
    ids = TBmakeSpids (STRcpy (tmp), NULL);

    INFO_FLAT_LASTASSIGN (arg_info)
      = TBmakeAssign (TBmakeLet (ids, arg_node), INFO_FLAT_LASTASSIGN (arg_info));

    DBUG_PRINT ("node %p inserted before %p", (void *)INFO_FLAT_LASTASSIGN (arg_info),
                (void *)ASSIGN_NEXT (INFO_FLAT_LASTASSIGN (arg_info)));

    res = TBmakeSpid (NULL, tmp);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *  node *FLATdoFlatten(node *arg_node)
 *
 * description:
 *   eliminates nested function applications:
 *
 *     a = f( a+b, c);         =>   __flat_<n> = a+b;
 *                                  a          = f( __flat_<n>, c);
 *
 ******************************************************************************/

node *
FLATdoFlatten (node *arg_node)
{
    info *info_node;

    DBUG_ENTER ();

    info_node = MakeInfo ();
    INFO_FLAT_CONTEXT (info_node) = CT_normal;

    TRAVpush (TR_flat);
    arg_node = TRAVdo (arg_node, info_node);
    TRAVpop ();

    info_node = FreeInfo (info_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FLATmodule(node *arg_node, info *arg_info)
 *
 * description:
 *   this function is needed to limit the traversal to the FUNS-son of
 *   N_modul!
 *
 ******************************************************************************/

node *
FLATmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt(MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FLATfundef(node *arg_node, info *arg_info)
 *
 * description:
 *   - calls TRAVdo to flatten the user defined functions if function body is not
 *   empty and resets tos after flatten of the function's body.
 *   - the formal parameters of a function will be traversed to put their names
 *   on the stack!
 *
 ******************************************************************************/

node *
FLATfundef (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    /*
     * Do not flatten imported functions. These functions have already been
     * flattened and if this is done again there may arise name clashes.
     * A new temp variable __flat42 may conflict with __flat42 which was
     * inserted in the first flatten phase (module compiliation).
     * Furthermore, imported code contains IDS nodes instead of SPIDS nodes!
     * This may lead to problems when this traversal is run.
     */
    if ((FUNDEF_BODY (arg_node) != NULL) && !FUNDEF_WASIMPORTED (arg_node)
        && FUNDEF_ISLOCAL (arg_node)) {
        DBUG_PRINT ("flattening function %s:", FUNDEF_NAME (arg_node));
        FUNDEF_ARGS (arg_node) = TRAVopt(FUNDEF_ARGS (arg_node), arg_info);
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    /*
     * Proceed with the next function...
     */
    FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FLATblock(node *arg_node, info *arg_info)
 *
 * @brief does traverse the body only; no vardecs!
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/

node *
FLATblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (BLOCK_ASSIGNS (arg_node) != NULL) {
        BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);
    } else {
        INFO_FLAT_FINALASSIGN (arg_info) = NULL; /* fixes bug 919!! */
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FLATassign(node *arg_node, info *arg_info)
 *
 * @brief sets INFO_FLAT_LASTASSIGN to this one,
 *        sets INFO_FLAT_FINALASSIGN to this one IFF (ASSIGN_NEXT == NULL)
 *        During traversal of the INSTR, new assignments can be prepanded
 *        to INFO_FLAT_LASTASSIGN. These are inserted into the tree by this
 *        function returning INFO_FLAT_LASTASSIGN!
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/

node *
FLATassign (node *arg_node, info *arg_info)
{
    node *mem_last_assign, *return_node;

    DBUG_ENTER ();

    mem_last_assign = INFO_FLAT_LASTASSIGN (arg_info);
    INFO_FLAT_LASTASSIGN (arg_info) = arg_node;
    DBUG_PRINT ("LASTASSIGN set to %p!", (void *)arg_node);

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    /*
     * newly inserted abstractions are prepanded in front of
     * INFO_FLAT_LASTASSIGN(arg_info). To properly insert these nodes,
     * that pointer has to be returned:
     */
    return_node = INFO_FLAT_LASTASSIGN (arg_info);

    if (return_node != arg_node) {
        DBUG_PRINT ("node %p will be inserted instead of %p",
                    (void *)return_node,
                    (void *)arg_node);
    }
    INFO_FLAT_LASTASSIGN (arg_info) = mem_last_assign;
    DBUG_PRINT ("LASTASSIGN (re)set to %p!", (void *)mem_last_assign);

    if (ASSIGN_NEXT (arg_node)) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    } else {
        INFO_FLAT_FINALASSIGN (arg_info) = arg_node;
        DBUG_PRINT ("FINALASSIGN set to %p!", (void *)arg_node);
    }

    DBUG_RETURN (return_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FLATcast(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/

node *
FLATcast (node *arg_node, info *arg_info)
{
    node *expr;

    DBUG_ENTER ();

    expr = CAST_EXPR (arg_node);
    if (NODE_TYPE (expr) != N_spid) {
        CAST_EXPR (arg_node) = Abstract (expr, arg_info);
        expr = TRAVdo (expr, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FLATarray(node *arg_node, info *arg_info)
 *
 * @brief set the context-flag of arg_info to CT_array, traverse the exprs.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/

node *
FLATarray (node *arg_node, info *arg_info)
{
    contextflag old_context;

    DBUG_ENTER ();

    if (ARRAY_AELEMS (arg_node) != NULL) {
        old_context = (contextflag)INFO_FLAT_CONTEXT (arg_info);
        INFO_FLAT_CONTEXT (arg_info) = CT_array;
        ARRAY_AELEMS (arg_node) = TRAVdo (ARRAY_AELEMS (arg_node), arg_info);
        INFO_FLAT_CONTEXT (arg_info) = old_context;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FLATspap(node *arg_node, info *arg_info)
 *
 * @brief set the context-flag of arg_info to CT_ap, traverse the args.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/

node *
FLATspap (node *arg_node, info *arg_info)
{
    contextflag old_ctxt;

    DBUG_ENTER ();

    DBUG_PRINT ("flattening application of %s:", SPAP_NAME (arg_node));

    if (SPAP_ARGS (arg_node) != NULL) {
        old_ctxt = (contextflag)INFO_FLAT_CONTEXT (arg_info);
        INFO_FLAT_CONTEXT (arg_info) = CT_ap;
        SPAP_ARGS (arg_node) = TRAVdo (SPAP_ARGS (arg_node), arg_info);
        INFO_FLAT_CONTEXT (arg_info) = old_ctxt;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FLATprf(node *arg_node, info *arg_info)
 *
 * @brief set the context-flag of arg_info to CT_ap, traverse the args.
 *  - It is important that all arguments are abstracted out as e.g.
 *    dim(0) cannot be evaluted by the implementation of the dim-prf.
 *  - ConstVarPropagation can de-flatten arguments when appropriate.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/

node *
FLATprf (node *arg_node, info *arg_info)
{
    contextflag old_ctxt;

    DBUG_ENTER ();

    DBUG_PRINT ("flattening application of %s:", global.prf_name[PRF_PRF (arg_node)]);

    if (PRF_ARGS (arg_node) != NULL) {
        old_ctxt = (contextflag)INFO_FLAT_CONTEXT (arg_info);
        INFO_FLAT_CONTEXT (arg_info) = CT_ap;
        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
        INFO_FLAT_CONTEXT (arg_info) = old_ctxt;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FLATreturn(node *arg_node, info *arg_info)
 *
 * @brief set the context-flag of arg_info to CT_return, traverse the exprs.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/

node *
FLATreturn (node *arg_node, info *arg_info)
{
    contextflag old_ctxt;

    DBUG_ENTER ();

    if (RETURN_EXPRS (arg_node) != NULL) {
        old_ctxt = (contextflag)INFO_FLAT_CONTEXT (arg_info);
        INFO_FLAT_CONTEXT (arg_info) = CT_return;
        RETURN_EXPRS (arg_node) = TRAVdo (RETURN_EXPRS (arg_node), arg_info);
        INFO_FLAT_CONTEXT (arg_info) = old_ctxt;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FltnExprs( node *arg_node, info *arg_info)
 *
 * description:
 *  - flattens all the exprs depending on the context INFO_FLAT_CONTEXT
 *  - collecting constant integer elements
 *
 ******************************************************************************/

node *
FLATexprs (node *arg_node, info *arg_info)
{
    bool abstract;
    node *expr, *expr2;

    DBUG_ENTER ();

    expr = EXPRS_EXPR (arg_node);

    /*
     * compute whether to abstract <expr> or not , depending on the
     * context of the expression, given by INFO_FLAT_CONTEXT( arg_info)
     */
    switch (INFO_FLAT_CONTEXT (arg_info)) {
    case CT_ap:
        abstract
          = ((NODE_TYPE (expr) == N_numbyte) || (NODE_TYPE (expr) == N_numshort)
             || (NODE_TYPE (expr) == N_numint) || (NODE_TYPE (expr) == N_numlong)
             || (NODE_TYPE (expr) == N_numlonglong) || (NODE_TYPE (expr) == N_numubyte)
             || (NODE_TYPE (expr) == N_numushort) || (NODE_TYPE (expr) == N_numuint)
             || (NODE_TYPE (expr) == N_numulong) || (NODE_TYPE (expr) == N_numulonglong)
             || (NODE_TYPE (expr) == N_num) || (NODE_TYPE (expr) == N_float)
             || (NODE_TYPE (expr) == N_floatvec) || (NODE_TYPE (expr) == N_double)
             || (NODE_TYPE (expr) == N_bool) || (NODE_TYPE (expr) == N_char)
             || (NODE_TYPE (expr) == N_str) || (NODE_TYPE (expr) == N_array)
             || (NODE_TYPE (expr) == N_spap) || (NODE_TYPE (expr) == N_prf)
             || (NODE_TYPE (expr) == N_with) || (NODE_TYPE (expr) == N_cast)
             || (NODE_TYPE (expr) == N_nested_init));
        break;
    case CT_array:
    case CT_return:
        abstract
          = ((NODE_TYPE (expr) == N_numbyte) || (NODE_TYPE (expr) == N_numshort)
             || (NODE_TYPE (expr) == N_numint) || (NODE_TYPE (expr) == N_numlong)
             || (NODE_TYPE (expr) == N_numlonglong) || (NODE_TYPE (expr) == N_numubyte)
             || (NODE_TYPE (expr) == N_numushort) || (NODE_TYPE (expr) == N_numuint)
             || (NODE_TYPE (expr) == N_numulong) || (NODE_TYPE (expr) == N_numulonglong)
             || (NODE_TYPE (expr) == N_num) || (NODE_TYPE (expr) == N_float)
             || (NODE_TYPE (expr) == N_floatvec) || (NODE_TYPE (expr) == N_double)
             || (NODE_TYPE (expr) == N_bool) || (NODE_TYPE (expr) == N_char)
             || (NODE_TYPE (expr) == N_str) || (NODE_TYPE (expr) == N_array)
             || (NODE_TYPE (expr) == N_spap) || (NODE_TYPE (expr) == N_prf)
             || (NODE_TYPE (expr) == N_with) || (NODE_TYPE (expr) == N_cast));
        break;
    case CT_normal:
        abstract = ((NODE_TYPE (expr) == N_spap) || (NODE_TYPE (expr) == N_prf)
                    || (NODE_TYPE (expr) == N_with) || (NODE_TYPE (expr) == N_cast));
        break;
#ifdef FIXME
    case CT_array:
        abstract = ((NODE_TYPE (expr) == N_str) || (NODE_TYPE (expr) == N_array)
                    || (NODE_TYPE (expr) == N_spap) || (NODE_TYPE (expr) == N_prf)
                    || (NODE_TYPE (expr) == N_with) || (NODE_TYPE (expr) == N_cast));
        break;
#endif // FIXME
    default:
        DBUG_UNREACHABLE ("illegal context !");
        /* the following assignment is used only for convincing the C compiler
         * that abstract will be initialized in any case!
         */
        abstract = 0;
    }

    DBUG_PRINT ("context: %s, abstract: %s, expr: %s",
                (INFO_FLAT_CONTEXT (arg_info) == CT_normal
                   ? "CT_normal"
                   : (INFO_FLAT_CONTEXT (arg_info) == CT_ap
                        ? "CT_ap"
                        : (INFO_FLAT_CONTEXT (arg_info) == CT_array
                             ? "CT_array"
                             : (INFO_FLAT_CONTEXT (arg_info) == CT_return
                                  ? "CT_return"
                                  : "unknown!!")))),
                (abstract ? "yes" : "no"), NODE_TEXT (expr));

    /*
     * if this is to be abstracted, we abstract and potentially annotate constant
     * arrays in the freshly generated N_id node.
     */
    if (abstract) {
        /*
         *  if there are type casts we need to abstract them too.
         *  if we leave them, empty arrays will be left uncasted and thus untyped.
         */
        EXPRS_EXPR (arg_node) = Abstract (expr, arg_info);
        expr2 = TRAVdo (expr, arg_info);
    } else {
        expr2 = TRAVdo (expr, arg_info);
    }

    DBUG_ASSERT (expr == expr2,
                 "return-node differs from arg_node while flattening an expr!");

    /*
     * Last but not least remaining exprs have to be done:
     */
    EXPRS_NEXT (arg_node) = TRAVopt(EXPRS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FLATcond(node *arg_node, info *arg_info)
 *
 * description:
 *   - flattens the predicate and both alternatives.
 *
 ******************************************************************************/

node *
FLATcond (node *arg_node, info *arg_info)
{
    node *pred, *pred2, *mem_last_assign;

    DBUG_ENTER ();

    pred = COND_COND (arg_node);
    if (NODE_TYPE (pred) != N_spid) {
        COND_COND (arg_node) = Abstract (pred, arg_info);
    }

    pred2 = TRAVdo (pred, arg_info);
    DBUG_ASSERT (pred == pred2,
                 "return-node differs from arg_node while flattening an expr!");

    mem_last_assign = INFO_FLAT_LASTASSIGN (arg_info);

    COND_THEN (arg_node) = TRAVopt(COND_THEN (arg_node), arg_info);

    COND_ELSE (arg_node) = TRAVopt(COND_ELSE (arg_node), arg_info);

    INFO_FLAT_LASTASSIGN (arg_info) = mem_last_assign;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FLATdo(node *arg_node, info *arg_info)
 *
 * description:
 *  - traverse the body
 *  - leave all entries on the stack since the loop will be executed
 *    at least once!
 *  - if the predicate has to be flattened out, insert the new assignment
 *    at the end of the do-loop body.
 *  - Anyway, invoke flatten on the condition => renaming!
 *
 ******************************************************************************/

node *
FLATdo (node *arg_node, info *arg_info)
{
    node *mem_last_assign, *final_assign, *pred, *pred2;

    DBUG_ENTER ();

    mem_last_assign = INFO_FLAT_LASTASSIGN (arg_info);

    /*
     * First, we traverse the body of the while-loop.
     * This guarantees that INFO_FLAT_FINALASSIGN( arg_info)
     * will be set to the last N_assign in the body of the loop
     * which may be required for inserting assignments that
     * flatten the break condition!
     */
    DO_BODY (arg_node) = TRAVdo (DO_BODY (arg_node), arg_info);
    final_assign = INFO_FLAT_FINALASSIGN (arg_info);
    INFO_FLAT_FINALASSIGN (arg_info) = NULL;

    pred = DO_COND (arg_node);
    if (NODE_TYPE (pred) != N_spid) {
        /*
         * abstract the condition out and insert it at the end of the do-loop:
         */
        INFO_FLAT_LASTASSIGN (arg_info) = NULL;
        DO_COND (arg_node) = Abstract (pred, arg_info);
    } else {
        INFO_FLAT_LASTASSIGN (arg_info) = NULL;
    }
    pred2 = TRAVdo (pred, arg_info);
    DBUG_ASSERT (pred == pred2,
                 "return-node differs from arg_node while flattening an expr!");
    if (final_assign == NULL) {
        DBUG_ASSERT (DO_ASSIGNS (arg_node) == NULL,
                     "INFO_FLAT_FINALASSIGN is NULL although do-body is non-empty");
        /*
         * loop-body is empty so far!
         */
        if (INFO_FLAT_LASTASSIGN (arg_info) != NULL) {
            DO_ASSIGNS (arg_node) = INFO_FLAT_LASTASSIGN (arg_info);
        }
    } else {
        ASSIGN_NEXT (final_assign) = INFO_FLAT_LASTASSIGN (arg_info);
    }

    DBUG_PRINT ("appending %p tp %p!",
                (void *)INFO_FLAT_LASTASSIGN (arg_info),
                (void *)final_assign);
    INFO_FLAT_LASTASSIGN (arg_info) = mem_last_assign;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FLATwith(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/

node *
FLATwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    WITH_WITHOP (arg_node) = TRAVopt(WITH_WITHOP (arg_node), arg_info);
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *FLATgenarray(node *arg_node, info *arg_info)
 *
 * description:
 *   flattens N_genarray
 *   - genarray: the shape is NOT flattened!
 *
 ******************************************************************************/

node *
FLATgenarray (node *arg_node, info *arg_info)
{
    node *expr, *expr2;

    DBUG_ENTER ();

    expr = GENARRAY_SHAPE (arg_node);

    if (!NODE_IS_ID (expr)) {
        GENARRAY_SHAPE (arg_node) = Abstract (expr, arg_info);
        expr2 = TRAVdo (expr, arg_info);
        DBUG_ASSERT (expr == expr2,
                     "return-node differs from arg_node while flattening an expr!");
    }

    expr = GENARRAY_DEFAULT (arg_node);

    if ((expr != NULL) && (!NODE_IS_ID (expr))) {
        GENARRAY_DEFAULT (arg_node) = Abstract (expr, arg_info);
        expr2 = TRAVdo (expr, arg_info);
        DBUG_ASSERT (expr == expr2,
                     "return-node differs from arg_node while flattening an expr!");
    }

    GENARRAY_NEXT (arg_node) = TRAVopt(GENARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *FLATmodarray(node *arg_node, info *arg_info)
 *
 * description:
 *   flattens N_modarray
 *   - modarray: the array has to be an id or is flattened otherwise.
 *
 ******************************************************************************/

node *
FLATmodarray (node *arg_node, info *arg_info)
{
    node *expr, *expr2;

    DBUG_ENTER ();

    expr = MODARRAY_ARRAY (arg_node);
    if ((NODE_TYPE (expr) == N_prf) || (NODE_TYPE (expr) == N_spap)
        || (NODE_TYPE (expr) == N_array) || (NODE_TYPE (expr) == N_with)
        || (NODE_TYPE (expr) == N_cast)) {
        MODARRAY_ARRAY (arg_node) = Abstract (expr, arg_info);
        expr2 = TRAVdo (expr, arg_info);
    } else {
        expr2 = TRAVdo (expr, arg_info);
    }
    DBUG_ASSERT (expr == expr2,
                 "return-node differs from arg_node while flattening an expr!");

    MODARRAY_NEXT (arg_node) = TRAVopt(MODARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *FLATspfold(node *arg_node, info *arg_info)
 *
 * description:
 *   flattens N_spfold
 *   - fold: the neutral element and the potential fix element have to be an id
 *           or is flattened otherwise.
 *           It is optional.
 *
 ******************************************************************************/

node *
FLATspfold (node *arg_node, info *arg_info)
{
    node *expr, *expr2;
    contextflag old_ctxt;

    DBUG_ENTER ();

    expr = SPFOLD_NEUTRAL (arg_node);
    if ((expr != NULL) && (!NODE_IS_ID (expr))) {
        SPFOLD_NEUTRAL (arg_node) = Abstract (expr, arg_info);
        expr2 = TRAVdo (expr, arg_info);

        DBUG_ASSERT (expr == expr2,
                     "return-node differs from arg_node while flattening an expr!");
    }

    expr = SPFOLD_GUARD (arg_node);
    if ((expr != NULL) && (!NODE_IS_ID (expr))) {
        SPFOLD_GUARD (arg_node) = Abstract (expr, arg_info);
        expr2 = TRAVdo (expr, arg_info);

        DBUG_ASSERT (expr == expr2,
                     "return-node differs from arg_node while flattening an expr!");
    }

    if (SPFOLD_ARGS (arg_node) != NULL) {
        old_ctxt = (contextflag)INFO_FLAT_CONTEXT (arg_info);
        INFO_FLAT_CONTEXT (arg_info) = CT_ap;
        SPFOLD_ARGS (arg_node) = TRAVdo (SPFOLD_ARGS (arg_node), arg_info);
        INFO_FLAT_CONTEXT (arg_info) = old_ctxt;
    }

    SPFOLD_NEXT (arg_node) = TRAVopt(SPFOLD_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *FLATpropagate(node *arg_node, info *arg_info)
 *
 * description:
 *   flattens N_propagate
 *
 ******************************************************************************/

node *
FLATpropagate (node *arg_node, info *arg_info)
{
    node *expr, *expr2;

    DBUG_ENTER ();

    expr = PROPAGATE_DEFAULT (arg_node);
    if ((NODE_TYPE (expr) == N_prf) || (NODE_TYPE (expr) == N_spap)
        || (NODE_TYPE (expr) == N_array) || (NODE_TYPE (expr) == N_with)
        || (NODE_TYPE (expr) == N_cast)) {
        MODARRAY_ARRAY (arg_node) = Abstract (expr, arg_info);
        expr2 = TRAVdo (expr, arg_info);
    } else {
        expr2 = TRAVdo (expr, arg_info);
    }
    DBUG_ASSERT (expr == expr2,
                 "return-node differs from arg_node while flattening an expr!");

    PROPAGATE_NEXT (arg_node) = TRAVopt(PROPAGATE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *FLATpart( node *arg_node, info *arg_info)
 *
 * description:
 *   flattens all N_part nodes
 *
 * remark:
 *   the index variables are always renamed to unique names (this differs
 *   from the old WLs). This is done in respect of later Withloop folding.
 *   During WL-folding WL bodies are merged and with unique generators name
 *   clashes can be avoided. Example:
 *     A: with (...i...) { ...B... }
 *     B: with((...j...) { ...tmp = i... }   i bound outside withloop.
 *   substitute B in A and i is bound to index variable of A.
 *
 ******************************************************************************/

node *
FLATpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* flatten the sons */
    PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);

    PART_WITHID (arg_node) = TRAVdo (PART_WITHID (arg_node), arg_info);

    PART_NEXT (arg_node) = TRAVopt(PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *FLATwithid( node *arg_node, info *arg_info)
 *
 * Description:
 *   Creates WITHID_VEC if missing.
 *
 ******************************************************************************/

node *
FLATwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (WITHID_VEC (arg_node) == NULL) {
        WITHID_VEC (arg_node) = TBmakeSpids (TRAVtmpVar (), NULL);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *FLATgenerator(node *arg_node, info *arg_info)
 *
 * description:
 *   flattens N_generator
 *   all non-N_ID-nodes are removed and the operators are changed
 *   to <= and < if possible (bounds != NULL).
 *
 ******************************************************************************/

node *
FLATgenerator (node *arg_node, info *arg_info)
{
    node **act_son, *act_son_expr, *act_son_expr2;
    int i;
    DBUG_ENTER ();

    for (i = 0; i < 4; i++) {
        switch (i) {
        case 0:
            act_son = &GENERATOR_BOUND1 (arg_node);
            DBUG_PRINT ("flattening left boundary!");
            break;
        case 1:
            act_son = &GENERATOR_BOUND2 (arg_node);
            DBUG_PRINT ("flattening right boundary!");
            break;
        case 2:
            act_son = &GENERATOR_STEP (arg_node);
            DBUG_PRINT ("flattening step parameter!");
            break;
        case 3:
            act_son = &GENERATOR_WIDTH (arg_node);
            DBUG_PRINT ("flattening width parameter!");
            break;
        default:
            /*
             * the following assignment is used only for convincing the C compiler
             * that act_son will be initialized in any case!
             */
            act_son = NULL;
        }

        act_son_expr = *act_son;

        if ((act_son_expr != NULL) && (!DOT_ISSINGLE (act_son_expr))) {
            if (!NODE_IS_ID (act_son_expr)) {
                *act_son = Abstract (act_son_expr, arg_info);
                act_son_expr2 = TRAVdo (act_son_expr, arg_info);
            } else {
                act_son_expr2 = TRAVdo (act_son_expr, arg_info);
            }

            DBUG_ASSERT (act_son_expr == act_son_expr2,
                         "return-node differs from arg_node while flattening an expr!");
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FLATcode(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/

node *
FLATcode (node *arg_node, info *arg_info)
{
    node *exprs, *exprs2, *mem_last_assign, *flatten_assignments, *final_assign;
    contextflag old_ctxt;

    DBUG_ENTER ();

    /**
     * First, we traverse the body so that INFO_FLAT_FINALASSIGN will
     * be set correctly.
     */
    DBUG_ASSERT (CODE_CBLOCK (arg_node) != NULL, "no code block found");
    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    final_assign = INFO_FLAT_FINALASSIGN (arg_info);
    INFO_FLAT_FINALASSIGN (arg_info) = NULL;

    /*
     * After traversing the body, we finally flatten the CEXPR!
     */
    exprs = CODE_CEXPRS (arg_node);
    if (exprs != NULL) {
        /**
         * we collect all potential lift-outs in LASTASSIGN
         * for later insertion in FINALASSIGN
         */
        mem_last_assign = INFO_FLAT_LASTASSIGN (arg_info);
        INFO_FLAT_LASTASSIGN (arg_info) = NULL;
        old_ctxt = (contextflag)INFO_FLAT_CONTEXT (arg_info);
        INFO_FLAT_CONTEXT (arg_info) = CT_return;

        exprs2 = TRAVdo (exprs, arg_info);

        DBUG_ASSERT (exprs == exprs2,
                     "return-node differs from arg_node while flattening WL-exprs!");
        /*
         * Here, INFO_FLAT_LASTASSIGN( arg_info) either points to the freshly
         * generated flatten-assignments or is NULL (if nothing had to be abstracted
         * out)!!
         */
        flatten_assignments = INFO_FLAT_LASTASSIGN (arg_info);
        INFO_FLAT_LASTASSIGN (arg_info) = mem_last_assign;
        INFO_FLAT_CONTEXT (arg_info) = old_ctxt;

        /**
         * Now, we insert the flatten_assignments:
         */
        if (flatten_assignments != NULL) {
            if (BLOCK_ASSIGNS (CODE_CBLOCK (arg_node)) == NULL) {
                BLOCK_ASSIGNS (CODE_CBLOCK (arg_node)) = flatten_assignments;
            } else {
                ASSIGN_NEXT (final_assign) = flatten_assignments;
            }
        }
    }

    CODE_NEXT (arg_node) = TRAVopt(CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
