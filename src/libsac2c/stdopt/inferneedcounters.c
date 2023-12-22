/******************************************************************************
 *
 * This module infers how often each variable is used in RHS expressions and
 * puts the value found into each variable's AVIS_NEEDCOUNT. This information,
 * IN PRINCIPLE, is needed by several optimisations to base the decisions on.
 * Amongst these are, at this time (2009):
 *   AL, DL, SISI, AWLF, and WLPROP.
 *
 * Since some of these optimisations actually would prefer to exclude certain
 * RHS occurances, the entry function takes a further parameter, the traversal
 * table TR_xx itself!
 *
 * The actual decision as to whether exclude or not a certain occurence happens
 * in the function ExclusionDueToHostTraversal.
 *
 * This eases reverse engineering if needed :-)
 *
 ******************************************************************************/
#include "memory.h"
#include "str.h"
#include "traverse.h"
#include "tree_basic.h"

#define DBUG_PREFIX "INFNC"
#include "debug.h"

#include "inferneedcounters.h"

/******************************************************************************
 *
 * @struct INFO
 *
 * @param INFO_PRF
 * @param INFO_ONEFUNDEF
 * @param INFO_TRAV
 *
 ******************************************************************************/
struct INFO {
    node *prf;
    bool onefundef;
    trav_t traversal;
};

#define INFO_PRF(n) ((n)->prf)
#define INFO_ONEFUNDEF(n) ((n)->onefundef)
#define INFO_TRAV(n) ((n)->traversal)

static info *
MakeInfo (trav_t trav)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_PRF (result) = NULL;
    INFO_ONEFUNDEF (result) = FALSE;
    INFO_TRAV (result) = trav;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * @fn bool ExclusionDueToHostTraversal (node *arg_node, info* arg_info)
 *
 * @brief Don't count references to array descriptors; only count
 * array value references, IFF in AWLF.
 *
 * @return TRUE if reference should not be counted.
 *
 ******************************************************************************/
static bool
ExclusionDueToHostTraversal (node *arg_node, info *arg_info)
{
    node *parent;
    bool res = FALSE;

    DBUG_ENTER ();

    if (INFO_TRAV (arg_info) == TR_awlfi) {
        parent = INFO_PRF (arg_info);
        if (parent != NULL && NODE_TYPE (parent) == N_prf) {
            switch PRF_PRF (parent) {
            /**
             * Don't count these.
             */
            case F_idx_shape_sel:
            case F_shape_A:
            case F_saabind:
            case F_dim_A:
            case F_non_neg_val_V:
            case F_val_lt_shape_VxA:
            case F_val_le_val_VxV:
            case F_shape_matches_dim_VxA:
            case F_guard:
            case F_noteintersect:
                res = TRUE;
                break;

            default:
                res = FALSE;
                break;
            }
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn node *INFNCdoInferNeedCounters (node *arg_node, trav_t trav)
 *
 * @brief Starting point of needcount inference.
 *
 ******************************************************************************/
node *
INFNCdoInferNeedCounters (node *arg_node, trav_t trav)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo (trav);

    TRAVpush (TR_infnc);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *INFNCdoInferNeedCountersOneFundef (node *arg_node, trav_t trav)
 *
 * @brief Starting point of needcount inference. Traverses one function only.
 *
 ******************************************************************************/
node *
INFNCdoInferNeedCountersOneFundef (node *arg_node, trav_t trav)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo (trav);
    INFO_ONEFUNDEF (info) = TRUE;

    TRAVpush (TR_infnc);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

node *
INFNCfundef (node *arg_node, info *arg_info)
{
    bool old_onefundef;

    DBUG_ENTER ();

    FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    old_onefundef = INFO_ONEFUNDEF (arg_info);
    INFO_ONEFUNDEF (arg_info) = FALSE;
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    INFO_ONEFUNDEF (arg_info) = old_onefundef;

    if (!INFO_ONEFUNDEF (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
INFNCblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    BLOCK_VARDECS (arg_node) = TRAVopt (BLOCK_VARDECS (arg_node), arg_info);
    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
INFNCavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    AVIS_NEEDCOUNT (arg_node) = 0;

    // Traverse AVIS_DIM, etc.
    arg_node = TRAVsons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *INFNCids (node *arg_node, info *arg_info)
 *
 * @brief We get here from the LHS of an N_assign, such as a WLPROP WL. We have
 * to traverse the AVIS_DIM/SHAPE/MIN/MAX of the LHS.
 *
 ******************************************************************************/
node *
INFNCids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Looking at N_ids %s", AVIS_NAME (IDS_AVIS (arg_node)));
    IDS_AVIS (arg_node) = TRAVdo (IDS_AVIS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *INFNCid (node *arg_node, info *arg_info)
 *
 * @brief Count the N_id nodes that refer to the data part of the N_id. We
 * ignore idx_shape_sel, dim, and shape, as they do not refer to the data part.
 * We treat saabind the same way, for the same reason.
 *
 ******************************************************************************/
node *
INFNCid (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ();

    avis = ID_AVIS (arg_node);
    DBUG_PRINT ("Looking at N_id %s", AVIS_NAME (avis));

    if (!ExclusionDueToHostTraversal (arg_node, arg_info)) {
        AVIS_NEEDCOUNT (avis) += 1;
        DBUG_PRINT ("Increasing %s AVIS_NEEDCOUNT to %d",
                    AVIS_NAME (avis), AVIS_NEEDCOUNT (avis));
    }

    DBUG_RETURN (arg_node);
}

node *
INFNCprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_PRF (arg_info) = arg_node;
    PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
    INFO_PRF (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
