/*
 * $Log$
 * Revision 1.2  2000/06/23 15:10:24  dkr
 * signature of DupTree changed
 *
 * Revision 1.1  2000/06/13 13:49:48  dkr
 * Initial revision
 *
 */

/*
 * This modul converts the layout of every WL as defined via stdin.
 *
 * The layout data must be given in the following format (BNF):
 *     layout  ->  [ row ]+
 *     row     ->  bound1 bound2 step width expr
 * Where bound1, bound2, step, width are space separated vectors of the same
 * dimension as *all* WLs in the used SAC code!!! (E.g. dim=3: 0 10 5)
 *
 *************
 *
 * Example:
 *
 *   Let a file named 'wlconv.sac' contain the following SAC program:
 *
 *     int main()
 *     {
 *       B = with ([0,0] <= idx=[i,j] < [5,5] step [1,1] width [1,1])
 *           genarray( [5,5], 1);
 *       return( B[0,0]);
 *     }
 *
 *   Let a file named 'nwith2.in' contain the following data:
 *
 *     00 0  100 100  15 1  12 1  0
 *     12 0  100 100  15 1  03 1  1
 *
 *   (Note, that the dimension of the array B in 'wlconv.sac' and the dimension
 *   of the new layout in 'nwith2.in' are equal (2)!)
 *
 *   Then PathWith() modifies the abtract syntax tree by analogy with the
 *   following SAC program:
 *
 *     with
 *       ([  0, 0 ] <= iv < [ 100, 100 ] step [ 15, 1 ] width [ 12, 1 ]) {} : 0,
 *       ([ 12, 0 ] <= iv < [ 100, 100 ] step [ 15, 1 ] width [  3, 1 ]) {} : 1
 *     genarray( [ 100, 100 ])
 *
 *************
 *
 * This modul can be activated with the sac2c flag
 *   -wlpatch
 * and is then called just before the transformation into the intermediate WL
 * representation is performed.
 */

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

/******************************************************************************
 *
 * Function:
 *   node *ReadOneGenPart(FILE *infile, types *type)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
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

/******************************************************************************
 *
 * Function:
 *   node *ReadCode(FILE *infile)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
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

static node *
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

        withid = DupTree (NPART_WITHID (arg_node));
        gen = DupTree (NPART_GEN (arg_node));
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
 *   node *PWpart(node *arg_node, node *arg_info)
 *
 * description:
 *   generates a more complex N_Npart syntaxtree (reads data from stdin)
 *
 ******************************************************************************/

node *
PWpart (node *arg_node, node *arg_info)
{
    FILE *infile;

    DBUG_ENTER ("PWpart");

    NPART_CODE (arg_node) = NULL;

    infile = stdin; /* use standard input */
    if (infile != NULL) {
        /*
         * generate a more complex N_Npart syntaxtree
         */
        arg_node = BuildNpart (infile, arg_node);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PWwith(node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PWwith (node *arg_node, node *arg_info)
{
    node *tmp_node, *shape, *bound2_el, *shape_el;
    int i;

    DBUG_ENTER ("PWwith");

    if (NPART_NEXT (NWITH_PART (arg_node)) != NULL) {
        NPART_NEXT (NWITH_PART (arg_node))
          = FreeTree (NPART_NEXT (NWITH_PART (arg_node)));
    }

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
        shape = DupTree (NGEN_BOUND2 (NPART_GEN (NWITH_PART (arg_node))));
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

/******************************************************************************
 *
 * Function:
 *   node *PatchWith( node *syntax_tree)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PatchWith (node *syntax_tree)
{
    DBUG_ENTER ("PatchWith");

    act_tab = patchwith_tab;
    syntax_tree = Trav (syntax_tree, NULL);

    DBUG_RETURN (syntax_tree);
}
