/*
 *
 * $Log$
 * Revision 1.3  1995/05/15 08:49:25  asi
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
#define UBD arg_info->mask[2] /* Variables used in loop before defined  */
#define LINVAR arg_info->mask[3]
#define COND_USE arg_info->mask[4]

#define MOVE arg_node->flag
#define ASSIGN_UP 2
#define EXPRESSION_UP 1
#define NONE 0
#define ASSIGN_DOWN -1
#define DONE -2

#define FALSE 0
#define TRUE 1
#define UNDEF -1

#define VAR "__lir"   /* name of new variable */
#define VAR_LENGTH 10 /* dimension for array of char */

#define UP arg_info->node[0]
#define DOWN arg_info->node[1]
#define TYPE arg_info->node[2]

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
LoopInvariantRemoval (node *arg_node, node *info_node)
{
    DBUG_ENTER ("LoopInvariantRemoval");
    act_tab = lir_tab;
    info_node = MakeNode (N_info);

    arg_node = Trav (arg_node, info_node);

    FREE (info_node);
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

    DBUG_PRINT ("OPT",
                ("Loop invariant removal in function: %s", arg_node->info.types->id));

    lir_expr_no = 0;
    arg_info->nodetype = N_fundef;
    VARNO = arg_node->varno;

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
    oldtype = arg_info->nodetype;

    switch (oldtype) {
    case N_with:
    case N_while:
    case N_do:
    case N_cond:
        oldmask[0] = LINVAR;
        oldmask[1] = UBD;
        oldmask[2] = COND_USE;
    default:
        switch (arg_node->nodetype) {
        case N_with:
            UBD = GenMask (VARNO); /* all variables automaticly set to FALSE */
            LINVAR = GenMask (VARNO);
            SetMask (LINVAR, TRUE, VARNO);
            for (i = 0; i < VARNO; i++) {
                /* All variables defined or used in generator are not loop invariant */
                if ((0 < arg_node->node[0]->mask[0][i])
                    || (0 < arg_node->node[0]->mask[1][i]))
                    LINVAR[i] = FALSE;
                /* If there are usages for genarray or modarray they are not loop */
                /* invariant too 							*/
                if ((NULL != arg_node->node[1]->mask[1])
                    && (0 < arg_node->node[1]->mask[1][i]))
                    LINVAR[i] = FALSE;
                /* The same for all variables defined twice in with body */
                if (1 < arg_node->mask[0][i])
                    LINVAR[i] = FALSE;
            }
            COND_USE = NULL;
            arg_info->nodetype = arg_node->nodetype;
            break;
        case N_while:
        case N_do:
            UBD = GenMask (VARNO); /* all variables automaticly set to FALSE */
            LINVAR = GenMask (VARNO);
            COND_USE = arg_node->mask[1];
            for (i = 0; i < VARNO; i++) {
                if (1 < ReadMask (arg_node->node[1]->mask[0], i))
                    LINVAR[i] = FALSE;
                else
                    LINVAR[i] = TRUE;

                if (0 == ReadMask (arg_node->node[1]->mask[0], i))
                    UBD[i] = FALSE;
                else
                    UBD[i] = UNDEF;
            }
            arg_info->nodetype = arg_node->nodetype;
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
        arg_node->mask[2] = UBD;
        UBD = oldmask[1];
        arg_info->nodetype = oldtype;
        break;
    default:
        switch (arg_node->nodetype) {
        case N_with:
        case N_while:
        case N_do:
            COND_USE = NULL;
            FREE (UBD);
            FREE (LINVAR);
            arg_info->nodetype = oldtype;
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

    DBUG_ENTER ("LIRMassign");
    DBUG_PRINT ("OPT",
                ("Travers assign - %s", mdb_nodetype[arg_node->node[0]->nodetype]));

    switch (arg_node->flag) {
    case ASSIGN_UP: /* move whole assignment above loop */

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
            UP = AppendNodeChain (1, UP, arg_node);
            arg_node = NULL;
        }
        break;

    case ASSIGN_DOWN: /* move whole assignment below loop */

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
            DOWN = AppendNodeChain (1, DOWN, arg_node);
            arg_node = NULL;
        }
        break;

    case EXPRESSION_UP: /* move only right hand side of let-expression and */
    {                   /* some definitions outside loop */
        node *last_node, *new_node, *new_vardec;
        ids *new_ids;

        /* make node that shall be moved outside the loop */
        move_node = MakeNode (N_assign);
        move_node->lineno = arg_node->lineno;
        move_node->node[0] = MakeNode (N_let);
        move_node->nnode = 1;
        move_node->node[0]->node[0] = arg_node->node[0]->node[0];
        move_node->node[0]->nnode = 1;
        arg_node->node[0]->nnode = 0;

        /* the moved node gets the same USE-mask as the original node */
        move_node->mask[1] = arg_node->mask[1];
        MinusMask (USE_IN, arg_node->mask[1], VARNO);
        arg_node->mask[1] = NULL;

        /* generate new mask's for moved node */
        move_node->mask[0] = GenMask (VARNO);

        /* initialize do loop */
        last_node = arg_node;
        do {
            switch (arg_node->node[0]->info.ids->flag) {
            case ASSIGN_UP:
                new_ids = arg_node->node[0]->info.ids;
                arg_node->node[0]->info.ids = new_ids->next;
                new_ids->next = NULL;
                move_node->node[0]->info.ids
                  = AppendIdsChain (move_node->node[0]->info.ids, new_ids);
                break;
            case ASSIGN_DOWN:
                /* make new node below loop */
                new_node = MakeNode (N_assign);
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
    long *tmp_mask[2];
    nodetype oldtype;

    DBUG_ENTER ("LIRcond");

    if (N_fundef == arg_info->nodetype) {
        arg_node = OptTrav (arg_node, arg_info, 1);
        arg_node = OptTrav (arg_node, arg_info, 2);
    } else {
        DBUG_EXECUTE ("LIR", char *text; text = PrintMask (LINVAR, VARNO);
                      printf ("LOOP INAVRIANT :%s\n", text); free (text);
                      text = PrintMask (UBD, VARNO);
                      printf ("USED BEFORE DEF:%s\n", text); free (text););
        DBUG_PRINT ("LIR", ("Cond Begin: line %d", arg_node->lineno));

        /*
         * set info-node's nodetype to remember in what expression we are
         */
        oldtype = arg_info->nodetype;
        arg_info->nodetype = N_cond;

        /*
         * Check if variables used in if condition are relative free
         * and prevent movements of definitions of these variables above
         * this condition.
         */
        for (i = 0; i < VARNO; i++) {
            if (0 < arg_node->mask[1][i]) {
                if (UNDEF == UBD[i])
                    UBD[i] = TRUE;
                LINVAR[i] = FALSE;
            }
        }

        old_mask[0] = DupMask (UBD, VARNO);
        old_mask[1] = DupMask (LINVAR, VARNO);

        /*
         * Traverse then part
         */
        arg_node = OptTrav (arg_node, arg_info, 1);

        tmp_mask[0] = UBD;
        tmp_mask[1] = LINVAR;

        UBD = old_mask[0];
        LINVAR = old_mask[1];

        /*
         * Traverse else part
         */
        arg_node = OptTrav (arg_node, arg_info, 2);

        for (i = 0; i < VARNO; i++) {
            if (TRUE == tmp_mask[0][i])
                UBD[i] = TRUE;
            if (FALSE == tmp_mask[1][i])
                LINVAR[i] = FALSE;
        }

        FREE (tmp_mask[0]);
        FREE (tmp_mask[1]);

        /*
         * reset info-node's nodetype
         */
        arg_info->nodetype = oldtype;

        DBUG_PRINT ("LIR", ("Cond End: line %d", arg_node->lineno));
        DBUG_EXECUTE ("LIR", char *text; text = PrintMask (LINVAR, VARNO);
                      printf ("LOOP INVARIANT :%s\n", text); free (text);
                      text = PrintMask (UBD, VARNO);
                      printf ("USED BEFORE DEF:%s\n", text); free (text););
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
    int term1 = TRUE, term2 = TRUE, term3 = TRUE, term4 = TRUE, term5, term6;
    int i, getvar;
    ids *ids_node;
    node *NodeBehind;

    DBUG_ENTER ("CheckUp");

    DBUG_EXECUTE ("LIR", char *text; text = PrintMask (LINVAR, VARNO);
                  printf ("\nLOOP INVARIANT :%s\n", text); free (text);
                  text = PrintMask (UBD, VARNO); printf ("USED BEFORE DEF:%s\n", text);
                  free (text););

    NodeBehind = NodeBehindCast (arg_node->node[0]->node[0]);

    if (N_with != NodeBehind->nodetype) {
        for (i = 0; i < VARNO; i++) {
            if (0 < USE[i]) {
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
                if (FALSE == LINVAR[i])
                    term3 = FALSE;
                if (TRUE == UBD[i])
                    term4 = FALSE;
            }
        }
    } else {
        for (i = 0; i < VARNO; i++) {
            if ((0 < NodeBehind->mask[2][i]) || (0 < NodeBehind->node[0]->mask[1][i])
                || ((0 == DEF[i]) && (0 < USE[i]))) {
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
                if (FALSE == LINVAR[i])
                    term3 = FALSE;
                if (TRUE == UBD[i])
                    term4 = FALSE;
            }
        }
    }

    if (term1 && term2 && term3 && term4) {
        MOVE = ASSIGN_UP;
    } else {
        if ((N_let == arg_node->node[0]->nodetype)) {
            switch (NodeBehind->nodetype) {
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

                        if (term5 && term6) {
                            ids_node->flag = ASSIGN_UP;
                        } else {
                            ids_node->flag = NONE;
                            LINVAR[i] = FALSE;
                        }
                        DBUG_PRINT ("LIR",
                                    ("Line %d VAR %d = %d, %d => %d", arg_node->lineno, i,
                                     term5, term6, ids_node->flag));
                        ids_node = ids_node->next;
                    } while (NULL != ids_node);
                    MOVE = EXPRESSION_UP;
                } else {
                    if (!getvar && print_warning) {
                        WARN1 (
                          ("WARNING: Not enough variables for loop invariant removal;\n"
                           "         use option -v no, with no > %d .\n",
                           optvar));
                        print_warning = FALSE;
                    }
                    for (i = 0; i < VARNO; i++) {
                        if ((0 < DEF[i]) || (0 < USE[i])) {
                            LINVAR[i] = FALSE;
                        }
                    }
                    MOVE = NONE;
                }
            default:
                break;
            }
        }
    }

    DBUG_PRINT ("LIR", ("Line %d = %d, %d, %d, %d => %d", arg_node->lineno, term1, term2,
                        term3, term4, MOVE));
    DBUG_EXECUTE ("LIR", char *text; text = PrintMask (LINVAR, VARNO);
                  printf ("LOOP INVARIANT :%s\n", text); free (text);
                  text = PrintMask (UBD, VARNO); printf ("USED BEFORE DEF:%s\n", text);
                  free (text););
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
    int i;
    ids *ids_node;

    DBUG_ENTER ("CheckDown");

    DBUG_EXECUTE ("LIR", char *text; text = PrintMask (LINVAR, VARNO);
                  printf ("\nLOOP INVARIANT :%s\n", text); free (text);
                  text = PrintMask (UBD, VARNO); printf ("USED BEFORE DEF:%s\n", text);
                  free (text););

    if (ASSIGN_UP == MOVE) {
        for (i = 0; i < VARNO; i++) {
            if (0 < DEF[i])
                LINVAR[i] = FALSE;
        }
    } else {
        for (i = 0; i < VARNO; i++) {
            if (0 < USE[i]) {
                if (FALSE == LINVAR[i])
                    term1 = FALSE;
            }
            if (0 < DEF[i]) {
                if (TRUE != LINVAR[i])
                    term2 = FALSE;
                if (TRUE == UBD[i])
                    term3 = FALSE;
            }
        }
        if (term1 && term2 && term3)
            MOVE = ASSIGN_DOWN;
        else {
            if (EXPRESSION_UP == MOVE) {
                ids_node = arg_node->node[0]->info.ids;
                do {
                    i = ids_node->node->varno;
                    if (FALSE == LINVAR[i])
                        term4 = FALSE;
                    else
                        term4 = TRUE;
                    if (TRUE == UBD[i])
                        term5 = FALSE;
                    else
                        term5 = TRUE;
                    if (term4 && term5) {
                        if (ASSIGN_UP != ids_node->flag)
                            ids_node->flag = ASSIGN_DOWN;
                    } else {
                        ids_node->flag = NONE;
                        if (TRUE == LINVAR[i]) {
                            LINVAR[i] = FALSE;
                        }
                    }
                    DBUG_PRINT ("LIR",
                                ("Line %d - VAR %d = %d, %d => %d", arg_node->lineno, i,
                                 term4, term5, ids_node->flag));
                    ids_node = ids_node->next;
                } while (NULL != ids_node);
            } else {
                for (i = 0; i < VARNO; i++) {
                    if (0 < DEF[i]) {
                        LINVAR[i] = FALSE;
                    }
                    if ((0 < USE[i]) && (TRUE == LINVAR[i])) {
                        LINVAR[i] = FALSE;
                    }
                }
            }
        }
    }
    DBUG_PRINT ("LIR", ("Line %d  = %d, %d, %d => %d", arg_node->lineno, term1, term2,
                        term3, MOVE));
    DBUG_EXECUTE ("LIR", char *text; text = PrintMask (LINVAR, VARNO);
                  printf ("LOOP INVARIANT :%s\n", text); free (text);
                  text = PrintMask (UBD, VARNO); printf ("USED BEFORE DEF:%s\n", text);
                  free (text););

    DBUG_RETURN (arg_node);
}

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
    node *new_node;
    long *oldmask[2];

    DBUG_ENTER ("LIRassign");

    DBUG_PRINT ("OPT",
                ("Travers assign - %s", mdb_nodetype[arg_node->node[0]->nodetype]));

    /********************************************/
    /* traverse into loops and add moved nodes:	*/
    /* 1) do loop				*/
    /* 2) while loop				*/
    /* 3) with loop				*/
    /********************************************/
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
                new_node->node[0]->node[0] = DupTree (arg_node->node[0]->node[0], NULL);
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
                  = AppendNodeChain (1, DOWN, new_node->node[0]->node[1]->node[0]);
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
        case N_let:
            if (N_with == (NodeBehindCast (arg_node->node[0]->node[0]))->nodetype) {
                arg_node->node[0] = Trav (arg_node->node[0], arg_info);
                act_tab = lir_mov_tab;
                arg_info->bblock = arg_node->bblock;

                arg_node = OptTrav (arg_node, arg_info, 0);

                arg_node->node[0]->flag = DONE;
                act_tab = lir_tab;
                PlusChainMasks (1, UP, arg_info);
                arg_node = AppendNodeChain (1, UP, arg_node);
                UP = NULL;
            }
            break;
        default:
            break;
        }
    } else {
        DBUG_PRINT ("LIR", ("Loop already done"));
    }

    /******************************************/
    /* consider in which expession we are:	*/
    /* 1) do or while loop			*/
    /* 2) conditional				*/
    /* 3) top level				*/
    /******************************************/
    switch (arg_info->nodetype) {
    /**************************/
    /* 1) do or while loop	  */
    /**************************/
    case N_with:
    case N_while:
    case N_do:
        /******************************************/
        /* check following subexpressions:	    */
        /* a) conditional			    */
        /* b) while loop			    */
        /* c) do loop				    */
        /* d) normal expr.			    */
        /******************************************/
        switch (arg_node->node[0]->nodetype) {
        /******************/
        /* a) conditional */
        /******************/
        case N_cond:
            /*
             * traverse further nodes
             */
            if (2 <= arg_node->nnode) {
                arg_node->node[1] = Trav (arg_node->node[1], arg_info);
            }
            break;
        /***********************/
        /* b) while loop       */
        /***********************/
        case N_while: {
            long *relfree;

            /* Calculate variables, which are relative in loop */
            relfree = DupMask (arg_node->node[0]->mask[2], VARNO); /* UBD in loop && */
            AndMask (relfree, arg_node->node[0]->mask[1], VARNO);  /* condition of loop */
            for (i = 0; i < VARNO; i++) {
                /* && variables not defined in loop but used */
                if ((0 == DEF[i]) && (0 < arg_node->node[0]->node[1]->mask[1][i]))
                    relfree[i] = TRUE;

                /* calculate new variables, which are used before defined in outer loop */
                /* and mark all variables defined in inner loop as no further movable */
                /* above the outer loop */
                if ((UNDEF == UBD[i]) && (TRUE == relfree[i]))
                    UBD[i] = TRUE;
                if (0 < DEF[i]) {
                    if (UNDEF == UBD[i])
                        UBD[i] = FALSE;
                    LINVAR[i] = FALSE;
                }
            }
            FREE (relfree);

            if (2 <= arg_node->nnode) {
                /*
                 * traverse further nodes
                 */
                arg_node->node[1] = Trav (arg_node->node[1], arg_info);
            } else {
                /*
                 * end of assign-chain reached:
                 */

                /*
                 * reset LIRVAR for all variables except
                 * variables used in do condition.
                 */
                SetMask (LINVAR, TRUE, VARNO);

                for (i = 0; i < VARNO; i++) {
                    if (0 != COND_USE[i])
                        LINVAR[i] = FALSE;
                }
            }

            /* variables defined in inner loop, */
            /* are no further movable below outer loop */
            for (i = 0; i < VARNO; i++) {
                if (0 < DEF[i])
                    LINVAR[i] = FALSE;
            }
        } break;
            /***********************/
            /* c) do loop          */
            /***********************/
            {
                long *relfree;

                /* Calculate variables, which are relative in loop */
                relfree
                  = DupMask (arg_node->node[0]->mask[2], VARNO); /* UBD in loop && */
                for (i = 0; i < VARNO; i++) {
                    /* && variables not defined in loop but used in loop or conditional */
                    if ((0 == DEF[i])
                        && ((0 < arg_node->node[0]->node[1]->mask[1][i])
                            || (0 < arg_node->node[0]->mask[1][i])))
                        relfree[i] = TRUE;

                    /* calculate new variables, which are used before defined in outer
                     * loop */
                    /* and mark all variables defined in inner loop as no further movable
                     */
                    /* above the outer loop */
                    if ((UNDEF == UBD[i]) && (TRUE == relfree[i]))
                        UBD[i] = TRUE;
                    if (0 < DEF[i]) {
                        if (UNDEF == UBD[i])
                            UBD[i] = FALSE;
                        LINVAR[i] = FALSE;
                    }
                }
                FREE (relfree);

                if (2 <= arg_node->nnode) {
                    /*
                     * traverse further nodes
                     */
                    arg_node->node[1] = Trav (arg_node->node[1], arg_info);
                } else {
                    /*
                     * end of assign-chain reached:
                     */

                    /*
                     * reset LIRVAR for all variables except
                     * variables used in do condition.
                     */
                    SetMask (LINVAR, TRUE, VARNO);

                    for (i = 0; i < VARNO; i++) {
                        if (0 != COND_USE[i])
                            LINVAR[i] = FALSE;
                    }
                }

                /* variables defined in inner loop, */
                /* are no further movable below outer loop */
                for (i = 0; i < VARNO; i++) {
                    if (0 < DEF[i])
                        LINVAR[i] = FALSE;
                }
            }
            break;
        /***********************/
        /* d) let expr.        */
        /***********************/
        case N_let: {
            /*
             * Check if expession may moved above Loop
             */
            arg_node = CheckUp (arg_node, arg_info);

            if (2 <= arg_node->nnode) {
                /*
                 * traverse further nodes
                 */
                arg_node->node[1] = Trav (arg_node->node[1], arg_info);
            } else {
                /*
                 * end of assign-chain reached:
                 */

                /*
                 * reset LIRVAR for all variables except
                 * variables used in do condition.
                 */
                SetMask (LINVAR, TRUE, VARNO);

                if (N_with != arg_info->nodetype) {
                    for (i = 0; i < VARNO; i++) {
                        if (0 != COND_USE[i])
                            LINVAR[i] = FALSE;
                    }
                }
            }
            /*
             * Check if expession may moved below loop
             */
            if (N_with != arg_info->nodetype) {
                arg_node = CheckDown (arg_node, arg_info);
            }
        } break;
        default:
            break;
        }
        break;
    /******************/
    /* 2) conditional */
    /******************/
    case N_cond:
        for (i = 0; i < VARNO; i++) {
            if (0 < USE[i]) {
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
        break;
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
