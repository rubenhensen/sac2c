/*
 *
 * $Id$
 *
 */

#include "libbuilder.h"

#define DBUG_PREFIX "LIBB"
#include "debug.h"

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
 * @fn char *AddVariantToDotOFile( const char *file)
 *
 * @brief If the 'file' is a .o file add the lib_variant to its name
 *        If not a .o file then just copy the name of the file
 *
 *****************************************************************************/
static char *
AddVariantToDotOFile (const char *file)
{
    char *res;
    DBUG_ENTER ();

    if (STRsuffix (".o", file)) {
        char *name = STRsubStr (file, 0, STRlen (file) - 2);
        res = STRcatn (3, name, global.config.lib_variant, ".o");
        name = MEMfree (name);
    } else {
        res = STRcpy (file);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn char *BuildDepLibsStringMod( const char *lib,
 *                                  strstype_t kind,
 *                                  void *rest)
 *
 * @brief fold function to generate a string of modules to link with.
 *
 *****************************************************************************/

static void *
BuildDepLibsStringMod (const char *lib, strstype_t kind, void *rest)
{
    char *result = NULL;
    char *select;

    DBUG_ENTER ();

    switch (kind) {
    case STRS_objfile:
        select = AddVariantToDotOFile (lib);
        break;
    default:
        select = NULL;
        break;
    }

    if ((rest != NULL) && (select != NULL)) {
        result = STRcatn (3, (char *)rest, " ", select);
        select = MEMfree (select);
        rest = MEMfree (rest);
    } else if (select != NULL) {
        result = select;
    } else { /* rest != NULL */
        result = (char *)rest;
    }

    DBUG_RETURN (result);
}

node *
LIBBcreateLibrary (node *syntax_tree)
{
    char *deplibs;
    char *libraryName;
    char *ldCmd;
    stringset_t *deps = global.dependencies;

    DBUG_ENTER ();

    if (global.gen_cccall) {
        /*
         * enable system call tracking
         */
        SYSstartTracking ();
    }

    deplibs = STRSfold (&BuildDepLibsStringMod, deps, STRcpy (""));

    CTInote ("Creating static SAC library `lib%sMod%s.a'", global.modulename,
             global.config.lib_variant);

    SYScall ("%s %slib%sMod%s.a %s/fun*_nonpic.o %s/globals_nonpic.o %s",
             global.config.ar_create, global.targetdir, global.modulename,
             global.config.lib_variant, global.tmp_dirname, global.tmp_dirname, deplibs);
    if (global.config.ranlib[0] != '\0') {
        SYScall ("%s %slib%sMod%s.a", global.config.ranlib, global.targetdir,
                 global.modulename, global.config.lib_variant);
    }

    if (STReq (global.config.ld_dynamic, "")) {
        CTInote ("Shared libraries are not supported by the target.");
    } else {
        CTInote ("Creating shared SAC library `lib%sMod%s.so'", global.modulename,
                 global.config.lib_variant);

        libraryName = STRcatn (5, "lib", global.modulename, "Mod",
                               global.config.lib_variant, ".so");
        ldCmd = STRsubstToken (global.config.ld_dynamic, "%libname%", libraryName);

        DBUG_PRINT ("linker command: %s", ldCmd);

        SYScall ("%s -o %s%s %s/fun*_pic.o %s/globals_pic.o %s", ldCmd, global.targetdir,
                 libraryName, global.tmp_dirname, global.tmp_dirname, deplibs);

        libraryName = MEMfree (libraryName);
        ldCmd = MEMfree (ldCmd);
    }

    deplibs = MEMfree (deplibs);

    CTInote ("Creating shared SAC library `lib%sTree%s.so'", global.modulename,
             global.config.lib_variant);

    libraryName
      = STRcatn (5, "lib", global.modulename, "Tree", global.config.lib_variant, ".so");
    ldCmd = STRsubstToken (global.config.tree_ld, "%libname%", libraryName);

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

    DBUG_ENTER ();

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

#undef DBUG_PREFIX
