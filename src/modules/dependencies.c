/*
 *
 * $Log$
 * Revision 1.2  2004/11/25 21:19:24  sah
 * COMPILES
 *
 * Revision 1.1  2004/11/08 19:04:54  sah
 * Initial revision
 *
 *
 *
 */

#include "dependencies.h"
#include "stringset.h"
#include "tree_basic.h"
#include "modulemanager.h"
#include "dbug.h"

static void *
BuildDepClosFoldFun (const char *entry, strstype_t kind, void *rest)
{
    stringset_t *result = NULL;
    module_t *module;

    DBUG_ENTER ("BuildDepClosFoldFun");

    if (kind == STRS_saclib) {
        module = MODMloadModule (entry);
        result = MODMgetDependencyTable (module);
        module = MODMunLoadModule (module);
    }

    result = STRSjoin ((stringset_t *)rest, result);

    DBUG_RETURN ((void *)result);
}

static stringset_t *
BuildDependencyClosure (stringset_t *deps)
{
    stringset_t *result;

    DBUG_ENTER ("BuildDependencyClosure");

    result = STRSfold (&BuildDepClosFoldFun, deps, NULL);

    result = STRSjoin (result, deps);

    DBUG_RETURN (result);
}

node *
DEPdoResolveDependencies (node *syntax_tree)
{
    DBUG_ENTER ("DEPdesolveDependencies");

    MODULE_DEPENDENCIES (syntax_tree)
      = BuildDependencyClosure (MODULE_DEPENDENCIES (syntax_tree));

    DBUG_RETURN (syntax_tree);
}
