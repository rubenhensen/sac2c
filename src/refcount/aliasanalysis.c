/*
 *
 * $Log$
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
#define NEW_INFO

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "new_types.h"
#include "DataFlowMask.h"
#include "user_types.h"

/**
 * CONTEXT enumeration: aa_context_t
 */
typedef enum { AA_undef, AA_begin, AA_let, AA_ap } aa_context_t;

/*
 * INFO structure
 */
struct INFO {
    aa_context_t context;
    node *fundef;
    ids *lhs;
    DFMmask_t mask;
    node *apargs;
    DFMmask_t apmask;
    node *funargs;
};

/*
 * INFO macros
 */
#define INFO_AA_CONTEXT(n) (n->context)
#define INFO_AA_FUNDEF(n) (n->fundef)
#define INFO_AA_LHS(n) (n->lhs)
#define INFO_AA_MASK(n) (n->mask)
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

    result = Malloc (sizeof (info));

    INFO_AA_CONTEXT (result) = AA_undef;
    INFO_AA_FUNDEF (result) = fundef;
    INFO_AA_LHS (result) = NULL;
    INFO_AA_MASK (result) = NULL;
    INFO_AA_APARGS (result) = NULL;
    INFO_AA_APMASK (result) = NULL;
    INFO_AA_FUNARGS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMAAAliasAnalysis( node *arg_node)
 *
 * @brief starting point of Alias Analysis traversal
 *
 * @param arg_node
 *
 * @return
 *
 *****************************************************************************/
node *
EMAAAliasAnalysis (node *syntax_tree)
{
    int unaliased;

    DBUG_ENTER ("EMAAAliasAnalysis");

    DBUG_PRINT ("EMAA", ("Starting alias analysis..."));

    act_tab = emaa_tab;

    unaliased = 0;
    syntax_tree = Trav (syntax_tree, NULL);

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

    if ((!AVIS_ALIAS (avis)) && newval) {
        DBUG_ASSERT ((0), "Alias analysis does not converge!");
    } else {
        if (AVIS_ALIAS (avis) && (!newval)) {
            AVIS_ALIAS (avis) = FALSE;
            unaliased += 1;
        }
    }
    DBUG_RETURN (avis);
}

static bool
GetRetAlias (node *fundef, int num)
{
    nodelist *nl;

    DBUG_ENTER ("GetRetAlias");

    nl = FUNDEF_RETALIAS (fundef);
    while (num > 0) {
        nl = NODELIST_NEXT (nl);
        num -= 1;
    }

    DBUG_RETURN (BOOL_VAL (NODELIST_NODE (nl)));
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
    ids *_ids;
    int argc;

    DBUG_ENTER ("EMAAap");

    if (FUNDEF_IS_CONDFUN (AP_FUNDEF (arg_node))) {
        /*
         * Traverse conditional functions in order of appearance
         */
        INFO_AA_APARGS (arg_info) = AP_ARGS (arg_node);
        AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), arg_info);
        INFO_AA_APARGS (arg_info) = NULL;
    }

    /*
     * Check whether arguments could have been aliased
     */
    INFO_AA_CONTEXT (arg_info) = AA_ap;
    INFO_AA_FUNARGS (arg_info) = FUNDEF_ARGS (AP_FUNDEF (arg_node));
    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }

    /*
     * Check whether return values are alias-free
     */
    argc = 0;
    _ids = INFO_AA_LHS (arg_info);

    while (_ids != NULL) {
        if (GetRetAlias (AP_FUNDEF (arg_node), argc)) {
            DFMSetMaskEntrySet (INFO_AA_MASK (arg_info), NULL, IDS_VARDEC (_ids));
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
        DFMSetMaskEntrySet (INFO_AA_MASK (arg_info), NULL, arg_node);
        break;

    default:
        DBUG_ASSERT ((0), "Illegal context!");
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
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
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
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

    if (NCODE_CBLOCK (arg_node) != NULL) {
        DFMmask_t oldmask;

        oldmask = INFO_AA_MASK (arg_info);
        INFO_AA_MASK (arg_info) = DFMGenMaskCopy (oldmask);

        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);

        INFO_AA_MASK (arg_info) = DFMRemoveMask (INFO_AA_MASK (arg_info));
        INFO_AA_MASK (arg_info) = oldmask;
    }

    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
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
    DFMmask_t thenmask, elsemask, oldmask;

    DBUG_ENTER ("EMAAcond");

    oldmask = INFO_AA_MASK (arg_info);
    thenmask = DFMGenMaskCopy (oldmask);
    elsemask = DFMGenMaskCopy (oldmask);

    INFO_AA_MASK (arg_info) = thenmask;
    COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);

    INFO_AA_MASK (arg_info) = elsemask;
    COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);

    DFMSetMaskOr (oldmask, thenmask);
    DFMSetMaskOr (oldmask, elsemask);

    thenmask = DFMRemoveMask (thenmask);
    elsemask = DFMRemoveMask (elsemask);
    INFO_AA_MASK (arg_info) = oldmask;

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

    thenid = EXPRS_EXPR (FUNCOND_THEN (arg_node));
    elseid = EXPRS_EXPR (FUNCOND_ELSE (arg_node));

    /*
     * Arguments must themselves not be aliases
     */
    if ((DFMTestMaskEntry (INFO_AA_MASK (arg_info), NULL, ID_VARDEC (thenid)))
        || (DFMTestMaskEntry (INFO_AA_MASK (arg_info), NULL, ID_VARDEC (elseid)))) {

        DFMSetMaskEntrySet (INFO_AA_MASK (arg_info), NULL,
                            IDS_VARDEC (INFO_AA_LHS (arg_info)));
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

    if ((!FUNDEF_IS_CONDFUN (arg_node)) || (arg_info != NULL)) {

        if (FUNDEF_BODY (arg_node) != NULL) {
            info *info;
            DFMmask_base_t maskbase;

            info = MakeInfo (arg_node);

            maskbase = DFMGenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));

            INFO_AA_MASK (info) = DFMGenMaskClear (maskbase);

            if (arg_info != NULL) {
                INFO_AA_APARGS (info) = INFO_AA_APARGS (arg_info);
                INFO_AA_APMASK (info) = INFO_AA_MASK (arg_info);
            }

            /*
             * Traverse function args to mark them AVIS_ALIAS
             */
            INFO_AA_CONTEXT (info) = AA_begin;
            if (FUNDEF_ARGS (arg_node) != NULL) {
                FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), info);
            }

            INFO_AA_APARGS (info) = NULL;
            INFO_AA_APMASK (info) = NULL;

            /*
             * Traverse function body
             */
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), info);

            INFO_AA_MASK (info) = DFMRemoveMask (INFO_AA_MASK (info));

            maskbase = DFMRemoveMaskBase (maskbase);

            info = FreeInfo (info);
        }
    }

    if (arg_info == NULL) {
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
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
        DFMSetMaskEntrySet (INFO_AA_MASK (arg_info), NULL, ID_VARDEC (arg_node));
        DFMSetMaskEntrySet (INFO_AA_MASK (arg_info), NULL,
                            IDS_VARDEC (INFO_AA_LHS (arg_info)));
        break;

    case AA_ap:
        if (INFO_AA_FUNARGS (arg_info) != NULL) {
            if (ARG_ALIAS (INFO_AA_FUNARGS (arg_info))) {
                DFMSetMaskEntrySet (INFO_AA_MASK (arg_info), NULL, ID_VARDEC (arg_node));
            }
            INFO_AA_FUNARGS (arg_info) = ARG_NEXT (INFO_AA_FUNARGS (arg_info));
        } else {
            DFMSetMaskEntrySet (INFO_AA_MASK (arg_info), NULL, ID_VARDEC (arg_node));
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
    DBUG_ENTER ("EMAAlet");

    INFO_AA_CONTEXT (arg_info) = AA_let;
    INFO_AA_LHS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

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
    ids *_ids;

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
        _ids = INFO_AA_LHS (arg_info);
        while (_ids != NULL) {
            DFMSetMaskEntrySet (INFO_AA_MASK (arg_info), NULL, IDS_VARDEC (_ids));
            _ids = IDS_NEXT (_ids);
        }
        break;

    case F_reuse:
        if (NODE_TYPE (PRF_ARG1 (arg_node)) == N_id) {
            DFMSetMaskEntrySet (INFO_AA_MASK (arg_info), NULL,
                                ID_VARDEC (PRF_ARG1 (arg_node)));
        } else {
            DFMSetMaskEntrySet (INFO_AA_MASK (arg_info), NULL,
                                ID_VARDEC (PRF_ARG2 (arg_node)));
        }
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

    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);
    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

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

    NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);
    NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMAAwithop( node *arg_node, info *arg_info)
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
EMAAwithop (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMAAap");

    switch (NWITHOP_TYPE (arg_node)) {
    case WO_foldfun:
    case WO_foldprf:
        DFMSetMaskEntrySet (INFO_AA_MASK (arg_info), NULL,
                            IDS_VARDEC (INFO_AA_LHS (arg_info)));
        DFMSetMaskEntrySet (INFO_AA_MASK (arg_info), NULL,
                            ID_VARDEC (NWITHOP_NEUTRAL (arg_node)));
        break;

    case WO_genarray:
    case WO_modarray:
        break;

    default:
        DBUG_ASSERT ((0), "Illegal withop!");
        break;
    }

    if (NWITHOP_NEXT (arg_node) != NULL) {
        NWITHOP_NEXT (arg_node) = Trav (NWITHOP_NEXT (arg_node), arg_info);
    }

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

    VARDEC_AVIS (arg_node)
      = SetAvisAlias (VARDEC_AVIS (arg_node),
                      DFMTestMaskEntry (INFO_AA_MASK (arg_info), NULL, arg_node));

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*@}*/
