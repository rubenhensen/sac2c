/**
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
 * And,
 *
 *   z, p = _val_lt_shape_VxA( x, y);
 *
 *  becomes:
 *
 *   pstart = TRUE;
 *   shp = _shape_A_( y);
 *   x0 = _sel_VxA_( [0], x);
 *   y0 = _sel_VxA_( [0], shp);
 *   z0, p0 = _val_lt_val_SxS( x0, y0);
 *   p0' = pstart && p0;
 *
 *   x1 = _sel_VxA_( [1], x);
 *   y1 = _sel_VxA_( [1], shp);
 *   z1, p1 = _val_lt_val_SxS( x1, y1);
 *   p1' = p0' && p1;
 *
 *   x2 = _sel_VxA_( [2]. x);
 *   y2 = _sel_VxA_( [2], shp);
 *   z2, p0 = _val_lt_val_SxS( x2, y2);
 *   p2' = p1' && p2;
 *
 *   z = [ z0, z1, z2];
 *   p = p2';
 *
 *  Note on code structure: It might be more readable if
 *  we a single switch() in UPRFprf, and effectively break
 *  the code into two pieces: (a) single-result primitives,
 *  and (b) multiple-result primitives (guards).
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

#define DBUG_PREFIX "UPRF"
#include "debug.h"

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
    node *vardec;
    node *lhs;
    node *preassign;
    node *lastp1pavis;
    node *fundef;
    node *shpavis;
    int len;
    bool onefundef;
};

/*
 * INFO macros
 */
#define INFO_VARDEC(n) ((n)->vardec)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_PREASSIGN(n) ((n)->preassign)
#define INFO_LASTP1PAVIS(n) ((n)->lastp1pavis)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_SHPAVIS(n) ((n)->shpavis)
#define INFO_LEN(n) ((n)->len)
#define INFO_ONEFUNDEF(n) ((n)->onefundef)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_VARDEC (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_PREASSIGN (result) = NULL;
    INFO_LASTP1PAVIS (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_SHPAVIS (result) = NULL;
    INFO_LEN (result) = 0;
    INFO_ONEFUNDEF (result) = TRUE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/* msd 18/06/2012 : could not find where UPRFdoUnrollPRFsPrf is used,
 *                  commenting out for now */

/** <!--********************************************************************-->
 *
 * @fn node *UPRFdoUnrollPRFsPrf( node *arg_node, node *vardecs, node *preassigns, node
 **lhs))
 *
 * @brief starting point of prf unrolling for a single N_prf
 *
 * @param arg_node: An N_prf node
 *        vardecs: A pointer to a pointer to an N_vardec chain
 *        preassigns: A pointer to a pointer to an N_assign chain
 *        lhs: A pointer to the LHS for the N_prf.
 *
 * @return updated arg_node, items added to vardecs and preassigns
 *
 *****************************************************************************/
// node *UPRFdoUnrollPRFsPrf( node *arg_node, node **vardecs,
//                           node **preassigns, node *lhs)
//{
//  info *arg_info;
//
//  DBUG_ENTER ();
//
//  DBUG_ASSERT (N_prf == NODE_TYPE( arg_node), "Expected N_prf");
//
//  arg_info = MakeInfo();
//  INFO_LHS( arg_info) = lhs;
//
//  TRAVpush( TR_uprf);
//  arg_node = TRAVdo( arg_node, arg_info);
//  TRAVpop();
//
//  *vardecs = TCappendVardec( INFO_VARDEC( arg_info), *vardecs);
//  *preassigns = TCappendAssign( *preassigns, INFO_PREASSIGN( arg_info));
//
//  arg_info = FreeInfo( arg_info);
//
//  DBUG_RETURN (arg_node);
//
//}

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

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_uprf);
    fundef = TRAVdo (fundef, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/
static bool
PRFUnrollOracle (node *arg_node)
{
    bool res;

    DBUG_ENTER ();

    switch (PRF_PRF (arg_node)) {

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
    case F_neg_V:
    case F_abs_V:

    case F_lt_VxS:
    case F_lt_SxV:
    case F_lt_VxV:

    case F_le_VxS:
    case F_le_SxV:
    case F_le_VxV:

    case F_eq_VxS:
    case F_eq_SxV:
    case F_eq_VxV:

    case F_ge_VxS:
    case F_ge_SxV:
    case F_ge_VxV:

    case F_gt_VxS:
    case F_gt_SxV:
    case F_gt_VxV:

    case F_neq_VxS:
    case F_neq_SxV:
    case F_neq_VxV:

    case F_mod_VxS:
    case F_mod_SxV:
    case F_mod_VxV:

    case F_min_VxS:
    case F_min_SxV:
    case F_min_VxV:

    case F_max_VxS:
    case F_max_SxV:
    case F_max_VxV:

        res = TRUE;
        break;

    case F_val_lt_shape_VxA:
        /* Unroll onlyif we know array dim */
        res = TYisAKD (AVIS_TYPE (ID_AVIS (PRF_ARG2 (arg_node))))
              || TYisAKS (AVIS_TYPE (ID_AVIS (PRF_ARG2 (arg_node))));
        break;

    case F_val_le_val_VxV:
        /* Unroll only if we know both array shapes */
        res
          = (TYisAKS (AVIS_TYPE (ID_AVIS (PRF_ARG1 (arg_node))))
             || TYisAKV (AVIS_TYPE (ID_AVIS (PRF_ARG1 (arg_node)))))
            && (TYisAKS (AVIS_TYPE (ID_AVIS (PRF_ARG2 (arg_node))))
                || TYisAKV (AVIS_TYPE (ID_AVIS (PRF_ARG2 (arg_node)))))
            && (SHgetUnrLen (TYgetShape (AVIS_TYPE (ID_AVIS (PRF_ARG1 (arg_node)))))
                == SHgetUnrLen (TYgetShape (AVIS_TYPE (ID_AVIS (PRF_ARG2 (arg_node))))));
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
    DBUG_ENTER ();

    switch (p) {

    case F_lt_SxV:
    case F_lt_VxS:
    case F_lt_VxV:
        p = F_lt_SxS;
        break;

    case F_le_SxV:
    case F_le_VxS:
    case F_le_VxV:
        p = F_le_SxS;
        break;

    case F_eq_SxV:
    case F_eq_VxS:
    case F_eq_VxV:
        p = F_eq_SxS;
        break;

    case F_ge_SxV:
    case F_ge_VxS:
    case F_ge_VxV:
        p = F_ge_SxS;
        break;

    case F_gt_SxV:
    case F_gt_VxS:
    case F_gt_VxV:
        p = F_gt_SxS;
        break;

    case F_neq_SxV:
    case F_neq_VxS:
    case F_neq_VxV:
        p = F_neq_SxS;
        break;

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

    case F_val_lt_shape_VxA:
        p = F_val_lt_val_SxS;
        break;

    case F_neg_V:
        p = F_neg_S;
        break;

    case F_val_le_val_VxV:
        p = F_val_le_val_SxS;
        break;

    case F_abs_V:
        p = F_abs_S;
        break;

    case F_mod_VxS:
    case F_mod_SxV:
    case F_mod_VxV:
        p = F_mod_SxS;
        break;

    case F_min_VxS:
    case F_min_SxV:
    case F_min_VxV:
        p = F_min_SxS;
        break;

    case F_max_VxS:
    case F_max_SxV:
    case F_max_VxV:
        p = F_max_SxS;
        break;

    default:
        DBUG_ASSERT (FALSE, "Illegal prf!");
        break;
    }

    DBUG_RETURN (p);
}

static node *
ReverseExprs (node *exprs, node *agg)
{
    node *res;

    DBUG_ENTER ();

    if (exprs == NULL) {
        res = agg;
    } else {
        res = EXPRS_NEXT (exprs);
        EXPRS_NEXT (exprs) = agg;
        res = ReverseExprs (res, exprs);
    }

    DBUG_RETURN (res);
}

static node *
ReverseAssignments (node *ass, node *agg)
{
    node *res;

    DBUG_ENTER ();

    if (ass == NULL) {
        res = agg;
    } else {
        res = ASSIGN_NEXT (ass);
        ASSIGN_NEXT (ass) = agg;
        res = ReverseAssignments (res, ass);
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

    DBUG_ENTER ();
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
 * @fn node *MakeSelOpArg1(...)
 *
 * @brief  Create _sel_VxA_( [i], avis)
 *
 *****************************************************************************/
static node *
MakeSelOpArg1 (node *arg_node, info *arg_info, int i, node *avis)
{
    node *selarravis;
    node *zavis = NULL;
    prf nprf;

    DBUG_ENTER ();

    nprf = PRF_PRF (arg_node);
    switch (PRF_PRF (arg_node)) {

    case F_add_SxV:
    case F_sub_SxV:
    case F_mul_SxV:
    case F_div_SxV:
    case F_mod_SxV:
    case F_min_SxV:
    case F_max_SxV:
        // break elided intentionally

    default:
        zavis = avis;
        break;

    case F_add_VxS:
    case F_add_VxV:

    case F_sub_VxS:
    case F_sub_VxV:

    case F_mul_VxS:
    case F_mul_VxV:

    case F_div_VxS:
    case F_div_VxV:

    case F_non_neg_val_V:
    case F_neg_V:
    case F_val_le_val_VxV:
    case F_abs_V:

    case F_lt_VxS:
    case F_le_VxS:
    case F_eq_VxS:
    case F_ge_VxS:
    case F_gt_VxS:
    case F_neq_VxS:

    case F_lt_VxV:
    case F_le_VxV:
    case F_eq_VxV:
    case F_ge_VxV:
    case F_gt_VxV:
    case F_neq_VxV:

    case F_mod_VxS:
    case F_mod_VxV:

    case F_min_VxS:
    case F_min_VxV:

    case F_max_VxS:
    case F_max_VxV:

        nprf = F_sel_VxA;
        break;

    case F_val_lt_shape_VxA:
        nprf = F_sel_VxA;
        break;
    }

    if (NULL == zavis) {
        selarravis = MakeIntVec (i, arg_info);
        zavis = TBmakeAvis (TRAVtmpVar (),
                            TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
        INFO_VARDEC (arg_info) = TBmakeVardec (zavis, INFO_VARDEC (arg_info));
        INFO_PREASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (zavis, NULL),
                                     TCmakePrf2 (nprf, TBmakeId (selarravis),
                                                 TBmakeId (avis))),
                          INFO_PREASSIGN (arg_info));
        AVIS_SSAASSIGN (zavis) = INFO_PREASSIGN (arg_info);
    }

    DBUG_RETURN (zavis);
}

/** <!--********************************************************************-->
 *
 * @fn node *MakeSelOpArg2(...)
 *
 * @brief Create _sel_VxA_( [i], PRF_ARG2);
 *        or, nothing for monadic ops
 *
 *        Return N_avis pointer for same.
 *
 *****************************************************************************/
static node *
MakeSelOpArg2 (node *arg_node, info *arg_info, int i, node *avis)
{
    node *zavis = NULL;
    node *selarravis;
    bool dyadic;
    prf nprf;

    DBUG_ENTER ();

    nprf = PRF_PRF (arg_node);
    switch (PRF_PRF (arg_node)) {

    case F_neg_V:
    case F_abs_V:
    case F_non_neg_val_V:
        dyadic = FALSE;
        break;

    case F_add_VxS:
    case F_sub_VxS:
    case F_mul_VxS:
    case F_div_VxS:

    case F_lt_VxS:
    case F_le_VxS:
    case F_eq_VxS:
    case F_ge_VxS:
    case F_gt_VxS:
    case F_neq_VxS:

    case F_mod_VxS:

    case F_min_VxS:
    case F_max_VxS:

        zavis = avis;
        dyadic = FALSE;
        nprf = F_sel_VxA;
        break;

    case F_val_lt_shape_VxA:
        nprf = F_sel_VxA;
        dyadic = TRUE;
        INFO_LEN (arg_info) = TYgetDim (AVIS_TYPE (ID_AVIS (PRF_ARG2 (arg_node))));
        avis = INFO_SHPAVIS (arg_info);
        break;

    default:
        dyadic = TRUE;
        nprf = F_sel_VxA;
        break;
    }

    if (dyadic) {
        selarravis = MakeIntVec (i, arg_info);
        zavis = TBmakeAvis (TRAVtmpVar (),
                            TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
        INFO_VARDEC (arg_info) = TBmakeVardec (zavis, INFO_VARDEC (arg_info));
        INFO_PREASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (zavis, NULL),
                                     TCmakePrf2 (nprf, TBmakeId (selarravis),
                                                 TBmakeId (avis))),
                          INFO_PREASSIGN (arg_info));
        AVIS_SSAASSIGN (zavis) = INFO_PREASSIGN (arg_info);
    }

    DBUG_RETURN (zavis);
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
 *  We also create an avis and vardec for the boolean scalar predicate.
 *
 * @result N_ids chain
 *
 *****************************************************************************/
static node *
MakeIdsAndPredAvis (node *resavis, node *arg_node, info *arg_info)
{
    node *res;
    node *predavis;

    DBUG_ENTER ();
    switch (PRF_PRF (arg_node)) {
    case F_non_neg_val_V:
    case F_val_lt_shape_VxA:
    case F_val_le_val_VxV:
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
 * @fn node *MakeUnrolledOp(...)
 *
 * @brief Emit the unrolled, scalar version of the PRF.
 *
 * @result
 *
 *****************************************************************************/
static node *
MakeUnrolledOp (node *arg_node, info *arg_info, node *ids, node *argavis1, node *argavis2,
                node *resavis)
{

    DBUG_ENTER ();

    switch (PRF_PRF (arg_node)) {

    /* Dyadic invocations */
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

    case F_lt_VxS:
    case F_lt_SxV:
    case F_lt_VxV:
    case F_le_VxS:
    case F_le_SxV:
    case F_le_VxV:
    case F_eq_VxS:
    case F_eq_SxV:
    case F_eq_VxV:
    case F_ge_VxS:
    case F_ge_SxV:
    case F_ge_VxV:
    case F_gt_VxS:
    case F_gt_SxV:
    case F_gt_VxV:
    case F_neq_VxS:
    case F_neq_SxV:
    case F_neq_VxV:

    case F_mod_VxS:
    case F_mod_SxV:
    case F_mod_VxV:

    case F_min_VxS:
    case F_min_SxV:
    case F_min_VxV:

    case F_max_VxS:
    case F_max_SxV:
    case F_max_VxV:

    case F_val_lt_shape_VxA:
    case F_val_le_val_VxV:

        INFO_PREASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (ids, TCmakePrf2 (NormalizePrf (PRF_PRF (arg_node)),
                                                      TBmakeId (argavis1),
                                                      TBmakeId (argavis2))),
                          INFO_PREASSIGN (arg_info));
        AVIS_SSAASSIGN (resavis) = INFO_PREASSIGN (arg_info);
        if (NULL != IDS_NEXT (ids)) { /* guards only */
            AVIS_SSAASSIGN (IDS_AVIS (IDS_NEXT (ids))) = INFO_PREASSIGN (arg_info);
        }
        break;

    case F_non_neg_val_V:
    case F_neg_V:
    case F_abs_V:
        INFO_PREASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (ids, TCmakePrf1 (NormalizePrf (PRF_PRF (arg_node)),
                                                      TBmakeId (argavis1))),
                          INFO_PREASSIGN (arg_info));
        AVIS_SSAASSIGN (resavis) = INFO_PREASSIGN (arg_info);
        if (NULL != IDS_NEXT (ids)) { /* guards only */
            AVIS_SSAASSIGN (IDS_AVIS (IDS_NEXT (ids))) = INFO_PREASSIGN (arg_info);
        }
        break;

    default:
        DBUG_ASSERT (FALSE, "Missed a case!");
        break;
    }

    DBUG_RETURN (resavis);
}

/** <!--********************************************************************-->
 *
 * @fn void MakeTrueScalar(...)
 *
 * @brief Create scalar TRUE for guards.
 *        Append assign to assign chain ass.
 *        This is the initial value for the and() fold operation on
 *        the guard's scalar predicate elements.
 *
 *
 * @result  none - it's all done with INFO_PREASSIGN
 *
 *****************************************************************************/
static void
MakeTrueScalar (node *arg_node, info *arg_info)
{
    node *assgn;
    node *shpavis;
    node *bavis;
    int dim;

    DBUG_ENTER ();

    switch (PRF_PRF (arg_node)) {

    case F_val_lt_shape_VxA:
        /* Create shp = _shape_A_( y); */
        dim = TYgetDim (AVIS_TYPE (ID_AVIS (PRF_ARG2 (arg_node))));
        shpavis = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                                        SHcreateShape (1, dim)));
        INFO_VARDEC (arg_info) = TBmakeVardec (shpavis, INFO_VARDEC (arg_info));
        assgn = TBmakeIds (shpavis, NULL);
        INFO_PREASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (assgn,
                                     TCmakePrf1 (F_shape_A,
                                                 DUPdoDupNode (PRF_ARG2 (arg_node)))),
                          INFO_PREASSIGN (arg_info));
        AVIS_SSAASSIGN (shpavis) = INFO_PREASSIGN (arg_info);
        INFO_SHPAVIS (arg_info) = shpavis;
        /* Break intentionally elided */

    case F_non_neg_val_V:
    case F_val_le_val_VxV:
        bavis = TBmakeAvis (TRAVtmpVar (),
                            TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0)));
        INFO_VARDEC (arg_info) = TBmakeVardec (bavis, INFO_VARDEC (arg_info));

        assgn
          = TBmakeAssign (TBmakeLet (TBmakeIds (bavis, NULL), TBmakeBool (TRUE)), NULL);
        INFO_PREASSIGN (arg_info) = TCappendAssign (INFO_PREASSIGN (arg_info), assgn);
        AVIS_SSAASSIGN (bavis) = assgn;
        INFO_LASTP1PAVIS (arg_info) = bavis;
        break;

    default:
        break;
    }

    DBUG_RETURN ();
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
    node *guardres;

    DBUG_ENTER ();

    /* Conditionally create guard result */
    switch (PRF_PRF (arg_node)) {
    case F_non_neg_val_V:
    case F_val_lt_shape_VxA:
    case F_val_le_val_VxV:
        guardres = IDS_NEXT (INFO_LHS (arg_info));
        IDS_NEXT (INFO_LHS (arg_info)) = NULL;
        INFO_PREASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (guardres, TBmakeId (INFO_LASTP1PAVIS (arg_info))),
                          INFO_PREASSIGN (arg_info));
        AVIS_SSAASSIGN (IDS_AVIS (guardres)) = INFO_PREASSIGN (arg_info);
        break;
    default:
        break;
    }

    /* Make N_array result */
    res = TCmakeVector (TYmakeAKS (TYcopyType (
                                     TYgetScalar (IDS_NTYPE (INFO_LHS (arg_info)))),
                                   SHmakeShape (0)),
                        ReverseExprs (elems, NULL));

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

    DBUG_ENTER ();

    switch (PRF_PRF (arg_node)) {
    case F_non_neg_val_V:
    case F_val_lt_shape_VxA:
    case F_val_le_val_VxV:
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
        AVIS_SSAASSIGN (p1pavis) = INFO_PREASSIGN (arg_info);
        break;
    default:
        break;
    }

    DBUG_RETURN ();
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
 * @fn node *UPRFmodule(node *arg_node, info *arg_info)
 *
 * @brief Traverses only functions of the module, skipping all the rest for
 *        performance reasons.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
UPRFmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *UPRFfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
UPRFfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_VARDEC (arg_info) = NULL;
        INFO_FUNDEF (arg_info) = arg_node; /* handy for debugging */
        DBUG_PRINT ("traversing body of (%s) %s",
                    (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                    FUNDEF_NAME (arg_node));
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        /* If new vardecs were made, append them to the current set */
        if (INFO_VARDEC (arg_info) != NULL) {
            FUNDEF_VARDECS (arg_node)
              = TCappendVardec (INFO_VARDEC (arg_info), FUNDEF_VARDECS (arg_node));
            INFO_VARDEC (arg_info) = NULL;
        }

        DBUG_PRINT ("leaving body of (%s) %s",
                    (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                    FUNDEF_NAME (arg_node));
    }

    INFO_FUNDEF (arg_info) = NULL;

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

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
    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
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
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    INFO_LHS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *UPRFprf( node *arg_node, info *arg_info)
 *
 * @brief Unroll primitive function, if its result is an AKS vector,
 *        and it belongs to a select set of functions.
 *
 *****************************************************************************/
node *
UPRFprf (node *arg_node, info *arg_info)
{
    node *argavis1, *argavis2, *resavis;
    node *ids;
    bool monadic;
    bool valltshp;
    int i;

    DBUG_ENTER ();

    if ((PRFUnrollOracle (arg_node)) && (TYisAKS (IDS_NTYPE (INFO_LHS (arg_info))))
        && (TYgetDim (IDS_NTYPE (INFO_LHS (arg_info))) == 1)) {
        ntype *nt1, *nt2;

        INFO_LEN (arg_info) = SHgetUnrLen (TYgetShape (IDS_NTYPE (INFO_LHS (arg_info))));
        nt1 = NTCnewTypeCheck_Expr (PRF_ARG1 (arg_node));
        monadic = (NULL == PRF_EXPRS2 (arg_node));
        valltshp = (F_val_lt_shape_VxA == PRF_PRF (arg_node));
        nt2 = monadic ? NULL : NTCnewTypeCheck_Expr (PRF_ARG2 (arg_node));

        if ((TUshapeKnown (nt1)) && (monadic || valltshp || (TUshapeKnown (nt2)))
            && (INFO_LEN (arg_info) < global.prfunrnum)) {
            node *avis1, *avis2;
            node *elems = NULL;

            avis1 = ID_AVIS (PRF_ARG1 (arg_node));
            avis2 = monadic ? NULL : ID_AVIS (PRF_ARG2 (arg_node));

            MakeTrueScalar (arg_node, arg_info);

            for (i = 0; i < INFO_LEN (arg_info); i++) {
                argavis1 = MakeSelOpArg1 (arg_node, arg_info, i, avis1);
                argavis2 = MakeSelOpArg2 (arg_node, arg_info, i, avis2);

                resavis = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                                                SHcreateShape (0)));
                INFO_VARDEC (arg_info) = TBmakeVardec (resavis, INFO_VARDEC (arg_info));
                ids = MakeIdsAndPredAvis (resavis, arg_node, arg_info);

                resavis
                  = MakeUnrolledOp (arg_node, arg_info, ids, argavis1, argavis2, resavis);
                MakeFoldOp (ids, arg_node, arg_info);
                elems = TBmakeExprs (TBmakeId (resavis), elems);
            }

            global.optcounters.prfunr_prf++;
            arg_node = MakeResultNode (arg_node, arg_info, elems, ids);

            INFO_PREASSIGN (arg_info)
              = ReverseAssignments (INFO_PREASSIGN (arg_info), NULL);
            DBUG_PRINT ("prf unrolled for %s",
                        AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
        }

        nt1 = TYfreeType (nt1);
        nt2 = (NULL != nt2) ? TYfreeType (nt2) : nt2;
    }

    DBUG_RETURN (arg_node);
}

/* @} */

#undef DBUG_PREFIX
