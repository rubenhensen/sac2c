/* $Id$ */

#include "load_module_contents.h"

#include "dbug.h"

node *
LMCdoLoadModuleContents (node *syntax_tree)
{
    DBUG_ENTER ("LMCdoLoadModuleContents");

    DBUG_ASSERT ((syntax_tree == NULL),
                 "SMCdoLoadModuleContents can only be called as long as no syntax tree "
                 "has been created!");

    DBUG_RETURN (syntax_tree);
}
