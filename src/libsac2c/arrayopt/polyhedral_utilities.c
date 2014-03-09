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
            DBUG_ASSERT (N_num == NODE_TYPE (el), "Not N_num!");
            //  DBUG_PRINT ("\n");
            tmp = EXPRS_NEXT (tmp);
            ;
        }
    }
#endif // PARANOIA

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
 * @fn node *AddIntegerToConstantColumn()
 *
 * @brief Perform matrix[ constantcol]+ = val, where constantcol
 *        is the last column of the Polylib row, indicating a constant.
 *
 * @param val: An integer to be added to the constant column of matrix.
 * @param matrix: An N_exprs chain representing a row of a Polylib constraint.
 * @param arg_info: as usual.
 *
 * @return The updated matrix
 *
 ******************************************************************************/
static node *
AddIntegerToConstantColumn (int val, node *matrix, info *arg_info)
{
    node *z = NULL;
    int col;

    DBUG_ENTER ();

    CheckExprsChain (matrix, arg_info);

    col = 1 + INFO_POLYLIBNUMIDS (arg_info); // The (last) column, for constants
    val = val + NUM_VAL (TCgetNthExprsExpr (col, matrix));
    z = TCputNthExprs (col, matrix, TBmakeNum (val));

    CheckExprsChain (z, arg_info);

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *AddValueToColumn()
 *
 * @brief Perform matrix[ col]+ = incr, where col is a previously assigned
 *        column index in the arg_node's N_avis. If col is -1, then
 *        the arg_node is a constant, and we add the constant's value,
 *        times incr, to the last column of matrix;
 *
 * @param arg_node: An N_id node or an N_ids node
 *        incr: The value to be added for non-constants, or the multiplier for
 *              constants.
 *        matrix: The N_exprs chain row of interest
 *        arg_info: as usual.
 *
 * @return The updated matrix
 *
 ******************************************************************************/
static node *
AddValueToColumn (node *arg_node, int incr, node *matrix, info *arg_info)
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
        incr = incr * TUtype2Int (AVIS_TYPE (avis));
        col = 1 + INFO_POLYLIBNUMIDS (arg_info); // The (last) column, for constants
    }

    incr = incr + NUM_VAL (TCgetNthExprsExpr (col, matrix));
    z = TCputNthExprs (col, matrix, TBmakeNum (incr));

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

/** <!-- ****************************************************************** -->
 * @fn void ( node *exprs, node *idlist)
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
                if (0 != val) {
                    fprintf (handle, "(%d*%s) + ", val, id);
                }
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
        // case F_val_lt_val_SxS: Perhaps we can follow PRF_ARG1?
        // case F_val_le_val_SxS:
    case F_add_SxS:
    case F_sub_SxS:
    case F_mul_SxS:
    // case F_noteminval:    // probably don't need these two.
    // case F_notemaxval:
    case F_non_neg_val_S:
    case F_min_SxS:
    case F_max_SxS:
    case F_mod_SxS:
    // case F_aplmod_SxS: // probably wrong
    // case F_abs_S:
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
    node *ids;
    node *avis;
    int val;

    DBUG_ENTER ();

    assgn = AVIS_SSAASSIGN (ID_AVIS (arg_node));
    rhs = LET_EXPR (ASSIGN_STMT (assgn));
    ids = LET_IDS (ASSIGN_STMT (assgn));

    if (isCompatibleAffinePrf (PRF_PRF (rhs))) {
        if (MODE_generatematrix == INFO_MODE (arg_info)) {
            switch (PRF_PRF (rhs)) {
            case F_add_SxS: // z = x + y;    -->   x + y  + (-z) == 0
                z = GenerateZeroExprs (INFO_POLYLIBNUMIDS (arg_info), PLEQUALITY);
                z = AddValueToColumn (PRF_ARG1 (rhs), 1, z, arg_info); //  x
                z = AddValueToColumn (PRF_ARG2 (rhs), 1, z, arg_info); //  +y
                z = AddValueToColumn (ids, -1, z, arg_info);           // -z
                break;

            case F_non_neg_val_S: // z = nonneg( y) -->  (z == y) && ( z >= 0)
                z = GenerateZeroExprs (INFO_POLYLIBNUMIDS (arg_info), PLEQUALITY);
                z = AddValueToColumn (ids, 1, z, arg_info);             // z
                z = AddValueToColumn (PRF_ARG1 (rhs), -1, z, arg_info); // -y

                z2 = GenerateZeroExprs (INFO_POLYLIBNUMIDS (arg_info), PLINEQUALITY);
                z2 = AddValueToColumn (ids, 1, z2, arg_info); //  z >= 0
                z = TCappendExprs (z, z2);
                break;

            case F_sub_SxS: // z = x - y;       -->   x + (-y) +    (-z) == 0
                z = GenerateZeroExprs (INFO_POLYLIBNUMIDS (arg_info), PLEQUALITY);
                z = AddValueToColumn (PRF_ARG1 (rhs), 1, z, arg_info);  //   x
                z = AddValueToColumn (PRF_ARG2 (rhs), -1, z, arg_info); //  -y
                z = AddValueToColumn (ids, -1, z, arg_info);            //  -z
                break;

            case F_max_SxS: // z = max( x, y)
                            //  -->  ( z >= x)        && ( z >= y)
                            //  -->  (( z - x) >= 0)  && ( z - y) >= 0
                z = GenerateZeroExprs (INFO_POLYLIBNUMIDS (arg_info), PLINEQUALITY);
                z = AddValueToColumn (ids, 1, z, arg_info);             // z
                z = AddValueToColumn (PRF_ARG1 (rhs), -1, z, arg_info); //  -x

                z2 = GenerateZeroExprs (INFO_POLYLIBNUMIDS (arg_info), PLINEQUALITY);
                z = AddValueToColumn (ids, 1, z, arg_info);             // z
                z = AddValueToColumn (PRF_ARG2 (rhs), -1, z, arg_info); //  -y
                z = TCappendExprs (z, z2);
                break;

            case F_min_SxS: // z = min( x, y)
                            //  -->  ( z <= x)        && ( z <= y)
                            //  -->  ( x >= z)        && ( y >= z)
                            //  -->  (( x - z) >= 0)  && ( y - z) >= 0
                z = GenerateZeroExprs (INFO_POLYLIBNUMIDS (arg_info), PLINEQUALITY);
                z = AddValueToColumn (PRF_ARG1 (rhs), 1, z, arg_info); //  x
                z = AddValueToColumn (ids, -1, z, arg_info);           // -z

                z2 = GenerateZeroExprs (INFO_POLYLIBNUMIDS (arg_info), PLINEQUALITY);
                z = AddValueToColumn (PRF_ARG2 (rhs), 1, z, arg_info); //  y
                z = AddValueToColumn (ids, -1, z, arg_info);           // -z
                z = TCappendExprs (z, z2);
                break;

            case F_mul_SxS: // We need one constant argument
                avis = ID_AVIS (PRF_ARG1 (rhs));
                if (TYisAKV (AVIS_TYPE (avis))) {
                    // z = ( const * y)  -->  (const * y) + (-z) == 0
                    val = TUtype2Int (AVIS_TYPE (avis));
                    z = GenerateZeroExprs (INFO_POLYLIBNUMIDS (arg_info), PLEQUALITY);
                    z = AddValueToColumn (PRF_ARG2 (rhs), val, z, arg_info); //  const*y
                    z = AddValueToColumn (ids, -1, z, arg_info);             // -z
                    break;
                }

                avis = ID_AVIS (PRF_ARG2 (rhs));
                if (TYisAKV (AVIS_TYPE (avis))) {
                    // z = ( x * const)  -->  ( x * const) + (-z) == 0
                    val = TUtype2Int (AVIS_TYPE (avis));
                    z = GenerateZeroExprs (INFO_POLYLIBNUMIDS (arg_info), PLEQUALITY);
                    z = AddValueToColumn (PRF_ARG1 (rhs), val, z, arg_info); //  const*x
                    z = AddValueToColumn (ids, -1, z, arg_info);             // -z
                    break;
                }
                break;

            // case F_val_lt_val_SxS: Perhaps we can follow PRF_ARG1?
            // case F_val_le_val_SxS:
            case F_mod_SxS:
            // case F_aplmod_SxS: // probably wrong
            // case F_abs_S:
            case F_neg_S:

            default:
                DBUG_ASSERT (FALSE, "Nprf coding time, senor");
                break;
            }
        }

        // Deal with PRF_ARGs
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
    node *avis;

    DBUG_ENTER ();

    avis = ID_AVIS (arg_node);
    switch (INFO_MODE (arg_info)) {
    case MODE_enumeratevars:

        if ((-1 == AVIS_POLYLIBCOLUMNINDEX (avis)) && (!TYisAKV (ID_NTYPE (arg_node)))) {
            INFO_POLYLIBNUMIDS (arg_info)++;
            AVIS_POLYLIBCOLUMNINDEX (avis) = INFO_POLYLIBNUMIDS (arg_info);
            DBUG_PRINT ("Assigned %d as polylib index for %s",
                        AVIS_POLYLIBCOLUMNINDEX (avis), AVIS_NAME (avis));
            AVIS_ISAFFINEHANDLED (avis) = TRUE;
            res = TCappendExprs (res, TBmakeExprs (DUPdoDupNode (arg_node), NULL));
        }
        break;

    case MODE_clearindices:
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

    // Handle RHS
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

    // Handle extrema
    res = TCappendExprs (res, collectAffineNid (arg_node, arg_info)); // AVIS_MIN/MAX

    // Handle arg_node
    res = assignPolylibColumnIndex (arg_node, arg_info, res);

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

/** <!-- ****************************************************************** -->
 * @fn node *PHUTgenerateAffineExprsForGuard( node *arg_node, node *fundef)
 *
 * @brief Construct a Polylib matrix for an N_prf guard.
 *        We build this as an inverted relational, so that
 *        the intersect of the guard polyhedron and the polyhedron of its
 *        arguments will be NULL if the guard is valid.
 *
 *        There might be a more sensible way to do this, but I think
 *        this will work.
 *
 * @param arg_node: an N_prf for a guard primitive
 * @param fundef: The N_fundef for the current function.
 *                Used only for debugging.
 * @param firstindex: 1+the number of distinct N_id nodes in the affine chain.
 *
 * @return A maximal N_exprs chain of expressions for the guard.
 *
 ******************************************************************************/
node *
PHUTgenerateAffineExprsForGuard (node *arg_node, node *fundef, int firstindex)
{
    node *z = NULL;
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_prf, "Expected N_prf node");

    arg_info = MakeInfo ();
    INFO_FUNDEF (arg_info) = fundef; // debug only. Not really needed.
    INFO_POLYLIBNUMIDS (arg_info) = firstindex;

    INFO_MODE (arg_info) = MODE_generatematrix;
    DBUG_PRINT ("Entering MODE_generatematrix");

    z = GenerateZeroExprs (INFO_POLYLIBNUMIDS (arg_info), PLINEQUALITY);

    switch (PRF_PRF (arg_node)) {

    case F_val_lt_val_SxS:
        // z = val_lt_val_SxS( x, y) -->   ( x - y) >= 0
        z = AddValueToColumn (PRF_ARG1 (arg_node), 1, z, arg_info);
        z = AddValueToColumn (PRF_ARG2 (arg_node), -1, z, arg_info);
        break;

    case F_val_le_val_SxS:
        // z = val_le_val_SxS( x, y) -->   ( x - y) > 0
        //                             ( x - y) - 1 >= 0
        //
        z = AddValueToColumn (PRF_ARG1 (arg_node), 1, z, arg_info);  //  x
        z = AddValueToColumn (PRF_ARG2 (arg_node), -1, z, arg_info); // -y
        z = AddIntegerToConstantColumn (-1, z, arg_info);            // - 1
        break;

    default:
        DBUG_ASSERT (FALSE, "Coding time for guard polyhedron");
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

    global.polylib_filenumber++;
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

    //  SYScall("$SAC2CBASE/src/tools/cuda/polyhedral < %s > %s\n",
    // Here, we depend on PATH to find the binary.
    exit_code = SYScallNoErr ("sacpolylibintersect < %s > %s\n", polyhedral_arg_filename,
                              polyhedral_res_filename);
    DBUG_PRINT ("exit_code=%d, WIFEXITED=%d, WIFSIGNALED=%d, WEXITSTATUS=%d", exit_code,
                WIFEXITED (exit_code), WIFSIGNALED (exit_code), WEXITSTATUS (exit_code));
    res_file = FMGRreadOpen (polyhedral_res_filename);
    polyres = atoi (fgets (buffer, MAXLINE, res_file));
    DBUG_PRINT ("intersection result is %d", polyres);
    res = (0 == polyres) ? FALSE : TRUE;
    FMGRclose (res_file);

    DBUG_RETURN (res);
}

#undef DBUG_PREFIX
