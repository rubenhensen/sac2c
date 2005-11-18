/**<!--******************************************************************-->
 *
 * $Id$
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
#include "new_types.h"
#include "dbug.h"
#include "internal_lib.h"
#include "free.h"
#include "DupTree.h"
#include "globals.h"
#include "tree_compound.h"
#include "inferneedcounters.h"
#include "wlselcount.h"
#include "LookUpTable.h"
#include "DataFlowMask.h"
#include "InferDFMs.h"
#include "SSAInferLI.h"

#include "wlpropagation.h"

typedef enum { S_undef, S_withloop_prop } travstate;

/*
 * info structure
 */
struct INFO {
    node *fundef;
    travstate travstate;
    node *ap;
    node *corr;
    int argnum;
};

#define INFO_FUNDEF(n) (n->fundef)
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

/**
 *-- Local Helper Functions ---------------------------------------
 */

/**<!--*************************************************************-->
 *
 * @fn static bool IdIsDefinedByWL(node *arg_node)
 *
 * @brief: returns TRUE if arg_node (=>N_id) is defined by
 *         with-loop
 *
 *  <+long description+>
 * @param arg_node
 *
 * @result
 *
 ********************************************************************/
static bool
IdIsDefinedByWL (node *arg_node)
{
    bool result = FALSE;
    node *tmp;

    DBUG_ENTER ("IdIsDefinedByWL");

    tmp = AVIS_SSAASSIGN (ID_AVIS (arg_node));
    if (NULL != tmp) {
        if (N_with == NODE_TYPE (LET_EXPR (ASSIGN_INSTR (tmp)))) {
            result = TRUE;
        }
    }

    DBUG_RETURN (result);
}

/**<!--*************************************************************-->
 *
 * @fn static node *GetRecursiveFunctionApplication(node *fundef)
 *
 * @brief: returns the N_assign-node conatining the
 *         recursive function call of 'fundef'
 *
 *
 *  <+long description+>
 * @param fundef is a N_fundef node, defining a do-function
 *
 * @result
 *
 ********************************************************************/
static node *
GetRecursiveFunctionApplication (node *fundef)
{
    node *chain;
    DBUG_ENTER ("GetRecursiveFunctionApplication");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef), "N_fundef node expected.");
    DBUG_ASSERT ((FUNDEF_ISDOFUN (fundef)), "Loop-Function expected.");

    /**
     * search for recursive fun call
     */
    chain = BLOCK_INSTR (FUNDEF_BODY (fundef));

    while ((chain != NULL) && (NODE_TYPE (ASSIGN_INSTR (chain)) != N_cond)) {
        chain = ASSIGN_NEXT (chain);
    }

    DBUG_ASSERT (chain != NULL, "Missing conditional in loop!");
    chain = ASSIGN_RHS (COND_THENINSTR (ASSIGN_INSTR (chain)));

    DBUG_RETURN (chain);
}

/**
 *--Global traversal functions -------------------------------
 */

/**<!--*************************************************************-->
 *
 * @fn node *WLPROPdoWithloopPropagation(node *arg_node)
 *
 * @brief: Starting routine of with-loop propagation traversal
 *
 *  <+long description+>
 * @param arg_node
 *
 * @result
 *
 ********************************************************************/
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

    DBUG_PRINT ("WLPROP", ("Starting WLPROP for %s", FUNDEF_NAME (arg_node)));

    /**
     * Infer before actual traversal:
     *   - number of applications (appearence on rhs)
     *     of identifiers
     *   - count applications of N_sel operations in
     *     withloop, as well as function application
     *     in with-loops
     *   - infer loop invariant arguments
     */
    arg_node = INFNCdoInferNeedCountersOneFundef (arg_node);
    arg_node = WLSELCdoWithloopSelectionCount (arg_node);
    arg_node = ILIdoInferLoopInvariants (arg_node);

    /**
     * do actual traversal
     */
    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

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

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (NULL != ASSIGN_NEXT (arg_node)) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

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

        /**
         * We apply a do-function, which is not the
         * recursive application of the current function
         */

        /**
         * set info structure
         */
        INFO_TRAVSTATE (arg_info) = S_withloop_prop;
        INFO_AP (arg_info) = arg_node;
        INFO_ARGNUM (arg_info) = 0;
        INFO_CORRESPONDINGFUNARG (arg_info) = FUNDEF_ARGS (AP_FUNDEF (arg_node));

        DBUG_PRINT ("WLPROP", ("Checking function arguments of %s",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));

        /**
         * traverse into argument chain and try to
         * propagate withloops into applied function
         */
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);

        /**
         * traverse into applied do-fun and do
         * with-loop propagation again
         */
        DBUG_PRINT ("WLPROP", ("Checking function application of %s",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));
        newinfo = MakeInfo ();
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
        /**
         * since we are trying to propagate withloops into
         * do-funs, set the arg-node of the
         * applied do-fun to correspond to the id-node of the
         * applying function which will be traversed next.
         */
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

        /**
         * is the argument defined by an with-loop
         * which is loop independent?
         */
        if ((IdIsDefinedByWL (arg_node)) && (AVIS_SSALPINV (ARG_AVIS (correspond_arg)))) {
            node *withloop
              = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (arg_node))));

            /**
             * Does the with-loop fulfil the required selection
             * conditions?
             */
            if ((WITH_SELMAX (withloop) == 0)
                || ((AVIS_NEEDCOUNT (ID_AVIS (arg_node)) == 1)
                    && (WITH_SELMAX (withloop) == 1))) {

                dfmask_t *inmask;
                lut_t *lut;
                node *next;
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

                /**
                 * create new identifiers for identifiers marked in dfm
                 * and append them to corresponding arg/exprs chains and insert
                 * them into the lut.
                 */

                next = DFMgetMaskEntryAvisSet (inmask);

                argchain_applied = FUNDEF_ARGS (AP_FUNDEF (INFO_AP (arg_info)));
                argchain_applying = AP_ARGS (INFO_AP (arg_info));
                argchain_recapp
                  = GetRecursiveFunctionApplication (AP_FUNDEF (INFO_AP (arg_info)));
                argchain_recapp = AP_ARGS (argchain_recapp);

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

                    argchain_applied = TCappendArgs (argchain_applied, copy);

                    argchain_recapp = TCappendExprs (argchain_recapp,
                                                     TBmakeExprs (TBmakeId (avis), NULL));

                    argchain_applying
                      = TCappendExprs (argchain_applying,
                                       TBmakeExprs (TBmakeId (origin), NULL));

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
                  = TBmakeAssign (new_withloop,
                                  FUNDEF_INSTR (AP_FUNDEF (INFO_AP (arg_info))));

                FUNDEF_INSTR (AP_FUNDEF (INFO_AP (arg_info))) = new_withloop;

                AVIS_SSAASSIGN (ARG_AVIS (witharg)) = new_withloop;

                /*
                 * Now the withloop definition was moved into the body
                 * of the applied function.
                 * Still missing: Transforming former withloop-arg to vardec
                 *                Replace the withloop-arg in signature
                 *                by dummy identifier
                 */

                withvardec
                  = TBmakeVardec (IDS_AVIS (LET_IDS (ASSIGN_INSTR (new_withloop))),
                                  BLOCK_VARDEC (
                                    FUNDEF_BODY (AP_FUNDEF (INFO_AP (arg_info)))));

                BLOCK_VARDEC (FUNDEF_BODY (AP_FUNDEF (INFO_AP (arg_info)))) = withvardec;

                /**
                 * change corresponding argument of current id to dummy
                 * identifier
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
