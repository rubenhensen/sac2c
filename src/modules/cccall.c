/*
 *
 * $Log$
 * Revision 2.13  2000/07/12 15:53:36  nmw
 * add generation of one big library file, including all neccessary object files
 *
 * Revision 2.12  2000/07/05 15:34:11  nmw
 * compilation and generation of c-library added
 *
 * Revision 2.11  2000/06/09 13:27:55  nmw
 * moving of libfile to targetdir added
 *
 * Revision 2.10  2000/06/09 09:23:27  nmw
 * moving of c library headerfile added
 *
 * Revision 2.9  2000/01/21 13:20:57  jhs
 * Adapted to new mt ...
 *
 * Revision 2.8  2000/01/17 17:23:05  cg
 * Wrapper functions for option -noPHM are now integrated into libsac.
 * So, there is no longer s special version of the heap manager library
 * to link with.
 *
 * Revision 2.7  2000/01/17 16:25:58  cg
 * Added new handling of various version of the heap manager
 * library.
 *
 * Revision 2.6  1999/07/16 09:34:12  cg
 * Added facilities for heap management diagnostics.
 *
 * Revision 2.5  1999/07/09 07:32:30  cg
 * Executables are now linked with customized versions of the
 * SAC runtime library:
 *   sequential vs multi-threaded
 *   private heap manager vs public heap manager
 *
 * Revision 2.4  1999/07/08 12:41:19  cg
 * Implementation of old command line optio -mt_all removed.
 * Prepared linking with several selected parts of the SAC runtime
 * library.
 *
 * Revision 2.3  1999/05/18 11:22:19  cg
 * Unfortunately, if tar fails, it does NOT produce an error condition.
 * So, we have to check whether extracted files actually exist after the
 * call of tar in order to handle this error condition correctly and
 * create an appropriate error message.
 *
 * Revision 2.2  1999/05/04 11:10:04  sbs
 * fixed a bug in GenLibStat:
 * HOST, PWD, and USER are not mandatory for compiling
 * modules anymore.
 *
 * Revision 2.1  1999/02/23 12:41:59  sacbase
 * new release made
 *
 * Revision 1.21  1999/02/22 13:02:16  cg
 * usage of efence: final version.
 *
 * Revision 1.20  1999/02/19 18:43:21  dkr
 * usage of efence added (first hack only :((
 *
 * Revision 1.19  1998/11/19 16:12:36  cg
 * new configuration entry: CCMTLINK
 * specifies libraries to link with for multi-threaded programs only.
 * This makes the target 'par' obsolete.
 *
 * Revision 1.18  1998/07/10 15:20:37  cg
 * corrected superfluous warning
 *
 * Revision 1.17  1998/07/07 13:42:55  cg
 *  implemented the command line option -mt-all
 *
 * Revision 1.16  1998/06/23 15:06:55  cg
 * Command line option -dcccall implemented.
 *
 * Revision 1.15  1998/03/25 10:41:43  cg
 * library format of SAC libraries slightly modified:
 * archives are now called lib<modname>.a instead of <modname>.a
 * This allows for using the -l option of C compilers in conjunction
 * with -L<tmpdir>.
 * Additionally, the dependence trees is simplified before the
 * link list is generated, resulting in a much shorter compiler
 * call string.
 *
 * Revision 1.14  1998/03/04 16:23:27  cg
 *  C compiler invocations and file handling converted to new
 * to usage of new  configuration files.
 *
 * Revision 1.13  1997/04/30 11:52:33  cg
 * full path names are used also for SAC modules in link list
 *
 * Revision 1.12  1997/04/24  10:07:37  cg
 * call to gcc converted to new environment variable SACBASE
 *
 * Revision 1.11  1997/03/19  13:46:23  cg
 * entirely re-implemented
 * now a global dependency tree is built up by import.c and readsib.c
 * So, linking should work now.
 *
 * Revision 1.10  1997/03/11  16:29:10  cg
 * new list of standard modules
 * old compiler option -deps ((updating makefile) no longer supported
 * use absolute pathnames for libstat
 *
 * Revision 1.9  1996/09/11  16:00:11  cg
 * small layout change and new standard modules added
 *
 * Revision 1.8  1996/09/11  06:21:34  cg
 * Converted to new lib-file format.
 * Added facilities for updating makefiles with dependencies
 * and creating libstat information.
 *
 * Revision 1.7  1996/01/25  15:58:19  cg
 * bug fixed when linking with external archive files
 *
 * Revision 1.6  1996/01/21  13:59:05  cg
 * Now, C object files are also looked for in $RCSROOT/src/compile/
 * where the SAC runtime library resides
 *
 * Revision 1.5  1996/01/07  16:57:30  cg
 * InvokeCC and CreateLibrary entirely rewritten
 *
 * Revision 1.4  1996/01/05  12:37:34  cg
 * Now, SAC library files are generated when compiling module/class
 * implementations.
 *
 * Revision 1.3  1996/01/02  16:01:35  cg
 * first really running revision
 *
 * Revision 1.2  1996/01/02  07:57:37  cg
 * first working revision
 *
 * Revision 1.1  1995/12/29  17:19:26  cg
 * Initial revision
 *
 *
 *
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"

#include "Error.h"
#include "dbug.h"
#include "internal_lib.h"
#include "free.h"
#include "globals.h"
#include "scnprs.h"

#include "filemgr.h"
#include "traverse.h"
#include "resource.h"
#include "gen_startup_code.h"

/*
 *
 *  functionname  : GenLibstatEntry
 *  arguments     : 1) output stream to statusfile
 *                  2) tree of dependencies
 *                  3) status for discriminating various kinds of dependencies
 *                     ST_own | ST_sac | ST_external | ST_system
 *                  4) list of dependencies which have already been printed
 *                     This is necessary because one library may reside
 *                     several times in the dependency tree.
 *  description   : writes one line of information for each dependency of
 *                  the given dependency type
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : MakeStrings, fprintf
 *  macros        :
 *
 *  remarks       : corresponds to the -libstat compiler option
 *
 */

static strings *
GenLibstatEntry (FILE *statusfile, deps *depends, statustype stat, strings *done)
{
    strings *tmp_done;
    deps *tmp;

    DBUG_ENTER ("GenLibstatEntry");

    tmp = depends;

    while (tmp != NULL) {
        if (DEPS_STATUS (tmp) == stat) {
            tmp_done = done;

            while ((tmp_done != NULL)
                   && (0 != strcmp (DEPS_NAME (tmp), STRINGS_STRING (tmp_done)))) {
                tmp_done = STRINGS_NEXT (tmp_done);
            }

            if (tmp_done == NULL) {
                done = MakeStrings (DEPS_NAME (tmp), done);

                fprintf (statusfile, "  %-15s found : %s\n",
                         strrchr (DEPS_LIBNAME (tmp), '/') + 1, DEPS_LIBNAME (tmp));
            }
        }

        tmp = DEPS_NEXT (tmp);
    }

    tmp = depends;

    while (tmp != NULL) {
        if (DEPS_SUB (tmp) != NULL) {
            done = GenLibstatEntry (statusfile, DEPS_SUB (tmp), stat, done);
        }

        tmp = DEPS_NEXT (tmp);
    }

    DBUG_RETURN (done);
}

/*
 *
 *  functionname  : GenLibStat
 *  arguments     : ---
 *  description   : generates a status information file. This is stored
 *                  within a SAC library file ('.lib'). This status information
 *                  may be retrieved from the library later by using the
 *                  -libstat compiler option
 *  global vars   : dependencies, commandline
 *  internal funs : GenLibstatEntry
 *  external funs : WriteOpen, time, ctime, getenv, fprintf, fclose
 *  macros        :
 *
 *  remarks       : corresponds to the -libstat compiler option
 *
 *  remarks       : uses the following environment variables:
 *                  PWD | HOST | USER
 *
 */

static void
GenLibStat ()
{
    FILE *statusfile;
    deps *tmp;
    long int current;
    char *env_entry;
    char unknown[4] = "???";

    DBUG_ENTER ("GenLibStat");

    statusfile = WriteOpen ("%s/%s.stt", tmp_dirname, modulename);

    current = time (NULL);

    fprintf (statusfile, "\n***  Status Report - %s.lib  ***\n\n", modulename);
    fprintf (statusfile, "Call : %s\n", commandline);

    env_entry = getenv ("PWD");
    if (env_entry == NULL)
        env_entry = unknown;
    fprintf (statusfile, "From : %s\n", env_entry);

    env_entry = getenv ("HOST");
    if (env_entry == NULL)
        env_entry = unknown;
    fprintf (statusfile, "On   : %s\n", env_entry);

    env_entry = getenv ("USER");
    if (env_entry == NULL)
        env_entry = unknown;
    fprintf (statusfile, "By   : %s\n", env_entry);

    fprintf (statusfile, "Date : %s", ctime (&current));

    fprintf (statusfile, "\nDependencies from imported modules and classes :\n");

    tmp = dependencies;

    while (tmp != NULL) {
        fprintf (statusfile, "  %-15s found : %s\n",
                 strrchr (DEPS_DECNAME (tmp), '/') + 1, DEPS_DECNAME (tmp));

        tmp = DEPS_NEXT (tmp);
    }

    fprintf (statusfile, "\nRequired SAC libraries :\n");

    GenLibstatEntry (statusfile, dependencies, ST_sac, NULL);

    fprintf (statusfile, "\nRequired external libraries :\n");

    GenLibstatEntry (statusfile, dependencies, ST_external, NULL);

    fprintf (statusfile, "\nRequired system libraries :\n");

    GenLibstatEntry (statusfile, dependencies, ST_system, NULL);

    fprintf (statusfile, "\n***  Status Report - %s.lib  ***\n\n", modulename);

    fclose (statusfile);

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : PrintLibStat
 *  arguments     : ---
 *  description   : extracts status file from SAC library and prints it
 *                  to stdout
 *  global vars   : sacfilename, tmp_dirname
 *  internal funs : ---
 *  external funs : SystemCall, SystemCall2, StringCopy, strlen, FindFile,
 *                  strrchr, AbsolutePathname
 *  macros        :
 *
 *  remarks       : corresponds to the -libstat compiler option
 *
 */

void
PrintLibStat ()
{
    char *pathname, *abspathname, *modname, *nopath, *stt_filename;
    int success;

    DBUG_ENTER ("PrintLibStat");

    pathname = FindFile (MODIMP_PATH, sacfilename);

    if (pathname == NULL) {
        SYSABORT (("Unable to find library file \"%s\"", sacfilename));
    }

    abspathname = AbsolutePathname (pathname);

    nopath = strrchr (sacfilename, '/');

    modname = StringCopy (nopath == NULL ? sacfilename : (nopath + 1));

    modname[strlen (modname) - 4] = 0;

    stt_filename = (char *)Malloc ((strlen (modname) + 6) * sizeof (char));
    strcpy (stt_filename, modname);
    strcat (stt_filename, ".stt");

    success
      = SystemCall2 ("%s %s; %s %s %s %s", config.chdir, tmp_dirname, config.tar_extract,
                     abspathname, stt_filename, config.dump_output);

    if ((success != 0) || !CheckExistFile (tmp_dirname, stt_filename)) {
        SYSABORT (("Corrupted library file format: \"%s\"", abspathname));
    }

    FREE (stt_filename);

    SystemCall ("%s %s/%s.stt", config.cat, tmp_dirname, modname);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void LinearizeDependencies(deps *depends)
 *
 * description:
 *
 *   This function realizes a pre-order traversal of the dependence tree
 *   and transforms it into a linear list.
 *
 *
 ******************************************************************************/

static void
LinearizeDependencies (deps *depends)
{
    deps *keep_next;
    deps *tmp;

    DBUG_ENTER ("LinearizeDependencies");

    while (depends != NULL) {
        if (DEPS_SUB (depends) != NULL) {
            keep_next = DEPS_NEXT (depends);
            DEPS_NEXT (depends) = DEPS_SUB (depends);
            DEPS_SUB (depends) = NULL;
            tmp = DEPS_NEXT (depends);
            while (DEPS_NEXT (tmp) != NULL) {
                tmp = DEPS_NEXT (tmp);
            }
            DEPS_NEXT (tmp) = keep_next;
        }

        depends = DEPS_NEXT (depends);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   int ProcessDependencies(deps *depends)
 *
 * description:
 *
 *   This function traverses the entire linearized dependence tree. For each
 *   dependent module it is checked whether this has further occurrences or
 *   not. In the former case it is marked using a special status. In the latter
 *   case the length-counter for the linklist string buffer is incremented
 *   depending on the type of library.
 *
 *   The total amount of memory needed to store the string linklist is returned.
 *
 ******************************************************************************/

static int
ProcessDependencies (deps *depends)
{
    int cnt = 0;
    deps *tmp;

    DBUG_ENTER ("ProcessDependencies");

    while (depends != NULL) {
        tmp = DEPS_NEXT (depends);

        while ((tmp != NULL) && (0 != strcmp (DEPS_NAME (depends), DEPS_NAME (tmp)))) {
            tmp = DEPS_NEXT (tmp);
        }

        if (tmp != NULL) {
            DEPS_STATUS (depends) = ST_regular;
        } else {
            if (DEPS_STATUS (depends) == ST_sac) {
                cnt += strlen (DEPS_NAME (depends)) + 3;
            } else if (DEPS_STATUS (depends) != ST_own) {
                cnt += strlen (DEPS_LIBNAME (depends)) + 1;
            }
        }

        depends = DEPS_NEXT (depends);
    }

    DBUG_RETURN (cnt);
}

/*
 *
 *  functionname  : FillLinklist
 *  arguments     : 1) dependency tree
 *                  2) pre-allocated linklist string
 *  description   : adds one entry to the link list string for each
 *                  dependent library
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : strcat
 *  macros        :
 *
 *  remarks       : The archive files of all dependent SAC libraries are stored
 *                  in the temporary directory. They are all named libXXX.a,
 *                  so we can exploit the -L and -l cc options to link with them.
 *                  For external and system libraries the respective pathname
 *                  collected by readsib.c is used directly.
 *
 */

static void
FillLinklist (deps *depends, char *linklist)
{
    DBUG_ENTER ("FillLinklist");

    while (depends != NULL) {
        switch (DEPS_STATUS (depends)) {
        case ST_regular:
            break;
        case ST_sac:
            strcat (linklist, "-l");
            strcat (linklist, DEPS_NAME (depends));
            strcat (linklist, " ");
            break;
        case ST_external:
        case ST_system:
            strcat (linklist, DEPS_LIBNAME (depends));
            strcat (linklist, " ");
            break;
        default:
            break;
        }

        depends = DEPS_NEXT (depends);
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : GenLinklist
 *  arguments     : 1) dependency tree
 *  description   : allocates memory for the link list and fills it with
 *                  the dependent libraries extracted from the processed dependency tree
 *  global vars   : ---
 *  internal funs : CountLinklistLength, FillLinklist
 *  external funs : Malloc, strcpy
 *  macros        :
 *
 *  remarks       :
 *
 */

static char *
GenLinklist (deps *depends)
{
    char *linklist;
    int size;

    DBUG_ENTER ("GenLinklist");

    LinearizeDependencies (depends);

    size = ProcessDependencies (depends) + 5;

    linklist = (char *)Malloc (size);

    strcpy (linklist, " ");

    FillLinklist (depends, linklist);

    DBUG_RETURN (linklist);
}

/*
 *
 *  functionname  : CreateLibrary
 *  arguments     : ---
 *  description   : creates a SAC library file.
 *                  All object files from the temporary directory are put
 *                  into an archive. This archive is tared together with
 *                  the status information file and the SIB file.
 *                  if specified to generate c interface/library the headerfile
 *                  is moved to the output dir
 *  global vars   : useranlib, tmp_dirname, modulename,
 *  internal funs : GenLibStat
 *  external funs : SystemCall
 *  macros        :
 *
 *  remarks       :
 *
 */

void
CreateLibrary ()
{
    DBUG_ENTER ("CreateLibrary");

    /* creating lib-file using ar, for internal usage by sac library
     * used as is by the c interface
     */

    SystemCall ("%s %s/lib%s.a %s/*.o", config.ar_create, tmp_dirname, modulename,
                tmp_dirname);

    if (config.ranlib[0] != '\0') {
        SystemCall ("%s %s/%s.a", config.ranlib, tmp_dirname, modulename);
    }

    /* generating sac library file*/
    if (generatelibrary & GENERATELIBRARY_SAC) {
        NOTE (("Creating SAC library \"%s%s.lib\"", targetdir, modulename));

        GenLibStat ();

        SystemCall ("%s %s; %s %s.lib lib%s.a %s.sib %s.stt", config.chdir, tmp_dirname,
                    config.tar_create, modulename, modulename, modulename, modulename);

        SystemCall ("%s %s/%s.lib %s", config.move, tmp_dirname, modulename, targetdir);
    }

    /*
     * generating c library files
     * here: only moving generated files to target dir
     *
     */
    if (generatelibrary & GENERATELIBRARY_C) {
        NOTE (("Creating c library \"lib%s.a\" and interface \"%s.h\"", modulename,
               modulename));

        /*add object files to preproduced archive of included object files */
        SystemCall ("%s %s/SAC_full.archive %s/*.o", config.ar_create, tmp_dirname,
                    tmp_dirname);

        SystemCall ("%s %s/%s.h %s", config.move, tmp_dirname, modulename, targetdir);

        SystemCall ("%s %s/SAC_full.archive %s/lib%s.a", config.move, tmp_dirname,
                    targetdir, modulename);
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : InvokeCC
 *  arguments     : ---
 *  description   : starts the gcc for final compilation
 *  global vars   : ccflagsstr, outfilename, cfilename, tmp_dirname,
 *                  dependencies
 *  internal funs : ---
 *  external funs : SystemCall
 *  macros        :
 *
 *
 */

void
InvokeCC ()
{
    int i;
    char opt_buffer[36];
    char *lib_efence;

    DBUG_ENTER ("InvokeCC");

    switch (cc_optimize) {
    case 0:
        strcpy (opt_buffer, config.opt_O0);
        break;
    case 1:
        strcpy (opt_buffer, config.opt_O1);
        break;
    case 2:
        strcpy (opt_buffer, config.opt_O2);
        break;
    case 3:
        strcpy (opt_buffer, config.opt_O3);
        break;
    default:
        strcpy (opt_buffer, " ");
    }

    if (cc_debug) {
        strcat (opt_buffer, " ");
        strcat (opt_buffer, config.opt_g);
    }

    if (filetype == F_prog) {
        char *linklist = GenLinklist (dependencies);
        FILE *shellscript;

        if (use_efence) {
            /*
             * caution: -lefence should be last -l<...> parameter!
             */
            lib_efence = FindFile (SYSTEMLIB_PATH, "libefence.a");
            if (lib_efence == NULL) {
                SYSWARN (("Unable to find libefence.a in SYSTEMLIB_PATH"));
                use_efence = 0;
            }
        }

        if ((gen_mt_code == GEN_MT_OLD) || (gen_mt_code == GEN_MT_NEW)) {
            if (gen_cccall) {
                shellscript = WriteOpen (".sac2c");
                fprintf (shellscript, "#!/bin/sh -v\n\n");
                fprintf (shellscript, "%s %s %s -L%s %s -o %s %s %s %s -lsac_mt %s %s",
                         config.cc, config.ccflags, config.ccdir, tmp_dirname, opt_buffer,
                         outfilename, cfilename, linklist,
                         (optimize & OPT_PHM
                            ? (runtimecheck & RUNTIMECHECK_HEAP ? "-lsac_heapmgr_mt_diag"
                                                                : "-lsac_heapmgr_mt")
                            : ""),
                         config.ccmtlink, (use_efence ? lib_efence : ""));
                fclose (shellscript);
                SystemCall ("chmod a+x .sac2c");
            }

            SystemCall ("%s %s %s -L%s %s -o %s %s %s %s -lsac_mt %s %s", config.cc,
                        config.ccflags, config.ccdir, tmp_dirname, opt_buffer,
                        outfilename, cfilename, linklist,
                        (optimize & OPT_PHM
                           ? (runtimecheck & RUNTIMECHECK_HEAP ? "-lsac_heapmgr_mt_diag"
                                                               : "-lsac_heapmgr_mt")
                           : ""),
                        config.ccmtlink, (use_efence ? lib_efence : ""));
        } else {
            if (gen_cccall) {
                shellscript = WriteOpen (".sac2c");
                fprintf (shellscript, "#!/bin/sh -v\n\n");
                fprintf (shellscript, "%s %s %s -L%s %s -o %s %s %s %s -lsac %s %s",
                         config.cc, config.ccflags, config.ccdir, tmp_dirname, opt_buffer,
                         outfilename, cfilename, linklist,
                         (optimize & OPT_PHM
                            ? (runtimecheck & RUNTIMECHECK_HEAP ? "-lsac_heapmgr_diag"
                                                                : "-lsac_heapmgr")
                            : ""),
                         config.cclink, (use_efence ? lib_efence : ""));
                fclose (shellscript);
                SystemCall ("chmod a+x .sac2c");
            }

            SystemCall ("%s %s %s -L%s %s -o %s %s %s %s -lsac %s %s", config.cc,
                        config.ccflags, config.ccdir, tmp_dirname, opt_buffer,
                        outfilename, cfilename, linklist,
                        (optimize & OPT_PHM
                           ? (runtimecheck & RUNTIMECHECK_HEAP ? "-lsac_heapmgr_diag"
                                                               : "-lsac_heapmgr")
                           : ""),
                        config.cclink, (use_efence ? lib_efence : ""));
        }

    } else {
        /* compiling for a library */

        if (generatelibrary & GENERATELIBRARY_C) {
            /* build up library file of all object files included in the several
             * xxx.lib files.
             * these files are extracted and archived again into SAC_full.archive
             * this shell script has to be done before compiling to object files
             * because it removes all extracted objectfiles from the archives
             * in the tempdir!
             */
            NOTE (("collecting used sac-libraries...\n"))
            SystemCall ("%s %s ;"
                        "for archive in *.a ;"
                        "  do ar -x $archive ;"
                        "  for file in *.o ;"
                        "    do %s $file \"$archive.$file\" ;"
                        "  done ;"
                        "  %s SAC_full.archive *.o ;"
                        "  %s *.o ;"
                        "done",
                        config.chdir, tmp_dirname, config.move, config.ar_create,
                        config.rmdir);

            /* compile wrapper-file */
            SystemCall ("%s %s %s %s -o %s/cwrapper.o -c %s/cwrapper.c", config.cc,
                        config.ccflags, config.ccdir, opt_buffer, tmp_dirname,
                        tmp_dirname);
            NOTEDOT;
        }

        if (linkstyle == 1) {
            SystemCall ("%s %s %s %s -o %s/%s.o -c %s/%s.c", config.cc, config.ccflags,
                        config.ccdir, opt_buffer, tmp_dirname, modulename, tmp_dirname,
                        modulename);
        } else {
            SystemCall ("%s %s %s %s -o %s/globals.o -c %s/globals.c", config.cc,
                        config.ccflags, config.ccdir, opt_buffer, tmp_dirname,
                        tmp_dirname);
            NOTEDOT;

            for (i = 1; i < function_counter; i++) {
                SystemCall ("%s %s %s %s -o %s/fun%d.o -c %s/fun%d.c", config.cc,
                            config.ccflags, config.ccdir, opt_buffer, tmp_dirname, i,
                            tmp_dirname, i);
                NOTEDOT;
            }
            NOTE (("\n"));
        }
    }

    DBUG_VOID_RETURN;
}
