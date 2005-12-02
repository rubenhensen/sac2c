/*
 * $Id$
 */

#include "free_dispatch_information.h"

#include "tree_basic.h"
#include "free.h"
#include "new_types.h"
#include "dbug.h"

/** <!-- ****************************************************************** -->
 * @brief frees the wrapper type from each wrapper in the given fundef
 *        chain. furthermore, all type error functions are freed.
 *
 * @param fundefs a N_fundef chain
 *
 * @return cleaned up N_fundef chain
 ******************************************************************************/
static node *
RemoveFromFundefChain (node *fundefs)
{
    DBUG_ENTER ("RemoveFromFundefChain");

    if (fundefs != NULL) {
        FUNDEF_NEXT (fundefs) = RemoveFromFundefChain (FUNDEF_NEXT (fundefs));

        if (FUNDEF_WRAPPERTYPE (fundefs) != NULL) {
            FUNDEF_WRAPPERTYPE (fundefs) = TYfreeType (FUNDEF_WRAPPERTYPE (fundefs));
        }

        if (FUNDEF_ISTYPEERROR (fundefs)) {
            fundefs = FREEdoFreeNode (fundefs);
        }
    }

    DBUG_RETURN (fundefs);
}

/** <!-- ****************************************************************** -->
 * @fn node *FDIdoFreeDispatchInformation( node *module)
 *
 * @brief Free the wrapper type attached to each wrapper fun and removes
 *        all type errror functions.
 *
 * @param module the N_module node
 *
 * @return cleaned up N_module node
 ******************************************************************************/
node *
FDIdoFreeDispatchInformation (node *module)
{
    DBUG_ENTER ("FDIdoFreeDispatchInformation");

    MODULE_FUNS (module) = RemoveFromFundefChain (MODULE_FUNS (module));

    MODULE_FUNDECS (module) = RemoveFromFundefChain (MODULE_FUNDECS (module));

    /*
     * get rid of alle the created zombie funs.
     * we have to call this explicitly as we are no traversal...
     */
    if (MODULE_FUNS (module) != NULL) {
        MODULE_FUNS (module) = FREEremoveAllZombies (MODULE_FUNS (module));
    }
    if (MODULE_FUNDECS (module) != NULL) {
        MODULE_FUNDECS (module) = FREEremoveAllZombies (MODULE_FUNDECS (module));
    }

    DBUG_RETURN (module);
}
