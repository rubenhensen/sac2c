/** <!--********************************************************************-->
 *
 * @defgroup phut polyhedral analysis utility functions
 *
 *  Overview: These functions are intended to provide useful
 *            manipulation services for polyhedron-based analysis.
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

#ifdef UNDERCONSTRUCTION
#include "/usr/local/include/polylib/polyhedron.h"

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!-- ****************************************************************** -->
 *
 * @fn node *collectAvisMin( node *arg_node)
 *
 * @brief Possibly generate fake N_prf for AVIS_MIN( nid)
 *
 * @param An N_id
 *
 * @return An N_prf or NULL
 *
 ******************************************************************************/
node *
collectAvisMin (node *arg_node)
{
    node *z = NULL;

    DBUG_ENTER ();

    if (NULL != AVIS_MIN (arg_node)) {
        z = TCmakePrf2 (F_ge_SxS, DUPdoDUPNode (arg_node),
                        DUPdoDupNode (AVIS_MIN (arg_node)));
        // should recurse here
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *collectAvisMax( node *arg_node)
 *
 * @brief Possibly generate fake N_prf for AVIS_MAX of arg_node
 *
 * @param An N_id
 *
 * @return An N_prf or NULL
 *
 ******************************************************************************/
node *
collectAvisMax (node *arg_node)
{
    node *z = NULL;

    DBUG_ENTER ();

    if (NULL != AVIS_MAX (arg_node)) {
        z = TCmakePrf2 (F_lt_SxS, DUPdoDUPNode (arg_node),
                        DUPdoDupNode (AVIS_MAX (arg_node)));
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 * @fn Matrix *PHUTcreateMatrix( unsigned rows, unsigned cols, int[] vals)
 *
 * @brief Create polylib matrix of shape [rows, cols], with ravel
 *        values of vals.
 *
 * @param See above.
 *
 * @return The new Matrix
 *
 ******************************************************************************/
Matrix *
PHUTcreateMatrix (unsigned rows, unsigned cols, int[] vals)
{
    Matrix *m;
    unsigned i, j, indx;

    DBUG_ENTER ();

    m = Matrix_Alloc (rows, cols);

    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            indx = j + (cols * i);
            value_assign (m->p[i][j], vals[indx]);
        }
    }

    DBUG_RETURN (z);
}
#endif UNDERCONSTRUCTION

/** <!-- ****************************************************************** -->
 * @fn
 *
 * @brief
 *
 * @param
 *
 * @return
 *
 ******************************************************************************/
prf
PHUTnormalizePrf (prf prf)
{
    DBUG_ENTER ();

    switch (prf) {
    case F_add_SxS:
    case F_add_SxV:
    case F_add_VxS:
    case F_add_VxV:
        prf = F_add_SxS;
        break;

    case F_mul_SxS:
    case F_mul_SxV:
    case F_mul_VxS:
    case F_mul_VxV:
        prf = F_mul_SxS;
        break;

    case F_neg_S:
    case F_neg_V:
        prf = F_mul_SxS;
        break;

    default:
        break;
    }

    DBUG_RETURN (prf);
}

/*
 * The classification of primitive functions should rather be
 * derived from corresponding information in prf_info.mac for
 * use here as well as in other compiler modules.
 * cg currently lacks the time to implement this.
 */
static bool
IsGuardPrf (prf op)
{
    bool res;

    DBUG_ENTER ();

    switch (op) {
        //  case F_guard:
        //  disbabled until the format of F_guard has been decided upon.
    case F_same_shape_AxA:
    case F_shape_matches_dim_VxA:
    case F_non_neg_val_S:
    case F_non_neg_val_V:
    case F_val_lt_shape_VxA:
    case F_val_lt_val_SxS:
    case F_val_le_val_SxS:
    case F_val_le_val_VxV:
    case F_prod_matches_prod_shape_VxA:
        res = TRUE;
        break;
    default:
        res = FALSE;
    }

    DBUG_RETURN (FALSE);
}

static bool
compatiblePrf (prf p1, prf p2)
{
    bool res;

    DBUG_ENTER ();

    res = (PHUTnormalizePrf (p1) == PHUTnormalizePrf (p2));

    DBUG_RETURN (res);
}

static bool
isArg1Scl (prf prf)
{
    bool res;
    DBUG_ENTER ();

    switch (prf) {
    case F_add_SxS:
    case F_add_SxV:
    case F_mul_SxS:
    case F_mul_SxV:
    case F_neg_S:
        res = TRUE;
        break;

    default:
        res = FALSE;
    }

    DBUG_RETURN (res);
}

static bool
isArg2Scl (prf prf)
{
    bool res;
    DBUG_ENTER ();

    switch (prf) {
    case F_add_SxS:
    case F_add_VxS:
    case F_mul_SxS:
    case F_mul_VxS:
        res = TRUE;
        break;
    default:
        res = FALSE;
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn node *PHUTcollectAffineExprs( node *arg_node)
 *
 * @brief Does a recursive search, starting at arg_node,
 *        for a maximal chain of affine instructions.
 *
 * @param arg_node: an N_id node or...
 *
 * @return A maximal N_exprs chain of expressions. E.g.:
 *
 * This is from AWLF unit test time2code.sac. If compiled with -doawlf -nowlf,
 * it generates this code:
 *
 *     p = _aplmod_SxS_( -1, m);   NB. AVIS_MIN(p) = 0;
 *     iv2 = _notemaxval( iv1, m);
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
 *
 *
 ******************************************************************************/
node *
PHUTcollectAffineExprs (node *arg_node)
{
    node *res = NULL;
    node *rhs;
    node *left, *right;
    node *ids, *exprs;

    DBUG_ENTER ();
#ifdef UNDERCONST
    if (NULL != arg_node) {
        if (AVIS_ISDEFINEDINCURRENTBLOCK (ID_AVIS (arg_node))
            && AVIS_SSAASSIGN (ID_AVIS (arg_node)) != NULL) {
            rhs = ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (arg_node)));

            switch (NODE_TYPE (rhs)) {

            case N_id:
                // res = PHUTcollectExprs( prf, rhs, sclprf);
                break;

            case N_prf:
                if (compatiblePrf (prf, PRF_PRF (rhs))) {
                    // left  = PHUTcollectExprs( prf, PRF_ARG1( rhs), isArg1Scl( PRF_PRF(
                    // rhs)));  right = PHUTcollectExprs( prf, PRF_ARG2( rhs), isArg2Scl(
                    // PRF_PRF( rhs)));
                    res = TCappendExprs (left, right);
                } else if (IsGuardPrf (PRF_PRF (rhs))) {
                    ids = LET_IDS (ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (arg_node))));
                    exprs = PRF_ARGS (rhs);
                    while (IDS_AVIS (ids) != ID_AVIS (arg_node) && exprs != NULL) {
                        ids = IDS_NEXT (ids);
                        exprs = EXPRS_NEXT (exprs);
                        DBUG_ASSERT (ids != NULL,
                                     "Syntax tree broken: "
                                     "AVIS must be found within IDS of AVIS_SSAASSIGN");
                    }
                    if (exprs != NULL && IDS_NEXT (ids) != NULL) {
                        /*
                         * we only continue through guards if we are in the transparent
                         * data flow through the guard, i.e. there is a right hand side
                         * corresponding expression and this is not the last left hand
                         * side identifier, i.e. not the guard's predicate.
                         */
                        DBUG_PRINT ("Ignoring guard: %s", global.prf_name[PRF_PRF (rhs)]);
                        DBUG_ASSERT (TYeqTypes (IDS_NTYPE (ids),
                                                ID_NTYPE (EXPRS_EXPR (exprs))),
                                     "Bug in guards: result id '%s' and arg id '%s' do "
                                     "have "
                                     "different types",
                                     IDS_NAME (ids), ID_NAME (EXPRS_EXPR (exprs)));
                        // res = PHUTcollectExprs( prf, EXPRS_EXPR( exprs), sclprf);
                    }
                }
                break;

            default:
                break;
            }
        }

        if (res == NULL) {
            res = TBmakeExprs (DUPdoDupNode (arg_node), NULL);
            ID_ISSCLPRF (EXPRS_EXPR (res)) = sclprf;
        }
    }
#endif // UNDERCONST

    DBUG_RETURN (res);
}

#undef DBUG_PREFIX
