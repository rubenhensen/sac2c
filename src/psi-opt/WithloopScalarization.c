/*
 *
 * $Log$
 * Revision 1.30  2003/03/13 07:22:02  ktr
 * Fixed bug #7 WLS does illegal transformation on non-scalar fold-WLs.
 *
 * Revision 1.29  2003/03/09 13:47:42  ktr
 * removed an unnecessary printf-line.
 *
 * Revision 1.28  2003/03/07 22:16:37  ktr
 * aggressive behaviour now creates copy-withloops for all expressions
 * that cannot be handled by other mechanisms.
 *
 * Revision 1.27  2003/01/28 18:19:02  ktr
 * Much cleaner version, does not recycle old WLs.
 * Phases CODE UNSHARE and DISTRIBUTE eliminated
 * Shared code is treated correctly
 * wlcorrect now detects shared code. This should be extended into a new optimization.
 *
 * Revision 1.26  2003/01/27 16:18:58  ktr
 * Code is now explicitely unshared before WLS
 *
 * Revision 1.25  2003/01/25 22:06:23  ktr
 * Fixed a lot of memory management and related issues.
 *
 * Revision 1.24  2002/11/04 21:16:10  ktr
 * New feature: Index Vector acceleration!!!
 *
 * Revision 1.23  2002/11/02 18:46:47  ktr
 * Withloopification phase now handles arrays of arrays correctly.
 *
 * Revision 1.22  2002/10/31 17:58:31  ktr
 * fixed a nasty bug about handling inner WLs with more than 2 parts.
 *
 * Revision 1.21  2002/10/30 11:47:04  ktr
 * Fixed a bug that made the new wl have a wrong shape when the inner wl
 * was a modarray-wl.
 *
 * Revision 1.20  2002/10/24 13:10:22  ktr
 * level of aggresiveness now controlled by flag -wls_aggressive
 *
 * Revision 1.19  2002/10/23 10:19:29  ktr
 * ASSIGN_INDENT is now increased by the number of dimensions of the inner with loop.
 *
 * Revision 1.18  2002/10/20 13:23:59  ktr
 * Added support for WLS N_assign indentation by adding field ASSIGN_INDENT(n)
 * to N_assign node which is increased on every indentation performed by WLS.
 *
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

/**
 *
 * @file WithloopScalarization.c
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
 *   - Phase 2: Withloopification
 *
 *     Withloopification makes WLS more applicable in case of the inner
 *     non-scalar not being a withloop.
 *     If the inner non-scalar is an N_array-node then a withloop with a
 *     part for each field in the Array is created, otherwise a copy-
 *     withloop is inserted.
 *
 *
 *   - Phase 3: Generator normalization
 *
 *     Since WLS cannot handle non-scalar ids in the boundaries of a part's
 *     generator, these ids are transformed to N_array nodes containg
 *     scalar values for each member of the previous vector.
 *
 *
 *   - Phase 4: Scalarization
 *
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
 ****************************************************************************
 *
 *  Usage of arg_info:
 *
 *  - flag   : POSSIBLE  : WL nesting is transformable.
 *                         (Detected in phase PROBE)
 *  - linno  : PHASE     : Phase of the WithloopScalarization
 *                         (PROBE | WITHLOOPIFICATION| NORMGEN |
 *                          DISTRIBUTE | SCALARIZE)
 *  - node[3]: FUNDEF    : pointer to current fundef node,
 *                         needed to access vardecs
 *  - counter: PARTS     : Number of parts of the new MG-Withloop
 *                         (calculated in phase DISTRIBUTE)
 *  - node[0]: WITHID    : reference to the outer WL's withid
 *  - node[1]: WITHOP    : reference to the outer WL's withop,
 *                         needed to check if both WL's types match
 *  - varno  : DIMS      : used in PROBE to count the inner wls' indexscalars
 *  - node[2]: BLOCK     : reference to the surrounding block of a wl
 *
 *
 */

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
#include "print.h"
#include "compare_tree.h"

#include "WithloopScalarization.h"

/****************************************************************************
 *
 * typedef
 *
 ****************************************************************************/

/**
 * Several traverse functions of this file are traversed for different
 * purposes. This enum determines ths function, see description at the
 * beginning of this file for details
 */
typedef enum {
    wls_probe,
    wls_withloopification,
    wls_normgen,
    wls_scalarize
} wls_phase_type;

/**
 * Structure needed to preserve code sharing
 */
typedef struct CODE_T {
    node *outercode;
    node *innercode;
    node *newcode;
    struct CODE_T *next;
} code_t;

/**
 * returns the current phase of WLS
 */
#define WLS_PHASE(n) ((wls_phase_type)INFO_WLS_PHASE (n))

/****************************************************************************
 *
 * Helper macros
 *
 ****************************************************************************/

/**
 * returns the assignment of the parts' CEXPR
 */
#define NPART_SSAASSIGN(n) (ID_SSAASSIGN (NPART_CEXPR (n)))

/**
 * returns the defining node of the part's CEXPR.
 * It is mainly used to find the inner WL.
 */
#define NPART_LETEXPR(n) (LET_EXPR (ASSIGN_INSTR (NPART_SSAASSIGN (n))))

/****************************************************************************
 *
 * Helper functions
 *
 ****************************************************************************/

/**
 * Creates an one-dimensional Array (aka Vector) of length nr whose
 * elements are all Nums with value 1.
 *
 * @param nr the nec vector's length
 *
 * @return A one-dimensional N_array (vector) of length nr whose elements
 * are Nums with value 1.
 */
node *
CreateOneVector (int nr)
{
    node *res;
    node *temp;

    DBUG_ENTER ("MakeOnes");

    DBUG_ASSERT (nr > 0, "CreateOneVector called with nr <= 0!");

    res = CreateZeroVector (nr, T_int);

    temp = ARRAY_AELEMS (res);

    while (temp != NULL) {
        NUM_VAL (EXPRS_EXPR (temp)) = 1;
        temp = EXPRS_NEXT (temp);
    }

    DBUG_RETURN (res);
}

/**
 * converts a chain of ids into an exprs-node.
 *
 * @param idschain a chain of ids
 *
 * @return An exprs-node containing all the ids in idschain
 */
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

/**
 * concatenates two vectors.
 *
 * @param vec1 A N_array node containing the first vector
 * @param vec2 A N_array node containing the second vector
 *
 * @return The concatenation vec1++vec2 as an N_array node
 */
node *
ConcatVecs (node *vec1, node *vec2)
{
    node *res;

    DBUG_ENTER ("CONCAT_VECS");

    DBUG_ASSERT (((NODE_TYPE (vec1) == N_array) && (NODE_TYPE (vec2) == N_array)),
                 "ConcatVecs called with not N_array nodes");

    res = CreateZeroVector (ARRAY_SHAPE (vec1, 0) + ARRAY_SHAPE (vec2, 0), T_int);

    ARRAY_AELEMS (res)
      = CombineExprs (DupTree (ARRAY_AELEMS (vec1)), DupTree (ARRAY_AELEMS (vec2)));

    ((int *)ARRAY_CONSTVEC (res)) = Array2IntVec (ARRAY_AELEMS (res), NULL);

    DBUG_RETURN (res);
}

/**
 * corrects the part/code pointer structure of a withloop in three steps.
 *
 * - add all parts' codes to the NWITH_CODE list
 * - share as many codes as possible
 * - determine which of the codes in NWITH_CODE list are actually used
 * - remove all unused codes
 *
 * @param arg_node The N_with node whose structure is to be fixed
 *
 * @return A correct N_with node
 */
node *
correctWL (node *arg_node)
{
    node *temp, *tempcode;
    node *tc1, *tc2;
    node **codepp;

    LUT_t lut;

    DBUG_ENTER ("correctWL");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_Nwith,
                 "correctWL called for non N_Nwith node");

    /* add new codes */
    temp = NWITH_PART (arg_node);

    while (temp != NULL) {
        tempcode = NWITH_CODE (arg_node);

        while ((tempcode != NULL) && (tempcode != NPART_CODE (temp)))
            tempcode = NCODE_NEXT (tempcode);

        if (tempcode == NULL) {
            NCODE_NEXT (NPART_CODE (temp)) = NWITH_CODE (arg_node);
            NWITH_CODE (arg_node) = NPART_CODE (temp);
        }
        temp = NPART_NEXT (temp);
    }

    /* share codes */
    temp = NWITH_PART (arg_node);

    if (temp != NULL)
        temp = NPART_NEXT (temp);

    while (temp != NULL) {
        tempcode = NWITH_PART (arg_node);
        lut = GenerateLUT ();

        tc1 = NCODE_NEXT (NPART_CODE (temp));
        NCODE_NEXT (NPART_CODE (temp)) = NULL;
        tc2 = NCODE_NEXT (NPART_CODE (tempcode));
        NCODE_NEXT (NPART_CODE (tempcode)) = NULL;

        while ((tempcode != temp) && (NPART_CODE (tempcode) != NPART_CODE (temp))
               && (CompareTreeLUT (NPART_CODE (temp), NPART_CODE (tempcode), lut)
                   == CMPT_NEQ)) {
            NCODE_NEXT (NPART_CODE (tempcode)) = tc2;
            tempcode = NPART_NEXT (tempcode);
            tc2 = NCODE_NEXT (NPART_CODE (tempcode));
            NCODE_NEXT (NPART_CODE (tempcode)) = NULL;
            lut = RemoveLUT (lut);
            lut = GenerateLUT ();
        }

        lut = RemoveLUT (lut);

        NCODE_NEXT (NPART_CODE (tempcode)) = tc2;
        NCODE_NEXT (NPART_CODE (temp)) = tc1;

        /* share code */
        if ((tempcode != temp) && (NPART_CODE (tempcode) != NPART_CODE (temp)))
            NPART_CODE (temp) = NPART_CODE (tempcode);

        temp = NPART_NEXT (temp);
    }

    /* determine unused codes */
    temp = NWITH_CODE (arg_node);
    while (temp != NULL) {
        NCODE_USED (temp) = 0;
        temp = NCODE_NEXT (temp);
    }

    temp = NWITH_PART (arg_node);
    while (temp != NULL) {
        NCODE_USED (NPART_CODE (temp))++;
        temp = NPART_NEXT (temp);
    }

    /* free unused codes */
    codepp = &(NWITH_CODE (arg_node));

    while (*codepp != NULL) {
        while ((*codepp != NULL) && (NCODE_USED (*codepp) == 0))
            *codepp = FreeNode (*codepp);
        if (*codepp != NULL)
            codepp = &(NCODE_NEXT (*codepp));
    }

    DBUG_RETURN (arg_node);
}

/****************************************************************************
 *
 * Probing functions
 *
 ****************************************************************************/

/**
 * checks if an assignment occurs as a part of a list of instructions.
 *
 * unfortunately this is a tail-end recursive implementation
 *
 * @param assign the assignment to look for inside of
 * @param instr the list of instructions
 *
 * @return TRUE if and only if assign occurs in instr.
 */
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

/**
 * checks whether a variable is defined inside a given N_Npart.
 *
 * @param outerpart The N_Npart node to be searched for the definition of id
 * @param id The identifiert whose definition is to be found
 *
 * @return FALSE if and only if id is defined in outerpart
 */
int
checkIdDefinition (node *outerpart, ids *id)
{
    int res = TRUE;
    ids *idtemp;
    node *assigntemp;

    DBUG_ENTER ("checkIdDefinition");

    DBUG_ASSERT (NODE_TYPE (outerpart) == N_Nwith,
                 "checkIdDefinition: first Argument must be called with a N_Npart node");

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

/**
 * checks if one of the Expressions in expr is computed inside
 * the withloop-part outerpart.
 * In this case a WLS would be impossible and FALSE is returned.
 *
 * This function is NOT COMPLETED YET since a search for an N_id always
 * returns FALSE
 *
 * @param outerpart An N_Npart in which expr is to be looked for.
 * @param expr An N_array or N_id node containing the EXPRS_EXPR to be checked
 *
 * @return
 **/
int
checkExprsDependencies (node *outerpart, node *expr)
{
    int res = TRUE;
    node *exprs;

    DBUG_ENTER ("checkExprsDependencies");

    DBUG_ASSERT (((NODE_TYPE (outerpart) == N_Npart)
                  && ((NODE_TYPE (expr) == N_array) || (NODE_TYPE (expr) == N_id))),
                 "checkExprsDepencies: wrong arguments!");

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

/**
 * checks if one of the generators of the inner withloop depends on
 * calculations in the withloop-part outerpart.
 * In this case a WLS would be impossible and FALSE is returned.
 *
 * @param outerpart  N_NPART
 * @param innerpart  N_NPART
 *
 * @return TRUE If and only if the generator of innerpart does not
 * depend on outerpart
 */
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

/**
 * checks if the two nested withloops have compatible types
 *
 * Compatibility means both Withloops are not FOLD-WLs
 *
 * @param outerWithOP   N_NWITHOP
 * @param innerWithOP   N_NWITHOP
 *
 * @return A boolean stating whether both WithOps are not WO_fold.
 */
int
compatWLTypes (node *outerWithOP, node *innerWithOP)
{
    return (((NWITHOP_TYPE (outerWithOP) == WO_genarray)
             || (NWITHOP_TYPE (outerWithOP) == WO_modarray))
            && ((NWITHOP_TYPE (innerWithOP) == WO_genarray)
                || (NWITHOP_TYPE (innerWithOP) == WO_modarray)));
}

/**
 * checks whether a part satisfies all the conditions needed in order to be
 * scalarized.
 *
 * @param arg_node N_NPART
 * @param arg_info N_INFO
 *
 * @return TRUE if and only if the part is able to be scalarized
 */
node *
probePart (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("probePart");

    /* Inner CEXPR must be a nonscalar */
    /* Outer Withloop must not be a fold Withloop */
    if ((VARDEC_DIM (AVIS_VARDECORARG (ID_AVIS (NPART_CEXPR (arg_node)))) == 0)
        || ((NWITHOP_TYPE (INFO_WLS_WITHOP (arg_info)) != WO_modarray)
            && (NWITHOP_TYPE (INFO_WLS_WITHOP (arg_info)) != WO_genarray))) {
        INFO_WLS_POSSIBLE (arg_info) = FALSE;
    } else {
        /* special case: array is to be filled with the index vector */
        if (NPART_SSAASSIGN (arg_node) != NULL) {
            /* Check whether inner CEXPR is computer by an INNER withloop */
            if ((NODE_TYPE (NPART_LETEXPR (arg_node)) == N_Nwith)
                && (isAssignInsideBlock (NPART_SSAASSIGN (arg_node),
                                         BLOCK_INSTR (NPART_CBLOCK (arg_node))))
                && (NWITH_PARTS (NPART_LETEXPR (arg_node)) > 0)) {

                /* initialize INFO_WLS_DIMS */
                if (INFO_WLS_DIMS (arg_info) == -1)
                    INFO_WLS_DIMS (arg_info)
                      = VARDEC_SHAPE (IDS_VARDEC (NWITH_VEC (NPART_LETEXPR (arg_node))),
                                      0);

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
                   ((wls_aggressive)
                    || (BLOCK_INSTR (NPART_CBLOCK (arg_node))
                        == NPART_SSAASSIGN (arg_node)))
                   &&

                   /* In non aggressive mode, all the inner WLs must
                      iterate over the same dimensions */
                   ((wls_aggressive)
                    || (INFO_WLS_DIMS (arg_info)
                        == VARDEC_SHAPE (IDS_VARDEC (
                                           NWITH_VEC (NPART_LETEXPR (arg_node))),
                                         0))));

            } else {
                INFO_WLS_POSSIBLE (arg_info) =
                  /* In non aggressive mode, assignment of CEXPR
                     must not be inside the WL */
                  (((wls_aggressive)
                    || (!isAssignInsideBlock (NPART_SSAASSIGN (arg_node),
                                              BLOCK_INSTR (NPART_CBLOCK (arg_node)))))
                   &&

                   /* In non aggressive mode, CBLOCK must be empty */
                   ((wls_aggressive)
                    || (NODE_TYPE (BLOCK_INSTR (NPART_CBLOCK (arg_node))) == N_empty)));
            }
        }
    }
    DBUG_RETURN (arg_node);
}

/****************************************************************************
 *
 * Withloopification functions
 *
 ****************************************************************************/

/**
 *
 */
node *
CreateCopyWithloop (node *arg_node, node *arg_info)
{
    node *newwith;
    node *selid;

    node *fundef;

    DBUG_ENTER ("CreateCopyWithloop");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_id,
                 "CreateCopyWithloop called for non N_id node");

    fundef = INFO_WLS_FUNDEF (arg_info);

    newwith
      = CreateScalarWith (INFO_WLS_DIMS (arg_info) >= 0 ? INFO_WLS_DIMS (arg_info)
                                                        : GetDim (ID_TYPE (arg_node)),
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
                            CreateCopyWithloop (NPART_CEXPR (arg_node), arg_info));

    ID_SSAASSIGN (assid) = assign;

    NPART_CEXPR (arg_node) = FreeTree (NPART_CEXPR (arg_node));
    NPART_CEXPR (arg_node) = assid;

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

/******************************************************************************
 *
 * function:
 *   node *CreateExprsPart(node *exprs,
 *                         int *partcount,
 *                         node *withid,
 *                         shpseg *shppos,
 *                         shpseg *shpmax,
 *                         int dim,
 *                         node *fundef,
 *                         simpletype btype)
 *
 * description:
 *   CreateArrayWithloop helper function
 *
 ******************************************************************************/
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

        if (NODE_TYPE (expr) == N_id) {
            res = MakeNPart (DupNode (withid),
                             MakeNGenerator (Shpseg2Array (shppos, dim),
                                             Shpseg2Array (UpperBound (shppos, dim), dim),
                                             F_le, F_lt, NULL, NULL),
                             MakeNCode (MakeBlock (MakeEmpty (), NULL), expr));
        } else {
            assid = MakeId (TmpVar (), NULL, ST_regular);

            vardec
              = MakeVardec (StringCopy (ID_NAME (assid)), MakeTypes1 (btype), vardec);

            ID_VARDEC (assid) = vardec;
            ID_AVIS (assid) = VARDEC_AVIS (vardec);

            AddVardecs (fundef, vardec);

            res = MakeNPart (DupNode (withid),
                             MakeNGenerator (Shpseg2Array (shppos, dim),
                                             Shpseg2Array (UpperBound (shppos, dim), dim),
                                             F_le, F_lt, NULL, NULL),
                             MakeNCode (MakeBlock (MakeAssignLet (StringCopy (
                                                                    ID_NAME (assid)),
                                                                  vardec, expr),
                                                   NULL),
                                        DupNode (assid)));
        }

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

/******************************************************************************
 *
 * function:
 *   node *CreateArrayWithloop(node *array, node *arg_info)
 *
 * description:
 *   creates a withloop with the same result as the given array.
 *
 * parameters:
 *   node *arg_node:   N_array
 *   node *arg_info:   N_INFO
 *
 ******************************************************************************/
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

/******************************************************************************
 *
 * function:
 *   node *Array2Withloop(node *arg_node, node *arg_info)
 *
 * description:
 *   creates an assignment for a withloop with the same result as the given
 *   array.
 *
 * parameters:
 *   node *arg_node:   N_node
 *   node *arg_info:   N_INFO
 *
 ******************************************************************************/
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

    NPART_CEXPR (arg_node) = FreeTree (NPART_CEXPR (arg_node));
    NPART_CEXPR (arg_node) = DupNode (assid);

    BLOCK_INSTR (NPART_CBLOCK (arg_node))
      = AppendAssign (BLOCK_INSTR (NPART_CBLOCK (arg_node)), assign);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *insertIndexDefinition(node *arg_node, node *arg_info)
 *
 * description:
 *   creates an assignment for the index vector's variables.
 *   this allows the withloopification of the special case where an
 *   array is to be filled with the index vector.
 *
 * parameters:
 *   node *arg_node:   N_node
 *   node *arg_info:   N_INFO
 *
 ******************************************************************************/
node *
insertIndexDefinition (node *arg_node, node *arg_info)
{
    node *assign;
    node *assid;
    node *vardec = NULL;

    node *array;

    DBUG_ENTER ("insertIndexVectorDefinition");

    assid = MakeId (TmpVar (), NULL, ST_regular);
    vardec = MakeVardec (StringCopy (ID_NAME (assid)),
                         DupOneTypes (ID_TYPE (NPART_CEXPR (arg_node))), vardec);

    ID_VARDEC (assid) = vardec;
    ID_AVIS (assid) = VARDEC_AVIS (vardec);

    AddVardecs (INFO_WLS_FUNDEF (arg_info), vardec);

    array = MakeArray (
      MakeExprsIdChain (DupAllIds (NWITHID_IDS (INFO_WLS_WITHID (arg_node)))));

    ARRAY_TYPE (array)
      = MakeTypes (T_int, 1,
                   MakeShpseg (MakeNums (CountExprs (ARRAY_AELEMS (array)), NULL)), NULL,
                   NULL);

    assign = MakeAssignLet (StringCopy (ID_NAME (assid)), vardec, array);

    ID_SSAASSIGN (assid) = assign;

    NPART_CEXPR (arg_node) = FreeTree (NPART_CEXPR (arg_node));
    NPART_CEXPR (arg_node) = DupNode (assid);

    BLOCK_INSTR (NPART_CBLOCK (arg_node))
      = AppendAssign (BLOCK_INSTR (NPART_CBLOCK (arg_node)), assign);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *withloopifyPart(node *arg_node, node *arg_info)
 *
 * description:
 *   if a part's inner nonscalar expression is an array or reshape command
 *   for an array, it is converted into a withloop.
 *   Otherwise a copy withloop is inserted.
 *
 * parameters:
 *   node *arg_node:   N_Npart
 *   node *arg_info:   N_INFO
 *
 ******************************************************************************/
node *
withloopifyPart (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("withloopifyPart");

    /* if the part's expr is the index vector we need to insert
       a definition of it into the codeblock */
    if (NPART_SSAASSIGN (arg_node) == NULL)
        insertIndexDefinition (arg_node, arg_info);

    /* if the part's LETEXPR is a withloop itself, we don't need to
       perform the withloopification */
    if (!((NODE_TYPE (NPART_LETEXPR (arg_node)) == N_Nwith)
          && (NWITH_PARTS (NPART_LETEXPR (arg_node)) > 0)
          && (isAssignInsideBlock (NPART_SSAASSIGN (arg_node),
                                   BLOCK_INSTR (NPART_CBLOCK (arg_node))))
          && (checkGeneratorDependencies (arg_node,
                                          NWITH_PART (NPART_LETEXPR (arg_node))))
          && (compatWLTypes (INFO_WLS_WITHOP (arg_info),
                             NWITH_WITHOP (NPART_LETEXPR (arg_node)))))) {
        /* if it is an array or a reshape command for an array,
           the array is converted to a withloop.
           Otherwise a copy withloop is inserted. */
        if ((NODE_TYPE (NPART_LETEXPR (arg_node)) == N_array)
            || ((NODE_TYPE (NPART_LETEXPR (arg_node)) == N_prf)
                && (PRF_PRF (NPART_LETEXPR (arg_node)) == F_reshape)
                && (NODE_TYPE (
                     EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (NPART_LETEXPR (arg_node))))))
                     == N_array))
            arg_node = Array2Withloop (arg_node, arg_info);
        else
            arg_node = InsertCopyWithloop (arg_node, arg_info);

        /* Now we need to apply the WLS to the newly created withloops */
        /* traverse all CODES */
        if (NPART_CODE (arg_node) != NULL) {
            NPART_CODE (arg_node) = Trav (NPART_CODE (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/****************************************************************************
 *
 * Generator Normalization functions
 *
 ****************************************************************************/

/******************************************************************************
 *
 * function:
 *   node *Nid2Narray(node *nid, node *arg_info)
 *
 * description:
 *   converts a generator's N_id node into a N_array node and makes correct
 *   assignments in the outer codeblock.
 *
 * parameters:
 *   node *arg_node:   N_id
 *   node *arg_info:   N_INFO
 *
 ******************************************************************************/
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

/******************************************************************************
 *
 * function:
 *   node *NormGenerator(node *gen, node *arg_info)
 *
 * description:
 *   ensures all of a generator's children are N_array nodes
 *
 * parameters:
 *   node *arg_node:   N_Ngenerator
 *   node *arg_info:   N_INFO
 *
 ******************************************************************************/
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

        newwithid = INFO_WLS_WITHID (arg_info);
    } else
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
 *   Creates a new block which is nothing which consists of the concatenation
 *   of the old outer codeblock (if any) and the old inner codeblock prepended
 *   with definitions of the two old WITHVECs
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

    LUT_t lut;
    ids *oldids, *newids;
    ids *newwithid_vec;

    DBUG_ENTER ("joinCodes");

    /* the new code consinst of... */
    /* the definitions of the two old withvecs */
    array = MakeArray (MakeExprsIdChain (DupAllIds (NWITHID_IDS (outerwithid))));

    ARRAY_TYPE (array)
      = MakeTypes (T_int, 1,
                   MakeShpseg (MakeNums (CountExprs (ARRAY_AELEMS (array)), NULL)), NULL,
                   NULL);

    newwithid_vec = DupAllIds (NWITHID_VEC (outerwithid));
    newcode = MakeAssignLet (IDS_NAME (newwithid_vec), IDS_VARDEC (newwithid_vec), array);

    newids = NWITHID_IDS (newwithid);
    oldids = NWITHID_IDS (outerwithid);
    while (oldids != NULL) {
        newids = IDS_NEXT (newids);
        oldids = IDS_NEXT (oldids);
    }

    array = MakeArray (MakeExprsIdChain (DupAllIds (newids)));

    ARRAY_TYPE (array)
      = MakeTypes (T_int, 1,
                   MakeShpseg (MakeNums (CountExprs (ARRAY_AELEMS (array)), NULL)), NULL,
                   NULL);

    newwithid_vec = DupAllIds (NWITHID_VEC (innerwithid));
    ASSIGN_NEXT (newcode)
      = MakeAssignLet (IDS_NAME (newwithid_vec), IDS_VARDEC (newwithid_vec), array);

    tmp_node = ASSIGN_NEXT (newcode);

    /* the old OUTER part's code (if any) */
    ASSIGN_NEXT (tmp_node) = DupTree (BLOCK_INSTR (NCODE_CBLOCK (outercode)));

    /* be sure to erase the last N_assign node from the copied block */
    tmp_node2 = newcode;
    while (ASSIGN_NEXT (ASSIGN_NEXT (tmp_node2)) != NULL)
        tmp_node2 = ASSIGN_NEXT (tmp_node2);

    ASSIGN_NEXT (tmp_node2) = FreeTree (ASSIGN_NEXT (tmp_node2));

    tmp_node = ASSIGN_NEXT (tmp_node);

    /* the old INNER part's code */
    /* Create a new LUT */
    lut = GenerateLUT ();

    /* Rename all occurences of old ids */
    oldids = NWITHID_IDS (outerwithid);
    newids = NWITHID_IDS (newwithid);

    while (oldids != NULL) {
        newids = IDS_NEXT (newids);
        oldids = IDS_NEXT (oldids);
    }
    oldids = NWITHID_IDS (innerwithid);

    while (oldids != NULL) {
        InsertIntoLUT_S (lut, IDS_NAME (oldids), IDS_NAME (newids));
        InsertIntoLUT_P (lut, IDS_VARDEC (oldids), IDS_VARDEC (newids));
        InsertIntoLUT_P (lut, IDS_AVIS (oldids), IDS_AVIS (newids));

        oldids = IDS_NEXT (oldids);
        newids = IDS_NEXT (newids);
    }

    tmp_node = newcode;

    newcode = DupTreeLUT (innercode, lut);
    if (NODE_TYPE (BLOCK_INSTR (NCODE_CBLOCK (newcode))) == N_empty)
        BLOCK_INSTR (NCODE_CBLOCK (newcode)) = tmp_node;
    else
        BLOCK_INSTR (NCODE_CBLOCK (newcode))
          = AppendAssign (tmp_node, BLOCK_INSTR (NCODE_CBLOCK (newcode)));

    RemoveLUT (lut);

    DBUG_RETURN (newcode);
}

/******************************************************************************
 *
 * function:
 *   node *scalarizePart(node *outerpart, node *arg_info)
 *
 * description:
 *   The heart of the WithloopScalarization.
 *   This function takes a part of MG-withloop that contains one withloop
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

    code_t *code_table, *temp;

    DBUG_ENTER ("scalarizePart");

    innerpart = NWITH_PART (NPART_LETEXPR (outerpart));

    while (innerpart != NULL) {
        /* Make a new generator */
        generator = joinGenerators (outerpart, innerpart);

        /* Make a new withid */
        withid
          = joinWithids (NPART_WITHID (outerpart), NPART_WITHID (innerpart), arg_info);

        /* Try to find a cached codeblock to make use of codesharing */
        code_table = INFO_WLS_CODETABLE (arg_info);
        while ((code_table != NULL)
               && ((code_table->outercode != NPART_CODE (outerpart))
                   || (code_table->innercode != NPART_CODE (innerpart))))
            code_table = code_table->next;

        if (code_table == NULL) {
            /* Make new code */
            code = joinCodes (NPART_CODE (outerpart), NPART_CODE (innerpart),
                              NPART_WITHID (outerpart), NPART_WITHID (innerpart), withid,
                              arg_info);

            NCODE_NEXT (code) = INFO_WLS_NEWCODES (arg_info);
            INFO_WLS_NEWCODES (arg_info) = code;

            temp = (code_t *)Malloc (sizeof (code_t));
            temp->outercode = NPART_CODE (outerpart);
            temp->innercode = NPART_CODE (innerpart);
            temp->newcode = code;
            temp->next = INFO_WLS_CODETABLE (arg_info);
            INFO_WLS_CODETABLE (arg_info) = temp;
        } else {
            code = code_table->newcode;
        }

        /* Now we can build a new part */
        newpart = MakeNPart (withid, generator, code);

        /* Enter in the part in the new chain of parts */
        NPART_NEXT (newpart) = INFO_WLS_NEWPARTS (arg_info);
        INFO_WLS_NEWPARTS (arg_info) = newpart;
        INFO_WLS_PARTS (arg_info)++;

        /* Present the Results */
        wls_expr++;

        innerpart = NPART_NEXT (innerpart);
    }

    DBUG_RETURN (outerpart);
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
    node *withop;
    code_t *codetable;

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
     *
     ***************************************************************************/

    tmpnode = arg_info;
    arg_info = MakeInfo ();
    INFO_WLS_FUNDEF (arg_info) = INFO_WLS_FUNDEF (tmpnode);
    INFO_WLS_BLOCK (arg_info) = outerblock;

    /* Check if WLS is possible vor all parts */
    INFO_WLS_PARTS (arg_info) = NWITH_PARTS (arg_node);

    /* First of all, the Withloop must be a MG-WL */
    INFO_WLS_POSSIBLE (arg_info) = (INFO_WLS_PARTS (arg_info) > 0);

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
         *  The applicabilty of the WLS is increased in the part by means of
         *    - transforming arrays to withloops
         *    - inserting copy withloops
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
         *  SCALARIZATION
         *
         *  All the outer part are melted with the contained single-generator
         *  withloops
         *
         ***************************************************************************/

        WLS_PHASE (arg_info) = wls_scalarize;

        INFO_WLS_WITHID (arg_info) = NULL;

        /* The new shape is the concatenation of both old shapes */

        if (NWITHOP_TYPE (INFO_WLS_WITHOP (arg_info)) == WO_genarray) {
            NWITH_SHAPE (arg_node)
              = ConcatVecs (NWITH_SHAPE (arg_node),
                            NWITH_TYPE (NPART_LETEXPR (NWITH_PART (arg_node)))
                                == WO_genarray
                              ? NWITH_SHAPE (NPART_LETEXPR (NWITH_PART (arg_node)))
                              : Shpseg2Array (ID_SHPSEG (NWITH_ARRAY (
                                                NPART_LETEXPR (NWITH_PART (arg_node)))),
                                              ID_DIM (NWITH_ARRAY (
                                                NPART_LETEXPR (NWITH_PART (arg_node))))));
        }

        withop = DupTree (NWITH_WITHOP (arg_node));

        /* traverse all PARTS  */

        INFO_WLS_NEWPARTS (arg_info) = NULL;
        INFO_WLS_NEWCODES (arg_info) = NULL;
        INFO_WLS_CODETABLE (arg_info) = NULL;
        INFO_WLS_PARTS (arg_info) = 0;

        if (NWITH_PART (arg_node) != NULL) {
            NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
        }

        /* clear the codetable */
        while (INFO_WLS_CODETABLE (arg_info) != NULL) {
            codetable = INFO_WLS_CODETABLE (arg_info)->next;
            Free (INFO_WLS_CODETABLE (arg_info));
            INFO_WLS_CODETABLE (arg_info) = codetable;
        }

        /* erase the old withloop */
        FreeTree (arg_node);

        /* create the new withloop */

        arg_node = MakeNWith (INFO_WLS_NEWPARTS (arg_info), INFO_WLS_NEWCODES (arg_info),
                              withop);

        NWITH_PARTS (arg_node) = INFO_WLS_PARTS (arg_info);

        arg_node = correctWL (arg_node);
    }

    arg_info = FreeNode (arg_info);
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

    case wls_scalarize:
        arg_node = scalarizePart (arg_node, arg_info);

        if (NPART_NEXT (arg_node) != NULL) {
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
        fundef = SSATransformOneFunction (fundef);

        act_tab = old_tab;

        arg_info = FreeTree (arg_info);
    }

    DBUG_RETURN (fundef);
}
