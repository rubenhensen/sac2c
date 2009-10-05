/*
 * $Id$
 */

/**
 * @defgroup wrci With-loop reuse candidate inference
 *
 * WL reuse candidates are all arrays A that are accessed only
 * by means of selection at A[iv]
 *
 * @ingroup mm
 *
 * @{
 */

/**
 * @file wrci.c
 *
 * Prefix: WRCI
 */
#include "wrci.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"
#include "ReuseWithArrays.h"
#include "reusewithoffset.h"
#include "compare_tree.h"

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
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_RC (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *WRCIdoInferWithloopReuseCandidates( node *syntax_tree)
 *
 * @brief
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/
node *
WRCIdoWithloopReuseCandidateInference (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ("WRCIdoWithloopReuseCandidateInference");

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
ElimDupesOfAvis (node *avis, node *exprs)
{
    DBUG_ENTER ("ElimDupesOfAvis");

    if (exprs != NULL) {
        if (EXPRS_NEXT (exprs) != NULL) {
            EXPRS_NEXT (exprs) = ElimDupesOfAvis (avis, EXPRS_NEXT (exprs));
        }

        if (ID_AVIS (EXPRS_EXPR (exprs)) == avis) {
            exprs = FREEdoFreeNode (exprs);
        }
    }

    DBUG_RETURN (exprs);
}

static node *
ElimDupes (node *exprs)
{
    DBUG_ENTER ("ElimDupes");

    if (exprs != NULL) {
        EXPRS_NEXT (exprs)
          = ElimDupesOfAvis (ID_AVIS (EXPRS_EXPR (exprs)), EXPRS_NEXT (exprs));

        EXPRS_NEXT (exprs) = ElimDupes (EXPRS_NEXT (exprs));
    }

    DBUG_RETURN (exprs);
}

static bool
ShapeMatch (ntype *t1, ntype *t2)
{
    ntype *aks1, *aks2;
    bool res;

    DBUG_ENTER ("ShapeMatch");

    aks1 = TYeliminateAKV (t1);
    aks2 = TYeliminateAKV (t2);

    res = TYisAKS (aks1) && TYeqTypes (aks1, aks2);

    aks1 = TYfreeType (aks1);
    aks2 = TYfreeType (aks2);

    DBUG_RETURN (res);
}

static node *
MatchingRCs (node *rcs, node *ids, node *modarray)
{
    node *match = NULL;

    DBUG_ENTER ("MatchingRCs");

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

    DBUG_ENTER ("MatchingRCs");

    if (rcs != NULL) {
        match = MatchingPRCs (EXPRS_NEXT (rcs), ids);

        if (TUravelsHaveSameStructure (ID_NTYPE (EXPRS_EXPR (rcs)), IDS_NTYPE (ids))
            && TUeqElementSize (ID_NTYPE (EXPRS_EXPR (rcs)), IDS_NTYPE (ids))) {
            match = TBmakeExprs (TBmakeId (ID_AVIS (EXPRS_EXPR (rcs))), match);
        }
    }

    DBUG_RETURN (match);
}

/******************************************************************************
 *
 * With-loop reuse candidate inference traversal (wrci_tab)
 *
 * prefix: WRCI
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *WRCIfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
WRCIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WRCIfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WRCIassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
WRCIassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WRCIassign");

    /*
     * Top-down traversal
     */
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WRCIlet( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
WRCIlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WRCIlet");

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WRCIwith( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
WRCIwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WRCIwith");

    /*
     * First, find all conventional reuse candidates.
     * These are all arrays A that are accessed only by means of selection at
     * A[iv]
     */
    INFO_RC (arg_info) = REUSEdoGetReuseArrays (arg_node, INFO_FUNDEF (arg_info));

    /*
     * Find more complex reuse candidates
     */
    INFO_RC (arg_info)
      = TCappendExprs (INFO_RC (arg_info),
                       RWOdoOffsetAwareReuseCandidateInference (arg_node));

    /*
     * Eliminate duplicates of reuse candidates
     */
    INFO_RC (arg_info) = ElimDupes (INFO_RC (arg_info));

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

/** <!--********************************************************************-->
 *
 * @fn node *WRCIgenerator( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
WRCIgenerator (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WRCIgenerator");

    if (GENERATOR_GENWIDTH (arg_node) != NULL) {
        GENERATOR_GENWIDTH (arg_node) = FREEdoFreeTree (GENERATOR_GENWIDTH (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WRCIgenarray( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
WRCIgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WRCIgenarray");

    /*
     * Annotate reuse candidates.
     */
    GENARRAY_RC (arg_node) = MatchingRCs (INFO_RC (arg_info), INFO_LHS (arg_info), NULL);

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

/** <!--********************************************************************-->
 *
 * @fn node *WRCImodarray( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
WRCImodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WRCImodarray");

    /*
     * Annotate conventional reuse candidates.
     */
    MODARRAY_RC (arg_node)
      = MatchingRCs (INFO_RC (arg_info), INFO_LHS (arg_info), MODARRAY_ARRAY (arg_node));

    if (MODARRAY_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WRCIfold( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
WRCIfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WRCIfold");

    if (FOLD_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        FOLD_NEXT (arg_node) = TRAVdo (FOLD_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/* @} */
