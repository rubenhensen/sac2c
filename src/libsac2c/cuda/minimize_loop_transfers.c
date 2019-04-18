/**
 * @file
 * @defgroup mltran Minimize Loop Transfers
 * @ingroup cuda
 *
 * @brief Lift memory transfers in loops whenever possible
 *
 * This module implements the transformation of lifting memory transfers
 * (<host2device>/<device2host>) out of a do-fun. Memory transfers that
 * are allowed to be moved out were tagged in the previous phase, i.e.
 * Annotate Memory Transfer (AMTRAN).
 *
 * @{
 */
#include "minimize_loop_transfers.h"

#include <stdlib.h>

#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"
#include "globals.h"

#define DBUG_PREFIX "MLTRAN"
#include "debug.h"

#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "print.h"
#include "DataFlowMask.h"
#include "NameTuplesUtils.h"
#include "scheduling.h"
#include "wl_bounds.h"
#include "new_types.h"
#include "user_types.h"
#include "shape.h"
#include "LookUpTable.h"
#include "convert.h"
#include "math_utils.h"
#include "types.h"
#include "deadcoderemoval.h"
#include "cuda_utils.h"
#include "pattern_match.h"

enum traverse_mode { trav_normalfun, trav_dofun };

/**
 * @name INFO structure
 * @{
 */
struct INFO {
    bool indofun;
    node *letids;
    node *lastassign;
    node *fundef;
    node *apargs;
    node *apids;
    enum traverse_mode travmode;
    lut_t *h2dlut;
    lut_t *d2hlut;
    node *appreassigns;
    node *appostassigns;
    node *vardecs;
    int funargnum;
    bool funapdone;
    bool isrecursiveapargs;
    node *recursiveapargs;
};

#define INFO_INDOFUN(n) (n->indofun)
#define INFO_LETIDS(n) (n->letids)
#define INFO_LASTASSIGN(n) (n->lastassign)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_TRAVMODE(n) (n->travmode)
#define INFO_APARGS(n) (n->apargs)
#define INFO_APIDS(n) (n->apids)
#define INFO_H2DLUT(n) (n->h2dlut)
#define INFO_D2HLUT(n) (n->d2hlut)
#define INFO_APPREASSIGNS(n) (n->appreassigns)
#define INFO_APPOSTASSIGNS(n) (n->appostassigns)
#define INFO_VARDECS(n) (n->vardecs)
#define INFO_FUNARGNUM(n) (n->funargnum)
#define INFO_FUNAPDONE(n) (n->funapdone)
#define INFO_ISRECURSIVEAPARGS(n) (n->isrecursiveapargs)
#define INFO_RECURSIVEAPARGS(n) (n->recursiveapargs)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_INDOFUN (result) = FALSE;
    INFO_LETIDS (result) = NULL;
    INFO_LASTASSIGN (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_APARGS (result) = NULL;
    INFO_APIDS (result) = NULL;
    INFO_TRAVMODE (result) = trav_normalfun;
    INFO_H2DLUT (result) = NULL;
    INFO_D2HLUT (result) = NULL;
    INFO_APPREASSIGNS (result) = NULL;
    INFO_APPOSTASSIGNS (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_FUNARGNUM (result) = 0;
    INFO_FUNAPDONE (result) = FALSE;
    INFO_ISRECURSIVEAPARGS (result) = FALSE;
    INFO_RECURSIVEAPARGS (result) = NULL;

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

/**
 * @name Entry functions
 * @{
 */

/**
 * @brief
 * @param syntax_tree
 * @return syntax tree
 */
node *
MLTRANdoMinimizeLoopTransfers (node *syntax_tree)
{
    info *info;
    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_mltran);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_PRINT ("invoking DCR");
    syntax_tree = DCRdoDeadCodeRemoval (syntax_tree);

    DBUG_RETURN (syntax_tree);
}

/** @} */

/**
 * @name Traversal functions
 * @{
 */

/**
 * @brief
 *
 * @param arg_node N_fundef
 * @param arg_info info structure
 * @return N_fundef
 */
node *
MLTRANfundef (node *arg_node, info *arg_info)
{
    bool old_indofun;

    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;

    /* If the function is not a do-fun, we traverse as normal */
    if (!FUNDEF_ISLOOPFUN (arg_node)) {
        DBUG_PRINT ("(not LOOP) Entering %s...", FUNDEF_NAME (arg_node));
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    } else {
        /* If the traverse mode is trav_dofun, we traverse the do-fun;
         * otherwise we traverse the next N_fundef.
         */
        if (INFO_TRAVMODE (arg_info) == trav_dofun) {
            DBUG_PRINT ("(LOOP) Entering %s...", FUNDEF_NAME (arg_node));

            /* We assign a sequential number (starting from 0)
             * to each argument of the do-fun */
            INFO_FUNARGNUM (arg_info) = 0;
            FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);

            old_indofun = INFO_INDOFUN (arg_info);
            INFO_INDOFUN (arg_info) = TRUE;
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
            INFO_INDOFUN (arg_info) = old_indofun;
        } else {
            FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief
 *
 * @param arg_node N_arg
 * @param arg_info info structure
 * @return N_arg
 */
node *
MLTRANarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ARG_LINKSIGN (arg_node) = INFO_FUNARGNUM (arg_info);
    INFO_FUNARGNUM (arg_info) += 1;

    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief
 *
 * @param arg_node N_assign
 * @param arg_info info structure
 * @return N_assign
 */
node *
MLTRANassign (node *arg_node, info *arg_info)
{
    node *new_lastassign;
    node *old_next;
    node *old_preassigns, *old_postassigns;
    node *old_vardecs;

    DBUG_ENTER ();

    INFO_LASTASSIGN (arg_info) = arg_node;

    /* Stack info */
    old_preassigns = INFO_APPREASSIGNS (arg_info);
    old_postassigns = INFO_APPOSTASSIGNS (arg_info);
    old_vardecs = INFO_VARDECS (arg_info);

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    /* If the RHS of this N_assign is an N_ap and we have finished traversing
     * the N_ap->N_fundef, add lifted memory transfer instructions (if
     * there's any) to a assigned chain and also any new N_vardec nodes. */
    if (INFO_FUNAPDONE (arg_info)) {
        old_next = ASSIGN_NEXT (arg_node);
        ASSIGN_NEXT (arg_node) = NULL;

        if (INFO_APPOSTASSIGNS (arg_info) != NULL) {
            ASSIGN_NEXT (arg_node) = INFO_APPOSTASSIGNS (arg_info);
            global.optcounters.cuda_min_trans+=1;
        }

        if (INFO_APPREASSIGNS (arg_info) != NULL) {
            arg_node = TCappendAssign (INFO_APPREASSIGNS (arg_info), arg_node);
            global.optcounters.cuda_min_trans+=1;
        }

        FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
          = TCappendVardec (INFO_VARDECS (arg_info),
                            FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

        /* Pop info */
        INFO_APPOSTASSIGNS (arg_info) = old_postassigns;
        INFO_APPREASSIGNS (arg_info) = old_preassigns;
        INFO_VARDECS (arg_info) = old_vardecs;
        INFO_FUNAPDONE (arg_info) = FALSE;

        /* Find the last N_assign after the memory transfers have been added */
        new_lastassign = arg_node;
        while (ASSIGN_NEXT (new_lastassign) != NULL) {
            new_lastassign = ASSIGN_NEXT (new_lastassign);
        }
        ASSIGN_NEXT (new_lastassign) = old_next;

        ASSIGN_NEXT (new_lastassign) = TRAVopt (ASSIGN_NEXT (new_lastassign), arg_info);
    } else {
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief
 *
 * @param arg_node N_let
 * @param arg_info info structure
 * @return N_let
 */
node *
MLTRANlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    INFO_LETIDS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/**
 * @brief
 *
 * @param arg_node N_ap
 * @param arg_info info structure
 * @return N_ap
 */
node *
MLTRANap (node *arg_node, info *arg_info)
{
    node *old_fundef;
    node *old_apargs, *old_apids;
    enum traverse_mode old_mode;
    lut_t *old_h2d_lut, *old_d2h_lut;

    DBUG_ENTER ();

    DBUG_PRINT ("ap %s", FUNDEF_NAME (AP_FUNDEF (arg_node)));

    /* If the N_ap->N_fundef is a do-fun */
    if (FUNDEF_ISLOOPFUN (AP_FUNDEF (arg_node))) {
        /* If this is NOT a recursive application of the enclosing do-fun */
        if (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info)) {
            DBUG_PRINT ("...non-recursive application");

            /* Traverse the N_ap arguments first */
            AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);

            /* Stack info */
            old_fundef = INFO_FUNDEF (arg_info);
            old_apargs = INFO_APARGS (arg_info);
            old_apids = INFO_APIDS (arg_info);
            old_mode = INFO_TRAVMODE (arg_info);
            old_h2d_lut = INFO_H2DLUT (arg_info);
            old_d2h_lut = INFO_D2HLUT (arg_info);

            INFO_VARDECS (arg_info) = NULL;
            INFO_APPREASSIGNS (arg_info) = NULL;
            INFO_APPOSTASSIGNS (arg_info) = NULL;
            INFO_TRAVMODE (arg_info) = trav_dofun;
            INFO_H2DLUT (arg_info) = LUTgenerateLut ();
            INFO_D2HLUT (arg_info) = LUTgenerateLut ();
            INFO_APARGS (arg_info) = AP_ARGS (arg_node);
            /* N_ids of the do-fun application in the calling context */
            INFO_APIDS (arg_info) = INFO_LETIDS (arg_info);

            /* Traverse the N_ap->N_fundef */
            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
            INFO_FUNAPDONE (arg_info) = TRUE;

            /* Pop info */
            INFO_H2DLUT (arg_info) = LUTremoveLut (INFO_H2DLUT (arg_info));
            INFO_D2HLUT (arg_info) = LUTremoveLut (INFO_D2HLUT (arg_info));
            INFO_H2DLUT (arg_info) = old_h2d_lut;
            INFO_D2HLUT (arg_info) = old_d2h_lut;
            INFO_TRAVMODE (arg_info) = old_mode;
            INFO_APIDS (arg_info) = old_apids;
            INFO_APARGS (arg_info) = old_apargs;
            INFO_FUNDEF (arg_info) = old_fundef;
        }
        /* If this is a recursive application of the enclosing do-fun. */
        else {
            DBUG_PRINT ("...recursive application");
            INFO_ISRECURSIVEAPARGS (arg_info) = TRUE;
            INFO_RECURSIVEAPARGS (arg_info) = AP_ARGS (arg_node);
            AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);
            INFO_RECURSIVEAPARGS (arg_info) = NULL;
            INFO_ISRECURSIVEAPARGS (arg_info) = FALSE;
        }
    } else {
        AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/**
 * @brief
 *
 * @param arg_node N_id
 * @param arg_info info structure
 * @return N_id
 */
node *
MLTRANid (node *arg_node, info *arg_info)
{
    node *avis, *ssaassign;

    DBUG_ENTER ();

    if (INFO_INDOFUN (arg_info)) {
        /* If this N_id occurs in a place other than the argument list
         * of a recursive application of the enclosing do-fun, reset its
         * N_avis to the new N_avis. This is necessary when
         * a <host2device> is lifted out of the do-fun, and therefore
         * the device variable is passed to the do-fun as an argument
         * instead of a locally declared/defined variable. */
        if (!INFO_ISRECURSIVEAPARGS (arg_info)) {
            avis = (node *)LUTsearchInLutPp (INFO_H2DLUT (arg_info), ID_AVIS (arg_node));
            if (avis != ID_AVIS (arg_node)) {
                ID_AVIS (arg_node) = avis;
            }
        } else {
            /* If this N_id occurs in the argument list of the
             * recursive application of the enclosing do-fun. */

            ssaassign = AVIS_SSAASSIGN (ID_AVIS (arg_node));
            if (ISDEVICE2HOST (ssaassign)
                && !ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN (ssaassign)) {
                /* If the SSA of this argument is <device2host>, and this
                 * <device2host> can be moved out of the do-fun. */
                avis
                  = (node *)LUTsearchInLutPp (INFO_D2HLUT (arg_info), ID_AVIS (arg_node));
                ID_AVIS (arg_node) = avis;
            }

            /**************** Ugly Code !!! **********************/
            /*
                  pattern *pat;
                  pat = PMprf( 1, PMAisPrf( F_device2host),
                               1, PMvar( 0, 0));

                  if( PMmatchFlat( pat, arg_node)) {
                    ssaassign = AVIS_SSAASSIGN( ID_AVIS( arg_node));
                    node *rhs = ASSIGN_RHS( ssaassign);
                    while( NODE_TYPE( rhs) != N_prf) {
                     DBUG_ASSERT( NODE_TYPE( rhs) == N_id, "Non-id node found!");
                     ssaassign = AVIS_SSAASSIGN( ID_AVIS( rhs));
                     rhs = ASSIGN_RHS( ssaassign);
                    }
                    if( !ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN( ssaassign)) {
                      // If the SSA of this argument is <device2host>, and this
                      // <device2host> can be moved out of the do-fun.
                      ID_AVIS( arg_node) = ID_AVIS( PRF_ARG1( ASSIGN_RHS( ssaassign)));
                    }
                  }
                  pat = PMfree( pat);
            */

            /*****************************************************/

            /* If the N_ap argument is also a N_fundef argument, we set its
             * avis to the N_fundef argument's avis. This has not effect when both
             * arguments are host variables. However, since the argument of
             * the N_fundef might changed to a device variable due to an earlier
             * <host2device>, resetting the avis ensures that we pass the
             * right argument to the recursive do-fun application. */
            if (NODE_TYPE (AVIS_DECL (ID_AVIS (arg_node))) == N_arg) {
                ID_AVIS (arg_node) = ARG_AVIS (AVIS_DECL (ID_AVIS (arg_node)));
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief
 *
 * @param arg_node N_funcond
 * @param arg_info info structure
 * @return N_funcond
 */
node *
MLTRANfuncond (node *arg_node, info *arg_info)
{
    node *then_id, *else_id;
    node *let_ids;
    node *avis;
    ntype *then_scalar_type, *ids_scalar_type;
    node *ssaassign;

    DBUG_ENTER ();

    if (INFO_INDOFUN (arg_info)) {
        then_id = FUNCOND_THEN (arg_node);
        else_id = FUNCOND_ELSE (arg_node);
        let_ids = INFO_LETIDS (arg_info);

        DBUG_ASSERT (NODE_TYPE (then_id) == N_id,
                     "THEN part of N_funcond must be a N_id node!");
        DBUG_ASSERT (NODE_TYPE (else_id) == N_id,
                     "ELSE part of N_funcond must be a N_id node!");

        /* The 'then' part of a N_funcond is result returned from the recursive
         * application of the do-fun and the 'else' part is result computed
         * within the do-fun. */
        ssaassign = AVIS_SSAASSIGN (ID_AVIS (else_id));
        if ((ISDEVICE2HOST (ssaassign)
             && !ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN (ssaassign))) {

            avis = (node *)LUTsearchInLutPp (INFO_D2HLUT (arg_info), ID_AVIS (else_id));
            if (avis != ID_AVIS (else_id)) {

                /* If the 'else' part is a host variable defined by <device2host>,
                 * it can be replaced by the corresponding device varaible. */
                ID_AVIS (else_id) = avis;

                /* Also change the 'then' part and N_ids to device type variables */
                AVIS_NAME (ID_AVIS (then_id)) = MEMfree (AVIS_NAME (ID_AVIS (then_id)));
                AVIS_NAME (IDS_AVIS (let_ids)) = MEMfree (AVIS_NAME (IDS_AVIS (let_ids)));
                AVIS_NAME (ID_AVIS (then_id)) = TRAVtmpVarName ("dev");
                AVIS_NAME (IDS_AVIS (let_ids)) = TRAVtmpVarName ("dev");

                then_scalar_type = TYgetScalar (AVIS_TYPE (ID_AVIS (then_id)));
                TYsetSimpleType (then_scalar_type, CUh2dSimpleTypeConversion (
                                                     TYgetSimpleType (then_scalar_type)));

                ids_scalar_type = TYgetScalar (AVIS_TYPE (IDS_AVIS (let_ids)));
                TYsetSimpleType (ids_scalar_type, CUh2dSimpleTypeConversion (
                                                    TYgetSimpleType (ids_scalar_type)));
            }
        }
        /* This branch is added to take care of the case when the else
         * id is a argument of the function (See bug in loop14.sac)*/
        else if (NODE_TYPE (AVIS_DECL (ID_AVIS (else_id))) == N_arg
                 && CUisDeviceTypeNew (
                      AVIS_TYPE (ARG_AVIS (AVIS_DECL (ID_AVIS (else_id)))))
                 && !CUisDeviceTypeNew (AVIS_TYPE (ID_AVIS (then_id)))) {
            ID_AVIS (else_id) = ARG_AVIS (AVIS_DECL (ID_AVIS (else_id)));

            /* Also change the 'then' part and N_ids to device type variables */
            AVIS_NAME (ID_AVIS (then_id)) = MEMfree (AVIS_NAME (ID_AVIS (then_id)));
            AVIS_NAME (IDS_AVIS (let_ids)) = MEMfree (AVIS_NAME (IDS_AVIS (let_ids)));
            AVIS_NAME (ID_AVIS (then_id)) = TRAVtmpVarName ("dev");
            AVIS_NAME (IDS_AVIS (let_ids)) = TRAVtmpVarName ("dev");

            then_scalar_type = TYgetScalar (AVIS_TYPE (ID_AVIS (then_id)));
            TYsetSimpleType (then_scalar_type, CUh2dSimpleTypeConversion (
                                                 TYgetSimpleType (then_scalar_type)));

            ids_scalar_type = TYgetScalar (AVIS_TYPE (IDS_AVIS (let_ids)));
            TYsetSimpleType (ids_scalar_type, CUh2dSimpleTypeConversion (
                                                TYgetSimpleType (ids_scalar_type)));
        }
    }
    DBUG_RETURN (arg_node);
}

/**
 * @brief
 *
 * @param arg_node N_return
 * @param arg_info info structure
 * @return N_return
 */
node *
MLTRANreturn (node *arg_node, info *arg_info)
{
    node *fun_rets;
    node *ret_exprs;
    node *id;
    node *ap_ids;
    simpletype simty;

    DBUG_ENTER ();

    if (INFO_INDOFUN (arg_info)) {
        /* N_ret nodes of the current do-fun */
        fun_rets = FUNDEF_RETS (INFO_FUNDEF (arg_info));
        /* One or more return values (N_exprs) */
        ret_exprs = RETURN_EXPRS (arg_node);
        /* N_ids of the application of this do-fun in the calling context */
        ap_ids = INFO_APIDS (arg_info);

        while (ret_exprs != NULL && fun_rets != NULL && ap_ids != NULL) {
            id = EXPRS_EXPR (ret_exprs);
            DBUG_ASSERT (NODE_TYPE (id) == N_id, "Return value must be a N_id node!");
            /* Set the simple type of N_ret to the simple type of the
             * corresponding return value (N_id). */
            simty = TYgetSimpleType (TYgetScalar (AVIS_TYPE (ID_AVIS (id))));
            TYsetSimpleType (TYgetScalar (RET_TYPE (fun_rets)), simty);

            /* If the return value is of device type while the corresponding
             * N_ids in the calling context is not, we need to insert a
             * <device2host> memory transfer after the do-fun application
             * in the calling context. */
            if (CUisDeviceTypeNew (AVIS_TYPE (ID_AVIS (id)))
                && !TYeqTypes (AVIS_TYPE (IDS_AVIS (ap_ids)), AVIS_TYPE (ID_AVIS (id)))) {
                node *new_avis = DUPdoDupNode (IDS_AVIS (ap_ids));

                AVIS_NAME (new_avis) = MEMfree (AVIS_NAME (new_avis));
                AVIS_NAME (new_avis) = TRAVtmpVarName ("dev");

                INFO_VARDECS (arg_info)
                  = TBmakeVardec (new_avis, INFO_VARDECS (arg_info));

                TYsetSimpleType (TYgetScalar (AVIS_TYPE (new_avis)), simty);
                INFO_APPOSTASSIGNS (arg_info)
                  = TBmakeAssign (TBmakeLet (TBmakeIds (IDS_AVIS (ap_ids), NULL),
                                             TBmakePrf (F_device2host,
                                                        TBmakeExprs (TBmakeId (new_avis),
                                                                     NULL))),
                                  INFO_APPOSTASSIGNS (arg_info));
                AVIS_SSAASSIGN (IDS_AVIS (ap_ids)) = INFO_APPOSTASSIGNS (arg_info);
                IDS_AVIS (ap_ids) = new_avis;
            }

            ret_exprs = EXPRS_NEXT (ret_exprs);
            fun_rets = RET_NEXT (fun_rets);
            ap_ids = IDS_NEXT (ap_ids);
        }
    }
    DBUG_RETURN (arg_node);
}

/**
 * @brief
 *
 * @param arg_node N_prf
 * @param arg_info info structure
 * @return N_prf
 */
node *
MLTRANprf (node *arg_node, info *arg_info)
{
    node *id;

    DBUG_ENTER ();

    if (INFO_INDOFUN (arg_info)) {
        switch (PRF_PRF (arg_node)) {
        case F_host2device:
            id = PRF_ARG1 (arg_node);
            DBUG_PRINT ("prf host2device %s -> %s", ID_NAME (id), IDS_NAME (INFO_LETIDS (arg_info)));
            if (!ASSIGN_ISNOTALLOWEDTOBEMOVEDUP ((INFO_LASTASSIGN (arg_info)))) {
                DBUG_PRINT ("...can be moved up");
                DBUG_ASSERT (NODE_TYPE (ID_DECL (id)) == N_arg,
                             "Host variable of H2D is not declared as an N_arg!");
                /* If the <host2device> is allowed to be moved out of the do-fun,
                 * the host variable argument can be replaced by the device variable.
                 * Note that if <host2device> can be moved out, the host variable
                 * must be an argument passed to the do-fun. This is garuanteed by
                 * the previous phase: Annotate Memory Transfers (AMTRAN).
                 * e.g
                 *
                 *   loop_fun(..., a_host, ...) {
                 *     ...
                 *     a_dev = host2device( a_host); (Can be moved out)
                 *     ...
                 *   }
                 *
                 *   ==>
                 *
                 *   a_dev = host2device( a_host);
                 *   loop_fun(..., a_dev, ...) {
                 *     ...
                 *     ...
                 *   }
                 *
                 */
                /* We change the argument, e.g. a_host to
                 * device variable, e.g. a_dev */
                node *arg = ID_DECL (id);
                node *vardec = IDS_DECL (INFO_LETIDS (arg_info));
                ARG_AVIS (arg) = DUPdoDupNode (VARDEC_AVIS (vardec));
                AVIS_SSAASSIGN (ARG_AVIS (arg)) = NULL;
                AVIS_DECL (ARG_AVIS (arg)) = arg;

                /* Insert pair [N_vardec->avis] -> [N_arg->avis] into H2D
                 * table. Therefore, N_vardec->avis of any subsequent N_id
                 * nodes will be replaced by N_arg->avis. */
                INFO_H2DLUT (arg_info)
                  = LUTinsertIntoLutP (INFO_H2DLUT (arg_info), VARDEC_AVIS (vardec),
                                       ARG_AVIS (arg));

                /* Create N_vardec and <host2device> in the calling context
                 * i.e. lifting the <host2device> */
                node *new_avis = DUPdoDupNode (ARG_AVIS (arg));
                INFO_VARDECS (arg_info)
                  = TBmakeVardec (new_avis, INFO_VARDECS (arg_info));

                /* Get the argument of the N_ap in calling context corresponding
                 * to the N_arg of the N_fundef being replaced. Since we've already
                 * assigned each N_arg a sequential number in the Linksign field,
                 * finding the N_ap argument at the same position is easy. */
                node *ap_arg = CUnthApArg (INFO_APARGS (arg_info), ARG_LINKSIGN (arg));
                DBUG_ASSERT (NODE_TYPE (ap_arg) == N_id,
                             "Arguments of N_ap must be N_id nodes!");

                INFO_APPREASSIGNS (arg_info)
                  = TBmakeAssign (TBmakeLet (TBmakeIds (new_avis, NULL),
                                             TBmakePrf (F_host2device,
                                                        TBmakeExprs (TBmakeId (
                                                                       ID_AVIS (ap_arg)),
                                                                     NULL))),
                                  INFO_APPREASSIGNS (arg_info));

                /* Replace the N_avis of ap_arg to the new device N_avis */
                ID_AVIS (ap_arg) = new_avis;
                /* Maintain SSA property */
                AVIS_SSAASSIGN (new_avis) = INFO_APPREASSIGNS (arg_info);
            }
            break;
        case F_device2host:
            DBUG_PRINT ("prf device2host");
            if (!ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN ((INFO_LASTASSIGN (arg_info)))) {
                DBUG_PRINT ("...can be moved down");
                /* We insert the pair [N_id(host)->avis] -> [N_id(device)->avis]
                 * into D2H table. */
                INFO_D2HLUT (arg_info)
                  = LUTinsertIntoLutP (INFO_D2HLUT (arg_info),
                                       IDS_AVIS (INFO_LETIDS (arg_info)),
                                       ID_AVIS (PRF_ARG1 (arg_node)));
            }
            break;
        default:
            PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
            break;
        }
    }
    DBUG_RETURN (arg_node);
}

/** @} */
/** @} */
#undef DBUG_PREFIX
