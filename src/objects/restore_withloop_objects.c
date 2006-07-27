/* $Id$ */

#include "restore_withloop_objects.h"

#include "ctinfo.h"
#include "dbug.h"
#include "free.h"
#include "globals.h"
#include "internal_lib.h"
#include "new_types.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "type_utils.h"
#include "user_types.h"

/*
 * INFO structure
 */

struct INFO {
    node *with_let_lhs; /* N_ids on the left-hand side of the with-loop let */
    node *with_exprs;   /* Result expressions of the with-loop */
    bool delete_assign; /* The current assignment should be deleted entirely */
};

/*
 * INFO macros
 */

#define INFO_WITH_LET_LHS(n) ((n)->with_let_lhs)
#define INFO_WITH_EXPRS(n) ((n)->with_exprs)
#define INFO_DELETE_ASSIGN(n) ((n)->delete_assign)

/*
 * INFO functions
 */

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_WITH_LET_LHS (result) = NULL;
    INFO_WITH_EXPRS (result) = NULL;
    INFO_DELETE_ASSIGN (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/*
 * Local helper functions
 */

/*
 * Iterates over the given withops, with-loop result expressions and left-hand
 * side assignment id's. Removes each corresponding entry if the withop
 * is a extract() withop, thus undoing most of the with-loop-object traversal.
 */
static node *
RemoveExtractedObjects (node *withops, node *withexprs, node *let_lhs, info *arg_info)
{
    node *temp;
    node *iter;
    node *prev_withop = NULL;
    node *prev_withexpr = NULL;
    node *prev_let_lhs = NULL;

    DBUG_ENTER ("RemoveExtractedObjects");

    DBUG_ASSERT (withexprs != NULL, "Less exprs in withloop than withops!");
    DBUG_ASSERT (let_lhs != NULL, "LHS != RHS of let expression");

    /* Iterate over the withops, withexprs and let_lhs ids in lockstep */

    iter = withops;
    while (iter != NULL) {
        if (iter->nodetype == N_extract) {

            /*
             * Remove the extract() withop, and the associated LHS expr and
             * with-loop result expression.
             */

            temp = iter;
            iter = WITHOP_NEXT (iter);
            if (prev_withop != NULL) {
                L_WITHOP_NEXT (prev_withop, iter);
            }
            FREEdoFreeNode (temp);

            temp = withexprs;
            withexprs = EXPRS_NEXT (withexprs);
            if (prev_withexpr != NULL) {
                EXPRS_NEXT (prev_withexpr) = withexprs;
            }
            FREEdoFreeNode (temp);

            temp = let_lhs;
            let_lhs = IDS_NEXT (let_lhs);
            if (prev_let_lhs != NULL) {
                IDS_NEXT (prev_let_lhs) = let_lhs;
            }
            FREEdoFreeNode (temp);

        } else {

            /* Do nothing, iterate to next entry */

            prev_withop = iter;
            iter = WITHOP_NEXT (iter);
            prev_withexpr = withexprs;
            withexprs = EXPRS_NEXT (withexprs);
            prev_let_lhs = let_lhs;
            let_lhs = IDS_NEXT (let_lhs);
        }
    }

    DBUG_RETURN (withops);
}

/*
 * Traversal functions
 */

node *
RWOAassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RWOAassign");

    bool dodel = FALSE;
    node *temp;

    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }

    dodel = INFO_DELETE_ASSIGN (arg_info);
    INFO_DELETE_ASSIGN (arg_info) = FALSE;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    if (dodel == TRUE) {
        temp = ASSIGN_NEXT (arg_node);
        FREEdoFreeNode (arg_node);
        DBUG_RETURN (temp);
    } else {
        DBUG_RETURN (arg_node);
    }
}

node *
RWOAblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RWOAblock");

    /* First traverse the body, so that we get a list of vardecs to be removed
     * (if any) */
    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    /* Traverse the vardecs */
    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RWOAfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RWOAfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RWOAid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RWOAid");

    arg_node = TRAVcont (arg_node, arg_info);
    DBUG_RETURN (arg_node);
}

node *
RWOAlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RWOAlet");

    node *old_lhs;

    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    if (LET_EXPR (arg_node) != NULL && LET_EXPR (arg_node)->nodetype == N_with2) {
        DBUG_ASSERT (LET_IDS (arg_node) != NULL, "N_let without LHS");

        /* Save the previous let's lhs */
        old_lhs = INFO_WITH_LET_LHS (arg_info);
        INFO_WITH_LET_LHS (arg_info) = LET_IDS (arg_node);

        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

        /* Restore it */
        LET_IDS (arg_node) = INFO_WITH_LET_LHS (arg_info);
        INFO_WITH_LET_LHS (arg_info) = old_lhs;

    } else {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RWOAprf (node *arg_node, info *arg_info)
{
    prf p = PRF_PRF (arg_node);

    DBUG_ENTER ("RWOAprf");

    if (p == F_inc_rc || p == F_dec_rc) {
        /*
         * For now we delete any inc_rc and dec_rc applied to N_globobj.
         * This is probably not right and will remove too much, but for now...
         */
        if (NODE_TYPE (EXPRS_EXPR (PRF_ARGS (arg_node))) == N_globobj) {
            INFO_DELETE_ASSIGN (arg_info) = TRUE;
        }
    }
    DBUG_RETURN (arg_node);
}

/*
 * Removes the N_vardec in the chain that correspond to the N_ids given.
 */
node *
RWOAvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RWOAvardec");

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RWOAwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RWOAwith2");

    if (WITH2_CODE (arg_node) != NULL) {
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
    }

    if (WITH2_WITHOP (arg_node) != NULL) {
        INFO_WITH_EXPRS (arg_info) = CODE_CEXPRS (WITH2_CODE (arg_node));
        WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
        WITH2_WITHOP (arg_node)
          = RemoveExtractedObjects (WITH2_WITHOP (arg_node), INFO_WITH_EXPRS (arg_info),
                                    INFO_WITH_LET_LHS (arg_info), arg_info);
        CODE_CEXPRS (WITH2_CODE (arg_node)) = INFO_WITH_EXPRS (arg_info);
        INFO_WITH_EXPRS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/*
 * Traversal start function
 */

node *
RWOAdoRestoreWithloopObjectAnalysis (node *syntax_tree)
{
    info *arg_info;
    DBUG_ENTER ("RWOAdoRestoreWithloopObjectAnalysis");

    TRAVpush (TR_rwoa);

    arg_info = MakeInfo ();

    syntax_tree = TRAVdo (syntax_tree, arg_info);

    FreeInfo (arg_info);

    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}
