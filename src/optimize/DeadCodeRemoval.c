/*
 *
 * $Log$
 * Revision 1.1  1995/02/13 16:37:42  asi
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

#include "optimize.h"
#include "DeadCodeRemoval.h"

/*
 *
 *  functionname  : DeadCodeRemoval
 *  arguments     : 1) ptr to root of the syntaxtree
 *                  R) ptr to root of the optimized syntaxtree
 *  description   : Dead Code Removal
 *  global vars   : syntax_tree, dead_tab
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
    act_tab = dead_tab;
    info_node = MakeNode (N_info);
    arg_node = Trav (arg_node, info_node);
    free (info_node);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : DEADfundef
 *  arguments     : 1) ptr to fundef-node
 *                  2) ptr to info_node
 *                  R) ptr to fundef-node 1) with removed dead code and dead local
 *                     variable removal in its subtrees.
 *  description   : first call Trav to collect informations about used vaiables in the
 *                  further controll flow, then call Trav to remove dead code from the
 *                  syntax tree. Last but not least remove local variables, which not
 *                  used yet.
 *  global vars   : syntax_tree, info_node
 *  internal funs : ---
 *  external funs : Trav, GenMask, free
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
node *
DEADfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DEADfundef");
    DBUG_PRINT ("DEAD", ("Optimizing function: %s", arg_node->info.types->id));
    arg_info->nodetype = N_fundef;
    if (arg_node->node[0] != NULL) {
        arg_info->lineno = 0;
        arg_info->mask[0] = GenMask ();
        arg_info->mask[1] = GenMask ();
        arg_info->mask[2] = GenMask ();
        arg_node->node[0]
          = Trav (arg_node->node[0], arg_info); /* Trav body of function */
        free (arg_info->mask[0]);               /* Gathering loop infos */
        free (arg_info->mask[1]);
        free (arg_info->mask[2]);

        arg_info->lineno = 1;
        arg_info->mask[0] = GenMask ();
        arg_info->mask[1] = GenMask ();
        arg_info->mask[2] = GenMask ();
        arg_node->node[0]
          = Trav (arg_node->node[0], arg_info);           /* Trav body of function */
        MinusMask (arg_node->mask[0], arg_info->mask[0]); /* with Dead-Code-Removal */
        MinusMask (arg_node->mask[1], arg_info->mask[1]);
        free (arg_info->mask[0]);
        free (arg_info->mask[1]);
        free (arg_info->mask[2]);

        if (arg_node->node[0]->node[1] != NULL) /* Dead-Variable-Removal */
        {
            arg_info->mask[1] = arg_node->mask[1];
            arg_node->node[0]->node[1] = Trav (arg_node->node[0]->node[1], arg_info);
            if (arg_node->node[0]->node[1] == NULL)
                arg_node->node[0]->nnode--;
            arg_info->mask[1] = NULL;
        }
    }
    if (NULL != arg_node->node[1]) /* next function */
        arg_node->node[1] = Trav (arg_node->node[1], arg_info);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : DEADvardec
 *  arguments     : 1) ptr to vardec-node
 *                  2) ptr to info_nodei
 *                  R) ptr to current node if variable is used in function
 *                     or ptr to next vardec-node if not used.
 *  description   : removes variable decleration if not used
 *  global vars   : syntax_tree, info_node
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
node *
DEADvardec (node *arg_node, node *arg_info)
{
    node *return_node;

    DBUG_ENTER ("DEADvardec");
    return_node = arg_node;
    if (NULL != arg_node->node[0]) {
        arg_node->node[0] = Trav (arg_node->node[0], arg_info);
        if (arg_node->node[0] == NULL)
            arg_node->nnode--;
    }
    if (arg_info->mask[1][arg_node->lineno] == 0) {
        deadvar++;
        DBUG_PRINT ("DEAD",
                    ("Variable decleration %s removed", arg_node->info.types->id));
        return_node = arg_node->node[0];
    }
    DBUG_RETURN (return_node);
}

/*
 *
 *  functionname  : DEADassign
 *  arguments     : 1) ptr to assign-node
 *                  2) ptr to info_node
 *                  R) modified assign_node 1) or next assign-node if whole assign is
 *                     not important for sac-programm.
 *  description   : if arg_info->lineno == 0 this function gathers informations about
 *                  used variables in the further controll flow,
 *                  otherwise the function performs dead code removal with this
 * assignment. global vars   : syntax_tree, info_node, optno internal funs : --- external
 * funs : Trav, PlusMask, OrMask, MinusMask, DupMask, GenMask, CheckMask, If3_2Mask, free
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
node *
DEADassign (node *arg_node, node *arg_info)
{
    node *return_node;
    long *oldmask[2];
    nodetype olditype;

    DBUG_ENTER ("DEADassign");
    olditype = arg_info->nodetype;
    return_node = arg_node;
    if (NULL != arg_node->node[1]) {
        arg_node->node[1] = Trav (arg_node->node[1], arg_info); /* Trav next assign */
        if (arg_node->node[1] == NULL)
            arg_node->nnode = 1;
    }

    switch (arg_node->node[0]->nodetype) {
    case N_return:
        DBUG_EXECUTE ("DEAD", if ((arg_info->lineno) == 1) arg_node->mask[2]
                              = DupMask (arg_info->mask[2]););
        OrMask (arg_info->mask[2], arg_node->mask[1]);
        break;
    case N_do:
    case N_while:
        if (arg_info->lineno != 0) {
            if (CheckMask (arg_info->mask[2], arg_node->mask[0])) {
                DBUG_PRINT ("DEAD", ("Removed node: %08x in line %d", arg_node,
                                     arg_node->lineno));
                PlusMask (arg_info->mask[0], arg_node->mask[0]);
                PlusMask (arg_info->mask[1], arg_node->mask[1]);
                if (olditype != N_fundef)
                    MinusMask (arg_info->mask[2], arg_node->mask[1]);
                return_node = arg_node->node[1]; /* remove this assignment */
                arg_node->node[1] = NULL;
                arg_node->nnode = 1;
                FreeTree (arg_node);
                optno++;
            } else {
                olditype = arg_node->node[0]->nodetype;
                oldmask[2] = DupMask (arg_info->mask[2]);
                oldmask[0] = arg_info->mask[0]; /* Save old dead def. variables */
                arg_info->mask[0] = GenMask ();
                oldmask[1] = arg_info->mask[1]; /* Save old dead used variables */
                arg_info->mask[1] = GenMask ();
                OrMask (arg_info->mask[2], arg_node->mask[2]);
                if (olditype == N_do)
                    OrMask (arg_info->mask[2], arg_node->node[0]->mask[1]);

                arg_node->node[0] = Trav (arg_node->node[0], arg_info);

                /* consider removed definitions in current and previous def_mask */
                MinusMask (arg_node->mask[0], arg_info->mask[0]);
                PlusMask (arg_info->mask[0], oldmask[0]);
                free (oldmask[0]);
                /* consider removed usages in current and previous used_mask */
                MinusMask (arg_node->mask[1], arg_info->mask[1]);
                PlusMask (arg_info->mask[1], oldmask[1]);
                free (oldmask[1]);
                if (olditype != N_do)
                    OrMask (arg_info->mask[2], oldmask[2]);
                if (olditype == N_while)
                    OrMask (arg_info->mask[2], arg_node->node[0]->mask[1]);
                DBUG_EXECUTE ("DEAD", oldmask[1] = DupMask (arg_node->mask[2]););
                free (arg_node->mask[2]);
                arg_node->mask[2] = NULL;
                DBUG_EXECUTE ("DEAD", arg_node->mask[2] = oldmask[1];);
            }
        } else {
            oldmask[2] = DupMask (arg_info->mask[2]);
            arg_node->node[0] = Trav (arg_node->node[0], arg_info);
            OrMask (arg_info->mask[2], oldmask[2]);
            arg_node->mask[2] = DupMask (arg_info->mask[2]);
        }
        break;
    case N_cond:
        if (CheckMask (arg_info->mask[2], arg_node->mask[0]) && arg_info->lineno) {
            DBUG_PRINT ("DEAD",
                        ("Removed node: %08x in line %d", arg_node, arg_node->lineno));
            PlusMask (arg_info->mask[0], arg_node->mask[0]);
            PlusMask (arg_info->mask[1], arg_node->mask[1]);
            if (olditype != N_fundef)
                MinusMask (arg_info->mask[2], arg_node->mask[1]);
            return_node = arg_node->node[1]; /* remove this assignment */
            arg_node->node[1] = NULL;
            arg_node->nnode = 1;
            FreeTree (arg_node);
            optno++;
        } else {
            DBUG_EXECUTE ("DEAD", if ((arg_info->lineno) == 1) arg_node->mask[2]
                                  = DupMask (arg_info->mask[2]););
            DBUG_ASSERT ((arg_node->node[0] != NULL), "While-expr. without body.");
            arg_node->node[0] = Trav (arg_node->node[0], arg_info);
        }
        break;
    default:
        if (CheckMask (arg_info->mask[2], arg_node->mask[0]) && arg_info->lineno) {
            DBUG_PRINT ("DEAD",
                        ("Removed node: %08x in line %d", arg_node, arg_node->lineno));
            PlusMask (arg_info->mask[0], arg_node->mask[0]);
            PlusMask (arg_info->mask[1], arg_node->mask[1]);
            if (olditype != N_fundef)
                MinusMask (arg_info->mask[2], arg_node->mask[1]);
            return_node = arg_node->node[1]; /* remove this assignment */
            arg_node->node[1] = NULL;
            arg_node->nnode = 1;
            FreeTree (arg_node);
            optno++;
        } else {
            DBUG_EXECUTE ("DEAD", if ((arg_info->lineno) == 1) arg_node->mask[2]
                                  = DupMask (arg_info->mask[2]););
            If3_2Mask (arg_info->mask[2], arg_node->mask[1], arg_node->mask[0]);
        }
    }
    DBUG_RETURN (return_node);
}

/*
 *
 *  functionname  : DEADloop
 *  arguments     : 1) ptr to while-node
 *                  2) ptr to info_node
 *                  R) not modified 1)
 *  description   : traverse only body of loop.
 *  global vars   : syntax_tree, info_node
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
node *
DEADloop (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DEADloop");
    DBUG_ASSERT ((arg_node->node[1] != NULL), "While-expr. without body.");
    arg_node->node[1] = Trav (arg_node->node[1], arg_info);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : DEADcond
 *  arguments     : 1) ptr to cond-node
 *                  2) ptr to info_node
 *                  R) node 1) without dead code in subtrees then and else.
 *  description   : if arg_info->lineno == 0 information gathering phase, see DEADassign.
 *                  otherwise delete whole then- or else- subtrees or traverse into it.
 *  global vars   : syntax_tree, info_node
 *  internal funs : ---
 *  external funs : Trav, PlusMask, GenMask
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
node *
DEADcond (node *arg_node, node *arg_info)
{
    long *oldmask[3];

    DBUG_ENTER ("DEADcond");
    if (arg_info->lineno == 0) {
        oldmask[2] = DupMask (arg_info->mask[2]);
        oldmask[3] = DupMask (arg_info->mask[2]);
        arg_node->node[1] = Trav (arg_node->node[1], arg_info); /* Trav then-subtree */
        OrMask (oldmask[3], arg_info->mask[2]);
        arg_info->mask[2] = oldmask[2];
        arg_node->node[2] = Trav (arg_node->node[2], arg_info); /* Trav else-subtree */
        OrMask (arg_info->mask[2], oldmask[3]);
        free (oldmask[3]);
        OrMask (arg_info->mask[2], arg_node->mask[1]);
    } else {
        oldmask[3] = DupMask (arg_info->mask[2]);
        if (arg_node->node[1] != NULL) {
            if (MaskIsNotZero (arg_node->node[1]->mask[0])
                && CheckMask (arg_info->mask[2], arg_node->node[1]->mask[0])) {
                FreeTree (arg_node->node[1]->node[0]);
                arg_node->node[1]->node[0] = MakeNode (N_empty);
                PlusMask (arg_info->mask[0], arg_node->node[1]->mask[0]);
                PlusMask (arg_info->mask[1], arg_node->node[1]->mask[1]);
                MinusMask (arg_node->node[1]->mask[0], arg_node->node[1]->mask[0]);
                MinusMask (arg_node->node[1]->mask[1], arg_node->node[1]->mask[1]);
                DBUG_EXECUTE ("DEAD", arg_node->node[1]->mask[2] = DupMask (oldmask[3]););
                optno++;
            } else {
                /* store later used variables in node */
                DBUG_EXECUTE ("DEAD", arg_node->node[1]->mask[2] = DupMask (oldmask[3]););
                /* save old dead def. variables */
                oldmask[0] = arg_info->mask[0];
                /* and gather a new set */
                arg_info->mask[0] = GenMask ();
                /* save old dead used variables */
                oldmask[1] = arg_info->mask[1];
                /* and gather a new set */
                arg_info->mask[1] = GenMask ();

                arg_node->node[1]
                  = Trav (arg_node->node[1], arg_info); /* Trav then-subtree */

                /* consider removed definitions in current and lokal def_mask */
                MinusMask (arg_node->node[1]->mask[0], arg_info->mask[0]);
                PlusMask (arg_info->mask[0], oldmask[0]);
                free (oldmask[0]);
                /* consider removed usages in current and lokal used_mask */
                MinusMask (arg_node->node[1]->mask[1], arg_info->mask[1]);
                PlusMask (arg_info->mask[1], oldmask[1]);
                free (oldmask[1]);
                /* save later used variables for then part of the condition */
                oldmask[2] = arg_info->mask[2];
            }
        }
        if (arg_node->node[2] != NULL) {
            /* later used vaiables are the same in the else- as in the then-subtree */
            arg_info->mask[2] = DupMask (oldmask[3]);
            if (MaskIsNotZero (arg_node->node[2]->mask[0])
                && CheckMask (arg_info->mask[2], arg_node->node[2]->mask[0])) {
                FreeTree (arg_node->node[2]->node[0]);
                arg_node->node[2]->node[0] = MakeNode (N_empty);
                PlusMask (arg_info->mask[0], arg_node->node[2]->mask[0]);
                PlusMask (arg_info->mask[1], arg_node->node[2]->mask[1]);
                MinusMask (arg_node->node[2]->mask[0], arg_node->node[2]->mask[0]);
                MinusMask (arg_node->node[2]->mask[1], arg_node->node[2]->mask[1]);
                DBUG_EXECUTE ("DEAD", arg_node->node[2]->mask[2] = DupMask (oldmask[3]););
                optno++;
            } else {
                /* store later used variables in node */
                DBUG_EXECUTE ("DEAD", arg_node->node[2]->mask[2] = DupMask (oldmask[3]););
                /* Save old dead def. variables */
                oldmask[0] = arg_info->mask[0];
                /* and gather a new set */
                arg_info->mask[0] = GenMask ();
                /* Save old dead used variables */
                oldmask[1] = arg_info->mask[1];
                /* and gather a new set */
                arg_info->mask[1] = GenMask ();

                arg_node->node[2]
                  = Trav (arg_node->node[2], arg_info); /* Trav else-subtree */

                /* consider removed definitions in current and previous def_mask */
                MinusMask (arg_node->node[2]->mask[0], arg_info->mask[0]);
                PlusMask (arg_info->mask[0], oldmask[0]);
                free (oldmask[0]);
                /* consider removed usages in current and previous used_mask */
                MinusMask (arg_node->node[2]->mask[1], arg_info->mask[1]);
                PlusMask (arg_info->mask[1], oldmask[1]);
                free (oldmask[1]);
            }
        }
        if (arg_node->node[1] != NULL) {
            OrMask (arg_info->mask[2], oldmask[2]);
            free (oldmask[2]);
        }
        DBUG_EXECUTE ("DEAD", arg_node->mask[2] = DupMask (arg_info->mask[2]););
        OrMask (arg_info->mask[2], arg_node->mask[1]);
        OrMask (arg_info->mask[2], oldmask[3]);
        free (oldmask[3]);
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : DEADblock
 *  arguments     : 1) ptr to block-node
 *                  2) ptr to info-node
 *                  R) not modified 1)
 *  description   : Only traverse body of the block
 *  global vars   : syntax_tree, info_node
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
node *
DEADblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("OPTblock");
    if (arg_node->node[0] != NULL)
        arg_node->node[0] = Trav (arg_node->node[0], arg_info); /* Dead-Code-Removal */
    DBUG_RETURN (arg_node);
}

            