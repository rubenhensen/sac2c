#include "dependencies.h"

#include <libgen.h>

#include "globals.h"
#include "stringset.h"
#include "tree_basic.h"
#include "modulemanager.h"
#include "filemgr.h"
#include "str_buffer.h"
#include "str.h"
#include "memory.h"
#include "namespaces.h"
#include "ctinfo.h"

#define DBUG_PREFIX "DEP"
#include "debug.h"

/*
 * functions for generating the dependency table c file
 */
static void
GenerateDependencyTableHead (FILE *file)
{
    DBUG_ENTER ();

    fprintf (file, "/* generated by sac2c %s */\n\n", global.version_id);
    fprintf (file, "#include \"sac_serialize.h\"\n\n");
    fprintf (file, "void *__%s__DEPTAB() {\n", global.modulename);
    fprintf (file, "void *result = (void *) 0;\n");

    DBUG_RETURN ();
}

static void
GenerateDependencyTableTail (FILE *file)
{
    DBUG_ENTER ();

    fprintf (file, "return(result);\n}\n");

    DBUG_RETURN ();
}

static void *
TableEntriesFoldFun (const char *val, strstype_t kind, void *rest)
{
    str_buf *result = (str_buf *)rest;

    DBUG_ENTER ();

    switch (kind) {
    case STRS_saclib:
    case STRS_extlib:
        result = SBUFprintf ((str_buf *)rest, "result = STRSadd( \"%s\", %d, result);\n",
                             val, kind);
        break;
    default:
        break;
    }

    DBUG_RETURN (result);
}

static void
GenerateDependencyTableEntries (stringset_t *deps, FILE *file)
{
    str_buf *buffer;
    char *string;

    DBUG_ENTER ();

    buffer = SBUFcreate (4096);

    buffer = (str_buf *)STRSfold (&TableEntriesFoldFun, deps, buffer);

    string = SBUF2str (buffer);

    fprintf (file, "%s", string);

    string = MEMfree (string);
    buffer = SBUFfree (buffer);

    DBUG_RETURN ();
}

static void
GenerateDependencyTable (stringset_t *deps)
{
    FILE *file;

    DBUG_ENTER ();

    file = FMGRwriteOpen ("%s/dependencytable.c", global.tmp_dirname);

    GenerateDependencyTableHead (file);

    GenerateDependencyTableEntries (deps, file);

    GenerateDependencyTableTail (file);

    fclose (file);

    DBUG_RETURN ();
}

/*
 * functions for printing dependency information
 */
static void *
BuildDepClosFoldFun (const char *entry, strstype_t kind, void *rest)
{
    stringset_t *result = NULL;
    module_t *module;

    DBUG_ENTER ();

    DBUG_PRINT ("Adding dependencies for %s...", entry);

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

    DBUG_ENTER ();

    result = (stringset_t *)STRSfold (&BuildDepClosFoldFun, deps, NULL);

    if (result != NULL) {
        /**
         * we have found some second level dependencies,
         * so next we build the closure over them.
         */
        result = BuildDependencyClosure (result);
    }

    result = STRSjoin (result, deps);

    DBUG_RETURN (result);
}

static void *
PrintLibDepFoldFun (const char *entry, strstype_t kind, void *modname)
{
    DBUG_ENTER ();

    /*
     * if the dependency is a sac library, we print a make
     * rule for the given library
     */
    if (kind == STRS_saclib) {
        char *libname;
        char *libfile;
        /*
            libname = (char *)MEMmalloc( sizeof( char) * ( STRlen( entry) + 5));
            sprintf( libname, "%s.sac", entry);
        */
        libname = STRcat (entry, ".sac");

        libfile = STRcpy (FMGRfindFile (PK_imp_path, libname));
        libname = MEMfree (libname);

        if (libfile != NULL) {
            char *libdir = dirname (libfile);

            printf ("alldeps_%s: \\\n  %s\n\n", (char *)modname, entry);

            printf (".PHONY: \\\n  %s\n\n", entry);

            /**
             *
             * The space before the colon is essential here!
             * This allows us to easily distinguish this case from the
             * alldeps rules, in particular, the empty rule in
             * doPrintLibDependencies
             */
            printf ("%s :\n\t( cd %s; $(MAKE) lib%sTree%s" SHARED_LIB_EXT ")\n\n", entry,
                    libdir, entry, global.config.lib_variant);
        }

        libfile = MEMfree (libfile);
    }

    DBUG_RETURN (modname);
}

static void
doPrintLibDependencies (node *tree)
{
    DBUG_ENTER ();

#ifndef DBUG_OFF
    printf ("#\n# extended dependencies for file %s\n#\n\n", global.filename);
#endif

    /**
     * First, we give an empty rule. This is required iff there
     * are no dependencies!
     * The non-space before the colon is essential here!
     * Cf. PrintLibDepFoldFun!
     */
    printf ("alldeps_%s:\n", NSgetName (MODULE_NAMESPACE (tree)));

    STRSfold (&PrintLibDepFoldFun, global.dependencies,
              (void *)NSgetName (MODULE_NAMESPACE (tree)));

    DBUG_RETURN ();
}

static void
PrintSACLib (const char *name)
{
    char *filename;
    char *result;

    DBUG_ENTER ();

    /*
     * first try to find the shared library file
     */
    /*  filename = (char *)MEMmalloc( sizeof( char) *
                            ( STRlen( name) + 11 +
                              STRlen( global.config.lib_variant)));
      sprintf( filename, "lib%sTree%s" SHARED_LIB_EXT, name, global.config.lib_variant);
    */
    filename
      = STRcatn (5, "lib", name, "Tree", global.config.lib_variant, SHARED_LIB_EXT);

    result = STRcpy (FMGRfindFile (PK_lib_path, filename));

    filename = MEMfree (filename);

    if (result == NULL) {
        /*
         * now try to find the .sac file
         */
        /*
            filename = (char *) MEMmalloc( sizeof( char) * ( STRlen( name) + 5));
            sprintf( filename, "%s.sac", name);
        */
        filename = STRcat (name, ".sac");

        result = STRcpy (FMGRdirname (FMGRfindFile (PK_imp_path, filename)));

        filename = MEMfree (filename);

        if (result != NULL) {
#if 0
      filename = (char *)MEMmalloc( sizeof( char) *
                            ( STRlen( result) +
                              16 + /* $(LIBTARGETDIR)/ */
  //                            3 + /* lib */
  //                            4 + /* Tree */
 //                             STRlen( SHARED_LIB_EXT) + 2 + /* .so\0 */
 //                             1024 +
 //                             STRlen( global.config.lib_variant)));
        sprintf( filename,
               "%s$(LIBTARGETDIR)/lib%sTree%s" SHARED_LIB_EXT, 
               result, 
               name,
               global.config.lib_variant);
#endif
            filename = STRcatn (6, result, "$(LIBTARGETDIR)/lib", name, "Tree",
                                global.config.lib_variant, SHARED_LIB_EXT);

            result = MEMfree (result);
            result = filename;
            filename = NULL;
        }
    }

    if (result == NULL) {
        /*
         * otherwise use the pure filename
         */

        /*    result = (char *)MEMmalloc( sizeof( char) *
                                ( STRlen( name) + 11 +
                                  STRlen( global.config.lib_variant)));
            sprintf( result, "lib%sTree%s" SHARED_LIB_EXT, name,
           global.config.lib_variant);
            */
        result
          = STRcatn (5, "lib", name, "Tree", global.config.lib_variant, SHARED_LIB_EXT);
    }

    printf (" \\\n  %s", result);

    result = MEMfree (result);

    DBUG_RETURN ();
}
#if 0
static void PrintSACLibInclude( const char *name)
{
  char *filename;
  const char *dir;
  char *cwd="./";

  DBUG_ENTER ();
/*
  filename = MEMmalloc( sizeof( char) * ( STRlen( name) + 5));
  sprintf( filename, "%s.sac", name);
*/
  filename = STRcat( name, ".sac");

  dir = FMGRdirname( FMGRfindFile( PK_imp_path, filename));

  filename = MEMfree( filename);

  if (dir == NULL) {
    dir = cwd;
  }

  printf( "FILE_%s%s%s_d := yes\n",
          FMGRfile2id( dir),
          name,
          global.config.lib_variant);
  printf( "ifneq ($(FILE_%s%s%s_d),yes)\n",
          FMGRfile2id( dir),
          name,
          global.config.lib_variant);
  printf( "-include %s.%s%s.d\n", dir, name, global.config.lib_variant);
  printf( "endif\n");

  DBUG_RETURN ();
}
#endif

static void
PrintObjFileInclude (const char *name)
{
    char *oName = NULL;
    char *dirName = NULL;
    DBUG_ENTER ();

    oName = STRncpy (name, STRlen (name) - 2);
    dirName = STRcpy (FMGRdirname (global.sacfilename));

    printf ("ifneq ($(FILE_%s%s_d),yes)\n", FMGRfile2id (oName),
            global.config.lib_variant);
    printf ("FILE_%s%s_d := yes\n", FMGRfile2id (oName), global.config.lib_variant);
    printf ("-include  %s%s.%s%s.d\n", dirName, FMGRdirname (oName), FMGRbasename (oName),
            global.config.lib_variant);
    printf ("endif\n");

    oName = MEMfree (oName);
    dirName = MEMfree (dirName);

    DBUG_RETURN ();
}

static void
PrintObjFile (const char *name)
{
    char *oName = NULL;
    DBUG_ENTER ();

    oName = STRncpy (name, STRlen (name) - 2);

    printf (" \\\n  %s%s%s.o", FMGRdirname (global.sacfilename), oName,
            global.config.lib_variant);

    oName = MEMfree (oName);

    DBUG_RETURN ();
}

static void *
PrintDepInclude (const char *entry, strstype_t kind, void *rest)
{
    DBUG_ENTER ();

    switch (kind) {
    case STRS_saclib:
#if 0
      PrintSACLibInclude( entry);
#endif
        break;
    case STRS_objfile:
        PrintObjFileInclude (entry);
        break;
    default:
        break;
    }

    DBUG_RETURN ((void *)NULL);
}

static void *
PrintDepFoldFun (const char *entry, strstype_t kind, void *rest)
{
    DBUG_ENTER ();

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

    DBUG_RETURN ((void *)NULL);
}

static void
PrintTargetName (node *tree)
{
    DBUG_ENTER ();

    switch (MODULE_FILETYPE (tree)) {
    case FT_prog:
        printf ("%s:", global.outfilename);
        break;
    case FT_modimp:
    case FT_classimp:
        printf ("%slib%sTree%s" SHARED_LIB_EXT
                " %slib%sMod%s.a %slib%sMod%s" SHARED_LIB_EXT ":",
                (global.targetdir), NSgetName (MODULE_NAMESPACE (tree)),
                global.config.lib_variant, (global.targetdir),
                NSgetName (MODULE_NAMESPACE (tree)), global.config.lib_variant,
                (global.targetdir), NSgetName (MODULE_NAMESPACE (tree)),
                global.config.lib_variant);
        break;
    default:
        DBUG_UNREACHABLE ("unknown file type found!");
        break;
    }

    DBUG_RETURN ();
}

node *
DEPdoPrintDependencies (node *syntax_tree)
{
    DBUG_ENTER ();

    printf ("#\n# dependencies for file %s\n#\n\n", global.filename);

    printf ("ifeq ($(RECURSIVE_DEPEND),yes)\n");
    STRSfold (&PrintDepInclude, global.dependencies, NULL);
    printf ("endif\n\n");

    /*
     * first, print how the output will be named
     */
    PrintTargetName (syntax_tree);

    /*
     * now add the dependencies
     */
    STRSfold (&PrintDepFoldFun, global.dependencies, NULL);

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

    CTIterminateCompilation (syntax_tree);

    DBUG_RETURN (syntax_tree);
}

/*
 * We use a rather weird signature here to fit the framework of the
 * compiler phase mechanism.
 */
node *
DEPdoHandleDependencies (node *syntax_tree)
{
    DBUG_ENTER ();

    if ((global.filetype == FT_modimp) || (global.filetype == FT_classimp)) {
        /*
         * finally generate the dependency table.
         * we do this here as new dependencies may be introduced
         * during the compilation steps up to here
         */
        GenerateDependencyTable (global.dependencies);
    } else {
        /*
         * for programs, we build the closure of all dependencies.
         * again, we cannot do this earlier, as new dependencies
         * might have been introduced until here.
         */
        global.dependencies = BuildDependencyClosure (global.dependencies);
    }

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
