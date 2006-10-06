/*
 * $Id: $
 */

#include "dbug.h"

#include "globals.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "DupTree.h"
#include "free.h"
#include "namespaces.h"

#include "handle_with_loop_generators.h"

/**
 * INFO structure
 */
struct INFO {
    node *lastassign;
    node *lhs;
    node *withops;
};

/**
 * INFO macros
 */
#define INFO_HWLG_LASTASSIGN(n) (n->lastassign)
#define INFO_HWLG_LHS(n) (n->lhs)
#define INFO_HWLG_NEW_WITHOPS(n) (n->withops)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_HWLG_LASTASSIGN (result) = NULL;
    INFO_HWLG_LHS (result) = NULL;
    INFO_HWLG_NEW_WITHOPS (result) = NULL;

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
 * @fn node *HWLGdoHandleWithLoops( node *syntax_tree)
 *
 * @brief starts the splitting of mgen/mop With-Loops
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/

node *
HWLGdoHandleWithLoops (node *arg_node)
{
    info *info_node;

    DBUG_ENTER ("HWLGdoHandleWithLoops");

    info_node = MakeInfo ();

    TRAVpush (TR_hwlg);
    arg_node = TRAVdo (arg_node, info_node);
    TRAVpop ();

    info_node = FreeInfo (info_node);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *HWLGassign(node *arg_node, info *arg_info)
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
HWLGassign (node *arg_node, info *arg_info)
{
    node *mem_last_assign, *return_node;

    DBUG_ENTER ("HWLGassign");

    mem_last_assign = INFO_HWLG_LASTASSIGN (arg_info);
    INFO_HWLG_LASTASSIGN (arg_info) = arg_node;
    DBUG_PRINT ("HWLG", ("LASTASSIGN set to %08x!", arg_node));

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    /*
     * newly inserted abstractions are prepanded in front of
     * INFO_HWLG_LASTASSIGN(arg_info). To properly insert these nodes,
     * that pointer has to be returned:
     */
    return_node = INFO_HWLG_LASTASSIGN (arg_info);

    if (return_node != arg_node) {
        DBUG_PRINT ("HWLG", ("node %08x will be inserted instead of %08x", return_node,
                             arg_node));
    }
    INFO_HWLG_LASTASSIGN (arg_info) = mem_last_assign;
    DBUG_PRINT ("HWLG", ("LASTASSIGN (re)set to %08x!", mem_last_assign));

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (return_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *HWLGwith(node *arg_node, info *arg_info)
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
HWLGwith (node *arg_node, info *arg_info)
{
    node *part, *code, *withop, *first_wl, *first_let;

    DBUG_ENTER ("HWLGwith");

    if ((PART_NEXT (WITH_PART (arg_node)) != NULL)
        && (CODE_NEXT (WITH_CODE (arg_node)) != NULL)) {
        /**
         * pull out the first part!
         */
        part = WITH_PART (arg_node);
        WITH_PART (arg_node) = PART_NEXT (part);
        PART_NEXT (part) = NULL;

        /**
         * pull out the first code!
         */
        code = TRAVdo (WITH_CODE (arg_node), arg_info);
        WITH_CODE (arg_node) = CODE_NEXT (code);
        CODE_NEXT (code) = NULL;

        /**
         * steal the withop(s)!
         */
        withop = WITH_WITHOP (arg_node);
        /**
         * and create the new first WL:
         */
        first_wl = TBmakeWith (part, code, withop);

        /**
         * Traversing the withops yields:
         *   a modifed withop chain in INFO_NEW_WITHOPS and
         *   a list of lhs variables (N_spids) for the first WL in INFO_HWLG_LHS
         */
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

        WITH_WITHOP (arg_node) = INFO_HWLG_NEW_WITHOPS (arg_info);
        INFO_HWLG_NEW_WITHOPS (arg_info) = NULL;

        first_let = TBmakeLet (INFO_HWLG_LHS (arg_info), first_wl);
        INFO_HWLG_LHS (arg_info) = NULL;

        arg_node = TRAVdo (arg_node, arg_info);

        INFO_HWLG_LASTASSIGN (arg_info)
          = TBmakeAssign (first_let, INFO_HWLG_LASTASSIGN (arg_info));
    } else {
        code = TRAVdo (WITH_CODE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *HWLGgenarray(node *arg_node, info *arg_info)
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
HWLGgenarray (node *arg_node, info *arg_info)
{
    char *tmp;
    node *new_withop;

    DBUG_ENTER ("HWLGgenarray");

    if (GENARRAY_NEXT (arg_node) != NULL) {
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    tmp = ILIBtmpVar ();

    new_withop = TBmakeModarray (TBmakeSpid (NULL, tmp));
    MODARRAY_NEXT (new_withop) = INFO_HWLG_NEW_WITHOPS (arg_info);
    INFO_HWLG_NEW_WITHOPS (arg_info) = new_withop;

    INFO_HWLG_LHS (arg_info)
      = TBmakeSpids (ILIBstringCopy (tmp), INFO_HWLG_LHS (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *HWLGmodarray(node *arg_node, info *arg_info)
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
HWLGmodarray (node *arg_node, info *arg_info)
{
    char *tmp;
    node *new_withop;

    DBUG_ENTER ("HWLGmodarray");

    if (MODARRAY_NEXT (arg_node) != NULL) {
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    tmp = ILIBtmpVar ();

    new_withop = TBmakeModarray (TBmakeSpid (NULL, tmp));
    MODARRAY_NEXT (new_withop) = INFO_HWLG_NEW_WITHOPS (arg_info);
    INFO_HWLG_NEW_WITHOPS (arg_info) = new_withop;

    INFO_HWLG_LHS (arg_info)
      = TBmakeSpids (ILIBstringCopy (tmp), INFO_HWLG_LHS (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *HWLGspfold(node *arg_node, info *arg_info)
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
HWLGspfold (node *arg_node, info *arg_info)
{
    char *tmp;
    node *new_withop;

    DBUG_ENTER ("HWLGspfold");

    if (SPFOLD_NEXT (arg_node) != NULL) {
        SPFOLD_NEXT (arg_node) = TRAVdo (SPFOLD_NEXT (arg_node), arg_info);
    }

    tmp = ILIBtmpVar ();

    new_withop = TBmakeSpfold (TBmakeSpid (NULL, tmp));
    SPFOLD_NS (new_withop) = NSdupNamespace (SPFOLD_NS (arg_node));
    SPFOLD_FUN (new_withop) = ILIBstringCopy (SPFOLD_FUN (arg_node));
    SPFOLD_GUARD (new_withop) = DUPdoDupTree (SPFOLD_GUARD (arg_node));

    SPFOLD_NEXT (new_withop) = INFO_HWLG_NEW_WITHOPS (arg_info);
    INFO_HWLG_NEW_WITHOPS (arg_info) = new_withop;

    INFO_HWLG_LHS (arg_info)
      = TBmakeSpids (ILIBstringCopy (tmp), INFO_HWLG_LHS (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *HWLGpropagate(node *arg_node, info *arg_info)
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
HWLGpropagate (node *arg_node, info *arg_info)
{
    char *tmp;
    node *new_withop;

    DBUG_ENTER ("HWLGpropagate");

    if (PROPAGATE_NEXT (arg_node) != NULL) {
        PROPAGATE_NEXT (arg_node) = TRAVdo (PROPAGATE_NEXT (arg_node), arg_info);
    }

    DBUG_ASSERT (NODE_TYPE (PROPAGATE_DEFAULT (arg_node)) == N_spid,
                 "propgate defaults should be N_spid!");
    tmp = ILIBstringCopy (SPID_NAME (PROPAGATE_DEFAULT (arg_node)));

    new_withop = TBmakePropagate (TBmakeSpid (NULL, tmp));
    PROPAGATE_NEXT (new_withop) = INFO_HWLG_NEW_WITHOPS (arg_info);
    INFO_HWLG_NEW_WITHOPS (arg_info) = new_withop;

    INFO_HWLG_LHS (arg_info)
      = TBmakeSpids (ILIBstringCopy (tmp), INFO_HWLG_LHS (arg_info));

    DBUG_RETURN (arg_node);
}
