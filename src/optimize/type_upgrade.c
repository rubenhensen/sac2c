/* *
 * $Log$
 * Revision 1.4  2004/12/09 13:38:43  mwe
 * some small changes
 *
 * Revision 1.3  2004/12/08 17:31:44  mwe
 * starting to implement TryToSpecializeFunction
 *
 * Revision 1.2  2004/12/08 13:59:24  mwe
 * work continued ...
 *
 * Revision 1.1  2004/12/08 12:17:59  mwe
 * Initial revision
 *
 */

/**************************************************************************
 *
 * DESCRIPTION:
 *
 **************************************************************************/

#include "tree_basic.h"
#include "traverse.h"
#include "node_basic.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "dbug.h"
#include "internal_lib.h"
#include "optimize.h"
#include "free.h"
#include "DupTree.h"
#include "globals.h"

#include "type_upgrade.h"

/************************************************************************
 *
 * LOCAL HELPER FUNCTIONS:
 *
 ***********************************************************************/

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *withid;
    bool corrlf;
    bool chklf;
};

#define INFO_TUP_FUNDEF(n) (n->fundef)
#define INFO_TUP_WITHID(n) (n->withid)
#define INFO_TUP_CORRECTFUNCTION(n) (n->corrlf)
#define INFO_TUP_CHECKLOOPFUN(n) (n->chklf)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");
    result = ILIBmalloc (sizeof (info));

    INFO_TUP_FUNDEF (result) = NULL;
    INFO_TUP_WITHID (result) = NULL;
    INFO_TUP_CORRECTFUNCTION (result) = TRUE;
    INFO_TUP_CHECKLOOPFUN (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/*****************************************************************************
 *
 *
 *
 *
 *****************************************************************************/
static node *
TryToDoTypeUpgrade (node *fundef)
{
    info *arg_info;

    DBUG_ENTER ("TryToDoTypeUpgrade");
    INFO_TUP_CORRECTFUNCTION (arg_info) = TRUE;
    INFO_TUP_CHECKLOOPFUN (arg_info) = TRUE;

    if (fundef != NULL) {

        arg_info = MakeInfo ();

        INFO_TUP_FUNDEF (arg_info) = fundef;

        TRAVpush (TR_tup);
        fundef = TRAVdo (fundef, arg_info);
        TRAVpop ();
    }

    if (!(INFO_TUP_CORRECTFUNCTION (arg_info))) {

        /*
         * Wrong types for recursive call
         */

        fundef = FREEdoFreeTree (fundef);
        fundef = NULL;
    }

    arg_info = FreeInfo (arg_info);
    DBUG_RETURN (fundef);
}

/***********************************************************************
 *
 * function:
 *   ntype *GetLowestType(ntype *a, ntype *b)
 *
 * description:
 *   This function returns a copy of the lowest type of 'a' and 'b'.
 *   If a or b are product-types, they contain only one element. Because
 *   only this element is needed, the product-type is freed.
 *   !! 'a' and 'b' are released at the end of the function !!
 *
 ***********************************************************************/
static ntype *
GetLowestType (ntype *a, ntype *b)
{

    ntype *result, *tmp;

    DBUG_ENTER ("GetLowestType");

    if (TYisProd (a)) {
        tmp = TYcopyType (TYgetProductMember (a, 0));
        a = TYfreeType (a);
        a = tmp;
    }

    if (TYisProd (b)) {
        tmp = TYcopyType (TYgetProductMember (b, 0));
        b = TYfreeType (b);
        b = tmp;
    }

    result = (TYleTypes (a, b)) ? TYcopyType (a) : TYcopyType (b);

    a = TYfreeType (a);
    b = TYfreeType (b);

    DBUG_RETURN (result);
}

/***********************************************************************
 *
 * function:
 *   node *AssignTypeToExpr(node *expr, ntype* type)
 *
 * description:
 *   The type information 'type' is appended to 'expr' depending on the
 *   NODE_TYPE of 'expr'.
 *   Type information for id nodes is appended at the corresponding
 *   avis-node. Type information for arrays is appended directly at
 *   the array-node.
 *
 ***********************************************************************/
static node *
AssignTypeToExpr (node *expr, ntype *type)
{

    DBUG_ENTER ("AssignTypeToExpr");

    if (NODE_TYPE (expr) == N_id) {

        /*
         * store type in avis node of expr
         */
        DBUG_ASSERT ((NULL != ID_AVIS (expr)), "missing AVIS node!");

        AVIS_TYPE (ID_AVIS (expr)) = TYfreeType (AVIS_TYPE (ID_AVIS (expr)));
        AVIS_TYPE (ID_AVIS (expr)) = TYcopyType (type);

    } else if (NODE_TYPE (expr) == N_array) {

        /*
         * nothing to do for array node
         */

    } else {

        DBUG_ASSERT ((FALSE), "Unexpected node type found");
    }

    DBUG_RETURN (expr);
}

/**********************************************************************
 *
 * function:
 *   bool *IsArgumentOfSpecialFunction(node *arg)
 *
 * description:
 *   This function returns TRUE iff 'arg' is an argument of a
 *   special function (confun, dofun, whilefun).
 *
 *********************************************************************/
static bool
IsArgumentOfSpecialFunction (node *arg)
{

    bool result;
    DBUG_ENTER ("IsArgumentOfSpecialFunction");

    if (N_array == NODE_TYPE (arg)) {

        result = FALSE;

    } else if (N_id == NODE_TYPE (arg)) {

        if (N_vardec == NODE_TYPE (AVIS_DECL (ID_AVIS (arg))))
            result = FALSE;

        else if (N_arg == NODE_TYPE (AVIS_DECL (ID_AVIS (arg)))) {

            /*
             * arg is an argument of a function.
             * Now we have to verify wheter it is a special function or not.
             */
            DBUG_ASSERT ((ARG_FUNDEF (AVIS_DECL (ID_AVIS (arg))) != NULL),
                         "No pointer to fundef found");

            if ((FUNDEF_ISCONDFUN (ARG_FUNDEF (AVIS_DECL (ID_AVIS (arg)))))
                || (FUNDEF_ISDOFUN (ARG_FUNDEF (AVIS_DECL (ID_AVIS (arg)))))) {
                result = TRUE;
            } else {
                result = FALSE;
            }
        } else {

            result = FALSE;
            DBUG_ASSERT ((FALSE), "unexpected node type found!");
        }

    } else {

        result = FALSE;
        DBUG_ASSERT ((FALSE), "unexpected node type found!");
    }

    DBUG_RETURN (result);
}

/***************************************************************************************
 *
 *
 *
 **************************************************************************************/
static node *
AdjustSignatureToArgs (node *signature, node *args)
{
    node *tmp = signature;
    DBUG_ENTER ("AdjustSignatureToArgs");

    while (tmp != NULL) {

        AVIS_TYPE (ARG_AVIS (signature)) = TYfreeType (AVIS_TYPE (ARG_AVIS (signature)));
        AVIS_TYPE (ARG_AVIS (signature))
          = TYcopyType (AVIS_TYPE (ID_AVIS (EXPRS_EXPR (args))));

        args = EXPRS_NEXT (args);
        tmp = ARG_NEXT (tmp);
    }

    DBUG_RETURN (signature);
}

/****************************************************************************
 *
 * function:
 *   node *TryToSpecializeFunction(node* fundef, node* args, info *arg_info)
 *
 * description:
 *   fundef is a pointer to the fundef-node of the function, which should be
 *   specialized and args are the arguments of the current function call.
 *   If the types of the arguments are more special then the types of the
 *   function, we can try to specialize.
 *   If the function is a condfun we can specialize immidiately.
 *   If the function is a loopfun we have to try:
 *      - copy the function and specialize the copied function
 *      - update all types inside the function
 *      - check if types in recursive function call fit to specialization
 *      - if yes: keep copy and delete original
 *      - if no: delete copy and keep original
 *   Every other kind of function:
 *
 ***************************************************************************/
static node *
TryToSpecializeFunction (node *fundef, node *args, info *arg_info)
{

    node *fun_args, *given_args;
    bool leave = FALSE;
    bool is_more_special = FALSE;

    DBUG_ENTER ("TryToSpecializeFunction");

    /*
     * check if args are more special the signature of function
     */

    given_args = args;
    fun_args = FUNDEF_ARGS (fundef);

    while ((!leave) && (NULL != fun_args)) {

        switch (TYcmpTypes (AVIS_TYPE (ARG_AVIS (fun_args)),
                            AVIS_TYPE (ID_AVIS (EXPRS_EXPR (given_args))))) {

        case TY_eq: /* same types: nothing to do */
            break;

        case TY_lt: /* ? found a supertype of function signature in 'args': not allowed
                       until now ? */
            is_more_special = FALSE;
            leave = TRUE;
            DBUG_ASSERT ((FALSE), "Argument of function is supertype of signature!");
            break;

        case TY_hcs:
        case TY_dis: /* arguments did not fit to signature, should never happen */
            DBUG_ASSERT ((FALSE), "Argument of function did not fit to signature!");
            break;

        case TY_gt: /* found a more special type in argument than in signature */
            is_more_special = TRUE;
            break;

        default: /* all cases are listed above, so this should never happen */
            DBUG_ASSERT ((FALSE), "New element in enumeration added?");
        }

        given_args = EXPRS_NEXT (given_args);
        fun_args = EXPRS_NEXT (fun_args);
    }

    if (is_more_special) {

        /*
         * more special types in 'args' found
         */

        if (FUNDEF_ISCONDFUN (fundef)) {

            /*
             * function is a conditional function
             */

            FUNDEF_ARGS (fundef) = AdjustSignatureToArgs (FUNDEF_ARGS (fundef), args);

            fundef = TUPdoTypeUpgrade (fundef);

        } else if (FUNDEF_ISDOFUN (fundef)) {

            /*
             * function is a loop function
             */

            node *tmp, *result;

            tmp = DUPdoDupNode (fundef);
            FUNDEF_ARGS (tmp) = AdjustSignatureToArgs (FUNDEF_ARGS (tmp), args);

            /*
             * die selbe Funktionalität wie TUPdoTypeUpgrade, aber:
             * prüft be N_ap auf rekursiven Aufruf und prüft auf passende Typen
             * Typen passen: Ergebnis ist 'upgegradete' Funktion
             * Typen passen nicht: Ergebnis ist NULL
             */
            result = TryToDoTypeUpgrade (tmp);

            if (result != NULL) {
                /*
                 * it is possible to specialiaze loop function
                 */

                FUNDEF_RETS (fundef) = FREEdoFreeTree (FUNDEF_RETS (fundef));
                FUNDEF_RETS (fundef) = FUNDEF_RETS (tmp);
                FUNDEF_RETS (tmp) = NULL;

                FUNDEF_ARGS (fundef) = FREEdoFreeTree (FUNDEF_ARGS (fundef));
                FUNDEF_ARGS (fundef) = FUNDEF_ARGS (tmp);
                FUNDEF_ARGS (tmp) = NULL;

                FUNDEF_BODY (fundef) = FREEdoFreeTree (FUNDEF_BODY (fundef));
                FUNDEF_BODY (fundef) = FUNDEF_BODY (tmp);
                FUNDEF_BODY (tmp) = NULL;

                FREEdoFreeTree (tmp);
            } else {
                /*
                 * it is not possible to specialize loop function
                 */
                FREEdoFreeTree (tmp);
            }

        } else {

            /*
             * function is no special function
             */
        }
    }

    DBUG_RETURN (fundef);
}

static node *
TryStaticDispatch (node *fundef, info *arg_info)
{
    DBUG_ENTER ("TryStaticDispatch");

    if (FUNDEF_ISWRAPPERFUN (fundef)) {

        /*
         * current fundef is fundef of wrapper function
         */
    }

    DBUG_RETURN (fundef);
}

/************************************************************************
 *
 * GLOBAL FUNCTIONS:
 *
 ************************************************************************/

/************************************************************************
 *
 * function:
 *    node *TypeUpgrade(node* arg_node, node* arg_info)
 *
 * description:
 *    This function is called from optimize.c, it is the starting point
 *    of TypeUpgrade.
 *    arg_node is expected to be an N_fundef node.
 *
 ***********************************************************************/

node *
TUPdoTypeUpgrade (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("TUPdoTypeUpgrade");

    if (arg_node != NULL) {

        arg_info = MakeInfo ();

        INFO_TUP_FUNDEF (arg_info) = arg_node;

        TRAVpush (TR_tup);
        arg_node = TRAVdo (arg_node, arg_info);
        TRAVpop ();
    }

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/***********************************************************************
 *
 * function:
 *   node *TUPblock(node* arg_node, node* arg_info)
 *
 * description:
 *   traverse through block nodes
 *
 **********************************************************************/
node *
TUPblock (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("TUPblock");

    if (BLOCK_INSTR (arg_node) != NULL) {

        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/***********************************************************************
 *
 * function:
 *   node *TUPassign(node* arg_node, node* arg_info)
 *
 * description:
 *   Traverse through assign nodes. While top-down traversal the
 *   instructions are handled.
 *
 **********************************************************************/
node *
TUPassign (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("TUPassign");

    if (ASSIGN_INSTR (arg_node) != NULL) {

        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {

        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**********************************************************************
 *
 * function:
 *   node *TUPreturn(node* arg_node, node* arg_info)
 *
 * description:
 *   The type of the expressions in the return statement are compared to
 *   the return type of the function. If necessary the return type of
 *   the function is updated.
 *
 **********************************************************************/
node *
TUPreturn (node *arg_node, info *arg_info)
{

    node *ret, *exprs;
    DBUG_ENTER ("TUPreturn");

    if (NULL != RETURN_EXPRS (arg_node)) {

        /* convert ntypes of return values to return type of function: add them to
         * ret-node*/

        ret = FUNDEF_RETS (INFO_TUP_FUNDEF (arg_info));
        exprs = RETURN_EXPRS (arg_node);

        while (ret != NULL) {

            switch (
              TYcmpTypes (AVIS_TYPE (ID_AVIS (EXPRS_EXPR (exprs))), RET_TYPE (ret))) {

            case TY_eq: /* same types, nothing changed */
                break;

            case TY_gt: /* lost type information, should not happen */
                DBUG_ASSERT ((FALSE), "lost type information");
                break;

            case TY_hcs:
            case TY_dis: /* both types are unrelated, should not be possible */
                DBUG_ASSERT ((FALSE), "former type is unrelated to new type! ");
                break;

            case TY_lt: /* new type is more special: update */
                RET_TYPE (ret) = TYfreeType (RET_TYPE (ret));
                RET_TYPE (ret) = TYcopyType (AVIS_TYPE (ID_AVIS (EXPRS_EXPR (exprs))));
                break;

            default: /* no other cases exist */
                DBUG_ASSERT ((FALSE), "New element in enumeration added?");
            }

            ret = RET_NEXT (ret);
            exprs = EXPRS_NEXT (exprs);
        }
    }

    DBUG_RETURN (arg_node);
}

/************************************************************************
 *
 * function:
 *   node *TUPlet(node* arg_node, node* arg_info)
 *
 * description:
 *   Depending on the 'expression' belonging to the let node, the neccessary
 *   functions for type upgrade are executed.
 *
 *************************************************************************/
node *
TUPlet (node *arg_node, info *arg_info)
{

    ntype *type, *old_type, *tmp;
    node *ret, *ids;

    DBUG_ENTER ("TUPlet");

    if (N_ap == NODE_TYPE (LET_EXPR (arg_node))) {

        /*
         *  expression is a function application
         */

        if (INFO_TUP_CHECKLOOPFUN (arg_info)) {

            /*
             * at the moment we are checking, if the recursive call of a loop function
             * will work with specialized signature
             */

            node *signature, *args;

            signature = FUNDEF_ARGS (AP_FUNDEF (LET_EXPR (arg_node)));
            args = AP_ARGS (LET_EXPR (arg_node));

            while ((INFO_TUP_CORRECTFUNCTION (arg_info)) && (signature != NULL)) {

                switch (TYcmpTypes (AVIS_TYPE (ARG_AVIS (signature)),
                                    AVIS_TYPE (ID_AVIS (EXPRS_EXPR (args))))) {

                case TY_eq: /* same types, that's ok */
                    break;
                case TY_gt: /* argument is more special than signature, that's ok */
                    break;
                case TY_lt: /* signature is more special than argument, that's a problem
                             */
                    INFO_TUP_CORRECTFUNCTION (arg_info) = FALSE;
                    break;
                case TY_hcs:
                case TY_dis: /* both types are unrelated, should not be possible */
                    DBUG_ASSERT ((FALSE), "former type is unrelated to new type! ");
                    break;

                default: /* no other cases exist */
                    DBUG_ASSERT ((FALSE), "New element in enumeration added?");
                }
                signature = ARG_NEXT (signature);
                args = EXPRS_NEXT (args);
            }
        } else {

            /* first of all, try to specialize function */
            AP_FUNDEF (LET_EXPR (arg_node))
              = TryToSpecializeFunction (AP_FUNDEF (LET_EXPR (arg_node)),
                                         AP_ARGS (LET_EXPR (arg_node)), arg_info);

            /* now we have the possibility to do an static dispatch */
            AP_FUNDEF (LET_EXPR (arg_node))
              = TryStaticDispatch (AP_FUNDEF (LET_EXPR (arg_node)), arg_info);
        }

        if (INFO_TUP_CORRECTFUNCTION (arg_info)) {
            /*
             * update left side
             * here is the only possibility for multiple ids on left side
             */

            ret = FUNDEF_RETS (AP_FUNDEF (LET_EXPR (arg_node)));
            ids = LET_IDS (arg_node);

            while (ret != NULL) {

                switch (TYcmpTypes (RET_TYPE (ret), AVIS_TYPE (IDS_AVIS (ids)))) {

                case TY_eq: /* same types, nothing changed */
                    break;

                case TY_gt: /* lost type information, should not happen */
                    DBUG_ASSERT ((FALSE), "lost type information");
                    break;

                case TY_hcs:
                case TY_dis: /* both types are unrelated, should not be possible */
                    DBUG_ASSERT ((FALSE), "former type is unrelated to new type! ");
                    break;

                case TY_lt: /* return type is more special: update */
                    AVIS_TYPE (IDS_AVIS (ids)) = TYfreeType (AVIS_TYPE (IDS_AVIS (ids)));
                    AVIS_TYPE (IDS_AVIS (ids)) = TYcopyType (RET_TYPE (ret));
                    break;

                default: /* no other cases exist */
                    DBUG_ASSERT ((FALSE), "New element in enumeration added?");
                }

                ret = RET_NEXT (ret);
                ids = IDS_NEXT (ids);
            }
        }
    } else if (N_with == NODE_TYPE (LET_EXPR (arg_node))) {

        /*
         * expression is a with loop
         */

        /* first traverse in withloop, update all expr parts */

        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    } else {
        /*
         * all other expressions: no need to traverse in substructures
         */
    }

    if (N_ap != NODE_TYPE (LET_EXPR (arg_node))) {

        /*
         * upgrade for N_ap nodes already done
         */

        DBUG_ASSERT ((IDS_AVIS (LET_IDS (arg_node)) != NULL),
                     "AVIS is NULL in IDS node!");

        old_type = AVIS_TYPE (IDS_AVIS (LET_IDS (arg_node)));
        AVIS_TYPE (IDS_AVIS (LET_IDS (arg_node))) = NULL;

        type = NTCnewTypeCheck_Expr (LET_EXPR (arg_node));

        if (TYisProd (type)) {
            tmp = TYcopyType (TYgetProductMember (type, 0));
            type = TYfreeType (type);
            type = tmp;
        }

        switch (TYcmpTypes (type, old_type)) {

        case TY_eq: /* types are equal, nothing to do */
            old_type = TYfreeType (old_type);
            AVIS_TYPE (IDS_AVIS (LET_IDS (arg_node))) = type;
            break;

        case TY_gt: /* old type is more specific, lost type information */
            DBUG_ASSERT ((FALSE), "Lost type information!");
            break;

        case TY_hcs:
        case TY_dis: /* both types are unrelated, should not be possible */
            DBUG_ASSERT ((FALSE), "former type is unrelated to new type! ");
            break;

        case TY_lt: /* new type is more specific, do upgrade */
            /*AVIS_TYPE(IDS_AVIS(LET_IDS(arg_node))) =
             * TYfreeType(AVIS_TYPE(IDS_AVIS(LET_IDS(arg_node))));*/
            old_type = TYfreeType (old_type);
            AVIS_TYPE (IDS_AVIS (LET_IDS (arg_node))) = type;
            tup_expr++;
            break;

        default: /* all cases are listed above, so default should never be entered*/
            DBUG_ASSERT ((FALSE), "New element in enumeration added?");
        }
    }

    DBUG_RETURN (arg_node);
}

/********************************************************************************
 *
 * function:
 *   node *TUPwith(node *arg_node, node* arg_info)
 *
 * description:
 *
 ********************************************************************************/
node *
TUPwith (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("TUPwith");

    /*
     * top node of with loop
     * traverse in substructures
     */

    if (NULL != WITH_PART (arg_node)) {
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    }

    if (NULL != WITH_CODE (arg_node)) {
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *TUPpart(node *arg_node, node *arg_info)
 *
 * description:
 *
 *****************************************************************************/
node *
TUPpart (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("TUPpart");

    /*
     * partition of with loop
     * traverse in generator, update types
     * traverse in next partition
     */

    INFO_TUP_WITHID (arg_info) = PART_WITHID (arg_node);

    if (PART_GENERATOR (arg_node) != NULL) {
        PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);
    }

    if (PART_NEXT (arg_node) != NULL) {
        PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**************************************************************************
 *
 * function:
 *   node *TUPgenerator(node *arg_node, node *arg_info)
 *
 * description:
 *   All 'expressions' belonging to generator-node should have (if possible)
 *   the same type.
 *   So the best possible type is determined and all expressions are updated
 *   if possible.
 *
 **************************************************************************/
node *
TUPgenerator (node *arg_node, info *arg_info)
{

    ntype *current, *tmp;
    node *withid;

    DBUG_ENTER ("TUPgenerator");

    current = NTCnewTypeCheck_Expr (GENERATOR_BOUND1 (arg_node));
    tmp = NTCnewTypeCheck_Expr (GENERATOR_BOUND2 (arg_node));
    current = GetLowestType (current, tmp);

    if (NULL != GENERATOR_STEP (arg_node)) {

        tmp = NTCnewTypeCheck_Expr (GENERATOR_STEP (arg_node));
        current = GetLowestType (current, tmp);

        tmp = NTCnewTypeCheck_Expr (GENERATOR_WIDTH (arg_node));
        current = GetLowestType (current, tmp);
    }

    /*
     * current is the best possible type for all generator values
     * assign current to all generator values iff the updated
     * value is no argument of a special function (funcond, dofun)
     */

    if (IsArgumentOfSpecialFunction (GENERATOR_BOUND1 (arg_node)) == FALSE) {
        GENERATOR_BOUND1 (arg_node)
          = AssignTypeToExpr (GENERATOR_BOUND1 (arg_node), current);
    }

    if (IsArgumentOfSpecialFunction (GENERATOR_BOUND2 (arg_node)) == FALSE) {
        GENERATOR_BOUND2 (arg_node)
          = AssignTypeToExpr (GENERATOR_BOUND2 (arg_node), current);
    }

    if (NULL != GENERATOR_STEP (arg_node)) {

        if (IsArgumentOfSpecialFunction (GENERATOR_STEP (arg_node)) == FALSE)
            GENERATOR_STEP (arg_node)
              = AssignTypeToExpr (GENERATOR_STEP (arg_node), current);

        if (IsArgumentOfSpecialFunction (GENERATOR_WIDTH (arg_node)) == FALSE)
            GENERATOR_WIDTH (arg_node)
              = AssignTypeToExpr (GENERATOR_WIDTH (arg_node), current);
    }

    /*
     * update withid with same type
     */

    withid = INFO_TUP_WITHID (arg_info);

    DBUG_ASSERT (TYleTypes (current, AVIS_TYPE (IDS_AVIS (WITHID_VEC (withid)))),
                 "Lost type information!");

    AVIS_TYPE (IDS_AVIS (WITHID_VEC (withid)))
      = TYfreeType (AVIS_TYPE (IDS_AVIS (WITHID_VEC (withid))));
    AVIS_TYPE (IDS_AVIS (WITHID_VEC (withid))) = TYcopyType (current);

    current = TYfreeType (current);

    DBUG_RETURN (arg_node);
}

/******************************************************************
 *
 * function:
 *   node *TUPcode(node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************/
node *
TUPcode (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("TUPcode");

    /*
     * this is the code block of a with loop
     * CBLOCK contains this code, CEXPRS conains the raturn value(s)
     * CEXPRS has to be an id-node, so no typeupgrade has to be done
     */

    if (CODE_CBLOCK (arg_node) != NULL) {
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    }

    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
