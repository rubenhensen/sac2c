/*
 *
 * $Log$
 * Revision 1.6  1995/07/19 18:54:21  asi
 * AnalyseLoop, DoUnroll, ReversePrf and structure linfo modified
 *
 * Revision 1.5  1995/07/12  15:26:24  asi
 * some macros moved from .c to .h
 * pointers to varables definitions added
 *
 * Revision 1.4  1995/07/07  15:02:22  asi
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
 *  external funs : GenMask, OptTrav
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
    PushVL (arg_info->varno);
    LEVEL = 1;

    arg_node = OptTrav (arg_node, arg_info, 0); /* functionbody */

    PopVL ();

    arg_node = OptTrav (arg_node, arg_info, 1); /* next function */
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  :
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
UNRid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("UNRid");
    arg_node->flag = LEVEL;
    arg_node->IDS_DEF = VAR (arg_node->IDS_VARNO);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  :
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
UNRlet (node *arg_node, node *arg_info)
{
    ids *ids_node;

    DBUG_ENTER ("UNRlet");
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
 *  functionname  : UNRassign
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
UNRassign (node *arg_node, node *arg_info)
{
    node *tmp_node;

    DBUG_ENTER ("UNRassign");

    /* unroll subexpressions */
    arg_node = OptTrav (arg_node, arg_info, 0);

    switch (arg_node->node[0]->nodetype) {
    case N_empty:
        DBUG_PRINT ("UNR", ("empty assign node removed from tree"));
        arg_node->nnode = 1;
        tmp_node = arg_node->node[1];
        FreeTree (arg_node);
        arg_node = Trav (tmp_node, arg_info);
        break;
    case N_assign:
        DBUG_PRINT ("UNR", ("double assign node moved into tree"));
        tmp_node = AppendNodeChain (1, arg_node->node[0], arg_node->node[1]);
        arg_node->nnode = 0;
        FreeTree (arg_node);
        arg_node = Trav (tmp_node, arg_info);
        break;
    default:
        /* next assign node */
        if (1 < arg_node->nnode) {
            arg_node->node[1] = Trav (arg_node->node[1], arg_info);
        }
        break;
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : ReversePrf
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
ReversePrf (prf fun)
{
    DBUG_ENTER ("ReversePrf");
    switch (fun) {
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

/*
 *
 *  functionname  : LoopIterations
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
            case F_le:
                loop_info->loop_num = num + 1;
                break;
            default:
                loop_info->loop_num = UNDEF;
            }
        } else if (N_do == loop_info->ltype)
            loop_info->loop_num = 1;
        else
            loop_info->loop_num = 0;

        /* calculate value of index variable after the loop */
        loop_info->end_num
          = loop_info->start_num + (loop_info->loop_num * loop_info->mod_num);
        break;
    case F_sub:
        num = (loop_info->start_num - loop_info->test_num) / loop_info->mod_num;
        rest = (loop_info->start_num - loop_info->test_num) % loop_info->mod_num;
        if (0 < num) {
            switch (loop_info->test_prf) {
            case F_ge:
                loop_info->loop_num = num + 1;
                break;
            default:
                loop_info->loop_num = UNDEF;
            }
        } else if (N_do == loop_info->ltype)
            loop_info->loop_num = 1;
        else
            loop_info->loop_num = 0;

        /* calculate value of index variable after the loop */
        loop_info->end_num
          = loop_info->start_num - (loop_info->loop_num * loop_info->mod_num);
        break;
    case F_mul:
        num = loop_info->test_num / (loop_info->start_num * loop_info->mod_num);
        rest = loop_info->test_num % (loop_info->start_num * loop_info->mod_num);
        if (0 < num) {
            switch (loop_info->test_prf) {
            case F_le:
                loop_info->loop_num = num + 1;
                break;
            default:
                loop_info->loop_num = UNDEF;
            }
        } else if (N_do == loop_info->ltype)
            loop_info->loop_num = 1;
        else
            loop_info->loop_num = 0;

        /* calculate value of index variable after the loop */
        loop_info->end_num
          = loop_info->start_num * (loop_info->loop_num * loop_info->mod_num);
        break;
    default:
        loop_info->loop_num = UNDEF;
    }

    DBUG_RETURN (loop_info);
}

/*
 *
 *  functionname  : AnalyseLoop
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
linfo *
AnalyseLoop (linfo *loop_info, node *id_node, int level)
{
    node *init, *test, *mod, *index, *arg[2];
    int allright = TRUE;
    prf test_prf;

    DBUG_ENTER ("AnalyseLoop");

    test = id_node->IDS_DEF;

    if ((NULL != test) && (N_prf == test->nodetype)
        && (F_le <= (test_prf = test->info.prf)) && (F_neq >= test_prf)
        && (test->flag == level)) {
        arg[0] = test->ARG1;
        arg[1] = test->ARG2;
        /* the constant value shall be on the right side of the expression */
        /* i.e. cond = Num op i will be changed to cond = i op Num         */
        if (IsConst (test->ARG1)) {
            test_prf = ReversePrf (test_prf);
            arg[0] = test->ARG2;
            arg[1] = test->ARG1;
        } else {
            arg[0] = test->ARG1;
            arg[1] = test->ARG2;
        }
        /* adjust definition of conditional variable */
        test->info.prf = test_prf;
        test->ARG1 = arg[0];
        test->ARG2 = arg[1];

        /* store test function */
        loop_info->test_prf = test_prf;

        /* store index variable and test nummber */
        if (N_id == arg[0]->nodetype) {
            loop_info->decl_node = arg[0]->IDS_NODE;
            index = arg[0];
        } else
            allright = FALSE;

        if (N_num == arg[1]->nodetype)
            loop_info->test_num = arg[1]->info.cint;
        else
            allright = FALSE;
    } else {
        allright = FALSE;
    }

    /* Do I have somthing like i = i op Num1 ...  cond = i op Num2 */
    /*			           ^^^^^^^^^             |         */
    /*		        	      mod  <-------------|         */
    if (allright) {
        mod = index->IDS_DEF;
        if ((NULL != mod) && (N_prf == mod->nodetype)
            && ((F_add == (loop_info->mod_prf = mod->info.prf))
                || (F_sub == loop_info->mod_prf) || (F_mul == loop_info->mod_prf))
            && (mod->flag == level)) {
            arg[0] = mod->ARG1;
            arg[1] = mod->ARG2;
            if ((N_id == arg[0]->nodetype)
                && (loop_info->decl_node->varno == arg[0]->IDS_VARNO)
                && (N_num == arg[1]->nodetype) && (0 != arg[1]->info.cint)) {
                loop_info->mod_num = arg[1]->info.cint;
                index = arg[0];
            } else if ((N_id == arg[1]->nodetype)
                       && (loop_info->decl_node->varno == arg[1]->IDS_VARNO)
                       && (N_num == arg[0]->nodetype) && (0 != arg[0]->info.cint)
                       && (F_sub != loop_info->mod_prf)) {
                loop_info->mod_num = arg[0]->info.cint;
                index = arg[1];
            } else
                allright = FALSE;
        } else
            allright = FALSE;
    }

    if (allright) {
        init = index->IDS_DEF;
        /* index variable must be initialize with a constant value */
        if ((NULL != init) && (init->flag < level) && (N_num == init->nodetype)) {
            loop_info->start_num = init->info.cint;
        } else
            allright = FALSE;
    }

    if (allright) {
        if ((F_mul == loop_info->mod_prf)
            && ((0 >= loop_info->mod_num) || (0 >= loop_info->start_num)))
            allright = FALSE;
        else {
            if (0 == loop_info->mod_num) {
                allright = FALSE;
            } else {
                if (0 > loop_info->mod_num) {
                    switch (loop_info->mod_prf) {
                    case F_add:
                        loop_info->mod_num = -loop_info->mod_num;
                        loop_info->mod_prf = F_sub;
                        break;
                    default:
                        allright = FALSE;
                        break;
                    }
                }
            }
        }
    }

    /* reduce numbers of different loops */
    if (allright) {
        switch (loop_info->test_prf) {
        case F_gt:
            if (F_sub == loop_info->mod_prf) {
                loop_info->test_prf = F_ge;
                loop_info->test_num++;
            } else
                allright = FALSE;
            break;
        case F_lt:
            if ((F_add == loop_info->mod_prf) || (F_mul == loop_info->mod_prf)) {
                loop_info->test_prf = F_le;
                loop_info->test_num--;
            } else
                allright = FALSE;
            break;
        case F_le:
            if (!((F_add == loop_info->mod_prf) || (F_mul == loop_info->mod_prf)))
                allright = FALSE;
            break;
        case F_ge:
            if (!(F_sub == loop_info->mod_prf))
                allright = FALSE;
            break;
        default:
            break;
        }
    }

    if (allright) {
        loop_info = LoopIterations (loop_info);
    } else {
        loop_info->loop_num = UNDEF;
    }

    DBUG_PRINT ("UNR", ("numbers of iteration %d", loop_info->loop_num));
    DBUG_RETURN (loop_info);
}

/*
 *
 *  functionname  : DoUnroll
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
DoUnroll (node *arg_node, node *arg_info, linfo *loop_info)
{
    int i;
    node *tmp, *unroll = NULL;

    DBUG_ENTER ("DoUnroll");
    if (opt_unr && (loop_info->loop_num <= unrnum)) {
        DBUG_PRINT ("UNR", ("Unrolling %d times %s in line %d", loop_info->loop_num,
                            mdb_nodetype[arg_node->nodetype], arg_node->lineno));
        unr_expr++;
        switch (loop_info->loop_num) {
        case 0:
            MinusMask (arg_info->mask[0], arg_node->node[1]->mask[0], VARNO);
            MinusMask (arg_info->mask[1], arg_node->node[1]->mask[1], VARNO);
            MinusMask (arg_info->mask[1], arg_node->mask[1], VARNO);
            FreeTree (arg_node);
            arg_node = MakeNode (N_empty);
            break;
        case 1:
            MinusMask (arg_info->mask[1], arg_node->mask[1], VARNO);
            unroll = arg_node->node[1]->node[0];
            arg_node->node[1]->nnode = 0;
            FreeTree (arg_node);
            arg_node = unroll;
            break;
        default:
            MinusMask (arg_info->mask[0], arg_node->node[1]->mask[0], VARNO);
            MinusMask (arg_info->mask[1], arg_node->node[1]->mask[1], VARNO);
            MinusMask (arg_info->mask[1], arg_node->mask[1], VARNO);

            for (i = 0; i < loop_info->loop_num; i++) {
                tmp = DupTree (arg_node->node[1]->node[0], NULL);
                unroll = AppendNodeChain (1, unroll, tmp);
            }
            unroll = GenerateMasks (unroll, arg_info);
            FreeTree (arg_node);
            arg_node = unroll;
            break;
        }
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : UNRdo
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
UNRdo (node *arg_node, node *arg_info)
{
    int i;
    linfo *loop_info = NULL;
    node *id_node;

    DBUG_ENTER ("UNRdo");

    PushDupVL (VARNO);

    LEVEL++;

    for (i = 0; i < TOS.vl_len; i++) {
        if (1 < ReadMask (arg_node->node[1]->mask[0], i)) {
            VAR (i) = NULL;
        }
    }

    DBUG_PRINT ("UNR", ("Trav do loop in line %d", arg_node->lineno));
    arg_node = OptTrav (arg_node, arg_info, 1); /* Trav do-body */
    arg_node = OptTrav (arg_node, arg_info, 0); /* Trav do-condition */

    id_node = arg_node->node[0];

    loop_info = (linfo *)MAlloc (sizeof (linfo));
    loop_info->loop_num = UNDEF;
    loop_info->ltype = N_do;

    /* Calculate numbers of iterations */
    if (N_bool == arg_node->node[0]->nodetype) {
        if (FALSE == id_node->info.cint) {
            loop_info->loop_num = 1;
        }
    } else {
        if (N_id == id_node->nodetype) {
            loop_info = AnalyseLoop (loop_info, id_node, LEVEL);
        }
    }

    if (UNDEF != loop_info->loop_num) {
        arg_node = DoUnroll (arg_node, arg_info, loop_info);
        FREE (loop_info);
    }

    LEVEL--;
    PopVL2 ();
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : UNRwhile
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
UNRwhile (node *arg_node, node *arg_info)
{
    int i;
    node *cond_node;

    DBUG_ENTER ("UNRwhile");
    cond_node = arg_node->node[0];

    if ((NULL != cond_node) && (N_id == cond_node->nodetype)
        && (N_bool == VAR (cond_node->IDS_VARNO)->nodetype)
        && (VAR (cond_node->IDS_VARNO)->info.cint)) {
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
 *  functionname  :
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
UNRcond (node *arg_node, node *arg_info)
{
    int i;

    DBUG_ENTER ("UNRcond");
    PushDupVL ();

    LEVEL++;
    /* Trav then */
    arg_node = OptTrav (arg_node, arg_info, 1);
    LEVEL--;

    PopVL ();
    PushDupVL ();

    LEVEL++;
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
 *  functionname  :
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
UNRwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("UNRwith");
    PushDupVL ();

    LEVEL++;
    /* Trav with-body */
    arg_node = OptTrav (arg_node, arg_info, 2);
    LEVEL--;

    PopVL ();
    DBUG_RETURN (arg_node);
}
