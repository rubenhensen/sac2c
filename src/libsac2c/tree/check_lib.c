/*
 * $Id$
 */

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
            CTIwarn ("%s", string);
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
 * @fn node *CHKexistAttribute( node *attribute, node *arg_node, char *string)
 *
 *******************************************************************************/
node *
CHKexistAttribute (void *attribute, node *arg_node, char *string)
{
    DBUG_ENTER ();

    if (attribute == NULL) {

        NODE_ERROR (arg_node) = CHKinsertError (NODE_ERROR (arg_node), string);
    }

    DBUG_RETURN (attribute);
}

/** <!--**********************************************************************-->
 *
 * @fn node *CHKnotExist( node *son_attribute, node *arg_node, char *string)
 *
 *******************************************************************************/

node *
CHKnotExist (void *son_attribute, node *arg_node, char *string)
{
    DBUG_ENTER ();

    if (son_attribute != NULL) {

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

    DBUG_RETURN (attribute);
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
 * @fn node *CHKassignAvisSSAAssign( node *arg_node)
 *
 *******************************************************************************/
node *
CHKassignAvisSSAAssign (node *arg_node)
{
    node *ids;

    DBUG_ENTER ();

    if (global.valid_ssaform) {
        if (NODE_TYPE (ASSIGN_STMT (arg_node)) == N_let) {
            ids = LET_IDS (ASSIGN_STMT (arg_node));
            while (ids != NULL) {
                if (AVIS_SSAASSIGN (IDS_AVIS (ids)) != arg_node) {
                    NODE_ERROR (IDS_AVIS (ids))
                      = CHKinsertError (NODE_ERROR (IDS_AVIS (ids)),
                                        "AVIS_SSAASSIGN does not point to correct "
                                        "N_assign node.");
                    DBUG_PRINT ("for %s", AVIS_NAME (IDS_AVIS (ids)));
                }
                ids = IDS_NEXT (ids);
            }
        }
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
 * @return: VOID;
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
 * @return: VOID;
 *
 *****************************************************************************/
node *
CHKisNullSsaassign (node *arg_node)
{

    DBUG_ENTER ();

    DBUG_ASSERT (NULL == AVIS_SSAASSIGN (ARG_AVIS (arg_node)),
                 "Non-NULL AVIS_SSAASSIGN in N_arg node");
    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
