/**
 * @defgroup scre Scope-Based Reuse Candidate Elimination
 *
 * This phase eliminates some illegitimate reuse candidates from _alloc_or_reuse_ /
 * _alloc_or_reshape_, and _alloc_or_resize_ t. In case all resuse candidates are
 * filtered out, the operation is reverted to an _alloc_.
 *
 * The reuse candidates that are filtered out are reuse candidates that stem
 * from references to variables defined outside a surrounding with-loop and, thus,
 * cannot serve as a reuse candidate for an inner with loop. For example:
 *

  a = [1,2,3,4,5,6,7,8,9,10];

  b = with {
        ( . <= iv <= [10]) {
           c = with {
              (. <= jv <= .) : a[iv]+1;
               } : modarray(a);
           } : c[5];
      } : genarray( [100], 0);

 * will identify "a" to be a reuse candidate for "c".
 * This optimisation identifies that "a" was defined outside of the
 * current (definition of c) with-loop level and, therefore, cannot
 * be reused for more than one instance. Hence, it is dismissed!
 *
 * @ingroup mm
 *
 * <pre>
 * </pre>
 * @{
 */

/**
 * @file scoperce.c
 *
 * Prefix: SRCE
 */
#include "scoperce.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "print.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "DataFlowMask.h"

/**
 * INFO structure
 */
struct INFO {
    node *fundef;
    dfmask_base_t *maskbase;
    dfmask_t *rcmask;
    bool rcelim;
};

/**
 * INFO macros
 */
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_MASKBASE(n) (n->maskbase)
#define INFO_RCMASK(n) (n->rcmask)
#define INFO_RCELIM(n) (n->rcelim)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_MASKBASE (result) = NULL;
    INFO_RCMASK (result) = NULL;
    INFO_RCELIM (result) = FALSE;

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
 *
 * @fn node *SRCEdoRemoveReuseCandidates( node *syntax_tree)
 *
 * @brief
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/
node *
SRCEdoRemoveReuseCandidates (node *syntax_tree)
{
    DBUG_ENTER ();

    TRAVpush (TR_srce);
    syntax_tree = TRAVdo (syntax_tree, NULL);
    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * Scope-based reuse candidate elimination traversal (emec_tab)
 *
 * prefix: SRCE
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *SRCEfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SRCEfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISLACFUN (arg_node) && (arg_info != NULL)) {
        if (FUNDEF_BODY (arg_node) != NULL) {
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        }
    } else {
        if (!FUNDEF_ISLACFUN (arg_node)) {
            if (FUNDEF_BODY (arg_node) != NULL) {
                info *info;

                info = MakeInfo ();
                INFO_FUNDEF (info) = arg_node;
                INFO_MASKBASE (info)
                  = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDECS (arg_node));
                INFO_RCMASK (info) = DFMgenMaskClear (INFO_MASKBASE (info));

                if (FUNDEF_ARGS (arg_node) != NULL) {
                    FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), info);
                }

                FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);

                INFO_RCMASK (info) = DFMremoveMask (INFO_RCMASK (info));
                INFO_MASKBASE (info) = DFMremoveMaskBase (INFO_MASKBASE (info));
                info = FreeInfo (info);
            }
        }
    }

    if ((!FUNDEF_ISLACFUN (arg_node)) || (arg_info == NULL)) {
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SRCEarg( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SRCEarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DFMsetMaskEntrySet (INFO_RCMASK (arg_info), NULL, ARG_AVIS (arg_node));

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SRCEassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SRCEassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SRCElet( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SRCElet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SRCEids( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SRCEids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DFMsetMaskEntrySet (INFO_RCMASK (arg_info), NULL, IDS_AVIS (arg_node));

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SRCEap( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SRCEap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node))
        && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))) {
        info *info;
        node *funargs, *apargs;

        info = MakeInfo ();
        INFO_FUNDEF (info) = AP_FUNDEF (arg_node);
        INFO_MASKBASE (info) = DFMgenMaskBase (FUNDEF_ARGS (AP_FUNDEF (arg_node)),
                                               FUNDEF_VARDECS (AP_FUNDEF (arg_node)));
        INFO_RCMASK (info) = DFMgenMaskClear (INFO_MASKBASE (info));

        funargs = FUNDEF_ARGS (AP_FUNDEF (arg_node));
        apargs = AP_ARGS (arg_node);

        while (funargs != NULL) {
            if (DFMtestMaskEntry (INFO_RCMASK (arg_info), NULL,
                                  ID_AVIS (EXPRS_EXPR (apargs)))) {
                DFMsetMaskEntrySet (INFO_RCMASK (info), NULL, ARG_AVIS (funargs));
            }

            funargs = ARG_NEXT (funargs);
            apargs = EXPRS_NEXT (apargs);
        }

        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), info);

        INFO_RCMASK (info) = DFMremoveMask (INFO_RCMASK (info));
        INFO_MASKBASE (info) = DFMremoveMaskBase (INFO_MASKBASE (info));
        info = FreeInfo (info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SRCEcode( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SRCEcode (node *arg_node, info *arg_info)
{
    dfmask_t *oldmask;

    DBUG_ENTER ();

    oldmask = INFO_RCMASK (arg_info);
    INFO_RCMASK (arg_info) = DFMgenMaskClear (INFO_MASKBASE (arg_info));

    if (CODE_CBLOCK (arg_node) != NULL) {
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    }

    INFO_RCMASK (arg_info) = DFMremoveMask (INFO_RCMASK (arg_info));
    INFO_RCMASK (arg_info) = oldmask;

    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SRCEprf( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SRCEprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if ((PRF_PRF (arg_node) == F_alloc_or_reuse)
        || (PRF_PRF (arg_node) == F_alloc_or_reshape)
        || (PRF_PRF (arg_node) == F_alloc_or_resize)) {

        INFO_RCELIM (arg_info) = TRUE;

        if (PRF_EXPRS3 (arg_node) != NULL) {
            PRF_EXPRS3 (arg_node) = TRAVdo (PRF_EXPRS3 (arg_node), arg_info);
        }

        if (PRF_EXPRS3 (arg_node) == NULL) {
            PRF_PRF (arg_node) = F_alloc;
        }

        INFO_RCELIM (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SRCEexprs( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SRCEexprs (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_RCELIM (arg_info)) {

        if (EXPRS_NEXT (arg_node) != NULL) {
            EXPRS_NEXT (arg_node) = TRAVdo (EXPRS_NEXT (arg_node), arg_info);
        }

        if (!DFMtestMaskEntry (INFO_RCMASK (arg_info), NULL,
                               ID_AVIS (EXPRS_EXPR (arg_node)))) {
            arg_node = FREEdoFreeNode (arg_node);
        }
    }

    DBUG_RETURN (arg_node);
}

/* @} */

#undef DBUG_PREFIX
