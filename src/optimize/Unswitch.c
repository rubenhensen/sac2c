/*
 *
 * $Log$
 * Revision 3.6  2001/03/22 21:10:21  dkr
 * no changes done
 *
 * Revision 3.5  2001/02/13 17:28:36  dkr
 * MakeNode() eliminated
 *
 * Revision 3.4  2001/02/02 10:21:55  dkr
 * import of access_macros.h removed
 *
 * Revision 3.3  2000/12/01 14:37:26  sbs
 * warnings eliminated.
 *
 * Revision 3.2  2000/11/23 16:41:42  sbs
 * Unswitch.c:139: warning: unused variable `mem_uns_expr' eliminated
 *
 * Revision 3.1  2000/11/20 18:00:36  sacbase
 * new release made
 *
 * Revision 2.8  2000/06/23 15:20:04  dkr
 * signature of DupTree chagned
 *
 * Revision 2.7  2000/06/13 12:24:06  dkr
 * function for old with-loop removed
 *
 * Revision 2.6  2000/06/13 12:14:07  dkr
 * obsolete NEW_TREE code removed
 * bug in GenLetNode() fixed: IDS_NAME is no longer shared but duplicated
 * now :-)
 * code brushed (compound macros!)
 *
 * Revision 2.5  2000/01/26 17:27:22  dkr
 * type of traverse-function-table changed.
 *
 * Revision 2.4  1999/11/15 18:05:19  dkr
 * VARNO replaced, INFO_VARNO with changed signature
 *
 * Revision 2.3  1999/04/19 12:51:57  jhs
 * TRUE and FALSE from internal_lib.h used from now on.
 *
 * Revision 2.2  1999/03/31 15:07:29  bs
 *  I did some code cosmetics with the MRD_GET... macros.
 *
 * Revision 2.1  1999/02/23 12:41:29  sacbase
 * new release made
 *
 * Revision 1.14  1999/02/16 21:56:20  sbs
 * eliminated an error in DoUnswitch:
 * arg_node1 in the "split-case" is used for generating the final
 * assign-list.
 *
 * Revision 1.13  1999/01/18 15:46:02  sbs
 * DBUG_PRINT( "OPTMEM",...) inserted for mem-info during optimization
 *
 * Revision 1.12  1999/01/07 13:56:58  sbs
 * optimization process restructured for a function-wise optimization!
 *
 * Revision 1.11  1998/03/04 09:47:18  srs
 * added support for new WL
 *
 * Revision 1.10  1997/11/26 14:21:31  srs
 * removed use of old macros from acssass_macros.h
 *
 * Revision 1.9  1997/11/07 14:27:14  dkr
 * with defined NEWTREE node.nnode is not used anymore
 *
 * Revision 1.8  1997/04/25 12:13:00  sbs
 * MAlloc replaced by Malloc from internal.lib
 *
 * Revision 1.7  1996/01/17  14:44:57  asi
 * new dataflow-information 'most-recently-defined-Listen' MRD used,
 * new trav-function OPTTrav used and some functions uses new access-macros for
 * virtuell syntax-tree
 *
 * Revision 1.6  1995/10/06  16:35:40  cg
 * calls to MakeIds adjusted to new signature (3 parameters)
 *
 * Revision 1.5  1995/07/19  18:52:23  asi
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
 */

#include <stdio.h>
#include <stdlib.h>

#include "tree.h" /* old tree definition */
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "globals.h"
#include "free.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"

#include "optimize.h"
#include "generatemasks.h"
#include "ConstantFolding.h"
#include "DupTree.h"
#include "Unroll.h"
#include "Unswitch.h"

#define COND arg_info->node[0]
#define LOOP_INFO arg_info->node[1]
#define ONCE_MORE arg_info->lineno

typedef enum { nothing, first, last, between, split, oneloop } todo;

typedef struct CINFO {
    prf test_prf;      /* test function                                         */
    long test_num;     /* test nummber                                          */
    long last_test;    /* last number which will be tested                      */
    todo todo;         /* what shall be done                                    */
    node *insert_node; /* ptr to assign-node the conditional has been linked to */
    node *chain1;      /* assign-chain                                          */
    node *chain2;      /* assign-chain                                          */
} cinfo;

/******************************************************************************
 *
 * Function:
 *   node *Unswitch(node *arg_node,node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
Unswitch (node *arg_node, node *arg_info)
{
    funtab *tmp_tab;
#ifndef DBUG_OFF
    int mem_uns_expr = uns_expr;
#endif

    DBUG_ENTER ("Unswitch");

    DBUG_PRINT ("OPT", ("LOOP UNSWITCHING"));
    DBUG_PRINT ("OPTMEM", ("mem currently allocated: %d bytes", current_allocated_mem));

    tmp_tab = act_tab;
    act_tab = unswitch_tab;
    arg_info = MakeInfo ();

    arg_node = Trav (arg_node, arg_info);

    FREE (arg_info);
    act_tab = tmp_tab;

    DBUG_PRINT ("OPT", ("                        result: %d", uns_expr - mem_uns_expr));
    DBUG_PRINT ("OPTMEM", ("mem currently allocated: %d bytes", current_allocated_mem));

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
 *                  - new entry will be pushed on the mrdl_stack
 *                  - generates two masks and links them to the info_node
 *                      [0] - variables additional defined in function and
 *                      [1] - variables additional used in function after optimization
 *                  - calls Trav to  unswitch loops in body of current function
 *                  - last entry will be poped from mrdl_stack
 *                  - updates masks in fundef node.
 *  global vars   : syntax_tree, mrdl_stack
 *  internal funs : PushMRDL, PopMRDL
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
    LEVEL = 1;

    if (NULL != FUNDEF_BODY (arg_node)) {
        do {
            ONCE_MORE = FALSE;
            FUNDEF_INSTR (arg_node)
              = OPTTrav (FUNDEF_INSTR (arg_node), arg_info, arg_node);
        } while (ONCE_MORE);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *UNSid(node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
UNSid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("UNSid");

    arg_node->flag = LEVEL;
    ID_DEF (arg_node) = MRD_GETLAST (ID_VARNO (arg_node), INFO_VARNO (arg_info));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *UNSlet(node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
UNSlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("UNSlet");

    /* genrate defined-pointer and maybe trav with-loop */
    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    arg_node->node[0]->flag = LEVEL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   prf InversePrf(prf fun)
 *
 * Description:
 *
 *
 ******************************************************************************/

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

/******************************************************************************
 *
 * Function:
 *   int DoesItHappen1(cinfo *cond_info, linfo *loop_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

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
            /*
             * to please the compiler, the following line is inserted. To make sure that
             * it doesn't harm, a DBUG_ASSERT follows!!
             */
            rest = 0;
            DBUG_ASSERT ((0),
                         "the variable rest will be referred to without initialization!");
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

/******************************************************************************
 *
 * Function:
 *   int DoesItHappen2(cinfo *cond_info, linfo *loop_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

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

/******************************************************************************
 *
 * Function:
 *   cinfo *HowUnswitch(cinfo *cond_info, linfo *loop_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

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
            cond_info->chain2 = FreeTree (cond_info->chain2);
        }
        break;

    case F_le:
        if (DoesItHappen2 (cond_info, loop_info)) {
            cond_info->todo = split;
        } else {
            if (loop_info->start_num > cond_info->test_num) {
                SWAP (cond_info->chain1, cond_info->chain2);
            }
            cond_info->todo = oneloop;
            cond_info->chain2 = FreeTree (cond_info->chain2);
        }
        break;

    case F_ge:
        if (DoesItHappen2 (cond_info, loop_info)) {
            cond_info->todo = split;
        } else {
            if (loop_info->start_num < cond_info->test_num) {
                SWAP (cond_info->chain1, cond_info->chain2);
            }
            cond_info->todo = oneloop;
            cond_info->chain2 = FreeTree (cond_info->chain2);
        }
        break;

    default:
        break;
    }

    DBUG_RETURN (cond_info);
}

/******************************************************************************
 *
 * Function:
 *   cinfo *AnalyseCond(linfo *loop_info, node *cond, int level)
 *
 * Description:
 *
 *
 ******************************************************************************/

cinfo *
AnalyseCond (linfo *loop_info, node *cond, int level)
{
    node *test, *arg[2], *cond_var;
    prf test_prf;
    cinfo *cond_info;

    DBUG_ENTER ("AnalyseCond");

    cond_info = (cinfo *)Malloc (sizeof (cinfo));
    cond_info->todo = nothing;
    cond_var = cond->node[0]->node[0];

    if (NULL != loop_info) {
        if (N_id == cond_var->nodetype) {
            test = cond_var->info.ids->def;
            if ((NULL != test) && (N_prf == NODE_TYPE (test))
                && (F_le <= (test_prf = test->info.prf)) && (F_neq >= test_prf)
                && (test->flag == level)) {

                /* the constant value shall be on the right side of the expression */
                /* i.e. cond = Num op i will be changed to cond = i op Num         */
                if (IsConst (PRF_ARG2 (test))) {
                    arg[0] = PRF_ARG1 (test);
                    arg[1] = PRF_ARG2 (test);
                } else {
                    test_prf = ReversePrf (test_prf);
                    arg[0] = PRF_ARG2 (test);
                    arg[1] = PRF_ARG1 (test);
                }

                /* adjust definition of conditional variable */
                test->info.prf = test_prf;
                PRF_ARG1 (test) = arg[0];
                PRF_ARG2 (test) = arg[1];

                /* do i have the same variable and is the calculations  */
                /* of the condition in  the loop ?                      */
                if ((N_id == arg[0]->nodetype)
                    && (loop_info->decl_node->varno == VARDEC_VARNO (ID_VARDEC (arg[0])))
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

/******************************************************************************
 *
 * Function:
 *   node *GenLetNode(linfo *loop_info, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
GenLetNode (linfo *loop_info, node *arg_info)
{
    node *assign_node;
    ids *let_ids;

    DBUG_ENTER ("GenLetNode");

    let_ids = MakeIds (StringCopy (VARDEC_OR_ARG_NAME (loop_info->decl_node)), NULL,
                       ST_regular);
    IDS_VARDEC (let_ids) = loop_info->decl_node;

    assign_node = MakeAssign (MakeLet (MakeNum (loop_info->start_num), let_ids), NULL);

    ASSIGN_MASK (assign_node, 0) = GenMask (INFO_VARNO (arg_info));
    ASSIGN_MASK (assign_node, 1) = GenMask (INFO_VARNO (arg_info));

    INC_VAR (ASSIGN_MASK (assign_node, 0), loop_info->decl_node->varno);
    INC_VAR (arg_info->mask[0], loop_info->decl_node->varno);

    DBUG_RETURN (assign_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DoUnswitch( node *arg_node, node *arg_info,
 *                     cinfo *cond_info, linfo *loop_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
DoUnswitch (node *arg_node, node *arg_info, cinfo *cond_info, linfo *loop_info)
{
    node *arg_node1;
    long old_test_num;
    node *second_loop = NULL;
    node *unrolled_loop = NULL;
    node *tmp_node;
    node *let_index;

    DBUG_ENTER ("DoUnswitch");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_assign),
                 "DoUnswitch(): arg_node must be a N_assign node!");
    DBUG_ASSERT ((NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_do),
                 "DoUnswitch(): arg_node must contain a do-loop!");
    DBUG_ASSERT ((NODE_TYPE (DO_COND (ASSIGN_INSTR (arg_node))) == N_id),
                 "Conditional of do-loop is not a N_id node!");

    switch (cond_info->todo) {

    case oneloop:
        DBUG_PRINT ("UNS",
                    ("conditional removed in loop at line %d", NODE_LINE (arg_node)));

        uns_expr++;
        /* subtract all variables used and defined in loop from modification masks */
        MinusMask (arg_info->mask[0], ASSIGN_MASK (arg_node, 0), INFO_VARNO (arg_info));
        MinusMask (arg_info->mask[1], ASSIGN_MASK (arg_node, 1), INFO_VARNO (arg_info));

        /* isolate loop */
        arg_node1 = ASSIGN_NEXT (arg_node);
        ASSIGN_NEXT (arg_node) = NULL;

        /* modify first loop */
        cond_info->insert_node->node[0] = cond_info->chain1;

        /* Generate masks */
        arg_node = GenerateMasks (arg_node, arg_info);

        /* append assign-chain below loop-assign */
        ASSIGN_NEXT (arg_node) = arg_node1;
        break;

    case between:
        DBUG_PRINT ("UNS", ("Move between loops at line %d", NODE_LINE (arg_node)));

        uns_expr++;
        /* subtract all variables used and defined in loop from modification masks */
        MinusMask (arg_info->mask[0], ASSIGN_MASK (arg_node, 0), INFO_VARNO (arg_info));
        MinusMask (arg_info->mask[1], ASSIGN_MASK (arg_node, 1), INFO_VARNO (arg_info));

        /* isolate loop */
        arg_node1 = ASSIGN_NEXT (arg_node); /* not used anymore ??? */
        ASSIGN_NEXT (arg_node) = NULL;

        if (cond_info->last_test != cond_info->test_num) {
            /* creat second loop */
            cond_info->insert_node->node[0] = cond_info->chain1;
            second_loop = DupTree (arg_node);

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
                second_loop = FreeTree (second_loop);
            }
        }

        /* create unrolled loop */
        cond_info->insert_node->node[0] = cond_info->chain2;
        unrolled_loop = DupTree (DO_INSTR (ASSIGN_INSTR (arg_node)));

        /* Generate masks */
        unrolled_loop = GenerateMasks (unrolled_loop, arg_info);

        /* modify first loop */
        cond_info->insert_node->node[0] = cond_info->chain1;
        ID_DEF (DO_COND (ASSIGN_INSTR (arg_node)))->info.prf = cond_info->test_prf;
        PRF_ARG2 (ID_DEF (DO_COND (ASSIGN_INSTR (arg_node))))->info.cint
          = cond_info->test_num;

        /* Generate masks */
        arg_node = GenerateMasks (arg_node, arg_info);

        /* append later assign nodes to second loop, ... */
        second_loop = AppendNodeChain (1, second_loop, ASSIGN_NEXT (arg_node));

        /* ... unrolled loop to second loop ... */
        unrolled_loop = AppendNodeChain (1, unrolled_loop, second_loop);

        /* ... and second loop to first loop */
        ASSIGN_NEXT (arg_node) = unrolled_loop;
        break;

    case split:
        DBUG_PRINT ("UNS", ("Splitting loop at line %d", NODE_LINE (arg_node)));

        uns_expr++;
        /* subtract all variables used and defined in loop from modification masks */
        MinusMask (arg_info->mask[0], ASSIGN_MASK (arg_node, 0), INFO_VARNO (arg_info));
        MinusMask (arg_info->mask[1], ASSIGN_MASK (arg_node, 1), INFO_VARNO (arg_info));

        /* isolate loop */
        arg_node1 = ASSIGN_NEXT (arg_node);
        ASSIGN_NEXT (arg_node) = NULL;

        if (cond_info->test_num != cond_info->last_test) {
            /* create second loop */
            /*
             * Here, we use a VERY UGLY TRICK !!!
             * cond_info->insert_node is the N_assign which holds the conditional
             * and cond_info->chain2 is the N_assign chain of the else-part.
             * Ergo, we create an N_assign with an N_assign as INSTR!!!
             * GNMassign in GenerateMasks eliminates this situation!
             */
            cond_info->insert_node->node[0] = cond_info->chain2;
            second_loop = DupTree (arg_node);

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
                /*
                 * Here, we try to apply Loop-Unrolling directly!
                 * That requires the masks to be set correctly!
                 */
                second_loop
                  = DoUnroll (second_loop->node[0], arg_info, (linfo *)loop_info);
                if (tmp_node->node[0] == second_loop) {
                    second_loop = tmp_node;
                    let_index = GenLetNode (loop_info, arg_info);
                    second_loop = AppendNodeChain (1, let_index, second_loop);
                }
            } else {
                second_loop = FreeTree (second_loop);
            }
        }

        /* modify first loop */
        /* Again, the UGLY TRICK is used here!! (see above) */
        cond_info->insert_node->node[0] = cond_info->chain1;
        ID_DEF (DO_COND (ASSIGN_INSTR (arg_node)))->info.prf = cond_info->test_prf;
        PRF_ARG2 (ID_DEF (DO_COND (ASSIGN_INSTR (arg_node))))->info.cint
          = cond_info->test_num;

        /* Generate masks */
        arg_node = GenerateMasks (arg_node, arg_info);

        /* append later assign nodes to second loop */
        second_loop = AppendNodeChain (1, second_loop, arg_node1);

        /* and second loop to first loop */
        ASSIGN_NEXT (arg_node) = second_loop;
        break;

    case nothing:
        /* here is no break missing */
    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *UNSassign(node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
UNSassign (node *arg_node, node *arg_info)
{
    cinfo *cond_info;
    int once_more = FALSE;
    node *tmp;

    DBUG_ENTER ("UNSassign");

    if (N_cond == arg_node->node[0]->nodetype) {
        COND = arg_node;
    }

    tmp = COND;

    /* unswitch subexpressions */
    ASSIGN_INSTR (arg_node) = OPTTrav (ASSIGN_INSTR (arg_node), arg_info, arg_node);

    if (N_do == NODE_TYPE (ASSIGN_INSTR (arg_node))) {
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

    if (!once_more) {
        do {
            ONCE_MORE = FALSE;
            ASSIGN_NEXT (arg_node) = OPTTrav (ASSIGN_NEXT (arg_node), arg_info, arg_node);
        } while (ONCE_MORE);
    } else {
        ONCE_MORE = TRUE;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *UNSdo(node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
UNSdo (node *arg_node, node *arg_info)
{
    linfo *loop_info;
    node *cond_node;

    DBUG_ENTER ("UNSdo");

    LEVEL++;
    COND = NULL;

    DBUG_PRINT ("UNS", ("Trav do loop in line %d", NODE_LINE (arg_node)));
    DO_INSTR (arg_node) = OPTTrav (DO_INSTR (arg_node), arg_info, arg_node);

    cond_node = DO_COND (arg_node);
    if (N_id == NODE_TYPE (cond_node)) {
        ID_DEF (cond_node) = MRD_GETLAST (ID_VARNO (cond_node), INFO_VARNO (arg_info));
    }

    loop_info = (linfo *)Malloc (sizeof (linfo));
    loop_info->loop_num = UNDEF;
    loop_info->ltype = N_do;

    /* Calculate numbers of iterations */
    if (N_id == cond_node->nodetype) {
        loop_info = AnalyseLoop (loop_info, cond_node, LEVEL);
    }

    if (2 > loop_info->loop_num) {
        LOOP_INFO = NULL;
        FREE (loop_info);
    } else {
        LOOP_INFO = (node *)loop_info;
    }

    LEVEL--;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *UNSwhile(node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
UNSwhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("UNSwhile");

    DBUG_PRINT ("UNS", ("Trav while loop in line %d", arg_node->lineno));
    WHILE_INSTR (arg_node) = OPTTrav (WHILE_INSTR (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *UNScond(node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
UNScond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("UNScond");

    /* Trav condition */
    COND_COND (arg_node) = OPTTrav (COND_COND (arg_node), arg_info, arg_node);

    LEVEL++;
    COND_THENINSTR (arg_node) = OPTTrav (COND_THENINSTR (arg_node), arg_info, arg_node);
    COND_ELSEINSTR (arg_node) = OPTTrav (COND_ELSEINSTR (arg_node), arg_info, arg_node);
    LEVEL--;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *UNSNwith(node *arg_node, node *arg_info)
 *
 * description:
 *   Unswitching for WLs is not don eyet. Just traverse into the bodies
 *   of the WLs.
 *
 ******************************************************************************/

node *
UNSNwith (node *arg_node, node *arg_info)
{
    node *tmpn;

    DBUG_ENTER ("UNSNwith");

    LEVEL++;

    /* traverse the N_Nwithop node */
    NWITH_WITHOP (arg_node) = OPTTrav (NWITH_WITHOP (arg_node), arg_info, arg_node);

    /* traverse all generators */
    tmpn = NWITH_PART (arg_node);
    while (tmpn) {
        tmpn = OPTTrav (tmpn, arg_info, arg_node);
        tmpn = NPART_NEXT (tmpn);
    }

    /* traverse bodies */
    tmpn = NWITH_CODE (arg_node);
    while (tmpn) {
        tmpn = OPTTrav (tmpn, arg_info, arg_node);
        tmpn = NCODE_NEXT (tmpn);
    }

    LEVEL--;

    DBUG_RETURN (arg_node);
}
