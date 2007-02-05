/*
 * $Id$
 */

/**
 *
 * @file inlining.c
 *
 * This file realizes the optimization technique function inlining.
 *
 * In order to properly handling inlining of (mutually) recursive functions,
 * the traversal sequence is unusual. We start traversing the fundef chain,
 * but as soon as we encounter an application of an inline-function, which
 * has not been treated yet, we continue with that function and complete
 * inlining in that function body first. As a consequence, whenever we
 * actually perform the inlining, we do not traverse the inlined code again.
 *
 * Inlining of recursive functions is controlled by the command line flag
 * -maxrecinl <n> and the corresponding global variable max_recursive_inlining.
 * At most <n> recursive applications (!) of a function will be inlined.
 * With <n>==0, a directly recursive function will not be inlined in itself,
 * but one incarnation will be inlined at every external application.
 * With two mutually recursive functions, one will be inlined into the other.
 * All external applications of each of them are inlined as well. As a
 * consequence, one of the two functions will become garbage, provided it is
 * not exported by a module, etc.
 *
 * Greater values of max_recursive_inlining lead to a partial unrolling
 * of recursion both for directly recursive functions as well as for systems
 * of mutually recursive functions.
 */

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "dbug.h"
#include "traverse.h"
#include "free.h"
#include "ctinfo.h" /* for CTIitemName */
#include "str.h"
#include "memory.h"
#include "type_utils.h"
#include "prepare_inlining.h"

#include "inlining.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *letids;
    node *code;
    node *vardecs;
    int depth;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LETIDS(n) (n->letids)
#define INFO_CODE(n) (n->code)
#define INFO_VARDECS(n) (n->vardecs)
#define INFO_DEPTH(n) (n->depth)

/*
  INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_LETIDS (result) = NULL;
    INFO_CODE (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_DEPTH (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/**<!--***********************************************************************-->
 *
 * @fn node *INLmodule( node *arg_node, info *arg_info)
 *
 * @brief traverses into function definitions only
 *
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/

node *
INLmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("INLmodule");

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *INLfundef( node *arg_node, info *arg_info)
 *
 * @brief realizes top-down traversal of fundef chain
 *
 *  This function is used not only for traversing the fundef chain, but also
 *  for starting traversal of fundefs referenced in applications. The two
 *  cases can be distinguished by INFO_DEPTH, which keeps track of the nesting
 *  depth of function applications, i.e. INFO_DEPTH == 0 means we are on the
 *  top-level fundef chain.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/

node *
INLfundef (node *arg_node, info *arg_info)
{
    info *old_info;

    DBUG_ENTER ("INLfundef");

    if ((FUNDEF_BODY (arg_node) != NULL) && (!FUNDEF_ISINLINECOMPLETED (arg_node))
        && (!FUNDEF_ISWRAPPERFUN (arg_node))
        && ((!FUNDEF_ISLACFUN (arg_node)) || (INFO_DEPTH (arg_info) >= 1))) {

        old_info = arg_info;
        arg_info = MakeInfo ();

        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_INLINECOUNTER (arg_node) += 1;

        DBUG_PRINT ("INL", ("Traversing body of %s", CTIitemName (arg_node)));

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        FUNDEF_INLINECOUNTER (arg_node) -= 1;
        FreeInfo (arg_info);
        arg_info = old_info;

        FUNDEF_ISINLINECOMPLETED (arg_node) = TRUE;
    }

    if (INFO_DEPTH (arg_info) == 0) {
        /*
         * We only continue with traversing the next function if we are on
         * the top level fundef chain.
         */
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }

        /*
         * Once we have finished inlining, we reset this flag to prepare the
         * inlining mechanism for subsequent applications in the optimization
         * cycle.
         */
        /*
        FUNDEF_ISINLINECOMPLETED( arg_node) = FALSE;
        */
    }

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *INLassign( node *arg_node, info *arg_info)
 *
 * @brief traverses into right hand side of assignment and realizes inlining
 *
 *  If we find a case for inlining during traversal of the right hand side,
 *  the code and the vardecs to be inlined are stored in the info structure.
 *  Here, we store pointers to both on the runtime stack and continue with
 *  the subsequent assignment. Only after traversal of the whole assignment
 *  chain, we actually realize the inlining and replace the current assignment
 *  by the stored code and append the vardecs to the chain of vardecs.
 *  Doing so, we never check inlined code for further cases of inlining.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/

node *
INLassign (node *arg_node, info *arg_info)
{
    bool inlined = FALSE;
    node *code = NULL;
    node *vardecs = NULL;

    DBUG_ENTER ("INLassign");

    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }

    if (INFO_CODE (arg_info) != NULL) {
        code = INFO_CODE (arg_info);
        INFO_CODE (arg_info) = NULL;
        vardecs = INFO_VARDECS (arg_info);
        INFO_VARDECS (arg_info) = NULL;
        inlined = TRUE;
        global.optcounters.inl_fun++; /* global optimization counter */
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    if (inlined) {
        DBUG_PRINT ("INL", ("Inlining code"));

        ASSIGN_NEXT (arg_node) = TCappendAssign (code, ASSIGN_NEXT (arg_node));
        BLOCK_VARDEC (FUNDEF_BODY (INFO_FUNDEF (arg_info)))
          = TCappendVardec (vardecs, BLOCK_VARDEC (FUNDEF_BODY (INFO_FUNDEF (arg_info))));
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *INLlet( node *arg_node, info *arg_info)
 *
 * @brief remembers LHS in INFO node and traverses RHS
 *
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/

node *
INLlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("INLlet");

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    INFO_LETIDS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *INLap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/

node *
INLap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("INLap");

    DBUG_PRINT ("INL", ("Processing call of fun %s", CTIitemName (AP_FUNDEF (arg_node))));

    if (AP_CONSIDERINLINE (arg_node)
        && (FUNDEF_INLINECOUNTER (AP_FUNDEF (arg_node)) <= global.max_recursive_inlining)
        && !TUretsContainBottom (FUNDEF_RETS (AP_FUNDEF (arg_node)))
        && !TUretsAreConstant (FUNDEF_RETS (AP_FUNDEF (arg_node)))) {

        if (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node))
            || (FUNDEF_ISINLINE (AP_FUNDEF (arg_node)))) {
            INFO_DEPTH (arg_info) += 1;

            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);

            INFO_DEPTH (arg_info) -= 1;
        }

        if (FUNDEF_ISINLINE (AP_FUNDEF (arg_node))
            && FUNDEF_ISINLINECOMPLETED (AP_FUNDEF (arg_node))) {
            DBUG_PRINT ("INL",
                        ("Inline preparing %s", CTIitemName (AP_FUNDEF (arg_node))));

            INFO_CODE (arg_info)
              = PINLdoPrepareInlining (&INFO_VARDECS (arg_info), AP_FUNDEF (arg_node),
                                       INFO_LETIDS (arg_info), AP_ARGS (arg_node));
        }
    } else {
        /*
         * There are three possible reasons to reach this code:
         *
         * I)
         *
         * Maximum number of recursive inlinings is exceeded, so we mark the ap
         * node and will never try again to inline here. This precaution is
         * necessary to enforce the limit on recursive inlinigs through multiple
         * applications of the function inlining optimization.
         *
         * II)
         *
         * The return types contain a bottom. Therefore, we will never want to
         * inline this function, as a bottom type basically is a constant that
         * has been propagated into the calling function anyways. As a
         * function that has been typed as bottom cannot get any other return
         * type in later runs, we can preserve this decision here.
         *
         * III)
         *
         * The return type is fully constant. In that case, there is no need
         * to inline as the constant has been propagated into the calling
         * context anyways. Furthermore, as the type cannot get coarser in
         * future runs of the TC, we can preserve this decision here.
         */
        AP_CONSIDERINLINE (arg_node) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *INLdoInlining( node *arg_node)
 *
 * @brief initiates function inlining as optimization phase
 *
 *
 * @param arg_node
 *
 * @return arg_node
 *
 *****************************************************************************/

node *
INLdoInlining (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("INLdoInlining");

#ifdef SHOW_MALLOC
    DBUG_PRINT ("OPTMEM",
                ("mem currently allocated: %d bytes", global.current_allocated_mem));
#endif

    if ((NODE_TYPE (arg_node) == N_module)
        || ((NODE_TYPE (arg_node) == N_fundef) && (!FUNDEF_ISLACFUN (arg_node)))) {
        arg_info = MakeInfo ();

        if (NODE_TYPE (arg_node) == N_fundef) {
            /*
             * Prevent traversal of FUNDEF_NEXT by faking nested traversal
             */
            INFO_DEPTH (arg_info) = 1;
        }

        TRAVpush (TR_inl);
        arg_node = TRAVdo (arg_node, arg_info);
        TRAVpop ();

        FreeInfo (arg_info);
    }

#ifdef SHOW_MALLOC
    DBUG_PRINT ("OPTMEM",
                ("mem currently allocated: %d bytes", global.current_allocated_mem));
#endif

    DBUG_RETURN (arg_node);
}
