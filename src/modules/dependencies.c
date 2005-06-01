/*
 *
 * $Log$
 * Revision 1.3  2005/06/01 18:01:24  sah
 * finished printing of dependencies
 *
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

#include <string.h>
#include "stringset.h"
#include "tree_basic.h"
#include "modulemanager.h"
#include "filemgr.h"
#include "internal_lib.h"
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

static void
PrintSACLib (const char *name)
{
    char *filename;
    const char *result;

    DBUG_ENTER ("PrintSACLib");

    /*
     * first try to find the .so file
     */

    filename = ILIBmalloc (sizeof (char) * (strlen (name) + 7));
    sprintf (filename, "lib%s.so", name);

    result = FMGRfindFile (PK_lib_path, filename);

    filename = ILIBfree (filename);

    if (result == NULL) {
        /*
         * now try to find the .sac file
         */

        filename = ILIBmalloc (sizeof (char) * (strlen (name) + 5));
        sprintf (filename, "%s.sac", name);

        result = FMGRfindFile (PK_imp_path, filename);

        filename = ILIBfree (filename);
    }

    if (result == NULL) {
        /*
         * still have not found a file, give up
         */
        CTIerror ("Can find neither library lib%s.so, nor implementation "
                  "%s.sac.",
                  name, name);
    } else {
        printf (" \\\n  %s", result);
    }

    DBUG_VOID_RETURN;
}

static void
PrintObjFile (const char *name)
{
    DBUG_ENTER ("PrintObjFile");

    printf (" \\\n  %s", name);

    DBUG_VOID_RETURN;
}

static void *
PrintDepFoldFun (const char *entry, strstype_t kind, void *rest)
{
    DBUG_ENTER ("PrintDepFoldFun");

    switch (kind) {
    case STRS_saclib:
        PrintSACLib (entry);
        break;
    case STRS_objfile:
        PrintObjFile (entry);
        break;
    default:
        break;
    }

    DBUG_RETURN (NULL);
}

static void
PrintTargetName (node *tree)
{
    DBUG_ENTER ("PrintTargetName");

    switch (MODULE_FILETYPE (tree)) {
    case F_prog:
        printf ("%s:", global.outfilename);
        break;
    case F_modimp:
    case F_classimp:
        printf ("%slib%s.so %slib%s.a:", global.targetdir, MODULE_NAME (tree),
                global.targetdir, MODULE_NAME (tree));
        break;
    default:
        DBUG_ASSERT (0, ("unknown file type found!"));
        break;
    }

    DBUG_VOID_RETURN;
}

node *
DEPdoPrintDependencies (node *syntax_tree)
{
    DBUG_ENTER ("DEPdoPrintDependencies");

    /*
     * first, print how the output will be named
     */
    PrintTargetName (syntax_tree);

    /*
     * now add the dependencies
     */
    STRSfold (&PrintDepFoldFun, MODULE_DEPENDENCIES (syntax_tree), NULL);

    /*
     * and finally two newline to make it look nicer
     */
    printf ("\n\n");

    exit (0);

    DBUG_RETURN (syntax_tree);
}
