/*
 *
 * $Id$
 *
 */

#include "libbuilder.h"
#include "dbug.h"
#include "ctinfo.h"
#include "system.h"
#include "str.h"
#include "memory.h"
#include "stringset.h"
#include "resource.h"
#include "filemgr.h"
#include "globals.h"

/** <!--********************************************************************-->
 *
 * @fn char *BuildDepLibsStringMod( const char *lib,
 *                                  strstype_t kind,
 *                                  void *rest)
 *
 * @brief fold funciton to generate a string of modules to link with.
 *
 *****************************************************************************/

static void *
BuildDepLibsStringMod (const char *lib, strstype_t kind, void *rest)
{
    char *result = NULL;
    char *llib = NULL;

    DBUG_ENTER ("BuildDepLibsStringMod");

    llib = STRcpy (lib);

    switch (kind) {
    case STRS_objfile:
        if (global.backend == BE_mutc) {
            char *tmp;
            DBUG_ASSERT (STRsuffix (".o", llib),
                         "found linkwith that does not end in .o");

            /* remove .o */
            tmp = llib;
            llib = STRsubStr (llib, 0, -2);
            MEMfree (tmp);

            /* add target to file name */
            tmp = llib;
            llib = STRcat (llib, global.target_name);
            MEMfree (tmp);

            /* add .o back */
            tmp = llib;
            llib = STRcat (llib, ".o");
            MEMfree (tmp);
        }
        result = STRcpy (llib);
        break;
    default:
        result = STRnull ();
        break;
    }

    if (rest != NULL) {
        char *temp
          = MEMmalloc (sizeof (char) * (STRlen ((char *)rest) + STRlen (result) + 2));

        sprintf (temp, "%s %s", (char *)rest, result);
        result = MEMfree (result);
        rest = MEMfree (rest);
        result = temp;
    }

    llib = MEMfree (llib);

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn node *CreateStaticLibrary(node *deplibs)
 *
 * @brief Create a static library for a module.
 *     or Create a cross compiled static library for a module for sl.
 *
 *****************************************************************************/
static void
CreateStaticLibrary (char *deplibs)
{
    DBUG_ENTER ("CreateStaticLibrary");

    CTInote ("Creating static SAC library `lib%sMod.a'", global.modulename);

    if (global.backend == BE_mutc) {
        /* Support cross compiling */
        SYScall ("%s %slib%sMod%s.a %s/fun*_nonpic.o %s/globals_nonpic.o %s",
                 global.config.ar_create, global.targetdir, global.modulename,
                 global.target_name, global.tmp_dirname, global.tmp_dirname, deplibs);
        if (global.config.ranlib[0] != '\0') {
            SYScall ("%s %slib%sMod%s.a", global.config.ranlib, global.targetdir,
                     global.modulename, global.target_name);
        }
    } else {
        SYScall ("%s %slib%sMod.a %s/fun*_nonpic.o %s/globals_nonpic.o %s",
                 global.config.ar_create, global.targetdir, global.modulename,
                 global.tmp_dirname, global.tmp_dirname, deplibs);
        if (global.config.ranlib[0] != '\0') {
            SYScall ("%s %slib%sMod.a", global.config.ranlib, global.targetdir,
                     global.modulename);
        }
    }

    DBUG_VOID_RETURN;
}

node *
LIBBcreateLibrary (node *syntax_tree)
{
    char *deplibs;
    char *libraryName;
    char *ldCmd;
    stringset_t *deps = global.dependencies;

    DBUG_ENTER ("LIBBcreateLibrary");

    if (global.gen_cccall) {
        /*
         * enable system call tracking
         */
        SYSstartTracking ();
    }

    deplibs = STRSfold (&BuildDepLibsStringMod, deps, STRcpy (""));

    CreateStaticLibrary (deplibs);

    CTInote ("Creating shared SAC library `lib%sMod.so'", global.modulename);

    libraryName = STRcatn (3, "lib", global.modulename, "Mod.so");
    ldCmd = STRsubstToken (global.config.ld_dynamic, "%libname%", libraryName);

    DBUG_PRINT ("LIBB", ("linker command: %s", ldCmd));

    SYScall ("%s -o %s%s %s/fun*_pic.o %s/globals_pic.o %s", ldCmd, global.targetdir,
             libraryName, global.tmp_dirname, global.tmp_dirname, deplibs);

    libraryName = MEMfree (libraryName);
    ldCmd = MEMfree (ldCmd);
    deplibs = MEMfree (deplibs);

    CTInote ("Creating shared SAC library `lib%sTree.so'", global.modulename);

    libraryName = STRcatn (3, "lib", global.modulename, "Tree.so");
    ldCmd = STRsubstToken (global.config.ld_dynamic, "%libname%", libraryName);

    SYScall ("%s -o %s%s %s/serialize.o %s/symboltable.o"
             " %s/dependencytable.o %s/namespacemap.o %s/filenames.o",
             ldCmd, global.targetdir, libraryName, global.tmp_dirname, global.tmp_dirname,
             global.tmp_dirname, global.tmp_dirname, global.tmp_dirname);

    libraryName = MEMfree (libraryName);
    ldCmd = MEMfree (ldCmd);

    if (global.gen_cccall) {
        /*
         * stop tracking and close file
         */
        SYSstopTracking ();
    }

    DBUG_RETURN (syntax_tree);
}

node *
LIBBcreateWrapperLibrary (node *syntax_tree)
{
    char *deplibs;
    char *libraryName;
    char *ldCmd;
    stringset_t *deps = global.dependencies;

    DBUG_ENTER ("LIBBcreateWrapperLibrary");

    if (global.gen_cccall) {
        /*
         * enable system call tracking
         */
        SYSstartTracking ();
    }

    deplibs = STRSfold (&BuildDepLibsStringMod, deps, STRcpy (""));

    CTInote ("Creating static wrapper library `lib%s.a'", global.outfilename);

    SYScall ("%s %s/lib%s.a %s/fun*_nonpic.o %s/globals_nonpic.o "
             "%s/interface_nonpic.o %s/sacargcopy_nonpic.o "
             "%s/sacargfree_nonpic.o %s",
             global.config.ar_create, STRonNull (".", global.lib_dirname),
             global.outfilename, global.tmp_dirname, global.tmp_dirname,
             global.tmp_dirname, global.tmp_dirname, global.tmp_dirname, deplibs);

    if (global.config.ranlib[0] != '\0') {
        SYScall ("%s %s/lib%s.a", global.config.ranlib,
                 STRonNull (".", global.lib_dirname), global.outfilename);
    }

    CTInote ("Creating shared wrapper library `lib%s.so'", global.outfilename);

    libraryName = STRcatn (3, "lib", global.outfilename, ".so");
    ldCmd = STRsubstToken (global.config.ld_dynamic, "%libname%", libraryName);

    SYScall ("%s -o %s/%s %s/fun*_pic.o %s/globals_pic.o "
             "%s/interface_pic.o %s/sacargcopy_pic.o "
             "%s/sacargfree_pic.o %s",
             ldCmd, STRonNull (".", global.lib_dirname), libraryName, global.tmp_dirname,
             global.tmp_dirname, global.tmp_dirname, global.tmp_dirname,
             global.tmp_dirname, deplibs);

    libraryName = MEMfree (libraryName);
    ldCmd = MEMfree (ldCmd);
    deplibs = MEMfree (deplibs);

    DBUG_RETURN (syntax_tree);
}
