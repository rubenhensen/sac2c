/**
 * @defgroup wrci With-loop reuse candidate inference
 *
 * This module tries to identify With-loop reuse candidates.
 * It supports 4 kinds of reuse candidates, each of which can be
 * turned on/off separately. These are:
 * 1) RIP (Reuse with In-Place selection) [implemented in ReuseWithArrays.c]
 *    WL reuse candidates are all arrays A that are accessed only
 *    by means of selection at A[iv]i within the given WL.
 * 2) RWO (Reuse With Offset) [implemented in reusewithoffset.c]
 *    Looks for a WL that contains n-1 copy partitions and exactly one non-copy partition.
 *    If that is found, it makes allows th non-copy partition to have an access into the
 *    copy-from-source (ie potential reuse candidate) of the form A[iv +- offset] where
 *    offset is bigger than the generator width. This guarantees accesses into the copy
 *    partitions (at least if the accesses are legal :-)
 * 3) PRA (Polyhedral Reuse Analysis) [implemented in polyhedral_reuse_analysis.c] Refines
 *    RWO and uses polyhedral stuff to enable accesses of the form A[ linear_expr( iv)]
 *    provided we can show all these accesses fall into copy partitions or non-modified
 *    partitions...
 * 4) EMR (Extended Memory Reuse) [implemented here :-)] Here, we add arrays that are of
 *    the same shape and tha are defined in the current function but are not referenced
 *    in WL-body at all. This may sound counter-intuitive at first, but it enables memory
 *    reuse of the following kind:
 *
 *    Consider:
 *      int main( )
 *      {
 *         a = genarray( [15], 0);
 *
 *         print(a);
 *
 *         b = genarray( [15], 1);
 *         print(b);
 *
 *         return(0);
 *      }
 *
 *    Here, we would like to reuse the memory allocated for a when compting b.
 *
 *    NOTE: that EMR does not update the WL RC, but instead collects potnetial
 *          candidates within another strucutre, ERC (extended reuse candidates),
 *          on both WL-OPs and LoopFuns. The overall intention is to optimise
 *          reuse candidates present within WL RC and allow for memeory reuse
 *          in loops (do/for/while) by propogating reuse candidates from a higher
 *          scope. XXX this will be achieved via two new phases (which have yet
 *          to be implemented) - in the mean time, docs/projects/prealloc has
 *          some formalism on how this is to be implemented.
 *
 *    The ability to do such reuse is instrumental when trying to achieve a dual buffer
 *    swapping solution for a loop around something like     a = relax( a);
 *
 * Note here, that all these reuse candidate inferences potentially over-approximate the
 * set of candidates (eg arrays that are still refered to later); however, those cases are
 * being narrowed by subphases of the mem-phase, specifically SRCE and FRC, and by dynamic
 * reference count inspections at runtime :-)
 *
 * XXX:
 * For some reason WRCI it is run prior to Index Vector Elimination (IVE). I think the
 * reason might initially have been so that there is no need to deal with sel and idx_sel
 * but I am not sure about this :-( It seems that all the anylyses actually do support
 * idx_sel now and the whole thing might be run later but I am not sure; more intensive
 * testing would be required to do this! How far later is not entirely clear. Since it
 * builds on N_with and not Nwith2, it definitly needs to be run before WLT (or adapted to
 * cope with N_with2).
 *
 * However, 4) does NEED to run later as we must not add reuse candidates that might have
 * been optimised away in the meantime.... For this very purpose we offer a second entry
 * point which applies EMR only and potentially adds more reuse candidates.
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
#include "DupTree.h"

#define DBUG_PREFIX "WRCI"
#include "debug.h"

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
#include "polyhedral_reuse_analysis.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *lhs;
    node *rc;
    /* extended reuse */
    bool do_emr;
    node *emr_rc;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_RC(n) ((n)->rc)
#define INFO_DO_EMR(n) ((n)->do_emr)
#define INFO_EMR_RC(n) ((n)->emr_rc)

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
    INFO_DO_EMR (result) = FALSE;
    INFO_EMR_RC (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

static void
printRC (node *arg_node)
{
    DBUG_ENTER ();

    switch (NODE_TYPE (arg_node)) {
    case N_exprs:
        if (EXPRS_EXPR (arg_node) != NULL) {
            printRC (EXPRS_EXPR (arg_node));
        }
        if (EXPRS_NEXT (arg_node) != NULL) {
            printRC (EXPRS_NEXT (arg_node));
        }
        break;
    case N_id:
        if (ID_AVIS (arg_node) != NULL && NODE_TYPE (ID_AVIS (arg_node)) == N_avis) {
            printRC (ID_AVIS (arg_node));
        } else {
            fprintf (global.outfile, " #removed#,");
        }
        break;
    case N_avis:
        fprintf (global.outfile, " %s,", AVIS_NAME (arg_node));
        break;
    default:
        CTIerrorInternal("Node reference that should not be possible. Expected N_avis, "
                "found NODE_TYPE(%d) instead.\n", NODE_TYPE (arg_node));
        break;
    }

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn node *WRCIprintRCs( node *arg_node, info* arg_info)
 *
 * @brief a traversal prefun that annotates the output of TR_prt with information
 *        about RCs and ERCs associated to N_genarray and N_modarray.
 *
 * @param arg_node  syntax tree (specifically a N_genarray or N_modarray)
 * @param arg_info  unused
 *
 * @return unmodified arg_node.
 *
 *****************************************************************************/
node *
WRCIprintRCs (node *arg_node, info *arg_info)
{
    node *rc = NULL, *erc = NULL, *prc = NULL;
    DBUG_ENTER ();

    switch (NODE_TYPE (arg_node)) {
    case N_genarray:
        prc = GENARRAY_PRC (arg_node);
        /* Falls through. */
    case N_modarray:
        rc = WITHOP_RC (arg_node);
        erc = WITHOP_ERC (arg_node);
        break;
    /*
    case N_fundef:
        // TODO printing this is not nice...
        erc = FUNDEF_ERC (arg_node);
        break;
    */
    default:
        break;
    }

    if (rc != NULL) {
        INDENT;
        fprintf (global.outfile, " /* RCs: ");
        printRC (rc);
        fprintf (global.outfile, " */\n");
    }
    if (erc != NULL) {
        INDENT;
        fprintf (global.outfile, " /* ERCs: ");
        printRC (erc);
        fprintf (global.outfile, " */\n");
    }
    if (prc != NULL) {
        INDENT;
        fprintf (global.outfile, " /* PRCs: ");
        printRC (prc);
        fprintf (global.outfile, " */\n");
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/
node *
WRCIdoWithloopExtendedReuseCandidateInference (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    INFO_DO_EMR (arg_info) = TRUE;

    TRAVpush (TR_wrci);
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
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
ElimDupesOfAvis (node *avis, node *exprs)
{
    DBUG_ENTER ();

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
    DBUG_ENTER ();

    if (exprs != NULL) {
        EXPRS_NEXT (exprs)
          = ElimDupesOfAvis (ID_AVIS (EXPRS_EXPR (exprs)), EXPRS_NEXT (exprs));

        EXPRS_NEXT (exprs) = ElimDupes (EXPRS_NEXT (exprs));
    }

    DBUG_RETURN (exprs);
}

static node *
EMRid (node *id, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_PRINT ("filtering out %s", ID_NAME (id));
    INFO_EMR_RC (arg_info) = ElimDupesOfAvis (ID_AVIS (id), INFO_EMR_RC (arg_info));
    DBUG_RETURN (id);
}

static bool
ShapeMatch (ntype *t1, ntype *t2)
{
    ntype *aks1, *aks2;
    bool res;

    DBUG_ENTER ();

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

    DBUG_ENTER ();

    DBUG_PRINT ("  looking for matching RC -> %s", IDS_NAME (ids));

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
    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_PRINT ("\nchecking function %s ...", FUNDEF_NAME (arg_node));
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        if (INFO_EMR_RC (arg_info) != NULL) {
            INFO_EMR_RC (arg_info) = FREEdoFreeTree (INFO_EMR_RC (arg_info));
        }
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WRCIap( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
WRCIap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_DO_EMR (arg_info)) {
        /* check to see if we have found the recursive loopfun call */
        if (FUNDEF_ISLOOPFUN (INFO_FUNDEF (arg_info))
                && AP_FUNDEF (arg_node) == INFO_FUNDEF (arg_info)) {
            FUNDEF_ERC (INFO_FUNDEF (arg_info)) = TCappendExprs (FUNDEF_ERC (INFO_FUNDEF (arg_info)), DUPdoDupTree (INFO_EMR_RC (arg_info)));
            DBUG_PRINT ("extended reuse candidates for rec loopfun application:");
            DBUG_EXECUTE (if (FUNDEF_ERC (INFO_FUNDEF (arg_info)) != NULL) {
                PRTdoPrintFile (stderr, FUNDEF_ERC (INFO_FUNDEF (arg_info)));
            });
        }
    }

    AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WRCIarg( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
WRCIarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_DO_EMR (arg_info)) {
        DBUG_PRINT ("adding emr_rc %s arg ...", ARG_NAME (arg_node));
        INFO_EMR_RC (arg_info)
          = TBmakeExprs (TBmakeId (ARG_AVIS (arg_node)), INFO_EMR_RC (arg_info));
    }

    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WRCIids( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
WRCIids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_DO_EMR (arg_info)) {
        DBUG_PRINT ("adding emr_rc %s ids ...", IDS_NAME (arg_node));
        INFO_EMR_RC (arg_info)
          = TBmakeExprs (TBmakeId (IDS_AVIS (arg_node)), INFO_EMR_RC (arg_info));
    }

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

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
    DBUG_ENTER ();

    /*
     * Top-down traversal
     */
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

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
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    /* Only AFTER we traversed the body we add the defined vars in the EMR_RCs */
    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);

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
    node *emr_chain;
    DBUG_ENTER ();

    /*
     * First, find all conventional reuse candidates.
     * These are all arrays A that are accessed only by means of selection at
     * A[iv]
     */
    if (global.optimize.dorip && !INFO_DO_EMR (arg_info)) {
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
    if (global.optimize.dorwo && !INFO_DO_EMR (arg_info)) {
        DBUG_PRINT ("Looking for more complex reuse candidates...");
        INFO_RC (arg_info)
          = TCappendExprs (INFO_RC (arg_info),
                           RWOdoOffsetAwareReuseCandidateInference (arg_node));
        DBUG_PRINT ("candidates after reuse-with-offset: ");
        DBUG_EXECUTE (if (INFO_RC (arg_info) != NULL) {
            PRTdoPrintFile (stderr, INFO_RC (arg_info));
        });
    }

    if (global.optimize.dopra && !INFO_DO_EMR (arg_info)) {
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

    if (INFO_DO_EMR (arg_info)) {
        DBUG_PRINT ("potential EMR candidates:");
        DBUG_EXECUTE (if (INFO_EMR_RC (arg_info) != NULL) {
            PRTdoPrintFile (stderr, INFO_EMR_RC (arg_info));
        });

        /*
         * First filter out those vars that are being used in the WL body.
         * However, we have to make sure that we preserve the current INFO_EMR_RC
         * chain when leaving the N_with again! Otherwise, we may loose RC
         * candidates. Consider:
         * int[1000000] f( int[1000000] a)
         * {
         *   res = with{
         *          (. <= [i] <=.) : a[[i]] + a[[999999-i]];
         *         } : modarray(a);
         *   ArrayIO::print( res);
         *   res2 = with{
         *          (. <= [i] <=.) : res[[i]] + res[[999999-i]];
         *         } : modarray(res);
         *   return (res2);
         * }
         * The argument "a" gets filtered out in the first WL but should be
         * a candidate for the second!
         *
         * To achieve this, we stack the chain here and restore it just before leaving
         * the N_with!
         */
        emr_chain = DUPdoDupTree (INFO_EMR_RC (arg_info));
        anontrav_t emrtrav[2] = {{N_id, &EMRid}, {(nodetype)0, NULL}};
        TRAVpushAnonymous (emrtrav, &TRAVsons);
        arg_node = TRAVdo (arg_node, arg_info);
        TRAVpop ();

        DBUG_PRINT ("potential EMR candidates after pruning those used in WL body:");
        DBUG_EXECUTE (if (INFO_EMR_RC (arg_info) != NULL) {
            PRTdoPrintFile (stderr, INFO_EMR_RC (arg_info));
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

    if (INFO_DO_EMR (arg_info)) {
        /*
         * Here, we have to resurrect the emr-candidates as we had them before
         * processing this with loop:
         */
        FREEdoFreeTree (INFO_EMR_RC (arg_info));
        INFO_EMR_RC (arg_info) = emr_chain;
    }

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
    DBUG_ENTER ();

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
    DBUG_ENTER ();

    /*
     * Annotate reuse candidates.
     */
    if (!INFO_DO_EMR (arg_info)) {
        GENARRAY_RC (arg_node) = MatchingRCs (INFO_RC (arg_info), INFO_LHS (arg_info), NULL);
        DBUG_PRINT ("Genarray RCs: ");
        DBUG_EXECUTE (if (GENARRAY_RC (arg_node) != NULL) {
            PRTdoPrintFile (stderr, GENARRAY_RC (arg_node));
        });
    }

    /*
     * Annotate extended reuse candidates.
     */
    if (INFO_DO_EMR (arg_info)) {
        GENARRAY_ERC (arg_node)
          = MatchingRCs (INFO_EMR_RC (arg_info), INFO_LHS (arg_info), NULL);
        DBUG_PRINT ("Genarray ERCs: ");
        DBUG_EXECUTE (if (GENARRAY_ERC (arg_node) != NULL) {
            PRTdoPrintFile (stderr, GENARRAY_ERC (arg_node));
        });
    }

    if (global.optimize.dopr && !INFO_DO_EMR (arg_info)) {
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
    DBUG_ENTER ();

    /*
     * Annotate conventional reuse candidates.
     */
    if (!INFO_DO_EMR (arg_info)) {
        MODARRAY_RC (arg_node)
          = MatchingRCs (INFO_RC (arg_info), INFO_LHS (arg_info), MODARRAY_ARRAY (arg_node));
        DBUG_PRINT ("Modarray RCs: ");
        DBUG_EXECUTE (if (MODARRAY_RC (arg_node) != NULL) {
            PRTdoPrintFile (stderr, MODARRAY_RC (arg_node));
        });
    }

    /*
     * Annotate extended reuse candidates.
     */
    if (INFO_DO_EMR (arg_info)) {
        MODARRAY_ERC (arg_node)
          = MatchingRCs (INFO_EMR_RC (arg_info), INFO_LHS (arg_info), MODARRAY_ARRAY (arg_node));
        DBUG_PRINT ("Modarray ERCs: ");
        DBUG_EXECUTE (if (MODARRAY_ERC (arg_node) != NULL) {
            PRTdoPrintFile (stderr, MODARRAY_ERC (arg_node));
        });
    }

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
    DBUG_ENTER ();

    if (FOLD_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        FOLD_NEXT (arg_node) = TRAVdo (FOLD_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/* @} */

#undef DBUG_PREFIX
