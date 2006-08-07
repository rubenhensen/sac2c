/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup mde Make Dim Expression
 *
 * Utility traversal to provide expressions for computing array dimensions.
 *
 * @ingroup isv
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file makedimexpr.c
 *
 * Prefix: MDE
 *
 *****************************************************************************/
#include "makedimexpr.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "internal_lib.h"
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

    result = ILIBmalloc (sizeof (info));

    INFO_AVIS (result) = NULL;
    INFO_ALLIDS (result) = NULL;
    INFO_FUNDEF (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

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
MakeScalarAvis (char *name)
{
    node *res;

    DBUG_ENTER ("MakeScalarAvis");

    res = TBmakeAvis (name, TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));

    AVIS_DIM (res) = TBmakeNum (0);
    AVIS_SHAPE (res) = TCmakeIntVector (NULL);

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
 * @fn node *MDEdoMakeDimExpression( node *expr, node *avis,
 *                                   node *allids, node *fundef)
 *
 *****************************************************************************/
node *
MDEdoMakeDimExpression (node *expr, node *avis, node *allids, node *fundef)
{
    info *info;
    node *res;
    node *dimavis;

    DBUG_ENTER ("MDEdoMakeDimExpression");

    DBUG_ASSERT (AVIS_DIM (avis) == NULL, "AVIS_DIM( avis) is already set!");

    info = MakeInfo ();

    INFO_AVIS (info) = avis;
    INFO_ALLIDS (info) = allids;
    INFO_FUNDEF (info) = fundef;

    dimavis = MakeScalarAvis (ILIBtmpVarName (AVIS_NAME (avis)));
    AVIS_DIM (avis) = TBmakeId (dimavis);

    TRAVpush (TR_mde);
    res = TRAVdo (expr, info);
    TRAVpop ();

    info = FreeInfo (info);

    if (res != NULL) {
        FUNDEF_VARDEC (fundef) = TBmakeVardec (dimavis, FUNDEF_VARDEC (fundef));
    } else {
        AVIS_DIM (avis) = FREEdoFreeNode (AVIS_DIM (avis));
        dimavis = FREEdoFreeNode (dimavis);
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
 * @fn node *MDEid( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
MDEid (node *arg_node, info *arg_info)
{
    node *lhsavis;
    node *dimavis;
    node *rhsnode;
    node *res;

    DBUG_ENTER ("MDEid");

    lhsavis = INFO_AVIS (arg_info);
    dimavis = ID_AVIS (AVIS_DIM (lhsavis));

    rhsnode = DUPdoDupNode (AVIS_DIM (ID_AVIS (arg_node)));
    res = TBmakeAssign (TBmakeLet (TBmakeIds (dimavis, NULL), rhsnode), NULL);
    AVIS_SSAASSIGN (dimavis) = res;

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *MDEfuncond( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
MDEfuncond (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("MDEfuncond");

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *MDEarray( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
MDEarray (node *arg_node, info *arg_info)
{
    node *lhsavis;
    node *dimavis;
    node *rhsnode;
    node *res = NULL;

    DBUG_ENTER ("MDEarray");

    lhsavis = INFO_AVIS (arg_info);
    dimavis = ID_AVIS (AVIS_DIM (lhsavis));

    if (ARRAY_AELEMS (arg_node) == NULL) {
        DBUG_ASSERT (TUshapeKnown (ARRAY_ELEMTYPE (arg_node)),
                     "Empty array without AKS elements encountered!");

        rhsnode = TBmakeNum (SHgetDim (ARRAY_SHAPE (arg_node))
                             + TYgetDim (ARRAY_ELEMTYPE (arg_node)));
    } else if (NODE_TYPE (EXPRS_EXPR (ARRAY_AELEMS (arg_node))) != N_id) {
        rhsnode = TBmakeNum (SHgetDim (ARRAY_SHAPE (arg_node)));
    } else {
        node *framedim;
        node *celldim;

        framedim = TBmakeNum (SHgetDim (ARRAY_SHAPE (arg_node)));
        celldim = AVIS_DIM (ID_AVIS (EXPRS_EXPR (ARRAY_AELEMS (arg_node))));

        rhsnode = TCmakePrf2 (F_add_SxS, framedim, DUPdoDupNode (celldim));
    }

    res = TBmakeAssign (TBmakeLet (TBmakeIds (dimavis, NULL), rhsnode), NULL);

    AVIS_SSAASSIGN (dimavis) = res;

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *MDEap( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
MDEap (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("MDEap");

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *MDEprf( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
MDEprf (node *arg_node, info *arg_info)
{
    node *lhsavis;
    node *dimavis;
    node *rhsnode = NULL;
    node *res = NULL;

    DBUG_ENTER ("MDEprf");

    lhsavis = INFO_AVIS (arg_info);
    dimavis = ID_AVIS (AVIS_DIM (lhsavis));

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
        rhsnode = TBmakeNum (0);
        break;

    case F_shape:
    case F_drop_SxV:
    case F_take_SxV:
    case F_cat_VxV:
        rhsnode = TBmakeNum (1);
        break;

    case F_modarray:
    case F_copy:
    case F_toi_A:
    case F_tof_A:
    case F_tod_A:
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
        rhsnode = DUPdoDupNode (AVIS_DIM (ID_AVIS (PRF_ARG1 (arg_node))));
        break;

    case F_add_SxA:
    case F_sub_SxA:
    case F_mul_SxA:
    case F_div_SxA:
        rhsnode = DUPdoDupNode (AVIS_DIM (ID_AVIS (PRF_ARG2 (arg_node))));
        break;

    case F_reshape:
        rhsnode = TCmakePrf2 (F_idx_shape_sel, TBmakeNum (0),
                              DUPdoDupNode (PRF_ARG1 (arg_node)));
        break;

    case F_dtype_conv:
        rhsnode = DUPdoDupNode (PRF_ARG1 (arg_node));
        break;

    case F_accu:
        break;

    default:
        break;
    }

    if (rhsnode != NULL) {
        res = TBmakeAssign (TBmakeLet (TBmakeIds (dimavis, NULL), rhsnode), NULL);

        AVIS_SSAASSIGN (dimavis) = res;
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *MDEwith( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
MDEwith (node *arg_node, info *arg_info)
{
    node *lhsavis;
    node *dimavis;
    node *rhsnode = NULL;
    node *res = NULL;
    node *ids;
    node *withop;
    int woc = 0;

    DBUG_ENTER ("MDEwith");

    lhsavis = INFO_AVIS (arg_info);
    dimavis = ID_AVIS (AVIS_DIM (lhsavis));

    ids = INFO_ALLIDS (arg_info);
    withop = WITH_WITHOP (arg_node);

    while (IDS_AVIS (ids) != lhsavis) {
        ids = IDS_NEXT (ids);
        withop = WITHOP_NEXT (withop);
        woc++;
    }

    switch (NODE_TYPE (withop)) {
    case N_modarray:
        rhsnode = DUPdoDupNode (AVIS_DIM (ID_AVIS (MODARRAY_ARRAY (withop))));
        break;

    case N_break:
        rhsnode = TBmakeNum (0);
        break;

    case N_genarray: {
        node *framedim = NULL;
        node *celldim = NULL;
        node *genshp;

        if (GENARRAY_DEFAULT (withop) != NULL) {
            celldim = DUPdoDupNode (AVIS_DIM (ID_AVIS (GENARRAY_DEFAULT (withop))));
        } else {
            node *code = WITH_CODE (arg_node);

            while (code != NULL) {
                int i;
                node *exprs = CODE_CEXPRS (code);
                for (i = 0; i < woc; i++)
                    exprs = EXPRS_NEXT (exprs);

                if (TUshapeKnown (ID_NTYPE (EXPRS_EXPR (exprs)))) {
                    celldim = TBmakeNum (TYgetDim (ID_NTYPE (EXPRS_EXPR (exprs))));
                    break;
                }

                code = CODE_NEXT (code);
            }

            DBUG_ASSERT (code != NULL, "Genarray WL without default element requires "
                                       "AKS elements!");
        }

        genshp = GENARRAY_SHAPE (withop);
        if (NODE_TYPE (genshp) == N_array) {
            framedim = TBmakeNum (TCcountExprs (ARRAY_AELEMS (genshp)));
        } else {
            node *fdavis;

            fdavis = TBmakeAvis (ILIBtmpVar (),
                                 TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));

            AVIS_DIM (fdavis) = TBmakeNum (0);
            AVIS_SHAPE (fdavis) = TCmakeIntVector (NULL);

            FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
              = TBmakeVardec (fdavis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

            res = TBmakeAssign (TBmakeLet (TBmakeIds (fdavis, NULL),
                                           TCmakePrf2 (F_idx_shape_sel, TBmakeNum (0),
                                                       DUPdoDupNode (genshp))),
                                NULL);

            AVIS_SSAASSIGN (fdavis) = res;

            framedim = TBmakeId (fdavis);
        }

        rhsnode = TCmakePrf2 (F_add_SxS, framedim, celldim);
    } break;

    case N_fold:
        break;

    default:
        DBUG_ASSERT ((0), "Unknown Withop encountered");
        break;
    }

    if (rhsnode != NULL) {
        node *newass
          = TBmakeAssign (TBmakeLet (TBmakeIds (dimavis, NULL), rhsnode), NULL);

        AVIS_SSAASSIGN (dimavis) = newass;

        res = TCappendAssign (res, newass);
    }

    DBUG_RETURN (res);
}

/** <!--******************************************************************-->
 *
 * Macro for defining the traversal of nodes representing the primitive
 * data types.
 *
 ***************************************************************************/
#define MDECONST(name)                                                                   \
    node *MDE##name (node *arg_node, info *arg_info)                                     \
    {                                                                                    \
        node *lhsavis;                                                                   \
        node *dimavis;                                                                   \
        node *rhsnode;                                                                   \
        node *res;                                                                       \
                                                                                         \
        DBUG_ENTER ("MDE" #name);                                                        \
                                                                                         \
        lhsavis = INFO_AVIS (arg_info);                                                  \
        dimavis = ID_AVIS (AVIS_DIM (lhsavis));                                          \
                                                                                         \
        rhsnode = TBmakeNum (0);                                                         \
                                                                                         \
        res = TBmakeAssign (TBmakeLet (TBmakeIds (dimavis, NULL), rhsnode), NULL);       \
                                                                                         \
        AVIS_SSAASSIGN (dimavis) = res;                                                  \
                                                                                         \
        DBUG_RETURN (res);                                                               \
    }

/** <!--******************************************************************-->
 *
 * @fn node *MDEbool( node *arg_node, info *arg_info)
 * @fn node *MDEchar( node *arg_node, info *arg_info)
 * @fn node *MDEfloat( node *arg_node, info *arg_info)
 * @fn node *MDEdouble( node *arg_node, info *arg_info)
 * @fn node *MDEnum( node *arg_node, info *arg_info)
 *
 ***************************************************************************/
MDECONST (bool)
MDECONST (char)
MDECONST (float)
MDECONST (double)
MDECONST (num)

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Make Dim Expression -->
 *****************************************************************************/