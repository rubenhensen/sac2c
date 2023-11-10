/******************************************************************************
 *
 * This traversal converts type patterns to pre-existing types. We do so to
 * avoid having to modify the type checker to work well with type patterns.
 *
 * For example, the following function signature:
 *
 * int[3,.] foo (int[7,5] a, int[n,m] b, int[.,d:shp] c)
 *
 * is converted to:
 *
 * int[.,.] foo (int[7,5] a, int[.,.] b, int[+] c)
 *
 * We convert type patterns to one of four following cases:
 *   - Array of unknown dimensionality:  int[*]
 *   - Array of non-zero dimensionality: int[+]
 *   - Array of known dimensionality:    int[.,.,.]
 *   - Array of known shape:             int[5,3,7]
 *
 * We do this for each type pattern, using the function AnalyseTP. This function
 * describes the formal rules that we apply to decide how to convert a type
 * pattern to a pre-existing type.
 *
 * Additionally, we store the number of fixed dimensions and the variable
 * dimension identifiers in the N_typepattern, as these will be required in the
 * type_pattern_resolve traversal.
 *
 ******************************************************************************/
#include "convert.h"
#include "ctinfo.h"
#include "free.h"
#include "memory.h"
#include "new_types.h"
#include "print.h"
#include "shape.h"
#include "str.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"

#define DBUG_PREFIX "ATP"
#include "debug.h"

#include "type_pattern_analyse.h"

struct INFO {
    bool has_type_pattern;
};

#define INFO_HASTYPEPATTERN(n) ((n)->has_type_pattern)

static info *
MakeInfo (void)
{
    info *res;

    DBUG_ENTER ();

    res = (info *)MEMmalloc (sizeof (info));

    INFO_HASTYPEPATTERN (res) = FALSE;

    DBUG_RETURN (res);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * @fn void RenameIgnorePattern (node *spid)
 *
 * @brief Type patterns allow for underscores (_) in type patterns, which
 * denote that that feature is ignored. Note that this means that if a type
 * patterns contains two _, this does not mean that they should be the same.
 * Thus, we replace _ by a unique name.
 *
 ******************************************************************************/
static void
RenameIgnorePattern (node *spid)
{
    DBUG_ENTER ();

    if (STReq (SPID_NAME (spid), "_")) {
        SPID_NAME (spid) = STRcpy (TRAVtmpVar ());
        DBUG_PRINT ("renamed `_' to `%s'", SPID_NAME (spid));
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * @fn ntype *DecideResultType (node *pattern, shape *shp)
 *
 * @brief After running the type pattern analysis, we must decide what the
 * resulting shape of the argument is. We do this using the following
 * decision tree:
 *
 * if there are variable dimensions:
 *   fdim == 0: AUD
 *   otherwise: AUDGZ
 * otherwise:
 *   fdim > fshp: AKD[fdim]
 *   otherwise:   AKS[fshp]
 *
 ******************************************************************************/
static ntype *
DecideResultType (node *pattern, shape *shp)
{
    ntype *res;

    DBUG_ENTER ();

    res = TYcopyType (TYPEPATTERN_ELEMENTTYPE (pattern));

    if (TYPEPATTERN_HASVDIM (pattern)) {
        if (TYPEPATTERN_FDIM (pattern) == 0) {
            res = TYmakeAUD (res);
            shp = SHfreeShape (shp);
        } else {
            res = TYmakeAUDGZ (res);
            shp = SHfreeShape (shp);
        }
    } else {
        if (TYPEPATTERN_FDIM (pattern) > TYPEPATTERN_FSHP (pattern)) {
            shp = SHfreeShape (shp);
            shp = SHmakeShape (0);
            res = TYmakeAKD (res, (size_t)TYPEPATTERN_FDIM (pattern), shp);
        } else {
            res = TYmakeAKS (res, shp);
        }
    }

    DBUG_EXECUTE ({ char *tmp = TYtype2String (res, FALSE, 0);
                    DBUG_PRINT ("resulting in %s", tmp);
                    tmp = MEMfree (tmp); });

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn ntype *AnalyseTP (node *pattern, info *arg_info)
 *
 * @brief Analyses the given N_typepattern and constructs a suitable ntype for
 * it. It solely relies on TYPEPATTERN_SHAPE and TYPEPATTERN_ELEMENTTYPE for
 * doing so. While doing so, we annotate:
 *
 * FShp: the number of dimensions with fixed shape value found
 * FDim: the number of fixed dimensions found
 * VDim: an N_spids chain of variable dimensions
 *
 * We also set INFO_HASTYPEPATTERN to indicate whether a type pattern was used.
 *
 * Abstractly speeking we implement the following algorithm:
 *
 * 1) ATP (type[<num>, …rest] v, fshp, fdim, vdim)
 *     = ATP (type[rest] v, fshp+1, fdim+1, vdim)
 *
 * 2) ATP (type[., …rest] v, fshp, fdim, vdim)
 *     = ATP (type[rest] v, fshp, fdim+1, vdim)
 *
 * 3) ATP (type[+, …rest] v, fshp, fdim, vdim)
 *     = ATP (type[rest] v, fshp, fdim+1, vdim+{*})
 *
 * 4) ATP (type[*, …rest] v, fshp, fdim, vdim)
 *     = ATP (type[rest] v, fshp, fdim, vdim+{*})
 *
 * 5) ATP (type[id, …rest] v, fshp, fdim, vdim)
 *     = ATP (type[rest] v, fshp, fdim+1, vdim)
 *
 * 6) ATP (type[<num>:shp, …rest] v, fshp, fdim, vdim)
 *     = ATP (type[rest] v, fshp, fdim+n, vdim)
 *
 * 7) ATP (type[id:shp, …rest] v, fshp, fdim, vdim)
 *     = ATP (type[rest] v, fshp, fdim, vdim+{id})
 *
 ******************************************************************************/
static ntype *
AnalyseTP (node *pattern, info *arg_info)
{
    ntype *res;
    shape *shp;
    node *exprs, *expr;
    bool contains_num = FALSE;
    bool contains_dot = FALSE;

    DBUG_ENTER ();

    DBUG_EXECUTE ({ char *tmp = CVtypePattern2String (pattern);
                    DBUG_PRINT ("analysing pattern %s", tmp);
                    tmp = MEMfree (tmp); });

    TYPEPATTERN_FSHP (pattern) = 0;
    TYPEPATTERN_FDIM (pattern) = 0;
    TYPEPATTERN_VDIM (pattern) = NULL;

    exprs = TYPEPATTERN_SHAPE (pattern);
    shp = SHmakeShape ((int)TCcountExprs (exprs));

    while (exprs != NULL) {
        expr = EXPRS_EXPR (exprs);

        DBUG_ASSERT (NODE_TYPE (expr) == N_num ||
                     NODE_TYPE (expr) == N_dot ||
                     NODE_TYPE (expr) == N_spid,
                     "expected N_num, N_dot, or N_spid node in type pattern");

        // Case 1
        if (NODE_TYPE (expr) == N_num) {
            SHsetExtent (shp, TYPEPATTERN_FSHP (pattern), NUM_VAL (expr));
            TYPEPATTERN_FSHP (pattern) += 1;
            TYPEPATTERN_FDIM (pattern) += 1;
            contains_num = TRUE;
        }
        // Case 2
        else if (NODE_TYPE (expr) == N_dot) {
            TYPEPATTERN_FDIM (pattern) += 1;
            contains_dot = TRUE;
        }
        // Case 3
        else if (STReq (SPID_NAME (expr), "+")) {
            TYPEPATTERN_HASVDIM (pattern) = TRUE;
            TYPEPATTERN_FDIM (pattern) += 1;
        }
        // Case 4
        else if (STReq (SPID_NAME (expr), "*")) {
            TYPEPATTERN_HASVDIM (pattern) = TRUE;
        }
        // Case 5
        else if (SPID_TDIM (expr) == NULL) {
            TYPEPATTERN_FDIM (pattern) += 1;
            INFO_HASTYPEPATTERN (arg_info) = TRUE;
        }
        // Case 6
        else if (NODE_TYPE (SPID_TDIM (expr)) == N_num) {
            RenameIgnorePattern (expr);
            TYPEPATTERN_FDIM (pattern) += NUM_VAL (SPID_TDIM (expr));
            INFO_HASTYPEPATTERN (arg_info) = TRUE;
        }
        // Case 7
        else {
            node *tdim = SPID_TDIM (expr);
            RenameIgnorePattern (expr);
            RenameIgnorePattern (tdim);
            TYPEPATTERN_VDIM (pattern) =
                TCappendSpids (TYPEPATTERN_VDIM (pattern),
                               TBmakeSpids (STRcpy (SPID_NAME (tdim)), NULL));
            INFO_HASTYPEPATTERN (arg_info) = TRUE;
            TYPEPATTERN_HASVDIM (pattern) = TRUE;
        }

        exprs = EXPRS_NEXT (exprs);
    }

    INFO_HASTYPEPATTERN (arg_info) |= contains_num && contains_dot;
    INFO_HASTYPEPATTERN (arg_info) |= contains_num && TYPEPATTERN_HASVDIM (pattern);
    TYPEPATTERN_ISTYPEPATTERN (pattern) = INFO_HASTYPEPATTERN (arg_info);

    DBUG_EXECUTE ({ char *tmp = CVspids2String (TYPEPATTERN_VDIM (pattern));
                    DBUG_PRINT ("fshp: %d, fdim: %d, vdim: %s",
                                TYPEPATTERN_FSHP (pattern),
                                TYPEPATTERN_FDIM (pattern),
                                tmp);
                    tmp = MEMfree (tmp); });

    res = DecideResultType (pattern, shp);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn void WarnNoSupport (char *text, node *arg_node, info *arg_info)
 *
 * @brief Warns if any of the pattern features have been detected in the latest
 * call to AnalysisTP. This info is conveyed through the arg_info.
 *
 ******************************************************************************/
static void
WarnNoSupport (char *node_name, node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_HASTYPEPATTERN (arg_info)) {
    }
        CTIwarn (NODE_LOCATION (arg_node),
                 "Type patterns are currently not supported for %s. "
                 "Ignoring all type variables.",
                 node_name);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * @fn node *ATPdoAnalyseTypePattern (node *arg_node)
 *
 * @brief Hook to start the handle dots traversal of the AST.
 *
 * @result Transformed AST with type patterns converted to pre-existing types.
 *
 ******************************************************************************/
node *
ATPdoAnalyseTypePattern (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_module,
                 "called with non-module node");

    arg_info = MakeInfo ();

    TRAVpush (TR_atp);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    CTIabortOnError ();

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *ATPfundef (node *arg_node, info *arg_info)
 *
 * @brief Converts the type patterns of arguments and return types in this
 * function to pre-existing types. We traverse the body of this function to
 * check whether it contains any type patterns, in which case we warn the
 * user that this is not supported.
 *
 ******************************************************************************/
node *
ATPfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("----- analysing type patterns of %s -----",
                FUNDEF_NAME (arg_node));

    /**
     * Traverse the body first so that we can give a warning to the user if they
     * applied type patterns to an unsupported type.
     */
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
    FUNDEF_RETS (arg_node) = TRAVopt (FUNDEF_RETS (arg_node), arg_info);

    if (FUNDEF_ISSPECIALISATION (arg_node)) {
        // For now, we simply ignore any pattern
        WarnNoSupport ("function specialisations", arg_node, arg_info);
    }

    /**
     * We want to give an unsupported warning if any of the arguments or returns
     * contains a type pattern, thus we only reset HASTYPEPATTERN to false after
     * traversing all arguments and returns.
     */
    INFO_HASTYPEPATTERN (arg_info) = FALSE;

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *ATPtypedef (node *arg_node, info *arg_info)
 *
 * @brief Convert type patterns of typedefs to pre-existing types to avoid
 * compilation errors, and give a warning that type patterns are not supported
 * for typedefs.
 *
 ******************************************************************************/
node *
ATPtypedef (node *arg_node, info *arg_info)
{
    node *pattern;

    DBUG_ENTER ();

    pattern = TYPEDEF_TYPEPATTERN (arg_node);
    if (pattern != NULL) {
        DBUG_ASSERT (TYPEDEF_NTYPE (arg_node) == NULL,
                     "N_typedef with both NTYPE and TYPEPATTERN found");

        TYPEDEF_NTYPE (arg_node) = AnalyseTP (pattern, arg_info);

        // For now, we simply ignore any pattern
        TYPEDEF_TYPEPATTERN (arg_node) = FREEdoFreeTree (pattern);
        WarnNoSupport ("type definitions", arg_node, arg_info);
        INFO_HASTYPEPATTERN (arg_info) = FALSE;
    } else {
        DBUG_ASSERT (TYPEDEF_NTYPE (arg_node) != NULL,
                     "N_typedef with neither NTYPE nor TYPEPATTERN found");
    }

    TYPEDEF_NEXT (arg_node) = TRAVopt (TYPEDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *ATPstructelem (node *arg_node, info *arg_info)
 *
 * @brief Convert type patterns of structelems to pre-existing types to avoid
 * compilation errors, and give a warning that type patterns are not supported
 * for structelems.
 *
 ******************************************************************************/
node *
ATPstructelem (node *arg_node, info *arg_info)
{
    node *pattern;

    DBUG_ENTER ();

    pattern = STRUCTELEM_TYPEPATTERN (arg_node);
    STRUCTELEM_TYPE (arg_node) = AnalyseTP (pattern, arg_info);

    // For now, we simply ignore any pattern
    STRUCTELEM_TYPEPATTERN (arg_node) = FREEdoFreeTree (pattern);
    WarnNoSupport ("struct component types", arg_node, arg_info);
    INFO_HASTYPEPATTERN (arg_info) = FALSE;

    STRUCTELEM_NEXT (arg_node) = TRAVopt (STRUCTELEM_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *ATPobjdef (node *arg_node, info *arg_info)
 *
 * @brief Convert type patterns of objdefs to pre-existing types to avoid
 * compilation errors, and give a warning that type patterns are not supported
 * for objdefs.
 *
 ******************************************************************************/
node *
ATPobjdef (node *arg_node, info *arg_info)
{
    node *pattern;

    DBUG_ENTER ();

    pattern = OBJDEF_TYPEPATTERN (arg_node);
    OBJDEF_TYPE (arg_node) = AnalyseTP (pattern, arg_info);

    // For now, we simply ignore any pattern
    OBJDEF_TYPEPATTERN (arg_node) = FREEdoFreeTree (pattern);
    WarnNoSupport ("global object definitions", arg_node, arg_info);
    INFO_HASTYPEPATTERN (arg_info) = FALSE;

    OBJDEF_NEXT (arg_node) = TRAVopt (OBJDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *ATParray (node *arg_node, info *arg_info)
 *
 * @brief Convert type patterns of arrays to pre-existing types to avoid
 * compilation errors, and give a warning that type patterns are not supported
 * for arrays.
 *
 ******************************************************************************/
node *
ATParray (node *arg_node, info *arg_info)
{
    node *pattern;

    DBUG_ENTER ();

    pattern = ARRAY_TYPEPATTERN (arg_node);
    if (pattern != NULL) {
        ARRAY_ELEMTYPE (arg_node) = AnalyseTP (pattern, arg_info);

        // For now, we simply ignore any pattern
        ARRAY_TYPEPATTERN (arg_node) = FREEdoFreeTree (pattern);
        WarnNoSupport ("empty arrays", arg_node, arg_info);
        INFO_HASTYPEPATTERN (arg_info) = FALSE;
    }

    ARRAY_AELEMS (arg_node) = TRAVopt (ARRAY_AELEMS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *ATPcast (node *arg_node, info *arg_info)
 *
 * @brief Convert type patterns of casts to pre-existing types to avoid
 * compilation errors, and give a warning that type patterns are not supported
 * for casts.
 *
 ******************************************************************************/
node *
ATPcast (node *arg_node, info *arg_info)
{
    node *pattern;

    DBUG_ENTER ();

    pattern = CAST_TYPEPATTERN (arg_node);
    CAST_NTYPE (arg_node) = AnalyseTP (pattern, arg_info);

    // For now, we simply ignore any pattern
    CAST_TYPEPATTERN (arg_node) = FREEdoFreeTree (pattern);
    WarnNoSupport ("type casts", arg_node, arg_info);
    INFO_HASTYPEPATTERN (arg_info) = FALSE;

    CAST_EXPR (arg_node) = TRAVdo (CAST_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *ATPobjdef (node *arg_node, info *arg_info)
 *
 * @brief Convert type patterns of rets to pre-existing types.
 *
 ******************************************************************************/
node *
ATPret (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    RET_TYPE (arg_node) = AnalyseTP (RET_TYPEPATTERN (arg_node), arg_info);

    RET_NEXT (arg_node) = TRAVopt (RET_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *ATPobjdef (node *arg_node, info *arg_info)
 *
 * @brief Convert type patterns of avises to pre-existing types.
 *
 ******************************************************************************/
node *
ATPavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    AVIS_TYPE (arg_node) = AnalyseTP (AVIS_TYPEPATTERN (arg_node), arg_info);
    AVIS_DECLTYPE (arg_node) = TYcopyType (AVIS_TYPE (arg_node));

    DBUG_RETURN (arg_node);
}
