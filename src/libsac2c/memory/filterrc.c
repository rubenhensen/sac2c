/**
 * @file
 * @defgroup frc Filter Reuse Candidates
 * @ingroup mm
 *
 * This traversal removes reuse candidates that are being referenced at a later
 * stage and, thus, can never have an RC of 1!
 *
 * The traversal works in two modes: (1) scans arguments of alloc/reuse/alloc_or_*
 * N_prf that allocate the memory of with-loops (WITHOP_MEM), or (2) scans N_with
 * WITHOP_*RC N_id elements. These two modes exist as the traversal is used in
 * both the Optimisations Phase and in the Memory Phase - in the former we have
 * no N_prf dealing with allocations, while in the latter the WITHOP_*RC fields
 * are empty.
 *
 * This is implemented by a bottum up traversal. Whenever an identifier is being
 * met in an argument position (N_id), a correposnding entry in a dataflow mask
 * is being set.
 * When finding the argument of a _fill_ or traversing into a N_genarray or
 * N_modarray, we simply filter out all those reuse candidates whose data
 * flow masks are being set!
 *
 * Extra:
 * This optimisation does three things: (1) it removes WL ERCs that share the same
 * N_avis in the rec-loop argument list, (2) filters out ERCs that are already RCs
 * (as it is not possible to use an existing RC later on, there is no point in
 * keeping these as ERCs. This opens further opportunities for lifting allocations
 * out of loops. Finally, (3) we filter out invalid RCs (calls FRC (Filter Reuse
 * Candidates) traversal (in mode 2) which filters away _invalid_ *RC candidates.
 * This last part is especially important as otherwise cases where EMRL (EMR Loop
 * Memory Propagation) could be applied might get missed.
 *
 * @{
 *
 */
#include "filterrc.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#define DBUG_PREFIX "FRC"
#include "debug.h"

#include "print.h"
#include "DataFlowMask.h"
#include "emr_utils.h"
#include "str.h"
#include "memory.h"
#include "free.h"

/**
 * Mode enum
 */
typedef enum {
    FRC_default, /**< looks used during memory phase when F_alloc, etc. are available */
    FRC_wl, /**< looks *only* at WLs */
    FRC_prf, /**< looks *only* at N_prf except F_alloc, etc. */
    FRC_unknown
} trav_mode_t;

/**
 * @name INFO structure
 * @{
 */
struct INFO {
    dfmask_t *usemask;
    dfmask_t *oldmask;
    dfmask_t *thenmask;
    dfmask_t *elsemask;
    node *condargs;
    node *fundef;
    trav_mode_t mode;
    bool is_erc;
};

#define INFO_USEMASK(n) (n->usemask)
#define INFO_OLDMASK(n) (n->oldmask)
#define INFO_THENMASK(n) (n->thenmask)
#define INFO_ELSEMASK(n) (n->elsemask)
#define INFO_CONDARGS(n) (n->condargs)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_TRAV_MODE(n) (n->mode)
#define INFO_IS_ERC(n) (n->is_erc)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_USEMASK (result) = NULL;
    INFO_OLDMASK (result) = NULL;
    INFO_THENMASK (result) = NULL;
    INFO_ELSEMASK (result) = NULL;
    INFO_CONDARGS (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_TRAV_MODE (result) = FRC_unknown;
    INFO_IS_ERC (result) = FALSE;

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
 * @}  <!-- INFO structure -->
 */

/**
 * @name Entry functions
 * @{
 */

/**
 * @brief starting point of filter reuse candidates traversal (mode 1)
 *
 * @param syntax_tree
 * @return modified syntax_tree.
 */
node *
FRCdoFilterReuseCandidates (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    DBUG_PRINT ("Starting to filter reuse candidates (default mode)...");

    info = MakeInfo ();
    INFO_TRAV_MODE (info) = FRC_default;

    TRAVpush (TR_frc);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_PRINT ("Filtering of reuse candidates complete.");

    DBUG_RETURN (syntax_tree);
}

/**
 * @brief starting point of filter reuse candidates traversal (mode WL)
 *
 * @param syntax_tree
 * @return modified syntax_tree.
 */
node *
FRCdoFilterReuseCandidatesWL (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    DBUG_PRINT ("Starting to filter reuse candidates (WL mode)...");

    info = MakeInfo ();
    INFO_TRAV_MODE (info) = FRC_wl;

    TRAVpush (TR_frc);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_PRINT ("Filtering of reuse candidates complete.");

    DBUG_RETURN (syntax_tree);
}

/**
 * @brief starting point of filter reuse candidates traversal (mode 2)
 *
 * @param syntax_tree
 * @return modified syntax_tree.
 */
node *
FRCdoFilterReuseCandidatesPrf (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    DBUG_PRINT ("Starting to filter reuse candidates (Prf mode)...");

    info = MakeInfo ();
    INFO_TRAV_MODE (info) = FRC_prf;

    TRAVpush (TR_frc);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_PRINT ("Filtering of reuse candidates complete.");

    DBUG_RETURN (syntax_tree);
}

/**
 * @}  <!-- Entry functions -->
 */

/**
 * @name Static helper funcions
 * @{
 */

/**
 * @brief Filter out all invalid RCs
 *
 * @param arg_node N_exprs if N_id
 * @param arg_info Info structure with datamask pointer
 */
static node *
FilterTrav (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (arg_node != NULL && NODE_TYPE (arg_node) == N_exprs,
                 "Must pass N_exprs!");

    if (EXPRS_NEXT (arg_node) != NULL) {
        EXPRS_NEXT (arg_node) = FilterTrav (EXPRS_NEXT (arg_node), arg_info);
    }

    if (DFMtestMaskEntry (INFO_USEMASK (arg_info),
                          ID_AVIS (EXPRS_EXPR (arg_node)))) {
        DBUG_PRINT ("Invalid reuse candidate removed: %s",
                    ID_NAME (EXPRS_EXPR (arg_node)));
        arg_node = FREEdoFreeNode (arg_node);
    } else {
        /* when dealing with ERCs, we don't want to add these to the mask
         * as we eliminate all earlier references in other WLs. Furthermore,
         * we treat ERCs differently within EMRI (reuse.c) by eliminating them
         * from being selected more than once within a function scope.
         */
        if (!INFO_IS_ERC (arg_info))
            EXPRS_EXPR (arg_node) = TRAVdo (EXPRS_EXPR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief Filter out RCs in N_prf
 *
 * @param arg_node N_id from N_prf
 * @param arg_info Info struct
 * @return the modified N_id node
 */
static node *
FilterRCsInPrf (node *arg_node, info *arg_info)
{
    node *alloc;

    DBUG_ENTER ();

    alloc = ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (arg_node)));

    DBUG_EXECUTE (PRTdoPrintFile (stderr, alloc));

    DBUG_ASSERT ((NODE_TYPE (alloc) == N_prf)
                   && ((PRF_PRF (alloc) == F_alloc)
                       || (PRF_PRF (alloc) == F_alloc_or_reuse)
                       || (PRF_PRF (alloc) == F_reuse)
                       || (PRF_PRF (alloc) == F_alloc_or_reshape)
                       || (PRF_PRF (alloc) == F_alloc_or_resize)
                       || (PRF_PRF (alloc) == F_suballoc)),
                 "Illegal node type!");

    if ((PRF_PRF (alloc) != F_suballoc) && (PRF_PRF (alloc) != F_reuse)) {

        if (PRF_EXPRS3 (alloc) != NULL) {
            PRF_EXPRS3 (alloc) = FilterTrav (PRF_EXPRS3 (alloc), arg_info);
        }

        if (PRF_EXPRS3 (alloc) == NULL) {
            PRF_PRF (alloc) = F_alloc;
        }
    }

    DBUG_RETURN (arg_node);
}

/**
 * @}  <!-- Static helper functions -->
 */

/**
 * @name Traversal functions
 * @{
 */

/**
 * @brief Collect CONDFUN arguments into info structure and traverse through
 *        application N_fundef and arguments.
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
FRCap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node))) {
        INFO_CONDARGS (arg_info) = AP_ARGS (arg_node);
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
    }

    /* We need to also filter out ERCs from loop functions, we do this
     * in two ways: application and definition. The first is needed to
     * prevent EMRL from making a bad choice due to aliasing, the second
     * helps when selecting ERCs for WLs.
     *
     * XXX we use FRC_wl as we want this check to only be run iff immediately
     * after EMRCI (in opt phase).
     */
    if (INFO_TRAV_MODE (arg_info) == FRC_wl
        && FUNDEF_ISLOOPFUN (AP_FUNDEF (arg_node))) {
        if (AP_FUNDEF (arg_node) == INFO_FUNDEF (arg_info)
            && FUNDEF_ERC (AP_FUNDEF (arg_node)) != NULL) {
            // recursive loop call
            DBUG_PRINT ("checking recursive loop call...");
            DBUG_EXECUTE (PRTdoPrintFile (stderr, FUNDEF_ERC (AP_FUNDEF (arg_node))););
            INFO_IS_ERC (arg_info) = TRUE;
            FUNDEF_ERC (AP_FUNDEF (arg_node)) = FilterTrav (FUNDEF_ERC (AP_FUNDEF (arg_node)), arg_info);
            INFO_IS_ERC (arg_info) = FALSE;
        } else if (FUNDEF_ERC (AP_FUNDEF (arg_node)) != NULL) {
            // initial call site

            DBUG_PRINT ("checking initial loop call...");
            node *args = AP_ARGS (arg_node);
            node *fargs = FUNDEF_ARGS (AP_FUNDEF (arg_node));

            /* we scan through the application arguments and check that these aren't
             * referenced later. If so, we make sure to remove that the function ERCs
             * are filter of any signature arguments.
             */
            while (args != NULL)
            {
                if (DFMtestMaskEntry (INFO_USEMASK (arg_info),
                                      ID_AVIS (EXPRS_EXPR (args)))) {
                    DBUG_PRINT ("Invalid function reuse candidate removed: %s <ap-fundef> %s",
                                ID_NAME (EXPRS_EXPR (args)),
                                AVIS_NAME (ARG_AVIS (fargs)));

                    FUNDEF_ERC (AP_FUNDEF (arg_node))
                      = ElimDupesOfAvis (ARG_AVIS (fargs), FUNDEF_ERC (AP_FUNDEF (arg_node)));
                }

                args = EXPRS_NEXT (args);
                fargs = ARG_NEXT (fargs);
            }

        }
    }

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief For each argument check if it exsists in datamask, if not, add it.
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
FRCarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (DFMtestMaskEntry (INFO_OLDMASK (arg_info),
                          ID_AVIS (EXPRS_EXPR (INFO_CONDARGS (arg_info))))) {
        DBUG_PRINT ("Variable used in calling context: %s", ARG_NAME (arg_node));
        DFMsetMaskEntrySet (INFO_USEMASK (arg_info), ARG_AVIS (arg_node));
    }

    if (ARG_NEXT (arg_node) != NULL) {
        INFO_CONDARGS (arg_info) = EXPRS_NEXT (INFO_CONDARGS (arg_info));

        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief Go bottom up through the assignments.
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
FRCassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * Bottom-up traversal
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief Traverse through conditional condition and branches, updating
 *        datamask appropriately.
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
FRCcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Filtering conditional");

    if (INFO_THENMASK (arg_info) == NULL) {
        /*
         * If a loop or cond fun has no return values, it also has no funcond.
         * So, here we must still care about non-existant INFO_THENMASK and
         * INFO_ELSEMASK.
         */
        INFO_THENMASK (arg_info) = DFMgenMaskCopy (INFO_USEMASK (arg_info));
        INFO_ELSEMASK (arg_info) = INFO_USEMASK (arg_info);
    }

    INFO_USEMASK (arg_info) = INFO_THENMASK (arg_info);
    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);

    INFO_USEMASK (arg_info) = INFO_ELSEMASK (arg_info);
    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    DFMsetMaskOr (INFO_USEMASK (arg_info), INFO_THENMASK (arg_info));

    INFO_THENMASK (arg_info) = DFMremoveMask (INFO_THENMASK (arg_info));
    INFO_ELSEMASK (arg_info) = NULL;

    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief Traverse through conditional condition and branches, updating
 *        datamask appropriately.
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
FRCfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    FUNCOND_IF (arg_node) = TRAVdo (FUNCOND_IF (arg_node), arg_info);

    if (INFO_THENMASK (arg_info) == NULL) {
        INFO_THENMASK (arg_info) = DFMgenMaskCopy (INFO_USEMASK (arg_info));
        INFO_ELSEMASK (arg_info) = INFO_USEMASK (arg_info);
    }

    INFO_USEMASK (arg_info) = INFO_THENMASK (arg_info);
    FUNCOND_THEN (arg_node) = TRAVdo (FUNCOND_THEN (arg_node), arg_info);

    INFO_USEMASK (arg_info) = INFO_ELSEMASK (arg_info);
    FUNCOND_ELSE (arg_node) = TRAVdo (FUNCOND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief Traverse through N_fundef N_body, using a fresh datamask.
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
FRCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        if ((!FUNDEF_ISCONDFUN (arg_node)) || (INFO_USEMASK (arg_info) != NULL)) {

            dfmask_base_t *maskbase;
            dfmask_t *oldmask, *oldthen, *oldelse;

            DBUG_PRINT ("Filtering reuse candidates in function %s",
                        FUNDEF_NAME (arg_node));

            oldmask = INFO_USEMASK (arg_info);
            oldthen = INFO_THENMASK (arg_info);
            oldelse = INFO_ELSEMASK (arg_info);

            maskbase = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDECS (arg_node));

            INFO_USEMASK (arg_info) = DFMgenMaskClear (maskbase);
            INFO_THENMASK (arg_info) = NULL;
            INFO_ELSEMASK (arg_info) = NULL;

            if (oldmask != NULL) {
                INFO_OLDMASK (arg_info) = oldmask;
                if (FUNDEF_ARGS (arg_node) != NULL) {
                    FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
                }
                INFO_OLDMASK (arg_info) = NULL;
            }

            INFO_FUNDEF (arg_info) = arg_node;
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

            INFO_USEMASK (arg_info) = DFMremoveMask (INFO_USEMASK (arg_info));

            maskbase = DFMremoveMaskBase (maskbase);

            INFO_USEMASK (arg_info) = oldmask;
            INFO_THENMASK (arg_info) = oldthen;
            INFO_ELSEMASK (arg_info) = oldelse;

            DBUG_PRINT ("Filtering reuse candidates in function %s complete",
                        FUNDEF_NAME (arg_node));
        }
    }

    if ((INFO_USEMASK (arg_info) == NULL) && (FUNDEF_NEXT (arg_node) != NULL)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief If N_id does not exist in datamask, add it, thereby marking is as used.
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
FRCid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("... looking at N_id %s", ID_NAME (arg_node));

    if (!DFMtestMaskEntry (INFO_USEMASK (arg_info), ID_AVIS (arg_node))) {

        DBUG_PRINT ("Used Variable: %s", ID_NAME (arg_node));

        DFMsetMaskEntrySet (INFO_USEMASK (arg_info), ID_AVIS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief Filter arguments of N_prf
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
FRCprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (PRF_PRF (arg_node)) {
    case F_fill:
        if (INFO_TRAV_MODE (arg_info) == FRC_default) {
            PRF_ARG2 (arg_node) = FilterRCsInPrf (PRF_ARG2 (arg_node), arg_info);
        }
        break;

    case F_host2device:
    case F_device2host:
        if (PRF_ERC (arg_node) != NULL) {
            DBUG_PRINT ("Checking CUDA transfer for valid ERCs...");
            INFO_IS_ERC (arg_info) = TRUE;
            PRF_ERC (arg_node) = FilterTrav (PRF_ERC (arg_node), arg_info);
            INFO_IS_ERC (arg_info) = FALSE;
        }
        break;

    default:
        // do nothing
        break;
    }

    PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief Traverse N_with operation, partition, and code body.
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
FRCwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("filtering reuse candidates in WL");
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    DBUG_PRINT ("filtering reuse candidates in WL *complete*");

    DBUG_RETURN (arg_node);
}

/**
 * @brief Traverse N_with2 operation, segments, and code body.
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
FRCwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("filtering reuse candidates in WL2");
    WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
    WITH2_WITHID (arg_node) = TRAVdo (WITH2_WITHID (arg_node), arg_info);
    WITH2_SEGS (arg_node) = TRAVdo (WITH2_SEGS (arg_node), arg_info);
    WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
    DBUG_PRINT ("filtering reuse candidates in WL2 *complete*");

    DBUG_RETURN (arg_node);
}

/**
 * @brief Traverse N_with3 operation and iteration ranges.
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
FRCwith3 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("filtering reuse candidates in WL3");
    WITH3_OPERATIONS (arg_node) = TRAVdo (WITH3_OPERATIONS (arg_node), arg_info);
    WITH3_RANGES (arg_node) = TRAVopt (WITH3_RANGES (arg_node), arg_info);
    DBUG_PRINT ("filtering reuse candidates in WL3 *complete*");

    DBUG_RETURN (arg_node);
}

/**
 * @brief
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
FRCbreak (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* NOTE: from phase mem-alloc this should not be NULL, but as we call
     * the filtering traversal in phase opt, this node will be NULL.
     */
    if (BREAK_MEM (arg_node) != NULL) {
        BREAK_MEM (arg_node) = TRAVdo (BREAK_MEM (arg_node), arg_info);
    }

    if (BREAK_NEXT (arg_node) != NULL) {
        BREAK_NEXT (arg_node) = TRAVdo (BREAK_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
FRCfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    FOLD_NEUTRAL (arg_node) = TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);

    if (FOLD_NEXT (arg_node) != NULL) {
        FOLD_NEXT (arg_node) = TRAVdo (FOLD_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief Filter N_genarray memory and RCs (and ERCs).
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
FRCgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (INFO_TRAV_MODE (arg_info)) {
    case FRC_default:
        GENARRAY_MEM (arg_node) = FilterRCsInPrf (GENARRAY_MEM (arg_node), arg_info);
        break;

    case FRC_wl:
        if (GENARRAY_RC (arg_node) != NULL) {
            DBUG_PRINT ("... we have RC");
            GENARRAY_RC (arg_node) = FilterTrav (GENARRAY_RC (arg_node), arg_info);
        }
        if (GENARRAY_PRC (arg_node) != NULL) {
            DBUG_PRINT ("... we have PRC");
            GENARRAY_PRC (arg_node) = FilterTrav (GENARRAY_PRC (arg_node), arg_info);
        }
        if (GENARRAY_ERC (arg_node) != NULL) {
            INFO_IS_ERC (arg_info) = TRUE;
            DBUG_PRINT ("... we have ERC");
            GENARRAY_ERC (arg_node) = FilterTrav (GENARRAY_ERC (arg_node), arg_info);
            INFO_IS_ERC (arg_info) = FALSE;
        }
        break;

    default:
        /* do nothing */
        break;
    }

    GENARRAY_SHAPE (arg_node) = TRAVdo (GENARRAY_SHAPE (arg_node), arg_info);

    if (GENARRAY_DEFAULT (arg_node) != NULL) {
        GENARRAY_DEFAULT (arg_node) = TRAVdo (GENARRAY_DEFAULT (arg_node), arg_info);
    }

    if (GENARRAY_NEXT (arg_node) != NULL) {
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief Filter N_modarray memory and RCs (and ERCs).
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
FRCmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (INFO_TRAV_MODE (arg_info)) {
    case FRC_default:
        MODARRAY_MEM (arg_node) = FilterRCsInPrf (MODARRAY_MEM (arg_node), arg_info);
        break;

    case FRC_wl:
        if (MODARRAY_RC (arg_node) != NULL) {
            DBUG_PRINT ("... we have RC");
            MODARRAY_RC (arg_node) = FilterTrav (MODARRAY_RC (arg_node), arg_info);
        }
        if (MODARRAY_ERC (arg_node) != NULL) {
            INFO_IS_ERC (arg_info) = TRUE;
            DBUG_PRINT ("... we have ERC");
            MODARRAY_ERC (arg_node) = FilterTrav (MODARRAY_ERC (arg_node), arg_info);
            INFO_IS_ERC (arg_info) = FALSE;
        }
        break;

    default:
        /* do nothing */
        break;
    }

    MODARRAY_ARRAY (arg_node) = TRAVdo (MODARRAY_ARRAY (arg_node), arg_info);

    if (MODARRAY_NEXT (arg_node) != NULL) {
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
FRCcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);

    if (CODE_CBLOCK (arg_node) != NULL) {
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    }

    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
FRCrange (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    RANGE_RESULTS (arg_node) = TRAVdo (RANGE_RESULTS (arg_node), arg_info);
    RANGE_UPPERBOUND (arg_node) = TRAVopt (RANGE_UPPERBOUND (arg_node), arg_info);
    RANGE_LOWERBOUND (arg_node) = TRAVopt (RANGE_LOWERBOUND (arg_node), arg_info);
    RANGE_CHUNKSIZE (arg_node) = TRAVopt (RANGE_CHUNKSIZE (arg_node), arg_info);

    if (RANGE_BODY (arg_node) != NULL) {
        RANGE_BODY (arg_node) = TRAVdo (RANGE_BODY (arg_node), arg_info);
    }

    if (RANGE_NEXT (arg_node) != NULL) {
        RANGE_NEXT (arg_node) = TRAVdo (RANGE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @}  <!-- Traversal functions -->
 */

/**
 * @}  <!-- Filter Reuse Candidates -->
 */

#undef DBUG_PREFIX
