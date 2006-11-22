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

/* these are not needed yet, maybe we will need them some day.
#include "print.h"
#include "new_types.h"
*/

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/

struct INFO {
    node *resshape;  /* The N_avis of the array in our generator */
    node *partarray; /* The N_avis of our Parts, if they consist
                        of just one assignment. */
    node *iv;        /* The vector of our wl-parts */
    bool valid;      /* Do we have a valid case for cwle? */
};

/**
 * Macros for accessing the info-structure
 */
#define INFO_RESSHAPE(n) (n->resshape)
#define INFO_PARTARRAY(n) (n->partarray)
#define INFO_IV(n) (n->iv)
#define INFO_VALID(n) (n->valid)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_VALID (result) = FALSE;
    INFO_RESSHAPE (result) = NULL;
    INFO_PARTARRAY (result) = NULL;
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
        TRAVdo (FUNDEF_BODY (arg_node), arg_info);
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
    INFO_RESSHAPE (arg_info) = NULL;
    INFO_PARTARRAY (arg_info) = NULL;

    /*
     * get the shape of our lhs
     */

    if (NULL != LET_IDS (arg_node)) {
        INFO_RESSHAPE (arg_info) = AVIS_SHAPE (IDS_AVIS (LET_IDS (arg_node)));
    } else {
        INFO_VALID (arg_info) = FALSE;
    }

    /*
     * if we got a shape, traverse into the loop, if there is one
     */

    if (INFO_VALID (arg_info)) {
        TRAVdo (LET_EXPR (arg_node), arg_info);

        if (INFO_VALID (arg_info) && NULL != INFO_PARTARRAY (arg_info)
            && INFO_RESSHAPE (arg_info)
                 == AVIS_SHAPE (ID_AVIS (INFO_PARTARRAY (arg_info)))) {

            /*
             * we now can safely replace the with by partarray
             */

            DBUG_PRINT ("CWLE", ("found target. let is replacing expr-node..."));
            DBUG_PRINT ("CWLE", ("partarray is of type %i",
                                 NODE_TYPE (INFO_PARTARRAY (arg_info))));

            LET_EXPR (arg_node) = DUPdoDupNode (INFO_PARTARRAY (arg_info));
        }
    }

    INFO_VALID (arg_info) = FALSE;

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
    DBUG_ENTER ("CWLEwith");
    DBUG_PRINT ("CWLE", ("Calling CWLEwith"));

    if (INFO_VALID (arg_info)) {

        DBUG_PRINT ("CWLE", ("with_vec is of type %i", NODE_TYPE (WITH_VEC (arg_node))));

        INFO_IV (arg_info) = IDS_AVIS (WITH_VEC (arg_node));

        TRAVdo (WITH_CODE (arg_node), arg_info);
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
    node *nlet;
    node *target = NULL;

    DBUG_ENTER ("CWLEcode");
    DBUG_PRINT ("CWLE", ("Calling CWLEcode"));

    if (INFO_VALID (arg_info)) {

        /*
         * take a look for our return value
         */

        ravis = ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (arg_node)));
        DBUG_PRINT ("CWLE", ("type of ravis: %i", NODE_TYPE (ravis)));

        /*
         * oke, check if our first assign is a n_let
         * you have to check for empty nodes first, though
         */

        if (N_empty != NODE_TYPE (BLOCK_INSTR (CODE_CBLOCK (arg_node)))
            && N_let == NODE_TYPE (ASSIGN_INSTR (BLOCK_INSTR (CODE_CBLOCK (arg_node))))) {
            nlet = ASSIGN_INSTR (BLOCK_INSTR (CODE_CBLOCK (arg_node)));

            /*
             * check, if the let assigns something to our ravis
             */

            if (IDS_AVIS (LET_IDS (nlet)) == ravis) {
                DBUG_PRINT ("CWLE", ("our nlet assigns to the ravis!"));

                /*
                 * is our let assigning a prf, and what are its parameters?
                 */

                DBUG_PRINT ("CWLE", ("nlet->expr type: %i", NODE_TYPE (LET_EXPR (nlet))));

                if (N_prf == NODE_TYPE (LET_EXPR (nlet))
                    && F_sel == PRF_PRF (LET_EXPR (nlet))
                    && N_id == NODE_TYPE (EXPRS_EXPR (PRF_ARGS (LET_EXPR (nlet))))) {

                    /*
                     * this thing has to be equal to iv in the "( . <= iv <= .)"
                     */

                    DBUG_PRINT ("CWLE", ("checking for iv ..."));

                    if (INFO_IV (arg_info)
                        == ID_AVIS (EXPRS_EXPR (PRF_ARGS (LET_EXPR (nlet))))) {

                        /*
                         * check for the second argument to prf; this is our array
                         * that we copy from
                         */

                        DBUG_PRINT ("CWLE", ("checking for target array ..."));

                        if (NULL != EXPRS_NEXT (PRF_ARGS (LET_EXPR (nlet)))) {
                            target = EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (LET_EXPR (nlet))));
                        }
                    }
                }
            }
        }

        if (NULL != target) {
            DBUG_PRINT ("CWLE", ("target found."));

            /*
             * oke, we got a target, now we have to check if its our target
             */

            if (NULL == INFO_PARTARRAY (arg_info)) {
                INFO_PARTARRAY (arg_info) = target;
            } else if (target != INFO_PARTARRAY (arg_info)) {

                /*
                 * if the target is not equal to a previous target, skip this wl
                 */

                INFO_VALID (arg_info) = FALSE;
            }
        }
    }

    if (INFO_VALID (arg_info) && CODE_NEXT (arg_node) != NULL) {
        TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/
