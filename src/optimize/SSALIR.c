
/*
 *
 * $Log$
 * Revision 1.13  2001/04/20 11:34:55  nmw
 * warning removed
 *
 * Revision 1.12  2001/04/20 08:50:42  nmw
 * additional assert for undefined identifiers added
 *
 * Revision 1.11  2001/04/19 16:34:38  nmw
 * with-loop independend removal implemented
 *
 * Revision 1.10  2001/04/19 11:46:23  nmw
 * bugs in move down expressions fixed
 *
 * Revision 1.9  2001/04/18 12:55:42  nmw
 * debug output for OPT traversal added
 *
 * Revision 1.8  2001/04/17 15:55:44  nmw
 * move down of expressions implemented (recursion is still buggy)
 *
 * Revision 1.7  2001/04/12 12:40:26  nmw
 * move up of loop independend expressions in do loops implemented
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
#include "SSATransform.h"

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

/* AVIS_DEFDEPTH */
#define DD_UNDEFINED -1

/* functions for local usage only */
static ids *SSALIRleftids (ids *arg_ids, node *arg_info);
static ids *LIRMOVleftids (ids *arg_ids, node *arg_info);
static node *CheckMoveDownFlag (node *instr, node *arg_info);
static node *CreateNewResult (node *avis, node *arg_info);
static LUT_t InsertMappingsIntoLUT (LUT_t move_table, nodelist *mappings);
static node *AdjustExternalResult (node *new_assigns, node *ext_assign, node *ext_fundef);
static nodelist *InsListPushFrame (nodelist *il);
static nodelist *InsListPopFrame (nodelist *il);
static nodelist *InsListAppendAssigns (nodelist *il, node *assign, int depth);
static nodelist *InsListSetAssigns (nodelist *il, node *assign, int depth);
static node *InsListGetAssigns (nodelist *il, int depth);
static nodelist *InsListGetFrame (nodelist *il, int depth);

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

    DBUG_ENTER ("CheckMoveDownFlag");

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

/******************************************************************************
 *
 * function:
 *   node *CreateNewResult(node *avis, node *arg_info)
 *
 * description:
 *   creates a new result in this fundef (returning given avis/vardec):
 *     1. create new vardec in external (calling) fundef
 *     2. add [ avis -> new_ext_avis] to RESULTMAP nodelist
 *     3. create new vardec in local fundef (as result in recursive call)
 *     4. create new vardec in local fundef (as phi copy target)
 *     5. modify functions signature (AddResult)
 *     6. insert phi-copy-assignments in then and else part of conditional
 *
 *****************************************************************************/
node *
CreateNewResult (node *avis, node *arg_info)
{
    node *new_ext_vardec;
    node *new_ext_avis;
    node *new_int_vardec;
    node *new_pct_vardec;
    char *new_name;
    nodelist *letlist;
    node *tmp;
    node *cond;
    node *right_id;
    node *assign_let;

    DBUG_ENTER ("CreateNewResult");

    /* 1. create new vardec in external (calling) fundef */
    if (NODE_TYPE (AVIS_VARDECORARG (avis)) == N_vardec) {
        new_ext_vardec = DupNode (AVIS_VARDECORARG (avis));
    } else if (NODE_TYPE (AVIS_VARDECORARG (avis)) == N_arg) {
        new_ext_vardec = MakeVardecFromArg (AVIS_VARDECORARG (avis));
    } else {
        new_ext_vardec = NULL;
        DBUG_ASSERT ((FALSE), "unsupported nodetype");
    }

    new_ext_avis = AdjustAvisData (new_ext_vardec, INFO_SSALIR_EXTFUNDEF (arg_info));
    new_name = TmpVarName (VARDEC_NAME (new_ext_vardec));
    FREE (VARDEC_NAME (new_ext_vardec));
    VARDEC_NAME (new_ext_vardec) = new_name;

    DBUG_PRINT ("SSALIR", ("create external vardec %s for %s", new_name,
                           VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (avis))));

    /* add vardec to chain of vardecs (ext. fundef) */
    BLOCK_VARDEC (FUNDEF_BODY (INFO_SSALIR_EXTFUNDEF (arg_info)))
      = AppendVardec (BLOCK_VARDEC (FUNDEF_BODY (INFO_SSALIR_EXTFUNDEF (arg_info))),
                      new_ext_vardec);

    /* 2. add [avis -> new_ext_avis] to RESULTMAP nodelist */
    INFO_SSALIR_RESULTMAP (arg_info)
      = NodeListAppend (INFO_SSALIR_RESULTMAP (arg_info), avis, new_ext_avis);

    /* mark variable as being a result of this function */
    AVIS_EXPRESULT (avis) = TRUE;

    /* 3. create new vardec in local fundef (as result in recursive call) */
    new_int_vardec = SSANewVardec (AVIS_VARDECORARG (avis));
    BLOCK_VARDEC (FUNDEF_BODY (INFO_SSALIR_FUNDEF (arg_info)))
      = AppendVardec (BLOCK_VARDEC (FUNDEF_BODY (INFO_SSALIR_FUNDEF (arg_info))),
                      new_int_vardec);

    /* 4. create new vardec in local fundef (as PhiCopyTarget) */
    new_pct_vardec = SSANewVardec (AVIS_VARDECORARG (avis));
    AVIS_SSAPHITARGET (VARDEC_AVIS (new_pct_vardec)) = PHIT_DO;
    BLOCK_VARDEC (FUNDEF_BODY (INFO_SSALIR_FUNDEF (arg_info)))
      = AppendVardec (BLOCK_VARDEC (FUNDEF_BODY (INFO_SSALIR_FUNDEF (arg_info))),
                      new_pct_vardec);

    /* 5. modify functions signature (AddResult) */
    /* recusrive call */
    letlist
      = NodeListAppend (NULL,
                        ASSIGN_INSTR (FUNDEF_INT_ASSIGN (INFO_SSALIR_FUNDEF (arg_info))),
                        new_int_vardec);

    /*external call */
    letlist = NodeListAppend (letlist,
                              ASSIGN_INSTR (NODELIST_NODE (
                                FUNDEF_EXT_ASSIGNS (INFO_SSALIR_FUNDEF (arg_info)))),
                              new_ext_vardec);

    INFO_SSALIR_FUNDEF (arg_info)
      = CSAddResult (INFO_SSALIR_FUNDEF (arg_info), new_pct_vardec, letlist);

    /* set correct assign nodes */
    AVIS_SSAASSIGN (VARDEC_AVIS (new_int_vardec))
      = FUNDEF_INT_ASSIGN (INFO_SSALIR_FUNDEF (arg_info));

    AVIS_SSAASSIGN (VARDEC_AVIS (new_ext_vardec))
      = NODELIST_NODE (FUNDEF_EXT_ASSIGNS (INFO_SSALIR_FUNDEF (arg_info)));

    /* 6. insert phi-copy-assignments in then and else part of conditional */
    /* search for conditional */
    tmp = BLOCK_INSTR (FUNDEF_BODY (INFO_SSALIR_FUNDEF (arg_info)));
    while ((NODE_TYPE (ASSIGN_INSTR (tmp)) != N_cond) && (tmp != NULL)) {
        tmp = ASSIGN_NEXT (tmp);
    }

    DBUG_ASSERT ((tmp != NULL), "missing conditional in do-loop special function");
    cond = ASSIGN_INSTR (tmp);

    /* insert copy assignment in then block */
    right_id = MakeId (StringCopy (VARDEC_OR_ARG_NAME (new_int_vardec)), NULL,
                       VARDEC_OR_ARG_STATUS (new_int_vardec));
    ID_VARDEC (right_id) = new_int_vardec;
    ID_AVIS (right_id) = VARDEC_AVIS (new_int_vardec);

    /* create one let assign for then part */
    assign_let = MakeAssignLet (StringCopy (VARDEC_OR_ARG_NAME (new_pct_vardec)),
                                new_pct_vardec, right_id);

    /* append new copy assignment to then-part block */
    BLOCK_INSTR (COND_THEN (cond))
      = AppendAssign (BLOCK_INSTR (COND_THEN (cond)), assign_let);
    AVIS_SSAASSIGN (VARDEC_AVIS (new_pct_vardec)) = assign_let;

    /*  insert copy assignment in else block (the given result identifier) */
    right_id = MakeId (StringCopy (VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (avis))), NULL,
                       VARDEC_OR_ARG_STATUS (AVIS_VARDECORARG (avis)));
    ID_VARDEC (right_id) = AVIS_VARDECORARG (avis);
    ID_AVIS (right_id) = avis;

    /* create one let assign for else part */
    assign_let = MakeAssignLet (StringCopy (VARDEC_OR_ARG_NAME (new_pct_vardec)),
                                new_pct_vardec, right_id);

    /* append new copy assignment to then-part block */
    BLOCK_INSTR (COND_ELSE (cond))
      = AppendAssign (BLOCK_INSTR (COND_ELSE (cond)), assign_let);
    AVIS_SSAASSIGN2 (VARDEC_AVIS (new_pct_vardec)) = assign_let;

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * function:
 *   LUT_t InsertMappingsIntoLUT(LUT_t move_table, nodelist *mappings)
 *
 * description:
 *   inserts all mappings from nodelist for vardec/avis/name into LUT.
 *   the nodelist contains pairs of [int. avis -> ext. avis].
 *
 *****************************************************************************/
static LUT_t
InsertMappingsIntoLUT (LUT_t move_table, nodelist *mappings)
{
    DBUG_ENTER ("InsertMappingsIntoLUT");

    /* add all internal->external connections to LUT: */
    while (mappings != NULL) {
        /* vardec */
        move_table
          = InsertIntoLUT_P (move_table, AVIS_VARDECORARG (NODELIST_NODE (mappings)),
                             AVIS_VARDECORARG (((node *)NODELIST_ATTRIB2 (mappings))));

        /* avis */
        move_table = InsertIntoLUT_P (move_table, NODELIST_NODE (mappings),
                                      ((node *)NODELIST_ATTRIB2 (mappings)));

        /* id name */
        move_table = InsertIntoLUT_S (move_table,
                                      VARDEC_OR_ARG_NAME (
                                        AVIS_VARDECORARG (NODELIST_NODE (mappings))),
                                      VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (
                                        ((node *)NODELIST_ATTRIB2 (mappings)))));

        mappings = NODELIST_NEXT (mappings);
    }

    DBUG_RETURN (move_table);
}

/******************************************************************************
 *
 * function:
 *   node *AdjustExternalResult(node *new_assigns,
 *                              node *ext_assign,
 *                              node *ext_fundef)
 *
 * description:
 *   remove duplicate definitions after inserting move down assignments by
 *   inserting new dummy variables in the result list of the external call.
 *   new_ids is the list of new defined identifiers
 *   ext_assign/ext_fundef are links to the external assignment and fundef
 *
 *****************************************************************************/
static node *
AdjustExternalResult (node *new_assigns, node *ext_assign, node *ext_fundef)
{
    ids *result_chain;
    node *new_vardec;
    ids *new_ids;

    DBUG_ENTER ("AdjustExternalResult");

    DBUG_ASSERT ((NODE_TYPE (ext_assign) == N_assign), "no external assignment node");

    /* search for corresponding result ids in external function call */
    do {
        /* get ids result chain of moved assignment */
        DBUG_ASSERT ((NODE_TYPE (ASSIGN_INSTR (new_assigns)) == N_let),
                     "moved assignments must be let nodes");

        new_ids = LET_IDS (ASSIGN_INSTR (new_assigns));

        while (new_ids != NULL) {
            result_chain = LET_IDS (ASSIGN_INSTR (ext_assign));

            while (result_chain != NULL) {
                if (IDS_AVIS (new_ids) == IDS_AVIS (result_chain)) {
                    /* corresponding ids found - create new vardec and rename result_ids
                     */
                    new_vardec
                      = SSANewVardec (AVIS_VARDECORARG (IDS_AVIS (result_chain)));
                    BLOCK_VARDEC (FUNDEF_BODY (ext_fundef))
                      = AppendVardec (BLOCK_VARDEC (FUNDEF_BODY (ext_fundef)),
                                      new_vardec);

                    /* rename ids */
                    IDS_VARDEC (result_chain) = new_vardec;
                    IDS_AVIS (result_chain) = VARDEC_AVIS (new_vardec);

#ifndef NO_ID_NAME
                    /* for compatiblity only
                     * there is no real need for name string in ids structure because
                     * you can get it from vardec without redundancy.
                     */
                    FREE (IDS_NAME (result_chain));
                    IDS_NAME (result_chain) = StringCopy (VARDEC_NAME (new_vardec));
#endif
                    /* stop seerching */
                    result_chain = NULL;

                } else {
                    /* contiune search */
                    result_chain = IDS_NEXT (result_chain);
                }
            }

            /* traverse to next ids in chain */
            new_ids = IDS_NEXT (new_ids);
        }

        /* traverse to next move-down assignment */
        new_assigns = ASSIGN_NEXT (new_assigns);
    } while (new_assigns != NULL);

    DBUG_RETURN (ext_fundef);
}

/******************************************************************************
 *
 * function:
 *   nodelist *InsListPushFrame(nodelist *il)
 *
 * description:
 *   Creates a new frame in the InsertList il. This datastructure is used like
 *   a stack (push/pop) when entering a new contained with-loop and leaving
 *   it. but it also allows direct access to lower frames via their depth
 *   number to add moved assignments. So this data structure is based on a
 *   chained list nodelist.
 *
 *****************************************************************************/
static nodelist *
InsListPushFrame (nodelist *il)
{
    DBUG_ENTER ("InsListPushFrame");

    if (il == NULL) {
        /* new insert list, create level 0 */
        il = MakeNodelistNode (NULL, NULL);
        NODELIST_INT (il) = 0;
    } else {
        /* insert new frame in front of list, increment level */
        il = MakeNodelistNode (NULL, il);
        NODELIST_INT (il) = NODELIST_INT (NODELIST_NEXT (il)) + 1;
    }

    DBUG_RETURN (il);
}

/******************************************************************************
 *
 * function:
 *   nodelist *InsListPopFrame(nodelist *il)
 *
 * description:
 *   removed the latest frame from the InserList il
 *
 *****************************************************************************/
static nodelist *
InsListPopFrame (nodelist *il)
{
    DBUG_ENTER ("InsListPopFrame");

    DBUG_ASSERT ((il != NULL), "tried to pop of empty insert list");

    il = FreeNodelistNode (il);

    DBUG_RETURN (il);
}

/******************************************************************************
 *
 * function:
 *   nodelist *InsListAppendAssigns(nodelist *il, node* assign, int depth)
 *
 * description:
 *   appends given assignment(s) to the chain in frame depth.
 *
 *****************************************************************************/
static nodelist *
InsListAppendAssigns (nodelist *il, node *assign, int depth)
{
    nodelist *tmp;

    DBUG_ENTER ("InsListAppendAssigns");

    tmp = InsListGetFrame (il, depth);

    NODELIST_NODE (tmp) = AppendAssign (assign, NODELIST_NODE (tmp));

    DBUG_RETURN (il);
}

/******************************************************************************
 *
 * function:
 *   nodelist *InsListAppendAssigns(nodelist *il, node* assign, int depth)
 *
 * description:
 *   set the given assignment in in frame depth.
 *
 *****************************************************************************/
static nodelist *
InsListSetAssigns (nodelist *il, node *assign, int depth)
{
    nodelist *tmp;

    DBUG_ENTER ("InsListSetAssigns");

    tmp = InsListGetFrame (il, depth);

    NODELIST_NODE (tmp) = assign;

    DBUG_RETURN (il);
}

/******************************************************************************
 *
 * function:
 *   node *InsListGetAssigns(nodelist *il, int depth)
 *
 * description:
 *   returns the assigns chain for given depth.
 *
 *****************************************************************************/
static node *
InsListGetAssigns (nodelist *il, int depth)
{
    DBUG_ENTER ("InsListGetAssigns");

    DBUG_RETURN (NODELIST_NODE (InsListGetFrame (il, depth)));
}

/******************************************************************************
 *
 * function:
 *   nodelist *InsListGetFrame(nodelist *il, int depth)
 *
 * description:
 *   gets the insert list frame for "depth".
 *   this function is for internal use only
 *
 *****************************************************************************/
static nodelist *
InsListGetFrame (nodelist *il, int depth)
{
    int pos;
    nodelist *tmp;

    DBUG_ENTER ("InsListGetFrame");

    DBUG_ASSERT ((il != NULL), "try to access empty insert list");

    DBUG_ASSERT (((depth >= 0) && (depth <= NODELIST_INT (il))),
                 "parameter depth out of range of given insert list");

    /* search for nodelist element of given depth */
    tmp = il;
    for (pos = NODELIST_INT (il); pos > depth; pos--) {
        DBUG_ASSERT ((tmp != NULL), "unexpected end of insert list");
        tmp = NODELIST_NEXT (tmp);
    }

    DBUG_ASSERT ((NODELIST_INT (tmp) == depth),
                 "select wrong frame - maybe corrupted insert list");

    DBUG_RETURN (tmp);
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

    /* init InsertList for with-loop independed removal */
    INFO_SSALIR_INSLIST (arg_info) = InsListPushFrame (NULL);

    /* traverse function body */
    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    /* start LIRMOV traversal of BODY to move out marked assignments */
    act_tab = lirmov_tab;
    FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    act_tab = ssalir_tab;

    /* clean up insert list */
    INFO_SSALIR_INSLIST (arg_info) = InsListPopFrame (INFO_SSALIR_INSLIST (arg_info));

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
    AVIS_DEFDEPTH (VARDEC_AVIS (arg_node)) = DD_UNDEFINED;
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

    /* in case of an empty block, insert at least the empty node */
    if (BLOCK_INSTR (arg_node) == NULL) {
        BLOCK_INSTR (arg_node) = MakeEmpty ();
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
 *   traverse assign instructions in top-down order to infere LI-assignments,
 *   mark move up expressions and do the WLIR movement on the bottomo up
 *   return traversal.
 *
 ******************************************************************************/
node *
SSALIRassign (node *arg_node, node *arg_info)
{
    bool remove_assign;
    node *pre_assign;
    node *tmp;
    int old_maxdepth;
    int wlir_move_up;

    DBUG_ENTER ("SSALIRassign");

    DBUG_ASSERT ((ASSIGN_INSTR (arg_node)), "missing instruction in assignment");

    /* init traversal flags */
    INFO_SSALIR_REMASSIGN (arg_info) = FALSE;
    INFO_SSALIR_ASSIGN (arg_info) = arg_node;
    INFO_SSALIR_PREASSIGN (arg_info) = NULL;
    INFO_SSALIR_POSTASSIGN (arg_info) = NULL;
    old_maxdepth = INFO_SSALIR_MAXDEPTH (arg_info);
    INFO_SSALIR_MAXDEPTH (arg_info) = 0;

    /* start traversl in instruction */
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    /*
     * check for with loop independend expressions:
     * if all used identifiers are defined on a outer level, we can move
     * up the whole assignment to this level.
     */
    if (INFO_SSALIR_MAXDEPTH (arg_info) < INFO_SSALIR_WITHDEPTH (arg_info)) {
        wlir_move_up = INFO_SSALIR_MAXDEPTH (arg_info);
    } else {
        wlir_move_up = -1;
    }

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

    /* move up assignment in case of WLIR */
    if (wlir_move_up >= 0) {
        tmp = arg_node;
        arg_node = ASSIGN_NEXT (arg_node);
        ASSIGN_NEXT (tmp) = NULL;

        /* append to InsertList on movement target level */
        INFO_SSALIR_INSLIST (arg_info)
          = InsListAppendAssigns (INFO_SSALIR_INSLIST (arg_info), tmp, wlir_move_up);

        /* increment statistic counter */
        wlir_expr++;
    }

    /*
     * insert pre-assign code
     * remark: the pre-assigned code will not be traversed during this cycle
     */
    if (pre_assign != NULL) {
        arg_node = AppendAssign (pre_assign, arg_node);
    }

    /* restore old maxdepth counter */
    INFO_SSALIR_MAXDEPTH (arg_info) = old_maxdepth;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALIRlet(node *arg_node, node *arg_info)
 *
 * description:
 *   traverse let expression and result identifiers
 *   checks, if the dependend used identifier are loop invariant and marks the
 *   expression for move_up (only in topblock). expressions in with loops
 *   that only depends on local identifiers are marked as local, too.
 *   all other expressions are marked as normal, which means nothing special.
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

    /* detect withloop independend expression, will be moved up */
    if (INFO_SSALIR_MAXDEPTH (arg_info) < INFO_SSALIR_WITHDEPTH (arg_info)) {
        /* new target definition depth */
        INFO_SSALIR_SETDEPTH (arg_info) = INFO_SSALIR_MAXDEPTH (arg_info);
        DBUG_PRINT ("SSALIR",
                    ("moving assignment from depth %d to depth %d",
                     INFO_SSALIR_WITHDEPTH (arg_info), INFO_SSALIR_MAXDEPTH (arg_info)));

    } else {
        /* current depth */
        INFO_SSALIR_SETDEPTH (arg_info) = INFO_SSALIR_WITHDEPTH (arg_info);
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

        /*
         * calc the maximum definition depth of all identifiers in the
         * current assignment
         */
        DBUG_ASSERT ((AVIS_DEFDEPTH (ID_AVIS (arg_node)) != DD_UNDEFINED),
                     "usage of undefined identifier");
        if (INFO_SSALIR_MAXDEPTH (arg_info) < AVIS_DEFDEPTH (ID_AVIS (arg_node))) {
            INFO_SSALIR_MAXDEPTH (arg_info) = AVIS_DEFDEPTH (ID_AVIS (arg_node));
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

                if (AVIS_NEEDCOUNT (ID_AVIS (id)) == 1) {

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
    COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);

    /* traverse else part */
    INFO_SSALIR_CONDSTATUS (arg_info) = CONDSTATUS_ELSEPART;
    COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);

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
 *   traverses with-loop, increments withdepth counter during traversal,
 *   adds a new frame to the InsertList stack for later code movement.
 *
 ******************************************************************************/
node *
SSALIRNwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRNwith");

    /* clear current InsertListFrame */
    INFO_SSALIR_INSLIST (arg_info)
      = InsListSetAssigns (INFO_SSALIR_INSLIST (arg_info), NULL,
                           INFO_SSALIR_WITHDEPTH (arg_info));

    /* increment withdepth counter */
    INFO_SSALIR_WITHDEPTH (arg_info) = INFO_SSALIR_WITHDEPTH (arg_info) + 1;

    /* create new InsertListFrame */
    INFO_SSALIR_INSLIST (arg_info) = InsListPushFrame (INFO_SSALIR_INSLIST (arg_info));

    /* traverse partition */
    NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);

    /* travserse code blocks */
    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

    /* traverse withop */
    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

    /* remove top InsertListFrame */
    INFO_SSALIR_INSLIST (arg_info) = InsListPopFrame (INFO_SSALIR_INSLIST (arg_info));

    /* decrement withdepth counter */
    INFO_SSALIR_WITHDEPTH (arg_info) = INFO_SSALIR_WITHDEPTH (arg_info) - 1;

    /* move the assigns into PREASSIGN */
    INFO_SSALIR_PREASSIGN (arg_info)
      = AppendAssign (INFO_SSALIR_PREASSIGN (arg_info),
                      InsListGetAssigns (INFO_SSALIR_INSLIST (arg_info),
                                         INFO_SSALIR_WITHDEPTH (arg_info)));
    /* clear this frame */
    INFO_SSALIR_INSLIST (arg_info)
      = InsListSetAssigns (INFO_SSALIR_INSLIST (arg_info), NULL,
                           INFO_SSALIR_WITHDEPTH (arg_info));

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
    INFO_SSALIR_SETDEPTH (arg_info) = INFO_SSALIR_WITHDEPTH (arg_info);

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
 *   set current withloop depth as ids definition depth
 *   set current movement flag as ids LIRMOV flag
 *
 *   also updates the AVIS_SSAASSIGN(2) attributes
 *
 *****************************************************************************/
static ids *
SSALIRleftids (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("SSALIRleftids");

    /* set current withloop depth as definition depth */
    AVIS_DEFDEPTH (IDS_AVIS (arg_ids)) = INFO_SSALIR_SETDEPTH (arg_info);

    /* propagte the currect FLAG to the ids */
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
    node *moved_assignments;

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

    if (INFO_SSALIR_TOPBLOCK (arg_info)) {
        /*
         * to do the code movement via DupTree and LookUpTable
         * the look-up table contains all pairs of internal/external
         * vardecs, avis and strings of id's.
         * because duptree adds several nodes to the look-up table we have
         * to duplicate our move_table before we use it, to have
         * no wrong pointers to freed data in it in next DupTree operation.
         */
        switch (INFO_SSALIR_FLAG (arg_info)) {
        case SSALIR_MOVEUP:
            move_table = DuplicateLUT (INFO_SSALIR_MOVELUT (arg_info));

            /* move up to external preassign chain */
            INFO_SSALIR_EXTPREASSIGN (arg_info)
              = AppendAssign (INFO_SSALIR_EXTPREASSIGN (arg_info),
                              DupNodeLUT (arg_node, move_table));

            /* free temp. LUT */
            move_table = RemoveLUT (move_table);

            /* one loop invarinat expression removed */
            lir_expr++;

            /* move up expression can be removed - there are no further references */
            remove_assignment = TRUE;
            break;

        case SSALIR_MOVEDOWN:
            move_table = DuplicateLUT (INFO_SSALIR_MOVELUT (arg_info));

            /* init LUT result mappings */
            move_table
              = InsertMappingsIntoLUT (move_table, INFO_SSALIR_RESULTMAP (arg_info));

            /* duplicate move-down expressions with LUT */
            moved_assignments = DupNodeLUT (arg_node, move_table);

            /* adjust external result ids (resolve duplicate definitions) */
            INFO_SSALIR_EXTFUNDEF (arg_info)
              = AdjustExternalResult (moved_assignments,
                                      NODELIST_NODE (FUNDEF_EXT_ASSIGNS (
                                        INFO_SSALIR_FUNDEF (arg_info))),
                                      INFO_SSALIR_EXTFUNDEF (arg_info));

            /* move down to external postassign chain */
            INFO_SSALIR_EXTPOSTASSIGN (arg_info)
              = AppendAssign (INFO_SSALIR_EXTPOSTASSIGN (arg_info), moved_assignments);

            /* free temp. LUT */
            move_table = RemoveLUT (move_table);

            /* one loop invarinat expression moved */
            lir_expr++;

            /*
             * move down expressions cannot be removed - due to further references
             * all these expressions will be removed by the next SSADeadCodeRemoval
             * traversal
             */
            remove_assignment = FALSE;
            break;

        default:
            /* by default do not remove anything */
            remove_assignment = FALSE;
        }
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

    /*
     * when moving down an expression:
     * for each non-local variable that is not already a result of this
     * function create an additional result parameter that will be added
     * to the RESULTMAP for adjusting the used identifiers
     */
    if ((INFO_SSALIR_FLAG (arg_info) == SSALIR_MOVEDOWN)
        && (AVIS_LIRMOVE (ID_AVIS (arg_node)) != LIRMOVE_LOCAL)
        && (AVIS_EXPRESULT (ID_AVIS (arg_node)) != TRUE)) {

        DBUG_PRINT ("SSALIR",
                    ("create new result in %s for %s",
                     FUNDEF_NAME (INFO_SSALIR_FUNDEF (arg_info)),
                     VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (ID_AVIS (arg_node)))));
        arg_info = CreateNewResult (ID_AVIS (arg_node), arg_info);
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
            /* ##nmw## */
            NWITHID_VEC (arg_node) = LIRMOVleftids (NWITHID_VEC (arg_node), arg_info);
            NWITHID_IDS (arg_node) = LIRMOVleftids (NWITHID_IDS (arg_node), arg_info);

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

    DBUG_PRINT ("OPT", ("starting loop independent removal (ssa) in function %s",
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
