/*
 *
 * $Log$
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
BuildDepClosFoldFun (const char *entry, SStype_t kind, void *rest)
{
    stringset_t *result = NULL;
    module_t *module;

    DBUG_ENTER ("BuildDepClosFoldFun");

    if (kind == SS_saclib) {
        module = LoadModule (entry);
        result = GetDependencyTable (module);
        module = UnLoadModule (module);
    }

    result = SSJoin ((stringset_t *)rest, result);

    DBUG_RETURN ((void *)result);
}

static stringset_t *
BuildDependencyClosure (stringset_t *deps)
{
    stringset_t *result;

    DBUG_ENTER ("BuildDependencyClosure");

    result = SSFold (&BuildDepClosFoldFun, deps, NULL);

    result = SSJoin (result, deps);

    DBUG_RETURN (result);
}

void
DoResolveDependencies (node *syntax_tree)
{
    DBUG_ENTER ("ResolveDependencies");

    MODUL_DEPENDENCIES (syntax_tree)
      = BuildDependencyClosure (MODUL_DEPENDENCIES (syntax_tree));

    DBUG_VOID_RETURN;
}
