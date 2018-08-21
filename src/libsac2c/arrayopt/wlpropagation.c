/** <!--******************************************************************-->
 *
 * @file wlpropagation.c
 *
 * The following functions implement the functionality of with-loop
 * propagation.
 * With-loop propagation moves a with-loop from one context into
 * another, here we are interested to move with-loops defined in one
 * function but used (via argument of a function call) within another
 * loop-function into that loop-function.
 *
 * This is only beneficial if no more overhead (with-loop moved into the
 * body of a loop) is produced as is prevented (because of less memory
 * access overhead (copying of arguments)).
 *
 * For that some preconditions have to be fulfilled:
 *
 * - The with-loop contains no selections (no expensive memory access)
 * - The with-loop contains just one selection but is used only
 *   once (that means: as argument of a function application).
 *
 * If these preconditions hold, then we assume it
 * is safe to do with-loop propagation.
 *
 * The rationale of with-loop propagation is the following:
 *
 * In the presence of with-loop folding, it sometimes makes sense not to
 * lift a loop-invariant with-loop out of a do-loop. More precisely, it
 * may make sense to move a with-loop actively into a do-loop. This is
 * the case if the with-loop can be folded with an existing with-loop in
 * the do-loop body, thus avoiding the actual creation of a temporary
 * array outside the do-loop.
 *
 * Consider the following example:
 *
 * A = with {
 *       ([0] <= iv < [100]) : 0;
 *     } genarray( [100]);
 *
 * do {
 *   B = with {
 *         ([0] <= iv < [100]) : B[iv] + A[iv];
 *       } genarray( [100]);
 * } while (...)
 *
 * Propagating the upper with-loop into the do-loop allows it to be folded
 * with the lower with-loop and thus disappear.
 *
 * Of course, with-loop propagation is a speculative optimisation in the
 * sense that we do not a-priori check whether folding will be successful.
 *
 * This would be relatively difficult per se and also difficult to keep in
 * sync with the actual implementation (or better implementations) of
 * with-loop folding. Therefore, we limit our checks to the ones mentioned
 * before. In case, with-loop folding does not happen eventually, a further
 * run of with-loop invariant removal will re-establish the original code.
 *
 * IMPLEMENTATION:
 *
 *  The algorithm effectively works at the N_ap level of the function
 *  that calls a LACFUN.
 *
 *  It appends new arguments to the AP_ARGS list, and alters,
 *  in place, the sons of the called function's N_fundef node,
 *  without having to rebuild the FUNDEF_LOCALFUN chain.
 *
 **************************************************************************/

#include "tree_basic.h"
#include "traverse.h"
#include "new_types.h"

#define DBUG_PREFIX "WLPROP"
#include "debug.h"

#include "str.h"
#include "memory.h"
#include "free.h"
#include "DupTree.h"
#include "globals.h"
#include "tree_compound.h"
#include "inferneedcounters.h"
#include "wlselcount.h"
#include "LookUpTable.h"
#include "DataFlowMask.h"
#include "infer_dfms.h"
#include "iteration_invariant_parameter_inference.h"
#include "phase.h"
#include "lacfun_utilities.h"

#include "wlpropagation.h"

typedef enum { S_undef, S_withloop_prop } travstate;

/*
 * info structure
 */
struct INFO {
    node *fundef;
    travstate travstate;
    node *ap;
    node *lacfunarg;
    size_t argnum;
    bool iscondfun;
    node *newlacfunargs;
    node *newlacfunreccall;
    node *newexternalcall;
#ifdef DEADCODE
    node *localfuns;
#endif // DEADCODE
};

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_TRAVSTATE(n) (n->travstate)
#define INFO_AP(n) (n->ap)
#define INFO_LACFUNARG(n) (n->lacfunarg)
#define INFO_ARGNUM(n) (n->argnum)
#define INFO_ISCONDFUN(n) (n->iscondfun)
#define INFO_NEWLACFUNARGS(n) (n->newlacfunargs)
#define INFO_NEWLACFUNRECCALL(n) (n->newlacfunreccall)
#define INFO_NEWEXTERNALCALL(n) (n->newexternalcall)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_TRAVSTATE (result) = S_undef;
    INFO_AP (result) = NULL;
    INFO_LACFUNARG (result) = NULL;
    INFO_ARGNUM (result) = 0;
    INFO_ISCONDFUN (result) = FALSE;
    INFO_NEWLACFUNARGS (result) = NULL;
    INFO_NEWLACFUNRECCALL (result) = NULL;
    INFO_NEWEXTERNALCALL (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *arg_info)
{
    DBUG_ENTER ();

    arg_info = MEMfree (arg_info);

    DBUG_RETURN (arg_info);
}

/**
 *-- Local Helper Functions ---------------------------------------
 */

/** <!--*************************************************************-->
 *
 * @fn static bool IdIsDefinedByWL(node *arg_node)
 *
 * @brief: returns TRUE if arg_node (always of type N_id) is defined by
 *         a with-loop
 *
 * @param arg_node of type N_id
 *
 * @result
 *
 ********************************************************************/
static bool
IdIsDefinedByWL (node *arg_node)
{
    bool result = FALSE;
    node *tmp;

    DBUG_ENTER ();

    tmp = AVIS_SSAASSIGN (ID_AVIS (arg_node));
    if (NULL != tmp) {
        if (N_with == NODE_TYPE (LET_EXPR (ASSIGN_STMT (tmp)))) {
            result = TRUE;
            DBUG_PRINT ("%s is defined by a WL", AVIS_NAME (ID_AVIS (arg_node)));
        } else {
            DBUG_PRINT ("%s is not defined by a WL", AVIS_NAME (ID_AVIS (arg_node)));
        }
    }

    DBUG_RETURN (result);
}

/** <!--*************************************************************-->
 *
 * @fn static lut_t*  insertNameIntoArgAndSig( node *origin, lut_t *lut...
 *
 * @brief: create new identifiers (become arg nodes)
 *         add them to applying and applied function signature
 *         insert old/new pair into lut. The old name is that
 *         in the external call; the new name is generated here.
 *         The lut will be used for renames in the code block
 *         copied into the lacfun.
 *
 * @param  oldname: The external function's N_avis to insert,
 *         or something AVIS_DIMish.
 *
 *         lut: The lut table we need to build
 *
 *         argchains:
 *          lacfunargs: the lacfun's new funargs
 *          lacfunreccall: the new args to a LOOPFUN recursive call
 *            If we are dealing with a CONDFUN, this will be NULL.
 *          externalcall: the new args to the  external call to a LACFUN
 *
 * @result updated LUT, used for renames within the LACFUN,
 *         renaming oldname to the newly generated name.
 *
 *         Side effects on argchains,
 *         NOP on non-N_avis/N_id nodes
 *
 ********************************************************************/
static lut_t *
insertNameIntoArgAndSig (node *oldname, lut_t *lut, info *arg_info)
{
    node *avis;
    node *copy;

    DBUG_ENTER ();

    if ((NULL != oldname) && (N_id == NODE_TYPE (oldname))) {
        oldname = ID_AVIS (oldname);
    }

    if ((NULL != oldname) && (N_avis == NODE_TYPE (oldname))) {

        avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (AVIS_TYPE (oldname)));
        copy = TBmakeArg (avis, NULL);

        INFO_NEWLACFUNARGS (arg_info)
          = TCappendArgs (INFO_NEWLACFUNARGS (arg_info), copy);

        if (!INFO_ISCONDFUN (arg_info)) {
            INFO_NEWLACFUNRECCALL (arg_info)
              = TCappendExprs (INFO_NEWLACFUNRECCALL (arg_info),
                               TBmakeExprs (TBmakeId (avis), NULL));
        }

        INFO_NEWEXTERNALCALL (arg_info)
          = TCappendExprs (INFO_NEWEXTERNALCALL (arg_info),
                           TBmakeExprs (TBmakeId (oldname), NULL));

        DBUG_PRINT ("Inserting into LUT, oldname=%s, newname=%s", AVIS_NAME (oldname),
                    AVIS_NAME (avis));

        lut = LUTinsertIntoLutP (lut, oldname, avis);
    }

    DBUG_RETURN (lut);
}

/**
 *--Global traversal functions -------------------------------
 */

/** <!--*************************************************************-->
 *
 * @fn node *WLPROPdoWithloopPropagation(node *arg_node)
 *
 * @brief: Starting routine of with-loop propagation traversal
 *
 * @param arg_node
 *
 * @result
 *
 ********************************************************************/
node *
WLPROPdoWithloopPropagation (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    if (!FUNDEF_ISLACFUN (arg_node)) {
        TRAVpush (TR_wlprop);
        arg_node = TRAVdo (arg_node, arg_info);
        TRAVpop ();
    }

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*************************************************************-->
 *
 * @fn node *WLPROPfundef(node *arg_node, info *arg_info)
 *
 * @brief: handles fundef nodes
 *
 * First all necessary information for with-loop propagation are
 * infered, then the actual traversal is started.
 *
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ********************************************************************/
node *
WLPROPfundef (node *arg_node, info *arg_info)
{
    info *old_info;

    DBUG_ENTER ();

    old_info = arg_info;
    arg_info = MakeInfo ();

    INFO_FUNDEF (arg_info) = arg_node;
    INFO_ISCONDFUN (arg_info) = INFO_ISCONDFUN (old_info);

    DBUG_PRINT ("Starting %s", FUNDEF_NAME (arg_node));

    /**
     * Infer before actual traversal:
     *   - number of applications (appearence on rhs)
     *     of identifiers
     *   - count applications of N_sel operations in
     *     withloop, as well as function application
     *     in with-loops
     *   - infer loop invariant arguments
     */
    arg_node = INFNCdoInferNeedCountersOneFundef (arg_node, TR_wlprop);
    arg_node = WLSELCdoWithloopSelectionCount (arg_node);
    arg_node = IIPIdoIterationInvariantParameterInference (arg_node);

    /**
     * do actual traversal
     */
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    FUNDEF_RETURN (arg_node) = LFUfindFundefReturn (arg_node);

    DBUG_PRINT ("Done with %s", FUNDEF_NAME (arg_node));

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*************************************************************-->
 *
 * @fn node *WLPROPassign(node *arg_node, info *arg_info)
 *
 * @brief: handles assign nodes, top-down traversal
 *
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ********************************************************************/
node *
WLPROPassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

#ifdef DEADCODE2
// the idea here is to handle AVIS son nodes cleanly. FIXME
/** <!--*************************************************************-->
 *
 * @fn node *WLPROPavis(node *arg_node, info *arg_info)
 *
 * @brief: This traversal handles the sons of avis nodes
 *         that are required by the WLs we copy into a lACFUN.
 *         E.g., AVIS_SHAPE,DIM/MIN/MAX
 *
 * @param arg_node
 * @param arg_info
 *
 * @result update arg_node
 *
 ********************************************************************/
node *
WLPROPavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Looking at N_avis %s", CTIitemName (arg_node));
    arg_node = TRAVsons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*************************************************************-->
 *
 * @fn node *WLPROPids(node *arg_node, info *arg_info)
 *
 * @brief: This traversal handles the LHS of assigments
 *         It is required by the WLs we copy into a lACFUN.
 *         E.g., AVIS_SHAPE,DIM/MIN/MAX
 *
 * @param arg_node
 * @param arg_info
 *
 * @result update arg_node
 *
 ********************************************************************/
node *
WLPROPavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Looking at N_ids %s", CTIitemName (arg_node));
    arg_node = TRAVsons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

#endif // DEADCODE2
/** <!--*************************************************************-->
 *
 * @fn node *WLPROPap(node *arg_node, info *arg_info)
 *
 * @brief: handles ap nodes
 *
 * If the current function application represents a do-loop the
 * argument chain is traversed and with-loop propagation can apply if
 * possible. After that, with-loop propagation is done again on the
 * same function to handle transitive dependencies.
 *
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ********************************************************************/
node *
WLPROPap (node *arg_node, info *arg_info)
{
    bool oldiscondfun;

    DBUG_ENTER ();

#ifdef SEEBUG1019
    if (((FUNDEF_ISLOOPFUN (AP_FUNDEF (arg_node)))
         || (FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node))))
        && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))) {
#else  //  SEEBUG1019
    if ((FUNDEF_ISLOOPFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))) {
#endif // SEEBUG1019

        /**
         * We apply a cond-function, or a do-function, if it is not the
         * recursive application of the current function
         */

        /**
         * set info structure
         */
        INFO_TRAVSTATE (arg_info) = S_withloop_prop;
        INFO_AP (arg_info) = arg_node;
        INFO_ARGNUM (arg_info) = 0;
        INFO_LACFUNARG (arg_info) = NULL;
        oldiscondfun = INFO_ISCONDFUN (arg_info);
        INFO_ISCONDFUN (arg_info) = FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node));
        INFO_NEWLACFUNARGS (arg_info) = NULL;
        INFO_NEWLACFUNRECCALL (arg_info) = NULL;
        INFO_NEWEXTERNALCALL (arg_info) = NULL;

        DBUG_PRINT ("Checking function arguments of %s",
                    FUNDEF_NAME (AP_FUNDEF (arg_node)));

        /**
         * traverse into argument chain and try to
         * propagate withloops into applied function
         */
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);

        /* Prefix any new arguments to external function call */
        AP_ARGS (arg_node)
          = TCappendExprs (INFO_NEWEXTERNALCALL (arg_info), AP_ARGS (arg_node));

        /* Prefix any new formal parameters to LACFUN args */
        FUNDEF_ARGS (AP_FUNDEF (arg_node))
          = TCappendArgs (INFO_NEWLACFUNARGS (arg_info),
                          FUNDEF_ARGS (AP_FUNDEF (arg_node)));

        /* Prefix any new arguments to recursive call in LACFUN */
        if (NULL != INFO_NEWLACFUNRECCALL (arg_info)) {
            AP_ARGS (FUNDEF_LOOPRECURSIVEAP (AP_FUNDEF (arg_node)))
              = TCappendExprs (INFO_NEWLACFUNRECCALL (arg_info),
                               AP_ARGS (FUNDEF_LOOPRECURSIVEAP (AP_FUNDEF (arg_node))));
        }
        INFO_NEWLACFUNARGS (arg_info) = NULL;
        INFO_NEWLACFUNRECCALL (arg_info) = NULL;
        INFO_NEWEXTERNALCALL (arg_info) = NULL;
        INFO_ISCONDFUN (arg_info) = oldiscondfun;

        /**
         * traverse into applied con/do-fun and do
         * with-loop propagation again
         */
        DBUG_PRINT ("Checking function application of %s",
                    FUNDEF_NAME (AP_FUNDEF (arg_node)));
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
    }

    INFO_TRAVSTATE (arg_info) = S_undef;

    DBUG_RETURN (arg_node);
}

/** <!--*************************************************************-->
 *
 * @fn node *WLPROPexprs(node *arg_node, info *arg_info)
 *
 * @brief: handles exprs nodes
 *
 * Does just a top-down traversal.
 * If the travstate equals withloop_prop, the current expr-chain
 * represents the argument chain of an N_ap node. Then the
 * corresponding N_arg node (linked to from arg_info) is also adjusted.
 *
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ********************************************************************/
node *
WLPROPexprs (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (S_withloop_prop == INFO_TRAVSTATE (arg_info)) {
        /**
         * since we are trying to propagate withloops into
         * do-funs, set the arg-node of the
         * applied do-fun to correspond to the id-node of the
         * applying (calling) function which will be traversed next.
         */
        INFO_LACFUNARG (arg_info)
          = TCgetNthArg (INFO_ARGNUM (arg_info),
                         FUNDEF_ARGS (AP_FUNDEF (INFO_AP (arg_info))));
        DBUG_PRINT ("Trying to propagate N_arg %s",
                    AVIS_NAME (ARG_AVIS (INFO_LACFUNARG (arg_info))));
        INFO_ARGNUM (arg_info) = INFO_ARGNUM (arg_info) + 1;
        EXPRS_EXPR (arg_node) = TRAVdo (EXPRS_EXPR (arg_node), arg_info);
    }

    EXPRS_NEXT (arg_node) = TRAVopt (EXPRS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*************************************************************-->
 *
 * @fn node *CopyWL(node *arg_node, info *arg_info)
 *
 * @brief: Copy one WL and its dependences into LACFUN body.
 *         Introduce required renames for code body.
 *         Introduce new parameters and arguments to
 *         N_ap calls and N_args.
 *
 * arg_node is defined by a with-loop and both the
 * with-loop and arg_node fulfill the conditions for with-loop propagation.
 *
 * A lookup table and a dataflow mask are used
 * to identify identifiers used in the with-loop, to create corresponding
 * new ones and to keep track of them. Then the with-loop is moved from
 * the calling function into the called function (insert code in
 * argument chain, adjust references and function signatures).
 *
 * @param arg_node
 * @param arg_info
 * @param withloop: N_assign for WL to be copied.
 *
 * @result updated arg_node.
 *         Side effects on LACFUN
 *         Side effect on N_ap call
 *
 ********************************************************************/
static node *
CopyWL (node *arg_node, info *arg_info)
{
    dfmask_t *inmask;
    lut_t *lut;
    node *next;
    node *old_withloop;
    node *new_withloop;
    node *wlavis;
    node *lacfundef;

    DBUG_ENTER ();

    DBUG_PRINT ("Copying WL %s from %s into LACFUN %s", AVIS_NAME (ID_AVIS (arg_node)),
                FUNDEF_NAME (INFO_FUNDEF (arg_info)),
                FUNDEF_NAME (AP_FUNDEF (INFO_AP (arg_info))));

    /*
     * create dataflowmask for withloop
     */
    old_withloop = AVIS_SSAASSIGN (ID_AVIS (arg_node));
    next = ASSIGN_NEXT (old_withloop);
    ASSIGN_NEXT (old_withloop) = NULL;

    lut = LUTgenerateLut ();
    inmask = INFDFMSdoInferInDfmAssignChain (old_withloop, INFO_FUNDEF (arg_info));

    ASSIGN_NEXT (old_withloop) = next;

    /**
     * create new identifiers for identifiers marked in dfm
     * and append them to corresponding arg/exprs chains and insert
     * them into the lut.
     */

    next = DFMgetMaskEntryAvisSet (inmask);

    while (next != NULL) {
        lut = insertNameIntoArgAndSig (next, lut, arg_info);
        /*
         * get next marked element in dfm
         */
        next = DFMgetMaskEntryAvisSet (NULL);
    }

    /*
     * Put WL result's AVIS sons nodes into LUT.
     * Find a cleaner way to do this...
     *
     */

    wlavis = ID_AVIS (arg_node);
    lut = insertNameIntoArgAndSig (AVIS_DIM (wlavis), lut, arg_info);
    lut = insertNameIntoArgAndSig (AVIS_SHAPE (wlavis), lut, arg_info);
    lut = insertNameIntoArgAndSig (AVIS_MIN (wlavis), lut, arg_info);
    lut = insertNameIntoArgAndSig (AVIS_MAX (wlavis), lut, arg_info);
    lut = insertNameIntoArgAndSig (AVIS_SCALARS (wlavis), lut, arg_info);

    /*
     * now all needed identifiers have been created
     * the lut is filled
     * copy the withloop and insert it into the applied function
     */
    lacfundef = AP_FUNDEF (INFO_AP (arg_info));

    /* Copy the WL RHS (but not its NEXT) & insert its variables into the LACFUN.
     * Give the copied WL's LHS its old N_arg name.
     */
    next = ASSIGN_NEXT (old_withloop);
    ASSIGN_NEXT (old_withloop) = NULL;
    new_withloop = DUPdoDupNodeLutSsa (old_withloop, lut, lacfundef);
    ASSIGN_NEXT (old_withloop) = next;
    lut = LUTremoveLut (lut);

    /*
     * At this point, the new local WL is ready for insertion, and we
     * have set up the new function parameters that it will use.
     *
     * Now, we work on the LACFUN:
     * Build vardec for new local WL.
     * Correct the AVIS_SONs of the new local WL's AVIS
     * I don't like what's going on here (or above with
     * avis sons, but have no great ideas on how to fix it cleanly.
     */

    wlavis = IDS_AVIS (LET_IDS (ASSIGN_STMT (new_withloop)));
    AVIS_DIM (wlavis) = DUPdoDupNodeLut (AVIS_DIM (wlavis), lut);
    AVIS_SHAPE (wlavis) = DUPdoDupNodeLut (AVIS_SHAPE (wlavis), lut);
    AVIS_MIN (wlavis) = DUPdoDupNodeLut (AVIS_MIN (wlavis), lut);
    AVIS_MAX (wlavis) = DUPdoDupNodeLut (AVIS_MAX (wlavis), lut);
    AVIS_SCALARS (wlavis) = DUPdoDupNodeLut (AVIS_SCALARS (wlavis), lut);

    /*
     * Copy the withloop definition into the body
     * of the lacfun.
     *
     */

    /* Finally, copy the WL to where it belongs in the LACFUN */
    lacfundef = LFUinsertAssignIntoLacfun (lacfundef, new_withloop,
                                           ARG_AVIS (INFO_LACFUNARG (arg_info)));

    /*
     * increase optimization counter
     */
    global.optcounters.wlprop_expr++;

    DBUG_RETURN (arg_node);
}

/** <!--*************************************************************-->
 *
 * @fn node *WLPROPid(node *arg_node, info *arg_info)
 *
 * @brief: handles id nodes
 *
 * Most of the actual code transformation is done here.
 * If 'arg_node' is defined by a with-loop and both the
 * with-loop and 'arg_node' fulfill the conditions for
 * with-loop propagation the transformation starts.
 * First of all a lookup table and a dataflow mask are used
 * to identify identifiers used in the with-loop, to create corresponding
 * new ones and to keep track of them. Then the with-loop is moved from
 * the calling function into the called function (insert code in
 * argument chain, adjust references and function signatures).
 *
 * @param arg_node
 * @param arg_info
 *
 * @result
 *
 ********************************************************************/
node *
WLPROPid (node *arg_node, info *arg_info)
{
    node *withloop;
    node *correspond_arg;

    DBUG_ENTER ();

    if (S_withloop_prop == INFO_TRAVSTATE (arg_info)) {

        DBUG_PRINT ("Checking argument number %zu, N_id=%s", INFO_ARGNUM (arg_info),
                    AVIS_NAME (ID_AVIS (arg_node)));

        /*
         * is the argument defined by a loop-independent with-loop,
         * or by a CONDFUN?
         */
        correspond_arg = INFO_LACFUNARG (arg_info);
        if ((IdIsDefinedByWL (arg_node)) && (NULL != INFO_LACFUNARG (arg_info))
            && ((INFO_ISCONDFUN (arg_info))
                || (AVIS_SSALPINV (ARG_AVIS (correspond_arg))))) {
            withloop = LET_EXPR (ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (arg_node))));

            /*
             * Does the with-loop fulfil the required selection
             * conditions? If so, then do the actual work.
             */
            if (!WITH_CONTAINSFUNAPS (withloop)) {
                arg_node = CopyWL (arg_node, arg_info);
            }
        }
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
