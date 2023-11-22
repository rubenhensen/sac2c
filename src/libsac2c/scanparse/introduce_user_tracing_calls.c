/** <!--********************************************************************-->
 *
 * @defgroup iutc Introduce User Trace Calls Traversal
 *
 * Module description goes here.
 *
 * @ingroup iutc
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file introduce_user_tracing_calls.c
 *
 * Prefix: IUTC
 *
 *****************************************************************************/
#include "introduce_user_tracing_calls.h"

/*
 * Other includes go here
 */

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "namespaces.h"
#include "str.h"
#include "memory.h"

#define TRACEFUN_NAMESPACE "UTrace"
#define TRACE_ARG_FUN "PrintArg"
#define TRACE_ASSIGN_FUN "PrintAssign"
#define TRACE_RETURN_FUN "PrintReturn"
#define TRACE_ENTER_FUN "PrintFunEnter"
#define TRACE_LEAVE_FUN "PrintFunLeave"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *postassign;
    node *preassign;
    node *blockassign;
    int level;
    const char *funname;
};

/**
 * Info structure macros
 */
#define INFO_POSTASSIGN(n) ((n)->postassign)
#define INFO_PREASSIGN(n) ((n)->preassign)
#define INFO_BLOCKASSIGN(n) ((n)->blockassign)
#define INFO_LEVEL(n) ((n)->level)
#define INFO_FUNNAME(n) ((n)->funname)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_POSTASSIGN (result) = NULL;
    INFO_PREASSIGN (result) = NULL;
    INFO_BLOCKASSIGN (result) = NULL;
    INFO_LEVEL (result) = 0;
    INFO_FUNNAME (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}
/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *IUTCdoIntroduceUserTraceCalls( node *syntax_tree)
 *
 *****************************************************************************/
node *
IUTCdoIntroduceUserTraceCalls (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_iutc);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

/** <!-- ****************************************************************** -->
 * @brief Creates a Spap call to the named function in UTrace.
 *
 * @param funname   function to call
 * @param filename  filename argument to call
 * @param pos       lineno argument to call
 * @param args      additional args
 *
 * @return          SPAp node.
 ******************************************************************************/
static node *
ApTraceFun (const char *funname, const char *filename, size_t pos, node *args)
{
    node *result;

    DBUG_ENTER ();

    result
      = TBmakeSpap (TBmakeSpid (NSgetNamespace (TRACEFUN_NAMESPACE), STRcpy (funname)),
                    TBmakeExprs (STRstring2Array (filename),
                                 TBmakeExprs (TBmakeNumulong (pos), args)));

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @brief Creates appropriate preassigns for the given return expressions.
 *
 * @param exprs     Return expressions
 * @param arg_info  info node
 *
 * @return modified expression chain
 ******************************************************************************/
static node *
ReturnExprs2Trace (node *exprs, info *arg_info)
{
    char *newvar;
    node *newexpr;

    DBUG_ENTER ();

    if (EXPRS_NEXT (exprs) != NULL) {
        EXPRS_NEXT (exprs) = ReturnExprs2Trace (EXPRS_NEXT (exprs), arg_info);
    }

    newvar = TRAVtmpVar ();
    newexpr = TBmakeSpid (NULL, newvar);

    INFO_PREASSIGN (arg_info) = TBmakeAssign (
      TBmakeLet (TBmakeSpids (STRcpy (newvar), NULL), EXPRS_EXPR (exprs)),
      TBmakeAssign (TBmakeLet (NULL,
                               ApTraceFun (TRACE_RETURN_FUN, NODE_FILE (exprs),
                                           NODE_LINE (exprs),
                                           TBmakeExprs (TBmakeSpid (NULL,
                                                                    STRcpy (newvar)),
                                                        NULL))),
                    INFO_PREASSIGN (arg_info)));

    EXPRS_EXPR (exprs) = newexpr;

    DBUG_RETURN (exprs);
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *IUTCfundef(node *arg_node, info *arg_info)
 *
 * @brief Performs a traversal of the fundef chain.
 *
 *****************************************************************************/
node *
IUTCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_FUNNAME (arg_info) = FUNDEF_NAME (arg_node);

        FUNDEF_ARGS (arg_node) = TRAVopt(FUNDEF_ARGS (arg_node), arg_info);

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNNAME (arg_info) = NULL;
    }

    FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IUTCarg(node *arg_node, info *arg_info)
 *
 * @brief Performs a traversal of the arg chain.
 *
 *****************************************************************************/
node *
IUTCarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ARG_NEXT (arg_node) = TRAVopt(ARG_NEXT (arg_node), arg_info);

    INFO_BLOCKASSIGN (arg_info) = TBmakeAssign (
      TBmakeLet (NULL,
                 ApTraceFun (TRACE_ARG_FUN, NODE_FILE (arg_node), NODE_LINE (arg_node),
                             TBmakeExprs (STRstring2Array (ARG_NAME (arg_node)),
                                          TBmakeExprs (TBmakeSpid (NULL,
                                                                   STRcpy (ARG_NAME (
                                                                     arg_node))),
                                                       NULL)))),
      INFO_BLOCKASSIGN (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IUTCspids(node *arg_node, info *arg_info)
 *
 * @brief Performs a traversal of the spids chain.
 *
 *****************************************************************************/
node *
IUTCspids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    SPIDS_NEXT (arg_node) = TRAVopt(SPIDS_NEXT (arg_node), arg_info);

    INFO_POSTASSIGN (arg_info) = TBmakeAssign (
      TBmakeLet (NULL,
                 ApTraceFun (TRACE_ASSIGN_FUN, NODE_FILE (arg_node), NODE_LINE (arg_node),
                             TBmakeExprs (STRstring2Array (SPIDS_NAME (arg_node)),
                                          TBmakeExprs (TBmakeSpid (NULL,
                                                                   STRcpy (SPIDS_NAME (
                                                                     arg_node))),
                                                       NULL)))),
      INFO_POSTASSIGN (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IUTCreturn(node *arg_node, info *arg_info)
 *
 * @brief Performs a traversal of the return chain.
 *
 *****************************************************************************/
node *
IUTCreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (RETURN_EXPRS (arg_node) != NULL) {
        RETURN_EXPRS (arg_node) = ReturnExprs2Trace (RETURN_EXPRS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IUTCassign(node *arg_node, info *arg_info)
 *
 * @brief Performs a bottom-up traversal of the assign chain.
 *
 *****************************************************************************/
node *
IUTCassign (node *arg_node, info *arg_info)
{
    node **chain;

    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt(ASSIGN_NEXT (arg_node), arg_info);

    if (INFO_PREASSIGN (arg_info) != NULL) {
        ASSIGN_NEXT (arg_node)
          = TCappendAssign (INFO_PREASSIGN (arg_info), ASSIGN_NEXT (arg_node));
        INFO_PREASSIGN (arg_info) = NULL;
    }

    if ((ASSIGN_NEXT (arg_node) == NULL) && (INFO_LEVEL (arg_info) == 1)) {
        /*
         * insert trace call for leaving fundef
         *
         * NOTE: if there is no return, we have to insert the code
         *       after the last node. Otherwise it needs to go before
         *       the last node. So we have to pick the right chain :)
         */

        chain = (NODE_TYPE (ASSIGN_STMT (arg_node)) == N_return)
                  ? &INFO_PREASSIGN (arg_info)
                  : &INFO_POSTASSIGN (arg_info);

        *chain
          = TBmakeAssign (TBmakeLet (NULL,
                                     ApTraceFun (TRACE_LEAVE_FUN, NODE_FILE (arg_node),
                                                 NODE_LINE (arg_node),
                                                 TBmakeExprs (STRstring2Array (
                                                                INFO_FUNNAME (arg_info)),
                                                              NULL))),
                          *chain);
    }

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_POSTASSIGN (arg_info) != NULL) {
        ASSIGN_NEXT (arg_node)
          = TCappendAssign (INFO_POSTASSIGN (arg_info), ASSIGN_NEXT (arg_node));
        INFO_POSTASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IUTCblock(node *arg_node, info *arg_info)
 *
 * @brief Fuses pre-assigns into beginning of block.
 *
 *****************************************************************************/
node *
IUTCblock (node *arg_node, info *arg_info)
{
    node *preassigns, *postassigns;

    DBUG_ENTER ();

    /*
     * preserve outer context
     */
    preassigns = INFO_PREASSIGN (arg_info);
    INFO_PREASSIGN (arg_info) = NULL;
    postassigns = INFO_POSTASSIGN (arg_info);
    INFO_POSTASSIGN (arg_info) = NULL;

    INFO_LEVEL (arg_info)++;

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    INFO_LEVEL (arg_info)--;

    if (INFO_LEVEL (arg_info) == 0) {
        /*
         * fuse in topblock assigns
         */
        if (INFO_BLOCKASSIGN (arg_info) != NULL) {
            INFO_PREASSIGN (arg_info)
              = TCappendAssign (INFO_BLOCKASSIGN (arg_info), INFO_PREASSIGN (arg_info));
            INFO_BLOCKASSIGN (arg_info) = NULL;
        }

        /*
         * insert function enter code
         */
        INFO_PREASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (NULL,
                                     ApTraceFun (TRACE_ENTER_FUN, NODE_FILE (arg_node),
                                                 NODE_LINE (arg_node),
                                                 TBmakeExprs (STRstring2Array (
                                                                INFO_FUNNAME (arg_info)),
                                                              NULL))),
                          INFO_PREASSIGN (arg_info));
    }

    if (INFO_PREASSIGN (arg_info) != NULL) {
        BLOCK_ASSIGNS (arg_node)
          = TCappendAssign (INFO_PREASSIGN (arg_info), BLOCK_ASSIGNS (arg_node));
        INFO_PREASSIGN (arg_info) = NULL;
    }

    DBUG_ASSERT (INFO_POSTASSIGN (arg_info) == NULL,
                 "there should be no more post-assigns at end of block traversal!");

    INFO_PREASSIGN (arg_info) = preassigns;
    INFO_POSTASSIGN (arg_info) = postassigns;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Introduce User Trace Calls -->
 *****************************************************************************/

#undef DBUG_PREFIX
