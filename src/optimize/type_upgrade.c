/* *
 * $Log$
 * Revision 1.33  2005/05/26 16:09:25  mwe
 * convert akv to aks types in TUPcode now
 *
 * Revision 1.32  2005/05/12 13:07:24  mwe
 * again some debugging, signature specialization of do-loops changed
 *
 * Revision 1.31  2005/05/11 13:25:00  mwe
 * some debugging
 *
 * Revision 1.30  2005/04/15 09:33:38  ktr
 * with-loop specific information is now stacked correctly.
 *
 * Revision 1.29  2005/03/10 13:38:22  mwe
 * bug fixed (duplication of loop funs)
 *
 * Revision 1.28  2005/03/04 21:21:42  cg
 * FUNDEF_USED counter etc removed.
 * Handling of FUNDEF_EXT_ASSIGNS drastically simplified.
 *
 * Revision 1.27  2005/02/20 19:43:42  mwe
 * trytospecialize changed
 *
 * Revision 1.26  2005/02/20 16:55:06  mwe
 * some assertions updated
 *
 * Revision 1.25  2005/02/18 22:19:02  mwe
 * bug fixed
 *
 * Revision 1.24  2005/02/16 14:10:19  mwe
 * some renaming done
 *
 * Revision 1.23  2005/02/14 15:51:48  mwe
 * bug fixed
 *
 * Revision 1.22  2005/02/11 12:10:42  mwe
 * names of flags changed
 *
 * Revision 1.21  2005/02/08 22:29:21  mwe
 * doxygen comments added
 *
 * Revision 1.20  2005/02/03 18:28:22  mwe
 * new counter and compiler flags added
 *
 * Revision 1.19  2005/02/02 18:10:57  mwe
 * reverse typeupgrade added
 *
 * Revision 1.18  2005/01/31 22:08:28  mwe
 * some changes done for withloops
 *
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
 * support for type_error (DONE) implement creation of fungroups (DONE) comment everything
 * in doxygen style (DONE) insert DBUG_PRINT's (DONE) CODE_CEXPRS could contain multiple
 * return values (DONE) use FUNDEF_COUNT in a correct way (DONE)
 */

/**<!--******************************************************************-->
 *
 * @file type_upgrade.c
 *
 * This file implements functionality of type upgrade (infer types of lhs
 * dependent on rhs), reverse type upgrade (infer types of rhs dependent
 * on lhs), function dispatch (removes calls of wrapper functions) and
 * function specialization (create more special function instances).
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
#include "ctinfo.h"

#include "type_upgrade.h"

/*
 * info structure
 */
struct INFO {
    node *fundef;
    node *withidvec;
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
#define INFO_TUP_WITHIDVEC(n) (n->withidvec)
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
    INFO_TUP_WITHIDVEC (result) = NULL;
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

/*<!--****************************************************************-->
 *
 * LOCAL HELPER FUNCTIONS:
 *
 ***********************************************************************/

/**<!--***********************************************************************-->
 *
 * @fn bool IsSupportedPrf(node *prf)
 *
 * @brief checks if primitive operation is supported by typechecker
 *
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
 * @param prf prf node which should be checked
 *
 * @return TRUE if the function is supported by typechecker, FALSE otherwise
 *
 *****************************************************************************/

static bool
IsSupportedPrf (node *prf)
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

/**<!--***********************************************************************-->
 *
 * @fn node *TryToDoTypeUpgrade( node* fundef)
 *
 * @brief internal used function to try signature update for do-funs
 *
 *   This function is similar to the global start function of typeupgrade.
 *   This function is used to check if signature updates of do-fun are
 *   possible, this means if all types of the recursive call fit to the
 *   updated signature.
 *   To achieve this, a traversal through the updated function is started,
 *   with flags for checking set. At the end of the traversal the flags are
 *   checked for problems while traversal. If the update caused problems
 *   the whole fundef is freed, otherwise fundef is returned.
 *
 * @param fundef contains a copy of a do-fun with updated signature to be checked
 *
 * @result the whole updated do-fun if recursive call fits, NULL otherwise
 *
 *****************************************************************************/

static node *
TryToDoTypeUpgrade (node *fundef)
{
    info *arg_info;

    DBUG_ENTER ("TryToDoTypeUpgrade");

    if (fundef != NULL) {

        DBUG_PRINT ("TUP", ("!!!!TryToDoTypeUpgrade!!! "));

        arg_info = MakeInfo ();
        INFO_TUP_CORRECTFUNCTION (arg_info) = TRUE;
        INFO_TUP_CHECKLOOPFUN (arg_info) = TRUE;

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

/**<!--*****************************************************************-->
 *
 * @fn ntype *GetLowestType(ntype *a, ntype *b)
 *
 * @brief This function returns a copy of the lowest type of 'a' and 'b'.
 *   !The arguments are released at the end of the function!
 *
 *   If a or b are product-types, they contain only one element. Because
 *   only this element is needed, the product-type is freed.
 *   The types have to be comparable!
 *
 * @param a type 1
 * @param b type 2
 *
 * @result least type of a and b
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

    DBUG_ASSERT (((TYcmpTypes (a, b) != TY_hcs) || (TYcmpTypes (a, b) != TY_dis)),
                 "Types are not compareable!");

    result = (TYleTypes (a, b)) ? TYcopyType (a) : TYcopyType (b);

    a = TYfreeType (a);
    b = TYfreeType (b);

    DBUG_RETURN (result);
}

/**<!--****************************************************************-->
 *
 * @fn int GetSizeOfChain(node *root)
 *
 * @brief returns the number of nodes of an arg/exprs-node chain
 *
 * @param root arg-node or exprs-node
 *
 * @result number of nodes in chain
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

/**<!--*****************************************************************-->
 *
 * @fn node *AssignTypeToExpr(node *expr, ntype* type)
 *
 * @brief Type information is appended at the corresponding
 *   avis-node of id-node.
 *
 *  This function has to be called only(!) from TUPgenerator.
 *
 * @param expr id-node to be updated, array-nodes are ignored
 * @param type type information to be appended
 *
 * @result updated id-node
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

        /*
         * check if type in id is less special as type to be assigned
         */
        if (TY_lt == TYcmpTypes (type, AVIS_TYPE (ID_AVIS (expr)))) {
            tup_rtu_expr++;
        }

        AVIS_TYPE (ID_AVIS (expr)) = TYfreeType (AVIS_TYPE (ID_AVIS (expr)));
        AVIS_TYPE (ID_AVIS (expr)) = TYcopyType (type);

    } else if (NODE_TYPE (expr) == N_array) {
        /*
         *nothing to do
         */
    } else {

        DBUG_ASSERT ((FALSE), "Unexpected node type found");
    }

    DBUG_RETURN (expr);
}

/**<!--****************************************************************-->
 *
 * @fn bool *IsArgumentOfSpecialFunction(node *arg, info *arg_info)
 *
 * @brief This function returns TRUE iff 'arg' is an argument of a
 *   special function (confun, dofun).
 *   While-funs are already changed to Do-funs.
 *
 * @param arg array-node or id-node
 * @param arg_info
 *
 * @result True if arg is an argument of a special function
 *
 *********************************************************************/
static bool
IsArgumentOfSpecialFunction (node *arg, info *arg_info)
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
            DBUG_ASSERT ((INFO_TUP_FUNDEF (arg_info) != NULL),
                         "No pointer to fundef found");

            if ((FUNDEF_ISCONDFUN (INFO_TUP_FUNDEF (arg_info)))
                || (FUNDEF_ISDOFUN (INFO_TUP_FUNDEF (arg_info)))) {
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

/**<!--*********************************************************************************-->
 *
 *  @fn node *AdjustSignatureToArgs(node *signature, node *args)
 *
 *  @brief All types of signature (arg_node chain) are freed and replaced by
 *  corresponding types of args (exprs_node chain).
 *
 * @param signature arg-node chain to be updated
 * @param args exprs-node chain with new types
 *
 * @result updated arg-node chain
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

/**<!--****************************************************************************-->
 *
 * @fn node *AppendFundef(node* fundef, node* result)
 *
 * @brief The fundef-node 'result' is appended at the end of the
 *   fundef-chain of fundef-node 'fundef'.
 *
 * @param fundef fundef chain
 * @param result fundef to be appended
 *
 * @result fundef chain
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

/**<!--******************************************************************-->
 *
 *  @fn node *TryStaticDispatch( node *fundef, node *args)
 *
 *  @brief If fundef belongs to a wrapper function a static dispatch is apllied
 *    if possible.
 *
 *   The code is reused from typechecker.
 *
 * @param fundef wrapper function
 * @param args arguments wrapper function is called with
 *
 * @result dispatched function if dispatch was possible, NULL otherwise
 *
 **************************************************************************/
static node *
TryStaticDispatch (node *fundef, node *args)
{
    ntype *prodarg;
    dft_res *dft_res;
    node *fundef_res = NULL;
    char *funname;
    DBUG_ENTER ("TryStaticDispatch");

    if ((FUNDEF_ISWRAPPERFUN (fundef)) && (NULL != args) && (global.optimize.dosfd)) {

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

/**<!--*************************************************************-->
 *
 *  @fn bool ArgumentIsSpecializeable(ntype *type)
 *
 *  @brief checks if type of argument could be upgraded with regard to
 *    sigspec_mode (least type for signature)
 *
 * @param type type to be checked
 *
 * @result TRUE if type could be upgraded, FALSE otherwise
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

/**<!--******************************************************************-->
 *
 *  @fn bool IsSpecializeable(node *fundef, node *args)
 *
 *  @brief This function checks if all formal conditions are satisfied for
 *    speciliazation of fundef.
 *
 *    This means: - speciliazation counter has not reached limit
 *                - at least one argument could be specialized
 *                - number of arguments in exprs- and arg-chain
 *                  are the same
 *    If this is fulfilled, this function returns TRUE,
 *    otherwise FALSE.
 *
 * @param fundef function to be checked
 * @param arguments fundef is called with
 *
 * @result TRUE if fundef could be specialized, FALSE otherwise
 *
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

/**<!--*********************************************************************-->
 *
 *  @fn node *GetBestPossibleType(ntype *type)
 *
 *  @brief transform the type in a less special type if necessary
 *     (with regards to sigspec_mode)
 *
 * @param type type to be checked
 *
 * @result type resulting from argument and current sigspec_mode
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
        if ((TYisAUD (type)) || (TYisAUDGZ (type)) || TYisAKD (type)) {

            result = TYcopyType (type);
        } else if ((TYisAKS (type)) || TYisAKV (type)) {
            result = TYmakeAKD (TYcopyType (TYgetScalar (type)), 0,
                                SHcopyShape (TYgetShape (type)));
        } else {
            DBUG_ASSERT ((FALSE), "unexpected type");
        }
        break;
    case SSP_aks:
        if (TYisAKV (type)) {

            result = TYmakeAKS (TYcopyType (TYgetScalar (type)),
                                SHcopyShape (TYgetShape (type)));
        } else {
            result = TYcopyType (type);
        }
        break;
    case SSP_akv:
        result = TYcopyType (type);
        break;
    default:
        DBUG_ASSERT ((FALSE),
                     "Unknown enumeration element! Incomplete switch statement!");
    }

    DBUG_RETURN (result);
}

/**<!--*********************************************************************-->
 *
 *  @fn node *SpecializationOracle(node *fundef, node *args)
 *
 *  @brief Duplicate and specialize fundef if possible.
 *     Specialized fundef is appended to AST
 *
 *   If all necessary prerequisites are fulfilled, fundef is duplicated.
 *   The signature is specialized and the Fungroup is updated.
 *
 *  @param fundef fundef to be specialized
 *  @param args arguments function is called with
 *
 *  @result if possible, specialized fundef, unchanged fundef otherwise
 *
 ***************************************************************************/
static node *
SpecializationOracle (node *fundef, node *args)
{
    node *result = fundef;
    node *signature;
    bool updated = FALSE;
    bool akv_in_signature = FALSE;
    ntype *new_type;
    DBUG_ENTER ("SpecializationOracle");

    if ((IsSpecializeable (fundef, args)) && (global.optimize.dofsp)) {

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
                     * new_type is not to special
                     */
                    AVIS_TYPE (ARG_AVIS (signature))
                      = TYfreeType (AVIS_TYPE (ARG_AVIS (signature)));
                    AVIS_TYPE (ARG_AVIS (signature)) = new_type;

                    updated = TRUE;
                    if (TYisAKV (new_type)) {
                        akv_in_signature = TRUE;
                    }
                }
            }
            signature = ARG_NEXT (signature);
            args = EXPRS_NEXT (args);
        }
    }

    if (updated) {
        DBUG_PRINT ("TUP", ("           *** function %s specialized via oracle:",
                            FUNDEF_NAME (fundef)));
        if (FUNDEF_ISWRAPPERFUN (fundef)) {
            tup_wdp_expr++;
        } else {
            tup_fsp_expr++;
        }

        if (akv_in_signature) {
            (FUNGROUP_AKVCOUNTER (FUNDEF_FUNGROUP (result)))++;
            FUNDEF_AKVID (result) = FUNGROUP_AKVCOUNTER (FUNDEF_FUNGROUP (result));
        }
    }

    DBUG_RETURN (result);
}

/**<!--**********************************************************************-->
 *
 * @fn node *TryToSpecializeFunction(node* fundef, node* args)
 *
 * @brief If possible fundef is specialized, according to its function type
 *   and the arguments it is called with.
 *
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
 *   Every other kind of functions: call SpecializationOracle
 *
 * @param fundef fundef to be specialized
 * @args ap_node ap-node, where function is called
 * @args arg_info
 *
 * @result if possible specialized fundef, otherwise unchanged fundef
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

        case TY_lt: /* found a supertype of function signature in 'args'*/
            is_more_special = FALSE;
            leave = TRUE;
            break;

        case TY_hcs:
        case TY_dis: /* arguments did not fit to signature, should never happen */
            CTIerrorLine (NODE_LINE (args),
                          "Argument of function %s did not fit to signature!",
                          FUNDEF_NAME (fundef));
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

            if (FUNDEF_ISDOFUN (fundef)) {
                node *new_fun;
                /*
                 * function is a loop function
                 */
                new_fun = DUPdoDupNode (fundef);

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
                     * it is possible to specialize loop function
                     */

                    DBUG_PRINT ("TUP", ("SUCCESS"));

                    new_fun = FREEdoFreeNode (new_fun);
                    FUNDEF_ARGS (fundef)
                      = AdjustSignatureToArgs (FUNDEF_ARGS (fundef), args);
                    fundef = TryToDoTypeUpgrade (fundef);
                    DBUG_PRINT ("TUP",
                                ("loop-function %s specialized:", FUNDEF_NAME (fundef)));

                    /*
                     * cg remark:
                     * Why not taking new_fun here?
                     * Who de-allocates new_fun as it is no longer used?
                     */
                    tup_fsp_expr++;

                } else {
                    /*
                     * nothing to do
                     */
                    DBUG_PRINT ("TUP", ("NO SUCCESS"));
                }
            }

            if (FUNDEF_ISCONDFUN (fundef)) {

                FUNDEF_ARGS (fundef) = AdjustSignatureToArgs (FUNDEF_ARGS (fundef), args);

                tup_fsp_expr++;

                DBUG_PRINT ("TUP",
                            ("cond-function %s specialized:", FUNDEF_NAME (fundef)));
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

/**<!--************************************************************-->
 *
 *  @fn node *GetBestFittingFun(node *fun1, node *fun2, node *args)
 *
 *  precondition:
 *    fun2 fits to args, this means the types in args (exprs-chain)
 *    are less or equal as the types in funs (fundef)
 *
 *  @brief Returns function which fits best to given arguments.
 *    Important: at least fun2 has to fit!
 *
 *    fun1 and fun2 are two fundefs. It is checked which of both
 *    fits better to the arguments in exprs-chain args. At least fun2
 *    has to fit! Fit means: all types in args are less or equal
 *    as the corresponding type in fun1/fun2. If both fun1 and fun2
 *    fit, the best fitting fun is the one with the most special
 *    arguments. The result is a pointer to that function.
 *
 * @param fun1 a fundef
 * @param funs a fundef fitting to args
 * @param args arguments
 *
 * @result best fitting fundef
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

/**<!--***************************************************************-->
 *
 *  @fn bool IsFittingSignature(node *fundef, node *wrapper)
 *
 *  @brief checks if wrapper could(!) be a wrapper contain fundef
 *
 *    If no type of the signature of fundef is a super type of the
 *    corresponding type of the signature of wrapper and
 *    no types are uncomparable, than the result is TRUE.
 *
 * @param fundef fundef to be checked
 * @param wrapper wrapper to be checked
 *
 * @result TRUE if wrapper could contain fundef, FALSE otherwise
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
            result = FALSE;
        }
        wrapper_args = ARG_NEXT (wrapper_args);
        fundef_args = ARG_NEXT (fundef_args);
    }
    if (fundef_args != wrapper_args) {
        result = FALSE;
    }

    DBUG_RETURN (result);
}

/**<!--*****************************************************************-->
 *
 *  @fn node *GetWrapperFun(node *fundef)
 *
 *  @brief If a wrapper function of fundef exist and this wrapper could
 *    contain fundef, than this wrapper is returned.
 *
 *    If more than one wrapper would fit, one of them (no special one)
 *    is returned.
 *
 * @param fundef fundef for which a wrapper should be found
 *
 * @result wrapper if existing
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

/**<!--***********************************************************************-->
 *
 * @fn bool ContainsAkvArgs(node *args)
 *
 * @brief arg-node chain, which is checked for akv-types
 *
 * @param args arg-chain
 *
 * @result TRUE is akv-types were found, FALSE otherwise
 *
 ****************************************************************************/
static bool
ContainsAkvArgs (node *args)
{
    bool result = FALSE;
    DBUG_ENTER ("ContainsAkvArgs");

    while (args != NULL) {
        if (TYisAKV (AVIS_TYPE (ARG_AVIS (args)))) {
            result = TRUE;
            break;
        }
        args = ARG_NEXT (args);
    }

    DBUG_RETURN (result);
}

/**<!--************************************************************************-->
 *
 *  @fn node *TyrToFindSpecializedFunction(node *fundef, node* args, info* arg_info)
 *
 *  @brief If a more special fundef exists it is returned
 *
 *    If fundef is a wrapper function a static dispatch is tried. If fundef
 *    is a non-wrapper function, static dispatch is applied (if possible) to
 *    an existing fitting wrapper function.
 *    If all fails, the best fitting function from fungroup is returned.
 *    So at the end the best fitting existing function is returned!
 *
 * @param fundef fundef a more special version is looked for
 * @param args arguments fundef is called with
 * @param arg_info
 *
 * @result most special existing fitting fundef
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

        result = TryStaticDispatch (fundef, args);
    } else {

        tmp = GetWrapperFun (fundef);
        if (tmp != NULL) {
            /*
             * wrapper function found
             */
            result = TryStaticDispatch (tmp, args);
        }
    }

    if ((result != NULL) && (result != fundef)) {
        tup_fdp_expr++;
    }
    /*
     * Static dispatch was not successful or no wrapper functions could be found
     * try to find a more special function in FUNGROUP_FUNLIST
     */

    if ((NULL != FUNDEF_FUNGROUP (fundef)) && (global.optimize.dofsp)) {
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
                tup_fsp_expr++;
            }
            if (ContainsAkvArgs (FUNDEF_ARGS (result))) {
                (FUNGROUP_AKVCOUNTER (FUNDEF_FUNGROUP (result)))++;
                FUNDEF_AKVID (result) = FUNGROUP_AKVCOUNTER (FUNDEF_FUNGROUP (result));
            }
            DBUG_PRINT ("TUP", ("specialization for %s found", FUNDEF_NAME (fundef)));
        }
    } else {
        result = fundef;
    }

    DBUG_RETURN (result);
}

/**<!--******************************************************************-->
 *
 * GLOBAL FUNCTIONS:
 *
 ************************************************************************/

/**<!--******************************************************************-->
 *
 * @fn node *TypeUpgrade(node* arg_node, node* arg_info)
 *
 * @brief This function is called from optimize.c, it is the starting point
 *    of TypeUpgrade. arg_node is expected to be a fundef node.
 *
 * @param fundef fundef node to be traversed
 *
 * @result traversed fundef
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

/**<!--****************************************************************-->
 *
 * @fn node *TUPfundef(node *arg_node, info *arg_info)
 *
 * @brief traverse through fundefs. Sets INFO_TUP_FUNDEF pointer
 *
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 *********************************************************************/
node *
TUPfundef (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("TUPfundef");
    DBUG_PRINT ("TUP", ("   --- Enter TUPfundef ---"));
    INFO_TUP_FUNDEF (arg_info) = arg_node;

    if (NULL != FUNDEF_BODY (arg_node)) {

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**<!--*****************************************************************-->
 *
 * @fn
 *   node *TUPblock(node* arg_node, node* arg_info)
 *
 * @brief traverse through block nodes
 *
 * @param arg_node
 * @param arg_info
 *
 * @result
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

/**<!--*****************************************************************-->
 *
 * @fn
 *   node *TUPassign(node* arg_node, node* arg_info)
 *
 * @brief Traverse through assign nodes. While top-down traversal the
 *   instructions are handled.
 *
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 *************************************************************************/
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

/**<!--****************************************************************-->
 *
 * @fn node *TUPreturn(node* arg_node, node* arg_info)
 *
 * @brief The type of the expressions in the return statement are compared to
 *   the return type of the function in ret-nodes. If necessary the return type of
 *   the function is updated.
 *
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 *************************************************************************/
node *
TUPreturn (node *arg_node, info *arg_info)
{

    node *ret, *exprs;
    ntype *best;
    DBUG_ENTER ("TUPreturn");

    DBUG_PRINT ("TUP", ("evaluationg return node"));

    if ((NULL != RETURN_EXPRS (arg_node))
        && (!FUNDEF_ISEXPORTED (INFO_TUP_FUNDEF (arg_info)))
        && (!FUNDEF_ISPROVIDED (INFO_TUP_FUNDEF (arg_info)))
        && (INFO_TUP_CORRECTFUNCTION (arg_info))) {

        /* convert ntypes of return values to return type of function add them to
         * ret-node*/

        ret = FUNDEF_RETS (INFO_TUP_FUNDEF (arg_info));
        exprs = RETURN_EXPRS (arg_node);

        while (ret != NULL) {

            best = GetBestPossibleType (AVIS_TYPE (ID_AVIS (EXPRS_EXPR (exprs))));

            switch (TYcmpTypes (best, RET_TYPE (ret))) {

            case TY_eq: /* same types, nothing changed */
                break;

            case TY_gt: /* lost type information, should only happen, if
                         * we are dealing with dead functions */
                /*DBUG_ASSERT( (FALSE), "lost type information");*/

                break;

            case TY_hcs:
            case TY_dis: /* both types are unrelated, should not be possible */
                CTIerrorLine (NODE_LINE (arg_node), "Former return type is unrelated to "
                                                    "new infered return type! ");
                break;

            case TY_lt: /* new type is more special: update */
                RET_TYPE (ret) = TYfreeType (RET_TYPE (ret));
                RET_TYPE (ret) = TYcopyType (best);
                break;

            default: /* no other cases exist */
                DBUG_ASSERT ((FALSE), "New element in enumeration added?");
            }
            best = TYfreeType (best);
            ret = RET_NEXT (ret);
            exprs = EXPRS_NEXT (exprs);
        }
    }

    DBUG_RETURN (arg_node);
}

/**<!--******************************************************************-->
 *
 * @fn node *TUPlet(node* arg_node, node* arg_info)
 *
 * @brief Traversal in expression to infer type, then traversal in ids
 *   to update type. Afterwards try reverse type upgrade.
 *
 * @param arg_node
 * @param arg_info
 *
 * @result
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

        /*
         * INFO_TUP_TYPE contains now the type of the lhs of current expression
         * check now if reverse type-upgrade is possible
         */

        if ((INFO_TUP_TYPE (arg_info) != NULL)
            && (NODE_TYPE (LET_EXPR (arg_node)) == N_id) && (global.optimize.dortup)) {

            /*
             * rhs is a single identifier
             * now check if it is an argument (no reverse upgrade)
             * or a local identifier (do reverse upgrade)
             */

            if (N_vardec == NODE_TYPE (AVIS_DECL (ID_AVIS (LET_EXPR (arg_node))))) {

                /*
                 * local identifier
                 */

                if (TYcmpTypes (INFO_TUP_TYPE (arg_info),
                                AVIS_TYPE (ID_AVIS (LET_EXPR (arg_node))))
                    == TY_lt) {

                    AVIS_TYPE (ID_AVIS (LET_EXPR (arg_node)))
                      = TYfreeType (AVIS_TYPE (ID_AVIS (LET_EXPR (arg_node))));
                    AVIS_TYPE (ID_AVIS (LET_EXPR (arg_node)))
                      = TYcopyType (INFO_TUP_TYPE (arg_info));

                    /*
                     * update counter
                     */
                    tup_rtu_expr++;
                }
            }
        }
        INFO_TUP_TYPE (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/**<!--**************************************************************************-->
 *
 * @fn node *TUPwith(node *arg_node, node* arg_info)
 *
 * @brief traverse in substructures
 *
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ********************************************************************************/
node *
TUPwith (node *arg_node, info *arg_info)
{
    node *oldwithid;
    node *oldwithidvec;
    ntype *oldwlexprs;

    DBUG_ENTER ("TUPwith");

    /*
     * Stack information about a surrounding withloop
     */
    oldwithid = INFO_TUP_WITHID (arg_info);
    oldwithidvec = INFO_TUP_WITHIDVEC (arg_info);
    oldwlexprs = INFO_TUP_WLEXPRS (arg_info);

    INFO_TUP_WITHID (arg_info) = NULL;
    INFO_TUP_WITHIDVEC (arg_info) = NULL;
    INFO_TUP_WLEXPRS (arg_info) = NULL;

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

    if (global.optimize.dotup) {
        if (NULL != WITH_WITHOP (arg_node)) {

            WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
        }
    }

    /*
     * restore information for traversal of surrounding withloop
     */
    INFO_TUP_WITHID (arg_info) = oldwithid;
    INFO_TUP_WITHIDVEC (arg_info) = oldwithidvec;
    INFO_TUP_WLEXPRS (arg_info) = oldwlexprs;

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn
 *   node *TUPpart(node *arg_node, node *arg_info)
 *
 * @brief traverse in substructures
 *
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 *****************************************************************************/
node *
TUPpart (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("TUPpart");

    if ((global.optimize.dotup) || (global.optimize.dortup)) {
        /*
         * partition of with loop
         * traverse in generator, update types
         * traverse in next partition
         */
        /*
         * First, we check whether we can extract some shape info from the
         * generator variable, i.e, we check whether we do have scalar indices:
         */

        INFO_TUP_WITHIDVEC (arg_info) = WITHID_VEC (PART_WITHID (arg_node));

        INFO_TUP_WITHID (arg_info) = PART_WITHID (arg_node);

        if (PART_GENERATOR (arg_node) != NULL) {

            PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);
        }

        if (PART_NEXT (arg_node) != NULL) {

            PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/**<!--********************************************************************-->
 *
 * @fn node *TUPgenerator(node *arg_node, node *arg_info)
 *
 * @brief upgrade types of generator elements if possible
 *
 *   All 'expressions' belonging to generator-node should have (if possible)
 *   the same type.
 *   So the best possible type is determined and all expressions are updated
 *   if possible.
 *
 * @param arg_node
 * @param arg_info
 *
 * @result
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

    if ((IsArgumentOfSpecialFunction (GENERATOR_BOUND1 (arg_node), arg_info) == FALSE)
        && (global.optimize.dortup)) {
        if (TYisAKV (lb)) {
            GENERATOR_BOUND1 (arg_node)
              = AssignTypeToExpr (GENERATOR_BOUND1 (arg_node), lb);
        } else {
            GENERATOR_BOUND1 (arg_node)
              = AssignTypeToExpr (GENERATOR_BOUND1 (arg_node), current);
        }
    }

    if ((IsArgumentOfSpecialFunction (GENERATOR_BOUND2 (arg_node), arg_info) == FALSE)
        && (global.optimize.dortup)) {
        if (TYisAKV (ub)) {
            GENERATOR_BOUND2 (arg_node)
              = AssignTypeToExpr (GENERATOR_BOUND2 (arg_node), ub);
        } else {
            GENERATOR_BOUND2 (arg_node)
              = AssignTypeToExpr (GENERATOR_BOUND2 (arg_node), current);
        }
    }

    if (NULL != GENERATOR_STEP (arg_node)) {

        if ((IsArgumentOfSpecialFunction (GENERATOR_STEP (arg_node), arg_info) == FALSE)
            && (global.optimize.dortup)) {
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
        if ((IsArgumentOfSpecialFunction (GENERATOR_WIDTH (arg_node), arg_info) == FALSE)
            && (global.optimize.dortup)) {

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
            /* Lost type information! */
            break;

        case TY_lt:
            AVIS_TYPE (IDS_AVIS (WITHID_VEC (withid)))
              = TYfreeType (AVIS_TYPE (IDS_AVIS (WITHID_VEC (withid))));
            AVIS_TYPE (IDS_AVIS (WITHID_VEC (withid))) = TYcopyType (current);
            /*
             * now it is necessary to check if WITHID_IDS contains already an ids-chain
             * if the chain is missing, create the needed number of ids-nodes
             */
            if (WITHID_IDS (withid) == NULL) {
                int dim;
                node *ids, *avis, *vardec;
                ids = NULL;
                dim = TYgetDim (current);
                while (dim > 0) {
                    avis = TBmakeAvis (ILIBtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                                                 SHcreateShape (0)));
                    ids = TBmakeIds (avis, ids);
                    vardec
                      = TBmakeVardec (avis, FUNDEF_VARDEC (INFO_TUP_FUNDEF (arg_info)));

                    FUNDEF_VARDEC (INFO_TUP_FUNDEF (arg_info)) = vardec;

                    AVIS_WITHID (avis) = withid;
                    AVIS_DECL (avis) = vardec;

                    dim--;
                }
                WITHID_IDS (withid) = ids;
            }
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

/**<!--*******************************************************************-->
 *
 * @fn node *TUPcode(node *arg_node, node *arg_info)
 *
 * @brief top-down: infer cexprs-type; bottom-up: try reverse type upgrade
 *
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ****************************************************************************/
node *
TUPcode (node *arg_node, info *arg_info)
{
    ntype *prod, *tmp, *tmp2;
    DBUG_ENTER ("TUPcode");

    /*
     * this is the code block of a with loop
     * CBLOCK contains this code, CEXPRS conains the return value(s)
     * CEXPRS has to be an id-node, so no typeupgrade has to be done
     */

    if (CODE_CBLOCK (arg_node) != NULL) {

        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    }

    prod = NTCnewTypeCheck_Expr (CODE_CEXPRS (arg_node));
    tmp = TYcopyType (TYgetProductMember (prod, 0));
    prod = TYfreeType (prod);

    if (TYisAKV (tmp)) {

        tmp2 = tmp;
        tmp = TYmakeAKS (TYcopyType (TYgetScalar (tmp)), SHcopyShape (TYgetShape (tmp)));
        tmp2 = TYfreeType (tmp2);
    }

    if (NULL == INFO_TUP_WLEXPRS (arg_info)) {

        INFO_TUP_WLEXPRS (arg_info) = tmp;
    } else {

        INFO_TUP_WLEXPRS (arg_info) = GetLowestType (tmp, INFO_TUP_WLEXPRS (arg_info));
    }

    if (CODE_NEXT (arg_node) != NULL) {

        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    /*
     * now: top-down traversal done, start bottom-up traversal
     * WLEXPRS needed later for computation of final type of withloop
     * reverse upgrade of types possible
     */

    if ((global.optimize.dortup)
        && (TYcmpTypes (INFO_TUP_WLEXPRS (arg_info),
                        AVIS_TYPE (ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (arg_node)))))
            == TY_lt)
        && (N_vardec
            == NODE_TYPE (AVIS_DECL (ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (arg_node))))))) {

        AVIS_TYPE (ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (arg_node))))
          = TYfreeType (AVIS_TYPE (ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (arg_node)))));
        AVIS_TYPE (ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (arg_node))))
          = TYcopyType (INFO_TUP_WLEXPRS (arg_info));

        tup_rtu_expr++;
    }

    DBUG_RETURN (arg_node);
}

/**<!--********************************************************************-->
 *
 * @fn node *TUPap(node *arg_node, info *arg_info)
 *
 * @brief try to specialize AP_FUNDEF
 *
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ***************************************************************************/
node *
TUPap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TUPap");

    if (INFO_TUP_CHECKLOOPFUN (arg_info)) {
        if (ASSIGN_RHS (FUNDEF_INT_ASSIGN (INFO_TUP_FUNDEF (arg_info))) == arg_node) {
            /*
             * at the moment we are checking, if the recursive call of a loop function
             * will work with specialized signature
             */

            node *signature, *args;
            ntype *tmp;

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
                            /*INFO_TUP_CORRECTFUNCTION( arg_info) = FALSE;
                              break;*/
                case TY_hcs:
                    /*INFO_TUP_CORRECTFUNCTION( arg_info) = FALSE;*/

                    /*
                     * find common supertype of signature and argument type
                     * change signature type to that argument type
                     * common supertype should not be worser than original type
                     */
                    tmp = AVIS_TYPE (ARG_AVIS (signature));

                    AVIS_TYPE (ARG_AVIS (signature))
                      = TYlubOfTypes (AVIS_TYPE (ARG_AVIS (signature)),
                                      AVIS_TYPE (ID_AVIS (EXPRS_EXPR (args))));

                    tmp = TYfreeType (tmp);

                    break;
                case TY_dis: /* both types are unrelated, should not be possible */
                    DBUG_ASSERT ((FALSE), "Former type is unrelated to new type! ");
                    INFO_TUP_CORRECTFUNCTION (arg_info) = FALSE;
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
         * first of all, try to find an already existin
         * specialized function
         */
        result = AP_FUNDEF (arg_node);

        if ((!FUNDEF_ISDOFUN (result))
            || (!ILIBstringCompare (FUNDEF_NAME (INFO_TUP_FUNDEF (arg_info)),
                                    FUNDEF_NAME (result)))) {
            DBUG_PRINT ("TUP", ("-> checking function call %s", FUNDEF_NAME (result)));

            result = TryToFindSpecializedFunction (AP_FUNDEF (arg_node),
                                                   AP_ARGS (arg_node), arg_info);

            if (result == AP_FUNDEF (arg_node)) {
                /*
                 * no better fitting specialized functions found
                 * try to specialize current function
                 */

                if ((!FUNDEF_ISPROVIDED (AP_FUNDEF (arg_node)))
                    && (!FUNDEF_ISEXPORTED (AP_FUNDEF (arg_node)))
                    && (FUNDEF_BODY (AP_FUNDEF (arg_node)) != NULL)) {
                    result = TryToSpecializeFunction (AP_FUNDEF (arg_node), arg_node,
                                                      arg_info);
                }
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
    if (INFO_TUP_CORRECTFUNCTION (arg_info)) {

        INFO_TUP_TYPE (arg_info)
          = TUmakeProductTypeFromRets (FUNDEF_RETS (AP_FUNDEF (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/**<!--************************************************************************-->
 *
 * @fn node *TUPids(node *arg_node, info *arg_info)
 *
 * @brief upgrades lhs with infered types from rhs
 *
 * @param arg_node
 * @param arg_info
 *
 * @result
 *******************************************************************************/
node *
TUPids (node *arg_node, info *arg_info)
{
    ntype *type, *oldtype;

    DBUG_ENTER ("TUPids");

    if ((INFO_TUP_TYPE (arg_info) != NULL) && (global.optimize.dotup)) {

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
            break;

        case TY_hcs:
        case TY_dis: /* both types are unrelated, should not be possible */
            CTIerrorLine (NODE_LINE (arg_node),
                          "Current type is unrelated to expression on right-hand side! ");
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
    }

    /*
     * now all ids elements are traversed in top-down order
     * bottom-up traversal: set INFO_TUP_TYPE to current node type for reverse type
     * upgarde
     */
    INFO_TUP_TYPE (arg_info) = AVIS_TYPE (IDS_AVIS (arg_node));

    DBUG_RETURN (arg_node);
}

/**<!--****************************************************************************-->
 *
 * @fn
 *   node *TUPmodarray(node *arg_node, info *arg_info)
 *
 * @brief infer type of modarray with-loop
 *
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 *********************************************************************************/
node *
TUPmodarray (node *arg_node, info *arg_info)
{
    ntype *idx, *array, *expr, *prod, *tmp;
    te_info *info;

    DBUG_ENTER ("TUPmodarray");

    expr = INFO_TUP_WLEXPRS (arg_info);

    idx = TYcopyType (AVIS_TYPE (IDS_AVIS (INFO_TUP_WITHIDVEC (arg_info))));

    array = NTCnewTypeCheck_Expr (MODARRAY_ARRAY (arg_node));

    if (!TYisArray (expr)) {

        DBUG_ASSERT ((TYisArray (expr)), "array type expected");
    }

    info = TEmakeInfo (global.linenum, "with", "", "modarray", NULL, NULL, NULL, NULL);
    prod = TYmakeProductType (3, idx, array, expr);

    tmp = NTCCTwl_mod (info, prod);

    prod = TYfreeType (prod);

    INFO_TUP_TYPE (arg_info) = tmp;
    INFO_TUP_WLEXPRS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/**<!--***************************************************************************-->
 *
 * @fn node *TUPgenarray(node *arg_node, info *arg_info)
 *
 * @brief infer type of genarray with-loop
 *
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ***********************************************************************************/
node *
TUPgenarray (node *arg_node, info *arg_info)
{
    ntype *idx, *shp, *expr, *dexpr, *prod, *tmp;
    te_info *info;

    DBUG_ENTER ("TUPgenarray");

    expr = INFO_TUP_WLEXPRS (arg_info);
    idx = TYcopyType (AVIS_TYPE (IDS_AVIS (INFO_TUP_WITHIDVEC (arg_info))));
    shp = NTCnewTypeCheck_Expr (GENARRAY_SHAPE (arg_node));
    if (GENARRAY_DEFAULT (arg_node) != NULL) {
        dexpr = NTCnewTypeCheck_Expr (GENARRAY_DEFAULT (arg_node));
    } else {
        dexpr = TYcopyType (expr);
    }

    prod = TYmakeProductType (4, idx, shp, expr, dexpr);
    info = TEmakeInfo (global.linenum, "with", "", "genarray", NULL, NULL, NULL, NULL);

    tmp = NTCCTwl_gen (info, prod);

    prod = TYfreeType (prod);

    INFO_TUP_TYPE (arg_info) = tmp;

    DBUG_RETURN (arg_node);
}

/**<!--***************************************************************************-->
 *
 * @fn node *TUPfold(node *arg_node, info *arg_info)
 *
 * @brief infer type of fold with-loop
 *
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 *********************************************************************************/
node *
TUPfold (node *arg_node, info *arg_info)
{
    ntype *neutr, *expr, *prod, *tmp;
    te_info *info;

    DBUG_ENTER ("TUPfold");

    expr = INFO_TUP_WLEXPRS (arg_info);
    neutr = NTCnewTypeCheck_Expr (FOLD_NEUTRAL (arg_node));

    DBUG_ASSERT ((TYisArray (neutr)), "non array node!");
    DBUG_ASSERT ((TYisArray (expr)), "non array node!");

    prod = TYmakeProductType (2, neutr, expr);
    info = TEmakeInfo (global.linenum, "with", "", "fold", NULL, NULL, NULL, NULL);

    tmp = NTCCTwl_fold (info, prod);

    prod = TYfreeType (prod);

    INFO_TUP_TYPE (arg_info) = tmp;

    DBUG_RETURN (arg_node);
}

/**<!--***************************************************************************-->
 *
 * @fn node *TUPexprs(node *arg_node, info *arg_info)
 *
 * @brief infer type of exprs
 *
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 *********************************************************************************/
node *
TUPexprs (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TUPexprs");

    if (global.optimize.dotup) {
        INFO_TUP_TYPE (arg_info) = NTCnewTypeCheck_Expr (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/**<!--*************************************************************************-->
 *
 * @fn node *TUPid(node *arg_node, info *arg_info)
 *
 * @brief The type of the id is transformed to product type
 *   and referenced by INFO_TUP_TYPE
 *
 * @param arg_node * @param arg_info
 *
 * @result
 *
 **********************************************************************************/
node *
TUPid (node *arg_node, info *arg_info)
{
    ntype *type;
    DBUG_ENTER ("TUPid");

    if (global.optimize.dotup) {

        type = AVIS_TYPE (ID_AVIS (arg_node));

        DBUG_ASSERT ((type != NULL), "Missing type information");

        if (!(TYisProd (type))) {
            DBUG_ASSERT ((TYisArray (type)), "non array node!");

            INFO_TUP_TYPE (arg_info) = TYmakeProductType (1, TYcopyType (type));
        } else {

            INFO_TUP_TYPE (arg_info) = TYcopyType (type);
        }
    }

    DBUG_RETURN (arg_node);
}

/**<!--*************************************************************************-->
 *
 * @fn node *TUParray(node *arg_node, info *arg_info)
 *
 * @brief Type inference for array nodes.
 *
 *   Code is reused from typechecker.
 *
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 *********************************************************************************/
node *
TUParray (node *arg_node, info *arg_info)
{
    ntype *elems, *type;
    int num_elems;
    te_info *info;

    DBUG_ENTER ("TUParray");

    if (global.optimize.dotup) {

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
            type = TYmakeProductType (1, TYmakeAKV (TYmakeSimpleType (T_int),
                                                    COmakeConstant (T_int,
                                                                    SHcreateShape (1, 0),
                                                                    NULL)));
        }

        INFO_TUP_TYPE (arg_info) = type;
    }

    DBUG_RETURN (arg_node);
}

/**<!--***************************************************************************-->
 *
 * @fn
 *   node *TUPprf(node *arg_node, info *arg_info)
 *
 * @brief Type inference for prf node.
 *
 *   Code is reused from typechecker.
 *
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 *********************************************************************************/
node *
TUPprf (node *arg_node, info *arg_info)
{
    ntype *args, *res;
    prf prf;
    te_info *info;

    DBUG_ENTER ("TUPprf");

    if ((IsSupportedPrf (arg_node)) && (global.optimize.dotup)) {
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

/**<!--***************************************************************************-->
 *
 * @fn
 *   node *TUPfuncond(node *arg_node, info *arg_info)
 *
 * @brief type inference for funcond-nodes
 *
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 *********************************************************************************/
node *
TUPfuncond (node *arg_node, info *arg_info)
{
    ntype *then_type, *else_type, *tmp;

    DBUG_ENTER ("TUPfuncond");

    if (global.optimize.dotup) {

        then_type = TYcopyType (AVIS_TYPE (ID_AVIS (FUNCOND_THEN (arg_node))));
        else_type = TYcopyType (AVIS_TYPE (ID_AVIS (FUNCOND_ELSE (arg_node))));

        tmp = TYlubOfTypes (then_type, else_type);

        if (tmp == NULL) {
            CTIerrorLine (NODE_LINE (arg_node), "No common super type for result of "
                                                "conditional could be infered!");
        }

        INFO_TUP_TYPE (arg_info) = TYmakeProductType (1, tmp);
    }

    DBUG_RETURN (arg_node);
}

/**<!--*****************************************************************************-->
 *
 * @fn node *TUPcond(node *arg_node, info *arg_info)
 *
 * @brief traverses in substructures
 *
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 *****************************************************************************************/
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

/**<!--***************************************************************************-->
 *
 * @fn node *TUP<basic>(node *arg_node, info *arg_info)
 *
 * @brief Type inference for basic SaC-Types (num, float, double, bool, char).
 *
 *    Code is reused from typechecker.
 *
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 *********************************************************************************/
#define TUPBASIC(name, base)                                                             \
    node *TUP##name (node *arg_node, info *arg_info)                                     \
    {                                                                                    \
        constant *cv;                                                                    \
        DBUG_ENTER ("TUP" #name);                                                        \
                                                                                         \
        if (global.optimize.dotup) {                                                     \
                                                                                         \
            cv = COaST2Constant (arg_node);                                              \
            if (cv == NULL) {                                                            \
                INFO_TUP_TYPE (arg_info)                                                 \
                  = TYmakeAKS (TYmakeSimpleType (base), SHcreateShape (0));              \
            } else {                                                                     \
                INFO_TUP_TYPE (arg_info) = TYmakeAKV (TYmakeSimpleType (base), cv);      \
            }                                                                            \
            INFO_TUP_TYPE (arg_info) = TYmakeProductType (1, INFO_TUP_TYPE (arg_info));  \
        }                                                                                \
        DBUG_RETURN (arg_node);                                                          \
    }

TUPBASIC (num, T_int)
TUPBASIC (double, T_double)
TUPBASIC (float, T_float)
TUPBASIC (char, T_char)
TUPBASIC (bool, T_bool)
