/** <!--********************************************************************-->
 *
 * @file cuda_tag_executionmode.c
 *
 * prefix: TEM
 *
 * description:
 *   tags the assignments, wheter their executionmode is CUDA_HOST_SINGLE,
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

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *lastassign;
    node *letids;
    bool fromap;
    bool inapargs;
    node *fundefargs;
    bool host_single;
};

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LASTASSIGN(n) (n->lastassign)
#define INFO_LETIDS(n) (n->letids)
#define INFO_FROMAP(n) (n->fromap)
#define INFO_INAPARGS(n) (n->inapargs)
#define INFO_FUNDEFARGS(n) (n->fundefargs)
#define INFO_HOST_SINGLE(n) (n->host_single)

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
    INFO_LETIDS (result) = NULL;
    INFO_FROMAP (result) = FALSE;
    INFO_INAPARGS (result) = FALSE;
    INFO_FUNDEFARGS (result) = NULL;
    INFO_HOST_SINGLE (result) = TRUE;

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

    arg_info = MakeInfo ();

    TRAVpush (TR_cutem);
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

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
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    } else {
        if (INFO_FROMAP (arg_info)) {
            printf ("Traversing function: %s\n", FUNDEF_NAME (arg_node));
            FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        } else {
            FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

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
    bool old_host_single;

    DBUG_ENTER ("CUTEMassign");

    old_assign = INFO_LASTASSIGN (arg_info);
    INFO_LASTASSIGN (arg_info) = arg_node;

    old_host_single = INFO_HOST_SINGLE (arg_info);
    INFO_HOST_SINGLE (arg_info) = TRUE;

    /* Each N_assign is default to be CUDA_HOST_SINGGLE */
    ASSIGN_EXECMODE (arg_node) = CUDA_HOST_SINGLE;
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    /*
      if( INFO_HOST_SINGLE( arg_info)) {
        ASSIGN_EXECMODE( arg_node) = CUDA_HOST_SINGLE;
      }
    */

    INFO_LASTASSIGN (arg_info) = old_assign;
    INFO_HOST_SINGLE (arg_info) = old_host_single;

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

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

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);
    INFO_LETIDS (arg_info) = NULL;

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

    if (INFO_INAPARGS (arg_info)) {
        DBUG_ASSERT (INFO_FUNDEFARGS (arg_info) != NULL, "Null in INFO_FUNDEFARGS!");
        DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (arg_node)) == N_id,
                     "Non N_id node found in N_ap args!");

        EXPRS_EXPR (arg_node) = TRAVopt (EXPRS_EXPR (arg_node), arg_info);
        INFO_FUNDEFARGS (arg_info) = ARG_NEXT (INFO_FUNDEFARGS (arg_info));
    } else {
        EXPRS_EXPR (arg_node) = TRAVopt (EXPRS_EXPR (arg_node), arg_info);
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
    node *ssa;
    ntype *type;

    DBUG_ENTER ("CUTEMid");

    /* Get the SSA N_assign of this N_id */
    ssa = AVIS_SSAASSIGN (ID_AVIS (arg_node));
    type = AVIS_TYPE (ID_AVIS (arg_node));

    /* If this N_id is defined by an N_assign that needs to be
     * executed on CUDA, the N_assign referenced this N_id will
     * be tagged as CUDA_DEVICE_SINGLE */
    if (ssa != NULL) {
        if ((TUisScalar (type) || TYisAKS (type))
            && (ASSIGN_EXECMODE (ssa) == CUDA_DEVICE_SINGLE
                || ASSIGN_EXECMODE (ssa) == CUDA_DEVICE_MULTI)) {
            ASSIGN_EXECMODE (INFO_LASTASSIGN (arg_info)) = CUDA_DEVICE_SINGLE;
            INFO_HOST_SINGLE (arg_info) = FALSE;
            if (INFO_INAPARGS (arg_info)) {
                ARG_ISCUDADEFINED (INFO_FUNDEFARGS (arg_info)) = TRUE;
            }
        }
    }
    /* If the id is passed as an argument and this id in the calling
     * context is defined by a CUDA_DEVICE_SINGLE or CUDA_DEVICE_MULTI
     * the current assignment is set to CUDA_DEVICE_SINGLE */
    else {
        if (NODE_TYPE (AVIS_DECL (ID_AVIS (arg_node))) == N_arg) {
            if (ARG_ISCUDADEFINED (AVIS_DECL (ID_AVIS (arg_node)))) {
                ASSIGN_EXECMODE (INFO_LASTASSIGN (arg_info)) = CUDA_DEVICE_SINGLE;
                INFO_HOST_SINGLE (arg_info) = FALSE;
                if (INFO_INAPARGS (arg_info)) {
                    ARG_ISCUDADEFINED (INFO_FUNDEFARGS (arg_info)) = TRUE;
                }
            }
        }
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
    node *fundef = NULL, *ids = NULL;
    bool ids_valid = TRUE, old_fromap;
    node *old_fundef;

    DBUG_ENTER ("CUTEMap");

    ids = INFO_LETIDS (arg_info);
    fundef = AP_FUNDEF (arg_node);

    /* If the N_ap is a lac-fun, we traverse its arguments and see
     * if it can possibly be executed in single thread in CUDA. However,
     * even if the lac-fun is tagged as cudarizable in this phase,
     * it maight be re-tagged as CUDA_HOST_SINGLE in the next phase
     * if the code in the function is not executable on CUDA e.g.
     * function applications. */
    if (fundef != NULL && FUNDEF_ISLACFUN (fundef) && fundef != INFO_FUNDEF (arg_info)) {
        while (ids != NULL) {
            if (!TYisAKS (AVIS_TYPE (IDS_AVIS (ids)))
                || TUisScalar (AVIS_TYPE (IDS_AVIS (ids)))) {
                ids_valid = FALSE;
                break;
            }
            ids = IDS_NEXT (ids);
        }

        /* Tag the IsCudaDefined attribute of each lac fun argument */
        INFO_INAPARGS (arg_info) = TRUE;
        INFO_FUNDEFARGS (arg_info) = FUNDEF_ARGS (fundef);
        AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);
        INFO_INAPARGS (arg_info) = FALSE;
        INFO_FUNDEFARGS (arg_info) = NULL;

        /* If the return values of the do fun contain scalars, the
         * do fun id tagged as host single since cuda function cannot
         * return scalars */
        if (!ids_valid) {
            ASSIGN_EXECMODE (INFO_LASTASSIGN (arg_info)) = CUDA_HOST_SINGLE;
        }

        old_fundef = INFO_FUNDEF (arg_info);
        old_fromap = INFO_FROMAP (arg_info);
        INFO_FROMAP (arg_info) = TRUE;
        fundef = TRAVdo (fundef, arg_info);
        INFO_FROMAP (arg_info) = old_fromap;
        INFO_FUNDEF (arg_info) = old_fundef;
    } else {
        /* All other N_aps are immediately tagged as CUDA_HOST_SINGLE */
        ASSIGN_EXECMODE (INFO_LASTASSIGN (arg_info)) = CUDA_HOST_SINGLE;
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
    DBUG_ENTER ("CUTEMwith");

    /* Cudarizbale N_with is tagged as CUDA_DEVICE_MULTI */
    if (WITH_CUDARIZABLE (arg_node)) {
        ASSIGN_EXECMODE (INFO_LASTASSIGN (arg_info)) = CUDA_DEVICE_MULTI;
        INFO_HOST_SINGLE (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}
