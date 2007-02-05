/*
 * $Log$
 * Revision 1.2  2004/12/07 17:45:59  sah
 * fixed bug
 *
 * Revision 1.1  2004/11/29 14:40:57  sah
 * Initial revision
 *
 *
 *
 */

#include "setlinksign.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "internal_lib.h"
#include "str.h"
#include "memory.h"

/*
 * INFO structure
 */
struct INFO {
    int counter;
};

/*
 * INFO macros
 */
#define INFO_SLS_COUNTER(n) ((n)->counter)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_SLS_COUNTER (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

node *
SLSret (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SLSret");

    INFO_SLS_COUNTER (arg_info)++;

    if (!RET_HASLINKSIGNINFO (arg_node)) {
        RET_LINKSIGN (arg_node) = INFO_SLS_COUNTER (arg_info);
    }

    if (RET_NEXT (arg_node) != NULL) {
        RET_NEXT (arg_node) = TRAVdo (RET_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
SLSarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SLSarg");

    INFO_SLS_COUNTER (arg_info)++;

    if (!ARG_HASLINKSIGNINFO (arg_node)) {
        ARG_LINKSIGN (arg_node) = INFO_SLS_COUNTER (arg_info);
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
SLSfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SLSfundef");

    INFO_SLS_COUNTER (arg_info) = 0;

    if (FUNDEF_RETS (arg_node) != NULL) {
        FUNDEF_RETS (arg_node) = TRAVdo (FUNDEF_RETS (arg_node), arg_info);
    }

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
SLSmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SLSmodule");

    if (MODULE_FUNDECS (arg_node) != NULL) {
        MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
    }

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
SLSdoSetLinksign (node *tree)
{
    info *info;

    DBUG_ENTER ("SLSdoSetLinksign");

    info = MakeInfo ();

    TRAVpush (TR_sls);

    tree = TRAVdo (tree, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (tree);
}
