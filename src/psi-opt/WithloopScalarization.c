/*
 *
<<<<<<< WithloopScalarization.c
 * $Log$
 * Revision 1.5  2002/06/09 20:40:09  ktr
 * works so good, it should be called alpha. :)
 *
 * Revision 1.4  2002/06/09 19:59:49  ktr
 * works even better, still some known bugs
 *
=======
>>>>>>> 1.2
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
 *   This file realizes the with-loop-scalarization in ssa-form.
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "internal_lib.h"
#include "optimize.h"
#include "free.h"
#include "DataFlowMask.h"
#include "DupTree.h"

#include "WithloopScalarization.h"

#define wls_probe 0
#define wls_distribute 1
#define wls_transform 2
#define wls_codecorrect 3

#define INFO_WLS_WITHVEC(n) (n->info.ids)
#define INFO_WLS_WITHOP(n) (n->node[1])

node *
WLSfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLSfundef");

    INFO_WLS_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        /* traverse block of fundef */
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_ARGS (arg_node) != NULL) {
        /* traverse args of fundef */
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

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

node *
WLSNwith (node *arg_node, node *arg_info)
{
    /* local variables */
    node *tmpnode;

    DBUG_ENTER ("WLSNwith");

    DBUG_PRINT ("WLS", ("\nstarting with-loop scalarization in Nwith %s.",
                        FUNDEF_NAME (arg_node)));

    /* First WLS traverses into the branches in order to scalarize and
       fold all other With-Loops */

    /* traverse WITHOP */
    if (NWITH_WITHOP (arg_node) != NULL) {
        NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);
    }

    /* traverse all CODES */
    if (NWITH_CODE (arg_node) != NULL) {
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    }

    /* Insert WLF here */

    /* Let's go! */

    /* We have to find out, whether all parts can be scalarized */
    /* Here we can distinguish between conservative and agressive behaviour */

    tmpnode = arg_info;
    arg_info = MakeInfo ();
    INFO_WLS_FUNDEF (arg_info) = INFO_WLS_FUNDEF (tmpnode);

    /* Check if WLS is possible vor all parts */

    INFO_WLS_POSSIBLE (arg_info) = TRUE;
    INFO_WLS_PHASE (arg_info) = wls_probe;
    INFO_WLS_WITHOP (arg_info) = NWITH_WITHOP (arg_node);

    if (NWITH_PART (arg_node) != NULL) {
        NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
    }

    /* If all parts are ready for scalarization we can start */

    if (INFO_WLS_POSSIBLE (arg_info)) {
        /* First, we have to distribute the outer part over all
           parts of the inner with-loop */

        INFO_WLS_PHASE (arg_info) = wls_distribute;

        INFO_WLS_PARTS (arg_info) = NWITH_PARTS (arg_node);
        /* traverse all parts */

        if (NWITH_PART (arg_node) != NULL) {
            NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
        }

        NWITH_CODE (arg_node) = NPART_CODE (NWITH_PART (arg_node));
        NWITH_PARTS (arg_node) = INFO_WLS_PARTS (arg_info);

        /* Now we can start the scalarization */

        INFO_WLS_PHASE (arg_info) = wls_transform;
        INFO_WLS_WITHVEC (arg_info) = NULL;

        /* The new shape is the concatenation of both old shapes */

        NWITH_SHAPE (arg_node) = ConcatVecs (NWITH_SHAPE (arg_node),
                                             NWITH_SHAPE (LET_EXPR (ASSIGN_INSTR (
                                               BLOCK_INSTR (NWITH_CBLOCK (arg_node))))));

        /* traverse all PARTS  */

        if (NWITH_PART (arg_node) != NULL) {
            NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
        }

        /* correct the Ncode pointers */
        INFO_WLS_PHASE (arg_info) = wls_codecorrect;

        NWITH_CODE (arg_node) = NPART_CODE (NWITH_PART (arg_node));

        if (NWITH_PART (arg_node) != NULL) {
            NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
        }
    }

    arg_info = FreeTree (arg_info);
    arg_info = tmpnode;

    DBUG_RETURN (arg_node);
}

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

/*
   joinGenerators creates the new Generator to iterate the
   same space as the former two Generators
*/
node *
joinGenerators (node *outergen, node *innergen, node *arg_info)
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

node *
joinCodes (node *outercode, node *innercode, node *outerwithid, node *innerwithid,
           node *arg_info)
{
    node *newcode;
    node *tmp_node;
    node *array;

    DBUG_ENTER ("joinCodes");

    newcode = DupTree (innercode);

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

    if (NODE_TYPE (BLOCK_INSTR (NCODE_CBLOCK (newcode))) != N_empty)
        ASSIGN_NEXT (ASSIGN_NEXT (tmp_node)) = BLOCK_INSTR (NCODE_CBLOCK (newcode));

    BLOCK_INSTR (NCODE_CBLOCK (newcode)) = tmp_node;
    NCODE_NEXT (newcode) = NCODE_NEXT (outercode);
    DBUG_RETURN (newcode);
}

/*
  joinPart joins a part with an inner single-generator withloop
 */
node *
joinPart (node *outerpart, node *arg_info)
{
    node *newpart;

    node *innerpart;

    node *generator;
    node *withid;
    node *code;

    DBUG_ENTER ("joinPart");

    /*
      innerpart = NWITH_PART(LET_EXPR(ASSIGN_INSTR(
                  AVIS_SSAASSIGN(ID_AVIS(NCODE_CEXPR(NPART_CODE(outerpart)))))));
    */
    innerpart = NWITH_PART (
      LET_EXPR (ASSIGN_INSTR (BLOCK_INSTR (NCODE_CBLOCK (NPART_CODE (outerpart))))));

    /* Make a new generator */
    generator = joinGenerators (NPART_GEN (outerpart), NPART_GEN (innerpart), arg_info);

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

        NWITH_PARTS (innerwith) = 1;
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

int
isAssignInsideBlock (node *arg_node, node *instr)
{
    int res = TRUE;

    DBUG_ENTER ("isAssignInsideBlock");

    if (NODE_TYPE (instr) == N_empty)
        res = FALSE;
    else if (arg_node == instr)
        res = TRUE;
    else if (ASSIGN_NEXT (instr) == NULL)
        res = FALSE;
    else
        res = isAssignInsideBlock (arg_node, ASSIGN_NEXT (instr));

    DBUG_RETURN (res);
}

int
checkExprsDependencies (node *outer, node *exprs)
{
    int res = TRUE;

    DBUG_ENTER ("checkExprsDependencies");

    if (exprs == NULL)
        res = TRUE;
    else if (NODE_TYPE (exprs) != N_array)
        res = FALSE; /* INTERESSANT!!! */
    else
        res = checkExprsDependencies (outer, EXPRS_NEXT (exprs));

    DBUG_RETURN (res);
}

int
checkGeneratorDependencies (node *outer, node *inner)
{
    int res;
    node *innergen;

    DBUG_ENTER ("checkGeneratorDependencies");

    if (inner == NULL)
        res = TRUE;
    else {
        innergen = NPART_GEN (inner);

        res = TRUE;

        if ((res) && (!(checkExprsDependencies (outer, NGEN_BOUND1 (innergen)))))
            res = FALSE;

        if ((res) && (!(checkExprsDependencies (outer, NGEN_BOUND2 (innergen)))))
            res = FALSE;
        if ((res) && (NGEN_STEP (innergen) != NULL))
            if (!(checkExprsDependencies (outer, NGEN_STEP (innergen))))
                res = FALSE;
        if ((res) && (NGEN_WIDTH (innergen) != NULL))
            if (!(checkExprsDependencies (outer, NGEN_WIDTH (innergen))))
                res = FALSE;

        if (res)
            res = checkGeneratorDependencies (outer, NPART_NEXT (inner));
    }
    DBUG_RETURN (res);
}

int
compatWLTypes (node *outerWO, node *innerWO)
{
    return ((NWITHOP_TYPE (outerWO) == WO_genarray)
            && (NWITHOP_TYPE (innerWO) == WO_genarray));
}

node *
WLSNpart (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLSNpart");

    switch (INFO_WLS_PHASE (arg_info)) {
    case wls_probe:
        if (NPART_WITHID (arg_node) != NULL) {
            /* traverse the withid of this part */
            NPART_WITHID (arg_node) = Trav (NPART_WITHID (arg_node), arg_info);
        }

        if (NPART_GEN (arg_node) != NULL) {
            /* traverse into the generator of this part */
            NPART_GEN (arg_node) = Trav (NPART_GEN (arg_node), arg_info);
        }

        /* Handelt es sich um eine korrekte Schachtelung? */

        INFO_WLS_POSSIBLE (arg_info)
          = (INFO_WLS_POSSIBLE (arg_info) &&
             /* Ist der Ausdruck überhaupt nichtskalar? */
             (TYPES_DIM (VARDEC_TYPE (
                AVIS_VARDECORARG (ID_AVIS (NCODE_CEXPR (NPART_CODE (arg_node))))))
              > 0)
             &&

             /* Ist der Ausdruck eine With-Loop? */
             (NODE_TYPE (LET_EXPR (ASSIGN_INSTR (
                AVIS_SSAASSIGN (ID_AVIS (NCODE_CEXPR (NPART_CODE (arg_node)))))))
              == N_Nwith)
             &&
             /* Ist der innere Withloop wirklich innen? */
             (isAssignInsideBlock (AVIS_SSAASSIGN (
                                     ID_AVIS (NCODE_CEXPR (NPART_CODE (arg_node)))),
                                   BLOCK_INSTR (NCODE_CBLOCK (NPART_CODE (arg_node)))))
             &&

             /* Ist vor der inneren With-Loop wirklich kein Code mehr? */
             /* Hier könnte evtl. noch aggressiv optimiert werden! */
             (BLOCK_INSTR (NCODE_CBLOCK (NPART_CODE (arg_node)))
              == AVIS_SSAASSIGN (ID_AVIS (NCODE_CEXPR (NPART_CODE (arg_node)))))
             &&

             /* Hängt der innere Generator nicht vom äußeren Code ab? */
             (checkGeneratorDependencies (arg_node,
                                          NWITH_PART (LET_EXPR (
                                            ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (
                                              NCODE_CEXPR (NPART_CODE (arg_node)))))))))
             &&

             /* Haben alle inneren With-Loops die gleiche Dimensionalität? */
             (TRUE) &&

             /* Haben beide Withloops einen kompatiblen Typ? */
             (compatWLTypes (INFO_WLS_WITHOP (arg_info),
                             NWITH_WITHOP (LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (
                               ID_AVIS (NCODE_CEXPR (NPART_CODE (arg_node))))))))));

        if (NPART_NEXT (arg_node) != NULL) {
            NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
        }
        break;

    case wls_transform:
        arg_node = joinPart (arg_node, arg_info);

        if (NPART_NEXT (arg_node) != NULL) {
            NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
        }
        break;

    case wls_distribute:
        arg_node = distributePart (arg_node, arg_info);

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

/*
 * TO BE REMOVED
 */

node *
WLSNwithid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLSNwithid");

    DBUG_RETURN (arg_node);
}

node *
WLSNgenerator (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLSNgenerator");

    if (NGEN_BOUND1 (arg_node) != NULL) {
        Trav (NGEN_BOUND1 (arg_node), arg_info);
    }
    if (NGEN_BOUND2 (arg_node) != NULL) {
        Trav (NGEN_BOUND2 (arg_node), arg_info);
    }
    if (NGEN_STEP (arg_node) != NULL) {
        Trav (NGEN_STEP (arg_node), arg_info);
    }
    if (NGEN_WIDTH (arg_node) != NULL) {
        Trav (NGEN_WIDTH (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
WLSNwithop (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLSNwithop");

    DBUG_RETURN (arg_node);
}

node *
WLSNcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLSNcode");

    /* traverse expression */
    if (NCODE_CEXPR (arg_node) != NULL) {
        NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);
    }

    /* traverse code block */
    if (NCODE_CBLOCK (arg_node) != NULL) {
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    }

    /* traverse expression */
    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
