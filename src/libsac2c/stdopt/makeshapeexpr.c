/*
 * $Id$
 */

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
#include "dbug.h"
#include "str.h"
#include "memory.h"
#include "DupTree.h"
#include "free.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"
#include "constants.h"
#include "compare_tree.h"

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
};

#define INFO_AVIS(n) ((n)->avis)
#define INFO_ALLIDS(n) ((n)->allids)
#define INFO_FUNDEF(n) ((n)->fundef)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_AVIS (result) = NULL;
    INFO_ALLIDS (result) = NULL;
    INFO_FUNDEF (result) = NULL;

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
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/
static node *
GenIntVector (node *element)
{
    node *res = NULL;

    DBUG_ENTER ("GenIntVector");

    res = TCmakeIntVector (TBmakeExprs (element, NULL));

    DBUG_RETURN (res);
}

static node *
MakeAssignForIdShape (node *id, node *fundef, node **preass)
{
    node *res;
    node *newass;

    DBUG_ENTER ("MakeAvisForIdShape");

    DBUG_ASSERT ((NULL != AVIS_DIM (ID_AVIS (id))),
                 "Making assign for Id without Dimension!");

    if (NODE_TYPE (AVIS_DIM (ID_AVIS (id))) == N_num) {
        int dim = NUM_VAL (AVIS_DIM (ID_AVIS (id)));
        res = TBmakeAvis (TRAVtmpVarName (ID_NAME (id)),
                          TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, dim)));
    } else {
        res = TBmakeAvis (TRAVtmpVarName (ID_NAME (id)),
                          TYmakeAKD (TYmakeSimpleType (T_int), 1, SHmakeShape (0)));
    }

    AVIS_DIM (res) = TBmakeNum (1);
    AVIS_SHAPE (res) = GenIntVector (DUPdoDupNode (AVIS_DIM (ID_AVIS (id))));

    FUNDEF_VARDEC (fundef) = TBmakeVardec (res, FUNDEF_VARDEC (fundef));

    newass = TBmakeAssign (TBmakeLet (TBmakeIds (res, NULL),
                                      DUPdoDupNode (AVIS_SHAPE (ID_AVIS (id)))),
                           NULL);

    AVIS_SSAASSIGN (res) = newass;

    *preass = TCappendAssign (*preass, newass);

    DBUG_RETURN (res);
}

static node *
MakeVectAvis (char *name, node *dim)
{
    node *res;

    DBUG_ENTER ("MakeVectAvis");

    if (NODE_TYPE (dim) == N_num) {
        res = TBmakeAvis (name, TYmakeAKS (TYmakeSimpleType (T_int),
                                           SHcreateShape (1, NUM_VAL (dim))));
    } else {
        res = TBmakeAvis (name, TYmakeAKD (TYmakeSimpleType (T_int), 1, SHmakeShape (0)));
    }

    AVIS_DIM (res) = TBmakeNum (1);
    AVIS_SHAPE (res) = GenIntVector (DUPdoDupNode (dim));

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

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

    DBUG_ENTER ("MSEdoMakeShapeExpression");

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
        FUNDEF_VARDEC (fundef) = TBmakeVardec (shpavis, FUNDEF_VARDEC (fundef));
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

    DBUG_ENTER ("MSEid");

    lhsavis = INFO_AVIS (arg_info);
    shapeavis = ID_AVIS (AVIS_SHAPE (lhsavis));

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

    DBUG_ENTER ("MSEfuncond");

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

    DBUG_ENTER ("MSEarray");

    lhsavis = INFO_AVIS (arg_info);
    shpavis = ID_AVIS (AVIS_SHAPE (lhsavis));

    if (ARRAY_AELEMS (arg_node) == NULL) {
        shape *cshape;

        DBUG_ASSERT (TUshapeKnown (ARRAY_ELEMTYPE (arg_node)),
                     "Empty array without AKS elements encountered!");

        cshape = SHappendShapes (ARRAY_SHAPE (arg_node),
                                 TYgetShape (ARRAY_ELEMTYPE (arg_node)));

        rhsnode = SHshape2Array (cshape);

        cshape = SHfreeShape (cshape);
    } else if (NODE_TYPE (EXPRS_EXPR (ARRAY_AELEMS (arg_node))) != N_id) {
        rhsnode = SHshape2Array (ARRAY_SHAPE (arg_node));
    } else {
        int framedim;
        node *fsavis;

        node *csavis;

        framedim = SHgetDim (ARRAY_SHAPE (arg_node));
        fsavis = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                                       SHcreateShape (1, framedim)));

        AVIS_DIM (fsavis) = TBmakeNum (1);
        AVIS_SHAPE (fsavis) = GenIntVector (TBmakeNum (framedim));

        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = TBmakeVardec (fsavis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

        preass = TBmakeAssign (TBmakeLet (TBmakeIds (fsavis, NULL),
                                          SHshape2Array (ARRAY_SHAPE (arg_node))),
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

    DBUG_ENTER ("MSEap");

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

    DBUG_ENTER ("MSEprf");

    lhsavis = INFO_AVIS (arg_info);
    shpavis = ID_AVIS (AVIS_SHAPE (lhsavis));

    switch (PRF_PRF (arg_node)) {
    case F_dim:
    case F_idxs2offset:
    case F_vect2offset:
    case F_sel:
    case F_idx_shape_sel:
    case F_add_SxS:
    case F_sub_SxS:
    case F_mul_SxS:
    case F_div_SxS:
    case F_toi_S:
    case F_tof_S:
    case F_tod_S:
    case F_mod:
    case F_min:
    case F_max:
    case F_idx_sel:
    case F_idx_modarray:
        rhsnode = TCmakeIntVector (NULL);
        break;

    case F_modarray:
    case F_copy:
    case F_neg:
    case F_not:
    case F_abs:
    case F_add_AxS:
    case F_sub_AxS:
    case F_mul_AxS:
    case F_div_AxS:
    case F_add_AxA:
    case F_mul_AxA:
    case F_sub_AxA:
    case F_div_AxA:
    case F_and: /* According to prf_node_info.mac, these are AxA */
    case F_or:
    case F_le:
    case F_lt:
    case F_eq:
    case F_neq:
    case F_ge:
    case F_gt:
        rhsnode = DUPdoDupNode (AVIS_SHAPE (ID_AVIS (PRF_ARG1 (arg_node))));
        break;

    case F_add_SxA:
    case F_sub_SxA:
    case F_mul_SxA:
    case F_div_SxA:
        rhsnode = DUPdoDupNode (AVIS_SHAPE (ID_AVIS (PRF_ARG2 (arg_node))));
        break;

    case F_reshape:
        rhsnode = DUPdoDupNode (PRF_ARG1 (arg_node));
        break;

    case F_shape:
        if (AVIS_DIM (ID_AVIS (PRF_ARG1 (arg_node))) != NULL) {
            node *adim = AVIS_DIM (ID_AVIS (PRF_ARG1 (arg_node)));
            rhsnode = TCmakeIntVector (TBmakeExprs (DUPdoDupNode (adim), NULL));
        }
        break;

    case F_take_SxV: {
        node *scalar;

        if (NODE_TYPE (PRF_ARG1 (arg_node)) == N_num) {
            scalar = TBmakeNum (abs (NUM_VAL (PRF_ARG1 (arg_node))));
        } else {
            node *idavis;
            node *absavis;

            idavis = ID_AVIS (PRF_ARG1 (arg_node));

            absavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (idavis)),
                                  TYeliminateAKV (AVIS_TYPE (idavis)));

            AVIS_DIM (absavis) = DUPdoDupNode (AVIS_DIM (idavis));
            AVIS_SHAPE (absavis) = DUPdoDupNode (AVIS_SHAPE (idavis));

            FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
              = TBmakeVardec (absavis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

            preass = TBmakeAssign (TBmakeLet (TBmakeIds (absavis, NULL),
                                              TCmakePrf1 (F_abs, TBmakeId (idavis))),
                                   preass);

            AVIS_SSAASSIGN (absavis) = preass;

            scalar = TBmakeId (absavis);
        }

        rhsnode = TCmakeIntVector (TBmakeExprs (scalar, NULL));
    } break;

    case F_drop_SxV: {
        node *scalar;
        node *vsavis;

        vsavis
          = MakeAssignForIdShape (PRF_ARG2 (arg_node), INFO_FUNDEF (arg_info), &preass);

        if (NODE_TYPE (PRF_ARG1 (arg_node)) == N_num) {
            scalar = TBmakeNum (abs (NUM_VAL (PRF_ARG1 (arg_node))));
        } else {
            node *idavis;
            node *absavis;

            idavis = ID_AVIS (PRF_ARG1 (arg_node));

            absavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (idavis)),
                                  TYeliminateAKV (AVIS_TYPE (idavis)));

            AVIS_DIM (absavis) = DUPdoDupNode (AVIS_DIM (idavis));
            AVIS_SHAPE (absavis) = DUPdoDupNode (AVIS_SHAPE (idavis));

            FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
              = TBmakeVardec (absavis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

            preass = TBmakeAssign (TBmakeLet (TBmakeIds (absavis, NULL),
                                              TCmakePrf1 (F_abs, TBmakeId (idavis))),
                                   preass);

            AVIS_SSAASSIGN (absavis) = preass;

            scalar = TBmakeId (absavis);
        }
        rhsnode = TCmakePrf2 (F_sub_AxS, TBmakeId (vsavis), scalar);
    } break;

    case F_cat_VxV: {
        node *v1savis, *v2savis;

        v1savis
          = MakeAssignForIdShape (PRF_ARG1 (arg_node), INFO_FUNDEF (arg_info), &preass);

        v2savis
          = MakeAssignForIdShape (PRF_ARG2 (arg_node), INFO_FUNDEF (arg_info), &preass);

        rhsnode = TCmakePrf2 (F_add_AxA, TBmakeId (v1savis), TBmakeId (v2savis));
    } break;

    case F_saabind:
        rhsnode = DUPdoDupNode (PRF_ARG2 (arg_node));
        break;

    case F_accu:
        break;

    default:
        break;
    }

    if (rhsnode != NULL) {
        res = TBmakeAssign (TBmakeLet (TBmakeIds (shpavis, NULL), rhsnode), NULL);

        AVIS_SSAASSIGN (shpavis) = res;

        res = TCappendAssign (preass, res);
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

    DBUG_ENTER ("MSEwith");

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

        if (GENARRAY_DEFAULT (withop) != NULL) {
            csavis = MakeAssignForIdShape (GENARRAY_DEFAULT (withop),
                                           INFO_FUNDEF (arg_info), &preass);
        } else {
            shape *cshp = NULL;
            node *code = WITH_CODE (arg_node);

            while (code != NULL) {
                int i;
                node *exprs = CODE_CEXPRS (code);
                for (i = 0; i < woc; i++)
                    exprs = EXPRS_NEXT (exprs);

                if (TUshapeKnown (ID_NTYPE (EXPRS_EXPR (exprs)))) {
                    cshp = TYgetShape (ID_NTYPE (EXPRS_EXPR (exprs)));
                    break;
                }

                code = CODE_NEXT (code);
            }

            DBUG_ASSERT (cshp != NULL, "Genarray WL without default element requires "
                                       "AKS elements!");

            csavis = TBmakeAvis (TRAVtmpVar (),
                                 TYmakeAKS (TYmakeSimpleType (T_int),
                                            SHcreateShape (1, SHgetDim (cshp))));

            AVIS_DIM (csavis) = TBmakeNum (1);
            AVIS_SHAPE (csavis) = GenIntVector (TBmakeNum (SHgetDim (cshp)));

            FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
              = TBmakeVardec (csavis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

            preass
              = TBmakeAssign (TBmakeLet (TBmakeIds (csavis, NULL), SHshape2Array (cshp)),
                              NULL);

            AVIS_SSAASSIGN (csavis) = preass;
        }

        genshp = GENARRAY_SHAPE (withop);
        if (NODE_TYPE (genshp) == N_id) {
            fsavis = ID_AVIS (genshp);
        } else {
            int framedim = TCcountExprs (ARRAY_AELEMS (genshp));

            fsavis = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                                           SHcreateShape (1, framedim)));

            AVIS_DIM (fsavis) = TBmakeNum (1);
            AVIS_SHAPE (fsavis) = GenIntVector (TBmakeNum (framedim));

            FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
              = TBmakeVardec (fsavis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

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
        DBUG_ASSERT ((0), "Unknown Withop encountered");
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
        DBUG_ENTER ("MSE" #name);                                                        \
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
MSECONST (double)
MSECONST (num)

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Make Dim Expression -->
 *****************************************************************************/
