/******************************************************************************
 *
 * @defgroup AA Alias Analysis
 *
 * Prefix: EMAA
 *
 * Annotates potentially aliased variables with the AVIS_ISALIAS flag.
 *
 ******************************************************************************/
#include "DataFlowMask.h"
#include "globals.h"
#include "memory.h"
#include "new_types.h"
#include "print.h"
#include "str.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "tree_utils.h"
#include "user_types.h"

#define DBUG_PREFIX "EMAA"
#include "debug.h"

#include "aliasanalysis.h"

/******************************************************************************
 *
 * @enum aa_context
 *
 * @brief Alias analysis context.
 *
 ******************************************************************************/
typedef enum {
    AA_undef,
    AA_begin,
    AA_end,
    AA_let,
    AA_ap
} aa_context;

/******************************************************************************
 *
 * @var unaliased
 *
 * @brief Convergence counter.
 *
 ******************************************************************************/
static int unaliased = 0;

/******************************************************************************
 *
 * @struct INFO
 *
 * @param INFO_CONTEXT
 * @param INFO_FUNDEF
 * @param INFO_LHS
 * @param INFO_MASK
 * @param INFO_LOCALMASK
 * @param INFO_APMASK
 * @param INFO_APARGS
 * @param INFO_FUNARGS
 *
 ******************************************************************************/
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

#define INFO_CONTEXT(n) ((n)->context)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_MASK(n) ((n)->mask)
#define INFO_LOCALMASK(n) ((n)->localmask)
#define INFO_APMASK(n) ((n)->apmask)
#define INFO_APARGS(n) ((n)->apargs)
#define INFO_FUNARGS(n) ((n)->funargs)

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

/******************************************************************************
 *
 * @fn node *EMAAdoAliasAnalysis (node *arg_node)
 *
 * @brief Starting point of Alias Analysis traversal.
 *
 ******************************************************************************/
node *
EMAAdoAliasAnalysis (node *arg_node)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Starting alias analysis...");

    TRAVpush (TR_emaa);
    arg_node = TRAVdo (arg_node, NULL);
    TRAVpop ();

    DBUG_PRINT ("%d variables unaliased.", unaliased);
    DBUG_PRINT ("Alias analysis complete.");

    DBUG_RETURN (arg_node);
}

static node *
SetAvisAlias (node *avis, bool newval)
{
    DBUG_ENTER ();

    if (AVIS_ISALIAS (avis) && !newval) {
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
    while (nl != NULL && num > 0) {
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

/******************************************************************************
 *
 * @fn node *EMAAap (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
EMAAap (node *arg_node, info *arg_info)
{
    node *ids;
    int argc;

    DBUG_ENTER ();

    if (FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node))) {
        // Traverse conditional functions in order of appearance
        INFO_APARGS (arg_info) = AP_ARGS (arg_node);
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
        INFO_APARGS (arg_info) = NULL;
    }

    // Check whether arguments could have been aliased
    INFO_CONTEXT (arg_info) = AA_ap;
    INFO_FUNARGS (arg_info) = FUNDEF_ARGS (AP_FUNDEF (arg_node));
    AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);

    // Check whether return values are alias-free
    argc = 0;
    ids = INFO_LHS (arg_info);

    while (ids != NULL) {
        if (GetRetAlias (AP_FUNDEF (arg_node), argc)) {
            DFMsetMaskEntrySet (INFO_MASK (arg_info), IDS_AVIS (ids));
        }

        ids = IDS_NEXT (ids);
        argc += 1;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *EMAAarg (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
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
        ARG_AVIS (arg_node) =
            SetAvisAlias (ARG_AVIS (arg_node),
                          DFMtestMaskEntry (INFO_MASK (arg_info),
                                            ARG_AVIS (arg_node)));
        break;

    default:
        DBUG_UNREACHABLE ("Illegal context!");
        break;
    }

    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *EMAAassign (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
EMAAassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_CONTEXT (arg_info) = AA_undef;
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *EMAAcode (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
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

    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *EMAAcond (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
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

/******************************************************************************
 *
 * @fn node *EMAAfold (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
EMAAfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (!FOLD_ISPARTIALFOLD (arg_node)) {
        DFMsetMaskEntrySet (INFO_MASK (arg_info),
                            IDS_AVIS (INFO_LHS (arg_info)));
    }

    DFMsetMaskEntrySet (INFO_MASK (arg_info),
                        ID_AVIS (FOLD_NEUTRAL (arg_node)));

    if (FOLD_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        FOLD_NEXT (arg_node) = TRAVdo (FOLD_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *EMAAgenarray (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
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

/******************************************************************************
 *
 * @fn node *EMAAmodarray (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
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

/******************************************************************************
 *
 * @fn node *EMAAfuncond (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
EMAAfuncond (node *arg_node, info *arg_info)
{
    node *thenid, *elseid;

    DBUG_ENTER ();

    thenid = FUNCOND_THEN (arg_node);
    elseid = FUNCOND_ELSE (arg_node);

    // Arguments must themselves not be aliases
    if (DFMtestMaskEntry (INFO_MASK (arg_info), ID_AVIS (thenid))
        || DFMtestMaskEntry (INFO_MASK (arg_info), ID_AVIS (elseid))) {

        DFMsetMaskEntrySet (INFO_MASK (arg_info),
                            IDS_AVIS (INFO_LHS (arg_info)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *EMAAfundef (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
EMAAfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Traversing function %s", FUNDEF_NAME (arg_node));

    if (!FUNDEF_ISCONDFUN (arg_node) || arg_info != NULL) {
        if (FUNDEF_BODY (arg_node) != NULL) {
            info *info;
            dfmask_base_t *maskbase;

            info = MakeInfo (arg_node);

            maskbase = DFMgenMaskBase (FUNDEF_ARGS (arg_node),
                                       FUNDEF_VARDECS (arg_node));

            INFO_MASK (info) = DFMgenMaskClear (maskbase);
            INFO_LOCALMASK (info) = DFMgenMaskClear (maskbase);

            if (arg_info != NULL) {
                INFO_APARGS (info) = INFO_APARGS (arg_info);
                INFO_APMASK (info) = INFO_MASK (arg_info);
            }

            // Traverse function args to mark them as ALIAS in MASK
            INFO_CONTEXT (info) = AA_begin;
            FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), info);

            // Aliased CONDFUN parameters must be marked as ALIAS
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

            // Traverse function body
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);

            // Traverse args to annotate AVIS_ISALIAS
            INFO_CONTEXT (info) = AA_end;
            FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), info);

            // Clean up
            INFO_MASK (info) = DFMremoveMask (INFO_MASK (info));
            INFO_LOCALMASK (info) = DFMremoveMask (INFO_LOCALMASK (info));
            maskbase = DFMremoveMaskBase (maskbase);
            info = FreeInfo (info);
        }
    }

    if (arg_info == NULL) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *EMAAid (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
EMAAid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (INFO_CONTEXT (arg_info)) {
    /**
     * ALIASING OPERATION a = b.
     * Mark a and b as aliases.
     */
    case AA_let:
        DFMsetMaskEntrySet (INFO_MASK (arg_info),
                            ID_AVIS (arg_node));
        DFMsetMaskEntrySet (INFO_MASK (arg_info),
                            IDS_AVIS (INFO_LHS (arg_info)));
        break;

    case AA_ap:
        if (INFO_FUNARGS (arg_info) != NULL) {
            if (ARG_ISALIASING (INFO_FUNARGS (arg_info))) {
                DFMsetMaskEntrySet (INFO_MASK (arg_info),
                                    ID_AVIS (arg_node));
            }
            INFO_FUNARGS (arg_info) = ARG_NEXT (INFO_FUNARGS (arg_info));
        } else {
            DFMsetMaskEntrySet (INFO_MASK (arg_info),
                                ID_AVIS (arg_node));
        }
        break;

    default:
        DBUG_UNREACHABLE ("Illegal context");
        break;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *EMAAlet (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
EMAAlet (node *arg_node, info *arg_info)
{
    node *ids;

    DBUG_ENTER ();

    INFO_CONTEXT (arg_info) = AA_let;
    INFO_LHS (arg_info) = LET_IDS (arg_node);

    // Mark LHS identifiers as local
    ids = LET_IDS (arg_node);
    while (ids != NULL) {
        DFMsetMaskEntrySet (INFO_LOCALMASK (arg_info), IDS_AVIS (ids));
        ids = IDS_NEXT (ids);
    }

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *EMAAprf (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
EMAAprf (node *arg_node, info *arg_info)
{
    node *lhs, *args;

    DBUG_ENTER ();

    lhs = INFO_LHS (arg_info);
    args = PRF_ARGS (arg_node);

    /**
     * Primitive functions in general yield a fresh result.
     * Hence, they won't return an alias of an argument.
     *
     * Exception: F_accu and F_prop_obj_in return an alias
     */
    switch (PRF_PRF (arg_node)) {

    case F_prop_obj_out:
    case F_prop_obj_in:
    case F_accu:
        MarkAllIdsAliasing (lhs, INFO_MASK (arg_info));
        break;

    /**
     * new_accu = unshare (old_accu, iv)
     * The output is always an alias of the first argument.
     */
    case F_unshare:
        MarkIdAliasing (PRF_ARG1 (arg_node), INFO_MASK (arg_info));
        break;

    case F_type_conv:
        MarkAllIdsAliasing (lhs, INFO_MASK (arg_info));
        MarkIdAliasing (PRF_ARG2 (arg_node), INFO_MASK (arg_info));
        break;

    case F_reshape_VxA:
        MarkIdAliasing (PRF_ARG3 (arg_node), INFO_MASK (arg_info));
        break;

    case F_reuse:
        MarkIdAliasing (PRF_ARG1 (arg_node), INFO_MASK (arg_info));
        break;

    /**
     * x1', .., xn' = guard (x1, .., xn, p1, .., pm)
     * Each xi' is an alias of xi.
     */
    case F_guard:
        MarkAllIdsAliasing (lhs, INFO_MASK (arg_info));
        while (lhs != NULL) {
            MarkIdAliasing (EXPRS_EXPR (args), INFO_MASK (arg_info));
            lhs = IDS_NEXT (lhs);
            args = EXPRS_NEXT (args);
        }
        break;

    case F_prefetch2device:
    case F_prefetch2host:
        MarkAllIdsAliasing (lhs, INFO_MASK (arg_info));
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

/******************************************************************************
 *
 * @fn node *EMAAwith (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
EMAAwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *EMAAwith2 (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
EMAAwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
    WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *EMAAwith3 (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
EMAAwith3 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    WITH3_OPERATIONS (arg_node) = TRAVdo (WITH3_OPERATIONS (arg_node), arg_info);
    WITH3_RANGES (arg_node) = TRAVdo (WITH3_RANGES (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *EMAAvardec (node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
EMAAvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (DFMtestMaskEntry (INFO_MASK (arg_info), VARDEC_AVIS (arg_node))) {
        DBUG_PRINT ("%s could not be unaliased", VARDEC_NAME (arg_node));
    }

    VARDEC_AVIS (arg_node) =
        SetAvisAlias (VARDEC_AVIS (arg_node),
                      DFMtestMaskEntry (INFO_MASK (arg_info),
                                        VARDEC_AVIS (arg_node)));

    VARDEC_NEXT (arg_node) = TRAVopt (VARDEC_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
