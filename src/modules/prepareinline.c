/*
 *
 * $Log$
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
#include "traverse.h"
#include "deserialize.h"
#include "internal_lib.h"

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

    if ((FUNDEF_BODY (arg_node) == NULL) && (FUNDEF_ISINLINE (arg_node))
        && (FUNDEF_SYMBOLNAME (arg_node) != NULL)) {
        arg_node = DSdoDeserialize (arg_node);
        INFO_PPI_FETCHED (arg_info)++;
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

    do {
        DBUG_PRINT ("PPI", ("Starting round %d.", rounds));

        INFO_PPI_FETCHED (info) = 0;

        syntax_tree = TRAVdo (syntax_tree, info);

        DBUG_PRINT ("PPI", ("Finished round %d, fetched %d bodies.", rounds,
                            INFO_PPI_FETCHED (info)));

#ifndef DBUG_OFF
        rounds++;
#endif
    } while (INFO_PPI_FETCHED (info) != 0);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
