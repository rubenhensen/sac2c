/*
 * $Log$
 * Revision 1.10  2004/07/14 15:20:57  ktr
 * WITHID is treated as RHS if the variables were allocated using alloc, too.
 *
 * Revision 1.9  2004/06/21 19:01:15  mwe
 * check compiler phase before creating new types (create no types before PH_typecheck)
 *
 * Revision 1.8  2004/06/10 14:46:53  mwe
 * after usage of SSANewVardec a ntype added to new avis node
 *
 * Revision 1.7  2004/06/08 14:30:46  ktr
 * WithIDs are treated as RHS-expressions iff the Index vector has been
 * allocated using F_alloc_or_reuse before.
 *
 * Revision 1.6  2004/05/12 12:59:40  ktr
 * Code for NCODE_EPILOGUE added
 *
 * Revision 1.5  2004/05/11 13:25:21  khf
 * NCODE_CEXPR in SSANcode() replaced by NCODE_CEXPRS
 *
 * Revision 1.4  2004/03/05 19:14:27  mwe
 * representation of conditional changed
 * using N_funcond node instead of phi
 *
 * Revision 1.3  2004/02/25 08:22:32  cg
 * Elimination of while-loops by conversion into do-loops with
 * leading conditional integrated into flatten.
 * Separate compiler phase while2do eliminated.
 *
 * Revision 1.2  2004/02/06 14:19:33  mwe
 * replace usage of PHITARGET with primitive phi function
 *
 * Revision 1.1  2004/01/28 16:53:48  skt
 * Initial revision
 *
 *
 *
 *
 *
 ************ Attention! ************
 * File was moved from ../tree
 * following older Revisions can be found there
 *
 *
 * Revision 1.30  2003/09/16 18:19:11  ktr
 * Added support for AVIS_WITHID
 *
 * Revision 1.29  2003/06/17 20:27:47  sbs
 * during or prior to TC SSATransform should not generate any types for vardecs
 * => modified SSAMakeVardec accordingly.
 *
 * Revision 1.28  2003/03/12 21:18:30  dkr
 * SSAicm() added
 *
 * Revision 1.27  2003/03/08 17:45:39  dkr
 * SSAInsertCopyAssignments(): IS_REFERENCE is set correctly now
 *
 * Revision 1.26  2002/10/18 13:50:20  sbs
 * SSATransformAllowGOs implemented
 * FLAGS for freshly made N_id nodes set correctly
 *
 * Revision 1.25  2002/10/09 02:09:55  dkr
 * SSANpart(): transformation of multi-generator WLs corrected
 * (calls of SSArightids() added!)
 *
 * Revision 1.24  2002/08/15 10:12:25  sbs
 * Now, SSArightids produces an error message whenever a non previously
 * defined variable i encountered.
 *
 * Revision 1.23  2002/08/05 09:52:04  sbs
 * eliminated the requirement for Nwithid nodes to always have both,
 * an IDS and a VEC attribute. This change is motivated by the requirement
 * to convert to SSA form prior to type checking. Furthermore, not being
 * restricted to the AKS case anymore, we cannot guarantee the existance
 * of the IDS attribute in all cases anymore !!!!
 *
 * Revision 1.21  2001/05/17 11:38:56  dkr
 * MALLOC FREE aliminated
 *
 * Revision 1.20  2001/05/15 15:50:41  nmw
 * little macro bug fixed
 *
 * Revision 1.19  2001/05/09 12:30:43  nmw
 * when creating ssaform set global valid_ssaform flag
 *
 * Revision 1.18  2001/05/04 11:55:50  nmw
 * added support for correct handling of AVIS_SSAASSIGN for withids
 *
 * Revision 1.17  2001/05/03 16:51:34  nmw
 * set correct SSAASSIGN attributes when updating phi_targets
 *
 * Revision 1.16  2001/04/24 16:09:07  nmw
 * SSATransformSingleFundef renamed to SSATransformOneFunction
 *
 * Revision 1.15  2001/04/19 08:03:22  dkr
 * macro F_PTR used as format string for pointers
 *
 * Revision 1.14  2001/04/18 12:58:47  nmw
 * additional traversal setup function for single fundef traversal added
 *
 * Revision 1.13  2001/03/29 09:10:20  nmw
 * tabs2spaces done
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
 *    i is renamed to i__SSA_1). The counter for this is stored in a
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

/* INFO_SSA_SINGLEFUNDEF */
#define SSA_TRAV_FUNDEFS 0
#define SSA_TRAV_SPECIALS 1
#define SSA_TRAV_NONE 2

/* INFO_SSA_CONDSTATUS */
#define CONDSTATUS_NOCOND 0
#define CONDSTATUS_THENPART 1
#define CONDSTATUS_ELSEPART 2

/* flag SSAstack operations */
#define STACKFLAG_DUMMY 0
#define STACKFLAG_THEN 1
#define STACKFLAG_ELSE 2

#define INFO_SSA_ALLOW_GOS(n) ((bool)(n->refcnt))

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
        if (compiler_phase <= PH_typecheck) {
            /**
             * we are running SSATransform prior or during TC! Therefore,
             * the type has to be virginized, i.e., set to _unknown_ !
             */
            VARDEC_TYPE (new_vardec) = FreeAllTypes (VARDEC_TYPE (new_vardec));
            VARDEC_TYPE (new_vardec) = MakeTypes1 (T_unknown);
            DBUG_PRINT ("SBS", ("POOP"));
        }
    } else {
        new_vardec = DupNode (old_vardec_or_arg);
    }

    /* increment ssa renaming counter */
    SSACNT_COUNT (ssacnt) = SSACNT_COUNT (ssacnt) + 1;

    /* create new unique name */
    sprintf (tmpstring, "__SSA_%d", SSACNT_COUNT (ssacnt));

    Free (VARDEC_NAME (new_vardec));
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
 *   inserts the necessary copy assignments for ssa behind the conditional.
 *
 * remarks:
 *   the inserted target identifier (left_ids) is a phi function with
 *   two arguments!
 *
 * if(p) {
 *   ...
 *   x__SSA_1 = ...;
 *   ...
 * } else {
 *   ...
 *   x__SSA_2 = ...;
 *   ...
 * }
 * newvar = phi(x__SSA_1, x__SSA_2);
 *
 ******************************************************************************/
static void
SSAInsertCopyAssignments (node *condassign, node *avis, node *arg_info)
{
    node *assign_let;
    node *right_id1, *right_id2;
    ids *left_ids;

    DBUG_ENTER ("SSAInsertCopyAssignments");

    /* THEN part */
    /* create right side (id) of copy assignment for then part */
    right_id1
      = MakeId (StringCopy (VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (AVIS_SSATHEN (avis)))),
                NULL, VARDEC_OR_ARG_STATUS (AVIS_VARDECORARG (AVIS_SSATHEN (avis))));
    ID_VARDEC (right_id1) = AVIS_VARDECORARG (AVIS_SSATHEN (avis));
    SET_FLAG (ID, right_id1, IS_GLOBAL, (NODE_TYPE (ID_VARDEC (right_id1)) == N_objdef));
    SET_FLAG (ID, right_id1, IS_REFERENCE, FALSE);
    ID_AVIS (right_id1) = AVIS_SSATHEN (avis);

    /* ELSE part */
    /* create right side (id) of copy assignment for else part */
    right_id2
      = MakeId (StringCopy (VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (AVIS_SSAELSE (avis)))),
                NULL, VARDEC_OR_ARG_STATUS (AVIS_VARDECORARG (AVIS_SSAELSE (avis))));
    ID_VARDEC (right_id2) = AVIS_VARDECORARG (AVIS_SSAELSE (avis));
    SET_FLAG (ID, right_id2, IS_GLOBAL, (NODE_TYPE (ID_VARDEC (right_id2)) == N_objdef));
    SET_FLAG (ID, right_id2, IS_REFERENCE, FALSE);
    ID_AVIS (right_id2) = AVIS_SSAELSE (avis);

    /* create let assign with prf_phi for then and else part */
    assign_let = MakeAssignLet (StringCopy (VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (avis))),
                                AVIS_VARDECORARG (avis),
                                MakeFuncond (MakeExprs (DupTree (COND_COND (
                                                          INFO_SSA_CONDSTMT (arg_info))),
                                                        NULL),
                                             MakeExprs (right_id1, NULL),
                                             MakeExprs (right_id2, NULL)));

    /* redefinition requieres new target variable (standard ssa mechanism) */
    LET_IDS (ASSIGN_INSTR (assign_let))
      = TravLeftIDS (LET_IDS (ASSIGN_INSTR (assign_let)), arg_info);
    left_ids = LET_IDS (ASSIGN_INSTR (assign_let));

    /*store assignment */
    AVIS_SSAASSIGN (IDS_AVIS (left_ids)) = assign_let;

    /* insert new phi assignment to arg_info for insertion
       into assignment chain later */
    if (INFO_SSA_PHIASSIGN (arg_info) == NULL) {
        INFO_SSA_PHIASSIGN (arg_info) = assign_let;
        INFO_SSA_LASTPHIASSIGN (arg_info) = assign_let;
    } else {
        ASSIGN_NEXT (INFO_SSA_LASTPHIASSIGN (arg_info)) = assign_let;
        INFO_SSA_LASTPHIASSIGN (arg_info)
          = ASSIGN_NEXT (INFO_SSA_LASTPHIASSIGN (arg_info));
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

    INFO_SSA_PHIASSIGN (arg_info) = INFO_SSA_LASTPHIASSIGN (arg_info) = NULL;

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
    if ((INFO_SSA_SINGLEFUNDEF (arg_info) == SSA_TRAV_FUNDEFS)
        && (FUNDEF_NEXT (arg_node) != NULL)) {
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

    DBUG_ENTER ("SSAblock");

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
    node *old_assign, *return_assign;

    DBUG_ENTER ("SSAassign");

    /* save old assignment link */
    old_assign = INFO_SSA_ASSIGN (arg_info);

    /* traverse expr */
    DBUG_ASSERT ((ASSIGN_INSTR (arg_node) != NULL), "no instruction in assign!");

    INFO_SSA_ASSIGN (arg_info) = arg_node;

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    /* traverse next exprs */
    if (ASSIGN_NEXT (arg_node) != NULL) {

        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);

        /* if next node is return node then insert all created
         * phi-assignments in assignment chain
         */
        if (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_cond) {

            if (INFO_SSA_PHIASSIGN (arg_info) != NULL) {

                return_assign = ASSIGN_NEXT (arg_node);
                ASSIGN_NEXT (arg_node) = INFO_SSA_PHIASSIGN (arg_info);
                ASSIGN_NEXT (INFO_SSA_LASTPHIASSIGN (arg_info)) = return_assign;

                INFO_SSA_PHIASSIGN (arg_info) = NULL;
                INFO_SSA_LASTPHIASSIGN (arg_info) = NULL;
            }
        }
    }

    /* restore old assignment link */
    INFO_SSA_ASSIGN (arg_info) = old_assign;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSAlet( node *arg_node, node *arg_info)
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
 *  node *SSAicm( node *arg_node, node *arg_info)
 *
 * description:
 *   travereses in expression and assigned ids.
 *
 ******************************************************************************/
node *
SSAicm (node *arg_node, node *arg_info)
{
    node *id;
    char *name;

    DBUG_ENTER ("SSAicm");

    name = ICM_NAME (arg_node);

    if (strstr (name, "USE_GENVAR_OFFSET") != NULL) {
        /*
         * USE_GENVAR_OFFSET( off, wl)
         * is expanded to
         *      off = wl__off    ,
         * where 'off' is a scalar and 'wl__off' an internal variable!
         *   -> traverse 'off' as a LHS
         */
        id = ICM_ARG1 (arg_node);
        DBUG_ASSERT ((NODE_TYPE (id) == N_id),
                     "1st argument of USE_GENVAR_OFFSET icm is no N_id node");
        ID_IDS (id) = TravLeftIDS (ID_IDS (id), arg_info);
    } else if (strstr (name, "VECT2OFFSET") != NULL) {
        /*
         * VECT2OFFSET( off, ., from, ., ., ...)
         * is expanded to
         *     off = ... from ...    ,
         * where 'off' is a scalar variable.
         *  -> traverse all but the first argument as RHS
         *  -> traverse first argument as LHS
         */
        ICM_EXPRS2 (arg_node) = Trav (ICM_EXPRS2 (arg_node), arg_info);

        id = ICM_ARG1 (arg_node);
        DBUG_ASSERT ((NODE_TYPE (id) == N_id),
                     "1st argument of VECT2OFFSET icm is no N_id node");
        ID_IDS (id) = TravLeftIDS (ID_IDS (id), arg_info);
    } else if (strstr (name, "IDXS2OFFSET") != NULL) {
        /*
         * IDXS2OFFSET( off_nt, ., idx_1_nt ... idx_n_nt, ., ...)
         * is expanded to
         *     off_nt = ... idx_1_nt[i] ... idx_n_nt[i] ...   ,
         * where 'off' is a scalar variable.
         *  -> traverse all but the first argument as RHS
         *  -> traverse first argument as LHS
         */
        ICM_EXPRS2 (arg_node) = Trav (ICM_EXPRS2 (arg_node), arg_info);

        id = ICM_ARG1 (arg_node);
        DBUG_ASSERT ((NODE_TYPE (id) == N_id),
                     "1st argument of IDXS2OFFSET icm is no N_id node");
        ID_IDS (id) = TravLeftIDS (ID_IDS (id), arg_info);
    } else {
        DBUG_ASSERT ((0), "unknown ICM found during RC");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSAarg(node *arg_node, node *arg_info)
 *
 * description:
 *   check for missing SSACOUNT attribute in AVIS node. installs and inits
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
     * mark stack as active
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
 *  node *SSAid( node *arg_node, node *arg_info)
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
        && (INFO_SSA_SINGLEFUNDEF (arg_info) == SSA_TRAV_SPECIALS)
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

        new_arg_info = FreeTree (new_arg_info);
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
    node *withid;

    DBUG_ENTER ("SSANwith");

    /* traverse in with-op */
    DBUG_ASSERT ((NWITH_WITHOP (arg_node) != NULL), "Nwith without WITHOP node!");
    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

    /*
     * before traversing all partions, init with-ids/-vec to check all
     * partions for identical index variables
     */
    DBUG_ASSERT ((NWITH_PART (arg_node) != NULL), "Nwith without PART node!");

    withid = NPART_WITHID (NWITH_PART (arg_node));
    if (NWITHID_IDS (withid) != NULL) {
        INFO_SSA_WITHIDS (arg_info) = IDS_AVIS (NWITHID_IDS (withid));
    } else {
        INFO_SSA_WITHIDS (arg_info) = NULL;
    }
    if (NWITHID_VEC (withid) != NULL) {
        INFO_SSA_WITHVEC (arg_info) = IDS_AVIS (NWITHID_VEC (withid));
    } else {
        INFO_SSA_WITHVEC (arg_info) = NULL;
    }

    /* traverse partitions */
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

    /* assert unique withids/withvec !!! */
    if (INFO_SSA_WITHIDS (arg_info) == NULL) {
        DBUG_ASSERT ((NWITHID_IDS (NPART_WITHID (arg_node)) == NULL),
                     "multigenerator withloop with inconsistent withids");
    } else {
        DBUG_ASSERT ((INFO_SSA_WITHIDS (arg_info)
                      == IDS_AVIS (NWITHID_IDS (NPART_WITHID (arg_node)))),
                     "multigenerator withloop with inconsistent withids");
    }
    if (INFO_SSA_WITHVEC (arg_info) == NULL) {
        DBUG_ASSERT ((NWITHID_VEC (NPART_WITHID (arg_node)) == NULL),
                     "multigenerator withloop with inconsistent withvec");
    } else {
        DBUG_ASSERT ((INFO_SSA_WITHVEC (arg_info)
                      == IDS_AVIS (NWITHID_VEC (NPART_WITHID (arg_node)))),
                     "multigenerator withloop with inconsistent withvec");
    }

    if (NPART_NEXT (arg_node) != NULL) {
        /* traverse next part */
        NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
        /*
         * not the last withid -> traverse it as RHS occurance
         */
        NWITHID_VEC (NPART_WITHID (arg_node))
          = SSArightids (NWITHID_VEC (NPART_WITHID (arg_node)), arg_info);
        NWITHID_IDS (NPART_WITHID (arg_node))
          = SSArightids (NWITHID_IDS (NPART_WITHID (arg_node)), arg_info);
    } else {
        /*
         * last withid -> traverse it as LHS occurance
         */
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

    /* traverse expressions */
    DBUG_ASSERT ((NCODE_CEXPRS (arg_node) != NULL), "Ncode without Ncexprs node!");
    NCODE_CEXPRS (arg_node) = Trav (NCODE_CEXPRS (arg_node), arg_info);

    /* traverse epilogue */
    if (NCODE_EPILOGUE (arg_node) != NULL) {
        NCODE_EPILOGUE (arg_node) = Trav (NCODE_EPILOGUE (arg_node), arg_info);
    }

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
 *  traverses in vector and ids strutures. because the withids do not have a
 *  defining assignment. we set the current assignment to NULL when processing
 *  these ids. NEW: instead we annotate the current withid
 *
 ******************************************************************************/
node *
SSANwithid (node *arg_node, node *arg_info)
{
    node *assign;

    DBUG_ENTER ("SSANwithid");

    /* set current assign to NULL for these special ids */
    assign = INFO_SSA_ASSIGN (arg_info);
    INFO_SSA_ASSIGN (arg_info) = NULL;
    INFO_SSA_WITHID (arg_info) = arg_node;

    /* The WITH must be treated like a RHS iff
       the index vector has been allocated explicitly */
    if ((IDS_AVIS (NWITHID_VEC (arg_node)) != NULL)
        && (AVIS_SSAASSIGN (IDS_AVIS (NWITHID_VEC (arg_node))) != NULL)
        && (NODE_TYPE (ASSIGN_RHS (AVIS_SSAASSIGN (IDS_AVIS (NWITHID_VEC (arg_node)))))
            == N_prf)
        && ((PRF_PRF (ASSIGN_RHS (AVIS_SSAASSIGN (IDS_AVIS (NWITHID_VEC (arg_node)))))
             == F_alloc_or_reuse)
            || (PRF_PRF (ASSIGN_RHS (AVIS_SSAASSIGN (IDS_AVIS (NWITHID_VEC (arg_node)))))
                == F_alloc))) {
        if (NWITHID_VEC (arg_node) != NULL) {
            NWITHID_VEC (arg_node) = TravRightIDS (NWITHID_VEC (arg_node), arg_info);
        }

        if (NWITHID_IDS (arg_node) != NULL) {
            NWITHID_IDS (arg_node) = TravRightIDS (NWITHID_IDS (arg_node), arg_info);
        }
    } else {
        if (NWITHID_VEC (arg_node) != NULL) {
            NWITHID_VEC (arg_node) = TravLeftIDS (NWITHID_VEC (arg_node), arg_info);
        }

        if (NWITHID_IDS (arg_node) != NULL) {
            NWITHID_IDS (arg_node) = TravLeftIDS (NWITHID_IDS (arg_node), arg_info);
        }
    }

    /* restore currect assign for further processing */
    INFO_SSA_ASSIGN (arg_info) = assign;
    INFO_SSA_WITHID (arg_info) = NULL;

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

    if (!AVIS_SSADEFINED (IDS_AVIS (arg_ids))) {
        /* first definition of variable (no renaming) */
        AVIS_SSASTACK_TOP (IDS_AVIS (arg_ids)) = IDS_AVIS (arg_ids);
        /* SSACNT_COUNT(AVIS_SSACOUNT(IDS_AVIS(arg_ids))) = 0; */
        AVIS_SSADEFINED (IDS_AVIS (arg_ids)) = TRUE;
        DBUG_PRINT ("SSA", ("first definition, no renaming: %s (" F_PTR ")",
                            VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (IDS_AVIS (arg_ids))),
                            IDS_AVIS (arg_ids)));

        AVIS_SSAASSIGN (IDS_AVIS (arg_ids)) = INFO_SSA_ASSIGN (arg_info);
        AVIS_WITHID (IDS_AVIS (arg_ids)) = INFO_SSA_WITHID (arg_info);

    } else {
        /* redefinition - create new unique variable/vardec */
        new_vardec = SSANewVardec (AVIS_VARDECORARG (IDS_AVIS (arg_ids)));
        BLOCK_VARDEC (INFO_SSA_BLOCK (arg_info))
          = AppendVardec (BLOCK_VARDEC (INFO_SSA_BLOCK (arg_info)), new_vardec);
        DBUG_PRINT ("SSA", ("re-definition, renaming: %s (" F_PTR ") -> %s",
                            VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (IDS_AVIS (arg_ids))),
                            IDS_AVIS (arg_ids), VARDEC_NAME (new_vardec)));

        /* new rename-to target for old vardec */
        AVIS_SSASTACK_TOP (IDS_AVIS (arg_ids)) = VARDEC_AVIS (new_vardec);

        if ((compiler_phase != PH_flatten) && (compiler_phase != PH_typecheck))
            AVIS_TYPE (VARDEC_AVIS (new_vardec))
              = TYCopyType (AVIS_TYPE (IDS_AVIS (arg_ids)));

        /* rename this ids */
        IDS_VARDEC (arg_ids) = new_vardec;
        IDS_AVIS (arg_ids) = VARDEC_AVIS (new_vardec);

#ifndef NO_ID_NAME
        /* for compatiblity only
         * there is no real need for name string in ids structure because
         * you can get it from vardec without redundancy.
         */
        Free (IDS_NAME (arg_ids));
        IDS_NAME (arg_ids) = StringCopy (VARDEC_NAME (new_vardec));
#endif

        /*
         * mark this avis for undo ssa transform:
         * all global objects and artificial identifier must
         * be mapped back to their original name in undossa.
         */
        if ((IDS_STATUS (arg_ids) == ST_artificial) || (IDS_ATTRIB (arg_ids) == ST_global)
            || (VARDEC_OR_ARG_ATTRIB (AVIS_VARDECORARG (IDS_AVIS (arg_ids)))
                == ST_unique)) {
            AVIS_SSAUNDOFLAG (IDS_AVIS (arg_ids)) = TRUE;
        }
        AVIS_SSAASSIGN (IDS_AVIS (arg_ids)) = INFO_SSA_ASSIGN (arg_info);
        AVIS_WITHID (IDS_AVIS (arg_ids)) = INFO_SSA_WITHID (arg_info);
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

    /* do renaming to new ssa vardec */
    if (!AVIS_SSADEFINED (IDS_AVIS (arg_ids))) {
        if (INFO_SSA_ALLOW_GOS (arg_info) == FALSE) {
            ERROR (linenum, ("var %s used without definition", IDS_NAME (arg_ids)));
        }
    } else {

        /*
         * mwe:
         *   After changing the implementation of the ssa-conditionals
         *   from PHITARGET to a real primitive function 'phi'
         *   it is possible to get empty conditional branches.
         *   Before the change this was unpossible.
         *
         *   Maybe that's the reason for the following problem:
         *   After changing to phi functions it happend that the
         *   SSASTACK was empty for the arguments of the phi function,
         *   when something was expected to be in the stack.
         *
         *   Because an empty stack at this point was not possible before,
         *   I thought it's no mistake to add the following conditional.
         */
        if (AVIS_SSASTACK_TOP (IDS_AVIS (arg_ids)) != NULL)
            IDS_AVIS (arg_ids) = AVIS_SSASTACK_TOP (IDS_AVIS (arg_ids));
    }

    /* restore all depended attributes with correct values */
    IDS_VARDEC (arg_ids) = AVIS_VARDECORARG (IDS_AVIS (arg_ids));

#ifndef NO_ID_NAME
    /* for compatiblity only
     * there is no real need for name string in ids structure because
     * you can get it from vardec without redundancy.
     */
    Free (IDS_NAME (arg_ids));
    IDS_NAME (arg_ids) = StringCopy (VARDEC_OR_ARG_NAME (IDS_VARDEC (arg_ids)));
#endif

    if (INFO_SSA_WITHID (arg_info) != NULL) {
        AVIS_WITHID (IDS_AVIS (arg_ids)) = INFO_SSA_WITHID (arg_info);
    }

    /* traverese next ids */
    if (IDS_NEXT (arg_ids) != NULL) {
        IDS_NEXT (arg_ids) = TravRightIDS (IDS_NEXT (arg_ids), arg_info);
    }

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

    DBUG_ASSERT ((arg_ids != NULL), "traversal in NULL ids");
    arg_ids = SSArightids (arg_ids, arg_info);

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   node *SSATransform(node *syntax_tree)
 *
 * description:
 *   Starts traversal of AST to transform code in SSA form. Every variable
 *   has exaclty one definition. This code transformtion relies on the
 *   lac2fun transformation! After all the valid_ssaform flag is set to TRUE.
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
    INFO_SSA_SINGLEFUNDEF (arg_info) = SSA_TRAV_FUNDEFS;
    INFO_SSA_ALLOW_GOS (arg_info) = FALSE;

    old_tab = act_tab;
    act_tab = ssafrm_tab;

    syntax_tree = Trav (syntax_tree, arg_info);

    act_tab = old_tab;

    arg_info = FreeTree (arg_info);

    valid_ssaform = TRUE;

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * function:
 *   node *SSATransformAllowGOs(node *syntax_tree)
 *
 * description:
 *   In principle, this is only a wrapper for SSATransform. The only difference
 *   is that it does not require all variables to be defined before used!!!
 *   This variant is required when the SSA form has to be built before
 *   introducing explicit data dependencies for GOs (global objects).
 *
 ******************************************************************************/
node *
SSATransformAllowGOs (node *syntax_tree)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("SSATransformAllowGOs");

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) == N_modul),
                 "SSATransformAllowGOs is used for module nodes only");

    DBUG_PRINT ("OPT", ("starting ssa transformation allowing GOs for ast"));

    arg_info = MakeInfo ();
    INFO_SSA_SINGLEFUNDEF (arg_info) = SSA_TRAV_FUNDEFS;
    INFO_SSA_ALLOW_GOS (arg_info) = TRUE;

    old_tab = act_tab;
    act_tab = ssafrm_tab;

    syntax_tree = Trav (syntax_tree, arg_info);

    act_tab = old_tab;

    arg_info = FreeTree (arg_info);

    valid_ssaform = TRUE;

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * function:
 *   node *SSATransformOneFunction(node *fundef)
 *
 * description:
 *   same as SSATransform, but traverses only the given single functions
 *   including the (implicit inlined) special fundefs, too.
 *
 ******************************************************************************/
node *
SSATransformOneFunction (node *fundef)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("SSATransformOneFunction");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "SSATransformOneFunction is used for fundef nodes only");

    if (!(FUNDEF_IS_LACFUN (fundef))) {
        DBUG_PRINT ("OPT", ("starting ssa transformation for %s", FUNDEF_NAME (fundef)));

        arg_info = MakeInfo ();
        INFO_SSA_SINGLEFUNDEF (arg_info) = SSA_TRAV_SPECIALS;

        old_tab = act_tab;
        act_tab = ssafrm_tab;

        fundef = Trav (fundef, arg_info);

        act_tab = old_tab;

        arg_info = FreeTree (arg_info);
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   node *SSATransformOneFundef(node *fundef)
 *
 * description:
 *   same as SSATransform, but traverses only the given single fundef without
 *   used special fundefs.
 *
 ******************************************************************************/
node *
SSATransformOneFundef (node *fundef)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("SSATransformOneFundef");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "SSATransformOneFundef is used for fundef nodes only");

    DBUG_PRINT ("OPT", ("starting ssa transformation for %s", FUNDEF_NAME (fundef)));

    arg_info = MakeInfo ();
    INFO_SSA_SINGLEFUNDEF (arg_info) = SSA_TRAV_NONE;

    old_tab = act_tab;
    act_tab = ssafrm_tab;

    fundef = Trav (fundef, arg_info);

    act_tab = old_tab;

    arg_info = Free (arg_info);

    DBUG_RETURN (fundef);
}
