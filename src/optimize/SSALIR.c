/*
 *
 * $Log$
 * Revision 1.5  2001/04/05 12:33:58  nmw
 * detection of loop invarinat expression implemented
 *
 * Revision 1.4  2001/04/04 09:55:44  nmw
 * missing include added
 *
 * Revision 1.3  2001/04/02 11:08:20  nmw
 * handling for multiple used special functions added
 *
 * Revision 1.2  2001/03/29 16:31:55  nmw
 * detection of loop invariant args implemented
 *
 * Revision 1.1  2001/03/26 15:37:45  nmw
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   SSALIR.c
 *
 * prefix: SSALIR
 *
 * description:
 *   this module implements loop invariant removal on code in ssa form.
 *   (only do-loops are directly supported by this algorithm, while-loops
 *   have to be transformed in do-loops!)
 *
 *****************************************************************************/

#include "dbug.h"
#include "tree_basic.h"
#include "internal_lib.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "optimize.h"
#include "SSALIR.h"
#include "tree_compound.h"

/* INFO_SSALIR_CONDSTATUS */
#define CONDSTATUS_NOCOND 0
#define CONDSTATUS_THENPART 1
#define CONDSTATUS_ELSEPART 2

/* INFO_SSALIR_FLAG */
#define SSALIR_NORMAL 0
#define SSALIR_MOVEUP 1
#define SSALIR_INRETURN 2

/* functions for local usage only */
static ids *TravLeftIDS (ids *arg_ids, node *arg_info);
static ids *SSALIRleftids (ids *arg_ids, node *arg_info);

/* traversal functions */
/******************************************************************************
 *
 * function:
 *   node* SSALIRfundef(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSALIRfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRfundef");

    DBUG_PRINT ("SSALIR",
                ("loop invariant removal in fundef %s", FUNDEF_NAME (arg_node)));

    INFO_SSALIR_FUNDEF (arg_info) = arg_node;

    /* traverse args of special (loop) functions to infere loop invariant args */
    if ((FUNDEF_ARGS (arg_node) != NULL) && (FUNDEF_IS_LOOPFUN (arg_node))) {

        DBUG_ASSERT ((FUNDEF_INT_ASSIGN (INFO_SSALIR_FUNDEF (arg_info)) != NULL),
                     "missing assignment link to internal recursive call");
        DBUG_ASSERT ((ASSIGN_INSTR (FUNDEF_INT_ASSIGN (INFO_SSALIR_FUNDEF (arg_info)))
                      != NULL),
                     "missing assigment instruction");
        DBUG_ASSERT ((NODE_TYPE (LET_EXPR (
                        ASSIGN_INSTR (FUNDEF_INT_ASSIGN (INFO_SSALIR_FUNDEF (arg_info)))))
                      == N_ap),
                     "missing recursive call in do/while special function");

        /* save pointer to argchain of recursive function application */
        INFO_SSALIR_ARGCHAIN (arg_info) = AP_ARGS (
          LET_EXPR (ASSIGN_INSTR (FUNDEF_INT_ASSIGN (INFO_SSALIR_FUNDEF (arg_info)))));
    } else {
        /* do loop functions with recursive call */
        INFO_SSALIR_ARGCHAIN (arg_info) = NULL;
    }

    /* traverse args */
    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    /* traverse function body */
    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALIRarg(node *arg_node, node *arg_info)
 *
 * description:
 *   in do/while special functions: set the SSALIR attribute for the args by
 *   comparing the args with the corresponding identifier in the recursive
 *   call. if they are identical the args is a loop invariant arg and will be
 *   tagged.
 *
 ******************************************************************************/
node *
SSALIRarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRarg");

    /* infere loop invarinat args */
    if (INFO_SSALIR_ARGCHAIN (arg_info) != NULL) {
        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (INFO_SSALIR_ARGCHAIN (arg_info)))),
                     "function args are no identifiers");

        /* compare arg and fun-ap argument */
        if (ARG_AVIS (arg_node)
            == ID_AVIS (EXPRS_EXPR (INFO_SSALIR_ARGCHAIN (arg_info)))) {
            DBUG_PRINT ("SSALIR", ("mark %s as loop invariant", ARG_NAME (arg_node)));
            if (AVIS_SSALPINV (ARG_AVIS (arg_node)) != TRUE) {
                lir_expr++;
                AVIS_SSALPINV (ARG_AVIS (arg_node)) = TRUE;
            }
        } else {
            DBUG_PRINT ("SSALIR", ("mark %s as non loop invariant", ARG_NAME (arg_node)));
        }
    }

    /* init other data */
    AVIS_NEEDCOUNT (ARG_AVIS (arg_node)) = 0;
    AVIS_DEFDEPTH (ARG_AVIS (arg_node)) = 0;

    if (ARG_NEXT (arg_node) != NULL) {
        /* when checking for LI-args traverse to next parameter of recursive call */
        if (INFO_SSALIR_ARGCHAIN (arg_info) != NULL) {
            INFO_SSALIR_ARGCHAIN (arg_info)
              = EXPRS_NEXT (INFO_SSALIR_ARGCHAIN (arg_info));
        }

        /* traverse to next arg */
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALIRvardec(node *arg_node, node *arg_info)
 *
 * description:
 *   init data for SSALIR traversal
 *
 ******************************************************************************/
node *
SSALIRvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRvardec");

    AVIS_NEEDCOUNT (VARDEC_AVIS (arg_node)) = 0;
    AVIS_DEFDEPTH (VARDEC_AVIS (arg_node)) = 0;
    AVIS_SSALPINV (VARDEC_AVIS (arg_node)) = FALSE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALIRblock(node *arg_node, node *arg_info)
 *
 * description:
 *   traverse vardecs and block in this order
 *
 ******************************************************************************/
node *
SSALIRblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }

    /* top level (not [directly] contained in any withloop) */
    INFO_SSALIR_WITHDEPTH (arg_info) = 0;

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALIRassign(node *arg_node, node *arg_info)
 *
 * description:
 *   traverse assign instructions in top-down order to infere LI-assignments.
 *
 ******************************************************************************/
node *
SSALIRassign (node *arg_node, node *arg_info)
{
    bool remove_assign;

    DBUG_ENTER ("SSALIRassign");

    DBUG_ASSERT ((ASSIGN_INSTR (arg_node)), "missing instruction in assignment");

    /* init traversal flags */
    INFO_SSALIR_REMASSIGN (arg_info) = FALSE;
    INFO_SSALIR_ASSIGN (arg_info) = arg_node;

    /* start traversl in instruction */
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    INFO_SSALIR_ASSIGN (arg_info) = NULL;
    remove_assign = INFO_SSALIR_REMASSIGN (arg_info);
    INFO_SSALIR_REMASSIGN (arg_info) = FALSE;

    /* traverse next assignment */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALIRlet(node *arg_node, node *arg_info)
 *
 * description:
 *   traverse let expression and result identifiers
 *
 ******************************************************************************/
node *
SSALIRlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRlet");

    if (INFO_SSALIR_WITHDEPTH (arg_info) == 0) {
        /* on toplevel: start counting non-lir args in expression */
        INFO_SSALIR_NONLIRUSE (arg_info) = 0;
    }

    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
    }

    /*
     * expression is on top-level of do-loop, not in a condition and uses only LI
     * arguments -> move up expression in front of loop
     */
    if ((INFO_SSALIR_WITHDEPTH (arg_info) == 0) && (INFO_SSALIR_NONLIRUSE (arg_info) == 0)
        && (INFO_SSALIR_CONDSTATUS (arg_info) == CONDSTATUS_NOCOND)
        && (FUNDEF_STATUS (INFO_SSALIR_FUNDEF (arg_info)) == ST_dofun)) {
        DBUG_PRINT ("SSALIR", ("loop independend expression detected - moving up"));
        INFO_SSALIR_FLAG (arg_info) = SSALIR_MOVEUP;
    } else {
        INFO_SSALIR_FLAG (arg_info) = SSALIR_NORMAL;
    }

    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TravLeftIDS (LET_IDS (arg_node), arg_info);
    }

    /* step back to normal mode */
    INFO_SSALIR_FLAG (arg_info) = SSALIR_NORMAL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALIRid(node *arg_node, node *arg_info)
 *
 * description:
 *   normal mode:
 *     checks identifier for being loop invariant or increments nonlituse counter
 *     always increments the needed counter.
 *
 *   inreturn mode:
 *     checks for move down assignments
 *
 ******************************************************************************/
node *
SSALIRid (node *arg_node, node *arg_info)
{
    node *id;

    DBUG_ENTER ("SSALIRid");

    switch (INFO_SSALIR_FLAG (arg_info)) {
    case SSALIR_NORMAL:
        /* increment need/uses counter */
        AVIS_NEEDCOUNT (ID_AVIS (arg_node)) = AVIS_NEEDCOUNT (ID_AVIS (arg_node)) + 1;

        /* if id is NOT loop invariant, increment nonliruse counter */
        if (!(AVIS_SSALPINV (ID_AVIS (arg_node)))) {
            INFO_SSALIR_NONLIRUSE (arg_info) = INFO_SSALIR_NONLIRUSE (arg_info) + 1;
        }
        break;

    case SSALIR_INRETURN:
        if (AVIS_SSAPHITARGET (ID_AVIS (arg_node))) {
            DBUG_ASSERT ((AVIS_SSAASSIGN2 (ID_AVIS (arg_node)) != NULL),
                         "missing definition assignment in else-part");

            DBUG_ASSERT ((NODE_TYPE (
                            ASSIGN_INSTR (AVIS_SSAASSIGN2 ((ID_AVIS (arg_node)))))
                          == N_let),
                         "non let assignment node");

            if ((NODE_TYPE (
                   LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN2 (ID_AVIS (arg_node)))))
                 == N_id)
                && (AVIS_NEEDCOUNT (ID_AVIS (
                      LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN2 (ID_AVIS (arg_node))))))
                    == 1)) {
                /*
                 * return of identifier that is only used once in phi copy assignment:
                 * this identifier can be moved down behind the loop, because it is not
                 * needed in the loop.
                 */

                id = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN2 (ID_AVIS (arg_node))));

                DBUG_PRINT ("SSALIR",
                            ("loop invarinat assignment - move down of %s (return %s)",
                             VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (ID_AVIS (id))),
                             VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (ID_AVIS (arg_node)))));
            }
        }
        break;

    default:
        DBUG_ASSERT ((FALSE), "unable to handle SSALIR_FLAG in SSALIRid");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALIRap(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses in dependend special function and integrates pre/post-assignment
 *   code.
 *
 ******************************************************************************/
node *
SSALIRap (node *arg_node, node *arg_info)
{
    node *new_arg_info;

    DBUG_ENTER ("SSALIRap");

    DBUG_ASSERT ((AP_FUNDEF (arg_node) != NULL), "missing fundef in ap-node");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }

    /* traverse special fundef without recursion */
    if ((FUNDEF_IS_LACFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_SSALIR_FUNDEF (arg_info))) {
        DBUG_PRINT ("SSALIR", ("traverse in special fundef %s",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));

        INFO_SSALIR_MODUL (arg_info)
          = CheckAndDupSpecialFundef (INFO_SSALIR_MODUL (arg_info), AP_FUNDEF (arg_node),
                                      INFO_SSALIR_ASSIGN (arg_info));

        DBUG_ASSERT ((FUNDEF_USED (AP_FUNDEF (arg_node)) == 1),
                     "more than one instance of special function used.");

        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();
        INFO_SSALIR_MODUL (new_arg_info) = INFO_SSALIR_MODUL (arg_info);

        /* start traversal of special fundef */
        AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), new_arg_info);

        DBUG_PRINT ("SSALIR", ("traversal of special fundef %s finished\n",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));
        FREE (new_arg_info);

    } else {
        /* no traversal into a normal fundef */
        DBUG_PRINT ("SSALIR", ("do not traverse in normal fundef %s",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALIRcond(node *arg_node, node *arg_info)
 *
 * description:
 *   set the correct conditional status flag and traverse the condition and
 *   the then and else blocks.
 *
 * remark:
 *   in ssaform there can be only one conditional per special functions, so
 *   there is no need to stack the information here.
 *
 ******************************************************************************/
node *
SSALIRcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRcond");

    /* traverse condition */
    INFO_SSALIR_CONDSTATUS (arg_info) = CONDSTATUS_NOCOND;
    COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);

    /* traverse then part */
    INFO_SSALIR_CONDSTATUS (arg_info) = CONDSTATUS_THENPART;
    COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);

    /* traverse else part */
    INFO_SSALIR_CONDSTATUS (arg_info) = CONDSTATUS_ELSEPART;
    COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);

    /* leaving conditional */
    INFO_SSALIR_CONDSTATUS (arg_info) = CONDSTATUS_NOCOND;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALIRreturn(node *arg_node, node *arg_info)
 *
 * description:
 *   look for move-down assignments and integrate them?
 *
 ******************************************************************************/
node *
SSALIRreturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRreturn");

    INFO_SSALIR_FLAG (arg_info) = SSALIR_INRETURN;
    RETURN_EXPRS (arg_node) = Trav (RETURN_EXPRS (arg_node), arg_info);
    INFO_SSALIR_FLAG (arg_info) = SSALIR_NORMAL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALIRNwith(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses with-loop, increments withdepth counter during traversal
 *
 ******************************************************************************/
node *
SSALIRNwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRNwith");

    /* increment withdepth counter */
    INFO_SSALIR_WITHDEPTH (arg_info) = INFO_SSALIR_WITHDEPTH (arg_info) + 1;

    /* traverse partition */
    NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);

    /* travserse code blocks */
    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

    /* traverse withop */
    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

    /* decrement withdepth counter */
    INFO_SSALIR_WITHDEPTH (arg_info) = INFO_SSALIR_WITHDEPTH (arg_info) - 1;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALIRNwithid(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSALIRNwithid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRNwithid");

    /* traverse all definitions to mark their depth in withloops */
    NWITHID_IDS (arg_node) = TravLeftIDS (NWITHID_IDS (arg_node), arg_info);
    NWITHID_VEC (arg_node) = TravLeftIDS (NWITHID_VEC (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   ids *TravLeftIDS(ids *arg_ids, node *arg_info)
 *
 * description:
 *   similar implementation of trav mechanism as used for nodes
 *   here used for ids.
 *
 ******************************************************************************/
static ids *
TravLeftIDS (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("TravLeftIDS");

    DBUG_ASSERT (arg_ids != NULL, "traversal in NULL ids");
    arg_ids = SSALIRleftids (arg_ids, arg_info);

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   static ids *SSALIRleftids (ids *arg_ids, node *arg_info)
 *
 * description:
 *   set current withloop depth as definition depth
 *
 ******************************************************************************/
static ids *
SSALIRleftids (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("SSALIRleftids");

    /* set current withloop depth as definition depth */
    AVIS_DEFDEPTH (IDS_AVIS (arg_ids)) = INFO_SSALIR_WITHDEPTH (arg_info);

    if (INFO_SSALIR_FLAG (arg_info) == SSALIR_MOVEUP) {
        DBUG_PRINT ("SSALIR", ("moving up vardec %s",
                               VARDEC_NAME (AVIS_VARDECORARG (IDS_AVIS (arg_ids)))));
        AVIS_SSALPINV (IDS_AVIS (arg_ids)) = TRUE;
    }

    /* update AVIS_SSAASSIGN(2) attributes */
    if ((INFO_SSALIR_CONDSTATUS (arg_info) == CONDSTATUS_ELSEPART)
        && (AVIS_SSAPHITARGET (IDS_AVIS (arg_ids)) != PHIT_NONE)) {
        /* set AVIS_ASSIGN2 attribute for second definition of phitargets */
        AVIS_SSAASSIGN2 (IDS_AVIS (arg_ids)) = INFO_SSALIR_ASSIGN (arg_info);
    } else {
        /* set AVIS_ASSIGN attribute */
        AVIS_SSAASSIGN (IDS_AVIS (arg_ids)) = INFO_SSALIR_ASSIGN (arg_info);
    }

    if (IDS_NEXT (arg_ids) != NULL) {
        IDS_NEXT (arg_ids) = TravLeftIDS (IDS_NEXT (arg_ids), arg_info);
    }

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   node* SSALoopInvariantRemoval(node* fundef, node* modul)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSALoopInvariantRemoval (node *fundef, node *modul)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("SSALoopInvariantRemoval");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "SSALoopInvariantRemoval called for non-fundef node");

    DBUG_PRINT ("SSALIR", ("starting loop independent removal in function %s",
                           FUNDEF_NAME (fundef)));

    /* do not start traversal in special functions */
    if ((FUNDEF_STATUS (fundef) != ST_condfun) && (FUNDEF_STATUS (fundef) != ST_dofun)
        && (FUNDEF_STATUS (fundef) != ST_whilefun)) {
        arg_info = MakeInfo ();
        INFO_SSALIR_MODUL (arg_info) = modul;

        old_tab = act_tab;
        act_tab = ssalir_tab;

        fundef = Trav (fundef, arg_info);

        act_tab = old_tab;

        FREE (arg_info);
    }

    DBUG_RETURN (fundef);
}
