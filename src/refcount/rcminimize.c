/*
 *
 * $Log$
 * Revision 1.2  2005/07/16 09:57:55  ktr
 * maintenance
 *
 * Revision 1.1  2005/07/03 16:58:08  ktr
 * Initial revision
 *
 */
#include "rcminimize.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "globals.h"
#include "dbug.h"
#include "free.h"
#include "internal_lib.h"
#include "DataFlowMask.h"
#include "NumLookUpTable.h"

/*
 * INFO structure
 */
struct INFO {
    nlut_t *env;
    dfmask_t *usedmask;
    nlut_t *env2;
    dfmask_t *usedmask2;
    node *lhs;
    node *fundef;
    bool remassign;
};

/*
 * INFO macros
 */
#define INFO_RCM_ENV(n) (n->env)
#define INFO_RCM_USEDMASK(n) (n->usedmask)
#define INFO_RCM_ENV2(n) (n->env2)
#define INFO_RCM_USEDMASK2(n) (n->usedmask2)
#define INFO_RCM_LHS(n) (n->lhs)
#define INFO_RCM_FUNDEF(n) (n->fundef)
#define INFO_RCM_REMASSIGN(n) (n->remassign)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_RCM_ENV (result) = NULL;
    INFO_RCM_USEDMASK (result) = NULL;
    INFO_RCM_ENV2 (result) = NULL;
    INFO_RCM_USEDMASK2 (result) = NULL;
    INFO_RCM_LHS (result) = NULL;
    INFO_RCM_FUNDEF (result) = NULL;
    INFO_RCM_REMASSIGN (result) = FALSE;

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
    DBUG_ENTER ("RCMdoRefcountMinimization");

    TRAVpush (TR_rcm);
    syntax_tree = TRAVdo (syntax_tree, NULL);
    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}

/*****************************************************************************
 *
 * HELPER FUNCTIONS
 *
 ****************************************************************************/
static node *
MakeRCMAssignments (nlut_t *nlut)
{
    node *res, *avis, *prf;
    int count;

    DBUG_ENTER ("MakeRCMAssignments");

    res = NULL;
    avis = DFMgetMaskEntryAvisSet (NLUTgetNonZeroMask (nlut));

    while (avis != NULL) {
        count = NLUTgetNum (nlut, avis);
        NLUTsetNum (nlut, avis, 0);

        DBUG_ASSERT ((count > 0), "Illegal increment found!");

        prf = TCmakePrf2 (F_inc_rc, TBmakeId (avis), TBmakeNum (count));

        res = TBmakeAssign (TBmakeLet (NULL, prf), res);

        avis = DFMgetMaskEntryAvisSet (NULL);
    }

    DBUG_RETURN (res);
}

static node *
ModifyExistingIncRcs (nlut_t *nlut, node *ass)
{
    DBUG_ENTER ("ModifyExistingIncRcs");

    if (ASSIGN_NEXT (ass) != NULL) {
        ASSIGN_NEXT (ass) = ModifyExistingIncRcs (nlut, ASSIGN_NEXT (ass));
    }

    if ((NODE_TYPE (ASSIGN_INSTR (ass)) == N_let)
        && (NODE_TYPE (ASSIGN_RHS (ass)) == N_prf)
        && (PRF_PRF (ASSIGN_RHS (ass)) == F_inc_rc)) {
        node *avis;
        int count;

        avis = ID_AVIS (PRF_ARG1 (ASSIGN_RHS (ass)));
        count = NLUTgetNum (nlut, avis);
        DBUG_ASSERT ((count >= 0), "Illegal increment found!");
        NLUTsetNum (nlut, avis, 0);

        NUM_VAL (PRF_ARG2 (ASSIGN_RHS (ass))) += count;
    }

    DBUG_RETURN (ass);
}

static node *
PrependRCMAssignments (nlut_t *nlut, node *ass)
{
    DBUG_ENTER ("PrependRCMAssignments");

    if ((ass != NULL) && (NODE_TYPE (ass) == N_empty)) {
        ass = FREEdoFreeNode (ass);
    }

    if (ass != NULL) {
        ass = ModifyExistingIncRcs (nlut, ass);
    }

    ass = TCappendAssign (MakeRCMAssignments (nlut), ass);

    if (ass == NULL) {
        ass = TBmakeEmpty ();
    }

    DBUG_RETURN (ass);
}

/******************************************************************************
 *
 * Reference counting minimization traversal (rcm_tab)
 *
 * prefix: RCM
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @node RCMap( node *arg_node, info *arg_info)
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
RCMap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RCMap");

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
 * @node RCMarg( node *arg_node, info *arg_info)
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
RCMarg (node *arg_node, info *arg_info)
{
    int n;

    DBUG_ENTER ("RCMarg");

    n = NLUTgetNum (INFO_RCM_ENV (arg_info), ARG_AVIS (arg_node));

    DBUG_ASSERT ((n == 0), "Enequal numbers of inc_rc / dec_rc removed!");

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node RCMassign( node *arg_node, info *arg_info)
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
RCMassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RCMassign");

    /*
     * bottom-up traversal
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_RCM_REMASSIGN (arg_info)) {
        arg_node = FREEdoFreeNode (arg_node);
        INFO_RCM_REMASSIGN (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node RCMcode( node *arg_node, info *arg_info)
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
RCMcode (node *arg_node, info *arg_info)
{
    nlut_t *oldenv;
    dfmask_t *oldusedmask;

    DBUG_ENTER ("RCMcode");

    oldenv = INFO_RCM_ENV (arg_info);
    oldusedmask = INFO_RCM_USEDMASK (arg_info);

    INFO_RCM_ENV (arg_info)
      = NLUTgenerateNlut (FUNDEF_ARGS (INFO_RCM_FUNDEF (arg_info)),
                          FUNDEF_VARDEC (INFO_RCM_FUNDEF (arg_info)));
    INFO_RCM_USEDMASK (arg_info) = DFMgenMaskCopy (oldusedmask);

    CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);
    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);

    INFO_RCM_ENV (arg_info) = NLUTremoveNlut (INFO_RCM_ENV (arg_info));
    INFO_RCM_USEDMASK (arg_info) = DFMremoveMask (INFO_RCM_USEDMASK (arg_info));

    INFO_RCM_ENV (arg_info) = oldenv;
    INFO_RCM_USEDMASK (arg_info) = oldusedmask;

    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node RCMcond( node *arg_node, info *arg_info)
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
RCMcond (node *arg_node, info *arg_info)
{
    nlut_t *env;
    dfmask_t *usedmask, *nzmask;
    node *avis;

    DBUG_ENTER ("RCMcond");

    /*
     * Traverse both branches
     */
    if (INFO_RCM_ENV2 (arg_info) == NULL) {
        INFO_RCM_ENV2 (arg_info) = NLUTduplicateNlut (INFO_RCM_ENV (arg_info));
        INFO_RCM_USEDMASK2 (arg_info) = DFMgenMaskCopy (INFO_RCM_USEDMASK (arg_info));
    }

    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);

    env = INFO_RCM_ENV (arg_info);
    usedmask = INFO_RCM_USEDMASK (arg_info);
    INFO_RCM_ENV (arg_info) = INFO_RCM_ENV2 (arg_info);
    INFO_RCM_USEDMASK (arg_info) = INFO_RCM_USEDMASK2 (arg_info);

    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    INFO_RCM_ENV (arg_info) = env;
    INFO_RCM_USEDMASK (arg_info) = usedmask;

    /*
     * Compute common environment and annotate missing inc_rc statements
     */
    env = NLUTgenerateNlut (FUNDEF_ARGS (INFO_RCM_FUNDEF (arg_info)),
                            FUNDEF_VARDEC (INFO_RCM_FUNDEF (arg_info)));
    usedmask = DFMgenMaskOr (INFO_RCM_USEDMASK (arg_info), INFO_RCM_USEDMASK2 (arg_info));

    nzmask = DFMgenMaskOr (NLUTgetNonZeroMask (INFO_RCM_ENV (arg_info)),
                           NLUTgetNonZeroMask (INFO_RCM_ENV2 (arg_info)));

    avis = DFMgetMaskEntryAvisSet (nzmask);
    while (avis != NULL) {
        int c, t, e;

        t = NLUTgetNum (INFO_RCM_ENV (arg_info), avis);
        e = NLUTgetNum (INFO_RCM_ENV2 (arg_info), avis);

        c = t > e ? t : e;
        NLUTsetNum (INFO_RCM_ENV (arg_info), avis, c - t);
        NLUTsetNum (INFO_RCM_ENV2 (arg_info), avis, c - e);
        NLUTsetNum (env, avis, c);

        avis = DFMgetMaskEntryAvisSet (NULL);
    }

    nzmask = DFMremoveMask (nzmask);

    COND_THENINSTR (arg_node)
      = PrependRCMAssignments (INFO_RCM_ENV (arg_info), COND_THENINSTR (arg_node));

    COND_ELSEINSTR (arg_node)
      = PrependRCMAssignments (INFO_RCM_ENV2 (arg_info), COND_ELSEINSTR (arg_node));

    INFO_RCM_ENV2 (arg_info) = NLUTremoveNlut (INFO_RCM_ENV2 (arg_info));
    INFO_RCM_ENV (arg_info) = NLUTremoveNlut (INFO_RCM_ENV (arg_info));
    INFO_RCM_ENV (arg_info) = env;

    INFO_RCM_USEDMASK (arg_info) = DFMremoveMask (INFO_RCM_USEDMASK (arg_info));
    INFO_RCM_USEDMASK2 (arg_info) = DFMremoveMask (INFO_RCM_USEDMASK2 (arg_info));
    INFO_RCM_USEDMASK (arg_info) = usedmask;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node RCMfuncond( node *arg_node, info *arg_info)
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
RCMfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RCMfuncond");

    if (INFO_RCM_ENV2 (arg_info) == NULL) {
        INFO_RCM_ENV2 (arg_info) = NLUTduplicateNlut (INFO_RCM_ENV (arg_info));
        INFO_RCM_USEDMASK2 (arg_info) = DFMgenMaskCopy (INFO_RCM_USEDMASK (arg_info));
    }

    /*
     * a = p ? b : c;
     * Env( b)  += Env( a);  Env( a)  = 0;
     * Env2( b) += Env2( a); Env2( a) = 0;
     */
    NLUTincNum (INFO_RCM_ENV (arg_info), ID_AVIS (FUNCOND_THEN (arg_node)),
                NLUTgetNum (INFO_RCM_ENV (arg_info), IDS_AVIS (INFO_RCM_LHS (arg_info))));

    NLUTsetNum (INFO_RCM_ENV (arg_info), IDS_AVIS (INFO_RCM_LHS (arg_info)), 0);

    NLUTincNum (INFO_RCM_ENV2 (arg_info), ID_AVIS (FUNCOND_ELSE (arg_node)),
                NLUTgetNum (INFO_RCM_ENV2 (arg_info),
                            IDS_AVIS (INFO_RCM_LHS (arg_info))));

    NLUTsetNum (INFO_RCM_ENV2 (arg_info), IDS_AVIS (INFO_RCM_LHS (arg_info)), 0);

    if (DFMtestMaskEntry (INFO_RCM_USEDMASK (arg_info), NULL,
                          IDS_AVIS (INFO_RCM_LHS (arg_info)))) {
        DFMsetMaskEntrySet (INFO_RCM_USEDMASK (arg_info), NULL,
                            ID_AVIS (FUNCOND_THEN (arg_node)));
        DFMsetMaskEntrySet (INFO_RCM_USEDMASK2 (arg_info), NULL,
                            ID_AVIS (FUNCOND_ELSE (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node RCMfundef( node *arg_node, info *arg_info)
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
RCMfundef (node *arg_node, info *arg_info)
{
    dfmask_base_t *maskbase;

    DBUG_ENTER ("RCMfundef");

    if ((!FUNDEF_ISCONDFUN (arg_node)) || (arg_info != NULL)) {

        if (FUNDEF_BODY (arg_node) != NULL) {
            info *info;

            info = MakeInfo ();
            INFO_RCM_FUNDEF (info) = arg_node;
            INFO_RCM_ENV (info)
              = NLUTgenerateNlut (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));
            maskbase = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));
            INFO_RCM_USEDMASK (info) = DFMgenMaskClear (maskbase);

            if (FUNDEF_ISCONDFUN (arg_node)) {
                /*
                 * Treat conditional functionals like inline code.
                 * Use environment/usedmask of applying context
                 */
                node *extlet;
                node *retexprs, *ids;
                node *args, *argexprs;

                extlet = ASSIGN_INSTR (FUNDEF_EXT_ASSIGN (arg_node));

                retexprs = RETURN_EXPRS (FUNDEF_RETURN (arg_node));
                ids = LET_IDS (extlet);
                while (ids != NULL) {
                    NLUTsetNum (INFO_RCM_ENV (info), ID_AVIS (EXPRS_EXPR (retexprs)),
                                NLUTgetNum (INFO_RCM_ENV (arg_info), IDS_AVIS (ids)));
                    NLUTsetNum (INFO_RCM_ENV (arg_info), IDS_AVIS (ids), 0);

                    if (DFMtestMaskEntry (INFO_RCM_USEDMASK (arg_info), NULL,
                                          IDS_AVIS (ids))) {
                        DFMsetMaskEntrySet (INFO_RCM_USEDMASK (info), NULL,
                                            ID_AVIS (EXPRS_EXPR (retexprs)));
                    }

                    ids = IDS_NEXT (ids);
                    retexprs = EXPRS_NEXT (retexprs);
                }

                args = FUNDEF_ARGS (arg_node);
                argexprs = AP_ARGS (LET_EXPR (extlet));
                while (args != NULL) {
                    NLUTsetNum (INFO_RCM_ENV (info), ARG_AVIS (args),
                                NLUTgetNum (INFO_RCM_ENV (arg_info),
                                            ID_AVIS (EXPRS_EXPR (argexprs))));

                    if (DFMtestMaskEntry (INFO_RCM_USEDMASK (arg_info), NULL,
                                          ID_AVIS (EXPRS_EXPR (argexprs)))) {
                        DFMsetMaskEntrySet (INFO_RCM_USEDMASK (info), NULL,
                                            ARG_AVIS (args));
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

                extlet = ASSIGN_INSTR (FUNDEF_EXT_ASSIGN (arg_node));

                args = FUNDEF_ARGS (arg_node);
                argexprs = AP_ARGS (LET_EXPR (extlet));
                while (args != NULL) {
                    NLUTsetNum (INFO_RCM_ENV (arg_info), ID_AVIS (EXPRS_EXPR (argexprs)),
                                NLUTgetNum (INFO_RCM_ENV (info), ARG_AVIS (args)));
                    NLUTsetNum (INFO_RCM_ENV (info), ARG_AVIS (args), 0);

                    if (DFMtestMaskEntry (INFO_RCM_USEDMASK (info), NULL,
                                          ARG_AVIS (args))) {
                        DFMsetMaskEntrySet (INFO_RCM_USEDMASK (arg_info), NULL,
                                            ID_AVIS (EXPRS_EXPR (argexprs)));
                    }
                    args = ARG_NEXT (args);
                    argexprs = EXPRS_NEXT (argexprs);
                }
            }

            if (FUNDEF_ARGS (arg_node) != NULL) {
                FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), info);
            }

            INFO_RCM_ENV (info) = NLUTremoveNlut (INFO_RCM_ENV (info));
            INFO_RCM_USEDMASK (info) = DFMremoveMask (INFO_RCM_USEDMASK (info));
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
 * @node RCMid( node *arg_node, info *arg_info)
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
RCMid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RCMid");

    DFMsetMaskEntrySet (INFO_RCM_USEDMASK (arg_info), NULL, ID_AVIS (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node RCMids( node *arg_node, info *arg_info)
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
RCMids (node *arg_node, info *arg_info)
{
    int n;

    DBUG_ENTER ("RCMids");

    n = NLUTgetNum (INFO_RCM_ENV (arg_info), IDS_AVIS (arg_node));

    DBUG_ASSERT ((n == 0), "Enequal numbers of inc_rc / dec_rc removed!");

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node RCMlet( node *arg_node, info *arg_info)
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
RCMlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RCMlet");

    INFO_RCM_LHS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node RCMprf( node *arg_node, info *arg_info)
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
RCMprf (node *arg_node, info *arg_info)
{
    int env, n, min;

    DBUG_ENTER ("RCMprf");

    switch (PRF_PRF (arg_node)) {
    case F_dec_rc:
        if (DFMtestMaskEntry (INFO_RCM_USEDMASK (arg_info), NULL,
                              ID_AVIS (PRF_ARG1 (arg_node)))) {
            /*
             * This dec_rc( x, n) is redundant.
             * Add n to Env( x) and remove this assignment
             */
            NLUTincNum (INFO_RCM_ENV (arg_info), ID_AVIS (PRF_ARG1 (arg_node)),
                        NUM_VAL (PRF_ARG2 (arg_node)));

            INFO_RCM_REMASSIGN (arg_info) = TRUE;
        } else {
            /*
             * This dec_rc( x, n) is the last reference to x.
             * Mark x as used.
             */
            DFMsetMaskEntrySet (INFO_RCM_USEDMASK (arg_info), NULL,
                                ID_AVIS (PRF_ARG1 (arg_node)));

            /*
             * Add n-1 to Env( x) and set n to 1
             */
            NLUTincNum (INFO_RCM_ENV (arg_info), ID_AVIS (PRF_ARG1 (arg_node)),
                        NUM_VAL (PRF_ARG2 (arg_node)) - 1);
            NUM_VAL (PRF_ARG2 (arg_node)) = 1;
        }
        break;

    case F_inc_rc:
        /*
         * inc_rc( x, n)
         */
        env = NLUTgetNum (INFO_RCM_ENV (arg_info), ID_AVIS (PRF_ARG1 (arg_node)));
        n = NUM_VAL (PRF_ARG2 (arg_node));
        min = (env < n) ? env : n;

        /*
         * decrease Env( x) by min( Env( x), n)
         */
        NLUTsetNum (INFO_RCM_ENV (arg_info), ID_AVIS (PRF_ARG1 (arg_node)), env - min);

        /*
         * decrease n by min( Env( x), n)
         */
        NUM_VAL (PRF_ARG2 (arg_node)) = n - min;

        /*
         * remove inc_rc( x, 0)
         */
        if (NUM_VAL (PRF_ARG2 (arg_node)) == 0) {
            INFO_RCM_REMASSIGN (arg_info) = TRUE;
        }
        break;

    case F_wl_assign:
    case F_accu:
    case F_suballoc:
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
 * @node RCMreturn( node *arg_node, info *arg_info)
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
RCMreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RCMreturn");

    if (!FUNDEF_ISCONDFUN (INFO_RCM_FUNDEF (arg_info))) {
        if (RETURN_EXPRS (arg_node) != NULL) {
            RETURN_EXPRS (arg_node) = TRAVdo (RETURN_EXPRS (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}
