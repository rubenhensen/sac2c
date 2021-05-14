/*****************************************************************************
 *
 * @defgroup Lift memory transfers in loops whenever possible
 *
 *
 *   This module implements the transformation of lifting memory transfers
 *   (<host2device>/<device2host>) out of a cond-fun. Memory transfers that
 *   are allowed to be moved out were tagged in the previous phase, i.e.
 *   Annotate Memory Transfer (ACTRAN).
 *   The annotations are attached to the corresponding assignments:
 *    - host2device : ASSIGN_ISNOTALLOWEDTOBEMOVEDUP
 *    - device2host : ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN
 *
 *   The transformations perform the following:
 *
 *   for host2device: (can be in either branch, once or multiple times! :-)
 *
 *    rt1, ..., rtn Cond( bool p, a1, ..., type ak, ... am)
 *    {
 *       ...
 *       type_dev ti;
 *       ...
 *       if (p) {
 *          ...
 *          ti = _host2device_(ak); // !ASSIGN_ISNOTALLOWEDTOBEMOVEDUP
 *          ... 
 *       } else {
 *          ...
 *       }
 *       r1 = ( p ? t1 : e1);
 *       ...
 *       rn = ( p ? tn : en);
 *       return (r1, ..., rn);
 *    }
 *
 *   is transformed into:
 *
 *    rt1, ..., rtn Cond( bool p, a1, ..., type_dev ti, ... am)
 *    {
 *       ...
 *       type_dev ti;
 *       ...
 *       if (p) {
 *          ...
 *          ti = ti;
 *          ...
 *       } else {
 *          ...
 *       }
 *       r1 = ( p ? t1 : e1);
 *       ...
 *       rn = ( p ? tn : en);
 *       return (r1, ..., rn);
 *    }
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file minimize_cond_transfers.c
 *
 * Prefix: MCTRAN
 *
 *****************************************************************************/
#include "minimize_cond_transfers.h"

#include <stdlib.h>

#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"
#include "globals.h"

#define DBUG_PREFIX "MCTRAN"
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

enum traverse_mode { trav_normalfun, trav_condfun };

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    bool incondfun;
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
    bool cudastonly;
    bool incudast;
};

#define INFO_INCONDFUN(n) (n->incondfun)
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
#define INFO_CUDASTONLY(n) (n->cudastonly)
#define INFO_INCUDAST(n) (n->incudast)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_INCONDFUN (result) = FALSE;
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
    INFO_INCUDAST (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *MCTRANdoMinimizeCondTransfers( node *syntax_tree)
 *
 *****************************************************************************/
node *
MCTRANdoMinimizeCondTransfers (node *syntax_tree)
{
    info *info;
    DBUG_ENTER ();

    info = MakeInfo ();
    INFO_CUDASTONLY (info) = FALSE;

    TRAVpush (TR_mctran);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *MCTRANdoMinimizeCudastCondTransfers( node *syntax_tree)
 *
 *****************************************************************************/
node *
MCTRANdoMinimizeCudastCondTransfers (node *syntax_tree)
{
    info *info;
    DBUG_ENTER ();

    info = MakeInfo ();
    INFO_CUDASTONLY (info) = TRUE;

    TRAVpush (TR_mctran);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    syntax_tree = DCRdoDeadCodeRemoval (syntax_tree);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *MCTRANfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
MCTRANfundef (node *arg_node, info *arg_info)
{
    bool old_incondfun;

    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;

    /* If the function is not a cond-fun, we traverse as normal */
    if (!FUNDEF_ISCONDFUN (arg_node)) {
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    } else {
        /* If the traverse mode is trav_condfun, we traverse the cond-fun;
         * otherwise we traverse the next N_fundef.
         */
        if (INFO_TRAVMODE (arg_info) == trav_condfun) {
            /* We assign a sequential number (starting from 0)
             * to each argument of the cond-fun */
            INFO_FUNARGNUM (arg_info) = 0;
            FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);

            old_incondfun = INFO_INCONDFUN (arg_info);
            INFO_INCONDFUN (arg_info) = TRUE;
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
            INFO_INCONDFUN (arg_info) = old_incondfun;
        } else {
            FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MCTRANarg( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
MCTRANarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ARG_LINKSIGN (arg_node) = INFO_FUNARGNUM (arg_info);
    INFO_FUNARGNUM (arg_info) += 1;

    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MCTRANassign( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
MCTRANassign (node *arg_node, info *arg_info)
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
            global.optcounters.cuda_min_trans++;
        }

        if (INFO_APPREASSIGNS (arg_info) != NULL) {
            arg_node = TCappendAssign (INFO_APPREASSIGNS (arg_info), arg_node);
            global.optcounters.cuda_min_trans++;
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

/** <!--********************************************************************-->
 *
 * @fn node *MCTRANlet( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
MCTRANlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    INFO_LETIDS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MCTRANcudast( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
MCTRANcudast (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_CUDASTONLY (arg_info)) {
        INFO_INCUDAST (arg_info) = TRUE;
        CUDAST_REGION (arg_node) = TRAVopt (CUDAST_REGION (arg_node), arg_info);
        INFO_INCUDAST (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MCTRANap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
MCTRANap (node *arg_node, info *arg_info)
{
    node *old_fundef;
    node *old_apargs, *old_apids;
    enum traverse_mode old_mode;
    lut_t *old_h2d_lut, *old_d2h_lut;
    bool traverse_cond;

    DBUG_ENTER ();

    traverse_cond = (INFO_CUDASTONLY (arg_info) && INFO_INCUDAST (arg_info))
                    || (!INFO_CUDASTONLY (arg_info) && !INFO_INCUDAST (arg_info));

    /* If the N_ap->N_fundef is a COND-fun */
    if (FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node)) && traverse_cond) {
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
        INFO_TRAVMODE (arg_info) = trav_condfun;
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
    } else {
        AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MCTRANid( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
MCTRANid (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ();

    if (INFO_INCONDFUN (arg_info)) {
        /* If this N_id occurs in a place other than the argument list
         * of a recursive application of the enclosing do-fun, reset its
         * N_avis to the new N_avis. This is necessary when
         * a <host2device> is lifted out of the cond-fun, and therefore
         * the device variable is passed to the cond-fun as an argument
         * instead of a locally declared/defined variable. */
        avis = (node *)LUTsearchInLutPp (INFO_H2DLUT (arg_info), ID_AVIS (arg_node));
        if (avis != ID_AVIS (arg_node)) {
            ID_AVIS (arg_node) = avis;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MCTRANfuncond( node *syntax_tree)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
MCTRANfuncond (node *arg_node, info *arg_info)
{
    node *then_id, *else_id;
    node *let_ids;
    ntype *ids_scalar_type;
    node *then_ssaassign, *else_ssaassign;
    node *then_avis, *else_avis;
    bool covert_ids = FALSE;

    DBUG_ENTER ();

    if (INFO_INCONDFUN (arg_info)) {
        then_id = FUNCOND_THEN (arg_node);
        else_id = FUNCOND_ELSE (arg_node);
        let_ids = INFO_LETIDS (arg_info);

        DBUG_ASSERT (NODE_TYPE (then_id) == N_id,
                     "THEN part of N_funcond must be a N_id node!");
        DBUG_ASSERT (NODE_TYPE (else_id) == N_id,
                     "ELSE part of N_funcond must be a N_id node!");

        then_ssaassign = AVIS_SSAASSIGN (ID_AVIS (then_id));
        else_ssaassign = AVIS_SSAASSIGN (ID_AVIS (else_id));

        if (ISDEVICE2HOST (then_ssaassign)
            && !ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN (then_ssaassign)
            && ISDEVICE2HOST (else_ssaassign)
            && !ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN (else_ssaassign)) {
            then_avis
              = (node *)LUTsearchInLutPp (INFO_D2HLUT (arg_info), ID_AVIS (then_id));
            else_avis
              = (node *)LUTsearchInLutPp (INFO_D2HLUT (arg_info), ID_AVIS (else_id));
            ID_AVIS (then_id) = then_avis;
            ID_AVIS (else_id) = else_avis;
            covert_ids = TRUE;
        } else if (ISDEVICE2HOST (then_ssaassign)
                   && !ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN (then_ssaassign)) {
            DBUG_ASSERT (NODE_TYPE (AVIS_DECL (ID_AVIS (else_id))) == N_arg,
                         "Non N_arg node found!");
            ID_AVIS (else_id) = ARG_AVIS (AVIS_DECL (ID_AVIS (else_id)));
            then_avis
              = (node *)LUTsearchInLutPp (INFO_D2HLUT (arg_info), ID_AVIS (then_id));
            ID_AVIS (then_id) = then_avis;
            covert_ids = TRUE;
        } else if (ISDEVICE2HOST (else_ssaassign)
                   && !ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN (else_ssaassign)) {
            DBUG_ASSERT (NODE_TYPE (AVIS_DECL (ID_AVIS (then_id))) == N_arg,
                         "Non N_arg node found!");
            ID_AVIS (then_id) = ARG_AVIS (AVIS_DECL (ID_AVIS (then_id)));
            else_avis
              = (node *)LUTsearchInLutPp (INFO_D2HLUT (arg_info), ID_AVIS (else_id));
            ID_AVIS (else_id) = else_avis;
            covert_ids = TRUE;
        } else {
            /* Do nothing */
        }

        if (covert_ids) {
            AVIS_NAME (IDS_AVIS (let_ids)) = MEMfree (AVIS_NAME (IDS_AVIS (let_ids)));
            AVIS_NAME (IDS_AVIS (let_ids)) = TRAVtmpVarName ("dev");

            ids_scalar_type = TYgetScalar (AVIS_TYPE (IDS_AVIS (let_ids)));
            TYsetSimpleType (ids_scalar_type, CUh2dSimpleTypeConversion (
                                                TYgetSimpleType (ids_scalar_type)));
        }
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MCTRANreturn( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
MCTRANreturn (node *arg_node, info *arg_info)
{
    node *fun_rets;
    node *ret_exprs;
    node *id;
    node *ap_ids;
    simpletype simty;

    DBUG_ENTER ();

    if (INFO_INCONDFUN (arg_info)) {
        /* N_ret nodes of the current do-fun */
        fun_rets = FUNDEF_RETS (INFO_FUNDEF (arg_info));
        /* One or more return values (N_exprs) */
        ret_exprs = RETURN_EXPRS (arg_node);
        /* N_ids of the application of this cond-fun in the calling context */
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
                ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN (INFO_APPOSTASSIGNS (arg_info)) = TRUE;
            }

            ret_exprs = EXPRS_NEXT (ret_exprs);
            fun_rets = RET_NEXT (fun_rets);
            ap_ids = IDS_NEXT (ap_ids);
        }
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MCTRANprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
MCTRANprf (node *arg_node, info *arg_info)
{
    node *id;

    DBUG_ENTER ();

    if (INFO_INCONDFUN (arg_info)) {
        switch (PRF_PRF (arg_node)) {
        case F_host2device:
            if (!ASSIGN_ISNOTALLOWEDTOBEMOVEDUP ((INFO_LASTASSIGN (arg_info)))) {
                id = PRF_ARG1 (arg_node);
                DBUG_ASSERT (NODE_TYPE (ID_DECL (id)) == N_arg,
                             "Host variable of is not declared as an N_arg!");

                /* We change the argument, e.g. a_host to
                 * device variable, e.g. a_dev */
                node *arg = ID_DECL (id);

                if (CUisDeviceTypeNew (AVIS_TYPE (ARG_AVIS (arg)))) {
                    // arg has been converted already!
                    arg_node = FREEdoFreeTree (arg_node);
                    arg_node = TBmakeId (ARG_AVIS (arg));
                } else {
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
                    node *ap_arg
                      = CUnthApArg (INFO_APARGS (arg_info), ARG_LINKSIGN (arg));
                    DBUG_ASSERT (NODE_TYPE (ap_arg) == N_id,
                                 "Arguments of N_ap must be N_id nodes!");

                    INFO_APPREASSIGNS (arg_info)
                      = TBmakeAssign (TBmakeLet (TBmakeIds (new_avis, NULL),
                                                 TBmakePrf (F_host2device,
                                                            TBmakeExprs (TBmakeId (
                                                                           ID_AVIS (
                                                                             ap_arg)),
                                                                         NULL))),
                                      INFO_APPREASSIGNS (arg_info));

                    /* Replace the N_avis of ap_arg to the new device N_avis */
                    ID_AVIS (ap_arg) = new_avis;
                    /* Maintain SSA property */
                    AVIS_SSAASSIGN (new_avis) = INFO_APPREASSIGNS (arg_info);
                    ASSIGN_ISNOTALLOWEDTOBEMOVEDUP (INFO_APPREASSIGNS (arg_info)) = TRUE;
                }
            }
            break;
        case F_device2host:
            if (!ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN ((INFO_LASTASSIGN (arg_info)))) {
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

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/

#undef DBUG_PREFIX
