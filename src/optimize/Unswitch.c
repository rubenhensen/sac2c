/*
 *
 * $Log$
 * Revision 1.5  1995/07/19 18:52:23  asi
 * added loop dependent unswitching
 *
 * Revision 1.3  1995/07/12  15:24:43  asi
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

typedef enum { nothing, first, last, between, split, oneloop } todo;

typedef struct CINFO {
    prf test_prf;      /* test function					 */
    long test_num;     /* test nummber						 */
    long last_test;    /* last number which will be tested			 */
    todo todo;         /* what shall be done					 */
    node *insert_node; /* ptr to assign-node the conditional has been linked to */
    node *chain1;      /* assign-chain						 */
    node *chain2;      /* assign-chain						 */
} cinfo;

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
 *  functionname  : InversePrf
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
prf
InversePrf (prf fun)
{
    DBUG_ENTER ("InversePrf");
    switch (fun) {
    case F_eq:
        fun = F_neq;
        break;
    case F_neq:
        fun = F_eq;
        break;
    case F_gt:
        fun = F_le;
        break;
    case F_ge:
        fun = F_lt;
        break;
    case F_lt:
        fun = F_ge;
        break;
    case F_le:
        fun = F_gt;
        break;
    default:
        break;
    }
    DBUG_RETURN (fun);
}

/*
 *
 *  functionname  : DoesItHappen1
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
int
DoesItHappen1 (cinfo *cond_info, linfo *loop_info)
{
    int rest, inside;
    int answer = TRUE;

    DBUG_ENTER ("DoesItHappen1");
    switch (loop_info->test_prf) {
    case F_le:
        switch (loop_info->mod_prf) {
        case F_add:
            cond_info->last_test = loop_info->end_num - loop_info->mod_num;
            rest = (cond_info->test_num - loop_info->start_num) % loop_info->mod_num;
            break;
        case F_mul:
            cond_info->last_test = loop_info->end_num / loop_info->mod_num;
            rest = (cond_info->test_num / loop_info->start_num) % loop_info->mod_num;
            break;
        default:
            break;
        }

        /* is test number of conditional inside this range */
        inside = ((loop_info->start_num <= cond_info->test_num)
                  && (cond_info->test_num <= cond_info->last_test));

        /* if not inside this range */
        if ((rest && inside) || !inside) {
            answer = FALSE;
        }
        break;
    case F_ge:

        cond_info->last_test = loop_info->end_num + loop_info->mod_num;
        rest = (cond_info->test_num + loop_info->start_num) % loop_info->mod_num;

        /* is test number of conditional inside this range */
        inside = ((loop_info->start_num >= cond_info->test_num)
                  && (cond_info->test_num >= cond_info->last_test));

        /* if not inside this range */
        if ((rest && inside) || !inside) {
            answer = FALSE;
        }
        break;
    default:
        break;
    }
    DBUG_RETURN (answer);
}

/*
 *
 *  functionname  : DoesItHappen2
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
int
DoesItHappen2 (cinfo *cond_info, linfo *loop_info)
{
    int inside;
    int answer = TRUE;

    DBUG_ENTER ("DoesItHappen2");
    switch (loop_info->test_prf) {
    case F_le:
        switch (loop_info->mod_prf) {
        case F_add:
            cond_info->last_test = loop_info->end_num - loop_info->mod_num;
            break;
        case F_mul:
            cond_info->last_test = loop_info->end_num / loop_info->mod_num;
            break;
        default:
            break;
        }

        /* is test number of conditional inside this range */
        inside = ((loop_info->start_num <= cond_info->test_num)
                  && (cond_info->test_num <= cond_info->last_test));

        /* if not inside this range */
        if (!inside) {
            answer = FALSE;
        }
        break;
    case F_ge:
        cond_info->last_test = loop_info->end_num + loop_info->mod_num;

        /* is test number of conditional inside this range */
        inside = ((loop_info->start_num >= cond_info->test_num)
                  && (cond_info->test_num >= cond_info->last_test));

        /* if not inside this range */
        if (!inside) {
            answer = FALSE;
        }
        break;
    default:
        break;
    }

    DBUG_RETURN (answer);
}

/*
 *
 *  functionname  : HowUnswitch
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
cinfo *
HowUnswitch (cinfo *cond_info, linfo *loop_info)
{
    DBUG_ENTER ("HowUnswitch");

    switch (cond_info->test_prf) {
    case F_neq:
        if (DoesItHappen1 (cond_info, loop_info)) {
            switch (loop_info->test_prf) {
            case F_le:
                cond_info->test_prf = F_lt;
                cond_info->todo = between;
                break;
            case F_ge:
                cond_info->test_prf = F_gt;
                cond_info->todo = between;
                break;
            default:
                break;
            }
        } else {
            cond_info->todo = oneloop;
            FreeTree (cond_info->chain2);
            cond_info->chain2 = NULL;
        }
        break;
    case F_le:
        if (DoesItHappen2 (cond_info, loop_info)) {
            cond_info->todo = split;
        } else {
            if (loop_info->start_num > cond_info->test_num)
                SWAP (cond_info->chain1, cond_info->chain2);
            cond_info->todo = oneloop;
            FreeTree (cond_info->chain2);
            cond_info->chain2 = NULL;
        }
        break;
    case F_ge:
        if (DoesItHappen2 (cond_info, loop_info)) {
            cond_info->todo = split;
        } else {
            if (loop_info->start_num < cond_info->test_num)
                SWAP (cond_info->chain1, cond_info->chain2);
            cond_info->todo = oneloop;
            FreeTree (cond_info->chain2);
            cond_info->chain2 = NULL;
        }
        break;
    default:
        break;
    }

    DBUG_RETURN (cond_info);
}

/*
 *
 *  functionname  : AnalyseCond
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
cinfo *
AnalyseCond (linfo *loop_info, node *cond, int level)
{
    node *test, *arg[2], *cond_var;
    prf test_prf;
    cinfo *cond_info;

    DBUG_ENTER ("AnalyseCond");
    cond_info = (cinfo *)MAlloc (sizeof (cinfo));
    cond_info->todo = nothing;
    cond_var = cond->node[0]->node[0];

    if (NULL != loop_info) {
        if (N_id == cond_var->nodetype) {
            test = cond_var->IDS_DEF;
            if ((NULL != test) && (N_prf == test->nodetype)
                && (F_le <= (test_prf = test->info.prf)) && (F_neq >= test_prf)
                && (test->flag == level)) {

                /* the constant value shall be on the right side of the expression */
                /* i.e. cond = Num op i will be changed to cond = i op Num         */
                if (IsConst (test->ARG2)) {
                    arg[0] = test->ARG1;
                    arg[1] = test->ARG2;
                } else {
                    test_prf = ReversePrf (test_prf);
                    arg[0] = test->ARG2;
                    arg[1] = test->ARG1;
                }

                /* adjust definition of conditional variable */
                test->info.prf = test_prf;
                test->ARG1 = arg[0];
                test->ARG2 = arg[1];

                /* do i have the same variable and is the calculations  */
                /* of the condition in  the loop ? 			*/
                if ((N_id == arg[0]->nodetype)
                    && (loop_info->decl_node->varno == arg[0]->IDS_VARNO)
                    && (N_num == arg[1]->nodetype)) {
                    cond_info->test_prf = test_prf;
                    cond_info->test_num = arg[1]->info.cint;
                    cond_info->chain1 = cond->node[0]->node[1]->node[0];
                    cond_info->chain2 = cond->node[0]->node[2]->node[0];

                    /* adjust the test prim.-function of conditional to the of the loop */
                    switch (loop_info->test_prf) {
                    case F_le:
                        if ((F_ge == cond_info->test_prf)
                            || (F_gt == cond_info->test_prf)) {
                            cond_info->test_prf = InversePrf (cond_info->test_prf);
                            SWAP (cond_info->chain1, cond_info->chain2);
                        }
                        break;
                    case F_ge:
                        if ((F_le == cond_info->test_prf)
                            || (F_lt == cond_info->test_prf)) {
                            cond_info->test_prf = InversePrf (cond_info->test_prf);
                            SWAP (cond_info->chain1, cond_info->chain2);
                        }
                        break;
                    default:
                        break;
                    }

                    /* reduce numbers of different conditions */
                    switch (cond_info->test_prf) {
                    case F_eq:
                        cond_info->test_prf = InversePrf (cond_info->test_prf);
                        SWAP (cond_info->chain1, cond_info->chain2);
                        break;
                    case F_lt:
                        cond_info->test_prf = F_le;
                        cond_info->test_num--;
                        break;
                    case F_gt:
                        cond_info->test_prf = F_ge;
                        cond_info->test_num++;
                        break;
                    default:
                        break;
                    }

                    cond_info->insert_node = cond;
                    cond_info = HowUnswitch (cond_info, loop_info);
                }
            }
        }
    }
    DBUG_RETURN (cond_info);
}

/*
 *
 *  functionname  : GenLetNode
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
GenLetNode (linfo *loop_info, node *arg_info)
{
    node *assign_node;

    DBUG_ENTER ("GenLetNode");
    assign_node = MakeNode (N_assign);
    assign_node->mask[0] = GenMask (VARNO);
    assign_node->mask[1] = GenMask (VARNO);
    INC_VAR (assign_node->mask[0], loop_info->decl_node->varno);
    INC_VAR (arg_info->mask[0], loop_info->decl_node->varno);
    assign_node->nnode = 1;

    assign_node->node[0] = MakeNode (N_let);
    assign_node->node[0]->info.ids = MakeIds (loop_info->decl_node->info.types->id);
    assign_node->node[0]->info.ids->node = loop_info->decl_node;

    assign_node->node[0]->nnode = 1;
    assign_node->node[0]->node[0] = MakeNode (N_num);
    assign_node->node[0]->node[0]->info.cint = loop_info->start_num;

    DBUG_RETURN (assign_node);
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
DoUnswitch (node *arg_node, node *arg_info, cinfo *cond_info, linfo *loop_info)
{
    int old_nnode;
    long old_test_num;
    node *second_loop = NULL;
    node *unrolled_loop = NULL;
    node *tmp_node;
    node *let_index;

    DBUG_ENTER ("DoUnswitch");
    switch (cond_info->todo) {

    case oneloop:

        DBUG_PRINT ("UNS", ("conditional removed in loop at line %d", arg_node->lineno));

        uns_expr++;
        /* subtract all variables used and defined in loop from modification masks */
        MinusMask (arg_info->mask[0], arg_node->mask[0], VARNO);
        MinusMask (arg_info->mask[1], arg_node->mask[1], VARNO);

        /* isulate loop */
        old_nnode = arg_node->nnode;
        arg_node->nnode = 1;

        /* modify first loop */
        cond_info->insert_node->node[0] = cond_info->chain1;

        /* Generate masks */
        arg_node = GenerateMasks (arg_node, arg_info);

        /* append assign-chain below loop-assign */
        arg_node->nnode = old_nnode;

        break;

    case between:

        DBUG_PRINT ("UNS", ("Move between loops at line %d", arg_node->lineno));

        uns_expr++;
        /* subtract all variables used and defined in loop from modification masks */
        MinusMask (arg_info->mask[0], arg_node->mask[0], VARNO);
        MinusMask (arg_info->mask[1], arg_node->mask[1], VARNO);

        /* isulate loop */
        arg_node->nnode = 1;

        if (cond_info->last_test != cond_info->test_num) {
            /* creat second loop */
            cond_info->insert_node->node[0] = cond_info->chain1;
            second_loop = DupTree (arg_node, NULL);

            /* Generate masks */
            second_loop = GenerateMasks (second_loop, arg_info);

            old_test_num = loop_info->test_num;
            loop_info->test_num = cond_info->test_num;

            loop_info = LoopIterations (loop_info);

            loop_info->test_num = old_test_num;
            loop_info->start_num = loop_info->end_num;

            loop_info = LoopIterations (loop_info);

            if (0 != loop_info->loop_num) {
                tmp_node = second_loop;
                second_loop
                  = DoUnroll (second_loop->node[0], arg_info, (linfo *)loop_info);
                if (tmp_node->node[0] == second_loop) {
                    second_loop = tmp_node;
                    let_index = GenLetNode (loop_info, arg_info);
                    second_loop = AppendNodeChain (1, let_index, second_loop);
                }
            } else {
                FreeTree (second_loop);
                second_loop = NULL;
            }
        }

        /* create unrolled loop */
        cond_info->insert_node->node[0] = cond_info->chain2;
        unrolled_loop = DupTree (arg_node->node[0]->node[1]->node[0], NULL);

        /* Generate masks */
        unrolled_loop = GenerateMasks (unrolled_loop, arg_info);

        /* modify first loop */
        cond_info->insert_node->node[0] = cond_info->chain1;
        arg_node->node[0]->node[0]->IDS_DEF->info.prf = cond_info->test_prf;
        arg_node->node[0]->node[0]->IDS_DEF->ARG2->info.cint = cond_info->test_num;

        /* Generate masks */
        arg_node = GenerateMasks (arg_node, arg_info);

        /* append later assign nodes to second loop */
        second_loop = AppendNodeChain (1, second_loop, arg_node->node[1]);

        /* , unrolled loop to second loop */
        unrolled_loop = AppendNodeChain (1, unrolled_loop, second_loop);

        /* and second loop to first loop */
        arg_node->node[1] = unrolled_loop;
        arg_node->nnode = 2;

        break;

    case split:

        DBUG_PRINT ("UNS", ("Splitting loop at line %d", arg_node->lineno));

        uns_expr++;
        /* subtract all variables used and defined in loop from modification masks */
        MinusMask (arg_info->mask[0], arg_node->mask[0], VARNO);
        MinusMask (arg_info->mask[1], arg_node->mask[1], VARNO);

        /* isulate loop */
        arg_node->nnode = 1;

        if (cond_info->test_num != cond_info->last_test) {
            /* creat second loop */
            cond_info->insert_node->node[0] = cond_info->chain2;
            second_loop = DupTree (arg_node, NULL);

            /* Generate masks */
            second_loop = GenerateMasks (second_loop, arg_info);

            old_test_num = loop_info->test_num;
            loop_info->test_num = cond_info->test_num;

            loop_info = LoopIterations (loop_info);

            loop_info->test_num = old_test_num;
            loop_info->start_num = loop_info->end_num;

            loop_info = LoopIterations (loop_info);

            if (0 != loop_info->loop_num) {
                tmp_node = second_loop;
                second_loop
                  = DoUnroll (second_loop->node[0], arg_info, (linfo *)loop_info);
                if (tmp_node->node[0] == second_loop) {
                    second_loop = tmp_node;
                    let_index = GenLetNode (loop_info, arg_info);
                    second_loop = AppendNodeChain (1, let_index, second_loop);
                }
            } else {
                FreeTree (second_loop);
                second_loop = NULL;
            }
        }

        /* modify first loop */
        cond_info->insert_node->node[0] = cond_info->chain1;
        arg_node->node[0]->node[0]->IDS_DEF->info.prf = cond_info->test_prf;
        arg_node->node[0]->node[0]->IDS_DEF->ARG2->info.cint = cond_info->test_num;

        /* Generate masks */
        arg_node = GenerateMasks (arg_node, arg_info);

        /* append later assign nodes to second loop */
        second_loop = AppendNodeChain (1, second_loop, arg_node->node[1]);

        /* and second loop to first loop */
        arg_node->node[1] = second_loop;
        arg_node->nnode = 2;

        break;

    case nothing:
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
    cinfo *cond_info;
    int once_more = FALSE;
    node *tmp;

    DBUG_ENTER ("UNSassign");

    if (N_cond == arg_node->node[0]->nodetype)
        COND = arg_node;

    tmp = COND;

    PushDupVL (VARNO);

    /* unswitch subexpressions */
    arg_node = OptTrav (arg_node, arg_info, 0);

    if (N_do == arg_node->node[0]->nodetype) {
        if ((NULL != COND) && (NULL != LOOP_INFO)) {
            cond_info = AnalyseCond ((linfo *)LOOP_INFO, COND, LEVEL + 1);
            if (nothing != cond_info->todo) {
                arg_node = DoUnswitch (arg_node, arg_info, cond_info, (linfo *)LOOP_INFO);
                once_more = TRUE;
            }
        }
        if (NULL != LOOP_INFO) {
            FREE (LOOP_INFO);
            LOOP_INFO = NULL;
        }
    }

    COND = tmp;

    if (once_more) {
        PopVL ();
        arg_node = Trav (arg_node, arg_info);
    } else {
        PopVL2 ();
        /* next assign node */
        if (1 < arg_node->nnode) {
            arg_node->node[1] = Trav (arg_node->node[1], arg_info);
        }
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
    node *id_node;

    DBUG_ENTER ("UNSdo");
    PushDupVL (VARNO);

    LEVEL++;

    for (i = 0; i < TOS.vl_len; i++) {
        if (1 < ReadMask (arg_node->node[1]->mask[0], i)) {
            VAR (i) = NULL;
        }
    }

    COND = NULL;

    DBUG_PRINT ("UNS", ("Trav do loop in line %d", arg_node->lineno));
    arg_node = OptTrav (arg_node, arg_info, 1); /* Trav do-body */
    arg_node = OptTrav (arg_node, arg_info, 0); /* Trav do-condition */

    id_node = arg_node->node[0];

    loop_info = (linfo *)MAlloc (sizeof (linfo));
    loop_info->loop_num = UNDEF;
    loop_info->ltype = N_do;

    /* Calculate numbers of iterations */
    if (N_id == id_node->nodetype) {
        loop_info = AnalyseLoop (loop_info, id_node, LEVEL);
    }

    if (2 > loop_info->loop_num) {
        LOOP_INFO = NULL;
        FREE (loop_info);
    } else
        LOOP_INFO = (node *)loop_info;

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
    node *id_node;

    DBUG_ENTER ("UNSwhile");
    id_node = arg_node->node[0];

    if ((NULL != id_node) && (N_id == id_node->nodetype)
        && (N_bool == VAR (id_node->IDS_VARNO)->nodetype)
        && (TRUE == VAR (id_node->IDS_VARNO)->info.cint)) {
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

        DBUG_PRINT ("UNS", ("Trav while loop in line %d", arg_node->lineno));
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
