/*
 *
 * $Log$
 * Revision 1.17  2002/10/18 17:14:11  ktr
 * Fixed a bug which made probePart crash when a part's CEXPR is the index vector.
 *
 * Revision 1.16  2002/10/18 14:18:57  ktr
 * Rewrote probePart
 *
 * Revision 1.15  2002/10/17 19:13:42  ktr
 * improved safety of aggressive mode
 *
 * Revision 1.14  2002/10/17 17:54:31  ktr
 * aggressive behaviour now based upon switch -wlsx
 *
 * Revision 1.13  2002/10/17 14:51:22  ktr
 * Fixed a bug found by sbs. Sample code now runs with 200% of speed.
 *
 * Revision 1.12  2002/07/11 12:49:27  ktr
 * WLS now supports conversion of other non-scalar expressions into a
 * Withloop-nesting, which is scalarizeble.
 *
 * Revision 1.11  2002/06/26 20:31:22  ktr
 * WLS now supports all tested MG-genarray WLs.
 *
 * Revision 1.10  2002/06/18 10:23:15  ktr
 * Support for N_id nodes in generator's N_array nodes added.
 *
 * Revision 1.9  2002/06/14 23:02:15  ktr
 * Code improved by definition of compund access macros.
 *
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
 *     - a part must be computed by a withloop itself to give WLS sense
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

#include "globals.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "optimize.h"
#include "DupTree.h"
#include "SSATransform.h"
#include "LookUpTable.h"
#include "SSAWLT.h"

#include "WithloopScalarization.h"

/****************************************************************************
 *
 * typedef
 *
 ****************************************************************************/

/* Several traverse functions of this file are traversed for different
   purposes. This enum determines ths function, see description at the
   beginning of this file for details */

typedef enum {
    wls_probe,
    wls_withloopification,
    wls_normgen,
    wls_distribute,
    wls_scalarize,
    wls_codecorrect
} wls_phase_type;

#define WLS_PHASE(n) ((wls_phase_type)INFO_WLS_PHASE (n))

/****************************************************************************
 *
 * Helper macros
 *
 ****************************************************************************/

/* NPART_SSAASSIGN returns the assignment of the parts' CEXPR */
#define NPART_SSAASSIGN(n) (ID_SSAASSIGN (NPART_CEXPR (n)))

/* NPART_LETEXPR returns the defining node of the part's CEXPR
   Here it is mainly used to find the inner WL. */
#define NPART_LETEXPR(n) (LET_EXPR (ASSIGN_INSTR (NPART_SSAASSIGN (n))))

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

    DBUG_ENTER ("CONCAT_VECS");

    res = CreateZeroVector (ARRAY_SHAPE (vec1, 0) + ARRAY_SHAPE (vec2, 0), T_int);

    ARRAY_AELEMS (res)
      = CombineExprs (DupTree (ARRAY_AELEMS (vec1)), DupTree (ARRAY_AELEMS (vec2)));

    ((int *)ARRAY_CONSTVEC (res)) = Array2IntVec (ARRAY_AELEMS (res), NULL);

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

int
checkIdDefinition (node *outerpart, ids *id)
{
    int res = TRUE;
    ids *idtemp;
    node *assigntemp;

    DBUG_ENTER ("checkIdDefinition");

    /* check withvec */
    res
      = res && strcmp (IDS_NAME (id), IDS_NAME (NWITHID_VEC (NPART_WITHID (outerpart))));

    /* check withids */
    idtemp = NWITHID_IDS (NPART_WITHID (outerpart));

    while ((res) && (idtemp != NULL)) {
        res = res && strcmp (IDS_NAME (id), IDS_NAME (idtemp));

        idtemp = IDS_NEXT (idtemp);
    }

    /* check codeblock */
    assigntemp = BLOCK_INSTR (NPART_CBLOCK (outerpart));

    while ((res) && (assigntemp != NULL) && (NODE_TYPE (assigntemp) != N_empty)) {

        if (NODE_TYPE (ASSIGN_INSTR (assigntemp)) == N_let) {
            idtemp = LET_IDS (ASSIGN_INSTR (assigntemp));

            while ((res) && (idtemp != NULL)) {
                res = res && strcmp (IDS_NAME (id), IDS_NAME (idtemp));

                idtemp = IDS_NEXT (idtemp);
            }
        }
        assigntemp = ASSIGN_NEXT (assigntemp);
    }
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   int checkExprsDependencies(node *outerpart, node *expr)
 *
 * description:
 *   checks if one of the Expressions in expr is computed inside
 *   the withloop-part outerpart.
 *   In this case a WLS would be impossible and FALSE is returned.
 *
 *   This function is NOT COMPLETED YET
 *
 * parameters:
 *   node *outerpart:   N_NPART
 *   node *expr:        "N_expr"
 *
 ******************************************************************************/
int
checkExprsDependencies (node *outerpart, node *expr)
{
    int res = TRUE;
    node *exprs;

    DBUG_ENTER ("checkExprsDependencies");

    if (expr == NULL)
        res = TRUE;
    else
      /* Generator parameters can be */
      /* a) N_array with N_nums or N_ids */
      if (NODE_TYPE (expr) == N_array) {
        exprs = ARRAY_AELEMS (expr);
        while (exprs != NULL) {
            if (NODE_TYPE (EXPRS_EXPR (exprs)) == N_id)
                res = res && checkIdDefinition (outerpart, ID_IDS (EXPRS_EXPR (exprs)));
            exprs = EXPRS_NEXT (exprs);
        }
    } else
        /* b) N_id
           res = checkIdDefinition(outerpart,ID_IDS(expr)); */
        res = FALSE;

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
 *   Compatibility means both Withloops are not FOLD-WLs
 *
 * parameters:
 *   node *outerWithOP:   N_NWITHOP
 *   node *innerWithOP:   N_NWITHOP
 *
 ******************************************************************************/
int
compatWLTypes (node *outerWithOP, node *innerWithOP)
{
    return (((NWITHOP_TYPE (outerWithOP) == WO_genarray)
             || (NWITHOP_TYPE (outerWithOP) == WO_modarray))
            && ((NWITHOP_TYPE (innerWithOP) == WO_genarray)
                || (NWITHOP_TYPE (innerWithOP) == WO_modarray)));
}

/******************************************************************************
 *
 * function:
 *   node *probePart(node *arg_node, node *arg_info)
 *
 * description:
 *
 * parameters:
 *   node *arg_node:   N_NPART
 *   node *arg_info:   N_INFO
 *
 ******************************************************************************/
node *
probePart (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("probePart");

    /* Inner CEXPR must be a nonscalar */
    if ((VARDEC_DIM (AVIS_VARDECORARG (ID_AVIS (NPART_CEXPR (arg_node)))) == 0) ||

        /* CEXPR of the withloop me be assigned in a known place */
        (NPART_SSAASSIGN (arg_node) == NULL)) {
        INFO_WLS_POSSIBLE (arg_info) = FALSE;
    } else {
        /* Check whether inner CEXPR is computer by an INNER withloop */
        if ((NODE_TYPE (NPART_LETEXPR (arg_node)) == N_Nwith)
            && (isAssignInsideBlock (NPART_SSAASSIGN (arg_node),
                                     BLOCK_INSTR (NPART_CBLOCK (arg_node))))) {

            /* initialize INFO_WLS_DIMS */
            if (INFO_WLS_DIMS (arg_info) == -1)
                INFO_WLS_DIMS (arg_info)
                  = VARDEC_SHAPE (IDS_VARDEC (NWITH_VEC (NPART_LETEXPR (arg_node))), 0);

            INFO_WLS_POSSIBLE (arg_info) =
              /* Inner withloop must be MG-WL */
              ((NWITH_PARTS (NPART_LETEXPR (arg_node)) > 0) &&

               /* Inner generators must be independent from the outer */
               (checkGeneratorDependencies (arg_node,
                                            NWITH_PART (NPART_LETEXPR (arg_node))))
               &&

               /* Both WLs must have compatible types */
               (compatWLTypes (INFO_WLS_WITHOP (arg_info),
                               NWITH_WITHOP (NPART_LETEXPR (arg_node))))
               &&

               /* In non aggressive mode, the nesting must be perfect */
               ((wls_aggressive == 2)
                || (BLOCK_INSTR (NPART_CBLOCK (arg_node)) == NPART_SSAASSIGN (arg_node)))
               &&

               /* In non aggressive mode, all the inner WLs must iterate over the same
                  dimensions */
               ((wls_aggressive == 2)
                || (INFO_WLS_DIMS (arg_info)
                    == VARDEC_SHAPE (IDS_VARDEC (NWITH_VEC (NPART_LETEXPR (arg_node))),
                                     0))));

        } else {
            INFO_WLS_POSSIBLE (arg_info) =
              /* In non aggressive mode, assignment of CEXPR must not be inside the WL */
              (((wls_aggressive == 2)
                || (!isAssignInsideBlock (NPART_SSAASSIGN (arg_node),
                                          BLOCK_INSTR (NPART_CBLOCK (arg_node)))))
               &&

               /* In non aggressive mode, CBLOCK must be empty */
               ((wls_aggressive == 2)
                || (NODE_TYPE (BLOCK_INSTR (NPART_CBLOCK (arg_node))) == N_empty)));
        }
    }
    DBUG_RETURN (arg_node);
}

/****************************************************************************
 *
 * Withloopification functions
 *
 ****************************************************************************/

node *
CreateCopyWithloop (node *arg_node, node *fundef)
{
    node *newwith;
    node *selid;

    DBUG_ENTER ("CreateCopyWithloop");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_id,
                 "CreateCopyWithloop called for non N_id node");

    newwith = CreateScalarWith (GetDim (ID_TYPE (arg_node)),
                                Type2Shpseg (ID_TYPE (arg_node), NULL),
                                GetBasetype (ID_TYPE (arg_node)), NULL, fundef);

    selid = MakeId (StringCopy (IDS_NAME (NWITH_VEC (newwith))), NULL, ST_regular);
    ID_VARDEC (selid) = IDS_VARDEC (NWITH_VEC (newwith));
    ID_AVIS (selid) = VARDEC_AVIS (ID_VARDEC (selid));

    ASSIGN_RHS (BLOCK_INSTR (NWITH_CBLOCK (newwith)))
      = MakePrf (F_sel, MakeExprs (selid, MakeExprs (DupNode (arg_node), NULL)));

    DBUG_RETURN (newwith);
}

node *
InsertCopyWithloop (node *arg_node, node *arg_info)
{
    node *assign;
    node *assid;
    node *vardec = NULL;

    DBUG_ENTER ("InsertCopyWithloop");

    assid = MakeId (TmpVar (), NULL, ST_regular);
    vardec = MakeVardec (StringCopy (ID_NAME (assid)),
                         DupOneTypes (ID_TYPE (NPART_CEXPR (arg_node))), vardec);

    ID_VARDEC (assid) = vardec;
    ID_AVIS (assid) = VARDEC_AVIS (vardec);

    AddVardecs (INFO_WLS_FUNDEF (arg_info), vardec);

    assign = MakeAssignLet (StringCopy (ID_NAME (assid)), vardec,
                            CreateCopyWithloop (NPART_CEXPR (arg_node),
                                                INFO_WLS_FUNDEF (arg_info)));

    ID_SSAASSIGN (assid) = assign;

    NPART_CEXPR (arg_node) = DupNode (assid);
    BLOCK_INSTR (NPART_CBLOCK (arg_node))
      = AppendAssign (BLOCK_INSTR (NPART_CBLOCK (arg_node)), assign);

    DBUG_RETURN (arg_node);
}

shpseg *
UpperBound (shpseg *shp, int dim)
{
    shpseg *res;
    int i;

    DBUG_ENTER ("UpperBound");

    res = DupShpseg (shp);
    for (i = 0; i < dim; i++)
        SHPSEG_SHAPE (res, i)++;

    DBUG_RETURN (res);
}

shpseg *
IncreaseShpseg (shpseg *shppos, shpseg *shpmax, int dim)
{
    shpseg *res;
    int i;

    DBUG_ENTER ("IncreaseShpseg");

    res = DupShpseg (shppos);

    for (i = dim - 1; i >= 0; i--) {
        SHPSEG_SHAPE (res, i)++;
        if (SHPSEG_SHAPE (res, i) >= SHPSEG_SHAPE (shpmax, i))
            SHPSEG_SHAPE (res, i) = 0;
        else
            break;
    }

    DBUG_RETURN (res);
}

node *
CreateExprsPart (node *exprs, int *partcount, node *withid, shpseg *shppos,
                 shpseg *shpmax, int dim, node *fundef, simpletype btype)
{
    node *res = NULL;
    node *next;
    node *assid;
    node *vardec = NULL;
    node *expr;

    DBUG_ENTER ("CreateExprsPart");

    if (exprs != NULL) {
        expr = DupNode (EXPRS_EXPR (exprs));

        assid = MakeId (TmpVar (), NULL, ST_regular);

        if (NODE_TYPE (expr) == N_id)
            vardec = MakeVardec (StringCopy (ID_NAME (assid)),
                                 DupOneTypes (ID_TYPE (expr)), vardec);
        else
            vardec
              = MakeVardec (StringCopy (ID_NAME (assid)), MakeTypes1 (btype), vardec);

        ID_VARDEC (assid) = vardec;
        ID_AVIS (assid) = VARDEC_AVIS (vardec);

        AddVardecs (fundef, vardec);

        res
          = MakeNPart (withid,
                       MakeNGenerator (Shpseg2Array (shppos, dim),
                                       Shpseg2Array (UpperBound (shppos, dim), dim), F_le,
                                       F_lt, NULL, NULL),
                       MakeNCode (MakeBlock (MakeAssignLet (StringCopy (ID_NAME (assid)),
                                                            vardec, expr),
                                             NULL),
                                  DupNode (assid)));

        shppos = IncreaseShpseg (shppos, shpmax, dim);
        next = CreateExprsPart (EXPRS_NEXT (exprs), partcount, withid, shppos, shpmax,
                                dim, fundef, btype);

        NPART_NEXT (res) = next;
        if (next != NULL)
            NCODE_NEXT (NPART_CODE (res)) = NPART_CODE (next);

        *partcount = *partcount + 1;
    }
    DBUG_RETURN (res);
}

node *
CreateArrayWithloop (node *array, node *fundef)
{
    int partcount = 0;
    node *res;
    node *part;
    node *withid;
    shpseg *shp;
    shpseg *shpmax;

    node *id;
    node *vardecs = NULL;
    ids *vec_ids;
    ids *scl_ids = NULL;
    ids *tmp_ids;
    int i;
    int dim;

    DBUG_ENTER ("CreateArrayWithloop");

    if (NODE_TYPE (array) == N_array) {
        dim = 1;
        shpmax = ARRAY_SHPSEG (array);
    } else {
        /* reshape */
        DBUG_ASSERT (NODE_TYPE (array) == N_prf, "Invalid Node Type!");
        DBUG_ASSERT (NODE_TYPE (PRF_ARGS (array)) == N_exprs,
                     "NODE_TYPE(PRF_ARGS(array)) != N_exprs");

        dim = ARRAY_SHAPE (EXPRS_EXPR (PRF_ARGS (array)), 0);
        shpmax = Array2Shpseg (EXPRS_EXPR (PRF_ARGS (array)), NULL);
        array = EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (array)));
    }

    vec_ids = MakeIds (TmpVar (), NULL, ST_regular);
    vardecs
      = MakeVardec (StringCopy (IDS_NAME (vec_ids)),
                    MakeTypes (T_int, 1, MakeShpseg (MakeNums (dim, NULL)), NULL, NULL),
                    vardecs);

    IDS_VARDEC (vec_ids) = vardecs;
    IDS_AVIS (vec_ids) = VARDEC_AVIS (vardecs);

    for (i = 0; i < dim; i++) {
        tmp_ids = MakeIds (TmpVar (), NULL, ST_regular);
        vardecs
          = MakeVardec (StringCopy (IDS_NAME (tmp_ids)), MakeTypes1 (T_int), vardecs);
        IDS_NEXT (tmp_ids) = scl_ids;
        scl_ids = tmp_ids;
        IDS_VARDEC (scl_ids) = vardecs;
        IDS_AVIS (scl_ids) = VARDEC_AVIS (vardecs);
    }

    id = MakeId (TmpVar (), NULL, ST_regular);
    vardecs = MakeVardec (StringCopy (ID_NAME (id)), MakeTypes1 (ARRAY_BASETYPE (array)),
                          vardecs);
    ID_VARDEC (id) = vardecs;
    ID_AVIS (id) = VARDEC_AVIS (vardecs);

    withid = MakeNWithid (vec_ids, scl_ids);

    shp = Array2Shpseg (CreateZeroVector (dim, T_int), NULL);

    part = CreateExprsPart (ARRAY_AELEMS (array), &partcount, withid, shp, shpmax, dim,
                            fundef, ARRAY_BASETYPE (array));

    res = MakeNWith (part, NPART_CODE (part),
                     MakeNWithOp (WO_genarray,
                                  Shpseg2Array (TYPES_SHPSEG (ARRAY_TYPE (array)), dim)));

    NWITH_PARTS (res) = partcount;

    AddVardecs (fundef, vardecs);

    DBUG_RETURN (res);
}

node *
Array2Withloop (node *arg_node, node *arg_info)
{
    node *assign;
    node *assid;
    node *vardec = NULL;

    DBUG_ENTER ("Array2Withloop");

    assid = MakeId (TmpVar (), NULL, ST_regular);
    vardec = MakeVardec (StringCopy (ID_NAME (assid)),
                         DupOneTypes (ID_TYPE (NPART_CEXPR (arg_node))), vardec);

    ID_VARDEC (assid) = vardec;
    ID_AVIS (assid) = VARDEC_AVIS (vardec);

    AddVardecs (INFO_WLS_FUNDEF (arg_info), vardec);

    assign = MakeAssignLet (StringCopy (ID_NAME (assid)), vardec,
                            CreateArrayWithloop (NPART_LETEXPR (arg_node),
                                                 INFO_WLS_FUNDEF (arg_info)));

    ID_SSAASSIGN (assid) = assign;

    NPART_CEXPR (arg_node) = DupNode (assid);
    BLOCK_INSTR (NPART_CBLOCK (arg_node))
      = AppendAssign (BLOCK_INSTR (NPART_CBLOCK (arg_node)), assign);

    DBUG_RETURN (arg_node);
}

node *
withloopifyPart (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("withloopifyPart");

    if (!((NODE_TYPE (NPART_LETEXPR (arg_node)) == N_Nwith)
          && (isAssignInsideBlock (NPART_SSAASSIGN (arg_node),
                                   BLOCK_INSTR (NPART_CBLOCK (arg_node))))
          && (compatWLTypes (INFO_WLS_WITHOP (arg_info),
                             NWITH_WITHOP (NPART_LETEXPR (arg_node)))))) {
        if ((NODE_TYPE (NPART_LETEXPR (arg_node)) == N_array)
            || ((NODE_TYPE (NPART_LETEXPR (arg_node)) == N_prf)
                && (PRF_PRF (NPART_LETEXPR (arg_node)) == F_reshape)
                && (NODE_TYPE (
                     EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (NPART_LETEXPR (arg_node))))))
                     == N_array))
            arg_node = Array2Withloop (arg_node, arg_info);
        else
            arg_node = InsertCopyWithloop (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/****************************************************************************
 *
 * Generator Normalization functions
 *
 ****************************************************************************/

node *
Nid2Narray (node *nid, node *arg_info)
{
    node *res;
    node *exprs = NULL;

    node *assign;
    node *assign_chain = NULL;
    char *newname;
    node *vardec;
    node *vardec_chain = NULL;

    node *nodep;
    node **blockrun;

    int i;

    DBUG_ENTER ("Nid2Narray");

    res = CreateZeroVector (ID_SHAPE (nid, 0), T_int);

    /* Create Assignments for all positions in the N_id */
    for (i = 0; i < ID_SHAPE (nid, 0); i++) {
        newname = TmpVar ();

        vardec = MakeVardec (StringCopy (TmpVar ()),
                             MakeTypes (T_int, 0, NULL, NULL, NULL), NULL);

        vardec_chain = AppendVardec (vardec_chain, vardec);

        assign
          = MakeAssignLet (StringCopy (newname), vardec,
                           MakePrf2 (F_sel, MakeArray (MakeExprs (MakeNum (i), NULL)),
                                     nid));

        assign_chain = AppendAssign (assign_chain, assign);

        exprs = CombineExprs (exprs,
                              MakeExprs (MakeId (StringCopy (newname), NULL, ST_regular),
                                         NULL));
    }

    /* Insert the assignment-chain in the right position */
    blockrun = &(BLOCK_INSTR (INFO_WLS_BLOCK (arg_info)));
    nodep = *blockrun;
    while (!((NODE_TYPE (nodep) == N_assign)
             && (NODE_TYPE (ASSIGN_INSTR (nodep)) == N_let)
             && (NODE_TYPE (ASSIGN_RHS (nodep)) == N_Nwith)
             && (NWITH_WITHOP (ASSIGN_RHS (nodep)) == INFO_WLS_WITHOP (arg_info)))) {
        blockrun = &(ASSIGN_NEXT (nodep));
        nodep = *blockrun;
    }

    assign_chain = AppendAssign (assign_chain, *blockrun);
    *blockrun = assign_chain;

    ARRAY_AELEMS (res) = exprs;

    /* Return N_array node */
    DBUG_RETURN (res);
}

node *
NormGenerator (node *gen, node *arg_info)
{
    DBUG_ENTER ("NormGenerator");

    if (NODE_TYPE (NGEN_BOUND1 (gen)) == N_id)
        NGEN_BOUND1 (gen) = Nid2Narray (NGEN_BOUND1 (gen), arg_info);

    if (NODE_TYPE (NGEN_BOUND2 (gen)) == N_id)
        NGEN_BOUND2 (gen) = Nid2Narray (NGEN_BOUND2 (gen), arg_info);

    if ((NGEN_STEP (gen) != NULL) && (NODE_TYPE (NGEN_STEP (gen)) == N_id))
        NGEN_STEP (gen) = Nid2Narray (NGEN_STEP (gen), arg_info);

    if ((NGEN_WIDTH (gen) != NULL) && (NODE_TYPE (NGEN_WIDTH (gen)) == N_id))
        NGEN_WIDTH (gen) = Nid2Narray (NGEN_WIDTH (gen), arg_info);

    DBUG_RETURN (gen);
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
    LUT_t lut;

    node *vardec;
    ids *ids;
    node *vardec_chain;
    char *new_name;
    node *temp;

    DBUG_ENTER ("distributePart");

    innerwith = NPART_LETEXPR (arg_node);

    DBUG_ASSERT (NWITH_PARTS (innerwith) >= 1, "NWITH_PARTS(innerwith) < 1");

    /* are there more two or more inner parts to distribute? */
    if (NWITH_PARTS (innerwith) <= 1)
        res = arg_node;
    else {
        /* duplicate this part and make the copy the next part in the chain. */

        /* Therefore we have to rename all assignments, we use a LUT */

        lut = GenerateLUT ();
        vardec = NULL;
        vardec_chain = NULL;
        temp = BLOCK_INSTR (NPART_CBLOCK (arg_node));

        /* Create all vardec and insert the names and pointers into the LUT */
        while ((temp != NULL) && (NODE_TYPE (temp) != N_empty)) {
            if (NODE_TYPE (ASSIGN_INSTR (temp)) == N_let) {
                ids = LET_IDS (ASSIGN_INSTR (temp));

                while (ids != NULL) {
                    new_name = TmpVar ();
                    vardec = DupNode (IDS_VARDEC (ids));
                    VARDEC_NAME (vardec) = StringCopy (new_name);

                    InsertIntoLUT_S (lut, IDS_NAME (ids), new_name);
                    InsertIntoLUT_P (lut, IDS_VARDEC (ids), vardec);
                    InsertIntoLUT_P (lut, IDS_AVIS (ids), VARDEC_AVIS (vardec));

                    if (vardec_chain == NULL)
                        vardec_chain = vardec;
                    else
                        vardec_chain = AppendVardec (vardec_chain, vardec);

                    ids = IDS_NEXT (ids);
                }
            }
            temp = ASSIGN_NEXT (temp);
        }

        /* append the vardecs to the function's vardecs */
        FUNDEF_VARDEC (INFO_WLS_FUNDEF (arg_info))
          = AppendVardec (FUNDEF_VARDEC (INFO_WLS_FUNDEF (arg_info)), vardec_chain);

        /* Duplicate the part */
        tmpnode = DupTreeLUT (arg_node, lut);
        NPART_CODE (tmpnode) = DupTreeLUT (NPART_CODE (arg_node), lut);
        NCODE_USED (NPART_CODE (tmpnode))++;

        RemoveLUT (lut);

        /* Make the AVIS-nodes point to the correct assignments */
        temp = BLOCK_INSTR (NPART_CBLOCK (tmpnode));
        vardec = vardec_chain;

        while ((temp != NULL) && (NODE_TYPE (temp) != N_empty)) {
            if (NODE_TYPE (ASSIGN_INSTR (temp)) == N_let) {
                ids = LET_IDS (ASSIGN_INSTR (temp));
                while (ids != NULL) {
                    AVIS_SSAASSIGN (VARDEC_AVIS (vardec)) = temp;
                    ids = IDS_NEXT (ids);
                }
                vardec = VARDEC_NEXT (vardec);
            }
            temp = ASSIGN_NEXT (temp);
        }

        /* Insert the part into the chain of parts */
        NCODE_NEXT (NPART_CODE (arg_node)) = NPART_CODE (tmpnode);
        NPART_NEXT (arg_node) = tmpnode;

        /* Drop all parts except of the first */
        NWITH_PARTS (innerwith) = -1;
        NPART_NEXT (NWITH_PART (innerwith)) = NULL;
        NCODE_NEXT (NWITH_CODE (innerwith)) = NULL;

        DBUG_ASSERT (innerwith != NPART_LETEXPR (tmpnode), "innerwith == NPART_LETEXPR");

        /* Drop the first part from the inner withloop of the next part */
        innerwith = NPART_LETEXPR (tmpnode);
        NWITH_PARTS (innerwith) -= 1;
        NWITH_PART (innerwith) = NPART_NEXT (NWITH_PART (innerwith));
        NWITH_CODE (innerwith) = NPART_CODE (NWITH_PART (innerwith));

        /* increase the outer withloop's partcounter */
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
 *   node *joinGenerators(node *outerpart, node *innerpart)
 *
 * description:
 *   Creates a new generator that iterates over the same space as the
 *   two generators specified in the paramters by concatenating
 *   all the vectors.
 *
 * parameters:
 *   node *outerpart:   N_NPART
 *   node *innerpart:   N_NPART
 *
 ******************************************************************************/
node *
joinGenerators (node *outerpart, node *innerpart)
{
    node *newgen;
    node *outergen, *innergen;

    node *b1, *b2, *s, *w;
    int d1, d2;

    DBUG_ENTER ("joinGenerators");

    innergen = NPART_GEN (innerpart);
    outergen = NPART_GEN (outerpart);

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

    /* Generate a new WITHID if we don't have one already */
    if (INFO_WLS_WITHID (arg_info) == NULL) {
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

        scalars = AppendIds (DupAllIds (NWITHID_IDS (outerwithid)),
                             DupAllIds (NWITHID_IDS (innerwithid)));

        INFO_WLS_WITHID (arg_info) = MakeNWithid (vec, scalars);
    }

    newwithid = DupTree (INFO_WLS_WITHID (arg_info));

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
           node *newwithid, node *arg_info)
{
    node *newcode;
    node *tmp_node;
    node *tmp_node2;
    node *array;

    ids *idp1, *idp2;

    DBUG_ENTER ("joinCodes");

    /* The new code is the old INNER part's code ... */
    newcode = DupTree (innercode);
    tmp_node = BLOCK_INSTR (NCODE_CBLOCK (newcode));

    BLOCK_INSTR (NCODE_CBLOCK (newcode))
      = DupTree (BLOCK_INSTR (NCODE_CBLOCK (outercode)));

    tmp_node2 = BLOCK_INSTR (NCODE_CBLOCK (newcode));

    if (ASSIGN_NEXT (tmp_node2) == NULL)
        BLOCK_INSTR (NCODE_CBLOCK (newcode)) = tmp_node;
    else {
        while (ASSIGN_NEXT (ASSIGN_NEXT (tmp_node2)) != NULL)
            tmp_node2 = ASSIGN_NEXT (tmp_node2);
        if (NODE_TYPE (tmp_node) != N_empty)
            ASSIGN_NEXT (tmp_node2) = tmp_node;
        else
            ASSIGN_NEXT (tmp_node2) = NULL;
    }

    /* prepended with definitions of the two old WITHVECs */
    array = MakeArray (MakeExprsIdChain (DupAllIds (NWITHID_IDS (outerwithid))));

    ARRAY_TYPE (array)
      = MakeTypes (T_int, 1,
                   MakeShpseg (MakeNums (CountExprs (ARRAY_AELEMS (array)), NULL)), NULL,
                   NULL);

    tmp_node = MakeAssignLet (IDS_NAME (NWITHID_VEC (outerwithid)),
                              IDS_VARDEC (NWITHID_VEC (outerwithid)), array);

    idp1 = NWITHID_IDS (newwithid);
    idp2 = NWITHID_IDS (outerwithid);
    while (idp2 != NULL) {
        idp1 = IDS_NEXT (idp1);
        idp2 = IDS_NEXT (idp2);
    }

    array = MakeArray (MakeExprsIdChain (DupAllIds (idp1)));

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
 *   node *scalarizePart(node *outerpart, node *arg_info)
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
scalarizePart (node *outerpart, node *arg_info)
{
    node *newpart;

    node *innerpart;

    node *generator;
    node *withid;
    node *code;

    LUT_t lut;
    ids *oldids;
    ids *newids;
    int diff;

    DBUG_ENTER ("scalarizePart");

    innerpart = NWITH_PART (NPART_LETEXPR (outerpart));

    /* Make a new generator */
    generator = joinGenerators (outerpart, innerpart);

    /* Make a new withid */
    withid = joinWithids (NPART_WITHID (outerpart), NPART_WITHID (innerpart), arg_info);

    /* Make new code */
    code
      = joinCodes (NPART_CODE (outerpart), NPART_CODE (innerpart),
                   NPART_WITHID (outerpart), NPART_WITHID (innerpart), withid, arg_info);

    /* Create a new LUT */
    lut = GenerateLUT ();

    /* Rename all occurences of old ids */
    oldids = NWITHID_IDS (NPART_WITHID (innerpart));
    newids = NWITHID_IDS (withid);

    diff = CountIds (newids) - CountIds (oldids);
    while (diff-- > 0)
        newids = IDS_NEXT (newids);

    while (oldids != NULL) {
        InsertIntoLUT_S (lut, IDS_NAME (oldids), IDS_NAME (newids));
        InsertIntoLUT_P (lut, IDS_VARDEC (oldids), IDS_VARDEC (newids));
        InsertIntoLUT_P (lut, IDS_AVIS (oldids), IDS_AVIS (newids));

        oldids = IDS_NEXT (oldids);
        newids = IDS_NEXT (newids);
    }
    /* Now we can build a new part */
    newpart = MakeNPart (withid, generator, DupTreeLUT (code, lut));

    RemoveLUT (lut);

    /* Rebuild the chain */
    NPART_NEXT (newpart) = NPART_NEXT (outerpart);

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
 *   node *WLSblock(node *arg_node, node *arg_info)
 *
 * description:
 *   This function's sole purpose is to annotate N_block in which the WLS
 *   is about to take place
 *
 * parameters:
 *   node *arg_node:   N_block
 *   node *arg_info:   N_INFO
 *
 ******************************************************************************/
node *
WLSblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLSblock");

    INFO_WLS_BLOCK (arg_info) = arg_node;

    if (BLOCK_INSTR (arg_node) != NULL) {
        /* traverse instructions of block */
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
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
    node *outerblock;

    DBUG_ENTER ("WLSNwith");

    DBUG_PRINT ("WLS", ("\nstarting with-loop scalarization in Nwith %s.",
                        FUNDEF_NAME (arg_node)));

    outerblock = INFO_WLS_BLOCK (arg_info);

    /* First WLS traverses into the branches in order to scalarize and
       fold all other With-Loops */

    /* traverse all CODES */
    if (NWITH_CODE (arg_node) != NULL) {
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    }

    /***************************************************************************
     *
     *  PROBING
     *
     *  We have to find out, whether all parts can be scalarized
     *  Here we can distinguish between conservative and aggressive behaviour
     *
     ***************************************************************************/

    tmpnode = arg_info;
    arg_info = MakeInfo ();
    INFO_WLS_FUNDEF (arg_info) = INFO_WLS_FUNDEF (tmpnode);
    INFO_WLS_BLOCK (arg_info) = outerblock;

    /* Check if WLS is possible vor all parts */
    INFO_WLS_PARTS (arg_info) = NWITH_PARTS (arg_node);

    /* First of all, the Withloop must be a MG-WL */
    INFO_WLS_POSSIBLE (arg_info) = INFO_WLS_PARTS (arg_info) > 0;

    if (INFO_WLS_POSSIBLE (arg_info)) {
        WLS_PHASE (arg_info) = wls_probe;
        INFO_WLS_WITHOP (arg_info) = NWITH_WITHOP (arg_node);
        INFO_WLS_DIMS (arg_info) = -1;

        if (NWITH_PART (arg_node) != NULL) {
            NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
        }
    }

    /* If everything is ok, we can start the scalarization */
    if (INFO_WLS_POSSIBLE (arg_info)) {

        /***************************************************************************
         *
         *  WITHLOOPIFICATION
         *
         ***************************************************************************/

        WLS_PHASE (arg_info) = wls_withloopification;

        if (NWITH_PART (arg_node) != NULL) {
            NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
        }

        /***************************************************************************
         *
         *  GENERATOR NORMALIZATION
         *
         *  Scalarazization can only be done if all vectors in all generators
         *  are N_array-nodes. They must not be N_id-nodes.
         *
         ***************************************************************************/

        WLS_PHASE (arg_info) = wls_normgen;

        if (NWITH_PART (arg_node) != NULL) {
            NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
        }

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

        NWITH_PARTS (arg_node) = INFO_WLS_PARTS (arg_info);

        /***************************************************************************
         *
         *  CODECORRECTION I
         *
         ***************************************************************************/

        WLS_PHASE (arg_info) = wls_codecorrect;

        NWITH_CODE (arg_node) = NPART_CODE (NWITH_PART (arg_node));
        if (NWITH_PART (arg_node) != NULL) {
            NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
        }

        /***************************************************************************
         *
         *  SCALARIZATION
         *
         ***************************************************************************/

        WLS_PHASE (arg_info) = wls_scalarize;

        INFO_WLS_WITHID (arg_info) = NULL;

        /* The new shape is the concatenation of both old shapes */

        if (NWITHOP_TYPE (INFO_WLS_WITHOP (arg_info)) == WO_genarray)
            NWITH_SHAPE (arg_node)
              = ConcatVecs (NWITH_SHAPE (arg_node),
                            NWITH_SHAPE (NPART_LETEXPR (NWITH_PART (arg_node))));

        /* traverse all PARTS  */

        if (NWITH_PART (arg_node) != NULL) {
            NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
        }

        /***************************************************************************
         *
         *  CODECORRECTION II
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
    node *innerpart;

    DBUG_ENTER ("WLSNpart");

    switch (WLS_PHASE (arg_info)) {
    case wls_probe:
        arg_node = probePart (arg_node, arg_info);

        if ((INFO_WLS_POSSIBLE (arg_info)) && (NPART_NEXT (arg_node) != NULL)) {
            NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
        }
        break;

    case wls_withloopification:
        arg_node = withloopifyPart (arg_node, arg_info);

        if ((INFO_WLS_POSSIBLE (arg_info)) && (NPART_NEXT (arg_node) != NULL)) {
            NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
        }
        break;

    case wls_normgen:
        NPART_GEN (arg_node) = NormGenerator (NPART_GEN (arg_node), arg_info);

        innerpart = NWITH_PART (NPART_LETEXPR (arg_node));

        while (innerpart != NULL) {
            NPART_GEN (innerpart) = NormGenerator (NPART_GEN (innerpart), arg_info);
            innerpart = NPART_NEXT (innerpart);
        }

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

    case wls_scalarize:
        arg_node = scalarizePart (arg_node, arg_info);

        if (NPART_NEXT (arg_node) != NULL) {
            NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
        }
        break;

    case wls_codecorrect:
        if (NPART_NEXT (arg_node) != NULL) {
            NCODE_NEXT (NPART_CODE (arg_node)) = NPART_CODE (NPART_NEXT (arg_node));

            NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
        } else
            NCODE_NEXT (NPART_CODE (arg_node)) = NULL;
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
