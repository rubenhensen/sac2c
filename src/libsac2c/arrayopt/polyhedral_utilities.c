/** <!--********************************************************************-->
 *
 * @defgroup phut polyhedral analysis utility functions
 *
 *  Overview: These functions are intended to provide useful
 *            manipulation services for polyhedron-based analysis.
 *            At present, we only support ISL. PolyLib support
 *            was removed 2015-05-25.
 *
 *
 * ISL input data Terminology, taken from http://barvinok.gforge.inria.fr/tutorial.pdf,
 *      Sven Verdoolaege's ISCC tutorial.
 *
 * A triangular "statement instance set" such as S:
 *
 *     for (i = 1; i <= n; ++i)
 *       for (j = 1; j <= i; ++j)
 *        **  S  **
 *
 * is represented as this input to ISL:
 *
 *          [n] -> { S[i,j] : 1 <= i <= n and 1 <= j <= i }
 *                            ---- Presburger formula
 *                     --- set variables
 *                   - optional name of space
 *           - symbolic constants
 *
 *    A Presburger formula is a union of one or more constraints.
 *
 * NB. This code assumes, in at least one place, that AVIS_NPART is
 *     being maintained for all WITHID elements by the calling traversal.
 *
 * NB. General assumptions made in these functions about the calling
 *     envionment:
 *
 *      1. The caller will maintain AVIS_NPART. It would be nice
 *         if this were maintained throughout compilation, but things
 *         can change underfoot enough we are unsure where we should
 *         invalidate it. Hence, we assume the calling traversal
 *         will pass through N_part codes on its way down, and set AVIS_NPART.
 *         Similarly, on its way up, it will unset AVIS_NPART.
 *
 *      2. Similarly, the caller must maintain FUNDEF_CALLAP and FUNDEF_CALLERFUNDEF
 *
 * NB.     Interprocedural affine function construction
 *         (Look at ~/sac/testsuite/optimizations/pogo/condfun.sac to see
 *          why we need to look at the LACFUN caller to build
 *          an affine function tree for XXX's shape vector.
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file polyhedral_utilities.c
 *
 * Prefix: PHUT
 *
 *****************************************************************************/

#include "globals.h"

#define DBUG_PREFIX "PHUT"
#include <stdlib.h>
#include "debug.h"
#include "memory.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "pattern_match.h"
#include "constants.h"
#include "shape.h"
#include "type_utils.h"
#include "new_types.h"
#include "constants.h"
#include "new_typecheck.h"
#include "narray_utilities.h"
#include "DupTree.h"
#include "print.h"
#include "system.h"
#include "filemgr.h"
#include "sys/param.h"
#include "polyhedral_utilities.h"
#include "polyhedral_defs.h"
#include "symbolic_constant_simplification.h"
#include "with_loop_utilities.h"
#include "lacfun_utilities.h"
#include "LookUpTable.h"

// There is a not-an-N_prf value for N_prf somewhere, but I can't find it.
#define NOPRFOP 0

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    lut_t *varlut;
};

/**
 * Macro definitions for INFO structure
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_VARLUT(n) ((n)->varlut)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));
    INFO_FUNDEF (result) = NULL;
    INFO_VARLUT (result) = NULL;

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
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!-- ****************************************************************** -->
 *
 * @fn void PHUTprintIslAffineFunctionTree( node *arg_node)
 *
 * @brief  Utility for printing an ISL affine function tree in slightly
 *         more human-readable form.
 *
 * @param An N_exprs chain for ISL
 *
 * @return none
 *
 ******************************************************************************/
void
PHUTprintIslAffineFunctionTree (node *arg_node)
{
    int n;
    int j;

    DBUG_ENTER ();

    n = TCcountExprs (arg_node);
    for (j = 0; j < n; j++) {
        PRTdoPrint (TCtakeDropExprs (1, j, arg_node));
    }

    DBUG_RETURN ();
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *RemoveRcaAssign( node *relarg, node *rca)
 *
 * @brief Remove the assign to rca from the relarg chain.
 *        Search relarg for a + or - on the induction variable, rca.
 *
 *        We are looking for an N_exprs chain like:
 *
 *            rca = X + Y;
 *            rca = X - Y;
 *
 *        where one of the arguments is constant.
 *
 * @param relarg: An N_exprs chain for ISL
 * @param rca: An N_id
 *
 * @return The updated relarg node
 *
 ******************************************************************************/
static node *
RemoveRcaAssign (node *relarg, node *rca)
{
    DBUG_ENTER ();

#ifdef FIXME
    node *z = NULL;
    node *exprsouter;
    node *exprs;
    node *exprs2;

    exprsouter = relarg;
    while ((NULL == z) && (NULL != exprsouter)) {
        exprs = EXPRS_EXPR (exprsouter);
        if ((ID_AVIS (rca) == ID_AVIS (EXPRS_EXPR (exprs)))
            && (N_prf == NODE_TYPE (EXPRS_EXPR (EXPRS_NEXT (exprs))))
            && (F_eq_SxS == PRF_PRF (EXPRS_EXPR (EXPRS_NEXT (exprs))))) { // rca = ...
            exprs2 = EXPRS_NEXT (EXPRS_NEXT (exprs));                     // drop rca =
            if ((N_prf == NODE_TYPE (EXPRS_EXPR (EXPRS_NEXT (exprs2))))
                && ((F_add_SxS == PRF_PRF (EXPRS_EXPR (EXPRS_NEXT (exprs2))))
                    || (F_sub_SxS == PRF_PRF (EXPRS_EXPR (EXPRS_NEXT (exprs2)))))) {
                if ((SCSisPositive (EXPRS_EXPR (exprs2)))
                    || (SCSisNegative (EXPRS_EXPR (exprs2)))) {
                    DBUG_PRINT ("Trimmed relarg arg1");
                    EXPRS_EXPR (exprsouter) = FREEdoFreeTree (EXPRS_EXPR (exprsouter));
                } else {
                    if ((SCSisPositive (EXPRS_EXPR (EXPRS_NEXT (EXPRS_NEXT (exprs2)))))
                        || (SCSisNegative (
                             EXPRS_EXPR (EXPRS_NEXT (EXPRS_NEXT (exprs2)))))) {
                        DBUG_PRINT ("Trimmed relarg arg2");
                        EXPRS_EXPR (exprsouter)
                          = FREEdoFreeTree (EXPRS_EXPR (exprsouter));
                    }
                }
            }
        }
        exprsouter = EXPRS_NEXT (exprsouter);
    }
#endif // FIXME

    DBUG_RETURN (relarg);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *Node2Avis( node *arg_node)
 *
 * @brief Find N_avis node for arg_node
 *
 * @param An N_avis, N_id, or N_ids node
 *
 * @return the associated N_avis node, or NULL if this is an N_num
 *         or if arg_node is NULL
 *
 ******************************************************************************/
static node *
Node2Avis (node *arg_node)
{
    node *avis = NULL;

    DBUG_ENTER ();

    if (NULL != arg_node) {
        switch (NODE_TYPE (arg_node)) {
        case N_id:
            avis = ID_AVIS (arg_node);
            break;

        case N_ids:
            avis = IDS_AVIS (arg_node);
            break;

        case N_avis:
            avis = arg_node;
            break;

        case N_num:
        case N_bool:
            break;

        default:
            DBUG_ASSERT (NULL != avis, "Expected N_id, N_avis, or N_ids node");
        }
    }

    DBUG_RETURN (avis);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *Node2Value( node *arg_node)
 *
 * @brief Find N_avis node for arg_node, unless it is
 *        an N_num/N_bool, in which case we XXX
 *
 * @param An N_avis, N_id, N_num, N_bool, or N_ids node
 *
 * @return An N_id if the argument is an N_id, N_avis, or N_ids.
 *         Otherwise, the associated N_avis node,
 *
 ******************************************************************************/
static node *
Node2Value (node *arg_node)
{
    node *z = NULL;
    constant *con = NULL;

    DBUG_ENTER ();

    if (NULL != arg_node) {
        z = Node2Avis (arg_node);
        if (NULL != z) {
            if (TYisAKV (AVIS_TYPE (z))) {
                if (TUisIntScalar (AVIS_TYPE (z))) {
                    z = TBmakeNum (TUtype2Int (AVIS_TYPE (z)));
                } else {
                    if (TUisBoolScalar (AVIS_TYPE (z))) {
                        con = TYgetValue (AVIS_TYPE (z)); // con is NOT a copy!
                        z = TBmakeNum (COconst2Int (con));
                    } else {
                        DBUG_ASSERT (FALSE, "Expected N_num or N_bool");
                    }
                }
            } else {
                DBUG_ASSERT (N_avis == NODE_TYPE (z), "Expected N_avis from Node2Avis");
                z = TBmakeId (z);
            }
        } else {
            switch (NODE_TYPE (arg_node)) {
            case N_num:
                z = DUPdoDupNode (arg_node);
                break;

            case N_bool:
                z = TBmakeNum (BOOL_VAL (arg_node));
                break;

            default:
                DBUG_ASSERT (FALSE,
                             "Expected N_id, N_avis, N_ids, N_num, or N_bool node");
            }
        }
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn bool InsertVarIntoLut( node *arg_node, lut_t *varlut)
 *
 * @brief Insert N_id or N_avis into varlut, if not already there.
 *        We use varlut LUT solely as a way to build the set of unique
 *        set variable names.
 *
 * @param arg_node: An N_id or N_avis. Or, just to make life more fun,
 *                  it may be an N_num or N_bool, which we ignore.
 *        arg_info: your basic arg_info
 *
 * @return TRUE if we the variables was inserted; FALSE if it was already there
 *         or not a variable.
 *
 ******************************************************************************/
static bool
InsertVarIntoLut (node *arg_node, lut_t *varlut)
{
    node *avis;
    node *founditem = NULL;
    bool z = FALSE;

    DBUG_ENTER ();

    avis = Node2Avis (arg_node);
    if (NULL != avis) {
        DBUG_ASSERT (NULL != varlut, "NULL VARLUT");
        LUTupdateLutP (varlut, avis, avis, (void **)&founditem);
        z = NULL == founditem;
        if (z) {
            DBUG_PRINT ("Inserted %s into VARLUT", AVIS_NAME (avis));
        } else {
            DBUG_PRINT ("%s already in VARLUT", AVIS_NAME (avis));
        }
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn char *Prf2Isl( prf arg_node)
 *
 * @brief Convert an N_prf PRF_PRF into
 *
 * @param arg_node an N_prf.
 *
 * @return An pointer to a character string representing some affine
 *         function on scalar arguments.
 *
 ******************************************************************************/
static char *
Prf2Isl (prf arg_node)
{
    char *z = NULL;

    DBUG_ENTER ();

    switch (arg_node) {

    default:
        DBUG_ASSERT (FALSE, "Did not find affine function in table");
        break;

    case F_lt_SxS:
    case F_val_lt_val_SxS:
        z = "<";
        break;

    case F_le_SxS:
    case F_val_le_val_SxS:
        z = "<=";
        break;

    case F_eq_SxS:
        z = "=";
        break;
    case F_ge_SxS:
        z = ">=";
        break;
    case F_gt_SxS:
        z = ">";
        break;
    case F_add_SxS:
        z = "+";
        break;
    case F_sub_SxS:
        z = "-";
        break;
    case F_mul_SxS:
        z = "*";
        break;
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn void GetIslSetVariablesFromLut(
 *
 * @brief Build the set variables used by the contraints.
 *
 *        The result is an N_exprs chain of the N_avis nodes comprising the
 *        set variables.
 *
 *           [i,j,k...]
 *
 * @param varlut: a lut of nodes that comprise the ISL set variables.
 *
 * @return An N_exprs chain of N_avis nodes
 *
 ******************************************************************************/
static void *
GetIslSetVariablesFromLutOne (void *rest, void *avis)
{
    node *z = NULL;

    DBUG_ENTER ();

    if (NULL != avis) {
        DBUG_PRINT ("Found %s in VARLUT", AVIS_NAME (avis));
        z = TCappendExprs (TBmakeExprs (TBmakeId (avis), NULL), rest);
    }

    DBUG_RETURN (z);
}

static node *
GetIslSetVariablesFromLut (lut_t *varlut)
{
    node *z;
    node *head = NULL;

    DBUG_ENTER ();

    z = (node *)LUTfoldLutP (varlut, head, GetIslSetVariablesFromLutOne);

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *BuildIslSimpleConstraint()
 * @fn node *BuildIslNotSoSimpleConstraint()
 *
 * @brief Build one constraint for ISL as a one-element N_exprs chain, containing
 *        an N_exprs chain of the constraint.
 *
 *        For ids nprf1  arg1 nprf2 arg2, we get, e.g.,
 *              0 <=     arg1 <     arg2
 *
 * @param ids, arg1, arg2: N_id, N_avis or N_num nodes
 *        nprf1, nprf2: the function(s) to be used, or NULL
 *
 * @return The result is a one-element N_exprs comprising an N_exprs chain
 *         of the constraint, which will eventually
 *         be assembled, then turned into text and written to file for passing to ISL.
 *         The two-deep nesting is so that we can introduce "and" conjunctions
 *         among the contraints when constructing the ISL input.
 *
 *         If ids, arg1, or arg2 is AKV, we replace it by its value.
 *
 ******************************************************************************/
static node *
BuildIslSimpleConstraint (node *ids, prf nprf1, node *arg1, prf nprf2, node *arg2)
{
    node *z;
    node *idsv;
    node *arg1v;
    node *arg2v;

    DBUG_ENTER ();

    DBUG_PRINT ("Generating simple constraint");
    idsv = Node2Avis (ids);
    idsv = (NULL == idsv) ? ids : idsv; // N_num support
    if ((NULL != idsv) && (N_avis == NODE_TYPE (idsv))) {
        idsv = TBmakeId (idsv);
    }
    DBUG_ASSERT (NULL != idsv, "Expected non-NULL ids");
    arg1v = Node2Value (arg1);
    arg2v = Node2Value (arg2);

    z = TBmakeExprs (idsv, NULL);
    z = TCappendExprs (z, TBmakeExprs (TBmakePrf (nprf1, NULL), NULL));
    z = TCappendExprs (z, TBmakeExprs (arg1v, NULL));

    if (NOPRFOP != nprf2) {
        z = TCappendExprs (z, TBmakeExprs (TBmakePrf (nprf2, NULL), NULL));
        if (NULL != arg2v) { // kludge for disjunction F_or_SxS
            z = TCappendExprs (z, TBmakeExprs (arg2v, NULL));
        }
    }

    z = TBmakeExprs (z, NULL);

    DBUG_RETURN (z);
}

static node *
BuildIslNotSoSimpleConstraint (node *ids, prf nprf1, node *arg1, prf nprf2, node *arg2,
                               prf nprf3, node *arg3)
{ // Like BuildIslSimpleConstraint, but with one more prf and arg.
    node *z;
    node *idsv;
    node *arg1v;
    node *arg2v;
    node *arg3v;

    DBUG_ENTER ();

    DBUG_PRINT ("Generating not-so-simple constraint");
    idsv = Node2Avis (ids);
    idsv = (NULL == idsv) ? ids : idsv; // N_num support
    if ((NULL != idsv) && (N_avis == NODE_TYPE (idsv))) {
        idsv = TBmakeId (idsv);
    }
    DBUG_ASSERT (NULL != idsv, "Expected non-NULL ids");
    arg1v = Node2Value (arg1);
    arg2v = Node2Value (arg2);
    arg3v = Node2Value (arg3);

    z = TBmakeExprs (idsv, NULL);
    z = TCappendExprs (z, TBmakeExprs (TBmakePrf (nprf1, NULL), NULL));
    z = TCappendExprs (z, TBmakeExprs (arg1v, NULL));

    if (NOPRFOP != nprf2) {
        z = TCappendExprs (z, TBmakeExprs (TBmakePrf (nprf2, NULL), NULL));
        if (NULL != arg2v) { // kludge for disjunction F_or_SxS
            z = TCappendExprs (z, TBmakeExprs (arg2v, NULL));
        }
    }

    if (NOPRFOP != nprf3) {
        z = TCappendExprs (z, TBmakeExprs (TBmakePrf (nprf3, NULL), NULL));
        z = TCappendExprs (z, TBmakeExprs (arg3v, NULL));
    }

    z = TBmakeExprs (z, NULL);

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *findBoundEl( node *arg_node, node *bnd, int k, info *arg_info)
 *
 * @brief arg_node may represent a WITHID element for a WL.
 *        If so, generate an ISL constraint of:
 *
 *             k = WITHID_IDS iota arg_node
 *             arg_node >= LB[k]
 *
 * @param arg_node: an N_avis for the WITHID_IDS
 *        bnd: GENERATOR_BOUND1 or GENERATOR_BOUND2
 *        k: index into WITHID_IDS, which may be -1.
 *
 * @return If arg_node is a member of a WITHID, LB[k] or UB[k]
 *         Otherwise, NULL.
 *
 ******************************************************************************/
static node *
findBoundEl (node *arg_node, node *bnd, int k, info *arg_info)
{
    node *z = NULL;

    DBUG_ENTER ();

    if ((NULL != AVIS_NPART (arg_node)) && (NULL != bnd)) {
        if (-1 != k) {
            z = TCgetNthExprsExpr (k, ARRAY_AELEMS (bnd));
        }
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *PHUTcollectWlGenerator( node *arg_node)
 *
 * @brief arg_node is a WITHID element for a WL, shown here as iv.
 *        The slightly messy part here is non-unit step and/or width, which entail
 *        adding two new set variables, iv', iv'', and ivw.
 *
 *        Assume we have this WL generator:
 *
 *          lb <= iv < ub step stp width wid
 *
 *        We generate ISL constraints as follows:
 *
 *          iv' = lb + stp * N   for some N
 *          0 <= ivw < wid
 *          iv'' = iv' + ivw
 *          iv = iv''
 *          lb <= iv < ub
 *
 *        We also have to ensure that these constraints are generated
 *        whenever there is ANY reference to a WITHID_IDS.
 *
 *        One key simplification here is that SAC requires that wid <= stp, and that
 *        both be constants.
 *
 *        NB. lb, ub, stp, and wid are N_num or N_id nodes.
 *
 *        See also S.B. Scholz: SAC - Efficient Support... p14
 *
 * @param An N_avis
 * @param res: an N_exprs chain for the growing result, initially NULL.
 *
 * @return If arg_node is a member of a WITHID, an N_exprs chain for
 *         the above relational, catenated to the incoming res.
 *
 *         Otherwise, NULL.
 *
 ******************************************************************************/
static node *
StepOrWidthHelper (node *stpwid)
{ // Find GENERATOR_STEP or GENERATOR_WIDTH element as an N_num.
    // If stpwid is NULL, generate a value of 1 for it.

    node *z;
    pattern *pat;
    constant *con = NULL;

    if (NULL != stpwid) {
        pat = PMconst (1, PMAgetVal (&con), 0);
        if (PMmatchFlat (pat, stpwid)) {
            z = TBmakeNum (COconst2Int (con));
            con = COfreeConstant (con);
        } else {
            z = NULL; // Make gcc happy
            DBUG_ASSERT (FALSE, "Found non-constant stride/width");
        }
        pat = PMfree (pat);
    } else {
        z = TBmakeNum (1);
    }

    return (z);
}

node *
PHUTcollectWlGenerator (node *arg_node, info *arg_info, node *res)
{
    node *z = NULL;
    node *partn;
    node *lbel = NULL;
    node *ubel = NULL;
    node *ivavis;
    int k;
    node *lb;
    node *ub;
    node *stp;
    node *wid;
    node *stpel;
    node *widel;
    node *ivpavis;
    node *ivppavis;
    node *ivwavis;
    node *navis;

    DBUG_ENTER ();

    partn = AVIS_NPART (arg_node);
    if ((NULL != partn)) {
        k = LFUindexOfMemberIds (arg_node, WITHID_IDS (PART_WITHID (partn)));
        if (-1 != k) {
            DBUG_PRINT ("Generating generator constraints for %s", AVIS_NAME (arg_node));
            ivavis = TCgetNthIds (k, WITHID_IDS (PART_WITHID (partn)));
            lb = WLUTfindArrayForBound (GENERATOR_BOUND1 (PART_GENERATOR (partn)));
            ub = WLUTfindArrayForBound (GENERATOR_BOUND2 (PART_GENERATOR (partn)));
            stp = WLUTfindArrayForBound (GENERATOR_STEP (PART_GENERATOR (partn)));
            wid = WLUTfindArrayForBound (GENERATOR_WIDTH (PART_GENERATOR (partn)));
            lbel = findBoundEl (arg_node, lb, k, arg_info);
            ubel = findBoundEl (arg_node, ub, k, arg_info);

            stpel = findBoundEl (arg_node, stp, k, arg_info);
            stpel = StepOrWidthHelper (stpel);
            widel = findBoundEl (arg_node, wid, k, arg_info);
            widel = StepOrWidthHelper (widel);

            res = PHUTcollectAffineExprsLocal (lbel, arg_info, res);
            res = PHUTcollectAffineExprsLocal (ubel, arg_info, res);

            // Generate new set variables for ISL. Apart from ISL analysis, these
            // names are unused. I.e., we do not generate any code that uses them.
            ivpavis
              = TBmakeAvis (TRAVtmpVarName ("iv"),
                            TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
            InsertVarIntoLut (ivpavis, INFO_VARLUT (arg_info));

            ivppavis
              = TBmakeAvis (TRAVtmpVarName ("ivp"),
                            TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
            InsertVarIntoLut (ivppavis, INFO_VARLUT (arg_info));

            ivwavis
              = TBmakeAvis (TRAVtmpVarName ("ivw"),
                            TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
            InsertVarIntoLut (ivwavis, INFO_VARLUT (arg_info));

            navis = TBmakeAvis (TRAVtmpVarName ("n"),
                                TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
            InsertVarIntoLut (navis, INFO_VARLUT (arg_info));

            // Generate: 0 <= ivw < wid
            z = BuildIslSimpleConstraint (TBmakeNum (0), F_le_SxS, ivwavis, F_lt_SxS,
                                          widel);
            res = TCappendExprs (res, z);

            // Generate iv' = lb + stp * N
            z = BuildIslNotSoSimpleConstraint (ivpavis, F_eq_SxS, DUPdoDupNode (lbel),
                                               F_add_SxS, stpel, F_mul_SxS, navis);
            res = TCappendExprs (res, z);

            // Generate iv'' = iv' + ivw
            z = BuildIslSimpleConstraint (ivppavis, F_eq_SxS, ivpavis, F_add_SxS,
                                          ivwavis);
            res = TCappendExprs (res, z);

            // Generate iv  = iv''
            z = BuildIslSimpleConstraint (ivavis, F_eq_SxS, ivppavis, NOPRFOP, NULL);
            res = TCappendExprs (res, z);

            // Generate lb <= iv < ub
            z = BuildIslSimpleConstraint (DUPdoDupNode (lbel), F_le_SxS, ivavis, F_lt_SxS,
                                          DUPdoDupNode (ubel));
            res = TCappendExprs (res, z);
        }
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn void Exprs2File( node *exprs, lut_t *varlut, char *tag)
 *
 *
 * @brief Append one ISL set of polyhedra to input file, from exprs and
 *        set variable list in varlut.
 *
 * @param handle: output file handle
 * @param exprs: N_exprs chain of N_exprs chains. Each element of exprs
 *               is a single constraint.
 *
 * @return void
 *
 ******************************************************************************/
static void
Exprs2File (FILE *handle, node *exprs, lut_t *varlut, char *tag)
{
    int i;
    int j;
    int k;
    int m;
    int n;
    char *v;
    node *idlist;
    node *expr;
    node *exprsone;
    char *txt;
    bool wasor;

    DBUG_ENTER ();

    idlist = GetIslSetVariablesFromLut (varlut);
    n = TCcountExprs (idlist);
    fprintf (handle, "{ [");

    // Append set variables:  [i,j,k...]
    for (i = 0; i < n; i++) {
        fprintf (handle, " %s", AVIS_NAME (ID_AVIS (TCgetNthExprsExpr (i, idlist))));
        if (i < (n - 1)) {
            fprintf (handle, ",");
        }
    }
    fprintf (handle, "] : \n");

    // Append constraints
    n = TCcountExprs (exprs);
    for (j = 0; j < n; j++) {
        wasor = FALSE;
        exprsone = TCgetNthExprsExpr (j, exprs);
        DBUG_ASSERT (N_exprs == NODE_TYPE (exprsone), "Wrong constraint type");
        m = TCcountExprs (exprsone);
        for (k = 0; k < m; k++) { // Emit one constraint
            expr = TCgetNthExprsExpr (k, exprsone);
            switch (NODE_TYPE (expr)) {
            default:
                DBUG_ASSERT (FALSE, "Unexpected constraint node type");
                break;

            case N_id:
                fprintf (handle, "%s", AVIS_NAME (ID_AVIS (expr)));
                break;

            case N_prf:
                if (F_or_SxS == PRF_PRF (expr)) {
                    wasor = TRUE;
                } else {
                    fprintf (handle, "%s", Prf2Isl (PRF_PRF (expr)));
                }
                break;

            case N_num:
                fprintf (handle, "%d", NUM_VAL (expr));
                break;

            case N_bool:
                v = BOOL_VAL (expr) ? "1" : "0";
                fprintf (handle, "%s", v);
                break;

            case N_char: // Support for disjunction because ISL does not support !=
                DBUG_ASSERT ('|' == CHAR_VAL (expr), "Expected disjunction |");
                wasor = TRUE;
                break;
            }
            fprintf (handle, " ");
        }
        if (j < (n - 1)) { // Handle conjunctions of constraints
            txt = wasor ? "\n   or " : "\n   and ";
            wasor = FALSE;
            fprintf (handle, "%s", txt);
        }
    }
    fprintf (handle, " } \n");

    DBUG_RETURN ();
}

/** <!-- ****************************************************************** -->
 *
 * @fn bool PHUTisCompatibleAffinePrf( prf nprf)
 *
 * @brief Predicate for affine node finding.
 *
 * @param A PRF_PRF
 *
 * @return TRUE if nprf member fns
 *
 ******************************************************************************/
bool
PHUTisCompatibleAffinePrf (prf nprf)
{
    bool z;

    DBUG_ENTER ();

    switch (nprf) {
    case F_idx_sel:
    case F_sel_VxA:
    case F_lt_SxS:
    case F_le_SxS:
    case F_eq_SxS:
    case F_ge_SxS:
    case F_gt_SxS:
    case F_neq_SxS:
    case F_val_lt_val_SxS:
    case F_val_le_val_SxS:
    case F_add_SxS:
    case F_sub_SxS:
    case F_mul_SxS:
    case F_non_neg_val_S:
    case F_min_SxS:
    case F_max_SxS:
    case F_mod_SxS:
    case F_aplmod_SxS:
    case F_abs_S:
    case F_neg_S:
    case F_shape_A:
    case F_not_S:
    case F_saabind:
        z = TRUE;
        break;

    default:
        z = FALSE;
        break;
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn bool isDyadicPrf( prf nprf)
 *
 * @brief Predicate for monadic vs. dyadic prfs
 *
 * @param An N_prf
 *
 * @return TRUE if nprf is dyadic
 *
 ******************************************************************************/
static bool
isDyadicPrf (prf nprf)
{
    bool z;

    DBUG_ENTER ();

    switch (nprf) {
    case F_lt_SxS:
    case F_le_SxS:
    case F_eq_SxS:
    case F_ge_SxS:
    case F_gt_SxS:
    case F_neq_SxS:
    case F_val_lt_val_SxS:
    case F_val_le_val_SxS:
    case F_add_SxS:
    case F_sub_SxS:
    case F_mul_SxS:
    case F_min_SxS:
    case F_max_SxS:
    case F_mod_SxS:
    case F_aplmod_SxS:
        z = TRUE;
        break;

    case F_abs_S: // Monadic
    case F_neg_S:
    case F_non_neg_val_S:
    case F_shape_A:
    case F_not_S:
        z = FALSE;
        break;

    default:
        z = FALSE;
        break;
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn bool isLoopfunCond( prf nprf)
 *
 * @brief Predicate to check for acceptable relational in LOOPFUN N_cond.
 *
 * @param An N_prf
 *
 * @return TRUE if nprf is legitimate for LOOPFUN
 *
 ******************************************************************************/
static bool
isLoopfunCond (prf nprf)
{
    bool z;

    DBUG_ENTER ();

    switch (nprf) {
    case F_lt_SxS:
    case F_le_SxS:
    case F_ge_SxS:
    case F_gt_SxS:
    case F_neq_SxS:
        z = TRUE;
        break;

    default:
        z = FALSE;
        break;
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn bool PHUTisCompatibleAffineTypes( node *arg_node)
 *
 * @brief ISL only supports integer (and Booleans, due to
 *        coercions made here), so we forbid other types.
 *
 * @param arg_node: An N_prf
 *
 * @return TRUE if arg_node arguments are all Boolean and/or integer
 *
 ******************************************************************************/
bool
PHUTisCompatibleAffineTypes (node *arg_node)
{
    bool z;
    node *avis;

    DBUG_ENTER ();

    avis = Node2Avis (PRF_ARG1 (arg_node));
    z = TUisBoolScalar (AVIS_TYPE (avis)) || TUisIntScalar (AVIS_TYPE (avis));
    if (isDyadicPrf (PRF_PRF (arg_node))) {
        avis = Node2Avis (PRF_ARG2 (arg_node));
        z = z && (TUisBoolScalar (AVIS_TYPE (avis)) || TUisIntScalar (AVIS_TYPE (avis)));
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *HandleNid( node *arg_node, node *rhs, info *arg_info, node *res)
 *
 * @brief Handler for N_id = N_id
 *        Generate: arg_node = rhs
 *
 * @param  arg_node: The N_avis for an assign with N_id as rhs
 * @param  rhs: The N_id
 * @param  arg_info: as usual
 * @param  res: the incoming N_exprs chain
 *
 * NB. In normal operation, calls to this code should be infrequent, due to
 *     the work done by CSE and VP.
 *
 ******************************************************************************/
static node *
HandleNid (node *arg_node, node *rhs, info *arg_info, node *res)
{
    node *z = NULL;

    DBUG_ENTER ();

    if (InsertVarIntoLut (rhs, INFO_VARLUT (arg_info))) {
        res = PHUTcollectAffineExprsLocal (rhs, arg_info, res);
        z = BuildIslSimpleConstraint (arg_node, F_eq_SxS, rhs, NOPRFOP, NULL);
        res = TCappendExprs (res, z);
    }

    DBUG_PRINT ("Leaving HandleNid for lhs=%s", AVIS_NAME (arg_node));

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node node *PHUThandleLoopfunArg( node *avis, info *arg_info, node *res)
 *
 * @brief avis is the N_avis of a LOOPFUN N_arg, and we are in a LOOPFUN.
 *
 * @param  avis: the N_avis for an N_id with no AVIS_SSAASSIGN, but we
 *         know that avis is not a WITHID element, so we assume
 *         we are in a LACFUN (or defined fun), looking at one of the
 *         function's arguments.
 * @param  arg_info: as usual
 * @param  res: Incoming N_exprs chain for ISL
 * @param  callerassign: The N_assign node of the external N_ap call to this
 *                       LACFUN
 * @param: callerfundef: The N_fundef node of the LACFUN's calling function
 * @param  initialvalue: The N_id node representing avis' initial value, from the
 *                       external caller.
 *
 * @return An N_exprs chain representing the ISL induction-variable constraint,
 *         appended to res.
 *         Otherwise, incoming res.
 *
 * NB. If the increment is positive, we generate:
 *
 *         rca' >= initialvalue
 *         rca' = initialvalue + N * increment
 *         rca' [dualprf] lim
 *         rca' = rca
 *
 *     If the increment is negative, we generate:
 *
 *         rca' <= initialvalue
 *         rca' = initialvalue + N * increment
 *         rca' [dualprf] lim
 *         rca' = rca
 *
 *      where N is unknown, but unimportant.
 *      dualprf is the dual to the relational function for the LACFUN's N_cond;
 *        e.g., if the condfun is _lt_SxS_, the dualprf is _ge_SxS.
 *      initialvalue is the initial value of the loop-induction variable,
 *      from the LACFUN caller.
 *
 *      lim is the non-induction-variable argument to the condfun.
 *
 ******************************************************************************/
static node *
PHUThandleLoopfunArg (node *avis, info *arg_info, node *res, node *callerassign,
                      node *callerfundef, node *initialvalue)
{
    node *rca = NULL;
    node *reccallargs = NULL;
    node *condprf;
    node *relarg1 = NULL;
    node *relarg2 = NULL;
    node *z = NULL;
    node *incrementnid;
    prf relfn;
    node *rcapp;
    node *navis;
    node *lim;
    node *zinit;
    node *ext_limit;
    int incrementsign = 0;
    bool swap;

    DBUG_ENTER ();

    DBUG_PRINT ("Looking at LOOPFUN %s", FUNDEF_NAME (INFO_FUNDEF (arg_info)));
    DBUG_ASSERT (N_avis == NODE_TYPE (initialvalue), "Expected avis for initialvalue");

    // If the variable is loop-independent, we are done, except for peeking outside.
    // See unit test ~/sac/testsuite/optimizations/pogorelationals/SCSprf_lt_SxS.LIR.sac
    reccallargs = LFUfindRecursiveCallAssign (INFO_FUNDEF (arg_info));
    reccallargs = AP_ARGS (LET_EXPR (ASSIGN_STMT (reccallargs)));
    rca = LFUgetLoopVariable (avis, INFO_FUNDEF (arg_info), reccallargs);
    DBUG_PRINT ("LACFUN arg %s has recursive call value of %s", AVIS_NAME (avis),
                AVIS_NAME (ID_AVIS (rca)));
    if (!LFUisLoopFunInvariant (INFO_FUNDEF (arg_info), avis, rca)) {
        // We are not done yet. Handle the recursive call.
        DBUG_PRINT ("LACFUN arg %s is loop-dependent", AVIS_NAME (avis));

        condprf = COND_COND (ASSIGN_STMT (LFUfindAssignForCond (INFO_FUNDEF (arg_info))));
        condprf = LET_EXPR (ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (condprf))));
        if ((NULL != condprf) && isLoopfunCond (PRF_PRF (condprf))) {
            DBUG_ASSERT (N_prf == NODE_TYPE (condprf), "Expected relational in LOOPFUN");
            lim = (ID_AVIS (rca) == ID_AVIS (PRF_ARG1 (condprf))) ? PRF_ARG2 (condprf)
                                                                  : PRF_ARG1 (condprf);
            relarg1 = PHUTgenerateAffineExprs (PRF_ARG1 (condprf), INFO_FUNDEF (arg_info),
                                               INFO_VARLUT (arg_info));

            relarg2 = PHUTgenerateAffineExprs (PRF_ARG2 (condprf), INFO_FUNDEF (arg_info),
                                               INFO_VARLUT (arg_info));

            // At this point, we need to know if the variable increment is
            // positive or negative. This loop-carried variable may, or may not, be
            // the induction variable.
            // If we get the increment from PRF_ARG1, we must swap the
            // order of the arguments to rca'' [dualprf] lim, below.
            swap = FALSE;
            incrementnid = LFUgetLoopIncrementFromIslChain (rca, relarg2);
            if (NULL == incrementnid) {
                incrementnid = LFUgetLoopIncrementFromIslChain (rca, relarg1);
                swap = TRUE;
            }

            relarg1 = RemoveRcaAssign (relarg1, rca);
            res = TCappendExprs (res, relarg1);
            relarg2 = RemoveRcaAssign (relarg2, rca);
            res = TCappendExprs (res, relarg2);

            if (NULL != incrementnid) {
                incrementsign = SCSisPositive (incrementnid)
                                  ? 1
                                  : SCSisNegative (incrementnid) ? -1 : 0;
            }

            if (0 != incrementsign) {
                // Leap into caller's world to collect the caller's AFT for the initial
                // value
                zinit = PHUTgenerateAffineExprs (initialvalue, callerfundef,
                                                 INFO_VARLUT (arg_info));
                res = TCappendExprs (res, zinit);

                // Leap into caller's world to collect the caller's AFT for the limit
                // Limit may be local or incoming
                ext_limit = LFUgetCallArg (lim, INFO_FUNDEF (arg_info), callerassign);
                ext_limit = (NULL != ext_limit) ? ext_limit : lim;
                ext_limit = ID_AVIS (ext_limit);
                zinit = PHUTgenerateAffineExprs (ext_limit, callerfundef,
                                                 INFO_VARLUT (arg_info));
                res = TCappendExprs (res, zinit);
                // Build lim = ext_limit
                z = BuildIslSimpleConstraint (ID_AVIS (lim), F_eq_SxS, ext_limit, NOPRFOP,
                                              NULL);
                res = TCappendExprs (res, z);

                // Build: rca' <= initialvalue   or   rca' >= initialvalue
                relfn = (incrementsign < 0) ? F_le_SxS : F_ge_SxS;
                rcapp
                  = TBmakeAvis (TRAVtmpVarName ("rcapp"),
                                TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
                InsertVarIntoLut (rcapp, INFO_VARLUT (arg_info));
                z = BuildIslSimpleConstraint (rcapp, relfn, initialvalue, NOPRFOP, NULL);
                res = TCappendExprs (res, z);

                // Build increment:  rca' = initialvalue + N * increment
                navis
                  = TBmakeAvis (TRAVtmpVarName ("N"),
                                TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
                InsertVarIntoLut (navis, INFO_VARLUT (arg_info));
                z = BuildIslNotSoSimpleConstraint (rcapp, F_eq_SxS, initialvalue,
                                                   F_add_SxS, navis, F_mul_SxS,
                                                   DUPdoDupNode (incrementnid));
                res = TCappendExprs (res, z);

                // Build rca'' [dualprf] lim
                if (swap) {
                    z = BuildIslSimpleConstraint (rcapp, PRF_PRF (condprf),
                                                  DUPdoDupNode (lim), NOPRFOP, NULL);
                } else {
                    z = BuildIslSimpleConstraint (DUPdoDupNode (lim), PRF_PRF (condprf),
                                                  rcapp, NOPRFOP, NULL);
                }
                res = TCappendExprs (res, z);

                // Build rca'' = incoming-loop-carried value
                z = BuildIslSimpleConstraint (rcapp, F_eq_SxS, avis, NOPRFOP, NULL);
                res = TCappendExprs (res, z);
            }
        }
    } else {
        DBUG_PRINT ("LACFUN arg %s is loop-independent", AVIS_NAME (avis));
        // Leap into caller's world to collect the AFT
        z = PHUTgenerateAffineExprs (initialvalue, callerfundef, INFO_VARLUT (arg_info));
        res = TCappendExprs (res, z);
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node node *PHUThandleCondfunArg()
 *
 * @brief avis is the N_avis of an N_arg, and we are in a CONDFUN.
 *
 * @param  avis: the N_avis for an N_id with no AVIS_SSAASSIGN, but we
 *         know that avis is not a WITHID element, so we assume
 *         we are in a LACFUN (or defined fun), looking at one of the
 *         function's arguments.
 *
 *         Our unit test is ~/sac/testsuite/optimizations/pogo/condfun.sac
 *
 * @param  arg_info: as usual
 * @param  res: Incoming N_exprs chain for ISL
 * @param  callerassign: The N_assign node of the external N_ap call to this
 *                       LACFUN
 * @param: callerfundef: The N_fundef node of the LACFUN's calling function
 * @param  initialvalue:  The N_id node representing avis' value from the
 *                       external caller.
 *
 * @return An N_exprs chain representing an ISL constraint, appended to res.
 *         Otherwise, incoming res.
 *
 ******************************************************************************/
static node *
PHUThandleCondfunArg (node *avis, info *arg_info, node *res, node *callerassign,
                      node *callerfundef, node *initialvalue)
{
    node *z = NULL;

    DBUG_ENTER ();

    DBUG_PRINT ("Looking at CONDFUN %s", FUNDEF_NAME (INFO_FUNDEF (arg_info)));

    // Leap into caller's world to collect the AFT
    z = PHUTgenerateAffineExprs (initialvalue, callerfundef, INFO_VARLUT (arg_info));
    res = TCappendExprs (res, z);

    // we now have the AFT from the caller's environment. We now
    // emit one more constraint, to make internal_name == external_name.
    z = BuildIslSimpleConstraint (avis, F_eq_SxS, initialvalue, NOPRFOP, NULL);
    res = TCappendExprs (res, z);

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node node *PHUThandleLacfunArg()
 *
 * @brief avis may be the N_avis of an N_arg, if we are in a LACFUN.
 *
 * @param  avis: the N_avis for an N_id with no AVIS_SSAASSIGN, but we
 *         know that avis is not a WITHID element, so we assume
 *         we are in a LACFUN (or defined fun), looking at one of the
 *         function's arguments.
 *
 *         We should be able to track LOOPFUNs and CONDFUNs, but today,
 *         we'll start by looking at CONDFUNs only. Our unit test is
 *         ~/sac/testsuite/optimizations/pogo/condfun.sac.
 *
 * @param  arg_info: as usual
 * @param  res: Incoming N_exprs chain for ISL
 *
 * @return An N_exprs chain representing an ISL constraint, appended to res.
 *         Otherwise, incoming res.
 *
 ******************************************************************************/
static node *
PHUThandleLacfunArg (node *avis, info *arg_info, node *res)
{
    node *ext_assign;
    node *initialvalue;
    node *callerfundef;

    DBUG_ENTER ();

    // N_assign of External call (N_ap) to LACFUN
    ext_assign = FUNDEF_CALLAP (INFO_FUNDEF (arg_info));
    if (NULL != ext_assign) { // Current function may not be a LACFUN
        callerfundef = FUNDEF_CALLERFUNDEF (INFO_FUNDEF (arg_info));
        DBUG_ASSERT (N_assign == NODE_TYPE (ext_assign),
                     "Expected FUNDEF_CALLAP to be N_assign");
        DBUG_ASSERT (AP_FUNDEF (LET_EXPR (ASSIGN_STMT (ext_assign)))
                       == INFO_FUNDEF (arg_info),
                     "Expected FUNDEF_CALLAP to point to its N_ap node");

        // Find caller's value for avis.
        initialvalue = ID_AVIS (LFUgetCallArg (avis, INFO_FUNDEF (arg_info), ext_assign));
        DBUG_PRINT ("Building affine function for LACFUN variable %s from caller's %s",
                    AVIS_NAME (avis), AVIS_NAME (initialvalue));

        if (FUNDEF_ISCONDFUN (INFO_FUNDEF (arg_info))) {
            res = PHUThandleCondfunArg (avis, arg_info, res, ext_assign, callerfundef,
                                        initialvalue);
        }

        if (FUNDEF_ISLOOPFUN (INFO_FUNDEF (arg_info))) {
            res = PHUThandleLoopfunArg (avis, arg_info, res, ext_assign, callerfundef,
                                        initialvalue);
        }
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *HandleNumber( node *arg_node, node *rhs, info *arg_info, node *res)
 *
 * @brief Handler for N_id = N_num or N_bool
 *
 * @param  arg_node: The N_avis for an assign with N_num/N_bool as rhs
 * @param  rhs: The N_num/N_bool
 *         If rhs is NULL, then arg_node may be AKV, in which we
 *         treat its value as rhs.
 * @param  arg_info: as usual
 * @param  res: Incoming N_exprs chain
 *
 * @return An N_exprs chain representing an ISL constraint, appended to res.
 *         All the N_exprs chain values will be N_num or N_bool nodes.
 *         Otherwise, incoming res.
 *
 ******************************************************************************/
static node *
HandleNumber (node *arg_node, node *rhs, info *arg_info, node *res)
{
    node *z;

    DBUG_ENTER ();

    DBUG_PRINT ("HandleNumber for lhs=%s", AVIS_NAME (arg_node));
    if ((NULL == rhs) && TYisAKV (AVIS_TYPE (arg_node))) {
        rhs = TBmakeNum (TUtype2Int (AVIS_TYPE (arg_node)));
    }
    z = BuildIslSimpleConstraint (arg_node, F_eq_SxS, rhs, NOPRFOP, NULL);
    res = TCappendExprs (res, z);

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *HandleComposition( node *arg_node, node *rhs, info *arg_info, node *res)
 *
 * @brief Handler for compositions of primitive functions.
 *        Currently supported:
 *          shpel = idx_sel( offset, _shape_A_( var));
 *          shpel = _sel_VxA_( iv, _shape_A_( var));
 *          Both of these will generate shpel >= 0.
 *
 *
 * @param  arg_node: The N_avis for an assign.
 * @param  rhs: The assign's rhs
 * @param  arg_info: as usual
 * @param  res: Incoming N_exprs chain
 *
 * @return An N_exprs chain (or NULL)
 *         representing an ISL constraint, appended to res.
 *         Otherwise, incoming res.
 *
 ******************************************************************************/
static node *
HandleComposition (node *arg_node, node *rhs, info *arg_info, node *res)
{
    node *z = NULL;
    pattern *pat1;
    pattern *pat2;
    pattern *pat3;
    node *iv = NULL;
    node *x = NULL;
    node *m = NULL;

    DBUG_ENTER ();

    /* z = _sel_VxA_( iv, x); */
    pat1 = PMprf (1, PMAisPrf (F_sel_VxA), 2, PMvar (1, PMAgetNode (&iv), 0),
                  PMvar (1, PMAgetNode (&x), 0));
    /* z = _idx_sel( offset, x); */
    pat2 = PMprf (1, PMAisPrf (F_idx_sel), 2, PMvar (1, PMAgetNode (&iv), 0),
                  PMvar (1, PMAgetNode (&x), 0));
    /* x = _shape_A_( m ); */
    pat3 = PMprf (1, PMAisPrf (F_shape_A), 1, PMvar (1, PMAgetNode (&m), 0));

    if ((PMmatchFlat (pat1, rhs) || PMmatchFlat (pat2, rhs)) && PMmatchFlat (pat3, x)) {
        z = BuildIslSimpleConstraint (arg_node, F_ge_SxS, TBmakeNum (0), NOPRFOP, NULL);
        res = TCappendExprs (res, z);
    }
    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);

    DBUG_PRINT ("Leaving HandleComposition for lhs=%s", AVIS_NAME (arg_node));

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *HandleNprf( node *arg_node, node *rhs info *arg_info, node *res)
 *
 * @brief If rhs is an affine function, generate ISL contraints for it.
 *
 *         z = x + y;
 *         z = x - y;
 *         z = x * const
 *         z = const * x;
 *         z =   - y;
 *         z = non_neg_val( y);
 *
 *
 * @param  arg_node: The N_avis for an assign with N_prf as rhs
 * @param  rhs; the rhs of the N_assign
 * @param  arg_info: as usual
 * @param  res: Incoming N_exprs chain
 *
 * @return An N_exprs chain, representing a polylib Matrix row, appended to res.
 *         All the N_exprs chain values will be N_num nodes.
 *
 ******************************************************************************/
static node *
HandleNprf (node *arg_node, node *rhs, info *arg_info, node *res)
{
    node *assgn = NULL;
    node *ids;
    node *argavis;
    node *z = NULL;
    node *nid;
    pattern *pat;

    DBUG_ENTER ();

    pat = PMvar (1, PMAgetNode (&assgn), 0);

    nid
      = (N_avis == NODE_TYPE (arg_node)) ? TBmakeId (arg_node) : DUPdoDupNode (arg_node);
    if (PMmatchFlatSkipGuards (pat, nid)) {
        assgn = AVIS_SSAASSIGN (ID_AVIS (assgn));
        ids = LET_IDS (ASSIGN_STMT (assgn));
    }
    nid = FREEdoFreeNode (nid);
    pat = PMfree (pat);

    if ((NULL != assgn) && (PHUTisCompatibleAffinePrf (PRF_PRF (rhs)))
        && (PHUTisCompatibleAffineTypes (rhs))) {
        DBUG_PRINT ("Entering HandleNprf for ids=%s", AVIS_NAME (IDS_AVIS (ids)));

        // Deal with PRF_ARGs
        if (F_saabind == PRF_PRF (rhs)) {
            res = PHUTcollectAffineExprsLocal (PRF_ARG3 (rhs), arg_info, res);
        } else {
            res = PHUTcollectAffineExprsLocal (PRF_ARG1 (rhs), arg_info, res);
            if (isDyadicPrf (PRF_PRF (rhs))) {
                res = PHUTcollectAffineExprsLocal (PRF_ARG2 (rhs), arg_info, res);
            }
        }

        switch (PRF_PRF (rhs)) {
        case F_add_SxS:
        case F_sub_SxS:
            z = BuildIslSimpleConstraint (ids, F_eq_SxS, PRF_ARG1 (rhs), PRF_PRF (rhs),
                                          PRF_ARG2 (rhs));
            res = TCappendExprs (res, z);
            break;

        case F_non_neg_val_S:
            z = BuildIslSimpleConstraint (ids, F_eq_SxS, PRF_ARG1 (rhs), NOPRFOP, NULL);
            res = TCappendExprs (res, z);
            break;

        case F_max_SxS: // z = max( x, y)  --> ( z >= x) and ( z >= y)
            z = BuildIslSimpleConstraint (ids, F_ge_SxS, PRF_ARG1 (rhs), NOPRFOP, NULL);
            res = TCappendExprs (res, z);
            z = BuildIslSimpleConstraint (ids, F_ge_SxS, PRF_ARG2 (rhs), NOPRFOP, NULL);
            res = TCappendExprs (res, z);
            break;

        case F_min_SxS: // z = min( x, y) --> ( z <= x) and ( z <= y)
            z = BuildIslSimpleConstraint (ids, F_le_SxS, PRF_ARG1 (rhs), NOPRFOP, NULL);
            res = TCappendExprs (res, z);
            z = BuildIslSimpleConstraint (ids, F_le_SxS, PRF_ARG2 (rhs), NOPRFOP, NULL);
            res = TCappendExprs (res, z);
            break;

        case F_mul_SxS: // We need one constant argument
            argavis = ID_AVIS (PRF_ARG1 (rhs));
            if ((TYisAKV (AVIS_TYPE (argavis)))
                || (TYisAKV (AVIS_TYPE (ID_AVIS (PRF_ARG2 (rhs)))))) {
                // z = ( const * y)
                z = BuildIslSimpleConstraint (ids, F_eq_SxS, PRF_ARG1 (rhs),
                                              PRF_PRF (rhs), PRF_ARG2 (rhs));
                res = TCappendExprs (res, z);
            }
            break;

        case F_shape_A:
            argavis = ID_AVIS (PRF_ARG1 (rhs));
            if ((TYisAKS (AVIS_TYPE (argavis))) || (TYisAKV (AVIS_TYPE (argavis)))) {
                // z >= 0
                z = BuildIslSimpleConstraint (ids, F_ge_SxS, TBmakeNum (0), NOPRFOP,
                                              NULL);
                res = TCappendExprs (res, z);
                // maximum: z < shape
                if ((NULL != AVIS_SHAPE (argavis))
                    && (N_id == NODE_TYPE (AVIS_SHAPE (argavis)))) {
                    z = BuildIslSimpleConstraint (ids, F_lt_SxS, AVIS_SHAPE (argavis),
                                                  NOPRFOP, NULL);
                    res = TCappendExprs (res, z);
                }
            }
            break;

        case F_not_S: // Treat Booleans as integers(for POGO)
            // z = !y  --->  ( z = 1 - y) and (y >= 0) and (y <= 1)
            z = BuildIslSimpleConstraint (ids, F_eq_SxS, TBmakeNum (1), F_sub_SxS,
                                          PRF_ARG1 (rhs));
            res = TCappendExprs (res, z);
            z = BuildIslSimpleConstraint (PRF_ARG1 (rhs), F_ge_SxS, TBmakeNum (0),
                                          NOPRFOP, NULL);
            res = TCappendExprs (res, z);
            z = BuildIslSimpleConstraint (PRF_ARG1 (rhs), F_le_SxS, TBmakeNum (1),
                                          NOPRFOP, NULL);
            res = TCappendExprs (res, z);
            break;

        case F_abs_S: // z >= 0
            z = BuildIslSimpleConstraint (ids, F_ge_SxS, TBmakeNum (0), NOPRFOP, NULL);
            res = TCappendExprs (res, z);
            break;

        case F_aplmod_SxS:
            /*
             *               Per the APL ISO N8485 standard:
             *               For aplmod modulus: z = aplmod( x, y),
             *               if ((x >= 0) && (y >= 0)) || (y >= 1)
             *               then z >= 0.
             */
            // mimumum
            if (((SCSisNonneg (PRF_ARG1 (rhs))) && (SCSisNonneg (PRF_ARG2 (rhs))))
                || (SCSisPositive (PRF_ARG2 (rhs)))) {
                z = BuildIslSimpleConstraint (ids, F_ge_SxS, TBmakeNum (0), NOPRFOP,
                                              NULL);
                res = TCappendExprs (res, z);
            }

            // maximum
            // (x>0) & (y>0) -->   z <=  y-1
            if ((SCSisPositive (PRF_ARG2 (rhs))) && (SCSisPositive (PRF_ARG1 (rhs)))) {
                z = BuildIslSimpleConstraint (ids, F_le_SxS, PRF_ARG2 (rhs), F_sub_SxS,
                                              TBmakeNum (1));
                res = TCappendExprs (res, z);
            }
            break;

        case F_mod_SxS:
            /*
             *               For C99, if (arg1 >=0 ) && ( arg2 > 0), then:
             *                        0 <= z < arg2
             *
             *               Apparently C99 chooses result sign to match
             *               that of PRF_ARG1, but we sidestep that issue here.
             *               FIXME: Perhaps extend this code to deal with negative
             *               PRF_ARG1?
             */
            // minimum
            if ((SCSisNonneg (PRF_ARG1 (rhs))) && (SCSisPositive (PRF_ARG2 (rhs)))) {
                // z >= 0
                z = BuildIslSimpleConstraint (ids, F_ge_SxS, TBmakeNum (0), NOPRFOP,
                                              NULL);
                res = TCappendExprs (res, z);
                // z < PRF_ARG2( rhs)
                z = BuildIslSimpleConstraint (ids, F_lt_SxS, PRF_ARG2 (rhs), NOPRFOP,
                                              NULL);
                res = TCappendExprs (res, z);
            }
            break;

        case F_lt_SxS:
        case F_le_SxS:
        case F_eq_SxS:
        case F_ge_SxS:
        case F_gt_SxS:
        case F_neq_SxS:
            z = BuildIslSimpleConstraint (ids, F_eq_SxS, PRF_ARG1 (rhs), PRF_PRF (rhs),
                                          PRF_ARG2 (rhs));
            res = TCappendExprs (res, z);
            break;

        case F_val_lt_val_SxS:
        case F_val_le_val_SxS:
            // Treat a guard within an affine expression as assignment.
            // z = PRF_ARG1;
            z = BuildIslSimpleConstraint (ids, F_eq_SxS, PRF_ARG1 (rhs), NOPRFOP, NULL);
            res = TCappendExprs (res, z);
            break;

        case F_neg_S: //  z = -y;  -->  z = 0 - y
            z = BuildIslSimpleConstraint (ids, F_eq_SxS, TBmakeNum (0), F_sub_SxS,
                                          PRF_ARG1 (rhs));
            res = TCappendExprs (res, z);
            break;

        case F_idx_sel:
        case F_sel_VxA:
            break;

        case F_saabind: // z = PRF_ARG3( rhs);
            // We need to capture the relationship among initial and final values
            // of LACFUN parameters. E.g., in simpleNonconstantUp.sac, we have a
            // loop from START to START+5, where the value of START is the result
            // of a function call [id(0)].
            z = BuildIslSimpleConstraint (ids, F_eq_SxS, PRF_ARG3 (rhs), NOPRFOP, NULL);
            res = TCappendExprs (res, z);
            break;

        default:
            DBUG_UNREACHABLE ("Nprf coding time, senor");
            break;
        }
    }

    DBUG_PRINT ("Leaving HandleNprf for ids=%s", AVIS_NAME (IDS_AVIS (ids)));

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn node *PHUTcollectAffineExprsLocal( node *arg_node, info *arg_info, node *res)
 *
 * @brief Does a recursive search, starting at arg_node,
 *        for a maximal chain of affine instructions.
 *
 * @param arg_node: an N_id node or an N_avis node or an N_ids node
 *        arg_info: External callers use NULL; we use
 *                  our own arg_info for traversal marking.
 * @param res: Partially built N_exprs chain
 *
 * @return A maximal N_exprs chain of expressions, appended to the incoming res.
 *
 *         See PHUTcollectExprs for more details.
 *
 ******************************************************************************/
node *
PHUTcollectAffineExprsLocal (node *arg_node, info *arg_info, node *res)
{
    node *assgn = NULL;
    node *rhs = NULL;
    node *avis = NULL;
    node *npart = NULL;

    DBUG_ENTER ();

    avis = Node2Avis (arg_node);
    if (NULL != avis) { // N_num exits here
        DBUG_PRINT ("Looking at %s", AVIS_NAME (avis));
        if (InsertVarIntoLut (avis, INFO_VARLUT (arg_info))) {

            // Handle RHS for all modes.
            assgn = AVIS_SSAASSIGN (avis);
            if ((NULL != assgn) && (N_let == NODE_TYPE (ASSIGN_STMT (assgn)))) {

                rhs = LET_EXPR (ASSIGN_STMT (assgn));

                switch (NODE_TYPE (rhs)) {
                case N_id: // straight assign:  var2 = var1;
                    res = HandleNid (avis, rhs, arg_info, res);
                    break;

                case N_prf:
                    res = HandleNprf (avis, rhs, arg_info, res);
                    res = HandleComposition (avis, rhs, arg_info, res);
                    break;

                case N_bool:
                case N_num:
                    res = HandleNumber (avis, rhs, arg_info, res);
                    break;

                default:
                    break;
                }
            } else {
                DBUG_ASSERT (NULL == assgn, "Confusion about AVIS_SSAASSIGN");
                // This may be a WITHID or a function parameter.
                // If it is constant, then we treat it as a number.
                if (TYisAKV (AVIS_TYPE (avis))) {
                    res = HandleNumber (avis, rhs, arg_info, res);
                } else {
                    // This presumes that AVIS_NPART is being maintained by our caller.
                    npart = AVIS_NPART (avis);
                    if (NULL != npart) {
                        if (-1
                            != LFUindexOfMemberIds (avis,
                                                    WITHID_IDS (PART_WITHID (npart)))) {
                            // arg_node is a withid element.
                            res = PHUTcollectWlGenerator (avis, arg_info, res);
                        } else {
                            // non-constant function parameter
                            DBUG_ASSERT (FALSE, "Coding time for lacfun args");
                        }
                    } else {
                        res = PHUThandleLacfunArg (avis, arg_info, res);
                    }
                }
            }
        }
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn node *PHUTgenerateAffineExprs( node *arg_node, node *fundef, int *numvars)
 *
 * @brief Does a recursive search, starting at arg_node,
 *        for a maximal chain of affine instructions.
 *
 * @param arg_node: an N_id node or N_avis node.
 * @param fundef: The N_fundef for the current function. Used only for debugging.
 * @param numvars: the number of variables in the resulting polylib matrix
 *
 * @return A maximal N_exprs chain of expressions, whose elements may be
 *         either N_num or N_id nodes.
 *
 * Some examples:
 *
 * This is nakedCWLSimple.sac, from PWLF unit tests:
 *
 * int main()
 * {
 *   XXX = Array::iota(50);
 *   z = _sel_VxA_( [42], XXX);
 *   z = _sub_SxS_( z, 42);
 *   return(z);
 * }
 *
 * The naked _sel_() (Effectively, the consumerWL) has a polyhedron of 42;
 * XXX has generators of [0] and [50];
 *
 * PWLF has to compute the lower and upper bounds of the intersection of
 * these.
 *
 *
 * This is from AWLF unit test time2code.sac. If compiled with -doawlf -nowlf,
 * it generates this code:
 *
 *     p = _aplmod_SxS_( -1, m);   NB. AVIS_MIN(p) = 0;
 *     iv2 = _notemaxval( iv1, m); NB. aka AVIS_MAX( iv2) = m;
 *     iv3 = iv2 - p;
 *     iv4, p = _val_lt_val_SxS_( iv3, m);
 *
 *  What we want to generate for this, eventually, is two sets
 *  of constraints:
 *
 * Set A:
 *   AVIS_MIN(p)                  -->   p   >= 0
 *   iv3 = iv2 - p;               -->   iv3 = iv2 - p
 *   iv2 = _notemaxval( iv1, m);  -->   m   >= iv2 + 1
 *
 * Set B:
 *   val_lt_val_SxS_( iv3, m)     -->   iv3 >= m    NB. Must prove converse
 *
 ******************************************************************************/
node *
PHUTgenerateAffineExprs (node *arg_node, node *fundef, lut_t *varlut)
{
    node *res = NULL;
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();
    INFO_FUNDEF (arg_info) = fundef;
    INFO_VARLUT (arg_info) = varlut;

    res = PHUTcollectAffineExprsLocal (arg_node, arg_info, res);
    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn node *PHUTgenerateAffineExprsForGuard(...)
 *
 * @brief Construct an ISL matrix for an N_prf guard or relational
 *
 * @param arg_node: an N_prf for a guard/relational primitive
 * @param fundef: The N_fundef for the current function, for debugging
 * @param relfn:  Either PRF_PRF( arg_node) or its companion function,
 *                e.g., if PRF_PRF is _gt_SxS_, its companion is _le_SxS.
 * @param exprsUcfn: An implicit result, set if PRF_PRF( arg_node) is _eq_SxS.
 * @param exprsUfn: An implicit result, set if PRF_PRF( arg_node) is _eq_SxS.
 *
 * @return A maximal N_exprs chain of expressions for the guard.
 *
 ******************************************************************************/
node *
PHUTgenerateAffineExprsForGuard (node *arg_node, node *fundef, prf relfn, node **exprsUfn,
                                 node **exprsUcfn, lut_t *varlut)
{
    node *z = NULL;
    node *z2 = NULL;
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_prf, "Expected N_prf node");

    InsertVarIntoLut (PRF_ARG1 (arg_node), varlut);
    arg_info = MakeInfo ();
    INFO_FUNDEF (arg_info) = fundef;
    switch (relfn) {
    case F_non_neg_val_S:
        z = BuildIslSimpleConstraint (PRF_ARG1 (arg_node), F_ge_SxS, TBmakeNum (0),
                                      NOPRFOP, NULL);
        break;

    case F_gt_SxS:
    case F_ge_SxS:
    case F_val_lt_val_SxS:
    case F_lt_SxS:
    case F_val_le_val_SxS:
    case F_le_SxS:
    case F_eq_SxS:
        if (F_non_neg_val_S != PRF_PRF (arg_node)) { // kludge for monadic CFN
            InsertVarIntoLut (PRF_ARG2 (arg_node), varlut);
            z = BuildIslSimpleConstraint (PRF_ARG1 (arg_node), relfn, PRF_ARG2 (arg_node),
                                          NOPRFOP, NULL);
        } else {
            z = BuildIslSimpleConstraint (PRF_ARG1 (arg_node), F_lt_SxS, TBmakeNum (0),
                                          NOPRFOP, NULL);
        }
        break;

    case F_neq_SxS:
        // This is harder. We need to construct a union
        // of (x<y) OR (x>y).
        InsertVarIntoLut (PRF_ARG2 (arg_node), varlut);
        z = BuildIslSimpleConstraint (PRF_ARG1 (arg_node), F_gt_SxS, PRF_ARG2 (arg_node),
                                      F_or_SxS, NULL);
        z2 = BuildIslSimpleConstraint (PRF_ARG1 (arg_node), F_lt_SxS, PRF_ARG2 (arg_node),
                                       NOPRFOP, NULL);
        z = TCappendExprs (z, z2);
        break;

    default:
        DBUG_UNREACHABLE ("Coding time for guard polyhedron");
        break;
    }

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node
 *
 * @brief Compute the intersections amongn ISL polyhedra:
 *
 *          exprs1, exprs2, exprs3
 *
 *        and, if exprs4 != NULL:
 *
 *          exprs1, exprs2, exprs4
 *
 *        The result is an element of sacpolylibinterface.h.
 *
 *        The monotonically increasing global.polylib_filenumber
 *        is for debugging, as we will generate MANY files.
 *
 * @param: exprspwl, exprscwl, exprs3, exprs4 - integer N_exprs chains of
 *         n ISL input sets.
 * @param: varlut: a lut containing the N_avis names referenced
 *         by the affine function trees in the exprs chains.
 *         This becomes the "set variable" list for ISL.
 * @param: opcode - the POLY_OPCODE to select a sacislinerface operation.
 *
 * @result: a POLY_RET return code
 *
 *****************************************************************************/
int
PHUTcheckIntersection (node *exprspwl, node *exprscwl, node *exprs3, node *exprs4,
                       node *exprsuf, node *exprsuc, lut_t *varlut, char opcode)
{
#define MAXLINE 1000
    int res = POLY_RET_INVALID;
    FILE *matrix_file;
    FILE *res_file;
    char polyhedral_arg_filename[PATH_MAX];
    char polyhedral_res_filename[PATH_MAX];
    static const char *argfile = "polyhedral_args";
    static const char *resfile = "polyhedral_res";
    char buffer[MAXLINE];
    int exit_code;

    DBUG_ENTER ();

    if (!global.cleanup) { // If -d nocleanup, keep all ISL files
        global.polylib_filenumber++;
    }

    sprintf (polyhedral_arg_filename, "%s/%s%d.arg", global.tmp_dirname, argfile,
             global.polylib_filenumber);
    sprintf (polyhedral_res_filename, "%s/%s%d.res", global.tmp_dirname, resfile,
             global.polylib_filenumber);

    DBUG_PRINT ("ISL arg filename: %s", polyhedral_arg_filename);
    DBUG_PRINT ("ISL res filename: %s", polyhedral_res_filename);
    matrix_file = FMGRwriteOpen (polyhedral_arg_filename, "w");
    Exprs2File (matrix_file, exprspwl, varlut, "pwl");
    Exprs2File (matrix_file, exprscwl, varlut, "cwl");
    Exprs2File (matrix_file, exprs3, varlut, "eq");
    Exprs2File (matrix_file, exprs4, varlut, "cfn");
    Exprs2File (matrix_file, exprsuf, varlut, "ufn");
    Exprs2File (matrix_file, exprsuc, varlut, "ucfn");
    FMGRclose (matrix_file);

    // We depend on PATH to find the sacpolylibinterface binary
    DBUG_PRINT ("calling " SACISLINTERFACEBINARY);
    exit_code = SYScallNoErr (SACISLINTERFACEBINARY " %c < %s > %s\n", opcode,
                              polyhedral_arg_filename, polyhedral_res_filename);
    DBUG_PRINT ("exit_code=%d, WIFEXITED=%d, WIFSIGNALED=%d, WEXITSTATUS=%d", exit_code,
                WIFEXITED (exit_code), WIFSIGNALED (exit_code), WEXITSTATUS (exit_code));
    res_file = FMGRreadOpen (polyhedral_res_filename);
    res = atoi (fgets (buffer, MAXLINE, res_file));
    DBUG_PRINT ("intersection result is %d", res);
    FMGRclose (res_file);

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn node *PHUTgenerateAffineExprsForPwl( node *arg_node, node *fundef, lut_t *varlut)
 *
 * @brief Construct ISL input N_exprs chain for producerWL
 *
 * @param arg_node: an N_part
 * @param fundef: The N_fundef for the current function.
 *                Used only for debugging.
 * @param varlut: The lut used to collect set variable names
 *
 * @return A maximal N_exprs chain of expressions for the Producer-WL partition.
 *
 ******************************************************************************/
node *
PHUTgenerateAffineExprsForPwl (node *arg_node, node *fundef, lut_t *varlut)
{
    node *z = NULL;
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_part, "Expected N_part node");

    arg_info = MakeInfo ();
    INFO_FUNDEF (arg_info) = fundef;

    DBUG_ASSERT (FALSE, "CODING TIME");

    InsertVarIntoLut (arg_node, INFO_VARLUT (arg_info));
    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 * @fn node *PHUTgenerateAffineExprsForCwl( node *arg_node, node *fundef, lut_t *varlut)
 *
 * @brief Construct an ISL constraint for a Consumer with-loop _sel_() expression.
 *
 * @param arg_node: an N_prf for an _sel_VxA_( iv, PWL)
 * @param fundef: The N_fundef for the current function.
 *                Used only for debugging.
 * @param varlut: the lut used to collect the list of set variables
 *
 * @return A maximal N_exprs chain of expressions for the Consumer-WL sel expn.
 *
 ******************************************************************************/
node *
PHUTgenerateAffineExprsForCwl (node *arg_node, node *fundef, lut_t *varlut)
{
    node *res = NULL;
    info *arg_info;
    node *exprs1 = NULL;
    node *exprs2 = NULL;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_prf, "Expected N_prf node");
    DBUG_ASSERT (F_sel_VxA == PRF_PRF (arg_node), "Expected _sel_VxA N_prf");

    arg_info = MakeInfo ();
    INFO_FUNDEF (arg_info) = fundef;
    InsertVarIntoLut (arg_node, INFO_VARLUT (arg_info));
    exprs1 = PHUTgenerateAffineExprs (PRF_ARG1 (arg_node), fundef, varlut);
    exprs2 = PHUTgenerateAffineExprs (PRF_ARG2 (arg_node), fundef, varlut);
    res = BuildIslSimpleConstraint (PRF_ARG1 (arg_node), F_eq_SxS, PRF_ARG2 (arg_node),
                                    NOPRFOP, NULL);

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn node *PHUTgenerateAffineExprsForPwlfIntersect( node *cwliv, node *pwliv,
 *           lut_t *varlut)
 *
 * @brief Construct an ISL constraint for the intersect of a consumerWL
 *
 * @param cwliv: The N_avis for the consumerWL index vector.
 * @param pwliv: THe N_avis for the producerWL generator index vector
 * @param varlut: The lut we use to collect set variable names.
 *
 * @return An N_exprs chain for the intersect, stating that:
 *         cwliv == pwliv
 *
 ******************************************************************************/
node *
PHUTgenerateAffineExprsForPwlfIntersect (node *cwliv, node *pwliv, lut_t *varlut)
{
    node *res;

    DBUG_ENTER ();

    InsertVarIntoLut (cwliv, varlut);
    InsertVarIntoLut (pwliv, varlut);
    res = BuildIslSimpleConstraint (cwliv, F_eq_SxS, pwliv, NOPRFOP, NULL);

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn node *PHUTsetClearAvisPart( node *arg_node, node *val)
 *
 * @brief Set or clear AVIS_NPART attributes for arg_node's WITHID_IDS,
 *        WITHID_VEC, and WITHID_IDXS to val.
 *
 * @param arg_node: The N_part of a WL
 * @param val: Either NULL or the address of the N_part.
 *
 * @return arg_node, unchanged. Side effects on the relevant N_avis nodes.
 *
 ******************************************************************************/
node *
PHUTsetClearAvisPart (node *arg_node, node *val)
{
    node *ids;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_part, "Expected N_part node");

    AVIS_NPART (IDS_AVIS (WITHID_VEC (PART_WITHID (arg_node)))) = val;

    ids = WITHID_IDS (PART_WITHID (arg_node));
    while (NULL != ids) {
        AVIS_NPART (IDS_AVIS (ids)) = val;
        ids = IDS_NEXT (ids);
    }

    ids = WITHID_IDXS (PART_WITHID (arg_node));
    while (NULL != ids) {
        AVIS_NPART (IDS_AVIS (ids)) = val;
        ids = IDS_NEXT (ids);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node node *PHUTsetClearCallAp( node *arg_node, node *callerfundef, node *nassign)
 *
 * @brief Set or clear FUNDEF_CALLAP in a LACFUN
 *        "nassign" should be set to the external function's N_assign node
 *        for the LACFUN's N_ap call, when the caller is being traversed,
 *        and NULLed afterward. The LACFUN can then be called to do its thing,
 *        and when that completes, it is nulled by the caller.
 *        FUNDEF_CALLAP and FUNDEF_CALLERFUNDEF are used within a LACFUN
 *        to find the argument list and fundef entry of its calling function.
 *
 *        This could have been done inline, but this way everbody uses
 *        the same code.
 *
 * @param arg_node: The N_fundef of a LACFUN.
 * @param callerfundef: The N_fundef of the caller function of the LACFUN
 * @param nassign: The N_assign node of the caller's N_ap of this LACFUN
 *
 * @return none
 *
 *
 ******************************************************************************/
void
PHUTsetClearCallAp (node *arg_node, node *callerfundef, node *nassign)
{

    DBUG_ENTER ();

    FUNDEF_CALLAP (arg_node) = nassign;
    FUNDEF_CALLERFUNDEF (arg_node) = callerfundef;

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
