

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
SeperateBlockReturn (node *block_node)
{
    node *last_assign;
    node *return_node;
    node *tmp;

    DBUG_ENTER ("SeperateBlockReturn");

    DBUG_ASSERT ((BLOCK_VARDEC (block_node) == NULL), "vardec in with-loop-block");

    /* search for the last but one assign-node */
    last_assign = BLOCK_INSTR (block_node);
    if (ASSIGN_NEXT (last_assign) == NULL) {
        last_assign = block_node;
    } else {
        while (ASSIGN_NEXT (ASSIGN_NEXT (last_assign)) != NULL)
            last_assign = ASSIGN_NEXT (last_assign);
    }
    tmp = ASSIGN_NEXT (last_assign);
    ASSIGN_NEXT (last_assign) = NULL; /* remove return-expr from block */

    DBUG_ASSERT ((NODE_TYPE (ASSIGN_INSTR (tmp)) == N_return),
                 "Last expression of block is no return");

    return_node = EXPRS_EXPR (RETURN_EXPRS (ASSIGN_INSTR (tmp))); /* get return-expr */
    EXPRS_EXPR (RETURN_EXPRS (ASSIGN_INSTR (tmp))) = NULL; /* do not free return-expr */

    FreeAssign (tmp, tmp); /* free the nodes between return-expr and rest of block */

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
    node *old_gen;
    node *old_op;
    node *new_withid;
    node *new_withop;
    node *new_block;
    node *new_expr;
    node *new_gen;
    ids *new_withid_ids;

    DBUG_ENTER ("O2Nwith");

    old_gen = WITH_GEN (arg_node);
    old_op = WITH_OPERATOR (arg_node);

    new_withid_ids = MakeIds (GEN_ID (old_gen), NULL, ST_regular);
    IDS_VARDEC (new_withid_ids) = GEN_VARDEC (old_gen);
    IDS_USE (new_withid_ids) = GEN_USE (old_gen);

    new_withid = MakeNWithid (1, new_withid_ids);
    new_gen = MakeNGenerator (GEN_LEFT (old_gen),
                              MakePrf (F_add, MakeExprs (GEN_RIGHT (old_gen),
                                                         MakeExprs (MakeNum (1), NULL))),
                              F_le, F_lt, NULL, NULL);
    GEN_LEFT (old_gen) = GEN_RIGHT (old_gen) = NULL;

    switch (NODE_TYPE (old_op)) {
    case N_genarray:
        new_withop = MakeNWithOp (WO_genarray);
        NWITHOP_SHAPE (new_withop) = GENARRAY_ARRAY (old_op);
        new_block = GENARRAY_BODY (old_gen);

        GENARRAY_ARRAY (old_gen) = NULL;
        GENARRAY_BODY (old_gen) = NULL;
        break;
    case N_modarray:
        new_withop = MakeNWithOp (WO_modarray);
        NWITHOP_ARRAY (new_withop) = MakeId (MODARRAY_ID (old_op), NULL, ST_regular);
        FreeArray (MODARRAY_ARRAY (old_gen), MODARRAY_ARRAY (old_gen));
        new_block = MODARRAY_BODY (old_gen);

        MODARRAY_ARRAY (old_gen) = NULL;
        MODARRAY_BODY (old_gen) = NULL;
        break;
    case N_foldprf:
        new_withop = MakeNWithOp (WO_foldprf);
        NWITHOP_PRF (new_withop) = FOLDPRF_PRF (old_op);
        NWITHOP_NEUTRAL (new_withop) = FOLDPRF_NEUTRAL (old_op);
        new_block = FOLDPRF_BODY (old_gen);

        FOLDPRF_NEUTRAL (old_gen) = NULL;
        FOLDPRF_BODY (old_gen) = NULL;
        break;
    case N_foldfun:
        new_withop = MakeNWithOp (WO_foldfun);
        NWITHOP_FUN (new_withop).id = FOLDFUN_NAME (old_op);
        NWITHOP_FUN (new_withop).id_mod = FOLDFUN_MOD (old_op);
        NWITHOP_NEUTRAL (new_withop) = FOLDFUN_NEUTRAL (old_op);
        new_block = FOLDFUN_BODY (old_gen);

        FOLDPRF_NEUTRAL (old_gen) = NULL;
        FOLDPRF_BODY (old_gen) = NULL;
        break;
    default:
        DBUG_ASSERT (0, "Unknown type of with-loop-operator");
        break;
    }
    new_expr = SeperateBlockReturn (new_block);

    FreeWith (arg_node, arg_node);

    DBUG_RETURN (MakeNWith (MakeNPart (new_withid, new_gen),
                            MakeNCode (new_block, new_expr), new_withop));
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
