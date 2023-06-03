/** <!--********************************************************************-->
 *
 * @defgroup rcm Reference Counting Minimization
 *
 * Removes obviously unnecessary reference counting instructions.
 *
 * @ingroup mm
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file rcminimize.c
 *
 * Prefix: RCM
 *
 *****************************************************************************/

#include "rcminimize.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "globals.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "free.h"
#include "str.h"
#include "memory.h"
#include "DataFlowMask.h"
#include "NumLookUpTable.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    nlut_t *env;
    dfmask_t *usedmask;
    nlut_t *env2;
    dfmask_t *usedmask2;
    node *assign;
    node *fundef;
    bool remassign;
};

#define INFO_ENV(n) (n->env)
#define INFO_USEDMASK(n) (n->usedmask)
#define INFO_ENV2(n) (n->env2)
#define INFO_USEDMASK2(n) (n->usedmask2)
#define INFO_ASSIGN(n) (n->assign)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_REMASSIGN(n) (n->remassign)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_ENV (result) = NULL;
    INFO_USEDMASK (result) = NULL;
    INFO_ENV2 (result) = NULL;
    INFO_USEDMASK2 (result) = NULL;
    INFO_ASSIGN (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_REMASSIGN (result) = FALSE;

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
 * @fn node *RCMdoRefcountMinimization( node *arg_node)
 *
 * @brief starting point of reference counting minimization traversal
 *
 * @param arg_node
 *
 * @return
 *
 *****************************************************************************/
node *
RCMdoRefcountMinimization (node *syntax_tree)
{
    DBUG_ENTER ();

    TRAVpush (TR_rcm);
    syntax_tree = TRAVdo (syntax_tree, NULL);
    TRAVpop ();

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
MakeRCMAssignments (nlut_t *nlut)
{
    node *res, *avis, *prf;
    int count;

    DBUG_ENTER ();

    res = NULL;

    avis = NLUTgetNonZeroAvis (nlut);
    while (avis != NULL) {
        count = NLUTgetNum (nlut, avis);
        NLUTsetNum (nlut, avis, 0);

        DBUG_ASSERT (count > 0, "Illegal increment found!");

        prf = TCmakePrf2 (F_inc_rc, TBmakeId (avis), TBmakeNum (count));

        res = TBmakeAssign (TBmakeLet (NULL, prf), res);

        avis = NLUTgetNonZeroAvis (NULL);
    }

    DBUG_RETURN (res);
}

static node *
ModifyExistingIncRcs (nlut_t *nlut, node *ass)
{
    DBUG_ENTER ();

    if (ASSIGN_NEXT (ass) != NULL) {
        ASSIGN_NEXT (ass) = ModifyExistingIncRcs (nlut, ASSIGN_NEXT (ass));
    }

    if ((NODE_TYPE (ASSIGN_STMT (ass)) == N_let)
        && (NODE_TYPE (ASSIGN_RHS (ass)) == N_prf)
        && (PRF_PRF (ASSIGN_RHS (ass)) == F_inc_rc)) {
        node *avis;
        int count;

        avis = ID_AVIS (PRF_ARG1 (ASSIGN_RHS (ass)));
        count = NLUTgetNum (nlut, avis);
        DBUG_ASSERT (count >= 0, "Illegal increment found!");
        NLUTsetNum (nlut, avis, 0);

        NUM_VAL (PRF_ARG2 (ASSIGN_RHS (ass))) += count;
    }

    DBUG_RETURN (ass);
}

static node *
PrependRCMAssignments (nlut_t *nlut, node *ass)
{
    DBUG_ENTER ();

    if (ass != NULL) {
        ass = ModifyExistingIncRcs (nlut, ass);
    }

    ass = TCappendAssign (MakeRCMAssignments (nlut), ass);

    DBUG_RETURN (ass);
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
 * @node *RCMap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
RCMap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node))) {
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
    } else {
        if (AP_ARGS (arg_node) != NULL) {
            AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *RCMarg( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
RCMarg (node *arg_node, info *arg_info)
{
    int n;

    DBUG_ENTER ();

    n = NLUTgetNum (INFO_ENV (arg_info), ARG_AVIS (arg_node));

    DBUG_ASSERT (n == 0, "Enequal numbers of inc_rc / dec_rc removed!");

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *RCMassign( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
RCMassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * bottom-up traversal
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    INFO_ASSIGN (arg_info) = arg_node;
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_REMASSIGN (arg_info)) {
        arg_node = FREEdoFreeNode (arg_node);
        INFO_REMASSIGN (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *RCMcode( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
RCMcode (node *arg_node, info *arg_info)
{
    nlut_t *oldenv;
    dfmask_t *oldusedmask;

    DBUG_ENTER ();

    oldenv = INFO_ENV (arg_info);
    oldusedmask = INFO_USEDMASK (arg_info);

    INFO_ENV (arg_info) = NLUTgenerateNlutFromNlut (oldenv);
    INFO_USEDMASK (arg_info) = DFMgenMaskCopy (oldusedmask);

    CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);
    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);

    INFO_ENV (arg_info) = NLUTremoveNlut (INFO_ENV (arg_info));
    INFO_USEDMASK (arg_info) = DFMremoveMask (INFO_USEDMASK (arg_info));

    INFO_ENV (arg_info) = oldenv;
    INFO_USEDMASK (arg_info) = oldusedmask;

    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *RCMfold( node *arg_node, info *arg_info)
 *
 * @brief avoid counting FOLD_ARGS should they exist!
 *
 *****************************************************************************/
node *
RCMfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    FOLD_NEUTRAL (arg_node) = TRAVopt (FOLD_NEUTRAL (arg_node), arg_info);
    FOLD_NEXT (arg_node) = TRAVopt (FOLD_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *RCMrange( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
RCMrange (node *arg_node, info *arg_info)
{
    nlut_t *oldenv;
    dfmask_t *oldusedmask;

    DBUG_ENTER ();

    /*
     * visit all N_id sons
     */
    RANGE_LOWERBOUND (arg_node) = TRAVdo (RANGE_LOWERBOUND (arg_node), arg_info);
    RANGE_UPPERBOUND (arg_node) = TRAVdo (RANGE_UPPERBOUND (arg_node), arg_info);
    RANGE_CHUNKSIZE (arg_node) = TRAVopt (RANGE_CHUNKSIZE (arg_node), arg_info);

    /*
     * stack info for new context in range body
     */
    oldenv = INFO_ENV (arg_info);
    oldusedmask = INFO_USEDMASK (arg_info);

    INFO_ENV (arg_info) = NLUTgenerateNlutFromNlut (oldenv);
    INFO_USEDMASK (arg_info) = DFMgenMaskCopy (oldusedmask);

    RANGE_RESULTS (arg_node) = TRAVdo (RANGE_RESULTS (arg_node), arg_info);
    RANGE_BODY (arg_node) = TRAVdo (RANGE_BODY (arg_node), arg_info);

    INFO_ENV (arg_info) = NLUTremoveNlut (INFO_ENV (arg_info));
    INFO_USEDMASK (arg_info) = DFMremoveMask (INFO_USEDMASK (arg_info));

    INFO_ENV (arg_info) = oldenv;
    INFO_USEDMASK (arg_info) = oldusedmask;

    if (RANGE_NEXT (arg_node) != NULL) {
        RANGE_NEXT (arg_node) = TRAVdo (RANGE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *RCMcond( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
RCMcond (node *arg_node, info *arg_info)
{
    nlut_t *env, *nzlut;
    dfmask_t *usedmask;
    node *avis;

    DBUG_ENTER ();

    /*
     * Traverse both branches
     */
    if (INFO_ENV2 (arg_info) == NULL) {
        INFO_ENV2 (arg_info) = NLUTduplicateNlut (INFO_ENV (arg_info));
        INFO_USEDMASK2 (arg_info) = DFMgenMaskCopy (INFO_USEDMASK (arg_info));
    }

    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);

    env = INFO_ENV (arg_info);
    usedmask = INFO_USEDMASK (arg_info);
    INFO_ENV (arg_info) = INFO_ENV2 (arg_info);
    INFO_USEDMASK (arg_info) = INFO_USEDMASK2 (arg_info);

    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    INFO_ENV (arg_info) = env;
    INFO_USEDMASK (arg_info) = usedmask;

    /*
     * Compute common environment and annotate missing inc_rc statements
     */
    env = NLUTgenerateNlutFromNlut (env);
    usedmask = DFMgenMaskOr (INFO_USEDMASK (arg_info), INFO_USEDMASK2 (arg_info));

    nzlut = NLUTaddNluts (INFO_ENV (arg_info), INFO_ENV2 (arg_info));

    avis = NLUTgetNonZeroAvis (nzlut);
    while (avis != NULL) {
        int c, t, e;

        t = NLUTgetNum (INFO_ENV (arg_info), avis);
        e = NLUTgetNum (INFO_ENV2 (arg_info), avis);

        c = t > e ? t : e;
        NLUTsetNum (INFO_ENV (arg_info), avis, c - t);
        NLUTsetNum (INFO_ENV2 (arg_info), avis, c - e);
        NLUTsetNum (env, avis, c);

        avis = NLUTgetNonZeroAvis (NULL);
    }

    nzlut = NLUTremoveNlut (nzlut);

    COND_THENASSIGNS (arg_node)
      = PrependRCMAssignments (INFO_ENV (arg_info), COND_THENASSIGNS (arg_node));

    COND_ELSEASSIGNS (arg_node)
      = PrependRCMAssignments (INFO_ENV2 (arg_info), COND_ELSEASSIGNS (arg_node));

    INFO_ENV2 (arg_info) = NLUTremoveNlut (INFO_ENV2 (arg_info));
    INFO_ENV (arg_info) = NLUTremoveNlut (INFO_ENV (arg_info));
    INFO_ENV (arg_info) = env;

    INFO_USEDMASK (arg_info) = DFMremoveMask (INFO_USEDMASK (arg_info));
    INFO_USEDMASK2 (arg_info) = DFMremoveMask (INFO_USEDMASK2 (arg_info));
    INFO_USEDMASK (arg_info) = usedmask;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *RCMfuncond( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
RCMfuncond (node *arg_node, info *arg_info)
{
    node *lhs;

    DBUG_ENTER ();

    if (INFO_ENV2 (arg_info) == NULL) {
        INFO_ENV2 (arg_info) = NLUTduplicateNlut (INFO_ENV (arg_info));
        INFO_USEDMASK2 (arg_info) = DFMgenMaskCopy (INFO_USEDMASK (arg_info));
    }

    /*
     * a = p ? b : c;
     * Env( b)  += Env( a);  Env( a)  = 0;
     * Env2( b) += Env2( a); Env2( a) = 0;
     */
    lhs = ASSIGN_LHS (INFO_ASSIGN (arg_info));

    NLUTincNum (INFO_ENV (arg_info), ID_AVIS (FUNCOND_THEN (arg_node)),
                NLUTgetNum (INFO_ENV (arg_info), IDS_AVIS (lhs)));
    NLUTsetNum (INFO_ENV (arg_info), IDS_AVIS (lhs), 0);

    NLUTincNum (INFO_ENV2 (arg_info), ID_AVIS (FUNCOND_ELSE (arg_node)),
                NLUTgetNum (INFO_ENV2 (arg_info), IDS_AVIS (lhs)));
    NLUTsetNum (INFO_ENV2 (arg_info), IDS_AVIS (lhs), 0);

    if (DFMtestMaskEntry (INFO_USEDMASK (arg_info), IDS_AVIS (lhs))) {
        DFMsetMaskEntrySet (INFO_USEDMASK (arg_info),
                            ID_AVIS (FUNCOND_THEN (arg_node)));
        DFMsetMaskEntrySet (INFO_USEDMASK2 (arg_info),
                            ID_AVIS (FUNCOND_ELSE (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *RCMfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
RCMfundef (node *arg_node, info *arg_info)
{
    dfmask_base_t *maskbase;

    DBUG_ENTER ();

    if ((!FUNDEF_ISCONDFUN (arg_node)) || (arg_info != NULL)) {

        if (FUNDEF_BODY (arg_node) != NULL) {
            info *info;

            info = MakeInfo ();
            INFO_FUNDEF (info) = arg_node;
            INFO_ENV (info)
              = NLUTgenerateNlut (FUNDEF_ARGS (arg_node), FUNDEF_VARDECS (arg_node));
            maskbase = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDECS (arg_node));
            INFO_USEDMASK (info) = DFMgenMaskClear (maskbase);

            if (FUNDEF_ISCONDFUN (arg_node)) {
                /*
                 * Treat conditional functionals like inline code.
                 * Use environment/usedmask of applying context
                 */
                node *extlet;
                node *retexprs, *ids;
                node *args, *argexprs;

                extlet = ASSIGN_STMT (INFO_ASSIGN (arg_info));

                retexprs = RETURN_EXPRS (FUNDEF_RETURN (arg_node));
                ids = LET_IDS (extlet);
                while (ids != NULL) {
                    NLUTsetNum (INFO_ENV (info), ID_AVIS (EXPRS_EXPR (retexprs)),
                                NLUTgetNum (INFO_ENV (arg_info), IDS_AVIS (ids)));
                    NLUTsetNum (INFO_ENV (arg_info), IDS_AVIS (ids), 0);

                    if (DFMtestMaskEntry (INFO_USEDMASK (arg_info),
                                          IDS_AVIS (ids))) {
                        DFMsetMaskEntrySet (INFO_USEDMASK (info),
                                            ID_AVIS (EXPRS_EXPR (retexprs)));
                    }

                    ids = IDS_NEXT (ids);
                    retexprs = EXPRS_NEXT (retexprs);
                }

                args = FUNDEF_ARGS (arg_node);
                argexprs = AP_ARGS (LET_EXPR (extlet));
                while (args != NULL) {
                    NLUTsetNum (INFO_ENV (info), ARG_AVIS (args),
                                NLUTgetNum (INFO_ENV (arg_info),
                                            ID_AVIS (EXPRS_EXPR (argexprs))));

                    if (DFMtestMaskEntry (INFO_USEDMASK (arg_info),
                                          ID_AVIS (EXPRS_EXPR (argexprs)))) {
                        DFMsetMaskEntrySet (INFO_USEDMASK (info), ARG_AVIS (args));
                    }
                    args = ARG_NEXT (args);
                    argexprs = EXPRS_NEXT (argexprs);
                }
            }

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);

            if (FUNDEF_ISCONDFUN (arg_node)) {
                /*
                 * Transscribe environment/usedmask back into applying context
                 */
                node *extlet;
                node *args, *argexprs;

                extlet = ASSIGN_STMT (INFO_ASSIGN (arg_info));

                args = FUNDEF_ARGS (arg_node);
                argexprs = AP_ARGS (LET_EXPR (extlet));
                while (args != NULL) {
                    NLUTsetNum (INFO_ENV (arg_info), ID_AVIS (EXPRS_EXPR (argexprs)),
                                NLUTgetNum (INFO_ENV (info), ARG_AVIS (args)));
                    NLUTsetNum (INFO_ENV (info), ARG_AVIS (args), 0);

                    if (DFMtestMaskEntry (INFO_USEDMASK (info), ARG_AVIS (args))) {
                        DFMsetMaskEntrySet (INFO_USEDMASK (arg_info),
                                            ID_AVIS (EXPRS_EXPR (argexprs)));
                    }
                    args = ARG_NEXT (args);
                    argexprs = EXPRS_NEXT (argexprs);
                }
            }

            if (FUNDEF_ARGS (arg_node) != NULL) {
                FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), info);
            }

            INFO_ENV (info) = NLUTremoveNlut (INFO_ENV (info));
            INFO_USEDMASK (info) = DFMremoveMask (INFO_USEDMASK (info));
            maskbase = DFMremoveMaskBase (maskbase);
            info = FreeInfo (info);
        }
    }

    /*
     * Traverse other fundefs if this is a regular fundef traversal
     */
    if ((arg_info == NULL) && (FUNDEF_NEXT (arg_node) != NULL)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *RCMid( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
RCMid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DFMsetMaskEntrySet (INFO_USEDMASK (arg_info), ID_AVIS (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *RCMids( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
RCMids (node *arg_node, info *arg_info)
{
    int n;

    DBUG_ENTER ();

    n = NLUTgetNum (INFO_ENV (arg_info), IDS_AVIS (arg_node));

    DBUG_ASSERT (n == 0, "Unequal numbers of inc_rc / dec_rc removed for %s!",
                 AVIS_NAME (IDS_AVIS (arg_node)));

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *RCMlet( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
RCMlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *RCMprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
RCMprf (node *arg_node, info *arg_info)
{
    int env, n, min;

    DBUG_ENTER ();

    switch (PRF_PRF (arg_node)) {
    case F_dec_rc:
        if (DFMtestMaskEntry (INFO_USEDMASK (arg_info),
                              ID_AVIS (PRF_ARG1 (arg_node)))) {
            /*
             * This dec_rc( x, n) is redundant.
             * Add n to Env( x) and remove this assignment
             */
            NLUTincNum (INFO_ENV (arg_info), ID_AVIS (PRF_ARG1 (arg_node)),
                        NUM_VAL (PRF_ARG2 (arg_node)));

            INFO_REMASSIGN (arg_info) = TRUE;
        } else {
            /*
             * This dec_rc( x, n) is the last reference to x.
             * Mark x as used.
             */
            DFMsetMaskEntrySet (INFO_USEDMASK (arg_info),
                                ID_AVIS (PRF_ARG1 (arg_node)));

            /*
             * Add n-1 to Env( x) and set n to 1
             */
            NLUTincNum (INFO_ENV (arg_info), ID_AVIS (PRF_ARG1 (arg_node)),
                        NUM_VAL (PRF_ARG2 (arg_node)) - 1);
            NUM_VAL (PRF_ARG2 (arg_node)) = 1;
        }
        break;

    case F_inc_rc:
        /*
         * inc_rc( x, n)
         */
        env = NLUTgetNum (INFO_ENV (arg_info), ID_AVIS (PRF_ARG1 (arg_node)));
        n = NUM_VAL (PRF_ARG2 (arg_node));
        min = (env < n) ? env : n;

        /*
         * decrease Env( x) by min( Env( x), n)
         */
        NLUTsetNum (INFO_ENV (arg_info), ID_AVIS (PRF_ARG1 (arg_node)), env - min);

        /*
         * decrease n by min( Env( x), n)
         */
        NUM_VAL (PRF_ARG2 (arg_node)) = n - min;

        /*
         * remove inc_rc( x, 0)
         */
        if (NUM_VAL (PRF_ARG2 (arg_node)) == 0) {
            INFO_REMASSIGN (arg_info) = TRUE;
        }
        break;

    case F_wl_assign:
    case F_wl_break:
    case F_prop_obj_in:
    case F_prop_obj_out:
    case F_accu:
    case F_suballoc:
    case F_noop:
        /*
         * Do nothing for these PRFs as the IV must not be artificially kept
         * alive
         */
        break;

    default:
        if (PRF_ARGS (arg_node) != NULL) {
            PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
        }
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *RCMreturn( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
RCMreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (!FUNDEF_ISCONDFUN (INFO_FUNDEF (arg_info))) {
        if (RETURN_EXPRS (arg_node) != NULL) {
            RETURN_EXPRS (arg_node) = TRAVdo (RETURN_EXPRS (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Reference counting minimization template -->
 *****************************************************************************/

#undef DBUG_PREFIX
