/*
 * $Id$
 *
 * This module sums, into AVIS_NEEDCOUNT, the total number of N_id nodes
 * that refer to that N_avis.
 * An argument to INFNCdoInferNeedCountersOneFundef permits counting
 * of only those N_id nodes that refer to the data part of
 * an array. At present, this ignores references in _idx_shape_sel ops.
 * This permits symbolic WLF to count only
 * array references, and ignore shape and dim references.
 *
 */

#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "str.h"
#include "memory.h"

#include "inferneedcounters.h"

/*
 * INFO structure
 */
struct INFO {
    node *prf;
    bool onefundef;
    bool dro; /* data reference only:
               * If true, ignore references to array shape or dim.
               */
};

/*
 * INFO macros
 */
#define INFO_PRF(n) ((n)->prf)
#define INFO_ONEFUNDEF(n) ((n)->onefundef)
#define INFO_DRO(n) ((n)->dro)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_PRF (result) = NULL;
    INFO_ONEFUNDEF (result) = FALSE;
    INFO_DRO (result) = FALSE;

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
 * @fn node *INFNCdoInferNeedCounters( node *arg_node)
 *
 * @brief starting point of needcount inference
 *
 * @param arg_node
 *
 * @return
 *
 *****************************************************************************/
node *
INFNCdoInferNeedCounters (node *arg_node)
{
    info *info;

    DBUG_ENTER ("INFNCdoInferNeedCounters");

    info = MakeInfo ();

    TRAVpush (TR_infnc);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *INFNCdoInferNeedCountersOneFundef( node *arg_node, bool dro)
 *
 * @brief starting point of needcount inference. Traverses one function only
 *
 * @param arg_node - the N_fundef to be traversed
 *        dro - Data References Only:  if TRUE, N_id nodes
 *              that only reference the shape or dim of the array are
 *              ignored.
 *
 * @return
 *
 *****************************************************************************/
node *
INFNCdoInferNeedCountersOneFundef (node *arg_node, bool dro)
{
    info *info;

    DBUG_ENTER ("INFNCdoInferNeedCountersOneFundef");

    info = MakeInfo ();
    INFO_ONEFUNDEF (info) = TRUE;
    INFO_DRO (info) = dro;

    TRAVpush (TR_infnc);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Needcount inference traversal (inc_tab)
 *
 * prefix: INFNC
 *
 *****************************************************************************/
node *
INFNCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("INFNCfundef");

    FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    if (!INFO_ONEFUNDEF (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
INFNCblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("INFNCblock");

    BLOCK_VARDEC (arg_node) = TRAVopt (BLOCK_VARDEC (arg_node), arg_info);

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
INFNCavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("INFNCavis");

    AVIS_NEEDCOUNT (arg_node) = 0;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * N_id Needcount inference traversal
 *
 * @fn node *INFNCid( node *arg_node, info *arg_info)
 *
 * @brief Count the N_id nodes that refer to the data part
 *        of the N_id.
 *        W ignore idx_shape_sel, dim, and shape, as they do
 *        not refer to the data part. We treat saabind the same
 *        way, for the same reason.
 *
 * @param arg_node
 *
 * @return
 *
 *****************************************************************************/
node *
INFNCid (node *arg_node, info *arg_info)
{
    node *avis;
    node *parent;

    DBUG_ENTER ("INFNCid");

    avis = ID_AVIS (arg_node);
    parent = INFO_PRF (arg_info);

    if ((parent != NULL) && (NODE_TYPE (parent) == N_prf)) {

        switch
            PRF_PRF (parent)
            {
            case F_idx_shape_sel: /* Don't count these */
            case F_shape_A:
            case F_saabind:
            case F_dim_A:
                break;
            default:
                AVIS_NEEDCOUNT (avis) += 1;
            }
    } else {
        AVIS_NEEDCOUNT (avis) += 1;
    }

    AVIS_DIM (avis) = TRAVopt (AVIS_DIM (avis), arg_info);

    AVIS_SHAPE (avis) = TRAVopt (AVIS_SHAPE (avis), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *INFNCprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
INFNCprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("INFNCprf");

    INFO_PRF (arg_info) = arg_node;

    if (PRF_ARGS (arg_node) != NULL) {
        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
    }

    INFO_PRF (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}
