/*
 * $Log$
 * Revision 1.4  2001/03/02 15:51:49  nmw
 * CSRemoveArg/CSRemoveResult moved to separate file change_signature
 *
 * Revision 1.3  2001/03/02 14:34:15  nmw
 * SSADeadCodeRemoval implemented
 *
 * Revision 1.2  2001/02/27 16:05:35  nmw
 * SSADeadCodeRemoval for intraprocedural code implemented
 *
 * Revision 1.1  2001/02/23 13:37:50  nmw
 * Initial revision
 *

 */

/*****************************************************************************
 *
 * file:   SSADeadCodeRemoval.c
 *
 * prefix: SSADCR
 *
 * description:
 *    this module traverses ONE FUNCTION (and its liftet special funtions)
 *    and removes all dead code.
 *    this transformation is NOT conservative (it might remove
 *    endless loops)!
 *
 *****************************************************************************/

#include "dbug.h"
#include "globals.h"
#include "tree.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "SSADeadCodeRemoval.h"
#include "optimize.h"
#include "change_signature.h"

/* local constants */
#define SSADCR_TOPLEVEL 0
#define SSADCR_NOTNEEDED 0

/* internal functions for traversing ids like nodes */
static ids *TravLeftIDS (ids *arg_ids, node *arg_info);
static ids *SSADCRleftids (ids *arg_ids, node *arg_info);
static ids *TravRightIDS (ids *arg_ids, node *arg_info);
static ids *SSADCRrightids (ids *arg_ids, node *arg_info);

/* helper functions for local use */
void SSADCRInitAvisFlags (node *fundef);
bool SSADCRCondIsEmpty (node *cond);

/******************************************************************************
 *
 * function:
 *   void SSADCRInitAvisFlags(node *fundef)
 *
 * description:
 *   inits flags needed for this module in all vardec and args.
 *
 ******************************************************************************/
void
SSADCRInitAvisFlags (node *fundef)
{
    node *tmp;

    DBUG_ENTER ("SSADCRInitAvisFlags");

    /* process args */
    tmp = FUNDEF_ARGS (fundef);
    while (tmp != NULL) {
        AVIS_NEEDCOUNT (ARG_AVIS (tmp)) = SSADCR_NOTNEEDED;
        tmp = ARG_NEXT (tmp);
    }

    /* process vardecs */
    if (FUNDEF_BODY (fundef) != NULL) {
        tmp = BLOCK_VARDEC (FUNDEF_BODY (fundef));
        while (tmp != NULL) {
            AVIS_NEEDCOUNT (VARDEC_AVIS (tmp)) = SSADCR_NOTNEEDED;
            tmp = VARDEC_NEXT (tmp);
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   bool SSADCRCondIsEmpty(node *cond)
 *
 * description:
 *   checks if conditional "cond" is empty, that means then and else blocks
 *   contain no assignments or are missing.
 *
 ******************************************************************************/
bool
SSADCRCondIsEmpty (node *cond)
{
    bool result;

    DBUG_ENTER ("SSADCRCondIsEmpty");

    result = TRUE;

    /* then part */
    if (COND_THEN (cond) != NULL) {
        if (BLOCK_INSTR (COND_THEN (cond)) != NULL) {
            if (NODE_TYPE (BLOCK_INSTR (COND_THEN (cond))) != N_empty) {
                result = FALSE;
            }
        }
    }

    /* else part */
    if (COND_THEN (cond) != NULL) {
        if (BLOCK_INSTR (COND_THEN (cond)) != NULL) {
            if (NODE_TYPE (BLOCK_INSTR (COND_THEN (cond))) != N_empty) {
                result = FALSE;
            }
        }
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRfundef(node *arg_node , node *arg_info)
 *
 * description:
 *   Starts the traversal of a given fundef. Does NOT traverse to
 *   next fundef in chain! The traversal mode (on toplevel, in special function)
 *   is annotated in the stacked INFO_SSADCR_DEPTH attribute.
 *   If not on toplevel, the unused arguments are cleared. For the toplevel
 *   functions changes of the signature are not possible.
 *
 ******************************************************************************/
node *
SSADCRfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSADCRfundef");

    DBUG_PRINT ("SSADCR",
                ("\nstarting dead code removal in fundef %s.", FUNDEF_NAME (arg_node)));

    /* store cuurent vardec for later access */
    INFO_SSADCR_FUNDEF (arg_info) = arg_node;

    /* init of needed-mask for all vardecs/args of this fundef */
    SSADCRInitAvisFlags (arg_node);

    if (FUNDEF_BODY (arg_node) != NULL) {
        /* traverse block of fundef */
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    if ((INFO_SSADCR_DEPTH (arg_info) != SSADCR_TOPLEVEL)
        && (FUNDEF_ARGS (arg_node) != NULL)) {
        /*
         * traverse args to remove unused ones from signature (not on toplevel!)
         */
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRarg(node *arg_node , node *arg_info)
 *
 * description:
 *   removes all args from function signature that have not been used in the
 *   function.
 *
 *
 ******************************************************************************/
node *
SSADCRarg (node *arg_node, node *arg_info)
{
    node *tmp;
    nodelist *letlist;

    DBUG_ENTER ("SSADCRarg");

    /* traverse to next arg */
    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    /*
     * process arg and remove it, if not needed anymore in code
     * or the arg is tagged as loop-invariant, that means it
     * occurs at least once in the recursive call. If the argument
     * ONLY appears there, it is dead code, too.
     */
    if ((AVIS_NEEDCOUNT (ARG_AVIS (arg_node)) == SSADCR_NOTNEEDED)
        || ((AVIS_NEEDCOUNT (ARG_AVIS (arg_node)) == 1)
            && (AVIS_SSALPINV (ARG_AVIS (arg_node))))) {
        DBUG_PRINT ("SSADCR", ("remove arg %sa", ARG_NAME (arg_node)));
        /* remove this argument from all applciations */
        letlist = NULL;
        if (FUNDEF_EXT_ASSIGN (INFO_SSADCR_FUNDEF (arg_info)) != NULL) {
            letlist = NodeListAppend (letlist,
                                      ASSIGN_INSTR (FUNDEF_EXT_ASSIGN (
                                        INFO_SSADCR_FUNDEF (arg_info))),
                                      NULL);
        }
        if (FUNDEF_INT_ASSIGN (INFO_SSADCR_FUNDEF (arg_info)) != NULL) {
            letlist = NodeListAppend (letlist,
                                      ASSIGN_INSTR (FUNDEF_INT_ASSIGN (
                                        INFO_SSADCR_FUNDEF (arg_info))),
                                      NULL);
        }

        INFO_SSADCR_FUNDEF (arg_info)
          = CSRemoveArg (INFO_SSADCR_FUNDEF (arg_info), arg_node, letlist, FALSE);

        FreeNodelist (letlist);

        /* remove arg from function signature */
        tmp = arg_node;
        arg_node = ARG_NEXT (arg_node);
        FreeNode (tmp);
        dead_var++;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRblock(node *arg_node , node *arg_info)
 *
 * description:
 *   traverses instructions and vardecs in this order.
 *
 ******************************************************************************/
node *
SSADCRblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSADCRblock");

    if (BLOCK_INSTR (arg_node) != NULL) {
        /* traverse assignmentchain in block */
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    if (BLOCK_VARDEC (arg_node) != NULL) {
        /*
         * traverse all vardecs in block (concerns only toplevel block in
         * each function) to remove useless ones
         */
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRvardec(node *arg_node , node *arg_info)
 *
 * description:
 *  traverses vardecs and removes unused ones.
 *
 ******************************************************************************/
node *
SSADCRvardec (node *arg_node, node *arg_info)
{
    node *tmp;

    DBUG_ENTER ("SSADCRvardec");

    /* traverse to next vardec */
    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    /* process vardec and remove it, if dead code */
    if (AVIS_NEEDCOUNT (VARDEC_AVIS (arg_node)) == SSADCR_NOTNEEDED) {
        DBUG_PRINT ("SSADCR", ("remove unused vardec %s", VARDEC_NAME (arg_node)));
        tmp = arg_node;
        arg_node = VARDEC_NEXT (arg_node);
        FreeNode (tmp);
        dead_var++;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRassign(node *arg_node , node *arg_info)
 *
 * description:
 *  traverses assignment chain bottom-up and removes all assignments not
 *  needed (marked by INFO_SSADCR_REMASSIGN)
 *
 ******************************************************************************/
node *
SSADCRassign (node *arg_node, node *arg_info)
{
    node *tmp;

    DBUG_ENTER ("SSADCRassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    /* traverse instruction */
    INFO_SSADCR_REMASSIGN (arg_info) = FALSE;
    INFO_SSADCR_ASSIGN (arg_info) = arg_node;
    DBUG_ASSERT ((ASSIGN_INSTR (arg_node) != NULL), "missing instruction in assign");

    /* traverse instruction */
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    /* free this assignment if unused anymore */
    if (INFO_SSADCR_REMASSIGN (arg_info)) {
        tmp = arg_node;
        arg_node = ASSIGN_NEXT (arg_node);
        FreeNode (tmp);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRlet(node *arg_node , node *arg_info)
 *
 * description:
 *  checks, if at least one of the left ids vardecs are needed. then this
 *  let is needed.
 *  a functions application of a special function requieres a special
 *  handling because you can remove parts of the results an modify the
 *  functions signature!
 *
 ******************************************************************************/
node *
SSADCRlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSADCRlet");

    DBUG_ASSERT ((LET_EXPR (arg_node) != NULL), "let without expression");

    /* default: do not modify return values of a function application */
    INFO_SSADCR_REMRESULTS (arg_info) = FALSE;

    /* check for special function as N_ap expression */
    if (NODE_TYPE (LET_EXPR (arg_node)) == N_ap) {
        if ((FUNDEF_STATUS (AP_FUNDEF (LET_EXPR (arg_node))) == ST_condfun)
            || (FUNDEF_STATUS (AP_FUNDEF (LET_EXPR (arg_node))) == ST_dofun)
            || (FUNDEF_STATUS (AP_FUNDEF (LET_EXPR (arg_node))) == ST_whilefun)) {

            /*
             * here we can modify the return values, because
             * there is only on call to this special function
             */
            INFO_SSADCR_REMRESULTS (arg_info) = TRUE;
            INFO_SSADCR_APFUNDEF (arg_info) = AP_FUNDEF (LET_EXPR (arg_node));

            /* set correct external assign reference in called fundef node */
            if (AP_FUNDEF (LET_EXPR (arg_node)) != INFO_SSADCR_FUNDEF (arg_info)) {
                /* no recursive (internal) call must be the one external call */
                FUNDEF_EXT_ASSIGN (AP_FUNDEF (LET_EXPR (arg_node)))
                  = INFO_SSADCR_ASSIGN (arg_info);
            }
        }
    }

    /* init ids counter for results */
    INFO_SSADCR_RESCOUNT (arg_info) = 0;
    INFO_SSADCR_RESNEEDED (arg_info) = 0;
    INFO_SSADCR_LET (arg_info) = arg_node;

    /* traverse left side identifier */
    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TravLeftIDS (LET_IDS (arg_node), arg_info);
    }

    if (INFO_SSADCR_RESNEEDED (arg_info) == 0) {
        /* let is useless -> can be removed! */

        DBUG_PRINT ("SSADCR", ("removing assignment"));
        arg_node = FreeNode (arg_node);

        dead_expr++;

        /* corresponding assign node can also be removed */
        INFO_SSADCR_REMASSIGN (arg_info) = TRUE;
    } else {
        /* traverse right side of let (mark needed variables) */
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
        /* do not remove assign node */
        INFO_SSADCR_REMASSIGN (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRid(node *arg_node , node *arg_info)
 *
 * description:
 *   "traverses" the contained ids structure.
 *
 ******************************************************************************/
node *
SSADCRid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSADCRid");

    DBUG_ASSERT ((ID_IDS (arg_node) != NULL), "id without ids");
    ID_IDS (arg_node) = TravRightIDS (ID_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRcond(node *arg_node , node *arg_info)
 *
 * description:
 *  traverses both conditional blocks. removes whole conditional if both
 *  parts are empty else traverse condition.
 *
 *
 ******************************************************************************/
node *
SSADCRcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSADCRcond");

    /* traverse else part */
    if (COND_ELSE (arg_node) != NULL) {
        COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);
    }

    /* traverse then part */
    if (COND_THEN (arg_node) != NULL) {
        COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);
    }

    if (SSADCRCondIsEmpty (arg_node)) {
        /* remove whole conditional */

        arg_node = FreeNode (arg_node);
        INFO_SSADCR_REMASSIGN (arg_info) = TRUE;
    } else {
        /* traverse condition */
        DBUG_ASSERT ((COND_COND (arg_node) != NULL), "conditional without condition");

        COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);

        INFO_SSADCR_REMASSIGN (arg_info) = FALSE;
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRdo(node *arg_node , node *arg_info)
 *
 * description:
 *   must not be executed in traversal of ssa-form
 *
 ******************************************************************************/
node *
SSADCRdo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSADCRdo");

    DBUG_ASSERT ((FALSE), "there must not be any do-statement in ssa-form!");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRwhile(node *arg_node , node *arg_info)
 *
 * description:
 *   must not be executed in traversal of ssa-form
 *
 ******************************************************************************/
node *
SSADCRwhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSADCRwhile");

    DBUG_ASSERT ((FALSE), "there must not be any while statement in ssa-form!");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRreturn(node *arg_node , node *arg_info)
 *
 * description:
 *   starts traversal of return expressions to mark them as needed.
 *
 ******************************************************************************/
node *
SSADCRreturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSADCRreturn");

    if (RETURN_EXPRS (arg_node) != NULL) {
        RETURN_EXPRS (arg_node) = Trav (RETURN_EXPRS (arg_node), arg_info);
    }

    /* do not remove return instruction */
    INFO_SSADCR_REMASSIGN (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRap(node *arg_node , node *arg_info)
 *
 * description:
 *   if application of special function (cond, do, while) traverse into this
 *   function except for recursive calls of the current function.
 *   traverse all arguments to marks them as needed
 *
 ******************************************************************************/
node *
SSADCRap (node *arg_node, node *arg_info)
{
    node *new_arg_info;

    DBUG_ENTER ("SSADCRap");

    /* traverse special fundef without recursion */
    if (((FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_condfun)
         || (FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_dofun)
         || (FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_whilefun))
        && (AP_FUNDEF (arg_node) != INFO_SSADCR_FUNDEF (arg_info))) {
        DBUG_PRINT ("SSADCR", ("traverse in special fundef %s",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));
        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        INFO_SSADCR_DEPTH (new_arg_info) = INFO_SSADCR_DEPTH (arg_info) + 1;

        /* start traversal of special fundef (and maybe reduce parameters!) */
        AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), new_arg_info);

        DBUG_PRINT ("SSADCR", ("traversal of special fundef %s finished\n",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));

        FREE (new_arg_info);
    } else {
        DBUG_PRINT ("SSADCR", ("do not traverse in normal fundef %s",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));
    }

    /* ##nmw## maybe remove whole function call if necessary ??? */
    /* mark all args as needed */
    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRNwith(node *arg_node , node *arg_info)
 *
 * description:
 *   traverses withop, code and partitions of withloop
 *
 ******************************************************************************/
node *
SSADCRNwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSADCRNwith");

    /* traverse withop */
    if (NWITH_WITHOP (arg_node) != NULL) {
        NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);
    }

    /* traverse code */
    if (NWITH_CODE (arg_node) != NULL) {
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    }

    /* traverse withop */
    if (NWITH_PART (arg_node) != NULL) {
        NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRNpart(node *arg_node , node *arg_info)
 *
 * description:
 *   traverses generator, withid and next part
 *
 ******************************************************************************/
node *
SSADCRNpart (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSADCRNpart");

    /* traverse generator */
    if (NPART_GEN (arg_node) != NULL) {
        NPART_GEN (arg_node) = Trav (NPART_GEN (arg_node), arg_info);
    }

    /* traverse withid */
    if (NPART_WITHID (arg_node) != NULL) {
        NPART_WITHID (arg_node) = Trav (NPART_WITHID (arg_node), arg_info);
    }
    /* traverse next part */
    if (NPART_NEXT (arg_node) != NULL) {
        NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRNcode(node *arg_node , node *arg_info)
 *
 * description:
 *   traverses expr, block and next in this order
 *
 ******************************************************************************/
node *
SSADCRNcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSADCRNcode");

    /* traverse expression */
    if (NCODE_CEXPR (arg_node) != NULL) {
        NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);
    }

    /* traverse code block */
    if (NCODE_CBLOCK (arg_node) != NULL) {
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    }

    /* traverse expression */
    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRNwithid(node *arg_node , node *arg_info)
 *
 * description:
 *   marks index vector and identifier as needed to prevent their removal
 *   if they are not explicit used in Withloop.
 *   to do so these identifier are handeld like ids on the RIGHT side of
 *   an assignment.
 *
 ******************************************************************************/
node *
SSADCRNwithid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSADCRNwithid");

    /* traverse ids */
    if (NWITHID_IDS (arg_node) != NULL) {
        NWITHID_IDS (arg_node) = TravRightIDS (NWITHID_IDS (arg_node), arg_info);
    }

    /* traverse vec */
    if (NWITHID_VEC (arg_node) != NULL) {
        NWITHID_VEC (arg_node) = TravRightIDS (NWITHID_VEC (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   static ids *SSADCRleftids(ids *arg_ids, node *arg_info)
 *
 * description:
 *   checks if ids is marked as needed and increments counter.
 *   if results of a special function, unused results are removed.
 *   traverses to next ids.
 *
 ******************************************************************************/
static ids *
SSADCRleftids (ids *arg_ids, node *arg_info)
{
    ids *next_ids;
    nodelist *letlist;

    DBUG_ENTER ("SSADCRleftids");

    /* position of result in result-list */
    INFO_SSADCR_RESCOUNT (arg_info) = INFO_SSADCR_RESCOUNT (arg_info) + 1;

    /* check result for usage */
    if (AVIS_NEEDCOUNT (IDS_AVIS (arg_ids)) == SSADCR_NOTNEEDED) {
        /*
         * result is not needed
         * if result of special function - remove this result
         * but first: save next ids in chain
         */

        next_ids = IDS_NEXT (arg_ids);

        if (INFO_SSADCR_REMRESULTS (arg_info)) {
            DBUG_PRINT ("SSADCR",
                        ("check left side %s  - not needed -> remove from resultlist",
                         VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (IDS_AVIS (arg_ids)))));

            /* remove result from all function applications - except this one */
            letlist = NULL;
            if (FUNDEF_EXT_ASSIGN (INFO_SSADCR_APFUNDEF (arg_info)) != NULL) {
                if (ASSIGN_INSTR (FUNDEF_EXT_ASSIGN (INFO_SSADCR_APFUNDEF (arg_info)))
                    != INFO_SSADCR_LET (arg_info)) {
                    letlist = NodeListAppend (letlist,
                                              ASSIGN_INSTR (FUNDEF_EXT_ASSIGN (
                                                INFO_SSADCR_APFUNDEF (arg_info))),
                                              NULL);
                }
            }
            if (FUNDEF_INT_ASSIGN (INFO_SSADCR_APFUNDEF (arg_info)) != NULL) {
                if (ASSIGN_INSTR (FUNDEF_INT_ASSIGN (INFO_SSADCR_APFUNDEF (arg_info)))
                    != INFO_SSADCR_LET (arg_info)) {
                    letlist = NodeListAppend (letlist,
                                              ASSIGN_INSTR (FUNDEF_INT_ASSIGN (
                                                INFO_SSADCR_APFUNDEF (arg_info))),
                                              NULL);
                }
            }

            INFO_SSADCR_APFUNDEF (arg_info)
              = CSRemoveResult (INFO_SSADCR_APFUNDEF (arg_info),
                                INFO_SSADCR_RESCOUNT (arg_info), letlist);

            FreeNodelist (letlist);
            FreeOneIds (arg_ids);

            /* decrement position in resultlist after deleting the result */
            INFO_SSADCR_RESCOUNT (arg_info) = INFO_SSADCR_RESCOUNT (arg_info) - 1;

            /* traverse rest of result chain */
            if (next_ids != NULL) {
                next_ids = TravLeftIDS (next_ids, arg_info);
            }

            /* set correct return value */
            arg_ids = next_ids;

        } else {
            DBUG_PRINT ("SSADCR",
                        ("check left side %s  - not needed",
                         VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (IDS_AVIS (arg_ids)))));

            /* variable is not needed, but mark variable as needed to preserve vardec */
            AVIS_NEEDCOUNT (IDS_AVIS (arg_ids)) = AVIS_NEEDCOUNT (IDS_AVIS (arg_ids)) + 1;

            /* traverse next ids as usual */
            if (IDS_NEXT (arg_ids) != NULL) {
                IDS_NEXT (arg_ids) = TravLeftIDS (IDS_NEXT (arg_ids), arg_info);
            }
        }
    } else {
        DBUG_PRINT ("SSADCR",
                    ("check left side %s  - needed",
                     VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (IDS_AVIS (arg_ids)))));

        /* result is needed, increment counter */
        INFO_SSADCR_RESNEEDED (arg_info) = INFO_SSADCR_RESNEEDED (arg_info) + 1;

        /* traverse next ids */
        if (IDS_NEXT (arg_ids) != NULL) {
            IDS_NEXT (arg_ids) = TravLeftIDS (IDS_NEXT (arg_ids), arg_info);
        }
    }

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   static ids *SSADCRrightids(ids *arg_ids, node *arg_info)
 *
 * description:
 *  increments the counter for each identifier usage
 *
 ******************************************************************************/
static ids *
SSADCRrightids (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("SSADCRrightids");

    DBUG_PRINT ("SSADCR", ("mark ids %s as needed",
                           VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (IDS_AVIS (arg_ids)))));

    AVIS_NEEDCOUNT (IDS_AVIS (arg_ids)) = AVIS_NEEDCOUNT (IDS_AVIS (arg_ids)) + 1;

    /* traverse next ids in ids chain */
    if (IDS_NEXT (arg_ids) != NULL) {
        IDS_NEXT (arg_ids) = TravRightIDS (IDS_NEXT (arg_ids), arg_info);
    }

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   static ids *TravLeftIDS(ids *arg_ids, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
static ids *
TravLeftIDS (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("TravLeftIDS");

    DBUG_ASSERT (arg_ids != NULL, "traversal in NULL ids");
    arg_ids = SSADCRleftids (arg_ids, arg_info);

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   static ids *TravRightIDS(ids *arg_ids, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
static ids *
TravRightIDS (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("TravRightIDS");

    DBUG_ASSERT (arg_ids != NULL, "traversal in NULL ids");
    arg_ids = SSADCRrightids (arg_ids, arg_info);

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   node *SSADeadCodeRemoval(node *fundef)
 *
 * description:
 *   starting point of DeadCodeRemoval for SSA form.
 *   Starting fundef must not be a special fundef (do, while, cond) created by
 *   lac2fun transformation. These "inline" functions will be traversed in their
 *   order of usage. The traversal mode (on toplevel, in special function) is
 *   annotated in the stacked INFO_SSADCR_DEPTH attribute.
 *
 ******************************************************************************/
node *
SSADeadCodeRemoval (node *fundef)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("SSADeadCodeRemoval");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "SSADeadCodeRemoval called for non-fundef node");

    /* do not start traversal in special functions */
    if ((FUNDEF_STATUS (fundef) != ST_condfun) && (FUNDEF_STATUS (fundef) != ST_dofun)
        && (FUNDEF_STATUS (fundef) != ST_whilefun)) {
        arg_info = MakeInfo ();

        INFO_SSADCR_DEPTH (arg_info) = SSADCR_TOPLEVEL; /* start on toplevel */

        old_tab = act_tab;
        act_tab = ssadcr_tab;

        fundef = Trav (fundef, arg_info);

        act_tab = old_tab;

        FREE (arg_info);
    }

    DBUG_RETURN (fundef);
}
