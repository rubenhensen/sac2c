/** <!--******************************************************************-->
 *
 * @file lacfun_utilities.c
 *
 * @brief: This file contains utilities that are common to
 *         traversals operating on LOOPFUNs and CONDFUNs.
 *
 **************************************************************************/

#include "tree_basic.h"
#include "traverse.h"
#include "node_basic.h"
#include "new_types.h"

#define DBUG_PREFIX "LFU"
#include "debug.h"

#include "memory.h"
#include "free.h"
#include "globals.h"
#include "constants.h"
#include "shape.h"
#include "tree_compound.h"
#include "lacfun_utilities.h"
#include "indexvectorutils.h"
#include "pattern_match.h"
#include "DupTree.h"
#include "LookUpTable.h"
#include "polyhedral_utilities.h"
#include "polyhedral_defs.h"
#include "symbolic_constant_simplification.h"

/** <!--********************************************************************-->
 *
 * @name Utility functions
 * @{
 *
 *****************************************************************************/

struct ca_info {
    node *exprs;
    node *avis;
    node *vardecs;
};

/** <!--***********************************************************************-->
 *
 * @fn node *LFUprefixFunctionArgument( node *arg_node, node *calleravis,
 *                                       node **callerapargs)
 *
 * @brief: Prefix calleravis to the parameter list of
 *         called function's N_fundef arg_node,
 *         and to called function's recursive call (if callee is a LOOPFUN).
 *         Also, prefix an N_ap element to the caller's N_ap node..
 *
 *         We assume that the traversal is at the N_fundef level
 *         within callee, so that it can be modified safely.
 *
 * @param: arg_node: The N_fundef of the callee function.
 *         calleravis: an N_avis in the calling function.
 *         callerapargs: The AP_ARGS chain of the external call.
 *
 * @result: N_avis for the new N_id in the callee's environment.
 *
 *         SIDE EFFECT: *callerapargs has the N_ap arguments
 *                      prefixed to it for the external call.
 *
 * Comment: For some reason(s), some of the optimizations, such
 *          as LURSSA, assume/require that AVIS son nodes referenced
 *          in the FUNDEF_ARGS nodes appear in the AP_ARGs list
 *          previous to (to the left of) their reference(s)
 *          in the AP_ARGS son nodes. For this reason, we use
 *          prefix instead of append here.
 *
 *****************************************************************************/
node *
LFUprefixFunctionArgument (node *arg_node, node *calleravis, node **callerapargs)
{
    node *newavis;
    node *outercall;
    node *reccall;

    DBUG_ENTER ();

    newavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (calleravis)),
                          TYcopyType (AVIS_TYPE (calleravis)));
    FUNDEF_ARGS (arg_node)
      = TCappendArgs (TBmakeArg (newavis, NULL), FUNDEF_ARGS (arg_node));
    outercall = TBmakeExprs (TBmakeId (calleravis), NULL);
    *callerapargs = TCappendExprs (outercall, *callerapargs);

    if (FUNDEF_ISLOOPFUN (arg_node)) {
        reccall = FUNDEF_LOOPRECURSIVEAP (arg_node);
        AP_ARGS (reccall)
          = TCappendExprs (TBmakeExprs (TBmakeId (newavis), NULL), AP_ARGS (reccall));
    }

    DBUG_RETURN (newavis);
}

/** <!--***********************************************************************-->
 *
 * @fn bool LFUisLoopFunInvariant( node *fundef, node *arg,
 *                                 node *rca)
 *
 * @brief true if fundef is not a LOOPFUN.
 *        true if fundef IS a LOOPFUN, and arg (the current
 *        LOOPFUN N_arg element) is the same as rca (recursivecallavis),
 *        the current inner N_ap recursive call element.
 *
 * @param fundef:    LACFUN N_fundef in question
 *        arg:       The current N_id or N_avis element of the lacfun's N_arg.
 *        rca:       The current N_id of the recursive call of fundef.
 *                   In the example below, this is iv'.
 *
 * @result: True if the above brief holds.
 *
 * @comment: The match check on the elements is somewhat
 *           subtle, because of the possibility of a selproxy
 *           being in the way. E.g., we may have this:
 *
 *             outer call:   Loop( outeriv...)
 *
 *             int Loop( arg...)
 *                ...
 *               s0 = arg[0];
 *               s1 = arg[1];
 *               iv' = [ s0, s1];
 *               Loop( iv'...);
 *
 *            This code recognizes that iv' and arg are the same.
 *
 ******************************************************************************/
static bool
LFUisLoopFunInvariant (node *fundef, node *arg, node *rca)
{
    bool z = TRUE;
    node *proxy;
    node *avis;

    DBUG_ENTER ();

    avis = (N_avis == NODE_TYPE (arg)) ? arg : ID_AVIS (arg);
    if (FUNDEF_ISLOOPFUN (fundef)) {
        z = avis == ID_AVIS (rca);
        if (!z) {
            proxy = IVUTarrayFromProxySel (rca);
            if (NULL != proxy) {
                z = (avis == ID_AVIS (proxy));
            }
        }

        if (!z) {
            proxy = IVUTarrayFromProxyIdxsel (rca);
            if (NULL != proxy) {
                z = (avis == ID_AVIS (proxy));
            }
        }
    }
    DBUG_PRINT ("inneriv=%s and rca=%s are %s loop-invariant", AVIS_NAME (avis),
                ((NULL != rca) ? AVIS_NAME (ID_AVIS (rca)) : "notrecursive!"),
                ((z ? "" : "not")));

    DBUG_RETURN (z);
}

/** <!--***********************************************************************-->
 *
 * @fn bool LFUisLoopFunDependent( node *fundef, node *var)
 *
 * @brief false if fundef is not a LOOPFUN.
 *        true if fundef IS a LOOPFUN, and arg (some variable in the LOOPFUN)
 *        is loop-dependent
 *
 * @param fundef:    LACFUN N_fundef in question
 *        arg:   An N_id or N_avis, corresponding to a variable in FUNDEF_ARGS
 *
 * @result: True if the above brief holds.
 *
 * @comment: See comment in LFUisLoopFunInvariant re selproxy
 *
 ******************************************************************************/
bool
LFUisLoopFunDependent (node *fundef, node *arg)
{
    node *avis = NULL;
    node *reccallass;
    node *reccallargs;
    node *rcv;
    char *isisnot;
    bool z = FALSE;

    DBUG_ENTER ();

    if (FUNDEF_ISLOOPFUN (fundef)) {
        avis = (N_avis == NODE_TYPE (arg)) ? arg : ID_AVIS (arg);
        reccallass = LFUfindRecursiveCallAssign (fundef);
        reccallargs = AP_ARGS (LET_EXPR (ASSIGN_STMT (reccallass)));
        rcv = LFUgetRecursiveCallVariableFromArgs (avis, fundef, reccallargs);
        z = (NULL != rcv) && (!LFUisLoopFunInvariant (fundef, avis, rcv));
        isisnot = z ? "" : "not ";
        DBUG_PRINT ("arg=%s is %sloop-dependent in %d", AVIS_NAME (avis), isisnot,
                    FUNDEF_NAME (fundef));
    }

    DBUG_RETURN (z);
}

#ifdef DEADCODE
/** <!--********************************************************************-->
 *
 * @fn node *LFUgetCallArg (node *id, node *fundef, node *ext_assign)
 *
 * @params id: An N_id node in fundef, a LACFUN.
 *         fundef: the N_fundef entry for the LACFUN.
 *         ext_assign: The N_assign of the external call to the LACFUN.
 *
 * @brief 1. Ensure that N_id/N_Avis id is an N_arg of fundef.
 *
 *        2. Search the parameter list of the fundef arg chain for
 *           id, and return the ext_assign element that corresponds to
 *           that.
 *
 *           I.e., ext_assign[ FUNDEF_ARGS( fundef) iota id]
 *
 * @result The appropriate N_id from the external call argument list.
 *
 ******************************************************************************/
node *
LFUgetCallArg (node *id, node *fundef, node *ext_assign)
{
    node *arg_chain;
    node *param_chain;
    node *param;
    node *avis;
    int pos;
    int i;

    DBUG_ENTER ();

    avis = (N_avis == NODE_TYPE (id)) ? id : ID_AVIS (id);
    /* Check if id is an arg of this fundef */
    if (NODE_TYPE (AVIS_DECL (avis)) != N_arg) {
        DBUG_PRINT ("identifier %s is not fundef argument", AVIS_NAME (avis));
        DBUG_RETURN (NULL);
    }

    /* Get argument position in fundef arg chain */
    arg_chain = FUNDEF_ARGS (fundef);
    pos = 1;
    while ((arg_chain != NULL) && (arg_chain != AVIS_DECL (avis))) {
        arg_chain = ARG_NEXT (arg_chain);
        pos++;
    }

    DBUG_ASSERT (arg_chain != NULL, "arg not found in fundef arg chain");

    /* Get matching parameter expr-node */
    param_chain = AP_ARGS (ASSIGN_RHS (ext_assign));

    for (i = 1; i < pos; i++) {
        param_chain = EXPRS_NEXT (param_chain);
    }

    DBUG_ASSERT (param_chain != NULL, "missing matching parameter");
    param = EXPRS_EXPR (param_chain);

    DBUG_RETURN (param);
}
#endif // DEADCODE

/** <!--********************************************************************-->
 *
 * @fn node *LFUgetRecursiveCallVariableFromArgs(node *var, node *fundef, node *args)
 *
 * @brief Given an N_id, var, that may appear in FUNDEF_ARGS( fundef),
 *        return the N_id that has the same position in args.
 *        Almost: We chase back across any direct assigns for the args element,
 *        a la VP, so as to reduce the number of variables involved.
 *
 *        I.e.,  args[ FUNDEF_ARGS iota var]
 *
 *        if var does not appear in args, we return NULL.
 *
 *        if var does not appear in params, we return NULL.
 *
 * @param: var:    an N_id node in the LACFUNs N_arg list,
 *                 or its N_avis node.
 * @param: fundef: the LACFUN N_fundef node
 * @param: reccallargs:
 *                 An N_exprs chain:  Either a recursive call AP_ARGS list, or
 *                 an outer call AP_ARGS list.
 *                 If you want to search the FUNDEF_ARGS chain, look elsewhere.
 *
 * @result: The N_id of the calling function's args that corresponds to var.
 *
 *****************************************************************************/
node *
LFUgetRecursiveCallVariableFromArgs (node *var, node *fundef, node *reccallargs)
{
    node *z = NULL;
    node *fargs = NULL;
    node *avis = NULL;

    DBUG_ENTER ();

    fargs = FUNDEF_ARGS (fundef);
    avis = (N_id == NODE_TYPE (var)) ? ID_AVIS (var) : var;
    while (reccallargs && fargs && (avis != ARG_AVIS (fargs))) {
        reccallargs = EXPRS_NEXT (reccallargs);
        fargs = ARG_NEXT (fargs);
    }

    if (reccallargs) {
        z = EXPRS_EXPR (reccallargs);
        DBUG_PRINT ("LACFUN %s arg %s has recursive call value of %s",
                    FUNDEF_NAME (fundef), AVIS_NAME (avis), AVIS_NAME (ID_AVIS (z)));
        z = PHUTskipChainedAssigns (z);
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *LFUgetArgFromRecursiveCallVariable( node *rcv,
              node *fundef);
 *
 * @brief Given, rcv, an N_id or N_avis that is a recursive
 *        call variable in fundef, find the N_id node in
 *        the formal arguments (FUNDEF_ARGS) that corresponds to it,
 *        or NULL if not found.
 *
 * @param: rcv:    an N_id node that appears in the recursive call
 *                 to the LOOPFUN, fundef.
 * @param: fundef: the LACFUN N_fundef node
 *
 *****************************************************************************/
node *
LFUgetArgFromRecursiveCallVariable (node *rcv, node *fundef)
{
    node *z = NULL;
    node *fargs;
    node *reccallass;
    node *reccallargs;
    node *avis;

    DBUG_ENTER ();

    fargs = FUNDEF_ARGS (fundef);
    avis = (N_avis == NODE_TYPE (rcv)) ? rcv : ID_AVIS (rcv);
    reccallass = LFUfindRecursiveCallAssign (fundef);
    reccallargs = AP_ARGS (LET_EXPR (ASSIGN_STMT (reccallass)));
    while (reccallargs && fargs && (avis != ID_AVIS (EXPRS_EXPR (reccallargs)))) {
        reccallargs = EXPRS_NEXT (reccallargs);
        fargs = ARG_NEXT (fargs);
    }

    if (fargs) {
        z = ARG_AVIS (fargs);
        DBUG_PRINT ("LACFUN %s arg %s has recursive call value of %s",
                    FUNDEF_NAME (fundef), AVIS_NAME (avis), AVIS_NAME (z));
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *LFUfindLacfunConditional
 *
 * @brief Find N_prf for conditional governing recursive call in LOOPFUN.
 *
 * @param An N_fundef.
 *
 * @return The N_prf address, or NULL
 *
 ******************************************************************************/
node *
LFUfindLacfunConditional (node *arg_node)
{
    node *z = NULL;

    DBUG_ENTER ();

    if (FUNDEF_ISLOOPFUN (arg_node)) {
        z = COND_COND (ASSIGN_STMT (LFUfindAssignForCond (arg_node)));
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *LFUfindAssignForCond( node *arg_node)
 *
 * @brief Find the N_assign for the N_cond in a LOOPFUN.
 *        We assume that there is only one N_cond in the LOOPFUN.
 *
 * @params arg_node: A LOOPFUN N_fundef node
 *
 * @result: The N_assign of the N_cond in a LOOPFUN
 *          Crash if not found.
 *
 ******************************************************************************/
node *
LFUfindAssignForCond (node *arg_node)
{
    node *assignchain;

    DBUG_ENTER ();
    /* separate loop body assignment chain */
    assignchain = BLOCK_ASSIGNS (FUNDEF_BODY (arg_node));

    while ((assignchain != NULL) && (NODE_TYPE (ASSIGN_STMT (assignchain)) != N_cond)) {
        assignchain = ASSIGN_NEXT (assignchain);
    }

    DBUG_ASSERT (assignchain != NULL, "Missing conditional in loop");

    DBUG_RETURN (assignchain);
}

/** <!--********************************************************************-->
 *
 * @fn node *LFUfindAssignBeforeCond( node *arg_node)
 *
 * @brief Find the N_assign that is just before the N_cond
 *        N_assign in a LOOPFUN.
 *
 *        Blind assumption that first N_assign is not the N_cond.
 *        We assume that there is only one N_cond in the LOOPFUN.
 *
 * @params arg_node: A LOOPFUN N_fundef node
 *
 * @result: The N_assign BEFORE the N_cond in a LOOPFUN
 *          Crash if not found.
 *
 ******************************************************************************/
node *
LFUfindAssignBeforeCond (node *arg_node)
{
    node *assignchain;
    node *z;

    DBUG_ENTER ();
    /* separate loop body assignment chain */
    assignchain = BLOCK_ASSIGNS (FUNDEF_BODY (arg_node));
    z = assignchain;
    assignchain = ASSIGN_NEXT (z);

    while ((assignchain != NULL) && (NODE_TYPE (ASSIGN_STMT (assignchain)) != N_cond)) {
        z = assignchain;
        assignchain = ASSIGN_NEXT (assignchain);
    }

    // z points to the last assignment of the loop body
    // and assignchain to the conditional of the loop
    DBUG_ASSERT (z != NULL, "Loop body missing");
    DBUG_ASSERT (assignchain != NULL, "Missing conditional in loop");

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 *  Helper function for Anonymous traversal searching for the
 *   only occurrence of local_info->nt type in the chain
 *  of assignments.
 *
 ******************************************************************************/
// Structure for the Anonymous traversal.
struct local_info {
    node *res;
    nodetype nt;
};

static node *
ATravFilter (node *arg_node, info *arg_info)
{
    struct local_info *linfo = (struct local_info *)arg_info;
    DBUG_ENTER ();

    if (NODE_TYPE (ASSIGN_STMT (arg_node)) == linfo->nt) {
        if (linfo->res == NULL) {
            ((struct local_info *)arg_info)->res = arg_node;
            arg_node = TRAVcont (arg_node, arg_info);

        } else {
            ((struct local_info *)arg_info)->res = NULL;
        }
    } else if (arg_node != NULL) {
        arg_node = TRAVcont (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * Search for the node of type n in the sequence of assignments
 * ASSIGNS and return the assignment. Return NULL when either
 * the node of type n was not found, or was found more than once.
 *
 ******************************************************************************/
node *
LFUfindAssignOfType (node *assigns, nodetype n)
{
    struct local_info linfo;

    DBUG_ENTER ();

    linfo.res = NULL, linfo.nt = n;

    TRAVpushAnonymous ((anontrav_t[]){{N_assign, &ATravFilter}, {(nodetype)0, NULL}},
                       &TRAVsons);
    assigns = TRAVopt (assigns, (info *)(void *)&linfo);
    TRAVpop ();

    DBUG_RETURN (linfo.res);
}

/** <!--********************************************************************-->
 *
 * @fn bool LFUisLURPredicate( node *arg_node)
 *
 * @brief checks the given expression to be of the form:
 *        expr    = id    {<=, >=, <, >, != } const
 *        or expr = const {<=, >=, <, >, != } id
 * @param arg_node: an N_prf
 *
 ******************************************************************************/
bool
LFUisLURPredicate (node *arg_node)
{
    node *arg1;
    node *arg2;
    prf comp_prf;
    pattern *pat;
    bool z = TRUE;

    DBUG_ENTER ();

    /* expression must be a primitive function */
    if (NODE_TYPE (arg_node) != N_prf) {
        DBUG_PRINT ("predicate expression without prf");
        z = FALSE;
    } else {
        /* prf must be a relational */
        comp_prf = PRF_PRF (arg_node);
        if ((comp_prf != F_le_SxS) && (comp_prf != F_lt_SxS) && (comp_prf != F_ge_SxS)
            && (comp_prf != F_gt_SxS) && (comp_prf != F_neq_SxS)) {
            z = FALSE;
            DBUG_PRINT ("predicate with non-relational prf");
        }
    }

    /* args must be one constant (N_num)
     * and one identifier (N_id) node  */
    if (z) {
        DBUG_ASSERT (PRF_ARGS (arg_node), "missing arguments to primitive function");
        DBUG_ASSERT (EXPRS_NEXT (PRF_ARGS (arg_node)),
                     "missing second arg of primitive function");

        arg1 = PRF_ARG1 (arg_node);
        arg2 = PRF_ARG2 (arg_node);

        pat = PMint (0, 0);

        if ((PMmatchFlat (pat, arg1) && (N_id == NODE_TYPE (arg2)))
            || (PMmatchFlat (pat, arg2) && (N_id == NODE_TYPE (arg1)))) {
            DBUG_PRINT ("loop predicate has correct form");
        } else {
            DBUG_PRINT ("loop predicate without id and constant args");
            z = FALSE;
        }
        pat = PMfree (pat);
    }

    DBUG_RETURN (z);
}

/** <!--***********************************************************************-->
 *
 * @fn node *LFUfindLoopInductionVariable( node *arg_node)
 *
 * @brief: Find the Loop Variant variable that controls the number
 *         of iterations of this loop function.
 *
 * We look at the N_cond in the LOOPFUN, then trace its argument back to
 * the relational, and look for a recognizable pattern of code that
 * could let us deduce the loop count. We look for one of these
 * patterns:
 *
 *   P = _lt_SxS_( I, const);   NB.  Any relational will do
 *                              NB.  const may be an N_num, or
 *                                   a loop-invariant scalar.
 *   P = _lt_SxS_( const, I);   NB.  Any relational will do
 *                              NB.  const may be an N_num, or
 *                                   a loop-invariant scalar.
 *
 * If CTZ has been tinkering with the relational, we may have:
 *
 *   V = I - const;
 *   P = _lt_SxS_( V, 0);
 *
 * @param: arg_node: The N_fundef of the LACFUN function.
 *
 * @result: N_avis for the variable, or NULL, if we are
 *          unable to figure it out.
 *
 *****************************************************************************/
node *
LFUfindLoopInductionVariable (node *arg_node)
{
    node *zavis = NULL;
    node *cond;
    node *condprf;
    pattern *pat;
    prf relop;
    node *arg1 = NULL;
    node *arg2 = NULL;

    DBUG_ENTER ();

    cond = LFUfindAssignForCond (arg_node);
    cond = COND_COND (ASSIGN_STMT (cond));
    condprf = LET_EXPR (ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (cond))));
    DBUG_PRINT ("Function %s induction variable predicate is %s", FUNDEF_NAME (arg_node),
                AVIS_NAME (ID_AVIS (cond)));

    pat = PMprf (1, PMAgetPrf (&relop), 2, PMvar (1, PMAgetNode (&arg1), 0),
                 PMvar (1, PMAgetNode (&arg2), 0));
    if ((PMmatchFlat (pat, cond)) && LFUisLURPredicate (condprf)) {
        if (COisConstant (arg1)) {
            zavis = ID_AVIS (arg2);
        } else if (COisConstant (arg2)) {
            zavis = ID_AVIS (arg1);
        } else {
            DBUG_UNREACHABLE ("Could not find constant argument to condprf");
        }
        DBUG_PRINT ("predicate relational args are ( %s, %s)", AVIS_NAME (ID_AVIS (arg1)),
                    AVIS_NAME (ID_AVIS (arg2)));
    } else {
        DBUG_UNREACHABLE ("Could not find relational for predicate");
    }

    pat = PMfree (pat);

    DBUG_RETURN (zavis);
}

/** <!--********************************************************************-->
 *
 * @fn bool LFUisAvisMemberArg( node *arg_node, node *args)
 *
 * @brief: Predicate for checking that an N_avis node is
 *         a member of an N_arg chain.
 *
 * @param: arg_node - an N_avis node.
 *         args     - an N_arg chain.
 *
 * @result: TRUE if arg_node is a member of the N_arg chain.
 *
 *****************************************************************************/
bool
LFUisAvisMemberArg (node *arg_node, node *args)
{
    bool z = FALSE;

    DBUG_ENTER ();

    DBUG_ASSERT (N_arg == NODE_TYPE (args), "Expected N_arg chain");
    while ((NULL != args) && (!z)) {
        z = (arg_node == ARG_AVIS (args));
        args = ARG_NEXT (args);
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool LFUisAvisMemberExprs( node *arg_node, node *exprs)
 *
 * @brief: Predicate for checking that an N_avis node is
 *         a member of an N_exprs chain.
 *
 * @param: arg_node - an N_avis node.
 *         exprs    - an N_exprs chain.
 *
 * @result: TRUE if arg_node is a member of the N_exprs chain.
 *
 *****************************************************************************/
bool
LFUisAvisMemberExprs (node *arg_node, node *exprs)
{
    bool z = FALSE;

    DBUG_ENTER ();

    DBUG_ASSERT (N_avis == NODE_TYPE (arg_node), "Expected N_avis node");
    while ((NULL != exprs) && (!z)) {
        z = (arg_node == ID_AVIS (EXPRS_EXPR (exprs)));
        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool LFUisAvisMemberIds( node *arg_node, node *ids)
 *
 * @brief: Predicate for checking that an N_avis node is
 *         a member of an N_ids chain.
 *
 * @param: arg_node - an N_avis node.
 *         ids      - an N_ids chain.
 *
 * @result: TRUE if arg_node is a member of the ids chain.
 *
 * FIXME: This function will be dead code later today. I'm leaving
 *        it here until LFUindexOfMemberIds is in place and
 *        has replaced all refernces to LFUisAvisMemberIds.
 *
 *****************************************************************************/
bool
LFUisAvisMemberIds (node *arg_node, node *ids)
{
    bool z = FALSE;

    DBUG_ENTER ();

    DBUG_ASSERT (N_avis == NODE_TYPE (arg_node), "Expected N_avis node");
    while ((NULL != ids) && (!z)) {
        z = (arg_node == IDS_AVIS (ids));
        ids = IDS_NEXT (ids);
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn int LFUindexOfMemberIds( node *arg_node, node *ids)
 *
 * @brief: Function for finding the index of an N_avis node
 *         in an N_ids chain.
 *
 * @param: arg_node - an N_avis node.
 *         ids      - an N_ids chain.
 *
 * @result: 0...(length of idschain -1) if arg_node is a member of the ids chain.
 *          -1 if arg_node is not a member of the ids chain.
 *
 *****************************************************************************/
int
LFUindexOfMemberIds (node *arg_node, node *ids)
{
    int z = -1;
    int j = 0;

    DBUG_ENTER ();

    DBUG_ASSERT (N_avis == NODE_TYPE (arg_node), "Expected N_avis node");
    while ((NULL != ids) && (-1 == z)) {
        if (arg_node == IDS_AVIS (ids)) {
            z = j;
        }
        ids = IDS_NEXT (ids);
        j++;
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *LFUfindRecursiveCallAssign( node *arg_node)
 *
 * @brief:  Find the recursive call to self in a LOOPFUN, if any.
 *          This code relies on the fact that a LOOPFUN contains only
 *          one condfun.
 *
 * @param: arg_node - an N_fundef node for a LOOPFUN.
 *
 * @result: Address of the N_assign node for the recursive LOOPFUN call, or NULL.
 *
 *****************************************************************************/
node *
LFUfindRecursiveCallAssign (node *arg_node)
{
    node *ass = NULL;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef, "Illegal argument!");
    if (FUNDEF_ISLOOPFUN (arg_node)) {
        DBUG_ASSERT (FUNDEF_BODY (arg_node) != NULL, "Loop function %s has no body",
                     FUNDEF_NAME (arg_node));

        ass = FUNDEF_ASSIGNS (arg_node);
        while ((ass != NULL) && (NODE_TYPE (ASSIGN_STMT (ass)) != N_cond)) {
            ass = ASSIGN_NEXT (ass);
        }

        DBUG_ASSERT (ass != NULL, "Loop function %s without conditional",
                     FUNDEF_NAME (arg_node));

        ass = COND_THENASSIGNS (ASSIGN_STMT (ass));

        while ((ass != NULL) && (NODE_TYPE (ASSIGN_STMT (ass)) == N_annotate)) {
            ass = ASSIGN_NEXT (ass);
        }

        DBUG_ASSERT ((ass != NULL) && (NODE_TYPE (ass) == N_assign)
                       && (NODE_TYPE (ASSIGN_STMT (ass)) == N_let)
                       && (NODE_TYPE (ASSIGN_RHS (ass)) == N_ap)
                       && (AP_FUNDEF (ASSIGN_RHS (ass)) == arg_node),
                     "No recursive call found in expected position in function %s",
                     FUNDEF_NAME (arg_node));
    }

    DBUG_RETURN (ass);
}

/** <!--********************************************************************-->
 *
 * @fn node *LFUinsertAssignIntoLacfun( arg_node, assign, oldavis);
 *
 * @brief: Insert assign into a LACFUN. If the LACFUNS is a LOOPFUN, this is
 *         trivial. If it's a CONDFUN, we need two copies, and
 *         they have to go into the THEN and ELSE legs of the COND.
 *
 * @param: arg_node - an N_fundef node.
 *         assign   - an N_assign node to be inserted into the blocks.
 *         oldavis  - an N_avis node. All references to oldavis are
 *                    changed to the assign LHS.
 *
 * @result: updated N_fundef node.
 *          WLPROP, at least, assumes that we do NOT allocate a
 *          new N_fundef node. This assumption could be
 *          eliminated, if need be.
 *
 *          We refresh the recursive AP call address in a LOOPFUN.
 *
 *****************************************************************************/
node *
LFUinsertAssignIntoLacfun (node *arg_node, node *assign, node *oldavis)
{
    node *block;
    node *thenelse;
    node *assignelse;
    lut_t *lut;

    DBUG_ENTER ();

    lut = LUTgenerateLut ();
    LUTinsertIntoLutP (lut, oldavis, IDS_AVIS (LET_IDS (ASSIGN_STMT (assign))));
    block = FUNDEF_BODY (arg_node);

    if (FUNDEF_ISLOOPFUN (arg_node)) {
        /* LOOPFUN */
        BLOCK_ASSIGNS (block) = DUPdoDupTreeLut (BLOCK_ASSIGNS (block), lut);
        BLOCK_ASSIGNS (block) = TCappendAssign (assign, BLOCK_ASSIGNS (block));
        FUNDEF_LOOPRECURSIVEAP (arg_node)
          = LET_EXPR (ASSIGN_STMT (LFUfindRecursiveCallAssign (arg_node)));
    } else {

        /* CONDFUN is harder */
        /* DUP gives us a shiny new vardec and avis! */
        assignelse = DUPdoDupNodeSsa (assign, arg_node);

        DBUG_ASSERT (FUNDEF_ISCONDFUN (arg_node), "Expected CONDFUN");
        thenelse = BLOCK_ASSIGNS (COND_THEN (ASSIGN_STMT (BLOCK_ASSIGNS (block))));
        thenelse = DUPdoDupTreeLut (thenelse, lut);
        thenelse = TCappendAssign (assign, thenelse);
        BLOCK_ASSIGNS (COND_THEN (ASSIGN_STMT (BLOCK_ASSIGNS (block)))) = thenelse;

        /* We have to rename uses of assignelse in the COND_ELSE block */
        lut = LUTremoveLut (lut);
        lut = LUTgenerateLut ();
        LUTinsertIntoLutP (lut, oldavis, IDS_AVIS (LET_IDS (ASSIGN_STMT (assignelse))));

        thenelse = BLOCK_ASSIGNS (COND_ELSE (ASSIGN_STMT (BLOCK_ASSIGNS (block))));
        BLOCK_ASSIGNS (COND_ELSE (ASSIGN_STMT (BLOCK_ASSIGNS (block))))
          = TCappendAssign (assignelse, thenelse);

        BLOCK_ASSIGNS (COND_ELSE (ASSIGN_STMT (BLOCK_ASSIGNS (block))))
          = DUPdoDupTreeLut (BLOCK_ASSIGNS (
                               COND_ELSE (ASSIGN_STMT (BLOCK_ASSIGNS (block)))),
                             lut);
    }
    lut = LUTremoveLut (lut);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *LFUfindFundefReturn( node *arg_node)
 *
 * @brief: Find the N_return for an N_fundef, if it exists.
 *         Wrappers do not have an N_return.
 *
 * @param: arg_node - an N_fundef node.
 *
 * @result: Address of the N_return, if it exists, or NULL.
 *
 *****************************************************************************/
node *
LFUfindFundefReturn (node *arg_node)
{
    node *z = NULL;
    node *assgn;

    DBUG_ENTER ();

    if ((!FUNDEF_ISWRAPPERFUN (arg_node)) && (global.compiler_anyphase >= PH_ptc_l2f)
        && (global.compiler_anyphase < PH_cg_ctr)) {
        assgn = FUNDEF_BODY (arg_node);
        if (NULL != assgn) { /* Some fns do not have a body. Weird... */
            assgn = BLOCK_ASSIGNS (assgn);
            while (NULL == z) {
                if (N_return == NODE_TYPE (ASSIGN_STMT (assgn))) {
                    z = ASSIGN_STMT (assgn);
                }
                assgn = ASSIGN_NEXT (assgn);
            }
        }
    }
    DBUG_RETURN (z);
}

/** <!--***********************************************************************-->
 *
 * @fn node *LFUarg2Vardec( node *arg_node, node *fundef)
 *
 * @brief Replace an N_arg by an N_vardec.
 *        This function is used by optimizations such as EDFA,
 *        when it wants to remove an N_arg from a FUNDEF_ARGS
 *        chain. We do not want to FREE the node, because
 *        that will take the ARG_AVIS and its sons with it,
 *        and they may still be referenced.
 *
 *        So, we create an N_vardec for the AVIS node and
 *        attach that to the FUNDEF. Eventually, DCR will
 *        come along and deallocate it properly.
 *
 * @param arg_node: N_args
 *        fundef:   N_fundef
 *
 * @result: ARG_NEXT node.
 *
 ******************************************************************************/
node *
LFUarg2Vardec (node *arg_node, node *fundef)
{
    node *z;

    DBUG_ENTER ();

    DBUG_PRINT ("Replacing N_arg %s by N_vardec in %s", AVIS_NAME (ARG_AVIS (arg_node)),
                FUNDEF_NAME (fundef));
    z = ARG_NEXT (arg_node);
    ARG_NEXT (arg_node) = NULL;
    FUNDEF_VARDECS (fundef) = TBmakeVardec (ARG_AVIS (arg_node), FUNDEF_VARDECS (fundef));
    ARG_AVIS (arg_node) = NULL;
    arg_node = FREEdoFreeNode (arg_node);

    DBUG_RETURN (z);
}

/** <!--*******************************************************************-->
 *
 * @fn void *LFUcreateVardecs( constant *idx, void *accu, void *scalar_type)
 *
 * @brief utility function for creating vardecs
 *
 *****************************************************************************/
static void *
LFUcreateVardecs (constant *idx, void *accu, void *scalar_type)
{
    accu = TBmakeVardec (TBmakeAvis (TRAVtmpVar (), TYcopyType ((ntype *)scalar_type)),
                         (node *)accu);
    DBUG_PRINT ("Created vardec: %s", VARDEC_NAME (accu));

    return (accu);
}

/** <!--*******************************************************************-->
 *
 * @fn void *LFUcreateAssigns( constant *idx, void *accu, void *local_info)
 *
 * fold function for creating assignment chains.
 *
 *****************************************************************************/
static void *
LFUcreateAssigns (constant *idx, void *accu, void *local_info)
{
    node *scal_avis;
    node *array_avis;
    node *avis;
    struct ca_info *l_info;

    l_info = (struct ca_info *)local_info;

    scal_avis = ID_AVIS (EXPRS_EXPR (l_info->exprs));
    array_avis = l_info->avis;

    /**
     * create a temp variable to hold the index:
     */
    avis = TBmakeAvis (TRAVtmpVar (),
                       TYmakeAKV (TYmakeSimpleType (T_int), COcopyConstant (idx)));
    DBUG_PRINT ("Created avis: %s", AVIS_NAME (avis));
    l_info->vardecs = TBmakeVardec (avis, l_info->vardecs);

    /**
     * create the selection:
     */
    accu = TBmakeAssign (TBmakeLet (TBmakeIds (scal_avis, NULL),
                                    TCmakePrf2 (F_idx_sel, TBmakeId (avis),
                                                TBmakeId (array_avis))),
                         (node *)accu);
    AVIS_SSAASSIGN (scal_avis) = (node *)accu;

    /**
     * create the assignment of the constant index:
     */
    accu = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), COconstant2AST (idx)),
                         (node *)accu);
    AVIS_SSAASSIGN (avis) = (node *)accu;

    l_info->exprs = EXPRS_NEXT (l_info->exprs);

    return (accu);
}

/** <!--*******************************************************************-->
 *
 * @fn node *LFUscalarizeArray( node *avis, node **preassigns,
 *                               node **vardecs, shape *shp)
 *
 * This function generates an exprs chain of new scalar identifiers a1, ...,
 * and of the same element type as avis, and returns these.
 *
 *    a1 = exprs[ shp-shp];
 *       ...
 *    an = exprs[ shp-1];
 *
 * which are stored in preassigns for later insertion.
 * The according vardecs are stored in vardecs.
 *
 * @params avis: the array we want scalarized.
 *
 *         preassigns: indirect pointer to where new preassign chain
 *                     should go.
 *
 *         vardecs:    indirect pointer to where new vardec chain should go.
 *
 *         shp:        optional shape to be used. If NULL, we use AVIS_SHAPE.
 *                     This argument was introduced to fix Bug #1027,
 *                     where the external call to a lacfun was int[2],
 *                     but the recursive call was still int[.].
 *
 *
 *****************************************************************************/
node *
LFUscalarizeArray (node *avis, node **preassigns, node **vardecs, shape *shp)
{
    node *new_exprs;
    ntype *scalar_type;
    node *new_vardecs;
    node *new_assigns;
    struct ca_info local_info;
    struct ca_info *local_info_ptr = &local_info;

    DBUG_ENTER ();

    shp = (NULL == shp) ? TYgetShape (AVIS_TYPE (avis)) : shp;
    shp = SHcopyShape (shp);

    /**
     * create the vardecs:
     */
    scalar_type
      = TYmakeAKS (TYcopyType (TYgetScalar (AVIS_TYPE (avis))), SHcreateShape (0));
    new_vardecs = (node *)COcreateAllIndicesAndFold (shp, LFUcreateVardecs, NULL,
                                                     scalar_type, TRUE);

    /**
     * create the exprs:
     */
    new_exprs = TCcreateExprsFromVardecs (new_vardecs);

    /**
     * create the assignments:
     */
    local_info_ptr->exprs = new_exprs;
    local_info_ptr->avis = avis;
    local_info_ptr->vardecs = NULL;

    new_assigns = (node *)COcreateAllIndicesAndFold (shp, LFUcreateAssigns, NULL,
                                                     local_info_ptr, TRUE);
    new_vardecs = TCappendVardec (new_vardecs, local_info_ptr->vardecs);

    /**
     * prefix new vardecs and assignments
     */
    *vardecs = TCappendVardec (new_vardecs, *vardecs);
    *preassigns = TCappendAssign (new_assigns, *preassigns);

    DBUG_RETURN (new_exprs);
}

/******************************************************************************
 * @fn node *LFUcorrectSSAAssigns( node *arg_node, node *nassgn)
 *
 * @brief: Correct the AVIS_SSAASSIGN links for this N_ids chain.
 *
 * @params: arg_node - an N_ids chain
 *          nassgn - the N_assign for this N_ids chain.
 *
 * @result: Updated N_ids chain
 *
 *****************************************************************************/
node *
LFUcorrectSSAAssigns (node *arg_node, node *nassgn)
{
    node *ids;

    DBUG_ENTER ();

    ids = arg_node;
    DBUG_ASSERT (NULL != nassgn, "Non-NULL AVIS_SSAASSIGNs only, please");

    while (NULL != ids) {
        AVIS_SSAASSIGN (IDS_AVIS (ids)) = nassgn;
        ids = IDS_NEXT (ids);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 * @fn node *LFUfindAffineFunctionForLIV( node *arg_node, node *lacfundef)
 *
 * @brief: Find the maximal affine function tree for the variable that
 *         controls the recursive call in a LOOPFUN.
 *
 * @params: arg_node - not sure yet FIXME
 *          lacfundef - the N_fundef node for the loopfun
 *
 * @result: xxxx FIXME or NULL, if lacfundef is not a LOOPFUN.
 *
 *****************************************************************************/
node *
LFUfindAffineFunctionForLIV (node *arg_node, node *lacfundef)
{
    node *z = NULL;

    DBUG_ENTER ();

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *LFUgetLoopIncrementFromIslChain( node *rca, node *islchain)
 *
 * @brief Search islchain for a + or - on the induction variable, rca.
 *        If found, return the other argument to the +/-. If not, return NULL.
 *
 *        The N_exprs chain should be one of:
 *
 *            rca + Y;
 *            rca - Y;
 *            Y   + rca;
 *
 *        where we can identify the sign of Y
 *
 * @param  rca: An induction variable N_id.
 * @param  islchain: An ISL N_exprs chain of N_exprs.
 *
 * @result: An N_id or N_num or NULL.
 *
 ******************************************************************************/
node *
LFUgetLoopIncrementFromIslChain (node *rca, node *islchain)
{
    node *z = NULL;
    node *exprs;

    DBUG_ENTER ();

    while ((NULL == z) && (NULL != islchain)) {
        exprs = EXPRS_EXPR (islchain);
        if ((ID_AVIS (rca) == ID_AVIS (EXPRS_EXPR (exprs)))
            && (N_prf == NODE_TYPE (EXPRS_EXPR (EXPRS_NEXT (exprs))))
            && (F_eq_SxS == PRF_PRF (EXPRS_EXPR (EXPRS_NEXT (exprs))))) { // rca = ...
            exprs = EXPRS_NEXT (EXPRS_NEXT (exprs));                      // drop rca =
            if ((N_prf == NODE_TYPE (EXPRS_EXPR (EXPRS_NEXT (exprs))))
                && ((F_add_SxS == PRF_PRF (EXPRS_EXPR (EXPRS_NEXT (exprs))))
                    || (F_sub_SxS == PRF_PRF (EXPRS_EXPR (EXPRS_NEXT (exprs)))))) {
                if ((SCSisPositive (EXPRS_EXPR (exprs)))
                    || (SCSisNegative (EXPRS_EXPR (exprs)))) {
                    z = EXPRS_EXPR (exprs);
                } else {
                    if ((SCSisPositive (EXPRS_EXPR (EXPRS_NEXT (EXPRS_NEXT (exprs)))))
                        || (SCSisNegative (
                             EXPRS_EXPR (EXPRS_NEXT (EXPRS_NEXT (exprs)))))) {
                        z = EXPRS_EXPR (EXPRS_NEXT (EXPRS_NEXT (exprs)));
                    }
                }
            }
        }
        islchain = EXPRS_NEXT (islchain);
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *LFUgetLoopIncrementFromCondprf( node *arg_node, node *rca)
 *
 * @brief arg_node is an N_prf relational. Return the argument that is
 *        not rca.
 *
 * @param  arg_node: An N_prf
 * @param  rca: The induction variable N_id.
 *
 * @result: An N_id.
 *
 ******************************************************************************/
node *
LFUgetLoopIncrementFromCondprf (node *arg_node, node *rca)
{
    node *z;

    DBUG_ENTER ();

    z = (ID_AVIS (rca) == ID_AVIS (PRF_ARG1 (arg_node))) ? PRF_ARG2 (arg_node)
                                                         : PRF_ARG1 (arg_node);
    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn prf LFUdualFun( prf nprf)
 *
 * @brief Find the dual function for a relational function
 *
 * @param An N_prf
 *
 * @return An N_prf
 *
 ******************************************************************************/
prf
LFUdualFun (prf nprf)
{
    prf z;

    DBUG_ENTER ();

    switch (nprf) {
    case F_lt_SxS:
        z = F_ge_SxS;
        break;
    case F_le_SxS:
        z = F_gt_SxS;
        break;
    case F_eq_SxS:
        z = F_neq_SxS;
        break;
    case F_ge_SxS:
        z = F_lt_SxS;
        break;
    case F_gt_SxS:
        z = F_le_SxS;
        break;
    case F_neq_SxS:
        z = F_eq_SxS;
        break;
    case F_val_lt_val_SxS:
        z = F_ge_SxS;
        break;
    case F_val_le_val_SxS:
        z = F_gt_SxS;
        break;
    case F_non_neg_val_S:
        z = F_lt_SxS;
        break; // NB. Kludge (dyadic vs. monadic!)

    default:
        DBUG_ASSERT (FALSE, "Oopsie. Expected relational prf!");
        z = nprf;
        break;
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *LFUgetStrideForAffineFun( node *rcv, node *lcv)
 *
 * @brief Given a recursive call variable, rcv, attempt to find the
 *        N_id of its stride. E.g., if rcv is II', then
 *        we look for:
 *         II' = II +- stride
 *        or
 *         II' = stride + II
 *
 * @param rcv - a recursive call variable in a LOOPFUN (II')
 * @param lcv - the FUNDEF_ARG value in LOOPFUN corresponding to rcv (II)
 *
 * @return the stride, or NULL if we are unable to determine the
 *         stride.
 *
 ******************************************************************************/
node *
LFUgetStrideForAffineFun (node *rcv, node *lcv)
{
    node *z = NULL;
    node *fn = NULL;

    DBUG_ENTER ();

    fn = LET_EXPR (ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (rcv))));
    if (N_prf == NODE_TYPE (fn)) {
        switch (PRF_PRF (fn)) {

        case F_add_SxS:
            if (ID_AVIS (PRF_ARG1 (fn)) == ID_AVIS (lcv)) { // II + stride
                z = PRF_ARG2 (fn);
            } else {
                if (ID_AVIS (PRF_ARG2 (fn)) == ID_AVIS (lcv)) { // stride + II
                    z = PRF_ARG1 (fn);
                }
            }
            break;

        case F_sub_SxS: // II - stride
            if (ID_AVIS (PRF_ARG1 (fn)) == ID_AVIS (lcv)) {
                z = PRF_ARG1 (fn);
            }

            break;

        default:
            break;
        }
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn int LFUgetMathSignumForAffineFun( node *rcv, node *lcv)
 *
 * @brief Given a recursive call variable, rcv, attempt to find the
 *        mathsignum of its stride. The mathsignum is 1 if the N_prf
 *        is +, and -1 if the N_prf is -.
 *        we look for:
 *         II' = II +- stride
 *        or
 *         II' = stride + II
 *
 * @param rcv - a recursive call variable in a LOOPFUN (II')
 * @param lcv - the FUNDEF_ARG value in LOOPFUN corresponding to rcv (II)
 *
 * @return the mathsignum, or zero if we are unable to determine it.
 *
 ******************************************************************************/
int
LFUgetMathSignumForAffineFun (node *rcv, node *lcv)
{
    node *fn;
    int mathsignum = 0;

    DBUG_ENTER ();

    fn = LET_EXPR (ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (rcv))));
    if (N_prf == NODE_TYPE (fn)) {
        mathsignum
          = (F_add_SxS == PRF_PRF (fn)) ? 1 : (F_sub_SxS == PRF_PRF (fn)) ? -1 : 0;
    }

    DBUG_RETURN (mathsignum);
}

#undef DBUG_PREFIX
