/*
 *
 * $Log$
 * Revision 1.14  2001/04/18 12:58:47  nmw
 * additional traversal setup function for single fundef traversal added
 *
 * Revision 1.13  2001/03/29 09:10:20  nmw
 * tabs2spaces done
 * y
 *
 * Revision 1.12  2001/03/26 13:26:14  nmw
 * SSANewVardec for general usage added
 *
 * Revision 1.11  2001/03/23 09:31:19  nmw
 * SSAwhile/do removed SSADummy added
 *
 * Revision 1.10  2001/03/22 20:01:41  dkr
 * include of tree.h eliminated
 *
 * Revision 1.9  2001/03/20 14:22:08  nmw
 * documentation added
 *
 * Revision 1.8  2001/03/19 14:23:02  nmw
 * handling for AVIS_ASSIGN2 attribute added
 *
 * Revision 1.7  2001/03/16 11:54:46  nmw
 * Storing of two definitions for phi targets implemented
 * AVIS_SSAPHITRAGET type changed
 *
 * Revision 1.6  2001/03/15 10:52:04  nmw
 * SSATransform now sets SSAUNDOFLAG for artificial vardecs
 *
 * Revision 1.5  2001/02/22 12:44:24  nmw
 * tranformation of multigenerator withloops added
 *
 * Revision 1.4  2001/02/20 15:53:29  nmw
 * code transformation in ssa form implemented
 *
 * Revision 1.3  2001/02/15 17:00:20  nmw
 * SSA form for flat assignments implemented
 *
 * Revision 1.2  2001/02/14 14:40:36  nmw
 * function bodies and traversal order implemented
 *
 * Revision 1.1  2001/02/13 15:16:15  nmw
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   SSATRansform.c
 *
 * prefix: SSA
 *
 * description:
 *    This module traverses the AST and transformes the code in SSA form.
 *    In ssa-form every identifier has exactly one defining assignment
 *    in the code. So redefinitions are not allowed and lead to a new
 *    identifier (here a renamed version of the original one extended
 *    by the postfix __SSA_<x>. Where x is an integer greater 0.
 *
 *    In performing a top-down traversal for each re-definition of some
 *    identifier there will be created a new, renamed identifier (e.g.
 *    i is renamend to i__SSA_1). The counter for this is stored in a
 *    SSACNT node for each variable name (stored as attribute in AVIS node).
 *    All SSACNT nodes are chained as attribute of the top block of the
 *    concerning function). The counter is used in sharing for all renamed
 *    instances of one original variable. So every additional renaming of
 *    a renamed identifier gets a fresh, unique name and it is still linked
 *    with the original name (which is also stored in SSACNT as BASID
 *    attribute).
 *    When doing the top-down traversal, the identifier are renamed to the
 *    actual instance that is defined at this point. In conditionals and
 *    with-loops the actual names are stacked in SSASTACK nodes.
 *
 *    To handle conditionals in ssaform, where the control flow merges and
 *    where are more than one definitions for an identifier, in ssa-form
 *    there are so called PHI-functions. These special functions are simulated
 *    by explicit copy assignments in both parts of the conditional, that
 *    have the same target (*). This leads to code not in (pure) ssa-form, but
 *    is is an compromiss between intruducing special functions that require
 *    a special handling in ALL working steps and preserve the ssa-form and
 *    the copy assignments which are the explicit version of an phi function
 *    that is ease to handle but destroys the pure ssa-form at one point.
 *
 *  Example: (usual form)       -->  (ssa-form)
 *    a = 6;                       a = 6;
 *    a = a + 4;                   a__SSA_1 = a + 4;
 *    b = a + a;                   b = a__SSA_1 + a__SSA_1;
 *    if (b < a) {                 if (b < a__SSA_1)
 *      a = 0;                       a__SSA_2 = 0;
 *      c = 1;                       c = 1;
 *                                   a__SSA_3 = a__SSA_2;     (*)
 *                                   c__SSA_2 = c;            (*)
 *    } else {                     } else {
 *      c = 2;                       c__SSA_1 = 2;
 *                                   a__SSA_3 = a__SSA_1;     (*)
 *                                   c__SSA_2 = c__SSA_1;     (*)
 *    }                            }
 *    print(c + b);                print(c__SSA_2 + b)
 *
 * Remarks:
 *    This module requires loops and conditionals in explicit functions.
 *    This transformation has to be done by lac2fun before calling
 *    SSATransform! SSATransform can be called again to preserve the ssa-form
 *    of the AST (e.g. after old code that cannot deal with the ssa-form).
 *    After using the ssa-form the code can be cleaned up by UndoSSATransform,
 *    that e.g. removes copy assignments and renamings of global objects.
 *
 *****************************************************************************/

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "dbug.h"
#include "globals.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "SSATransform.h"

#define TMP_STRING_LEN 16

/* INFO_SSA_CONDSTATUS */
#define CONDSTATUS_NOCOND 0
#define CONDSTATUS_THENPART 1
#define CONDSTATUS_ELSEPART 2

/* flag SSAstack operations */
#define STACKFLAG_DUMMY 0
#define STACKFLAG_THEN 1
#define STACKFLAG_ELSE 2

/* global helper functions */
node *SSANewVardec (node *old_vardec_or_arg);

/* helper functions for internal usage */
static void SSAInsertCopyAssignments (node *condassign, node *avis, node *arg_info);
static node *SSAGetSSAcount (char *baseid, int initvalue, node *block);

/* internal functions for traversing ids like nodes */
static ids *TravLeftIDS (ids *arg_ids, node *arg_info);
static ids *SSAleftids (ids *arg_ids, node *arg_info);
static ids *TravRightIDS (ids *arg_ids, node *arg_info);
static ids *SSArightids (ids *arg_ids, node *arg_info);

/* special push/pop operations for ONE ssastack */
static node *PopSSAstack (node *ssastack, node *avis, int flag);
static node *PushSSAstack (node *ssastack, node *avis, int flag);
static node *SaveTopSSAstack (node *ssastack, node *avis, int flag);

#define FOR_ALL_VARDEC_AND_ARGS(fun, flag, vardecchain, argchain)                        \
    {                                                                                    \
        node *vardec;                                                                    \
        node *arg;                                                                       \
                                                                                         \
        vardec = vardecchain;                                                            \
        while (vardec != NULL) {                                                         \
            AVIS_SSASTACK (VARDEC_AVIS (vardec))                                         \
              = fun (AVIS_SSASTACK (VARDEC_AVIS (vardec)), VARDEC_AVIS (vardec), flag);  \
            vardec = VARDEC_NEXT (vardec);                                               \
        }                                                                                \
        arg = argchain;                                                                  \
        while (arg != NULL) {                                                            \
            AVIS_SSASTACK (ARG_AVIS (arg))                                               \
              = fun (AVIS_SSASTACK (ARG_AVIS (arg)), ARG_AVIS (arg), flag);              \
            arg = ARG_NEXT (arg);                                                        \
        }                                                                                \
    }

/******************************************************************************
 *
 * function:
 *  static node *PopSSAstack(node *ssastack, node *avis, int flag)
 *
 * description:
 *  kills top of SSAstack
 *  if stack is not in use to nothing
 *
 ******************************************************************************/
static node *
PopSSAstack (node *ssastack, node *avis, int flag)
{
    node *rest;

    DBUG_ENTER ("PopSSAstack");

    if (SSASTACK_INUSE (ssastack)) {
        /* frees top of stack and returns rest */
        rest = SSASTACK_NEXT (ssastack);
        FreeNode (ssastack);
    } else {
        /* do nothing */
        rest = ssastack;
    }

    DBUG_RETURN (rest);
}

/******************************************************************************
 *
 * function:
 *  static  node *PushSSAstack(node *ssastack, node *avis, int flag)
 *
 * description:
 *  Duplicates value on top of stack.
 *
 ******************************************************************************/
static node *
PushSSAstack (node *ssastack, node *avis, int flag)
{
    DBUG_ENTER ("PushSSAstack");

    if (SSASTACK_INUSE (ssastack)) {
        ssastack = MakeSSAstack (ssastack, SSASTACK_AVIS (ssastack));
        SSASTACK_INUSE (ssastack) = TRUE;
    }

    DBUG_RETURN (ssastack);
}

/******************************************************************************
 *
 * function:
 *  static  node *SaveTopSSAstack(node *ssastack, node *avis, int flag)
 *
 * description:
 *  Saves Top-Value of stack to AVIS_SSATHEN  (flag=STACKFLAG_THEN)
 *  or                          AVIS_SSAELSE  (flag=STACLFLAG_ELSE)
 *
 ******************************************************************************/
static node *
SaveTopSSAstack (node *ssastack, node *avis, int flag)
{
    DBUG_ENTER ("SaveTopSSAstack");

    if (SSASTACK_INUSE (ssastack)) {
        switch (flag) {
        case STACKFLAG_THEN:
            AVIS_SSATHEN (avis) = SSASTACK_AVIS (ssastack);
            break;

        case STACKFLAG_ELSE:
            AVIS_SSAELSE (avis) = SSASTACK_AVIS (ssastack);
            break;

        default:
            DBUG_ASSERT ((FALSE), "unknown flag in SaveTopSSAstack");
        }
    }

    DBUG_RETURN (ssastack);
}

/******************************************************************************
 *
 * function:
 *   node *SSANewVardec(node *old_vardec_or_arg)
 *
 * description:
 *   creates a new (renamed) vardec of the given original vardec. the global
 *   ssa rename counter of the baseid is incremented. the ssacnt node is shared
 *   between all renamed instances of the original vardec.
 *
 ******************************************************************************/
node *
SSANewVardec (node *old_vardec_or_arg)
{
    char tmpstring[TMP_STRING_LEN];
    node *ssacnt;
    node *new_vardec;

    DBUG_ENTER ("SSANewVardec");

    ssacnt = AVIS_SSACOUNT (VARDEC_OR_ARG_AVIS (old_vardec_or_arg));

    if (NODE_TYPE (old_vardec_or_arg) == N_arg) {
        new_vardec = MakeVardecFromArg (old_vardec_or_arg);
    } else {
        new_vardec = DupNode (old_vardec_or_arg);
    }

    /* increment ssa renaming counter */
    SSACNT_COUNT (ssacnt) = SSACNT_COUNT (ssacnt) + 1;

    /* create new unique name */
    sprintf (tmpstring, "__SSA_%d", SSACNT_COUNT (ssacnt));

    FREE (VARDEC_NAME (new_vardec));
    VARDEC_NAME (new_vardec) = StringConcat (SSACNT_BASEID (ssacnt), tmpstring);

    DBUG_RETURN (new_vardec);
}

/******************************************************************************
 *
 * function:
 *  static void SSAInsertCopyAssignments(node *condassign, node *avis)
 *
 * description:
 *   creates a new renamed vardec as target for the ssa phi-function.
 *   inserts the necessary copy assignments for ssa in the conditional.
 *
 * remarks:
 *   the inserted target identifier (left_ids) has two definitions!
 *   (this is not really in ssa-form, but it simulates the ssa phi-function
 *   to merge different flows in execution).
 *   these two assignments are stored in AVIS_SSAASSIGN and AVIS_SSAASSIGN2.
 *   the AVIS node is marked as SSAPHITRAGET.
 *
 * if(p) {
 *   ...
 *   newvar = x__SSA_1;
 * } else {
 *   ...
 *   newvar = x__SSA_2;
 * }
 *
 ******************************************************************************/
static void
SSAInsertCopyAssignments (node *condassign, node *avis, node *arg_info)
{
    node *assign_let;
    node *right_id;
    ids *left_ids;

    DBUG_ENTER ("SSAInsertCopyAssignments");

    /* THEN part */
    /* create right side (id) of copy assignment for then part */
    right_id
      = MakeId (StringCopy (VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (AVIS_SSATHEN (avis)))),
                NULL, VARDEC_OR_ARG_STATUS (AVIS_VARDECORARG (AVIS_SSATHEN (avis))));
    ID_VARDEC (right_id) = AVIS_VARDECORARG (AVIS_SSATHEN (avis));
    ID_AVIS (right_id) = AVIS_SSATHEN (avis);

    /* create one let assign for then part */
    assign_let = MakeAssignLet (StringCopy (VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (avis))),
                                AVIS_VARDECORARG (avis), right_id);

    /* redefinition requieres new target variable (standard ssa mechanism) */
    LET_IDS (ASSIGN_INSTR (assign_let))
      = TravLeftIDS (LET_IDS (ASSIGN_INSTR (assign_let)), arg_info);
    left_ids = LET_IDS (ASSIGN_INSTR (assign_let));

    /* append new copy assignment to then-part block */
    BLOCK_INSTR (COND_THEN (condassign))
      = AppendAssign (BLOCK_INSTR (COND_THEN (condassign)), assign_let);
    /* store 1. definition assignment */
    AVIS_SSAASSIGN (IDS_AVIS (left_ids)) = assign_let;
    DBUG_PRINT ("SSA", ("insert phi copy assignment in then part: %s = %s",
                        VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (IDS_AVIS (left_ids))),
                        VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (ID_AVIS (right_id)))));
    right_id = NULL;
    assign_let = NULL;

    /* ELSE part */
    /* create right side (id) of copy assignment for else part */
    right_id
      = MakeId (StringCopy (VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (AVIS_SSAELSE (avis)))),
                NULL, VARDEC_OR_ARG_STATUS (AVIS_VARDECORARG (AVIS_SSAELSE (avis))));
    ID_VARDEC (right_id) = AVIS_VARDECORARG (AVIS_SSAELSE (avis));
    ID_AVIS (right_id) = AVIS_SSAELSE (avis);

    /* create let assign for else part with same left side as then part */
    assign_let = MakeAssignLet (StringCopy (VARDEC_OR_ARG_NAME (
                                  AVIS_VARDECORARG (IDS_AVIS (left_ids)))),
                                AVIS_VARDECORARG (IDS_AVIS (left_ids)), right_id);

    /* append new copy assignment to else-part block */
    BLOCK_INSTR (COND_ELSE (condassign))
      = AppendAssign (BLOCK_INSTR (COND_ELSE (condassign)), assign_let);
    /* store 2. definition assignment */
    AVIS_SSAASSIGN2 (IDS_AVIS (left_ids)) = assign_let;
    DBUG_PRINT ("SSA", ("insert phi copy assignment in else part: %s = %s",
                        VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (IDS_AVIS (left_ids))),
                        VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (ID_AVIS (right_id)))));

    /* mark vardec as special ssa phi target */
    switch (FUNDEF_STATUS (INFO_SSA_FUNDEF (arg_info))) {
    case ST_condfun:
        AVIS_SSAPHITARGET (IDS_AVIS (left_ids)) = PHIT_COND;
        break;

    case ST_whilefun:
        AVIS_SSAPHITARGET (IDS_AVIS (left_ids)) = PHIT_DO;
        break;

    case ST_dofun:
        AVIS_SSAPHITARGET (IDS_AVIS (left_ids)) = PHIT_WHILE;
        break;

    default:
        DBUG_ASSERT ((FALSE),
                     "conditional in reglular function! (no cond, do or while function)");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *  static node* SSAGetSSAcount(char *baseid, int initvalue, node *block)
 *
 * description:
 *   looks in list of available ssacounter (in block) for matching baseid
 *   and returns the corresponding ssacnt node.
 *   if search fails it will create a new ssacnt in list (counter will be
 *   initialized with initvalue)
 *
 ******************************************************************************/
static node *
SSAGetSSAcount (char *baseid, int initvalue, node *block)
{
    node *ssacnt;
    node *tmp;

    DBUG_ENTER ("SSAGetSSAcount");

    ssacnt = NULL;

    if (BLOCK_SSACOUNTER (block) != NULL) {
        /* look for existing ssacnt to this base_id */

        tmp = BLOCK_SSACOUNTER (block);
        do {
            if (strcmp (SSACNT_BASEID (tmp), baseid) == 0) {
                /* matching baseid */
                ssacnt = tmp;
            }

            tmp = SSACNT_NEXT (tmp);
        } while ((tmp != NULL) && (ssacnt == NULL));
    }

    if (ssacnt == NULL) {
        /* insert NEW ssa-counter to this baseid */
        ssacnt = MakeSSAcnt (BLOCK_SSACOUNTER (block), initvalue, StringCopy (baseid));

        /* add to list of ssacnt nodes */
        BLOCK_SSACOUNTER (block) = ssacnt;
    }

    DBUG_RETURN (ssacnt);
}

/******************************************************************************
 *
 * function:
 *  node *SSAfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses args and block of fundef in this order
 *
 ******************************************************************************/
node *
SSAfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSAfundef");

    /*
     * process only fundefs with body
     */
    if (FUNDEF_BODY (arg_node) != NULL) {
        /* stores access points for later insertions in this fundef */
        INFO_SSA_FUNDEF (arg_info) = arg_node;
        INFO_SSA_BLOCK (arg_info) = FUNDEF_BODY (arg_node);

        if (FUNDEF_ARGS (arg_node) != NULL) {
            /* there are some args */
            FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
        }

        if (FUNDEF_BODY (arg_node) != NULL) {
            /* there is a block */
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
        }
    }

    /* traverse next fundef */
    if ((INFO_SSA_SINGLEFUNDEF (arg_info) == FALSE) && (FUNDEF_NEXT (arg_node) != NULL)) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSAblock(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses vardecs and instructions in this order, subblocks do not have
 *   any vardecs.
 *
 ******************************************************************************/
node *
SSAblock (node *arg_node, node *arg_info)
{
    node *old_assign;

    DBUG_ENTER ("SSAblock");

    /* save old assignment link when starting with new block */
    old_assign = INFO_SSA_ASSIGN (arg_info);

    if (BLOCK_VARDEC (arg_node) != NULL) {
        /* there are some vardecs */
        DBUG_ASSERT ((arg_node == INFO_SSA_BLOCK (arg_info)),
                     "subblock contains vardecs");
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }

    if (BLOCK_INSTR (arg_node) != NULL) {
        /* there are some instructions */
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    /* restore old assignment link */
    INFO_SSA_ASSIGN (arg_info) = old_assign;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSAexprs(node *arg_node, node *arg_info)
 *
 * description:
 *  traverses all exprs nodes.
 *  when used in N_return subtree
 *
 ******************************************************************************/
node *
SSAexprs (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSAexprs");
    /* traverse expr */
    DBUG_ASSERT ((EXPRS_EXPR (arg_node) != NULL), "no expression in exprs node!");
    EXPRS_EXPR (arg_node) = Trav (EXPRS_EXPR (arg_node), arg_info);

    /* traverse to next exprs */
    if (EXPRS_NEXT (arg_node) != NULL) {
        EXPRS_NEXT (arg_node) = Trav (EXPRS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSAassign(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses assign chain top down.
 *
 ******************************************************************************/
node *
SSAassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSAassign");
    /* traverse expr */
    DBUG_ASSERT ((ASSIGN_INSTR (arg_node) != NULL), "no instruction in assign!");

    INFO_SSA_ASSIGN (arg_info) = arg_node;

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    /* traverse next exprs */
    if (EXPRS_NEXT (arg_node) != NULL) {
        EXPRS_NEXT (arg_node) = Trav (EXPRS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSAlet(node *arg_node, node *arg_info)
 *
 * description:
 *   travereses in expression and assigned ids.
 *
 ******************************************************************************/
node *
SSAlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSAlet");

    DBUG_ASSERT ((LET_EXPR (arg_node) != NULL), "N_let with empty EXPR attribute.");
    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    if (LET_IDS (arg_node) != NULL) {
        /* there are some ids not in a special ssa copy let */
        LET_IDS (arg_node) = TravLeftIDS (LET_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSAarg(node *arg_node, node *arg_info)
 *
 * description:
 *   check for missing SSACOUT attribute in AVIS node. installs and inits
 *   new ssa-counter if necessary (init with 0, means unrenamed argument)
 *
 ******************************************************************************/
node *
SSAarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSAarg");

    DBUG_PRINT ("SSA", ("working on arg %s", ARG_NAME (arg_node)));

    if (AVIS_SSACOUNT (ARG_AVIS (arg_node)) == NULL) {
        /* insert ssa-counter to this baseid */
        AVIS_SSACOUNT (ARG_AVIS (arg_node))
          = SSAGetSSAcount (ARG_NAME (arg_node), 0, INFO_SSA_BLOCK (arg_info));
    }

    /* actual rename-to target on stack*/
    AVIS_SSASTACK_TOP (ARG_AVIS (arg_node)) = ARG_AVIS (arg_node);
    AVIS_SSADEFINED (ARG_AVIS (arg_node)) = TRUE;

    /*
     * mark stack as activ
     * (later added vardecs and stacks are ignored when stacking)
     */
    AVIS_SSASTACK_INUSE (ARG_AVIS (arg_node)) = TRUE;

    /* clear all traversal infos in avis node */
    AVIS_SSATHEN (ARG_AVIS (arg_node)) = NULL;
    AVIS_SSAELSE (ARG_AVIS (arg_node)) = NULL;

    /* no direct assignment available (yet) */
    AVIS_SSAASSIGN (ARG_AVIS (arg_node)) = NULL;

    /* traverse next arg */
    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSAvardec(node *arg_node, node *arg_info)
 *
 * description:
 *   check for missing SSACOUNT attribute in AVIS node. installs and inits
 *   new ssa-counter if necessary (init with undef)
 *
 ******************************************************************************/
node *
SSAvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSAvardec");

    if (AVIS_SSACOUNT (VARDEC_AVIS (arg_node)) == NULL) {
        /* insert ssa-counter to this baseid */
        AVIS_SSACOUNT (VARDEC_AVIS (arg_node))
          = SSAGetSSAcount (VARDEC_NAME (arg_node), 0, INFO_SSA_BLOCK (arg_info));
    }

    /* jet undefined on stack */
    AVIS_SSASTACK_TOP (VARDEC_AVIS (arg_node)) = NULL;
    AVIS_SSADEFINED (VARDEC_AVIS (arg_node)) = FALSE;

    /*
     * mark stack as activ
     * (later added vardecs and stacks are ignored when stacking)
     */
    AVIS_SSASTACK_INUSE (VARDEC_AVIS (arg_node)) = TRUE;

    /* clear all traversal infos in avis node */
    AVIS_SSATHEN (VARDEC_AVIS (arg_node)) = NULL;
    AVIS_SSAELSE (VARDEC_AVIS (arg_node)) = NULL;

    /* traverse next vardec */
    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSAid(node *arg_node, node *arg_info)
 *
 * description:
 *  does necessary renaming of variables used on right sides of expressions.
 *
 ******************************************************************************/
node *
SSAid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSAid");

    DBUG_ASSERT ((ID_IDS (arg_node) != NULL), "missing IDS in N_id!");

    ID_IDS (arg_node) = TravRightIDS (ID_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSAap(node *arg_node, node *arg_info)
 *
 * description:
 *  traverses args and does a recursive call in case of special function
 *  applications.
 *
 ******************************************************************************/
node *
SSAap (node *arg_node, node *arg_info)
{
    node *new_arg_info;

    DBUG_ENTER ("SSAap");

    DBUG_ASSERT ((AP_FUNDEF (arg_node) != NULL), "missing fundef in ap-node");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }

    /* traverse special fundef without recursion (only in single fundef mode) */
    if ((FUNDEF_IS_LACFUN (AP_FUNDEF (arg_node)))
        && (INFO_SSA_SINGLEFUNDEF (arg_info) == TRUE)
        && (AP_FUNDEF (arg_node) != INFO_SSA_FUNDEF (arg_info))) {
        DBUG_PRINT ("SSA", ("traverse in special fundef %s",
                            FUNDEF_NAME (AP_FUNDEF (arg_node))));

        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        INFO_SSA_SINGLEFUNDEF (new_arg_info) = INFO_SSA_SINGLEFUNDEF (arg_info);

        /* start traversal of special fundef */
        AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), new_arg_info);

        DBUG_PRINT ("SSA", ("traversal of special fundef %s finished\n",
                            FUNDEF_NAME (AP_FUNDEF (arg_node))));
        FREE (new_arg_info);

    } else {
        DBUG_PRINT ("SSA", ("do not traverse in normal fundef %s",
                            FUNDEF_NAME (AP_FUNDEF (arg_node))));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSANwith(node *arg_node, node *arg_info)
 *
 * description:
 *  traverses with-op, partitions and code in this order
 *
 ******************************************************************************/
node *
SSANwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSANwith");

    /* traverse in with-op */
    DBUG_ASSERT ((NWITH_WITHOP (arg_node) != NULL), "Nwith without WITHOP node!");
    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

    /*
     * before traversing all partions, clear with ids/vec to check all
     * partions for identical index variables
     */
    INFO_SSA_WITHIDS (arg_info) = NULL;
    INFO_SSA_WITHVEC (arg_info) = NULL;

    /* traverse partitions */
    DBUG_ASSERT ((NWITH_PART (arg_node) != NULL), "Nwith without PART node!");
    NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);

    /* do stacking of current renaming status */
    FOR_ALL_VARDEC_AND_ARGS (PushSSAstack, STACKFLAG_DUMMY,
                             BLOCK_VARDEC (INFO_SSA_BLOCK (arg_info)),
                             FUNDEF_ARGS (INFO_SSA_FUNDEF (arg_info)));

    /* traverse code */
    DBUG_ASSERT ((NWITH_CODE (arg_node) != NULL), "Nwith without CODE node!");
    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

    /* pop old renaming status from stack */
    FOR_ALL_VARDEC_AND_ARGS (PopSSAstack, STACKFLAG_DUMMY,
                             BLOCK_VARDEC (INFO_SSA_BLOCK (arg_info)),
                             FUNDEF_ARGS (INFO_SSA_FUNDEF (arg_info)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSANpart(node *arg_node, node *arg_info)
 *
 * description:
 *  traverses generator in this order for all partitions and last
 * (only one time!) withid
 *
 ******************************************************************************/
node *
SSANpart (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSANpart");

    /* traverse generator */
    DBUG_ASSERT ((NPART_GEN (arg_node) != NULL), "Npart without Ngen node!");
    NPART_GEN (arg_node) = Trav (NPART_GEN (arg_node), arg_info);

    DBUG_ASSERT ((NPART_WITHID (arg_node) != NULL), "Npart without Nwithid node!");

    /* store first occurrence of withids and withvec for later ASSERT */
    if (INFO_SSA_WITHIDS (arg_info) == NULL) {
        INFO_SSA_WITHIDS (arg_info) = IDS_AVIS (NWITHID_IDS (NPART_WITHID (arg_node)));
    }
    if (INFO_SSA_WITHVEC (arg_info) == NULL) {
        INFO_SSA_WITHVEC (arg_info) = IDS_AVIS (NWITHID_VEC (NPART_WITHID (arg_node)));
    }

    /* assert unique withids/withvec !!! */
    DBUG_ASSERT ((INFO_SSA_WITHIDS (arg_info)
                  == IDS_AVIS (NWITHID_IDS (NPART_WITHID (arg_node)))),
                 "multigenerator withloop with inconsistent withids");
    DBUG_ASSERT ((INFO_SSA_WITHVEC (arg_info)
                  == IDS_AVIS (NWITHID_VEC (NPART_WITHID (arg_node)))),
                 "multigenerator withloop with inconsistent withvec");

    /* assert unique withids !!! */
    DBUG_ASSERT ((INFO_SSA_WITHIDS (arg_info)), "no withids found");
    DBUG_ASSERT ((INFO_SSA_WITHVEC (arg_info)), "no withvec found");

    if (NPART_NEXT (arg_node) != NULL) {
        /* traverse next part */
        NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
    } else {
        /* traverse in withid */
        NPART_WITHID (arg_node) = Trav (NPART_WITHID (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSANcode(node *arg_node, node *arg_info)
 *
 * description:
 *  traverses block and expr in this order. then next Ncode node
 *
 ******************************************************************************/
node *
SSANcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSANcode");

    /* do stacking of current renaming status */
    FOR_ALL_VARDEC_AND_ARGS (PushSSAstack, STACKFLAG_DUMMY,
                             BLOCK_VARDEC (INFO_SSA_BLOCK (arg_info)),
                             FUNDEF_ARGS (INFO_SSA_FUNDEF (arg_info)));

    /* traverse block */
    if (NCODE_CBLOCK (arg_node) != NULL) {
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    }

    /* traverse expression */
    DBUG_ASSERT ((NCODE_CEXPR (arg_node) != NULL), "Ncode without Ncexpr node!");
    NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);

    /* restore old rename stack !!! */
    FOR_ALL_VARDEC_AND_ARGS (PopSSAstack, STACKFLAG_DUMMY,
                             BLOCK_VARDEC (INFO_SSA_BLOCK (arg_info)),
                             FUNDEF_ARGS (INFO_SSA_FUNDEF (arg_info)));

    if (NCODE_NEXT (arg_node) != NULL) {
        /* traverse next part */
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
/******************************************************************************
 *
 * function:
 *  node *SSANwithid(node *arg_node, node *arg_info)
 *
 * description:
 *  traverses in vector and ids strutures
 *
 ******************************************************************************/
node *
SSANwithid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSANwithid");

    DBUG_ASSERT ((NWITHID_VEC (arg_node) != NULL),
                 "NWITHID node with empty VEC attribute");
    NWITHID_VEC (arg_node) = TravLeftIDS (NWITHID_VEC (arg_node), arg_info);

    DBUG_ASSERT ((NWITHID_IDS (arg_node) != NULL),
                 "NWITHID node with empty IDS attribute");
    NWITHID_IDS (arg_node) = TravLeftIDS (NWITHID_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSAcond(node *arg_node, node *arg_info)
 *
 * description:
 *   this top-level conditional requieres stacking of renaming status.
 *   traverses conditional, then and else branch in this order.
 *
 ******************************************************************************/
node *
SSAcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSAcond");

    /* save this cond_node for later insertions of copy assignments */
    INFO_SSA_CONDSTMT (arg_info) = arg_node;

    /* traverse conditional */
    DBUG_ASSERT ((COND_COND (arg_node) != NULL), "Ncond without cond node!");
    COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);

    /* do stacking of current renaming status */
    FOR_ALL_VARDEC_AND_ARGS (PushSSAstack, STACKFLAG_DUMMY,
                             BLOCK_VARDEC (INFO_SSA_BLOCK (arg_info)),
                             FUNDEF_ARGS (INFO_SSA_FUNDEF (arg_info)));

    /* traverse then */
    INFO_SSA_CONDSTATUS (arg_info) = CONDSTATUS_THENPART;
    if (COND_THEN (arg_node) != NULL) {
        COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);
    }

    /* save to then for later merging */
    FOR_ALL_VARDEC_AND_ARGS (SaveTopSSAstack, STACKFLAG_THEN,
                             BLOCK_VARDEC (INFO_SSA_BLOCK (arg_info)),
                             FUNDEF_ARGS (INFO_SSA_FUNDEF (arg_info)));

    /* so some status restauration */
    FOR_ALL_VARDEC_AND_ARGS (PopSSAstack, STACKFLAG_DUMMY,
                             BLOCK_VARDEC (INFO_SSA_BLOCK (arg_info)),
                             FUNDEF_ARGS (INFO_SSA_FUNDEF (arg_info)));

    /* do stacking of current renaming status */
    FOR_ALL_VARDEC_AND_ARGS (PushSSAstack, STACKFLAG_DUMMY,
                             BLOCK_VARDEC (INFO_SSA_BLOCK (arg_info)),
                             FUNDEF_ARGS (INFO_SSA_FUNDEF (arg_info)));

    /* traverse else */
    INFO_SSA_CONDSTATUS (arg_info) = CONDSTATUS_ELSEPART;
    if (COND_ELSE (arg_node) != NULL) {
        COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);
    }

    /* save to else for later merging */
    FOR_ALL_VARDEC_AND_ARGS (SaveTopSSAstack, STACKFLAG_ELSE,
                             BLOCK_VARDEC (INFO_SSA_BLOCK (arg_info)),
                             FUNDEF_ARGS (INFO_SSA_FUNDEF (arg_info)));

    /* so some status restauration */
    FOR_ALL_VARDEC_AND_ARGS (PopSSAstack, STACKFLAG_DUMMY,
                             BLOCK_VARDEC (INFO_SSA_BLOCK (arg_info)),
                             FUNDEF_ARGS (INFO_SSA_FUNDEF (arg_info)));

    INFO_SSA_CONDSTATUS (arg_info) = CONDSTATUS_NOCOND;
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSAreturn(node *arg_node, node *arg_info)
 *
 * description:
 *   checks returning variables for different definitions on cond then and else
 *   branch. traverses exprs nodes
 *
 ******************************************************************************/
node *
SSAreturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSAreturn");

    /*
     * set flag in Ninfo node
     * when processing following N_id nodes, do the phi-merging
     */
    INFO_SSA_RETINSTR (arg_info) = TRUE;

    /* traverse exprs */
    if (RETURN_EXPRS (arg_node) != NULL) {
        RETURN_EXPRS (arg_node) = Trav (RETURN_EXPRS (arg_node), arg_info);
    }

    INFO_SSA_RETINSTR (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSAleftids(ids *arg_ids, node *arg_info)
 *
 * description:
 *  creates new (renamed) instance of defined variable.
 *
 ******************************************************************************/
static ids *
SSAleftids (ids *arg_ids, node *arg_info)
{
    node *new_vardec;

    DBUG_ENTER ("SSAleftids");

    if (AVIS_SSAPHITARGET (IDS_AVIS (arg_ids)) != PHIT_NONE) {
        /* special ssa phi target - no renaming needed */
        AVIS_SSASTACK_TOP (IDS_AVIS (arg_ids)) = IDS_AVIS (arg_ids);
        DBUG_PRINT ("SSA", ("phi target, no renaming: %s (%p)",
                            VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (IDS_AVIS (arg_ids))),
                            IDS_AVIS (arg_ids)));

    } else if (AVIS_SSADEFINED (IDS_AVIS (arg_ids)) == FALSE) {
        /* first definition of variable (no renaming) */
        AVIS_SSASTACK_TOP (IDS_AVIS (arg_ids)) = IDS_AVIS (arg_ids);
        /* SSACNT_COUNT(AVIS_SSACOUNT(IDS_AVIS(arg_ids))) = 0; */
        AVIS_SSADEFINED (IDS_AVIS (arg_ids)) = TRUE;
        DBUG_PRINT ("SSA", ("first definition, no renaming: %s (%p)",
                            VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (IDS_AVIS (arg_ids))),
                            IDS_AVIS (arg_ids)));
        AVIS_SSAASSIGN (IDS_AVIS (arg_ids)) = INFO_SSA_ASSIGN (arg_info);

    } else {
        /* redefinition - create new unique variable/vardec */
        new_vardec = SSANewVardec (AVIS_VARDECORARG (IDS_AVIS (arg_ids)));
        BLOCK_VARDEC (INFO_SSA_BLOCK (arg_info))
          = AppendVardec (BLOCK_VARDEC (INFO_SSA_BLOCK (arg_info)), new_vardec);
        DBUG_PRINT ("SSA", ("re-definition, renaming: %s (%p) -> %s",
                            VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (IDS_AVIS (arg_ids))),
                            IDS_AVIS (arg_ids), VARDEC_NAME (new_vardec)));

        /* new rename-to target for old vardec */
        AVIS_SSASTACK_TOP (IDS_AVIS (arg_ids)) = VARDEC_AVIS (new_vardec);

        /* rename this ids */
        IDS_VARDEC (arg_ids) = new_vardec;
        IDS_AVIS (arg_ids) = VARDEC_AVIS (new_vardec);

#ifndef NO_ID_NAME
        /* for compatiblity only
         * there is no real need for name string in ids structure because
         * you can get it from vardec without redundancy.
         */
        FREE (IDS_NAME (arg_ids));
        IDS_NAME (arg_ids) = StringCopy (VARDEC_NAME (new_vardec));
#endif

        /*
         * mark this avis for undo ssa transform:
         * all global objects and artificial identifier must
         * be mapped back to their original name in undossa.
         *
         */
        if ((IDS_STATUS (arg_ids) == ST_artificial) || (IDS_ATTRIB (arg_ids) == ST_global)
            || (VARDEC_OR_ARG_ATTRIB (AVIS_VARDECORARG (IDS_AVIS (arg_ids)))
                == ST_unique)) {
            AVIS_SSAUNDOFLAG (IDS_AVIS (arg_ids)) = TRUE;
        }
        AVIS_SSAASSIGN (IDS_AVIS (arg_ids)) = INFO_SSA_ASSIGN (arg_info);
    }

    /* traverese next ids */
    if (IDS_NEXT (arg_ids) != NULL) {
        IDS_NEXT (arg_ids) = TravLeftIDS (IDS_NEXT (arg_ids), arg_info);
    }
    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *  node *SSArightids(ids *arg_ids, node *arg_info)
 *
 * description:
 *   renames variable to actual ssa renaming counter.
 *
 ******************************************************************************/
static ids *
SSArightids (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("SSArightids");

    /*
     * if ids is used in return instruction it has to be checked
     * for needed copy assignments in a conditional
     */
    if (INFO_SSA_RETINSTR (arg_info)) {
        /* check for different assignments in then and else part */
        if (AVIS_SSATHEN (IDS_AVIS (arg_ids)) != AVIS_SSAELSE (IDS_AVIS (arg_ids))) {
            DBUG_ASSERT ((AVIS_SSATHEN (IDS_AVIS (arg_ids))),
                         "undefined variable in then part");
            DBUG_ASSERT ((AVIS_SSATHEN (IDS_AVIS (arg_ids))),
                         "undefined variable in then part");
            SSAInsertCopyAssignments (INFO_SSA_CONDSTMT (arg_info), IDS_AVIS (arg_ids),
                                      arg_info);
        }
    }

    /*
     * existing phi copy target must not be renamed
     */
    if (AVIS_SSAPHITARGET (IDS_AVIS (arg_ids)) == PHIT_NONE) {
        /* do renaming to new ssa vardec */
        IDS_AVIS (arg_ids) = AVIS_SSASTACK_TOP (IDS_AVIS (arg_ids));
    }

    /* restore all depended attributes with correct values */
    IDS_VARDEC (arg_ids) = AVIS_VARDECORARG (IDS_AVIS (arg_ids));

#ifndef NO_ID_NAME
    /* for compatiblity only
     * there is no real need for name string in ids structure because
     * you can get it from vardec without redundancy.
     */
    FREE (IDS_NAME (arg_ids));
    IDS_NAME (arg_ids) = StringCopy (VARDEC_OR_ARG_NAME (IDS_VARDEC (arg_ids)));
#endif

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   ids *Trav[Left,Right]IDS(ids *arg_ids, node *arg_info)
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
    arg_ids = SSAleftids (arg_ids, arg_info);

    DBUG_RETURN (arg_ids);
}

static ids *
TravRightIDS (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("TravRightIDS");

    DBUG_ASSERT (arg_ids != NULL, "traversal in NULL ids");
    arg_ids = SSArightids (arg_ids, arg_info);

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *  node *SSADummy(node *arg_node, node *arg_info)
 *
 * description:
 *   after lac2fun no more do/while node are allowed in the ast!
 *   this dummy dunctions is called for do and while nodes in SSA traversals.
 *
 ******************************************************************************/
node *
SSADummy (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSADummy");

    DBUG_ASSERT ((FALSE), "no do- and while-loops allowed in ssa form!");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSATransform(node *syntax_tree)
 *
 * description:
 *   Starts traversal of AST to transform code in SSA form. Every variable
 *   has exaclty one definition. This code transformtion relies on the
 *   lac2fun transformation!
 *
 ******************************************************************************/
node *
SSATransform (node *syntax_tree)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("SSATransform");

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) == N_modul),
                 "SSATransform is used for module nodes only");

    DBUG_PRINT ("OPT", ("starting ssa transformation for ast"));

    arg_info = MakeInfo ();
    INFO_SSA_SINGLEFUNDEF (arg_info) = FALSE;

    old_tab = act_tab;
    act_tab = ssafrm_tab;

    syntax_tree = Trav (syntax_tree, arg_info);

    act_tab = old_tab;

    FREE (arg_info);

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * function:
 *   node *SSATransformSingleFundef(node *fundef)
 *
 * description:
 *   same as SSATransform, but traverses only the given single fundef. this
 *   improves performance, when updating ssaform in the optimization cycle.
 *   it does NOT traverse special fundefs (they will be traverses in order
 *   of their usage)
 *
 ******************************************************************************/
node *
SSATransformSingleFundef (node *fundef)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("SSATransformSingleFundef");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "SSATransformSingleFundef is used for fundef nodes only");

    if (!(FUNDEF_IS_LACFUN (fundef))) {
        DBUG_PRINT ("OPT", ("starting ssa transformation for %s", FUNDEF_NAME (fundef)));

        arg_info = MakeInfo ();
        INFO_SSA_SINGLEFUNDEF (arg_info) = TRUE;

        old_tab = act_tab;
        act_tab = ssafrm_tab;

        fundef = Trav (fundef, arg_info);

        act_tab = old_tab;

        FREE (arg_info);
    }

    DBUG_RETURN (fundef);
}
