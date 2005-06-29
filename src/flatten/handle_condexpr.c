/*
 *
 * $Log$
 * Revision 1.2  2005/06/29 08:49:53  ktr
 * added rules for N_code, N_with and N_do
 *
 * Revision 1.1  2005/06/28 20:52:13  cg
 * Initial revision
 *
 */

#include "handle_condexpr.h"

#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "DupTree.h"
#include "dbug.h"
#include "free.h"

/**
 * INFO structure
 */
struct INFO {
    node *preassign;
};

/**
 * INFO macros
 */
#define INFO_HCE_PREASSIGN(n) (n->preassign)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_HCE_PREASSIGN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/** <!--**********************************************************************
 *
 * @fn node *HCEdoHandleConditionalExpressions( node *syntax_tree)
 *
 * @brief starts the elimination of N_funcond nodes
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/
node *
HCEdoHandleConditionalExpressions (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("HCEdoHandleConditionalExpressions");

    info = MakeInfo ();

    TRAVpush (TR_hce);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * Handle conditional expressions traversal (hce_tab)
 *
 * prefix: HCE
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *HCEassign(node *arg_node, info *arg_info)
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
HCEassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("HCEassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_HCE_PREASSIGN (arg_info) != NULL) {
        node *preassign;

        preassign = INFO_HCE_PREASSIGN (arg_info);
        INFO_HCE_PREASSIGN (arg_info) = NULL;

        preassign = TRAVdo (preassign, arg_info);
        arg_node = TCappendAssign (preassign, arg_node);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *HCEcode(node *arg_node, info *arg_info)
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
HCEcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("HCEcode");

    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);

    if (INFO_HCE_PREASSIGN (arg_info) != NULL) {
        CODE_CBLOCK_INSTR (arg_node)
          = TCappendAssign (CODE_CBLOCK_INSTR (arg_node), INFO_HCE_PREASSIGN (arg_info));

        INFO_HCE_PREASSIGN (arg_info) = NULL;
    }

    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *HCEcond(node *arg_node, info *arg_info)
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
HCEcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("HCEcond");

    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);
    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *HCEdo(node *arg_node, info *arg_info)
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
HCEdo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("HCEdo");

    DO_COND (arg_node) = TRAVdo (DO_COND (arg_node), arg_info);

    if (INFO_HCE_PREASSIGN (arg_info) != NULL) {
        DO_INSTR (arg_node)
          = TCappendAssign (DO_INSTR (arg_node), INFO_HCE_PREASSIGN (arg_info));

        INFO_HCE_PREASSIGN (arg_info) = NULL;
    }

    DO_BODY (arg_node) = TRAVdo (DO_BODY (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *HCEfuncond(node *arg_node, info *arg_info)
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
HCEfuncond (node *arg_node, info *arg_info)
{
    char *n;
    node *p, *t, *e;

    DBUG_ENTER ("HCEfuncond");

    n = ILIBtmpVar ();

    p = FUNCOND_IF (arg_node);
    FUNCOND_IF (arg_node) = NULL;

    t = TBmakeBlock (TBmakeAssign (TBmakeLet (TBmakeSpids (ILIBstringCopy (n), NULL),
                                              FUNCOND_THEN (arg_node)),
                                   NULL),
                     NULL);
    FUNCOND_THEN (arg_node) = NULL;

    e = TBmakeBlock (TBmakeAssign (TBmakeLet (TBmakeSpids (ILIBstringCopy (n), NULL),
                                              FUNCOND_ELSE (arg_node)),
                                   NULL),
                     NULL);
    FUNCOND_ELSE (arg_node) = NULL;

    INFO_HCE_PREASSIGN (arg_info)
      = TCappendAssign (INFO_HCE_PREASSIGN (arg_info),
                        TBmakeAssign (TBmakeCond (p, t, e), NULL));

    arg_node = FREEdoFreeNode (arg_node);

    DBUG_RETURN (TBmakeSpid (NULL, n));
}

/** <!--********************************************************************-->
 *
 * @fn node *HCEwith(node *arg_node, info *arg_info)
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
HCEwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("HCEwith");

    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}
