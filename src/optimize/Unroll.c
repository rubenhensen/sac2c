/*
 *
 * $Log$
 * Revision 3.5  2001/03/22 21:10:51  dkr
 * no changes done
 *
 * Revision 3.4  2001/02/13 17:16:59  dkr
 * MakeNode() eliminated
 *
 * Revision 3.3  2001/02/02 10:16:38  dkr
 * superfluous import of access_macros.h removed
 *
 * Revision 3.2  2000/11/23 16:29:17  sbs
 * warnings of product version eliminated.
 *
 * Revision 3.1  2000/11/20 18:00:35  sacbase
 * new release made
 *
 * Revision 2.11  2000/10/10 14:28:17  dkr
 * comment added
 *
 * Revision 2.10  2000/10/10 12:45:28  dkr
 * no changes done
 *
 * Revision 2.9  2000/06/23 15:19:38  dkr
 * signature of DupTree changed
 *
 * Revision 2.8  2000/06/13 12:31:12  dkr
 * function for old with-loop removed
 *
 * Revision 2.7  2000/02/23 19:08:50  cg
 * The optimizations LUR and WLUR now issue a notification if
 * the setting of -maxlur or -maxwlur, respectively, prevents
 * the unrolling of an otherwise unrollable loop or with-loop.
 * Currently the upper boundary for issuing such a note is set to 32.
 *
 * Revision 2.6  2000/01/26 17:27:17  dkr
 * type of traverse-function-table changed.
 *
 * Revision 2.5  1999/11/15 18:05:12  dkr
 * VARNO replaced, INFO_VARNO with changed signature
 *
 * Revision 2.4  1999/05/12 14:35:16  cg
 * Optimizations are now triggered by bit field optimize instead
 * of single individual int variables.
 *
 * Revision 2.3  1999/04/19 12:50:45  jhs
 * TRUE and FALSE from internal_lib.h used from now on.
 *
 * Revision 2.2  1999/03/31 15:06:17  bs
 * I did some code cosmetics with the MRD_GET... macros.
 *
 * Revision 2.1  1999/02/23 12:41:26  sacbase
 * new release made
 *
 * Revision 1.17  1999/01/18 15:46:02  sbs
 * DBUG_PRINT( "OPTMEM",...) inserted for mem-info during optimization
 *
 * Revision 1.16  1999/01/07 13:56:58  sbs
 * optimization process restructured for a function-wise optimization!
 *
 * Revision 1.15  1998/07/16 15:56:27  srs
 * initialized INFO_UNR_FUNDEF
 *
 * Revision 1.14  1998/05/15 14:42:45  srs
 * removed NEWTREE
 * added WL unrolling (UNRNwith() UNRassign())
 *
 * Revision 1.13  1998/05/13 13:46:35  srs
 * added WL unrolling
 *
 * Revision 1.12  1998/02/26 12:35:05  srs
 * traversal through new WLs possible
 *
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
#include "WLUnroll.h"

/******************************************************************************
 *
 * Function:
 *   prf ReversePrf(prf fun)
 *
 * Description:
 *
 *
 ******************************************************************************/

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

/******************************************************************************
 *
 * Function:
 *   linfo *LoopIterations(linfo *loop_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

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
        } else {
            if (N_do == loop_info->ltype) {
                loop_info->loop_num = 1;
            } else {
                loop_info->loop_num = 0;
            }
        }

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
        } else {
            if (N_do == loop_info->ltype) {
                loop_info->loop_num = 1;
            } else {
                loop_info->loop_num = 0;
            }
        }

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
        } else {
            if (N_do == loop_info->ltype) {
                loop_info->loop_num = 1;
            } else {
                loop_info->loop_num = 0;
            }
        }

        /* calculate value of index variable after the loop */
        loop_info->end_num
          = loop_info->start_num * (loop_info->loop_num * loop_info->mod_num);
        break;

    default:
        loop_info->loop_num = UNDEF;
    }

    DBUG_RETURN (loop_info);
}

/******************************************************************************
 *
 * Function:
 *   linfo *AnalyseLoop(linfo *loop_info, node *id_node, int level)
 *
 * Description:
 *
 *
 ******************************************************************************/

linfo *
AnalyseLoop (linfo *loop_info, node *id_node, int level)
{
    node *init, *test, *mod, *index = NULL, *arg[2];
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
        } else {
            allright = FALSE;
        }

        if (N_num == NODE_TYPE (arg[1])) {
            loop_info->test_num = NUM_VAL (arg[1]);
        } else {
            allright = FALSE;
        }
    } else {
        allright = FALSE;
    }

    /* Do I have somthing like i = i op Num1 ...  cond = i op Num2 */
    /*                               ^^^^^^^^^             |         */
    /*                                  mod  <-------------|         */
    if (allright) { /* this implies that index is set!! */
        mod = ID_DEF (index);
        if ((NULL != mod) && (N_prf == NODE_TYPE (mod))
            && ((F_add == (loop_info->mod_prf = PRF_PRF (mod)))
                || (F_sub == loop_info->mod_prf) || (F_mul == loop_info->mod_prf))
            && (mod->flag == level)) {

            arg[0] = PRF_ARG1 (mod);
            arg[1] = PRF_ARG2 (mod);

            if ((N_id == NODE_TYPE (arg[0]))
                && (VARDEC_VARNO (loop_info->decl_node) == ID_VARNO (arg[0]))
                && (N_num == NODE_TYPE (arg[1])) && (0 != NUM_VAL (arg[1]))) {
                loop_info->mod_num = NUM_VAL (arg[1]);
                index = arg[0];
            } else {
                if ((N_id == NODE_TYPE (arg[1]))
                    && (loop_info->decl_node->varno == ID_VARNO (arg[1]))
                    && (N_num == NODE_TYPE (arg[0])) && (0 != NUM_VAL (arg[0]))
                    && (F_sub != loop_info->mod_prf)) {
                    loop_info->mod_num = NUM_VAL (arg[0]);
                    index = arg[1];
                } else {
                    allright = FALSE;
                }
            }
        } else {
            allright = FALSE;
        }
    }

    if (allright) {
        init = index->info.ids->def;

        /* index variable must be initialize with a constant value */
        if ((NULL != init) && (init->flag < level) && (N_num == init->nodetype)) {
            loop_info->start_num = init->info.cint;
        } else {
            allright = FALSE;
        }
    }

    if (allright) {
        if ((F_mul == loop_info->mod_prf)
            && ((0 == loop_info->mod_num) || (0 == loop_info->start_num))) {
            allright = FALSE;
        } else {
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
            } else {
                allright = FALSE;
            }
            break;

        case F_lt:
            if ((F_add == loop_info->mod_prf) || (F_mul == loop_info->mod_prf)) {
                loop_info->test_prf = F_le;
                loop_info->test_num--;
            } else {
                allright = FALSE;
            }
            break;

        case F_le:
            if (!((F_add == loop_info->mod_prf) || (F_mul == loop_info->mod_prf))) {
                allright = FALSE;
            }
            break;

        case F_ge:
            if (!(F_sub == loop_info->mod_prf)) {
                allright = FALSE;
            }
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

/******************************************************************************
 *
 * Function:
 *   node *DoUnroll(node *arg_node, node *arg_info, linfo *loop_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
DoUnroll (node *arg_node, node *arg_info, linfo *loop_info)
{
    int i;
    node *tmp;
    node *unroll = NULL;

    DBUG_ENTER ("DoUnroll");

    if (optimize & OPT_LUR) {
        if (loop_info->loop_num <= unrnum) {
            DBUG_PRINT ("UNR", ("Unrolling %d times %s in line %d", loop_info->loop_num,
                                mdb_nodetype[arg_node->nodetype], arg_node->lineno));
            lunr_expr++;

            switch (loop_info->loop_num) {
            case 0:
                MinusMask (arg_info->mask[0], arg_node->node[1]->mask[0],
                           INFO_VARNO (arg_info));
                MinusMask (arg_info->mask[1], arg_node->node[1]->mask[1],
                           INFO_VARNO (arg_info));
                MinusMask (arg_info->mask[1], arg_node->mask[1], INFO_VARNO (arg_info));
                FreeTree (arg_node);
                arg_node = MakeEmpty ();
                break;

            case 1:
                MinusMask (arg_info->mask[1], arg_node->mask[1], INFO_VARNO (arg_info));
                unroll = arg_node->node[1]->node[0];
                arg_node->node[1]->node[0] = NULL;
                FreeTree (arg_node);
                arg_node = unroll;
                break;

            default:
                MinusMask (arg_info->mask[0], arg_node->node[1]->mask[0],
                           INFO_VARNO (arg_info));
                MinusMask (arg_info->mask[1], arg_node->node[1]->mask[1],
                           INFO_VARNO (arg_info));
                MinusMask (arg_info->mask[1], arg_node->mask[1], INFO_VARNO (arg_info));
                for (i = 0; i < loop_info->loop_num; i++) {
                    tmp = DupTree (arg_node->node[1]->node[0]);
                    unroll = AppendNodeChain (1, unroll, tmp);
                }
                unroll = GenerateMasks (unroll, arg_info);
                FreeTree (arg_node);
                arg_node = unroll;
                break;
            }
        } else {
            if (loop_info->loop_num <= 32) {
                NOTE (("LUR: -maxlur %d would unroll loop", loop_info->loop_num));
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *UNRfundef(node *arg_node, node *arg_info)
 *
 * Description:
 *   - generates info_node
 *   - varno of the info_node will be set to the number of local variables
 *     and arguments in this functions
 *   - new entry will be pushed on the mrdl_stack
 *   - generates two masks and links them to the info_node
 *       [0] - variables additional defined in function and
 *       [1] - variables additional used in function after optimization
 *   - calls Trav to  unrolled loops in body of current function
 *   - last entry will be poped from mrdl_stack
 *   - updates masks in fundef node.
 *
 ******************************************************************************/

node *
UNRfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("UNRfundef");

    DBUG_PRINT ("UNR", ("Unroll in function: %s", arg_node->info.types->id));
    LEVEL = 1;

    /* needed to access the vardec in DoUnrollFold() in file  WLUnroll.c */
    INFO_UNR_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node)) {
        FUNDEF_INSTR (arg_node) = OPTTrav (FUNDEF_INSTR (arg_node), arg_info, arg_node);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *UNRid(node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
UNRid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("UNRid");

    arg_node->flag = LEVEL;
    ID_DEF (arg_node) = MRD_GETLAST (ID_VARNO (arg_node), INFO_VARNO (arg_info));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *UNRlet(node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
UNRlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("UNRlet");

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
    LET_EXPR (arg_node)->flag = LEVEL; /* Aaaarggggghhhh $&%*@ ... :-(( */

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *UNRdo(node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
UNRdo (node *arg_node, node *arg_info)
{
    linfo *loop_info = NULL;
    node *cond_node;

    DBUG_ENTER ("UNRdo");

    LEVEL++;

    DBUG_PRINT ("UNR", ("Trav do loop in line %d", NODE_LINE (arg_node)));
    DO_INSTR (arg_node) = OPTTrav (DO_INSTR (arg_node), arg_info, arg_node);

    if (optimize & OPT_LUR) {
        cond_node = DO_COND (arg_node);

        if (N_id == NODE_TYPE (cond_node)) {
            ID_DEF (cond_node)
              = MRD_GETLAST (ID_VARNO (cond_node), INFO_VARNO (arg_info));
        }

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
    }

    LEVEL--;
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *UNRwhile(node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
UNRwhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("UNRwhile");

    DBUG_PRINT ("UNR", ("Trav while loop in line %d", NODE_LINE (arg_node)));
    WHILE_INSTR (arg_node) = OPTTrav (WHILE_INSTR (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *UNRcond(node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

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

/******************************************************************************
 *
 * function:
 *   node *UNRNwith(node *arg_node, node *arg_info)
 *
 * description:
 *   at the moment, unrolling for WL is not done. Here we just
 *   traverse the bodies of the WL.
 *
 ******************************************************************************/

node *
UNRNwith (node *arg_node, node *arg_info)
{
    node *tmpn, *save;

    DBUG_ENTER ("UNRNwith");

    LEVEL++;

    save = INFO_UNR_ASSIGN (arg_info);

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

    INFO_UNR_ASSIGN (arg_info) = save;

    if (optimize & OPT_WLUR) {
        /* can this WL be unrolled? */
        switch (NWITH_TYPE (arg_node)) {
        case WO_modarray:
            if (CheckUnrollModarray (arg_node)) {
                wlunr_expr++;
                /* remove old mask information */
                MinusMask (INFO_DEF, ASSIGN_MASK (save, 0), INFO_VARNO (arg_info));
                MinusMask (INFO_USE, ASSIGN_MASK (save, 1), INFO_VARNO (arg_info));
                /* unroll and generate new masks */
                tmpn = DoUnrollModarray (arg_node,
                                         arg_info); /* returns list of assignments */
                tmpn = GenerateMasks (tmpn, arg_info);
                /* delete old WL and insert new code */
                FreeTree (arg_node);
                arg_node = tmpn;
            }
            break;

        case WO_genarray:
            if (CheckUnrollGenarray (arg_node, arg_info)) {
                wlunr_expr++;
                /* remove old mask information */
                MinusMask (INFO_DEF, ASSIGN_MASK (save, 0), INFO_VARNO (arg_info));
                MinusMask (INFO_USE, ASSIGN_MASK (save, 1), INFO_VARNO (arg_info));
                /* unroll and generate new masks */
                tmpn = DoUnrollGenarray (arg_node,
                                         arg_info); /* returns list of assignments */
                tmpn = GenerateMasks (tmpn, arg_info);
                /* delete old WL and insert new code */
                FreeTree (arg_node);
                arg_node = tmpn;
            }
            break;

        default:
            if (CheckUnrollFold (arg_node)) {
                wlunr_expr++;
                /* remove old mask information */
                MinusMask (INFO_DEF, ASSIGN_MASK (save, 0), INFO_VARNO (arg_info));
                MinusMask (INFO_USE, ASSIGN_MASK (save, 1), INFO_VARNO (arg_info));
                /* unroll and generate new masks */
                tmpn
                  = DoUnrollFold (arg_node, arg_info); /* returns list of assignments */
                tmpn = GenerateMasks (tmpn, arg_info);
                /* delete old WL and insert new code */
                FreeTree (arg_node);
                arg_node = tmpn;
            }
            break;
        }
    }

    LEVEL--;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *UNRassign(node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
UNRassign (node *arg_node, node *arg_info)
{
    node *tmp_node;
    nodetype ntype;

    DBUG_ENTER ("UNRassign");

    ntype = NODE_TYPE (ASSIGN_INSTR (arg_node));

    /* unroll subexpressions */
    INFO_UNR_ASSIGN (arg_info) = arg_node;
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

    case N_let:
        /* if WL unrolling was done (modarray and fold), an N_assign node is
           received and put into the LET_EXPR. Insert this subtree into the tree. */
        if (N_assign == NODE_TYPE (LET_EXPR (ASSIGN_INSTR (arg_node)))) {
            tmp_node = AppendNodeChain (1, LET_EXPR (ASSIGN_INSTR (arg_node)),
                                        ASSIGN_NEXT (arg_node));
            LET_EXPR (ASSIGN_INSTR (arg_node)) = NULL;
            ASSIGN_NEXT (arg_node) = NULL;
            FreeTree (arg_node);
            arg_node = UNRassign (tmp_node, arg_info);
        } else {
            ASSIGN_NEXT (arg_node) = OPTTrav (ASSIGN_NEXT (arg_node), arg_info, arg_node);
        }
        break;

    default:
        ASSIGN_NEXT (arg_node) = OPTTrav (ASSIGN_NEXT (arg_node), arg_info, arg_node);
        break;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *Unroll(node *arg_node,node *arg_info)
 *
 * Description:
 *   Top-level function for loop-unrolling. Starts the traversal.
 *
 ******************************************************************************/

node *
Unroll (node *arg_node, node *arg_info)
{
#ifndef DBUG_OFF
    int mem_lunr_expr = lunr_expr;
    int mem_wlunr_expr = wlunr_expr;
#endif
    funtab *tmp_tab;

    DBUG_ENTER ("Unroll");

    DBUG_PRINT ("OPT", ("LOOP/WL UNROLLING"));
    DBUG_PRINT ("OPTMEM", ("mem currently allocated: %d bytes", current_allocated_mem));

    tmp_tab = act_tab;
    act_tab = unroll_tab;
    arg_info = MakeInfo ();

    arg_node = Trav (arg_node, arg_info);

    arg_info = FreeTree (arg_info);
    act_tab = tmp_tab;

    DBUG_PRINT ("OPT", ("                   LOOP result: %d", lunr_expr - mem_lunr_expr));
    DBUG_PRINT ("OPT",
                ("                     WL result: %d", wlunr_expr - mem_wlunr_expr));
    DBUG_PRINT ("OPTMEM", ("mem currently allocated: %d bytes", current_allocated_mem));

    DBUG_RETURN (arg_node);
}
