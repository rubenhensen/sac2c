/******************************************************************************
 *
 * Need Count in With-Loop
 *
 * Prefix: WLNC
 *
 * This phase counts the number of _sel_VxA_ (iv, producerWL) references within
 * a consumerWL to the producerWL.
 *
 * The intent is to determine whether or not all references to the producerWL
 * are from the consumerWL. If so, the producerWL may be foldable into the
 * consumerWL. If not, no folding can occur.
 *
 ******************************************************************************/
#include "memory.h"
#include "pattern_match.h"
#include "print.h"
#include "str.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"

#define DBUG_PREFIX "WLNC"
#include "debug.h"

#include "wl_needcount.h"

/******************************************************************************
 *
 * @struct INFO
 *
 * @param INFO_WITH
 * @param INFO_FUN
 *
 ******************************************************************************/
struct INFO {
    node *with;
    node *fun;
};

#define INFO_WITH(n) ((n)->with)
#define INFO_FUN(n) ((n)->fun)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_WITH (result) = NULL;
    INFO_FUN (result) = NULL;

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
 * @fn node *WLNCdoWLNeedCount (node *arg_node)
 *
 * @brief Starting point of needcount inference.
 *
 ******************************************************************************/
node *
WLNCdoWLNeedCount (node *arg_node)
{
    info *info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef,
                 "WLNCdoWLNeedCount called for non-fundef node");

    info = MakeInfo ();

    TRAVpush (TR_wlnc);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn void incrementNeedcount (node *avis, info *arg_info)
 *
 * @brief Conditionally increments WL use count for the producerWL result.
 *
 ******************************************************************************/
static void
incrementNeedcount (node *avis, info *arg_info)
{

    DBUG_ENTER ();

    if (AVIS_COUNTING_WL (avis) == NULL
        || AVIS_COUNTING_WL (avis) == INFO_WITH (arg_info)) {
        AVIS_WL_NEEDCOUNT (avis) += 1;
        AVIS_COUNTING_WL (avis) = INFO_WITH (arg_info);

        DBUG_PRINT ("WLNCid incremented AVIS_WL_NEEDCOUNT (%s) = %d",
                    AVIS_NAME (avis), AVIS_WL_NEEDCOUNT (avis));
    } else {
        DBUG_PRINT ("incrementNeedCount (%s) reference from different WL",
                    AVIS_NAME (avis));
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * @fn node *WLNCfundef (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
WLNCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("WL-needcounting for %s %s begins",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                FUNDEF_NAME (arg_node));

    FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    DBUG_PRINT ("WL-needcounting for %s %s ends",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                FUNDEF_NAME (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *WLNCblock (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
WLNCblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    BLOCK_VARDECS (arg_node) = TRAVopt (BLOCK_VARDECS (arg_node), arg_info);
    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *WLNCavis (node *arg_node, info *arg_info)
 *
 * @brief Reset all counters. This works because we traverse the N_vardec
 * entries upon entry to each block.
 *
 ******************************************************************************/
node *
WLNCavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Zeroing AVIS_WL_NEEDCOUNT(%s)", AVIS_NAME (arg_node));
    AVIS_WL_NEEDCOUNT (arg_node) = 0;
    AVIS_COUNTING_WL (arg_node) = NULL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *WLNCwith (node *arg_node, info *arg_info)
 *
 * @brief Applies WLNC on a with loop.
 *
 ******************************************************************************/
node *
WLNCwith (node *arg_node, info *arg_info)
{
    node *avis;
    node *outer_with;

    DBUG_ENTER ();

    outer_with = INFO_WITH (arg_info);
    INFO_WITH (arg_info) = arg_node;

    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);

    if (N_modarray == NODE_TYPE (WITH_WITHOP (arg_node))) {
        avis = ID_AVIS (MODARRAY_ARRAY (WITH_WITHOP (arg_node)));
        incrementNeedcount (avis, arg_info);
    }

    INFO_WITH (arg_info) = outer_with;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *WLNCpart (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
WLNCpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);
    PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);
    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *WLNCcode (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
WLNCcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *WLNCprf (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
WLNCprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_FUN (arg_info) = arg_node;
    PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
    INFO_FUN (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *WLNCap (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
WLNCap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_FUN (arg_info) = arg_node;
    AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);
    INFO_FUN (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *WLNCid (node *arg_node, info *arg_info)
 *
 * @brief Count sel() references in WLs for one WL only.
 * The AVIS_COUNTING_WL code is to ensure that only one consumerWL contributes
 * to the count. That is the first-past-the-post WL: if more than one WL tries
 * to contribute, the latter ones will be ignored. This will cause a mismatch in
 * AWLF between AVIS_NEEDCOUNT and AVIS_WL_NEEDCOUNT, properly preventing the
 * folding from happening.
 *
 * The PM is intended to handle the -ecc/-check c case, where instead of:
 *
 *   PWL = with { ... };
 *   CWL = with { ...
 *           el = _sel_VxA_ (iv, PWL);
 *         };
 *
 * We have:
 *
 *   PWL = with { ... };
 *   PWL' = guard (PWL, predicates...);
 *   CWL = with { ...
 *           el = _sel_VxA_ (iv, PWL');
 *         };
 *
 ******************************************************************************/
node *
WLNCid (node *arg_node, info *arg_info)
{
    node *parent, *producerWL, *avis;
    pattern *pat;

    DBUG_ENTER ();

    parent = INFO_FUN (arg_info);

    if (parent != NULL && NODE_TYPE (parent) == N_prf) {
        switch (PRF_PRF (parent)) {
        case F_sel_VxA:
        case F_idx_sel:
            pat = PMvar (1, PMAgetNode (&producerWL), 0);
            if (PMmatchFlatSkipGuards (pat, PRF_ARG2 (parent))) {
                avis = ID_AVIS (producerWL);
                DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, parent));
                DBUG_PRINT ("WLNCid looking at %s.", AVIS_NAME (avis));

                if (avis == ID_AVIS (arg_node)) {
                    incrementNeedcount (avis, arg_info);
                }
            }

            pat = PMfree (pat);
            break;

        /**
         * Don't count these.
         */
        case F_idx_shape_sel:
        case F_shape_A:
        case F_saabind:
        case F_dim_A:
        case F_non_neg_val_V:
        case F_non_neg_val_S:
        case F_val_lt_shape_VxA:
        case F_val_le_val_VxV:
        case F_val_le_val_SxS:
        case F_shape_matches_dim_VxA:
        case F_guard:
        case F_noteintersect:
            break;

        default:
            break;
        }
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
