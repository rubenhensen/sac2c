/*
 *
 * $Log$
 * Revision 1.8  2002/06/12 21:03:08  ktr
 * Comments added, everything in english
 *
 * Revision 1.7  2002/06/10 20:44:09  ktr
 * Bugfix: Only scalarize fully partitioned Withloops.
 *
 * Revision 1.6  2002/06/09 21:04:46  ktr
 * Update due to problems with RCS/CVS
 *
 * Revision 1.5  2002/06/09 20:40:09  ktr
 * works so good, it should be called alpha. :)
 *
 * Revision 1.4  2002/06/09 19:59:49  ktr
 * works even better, still some known bugs
 *
 * Revision 1.3  2002/05/16 09:40:07  ktr
 * an early version of a working WithLoop-Scalarization
 *
 * Revision 1.2  2002/04/09 08:13:02  ktr
 * Some functionality added, but still bugs
 *
 * Revision 1.1  2002/03/13 15:57:52  ktr
 * Initial revision
 *
 *
 */

/****************************************************************************
 *
 * file:    WithloopScalarization.c
 *
 * prefix:  WLS
 *
 * description:
 *
 *   This implements the WithloopScalarization in ssa-form.
 *
 *   WithloopScalarization is a high-level optimization which composes
 *   a single withloop from two nested ones in order to minimize memory-
 *   transactions and thereby improving time of program execution.
 *
 *
 *   WithloopScalarization works in four phases:
 *
 *   - Phase 1: Probing
 *
 *     In the probing-phase we check whether all conditions for a
 *     successful WithloopScalarization are met in the current withloop.
 *     NOTE: these conditions must be met for ALL Parts of the WL:
 *
 *     - a part must be computed by a withloop itself to give WLS sence
 *     - the computation must take place INSIDE of the part
 *     - the inner withloop must be a fully partitioned MG-WL
 *     - nesting of withloops must be perfect,
 *       i.e. the inner WL has to be the first instruction in the
 *       part's codeblock
 *     - all inner generators have to be independent from the outer part's
 *       generator
 *     - all inner WLs have to of the same type as the outer WL,
 *       because scalarization can only by done if we only have
 *       genarray-WLs OR only fold-WLs with the same fold-opration and
 *       neutral element.
 *
 *
 *   - Phase 2: Distribution
 *
 *     In this step, we distribute the parts of the outer WL over
 *     all the parts of the inner WLs in order to get an outer WL which's
 *     parts contain a WL holding exactly ONE part each.
 *
 *
 *   - Phase 3: Scalarization
 *
 *     After the structure has been simplified in phase 2, we can now
 *     start the main process of scalarization:
 *     For each part of the outer WL, the outer and the inner parts are
 *     joined into one part of the outer WL:
 *     - The new generator consists of concatenations of the former
 *       bound1,bound2,step and width vectors
 *     - The new code is the code of the inner WL, prepended with
 *       assignments that bind the old withvecs' names to arrays
 *       consisting of the same variables as before.
 *     - The new withids have a new withvec and concatenate the old ids
 *     Finally the new WLs shape is the concatenation of both old shapes.
 *
 *
 *   - Phase 4: Codecorrection
 *
 *     Finally we have to make sure that the NCODE_NEXT and NPART_NEXT
 *     pointers still point correctly.
 *
 *
 ****************************************************************************
 *
 *  Usage of arg_info:
 *
 *  - flag:    POSSIBLE  : WL nesting is transformable.
 *                         (Detected in phase PROBE)
 *  - linno:   PHASE     : Phase of the WithloopScalarization
 *                         (PROBE | DISTRIBUTE | SCALARIZE | CODE_CORRECT)
 *  - node[0]: FUNDEF    : pointer to current fundef node,
 *                         needed to access vardecs
 *  - counter: PARTS     : Number of parts of the new MG-Withloop
 *                         (calculated in phase DISTRIBUTE)
 *  - ids:     WITHVEC   : reference to the new MG-Withloop's withvec
 *  - node[1]: WITHOP    : reference to the outer WL's withop,
 *                         needed to check if both WL's types match
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "optimize.h"
#include "DupTree.h"

#include "WithloopScalarization.h"

/****************************************************************************
 *
 * typedef
 *
 ****************************************************************************/

/* Several traverse functions of this file are traversed for different
   purposes. This enum determines ths function, see description at the
   beginning of this file for details */

typedef enum { wls_probe, wls_distribute, wls_scalarize, wls_codecorrect } wls_phase_type;

#define WLS_PHASE(n) ((wls_phase_type)INFO_WLS_PHASE (n))

/****************************************************************************
 *
 * Helper functions
 *
 ****************************************************************************/

/******************************************************************************
 *
 * function:
 *   node *CreateOneVector(int nr)
 *
 * description:
 *   Creates an one-dimensional Array (aka Vector) of length nr whose
 *   elements are all Nums with value 1.
 *
 ******************************************************************************/
node *
CreateOneVector (int nr)
{
    node *res;
    node *temp;

    DBUG_ENTER ("MakeOnes");

    res = CreateZeroVector (nr, T_int);

    temp = ARRAY_AELEMS (res);

    while (temp != NULL) {
        NUM_VAL (EXPRS_EXPR (temp)) = 1;
        temp = EXPRS_NEXT (temp);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *MakeExprsIdChain(ids *idschain)
 *
 * description:
 *   Converts a chain of ids into an exprs-node
 *
 ******************************************************************************/
node *
MakeExprsIdChain (ids *idschain)
{
    node *res;

    DBUG_ENTER ("MakeExprsIdChain");

    if (idschain != NULL) {
        node *id;

        id = MakeId (IDS_NAME (idschain), IDS_MOD (idschain), ST_regular);

        ID_VARDEC (id) = IDS_VARDEC (idschain);
        ID_AVIS (id) = IDS_AVIS (idschain);

        res = MakeExprs (id, MakeExprsIdChain (IDS_NEXT (idschain)));
    } else
        res = NULL;

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *ConcatVecs(node *vec1, node *vec2)
 *
 * description:
 *   returns a vector which is the concatenation of vec1++vec2
 *
 ******************************************************************************/
node *
ConcatVecs (node *vec1, node *vec2)
{
    node *res;
    node *quelle, *ziel;

    DBUG_ENTER ("CONCAT_VECS");

    res = CreateZeroVector (ARRAY_SHAPE (vec1, 0) + ARRAY_SHAPE (vec2, 0), T_int);

    ziel = ARRAY_AELEMS (res);
    quelle = ARRAY_AELEMS (vec1);

    while (quelle != NULL) {
        NUM_VAL (EXPRS_EXPR (ziel)) = NUM_VAL (EXPRS_EXPR (quelle));
        quelle = EXPRS_NEXT (quelle);
        ziel = EXPRS_NEXT (ziel);
    }

    quelle = ARRAY_AELEMS (vec2);

    while (quelle != NULL) {
        NUM_VAL (EXPRS_EXPR (ziel)) = NUM_VAL (EXPRS_EXPR (quelle));
        quelle = EXPRS_NEXT (quelle);
        ziel = EXPRS_NEXT (ziel);
    }

    DBUG_RETURN (res);
}

/****************************************************************************
 *
 * Probing functions
 *
 ****************************************************************************/

/******************************************************************************
 *
 * function:
 *   int isAssignInsideBlock(node *assign, node *instr)
 *
 * description:
 *   checks if an assignment is part of a list of instructions
 *
 * parameters:
 *   node *assign:   the assignment to look for inside of
 *   node *instr:    the list of instructions
 *
 ******************************************************************************/
int
isAssignInsideBlock (node *assign, node *instr)
{
    int res = TRUE;

    DBUG_ENTER ("isAssignInsideBlock");

    if (NODE_TYPE (instr) == N_empty)
        res = FALSE;
    else if (assign == instr)
        res = TRUE;
    else if (ASSIGN_NEXT (instr) == NULL)
        res = FALSE;
    else
        res = isAssignInsideBlock (assign, ASSIGN_NEXT (instr));

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   int checkExprsDependencies(node *outerpart, node *exprs)
 *
 * description:
 *   checks if one of the Expressions in exprs is computed inside
 *   the withloop-part outerpart.
 *   In this case a WLS would be impossible and FALSE is returned.
 *
 *   This function is NOT COMPLETED YET
 *
 * parameters:
 *   node *outerpart:   N_NPART
 *   node *exprs:       N_exprs
 *
 ******************************************************************************/
int
checkExprsDependencies (node *outerpart, node *exprs)
{
    int res = TRUE;

    DBUG_ENTER ("checkExprsDependencies");

    if (exprs == NULL)
        res = TRUE;
    else if (NODE_TYPE (exprs) != N_array)
        res = FALSE; /* INTERESTING!!! */
    else
        res = checkExprsDependencies (outerpart, EXPRS_NEXT (exprs));

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   int checkGeneratorDependencies(node *outerpart, node *innerpart)
 *
 * description:
 *   checks if one of the generators of the inner withloop depends on
 *   calculations in the withloop-part outerpart.
 *   In this case a WLS would be impossible and FALSE is returned.
 *
 * parameters:
 *   node *outerpart:   N_NPART
 *   node *innerpart:   N_NPART
 *
 ******************************************************************************/
int
checkGeneratorDependencies (node *outerpart, node *innerpart)
{
    int res;
    node *innergen;

    DBUG_ENTER ("checkGeneratorDependencies");

    if (innerpart == NULL)
        res = TRUE;
    else {
        innergen = NPART_GEN (innerpart);

        res = TRUE;

        if ((res) && (!(checkExprsDependencies (outerpart, NGEN_BOUND1 (innergen)))))
            res = FALSE;

        if ((res) && (!(checkExprsDependencies (outerpart, NGEN_BOUND2 (innergen)))))
            res = FALSE;
        if ((res) && (NGEN_STEP (innergen) != NULL))
            if (!(checkExprsDependencies (outerpart, NGEN_STEP (innergen))))
                res = FALSE;
        if ((res) && (NGEN_WIDTH (innergen) != NULL))
            if (!(checkExprsDependencies (outerpart, NGEN_WIDTH (innergen))))
                res = FALSE;

        if (res)
            res = checkGeneratorDependencies (outerpart, NPART_NEXT (innerpart));
    }
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   int compatWLTypes(node *outerWithOP, node *innerWithOP)
 *
 * description:
 *   checks if the two nested withloops have compatible types
 *
 *   Compatibility means both Withloops are genarray-WLs OR
 *   they are both fold-WLs and have the same fold-operation/neutral-element
 *
 *   This function is NOT COMPLETED yet.
 *
 * parameters:
 *   node *outerWithOP:   N_NWITHOP
 *   node *innerWithOP:   N_NWITHOP
 *
 ******************************************************************************/
int
compatWLTypes (node *outerWithOP, node *innerWithOP)
{
    return ((NWITHOP_TYPE (outerWithOP) == WO_genarray)
            && (NWITHOP_TYPE (innerWithOP) == WO_genarray));
}

/****************************************************************************
 *
 * Distribution functions
 *
 ****************************************************************************/

/******************************************************************************
 *
 * function:
 *   node *distributePart(node *arg_node, node *arg_info)
 *
 * description:
 *   distributes an outer part over all n parts of an inner withloop,
 *   creating n outer parts with one inner part each.
 *
 * parameters:
 *   node *arg_node:   N_NPART
 *   node *arg_info:   N_INFO
 *
 ******************************************************************************/
node *
distributePart (node *arg_node, node *arg_info)
{
    node *res;
    node *innerwith;
    node *tmpnode;

    DBUG_ENTER ("distributePart");

    innerwith
      = LET_EXPR (ASSIGN_INSTR (BLOCK_INSTR (NCODE_CBLOCK (NPART_CODE (arg_node)))));

    if (NWITH_PARTS (innerwith) == 1)
        res = arg_node;
    else {
        tmpnode = DupTree (arg_node);
        NPART_CODE (tmpnode) = DupTree (NPART_CODE (arg_node));
        NCODE_USED (NPART_CODE (tmpnode))++;

        NCODE_NEXT (NPART_CODE (arg_node)) = NPART_CODE (tmpnode);
        NPART_NEXT (arg_node) = tmpnode;

        NWITH_PARTS (innerwith) = -1;
        NPART_NEXT (NWITH_PART (innerwith)) = NULL;
        NCODE_NEXT (NWITH_CODE (innerwith)) = NULL;

        innerwith
          = LET_EXPR (ASSIGN_INSTR (BLOCK_INSTR (NCODE_CBLOCK (NPART_CODE (tmpnode)))));
        NWITH_PARTS (innerwith) -= 1;
        NWITH_PART (innerwith) = NPART_NEXT (NWITH_PART (innerwith));
        NWITH_CODE (innerwith) = NCODE_NEXT (NWITH_CODE (innerwith));

        INFO_WLS_PARTS (arg_info) += 1;
        res = arg_node;
    }
    DBUG_RETURN (res);
}

/****************************************************************************
 *
 * Scalarization functions
 *
 ****************************************************************************/

/******************************************************************************
 *
 * function:
 *   node *joinGenerators(node *outergen, node *innergen)
 *
 * description:
 *   Creates a new generator that iterates over the same space as the
 *   two generators specified in the paramters by concatenating
 *   all the vectors.
 *
 * parameters:
 *   node *outergen:   N_NGEN
 *   node *innergen:   N_NGEN
 *
 ******************************************************************************/
node *
joinGenerators (node *outergen, node *innergen)
{
    node *newgen;

    node *b1, *b2, *s, *w;
    int d1, d2;

    DBUG_ENTER ("joinGenerators");

    d1 = CountExprs (ARRAY_AELEMS (NGEN_BOUND1 (outergen)));
    d2 = CountExprs (ARRAY_AELEMS (NGEN_BOUND1 (innergen)));

    b1 = ConcatVecs (NGEN_BOUND1 (outergen), NGEN_BOUND1 (innergen));

    b2 = ConcatVecs (NGEN_BOUND2 (outergen), NGEN_BOUND2 (innergen));

    /* Join the Grids */

    if ((NGEN_STEP (outergen) == NULL) && (NGEN_STEP (innergen) == NULL))
        s = NULL;
    else {
        if (NGEN_STEP (outergen) == NULL)
            NGEN_STEP (outergen) = CreateOneVector (d1);

        if (NGEN_STEP (innergen) == NULL)
            NGEN_STEP (innergen) = CreateOneVector (d2);

        s = ConcatVecs (NGEN_STEP (outergen), NGEN_STEP (innergen));
    }

    if ((NGEN_WIDTH (outergen) == NULL) && (NGEN_WIDTH (innergen) == NULL))
        w = NULL;
    else {
        if (NGEN_WIDTH (outergen) == NULL)
            NGEN_WIDTH (outergen) = CreateOneVector (d1);

        if (NGEN_WIDTH (innergen) == NULL)
            NGEN_WIDTH (innergen) = CreateOneVector (d2);

        w = ConcatVecs (NGEN_WIDTH (outergen), NGEN_WIDTH (innergen));
    }

    newgen = MakeNGenerator (b1, b2, NGEN_OP1 (outergen), NGEN_OP2 (outergen), s, w);

    DBUG_RETURN (newgen);
}

/******************************************************************************
 *
 * function:
 *   node *joinWithids(node *outerwithid, node *innerwithid, node *arg_info)
 *
 * description:
 *   Creates a new withid by concatenating the WITH_IDS-Vectors and
 *   adding a new WITH_VEC.
 *
 * parameters:
 *   node *outerwithid:   N_NWITHID
 *   node *innerwithid:   N_NWITHID
 *
 ******************************************************************************/
node *
joinWithids (node *outerwithid, node *innerwithid, node *arg_info)
{
    node *newwithid;

    ids *vec;
    node *vardec;
    types *newtype;
    shpseg *newshpseg;

    ids *scalars;
    char *new_name;

    DBUG_ENTER ("joinWithids");

    /* Generate a new WITHVEC if we don't have one already */
    if (INFO_WLS_WITHVEC (arg_info) == NULL) {
        new_name = TmpVar ();

        newshpseg = MakeShpseg (NULL);

        SHPSEG_SHAPE (newshpseg, 0) = IDS_SHAPE (NWITHID_VEC (outerwithid), 0)
                                      + IDS_SHAPE (NWITHID_VEC (innerwithid), 0);

        newtype = MakeTypes (T_int, 1, newshpseg, NULL, NULL);

        vardec = MakeVardec (new_name, newtype, NULL);

        vec = MakeIds (StringCopy (new_name), NULL, ST_used);

        FUNDEF_VARDEC (INFO_WLS_FUNDEF (arg_info))
          = AppendVardec (FUNDEF_VARDEC (INFO_WLS_FUNDEF (arg_info)), vardec);

        IDS_VARDEC (vec) = vardec;
        IDS_AVIS (vec) = VARDEC_AVIS (vardec);

        INFO_WLS_WITHVEC (arg_info) = vec;
    }

    scalars = AppendIds (DupAllIds (NWITHID_IDS (outerwithid)),
                         DupAllIds (NWITHID_IDS (innerwithid)));

    newwithid = MakeNWithid (DupAllIds (INFO_WLS_WITHVEC (arg_info)), scalars);

    DBUG_RETURN (newwithid);
}

/******************************************************************************
 *
 * function:
 *   node *joinCodes(node *outercode,   node *innercode,
 *                   node *outerwithid, node *innerwithid,
 *                   node *arg_info)
 *
 * description:
 *   Creates a new block which is nothing but the old INNER codeblock
 *   prepended with definitions of the two old WITHVECs
 *
 * parameters:
 *   node *outercode:   N_NCODE
 *   node *innercode:   N_NCODE
 *   node *outerwithid: N_NWITHID
 *   node *innerwithid: N_NWITHID
 *   node *arg_info:    N_INFO
 *
 ******************************************************************************/
node *
joinCodes (node *outercode, node *innercode, node *outerwithid, node *innerwithid,
           node *arg_info)
{
    node *newcode;
    node *tmp_node;
    node *array;

    DBUG_ENTER ("joinCodes");

    /* The new code is the old INNER part's code ... */
    newcode = DupTree (innercode);

    /* prepended with definitions of the two old WITHVECs */
    array = MakeArray (MakeExprsIdChain (DupAllIds (NWITHID_IDS (outerwithid))));

    ARRAY_TYPE (array)
      = MakeTypes (T_int, 1,
                   MakeShpseg (MakeNums (CountExprs (ARRAY_AELEMS (array)), NULL)), NULL,
                   NULL);

    tmp_node = MakeAssignLet (IDS_NAME (NWITHID_VEC (outerwithid)),
                              IDS_VARDEC (NWITHID_VEC (outerwithid)), array);

    array = MakeArray (MakeExprsIdChain (DupAllIds (NWITHID_IDS (innerwithid))));

    ARRAY_TYPE (array)
      = MakeTypes (T_int, 1,
                   MakeShpseg (MakeNums (CountExprs (ARRAY_AELEMS (array)), NULL)), NULL,
                   NULL);

    ASSIGN_NEXT (tmp_node)
      = MakeAssignLet (IDS_NAME (NWITHID_VEC (innerwithid)),
                       IDS_VARDEC (NWITHID_VEC (innerwithid)), array);

    /* Bring everything in the right order */
    if (NODE_TYPE (BLOCK_INSTR (NCODE_CBLOCK (newcode))) != N_empty)
        ASSIGN_NEXT (ASSIGN_NEXT (tmp_node)) = BLOCK_INSTR (NCODE_CBLOCK (newcode));

    /* Set all the necessary pointers */
    BLOCK_INSTR (NCODE_CBLOCK (newcode)) = tmp_node;
    NCODE_NEXT (newcode) = NCODE_NEXT (outercode);

    /* Return the new Codeblock */
    DBUG_RETURN (newcode);
}

/******************************************************************************
 *
 * function:
 *   node *joinPart(node *outerpart, node *arg_info)
 *
 * description:
 *   The heart of the WithloopScalarization.
 *   This function takes a part of MG-withloop that contains exactly one Withloop
 *   with one part and applies the above join-functions to create a single part
 *   that iterates over both old parts' dimensions.
 *
 * parameters:
 *   node *outerpart:   N_NPART
 *   node *arg_info:    N_INFO
 *
 ******************************************************************************/
node *
joinPart (node *outerpart, node *arg_info)
{
    node *newpart;

    node *innerpart;

    node *generator;
    node *withid;
    node *code;

    DBUG_ENTER ("joinPart");

    innerpart = NWITH_PART (
      LET_EXPR (ASSIGN_INSTR (BLOCK_INSTR (NCODE_CBLOCK (NPART_CODE (outerpart))))));

    /* Make a new generator */
    generator = joinGenerators (NPART_GEN (outerpart), NPART_GEN (innerpart));

    /* Make a new withid */
    withid = joinWithids (NPART_WITHID (outerpart), NPART_WITHID (innerpart), arg_info);

    /* Make new code */
    code = joinCodes (NPART_CODE (outerpart), NPART_CODE (innerpart),
                      NPART_WITHID (outerpart), NPART_WITHID (innerpart), arg_info);

    /* Now we can build a new part */
    newpart = MakeNPart (withid, generator, code);

    /* Rebuild the chain */
    NPART_NEXT (newpart) = NPART_NEXT (outerpart);
    NCODE_NEXT (NPART_CODE (newpart)) = NCODE_NEXT (NPART_CODE (outerpart));

    /* Present the Results */
    wls_expr++;

    DBUG_RETURN (newpart);
}

/******************************************************************************
 *
 * traversal functions
 *
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *   node *WLSfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   This function's sole purpose is to annotate the fundes-node of the
 *   currently traversed function so that the vardecs can be referenced later
 *
 * parameters:
 *   node *arg_node:   N_FUNDEF
 *   node *arg_info:   N_INFO
 *
 ******************************************************************************/
node *
WLSfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLSfundef");

    INFO_WLS_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        /* traverse block of fundef */
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLSNwith(node *arg_node, node *arg_info)
 *
 * description:
 *   manages the WithloopScalarization by stepping through the various phases,
 *   but before this can be done, all the codes are traversed in order to
 *   scalerize nested withloops first.
 *
 * parameters:
 *   node *arg_node:   N_Nwith
 *   node *arg_info:   N_INFO
 *
 ******************************************************************************/
node *
WLSNwith (node *arg_node, node *arg_info)
{
    node *tmpnode;

    DBUG_ENTER ("WLSNwith");

    DBUG_PRINT ("WLS", ("\nstarting with-loop scalarization in Nwith %s.",
                        FUNDEF_NAME (arg_node)));

    /* First WLS traverses into the branches in order to scalarize and
       fold all other With-Loops */

    /* traverse all CODES */
    if (NWITH_CODE (arg_node) != NULL) {
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    }

    /**************************************************

        it could be a good idea to do some WLF here

    ***************************************************/

    /***************************************************************************
     *
     *  PROBING
     *
     *  We have to find out, whether all parts can be scalarized
     *  Here we can distinguish between conservative and agressive behaviour
     *
     ***************************************************************************/

    tmpnode = arg_info;
    arg_info = MakeInfo ();
    INFO_WLS_FUNDEF (arg_info) = INFO_WLS_FUNDEF (tmpnode);

    /* Check if WLS is possible vor all parts */

    INFO_WLS_PARTS (arg_info) = NWITH_PARTS (arg_node);

    INFO_WLS_POSSIBLE (arg_info) = TRUE;

    if (INFO_WLS_POSSIBLE (arg_info)) {
        WLS_PHASE (arg_info) = wls_probe;
        INFO_WLS_WITHOP (arg_info) = NWITH_WITHOP (arg_node);

        if (NWITH_PART (arg_node) != NULL) {
            NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
        }
    }

    /* create a full partitioned outer Withloop */
    if (INFO_WLS_PARTS (arg_info) < 0) {
        /*

          CODE TO CREATE A FULL PARTITIONED WITHLOOP

        */

        INFO_WLS_PARTS (arg_info) = NWITH_PARTS (arg_node);
    }

    /* Scalarize only complete partitions */
    INFO_WLS_POSSIBLE (arg_info)
      = INFO_WLS_POSSIBLE (arg_info) && (INFO_WLS_PARTS (arg_info) > 0);

    /* If everything is ok, we can start phase 2 */
    if (INFO_WLS_POSSIBLE (arg_info)) {

        /***************************************************************************
         *
         *  DISTRIBUTION
         *
         *  First, we have to distribute the outer part over all
         *  parts of the inner with-loop
         *
         ***************************************************************************/

        WLS_PHASE (arg_info) = wls_distribute;

        /* traverse all parts */

        if (NWITH_PART (arg_node) != NULL) {
            NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
        }

        NWITH_CODE (arg_node) = NPART_CODE (NWITH_PART (arg_node));
        NWITH_PARTS (arg_node) = INFO_WLS_PARTS (arg_info);

        /***************************************************************************
         *
         *  SCALARIZATION
         *
         ***************************************************************************/

        WLS_PHASE (arg_info) = wls_scalarize;

        INFO_WLS_WITHVEC (arg_info) = NULL;

        /* The new shape is the concatenation of both old shapes */

        NWITH_SHAPE (arg_node) = ConcatVecs (NWITH_SHAPE (arg_node),
                                             NWITH_SHAPE (LET_EXPR (ASSIGN_INSTR (
                                               BLOCK_INSTR (NWITH_CBLOCK (arg_node))))));

        /* traverse all PARTS  */

        if (NWITH_PART (arg_node) != NULL) {
            NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
        }

        /***************************************************************************
         *
         *  CODECORRECTION
         *
         ***************************************************************************/

        WLS_PHASE (arg_info) = wls_codecorrect;

        NWITH_CODE (arg_node) = NPART_CODE (NWITH_PART (arg_node));
        if (NWITH_PART (arg_node) != NULL) {
            NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
        }
    }

    arg_info = FreeTree (arg_info);
    arg_info = tmpnode;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLSNpart(node *arg_node, node *arg_info)
 *
 * description:
 *   performs the different actions that are needed to make in the four phases
 *
 * parameters:
 *   node *arg_node:   N_Npart
 *   node *arg_info:   N_INFO
 *
 ******************************************************************************/
node *
WLSNpart (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLSNpart");

    switch (WLS_PHASE (arg_info)) {
    case wls_probe:
        /* Check wheter this part meets all conditions for WLS */

        INFO_WLS_POSSIBLE (arg_info)
          = (INFO_WLS_POSSIBLE (arg_info) &&
             /* Is the inner CEXPR a nonscalar? */
             (TYPES_DIM (VARDEC_TYPE (
                AVIS_VARDECORARG (ID_AVIS (NCODE_CEXPR (NPART_CODE (arg_node))))))
              > 0)
             &&

             /* Is the inner CEXPR computed by a withloop? */
             (NODE_TYPE (LET_EXPR (ASSIGN_INSTR (
                AVIS_SSAASSIGN (ID_AVIS (NCODE_CEXPR (NPART_CODE (arg_node)))))))
              == N_Nwith)
             &&

             /* Is the inner WL fully partitioned? */
             (NWITH_PARTS (LET_EXPR (ASSIGN_INSTR (
                AVIS_SSAASSIGN (ID_AVIS (NCODE_CEXPR (NPART_CODE (arg_node)))))))
              > 0)
             &&

             /* Is the inner Withloop really inside of this part? */
             (isAssignInsideBlock (AVIS_SSAASSIGN (
                                     ID_AVIS (NCODE_CEXPR (NPART_CODE (arg_node)))),
                                   BLOCK_INSTR (NCODE_CBLOCK (NPART_CODE (arg_node)))))
             &&

             /* Is this a perfect nesting of WLs? */
             (BLOCK_INSTR (NCODE_CBLOCK (NPART_CODE (arg_node)))
              == AVIS_SSAASSIGN (ID_AVIS (NCODE_CEXPR (NPART_CODE (arg_node)))))
             &&

             /* Is the inner Generator independent from the outer? */
             (checkGeneratorDependencies (arg_node,
                                          NWITH_PART (LET_EXPR (
                                            ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (
                                              NCODE_CEXPR (NPART_CODE (arg_node)))))))))
             &&

             /* Are both WLs of compatible Type? */
             (compatWLTypes (INFO_WLS_WITHOP (arg_info),
                             NWITH_WITHOP (LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (
                               ID_AVIS (NCODE_CEXPR (NPART_CODE (arg_node))))))))));

        /* Check the next part for all conditions */
        if ((INFO_WLS_POSSIBLE (arg_info)) && (NPART_NEXT (arg_node) != NULL)) {
            NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
        }
        break;

    case wls_distribute:
        arg_node = distributePart (arg_node, arg_info);

        if (NPART_NEXT (arg_node) != NULL) {
            NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
        }
        break;

    case wls_scalarize:
        arg_node = joinPart (arg_node, arg_info);

        if (NPART_NEXT (arg_node) != NULL) {
            NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
        }
        break;

    case wls_codecorrect:
        if (NPART_NEXT (arg_node) != NULL) {
            NCODE_NEXT (NPART_CODE (arg_node)) = NPART_CODE (NPART_NEXT (arg_node));

            NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
        }
        break;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WithloopScalarization(node *fundef, node *modul)
 *
 * description:
 *   starting point of WithloopScalarization
 *   Starting fundef must not be a special fundef (do, while, cond) created by
 *   lac2fun transformation. These "inline" functions will be traversed in
 *   their order of usage. The traversal mode (on toplevel, in special
 *   function) is annotated in the stacked INFO_SSADCR_DEPTH attribute.
 *
 *****************************************************************************/

node *
WithloopScalarization (node *fundef, node *modul)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("WithloopScalarization");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "WithloopScalarization called for non-fundef node");

    DBUG_PRINT ("OPT", ("starting WithloopScalarization (ssa) in function %s",
                        FUNDEF_NAME (fundef)));

    /* do not start traversal in special functions */
    if (!(FUNDEF_IS_LACFUN (fundef))) {
        arg_info = MakeInfo ();

        old_tab = act_tab;
        act_tab = wls_tab;

        fundef = Trav (fundef, arg_info);

        act_tab = old_tab;

        arg_info = FreeTree (arg_info);
    }

    DBUG_RETURN (fundef);
}
