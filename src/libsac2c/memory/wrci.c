/**
 * @file
 * @defgroup wrci With-loop reuse candidate inference
 *
 * This module tries to identify With-loop reuse candidates.  It supports 4
 * kinds of reuse candidates, each of which can be turned on/off separately.
 * These are:
 * 1. RIP (Reuse with In-Place selection) [implemented in ReuseWithArrays.c]
 *    WL reuse candidates are all arrays A that are accessed only
 *    by means of selection at A[iv]i within the given WL.
 * 2. RWO (Reuse With Offset) [implemented in reusewithoffset.c]
 *    Looks for a WL that contains n-1 copy partitions and exactly one non-copy
 *    partition.  If that is found, it makes allows th non-copy partition to
 *    have an access into the copy-from-source (ie potential reuse candidate) of
 *    the form A[iv +- offset] where offset is bigger than the generator width.
 *    This guarantees accesses into the copy partitions (at least if the
 *    accesses are legal :-)
 * 3. PRA (Polyhedral Reuse Analysis) [implemented in polyhedral_reuse_analysis.c]
 *    Refines RWO and uses polyhedral stuff to enable accesses of the form A[
 *    linear_expr( iv)] provided we can show all these accesses fall into copy
 *    partitions or non-modified partitions...
 * 4. EMR (Extended Memory Reuse) [Implemented in emr_inference.c]
 *
 * @note
 * All these reuse candidate inferences potentially over-approximate the set of
 * candidates (e.g. arrays that are still refered to later); however, those cases
 * are being narrowed by subphases of the mem-phase, specifically SRCE and FRC,
 * and by dynamic reference count inspections at runtime.
 *
 * @note
 * For some reason WRCI it is run prior to Index Vector Elimination (IVE). I
 * think the reason might initially have been so that there is no need to deal
 * with sel and idx_sel but I am not sure about this. It seems that all the
 * anylyses actually do support idx_sel now and the whole thing might be run
 * later but I am not sure; more intensive testing would be required to do this!
 * How far later is not entirely clear. Since it builds on N_with and not
 * Nwith2, it definitly needs to be run before WLT (or adapted to cope with
 * N_with2).
 *
 * @ingroup mm
 *
 * @{
 */
#include "wrci.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#define DBUG_PREFIX "WRCI"
#include "debug.h"

#include "print.h"
#include "str.h"
#include "memory.h"
#include "emr_utils.h"
#include "free.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"
#include "ReuseWithArrays.h"
#include "reusewithoffset.h"
#include "compare_tree.h"
#include "polyhedral_reuse_analysis.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *lhs;
    node *rc;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_RC(n) ((n)->rc)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_RC (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/**
 * @brief Function is used by the Pre/Post traversal function injection. It
 *        prints out the RC/PRC for WITHOPs (except N_fold).
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
WRCIprintPreFun (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (NODE_TYPE (arg_node)) {
    case N_genarray:
        if (GENARRAY_RC (arg_node)) {
            INDENT;
            fprintf (global.outfile, "/* RC (");
            GENARRAY_RC (arg_node) = PRTexprs (GENARRAY_RC (arg_node), arg_info);
            fprintf (global.outfile, ") */\n");
        }
        if (GENARRAY_PRC (arg_node)) {
            INDENT;
            fprintf (global.outfile, "/* PRC (");
            GENARRAY_PRC (arg_node) = PRTexprs (GENARRAY_PRC (arg_node), arg_info);
            fprintf (global.outfile, ") */\n");
        }
        break;
    case N_modarray:
        if (MODARRAY_RC (arg_node)) {
            INDENT;
            fprintf (global.outfile, "/* RC (");
            MODARRAY_RC (arg_node) = PRTexprs (MODARRAY_RC (arg_node), arg_info);
            fprintf (global.outfile, ") */\n");
        }
        break;
    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief Entry function into WRCI traversal
 *
 * @param syntax_tree
 * @return modified syntax_tree.
 */
node *
WRCIdoWithloopReuseCandidateInference (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    TRAVpush (TR_wrci);
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/

static node *
MatchingRCs (node *rcs, node *ids, node *modarray)
{
    node *match = NULL;

    DBUG_ENTER ();

    DBUG_PRINT ("looking for matching RC -> %s", IDS_NAME (ids));

    if (rcs != NULL) {
        match = MatchingRCs (EXPRS_NEXT (rcs), ids, modarray);

        if (((ShapeMatch (ID_NTYPE (EXPRS_EXPR (rcs)), IDS_NTYPE (ids))
              || TCshapeVarsMatch (ID_AVIS (EXPRS_EXPR (rcs)), IDS_AVIS (ids)))
             && TUeqElementSize (ID_NTYPE (EXPRS_EXPR (rcs)), IDS_NTYPE (ids)))
            || ((modarray != NULL)
                && (ID_AVIS (EXPRS_EXPR (rcs)) == ID_AVIS (modarray)))) {
            match = TBmakeExprs (TBmakeId (ID_AVIS (EXPRS_EXPR (rcs))), match);
        }
    }

    DBUG_RETURN (match);
}

static node *
MatchingPRCs (node *rcs, node *ids)
{
    node *match = NULL;

    DBUG_ENTER ();

    if (rcs != NULL) {
        match = MatchingPRCs (EXPRS_NEXT (rcs), ids);

        if (TUravelsHaveSameStructure (ID_NTYPE (EXPRS_EXPR (rcs)), IDS_NTYPE (ids))
            && TUeqElementSize (ID_NTYPE (EXPRS_EXPR (rcs)), IDS_NTYPE (ids))) {
            match = TBmakeExprs (TBmakeId (ID_AVIS (EXPRS_EXPR (rcs))), match);
        }
    }

    DBUG_RETURN (match);
}

/**
 * @brief traverse N_fundef body
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
WRCIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_PRINT ("\nchecking function %s ...", FUNDEF_NAME (arg_node));
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief traverse N_assign top-down
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
WRCIassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * Top-down traversal
     */
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief traverse N_let expressions and store the LHS of the N_let
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
WRCIlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief traverse N_with and apply reuse candidate inferences. Reuse
 *        candidates are stored in INFO structure.
 *
 * @see ReuseWithArrays.c
 * @see reusewithoffset.c
 * @see polyhedral_reuse_analysis.c
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
WRCIwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * First, find all conventional reuse candidates.
     * These are all arrays A that are accessed only by means of selection at
     * A[iv]
     */
    if (global.optimize.dorip) {
        DBUG_PRINT ("Looking for A[iv] only use...");
        INFO_RC (arg_info) = REUSEdoGetReuseArrays (arg_node, INFO_FUNDEF (arg_info));
        DBUG_PRINT ("candidates after conventional reuse: ");
        DBUG_EXECUTE (if (INFO_RC (arg_info) != NULL) {
            PRTdoPrintFile (stderr, INFO_RC (arg_info));
        });
    }

    /*
     * Find more complex reuse candidates
     */
    if (global.optimize.dorwo) {
        DBUG_PRINT ("Looking for more complex reuse candidates...");
        INFO_RC (arg_info)
          = TCappendExprs (INFO_RC (arg_info),
                           RWOdoOffsetAwareReuseCandidateInference (arg_node));
        DBUG_PRINT ("candidates after reuse-with-offset: ");
        DBUG_EXECUTE (if (INFO_RC (arg_info) != NULL) {
            PRTdoPrintFile (stderr, INFO_RC (arg_info));
        });
    }

    if (global.optimize.dopra) {
        /*
         * Find more complex reuse candidates
         */
        DBUG_PRINT ("Looking for polyhedra-analysis based reuse candidates...");
        INFO_RC (arg_info)
          = TCappendExprs (INFO_RC (arg_info),
                           PRAdoPolyhedralReuseAnalysis (arg_node,
                                                         INFO_FUNDEF (arg_info)));
        DBUG_PRINT ("candidates after polyhedral reuse analysis: ");
        DBUG_EXECUTE (if (INFO_RC (arg_info) != NULL) {
            PRTdoPrintFile (stderr, INFO_RC (arg_info));
        });
    }

    /*
     * Eliminate duplicates of reuse candidates
     */
    INFO_RC (arg_info) = ElimDupes (INFO_RC (arg_info));

    DBUG_PRINT ("final RCs after ElimDupes: ");
    DBUG_EXECUTE (
      if (INFO_RC (arg_info) != NULL) { PRTdoPrintFile (stderr, INFO_RC (arg_info)); });
    /*
     * Annotate RCs and find further reuse candidates if appropriate
     */
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    /*
     * Remove RC list
     */
    if (INFO_RC (arg_info) != NULL) {
        INFO_RC (arg_info) = FREEdoFreeTree (INFO_RC (arg_info));
    }

    /*
     * Be sure to remove all GENERATOR_GENWIDTH
     */
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    /*
     * Continue RC inference for nested WLs
     */
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief traverse N_generator of with-loop and remove generator width
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
WRCIgenerator (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (GENERATOR_GENWIDTH (arg_node) != NULL) {
        GENERATOR_GENWIDTH (arg_node) = FREEdoFreeTree (GENERATOR_GENWIDTH (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief traverse N_genarray and add infered reuse candidates to node
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
WRCIgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * Annotate reuse candidates.
     */
    GENARRAY_RC (arg_node) = MatchingRCs (INFO_RC (arg_info), INFO_LHS (arg_info), NULL);
    DBUG_PRINT ("Genarray RCs: ");
    DBUG_EXECUTE (if (GENARRAY_RC (arg_node) != NULL) {
        PRTdoPrintFile (stderr, GENARRAY_RC (arg_node));
    });

    if (global.optimize.dopr) {
        /*
         * Annotate partial reuse candidates
         */
        GENARRAY_PRC (arg_node) = MatchingPRCs (INFO_RC (arg_info), INFO_LHS (arg_info));
    }

    if (GENARRAY_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief traverse N_modarray and add infered reuse candidates to node
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
WRCImodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * Annotate conventional reuse candidates.
     */
    MODARRAY_RC (arg_node)
      = MatchingRCs (INFO_RC (arg_info), INFO_LHS (arg_info), MODARRAY_ARRAY (arg_node));
    DBUG_PRINT ("Modarray RCs: ");
    DBUG_EXECUTE (if (MODARRAY_RC (arg_node) != NULL) {
        PRTdoPrintFile (stderr, MODARRAY_RC (arg_node));
    });

    if (MODARRAY_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief traverse N_fold
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
WRCIfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FOLD_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        FOLD_NEXT (arg_node) = TRAVdo (FOLD_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** @} */

#undef DBUG_PREFIX
