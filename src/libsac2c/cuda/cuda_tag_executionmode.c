/** <!--********************************************************************-->
 *
 * @file cuda_TAG_executionmode.c
 *
 * prefix: CUTEM
 *
 * description:
 *   Tags the assignments, whether their executionmode is CUDA_HOST_SINGLE,
 *   CUDA_DEVICE_SINGLE, or CUDA_DEVICE_MULTI
 *
 *****************************************************************************/

#include "cuda_tag_executionmode.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "str.h"
#include "memory.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "globals.h"
#include "type_utils.h"
#include "new_types.h"

static bool CHANGED = FALSE;
static int ITERATION = 1;

typedef enum { cutem_tag, cutem_untag, cutem_update } travmode_t;

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *lastassign;
    bool fromap;
    travmode_t travmode;
    bool inwith;
    bool incond;
    node *lhs;
    node *fundefargs;
    node *apargs;
    bool dofunargs :1;
    bool at_iscudarizable :1;
    int at_wlcount :1;
};

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LASTASSIGN(n) (n->lastassign)
#define INFO_FROMAP(n) (n->fromap)
#define INFO_TRAVMODE(n) (n->travmode)
#define INFO_INWITH(n) (n->inwith)
#define INFO_INCOND(n) (n->incond)
#define INFO_LHS(n) (n->lhs)
#define INFO_FUNDEFARGS(n) (n->fundefargs)
#define INFO_APARGS(n) (n->apargs)
#define INFO_DOFUNARGS(n) (n->dofunargs)
#define INFO_AT_ISCUDARIZABLE(n) (n->at_iscudarizable)
#define INFO_AT_WLCOUNT(n) (n->at_wlcount)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_LASTASSIGN (result) = NULL;
    INFO_FROMAP (result) = FALSE;
    INFO_TRAVMODE (result) = cutem_tag;
    INFO_INWITH (result) = FALSE;
    INFO_INCOND (result) = FALSE;
    INFO_LHS (result) = NULL;
    INFO_FUNDEFARGS (result) = NULL;
    INFO_APARGS (result) = NULL;
    INFO_DOFUNARGS (result) = FALSE;
    INFO_AT_ISCUDARIZABLE (result) = TRUE;
    INFO_AT_WLCOUNT (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

static bool
CheckApIds (node *ids)
{
    bool res = TRUE;
    ntype *type;

    DBUG_ENTER ();

    while (ids != NULL) {
        type = IDS_NTYPE (ids);
        res = res && (TUisScalar (type) || TYisAKS (type) || TYisAKD (type))
              && !AVIS_ISHOSTREFERENCED (IDS_AVIS (ids));
        ids = IDS_NEXT (ids);
    }

    DBUG_RETURN (res);
}

static bool
IsIdCudaDefined (node *id, info *arg_info)
{
    bool res = FALSE;
    node *ssa;
    // ntype *type;

    DBUG_ENTER ();

    /* Get the SSAASSIGN of this N_id */
    ssa = ID_SSAASSIGN (id);
    // type = ID_NTYPE( id);

    /* If this N_id is defined by an N_assign that needs to be
     * executed on CUDA, the N_assign referenced this N_id will
     * be tagged as CUDA_DEVICE_SINGLE. Note the N_id must not be
     * referenced in any CUDA_HOST_SINGLE N_assign. Otherwise the current
     * N_assign referencing this N_id is not executable on CUDA either. */
    if (ssa != NULL) {
        if (/* ( TUisScalar( type) || TYisAKS( type)) && */ /* Scalar or AKS */
            !AVIS_ISHOSTREFERENCED (ID_AVIS (id)) && /* NOT referenced in host N_assign */
            (ASSIGN_CUDAEXECMODE (ssa) == CUDA_DEVICE_SINGLE
             || /* Define by DEVICE_SINGLE or DEVICE_MULTI N_assign */
             ASSIGN_CUDAEXECMODE (ssa) == CUDA_DEVICE_MULTI)) {
            res = TRUE;
        }
    }
    /* If the id is passed as an argument and we are in a loop function,
     * we count that as cuda defined. This is because if we do that, the
     * assignment might be tagged as DEVICE_SINGLE, and we have the chance
     * to cudarize it and therefore further remove tranfers from for-loops.
     * Here is an example:
     *
     *   loop_function( ... A ...) {
     *     val = sel( A, 0);
     *     B = cuda_with {
     *           ... val ...
     *     } : genarray();
     *     ...
     *   }
     *
     * If we tag the selection as DEVICE_SINGLE, it can later be moved into
     * the withloop and the transfer associated with A can be lifted out.
     * Of couse, to be able to achieve this effect, A must not be referenced
     * in any HOST_SINGLE instructions in the loop function
     */
    else if (NODE_TYPE (ID_DECL (id)) == N_arg) {
        if (FUNDEF_ISLOOPFUN (INFO_FUNDEF (arg_info)) && !TUisScalar (ID_NTYPE (id))
            && !AVIS_ISHOSTREFERENCED (ID_AVIS (id))
            && FUNDEF_WLCOUNT (INFO_FUNDEF (arg_info)) != 0) {
            res = TRUE;
        }
    } else {
        DBUG_UNREACHABLE ("N_id's SSAASSIGN is NULL but it's not a function argument!");
    }

    DBUG_RETURN (res);
}

static bool
HasCudaDefinedId (node *ap_args, info *arg_info)
{
    bool res = FALSE;

    DBUG_ENTER ();

    while (ap_args != NULL) {
        if (IsIdCudaDefined (EXPRS_EXPR (ap_args), arg_info)) {
            res = TRUE;
            break;
        }
        ap_args = EXPRS_NEXT (ap_args);
    }

    DBUG_RETURN (res);
}

static node *
TraverseLacFun (node *fundef, node *ap, info *arg_info)
{
    node *old_fundef, *old_fundefargs, *old_apargs;
    bool old_fromap;

    DBUG_ENTER ();

    /* Push info */
    old_fundef = INFO_FUNDEF (arg_info);
    old_fromap = INFO_FROMAP (arg_info);
    old_fundefargs = INFO_FUNDEFARGS (arg_info);
    old_apargs = INFO_APARGS (arg_info);

    INFO_FUNDEF (arg_info) = fundef;
    INFO_FROMAP (arg_info) = TRUE;
    INFO_FUNDEFARGS (arg_info) = FUNDEF_ARGS (fundef);
    INFO_APARGS (arg_info) = AP_ARGS (ap);

    fundef = TRAVdo (fundef, arg_info);

    /* Pop info */
    INFO_FROMAP (arg_info) = old_fromap;
    INFO_FUNDEF (arg_info) = old_fundef;
    INFO_FUNDEFARGS (arg_info) = old_fundefargs;
    INFO_APARGS (arg_info) = old_apargs;

    DBUG_RETURN (fundef);
}

static node *
GetApArgFromFundefArg (node *apargs, node *fundefargs, node *arg)
{
    node *res = NULL;

    DBUG_ENTER ();

    DBUG_ASSERT (fundefargs != NULL, "Function argument list is NULL!");
    DBUG_ASSERT (apargs != NULL, "Application parameter list is NULL!");

    while (apargs != NULL) {
        if (fundefargs == arg) {
            res = EXPRS_EXPR (apargs);
            break;
        }
        apargs = EXPRS_NEXT (apargs);
        fundefargs = ARG_NEXT (fundefargs);
    }

    DBUG_ASSERT (res != NULL, "No matching application arg found for fundef arg!");

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *ATravFundefCheckCudarizable(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
static node *
ATravFundefCheckCudarizable (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ATravApCheckCudarizable(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
static node *
ATravApCheckCudarizable (node *arg_node, info *arg_info)
{
    node *fundef;

    DBUG_ENTER ();

    fundef = AP_FUNDEF (arg_node);

    if (fundef != NULL) {
        if (FUNDEF_ISCONDFUN (fundef)) {
            /* If we found a cond lac function, we continues traverse
             * into its body */
            fundef = TRAVdo (fundef, arg_info);
        } else if (FUNDEF_ISLOOPFUN (fundef)) {
            /* If we found a loop, the lac function is not cudarizable */
            INFO_AT_ISCUDARIZABLE (arg_info) = FALSE;
        } else {
            /* If we found a normal function application, lac
             * function is not cudarizable */
            INFO_AT_ISCUDARIZABLE (arg_info) = FALSE;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ATravWithCheckCudarizable(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
static node *
ATravWithCheckCudarizable (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* Any withloops within a lac function makes it un-cudarizable */
    INFO_AT_ISCUDARIZABLE (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ATravFundefWLCount(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
static node *
ATravFundefWLCount (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ATravWithWLCount(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
static node *
ATravWithWLCount (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_AT_WLCOUNT (arg_info)++;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUTEMdoTagExecutionmode(node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param
 *   @param
 *   @return
 *
 *****************************************************************************/
node *
CUTEMdoTagExecutionmode (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ();

    TRAVpush (TR_cutem);

    /* Fixed point iteration */
    do {
        CHANGED = FALSE;

        printf ("********** Starting iteration %d **********\n", ITERATION);

        arg_info = MakeInfo ();
        INFO_TRAVMODE (arg_info) = cutem_tag;
        syntax_tree = TRAVdo (syntax_tree, arg_info);
        arg_info = FreeInfo (arg_info);

        arg_info = MakeInfo ();
        INFO_TRAVMODE (arg_info) = cutem_untag;
        // if( ITERATION != 4)
        syntax_tree = TRAVdo (syntax_tree, arg_info);
        arg_info = FreeInfo (arg_info);

        ITERATION++;
    } while (/* ITERATION<=1 */ CHANGED);

    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUTEMfundef(node *arg_node, info)
 *
 *   @brief
 *
 *   @param
 *   @param
 *   @return
 *
 *****************************************************************************/
node *
CUTEMfundef (node *arg_node, info *arg_info)
{
    info *anon_info;

    DBUG_ENTER ();

    /************ Anonymous Traversal ************/
    anontrav_t atrav[3] = {{N_fundef, &ATravFundefWLCount},
                           {N_with, &ATravWithWLCount},
                           {(nodetype)0, NULL}};

    TRAVpushAnonymous (atrav, &TRAVsons);

    anon_info = MakeInfo ();

    arg_node = TRAVdo (arg_node, anon_info);
    /*********************************************/

    FUNDEF_WLCOUNT (arg_node) = INFO_AT_WLCOUNT (anon_info);

    /************ Anonymous Traversal ************/
    anon_info = FreeInfo (anon_info);
    TRAVpop ();
    /*********************************************/

    INFO_FUNDEF (arg_info) = arg_node;

    if (!FUNDEF_ISLACFUN (arg_node)) {
        FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    } else {
        if (INFO_FROMAP (arg_info)) {
            FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        } else {
            FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUTEMarg(node *arg_node, info)
 *
 *   @brief
 *
 *   @param
 *   @param
 *   @return
 *
 *****************************************************************************/
node *
CUTEMarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* We initilise IsHostReferenced in the first iteration of TAG traversal.
     * This is needed only once since once the flag has been changed to TRUE,
     * it will never be set back to FLASE again. */
    if (INFO_TRAVMODE (arg_info) == cutem_tag && ITERATION == 1) {
        AVIS_ISHOSTREFERENCED (ARG_AVIS (arg_node)) = FALSE;
    }

    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUTEMblock(node *arg_node, info)
 *
 *   @brief
 *
 *   @param
 *   @param
 *   @return
 *
 *****************************************************************************/
node *
CUTEMblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* Must traverse all vardecs before traversing the body */
    BLOCK_VARDECS (arg_node) = TRAVopt (BLOCK_VARDECS (arg_node), arg_info);
    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUTEMvardec(node *arg_node, info)
 *
 *   @brief
 *
 *   @param
 *   @param
 *   @return
 *
 *****************************************************************************/
node *
CUTEMvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* We initilise IsHostReferenced in the first iteration of TAG traversal.
     * This is needed only once since once the flag has been changed to TRUE,
     * it will never be set back to FLASE again. */
    if (INFO_TRAVMODE (arg_info) == cutem_tag && ITERATION == 1) {
        AVIS_ISHOSTREFERENCED (VARDEC_AVIS (arg_node)) = FALSE;
    }

    VARDEC_NEXT (arg_node) = TRAVopt (VARDEC_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUTEMassign(node *arg_node, info)
 *
 *   @brief
 *
 *   @param
 *   @param
 *   @return
 *
 *****************************************************************************/
node *
CUTEMassign (node *arg_node, info *arg_info)
{
    node *old_assign;
    cudaexecmode_t old_mode;

    DBUG_ENTER ();

    old_assign = INFO_LASTASSIGN (arg_info);
    INFO_LASTASSIGN (arg_info) = arg_node;

    if (INFO_TRAVMODE (arg_info) == cutem_tag) {
        /* Each N_assign is intially tagged to be CUDA_HOST_SINGLE */
        ASSIGN_CUDAEXECMODE (arg_node) = CUDA_HOST_SINGLE;
        ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

        /* If after tagging, the execution mode of this N_assign
         * remains to be CUDA_HOST_SINGLE (i.e. this N_assign must
         * be executed on the host), we update its RHS variables */
        if (ASSIGN_CUDAEXECMODE (arg_node) == CUDA_HOST_SINGLE) {
            INFO_TRAVMODE (arg_info) = cutem_update;
            ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
        }
        INFO_TRAVMODE (arg_info) = cutem_tag;
    } else if (INFO_TRAVMODE (arg_info) == cutem_untag) {
        old_mode = ASSIGN_CUDAEXECMODE (arg_node);
        ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

        /* If after traversing the RHS, we found that the execution
         * mode of this N_assign has been changed from CUDA_DEVICE_SINGLE
         * to CUDA_HOST_SINGLE, we update its RHS variables. */
        if (old_mode == CUDA_DEVICE_SINGLE
            && ASSIGN_CUDAEXECMODE (arg_node) == CUDA_HOST_SINGLE) {
            INFO_TRAVMODE (arg_info) = cutem_update;
            ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

            /* Everytime we see a mode changing from CUDA_DEVICE_SINGLE to
             * CUDA_HOST_SINGLE, we continue the tagging process ( new HOST_SINGLE
             * instructions might trigger more insutrctions to be set to
             * HOST_SINGLE). */
            CHANGED = TRUE;
        }
        INFO_TRAVMODE (arg_info) = cutem_untag;
    } else if (INFO_TRAVMODE (arg_info) == cutem_update) {
        /* If the mode is cutem_update when traversing this N_assign,
         * we are in either a withloop or a conditional body. We continue
         * updating any RHS variables in the body */
        if (INFO_INWITH (arg_info) || INFO_INCOND (arg_info)) {
            ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
        } else {
            DBUG_UNREACHABLE ("Wrong traverse mode in CUTEMassign!");
        }
    } else {
        DBUG_UNREACHABLE ("Unknown traverse mode in CUTEMassign!");
    }

    INFO_LASTASSIGN (arg_info) = old_assign;
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUTEMcond(node *arg_node, info)
 *
 *   @brief
 *
 *   @param
 *   @param
 *   @return
 *
 *****************************************************************************/
node *
CUTEMcond (node *arg_node, info *arg_info)
{
    bool old_incond;

    DBUG_ENTER ();

    /* We do not traverse the conditional in a do-fun, i.e. recursive call. */
    if (INFO_TRAVMODE (arg_info) == cutem_tag
        || INFO_TRAVMODE (arg_info) == cutem_untag) {
        if (!FUNDEF_ISLOOPFUN (INFO_FUNDEF (arg_info))) {
            COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);
            COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
            COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);
        }
    } else if (INFO_TRAVMODE (arg_info) == cutem_update) {
        old_incond = INFO_INCOND (arg_info);
        INFO_INCOND (arg_info) = TRUE;
        COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);
        COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
        COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);
        INFO_INCOND (arg_info) = old_incond;
    } else {
        DBUG_UNREACHABLE ("Unknown traverse mode in CUTEMcond!");
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUTEMlet(node *arg_node, info)
 *
 *   @brief
 *
 *   @param
 *   @param
 *   @return
 *
 *****************************************************************************/
node *
CUTEMlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_TRAVMODE (arg_info) == cutem_tag
        || INFO_TRAVMODE (arg_info) == cutem_untag) {
        /* Technically, for cutem_untag traversal, we only need to traverse
         * the ids and not the expr. However, since LAC functions are
         * traversed inline, to untag N_assigns in LAC functions,
         * we need to traverse the expr as well */
        INFO_LHS (arg_info) = LET_IDS (arg_node);
        LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);
        LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);
    } else if (INFO_TRAVMODE (arg_info) == cutem_update) {
        LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);
    } else {
        DBUG_UNREACHABLE ("Unknown traverse mode in CUTEMlet!");
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUTEMids(node *arg_node, info)
 *
 *   @brief
 *
 *   @param
 *   @param
 *   @return
 *
 *****************************************************************************/
node *
CUTEMids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_TRAVMODE (arg_info) == cutem_tag) {
        /* Currently, we allow the LHS to be anything (e.g scalar, AKS, AKD, AUD)
         * as long as there is only on ID there (note that we currently do not
         * cudarize wls with more than one return values, so this is fine with
         * wls too)*/
        if (/* !TUisScalar( IDS_NTYPE( arg_node)) && !TYisAKS( IDS_NTYPE( arg_node)) */
            IDS_NEXT (arg_node) != NULL) {
            ASSIGN_CUDAEXECMODE (INFO_LASTASSIGN (arg_info)) = CUDA_HOST_SINGLE;
        }
    } else if (INFO_TRAVMODE (arg_info) == cutem_untag) {
        /* If the result produced by this N_assign is referenced in
         * CUDA_HOST_SINGLE N_assign, the current N_assign is tagged
         * as CUDA_HOST_SINGLE too */
        if (AVIS_ISHOSTREFERENCED (IDS_AVIS (arg_node))) {
            ASSIGN_CUDAEXECMODE (INFO_LASTASSIGN (arg_info)) = CUDA_HOST_SINGLE;
        }
    } else {
        DBUG_UNREACHABLE ("Invalid traverse mode!");
    }

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUTEMid(node *arg_node, info)
 *
 *   @brief
 *
 *   @param
 *   @param
 *   @return
 *
 *****************************************************************************/
node *
CUTEMid (node *arg_node, info *arg_info)
{
    node *lastassign, *param;

    DBUG_ENTER ();

    lastassign = INFO_LASTASSIGN (arg_info);

    if (INFO_TRAVMODE (arg_info) == cutem_tag) {
        /* If we found a N_id defined by CUDA_DEVICE_SINGLE
         * N_assign, the N_assign referenced this N_id is also
         * tagged as CUDA_DEVICE_SINGLE */
        if (IsIdCudaDefined (arg_node, arg_info)) {
            ASSIGN_CUDAEXECMODE (lastassign) = CUDA_DEVICE_SINGLE;
        }
    } else if (INFO_TRAVMODE (arg_info) == cutem_update) {
        DBUG_ASSERT (ASSIGN_CUDAEXECMODE (lastassign) == CUDA_HOST_SINGLE,
                     "Updating N_id in non-CUDA_HOST_SINGLE N_assign!");

        /* In update mode, the N_id should be set host referenced. */
        if (INFO_DOFUNARGS (arg_info) && TUisScalar (ID_NTYPE (arg_node))) {
            /* If we are traversing the parameter list of a do-fun and the
             * parameter is a scalar, we set its flag to TRUE */
            if (TUisScalar (ID_NTYPE (arg_node))) {
                AVIS_ISHOSTREFERENCED (ID_AVIS (arg_node)) = TRUE;
            }
        } else {
            AVIS_ISHOSTREFERENCED (ID_AVIS (arg_node)) = TRUE;
        }

        /* If the id is an array argument passed to the current enclosing
         * do-fun, we need to set the flag of the corresponding parameter
         * in the calling context to also TRUE */
        if (NODE_TYPE (ID_DECL (arg_node)) == N_arg && !TUisScalar (ID_NTYPE (arg_node))
            && FUNDEF_ISLOOPFUN (INFO_FUNDEF (arg_info))) {
            param
              = GetApArgFromFundefArg (INFO_APARGS (arg_info), INFO_FUNDEFARGS (arg_info),
                                       ID_DECL (arg_node));
            AVIS_ISHOSTREFERENCED (ID_AVIS (param)) = TRUE;
        }
    } else {
        /* Do nothing */
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUTEMap(node *arg_node, info)
 *
 *   @brief
 *
 *   @param
 *   @param
 *   @return
 *
 *****************************************************************************/
node *
CUTEMap (node *arg_node, info *arg_info)
{
    node *fundef = NULL;
    info *anon_info;

    DBUG_ENTER ();

    fundef = AP_FUNDEF (arg_node);

    DBUG_ASSERT (fundef != NULL, "Null fundef found!");

    if (INFO_TRAVMODE (arg_info) == cutem_tag) {
        /* If the ap is conditional */
        if (FUNDEF_ISCONDFUN (fundef)) {
            /* If at least one of the arguments of the cond-fun
             * is CUDA defined, we check the suitability of executing
             * this conditional on CUDA */
            if (HasCudaDefinedId (AP_ARGS (arg_node), arg_info)
                && CheckApIds (INFO_LHS (arg_info))) {

                /************ Anonymous Traversal ************/
                anontrav_t atrav[4] = {{N_fundef, &ATravFundefCheckCudarizable},
                                       {N_ap, &ATravApCheckCudarizable},
                                       {N_with, &ATravWithCheckCudarizable},
                                       {(nodetype)0, NULL}};

                TRAVpushAnonymous (atrav, &TRAVsons);

                anon_info = MakeInfo ();

                fundef = TRAVdo (fundef, anon_info);
                /*********************************************/

                FUNDEF_ISCUDALACFUN (fundef) = INFO_AT_ISCUDARIZABLE (arg_info);

                /* If the conditional lac fun is cudarizbale, we tag the
                 * application of it as CUDA_DEVICE_SINGLE */
                if (FUNDEF_ISCUDALACFUN (fundef)) {
                    ASSIGN_CUDAEXECMODE (INFO_LASTASSIGN (arg_info)) = CUDA_DEVICE_SINGLE;
                }

                /************ Anonymous Traversal ************/
                anon_info = FreeInfo (anon_info);
                TRAVpop ();
                /*********************************************/
            } else {
                /* If the cond-fun is not cudarizable, we traverse
                 * into it to tag the N_assigns in it */
                fundef = TraverseLacFun (fundef, arg_node, arg_info);
            }
        }
        /* If the ap is do-fun, it's tagged as host single and
         * traverse into the loop body */
        else if (FUNDEF_ISLOOPFUN (fundef) && fundef != INFO_FUNDEF (arg_info)) {
            ASSIGN_CUDAEXECMODE (INFO_LASTASSIGN (arg_info)) = CUDA_HOST_SINGLE;
            fundef = TraverseLacFun (fundef, arg_node, arg_info);
        }
        /* If the ap is normal fun app */
        else {
            /* All other N_aps are immediately tagged as CUDA_HOST_SINGLE */
            ASSIGN_CUDAEXECMODE (INFO_LASTASSIGN (arg_info)) = CUDA_HOST_SINGLE;
        }
    } else if (INFO_TRAVMODE (arg_info) == cutem_untag) {
        if (FUNDEF_ISLACFUN (fundef) && fundef != INFO_FUNDEF (arg_info)) {
            /* Traverse into lac function body to untag N_assigns if needed */
            fundef = TraverseLacFun (fundef, arg_node, arg_info);
        }
    } else if (INFO_TRAVMODE (arg_info) == cutem_update) {
        /* This ensures that we don't update the parameter list of the
         * recursive loop function call. */
        if (fundef != INFO_FUNDEF (arg_info)) {
            /* If the fun app is a do fun, we treat its paramenter slightly
             * differently. The ISHOSTREFERENCED flag of each array parameter
             * is only set when we traverse the do fun body. So here we only
             * set the flag for non-array parameters, i.e. scalars. */
            if (FUNDEF_ISLOOPFUN (fundef)) {
                INFO_DOFUNARGS (arg_info) = TRUE;
            }
            AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);
            INFO_DOFUNARGS (arg_info) = FALSE;
        }
    } else {
        DBUG_UNREACHABLE ("Invalid traverse mode!");
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUTEMwith(node *arg_node, info)
 *
 *   @brief
 *
 *   @param
 *   @param
 *   @return
 *
 *****************************************************************************/
node *
CUTEMwith (node *arg_node, info *arg_info)
{
    bool old_inwith;

    DBUG_ENTER ();

    if (INFO_TRAVMODE (arg_info) == cutem_tag) {
        /* Cudarizbale N_with is tagged as CUDA_DEVICE_MULTI */
        if (WITH_CUDARIZABLE (arg_node)) {
            ASSIGN_CUDAEXECMODE (INFO_LASTASSIGN (arg_info)) = CUDA_DEVICE_MULTI;
        }
    } else if (INFO_TRAVMODE (arg_info) == cutem_update) {
        if (!WITH_CUDARIZABLE (arg_node)) {
            old_inwith = INFO_INWITH (arg_info);
            INFO_INWITH (arg_info) = TRUE;
            /* For non cudarizable withloops, we traverse its body to
             * set the IsHostReferenced attributes of all N_ids */
            WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);
            INFO_INWITH (arg_info) = old_inwith;
        }
    } else {
        /* Do nothing */
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
