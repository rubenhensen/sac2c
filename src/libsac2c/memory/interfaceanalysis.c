/** <!--********************************************************************-->
 *
 * @defgroup ia Interface analysis
 *
 * @ingroup mm
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file interfaceanalysis.c
 *
 * Prefix: IA
 *
 *****************************************************************************/
#include "interfaceanalysis.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#define DBUG_PREFIX "IA"
#include "debug.h"

#include "print.h"
#include "new_types.h"
#include "DataFlowMask.h"
#include "free.h"
#include "str.h"
#include "memory.h"
#include "type_utils.h"

/**
 * Convergence counter
 */
static int unaliased;

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

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    ia_context_t context;
    node *fundef;
    node *lhs;
    dfmask_base_t *maskbase;
    node *apfundef;
    node *apfunargs;
    bool retalias;
};

#define INFO_CONTEXT(n) (n->context)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LHS(n) (n->lhs)
#define INFO_MASKBASE(n) (n->maskbase)
#define INFO_APFUNDEF(n) (n->apfundef)
#define INFO_APFUNARGS(n) (n->apfunargs)
#define INFO_RETALIAS(n) (n->retalias)

static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_CONTEXT (result) = IA_undef;
    INFO_FUNDEF (result) = fundef;
    INFO_LHS (result) = NULL;
    INFO_MASKBASE (result) = NULL;
    INFO_APFUNDEF (result) = NULL;
    INFO_APFUNARGS (result) = NULL;
    INFO_RETALIAS (result) = FALSE;

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
 * @fn node *SHALprintPreFun( node *arg_node, info *arg_info)
 *
 * Printing prefun that prints informations about aliasing if the compilation
 * is aborted during memory management.
 *
 *****************************************************************************/
node *
SHALprintPreFun (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (NODE_TYPE (arg_node)) {
    case N_arg:
        if (ARG_ISALIASING (arg_node)) {
            fprintf (global.outfile, " /* IsAliasing */");
        }
        if (AVIS_ISALIAS (ARG_AVIS (arg_node))) {
            fprintf (global.outfile, " /* IsAlias */");
        }
        break;
    case N_ret:
        if (RET_ISALIASING (arg_node)) {
            fprintf (global.outfile, " /* IsAliasing*/");
        }
        break;
    case N_vardec:
        if (AVIS_ISALIAS (VARDEC_AVIS (arg_node))) {
            INDENT;
            fprintf (global.outfile, " /* IsAlias */\n");
        }
        break;
    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/

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

    DBUG_ENTER ();

    DBUG_PRINT_TAG ("EMIA", "Starting interface alias analysis...");

    counter = 0;
    unaliased = -1;

    while (unaliased != 0) {
        unaliased = 0;
        DBUG_PRINT_TAG ("EMIA", "Starting interface analysis traversal...");
        TRAVpush (TR_emia);
        syntax_tree = TRAVdo (syntax_tree, NULL);
        TRAVpop ();
        DBUG_PRINT_TAG ("EMIA", "Interface analysis traversal complete.");
        counter += unaliased;
    }

    DBUG_PRINT_TAG ("EMIA", "%d interfaces unaliased.", counter);
    DBUG_PRINT_TAG ("EMIA", "Interface alias analysis complete.");

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
SetArgAlias (node *arg, bool newval)
{
    DBUG_ENTER ();

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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

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
 * @fn node* EMIAap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMIAap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * Check whether arguments could have been aliased
     */
    INFO_CONTEXT (arg_info) = IA_ap;
    INFO_APFUNDEF (arg_info) = AP_FUNDEF (arg_node);
    INFO_APFUNARGS (arg_info) = FUNDEF_ARGS (AP_FUNDEF (arg_node));

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node* EMIAarg( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMIAarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (INFO_CONTEXT (arg_info)) {
    case IA_begin:
        /*
         * Create ALIASMASK
         */
        AVIS_ALIASMASK (ARG_AVIS (arg_node)) = DFMgenMaskClear (INFO_MASKBASE (arg_info));

        DFMsetMaskEntrySet (AVIS_ALIASMASK (ARG_AVIS (arg_node)), NULL,
                            ARG_AVIS (arg_node));

        DBUG_PRINT ("Mask created for %s", ARG_NAME (arg_node));
        break;

    case IA_end:
        /*
         * Remove ALIASMASK
         */
        AVIS_ALIASMASK (ARG_AVIS (arg_node))
          = DFMremoveMask (AVIS_ALIASMASK (ARG_AVIS (arg_node)));
        DBUG_PRINT ("Mask removed for %s", ARG_NAME (arg_node));
        break;

    case IA_unqargs:
        if (AVIS_TYPE (ARG_AVIS (arg_node)) != NULL) {
            if (TUisUniqueUserType (TYgetScalar (AVIS_TYPE (ARG_AVIS (arg_node))))) {
                arg_node = SetArgAlias (arg_node, FALSE);
            }
        }
        break;

    case IA_argnoalias:
        arg_node = SetArgAlias (arg_node, FALSE);
        break;

    default:
        DBUG_UNREACHABLE ("Illegal context");
        break;
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node* EMIAassign( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMIAassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * Top-down traversal
     */
    INFO_CONTEXT (arg_info) = IA_undef;
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node* EMIAblock( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMIAblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node* EMIAcond( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMIAcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node* EMIAfuncond( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMIAfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_CONTEXT (arg_info) = IA_funcond;

    /*
     * LOOP Functions: Alias of the return value does not depend on
     *                 the recursive application!
     */
    if (!FUNDEF_ISLOOPFUN (INFO_FUNDEF (arg_info))) {
        FUNCOND_THEN (arg_node) = TRAVdo (FUNCOND_THEN (arg_node), arg_info);
    }
    FUNCOND_ELSE (arg_node) = TRAVdo (FUNCOND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node* EMIAfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMIAfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    arg_info = MakeInfo (arg_node);

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_MASKBASE (arg_info)
          = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDECS (arg_node));

        /*
         * Traverse vardecs/args to initialize ALIASMASKS
         */
        INFO_CONTEXT (arg_info) = IA_begin;
        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
        }
        if (FUNDEF_VARDECS (arg_node) != NULL) {
            FUNDEF_VARDECS (arg_node) = TRAVdo (FUNDEF_VARDECS (arg_node), arg_info);
        }

        DBUG_PRINT ("Going into function %s", FUNDEF_NAME (arg_node));

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        DBUG_PRINT ("Coming from function %s", FUNDEF_NAME (arg_node));

        /*
         * Traverse vardecs/args to remove ALIASMASKS
         */
        INFO_CONTEXT (arg_info) = IA_end;
        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
        }
        if (FUNDEF_VARDECS (arg_node) != NULL) {
            FUNDEF_VARDECS (arg_node) = TRAVdo (FUNDEF_VARDECS (arg_node), arg_info);
        }

        INFO_MASKBASE (arg_info) = DFMremoveMaskBase (INFO_MASKBASE (arg_info));
    }

    /*
     * Mark unique parameters and return values not ALIAS
     */
    INFO_CONTEXT (arg_info) = IA_unqargs;
    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    INFO_RETALIAS (arg_info) = FUNDEF_HASDOTRETS (arg_node);
    if (FUNDEF_RETS (arg_node) != NULL) {
        FUNDEF_RETS (arg_node) = TRAVdo (FUNDEF_RETS (arg_node), arg_info);
    }

    /*
     * If no return value is marked ALIASING, no argument can be ALIASED
     */
    if (!INFO_RETALIAS (arg_info)) {
        INFO_CONTEXT (arg_info) = IA_argnoalias;
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
 * @fn node* EMIAid( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMIAid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (INFO_CONTEXT (arg_info)) {
    case IA_funcond:
    case IA_neutral:
    case IA_let:
        /*
         * ALIASING OPERATION a = b
         */
        DFMsetMaskOr (AVIS_ALIASMASK (IDS_AVIS (INFO_LHS (arg_info))),
                      AVIS_ALIASMASK (ID_AVIS (arg_node)));
        break;

    case IA_ap:
        /*
         * Function application
         */
        if ((INFO_APFUNARGS (arg_info) == NULL)
            || (ARG_ISALIASING (INFO_APFUNARGS (arg_info)))) {
            int retcount = 0;
            node *lhs = INFO_LHS (arg_info);
            while (lhs != NULL) {
                if (GetRetAlias (INFO_APFUNDEF (arg_info), retcount)) {
                    DFMsetMaskOr (AVIS_ALIASMASK (IDS_AVIS (lhs)),
                                  AVIS_ALIASMASK (ID_AVIS (arg_node)));
                }

                lhs = IDS_NEXT (lhs);
                retcount += 1;
            }
        }

        if (INFO_APFUNARGS (arg_info) != NULL) {
            INFO_APFUNARGS (arg_info) = ARG_NEXT (INFO_APFUNARGS (arg_info));
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
 * @fn node* EMIAlet( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMIAlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_CONTEXT (arg_info) = IA_let;
    INFO_LHS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node* EMIAret( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMIAret (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (TUisUniqueUserType (TYgetScalar (RET_TYPE (arg_node)))) {
        if (RET_ISALIASING (arg_node)) {
            RET_ISALIASING (arg_node) = FALSE;
            unaliased += 1;
        }
    }

    INFO_RETALIAS (arg_info) = INFO_RETALIAS (arg_info) || RET_ISALIASING (arg_node);

    if (RET_NEXT (arg_node) != NULL) {
        RET_NEXT (arg_node) = TRAVdo (RET_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node* EMIAreturn( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMIAreturn (node *arg_node, info *arg_info)
{
    dfmask_t *retmask, *argmask;
    node *retexprs;
    node *funargs;
    int retcount;

    DBUG_ENTER ();

    retmask = DFMgenMaskClear (INFO_MASKBASE (arg_info));
    argmask = DFMgenMaskClear (INFO_MASKBASE (arg_info));

    /*
     * 1. Arguments cannot return as aliases iff none of the return values
     * is an alias of an argument
     */
    retexprs = RETURN_EXPRS (arg_node);
    while (retexprs != NULL) {
        DFMsetMaskOr (retmask, AVIS_ALIASMASK (ID_AVIS (EXPRS_EXPR (retexprs))));
        retexprs = EXPRS_NEXT (retexprs);
    }

    funargs = FUNDEF_ARGS (INFO_FUNDEF (arg_info));
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
    funargs = FUNDEF_ARGS (INFO_FUNDEF (arg_info));
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
            INFO_FUNDEF (arg_info)
              = SetRetAlias (INFO_FUNDEF (arg_info), retcount, FALSE);
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
 * @fn node* EMIAvardec( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMIAvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (INFO_CONTEXT (arg_info)) {
    case IA_begin:
        /*
         * Create ALIASMASK
         */
        AVIS_ALIASMASK (VARDEC_AVIS (arg_node))
          = DFMgenMaskClear (INFO_MASKBASE (arg_info));

        DFMsetMaskEntrySet (AVIS_ALIASMASK (VARDEC_AVIS (arg_node)), NULL,
                            VARDEC_AVIS (arg_node));
        DBUG_PRINT ("Mask created for %s", VARDEC_NAME (arg_node));
        break;

    case IA_end:
        /*
         * Remove ALIASMASK
         */
        AVIS_ALIASMASK (VARDEC_AVIS (arg_node))
          = DFMremoveMask (AVIS_ALIASMASK (VARDEC_AVIS (arg_node)));
        DBUG_PRINT ("Mask removed for %s", VARDEC_NAME (arg_node));
        break;

    default:
        DBUG_UNREACHABLE ("Illegal context");
        break;
    }

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node* EMIAwith( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMIAwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node* EMIAwith2( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMIAwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node* EMIAfold( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMIAfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_CONTEXT (arg_info) = IA_neutral;
    FOLD_NEUTRAL (arg_node) = TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);

    INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));

    if (FOLD_NEXT (arg_node) != NULL) {
        FOLD_NEXT (arg_node) = TRAVdo (FOLD_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node* EMIAgenarray( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMIAgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));

    if (GENARRAY_NEXT (arg_node) != NULL) {
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node* EMIAmodarray( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMIAmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));

    if (MODARRAY_NEXT (arg_node) != NULL) {
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Interface analysis -->
 *****************************************************************************/

#undef DBUG_PREFIX
