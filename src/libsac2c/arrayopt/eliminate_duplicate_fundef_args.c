/*
 * $Id$
 */

/**<!--******************************************************************-->
 *
 * @file eliminate_duplicate_undef_args.c
 *
 * This traversal, eliminates duplicate
 * arguments from LACFUN calls. It does not, at present, perform
 * any other tasks theoretically performed by SISI.
 *
 * Overview of traversal:
 *
 *  We traverse all functions, including LACFUNs, several times:
 *
 *  Pass markdups: Find the duplicated arguments to each LACFUN,
 *          and mark the FUNDEF_ARGS of the LACFUN to indicate
 *          that they are superfluous.
 *
 *          We also rewrite the LACFUN body, renaming
 *          references to duplicated parameters to refer to
 *          the non-duplicated ones.
 *
 *  Pass simplifycalls: Find the calls to each LACFUN, and eliminate
 *          the duplicate elements from those calls, using
 *          the marking on the LACFUN parameters as a guide.
 *
 *          Also, remove the duplicate elements on the
 *          recursive LOOPFUN calls.
 *
 *  Pass simplifylacfun:
 *          Finally, remove the duplicate entries from the
 *          LACFUN parameters.
 *
 **************************************************************************/

#include "tree_basic.h"
#include "traverse.h"
#include "node_basic.h"
#include "new_types.h"

#define DBUG_PREFIX "EDFA"
#include "debug.h"

#include "str.h"
#include "memory.h"
#include "free.h"
#include "globals.h"
#include "constants.h"
#include "shape.h"
#include "tree_compound.h"
#include "LookUpTable.h"
#include "DupTree.h"
#include "indexvectorutils.h"
#include "eliminate_duplicate_fundef_args.h"
#include "lacfun_utilities.h"

typedef enum { markdups, simplifycalls, simplifylacfun } travphases;
/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    bool onefundef;
    lut_t *lutargs;
    lut_t *lutrenames;
    travphases phase;
};

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_ONEFUNDEF(n) (n->onefundef)
#define INFO_LUTARGS(n) (n->lutargs)
#define INFO_LUTRENAMES(n) (n->lutrenames)
#define INFO_PHASE(n) (n->phase)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();
    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_ONEFUNDEF (result) = FALSE;
    INFO_LUTARGS (result) = NULL;
    INFO_LUTRENAMES (result) = NULL;
    INFO_PHASE (result) = markdups;

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
 *
 * @name Static helper functions
 * @{
 *
 *****************************************************************************/

/**<!--***********************************************************************-->
 *
 * @fn node *SimplifyCall( node *arg_node, info *arg_info)
 *
 * @brief Remove any duplicate parameters from a call to a LACFUN.
 *
 * @param arg_node: N_ap call to some LACFUN (inner and outer)
 *
 * @result: Updated N_ap node.
 *
 * @implementation:
 *   Loop over the N_ap's N_exprs chain, and the LACFUN's parameters.
 *   Remove any N_ap elements that correspond to duplicated LACFUN args.
 *
 ******************************************************************************/
static node *
SimplifyCall (node *arg_node, info *arg_info)
{
    node *args;
    node *newargs = NULL;
    node *calledfn;
    node *lacfunparms;
    node *next;

    DBUG_ENTER ();

    calledfn = AP_FUNDEF (arg_node);
    args = AP_ARGS (arg_node);
    lacfunparms = FUNDEF_ARGS (calledfn);

    while (NULL != args) {
        next = EXPRS_NEXT (args);
        EXPRS_NEXT (args) = NULL;
        if (!ARG_ISDUPLICATE (lacfunparms)) {
            newargs = TCappendExprs (newargs, args);
        } else {
            DBUG_PRINT ("Removing dup LACFUN arg: %s called from: %s",
                        AVIS_NAME (ID_AVIS (EXPRS_EXPR (args))),
                        FUNDEF_NAME (INFO_FUNDEF (arg_info)));
            args = FREEdoFreeNode (args);
            global.optcounters.edfa_expr++;
        }
        args = next;
        lacfunparms = ARG_NEXT (lacfunparms);
    }
    AP_ARGS (arg_node) = newargs;

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *SimplifyFunctionHeader( node *arg_node)
 *
 * @brief Eliminate any duplicate parameters from the LACFUN function
 *        header.
 *
 * @param arg_node: N_fundef
 *
 * @result: Updated LACFUN N_fundef node.
 *
 ******************************************************************************/
static node *
SimplifyFunctionHeader (node *arg_node)
{
    node *args;
    node *newargs = NULL;
    node *next;

    DBUG_ENTER ();

    if (FUNDEF_ISLACFUN (arg_node)) {
        args = FUNDEF_ARGS (arg_node);

        while (NULL != args) {
            next = ARG_NEXT (args);
            ARG_NEXT (args) = NULL;
            if (!ARG_ISDUPLICATE (args)) {
                newargs = TCappendArgs (newargs, args);
            } else {
                DBUG_PRINT ("Duplicate LACFUN parameter %s deleted from %s",
                            AVIS_NAME (ARG_AVIS (args)), FUNDEF_NAME (arg_node));
                args = FREEdoFreeNode (args);
            }
            args = next;
        }
        FUNDEF_ARGS (arg_node) = newargs;
    }

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *RenameArgs( node *arg_node, lut_t *lutrenames)
 *
 * @brief: Update the fundef's N_arg chain, renaming AVIS sons according to
 *         the LUT.
 *
 *         We must not use DUPdoDupTreeLut because that would
 *         copy the N_avis nodes, and give us ugly problems such
 *         as multiple N_avis nodes with the same AVIS_NAME.
 *
 *         This is REALLY ugly, and I would like to burn this code...
 *
 * @param arg_node: N_arg node for the fundef
 *
 * @result: The DUP'd N_arg chain with renames performed.
 *
 ******************************************************************************/
static node *
RenameArgs (node *arg_node, lut_t *lutrenames)
{
    node *avis;
    node *son;
    node *curarg;

    DBUG_ENTER ();

    curarg = arg_node;
    while (NULL != curarg) {
        avis = ARG_AVIS (curarg);
        if ((NULL != AVIS_DIM (avis)) && (N_id == NODE_TYPE (AVIS_DIM (avis)))) {
            son = DUPdoDupNodeLut (AVIS_DIM (avis), lutrenames);
            FREEdoFreeNode (AVIS_DIM (avis));
            AVIS_DIM (avis) = son;
        }
        if ((NULL != AVIS_SHAPE (avis)) && (N_id == NODE_TYPE (AVIS_SHAPE (avis)))) {
            son = DUPdoDupNodeLut (AVIS_SHAPE (avis), lutrenames);
            FREEdoFreeNode (AVIS_SHAPE (avis));
            AVIS_SHAPE (avis) = son;
        }
        if ((NULL != AVIS_MIN (avis)) && (N_id == NODE_TYPE (AVIS_MIN (avis)))) {
            son = DUPdoDupNodeLut (AVIS_MIN (avis), lutrenames);
            FREEdoFreeNode (AVIS_MIN (avis));
            AVIS_MIN (avis) = son;
        }
        if ((NULL != AVIS_MAX (avis)) && (N_id == NODE_TYPE (AVIS_MAX (avis)))) {
            son = DUPdoDupNodeLut (AVIS_MAX (avis), lutrenames);
            FREEdoFreeNode (AVIS_MAX (avis));
            AVIS_MAX (avis) = son;
        }
        curarg = ARG_NEXT (curarg);
    }

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *MarkDupsAndRenameBody( node *arg_node, info *arg_info)
 *
 * @brief Mark those LACFUN FUNDEF_ARGS arguments appearing more than once
 *        in their N_ap function call, and which are loop-invariant
 *        in a LOOPFUN. Rename FUNDEF_BODY references to those
 *        names to refer to the non-duplicated names.
 *
 * @param N_fundef arg_node to be traversed
 *
 * @result: fundef node, with ARG_ISDUPLICATE( FUNDEF_ARGS()) set
 *          for some nodes, and FUNDEF_BODY references to the
 *          dups renamed.
 *
 * @implementation:
 *         We populate a LUT, LUTARGS, to quickly(?) find
 *         duplicate N_ap arguments, and at the same time,
 *         map the argument name into the LACFUN
 *         formal parameter (FUNDEF_ARG) name.
 *
 *         As an example of this inference, if we have the following call:
 *
 *           z = Loop_1( A, B, A, D, A);
 *
 *         and Loop_1's function header is:
 *
 *           int Loop_1( int CA, int CB, int CC, int CD, int CE)
 *
 *         with this recursive call, in which CC NOT loop-invariant:
 *
 *           xx = Loop_1( A, B, XX, D, A);
 *
 *        We put the following entries into LUTARGS:
 *
 *            A --> CA
 *            B --> CB
 *            D --> CD
 *
 *        The last value of A in the call is a dup,
 *        as we find by looking at LUTARGS, so we mark
 *        its ARG_ISDUPLICATE. We also place an entry into LUTRENAMES:
 *
 *            CE --> CA
 *
 *        At the end up the pass, we rename the FUNDEF_BODY
 *        using that LUT, at which point the duplicated
 *        FUNDEF_ARG entries are no longer referenced in the LACFUN.
 *
 ******************************************************************************/
static node *
MarkDupsAndRenameBody (node *arg_node, info *arg_info)
{
    node *argavis;
    node *argid;
    node *apargs;
    node *fundefargs = NULL;
    node *rca = NULL;
    bool lutnonempty = FALSE;
    node **lutitem = NULL;
    node *formalargavis = NULL;

    DBUG_ENTER ();

    DBUG_PRINT ("Examining LACFUN %s", FUNDEF_NAME (arg_node));

    if (NULL != FUNDEF_CALLAP (arg_node)) { /* did not find call ap yet */
        LUTremoveContentLut (INFO_LUTARGS (arg_info));
        LUTremoveContentLut (INFO_LUTRENAMES (arg_info));

        apargs = AP_ARGS (FUNDEF_CALLAP (arg_node)); /* outer call */
        fundefargs = FUNDEF_ARGS (arg_node);         /* formal parameters */
        /* recursive callargs */
        rca = FUNDEF_LOOPRECURSIVEAP (arg_node);
        rca = (NULL != rca) ? AP_ARGS (rca) : NULL;

        while (NULL != apargs) {
            argid = EXPRS_EXPR (apargs);
            argavis = ID_AVIS (argid);
            if (LFUisLoopFunInvariant (arg_node, fundefargs, rca)) {
                lutitem = (node **)LUTsearchInLutP (INFO_LUTARGS (arg_info), argavis);
                if (NULL == lutitem) {
                    /* Entry not in LUT. This is a new argument.
                     * Insert avis for set membership.
                     * Also, insert for renaming to corresponding name inside lacfun.
                     */
                    INFO_LUTARGS (arg_info)
                      = LUTinsertIntoLutP (INFO_LUTARGS (arg_info), argavis,
                                           ARG_AVIS (fundefargs));
                    DBUG_PRINT ("Non-duplicate argument %s found", AVIS_NAME (argavis));
                } else {
                    /* This argument is a loop-invariant dup */
                    ARG_ISDUPLICATE (fundefargs) = TRUE;
                    formalargavis = *lutitem;
                    DBUG_PRINT ("Duplicate arg %s renamed to %s in LACFUN %s",
                                AVIS_NAME (argavis), AVIS_NAME (formalargavis),
                                FUNDEF_NAME (arg_node));
                    INFO_LUTRENAMES (arg_info)
                      = LUTinsertIntoLutP (INFO_LUTRENAMES (arg_info),
                                           ARG_AVIS (fundefargs), formalargavis);
                    lutnonempty = TRUE;
                }
            }
            apargs = EXPRS_NEXT (apargs);
            fundefargs = ARG_NEXT (fundefargs);
            rca = (NULL != rca) ? EXPRS_NEXT (rca) : NULL;
        }

        if (lutnonempty) {
            DBUG_PRINT ("Performing renames for LACFUN %s", FUNDEF_NAME (arg_node));
            FUNDEF_ARGS (arg_node)
              = RenameArgs (FUNDEF_ARGS (arg_node), INFO_LUTRENAMES (arg_info));

            FUNDEF_BODY (arg_node)
              = DUPdoDupNodeLut (FUNDEF_BODY (arg_node), INFO_LUTRENAMES (arg_info));
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/**<!--***********************************************************************-->
 *
 * @fn node *EDFAdoEliminateDuplicateFundefArgs(node *arg_node)
 *
 * @brief starting point of traversal
 *
 * @param N_fundef or N_module to be traversed
 *
 * @result
 *
 ******************************************************************************/
node *
EDFAdoEliminateDuplicateFundefArgs (node *arg_node)
{
    info *arg_info;
    DBUG_ENTER ();

    arg_info = MakeInfo ();
    INFO_ONEFUNDEF (arg_info) = (N_fundef == NODE_TYPE (arg_node));

    INFO_LUTARGS (arg_info) = LUTgenerateLut ();
    INFO_LUTRENAMES (arg_info) = LUTgenerateLut ();

    TRAVpush (TR_edfa);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    INFO_LUTARGS (arg_info) = LUTremoveLut (INFO_LUTARGS (arg_info));
    INFO_LUTRENAMES (arg_info) = LUTremoveLut (INFO_LUTRENAMES (arg_info));
    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *EDFAmodule(node *arg_node, info *arg_info)
 *
 * @brief traverses the functions of the module
 *
 * @param arg_node node to be traversed
 * @param arg_info
 *
 * @result
 *
 ******************************************************************************/
node *
EDFAmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *EDFAfundef(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node fundef to be traversed
 * @param arg_info
 *
 * @result Same old fundef, but LOCALFUNS may be updated.
 *
 ******************************************************************************/
node *
EDFAfundef (node *arg_node, info *arg_info)
{
    info *oldinfo;

    DBUG_ENTER ();

    oldinfo = arg_info;
    arg_info = MakeInfo ();
    INFO_FUNDEF (arg_info) = arg_node;
    INFO_PHASE (arg_info) = INFO_PHASE (oldinfo);
    INFO_LUTARGS (arg_info) = INFO_LUTARGS (oldinfo);
    INFO_LUTRENAMES (arg_info) = INFO_LUTRENAMES (oldinfo);

    /* Set ARG_ISDUPLICATE for all LACFUN parameters */
    if (markdups == INFO_PHASE (arg_info)) {
        DBUG_PRINT ("Marking in: %s", FUNDEF_NAME (arg_node));
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
        if (FUNDEF_ISLACFUN (arg_node)) {
            arg_node = MarkDupsAndRenameBody (arg_node, arg_info);
        }

        /* Phase change when we are back at non-LACFUN level */
        if (!FUNDEF_ISLACFUN (arg_node)) {
            INFO_PHASE (arg_info) = simplifycalls;
        }
    }

    /* Remove duplicated arguments from all calls to LACFUNs */
    if ((simplifycalls == INFO_PHASE (arg_info))) {
        DBUG_PRINT ("Simplifying calls in: %s", FUNDEF_NAME (arg_node));
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
        if (!FUNDEF_ISLACFUN (arg_node)) {
            INFO_PHASE (arg_info) = simplifylacfun;
        }
    }

    if ((simplifylacfun == INFO_PHASE (arg_info))) {
        DBUG_PRINT ("Simplifying header in: %s", FUNDEF_NAME (arg_node));
        arg_node = SimplifyFunctionHeader (arg_node);
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
        FUNDEF_CALLAP (arg_node) = NULL;
    }

    if (!INFO_ONEFUNDEF (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *EDFAap(node *arg_node, info *arg_info)
 *
 * @brief Pass markdups:
 *          We have found a function call site.
 *          If the callee is a LACFUN, put the
 *          address of our N_ap into FUNDEF_CALLAP.
 *
 *        Pass simplifycalls:
 *          Here, we are looking at a LACFUN call site;
 *          we use the LACFUN header parameters, previously
 *          perhaps marked as duplicates, as guidance for
 *          eliminating the corresponding elements from
 *          the N_ap call.
 *
 * @param arg_node N_ap node to be traversed
 * @param arg_info
 *
 * @result arg_node
 *
 ******************************************************************************/
node *
EDFAap (node *arg_node, info *arg_info)
{
    node *calledfn;

    DBUG_ENTER ();

    calledfn = AP_FUNDEF (arg_node);
    if ((markdups == INFO_PHASE (arg_info)) && (FUNDEF_ISLACFUN (calledfn))
        && (calledfn != INFO_FUNDEF (arg_info))) { /* Ignore recursive call */
        DBUG_PRINT ("Found LACFUN: %s call from: %s", FUNDEF_NAME (calledfn),
                    FUNDEF_NAME (INFO_FUNDEF (arg_info)));
        FUNDEF_CALLAP (calledfn) = arg_node; /* Point LACFUN at our N_ap */
    }

    if ((simplifycalls == INFO_PHASE (arg_info)) && (FUNDEF_ISLACFUN (calledfn))) {
        DBUG_PRINT ("Simplifying call to LACFUN: %s from: %s", FUNDEF_NAME (calledfn),
                    FUNDEF_NAME (INFO_FUNDEF (arg_info)));
        arg_node = SimplifyCall (arg_node, arg_info);

        /* Correct pointer to recursive call in the LOOPFUN */
        if (calledfn == INFO_FUNDEF (arg_info)) {
            FUNDEF_LOOPRECURSIVEAP (calledfn) = arg_node;
        }
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
