/*
 *
 * $Log$
 * Revision 1.11  2005/08/20 12:09:09  ktr
 * typeconv introduces aliasing
 *
 * Revision 1.10  2004/12/09 21:09:26  ktr
 * bugfix roundup
 *
 * Revision 1.9  2004/11/23 15:04:02  ktr
 * COMPILES!!!
 *
 * Revision 1.8  2004/11/21 18:07:02  ktr
 * the big 2004 codebrushing event
 *
 * Revision 1.7  2004/11/19 15:42:41  ktr
 * Support for F_alloc_or_reshape added.
 *
 * Revision 1.6  2004/11/12 10:18:49  ktr
 * Alias property of inner identifiers is no longer ignored.
 *
 * Revision 1.5  2004/11/02 14:31:11  ktr
 * Aliasanalysis is now performed seperately for each branch of a
 * conditional.
 *
 * Revision 1.4  2004/10/26 11:19:38  ktr
 * Intermediate update for stephan
 *
 * Revision 1.3  2004/10/22 17:10:04  ktr
 * moved a declaration.
 *
 * Revision 1.2  2004/10/22 15:38:19  ktr
 * Ongoing implementation.
 *
 * Revision 1.1  2004/10/15 09:05:08  ktr
 * Initial revision
 *
 */

/**
 *
 * @defgroup aa Alias analysis
 * @ingroup rcp
 *
 * <pre>
 * </pre>
 * @{
 */

/**
 *
 * @file aliasanalysis.c
 *
 *
 */
#include "aliasanalysis.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "new_types.h"
#include "DataFlowMask.h"
#include "user_types.h"
#include "internal_lib.h"
#include <string.h>

/**
 * CONTEXT enumeration: aa_context
 */
typedef enum { AA_undef, AA_begin, AA_end, AA_let, AA_ap } aa_context;

/*
 * INFO structure
 */
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

/*
 * INFO macros
 */
#define INFO_AA_CONTEXT(n) (n->context)
#define INFO_AA_FUNDEF(n) (n->fundef)
#define INFO_AA_LHS(n) (n->lhs)
#define INFO_AA_MASK(n) (n->mask)
#define INFO_AA_LOCALMASK(n) (n->localmask)
#define INFO_AA_APMASK(n) (n->apmask)
#define INFO_AA_APARGS(n) (n->apargs)
#define INFO_AA_FUNARGS(n) (n->funargs)

/*
 * Convergence counter
 */
int unaliased;

/*
 * INFO functions
 */
static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_AA_CONTEXT (result) = AA_undef;
    INFO_AA_FUNDEF (result) = fundef;
    INFO_AA_LHS (result) = NULL;
    INFO_AA_MASK (result) = NULL;
    INFO_AA_LOCALMASK (result) = NULL;
    INFO_AA_APARGS (result) = NULL;
    INFO_AA_APMASK (result) = NULL;
    INFO_AA_FUNARGS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

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
    DBUG_ENTER ("EMAAdoAliasAnalysis");

    DBUG_PRINT ("EMAA", ("Starting alias analysis..."));

    unaliased = 0;
    TRAVpush (TR_emaa);
    syntax_tree = TRAVdo (syntax_tree, NULL);
    TRAVpop ();

    DBUG_PRINT ("EMAA", ("%d variables unaliased.", unaliased));
    DBUG_PRINT ("EMAA", ("Alias analysis complete."));

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/
static node *
SetAvisAlias (node *avis, bool newval)
{
    DBUG_ENTER ("SetAvisAlias");

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

    DBUG_ENTER ("GetRetAlias");

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
    DBUG_ENTER ("MarkAllIdsAliasing");

    while (ids != NULL) {
        DFMsetMaskEntrySet (mask, NULL, IDS_AVIS (ids));
        ids = IDS_NEXT (ids);
    }

    DBUG_VOID_RETURN;
}

static void
MarkIdAliasing (node *id, dfmask_t *mask)
{
    DBUG_ENTER ("MarkIdAliasing");

    if (NODE_TYPE (id) == N_id) {
        DFMsetMaskEntrySet (mask, NULL, ID_AVIS (id));
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Alias analysis traversal (emaa_tab)
 *
 * prefix: EMAA
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @node EMAAap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMAAap (node *arg_node, info *arg_info)
{
    node *_ids;
    int argc;

    DBUG_ENTER ("EMAAap");

    if (FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node))) {
        /*
         * Traverse conditional functions in order of appearance
         */
        INFO_AA_APARGS (arg_info) = AP_ARGS (arg_node);
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
        INFO_AA_APARGS (arg_info) = NULL;
    }

    /*
     * Check whether arguments could have been aliased
     */
    INFO_AA_CONTEXT (arg_info) = AA_ap;
    INFO_AA_FUNARGS (arg_info) = FUNDEF_ARGS (AP_FUNDEF (arg_node));
    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    }

    /*
     * Check whether return values are alias-free
     */
    argc = 0;
    _ids = INFO_AA_LHS (arg_info);

    while (_ids != NULL) {
        if (GetRetAlias (AP_FUNDEF (arg_node), argc)) {
            DFMsetMaskEntrySet (INFO_AA_MASK (arg_info), NULL, IDS_AVIS (_ids));
        }
        _ids = IDS_NEXT (_ids);
        argc += 1;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMAAarg( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMAAarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMAAarg");

    switch (INFO_AA_CONTEXT (arg_info)) {

    case AA_begin:
        if (INFO_AA_APARGS (arg_info) != NULL) {
            node *id = EXPRS_EXPR (INFO_AA_APARGS (arg_info));
            if (DFMtestMaskEntry (INFO_AA_APMASK (arg_info), NULL, ID_AVIS (id))) {

                DFMsetMaskEntrySet (INFO_AA_MASK (arg_info), NULL, ARG_AVIS (arg_node));
            }
            INFO_AA_APARGS (arg_info) = EXPRS_NEXT (INFO_AA_APARGS (arg_info));
        } else {
            if (AVIS_ISALIAS (ARG_AVIS (arg_node))) {
                DFMsetMaskEntrySet (INFO_AA_MASK (arg_info), NULL, ARG_AVIS (arg_node));
            }
        }
        break;

    case AA_end:
        ARG_AVIS (arg_node) = SetAvisAlias (ARG_AVIS (arg_node),
                                            DFMtestMaskEntry (INFO_AA_MASK (arg_info),
                                                              NULL, ARG_AVIS (arg_node)));
        break;

    default:
        DBUG_ASSERT ((0), "Illegal context!");
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMAAassign( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMAAassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMAAassign");

    INFO_AA_CONTEXT (arg_info) = AA_undef;
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMAAcode( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMAAcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMAAcode");

    if (CODE_CBLOCK (arg_node) != NULL) {
        dfmask_t *oldmask, *oldlocalmask;

        oldmask = INFO_AA_MASK (arg_info);
        oldlocalmask = INFO_AA_LOCALMASK (arg_info);

        INFO_AA_MASK (arg_info) = DFMgenMaskCopy (oldmask);
        INFO_AA_LOCALMASK (arg_info) = DFMgenMaskCopy (oldlocalmask);
        DFMsetMaskClear (INFO_AA_LOCALMASK (arg_info));

        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);

        DFMsetMaskAnd (INFO_AA_MASK (arg_info), INFO_AA_LOCALMASK (arg_info));
        DFMsetMaskOr (oldmask, INFO_AA_MASK (arg_info));
        DFMsetMaskOr (oldlocalmask, INFO_AA_LOCALMASK (arg_info));

        INFO_AA_MASK (arg_info) = DFMremoveMask (INFO_AA_MASK (arg_info));
        INFO_AA_LOCALMASK (arg_info) = DFMremoveMask (INFO_AA_LOCALMASK (arg_info));

        INFO_AA_LOCALMASK (arg_info) = oldlocalmask;
        INFO_AA_MASK (arg_info) = oldmask;
    }

    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMAAcond( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMAAcond (node *arg_node, info *arg_info)
{
    dfmask_t *thenmask, *elsemask, *oldmask;

    DBUG_ENTER ("EMAAcond");

    oldmask = INFO_AA_MASK (arg_info);
    thenmask = DFMgenMaskCopy (oldmask);
    elsemask = DFMgenMaskCopy (oldmask);

    INFO_AA_MASK (arg_info) = thenmask;
    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);

    INFO_AA_MASK (arg_info) = elsemask;
    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    DFMsetMaskOr (oldmask, thenmask);
    DFMsetMaskOr (oldmask, elsemask);

    thenmask = DFMremoveMask (thenmask);
    elsemask = DFMremoveMask (elsemask);
    INFO_AA_MASK (arg_info) = oldmask;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMAAfold( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMAAfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMAAfold");

    DFMsetMaskEntrySet (INFO_AA_MASK (arg_info), NULL, IDS_AVIS (INFO_AA_LHS (arg_info)));
    DFMsetMaskEntrySet (INFO_AA_MASK (arg_info), NULL, ID_AVIS (FOLD_NEUTRAL (arg_node)));

    if (FOLD_NEXT (arg_node) != NULL) {
        INFO_AA_LHS (arg_info) = IDS_NEXT (INFO_AA_LHS (arg_info));
        FOLD_NEXT (arg_node) = TRAVdo (FOLD_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMAAgenarray( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMAAgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMAAgenarray");

    if (GENARRAY_NEXT (arg_node) != NULL) {
        INFO_AA_LHS (arg_info) = IDS_NEXT (INFO_AA_LHS (arg_info));
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMAAmodarray( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMAAmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMAAmodarray");

    if (MODARRAY_NEXT (arg_node) != NULL) {
        INFO_AA_LHS (arg_info) = IDS_NEXT (INFO_AA_LHS (arg_info));
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMAAfuncond( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMAAfuncond (node *arg_node, info *arg_info)
{
    node *thenid, *elseid;

    DBUG_ENTER ("EMAAfuncond");

    thenid = FUNCOND_THEN (arg_node);
    elseid = FUNCOND_ELSE (arg_node);

    /*
     * Arguments must themselves not be aliases
     */
    if ((DFMtestMaskEntry (INFO_AA_MASK (arg_info), NULL, ID_AVIS (thenid)))
        || (DFMtestMaskEntry (INFO_AA_MASK (arg_info), NULL, ID_AVIS (elseid)))) {

        DFMsetMaskEntrySet (INFO_AA_MASK (arg_info), NULL,
                            IDS_AVIS (INFO_AA_LHS (arg_info)));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMAAfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMAAfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMAAfundef");

    DBUG_PRINT ("EMAA", ("Traversing function %s", FUNDEF_NAME (arg_node)));

    if ((!FUNDEF_ISCONDFUN (arg_node)) || (arg_info != NULL)) {

        if (FUNDEF_BODY (arg_node) != NULL) {
            info *info;
            dfmask_base_t *maskbase;

            info = MakeInfo (arg_node);

            maskbase = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));

            INFO_AA_MASK (info) = DFMgenMaskClear (maskbase);
            INFO_AA_LOCALMASK (info) = DFMgenMaskClear (maskbase);

            if (arg_info != NULL) {
                INFO_AA_APARGS (info) = INFO_AA_APARGS (arg_info);
                INFO_AA_APMASK (info) = INFO_AA_MASK (arg_info);
            }

            /*
             * Traverse function args to mark them as ALIAS in MASK
             */
            INFO_AA_CONTEXT (info) = AA_begin;
            if (FUNDEF_ARGS (arg_node) != NULL) {
                FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), info);
            }

            /*
             * Aliased CONDFUN parameters must be marked as ALIAS
             */
            if (arg_info != NULL) {
                node *funargs = FUNDEF_ARGS (arg_node);
                node *apargs = INFO_AA_APARGS (arg_info);

                while (funargs != NULL) {
                    node *apargs2 = INFO_AA_APARGS (arg_info);

                    while (apargs2 != NULL) {
                        if (apargs != apargs2) {
                            if (ID_AVIS (EXPRS_EXPR (apargs))
                                == ID_AVIS (EXPRS_EXPR (apargs2))) {
                                DFMsetMaskEntrySet (INFO_AA_MASK (info), NULL,
                                                    ARG_AVIS (funargs));
                            }
                        }
                        apargs2 = EXPRS_NEXT (apargs2);
                    }

                    funargs = ARG_NEXT (funargs);
                    apargs = EXPRS_NEXT (apargs);
                }
            }

            INFO_AA_APARGS (info) = NULL;
            INFO_AA_APMASK (info) = NULL;

            /*
             * Traverse function body
             */
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);

            /*
             * Traverse args to annotate AVIS_ISALIAS
             */
            INFO_AA_CONTEXT (info) = AA_end;
            if (FUNDEF_ARGS (arg_node) != NULL) {
                FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), info);
            }

            /*
             * Clean up
             */
            INFO_AA_MASK (info) = DFMremoveMask (INFO_AA_MASK (info));
            INFO_AA_LOCALMASK (info) = DFMremoveMask (INFO_AA_LOCALMASK (info));

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
 * @node EMAAid( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMAAid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMAAid");

    switch (INFO_AA_CONTEXT (arg_info)) {
    case AA_let:
        /*
         * ALIASING OPERATION a = b
         * mark a and b as aliases
         */
        DFMsetMaskEntrySet (INFO_AA_MASK (arg_info), NULL, ID_AVIS (arg_node));
        DFMsetMaskEntrySet (INFO_AA_MASK (arg_info), NULL,
                            IDS_AVIS (INFO_AA_LHS (arg_info)));
        break;

    case AA_ap:
        if (INFO_AA_FUNARGS (arg_info) != NULL) {
            if (ARG_ISALIASING (INFO_AA_FUNARGS (arg_info))) {
                DFMsetMaskEntrySet (INFO_AA_MASK (arg_info), NULL, ID_AVIS (arg_node));
            }
            INFO_AA_FUNARGS (arg_info) = ARG_NEXT (INFO_AA_FUNARGS (arg_info));
        } else {
            DFMsetMaskEntrySet (INFO_AA_MASK (arg_info), NULL, ID_AVIS (arg_node));
        }
        break;

    default:
        DBUG_ASSERT ((0), "Illegal context");
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMAAicm( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMAAicm (node *arg_node, info *arg_info)
{
    char *name;

    DBUG_ENTER ("EMAAicm");

    name = ICM_NAME (arg_node);

    if ((strstr (name, "USE_GENVAR_OFFSET") != NULL)
        || (strstr (name, "VECT2OFFSET") != NULL)
        || (strstr (name, "IDXS2OFFSET") != NULL)) {
        DFMsetMaskEntrySet (INFO_AA_LOCALMASK (arg_info), NULL,
                            ID_AVIS (ICM_ARG1 (arg_node)));
    } else {
        DBUG_ASSERT ((0), "Unknown ICM found during EMRI");
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMAAlet( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMAAlet (node *arg_node, info *arg_info)
{
    node *_ids;
    DBUG_ENTER ("EMAAlet");

    INFO_AA_CONTEXT (arg_info) = AA_let;
    INFO_AA_LHS (arg_info) = LET_IDS (arg_node);

    /*
     * Mark LHS identifiers as local
     */
    _ids = LET_IDS (arg_node);
    while (_ids != NULL) {
        DFMsetMaskEntrySet (INFO_AA_LOCALMASK (arg_info), NULL, IDS_AVIS (_ids));
        _ids = IDS_NEXT (_ids);
    }

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMAAprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMAAprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMAAprf");

    /*
     * Primitive functions in general yield a fresh result.
     * Hence, they won't return an alias of an argument.
     *
     * Exception: F_accu
     * F_accu returns an alias
     */
    switch (PRF_PRF (arg_node)) {

    case F_accu:
        MarkAllIdsAliasing (INFO_AA_LHS (arg_info), INFO_AA_MASK (arg_info));
        break;

    case F_type_conv:
        MarkAllIdsAliasing (INFO_AA_LHS (arg_info), INFO_AA_MASK (arg_info));
        MarkIdAliasing (PRF_ARG2 (arg_node), INFO_AA_MASK (arg_info));
        break;

    case F_reshape:
        MarkIdAliasing (PRF_ARG3 (arg_node), INFO_AA_MASK (arg_info));
        break;

    case F_reuse:
        MarkIdAliasing (PRF_ARG1 (arg_node), INFO_AA_MASK (arg_info));
        break;

    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMAAwith( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMAAwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMAAwith");

    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMAAwith2( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMAAwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMAAwith2");

    WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
    WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMAAvardec( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMAAvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMAAvardec");

    if (DFMtestMaskEntry (INFO_AA_MASK (arg_info), NULL, VARDEC_AVIS (arg_node))) {
        DBUG_PRINT ("EMAA", ("%s could not be unaliased", VARDEC_NAME (arg_node)));
    }

    VARDEC_AVIS (arg_node)
      = SetAvisAlias (VARDEC_AVIS (arg_node),
                      DFMtestMaskEntry (INFO_AA_MASK (arg_info), NULL,
                                        VARDEC_AVIS (arg_node)));

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*@}*/
