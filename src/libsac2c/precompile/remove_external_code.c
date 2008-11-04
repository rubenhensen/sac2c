/* $Id$ */

#include "remove_external_code.h"

#include "tree_basic.h"
#include "traverse.h"
#include "ctinfo.h"
#include "free.h"
#include "DataFlowMask.h"
#include "dbug.h"

node *
RECfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RECfundef");

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    if ((!FUNDEF_ISLOCAL (arg_node)) && (FUNDEF_BODY (arg_node) != NULL)) {
        /*
         * this fundef comes from another module and
         * an compiled version is already present within
         * that module, so we do not need to compile it
         * again. So we remove the body here
         */

        DBUG_PRINT ("REC", ("removing fundef body of external fundef %s...",
                            CTIitemName (arg_node)));
        /*
         * we set the nasty FUNDEF_RETURN pointer to NULL as we free its
         * target!
         */
        FUNDEF_RETURN (arg_node) = NULL;

        /*
         * free the body
         */
        FUNDEF_BODY (arg_node) = FREEdoFreeTree (FUNDEF_BODY (arg_node));
    }

    DBUG_RETURN (arg_node);
}

node *
RECdoRemoveExternalCode (node *tree)
{
    DBUG_ENTER ("RECdoRemoveExternalCode");

    DBUG_PRINT ("REC", (">>> starting to remove unused external code"));

    DBUG_ASSERT ((NODE_TYPE (tree) = N_module),
                 "RECdoRemoveExternalCode is intended to be called on modules");

    TRAVpush (TR_rec);

    tree = TRAVdo (tree, NULL);

    TRAVpop ();

    DBUG_PRINT ("REC", (">>> finished removing unused external code"));

    DBUG_RETURN (tree);
}
