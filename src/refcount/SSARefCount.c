/*
 *
 * $Log$
 * Revision 1.13  2004/05/10 16:23:28  ktr
 * RCOracle inserted.
 *
 * Revision 1.12  2004/05/10 16:08:19  ktr
 * Removed some printf output.
 *
 * Revision 1.11  2004/05/06 17:51:10  ktr
 * SSARefCount now should handle IVE ICMs, too. :)
 *
 * Revision 1.10  2004/05/05 20:23:31  ktr
 * Home -> ISP
 *
 * Revision 1.9  2004/05/05 19:18:44  ktr
 * C functions should now be correctly refcounted.
 *
 * Revision 1.8  2004/05/05 15:34:05  ktr
 * Log added.
 *
 *
 */

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "traverse.h"
#include "free.h"
#include "dbug.h"
#include "DupTree.h"
#include "print.h"
#include "ssa.h"
#include "SSARefCount.h"

#define SSARC_DEVELOP

typedef enum {
    rc_undef,
    rc_return,
    rc_copy,
    rc_funap,
    rc_prfap,
    rc_array,
    rc_const,
    rc_cond,
    rc_funcond,
    rc_cexprs,
    rc_icm,
    rc_with
} rc_rhs_type;

typedef enum { rc_default, rc_else, rc_annotate_cfuns } rc_mode;

typedef struct RC_LIST_STRUCT {
    node *avis;
    int count;
    struct RC_LIST_STRUCT *next;
} rc_list_struct;

typedef struct RC_COUNTER {
    int depth;
    int count;
    struct RC_COUNTER *next;
} rc_counter;

/* Oracle to tell which parameters of a external function must be
   refcounted like primitive function parameters */
#define FUNDEF_EXT_NOT_REFCOUNTED(n, idx)                                                \
    ((FUNDEF_STATUS (n) == ST_Cfun)                                                      \
     && ((FUNDEF_PRAGMA (n) == NULL) || (FUNDEF_REFCOUNTING (n) == NULL)                 \
         || (PRAGMA_NUMPARAMS (FUNDEF_PRAGMA (n)) <= (idx))                              \
         || (!(FUNDEF_REFCOUNTING (n)[idx]))))

#define INFO_SSARC_MODE(n) ((rc_mode) (n->counter))
#define INFO_SSARC_DEPTH(n) (n->varno)
#define INFO_SSARC_RHS(n) ((rc_rhs_type) (n->flag))
#define INFO_SSARC_DECLIST(n) ((rc_list_struct *)(n->node[0]))
#define INFO_SSARC_INCLIST(n) ((rc_list_struct *)(n->node[1]))
#define INFO_SSARC_FUNDEF(n) (n->node[2])
#define INFO_SSARC_WITHID(n) (n->node[3])
#define INFO_SSARC_LHS_COUNT(n) (n->refcnt)
#define INFO_SSARC_FUNAP(n) (n->node[4])

#define AVIS_SSARC_COUNTED(n) ((bool)(n->info.cint))
#define AVIS_SSARC_COUNTER(n) ((rc_counter *)(n->dfmask[0]))
#define AVIS_SSARC_COUNTER2(n) ((rc_counter *)(n->dfmask[1]))

/**
 *
 *  HELPER FUNCTIONS
 *
 ****************************************************************************/
void
InitializeEnv (node *avis, node *arg_info)
{
    AVIS_SSARC_COUNTED (avis) = FALSE;
    AVIS_SSARC_COUNTER2 (avis) = NULL;
    AVIS_SSARC_COUNTER (avis) = NULL;
}

void
PushEnv (node *avis)
{
    if (AVIS_SSARC_COUNTED (avis) == FALSE) {
        AVIS_SSARC_COUNTER2 (avis) = AVIS_SSARC_COUNTER (avis);
        AVIS_SSARC_COUNTER (avis) = NULL;
    }
}

void
KillSecondEnv (node *avis)
{
    DBUG_ASSERT (AVIS_SSARC_COUNTER (avis) == NULL, "Environement != NULL!!!");
    AVIS_SSARC_COUNTER (avis) = AVIS_SSARC_COUNTER2 (avis);
    AVIS_SSARC_COUNTER2 (avis) = NULL;
    AVIS_SSARC_COUNTED (avis) = FALSE;
}

bool
AddEnvPar (node *avis, node *arg_info, int n, bool incOverOne)
{
    rc_counter *rcc;

    DBUG_ASSERT ((AVIS_SSARC_COUNTER (avis) == NULL)
                   || (INFO_SSARC_DEPTH (arg_info) >= AVIS_SSARC_COUNTER (avis)->depth),
                 "Illegal reference counter stack state");

    if ((AVIS_SSARC_COUNTER (avis) == NULL)
        || (INFO_SSARC_DEPTH (arg_info) > AVIS_SSARC_COUNTER (avis)->depth)) {
        rcc = Malloc (sizeof (rc_counter));
        rcc->depth = INFO_SSARC_DEPTH (arg_info);
        rcc->count = n;
        rcc->next = AVIS_SSARC_COUNTER (avis);
        AVIS_SSARC_COUNTER (avis) = rcc;
        return TRUE;
    } else {
        if (incOverOne)
            AVIS_SSARC_COUNTER (avis)->count += n;
        return FALSE;
    }
}

void
AddEnv (node *avis, node *arg_info, int n)
{
    AddEnvPar (avis, arg_info, n, TRUE);
}

bool
IncreaseEnv (node *avis, node *arg_info)
{
    return AddEnvPar (avis, arg_info, 1, TRUE);
}

bool
IncreaseEnvOnZero (node *avis, node *arg_info)
{
    return AddEnvPar (avis, arg_info, 1, FALSE);
}

bool
IsIndexVariable (node *avis, node *arg_info)
{
    node *wid;
    ids *i;

    wid = INFO_SSARC_WITHID (arg_info);
    if (wid == NULL)
        return FALSE;

    if ((NWITHID_VEC (wid) != NULL) && (IDS_AVIS (NWITHID_VEC (wid)) == avis))
        return TRUE;

    i = NWITHID_IDS (wid);
    while (i != NULL) {
        if (IDS_AVIS (i) == avis)
            return TRUE;
        i = IDS_NEXT (i);
    }

    return FALSE;
}

int
PopEnv (node *avis, node *arg_info)
{
    int res;
    rc_counter *rcc;

    DBUG_ASSERT (AVIS_SSARC_COUNTED (avis) == FALSE,
                 "Variable has already been counted!");

    AVIS_SSARC_COUNTED (avis) = TRUE;

    if (AVIS_SSARC_COUNTER (avis) == NULL)
        return 0;
    else {
        if (AVIS_SSARC_COUNTER (avis)->depth < INFO_SSARC_DEPTH (arg_info))
            return 0;
        else {
            DBUG_ASSERT (AVIS_SSARC_COUNTER (avis)->depth == INFO_SSARC_DEPTH (arg_info),
                         "Wrong depth!!!");
            rcc = AVIS_SSARC_COUNTER (avis);
            AVIS_SSARC_COUNTER (avis) = rcc->next;
            rcc->next = NULL;
            res = rcc->count;
            rcc = Free (rcc);

            return (res);
        }
    }
}

bool
DecList_HasNext (node *arg_info)
{
    return (INFO_SSARC_DECLIST (arg_info) != NULL);
}

void
DecList_Insert (node *arg_info, node *avis)
{
    rc_list_struct *rls = INFO_SSARC_DECLIST (arg_info);

    while ((rls != NULL) && (rls->avis != avis))
        rls = rls->next;

    if (rls != NULL) {
        rls->count -= 1;
    } else {
        rls = Malloc (sizeof (rc_list_struct));
        rls->avis = avis;
        rls->count = -1;
        rls->next = INFO_SSARC_DECLIST (arg_info);
        INFO_SSARC_DECLIST (arg_info) = rls;
    }
}

rc_list_struct *
RCLS_Remove (rc_list_struct *rls, node *avis)
{
    rc_list_struct *temp;

    if (rls->avis == avis) {
        temp = rls->next;
        Free (rls);
        return (temp);
    } else {
        if (rls->next != NULL)
            rls->next = RCLS_Remove (rls->next, avis);
        return (rls);
    }
}

void
DecList_Remove (node *arg_info, node *avis)
{
    if (INFO_SSARC_DECLIST (arg_info) != NULL)
        INFO_SSARC_DECLIST (arg_info) = RCLS_Remove (INFO_SSARC_DECLIST (arg_info), avis);
}

rc_list_struct *
DecList_PopNext (node *arg_info)
{
    rc_list_struct *res;

    DBUG_ASSERT (DecList_HasNext (arg_info), "DECLIST is empty!!");

    res = INFO_SSARC_DECLIST (arg_info);
    INFO_SSARC_DECLIST (arg_info) = res->next;

    return (res);
}

bool
IncList_HasNext (node *arg_info)
{
    return (INFO_SSARC_INCLIST (arg_info) != NULL);
}

void
IncList_Insert (node *arg_info, node *avis, int count)
{
    rc_list_struct *rls = INFO_SSARC_INCLIST (arg_info);

    while ((rls != NULL) && (rls->avis != avis))
        rls = rls->next;

    if (rls != NULL) {
        rls->count += count;
    } else {
        rls = Malloc (sizeof (rc_list_struct));
        rls->avis = avis;
        rls->count = count;
        rls->next = INFO_SSARC_INCLIST (arg_info);
        INFO_SSARC_INCLIST (arg_info) = rls;
    }
}

rc_list_struct *
IncList_PopNext (node *arg_info)
{
    rc_list_struct *res;

    DBUG_ASSERT (IncList_HasNext (arg_info), "INCLIST is empty!!");

    res = INFO_SSARC_INCLIST (arg_info);
    INFO_SSARC_INCLIST (arg_info) = res->next;

    return (res);
}

bool
RCOracle (node *avis, int count)
{
    return (count != 0);
}

node *
MakeAdjustRC (node *avis, int count, node *next_node)
{
    node *n;
    ids *ids1, *ids2;

    if (!RCOracle (avis, count))
        return next_node;

    ids1 = MakeIds (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (avis))), NULL, ST_regular);

    IDS_AVIS (ids1) = avis;
    IDS_VARDEC (ids1) = AVIS_VARDECORARG (avis);

    ids2 = MakeIds (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (avis))), NULL, ST_regular);

    IDS_AVIS (ids2) = avis;
    IDS_VARDEC (ids2) = AVIS_VARDECORARG (avis);

    n = MakeAssign (MakeLet (MakePrf (F_adjust_rc,
                                      MakeExprs (MakeIdFromIds (ids1),
                                                 MakeExprs (MakeNum (count), NULL))),
                             ids2),
                    (((next_node != NULL) && (NODE_TYPE (next_node) == N_assign))
                       ? next_node
                       : NULL));

    return (n);
}

node *
MakeAdjustRCfromRLS (rc_list_struct *rls, node *next_node)
{
    return MakeAdjustRC (rls->avis, rls->count, next_node);
}

/**
 *
 *  TRAVERSAL FUNCTIONS
 *
 ****************************************************************************/

node *
SSARCfundef (node *fundef, node *arg_info)
{
    int i;
    node *arg;

    DBUG_ENTER ("SSARCfundef");

    if (INFO_SSARC_MODE (arg_info) == rc_annotate_cfuns) {
        /*
         * special module name -> must be an external C-fun
         */
        if (((sbs == 1) && (strcmp (FUNDEF_MOD (fundef), EXTERN_MOD_NAME) == 0))
            || ((sbs == 0) && (FUNDEF_MOD (fundef) == NULL))) {
            FUNDEF_STATUS (fundef) = ST_Cfun;
        }
        if (FUNDEF_NEXT (fundef) != NULL)
            FUNDEF_NEXT (fundef) = Trav (FUNDEF_NEXT (fundef), arg_info);

        DBUG_RETURN (fundef);
    }

    INFO_SSARC_FUNDEF (arg_info) = fundef;
    INFO_SSARC_DEPTH (arg_info) = 0;
    INFO_SSARC_MODE (arg_info) = rc_default;
    INFO_SSARC_WITHID (arg_info) = NULL;

    /* Traverse args in order to initialize refcounting environment */
    if (FUNDEF_ARGS (fundef) != NULL)
        FUNDEF_ARGS (fundef) = Trav (FUNDEF_ARGS (fundef), arg_info);

    /* Traverse block */
    if (FUNDEF_BODY (fundef) != NULL) {
        FUNDEF_BODY (fundef) = Trav (FUNDEF_BODY (fundef), arg_info);

        /* Annotate missing ADJUST_RCs */
        arg = FUNDEF_ARGS (fundef);
        while (arg != NULL) {
            BLOCK_INSTR (FUNDEF_BODY (fundef))
              = MakeAdjustRC (ARG_AVIS (arg), PopEnv (ARG_AVIS (arg), arg_info) - 1,
                              BLOCK_INSTR (FUNDEF_BODY (fundef)));
            arg = ARG_NEXT (arg);
        }

        /* Restore SSA form */
        fundef = RestoreSSAOneFundef (fundef);
    }

    /* Traverse other fundefs */
    if (FUNDEF_NEXT (fundef) != NULL)
        FUNDEF_NEXT (fundef) = Trav (FUNDEF_NEXT (fundef), arg_info);

    DBUG_RETURN (fundef);
}

node *
SSARCarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSARCarg");

    InitializeEnv (ARG_AVIS (arg_node), arg_info);

    if (ARG_NEXT (arg_node) != NULL)
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
SSARCvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSARCvardec");

    InitializeEnv (VARDEC_AVIS (arg_node), arg_info);

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
SSARCblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSARCblock");

    /* Traverse vardecs in order to initialize RC-Counters */
    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }

    DBUG_ASSERT ((BLOCK_INSTR (arg_node) != NULL), "first instruction of block is NULL"
                                                   " (should be a N_empty node)");

    BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
SSARCprf (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSARCprf");

    INFO_SSARC_RHS (arg_info) = rc_prfap;

    if (PRF_ARGS (arg_node) != NULL)
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
SSARCap (node *arg_node, node *arg_info)
{
    node *args, *id;
    DBUG_ENTER ("SSARCap");

    INFO_SSARC_RHS (arg_info) = rc_funap;

    INFO_SSARC_FUNAP (arg_info) = AP_FUNDEF (arg_node);

    args = AP_ARGS (arg_node);
    while (args != NULL) {
        if ((EXPRS_EXPR (args) != NULL) && (NODE_TYPE (EXPRS_EXPR (args)) == N_id)) {
            id = EXPRS_EXPR (args);

            if (FUNDEF_EXT_NOT_REFCOUNTED (INFO_SSARC_FUNAP (arg_info),
                                           INFO_SSARC_LHS_COUNT (arg_info))) {
                /* Add one to the environment iff it is zero */
                if (IncreaseEnvOnZero (ID_AVIS (id), arg_info))

                    /* Put id into declist as we are traversing a N_prf */
                    DecList_Insert (arg_info, ID_AVIS (id));
            } else {
                /* Add one to the environment */
                IncreaseEnv (ID_AVIS (id), arg_info);

                /* Don't put id into declist as we N_return and N_funap
                   are implicitly consuming references */
            }
        }
        INFO_SSARC_LHS_COUNT (arg_info) += 1;
        args = EXPRS_NEXT (args);
    }

    DBUG_RETURN (arg_node);
}

node *
SSARCarray (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSARCarray");

    INFO_SSARC_RHS (arg_info) = rc_array;

    if (ARRAY_AELEMS (arg_node) != NULL)
        ARRAY_AELEMS (arg_node) = Trav (ARRAY_AELEMS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
SSARCreturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSARCreturn");

    INFO_SSARC_RHS (arg_info) = rc_return;

    if (RETURN_EXPRS (arg_node) != NULL)
        RETURN_EXPRS (arg_node) = Trav (RETURN_EXPRS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
SSARCcond (node *arg_node, node *arg_info)
{
    int m, t, e;
    node *n;

    DBUG_ENTER ("SSARCcond");

    if (INFO_SSARC_MODE (arg_info) == rc_default) {
        COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);
        if (IncreaseEnvOnZero (ID_AVIS (COND_COND (arg_node)), arg_info))
            BLOCK_INSTR (COND_THEN (arg_node))
              = MakeAdjustRC (ID_AVIS (COND_COND (arg_node)), -1,
                              BLOCK_INSTR (COND_THEN (arg_node)));

    } else {
        COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);
        if (IncreaseEnvOnZero (ID_AVIS (COND_COND (arg_node)), arg_info))
            BLOCK_INSTR (COND_ELSE (arg_node))
              = MakeAdjustRC (ID_AVIS (COND_COND (arg_node)), -1,
                              BLOCK_INSTR (COND_ELSE (arg_node)));

        /* After both environments have been created,
           annote missing ADJUST_RCs at the beginning of blocks and
           simultaneously merge both environments */
        n = FUNDEF_ARGS (INFO_SSARC_FUNDEF (arg_info));
        while (n != NULL) {
            e = PopEnv (ARG_AVIS (n), arg_info);
            KillSecondEnv (ARG_AVIS (n));
            t = PopEnv (ARG_AVIS (n), arg_info);
            KillSecondEnv (ARG_AVIS (n));

            if (e + t > 0) {
                m = e < t ? e : t;
                m = m > 1 ? m : 1;
                AddEnv (ARG_AVIS (n), arg_info, m);
                BLOCK_INSTR (COND_THEN (arg_node))
                  = MakeAdjustRC (ARG_AVIS (n), t - m,
                                  BLOCK_INSTR (COND_THEN (arg_node)));
                BLOCK_INSTR (COND_ELSE (arg_node))
                  = MakeAdjustRC (ARG_AVIS (n), e - m,
                                  BLOCK_INSTR (COND_ELSE (arg_node)));
            }
            n = ARG_NEXT (n);
        }

        n = BLOCK_VARDEC (FUNDEF_BODY (INFO_SSARC_FUNDEF (arg_info)));
        while (n != NULL) {
            if (AVIS_SSARC_COUNTED (VARDEC_AVIS (n)) == FALSE) {
                e = PopEnv (VARDEC_AVIS (n), arg_info);
                KillSecondEnv (VARDEC_AVIS (n));
                t = PopEnv (VARDEC_AVIS (n), arg_info);
                KillSecondEnv (VARDEC_AVIS (n));

                if (e + t > 0) {
                    m = e < t ? e : t;
                    m = m > 1 ? m : 1;
                    AddEnv (VARDEC_AVIS (n), arg_info, m);
                    BLOCK_INSTR (COND_THEN (arg_node))
                      = MakeAdjustRC (VARDEC_AVIS (n), t - m,
                                      BLOCK_INSTR (COND_THEN (arg_node)));
                    BLOCK_INSTR (COND_ELSE (arg_node))
                      = MakeAdjustRC (VARDEC_AVIS (n), e - m,
                                      BLOCK_INSTR (COND_ELSE (arg_node)));
                }
            }
            n = VARDEC_NEXT (n);
        }
    }

    INFO_SSARC_RHS (arg_info) = rc_cond;

    DBUG_RETURN (arg_node);
}

node *
SSARClet (node *arg_node, node *arg_info)
{
    ids *ids;
    int n;

    DBUG_ENTER ("SSARClet");

    /* Count LHS values such that we can identify which fun ap parameters
       must be refcounted like prf paramters using PRAGMA_REFCOUNTING */
    INFO_SSARC_LHS_COUNT (arg_info) = CountIds (LET_IDS (arg_node));

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    switch (INFO_SSARC_RHS (arg_info)) {
    case rc_funap:
        /* Add all lhs ids to inclist */
        n = 0;
        ids = LET_IDS (arg_node);
        while (ids != NULL) {
            IncList_Insert (arg_info, IDS_AVIS (ids),
                            PopEnv (IDS_AVIS (ids), arg_info)
                              + (FUNDEF_EXT_NOT_REFCOUNTED (INFO_SSARC_FUNAP (arg_info),
                                                            n)
                                   ? 0
                                   : -1));
            n = n + 1;
            ids = IDS_NEXT (ids);
        }
        break;
    case rc_prfap:
    case rc_const:
    case rc_array:
    case rc_with:
        /* Add all lhs ids to inclist */
        ids = LET_IDS (arg_node);
        while (ids != NULL) {
            IncList_Insert (arg_info, IDS_AVIS (ids), PopEnv (IDS_AVIS (ids), arg_info));
            ids = IDS_NEXT (ids);
        }
        break;
    case rc_copy:
        /* Copy assignment: a=b */
        /* Env(b) += Env(a) */
        AddEnv (IDS_AVIS (ID_IDS (LET_EXPR (arg_node))), arg_info,
                PopEnv (IDS_AVIS (LET_IDS (arg_node)), arg_info));
        break;
    case rc_funcond:
        /* Treat FunCond like a variable Copy assignment */
        switch (INFO_SSARC_MODE (arg_info)) {
        case rc_default:
            AddEnv (IDS_AVIS (ID_IDS (EXPRS_EXPR (FUNCOND_THEN (LET_EXPR (arg_node))))),
                    arg_info, PopEnv (IDS_AVIS (LET_IDS (arg_node)), arg_info));
            KillSecondEnv (IDS_AVIS (LET_IDS (arg_node)));
            break;
        case rc_else:
            AddEnv (IDS_AVIS (ID_IDS (EXPRS_EXPR (FUNCOND_ELSE (LET_EXPR (arg_node))))),
                    arg_info, PopEnv (IDS_AVIS (LET_IDS (arg_node)), arg_info));
            break;
        default:
            DBUG_ASSERT (FALSE, "Cannot happen");
        }
        break;
    default:
        Print (arg_node);
        DBUG_ASSERT (FALSE, "Cannot happen");
    }
    DBUG_RETURN (arg_node);
}

node *
SSARCicm (node *arg_node, node *arg_info)
{
    char *name;

    DBUG_ENTER ("SSARCicm");

    INFO_SSARC_RHS (arg_info) = rc_icm;

    name = ICM_NAME (arg_node);

    if (strstr (name, "USE_GENVAR_OFFSET") != NULL) {
        /*
         * USE_GENVAR_OFFSET( off_nt, wl_nt)
         * does *not* consume its arguments! It is expanded to
         *      off_nt = wl_nt__off    ,
         * where 'off_nt' is a scalar and 'wl_nt__off' an internal variable!
         *   -> store actual RC of the first argument (defined)
         *   -> do NOT traverse the second argument (used)
         */
        IncList_Insert (arg_info, IDS_AVIS (ID_IDS (ICM_ARG1 (arg_node))),
                        PopEnv (IDS_AVIS (ID_IDS (ICM_ARG1 (arg_node))), arg_info));
    } else if (strstr (name, "VECT2OFFSET") != NULL) {
        /*
         * VECT2OFFSET( off_nt, ., from_nt, ., ., ...)
         * needs RC on all but the first argument. It is expanded to
         *     off_nt = ... from_nt ...    ,
         * where 'off_nt' is a scalar variable.
         *  -> store actual RC of the first argument (defined)
         *  -> traverse all but the first argument (used)
         *  -> handle ICM like a prf (RCO)
         */
        IncList_Insert (arg_info, IDS_AVIS (ID_IDS (ICM_ARG1 (arg_node))),
                        PopEnv (IDS_AVIS (ID_IDS (ICM_ARG1 (arg_node))), arg_info));

        ICM_EXPRS2 (arg_node) = Trav (ICM_EXPRS2 (arg_node), arg_info);
    } else if (strstr (name, "IDXS2OFFSET") != NULL) {
        /*
         * IDXS2OFFSET( off_nt, ., idx_1_nt ... idx_n_nt, ., ...)
         * needs RC on all but the first argument. It is expanded to
         *     off_nt = ... idx_1_nt[i] ... idx_n_nt[i] ...   ,
         * where 'off_nt' is a scalar variable.
         *  -> store actual RC of the first argument (defined)
         *  -> traverse all but the first argument (used)
         *  -> handle ICM like a prf (RCO)
         */
        IncList_Insert (arg_info, IDS_AVIS (ID_IDS (ICM_ARG1 (arg_node))),
                        PopEnv (IDS_AVIS (ID_IDS (ICM_ARG1 (arg_node))), arg_info));

        ICM_EXPRS2 (arg_node) = Trav (ICM_EXPRS2 (arg_node), arg_info);
    } else {
        DBUG_ASSERT ((0), "unknown ICM found during SSARC");
    }

    DBUG_RETURN (arg_node);
}
node *
SSARCfuncond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSARCfuncond");

    INFO_SSARC_RHS (arg_info) = rc_funcond;

    DBUG_RETURN (arg_node);
}

node *
SSARCid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSARCid");

    switch (INFO_SSARC_RHS (arg_info)) {
    case rc_undef:
        INFO_SSARC_RHS (arg_info) = rc_copy;
        break;
    case rc_return:
        /* Add one to the environment */
        IncreaseEnv (ID_AVIS (arg_node), arg_info);

        /* Don't put id into declist as we N_return and N_funap
           are implicitly consuming references */
        break;

    case rc_array:
        /* Here is no break missing! */
    case rc_prfap:
        /* Here is no break missing! */
    case rc_cond:
        /* Here is no break missing! */
    case rc_icm:
        /* Here is no break missing! */
    case rc_cexprs:
        /* Here is no break missing! */
    case rc_with:
        /* Add one to the environment iff it is zero */
        if (IncreaseEnvOnZero (ID_AVIS (arg_node), arg_info))

            /* Put id into declist as we are traversing a N_prf */
            DecList_Insert (arg_info, ID_AVIS (arg_node));
        break;
    case rc_funcond:
        /* Here is no break missing! */
    case rc_funap:
        /* Here is no break missing! */
    case rc_const:
        /* Here is no break missing! */
    case rc_copy:
        Print (arg_node);
        DBUG_ASSERT (FALSE, "No ID node must appear in this context");
    }
    DBUG_RETURN (arg_node);
}

node *
SSARCnum (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSARCnum");

    if (INFO_SSARC_RHS (arg_info) == rc_undef)
        INFO_SSARC_RHS (arg_info) = rc_const;

    DBUG_RETURN (arg_node);
}

node *
SSARCchar (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSARCchar");

    if (INFO_SSARC_RHS (arg_info) == rc_undef)
        INFO_SSARC_RHS (arg_info) = rc_const;

    DBUG_RETURN (arg_node);
}

node *
SSARCbool (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSARCbool");

    if (INFO_SSARC_RHS (arg_info) == rc_undef)
        INFO_SSARC_RHS (arg_info) = rc_const;

    DBUG_RETURN (arg_node);
}

node *
SSARCfloat (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSARCfloat");

    if (INFO_SSARC_RHS (arg_info) == rc_undef)
        INFO_SSARC_RHS (arg_info) = rc_const;

    DBUG_RETURN (arg_node);
}

node *
SSARCdouble (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSARCdouble");

    if (INFO_SSARC_RHS (arg_info) == rc_undef)
        INFO_SSARC_RHS (arg_info) = rc_const;

    DBUG_RETURN (arg_node);
}

node *
SSARCstr (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSARCstr");

    if (INFO_SSARC_RHS (arg_info) == rc_undef)
        INFO_SSARC_RHS (arg_info) = rc_const;

    DBUG_RETURN (arg_node);
}

node *
SSARCNwith (node *arg_node, node *arg_info)
{
    node *oldwithid;

    DBUG_ENTER ("SSARCNwith");

    oldwithid = INFO_SSARC_WITHID (arg_info);

    if (NODE_TYPE (arg_node) == N_Nwith) {
        INFO_SSARC_DEPTH (arg_info) += 1;
        INFO_SSARC_WITHID (arg_info) = NWITH_WITHID (arg_node);
        if (NWITH_CODE (arg_node) != NULL)
            NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
        INFO_SSARC_DEPTH (arg_info) -= 1;
        INFO_SSARC_RHS (arg_info) = rc_with;
        NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
        NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);
    } else {
        INFO_SSARC_DEPTH (arg_info) += 1;
        INFO_SSARC_WITHID (arg_info) = NWITH2_WITHID (arg_node);
        if (NWITH2_CODE (arg_node) != NULL)
            NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
        INFO_SSARC_DEPTH (arg_info) -= 1;
        INFO_SSARC_RHS (arg_info) = rc_with;
        NWITH2_WITHID (arg_node) = Trav (NWITH2_WITHID (arg_node), arg_info);
        NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);
        NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);
    }

    INFO_SSARC_WITHID (arg_info) = oldwithid;

    DBUG_RETURN (arg_node);
}

node *
SSARCNcode (node *arg_node, node *arg_info)
{
    node *n, *a;
    rc_list_struct *declist, *rls;
    int env;

    DBUG_ENTER ("SSARCNcode");

    /* Save DecList */
    declist = INFO_SSARC_DECLIST (arg_info);
    INFO_SSARC_DECLIST (arg_info) = NULL;

    /* Traverse CEXPRS and insert adjust_rc operations into
       NCODE_EPILOGUE */
    INFO_SSARC_RHS (arg_info) = rc_cexprs;
    NCODE_CEXPRS (arg_node) = Trav (NCODE_CEXPRS (arg_node), arg_info);

    DBUG_ASSERT (NCODE_EPILOGUE (arg_node) == NULL, "Epilogue must not exist yet!");

    /* Insert ADJUST_RC prfs from DECLIST into EPILOGUE*/
    a = NULL;
    while (DecList_HasNext (arg_info)) {
        rls = DecList_PopNext (arg_info);
        a = MakeAdjustRCfromRLS (rls, a);
        rls = Free (rls);
    }

    if (a != NULL)
        NCODE_EPILOGUE (arg_node) = MakeBlock (a, NULL);

    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);

    /* Restore DecList */
    DBUG_ASSERT (INFO_SSARC_DECLIST (arg_info) == NULL, "DecList must be NULL");
    INFO_SSARC_DECLIST (arg_info) = declist;

    /* Prepend block with Adjust_RC prfs */
    n = FUNDEF_ARGS (INFO_SSARC_FUNDEF (arg_info));
    while (n != NULL) {
        env = PopEnv (ARG_AVIS (n), arg_info);

        AVIS_SSARC_COUNTED (ARG_AVIS (n)) = FALSE;

        if (env > 0) {
            BLOCK_INSTR (NCODE_CBLOCK (arg_node))
              = MakeAdjustRC (ARG_AVIS (n), env, BLOCK_INSTR (NCODE_CBLOCK (arg_node)));

            INFO_SSARC_DEPTH (arg_info) -= 1;
            if (IncreaseEnvOnZero (ARG_AVIS (n), arg_info))
                DecList_Insert (arg_info, ARG_AVIS (n));
            INFO_SSARC_DEPTH (arg_info) += 1;
        }
        n = ARG_NEXT (n);
    }

    n = BLOCK_VARDEC (FUNDEF_BODY (INFO_SSARC_FUNDEF (arg_info)));
    while (n != NULL) {
        if (AVIS_SSARC_COUNTED (VARDEC_AVIS (n)) == FALSE) {
            env = PopEnv (VARDEC_AVIS (n), arg_info)
                  + (IsIndexVariable (VARDEC_AVIS (n), arg_info) ? -1 : 0);

            AVIS_SSARC_COUNTED (VARDEC_AVIS (n)) = FALSE;

            if (env != 0) {
                BLOCK_INSTR (NCODE_CBLOCK (arg_node))
                  = MakeAdjustRC (VARDEC_AVIS (n), env,
                                  BLOCK_INSTR (NCODE_CBLOCK (arg_node)));

                INFO_SSARC_DEPTH (arg_info) -= 1;
                if (IncreaseEnvOnZero (VARDEC_AVIS (n), arg_info))
                    DecList_Insert (arg_info, VARDEC_AVIS (n));
                INFO_SSARC_DEPTH (arg_info) += 1;
            }
        }
        n = VARDEC_NEXT (n);
    }

    /*
     * count the references in next code
     */
    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
SSARCNwithid (node *arg_node, node *arg_info)
{
    ids *iv_ids;
    DBUG_ENTER ("SSARCNwithid");

    /* Index vector and Index variables must be removed from DecList */
    if (NWITHID_VEC (arg_node) != NULL)
        DecList_Remove (arg_info, IDS_AVIS (NWITHID_VEC (arg_node)));

    iv_ids = NWITHID_IDS (arg_node);
    while (iv_ids != NULL) {
        DecList_Remove (arg_info, IDS_AVIS (iv_ids));
        iv_ids = IDS_NEXT (iv_ids);
    }

    DBUG_RETURN (arg_node);
}

node *
SSARCassign (node *arg_node, node *arg_info)
{
    rc_list_struct *rls;
    node *n;

    DBUG_ENTER ("SSARCassign");

    /*
     * Bottom up traversal!!
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    INFO_SSARC_RHS (arg_info) = rc_undef;
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    /* If this node happens to be a conditional,
       traverse again! */
    if (INFO_SSARC_RHS (arg_info) == rc_cond) {
        INFO_SSARC_MODE (arg_info) = rc_else;

        n = FUNDEF_ARGS (INFO_SSARC_FUNDEF (arg_info));
        while (n != NULL) {
            PushEnv (ARG_AVIS (n));
            n = ARG_NEXT (n);
        }

        n = BLOCK_VARDEC (FUNDEF_BODY (INFO_SSARC_FUNDEF (arg_info)));
        while (n != NULL) {
            PushEnv (VARDEC_AVIS (n));
            n = VARDEC_NEXT (n);
        }

        if (ASSIGN_NEXT (arg_node) != NULL) {
            ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        }

        INFO_SSARC_RHS (arg_info) = rc_undef;
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

        INFO_SSARC_MODE (arg_node) = rc_default;
    }

    /* Insert ADJUST_RC prfs from DECLIST */
    while (DecList_HasNext (arg_info)) {
        rls = DecList_PopNext (arg_info);
        ASSIGN_NEXT (arg_node) = MakeAdjustRCfromRLS (rls, ASSIGN_NEXT (arg_node));
        rls = Free (rls);
    }

    /* Insert ADJUST_RC prfs from INCLIST */
    while (IncList_HasNext (arg_info)) {
        rls = IncList_PopNext (arg_info);
        ASSIGN_NEXT (arg_node) = MakeAdjustRCfromRLS (rls, ASSIGN_NEXT (arg_node));
        rls = Free (rls);
    }

    DBUG_RETURN (arg_node);
}

node *
SSARefCount (node *syntax_tree)
{
    node *info;

    DBUG_ENTER ("SSARefCount");

    info = MakeInfo ();

    INFO_SSARC_INCLIST (info) = NULL;
    INFO_SSARC_DECLIST (info) = NULL;

    act_tab = ssarefcnt_tab;
    INFO_SSARC_MODE (info) = rc_annotate_cfuns;
    syntax_tree = Trav (syntax_tree, info);

    INFO_SSARC_MODE (info) = rc_default;
    syntax_tree = Trav (syntax_tree, info);

    info = FreeTree (info);

    /*  syntax_tree = UndoSSA( syntax_tree ); */

    show_refcnt = FALSE;

    DBUG_RETURN (syntax_tree);
}
