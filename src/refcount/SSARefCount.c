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

/*#define SSARC_DEVELOP*/

typedef enum {
    rc_undef,
    rc_return,
    rc_copy,
    rc_funap,
    rc_prfap,
    rc_array,
    rc_const
} rc_rhs_type;

typedef enum {
    rc_default,
    rc_both, /* Needed in order to refcount conditionals */
    rc_then,
    rc_else
} rc_mode;

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

#define INFO_SSARC_MODE(n) ((rc_mode) (n->counter))
#define INFO_SSARC_DEPTH(n) (n->varno)
#define INFO_SSARC_RHS(n) ((rc_rhs_type) (n->flag))
#define INFO_SSARC_DECLIST(n) ((rc_list_struct *)(n->node[0]))
#define INFO_SSARC_INCLIST(n) ((rc_list_struct *)(n->node[1]))
#define INFO_SSARC_RESULT(n) (n->refcnt)

#define AVIS_SSARC_COUNTED(n) ((bool)(n->info.cint))
#define AVIS_SSARC_DEFAULT(n) ((rc_counter *)(n->dfmask[0]))
#define AVIS_SSARC_ELSE(n) ((rc_counter *)(n->dfmask[1]))
/**
 *
 *  HELPER FUNCTIONS
 *
 ****************************************************************************/
void
InitializeEnv (node *avis)
{
    AVIS_SSARC_COUNTED (avis) = FALSE;
    AVIS_SSARC_DEFAULT (avis) = NULL;
    AVIS_SSARC_ELSE (avis) = NULL;
}

rc_counter *
IncreaseGivenEnv (rc_counter *rc_count, node *arg_info, bool incOverOne)
{
    rc_counter *rcc;

    DBUG_ASSERT ((rc_count == NULL) || (INFO_SSARC_DEPTH (arg_info) >= rc_count->depth),
                 "Illegal reference counter stack state");

    if ((rc_count == NULL) || (INFO_SSARC_DEPTH (arg_info) > rc_count->depth)) {
        rcc = Malloc (sizeof (rc_counter));
        rcc->depth = INFO_SSARC_DEPTH (arg_info);
        rcc->count = 1;
        rcc->next = rc_count;
        INFO_SSARC_RESULT (arg_info) = TRUE;
        return rcc;
    } else {
        if (incOverOne)
            rc_count->count += 1;
        INFO_SSARC_RESULT (arg_info) = FALSE;
        return rc_count;
    }
}

bool
IncreaseEnvPar (node *avis, node *arg_info, bool incOverOne)
{
    switch (INFO_SSARC_MODE (arg_info)) {
    case rc_then:
    case rc_default:
        AVIS_SSARC_DEFAULT (avis)
          = IncreaseGivenEnv (AVIS_SSARC_DEFAULT (avis), arg_info, incOverOne);
        return (INFO_SSARC_RESULT (arg_info));
        break;
    case rc_else:
        AVIS_SSARC_ELSE (avis)
          = IncreaseGivenEnv (AVIS_SSARC_ELSE (avis), arg_info, incOverOne);
        return (INFO_SSARC_RESULT (arg_info));
        break;
    case rc_both:
        AVIS_SSARC_DEFAULT (avis)
          = IncreaseGivenEnv (AVIS_SSARC_DEFAULT (avis), arg_info, incOverOne);
        AVIS_SSARC_ELSE (avis)
          = IncreaseGivenEnv (AVIS_SSARC_ELSE (avis), arg_info, incOverOne);
        return (INFO_SSARC_RESULT (arg_info));
        break;
    }
    DBUG_ASSERT (FALSE, "Cannot arrive here");
    return FALSE;
}

bool
IncreaseEnv (node *avis, node *arg_info)
{
    return IncreaseEnvPar (avis, arg_info, TRUE);
}

bool
IncreaseEnvOnZero (node *avis, node *arg_info)
{
    return IncreaseEnvPar (avis, arg_info, FALSE);
}

int
PopEnv (node *avis, node *arg_info)
{
    int res;

    DBUG_ASSERT (AVIS_SSARC_COUNTED (avis) == FALSE,
                 "Variable has already been counted!");

    AVIS_SSARC_COUNTED (avis) = TRUE;

    switch (INFO_SSARC_MODE (arg_info)) {
    case rc_then:
    case rc_default:
    case rc_both:
        if (AVIS_SSARC_DEFAULT (avis) == NULL)
            return 0;
        else {
            res = AVIS_SSARC_DEFAULT (avis)->count;
            AVIS_SSARC_DEFAULT (avis) = Free (AVIS_SSARC_DEFAULT (avis));
            if (AVIS_SSARC_ELSE (avis) != NULL)
                AVIS_SSARC_ELSE (avis) = Free (AVIS_SSARC_ELSE (avis));
            return (res);
        }
        break;
    case rc_else:
        if (AVIS_SSARC_ELSE (avis) == NULL)
            return 0;
        else {
            res = AVIS_SSARC_ELSE (avis)->count;
            AVIS_SSARC_ELSE (avis) = Free (AVIS_SSARC_ELSE (avis));
            return (res);
        }
        break;
    }

    DBUG_ASSERT (FALSE, "Cannot arrive here!");
    return 0;
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

node *
MakeAdjustRC (node *avis, int count, node *next_node)
{
    node *n;
    ids *ids1, *ids2;

#ifndef SSARC_DEVELOP
    if ((count == 0) || (TYPES_DIM (VARDEC_TYPE (AVIS_VARDECORARG (avis))) == 0))
        return next_node;
#endif

    ids1 = MakeIds (VARDEC_NAME (AVIS_VARDECORARG (avis)), NULL, ST_regular);

    IDS_AVIS (ids1) = avis;
    IDS_VARDEC (ids1) = AVIS_VARDECORARG (avis);

    ids2 = MakeIds (VARDEC_NAME (AVIS_VARDECORARG (avis)), NULL, ST_regular);

    IDS_AVIS (ids2) = avis;
    IDS_VARDEC (ids2) = AVIS_VARDECORARG (avis);

    n = MakeAssign (MakeLet (MakePrf (F_adjust_rc,
                                      MakeExprs (MakeIdFromIds (ids1),
                                                 MakeExprs (MakeNum (count), NULL))),
                             DupOneIds (ids2)),
                    next_node);

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
    node *arg;

    DBUG_ENTER ("SSARCfundef");

    if (FUNDEF_IS_CONDFUN (fundef))
        INFO_SSARC_MODE (arg_info) = rc_both;
    else
        INFO_SSARC_MODE (arg_info) = rc_default;

    INFO_SSARC_DEPTH (arg_info) = 0;

    /* Traverse args in order to initialize refcounting environment */
    if (FUNDEF_ARGS (fundef) != NULL)
        FUNDEF_ARGS (fundef) = Trav (FUNDEF_ARGS (fundef), arg_info);

    /* Traverse block */
    if (FUNDEF_BODY (fundef) != NULL)
        FUNDEF_BODY (fundef) = Trav (FUNDEF_BODY (fundef), arg_info);

    arg = FUNDEF_ARGS (fundef);
    while (arg != NULL) {
        BLOCK_INSTR (FUNDEF_BODY (fundef))
          = MakeAdjustRC (ARG_AVIS (arg), PopEnv (ARG_AVIS (arg), arg_info) - 1,
                          BLOCK_INSTR (FUNDEF_BODY (fundef)));
        arg = ARG_NEXT (arg);
    }

#ifndef SSARC_DEVELOP
    /* Restore SSA form */
    fundef = RestoreSSAOneFundef (fundef);
#endif

    /* Traverse other fundefs */
    if (FUNDEF_NEXT (fundef) != NULL)
        FUNDEF_NEXT (fundef) = Trav (FUNDEF_NEXT (fundef), arg_info);

    DBUG_RETURN (fundef);
}

node *
SSARCarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSARCarg");

    InitializeEnv (ARG_AVIS (arg_node));

    if (ARG_NEXT (arg_node) != NULL)
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);

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
SSARCvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSARCvardec");

    InitializeEnv (VARDEC_AVIS (arg_node));

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

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
    DBUG_ENTER ("SSARCap");

    INFO_SSARC_RHS (arg_info) = rc_funap;

    if (AP_ARGS (arg_node) != NULL)
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);

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
SSARClet (node *arg_node, node *arg_info)
{
    ids *ids;
    int rhs_val;

    DBUG_ENTER ("SSARClet");

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    switch (INFO_SSARC_RHS (arg_info)) {
    case rc_funap:
    case rc_copy:
        rhs_val = -1;
        break;
    case rc_prfap:
    case rc_const:
    case rc_array:
        rhs_val = 0;
        break;
    default:
        DBUG_ASSERT (FALSE, "Cannot happen");
    }

    /* Add all lhs ids to inclist */
    ids = LET_IDS (arg_node);
    while (ids != NULL) {
        IncList_Insert (arg_info, IDS_AVIS (ids),
                        PopEnv (IDS_AVIS (ids), arg_info) + rhs_val);
        ids = IDS_NEXT (ids);
    }

    DBUG_RETURN (arg_node);
}

node *
SSARCid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSARCid");

    switch (INFO_SSARC_RHS (arg_info)) {
    case rc_undef:
        INFO_SSARC_RHS (arg_info) = rc_copy;
        /* Add one to the environment */
        IncreaseEnv (ID_AVIS (arg_node), arg_info);
        /* Don't put id into declist as this is a copy assignment */
        break;
    case rc_funap:
    case rc_return:
        /* Add one to the environment */
        IncreaseEnv (ID_AVIS (arg_node), arg_info);

        /* Don't put id into declist as we N_return and N_funap
           are implicitly consuming references */
        break;

    case rc_array:
    case rc_prfap:
        /* Add one to the environment iff it is zero */
        if (IncreaseEnvOnZero (ID_AVIS (arg_node), arg_info))

            /* Put id into declist as we are traversing a N_prf */
            DecList_Insert (arg_info, ID_AVIS (arg_node));
        break;
    case rc_const:
    case rc_copy:
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
SSARCassign (node *arg_node, node *arg_info)
{
    rc_list_struct *rls;

    DBUG_ENTER ("SSARCassign");

    /*
     * Bottom up traversal!!
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    INFO_SSARC_RHS (arg_node) = rc_undef;
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

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
    syntax_tree = Trav (syntax_tree, info);

    info = FreeTree (info);

    DBUG_RETURN (syntax_tree);
}
