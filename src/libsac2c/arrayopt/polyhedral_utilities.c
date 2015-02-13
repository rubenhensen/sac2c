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
#include "polyhedral_utilities.h"
#include "print.h"
#include "system.h"
#include "filemgr.h"
#include "sys/param.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
typedef enum { MODE_enumeratevars, MODE_generatematrix, MODE_clearvars } trav_mode_t;

struct INFO {
    trav_mode_t mode;
    int polylibnumvars;
    node *fundef;
};

/**
 * Macro definitions for INFO structure
 */
#define INFO_MODE(n) ((n)->mode)
#define INFO_POLYLIBNUMVARS(n) ((n)->polylibnumvars)
#define INFO_FUNDEF(n) ((n)->fundef)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));
    INFO_MODE (result) = MODE_clearvars;
    INFO_POLYLIBNUMVARS (result) = 0;
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
 * @fn node *Node2Avis( node *arg_node)
 *
 * @brief Find N_avis node for arg_node
 *
 * @param An N_avis, N_id, or N_ids node
 *
 * @return the associated N_avis node, or NULL if this is an N_num
 *
 ******************************************************************************/
static node *
Node2Avis (node *arg_node)
{
    node *avis = NULL;

    DBUG_ENTER ();

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
        break;

    default:
        DBUG_ASSERT (NULL != avis, "Expected N_id, N_avis, or N_ids node");
    }
    DBUG_RETURN (avis);
}

/** <!-- ****************************************************************** -->
 *
 * @fn void CheckExprsChain( node *exprs, info *arg_info)
 *
 * @brief Ensure that exprs chain consists entirely of N_num nodes,
 *        if we are generating matrix.
 *
 * @param An N_exprs chain
 *
 * @return void
 *
 ******************************************************************************/
static void
CheckExprsChain (node *exprs, info *arg_info)
{
#define PARANOIA
#ifdef PARANOIA
    node *tmp;
    node *el;
#endif // PARANOIA

    DBUG_ENTER ();

#ifdef PARANOIA
    tmp = exprs;
    if (MODE_generatematrix == INFO_MODE (arg_info)) {
        while (tmp != NULL) {
            el = EXPRS_EXPR (tmp);
            //  DBUG_EXECUTE( PRTdoPrintNode( el));
            DBUG_ASSERT ((N_num == NODE_TYPE (el)) || (N_id == NODE_TYPE (el)),
                         "Not N_num or N_id!");
            //  DBUG_PRINT ("\n");
            tmp = EXPRS_NEXT (tmp);
            ;
        }
    }
#endif // PARANOIA
#undef PARANOIA

    DBUG_RETURN ();
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *GenerateMatrixRow( int polylibcols, int relat);
 *
 * @brief Generate a row of a polyhedral matrix.
 *        This is an N_exprs chain of length polylibcols, containing all zeros.
 *        This chain, suitably modified, will comprise one row
 *        of the polylib input matrix.
 *
 *        The chain length is the number of distinct variables
 *        in the computation, plus two: one for the equality/inequality
 *        marker, and one for the constant last element of each matrix row.
 *
 * @param numvars: An integer, specifying the number of
 *                 distinct variables in the matrix.
 *                 The matrix will have two more columns than numvars,
 *                 to account for the polylib function (column 0)
 *                 and the constant column (the ultimate column).
 *
 *       relat: The function specified by this matrix row, either
 *              == 0 or >= 0
 *
 * @return the generated chain, with two extra elements, as noted above
 *         One for the polylib equality/inequality marker, and one for
 *         the constant value.
 *
 ******************************************************************************/
static node *
GenerateMatrixRow (int numvars, int relat)
{
    node *z = NULL;
    int cnt;

    DBUG_ENTER ();

    cnt = numvars + 1;
    DBUG_PRINT ("Generating matrix row of length %d", cnt + 1);

    z = TBmakeExprs (TBmakeNum (relat), NULL);
    while (cnt > 0) {
        z = TCappendExprs (z, TBmakeExprs (TBmakeNum (0), NULL));
        cnt--;
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *AddIntegerToConstantColumn()
 *
 * @brief Perform prow[ constantcol]+ = val, where constantcol
 *        is the last column of the Polylib prow, indicating a constant.
 *
 * @param val: An integer to be added to the constant column of prow.
 * @param prow: An N_exprs chain representing a row of a Polylib constraint.
 * @param arg_info: as usual.
 *
 * @return The updated matrix
 *
 ******************************************************************************/
static node *
AddIntegerToConstantColumn (int val, node *prow, info *arg_info)
{
    node *z = NULL;
    int col;

    DBUG_ENTER ();

    CheckExprsChain (prow, arg_info);

    col = TCcountExprs (prow) - 1;
    val = val + NUM_VAL (TCgetNthExprsExpr (col, prow));
    z = TCputNthExprs (col, prow, TBmakeNum (val));

    CheckExprsChain (z, arg_info);

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *AddValueToColumn()
 *
 * @brief Perform prow[ col]+ = incr, where col is a previously assigned
 *        column index in the arg_node's N_avis. If col is -1, then
 *        the arg_node is a constant, and we add the constant's value,
 *        times incr, to the last column of prow;
 *
 * @param arg_node: An N_id node or an N_ids node or an N_avis node
 *                  or an N_num node
 *        incr: The value to be added for non-constants, or the multiplier for
 *              constants.
 *        prow: An N_exprs chain, the polylib row of interest
 *        arg_info: as usual.
 *
 * @return The updated prow
 *
 ******************************************************************************/
static node *
AddValueToColumn (node *arg_node, int incr, node *prow, info *arg_info)
{
    node *z = NULL;
    int col;
    node *avis = NULL;

    DBUG_ENTER ();

    avis = Node2Avis (arg_node);
    CheckExprsChain (prow, arg_info);

    if (N_num != NODE_TYPE (arg_node)) {
        col = AVIS_POLYLIBCOLUMNINDEX (avis);
        if (-1 == col) {
            DBUG_ASSERT (TYisAKV (AVIS_TYPE (avis)),
                         "Failure to assign column index to non-constant");
            incr = incr * TUtype2Int (AVIS_TYPE (avis));
            col = 1 + INFO_POLYLIBNUMVARS (arg_info); // (last) column, for constants
        }
    } else { // int constant, not flattened
        incr = incr * NUM_VAL (arg_node);
        col = 1 + INFO_POLYLIBNUMVARS (arg_info); // (last) column, for constants
    }

    incr = incr + NUM_VAL (TCgetNthExprsExpr (col, prow));
    z = TCputNthExprs (col, prow, TBmakeNum (incr));

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
 * @param An N_avis
 *
 * @return If in MODE_generatematrix, an N_exprs chain for the above relational.
 *         Otherwise, NULL
 *
 ******************************************************************************/
static node *
collectAvisMin (node *arg_node, info *arg_info)
{
    node *z = NULL;
    node *minmax = NULL;
    node *res = NULL;

    DBUG_ENTER ();

    minmax = AVIS_MIN (arg_node);
    if (NULL != minmax) {
        res = PHUTcollectAffineExprsLocal (minmax, arg_info);
        DBUG_PRINT ("Generating %s >= %s for AVIS_MIN", AVIS_NAME (arg_node),
                    AVIS_NAME (ID_AVIS (minmax)));

        if ((MODE_generatematrix == INFO_MODE (arg_info))
            && (!AVIS_ISAFFINEHANDLED (ID_AVIS (minmax)))) {
            z = GenerateMatrixRow (INFO_POLYLIBNUMVARS (arg_info), PLINEQUALITY);
            z = AddValueToColumn (arg_node, 1, z, arg_info); // arg_node
            z = AddValueToColumn (minmax, -1, z, arg_info);  // -minmax
            z = TCappendExprs (z, res);
        } else {
            z = res;
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
 * @param arg_node: An N_avis or N_id node
 *
 ******************************************************************************/
static node *
collectAvisMax (node *arg_node, info *arg_info)
{
    node *z = NULL;
    node *minmax = NULL;
    node *res = NULL;
    node *avis;
    int col;
    int val;

    DBUG_ENTER ();

    avis = Node2Avis (arg_node);
    minmax = AVIS_MAX (avis);
    if (NULL != minmax) {
        res = PHUTcollectAffineExprsLocal (minmax, arg_info);
        DBUG_PRINT ("Generating %s < %s for AVIS_MAX", AVIS_NAME (avis),
                    AVIS_NAME (ID_AVIS (minmax)));

        if ((MODE_generatematrix == INFO_MODE (arg_info))
            && (!AVIS_ISAFFINEHANDLED (ID_AVIS (minmax)))) {
            z = GenerateMatrixRow (INFO_POLYLIBNUMVARS (arg_info), PLINEQUALITY);
            z = AddValueToColumn (minmax, 1, z, arg_info);    //  max
            z = AddValueToColumn (arg_node, -1, z, arg_info); // -arg_node
            col = 1 + INFO_POLYLIBNUMVARS (arg_info);
            val = (-1) + NUM_VAL (TCgetNthExprsExpr (col, z)); // -1
            z = TCputNthExprs (col, z, TBmakeNum (val));
            z = TCappendExprs (z, res);
        } else {
            z = res;
        }
    }

    CheckExprsChain (z, arg_info);

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 * @fn void Exprs2File( node *exprs, node *idlist)
 *
 * @brief Append one polyhedron to polylib input file, from exprs and idlist
 *
 * @param handle: output file handle
 * @param exprs: N_exprs chain of N_num nodes to be appended
 * @param idlist N_exprs chain of N_id nodes
 *
 * @return void
 *
 ******************************************************************************/
static void
Exprs2File (FILE *handle, node *exprs, node *idlist)
{
    int i;
    int j;
    int rows;
    int cols;
    int indx;
    int val;
    char *id;
    node *avis;

    DBUG_ENTER ();

    cols = TCcountExprs (idlist);
    cols = cols + 2; // polylib function column and constants column
    rows = TCcountExprs (exprs) / cols;
    DBUG_PRINT ("Writing %d rows and %d columns", rows, cols);

    if (0 != rows) {                             // Ignore empty polyhedra
        fprintf (handle, "%d %d\n", rows, cols); // polyhedron descriptor

        for (i = 0; i < rows; i++) {

            // human-readable form
            fprintf (handle, "#  ");
            for (j = 1; j < cols - 1; j++) {
                indx = j + (cols * i);
                val = NUM_VAL (TCgetNthExprsExpr (indx, exprs));
                avis = ID_AVIS (TCgetNthExprsExpr (j - 1, idlist));
                id = AVIS_NAME (avis);
                DBUG_ASSERT (j == AVIS_POLYLIBCOLUMNINDEX (avis),
                             "column index confusion");
                // if( 0 != val) {
                fprintf (handle, "(%d*%s) + ", val, id);
                //}
            }

            // The constant
            val = NUM_VAL (TCgetNthExprsExpr (((cols - 1) + (cols * i)), exprs));
            if (0 != val) {
                fprintf (handle, "(%d) ", val);
            }

            // The relational
            val = NUM_VAL (TCgetNthExprsExpr ((cols * i), exprs));
            fprintf (handle, "%s\n", (0 == val) ? "== 0" : ">= 0");

            // machine-readable form
            for (j = 0; j < cols; j++) {
                indx = j + (cols * i);
                val = NUM_VAL (TCgetNthExprsExpr (indx, exprs));
                fprintf (handle, "%d ", val);
            }
            fprintf (handle, "\n"); // make reading easier
        }
    }

    DBUG_RETURN ();
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *collectAffineNid( node *arg_node, info *arg_info)
 *
 * @brief Collect values for AVIS_MIN and AVIS_MAX for this N_id
 *
 * @param An N_id or an N_avis
 *
 * @return An N_exprs chain, or  NULL
 *
 ******************************************************************************/
static node *
collectAffineNid (node *arg_node, info *arg_info)
{
    node *z;
    node *mn;
    node *mx;
    node *avis;

    DBUG_ENTER ();

    avis = (N_id == NODE_TYPE (arg_node)) ? ID_AVIS (arg_node) : arg_node;

    mn = collectAvisMin (avis, arg_info);
    mx = collectAvisMax (avis, arg_info);
    z = TCappendExprs (mn, mx);

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
    case F_val_lt_val_SxS: // Dyadic
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
 * @fn node *HandleNprf( node *avis, info *arg_info)
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
 * @param  avis: The N_avis for an assign with N_prf as rhs
 *         arg_info: as usual
 *
 * @return An N_exprs chain, representing a polylib Matrix row, or  NULL
 *         All the N_exprs chain values will be N_num nodes, if we
 *         are in MODE_generatematrix mode.
 *
 ******************************************************************************/
static node *
HandleNprf (node *avis, info *arg_info)
{
    node *z = NULL;
    node *z2;
    node *res = NULL;
    node *right = NULL;
    node *assgn;
    node *rhs;
    node *ids;
    int val;

    DBUG_ENTER ();

    assgn = AVIS_SSAASSIGN (avis);
    rhs = LET_EXPR (ASSIGN_STMT (assgn));
    ids = LET_IDS (ASSIGN_STMT (assgn));

    if (isCompatibleAffinePrf (PRF_PRF (rhs))) {
        if (MODE_generatematrix == INFO_MODE (arg_info)) {
            // Start by grabbing any extrema information attached to the main result.
            // We should compute this properly, rather than relying on IVEXP.
            // For now, we just use the extrema, if present.
            if (NULL != AVIS_MIN (IDS_AVIS (ids))) { // z            >= AVIS_MIN
                                                     // z - AVIS_MIN >= 0
                z = GenerateMatrixRow (INFO_POLYLIBNUMVARS (arg_info), PLINEQUALITY);
                z = AddValueToColumn (ids, 1, z, arg_info); // z
                // -AVIS_MIN
                z = AddValueToColumn (AVIS_MIN (IDS_AVIS (ids)), -1, z, arg_info);
            }

            if (NULL != AVIS_MAX (IDS_AVIS (ids))) { // z < AVIS_MAX
                                                     // AVIS_MAX > z
                                                     // AVIS_MAX - z > 0
                                                     // AVIS_MAX + ( -z) + (-1) >= 0
                z2 = GenerateMatrixRow (INFO_POLYLIBNUMVARS (arg_info), PLINEQUALITY);
                // AVIS_MAX
                z2 = AddValueToColumn (AVIS_MAX (IDS_AVIS (ids)), 1, z2, arg_info);
                z2 = AddValueToColumn (ids, -1, z2, arg_info);      // -z
                z2 = AddIntegerToConstantColumn (-1, z2, arg_info); // - 1
                z = TCappendExprs (z2, z);
            }

            switch (PRF_PRF (rhs)) {
            case F_add_SxS: // z = x + y;    -->   x + y  + (-z) == 0
                z2 = GenerateMatrixRow (INFO_POLYLIBNUMVARS (arg_info), PLEQUALITY);
                z2 = AddValueToColumn (PRF_ARG1 (rhs), 1, z2, arg_info); //  x
                z2 = AddValueToColumn (PRF_ARG2 (rhs), 1, z2, arg_info); //  +y
                z2 = AddValueToColumn (ids, -1, z2, arg_info);           // -z
                z = TCappendExprs (z2, z);
                break;

            case F_non_neg_val_S: // z = nonneg( y) -->  (z == y) && ( z >= 0)
                z2 = GenerateMatrixRow (INFO_POLYLIBNUMVARS (arg_info), PLEQUALITY);
                z2 = AddValueToColumn (ids, 1, z2, arg_info);             // z
                z2 = AddValueToColumn (PRF_ARG1 (rhs), -1, z2, arg_info); // -y
                z = TCappendExprs (z2, z);

                z2 = GenerateMatrixRow (INFO_POLYLIBNUMVARS (arg_info), PLINEQUALITY);
                z2 = AddValueToColumn (ids, 1, z2, arg_info); //  z >= 0
                z = TCappendExprs (z2, z);
                break;

            case F_sub_SxS: // z = x - y;       -->   x + (-y) +    (-z) == 0
                z2 = GenerateMatrixRow (INFO_POLYLIBNUMVARS (arg_info), PLEQUALITY);
                z2 = AddValueToColumn (PRF_ARG1 (rhs), 1, z2, arg_info);  //   x
                z2 = AddValueToColumn (PRF_ARG2 (rhs), -1, z2, arg_info); //  -y
                z2 = AddValueToColumn (ids, -1, z2, arg_info);            //  -z
                z = TCappendExprs (z2, z);
                break;

            case F_max_SxS: // z = max( x, y)
                            //  -->  ( z >= x)        && ( z >= y)
                            //  -->  (( z - x) >= 0)  && ( z - y) >= 0
                z2 = GenerateMatrixRow (INFO_POLYLIBNUMVARS (arg_info), PLINEQUALITY);
                z2 = AddValueToColumn (ids, 1, z2, arg_info);             // z
                z2 = AddValueToColumn (PRF_ARG1 (rhs), -1, z2, arg_info); //  -x
                z = TCappendExprs (z2, z);

                z2 = GenerateMatrixRow (INFO_POLYLIBNUMVARS (arg_info), PLINEQUALITY);
                z2 = AddValueToColumn (ids, 1, z2, arg_info);             // z
                z2 = AddValueToColumn (PRF_ARG2 (rhs), -1, z2, arg_info); //  -y
                z = TCappendExprs (z2, z);
                break;

            case F_min_SxS: // z = min( x, y)
                            //  -->  ( z <= x)        && ( z <= y)
                            //  -->  ( x >= z)        && ( y >= z)
                            //  -->  (( x - z) >= 0)  && ( y - z) >= 0
                z2 = GenerateMatrixRow (INFO_POLYLIBNUMVARS (arg_info), PLINEQUALITY);
                z2 = AddValueToColumn (PRF_ARG1 (rhs), 1, z2, arg_info); //  x
                z2 = AddValueToColumn (ids, -1, z2, arg_info);           // -z
                z = TCappendExprs (z2, z);

                z2 = GenerateMatrixRow (INFO_POLYLIBNUMVARS (arg_info), PLINEQUALITY);
                z2 = AddValueToColumn (PRF_ARG2 (rhs), 1, z2, arg_info); //  y
                z2 = AddValueToColumn (ids, -1, z2, arg_info);           // -z
                z = TCappendExprs (z2, z);
                break;

            case F_mul_SxS: // We need one constant argument
                avis = ID_AVIS (PRF_ARG1 (rhs));
                if (TYisAKV (AVIS_TYPE (avis))) {
                    // z = ( const * y)  -->  (const * y) + (-z) == 0
                    val = TUtype2Int (AVIS_TYPE (avis));
                    z2 = GenerateMatrixRow (INFO_POLYLIBNUMVARS (arg_info), PLEQUALITY);
                    z2 = AddValueToColumn (PRF_ARG2 (rhs), val, z2, arg_info); //  const*y
                    z2 = AddValueToColumn (ids, -1, z2, arg_info);             // -z
                    z = TCappendExprs (z2, z);
                    break;
                }

                avis = ID_AVIS (PRF_ARG2 (rhs));
                if (TYisAKV (AVIS_TYPE (avis))) {
                    // z = ( x * const)  -->  ( x * const) + (-z) == 0
                    val = TUtype2Int (AVIS_TYPE (avis));
                    z2 = GenerateMatrixRow (INFO_POLYLIBNUMVARS (arg_info), PLEQUALITY);
                    z2 = AddValueToColumn (PRF_ARG1 (rhs), val, z2, arg_info); //  const*x
                    z2 = AddValueToColumn (ids, -1, z2, arg_info);             // -z
                    z = TCappendExprs (z2, z);
                    break;
                }
                break;

            case F_abs_S:
            case F_mod_SxS:
            case F_aplmod_SxS:
                // We got all we could from the extrema
                break;

            case F_val_lt_val_SxS:
            case F_val_le_val_SxS:
                // This is a guard within an affine expression
                break;

            case F_neg_S: //  z = -y;  -->   (-y) + (-z) == 0
                z2 = GenerateMatrixRow (INFO_POLYLIBNUMVARS (arg_info), PLEQUALITY);
                z2 = AddValueToColumn (PRF_ARG1 (rhs), -1, z2, arg_info); // -y
                z2 = AddValueToColumn (ids, -1, z2, arg_info);            // -z
                z = TCappendExprs (z2, z);
                break;

            default:
                DBUG_UNREACHABLE ("Nprf coding time, senor");
                break;
            }
        }

        // Deal with PRF_ARGs
        res = PHUTcollectAffineExprsLocal (PRF_ARG1 (rhs), arg_info);
        if (isDyadicPrf (PRF_PRF (rhs))) {
            right = PHUTcollectAffineExprsLocal (PRF_ARG2 (rhs), arg_info);
            res = TCappendExprs (res, right);
        }
        z = TCappendExprs (z, res);
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
    node *avis;

    DBUG_ENTER ();

    avis = Node2Avis (arg_node);
    switch (INFO_MODE (arg_info)) {
    case MODE_enumeratevars:

        if ((-1 == AVIS_POLYLIBCOLUMNINDEX (avis)) && (!TYisAKV (AVIS_TYPE (avis)))) {
            INFO_POLYLIBNUMVARS (arg_info)++;
            AVIS_POLYLIBCOLUMNINDEX (avis) = INFO_POLYLIBNUMVARS (arg_info);
            DBUG_PRINT ("Assigned %d as polylib index for %s",
                        AVIS_POLYLIBCOLUMNINDEX (avis), AVIS_NAME (avis));
            res = TCappendExprs (res, TBmakeExprs (TBmakeId (avis), NULL));
        }
        break;

    case MODE_clearvars:
        AVIS_POLYLIBCOLUMNINDEX (avis) = -1;
        AVIS_ISAFFINEHANDLED (avis) = FALSE;
        DBUG_PRINT ("cleared polylib index for %s", AVIS_NAME (avis));
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
 * @param arg_node: an N_id node or an N_avis node or an N_ids node
 *        arg_info: External callers use NULL; we use
 *                  our own arg_info for traversal marking.
 *
 * @return A maximal N_exprs chain of expressions, whose elements are
 *         N_id or N_num nodes.
 *         See PHUTcollectExprs for more details.
 *
 ******************************************************************************/
node *
PHUTcollectAffineExprsLocal (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *assgn = NULL;
    node *rhs = NULL;
    node *avis = NULL;

    DBUG_ENTER ();

    avis = Node2Avis (arg_node);
    DBUG_PRINT ("Looking at %s", AVIS_NAME (avis));

    if (N_num != NODE_TYPE (arg_node)) {
        switch (INFO_MODE (arg_info)) {
        case MODE_enumeratevars:
            break;

        case MODE_clearvars:
            AVIS_POLYLIBCOLUMNINDEX (avis) = -1;
            AVIS_ISAFFINEHANDLED (avis) = FALSE;
            res = (NULL != res) ? FREEdoFreeTree (res) : NULL;
            DBUG_PRINT ("cleared polylib index for %s", AVIS_NAME (avis));
            break;

        case MODE_generatematrix:
            AVIS_ISAFFINEHANDLED (avis) = TRUE;
            break;
        }

        // Handle RHS for all modes.
        assgn = AVIS_SSAASSIGN (avis);
        if ((NULL != assgn) && (N_let == NODE_TYPE (ASSIGN_STMT (assgn)))) {

            rhs = LET_EXPR (ASSIGN_STMT (assgn));
            switch (NODE_TYPE (rhs)) {
            case N_id: // straight assign: arg_node = rhs
                res = assignPolylibColumnIndex (rhs, arg_info, res);
                res = TCappendExprs (res, collectAffineNid (rhs, arg_info));
                res = TCappendExprs (res, PHUTcollectAffineExprsLocal (rhs, arg_info));
                break;

            case N_prf:
                res = TCappendExprs (res, HandleNprf (avis, arg_info));
                break;

            case N_num:
                if (MODE_generatematrix == INFO_MODE (arg_info)) {
                    res = GenerateMatrixRow (INFO_POLYLIBNUMVARS (arg_info), PLEQUALITY);
                    res = AddIntegerToConstantColumn (NUM_VAL (rhs), res, arg_info);
                }
                break;

            default:
                break;
            }
        }

        // Handle extrema
        res = TCappendExprs (res, collectAffineNid (arg_node, arg_info));

        // Handle arg_node
        res = assignPolylibColumnIndex (arg_node, arg_info, res);
    }

    // Discard result if we do not need it.
    if (MODE_clearvars == INFO_MODE (arg_info) && (NULL != res)) {
        res = FREEdoFreeTree (res);
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
 * @param arg_node: an N_id node, an N_avis node, or an N_num node
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

    DBUG_ASSERT (((N_id == NODE_TYPE (arg_node)) || (N_num == NODE_TYPE (arg_node))
                  || (N_avis == NODE_TYPE (arg_node))),
                 "Expected N_id or N_avis node");

    if (N_num != NODE_TYPE (arg_node)) {
        arg_info = MakeInfo ();
        INFO_FUNDEF (arg_info) = fundef; // debug only. Not really needed.

        DBUG_PRINT ("Entering MODE_clearvars");
        INFO_MODE (arg_info) = MODE_clearvars;
        junk = PHUTcollectAffineExprsLocal (arg_node, arg_info);
        DBUG_ASSERT (NULL == junk, "Someone did not clean up indices");
        arg_info = FreeInfo (arg_info);
    }

    DBUG_RETURN ();
}

/** <!-- ****************************************************************** -->
 * @fn node *PHUTcollectAffineNids( node *arg_node, node *fundef,
 *                                  int firstindex)
 *
 * @brief Does a recursive search, starting at arg_node,
 *        for a maximal chain of affine instructions.
 *        The result is an N_exprs chain of the unique N_id nodes encountered
 *        in that chain, catenated to exprs.
 *
 *        As a side effect, AVIS_POLYLIBCOLUMNINDEX is set for each of those
 *        N_id nodes.
 *
 * @param arg_node: an N_id node
 * @param fundef: The N_fundef of the current function. Used only for
 *                debugging purposes, and could be removed.
 * @param numvars: The next column index to be assigned. This
 *                 number is origin-one, as column 0 assigned to
 *                 Polylib's "function" ( == or >= ).
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
 *   Also, as a side effect, we set numvars.
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
 *     MODE_clearvars:  This restores the N_avis nodes to their
 *     pristine values.
 *
 ******************************************************************************/

node *
PHUTcollectAffineNids (node *arg_node, node *fundef, int *numvars)
{
    node *res = NULL;
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (((N_id == NODE_TYPE (arg_node)) || (N_num == NODE_TYPE (arg_node))
                  || (N_avis == NODE_TYPE (arg_node))),
                 "Expected N_id or N_avis node");

    arg_info = MakeInfo ();

    INFO_FUNDEF (arg_info) = fundef; // debug only. Not really needed.

    INFO_POLYLIBNUMVARS (arg_info) = *numvars;
    INFO_MODE (arg_info) = MODE_enumeratevars;
    DBUG_PRINT ("Entering MODE_enumeratevars with numids=%d", *numvars);
    res = PHUTcollectAffineExprsLocal (arg_node, arg_info);
    DBUG_PRINT ("Found %d affine variables", TCcountExprs (res));
    *numvars = INFO_POLYLIBNUMVARS (arg_info);
    arg_info = FreeInfo (arg_info);

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
PHUTgenerateAffineExprs (node *arg_node, node *fundef, int *numvars)
{
    node *res = NULL;
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();
    INFO_FUNDEF (arg_info) = fundef; // debug only. Not really needed.
    INFO_POLYLIBNUMVARS (arg_info) = *numvars;

    INFO_MODE (arg_info) = MODE_generatematrix;
    DBUG_PRINT ("Entering MODE_generatematrix with numvars=%d", *numvars);
    res = PHUTcollectAffineExprsLocal (arg_node, arg_info);

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn node *PHUTgenerateAffineExprsForGuard( node *arg_node, node *fundef)
 *
 * @brief Construct a Polylib matrix for an N_prf guard.
 *
 *        We build this as an inverted relational, so that
 *        the intersect of the guard polyhedron and the polyhedron of its
 *        arguments will be empty, if the guard is valid.
 *        There might be a more sensible way to do this, but I think
 *        this will work.
 *
 * @param arg_node: an N_prf for a guard primitive
 * @param fundef: The N_fundef for the current function.
 *                Used only for debugging.
 * @param numvars: the number of distinct variables in the affine function tree.
 *
 * @return A maximal N_exprs chain of expressions for the guard.
 *
 ******************************************************************************/
node *
PHUTgenerateAffineExprsForGuard (node *arg_node, node *fundef, int *numvars)
{
    node *z = NULL;
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_prf, "Expected N_prf node");

    arg_info = MakeInfo ();
    INFO_FUNDEF (arg_info) = fundef; // debug only. Not really needed.
    INFO_POLYLIBNUMVARS (arg_info) = *numvars;

    INFO_MODE (arg_info) = MODE_generatematrix;
    DBUG_PRINT ("Entering MODE_generatematrix with numids=%d", *numvars);

    z = GenerateMatrixRow (INFO_POLYLIBNUMVARS (arg_info), PLINEQUALITY);

    switch (PRF_PRF (arg_node)) {

    case F_val_lt_val_SxS:
        // z = val_lt_val_SxS( x, y) -->   x             <  y
        // Inverse:                        x             >= y
        //                                 ( x - y)      >= 0
        z = AddValueToColumn (PRF_ARG1 (arg_node), 1, z, arg_info);
        z = AddValueToColumn (PRF_ARG2 (arg_node), -1, z, arg_info);
        break;

    case F_val_le_val_SxS:
        // z = val_le_val_SxS( x, y) -->   x             <= y
        // Inverse:                        x             >  y
        //                                 ( x - y)      >  0
        //                                 ( x - y) - 1  >= 0
        z = AddValueToColumn (PRF_ARG1 (arg_node), 1, z, arg_info);
        z = AddValueToColumn (PRF_ARG2 (arg_node), -1, z, arg_info);
        z = AddIntegerToConstantColumn (-1, z, arg_info);
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
 * @brief Compute the intersection between two Polylib polyhedra,
 *        represented by exprs1 and exprs2.
 *
 *        If the intersect is NULL, return TRUE; else FALSE.
 *
 *        The monotonically increasing global.polylib_filenumber
 *        is for debugging, as we will generate MANY files.
 *
 *****************************************************************************/
bool
PHUTcheckIntersection (node *exprs1, node *exprs2, node *idlist)
{
#define MAXLINE 100
    bool res;
    int polyres;
    FILE *matrix_file;
    FILE *res_file;
    char polyhedral_arg_filename[PATH_MAX];
    char polyhedral_res_filename[PATH_MAX];
    static const char *argfile = "polyhedral_args";
    static const char *resfile = "polyhedral_res";
    char buffer[MAXLINE];
    int exit_code;

    DBUG_ENTER ();

#ifndef DBUG_OFF
    global.polylib_filenumber++;
#endif // DBUG_OFF

    sprintf (polyhedral_arg_filename, "%s/%s%d.arg", global.tmp_dirname, argfile,
             global.polylib_filenumber);
    sprintf (polyhedral_res_filename, "%s/%s%d.res", global.tmp_dirname, resfile,
             global.polylib_filenumber);

    DBUG_PRINT ("Polylib arg filename: %s", polyhedral_arg_filename);
    DBUG_PRINT ("Polylib res filename: %s", polyhedral_res_filename);
    matrix_file = FMGRwriteOpen (polyhedral_arg_filename, "w");

    Exprs2File (matrix_file, exprs1, idlist);
    Exprs2File (matrix_file, exprs2, idlist);

    FMGRclose (matrix_file);

    // We depend on PATH to find the sacpolylibintersect binary
    DBUG_PRINT ("calling sacpolylibisnullintersect");
    exit_code = SYScallNoErr ("sacpolylibisnullintersect < %s > %s\n",
                              polyhedral_arg_filename, polyhedral_res_filename);
    DBUG_PRINT ("exit_code=%d, WIFEXITED=%d, WIFSIGNALED=%d, WEXITSTATUS=%d", exit_code,
                WIFEXITED (exit_code), WIFSIGNALED (exit_code), WEXITSTATUS (exit_code));
    res_file = FMGRreadOpen (polyhedral_res_filename);
    polyres = atoi (fgets (buffer, MAXLINE, res_file));
    DBUG_PRINT ("intersection result is %d", polyres);
    res = (1 == polyres) ? TRUE : FALSE;
    FMGRclose (res_file);

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn
 *
 * @brief
 *
 * @param
 *
 * @return void
 *
 ******************************************************************************/
#ifdef UNDERCONSTRUCTION
node *PHUTsomething( node *
{
    PHUTclearColumnIndices (PRF_ARG1 (arg_node), INFO_FUNDEF (arg_info));
    PHUTclearColumnIndices (PRF_ARG2 (arg_node), INFO_FUNDEF (arg_info));

    idlist1
      = PHUTcollectAffineNids (PRF_ARG1 (arg_node), INFO_FUNDEF (arg_info), &numvars);

    idlist2
      = PHUTcollectAffineNids (PRF_ARG2 (arg_node), INFO_FUNDEF (arg_info), &numvars);

    exprs1
      = PHUTgenerateAffineExprs (PRF_ARG1 (arg_node), INFO_FUNDEF (arg_info), &numvars);
    exprs2
      = PHUTgenerateAffineExprs (PRF_ARG2 (arg_node), INFO_FUNDEF (arg_info), &numvars);
    exprs3 = PHUTgenerateAffineExprsForGuard (arg_node, INFO_FUNDEF (arg_info), &numvars);

    idlist1 = TCappendExprs (idlist1, idlist2);
    exprs1 = TCappendExprs (exprs1, exprs2);

    // Don't bother calling Polylib if it can't do anything for us.
    z = (NULL != exprs1) && (NULL != exprs3) && (NULL != idlist1);
    z = z && PHUTcheckIntersection (exprs1, exprs3, idlist1);

    PHUTclearColumnIndices (PRF_ARG1 (arg_node), INFO_FUNDEF (arg_info));
    PHUTclearColumnIndices (PRF_ARG2 (arg_node), INFO_FUNDEF (arg_info));

    idlist1 = (NULL != idlist1) ? FREEdoFreeTree (idlist1) : NULL;
    exprs1 = (NULL != exprs1) ? FREEdoFreeTree (exprs1) : NULL;
    exprs3 = (NULL != exprs3) ? FREEdoFreeTree (exprs3) : NULL;
    DBUG_RETURN (res);
}
#endif // UNDERCONSTRUCTION

#undef DBUG_PREFIX
