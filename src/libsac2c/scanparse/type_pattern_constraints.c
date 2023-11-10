/******************************************************************************
 *
 * This file contains helper methods for generating constraints for type
 * patterns. For example, to select a slice from the shape for the pattern
 * d:shp, we generate an expression "take (d, drop (fdim, shape(v)))".
 *
 * These functions are used by type_pattern_resolve to generate assignments and
 * checks for features of type patterns.
 *
 ******************************************************************************/
#include "convert.h"
#include "free.h"
#include "DupTree.h"
#include "memory.h"
#include "new_types.h"
#include "tree_basic.h"
#include "tree_compound.h"

#define DBUG_PREFIX "TPC"
#include "debug.h"

#include "type_pattern_constraints.h"

/******************************************************************************
 *
 * @fn bool TPCtryAddSpid (node **spids, char *name)
 *
 * @brief Appends a new spid with the given name to the end of the given N_spids
 * chain. Nothing happens if a spid with the given name already exists.
 *
 * @returns Whether a new N_spids node was is_added to the given N_spids chain.
 *
 ******************************************************************************/
bool
TPCtryAddSpid (node **spids, char *name)
{
    bool is_added = FALSE;

    DBUG_ENTER ();

    if (!TCspidsContains (*spids, name)) {
        // The spid is not yet in the chain, add it
        *spids = TCappendSpids (*spids, TBmakeSpids (STRcpy (name), NULL));
        is_added = TRUE;
    }

    DBUG_RETURN (is_added);
}

/******************************************************************************
 *
 * @fn bool TPChasMissing (node *spids, node *defined)
 *
 * @returns Whether the given N_spids chain has missing dependencies.
 *
 ******************************************************************************/
bool
TPChasMissing (node *spids, node *defined)
{
    bool has_missing = FALSE;

    DBUG_ENTER ();

    while (!has_missing && spids != NULL) {
        // Is this spid in the defined set?
        has_missing = !TCspidsContains (defined, SPIDS_NAME (spids));
        spids = SPIDS_NEXT (spids);
    }

    DBUG_RETURN (has_missing);
}

/******************************************************************************
 *
 * @fn void TPCremoveSpid (node **spids, char *name)
 *
 * @brief Removes a single spid with the given name from the given N_spids
 * chain. Nothing happens if no spid with the given name exists.
 *
 * @returns Whether a spid was removed.
 *
 ******************************************************************************/
bool
TPCremoveSpid (node **spids, char *name)
{
    bool is_removed = TRUE;

    DBUG_ENTER ();

    if (*spids != NULL) {
        if (STReq (SPIDS_NAME (*spids), name)) {
            // Found a spid with the given name
            *spids = FREEdoFreeNode (*spids);
            is_removed = TRUE;
        } else {
            // Try the next spid
            is_removed = TPCremoveSpid (&SPIDS_NEXT (*spids), name);
        }
    }

    DBUG_RETURN (is_removed);
}

/******************************************************************************
 *
 * @fn char *TPCmakeFeatureError (node *pattern, char *v, char *fundef,
 *                                bool is_argument)
 *
 * @brief Creates a human-reabable error for type pattern features.
 *
 ******************************************************************************/
char *
TPCmakeFeatureError (node *pattern, char *v, char *fundef, bool is_argument)
{
    char *error;

    DBUG_ENTER ();

    v = is_argument ? v : "return value";
    error = STRcatn (6, "Type pattern error in definition of ", fundef,
                        ": feature `", CVtypePatternShape2String (pattern),
                        "' in ", v);

    DBUG_RETURN (error);
}

/******************************************************************************
 *
 * @fn char *TPCmakeDimError (node *pattern, char *v, char *fundef, int fdim,
 *                            bool is_argument)
 *
 * @brief Creates a human-reabable error for a type pattern's dimensionality.
 *
 ******************************************************************************/
char *
TPCmakeDimError (node *pattern, char *v, char *fundef, int fdim,
                 bool is_argument)
{
    char num_str[20];
    char *error;

    DBUG_ENTER ();

    sprintf (num_str, "%d", fdim);
    v = is_argument ? v : "return value";
    error = STRcatn (7, "Type pattern error in definition of ", fundef,
                        ": dimensionality of ", v, " expected to be ",
                        (TYPEPATTERN_HASVDIM (pattern) ? ">= " : ""),
                        num_str);

    DBUG_RETURN (error);
}

/******************************************************************************
 *
 * @fn node *TPCmakePrimitive (node *pattern, prf built_in, char *user_defined,
 *                             node *expr)
 *
 * @brief If the type pattern's ElementType is a simple type, generate a built-in
 * primitive function for that value, e.g. _dim_A_. Otherwise, generate an
 * application of a user-defined function, e.g. dim. This is necessary to allow
 * users to apply type patterns to user-defined types, by only implementing dim
 * and shape for that user-defined type.
 *
 ******************************************************************************/
node *
TPCmakePrimitive (node *pattern, prf built_in, char *user_defined, node *expr)
{
    node *res;

    DBUG_ENTER ();

    res = TYisSimple (TYPEPATTERN_ELEMENTTYPE (pattern))
            ? TCmakePrf1 (built_in, expr)
            : TCmakeSpap1 (NULL, STRcpy (user_defined), expr);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn node *TPCmakeDimSum (char *v, int fdim, node *vdim)
 *
 * @brief Creates an expression to calculate the current dimensionality index.
 *
 * @returns An expression "<fdim> + sum (vdim)"
 *
 ******************************************************************************/
node *
TPCmakeDimSum (char *v, int fdim, node *vdim)
{
    node *spid, *res;

    DBUG_ENTER ();

    res = TBmakeNum (fdim);

    while (vdim != NULL) {
        spid = TBmakeSpid (NULL, STRcpy (SPIDS_NAME (vdim)));
        res = TCmakePrf2 (F_add_SxS, res, spid);
        vdim = SPIDS_NEXT (vdim);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn node *TPCmakeNumCheck (int num, node *expr)
 *
 * @brief Creates an expression to check whether the given constant matches the
 * length of the given dimension.
 *
 * @returns An expression "<num> == expr"
 *
 ******************************************************************************/
node *
TPCmakeNumCheck (int num, node *expr)
{
    node *res;

    DBUG_ENTER ();

    res = TCmakePrf2 (F_eq_SxS, TBmakeNum (num), expr);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn node *TPCmakeShapeSel (char *v, node *pattern, node *dim)
 *
 * @brief Creates an expression to select a single dimension in the given
 * identifier.
 *
 * @returns An expression "shape(v)[dim]"
 *
 ******************************************************************************/
node *
TPCmakeShapeSel (char *v, node *pattern, node *dim)
{
    node *spid, *res;

    DBUG_ENTER ();

    spid = TBmakeSpid (NULL, STRcpy (v));
    res = TPCmakePrimitive (pattern, F_shape_A, "shape", spid);
    dim = TCmakeIntVector (TBmakeExprs (dim, NULL));
    res = TCmakePrf2 (F_sel_VxA, dim, res);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn node *TPCmakeDimCalc (char *v, node *pattern, char *tdim)
 *
 * @brief Creates an expression to calculate the dimensionality of a given
 * dim:shp pattern.
 *
 * @returns An expression "dim(v) - fdots - sum (vdots\tdim)"
 *
 ******************************************************************************/
node *
TPCmakeDimCalc (char *v, node *pattern, char *tdim)
{
    node *spid, *vdots, *sum, *res;

    DBUG_ENTER ();

    spid = TBmakeSpid (NULL, STRcpy (v));
    res = TPCmakePrimitive (pattern, F_dim_A, "dim", spid);

    if (TYPEPATTERN_FDIM (pattern) > 0) {
        node *num = TBmakeNum (TYPEPATTERN_FDIM (pattern));
        res = TCmakePrf2 (F_sub_SxS, res, num);
    }

    vdots = DUPdoDupTree (TYPEPATTERN_VDIM (pattern));
    TPCremoveSpid (&vdots, tdim);

    if (vdots != NULL) {
        sum = TBmakeSpid (NULL, STRcpy (SPIDS_NAME (vdots)));
        vdots = FREEdoFreeNode (vdots);

        while (vdots != NULL) {
            spid = TBmakeSpid (NULL, STRcpy (SPIDS_NAME (vdots)));
            sum = TCmakePrf2 (F_sub_SxS, sum, spid);
            vdots = FREEdoFreeNode (vdots);
        }

        res = TCmakePrf2 (F_sub_SxS, res, sum);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn node *TPCmakeTakeDrop (char *v, node *pattern, node *nid, node *dim)
 *
 * @brief Creates an expression to select a slice of a shape, with the given
 * offset and length.
 *
 * @returns An expression "take (nid, drop (dim, shape(v)))"
 *
 ******************************************************************************/
node *
TPCmakeTakeDrop (char *v, node *pattern, node *nid, node *dim)
{
    node *spid, *res;

    DBUG_ENTER ();

    spid = TBmakeSpid (NULL, STRcpy (v));
    res = TPCmakePrimitive (pattern, F_shape_A, "shape", spid);

    if (NODE_TYPE (dim) != N_num || NUM_VAL (dim) > 0) {
        res = TCmakePrf2 (F_drop_SxV, dim, res);
    }

    res = TCmakePrf2 (F_take_SxV, DUPdoDupTree (nid), res);

    DBUG_RETURN (res);
}
