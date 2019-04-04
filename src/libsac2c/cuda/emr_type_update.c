/**
 * @file
 * @defgroup emrcudalift EMR loop allocation lifting for CUDA
 * @ingroup cuda
 *
 * @brief Convert all ERCs in EMRL affected fundefs with CUDA-WL to CUDA device types.
 *
 * This traverse works *only* with IWLMEM in two modes:
 *  * we update initial application of loop fun, and loop fun arguments
 *  * we update the recursive application.
 *
 * The first mode is done *before* applying CUDA WL transfer insertion (IWLMEM), as we
 * want to avoid adding host2device transfers for lifted ERCs. The second mode is applied
 * *after* IWLMEM to update the recursive application with any LHS of host2device calls.
 *
 * To ilistrate what this traversal does, lets walk through with a code example. Before
 * we reach IWLMEM, we have this situation:
 *
 * ~~~~
 * loop_entry (A)
 * {
 *    type var_emr_lift;
 *
 *    return loop_0 (A, var_emr_lift);
 * }
 *
 * loop_0 (A, var_emr_tmp)
 * {
 *    type a;
 *    type a_0;
 *
 *    a = wl ( A ) [ERC: var_emr_tmp];
 *
 *    if (some cond)
 *      a_0 = loop_0 (a, A);
 *    return (some cond) ? a_0 : a;
 * }
 * ~~~~
 *
 * After first mode step of EMRTU:
 *
 * ~~~~
 * loop_entry (A)
 * {
 *    type_dev var_emr_lift_dev;
 *
 *    return loop_0 (A, var_emr_lift_dev);
 * }
 *
 * loop_0 (A, var_emr_tmp_dev)
 * {
 *    type a;
 *    type a_0;
 *
 *    a = wl ( A ) [ERC: var_emr_tmp_dev];
 *
 *    if (some cond)
 *      a_0 = loop_0 (a, A);
 *    return (some cond) ? a_0 : a;
 * }
 * ~~~~
 *
 * After IWLMEM traversal:
 *
 * ~~~~
 * loop_entry (A)
 * {
 *    type_dev var_emr_lift_dev;
 *
 *    return loop_0 (A, var_emr_lift_dev);
 * }
 *
 * loop_0 (A, var_emr_tmp_dev)
 * {
 *    type a;
 *    type_dev A_dev;
 *    type_dev a_dev;
 *    type a_0;
 *
 *    A_dev = host2device (A);
 *    a_dev = wl ( A_dev ) [ERC: var_emr_tmp_dev];
 *    a = device2host (a_dev);
 *
 *    if (some cond)
 *      a_0 = loop_0 (a, A);
 *    return (some cond) ? a_0 : a;
 * }
 * ~~~~
 *
 * And finally after step two of EMRTU:
 *
 * ~~~~
 * loop_entry (A)
 * {
 *    type_dev var_emr_lift_dev;
 *
 *    return loop_0 (A, var_emr_lift_dev);
 * }
 *
 * loop_0 (A, var_emr_tmp_dev)
 * {
 *    type a;
 *    type_dev A_dev;
 *    type_dev a_dev;
 *    type a_0;
 *
 *    A_dev = host2device (A);
 *    a_dev = wl ( A_dev ) [ERC: var_emr_tmp_dev];
 *    a = device2host (a_dev);
 *
 *    if (some cond)
 *      a_0 = loop_0 (a, A_dev);
 *    return (some cond) ? a_0 : a;
 * }
 * ~~~~
 *
 * @note This transformation can only be applied *after* cudarizable WLs have been
 *       identified.
 *
 * @{
 */
#include "emr_type_update.h"

#include "types.h"
#include "type_utils.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "memory.h"

#define DBUG_PREFIX "EMRTU"
#include "debug.h"

#include "free.h"
#include "new_types.h"
#include "cuda_utils.h"
#include "LookUpTable.h"
#include "DupTree.h"

/** @name INFO structure
 *  @{
 */
struct INFO {
    node *fundef; /**< Holds current N_fundef */
    lut_t *lut; /**< LUT is used for storing either new device N_avis (for fundef update),
                   or N_avis pairs (for ap update) */
    node *ap_args;  /**< N_ap arguments */
    bool update_ap; /**< Flag indicating what mode we are in (either fundef updating, or
                       ap updating) */
    node *letids;   /**< The the LHS of N_prf */
    node *wl_ercs;  /**< Holds all CUDA-WL ERCs for a given fundef */
};

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LUT(n) ((n)->lut)
#define INFO_AP_ARGS(n) ((n)->ap_args)
#define INFO_UPDATE_AP(n) ((n)->update_ap)
#define INFO_LETIDS(n) ((n)->letids)
#define INFO_WLERCS(n) ((n)->wl_ercs)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_LUT (result) = NULL;
    INFO_AP_ARGS (result) = NULL;
    INFO_UPDATE_AP (result) = FALSE;
    INFO_LETIDS (result) = NULL;
    INFO_WLERCS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** @} */

/** @name Static helper functions
 *  @{
 */

/**
 * @brief Convert from a host ntype to a device ntype, while preserving shape information.
 *
 * @param host_type The host ntype
 * @return A device ntype struct
 */
static ntype *
ConvertHost2DeviceType (ntype *host_type)
{
    ntype *scalar_type, *dev_type = NULL;
    simpletype sty;

    DBUG_ENTER ();

    /* If the N_id is of known dimension and is not a scalar */
    DBUG_ASSERT (TUdimKnown (host_type), "AUD N_id found!");
    if (TYgetDim (host_type) > 0) {
        /* If the scalar type is simple, e.g. int, float ... */
        if (TYisSimple (TYgetScalar (host_type))) {
            dev_type = TYcopyType (host_type);
            scalar_type = TYgetScalar (dev_type);
            /* Get the corresponding device simple type e.g. int_dev, float_dev...*/
            sty = CUh2dSimpleTypeConversion (TYgetSimpleType (scalar_type));
            /* Set the device simple type */
            scalar_type = TYsetSimpleType (scalar_type, sty);
        }
    }

    DBUG_RETURN (dev_type);
}

/**
 * @brief Used as part of LUTmap operation to free LUT keys.
 *
 * @param value The LUT value
 * @param key The LUT key, which will be freed
 * @return the value
 */
static void *
FreeLutArgs (void *value, void *key)
{
    key = MEMfree (key);
    return value;
}

/**
 * @brief Search through N_exprs chain (of N_id) for N_avis
 *
 * @param exprs_chain N_exprs chain containing N_id
 * @param avis The N_avis to look for
 * @return true if found, otherwise false
 */
static bool
IsAvisInExprs (node *exprs_chain, node *avis)
{
    bool ret = FALSE;

    DBUG_ENTER ();

    while (exprs_chain != NULL) {
        if (ID_AVIS (EXPRS_EXPR (exprs_chain)) == avis) {
            ret = TRUE;
            break;
        }
        exprs_chain = EXPRS_NEXT (exprs_chain);
    }

    DBUG_RETURN (ret);
}

/**
 * @brief Anonymouse traversal function (N_with)
 *
 * @param arg_node N_with
 * @param arg_info info struct
 * @return N_with
 */
static node *
ATravWith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* we only need to deal with CUDA WLs */
    if (WITH_CUDARIZABLE (arg_node)) {
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief Anonymouse traversal function (N_modarray)
 *
 * @param arg_node N_modarray
 * @param arg_info info struct
 * @return N_modarray
 */
static node *
ATravModarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_WLERCS (arg_info)
      = TCappendExprs (INFO_WLERCS (arg_info), DUPdoDupTree (MODARRAY_ERC (arg_node)));
    MODARRAY_NEXT (arg_node) = TRAVopt (MODARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief Anonymouse traversal function (N_genarray)
 *
 * @param arg_node N_genarray
 * @param arg_info info struct
 * @return N_genarray
 */
static node *
ATravGenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_WLERCS (arg_info)
      = TCappendExprs (INFO_WLERCS (arg_info), DUPdoDupTree (GENARRAY_ERC (arg_node)));
    GENARRAY_NEXT (arg_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}
/** @} */

/** @name Entry functions
 *  @{
 */

/**
 * @brief Traverse syntax_tree looking for loop functions with additional arguments
 *        due to EMRL (EMR loop optimisation), and convert these if they used by
 *        CUDA wothloops.
 *
 * @param syntax_tree
 * @return syntax_tree
 */
node *
EMRTUdoEMRUpdateFun (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_emrtu);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/**
 * @brief Traverse syntax_tree and update all loop function recursive calls where
 *        one or more arguments are the result of EMRL optimisation. We search for
 *        host2device primitives and replace their argument with the one in the
 *        application.
 *
 * @param syntax_tree
 * @return syntax_tree
 */
node *
EMRTUdoEMRUpdateAp (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    INFO_UPDATE_AP (info) = TRUE;

    TRAVpush (TR_emrtu);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
/** @} */

/** @name Traversal functions
 *  @{
 */

/**
 * @brief Traverse body of LAC functions
 *
 * We do this in two stages, the first stage looks for initial application of loop
 * function. The second stage enters into the loop function to search for or modify its
 * arguments.
 *
 * @param arg_node N_fundef
 * @param arg_info info struct
 * @return N_fundef
 */
node *
EMRTUfundef (node *arg_node, info *arg_info)
{
    node *old_fundef;

    DBUG_ENTER ();

    if (!FUNDEF_ISLACFUN (arg_node)) {
        DBUG_PRINT ("inspecting body of %s", FUNDEF_NAME (arg_node));
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    } else if (INFO_AP_ARGS (arg_info) != NULL) {
        DBUG_PRINT ("inspecting application body of %s", FUNDEF_NAME (arg_node));
        old_fundef = INFO_FUNDEF (arg_info);
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = old_fundef;
    } else
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief Traverse application functions if they are LAC functions *and* were modified by
 *        EMRL previously (that they have lifted allocations)
 *
 * If we are at the initial application, we either
 *
 * a. search for CUDA-WL within fundef and collect ERCs, and if any of the ERCs are
 *    arguments of the fundef, we update these to be device type.
 * b. check for host2device primitives, and if their RHS is an argument of the recursive
 *    N_ap, change the argument to be the LHS.
 *
 * @param arg_node N_ap
 * @param arg_info info struct
 * @return N_ap
 */
node *
EMRTUap (node *arg_node, info *arg_info)
{
    ntype *dev_type;
    node *ap_args, *fundef_args, *old_ap_args;
    node *id_avis, *arg_avis, *new_avis, *vardec;
    lut_t *old_lut;

    DBUG_ENTER ();

    DBUG_PRINT ("found application of %s", AP_NAME (arg_node));

    /* we only traverse loop functions that were changed by the EMRL (EMR loop
     * optimisation) previously */
    if (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node))
        && FUNDEF_ISEMRLIFTED (AP_FUNDEF (arg_node))) {
        if (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info)) { /* initial application */
            DBUG_PRINT ("...checking for lifted allocations");

            ap_args = AP_ARGS (arg_node);
            fundef_args = FUNDEF_ARGS (AP_FUNDEF (arg_node));

            /* We use the LUT for two different stages:
             * 1. when looking for fundef arguments that have to be changed to device
             *    type)
             * 2. when replacing recursive N_ap arguments with device types (to maintain
             *    the buffer swapping pattern)
             *
             * This is why we create a new LUT in all cases.
             */
            old_lut = INFO_LUT (arg_info);
            INFO_LUT (arg_info) = LUTgenerateLut ();

            /* if we are only updating the recurisve N_ap arguments, we don't need to
             * update the initial N_ap. */
            if (!INFO_UPDATE_AP (arg_info)) {

                /* We use an anonymous traversal to find all ERCs in CUDA-WLs */
                anontrav_t atrav[4] = {{N_with, &ATravWith},
                                       {N_genarray, &ATravGenarray},
                                       {N_modarray, &ATravModarray},
                                       {(nodetype)0, NULL}};

                TRAVpushAnonymous (atrav, &TRAVsons);
                AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
                TRAVpop ();

                if (INFO_WLERCS (arg_info) != NULL) { /* found ERCs in CUDA-WLs */
                    while (ap_args != NULL) {
                        DBUG_ASSERT (fundef_args != NULL,
                                     "# of Ap args != # of Fundef args!");

                        id_avis = ID_AVIS (EXPRS_EXPR (ap_args));
                        arg_avis = ARG_AVIS (fundef_args);

                        /* if the lifted argument is *not* a ERC of a CUDA-WL, ignore */
                        if (AVIS_ISALLOCLIFT (id_avis)
                            && IsAvisInExprs (INFO_WLERCS (arg_info), arg_avis)) {
                            DBUG_PRINT ("......found lifted argument ap arg %s, fundef "
                                        "arg %s",
                                        AVIS_NAME (id_avis), AVIS_NAME (arg_avis));

                            /* create new local avis (cuda device type) */
                            dev_type = ConvertHost2DeviceType (AVIS_TYPE (id_avis));
                            new_avis
                              = TBmakeAvis (TRAVtmpVarName ("emr_lift_dev"), dev_type);
                            AVIS_ISALLOCLIFT (new_avis) = TRUE;

                            /* get old vardec and update it */
                            DBUG_ASSERT (AVIS_DECL (id_avis) != NULL
                                           && NODE_TYPE (AVIS_DECL (id_avis)) == N_vardec,
                                         "Local avis has no vardec!");
                            vardec = AVIS_DECL (id_avis);
                            VARDEC_AVIS (vardec) = new_avis;
                            AVIS_DECL (new_avis) = vardec;

                            /* update application arguments */
                            ID_AVIS (EXPRS_EXPR (ap_args)) = FREEdoFreeTree (id_avis);
                            ID_AVIS (EXPRS_EXPR (ap_args)) = new_avis;

                            /* update function signature */
                            dev_type = ConvertHost2DeviceType (AVIS_TYPE (arg_avis));
                            new_avis
                              = TBmakeAvis (TRAVtmpVarName ("emr_tmp_dev"), dev_type);

                            ARG_AVIS (fundef_args) = new_avis;
                            AVIS_DECL (new_avis) = fundef_args;

                            /* add N_avis to LUT, and use to replace values within the
                             * function body */
                            INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info),
                                                                     arg_avis, new_avis);
                        }

                        ap_args = EXPRS_NEXT (ap_args);
                        fundef_args = ARG_NEXT (fundef_args);
                    }
                    INFO_WLERCS (arg_info) = FREEdoFreeTree (INFO_WLERCS (arg_info));
                }
            }

            /* now update all N_avis in the fundef */
            DBUG_PRINT ("...going into application fundef");
            old_ap_args = INFO_AP_ARGS (arg_info);
            INFO_AP_ARGS (arg_info) = AP_ARGS (arg_node);
            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
            INFO_AP_ARGS (arg_info) = old_ap_args;

            if (!INFO_UPDATE_AP (arg_info)) {
                /* free all old fundef args */
                INFO_LUT (arg_info) = LUTmapLutP (INFO_LUT (arg_info), FreeLutArgs);
            }
            INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));
            INFO_LUT (arg_info) = old_lut;
        } else if (AP_FUNDEF (arg_node) == INFO_FUNDEF (arg_info)
                   && INFO_UPDATE_AP (arg_info)) { /* is recursive application */
            DBUG_PRINT ("...is recursive call");
            ap_args = INFO_AP_ARGS (arg_info);
            fundef_args = AP_ARGS (arg_node);

            DBUG_ASSERT (INFO_LUT (arg_info) != NULL, "There is no LUT!");

            while (ap_args != NULL) {
                DBUG_ASSERT (fundef_args != NULL,
                             "# of outer Ap args != # of recursive Ap args!");

                arg_avis = ID_AVIS (EXPRS_EXPR (fundef_args));

                new_avis = LUTsearchInLutPp (INFO_LUT (arg_info), arg_avis);
                if (arg_avis != new_avis) {
                    DBUG_PRINT ("......found matching ERC lift %s -> %s",
                                AVIS_NAME (arg_avis), AVIS_NAME (new_avis));
                    ID_AVIS (EXPRS_EXPR (fundef_args)) = new_avis;
                }

                ap_args = EXPRS_NEXT (ap_args);
                fundef_args = EXPRS_NEXT (fundef_args);
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief If we are updating the function argument, replace the ERC with the new device
 *        type function argument
 *
 * @param arg_node N_modarray
 * @param arg_info info struct
 * @return N_modarray
 */
node *
EMRTUmodarray (node *arg_node, info *arg_info)
{
    node *ercs, *erc;

    DBUG_ENTER ();

    if (!INFO_UPDATE_AP (arg_info)) {
        ercs = MODARRAY_ERC (arg_node);
        while (ercs != NULL) {
            erc = LUTsearchInLutPp (INFO_LUT (arg_info), ID_AVIS (EXPRS_EXPR (ercs)));
            if (ID_AVIS (EXPRS_EXPR (ercs)) != erc) {
                DBUG_PRINT ("...found %s, replacing with %s", ID_NAME (EXPRS_EXPR (ercs)),
                            AVIS_NAME (erc));
                ID_AVIS (EXPRS_EXPR (ercs)) = erc;
                break;
            }
            ercs = EXPRS_NEXT (ercs);
        }

        MODARRAY_NEXT (arg_node) = TRAVopt (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief If we are updating the function argument, replace the ERC with the new device
 *        type function argument
 *
 * @param arg_node N_genarray
 * @param arg_info info struct
 * @return N_genarray
 */
node *
EMRTUgenarray (node *arg_node, info *arg_info)
{
    node *ercs, *erc;

    DBUG_ENTER ();

    if (!INFO_UPDATE_AP (arg_info)) {
        ercs = GENARRAY_ERC (arg_node);
        while (ercs != NULL) {
            erc = LUTsearchInLutPp (INFO_LUT (arg_info), ID_AVIS (EXPRS_EXPR (ercs)));
            if (ID_AVIS (EXPRS_EXPR (ercs)) != erc) {
                DBUG_PRINT ("...found %s, replacing with %s", ID_NAME (EXPRS_EXPR (ercs)),
                            AVIS_NAME (erc));
                ID_AVIS (EXPRS_EXPR (ercs)) = erc;
                break;
            }
            ercs = EXPRS_NEXT (ercs);
        }

        GENARRAY_NEXT (arg_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief Store LHS before traversing into RHS
 *
 * @param arg_node N_let
 * @param arg_info info struct
 * @return N_let
 */
node *
EMRTUlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief If we find a host2device primitive, add its LHS and argument to the LUT
 *
 * @param arg_node N_prf
 * @param arg_info info struct
 * @return N_prf
 */
node *
EMRTUprf (node *arg_node, info *arg_info)
{
    node *id_avis, *let_avis;

    DBUG_ENTER ();

    if (INFO_UPDATE_AP (arg_info)) {
        if (PRF_PRF (arg_node) == F_host2device) {
            id_avis = ID_AVIS (PRF_ARG1 (arg_node));
            let_avis = IDS_AVIS (INFO_LETIDS (arg_info));
            DBUG_PRINT ("Found host2device, %s -> %s", AVIS_NAME (id_avis),
                        AVIS_NAME (let_avis));
            DBUG_ASSERT (INFO_LUT (arg_info) != NULL, "There is no LUT!");
            INFO_LUT (arg_info)
              = LUTinsertIntoLutP (INFO_LUT (arg_info), id_avis, let_avis);
        }
    }

    DBUG_RETURN (arg_node);
}
/** @} */
