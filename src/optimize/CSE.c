/*
 *
 * $Log$
 * Revision 1.1  1996/01/17 15:54:09  asi
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

    WHILE_COND (arg_node) = OPTTrav (WHILE_COND (arg_node), arg_info, arg_node);
    WHILE_INSTR (arg_node) = OPTTrav (WHILE_INSTR (arg_node), arg_info, arg_node);

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

    DO_INSTR (arg_node) = OPTTrav (DO_INSTR (arg_node), arg_info, arg_node);
    DO_COND (arg_node) = OPTTrav (DO_COND (arg_node), arg_info, arg_node);

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

    COND_THENINSTR (arg_node) = OPTTrav (COND_THENINSTR (arg_node), arg_info, arg_node);
    COND_ELSEINSTR (arg_node) = OPTTrav (COND_ELSEINSTR (arg_node), arg_info, arg_node);

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
 *  macros        : INFO_TYPE, WITH_GEN, GEN_VARDEC, VARDEC_TYPE, WITH_OPERATOR,
 *                  NODE_TYPE, GENARRAY_BODY, MODARRAY_BODY, FOLDPRF_BODY, FOLDFUN_BODY,
 *                  BLOCK_INSTR
 *
 *  remarks       : ---
 *
 */
node *
CSEwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CSEwith");

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
    DBUG_RETURN (arg_node);
}
