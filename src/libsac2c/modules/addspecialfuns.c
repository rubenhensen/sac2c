/*
 * $Id$
 *
 */

/*
 * this file contains the helper functions ASFdoAddSpecialFunctions
 * which is intended to load all the sac2c helper functions
 * from the special sacprelude module, that will be used later on.
 * Unfortunately, deadfunctionremoval will remove these during
 * the optimisations, so make sure that they are being used
 * prior to that phase
 */

#include "addspecialfuns.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "deserialize.h"
#include "namespaces.h"
#include "ctinfo.h"
#include "str.h"
#include "globals.h"
#include "map_fun_trav.h"

static node *
TagNamespaceAsSticky (node *fundef, namespace_t *ns)
{
    DBUG_ENTER ();

    if (!FUNDEF_ISLACFUN (fundef) && NSequals (FUNDEF_NS (fundef), ns)) {
        FUNDEF_ISSTICKY (fundef) = TRUE;
    }

    DBUG_RETURN (fundef);
}

static node *
TagPreludeAsSticky (node *syntax_tree)
{
    namespace_t *prelude;

    DBUG_ENTER ();

    prelude = NSgetNamespace (global.preludename);

    syntax_tree
      = MFTdoMapFunTrav (syntax_tree, (info *)prelude, (travfun_p)TagNamespaceAsSticky);

    prelude = NSfreeNamespace (prelude);

    DBUG_RETURN (syntax_tree);
}

node *
ASFdoAddSpecialFunctions (node *syntaxtree)
{
    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (syntaxtree) == N_module,
                 "ASFdoAddSpecialFunctions is designed for use on module nodes!");

    if (global.loadprelude) {
        /*
         * add functions from sac prelude
         * if we currently compile a module of the same name,
         * give an error!
         */
        if (STReq (global.modulename, global.preludename)) {
            CTIabort ("Cannot load `%s' when compiling a module of the same "
                      "name. Try compiling with option -noprelude!");
        }

        DSinitDeserialize (syntaxtree);

        DSaddSymbolByName ("sel", SET_wrapperhead, global.preludename);
        DSaddSymbolByName ("zero", SET_wrapperhead, global.preludename);
        DSaddSymbolByName ("eq", SET_wrapperhead, global.preludename);
        DSaddSymbolByName ("adjustLacFunParams", SET_wrapperhead, global.preludename);
        DSaddSymbolByName ("prod", SET_wrapperhead, global.preludename);
        DSaddSymbolByName ("adjustLacFunParamsReshape", SET_wrapperhead,
                           global.preludename);
        DSaddSymbolByName ("partitionSlicer", SET_wrapperhead, global.preludename);
        DSaddSymbolByName ("gridFiller", SET_wrapperhead, global.preludename);
        DSaddSymbolByName ("partitionIntersectMax", SET_wrapperhead, global.preludename);
        DSaddSymbolByName ("partitionIntersectMin", SET_wrapperhead, global.preludename);
        DSaddSymbolByName ("isPartitionIntersectNull", SET_wrapperhead,
                           global.preludename);

        DSfinishDeserialize (syntaxtree);

        /*
         * prevent prelude functions from being deleted
         */
        syntaxtree = TagPreludeAsSticky (syntaxtree);
    } else {
        CTInote ("The prelude library `%s' has not been loaded.", global.preludename);
    }

    DBUG_RETURN (syntaxtree);
}

#undef DBUG_PREFIX
