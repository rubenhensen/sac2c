/*
 *
 * $Log$
 * Revision 1.11  1997/11/26 14:22:01  srs
 * removed use of old macros from acssass_macros.h
 *
 * Revision 1.10  1997/11/05 10:16:49  dkr
 * with defined NEWTREE node.nnode is not used anymore
 *
 * Revision 1.9  1997/04/25 12:13:00  sbs
 * MAlloc replaced by Malloc from internal.lib
 *
 * Revision 1.8  1996/01/17  14:44:57  asi
 * new dataflow-information 'most-recently-defined-Listen' MRD used,
 * new trav-function OPTTrav used and some functions uses new access-macros for
 * virtuell syntax-tree
 *
 * Revision 1.7  1995/07/20  12:23:38  asi
 * while loop, who's condition is false will be removed now
 *
 * Revision 1.6  1995/07/19  18:54:21  asi
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

#include "globals.h"
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

    arg_node = Trav (arg_node, arg_info);

    FREE (arg_info);
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
 *                  - new entry will be pushed on the mrdl_stack
 *                  - generates two masks and links them to the info_node
 *                      [0] - variables additional defined in function and
 *                      [1] - variables additional used in function after optimization
 *                  - calls Trav to  unrolled loops in body of current function
 *                  - last entry will be poped from mrdl_stack
 *                  - updates masks in fundef node.
 *  global vars   : syntax_tree, mrdl_stack
 *  internal funs :
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
    LEVEL = 1;

    if (NULL != FUNDEF_BODY (arg_node))
        FUNDEF_INSTR (arg_node) = OPTTrav (FUNDEF_INSTR (arg_node), arg_info, arg_node);

    FUNDEF_NEXT (arg_node) = OPTTrav (FUNDEF_NEXT (arg_node), arg_info, arg_node);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : UNRid
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
    MRD_GETLAST (ID_DEF (arg_node), ID_VARNO (arg_node), INFO_VARNO);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : UNRlet
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
    DBUG_ENTER ("UNRlet");
    /* maybe trav with-loop */
    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    arg_node->node[0]->flag = LEVEL;

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

    test = ID_DEF (id_node);

    if ((NULL != test) && (N_prf == NODE_TYPE (test))
        && (F_le <= (test_prf = PRF_PRF (test))) && (F_neq >= test_prf)
        && (test->flag == level)) {
        /* the constant value shall be on the right side of the expression */
        /* i.e. cond = Num op i will be changed to cond = i op Num         */
        if (IsConst (PRF_ARG1 (test))) {
            test_prf = ReversePrf (test_prf);
            arg[0] = PRF_ARG2 (test);
            arg[1] = PRF_ARG1 (test);
        } else {
            arg[0] = PRF_ARG1 (test);
            arg[1] = PRF_ARG2 (test);
        }

        /* adjust definition of conditional variable */
        PRF_PRF (test) = test_prf;
        PRF_ARG1 (test) = arg[0];
        PRF_ARG2 (test) = arg[1];

        /* store test function */
        loop_info->test_prf = test_prf;

        /* store index variable and test nummber */
        if (N_id == NODE_TYPE (arg[0])) {
            loop_info->decl_node = ID_VARDEC (arg[0]);
            index = arg[0];
        } else
            allright = FALSE;

        if (N_num == NODE_TYPE (arg[1]))
            loop_info->test_num = NUM_VAL (arg[1]);
        else
            allright = FALSE;
    } else {
        allright = FALSE;
    }

    /* Do I have somthing like i = i op Num1 ...  cond = i op Num2 */
    /*			           ^^^^^^^^^             |         */
    /*		        	      mod  <-------------|         */
    if (allright) {
        mod = ID_DEF (index);
        if ((NULL != mod) && (N_prf == NODE_TYPE (mod))
            && ((F_add == (loop_info->mod_prf = PRF_PRF (mod)))
                || (F_sub == loop_info->mod_prf) || (F_mul == loop_info->mod_prf))
            && (mod->flag == level)) {
            arg[0] = mod->ARG1;
            arg[1] = mod->ARG2;
            if ((N_id == NODE_TYPE (arg[0]))
                && (VARDEC_VARNO (loop_info->decl_node) == ID_VARNO (arg[0]))
                && (N_num == NODE_TYPE (arg[1])) && (0 != NUM_VAL (arg[1]))) {
                loop_info->mod_num = NUM_VAL (arg[1]);
                index = arg[0];
            } else if ((N_id == NODE_TYPE (arg[1]))
                       && (loop_info->decl_node->varno == ID_VARNO (arg[1]))
                       && (N_num == NODE_TYPE (arg[0])) && (0 != NUM_VAL (arg[0]))
                       && (F_sub != loop_info->mod_prf)) {
                loop_info->mod_num = NUM_VAL (arg[0]);
                index = arg[1];
            } else
                allright = FALSE;
        } else
            allright = FALSE;
    }

    if (allright) {
        init = index->info.ids->def;
        /* index variable must be initialize with a constant value */
        if ((NULL != init) && (init->flag < level) && (N_num == init->nodetype)) {
            loop_info->start_num = init->info.cint;
        } else
            allright = FALSE;
    }

    if (allright) {
        if ((F_mul == loop_info->mod_prf)
            && ((0 == loop_info->mod_num) || (0 == loop_info->start_num)))
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
            arg_node->node[1]->node[0] = NULL;
#ifndef NEWTREE
            arg_node->node[1]->nnode = 0;
#endif
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
    linfo *loop_info = NULL;
    node *cond_node;

    DBUG_ENTER ("UNRdo");

    LEVEL++;

    DBUG_PRINT ("UNR", ("Trav do loop in line %d", NODE_LINE (arg_node)));
    DO_INSTR (arg_node) = OPTTrav (DO_INSTR (arg_node), arg_info, arg_node);

    cond_node = DO_COND (arg_node);

    if (N_id == NODE_TYPE (cond_node))
        MRD_GETLAST (ID_DEF (cond_node), ID_VARNO (cond_node), INFO_VARNO);

    loop_info = (linfo *)Malloc (sizeof (linfo));
    loop_info->loop_num = UNDEF;
    loop_info->ltype = N_do;

    /* Calculate numbers of iterations */
    switch (NODE_TYPE (cond_node)) {
    case N_bool:
        if (FALSE == BOOL_VAL (cond_node)) {
            loop_info->loop_num = 1;
        }
        break;
    case N_id:
        loop_info = AnalyseLoop (loop_info, cond_node, LEVEL);
        break;
    default:
        break;
    }

    if (UNDEF != loop_info->loop_num) {
        arg_node = DoUnroll (arg_node, arg_info, loop_info);
        FREE (loop_info);
    }

    LEVEL--;
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
    DBUG_ENTER ("UNRwhile");
    DBUG_PRINT ("UNR", ("Trav while loop in line %d", NODE_LINE (arg_node)));
    WHILE_INSTR (arg_node) = OPTTrav (WHILE_INSTR (arg_node), arg_info, arg_node);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : UNRcond
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
    DBUG_ENTER ("UNRcond");

    LEVEL++;
    COND_THENINSTR (arg_node) = OPTTrav (COND_THENINSTR (arg_node), arg_info, arg_node);
    COND_ELSEINSTR (arg_node) = OPTTrav (COND_ELSEINSTR (arg_node), arg_info, arg_node);
    LEVEL--;

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : UNRwith
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

    LEVEL++;
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
    LEVEL--;

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
    nodetype ntype;

    DBUG_ENTER ("UNRassign");

    ntype = NODE_TYPE (ASSIGN_INSTR (arg_node));

    /* unroll subexpressions */
    ASSIGN_INSTR (arg_node) = OPTTrav (ASSIGN_INSTR (arg_node), arg_info, arg_node);

    switch (NODE_TYPE (ASSIGN_INSTR (arg_node))) {
    case N_empty:
        DBUG_PRINT ("UNR", ("empty assign node removed from tree"));
        tmp_node = OPTTrav (ASSIGN_NEXT (arg_node), arg_info, arg_node);
        FreeNode (arg_node);
        arg_node = tmp_node;
        break;
    case N_assign:
        DBUG_PRINT ("UNR", ("double assign node moved into tree"));
        if (N_do == ntype) {
            ASSIGN_NEXT (arg_node) = OPTTrav (ASSIGN_NEXT (arg_node), arg_info, arg_node);
            tmp_node
              = AppendNodeChain (1, ASSIGN_INSTR (arg_node), ASSIGN_NEXT (arg_node));
            ASSIGN_INSTR (arg_node) = NULL;
            ASSIGN_NEXT (arg_node) = NULL;
            FreeTree (arg_node);
            arg_node = tmp_node;
        } else {
            tmp_node
              = AppendNodeChain (1, ASSIGN_INSTR (arg_node), ASSIGN_NEXT (arg_node));
            ASSIGN_INSTR (arg_node) = NULL;
            ASSIGN_NEXT (arg_node) = NULL;
            FreeTree (arg_node);
            arg_node = UNRassign (tmp_node, arg_info);
        }
        break;
    default:
        ASSIGN_NEXT (arg_node) = OPTTrav (ASSIGN_NEXT (arg_node), arg_info, arg_node);
        break;
    }
    DBUG_RETURN (arg_node);
}
