/*
 * $Id$
 */

#include <stdio.h>
#include <string.h>

#include "globals.h"
#include "dbug.h"

#include "types.h"
#include "internal_lib.h"
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
 * (node[0])    functions may place instructions IN FRONT of that assignment.
 *              FltnAssign returns this node[0]
 *
 * LASTWLBLOCK: This node is only used in the context of WLs. It is necessary
 * (node[1])    to put assignments (var initialisations) at the beginning of
 *              the WL-body. These assignments are stored here. See comment in
 *              FltnNcode.
 *
 * FINALASSIGN: Every FltnBlock resets node[2] to NULL.
 * (node[2])    Every FltnAssign replaces node[2] with arg_node if
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
#define INFO_FLAT_LASTWLBLOCK(n) (n->lastwlblock)
#define INFO_FLAT_FINALASSIGN(n) (n->finalassign)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_FLAT_CONTEXT (result) = 0;
    INFO_FLAT_LASTASSIGN (result) = NULL;
    INFO_FLAT_LASTWLBLOCK (result) = NULL;
    INFO_FLAT_FINALASSIGN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

#define EXPR_VAL(node)                                                                   \
    (NODE_TYPE (node) == N_double)                                                       \
      ? (DOUBLE_VAL (node))                                                              \
      : ((NODE_TYPE (node) == N_float)                                                   \
           ? (FLOAT_VAL (node))                                                          \
           : ((NODE_TYPE (node) == N_num)                                                \
                ? (NUM_VAL (node))                                                       \
                : ((NODE_TYPE (node) == N_bool) ? (BOOL_VAL (node))                      \
                                                : (CHAR_VAL (node)))))

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

    DBUG_ENTER ("Abstract");

    tmp = ILIBtmpVar ();
    ids = TBmakeSpids (ILIBstringCopy (tmp), NULL);

    INFO_FLAT_LASTASSIGN (arg_info)
      = TBmakeAssign (TBmakeLet (ids, arg_node), INFO_FLAT_LASTASSIGN (arg_info));

    DBUG_PRINT ("FLATTEN",
                ("node %08x inserted before %08x", INFO_FLAT_LASTASSIGN (arg_info),
                 ASSIGN_NEXT (INFO_FLAT_LASTASSIGN (arg_info))));

    res = TBmakeSpid (NULL, tmp);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn  node *FltnMGNwith( node *wloop)
 *
 *   @brief  splits the given multi generator WL into a nesting of WLs.
 *
 *   @param  wloop
 *   @return the single generator WL
 *
 ******************************************************************************/

static node *
FltnMgwith (node *wloop)
{
    node *part, *code, *withop, *first_wl;

    DBUG_ENTER ("FltnMgwith");

    DBUG_ASSERT ((NODE_TYPE (wloop) == N_with), "FltnMgwith applied to non With-Loop!");

    while ((PART_NEXT (WITH_PART (wloop)) != NULL)
           && (CODE_NEXT (WITH_CODE (wloop)) != NULL)) {
        /**
         * pull out the first part!
         */
        part = WITH_PART (wloop);
        WITH_PART (wloop) = PART_NEXT (part);
        PART_NEXT (part) = NULL;

        /**
         * pull out the first code!
         */
        code = WITH_CODE (wloop);
        WITH_CODE (wloop) = CODE_NEXT (code);
        CODE_NEXT (code) = NULL;

        /**
         * steal the withop!
         */
        withop = WITH_WITHOP (wloop);
        first_wl = TBmakeWith (part, code, withop);

        /**
         * Finally, we construct the resulting WL:
         */
        if (NODE_TYPE (WITH_WITHOP (wloop)) == N_spfold) {
            WITH_WITHOP (wloop) = TBmakeSpfold (first_wl);
            SPFOLD_NS (WITH_WITHOP (wloop)) = NSdupNamespace (SPFOLD_NS (withop));
            SPFOLD_FUN (WITH_WITHOP (wloop)) = ILIBstringCopy (SPFOLD_FUN (withop));
        } else {
            WITH_WITHOP (wloop) = TBmakeModarray (first_wl);
        }
    }

    DBUG_RETURN (wloop);
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

    DBUG_ENTER ("FLATdoFlatten");

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
    DBUG_ENTER ("FLATmodule");

    if (MODULE_FUNS (arg_node)) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

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

    DBUG_ENTER ("FLATfundef");

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
        DBUG_PRINT ("FLATTEN", ("flattening function %s:", FUNDEF_NAME (arg_node)));
        if (FUNDEF_ARGS (arg_node)) {
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
        }
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    /*
     * Proceed with the next function...
     */
    if (FUNDEF_NEXT (arg_node)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FLATblock(node *arg_node, info *arg_info)
 *
 * description:
 *   - if CONTEXT is CT_wl arg_node is inserted in LASTWLBLOCK and after
 *     traversing the body LASTWLBLOCK -> NEXT is set to the result of the
 *     traversal. This ensures that flattening and renaming do not interfere!
 *   - resets FINALASSIGN to NULL.
 *
 ******************************************************************************/

node *
FLATblock (node *arg_node, info *arg_info)
{
    node *assigns, *mem_last_wlblock;

    DBUG_ENTER ("FLATblock");

    INFO_FLAT_FINALASSIGN (arg_info) = NULL;
    if (BLOCK_INSTR (arg_node) != NULL) {
        if (INFO_FLAT_CONTEXT (arg_info) == CT_wl) {
            /*
             * First, we reset INFO_FLAT_CONTEXT( arg_info) to CT_normal!
             * This is essential since otherwise non-WL-blocks within a
             * WL would penetrate INFO_FLAT_LASTWLBLOCK( arg_info)!!
             */
            INFO_FLAT_CONTEXT (arg_info) = CT_normal;
            mem_last_wlblock = INFO_FLAT_LASTWLBLOCK (arg_info);
            INFO_FLAT_LASTWLBLOCK (arg_info) = arg_node;
            DBUG_PRINT ("RENAME", ("LASTWLBLOCK set to %08x", arg_node));
            assigns = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
            if (NODE_TYPE (INFO_FLAT_LASTWLBLOCK (arg_info)) == N_block) {
                BLOCK_INSTR (INFO_FLAT_LASTWLBLOCK (arg_info)) = assigns;
            } else {
                DBUG_ASSERT ((NODE_TYPE (INFO_FLAT_LASTWLBLOCK (arg_info)) == N_assign),
                             ("LASTWLBLOCK in flatten does neither point to"
                              " an N_block nor to an N_assign node !"));
                ASSIGN_NEXT (INFO_FLAT_LASTWLBLOCK (arg_info)) = assigns;
            }
            DBUG_PRINT ("RENAME", ("connecting %08x to %08x!",
                                   INFO_FLAT_LASTWLBLOCK (arg_info), assigns));
            INFO_FLAT_LASTWLBLOCK (arg_info) = mem_last_wlblock;
        } else {
            BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FLATassign(node *arg_node, info *arg_info)
 *
 * description:
 *   - stores arg_node in INFO_FLAT_LASTASSIGN( arg_info)
 *   - stores arg_node in INFO_FLAT_FINALASSIGN( arg_info)
 *   iff (ASSIGN_NEXT == NULL)
 *   - returns the modified INFO_FLAT_LASTASSIGN( arg_info) yielded
 *   by traversing the instruction so that newly created abstractions
 *   will automatically be inserted by the calling function!
 *
 ******************************************************************************/

node *
FLATassign (node *arg_node, info *arg_info)
{
    node *return_node;

    DBUG_ENTER ("FLATassign");

    INFO_FLAT_LASTASSIGN (arg_info) = arg_node;
    DBUG_PRINT ("FLATTEN", ("LASTASSIGN set to %08x!", arg_node));

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    /*
     * newly inserted abstractions are prepanded in front of
     * INFO_FLAT_LASTASSIGN(arg_info). To properly insert these nodes,
     * that pointer has to be returned:
     */
    return_node = INFO_FLAT_LASTASSIGN (arg_info);

    if (return_node != arg_node) {
        DBUG_PRINT ("FLATTEN", ("node %08x will be inserted instead of %08x", return_node,
                                arg_node));
    }

    if (ASSIGN_NEXT (arg_node)) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    } else {
        INFO_FLAT_FINALASSIGN (arg_info) = arg_node;
        DBUG_PRINT ("FLATTEN", ("FINALASSIGN set to %08x!", arg_node));
    }

    DBUG_RETURN (return_node);
}

/******************************************************************************
 *
 * function:
 *  node *FLATcast(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
FLATcast (node *arg_node, info *arg_info)
{
    node *expr, *expr2;

    DBUG_ENTER ("FLATcast");

    expr = CAST_EXPR (arg_node);
    if (NODE_TYPE (expr) != N_spid) {
        CAST_EXPR (arg_node) = Abstract (expr, arg_info);
        expr2 = TRAVdo (expr, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FLATarray(node *arg_node, info *arg_info)
 *
 * description:
 *  set the context-flag of arg_info to CT_array, traverse the arguments,
 *
 ******************************************************************************/

node *
FLATarray (node *arg_node, info *arg_info)
{
    contextflag old_context;

    DBUG_ENTER ("FLATarray");

    if (ARRAY_AELEMS (arg_node) != NULL) {
        /*
         *  The array is not empty; so we have to traverse it
         *
         *  During the following traversal some values of arg_info will be changed,
         *  so we need to save the actual values here, so they can be restored
         *  later.
         */
        old_context = INFO_FLAT_CONTEXT (arg_info);

        INFO_FLAT_CONTEXT (arg_info) = CT_array;

        ARRAY_AELEMS (arg_node) = TRAVdo (ARRAY_AELEMS (arg_node), arg_info);

        /*
         *  As mentioned above, the values are restored now.
         */
        INFO_FLAT_CONTEXT (arg_info) = old_context;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FLATspap(node *arg_node, info *arg_info)
 *
 * description:
 *  if the application has some arguments, set the context-flag of
 *  arg_info to CT_ap, traverse the arguments, and finally restore
 *  the old context-flag.
 *
 ******************************************************************************/

node *
FLATspap (node *arg_node, info *arg_info)
{
    contextflag old_ctxt;

    DBUG_ENTER ("FLATspap");

    DBUG_PRINT ("FLATTEN", ("flattening application of %s:", SPAP_NAME (arg_node)));

    if (SPAP_ARGS (arg_node) != NULL) {
        old_ctxt = INFO_FLAT_CONTEXT (arg_info);
        INFO_FLAT_CONTEXT (arg_info) = CT_ap;
        SPAP_ARGS (arg_node) = TRAVdo (SPAP_ARGS (arg_node), arg_info);
        INFO_FLAT_CONTEXT (arg_info) = old_ctxt;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FLATprf(node *arg_node, info *arg_info)
 *
 * description:
 *  - If the prf has some arguments, set the context-flag of arg_info
 *    CT_ap, traverse the arguments, and finally restore the old context-flag.
 *
 *  - It is important that all arguments are abstracted out as e.g.
 *    dim(0) cannot be evaluted by the implementation of the dim-prf.
 *
 *  - ConstVarPropagation can de-flatten arguments when appropriate.
 *
 *****************************************************************************/
node *
FLATprf (node *arg_node, info *arg_info)
{
    contextflag old_ctxt;

    DBUG_ENTER ("FLATprf");

    DBUG_PRINT ("FLATTEN",
                ("flattening application of %s:", global.mdb_prf[PRF_PRF (arg_node)]));

    if (PRF_ARGS (arg_node) != NULL) {
        old_ctxt = INFO_FLAT_CONTEXT (arg_info);
        INFO_FLAT_CONTEXT (arg_info) = CT_ap;
        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
        INFO_FLAT_CONTEXT (arg_info) = old_ctxt;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FLATreturn(node *arg_node, info *arg_info)
 *
 * description:
 *  if the function returns values, set the context-flag of
 *  arg_info to CT_return, traverse the results, and finally restore
 *  the old context-flag.
 *
 ******************************************************************************/

node *
FLATreturn (node *arg_node, info *arg_info)
{
    contextflag old_ctxt;

    DBUG_ENTER ("FltnReturn");

    if (RETURN_EXPRS (arg_node) != NULL) {
        old_ctxt = INFO_FLAT_CONTEXT (arg_info);
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

    DBUG_ENTER ("FLATexprs");

    expr = EXPRS_EXPR (arg_node);

    /*
     * compute whether to abstract <expr> or not , depending on the
     * context of the expression, given by INFO_FLAT_CONTEXT( arg_info)
     */
    switch (INFO_FLAT_CONTEXT (arg_info)) {
    case CT_ap:
        abstract = ((NODE_TYPE (expr) == N_num) || (NODE_TYPE (expr) == N_float)
                    || (NODE_TYPE (expr) == N_double) || (NODE_TYPE (expr) == N_bool)
                    || (NODE_TYPE (expr) == N_char) || (NODE_TYPE (expr) == N_str)
                    || (NODE_TYPE (expr) == N_array) || (NODE_TYPE (expr) == N_spap)
                    || (NODE_TYPE (expr) == N_prf) || (NODE_TYPE (expr) == N_with)
                    || (NODE_TYPE (expr) == N_cast));
        break;
    case CT_return:
        abstract = ((NODE_TYPE (expr) == N_num) || (NODE_TYPE (expr) == N_float)
                    || (NODE_TYPE (expr) == N_double) || (NODE_TYPE (expr) == N_bool)
                    || (NODE_TYPE (expr) == N_char) || (NODE_TYPE (expr) == N_str)
                    || (NODE_TYPE (expr) == N_array) || (NODE_TYPE (expr) == N_spap)
                    || (NODE_TYPE (expr) == N_prf) || (NODE_TYPE (expr) == N_with)
                    || (NODE_TYPE (expr) == N_cast));
        break;
    case CT_normal:
        abstract = ((NODE_TYPE (expr) == N_spap) || (NODE_TYPE (expr) == N_prf)
                    || (NODE_TYPE (expr) == N_with) || (NODE_TYPE (expr) == N_cast));
        break;
    case CT_array:
        abstract = ((NODE_TYPE (expr) == N_str) || (NODE_TYPE (expr) == N_array)
                    || (NODE_TYPE (expr) == N_spap) || (NODE_TYPE (expr) == N_prf)
                    || (NODE_TYPE (expr) == N_with) || (NODE_TYPE (expr) == N_cast));
        break;
    default:
        DBUG_ASSERT (0, "illegal context !");
        /* the following assignment is used only for convincing the C compiler
         * that abstract will be initialized in any case!
         */
        abstract = 0;
    }

    DBUG_PRINT ("FLATTEN", ("context: %d, abstract: %d, expr: %s",
                            INFO_FLAT_CONTEXT (arg_info), abstract, NODE_TEXT (expr)));

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

    DBUG_ASSERT ((expr == expr2),
                 "return-node differs from arg_node while flattening an expr!");

    /*
     * Last but not least remaining exprs have to be done:
     */
    if (EXPRS_NEXT (arg_node) != NULL) {
        EXPRS_NEXT (arg_node) = TRAVdo (EXPRS_NEXT (arg_node), arg_info);
    }

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

    DBUG_ENTER ("FltnCond");

    pred = COND_COND (arg_node);
    if (NODE_TYPE (pred) != N_spid) {
        COND_COND (arg_node) = Abstract (pred, arg_info);
    }

    pred2 = TRAVdo (pred, arg_info);
    DBUG_ASSERT ((pred == pred2),
                 "return-node differs from arg_node while flattening an expr!");

    mem_last_assign = INFO_FLAT_LASTASSIGN (arg_info);

    if (COND_THEN (arg_node)) {
        COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
    }

    if (COND_ELSE (arg_node)) {
        COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);
    }

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

    DBUG_ENTER ("FLATdo");

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
    DBUG_ASSERT ((pred == pred2),
                 "return-node differs from arg_node while flattening an expr!");
    if (final_assign == NULL) {
        DBUG_ASSERT ((NODE_TYPE (DO_INSTR (arg_node)) == N_empty),
                     "INFO_FLAT_FINALASSIGN is NULL although do-body is non-empty");
        /*
         * loop-body ist empty so far!
         */
        if (INFO_FLAT_LASTASSIGN (arg_info) != NULL) {
            DO_INSTR (arg_node) = FREEdoFreeTree (DO_INSTR (arg_node));
            DO_INSTR (arg_node) = INFO_FLAT_LASTASSIGN (arg_info);
        }
    } else {
        ASSIGN_NEXT (final_assign) = INFO_FLAT_LASTASSIGN (arg_info);
    }
    DBUG_PRINT ("FLATTEN", ("appending %08x tp %08x!", INFO_FLAT_LASTASSIGN (arg_info),
                            final_assign));
    INFO_FLAT_LASTASSIGN (arg_info) = mem_last_assign;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *FLATwith(node *arg_node, info *arg_info)
 *
 * description:
 *   flattens node N_Nwith
 *   increments with_level and saves local stack only.
 *
 ******************************************************************************/

node *
FLATwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FLATWith");

    arg_node = FltnMgwith (arg_node);

    /*
     * for traversing the operation, the generator var(s) should
     * not yet be pushed!
     */
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    /*
     * for traversing the body, the generator var(s) should BE pushed!
     */
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

    DBUG_ENTER ("FLATgenarray");

    expr = GENARRAY_SHAPE (arg_node);

    if (NODE_TYPE (expr) != N_id) {
        GENARRAY_SHAPE (arg_node) = Abstract (expr, arg_info);
        expr2 = TRAVdo (expr, arg_info);
        DBUG_ASSERT ((expr == expr2),
                     "return-node differs from arg_node while flattening an expr!");
    }

    expr = GENARRAY_DEFAULT (arg_node);

    if ((expr != NULL) && (NODE_TYPE (expr) != N_id)) {
        GENARRAY_DEFAULT (arg_node) = Abstract (expr, arg_info);
        expr2 = TRAVdo (expr, arg_info);
        DBUG_ASSERT ((expr == expr2),
                     "return-node differs from arg_node while flattening an expr!");
    }

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

    DBUG_ENTER ("FLATmodarray");

    expr = MODARRAY_ARRAY (arg_node);
    if ((NODE_TYPE (expr) == N_prf) || (NODE_TYPE (expr) == N_spap)
        || (NODE_TYPE (expr) == N_array) || (NODE_TYPE (expr) == N_with)
        || (NODE_TYPE (expr) == N_cast)) {
        MODARRAY_ARRAY (arg_node) = Abstract (expr, arg_info);
        expr2 = TRAVdo (expr, arg_info);
    } else {
        expr2 = TRAVdo (expr, arg_info);
    }
    DBUG_ASSERT ((expr == expr2),
                 "return-node differs from arg_node while flattening an expr!");

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

    DBUG_ENTER ("FLATspfold");

    expr = SPFOLD_NEUTRAL (arg_node);
    if ((expr != NULL) && (NODE_TYPE (expr) != N_id)) {
        SPFOLD_NEUTRAL (arg_node) = Abstract (expr, arg_info);
        expr2 = TRAVdo (expr, arg_info);

        DBUG_ASSERT ((expr == expr2),
                     "return-node differs from arg_node while flattening an expr!");
    }

    expr = SPFOLD_GUARD (arg_node);
    if ((expr != NULL) && (NODE_TYPE (expr) != N_id)) {
        SPFOLD_GUARD (arg_node) = Abstract (expr, arg_info);
        expr2 = TRAVdo (expr, arg_info);

        DBUG_ASSERT ((expr == expr2),
                     "return-node differs from arg_node while flattening an expr!");
    }

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
    DBUG_ENTER ("FLATpart");

    /* flatten the sons */
    PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);

    PART_WITHID (arg_node) = TRAVdo (PART_WITHID (arg_node), arg_info);

    /* at this early point there are no other N_Npart nodes */
    DBUG_ASSERT ((PART_NEXT (arg_node) == NULL), "PART_NEXT should not yet exist");

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
    DBUG_ENTER ("FLATwithid");

    if (WITHID_VEC (arg_node) == NULL) {
        WITHID_VEC (arg_node) = TBmakeSpids (ILIBtmpVar (), NULL);
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
    DBUG_ENTER ("FLATgenerator");

    for (i = 0; i < 4; i++) {
        switch (i) {
        case 0:
            act_son = &GENERATOR_BOUND1 (arg_node);
            DBUG_PRINT ("FLATTEN", ("flattening left boundary!"));
            break;
        case 1:
            act_son = &GENERATOR_BOUND2 (arg_node);
            DBUG_PRINT ("FLATTEN", ("flattening right boundary!"));
            break;
        case 2:
            act_son = &GENERATOR_STEP (arg_node);
            DBUG_PRINT ("FLATTEN", ("flattening step parameter!"));
            break;
        case 3:
            act_son = &GENERATOR_WIDTH (arg_node);
            DBUG_PRINT ("FLATTEN", ("flattening width parameter!"));
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
            if (N_id != NODE_TYPE (act_son_expr)) {
                *act_son = Abstract (act_son_expr, arg_info);
                act_son_expr2 = TRAVdo (act_son_expr, arg_info);
            } else {
                act_son_expr2 = TRAVdo (act_son_expr, arg_info);
            }

            DBUG_ASSERT ((act_son_expr == act_son_expr2),
                         "return-node differs from arg_node while flattening an expr!");
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *FltnNcode(node *arg_node, info *arg_info)
 *
 * description:
 *   flattens the Ncode nodes.
 *   it's important to have Npart flattened before to avoid name clashes.
 *
 ******************************************************************************/

node *
FLATcode (node *arg_node, info *arg_info)
{
    node **insert_at, *expr, *expr2, *mem_last_assign, *empty_block;

    DBUG_ENTER ("FLATcode");

    DBUG_ASSERT ((CODE_NEXT (arg_node) == NULL),
                 "there should be only one code block during flatten!");

    /* ATTENTION!!! code MUST NOT be traversed within flatten anymore!!!!! */

    mem_last_assign = INFO_FLAT_LASTASSIGN (arg_info);

    /*
     * First, we traverse the body so that INFO_FLAT_FINALASSIGN will
     * be set correctly, and all renamings will be pushed already!
     * For inserting assignments that (may) result from flattening CEXPR later,
     * we memoize the address which WOULD point to the next assignment
     * IFF one more WOULD exist, i.e. ASSIGNMENT_NEXT if the block is
     * non-empty, BLOCK_INSTR if the block is empty!
     * That allows us, to insert the result of flattening CEXPR
     * by simply assigning to (*insert_at) rather than using 2 different
     * ACCESS-Macros.
     * empty_block holds the pointer to the N_empty node iff one exists!
     */
    DBUG_ASSERT ((CODE_CBLOCK (arg_node) != NULL), "no code block found");
    if (NODE_TYPE (BLOCK_INSTR (CODE_CBLOCK (arg_node))) == N_empty) {
        /*
         * The body is empty; hence we do not need to traverse it!!
         */
        insert_at = &BLOCK_INSTR (CODE_CBLOCK (arg_node));
        empty_block = BLOCK_INSTR (CODE_CBLOCK (arg_node));
    } else {
        INFO_FLAT_CONTEXT (arg_info) = CT_wl;
        DBUG_PRINT ("RENAME", ("CONTEXT set to CT_wl"));
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
        insert_at = &ASSIGN_NEXT (INFO_FLAT_FINALASSIGN (arg_info));
        empty_block = NULL;
    }

    /*
     * After traversing the body, we finally flatten the CEXPR!
     */
    INFO_FLAT_LASTASSIGN (arg_info) = NULL;
    expr = EXPRS_EXPR (CODE_CEXPRS (arg_node));
    if (NODE_TYPE (expr) != N_id) {
        EXPRS_EXPR (CODE_CEXPRS (arg_node)) = Abstract (expr, arg_info);
        expr2 = TRAVdo (expr, arg_info);
    } else {
        expr2 = TRAVdo (expr, arg_info);
    }
    DBUG_ASSERT ((expr == expr2),
                 "return-node differs from arg_node while flattening an expr!");
    /*
     * Here, INFO_FLAT_LASTASSIGN( arg_info) either points to the freshly
     * generated flatten-assignments or is NULL (if nothing had to be abstracted out)!!
     */
    *insert_at = INFO_FLAT_LASTASSIGN (arg_info);

    /*
     * Now, we take care of the fu....g N_empty node...
     */
    if (BLOCK_INSTR (CODE_CBLOCK (arg_node)) == NULL) {
        /*
         * Block must have been empty & there is nothing to be flatted out
         * from CEXPR! => re-use empty_block !!
         */
        DBUG_ASSERT ((empty_block != NULL),
                     "flattened body is empty although un-flattened body isn't!!");
        BLOCK_INSTR (CODE_CBLOCK (arg_node)) = empty_block;
    } else {
        if (empty_block != NULL)
            FREEdoFreeTree (empty_block);
    }

    INFO_FLAT_LASTASSIGN (arg_info) = mem_last_assign;

    DBUG_RETURN (arg_node);
}
