/** <!--********************************************************************-->
 *
 * @defgroup ctzg
 *
 *   FIXME: Rename this traversal, as the name
 *   no longer bears any resemblance to its purpose.
 *
 *   This module searches for selected guards, adds an extra argument,
 *   which will hopefully be reduced to a Boolean result.
 *
 *   For example:
 *
 *     iv  = q + [4];
 *     iv2 = q + [10];
 *     iv', p = _val_lt_val_SxS_( iv, iv2);
 *
 *   This can not be optimized any further, and we must not
 *   play the subtract-iv-from-both-arguments trick, because
 *   we need to preserve PRF_ARG1.
 *
 *   So, we add a third argument:
 *
 *     arg3 = iv < iv2;
 *     iv', p = _val_lt_val_SxS_( iv, iv2, arg3);
 *
 *   CF will later examine PRF_ARG3 and optimize the guard away
 *   if PRF_ARG3 is true.
 *
 * @ingroup opt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file comparison_to_zero_guards.c
 *
 * Prefix: CTZG
 *
 *****************************************************************************/
#include "comparison_to_zero_guards.h"

#define DBUG_PREFIX "CTZG"
#include "debug.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "memory.h"
#include "traverse.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "constants.h"
#include "type_utils.h"
#include "flattengenerators.h"
#include "free.h"
#include "shape.h"
#include "DupTree.h"
#include "ivexpropagation.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/

struct INFO {
    bool onefundef;
    node *fundef;
    node *preassigns;
    node *vardecs;
    node *let;
};

#define INFO_ONEFUNDEF(n) ((n)->onefundef)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_LET(n) ((n)->let)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_ONEFUNDEF (result) = TRUE;
    INFO_FUNDEF (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_LET (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

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
 * @fn node *CTZGdoComparisonToZeroGuards( node *arg_node)
 *
 *****************************************************************************/

node *
CTZGdoComparisonToZeroGuards (node *arg_node)
{
    info *info;
    DBUG_ENTER ();

    info = MakeInfo ();

    INFO_ONEFUNDEF (info) = TRUE;

    TRAVpush (TR_ctzg);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn static bool IsSuitableGuard( prf op)
 *
 * @brief Predicate for whether the prf is one of the guards we want to
 *        rewrite.
 *
 * @param op PRF_PRF
 *
 * @return TRUE if op is member of the desired set.
 *
 *****************************************************************************/
static bool
IsSuitableGuard (prf op)
{
    bool z;

    DBUG_ENTER ();

    z = (op == F_val_lt_val_SxS);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn static prf GetRelationalPrimitive( prf op)
 *
 * @brief This function returns the relational primitive to use
 *        with the rewritten guard.
 *
 * @param prf guard
 *
 * @return relational prf
 *
 *****************************************************************************/
static prf
GetRelationalPrimitive (prf op)
{
    prf result;

    DBUG_ENTER ();

    switch (op) {
    case F_val_lt_val_SxS:
        result = F_lt_SxS;
        break;

#ifdef DEADORNOTYETALIVE
    case F_val_le_val_VxV:
        result = F_le_VxV;
        break;

    case F_val_le_val_SxS:
        result = F_le_SxS;
        break;
#endif // DEADORNOTYETALIVE

    default:
        DBUG_ASSERT (0, "Illegal argument.");
        result = F_unknown;
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *CTZGfundef(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
CTZGfundef (node *arg_node, info *arg_info)
{
    bool old_onefundef;

    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;

    DBUG_PRINT ("traversing body of (%s) %s",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                FUNDEF_NAME (arg_node));

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    /* If new vardecs were made, append them to the current set */
    if (INFO_VARDECS (arg_info) != NULL) {
        BLOCK_VARDECS (FUNDEF_BODY (arg_node))
          = TCappendVardec (INFO_VARDECS (arg_info),
                            BLOCK_VARDECS (FUNDEF_BODY (arg_node)));
        INFO_VARDECS (arg_info) = NULL;
    }

    old_onefundef = INFO_ONEFUNDEF (arg_info);
    INFO_ONEFUNDEF (arg_info) = FALSE;
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    INFO_ONEFUNDEF (arg_info) = old_onefundef;

    if (!INFO_ONEFUNDEF (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CTZGassign(node *arg_node, info *arg_info)
 *
 * @brief Traverses into instructions and inserts new assignments.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
CTZGassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);
    if (INFO_PREASSIGNS (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CTZGlet(node *arg_node, info *arg_info)
 *
 * @brief Traverse into the LET_EXPR.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return Possibly updated N_let node.
 *
 *****************************************************************************/
node *
CTZGlet (node *arg_node, info *arg_info)
{
    node *oldlet;

    DBUG_ENTER ();

    oldlet = INFO_LET (arg_info);
    INFO_LET (arg_info) = arg_node;
    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);
    INFO_LET (arg_info) = oldlet;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CTZGprf(node *arg_node, info *arg_info)
 *
 * @brief This function looks for suitable guards, applies the
 *        optimization.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
CTZGprf (node *arg_node, info *arg_info)
{
    node *relop;
    node *relopavis;
    node *passign;
    node *pavis;

    DBUG_ENTER ();

    DBUG_PRINT ("Looking at prf for %s",
                AVIS_NAME (IDS_AVIS (LET_IDS (INFO_LET (arg_info)))));

    if ((IsSuitableGuard (PRF_PRF (arg_node))) && (NULL == PRF_EXPRS3 (arg_node))) {
        DBUG_PRINT ("Appending to guard on %s",
                    AVIS_NAME (IDS_AVIS (LET_IDS (INFO_LET (arg_info)))));

        relop = TBmakePrf (GetRelationalPrimitive (PRF_PRF (arg_node)),
                           TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                                        TBmakeExprs (DUPdoDupNode (PRF_ARG2 (arg_node)),
                                                     NULL)));

        relopavis = FLATGexpression2Avis (relop, &INFO_VARDECS (arg_info),
                                          &INFO_PREASSIGNS (arg_info),
                                          TYmakeAKS (TYmakeSimpleType (T_bool),
                                                     SHcreateShape (0)));

#ifdef DEADCODE
        pavis = IDS_AVIS (IDS_NEXT (LET_IDS (INFO_LET (arg_info))));
        passign = TBmakeAssign (TBmakeLet (TBmakeIds (pavis, NULL), TBmakeId (relopavis)),
                                NULL);
        AVIS_SSAASSIGN (pavis) = passign;
        INFO_PREASSIGNS (arg_info) = TCappendAssign (INFO_PREASSIGNS (arg_info), passign);
#endif // DEADCODE

        PRF_ARGS (arg_node)
          = TCappendExprs (PRF_ARGS (arg_node), TBmakeExprs (TBmakeId (relopavis), NULL));

        global.optcounters.ctzg_expr += 1;
    }

    DBUG_PRINT ("Leaving prf");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Comparison Against Zero For Guards -->
 *****************************************************************************/

#undef DBUG_PREFIX
