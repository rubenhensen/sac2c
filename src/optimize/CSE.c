/*
 *
 * $Log$
 * Revision 1.3  1996/02/13 13:59:53  asi
 * basic algorithm added
 *
 * Revision 1.1  1996/01/17  15:54:09  asi
 * Initial revision
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "free.h"
#include "print.h"
#include "Error.h"
#include "dbug.h"
#include "globals.h"
#include "my_debug.h"
#include "traverse.h"
#include "typecheck.h"
#include "internal_lib.h"

#include "optimize.h"
#include "DupTree.h"
#include "CSE.h"

#define FALSE 0
#define TRUE 1

/*
 *
 *  functionname  : CSE
 *  arguments     : 1) ptr to root of the syntaxtree or a N_fundef - node.
 *		    2) NULL
 *                  R) ptr to root of the optimized syntax-tree
 *  description   :i
 *  global vars   : syntax_tree, cse_tab, act_tab, mrdl_stack
 *  internal funs : ---
 *  external funs : Trav (traverse.h), MakeNode (tree_basic.h)
 *
 *  remarks       : --
 *
 *
 */
node *
CSE (node *arg_node, node *info_node)
{
    DBUG_ENTER ("CSE");
    act_tab = cse_tab;
    info_node = MakeNode (N_info);

    arg_node = Trav (arg_node, info_node);

    FREE (info_node);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CSEfundef
 *  arguments     : 1) N_fundef - node
 *		    2) N_info - node
 *		    R) N_fundef - node
 *  description   : calls OPTTrav to fold constants in current and following functions
 *  global vars   : syntax_tree, mrdl_stack
 *  internal funs : ---
 *  external funs : OPTTrav (optimize.h)
 *
 *  remarks       : ---
 *
 */
node *
CSEfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CSEfundef");

    DBUG_PRINT ("CSE", ("CSE function: %s", FUNDEF_NAME (arg_node)));
    if (NULL != FUNDEF_BODY (arg_node))
        FUNDEF_INSTR (arg_node) = OPTTrav (FUNDEF_INSTR (arg_node), arg_info, arg_node);
    FUNDEF_NEXT (arg_node) = OPTTrav (FUNDEF_NEXT (arg_node), arg_info, arg_node);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CSEwhile
 *  arguments     : 1) N_while - node
 *		    2) N_info  - node
 *		    R) N_while - node
 *  description   : initiates cse inside while-loop
 *  global vars   : syntax_tree, mrdl_stack
 *  internal funs : ---
 *  external funs : OPTTrav (optimize.h)
 *
 *  remarks       : ---
 *
 */
node *
CSEwhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CSEwhile");
    DBUG_PRINT ("CSE", ("CSE while-loop in line: %d START", NODE_LINE (arg_node)));

    WHILE_COND (arg_node) = OPTTrav (WHILE_COND (arg_node), arg_info, arg_node);
    WHILE_INSTR (arg_node) = OPTTrav (WHILE_INSTR (arg_node), arg_info, arg_node);

    DBUG_PRINT ("CSE", ("CSE while-loop in line: %d END", NODE_LINE (arg_node)));
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CSEdo
 *  arguments     : 1) N_do   - node
 *		    2) N_info - node
 *		    R) N_do   - node
 *  description   : initiates cse inside do-loop
 *  global vars   : syntax_tree, mrdl_stack
 *  internal funs : ---
 *  external funs : OPTTrav (optimize.h)
 *
 *  remarks       : ---
 *
 */
node *
CSEdo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CFdo");
    DBUG_PRINT ("CSE", ("CSE do-loop in line: %d START", NODE_LINE (arg_node)));

    DO_INSTR (arg_node) = OPTTrav (DO_INSTR (arg_node), arg_info, arg_node);
    DO_COND (arg_node) = OPTTrav (DO_COND (arg_node), arg_info, arg_node);

    DBUG_PRINT ("CSE", ("CSE do-loop in line: %d END", NODE_LINE (arg_node)));
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CSEcond
 *  arguments     : 1) N_cond - node
 *		    2) N_info - node
 *		    R) N_cond - node , N_assign - node or N_empty - node
 *  description   : initiates cse for the conditional
 *  global vars   : syntax_tree, mrdl_stack
 *  internal funs : ---
 *  external funs : OPTTrav (optimize.h)
 *
 *  remarks       : ---
 *
 */
node *
CSEcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CFcond");

    COND_COND (arg_node) = OPTTrav (COND_COND (arg_node), arg_info, arg_node);

    DBUG_PRINT ("CSE", ("CSE cond-then in line: %d START", NODE_LINE (arg_node)));
    COND_THENINSTR (arg_node) = OPTTrav (COND_THENINSTR (arg_node), arg_info, arg_node);
    DBUG_PRINT ("CSE", ("CSE cond-then in line: %d END", NODE_LINE (arg_node)));
    DBUG_PRINT ("CSE", ("CSE cond-else in line: %d START", NODE_LINE (arg_node)));
    COND_ELSEINSTR (arg_node) = OPTTrav (COND_ELSEINSTR (arg_node), arg_info, arg_node);
    DBUG_PRINT ("CSE", ("CSE cond-else in line: %d END", NODE_LINE (arg_node)));

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CSEwith
 *  arguments     : 1) N_with - node
 *                  2) N_info - node
 *                  R) N_with - node
 *  description   : Travereses generator, then opertator
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : OPTTrav (optimize.h)
 *
 *  remarks       : ---
 *
 */
node *
CSEwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CSEwith");
    DBUG_PRINT ("CSE", ("CSE with-loop in line: %d START", NODE_LINE (arg_node)));

    WITH_GEN (arg_node) = OPTTrav (WITH_GEN (arg_node), arg_info, arg_node);

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
    DBUG_PRINT ("CSE", ("CSE with-loop in line: %d END", NODE_LINE (arg_node)));
    DBUG_RETURN (arg_node);
}

node *
CSEid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CSEid");
    MRD_GETCSE (ID_DEF (arg_node), ID_VARNO (arg_node), INFO_VARNO);
    DBUG_RETURN (arg_node);
}

int
Equal (node *arg1, node *arg2, node *arg_info)
{
    int equal = FALSE;

    DBUG_ENTER ("Equal");
    if (N_id == NODE_TYPE (arg1))
        arg1 = MRD_GETCSE (ID_DEF (arg1), ID_VARNO (arg1), INFO_VARNO);
    if (N_id == NODE_TYPE (arg2))
        arg2 = ID_DEF (arg2);
    if (arg1 == arg2) {
        equal = TRUE;
        DBUG_PRINT ("CSE", (">Arguments are equal, same node"));
    } else {
        if ((NULL != arg1) && (NULL != arg2) && (NODE_TYPE (arg1) == NODE_TYPE (arg2))) {
            switch (NODE_TYPE (arg1)) {
            case N_num:
                if ((NUM_VAL (arg1) == NUM_VAL (arg2))) {
                    equal = TRUE;
                    DBUG_PRINT ("CSE", (">Arguments are equal, same int value"));
                }
                break;
            case N_bool:
                if ((BOOL_VAL (arg1) == BOOL_VAL (arg2))) {
                    equal = TRUE;
                    DBUG_PRINT ("CSE", (">Arguments are equal, same bool value"));
                }
                break;
            case N_float:
                if ((FLOAT_VAL (arg1) == FLOAT_VAL (arg2))) {
                    equal = TRUE;
                    DBUG_PRINT ("CSE", (">Arguments are equal, same float value"));
                }
                break;
            case N_double:
                if ((DOUBLE_VAL (arg1) == DOUBLE_VAL (arg2))) {
                    equal = TRUE;
                    DBUG_PRINT ("CSE", (">Arguments are equal, same double value"));
                }
                break;
            case N_char:
                if ((CHAR_VAL (arg1) == CHAR_VAL (arg2))) {
                    equal = TRUE;
                    DBUG_PRINT ("CSE", (">Arguments are equal, same char value"));
                }
                break;
            default:
                break;
            }
        }
    }
    DBUG_RETURN (equal);
}

node *
FindCS (node *arg1, node *arg2, node *arg_info)
{
    node *cs = NULL, *arg1_instr, *arg2_instr, *arg1_expr, *arg2_expr;
    int equal;

#define EQUAL_NTYPE (NODE_TYPE (arg1_expr) == NODE_TYPE (arg2_expr))

    DBUG_ENTER ("FindCS");

    if ((NULL != arg1) && (NULL != arg2)) {
        if (arg1 == arg2) {
            cs = arg2;
            DBUG_PRINT ("CSE", (">CSE, because same node"));
        } else {
            arg1_instr = ASSIGN_INSTR (arg1);
            arg2_instr = ASSIGN_INSTR (arg2);
            if ((N_let == NODE_TYPE (arg1_instr)) && (N_let == NODE_TYPE (arg2_instr))) {
                arg1_expr = LET_EXPR (arg1_instr);
                arg2_expr = LET_EXPR (arg2_instr);
                switch (NODE_TYPE (arg1_expr)) {
                case N_num:
                case N_bool:
                case N_float:
                case N_double:
                case N_char:
                    if (Equal (arg1_expr, arg2_expr, arg_info))
                        cs = arg2;
                    DBUG_PRINT ("CSE", (">CSE, same constant value"));
                    break;
                case N_prf:
                    if (EQUAL_NTYPE && (PRF_PRF (arg1_expr) == PRF_PRF (arg2_expr))) {
                        equal = TRUE;
                        arg1_expr = PRF_ARGS (arg1_expr);
                        arg2_expr = PRF_ARGS (arg2_expr);
                        do {
                            if (Equal (EXPRS_EXPR (arg1_expr), EXPRS_EXPR (arg2_expr),
                                       arg_info)) {
                                arg1_expr = EXPRS_NEXT (arg1_expr);
                                arg2_expr = EXPRS_NEXT (arg2_expr);
                            } else
                                equal = FALSE;
                        } while (equal && ((NULL != arg1_expr) || (NULL != arg2_expr)));
                        if (equal) {
                            DBUG_PRINT ("CSE", (">CSE, same primitive function"));
                            cs = arg2;
                        }
                    }
                    break;
                case N_ap:
                    if (EQUAL_NTYPE && (AP_FUNDEF (arg1_expr) == AP_FUNDEF (arg2_expr))) {
                        equal = TRUE;
                        arg1_expr = AP_ARGS (arg1_expr);
                        arg2_expr = AP_ARGS (arg2_expr);
                        do {
                            if (Equal (EXPRS_EXPR (arg1_expr), EXPRS_EXPR (arg2_expr),
                                       arg_info)) {
                                arg1_expr = EXPRS_NEXT (arg1_expr);
                                arg2_expr = EXPRS_NEXT (arg2_expr);
                            } else
                                equal = FALSE;
                        } while (equal && ((NULL != arg1_expr) || (NULL != arg2_expr)));
                        if (equal) {
                            DBUG_PRINT ("CSE", (">CSE, same user defined function"));
                            cs = arg2;
                        }
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }
    DBUG_RETURN (cs);
}

node *
Eliminate (node *arg_node, node *equal_node, node *arg_info)
{
    node *new_node;
    ids *ids_node;
    node *id_node, *let_node;

    DBUG_ENTER ("Eliminate");
    switch (NODE_TYPE (ASSIGN_INSTR (LET_EXPR (arg_node)))) {
    case N_num:
    case N_float:
    case N_double:
    case N_str:
    case N_bool:
        ids_node = DupIds (LET_IDS (ASSIGN_INSTR (equal_node)), arg_info);
        id_node = MakeId2 (ids_node);
        ids_node = DupIds (LET_IDS (ASSIGN_INSTR (arg_node)), arg_info);
        let_node = MakeLet (id_node, ids_node);
        new_node = MakeAssign (let_node, NULL);
        break;
    case N_prf:
        if (CheckScope (MRD_LIST, equal_node, INFO_VARNO, TRUE)) {
            ids_node = DupIds (LET_IDS (ASSIGN_INSTR (equal_node)), arg_info);
            id_node = MakeId2 (ids_node);
            ids_node = DupIds (LET_IDS (ASSIGN_INSTR (arg_node)), arg_info);
            let_node = MakeLet (id_node, ids_node);
            new_node = MakeAssign (let_node, NULL);
        } else
            new_node = NULL;
        break;
    case N_ap:
        if (CheckScope (MRD_LIST, equal_node, INFO_VARNO, TRUE)) {
            ids_node = DupIds (LET_IDS (ASSIGN_INSTR (equal_node)), arg_info);
            id_node = MakeId2 (ids_node);
            ids_node = DupIds (LET_IDS (ASSIGN_INSTR (arg_node)), arg_info);
            let_node = MakeLet (id_node, ids_node);
            new_node = MakeAssign (let_node, NULL);
        } else
            new_node = NULL;
        break;
    default:
        new_node = NULL;
        break;
    }
    DBUG_RETURN (new_node);
}

/*
 *
 *  functionname  : CSEassign
 *  arguments     : 1) N_with - node
 *                  2) N_info - node
 *                  R) N_with - node
 *  description   : Travereses generator, then opertator
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : OPTTrav (optimize.h)
 *
 *  remarks       : ---
 *
 */
node *
CSEassign (node *arg_node, node *arg_info)
{
    int i;
    int next = TRUE;
    node *cmp_node, *equal_node, *new_node, *stack_node;

    DBUG_ENTER ("CSEassign");

    ASSIGN_CSE (arg_node) = NULL;

    if (NULL == (cmp_node = GetCompoundNode (arg_node))) {
        i = 0;
        equal_node = NULL;
        DBUG_PRINT ("CSE",
                    ("Searching common subexpression for line %d", NODE_LINE (arg_node)));

        while ((i < INFO_VARNO) && (NULL == equal_node)) {
            stack_node = MRD (i);
            equal_node = FindCS (arg_node, stack_node, arg_info);
            i++;
        }

        if (NULL != equal_node) {
            DBUG_PRINT ("CSE", (">Found common subexpression in line %d",
                                NODE_LINE (equal_node)));
            new_node = Eliminate (arg_node, equal_node, arg_info);
            if (NULL != new_node) {
                DBUG_PRINT ("CSE", (">Common subexpression eliminated in line %d",
                                    NODE_LINE (arg_node)));
                cse_expr++;
                MinusMask (INFO_DEF, ASSIGN_DEFMASK (arg_node), INFO_VARNO);
                MinusMask (INFO_USE, ASSIGN_USEMASK (arg_node), INFO_VARNO);
                new_node = GenerateMasks (new_node, arg_info);
                ASSIGN_NEXT (new_node) = ASSIGN_NEXT (arg_node);
/*-----------------------------------------------------------------------------------*/
#ifndef NEWTREE
                if (NULL == ASSIGN_NEXT (new_node))
                    new_node->nnode = 1;
                else
                    new_node->nnode = 2;
#endif
                /*-----------------------------------------------------------------------------------*/
                ASSIGN_NEXT (arg_node) = NULL;
/*-----------------------------------------------------------------------------------*/
#ifndef NEWTREE
                arg_node->nnode = 1;
#endif
                /*-----------------------------------------------------------------------------------*/
                FreeTree (arg_node);
                arg_node = CSEassign (new_node, arg_info);
                next = FALSE;
            }
        }
    }

    if (next) {
        ASSIGN_INSTR (arg_node) = OPTTrav (ASSIGN_INSTR (arg_node), arg_info, arg_node);
        ASSIGN_NEXT (arg_node) = OPTTrav (ASSIGN_NEXT (arg_node), arg_info, arg_node);
        if (NULL != ASSIGN_CSE (arg_node)) {
        }
    }

    DBUG_RETURN (arg_node);
}
