/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup cwle Copy with-loop elimination
 *
 * This traversal takes care of with-loops that do nothing more
 * than copying some array. These are replaced by A = B, like in
 *
 * B = with { (.<=iv<=.) : A[iv]; } : genarray( shape(A), n );
 * will be transformed to
 * B = A;
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
#include "dbug.h"
#include "internal_lib.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "DupTree.h"
#include "free.h"
#include "compare_tree.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/

struct INFO {
    node *lhs;  /* The N_avis of the array in our generator */
    node *rhs;  /* The N_avis of our Parts, if they consist
                         of just one assignment. */
    node *iv;   /* The vector of our wl-parts */
    bool valid; /* Do we have a valid case for cwle? */
};

#define INFO_LHS(n) (n->lhs)
#define INFO_RHS(n) (n->rhs)
#define INFO_IV(n) (n->iv)
#define INFO_VALID(n) (n->valid)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_VALID (result) = FALSE;
    INFO_LHS (result) = NULL;
    INFO_RHS (result) = NULL;
    INFO_IV (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

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
 * @fn node *CWLEdoTemplateTraversal( node *syntax_tree)
 *
 *****************************************************************************/
node *
CWLEdoTemplateTraversal (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("CWLEdoTemplateTraversal");

    info = MakeInfo ();

    DBUG_PRINT ("CWLE", ("Starting template traversal."));

    TRAVpush (TR_cwle);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_PRINT ("CWLE", ("Template traversal complete."));

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
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
 * @brief Just traverses into the function-body.
 *
 * This needs to be overwritten, as the generated version does not traverse
 * into the function body.
 *
 *****************************************************************************/

node *
CWLEfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CWLEfundef");
    DBUG_PRINT ("CWLE", ("Calling CWLEfundef"));

    if (NULL != FUNDEF_BODY (arg_node)) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CWLElet(node *arg_node, info *arg_info)
 *
 * @brief checks for the shape of its LHS and passes it on
 *
 * This function checks for the shape of the identifier, that is placed
 *   on its left hand side. We need this information to check, if the array,
 *   that we copy from, is the same size as the array that we would like
 *   to copy into.
 *
 *****************************************************************************/

node *
CWLElet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CWLElet");
    DBUG_PRINT ("CWLE", ("Calling CWLElet"));

    INFO_VALID (arg_info) = TRUE;
    INFO_LHS (arg_info) = NULL;
    INFO_RHS (arg_info) = NULL;
    INFO_IV (arg_info) = NULL;

    /*
     * save the ids of our let and traverse into the epression, hoping
     * that it is a with-loop.
     */

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    /*
     * we are through, everything is false now.
     */

    INFO_VALID (arg_info) = FALSE;
    INFO_LHS (arg_info) = NULL;
    INFO_RHS (arg_info) = NULL;
    INFO_IV (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CWLEwith(node *arg_node, info *arg_info)
 *
 * @brief redirects the checks into the N_code nodes
 *
 * Here we just check, if we are inside a cwle, and if, redirect control
 *   to the N_code nodes, to check for copying action.
 *
 *****************************************************************************/

node *
CWLEwith (node *arg_node, info *arg_info)
{
    node *target;

    DBUG_ENTER ("CWLEwith");
    DBUG_PRINT ("CWLE", ("Calling CWLEwith"));

    /*
     * save the index-vector of our with-loop and
     * traverse into the codes.
     */

    DBUG_PRINT ("CWLE", ("traversing codes ..."));

    INFO_IV (arg_info) = IDS_AVIS (WITH_VEC (arg_node));
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    DBUG_PRINT ("CWLE", ("finished traversing codes."));

    /*
     * if our codes indicate that we are still on the run
     * we have to check, if this is a one-generator-loop.
     */

    if (INFO_VALID (arg_info) && NULL == IDS_NEXT (INFO_LHS (arg_info))) {

        /*
         * from now on, we just want to know about the LHS-Avis.
         */

        INFO_LHS (arg_info) = IDS_AVIS (INFO_LHS (arg_info));

        /*
         * we may now compare shape size.
         */

        DBUG_PRINT ("CWLE", ("codes signal valid. comparing shapes ..."));
        DBUG_PRINT ("CWLE", ("lhs is of type %i", NODE_TYPE (INFO_LHS (arg_info))));
        DBUG_PRINT ("CWLE", ("rhs is of type %i", NODE_TYPE (INFO_RHS (arg_info))));

        if (NULL != AVIS_SHAPE (INFO_LHS (arg_info))) {
            DBUG_PRINT ("CWLE", ("LHS-shape is not null."));

            if (NULL != AVIS_SHAPE (INFO_RHS (arg_info))) {
                DBUG_PRINT ("CWLE", ("RHS-shape is not null."));

                if (CMPT_EQ
                    == CMPTdoCompareTree (AVIS_SHAPE (INFO_LHS (arg_info)),
                                          AVIS_SHAPE (INFO_RHS (arg_info)))) {

                    DBUG_PRINT ("CWLE", ("shapes are equal. replacing loop..."));

                    /*
                     * 3 steps:
                     * 1. create a new N_id with our rhs-array inside.
                     * 2. free the with-loop, we do not need it anymore.
                     * 3. return the brandnew N_id.
                     */

                    target = TBmakeId (DUPdoDupNode (INFO_RHS (arg_info)));
                    FREEdoFreeTree (arg_node);
                    DBUG_RETURN (target);
                }
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CWLEcode(node *arg_node, info *arg_info)
 *
 * @brief does the banana-dance ;)
 *
 *****************************************************************************/

node *
CWLEcode (node *arg_node, info *arg_info)
{
    node *ravis;
    node *target = NULL;
    info *subinfo;

    DBUG_ENTER ("CWLEcode");
    DBUG_PRINT ("CWLE", ("Calling CWLEcode"));

    /*
     * create a new info-structure for traversing the code-block,
     * traverse, and release the info-structure.
     * this needs to be done in order to keep the info-structures
     * seperate in the case of nested with-loops.
     */

    subinfo = MakeInfo ();
    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), subinfo);
    subinfo = FreeInfo (subinfo);

    /*
     * before checking ourselves, lets traverse into the next
     * code, so we can search therein for other with-loops.
     */

    if (NULL != CODE_NEXT (arg_node)) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    /*
     * if we still have a valid cwle-case, check if we do agree with that.
     */

    if (INFO_VALID (arg_info)) {
        DBUG_PRINT ("CWLE", ("previous nodes signal ok..."));

        /*
         * we have to check for:
         * code->cexprs->expr-(id)>avis->ssaassign->n_assign->n_let->expr->prf existing?
         * prf of type F_sel?
         * prf->args->expr-(id)>avis == with_iv?
         * target = prf->args->next->expr-(id)>avis
         */

        if (N_id == NODE_TYPE (ravis = EXPRS_EXPR (CODE_CEXPRS (arg_node)))) {
            DBUG_PRINT ("CWLE", ("code_cexprs is of type id"));

            if (N_let
                == NODE_TYPE (ravis = ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (ravis))))) {
                DBUG_PRINT ("CWLE", ("cexprs is defined at a n_let expression"));

                if (N_prf == NODE_TYPE (ravis = LET_EXPR (ravis))) {
                    DBUG_PRINT ("CWLE", ("cexprs is created by a n_prf"));

                    if (F_sel == PRF_PRF (ravis)) {
                        DBUG_PRINT ("CWLE", ("the prf is of type F_sel"));

                        if (INFO_IV (arg_info)
                            == ID_AVIS (EXPRS_EXPR (PRF_ARGS (ravis)))) {
                            DBUG_PRINT ("CWLE", ("prf selects at iv"));

                            target = EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (ravis)));
                        }
                    }
                }
            }
        }
    } else {
        DBUG_PRINT ("CWLE", ("previous nodes signal NO ok"));
    }

    if (NULL != ID_AVIS (target)) {
        DBUG_PRINT ("CWLE", ("found a target."));
    } else {
        INFO_VALID (arg_info) = FALSE;
    }

    /*
     * if we have found some avis, that meets the requirements, then
     * lets check if it is the same that we have found before. If we
     * do not have found anything before, assign it to INFO_RHS.
     */

    if (INFO_VALID (arg_info)) {
        DBUG_PRINT ("CWLE", ("checking if target is legitimate ..."));

        if (NULL == INFO_RHS (arg_info) || target == INFO_RHS (arg_info)) {
            DBUG_PRINT ("CWLE", ("target is valid. saving"));

            INFO_RHS (arg_info) = target;
        } else {
            DBUG_PRINT ("CWLE", ("target is NOT valid. skipping wl"));

            INFO_VALID (arg_info) = FALSE;
            INFO_RHS (arg_info) = NULL;
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
