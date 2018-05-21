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
    /* loop memory propogation */
    bool do_update;
    node *tmp_rcs;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_RC(n) ((n)->rc)
#define INFO_DO_EMR(n) ((n)->do_emr)
#define INFO_EMR_RC(n) ((n)->emr_rc)
#define INFO_DO_UPDATE(n) ((n)->do_update)
#define INFO_TMP_RCS(n) ((n)->tmp_rcs)

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
    INFO_DO_UPDATE (result) = FALSE;
    INFO_TMP_RCS (result) = NULL;

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

    if (global.optimize.doelaaf) {
        arg_info = MakeInfo ();

        // FIXME probably should push this out into its own file
        TRAVpush (TR_elaaf);
        syntax_tree = TRAVdo (syntax_tree, arg_info);
        TRAVpop ();

        arg_info = FreeInfo (arg_info);
    }

    if (global.optimize.doelmp) {
        arg_info = MakeInfo ();

        // FIXME probably should push this out into its own file
        TRAVpush (TR_elmp);
        syntax_tree = TRAVdo (syntax_tree, arg_info);
        TRAVpop ();

        INFO_DO_UPDATE (arg_info) = TRUE;

        TRAVpush (TR_elmp);
        syntax_tree = TRAVdo (syntax_tree, arg_info);
        TRAVpop ();

        arg_info = FreeInfo (arg_info);
    }

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

/**
 * @brief
 *
 * @param exprs N_exprs chain
 * @param id N_id
 *
 * @return true or false
 */
static bool
doAvisMatch (node * exprs, node * id)
{
    if (exprs == NULL) {
        return FALSE;
    } else {
        if (ID_AVIS (id) == ID_AVIS (EXPRS_EXPR (exprs))) {
            return TRUE;
        } else {
            return doAvisMatch (EXPRS_NEXT (exprs), id);
        }
    }
}

/**
 * @brief
 *
 * @param fexprs N_exprs chain of N_id that are to be found
 * @param exprs N_exprs chain of N_id that is to be filtered
 *
 * @return node N_exprs chain after filtering
 */
static node *
filterDuplicateArgs (node * fexprs, node ** exprs)
{
    node * filtered;
    DBUG_ENTER ();

    DBUG_PRINT ("filtering out duplicate N_avis");

    filtered = TCfilterExprsArg (doAvisMatch, fexprs, exprs);

    /* we delete all duplicates from col */
    if (filtered != NULL) {
        DBUG_PRINT ("  found and removed the following duplicates:");
        DBUG_EXECUTE (PRTdoPrintFile (stderr, filtered));
        filtered = FREEdoFreeTree (filtered);
    }

    DBUG_RETURN (*exprs);
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
 * @fn node *WRCIlet( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
WRCIprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_DO_EMR (arg_info)) {
        DBUG_PRINT ("checking N_prf referencing an emr...");

        switch (PRF_PRF (arg_node)) {
            case F_idx_modarray_AxSxS:
            case F_idx_modarray_AxSxA:
                /*
                 * We intentionally drop all referenced ERCs because from observation, the
                 * result is typically stored in the referenced array (reused). As such, we
                 * force these arrays to be used as ERCs, then we force an extra allocation
                 * *and* copy operation.
                 *
                 * XXX this is somewhat harsh as it removes a large number of possible ERCs
                 *     without regard for whether or not the actual memory is being reused
                 *     in place. Within OPT this can't be determined, we would need to move
                 *     this filtering to MEM phases.
                 */
                INFO_EMR_RC (arg_info) = filterDuplicateArgs (PRF_ARGS (arg_node), &INFO_EMR_RC (arg_info));
                DBUG_PRINT ("EMR RCs left after filtering out N_prf args");
                DBUG_EXECUTE (if (INFO_EMR_RC (arg_info) != NULL) {
                    PRTdoPrintFile (stderr, INFO_EMR_RC (arg_info));
                });
                break;

            default:
                break;
        }
    }

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
        if (INFO_EMR_RC (arg_info) != NULL)
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

/* XXX
 * EMR Loop Application Arg Filtering
 * XXX
 */

node *
ELAAFgenarray (node * arg_node, info * arg_info)
{
    DBUG_ENTER ();

    if (GENARRAY_ERC (arg_node) != NULL) {
        GENARRAY_ERC (arg_node) = filterDuplicateArgs (INFO_EMR_RC (arg_info), &GENARRAY_ERC (arg_node));
        DBUG_PRINT ("filtering genarray ERC:");
        DBUG_EXECUTE (if (GENARRAY_ERC (arg_node) != NULL) {
            PRTdoPrintFile (stderr, GENARRAY_ERC (arg_node));
        });
    }

    GENARRAY_NEXT (arg_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
ELAAFmodarray (node * arg_node, info * arg_info)
{
    DBUG_ENTER ();

    if (MODARRAY_ERC (arg_node) != NULL) {
        MODARRAY_ERC (arg_node) = filterDuplicateArgs (INFO_EMR_RC (arg_info), &MODARRAY_ERC (arg_node));
        DBUG_PRINT ("filtering modarray ERC:");
        DBUG_EXECUTE (if (MODARRAY_ERC (arg_node) != NULL) {
            PRTdoPrintFile (stderr, MODARRAY_ERC (arg_node));
        });

        INFO_EMR_RC (arg_info) = TCappendExprs (INFO_EMR_RC (arg_info), TBmakeExprs(DUPdoDupNode (MODARRAY_ARRAY (arg_node)), NULL));
    }

    MODARRAY_NEXT (arg_node) = TRAVopt (MODARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
ELAAFap (node * arg_node, info * arg_info)
{
    DBUG_ENTER ();

    /* check to see if we have found the recursive loopfun call */
    if (FUNDEF_ISLOOPFUN (INFO_FUNDEF (arg_info))
            && AP_FUNDEF (arg_node) == INFO_FUNDEF (arg_info)) {
        DBUG_PRINT ("at loop rec N_ap, copying args:");
        DBUG_EXECUTE (PRTdoPrintFile (stderr, AP_ARGS (arg_node)));
        INFO_EMR_RC (arg_info) = TCappendExprs (INFO_EMR_RC (arg_info), DUPdoDupTree (AP_ARGS (arg_node)));

        DBUG_PRINT ("filtering fun ERC:");
        FUNDEF_ERC (INFO_FUNDEF (arg_info)) = filterDuplicateArgs (AP_ARGS (arg_node), &FUNDEF_ERC (INFO_FUNDEF (arg_info)));
        DBUG_EXECUTE (if (FUNDEF_ERC (INFO_FUNDEF (arg_info)) != NULL) {
            PRTdoPrintFile (stderr, FUNDEF_ERC (INFO_FUNDEF (arg_info)));
        });
    }

    DBUG_RETURN (arg_node);
}

node *
ELAAFassign (node * arg_node, info * arg_info)
{
    DBUG_ENTER ();

    /*
     * Bottom-up
     */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
ELAAFfundef (node * arg_node, info * arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("inspecting N_fundef %s ...", FUNDEF_NAME (arg_node));

    /*
     * Top-down N_fundef chain
     */

    /* store fundef */
    INFO_FUNDEF (arg_info) = arg_node;

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    if (INFO_EMR_RC (arg_info) != NULL)
        INFO_EMR_RC (arg_info) = FREEdoFreeTree (INFO_EMR_RC (arg_info));

    /* reset */
    INFO_FUNDEF (arg_info) = NULL;

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/* XXX
 * EMR Loop Memory Propogation
 * XXX
 */

/* runs in two modes - INFO_DO_UPDATE is:
 *   FALSE : fundef top-down traversal looking for loop funs, and
 *           updating fundef args and rec loop ap. We unset fundef_erc
 *           as these are no longer needed.
 *   TRUE  : fundef top-down looking at fun_body at initil loop app -
 *           if the fundef has an updated signiture, update the app
 *           args
 */

/**
 * @brief
 *
 * @param avis
 * @param exprs
 *
 * @return
 */
static node *
isSameShapeAvis (node * avis, node * exprs)
{
    node * ret;
    DBUG_ENTER ();

    if (exprs == NULL) {
        ret = NULL;
    } else {
        if ((ShapeMatch (AVIS_TYPE (avis), ID_NTYPE (EXPRS_EXPR (exprs)))
              || TCshapeVarsMatch (avis, ID_AVIS (EXPRS_EXPR (exprs))))
             && TUeqElementSize (AVIS_TYPE (avis), ID_NTYPE (EXPRS_EXPR (exprs))))
            ret = EXPRS_EXPR (exprs);
        else
            ret = isSameShapeAvis (avis, EXPRS_NEXT (exprs));
    }

    DBUG_RETURN (ret);
}

/**
 * @brief
 *
 * @param id
 * @param exprs
 *
 * @return
 */
static node *
isSameShape (node * id, node * exprs)
{
    node * ret;
    DBUG_ENTER ();

    ret = isSameShapeAvis (ID_AVIS (id), exprs);

    DBUG_RETURN (ret);
}

/**
 * @brief
 *
 * @param exprs  exprs chain of N_id nodes
 * @param pot
 *
 * @return
 */
static node *
findMatchingArgs (node * exprs, node * pot)
{
    int size, i;
    node * res = NULL,
         * tmp,
         * find;
    DBUG_ENTER ();

    DBUG_PRINT ("finding suitable args inplace of tmp vars:");
    DBUG_EXECUTE (if (pot != NULL) {
            PRTdoPrintFile (stderr, pot); });

    if (pot != NULL) {
        /* for each var in tmp RC, find a suitable var in ERC and replace it */
        size = TCcountExprs (exprs);
        for (i = 0; i < size; i++)
        {
            tmp = TCgetNthExprsExpr (i, exprs);
            find = isSameShape (tmp, pot);
            if (find == NULL)
            {
                DBUG_UNREACHABLE ("  unable to find a valid extended reuse candidate to replace tmp arg in recurisve loopfun!");
            } else {
                DBUG_PRINT ("  found match for tmp arg %s => %s", ID_NAME (tmp), ID_NAME (find));

                /* adding res to args of ap_fundef */
                if (res == NULL)
                    res = TBmakeExprs (find, NULL);
                else
                    res = TCappendExprs (res, TBmakeExprs (find, NULL));
            }
        }
    }

    DBUG_RETURN (res);
}

node *
ELMPlet (node * arg_node, info * arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);

    DBUG_PRINT ("LHS vars are:");
    DBUG_EXECUTE (PRTdoPrintFile (stderr, INFO_LHS (arg_info)));

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
ELMPwith (node * arg_node, info * arg_info)
{
    DBUG_ENTER ();

    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
ELMPgenarray (node * arg_node, info * arg_info)
{
    node * new_avis;
    DBUG_ENTER ();

    if (!INFO_DO_UPDATE (arg_info)
            && GENARRAY_RC (arg_node) == NULL
            && GENARRAY_ERC (arg_node) == NULL
            && TYisAKS (IDS_NTYPE (INFO_LHS (arg_info)))) {
        DBUG_PRINT (" genarray in loopfun has no RCs or ERCs, generating tmp one!");

        // the new avis must have the same type/shape as genarray shape
        new_avis = TBmakeAvis ( TRAVtmpVarName ("emr_tmp"),
                TYcopyType (IDS_NTYPE (INFO_LHS (arg_info)))
                );

        // extend the fundef arguments to include the new var
        FUNDEF_ARGS (INFO_FUNDEF (arg_info)) = TCappendArgs (FUNDEF_ARGS (INFO_FUNDEF (arg_info)), TBmakeArg (new_avis, NULL));

        // add the new var to RC
        GENARRAY_RC (arg_node) = TBmakeExprs( TBmakeId (new_avis), NULL);
        INFO_TMP_RCS (arg_info) = TCappendExprs (INFO_TMP_RCS (arg_info), DUPdoDupNode (GENARRAY_RC (arg_node)));
    }

    if (GENARRAY_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ELMPmodarray (node * arg_node, info * arg_info)
{
    node * new_avis;
    DBUG_ENTER ();

    if (!INFO_DO_UPDATE (arg_info)
            && MODARRAY_RC (arg_node) == NULL
            && MODARRAY_ERC (arg_node) == NULL
            && TYisAKS (IDS_NTYPE (INFO_LHS (arg_info)))) {
        DBUG_PRINT (" modarray in loopfun has no RCs or ERCs, generating tmp one!");

        // the new avis must have the same type/shape as genarray shape
        new_avis = TBmakeAvis ( TRAVtmpVarName ("emr_tmp"),
                TYcopyType (IDS_NTYPE (INFO_LHS (arg_info)))
                );

        // extend the fundef arguments to include the new var
        FUNDEF_ARGS (INFO_FUNDEF (arg_info)) = TCappendArgs (FUNDEF_ARGS (INFO_FUNDEF (arg_info)), TBmakeArg (new_avis, NULL));

        // add the new var to RC
        MODARRAY_RC (arg_node) = TBmakeExprs( TBmakeId (new_avis), NULL);
        INFO_TMP_RCS (arg_info) = TCappendExprs (INFO_TMP_RCS (arg_info), DUPdoDupNode (MODARRAY_RC (arg_node)));
    }

    if (MODARRAY_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ELMPap (node * arg_node, info * arg_info)
{
    node * rec_filt, * new_args;
    DBUG_ENTER ();

    if (FUNDEF_ISLOOPFUN (AP_FUNDEF (arg_node))) {
        DBUG_PRINT ("checking application of %s ...", FUNDEF_NAME (AP_FUNDEF (arg_node)));

        /* check to see if we have found the recursive loopfun call */
        if (AP_FUNDEF (arg_node) == INFO_FUNDEF (arg_info)
                && !INFO_DO_UPDATE (arg_info)) {
            DBUG_PRINT ("  this is the recursive loop application");

            if (INFO_TMP_RCS (arg_info) != NULL) {
                DBUG_PRINT ("  have found tmp vars at application's fundef");

                /* we filter out current application args */
                rec_filt = filterDuplicateArgs (AP_ARGS (arg_node), &FUNDEF_ERC (INFO_FUNDEF (arg_info)));

                /* for each var in tmp RC, find a suitable var in ERC and replace it */
                new_args = findMatchingArgs (INFO_TMP_RCS (arg_info), rec_filt);
                AP_ARGS (arg_node) = TCappendExprs (AP_ARGS (arg_node), DUPdoDupTree (new_args));

                DBUG_PRINT ("  args are now:");
                DBUG_EXECUTE (if (AP_ARGS (arg_node) != NULL) {
                        PRTdoPrintFile (stderr, AP_ARGS (arg_node));});

                /* clear tmp rcs */
                INFO_TMP_RCS (arg_info) = FREEdoFreeTree (INFO_TMP_RCS (arg_info));

                /* clear the fundef ERC */
                FUNDEF_ERC (INFO_FUNDEF (arg_info)) = FREEdoFreeTree (FUNDEF_ERC (INFO_FUNDEF (arg_info)));
            }
        } else if (INFO_DO_UPDATE (arg_info)) { /* we are at the initial application */
            node * new_avis = NULL;
            node * new_vardec = NULL;
            int ap_arg_len = TCcountExprs (AP_ARGS (arg_node));
            int fun_arg_len = TCcountArgs (FUNDEF_ARGS (AP_FUNDEF (arg_node)));

            if (ap_arg_len != fun_arg_len) {
                DBUG_PRINT ("  number args for ap do not match fundef: %d != %d", ap_arg_len, fun_arg_len);

                /* we *always* append new args on fundef */
                for (; ap_arg_len < fun_arg_len; ap_arg_len++)
                {
                    /* we create a new variable and declare it to have the same shape as the
                     * emr tmp variable in the fundef args. Within ELMR this variable is then
                     * allocated using that shape information.
                     */
                    node * tmp = TCgetNthArg (ap_arg_len, FUNDEF_ARGS (AP_FUNDEF (arg_node)));
                    DBUG_PRINT ("  creating a new arg...");

                    /* the new avis must have the same type/shape as tmp arg in the fundef */
                    new_avis = TBmakeAvis ( TRAVtmpVarName ("emr_lifted"),
                            TYcopyType (ARG_NTYPE (tmp)));
                    AVIS_ISALLOCLIFT (new_avis) = TRUE;

                    /* append the avis to the args of ap */
                    AP_ARGS (arg_node) = TCappendExprs (AP_ARGS (arg_node), TBmakeExprs (TBmakeId (new_avis), NULL));

                    /* append to the current fundef's vardecs */
                    new_vardec = TBmakeVardec (new_avis, NULL);
                    AVIS_DECLTYPE (VARDEC_AVIS (new_vardec)) = TYcopyType (ARG_NTYPE (tmp));
                    INFO_FUNDEF (arg_info) = TCaddVardecs (INFO_FUNDEF (arg_info), new_vardec);
                    DBUG_PRINT ("  appended %s to fundef %s vardecs", AVIS_NAME (new_avis), FUNDEF_NAME (INFO_FUNDEF (arg_info)));
                }
            }
        }
    }

    DBUG_RETURN (arg_node);
}

node *
ELMPfundef (node * arg_node, info * arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_PRINT ("at N_fundef %s ...", FUNDEF_NAME (arg_node));
        INFO_FUNDEF (arg_info) = arg_node;

        if (!INFO_DO_UPDATE (arg_info) && FUNDEF_ISLOOPFUN (arg_node)) {
            DBUG_PRINT ("  found loopfun, inspecting body...");

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        } else if (INFO_DO_UPDATE (arg_info) && !FUNDEF_ISLOOPFUN (arg_node)) {
            DBUG_PRINT ("  inspecting body...");

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        }

        INFO_FUNDEF (arg_info) = NULL;
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}


/* @} */

#undef DBUG_PREFIX
