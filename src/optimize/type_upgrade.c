/* *
 * $Log$
 * Revision 1.17  2005/01/30 23:09:03  mwe
 * ... and one more time: debugging
 *
 * Revision 1.16  2005/01/28 22:34:23  mwe
 * correct duplication of lac-funs
 * support for CODE_CEXPRS chain added
 *
 * Revision 1.15  2005/01/28 16:39:18  mwe
 * debugging
 *
 * Revision 1.14  2005/01/27 17:01:59  mwe
 * debugging
 *
 * Revision 1.13  2005/01/26 17:35:43  mwe
 * debugging
 *
 * Revision 1.12  2005/01/21 17:37:59  mwe
 * small changes
 *
 * Revision 1.11  2005/01/21 13:20:17  mwe
 * more implementation done
 *
 * Revision 1.9  2005/01/18 16:30:24  mwe
 * ongoing implementation...
 *
 * Revision 1.8  2005/01/17 08:52:41  mwe
 * implementation continue...
 *
 * Revision 1.7  2005/01/11 16:05:37  mwe
 * changes in todo-list :-)
 *
 * Revision 1.6  2005/01/11 15:21:11  mwe
 * ongoing implementation
 *
 * Revision 1.5  2004/12/13 14:08:21  mwe
 * new traversals added
 *
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

/* TODO:
 *   ast.xml, DupTree, print: implement FUNGROUP (DONE)
 *   DeadFunctionRemoval: remove function from FUNGROUP_FUNLIST and decrease REFCOUNTER
 * (DONE) DupTree: check if all necessary attributes of FUNDEF are copied (DONE) ast.xml:
 * add missing arguments to FUNDEF (DONE) implement funcond traversal (DONE) implement
 * support for type_error (DONE) implement creation of fungroups (DONE, but maybe find
 * better place for code) comment everything in doxygen style insert DBUG_PRINT's
 *   CODE_CEXPRS could contain multiple return values (DONE)
 *   use FUNDEF_COUNT in a correct way (DONE)
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
#include "type_utils.h"
#include "ct_with.h"
#include "type_errors.h"
#include "ct_prf.h"
#include "ct_with.h"
#include "ct_fun.h"
#include "constants.h"
#include "shape.h"
#include "ct_basic.h"
#include "tree_compound.h"
#include "ssi.h"

#include "Error.h"

#include "type_upgrade.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    ntype *withindex;
    node *withid;
    bool corrlf;
    bool chklf;
    int counter;
    ntype *type;
    ntype *wlexprs;
    node *bestfit;
    bool then_terr;
    bool else_terr;
    bool type_error;
    node *assign;
};

#define INFO_TUP_FUNDEF(n) (n->fundef)
#define INFO_TUP_WITHINDEX(n) (n->withindex)
#define INFO_TUP_WITHID(n) (n->withid)
#define INFO_TUP_CORRECTFUNCTION(n) (n->corrlf)
#define INFO_TUP_CHECKLOOPFUN(n) (n->chklf)
#define INFO_TUP_TYPE(n) (n->type)
#define INFO_TUP_TYPECOUNTER(n) (n->counter)
#define INFO_TUP_WLEXPRS(n) (n->wlexprs)
#define INFO_TUP_BESTFITTINGFUN(n) (n->bestfit)
#define INFO_TUP_THENTYPEERROR(n) (n->then_terr)
#define INFO_TUP_ELSETYPEERROR(n) (n->else_terr)
#define INFO_TUP_TYPEERROR(n) (n->type_error)
#define INFO_TUP_ASSIGN(n) (n->assign)
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");
    result = ILIBmalloc (sizeof (info));

    INFO_TUP_FUNDEF (result) = NULL;
    INFO_TUP_WITHID (result) = NULL;
    INFO_TUP_WITHINDEX (result) = NULL;
    INFO_TUP_CORRECTFUNCTION (result) = TRUE;
    INFO_TUP_CHECKLOOPFUN (result) = FALSE;
    INFO_TUP_TYPE (result) = NULL;
    INFO_TUP_TYPECOUNTER (result) = 0;
    INFO_TUP_WLEXPRS (result) = NULL;
    INFO_TUP_BESTFITTINGFUN (result) = NULL;
    INFO_TUP_THENTYPEERROR (result) = FALSE;
    INFO_TUP_ELSETYPEERROR (result) = FALSE;
    INFO_TUP_TYPEERROR (result) = FALSE;
    INFO_TUP_ASSIGN (result) = NULL;
    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/************************************************************************
 *
 * LOCAL HELPER FUNCTIONS:
 *
 ***********************************************************************/

/*****************************************************************************
 *
 * function:
 *   bool IsSupportedPrf(node *prf, info *arg_info)
 *
 * description:
 *   Unfortunately not all primitive operations are supported by
 *   the type checker. All primitive operations and necessary informations
 *   are defined in prf_node_info.mac.
 *   All primitive operations, which call NTCCTprf_dummy as evaluation
 *   function for the result type are not supported at this time.
 *   PRF_PRF contains an enumeration type of all primitive operations, which
 *   is used as an index in global.ntc_funtab to find the corresponding
 *   evaluation function.
 *   If NTCCTprf_dummy is that function, this primitive operation is not
 *   supported, so FALSE is the result of this function.
 *   Otherwise it is TRUE.
 *
 *****************************************************************************/

static bool
IsSupportedPrf (node *prf, info *arg_info)
{
    bool supported;
    DBUG_ENTER ("IsSupportedPrf");

    if (global.ntc_funtab[PRF_PRF (prf)] == NTCCTprf_dummy) {
        supported = FALSE;
    } else {
        supported = TRUE;
    }

    DBUG_RETURN (supported);
}

/*****************************************************************************
 *
 * function:
 *   node *TryToDoTypeUpgrade( node* fundef)
 *
 * description:
 *
 *****************************************************************************/

static node *
TryToDoTypeUpgrade (node *fundef)
{
    info *arg_info;

    DBUG_ENTER ("TryToDoTypeUpgrade");

    if (fundef != NULL) {

        NOTE (("!!!!TryToDoTypeUpgrade!!! "));

        arg_info = MakeInfo ();
        INFO_TUP_CORRECTFUNCTION (arg_info) = TRUE;
        INFO_TUP_CHECKLOOPFUN (arg_info) = TRUE;

        INFO_TUP_FUNDEF (arg_info) = fundef;

        TRAVpush (TR_tup);
        FUNDEF_BODY (fundef) = TRAVdo (FUNDEF_BODY (fundef), arg_info);
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

/**********************************************************************
 *
 * @fn
 * @brief
 * @param
 * @return
 *
 **********************************************************************/
static int
GetSizeOfChain (node *root)
{
    int result = 0;
    DBUG_ENTER ("GetSizeOfChain");

    if (root != NULL) {
        if (N_arg == NODE_TYPE (root)) {
            while (root != NULL) {
                result++;
                root = ARG_NEXT (root);
            }
        } else if (N_exprs == NODE_TYPE (root)) {
            while (root != NULL) {
                result++;
                root = EXPRS_NEXT (root);
            }
        } else {
            DBUG_ASSERT ((FALSE), "unexpected node type found");
        }
    }

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
 *   special function (confun, dofun). Whilefuns are now already
 *   changed to Dofuns.
 *
 *********************************************************************/
static bool
IsArgumentOfSpecialFunction (node *arg)
{
    bool result;
    DBUG_ENTER ("IsArgumentOfSpecialFunction");

    switch (NODE_TYPE (arg)) {
    case N_array:
        result = FALSE;
        break;

    case N_id:
        if (N_vardec == NODE_TYPE (AVIS_DECL (ID_AVIS (arg)))) {
            result = FALSE;
        }

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
        break;

    default:
        result = FALSE;
        DBUG_ASSERT ((FALSE), "unexpected node type found!");
    }

    DBUG_RETURN (result);
}

/***************************************************************************************
 *
 *  function:
 *    node *AdjustSignatureToArgs(node *signature, node *args)
 *
 *  description:
 *    All types of signature (arg_node chain) are freed and replaced by corresponding
 *    types of args (exprs_node chain).
 *
 **************************************************************************************/
static node *
AdjustSignatureToArgs (node *signature, node *args)
{
    node *tmp = signature;
    ntype *old, *new;
    DBUG_ENTER ("AdjustSignatureToArgs");

    while (tmp != NULL) {

        old = AVIS_TYPE (ARG_AVIS (tmp));
        new = AVIS_TYPE (ID_AVIS (EXPRS_EXPR (args)));
        AVIS_TYPE (ARG_AVIS (tmp)) = TYfreeType (AVIS_TYPE (ARG_AVIS (tmp)));
        AVIS_TYPE (ARG_AVIS (tmp)) = TYcopyType (AVIS_TYPE (ID_AVIS (EXPRS_EXPR (args))));

        args = EXPRS_NEXT (args);
        tmp = ARG_NEXT (tmp);
    }

    DBUG_RETURN (signature);
}

/**********************************************************************************
 *
 * function:
 *   node *AppendFundef(node* fundef, node* result)
 *
 * description:
 *   The fundef-node 'result' is appended at the end of the
 *   fundef-chain of fundef-node 'fundef'.
 *
 ***********************************************************************************/
static node *
AppendFundef (node *fundef, node *result)
{
    node *tmp;

    DBUG_ENTER ("AppendFundef");

    if ((NULL != fundef) && (NULL != result)) {

        tmp = fundef;
        while (NULL != FUNDEF_NEXT (tmp)) {
            tmp = FUNDEF_NEXT (tmp);
        }

        FUNDEF_NEXT (result) = NULL;
        FUNDEF_NEXT (tmp) = result;
    }

    DBUG_RETURN (fundef);
}

/************************************************************************
 *
 *  function:
 *    node *TryStaticDispatch( node *fundef, node *args, info *arg_info)
 *
 *  description:
 *    If fundef belongs to a wrapper function a static dispatch is apllied
 *    if possible. The code is reused from typechecker.
 *
 **************************************************************************/
static node *
TryStaticDispatch (node *fundef, node *args, info *arg_info)
{
    ntype *prodarg;
    dft_res *dft_res;
    node *fundef_res = NULL;
    char *funname;
    DBUG_ENTER ("TryStaticDispatch");

    if ((FUNDEF_ISWRAPPERFUN (fundef)) && (NULL != args)) {

        /*
         * current fundef belongs to wrapper function
         */
        prodarg = NTCnewTypeCheck_Expr (args);

        dft_res = NTCCTdispatchFunType (fundef, prodarg);

        funname = FUNDEF_NAME (fundef);
        if (dft_res == NULL) {
            DBUG_ASSERT ((TYgetProductSize (prodarg) == 0),
                         "illegal dispatch result found!");
            /*
             * no args found -> static dispatch possible
             *
             * fundef can be found in FUNDEF_IMPL (dirty hack!)
             */

            DBUG_ASSERT ((FALSE), "This should not happen!");
        } else if ((dft_res->num_partials == 0)
                   && (dft_res->num_deriveable_partials == 0)) {
            /*
             * static dispatch possible
             */
            if (dft_res->def != NULL) {
                DBUG_ASSERT ((dft_res->deriveable == NULL), "def and deriveable found!");
                fundef_res = dft_res->def;
            } else {
                fundef_res = dft_res->deriveable;
            }

            DBUG_PRINT ("TUP", ("  dispatched statically %s", funname));
        }
    }

    DBUG_RETURN (fundef_res);
}

/*******************************************************************
 *
 *  function:
 *    bool ArgumentIsSpecializeable(ntype *type)
 *
 *  description:
 *
 **********************************************************************/
static bool
ArgumentIsSpecializeable (ntype *type)
{
    bool result = TRUE;

    DBUG_ENTER ("ArgumentIsSpecializeable");

    switch (global.sigspec_mode) {

    case SSP_aud:
        result = FALSE;
        break;

    case SSP_akd:
        if (TYisAKD (type)) {
            result = FALSE;
        }

    case SSP_aks:
        if (TYisAKS (type)) {
            result = FALSE;
        }

    case SSP_akv:
        if (TYisAKV (type)) {
            result = FALSE;
        }
        break;
        ;
    default:
        DBUG_ASSERT ((FALSE),
                     "Unknown enumeration element! Incomplete switch statement!");
    }

    DBUG_RETURN (result);
}

/************************************************************************
 *
 *  function:
 *    bool IsSpecializeable(node *fundef, node *args)
 *
 *  description:
 *    This function checks if all formal conditions are satisfied for
 *    speciliazation of fundef.
 *    This means: - speciliazation counter has not reached limit
 *                - at least one argument could be specialized
 *                - number of arguments in exprs- and arg-chain
 *                  are the same
 *    If this is fulfilled, this function returns TRUE,
 *    otherwise FALSE.
 ***********************************************************************/
static bool
IsSpecializeable (node *fundef, node *args)
{
    bool result = FALSE;
    node *fun_args;

    DBUG_ENTER ("IsSpecializeable");

    /*
     * check specialization counter
     * check arguments, whether the are already special enough or not
     */
    if (FUNGROUP_SPECCOUNTER (FUNDEF_FUNGROUP (fundef)) < global.maxspec) {

        /*
         * limit of function specialization is still not reached
         */
        fun_args = FUNDEF_ARGS (fundef);

        while ((fun_args != NULL) && (args != NULL)) {

            if ((ArgumentIsSpecializeable (AVIS_TYPE (ARG_AVIS (fun_args))))
                && (TY_lt
                    == TYcmpTypes (AVIS_TYPE (ID_AVIS (EXPRS_EXPR (args))),
                                   AVIS_TYPE (ARG_AVIS (fun_args))))) {
                /*
                 * at least one argument could be specialized
                 */
                result = TRUE;
            }
            fun_args = ARG_NEXT (fun_args);
            args = EXPRS_NEXT (args);
        }
        if (args != fun_args) {
            result = FALSE;
        }
    }

    DBUG_RETURN (result);
}

/***************************************************************************
 *
 *  function:
 *    node *GetBestPossibleType(node *fundef, node *args)
 *
 *  description:
 *
 ***************************************************************************/
static ntype *
GetBestPossibleType (ntype *type)
{
    ntype *result = NULL;
    DBUG_ENTER ("GetBestPossibleType");

    switch (global.sigspec_mode) {
    case SSP_aud:
        /*
         * nothing to do
         */
        break;
    case SSP_akd:
        if ((TYisAUD (type)) || (TYisAUDGZ (type))) {
            DBUG_ASSERT ((FALSE), "current type could not be transformed to akd type");
        } else if (TYisAKD (type)) {
            result = TYcopyType (type);
        } else if ((TYisAKS (type)) || TYisAKV (type)) {
            result = TYmakeAKD (TYcopyType (TYgetScalar (type)), 0,
                                SHcopyShape (TYgetShape (type)));
        } else {
            DBUG_ASSERT ((FALSE), "unexpected type");
        }
        break;
    case SSP_aks:
        if ((TYisAUD (type)) || (TYisAUDGZ (type)) || (TYisAKD (type))) {
            DBUG_ASSERT ((FALSE), "current type could not be transformed to aks type");
        } else if (TYisAKS (type)) {
            result = TYcopyType (type);
        } else if (TYisAKV (type)) {

            result = TYmakeAKS (TYcopyType (TYgetScalar (type)),
                                SHcopyShape (TYgetShape (type)));
        } else {
            DBUG_ASSERT ((FALSE), "unexpected type");
        }
        break;
    case SSP_akv:
        if ((TYisAUD (type)) || (TYisAUDGZ (type)) || (TYisAKD (type))
            || (TYisAKS (type))) {
            DBUG_ASSERT ((FALSE), "current type could not be transformed to akv type");
        } else if (TYisAKV (type)) {
            result = TYcopyType (type);
        } else {
            DBUG_ASSERT ((FALSE), "unexpected type");
        }
        break;
    default:
        DBUG_ASSERT ((FALSE),
                     "Unknown enumeration element! Incomplete switch statement!");
    }

    DBUG_RETURN (result);
}

/***************************************************************************
 *
 *  function:
 *    node *SpecializationOracle(node *fundef, node *args)
 *
 *  description:
 *
 ***************************************************************************/
static node *
SpecializationOracle (node *fundef, node *args)
{
    node *result = fundef;
    node *signature;
    bool updated = FALSE;
    ntype *new_type;
    DBUG_ENTER ("SpecializationOracle");

    if (IsSpecializeable (fundef, args)) {

        /*
         * duplicate function
         */
        result = DUPdoDupNode (fundef);

        /*
         * increse specialization counter in fungroup
         */
        (FUNGROUP_SPECCOUNTER (FUNDEF_FUNGROUP (result))) += 1;

        /*
         * append function to AST
         */
        fundef = AppendFundef (fundef, result);

        /*
         * change signature
         */
        signature = FUNDEF_ARGS (result);
        while (signature != NULL) {

            if (ArgumentIsSpecializeable (AVIS_TYPE (ARG_AVIS (signature)))) {

                /*
                 * signature could be updated
                 * first: check if argument is more special
                 * second: get the best inferable type
                 */
                if (TY_lt
                    == TYcmpTypes (AVIS_TYPE (ID_AVIS (EXPRS_EXPR (args))),
                                   AVIS_TYPE (ARG_AVIS (signature)))) {

                    new_type
                      = GetBestPossibleType (AVIS_TYPE (ID_AVIS (EXPRS_EXPR (args))));

                    /*
                     * nwe_type is not to special
                     */
                    AVIS_TYPE (ARG_AVIS (signature))
                      = TYfreeType (AVIS_TYPE (ARG_AVIS (signature)));
                    AVIS_TYPE (ARG_AVIS (signature)) = new_type;

                    updated = TRUE;
                }
            }
            signature = ARG_NEXT (signature);
            args = EXPRS_NEXT (args);
        }
    }

    if (updated) {
        NOTE2 (("           *** function specialized via oracle:", FUNDEF_NAME (fundef)));
        if (FUNDEF_ISWRAPPERFUN (fundef)) {
            tup_wdp_expr++;
        } else {
            tup_fdp_expr++;
        }
    }

    DBUG_RETURN (result);
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
TryToSpecializeFunction (node *fundef, node *ap_node, info *arg_info)
{

    node *fun_args, *given_args, *args;
    bool leave = FALSE;
    bool is_more_special = FALSE;

    ntype *old, *new;

    DBUG_ENTER ("TryToSpecializeFunction");

    /*
     * check if args are more special than signature of function
     */

    args = AP_ARGS (ap_node);
    given_args = args;
    fun_args = FUNDEF_ARGS (fundef);

    while ((!leave) && (NULL != fun_args)) {

        old = AVIS_TYPE (ARG_AVIS (fun_args));
        new = AVIS_TYPE (ID_AVIS (EXPRS_EXPR (given_args)));

        switch (TYcmpTypes (AVIS_TYPE (ARG_AVIS (fun_args)),
                            AVIS_TYPE (ID_AVIS (EXPRS_EXPR (given_args))))) {

        case TY_eq: /* same types: nothing to do */
            break;

        case TY_lt: /* ? found a supertype of function signature in 'args': not allowed
                       until now ? */
            is_more_special = FALSE;
            leave = TRUE;
            /*DBUG_ASSERT( (FALSE) ,"Argument of function is supertype of signature!");*/
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
        fun_args = ARG_NEXT (fun_args);
    }

    if (is_more_special) {

        /*
         * more special types in 'args' found
         */

        if (FUNDEF_ISLACFUN (fundef)) {
            node *fun;

            if (FUNDEF_USED (fundef) > 1) {
                node *module;

                module = TBmakeModule ("dummy", F_prog, NULL, NULL, NULL, NULL, NULL);
                module = DUPcheckAndDupSpecialFundef (module, fundef, ap_node);
                fun = MODULE_FUNS (module);
                MODULE_FUNS (module) = NULL;
                module = FREEdoFreeNode (module);
            } else {
                fun = fundef;
            }

            if (FUNDEF_ISDOFUN (fundef)) {
                node *new_fun;
                /*
                 * function is a loop function
                 */
                new_fun = DUPdoDupTree (fun);

                FUNDEF_ARGS (new_fun)
                  = AdjustSignatureToArgs (FUNDEF_ARGS (new_fun), args);

                /*
                 * die selbe Funktionalität wie TUPdoTypeUpgrade, aber:
                 * prüft bei N_ap auf rekursiven Aufruf und prüft auf passende Typen
                 * Typen passen: Ergebnis ist 'upgegradete' Funktion
                 * Typen passen nicht: Ergebnis ist NULL
                 */

                new_fun = TryToDoTypeUpgrade (new_fun);

                if (new_fun != NULL) {
                    /*
                     * it is possible to specialiaze loop function
                     */

                    NOTE (("SUCCESS"));
                    /*	  AppendFundef(fundef, new_fun);
                              fundef = new_fun;
                              tup_fdp_expr++;
                              fun = FREEdoFreeTree( fun);*/
                    if (fundef != fun) {
                        AppendFundef (fundef, fun);
                        FUNDEF_NEXT (fun) = NULL;
                        fundef = fun;
                    } else {
                    }

                    FUNDEF_ARGS (fundef)
                      = AdjustSignatureToArgs (FUNDEF_ARGS (fun), args);
                    fun = TryToDoTypeUpgrade (fun);
                    NOTE2 (
                      ("           *** loop-function specialized:", FUNDEF_NAME (fun)));

                    tup_fdp_expr++;

                } else {
                    /*
                     * nothing to do
                     */
                    NOTE (("NO SUCCESS"));
                }
            }
            if (FUNDEF_ISCONDFUN (fundef)) {

                if (fundef != fun) {
                    FUNDEF_ARGS (fun) = AdjustSignatureToArgs (FUNDEF_ARGS (fun), args);
                    AppendFundef (fundef, fun);
                    fundef = fun;
                    FUNDEF_NEXT (fun) = NULL;
                } else {
                    FUNDEF_ARGS (fundef)
                      = AdjustSignatureToArgs (FUNDEF_ARGS (fundef), args);
                }
                tup_fdp_expr++;

                NOTE2 (("           *** cond-function specialized:", FUNDEF_NAME (fun)));
            }
        } else {

            /*
             * function is no special function
             */

            /*
             * check specialization counter (ask oracle for specialization)
             * copy fundef
             * go to wrapper fundef
             * change signature of copied fundef according to given args
             * upgrade types in new function
             * change pointer to used function, append new function to linklist
             */

            fundef = SpecializationOracle (fundef, args);
        }
    }

    DBUG_RETURN (fundef);
}

/******************************************************************
 *
 *  function:
 *    node *GetBestFittingFun(node *fun1, node *fun2, node *args)
 *
 *  precondition:
 *    fun2 fits to args, this means the types in args (exprs-chain)
 *    are less or equal as the types in funs (fundef)

 *  description:
 *    fun1 and fun2 are two fundefs. It is checked which of both
 *    fits better to the arguments in exprs-chain args. At least fun2
 *    has to fit! Fit means: all types in args are less or equal
 *    as the corresponding type in fun1/fun2. If both fun1 and fun2
 *    fit, the best fitting fun is the one with the most special
 *    arguments. The result is a pointer to that function.
 *
 ******************************************************************/
static node *
GetBestFittingFun (node *fun1, node *fun2, node *args)
{
    node *result = fun2;
    node *arg1, *arg2, *args_tmp;
    ntype *t1, *t2;
    bool fits = TRUE;
    ct_res cmp;

    DBUG_ENTER ("GetBestFittingFun");

    arg1 = FUNDEF_ARGS (fun1);
    arg2 = FUNDEF_ARGS (fun2);
    args_tmp = args;

    if (GetSizeOfChain (args) == GetSizeOfChain (arg1)) {
        while (arg1 != NULL) {
            t1 = AVIS_TYPE (ARG_AVIS (arg1));
            t2 = AVIS_TYPE (ID_AVIS (EXPRS_EXPR (args_tmp)));
            cmp = TYcmpTypes (t1, t2);
            if ((cmp == TY_lt) || (cmp == TY_dis) || (cmp == TY_hcs)) {
                /*
                 * function signature is to special or does not fit
                 */
                fits = FALSE;
                break;
            }
            arg1 = ARG_NEXT (arg1);
            args_tmp = EXPRS_NEXT (args_tmp);
        }

        if (fits) {
            int t1_counter, t2_counter;

            /*
             * fun1 could be used as function call for args
             * fun2 would also fit (precondition)
             * now find best fitting function
             */
            t1_counter = t2_counter = 0;
            arg1 = FUNDEF_ARGS (fun1);
            while (arg1 != NULL) {
                t1 = AVIS_TYPE (ARG_AVIS (arg1));
                t2 = AVIS_TYPE (ARG_AVIS (arg2));

                switch (TYcmpTypes (t1, t2)) {
                case TY_eq:
                    break;
                case TY_lt:
                    t1_counter++;
                    break;
                case TY_gt:
                    t2_counter++;
                    break;
                case TY_dis:
                case TY_hcs:
                default:
                    DBUG_ASSERT ((FALSE), "This should not happen!");
                }

                arg1 = ARG_NEXT (arg1);
                arg2 = ARG_NEXT (arg2);
            }
            if (t1_counter > t2_counter) {

                result = fun1;
            }
        }
    }
    DBUG_RETURN (result);
}

/*********************************************************************
 *
 *  function:
 *    bool IsFittingSignature(node *fundef, node *wrapper)
 *
 *  description:
 *    If no type of the signature of fundef is a super type of the
 *    corresponding type of of the signature of wrapper and
 *    no types are uncomparable, than the result is TRUE.
 *
 ********************************************************************/
static bool
IsFittingSignature (node *fundef, node *wrapper)
{
    bool result = TRUE;
    node *wrapper_args, *fundef_args;

    DBUG_ENTER ("IsFittingSignature");

    wrapper_args = FUNDEF_ARGS (wrapper);
    fundef_args = FUNDEF_ARGS (fundef);

    while ((NULL != wrapper_args) && (NULL != fundef_args)) {
        switch (TYcmpTypes (AVIS_TYPE (ARG_AVIS (fundef_args)),
                            AVIS_TYPE (ARG_AVIS (wrapper_args)))) {

        case TY_lt:
        case TY_eq:
            break;
        case TY_hcs:
        case TY_gt:
            result = FALSE;
            break;
        default:
            DBUG_ASSERT ((FALSE), "Different basetypes on same position");
        }
        wrapper_args = ARG_NEXT (wrapper_args);
        fundef_args = ARG_NEXT (fundef_args);
    }
    if (fundef_args != wrapper_args) {
        result = FALSE;
    }

    DBUG_RETURN (result);
}

/***********************************************************************
 *
 *  function:
 *    node *GetWrapperFun(node *fundef)
 *
 *  description:
 *    If a wrapper function of fundef exist and this wrapper could
 *    include fundef, than this wrapper is returned.
 *    If more than one wrapper would fit, one of them (no special one)
 *    is returned.
 *
 **********************************************************************/
static node *
GetWrapperFun (node *fundef)
{
    node *wrapper, *list;
    DBUG_ENTER ("GetWrapperFun");

    if (FUNDEF_FUNGROUP (fundef) != NULL) {
        list = FUNGROUP_FUNLIST (FUNDEF_FUNGROUP (fundef));
    } else {
        list = NULL;
    }
    wrapper = NULL;

    while (list != NULL) {

        if ((FUNDEF_ISWRAPPERFUN (LINKLIST_LINK (list)))
            && (IsFittingSignature (fundef, LINKLIST_LINK (list)))) {
            /*
             * function is wrapper function and signature fits to arguments
             */
            wrapper = LINKLIST_LINK (list);
        }
        list = LINKLIST_NEXT (list);
    }

    DBUG_RETURN (wrapper);
}

/******************************************************************************
 *
 *  function:
 *    node *TyrToFindSpecializedFunction(node *fundef, node* args, info* arg_info)
 *
 *  description:
 *    If fundef is a wrapper function a static dispatch is tried. If fundef
 *    is a non-wrapper function, static dispatch is applied (if possible) to
 *    an existing fitting wrapper function.
 *    If all fails, the best fitting function from fungroup is returned.
 *    So at the end the best fitting existing function is returned!
 *
 *******************************************************************************/
static node *
TryToFindSpecializedFunction (node *fundef, node *args, info *arg_info)
{

    node *tmp, *result = NULL;
    node *funs;

    DBUG_ENTER ("TryToFindSpecializedFunction");

    if (FUNDEF_ISWRAPPERFUN (fundef)) {

        /*
         * fundef belongs to a wrapper function
         * try a static dispatch
         */

        result = TryStaticDispatch (fundef, args, arg_info);
    } else {

        tmp = GetWrapperFun (fundef);
        if (tmp != NULL) {
            /*
             * wrapper function found
             */
            result = TryStaticDispatch (tmp, args, arg_info);
        }
    }

    /*  if (result == NULL) {*/

    /*
     * Static dispatch was not successful or no wrapper functions could be found
     * try to find a more special function in FUNGROUP_FUNLIST
     */

    if (NULL != FUNDEF_FUNGROUP (fundef)) {
        funs = FUNGROUP_FUNLIST (FUNDEF_FUNGROUP (fundef));

        if (NULL == result) {
            INFO_TUP_BESTFITTINGFUN (arg_info) = fundef;
        } else {
            INFO_TUP_BESTFITTINGFUN (arg_info) = result;
        }

        while (funs != NULL) {
            INFO_TUP_BESTFITTINGFUN (arg_info)
              = GetBestFittingFun (LINKLIST_LINK (funs),
                                   INFO_TUP_BESTFITTINGFUN (arg_info), args);
            funs = LINKLIST_NEXT (funs);
        }
        result = INFO_TUP_BESTFITTINGFUN (arg_info);
        if (result != fundef) {
            if (FUNDEF_ISWRAPPERFUN (fundef)) {
                tup_wdp_expr++;
            } else {
                tup_fdp_expr++;
            }
            NOTE2 (("           *** specialization found:", FUNDEF_NAME (fundef)));
        }
    } else {
        result = fundef;
    }
    /*  }
      else {
        tup_fdp_expr++;
      }*/

    DBUG_RETURN (result);
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

/**********************************************************************
 *
 * function:
 *   node *TUPfundef(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse through fundefs. Sets INFO_TUP_FUNDEF pointer
 *
 *********************************************************************/
node *
TUPfundef (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("TUPfundef");

    INFO_TUP_FUNDEF (arg_info) = arg_node;

    if (NULL != FUNDEF_BODY (arg_node)) {

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

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

    INFO_TUP_ASSIGN (arg_info) = arg_node;

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

    DBUG_ENTER ("TUPlet");

    INFO_TUP_TYPE (arg_info) = NULL;

    /*
     * traverse in RHS-expression
     */
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (INFO_TUP_CORRECTFUNCTION (arg_info)) {
        /*
         * no typecheck errors while traversal
         */

        /*
         * insert result types in ids-chain
         */
        INFO_TUP_TYPECOUNTER (arg_info) = 0;
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
        INFO_TUP_TYPE (arg_info) = NULL;
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

    /*
     * get result type of withloop
     */

    if (NULL != WITH_WITHOP (arg_node)) {

        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
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
    node *idxs;
    int num_ids;
    ntype *idx;
    DBUG_ENTER ("TUPpart");

    /*
     * partition of with loop
     * traverse in generator, update types
     * traverse in next partition
     */
    /*
     * First, we check whether we can extract some shape info from the
     * generator variable, i.e, we check whether we do have scalar indices:
     */
    idxs = WITHID_IDS (PART_WITHID (arg_node));

    if (idxs != NULL) {

        num_ids = TCcountIds (idxs);
        idx = TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, num_ids));
    } else {

        idx = TYmakeAKD (TYmakeSimpleType (T_int), 1, SHcreateShape (0));
    }

    INFO_TUP_WITHINDEX (arg_info) = idx;
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

    ntype *current, *tmp, *lb, *ub, *st, *wi;
    node *withid;

    DBUG_ENTER ("TUPgenerator");

    lb = NTCnewTypeCheck_Expr (GENERATOR_BOUND1 (arg_node));
    ub = NTCnewTypeCheck_Expr (GENERATOR_BOUND2 (arg_node));
    current = GetLowestType (TYcopyType (lb), TYcopyType (ub));

    if (NULL != GENERATOR_STEP (arg_node)) {

        st = NTCnewTypeCheck_Expr (GENERATOR_STEP (arg_node));
        current = GetLowestType (current, TYcopyType (st));
    } else {
        st = NULL;
    }

    if (NULL != GENERATOR_WIDTH (arg_node)) {

        wi = NTCnewTypeCheck_Expr (GENERATOR_WIDTH (arg_node));
        current = GetLowestType (current, TYcopyType (wi));
    } else {
        wi = NULL;
    }

    if (TYisAKV (current)) {
        tmp = TYmakeAKS (TYcopyType (TYgetScalar (current)),
                         SHcopyShape (TYgetShape (current)));
        current = TYfreeType (current);
        current = tmp;
    }

    /*
     * current is the best possible type for all generator values
     * assign current to all generator values iff the updated
     * value is no argument of a special function (funcond, dofun)
     */

    if (IsArgumentOfSpecialFunction (GENERATOR_BOUND1 (arg_node)) == FALSE) {
        if (TYisAKV (lb)) {
            GENERATOR_BOUND1 (arg_node)
              = AssignTypeToExpr (GENERATOR_BOUND1 (arg_node), lb);
        } else {
            GENERATOR_BOUND1 (arg_node)
              = AssignTypeToExpr (GENERATOR_BOUND1 (arg_node), current);
        }
    }

    if (IsArgumentOfSpecialFunction (GENERATOR_BOUND2 (arg_node)) == FALSE) {
        if (TYisAKV (ub)) {
            GENERATOR_BOUND2 (arg_node)
              = AssignTypeToExpr (GENERATOR_BOUND2 (arg_node), ub);
        } else {
            GENERATOR_BOUND2 (arg_node)
              = AssignTypeToExpr (GENERATOR_BOUND2 (arg_node), current);
        }
    }

    if (NULL != GENERATOR_STEP (arg_node)) {

        if (IsArgumentOfSpecialFunction (GENERATOR_STEP (arg_node)) == FALSE) {
            if (TYisAKV (st)) {
                GENERATOR_STEP (arg_node)
                  = AssignTypeToExpr (GENERATOR_STEP (arg_node), st);
            } else {

                GENERATOR_STEP (arg_node)
                  = AssignTypeToExpr (GENERATOR_STEP (arg_node), current);
            }
        }
    }

    if (NULL != GENERATOR_WIDTH (arg_node)) {
        if (IsArgumentOfSpecialFunction (GENERATOR_WIDTH (arg_node)) == FALSE) {

            if (TYisAKV (wi)) {
                GENERATOR_WIDTH (arg_node)
                  = AssignTypeToExpr (GENERATOR_WIDTH (arg_node), wi);
            } else {
                GENERATOR_WIDTH (arg_node)
                  = AssignTypeToExpr (GENERATOR_WIDTH (arg_node), current);
            }
        }
    }

    /*
     * update withid with same type
     */

    withid = INFO_TUP_WITHID (arg_info);

    /*
     * WITHID_VEC should never become an AKV-type
     */
    if (!TYisAKV (current)) {
        switch (TYcmpTypes (current, AVIS_TYPE (IDS_AVIS (WITHID_VEC (withid))))) {
        case TY_eq:
            break;

        case TY_gt:
            DBUG_ASSERT ((FALSE), "Lost type information!");
            break;

        case TY_lt:
            AVIS_TYPE (IDS_AVIS (WITHID_VEC (withid)))
              = TYfreeType (AVIS_TYPE (IDS_AVIS (WITHID_VEC (withid))));
            AVIS_TYPE (IDS_AVIS (WITHID_VEC (withid))) = TYcopyType (current);
            tup_tu_expr++;
            break;

        default:
            DBUG_ASSERT ((FALSE), "unexpected result of type comparison!");
        }
    }

    current = TYfreeType (current);
    lb = TYfreeType (lb);
    ub = TYfreeType (ub);
    if (st != NULL) {
        st = TYfreeType (st);
    }
    if (wi != NULL) {
        wi = TYfreeType (wi);
    }

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
    ntype *tmp;
    DBUG_ENTER ("TUPcode");

    /*
     * this is the code block of a with loop
     * CBLOCK contains this code, CEXPRS conains the return value(s)
     * CEXPRS has to be an id-node, so no typeupgrade has to be done
     */

    if (CODE_CBLOCK (arg_node) != NULL) {

        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    }

    if (CODE_NEXT (arg_node) != NULL) {

        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    /*
     * WLEXPRS needed later for computation of final type of withloop
     */
    tmp = NTCnewTypeCheck_Expr (CODE_CEXPRS (arg_node));

    INFO_TUP_WLEXPRS (arg_info) = tmp;

    DBUG_RETURN (arg_node);
}

/**************************************************************************
 *
 * function:
 *   node *TUPap(node *arg_node, info *arg_info)
 *
 *   description:
 *
 ***************************************************************************/
node *
TUPap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TUPap");

    if (INFO_TUP_CHECKLOOPFUN (arg_info)) {
        if (ILIBstringCompare (FUNDEF_NAME (INFO_TUP_FUNDEF (arg_info)),
                               FUNDEF_NAME (AP_FUNDEF (arg_node)))) {
            /*
             * at the moment we are checking, if the recursive call of a loop function
             * will work with specialized signature
             */

            node *signature, *args;

            signature = FUNDEF_ARGS (AP_FUNDEF (arg_node));
            args = AP_ARGS (arg_node);

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
        }
    } else {

        node *result = NULL;
        /*
         * first of all, try to find an already existing
         * specialized function
         */
        result = AP_FUNDEF (arg_node);

        /*    if ((FUNDEF_ISDOFUN( INFO_TUP_FUNDEF(arg_info))) &&
                (FUNDEF_ISCONDFUN( result)) &&
                (ASSIGN_NEXT(INFO_TUP_ASSIGN(arg_info)) != NULL) &&
                (N_return == NODE_TYPE( ASSIGN_INSTR(
           ASSIGN_NEXT(INFO_TUP_ASSIGN(arg_info))))) ) {
            }
            else

              {
        */
        if ((!FUNDEF_ISDOFUN (result))
            || (!ILIBstringCompare (FUNDEF_NAME (INFO_TUP_FUNDEF (arg_info)),
                                    FUNDEF_NAME (result)))) {
            NOTE2 (("       -> checking function call %s", FUNDEF_NAME (result)));

            result = TryToFindSpecializedFunction (AP_FUNDEF (arg_node),
                                                   AP_ARGS (arg_node), arg_info);

            if (result == AP_FUNDEF (arg_node)) {
                /*
                 * no better fitting specialized functions found
                 * try to specialize current function
                 */
                result
                  = TryToSpecializeFunction (AP_FUNDEF (arg_node), arg_node, arg_info);
            }

            if (result != AP_FUNDEF (arg_node)) {
                /*
                 * more special function exists
                 * use more special function
                 */
                AP_FUNDEF (arg_node) = result;
            }
        }
    }
    /*  }*/

    if (INFO_TUP_CORRECTFUNCTION (arg_info)) {

        INFO_TUP_TYPE (arg_info)
          = TUmakeProductTypeFromRets (FUNDEF_RETS (AP_FUNDEF (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TUPids(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
TUPids (node *arg_node, info *arg_info)
{
    ntype *type, *oldtype;

    DBUG_ENTER ("TUPids");

    if (INFO_TUP_TYPE (arg_info) != NULL) {

        /*
         * correct AVIS_SSAASSIGN pointer
         */
        /*    AVIS_SSAASSIGN( IDS_AVIS( arg_node)) = INFO_TUP_ASSIGN( arg_info);*/
        /*    if ( N_assign != NODE_TYPE( AVIS_SSAASSIGN( IDS_AVIS( arg_node)))) {*/

        DBUG_ASSERT ((N_assign == NODE_TYPE (AVIS_SSAASSIGN (IDS_AVIS (arg_node)))),
                     "wrong backref...");
        /* }*/

        oldtype = AVIS_TYPE (IDS_AVIS (arg_node));

        if (!TYisProd (INFO_TUP_TYPE (arg_info))) {

            DBUG_ASSERT ((TYisProd (INFO_TUP_TYPE (arg_info))), "Product type expected!");
        }

        type = TYgetProductMember (INFO_TUP_TYPE (arg_info),
                                   INFO_TUP_TYPECOUNTER (arg_info));

        DBUG_ASSERT ((TYisArray (type)), "array type expected");

        switch (TYcmpTypes (type, oldtype)) {

        case TY_eq: /* types are equal, nothing to do */
            break;

        case TY_gt: /* old type is more specific, lost type information */
            DBUG_ASSERT ((FALSE), "Lost type information!");
            break;

        case TY_hcs:
        case TY_dis: /* both types are unrelated, should not be possible */
            DBUG_ASSERT ((FALSE), "former type is unrelated to new type! ");
            break;

        case TY_lt: /* new type is more specific, do upgrade */
            AVIS_TYPE (IDS_AVIS (arg_node))
              = TYfreeType (AVIS_TYPE (IDS_AVIS (arg_node)));
            AVIS_TYPE (IDS_AVIS (arg_node)) = TYcopyType (type);
            tup_tu_expr++;
            break;

        default: /* all cases are listed above, so default should never be entered*/
            DBUG_ASSERT ((FALSE), "New element in enumeration added?");
        }

        INFO_TUP_TYPECOUNTER (arg_info) = INFO_TUP_TYPECOUNTER (arg_info) + 1;

        if (IDS_NEXT (arg_node) != NULL) {

            IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
        }

        DBUG_ASSERT ((N_assign == NODE_TYPE (AVIS_SSAASSIGN (IDS_AVIS (arg_node)))),
                     "wrong back-ref");
    }

    DBUG_RETURN (arg_node);
}

/**********************************************************************************
 *
 * function:
 *   node *TUPmodarray(node *arg_node, info *arg_info)
 *
 * description:
 *
 *********************************************************************************/
node *
TUPmodarray (node *arg_node, info *arg_info)
{
    ntype *idx, *array, *expr, *prod, *wlexprs, *result, *tmp;
    te_info *info;
    int count, i;
    DBUG_ENTER ("TUPmodarray");

    wlexprs = INFO_TUP_WLEXPRS (arg_info);
    count = TYgetProductSize (wlexprs);

    DBUG_ASSERT ((count > 0), "empty product type found");

    result = TYmakeEmptyProductType (count);

    for (i = 0; i < count; i++) {

        idx = TYcopyType (INFO_TUP_WITHINDEX (arg_info));
        array = NTCnewTypeCheck_Expr (MODARRAY_ARRAY (arg_node));
        expr = TYcopyType (TYgetProductMember (wlexprs, i));

        if (!TYisArray (expr)) {

            DBUG_ASSERT ((TYisArray (expr)), "array type expected");
        }

        info
          = TEmakeInfo (global.linenum, "with", "", "modarray", NULL, NULL, NULL, NULL);
        prod = TYmakeProductType (3, idx, array, expr);

        tmp = NTCCTwl_mod (info, prod);
        result = TYsetProductMember (result, i, TYcopyType (TYgetProductMember (tmp, 0)));

        prod = TYfreeType (prod);
    }

    INFO_TUP_TYPE (arg_info) = result;
    INFO_TUP_WLEXPRS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/*********************************************************************************
 *
 * function:
 *   node *TUPgenarray(node *arg_node, info *arg_info)
 *
 * description:
 *
 *********************************************************************************/
node *
TUPgenarray (node *arg_node, info *arg_info)
{
    ntype *idx, *shp, *expr, *dexpr, *prod, *wlexprs, *result, *tmp;
    te_info *info;
    int count, i;
    DBUG_ENTER ("TUPgenarray");

    wlexprs = INFO_TUP_WLEXPRS (arg_info);
    count = TYgetProductSize (wlexprs);

    DBUG_ASSERT ((count > 0), "empty product type found");

    result = TYmakeEmptyProductType (count);

    for (i = 0; i < count; i++) {

        idx = TYcopyType ((INFO_TUP_WITHINDEX (arg_info)));
        shp = NTCnewTypeCheck_Expr (GENARRAY_SHAPE (arg_node));
        expr = TYcopyType (TYgetProductMember (wlexprs, i));
        dexpr = NTCnewTypeCheck_Expr (GENARRAY_DEFAULT (arg_node));

        prod = TYmakeProductType (4, idx, shp, expr, dexpr);
        info
          = TEmakeInfo (global.linenum, "with", "", "genarray", NULL, NULL, NULL, NULL);

        tmp = NTCCTwl_gen (info, prod);
        result = TYsetProductMember (result, i, TYcopyType (TYgetProductMember (tmp, 0)));

        prod = TYfreeType (prod);
    }

    INFO_TUP_TYPE (arg_info) = result;
    INFO_TUP_WLEXPRS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/*********************************************************************************
 *
 * function:
 *   node *TUPfold(node *arg_node, info *arg_info)
 *
 * description:
 *
 *********************************************************************************/
node *
TUPfold (node *arg_node, info *arg_info)
{
    ntype *neutr, *expr, *prod, *wlexprs, *result, *tmp;
    te_info *info;
    int count, i;
    DBUG_ENTER ("TUPfold");

    wlexprs = INFO_TUP_WLEXPRS (arg_info);
    count = TYgetProductSize (wlexprs);

    DBUG_ASSERT ((count > 0), "empty product type found");

    result = TYmakeEmptyProductType (count);

    for (i = 0; i < count; i++) {

        neutr = NTCnewTypeCheck_Expr (FOLD_NEUTRAL (arg_node));
        expr = TYcopyType (TYgetProductMember (wlexprs, i));

        DBUG_ASSERT ((TYisArray (neutr)), "non array node!");
        DBUG_ASSERT ((TYisArray (expr)), "non array node!");

        prod = TYmakeProductType (2, neutr, expr);
        info = TEmakeInfo (global.linenum, "with", "", "fold", NULL, NULL, NULL, NULL);

        tmp = NTCCTwl_fold (info, prod);
        result = TYsetProductMember (result, i, TYcopyType (TYgetProductMember (tmp, 0)));

        prod = TYfreeType (prod);
    }

    INFO_TUP_TYPE (arg_info) = result;
    INFO_TUP_WLEXPRS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/*********************************************************************************
 *
 * function:
 *   node *TUPexprs(node *arg_node, info *arg_info)
 *
 * description:
 *
 *********************************************************************************/
node *
TUPexprs (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TUPexprs");

    INFO_TUP_TYPE (arg_info) = NTCnewTypeCheck_Expr (arg_node);

    DBUG_RETURN (arg_node);
}

/*********************************************************************************
 *
 * function:
 *   node *TUPid(node *arg_node, info *arg_info)
 *
 * description:
 *   The type of the id is transformed to the correct form
 *   and referenced by INFO_TUP_TYPE
 *
 *********************************************************************************/
node *
TUPid (node *arg_node, info *arg_info)
{
    ntype *type;
    DBUG_ENTER ("TUPid");

    type = AVIS_TYPE (ID_AVIS (arg_node));

    DBUG_ASSERT ((type != NULL), "Missing type information");

    if (!(TYisProd (type))) {
        DBUG_ASSERT ((TYisArray (type)), "non array node!");

        INFO_TUP_TYPE (arg_info) = TYmakeProductType (1, TYcopyType (type));
    } else {

        INFO_TUP_TYPE (arg_info) = TYcopyType (type);
    }

    DBUG_RETURN (arg_node);
}

/*********************************************************************************
 *
 * function:
 *   node *TUParray(node *arg_node, info *arg_info)
 *
 * description:
 *   Type inference for array nodes. Code is reused from typechecker.
 *
 *********************************************************************************/
node *
TUParray (node *arg_node, info *arg_info)
{
    ntype *elems, *type;
    int num_elems;
    te_info *info;

    DBUG_ENTER ("TUParray");

    if (NULL != ARRAY_AELEMS (arg_node)) {

        ARRAY_AELEMS (arg_node) = TRAVdo (ARRAY_AELEMS (arg_node), arg_info);
    } else {

        DBUG_ASSERT ((FALSE), "array without elements");
        INFO_TUP_TYPE (arg_info) = TYmakeProductType (0);
    }

    DBUG_ASSERT (TYisProd (INFO_TUP_TYPE (arg_info)),
                 "NTCexprs did not create a product type");

    elems = INFO_TUP_TYPE (arg_info);
    INFO_TUP_TYPE (arg_info) = NULL;

    /*
     * Now, we built the resulting (AKS-)type type from the product type found:
     */
    num_elems = TYgetProductSize (elems);
    if (num_elems > 0) {

        info = TEmakeInfo (global.linenum, "prf", "", "array-constructor", NULL, NULL,
                           NULL, NULL);
        type = NTCCTprf_array (info, elems);

    } else {
        /**
         * we are dealing with an empty array here!
         * To get started, we assume all empty arrays to be of type int[0].
         * If an other type is desired, it has to be casted to that type
         * (which - at the time being - is not yet supported 8-)
         */
        type
          = TYmakeProductType (1, TYmakeAKV (TYmakeSimpleType (T_int),
                                             COmakeConstant (T_int, SHcreateShape (1, 0),
                                                             NULL)));
    }

    INFO_TUP_TYPE (arg_info) = type;

    DBUG_RETURN (arg_node);
}

/*********************************************************************************
 *
 * function:
 *   node *TUPprf(node *arg_node, info *arg_info)
 *
 * description:
 *   Type inference for prf node. Code is reused from typechecker.
 *
 *********************************************************************************/
node *
TUPprf (node *arg_node, info *arg_info)
{
    ntype *args, *res;
    prf prf;
    te_info *info;

    DBUG_ENTER ("TUPprf");

    if (IsSupportedPrf (arg_node, arg_info)) {
        prf = PRF_PRF (arg_node);

        /*
         * First we collect the argument types. NTCexprs puts them into a product type
         * which is expected in INFO_NTC_TYPE( arg_info) afterwards!
         * INFO_NTC_NUM_EXPRS_SOFAR is used to count the number of exprs "on the fly"!
         */

        if (NULL != PRF_ARGS (arg_node)) {

            PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
        } else {

            DBUG_ASSERT ((FALSE), "primitive function without arguments");
            INFO_TUP_TYPE (arg_info) = TYmakeProductType (0);
        }

        DBUG_ASSERT (TYisProd (INFO_TUP_TYPE (arg_info)),
                     "TUPexprs did not create a product type");

        args = INFO_TUP_TYPE (arg_info);
        INFO_TUP_TYPE (arg_info) = NULL;

        info = TEmakeInfo (global.linenum, "prf", "", global.prf_string[prf], NULL, NULL,
                           global.ntc_cffuntab[prf], NULL);
        res = NTCCTcomputeType (global.ntc_funtab[prf], info, args);

        TYfreeType (args);
        INFO_TUP_TYPE (arg_info) = res;
    } else {

        INFO_TUP_TYPE (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/*********************************************************************************
 *
 * function:
 *   node *TUPfuncond(node *arg_node, info *arg_info)
 *
 * description:
 *
 *********************************************************************************/
node *
TUPfuncond (node *arg_node, info *arg_info)
{
    ntype *then_type, *else_type, *tmp;

    DBUG_ENTER ("TUPfuncond");

    then_type = TYcopyType (AVIS_TYPE (ID_AVIS (FUNCOND_THEN (arg_node))));
    else_type = TYcopyType (AVIS_TYPE (ID_AVIS (FUNCOND_ELSE (arg_node))));
    tmp = NULL;

    switch (TYcmpTypes (then_type, else_type)) {
    case TY_eq:
    case TY_lt:
        tmp = TYcopyType (else_type);
        break;
    case TY_gt:
        tmp = TYcopyType (then_type);
        break;
    case TY_hcs:
        /*
         * TODO: find smallest common supertype
         */
        DBUG_ASSERT ((FALSE), "Ooops!! Not implemented now! :-(");
        break;
    default:
        DBUG_ASSERT ((FALSE), "No common supertype could be infered!");
    }

    INFO_TUP_TYPE (arg_info) = TYmakeProductType (1, tmp);

    DBUG_RETURN (arg_node);
}

/****************************************************************************************
 *
 *
 ***************************************************************************************/
node *
TUPcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TUPcond");

    INFO_TUP_THENTYPEERROR (arg_info) = FALSE;
    INFO_TUP_ELSETYPEERROR (arg_info) = FALSE;
    INFO_TUP_TYPEERROR (arg_info) = FALSE;

    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);

    INFO_TUP_THENTYPEERROR (arg_info) = INFO_TUP_TYPEERROR (arg_info);
    INFO_TUP_TYPEERROR (arg_info) = FALSE;

    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    INFO_TUP_ELSETYPEERROR (arg_info) = INFO_TUP_TYPEERROR (arg_info);

    DBUG_RETURN (arg_node);
}

/*********************************************************************************
 *
 * function:
 *   node *TUP<basic>(node *arg_node, info *arg_info)
 *
 * description:
 *   Type inference for basic SaC-Types. Code is reused from typechecker.
 *
 *********************************************************************************/
#define TUPBASIC(name, base)                                                             \
    node *TUP##name (node *arg_node, info *arg_info)                                     \
    {                                                                                    \
        constant *cv;                                                                    \
        DBUG_ENTER ("TUP" #name);                                                        \
                                                                                         \
        cv = COaST2Constant (arg_node);                                                  \
        if (cv == NULL) {                                                                \
            INFO_TUP_TYPE (arg_info)                                                     \
              = TYmakeAKS (TYmakeSimpleType (base), SHcreateShape (0));                  \
        } else {                                                                         \
            INFO_TUP_TYPE (arg_info) = TYmakeAKV (TYmakeSimpleType (base), cv);          \
        }                                                                                \
        INFO_TUP_TYPE (arg_info) = TYmakeProductType (1, INFO_TUP_TYPE (arg_info));      \
        DBUG_RETURN (arg_node);                                                          \
    }

TUPBASIC (num, T_int)
TUPBASIC (double, T_double)
TUPBASIC (float, T_float)
TUPBASIC (char, T_char)
TUPBASIC (bool, T_bool)
