/**
 * $Id$
 *
 * @defgroup uprf unroll primitive operations
 * @ingroup opt
 *
 * <pre>
 *
 *  If given a vector primitive, such as:
 *
 *   x = [1,2,3];
 *   y = [4,5,6];
 *   z = _add_VxV_( x, y);
 *
 *   replace it with:
 *
 *   x = [1,2,3];
 *   y = [4,5,6];
 *
 *   x0 = _sel_VxA_( [0], x);
 *   y0 = _sel_VxA_( [0], y);
 *   z0 = _add_SxS_( x0, y0);
 *
 *   x1 = _sel_VxA_( [1], x);
 *   y1 = _sel_VxA_( [1], y);
 *   z1 = _add_SxS_( x1, y1);
 *
 *   x2 = _sel_VxA_( [2]. x);
 *   y2 = _sel_VxA_( [2], y);
 *   z2 = _add_SxS_( x2, y2);
 *
 *   z = [ z0, z1, z2];
 *
 * Non-scalar primitives need a bit more custom work.
 * For example:
 *
 *  z, p = _non_neg_val_V_( x);
 *
 *  becomes:
 *
 *   pstart = TRUE;
 *   x0 = _sel_VxA_( [0], x);
 *   z0, p0 = _non_neg_val_S_( x0);
 *   p0' = pstart && p0;
 *
 *   x1 = _sel_VxA_( [1], x);
 *   z1, p1 = _non_neg_val_S_( x1);
 *   p1' = p0' && p1;
 *
 *   x2 = _sel_VxA_( [2]. x);
 *   z2, p2 = _non_neg_val_S_( x1);
 *   p2' = p1' && p2;
 *
 *   z = [ z0, z1, z2];
 *   p = p2';
 *
 *
 * </pre>
 * @{
 */

/**
 *
 * @file prfunroll.c
 *
 *
 */
#include "prfunroll.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "DupTree.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "new_typecheck.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"

/*
 * INFO structure
 */
struct INFO {
    bool onefundef;
    node *vardec;
    node *lhs;
    node *preassign;
    node *lastp1pavis;
};

/*
 * INFO macros
 */
#define INFO_ONEFUNDEF(n) ((n)->onefundef)
#define INFO_VARDEC(n) ((n)->vardec)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_PREASSIGN(n) ((n)->preassign)
#define INFO_LASTP1PAVIS(n) ((n)->lastp1pavis)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_ONEFUNDEF (result) = TRUE;
    INFO_VARDEC (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_PREASSIGN (result) = NULL;
    INFO_LASTP1PAVIS (result) = NULL;

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
 * @fn node *UPRFdoUnrollPRFs( node *fundef)
 *
 * @brief starting point of prf unrolling
 *
 * @param fundef
 *
 * @return fundef
 *
 *****************************************************************************/
node *
UPRFdoUnrollPRFs (node *fundef)
{
    info *info;

    DBUG_ENTER ("UPRFdoUnrollPRFs");

    info = MakeInfo ();

    TRAVpush (TR_uprf);
    fundef = TRAVdo (fundef, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (fundef);
}

/** <!--********************************************************************-->
 *
 * @fn node *UPRFdoUnrollPRFsModule( node *syntax_tree)
 *
 * @brief starting point of prf unrolling
 *
 * @param syntax_tree
 *
 * @return syntax_tree
 *
 *****************************************************************************/
node *
UPRFdoUnrollPRFsModule (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("UPRFdoUnrollPRFsModule");

    info = MakeInfo ();

    INFO_ONEFUNDEF (info) = FALSE;

    TRAVpush (TR_uprf);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/
static bool
PRFUnrollOracle (prf p)
{
    bool res;

    DBUG_ENTER ("PRFUnrollOracle");

    switch (p) {
    case F_add_VxS:
    case F_add_SxV:
    case F_add_VxV:

    case F_sub_VxS:
    case F_sub_SxV:
    case F_sub_VxV:

    case F_mul_VxS:
    case F_mul_SxV:
    case F_mul_VxV:

    case F_div_VxS:
    case F_div_SxV:
    case F_div_VxV:

    case F_non_neg_val_V:

        res = TRUE;
        break;

    default:
        res = FALSE;
        break;
    }

    DBUG_RETURN (res);
}

static prf
NormalizePrf (prf p)
{
    DBUG_ENTER ("NormalizePrf");

    switch (p) {
    case F_add_VxS:
    case F_add_SxV:
    case F_add_VxV:
        p = F_add_SxS;
        break;

    case F_sub_VxS:
    case F_sub_SxV:
    case F_sub_VxV:
        p = F_sub_SxS;
        break;

    case F_mul_VxS:
    case F_mul_SxV:
    case F_mul_VxV:
        p = F_mul_SxS;
        break;

    case F_div_VxS:
    case F_div_SxV:
    case F_div_VxV:
        p = F_div_SxS;
        break;

    case F_non_neg_val_V:
        p = F_non_neg_val_S;
        break;

    default:
        DBUG_ASSERT ((FALSE), "Illegal prf!");
        break;
    }

    DBUG_RETURN (p);
}

static bool
FirstArgScalar (prf p)
{
    bool res;

    DBUG_ENTER ("FirstArgScalar");

    switch (p) {
    case F_add_SxV:
    case F_sub_SxV:
    case F_mul_SxV:
    case F_div_SxV:
        res = TRUE;
        break;

    default:
        res = FALSE;
        break;
    }

    DBUG_RETURN (res);
}

static bool
SecondArgScalar (prf p)
{
    bool res;

    DBUG_ENTER ("SecondArgScalar");

    switch (p) {
    case F_add_VxS:
    case F_sub_VxS:
    case F_mul_VxS:
    case F_div_VxS:
        res = TRUE;
        break;

    default:
        res = FALSE;
        break;
    }

    DBUG_RETURN (res);
}

static node *
RevertExprs (node *exprs, node *agg)
{
    node *res;

    DBUG_ENTER ("RevertExprs");

    if (exprs == NULL) {
        res = agg;
    } else {
        res = EXPRS_NEXT (exprs);
        EXPRS_NEXT (exprs) = agg;
        res = RevertExprs (res, exprs);
    }

    DBUG_RETURN (res);
}

/* I think the author meant "reverse", not "revert */
static node *
RevertAssignments (node *ass, node *agg)
{
    node *res;

    DBUG_ENTER ("RevertAssignments");

    if (ass == NULL) {
        res = agg;
    } else {
        res = ASSIGN_NEXT (ass);
        ASSIGN_NEXT (ass) = agg;
        res = RevertAssignments (res, ass);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *MakeIntVec(...)
 *
 * @brief Create one-element integer vector, i.
 *        Return N_avis pointer for same.
 *
 *****************************************************************************/
static node *
MakeIntVec (int i, info *arg_info)
{
    node *selarravis;

    DBUG_ENTER ("MakeIntVec");
    selarravis = TBmakeAvis (TRAVtmpVar (),
                             TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, 1)));
    INFO_VARDEC (arg_info) = TBmakeVardec (selarravis, INFO_VARDEC (arg_info));
    INFO_PREASSIGN (arg_info)
      = TBmakeAssign (TBmakeLet (TBmakeIds (selarravis, NULL),
                                 TCmakeIntVector (TBmakeExprs (TBmakeNum (i), NULL))),
                      INFO_PREASSIGN (arg_info));
    AVIS_SSAASSIGN (selarravis) = INFO_PREASSIGN (arg_info);

    DBUG_RETURN (selarravis);
}

/** <!--********************************************************************-->
 *
 * @fn node *MakeSelOp(...)
 *
 * @brief Create  _sel_VxA_( [i], arg)
 *
 *        Return N_avis pointer for same.
 *
 *****************************************************************************/
static node *
MakeSelOp (ntype *scl, node *selarravis, node *argavis, info *arg_info)
{
    node *avis;

    DBUG_ENTER ("MakeSelOp");

    avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (scl));
    INFO_VARDEC (arg_info) = TBmakeVardec (avis, INFO_VARDEC (arg_info));
    INFO_PREASSIGN (arg_info)
      = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL),
                                 TCmakePrf2 (F_sel_VxA, TBmakeId (selarravis),
                                             TBmakeId (argavis))),
                      INFO_PREASSIGN (arg_info));
    AVIS_SSAASSIGN (avis) = INFO_PREASSIGN (arg_info);

    DBUG_RETURN (avis);
}

/** <!--********************************************************************-->
 *
 * @fn node *MakeIdsAndPredAvis(...)
 *
 * @brief Create LHS N_ids nodes for scalar element results.
 *  Most are simple, but the guards are a bit fussier.
 *
 *  For guards, we produce two results: clone of PRF_ARG1, and predicate
 *  Boolean.
 *
 *  We also create an avis and vardec for the  boolean scalar predicate.
 *
 * @result N_ids chain
 *
 *****************************************************************************/
static node *
MakeIdsAndPredAvis (node *resavis, node *arg_node, info *arg_info)
{
    node *res;
    node *predavis;

    DBUG_ENTER ("MakeIdsAndPredAvis");
    switch (PRF_PRF (arg_node)) {
    case F_non_neg_val_V:
        predavis = TBmakeAvis (TRAVtmpVar (),
                               TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0)));
        INFO_VARDEC (arg_info) = TBmakeVardec (predavis, INFO_VARDEC (arg_info));
        res = TBmakeIds (resavis, TBmakeIds (predavis, NULL));
        break;

    default:
        res = TBmakeIds (resavis, NULL);
        break;
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *MakeTrueScalar(...)
 *
 * @brief Create scalar TRUE for guards.
 *        Append assign to assign chain ass.
 *        This is the initial value for the and() fold operation on
 *        the guard's scalar predicate elements.
 *
 *
 * @result  N_assign for created node, or NULL
 *
 *****************************************************************************/
static node *
MakeTrueScalar (node *arg_node, info *arg_info)
{
    node *ass = NULL;
    node *bavis = NULL;

    DBUG_ENTER ("MakeTrueScalar");

    switch (PRF_PRF (arg_node)) {

    case F_non_neg_val_V:

        bavis = TBmakeAvis (TRAVtmpVar (),
                            TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0)));
        INFO_VARDEC (arg_info) = TBmakeVardec (bavis, INFO_VARDEC (arg_info));

        ass = TBmakeAssign (TBmakeLet (TBmakeIds (bavis, NULL), TBmakeBool (TRUE)), NULL);
        AVIS_SSAASSIGN (bavis) = ass;
        INFO_LASTP1PAVIS (arg_info) = bavis;
        break;

    default:
        break;
    }

    DBUG_RETURN (ass);
}

/** <!--********************************************************************-->
 *
 * @fn node *MakeResultNode(...)
 *
 * @brief Construct result node from generated N_array and
 *        perhaps assign guard predicate.
 *
 * @params arg_node: N_prf for the vector operation.
 *         arg_info: Your basic arg_info node.
 *         elems: N_array node just generated from scalarized vector
 *         ids:   N_ids node for LHS
 *
 * @result N_exprs chain for result
 *
 *****************************************************************************/
static node *
MakeResultNode (node *arg_node, info *arg_info, node *elems, node *ids)
{
    node *res;

    DBUG_ENTER ("MakeResultNode");

    /* Make N_array result */
    res = TCmakeVector (TYmakeAKS (TYcopyType (
                                     TYgetScalar (IDS_NTYPE (INFO_LHS (arg_info)))),
                                   SHmakeShape (0)),
                        RevertExprs (elems, NULL));

    /* Conditionally create guard result */
    switch (PRF_PRF (arg_node)) {
    case F_non_neg_val_V:
        INFO_PREASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (IDS_AVIS (IDS_NEXT (INFO_LHS (arg_info))),
                                                NULL),
                                     TBmakeId (INFO_LASTP1PAVIS (arg_info))),
                          INFO_PREASSIGN (arg_info));
        AVIS_SSAASSIGN (IDS_AVIS (IDS_NEXT (INFO_LHS (arg_info))))
          = INFO_PREASSIGN (arg_info);

        IDS_NEXT (INFO_LHS (arg_info))
          = (NULL != IDS_NEXT (INFO_LHS (arg_info)))
              ? FREEdoFreeTree (IDS_NEXT (INFO_LHS (arg_info)))
              : NULL;
        break;
    default:
        break;
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *MakeFoldOp(...)
 *
 * @brief  Generate partial and-reduce for guard predicates
 *
 * @params ids: N_ids node for LHS of scalarized guard function call
 *         arg_node: N_prf for the vector operation.
 *         arg_info: Your basic arg_info node.
 *         ids:   N_ids node for LHS
 *
 * @result: the N_assign chain
 *
 *****************************************************************************/
static void
MakeFoldOp (node *ids, node *arg_node, info *arg_info)
{
    node *p1pavis;
    node *x;
    node *y;

    DBUG_ENTER ("MakeFoldOp");

    switch (PRF_PRF (arg_node)) {
    case F_non_neg_val_V:
        /*  make  p1' = p0' && p1;              */
        p1pavis = TBmakeAvis (TRAVtmpVar (),
                              TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0)));
        INFO_VARDEC (arg_info) = TBmakeVardec (p1pavis, INFO_VARDEC (arg_info));
        x = INFO_LASTP1PAVIS (arg_info);
        INFO_LASTP1PAVIS (arg_info) = p1pavis;
        y = IDS_AVIS (IDS_NEXT (ids));
        INFO_PREASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (p1pavis, NULL),
                                     TCmakePrf2 (F_and_SxS, TBmakeId (x), TBmakeId (y))),
                          INFO_PREASSIGN (arg_info));
        break;
    default:
        break;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * PRF unrolling traversal (uprf_tab)
 *
 * prefix: UPRF
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *UPRFfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
UPRFfundef (node *arg_node, info *arg_info)
{
    bool old_onefundef;
    DBUG_ENTER ("UPRFfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_VARDEC (arg_info) = FUNDEF_VARDEC (arg_node);
        DBUG_PRINT ("UPRF", ("traversing body of (%s) %s",
                             (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                             FUNDEF_NAME (arg_node)));
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        FUNDEF_VARDEC (arg_node) = INFO_VARDEC (arg_info);
        DBUG_PRINT ("UPRF", ("leaving body of (%s) %s",
                             (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                             FUNDEF_NAME (arg_node)));
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
 * @fn node *UPRFassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
UPRFassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("UPRFassign");

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    if (INFO_PREASSIGN (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGN (arg_info), arg_node);
        INFO_PREASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *UPRFlet( node *arg_node, info *arg_info)
 *
 * If this primitive returns multiple results and was
 * unrolled, UPRFprf has eliminated all but the first of them.
 *
 *****************************************************************************/
node *
UPRFlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("UPRFlet");

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    INFO_LHS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *UPRFprf( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
UPRFprf (node *arg_node, info *arg_info)
{
    node *selarravis;
    node *argavis1, *argavis2, *resavis;
    node *ids;
    node *res;
    bool monadic;

    DBUG_ENTER ("UPRFprf");

    if ((PRFUnrollOracle (PRF_PRF (arg_node)))
        && (TYisAKS (IDS_NTYPE (INFO_LHS (arg_info))))
        && (TYgetDim (IDS_NTYPE (INFO_LHS (arg_info))) == 1)) {
        int len;
        ntype *nt1, *nt2;

        len = SHgetUnrLen (TYgetShape (IDS_NTYPE (INFO_LHS (arg_info))));
        nt1 = NTCnewTypeCheck_Expr (PRF_ARG1 (arg_node));
        monadic = (NULL == PRF_EXPRS2 (arg_node));
        nt2 = monadic ? NULL : NTCnewTypeCheck_Expr (PRF_ARG2 (arg_node));

        if ((TUshapeKnown (nt1)) && (monadic || (TUshapeKnown (nt2)))
            && (len < global.wlunrnum)) {
            ntype *scl;
            node *avis1, *avis2;
            node *elems = NULL;
            int i;

            avis1 = ID_AVIS (PRF_ARG1 (arg_node));
            avis2 = monadic ? NULL : ID_AVIS (PRF_ARG2 (arg_node));

            scl = TYmakeAKS (TYcopyType (TYgetScalar (nt1)), SHmakeShape (0));

            INFO_PREASSIGN (arg_info) = MakeTrueScalar (arg_node, arg_info);

            for (i = 0; i < len; i++) {
                if (FirstArgScalar (PRF_PRF (arg_node))) {
                    argavis1 = avis1;
                } else {
                    selarravis = MakeIntVec (i, arg_info);
                    argavis1 = MakeSelOp (scl, selarravis, avis1, arg_info);
                }

                if (monadic || SecondArgScalar (PRF_PRF (arg_node))) {
                    argavis2 = avis2;
                } else {
                    selarravis = MakeIntVec (i, arg_info);
                    argavis2 = MakeSelOp (scl, selarravis, avis2, arg_info);
                }

                resavis = TBmakeAvis (TRAVtmpVar (), TYcopyType (scl));
                INFO_VARDEC (arg_info) = TBmakeVardec (resavis, INFO_VARDEC (arg_info));
                ids = MakeIdsAndPredAvis (resavis, arg_node, arg_info);

                if (monadic) {
                    INFO_PREASSIGN (arg_info)
                      = TBmakeAssign (TBmakeLet (ids, TCmakePrf1 (NormalizePrf (
                                                                    PRF_PRF (arg_node)),
                                                                  TBmakeId (argavis1))),
                                      INFO_PREASSIGN (arg_info));
                } else {
                    INFO_PREASSIGN (arg_info)
                      = TBmakeAssign (TBmakeLet (ids, TCmakePrf2 (NormalizePrf (
                                                                    PRF_PRF (arg_node)),
                                                                  TBmakeId (argavis1),
                                                                  TBmakeId (argavis2))),
                                      INFO_PREASSIGN (arg_info));
                }
                AVIS_SSAASSIGN (resavis) = INFO_PREASSIGN (arg_info);

                MakeFoldOp (ids, arg_node, arg_info);
                elems = TBmakeExprs (TBmakeId (resavis), elems);
            }

            scl = TYfreeType (scl);

            global.optcounters.prfunr_prf++;
            res = MakeResultNode (arg_node, arg_info, elems, ids);
            arg_node = FREEdoFreeNode (arg_node);
            arg_node = res;

            INFO_PREASSIGN (arg_info)
              = RevertAssignments (INFO_PREASSIGN (arg_info), NULL);
            DBUG_PRINT ("UPRF", ("prf unrolled for %s", INFO_LHS (arg_info)));
        }

        nt1 = TYfreeType (nt1);
        nt2 = (NULL != nt2) ? TYfreeType (nt2) : nt2;
    }

    DBUG_RETURN (arg_node);
}

/* @} */
