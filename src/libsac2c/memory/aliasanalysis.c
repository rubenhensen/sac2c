/** <!--********************************************************************-->
 *
 * @defgroup aa Alias Analysis
 *
 * Annotates potentially aliased variables with the AVIS_ISALIAS flag.
 *
 * @ingroup mm
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file aliasanalysis.c
 *
 * Prefix: EMAA
 *
 *****************************************************************************/
#include "aliasanalysis.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#define DBUG_PREFIX "EMAA"
#include "debug.h"

#include "print.h"
#include "new_types.h"
#include "DataFlowMask.h"
#include "user_types.h"
#include "str.h"
#include "memory.h"

/**
 * CONTEXT enumeration: aa_context
 */
typedef enum { AA_undef, AA_begin, AA_end, AA_let, AA_ap } aa_context;

/**
 * Convergence counter
 */

static int unaliased = 0;

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    aa_context context;
    node *fundef;
    node *lhs;
    dfmask_t *mask;
    dfmask_t *localmask;
    node *apargs;
    dfmask_t *apmask;
    node *funargs;
};

#define INFO_CONTEXT(n) (n->context)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LHS(n) (n->lhs)
#define INFO_MASK(n) (n->mask)
#define INFO_LOCALMASK(n) (n->localmask)
#define INFO_APMASK(n) (n->apmask)
#define INFO_APARGS(n) (n->apargs)
#define INFO_FUNARGS(n) (n->funargs)

static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_CONTEXT (result) = AA_undef;
    INFO_FUNDEF (result) = fundef;
    INFO_LHS (result) = NULL;
    INFO_MASK (result) = NULL;
    INFO_LOCALMASK (result) = NULL;
    INFO_APARGS (result) = NULL;
    INFO_APMASK (result) = NULL;
    INFO_FUNARGS (result) = NULL;

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
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *EMAAdoAliasAnalysis( node *arg_node)
 *
 * @brief starting point of Alias Analysis traversal
 *
 * @param arg_node
 *
 * @return
 *
 *****************************************************************************/
node *
EMAAdoAliasAnalysis (node *syntax_tree)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Starting alias analysis...");

    TRAVpush (TR_emaa);
    syntax_tree = TRAVdo (syntax_tree, NULL);
    TRAVpop ();

    DBUG_PRINT ("%d variables unaliased.", unaliased);
    DBUG_PRINT ("Alias analysis complete.");

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

static node *
SetAvisAlias (node *avis, bool newval)
{
    DBUG_ENTER ();

    if (AVIS_ISALIAS (avis) && (!newval)) {
        unaliased += 1;
    }

    AVIS_ISALIAS (avis) = newval;

    DBUG_RETURN (avis);
}

static bool
GetRetAlias (node *fundef, int num)
{
    bool res = TRUE;
    node *nl;

    DBUG_ENTER ();

    nl = FUNDEF_RETS (fundef);
    while ((nl != NULL) && (num > 0)) {
        nl = RET_NEXT (nl);
        num -= 1;
    }

    if (nl != NULL) {
        res = RET_ISALIASING (nl);
    }

    DBUG_RETURN (res);
}

static void
MarkAllIdsAliasing (node *ids, dfmask_t *mask)
{
    DBUG_ENTER ();

    while (ids != NULL) {
        DFMsetMaskEntrySet (mask, IDS_AVIS (ids));
        ids = IDS_NEXT (ids);
    }

    DBUG_RETURN ();
}

static void
MarkIdAliasing (node *id, dfmask_t *mask)
{
    DBUG_ENTER ();

    if (NODE_TYPE (id) == N_id) {
        DFMsetMaskEntrySet (mask, ID_AVIS (id));
    }

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *EMAAap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMAAap (node *arg_node, info *arg_info)
{
    node *_ids;
    int argc;

    DBUG_ENTER ();

    if (FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node))) {
        /*
         * Traverse conditional functions in order of appearance
         */
        INFO_APARGS (arg_info) = AP_ARGS (arg_node);
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
        INFO_APARGS (arg_info) = NULL;
    }

    /*
     * Check whether arguments could have been aliased
     */
    INFO_CONTEXT (arg_info) = AA_ap;
    INFO_FUNARGS (arg_info) = FUNDEF_ARGS (AP_FUNDEF (arg_node));
    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    }

    /*
     * Check whether return values are alias-free
     */
    argc = 0;
    _ids = INFO_LHS (arg_info);

    while (_ids != NULL) {
        if (GetRetAlias (AP_FUNDEF (arg_node), argc)) {
            DFMsetMaskEntrySet (INFO_MASK (arg_info), IDS_AVIS (_ids));
        }
        _ids = IDS_NEXT (_ids);
        argc += 1;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMAAarg( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMAAarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (INFO_CONTEXT (arg_info)) {

    case AA_begin:
        if (INFO_APARGS (arg_info) != NULL) {
            node *id = EXPRS_EXPR (INFO_APARGS (arg_info));
            if (DFMtestMaskEntry (INFO_APMASK (arg_info), ID_AVIS (id))) {

                DFMsetMaskEntrySet (INFO_MASK (arg_info), ARG_AVIS (arg_node));
            }
            INFO_APARGS (arg_info) = EXPRS_NEXT (INFO_APARGS (arg_info));
        } else {
            if (AVIS_ISALIAS (ARG_AVIS (arg_node))) {
                DFMsetMaskEntrySet (INFO_MASK (arg_info), ARG_AVIS (arg_node));
            }
        }
        break;

    case AA_end:
        ARG_AVIS (arg_node) = SetAvisAlias (ARG_AVIS (arg_node),
                                            DFMtestMaskEntry (INFO_MASK (arg_info),
                                                              ARG_AVIS (arg_node)));
        break;

    default:
        DBUG_UNREACHABLE ("Illegal context!");
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMAAassign( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMAAassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_CONTEXT (arg_info) = AA_undef;
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMAAcode( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMAAcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (CODE_CBLOCK (arg_node) != NULL) {
        dfmask_t *oldmask, *oldlocalmask;

        oldmask = INFO_MASK (arg_info);
        oldlocalmask = INFO_LOCALMASK (arg_info);

        INFO_MASK (arg_info) = DFMgenMaskCopy (oldmask);
        INFO_LOCALMASK (arg_info) = DFMgenMaskCopy (oldlocalmask);
        DFMsetMaskClear (INFO_LOCALMASK (arg_info));

        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);

        DFMsetMaskAnd (INFO_MASK (arg_info), INFO_LOCALMASK (arg_info));
        DFMsetMaskOr (oldmask, INFO_MASK (arg_info));
        DFMsetMaskOr (oldlocalmask, INFO_LOCALMASK (arg_info));

        INFO_MASK (arg_info) = DFMremoveMask (INFO_MASK (arg_info));
        INFO_LOCALMASK (arg_info) = DFMremoveMask (INFO_LOCALMASK (arg_info));

        INFO_LOCALMASK (arg_info) = oldlocalmask;
        INFO_MASK (arg_info) = oldmask;
    }

    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMAAcond( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMAAcond (node *arg_node, info *arg_info)
{
    dfmask_t *thenmask, *elsemask, *oldmask;

    DBUG_ENTER ();

    oldmask = INFO_MASK (arg_info);
    thenmask = DFMgenMaskCopy (oldmask);
    elsemask = DFMgenMaskCopy (oldmask);

    INFO_MASK (arg_info) = thenmask;
    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);

    INFO_MASK (arg_info) = elsemask;
    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    DFMsetMaskOr (oldmask, thenmask);
    DFMsetMaskOr (oldmask, elsemask);

    thenmask = DFMremoveMask (thenmask);
    elsemask = DFMremoveMask (elsemask);
    INFO_MASK (arg_info) = oldmask;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMAAfold( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMAAfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (!FOLD_ISPARTIALFOLD (arg_node)) {
        DFMsetMaskEntrySet (INFO_MASK (arg_info), IDS_AVIS (INFO_LHS (arg_info)));
    }

    DFMsetMaskEntrySet (INFO_MASK (arg_info), ID_AVIS (FOLD_NEUTRAL (arg_node)));

    if (FOLD_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        FOLD_NEXT (arg_node) = TRAVdo (FOLD_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMAAgenarray( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMAAgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (GENARRAY_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMAAmodarray( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMAAmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (MODARRAY_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMAAfuncond( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMAAfuncond (node *arg_node, info *arg_info)
{
    node *thenid, *elseid;

    DBUG_ENTER ();

    thenid = FUNCOND_THEN (arg_node);
    elseid = FUNCOND_ELSE (arg_node);

    /*
     * Arguments must themselves not be aliases
     */
    if ((DFMtestMaskEntry (INFO_MASK (arg_info), ID_AVIS (thenid)))
        || (DFMtestMaskEntry (INFO_MASK (arg_info), ID_AVIS (elseid)))) {

        DFMsetMaskEntrySet (INFO_MASK (arg_info), IDS_AVIS (INFO_LHS (arg_info)));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMAAfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMAAfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Traversing function %s", FUNDEF_NAME (arg_node));

    if ((!FUNDEF_ISCONDFUN (arg_node)) || (arg_info != NULL)) {

        if (FUNDEF_BODY (arg_node) != NULL) {
            info *info;
            dfmask_base_t *maskbase;

            info = MakeInfo (arg_node);

            maskbase = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDECS (arg_node));

            INFO_MASK (info) = DFMgenMaskClear (maskbase);
            INFO_LOCALMASK (info) = DFMgenMaskClear (maskbase);

            if (arg_info != NULL) {
                INFO_APARGS (info) = INFO_APARGS (arg_info);
                INFO_APMASK (info) = INFO_MASK (arg_info);
            }

            /*
             * Traverse function args to mark them as ALIAS in MASK
             */
            INFO_CONTEXT (info) = AA_begin;
            if (FUNDEF_ARGS (arg_node) != NULL) {
                FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), info);
            }

            /*
             * Aliased CONDFUN parameters must be marked as ALIAS
             */
            if (arg_info != NULL) {
                node *funargs = FUNDEF_ARGS (arg_node);
                node *apargs = INFO_APARGS (arg_info);

                while (funargs != NULL) {
                    node *apargs2 = INFO_APARGS (arg_info);

                    while (apargs2 != NULL) {
                        if (apargs != apargs2) {
                            if (ID_AVIS (EXPRS_EXPR (apargs))
                                == ID_AVIS (EXPRS_EXPR (apargs2))) {
                                DFMsetMaskEntrySet (INFO_MASK (info),
                                                    ARG_AVIS (funargs));
                            }
                        }
                        apargs2 = EXPRS_NEXT (apargs2);
                    }

                    funargs = ARG_NEXT (funargs);
                    apargs = EXPRS_NEXT (apargs);
                }
            }

            INFO_APARGS (info) = NULL;
            INFO_APMASK (info) = NULL;

            /*
             * Traverse function body
             */
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);

            /*
             * Traverse args to annotate AVIS_ISALIAS
             */
            INFO_CONTEXT (info) = AA_end;
            if (FUNDEF_ARGS (arg_node) != NULL) {
                FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), info);
            }

            /*
             * Clean up
             */
            INFO_MASK (info) = DFMremoveMask (INFO_MASK (info));
            INFO_LOCALMASK (info) = DFMremoveMask (INFO_LOCALMASK (info));

            maskbase = DFMremoveMaskBase (maskbase);

            info = FreeInfo (info);
        }
    }

    if (arg_info == NULL) {
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMAAid( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMAAid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (INFO_CONTEXT (arg_info)) {
    case AA_let:
        /*
         * ALIASING OPERATION a = b
         * mark a and b as aliases
         */
        DFMsetMaskEntrySet (INFO_MASK (arg_info), ID_AVIS (arg_node));
        DFMsetMaskEntrySet (INFO_MASK (arg_info), IDS_AVIS (INFO_LHS (arg_info)));
        break;

    case AA_ap:
        if (INFO_FUNARGS (arg_info) != NULL) {
            if (ARG_ISALIASING (INFO_FUNARGS (arg_info))) {
                DFMsetMaskEntrySet (INFO_MASK (arg_info), ID_AVIS (arg_node));
            }
            INFO_FUNARGS (arg_info) = ARG_NEXT (INFO_FUNARGS (arg_info));
        } else {
            DFMsetMaskEntrySet (INFO_MASK (arg_info), ID_AVIS (arg_node));
        }
        break;

    default:
        DBUG_UNREACHABLE ("Illegal context");
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMAAlet( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMAAlet (node *arg_node, info *arg_info)
{
    node *_ids;
    DBUG_ENTER ();

    INFO_CONTEXT (arg_info) = AA_let;
    INFO_LHS (arg_info) = LET_IDS (arg_node);

    /*
     * Mark LHS identifiers as local
     */
    _ids = LET_IDS (arg_node);
    while (_ids != NULL) {
        DFMsetMaskEntrySet (INFO_LOCALMASK (arg_info), IDS_AVIS (_ids));
        _ids = IDS_NEXT (_ids);
    }

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMAAprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMAAprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * Primitive functions in general yield a fresh result.
     * Hence, they won't return an alias of an argument.
     *
     * Exception: F_accu and F_prop_obj_in return an alias
     */
    switch (PRF_PRF (arg_node)) {

    case F_prop_obj_out:
    case F_prop_obj_in:
    case F_accu:
        MarkAllIdsAliasing (INFO_LHS (arg_info), INFO_MASK (arg_info));
        break;

    case F_unshare:
        /* new_accu = unshare( old_accu, iv);
         * The output is always an alias of the first argument.
         */
        MarkIdAliasing (PRF_ARG1 (arg_node), INFO_MASK (arg_info));
        break;

    case F_type_conv:
        MarkAllIdsAliasing (INFO_LHS (arg_info), INFO_MASK (arg_info));
        MarkIdAliasing (PRF_ARG2 (arg_node), INFO_MASK (arg_info));
        break;

    case F_reshape_VxA:
        MarkIdAliasing (PRF_ARG3 (arg_node), INFO_MASK (arg_info));
        break;

    case F_reuse:
        MarkIdAliasing (PRF_ARG1 (arg_node), INFO_MASK (arg_info));
        break;

    case F_afterguard:
        MarkAllIdsAliasing (INFO_LHS (arg_info), INFO_MASK (arg_info));
        MarkIdAliasing (PRF_ARG1 (arg_node), INFO_MASK (arg_info));
        break;

    case F_guard:
        MarkAllIdsAliasing (INFO_LHS (arg_info), INFO_MASK (arg_info));
        {
            node *exprs = EXPRS_EXPRS2 (PRF_ARGS (arg_node));
            while (exprs != NULL) {
                MarkIdAliasing (EXPRS_EXPR (exprs), INFO_MASK (arg_info));
                exprs = EXPRS_NEXT (exprs);
            }
        }
        break;

    case F_prefetch2device:
    case F_prefetch2host:
        MarkAllIdsAliasing (INFO_LHS (arg_info), INFO_MASK (arg_info));
        MarkIdAliasing (PRF_ARG1 (arg_node), INFO_MASK (arg_info));
        break;

    case F_host2device_end:
    case F_device2host_end:
        MarkIdAliasing (PRF_ARG1 (arg_node), INFO_MASK (arg_info));
        break;

    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMAAwith( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMAAwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMAAwith2( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMAAwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
    WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMAAwith3( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMAAwith3 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    WITH3_OPERATIONS (arg_node) = TRAVdo (WITH3_OPERATIONS (arg_node), arg_info);
    WITH3_RANGES (arg_node) = TRAVdo (WITH3_RANGES (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMAAvardec( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMAAvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (DFMtestMaskEntry (INFO_MASK (arg_info), VARDEC_AVIS (arg_node))) {
        DBUG_PRINT ("%s could not be unaliased", VARDEC_NAME (arg_node));
    }

    VARDEC_AVIS (arg_node) = SetAvisAlias (VARDEC_AVIS (arg_node),
                                           DFMtestMaskEntry (INFO_MASK (arg_info),
                                                             VARDEC_AVIS (arg_node)));

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Alias Analysis -->
 *****************************************************************************/

#undef DBUG_PREFIX
