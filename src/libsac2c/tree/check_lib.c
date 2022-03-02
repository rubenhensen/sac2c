#include "check_lib.h"

#include "print.h"
#include "free.h"
#include "str.h"
#include "traverse.h"
#include "tree_basic.h"
#include "types.h"
#include "tree_compound.h"
#include "globals.h"
#include "ctinfo.h"
#include "phase.h"

#define DBUG_PREFIX "CHK"
#include "debug.h"

/** <!--**********************************************************************-->
 *
 * @fn node *CHKinsertError( node *arg_node, char *string)
 *******************************************************************************/
node *
CHKinsertError (node *arg_node, char *string)
{
    DBUG_ENTER ();

    if (arg_node == NULL) {

        /*
         * CTIwarn internaly frees memory that was allocated before the
         * memtab has been copied in CHKMdoAnalyzeMemtab. Therefore it must
         * not be used to print the error string when the memcheck mechanism
         * is active
         */
        if (global.memcheck) {
            fprintf (stderr, "WARNING: %s\n", string);
        } else {
            CTIwarn (EMPTY_LOC, "%s", string);
        }

        arg_node = TBmakeError (STRcpy (string), global.compiler_anyphase, arg_node);
    } else {
        if (!(STReq (string, ERROR_MESSAGE (arg_node)))) {
            ERROR_NEXT (arg_node) = CHKinsertError (ERROR_NEXT (arg_node), string);
        } else {
            ERROR_ANYPHASE (arg_node) = global.compiler_anyphase;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--**********************************************************************-->
 *
 * @fn node *CHKexistSon( node *child, node *arg_node, char *string)
 *
 *******************************************************************************/
node *
CHKexistSon (node *son, node *arg_node, char *string)
{
    DBUG_ENTER ();

    if (son == NULL) {

        NODE_ERROR (arg_node) = CHKinsertError (NODE_ERROR (arg_node), string);
    }

    DBUG_RETURN (son);
}

/** <!--**********************************************************************-->
 *
 * @fn intptr_t CHKexistAttribute( intptr_t attribute, node *arg_node, char *string)
 *
 *******************************************************************************/
intptr_t
CHKexistAttribute (intptr_t attribute, node *arg_node, char *string)
{
    DBUG_ENTER ();

    if (attribute == (intptr_t)NULL) {

        NODE_ERROR (arg_node) = CHKinsertError (NODE_ERROR (arg_node), string);
    }

    DBUG_RETURN (attribute);
}

/** <!--**********************************************************************-->
 *
 * @fn intptr_t CHKnotExist( intptr_t son_attribute, node *arg_node, char *string)
 *
 *******************************************************************************/

intptr_t
CHKnotExist (intptr_t son_attribute, node *arg_node, char *string)
{
    DBUG_ENTER ();

    if (son_attribute != (intptr_t)NULL) {

        NODE_ERROR (arg_node) = CHKinsertError (NODE_ERROR (arg_node), string);
    }

    DBUG_RETURN (son_attribute);
}

/** <!--********************************************************************-->
 *
 * @fn node *CHKnotExistAttribute( node *attribute, node *arg_node, char *string)
 *
 *****************************************************************************/
node *
CHKnotExistAttribute (void *attribute, node *arg_node, char *string)
{
    DBUG_ENTER ();

    if (attribute != NULL) {

        NODE_ERROR (arg_node) = CHKinsertError (NODE_ERROR (arg_node), string);
    }

    DBUG_RETURN ((node *)attribute);
}

/** <!--**********************************************************************-->
 *
 * @fn node *CHKcorrectTypeInsertError( node *arg_node, char *string)
 *
 *******************************************************************************/
node *
CHKcorrectTypeInsertError (node *arg_node, char *string)
{
    DBUG_ENTER ();

    NODE_ERROR (arg_node) = CHKinsertError (NODE_ERROR (arg_node), string);

    DBUG_RETURN (arg_node);
}

/** <!--**********************************************************************-->
 *
 * @fn node *CHKisAttribIds( node *arg_node)
 *
 * @desc: If in SSA mode, check that each N_ids' AVIS_SSAASSIGN is either NULL,
 *        or that it points to N_assign node assgn.
 *        If assgn is NULL, we are being called from some place where we do not
 *        know the N_assign node, so do the best we can.
 *
 *        If not in SSA mode, we allow anything, for better or worse...
 *
 *        Result is TRUE if all ok, else FALSE and NODE_ERROR.
 *        Also true if the node is an N_spids, because that means
 *        we are very early in the compilation process, and N_avis nodes
 *        do not exist yet.
 *
 *        arg_node may be NULL if the N_let has no result, e.g., "_dec_rc_()".
 *
 *******************************************************************************/
bool
CHKisAttribIds (node *arg_node, node *assgn)
{
    node *ids;
    node *avis;
    node *aassgn;
    bool b = TRUE;

    DBUG_ENTER ();

    ids = arg_node;
    if ((NULL != arg_node) && (N_ids == NODE_TYPE (arg_node))) { // Ignore N_spids
        while (ids != NULL) {
            avis = IDS_AVIS (ids);
            aassgn = AVIS_SSAASSIGN (avis);
            if (NULL != aassgn) { // Sadly, NULL treated as ok here.
                b = b && (N_assign == NODE_TYPE (aassgn));
                b = b && ((NULL == assgn) || (assgn == aassgn));
                if (!b) {
                    NODE_ERROR (arg_node)
                      = CHKinsertError (NODE_ERROR (avis), "AVIS_SSAASSIGN is does not "
                                                           "point to correct N_assign "
                                                           "node for N_ids");
                    DBUG_PRINT ("for %s", AVIS_NAME (avis));
                }
            }
            ids = IDS_NEXT (ids);
        }
    }

    DBUG_RETURN (b);
}

/** <!--**********************************************************************-->
 *
 * @fn node *CHKattribsAssign( node *arg_node)
 *
 * @desc: Check all N_ids in this N_assign, to ensure that
 *        their AVIS_SSAASSIGN either points to this assign, or is NULL.
 *        If not in SSA mode, must be NULL.
 *
 *******************************************************************************/
node *
CHKattribsAssign (node *arg_node)
{
    node *nlet;

    DBUG_ENTER ();

    nlet = ASSIGN_STMT (arg_node);
    if ((N_let == NODE_TYPE (nlet)) && (!CHKisAttribIds (LET_IDS (nlet), arg_node))) {
        NODE_ERROR (arg_node)
          = CHKinsertError (NODE_ERROR (arg_node),
                            "AVIS_SSAASSIGN does not point to correct N_assign node");
    }

    DBUG_RETURN (arg_node);
}

/** <!--**********************************************************************-->
 *
 * @fn node *CHKattribsIds( node *arg_node)
 *
 * @desc: Check all N_ids elements
 *
 *******************************************************************************/
node *
CHKattribsIds (node *arg_node)
{
    DBUG_ENTER ();

    if (!CHKisAttribIds (arg_node, NULL)) {
        DBUG_PRINT ("Offending N_ids is %s", AVIS_NAME (IDS_AVIS (arg_node)));
        NODE_ERROR (arg_node)
          = CHKinsertError (NODE_ERROR (arg_node), "AVIS_SSAASSIGN is invalid for N_ids");
    }

    DBUG_RETURN (arg_node);
}

/** <!--**********************************************************************-->
 *
 * @fn static bool isMemberVardecs( node *arg_node, node *fundef )
 *
 * @brief: Predicate to determine if N_avis arg_node
 *         is a member of fundef's N_vardec chain.
 *
 * @params: arg_node: The N_avis of interest.
 *          fundef: The N_fundef node for this fundef.
 *
 * @return: True if the N_avis is found; else false.
 *
 *****************************************************************************/
static bool
isMemberVardecs (node *arg_node, node *fundef)
{
    node *vardecs;
    bool z = FALSE;

    DBUG_ENTER ();

    vardecs = FUNDEF_BODY (fundef);
    if ((NULL != vardecs) && (NULL != BLOCK_VARDECS (vardecs))) {
        vardecs = BLOCK_VARDECS (vardecs);
        while ((!z) && NULL != vardecs) {
            if (arg_node == VARDEC_AVIS (vardecs)) {
                z = TRUE;
            } else {
                vardecs = VARDEC_NEXT (vardecs);
            }
        }
    }

    DBUG_RETURN (z);
}

/** <!--**********************************************************************-->
 *
 * @fn static bool isMemberArgs( node *arg_node, node *fundef )
 *
 * @brief: Predicate to determine if N_avis arg_node
 *         is a member of fundef's N_args chain.
 *
 * @params: arg_node: The N_avis of interest.
 *          fundef: The N_fundef node for this fundef.
 *
 * @return: True if the N_avis is found; else false.
 *
 *****************************************************************************/
static bool
isMemberArgs (node *arg_node, node *fundef)
{
    node *args;
    bool z = FALSE;

    DBUG_ENTER ();

    args = FUNDEF_ARGS (fundef);
    while ((!z) && NULL != args) {
        if (arg_node == ARG_AVIS (args)) {
            z = TRUE;
        } else {
            args = ARG_NEXT (args);
        }
    }

    DBUG_RETURN (z);
}

/** <!--**********************************************************************-->
 *
 * @fn node *CHKfundefVardecExtrema( node *arg_node)
 *
 * @brief: Check all vardecs in this function, to ensure that
 *         AVIS_MIN and AVIS_MAX point to a vardec in this function,
 *         or to one of the N_args.
 *
 *
 * @params: arg_node: The N_fundef node for this fundef.
 *
 * @return: arg_node
 *
 *****************************************************************************/
node *
CHKfundefVardecExtrema (node *arg_node)
{
    node *curvardec;
    node *minmax;
    node *vardecs;

    DBUG_ENTER ();

    if (NULL != arg_node) {

        vardecs = FUNDEF_BODY (arg_node);
        if ((NULL != vardecs) && (NULL != BLOCK_VARDECS (vardecs))) {
            vardecs = BLOCK_VARDECS (vardecs);
            curvardec = vardecs;
            while (NULL != curvardec) {
                minmax = AVIS_MIN (VARDEC_AVIS (curvardec));
                if ((NULL != minmax)
                    && (!(isMemberVardecs (ID_AVIS (minmax), arg_node)
                          || isMemberArgs (ID_AVIS (minmax), arg_node)))) {
                    DBUG_PRINT ("WARNING: AVIS_MIN(%s)= %s does not point to a "
                                "vardec/arg in fundef %s",
                                AVIS_NAME (VARDEC_AVIS (curvardec)),
                                AVIS_NAME (ID_AVIS (minmax)), FUNDEF_NAME (arg_node));
                }

                minmax = AVIS_MAX (VARDEC_AVIS (curvardec));
                if ((NULL != minmax)
                    && (!(isMemberVardecs (ID_AVIS (minmax), arg_node)
                          || isMemberArgs (ID_AVIS (minmax), arg_node)))) {
                    DBUG_PRINT ("WARNING: AVIS_MAX(%s)= %s does not point to an "
                                "vardec/arg in fundef %s",
                                AVIS_NAME (VARDEC_AVIS (curvardec)),
                                AVIS_NAME (ID_AVIS (minmax)), FUNDEF_NAME (arg_node));
                }
                curvardec = VARDEC_NEXT (curvardec);
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--**********************************************************************-->
 *
 * @fn node *CHKisNullSsaassign( node *arg_node)
 *
 * @brief: arg_node is an N_arg. It must have a NULL AVIS_SSAASSIGN.
 *
 *
 * @params: arg_node: N_arg.
 *
 * @return: arg_node
 *
 *****************************************************************************/
node *
CHKisNullSsaassign (node *arg_node)
{
    DBUG_ENTER ();

    if (NULL != AVIS_SSAASSIGN (ARG_AVIS (arg_node))) {
        NODE_ERROR (arg_node) = CHKinsertError (NODE_ERROR (arg_node),
                                                "non-NULL AVIS_SSAASSIGN in N_arg node");
    }

    DBUG_RETURN (arg_node);
}

/** <!--**********************************************************************-->
 *
 * @fn node *CHKcondfun( node *arg_node)
 *
 * @brief: arg_node is an N_fundef.
 *         If the function is a CONDFUN, then its
 *         first N_assign node must be an N_cond or N_funcond.
 *
 * @params: arg_node: N_fundef.
 *
 * @return: arg_node
 *
 *****************************************************************************/
node *
CHKcondfun (node *arg_node)
{
    node *assgn;

    DBUG_ENTER ();

    if (FUNDEF_ISCONDFUN (arg_node)) {
        assgn = BLOCK_ASSIGNS (FUNDEF_BODY (arg_node));
        DBUG_ASSERT (NULL != assgn, "Expected non-NULL BLOCK_ASSIGNS");
        assgn = ASSIGN_STMT (assgn);
        if (NULL == assgn) {
            NODE_ERROR (arg_node)
              = CHKinsertError (NODE_ERROR (arg_node), "Expected non-NULL ASSIGN_STMT");
        }
        if ((N_cond != NODE_TYPE (assgn)) && (N_funcond != NODE_TYPE (assgn))) {
            NODE_ERROR (arg_node)
              = CHKinsertError (NODE_ERROR (arg_node),
                                "No leading N_cond/N_funcond in CONDFUN");
            DBUG_PRINT ("Offending function is %s", FUNDEF_NAME (arg_node));
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--**********************************************************************-->
 *
 * @fn node *CHKavisReflection( node *arg_node)
 *
 * @brief: arg_node is an N_avis. Ensure that one of the following holds:
 *
 *    1. AVIS_DECL points to an N_vardec whose VARDEC_AVIS points to the avis.
 *    2. AVIS_DECL points to an N_arg whose ARG_AVIS points to the avis.
 *
 * @params: arg_node: N_avis.
 *
 * @return: arg_node
 *
 *****************************************************************************/
node *
CHKavisReflection (node *arg_node)
{
    node *arg;

    DBUG_ENTER ();

    if (NULL == AVIS_DECL (arg_node)) {
        NODE_ERROR (arg_node) = CHKinsertError (NODE_ERROR (arg_node), "NULL AVIS_DECL");
    }

    arg = AVIS_DECL (arg_node);

    if ((N_arg == NODE_TYPE (arg)) && (arg_node != ARG_AVIS (arg))) {
        NODE_ERROR (arg_node) = CHKinsertError (NODE_ERROR (arg_node),
                                                "AVIS_DECL and ARG_AVIS do not reflect");
    }
    if ((N_vardec == NODE_TYPE (arg)) && (arg_node != VARDEC_AVIS (arg))) {
        NODE_ERROR (arg_node)
          = CHKinsertError (NODE_ERROR (arg_node),
                            "AVIS_DECL and VARDEC_AVIS do not reflect");
    }

    DBUG_RETURN (arg_node);
}

/** <!--**********************************************************************-->
 *
 * @fn node *CHKavisSsaassignNodeType( node *arg_node)
 *
 * @brief: arg_node is an N_avis. If running in SSA mode,
 *         check that AVIS_SSAASSIGN is either NULL or
 *         a pointer to an N_assign node.
 *
 *         NB. This function can report an error falsely in the case
 *             where a variable exists, but has no definition,
 *             such as a dead variable in a copy WL eliminated
 *             by CWLE. (See apex/ipbb/ipbb.sac for an example.)
 *             I have no idea how to correct this shortcoming.
 *
 * @params: arg_node: N_avis.
 *
 * @return: arg_node
 *
 *****************************************************************************/
node *
CHKavisSsaassignNodeType (node *arg_node)
{
    node *ssaassign; // mostly for debug aid

    DBUG_ENTER ();

    ssaassign = AVIS_SSAASSIGN (arg_node);
    if ((NULL != ssaassign) && global.valid_ssaform
        && (N_assign != NODE_TYPE (ssaassign))) {
        NODE_ERROR (arg_node)
          = CHKinsertError (NODE_ERROR (arg_node), "Illegal node type in AVIS_SSAASSIGN");
        DBUG_PRINT ("Offending variable is %s", AVIS_NAME (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--**********************************************************************-->
 *
 * @fn node *CHKapArgCount( node *arg_node)
 *
 * @brief: arg_node is an N_ap.
 *         Ensure that the number of arguments in the call matches
 *         the number of arguments in the called function.
 *
 *         If this is a LOOPFUN, attempt to do the same for the
 *         recursive call.
 *
 *         The check of the recursive LACFUN call is made
 *         only when we are in LACFUN mode, and does not apply
 *         to functions, such as printf, that can take an arbitrary
 *         number of arguments.
 *
 * @params: arg_node: N_ap.
 *
 * @return: arg_node
 *
 *****************************************************************************/
node *
CHKapArgCount (node *arg_node)
{
    node *fundef_args;
    node *ap_args;
    size_t numapargs;
    size_t numfundefargs;

    DBUG_ENTER ();

    if ((!FUNDEF_HASDOTARGS (AP_FUNDEF (arg_node)))
        && (!FUNDEF_ISOBJINITFUN (AP_FUNDEF (arg_node)))
        && (!FUNDEF_ISWRAPPERFUN (AP_FUNDEF (arg_node)))) {
        fundef_args = FUNDEF_ARGS (AP_FUNDEF (arg_node));
        ap_args = AP_ARGS (arg_node);
        numapargs = TCcountExprs (ap_args);
        numfundefargs = TCcountArgs (fundef_args);
        if (numapargs != numfundefargs) {
            NODE_ERROR (arg_node)
              = CHKinsertError (NODE_ERROR (arg_node),
                                "Function parameter/argument count mismatch");
            DBUG_PRINT ("Offender %s has %zu parameters, call has %zu arguments",
                        FUNDEF_NAME (AP_FUNDEF (arg_node)), numfundefargs, numapargs);
        }

        if ((NULL != FUNDEF_LOOPRECURSIVEAP (AP_FUNDEF (arg_node)))
            && (global.compiler_anyphase >= PH_ptc_l2f)
            && (global.compiler_anyphase < PH_ussa_f2l)) {
            ap_args = AP_ARGS (FUNDEF_LOOPRECURSIVEAP (AP_FUNDEF (arg_node)));
            numapargs = TCcountExprs (ap_args);
            if (numapargs != numfundefargs) {
                NODE_ERROR (arg_node)
                  = CHKinsertError (NODE_ERROR (arg_node), "Loopfun recursive call "
                                                           "parameter/argument count "
                                                           "mismatch");
                DBUG_PRINT ("Offender %s has %zu parameters, call has %zu arguments",
                            FUNDEF_NAME (AP_FUNDEF (arg_node)), numfundefargs, numapargs);
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--**********************************************************************-->
 *
 * @fn node *CHKfundefReturn( node *arg_node)
 *
 * @brief: arg_node is an N_fundef.
 *         Ensure that the N_return in the fundef body
 *         is pointed to by the FUNDEF_RETURN attribute.
 *
 *         For better or worse, we ignore 0 == FUNDEF_RETURN().
 *
 *         We make this check only in LACFUN mode.
 *
 * @params: arg_node: N_fundef.
 *
 * @return: arg_node
 *
 *****************************************************************************/
node *
CHKfundefReturn (node *arg_node)
{
    node *assgn;
    node *ret = NULL;

    DBUG_ENTER ();

    if ((!FUNDEF_ISWRAPPERFUN (arg_node)) && (global.compiler_anyphase >= PH_ptc_l2f)) {
        assgn = FUNDEF_BODY (arg_node);
        if (NULL != assgn) { /* Some fns do not have a body. Weird... */
            assgn = BLOCK_ASSIGNS (assgn);
            while (NULL == ret) {
                if (N_return == NODE_TYPE (ASSIGN_STMT (assgn))) {
                    ret = ASSIGN_STMT (assgn);
                }
                assgn = ASSIGN_NEXT (assgn);
            }

            if ((NULL != FUNDEF_RETURN (arg_node)) && (FUNDEF_RETURN (arg_node) != ret)) {
                NODE_ERROR (arg_node)
                  = CHKinsertError (NODE_ERROR (arg_node),
                                    "Function's FUNDEF_RETURN is wrong");
                DBUG_PRINT ("Offending function is %s",
                            FUNDEF_NAME (AP_FUNDEF (arg_node)));
            }
        }
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
