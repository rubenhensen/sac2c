/**
 * @file
 * @defgroup emrci Extended memory reuse candidate inference
 *
 * Here, we add arrays that are of
 * the same shape and tha are defined in the current function but are not referenced
 * in WL-body at all. This may sound counter-intuitive at first, but it enables memory
 * reuse of the following kind:
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
 * Here, we would like to reuse the memory allocated for a when compting b.
 *
 * @note
 * that EMR does not update the WL RC, but instead collects potnetial
 * candidates within another strucutre, ERC (extended reuse candidates),
 * on both WL-OPs and LoopFuns. The overall intention is to optimise
 * reuse candidates present within WL RC and allow for memeory reuse
 * in loops (do/for/while) by propogating reuse candidates from a higher
 * scope. More details in docs/projects/prealloc
 *
 * The ability to do such reuse is instrumental when trying to achieve a dual buffer
 * swapping solution for a loop around something like     a = relax( a);
 *
 * Note here, that all these reuse candidate inferences potentially over-approximate the
 * set of candidates (eg arrays that are still refered to later); however, those cases are
 * being narrowed by subphases of the mem-phase, specifically SRCE and FRC, and by dynamic
 * reference count inspections at runtime :-)
 *
 * However, 4) does NEED to run later as we must not add reuse candidates that might have
 * been optimised away in the meantime.... For this very purpose we offer a second entry
 * point which applies EMR only and potentially adds more reuse candidates.
 *
 * @ingroup mm
 *
 * @{
 */
#include "emr_candidate_inference.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "DupTree.h"

#define DBUG_PREFIX "EMRCI"
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
#include "filterrc.h"
#include "emr_loop_optimisation.h"

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

/**
 * @brief
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/
node *
EMRCIdoWithloopExtendedReuseCandidateInference (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    INFO_DO_EMR (arg_info) = TRUE;

    TRAVpush (TR_emrci);
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    if (global.optimize.doelaaf) {
/*
 * EMR Loop Application Arg Filtering
 *
 * This optimisation does three things: (1) it removes WL ERCs that share the same N_avis
 * in the rec-loop argument list, (2) filters out ERCs that are already RCs (as it
 * is not possible to use an existing RC later on, there is no point in keeping these
 * as ERCs. This opens further opportunities for lifting allocations out of loops. Finally,
 * (3) we filter out invalid RCs (calls FRC (Filter Reuse Candidates)
 * traversal (in mode 2) which filters away _invalid_ *RC candidates. This last
 * part is especially important as otherwise cases where EMRL (EMR Loop Memory
 * Propagation) could be applied might get missed.
 */
        syntax_tree = FRCdoFilterReuseCandidatesNoPrf (syntax_tree);
    }

    if (global.optimize.doemrl) {
        syntax_tree = EMRLdoExtendLoopMemoryPropagation (syntax_tree);
    }

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
    DBUG_PRINT_TAG (DBUG_PREFIX "_EMR", "filtering out %s", ID_NAME (id));
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

    DBUG_PRINT_TAG (DBUG_PREFIX "_EMR", "filtering out duplicate N_avis");

    filtered = TCfilterExprsArg (doAvisMatch, fexprs, exprs);

    /* we delete all duplicates from col */
    if (filtered != NULL) {
        DBUG_PRINT_TAG (DBUG_PREFIX "_EMR", "  found and removed the following duplicates:");
        DBUG_EXECUTE_TAG (DBUG_PREFIX "_EMR", PRTdoPrintFile (stderr, filtered));
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

/**
 * @brief traverse N_fundef arguments and body
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
EMRCIfundef (node *arg_node, info *arg_info)
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

/**
 * @brief traverse N_ap for recursive loop function application
 *
 * When we find the application, we update the current N_fundef ERC
 * value with the current state of ERCs. We then continue through the
 * arguments of the N_ap.
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
EMRCIap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISLOOPFUN (INFO_FUNDEF (arg_info))
            && AP_FUNDEF (arg_node) == INFO_FUNDEF (arg_info)) {
        FUNDEF_ERC (INFO_FUNDEF (arg_info)) = TCappendExprs (FUNDEF_ERC (INFO_FUNDEF (arg_info)), DUPdoDupTree (INFO_EMR_RC (arg_info)));
        DBUG_PRINT_TAG (DBUG_PREFIX "_EMR", "extended reuse candidates for rec loopfun application:");
        DBUG_EXECUTE_TAG (DBUG_PREFIX "_EMR", if (FUNDEF_ERC (INFO_FUNDEF (arg_info)) != NULL) {
            PRTdoPrintFile (stderr, FUNDEF_ERC (INFO_FUNDEF (arg_info)));
        });
    }

    AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief traverse N_cond branches collecting more ERCs
 *
 * We intentionally stack the INFO_EMR_RC for each branch to prevent
 * incorrect updting of WLs within each branch.
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
EMRCIcond (node *arg_node, info *arg_info)
{
    node * old_chain;
    DBUG_ENTER ();

    DBUG_PRINT (" inspecting cond ...");
    COND_COND (arg_node) = TRAVopt (COND_COND (arg_node), arg_info);

    old_chain = INFO_EMR_RC (arg_info);

    if (COND_THEN (arg_node) != NULL) {
        INFO_EMR_RC (arg_info) = DUPdoDupTree (old_chain);
        COND_THEN (arg_node) = TRAVopt (COND_THEN (arg_node), arg_info);
        if (INFO_EMR_RC (arg_info) != NULL) {
            INFO_EMR_RC (arg_info) = FREEdoFreeTree (INFO_EMR_RC (arg_info));
        }
    }

    if (COND_ELSE (arg_node) != NULL) {
        INFO_EMR_RC (arg_info) = DUPdoDupTree (old_chain);
        COND_ELSE (arg_node) = TRAVopt (COND_ELSE (arg_node), arg_info);
        if (INFO_EMR_RC (arg_info) != NULL) {
            INFO_EMR_RC (arg_info) = FREEdoFreeTree (INFO_EMR_RC (arg_info));
        }
    }

    INFO_EMR_RC (arg_info) = old_chain;

    DBUG_RETURN (arg_node);
}

/**
 * @brief traverse N_arg collecting ERCs
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
EMRCIarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("adding emr_rc %s arg ...", ARG_NAME (arg_node));
    INFO_EMR_RC (arg_info)
      = TBmakeExprs (TBmakeId (ARG_AVIS (arg_node)), INFO_EMR_RC (arg_info));

    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief traverse N_ids collecting ERCs
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
EMRCIids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT_TAG (DBUG_PREFIX "_EMR", "adding emr_rc %s ids ...", IDS_NAME (arg_node));
    INFO_EMR_RC (arg_info)
      = TBmakeExprs (TBmakeId (IDS_AVIS (arg_node)), INFO_EMR_RC (arg_info));

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

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
EMRCIassign (node *arg_node, info *arg_info)
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
 * @brief traverse N_let storing the LHS and collecting ERCs
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
EMRCIlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    /* Only AFTER we traversed the body we add the defined vars in the EMR_RCs */
    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief traverse N_prf removing some collected from ERCs
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
EMRCIprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("checking if N_prf references an ERC...");

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

    DBUG_RETURN (arg_node);
}

/**
 * @brief traverse N_with and inspect its body
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
EMRCIwith (node *arg_node, info *arg_info)
{
    node *emr_chain = NULL;
    DBUG_ENTER ();

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

    /*
     * Annotate RCs and find further reuse candidates if appropriate
     */
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    /*
     * Be sure to remove all GENERATOR_GENWIDTH
     */
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    /*
     * Continue RC inference for nested WLs
     */
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    /*
     * Here, we have to resurrect the emr-candidates as we had them before
     * processing this with loop:
     */
    if (INFO_EMR_RC (arg_info) != NULL)
        FREEdoFreeTree (INFO_EMR_RC (arg_info));
    INFO_EMR_RC (arg_info) = emr_chain;

    DBUG_RETURN (arg_node);
}

/**
 * @brief traverse N_genarray and set current ERCs
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
EMRCIgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * Annotate extended reuse candidates.
     */
    GENARRAY_ERC (arg_node)
      = MatchingRCs (INFO_EMR_RC (arg_info), INFO_LHS (arg_info), NULL);
    DBUG_PRINT_TAG (DBUG_PREFIX "_EMR", "Genarray ERCs: ");
    DBUG_EXECUTE_TAG (DBUG_PREFIX "_EMR", if (GENARRAY_ERC (arg_node) != NULL) {
        PRTdoPrintFile (stderr, GENARRAY_ERC (arg_node));
    });

    if (GENARRAY_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief traverse N_modarray and set current ERCs
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
EMRCImodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * Annotate extended reuse candidates.
     */
    MODARRAY_ERC (arg_node)
      = MatchingRCs (INFO_EMR_RC (arg_info), INFO_LHS (arg_info), MODARRAY_ARRAY (arg_node));
    DBUG_PRINT_TAG (DBUG_PREFIX "_EMR", "Modarray ERCs: ");
    DBUG_EXECUTE_TAG (DBUG_PREFIX "_EMR", if (MODARRAY_ERC (arg_node) != NULL) {
        PRTdoPrintFile (stderr, MODARRAY_ERC (arg_node));
    });

    if (MODARRAY_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/* @} */

#undef DBUG_PREFIX
