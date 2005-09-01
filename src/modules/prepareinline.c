/*
 *
 * $Log$
 * Revision 1.9  2005/09/01 12:19:51  sah
 * streamlined it a bit
 *
 * Revision 1.8  2005/08/09 12:18:01  sah
 * fixed a DBUG message
 *
 * Revision 1.7  2005/07/22 13:11:39  sah
 * interface changes
 *
 * Revision 1.6  2005/07/15 15:57:02  sah
 * introduced namespaces
 *
 * Revision 1.5  2005/05/25 20:27:16  sah
 * bodies of lacfuns are fetched now as well
 *
 * Revision 1.4  2005/05/25 19:11:35  sah
 * prepareinline disabled if inline disabled as well
 *
 * Revision 1.3  2005/05/17 16:20:50  sah
 * added some reasonable error messages
 *
 * Revision 1.2  2004/11/25 12:00:27  sah
 * COMPILES
 *
 * Revision 1.1  2004/10/28 17:53:50  sah
 * Initial revision
 *
 *
 *
 */

#define NEW_INFO

#include "prepareinline.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "add_function_body.h"
#include "deserialize.h"
#include "internal_lib.h"
#include "type_utils.h"
#include "namespaces.h"

/*
 * INFO structure
 */
struct INFO {
    node *module;
    int fetched;
};

/*
 * INFO macros
 */
#define INFO_PPI_MODULE(n) ((n)->module)
#define INFO_PPI_FETCHED(n) ((n)->fetched)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_PPI_MODULE (result) = NULL;
    INFO_PPI_FETCHED (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

node *
PPIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PPIfundef");

    DBUG_PRINT ("PPI", ("processing '%s'", CTIitemName (arg_node)));
    /*
     * we fetch bodies for functions which are inline,
     * thus the body is needed for inlining
     */
    if ((FUNDEF_BODY (arg_node) == NULL) && (FUNDEF_ISINLINE (arg_node))
        && (FUNDEF_SYMBOLNAME (arg_node) != NULL)) {
        arg_node = AFBdoAddFunctionBody (arg_node);

        if (FUNDEF_BODY (arg_node) != NULL) {
            INFO_PPI_FETCHED (arg_info)++;

            DBUG_PRINT ("PPI",
                        ("fetched function body for '%s'", CTIitemName (arg_node)));
        } else {
            char *funsig = TUtypeSignature2String (arg_node);

            CTIerror ("Unable to find body of function '%s:%s' with args '%s' in module.",
                      NSgetName (FUNDEF_NS (arg_node)), FUNDEF_NAME (arg_node), funsig);

            funsig = ILIBfree (funsig);
        }
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
PPImodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PPImodul");

    DSinitDeserialize (arg_node);

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DSfinishDeserialize (arg_node);

    DBUG_RETURN (arg_node);
}

node *
PPIdoPrepareInline (node *syntax_tree)
{
    info *info;
#ifndef DBUG_OFF
    int rounds = 0;
#endif

    DBUG_ENTER ("PPIdoPrepareInline");

    info = MakeInfo ();

    TRAVpush (TR_ppi);

    if (global.optimize.doinl) {
        do {
            DBUG_PRINT ("PPI", ("Starting round %d.", rounds));

            INFO_PPI_FETCHED (info) = 0;

            syntax_tree = TRAVdo (syntax_tree, info);

            DBUG_PRINT ("PPI", ("Finished round %d, fetched %d bodies.", rounds,
                                INFO_PPI_FETCHED (info)));

#ifndef DBUG_OFF
            rounds++;
#endif

            CTIabortOnError ();
        } while (INFO_PPI_FETCHED (info) != 0);
    } else {
        DBUG_PRINT ("PPI", ("skipping PPI as inlining is disabled..."));
    }

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
