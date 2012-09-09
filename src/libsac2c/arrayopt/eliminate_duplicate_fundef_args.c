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
 *  We traverse all functions, including LACFUNs, once.
 *  All of the work is done from the N_ap node of the caller.
 *
 *  First, we mark the duplicate entries in the call to the LACFUN, and
 *  build a LUT for mapping duplicate names in the LACFUN itself.
 *
 *  Then, we DUP the LACFUN body, renaming references to
 *  duplicated parameters to refer to the non-duplicated ones.
 *
 *  Then we find the call to each LACFUN, and eliminate
 *  the duplicate elements from those calls, using
 *  the marking on the LACFUN parameters as a guide.
 *
 *  Then, we remove the duplicate elements from the recursive LOOPFUN call.
 *
 *  Finally, we remove the duplicated entries from the LACFUN header.
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
#include "eliminate_duplicate_fundef_args.h"
#include "lacfun_utilities.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    bool onefundef;
    lut_t *lutargs;
    lut_t *lutrenames;
};

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_ONEFUNDEF(n) (n->onefundef)
#define INFO_LUTARGS(n) (n->lutargs)
#define INFO_LUTRENAMES(n) (n->lutrenames)

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
 * @param arg_node: N_ap call to some LACFUN.
 *
 * @result: Updated N_ap node.
 *
 * @implementation:
 *
 *   Loop over the N_ap's N_exprs chain, and the LACFUN's parameters.
 *   Remove any N_ap elements that correspond to duplicated LACFUN args.
 *   This alters AP_ARGS, but does not return a new AP, so we
 *   can run in place.
 *
 ******************************************************************************/
static node *
SimplifyCall (node *arg_node, info *arg_info)
{
    node *args;
    node *newargs = NULL;
    node *lacfundef;
    node *lacfunparms;
    node *next;

    DBUG_ENTER ();

    lacfundef = AP_FUNDEF (arg_node);
    args = AP_ARGS (arg_node);
    lacfunparms = FUNDEF_ARGS (lacfundef);

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
 * @fn node *SimplifyLacfunHeader( node *arg_node, node *lacfundef)
 *
 * @brief Eliminate any duplicate parameters from the LACFUN function
 *        header.
 *
 * @param arg_node: N_args
 *
 * @result: Updated LACFUN N_args node.
 *
 ******************************************************************************/
static node *
SimplifyLacfunHeader (node *arg_node, node *lacfundef)
{
    node *newargs = NULL;
    node *next;

    DBUG_ENTER ();

    while (NULL != arg_node) {
        next = ARG_NEXT (arg_node);
        ARG_NEXT (arg_node) = NULL;
        if (!ARG_ISDUPLICATE (arg_node)) {
            newargs = TCappendArgs (newargs, arg_node);
        } else {
            DBUG_PRINT ("Duplicate LACFUN parameter %s deleted from %s",
                        AVIS_NAME (ARG_AVIS (arg_node)), FUNDEF_NAME (lacfundef));
            LFUarg2Vardec (arg_node, lacfundef);
        }
        arg_node = next;
    }

    DBUG_RETURN (newargs);
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
        if (NULL != AVIS_DIM (avis)) {
            son = DUPdoDupNodeLut (AVIS_DIM (avis), lutrenames);
            FREEdoFreeNode (AVIS_DIM (avis));
            AVIS_DIM (avis) = son;
        }
        if (NULL != AVIS_SHAPE (avis)) {
            son = DUPdoDupNodeLut (AVIS_SHAPE (avis), lutrenames);
            FREEdoFreeNode (AVIS_SHAPE (avis));
            AVIS_SHAPE (avis) = son;
        }
        if (NULL != AVIS_MIN (avis)) {
            son = DUPdoDupNodeLut (AVIS_MIN (avis), lutrenames);
            FREEdoFreeNode (AVIS_MIN (avis));
            AVIS_MIN (avis) = son;
        }
        if (NULL != AVIS_MAX (avis)) {
            son = DUPdoDupNodeLut (AVIS_MAX (avis), lutrenames);
            FREEdoFreeNode (AVIS_MAX (avis));
            AVIS_MAX (avis) = son;
        }
        if (NULL != AVIS_SCALARS (avis)) {
            son = DUPdoDupNodeLut (AVIS_SCALARS (avis), lutrenames);
            FREEdoFreeNode (AVIS_SCALARS (avis));
            AVIS_SCALARS (avis) = son;
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
 * @param N_ap arg_node to be traversed
 *
 * @result: N_ap, unchanged?
 *          Side effects: ARG_ISDUPLICATE( FUNDEF_ARGS(lacunf)) set
 *          for some nodes, and lacfun FUNDEF_BODY references to the
 *          dups renamed.
 *
 * @implementation:
 *         We populate a LUT, LUTARGS, to quickly(?) find
 *         duplicate N_ap arguments, and at the same time,
 *         map the argument name into the LACFUN
 *         formal parameter (FUNDEF_ARG) name, using another LUT.
 *
 *         As an example of this inference, if we have the following call:
 *
 *           z = Loop_1( A, B, A, D, A);
 *
 *         and Loop_1's function header is:
 *
 *           int Loop_1( int CA, int CB, int CC, int CD, int CE)
 *
 *         with this recursive call (in which CC is NOT loop-invariant):
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
    node *lacfundef;

    DBUG_ENTER ();

    lacfundef = AP_FUNDEF (arg_node);
    DBUG_PRINT ("Examining LACFUN %s", FUNDEF_NAME (lacfundef));

    LUTremoveContentLut (INFO_LUTARGS (arg_info));
    LUTremoveContentLut (INFO_LUTRENAMES (arg_info));

    apargs = AP_ARGS (arg_node);          /* outer call */
    fundefargs = FUNDEF_ARGS (lacfundef); /* formal parameters */
    /* recursive callargs, if any */
    rca = FUNDEF_LOOPRECURSIVEAP (lacfundef);
    rca = (NULL != rca) ? AP_ARGS (rca) : NULL;

    while (NULL != apargs) {
        argid = EXPRS_EXPR (apargs);
        argavis = ID_AVIS (argid);
        if (LFUisLoopFunInvariant (lacfundef, ARG_AVIS (fundefargs), rca)) {
            lutitem = (node **)LUTsearchInLutP (INFO_LUTARGS (arg_info), argavis);
            if (NULL == lutitem) {
                /* Entry not in LUT. This is a new argument.
                 * Insert avis for set membership.
                 * Also, insert for renaming to corresponding name inside lacfun.
                 */
                INFO_LUTARGS (arg_info)
                  = LUTinsertIntoLutP (INFO_LUTARGS (arg_info), argavis,
                                       ARG_AVIS (fundefargs));
                DBUG_PRINT ("Non-dup arg %s (%s in lacfun) found", AVIS_NAME (argavis),
                            AVIS_NAME (ARG_AVIS (fundefargs)));
            } else {
                /* This parameter is a loop-invariant dup */
                ARG_ISDUPLICATE (fundefargs) = TRUE;
                formalargavis = *lutitem;
                DBUG_PRINT ("Duplicate arg %s renamed to %s in LACFUN %s",
                            AVIS_NAME (ARG_AVIS (fundefargs)), formalargavis,
                            FUNDEF_NAME (lacfundef));
                INFO_LUTRENAMES (arg_info)
                  = LUTinsertIntoLutP (INFO_LUTRENAMES (arg_info), ARG_AVIS (fundefargs),
                                       formalargavis);
                lutnonempty = TRUE;
            }
        } else {
            DBUG_PRINT ("argument %s (%s in lacfun) is not loop-invariant",
                        AVIS_NAME (argavis), AVIS_NAME (ARG_AVIS (fundefargs)));
        }

        apargs = EXPRS_NEXT (apargs);
        fundefargs = ARG_NEXT (fundefargs);
        rca = (NULL != rca) ? EXPRS_NEXT (rca) : NULL;
    }

    if (lutnonempty) {
        DBUG_PRINT ("Performing renames for LACFUN %s", FUNDEF_NAME (lacfundef));
        FUNDEF_ARGS (lacfundef)
          = RenameArgs (FUNDEF_ARGS (lacfundef), INFO_LUTRENAMES (arg_info));
        FUNDEF_BODY (lacfundef)
          = DUPdoDupNodeLut (FUNDEF_BODY (lacfundef), INFO_LUTRENAMES (arg_info));
        FUNDEF_RETS (lacfundef)
          = DUPdoDupTreeLut (FUNDEF_RETS (lacfundef), INFO_LUTRENAMES (arg_info));
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

    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;

    DBUG_PRINT ("Handling %s", FUNDEF_NAME (arg_node));
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

    if (!INFO_ONEFUNDEF (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *EDFAap(node *arg_node, info *arg_info)
 *
 * @brief We have found a function call site.
 *        If the callee is a LACFUN, examine and deal with it.
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
    node *lacfundef;
    node *reccall;

    DBUG_ENTER ();

    lacfundef = AP_FUNDEF (arg_node);
    if (FUNDEF_ISLACFUN (lacfundef) && (lacfundef != INFO_FUNDEF (arg_info))) {
        /* non-recursive LACFUN calls only */
        DBUG_PRINT ("Found LACFUN: %s call from: %s", FUNDEF_NAME (lacfundef),
                    FUNDEF_NAME (INFO_FUNDEF (arg_info)));
        arg_node = MarkDupsAndRenameBody (arg_node, arg_info);
        arg_node = SimplifyCall (arg_node, arg_info); /* outer call */
        if (FUNDEF_ISLOOPFUN (lacfundef)) {
            FUNDEF_LOOPRECURSIVEAP (lacfundef) = LFUfindRecursiveCallAp (lacfundef);
            reccall = FUNDEF_LOOPRECURSIVEAP (lacfundef);
            reccall = SimplifyCall (reccall, arg_info); /* recursive call */
        }
        FUNDEF_ARGS (lacfundef)
          = SimplifyLacfunHeader (FUNDEF_ARGS (lacfundef), lacfundef);
        FUNDEF_RETURN (lacfundef) = LFUfindFundefReturn (lacfundef);
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
