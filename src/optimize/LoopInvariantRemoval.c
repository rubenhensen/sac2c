/*
 *
 * $Log$
 * Revision 1.7  1995/06/08 15:07:31  asi
 * now considering conditional of while loops
 *
 * Revision 1.6  1995/06/02  14:37:09  asi
 * Corrected used-variables for conditional in inner loops
 *
 * Revision 1.5  1995/05/25  17:29:32  asi
 * algorithm enhanced - two definitions in a loop doesn't matter anymore ...
 *
 * Revision 1.4  1995/05/16  12:41:50  asi
 * right and left hand side of let-expression handled seperatly now,
 * and some bugs fixed.
 *
 * Revision 1.3  1995/05/15  08:49:25  asi
 * LIRfundef, LIRMblock, LIRassign, LIRMassign, LIRloop, LIRMloop, LIRcond added.
 * First version of loop invariant removal implemented.
 *
 * Revision 1.2  1995/04/06  08:48:35  asi
 * loop invariant removal added
 *
 * Revision 1.1  1995/04/05  15:16:10  asi
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "free.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"
#include "typecheck.h"

#include "optimize.h"
#include "DupTree.h"
#include "LoopInvariantRemoval.h"

extern node *MakeNode (nodetype); /* defined in sac.y or y.tab.c respectively */

#define DEF_IN arg_info->mask[0]
#define USE_IN arg_info->mask[1]
#define LINVAR arg_info->mask[2]
#define UBD arg_info->mask[3] /* variables used in loop before defined  */
#define COND_USE arg_info->mask[4]
#define UBD_MAKE arg_info->mask[5]

#define MOVE arg_node->flag
#define MAYBE_UP 2
#define MOVE_UP 1
#define NONE 0
#define MOVE_DOWN -1
#define CAUTION -2
#define DONE -3

#define TRUE 1
#define FALSE 0
#define UNDEF -1

#define VAR "__lir"   /* name of new variable */
#define VAR_LENGTH 10 /* dimension for array of char */

#define LOOP_TYPE arg_info->nodetype
#define UP arg_info->node[0]
#define DOWN arg_info->node[1]
#define TYPE arg_info->node[2]

#define MAXVARNO arg_info->lineno

int lir_expr_no;
int print_warning = TRUE;

/*
 *
 *  functionname  : LoopInvariantRemoval
 *  arguments     : 1) ptr to root of the syntaxtree or a N_fundef - node.
 *		    2) NULL
 *                  R) ptr optimized 1)
 *  description   : initiates loop invariant removal reduction for the intermediate
 *                  sac-code
 *  global vars   : syntax_tree, act_tab
 *  internal funs : ---
 *  external funs : Trav, MakeNode
 *  macros        : DBUG...
 *
 *  remarks       : --
 *
 *
 */
node *
LoopInvariantRemoval (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("LoopInvariantRemoval");
    act_tab = lir_tab;
    arg_info = MakeNode (N_info);
    DUPTYPE = NORMAL;

    arg_node = Trav (arg_node, arg_info);

    FREE (arg_info);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : LIRfundef
 *  arguments     : 1) fundef-node
 *                  2) info-node
 *                  R) fundef-node with loop invariants moved in body of function
 *  description   :
 *  global vars   : syntax_tree
 *  internal funs :
 *  external funs : OptTrav
 *  macros        : DBUG...
 *
 *  remarks       : -)
 *
 */
node *
LIRfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("LIRfundef");

    DBUG_PRINT ("LIR",
                ("Loop invariant removal in function: %s", arg_node->info.types->id));

    lir_expr_no = 0;
    LOOP_TYPE = N_fundef;
    VARNO = arg_node->varno;
    if (NULL != arg_node->node[0])
        MAXVARNO = arg_node->node[0]->varno;

    arg_node = OptTrav (arg_node, arg_info, 0); /* functionbody */

    if (NULL != TYPE) {
        if (NULL == arg_node->node[0]->node[1])
            arg_node->node[0]->nnode++;
        arg_node->node[0]->node[1]
          = AppendNodeChain (0, arg_node->node[0]->node[1], TYPE);
        TYPE = NULL;
    }
    arg_node->varno = VARNO;

    arg_node = OptTrav (arg_node, arg_info, 1); /* next function */

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : LIRloop
 *  arguments     : 1) do-node or while-node
 *                  2) info-node
 *                  R) loop-node with loop invariants removed and stored
 *                     in UP or DOWN
 *  description   :
 *  global vars   : syntax_tree
 *  internal funs : --
 *  external funs : OptTrav, GenMask, DupMask
 *  macros        : DBUG..., VARNO, UP, DOWN
 *
 *  remarks       : --
 *
 */
node *
LIRloop (node *arg_node, node *arg_info)
{
    nodetype oldtype;
    int i;
    long *oldmask[4];

    DBUG_ENTER ("LIRloop");

    DBUG_PRINT ("LIR", ("Begin Loop - %s", mdb_nodetype[arg_node->nodetype]));
    oldtype = LOOP_TYPE;

    switch (oldtype) {
    case N_with:
    case N_while:
    case N_do:
    case N_cond:
        oldmask[0] = LINVAR;
        oldmask[1] = UBD;
        oldmask[2] = COND_USE;
        oldmask[3] = UBD_MAKE;
    default:
        switch (arg_node->nodetype) {
        case N_with:
            UBD = GenMask (VARNO); /* all variables automaticly set to FALSE */
            LINVAR = GenMask (VARNO);
            SetMask (LINVAR, TRUE, MAXVARNO);
            UBD_MAKE = GenMask (VARNO);
            SetMask (UBD_MAKE, UNDEF, MAXVARNO);
            for (i = 0; i < VARNO; i++) {
                /* all variables defined in with body may be used before defined */
                if (0 < arg_node->mask[0][i])
                    UBD[i] = UNDEF;
                /* All variables defined in generator are not loop invariant */
                if (0 < arg_node->node[0]->mask[0][i])
                    LINVAR[i] = FALSE;
                /* All variables used in generator are not loop invariant, and */
                /* perhaps used before defined. */
                if (0 < arg_node->node[0]->mask[1][i]) {
                    if (0 < arg_node->mask[0][i])
                        UBD[i] = TRUE;
                    LINVAR[i] = FALSE;
                }
                /* If there are usages for genarray or modarray they are not loop */
                /* invariant too 							*/
                if ((NULL != arg_node->node[1]->mask[1])
                    && (0 < arg_node->node[1]->mask[1][i])) {
                    if (0 < arg_node->mask[0][i])
                        UBD[i] = TRUE;
                    LINVAR[i] = FALSE;
                }
            }
            COND_USE = NULL;
            LOOP_TYPE = arg_node->nodetype;
            break;
        case N_while:
        case N_do:
            UBD = GenMask (VARNO); /* all variables automaticly set to FALSE */
            /* all variables defined in loop body may be used before defined */
            for (i = 0; i < VARNO; i++) {
                if (0 < arg_node->node[1]->mask[0][i])
                    UBD[i] = UNDEF;
            }
            LINVAR = GenMask (VARNO);
            SetMask (LINVAR, TRUE, MAXVARNO);
            UBD_MAKE = GenMask (VARNO);
            SetMask (UBD_MAKE, UNDEF, MAXVARNO);
            COND_USE = arg_node->mask[1];
            LOOP_TYPE = arg_node->nodetype;
            break;
        default:
            break;
        }
    }

    arg_node->node[1] = Trav (arg_node->node[1], arg_info);

    switch (oldtype) {
    case N_with:
    case N_while:
    case N_cond:
    case N_do:
        FREE (LINVAR);
        LINVAR = oldmask[0];
        COND_USE = oldmask[2];
        FREE (UBD_MAKE);
        UBD_MAKE = oldmask[3];
        LOOP_TYPE = oldtype;
        if (NULL != arg_node->mask[2])
            FREE (arg_node->mask[2]);
        arg_node->mask[2] = UBD;
        UBD = oldmask[1];
        break;
    default:
        switch (arg_node->nodetype) {
        case N_with:
        case N_while:
        case N_do:
            COND_USE = NULL;
            if (NULL != arg_node->mask[2])
                FREE (arg_node->mask[2]);
            arg_node->mask[2] = UBD;
            UBD = NULL;
            FREE (LINVAR);
            FREE (UBD_MAKE);
            LOOP_TYPE = oldtype;
            break;
        default:
            break;
        }
    }
    DBUG_PRINT ("LIR", ("End Loop - %s", mdb_nodetype[arg_node->nodetype]));

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : GenCseVar
 *  arguments     : 1) counter number
 *  description   : allocate string for cse-variable
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : ---
 *
 *  remarks       :
 *
 */

char *
GenCseVar (int count)
{
    char *string;

    DBUG_ENTER ("GenCseVar");

    string = (char *)malloc (sizeof (char) * VAR_LENGTH);
    sprintf (string, VAR "%d", count);

    DBUG_PRINT ("CSE", ("new variable: %s", string));

    DBUG_RETURN (string);
}

node *
DupDecleration (node *var_node, char *var_name, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupDecleration");

    DBUG_ASSERT ((0 != optvar_counter), "Not enough variables for LIR");
    optvar_counter--;
    new_node = MakeNode (N_vardec);
    new_node->info.types = DuplicateTypes (var_node->info.types);
    new_node->varno = VARNO++;
    FREE (new_node->info.types->id);
    new_node->info.types->id = var_name;

    DBUG_RETURN (new_node);
}

/*
 *
 *  functionname  : NodeBehindCast
 *  arguments     : 1) expression-node of a let-node
 *                  R) node behind various cast's
 *  description   : determine what node is hidden behind the cast-nodes
 *  global vars   : --
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG..
 *
 *  remarks       :
 *
 */
node *
NodeBehindCast (node *arg_node)
{
    DBUG_ENTER ("NodeBehindCast");
    while (N_cast == arg_node->nodetype)
        arg_node = arg_node->node[0];
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : LIRMassign
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */
node *
LIRMassign (node *arg_node, node *arg_info)
{
    node *move_node, *next_node;
    ids *ids_node;
    int all_up = TRUE;
    int todo;

    DBUG_ENTER ("LIRMassign");
    DBUG_PRINT ("OPT",
                ("Travers assign - %s", mdb_nodetype[arg_node->node[0]->nodetype]));
    todo = arg_node->flag;
    if ((N_let == arg_node->node[0]->nodetype) && (MOVE_UP == todo)) {
        ids_node = arg_node->node[0]->info.ids;
        do {
            if (MOVE_UP != ids_node->flag)
                all_up = FALSE;
            ids_node = ids_node->next;
        } while (NULL != ids_node);
        if (!all_up)
            todo = 42;
    }

    switch (todo) {
    case MOVE_UP: /* move whole assignment above loop */

        DBUG_PRINT ("LIR", ("Line %d moved up.", arg_node->lineno));
        MinusMask (DEF_IN, arg_node->mask[0], VARNO);
        MinusMask (USE_IN, arg_node->mask[1], VARNO);
        lir_expr++;
        if (2 == arg_node->nnode) {
            move_node = arg_node;
            move_node->nnode = 1;
            next_node = move_node->node[1];
            move_node->node[1] = NULL;
            move_node->flag = NONE;
            move_node->bblock = arg_info->bblock;
            UP = AppendNodeChain (1, UP, move_node);
            arg_node = LIRMassign (next_node, arg_info);
        } else {
            arg_node->flag = NONE;
            UP = AppendNodeChain (1, UP, arg_node);
            arg_node = NULL;
        }
        break;

    case 42: /* move only right hand side of let-expression and */
    {        /* some definitions outside loop */
        node *last_node, *new_node = NULL, *new_vardec;
        ids *new_ids;

        DBUG_PRINT ("LIR", ("Line %d moved partial up.", arg_node->lineno));
        /* make node that shall be moved outside the loop */
        arg_node->flag = NONE;
        move_node = MakeNode (N_assign);
        move_node->lineno = arg_node->lineno;
        move_node->node[0] = MakeNode (N_let);
        move_node->nnode = 1;
        move_node->flag = NONE;
        move_node->node[0]->node[0] = arg_node->node[0]->node[0];
        move_node->node[0]->nnode = 1;
        arg_node->node[0]->nnode = 0;

        /* the moved node gets the same USE-mask as the original node */
        move_node->mask[1] = arg_node->mask[1];
        MinusMask (USE_IN, arg_node->mask[1], VARNO);
        arg_node->mask[1] = NULL;
        arg_node->flag = NONE;

        /* generate new mask's for moved node */
        move_node->mask[0] = GenMask (VARNO);

        /* initialize do loop */
        last_node = arg_node;
        do {
            switch (arg_node->node[0]->info.ids->flag) {
            case MOVE_UP:
                new_ids = arg_node->node[0]->info.ids;
                new_ids->flag = NONE;
                arg_node->node[0]->info.ids = new_ids->next;
                new_ids->next = NULL;
                move_node->node[0]->info.ids
                  = AppendIdsChain (move_node->node[0]->info.ids, new_ids);
                break;
            case MOVE_DOWN:
                /* make new node below loop */
                new_node = MakeNode (N_assign);
                new_node->flag = NONE;
                new_node->lineno = arg_node->lineno;
                new_node->nnode = 1;
                DOWN = AppendNodeChain (1, DOWN, new_node);

                /* generate new mask's for new node */
                new_node->mask[0] = GenMask (VARNO);
                new_node->mask[1] = GenMask (VARNO);

                /* make new vardec for defined variable */
                new_vardec = DupDecleration (arg_node->node[0]->info.ids->node,
                                             GenCseVar (lir_expr_no), arg_info);
                TYPE = AppendNodeChain (0, TYPE, new_vardec);

                /* make new ids-node for move-node */
                new_ids = MakeIds (GenCseVar (lir_expr_no));
                new_ids->node = new_vardec;
                new_ids->flag = NONE;
                move_node->node[0]->info.ids
                  = AppendIdsChain (move_node->node[0]->info.ids, new_ids);
                INC_VAR (move_node->mask[0], new_vardec->varno);

                new_node->node[0] = MakeNode (N_let);
                new_node->node[0]->info.ids = arg_node->node[0]->info.ids;
                arg_node->node[0]->info.ids = new_node->node[0]->info.ids->next;
                new_node->node[0]->info.ids->next = NULL;
                new_node->node[0]->node[0] = MakeNode (N_id);
                new_node->node[0]->node[0]->info.ids = MakeIds (GenCseVar (lir_expr_no));
                new_node->node[0]->node[0]->info.ids->node = new_vardec;
                lir_expr_no++;

                INC_VAR (new_node->mask[0], new_node->node[0]->info.ids->node->varno);
                INC_VAR (new_node->mask[1], new_vardec->varno);

                last_node = new_node;
                break;
            case NONE:
                /* make new node inside loop */
                new_node = MakeNode (N_assign);
                new_node->flag = NONE;
                new_node->lineno = arg_node->lineno;
                new_node->node[1] = last_node->node[1];
                new_node->nnode = last_node->nnode;
                last_node->node[1] = new_node;
                last_node->nnode = 2;

                /* generate new mask's for new node */
                new_node->mask[0] = GenMask (VARNO);
                new_node->mask[1] = GenMask (VARNO);

                /* make new vardec for defined variable */
                new_vardec = DupDecleration (arg_node->node[0]->info.ids->node,
                                             GenCseVar (lir_expr_no), arg_info);
                TYPE = AppendNodeChain (0, TYPE, new_vardec);

                /* make new ids-node for move-node */
                new_ids = MakeIds (GenCseVar (lir_expr_no));
                new_ids->flag = NONE;
                new_ids->node = new_vardec;
                move_node->node[0]->info.ids
                  = AppendIdsChain (move_node->node[0]->info.ids, new_ids);
                INC_VAR (move_node->mask[0], new_vardec->varno);

                new_node->node[0] = MakeNode (N_let);
                new_node->node[0]->info.ids = arg_node->node[0]->info.ids;
                arg_node->node[0]->info.ids = new_node->node[0]->info.ids->next;
                new_node->node[0]->info.ids->next = NULL;
                new_node->node[0]->node[0] = MakeNode (N_id);
                new_node->node[0]->node[0]->info.ids = MakeIds (GenCseVar (lir_expr_no));
                new_node->node[0]->node[0]->info.ids->node = new_vardec;
                lir_expr_no++;

                /* add new variable to USE- rsp. DEF-mask */
                INC_VAR (new_node->mask[1], new_vardec->varno);
                INC_VAR (USE_IN, new_vardec->varno);
                /* consider left hand side */
                INC_VAR (new_node->mask[0], new_node->node[0]->info.ids->node->varno);

                last_node = new_node;
                break;
            default:
                ERROR2 (1, ("INTERNAL ERROR: Wrong move direction ( %d ) in line %d.",
                            arg_node->node[0]->info.ids->flag, arg_node->lineno));
                break;
            }
        } while (NULL != arg_node->node[0]->info.ids);

        UP = AppendNodeChain (1, UP, move_node);
        lir_expr++;
        new_node = OptTrav (new_node, arg_info, 1);
        last_node = arg_node;
        arg_node = last_node->node[1];
        last_node->nnode = 1;
        FreeTree (last_node);
    } break;
    case MOVE_DOWN: /* move whole assignment below loop */

        DBUG_PRINT ("LIR", ("Line %d moved down.", arg_node->lineno));
        MinusMask (DEF_IN, arg_node->mask[0], VARNO);
        MinusMask (USE_IN, arg_node->mask[1], VARNO);
        lir_expr++;
        if (2 == arg_node->nnode) {
            move_node = arg_node;
            move_node->nnode = 1;
            next_node = move_node->node[1];
            move_node->node[1] = NULL;
            move_node->flag = NONE;
            move_node->bblock = arg_info->bblock;
            DOWN = AppendNodeChain (1, DOWN, move_node);
            arg_node = LIRMassign (next_node, arg_info);
        } else {
            arg_node->flag = NONE;
            DOWN = AppendNodeChain (1, DOWN, arg_node);
            arg_node = NULL;
        }
        break;

    default:
        arg_node = OptTrav (arg_node, arg_info, 1);
        if (NULL == arg_node->node[1]) {
            arg_node->nnode = 1;
        }
        break;
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : LIRMloop
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */
node *
LIRMloop (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("LIRMloop");
    if (N_with == arg_node->nodetype)
        arg_node = OptTrav (arg_node, arg_info, 2);
    else
        arg_node = OptTrav (arg_node, arg_info, 1);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : LIRcond
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */
node *
LIRcond (node *arg_node, node *arg_info)
{
    int i;
    long *old_mask[2];
    long *then_UBD;
    long *then_LINVAR;
    long *else_UBD;
    long *else_LINVAR;
    nodetype oldtype;
    long *used_vars;

    DBUG_ENTER ("LIRcond");

    if (N_fundef == LOOP_TYPE) {
        arg_node = OptTrav (arg_node, arg_info, 1);
        arg_node = OptTrav (arg_node, arg_info, 2);
    } else {
        DBUG_PRINT ("LIR", ("Cond Begin: line %d", arg_node->lineno));

        /*
         * set info-node's nodetype to remember in what expression we are
         */

        oldtype = LOOP_TYPE;
        LOOP_TYPE = N_cond;

        /* all variables used in condition are relative free in if-then-else clause */
        used_vars = DupMask (arg_node->mask[1], VARNO);

        old_mask[0] = UBD;
        old_mask[1] = LINVAR;
        UBD = GenMask (VARNO);
        SetMask (UBD, UNDEF, MAXVARNO);
        LINVAR = GenMask (VARNO);
        SetMask (LINVAR, TRUE, MAXVARNO);

        /*
         * Traverse then part
         */
        arg_node = OptTrav (arg_node, arg_info, 1);

        then_UBD = UBD;
        then_LINVAR = LINVAR;
        UBD = GenMask (VARNO);
        SetMask (UBD, UNDEF, MAXVARNO);
        LINVAR = GenMask (VARNO);
        SetMask (LINVAR, TRUE, MAXVARNO);

        /*
         * Traverse else part
         */
        arg_node = OptTrav (arg_node, arg_info, 2);

        else_UBD = UBD;
        else_LINVAR = LINVAR;

        for (i = 0; i < VARNO; i++) {
            if ((TRUE == then_UBD[i]) || (TRUE == else_UBD[i])) {
                used_vars[i] = TRUE;
            }
        }

        FREE (then_UBD);
        FREE (then_LINVAR);
        FREE (else_UBD);
        FREE (else_LINVAR);

        UBD = old_mask[0];
        LINVAR = old_mask[1];

        arg_node->mask[2] = used_vars;

        /*
         * reset info-node's nodetype
         */
        LOOP_TYPE = oldtype;

        DBUG_PRINT ("LIR", ("Cond End: line %d", arg_node->lineno));
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : LIRMblock
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */
node *
LIRMblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("LIRMblock");
    arg_node = OptTrav (arg_node, arg_info, 0);
    if (NULL == arg_node->node[0])
        arg_node->node[0] = MakeNode (N_empty);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : GetUsed
 *  arguments     : 1) assign_node
 *		    2) node behind cast-nodes in let-expressions
 *		    R) ptr to the mask, which contains information's about used
 *                     variables in this expression.
 *  description   : this functions determines, what expression this assignment is,
 *                  and delivers the mask (which is calculated before) to the calling
 *		    function.
 *  global vars   : --
 *  internal funs : --
 *  external funs : --
 *  macros        : USE
 *
 *  remarks       : the mask must be calculated before
 *
 */
long *
GetUsed (node *arg_node, node *node_behind)
{
    long *used_vars;

    switch (arg_node->node[0]->nodetype) {
    case N_do:
    case N_while:
    case N_cond:
        used_vars = arg_node->node[0]->mask[2];
        break;
    case N_let:
    case N_post:
    case N_pre:
    case N_return:
        if (N_with == node_behind->nodetype) {
            used_vars = node_behind->mask[2];
        } else {
            used_vars = USE;
        }
        break;
    default:
        ERROR2 (1,
                ("INTERNAL ERROR: Assign %s not implemented in loop invariant removal !",
                 mdb_nodetype[arg_node->node[0]->nodetype]));
    }

    return (used_vars);
}

/*
 *
 *  functionname  : CheckUp
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */
node *
CheckUp (node *arg_node, node *arg_info)
{
    int term1 = TRUE, term2 = TRUE, term3 = TRUE, term4 = TRUE, term5, term6, term7;
    int i, getvar = 0;
    ids *ids_node;
    node *node_behind;
    long *used_vars;

    DBUG_ENTER ("CheckUp");

    DBUG_EXECUTE ("LIRI", char *text; text = PrintMask (LINVAR, VARNO);
                  printf ("\nLOOP INVARIANT :%s\n", text); free (text);
                  text = PrintMask (UBD, VARNO); printf ("USED BEFORE DEF:%s\n", text);
                  free (text); text = PrintMask (UBD_MAKE, VARNO);
                  printf ("UBD MAKED      :%s\n", text); free (text););

    node_behind = NodeBehindCast (arg_node->node[0]->node[0]);

    used_vars = GetUsed (arg_node, node_behind);

    for (i = 0; i < VARNO; i++) {
        if (0 < used_vars[i]) {
            if (UNDEF == UBD[i])
                UBD[i] = TRUE;
            if (TRUE == UBD[i])
                term1 = FALSE;
            if (FALSE == LINVAR[i])
                term2 = FALSE;
        }
        if (0 < DEF[i]) {
            if (UNDEF == UBD[i])
                UBD[i] = FALSE;
        }
    }

    if (term1 && term2 && term3 && term4 && (N_return != arg_node->node[0]->nodetype)) {
        MOVE = MOVE_UP;
    } else {
        MOVE = NONE;
        for (i = 0; i < VARNO; i++) {
            if ((0 < used_vars[i]) && (TRUE == UBD_MAKE[i])) {
                MOVE = CAUTION;
            }
        }
    }

    if (MOVE_UP == MOVE) {
        if (N_let == arg_node->node[0]->nodetype) {
            switch (node_behind->nodetype) {
            case N_ap:
            case N_prf:
            case N_with:
                i = 0;
                ids_node = arg_node->node[0]->info.ids;
                do {
                    i++;
                    ids_node = ids_node->next;
                } while (NULL != ids_node);
                if (term1 && term2 && (N_let == arg_node->node[0]->nodetype)
                    && (getvar = (i <= optvar_counter))) {
                    ids_node = arg_node->node[0]->info.ids;
                    do {
                        i = ids_node->node->varno;

                        if (FALSE == LINVAR[i])
                            term5 = FALSE;
                        else
                            term5 = TRUE;
                        if (TRUE == UBD[i])
                            term6 = FALSE;
                        else
                            term6 = TRUE;
                        if (FALSE == UBD_MAKE[i])
                            term7 = FALSE;
                        else
                            term7 = TRUE;

                        if (term5 && term6 && term7) {
                            if (UNDEF == UBD_MAKE[i]) {
                                ids_node->flag = MAYBE_UP;
                                UBD_MAKE[i] = TRUE;
                            } else {
                                ids_node->flag = MOVE_UP;
                            }
                        } else {
                            ids_node->flag = NONE;
                            LINVAR[i] = FALSE;
                        }
                        DBUG_PRINT ("LIR", ("Line %d VAR %d = %d, %d, %d => %d",
                                            arg_node->lineno, i, term5, term6, term7,
                                            ids_node->flag));
                        ids_node = ids_node->next;
                    } while (NULL != ids_node);
                } else {
                    if (!getvar && print_warning) {
                        WARN1 (
                          ("WARNING: Not enough variables for loop invariant removal:\n"
                           "         use option -v no (no > %d) for full optimization.\n",
                           optvar));
                        print_warning = FALSE;
                    }
                }
                break;
            default:
                ids_node = arg_node->node[0]->info.ids;
                do {
                    i = ids_node->node->varno;
                    if (FALSE == LINVAR[i])
                        term5 = FALSE;
                    else
                        term5 = TRUE;
                    if (TRUE == UBD[i])
                        term6 = FALSE;
                    else
                        term6 = TRUE;
                    if (FALSE == UBD_MAKE[i])
                        term7 = FALSE;
                    else
                        term7 = TRUE;
                    if (term5 && term6 && term7) {
                        if (UNDEF == UBD_MAKE[i]) {
                            ids_node->flag = MAYBE_UP;
                            UBD_MAKE[i] = TRUE;
                        } else {
                            ids_node->flag = MOVE_UP;
                        }
                    } else {
                        ids_node->flag = NONE;
                        LINVAR[i] = FALSE;
                        MOVE = NONE;
                    }
                    ids_node = ids_node->next;
                } while (NULL != ids_node);
                break;
            }
        } else {
            arg_node->node[0]->mask[3] = GenMask (VARNO);
            for (i = 0; i < VARNO; i++) {
                if (0 < DEF[i]) {
                    if (FALSE == LINVAR[i])
                        term5 = FALSE;
                    else
                        term5 = TRUE;
                    if (TRUE == UBD[i])
                        term6 = FALSE;
                    else
                        term6 = TRUE;
                    if (FALSE == UBD_MAKE[i])
                        term7 = FALSE;
                    else
                        term7 = TRUE;
                    if (term5 && term6 && term7) {
                        if (UNDEF == UBD_MAKE[i]) {
                            arg_node->node[0]->mask[3][i] = MAYBE_UP;
                            UBD_MAKE[i] = TRUE;
                        } else {
                            arg_node->node[0]->mask[3][i] = MOVE_UP;
                        }
                    } else {
                        arg_node->node[0]->mask[3][i] = NONE;
                        LINVAR[i] = FALSE;
                        MOVE = NONE;
                    }
                }
            }
        }
    }

    if ((NONE == MOVE) || (CAUTION == MOVE)) {
        for (i = 0; i < VARNO; i++) {
            if (0 < DEF[i]) {
                LINVAR[i] = FALSE;
            }
            if ((0 < used_vars[i]) && (TRUE == UBD_MAKE[i])) {
                UBD_MAKE[i] = FALSE;
            }
        }
    }

    DBUG_PRINT ("LIR", ("Line %d = %d, %d, %d, %d => %d", arg_node->lineno, term1, term2,
                        term3, term4, MOVE));
    DBUG_EXECUTE ("LIRI", char *text; text = PrintMask (LINVAR, VARNO);
                  printf ("LOOP INVARIANT :%s\n", text); free (text);
                  text = PrintMask (UBD, VARNO); printf ("USED BEFORE DEF:%s\n", text);
                  free (text); text = PrintMask (UBD_MAKE, VARNO);
                  printf ("UBD MAKED      :%s\n", text); free (text););
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CheckDown
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */
node *
CheckDown (node *arg_node, node *arg_info)
{
    int term1 = TRUE, term2 = TRUE, term3 = TRUE, term4, term5;
    int i, trap;
    ids *ids_node;
    node *node_behind;
    long *used_vars;

    DBUG_ENTER ("CheckDown");

    DBUG_EXECUTE ("LIRI", char *text; text = PrintMask (LINVAR, VARNO);
                  printf ("\nLOOP INVARIANT :%s\n", text); free (text);
                  text = PrintMask (UBD, VARNO); printf ("USED BEFORE DEF:%s\n", text);
                  free (text); text = PrintMask (UBD_MAKE, VARNO);
                  printf ("UBD MAKED      :%s\n", text); free (text););

    node_behind = NodeBehindCast (arg_node->node[0]->node[0]);

    used_vars = GetUsed (arg_node, node_behind);

    if (CAUTION == MOVE) {
        trap = TRUE;
        MOVE = NONE;
    } else {
        trap = FALSE;
    }

    /*
     * exemain all variables used or defined in this expression.
     */
    for (i = 0; i < VARNO; i++) {
        if (0 < DEF[i]) /* check defined variables first */
        {
            if (TRUE != LINVAR[i])
                term2 = FALSE; /* all definitions below moved ?          */
            if (UNDEF == UBD[i])
                UBD[i] = TRUE; /* first definition from below reached ?  */
            if (TRUE == UBD[i])
                term3 = FALSE; /* is it a variable used before defined ? */
        }
        if (0 < used_vars[i]) /* check used variables second */
        {
            if (TRUE == UBD[i])
                term1 = FALSE; /* is it a variable used before defined ? */
        }
    }

    /*
     * If all all terms TRUE mark this node as MOVE_DOWN
     */
    if (term1 && term2 && term3 && (N_with != node_behind->nodetype)
        && (N_return != arg_node->node[0]->nodetype))
        MOVE = MOVE_DOWN;

    /*
     * If this expression might be moved above loop, check if there are definitions
     * that might not be moved above loop but might be moved below the loop.
     */
    if (MOVE_UP == MOVE) {
        if (N_let == arg_node->node[0]->nodetype) {
            ids_node = arg_node->node[0]->info.ids;

            do {
                i = ids_node->node->varno;
                term4 = LINVAR[i];
                term5 = !UBD[i];
                if (term4 && term5 && (MOVE_UP != ids_node->flag)) {
                    ids_node->flag = MOVE_DOWN;
                } else {
                    LINVAR[i] = FALSE;
                }

                DBUG_PRINT ("LIR", ("Line %d - VAR %d = %d, %d => %d", arg_node->lineno,
                                    i, term4, term5, ids_node->flag));
                ids_node = ids_node->next;
            } while (NULL != ids_node);
        }
    }

    /*
     * if no movements are possible, set the LINVAR-mask.
     */
    if ((NONE == MOVE) || ((N_let != arg_node->node[0]->nodetype) && (MOVE_UP == MOVE))) {
        for (i = 0; i < VARNO; i++) {
            if (0 < DEF[i]) {
                LINVAR[i] = FALSE;
            }
        }
    }

    /*
     * trap means, that MOVE is equal CAUTION and this means, that this line cannot be
     * moved up but the definition of one of his used variables may be moved up, if no
     * other definition remains in this loop.
     */
    if (trap) {
        MOVE = NONE;
        for (i = 0; i < VARNO; i++) {
            if (0 < used_vars[i]) {
                if (TRUE == LINVAR[i]) /* no remaining definitions blow this line left */
                {
                    UBD_MAKE[i] = TRUE;
                }
            }
        }
    }

    /*
     * if no movements are possible, set the LINVAR-mask.
     */
    if (NONE == MOVE) {
        for (i = 0; i < VARNO; i++) {
            if ((0 < used_vars[i]) && (TRUE == LINVAR[i])) {
                LINVAR[i] = FALSE;
            }
        }
    }

    DBUG_PRINT ("LIR", ("Line %d  = %d, %d, %d => %d", arg_node->lineno, term1, term2,
                        term3, MOVE));

    DBUG_EXECUTE ("LIRI", char *text; text = PrintMask (LINVAR, VARNO);
                  printf ("LOOP INVARIANT :%s\n", text); free (text);
                  text = PrintMask (UBD, VARNO); printf ("USED BEFORE DEF:%s\n", text);
                  free (text); text = PrintMask (UBD_MAKE, VARNO);
                  printf ("UBD MAKED      :%s\n", text); free (text););

    DBUG_RETURN (arg_node);
} /* END - CheckDown */

node *
LIRsubexpr (node *arg_node, node *arg_info)
{
    node *new_node;
    long *oldmask[2];
    int i;

    DBUG_ENTER ("LIRsubexpr");

    /*****************************************************/
    /* traverse into subexpressions and add moved nodes: */
    /* 1) cond						 */
    /* 2) do loop					 */
    /* 3) while loop					 */
    /* 4) with loop					 */
    /*****************************************************/
    if (DONE != arg_node->node[0]->flag) {
        switch (arg_node->node[0]->nodetype) {
        case N_cond:
            arg_node = OptTrav (arg_node, arg_info, 0);
            break;
        case N_do:
            arg_node->node[0] = Trav (arg_node->node[0], arg_info);
            act_tab = lir_mov_tab;
            arg_info->bblock = arg_node->bblock;

            arg_node = OptTrav (arg_node, arg_info, 0);

            /* Calculate variables, which are relative in loop       */
            /* UBD-mask already linked to arg_node->node[0]->mask[2] */
            for (i = 0; i < VARNO; i++) {
                if ((0 == DEF[i])
                    && ((0 < arg_node->node[0]->node[1]->mask[1][i])
                        || (0 < arg_node->node[0]->mask[1][i])))
                    arg_node->node[0]->mask[2][i] = TRUE;
            }

            if (NULL != UP)
                arg_node->node[0]->flag = DONE;
            act_tab = lir_tab;
            PlusChainMasks (1, DOWN, arg_info);
            arg_node->node[1] = AppendNodeChain (1, DOWN, arg_node->node[1]);
            if ((1 == arg_node->nnode) && (NULL != DOWN))
                arg_node->nnode = 2;
            DOWN = NULL;
            PlusChainMasks (1, UP, arg_info);
            arg_node = AppendNodeChain (1, UP, arg_node);
            UP = NULL;
            break;
        case N_while:
            arg_node->node[0] = Trav (arg_node->node[0], arg_info);
            act_tab = lir_mov_tab;
            arg_info->bblock = arg_node->bblock;

            arg_node = OptTrav (arg_node, arg_info, 0);

            /* Calculate variables, which are relative in loop */
            for (i = 0; i < VARNO; i++) {
                /* add condition of loop */
                if ((NULL == UP) && (NULL == DOWN)
                    && (N_while == arg_node->node[0]->nodetype) && (0 != DEF[i])
                    && (0 != arg_node->node[0]->mask[1]))
                    arg_node->node[0]->mask[2][i] = TRUE;
                /* && variables not defined in loop but used */
                if ((0 == DEF[i]) && (0 < arg_node->node[0]->node[1]->mask[1][i]))
                    arg_node->node[0]->mask[2][i] = TRUE;
            }

            if (NULL != UP)
                arg_node->node[0]->flag = DONE;

            act_tab = lir_tab;

            if ((NULL != UP) || (NULL != DOWN)) {
                /* add if-then-else-clause to syntax-tree */
                new_node = MakeNode (N_assign);
                new_node->nnode = arg_node->nnode;
                new_node->node[1] = arg_node->node[1];
                arg_node->nnode = 1;
                arg_node->node[1] = NULL;
                new_node->mask[0] = DupMask (arg_node->mask[0], VARNO);
                new_node->mask[1] = DupMask (arg_node->mask[1], VARNO);

                new_node->node[0] = MakeNode (N_cond);
                new_node->node[0]->nnode = 3;

                /* Duplicate while-condition and use it with if-then-else-clause */
                new_node->node[0]->node[0]
                  = DupTree (arg_node->node[0]->node[0], arg_info);
                new_node->node[0]->mask[1] = DupMask (arg_node->node[0]->mask[1], VARNO);
                PlusMask (new_node->mask[1], arg_node->node[0]->mask[1], VARNO);
                PlusMask (arg_info->mask[1], arg_node->node[0]->mask[1], VARNO);

                /* convert while-statement to do-statement */
                arg_node->node[0]->nodetype = N_do;

                /* make then-block and link do-statement into it */
                new_node->node[0]->node[1] = MakeNode (N_block);
                new_node->node[0]->node[1]->nnode = 1;
                new_node->node[0]->node[1]->node[0] = arg_node;
                new_node->node[0]->node[1]->mask[0] = DupMask (arg_node->mask[0], VARNO);
                new_node->node[0]->node[1]->mask[1] = DupMask (arg_node->mask[1], VARNO);

                /* make empty else-block */
                new_node->node[0]->node[2] = MakeNode (N_block);
                new_node->node[0]->node[2]->nnode = 1;
                new_node->node[0]->node[2]->node[0] = MakeNode (N_empty);
                new_node->node[0]->node[2]->mask[0] = GenMask (VARNO);
                new_node->node[0]->node[2]->mask[1] = GenMask (VARNO);

                oldmask[0] = arg_info->mask[0];
                oldmask[1] = arg_info->mask[1];
                arg_info->mask[0] = GenMask (VARNO);
                arg_info->mask[1] = GenMask (VARNO);

                PlusChainMasks (1, DOWN, arg_info);
                new_node->node[0]->node[1]->node[0]
                  = AppendNodeChain (1, new_node->node[0]->node[1]->node[0], DOWN);
                DOWN = NULL;
                PlusChainMasks (1, UP, arg_info);
                new_node->node[0]->node[1]->node[0]
                  = AppendNodeChain (1, UP, new_node->node[0]->node[1]->node[0]);
                UP = NULL;

                PlusMask (new_node->node[0]->node[1]->mask[0], arg_info->mask[0], VARNO);
                PlusMask (new_node->node[0]->node[1]->mask[1], arg_info->mask[1], VARNO);

                PlusMask (new_node->mask[0], arg_info->mask[0], VARNO);
                PlusMask (new_node->mask[1], arg_info->mask[1], VARNO);

                PlusMask (arg_info->mask[0], oldmask[0], VARNO);
                PlusMask (arg_info->mask[1], oldmask[1], VARNO);

                FREE (oldmask[0]);
                FREE (oldmask[1]);

                arg_node = new_node;
            }
            break;
        case N_let: {
            node *node_behind;

            node_behind = NodeBehindCast (arg_node->node[0]->node[0]);

            if (N_with == node_behind->nodetype) {
                arg_node->node[0] = Trav (arg_node->node[0], arg_info);
                act_tab = lir_mov_tab;
                arg_info->bblock = arg_node->bblock;

                arg_node = OptTrav (arg_node, arg_info, 0);

                /* Calculate variables, which are relative in loop */
                for (i = 0; i < VARNO; i++) {
                    /* variables used in generator are relative free */
                    if (0 < node_behind->node[0]->mask[1][i])
                        node_behind->mask[2][i] = TRUE;
                    /* variables used for genarray or modarray are relative free */
                    if ((NULL != arg_node->node[1]->mask[1])
                        && (0 < arg_node->node[1]->mask[1][i]))
                        node_behind->mask[2][i] = TRUE;
                }

                arg_node->node[0]->flag = DONE;
                act_tab = lir_tab;
                PlusChainMasks (1, UP, arg_info);
                arg_node = AppendNodeChain (1, UP, arg_node);
                UP = NULL;
            }
        } break;
        default:
            break;
        }
    } else {
        arg_node->node[0]->flag = NONE;
        DBUG_PRINT ("LIR", ("Loop already done"));
    }
    DBUG_RETURN (arg_node);
} /* END - LIRsubexpr */

/*
 *  functionname  : LIRassign
 *  arguments     : 1) fundef-node
 *                  2) info-node
 *                  R) do-node with loop invariants removed and stored
 *                     in arg_info->node[0]
 *  description   :
 *  global vars   : syntax_tree
 *  internal funs :
 *  external funs : OptTrav
 *  macros        : DBUG...
 *
 *  remarks       : --
 *
 */
node *
LIRassign (node *arg_node, node *arg_info)
{
    int i;

    DBUG_ENTER ("LIRassign");

    DBUG_PRINT ("OPT", ("Travers assign - %s, line - %d",
                        mdb_nodetype[arg_node->node[0]->nodetype], arg_node->lineno));

    /* optimize subexpression */
    arg_node = LIRsubexpr (arg_node, arg_info);

    /******************************************/
    /* consider in which expession we are:    */
    /******************************************/
    switch (LOOP_TYPE) {
    /**************************/
    /* 1) loop expression !!  */
    /**************************/
    case N_with:
    case N_while:
    case N_do:
        switch (arg_node->node[0]->nodetype) {
        case N_cond:
        case N_while:
        case N_do:
        case N_let:
        case N_post:
        case N_pre:
        case N_return: {
            long *old_LINVAR;
            long *old_UBD_MAKE;
            int once_more;
            ids *ids_node;

            do {
                once_more = FALSE;
                /*
                 * Check if expession may moved above Loop
                 */
                arg_node = CheckUp (arg_node, arg_info);

                old_LINVAR = DupMask (LINVAR, VARNO);
                old_UBD_MAKE = DupMask (UBD_MAKE, VARNO);

                if (2 <= arg_node->nnode) {
                    /*
                     * traverse further nodes
                     */
                    arg_node->node[1] = Trav (arg_node->node[1], arg_info);
                } else /* end of assign-chain reached: */
                {
                    /*
                     * reset LIRVAR for all variables except
                     * variables used in do condition.
                     */
                    SetMask (LINVAR, TRUE, MAXVARNO);

                    if (N_with != LOOP_TYPE) {
                        for (i = 0; i < VARNO; i++) {
                            if ((0 != COND_USE[i])
                                && ((N_do == LOOP_TYPE) || (N_while == LOOP_TYPE)))
                                LINVAR[i] = FALSE;
                            if (TRUE == UBD[i])
                                UBD[i] = UNDEF;
                        }
                    }
                }

                if (MOVE_UP == MOVE) {
                    if (N_let == arg_node->node[0]->nodetype) {
                        ids_node = arg_node->node[0]->info.ids;
                        do {
                            i = ids_node->node->varno;
                            /*
                             * when the flag was set to MAYBE_UP the UBD_MAKE was set to
                             * TRUE, if the traversal retuns with UBD_MAKE set to FALSE
                             * this node may not be moved, and the traversal starts again
                             * whith his old masks for LINVAR and UBD_MAKE.
                             */
                            if (MAYBE_UP == ids_node->flag) {
                                if (FALSE == UBD_MAKE[i]) {
                                    ids_node->flag = NONE;
                                    once_more = TRUE;
                                    old_LINVAR[i] = FALSE;
                                } else {
                                    ids_node->flag = MOVE_UP;
                                }
                            }
                            ids_node = ids_node->next;
                        } while (NULL != ids_node);
                    } else {
                        for (i = 0; i < VARNO; i++) {
                            if ((MAYBE_UP == arg_node->node[0]->mask[3][i])
                                && (FALSE == UBD_MAKE[i])) {
                                once_more = TRUE;
                            }
                        }
                        if (once_more) {
                            FREE (arg_node->node[0]->mask[3]);
                            MOVE = NONE;
                            for (i = 0; i < VARNO; i++) {
                                if (0 < DEF[i]) {
                                    old_LINVAR[i] = FALSE;
                                }
                            }
                        }
                    }
                }

                if (TRUE == once_more) {
                    FREE (LINVAR);
                    LINVAR = old_LINVAR;
                    FREE (UBD_MAKE);
                    UBD_MAKE = old_UBD_MAKE;
                    DBUG_PRINT ("LIR", ("Once more in Line %d", arg_node->lineno));
                } else {
                    FREE (old_LINVAR);
                    FREE (old_UBD_MAKE);
                }
            } while (once_more);

            /*
             * Check if expession may moved below loop
             */
            arg_node = CheckDown (arg_node, arg_info);
        } break;
        default:
            ERROR2 (1, ("INTERNAL ERROR:"
                        " Assign %s not implemented in loop invariant removal !",
                        mdb_nodetype[arg_node->node[0]->nodetype]));
            break;
        }
        break;
    /******************/
    /* 2) conditional */
    /******************/
    case N_cond: {
        node *node_behind;
        long *used_vars;

        node_behind = NodeBehindCast (arg_node->node[0]->node[0]);
        used_vars = GetUsed (arg_node, node_behind);

        for (i = 0; i < VARNO; i++) {
            if (0 < used_vars[i]) {
                if (UNDEF == UBD[i]) {
                    UBD[i] = TRUE;
                }
            }
            if (0 < DEF[i]) {
                if (UNDEF == UBD[i]) {
                    UBD[i] = FALSE;
                }
                LINVAR[i] = FALSE;
            }
        }
        if (2 <= arg_node->nnode) {
            arg_node->node[1] = Trav (arg_node->node[1], arg_info);
        }
    } break;
    /******************/
    /* 3) top level   */
    /******************/
    default:
        if (2 <= arg_node->nnode) {
            arg_node->node[1] = Trav (arg_node->node[1], arg_info);
        }
        break;
    }
    DBUG_RETURN (arg_node);
}
