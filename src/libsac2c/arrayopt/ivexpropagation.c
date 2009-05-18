/*
 * $Id: ivexpropagation.c 15815 2008-10-24 18:04:47Z rbe $
 */

/** <!--********************************************************************-->
 *
 * @defgroup ivexp Index Vector Extrema Propagation Traversal
 *
 * @brief:
 *
 *  This code propagates extrema (AVIS_MINVAL and AVIS_MAXVAL) through
 *  assigns and, where we are able to do so, primitive functions.
 *
 *  Details:
 *     Assigns: RHS extrema are copied to LHS.
 *
 *     F_dataflowguard: PRF_ARG1's extrema are copied to result.
 *
 *
 * @ingroup ivexp
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file ivexpropagation.c
 *
 * Prefix: IVEXP
 *
 *****************************************************************************/
#include "ivexpropagation.h"

/*
 * Other includes go here
 */
#include "dbug.h"
#include "traverse.h"
#include "free.h"
#include "tree_basic.h"
#include "memory.h"
#include "new_types.h"
#include "constants.h"
#include "tree_compound.h"
#include "pattern_match.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    node *minval;
    node *maxval;
};

/**
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_MINVAL(n) ((n)->minval)
#define INFO_MAXVAL(n) ((n)->maxval)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_MINVAL (result) = NULL;
    INFO_MAXVAL (result) = NULL;

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
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *IVEXPdoIndexVectorExtremaProp( node *arg_node)
 *
 * @brief: Perform index vector extrema propagation on a module.
 *
 *****************************************************************************/
node *
IVEXPdoIndexVectorExtremaPropModule (node *arg_node)
{
    info *arg_info;
    DBUG_ENTER ("IVEXPdoIndexVectorExtremaPropModule");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module),
                 "IVEXPdoIndexVectorExtremaPropModule expected N_module");

    arg_info = MakeInfo ();

    DBUG_PRINT ("IVEXP", ("Starting index vector extrema propagation traversal."));

    TRAVpush (TR_ivexp);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    DBUG_PRINT ("IVEXP", ("Index vector extrema propagation complete."));

    arg_info = FreeInfo (arg_info);
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/******************************************************************************
 *
 * function:
 *   bool PrfExtrema( node *arg_node, info *arg_info)
 *
 * description: Identifies extrema within a primitive.
 *
 * @params  arg_node: an N_prf node.
 * @result: True if we have found extrema, and stored
 *          them in INFO_MINVAL and INFO_MAXVAL.
 *
 ******************************************************************************/
static bool
PrfExtrema (node *arg_node, info *arg_info)
{
    node *avis;
    bool z = FALSE;

    DBUG_ENTER ("PrfExtrema");
    DBUG_PRINT ("IVEXP", ("Found prf"));

    INFO_MINVAL (arg_info) = NULL;
    INFO_MAXVAL (arg_info) = NULL;

    switch (PRF_PRF (arg_node)) {
    case F_dataflowguard:
        avis = ID_AVIS (PRF_ARG1 (arg_node));
        INFO_MINVAL (arg_info) = AVIS_MINVAL (avis);
        INFO_MAXVAL (arg_info) = AVIS_MAXVAL (avis);
        z = TRUE;
        break;

    /* The following are ISMOP */
    case F_add_SxS:
    case F_add_SxV:
    case F_add_VxS:
    case F_add_VxV:
    case F_sub_SxS:
    case F_sub_SxV:
    case F_sub_VxS:
    case F_sub_VxV:
        DBUG_PRINT ("IVEXP", ("Missed an ISMOP N_prf"));
        break;

    case F_attachminmax:
        INFO_MINVAL (arg_info) = ID_AVIS (PRF_ARG2 (arg_node));
        INFO_MAXVAL (arg_info) = ID_AVIS (PRF_ARG3 (arg_node));
        z = (NULL != INFO_MINVAL (arg_info)) || (NULL != INFO_MAXVAL (arg_info));
        break;

    default:
        DBUG_PRINT ("IVEXP", ("Missed an N_prf"));
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/******************************************************************************
 *
 * function:
 *   node *IVEXPlet( node *arg_node, info *arg_info)
 *
 * description:
 *   Propagate iv extrema from RHS to LHS.
 *
 ******************************************************************************/
node *
IVEXPlet (node *arg_node, info *arg_info)
{
    node *rhs;
    node *lhsavis;
    node *rhsavis;

    DBUG_ENTER ("IVEXPlet");

    DBUG_PRINT ("IVEXP", ("Found let"));

    lhsavis = IDS_AVIS (LET_IDS (arg_node));
    rhs = LET_EXPR (arg_node);
    switch (NODE_TYPE (rhs)) {
    case N_id:
        rhsavis = ID_AVIS (rhs);
        if ((NULL != AVIS_MINVAL (rhsavis)) || (NULL != AVIS_MAXVAL (rhsavis))) {
            DBUG_PRINT ("IVEXP", ("IVEXP N_id: propagating extrema from %s to %s",
                                  AVIS_NAME (rhsavis), AVIS_NAME (lhsavis)));
            AVIS_MINVAL (lhsavis) = AVIS_MINVAL (rhsavis);
            AVIS_MAXVAL (lhsavis) = AVIS_MAXVAL (rhsavis);
        }
        break;

    case N_prf:
        if (PrfExtrema (rhs, arg_info)) {
            DBUG_PRINT ("IVEXP", ("IVEXP N_prf: propagating extrema to lhs %s",
                                  AVIS_NAME (lhsavis)));
            AVIS_MINVAL (lhsavis) = INFO_MINVAL (arg_info);
            AVIS_MAXVAL (lhsavis) = INFO_MAXVAL (arg_info);
        }
        break;

    /* Constant RHS */
    case N_bool:
    case N_char:
    case N_num:
    case N_float:
    case N_double:
    case N_array:
        if (TYisAKV (AVIS_TYPE (lhsavis))) {
            DBUG_PRINT ("IVEXP", ("IVEXP propagating constant extrema to lhs: %s",
                                  AVIS_NAME (lhsavis)));

#define CRUD
#ifdef CRUD

            /*
               See if this is the cause of CSE on constants no longer working.
               (nested.sac)
            */

            AVIS_MINVAL (lhsavis) = lhsavis;
            AVIS_MAXVAL (lhsavis) = lhsavis;
#endif // CRUD
        }
        break;

    /* We are unable to help these poor souls */
    case N_ap:

        break;

    case N_with:
        /* We have to descend into the depths here */
        WITH_PART (rhs) = TRAVdo (WITH_PART (rhs), arg_info);
        break;

    default:
        DBUG_PRINT ("IVEXP", ("IVEXP ISMOP: please fix this RHS for LHS: %s",
                              AVIS_NAME (lhsavis)));
        break;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXPwith( node *arg_node, info *arg_info)
 *
 * description: Into the depths
 *
 ******************************************************************************/
node *
IVEXPwith (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("IVEXPwith");

    DBUG_PRINT ("IVEXP", ("Found with"));

    arg_node = TRAVdo (WITH_PART (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXPpart( node *arg_node, info *arg_info)
 *
 * description: Into the depths
 *
 ******************************************************************************/
node *
IVEXPpart (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("IVEXPpart");

    DBUG_PRINT ("IVEXP", ("Found part"));

    PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);
    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXPcond( node *arg_node, info *arg_info)
 *
 * description: Into the depths
 *
 ******************************************************************************/
node *
IVEXPcond (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("IVEXPcond");

    DBUG_PRINT ("IVEXP", ("Found cond"));

    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXPfuncond( node *arg_node, info *arg_info)
 *
 * description: Into the depths
 *
 ******************************************************************************/
node *
IVEXPfuncond (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("IVEXPfuncond");

    DBUG_PRINT ("IVEXP", ("Found funcond"));

    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);
    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IVEXPwhile( node *arg_node, info *arg_info)
 *
 * description: Into the depths
 *
 ******************************************************************************/
node *
IVEXPwhile (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("IVEXPwhile");

    DBUG_PRINT ("IVEXP", ("Found while"));

    WHILE_COND (arg_node) = TRAVdo (WHILE_COND (arg_node), arg_info);
    WHILE_BODY (arg_node) = TRAVdo (WHILE_BODY (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}
