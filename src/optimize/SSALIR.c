/*
 *
 * $Log$
 * Revision 1.7  2001/04/12 12:40:26  nmw
 * move up of loop independen expressions in do loops implemented
 *
 * Revision 1.6  2001/04/09 15:57:08  nmw
 * first implementation of code move up (not tested yet)
 *
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
#include "LookUpTable.h"
#include "change_signature.h"

/* INFO_SSALIR_CONDSTATUS */
#define CONDSTATUS_NOCOND 0
#define CONDSTATUS_THENPART 1
#define CONDSTATUS_ELSEPART 2

/* INFO_SSALIR_FLAG (mode of traversal) */
#define SSALIR_NORMAL 0
#define SSALIR_MOVEUP 1
#define SSALIR_INRETURN 2
#define SSALIR_MOVELOCAL 3
#define SSALIR_MOVEDOWN 4

/* AVIS_LIRMOVE / LET_LIRFLAG */
#define LIRMOVE_NONE 0x0
#define LIRMOVE_UP 0x1
#define LIRMOVE_DOWN 0x2
#define LIRMOVE_LOCAL 0x4

/* functions for local usage only */
static ids *SSALIRleftids (ids *arg_ids, node *arg_info);
static ids *LIRMOVleftids (ids *arg_ids, node *arg_info);
static node *CheckMoveDownFlag (node *instr, node *arg_info);

/******************************************************************************
 *
 * function:
 *   ids *CheckMoveDownFlag(node *instr)
 *
 * description:
 *   checks the given assign-instruction (should be a let) to have all results
 *   marked as move_down to mark the whole let for move_down. a partial move
 *   down of expressions is not supported.
 *
 *****************************************************************************/
static node *
CheckMoveDownFlag (node *instr, node *arg_info)
{
    ids *result;
    int non_move_down;
    int move_down;

    DBUG_ENTER ("");

    if (NODE_TYPE (instr) == N_let) {
        /* traverse result-chain and check for move_down flags */
        result = LET_IDS (instr);
        non_move_down = 0;
        move_down = 0;

        while (result != NULL) {
            if (AVIS_LIRMOVE (IDS_AVIS (result)) & LIRMOVE_DOWN) {
                move_down++;
            } else {
                non_move_down++;
            }
            result = IDS_NEXT (result);
        }

        if ((move_down > 0) && (non_move_down == 0)) {
            /*
             * all results are marked for move_down, so expression can
             * be moved down
             */
            LET_LIRFLAG (instr) = LET_LIRFLAG (instr) | LIRMOVE_DOWN;
            DBUG_PRINT ("SSALIR", ("whole expression marked for move-down"));
        }
    }

    DBUG_RETURN (instr);
}

/* traversal functions */
/******************************************************************************
 *
 * function:
 *   node* SSALIRfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses fundef two times:
 *      first: infere expressions to move (and do WLIR locally)
 *     second: do the external code movement and fundef adjustment
 *
 *****************************************************************************/
node *
SSALIRfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRfundef");

    DBUG_PRINT ("SSALIR",
                ("loop invariant removal in fundef %s", FUNDEF_NAME (arg_node)));

    INFO_SSALIR_FUNDEF (arg_info) = arg_node;

    /* build up LUT for vardec move/rename operartions */
    if (FUNDEF_IS_LOOPFUN (arg_node)) {
        INFO_SSALIR_MOVELUT (arg_info) = GenerateLUT ();
    } else {
        INFO_SSALIR_MOVELUT (arg_info) = NULL;
    }

    /* init empty result map */
    INFO_SSALIR_RESULTMAP (arg_info) = NULL;

    /* traverse args of special (loop) functions to infere loop invariant args */
    if ((FUNDEF_ARGS (arg_node) != NULL) && (FUNDEF_IS_LOOPFUN (arg_node))) {

        DBUG_ASSERT ((FUNDEF_INT_ASSIGN (arg_node) != NULL),
                     "missing assignment link to internal recursive call");
        DBUG_ASSERT ((ASSIGN_INSTR (FUNDEF_INT_ASSIGN (arg_node)) != NULL),
                     "missing internal assigment instruction");
        DBUG_ASSERT ((NODE_TYPE (LET_EXPR (ASSIGN_INSTR (FUNDEF_INT_ASSIGN (arg_node))))
                      == N_ap),
                     "missing recursive call in do/while special function");

        /* save pointer to argchain of recursive function application */
        INFO_SSALIR_ARGCHAIN (arg_info) = AP_ARGS (
          LET_EXPR (ASSIGN_INSTR (FUNDEF_INT_ASSIGN (INFO_SSALIR_FUNDEF (arg_info)))));

        DBUG_ASSERT ((FUNDEF_EXT_ASSIGNS (arg_node) != NULL),
                     "missing external function application nodelist");

        DBUG_ASSERT ((NODELIST_NEXT (FUNDEF_EXT_ASSIGNS (arg_node)) == NULL),
                     "more than one external function application to special function");

        DBUG_ASSERT ((NODELIST_NODE (FUNDEF_EXT_ASSIGNS (arg_node)) != NULL),
                     "missing external assignment");

        DBUG_ASSERT ((ASSIGN_INSTR (NODELIST_NODE (FUNDEF_EXT_ASSIGNS (arg_node)))
                      != NULL),
                     "missing external assigment instruction");

        DBUG_ASSERT ((NODE_TYPE (LET_EXPR (
                        ASSIGN_INSTR (NODELIST_NODE (FUNDEF_EXT_ASSIGNS (arg_node)))))
                      == N_ap),
                     "missing recursive call in do/while special function");

        /* save pointer to archain of external function application */
        INFO_SSALIR_APARGCHAIN (arg_info) = AP_ARGS (
          LET_EXPR (ASSIGN_INSTR (NODELIST_NODE (FUNDEF_EXT_ASSIGNS (arg_node)))));

    } else {
        /* non loop function */
        INFO_SSALIR_ARGCHAIN (arg_info) = NULL;
        INFO_SSALIR_APARGCHAIN (arg_info) = NULL;
    }

    /* traverse args */
    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    /* top level (not [directly] contained in any withloop) */
    INFO_SSALIR_WITHDEPTH (arg_info) = 0;

    /* traverse function body */
    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    /* start LIRMOV traversal of BODY to move out marked assignments */
    act_tab = lirmov_tab;
    FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    act_tab = ssalir_tab;

    /* clean up LUT */
    if (INFO_SSALIR_MOVELUT (arg_info) != NULL) {
        RemoveLUT (INFO_SSALIR_MOVELUT (arg_info));
    }

    /* clean up result map nodelist */
    if (INFO_SSALIR_RESULTMAP (arg_info) != NULL) {
        INFO_SSALIR_RESULTMAP (arg_info)
          = NodeListFree (INFO_SSALIR_RESULTMAP (arg_info), 0);
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
 *****************************************************************************/
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

    /* build up LUT between args and their corresponding calling vardecs */
    if ((INFO_SSALIR_MOVELUT (arg_info) != NULL)
        && (INFO_SSALIR_APARGCHAIN (arg_info) != NULL)) {
        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (INFO_SSALIR_APARGCHAIN (arg_info))) == N_id),
                     "non N_id node in function application");

        /* add internal->external connections to LUT: */
        /* vardec */
        INFO_SSALIR_MOVELUT (arg_info)
          = InsertIntoLUT_P (INFO_SSALIR_MOVELUT (arg_info), arg_node,
                             ID_VARDEC (EXPRS_EXPR (INFO_SSALIR_APARGCHAIN (arg_info))));

        /* avis */
        INFO_SSALIR_MOVELUT (arg_info)
          = InsertIntoLUT_P (INFO_SSALIR_MOVELUT (arg_info), ARG_AVIS (arg_node),
                             ID_AVIS (EXPRS_EXPR (INFO_SSALIR_APARGCHAIN (arg_info))));

        /* id name */
        INFO_SSALIR_MOVELUT (arg_info)
          = InsertIntoLUT_S (INFO_SSALIR_MOVELUT (arg_info), ARG_NAME (arg_node),
                             VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (ID_AVIS (
                               EXPRS_EXPR (INFO_SSALIR_APARGCHAIN (arg_info))))));
    }

    /* init avis data for argument */
    AVIS_NEEDCOUNT (ARG_AVIS (arg_node)) = 0;
    AVIS_DEFDEPTH (ARG_AVIS (arg_node)) = 0;
    AVIS_LIRMOVE (ARG_AVIS (arg_node)) = LIRMOVE_NONE;
    AVIS_EXPRESULT (ARG_AVIS (arg_node)) = FALSE;

    if (ARG_NEXT (arg_node) != NULL) {
        /* when checking for LI-args traverse to next parameter of recursive call */
        if (INFO_SSALIR_ARGCHAIN (arg_info) != NULL) {
            INFO_SSALIR_ARGCHAIN (arg_info)
              = EXPRS_NEXT (INFO_SSALIR_ARGCHAIN (arg_info));
        }

        /* when building LUT traverse to next arg pf external call */
        if (INFO_SSALIR_APARGCHAIN (arg_info) != NULL) {
            INFO_SSALIR_APARGCHAIN (arg_info)
              = EXPRS_NEXT (INFO_SSALIR_APARGCHAIN (arg_info));
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
 *   init avis data of vardecs for SSALIR traversal
 *
 ******************************************************************************/
node *
SSALIRvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRvardec");

    AVIS_NEEDCOUNT (VARDEC_AVIS (arg_node)) = 0;
    AVIS_DEFDEPTH (VARDEC_AVIS (arg_node)) = 0;
    AVIS_SSALPINV (VARDEC_AVIS (arg_node)) = FALSE;
    AVIS_LIRMOVE (ARG_AVIS (arg_node)) = LIRMOVE_NONE;
    AVIS_EXPRESULT (ARG_AVIS (arg_node)) = FALSE;

    /* traverse to next vardec */
    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALIRblock(node *arg_node, node *arg_info)
 *
 * description:
 *   traverse vardecs and block in this order
 *   set INFO_SSALIR_TOPBLOCK according to block type
 *
 ******************************************************************************/
node *
SSALIRblock (node *arg_node, node *arg_info)
{
    int old_flag;

    DBUG_ENTER ("SSALIRblock");

    /* save block mode */
    old_flag = INFO_SSALIR_TOPBLOCK (arg_info);

    if (FUNDEF_BODY (INFO_SSALIR_FUNDEF (arg_info)) == arg_node) {
        /* top block */
        INFO_SSALIR_TOPBLOCK (arg_info) = TRUE;
    } else {
        /* any other block */
        INFO_SSALIR_TOPBLOCK (arg_info) = FALSE;
    }

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    /* restore block mode */
    INFO_SSALIR_TOPBLOCK (arg_info) = old_flag;

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
    node *pre_assign;
    node *tmp;

    DBUG_ENTER ("SSALIRassign");

    DBUG_ASSERT ((ASSIGN_INSTR (arg_node)), "missing instruction in assignment");

    /* init traversal flags */
    INFO_SSALIR_REMASSIGN (arg_info) = FALSE;
    INFO_SSALIR_ASSIGN (arg_info) = arg_node;
    INFO_SSALIR_PREASSIGN (arg_info) = NULL;
    INFO_SSALIR_POSTASSIGN (arg_info) = NULL;

    /* start traversl in instruction */
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    /* analyse and store results of instruction traversal */
    INFO_SSALIR_ASSIGN (arg_info) = NULL;
    remove_assign = INFO_SSALIR_REMASSIGN (arg_info);
    INFO_SSALIR_REMASSIGN (arg_info) = FALSE;

    pre_assign = INFO_SSALIR_PREASSIGN (arg_info);
    INFO_SSALIR_PREASSIGN (arg_info) = NULL;

    /* insert post-assign code */
    if (INFO_SSALIR_POSTASSIGN (arg_info) != NULL) {
        ASSIGN_NEXT (arg_node)
          = AppendAssign (INFO_SSALIR_POSTASSIGN (arg_info), ASSIGN_NEXT (arg_node));
        INFO_SSALIR_POSTASSIGN (arg_node) = NULL;
    }

    /* traverse next assignment */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    /* in bottom-up traversal: */
    /*
     * check for complete marked move-down result lists (only in topblock)
     */
    if (INFO_SSALIR_TOPBLOCK (arg_info)) {
        ASSIGN_INSTR (arg_node) = CheckMoveDownFlag (ASSIGN_INSTR (arg_node), arg_info);
    }

    /* remove this assignment */
    if (remove_assign) {
        tmp = arg_node;
        arg_node = ASSIGN_NEXT (arg_node);
        FreeNode (tmp);
    }

    /*
     * insert pre-assign code
     * remark: the pre-assign code has already been traversed in SSALIRap!
     */
    if (pre_assign != NULL) {
        arg_node = AppendAssign (pre_assign, arg_node);
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

    if (INFO_SSALIR_TOPBLOCK (arg_info)) {
        /* on toplevel: start counting non-lir args in expression */
        INFO_SSALIR_NONLIRUSE (arg_info) = 0;
    }

    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
    }

    if (INFO_SSALIR_TOPBLOCK (arg_info)) {
        /* in topblock mark let statement according to the infered data */
        if ((INFO_SSALIR_NONLIRUSE (arg_info) == 0)
            && (INFO_SSALIR_CONDSTATUS (arg_info) == CONDSTATUS_NOCOND)
            && (FUNDEF_STATUS (INFO_SSALIR_FUNDEF (arg_info)) == ST_dofun)) {

            DBUG_PRINT ("SSALIR",
                        ("loop independend expression detected - mark it for moving up"));
            /*
             * expression is  not in a condition and uses only LI
             * arguments -> mark expression for move up in front of loop
             */

            LET_LIRFLAG (arg_node) = LIRMOVE_UP;
            INFO_SSALIR_FLAG (arg_info) = SSALIR_MOVEUP;
        } else {
            LET_LIRFLAG (arg_node) = LIRMOVE_NONE;
            INFO_SSALIR_FLAG (arg_info) = SSALIR_NORMAL;
        }
    } else if (INFO_SSALIR_WITHDEPTH (arg_info) > 0) {
        /* in other blocks (with-loops) */
        if ((INFO_SSALIR_NONLIRUSE (arg_info) == 0)
            && (INFO_SSALIR_CONDSTATUS (arg_info) == CONDSTATUS_NOCOND)) {
            DBUG_PRINT ("SSALIR", ("local expression detected - mark it"));

            INFO_SSALIR_FLAG (arg_info) = SSALIR_MOVELOCAL;
        } else {
            INFO_SSALIR_FLAG (arg_info) = SSALIR_NORMAL;
        }
        LET_LIRFLAG (arg_node) = LIRMOVE_NONE;
    } else {
        /* in all other cases */
        INFO_SSALIR_FLAG (arg_info) = SSALIR_NORMAL;
        LET_LIRFLAG (arg_node) = LIRMOVE_NONE;
    }

    /* traverse ids to mark them as loop-invariant/local or normal */
    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = SSALIRleftids (LET_IDS (arg_node), arg_info);
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
 *     checks identifier for being loop invariant or increments nonlituse
 *     counter always increments the needed counter.
 *
 *   inreturn mode:
 *     checks for move down assignments and set flag in avis node
 *     creates a mapping between local vardec/avis/name and external result
 *     vardec/avis/name for later code movement with LUT. because we do not
 *     want this modification when we move up expressions, we cannot store
 *     these mapping in LUT directly. therefore we save the infered information
 *     in a nodelist RESULTMAP for later access on move_down operations.
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

        /*
         * if id is NOT loop invariant or locally defined
         * increment nonliruse counter
         */
        if (!((AVIS_SSALPINV (ID_AVIS (arg_node)))
              || (AVIS_LIRMOVE (ID_AVIS (arg_node)) & LIRMOVE_LOCAL))) {
            INFO_SSALIR_NONLIRUSE (arg_info) = INFO_SSALIR_NONLIRUSE (arg_info) + 1;

            DBUG_PRINT ("SSALIR",
                        ("non loop invariant/nonlocal id %s",
                         VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (ID_AVIS (arg_node)))));
        } else {
            DBUG_PRINT ("SSALIR",
                        ("loop invariant or local id %s",
                         VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (ID_AVIS (arg_node)))));
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
                 == N_id)) {
                /*
                 * this is the identifier in the loop, that is returned via the
                 * ssa-phicopy assignment
                 */
                id = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN2 (ID_AVIS (arg_node))));

                /*
                 * look for the corresponding result ids in the let_node of
                 * the external function application and add the mapping
                 * [local id -> result ids] to the RESULTMAP nodelist.
                 */
                DBUG_ASSERT ((INFO_SSALIR_APRESCHAIN (arg_info) != NULL),
                             "missing external result ids");

                INFO_SSALIR_RESULTMAP (arg_info)
                  = NodeListAppend (INFO_SSALIR_RESULTMAP (arg_info), ID_AVIS (id),
                                    IDS_AVIS (INFO_SSALIR_APRESCHAIN (arg_info)));
                /* mark variable as being a result of this function */
                AVIS_EXPRESULT (ID_AVIS (id)) = TRUE;

                /*
                 * if return identifier is used only once (in phi copy assignment):
                 * this identifier can be moved down behind the loop, because it is not
                 * needed in the loop.
                 * the marked variables will be checked in the bottom up traversal
                 * to be defined on an left side where all identifiers are marked for
                 * move down (see CheckMoveDownFlag).
                 */
                if (AVIS_NEEDCOUNT (ID_AVIS (
                      LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN2 (ID_AVIS (arg_node))))))
                    == 1) {

                    DBUG_PRINT (
                      "SSALIR",
                      ("loop invariant assignment (marked for move down) [%s, %s]",
                       VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (ID_AVIS (id))),
                       VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (ID_AVIS (arg_node)))));

                    (AVIS_LIRMOVE (ID_AVIS (id))) |= LIRMOVE_DOWN;
                }
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
        INFO_SSALIR_EXTFUNDEF (new_arg_info) = INFO_SSALIR_FUNDEF (arg_info);
        INFO_SSALIR_EXTPREASSIGN (new_arg_info) = NULL;
        INFO_SSALIR_EXTPOSTASSIGN (new_arg_info) = NULL;

        /* start traversal of special fundef */
        AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), new_arg_info);

        DBUG_PRINT ("SSALIR", ("traversal of special fundef %s finished\n",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));

        /* save post/preassign to integrate them in currect assignment chain */
        INFO_SSALIR_PREASSIGN (arg_info) = INFO_SSALIR_EXTPREASSIGN (new_arg_info);
        INFO_SSALIR_POSTASSIGN (arg_info) = INFO_SSALIR_EXTPOSTASSIGN (new_arg_info);

        FREE (new_arg_info);

        /*
         * if there are new preassigns to this function application
         * traverse them first, to infere loop invariant expressions
         * in this function to move this function application out of this
         * function
         */
        if (INFO_SSALIR_PREASSIGN (arg_info) != NULL) {
            INFO_SSALIR_PREASSIGN (arg_info)
              = Trav (INFO_SSALIR_PREASSIGN (arg_info), arg_info);
        }
    } else {
        /* no traversal into a normal fundef */
        DBUG_PRINT ("SSALIR", ("do not traverse in normal fundef %s",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));
    }

    /* traverse args of function application */
    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
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
 *   in loops look for move-down assignments and mark them
 *
 ******************************************************************************/
node *
SSALIRreturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRreturn");

    if (FUNDEF_STATUS (INFO_SSALIR_FUNDEF (arg_info)) == ST_dofun) {
        /* init INFO_SSALIR_APRESCHAIN with external result chain */
        DBUG_ASSERT ((FUNDEF_EXT_ASSIGNS (INFO_SSALIR_FUNDEF (arg_info)) != NULL),
                     "missing link to external calling fundef");
        DBUG_ASSERT ((NODELIST_NODE (FUNDEF_EXT_ASSIGNS (INFO_SSALIR_FUNDEF (arg_info)))
                      != NULL),
                     "missing link to external fundef calling assignment");

        INFO_SSALIR_APRESCHAIN (arg_info) = LET_IDS (ASSIGN_INSTR (
          NODELIST_NODE (FUNDEF_EXT_ASSIGNS (INFO_SSALIR_FUNDEF (arg_info)))));

        INFO_SSALIR_FLAG (arg_info) = SSALIR_INRETURN;

    } else {
        /* no special loop function */
        INFO_SSALIR_APRESCHAIN (arg_info) = NULL;
        INFO_SSALIR_FLAG (arg_info) = SSALIR_NORMAL;
    }

    /* traverse results */
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
 *   mark index vetor as local variable to allow a code moving
 *
 ******************************************************************************/
node *
SSALIRNwithid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRNwithid");

    /* traverse all local definitions to mark their depth in withloops */
    INFO_SSALIR_FLAG (arg_info) = SSALIR_MOVELOCAL;
    NWITHID_IDS (arg_node) = SSALIRleftids (NWITHID_IDS (arg_node), arg_info);
    NWITHID_VEC (arg_node) = SSALIRleftids (NWITHID_VEC (arg_node), arg_info);
    INFO_SSALIR_FLAG (arg_info) = SSALIR_NORMAL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALIRexprs(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses all exprs nodes.
 *   when used in return of a do-loop (with given INFO_SSALIR_APRECHAIN
 *   do a parallel result traversal
 *
 ******************************************************************************/
node *
SSALIRexprs (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRexprs");

    /* traverse expression */
    if (EXPRS_EXPR (arg_node) != NULL) {
        EXPRS_EXPR (arg_node) = Trav (EXPRS_EXPR (arg_node), arg_info);
    }

    if (EXPRS_NEXT (arg_node) != NULL) {
        if ((INFO_SSALIR_APRESCHAIN (arg_info) != NULL)
            && (INFO_SSALIR_FLAG (arg_info) == SSALIR_INRETURN)) {
            /* traverse the result ids chain of the external function application */
            INFO_SSALIR_APRESCHAIN (arg_info)
              = IDS_NEXT (INFO_SSALIR_APRESCHAIN (arg_info));
        }

        /* traverse next exprs node */
        EXPRS_NEXT (arg_node) = Trav (EXPRS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
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

    switch (INFO_SSALIR_FLAG (arg_info)) {
    case SSALIR_MOVEUP:
        DBUG_PRINT ("SSALIR", ("mark: moving up vardec %s",
                               VARDEC_NAME (AVIS_VARDECORARG (IDS_AVIS (arg_ids)))));
        AVIS_SSALPINV (IDS_AVIS (arg_ids)) = TRUE;

        (AVIS_LIRMOVE (IDS_AVIS (arg_ids))) |= LIRMOVE_UP;
        break;

    case SSALIR_MOVELOCAL:
        DBUG_PRINT ("SSALIR", ("mark: local vardec %s",
                               VARDEC_NAME (AVIS_VARDECORARG (IDS_AVIS (arg_ids)))));

        AVIS_LIRMOVE (IDS_AVIS (arg_ids)) = LIRMOVE_LOCAL;
        break;

    case SSALIR_NORMAL:
        AVIS_LIRMOVE (IDS_AVIS (arg_ids)) = LIRMOVE_NONE;
        break;

    default:
        DBUG_ASSERT ((FALSE), "unable to handle case");
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

    /* traverse to next expression */
    if (IDS_NEXT (arg_ids) != NULL) {
        IDS_NEXT (arg_ids) = SSALIRleftids (IDS_NEXT (arg_ids), arg_info);
    }

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   node* LIRMOVblock(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses body (not the vardecs)
 *
 ******************************************************************************/

node *
LIRMOVblock (node *arg_node, node *arg_info)
{
    int old_flag;

    DBUG_ENTER ("LIRMOVblock");

    /* save block mode */
    old_flag = INFO_SSALIR_TOPBLOCK (arg_info);

    if (FUNDEF_BODY (INFO_SSALIR_FUNDEF (arg_info)) == arg_node) {
        /* top block */
        INFO_SSALIR_TOPBLOCK (arg_info) = TRUE;
    } else {
        /* any other block */
        INFO_SSALIR_TOPBLOCK (arg_info) = FALSE;
    }

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    /* restore block mode */
    INFO_SSALIR_TOPBLOCK (arg_info) = old_flag;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* LIRMOVassign(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses the top-level assignment chain and moves marked assignments
 *   out of the loop. In all other assignment chains traverse the instruction.
 *   if an assignment is marked for move up AND move down we will move it up
 *   because it results in fewer additional arguments.
 *
 *****************************************************************************/
node *
LIRMOVassign (node *arg_node, node *arg_info)
{
    bool remove_assignment;
    node *tmp;
    LUT_t move_table;

    DBUG_ENTER ("LIRMOVassign");

    DBUG_ASSERT ((ASSIGN_INSTR (arg_node) != NULL), "missing instruction in assignment");

    if (INFO_SSALIR_TOPBLOCK (arg_info)) {
        if ((NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_let)
            && (LET_LIRFLAG (ASSIGN_INSTR (arg_node)) & LIRMOVE_UP)) {
            /* adjust identifier AND create new local variables in external fundef */
            INFO_SSALIR_FLAG (arg_info) = SSALIR_MOVEUP;

        } else if ((NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_let)
                   && (LET_LIRFLAG (ASSIGN_INSTR (arg_node)) == LIRMOVE_DOWN)) {
            /*
             * adjust identifier, create new local variables in external fundef
             * add movedown results to RESULTMAP nodelist
             */
            INFO_SSALIR_FLAG (arg_info) = SSALIR_MOVEDOWN;

        } else {
            /* only adjust identifiers */
            INFO_SSALIR_FLAG (arg_info) = SSALIR_NORMAL;
        }
    } else {
        /* we are in some subblock - do not change SSALIR_FLAG set in topblock !*/
    }

    /* traverse expression */
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    if ((INFO_SSALIR_FLAG (arg_info) == SSALIR_MOVEUP)
        && (INFO_SSALIR_TOPBLOCK (arg_info))) {
        /*
         * do the code movement via DupTree and LookUpTable
         * the look-up table contains all pairs of internal/external
         * vardecs, avis and strings of id's.
         * because duptree adds several nodes to the look-up table we have
         * to duplicate our move_table before we use it, to be shure to have
         * no wrong pointer of freed data in it.
         */

        move_table = DuplicateLUT (INFO_SSALIR_MOVELUT (arg_info));

        INFO_SSALIR_EXTPREASSIGN (arg_info)
          = AppendAssign (INFO_SSALIR_EXTPREASSIGN (arg_info),
                          DupNodeLUT (arg_node, move_table));

        move_table = RemoveLUT (move_table);

        /* one loop invarinat expression removed */
        lir_expr++;

        remove_assignment = TRUE;
    } else {
        remove_assignment = FALSE;
    }

    /* traverse to next assignment */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    /* remove this assignment */
    if (remove_assignment) {
        tmp = arg_node;
        arg_node = ASSIGN_NEXT (arg_node);
        FreeNode (tmp);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* LIRMOVlet(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses right side and left side in this order
 *
 ******************************************************************************/
node *
LIRMOVlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("LIRMOVlet");

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = LIRMOVleftids (LET_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* LIRMOVid(node *arg_node, node *arg_info)
 *
 * description:
 *   does the renaming according to the AVIS_SUBST setting
 *
 ******************************************************************************/
node *
LIRMOVid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("LIRMOVid");

    /*
     * do the necessary substitution, but only if this identifier stays
     * in this fundef. if it is moved up, we do not substitute, because
     * the moving duptree will adjust the references to the according external
     * vardec and avis nodes
     */
    if ((AVIS_SUBST (ID_AVIS (arg_node)) != NULL)
        && (INFO_SSALIR_FLAG (arg_info) != SSALIR_MOVEUP)) {
        DBUG_PRINT ("SSALIR", ("substitution: %s -> %s",
                               VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (ID_AVIS (arg_node))),
                               VARDEC_OR_ARG_NAME (
                                 AVIS_VARDECORARG (AVIS_SUBST (ID_AVIS (arg_node))))));

        /* do renaming to new ssa vardec */
        ID_AVIS (arg_node) = AVIS_SUBST (ID_AVIS (arg_node));

        /* restore all depended attributes with correct values */
        ID_VARDEC (arg_node) = AVIS_VARDECORARG (ID_AVIS (arg_node));

#ifndef NO_ID_NAME
        /* for compatiblity only
         * there is no real need for name string in ids structure because
         * you can get it from vardec without redundancy.
         */
        FREE (ID_NAME (arg_node));
        ID_NAME (arg_node) = StringCopy (VARDEC_OR_ARG_NAME (ID_VARDEC (arg_node)));
#endif
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* LIRMOVNwithid(node *arg_node, node *arg_info)
 *
 * description:
 *   the withid identifiers are local variables. if we move a with-loop
 *   out of a do-loop, we have to move this local variables, too. These
 *   variables are loop invariant, because the whole with-loop must be moved.
 *   so for each local variable we create a new one in the target context and
 *   add the necessary information to the LUT to have a correct transformation
 *   information for a later moving duptree call.
 *
 ******************************************************************************/
node *
LIRMOVNwithid (node *arg_node, node *arg_info)
{
    int old_flag;

    DBUG_ENTER ("LIRMOVNwithid");

    if ((INFO_SSALIR_FLAG (arg_info) == SSALIR_MOVEUP)
        || (INFO_SSALIR_FLAG (arg_info) == SSALIR_MOVEDOWN)) {
        if (NWITHID_VEC (arg_node) != NULL) {
            /* traverse identifier in move_local variable mode */
            old_flag = INFO_SSALIR_FLAG (arg_info);

            INFO_SSALIR_FLAG (arg_info) = SSALIR_MOVELOCAL;
            NWITHID_VEC (arg_node) = SSALIRleftids (NWITHID_VEC (arg_node), arg_info);
            NWITHID_IDS (arg_node) = SSALIRleftids (NWITHID_IDS (arg_node), arg_info);

            /* switch back to previous mode */
            INFO_SSALIR_FLAG (arg_info) = old_flag;
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* LIRMOVreturn(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
LIRMOVreturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("LIRMOVreturn");

    RETURN_EXPRS (arg_node) = Trav (RETURN_EXPRS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   ids* LIRMOVleftids(ids *arg_ids, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
static ids *
LIRMOVleftids (ids *arg_ids, node *arg_info)
{
    node *new_vardec;
    node *new_vardec_id;
    node *new_avis;
    char *new_name;
    node *new_arg;
    node *new_arg_id;
    nodelist *letlist;

    DBUG_ENTER ("LIRMOVleftids");

    if ((INFO_SSALIR_FLAG (arg_info) == SSALIR_MOVEUP)
        || (INFO_SSALIR_FLAG (arg_info) == SSALIR_MOVELOCAL)) {
        /*
         * create new vardec in ext fundef
         * set LUT information for later code movement
         */

        /* create new vardec */
        new_vardec = DupNode (IDS_VARDEC (arg_ids));
        new_avis = AdjustAvisData (new_vardec, INFO_SSALIR_EXTFUNDEF (arg_info));
        new_name = TmpVarName (VARDEC_NAME (new_vardec));
        FREE (VARDEC_NAME (new_vardec));
        VARDEC_NAME (new_vardec) = new_name;

        DBUG_PRINT ("SSALIR",
                    ("create external vardec %s for %s", new_name,
                     VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (IDS_AVIS (arg_ids)))));

        /* add vardec to chain of vardecs (ext. fundef) */
        BLOCK_VARDEC (FUNDEF_BODY (INFO_SSALIR_EXTFUNDEF (arg_info)))
          = AppendVardec (BLOCK_VARDEC (FUNDEF_BODY (INFO_SSALIR_EXTFUNDEF (arg_info))),
                          new_vardec);

        /*
         * setup LUT for later DupTree:
         *   subst local vardec with external vardec
         */
        /* vardec */
        INFO_SSALIR_MOVELUT (arg_info)
          = InsertIntoLUT_P (INFO_SSALIR_MOVELUT (arg_info), IDS_VARDEC (arg_ids),
                             new_vardec);

        /* avis */
        INFO_SSALIR_MOVELUT (arg_info) = InsertIntoLUT_P (INFO_SSALIR_MOVELUT (arg_info),
                                                          IDS_AVIS (arg_ids), new_avis);

        /* id name */
        INFO_SSALIR_MOVELUT (arg_info) = InsertIntoLUT_S (INFO_SSALIR_MOVELUT (arg_info),
                                                          IDS_NAME (arg_ids), new_name);

        /*
         *  modify functions signature:
         *    add an additional arg to fundef, ext. funap, int funap
         *    set renaming attributes for further processing
         */
        if ((INFO_SSALIR_FLAG (arg_info) == SSALIR_MOVEUP)
            && (INFO_SSALIR_TOPBLOCK (arg_info))) {
            /* make identifier for external function application */
            new_vardec_id = MakeId (StringCopy (new_name), NULL, ST_regular);
            ID_VARDEC (new_vardec_id) = new_vardec;
            ID_AVIS (new_vardec_id) = new_avis;

            /* make new arg for this functions (instead of vardec) */
            new_arg = MakeArgFromVardec (AVIS_VARDECORARG (IDS_AVIS (arg_ids)));
            FREE (ARG_NAME (new_arg));
            ARG_NAME (new_arg) = StringCopy (new_name);

            /* make identifier for recursive function call */
            new_arg_id = MakeId (StringCopy (new_name), NULL, ST_regular);
            ID_VARDEC (new_arg_id) = new_arg;
            ID_AVIS (new_arg_id) = ARG_AVIS (new_arg);

            /* change functions signature, internal and external application */
            DBUG_ASSERT ((FUNDEF_INT_ASSIGN (INFO_SSALIR_FUNDEF (arg_info)) != NULL),
                         "missing recursive call");
            DBUG_ASSERT ((FUNDEF_EXT_ASSIGNS (INFO_SSALIR_FUNDEF (arg_info)) != NULL),
                         "missing external call");
            DBUG_ASSERT ((NODELIST_NEXT (
                            FUNDEF_EXT_ASSIGNS (INFO_SSALIR_FUNDEF (arg_info)))
                          == NULL),
                         "more than one external call");

            /* recursive call */
            letlist = NodeListAppend (NULL,
                                      ASSIGN_INSTR (FUNDEF_INT_ASSIGN (
                                        INFO_SSALIR_FUNDEF (arg_info))),
                                      new_arg_id);
            letlist = NodeListAppend (letlist,
                                      ASSIGN_INSTR (NODELIST_NODE (FUNDEF_EXT_ASSIGNS (
                                        INFO_SSALIR_FUNDEF (arg_info)))),
                                      new_vardec_id);

            INFO_SSALIR_FUNDEF (arg_info)
              = CSAddArg (INFO_SSALIR_FUNDEF (arg_info), new_arg, letlist);

            /*
             * set renaming information: all old uses will be renamed to the new arg
             */
            AVIS_SUBST (IDS_AVIS (arg_ids)) = ARG_AVIS (new_arg);
        }
    }

    /* traverse next ids */
    if (IDS_NEXT (arg_ids) != NULL) {
        IDS_NEXT (arg_ids) = LIRMOVleftids (IDS_NEXT (arg_ids), arg_info);
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
