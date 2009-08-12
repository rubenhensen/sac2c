/*****************************************************************************
 *
 *
 * file:   annotate_cuda_withloop.c
 *
 * prefix: ACUWL
 *
 * description:
 *
 *
 *****************************************************************************/

#include "minimize_transfers.h"

#include <stdlib.h>

#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"
#include "globals.h"
#include "dbug.h"
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

static bool nochange = TRUE;

enum traverse_mode { trav_normalfun, trav_dofun };

/*
 * INFO structure
 */
struct INFO {
    bool indofun;
    node *letids;
    node *lastassign;
    node *fundef;
    node *apargs;
    node *apids;
    enum traverse_mode travmode;
    lut_t *h2dlut; /* look up table used to store data flow informatino reagarded to
                      host2device */
    lut_t *d2hlut; /* look up table used to store data flow informatino reagarded to
                      device2host */
    node *appreassigns;
    node *appostassigns;
    node *vardecs;
    int funargnum;
    bool funapdone;
    bool issecondapargs; /* traversal of the arguments of the second do-fun application */
    node *secondapargs;  /* traversal of the arguments of the second do-fun application */
    int round;
};

/*
 * INFO macros
 */

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
#define INFO_ISSECONDAPARGS(n) (n->issecondapargs)
#define INFO_SECONDAPARGS(n) (n->secondapargs)
#define INFO_ROUND(n) (n->round)

#define ISHOST2DEVICE(assign)                                                            \
    (assign == NULL                                                                      \
       ? FALSE                                                                           \
       : (NODE_TYPE (ASSIGN_INSTR (assign)) != N_let                                     \
            ? FALSE                                                                      \
            : (NODE_TYPE (ASSIGN_RHS (assign)) != N_prf                                  \
                 ? FALSE                                                                 \
                 : (PRF_PRF (ASSIGN_RHS (assign)) == F_host2device ? TRUE : FALSE))))

#define ISDEVICE2HOST(assign)                                                            \
    (assign == NULL                                                                      \
       ? FALSE                                                                           \
       : (NODE_TYPE (ASSIGN_INSTR (assign)) != N_let                                     \
            ? FALSE                                                                      \
            : (NODE_TYPE (ASSIGN_RHS (assign)) != N_prf                                  \
                 ? FALSE                                                                 \
                 : (PRF_PRF (ASSIGN_RHS (assign)) == F_device2host ? TRUE : FALSE))))

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

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
    INFO_ISSECONDAPARGS (result) = FALSE;
    INFO_SECONDAPARGS (result) = NULL;
    INFO_ROUND (result) = 0;

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
 * @fn
 *
 * @brief node *MLTRANdoMinimizeLoopTransfers( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
MLTRANdoMinimizeLoopTransfers (node *syntax_tree, bool *flag)
// node *MLTRANdoMinimizeLoopTransfers(node *syntax_tree)
{
    info *info;
    DBUG_ENTER ("MLTRANdoMinimizeTransfers");

    static int round = 1;

    info = MakeInfo ();

    INFO_ROUND (info) = round;

    TRAVpush (TR_mltran);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    *flag = nochange;

    // if( round !=2)
    syntax_tree = DCRdoDeadCodeRemovalModule (syntax_tree);

    round++;

    DBUG_RETURN (syntax_tree);
}
/*
node *MLTRANassign(node *arg_node, info *arg_info)
{
  node *preassigns = NULL;
  node *postassigns = NULL;
  bool funapdone = FALSE;
  bool isunused;

  DBUG_ENTER("MLTRANassign");

  INFO_LASTASSIGN( arg_info) = arg_node;

  ASSIGN_INSTR( arg_node) = TRAVdo( ASSIGN_INSTR( arg_node), arg_info);

  isunused = ASSIGN_ISUNUSED( arg_node);

  if( !INFO_INDOFUN( arg_info)) {
    preassigns = INFO_APPREASSIGNS( arg_info);
    postassigns = INFO_APPOSTASSIGNS( arg_info);
    funapdone = INFO_FUNAPDONE( arg_info);
    INFO_APPREASSIGNS( arg_info) = NULL;
    INFO_APPOSTASSIGNS( arg_info) = NULL;
    INFO_FUNAPDONE( arg_info) = FALSE;
  }

  ASSIGN_NEXT( arg_node) = TRAVopt( ASSIGN_NEXT( arg_node), arg_info);

  if( funapdone) {
    if( postassigns != NULL) {
      postassigns = TCappendAssign( postassigns, ASSIGN_NEXT( arg_node));
      arg_node = TCappendAssign( arg_node, postassigns);
      INFO_APPOSTASSIGNS( arg_info) = NULL;
    }

    if( preassigns != NULL) {
      arg_node = TCappendAssign( preassigns, arg_node);
      INFO_APPREASSIGNS( arg_info) = NULL;
    }

    FUNDEF_VARDEC( INFO_FUNDEF( arg_info)) =
        TCappendVardec( INFO_VARDECS( arg_info),
                        FUNDEF_VARDEC( INFO_FUNDEF( arg_info)));
    INFO_FUNAPDONE( arg_info) = FALSE;
  }

  if( isunused) {
    arg_node = FREEdoFreeNode( arg_node);
  }

  DBUG_RETURN( arg_node);
}
*/

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *MLTRANassign( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
MLTRANassign (node *arg_node, info *arg_info)
{
    node *tmp;
    node *next;
    node *preassigns, *postassigns;
    node *vardecs;

    DBUG_ENTER ("MLTRANassign");

    INFO_LASTASSIGN (arg_info) = arg_node;

    preassigns = INFO_APPREASSIGNS (arg_info);
    postassigns = INFO_APPOSTASSIGNS (arg_info);
    vardecs = INFO_VARDECS (arg_info);

    // INFO_APPREASSIGNS( arg_info) = NULL;
    // INFO_APPOSTASSIGNS( arg_info) = NULL;

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    /* If the assign is a funap and we have finished travering the fundef
     * of the funap, we prepend and append the lifted memory transfer
     * instructions and corresponding vardecs.
     */
    if (INFO_FUNAPDONE (arg_info)) {
        next = ASSIGN_NEXT (arg_node);
        ASSIGN_NEXT (arg_node) = NULL;

        if (INFO_APPOSTASSIGNS (arg_info) != NULL) {
            ASSIGN_NEXT (arg_node) = INFO_APPOSTASSIGNS (arg_info);
            // nochange = FALSE;
        }
        INFO_APPOSTASSIGNS (arg_info) = postassigns;

        if (INFO_APPREASSIGNS (arg_info) != NULL) {
            arg_node = TCappendAssign (INFO_APPREASSIGNS (arg_info), arg_node);
            // nochange = FALSE;
        }
        INFO_APPREASSIGNS (arg_info) = preassigns;

        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = TCappendVardec (INFO_VARDECS (arg_info),
                            FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));
        INFO_VARDECS (arg_info) = vardecs;

        INFO_FUNAPDONE (arg_info) = FALSE;

        tmp = arg_node;

        while (ASSIGN_NEXT (tmp) != NULL) {
            tmp = ASSIGN_NEXT (tmp);
        }

        ASSIGN_NEXT (tmp) = next;

        ASSIGN_NEXT (tmp) = TRAVopt (ASSIGN_NEXT (tmp), arg_info);
    } else {
        // INFO_APPREASSIGNS( arg_info) = preassigns;
        // INFO_APPOSTASSIGNS( arg_info) = postassigns;
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *MLTRANfundef( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
MLTRANfundef (node *arg_node, info *arg_info)
{
    bool indofun;
    DBUG_ENTER ("MLTRANfundef");

    INFO_FUNDEF (arg_info) = arg_node;

    /* If the function is not a do-fun, we travers as normal */
    if (!FUNDEF_ISDOFUN (arg_node)) {
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    } else {
        /* If the travers mode is dofun, we travers the do-fun; otherwise
         * we traverse the next function.
         */
        if (INFO_TRAVMODE (arg_info) == trav_dofun) {

            /* We assign a number to each argument of the function */
            INFO_FUNARGNUM (arg_info) = 0;
            FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);

            indofun = INFO_INDOFUN (arg_info);
            INFO_INDOFUN (arg_info) = TRUE;
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
            INFO_INDOFUN (arg_info) = indofun;
        } else {
            FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *MLTRANarg( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
MLTRANarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MLTRANarg");

    ARG_LINKSIGN (arg_node) = INFO_FUNARGNUM (arg_info);
    INFO_FUNARGNUM (arg_info) += 1;

    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *MLTRANap( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
MLTRANap (node *arg_node, info *arg_info)
{
    node *old_fundef;
    node *apargs, *apids;
    enum traverse_mode mode;
    lut_t *h2d_lut, *d2h_lut;

    DBUG_ENTER ("MLTRANap");

    /* If the application is to a do-fun and the application is not
     * in a do-fun. This prevents the do-fun from being traversed
     * for more than once.
     */
    if (FUNDEF_ISDOFUN (AP_FUNDEF (arg_node))) {
        /* If this is an application of do-fun not within a do-fun context */
        if (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info)) {

            /* traverse the function arguments first */
            AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);

            /* Stack info */
            old_fundef = INFO_FUNDEF (arg_info);
            apargs = INFO_APARGS (arg_info);
            apids = INFO_APIDS (arg_info);
            mode = INFO_TRAVMODE (arg_info);
            // done = INFO_FUNAPDONE( arg_info);
            h2d_lut = INFO_H2DLUT (arg_info);
            d2h_lut = INFO_D2HLUT (arg_info);

            INFO_VARDECS (arg_info) = NULL;
            INFO_APPREASSIGNS (arg_info) = NULL;
            INFO_APPOSTASSIGNS (arg_info) = NULL;
            INFO_TRAVMODE (arg_info) = trav_dofun;
            INFO_H2DLUT (arg_info) = LUTgenerateLut ();
            INFO_D2HLUT (arg_info) = LUTgenerateLut ();
            INFO_APARGS (arg_info) = AP_ARGS (arg_node);
            INFO_APIDS (arg_info) = INFO_LETIDS (arg_info);
            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
            INFO_FUNAPDONE (arg_info) = TRUE;

            /* Pop info */
            INFO_H2DLUT (arg_info) = LUTremoveLut (INFO_H2DLUT (arg_info));
            INFO_D2HLUT (arg_info) = LUTremoveLut (INFO_D2HLUT (arg_info));
            INFO_H2DLUT (arg_info) = h2d_lut;
            INFO_D2HLUT (arg_info) = d2h_lut;
            // INFO_FUNAPDONE( arg_info) = done;
            INFO_TRAVMODE (arg_info) = mode;
            INFO_APIDS (arg_info) = apids;
            INFO_APARGS (arg_info) = apargs;
            INFO_FUNDEF (arg_info) = old_fundef;
        }
        /* If this is an application of do-fun within a do-fun
         * (i.e. the recursive call).
         */
        else {
            INFO_ISSECONDAPARGS (arg_info) = TRUE;
            INFO_SECONDAPARGS (arg_info) = AP_ARGS (arg_node);
            AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);
            INFO_SECONDAPARGS (arg_info) = NULL;
            INFO_ISSECONDAPARGS (arg_info) = FALSE;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *MLTRANlet( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
MLTRANlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MLTRANlet");

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    INFO_LETIDS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

static node *
GetMatchingFundefargFromAparg (node *fundefargs, node *apargs, node *aparg)
{
    node *ret_node = NULL;

    DBUG_ENTER ("MLGetMatchingFundefargFromApargTRANlet");
    while (fundefargs != NULL && apargs != NULL) {
        if (EXPRS_EXPR (apargs) == aparg) {
            ret_node = fundefargs;
            break;
        }
        fundefargs = ARG_NEXT (fundefargs);
        apargs = EXPRS_NEXT (apargs);
    }
    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *MLTRANid( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
MLTRANid (node *arg_node, info *arg_info)
{
    node *avis;
    DBUG_ENTER ("MLTRANid");

    if (INFO_INDOFUN (arg_info)) {
        /* Ensure every id has it's avis pointing to the new avis of an argument */
        if (!INFO_ISSECONDAPARGS (arg_info)) {

            avis = LUTsearchInLutPp (INFO_H2DLUT (arg_info), ID_AVIS (arg_node));
            if (avis != ID_AVIS (arg_node)) {
                ID_AVIS (arg_node) = avis;
            }
        } else {
            /* this is an id in the arg lists of the recursive application of the loop
             * function */
            if (ISDEVICE2HOST (AVIS_SSAASSIGN (ID_AVIS (arg_node)))) {
                node *fundefarg
                  = GetMatchingFundefargFromAparg (FUNDEF_ARGS (INFO_FUNDEF (arg_info)),
                                                   INFO_SECONDAPARGS (arg_info),
                                                   arg_node);
                /* we can only set the arg to a dev varaible if and only if the
                   corresponding fundef arg is also a dev variable */
                if (TYgetSimpleType (TYgetScalar (AVIS_TYPE (ARG_AVIS (fundefarg))))
                      == T_float_dev
                    || TYgetSimpleType (TYgetScalar (AVIS_TYPE (ARG_AVIS (fundefarg))))
                         == T_int_dev) {
                    avis = LUTsearchInLutPp (INFO_D2HLUT (arg_info), ID_AVIS (arg_node));
                    ID_AVIS (arg_node) = avis;
                }
            }
            if (NODE_TYPE (AVIS_DECL (ID_AVIS (arg_node))) == N_arg) {
                ID_AVIS (arg_node) = ARG_AVIS (AVIS_DECL (ID_AVIS (arg_node)));
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *MLTRANfuncond( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
MLTRANfuncond (node *arg_node, info *arg_info)
{
    node *then_id, *else_id;
    node *let_ids;
    node *avis;
    ntype *then_scalar_type;
    ntype *ids_scalar_type;

    DBUG_ENTER ("MLTRANfuncond");

    if (INFO_INDOFUN (arg_info)) {
        DBUG_ASSERT ((NODE_TYPE (FUNCOND_THEN (arg_node)) == N_id),
                     "THEN part of funcond should be an id node!");
        DBUG_ASSERT ((NODE_TYPE (FUNCOND_ELSE (arg_node)) == N_id),
                     "ELSE part of funcond should be an id node!");

        then_id = FUNCOND_THEN (arg_node);
        else_id = FUNCOND_ELSE (arg_node);
        let_ids = INFO_LETIDS (arg_info);

        if (ISDEVICE2HOST (AVIS_SSAASSIGN (ID_AVIS (else_id)))) {
            avis = LUTsearchInLutPp (INFO_D2HLUT (arg_info), ID_AVIS (else_id));
            if (avis != ID_AVIS (else_id)) {
                ID_AVIS (else_id) = avis;

                AVIS_NAME (ID_AVIS (then_id)) = MEMfree (AVIS_NAME (ID_AVIS (then_id)));
                AVIS_NAME (IDS_AVIS (let_ids)) = MEMfree (AVIS_NAME (IDS_AVIS (let_ids)));

                AVIS_NAME (ID_AVIS (then_id)) = TRAVtmpVarName ("dev");
                AVIS_NAME (IDS_AVIS (let_ids)) = TRAVtmpVarName ("dev");

                then_scalar_type = TYgetScalar (AVIS_TYPE (ID_AVIS (then_id)));
                ids_scalar_type = TYgetScalar (AVIS_TYPE (IDS_AVIS (let_ids)));
                switch (TYgetSimpleType (then_scalar_type)) {
                case T_float:
                    then_scalar_type = TYsetSimpleType (then_scalar_type, T_float_dev);
                    ids_scalar_type = TYsetSimpleType (ids_scalar_type, T_float_dev);
                    break;
                case T_int:
                    then_scalar_type = TYsetSimpleType (then_scalar_type, T_int_dev);
                    ids_scalar_type = TYsetSimpleType (ids_scalar_type, T_int_dev);
                    break;
                default:
                    break;
                }
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *MLTRANreturn( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
MLTRANreturn (node *arg_node, info *arg_info)
{
    node *fun_rets;
    node *ret_exprs;
    node *id;
    node *ap_ids;
    simpletype simty;

    DBUG_ENTER ("MLTRANreturn");

    if (INFO_INDOFUN (arg_info)) {
        fun_rets = FUNDEF_RETS (INFO_FUNDEF (arg_info));
        ret_exprs = RETURN_EXPRS (arg_node);
        ap_ids = INFO_APIDS (arg_info);

        while (ret_exprs != NULL && fun_rets != NULL && ap_ids != NULL) {
            id = EXPRS_EXPR (ret_exprs);
            DBUG_ASSERT ((NODE_TYPE (id) == N_id), "Return value must be an id node!");
            simty = TYgetSimpleType (TYgetScalar (AVIS_TYPE (ID_AVIS (id))));
            TYsetSimpleType (TYgetScalar (RET_TYPE (fun_rets)), simty);

            if ((simty == T_int_dev || simty == T_float_dev)
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
                nochange = FALSE;
            }

            ret_exprs = EXPRS_NEXT (ret_exprs);
            fun_rets = RET_NEXT (fun_rets);
            ap_ids = IDS_NEXT (ap_ids);
        }
    }

    DBUG_RETURN (arg_node);
}

static node *
nthApArg (node *args, int n)
{
    int i = 0;
    node *tmp = args;

    DBUG_ENTER ("nthApArg");

    while (i < n) {
        tmp = EXPRS_NEXT (tmp);
        i++;
    }

    tmp = EXPRS_EXPR (tmp);
    DBUG_RETURN (tmp);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *MLTRANprf( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
MLTRANprf (node *arg_node, info *arg_info)
{
    node *id;
    DBUG_ENTER ("MLTRANprf");

    if (INFO_INDOFUN (arg_info)) {
        switch (PRF_PRF (arg_node)) {
        case F_host2device:
            if (!ASSIGN_ISNOTALLOWEDTOBEMOVEDUP ((INFO_LASTASSIGN (arg_info)))) {
                id = PRF_ARG1 (arg_node);
                if (NODE_TYPE (ID_DECL (id)) == N_arg) {

                    node *arg = ID_DECL (id);
                    node *vardec = IDS_DECL (INFO_LETIDS (arg_info));
                    ARG_AVIS (arg) = DUPdoDupNode (VARDEC_AVIS (vardec));
                    AVIS_SSAASSIGN (ARG_AVIS (arg)) = NULL;
                    AVIS_DECL (ARG_AVIS (arg)) = arg;
                    /* Insert vardec_avis -> arg_avis into table.Therefore, any
                     * subsequent reference to the original vardec id will be
                     * replaced by the arg avis
                     */
                    INFO_H2DLUT (arg_info)
                      = LUTinsertIntoLutP (INFO_H2DLUT (arg_info), VARDEC_AVIS (vardec),
                                           ARG_AVIS (arg));

                    /* create vardec and host2device prf in the calling context */
                    node *new_avis = DUPdoDupNode (ARG_AVIS (arg));
                    INFO_VARDECS (arg_info)
                      = TBmakeVardec (new_avis, INFO_VARDECS (arg_info));

                    node *ap_arg = nthApArg (INFO_APARGS (arg_info), ARG_LINKSIGN (arg));
                    DBUG_ASSERT ((NODE_TYPE (ap_arg) == N_id),
                                 "Non id node found in funap arguments");

                    INFO_APPREASSIGNS (arg_info)
                      = TBmakeAssign (TBmakeLet (TBmakeIds (new_avis, NULL),
                                                 TBmakePrf (F_host2device,
                                                            TBmakeExprs (TBmakeId (
                                                                           ID_AVIS (
                                                                             ap_arg)),
                                                                         NULL))),
                                      INFO_APPREASSIGNS (arg_info));
                    ID_AVIS (ap_arg) = new_avis;

                    AVIS_SSAASSIGN (new_avis) = INFO_APPREASSIGNS (arg_info);
                    nochange = FALSE;
                }
            }
            break;
        case F_device2host:
            INFO_D2HLUT (arg_info) = LUTinsertIntoLutP (INFO_D2HLUT (arg_info),
                                                        IDS_AVIS (INFO_LETIDS (arg_info)),
                                                        ID_AVIS (PRF_ARG1 (arg_node)));
            break;
        default:
            PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
            break;
        }
    }

    DBUG_RETURN (arg_node);
}
