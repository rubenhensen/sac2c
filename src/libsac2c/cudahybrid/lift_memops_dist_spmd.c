/*****************************************************************************
 *
 * file:   lift_memops_dist_spmd.c
 *
 * prefix: MTLMOFC
 *
 * description:
 *
 *
 *****************************************************************************/

#include "lift_memops_dist_spmd.h"

#define DBUG_PREFIX "MTLMOFC"
#include "debug.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "DupTree.h"
#include "free.h"
#include "memory.h"
#include "LookUpTable.h"
#include "namespaces.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"
#include "str.h"

/**
 * INFO structure
 */

struct INFO {
    bool inspmd;
    bool indistcond;
    prf thisprf;
    bool insertallocs;
    node *preassigns;
    node *postassigns;
    node *predavis;
    node *freeavis;
};

/**
 * INFO macros
 */

#define INFO_INSPMD(n) ((n)->inspmd)
#define INFO_INDISTCOND(n) ((n)->indistcond)
#define INFO_THISPRF(n) ((n)->thisprf)
#define INFO_INSERTALLOCS(n) ((n)->insertallocs)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_POSTASSIGNS(n) ((n)->postassigns)
#define INFO_PREDAVIS(n) ((n)->predavis)
#define INFO_FREEAVIS(n) ((n)->freeavis)

/**
 * INFO functions
 */

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_INSPMD (result) = FALSE;
    INFO_INDISTCOND (result) = FALSE;
    INFO_THISPRF (result) = F_unknown;
    INFO_INSERTALLOCS (result) = FALSE;
    INFO_PREASSIGNS (result) = NULL;
    INFO_POSTASSIGNS (result) = NULL;
    INFO_PREDAVIS (result) = NULL;
    INFO_FREEAVIS (result) = NULL;

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
 *
 * @fn node *MTLMOFCmodule( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTLMOFCmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTLMOFCfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTLMOFCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISSTFUN (arg_node) || FUNDEF_ISXTFUN (arg_node)) {
        /*
         * ST and XT funs may contain parallel with-loops.
         * Hence, we constrain our search accordingly.
         */
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTLMOFCassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTLMOFCassign (node *arg_node, info *arg_info)
{
    node *res, *next;
    info *old_info;

    DBUG_ENTER ();

    INFO_THISPRF (arg_info) = F_unknown;

    if (!INFO_INDISTCOND (arg_info)) {
        ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

        // outside the conditional
        if (INFO_THISPRF (arg_info) == F_is_cuda_thread) {
            /* This is the _is_cuda_thread() assignment, signal the previous
             assignment node that the pre-assigns have to be inserted there */
            INFO_INSPMD (arg_info) = TRUE;
            ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
            INFO_INSERTALLOCS (arg_info) = TRUE;
            res = arg_node;
        } else if (INFO_POSTASSIGNS (arg_info) != NULL) {
            /* If there are post-assigns to insert, this node is the conditional.
             We save the info structure before we traverse the next node, then add the
             post-assignments when we come back up the tree.*/
            old_info = arg_info;
            arg_info = MakeInfo ();
            next = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
            FreeInfo (arg_info);
            arg_info = old_info;
            ASSIGN_NEXT (arg_node) = TCappendAssign (INFO_POSTASSIGNS (arg_info), next);
            INFO_POSTASSIGNS (arg_info) = NULL;
            res = arg_node;
        } else {
            ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
            if (INFO_INSERTALLOCS (arg_info)) {
                /* insert pre-assignments if necessary */
                res = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
                INFO_INSERTALLOCS (arg_info) = FALSE;
                INFO_PREASSIGNS (arg_info) = NULL;
            } else {
                res = arg_node;
            }
        }
    } else {
        ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

        /* we are on one of the conditional's branches*/
        if (INFO_THISPRF (arg_info) == F_alloc || INFO_THISPRF (arg_info) == F_inc_rc
            || INFO_THISPRF (arg_info) == F_fill) {
            /*
             * This is one of the assignments we want to move out of the
             * conditional, placing it before.
             */
            res = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
            ASSIGN_NEXT (arg_node) = NULL;
            INFO_PREASSIGNS (arg_info)
              = TCappendAssign (arg_node, INFO_PREASSIGNS (arg_info));
        } else if (INFO_THISPRF (arg_info) == F_dec_rc
                   || (INFO_THISPRF (arg_info) == F_free
                       && INFO_FREEAVIS (arg_info) != INFO_PREDAVIS (arg_info))) {
            /*
             * This is one of the assignments we want to move out of the
             * conditional, placing it afterwards.
             */
            res = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
            ASSIGN_NEXT (arg_node) = NULL;
            INFO_POSTASSIGNS (arg_info)
              = TCappendAssign (arg_node, INFO_POSTASSIGNS (arg_info));
        } else {
            ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
            res = arg_node;
        }
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTLMOFCprf( node *arg_node, info *arg_info)
 *
 * description:
 *   Saves the kind of prf in info. Additionally, replaces the single threaded
 *   versions of distributed memory transfer primitives to their SPMD versions
 *   when inside the distributed with-loop conditional.
 *
 *****************************************************************************/

node *
MTLMOFCprf (node *arg_node, info *arg_info)
{
    prf self;
    node *arg;

    DBUG_ENTER ();

    PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);

    self = PRF_PRF (arg_node);
    INFO_THISPRF (arg_info) = self;

    if (self == F_fill) {
        /* we are not interested in the fill function itself, rather any other prf's
         inside it */
        arg = PRF_ARG1 (arg_node);
        if (NODE_TYPE (arg) == N_prf) {
            INFO_THISPRF (arg_info) = PRF_PRF (arg);
        }
    } else if (self == F_host2dist_st && INFO_INDISTCOND (arg_info)) {
        // change the host2dist_st() functions in the conditional to the spmd version
        PRF_PRF (arg_node) = F_host2dist_spmd;
        INFO_THISPRF (arg_info) = F_host2dist_spmd;
    } else if (self == F_free) {
        /* save the avis of the free() argument. Should it be the predicate avis, we
         don't want to move this free() assignment out of the conditional. */
        INFO_FREEAVIS (arg_info) = ID_AVIS (PRF_ARG1 (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTLMOFCcond( node *arg_node, info *arg_info)
 *
 * description:
 *   Traverse the conditional, then unset the INDISTCOND flag.
 *
 *****************************************************************************/

node *
MTLMOFCcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_INSPMD (arg_info)) {
        INFO_INDISTCOND (arg_info) = TRUE;
    }

    COND_THEN (arg_node) = TRAVopt (COND_THEN (arg_node), arg_info);
    COND_ELSE (arg_node) = TRAVopt (COND_ELSE (arg_node), arg_info);

    INFO_INDISTCOND (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTLMOFClet( node *arg_node, info *arg_info)
 *
 * description:
 *   If RHS is a _is_cuda_thread_() primitive, record the LHS avis in info.
 *
 *****************************************************************************/

node *
MTLMOFClet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (INFO_THISPRF (arg_info) == F_is_cuda_thread) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTLMOFCids( node *arg_node, info *arg_info)
 *
 * description:
 *   If RHS is a _is_cuda_thread_() primitive, record the LHS avis in info.
 *
 *****************************************************************************/

node *
MTLMOFCids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_THISPRF (arg_info) == F_is_cuda_thread) {
        INFO_PREDAVIS (arg_info) = IDS_AVIS (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *MTLMOFCwith2( node *arg_node, info *arg_info)
 *
 * description:
 *   we don't traverse the with-loop, as we are not interested in lifting any of
 *   the assignments inside it.
 *
 ******************************************************************************/

node *
MTLMOFCwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *MTLMOFCwith( node *arg_node, info *arg_info)
 *
 * description:
 *   we don't traverse the with-loop, as we are not interested in lifting any of
 *   the assignments inside it.
 *
 ******************************************************************************/

node *
MTLMOFCwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn MTLMOFCdoCreateDistributedSpmdFuns( node *syntax_tree)
 *
 *  @brief initiates traversal for lifting memory operations from the
 *  distributed with-loop conditionals
 *
 *****************************************************************************/

node *
MTLMOFCdoLiftMemoryOperationsFromConditional (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module, "Illegal argument node!");

    info = MakeInfo ();

    TRAVpush (TR_mtlmofc);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
