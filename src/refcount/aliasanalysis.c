/*
 *
 * $Log$
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

/**
 * CONTEXT enumeration: aa_context_t
 */
typedef enum { AA_undef, AA_begin, AA_end, AA_let, AA_ap } aa_context_t;

/*
 * INFO structure
 */
struct INFO {
    aa_context_t context;
    node *fundef;
    ids *lhs;
    DFMmask_t mask;
    node *funargs;
};

/*
 * INFO macros
 */
#define INFO_AA_CONTEXT(n) (n->context)
#define INFO_AA_FUNDEF(n) (n->fundef)
#define INFO_AA_LHS(n) (n->lhs)
#define INFO_AA_MASK(n) (n->mask)
#define INFO_AA_FUNARGS(n) (n->funargs)

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
    DBUG_ENTER ("EMAAAliasAnalysis");

    DBUG_PRINT ("EMAA", ("Starting alias analysis..."));

    act_tab = emaa_tab;

    syntax_tree = Trav (syntax_tree, NULL);

    DBUG_PRINT ("EMSRF", ("Alias analysis complete."));

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @node EliminateDupes( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
static node *
EliminateDupes (node *arg_node, info *arg_info)
{
    node *exprs;

    DBUG_ENTER ("EliminateDupes");

    exprs = arg_node;

    while (exprs != NULL) {
        node *rest = EXPRS_NEXT (exprs);
        while (rest != NULL) {
            if (ID_AVIS (EXPRS_EXPR (exprs)) == ID_AVIS (EXPRS_EXPR (rest))) {
                DFMSetMaskEntrySet (INFO_AA_MASK (arg_info), NULL,
                                    ID_VARDEC (EXPRS_EXPR (exprs)));
            }
            rest = EXPRS_NEXT (rest);
        }
        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_RETURN (arg_node);
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
    DBUG_ENTER ("EMAAap");

    if (FUNDEF_IS_LACFUN (AP_FUNDEF (arg_node))) {
        /*
         * Parameters must not be passed more than once to special functions
         */
        AP_ARGS (arg_node) = EliminateDupes (AP_ARGS (arg_node), arg_info);

        /*
         * Traverse special functions in order of appearance
         */
        if (AP_FUNDEF (arg_node) != INFO_AA_FUNDEF (arg_info)) {
            AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), arg_info);
        }
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
    if (FUNDEF_RETURN (AP_FUNDEF (arg_node)) != NULL) {
        ids *_ids;
        node *retexprs;

        _ids = INFO_AA_LHS (arg_info);
        retexprs = RETURN_EXPRS (FUNDEF_RETURN (AP_FUNDEF (arg_node)));

        while (_ids != NULL) {
            if (AVIS_ALIAS (ID_AVIS (EXPRS_EXPR (retexprs)))) {
                DFMSetMaskEntrySet (INFO_AA_MASK (arg_info), NULL, IDS_VARDEC (_ids));
            }
            retexprs = EXPRS_NEXT (retexprs);
            _ids = IDS_NEXT (_ids);
        }
    } else {
        ids *_ids;

        _ids = INFO_AA_LHS (arg_info);

        while (_ids != NULL) {
            DFMSetMaskEntrySet (INFO_AA_MASK (arg_info), NULL, IDS_VARDEC (_ids));
            _ids = IDS_NEXT (_ids);
        }
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

    case AA_end:
        ARG_ALIAS (arg_node) = DFMTestMaskEntry (INFO_AA_MASK (arg_info), NULL, arg_node);
        AVIS_ALIAS (ARG_AVIS (arg_node)) = TRUE;
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
    DBUG_ENTER ("EMAAfuncond");

    if ((DFMTestMaskEntry (INFO_AA_MASK (arg_info), NULL,
                           ID_VARDEC (EXPRS_EXPR (FUNCOND_THEN (arg_node)))))
        || (DFMTestMaskEntry (INFO_AA_MASK (arg_info), NULL,
                              ID_VARDEC (EXPRS_EXPR (FUNCOND_THEN (arg_node)))))) {
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

    if ((!FUNDEF_IS_LACFUN (arg_node)) || (arg_info != NULL)) {

        if (FUNDEF_BODY (arg_node) != NULL) {
            DFMmask_base_t maskbase;
            info *info;

            info = MakeInfo (arg_node);

            maskbase = DFMGenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));

            INFO_AA_MASK (info) = DFMGenMaskClear (maskbase);

            /*
             * Traverse function body
             */
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), info);

            /*
             * Traverse args to annotate ARG_ALIAS information
             */
            INFO_AA_CONTEXT (info) = AA_end;
            if (FUNDEF_ARGS (arg_node) != NULL) {
                FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), info);
            }

            INFO_AA_MASK (info) = DFMRemoveMask (INFO_AA_MASK (info));

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
     * F_accu returns an
     */
    switch (PRF_PRF (arg_node)) {

    case F_accu:
        _ids = INFO_AA_LHS (arg_info);
        while (_ids != NULL) {
            AVIS_ALIAS (IDS_AVIS (_ids)) = TRUE;
            _ids = IDS_NEXT (_ids);
        }
        break;

    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMAAreturn( node *arg_node, info *arg_info)
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
EMAAreturn (node *arg_node, info *arg_info)
{
    node *retexprs;

    DBUG_ENTER ("EMAAreturn");

    retexprs = RETURN_EXPRS (arg_node);

    /*
     * A variable must not be returned more than once.
     */
    retexprs = EliminateDupes (retexprs, arg_info);

    /*
     * Returned variables must not be function arguments
     */
    while (retexprs != NULL) {
        if (NODE_TYPE (ID_VARDEC (EXPRS_EXPR (retexprs))) == N_arg) {
            DFMSetMaskEntrySet (INFO_AA_MASK (arg_info), NULL,
                                ID_VARDEC (EXPRS_EXPR (retexprs)));
        }
        retexprs = EXPRS_NEXT (retexprs);
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
        AVIS_ALIAS (IDS_AVIS (INFO_AA_LHS (arg_info))) = TRUE;
        AVIS_ALIAS (ID_AVIS (NWITHOP_NEUTRAL (arg_node))) = TRUE;
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

    AVIS_ALIAS (VARDEC_AVIS (arg_node))
      = DFMTestMaskEntry (INFO_AA_MASK (arg_info), NULL, arg_node);

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*@}*/
