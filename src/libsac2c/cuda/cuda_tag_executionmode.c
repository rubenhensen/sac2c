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

static bool CHANGED = FALSE;
static int ITERATION = 1;

typedef enum { TAG, UNTAG, VARUPDATE } travmode_t;

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *lastassign;
    bool fromap;
    bool inapargs;
    node *fundefargs;
    travmode_t travmode;
    bool inlacfun;
    bool cudarizable;
    bool inwith;
    bool incond;
};

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LASTASSIGN(n) (n->lastassign)
#define INFO_FROMAP(n) (n->fromap)
#define INFO_INAPARGS(n) (n->inapargs)
#define INFO_FUNDEFARGS(n) (n->fundefargs)
#define INFO_TRAVMODE(n) (n->travmode)
#define INFO_INLACFUN(n) (n->inlacfun)
#define INFO_CUDARIZABLE(n) (n->cudarizable)
#define INFO_INWITH(n) (n->inwith)
#define INFO_INCOND(n) (n->incond)

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
    INFO_INAPARGS (result) = FALSE;
    INFO_FUNDEFARGS (result) = NULL;
    INFO_TRAVMODE (result) = TAG;
    INFO_INLACFUN (result) = FALSE;
    INFO_CUDARIZABLE (result) = FALSE;
    INFO_INWITH (result) = FALSE;
    INFO_INCOND (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUTEMdoTagExecutionmode(node *arg_node, info)
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

        printf (
          "************************* Starting iteration %d *************************\n",
          ITERATION);

        arg_info = MakeInfo ();
        INFO_TRAVMODE (arg_info) = TAG;
        syntax_tree = TRAVdo (syntax_tree, arg_info);
        arg_info = FreeInfo (arg_info);

        arg_info = MakeInfo ();
        INFO_TRAVMODE (arg_info) = UNTAG;
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
            printf ("Traversing function %s\n", FUNDEF_NAME (arg_node));
            FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

            if (INFO_TRAVMODE (arg_info) == TAG) {
                printf ("Setting cudarizable function %s(%d)\n", FUNDEF_NAME (arg_node),
                        INFO_CUDARIZABLE (arg_info));
                FUNDEF_ISCUDALACFUN (arg_node) = INFO_CUDARIZABLE (arg_info);
            }
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

    /* We only initilise the needcount to 0 in the first iteration of tagging */
    if (INFO_TRAVMODE (arg_info) == TAG && ITERATION == 1) {
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
    if (INFO_TRAVMODE (arg_info) == TAG && ITERATION == 1) {
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

    if (INFO_TRAVMODE (arg_info) == TAG) {
        /* Each N_assign is intially tagged to be CUDA_HOST_SINGLE */
        ASSIGN_EXECMODE (arg_node) = CUDA_HOST_SINGLE;
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

        /* If after tagging, the execution mode of this N_assign
         * is still CUDA_HOST_SINGLE(means that this N_assign has
         * to be executed on the host), we update it's RHS variables */
        if (ASSIGN_EXECMODE (arg_node) == CUDA_HOST_SINGLE) {
            INFO_TRAVMODE (arg_info) = VARUPDATE;
            ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
        }
        INFO_TRAVMODE (arg_info) = TAG;
    } else if (INFO_TRAVMODE (arg_info) == UNTAG) {

        old_mode = ASSIGN_EXECMODE (arg_node);
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

        /* If after traversing the RHS, we found that the execution
         * mode of this N_assign has been changed from CUDA_DEVICE_SINGLE
         * to CUDA_HOST_SINGLE, we update all variables on the RHS */
        if (old_mode == CUDA_DEVICE_SINGLE
            && ASSIGN_EXECMODE (arg_node) == CUDA_HOST_SINGLE) {
            INFO_TRAVMODE (arg_info) = VARUPDATE;
            ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
            CHANGED = TRUE;
        }
        INFO_TRAVMODE (arg_info) = UNTAG;
    } else {
        /* If we have a mode of VARUPDATE, this means we should be either
         * in the withloop body or in a conditional */
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

    if (INFO_TRAVMODE (arg_info) != VARUPDATE) {
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

    if (INFO_TRAVMODE (arg_info) == TAG || INFO_TRAVMODE (arg_info) == UNTAG) {
        /* Techonically, for UNTAG traversal, we only need to traverse
         * the ids and not the expr. However, since LAC functions are
         * traverse inline, therefore, to untag N_assigns in LAC functions,
         * we need to traverse expr as well */
        LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);
        LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);
    } else if (INFO_TRAVMODE (arg_info) == VARUPDATE) {
        LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);
    } else {
        DBUG_ASSERT ((0), "Unknown traverse mode!");
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

    if (INFO_TRAVMODE (arg_info) == TAG) {
        /* If the LHS contains non-AKS arrays, it can only be executed
         * on the host, i.e. CUDA_HOST_SINGLE */
        if (!TUisScalar (IDS_NTYPE (arg_node)) && !TYisAKS (IDS_NTYPE (arg_node))) {
            ASSIGN_EXECMODE (INFO_LASTASSIGN (arg_info)) = CUDA_HOST_SINGLE;
        }
    } else if (INFO_TRAVMODE (arg_info) == UNTAG) {
        if (AVIS_ISHOSTREFERENCED (IDS_AVIS (arg_node))) {
            ASSIGN_EXECMODE (INFO_LASTASSIGN (arg_info)) = CUDA_HOST_SINGLE;
        }
    } else {
        DBUG_ASSERT ((0), "Wrong traverse mode in CUTEMids!");
    }

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUTEMexprs(node *arg_node, info)
 *
 *   @brief
 *
 *   @param
 *   @param
 *   @return
 *
 *****************************************************************************/
node *
CUTEMexprs (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUTEMexprs");

    if (INFO_TRAVMODE (arg_info) == TAG || INFO_TRAVMODE (arg_info) == VARUPDATE) {
        EXPRS_EXPR (arg_node) = TRAVopt (EXPRS_EXPR (arg_node), arg_info);
        if (INFO_INAPARGS (arg_info)) {
            DBUG_ASSERT (INFO_FUNDEFARGS (arg_info) != NULL, "INFO_FUNDEFARGS is NULL!");
            INFO_FUNDEFARGS (arg_info) = ARG_NEXT (INFO_FUNDEFARGS (arg_info));
        }
    }

    EXPRS_NEXT (arg_node) = TRAVopt (EXPRS_NEXT (arg_node), arg_info);

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
    node *ssa, *lastassign;
    ntype *type;
    bool cudadefined = FALSE; /* Default to FALSE */

    DBUG_ENTER ("CUTEMid");

    lastassign = INFO_LASTASSIGN (arg_info);

    if (INFO_TRAVMODE (arg_info) == TAG) {
        /* Get the SSAASSIGN of this N_id */
        ssa = ID_SSAASSIGN (arg_node);
        type = ID_NTYPE (arg_node);

        /* If this N_id is defined by an N_assign that needs to be
         * executed on CUDA, the N_assign referenced this N_id will
         * be tagged as CUDA_DEVICE_SINGLE. Note the N_id must not be
         * referenced in any CUDA_HOST_SINGLE N_assign. Otherwise the current
         * N_assign referencing this N_id is not executable on CUDA either. */
        if (ssa != NULL) {
            if ((TUisScalar (type) || TYisAKS (type)) && /* Scalar or AKS */
                !AVIS_ISHOSTREFERENCED (ID_AVIS (arg_node))
                && /* NOT referenced in host N_assign */
                (ASSIGN_EXECMODE (ssa) == CUDA_DEVICE_SINGLE
                 || /* Define by DEVICE_SINGLE or DEVICE_MULTI N_assign */
                 ASSIGN_EXECMODE (ssa) == CUDA_DEVICE_MULTI)) {
                ASSIGN_EXECMODE (lastassign) = CUDA_DEVICE_SINGLE;
                cudadefined = TRUE;
            }
        }
        /* If the id is passed as an argument and this id in the calling
         * context is defined by a CUDA_DEVICE_SINGLE or CUDA_DEVICE_MULTI
         * the current assignment is set to CUDA_DEVICE_SINGLE */
        else if (NODE_TYPE (ID_DECL (arg_node)) == N_arg) {
            if (ARG_ISCUDADEFINED (ID_DECL (arg_node))
                && !AVIS_ISHOSTREFERENCED (ID_AVIS (arg_node))) {
                ASSIGN_EXECMODE (lastassign) = CUDA_DEVICE_SINGLE;
                cudadefined = TRUE;
            }
        }
    } else if (INFO_TRAVMODE (arg_info) == VARUPDATE) {
        DBUG_ASSERT (ASSIGN_EXECMODE (lastassign) == CUDA_HOST_SINGLE,
                     "Update variable in non-CUDA_HOST_SINGLE N_assign!");

        /* If we are in update mode, the N_id should be set host referenced
         * Also, if the N_id is in the argument list of LAC funap, we need
         * to propagate this information to the fundef as well */

        /*
        if( INFO_INAPARGS( arg_info)) {
          ARG_ISCUDADEFINED( INFO_FUNDEFARGS( arg_info)) = FALSE;
          AVIS_ISHOSTREFERENCED( ARG_AVIS( INFO_FUNDEFARGS( arg_info))) = TRUE;
        }
        */

        AVIS_ISHOSTREFERENCED (ID_AVIS (arg_node)) = TRUE;
    }

    /* If the N_id is in the arguments list of a LAC N_ap,
     * we propogate the information to the arguments of
     * LAC N_fundef */
    if (INFO_INAPARGS (arg_info)) {
        printf ("Setting argument %s CUDADEFINED attribute to %d\n",
                ARG_NAME (INFO_FUNDEFARGS (arg_info)), cudadefined);
        ARG_ISCUDADEFINED (INFO_FUNDEFARGS (arg_info)) = cudadefined;
        AVIS_ISHOSTREFERENCED (ARG_AVIS (INFO_FUNDEFARGS (arg_info))) = !cudadefined;
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
    bool old_fromap, old_cudarizable, old_inlacfun, traverse_lacfun;
    node *old_fundef;

    DBUG_ENTER ("CUTEMap");

    fundef = AP_FUNDEF (arg_node);

    traverse_lacfun
      = (fundef != NULL && FUNDEF_ISLACFUN (fundef) && fundef != INFO_FUNDEF (arg_info));

    if (INFO_TRAVMODE (arg_info) == TAG) {

        /* If the N_ap is a lac-fun, we traverse its arguments and see
         * if it can possibly be executed in single thread in CUDA. However,
         * even if the N_ap is tagged as cudarizable in this phase,
         * it might be re-tagged as CUDA_HOST_SINGLE in the next phase
         * (annotate cuda lacfun) if the code in the function is not
         * executable on CUDA e.g. it contains function applications. */
        if (traverse_lacfun) {

            /* Tag the IsCudaDefined attribute of each lac  fun argument */
            INFO_INAPARGS (arg_info) = TRUE;
            INFO_FUNDEFARGS (arg_info) = FUNDEF_ARGS (fundef);
            AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);
            INFO_INAPARGS (arg_info) = FALSE;
            INFO_FUNDEFARGS (arg_info) = NULL;

            /* Push info */
            old_cudarizable = INFO_CUDARIZABLE (arg_info);
            old_inlacfun = INFO_INLACFUN (arg_info);
            old_fundef = INFO_FUNDEF (arg_info);
            old_fromap = INFO_FROMAP (arg_info);

            INFO_CUDARIZABLE (arg_info) = TRUE;
            //( ASSIGN_EXECMODE( INFO_LASTASSIGN( arg_info)) == CUDA_DEVICE_SINGLE);
            INFO_INLACFUN (arg_info) = TRUE;
            INFO_FROMAP (arg_info) = TRUE;
            fundef = TRAVdo (fundef, arg_info);

            INFO_CUDARIZABLE (arg_info) = old_cudarizable && FUNDEF_ISCUDALACFUN (fundef);

            /* If the lac fun is not cudarizbale, we tag the
             * application of this lac fun as CUDA_HOST_SINGLE */
            if (!FUNDEF_ISCUDALACFUN (fundef)) {
                ASSIGN_EXECMODE (INFO_LASTASSIGN (arg_info)) = CUDA_HOST_SINGLE;
            }

            /* Pop info */
            INFO_FROMAP (arg_info) = old_fromap;
            INFO_FUNDEF (arg_info) = old_fundef;
            INFO_INLACFUN (arg_info) = old_inlacfun;

        } else {
            if (fundef != INFO_FUNDEF (arg_info)) {
                /* All other N_aps are immediately tagged as CUDA_HOST_SINGLE */
                ASSIGN_EXECMODE (INFO_LASTASSIGN (arg_info)) = CUDA_HOST_SINGLE;

                /* A normal funap in LAC fun makes it un-cudarizable */
                if (INFO_INLACFUN (arg_info)) {
                    INFO_CUDARIZABLE (arg_info) = FALSE;
                }
            }
        }
    } else if (INFO_TRAVMODE (arg_info) == VARUPDATE) {
        if (traverse_lacfun) {
            /* Tag the IsCudaDefined attribute of each lac fun argument */
            INFO_INAPARGS (arg_info) = TRUE;
            INFO_FUNDEFARGS (arg_info) = FUNDEF_ARGS (fundef);
            AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);
            INFO_INAPARGS (arg_info) = FALSE;
            INFO_FUNDEFARGS (arg_info) = NULL;

            /* The LaC fun cannot be executed on CUDA */
            FUNDEF_ISCUDALACFUN (fundef) = FALSE;
        } else if (fundef != INFO_FUNDEF (arg_info)) {
            AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);
        }
    } else if (INFO_TRAVMODE (arg_info) == UNTAG) {
        if (traverse_lacfun) {
            /* Push info */
            old_inlacfun = INFO_INLACFUN (arg_info);
            old_fundef = INFO_FUNDEF (arg_info);
            old_fromap = INFO_FROMAP (arg_info);

            INFO_INLACFUN (arg_info) = TRUE;
            INFO_FROMAP (arg_info) = TRUE;
            fundef = TRAVdo (fundef, arg_info);

            /* Pop info */
            INFO_FROMAP (arg_info) = old_fromap;
            INFO_FUNDEF (arg_info) = old_fundef;
            INFO_INLACFUN (arg_info) = old_inlacfun;
        }
    } else {
        DBUG_ASSERT ((0), "Wrong traverse mode in CUTEMap!");
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

    if (INFO_TRAVMODE (arg_info) == TAG) {
        /* Cudarizbale N_with is tagged as CUDA_DEVICE_MULTI */
        if (WITH_CUDARIZABLE (arg_node)) {
            ASSIGN_EXECMODE (INFO_LASTASSIGN (arg_info)) = CUDA_DEVICE_MULTI;
        }

        if (INFO_INLACFUN (arg_info)) {
            INFO_CUDARIZABLE (arg_info) = FALSE;
        }
    } else {
        if (!WITH_CUDARIZABLE (arg_node)) {
            old_inwith = INFO_INWITH (arg_info);
            INFO_INWITH (arg_info) = TRUE;
            WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);
            INFO_INWITH (arg_info) = old_inwith;
        }
    }

    DBUG_RETURN (arg_node);
}
