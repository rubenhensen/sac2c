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
 *     2015-02-25: Extended POGO to operate on some relationals:
 *
 *        x <  y
 *        x <= y
 *        x == y
 *        x >= y
 *        x >  y
 *        x != y
 *
 *      and also on _non_neg_val( x);
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
#include "tree_utils.h"
#include "sacpolylibisnullintersect.h"

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
    node *with;
};

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_WITH(n) ((n)->with)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_WITH (result) = NULL;

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

/** <!-- ****************************************************************** -->
 *
 * @fn prf CompanionFn( prf nprf)
 *
 * @brief
 *
 * @param An N_prf
 *
 * @return An N_prf
 *
 ******************************************************************************/
static prf
CompanionFn (prf nprf)
{
    prf z;

    DBUG_ENTER ();

    switch (nprf) {
    case F_lt_SxS:
        z = F_ge_SxS;
        break;
    case F_le_SxS:
        z = F_gt_SxS;
        break;
    // FIXME LATER case F_eq_SxS:         z = F_neq_SxS; break;
    case F_ge_SxS:
        z = F_lt_SxS;
        break;
    case F_gt_SxS:
        z = F_le_SxS;
        break;
    // FIXME LATER case F_neq_SxS:        z = F_eq_SxS;  break;
    case F_val_lt_val_SxS:
        z = F_ge_SxS;
        break;
    case F_val_le_val_SxS:
        z = F_gt_SxS;
        break;
    case F_non_neg_val_S:
        z = F_lt_SxS;
        break; // NB. Kludge (dyadic vs. monadic!)

    default:
        DBUG_ASSERT (FALSE, "Oopsie. Expected relational prf!");
        z = nprf;
        break;
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn bool GetABempty( prf nprf)
 *
 * @brief If
 *
 * @param An N_prf
 *
 * @return A Boolean, giving the relational result if the intersection
 *         of PRF_ARG1 and PRF_ARG2 is an empty set.
 *
 ******************************************************************************/
static bool
GetABempty (prf nprf)
{
    bool z;

    DBUG_ENTER ();

    switch (nprf) {
    case F_lt_SxS:
        z = FALSE;
        break;
    case F_le_SxS:
        z = TRUE;
        break;
        // FIXME LATER      case F_eq_SxS:         z = TRUE;  break;
    case F_ge_SxS:
        z = TRUE;
        break;
    case F_gt_SxS:
        z = FALSE;
        break;
        // FIXME LATER case F_neq_SxS:        z = FALSE; break;
    case F_val_lt_val_SxS:
        z = FALSE;
        break;
    case F_val_le_val_SxS:
        z = TRUE;
        break;
    case F_non_neg_val_S:
        z = FALSE;
        break; // NB. Kludge (dyadic vs. monadic!)

    default:
        DBUG_ASSERT (FALSE, "Oopsie. Expected relational prf!");
        z = FALSE;
        break;
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn bool POGOisPogoPrf( prf nprf)
 *
 * @brief Predicate for determining if N_prf is supported by POGO.
 *
 * @param An N_prf
 *
 * @return TRUE if nprf is supported by POGO; else FALSE;
 *
 ******************************************************************************/
bool
POGOisPogoPrf (prf nprf)
{
    bool z;

    DBUG_ENTER ();

    switch (nprf) {
    case F_lt_SxS:
    case F_le_SxS:
        // FIXME LATER    case F_eq_SxS:
    case F_ge_SxS:
    case F_gt_SxS:
        // FIXME LATER  case F_neq_SxS:
    case F_val_lt_val_SxS:
    case F_val_le_val_SxS:
    case F_non_neg_val_S:
        z = TRUE;
        break;

    default:
        z = FALSE;
        break;
    }

    DBUG_RETURN (z);
}

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
 * @fn node *POGOpart( node *arg_node, info *arg_info)
 *
 * @brief Mark the WITHID_VEC, WITHID_IDS, and WITHID_IDXS of this N_part
 *        with the N_part address when we enter, and NULL them on the way out.
 *
 *        These values are used by PHUT to locate GENERATOR_BOUND values
 *        for the WL variables. Those bounds are then used to create
 *        polyhedral inequalities for the WITHID variables.
 *
 *****************************************************************************/
node *
POGOpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    arg_node = PHUTsetClearAvisPart (arg_node, arg_node);

    CODE_CBLOCK (PART_CODE (arg_node))
      = TRAVopt (CODE_CBLOCK (PART_CODE (arg_node)), arg_info);
    arg_node = PHUTsetClearAvisPart (arg_node, NULL);

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *POGOwith( node *arg_node, info *arg_info)
 *
 * @brief Traverse with-loop partitions. Rebuild WITH_CODE
 *        We disconnect the current WITH_CODE chain.
 *
 *****************************************************************************/
node *
POGOwith (node *arg_node, info *arg_info)
{
    node *lastwith;

    DBUG_ENTER ();

    lastwith = INFO_WITH (arg_info);
    INFO_WITH (arg_info) = arg_node;

    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    WITH_CODE (arg_node) = TUremoveUnusedCodes (WITH_CODE (arg_node));
    INFO_WITH (arg_info) = lastwith;

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
 * @brief: If we can show that a relational or guard is always
 *         TRUE, remove it.
 *
 * @result: Possibly altered N_prf node.
 *
 * Algorithm: We build four maximal affine expression trees:
 *              (A, B) for both arguments.
 *              C for the N_prf relational function.
 *              D for the companion relational function.
 *
 *            We then invoke the polyhedral solver( e.g., PolyLib).
 *            If it can prove that A and B represent identical sets,
 *            it returns POLY_MATCH_AB. If not, it
 *            then performs set intersection on A,B,C,
 *            and A,B,D, generating POLY_EMPTYSET_ABC and/or POLY_EMPTYSET_ABD,
 *            if those intersections are empty.
 *
 *            Those results determine if we can optimize, as follows:
 *            First, based on matching, we obtain:
 *
 *            Relational  POLY_MATCH_AB
 *              x <  y      FALSE
 *              x <= y      TRUE
 *              x == y      TRUE
 *              x >= y      TRUE
 *              x >  y      FALSE
 *              x != y      FALSE
 *              non_neg_val FALSE (ignored for this check)
 *
 *            Next, if the result is still unknown, we look for empty
 *            set intersections:
 *
 *             If the intersection of A,B,C is empty, the result is FALSE; else unknown.
 *             If the intersection of A,B,D is empty, the result is TRUE; else unknown.
 *
 *****************************************************************************/
node *
POGOprf (node *arg_node, info *arg_info)
{
    node *exprs1 = NULL;
    node *exprs2 = NULL;
    node *exprs3 = NULL;
    node *exprs4 = NULL;
    node *idlist1 = NULL;
    node *idlist2 = NULL;
    int numvars = 0;
    bool z = FALSE;
    bool resval = FALSE;
    node *res;
    node *resp;
    node *guardp;
    bool dopoly;
    int emp = POLY_UNKNOWN;

    DBUG_ENTER ();

    res = arg_node;

    switch (PRF_PRF (arg_node)) {

    case F_lt_SxS:
    case F_le_SxS:
    // FIXME LATERcase F_eq_SxS:
    case F_ge_SxS:
    case F_gt_SxS:
    // FIXME LATERcase F_neq_SxS:
    case F_val_lt_val_SxS:
    case F_val_le_val_SxS:
        resval = GetABempty (PRF_PRF (arg_node));
        dopoly = TRUE;
        DBUG_PRINT ("Looking at dyadic N_prf for %s",
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
        idlist1 = TCappendExprs (idlist1, idlist2);
        break;

    case F_non_neg_val_S:
        dopoly = TRUE;
        DBUG_PRINT ("Looking at monadic N_prf for %s",
                    AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));

        PHUTclearColumnIndices (PRF_ARG1 (arg_node), INFO_FUNDEF (arg_info));
        idlist1
          = PHUTcollectAffineNids (PRF_ARG1 (arg_node), INFO_FUNDEF (arg_info), &numvars);
        exprs1 = PHUTgenerateAffineExprs (PRF_ARG1 (arg_node), INFO_FUNDEF (arg_info),
                                          &numvars);
        exprs2 = PHUTgenerateIdentityExprs (numvars);
        break;

    default:
        dopoly = FALSE;
        break;
    }

    if (dopoly) {
        // Don't bother calling Polylib if it can't do anything for us.
        if (NULL != idlist1) {
            exprs3 = PHUTgenerateAffineExprsForGuard (arg_node, INFO_FUNDEF (arg_info),
                                                      &numvars, PRF_PRF (arg_node));
            exprs4 = PHUTgenerateAffineExprsForGuard (arg_node, INFO_FUNDEF (arg_info),
                                                      &numvars,
                                                      CompanionFn (PRF_PRF (arg_node)));
            emp
              = PHUTcheckIntersection (exprs1, exprs2, exprs3, exprs4, idlist1, numvars);

            // Match analysis for A,B, but not for monadic prf.
            if ((emp & POLY_MATCH_AB) && (F_non_neg_val_S != PRF_PRF (arg_node))) {
                z = TRUE;
            }

            if ((!z) && (emp & POLY_EMPTYSET_ABC)) {
                resval = FALSE;
                z = TRUE;
            }

            if ((!z) && (emp & POLY_EMPTYSET_ABD)) {
                resval = TRUE;
                z = TRUE;
            }
        }

        idlist1 = (NULL != idlist1) ? FREEdoFreeTree (idlist1) : NULL;
        exprs1 = (NULL != exprs1) ? FREEdoFreeTree (exprs1) : NULL;
        exprs2 = (NULL != exprs2) ? FREEdoFreeTree (exprs2) : NULL;
        exprs3 = (NULL != exprs3) ? FREEdoFreeTree (exprs3) : NULL;
        exprs4 = (NULL != exprs4) ? FREEdoFreeTree (exprs4) : NULL;
        PHUTclearColumnIndices (PRF_ARG1 (arg_node), INFO_FUNDEF (arg_info));
        if (F_non_neg_val_S != PRF_PRF (arg_node)) { // Ignore monadic fns
            PHUTclearColumnIndices (PRF_ARG2 (arg_node), INFO_FUNDEF (arg_info));
        }

        if (z) { // guard/primitive can be removed
            DBUG_PRINT ("Guard/relational for result %s removed",
                        AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
            resp = TBmakeBool (resval);

            if (TUisPrfGuard (arg_node)) { // Need two results
                res = DUPdoDupNode (PRF_ARG1 (arg_node));
                arg_node = FREEdoFreeNode (arg_node);
                guardp = IDS_NEXT (INFO_LHS (arg_info));
                resp = TBmakeAssign (TBmakeLet (guardp, resp), NULL);
                AVIS_SSAASSIGN (IDS_AVIS (guardp)) = resp;
                IDS_NEXT (INFO_LHS (arg_info)) = NULL;
                INFO_PREASSIGNS (arg_info)
                  = TCappendAssign (INFO_PREASSIGNS (arg_info), resp);
            } else {
                res = resp;
                arg_node = FREEdoFreeNode (arg_node);
            }
        } else {
            DBUG_PRINT ("Unable to remove guard/primitive for result %s",
                        AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
        }
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
