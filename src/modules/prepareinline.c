/*
 *
 * $Log$
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

    result = Malloc (sizeof (info));

    INFO_PPI_MODULE (result) = NULL;
    INFO_PPI_FETCHED (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

node *
PPIFundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PPIFundef");

    if ((FUNDEF_BODY (arg_node) == NULL) && (FUNDEF_INLINE (arg_node))
        && (FUNDEF_SYMBOLNAME (arg_node) != NULL)) {
        arg_node = AddFunctionBodyToHead (arg_node);
        INFO_PPI_FETCHED (arg_info)++;
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
PPIModul (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PPIModul");

    InitDeserialize (arg_node);

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    FinishDeserialize (arg_node);

    DBUG_RETURN (arg_node);
}

void
PrepareInline (node *syntax_tree)
{
    funtab *store_tab;
    info *info;
#ifndef DBUG_OFF
    int rounds = 0;
#endif

    DBUG_ENTER ("PrepareInline");

    info = MakeInfo ();

    store_tab = act_tab;
    act_tab = ppi_tab;

    do {
        DBUG_PRINT ("PPI", ("Starting round %d.", rounds));

        INFO_PPI_FETCHED (info) = 0;

        syntax_tree = Trav (syntax_tree, info);

        DBUG_PRINT ("PPI", ("Finished round %d, fetched %d bodies.", rounds,
                            INFO_PPI_FETCHED (info)));

        DBUG_EXECUTE ("PPI", INFO_PPI_FETCHED (info)++;);
    } while (INFO_PPI_FETCHED (info) != 0);

    act_tab = store_tab;

    info = FreeInfo (info);

    DBUG_VOID_RETURN;
}
