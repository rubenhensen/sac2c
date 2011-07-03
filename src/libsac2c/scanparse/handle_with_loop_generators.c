/*
 * $Id$
 */

#define DBUG_PREFIX "HWLG"
#include "debug.h"

#include "globals.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
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
 *
 * are semantically equivalent to
 *                                                 fun( tmp, fun( tmp2, tmp3));
 *     with {                    with {                    tmp3 = with {
 *       ( <p3> ) : e3;            ( <p3> ) : e3;            ( <p3> ) : e3;
 *     } modarray( tmp)          } modarray( tmp)          } fold( fun, neutr)
 *
 * provided the variable tmp is defined as
 *
 * tmp = with {                  tmp = with {              tmp = with {
 *         ( <p2> ) : e2;                ( <p2> ) : e2;            ( <p2> ) : e2;
 *       } modarray( tmp2);            } modarray( tmp2);        } fold( fun, neutr);
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

typedef enum { T_traverse, T_create } traversal_mode_t;

/**
 * INFO structure
 */
struct INFO {
    node *lastassign;
    node *lhs;
    node *withops;
    /*
     * Chain of exprs that point to NULL if non fold or to a spap if
     * corresponds to fold withop
     * NULL's are present to line up with spids of lhs of with loop
     */
    node *fold;
    node *postassign;
    traversal_mode_t withop_traversal_mode;
};

/**
 * INFO macros
 */
#define INFO_HWLG_LASTASSIGN(n) (n->lastassign)
#define INFO_HWLG_LHS(n) (n->lhs)
#define INFO_HWLG_NEW_WITHOPS(n) (n->withops)
#define INFO_HWLG_MODE(n) (n->withop_traversal_mode)
#define INFO_FOLD(n) (n->fold)
#define INFO_POSTASSIGN(n) (n->postassign)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_HWLG_LASTASSIGN (result) = NULL;
    INFO_HWLG_LHS (result) = NULL;
    INFO_HWLG_NEW_WITHOPS (result) = NULL;
    INFO_HWLG_MODE (result) = T_traverse;
    INFO_FOLD (result) = NULL;
    INFO_POSTASSIGN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    /*  DBUG_ASSERT( ( INFO_FOLD( info) == NULL),
        "Freeing non empty info");*/

    DBUG_ASSERT (INFO_POSTASSIGN (info) == NULL, "Freeing info with post assigns");

    info = MEMfree (info);

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

    DBUG_ENTER ();

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
    node *mem_last_assign, *return_node, *mem_postassign;

    DBUG_ENTER ();

    mem_last_assign = INFO_HWLG_LASTASSIGN (arg_info);
    INFO_HWLG_LASTASSIGN (arg_info) = arg_node;
    DBUG_PRINT ("LASTASSIGN set to %08x!", arg_node);

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    /*
     * newly inserted abstractions are prepanded in front of
     * INFO_HWLG_LASTASSIGN(arg_info). To properly insert these nodes,
     * that pointer has to be returned:
     */
    return_node = INFO_HWLG_LASTASSIGN (arg_info);

    if (return_node != arg_node) {
        DBUG_PRINT ("node %08x will be inserted instead of %08x", return_node, arg_node);
    }
    INFO_HWLG_LASTASSIGN (arg_info) = mem_last_assign;
    DBUG_PRINT ("LASTASSIGN (re)set to %08x!", mem_last_assign);

    mem_postassign = INFO_POSTASSIGN (arg_info);
    INFO_POSTASSIGN (arg_info) = NULL;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_NEXT (arg_node) = TCappendAssign (mem_postassign, ASSIGN_NEXT (arg_node));

    DBUG_RETURN (return_node);
}
/** <!--********************************************************************-->
 *
 * @fn node *InsertInitial(node *fun, char *var)
 *
 * @brief given a chain of 2 argument functions where the function chain
 *        continues on the second argument of the function place the var
 *        as the initial value of the chain
 *
 *        f( ..., f( ..., f( ..., NULL);
 *
 *        f( ..., f( ..., f( ...., spid(var));
 *
 *****************************************************************************/

static node *
InsertInitial (node *fun, char *var)
{
    DBUG_ENTER ();

    if (fun != NULL) {
        if (EXPRS_EXPR (EXPRS_NEXT (SPAP_ARGS (fun))) == NULL) {
            EXPRS_EXPR (EXPRS_NEXT (SPAP_ARGS (fun))) = TBmakeSpid (NULL, var);
        } else {
            EXPRS_EXPR (EXPRS_NEXT (SPAP_ARGS (fun)))
              = InsertInitial (EXPRS_EXPR (EXPRS_NEXT (SPAP_ARGS (fun))), var);
        }
    }

    DBUG_RETURN (fun);
}

/** <!--********************************************************************-->
 *
 * @fn node *RenameLhs(node *arg_node, info *arg_info)
 *
 * @brief Find all lhs' that are for folds.  Rename these to a new var
 *        and finish the creation of the f(a,b..) assignment using with lhs for
 *        f(a,b...)'s lhs
 *
 *        a = with {} fold(f, ...);
 *
 *        a' = with{} fold(f, ...);
 *        a = f( ..., ... f(..., a') ...);
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/
static node *
RenameLhs (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (arg_node != NULL) {

        if (EXPRS_EXPR (INFO_FOLD (arg_info)) == NULL) {
            /* non fold lhs */
            INFO_FOLD (arg_info) = FREEdoFreeNode (INFO_FOLD (arg_info));
            SPIDS_NEXT (arg_node) = RenameLhs (SPIDS_NEXT (arg_node), arg_info);
        } else {
            /* fold lhs */
            char *newVar = TRAVtmpVar ();
            node *fun = DUPdoDupTree (EXPRS_EXPR (INFO_FOLD (arg_info)));
            node *next = SPIDS_NEXT (arg_node);
            INFO_FOLD (arg_info) = FREEdoFreeNode (INFO_FOLD (arg_info));

            fun = InsertInitial (fun, newVar);

            SPIDS_NEXT (arg_node) = NULL;
            INFO_POSTASSIGN (arg_info)
              = TBmakeAssign (TBmakeLet (arg_node, fun), INFO_POSTASSIGN (arg_info));

            arg_node = TBmakeSpids (STRcpy (newVar), RenameLhs (next, arg_info));
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *HWLGlet(node *arg_node, info *arg_info)
 *
 * @brief If we have fold withloop replace lhs for fold with new var and
 *        use old lhs for result of fun( a, ...).
 *        Create new lhs and place in inner nesting of fold fun.
 *
 * @param arg_node N_with to be potentially transformed
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/
node *
HWLGlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    arg_node = TRAVcont (arg_node, arg_info);

    if (INFO_FOLD (arg_info) != NULL) {
        LET_IDS (arg_node) = RenameLhs (LET_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *HWLGwith(node *arg_node, info *arg_info)
 *
 * @brief creates a sequences of With-Loop assignments with Single-Generator
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

    DBUG_ENTER ();

    if (WITH_PART (arg_node) == NULL) {
        /**
         * no generators at all
         */
        DBUG_ASSERT (WITH_CODE (arg_node) == NULL,
                     "found a WL w/o generators, but with code blocks!");
    } else if ((PART_NEXT (WITH_PART (arg_node)) != NULL)
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
    node *old_lastassign, *new_assigns = NULL;

    DBUG_ENTER ();

    /**
     * First, we extract potential Multi-Generator With-Loops within
     * the withops. These may occur as we are run prior to flatten!
     */
    if (WITH_WITHOP (arg_node) != NULL) {
        DBUG_PRINT ("N_with found; traversing withops now:");

        old_lastassign = INFO_HWLG_LASTASSIGN (arg_info);
        INFO_HWLG_LASTASSIGN (arg_info) = NULL;
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
        new_assigns = INFO_HWLG_LASTASSIGN (arg_info);
        INFO_HWLG_LASTASSIGN (arg_info) = old_lastassign;
    }

    DBUG_PRINT ("              splitting generators now:");
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

    DBUG_ENTER ();

    if (INFO_HWLG_MODE (arg_info) == T_create) {

        if (INFO_FOLD (arg_info) != NULL) {
            INFO_FOLD (arg_info) = FREEdoFreeNode (INFO_FOLD (arg_info));
        }

        if (GENARRAY_NEXT (arg_node) != NULL) {
            GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
        }

        INFO_FOLD (arg_info) = TBmakeExprs (NULL, INFO_FOLD (arg_info));

        tmp = TRAVtmpVar ();

        new_withop = TBmakeModarray (TBmakeSpid (NULL, tmp));
        MODARRAY_NEXT (new_withop) = INFO_HWLG_NEW_WITHOPS (arg_info);
        INFO_HWLG_NEW_WITHOPS (arg_info) = new_withop;

        INFO_HWLG_LHS (arg_info) = TBmakeSpids (STRcpy (tmp), INFO_HWLG_LHS (arg_info));
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

    DBUG_ENTER ();

    if (INFO_HWLG_MODE (arg_info) == T_create) {

        if (INFO_FOLD (arg_info) != NULL) {
            INFO_FOLD (arg_info) = FREEdoFreeNode (INFO_FOLD (arg_info));
        }

        if (MODARRAY_NEXT (arg_node) != NULL) {
            MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
        }

        INFO_FOLD (arg_info) = TBmakeExprs (NULL, INFO_FOLD (arg_info));

        tmp = TRAVtmpVar ();

        new_withop = TBmakeModarray (TBmakeSpid (NULL, tmp));
        MODARRAY_NEXT (new_withop) = INFO_HWLG_NEW_WITHOPS (arg_info);
        INFO_HWLG_NEW_WITHOPS (arg_info) = new_withop;

        INFO_HWLG_LHS (arg_info) = TBmakeSpids (STRcpy (tmp), INFO_HWLG_LHS (arg_info));
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
    node *current_fold = NULL;

    DBUG_ENTER ();

    if (INFO_HWLG_MODE (arg_info) == T_create) {

        if (INFO_FOLD (arg_info) != NULL) {
            /* Already have atleast one f( a, b) */
            current_fold = INFO_FOLD (arg_info);
            INFO_FOLD (arg_info) = EXPRS_NEXT (current_fold);
        }

        arg_node = TRAVcont (arg_node, arg_info);

        if (current_fold != NULL) {
            EXPRS_NEXT (current_fold) = INFO_FOLD (arg_info);
            INFO_FOLD (arg_info) = current_fold;
        } else {
            INFO_FOLD (arg_info) = TBmakeExprs (NULL, INFO_FOLD (arg_info));
        }

        new_withop = TBmakeSpfold (DUPdoDupTree (SPFOLD_NEUTRAL (arg_node)));
        SPFOLD_NS (new_withop) = NSdupNamespace (SPFOLD_NS (arg_node));
        SPFOLD_FUN (new_withop) = STRcpy (SPFOLD_FUN (arg_node));
        SPFOLD_GUARD (new_withop) = DUPdoDupTree (SPFOLD_GUARD (arg_node));

        SPFOLD_NEXT (new_withop) = INFO_HWLG_NEW_WITHOPS (arg_info);
        INFO_HWLG_NEW_WITHOPS (arg_info) = new_withop;

        tmp = TRAVtmpVar ();

        INFO_HWLG_LHS (arg_info) = TBmakeSpids (STRcpy (tmp), INFO_HWLG_LHS (arg_info));

        {
            node *exprRight
              = TBmakeExprs (DUPdoDupTree (EXPRS_EXPR (INFO_FOLD (arg_info))), NULL);
            node *exprDown = FREEdoFreeNode (INFO_FOLD (arg_info));
            node *funName = TBmakeSpid (NSdupNamespace (SPFOLD_NS (arg_node)),
                                        STRcpy (SPFOLD_FUN (arg_node)));
            node *var = TBmakeSpid (NULL, STRcpy (tmp));

            INFO_FOLD (arg_info)
              = TBmakeExprs (TBmakeSpap (funName, TBmakeExprs (var, exprRight)),
                             exprDown);
        }
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

    DBUG_ENTER ();

    if (INFO_HWLG_MODE (arg_info) == T_create) {

        if (INFO_FOLD (arg_info) != NULL) {
            INFO_FOLD (arg_info) = FREEdoFreeNode (INFO_FOLD (arg_info));
        }

        if (PROPAGATE_NEXT (arg_node) != NULL) {
            PROPAGATE_NEXT (arg_node) = TRAVdo (PROPAGATE_NEXT (arg_node), arg_info);
        }

        INFO_FOLD (arg_info) = TBmakeExprs (NULL, INFO_FOLD (arg_info));

        DBUG_ASSERT (NODE_TYPE (PROPAGATE_DEFAULT (arg_node)) == N_spid,
                     "propgate defaults should be N_spid!");
        tmp = STRcpy (SPID_NAME (PROPAGATE_DEFAULT (arg_node)));

        new_withop = TBmakePropagate (TBmakeSpid (NULL, tmp));
        PROPAGATE_NEXT (new_withop) = INFO_HWLG_NEW_WITHOPS (arg_info);
        INFO_HWLG_NEW_WITHOPS (arg_info) = new_withop;

        INFO_HWLG_LHS (arg_info) = TBmakeSpids (STRcpy (tmp), INFO_HWLG_LHS (arg_info));
    } else {
        arg_node = TRAVcont (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
