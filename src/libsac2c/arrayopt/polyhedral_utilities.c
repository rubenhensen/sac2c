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
#include "polyhedral_utilities.h"
#include "print.h"

#ifdef UNDERCONSTRUCTION
#include "/usr/local/include/polylib/polyhedron.h"
#endif // UNDERCONSTRUCTION

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
typedef enum { MODE_enumeratevars, MODE_generatematrix, MODE_clearindices } trav_mode_t;

struct INFO {
    trav_mode_t mode;
    int polylibnumids;
    node *fundef;
};

/**
 * Macro definitions for INFO structure
 */
#define INFO_MODE(n) ((n)->mode)
#define INFO_POLYLIBNUMIDS(n) ((n)->polylibnumids)
#define INFO_FUNDEF(n) ((n)->fundef)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));
    INFO_MODE (result) = MODE_clearindices;
    INFO_POLYLIBNUMIDS (result) = 1;
    INFO_FUNDEF (result) = NULL;

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
 * @fn void CheckExprsChain( node *exprs, info *arg_info)
 *
 * @brief Ensure that exprs chain consists entirely of N_num nodes.
 *
 * @param An N_exprs chain
 *
 * @return void
 *
 ******************************************************************************/
static void
CheckExprsChain (node *exprs, info *arg_info)
{
    node *tmp;
    node *el;

    DBUG_ENTER ();

    tmp = exprs;
    if (MODE_generatematrix == INFO_MODE (arg_info)) {
        while (tmp != NULL) {
            el = EXPRS_EXPR (tmp);
            //  DBUG_EXECUTE( PRTdoPrintNode( el));
            DBUG_ASSERT (N_num == NODE_TYPE (el), "Not N_num!");
            //  DBUG_PRINT ("\n");
            tmp = EXPRS_NEXT (tmp);
            ;
        }
    }

    DBUG_RETURN ();
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *GenerateZeroExprs( int polylibcols, int relat);
 *
 * @brief Generate an N_exprs chain of length polylibcols,
 *        containing all zeros.
 *
 *        This chain, suitably modified, will comprise one row
 *        of the polylib input matrix.
 *
 *        The chain length is the number of distinct variables
 *        in the computation, plus two: one for the equality/inequality
 *        marker, and one for the constant last element of each matrix row.
 *
 * @param polylibcols: An integer, specifying the number of
 *                     distinct variables in the matrix
 *       relat: The operation specified by this matrix row, either
 *              == 0 or >= 0
 *
 * @return the generated chain, with two extra elements:
 *         One for the polylib equality/inequality marker, and one for
 *         the constant value.
 *
 ******************************************************************************/
static node *
GenerateZeroExprs (int polylibcols, int relat)
{
    node *z = NULL;
    int cnt;
    int val;

    DBUG_ENTER ();

    cnt = polylibcols + 2;
    DBUG_PRINT ("Generating ZeroExprs chain of length %d", cnt);
    val = relat;
    while (cnt > 0) {
        z = TCappendExprs (z, TBmakeExprs (TBmakeNum (val), NULL));
        val = 0;
        cnt--;
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *AddValueToColumn()
 *
 * @brief Perform matrix[ col] = val, where col is a previously assigned
 *        column index in the arg_node's N_avis. If col is -1, then
 *        the arg_node is a constant, and we assign the constant's value
 *        to the last column of matrix;
 *
 * @param arg_node: An N_id node or an N_ids node
 *        val: The value to be set for non-constants
 *        matrix: The N_exprs chain row of interest
 *        arg_info: as usual.
 *
 * @return The updated matrix
 *
 ******************************************************************************/
static node *
AddValueToColumn (node *arg_node, int val, node *matrix, info *arg_info)
{
    node *z = NULL;
    int col;
    node *avis;

    DBUG_ENTER ();

    CheckExprsChain (matrix, arg_info);
    if (N_id == NODE_TYPE (arg_node)) {
        avis = ID_AVIS (arg_node);
    } else {
        avis = IDS_AVIS (arg_node);
    }

    col = AVIS_POLYLIBCOLUMNINDEX (avis);
    if (-1 == col) {
        DBUG_ASSERT (TYisAKV (AVIS_TYPE (avis)), "Failure to assign column index");
        val = val * TUtype2Int (AVIS_TYPE (avis));
        col = 1 + INFO_POLYLIBNUMIDS (arg_info); // The (last) column, for constants
    }

    val = val + NUM_VAL (TCgetNthExprsExpr (col, matrix));
    z = TCputNthExprs (col, matrix, TBmakeNum (val));

    CheckExprsChain (z, arg_info);

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *collectAvisMin( node *arg_node)
 *
 * @brief Possibly generate polylib input matrix row
 *        for AVIS_MIN( arg_node) vs arg_node.
 *
 *        We know that:
 *         arg_node >= AVIS_MIN( arg_node)
 *       so we generate:
 *         arg_node - AVIS_MIN( arg_node) >= 0
 *
 * @param An N_id
 *
 * @return If in MODE_generatematrix, an N_exprs chain for the above relational.
 *         Otherwise, NULL
 *
 ******************************************************************************/
static node *
collectAvisMin (node *arg_node, info *arg_info)
{
    node *z = NULL;
    node *minmax;

    DBUG_ENTER ();

    minmax = AVIS_MIN (ID_AVIS (arg_node));
    if ((NULL != minmax) && (!AVIS_ISAFFINEHANDLED (ID_AVIS (minmax)))) {
        z = PHUTcollectAffineExprsLocal (minmax, arg_info);
        DBUG_PRINT ("Generating %s >= %s for AVIS_MIN", AVIS_NAME (ID_AVIS (arg_node)),
                    AVIS_NAME (ID_AVIS (minmax)));

        z = TCappendExprs (z, PHUTcollectAffineExprsLocal (minmax, arg_info));

        if (MODE_generatematrix == INFO_MODE (arg_info)) {
            z = GenerateZeroExprs (INFO_POLYLIBNUMIDS (arg_info), PLINEQUALITY);
            z = AddValueToColumn (arg_node, 1, z, arg_info); // arg_node
            z = AddValueToColumn (minmax, -1, z, arg_info);  // -minmax
        }
    }

    CheckExprsChain (z, arg_info);

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *collectAvisMax( node *arg_node)
 *
 * @brief Possibly generate polylib input matrix row for
 *        AVIS_MAX( arg_node) vs arg_node:
 *
 *        We know that:
 *         arg_node < AVIS_MAX( arg_node)         --->
 *         AVIS_MAX( arg_node) > arg_node         --->
 *         AVIS_MAX( arg_node) - arg_node) > 0    --->
 *
 *       so we generate:
 *
 *         AVIS_MAX( arg_node) + ( -1 * arg_node) - 1 >= 0
 *
 * @param An N_id
 *
 ******************************************************************************/
static node *
collectAvisMax (node *arg_node, info *arg_info)
{
    node *z = NULL;
    node *minmax;
    int col;
    int val;

    DBUG_ENTER ();

    minmax = AVIS_MAX (ID_AVIS (arg_node));
    if ((NULL != minmax) && (!AVIS_ISAFFINEHANDLED (ID_AVIS (minmax)))) {
        z = PHUTcollectAffineExprsLocal (minmax, arg_info);
        DBUG_PRINT ("Generating %s < %s for AVIS_MAX", AVIS_NAME (ID_AVIS (arg_node)),
                    AVIS_NAME (ID_AVIS (minmax)));

        z = TCappendExprs (z, PHUTcollectAffineExprsLocal (minmax, arg_info));

        if (MODE_generatematrix == INFO_MODE (arg_info)) {
            z = GenerateZeroExprs (INFO_POLYLIBNUMIDS (arg_info), PLINEQUALITY);
            z = AddValueToColumn (minmax, 1, z, arg_info);    //  max
            z = AddValueToColumn (arg_node, -1, z, arg_info); // -arg_node
            col = 1 + INFO_POLYLIBNUMIDS (arg_info);
            val = (-1) + NUM_VAL (TCgetNthExprsExpr (col, z)); // -1
            z = TCputNthExprs (col, z, TBmakeNum (val));
        }
    }

    CheckExprsChain (z, arg_info);

    DBUG_RETURN (z);
}

#ifdef UNDERCONSTRUCTION
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
#endif // UNDERCONSTRUCTION

/** <!-- ****************************************************************** -->
 * @fn node *PHUTnormalizeAffineExprs( node *exprs)
 *
 * @brief From an N_exprs chain of affine expressions,
 *        generate a normalized N_exprs chain suitable for
 *        input to polylib.
 *
 *        The transformations made are as follows, to bring
 *        into the polylib canonical input format:
 *
 *          AVIS_MIN( z) = mn;   --->    z >= mn
 *                                       ( z - mn) >= 0
 *
 *          AVIS_MAX( z) = mx;   --->    z  <  mx
 *                                        mx > z
 *                                       (mx - z) > 0
 *                                       (mx - z) - 1 >= 0
 *
 *          z = x + y;           --->    z = x + y
 *                                       z - ( x + y) == 0
 *                                       z + ( -x) + ( -y) == 0
 *
 *          z = x - y;           --->    z = x - y
 *                                       z - ( x - y) == 0
 *                                       z + y - x    == 0
 *
 *          z = x * y;           --->    z = x * y
 *          We need x or y to be constant
 *                                       z - ( x * y) == 0
 *
 *          z =   - y;           --->    z = -y
 *                                       z + y == 0
 *
 * @param See above.
 *
 * @return
 *
 *
 ******************************************************************************/
node *
PHUTnormalizeAffineExprs (node *exprs)
{
    node *z = NULL;

    DBUG_ENTER ();

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *collectAffineNid( node *arg_node, info *arg_info)
 *
 * @brief Collect values for AVIS_MIN and AVIS_MAX for this N_id
 *
 * @param An N_id
 *
 * @return An N_exprs chain, or  NULL
 *
 ******************************************************************************/
static node *
collectAffineNid (node *arg_node, info *arg_info)
{
    node *z;

    DBUG_ENTER ();

    z = TCappendExprs (collectAvisMin (arg_node, arg_info),
                       collectAvisMax (arg_node, arg_info));

    CheckExprsChain (z, arg_info);

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn bool isCompatibleAffinePrf( prf nprf)
 *
 * @brief Predicate for affine node finding.
 *
 * @param An N_prf
 *
 * @return TRUE if nprf member fns
 *
 ******************************************************************************/
static bool
isCompatibleAffinePrf (prf nprf)
{
    bool z;

    DBUG_ENTER ();

    switch (nprf) {
    case F_val_lt_val_SxS:
    case F_val_le_val_SxS:
    case F_add_SxS:
    case F_sub_SxS:
    case F_mul_SxS:
    // case F_noteminval:    // probably don't need these two.
    // case F_notemaxval:
    case F_non_neg_val_S:
    case F_min_SxS:
    case F_max_SxS:
    case F_mod_SxS:
    case F_aplmod_SxS:
    case F_abs_S:
    case F_neg_S:
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
 * @fn node *HandleNprf( node *arg_node, info *arg_info)
 *
 * @brief If rhs is an affine function, generate a polylib Matrix row for it,
 *        as follows:
 *
 *         z = x + y;             -->   x +   y  +    (-z) == 0
 *         z = x - y;             -->   x + (-y) +    (-z) == 0
 *         z = x * const          -->   (const * x) + (-z) == 0
 *         z = const * x          -->   (const * x) + (-z) == 0
 *         z =   - y;             -->       (-y)    + (-z) == 0
 *         z = non_neg_val( y)    -->         y            >= 0
 *                                      z + (-y)           == 0
 *
 *
 * @param  arg_node: The N_id for an assign with N_prf as rhs
 *         arg_info: as usual
 *
 * @return An N_exprs chain, representing a polylib Matrix row, or  NULL
 *
 ******************************************************************************/
static node *
HandleNprf (node *arg_node, info *arg_info)
{
    node *z = NULL;
    node *z2;
    node *res = NULL;
    node *right = NULL;
    node *assgn;
    node *rhs;

    DBUG_ENTER ();

    assgn = AVIS_SSAASSIGN (ID_AVIS (arg_node));
    rhs = LET_EXPR (ASSIGN_STMT (assgn));

    if (isCompatibleAffinePrf (PRF_PRF (rhs))) {
        if (MODE_generatematrix == INFO_MODE (arg_info)) {
            z = GenerateZeroExprs (INFO_POLYLIBNUMIDS (arg_info), PLEQUALITY);
            switch (PRF_PRF (rhs)) {
            case F_add_SxS: // z = x + y;    -->   x + y  + (-z) == 0
                z = AddValueToColumn (PRF_ARG1 (rhs), 1, z, arg_info); //  x
                z = AddValueToColumn (PRF_ARG2 (rhs), 1, z, arg_info); //  +y
                z = AddValueToColumn (LET_IDS (ASSIGN_STMT (assgn)), -1, z,
                                      arg_info); // -z
                break;

            case F_non_neg_val_S:
                z = AddValueToColumn (LET_IDS (ASSIGN_STMT (assgn)), 1, z, arg_info); // z
                z = AddValueToColumn (PRF_ARG1 (rhs), -1, z, arg_info); // -y

                z2 = GenerateZeroExprs (INFO_POLYLIBNUMIDS (arg_info), PLINEQUALITY);
                z2 = AddValueToColumn (PRF_ARG1 (rhs), 1, z2, arg_info); //  y >= 0
                z = TCappendExprs (z, z2);
                break;

            case F_sub_SxS: // z = x - y;       -->   x + (-y) +    (-z) == 0
                z = AddValueToColumn (PRF_ARG1 (rhs), 1, z, arg_info);  //    x
                z = AddValueToColumn (PRF_ARG2 (rhs), -1, z, arg_info); //  -y
                z = AddValueToColumn (LET_IDS (ASSIGN_STMT (assgn)), -1, z,
                                      arg_info); // -z
                break;

            case F_val_lt_val_SxS:
            case F_val_le_val_SxS:
            case F_mul_SxS:
            case F_min_SxS:
            case F_max_SxS:
            case F_mod_SxS:
            case F_aplmod_SxS:
            case F_abs_S:
            case F_neg_S:

            default:
                DBUG_ASSERT (FALSE, "Nprf coding time, senor");
                break;
            }
        }
        res = PHUTcollectAffineExprsLocal (PRF_ARG1 (rhs), arg_info);
        if ((F_non_neg_val_S != PRF_PRF (rhs)) && (F_neg_S != PRF_PRF (rhs))) {
            right = PHUTcollectAffineExprsLocal (PRF_ARG2 (rhs), arg_info);
            res = TCappendExprs (res, right);
        }
        z = TCappendExprs (res, z);
    }

    CheckExprsChain (z, arg_info);

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *assignPolylibColumnIndex( node *arg_node, info *arg_info, node *res)
 *
 * @brief Assign the next available polylib column index to arg_node,
 *        if we are in numbering/counting mode,
 *        and this node does not already have a column assigned to it,
 *        and this node is not constant
 *
 * @param arg_node: An N_id
 *        arg_info: your basic arg_info
 *        res: An N_exprs chain of N_id nodes which have had a column number
 *             assigned to them.
 *
 * @return An amended res N_exprs chain, if a column number was assigned,
 *         or res otherwise.
 *
 ******************************************************************************/
static node *
assignPolylibColumnIndex (node *arg_node, info *arg_info, node *res)
{

    DBUG_ENTER ();

    switch (INFO_MODE (arg_info)) {
    case MODE_enumeratevars:

        if ((-1 == AVIS_POLYLIBCOLUMNINDEX (ID_AVIS (arg_node)))
            && (!TYisAKV (ID_NTYPE (arg_node)))) {
            INFO_POLYLIBNUMIDS (arg_info)++;
            AVIS_POLYLIBCOLUMNINDEX (ID_AVIS (arg_node)) = INFO_POLYLIBNUMIDS (arg_info);
            DBUG_PRINT ("%d is polylib index for %s",
                        AVIS_POLYLIBCOLUMNINDEX (ID_AVIS (arg_node)),
                        AVIS_NAME (ID_AVIS (arg_node)));
            res = TCappendExprs (res, TBmakeExprs (DUPdoDupNode (arg_node), NULL));
        }
        break;

    case MODE_clearindices:
        AVIS_POLYLIBCOLUMNINDEX (ID_AVIS (arg_node)) = -1;
        AVIS_ISAFFINEHANDLED (ID_AVIS (arg_node)) = FALSE;
        DBUG_PRINT ("cleared polylib index for %s", AVIS_NAME (ID_AVIS (arg_node)));
        break;

    case MODE_generatematrix:
        break;
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn node *PHUTcollectAffineExprsLocal( node *arg_node, info *arg_info)
 *
 * @brief Does a recursive search, starting at arg_node,
 *        for a maximal chain of affine instructions.
 *
 * @param arg_node: an N_id node or...
 *        arg_info: External callers use NULL; we use
 *                  our own arg_info for traversal marking.
 *
 * @return A maximal N_exprs chain of expressions. E.g.:
 *
 * NB. See PHUTcollectExprs for more details.
 *
 * NB. The code performs three traversals of arg_node and its
 *     ancestors:
 *
 *     MODE_enumeratevars: counts the number of affine variables, and
 *     returns an N_exprs chain of their N_id values.
 *
 *     MODE_generatematrix: This traversal does the actual work,
 *     marking N_avis nodes as it goes, so that we do
 *     not make duplicate entries.
 *
 *     MODE_clearindices:  This restores the N_avis nodes to their
 *     pristine values.
 *
 * FIXME blah blah
 *
 *
 *
 ******************************************************************************/
node *
PHUTcollectAffineExprsLocal (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *assgn = NULL;
    node *rhs = NULL;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_id, "Expected N_id node");
    DBUG_PRINT ("Looking at %s", AVIS_NAME (ID_AVIS (arg_node)));

    res = assignPolylibColumnIndex (arg_node, arg_info, res);
    res = TCappendExprs (res, collectAffineNid (arg_node, arg_info)); // AVIS_MIN/MAX

    assgn = AVIS_SSAASSIGN (ID_AVIS (arg_node));
    if ((NULL != assgn) && (!AVIS_ISAFFINEHANDLED (ID_AVIS (arg_node)))
        && (N_let == NODE_TYPE (ASSIGN_STMT (assgn)))) {

        rhs = LET_EXPR (ASSIGN_STMT (assgn));
        switch (NODE_TYPE (rhs)) {
        case N_id: // straight assign: arg_node = rhs
            res = assignPolylibColumnIndex (rhs, arg_info, res);
            res = TCappendExprs (res, collectAffineNid (rhs, arg_info)); // AVIS_MIN/MAX
            res = TCappendExprs (res,
                                 PHUTcollectAffineExprsLocal (rhs,
                                                              arg_info)); // look further
            break;

        case N_prf:
            res = TCappendExprs (res, HandleNprf (arg_node, arg_info));
            break;

        case N_num:
            break;

        default:
            break;
        }
    }

    if (MODE_enumeratevars == INFO_MODE (arg_info)) {
        AVIS_ISAFFINEHANDLED (ID_AVIS (arg_node)) = FALSE;
    }

    if (MODE_clearindices == INFO_MODE (arg_info)) {
        AVIS_POLYLIBCOLUMNINDEX (ID_AVIS (arg_node)) = -1;
        AVIS_ISAFFINEHANDLED (ID_AVIS (arg_node)) = FALSE;
        res = (NULL != res) ? FREEdoFreeTree (res) : NULL;
        DBUG_PRINT ("cleared polylib index for %s", AVIS_NAME (ID_AVIS (arg_node)));
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn void PHUTclearColumnIndices( node *arg_node, node *fundef)
 *
 * @brief Does a recursive search, starting at arg_node,
 *        for a maximal chain of affine instructions.
 *
 *        As a side effect, AVIS_POLYLIBCOLUMNINDEX is set for each of those
 *        N_id nodes.
 *
 * @param arg_node: an N_id node
 *
 * @return A maximal N_exprs chain of N_id nodes, e.g.,
 *
 *    We have:
 *      flat1 = 1;
 *      x = _sub_SxS_( isaa, flat1);
 *      xp, p = _val_lt_val_SxS_( x, isaa);
 *
 *    If we call PHUTfindAffineNids with arg_node xp, we get back res as:
 *
 *     x, isaa, flat1
 *
 *    The column index values assigned are 1+iota( TCcountExprs( res));
 *
 ******************************************************************************/
void
PHUTclearColumnIndices (node *arg_node, node *fundef)
{
    node *junk = NULL;
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_id, "Expected N_id node");

    arg_info = MakeInfo ();

    INFO_FUNDEF (arg_info) = fundef; // debug only. Not really needed.

    DBUG_PRINT ("Entering MODE_clearindices");
    INFO_MODE (arg_info) = MODE_clearindices;
    junk = PHUTcollectAffineExprsLocal (arg_node, arg_info);
    DBUG_ASSERT (NULL == junk, "Someone did not clean up indices");

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN ();
}

/** <!-- ****************************************************************** -->
 * @fn node *PHUTcollectAffineNids( node *arg_node, node *fundef, int firstindex)
 *
 * @brief Does a recursive search, starting at arg_node,
 *        for a maximal chain of affine instructions.
 *        The result is an N_exprs chain of the unique N_id nodes encountered
 *        in that chain.
 *
 *        As a side effect, AVIS_POLYLIBCOLUMNINDEX is set for each of those
 *        N_id nodes.
 *
 * @param arg_node: an N_id node
 * @param fundef: The N_fundef of the current function. Used only for
 *                debugging purposes, and could be removed.
 * @param firstindex: The next column index to be assigned. This
 *                number is origin-1, as column 0 is Polylib's "function"
 *                ( == or >=).
 *
 *
 * @return A maximal N_exprs chain of N_id nodes, e.g.,
 *
 *    We have:
 *      flat1 = 1;
 *      x = _sub_SxS_( isaa, flat1);
 *      xp, p = _val_lt_val_SxS_( x, isaa);
 *
 *    If we call PHUTfindAffineNids with arg_node xp, we get back res as:
 *
 *     x, isaa, flat1
 *
 *    The column index values assigned are 1+iota( TCcountExprs( res));
 *
 ******************************************************************************/

node *
PHUTcollectAffineNids (node *arg_node, node *fundef, int firstindex)
{
    node *res = NULL;
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_id, "Expected N_id node");

    arg_info = MakeInfo ();

    INFO_FUNDEF (arg_info) = fundef; // debug only. Not really needed.

    INFO_POLYLIBNUMIDS (arg_info) = firstindex;
    INFO_MODE (arg_info) = MODE_enumeratevars;
    DBUG_PRINT ("Entering MODE_enumeratevars");
    res = PHUTcollectAffineExprsLocal (arg_node, arg_info);
    DBUG_PRINT ("Found %d affine variables", TCcountExprs (res));

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn node *PHUTgenerateAffineExprs( node *arg_node, node *fundef)
 *
 * @brief Does a recursive search, starting at arg_node,
 *        for a maximal chain of affine instructions.
 *
 * @param arg_node: an N_id node or...
 * @param fundef: The N_fundef for the current function. Used only for debugging.
 * @param firstindex: 1+the number of distinct N_id nodes in the affine chain.
 *
 * @return A maximal N_exprs chain of expressions. E.g.:
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
PHUTgenerateAffineExprs (node *arg_node, node *fundef, int firstindex)
{
    node *res = NULL;
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_id, "Expected N_id node");

    arg_info = MakeInfo ();
    INFO_FUNDEF (arg_info) = fundef; // debug only. Not really needed.
    INFO_POLYLIBNUMIDS (arg_info) = firstindex;

    INFO_MODE (arg_info) = MODE_generatematrix;
    DBUG_PRINT ("Entering MODE_generatematrix");
    res = PHUTcollectAffineExprsLocal (arg_node, arg_info);

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (res);
}

#ifdef DEADCODE
/** <!-- ****************************************************************** -->
 * @fn node *PHUTgenerateAffinesExprsLocal( node *arg_node, info *arg_info)
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
static node *
PHUTgenerateAffinesExprsLocal (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_id, "Expected N_id node");

    res = PHUTcollectAffineExprsLocal (arg_node, arg_info);

    DBUG_RETURN (res);
}
#endif // DEADCODE

#ifdef UNDERCONSTRUCTION

/** <!--********************************************************************-->
 *
 * @fn node
 *
 * @brief
 *
 *
 *****************************************************************************/
bool
CheckIntersection (IntMatrix constraints, IntMatrix write_fas, IntMatrix read_fas)
{
    bool res;
    FILE *matrix_file, *res_file;
    char buffer[MAXLINE];
    char polyhedral_filename[MAXLINE];
    char result_filename[MAXLINE];

    DBUG_ENTER ();

    count++;
    sprintf (polyhedral_filename, "%s%d.out", outfile, count);
    sprintf (result_filename, "%s%d.out", infile, count);

    matrix_file = FMGRwriteOpen (polyhedral_filename, "w");
    MatrixToFile (constraints, matrix_file);
    MatrixToFile (write_fas, matrix_file);
    MatrixToFile (read_fas, matrix_file);
    FMGRclose (matrix_file);

    SYScall ("$SAC2CBASE/src/tools/cuda/polyhedral < %s > %s\n", polyhedral_filename,
             result_filename);

    res_file = FMGRreadOpen (result_filename);
    res = atoi (fgets (buffer, MAXLINE, res_file)) == 0 ? FALSE : TRUE;
    FMGRclose (res_file);

    // SYScall("rm -f *.out\n");

    DBUG_RETURN (res);
}
#endif // UNDERCONSTRUCTION

#undef DBUG_PREFIX
