/*
 *
 * $Log$
 * Revision 1.1  2004/10/26 11:18:16  ktr
 * Initial revision
 *
 */

/**
 *
 * @defgroup ia Interface analysis
 * @ingroup rcp
 *
 * <pre>
 * </pre>
 * @{
 */

/**
 *
 * @file interfaceanalysis.c
 *
 *
 */
#define NEW_INFO

#define AVIS_ALIASMASK(n) (n->dfmask[0])

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
 * CONTEXT enumeration: ia_context_t
 */
typedef enum {
    IA_undef,
    IA_begin,
    IA_end,
    IA_let,
    IA_ap,
    IA_neutral,
    IA_funcond,
    IA_unqargs,
    IA_argnoalias
} ia_context_t;

/*
 * INFO structure
 */
struct INFO {
    ia_context_t context;
    node *fundef;
    ids *lhs;
    DFMmask_base_t maskbase;
    node *apfundef;
    node *apfunargs;
};

/*
 * INFO macros
 */
#define INFO_IA_CONTEXT(n) (n->context)
#define INFO_IA_FUNDEF(n) (n->fundef)
#define INFO_IA_LHS(n) (n->lhs)
#define INFO_IA_MASKBASE(n) (n->maskbase)
#define INFO_IA_APFUNDEF(n) (n->apfundef)
#define INFO_IA_APFUNARGS(n) (n->apfunargs)

/*
 * INFO functions
 */
static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_IA_CONTEXT (result) = IA_undef;
    INFO_IA_FUNDEF (result) = fundef;
    INFO_IA_LHS (result) = NULL;
    INFO_IA_MASKBASE (result) = NULL;
    INFO_IA_APFUNDEF (result) = NULL;
    INFO_IA_APFUNARGS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

/**
 * Convergence counter
 */
int unaliased;

/** <!--********************************************************************-->
 *
 * @fn node *EMIAInterfaceAnalysis( node *arg_node)
 *
 * @brief starting point of Interface Analysis traversal
 *
 * @param arg_node
 *
 * @return
 *
 *****************************************************************************/
node *
EMIAInterfaceAnalysis (node *syntax_tree)
{
    int counter;

    DBUG_ENTER ("EMIAInterfaceAnalysis");

    DBUG_PRINT ("EMIA", ("Starting interface alias analysis..."));

    act_tab = emia_tab;

    counter = 0;
    unaliased = -1;

    while (unaliased != 0) {
        unaliased = 0;
        syntax_tree = Trav (syntax_tree, NULL);
        counter += unaliased;
    }

    DBUG_PRINT ("EMIA", ("%d interfaces unaliased.", counter));
    DBUG_PRINT ("EMIA", ("Interface alias analysis complete."));

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/
static node *
SetArgAlias (node *arg, bool newval)
{
    DBUG_ENTER ("SetArgAlias");

    if (ARG_ALIAS (arg) && (!newval)) {
        ARG_ALIAS (arg) = FALSE;
        unaliased += 1;
    }

    DBUG_RETURN (arg);
}

static node *
InitializeRetAlias (node *fundef)
{
    DBUG_ENTER ("InitializeRetAlias");

    if (FUNDEF_RETALIAS (fundef) == NULL) {
        int retvals = TYGetProductSize (FUNDEF_RET_TYPE (fundef));
        DBUG_PRINT ("EMAA",
                    ("FUNDEF_RETALIAS initialized function: %s", FUNDEF_NAME (fundef)));
        while (retvals > 0) {
            FUNDEF_RETALIAS (fundef)
              = MakeNodelistNode (MakeBool (TRUE), FUNDEF_RETALIAS (fundef));
            retvals -= 1;
        }
    }

    DBUG_RETURN (fundef);
}

static bool
GetRetAlias (node *fundef, int num)
{
    nodelist *nl;

    DBUG_ENTER ("GetRetAlias");

    fundef = InitializeRetAlias (fundef);

    nl = FUNDEF_RETALIAS (fundef);
    while (num > 0) {
        nl = NODELIST_NEXT (nl);
        num -= 1;
    }

    DBUG_RETURN (BOOL_VAL (NODELIST_NODE (nl)));
}

static node *
SetRetAlias (node *fundef, int num, bool newval)
{
    nodelist *nl;

    DBUG_ENTER ("SetRetAlias");

    fundef = InitializeRetAlias (fundef);

    nl = FUNDEF_RETALIAS (fundef);
    while (num > 0) {
        nl = NODELIST_NEXT (nl);
        num -= 1;
    }

    if (BOOL_VAL (NODELIST_NODE (nl)) && (!newval)) {
        BOOL_VAL (NODELIST_NODE (nl)) = FALSE;
        unaliased += 1;
    }

    DBUG_RETURN (fundef);
}

static bool
IsUniqueNT (ntype *ty)
{
    bool res = FALSE;

    DBUG_ENTER ("IsUniqueNT");

    if (TYIsUser (ty)) {
        node *tdef = UTGetTdef (TYGetUserType (ty));
        DBUG_ASSERT (tdef != NULL, "Failed attempt to look up typedef");

        if (TYPEDEF_ATTRIB (tdef) == ST_unique) {
            res = TRUE;
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * Interface alias analysis traversal (emia_tab)
 *
 * prefix: EMIA
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @node EMIAap( node *arg_node, info *arg_info)
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
EMIAap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMIAap");

    /*
     * Check whether arguments could have been aliased
     */
    INFO_IA_CONTEXT (arg_info) = IA_ap;
    INFO_IA_APFUNDEF (arg_info) = AP_FUNDEF (arg_node);
    INFO_IA_APFUNARGS (arg_info) = FUNDEF_ARGS (AP_FUNDEF (arg_node));

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMIAarg( node *arg_node, info *arg_info)
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
EMIAarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMIAarg");

    switch (INFO_IA_CONTEXT (arg_info)) {
    case IA_begin:
        /*
         * Create ALIASMASK
         */
        AVIS_ALIASMASK (ARG_AVIS (arg_node))
          = DFMGenMaskClear (INFO_IA_MASKBASE (arg_info));

        DFMSetMaskEntrySet (AVIS_ALIASMASK (ARG_AVIS (arg_node)), NULL, arg_node);
        break;

    case IA_end:
        /*
         * Remove ALIASMASK
         */
        AVIS_ALIASMASK (ARG_AVIS (arg_node))
          = DFMRemoveMask (AVIS_ALIASMASK (ARG_AVIS (arg_node)));
        break;

    case IA_unqargs:
        if (AVIS_TYPE (ARG_AVIS (arg_node)) != NULL) {
            if (IsUniqueNT (TYGetScalar (AVIS_TYPE (ARG_AVIS (arg_node))))) {
                arg_node = SetArgAlias (arg_node, FALSE);
            }
        }
        break;

    case IA_argnoalias:
        arg_node = SetArgAlias (arg_node, FALSE);
        break;

    default:
        DBUG_ASSERT ((0), "Illegal context");
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMIAassign( node *arg_node, info *arg_info)
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
EMIAassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMIAassign");

    /*
     * Top-down traversal
     */
    INFO_IA_CONTEXT (arg_info) = IA_undef;
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMIAblock( node *arg_node, info *arg_info)
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
EMIAblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMIAblock");

    BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMIAcond( node *arg_node, info *arg_info)
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
EMIAcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMIAcond");

    COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);
    COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMIAfuncond( node *arg_node, info *arg_info)
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
EMIAfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMIAfuncond");

    INFO_IA_CONTEXT (arg_info) = IA_funcond;
    FUNCOND_THEN (arg_node) = Trav (FUNCOND_THEN (arg_node), arg_info);
    FUNCOND_ELSE (arg_node) = Trav (FUNCOND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMIAfundef( node *arg_node, info *arg_info)
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
EMIAfundef (node *arg_node, info *arg_info)
{
    int count;
    bool retalias;

    DBUG_ENTER ("EMIAfundef");

    arg_info = MakeInfo (arg_node);

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_IA_MASKBASE (arg_info)
          = DFMGenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));

        /*
         * Traverse vardecs/args to initialize ALIASMASKS
         */
        INFO_IA_CONTEXT (arg_info) = IA_begin;
        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
        }
        if (FUNDEF_VARDEC (arg_node) != NULL) {
            FUNDEF_VARDEC (arg_node) = Trav (FUNDEF_VARDEC (arg_node), arg_info);
        }

        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

        /*
         * Traverse vardecs/args to remove ALIASMASKS
         */
        INFO_IA_CONTEXT (arg_info) = IA_end;
        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
        }
        if (FUNDEF_VARDEC (arg_node) != NULL) {
            FUNDEF_VARDEC (arg_node) = Trav (FUNDEF_VARDEC (arg_node), arg_info);
        }

        INFO_IA_MASKBASE (arg_info) = DFMRemoveMaskBase (INFO_IA_MASKBASE (arg_info));
    }

    /*
     * Mark unique parameters and return values not ALIAS
     */
    INFO_IA_CONTEXT (arg_info) = IA_unqargs;
    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    for (count = 0; count < TYGetProductSize (FUNDEF_RET_TYPE (arg_node)); count++) {
        ntype *scl;
        scl = TYGetScalar (TYGetProductMember (FUNDEF_RET_TYPE (arg_node), count));
        if (IsUniqueNT (scl)) {
            arg_node = SetRetAlias (arg_node, count, FALSE);
        }
    }

    /*
     * If no return value is aliased, no argument is aliased
     */
    retalias = FALSE;

    for (count = 0; count < TYGetProductSize (FUNDEF_RET_TYPE (arg_node)); count++) {
        retalias = retalias || GetRetAlias (arg_node, count);
    }

    if (!retalias) {
        INFO_IA_CONTEXT (arg_info) = IA_argnoalias;
        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
        }
    }

    arg_info = FreeInfo (arg_info);

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMIAid( node *arg_node, info *arg_info)
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
EMIAid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMIAid");

    switch (INFO_IA_CONTEXT (arg_info)) {
    case IA_funcond:
    case IA_neutral:
    case IA_let:
        /*
         * ALIASING OPERATION a = b
         */
        DFMSetMaskOr (AVIS_ALIASMASK (IDS_AVIS (INFO_IA_LHS (arg_info))),
                      AVIS_ALIASMASK (ID_AVIS (arg_node)));
        break;

    case IA_ap:
        /*
         * Function application
         */
        if (ARG_ALIAS (INFO_IA_APFUNARGS (arg_info))) {
            int retcount = 0;
            ids *lhs = INFO_IA_LHS (arg_info);
            while (lhs != NULL) {
                if (GetRetAlias (INFO_IA_APFUNDEF (arg_info), retcount)) {
                    DFMSetMaskOr (AVIS_ALIASMASK (IDS_AVIS (lhs)),
                                  AVIS_ALIASMASK (ID_AVIS (arg_node)));
                }

                lhs = IDS_NEXT (lhs);
                retcount += 1;
            }
        }
        INFO_IA_APFUNARGS (arg_info) = ARG_NEXT (INFO_IA_APFUNARGS (arg_info));
        break;

    default:
        DBUG_ASSERT ((0), "Illegal context");
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMIAlet( node *arg_node, info *arg_info)
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
EMIAlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMIAlet");

    INFO_IA_CONTEXT (arg_info) = IA_let;
    INFO_IA_LHS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMIAreturn( node *arg_node, info *arg_info)
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
EMIAreturn (node *arg_node, info *arg_info)
{
    DFMmask_t retmask, argmask;
    node *retexprs;
    node *funargs;
    int retcount;

    DBUG_ENTER ("EMIAreturn");

    retmask = DFMGenMaskClear (INFO_IA_MASKBASE (arg_info));
    argmask = DFMGenMaskClear (INFO_IA_MASKBASE (arg_info));

    /*
     * 1. Arguments cannot return as aliases iff none of the return values
     * is an alias of an argument
     */
    retexprs = RETURN_EXPRS (arg_node);
    while (retexprs != NULL) {
        DFMSetMaskOr (retmask, AVIS_ALIASMASK (ID_AVIS (EXPRS_EXPR (retexprs))));
        retexprs = EXPRS_NEXT (retexprs);
    }

    funargs = FUNDEF_ARGS (INFO_IA_FUNDEF (arg_info));
    while (funargs != NULL) {
        if (!DFMTestMaskEntry (retmask, NULL, funargs)) {
            funargs = SetArgAlias (funargs, FALSE);
        }
        funargs = ARG_NEXT (funargs);
    }

    /*
     * 2. A return value is alias free iff
     *    - it is not an alias of a function argument
     *    - it is not an alias of another return value
     */
    funargs = FUNDEF_ARGS (INFO_IA_FUNDEF (arg_info));
    while (funargs != NULL) {
        DFMSetMaskEntrySet (argmask, NULL, funargs);
        funargs = ARG_NEXT (funargs);
    }

    retexprs = RETURN_EXPRS (arg_node);
    retcount = 0;
    while (retexprs != NULL) {
        node *retexprs2 = RETURN_EXPRS (arg_node);
        DFMSetMaskCopy (retmask, argmask);

        while (retexprs2 != NULL) {
            if (retexprs != retexprs2) {
                DFMSetMaskOr (retmask, AVIS_ALIASMASK (ID_AVIS (EXPRS_EXPR (retexprs2))));
            }
            retexprs2 = EXPRS_NEXT (retexprs2);
        }

        DFMSetMaskAnd (retmask, AVIS_ALIASMASK (ID_AVIS (EXPRS_EXPR (retexprs))));

        if (DFMTestMask (retmask) == 0) {
            INFO_IA_FUNDEF (arg_info)
              = SetRetAlias (INFO_IA_FUNDEF (arg_info), retcount, FALSE);
        }

        retexprs = EXPRS_NEXT (retexprs);
        retcount += 1;
    }

    retmask = DFMRemoveMask (retmask);
    argmask = DFMRemoveMask (argmask);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMIAvardec( node *arg_node, info *arg_info)
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
EMIAvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMIAvardec");

    switch (INFO_IA_CONTEXT (arg_info)) {
    case IA_begin:
        /*
         * Create ALIASMASK
         */
        AVIS_ALIASMASK (VARDEC_AVIS (arg_node))
          = DFMGenMaskClear (INFO_IA_MASKBASE (arg_info));

        DFMSetMaskEntrySet (AVIS_ALIASMASK (VARDEC_AVIS (arg_node)), NULL, arg_node);
        break;

    case IA_end:
        /*
         * Remove ALIASMASK
         */
        AVIS_ALIASMASK (VARDEC_AVIS (arg_node))
          = DFMRemoveMask (AVIS_ALIASMASK (VARDEC_AVIS (arg_node)));
        break;

    default:
        DBUG_ASSERT ((0), "Illegal context");
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMIAwith( node *arg_node, info *arg_info)
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
EMIAwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMIAwith");

    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMIAwith2( node *arg_node, info *arg_info)
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
EMIAwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMIAwith2");

    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMIAwithop( node *arg_node, info *arg_info)
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
EMIAwithop (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMIAwithop");

    switch (NWITHOP_TYPE (arg_node)) {
    case WO_foldfun:
    case WO_foldprf:
        INFO_IA_CONTEXT (arg_info) = IA_neutral;
        NWITHOP_NEUTRAL (arg_node) = Trav (NWITHOP_NEUTRAL (arg_node), arg_info);
        break;

    case WO_genarray:
    case WO_modarray:
        break;

    default:
        DBUG_ASSERT ((0), "Illegal withop!");
        break;
    }

    INFO_IA_LHS (arg_info) = IDS_NEXT (INFO_IA_LHS (arg_info));

    if (NWITHOP_NEXT (arg_node) != NULL) {
        NWITHOP_NEXT (arg_node) = Trav (NWITHOP_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/* @} */
