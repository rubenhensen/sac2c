/*
 *
 * $Log$
 * Revision 2.5  2000/01/26 17:27:04  dkr
 * type of traverse-function-table changed.
 *
 * Revision 2.3  1999/11/15 18:06:29  dkr
 * VARNO replaced, INFO_VARNO with changed signature
 * INFO_DCR_VARNO replaced
 *
 * Revision 2.2  1999/04/19 11:36:03  jhs
 * TRUE and FALSE from internal_lib.h will be used from now on.
 *
 * Revision 2.1  1999/02/23 12:41:11  sacbase
 * new release made
 *
 * Revision 1.25  1999/01/19 13:20:27  sbs
 * #ifndef DBUG_OFF inserted
 *
 * Revision 1.24  1999/01/18 15:46:02  sbs
 * DBUG_PRINT( "OPTMEM",...) inserted for mem-info during optimization
 *
 * Revision 1.23  1999/01/11 16:53:22  sbs
 * || in conditional does NOT work "lazy"[3~[3~ly"
 * only %[3~&& does!!
 * ,.
 *
 * Revision 1.22  1999/01/07 13:56:58  sbs
 * optimization process restructured for a function-wise optimization!
 *
 * Revision 1.21  1998/04/02 14:11:47  srs
 * removed Warnings
 * Conditional
 *
 * Revision 1.20  1998/02/26 12:34:40  srs
 * changed traversal of new WL
 *
 * Revision 1.19  1998/02/24 15:46:19  srs
 * fixed bug in DCRfundef
 *
 * Revision 1.18  1998/02/23 13:09:43  srs
 * added DCR for new WLs
 *
 * Revision 1.17  1997/05/13 16:31:49  sbs
 * N_assign node with N_annotate-instr generally marked as
 * active!
 *
 * Revision 1.16  1997/04/23  12:52:36  cg
 * decleration changed to declaration
 *
 * Revision 1.14  1996/09/11  14:13:08  asi
 * DFRmodul added
 *
 * Revision 1.13  1996/09/10  13:42:32  asi
 * Dead function removal no longer enabled in modules
 *
 * Revision 1.12  1996/08/09  16:42:52  asi
 * dead function removal added
 *
 * Revision 1.11  1996/01/17  14:39:34  asi
 * bug fixed: INFO_DEF and INFO_USE masks now updated correctly
 *
 * Revision 1.10  1995/12/21  13:27:48  asi
 * New algorithm implemented
 *
 * Revision 1.9  1995/07/24  11:49:46  asi
 * DEADvardec realizes now, if variable is never used because of array elimination
 *
 * Revision 1.8  1995/06/09  09:17:12  asi
 * bug fixed for while loops
 *
 * Revision 1.7  1995/05/15  08:42:08  asi
 * third mask - lokal defined - removed. No longer needed, because of
 * variable renaming in flatten.c .
 *
 * Revision 1.6  1995/03/24  16:08:00  asi
 * ci - Error removed
 *
 * Revision 1.5  1995/03/24  16:01:08  asi
 * changed DEADwith - with-loops handeled like lokal funktions now
 * changed free() -> FREE()
 *
 * Revision 1.4  1995/03/13  17:58:37  asi
 * changover from 'info.id' to 'info.ids' of node N_id
 *
 * Revision 1.3  1995/03/07  10:23:59  asi
 * added DEADwith
 *
 * Revision 1.2  1995/02/28  18:33:20  asi
 * changed arguments for functioncalls ...Mask(..)
 *
 * Revision 1.1  1995/02/13  16:37:42  asi
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "Error.h"
#include "dbug.h"
#include "globals.h"
#include "my_debug.h"
#include "traverse.h"

#include "optimize.h"
#include "generatemasks.h"
#include "DeadCodeRemoval.h"

#define INFO_ACT INFO_DCR_ACT (arg_info) /* active variables */
#define INFO_NEWACT                                                                      \
    INFO_DCR_NEWACT (arg_info) /* is there any new active                                \
                                  assignment in loop ? */
#define INFO_TRAVTYPE INFO_DCR_TRAVTYPE (arg_info)

typedef enum { active, redundant } assignstatus;

/*
 *
 *  functionname  : DeadCodeRemoval
 *  arguments     : 1) ptr to root of the syntaxtree
 *                  R) ptr to root of the optimized syntaxtree
 *  description   : Dead Code Removal
 *  global vars   : syntax_tree, dcr_tab, active_tab
 *  internal funs : ---
 *  external funs : Trav, MakeNode
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 *
 */
node *
DeadCodeRemoval (node *arg_node, node *info_node)
{
    funtab *tmp_tab;
#ifndef DBUG_OFF
    int mem_dead_var = dead_var;
    int mem_dead_expr = dead_expr;
#endif

    DBUG_ENTER ("DeadCodeRemoval");
    DBUG_PRINT ("OPT", ("DEAD CODE REMOVAL"));
    DBUG_PRINT ("OPTMEM", ("mem currently allocated: %d bytes", current_allocated_mem));

    /*
     * First, we traverse with active_tab, i.e. using
     * ACTfundef, ACTassign, ACTcond, ACTdo, ...
     * The effect, as far as I (sbs) can see, of that phase
     * is that all N_assign nodes are marked either
     * "active" or "redundant" in their ASSIGN_STATUS.
     * This is done by a bottom up traversal during which
     * the INFO_DCR_ACT - mask indicates (TRUE / FALSE) which
     * vars are needed further below.
     * For loops this requires multiple traversals...
     */
    tmp_tab = act_tab;
    act_tab = active_tab;
    info_node = MakeNode (N_info);
    arg_node = Trav (arg_node, info_node);
    FREE (info_node);

    act_tab = dcr_tab;
    info_node = MakeNode (N_info);
    arg_node = Trav (arg_node, info_node);
    FREE (info_node);

    DBUG_PRINT ("OPT", ("                        result: %d",
                        (dead_var + dead_expr) - (mem_dead_var + mem_dead_expr)));
    DBUG_PRINT ("OPTMEM", ("mem currently allocated: %d bytes", current_allocated_mem));
    act_tab = tmp_tab;
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : DCRfundef
 *  arguments     : 1) N_fundef - node
 *                  2) N_info - node
 *                  R) N_fundef - node
 *  description   : Traverses instruction-, declaration- and function-chain
 *                  in this sequence.
 *  global vars   : --
 *  internal funs : --
 *  external funs : OPTTrav (optimize.h) - will call Trav and updates DEF- and USE-masks
 *  macros        : FUNDEF_NAME, FUNDEF_BODY, FUNDEF_INSTR, FUNDEF_VARDEC,
 *                  FUNDEF_NEXT, FUNDEF_VARNO, FREE, INFO_DEF, INFO_USE, INFO_ACT,
 *
 *  remarks       : --
 *
 */
node *
DCRfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DCRfundef");
    DBUG_PRINT ("DCR", ("Dead Code Removal in function: %s", FUNDEF_NAME (arg_node)));
    INFO_VARNO (arg_info) = FUNDEF_VARNO (arg_node);

    if (FUNDEF_BODY (arg_node))
        FUNDEF_INSTR (arg_node) = OPTTrav (FUNDEF_INSTR (arg_node), arg_info, arg_node);

    INFO_DEF = FUNDEF_DEFMASK (arg_node);
    INFO_USE = FUNDEF_USEMASK (arg_node);
    INFO_VARNO (arg_info) = FUNDEF_VARNO (arg_node);

    if (FUNDEF_BODY (arg_node))
        FUNDEF_VARDEC (arg_node) = OPTTrav (FUNDEF_VARDEC (arg_node), arg_info, arg_node);
    INFO_DEF = NULL;
    INFO_USE = NULL;
    INFO_VARNO (arg_info) = 0;

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : ACTfundef
 *  arguments     : 1) N_fundef - node
 *                  2) N_info - node
 *                  R) N_fundef - node
 *  description   : Traverses instruction- and function-chain
 *                  in this sequence.
 *  global vars   : --
 *  internal funs : --
 *  external funs : Trav    (traverse.h) - higher order traverse function
 *                  GenMask (optimize.h) - claims mem for a mask and initialize it with 0
 *  macros        : FUNDEF_NAME, FUNDEF_BODY, FUNDEF_INSTR, FUNDEF_VARDEC, FUNDEF_NEXT,
 *                  INFO_VARNO, INFO_ACT
 *
 *  remarks       : --
 *
 */
node *
ACTfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ACTfundef");
    DBUG_PRINT ("DCR",
                ("Active Asssignment Search in function: %s", FUNDEF_NAME (arg_node)));

    INFO_VARNO (arg_info) = FUNDEF_VARNO (arg_node);

    INFO_DCR_TRAVTYPE (arg_info) = N_assign;
    INFO_DCR_ACT (arg_info) = GenMask (INFO_VARNO (arg_info));
    if (NULL != FUNDEF_BODY (arg_node) && NULL != FUNDEF_INSTR (arg_node))
        FUNDEF_INSTR (arg_node) = Trav (FUNDEF_INSTR (arg_node), arg_info);
    FREE (INFO_ACT);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : DCRvardec
 *  arguments     : 1) N_vardec - node
 *                  2) N_info - node
 *                  R) N_vardec - node
 *  description   : removes declarations if variable not used and defined
 *                  in this function
 *  global vars   : elim_arrays, dead_var
 *  internal funs : --
 *  external funs : Trav     (traverse.h) - higher order traverse function
 *                  FreeNode (free.h)     - removes the given node and returns a pointer
 *                                          to the next note in a list of node structures
 *  macros        : VARDEC_NEXT, VARDEC_VARNO, VARDEC_FLAG, VARDEC_NAME,
 *                  INFO_DEF, INFO_USE
 *  remarks       : --
 *
 */
node *
DCRvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DCRvardec");
    if (VARDEC_NEXT (arg_node))
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    if ((0 == INFO_DEF[VARDEC_VARNO (arg_node)])
        && (0 == INFO_USE[VARDEC_VARNO (arg_node)])) {
        if (VARDEC_FLAG (arg_node))
            elim_arrays++;
        dead_var++;
        DBUG_PRINT ("DCR",
                    ("Variable declaration for `%s' removed", VARDEC_NAME (arg_node)));
        arg_node = FreeNode (arg_node);
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : DCRassign
 *  arguments     : 1) N_assign - node
 *                  2) N_info - node
 *                  R) N_assign - node
 *  description   : removes all redundat marked nodes
 *  global vars   : dead_expr
 *  internal funs : --
 *  external funs : OPTTrav (optimize.h) - will call Trav and updates DEF- and USE-masks
 *                  FreeNode (free.h)     - removes the given node and returns a pointer
 *                                          to the next note in a list of node structures
 *  macros        : ASSIGN_NEXT, ASSIGN_STATUS, NODE_LINE, ASSIGN_INSTR
 *
 *  remarks       : --
 *
 */
node *
DCRassign (node *arg_node, node *arg_info)
{
    node *tmp;

    DBUG_ENTER ("DCRassign");
    ASSIGN_NEXT (arg_node) = OPTTrav (ASSIGN_NEXT (arg_node), arg_info, arg_node);
    if (redundant == ASSIGN_STATUS (arg_node)) {
        DBUG_PRINT ("DCR", ("Assignment removed in line %d", NODE_LINE (arg_node)));
        if ((tmp = GetCompoundNode (arg_node))) {
            switch (NODE_TYPE (tmp)) {
            case N_cond:
                /* 	WARN(NODE_LINE(tmp), ("Conditional removed")); */
                break;
            case N_with:
            case N_Nwith:
                /* 	WARN(NODE_LINE(tmp), ("With-expression removed")); */
                break;
            case N_do:
            case N_while:
                /* 	WARN(NODE_LINE(tmp), ("Loop removed")); */
                break;
            default:
                break;
            }
        }
        MinusMask (INFO_DEF, ASSIGN_DEFMASK (arg_node), INFO_VARNO (arg_info));
        MinusMask (INFO_USE, ASSIGN_USEMASK (arg_node), INFO_VARNO (arg_info));
        arg_node = FreeNode (arg_node);
        dead_expr++;
    } else
        ASSIGN_INSTR (arg_node) = OPTTrav (ASSIGN_INSTR (arg_node), arg_info, arg_node);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : ACTassign
 *  arguments     : 1) N_assign - node
 *                  2) N_info - node
 *                  R) N_assign - node
 *  description   : assign-node will be marked redundant or active
 *                  if assign-node is active traverses subtree
 *  global vars   : --
 *  internal funs : --
 *  external funs : Trav     (traverse.h) - higher order traverse function
 *  macros        : ASSIGN_NEXT, ASSIGN_INSTRTYPE, ASSIGN_STATUS, NODE_LINE
 *                  ASSIGN_DEFMASK, ASSIGN_USEMASK, INFO_ACT
 *
 *  remarks       : --
 *
 */
node *
ACTassign (node *arg_node, node *arg_info)
{
    int i;
    node *tmp;
    node *instr;
    nodetype instrtype;

    DBUG_ENTER ("ACTassign");

    /*
     * On our way down, we mark all assignments redundant:
     */
    if (N_assign == INFO_DCR_TRAVTYPE (arg_info))
        ASSIGN_STATUS (arg_node) = redundant;

    if (ASSIGN_NEXT (arg_node))
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);

    instr = ASSIGN_INSTR (arg_node);
    instrtype = NODE_TYPE (instr);

    if (redundant == ASSIGN_STATUS (arg_node)) {
        /*
         * check, whether this assignment has to be marked active:
         */
        if (N_return == instrtype || N_annotate == instrtype)
            ASSIGN_STATUS (arg_node) = active;
        else {
            /*
             * if we are dealing with an assignment that contains
             * active vars on its LHS (DEF-mask), activate the assignment.
             * The only exception is a simple assignment of the form:
             *   a = a;      where a is marked active
             * In this case, the assignment is NOT marked active.
             * (sbs: though I don't know why)
             *
             * NOTE here, that we can not apply de Morgan here, since
             * only && in C guarantees "lazy evaluation".
             */
            if (!((instrtype == N_let) && (NODE_TYPE (LET_EXPR (instr)) == N_id)
                  && (LET_VARNO (instr) == ID_VARNO (LET_EXPR (instr))))) {
                for (i = 0; i < INFO_VARNO (arg_info); i++) {
                    if (ASSIGN_DEFMASK (arg_node)[i] && INFO_DCR_ACT (arg_info)[i]) {
                        ASSIGN_STATUS (arg_node) = active;
                        INFO_DCR_NEWACT (arg_info) = TRUE;
                        DBUG_PRINT ("DCR", ("Assignment marked active in line %d",
                                            NODE_LINE (arg_node)));
                    }
                }
            }
        }
    }

    if (active == ASSIGN_STATUS (arg_node)) {
        if ((tmp = GetCompoundNode (arg_node))) {
            switch (NODE_TYPE (tmp)) {
            case N_cond:
            case N_while:
            case N_do:
                ASSIGN_INSTR (arg_node) = Trav (instr, arg_info);
                break;
            case N_with:
            case N_Nwith:
                /* i is varno of the resulting WL array. */
                i = VARDEC_VARNO (IDS_VARDEC (LET_IDS (ASSIGN_INSTR (arg_node))));
                INFO_DCR_ACT (arg_info)[i] = FALSE;
                ASSIGN_INSTR (arg_node) = Trav (instr, arg_info);
                break;
            default:
                DBUG_ASSERT ((FALSE), "Compound-node not implemented for DCR");
                break;
            }
        } else {
            /*
             * we are dealing with a simple assignment!
             * e.g.    a,b = f( c, d, e);
             * foreach variable do:
             *    mark vars on the LHS as passive (a,b)
             *    mark vars on the RHS as active  (c,d,e)
             */
            for (i = 0; i < INFO_VARNO (arg_info); i++) {
                if (ASSIGN_DEFMASK (arg_node)[i])
                    INFO_DCR_ACT (arg_info)[i] = FALSE;
                if (ASSIGN_USEMASK (arg_node)[i])
                    INFO_DCR_ACT (arg_info)[i] = TRUE;
            }
        }
    }

    DBUG_RETURN (arg_node);
}
/*
 *
 *  functionname  : ACTcond
 *  arguments     : 1) N_cond - node
 *                  2) N_info - node
 *                  R) N_cond - node
 *  description   : determines active variables in then and else subtrees
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Trav     (traverse.h) - higher order traverse function
 *  macros        : INFO_ACT, INFO_VARNO, COND_THEN, COND_ELSE, FREE
 *                  COND_THENDEFMASK, COND_ELSEDEFMASK, COND_CONDUSEMASK
 *
 *  remarks       : --
 *
 */
node *
ACTcond (node *arg_node, node *arg_info)
{
    long *old_INFO_ACT, *then_ACT, *else_ACT;
    int i;

    DBUG_ENTER ("ACTcond");
    old_INFO_ACT = DupMask (INFO_ACT, INFO_VARNO (arg_info));
    DBUG_PRINT ("DCR", ("Travers cond-then-body BEGIN"));
    COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);
    DBUG_PRINT ("DCR", ("Travers cond-then-body END"));
    then_ACT = INFO_ACT;
    INFO_ACT = DupMask (old_INFO_ACT, INFO_VARNO (arg_info));
    DBUG_PRINT ("DCR", ("Travers cond-else-body BEGIN"));
    COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);
    DBUG_PRINT ("DCR", ("Travers cond-else-body END"));
    else_ACT = INFO_ACT;
    INFO_ACT = old_INFO_ACT;

    for (i = 0; i < INFO_VARNO (arg_info); i++) {
        INFO_ACT[i]
          = ((INFO_ACT[i]
              && !(COND_THENDEFMASK (arg_node)[i] || COND_ELSEDEFMASK (arg_node)[i]))
             || then_ACT[i] || else_ACT[i] || COND_CONDUSEMASK (arg_node)[i]);
    }

    FREE (then_ACT);
    FREE (else_ACT);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : DCRcond
 *  arguments     : 1) N_cond - node
 *                  2) N_info - node
 *                  R) N_cond - node
 *  description   : then and else subrees will be traversed and mask's will be updated
 *  global vars   : --
 *  internal funs : --
 *  external funs : OPTTrav (optimize.h) - will call Trav and updates DEF- and USE-masks
 *  macros        : COND_THENINSTR, COND_ELSEINSTR
 *
 *  remarks       : --
 *
 */
node *
DCRcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DCRcond");
    COND_THENINSTR (arg_node) = OPTTrav (COND_THENINSTR (arg_node), arg_info, arg_node);
    COND_ELSEINSTR (arg_node) = OPTTrav (COND_ELSEINSTR (arg_node), arg_info, arg_node);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : ACTdo
 *  arguments     : 1) N_do - node
 *                  2) N_info - node
 *                  R) N_do - node
 *  description   : determines active assign-node in do-body
 *  global vars   : --
 *  internal funs : --
 *  external funs : Trav     (traverse.h) - higher order traverse function
 *  macros        : INFO_VARNO, DO_TERMMASK, INFO_ACT, TRUE, FALSE, INFO_NEWACT,
 *                  INFO_TRAVTYPE, DO_INSTR
 *
 *  remarks       : --
 *
 */
node *
ACTdo (node *arg_node, node *arg_info)
{
    int i;
    int old_newact;
    nodetype old_nodetype;

    DBUG_ENTER ("ACTdo");
    for (i = 0; i < INFO_VARNO (arg_info); i++) {
        if (0 < DO_TERMMASK (arg_node)[i])
            INFO_ACT[i] = TRUE;
    }

    old_newact = INFO_NEWACT;
    old_nodetype = INFO_TRAVTYPE;
    INFO_TRAVTYPE = N_assign;
    do {
        INFO_NEWACT = FALSE;

        DBUG_PRINT ("DCR", ("Travers do-body BEGIN"));
        DO_INSTR (arg_node) = Trav (DO_INSTR (arg_node), arg_info);
        DBUG_PRINT ("DCR", ("Travers do-body END"));

        if (INFO_NEWACT)
            old_newact = TRUE;
        INFO_TRAVTYPE = N_do;
    } while (INFO_NEWACT);

    INFO_TRAVTYPE = old_nodetype;
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : DCRdo
 *  arguments     : 1) N_do - node
 *                  2) N_info - node
 *                  R) N_do - node
 *  description   : assignment-block will be traversed and mask's will be updated
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : OPTTrav (optimize.h) - will call Trav and updates DEF- and USE-masks
 *  macros        : DO_INSTR
 *
 *  remarks       : ---
 *
 */
node *
DCRdo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DCRdo");
    DO_INSTR (arg_node) = OPTTrav (DO_INSTR (arg_node), arg_info, arg_node);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : ACTwhile
 *  arguments     : 1) N_while - node
 *                  2) N_info - node
 *                  R) N_while - node
 *  description   : determines active assign-node in while-body and
 *                  calculates active variables for next assign-node
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Trav     (traverse.h) - higher order traverse function
 *  macros        : INFO_ACT, INFO_VARNO, WHILE_TERMMASK, TRUE, INFO_NEWACT,
                    INFO_TRAVTYPE, FALSE, WHILE_INSTR, FREE
 *
 *  remarks       : ---
 *
 */
node *
ACTwhile (node *arg_node, node *arg_info)
{
    int i;
    int old_newact;
    nodetype old_nodetype;
    long *old_INFOACT;

    DBUG_ENTER ("ACTwhile");
    old_INFOACT = DupMask (INFO_ACT, INFO_VARNO (arg_info));

    for (i = 0; i < INFO_VARNO (arg_info); i++) {
        if (0 < WHILE_TERMMASK (arg_node)[i])
            INFO_ACT[i] = TRUE;
    }

    old_newact = INFO_NEWACT;
    old_nodetype = INFO_TRAVTYPE;
    INFO_TRAVTYPE = N_assign;
    do {
        INFO_NEWACT = FALSE;

        DBUG_PRINT ("DCR", ("Travers while-body BEGIN"));
        WHILE_INSTR (arg_node) = Trav (WHILE_INSTR (arg_node), arg_info);
        DBUG_PRINT ("DCR", ("Travers while-body END"));

        if (INFO_NEWACT)
            old_newact = TRUE;
        INFO_TRAVTYPE = N_while;
    } while (INFO_NEWACT);

    for (i = 0; i < INFO_VARNO (arg_info); i++)
        INFO_ACT[i] = INFO_ACT[i] || WHILE_TERMMASK (arg_node)[i] || old_INFOACT[i];

    FREE (old_INFOACT);
    INFO_TRAVTYPE = old_nodetype;
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : DCRwhile
 *  arguments     : 1) N_while - node
 *                  2) N_info - node
 *                  R) N_while - node
 *  description   : assignment-block will be traversed and mask's will be updated
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : OPTTrav (optimize.h) - will call Trav and updates DEF- and USE-masks
 *  macros        : WHILE_INSTR
 *
 *  remarks       : --
 *
 */
node *
DCRwhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DCRwhile");
    WHILE_INSTR (arg_node) = OPTTrav (WHILE_INSTR (arg_node), arg_info, arg_node);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : ACTwith
 *  arguments     : 1) N_with - node
 *                  2) N_info - node
 *                  R) N_with - node
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs : Trav     (traverse.h) - higher order traverse function
 *  macros        :
 *
 *  remarks       :
 *
 */
node *
ACTwith (node *arg_node, node *arg_info)
{
    int i;
    long *old_INFOACT;

    DBUG_ENTER ("ACTwith");
    old_INFOACT = INFO_ACT;
    INFO_ACT = GenMask (INFO_VARNO (arg_info));

    DBUG_PRINT ("DCR", ("Travers with-body BEGIN"));
    switch (NODE_TYPE (WITH_OPERATOR (arg_node))) {
    case N_genarray:
        BLOCK_INSTR (GENARRAY_BODY (WITH_OPERATOR (arg_node)))
          = Trav (BLOCK_INSTR (GENARRAY_BODY (WITH_OPERATOR (arg_node))), arg_info);
        break;
    case N_modarray:
        BLOCK_INSTR (MODARRAY_BODY (WITH_OPERATOR (arg_node)))
          = Trav (BLOCK_INSTR (MODARRAY_BODY (WITH_OPERATOR (arg_node))), arg_info);
        break;
    case N_foldprf:
        BLOCK_INSTR (FOLDPRF_BODY (WITH_OPERATOR (arg_node)))
          = Trav (BLOCK_INSTR (FOLDPRF_BODY (WITH_OPERATOR (arg_node))), arg_info);
        break;
    case N_foldfun:
        BLOCK_INSTR (FOLDFUN_BODY (WITH_OPERATOR (arg_node)))
          = Trav (BLOCK_INSTR (FOLDFUN_BODY (WITH_OPERATOR (arg_node))), arg_info);
        break;
    default:
        DBUG_ASSERT ((FALSE), "Operator not implemented for with_node");
        break;
    }
    DBUG_PRINT ("DCR", ("Travers with-body END"));

    /*   for(i=0;i<INFO_VARNO(arg_info);i++) { */
    /*     INFO_ACT[i] = INFO_ACT[i]  */
    /*       || (old_INFOACT[i] && !(WITH_GENDEFMASK(arg_node)[i])) */
    /*       || WITH_GENUSEMASK(arg_node)[i]  */
    /*       || WITH_OPERATORUSEMASK(arg_node)[i]; */
    /*   } */

    /*  The situation
        (old_INFOACT[i] && !(WITH_GENDEFMASK(arg_node)[i]))
        is independent from the second term since every DEF in the generator
        refers only to WL-local variables.

        if we find a situation
          with (..iv..)..;
          ..=iv;
        we get a typecheck error because iv is unknown in the 2nd line.

        if we have
          iv = ..;
          with (..iv..)..;
          ..=iv
        the index vector is renamed (therefore cannot be active) and

        if we have
          with (..iv..)..;
          iv = ..;
          ..=iv
        the index vector will keep it's name but iv is not active while
        processing the WL. */

    for (i = 0; i < INFO_VARNO (arg_info); i++)
        INFO_ACT[i] = INFO_ACT[i] || old_INFOACT[i] || WITH_GENUSEMASK (arg_node)[i]
                      || WITH_OPERATORUSEMASK (arg_node)[i];

    FREE (old_INFOACT);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : DCRwith
 *  arguments     : 1) N_with - node
 *                  2) N_info - node
 *                  R) N_with - node
 *  description   : with-block will be traversed and mask's will be updated
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : OPTTrav (optimize.h) - will call Trav and updates DEF- and USE-masks
 *  macros        : NODE_TYPE, WITH_OPERATOR, WITH_OPERATOR, BLOCK_INSTR,
 *                  GENARRAY_BODY, MODARRAY_BODY, FOLDPRF_BODY, FOLDFUN_BODY
 *
 *  remarks       : ---
 *
 */
node *
DCRwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DCRwith");
    switch (NODE_TYPE (WITH_OPERATOR (arg_node))) {
    case N_genarray:
        BLOCK_INSTR (GENARRAY_BODY (WITH_OPERATOR (arg_node)))
          = OPTTrav (BLOCK_INSTR (GENARRAY_BODY (WITH_OPERATOR (arg_node))), arg_info,
                     arg_node);
        break;
    case N_modarray:
        BLOCK_INSTR (MODARRAY_BODY (WITH_OPERATOR (arg_node)))
          = OPTTrav (BLOCK_INSTR (MODARRAY_BODY (WITH_OPERATOR (arg_node))), arg_info,
                     arg_node);
        break;
    case N_foldprf:
        BLOCK_INSTR (FOLDPRF_BODY (WITH_OPERATOR (arg_node)))
          = OPTTrav (BLOCK_INSTR (FOLDPRF_BODY (WITH_OPERATOR (arg_node))), arg_info,
                     arg_node);
        break;
    case N_foldfun:
        BLOCK_INSTR (FOLDFUN_BODY (WITH_OPERATOR (arg_node)))
          = OPTTrav (BLOCK_INSTR (FOLDFUN_BODY (WITH_OPERATOR (arg_node))), arg_info,
                     arg_node);
        break;
    default:
        DBUG_ASSERT ((FALSE), "Operator not implemented for with_node");
        break;
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *ACTNwith(node *arg_node, node *arg_info)
 *
 * description:
 *   activate variables which are needed inside a WL.
 *
 * remark:
 *   bodies are traversed from within ACTNpart.
 *
 ******************************************************************************/

node *
ACTNwith (node *arg_node, node *arg_info)
{
    int i;

    DBUG_ENTER ("ACTNwith");

    /* activate used vars in generator, call traversal of the body and
       deactivate index variables. */
    NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);

    /* activate variables used in N_Nwithop (neutral elt,...) */
    for (i = 0; i < INFO_VARNO (arg_info); i++)
        INFO_ACT[i] = INFO_ACT[i] || NWITHOP_MASK (NWITH_WITHOP (arg_node), 1)[i];

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *ACTNpart(node *arg_node, node *arg_info)
 *
 * description:
 *   All variables which are used in the generator are activated.
 *   The body is traversed in a new active-list scope, the index variables
 *   are removed from this list and the active variables are added to the
 *   superior active-list.
 *
 * remarks:
 *   The variables which are DEFined in the N_Nwithid node do not have to
 *   be defined outside for the WL to work properly. So for each body, in which
 *   an adequate index vector is active, it should be deactivated again.
 *   This would not be necessary since variable renaming in flatten but it
 *   keeps the ACT list consistent.
 *
 ******************************************************************************/

node *
ACTNpart (node *arg_node, node *arg_info)
{
    int i;
    long *old_INFOACT;

    DBUG_ENTER ("ACTNpart");

    /* create new scope */
    old_INFOACT = INFO_ACT;
    INFO_ACT = GenMask (INFO_VARNO (arg_info));

    /* activate all used vars in body. */
    /* if code is shared between Nparts this code will be traversed
       multiple times. But this is necessary because the generators may
       differ. (Well, practically the index variables will be identical.) */
    NPART_CODE (arg_node) = Trav (NPART_CODE (arg_node), arg_info);

    for (i = 0; i < INFO_VARNO (arg_info); i++)
        INFO_ACT[i]
          = (INFO_ACT[i] && !NPART_MASK (arg_node, 0)[i]) || /* deactivate withid vars */
            NPART_MASK (arg_node,
                        1)[i]; /* activate all vars which are used in the generator */

    /* merge ACT lists */
    for (i = 0; i < INFO_VARNO (arg_info); i++)
        INFO_ACT[i] = INFO_ACT[i] || old_INFOACT[i];

    FREE (old_INFOACT);

    /* next N_Npart */
    if (NPART_NEXT (arg_node))
        NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *ACTNcode(node *arg_node, node *arg_info)
 *
 * description:
 *   count active variables for body
 *
 * attention:
 *   We need to activate the NCODE_CEXPR before traversing the NCODE_CBLOCK
 *   because we do not have a return statement at the end of NCODE_CBLOCK
 *   (which is usually the sign for ACTassign to activate a var).
 *
 ******************************************************************************/

node *
ACTNcode (node *arg_node, node *arg_info)
{
    int i;

    DBUG_ENTER ("ACTNcode");

    /* activate NCODE_CEXPR */
    /* we should have an id since flatten phase */
    DBUG_ASSERT (N_id == NODE_TYPE (NCODE_CEXPR (arg_node)), ("No id in NCODE_CEXPR"));
    i = VARDEC_VARNO (ID_VARDEC (NCODE_CEXPR (arg_node)));
    INFO_ACT[i] = TRUE;

    /* traverse body */
    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRNwith(node *arg_node, node *arg_info)
 *
 * description:
 *   Traverses into the WL bodies of active WLs.
 *
 *
 ******************************************************************************/

node *
DCRNwith (node *arg_node, node *arg_info)
{
    node *code;

    DBUG_ENTER ("DCRNwith");

    code = NWITH_CODE (arg_node);
    while (code) {
        code = OPTTrav (code, arg_info, arg_node);
        code = NCODE_NEXT (code);
    }

    DBUG_RETURN (arg_node);
}
