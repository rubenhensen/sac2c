
#include "tree.h"
#include "traverse.h"
#include "free.h"
#include "dbug.h"

/*
 *
 *  functionname  : SeperateBlockReturn
 *  arguments     : 1) an expr-block
 *  description   : seperates the return-expr of a block from the rest.
 *                  (returns the return-expr [return value] and the rest of the block
 * [block])
 *
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : FreeAssign
 *  macros        : ---
 *
 *  remarks       : as a matter of course, the return-expr should be the last expr of the
 * block
 *
 */

node *
SeperateBlockReturn (node **block_node)
{
    node *last_assign;
    node *return_node;
    node *free_node;
    node *tmp;

    DBUG_ENTER ("SeperateBlockReturn");

    DBUG_ASSERT ((BLOCK_VARDEC ((*block_node)) == NULL), "vardec in with-loop-block");

    /* search for the last but one assign-node */
    last_assign = BLOCK_INSTR ((*block_node));
    if (ASSIGN_NEXT (last_assign) == NULL) {
        /* the block contains only the return-expr */
        tmp = ASSIGN_INSTR (last_assign);
        free_node = last_assign = (*block_node);
        (*block_node) = NULL;
    } else {
        while (ASSIGN_NEXT (ASSIGN_NEXT (last_assign)) != NULL)
            last_assign = ASSIGN_NEXT (last_assign);
        tmp = ASSIGN_INSTR (ASSIGN_NEXT (last_assign));
        free_node = ASSIGN_NEXT (last_assign);
    }
    DBUG_ASSERT ((NODE_TYPE (tmp) == N_return), "Last expression of block is no return");

    return_node = EXPRS_EXPR (RETURN_EXPRS (tmp)); /* get return-expr */
    ASSIGN_NEXT (last_assign) = NULL;              /* remove return-expr from block */
    EXPRS_EXPR (RETURN_EXPRS (tmp)) = NULL;        /* do not free return-expr */

    FreeAssign (free_node,
                free_node); /* free the nodes between return-expr and rest of block */

    DBUG_RETURN (return_node);
}

/*
 *
 *  functionname  : O2Nwith
 *  arguments     : 1) argument node
 *                  2) info node
 *  description   : converts N_with-nodes to N_Nwith
 *
 *  global vars   : ---
 *  internal funs : SeperateBlockReturn
 *  external funs : MakeNWith(), ..., FreeWith()
 *  macros        : ---
 *
 *  remarks       :
 *
 */

node *
O2Nwith (node *arg_node, node *arg_info)
{
    node *old_generator;
    node *old_operator;
    node *new_withid;
    node *new_withop;
    node *new_block;
    node *new_expr;
    node *new_generator;
    node *new_code;
    node *new_part;

    DBUG_ENTER ("O2Nwith");

    old_generator = Trav (WITH_GEN (arg_node), arg_info);
    old_operator = Trav (WITH_OPERATOR (arg_node), arg_info);

    new_withid
      = MakeNWithid (WI_vector, MakeIds (GEN_ID (old_generator), NULL, ST_regular));
    new_generator
      = MakeNGenerator (GEN_LEFT (old_generator),
                        MakePrf (F_add, MakeExprs (GEN_RIGHT (old_generator),
                                                   MakeExprs (MakeNum (1), NULL))),
                        F_le, F_lt, NULL, NULL);
    GEN_LEFT (old_generator) = GEN_RIGHT (old_generator) = NULL;

    switch (NODE_TYPE (old_operator)) {
    case N_genarray:
        new_withop = MakeNWithOp (WO_genarray);
        NWITHOP_SHAPE (new_withop) = GENARRAY_ARRAY (old_operator);
        new_block = GENARRAY_BODY (old_operator);

        GENARRAY_ARRAY (old_operator) = NULL;
        GENARRAY_BODY (old_operator) = NULL;
        break;
    case N_modarray:
        new_withop = MakeNWithOp (WO_modarray);
        NWITHOP_ARRAY (new_withop) = MODARRAY_ARRAY (old_operator);
        new_block = MODARRAY_BODY (old_operator);

        MODARRAY_ARRAY (old_operator) = NULL;
        MODARRAY_BODY (old_operator) = NULL;
        break;
    case N_foldprf:
        new_withop = MakeNWithOp (WO_foldprf);
        NWITHOP_PRF (new_withop) = FOLDPRF_PRF (old_operator);
        NWITHOP_NEUTRAL (new_withop) = FOLDPRF_NEUTRAL (old_operator);
        new_block = FOLDPRF_BODY (old_operator);

        FOLDPRF_NEUTRAL (old_operator) = NULL;
        FOLDPRF_BODY (old_operator) = NULL;
        break;
    case N_foldfun:
        new_withop = MakeNWithOp (WO_foldfun);
        NWITHOP_FUN (new_withop) = FOLDFUN_NAME (old_operator);
        NWITHOP_MOD (new_withop) = FOLDFUN_MOD (old_operator);
        NWITHOP_NEUTRAL (new_withop) = FOLDFUN_NEUTRAL (old_operator);
        new_block = FOLDFUN_BODY (old_operator);

        FOLDPRF_NEUTRAL (old_operator) = NULL;
        FOLDPRF_BODY (old_operator) = NULL;
        break;
    default:
        DBUG_ASSERT (0, "Unknown type of with-loop-operator");
        break;
    }
    new_expr = SeperateBlockReturn (&new_block);

    new_code = MakeNCode (new_block, new_expr);
    new_part = MakeNPart (new_withid, new_generator);
    NPART_CODE (new_part) = new_code;

    FreeWith (arg_node, arg_node);

    arg_node = MakeNWith (new_part, new_code, new_withop);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : Old2NewWith
 *  arguments     : 1) syntax tree
 *  description   : initializes o2nWith_tab and starts the conversion
 *
 *  global vars   : act_tab
 *  internal funs :
 *  external funs : Trav
 *  macros        : DBUG...,
 *  remarks       : ----
 *
 */

node *
Old2NewWith (node *arg_node)
{
    DBUG_ENTER ("Old2NewWith");

    act_tab = o2nWith_tab; /* set new function-table for traverse */

    arg_node = Trav (arg_node, NULL);

    DBUG_RETURN (arg_node);
}
