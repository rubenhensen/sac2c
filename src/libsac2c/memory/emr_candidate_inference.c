/**
 * @file
 * @defgroup emrci Extended memory reuse candidate inference
 * @ingroup mm
 *
 * Here, we add arrays that are of the same shape and tha are defined in the
 * current function but are not referenced in WL-body at all. This may sound
 * counter-intuitive at first, but it enables memory reuse of the following
 * kind:
 *
 * Consider:
 *
 * ~~~~{.c}
 * int main( )
 * {
 *    a = genarray( [15], 0);
 *
 *    print(a);
 *
 *    b = genarray( [15], 1);
 *    print(b);
 *
 *    return(0);
 * }
 * ~~~~
 *
 * Here, we would like to reuse the memory allocated for a when computing b.
 *
 * @note
 * that EMR does not update the with-loop RC, but instead collects potential
 * candidates within another structure, ERC (extended reuse candidates), on both
 * with-loop operations and loop functions. The overall intention is to optimise
 * reuse candidates present within with-loops and allow for memory reuse in
 * loops (do/for/while) by propagating reuse candidates from a higher scope.
 * More details in docs/projects/prealloc
 *
 * The ability to do such reuse is instrumental when trying to achieve a dual
 * buffer swapping solution for a loop around something like `a = relax( a);`.
 *
 * @note
 * That all these reuse candidate inferences potentially over-approximate the
 * set of candidates (e.g. arrays that are still referred to later); however,
 * those cases are being narrowed by subphases of the mem-phase, specifically
 * SRCE and FRC, and by dynamic reference count inspections at runtime :-)
 *
 * However EMRCI does need to run later as we must not add reuse candidates that
 * might have been optimised away in the meantime.
 *
 * @{
 */
#include "emr_candidate_inference.h"
#include "emr_utils.h"

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
 * Mode enum
 */
typedef enum {
    EMR_all,
    EMR_prf,
    EMR_none
} emr_mode_t;

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *lhs;
    node *rc;
    /* extended reuse */
    node *emr_rc;
    emr_mode_t mode;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_RC(n) ((n)->rc)
#define INFO_EMR_RC(n) ((n)->emr_rc)
#define INFO_EMR_MODE(n) ((n)->mode)

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
    INFO_EMR_RC (result) = NULL;
    INFO_EMR_MODE (result) = EMR_none;

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
 *        prints out the ERC for WITHOPs (except N_fold).
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
EMRCIprintPreFun (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (NODE_TYPE (arg_node)) {
    case N_genarray:
        if (GENARRAY_ERC (arg_node)) {
            INDENT;
            fprintf (global.outfile, "/* ERC (");
            if (GENARRAY_ERC (arg_node) != NULL) {
                GENARRAY_ERC (arg_node) = PRTexprs (GENARRAY_ERC (arg_node), arg_info);
            }
            fprintf (global.outfile, ") */\n");
        }
        break;
    case N_modarray:
        if (MODARRAY_ERC (arg_node)) {
            INDENT;
            fprintf (global.outfile, "/* ERC (");
            if (MODARRAY_ERC (arg_node) != NULL) {
                MODARRAY_ERC (arg_node) = PRTexprs (MODARRAY_ERC (arg_node), arg_info);
            }
            fprintf (global.outfile, ") */\n");
        }
        break;
    case N_prf:
        switch (PRF_PRF (arg_node)) {
        case F_host2device:
        case F_device2host:
        case F_host2device_start:
        case F_device2host_start:
            fprintf (global.outfile, "/* ERC (");
            if (PRF_ERC (arg_node) != NULL) {
                PRF_ERC (arg_node) = PRTexprs (PRF_ERC (arg_node), arg_info);
            }
            fprintf (global.outfile, ") */ ");
            break;
        default:
            break;
        }
    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief Entry function into EMRCI traversal
 *
 * @param syntax_tree
 * @return modified syntax_tree.
 */
node *
EMRCIdoWithloopExtendedReuseCandidateInference (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    INFO_EMR_MODE (arg_info) = EMR_all;

    TRAVpush (TR_emrci);
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

/**
 * @brief Entry function into EMRCI traversal, looks only at N_prf
 *
 * @param syntax_tree
 * @return modified syntax_tree.
 */
node *
EMRCIdoWithloopExtendedReuseCandidateInferencePrf (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    INFO_EMR_MODE (arg_info) = EMR_prf;

    TRAVpush (TR_emrci);
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

static node *
MatchingRCs (node *rcs, node *ids, node *modarray)
{
    node *match = NULL;

    DBUG_ENTER ();

    DBUG_PRINT ("looking for matching RC -> %s", IDS_NAME (ids));

    if (rcs != NULL) {
        match = MatchingRCs (EXPRS_NEXT (rcs), ids, modarray);

        if ((TUeqElementSize (ID_NTYPE (EXPRS_EXPR (rcs)), IDS_NTYPE (ids))
             && (ShapeMatch (ID_NTYPE (EXPRS_EXPR (rcs)), IDS_NTYPE (ids))
                 || TCshapeVarsMatch (ID_AVIS (EXPRS_EXPR (rcs)), IDS_AVIS (ids))))
            || ((modarray != NULL)
                 && (ID_AVIS (EXPRS_EXPR (rcs)) == ID_AVIS (modarray)))) {
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
        FUNDEF_ERC (INFO_FUNDEF (arg_info))
          = TCappendExprs (FUNDEF_ERC (INFO_FUNDEF (arg_info)),
                           DUPdoDupTree (INFO_EMR_RC (arg_info)));
        DBUG_PRINT ("extended reuse candidates for rec loopfun application:");
        DBUG_EXECUTE (if (FUNDEF_ERC (INFO_FUNDEF (arg_info)) != NULL) {
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

    DBUG_PRINT ("adding emr_rc %s ids ...", IDS_NAME (arg_node));
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
        DBUG_PRINT ("checking if F_idx_modarray_* references an ERC...");
        INFO_EMR_RC (arg_info)
          = filterDuplicateId (PRF_ARGS (arg_node), &INFO_EMR_RC (arg_info));
        DBUG_PRINT ("EMR RCs left after filtering out N_prf args");
        DBUG_EXECUTE (if (INFO_EMR_RC (arg_info) != NULL) {
            PRTdoPrintFile (stderr, INFO_EMR_RC (arg_info));
        });
        break;

    case F_host2device:
    case F_device2host:
        DBUG_PRINT ("checking for ERCs of CUDA transfer N_prf...");
        PRF_ERC (arg_node)
          = MatchingRCs (INFO_EMR_RC (arg_info), INFO_LHS (arg_info), NULL);
        DBUG_PRINT ("Found ERCs: ");
        DBUG_EXECUTE (if (PRF_ERC (arg_node) != NULL) {
            PRTdoPrintFile (stderr, PRF_ERC (arg_node));
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
    if (INFO_EMR_MODE (arg_info) == EMR_all) {
        GENARRAY_ERC (arg_node)
          = MatchingRCs (INFO_EMR_RC (arg_info), INFO_LHS (arg_info), NULL);
        DBUG_PRINT ("Genarray ERCs: ");
        DBUG_EXECUTE (if (GENARRAY_ERC (arg_node) != NULL) {
            PRTdoPrintFile (stderr, GENARRAY_ERC (arg_node));
        });

        if (GENARRAY_NEXT (arg_node) != NULL) {
            INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
            GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
        }
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
    if (INFO_EMR_MODE (arg_info) == EMR_all) {
        MODARRAY_ERC (arg_node)
          = MatchingRCs (INFO_EMR_RC (arg_info), INFO_LHS (arg_info), MODARRAY_ARRAY (arg_node));
        DBUG_PRINT ("Modarray ERCs: ");
        DBUG_EXECUTE (if (MODARRAY_ERC (arg_node) != NULL) {
            PRTdoPrintFile (stderr, MODARRAY_ERC (arg_node));
        });

        if (MODARRAY_NEXT (arg_node) != NULL) {
            INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
            MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** @} */

#undef DBUG_PREFIX
