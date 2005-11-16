/* *
 * $Log$
 *
 */

/**<!--******************************************************************-->
 *
 * @file wlpropagation.c
 *
 * This file implements functionality of type upgrade (infer types of lhs
 * dependent on rhs), reverse type upgrade (infer types of rhs dependent
 * on lhs), function dispatch (removes calls of wrapper functions) and
 * function specialization (create more special function instances).
 *
 **************************************************************************/

#include "tree_basic.h"
#include "traverse.h"
#include "node_basic.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "dbug.h"
#include "internal_lib.h"
#include "free.h"
#include "DupTree.h"
#include "globals.h"
#include "type_utils.h"
#include "ct_with.h"
#include "type_errors.h"
#include "ct_prf.h"
#include "ct_with.h"
#include "ct_fun.h"
#include "constants.h"
#include "shape.h"
#include "ct_basic.h"
#include "tree_compound.h"
#include "ctinfo.h"
#include "inferneedcounters.h"
#include "wlselcount.h"
#include "LookUpTable.h"
#include "DataFlowMask.h"
#include "InferDFMs.h"

#include "wlpropagation.h"

typedef enum { S_undef, S_withloop_prop } travstate;

/*
 * info structure
 */
struct INFO {
    node *fundef;
    bool isdofun;
    node *assign;
    travstate travstate;
    node *ap;
    node *corr;
    int argnum;
};

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_ISDOFUN(n) (n->isdofun)
#define INFO_ASSIGN(n) (n->assign)
#define INFO_TRAVSTATE(n) (n->travstate)
#define INFO_AP(n) (n->ap)
#define INFO_CORRESPONDINGFUNARG(n) (n->corr)
#define INFO_ARGNUM(n) (n->argnum)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_ISDOFUN (result) = FALSE;
    INFO_ASSIGN (result) = NULL;
    INFO_TRAVSTATE (result) = S_undef;
    INFO_AP (result) = NULL;
    INFO_CORRESPONDINGFUNARG (result) = NULL;
    INFO_ARGNUM (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *arg_info)
{
    DBUG_ENTER ("FreeInfo");

    arg_info = ILIBfree (arg_info);

    return (arg_info);
}

static bool
ArgIsDefinedByWL (node *arg_node)
{
    bool result = FALSE;
    node *tmp;

    DBUG_ENTER ("ArgIsDefinedByWL");

    tmp = AVIS_SSAASSIGN (ID_AVIS (arg_node));
    if (NULL != tmp) {
        if (N_with == NODE_TYPE (LET_EXPR (ASSIGN_INSTR (tmp)))) {
            result = TRUE;
        }
    }

    if (result) {
        DBUG_PRINT ("WLPROP", ("Argument is defined by WL"));
    } else {
        DBUG_PRINT ("WLPROP", ("Argument is not defined by WL"));
    }
    DBUG_RETURN (result);
}

static node *
GetRecursiveFunctionApplication (node *fundef)
{
    node *chain;
    DBUG_ENTER ("GetRecursiveFunctionApplication");

    chain = BLOCK_INSTR (FUNDEF_BODY (fundef));

    while ((chain != NULL) && (NODE_TYPE (ASSIGN_INSTR (chain)) != N_cond)) {
        chain = ASSIGN_NEXT (chain);
    }

    DBUG_ASSERT (chain != NULL, "Missing conditional in loop!");
    chain = ASSIGN_RHS (COND_THENINSTR (ASSIGN_INSTR (chain)));

    DBUG_RETURN (chain);
}

static bool
ArgIsLoopInvariant (node *fundef, node *arg, int pos)
{
    bool result = FALSE;
    node *chain;
    int j;

    DBUG_ENTER ("ArgIsLoopInvariant");

    chain = GetRecursiveFunctionApplication (fundef);

    DBUG_ASSERT ((N_ap == NODE_TYPE (chain)) && (FUNDEF_ISDOFUN (AP_FUNDEF (chain))),
                 "No recursive function application found!");

    chain = AP_ARGS (chain);

    for (j = 0; j < pos; j++) {
        chain = EXPRS_NEXT (chain);
    }

    if (ARG_AVIS (arg) == ID_AVIS (EXPRS_EXPR (chain))) {
        result = TRUE;
    }
    if (result) {
        DBUG_PRINT ("WLPROP", ("Argument is loop invariant"));
    } else {
        DBUG_PRINT ("WLPROP", ("Argument is not loop invariant"));
    }

    DBUG_RETURN (result);
}

node *
WLPROPdoWithloopPropagation (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("WLPROPdoWithloopPropagation");

    arg_info = MakeInfo ();

    if (!FUNDEF_ISDOFUN (arg_node)) {

        TRAVpush (TR_wlprop);
        arg_node = TRAVdo (arg_node, arg_info);
        TRAVpop ();
    }

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/**<!--*************************************************************-->
 *
 * @fn node *WLPROPfundef(node *arg_node, info *arg_info)
 *
 * @brief: handles fundef nodes
 *
 *  <+long description+>
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ********************************************************************/
node *
WLPROPfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLPROPfundef");

    INFO_FUNDEF (arg_info) = arg_node;

    INFO_ISDOFUN (arg_info) = FUNDEF_ISDOFUN (arg_node);

    DBUG_PRINT ("WLPROP", ("Starting WLPROP for %s", FUNDEF_NAME (arg_node)));
    arg_node = INFNCdoInferNeedCountersOneFundef (arg_node);
    arg_node = WLSELCdoWithloopSelectionCount (arg_node);

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**<!--*************************************************************-->
 *
 * @fn node *WLPROPblock(node *arg_node, info *arg_info)
 *
 * @brief: handles block nodes
 *
 *  <+long description+>
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ********************************************************************/
node *
WLPROPblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLPROPblock");

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**<!--*************************************************************-->
 *
 * @fn node *WLPROPassign(node *arg_node, info *arg_info)
 *
 * @brief: handles assign nodes
 *
 *  <+long description+>
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ********************************************************************/
node *
WLPROPassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLPROPassign");

    INFO_ASSIGN (arg_info) = arg_node;

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (NULL != ASSIGN_NEXT (arg_node)) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**<!--*************************************************************-->
 *
 * @fn node *WLPROPlet(node *arg_node, info *arg_info)
 *
 * @brief: handles let nodes
 *
 *  <+long description+>
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ********************************************************************/
node *
WLPROPlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLPROPlet");

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**<!--*************************************************************-->
 *
 * @fn node *WLPROPap(node *arg_node, info *arg_info)
 *
 * @brief: handles ap nodes
 *
 *  <+long description+>
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ********************************************************************/
node *
WLPROPap (node *arg_node, info *arg_info)
{

    info *newinfo;

    DBUG_ENTER ("WLPROPap");

    if ((FUNDEF_ISDOFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))) {

        INFO_TRAVSTATE (arg_info) = S_withloop_prop;
        INFO_AP (arg_info) = arg_node;
        INFO_ARGNUM (arg_info) = 0;
        INFO_CORRESPONDINGFUNARG (arg_info) = FUNDEF_ARGS (AP_FUNDEF (arg_node));

        /*
         * propagate withloops into applied function
         */
        DBUG_PRINT ("WLPROP", ("Checking function arguments of %s",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));

        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);

        /*
         * traverse into do-funs and apply with-loop propagation again
         */
        newinfo = MakeInfo ();

        DBUG_PRINT ("WLPROP", ("Checking function application of %s",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));

        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), newinfo);

        newinfo = FreeInfo (newinfo);
    }

    if (FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node))) {

        /*
         * TODO: to be implemented
         */
    }

    INFO_TRAVSTATE (arg_info) = S_undef;

    DBUG_RETURN (arg_node);
}
/**<!--*************************************************************-->
 *
 * @fn node *WLPROPexprs(node *arg_node, info *arg_info)
 *
 * @brief: handles exprs nodes
 *
 *  <+long description+>
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ********************************************************************/
node *
WLPROPexprs (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLPROPexprs");

    EXPRS_EXPR (arg_node) = TRAVdo (EXPRS_EXPR (arg_node), arg_info);

    if (S_withloop_prop == INFO_TRAVSTATE (arg_info)) {
        INFO_CORRESPONDINGFUNARG (arg_info)
          = ARG_NEXT (INFO_CORRESPONDINGFUNARG (arg_info));

        INFO_ARGNUM (arg_info) = INFO_ARGNUM (arg_info) + 1;
    }
    if (EXPRS_NEXT (arg_node) != NULL) {
        EXPRS_NEXT (arg_node) = TRAVdo (EXPRS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**<!--*************************************************************-->
 *
 * @fn node *WLPROPid(node *arg_node, info *arg_info)
 *
 * @brief: handles id nodes
 *
 *  <+long description+>
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ********************************************************************/
node *
WLPROPid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLPROPid");

    if (S_withloop_prop == INFO_TRAVSTATE (arg_info)) {

        node *correspond_arg;

        correspond_arg = INFO_CORRESPONDINGFUNARG (arg_info);

        DBUG_PRINT ("WLPROP", ("Checking argument number %i", INFO_ARGNUM (arg_info)));

        if ((ArgIsDefinedByWL (arg_node))
            && (ArgIsLoopInvariant (AP_FUNDEF (INFO_AP (arg_info)), correspond_arg,
                                    INFO_ARGNUM (arg_info)))) {
            node *withloop
              = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (arg_node))));

            if ((WITH_SELMAX (withloop) == 0)
                || ((AVIS_NEEDCOUNT (ID_AVIS (arg_node)) == 1)
                    && (WITH_SELMAX (withloop) == 1))) {

                dfmask_t *inmask;
                lut_t *lut;
                node *next, *id;
                node *argchain_applying, *argchain_applied, *argchain_recapp;
                node *new_withloop, *old_withloop;
                node *witharg, *withvardec;

                DBUG_PRINT ("WLPROP", ("Checking argument number %i successful",
                                       INFO_ARGNUM (arg_info)));

                /*
                 * generate LUT
                 */
                lut = LUTgenerateLut ();

                /*
                 * create dataflowmask for withloop
                 */
                old_withloop = AVIS_SSAASSIGN (ID_AVIS (arg_node));
                next = ASSIGN_NEXT (old_withloop);
                ASSIGN_NEXT (old_withloop) = NULL;

                inmask
                  = INFDFMSdoInferInDfmAssignChain (old_withloop, INFO_FUNDEF (arg_info));

                ASSIGN_NEXT (old_withloop) = next;
                /*
                 * create new identifiers for identifiers marked in dfm
                 */

                next = DFMgetMaskEntryAvisSet (inmask);

                /*
                 * find last element of argument chain
                 * of applied fundef and applying ap.
                 * mark current and corresponding argument as dead
                 */

                argchain_applied = FUNDEF_ARGS (AP_FUNDEF (INFO_AP (arg_info)));
                argchain_applying = AP_ARGS (INFO_AP (arg_info));
                argchain_recapp
                  = GetRecursiveFunctionApplication (AP_FUNDEF (INFO_AP (arg_info)));
                argchain_recapp = AP_ARGS (argchain_recapp);

                /**
                 * TODO: replace following by TCappendArg/Epxr
                 */
                while (ARG_NEXT (argchain_applied) != NULL) {

                    DBUG_ASSERT ((NULL != argchain_applying), "Argument is missing!");

                    argchain_applying = EXPRS_NEXT (argchain_applying);
                    argchain_applied = ARG_NEXT (argchain_applied);
                    argchain_recapp = EXPRS_NEXT (argchain_recapp);
                }

                while (next != NULL) {
                    node *avis, *copy, *origin;

                    /*
                     * create new identifiers (become arg nodes)
                     * add them to applying and applied function signature
                     * insert old/new pair into lut
                     */
                    origin = next;
                    avis = TBmakeAvis (ILIBtmpVar (), TYcopyType (AVIS_TYPE (origin)));
                    copy = TBmakeArg (avis, NULL);

                    ARG_NEXT (argchain_applied) = copy;
                    argchain_applied = ARG_NEXT (argchain_applied);

                    EXPRS_NEXT (argchain_recapp) = TBmakeExprs (TBmakeId (avis), NULL);
                    argchain_recapp = EXPRS_NEXT (argchain_recapp);

                    id = TBmakeExprs (TBmakeId (origin), NULL);
                    EXPRS_NEXT (argchain_applying) = id;
                    argchain_applying = EXPRS_NEXT (argchain_applying);

                    lut = LUTinsertIntoLutP (lut, origin, avis);

                    /*
                     * get next marked element in dfm
                     */
                    next = DFMgetMaskEntryAvisSet (NULL);
                }
                /*
                 * now all needed identifiers were created
                 * the lut was filled
                 * now it is time to copy the withloop and insert it into
                 * the applied function
                 */
                new_withloop
                  = DUPdoDupTreeLutSsa (withloop, lut, AP_FUNDEF (INFO_AP (arg_info)));

                witharg = INFO_CORRESPONDINGFUNARG (arg_info);
                new_withloop
                  = TBmakeLet (TBmakeIds (ARG_AVIS (witharg), NULL), new_withloop);
                new_withloop
                  = TBmakeAssign (new_withloop, BLOCK_INSTR (FUNDEF_BODY (
                                                  AP_FUNDEF (INFO_AP (arg_info)))));
                BLOCK_INSTR (FUNDEF_BODY (AP_FUNDEF (INFO_AP (arg_info)))) = new_withloop;
                AVIS_SSAASSIGN (IDS_AVIS (LET_IDS (ASSIGN_INSTR (new_withloop))))
                  = new_withloop;

                /*
                 * Now the withloop definition was moved into the body
                 * of the applied function.
                 * Still missing: Transforming former withloop-arg to vardec
                 *                Adjust the recursive function call accordingly
                 *                Remove the withloop-arg from all signatures
                 *                is (done after whole function arguments are
                 *                checked)
                 */

                withvardec
                  = TBmakeVardec (IDS_AVIS (LET_IDS (ASSIGN_INSTR (new_withloop))),
                                  BLOCK_VARDEC (
                                    FUNDEF_BODY (AP_FUNDEF (INFO_AP (arg_info)))));

                BLOCK_VARDEC (FUNDEF_BODY (AP_FUNDEF (INFO_AP (arg_info)))) = withvardec;
                AVIS_DECL (IDS_AVIS (LET_IDS (ASSIGN_INSTR (new_withloop)))) = withvardec;

                /**
                 * change corresponding argument of current id to dummy identifier
                 */
                ARG_AVIS (INFO_CORRESPONDINGFUNARG (arg_info))
                  = TBmakeAvis (ILIBtmpVar (), TYcopyType (AVIS_TYPE (ARG_AVIS (
                                                 INFO_CORRESPONDINGFUNARG (arg_info)))));

                /*
                 * increase optimization counter
                 */
                global.optcounters.wlprop_expr++;
            }
        }
    }

    DBUG_RETURN (arg_node);
}
