/*
 *
 * $Id$
 *
 */

/** <!--********************************************************************-->
 *
 * @defgroup sci Shape Clique Inference
 *
 *  This file implements inference of shape cliques.
 *  Shape cliques are sets of variables that are known to have the same
 *  shape, even if we don't know their exact shape at compile time.
 *  Information from shape clique inference is presently used for index
 *  vector elimination (IVE), and may be used for other optimizations, such as
 *  array memory reuse.
 *
 *  When SCI completes, all AST N_avis nodes will have their AVIS_SHAPECLIQUEID
 *  fields updated, as follows: each AVIS_SHAPECLIQUEID is an element of a
 *  circular linked-list of N_avis nodes whose associated arrays are known to
 *  have the same shape, even if we don't know their exact shape.
 *
 * @ingroup sci
 *
 * @{
 *
 **************************************************************************/

#include "shape_cliques.h"

#include "tree_basic.h"
#include "dbug.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "traverse.h"
#include "shape.h"
#include "new_types.h"
#include "type_utils.h"
#include "ctinfo.h"
#include "index_infer.h"

/*
 * OPEN PROBLEMS:
 *
 * I) not yet solved:
 *
 * II) to be fixed here:
 *  1. Support shape cliques for genarray in withloops.
 *
 * III) to be fixed somewhere else:
 *  1. How to make shape cliques for dyadic scalar functions, e.g.:
 *         C = _add_AxA_( A,B).
 *     We probably want to place a guard before
 *     this operation, asserting that A and B have matching shapes.
 *     Once that is done, we can place (A,B,C) in the same shape clique.
 *
 *  2. We want to support shape cliques for mod, max, min, and, or, eq, neq,
 *     le, lt, and perhaps a few others. The right way to do this is to
 *     create new primitives for those functions, e.g.:
 *        _mod_SxS_, _mod_SxA_, _mod_AxS, _mod_AxA_
 *     and then treat them the same as _add_AxS_, etc.
 *     Stephan says this is a several days work spread all over the compiler,
 *     so I'm deferring it.
 */

/**
 *
 * @name Some utility functions:
 *
 * @{
 */

/*
 *  * INFO structure
 *   */
struct INFO {
    node *lhs;
    node *fundef;
};

/*
 *  * INFO macros
 *   */
#define INFO_LHS(n) ((n)->lhs)
#define INFO_FUNDEF(n) ((n)->fundef)

/*
 *  * INFO functions
 *   */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_LHS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/** <!--*******************************************************************-->
 *
 * @fn void PrintShapeCliqueNames( node *avis, printall)
 * @brief This function is a debugging function, to print the names of
 * all entries in one shape clique.
 * The printall flag allows suppression of multiple printing of the same
 * clique from different starting nodes.
 *
 * ****************************************************************************/
static void
PrintShapeCliqueNames (node *avis, bool printall)
{
    node *curavis;

    DBUG_ENTER ("PrintShapeCliqueNames");
    curavis = avis;
    if (AVIS_SHAPECLIQUEID (avis) == avis) {
        DBUG_PRINT ("SCI", ("%s is degenerate shape clique", AVIS_NAME (avis)));
    } else {
        DBUG_PRINT ("SCI", ("Members of shape clique: %s are:", AVIS_NAME (avis)));
        do {
            if (printall | !AVIS_ISSCIPRINTED (curavis)) {
                DBUG_PRINT ("SCI", ("   %s", AVIS_NAME (curavis)));
            }
            AVIS_ISSCIPRINTED (curavis) = TRUE & !printall;
            curavis = AVIS_SHAPECLIQUEID (curavis);
        } while (avis != curavis);
    }
    DBUG_VOID_RETURN;
}

/** <!--*******************************************************************-->
 *
 * @fn node *AppendAvisToShapeClique( node *avis1, node *avis2, info *arg_info)
 * @brief This function appends avis1 to the shape clique of avis2.
 *        The result is the canonical shape clique id for avis2.
 *
 *****************************************************************************/
static node *
AppendAvisToShapeClique (node *avis1, node *avis2)
{
    node *nextavis1, *curavis2;

    DBUG_ENTER ("AppendAvisToShapeClique");

    /* If avises are already in same shape clique, this function is a no-op */
    if (!SCIAvisesAreInSameShapeClique (avis1, avis2)) {

        DBUG_PRINT ("SCI_APPEND", ("Adding id: %s to shape clique: %s", AVIS_NAME (avis1),
                                   AVIS_NAME (avis2)));

        nextavis1 = AVIS_SHAPECLIQUEID (avis1); /* Point avis1 at avis2 */
        AVIS_SHAPECLIQUEID (avis1) = avis2;

        /* Find avis2 that points at avis2, and point it at avis1 */
        curavis2 = avis2;

        while (AVIS_SHAPECLIQUEID (curavis2) != avis2) {
            curavis2 = AVIS_SHAPECLIQUEID (curavis2);
        }

        AVIS_SHAPECLIQUEID (curavis2) = nextavis1;

        DBUG_PRINT ("SCI_APPEND", ("Resulting shape clique is:"));
        PrintShapeCliqueNames (avis2, TRUE);
    }
    DBUG_RETURN (avis2);
}

/** <!--*******************************************************************-->
 *
 * @fn void *ResetAllShapeCliques( node *arg_node, node *arg_info))
 * @brief This function resets the shape clique ID entries in all N_avis nodes
 *        for this function.
 *        This is required for shape clique inference and to keep the
 *          ast validation thingy happy.
 *          Resetting consists of making each N_avis node a one-element
 *          (degenerate) shape clique.
 *
 *****************************************************************************/
static void
ResetAllShapeCliques (node *arg_node, info *arg_info)
{
    node *arg, *argavis;

    DBUG_ENTER ("ResetAllShapeCliques");
    /* reset all arguments */
    arg = FUNDEF_ARGS (arg_node);
    while (arg != NULL) {
        argavis = ARG_AVIS (arg);
        AVIS_SHAPECLIQUEID (argavis) = SHAPECLIQUEIDNONE (argavis);
        AVIS_ISSCIPRINTED (argavis) = FALSE;
        AVIS_ISVECT2OFFSETISSUED (argavis) = FALSE;
        arg = ARG_NEXT (arg);
    }

    /* reset all locals */
    arg = FUNDEF_BODY (arg_node); /* maybe N_block */
    if (NULL != arg) {
        arg = BLOCK_VARDEC (arg);
        while (arg != NULL) {
            argavis = VARDEC_AVIS (arg);
            AVIS_SHAPECLIQUEID (argavis) = SHAPECLIQUEIDNONE (argavis);
            AVIS_ISSCIPRINTED (argavis) = FALSE;
            AVIS_ISVECT2OFFSETISSUED (argavis) = FALSE;
            arg = VARDEC_NEXT (arg);
        }
    }

    DBUG_VOID_RETURN;
}

/** <!--*******************************************************************-->
 *
 * @fn void *ResetIsSCIPrinted( node *arg_node)
 * @brief This function resets the shape clique was-printed in all N_avis nodes
 *    *
 *****************************************************************************/
static void
ResetIsSCIPrinted (node *arg_node)
{
    node *arg, *argavis;

    DBUG_ENTER ("ResetIsSCIPrinted");

    /* reset all arguments */
    arg = FUNDEF_ARGS (arg_node);
    while (arg != NULL) {
        argavis = ARG_AVIS (arg);
        AVIS_ISSCIPRINTED (argavis) = FALSE;
        arg = ARG_NEXT (arg);
    }

    /* reset all locals */
    arg = FUNDEF_BODY (arg_node); /* maybe N_block */
    if (NULL != arg) {
        arg = BLOCK_VARDEC (arg);
        while (arg != NULL) {
            argavis = VARDEC_AVIS (arg);
            AVIS_ISSCIPRINTED (argavis) = FALSE;
            arg = VARDEC_NEXT (arg);
        }
    }

    DBUG_VOID_RETURN;
}

/** <!--*******************************************************************-->
 *
 * @fn void ShapeCliquePrintIDs(  node *arg_node)
 * @brief This function is a debugging function, which is intended to print
 *        all names and ShapeCliqueIDs for this function.
 *
 * ****************************************************************************/
static void
ShapeCliquePrintIDs (node *arg_node)
{
    node *arg, *argavis;

    DBUG_ENTER ("ShapeCliquePrintIDs");
    ResetIsSCIPrinted (arg_node);

    /* print all arguments */
    arg = FUNDEF_ARGS (arg_node);
    while (arg != NULL) {
        argavis = ARG_AVIS (arg);
        if (!AVIS_ISSCIPRINTED (argavis)) {
            PrintShapeCliqueNames (argavis, FALSE);
        }
        AVIS_ISSCIPRINTED (argavis) = TRUE;
        arg = ARG_NEXT (arg);
    }

    /* print all locals */
    arg = FUNDEF_BODY (arg_node); /* maybe N_block */
    if (NULL != arg) {
        arg = BLOCK_VARDEC (arg);
        while (arg != NULL) {
            argavis = VARDEC_AVIS (arg);
            if (!AVIS_ISSCIPRINTED (argavis))
                PrintShapeCliqueNames (argavis, FALSE);
            AVIS_ISSCIPRINTED (argavis) = TRUE;
            arg = VARDEC_NEXT (arg);
        }
    }

    ResetIsSCIPrinted (arg_node);
    DBUG_VOID_RETURN;
}

/** <!--*******************************************************************-->
 *
 * @fn bool SCIAvisesAreInSameShapeClique( node *avis1, node *avis2)
 * @brief Predicate for determining if two avis nodes are in same
 *          shape clique. The main reason for having a function to do this
 *          today is that I am contemplating a very different representation
 *          for shape clique membership.
 *
 *****************************************************************************/
bool
SCIAvisesAreInSameShapeClique (node *avis1, node *avis2)
{
    node *curavis2;
    bool z;

    z = FALSE;
    curavis2 = avis2;
    do {
        if (avis1 == curavis2) {
            z = TRUE;
            break;
        }
        curavis2 = AVIS_SHAPECLIQUEID (curavis2);
    } while (avis2 != curavis2);

    return (z);
}

/** <!--*******************************************************************-->
 *
 * @fn node *SCIFindMarkedAvisInSameShapeClique( node *avis1)
 *
 * @brief Search elements of avis1's shape clique until we find
 * one marked with IsVect2OffsetIssued. If we find one,
 * we return its N_avis. Otherwise, we just return avis1.
 * The returned N_avis is always marked.
 *
 * ****************************************************************************/
node *
SCIFindMarkedAvisInSameShapeClique (node *avis1)
{
    node *curavis;

    curavis = avis1;
    do {
        if (AVIS_ISVECT2OFFSETISSUED (curavis)) {
            break;
        }
        curavis = AVIS_SHAPECLIQUEID (curavis);
    } while (avis1 != curavis);

    AVIS_ISVECT2OFFSETISSUED (curavis) = TRUE;
    return (curavis);
}

/** <!--*******************************************************************-->
 *
 * @fn void BuildAKSShapeCliqueOne( node *arg_node, node *avis)
 * @brief Build a shape cliques for all AKS arrays with same shape as
 * AKS array avis1.
 *
 *****************************************************************************/
static void
BuildAKSShapeCliqueOne (node *arg_node, node *avis1)
{
    node *arg, *argavis;
    ntype *typearg, *typeargavis;

    DBUG_ENTER ("BuildAKSShapeCliqueOne");

    /* handle function arguments */
    arg = FUNDEF_ARGS (arg_node);
    typearg = AVIS_TYPE (avis1);
    while (arg != NULL) {
        argavis = ARG_AVIS (arg);
        typeargavis = AVIS_TYPE (argavis);
        if (TUshapeKnown (typeargavis) && (argavis != avis1)
            && TUeqShapes (typeargavis, typearg)) {
            AppendAvisToShapeClique (argavis, avis1);
        }
        arg = ARG_NEXT (arg);
    }

    /* Now handle function locals */
    arg = FUNDEF_BODY (arg_node); /* maybe N_block */
    typearg = AVIS_TYPE (avis1);
    if (arg != NULL) {
        arg = BLOCK_VARDEC (arg);
        while (arg != NULL) {
            argavis = VARDEC_AVIS (arg);
            typeargavis = AVIS_TYPE (argavis);
            if (TUshapeKnown (typeargavis) && (argavis != avis1)
                && TUeqShapes (typeargavis, typearg)) {
                AppendAvisToShapeClique (argavis, avis1);
            }
            arg = VARDEC_NEXT (arg);
        }
    }

    DBUG_VOID_RETURN;
}

/** <!--*******************************************************************-->
 *
 * @fn void BuildAKSShapeCliques( node *arg_node)
 * @brief Build shape cliques for all AKS arrays.
 *
 * ****************************************************************************/
static void
BuildAKSShapeCliques (node *arg_node)
{
    node *arg, *argavis;
    ntype *type1;

    DBUG_ENTER ("BuildAKSShapeCliques");
    /* handle function arguments */
    arg = FUNDEF_ARGS (arg_node);
    while (arg != NULL) {
        argavis = ARG_AVIS (arg);
        type1 = AVIS_TYPE (argavis);
        if (TUshapeKnown (type1)) {
            BuildAKSShapeCliqueOne (arg_node, argavis);
        }
        arg = ARG_NEXT (arg);
    }
    /* Now handle function locals */
    arg = FUNDEF_BODY (arg_node); /* maybe N_block */
    if (arg != NULL) {
        arg = BLOCK_VARDEC (arg);
        while (arg != NULL) {
            argavis = VARDEC_AVIS (arg);
            type1 = AVIS_TYPE (argavis);
            if (TUshapeKnown (type1)) {
                BuildAKSShapeCliqueOne (arg_node, argavis);
            }
            arg = VARDEC_NEXT (arg);
        }
    }
    DBUG_VOID_RETURN;
}

/** <!--*******************************************************************-->
 *
 * @fn avis ShapeCliqueMin(node *avis)
 * @brief Utility function to provide smallest address of any
 *        N_avis in this shape clique. This serves only as
 *        a convenient method for getting a unique identifier for
 *        each shape clique.
 *
 * ****************************************************************************/
node *
ShapeCliqueMin (node *avis)
{
    node *minavis;
    node *curavis;

    DBUG_ENTER ("ShapeCliqueNumber");
    minavis = avis;
    curavis = avis;
    do {
        if (curavis < minavis)
            minavis = curavis;
        curavis = AVIS_SHAPECLIQUEID (curavis);
    } while (avis != curavis);
    DBUG_RETURN (minavis);
}

/** <!--*******************************************************************-->
 *
 * @fn int ShapeCliqueLookup(node *scavis, node *arg_node)
 * @brief Utility function to turn an N_avis address into
 *        a small integer.
 *        This performs: (funargs, vardev) iota scavis
 *
 * ****************************************************************************/
int
ShapeCliqueLookup (node *scavis, node *arg_node)
{

    node *arg;
    int res;
    bool fini;

    DBUG_ENTER ("ShapeCliqueLookup");

    res = 0;
    fini = FALSE;
    /* scan all function arguments */
    arg = FUNDEF_ARGS (arg_node);
    while (arg != NULL) {
        if (ARG_AVIS (arg) == scavis) {
            fini = TRUE;
            break;
        }
        res++;
        arg = ARG_NEXT (arg);
    }

    /* scan all locals if we are still looking */
    if (!fini) {
        arg = FUNDEF_BODY (arg_node); /* maybe N_block */
        if (NULL != arg) {
            arg = BLOCK_VARDEC (arg);
            while (arg != NULL) {
                if (VARDEC_AVIS (arg) == scavis) {
                    fini = TRUE;
                    break;
                }
                res++;
                arg = VARDEC_NEXT (arg);
            }
        }
    }

    DBUG_ASSERT (fini, "Failed to find N_avis in args or vardecs");
    DBUG_RETURN (res);
}

/** <!--*******************************************************************-->
 *
 * @fn int SCIShapeCliqueNumber(node *avis, node *arg_node)
 * @brief Utility function to provide shape clique identifier for
 *        human-readable printing.
 *
 *     @param arg_node for function
 *     @param N_avis address for any member of the shape clique
 *
 * ****************************************************************************/
int
SCIShapeCliqueNumber (node *avis, node *arg_node)
{

    /* This function generates a unique small integer identifier for
     * each shape clique. It does this by finding the minimum N_avis
     * address in each clique, then returns the index of that N_avis
     * entry in the list of arguments and locals. In APLish notation,
     * we'd write this as:
     *
     *
     * We could use the result of SCIShapeCliqueMin for this,
     * but the small integer we generate here is much more readable.
     */

    node *cliquemin;
    int res;

    DBUG_ENTER ("SCIShapeCliqueNumber");
    cliquemin = ShapeCliqueMin (avis);
    res = ShapeCliqueLookup (cliquemin, arg_node);
    DBUG_RETURN (res);
}

/*@}*/
/*@}*/

/** <!--*******************************************************************-->
 *
 * @name Traversal Functions for SCI:
 *
 * @{
 ****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *SCIprf( node *arg_node, info *arg_info )
 *
 *****************************************************************************/

node *
SCIprf (node *arg_node, info *arg_info)
{
    node *lhs, *arg1, *arg2, *lhsavis, *arg1avis, *arg2avis;

    DBUG_ENTER ("SCIprf");

    switch (PRF_PRF (arg_node)) {
    case F_add_SxA: /* Scalar-left dyadic scalar functions */
    case F_sub_SxA:
    case F_mul_SxA:
    case F_div_SxA:
    case F_rotate:
        /* Place lhs and arg2 in same shape clique */
        lhs = INFO_LHS (arg_info);
        arg2 = PRF_ARG2 (arg_node);
        lhsavis = IDS_AVIS (lhs);
        arg2avis = ID_AVIS (arg2);
        DBUG_ASSERT ((AVIS_SHAPECLIQUEID (lhsavis) == SHAPECLIQUEIDNONE (lhsavis)),
                     "PRF SxA lhs shape clique is non-degenerate");
        /* By non-degenerate, I mean that the clique has more than 1 member */
        AppendAvisToShapeClique (lhsavis, arg2avis);
        break;

    case F_add_AxS: /* Scalar-right dyadic scalar functions */
    case F_sub_AxS:
    case F_mul_AxS:
    case F_div_AxS:
    case F_modarray: /* the modarray primitive function */
    case F_toi_A:    /* Monadic type coercion functions */
    case F_tof_A:
    case F_tod_A:
        /* Place lhs id and arg1 in same shape clique */
        lhs = INFO_LHS (arg_info);
        arg1 = PRF_ARG1 (arg_node);
        lhsavis = IDS_AVIS (lhs);
        arg1avis = ID_AVIS (arg1);
        DBUG_ASSERT ((AVIS_SHAPECLIQUEID (lhsavis) == SHAPECLIQUEIDNONE (lhsavis)),
                     "PRF AxS lhs shape clique is non-degenerate");
        AppendAvisToShapeClique (lhsavis, arg1avis);
        break;
        /* Various fns which we are confused about */
        /* The right solution is to invent F_mod_AxS_, etc., for all these */
    case F_mod:
    case F_max:
    case F_min:
    case F_and:
    case F_or:
    case F_eq:
    case F_neq:
    case F_le:
    case F_ge:
    case F_gt:
    case F_lt:
        break;

    case F_neg: /* Monadic scalar functions */
    case F_abs:
    case F_not:
    case F_toi_S: /* Monadic coercion function */
    case F_tof_S:
    case F_tod_S:
        /* Place lhs id and arg1 in same shape clique */
        lhs = INFO_LHS (arg_info);
        lhsavis = IDS_AVIS (lhs);

        arg1 = PRF_ARG1 (arg_node);
        if (N_id == NODE_TYPE (arg1)) {
            arg1avis = ID_AVIS (arg1);
            DBUG_ASSERT ((AVIS_SHAPECLIQUEID (lhsavis) == SHAPECLIQUEIDNONE (lhsavis)),
                         "PRF AxS lhs shape clique is non-degenerate");
            AppendAvisToShapeClique (lhsavis, arg1avis);
        }
        break;

    case F_add_SxS: /* Scalar-Scalar dyadic scalar functions */
    case F_sub_SxS:
    case F_mul_SxS:
    case F_div_SxS:
        /* Maybe place lhs , arg1, and arg2 in same shape clique */
        lhs = INFO_LHS (arg_info);
        lhsavis = IDS_AVIS (lhs);
        DBUG_ASSERT ((AVIS_SHAPECLIQUEID (lhsavis) == SHAPECLIQUEIDNONE (lhsavis)),
                     "PRF AxS lhs shape clique is non-degenerate");

        arg1 = PRF_ARG1 (arg_node);
        if (N_id == NODE_TYPE (arg1)) {
            arg1avis = ID_AVIS (arg1);
            AppendAvisToShapeClique (lhsavis, arg1avis);
        }

        arg2 = PRF_ARG2 (arg_node);
        if (N_id == NODE_TYPE (arg2)) {
            arg2avis = ID_AVIS (arg2);
            AppendAvisToShapeClique (lhsavis, arg2avis);
        }

        break;

        /* Can't handle dyadic scalar fns today, as */
        /* they need shape clique guards. */
    case F_add_AxA:
    case F_sub_AxA:
    case F_mul_AxA:
    case F_div_AxA:
        break;

        /* Can't handle these, either */
    case F_shape:
    case F_reshape:
    case F_shape_sel:
    case F_run_mt_fold:
    case F_noop:
    case F_copy:
    case F_dim:
    case F_idx_shape_sel:
    case F_idx_sel:
    case F_idx_modarray:
    case F_sel:
    case F_take:
    case F_drop:
    case F_cat:
    case F_take_SxV:
    case F_drop_SxV:
    case F_vect2offset:
    case F_idxs2offset:
    case F_cat_VxV:
    case F_accu:
    case F_alloc:
    case F_reuse:
    case F_alloc_or_reuse:
    case F_alloc_or_reshape:
    case F_isreused:
    case F_suballoc:
    case F_wl_assign:
    case F_wl_break:
    case F_fill:
    case F_inc_rc:
    case F_dec_rc:
    case F_free:
    case F_to_unq:
    case F_from_unq:
    case F_type_conv:
    case F_dispatch_error:
    case F_type_error:
    case F_esd_neg:
    case F_esd_rec:
        /* the following are in the land of the living dead. */
    case F_run_mt_genarray:
    case F_run_mt_modarray:
    case F_genarray:
        break;
        /* default intentionally omitted, so we catch any new entries... */
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCImodarray(node *arg_node, info *arg_info)
 *
 * @brief   For a WL of the form:
 *   C = with() modarray( B, iv, val),  place C in same shape clique as B.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 * *****************************************************************************/
node *
SCImodarray (node *arg_node, info *arg_info)
{
    node *lhs, *rhs, *lhsavis, *rhsavis;

    DBUG_ENTER ("SCImodarray");

    rhs = MODARRAY_ARRAY (arg_node);
    if (N_id == NODE_TYPE (rhs)) {
        DBUG_PRINT ("SCI", ("found modarray N_id"));
        /* Place C in same shape clique as B */
        lhs = INFO_LHS (arg_info);
        lhsavis = IDS_AVIS (lhs);
        rhsavis = ID_AVIS (rhs);
        DBUG_ASSERT ((AVIS_SHAPECLIQUEID (lhsavis) == SHAPECLIQUEIDNONE (lhsavis)),
                     "modarray lhs is non-degenerate shape clique");

        AppendAvisToShapeClique (lhsavis, rhsavis);
    } else {
        DBUG_PRINT ("SCI", ("found modarray non- N_id"));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCIgenarray(node *arg_node, info *arg_info)
 *
 * @brief  node *SCIgenarray( node *arg_node, info *arg_info)
 *         With-loop genarray shape clique inference
 *
 *     @param arg_node
 *     @param arg_info
 *
 *    @return
 *
 * *****************************************************************************/
node *
SCIgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCIgenarray");
    /*
     * For a WL of the form:
     *
     *    S = shape(B);
     *    C = with() genarray( S, defaultelement),
     *
     * place C in same shape clique as B, IFF def is a scalar.
     * It may be possible to get fancier with non-scalar default element, but
     * you gotta start somewhere.
     */

    node *shp, *def;
    shp = GENARRAY_SHAPE (arg_node);
    def = GENARRAY_DEFAULT (arg_node);
    if (N_id == NODE_TYPE (shp)) { /* S may be an N_id or an N_array */
        DBUG_PRINT ("SCI", ("found genarray shape N_id"));
    }
    if (NULL == def) { /* No default cell or scalar cell is good! */
        /*
         * TODO: If def non-null, have to check exprs for scalar default
         *       element!
         */
        def = def;
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *SCIlet( node *arg_node, info *arg_info)
 * @brief This function handles assignments of values to names.
 *          The lhs and rhs are placed into the same shape clique via
 *          a set join operation.
 *
 *****************************************************************************/
node *
SCIlet (node *arg_node, info *arg_info)
{
    node *oldids;

    DBUG_ENTER ("SCIlet");

    /*
     * Place lhs in same clique as rhs. If rhs is not in a clique already,
     * make a clique from lhs and rhs.
     * We tuck the lhs into the arg_info and pass it down to whoever
     * finds the rhs avis; they will do the dirty work.
     */
    oldids = INFO_LHS (arg_info); /* with within with can cause recursion */
    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    INFO_LHS (arg_info) = oldids;

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *SCIid( node *arg_node, info *arg_info)
 * @brief This function handles assignments of values to names.
 *          The lhs and rhs are placed into the same shape clique via
 *          a set join operation.
 *
 *****************************************************************************/
node *
SCIid (node *arg_node, info *arg_info)
{
    node *lhs, *rhs;

    DBUG_ENTER ("SCIid");

    lhs = INFO_LHS (arg_info);
    rhs = ID_AVIS (arg_node);
    AppendAvisToShapeClique (ID_AVIS (lhs), ID_AVIS (rhs));

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *SCIfundef( node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
SCIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCIfundef");

    /*
     * Do nothing if wrapper function, external function, do-fun or cond-fun,
     * or if fn does not have a body
     */
    if ((FUNDEF_BODY (arg_node) != NULL) && (!FUNDEF_ISWRAPPERFUN (arg_node))
        && (!FUNDEF_ISCONDFUN (arg_node)) && (!FUNDEF_ISDOFUN (arg_node))) {

        ResetAllShapeCliques (arg_node, arg_info);
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        BuildAKSShapeCliques (arg_node);

        DBUG_PRINT ("SCI",
                    ("Function: %s: Shape cliques inferred:", FUNDEF_NAME (arg_node)));
        ShapeCliquePrintIDs (arg_node);
    }

    if (NULL != FUNDEF_NEXT (arg_node)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *SCIdoShapeCliqueInference( node *syntax_tree)
 *
 *   @brief this function infers shape clique membership for all arrays.
 *   @param part of the AST (usually the entire tree) i is to be applied on.
 *   @return modified AST.
 *
 *****************************************************************************/
node *
SCIdoShapeCliqueInference (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("SCIdoShapeCliqueInference");

    DBUG_PRINT ("OPT", ("Starting shape clique inference..."));

    TRAVpush (TR_sci);

    info = MakeInfo ();
    syntax_tree = TRAVdo (syntax_tree, info);

    info = FreeInfo (info);
    TRAVpop ();

    DBUG_PRINT ("OPT", ("shape clique inference complete!"));

    DBUG_RETURN (syntax_tree);
}

/** <!--*******************************************************************-->
 *
 * @fn node *SCIfindShapeCliqueForShape(shape * shp, node *arg_node)
 * @brief Find shapeclique for shape
 *  @param - shp is AKS shape we are looking for
 *  @param - arg_node is N_fundef for the function
 *
 *  @return N_avis of the shape clique with that shape
 *          or NULL if no such shape is found.
 * ****************************************************************************/
node *
SCIfindShapeCliqueForShape (shape *shp, node *arg_node)
{
    node *res;
    node *arg;
    node *curavis;
    shape *curshape;
    bool fini;
    ntype *curtype;

    DBUG_ENTER ("SCIfindCliqueForShape");
    DBUG_ASSERT ((N_fundef == NODE_TYPE (arg_node)),
                 "SCIfindCliqueForShape expected N_fundef as arg");

    res = NULL;
    fini = FALSE;

    /* search function arguments */
    arg = FUNDEF_ARGS (arg_node);
    while (arg != NULL) {
        curavis = ARG_AVIS (arg);
        curtype = AVIS_TYPE (curavis);
        if (TUshapeKnown (curtype)) { /* AKs only, please */
            curshape = TYgetShape (curtype);
            if (SHcompareShapes (shp, curshape)) {
                res = curavis;
                fini = TRUE;
                break;
            }
        }
        arg = ARG_NEXT (arg);
    }

    /* search function locals */
    arg = FUNDEF_BODY (arg_node); /* maybe N_block */
    if ((!fini) && (arg != NULL)) {
        arg = BLOCK_VARDEC (arg);
        while (arg != NULL) {
            curavis = VARDEC_AVIS (arg);
            curtype = AVIS_TYPE (curavis);
            if (!TUshapeKnown (curtype)) { /* AKs only, please */
                curshape = TYgetShape (curtype);
                if (SHcompareShapes (shp, curshape)) {
                    res = curavis;
                    fini = TRUE;
                    break;
                }
            }
            arg = VARDEC_NEXT (arg);
        }
    }
    DBUG_RETURN (res);
}

/*@}*/
/*@} */ /* defgroup SCI */
