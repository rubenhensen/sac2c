/** <!--********************************************************************-->
 *
 * @defgroup cwle Copy with-loop elimination
 *
 *  Overview:
 *  ---------
 *
 * This traversal eliminates with-loops that do nothing more
 * than copy some array, in toto.
 *
 * The code ensures that, in _sel_VxA_( IV, A),
 * that IV matches the iv in the WL generator. I.e., there are
 * no offsets, nor transpositions, etc, happening, merely a
 * straight element-by-element copy.
 *
 * WLs matching the above criteria are replaced by A = B, e.g.:
 *
 * B = with {
 *       (.<=iv<=.) : A[iv];
 *     } : genarray( shape(A), n );
 *
 * will be transformed to:
 *
 * B = A;
 *
 * There are several cases, depending various factors such as
 * SAACYC vs. CYC, producer-WL as LACFUN parameter, etc.
 *
 * Implementation issues:
 * -----------------------
 *
 * When matching for the pattern A[iv] (in CWLEcode), we need to make sure
 * that "A" has been defined BEFORE this WL-body rather than within it.
 * Otherwise we wrongly transform a WL of the following (pathological :-) kind:
 *
 * B = with {
 *       (.<=iv<=.) {
 *         A = genarray( shp, 21);
 *       } : A[iv];
 *     } : genarray( shp, n);
 *
 * To detect these cases, we use the DFMs and mark all LHS defined BEFORE
 * entering the WL as such.
 *
 * NB: There should a better way to do this than using DFMs, but I'm
 *     busy...
 *
 * @ingroup opt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file copywlelim.c
 *
 * Prefix: CWLE
 *
 *****************************************************************************/
#include "copywlelim.h"

#include "globals.h"

#define DBUG_PREFIX "CWLE"
#include "debug.h"

#include "str.h"
#include "memory.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "DupTree.h"
#include "free.h"
#include "compare_tree.h"
#include "DataFlowMask.h"
#include "pattern_match.h"
#include "constants.h"
#include "shape.h"
#include "type_utils.h"
#include "new_types.h"
#include "indexvectorutils.h"
#include "with_loop_utilities.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/

struct INFO {
    node *lhs;
    node *fundef;
    node *pavis;
    node *withid;
    bool valid;
    dfmask_t *dfm;
};

/* The left hand side of the N_let for the consumer-WL */
#define INFO_LHS(n) (n->lhs)
#define INFO_FUNDEF(n) (n->fundef)
/* The function currently being traversed. This is here to ease debugging */
#define INFO_PAVIS(n) (n->pavis)
/* This is the selection-vector inside our with-loop */
#define INFO_WITHID(n) (n->withid)
/* Do we (still) have a valid case of cwle? */
#define INFO_VALID(n) (n->valid)
/* This saves all visited identifiers, so we only replace loops with known
   values */
#define INFO_DFM(n) (n->dfm)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_VALID (result) = FALSE;
    INFO_LHS (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_PAVIS (result) = NULL;
    INFO_WITHID (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    INFO_LHS (info) = NULL;
    INFO_PAVIS (info) = NULL;
    INFO_WITHID (info) = NULL;

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
 * @fn node *CWLEdoCopyWithLoopElimination( node *arg_node)
 *
 *****************************************************************************/
node *
CWLEdoCopyWithLoopElimination (node *arg_node)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef, "Called on non-N_fundef node");

    DBUG_PRINT ("Copy with-loop elimination traversal started.");

    TRAVpush (TR_cwle);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    DBUG_PRINT ("Copy with-loop elimination traversal ended.");

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *CWLEfundef(node *arg_node, info *arg_info)
 *
 * @brief Sets up a DFM and traverses into the function-body.
 *
 * Here we do set up a DataFlowMask, which we do need for checking if the
 *   array that we potentially copy from is already defined before the
 *   respective with-loop.
 *
 * Afterwards we just traverse into the function args (so they can be
 * marked in our DFM) and body.
 *
 *****************************************************************************/

node *
CWLEfundef (node *arg_node, info *arg_info)
{
    node *oldfundef;
    dfmask_base_t *dfmask_base = NULL;

    DBUG_ENTER ();

    oldfundef = INFO_FUNDEF (arg_info);
    INFO_FUNDEF (arg_info) = arg_node;
    if (NULL != FUNDEF_BODY (arg_node)) {
        DBUG_PRINT ("traversing body of (%s) %s",
                    (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                    FUNDEF_NAME (arg_node));

        dfmask_base = DFMgenMaskBase (FUNDEF_ARGS (arg_node),
                                      BLOCK_VARDECS (FUNDEF_BODY (arg_node)));
        INFO_DFM (arg_info) = DFMgenMaskClear (dfmask_base);

        FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        INFO_DFM (arg_info) = DFMremoveMask (INFO_DFM (arg_info));
        DFMremoveMaskBase (dfmask_base);
        DBUG_PRINT ("leaving body of (%s) %s",
                    (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                    FUNDEF_NAME (arg_node));
    }

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    INFO_FUNDEF (arg_info) = oldfundef;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CWLEarg( node *arg_node, info *arg_info)
 *
 * @brief for setting the bitmask in our DFM.
 *
 *****************************************************************************/
node *
CWLEarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    DBUG_PRINT ("Setting DFM for argument %s", AVIS_NAME (ARG_AVIS (arg_node)));
    DFMsetMaskEntrySet (INFO_DFM (arg_info), NULL, ARG_AVIS (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CWLElet(node *arg_node, info *arg_info)
 *
 * @brief checks for the shape of its LHS and passes it on
 *
 * This function checks for the shape of the identifier that is placed
 *   on its left hand side. We need this information to check if the array
 *   that we copy from is the same size as the array that we would like
 *   to copy into.
 *
 *****************************************************************************/

node *
CWLElet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
#ifdef VERBOSE
    DBUG_PRINT ("Looking at %s", AVIS_NAME (IDS_AVIS (LET_IDS (arg_node))));
#endif // VERBOSE

    INFO_VALID (arg_info) = TRUE;

    if (NULL != LET_IDS (arg_node)) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
        INFO_LHS (arg_info) = LET_IDS (arg_node);
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    /*
     * reset to false, because the handling of this case is over, independent
     * of its success.
     */

    INFO_VALID (arg_info) = FALSE;
    INFO_LHS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CWLEassign( node *arg_node, info *arg_info)
 *
 * @brief ensures a top-down traversal
 *
 *****************************************************************************/
node *
CWLEassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CWLEids( node *arg_node, info *arg_info)
 *
 * @brief sets the bitmask for its bitmask in info_dfm.
 *
 *****************************************************************************/
node *
CWLEids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Setting DFM for %s", AVIS_NAME (IDS_AVIS (arg_node)));
    DFMsetMaskEntrySet (INFO_DFM (arg_info), NULL, IDS_AVIS (arg_node));

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CWLEwith(node *arg_node, info *arg_info)
 *
 * @brief traverses the codes and replaces itself with a N_id on success.
 *
 * Here we traverse into the codeblocks for checking for nested wls and to
 *   have a look for some cwle-action.
 *   If the checks in the codes succeed, we compare the found array we
 *   apparently copy from with the array we would like to create; if these
 *   fit (in shape) we may replace ourselves with a N_id node of the array
 *   found in the codes.
 *
 *****************************************************************************/
node *
CWLEwith (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    DBUG_PRINT ("Looking at WL %s", AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));

    INFO_WITHID (arg_info) = WITH_WITHID (arg_node);

    // We only care about WLs with a single partition.
    if (NULL == PART_NEXT (WITH_PART (arg_node))) {
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    }

    /*
     * if our codes indicate that we are still on the run we have to check
     *   multiple things:
     *   1. is this a one-generator-loop?
     *   2. do we look at a modarray/genarray-loop?
     *   3. do the shapes of INFO_LHS and INFO_RHS match?
     */

    if ((INFO_VALID (arg_info)) && (NULL == WITHOP_NEXT (WITH_WITHOP (arg_node)))
        && (NULL == PART_NEXT (WITH_PART (arg_node)))
        && (N_genarray == WITH_TYPE (arg_node) || N_modarray == WITH_TYPE (arg_node))) {

        DBUG_PRINT ("Codes OK. Comparing shapes of LHS(%s), RHS(%s)",
                    AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))),
                    AVIS_NAME (INFO_PAVIS (arg_info)));

        if (IVUTisShapesMatch (INFO_PAVIS (arg_info), IDS_AVIS (INFO_LHS (arg_info)),
                               GENERATOR_BOUND2 (
                                 PART_GENERATOR (WITH_PART (arg_node))))) {
            DBUG_PRINT ("All ok. replacing LHS(%s) WL by %s",
                        AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))),
                        AVIS_NAME (INFO_PAVIS (arg_info)));
            global.optcounters.cwle_wl++;

            /*
             * 1. free the consumer; we do not need it anymore.
             * 2. return a brandnew N_id, built from the producer.
             */
            arg_node = FREEdoFreeTree (arg_node);
            arg_node = TBmakeId (INFO_PAVIS (arg_info));

            /*
             * To avoid loss of type information, we attach the old LHS type to the RHS
             * in the form of a type-conv! See bug 1147 for details.
             */
            arg_node = TCmakePrf2 (F_type_conv,
                                   TBmakeType (TYcopyType (
                                     AVIS_TYPE (IDS_AVIS (INFO_LHS (arg_info))))),
                                   arg_node);
        } else {
            DBUG_PRINT ("Shape mismatch: Unable to replace LHS(%s) WL by RHS(%s)",
                        AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))),
                        AVIS_NAME (INFO_PAVIS (arg_info)));
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CWLEcode(node *arg_node, info *arg_info)
 *
 * @brief checks for a valid case of cwle in this wl and in nested with loops.
 *
 * several tasks are done in the N_code nodes:
 *
 *   1. At first we do check for a case of cwle in this withloop.
 *
 *   2. Traverse into all succeeding code-blocks, checking if they allow for
 *      a cwle.
 *
 *   3. If we had a look into all code blocks, we do mark the WITHID in our
 *      DFM, so it is available in nested withloops, and traverse into just
 *      these.
 *
 *****************************************************************************/
node *
CWLEcode (node *arg_node, info *arg_info)
{
    node *cexpr;
    node *srcwl = NULL;
#ifndef DBUG_OFF
    char *lhs;
#endif
    info *subinfo;

    DBUG_ENTER ();

    if (INFO_VALID (arg_info)) {
        DBUG_PRINT ("prev nodes and wl signal ok");
        cexpr = EXPRS_EXPR (CODE_CEXPRS (arg_node));
        srcwl = WLUTfindCopyPartitionFromCexpr (cexpr, INFO_WITHID (arg_info));
        if (NULL == srcwl) {
#ifndef DBUG_OFF
            lhs = AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info)));
#endif
            DBUG_PRINT ("body of %s does not match _sel_VxA_( withid, &srcwl)", lhs);
            INFO_VALID (arg_info) = FALSE;
        }
    } else {
        DBUG_PRINT ("previous nodes signal NOT ok");
    }

    /*
     * if we have found some avis that meets the requirements, then lets check if
     * it is the same that we have found before. If we do not have found anything
     * before, assign it to INFO_PAVIS.
     * At this point we also check the DataFlowMask, to see if our source array
     * was defined _before_ this wl.
     */
    if (INFO_VALID (arg_info)) {
        DBUG_PRINT ("checking if target is legitimate and known");

        if ((NULL == INFO_PAVIS (arg_info) || srcwl == INFO_PAVIS (arg_info))
            && DFMtestMaskEntry (INFO_DFM (arg_info), NULL, srcwl)) {
            DBUG_PRINT ("srcwl is valid. saving");

            INFO_PAVIS (arg_info) = srcwl;
        } else {
            DBUG_PRINT ("srcwl is NOT valid. skipping wl");

            INFO_VALID (arg_info) = FALSE;
            INFO_PAVIS (arg_info) = NULL;
        }
    }

    /*
     * if we got another code, traverse it; if we are at the end of all codes,
     * mark the withid. This ensures that the withid is available inside all
     * wls which may be nested inside and copy from our withid.
     */
    if (NULL != CODE_NEXT (arg_node)) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    } else {
        INFO_WITHID (arg_info) = TRAVdo (INFO_WITHID (arg_info), arg_info);
    }

    /*
     * create a new info-structure for traversing the code-block, traverse, and
     * release the info-structure. we obviously need to pass it the dfmask, so
     * we use that from the local info structure.
     * We need the seperate structure so we do not mess with the current wl.
     */
    subinfo = MakeInfo ();
    INFO_DFM (subinfo) = INFO_DFM (arg_info);
    INFO_FUNDEF (subinfo) = INFO_FUNDEF (arg_info);
    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), subinfo);
    subinfo = FreeInfo (subinfo);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/

#undef DBUG_PREFIX
