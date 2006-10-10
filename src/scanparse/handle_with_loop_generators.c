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
 * This traversal transformes Multi-Generator With-Loops into sequences of
 * Single-Generator With-loops. To understand the basic principle, let us look
 * at the three base cases:
 * Expressions of the forms:
 *
 *    with {                    with {                    with {
 *       ( <p1> ) : e1;            ( <p1> ) : e1;            ( <p1> ) : e1;
 *       ( <p2> ) : e2;            ( <p2> ) : e2;            ( <p2> ) : e2;
 *       ( <p3> ) : e3;            ( <p3> ) : e3;            ( <p3> ) : e3;
 *     } genarray( shp, def)     } modarray( a)            } fold( fun, neutr)
 *
 * are semantically equivalent to
 *
 *     with {                    with {                    with {
 *       ( <p3> ) : e3;            ( <p3> ) : e3;            ( <p3> ) : e3;
 *     } modarray( tmp)          } modarray( tmp)          } fold( fun, tmp)
 *
 * provided the variable tmp is defined as
 *
 * tmp = with {                  tmp = with {              tmp = with {
 *         ( <p2> ) : e2;                ( <p2> ) : e2;            ( <p2> ) : e2;
 *       } modarray( tmp2);            } modarray( tmp2);        } fold( fun, tmp2);
 *
 * and tmp2 is defined as
 *
 * tmp2 = with {                 tmp2 = with {             tmp2 = with {
 *          ( <p1> ) : e1;                ( <p1> ) : e1;            ( <p1> ) : e1;
 *        } genarray( shp, def);        } modarray( a);           } fold( fun, neutr);
 *
 * Note here, that
 *   a) It does not matter where the original with-loop resides, i.e., it can be in any
 *      expression position! As a consequence, this traversal can be run prior to flatten!
 *      All we need to make sure is to place the freshly created tmp variables, more
 * precisely their definitions, correcly. We need to ensure that the With-Loop is in their
 * scope.
 *
 *   b) This transformation can also be applied if we are dealing with Multi-Operator
 *      With-Loops! In that case, we simply map the transformation as outlined above to
 *      all operation parts.
 *
 * As for the newer operation parts, i.e., foldfix and propagate, we apply the following
 * transformations:
 *
 *     with {                               with {
 *       ( <p1> ) : e1;                       ( <p1> ) : e1;
 *       ( <p2> ) : e2;                       ( <p2> ) : e2;
 *       ( <p3> ) : e3;                       ( <p3> ) : e3;
 *     } foldfix( fun, neutr, fix)          } propagate( x)
 *
 * turn into
 *
 *     with {                               with {
 *       ( <p3> ) : e3;                       ( <p3> ) : e3;
 *     } foldfix( fun, tmp, fix)            } propagate( x)
 *
 * where
 *
 * tmp = with {                             x = with {
 *         ( <p2> ) : e2;                         ( <p2> ) : e2;
 *       } foldfix( fun, tmp2, fix)             } propagate( x);
 *
 * and
 *
 * tmp2 = with {                            x = with {
 *          ( <p1> ) : e1;                        ( <p1> ) : e1;
 *        } foldfix( fun, neutr, fix);          } propagate( x);
 *
 * While the foldfix very closely resembles the standard fold, the most striking
 * difference for propagate is the reuse of the propagated variable name. This is due to
 * the fact that within the individual code bodies e1, e2, and e3, the incoming object is
 * always the same, more precisely: x.
 *
 * Note here, that the reuse of the variable x does not comply with the ssa form. Hence,
 * this taversal either needs to be run prior to ssa conversion, or ssa form needs to be
 * re-generated afterwards!
 *
 * Outline of the Implementation
 * =============================
 *
 * While we traverse through the syntax tree, most of the action happens at the N_with
 * node and in the withop nodes N_genarray, N_modarray, N_spfold, and N_propagate. Apart
 * from these, we only take further interest in the N_assign nodes. There, we ensure that
 * INFO_HWLG_LASTASSIGN always points to the assignment in whose scope we are currently
 * traversing. If we transform a With-Loop, we insert new assignments here, and, once we
 * come back we insert the new assignments into the tree. Note here, that any traversal on
 * newly generated assignments needs to be done prior to insertion as the code on N_assign
 * does NOT traverse these anymore! The With-Loop transformation itself happens at the
 * N_with node in a recursive fashion. As long as we have more than one generator, we
 * split off the first one, generate a new With-Loop, apply the traversal recursively on
 * the remaining With-Loop, and, AFTER that, insert the new With-Loop in
 * INFO_HWLG_LASTASSIGN. This allows us to prepand the With-Loops without violating the
 * required data dependencies! In order to create the new (split-off) With-Loops, we do so
 * while traversing the individual withop nodes. That way, handling Multi-Operator
 * With-Loops properly comes for free! However, since we are most liekely pre-flatten, we
 * need to traverse the withops as well if we do not want to create new withops.
 * Therefore, we have two withop traversal modi: T_traverse and T_create.
 *
 * NB: As we assume this phase to be applied rather early in the compiler, we expect all
 * identifiers to be in N_spid / N_spids still! Application of this phase in a later stage
 * would require an extension to N_id, N_ids, N_fold and, potentially, N_break !
 *
 */

typedef enum { T_traverse, T_create } mode_t;

/**
 * INFO structure
 */
struct INFO {
    node *lastassign;
    node *lhs;
    node *withops;
    mode_t withop_traversal_mode;
};

/**
 * INFO macros
 */
#define INFO_HWLG_LASTASSIGN(n) (n->lastassign)
#define INFO_HWLG_LHS(n) (n->lhs)
#define INFO_HWLG_NEW_WITHOPS(n) (n->withops)
#define INFO_HWLG_MODE(n) (n->withop_traversal_mode)

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
    INFO_HWLG_MODE (result) = T_traverse;

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
 * @brief ensures that INFO_HWLG_LASTASSIGN always points to the last assignment
 *        in whose scope we are traversing. After traversing the rhs, assignments
 *        that might have been prepanded are inserted into the syntax tree.
 *        Note here, that such newly inserted assignments are NOT traversed here!
 *
 * @param arg_node N_assign to be traversed
 * @param arg_info INFO_HWLG_LASTASSIGN is preserved but modified during traversal
 *                 of the rhs ASSIGN_INSTR.
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
 * @brief creates aa sequences of With-Loop assignments with Single-Generator
 *        With-Loops from Multi-Generator With-Loops.
 *
 * @param arg_node N_with to be potentially transformed
 * @param arg_info uses INFO_HWLG_LASTASSIGN for inserting new assignments
 *                 temporarily uses INFO_HWLG_NEW_WITHOPS and INFO_HWLG_LHS
 *                 for collecting the withops / lhs vars during traversal
 *                 of the withops. Exits with these being set to NULL
 *
 * @return arg_node
 *
 *****************************************************************************/

static node *
SplitWith (node *arg_node, info *arg_info)
{
    node *part, *code, *withop, *first_wl, *first_let;

    DBUG_ENTER ("SplitWith");

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
        if (WITH_WITHOP (arg_node) != NULL) {
            DBUG_ASSERT (INFO_HWLG_MODE (arg_info) == T_traverse,
                         "withop mode non traverse in HWLGwith found!");
            INFO_HWLG_MODE (arg_info) = T_create;
            WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
            INFO_HWLG_MODE (arg_info) = T_traverse;
        }

        WITH_WITHOP (arg_node) = INFO_HWLG_NEW_WITHOPS (arg_info);
        INFO_HWLG_NEW_WITHOPS (arg_info) = NULL;

        first_let = TBmakeLet (INFO_HWLG_LHS (arg_info), first_wl);
        INFO_HWLG_LHS (arg_info) = NULL;

        arg_node = SplitWith (arg_node, arg_info);

        INFO_HWLG_LASTASSIGN (arg_info)
          = TBmakeAssign (first_let, INFO_HWLG_LASTASSIGN (arg_info));
        /**
         * Now the extracted part can be traversed as the new With-Loop has been
         * inserted already. This preserves the data dependency in case we have to
         * extract something from the part!
         */
        WITH_PART (first_wl) = TRAVdo (WITH_PART (first_wl), arg_info);
    } else {
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
HWLGwith (node *arg_node, info *arg_info)
{
    node *old_lastassign, *new_assigns;

    DBUG_ENTER ("HWLGwith");

    DBUG_PRINT ("HWLG", ("N_with found; traversing withops now:"));
    /**
     * First, we extract potential Multi-Generator With-Loops within
     * the withops. These may occur as we are run prior to flatten!
     */
    old_lastassign = INFO_HWLG_LASTASSIGN (arg_info);
    INFO_HWLG_LASTASSIGN (arg_info) = NULL;
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    new_assigns = INFO_HWLG_LASTASSIGN (arg_info);
    INFO_HWLG_LASTASSIGN (arg_info) = old_lastassign;

    DBUG_PRINT ("HWLG", ("              splitting generators now:"));
    /**
     * Now, we recursively split Multi-Generator With-Loops:
     */
    arg_node = SplitWith (arg_node, arg_info);

    /**
     * Finally, we insert the potentially extracted with-loops:
     */
    if (new_assigns != NULL) {
        INFO_HWLG_LASTASSIGN (arg_info)
          = TCappendAssign (new_assigns, INFO_HWLG_LASTASSIGN (arg_info));
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

    if (INFO_HWLG_MODE (arg_info) == T_create) {

        if (GENARRAY_NEXT (arg_node) != NULL) {
            GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
        }

        tmp = ILIBtmpVar ();

        new_withop = TBmakeModarray (TBmakeSpid (NULL, tmp));
        MODARRAY_NEXT (new_withop) = INFO_HWLG_NEW_WITHOPS (arg_info);
        INFO_HWLG_NEW_WITHOPS (arg_info) = new_withop;

        INFO_HWLG_LHS (arg_info)
          = TBmakeSpids (ILIBstringCopy (tmp), INFO_HWLG_LHS (arg_info));
    } else {
        arg_node = TRAVcont (arg_node, arg_info);
    }

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

    if (INFO_HWLG_MODE (arg_info) == T_create) {
        if (MODARRAY_NEXT (arg_node) != NULL) {
            MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
        }

        tmp = ILIBtmpVar ();

        new_withop = TBmakeModarray (TBmakeSpid (NULL, tmp));
        MODARRAY_NEXT (new_withop) = INFO_HWLG_NEW_WITHOPS (arg_info);
        INFO_HWLG_NEW_WITHOPS (arg_info) = new_withop;

        INFO_HWLG_LHS (arg_info)
          = TBmakeSpids (ILIBstringCopy (tmp), INFO_HWLG_LHS (arg_info));
    } else {
        arg_node = TRAVcont (arg_node, arg_info);
    }

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

    if (INFO_HWLG_MODE (arg_info) == T_create) {
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
    } else {
        arg_node = TRAVcont (arg_node, arg_info);
    }

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

    if (INFO_HWLG_MODE (arg_info) == T_create) {
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
    } else {
        arg_node = TRAVcont (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}
