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

#include "handle_with_loop_operators.h"

/**
 * INFO structure
 */
struct INFO {
    node *lastassign;
    int numops;
    node *cexprs;
    node *ncexprs;
    node *lhs;
    node *nlhs;
    node *withops;
};

/**
 * INFO macros
 */
#define INFO_HWLO_LASTASSIGN(n) (n->lastassign)
#define INFO_HWLO_NUM_STD_OPS(n) (n->numops)
#define INFO_HWLO_LHS(n) (n->lhs)
#define INFO_HWLO_NEW_LHS(n) (n->nlhs)
#define INFO_HWLO_CEXPRS(n) (n->cexprs)
#define INFO_HWLO_NEW_CEXPRS(n) (n->ncexprs)
#define INFO_HWLO_NEW_WITHOPS(n) (n->withops)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_HWLO_LASTASSIGN (result) = NULL;
    INFO_HWLO_NUM_STD_OPS (result) = 0;
    INFO_HWLO_CEXPRS (result) = NULL;
    INFO_HWLO_NEW_CEXPRS (result) = NULL;
    INFO_HWLO_LHS (result) = NULL;
    INFO_HWLO_NEW_LHS (result) = NULL;
    INFO_HWLO_NEW_WITHOPS (result) = NULL;

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
 * @fn node *HWLOdoHandleWithLoops( node *syntax_tree)
 *
 * @brief starts the splitting of mgen/mop With-Loops
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/

node *
HWLOdoHandleWithLoops (node *arg_node)
{
    info *info_node;

    DBUG_ENTER ("HWLOdoHandleWithLoops");

    info_node = MakeInfo ();

    TRAVpush (TR_hwlo);
    arg_node = TRAVdo (arg_node, info_node);
    TRAVpop ();

    info_node = FreeInfo (info_node);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *HWLOassign(node *arg_node, info *arg_info)
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
HWLOassign (node *arg_node, info *arg_info)
{
    node *mem_last_assign, *return_node;

    DBUG_ENTER ("HWLOassign");

    mem_last_assign = INFO_HWLO_LASTASSIGN (arg_info);
    INFO_HWLO_LASTASSIGN (arg_info) = arg_node;
    DBUG_PRINT ("HWLO", ("LASTASSIGN set to %08x!", arg_node));

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    /*
     * newly inserted abstractions are prepanded in front of
     * INFO_HWLO_LASTASSIGN(arg_info). To properly insert these nodes,
     * that pointer has to be returned:
     */
    return_node = INFO_HWLO_LASTASSIGN (arg_info);

    if (return_node != arg_node) {
        DBUG_PRINT ("HWLO", ("node %08x will be inserted instead of %08x", return_node,
                             arg_node));
    }
    INFO_HWLO_LASTASSIGN (arg_info) = mem_last_assign;
    DBUG_PRINT ("HWLO", ("LASTASSIGN (re)set to %08x!", mem_last_assign));

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (return_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *HWLOlet(node *arg_node, info *arg_info)
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
HWLOlet (node *arg_node, info *arg_info)
{
    node *mem_last_lhs;

    DBUG_ENTER ("HWLOlet");

    mem_last_lhs = INFO_HWLO_LHS (arg_info);
    if (NODE_TYPE (LET_EXPR (arg_node)) == N_with) {
        INFO_HWLO_LHS (arg_info) = LET_IDS (arg_node);

        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
        LET_IDS (arg_node) = INFO_HWLO_LHS (arg_info);
    } else {
        INFO_HWLO_LHS (arg_info) = NULL;

        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    INFO_HWLO_LHS (arg_info) = mem_last_lhs;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *HWLOwith(node *arg_node, info *arg_info)
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
HWLOwith (node *arg_node, info *arg_info)
{
    node *new_let, *new_part, *new_code;

    DBUG_ENTER ("HWLOwith");

    DBUG_ASSERT (CODE_NEXT (WITH_CODE (arg_node)) == NULL,
                 "HWLO requires all WLs to be single-generator!");

    INFO_HWLO_CEXPRS (arg_info) = CODE_CEXPRS (WITH_CODE (arg_node));

    if (TCcountExprs (INFO_HWLO_CEXPRS (arg_info)) > 1) {
        if (INFO_HWLO_LHS (arg_info) == NULL) {
            CTIerrorLine (global.linenum,
                          "Multi-Operator With-Loop used in expression position");
            CTIabortOnError ();
        }
        /**
         * Traversing the withops yields:
         *   the number of std-WLops, i.e., (gen/mod/fold) in INFO_HWLO_NUM_STD_OPS
         *   iff that number is >1, we also obtain:
         *     - a new chain of (extracted (stdwlop) / copied (propagate)) wlops in
         *       INFO_HWLO_NEW_WITHOPS
         *     - a new exprs chain of (extracted (stdwlop) / copied (propagate)) exprs
         *       derived from INFO_HWLO_CEXPRS in INFO_HWLO_NEW_CEXPRS
         *     - a new list of (extracted (stdwlop) / copied (propagate)) lhs
         *       variables (N_spids) derived from INFO_HWLO_LHS in INFO_HWLO_NEW_LHS
         *     Note here, that the existing chaines are being modified!!!
         */
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

        if (INFO_HWLO_NUM_STD_OPS (arg_info) > 1) {
            CODE_CEXPRS (WITH_CODE (arg_node)) = INFO_HWLO_CEXPRS (arg_info);

            new_part = DUPdoDupTree (WITH_PART (arg_node));
            new_code = TBmakeCode (DUPdoDupTree (CODE_CBLOCK (WITH_CODE (arg_node))),
                                   INFO_HWLO_NEW_CEXPRS (arg_info));
            new_code = TRAVdo (new_code, arg_info);
            PART_CODE (new_part) = new_code;
            CODE_USED (new_code)++;

            new_let = TBmakeLet (INFO_HWLO_NEW_LHS (arg_info),
                                 TBmakeWith (new_part, new_code,
                                             INFO_HWLO_NEW_WITHOPS (arg_info)));

            INFO_HWLO_NEW_WITHOPS (arg_info) = NULL;
            INFO_HWLO_NEW_CEXPRS (arg_info) = NULL;
            INFO_HWLO_NEW_LHS (arg_info) = NULL;
            INFO_HWLO_NUM_STD_OPS (arg_info) = 0;

            arg_node = TRAVdo (arg_node, arg_info);

            INFO_HWLO_LASTASSIGN (arg_info)
              = TBmakeAssign (new_let, INFO_HWLO_LASTASSIGN (arg_info));
        } else {
            INFO_HWLO_NUM_STD_OPS (arg_info) = 0;
        }
    } else {
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

static node *
StdWithOp (node *arg_node, info *arg_info)
{
    node *my_lhs, *my_cexprs, *return_node;
    int my_pos;

    DBUG_ENTER ("StdWithOp");

    INFO_HWLO_NUM_STD_OPS (arg_info)++;

    my_pos = INFO_HWLO_NUM_STD_OPS (arg_info);
    my_cexprs = INFO_HWLO_CEXPRS (arg_info);
    my_lhs = INFO_HWLO_LHS (arg_info);

    INFO_HWLO_CEXPRS (arg_info) = EXPRS_NEXT (my_cexprs);
    INFO_HWLO_LHS (arg_info) = SPIDS_NEXT (my_lhs);

    if (WITHOP_NEXT (arg_node) != NULL) {
        if (EXPRS_NEXT (my_cexprs) == NULL) {
            CTIerrorLine (global.linenum,
                          "more operator parts than body expressions in with loop");
        }
        if (SPIDS_NEXT (my_lhs) == NULL) {
            CTIerrorLine (global.linenum, "more operator parts in with loop than left "
                                          "hand side variables");
        }
        CTIabortOnError ();

        L_WITHOP_NEXT (arg_node, TRAVdo (WITHOP_NEXT (arg_node), arg_info));
    } else {
        if (EXPRS_NEXT (my_cexprs) != NULL) {
            CTIerrorLine (global.linenum,
                          "less operator parts than body expressions in with loop");
        }
        if (SPIDS_NEXT (my_lhs) != NULL) {
            CTIerrorLine (global.linenum, "less operator parts in with loop than left "
                                          "hand side variables");
        }
        CTIabortOnError ();
    }

    if ((INFO_HWLO_NUM_STD_OPS (arg_info) > 1) && (my_pos == 1)) {
        EXPRS_NEXT (my_cexprs) = INFO_HWLO_NEW_CEXPRS (arg_info);
        INFO_HWLO_NEW_CEXPRS (arg_info) = my_cexprs;

        SPIDS_NEXT (my_lhs) = INFO_HWLO_NEW_LHS (arg_info);
        INFO_HWLO_NEW_LHS (arg_info) = my_lhs;

        return_node = WITHOP_NEXT (arg_node);
        L_WITHOP_NEXT (arg_node, INFO_HWLO_NEW_WITHOPS (arg_info));
        INFO_HWLO_NEW_WITHOPS (arg_info) = arg_node;
        arg_node = return_node;
    } else {
        EXPRS_NEXT (my_cexprs) = INFO_HWLO_CEXPRS (arg_info);
        INFO_HWLO_CEXPRS (arg_info) = my_cexprs;

        SPIDS_NEXT (my_lhs) = INFO_HWLO_LHS (arg_info);
        INFO_HWLO_LHS (arg_info) = my_lhs;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *HWLOgenarray(node *arg_node, info *arg_info)
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
HWLOgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("HWLOgenarray");

    DBUG_RETURN (StdWithOp (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *HWLOmodarray(node *arg_node, info *arg_info)
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
HWLOmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("HWLOmodarray");

    DBUG_RETURN (StdWithOp (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *HWLOspfold(node *arg_node, info *arg_info)
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
HWLOspfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("HWLOspfold");

    DBUG_RETURN (StdWithOp (arg_node, arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn node *HWLOpropagate(node *arg_node, info *arg_info)
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
HWLOpropagate (node *arg_node, info *arg_info)
{
    char *tmp;
    node *my_lhs, *my_cexprs, *new_withop;

    DBUG_ENTER ("HWLOpropagate");

    my_cexprs = INFO_HWLO_CEXPRS (arg_info);
    my_lhs = INFO_HWLO_LHS (arg_info);

    INFO_HWLO_CEXPRS (arg_info) = EXPRS_NEXT (my_cexprs);
    INFO_HWLO_LHS (arg_info) = SPIDS_NEXT (my_lhs);

    if (PROPAGATE_NEXT (arg_node) != NULL) {
        if (EXPRS_NEXT (my_cexprs) == NULL) {
            CTIerror ("more operator parts than body expressions in with loop");
        }
        if (SPIDS_NEXT (my_lhs) == NULL) {
            CTIerror ("more operator parts in with loop than left hand side variables");
        }
        CTIabortOnError ();

        PROPAGATE_NEXT (arg_node) = TRAVdo (PROPAGATE_NEXT (arg_node), arg_info);
    } else {
        if (EXPRS_NEXT (my_cexprs) != NULL) {
            CTIerrorLine (global.linenum,
                          "less operator parts than body expressions in with loop");
        }
        if (SPIDS_NEXT (my_lhs) != NULL) {
            CTIerrorLine (global.linenum, "less operator parts in with loop than left "
                                          "hand side variables");
        }
        CTIabortOnError ();
    }

    if (INFO_HWLO_NUM_STD_OPS (arg_info) > 1) {

        DBUG_ASSERT (NODE_TYPE (PROPAGATE_DEFAULT (arg_node)) == N_spid,
                     "propgate defaults should be N_spid!");
        tmp = ILIBstringCopy (SPID_NAME (PROPAGATE_DEFAULT (arg_node)));

        new_withop = TBmakePropagate (TBmakeSpid (NULL, tmp));
        PROPAGATE_NEXT (new_withop) = INFO_HWLO_NEW_WITHOPS (arg_info);

        INFO_HWLO_NEW_WITHOPS (arg_info) = new_withop;

        INFO_HWLO_NEW_LHS (arg_info)
          = TBmakeSpids (ILIBstringCopy (tmp), INFO_HWLO_NEW_LHS (arg_info));

        INFO_HWLO_NEW_CEXPRS (arg_info)
          = TBmakeExprs (DUPdoDupTree (EXPRS_EXPR (my_cexprs)),
                         INFO_HWLO_NEW_CEXPRS (arg_info));
    }

    EXPRS_NEXT (my_cexprs) = INFO_HWLO_CEXPRS (arg_info);
    INFO_HWLO_CEXPRS (arg_info) = my_cexprs;

    SPIDS_NEXT (my_lhs) = INFO_HWLO_LHS (arg_info);
    INFO_HWLO_LHS (arg_info) = my_lhs;

    DBUG_RETURN (arg_node);
}
