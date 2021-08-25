#define DBUG_PREFIX "HWLO"
#include "debug.h"

#include "globals.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "compare_tree.h"
#include "str.h"
#include "memory.h"
#include "DupTree.h"
#include "free.h"
#include "namespaces.h"
#include "ctinfo.h"

#include "handle_with_loop_operators.h"

/**
 * This traversal transformes Single-Generator Multi-Operator With-Loops into
 * sequences of Single-Generator With-Loops that contain at most one operator
 * of the three standard operators {genarray, modarray, and fold}.
 * NB: The reason why we want to do this is that we do not want to add full
 * support for Multi-Operator With-Loops with arbitrary ranges in the back-end.
 * (At the time being 10.10.2006, we require Multi-Operator With-Loops in the
 * back-end to have identical overall ranges....)
 *
 * As of 21.11.2018, we still do not support Multi-Operator With-Loops with
 * arbitrary ranges in the back-end. However, as WLFS is not effective enough
 * we want to avoid splitting multi-operator WLs that have only generators with
 * identical range or propagate operators.
 * We implement a check in the local function IsLegitimateMoWl (node *withop, info*)
 * which is being used in HWLOwith to prevent splitting.
 *
 * As of 21.8.2021, we try to extend the capabilities for keeping Multi-Operator
 * With-Loops even further. This time, we want to support With-Loops that contain
 * several modarray operators provided they modify the same array and we want to
 * support multiple fold-operators. These should be straight forward as single generator 
 * fold with-loops solely iterate over the generator guaranteeing identical ranges.
 * We extend IsLegitimateMoWl accordingly.
 *
 *
 * To understand the basic principle of splitting Multi-Operator With-Loops,
 * let us consider Multi-Operator With-Loops that contain standard operators
 * only, i.e., they do NOT contain propagates.
 * In these cases, we can split the operators into individual ones by simply
 * copying the entire With-Lopp body and by splitting the body-returns and
 * left hand sides accordingly. An assignment of the form
 *
 * a, b, c = with {
 *             ( [0] <= iv <= [5]) : ( 1, 2, 3);
 *           } ( genarray( [10]), modarray( x), fold( +, 0));
 *
 * is semantically equivalent to
 *
 * a = with {
 *       ( [0] <= iv <= [5]) : 1;
 *     } genarray( [10]);
 * b = with {
 *       ( [0] <= iv <= [5]) : 2;
 *     } modarray( x);
 * c = with {
 *       ( [0] <= iv <= [5]) : 3;
 *     } fold( +, 0);
 *
 * However, we have to be a bit careful here. By splitting the assignment into
 * several subsequent ones we might introduce parasitic bindings. Consider
 * a slight variation of the example above: rather than having an operator
 * "modarray( x)" we could have had an operator "modarray( a)". That "a" would
 * have referred to an array "a" defined earlier. If we now split naively as
 * outlined above, "a" would refer to the array generated by the first
 * With-Loop :-(
 * As a consequence, we need to introduce fresh variable names for the
 * split-off With-Loops and do a variable renaming afterwards.
 * Thus, we obtain:
 *
 * tmp_a = with {
 *           ( [0] <= iv <= [5]) : 1;
 *         } genarray( [10]);
 * tmp_b = with {
 *           ( [0] <= iv <= [5]) : 2;
 *         } modarray( x);
 * tmp_c = with {
 *           ( [0] <= iv <= [5]) : 3;
 *         } fold( +, 0);
 * a = tmp_a;
 * b = tmp_b;
 * c = tmp_c;
 *
 * So far, we have considered standard operators only. As soon as we introduce
 * propagate operators, the situation becomes more difficult.
 * As propagate operators can adapt to any range, in principle, we do not have
 * to split-off these at all. However, if we have to split a Multi-Operator
 * With-Loop that contains more than one standard operator, the question arises
 * where we put the propagate opertors. Essentially, three different approaches
 * may seem to be feasible at first sight here:
 *
 * APPROACH 1:
 *
 * we simply forbid Multi-Operator With-Loops that contain more than one
 * standard operator AND propagate operators.
 *
 * The drawbacks of this approach are:
 * - it seems to be a rather artificial restriction that is solely imposed
 *   by implementation limitations (which in fact is true;-).
 * - the illegal situation might be less obvious than one would assume as
 *   not all propagates are explicit in SaC. Using print within the
 *   With-Loop body suffices to make the compiler reject a program:-(
 *
 * APPROACH 2:
 *
 * we try to split-off all propagates as well.
 *
 * The drawback of this approach is:
 * - we may not be able to do that at all. Consider the following With-Loop:
 *
 *   a, file = with {
 *               ( [0] <= iv <= [5]) {
 *                 x, file = foo( iv, file);
 *               } : ( x, file);
 *             } ( genarray( shp), propagate( file));
 *
 *   Splitting-up this With-Loop in exactly the same manor as indicated
 *   before, we would obtain:
 *
 *   tmp_a = with {
 *             ( [0] <= iv <= [5]) {
 *               x, file = foo( iv, file);
 *             } : x;
 *           }  genarray( shp);
 *   tmp_file = with {
 *                ( [0] <= iv <= [5]) {
 *                  x, file = foo( iv, file);
 *                } : file;
 *              } propagate( file);
 *   a = tmp_a;
 *   file = tmp_file;
 *
 *   Unfortunately, this leads to a uniqueness error, as the initial
 *   file is being used, i.e., conceptually consumed, within the body
 *   of the first With-Loop but not propagated. Hence, the transformation
 *   is illegal! Detecting these cases statically may be doable (?) but
 *   seems to be rather difficult to detect.
 *
 * APPROACH 3:
 *
 * we copy all propagates into all With-Loops that we split-off.
 * For our example above that would mean that it stays unmodifed.
 * Looking at a slightly more complex example that contains more than one
 * standard operator, we would split
 *
 * a, b, file = with {
 *                ( [0] <= iv <= [5]) {
 *                  x, file = foo( iv, file);
 *                  y, file = foo( iv, file);
 *                } : ( x, y, file);
 *              } (genarray([20]), genarray([30]), propagate( file));
 * into
 * tmp_a, file = with {
 *                 ( [0] <= iv <= [5]) {
 *                   x, file = foo( iv, file);
 *                   y, file = foo( iv, file);
 *                 } : ( x, file);
 *               } (genarray([20]), propagate( file));
 * tmp_b, file = with {
 *                  ( [0] <= iv <= [5]) {
 *                    x, file = foo( iv, file);
 *                    y, file = foo( iv, file);
 *                  } : ( y, file);
 *               } ( genarray([30]), propagate( file));
 * a = tmp_a;
 * b = tmp_b;
 *
 * The drawback of this approach is:
 * - we would need to define the semantics of Multi-Operator With-Loops
 *   slightly unintuitively: all side-effects contained in any of the
 *   parts are executed as often as we have standard operators in the
 *   operator part.
 *
 * As can be seen from these deliberations, none of the three approaches
 * is really satisfying. Approach 1 is the only one really sound but
 * leaves the language incomplete; approach 2 may turn legal into illegal
 * code; and approach 3 requires the semantics to be defined in a rather
 * non-intuitive way.
 * However, since the ability to insert print statements ANYWHERE is
 * really helpfull for debugging, AND, since approach 3 comes almost for
 * free once we have implemented the splitting of standard operators anyways,
 * this module follows that approach.
 *
 * IMPORTANT:
 * In the long run, though, I think we will have to extnd the backend to be
 * able to cope with Multi-Operator With-Loops that may have different
 * overall ranges. That would make this entire traversal redundant and still
 * keep the intuitive semantics intact!
 *
 *
 *
 * TO BE DONE:
 * the renaming as mentioned above is not yet implemented!
 */

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
    bool legal;
    node *range;
    nodetype opkind;
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
#define INFO_HWLO_LEGAL_MOWL(n) (n->legal)
#define INFO_HWLO_RANGE(n) (n->range)
#define INFO_HWLO_OPKIND(n) (n->opkind)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_HWLO_LASTASSIGN (result) = NULL;
    INFO_HWLO_NUM_STD_OPS (result) = 0;
    INFO_HWLO_CEXPRS (result) = NULL;
    INFO_HWLO_NEW_CEXPRS (result) = NULL;
    INFO_HWLO_LHS (result) = NULL;
    INFO_HWLO_NEW_LHS (result) = NULL;
    INFO_HWLO_NEW_WITHOPS (result) = NULL;

    INFO_HWLO_LEGAL_MOWL (result) = TRUE;
    INFO_HWLO_RANGE (result) = NULL;
    INFO_HWLO_OPKIND (result) = N_with;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

static node *
ATravILMOWLgenarray (node *genarray, info *arg_info)
{
    DBUG_ENTER ();

    // We know that (INFO_HWLO_LEGAL_MOWL (arg_info) == TRUE)!
    if (INFO_HWLO_OPKIND (arg_info) == N_with) { // first genarray/modarray/fold!
        INFO_HWLO_OPKIND (arg_info) = N_genarray;
        INFO_HWLO_RANGE (arg_info) = GENARRAY_SHAPE (genarray);
        GENARRAY_NEXT (genarray) = TRAVopt (GENARRAY_NEXT (genarray), arg_info);
    } else if (INFO_HWLO_OPKIND (arg_info) == N_genarray) {
        if ( CMPTdoCompareTree (INFO_HWLO_RANGE (arg_info),
                                GENARRAY_SHAPE (genarray)) == CMPT_EQ) {
            GENARRAY_NEXT (genarray) = TRAVopt (GENARRAY_NEXT (genarray), arg_info);
        } else {
            INFO_HWLO_LEGAL_MOWL (arg_info) = FALSE;
        }
    } else {
        INFO_HWLO_LEGAL_MOWL (arg_info) = FALSE;
    }

    DBUG_RETURN (genarray);
}

static node *
ATravILMOWLmodarray (node *modarray, info *arg_info)
{
    DBUG_ENTER ();
    
    // We know that (INFO_HWLO_LEGAL_MOWL (arg_info) == TRUE)!
    if (INFO_HWLO_OPKIND (arg_info) == N_with) { // first genarray/modarray/fold!
        INFO_HWLO_OPKIND (arg_info) = N_modarray;
        INFO_HWLO_RANGE (arg_info) = MODARRAY_ARRAY (modarray);
        MODARRAY_NEXT (modarray) = TRAVopt (MODARRAY_NEXT (modarray), arg_info);
    } else if (INFO_HWLO_OPKIND (arg_info) == N_modarray) {
        if ( CMPTdoCompareTree (INFO_HWLO_RANGE (arg_info),
                                MODARRAY_ARRAY (modarray)) == CMPT_EQ) {
            MODARRAY_NEXT (modarray) = TRAVopt (MODARRAY_NEXT (modarray), arg_info);
        } else {
            INFO_HWLO_LEGAL_MOWL (arg_info) = FALSE;
        }
    } else {
        INFO_HWLO_LEGAL_MOWL (arg_info) = FALSE;
    }

    DBUG_RETURN (modarray);
}

static node *
ATravILMOWLspfold (node *fold, info *arg_info)
{
    DBUG_ENTER ();

    // We know that (INFO_HWLO_LEGAL_MOWL (arg_info) == TRUE)!
    if (((INFO_HWLO_OPKIND (arg_info) == N_with) || (INFO_HWLO_OPKIND (arg_info) == N_fold))
         && (SPFOLD_GUARD (fold) == NULL)) {
        INFO_HWLO_OPKIND (arg_info) = N_fold;
        SPFOLD_NEXT (fold) = TRAVopt (SPFOLD_NEXT (fold), arg_info);
    } else {
        INFO_HWLO_LEGAL_MOWL (arg_info) = FALSE;
    }

    DBUG_RETURN (fold);
}

static node *
ATravILMOWLpropagate (node *propagate, info *arg_info)
{
    DBUG_ENTER ();
    PROPAGATE_NEXT (propagate) = TRAVopt (PROPAGATE_NEXT (propagate), arg_info);
    DBUG_RETURN (propagate);
}

static bool
IsLegitimateMoWl (node *withop, info *arg_info)
{
    anontrav_t ilmowl_trav[5]
      = {{N_genarray, &ATravILMOWLgenarray},
         {N_modarray, &ATravILMOWLmodarray},
         {N_spfold, &ATravILMOWLspfold},
         {N_propagate, &ATravILMOWLpropagate},
         {(nodetype)0, NULL}};

    DBUG_ENTER ();

    DBUG_PRINT ("checking multi-operator WL for splitting...\n");

    TRAVpushAnonymous (ilmowl_trav, &TRAVsons);

    INFO_HWLO_LEGAL_MOWL (arg_info) = TRUE;
    INFO_HWLO_RANGE (arg_info) = NULL;
    INFO_HWLO_OPKIND (arg_info) = N_with;

    withop = TRAVopt (withop, arg_info);

    TRAVpop ();

    DBUG_PRINT ("... splitting is %s required\n",
                (INFO_HWLO_LEGAL_MOWL (arg_info) ?  "not" : "")); 
    DBUG_RETURN (INFO_HWLO_LEGAL_MOWL (arg_info));
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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

    mem_last_assign = INFO_HWLO_LASTASSIGN (arg_info);
    INFO_HWLO_LASTASSIGN (arg_info) = arg_node;
    DBUG_PRINT ("LASTASSIGN set to %p!", (void *)arg_node);

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    /*
     * newly inserted abstractions are prepanded in front of
     * INFO_HWLO_LASTASSIGN(arg_info). To properly insert these nodes,
     * that pointer has to be returned:
     */
    return_node = INFO_HWLO_LASTASSIGN (arg_info);

    if (return_node != arg_node) {
        DBUG_PRINT ("node %p will be inserted instead of %p",
                    (void *)return_node,
                    (void *)arg_node);
    }
    INFO_HWLO_LASTASSIGN (arg_info) = mem_last_assign;
    DBUG_PRINT ("LASTASSIGN (re)set to %p!", (void *)mem_last_assign);

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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

    DBUG_ASSERT ((WITH_CODE (arg_node) == NULL)
                   || (CODE_NEXT (WITH_CODE (arg_node)) == NULL),
                 "HWLO requires all WLs to be single-generator!");

    INFO_HWLO_CEXPRS (arg_info)
      = (WITH_CODE (arg_node) == NULL) ? NULL : CODE_CEXPRS (WITH_CODE (arg_node));

    if ( (TCcountExprs (INFO_HWLO_CEXPRS (arg_info)) > 1)
         && !IsLegitimateMoWl (WITH_WITHOP (arg_node), arg_info)) {
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
        if (WITH_CODE (arg_node) != NULL) {
            WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

static node *
StdWithOp (node *arg_node, info *arg_info)
{
    node *my_lhs, *my_cexprs, *return_node;
    int my_pos;

    DBUG_ENTER ();

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
    DBUG_ENTER ();

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
    DBUG_ENTER ();

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
    DBUG_ENTER ();

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

    DBUG_ENTER ();

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
        tmp = STRcpy (SPID_NAME (PROPAGATE_DEFAULT (arg_node)));

        new_withop = TBmakePropagate (TBmakeSpid (NULL, tmp));
        PROPAGATE_NEXT (new_withop) = INFO_HWLO_NEW_WITHOPS (arg_info);

        INFO_HWLO_NEW_WITHOPS (arg_info) = new_withop;

        INFO_HWLO_NEW_LHS (arg_info)
          = TBmakeSpids (STRcpy (tmp), INFO_HWLO_NEW_LHS (arg_info));

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

#undef DBUG_PREFIX
