/*
 *
 * $Log$
 * Revision 1.8  2004/12/08 10:39:28  ktr
 * bugfix.
 *
 * Revision 1.7  2004/11/30 22:03:34  ktr
 * even more brushing
 *
 * Revision 1.6  2004/11/24 13:55:41  ktr
 * COMPILES!!!
 *
 * Revision 1.5  2004/11/24 12:57:15  jhb
 * imsop SACdevCamp 2k4
 *
 * Revision 1.4  2004/11/15 12:32:53  ktr
 * Interfaceanalysis now works with old types again until ... and uniqueness
 * issues are sorted out with the new TC
 *
 * Revision 1.3  2004/11/09 19:32:33  ktr
 * Bugfix
 *
 * Revision 1.2  2004/11/02 14:28:44  ktr
 * Better loop support.
 *
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
#include "interfaceanalysis.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "new_types.h"
#include "DataFlowMask.h"
#include "user_types.h"
#include "free.h"
#include "internal_lib.h"

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
    node *lhs;
    dfmask_base_t *maskbase;
    node *apfundef;
    node *apfunargs;
    bool retalias;
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
#define INFO_IA_RETALIAS(n) (n->retalias)

/*
 * INFO functions
 */
static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_IA_CONTEXT (result) = IA_undef;
    INFO_IA_FUNDEF (result) = fundef;
    INFO_IA_LHS (result) = NULL;
    INFO_IA_MASKBASE (result) = NULL;
    INFO_IA_APFUNDEF (result) = NULL;
    INFO_IA_APFUNARGS (result) = NULL;
    INFO_IA_RETALIAS (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

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
EMIAdoInterfaceAnalysis (node *syntax_tree)
{
    int counter;

    DBUG_ENTER ("EMIAInterfaceAnalysis");

    DBUG_PRINT ("EMIA", ("Starting interface alias analysis..."));

    counter = 0;
    unaliased = -1;

    while (unaliased != 0) {
        unaliased = 0;
        DBUG_PRINT ("EMIA", ("Starting interface analysis traversal..."));
        TRAVpush (TR_emia);
        syntax_tree = TRAVdo (syntax_tree, NULL);
        TRAVpop ();
        DBUG_PRINT ("EMIA", ("Interface analysis traversal complete."));
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

    if (ARG_ISALIASING (arg) && (!newval)) {
        ARG_ISALIASING (arg) = FALSE;
        unaliased += 1;
    }

    DBUG_RETURN (arg);
}

static bool
GetRetAlias (node *fundef, int num)
{
    bool res = TRUE;
    node *ret;

    DBUG_ENTER ("GetRetAlias");

    ret = FUNDEF_RETS (fundef);
    while ((ret != NULL) && (num > 0)) {
        ret = RET_NEXT (ret);
        num -= 1;
    }

    if (ret != NULL) {
        res = RET_ISALIASING (ret);
    }

    DBUG_RETURN (res);
}

static node *
SetRetAlias (node *fundef, int num, bool newval)
{
    node *ret;

    DBUG_ENTER ("SetRetAlias");

    ret = FUNDEF_RETS (fundef);

    while (num > 0) {
        ret = RET_NEXT (ret);
        num -= 1;
    }

    if (RET_ISALIASING (ret) && (!newval)) {
        RET_ISALIASING (ret) = FALSE;
        unaliased += 1;
    }

    DBUG_RETURN (fundef);
}

static bool
IsUniqueNT (ntype *ty)
{
    bool res = FALSE;

    DBUG_ENTER ("IsUniqueNT");

    if (TYisUser (ty)) {
        node *tdef = UTgetTdef (TYgetUserType (ty));
        DBUG_ASSERT (tdef != NULL, "Failed attempt to look up typedef");

        if (TYPEDEF_ISUNIQUE (tdef)) {
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
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
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
          = DFMgenMaskClear (INFO_IA_MASKBASE (arg_info));

        DFMsetMaskEntrySet (AVIS_ALIASMASK (ARG_AVIS (arg_node)), NULL,
                            ARG_AVIS (arg_node));
        break;

    case IA_end:
        /*
         * Remove ALIASMASK
         */
        AVIS_ALIASMASK (ARG_AVIS (arg_node))
          = DFMremoveMask (AVIS_ALIASMASK (ARG_AVIS (arg_node)));
        break;

    case IA_unqargs:
        if (AVIS_TYPE (ARG_AVIS (arg_node)) != NULL) {
            if (IsUniqueNT (TYgetScalar (AVIS_TYPE (ARG_AVIS (arg_node))))) {
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

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
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
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
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

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

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

    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

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

    /*
     * LOOP Functions: Alias of the return value does not depend on
     *                 the recursive application!
     */
    if (!FUNDEF_ISDOFUN (INFO_IA_FUNDEF (arg_info))) {
        FUNCOND_THEN (arg_node) = TRAVdo (FUNCOND_THEN (arg_node), arg_info);
    }
    FUNCOND_ELSE (arg_node) = TRAVdo (FUNCOND_ELSE (arg_node), arg_info);

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
    DBUG_ENTER ("EMIAfundef");

    arg_info = MakeInfo (arg_node);

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_IA_MASKBASE (arg_info)
          = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));

        /*
         * Traverse vardecs/args to initialize ALIASMASKS
         */
        INFO_IA_CONTEXT (arg_info) = IA_begin;
        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
        }
        if (FUNDEF_VARDEC (arg_node) != NULL) {
            FUNDEF_VARDEC (arg_node) = TRAVdo (FUNDEF_VARDEC (arg_node), arg_info);
        }

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        /*
         * Traverse vardecs/args to remove ALIASMASKS
         */
        INFO_IA_CONTEXT (arg_info) = IA_end;
        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
        }
        if (FUNDEF_VARDEC (arg_node) != NULL) {
            FUNDEF_VARDEC (arg_node) = TRAVdo (FUNDEF_VARDEC (arg_node), arg_info);
        }

        INFO_IA_MASKBASE (arg_info) = DFMremoveMaskBase (INFO_IA_MASKBASE (arg_info));
    }

    /*
     * Mark unique parameters and return values not ALIAS
     */
    INFO_IA_CONTEXT (arg_info) = IA_unqargs;
    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    INFO_IA_RETALIAS (arg_info) = FALSE;
    if (FUNDEF_RETS (arg_node) != NULL) {
        FUNDEF_RETS (arg_node) = TRAVdo (FUNDEF_RETS (arg_node), arg_info);
    }

    /*
     * If no return value is marked ALIASING, no argument can be ALIASED
     */
    if (!INFO_IA_RETALIAS (arg_info)) {
        INFO_IA_CONTEXT (arg_info) = IA_argnoalias;
        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
        }
    }

    arg_info = FreeInfo (arg_info);

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
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
        DFMsetMaskOr (AVIS_ALIASMASK (IDS_AVIS (INFO_IA_LHS (arg_info))),
                      AVIS_ALIASMASK (ID_AVIS (arg_node)));
        break;

    case IA_ap:
        /*
         * Function application
         */
        if (ARG_ISALIASING (INFO_IA_APFUNARGS (arg_info))) {
            int retcount = 0;
            node *lhs = INFO_IA_LHS (arg_info);
            while (lhs != NULL) {
                if (GetRetAlias (INFO_IA_APFUNDEF (arg_info), retcount)) {
                    DFMsetMaskOr (AVIS_ALIASMASK (IDS_AVIS (lhs)),
                                  AVIS_ALIASMASK (ID_AVIS (arg_node)));
                }

                lhs = IDS_NEXT (lhs);
                retcount += 1;
            }
        }

        if (TYPES_BASETYPE (ARG_TYPE (INFO_IA_APFUNARGS (arg_info))) != T_dots) {
            INFO_IA_APFUNARGS (arg_info) = ARG_NEXT (INFO_IA_APFUNARGS (arg_info));
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

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMIAret( node *arg_node, info *arg_info)
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
EMIAret (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMIAret");

    if (IsUniqueNT (TYgetScalar (RET_TYPE (arg_node)))) {
        if (RET_ISALIASING (arg_node)) {
            RET_ISALIASING (arg_node) = FALSE;
            unaliased += 1;
        }
    }

    INFO_IA_RETALIAS (arg_info)
      = INFO_IA_RETALIAS (arg_info) || RET_ISALIASING (arg_node);

    if (RET_NEXT (arg_node) != NULL) {
        RET_NEXT (arg_node) = TRAVdo (RET_NEXT (arg_node), arg_info);
    }

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
    dfmask_t *retmask, *argmask;
    node *retexprs;
    node *funargs;
    int retcount;

    DBUG_ENTER ("EMIAreturn");

    retmask = DFMgenMaskClear (INFO_IA_MASKBASE (arg_info));
    argmask = DFMgenMaskClear (INFO_IA_MASKBASE (arg_info));

    /*
     * 1. Arguments cannot return as aliases iff none of the return values
     * is an alias of an argument
     */
    retexprs = RETURN_EXPRS (arg_node);
    while (retexprs != NULL) {
        DFMsetMaskOr (retmask, AVIS_ALIASMASK (ID_AVIS (EXPRS_EXPR (retexprs))));
        retexprs = EXPRS_NEXT (retexprs);
    }

    funargs = FUNDEF_ARGS (INFO_IA_FUNDEF (arg_info));
    while (funargs != NULL) {
        if (!DFMtestMaskEntry (retmask, NULL, ARG_AVIS (funargs))) {
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
        DFMsetMaskEntrySet (argmask, NULL, ARG_AVIS (funargs));
        funargs = ARG_NEXT (funargs);
    }

    retexprs = RETURN_EXPRS (arg_node);
    retcount = 0;
    while (retexprs != NULL) {
        node *retexprs2 = RETURN_EXPRS (arg_node);
        DFMsetMaskCopy (retmask, argmask);

        while (retexprs2 != NULL) {
            if (retexprs != retexprs2) {
                DFMsetMaskOr (retmask, AVIS_ALIASMASK (ID_AVIS (EXPRS_EXPR (retexprs2))));
            }
            retexprs2 = EXPRS_NEXT (retexprs2);
        }

        DFMsetMaskAnd (retmask, AVIS_ALIASMASK (ID_AVIS (EXPRS_EXPR (retexprs))));

        if (DFMtestMask (retmask) == 0) {
            INFO_IA_FUNDEF (arg_info)
              = SetRetAlias (INFO_IA_FUNDEF (arg_info), retcount, FALSE);
        }

        retexprs = EXPRS_NEXT (retexprs);
        retcount += 1;
    }

    retmask = DFMremoveMask (retmask);
    argmask = DFMremoveMask (argmask);

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
          = DFMgenMaskClear (INFO_IA_MASKBASE (arg_info));

        DFMsetMaskEntrySet (AVIS_ALIASMASK (VARDEC_AVIS (arg_node)), NULL,
                            VARDEC_AVIS (arg_node));
        break;

    case IA_end:
        /*
         * Remove ALIASMASK
         */
        AVIS_ALIASMASK (VARDEC_AVIS (arg_node))
          = DFMremoveMask (AVIS_ALIASMASK (VARDEC_AVIS (arg_node)));
        break;

    default:
        DBUG_ASSERT ((0), "Illegal context");
        break;
    }

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
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

    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

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

    WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node EMIAfold( node *arg_node, info *arg_info)
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
EMIAfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMIAfold");

    INFO_IA_CONTEXT (arg_info) = IA_neutral;
    FOLD_NEUTRAL (arg_node) = TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);

    INFO_IA_LHS (arg_info) = IDS_NEXT (INFO_IA_LHS (arg_info));

    if (FOLD_NEXT (arg_node) != NULL) {
        FOLD_NEXT (arg_node) = TRAVdo (FOLD_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/* @} */
