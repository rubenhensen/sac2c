/*
 *
 * $Log$
 * Revision 1.5  2005/06/02 17:02:34  sbs
 * added an empty alldeps rule for -Mlib in case there are no
 * dependencies at all.
 *
 * Revision 1.4  2005/06/02 15:02:37  sah
 * added -Mlib option and corresponding implementation
 *
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
#include <libgen.h>
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

static void *
PrintLibDepFoldFun (const char *entry, strstype_t kind, void *modname)
{
    DBUG_ENTER ("PrintLibDepFoldFun");

    /*
     * if the dependency is a sac library, we print a make
     * rule for the given library
     */
    if (kind == STRS_saclib) {
        char *libname;
        char *libfile;

        libname = ILIBmalloc (sizeof (char) * (strlen (entry) + 5));
        sprintf (libname, "%s.sac", entry);

        libfile = ILIBstringCopy (FMGRfindFile (PK_imp_path, libname));
        libname = ILIBfree (libname);

        if (libfile != NULL) {
            char *libdir = dirname (libfile);

            printf ("alldeps_%s: \\\n  %s\n\n", (char *)modname, entry);

            printf (".PHONY: \\\n  %s\n\n", entry);

            /**
             *
             * The space before the colon is essential here!
             * This allows us to easily distinguish this case from the
             * alldeps rules, in particular, the empty rule in doPrintLibDependencies
             */
            printf ("%s :\n\t( cd %s; $(MAKE) lib%s.so)\n\n", entry, libdir, entry);
        }

        libfile = ILIBfree (libfile);
    }

    DBUG_RETURN (modname);
}

static void
doPrintLibDependencies (node *tree)
{
    DBUG_ENTER ("doPrintLibDependencies");

#ifndef DBUG_OFF
    printf ("#\n# extended dependencies for file %s\n#\n\n", global.filename);
#endif

    /**
     * First, we give an empty rule. This is required iff there
     * are no dependencies!
     * The non-space before the colon is essential here!
     * Cf. PrintLibDepFoldFun!
     */
    printf ("alldeps_%s:\n", MODULE_NAME (tree));

    STRSfold (&PrintLibDepFoldFun, MODULE_DEPENDENCIES (tree), MODULE_NAME (tree));

    DBUG_VOID_RETURN;
}

static void
PrintSACLib (const char *name)
{
    char *filename;
    char *result;

    DBUG_ENTER ("PrintSACLib");

    /*
     * first try to find the .so file
     */

    filename = ILIBmalloc (sizeof (char) * (strlen (name) + 7));
    sprintf (filename, "lib%s.so", name);

    result = ILIBstringCopy (FMGRfindFile (PK_lib_path, filename));

    filename = ILIBfree (filename);

    if (result == NULL) {
        /*
         * otherwise use the pure filename
         */

        result = ILIBmalloc (sizeof (char) * (strlen (name) + 7));
        sprintf (result, "lib%s.so", name);
    }

    printf (" \\\n  %s", result);

    result = ILIBfree (result);

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

#ifndef DBUG_OFF
    printf ("#\n# dependencies for file %s\n#\n\n", global.filename);
#endif

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

    /*
     * if we are in special Mlib (aka sbs) mode, we have to print
     * some more dependencies, handeled by doPrintLibDependencies
     */
    if (global.makelibdeps) {
        doPrintLibDependencies (syntax_tree);
    }

    exit (0);

    DBUG_RETURN (syntax_tree);
}
