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
 * FIXME: BODO WANTS annotation of above, and how ISL (POLYSTUFF)
 * can solve this.
 *
 *
 #ifdef DEADCODE
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
#endif // DEADCODE
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

int FIXME; // See Bodo above

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
#include "polyhedral_defs.h"
#include "polyhedral_setup.h"
#include "print.h"
#include "tree_utils.h"
#include "LookUpTable.h"
#include "lacfun_utilities.h"
#include "symbolic_constant_simplification.h"
#include "with_loop_utilities.h"

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
    lut_t *varlut;
    node *nassign;
    node *lacfun;
    node *lacfunprf;
};

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_WITH(n) ((n)->with)
#define INFO_VARLUT(n) ((n)->varlut)
#define INFO_NASSIGN(n) ((n)->nassign)
#define INFO_LACFUN(n) ((n)->lacfun)
#define INFO_LACFUNPRF(n) ((n)->lacfunprf)

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
    INFO_VARLUT (result) = NULL;
    INFO_NASSIGN (result) = NULL;
    INFO_LACFUN (result) = NULL;
    INFO_LACFUNPRF (result) = NULL;

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
 * @fn bool GetXYmatch( prf nprf)
 *
 * @brief A Boolean, giving the relational result if the arguments match
 * @param An N_prf
 *
 * @return A Boolean
 *
 ******************************************************************************/
static bool
GetXYmatch (prf nprf)
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
    case F_eq_SxS:
        z = TRUE;
        break;
    case F_ge_SxS:
        z = TRUE;
        break;
    case F_gt_SxS:
        z = FALSE;
        break;
    case F_neq_SxS:
        z = FALSE;
        break;
    case F_val_lt_val_SxS:
        z = FALSE;
        break;
    case F_val_le_val_SxS:
        z = TRUE;
        break;
    case F_non_neg_val_S:
        z = TRUE;
        break;

    default:
        DBUG_ASSERT (FALSE, "Oopsie. Expected relational prf!");
        z = FALSE;
        break;
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn prf POGOmapPrf( prf nprf)
 *
 * @brief Map guard N_prfs to relationals.
 *
 * @param An N_prf
 *
 * @return An N_prf.
 *
 ******************************************************************************/
static prf
POGOmapPrf (prf nprf)
{
    prf z;

    DBUG_ENTER ();

    z = nprf;
    switch (nprf) {
    case F_val_lt_val_SxS:
        z = F_lt_SxS;
        break;

    case F_val_le_val_SxS:
        z = F_le_SxS;
        break;

    default:
        z = nprf;
        break;
    }

    DBUG_RETURN (z);
}

/** <!--*******************************************************************-->
 *
 * @fn node *POGOfundef( node *arg_node, info *arg_info)
 *
 * @brief Traverse this function only. Do not traverse any LACFUN, unless
 *        we come via POGOap.
 *
 *****************************************************************************/
node *
POGOfundef (node *arg_node, info *arg_info)
{
    node *fundefold;
    node *lacfunprfold;
    node *lacfunprf;

    DBUG_ENTER ();

    fundefold = INFO_FUNDEF (arg_info);
    INFO_FUNDEF (arg_info) = arg_node;
    lacfunprfold = INFO_LACFUNPRF (arg_info);
    INFO_LACFUNPRF (arg_info) = NULL;

    if (!FUNDEF_ISWRAPPERFUN (arg_node)) {
        if ((!FUNDEF_ISLACFUN (arg_node)) || (arg_node == INFO_LACFUN (arg_info))) {
            /* Vanilla traversal or LACFUN via POGOap */
            DBUG_PRINT ("Starting to traverse %s %s",
                        (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                        FUNDEF_NAME (arg_node));
            lacfunprf = LFUfindLacfunConditional (arg_node);
            if (NULL != lacfunprf) { // LOOPFUNs only
                lacfunprf = ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (lacfunprf)));
                INFO_LACFUNPRF (arg_info) = LET_EXPR (lacfunprf);
            }
            FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
            INFO_LACFUNPRF (arg_info) = NULL;
        }
    }

    INFO_FUNDEF (arg_info) = fundefold;
    INFO_LACFUNPRF (arg_info) = lacfunprfold;

    DBUG_PRINT ("leaving function %s", FUNDEF_NAME (arg_node));

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

    arg_node = POLYSsetClearAvisPart (arg_node, arg_node);

    CODE_CBLOCK (PART_CODE (arg_node))
      = TRAVopt (CODE_CBLOCK (PART_CODE (arg_node)), arg_info);

    arg_node = POLYSsetClearAvisPart (arg_node, NULL);

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
    WITH_CODE (arg_node) = WLUTremoveUnusedCodes (WITH_CODE (arg_node));
    INFO_WITH (arg_info) = lastwith;

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *POGOassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
POGOassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_NASSIGN (arg_info) = arg_node;
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    INFO_NASSIGN (arg_info) = NULL;

    if (INFO_PREASSIGNS (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *POGOap( node *arg_node, info *arg_info)
 *
 * @brief: If this is a non-recursive call of a LACFUN,
 *         set FUNDEF_CALLAP to point to this N_ap's N_assign node,
 *         then traverse the LACFUN.
 *
 *****************************************************************************/
node *
POGOap (node *arg_node, info *arg_info)
{
    node *lacfundef;
    node *newfundef;

    DBUG_ENTER ();

    lacfundef = AP_FUNDEF (arg_node);
    if ((FUNDEF_ISLACFUN (lacfundef)) &&         /* Ignore call to non-lacfun */
        (lacfundef != INFO_FUNDEF (arg_info))) { /* Ignore recursive call */
        DBUG_PRINT ("Found LACFUN: %s non-recursive call from: %s",
                    FUNDEF_NAME (lacfundef), FUNDEF_NAME (INFO_FUNDEF (arg_info)));
        /* Traverse into the LACFUN */
        INFO_LACFUN (arg_info) = lacfundef; /* The called lacfun */
        newfundef = TRAVdo (lacfundef, arg_info);
        DBUG_ASSERT (newfundef = lacfundef,
                     "Did not expect N_fundef of LACFUN to change");
        INFO_LACFUN (arg_info) = NULL; /* Back to normal traversal */
    }

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

/** <!-- ****************************************************************** -->
 *
 * @fn int getLoopCountForFundef( node *arg_node, node *fundef)
 *
 * @brief Get the loopcount for the arithmetic progression vector
 *        of this loopfun.
 *
 * @param  nid - the parameter variable we are analyzing
 * @param  fundef - the N_fundef node for this loopfun
 *
 * @return: If nid is not part of the COND_COND that controls this
 *          loopfun, return the loopcount. Otherwise, return -1.
 *
 ******************************************************************************/
static int
getLoopCountForFundef (node *arg_node, node *fundef)
{
    node *lacfunprf;
    int z = -1;

    DBUG_ENTER ();

    lacfunprf = LFUfindLacfunConditional (fundef);
    if (NULL != lacfunprf) { // LOOPFUNs only
        lacfunprf = LET_EXPR (ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (lacfunprf))));
        if (lacfunprf != arg_node) { // THis is the COND_COND
            // We add 1 because LOOPCOUNT excludes first iteration (tail recursion).
            z = (UNR_NONE != FUNDEF_LOOPCOUNT (fundef)) ? 1 + FUNDEF_LOOPCOUNT (fundef)
                                                        : -1;
        }
    }

    DBUG_RETURN (z);
}

/** <!--*******************************************************************-->
 *
 * @fn node *POGOprf( node *arg_node, info *arg_info)
 *
 * @brief: If we can show that a relational or guard is always
 *         TRUE or always FALSE, replace it by that constant.
 *
 * @result: Possibly altered N_prf node.
 *
 * Algorithm: We build four maximal affine expression trees:
 *              (B,C) for PRF_ARG1 and PRF_ARG2.
 *              RFN for the N_prf relational function.
 *              CFN for the companion relational function.
 *
 *            We then invoke the polyhedral solver( e.g., PolyLib).
 *            If it can prove that B and C represent identical sets,
 *            it returns POLY_MATCH_BC. If not, it
 *            then performs set intersection on B,C,RFN,
 *            and B,C,CFN generating POLY_EMPTYSET_BCR and/or POLY_EMPTYSET_BCC,
 *            if those intersections are empty.
 *
 *            Those results determine if we can optimize, as follows:
 *            First, based on matching, we obtain:
 *
 *            Relational  POLY_MATCH_BC
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
 *            If the intersection of B,C,RFN is empty, the result is FALSE;
 *               else unknown.
 *            If the intersection of B,C,CFN is empty, the result is TRUE;
 *               else unknown.
 *
 *****************************************************************************/
node *
POGOprf (node *arg_node, info *arg_info)
{
    node *exprsX = NULL;
    node *exprsY = NULL;
    node *exprsFn = NULL;
    node *exprsCfn = NULL;
    node *res;
    node *resp;
    node *resa;
    node *guardp;
    node *arg1 = NULL;
    node *arg2 = NULL;
    int emp = POLY_RET_UNKNOWN;
    int loopcount = -1;
    bool dopoly = FALSE;
    bool z = FALSE;
    bool resval = FALSE;
    prf mappedprf;

    DBUG_ENTER ();

    res = arg_node;
    if ((PHUTisCompatibleAffinePrf (PRF_PRF (arg_node))) && (global.optimize.dopogo)
        && (PHUTisCompatibleAffineTypes (arg_node))) {
        switch (PRF_PRF (arg_node)) {
        case F_eq_SxS:
        case F_neq_SxS:
        case F_lt_SxS:
        case F_le_SxS:
        case F_ge_SxS:
        case F_gt_SxS:
        case F_val_lt_val_SxS:
        case F_val_le_val_SxS:
            DBUG_PRINT ("Looking at dyadic N_prf for %s",
                        AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
            loopcount = getLoopCountForFundef (arg_node, INFO_FUNDEF (arg_info));
            // I don't like this, but we have to get both PRF_ARGs
            // as set variables before we get the AFT built.
            arg1 = PHUTskipChainedAssigns (PRF_ARG1 (arg_node));
            AVIS_ISLCLASS (ID_AVIS (arg1)) = AVIS_ISLCLASSSETVARIABLE;
            exprsX = PHUTgenerateAffineExprs (arg1, INFO_FUNDEF (arg_info),
                                              INFO_VARLUT (arg_info),
                                              AVIS_ISLCLASSSETVARIABLE, loopcount);

            arg2 = PHUTskipChainedAssigns (PRF_ARG2 (arg_node));
            AVIS_ISLCLASS (ID_AVIS (arg2)) = AVIS_ISLCLASSSETVARIABLE;
            exprsY = PHUTgenerateAffineExprs (arg2, INFO_FUNDEF (arg_info),
                                              INFO_VARLUT (arg_info),
                                              AVIS_ISLCLASSSETVARIABLE, loopcount);

            dopoly = (NULL != exprsX) && (NULL != exprsY);
            break;

        case F_non_neg_val_S:
            DBUG_PRINT ("Looking at monadic N_prf for %s",
                        AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
            loopcount = getLoopCountForFundef (arg_node, INFO_FUNDEF (arg_info));
            arg1 = PHUTskipChainedAssigns (PRF_ARG1 (arg_node));
            AVIS_ISLCLASS (ID_AVIS (arg1)) = AVIS_ISLCLASSSETVARIABLE;
            exprsX = PHUTgenerateAffineExprs (arg1, INFO_FUNDEF (arg_info),
                                              INFO_VARLUT (arg_info),
                                              AVIS_ISLCLASSSETVARIABLE, loopcount);
            exprsY = NULL;
            dopoly = (NULL != exprsX);
            break;

        default:
            dopoly = FALSE;
            break;
        }

        if (dopoly) {
            // Don't bother calling ISL if it can't do anything for us.
            mappedprf = POGOmapPrf (PRF_PRF (arg_node));
            exprsFn = PHUTgenerateAffineExprsForGuard (mappedprf, arg1, arg2,
                                                       INFO_FUNDEF (arg_info), mappedprf,
                                                       INFO_VARLUT (arg_info), 0);
            exprsCfn = PHUTgenerateAffineExprsForGuard (mappedprf, arg1, arg2,
                                                        INFO_FUNDEF (arg_info),
                                                        LFUdualFun (PRF_PRF (arg_node)),
                                                        INFO_VARLUT (arg_info), 0);

            emp = PHUTcheckIntersection (exprsX, exprsY, exprsFn, exprsCfn,
                                         INFO_VARLUT (arg_info), POLY_OPCODE_INTERSECT,
                                         AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
            exprsX = NULL; // PHUTcheckIntersection consumes the N_exprs
            exprsY = NULL;
            exprsFn = NULL;
            exprsCfn = NULL;
            DBUG_PRINT ("PHUTcheckIntersection result for %s is %d",
                        AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))), emp);

            // Match analysis for B,C
            if (emp & POLY_RET_MATCH_BC) {
                DBUG_PRINT ("Matching arguments for %s",
                            AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
                resval = GetXYmatch (PRF_PRF (arg_node));
                z = TRUE;
            }

            if ((!z) && (emp & POLY_RET_EMPTYSET_BCF)) {
                DBUG_PRINT ("Matching Fn sets for %s",
                            AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
                resval = FALSE;
                z = TRUE;
            }

            // We treat an empty set exprsX as TRUE.
            if ((!z)
                && ((emp & POLY_RET_EMPTYSET_B) || ((emp & POLY_RET_EMPTYSET_BCG)))) {
                DBUG_PRINT ("Matching DualFun sets for %s",
                            AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
                resval = TRUE;
                z = TRUE;
            }

            if (z) { // guard/primitive can be removed
                DBUG_PRINT ("Guard/relational for result %s replaced, predicate is %d",
                            AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))), resval);
                resp = TBmakeBool (resval);
                if (TUisPrfGuard (arg_node)) { // Guard needs two results
                    if (!resval) {
                        CTIwarn ("Guard failure detected for result %s",
                                 AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
                    }
                    res = DUPdoDupNode (PRF_ARG1 (arg_node));
                    arg_node = FREEdoFreeNode (arg_node);
                    guardp = IDS_NEXT (INFO_LHS (arg_info));
                    resa = TBmakeAssign (TBmakeLet (guardp, resp), NULL);
                    AVIS_SSAASSIGN (IDS_AVIS (guardp)) = resa;
                    IDS_NEXT (INFO_LHS (arg_info)) = NULL;
                    INFO_PREASSIGNS (arg_info)
                      = TCappendAssign (INFO_PREASSIGNS (arg_info), resa);
                } else { // Successful removal of non-guard
                    res = resp;
                    arg_node = FREEdoFreeNode (arg_node);
                }
            } else {
                DBUG_PRINT ("Unable to remove guard/primitive for result %s",
                            AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
            }
        }

        exprsX = (NULL != exprsX) ? FREEdoFreeTree (exprsX) : NULL;
        exprsY = (NULL != exprsY) ? FREEdoFreeTree (exprsY) : NULL;
        exprsFn = (NULL != exprsFn) ? FREEdoFreeTree (exprsFn) : NULL;
        exprsCfn = (NULL != exprsCfn) ? FREEdoFreeTree (exprsCfn) : NULL;
    }

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

    INFO_VARLUT (arg_info) = LUTgenerateLut ();
    TRAVpush (TR_pogo);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();
    PHUTclearAvisIslAttributes (INFO_VARLUT (arg_info));
    INFO_VARLUT (arg_info) = LUTremoveLut (INFO_VARLUT (arg_info));

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 * bool POGOisPositive( node *arg_node)
 *
 * function: Predicate for determining if an argument is known to
 *           be positive.
 *
 * @param: arg_node: An N_exprs ISL chain
 * @param: aft: An AFT
 * @param: fundef : The N_fundef we are in.
 * @param: varlut: The LUT used by PHUT/POGO.
 *
 * result: True if argument is known to be positive.
 *         Else false.
 *
 *         NB. False does NOT imply that the argument is non-positive.
 *
 * NB. This function is used in PHUT for mod() and aplmod(), where
 *     SCSisPositive is too weak to do the job.
 *
 *     It must only be called from code that invokes PHUT. I.e.,
 *     the caller maintains AVIS_NPART and FUNDEF_CALLAP.
 *
 *     In particular, all elements of arg_node are already in the LUT.
 *     This is why we can't just call PHUTgenerateAffineExprs - it would
 *     do nothing.
 *
 *****************************************************************************/
bool
POGOisPositive (node *arg_node, node *aft, node *fundef, lut_t *varlut)
{
    bool z;
    node *exprsFn;
    node *zro;
    node *arg1;
    int emp = POLY_RET_UNKNOWN;

    DBUG_ENTER ();

    z = SCSisPositive (arg_node);
    if (!z) {
        arg1 = PHUTskipChainedAssigns (arg_node);
        zro = TBmakeNum (0);
        // We look for NULL intersect on the dual function: arg1 <= 0
        exprsFn = PHUTgenerateAffineExprsForGuard (F_le_SxS, arg1, zro, fundef, F_le_SxS,
                                                   varlut, 0);
        emp = PHUTcheckIntersection (DUPdoDupTree (aft), NULL, exprsFn, NULL, varlut,
                                     POLY_OPCODE_INTERSECT, "POGOisPositive");
        z = 0 != (emp & POLY_RET_EMPTYSET_BCF);
        FREEdoFreeNode (zro);
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 * bool POGOisNegative( node *arg_node)
 *
 * function: Predicate for determining if an argument is known to
 *           be negative.
 *
 * @param: arg_node: An N_exprs ISL chain
 * @param: aft: An AFT
 * @param: fundef : The N_fundef we are in.
 * @param: varlut: The LUT used by PHUT/POGO.
 *
 * result: True if argument is known to be negative.
 *         Else false.
 *
 *         NB. False does NOT imply that the argument is non-negative.
 *
 * NB. This function is used in PHUT for mod() and aplmod(), where
 *     SCSisNegative is too weak to do the job.
 *
 *     It must only be called from code that invokes PHUT. I.e.,
 *     the caller maintains AVIS_NPART and FUNDEF_CALLAP.
 *
 *     In particular, all elements of arg_node are already in the LUT.
 *     This is why we can't just call PHUTgenerateAffineExprs - it would
 *     do nothing.
 *
 *****************************************************************************/
bool
POGOisNegative (node *arg_node, node *aft, node *fundef, lut_t *varlut)
{
    bool z;
    node *exprsFn;
    node *zro;
    node *arg1;
    int emp = POLY_RET_UNKNOWN;

    DBUG_ENTER ();

    z = SCSisPositive (arg_node);
    if (!z) {
        arg1 = PHUTskipChainedAssigns (arg_node);
        zro = TBmakeNum (0);
        // We look for NULL intersect on the dual function: arg1 >= 0
        exprsFn = PHUTgenerateAffineExprsForGuard (F_ge_SxS, arg1, zro, fundef, F_ge_SxS,
                                                   varlut, 0);
        emp = PHUTcheckIntersection (DUPdoDupTree (aft), NULL, exprsFn, NULL, varlut,
                                     POLY_OPCODE_INTERSECT, "POGOisNegative");
        z = 0 != (emp & POLY_RET_EMPTYSET_BCF);
        FREEdoFreeNode (zro);
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 * bool POGOisNonPositive( node *arg_node)
 *
 * function: Predicate for determining if an argument is known to
 *           be non-positive.
 *
 * @param: arg_node: An N_exprs ISL chain
 * @param: aft: An AFT
 * @param: fundef : The N_fundef we are in.
 * @param: varlut: The LUT used by PHUT/POGO.
 *
 * result: True if argument is known to be non-positive.
 *         Else false.
 *
 *         NB. False does NOT imply that the argument is positive.
 *
 * NB. This function is used in PHUT for mod() and aplmod(), where
 *     SCSisNonPositive is too weak to do the job.
 *
 *     It must only be called from code that invokes PHUT. I.e.,
 *     the caller maintains AVIS_NPART and FUNDEF_CALLAP.
 *
 *     In particular, all elements of arg_node are already in the LUT.
 *     This is why we can't just call PHUTgenerateAffineExprs - it would
 *     do nothing.
 *
 *****************************************************************************/
bool
POGOisNonPositive (node *arg_node, node *aft, node *fundef, lut_t *varlut)
{
    bool z;
    node *exprsFn;
    node *zro;
    node *arg1;
    int emp = POLY_RET_UNKNOWN;

    DBUG_ENTER ();

    z = SCSisNonnegative (arg_node);
    if (!z) {
        arg1 = PHUTskipChainedAssigns (arg_node);
        zro = TBmakeNum (0);
        // We look for NULL intersect on the dual function  arg1 > 0
        exprsFn = PHUTgenerateAffineExprsForGuard (F_gt_SxS, arg1, zro, fundef, F_gt_SxS,
                                                   varlut, 0);
        emp = PHUTcheckIntersection (DUPdoDupTree (aft), NULL, exprsFn, NULL, varlut,
                                     POLY_OPCODE_INTERSECT, "POGOisNonPositive");
        z = 0 != (emp & POLY_RET_EMPTYSET_BCF);
        FREEdoFreeNode (zro);
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 * bool POGOisNonNegative( node *arg_node)
 *
 * function: Predicate for determining if an argument is known to
 *           be non-negative.
 *
 * @param: arg_node: An N_exprs ISL chain
 * @param: aft: An AFT
 * @param: fundef : The N_fundef we are in.
 * @param: varlut: The LUT used by PHUT/POGO.
 *
 * result: True if argument is known to be non-negative.
 *         Else false.
 *
 *         NB. False does NOT imply that the argument is negative.
 *
 * NB. This function is used in PHUT for mod() and aplmod(), where
 *     SCSisNonNegative is too weak to do the job.
 *
 *     It must only be called from code that invokes PHUT. I.e.,
 *     the caller maintains AVIS_NPART and FUNDEF_CALLAP.
 *
 *     In particular, all elements of arg_node are already in the LUT.
 *     This is why we can't just call PHUTgenerateAffineExprs - it would
 *     do nothing.
 *
 *****************************************************************************/
bool
POGOisNonNegative (node *arg_node, node *aft, node *fundef, lut_t *varlut)
{
    bool z;
    node *exprsFn;
    node *zro;
    node *arg1;
    int emp = POLY_RET_UNKNOWN;

    DBUG_ENTER ();

    z = SCSisNonnegative (arg_node);
    if (!z) {
        arg1 = PHUTskipChainedAssigns (arg_node);
        zro = TBmakeNum (0);
        // We look for NULL intersect on the dual function  arg1 < 0
        exprsFn = PHUTgenerateAffineExprsForGuard (F_lt_SxS, arg1, zro, fundef, F_lt_SxS,
                                                   varlut, 0);
        emp = PHUTcheckIntersection (DUPdoDupTree (aft), NULL, exprsFn, NULL, varlut,
                                     POLY_OPCODE_INTERSECT, "POGOisNonNegative");
        z = 0 != (emp & POLY_RET_EMPTYSET_BCF);
        FREEdoFreeNode (zro);
    }

    DBUG_RETURN (z);
}

#undef DBUG_PREFIX
