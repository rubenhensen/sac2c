/*
 *
 * $Id$
 */

/**
 *
 * @file Polyhedral Guard Optimization
 *
 * This traversal is intended to remove, when possible, guard
 * instructions, based on a polyhedral analysis of the guard
 * arguments. An example of such a guard can be seen in the
 * AWLF unit test time2code.sac. If compiled with -doawlf -nowlf,
 * it generates this code:
 *
 *     p = _aplmod_SxS_( -1, m);   NB. AVIS_MIN(p) = 0;
 *     iv2 = _notemaxval( iv1, m);
 *     iv3 = iv2 - p;
 *     iv4, p = _val_lt_val_SxS_( iv3, m);
 *
 * The code in AL/DL/AS/GGS is unable to solve this one.
 *
 * NB. This traversal runs within SAACYC, because PRFUNR and friends
 *     may create new guards on the fly.
 *
 * NB. In removing guards, we want to be sure that we have
 *     extrema on the guard result, based on the guard itself.
 *     For that reason, I moved the POGO traversal to follow
 *     IVEXP. I hope that is adequate. Before that change,
 *     the AWLF unit test nakedConsumerAndSumAKD.sac would fail to
 *     fold the naked consumer if -dopogo was active.
 *
 *     A better approach would be to make POGO operate only
 *     on nodes where the lhs (e.g., iv4 in the above example) has
 *     its appropriate extrema present.
 *
 */

#define DBUG_PREFIX "POGO"
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
#include "polyhedral_guard_optimization.h"
#include "polyhedral_utilities.h"
#include "print.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    node *lhs;
    node *preassigns;
};

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_PREASSIGNS(n) ((n)->preassigns)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;

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

/** <!--********************************************************************-->
 *
 * @name Static helper functions
 * @{
 *
 *****************************************************************************/
#ifdef CRUD
/** <!--*******************************************************************-->
 *
 * @fn node *CollectAffineExprs( node *arg_node, info *arg_info)
 *
 *
 *****************************************************************************/
static node *
CollectAffineExprs (node *arg_node, info *arg_info)
{
    node *exprs = NULL;

    DBUG_ENTER ();

    DBUG_RETURN (exprs);
}

#endif // CRUD

/** <!--*******************************************************************-->
 *
 * @fn node *POGOfundef( node *arg_node, info *arg_info)
 *
 * @brief Traverse this function only.
 *
 *****************************************************************************/
node *
POGOfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Starting to traverse %s %s",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                FUNDEF_NAME (arg_node));
    INFO_FUNDEF (arg_info) = arg_node;

    if (!FUNDEF_ISWRAPPERFUN (arg_node)) {
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_PRINT ("leaving function %s", FUNDEF_NAME (arg_node));

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    INFO_FUNDEF (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *POGOassign( node *arg_node, info *arg_info)
 *
 *
 *****************************************************************************/
node *
POGOassign (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_PREASSIGNS (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *POGOlet( node *arg_node, info *arg_info)
 *
 *
 *****************************************************************************/
node *
POGOlet (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    INFO_LHS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *POGOprf( node *arg_node, info *arg_info)
 *
 * @brief:
 *
 *
 * @result: Possibly altered N_prf node.
 *
 *****************************************************************************/
node *
POGOprf (node *arg_node, info *arg_info)
{
    node *exprs1 = NULL;
    node *exprs2 = NULL;
    node *exprs3 = NULL;
    node *idlist1 = NULL;
    node *idlist2 = NULL;
    int numvars = 0;
    bool z = FALSE;
    node *res;
    node *resp;
    node *guardp;

    DBUG_ENTER ();

    res = arg_node;

    switch (PRF_PRF (arg_node)) {

    case F_val_lt_val_SxS:
    case F_val_le_val_SxS:
        DBUG_PRINT ("Looking at N_prf for %s",
                    AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));

        PHUTclearColumnIndices (PRF_ARG1 (arg_node), INFO_FUNDEF (arg_info));
        PHUTclearColumnIndices (PRF_ARG2 (arg_node), INFO_FUNDEF (arg_info));

        idlist1
          = PHUTcollectAffineNids (PRF_ARG1 (arg_node), INFO_FUNDEF (arg_info), &numvars);

        idlist2
          = PHUTcollectAffineNids (PRF_ARG2 (arg_node), INFO_FUNDEF (arg_info), &numvars);

        exprs1 = PHUTgenerateAffineExprs (PRF_ARG1 (arg_node), INFO_FUNDEF (arg_info),
                                          &numvars);
        exprs2 = PHUTgenerateAffineExprs (PRF_ARG2 (arg_node), INFO_FUNDEF (arg_info),
                                          &numvars);
        exprs3
          = PHUTgenerateAffineExprsForGuard (arg_node, INFO_FUNDEF (arg_info), &numvars);

        idlist1 = TCappendExprs (idlist1, idlist2);
        exprs1 = TCappendExprs (exprs1, exprs2);

        // Don't bother calling Polylib if it can't do anything for us.
        z = (NULL != exprs1) && (NULL != exprs3) && (NULL != idlist1);
        z = z && PHUTcheckIntersection (exprs1, exprs3, idlist1);

        PHUTclearColumnIndices (PRF_ARG1 (arg_node), INFO_FUNDEF (arg_info));
        PHUTclearColumnIndices (PRF_ARG2 (arg_node), INFO_FUNDEF (arg_info));

        idlist1 = (NULL != idlist1) ? FREEdoFreeTree (idlist1) : NULL;
        exprs1 = (NULL != exprs1) ? FREEdoFreeTree (exprs1) : NULL;
        exprs3 = (NULL != exprs3) ? FREEdoFreeTree (exprs3) : NULL;
        if (z) { // guard can be removed
            DBUG_PRINT ("Guard for %s removed",
                        AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
            res = DUPdoDupNode (PRF_ARG1 (arg_node));
            resp = TBmakeBool (TRUE);
            arg_node = FREEdoFreeNode (arg_node);

            guardp = IDS_NEXT (INFO_LHS (arg_info));
            resp = TBmakeAssign (TBmakeLet (guardp, resp), NULL);
            AVIS_SSAASSIGN (IDS_AVIS (guardp)) = resp;
            IDS_NEXT (INFO_LHS (arg_info)) = NULL;
            INFO_PREASSIGNS (arg_info)
              = TCappendAssign (INFO_PREASSIGNS (arg_info), resp);
        } else {
            DBUG_PRINT ("Unable to remove guard for %s",
                        AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
        }
        break;

    default:
        break;
    }

    res = TRAVcont (res, arg_info);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *POGOdoPolyhedralGuardOptimization( node *arg_node)
 *
 *   @brief
 *   @param arg_node
 *   @return modified AST.
 *
 *****************************************************************************/
node *
POGOdoPolyhedralGuardOptimization (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    TRAVpush (TR_pogo);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
