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
#include "dbug.h"
#include "tree_basic.h"
#include "deserialize.h"
#include "namespaces.h"
#include "ctinfo.h"
#include "str.h"
#include "globals.h"

node *
ASFdoAddSpecialFunctions (node *syntaxtree)
{
    DBUG_ENTER ("ASFdoAddSpecialFunctions");

    DBUG_ASSERT ((NODE_TYPE (syntaxtree) == N_module),
                 "ASFdoAddSpecialFunctions is designed for use on module nodes!");

    DSinitDeserialize (syntaxtree);

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

        DSaddSymbolByName ("sel", SET_wrapperhead, global.preludename);
        DSaddSymbolByName ("zero", SET_wrapperhead, global.preludename);
        DSaddSymbolByName ("eq", SET_wrapperhead, global.preludename);
        DSaddSymbolByName ("adjustLacFunParams", SET_wrapperhead, global.preludename);
        DSaddSymbolByName ("adjustLacFunParamsReshape", SET_wrapperhead,
                           global.preludename);
    } else {
        CTInote ("The prelude library `%s' has not been loaded.", global.preludename);
    }

    DSfinishDeserialize (syntaxtree);

    DBUG_RETURN (syntaxtree);
}
