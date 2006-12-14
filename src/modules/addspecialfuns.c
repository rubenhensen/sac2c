/*
 * $Id$
 *
 */

/*
 * this file contains the helper functions ASFdoAddSpecialFunctions
 * which is intended to load all the sac2c helper functions
 * from the special sac2c module, that will be used later on.
 * Unfortunately, deadfunctionremoval will remove these during
 * the optimisations, so make sure that they are being used
 * prior to that phase
 */

#include "addspecialfuns.h"
#include "dbug.h"
#include "tree_basic.h"
#include "deserialize.h"
#include "internal_lib.h"
#include "namespaces.h"
#include "ctinfo.h"

node *
ASFdoAddSpecialFunctions (node *syntaxtree)
{
    DBUG_ENTER ("ASFdoAddSpecialFunctions");

    DBUG_ASSERT ((NODE_TYPE (syntaxtree) == N_module),
                 "ASFdoAddSpecialFunctions is desined for use on module nodes!");

    DSinitDeserialize (syntaxtree);

    /*
     * add sac2c::sel and sac2c::zero as used by wlpg
     * except if this already is module sac2c ;)
     */

    if (!ILIBstringCompare ("sac2c", NSgetModule (MODULE_NAMESPACE (syntaxtree)))) {
        DSaddSymbolByName ("sel", SET_wrapperhead, "sac2c");
        DSaddSymbolByName ("zero", SET_wrapperhead, "sac2c");
        DSaddSymbolByName ("eq", SET_wrapperhead, "sac2c");
        DSaddSymbolByName ("adjustLacFunParams", SET_wrapperhead, "sac2c");
        DSaddSymbolByName ("adjustLacFunParamsReshape", SET_wrapperhead, "sac2c");
    } else {
        CTInote ("compiling module `sac2c', no prelude functions loaded");
    }

    DSfinishDeserialize (syntaxtree);

    DBUG_RETURN (syntaxtree);
}
