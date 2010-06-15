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
#include "dbug.h"
#include "globals.h"
#include "type_utils.h"
#include "new_types.h"
#include "tag_cuda_lac.h"

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
};

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LASTASSIGN(n) (n->lastassign)
#define INFO_FROMAP(n) (n->fromap)
#define INFO_TRAVMODE(n) (n->travmode)
#define INFO_INWITH(n) (n->inwith)
#define INFO_INCOND(n) (n->incond)
#define INFO_LHS(n) (n->lhs)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_LASTASSIGN (result) = NULL;
    INFO_FROMAP (result) = FALSE;
    INFO_TRAVMODE (result) = cutem_tag;
    INFO_INWITH (result) = FALSE;
    INFO_INCOND (result) = FALSE;
    INFO_LHS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

static bool
IsIdCudaDefined (node *id)
{
    bool res = FALSE;
    node *ssa;
    ntype *type;

    DBUG_ENTER ("IsIdCudaDefined");

    /* Get the SSAASSIGN of this N_id */
    ssa = ID_SSAASSIGN (id);
    type = ID_NTYPE (id);

    /* If this N_id is defined by an N_assign that needs to be
     * executed on CUDA, the N_assign referenced this N_id will
     * be tagged as CUDA_DEVICE_SINGLE. Note the N_id must not be
     * referenced in any CUDA_HOST_SINGLE N_assign. Otherwise the current
     * N_assign referencing this N_id is not executable on CUDA either. */
    if (ssa != NULL) {
        if ((TUisScalar (type) || TYisAKS (type)) && /* Scalar or AKS */
            !AVIS_ISHOSTREFERENCED (ID_AVIS (id)) && /* NOT referenced in host N_assign */
            (ASSIGN_EXECMODE (ssa) == CUDA_DEVICE_SINGLE
             || /* Define by DEVICE_SINGLE or DEVICE_MULTI N_assign */
             ASSIGN_EXECMODE (ssa) == CUDA_DEVICE_MULTI)) {
            res = TRUE;
        }
    }
    /* If the id is passed as an argument and this id in the calling
     * context is defined by a CUDA_DEVICE_SINGLE or CUDA_DEVICE_MULTI
     * the current assignment is set to CUDA_DEVICE_SINGLE */
    else if (NODE_TYPE (ID_DECL (id)) == N_arg) {
        if (ARG_ISCUDADEFINED (ID_DECL (id)) && !AVIS_ISHOSTREFERENCED (ID_AVIS (id))) {
            res = TRUE;
        }
    }

    DBUG_RETURN (res);
}

static void
SetArgCudaDefined (node *ap_args, node *fundef_args)
{
    DBUG_ENTER ("SetArgCudaDefined");

    while (ap_args != NULL) {
        ARG_ISCUDADEFINED (fundef_args) = IsIdCudaDefined (EXPRS_EXPR (ap_args));

        ap_args = EXPRS_NEXT (ap_args);
        fundef_args = ARG_NEXT (fundef_args);
    }

    DBUG_VOID_RETURN;
}

static void
ClearArgCudaDefined (node *fundef_args)
{
    DBUG_ENTER ("ClearArgCudaDefined");

    while (fundef_args != NULL) {
        ARG_ISCUDADEFINED (fundef_args) = FALSE;
        fundef_args = ARG_NEXT (fundef_args);
    }

    DBUG_VOID_RETURN;
}

static bool
HasCudaDefinedId (node *ap_args)
{
    bool res = FALSE;

    DBUG_ENTER ("HasCudaDefinedId");

    while (ap_args != NULL) {
        if (IsIdCudaDefined (EXPRS_EXPR (ap_args))) {
            res = TRUE;
            break;
        }
        ap_args = EXPRS_NEXT (ap_args);
    }

    DBUG_RETURN (res);
}

static node *
TraverseLacFun (node *fundef, info *arg_info)
{
    node *old_fundef;
    bool old_fromap;

    DBUG_ENTER ("TraverseLacFun");

    /* Push info */
    old_fundef = INFO_FUNDEF (arg_info);
    old_fromap = INFO_FROMAP (arg_info);

    INFO_FROMAP (arg_info) = TRUE;
    fundef = TRAVdo (fundef, arg_info);

    /* Pop info */
    INFO_FROMAP (arg_info) = old_fromap;
    INFO_FUNDEF (arg_info) = old_fundef;

    DBUG_RETURN (fundef);
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

    DBUG_ENTER ("CUTagExecutionmode");

    TRAVpush (TR_cutem);

    /* Fixed point iteration */
    do {
        CHANGED = FALSE;

        printf ("**************** Starting iteration %d ****************\n", ITERATION);

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
    DBUG_ENTER ("CUTEMfundef");

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
 * @fn node *CUTEMargs(node *arg_node, info)
 *
 *   @brief
 *
 *   @param
 *   @param
 *   @return
 *
 *****************************************************************************/
node *
CUTEMargs (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUTEMargs");

    /* We initilise IsHostReferenced in the first iteration of TAG traversal */
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
    DBUG_ENTER ("CUTEMblock");

    /* Must traverse all vardecs before traversing assignment chain */
    BLOCK_VARDEC (arg_node) = TRAVopt (BLOCK_VARDEC (arg_node), arg_info);
    BLOCK_INSTR (arg_node) = TRAVopt (BLOCK_INSTR (arg_node), arg_info);

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
    DBUG_ENTER ("CUTEMvardec");

    /* We only initilise the needcount to 0 in the first iteration of tagging */
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

    DBUG_ENTER ("CUTEMassign");

    old_assign = INFO_LASTASSIGN (arg_info);
    INFO_LASTASSIGN (arg_info) = arg_node;

    if (INFO_TRAVMODE (arg_info) == cutem_tag) {
        /* Each N_assign is intially tagged to be CUDA_HOST_SINGLE */
        ASSIGN_EXECMODE (arg_node) = CUDA_HOST_SINGLE;
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

        /* If after tagging, the execution mode of this N_assign
         * is still CUDA_HOST_SINGLE(means that this N_assign has
         * to be executed on the host), we update it's RHS variables */
        if (ASSIGN_EXECMODE (arg_node) == CUDA_HOST_SINGLE) {
            INFO_TRAVMODE (arg_info) = cutem_update;
            ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
        }
        INFO_TRAVMODE (arg_info) = cutem_tag;
    } else if (INFO_TRAVMODE (arg_info) == cutem_untag) {

        old_mode = ASSIGN_EXECMODE (arg_node);
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

        /* If after traversing the RHS, we found that the execution
         * mode of this N_assign has been changed from CUDA_DEVICE_SINGLE
         * to CUDA_HOST_SINGLE, we update all variables on the RHS */
        if (old_mode == CUDA_DEVICE_SINGLE
            && ASSIGN_EXECMODE (arg_node) == CUDA_HOST_SINGLE) {
            INFO_TRAVMODE (arg_info) = cutem_update;
            ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
            CHANGED = TRUE;
        }
        INFO_TRAVMODE (arg_info) = cutem_untag;
    } else {
        /* If the mode is cutem_update when traversing this N_assign,
         * then we are either in withloop body or in a conditional */
        if (INFO_INWITH (arg_info) || INFO_INCOND (arg_info)) {
            ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
        } else {
            DBUG_ASSERT ((0), "Wrong traverse mode in CUTEMassign!");
        }
    }

    INFO_LASTASSIGN (arg_info) = old_assign;
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUTEcond(node *arg_node, info)
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

    DBUG_ENTER ("CUTEMcond");

    if (INFO_TRAVMODE (arg_info) != cutem_update) {
        old_incond = INFO_INCOND (arg_info);
        INFO_INCOND (arg_info) = TRUE;
        COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);
        COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
        COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);
        INFO_INCOND (arg_info) = old_incond;
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
    DBUG_ENTER ("CUTEMlet");

    if (INFO_TRAVMODE (arg_info) == cutem_tag
        || INFO_TRAVMODE (arg_info) == cutem_untag) {
        /* Technically, for cutem_untag traversal, we only need to traverse
         * the ids and not the expr. However, since LAC functions are
         * traverse inline, therefore, to untag N_assigns in LAC functions,
         * we need to traverse expr as well */
        INFO_LHS (arg_info) = LET_IDS (arg_node);
        LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);
        LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);
    } else if (INFO_TRAVMODE (arg_info) == cutem_update) {
        LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);
    } else {
        DBUG_ASSERT ((0), "Invalid traverse mode!");
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
    DBUG_ENTER ("CUTEMids");

    if (INFO_TRAVMODE (arg_info) == cutem_tag) {
        /* If the LHS is neither scalar or AKS array, the N_assign can
         * only be executed on the host, i.e. CUDA_HOST_SINGLE */
        if (!TUisScalar (IDS_NTYPE (arg_node)) && !TYisAKS (IDS_NTYPE (arg_node))) {
            ASSIGN_EXECMODE (INFO_LASTASSIGN (arg_info)) = CUDA_HOST_SINGLE;
        }
    } else if (INFO_TRAVMODE (arg_info) == cutem_untag) {
        /* If the result produced by this N_assign is referenced in
         * CUDA_HOST_SINGLE N_assign, the current N_assign is tagged
         * as CUDA_HOST_SINGLE too */
        if (AVIS_ISHOSTREFERENCED (IDS_AVIS (arg_node))) {
            ASSIGN_EXECMODE (INFO_LASTASSIGN (arg_info)) = CUDA_HOST_SINGLE;
        }
    } else {
        DBUG_ASSERT ((0), "Invalid traverse mode!");
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
    node *lastassign;

    DBUG_ENTER ("CUTEMid");

    lastassign = INFO_LASTASSIGN (arg_info);

    if (INFO_TRAVMODE (arg_info) == cutem_tag) {
        /* If we found a N_id defined by CUDA_DEVICE_SINGLE
         * N_assign, the N_assign referenced this N_id is also
         * tagged as CUDA_DEVICE_SINGLE */
        if (IsIdCudaDefined (arg_node)) {
            ASSIGN_EXECMODE (lastassign) = CUDA_DEVICE_SINGLE;
        }
    } else if (INFO_TRAVMODE (arg_info) == cutem_update) {
        DBUG_ASSERT (ASSIGN_EXECMODE (lastassign) == CUDA_HOST_SINGLE,
                     "Updating N_id in non-CUDA_HOST_SINGLE N_assign!");

        /* If we are in update mode, the N_id should be set host referenced
         * Also, if the N_id is in the argument list of LAC funap, we need
         * to propagate this information to the fundef as well */
        AVIS_ISHOSTREFERENCED (ID_AVIS (arg_node)) = TRUE;
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

    DBUG_ENTER ("CUTEMap");

    fundef = AP_FUNDEF (arg_node);

    DBUG_ASSERT (fundef != NULL, "Null fundef found!");

    if (INFO_TRAVMODE (arg_info) == cutem_tag) {
        /* If the ap is conditional */
        if (FUNDEF_ISCONDFUN (fundef)) {
            /* If at least one of the arguments of the cond-fun
             * is cuda defined, we check the suitability of executing
             * this conditional on cuda */
            if (HasCudaDefinedId (AP_ARGS (arg_node))) {
                arg_node = TCULACdoTagCudaLac (arg_node, INFO_LHS (arg_info),
                                               FUNDEF_ARGS (fundef));
                /* If the lac fun is cudarizbale, we tag the
                 * application of this lac fun as CUDA_DEVICE_SINGLE */
                if (FUNDEF_ISCUDALACFUN (fundef)) {
                    ASSIGN_EXECMODE (INFO_LASTASSIGN (arg_info)) = CUDA_DEVICE_SINGLE;
                }
            } else {
                fundef = TraverseLacFun (fundef, arg_info);
            }
        }
        /* If the ap is loop */
        else if (FUNDEF_ISDOFUN (fundef) && fundef != INFO_FUNDEF (arg_info)) {
            SetArgCudaDefined (AP_ARGS (arg_node), FUNDEF_ARGS (fundef));
            fundef = TraverseLacFun (fundef, arg_info);
        }
        /* If the ap is normal fun app */
        else {
            /* All other N_aps are immediately tagged as CUDA_HOST_SINGLE */
            ASSIGN_EXECMODE (INFO_LASTASSIGN (arg_info)) = CUDA_HOST_SINGLE;
        }
    } else if (INFO_TRAVMODE (arg_info) == cutem_untag) {
        if (FUNDEF_ISLACFUN (fundef) && fundef != INFO_FUNDEF (arg_info)) {
            /* Traverse into lac function body to untage any N_assigns
             * if needed */
            fundef = TraverseLacFun (fundef, arg_info);
        }
    } else if (INFO_TRAVMODE (arg_info) == cutem_update) {
        if (fundef != INFO_FUNDEF (arg_info)) {
            AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);
            // ClearArgCudaDefined( FUNDEF_ARGS( fundef));
        }
    } else {
        DBUG_ASSERT ((0), "Invalid traverse mode!");
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

    DBUG_ENTER ("CUTEMwith");

    if (INFO_TRAVMODE (arg_info) == cutem_tag) {
        /* Cudarizbale N_with is tagged as CUDA_DEVICE_MULTI */
        if (WITH_CUDARIZABLE (arg_node)) {
            ASSIGN_EXECMODE (INFO_LASTASSIGN (arg_info)) = CUDA_DEVICE_MULTI;
        }
    } else {
        if (!WITH_CUDARIZABLE (arg_node)) {
            old_inwith = INFO_INWITH (arg_info);
            INFO_INWITH (arg_info) = TRUE;
            /* For non cudarizable withloops, we traverse its body to
             * set the IsHostReferenced attributes of all N_ids */
            WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);
            INFO_INWITH (arg_info) = old_inwith;
        }
    }

    DBUG_RETURN (arg_node);
}
