/*
 *
 * $Log$
 * Revision 1.4  1995/07/07 15:02:22  asi
 * loop unrolling completed
 *
 * Revision 1.3  1995/06/26  16:24:38  asi
 * added UNRfundef and enhanced Unroll
 *
 * Revision 1.2  1995/06/14  13:31:23  asi
 * added Unroll, UNRdo, UNRwhile and UNRassign
 *
 * Revision 1.1  1995/05/26  14:22:26  asi
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

#define UNROLL_NO arg_info->refcnt

#define FALSE 0
#define TRUE 1

/*
 *
 *  functionname  : Unroll
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
Unroll (node *arg_node, node *arg_info)
{
    funptr *tmp_tab;

    DBUG_ENTER ("Unroll");
    tmp_tab = act_tab;
    act_tab = unroll_tab;
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
 *  functionname  : UNRfundef
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
 *                  - calls Trav to  unrolled loops in body of current function
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
UNRfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("UNRfundef");

    DBUG_PRINT ("UNR", ("Unroll in function: %s", arg_node->info.types->id));
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
UNRlet (node *arg_node, node *arg_info)
{
    node *arg1;
    node *arg2;
    ids *ids_node;

    DBUG_ENTER ("UNRlet");
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
UNRassign (node *arg_node, node *arg_info)
{
    node *unroll = NULL, *tmp;
    int i;

    DBUG_ENTER ("UNRassign");

    /* unroll subexpressions and/or calculate UNROLL_NO */
    arg_node = OptTrav (arg_node, arg_info, 0);

    switch (arg_node->node[0]->nodetype) {
    case N_while:
    case N_do:
        DBUG_PRINT ("UNR", ("Unrolling %d times %s in line %d", UNROLL_NO,
                            mdb_nodetype[arg_node->node[0]->nodetype], arg_node->lineno));
        if (0 <= UNROLL_NO) {
            if (N_assign == arg_node->node[0]->node[1]->node[0]->nodetype) {
                if (0 < UNROLL_NO) {
                    for (i = 0; i < UNROLL_NO; i++) {
                        tmp = DupTree (arg_node->node[0]->node[1]->node[0], NULL);
                        unroll = AppendNodeChain (1, unroll, tmp);
                    }
                    unroll = GenerateMasks (unroll, arg_info);
                }
                unroll = AppendNodeChain (1, unroll, arg_node->node[1]);
            }
            arg_node->nnode = 1;
            FreeTree (arg_node);
            arg_node = unroll;
            unr_expr++;
            UNROLL_NO = -1;
        }
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

prf
InversePrf (prf fun)
{
    DBUG_ENTER ("InversePrf");
    switch (fun) {
    case F_add:
        fun = F_sub;
        break;
    case F_sub:
        fun = F_add;
        break;
    case F_mul:
        fun = F_div;
        break;
    case F_div:
        fun = F_mul;
        break;
    case F_gt:
        fun = F_lt;
        break;
    case F_ge:
        fun = F_le;
        break;
    case F_lt:
        fun = F_gt;
        break;
    case F_le:
        fun = F_ge;
        break;
    default:
        break;
    }
    DBUG_RETURN (fun);
}

linfo *
LoopIterations (linfo *loop_info)
{
    int num, rest;

    DBUG_ENTER ("LoopIterations");
    switch (loop_info->mod_prf) {
    case F_add:
        num = (loop_info->test_num - loop_info->start_num) / loop_info->mod_num;
        rest = (loop_info->test_num - loop_info->start_num) % loop_info->mod_num;
        if (0 < num) {
            switch (loop_info->test_prf) {
            case F_lt:
                if (0 == rest)
                    loop_info->loop_num = num;
                else
                    loop_info->loop_num = num + 1;
                break;
            case F_le:
                loop_info->loop_num = num + 1;
                break;
            default:
                loop_info->loop_num = -1;
            }
        } else
            loop_info->loop_num = 0;
        break;
    case F_sub:
        num = (loop_info->start_num - loop_info->test_num) / loop_info->mod_num;
        rest = (loop_info->start_num - loop_info->test_num) % loop_info->mod_num;
        if (0 < num) {
            switch (loop_info->test_prf) {
            case F_gt:
                if (0 == rest)
                    loop_info->loop_num = num;
                else
                    loop_info->loop_num = num + 1;
                break;
            case F_ge:
                loop_info->loop_num = num + 1;
                break;
            default:
                loop_info->loop_num = -1;
            }
        } else
            loop_info->loop_num = 0;
        break;
    default:
        loop_info->loop_num = -1;
    }

    if (0 >= loop_info->loop_num) {
        FREE (loop_info);
        loop_info = NULL;
    }
    DBUG_RETURN (loop_info);
}

linfo *
AnalyseLoop (node *test, node *mod, nodetype loop_type)
{
    node *tmp, *arg[2];
    int allright = TRUE;
    int swap;
    linfo *loop_info;
    prf test_prf, mod_prf;

    DBUG_ENTER ("AnalyseLoop");

    loop_info = (linfo *)MAlloc (sizeof (linfo));
    loop_info->type = loop_type;

    if (!(
          /* all arguments available */
          (NULL != test) && (NULL != mod) &&

          /* is it a primitive function */
          (N_prf == test->nodetype) && (N_prf == mod->nodetype) &&

          /* comparison operator as loop-test prim-function */
          (F_le <= (test_prf = test->info.prf)) && (F_neq >= test_prf) &&

          /* test index-variable modification prim-function */
          ((F_add == (mod_prf = mod->info.prf)) || (F_sub == mod_prf)))) {
        allright = FALSE;
    }

    if (allright) {
        arg[0] = test->ARG1;
        arg[1] = test->ARG2;
        /* the constant value shall be on the right side of the expression */
        /* i.e. cond = Num op i will be changed to cond = i op Num         */
        if (IsConst (arg[0])) {
            test_prf = InversePrf (test_prf);
            swap = TRUE;
        } else
            swap = FALSE;

        /* store test function */
        loop_info->test_prf = test_prf;

        /* store index variable and test nummber */
        if (!swap) {
            if (N_id == arg[0]->nodetype)
                loop_info->varno = arg[0]->IDS_VARNO;
            else
                allright = FALSE;

            if (N_num == arg[1]->nodetype)
                loop_info->test_num = arg[1]->info.cint;
            else
                allright = FALSE;
        } else {
            if (N_id == arg[1]->nodetype)
                loop_info->varno = arg[1]->IDS_VARNO;
            else
                allright = FALSE;

            if (N_num == arg[0]->nodetype)
                loop_info->test_num = arg[0]->info.cint;
            else
                allright = FALSE;
        }
    }

    if (allright) {
        tmp = def_stack->stack[(def_stack->tos) - 1].varlist[loop_info->varno];
        /* index variable must be initialize with a constant value */
        if (N_num == tmp->nodetype)
            loop_info->start_num = tmp->info.cint;
        else
            allright = FALSE;
    }

    /* Do I have somthing like i = i op Num1 ...  cond = i op Num2 */
    /*			           ^^^^^^^^^             |         */
    /*		        	      mod  <-------------|         */
    if (allright) {
        arg[0] = mod->ARG1;
        arg[1] = mod->ARG2;
        if ((N_id == arg[0]->nodetype) && (loop_info->varno == arg[0]->IDS_VARNO)
            && (N_num == arg[1]->nodetype) && (0 != arg[1]->info.cint)) {
            loop_info->mod_num = arg[1]->info.cint;
            loop_info->mod_prf = mod_prf;
        } else if ((N_id == arg[1]->nodetype) && (loop_info->varno == arg[1]->IDS_VARNO)
                   && (N_num == arg[0]->nodetype) && (0 != arg[0]->info.cint)) {
            loop_info->mod_num = arg[0]->info.cint;
            loop_info->mod_prf = mod_prf;
        } else
            allright = FALSE;
    }

    if (allright) {
        loop_info = LoopIterations (loop_info);
    } else {
        FREE (loop_info);
        loop_info = NULL;
    }

    DBUG_RETURN (loop_info);
}

node *
UNRdo (node *arg_node, node *arg_info)
{
    int i;
    linfo *loop_info;
    node *cond_node;
    int old_SEARCH;
    int old_COND_ID;
    node *old_COND_TEST;
    node *old_COND_MOD;
    long *old_PREV_DEF;

    DBUG_ENTER ("UNRdo");

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

    DBUG_PRINT ("UNR", ("Trav do loop in line %d", arg_node->lineno));
    arg_node = OptTrav (arg_node, arg_info, 1); /* Trav do-body */

    loop_info = AnalyseLoop (COND_TEST, COND_MOD, N_do);
    if ((NULL != loop_info) && (loop_info->loop_num <= unrnum)) {
        if (0 == loop_info->loop_num)
            loop_info->loop_num = 1;
        UNROLL_NO = loop_info->loop_num;
        FREE (loop_info);
    } else
        UNROLL_NO = -1;

    switch (cond_node->nodetype) {
    case N_id:
        FREE (PREV_DEF);
        PREV_DEF = old_PREV_DEF;
        SEARCH = old_SEARCH;
        COND_ID = old_COND_ID;
        COND_TEST = old_COND_TEST;
        COND_MOD = old_COND_MOD;
        break;
    case N_bool:
        if (FALSE == cond_node->info.cint)
            UNROLL_NO = 1;
        break;
    default:
        break;
    }

    PopVL2 ();
    DBUG_PRINT ("UNR", ("numbers of iteration %d", UNROLL_NO));
    DBUG_RETURN (arg_node);
}

node *
UNRwhile (node *arg_node, node *arg_info)
{
    int i;
    linfo *loop_info;
    node *cond_node;
    int old_SEARCH;
    int old_COND_ID;
    node *old_COND_TEST;
    node *old_COND_MOD;
    long *old_PREV_DEF;

    DBUG_ENTER ("UNRwhile");
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

    DBUG_PRINT ("UNR", ("Trav while loop in line %d", arg_node->lineno));
    arg_node = OptTrav (arg_node, arg_info, 1); /* Trav do-body */

    loop_info = AnalyseLoop (COND_TEST, COND_MOD, N_while);
    if ((NULL != loop_info) && (loop_info->loop_num <= unrnum)) {
        UNROLL_NO = loop_info->loop_num;
        FREE (loop_info);
    } else
        UNROLL_NO = -1;

    switch (cond_node->nodetype) {
    case N_id:
        FREE (PREV_DEF);
        PREV_DEF = old_PREV_DEF;
        SEARCH = old_SEARCH;
        COND_ID = old_COND_ID;
        COND_TEST = old_COND_TEST;
        COND_MOD = old_COND_MOD;
        break;
    case N_bool:
        if (FALSE == cond_node->info.cint)
            UNROLL_NO = 0;
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
UNRcond (node *arg_node, node *arg_info)
{
    int i;

    DBUG_ENTER ("UNRcond");
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
UNRwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("UNRwith");
    PushDupVL ();

    /* Trav with-body */
    arg_node = OptTrav (arg_node, arg_info, 2);

    PopVL ();
    DBUG_RETURN (arg_node);
}
