/*
 *
 * $Log$
 * Revision 1.16  2004/06/07 12:39:33  ktr
 * Invalid assumptions about C evaluation order had been made which led
 * to nasty lockups on x86-Systems.
 *
 * Revision 1.15  2004/06/03 15:22:53  ktr
 * New version featuring:
 * - alloc_or_reuse
 * - fill
 * - explicit index vector allocation
 *
 * Revision 1.14  2004/05/12 13:05:05  ktr
 * UndeSSA removed
 *
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

typedef struct AR_LIST_STRUCT {
    node *avis;
    node *exprs; /* shape1, shape2, candidates */
    struct AR_LIST_STRUCT *next;
} ar_list_struct;

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
#define INFO_SSARC_ALLOCLIST(n) ((ar_list_struct *)(n->node[2]))
#define INFO_SSARC_FUNDEF(n) (n->node[3])
#define INFO_SSARC_WITHID(n) (n->node[4])
#define INFO_SSARC_LHS_COUNT(n) (n->refcnt)
#define INFO_SSARC_FUNAP(n) (n->node[5])
#define INFO_SSARC_REUSELIST(n) ((node *)(n->dfmask[0]))

#define AVIS_SSARC_DEFLEVEL(n) (n->int_data)
#define AVIS_SSARC_COUNTER(n) ((rc_counter *)(n->dfmask[0]))
#define AVIS_SSARC_COUNTER2(n) ((rc_counter *)(n->dfmask[1]))

/**
 *
 * RC LISTSTRUCT FUNCTIONS
 *
 ****************************************************************************/
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

rc_list_struct *
MakeRCLS (node *avis, int counter, rc_list_struct *next)
{
    rc_list_struct *rls = Malloc (sizeof (rc_list_struct));

    rls->avis = avis;
    rls->count = counter;
    rls->next = next;

    return (rls);
}

/**
 *
 * DECLIST FUNCTIONS
 *
 ****************************************************************************/
bool
DecList_HasNext (node *arg_info, int depth)
{
    return ((INFO_SSARC_DECLIST (arg_info) != NULL)
            && (INFO_SSARC_DECLIST (arg_info)->count == depth));
}

void
DecList_Insert (node *arg_info, node *avis, int depth)
{
    rc_list_struct *rls;

    if ((INFO_SSARC_DECLIST (arg_info) == NULL)
        || (INFO_SSARC_DECLIST (arg_info)->count < depth))
        INFO_SSARC_DECLIST (arg_info)
          = MakeRCLS (avis, depth, INFO_SSARC_DECLIST (arg_info));
    else {
        rls = INFO_SSARC_DECLIST (arg_info);
        while ((rls->next != NULL) && (rls->next->count >= depth))
            rls = rls->next;
        rls->next = MakeRCLS (avis, depth, rls->next);
    }
}

bool
DecList_Contains (node *arg_info, node *avis, int depth)
{
    rc_list_struct *rls = INFO_SSARC_DECLIST (arg_info);
    while (rls != NULL) {
        if ((rls->avis == avis) && (rls->count == depth))
            return TRUE;
        rls = rls->next;
    }
    return FALSE;
}

rc_list_struct *
DecList_PopNext (node *arg_info, int depth)
{
    rc_list_struct *res;

    DBUG_ASSERT (DecList_HasNext (arg_info, depth), "DECLIST is empty!!");

    res = INFO_SSARC_DECLIST (arg_info);
    INFO_SSARC_DECLIST (arg_info) = res->next;
    res->count = -1;

    return (res);
}

/**
 *
 *  ENVIRONMENT FUNCTIONS
 *
 ****************************************************************************/
rc_counter *
MakeRCCounter (int depth, int count, rc_counter *next)
{
    rc_counter *rcc;

    rcc = Malloc (sizeof (rc_counter));
    rcc->depth = depth;
    rcc->count = count;
    rcc->next = next;

    return (rcc);
}

void
InitializeEnv (node *avis, int c)
{
    AVIS_SSARC_DEFLEVEL (avis) = c;
    AVIS_SSARC_COUNTER2 (avis) = NULL;
    AVIS_SSARC_COUNTER (avis) = NULL;
}

void
PushEnv (node *avis)
{
    AVIS_SSARC_COUNTER2 (avis) = AVIS_SSARC_COUNTER (avis);
    AVIS_SSARC_COUNTER (avis) = NULL;
}

void
KillSecondEnv (node *avis)
{
    DBUG_ASSERT (AVIS_SSARC_COUNTER (avis) == NULL, "Environment != NULL!!!");
    AVIS_SSARC_COUNTER (avis) = AVIS_SSARC_COUNTER2 (avis);
    AVIS_SSARC_COUNTER2 (avis) = NULL;
}

int
GetEnv (node *avis, int depth)
{
    rc_counter *rcc = AVIS_SSARC_COUNTER (avis);

    while ((rcc != NULL) && (rcc->depth > depth))
        rcc = rcc->next;

    if ((rcc != NULL) && (rcc->depth == depth))
        return (rcc->count);
    else
        return (0);
}

rc_counter *
IncreaseEnvCounter (rc_counter *rcc, int depth, int n)
{
    if (rcc == NULL)
        return (MakeRCCounter (depth, n, NULL));
    else {
        if (rcc->depth == depth) {
            rcc->count += n;
        } else {
            rcc->next = IncreaseEnvCounter (rcc->next, depth, n);
        }
        return (rcc);
    }
}

bool
IncreaseEnvOnZero (node *arg_info, node *avis, int depth)
{
    if (depth == AVIS_SSARC_DEFLEVEL (avis)) {
        if (GetEnv (avis, depth) == 0) {
            AVIS_SSARC_COUNTER (avis)
              = IncreaseEnvCounter (AVIS_SSARC_COUNTER (avis), depth, 1);
            DecList_Insert (arg_info, avis, depth);
            return (TRUE);
        } else
            return (FALSE);
        return (DecList_Contains (arg_info, avis, depth));
    } else {
        return (IncreaseEnvOnZero (arg_info, avis, AVIS_SSARC_DEFLEVEL (avis)));
    }
}

void
AddEnv (node *arg_info, node *avis, int depth, int n)
{
    AVIS_SSARC_COUNTER (avis) = IncreaseEnvCounter (AVIS_SSARC_COUNTER (avis), depth, n);
    IncreaseEnvOnZero (arg_info, avis, AVIS_SSARC_DEFLEVEL (avis));
}

void
IncreaseEnv (node *arg_info, node *avis, int depth)
{
    AddEnv (arg_info, avis, depth, 1);
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
PopEnv (node *avis, int depth)
{
    rc_counter *tmp;
    int res;

    res = GetEnv (avis, depth);

    while ((AVIS_SSARC_COUNTER (avis) != NULL)
           && (AVIS_SSARC_COUNTER (avis)->depth >= depth)) {
        tmp = AVIS_SSARC_COUNTER (avis)->next;
        Free (AVIS_SSARC_COUNTER (avis));
        AVIS_SSARC_COUNTER (avis) = tmp;
    }

    return (res);
}

void
SetDefLevel (node *avis, int depth)
{
    if (AVIS_SSARC_DEFLEVEL (avis) == -1)
        AVIS_SSARC_DEFLEVEL (avis) = depth;
}

/**
 *
 * INCLIST FUNCTIONS
 *
 ****************************************************************************/
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
        INFO_SSARC_INCLIST (arg_info)
          = MakeRCLS (avis, count, INFO_SSARC_INCLIST (arg_info));
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

/**
 *
 * ADJUST_RC HELPER FUNCTIONS
 *
 ****************************************************************************/
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
MakeAdjustRCFromRLS (rc_list_struct *rls, node *next_node)
{
    node *res = MakeAdjustRC (rls->avis, rls->count, next_node);
    Free (rls);
    return (res);
}

/**
 *
 *  ALLOC_OR_REUSE LIST FUNCTIONS
 *
 ****************************************************************************/

bool
AllocList_HasNext (node *arg_info)
{
    return (INFO_SSARC_ALLOCLIST (arg_info) != NULL);
}

ar_list_struct *
AllocList_PopNext (node *arg_info)
{
    ar_list_struct *res;

    DBUG_ASSERT (AllocList_HasNext (arg_info), "ALLOCLIST is empty!!");

    res = INFO_SSARC_ALLOCLIST (arg_info);
    INFO_SSARC_ALLOCLIST (arg_info) = res->next;

    return (res);
}

node *
MakeAllocOrReuseFromALS (ar_list_struct *als, node *next_node)
{
    node *n;
    ids *ids;

    ids = MakeIds (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (als->avis))), NULL,
                   ST_regular);

    IDS_AVIS (ids) = als->avis;
    IDS_VARDEC (ids) = AVIS_VARDECORARG (als->avis);

    n = MakeAssign (MakeLet (MakePrf (F_alloc_or_reuse, als->exprs), ids), next_node);

    als->exprs = NULL;
    Free (als);

    return (n);
}

void
AllocList_Insert (node *arg_info, node *avis, node *shp1, node *shp2, node *cand)
{
    ar_list_struct *als;

    DBUG_ASSERT (shp1 != NULL, "Outer shape must not be NULL");
    DBUG_ASSERT (shp2 != NULL, "Inner shape must not be NULL");

    als = Malloc (sizeof (ar_list_struct));
    als->avis = avis;
    als->exprs = MakeExprs (shp1, MakeExprs (shp2, cand));

    als->next = INFO_SSARC_ALLOCLIST (arg_info);
    INFO_SSARC_ALLOCLIST (arg_info) = als;
}

/**
 *
 * *List -> Assigment conversion
 *
 ****************************************************************************/
node *
MakeDecAssignments (node *arg_info, node *next_node)
{
    rc_list_struct *rls;

    if (DecList_HasNext (arg_info, INFO_SSARC_DEPTH (arg_info))) {
        rls = DecList_PopNext (arg_info, INFO_SSARC_DEPTH (arg_info));
        return (MakeAdjustRCFromRLS (rls, MakeDecAssignments (arg_info, next_node)));
    } else {
        return (next_node);
    }
}

node *
MakeIncAssignments (node *arg_info, node *next_node)
{
    rc_list_struct *rls;

    if (IncList_HasNext (arg_info)) {
        rls = IncList_PopNext (arg_info);
        return (MakeAdjustRCFromRLS (rls, MakeIncAssignments (arg_info, next_node)));
    } else {
        return (next_node);
    }
}

node *
MakeAllocAssignments (node *arg_info, node *next_node)
{
    ar_list_struct *als;

    if (AllocList_HasNext (arg_info)) {
        als = AllocList_PopNext (arg_info);
        return (
          MakeAllocOrReuseFromALS (als, MakeAllocAssignments (arg_info, next_node)));
    } else {
        return (next_node);
    }
}

/**
 *
 *  TRAVERSAL FUNCTIONS
 *
 ****************************************************************************/

node *
SSARCfundef (node *fundef, node *arg_info)
{
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
            IncList_Insert (arg_info, ARG_AVIS (arg),
                            PopEnv (ARG_AVIS (arg), INFO_SSARC_DEPTH (arg_info)) - 1);
            arg = ARG_NEXT (arg);
        }
        BLOCK_INSTR (FUNDEF_BODY (fundef))
          = MakeIncAssignments (arg_info, BLOCK_INSTR (FUNDEF_BODY (fundef)));

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

    InitializeEnv (ARG_AVIS (arg_node), 0);

    if (ARG_NEXT (arg_node) != NULL)
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
SSARCvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSARCvardec");

    InitializeEnv (VARDEC_AVIS (arg_node), -1);

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
                IncreaseEnvOnZero (arg_info, ID_AVIS (id), INFO_SSARC_DEPTH (arg_info));
            } else {
                /* Add one to the environment */
                IncreaseEnv (arg_info, ID_AVIS (id), INFO_SSARC_DEPTH (arg_info));
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

void
AnnotateBranches (node *avis, node *cond, node *arg_info)
{
    int m, t, e;

    DBUG_ASSERT (INFO_SSARC_DEPTH (arg_info) == 0, "Invalid DEPTH: != 0");

    e = PopEnv (avis, INFO_SSARC_DEPTH (arg_info));
    KillSecondEnv (avis);
    t = PopEnv (avis, INFO_SSARC_DEPTH (arg_info));
    KillSecondEnv (avis);

    if (e + t > 0) {
        m = e < t ? e : t;
        m = m > 1 ? m : 1;
        AddEnv (arg_info, avis, INFO_SSARC_DEPTH (arg_info), m);

        IncList_Insert (arg_info, avis, t - m);
        BLOCK_INSTR (COND_THEN (cond))
          = MakeIncAssignments (arg_info, BLOCK_INSTR (COND_THEN (cond)));

        IncList_Insert (arg_info, avis, e - m);
        BLOCK_INSTR (COND_ELSE (cond))
          = MakeIncAssignments (arg_info, BLOCK_INSTR (COND_ELSE (cond)));
    }
}

node *
SSARCcond (node *arg_node, node *arg_info)
{
    node *n;

    DBUG_ENTER ("SSARCcond");

    if (INFO_SSARC_MODE (arg_info) == rc_default) {
        COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);

        /* Insert Adjust_RC for COND_COND into THEN-Branch */
        IncreaseEnvOnZero (arg_info, ID_AVIS (COND_COND (arg_node)),
                           INFO_SSARC_DEPTH (arg_info));
        BLOCK_INSTR (COND_THEN (arg_node))
          = MakeDecAssignments (arg_info, BLOCK_INSTR (COND_THEN (arg_node)));

    } else {
        COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);

        /* Insert Adjust_RC for COND_COND into ELSE-Branch */
        IncreaseEnvOnZero (arg_info, ID_AVIS (COND_COND (arg_node)),
                           INFO_SSARC_DEPTH (arg_info));
        BLOCK_INSTR (COND_ELSE (arg_node))
          = MakeDecAssignments (arg_info, BLOCK_INSTR (COND_ELSE (arg_node)));

        /* After both environments have been created,
           annote missing ADJUST_RCs at the beginning of blocks and
           simultaneously merge both environments */
        n = FUNDEF_ARGS (INFO_SSARC_FUNDEF (arg_info));
        while (n != NULL) {
            AnnotateBranches (ARG_AVIS (n), arg_node, arg_info);
            n = ARG_NEXT (n);
        }

        n = BLOCK_VARDEC (FUNDEF_BODY (INFO_SSARC_FUNDEF (arg_info)));
        while (n != NULL) {
            AnnotateBranches (VARDEC_AVIS (n), arg_node, arg_info);
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
            /* Commented out, because
               we demand that ALL functions yield results with RC == 1

               IncList_Insert(arg_info, IDS_AVIS(ids),
                           PopEnv(IDS_AVIS(ids), arg_info) +
                           (FUNDEF_EXT_NOT_REFCOUNTED(INFO_SSARC_FUNAP(arg_info),
                                                      n) ? 0 : -1));
            */
            IncList_Insert (arg_info, IDS_AVIS (ids),
                            PopEnv (IDS_AVIS (ids), INFO_SSARC_DEPTH (arg_info)) - 1);
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

        /* Insert fill-prf */
        LET_EXPR (arg_node)
          = MakePrf (F_fill, AppendExprs (Ids2Exprs (ids),
                                          MakeExprs (LET_EXPR (arg_node), NULL)));

        while (ids != NULL) {
            AllocList_Insert (arg_info, IDS_AVIS (ids), CreateZeroVector (0, T_int),
                              CreateZeroVector (0, T_int),
                              DupTree (INFO_SSARC_REUSELIST (arg_info)));

            /* Subtraction needed because ALLOC initializes RC with 1 */
            IncList_Insert (arg_info, IDS_AVIS (ids),
                            PopEnv (IDS_AVIS (ids), INFO_SSARC_DEPTH (arg_info)) - 1);
            ids = IDS_NEXT (ids);
        }
        if (INFO_SSARC_REUSELIST (arg_info) != NULL)
            INFO_SSARC_REUSELIST (arg_info) = FreeTree (INFO_SSARC_REUSELIST (arg_info));
        break;
    case rc_copy:
        /* Copy assignment: a=b */
        /* Env(b) += Env(a) */
        AddEnv (arg_info, IDS_AVIS (ID_IDS (LET_EXPR (arg_node))),
                INFO_SSARC_DEPTH (arg_info),
                PopEnv (IDS_AVIS (LET_IDS (arg_node)), INFO_SSARC_DEPTH (arg_info)));
        break;
    case rc_funcond:
        /* Treat FunCond like a variable Copy assignment */
        switch (INFO_SSARC_MODE (arg_info)) {
        case rc_default:
            SetDefLevel (ID_AVIS (EXPRS_EXPR (FUNCOND_THEN (LET_EXPR (arg_node)))),
                         INFO_SSARC_DEPTH (arg_info));
            AddEnv (arg_info, ID_AVIS (EXPRS_EXPR (FUNCOND_THEN (LET_EXPR (arg_node)))),
                    INFO_SSARC_DEPTH (arg_info),
                    PopEnv (IDS_AVIS (LET_IDS (arg_node)), INFO_SSARC_DEPTH (arg_info)));
            KillSecondEnv (IDS_AVIS (LET_IDS (arg_node)));
            break;
        case rc_else:
            SetDefLevel (ID_AVIS (EXPRS_EXPR (FUNCOND_ELSE (LET_EXPR (arg_node)))),
                         INFO_SSARC_DEPTH (arg_info));
            AddEnv (arg_info, ID_AVIS (EXPRS_EXPR (FUNCOND_ELSE (LET_EXPR (arg_node)))),
                    INFO_SSARC_DEPTH (arg_info),
                    PopEnv (IDS_AVIS (LET_IDS (arg_node)), INFO_SSARC_DEPTH (arg_info)));
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
                        PopEnv (IDS_AVIS (ID_IDS (ICM_ARG1 (arg_node))),
                                INFO_SSARC_DEPTH (arg_info)));
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
                        PopEnv (IDS_AVIS (ID_IDS (ICM_ARG1 (arg_node))),
                                INFO_SSARC_DEPTH (arg_info)));
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
                        PopEnv (IDS_AVIS (ID_IDS (ICM_ARG1 (arg_node))),
                                INFO_SSARC_DEPTH (arg_info)));

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
        IncreaseEnv (arg_info, ID_AVIS (arg_node), INFO_SSARC_DEPTH (arg_info));

        /* Don't put id into declist as we N_return and N_funap
           are implicitly consuming references */
        break;

    case rc_cexprs:
        /* If DEFLEVEL of CEXPR is undefinded, set it current DEPTH
           as this can only happen if the variable has
           been defined in the current CBLOCK */
        SetDefLevel (ID_AVIS (arg_node), INFO_SSARC_DEPTH (arg_info));
        /* Here is no break missing */
    case rc_array:
    case rc_cond:
    case rc_icm:
    case rc_with:
        IncreaseEnvOnZero (arg_info, ID_AVIS (arg_node), INFO_SSARC_DEPTH (arg_info));
        break;

    case rc_prfap:
        /* Add one to the environment iff it is zero */
        if (IncreaseEnvOnZero (arg_info, ID_AVIS (arg_node), INFO_SSARC_DEPTH (arg_info)))
            /* Potentially, this N_id could be reused.
               -> Add it to REUSELIST */
            INFO_SSARC_REUSELIST (arg_info)
              = AppendExprs (INFO_SSARC_REUSELIST (arg_info),
                             MakeExprs (DupNode (arg_node), NULL));
        break;

    case rc_funcond:
    case rc_funap:
    case rc_const:
    case rc_copy:
        Print (arg_node);
        DBUG_ASSERT (FALSE, "No ID node must appear in this context");
    }
    DBUG_RETURN (arg_node);
}

node *
SSARCconst (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSARCconst");

    if (INFO_SSARC_RHS (arg_info) == rc_undef)
        INFO_SSARC_RHS (arg_info) = rc_const;

    DBUG_RETURN (arg_node);
}

void
AllocateWithID (node *withid, node *arg_info)
{
    ids *i;
    AllocList_Insert (arg_info, IDS_AVIS (NWITHID_VEC (withid)),
                      CreateZeroVector (0, T_int), CreateZeroVector (0, T_int), NULL);
    IncreaseEnvOnZero (arg_info, IDS_AVIS (NWITHID_VEC (withid)),
                       INFO_SSARC_DEPTH (arg_info));
    PopEnv (IDS_AVIS (NWITHID_VEC (withid)), INFO_SSARC_DEPTH (arg_info) - 1);

    i = NWITHID_IDS (withid);
    while (i != NULL) {
        AllocList_Insert (arg_info, IDS_AVIS (i), CreateZeroVector (0, T_int),
                          CreateZeroVector (0, T_int), NULL);
        IncreaseEnvOnZero (arg_info, IDS_AVIS (i), INFO_SSARC_DEPTH (arg_info));
        PopEnv (IDS_AVIS (i), INFO_SSARC_DEPTH (arg_info) - 1);
        i = i->next;
    }
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
        NWITH_WITHID (arg_node) = Trav (NWITH_WITHID (arg_node), arg_info);
        if (NWITH_CODE (arg_node) != NULL)
            NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
        AllocateWithID (NWITH_WITHID (arg_node), arg_info);
        INFO_SSARC_DEPTH (arg_info) -= 1;

        INFO_SSARC_RHS (arg_info) = rc_with;
        NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
        NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);
    } else {

        INFO_SSARC_DEPTH (arg_info) += 1;
        INFO_SSARC_WITHID (arg_info) = NWITH2_WITHID (arg_node);
        NWITH2_WITHID (arg_node) = Trav (NWITH2_WITHID (arg_node), arg_info);
        if (NWITH2_CODE (arg_node) != NULL)
            NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
        AllocateWithID (NWITH2_WITHID (arg_node), arg_info);
        INFO_SSARC_DEPTH (arg_info) -= 1;

        INFO_SSARC_RHS (arg_info) = rc_with;
        NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);
        NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);
    }

    INFO_SSARC_WITHID (arg_info) = oldwithid;

    DBUG_RETURN (arg_node);
}

void
AnnotateCBlock (node *avis, node *cblock, node *arg_info)
{
    int env = PopEnv (avis, INFO_SSARC_DEPTH (arg_info));

    /*   if (IsIndexVariable( avis, arg_info)) */
    /*     env -= 1; */

    IncList_Insert (arg_info, avis, env);

    BLOCK_INSTR (cblock) = MakeIncAssignments (arg_info, BLOCK_INSTR (cblock));
}

node *
SSARCNcode (node *arg_node, node *arg_info)
{
    node *n, *epicode;

    DBUG_ENTER ("SSARCNcode");

    /* Traverse CEXPRS and insert adjust_rc operations into
       NCODE_EPILOGUE */
    INFO_SSARC_RHS (arg_info) = rc_cexprs;
    NCODE_CEXPRS (arg_node) = Trav (NCODE_CEXPRS (arg_node), arg_info);

    DBUG_ASSERT (NCODE_EPILOGUE (arg_node) == NULL, "Epilogue must not exist yet!");

    /* Insert ADJUST_RC prfs from DECLIST into EPILOGUE*/
    epicode = MakeDecAssignments (arg_info, NULL);
    if (epicode != NULL)
        NCODE_EPILOGUE (arg_node) = MakeBlock (epicode, NULL);

    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);

    /* Prepend block with Adjust_RC prfs */
    n = FUNDEF_ARGS (INFO_SSARC_FUNDEF (arg_info));
    while (n != NULL) {
        AnnotateCBlock (ARG_AVIS (n), NCODE_CBLOCK (arg_node), arg_info);
        n = ARG_NEXT (n);
    }

    n = BLOCK_VARDEC (FUNDEF_BODY (INFO_SSARC_FUNDEF (arg_info)));
    while (n != NULL) {
        AnnotateCBlock (VARDEC_AVIS (n), NCODE_CBLOCK (arg_node), arg_info);
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

    /* IV must be refcounted like if it was taken into the WL
       from a higher level */
    AVIS_SSARC_DEFLEVEL (IDS_AVIS (NWITHID_VEC (arg_node)))
      = INFO_SSARC_DEPTH (arg_info) - 1;

    iv_ids = NWITHID_IDS (arg_node);
    while (iv_ids != NULL) {
        AVIS_SSARC_DEFLEVEL (IDS_AVIS (iv_ids)) = INFO_SSARC_DEPTH (arg_info) - 1;
        iv_ids = IDS_NEXT (iv_ids);
    }

    DBUG_RETURN (arg_node);
}

node *
SSARCassign (node *arg_node, node *arg_info)
{
    node *n;
    ids *i;

    DBUG_ENTER ("SSARCassign");

    /*
     * Top down traversal:
     * Annotate definition level at avis nodes
     */
    if (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_let) {
        i = LET_IDS (ASSIGN_INSTR (arg_node));
        while (i != NULL) {
            AVIS_SSARC_DEFLEVEL (IDS_AVIS (i)) = INFO_SSARC_DEPTH (arg_info);
            i = IDS_NEXT (i);
        }
    }
    if (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_icm) {
        AVIS_SSARC_DEFLEVEL (ID_AVIS (ICM_ARG1 (ASSIGN_INSTR (arg_node))))
          = INFO_SSARC_DEPTH (arg_info);
    }

    /*
     * Bottom up traversal:
     * Annotate memory management instructions
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
    ASSIGN_NEXT (arg_node) = MakeDecAssignments (arg_info, ASSIGN_NEXT (arg_node));

    /* Insert ADJUST_RC prfs from INCLIST */
    ASSIGN_NEXT (arg_node) = MakeIncAssignments (arg_info, ASSIGN_NEXT (arg_node));

    /* Insert Alloc_or_Reuse prfs before this N_assign */
    arg_node = MakeAllocAssignments (arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

node *
SSARefCount (node *syntax_tree)
{
    node *info;

    DBUG_ENTER ("SSARefCount");

    show_refcnt = FALSE;

    info = MakeInfo ();

    INFO_SSARC_INCLIST (info) = NULL;
    INFO_SSARC_DECLIST (info) = NULL;
    INFO_SSARC_ALLOCLIST (info) = NULL;
    INFO_SSARC_REUSELIST (info) = NULL;

    act_tab = ssarefcnt_tab;
    INFO_SSARC_MODE (info) = rc_annotate_cfuns;
    syntax_tree = Trav (syntax_tree, info);

    INFO_SSARC_MODE (info) = rc_default;
    syntax_tree = Trav (syntax_tree, info);

    info = FreeTree (info);

    DBUG_RETURN (syntax_tree);
}
