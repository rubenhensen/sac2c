/*
 *
 * $Log$
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
typedef enum {
    AA_undef,
    AA_begin,
    AA_end,
    AA_let,
    AA_ap,
    AA_unqargs,
    AA_argnoalias
} aa_context_t;

/*
 * INFO structure
 */
struct INFO {
    aa_context_t context;
    node *fundef;
    ids *lhs;
    DFMmask_t mask;
    DFMmask_t apmask;
    node *funargs;
    node *apargs;
};

/*
 * INFO macros
 */
#define INFO_AA_CONTEXT(n) (n->context)
#define INFO_AA_FUNDEF(n) (n->fundef)
#define INFO_AA_LHS(n) (n->lhs)
#define INFO_AA_MASK(n) (n->mask)
#define INFO_AA_APMASK(n) (n->apmask)
#define INFO_AA_FUNARGS(n) (n->funargs)
#define INFO_AA_APARGS(n) (n->apargs)

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
    INFO_AA_APMASK (result) = NULL;
    INFO_AA_FUNARGS (result) = NULL;
    INFO_AA_APARGS (result) = NULL;

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
    int counter;

    DBUG_ENTER ("EMAAAliasAnalysis");

    DBUG_PRINT ("EMAA", ("Starting alias analysis..."));

    act_tab = emaa_tab;

    counter = 0;
    unaliased = -1;

    while (unaliased != 0) {
        unaliased = 0;
        syntax_tree = Trav (syntax_tree, NULL);
        counter += unaliased;
    }

    DBUG_PRINT ("EMAA", ("%d variables unaliased.", counter));
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

static node *
SetArgAlias (node *arg, bool newval)
{
    DBUG_ENTER ("SetArgAlias");

    if ((!ARG_ALIAS (arg)) && newval) {
        /*     DBUG_ASSERT( (0), "Alias analysis does not converge!"); */
    } else {
        if (ARG_ALIAS (arg) && (!newval)) {
            ARG_ALIAS (arg) = FALSE;
        }
    }
    DBUG_RETURN (arg);
}

static node *
InitializeRetAlias (node *fundef)
{
    DBUG_ENTER ("InitializeRetAlias");

    if (FUNDEF_RETALIAS (fundef) == NULL) {
        DBUG_PRINT ("EMAA",
                    ("FUNDEF_RETALIAS initialized function: %s", FUNDEF_NAME (fundef)));
        int retvals = TYGetProductSize (FUNDEF_RET_TYPE (fundef));
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

    if ((!BOOL_VAL (NODELIST_NODE (nl))) && newval) {
        /*     DBUG_ASSERT( (0), "Alias analysis does not converge!"); */
    } else {
        if (BOOL_VAL (NODELIST_NODE (nl)) && (!newval)) {
            BOOL_VAL (NODELIST_NODE (nl)) = FALSE;
        }
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

/** <!--********************************************************************-->
 *
 * @node EliminateArg( node *arg_node, info *arg_info)
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
EliminateArg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EliminateArg");

    if (NODE_TYPE (ID_VARDEC (arg_node)) == N_arg) {
        DFMSetMaskEntrySet (INFO_AA_MASK (arg_info), NULL, ID_VARDEC (arg_node));
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
    ids *_ids;
    int argc;

    DBUG_ENTER ("EMAAap");

    if (FUNDEF_IS_CONDFUN (AP_FUNDEF (arg_node))) {
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

    case AA_end:
        arg_node = SetArgAlias (arg_node, DFMTestMaskEntry (INFO_AA_MASK (arg_info), NULL,
                                                            arg_node));

        if (FUNDEF_IS_CONDFUN (INFO_AA_FUNDEF (arg_info))) {
            node *id = EXPRS_EXPR (INFO_AA_APARGS (arg_info));
            bool apargalias
              = DFMTestMaskEntry (INFO_AA_APMASK (arg_info), NULL, ID_VARDEC (id));

            ARG_AVIS (arg_node)
              = SetAvisAlias (ARG_AVIS (arg_node), ARG_ALIAS (arg_node) || apargalias);

            INFO_AA_APARGS (arg_info) = EXPRS_NEXT (INFO_AA_APARGS (arg_info));
        }
        break;

    case AA_unqargs:
        if (AVIS_TYPE (ARG_AVIS (arg_node)) != NULL) {
            if (IsUniqueNT (TYGetScalar (AVIS_TYPE (ARG_AVIS (arg_node))))) {
                arg_node = SetArgAlias (arg_node, FALSE);
            }
        }
        break;

    case AA_argnoalias:
        arg_node = SetArgAlias (arg_node, FALSE);
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
     * Arguments must not be function arguments
     */
    thenid = EliminateArg (thenid, arg_info);
    elseid = EliminateArg (elseid, arg_info);

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
        info *info;
        int count;
        bool retalias;

        info = MakeInfo (arg_node);

        if (FUNDEF_BODY (arg_node) != NULL) {
            DFMmask_base_t maskbase;
            node *retexprs;
            int retcount;

            maskbase = DFMGenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));

            INFO_AA_MASK (info) = DFMGenMaskClear (maskbase);

            /*
             * Traverse function body
             */
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), info);

            /*
             * Annotate RETALIAS information
             */
            retexprs = RETURN_EXPRS (ASSIGN_RHS (FUNDEF_RETURN (arg_node)));
            retcount = 0;
            while (retexprs != NULL) {
                arg_node = SetRetAlias (arg_node, retcount,
                                        AVIS_ALIAS (ID_AVIS (EXPRS_EXPR (retexprs))));

                retexprs = EXPRS_NEXT (retexprs);
                retcount += 1;
            }

            /*
             * Traverse args to annotate ARG_ALIAS information
             */
            if (FUNDEF_IS_CONDFUN (arg_node)) {
                DBUG_ASSERT (FUNDEF_USED (arg_node) == 1, "CONDFUN used more than once!");
                INFO_AA_APARGS (info)
                  = AP_ARGS (ASSIGN_RHS (NODELIST_NODE (FUNDEF_EXT_ASSIGNS (arg_node))));
                INFO_AA_APMASK (info) = INFO_AA_MASK (arg_info);
            }

            INFO_AA_CONTEXT (info) = AA_end;
            if (FUNDEF_ARGS (arg_node) != NULL) {
                FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), info);
            }

            INFO_AA_MASK (info) = DFMRemoveMask (INFO_AA_MASK (info));
        }

        /*
         * Mark unique parameters and return values not ALIAS
         */
        INFO_AA_CONTEXT (info) = AA_unqargs;
        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), info);
        }

        for (count = 0; count < TYGetProductSize (FUNDEF_RET_TYPE (arg_node)); count++) {
            ntype *scl;
            scl = TYGetScalar (TYGetProductMember (FUNDEF_RET_TYPE (arg_node), count));
            if (IsUniqueNT (scl)) {
                arg_node = SetRetAlias (arg_node, count, FALSE);
            }
        }

        /*
         * When no return value is aliased, no argument is aliased
         */
        retalias = FALSE;

        for (count = 0; count < TYGetProductSize (FUNDEF_RET_TYPE (arg_node)); count++) {
            retalias = retalias || GetRetAlias (arg_node, count);
        }

        if (!retalias) {
            INFO_AA_CONTEXT (info) = AA_argnoalias;
            if (FUNDEF_ARGS (arg_node) != NULL) {
                FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), info);
            }
        }

        info = FreeInfo (info);
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
            DFMSetMaskEntrySet (INFO_AA_MASK (arg_info), NULL, IDS_VARDEC (_ids));
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
        EXPRS_EXPR (retexprs) = EliminateArg (EXPRS_EXPR (retexprs), arg_info);
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
