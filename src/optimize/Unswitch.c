/*
 *
 * $Log$
 * Revision 1.3  1995/07/12 15:24:43  asi
 * added UNSid and WhereUnswitch
 * pointers to varables definitions added
 *
 * Revision 1.2  1995/07/07  14:58:38  asi
 * added loop unswitching
 *
 * Revision 1.1  1995/07/07  13:40:15  asi
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
#include "Unswitch.h"

#define FALSE 0
#define TRUE 1
#define COND arg_info->node[0]
#define LOOP_INFO arg_info->node[1]

typedef enum { undef, first, medium, last } position;

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
 *                  R) fundef-node with unswitched loops in body of function
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
    PushVL (arg_info->varno);
    LEVEL = 1;

    arg_node = OptTrav (arg_node, arg_info, 0); /* functionbody */

    PopVL ();

    arg_node = OptTrav (arg_node, arg_info, 1); /* next function */
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : UNSid
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
UNSid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("UNSid");
    arg_node->flag = LEVEL;
    DBUG_PRINT ("UNS",
                ("%s defined at %06x", arg_node->IDS_ID, VAR (arg_node->IDS_VARNO)));
    arg_node->IDS_DEF = VAR (arg_node->IDS_VARNO);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : UNSlet
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
UNSlet (node *arg_node, node *arg_info)
{
    ids *ids_node;

    DBUG_ENTER ("UNSlet");
    /* genrate defined-pointer and maybe trav with-loop */
    arg_node = OptTrav (arg_node, arg_info, 0);

    arg_node->node[0]->flag = LEVEL;

    ids_node = arg_node->info.ids;
    while (NULL != ids_node) /* determine defined variables */
    {
        VAR (ids_node->node->varno) = arg_node->node[0];
        ids_node = ids_node->next;
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : GetPosition
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
position
GetPosition (int num, prf test_prf, linfo *loop_info)
{
    position pos = undef;

    DBUG_ENTER ("GetPosition");

    DBUG_RETURN (pos);
}
/*
 *
 *  functionname  : WhereUnswitch
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
position
WhereUnswitch (linfo *loop_info, node *cond_node, int level)
{
    node *test, *arg[2];
    prf test_prf;
    position pos = undef;

    DBUG_ENTER ("DoUnswitch");
    if (NULL != loop_info) {
        if (N_id == cond_node->nodetype) {
            test = cond_node->IDS_DEF;
            if ((NULL != test) && (N_prf == test->nodetype)
                && (F_le <= (test_prf = test->info.prf)) && (F_neq >= test_prf)
                && (test->flag == level))
                /* the constant value shall be on the right side of the expression */
                /* i.e. cond = Num op i will be changed to cond = i op Num         */
                if (IsConst (test->ARG2)) {
                    arg[0] = test->ARG1;
                    arg[1] = test->ARG2;
                } else {
                    test_prf = InversePrf (test_prf);
                    arg[0] = test->ARG2;
                    arg[1] = test->ARG1;
                }
            if ((N_id == arg[0]->nodetype) && (loop_info->varno == arg[0]->IDS_VARNO)
                && (N_num == arg[1]->nodetype)) {
                pos = GetPosition (arg[1]->info.cint, test_prf, loop_info);
            }
        }
    }
    DBUG_RETURN (pos);
}

/*
 *
 *  functionname  : DoUnswitch
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
DoUnswitch (node *arg_node, node *arg_info, position pos)
{
    DBUG_ENTER ("DoUnswitch");
    switch (pos) {
    case first:
        break;
    case medium:
        cond_node break;
    case last:
        break;
    default:
        break;
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : UNSassign
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
UNSassign (node *arg_node, node *arg_info)
{
    position pos;

    DBUG_ENTER ("UNSassign");

    /* unswitch subexpressions */
    arg_node = OptTrav (arg_node, arg_info, 0);

    switch (arg_node->node[0]->nodetype) {
    case N_cond:
        COND = arg_node;
        break;
    case N_do:
        pos = WhereUnswitch ((linfo *)LOOP_INFO, COND->node[0]->node[0], LEVEL);
        if (undef != pos)
            arg_node = DoUnswitch (arg_node, arg_info, pos);
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

/*
 *
 *  functionname  : UNSdo
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
UNSdo (node *arg_node, node *arg_info)
{
    int i;
    linfo *loop_info;
    node *cond_node;

    DBUG_ENTER ("UNSdo");
    PushDupVL (VARNO);

    LEVEL++;

    for (i = 0; i < TOS.vl_len; i++) {
        if (1 < ReadMask (arg_node->node[1]->mask[0], i)) {
            VAR (i) = NULL;
        }
    }

    COND = NULL;

    DBUG_PRINT ("UNR", ("Trav do loop in line %d", arg_node->lineno));
    arg_node = OptTrav (arg_node, arg_info, 1); /* Trav do-body */
    arg_node = OptTrav (arg_node, arg_info, 0); /* Trav do-condition */

    cond_node = arg_node->node[0];

    /* Calculate numbers of iterations */
    if (N_id == cond_node->nodetype)
        loop_info = AnalyseLoop (cond_node, LEVEL);

    if (NULL != loop_info) {
        /* do-loops minimum iterations number is one */
        if (0 == loop_info->loop_num)
            loop_info->loop_num = 1;
        LOOP_INFO = (node *)loop_info;
        FREE (loop_info);
    } else
        LOOP_INFO = NULL;

    LEVEL--;
    PopVL2 ();
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : UNSwhile
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
UNSwhile (node *arg_node, node *arg_info)
{
    int i;
    node *cond_node;

    DBUG_ENTER ("UNSwhile");
    cond_node = arg_node->node[0];

    if ((NULL != cond_node) && (N_id == cond_node->nodetype)
        && (N_bool == VAR (cond_node->IDS_VARNO)->nodetype)
        && (TRUE == VAR (cond_node->IDS_VARNO)->info.cint)) {
        arg_node->nodetype = N_do;
        arg_node = Trav (arg_node, arg_info);
    } else {
        PushVL (VARNO);
        LEVEL++;

        for (i = 0; i < TOS.vl_len; i++) {
            if (0 != ReadMask (arg_node->node[1]->mask[0], i)) {
                VAR (i) = NULL;
            }
        }

        DBUG_PRINT ("UNR", ("Trav while loop in line %d", arg_node->lineno));
        arg_node = OptTrav (arg_node, arg_info, 1); /* Trav while-body */

        LOOP_INFO = NULL;

        LEVEL--;
        PopVL ();
        for (i = 0; i < TOS.vl_len; i++) {
            if (ReadMask (arg_node->node[1]->mask[0], i) != 0) {
                VAR (i) = NULL;
            }
        }
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : UNScond
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
UNScond (node *arg_node, node *arg_info)
{
    int i;

    DBUG_ENTER ("UNScond");
    PushDupVL ();

    LEVEL++;

    /* Trav condition */
    arg_node = OptTrav (arg_node, arg_info, 0);

    /* Trav then */
    arg_node = OptTrav (arg_node, arg_info, 1);

    PopVL ();
    PushDupVL ();

    /* Trav else */
    arg_node = OptTrav (arg_node, arg_info, 2);
    LEVEL--;

    PopVL ();
    for (i = 0; i < TOS.vl_len; i++) {
        if ((ReadMask (arg_node->node[1]->mask[0], i) != 0)
            || (ReadMask (arg_node->node[2]->mask[0], i) != 0)) {
            VAR (i) = NULL;
        }
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : UNSwith
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
UNSwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("UNSwith");
    PushDupVL ();

    LEVEL++;
    /* Trav with-body */
    arg_node = OptTrav (arg_node, arg_info, 2);
    LEVEL--;

    PopVL ();
    DBUG_RETURN (arg_node);
}
