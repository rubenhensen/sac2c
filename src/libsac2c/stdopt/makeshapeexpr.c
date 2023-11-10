/** <!--********************************************************************-->
 *
 * @defgroup mse Make Shape Expression
 *
 * Utility traversal to provide expressions for computing array shapes.
 *
 * @ingroup isaa
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file makeshapeexpr.c
 *
 * Prefix: MSE
 *
 *****************************************************************************/
#include "makeshapeexpr.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#define DBUG_PREFIX "MSE"
#include "debug.h"

#include "str.h"
#include "memory.h"
#include "DupTree.h"
#include "free.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"
#include "constants.h"
#include "compare_tree.h"
#include "phase.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *avis;
    node *allids;
    node *fundef;
    node *preass;
};

#define INFO_AVIS(n) ((n)->avis)
#define INFO_ALLIDS(n) ((n)->allids)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_PREASS(n) ((n)->preass)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_AVIS (result) = NULL;
    INFO_ALLIDS (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_PREASS (result) = NULL;

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
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/
static node *
GenIntVector (node *element)
{
    node *res = NULL;

    DBUG_ENTER ();

    res = TCmakeIntVector (TBmakeExprs (element, NULL));

    DBUG_RETURN (res);
}

static node *
MakeAssignForIdShape (node *id, node *fundef, node **preass)
{ // Return the N_avis we create
    node *res;
    node *newass;
    int dim;

    DBUG_ENTER ();

    DBUG_ASSERT (NULL != AVIS_DIM (ID_AVIS (id)),
                 "Making assign for Id without Dimension!");

    if (NODE_TYPE (AVIS_DIM (ID_AVIS (id))) == N_num) {
        dim = NUM_VAL (AVIS_DIM (ID_AVIS (id)));
        res = TBmakeAvis (TRAVtmpVarName (ID_NAME (id)),
                          TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, dim)));
    } else {
        res = TBmakeAvis (TRAVtmpVarName (ID_NAME (id)),
                          TYmakeAKD (TYmakeSimpleType (T_int), 1, SHmakeShape (0)));
    }

    AVIS_DIM (res) = TBmakeNum (1);
    AVIS_SHAPE (res) = GenIntVector (DUPdoDupNode (AVIS_DIM (ID_AVIS (id))));

    FUNDEF_VARDECS (fundef) = TBmakeVardec (res, FUNDEF_VARDECS (fundef));

    DBUG_ASSERT (NULL != AVIS_SHAPE (ID_AVIS (id)), "NULL AVIS_SHAPE for id");
    newass = TBmakeAssign (TBmakeLet (TBmakeIds (res, NULL),
                                      DUPdoDupNode (AVIS_SHAPE (ID_AVIS (id)))),
                           NULL);

    AVIS_SSAASSIGN (res) = newass;

    *preass = TCappendAssign (*preass, newass);

    DBUG_RETURN (res);
}

/*
 * function node * InjectTypeConv( int dim, node *avis)
 *
 * modify SSA_ASSIGN(avis)
 * from    mse_var = def_shp_id;
 * into    mse_var = F_type_conv( int[dim], def_shp_id);
 */
static node *
InjectTypeConv (int dim, node *avis)
{
    ntype *type;
    node *let;

    DBUG_ENTER ();
    let = ASSIGN_STMT (AVIS_SSAASSIGN (avis));
    type = TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, dim));
    LET_EXPR (let) = TCmakePrf2 (F_type_conv, TBmakeType (type), LET_EXPR (let));

    DBUG_RETURN (avis);
}

node *
MakeVectAvis (char *name, node *dim)
{
    node *res;

    DBUG_ENTER ();

    if (NODE_TYPE (dim) == N_num) {
        res = TBmakeAvis (name, TYmakeAKS (TYmakeSimpleType (T_int),
                                           SHcreateShape (1, NUM_VAL (dim))));
    } else {
        res = TBmakeAvis (name, TYmakeAKD (TYmakeSimpleType (T_int), 1, SHmakeShape (0)));
    }

    if (PHisSAAMode ()) {
        AVIS_DIM (res) = TBmakeNum (1);
        AVIS_SHAPE (res) = GenIntVector (DUPdoDupNode (dim));
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/*
 * The following functions are used in a function table initialised
 * via prf_info.mac.
 */

static node *
SAAshp_is_empty (node *arg_node, info *arg_info)
{
    node *shp_expr;

    DBUG_ENTER ();

    shp_expr = TCmakeIntVector (NULL);

    DBUG_RETURN (shp_expr);
}

static node *
SAAshp_is_arg1 (node *arg_node, info *arg_info)
{
    node *shp_expr;

    DBUG_ENTER ();

    shp_expr = DUPdoDupNode (PRF_ARG1 (arg_node));

    DBUG_RETURN (shp_expr);
}

static node *
SAAshp_is_arg2 (node *arg_node, info *arg_info)
{
    node *shp_expr;

    DBUG_ENTER ();

    shp_expr = DUPdoDupNode (PRF_ARG2 (arg_node));

    DBUG_RETURN (shp_expr);
}

static node *
SAAshp_of_arg1 (node *arg_node, info *arg_info)
{
    node *shp_expr;

    DBUG_ENTER ();

    shp_expr = DUPdoDupNode (AVIS_SHAPE (ID_AVIS (PRF_ARG1 (arg_node))));

    DBUG_RETURN (shp_expr);
}

static node *
SAAshp_of_arg2 (node *arg_node, info *arg_info)
{
    node *shp_expr;

    DBUG_ENTER ();

    shp_expr = DUPdoDupNode (AVIS_SHAPE (ID_AVIS (PRF_ARG2 (arg_node))));

    DBUG_RETURN (shp_expr);
}

static node *
SAAshp_of_arg3 (node *arg_node, info *arg_info)
{
    node *shp_expr;

    DBUG_ENTER ();

    shp_expr = DUPdoDupNode (AVIS_SHAPE (ID_AVIS (PRF_ARG3 (arg_node))));

    DBUG_RETURN (shp_expr);
}

static node *
SAAshp_for_shape (node *arg_node, info *arg_info)
{
    node *shp_expr = NULL;

    DBUG_ENTER ();

    if (AVIS_DIM (ID_AVIS (PRF_ARG1 (arg_node))) != NULL) {
        node *adim = AVIS_DIM (ID_AVIS (PRF_ARG1 (arg_node)));
        shp_expr = TCmakeIntVector (TBmakeExprs (DUPdoDupNode (adim), NULL));
    }

    DBUG_RETURN (shp_expr);
}

static node *
SAAshp_for_simd_sel (node *arg_node, info *arg_info)
{
    node *shp_expr = NULL;

    DBUG_ENTER ();

    if (AVIS_DIM (ID_AVIS (PRF_ARG1 (arg_node))) != NULL) {
        node *simd_length = AVIS_DIM (ID_AVIS (PRF_ARG1 (arg_node)));
        shp_expr = TCmakeIntVector (TBmakeExprs (DUPdoDupNode (simd_length), NULL));
    }

    DBUG_RETURN (shp_expr);
}

static node *
SAAshp_for_cat (node *arg_node, info *arg_info)
{
    node *shp_expr;
    node *v1savis, *v2savis;
    node *preass = NULL;

    DBUG_ENTER ();

    v1savis = MakeAssignForIdShape (PRF_ARG1 (arg_node), INFO_FUNDEF (arg_info), &preass);

    v2savis = MakeAssignForIdShape (PRF_ARG2 (arg_node), INFO_FUNDEF (arg_info), &preass);

    shp_expr = TCmakePrf2 (F_add_VxV, TBmakeId (v1savis), TBmakeId (v2savis));

    INFO_PREASS (arg_info) = preass;

    DBUG_RETURN (shp_expr);
}

static node *
SAAshp_for_take (node *arg_node, info *arg_info)
{
    node *shp_expr;
    node *scalar;
    node *idavis;
    node *absavis;
    node *preass = NULL;

    DBUG_ENTER ();

    if (NODE_TYPE (PRF_ARG1 (arg_node)) == N_num) {
        scalar = TBmakeNum (abs (NUM_VAL (PRF_ARG1 (arg_node))));
    } else {
        idavis = ID_AVIS (PRF_ARG1 (arg_node));

        absavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (idavis)),
                              TYeliminateAKV (AVIS_TYPE (idavis)));

        AVIS_DIM (absavis) = DUPdoDupNode (AVIS_DIM (idavis));
        AVIS_SHAPE (absavis) = DUPdoDupNode (AVIS_SHAPE (idavis));

        FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
          = TBmakeVardec (absavis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

        preass = TBmakeAssign (TBmakeLet (TBmakeIds (absavis, NULL),
                                          TCmakePrf1 (F_abs_S, TBmakeId (idavis))),
                               preass);

        AVIS_SSAASSIGN (absavis) = preass;

        scalar = TBmakeId (absavis);
    }

    shp_expr = TCmakeIntVector (TBmakeExprs (scalar, NULL));

    INFO_PREASS (arg_info) = preass;

    DBUG_RETURN (shp_expr);
}

static node *
SAAshp_for_drop (node *arg_node, info *arg_info)
{
    node *shp_expr;
    node *scalar;
    node *vsavis;
    node *idavis;
    node *absavis;
    node *preass = NULL;

    DBUG_ENTER ();

    vsavis = MakeAssignForIdShape (PRF_ARG2 (arg_node), INFO_FUNDEF (arg_info), &preass);

    if (NODE_TYPE (PRF_ARG1 (arg_node)) == N_num) {
        scalar = TBmakeNum (abs (NUM_VAL (PRF_ARG1 (arg_node))));
    } else {
        idavis = ID_AVIS (PRF_ARG1 (arg_node));

        absavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (idavis)),
                              TYeliminateAKV (AVIS_TYPE (idavis)));

        AVIS_DIM (absavis) = DUPdoDupNode (AVIS_DIM (idavis));
        AVIS_SHAPE (absavis) = DUPdoDupNode (AVIS_SHAPE (idavis));

        FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
          = TBmakeVardec (absavis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

        preass = TBmakeAssign (TBmakeLet (TBmakeIds (absavis, NULL),
                                          TCmakePrf1 (F_abs_S, TBmakeId (idavis))),
                               preass);

        AVIS_SSAASSIGN (absavis) = preass;

        scalar = TBmakeId (absavis);
    }

    shp_expr = TCmakePrf2 (F_sub_VxS, TBmakeId (vsavis), scalar);

    INFO_PREASS (arg_info) = preass;

    DBUG_RETURN (shp_expr);
}

static node *
SAAshp_guard (node *arg_node, info *arg_info)
{
    node *shp_expr, *lhsavis, *ids, *exprs;

    DBUG_ENTER ();

    lhsavis = INFO_AVIS (arg_info);
    ids = INFO_ALLIDS (arg_info);
    exprs = PRF_ARGS (arg_node);

    while (IDS_AVIS (ids) != lhsavis) {
        ids = IDS_NEXT (ids);
        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_ASSERT (EXPRS_NEXT (exprs) != NULL,
                 "guard predicate does not have a corresponding return value");

    // We are dealing with one of the guarded arguments
    shp_expr = DUPdoDupNode (AVIS_SHAPE (ID_AVIS (EXPRS_EXPR (exprs))));

    DBUG_RETURN (shp_expr);
}

static node *
SAAshp_type_constraint (node *arg_node, info *arg_info)
{
    node *shp_expr;
    node *lhsavis;
    node *ids;

    DBUG_ENTER ();

    lhsavis = INFO_AVIS (arg_info);

    ids = INFO_ALLIDS (arg_info);

    if (lhsavis == IDS_AVIS (ids)) {
        /* We are dealing with the first return value */
        if (TUshapeKnown (TYPE_TYPE (PRF_ARG1 (arg_node)))) {
            shp_expr = SHshape2Array (TYgetShape (TYPE_TYPE (PRF_ARG1 (arg_node))));
        } else {
            shp_expr = DUPdoDupNode (AVIS_SHAPE (ID_AVIS (PRF_ARG2 (arg_node))));
        }
    } else {
        /* We are dealing with the boolean result */
        shp_expr = TCmakeIntVector (NULL);
    }

    DBUG_RETURN (shp_expr);
}

static node *
SAAshp_same_shape_AxA (node *arg_node, info *arg_info)
{
    node *shp_expr;
    node *lhsavis;
    node *ids;

    DBUG_ENTER ();

    lhsavis = INFO_AVIS (arg_info);

    ids = INFO_ALLIDS (arg_info);

    if ((lhsavis == IDS_AVIS (ids)) || (lhsavis == IDS_AVIS (IDS_NEXT (ids)))) {
        /* We are dealing with the first two return values */
        shp_expr = DUPdoDupNode (AVIS_SHAPE (ID_AVIS (PRF_ARG1 (arg_node))));
    } else {
        /* We are dealing with the boolean result */
        shp_expr = TCmakeIntVector (NULL);
    }

    DBUG_RETURN (shp_expr);
}

static node *
SAAshp_shape_matches_dim_VxA (node *arg_node, info *arg_info)
{
    node *shp_expr;
    node *lhsavis;
    node *ids;

    DBUG_ENTER ();

    lhsavis = INFO_AVIS (arg_info);

    ids = INFO_ALLIDS (arg_info);

    if (lhsavis == IDS_AVIS (ids)) {
        /* We are dealing with the first return value */
        shp_expr = DUPdoDupNode (AVIS_DIM (ID_AVIS (PRF_ARG2 (arg_node))));
        shp_expr = TCmakeIntVector (TBmakeExprs (shp_expr, NULL));
    } else {
        /* We are dealing with the boolean result */
        shp_expr = TCmakeIntVector (NULL);
    }

    DBUG_RETURN (shp_expr);
}

static node *
SAAshp_cc_inherit (node *arg_node, info *arg_info)
{
    node *shp_expr;
    node *lhsavis;
    node *ids;

    DBUG_ENTER ();

    lhsavis = INFO_AVIS (arg_info);

    ids = INFO_ALLIDS (arg_info);

    if (lhsavis == IDS_AVIS (ids)) {
        /* We are dealing with the first return value */
        shp_expr = DUPdoDupNode (AVIS_SHAPE (ID_AVIS (PRF_ARG1 (arg_node))));
    } else {
        /* We are dealing with the boolean result */
        shp_expr = TCmakeIntVector (NULL);
    }

    DBUG_RETURN (shp_expr);
}

static const travfun_p makeshp_funtab[] = {
#define PRFmakeshp_fun(makeshp_fun) makeshp_fun
#include "prf_info.mac"
};

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *MSEdoMakeShapeExpression( node *expr, node *avis,
 *                                     node *allids, node *fundef)
 *
 *****************************************************************************/
node *
MSEdoMakeShapeExpression (node *expr, node *avis, node *allids, node *fundef)
{
    info *info;
    node *res;
    node *shpavis;

    DBUG_ENTER ();

    DBUG_ASSERT ((AVIS_DIM (avis) != NULL) && (AVIS_SHAPE (avis) == NULL),
                 "AVIS_DIM( avis) must not be NULL "
                 "whereas AVIS_SHAPE( avis) must be NULL");

    info = MakeInfo ();

    INFO_AVIS (info) = avis;
    INFO_ALLIDS (info) = allids;
    INFO_FUNDEF (info) = fundef;

    shpavis = MakeVectAvis (TRAVtmpVarName (AVIS_NAME (avis)), AVIS_DIM (avis));
    AVIS_SHAPE (avis) = TBmakeId (shpavis);

    TRAVpush (TR_mse);
    res = TRAVdo (expr, info);
    TRAVpop ();

    info = FreeInfo (info);

    if (res != NULL) {
        FUNDEF_VARDECS (fundef) = TBmakeVardec (shpavis, FUNDEF_VARDECS (fundef));
    } else {
        AVIS_SHAPE (avis) = FREEdoFreeNode (AVIS_SHAPE (avis));
        shpavis = FREEdoFreeNode (shpavis);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *MSEid( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
MSEid (node *arg_node, info *arg_info)
{
    node *lhsavis;
    node *shapeavis;
    node *rhsnode;
    node *res;

    DBUG_ENTER ();

    lhsavis = INFO_AVIS (arg_info);
    shapeavis = ID_AVIS (AVIS_SHAPE (lhsavis));

    DBUG_ASSERT (NULL != AVIS_SHAPE (ID_AVIS (arg_node)), "NULL AVIS_SHAPE");
    rhsnode = DUPdoDupNode (AVIS_SHAPE (ID_AVIS (arg_node)));
    res = TBmakeAssign (TBmakeLet (TBmakeIds (shapeavis, NULL), rhsnode), NULL);
    AVIS_SSAASSIGN (shapeavis) = res;

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *MSEfuncond( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
MSEfuncond (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *MSEarray( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
MSEarray (node *arg_node, info *arg_info)
{
    node *lhsavis;
    node *shpavis;
    node *rhsnode;
    node *res = NULL;
    node *preass = NULL;
    shape *cshape;
    int framedim;
    node *fsavis;

    node *csavis;

    DBUG_ENTER ();

    lhsavis = INFO_AVIS (arg_info);
    shpavis = ID_AVIS (AVIS_SHAPE (lhsavis));

    if (ARRAY_AELEMS (arg_node) == NULL) {
        DBUG_ASSERT (TUshapeKnown (ARRAY_ELEMTYPE (arg_node)),
                     "Empty array without AKS elements encountered!");

        cshape = SHappendShapes (ARRAY_FRAMESHAPE (arg_node),
                                 TYgetShape (ARRAY_ELEMTYPE (arg_node)));
        rhsnode = SHshape2Array (cshape);
        cshape = SHfreeShape (cshape);
    } else if (NODE_TYPE (EXPRS_EXPR (ARRAY_AELEMS (arg_node))) != N_id) {
        rhsnode = SHshape2Array (ARRAY_FRAMESHAPE (arg_node));
    } else {
        framedim = SHgetDim (ARRAY_FRAMESHAPE (arg_node));
        fsavis = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                                       SHcreateShape (1, framedim)));

        AVIS_DIM (fsavis) = TBmakeNum (1);
        AVIS_SHAPE (fsavis) = GenIntVector (TBmakeNum (framedim));

        FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
          = TBmakeVardec (fsavis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

        preass = TBmakeAssign (TBmakeLet (TBmakeIds (fsavis, NULL),
                                          SHshape2Array (ARRAY_FRAMESHAPE (arg_node))),
                               NULL);

        AVIS_SSAASSIGN (fsavis) = preass;

        csavis = MakeAssignForIdShape (EXPRS_EXPR (ARRAY_AELEMS (arg_node)),
                                       INFO_FUNDEF (arg_info), &preass);

        rhsnode = TCmakePrf2 (F_cat_VxV, TBmakeId (fsavis), TBmakeId (csavis));
    }

    res = TBmakeAssign (TBmakeLet (TBmakeIds (shpavis, NULL), rhsnode), NULL);
    AVIS_SSAASSIGN (shpavis) = res;

    res = TCappendAssign (preass, res);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *MSEap( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
MSEap (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *MSEprf( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
MSEprf (node *arg_node, info *arg_info)
{
    node *lhsavis;
    node *shpavis;
    node *rhsnode = NULL;
    node *res = NULL;
    node *preass = NULL;

    DBUG_ENTER ();

    lhsavis = INFO_AVIS (arg_info);
    shpavis = ID_AVIS (AVIS_SHAPE (lhsavis));

    if (makeshp_funtab[PRF_PRF (arg_node)] != NULL) {
        rhsnode = makeshp_funtab[PRF_PRF (arg_node)](arg_node, arg_info);

        /*
         * there are some primitive functions where the shape
         * might not be known!
         */
        if (rhsnode != NULL) {
            preass = INFO_PREASS (arg_info);
            INFO_PREASS (arg_info) = NULL;

            res = TBmakeAssign (TBmakeLet (TBmakeIds (shpavis, NULL), rhsnode), NULL);

            AVIS_SSAASSIGN (shpavis) = res;

            res = TCappendAssign (preass, res);
        }
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *MSEwith( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
MSEwith (node *arg_node, info *arg_info)
{
    node *lhsavis;
    node *shpavis;
    node *rhsnode = NULL;
    node *res = NULL;
    node *preass = NULL;
    node *ids;
    node *withop;
    int woc = 0;

    DBUG_ENTER ();

    lhsavis = INFO_AVIS (arg_info);
    shpavis = ID_AVIS (AVIS_SHAPE (lhsavis));

    ids = INFO_ALLIDS (arg_info);
    withop = WITH_WITHOP (arg_node);

    while (IDS_AVIS (ids) != lhsavis) {
        ids = IDS_NEXT (ids);
        withop = WITHOP_NEXT (withop);
        woc++;
    }

    switch (NODE_TYPE (withop)) {
    case N_modarray:
        rhsnode = DUPdoDupNode (AVIS_SHAPE (ID_AVIS (MODARRAY_ARRAY (withop))));
        break;

    case N_break:
        rhsnode = TCmakeIntVector (NULL);
        break;

    case N_genarray: {
        node *fsavis;
        node *csavis;
        node *genshp;
        shape *cshp = NULL;
        node *code = WITH_CODE (arg_node);
        ntype *min_type = NULL;

        /* Search for the most precise body type of all partitions
         * and provide it in min_type.
         */
        while (code != NULL) {
            int i;
            node *exprs = CODE_CEXPRS (code);

            for (i = 0; i < woc; i++)
                exprs = EXPRS_NEXT (exprs);

            if ((min_type == NULL)
                || TYleTypes (ID_NTYPE (EXPRS_EXPR (exprs)), min_type)) {
                min_type = ID_NTYPE (EXPRS_EXPR (exprs));
            }

            code = CODE_NEXT (code);
        }

        /*
         * If the min_type is AKS, we can provide the cell shape of the WL
         * as a constant. This is always better than using the shape of the default
         * element (see bug 1185 which was a result of earlier code that preferred
         * using the default element!)
         */
        if (TUshapeKnown (min_type)) {
            cshp = TYgetShape (min_type);
            csavis = TBmakeAvis (TRAVtmpVar (),
                                 TYmakeAKS (TYmakeSimpleType (T_int),
                                            SHcreateShape (1, SHgetDim (cshp))));

            AVIS_DIM (csavis) = TBmakeNum (1);
            AVIS_SHAPE (csavis) = GenIntVector (TBmakeNum (SHgetDim (cshp)));

            FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
              = TBmakeVardec (csavis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

            preass
              = TBmakeAssign (TBmakeLet (TBmakeIds (csavis, NULL), SHshape2Array (cshp)),
                              NULL);

            AVIS_SSAASSIGN (csavis) = preass;
        } else {
            /*
             * We do not know the cell shape from the bodies, this means we must have
             * a default element; otherwise the compiler should have complained earlier!
             */
            DBUG_ASSERT (GENARRAY_DEFAULT (withop) != NULL,
                         "Genarray WL without AKS elements"
                         "requires default element!");
            /*
             * Since we do have the default element we can use it to determine the
             * cell shape dynamically.
             */
            csavis = MakeAssignForIdShape (GENARRAY_DEFAULT (withop),
                                           INFO_FUNDEF (arg_info), &preass);
            /*
             * If the default shape is at least as good as the shape info from the bodies
             * in min_type, we are good. Unfortunately, we can have one constellation
             * where the bodies provide more information that the default element while
             * not being AKS: if the min_type is AKD while the default type is less
             * precise In that situation we potentially can run into bug 1185 again, ie
             * have a temporary loss of type precision which is not appreciated by the
             * subsequent type upgrade. To prevent from that, we insert an F_type_conv:
             */
            if (!TUdimKnown (ID_NTYPE (GENARRAY_DEFAULT (withop)))
                && TUdimKnown (min_type)) {
                /*
                 * modify SSA_ASSIGN(csavis)
                 * from    mse_var = def_shp_id;
                 * into    mse_var = F_type_conv( int[dim(min_type)], def_shp_id);
                 */
                csavis = InjectTypeConv (TYgetDim (min_type), csavis);
            }
        }

        genshp = GENARRAY_SHAPE (withop);
        if (NODE_TYPE (genshp) == N_id) {
            fsavis = ID_AVIS (genshp);
        } else {
            size_t framedim = TCcountExprs (ARRAY_AELEMS (genshp));

            fsavis = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                                           SHcreateShape (1, framedim)));

            AVIS_DIM (fsavis) = TBmakeNum (1);
            AVIS_SHAPE (fsavis) = GenIntVector (TBmakeNum (framedim));

            FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
              = TBmakeVardec (fsavis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

            preass
              = TBmakeAssign (TBmakeLet (TBmakeIds (fsavis, NULL), DUPdoDupNode (genshp)),
                              preass);

            AVIS_SSAASSIGN (fsavis) = preass;
        }

        rhsnode = TCmakePrf2 (F_cat_VxV, TBmakeId (fsavis), TBmakeId (csavis));
    } break;

    case N_fold:
        break;

    default:
        DBUG_UNREACHABLE ("Unknown Withop encountered");
        break;
    }

    if (rhsnode != NULL) {
        res = TBmakeAssign (TBmakeLet (TBmakeIds (shpavis, NULL), rhsnode), NULL);

        AVIS_SSAASSIGN (shpavis) = res;

        res = TCappendAssign (preass, res);
    }

    DBUG_RETURN (res);
}

/** <!--******************************************************************-->
 *
 * Macro for defining the traversal of nodes representing the primitive
 * data types.
 *
 ***************************************************************************/
#define MSECONST(name)                                                                   \
    node *MSE##name (node *arg_node, info *arg_info)                                     \
    {                                                                                    \
        node *lhsavis;                                                                   \
        node *shpavis;                                                                   \
        node *rhsnode;                                                                   \
        node *res;                                                                       \
                                                                                         \
        DBUG_ENTER ();                                                                   \
                                                                                         \
        lhsavis = INFO_AVIS (arg_info);                                                  \
        shpavis = ID_AVIS (AVIS_SHAPE (lhsavis));                                        \
                                                                                         \
        rhsnode = TCmakeIntVector (NULL);                                                \
                                                                                         \
        res = TBmakeAssign (TBmakeLet (TBmakeIds (shpavis, NULL), rhsnode), NULL);       \
                                                                                         \
        AVIS_SSAASSIGN (shpavis) = res;                                                  \
                                                                                         \
        DBUG_RETURN (res);                                                               \
    }

/** <!--******************************************************************-->
 *
 * @fn node *MSEbool( node *arg_node, info *arg_info)
 * @fn node *MSEchar( node *arg_node, info *arg_info)
 * @fn node *MSEfloat( node *arg_node, info *arg_info)
 * @fn node *MSEdouble( node *arg_node, info *arg_info)
 * @fn node *MSEnum( node *arg_node, info *arg_info)
 *
 ***************************************************************************/
MSECONST (bool)
MSECONST (char)
MSECONST (float)
MSECONST (floatvec)
MSECONST (double)
MSECONST (num)
MSECONST (numbyte)
MSECONST (numshort)
MSECONST (numint)
MSECONST (numlong)
MSECONST (numlonglong)
MSECONST (numubyte)
MSECONST (numushort)
MSECONST (numuint)
MSECONST (numulong)
MSECONST (numulonglong)

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Make Shape Expression -->
 *****************************************************************************/

#undef DBUG_PREFIX
