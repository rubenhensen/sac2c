/*
 *
 * $Log$
 * Revision 1.17  1997/05/13 16:31:49  sbs
 * N_assign node with N_annotate-instr generally marked as
 * active!
 *
 * Revision 1.16  1997/04/23  12:52:36  cg
 * decleration changed to declaration
 *
 * Revision 1.15  1996/09/11  14:25:55  asi
 * *** empty log message ***
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
#include "DeadCodeRemoval.h"

#define INFO_ACT arg_info->mask[2]   /* active variables */
#define INFO_NEWACT arg_info->lineno /* is there any new active assignment in loop ? */
#define INFO_TRAVTYPE NODE_TYPE (arg_info)
#define TRUE 1
#define FALSE 0

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
    DBUG_ENTER ("DeadCodeRemoval");
    act_tab = active_tab;
    info_node = MakeNode (N_info);
    arg_node = Trav (arg_node, info_node);
    FREE (info_node);

    act_tab = dcr_tab;
    info_node = MakeNode (N_info);
    arg_node = Trav (arg_node, info_node);
    FREE (info_node);

    if (opt_dfr) {
        act_tab = dfr_tab;
        info_node = MakeNode (N_info);
        arg_node = Trav (arg_node, info_node);
        FREE (info_node);
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : DFRmodul
 *  arguments     : 1) N_modul - node
 *                  R) N_modul - node
 *  description   : Prevents DFR in modules
 *  global vars   : syntax_tree,
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : DBUG..., MODUL_FILETYPE
 *
 *  remarks       :
 *
 *
 */
node *
DFRmodul (node *arg_node, node *info_node)
{
    DBUG_ENTER ("DFRmodul");
    if (F_prog == MODUL_FILETYPE (arg_node)) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), info_node);
    }
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
 *  macros        : VARNO, FUNDEF_NAME, FUNDEF_BODY, FUNDEF_INSTR, FUNDEF_VARDEC,
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
    INFO_VARNO = FUNDEF_VARNO (arg_node);

    if (NULL != FUNDEF_BODY (arg_node))
        FUNDEF_INSTR (arg_node) = OPTTrav (FUNDEF_INSTR (arg_node), arg_info, arg_node);

    INFO_DEF = FUNDEF_DEFMASK (arg_node);
    INFO_USE = FUNDEF_USEMASK (arg_node);
    if (NULL != FUNDEF_BODY (arg_node))
        FUNDEF_VARDEC (arg_node) = OPTTrav (FUNDEF_VARDEC (arg_node), arg_info, arg_node);
    INFO_DEF = NULL;
    INFO_USE = NULL;

    FUNDEF_NEXT (arg_node) = OPTTrav (FUNDEF_NEXT (arg_node), arg_info, arg_node);
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
    INFO_VARNO = FUNDEF_VARNO (arg_node);

    NODE_TYPE (arg_info) = N_assign;
    INFO_ACT = GenMask (VARNO);
    if (NULL != FUNDEF_BODY (arg_node) && NULL != FUNDEF_INSTR (arg_node))
        FUNDEF_INSTR (arg_node) = Trav (FUNDEF_INSTR (arg_node), arg_info);
    FREE (INFO_ACT);

    if (NULL != FUNDEF_NEXT (arg_node))
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : DFRfundef
 *  arguments     : 1) N_fundef - node
 *                  2) N_info - node
 *                  R) N_fundef - node
 *  description   : Traverses instruction- (if not inline marked) and function-chain
 *                  in this sequence.
 *  global vars   : --
 *  internal funs : --
 *  external funs : Trav (optimize.h)
 *  macros        : FUNDEF_NAME, FUNDEF_BODY, FUNDEF_INSTR, FUNDEF_NEXT
 *
 *  remarks       : --
 *
 */
node *
DFRfundef (node *arg_node, node *arg_info)
{
    node *nextfun;

    DBUG_ENTER ("DFRfundef");
    DBUG_PRINT ("DCR", ("Dead Function Removal in function: %s", FUNDEF_NAME (arg_node)));

    if ((NULL != FUNDEF_BODY (arg_node)) && (0 == FUNDEF_INLINE (arg_node)))
        FUNDEF_INSTR (arg_node) = Trav (FUNDEF_INSTR (arg_node), arg_info);

    if (NULL != FUNDEF_NEXT (arg_node))
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);

    if (1 == FUNDEF_INLINE (arg_node)) {
        dead_fun++;
        nextfun = FUNDEF_NEXT (arg_node);
        FUNDEF_NEXT (arg_node) = NULL;
/*-----------------------------------------------------------------------------------*/
#ifndef NEWTREE
        arg_node->nnode = 1;
#endif
        /*-----------------------------------------------------------------------------------*/
        FreeTree (arg_node);
        arg_node = nextfun;
    }
    DBUG_RETURN (arg_node);
}

node *
DFRap (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DFRap");
    if ((1 == FUNDEF_INLINE (AP_FUNDEF (arg_node)))
        && (NULL != FUNDEF_INSTR (AP_FUNDEF (arg_node)))) {
        FUNDEF_INLINE (AP_FUNDEF (arg_node)) = 0;
        FUNDEF_INSTR (AP_FUNDEF (arg_node))
          = Trav (FUNDEF_INSTR (AP_FUNDEF (arg_node)), arg_info);
    }
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
    if (NULL != VARDEC_NEXT (arg_node)) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
/*-----------------------------------------------------------------------------------*/
#ifndef NEWTREE
        if (NULL == VARDEC_NEXT (arg_node))
            arg_node->nnode = 0;
#endif
        /*-----------------------------------------------------------------------------------*/
    }
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
/*-----------------------------------------------------------------------------------*/
#ifndef NEWTREE
    if (NULL == ASSIGN_NEXT (arg_node))
        arg_node->nnode = 1;
#endif
    /*-----------------------------------------------------------------------------------*/
    if (redundant == ASSIGN_STATUS (arg_node)) {
        DBUG_PRINT ("DCR", ("Assignment removed in line %d", NODE_LINE (arg_node)));
        if (NULL != (tmp = GetCompoundNode (arg_node))) {
            switch (NODE_TYPE (tmp)) {
            case N_cond:
                WARN (NODE_LINE (tmp), ("Conditional removed"));
                break;
            case N_with:
                WARN (NODE_LINE (tmp), ("With-expression removed"));
                break;
            case N_do:
            case N_while:
                WARN (NODE_LINE (tmp), ("Loop removed"));
                break;
            default:
                break;
            }
        }
        MinusMask (INFO_DEF, ASSIGN_DEFMASK (arg_node), INFO_VARNO);
        MinusMask (INFO_USE, ASSIGN_USEMASK (arg_node), INFO_VARNO);
        arg_node = FreeNode (arg_node);
        dead_expr++;
    } else {
        ASSIGN_INSTR (arg_node) = OPTTrav (ASSIGN_INSTR (arg_node), arg_info, arg_node);
    }
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

    DBUG_ENTER ("ACTassign");
    if (N_assign == INFO_TRAVTYPE)
        ASSIGN_STATUS (arg_node) = redundant;

    if (NULL != ASSIGN_NEXT (arg_node))
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);

    if (redundant == ASSIGN_STATUS (arg_node)) {
        if ((N_return == ASSIGN_INSTRTYPE (arg_node))
            || (N_annotate == ASSIGN_INSTRTYPE (arg_node)))
            ASSIGN_STATUS (arg_node) = active;
        else {
            int egal = FALSE;

            if ((N_let == ASSIGN_INSTRTYPE (arg_node))
                && (N_id == NODE_TYPE (LET_EXPR (ASSIGN_INSTR (arg_node)))))
                if (LET_VARNO (ASSIGN_INSTR (arg_node))
                    == ID_VARNO (LET_EXPR ((ASSIGN_INSTR (arg_node)))))
                    egal = TRUE;

            if (!egal) {
                for (i = 0; i < VARNO; i++) {
                    if (0 != ASSIGN_DEFMASK (arg_node)[i] && INFO_ACT[i]) {
                        ASSIGN_STATUS (arg_node) = active;
                        INFO_NEWACT = TRUE;
                        DBUG_PRINT ("DCR", ("Assignment marked active in line %d",
                                            NODE_LINE (arg_node)));
                    }
                }
            }
        }
    }

    if (active == ASSIGN_STATUS (arg_node)) {
        if (NULL != (tmp = GetCompoundNode (arg_node))) {
            switch (NODE_TYPE (tmp)) {
            case N_cond:
            case N_while:
            case N_do:
                ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
                break;
            case N_with:
                ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
                i = VARDEC_VARNO (IDS_VARDEC (LET_IDS (ASSIGN_INSTR (arg_node))));
                INFO_ACT[i] = FALSE;
                break;
            default:
                DBUG_ASSERT ((FALSE), "Compound-node not implemented for DCR");
                break;
            }
        } else {
            for (i = 0; i < VARNO; i++) {
                if (0 != ASSIGN_DEFMASK (arg_node)[i])
                    INFO_ACT[i] = FALSE;
                if (0 != ASSIGN_USEMASK (arg_node)[i])
                    INFO_ACT[i] = TRUE;
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
    old_INFO_ACT = DupMask (INFO_ACT, INFO_VARNO);
    DBUG_PRINT ("DCR", ("Travers cond-then-body BEGIN"));
    COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);
    DBUG_PRINT ("DCR", ("Travers cond-then-body END"));
    then_ACT = INFO_ACT;
    INFO_ACT = DupMask (old_INFO_ACT, INFO_VARNO);
    DBUG_PRINT ("DCR", ("Travers cond-else-body BEGIN"));
    COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);
    DBUG_PRINT ("DCR", ("Travers cond-else-body END"));
    else_ACT = INFO_ACT;
    INFO_ACT = old_INFO_ACT;

    for (i = 0; i < VARNO; i++) {
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
    for (i = 0; i < INFO_VARNO; i++) {
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
    old_INFOACT = DupMask (INFO_ACT, INFO_VARNO);

    for (i = 0; i < INFO_VARNO; i++) {
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

    for (i = 0; i < INFO_VARNO; i++)
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
    INFO_ACT = GenMask (INFO_VARNO);

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

    for (i = 0; i < INFO_VARNO; i++) {
        INFO_ACT[i] = INFO_ACT[i] || (old_INFOACT[i] && !(WITH_GENDEFMASK (arg_node)[i]))
                      || WITH_GENUSEMASK (arg_node)[i]
                      || WITH_OPERATORUSEMASK (arg_node)[i];
    }

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
