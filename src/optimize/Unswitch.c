/*
 *
 * $Log$
 * Revision 1.1  1995/07/07 13:40:15  asi
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
#include "access_macros.h"
#include "internal_lib.h"

#include "optimize.h"
#include "ConstantFolding.h"
#include "DupTree.h"
#include "Unroll.h"
#include "Unswitsh.h"

#define FALSE 0
#define TRUE 1

/*
 *
 *  functionname  : Unswitch
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
Unswitch (node *arg_node, node *arg_info)
{
    funptr *tmp_tab;

    DBUG_ENTER ("Unswitch");
    tmp_tab = act_tab;
    act_tab = unswitch_tab;
    arg_info = MakeNode (N_info);
    def_stack = MAlloc (sizeof (stack));
    def_stack->tos = -1;
    def_stack->st_len = MIN_STACK_SIZE;
    def_stack->stack = (stelm *)MAlloc (sizeof (stelm) * MIN_STACK_SIZE);

    arg_node = Trav (arg_node, arg_info);

    FREE (arg_info);
    FREE (def_stack->stack);
    FREE (def_stack);
    act_tab = tmp_tab;
    DBUG_RETURN (arg_node);
}
/*
 *
 *  functionname  : UNSfundef
 *  arguments     : 1) fundef-node
 *                  2) NULL
 *                  R) fundef-node with unrolled loops in body of function
 *  description   : - generates info_node
 *                  - varno of the info_node will be set to the number of local variables
 *                      and arguments in this functions
 *                  - new entry will be pushed on the def_stack
 *                  - generates two masks and links them to the info_node
 *                      [0] - variables additional defined in function and
 *                      [1] - variables additional used in function after optimization
 *                  - calls Trav to  unswitch loops in body of current function
 *                  - last entry will be poped from def_stack
 *                  - updates masks in fundef node.
 *  global vars   : syntax_tree, def_stack
 *  internal funs : PushVL, PopVL
 *  external funs : GenMask, MinusMask, OptTrav
 *  macros        : DBUG...
 *
 *  remarks       : --
 *
 */
node *
UNSfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("UNSfundef");

    DBUG_PRINT ("UNS", ("Unswitch in function: %s", arg_node->info.types->id));
    VARNO = arg_node->varno;
    SEARCH = FALSE;
    COND_ID = -1;
    PREV_DEF = GenMask (VARNO);
    PushVL (arg_info->varno);

    arg_node = OptTrav (arg_node, arg_info, 0); /* functionbody */

    PopVL ();
    FREE (PREV_DEF);

    arg_node = OptTrav (arg_node, arg_info, 1); /* next function */
    DBUG_RETURN (arg_node);
}

node *
UNSlet (node *arg_node, node *arg_info)
{
    node *arg1;
    node *arg2;
    ids *ids_node;

    DBUG_ENTER ("UNSlet");
    arg_node = OptTrav (arg_node, arg_info, 0); /* maybe trav with-loop */

    if ((COND_ID == arg_node->IDS_NODE->varno)
        && (N_prf == arg_node->node[0]->nodetype)) {
        COND_TEST = arg_node->node[0];
        arg1 = arg_node->node[0]->ARG1;
        arg2 = arg_node->node[0]->ARG2;

        if ((N_id == arg1->nodetype) && (1 == ReadMask (PREV_DEF, arg1->IDS_VARNO)))
            COND_MOD = VAR (arg1->IDS_NODE->varno);
        else {
            if ((N_id == arg2->nodetype) && (1 == ReadMask (PREV_DEF, arg2->IDS_VARNO)))
                COND_MOD = VAR (arg2->IDS_NODE->varno);
        }
    }

    ids_node = arg_node->info.ids;
    while (NULL != ids_node) /* determine defined variables */
    {
        VAR (ids_node->node->varno) = arg_node->node[0];
        INC_VAR (PREV_DEF, ids_node->node->varno);
        ids_node = ids_node->next;
    }
    DBUG_RETURN (arg_node);
}

node *
UNSassign (node *arg_node, node *arg_info)
{
    node *unroll = NULL, *tmp;
    int i;

    DBUG_ENTER ("UNSassign");

    /* unswitch subexpressions */
    arg_node = OptTrav (arg_node, arg_info, 0);

    switch (arg_node->node[0]->nodetype) {
    case N_while:
    case N_do:
        break;
    default:
        break;
    }

    /* next assign node */
    if (1 < arg_node->nnode) {
        arg_node->node[1] = Trav (arg_node->node[1], arg_info);
    }
    DBUG_RETURN (arg_node);
}

node *
UNSdo (node *arg_node, node *arg_info)
{
    int i;
    linfo *loop_info;
    node *cond_node;
    int old_SEARCH;
    int old_COND_ID;
    node *old_COND_TEST;
    node *old_COND_MOD;
    long *old_PREV_DEF;

    DBUG_ENTER ("UNSdo");

    PushDupVL (VARNO);

    for (i = 0; i < TOS.vl_len; i++) {
        if (ReadMask (arg_node->node[1]->mask[0], i) != 0) {
            VAR (i) = NULL;
        }
    }

    cond_node = arg_node->node[0];
    switch (cond_node->nodetype) {
    case N_id:
        old_PREV_DEF = PREV_DEF;
        old_SEARCH = SEARCH;
        old_COND_ID = COND_ID;
        old_COND_TEST = COND_TEST;
        old_COND_MOD = COND_MOD;

        PREV_DEF = GenMask (VARNO);
        SEARCH = TRUE;
        COND_ID = cond_node->IDS_NODE->varno;
        COND_TEST = NULL;
        COND_MOD = NULL;
        break;
    default:
        break;
    }

    DBUG_PRINT ("UNS", ("Trav do loop in line %d", arg_node->lineno));
    arg_node = OptTrav (arg_node, arg_info, 1); /* Trav do-body */

    loop_info = AnalyseLoop (COND_TEST, COND_MOD, N_do);
    if ((NULL != loop_info) && (loop_info->loop_num <= unrnum)) {
        FREE (loop_info);
    }

    switch (cond_node->nodetype) {
    case N_id:
        FREE (PREV_DEF);
        PREV_DEF = old_PREV_DEF;
        SEARCH = old_SEARCH;
        COND_ID = old_COND_ID;
        COND_TEST = old_COND_TEST;
        COND_MOD = old_COND_MOD;
        break;
    default:
        break;
    }

    PopVL2 ();
    DBUG_RETURN (arg_node);
}

node *
UNSwhile (node *arg_node, node *arg_info)
{
    int i;
    linfo *loop_info;
    node *cond_node;
    int old_SEARCH;
    int old_COND_ID;
    node *old_COND_TEST;
    node *old_COND_MOD;
    long *old_PREV_DEF;

    DBUG_ENTER ("UNSwhile");
    PushVL (VARNO);
    for (i = 0; i < TOS.vl_len; i++) {
        if (ReadMask (arg_node->node[1]->mask[0], i) != 0) {
            VAR (i) = NULL;
        }
    }

    cond_node = arg_node->node[0];
    switch (cond_node->nodetype) {
    case N_id:
        old_PREV_DEF = PREV_DEF;
        old_SEARCH = SEARCH;
        old_COND_ID = COND_ID;
        old_COND_TEST = COND_TEST;
        old_COND_MOD = COND_MOD;

        PREV_DEF = GenMask (VARNO);
        SEARCH = TRUE;
        COND_ID = cond_node->IDS_NODE->varno;
        COND_TEST = NULL;
        COND_MOD = NULL;
        break;
    default:
        break;
    }

    DBUG_PRINT ("UNS", ("Trav while loop in line %d", arg_node->lineno));
    arg_node = OptTrav (arg_node, arg_info, 1); /* Trav do-body */

    loop_info = AnalyseLoop (COND_TEST, COND_MOD, N_while);
    if ((NULL != loop_info) && (loop_info->loop_num <= unrnum)) {
        FREE (loop_info);
    }

    switch (cond_node->nodetype) {
    case N_id:
        FREE (PREV_DEF);
        PREV_DEF = old_PREV_DEF;
        SEARCH = old_SEARCH;
        COND_ID = old_COND_ID;
        COND_TEST = old_COND_TEST;
        COND_MOD = old_COND_MOD;
        break;
    default:
        break;
    }

    PopVL ();
    for (i = 0; i < TOS.vl_len; i++) {
        if (ReadMask (arg_node->node[1]->mask[0], i) != 0) {
            VAR (i) = NULL;
        }
    }
    DBUG_RETURN (arg_node);
}

node *
UNScond (node *arg_node, node *arg_info)
{
    int i;

    DBUG_ENTER ("UNScond");
    PushDupVL ();

    /* Trav then */
    arg_node = OptTrav (arg_node, arg_info, 1);

    PopVL ();
    PushDupVL ();

    /* Trav else */
    arg_node = OptTrav (arg_node, arg_info, 2);

    PopVL ();
    for (i = 0; i < TOS.vl_len; i++) {
        if ((ReadMask (arg_node->node[1]->mask[0], i) != 0)
            || (ReadMask (arg_node->node[2]->mask[0], i) != 0)) {
            VAR (i) = NULL;
        }
    }
    DBUG_RETURN (arg_node);
}

node *
UNSwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("UNSwith");
    PushDupVL ();

    /* Trav with-body */
    arg_node = OptTrav (arg_node, arg_info, 2);

    PopVL ();
    DBUG_RETURN (arg_node);
}
