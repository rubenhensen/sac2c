/*
 *
 * $Log$
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
    lut_t *env;
    dfmask_t *usedmask;
    lut_t *env2;
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
    lut_t *oldenv;
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
    lut_t *env;
    dfmask_t *usedmask;
    node *temp;

    DBUG_ENTER ("RCMcond");

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

    INFO_RCM_ENV2 (arg_info) = INFO_RCM_ENV (arg_info);
    INFO_RCM_USEDMASK2 (arg_info) = INFO_RCM_USEDMASK (arg_info);

    INFO_RCM_ENV (arg_info) = env;
    INFO_RCM_USEDMASK (arg_info) = usedmask;

    temp = FUNDEF_ARGS (INFO_RCM_FUNDEF (arg_info));
    while (temp != NULL) {
        int c, t, e;

        t = NLUTgetNum (INFO_RCM_ENV (arg_info), ARG_AVIS (temp));
        e = NLUTgetNum (INFO_RCM_ENV2 (arg_info), ARG_AVIS (temp));

        c = t > e ? t : e;
        NLUTsetNum (INFO_RCM_ENV (arg_info), ARG_AVIS (temp), c);

        if (c - t > 0) {
            if (NODE_TYPE (BLOCK_INSTR (COND_THEN (arg_node))) == N_empty) {
                BLOCK_INSTR (COND_THEN (arg_node))
                  = FREEdoFreeNode (BLOCK_INSTR (COND_THEN (arg_node)));
            }

            BLOCK_INSTR (COND_THEN (arg_node))
              = TBmakeAssign (TBmakeLet (NULL,
                                         TCmakePrf2 (F_inc_rc, TBmakeId (ARG_AVIS (temp)),
                                                     TBmakeNum (c - t))),
                              BLOCK_INSTR (COND_THEN (arg_node)));
        }
        if (c - e > 0) {
            if (NODE_TYPE (BLOCK_INSTR (COND_ELSE (arg_node))) == N_empty) {
                BLOCK_INSTR (COND_ELSE (arg_node))
                  = FREEdoFreeNode (BLOCK_INSTR (COND_ELSE (arg_node)));
            }

            BLOCK_INSTR (COND_ELSE (arg_node))
              = TBmakeAssign (TBmakeLet (NULL,
                                         TCmakePrf2 (F_inc_rc, TBmakeId (ARG_AVIS (temp)),
                                                     TBmakeNum (c - e))),
                              BLOCK_INSTR (COND_ELSE (arg_node)));
        }

        temp = ARG_NEXT (temp);
    }

    temp = FUNDEF_VARDEC (INFO_RCM_FUNDEF (arg_info));
    while (temp != NULL) {
        int c, t, e;

        t = NLUTgetNum (INFO_RCM_ENV (arg_info), VARDEC_AVIS (temp));
        e = NLUTgetNum (INFO_RCM_ENV2 (arg_info), VARDEC_AVIS (temp));

        c = t > e ? t : e;
        NLUTsetNum (INFO_RCM_ENV (arg_info), VARDEC_AVIS (temp), c);

        if (c - t > 0) {
            if (NODE_TYPE (BLOCK_INSTR (COND_THEN (arg_node))) == N_empty) {
                BLOCK_INSTR (COND_THEN (arg_node))
                  = FREEdoFreeNode (BLOCK_INSTR (COND_THEN (arg_node)));
            }

            BLOCK_INSTR (COND_THEN (arg_node))
              = TBmakeAssign (TBmakeLet (NULL, TCmakePrf2 (F_inc_rc,
                                                           TBmakeId (VARDEC_AVIS (temp)),
                                                           TBmakeNum (c - t))),
                              BLOCK_INSTR (COND_THEN (arg_node)));
        }
        if (c - e > 0) {
            if (NODE_TYPE (BLOCK_INSTR (COND_ELSE (arg_node))) == N_empty) {
                BLOCK_INSTR (COND_ELSE (arg_node))
                  = FREEdoFreeNode (BLOCK_INSTR (COND_ELSE (arg_node)));
            }

            BLOCK_INSTR (COND_ELSE (arg_node))
              = TBmakeAssign (TBmakeLet (NULL, TCmakePrf2 (F_inc_rc,
                                                           TBmakeId (VARDEC_AVIS (temp)),
                                                           TBmakeNum (c - e))),
                              BLOCK_INSTR (COND_ELSE (arg_node)));
        }

        temp = VARDEC_NEXT (temp);
    }

    if (BLOCK_INSTR (COND_THEN (arg_node)) == NULL) {
        BLOCK_INSTR (COND_THEN (arg_node)) = TBmakeEmpty ();
    }

    if (BLOCK_INSTR (COND_ELSE (arg_node)) == NULL) {
        BLOCK_INSTR (COND_ELSE (arg_node)) = TBmakeEmpty ();
    }

    INFO_RCM_ENV2 (arg_info) = NLUTremoveNlut (INFO_RCM_ENV2 (arg_info));
    INFO_RCM_USEDMASK2 (arg_info) = DFMremoveMask (INFO_RCM_USEDMASK2 (arg_info));

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

    NLUTsetNum (INFO_RCM_ENV (arg_info), ID_AVIS (FUNCOND_THEN (arg_node)),
                (NLUTgetNum (INFO_RCM_ENV (arg_info), ID_AVIS (FUNCOND_THEN (arg_node)))
                 + NLUTgetNum (INFO_RCM_ENV (arg_info),
                               IDS_AVIS (INFO_RCM_LHS (arg_info)))));

    NLUTsetNum (INFO_RCM_ENV2 (arg_info), ID_AVIS (FUNCOND_ELSE (arg_node)),
                (NLUTgetNum (INFO_RCM_ENV2 (arg_info), ID_AVIS (FUNCOND_ELSE (arg_node)))
                 + NLUTgetNum (INFO_RCM_ENV2 (arg_info),
                               IDS_AVIS (INFO_RCM_LHS (arg_info)))));

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

    if (FUNDEF_ISCONDFUN (arg_node)) {
        if (arg_info != NULL) {
            node *extlet;
            node *retexprs, *ids;
            node *args, *argexprs;

            info *oldinfo = arg_info;

            arg_info = MakeInfo ();
            INFO_RCM_FUNDEF (arg_info) = arg_node;
            INFO_RCM_ENV (arg_info)
              = NLUTgenerateNlut (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));
            maskbase = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));
            INFO_RCM_USEDMASK (arg_info) = DFMgenMaskClear (maskbase);

            /*
             * Treat conditional functionals like inline code.
             * Use environment/usedmask of applying context
             */
            extlet = ASSIGN_INSTR (FUNDEF_EXT_ASSIGN (arg_node));

            retexprs = RETURN_EXPRS (FUNDEF_RETURN (arg_node));
            ids = LET_IDS (extlet);
            while (ids != NULL) {
                NLUTsetNum (INFO_RCM_ENV (arg_info), ID_AVIS (EXPRS_EXPR (retexprs)),
                            NLUTgetNum (INFO_RCM_ENV (oldinfo), IDS_AVIS (ids)));
                if (DFMtestMaskEntry (INFO_RCM_USEDMASK (oldinfo), NULL,
                                      IDS_AVIS (ids))) {
                    DFMsetMaskEntrySet (INFO_RCM_USEDMASK (arg_info), NULL,
                                        ID_AVIS (EXPRS_EXPR (retexprs)));
                }

                ids = IDS_NEXT (ids);
                retexprs = EXPRS_NEXT (retexprs);
            }

            args = FUNDEF_ARGS (arg_node);
            argexprs = AP_ARGS (LET_EXPR (extlet));
            while (args != NULL) {
                NLUTsetNum (INFO_RCM_ENV (arg_info), ARG_AVIS (args),
                            NLUTgetNum (INFO_RCM_ENV (oldinfo),
                                        ID_AVIS (EXPRS_EXPR (argexprs))));

                if (DFMtestMaskEntry (INFO_RCM_USEDMASK (oldinfo), NULL,
                                      ID_AVIS (EXPRS_EXPR (argexprs)))) {
                    DFMsetMaskEntrySet (INFO_RCM_USEDMASK (arg_info), NULL,
                                        ARG_AVIS (args));
                }
                args = ARG_NEXT (args);
                argexprs = EXPRS_NEXT (argexprs);
            }

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

            /*
             * Transscribe environment/usedmask back into applying context
             */
            args = FUNDEF_ARGS (arg_node);
            argexprs = AP_ARGS (LET_EXPR (extlet));
            while (args != NULL) {
                NLUTsetNum (INFO_RCM_ENV (oldinfo), ID_AVIS (EXPRS_EXPR (argexprs)),
                            NLUTgetNum (INFO_RCM_ENV (arg_info), ARG_AVIS (args)));

                if (DFMtestMaskEntry (INFO_RCM_USEDMASK (arg_info), NULL,
                                      ARG_AVIS (args))) {
                    DFMsetMaskEntrySet (INFO_RCM_USEDMASK (oldinfo), NULL,
                                        ID_AVIS (EXPRS_EXPR (argexprs)));
                }
                args = ARG_NEXT (args);
                argexprs = EXPRS_NEXT (argexprs);
            }

            INFO_RCM_ENV (arg_info) = NLUTremoveNlut (INFO_RCM_ENV (arg_info));
            INFO_RCM_USEDMASK (arg_info) = DFMremoveMask (INFO_RCM_USEDMASK (arg_info));
            maskbase = DFMremoveMaskBase (maskbase);
            arg_info = FreeInfo (arg_info);

            arg_info = oldinfo;
        } else {
            /*
             * Traverse next function iff conditional function was not
             * traversed recursively
             */
            if (FUNDEF_NEXT (arg_node) != NULL) {
                FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
            }
        }
    } else {
        if (FUNDEF_BODY (arg_node) != NULL) {

            arg_info = MakeInfo ();
            INFO_RCM_FUNDEF (arg_info) = arg_node;
            INFO_RCM_ENV (arg_info)
              = NLUTgenerateNlut (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));
            maskbase = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));
            INFO_RCM_USEDMASK (arg_info) = DFMgenMaskClear (maskbase);

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

            INFO_RCM_ENV (arg_info) = NLUTremoveNlut (INFO_RCM_ENV (arg_info));
            INFO_RCM_USEDMASK (arg_info) = DFMremoveMask (INFO_RCM_USEDMASK (arg_info));
            maskbase = DFMremoveMaskBase (maskbase);
            arg_info = FreeInfo (arg_info);
        }

        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
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

    if (NODE_TYPE (LET_EXPR (arg_node)) == N_id) {
        /*
         * v = a;
         * Env( a) += Env( v);
         */
        NLUTsetNum (INFO_RCM_ENV (arg_info), ID_AVIS (LET_EXPR (arg_node)),
                    (NLUTgetNum (INFO_RCM_ENV (arg_info), ID_AVIS (LET_EXPR (arg_node)))
                     + NLUTgetNum (INFO_RCM_ENV (arg_info),
                                   IDS_AVIS (LET_IDS (arg_node)))));
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
            NLUTsetNum (INFO_RCM_ENV (arg_info), ID_AVIS (PRF_ARG1 (arg_node)),
                        (NLUTgetNum (INFO_RCM_ENV (arg_info),
                                     ID_AVIS (PRF_ARG1 (arg_node)))
                         + NUM_VAL (PRF_ARG2 (arg_node))));

            INFO_RCM_REMASSIGN (arg_info) = TRUE;
        } else {
            /*
             * This dec_rc( x, n) is the last reference to x.
             * Mark x as used.
             */
            DFMsetMaskEntrySet (INFO_RCM_USEDMASK (arg_info), NULL,
                                ID_AVIS (PRF_ARG1 (arg_node)));
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
