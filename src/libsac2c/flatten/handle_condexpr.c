#include "handle_condexpr.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "DupTree.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "free.h"

/*
 * Overall picture:
 * ----------------
 *
 * This traversal elides funcond expressions and produces proper cond-assignments
 * instead. The reason this is done is to standardise the way conditionals can
 * appear throughout.
 *
 * An example:
 *   a = ( p ? t : e);
 *
 * is replaced by:
 *
 *   if (p) {
 *     _hce_0 = t;
 *   } else {
 *     _hce_0 = e;
 *   }
 *   a = _hce_0;
 *
 * So, in general, one replaces the funcond by a fresh variable, creates an assignment
 * chain containing a cond and places this before the current assignment.
 *
 * Implementation:
 * ---------------
 *
 * In principle, the implementation is straight forward. Most of the music happens in
 * HCEfuncond, where a replacement variable is generated, the assignment chain is
 * constructed and put into the info-node (INFO_HCE_PREASSIGN) and the funcond is
 * replaced by the new variable.
 * The only tricky part then is to inject the assignemnt chain in the right place.
 * Non-surprisingly, HCEassign does so during a bottom up traversal.
 * However, we do have a few language constructs that may contain funcond
 * expressions and assignment chaines by themselves such as loops, conditionals
 * and with-loops. In these cases we need to make sure that any INFO_HCE_PREASSIGN
 * expressions are inserted in the right places!
 *
 * For conditionals (HCEcond), it suffices to make sure we traverse the branches
 * BEFORE we traverse the predicate. If it so happens that we lift a funcond from
 * the predicate expression, we are sure this is inserted in the immediately
 * surrounding assignment chain as the cond itself lives within an N_assign
 *
 * The only loops left here are DO-loops. Here we first traverse the condition
 * expression. If (INFO_HCE_PREASSIGN) contains assignments, we append them to
 * the loop body BEFORE traversing the body itself thus making sure that the
 * lifted expression ends up in the irght place.
 *
 * With-Loops are more challenging. Indeed issue 2274 is a bug in the initial
 * implementation of this traversal.
 * A WL has three sons: PARTs, CODE, and WITHOPs.
 *  - CODE can be easily handled: the CEXPRS are done first, if something needs
 *    lifting HCEcode appends it to the CBLOCK before traversing that block.
 *    Problem solved.
 *  - PARTs and WITHOPs require lifting OUTSIDE of the WL. This is difficult
 *    to achieve as a WL is an expression and thus can be deeply nested.
 *    The idea is that we keep the assignment chain long enough so that it can
 *    be inserted when we reach the surrounding assignmemnt.
 *    Unfortunately, PART and WITHOP both have more than one subexpression
 *    all of which can (a) add further assignment chaines and (b)
 *    can contain WLs themselves which contain assignment chains that can
 *    trigger un-intended offloading of chains lifted from unrelated expressions.
 *    issue 2274 is exactly an instance of such a situation:
 *
 *         a  = with {
 *          ([0] <= _hse_0 < [7]) : 0;
 *       } : genarray( _shape_A_( with {
 *                                  ([0] <= _hzgwl_3 < [ 7 ]) : 0;
 *                                } : genarray( [ 7 ], ( _eq_SxS_( 0, 0) ? 1 : 2 ))),
 *
 *                       zero( with {
 *                               ([ 0 ] <= _hzgwl_4 < [ 7 ])  : ( _eq_SxS_( 0, 0) ? 0 : 42 ) ;
 *                             } : genarray( [ 7 ], 0))       );
 *
 *    Here, the SHAPE lifts a funcond which is parasitically injected into the WL
 *    within the DEFAULT element.
 *
 *    To avoid such parasitic offloadings, we stack INFO_HCE_PREASSIGN whenever
 *    entering HCEwith and we prepand them thereafter. This way, any possible
 *    INFO_HCE_PREASSIGN chains will survive until the next surounding N_assign.
 */


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
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_HCE_PREASSIGN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

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

    DBUG_ENTER ();

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
    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt(ASSIGN_NEXT (arg_node), arg_info);

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

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
    DBUG_ENTER ();

    CODE_NEXT (arg_node) = TRAVopt(CODE_NEXT (arg_node), arg_info);

    CODE_CEXPRS (arg_node) = TRAVopt(CODE_CEXPRS (arg_node), arg_info);

    if (INFO_HCE_PREASSIGN (arg_info) != NULL) {
        CODE_CBLOCK_ASSIGNS (arg_node) = TCappendAssign (CODE_CBLOCK_ASSIGNS (arg_node),
                                                         INFO_HCE_PREASSIGN (arg_info));

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
    DBUG_ENTER ();

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
    DBUG_ENTER ();

    DO_COND (arg_node) = TRAVdo (DO_COND (arg_node), arg_info);

    if (INFO_HCE_PREASSIGN (arg_info) != NULL) {
        DO_ASSIGNS (arg_node)
          = TCappendAssign (DO_ASSIGNS (arg_node), INFO_HCE_PREASSIGN (arg_info));

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

    DBUG_ENTER ();

    n = TRAVtmpVar ();

    p = FUNCOND_IF (arg_node);
    FUNCOND_IF (arg_node) = NULL;

    t = TBmakeBlock (TBmakeAssign (TBmakeLet (TBmakeSpids (STRcpy (n), NULL),
                                              FUNCOND_THEN (arg_node)),
                                   NULL),
                     NULL);
    FUNCOND_THEN (arg_node) = NULL;

    e = TBmakeBlock (TBmakeAssign (TBmakeLet (TBmakeSpids (STRcpy (n), NULL),
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
    node *old_pre_assigns;
    DBUG_ENTER ();

    /* hide any potential PREASSIGNS that need to get to the outer context! */
    old_pre_assigns = INFO_HCE_PREASSIGN (arg_info);
    INFO_HCE_PREASSIGN (arg_info) = NULL;

    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    WITH_WITHOP (arg_node) = TRAVopt(WITH_WITHOP (arg_node), arg_info);
    /* Now we bring old preassigns back! */
    if (old_pre_assigns != NULL) {
        INFO_HCE_PREASSIGN (arg_info) = TCappendAssign (old_pre_assigns,
                                                        INFO_HCE_PREASSIGN (arg_info));
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
