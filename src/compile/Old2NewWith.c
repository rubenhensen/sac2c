
#include "tree.h"
#include "traverse.h"
#include "typecheck.h"
#include "free.h"
#include "dbug.h"
#include "DupTree.h"

#define CODETAB_MAX 10

static int error = 1;
static node *codetab[CODETAB_MAX];
static int codetab_lastidx;

/*
 *
 *  functionname  : SeperateBlockReturn
 *  arguments     : 1) an expr-block
 *  description   : seperates the return-expr of a block from the rest.
 *                  (returns the return-expr [return value] and the rest of the block
 * [block])
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
    new_part = MakeNPart (new_withid, new_generator, new_code);

    FreeWith (arg_node, arg_node);

    arg_node = MakeNWith (new_part, new_code, new_withop);

    DBUG_RETURN (arg_node);
}

node *
ReadOneGenPart (FILE *infile, types *type)
{
    node *arr_node, *aelem1, *aelems = NULL;
    int val, i;

    DBUG_ENTER ("ReadOneGenPart");

    for (i = 0; i < SHPSEG_SHAPE (TYPES_SHPSEG (type), 0); i++) {
        if ((error = (feof (infile) || (fscanf (infile, "%d ", &val) == EOF)
                      || ferror (infile))))
            break;
        if (aelems == NULL) {
            aelem1 = aelems = MakeExprs (MakeNum (val), NULL);
        } else {
            EXPRS_NEXT (aelem1) = MakeExprs (MakeNum (val), NULL);
            aelem1 = EXPRS_NEXT (aelem1);
        }
    }
    if (error) {
        if (aelems != NULL)
            aelems = FreeTree (aelems);
        arr_node = NULL;
    } else {
        arr_node = MakeArray (aelems);
        ARRAY_TYPE (arr_node) = DuplicateTypes (type, 1);
    }

    DBUG_RETURN (arr_node);
}

node *
ReadCode (FILE *infile)
{
    node *code = NULL;
    int val;

    DBUG_ENTER ("ReadCode");

    error = (feof (infile) || (fscanf (infile, "%d ", &val) == EOF) || ferror (infile));

    if (!error) {
        DBUG_ASSERT ((val >= 0), "wrong index for codetab");
        DBUG_ASSERT ((val < CODETAB_MAX), "codetab too small");
        code = codetab[val];
        if (val > codetab_lastidx) {
            codetab_lastidx = val;
        }
    }

    DBUG_RETURN (code);
}

/******************************************************************************
 *
 * function:
 *   node *BuildNpart(FILE *infile, node *arg_node)
 *
 * description:
 *   reads the file 'infile' and generates with this data a more complex
 *   N_Npart-syntaxtree based on 'arg_node'.
 *
 *   syntax of the inputfile:
 *       a1 b1 s1 w1
 *       a2 b2 s2 w2
 *       ...
 *     (the values must have the type [int, int, ...] or int)
 *
 ******************************************************************************/

node *
BuildNpart (FILE *infile, node *arg_node)
{
    node *new_parts, *new_part, *last_part, *withid, *gen, *code, *tmp;
    types *type;

    DBUG_ENTER ("BuildNpart");

    error = 0;
    new_parts = new_part = NULL;

    do {
        last_part = new_part;

        tmp = NGEN_BOUND1 (NPART_GEN (arg_node));
        DBUG_ASSERT (((NODE_TYPE (tmp) == N_id) || (NODE_TYPE (tmp) == N_array)),
                     "wrong node type found");
        type
          = (NODE_TYPE (tmp) == N_id) ? VARDEC_TYPE (ID_VARDEC (tmp)) : ARRAY_TYPE (tmp);

        withid = DupTree (NPART_WITHID (arg_node), NULL);
        gen = DupTree (NPART_GEN (arg_node), NULL);
        code = NULL;

        /* replace now the generator-nodes */
        if (!error) {
            NGEN_BOUND1 (gen) = ReadOneGenPart (infile, type);
        }
        if (!error) {
            NGEN_BOUND2 (gen) = ReadOneGenPart (infile, type);
        }
        if (!error) {
            NGEN_STEP (gen) = ReadOneGenPart (infile, type);
        }
        if (!error) {
            NGEN_WIDTH (gen) = ReadOneGenPart (infile, type);
        }
        if (!error) {
            code = ReadCode (infile);
        }

        new_part = MakeNPart (withid, gen, code);
        if (new_parts == NULL) {
            new_parts = new_part;
        } else {
            NPART_NEXT (last_part) = new_part;
        }

    } while (!error);

    if (error) {
        /* remove uncompleted generator */
        NPART_NEXT (last_part) = FreeTree (NPART_NEXT (last_part));
    }

    /* if generation was succesful, only use new Npart-syntaxtree */
    if (new_parts != NULL) {
        arg_node = FreeTree (arg_node);
        arg_node = new_parts;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *O2Nnpart(node *arg_node, node *arg_info)
 *
 * description:
 *   generates a more complex N_Npart syntaxtree (reads data from stdin)
 *
 ******************************************************************************/

node *
O2Nnpart (node *arg_node, node *arg_info)
{
    FILE *infile;

    DBUG_ENTER ("O2Nnpart");

    NPART_CODE (arg_node) = NULL;

    infile = stdin; /* use standard input */
    if (infile != NULL) {
        arg_node = BuildNpart (infile,
                               arg_node); /* generate a more complex N_Npart syntaxtree */
    }

    DBUG_RETURN (arg_node);
}

node *
O2Nnwith (node *arg_node, node *arg_info)
{
    node *tmp_node, *shape, *bound2_el, *shape_el;
    int i;

    DBUG_ENTER ("O2Nnwith");

    NWITH_CODE (arg_node) = FreeTree (NWITH_CODE (arg_node));

    /* generate a new code table */
    for (i = 0; i < CODETAB_MAX; i++) {
        codetab[i] = MakeNCode (NULL, MakeNum (i));
        if (i == 0) {
            NWITH_CODE (arg_node) = codetab[i];
        } else {
            NCODE_NEXT (codetab[i - 1]) = codetab[i];
        }
    }

    NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);

    NCODE_NEXT (codetab[codetab_lastidx])
      = FreeTree (NCODE_NEXT (codetab[codetab_lastidx]));

    /* modify the genarray-shape */
    if (NWITHOP_TYPE (NWITH_WITHOP (arg_node)) == WO_genarray) {
        /* shape is the maximum of all bound2-values */
        shape = DupTree (NGEN_BOUND2 (NPART_GEN (NWITH_PART (arg_node))), NULL);
        ARRAY_TYPE (shape)
          = DuplicateTypes (ARRAY_TYPE (NGEN_BOUND2 (NPART_GEN (NWITH_PART (arg_node)))),
                            1);
        tmp_node = NPART_NEXT (NWITH_PART (arg_node));
        while (tmp_node != NULL) {
            bound2_el = ARRAY_AELEMS (NGEN_BOUND2 (NPART_GEN (tmp_node)));
            shape_el = ARRAY_AELEMS (shape);
            while (bound2_el != NULL) {
                if (NUM_VAL (EXPRS_EXPR (bound2_el)) > NUM_VAL (EXPRS_EXPR (shape_el))) {
                    NUM_VAL (EXPRS_EXPR (shape_el)) = NUM_VAL (EXPRS_EXPR (bound2_el));
                }
                bound2_el = EXPRS_NEXT (bound2_el);
                shape_el = EXPRS_NEXT (shape_el);
            }
            tmp_node = NPART_NEXT (tmp_node);
        }

        tmp_node = NWITHOP_SHAPE (NWITH_WITHOP (arg_node));
        NWITHOP_SHAPE (NWITH_WITHOP (arg_node)) = shape;
        tmp_node = FreeTree (tmp_node);
    }

    DBUG_RETURN (arg_node);
}

node *
Old2NewWith (node *arg_node)
{
    DBUG_ENTER ("Old2NewWith");

    act_tab = o2nWith_tab; /* set new function-table for traverse */

    arg_node = Trav (arg_node, NULL); /* convert old with-loop into new syntax */

    DBUG_RETURN (arg_node);
}
