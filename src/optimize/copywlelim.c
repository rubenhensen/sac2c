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
    node *genshape;  /* The N_avis of the array in our generator */
    node *partarray; /* The N_avis of our Parts, if they consist
                        of just one assignment. */
    bool valid;      /* Do we have a valid case for cwle? */
};

/**
 * Macros for accessing the info-structure
 */
#define INFO_GENSHAPE(n) (n->genshape)
#define INFO_PARTARRAY(n) (n->partarray)
#define INFO_VALID(n) (n->valid)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_VALID (result) = FALSE;
    INFO_GENSHAPE (result) = NULL;
    INFO_PARTARRAY (result) = NULL;

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

    TRAVdo (FUNDEF_BODY (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CWLEwith(node *arg_node, info *arg_info)
 *
 * @brief checks if we just copy an array.
 *
 * This function checks, if the with-loop is just copiing an array.
 * For that we need several attributes:
 *
 * At first, we have a look at all our parts. If they just consist of a
 *   copying of some other array, we point INFO_PARTARRAY to this array.
 *   If they do not, then INFO_VALID has to be set to false.
 *
 * As second, we look at our generator. If this is a modarray or a genarray,
 *   we assign the shape to INFO_GENSHAPE, else INFO_VALID is set to false.
 *   If we are still in, then we check the shapes of INFO_GENSHAPE and
 *   INFO_PARTARRAY, and, on equality, replace the with-loop with the
 *   INFO_PARTARRAY.
 *
 *****************************************************************************/

node *
CWLEwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CWLEwith");
    DBUG_PRINT ("CWLE", ("Calling CWLEwith"));

    INFO_VALID (arg_info) = TRUE;
    INFO_GENSHAPE (arg_info) = NULL;
    INFO_PARTARRAY (arg_info) = NULL;

    /*
     * first part: check the parts.
     */

    /*
    TRAVdo( WITH_PART( arg_node ), arg_info );
    */

    /*
     * third part: check the shapes we got.
     */

    if (INFO_VALID (arg_info)) {
    }

    INFO_VALID (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/
