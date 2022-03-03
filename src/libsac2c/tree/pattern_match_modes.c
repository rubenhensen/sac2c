/**
 *
 * @file pattern_match_modes.c
 *
 * Overview:
 * =========
 *
 * This module provides the different "modes" of pattern-matching
 * supported in the compiler framework. For basics on pattern matching
 * see the file pattern_match.c.
 *
 * The "mode" of a pattern match concerns the way the source code
 * is being looked at. Here a few examples:
 *
 * A source-code of the form
 *
 *  a = 10;
 *  b = 20;
 *  c,g = _non_neg_val_S( b);
 *  d = _add_SxS_( a, c);
 *
 * when looking at the expression _add_SxS_( a, c) is considered either
 *
 *    _add_SxS_( a, c)      (exact mode),
 *    _add_SxS_( 10, c)     (flat mode), or
 *    _add_SxS_( 10, 20)    (flatSkipGuards mode)
 *
 * So, if we match against a pattern that looks for a constant first argument,
 * only the two latter modes will be successfull!
 *
 * In most occasions, the predefined modes will suffice. However, in some
 * situations more flexibility on how expressions are being looked at is
 * required. The DistributiveLaw optimisation is one such case. There, a
 * mechanism is needed that inhibits matching of subexpressions that are
 * shared. However, the ability to look beyond definitions and to skip
 * guards are both desireable in that context too.
 *
 * The solution to this is a rather generic mechanism for specifying
 * patern matching modes as provided through this module. The basic idea
 * is to provide a list of skipping aspects which are being applied
 * *before* matching an expression or subexpression. The aspects are applied
 * one after the other in a cyclic fashion until either there is no further
 * skipping heppenening or one of the aspects return NULL.
 *
 * We have two pre-defined skipping aspects:
 *
 * 1) node *PMMskipId( void * param, node *expr)
 *
 *    This aspects follows from a variable to its definition. In the above
 *    example it is responsible for seeing 10 rather than 'a' or for
 *    seing 20 rather than 'b'. The optional parameter param takes a
 *    lookup table which enables looking through function definitions as
 *    well. The lookup table provided should be generated by applying
 *                PMBLdoBuildPatternMatchingLut
 *
 * 2) node *PMMskipPrf( void *param, node *expr)
 *
 *    This aspect follows from a variable that is the results of the
 *    application of a prf from a specific set of prfs to a
 *    "corresponding" argument of that prf. Correspondence here is simply
 *    the argument/ result position! In our example this
 *    enables 'c' to be viewed as being 'b'. As the flatSkipGuards
 *    mode contains both aspects, 'c' is actualy looked at as 20.
 *
 *    The set of prfs is characterised through a predicate function given
 *    as a parameter to PMMskipPrf.
 */

/** <!--*********************************************************************-->
 *
 * includes, typedefs, and static variables:
 */

#include "pattern_match_modes.h"

#define DBUG_PREFIX "PMM"
#include "debug.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "LookUpTable.h"

/*
 * helper functions:
 */

/** <!--*******************************************************************-->
 *
 * @fn node *findCorrespondingArg( node *avis, node *ids, node *exprs)
 *
 * @brief searches for the identifier specified in'avis' in 'ids' and
 *        returns the expression in 'exprs' whose position corresponds
 *        to the position of 'avis' in 'ids'; otherwise, it returns NULL.
 *
 * @param avis: identifier we are looking for
 *        ids:  chain of identifiers
 *        exprs: chain of expressions
 *
 * @return pointer to expression or NULL
 *
 *****************************************************************************/

static node *
findCorrespondingArg (node *avis, node *ids, node *exprs)
{
    DBUG_ENTER ();

    node *expr;

    while ((ids != NULL) && (avis != IDS_AVIS (ids)) && (exprs != NULL)) {
        ids = IDS_NEXT (ids);
        exprs = EXPRS_NEXT (exprs);
    }

    if ((ids != NULL) && (avis == IDS_AVIS (ids)) && (exprs != NULL)) {
        expr = EXPRS_EXPR (exprs);
    } else {
        expr = NULL;
    }

    DBUG_RETURN (expr);
}

/*
 * pre-defined skipping functions:
 */

/** <!--*******************************************************************-->
 *
 * @fn node *PMMskipId( void *param, node * expr)
 *
 * @brief follows "expr" to its definition if "expr" is an N_id node and
 *        is either locally defined (AVIS_SSAASSIGN) or the corresponding
 *        avis is contained in a LUT given as parameter.
 *
 * @param param: optional LUT for matching across function boundaries
 *        expr: expression to skip
 *
 * @return either 'expr' or pointer to its definition
 *
 *****************************************************************************/

node *
PMMskipId (intptr_t param, node *expr)
{
    DBUG_ENTER ();
    lut_t *follow_lut = (lut_t *)param;
    node *let;
    node *new_id;

    if (NODE_TYPE (expr) == N_id) {
        if (AVIS_SSAASSIGN (ID_AVIS (expr)) != NULL) {
            let = ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (expr)));
            if (TCcountIds (LET_IDS (let)) == 1) {
                expr = LET_EXPR (let);
            }
        } else if (AVIS_SCALARS (ID_AVIS (expr)) != NULL) {
            expr = AVIS_SCALARS (ID_AVIS (expr));
        } else if (follow_lut != NULL) {
            new_id = (node *)LUTsearchInLutP (follow_lut, ID_AVIS (expr));
            expr = (new_id != NULL ? new_id : expr);
        }
    }

    DBUG_RETURN (expr);
}

/** <!--*******************************************************************-->
 *
 * @fn node *PMMskipPrf( void *param, node * expr)
 *
 * @brief follows "expr" to its definition if "expr" is an N_id node and
 *        is locally defined (AVIS_SSAASSIGN) by a prf contained in a set
 *        of prfs characterised through "param".
 *        If so, the corresponding (in position) argument is returned.
 *
 * @param param: predicate on prfs
 *        expr: expression to skip
 *
 * @return either 'expr' or pointer to the corresponding argument.
 *
 *****************************************************************************/

node *
PMMskipPrf (intptr_t param, node *expr)
{
    DBUG_ENTER ();
    prf_match_fun_t *prfInspectFun = (prf_match_fun_t *)param;
    node *let;
    node *rhs;
    node *ids, *avis;

    if (NODE_TYPE (expr) == N_id) {
        if (AVIS_SSAASSIGN (ID_AVIS (expr)) != NULL) {
            let = ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (expr)));
            ids = LET_IDS (let);
            avis = ID_AVIS (expr);
            rhs = LET_EXPR (let);
            if ((NODE_TYPE (rhs) == N_prf) && (prfInspectFun (PRF_PRF (rhs)))) {
                switch (PRF_PRF(rhs)) {
                    case F_guard:            // X1', ... = guard (X1, ..., p)
                    case F_afterguard:       // X' = afterguard (X, p1, ...)
                    case F_same_shape_AxA:   // X', Y', p = same_shape (X, Y)
                    case F_non_neg_val_S:    // X', p = non_neg (X)
                    case F_non_neg_val_V:    // X', p = non_neg (X)
                    case F_noteminval:       // iv'  = noteminval(iv, bound)
                    case F_notemaxval:       // iv'  = notemaxval(iv, bound)
                    case F_noteintersect:    // iv'  = noteintersect(iv, bound)
                        expr = findCorrespondingArg (avis, ids, PRF_ARGS (rhs));
                        break;
                    case F_type_constraint:  // X' = type_constraint (type, X')
                        expr = (avis == IDS_AVIS (ids) ? PRF_ARG2 (rhs) : expr);
                        break;
                    case F_shape_matches_dim_VxA:  // idx', p = shp_m_dim (idx, a)
                    case F_val_lt_shape_VxA:       // idx', p = val_lt_shape (idx, a)
                    case F_val_le_val_VxV:         // v1', p = val_le_val (v1, v2)
                    case F_val_le_val_SxS:         // v1', p = val_le_val (v1, v2)
                    case F_val_lt_val_SxS:         // v1', p = val_lt_val (v1, v2)
                    case F_prod_matches_prod_shape_VxA: // s, p = p_m_p_s (s, a)
                        expr = (avis == IDS_AVIS (ids) ? PRF_ARG1 (rhs) : expr);
                        break;
                    default:
                        break;
                    
                }
            }
        }
    }

    DBUG_RETURN (expr);
}

/**
 *
 * A few suitable prf predicates:
 */

bool
PMMisInExtrema (prf prfun)
{
    DBUG_ENTER ();
    DBUG_RETURN ((prfun == F_noteminval) || (prfun == F_notemaxval)
                 || (prfun == F_noteintersect));
}

bool
PMMisInGuards (prf prfun)
{
    DBUG_ENTER ();
    DBUG_RETURN ((prfun == F_guard) || (prfun == F_afterguard)
                 || (prfun == F_type_constraint) || (prfun == F_same_shape_AxA)
                 || (prfun == F_shape_matches_dim_VxA) || (prfun == F_non_neg_val_S)
                 || (prfun == F_non_neg_val_V) || (prfun == F_val_lt_shape_VxA)
                 || (prfun == F_val_le_val_VxV) || (prfun == F_val_le_val_SxS)
                 || (prfun == F_val_lt_val_SxS)
                 || (prfun == F_prod_matches_prod_shape_VxA));
}

bool
PMMisInExtremaOrGuards (prf prfun)
{
    DBUG_ENTER ();
    DBUG_RETURN (PMMisInExtrema (prfun) || PMMisInGuards (prfun));
}

bool
PMMisAfterguard (prf prfun)
{
    DBUG_ENTER ();
    DBUG_RETURN ((prfun == F_afterguard));
}

/**
 *
 * pre-defined modes:
 */

static pm_mode_t pmm_exact[1] = {{NULL, (intptr_t)NULL}};

static pm_mode_t pmm_flat[2] = {{PMMskipId, (intptr_t)NULL}, {NULL, (intptr_t)NULL}};

static pm_mode_t pmm_flatPrf[3]
  = {{PMMskipPrf, (intptr_t)NULL}, {PMMskipId, (intptr_t)NULL}, {NULL, (intptr_t)NULL}};

pm_mode_t *
PMMexact ()
{
    return (pmm_exact);
}

pm_mode_t *
PMMflat ()
{
    pmm_flat[0].param = (intptr_t)NULL;
    return (pmm_flat);
}

pm_mode_t *
PMMflatLut (lut_t *f_lut)
{
    pmm_flat[0].param = (intptr_t)f_lut;
    return (pmm_flat);
}

pm_mode_t *
PMMflatPrf (prf_match_fun_t *prfInspectFun)
{
    pmm_flatPrf[0].param = (intptr_t)prfInspectFun;
    pmm_flatPrf[1].param = (intptr_t)NULL;
    return (pmm_flatPrf);
}

pm_mode_t *
PMMflatPrfLut (prf_match_fun_t *prfInspectFun, lut_t *f_lut)
{
    pmm_flatPrf[0].param = (intptr_t)prfInspectFun;
    pmm_flatPrf[1].param = (intptr_t)f_lut;
    return (pmm_flatPrf);
}

#undef DBUG_PREFIX
