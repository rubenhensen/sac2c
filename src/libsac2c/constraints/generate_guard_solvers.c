/*
 *
 * $Id$
 */

/**
 *
 * @file generate_guard_solvers.c
 *
 * This traversal is intended to enhance the optimization of certain
 * guard expressions, by generating an additional N_prf argument
 * that contains the other arguments in a form that is solvable by
 * CF/AL/AS/DL, etc.
 *
 * The motivation for this traversal is this:
 *
 *    x = id( 42);
 *    y = x + 1;
 *    x',p = _val_lt_val_SxS_( x, y);
 *    z = x' >= 0;
 *
 * Symbolically, we want to solve:
 *
 *    z = x < y;
 *
 * However, we must not apply a rewrite trick such as:
 *
 *
 *   x',p = _val_lt_val_SxS_( x-x, y-x);
 *
 * because we have to preserve the value of PRF_ARG1 for x'.
 *
 * Hence, this traversal generates PRF_ARG3, as follows:
 *
 *
 *   tmp = y - x;
 *   x',p = _val_lt_val_SxS_( x, y, tmp);
 *
 * This is solvable, so if tmp ever becomes AKV, CF
 * or TC can eliminate the guard (if tmp is TRUE), and complain
 * (if tmp is FALSE).
 *
 * The cleanup traversal eliminates PRF_ARG3 at the end of optimization.
 *
 * NB. This traversal runs within SAACYC, because PRFUNR and friends
 *     may create new guards on the fly.
 *
 */

#define DBUG_PREFIX "GGS"
#include "debug.h"

#include "types.h"
#include "DupTree.h"
#include "free.h"
#include "new_types.h"
#include "constants.h"
#include "globals.h"
#include "memory.h"
#include "shape.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "flattengenerators.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    node *vardecs;
    node *preassigns;
    bool generate;
};

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_GENERATE(n) ((n)->generate)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_GENERATE (result) = FALSE;

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

/** <!--*******************************************************************-->
 *
 * @fn void *
 *
 *
 *****************************************************************************/

/** <!--*******************************************************************-->
 *
 * @fn node *GGSfundef( node *arg_node, info *arg_info)
 *
 * @brief Traverse this function only.
 *
 *****************************************************************************/
node *
GGSfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Starting to traverse %s %s",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                FUNDEF_NAME (arg_node));
    INFO_FUNDEF (arg_info) = arg_node;

    if (!FUNDEF_ISWRAPPERFUN (arg_node)) {
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

        if (NULL != INFO_VARDECS (arg_info)) {
            FUNDEF_VARDECS (arg_node)
              = TCappendVardec (INFO_VARDECS (arg_info), FUNDEF_VARDECS (arg_node));
            INFO_VARDECS (arg_info) = NULL;
        }
    }

    DBUG_PRINT ("leaving function %s", FUNDEF_NAME (arg_node));

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    INFO_FUNDEF (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *GGSassign( node *arg_node, info *arg_info)
 *
 * @note
 *
 *****************************************************************************/
node *
GGSassign (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (NULL != INFO_PREASSIGNS (arg_info)) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *GGSprf( node *arg_node, info *arg_info)
 *
 * @brief: If this is one of the guards we want to enhance
 *         (or deenhance), do it here.
 *
 * @result: Possibly altered N_prf node.
 *
 *****************************************************************************/
node *
GGSprf (node *arg_node, info *arg_info)
{
    node *avis;
    prf nprf;

    DBUG_ENTER ();

    switch (PRF_PRF (arg_node)) {

    case F_val_lt_val_SxS:
        nprf = F_lt_SxS;
        break;

    case F_val_le_val_SxS:
        nprf = F_le_SxS;
        break;

    default:
        nprf = F_unknown;
        break;
    }

    if ((F_unknown != nprf) && (INFO_GENERATE (arg_info))
        && (NULL == PRF_EXPRS3 (arg_node))) {
        // generate PRF_ARG3
        avis
          = FLATGflattenExpression (TCmakePrf2 (nprf, DUPdoDupNode (PRF_ARG1 (arg_node)),
                                                DUPdoDupNode (PRF_ARG2 (arg_node))),
                                    &INFO_VARDECS (arg_info), &INFO_PREASSIGNS (arg_info),
                                    NULL);
        PRF_ARGS (arg_node)
          = TCappendExprs (PRF_ARGS (arg_node), TBmakeExprs (TBmakeId (avis), NULL));
    }

    if ((F_unknown != nprf) && (!INFO_GENERATE (arg_info))
        && (NULL != PRF_EXPRS3 (arg_node))) {
        // Remove PRF_ARG3
        PRF_EXPRS3 (arg_node) = FREEdoFreeNode (PRF_ARG3 (arg_node));
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *GGSdoGenerateGuardSolvers( node *arg_node)
 *
 *   @brief
 *   @param arg_node
 *   @return modified AST.
 *
 *****************************************************************************/
node *
GGSdoGenerateGuardSolvers (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();
    INFO_GENERATE (arg_info) = TRUE;

    TRAVpush (TR_ggs);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *GGSdoRemoveGuardSolvers( node *arg_node)
 *
 *   @brief
 *   @param arg_node
 *   @return modified AST.
 *
 *****************************************************************************/
node *
GGSdoRemoveGuardSolvers (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();
    INFO_GENERATE (arg_info) = FALSE;

    TRAVpush (TR_ggs);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}
#undef DBUG_PREFIX
