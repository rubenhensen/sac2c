/*
 *
 * $Log$
 * Revision 1.10  1997/03/11 16:29:10  cg
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

static strings *linklist = NULL;
static strings *included_libs = NULL;
static strings *required_libs = NULL;
static strings *required_stdlibs = NULL;

strings *imported_decs = NULL; /* set by import.c */

static int link_archives = 0;

/*
 *
 *  functionname  :
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

/*
 *
 *  functionname  :
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

/*
void UpdateMakefile()
{
  FILE *makefile, *new;
  char buffer[255], *newname, makefilename[30];
  int depflag=0;
  strings *deps;

  DBUG_ENTER("UpdateMakefile");

  makefile=fopen("Makefile", "r");

  if (makefile==NULL)
  {
    makefile=fopen("makefile", "r");

    if (makefile==NULL)
    {
      SYSWARN(("Unable to update makefile in current directory"));
    }
    else
    {
      strcpy(makefilename, "makefile");
    }

  }
  else
  {
    strcpy(makefilename, "Makefile");
  }

  if (makefile!=NULL)
  {
    newname = tmpnam(NULL);
    new = WriteOpen(newname);

    while (!feof(makefile))
    {
      fgets(buffer, 255, makefile);

      if (strncmp(buffer, "# DO NOT DELETE", 15)==0)
      {
        depflag=1;
      }

      if ((!depflag) || (strncmp(outfilename, buffer, strlen(outfilename))!=0))
      {
        fprintf(new, "%s", buffer);
      }
    }

    if (!depflag)
    {
      fprintf(new, "\n# DO NOT DELETE THIS LINE"
              "-- make depend depends on it.\n\n");
    }

    deps=dependencies;

    while (deps!=NULL)
    {
      fprintf(new, "%s: %s\n", outfilename, STRINGS_STRING(deps));
      deps=STRINGS_NEXT(deps);
    }

    fclose(makefile);
    fclose(new);

    SystemCall("mv %s %s", newname, makefilename);

  }

  DBUG_VOID_RETURN;
}

UpdateMakefile() is no longer used with the new -M compiler option.
All dependencies are now written directly to stdout to be conform
with gcc and other C compilers.
*/

/*
 *
 *  functionname  :
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

void
StoreRequiredMod (char *name, char *pathname, int stdmod)
{
    static char buffer[MAX_PATH_LEN];

    DBUG_ENTER ("StoreRequiredMod");

    if (pathname == NULL) {
        sprintf (buffer, "  %-15sfound implicit version", name);
    } else {
        sprintf (buffer, "  %-15sfound : %s", name, pathname);
    }

    if (stdmod) {
        required_stdlibs = MakeStrings (StringCopy (buffer), required_stdlibs);
    } else {
        if (linkstyle == 3) {
            included_libs = MakeStrings (StringCopy (buffer), included_libs);
        } else {
            required_libs = MakeStrings (StringCopy (buffer), required_libs);
        }
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : PrintLibStat
 *  arguments     : ---
 *  description   : extracts status file from SAC library and prints it
 *                  to stdout
 *  global vars   : sacfilename
 *  internal funs : ---
 *  external funs : SystemCall, SystemCall2, StringCopy, strlen, FindFile,
 *                  strrchr
 *  macros        :
 *
 *  remarks       :
 *
 */

void
PrintLibStat ()
{
    char *pathname, *modname, *nopath;
    int success;

    DBUG_ENTER ("PrintLibStat");

    pathname = FindFile (MODIMP_PATH, sacfilename);

    if (pathname == NULL) {
        SYSABORT (("Unable to find library file \"%s\"", sacfilename));
    }

    nopath = strrchr (sacfilename, '/');

    modname = StringCopy (nopath == NULL ? sacfilename : (nopath + 1));

    modname[strlen (modname) - 4] = 0;

    success = SystemCall2 ("tar xf %s %s.stt >/dev/null 2>&1", pathname, modname);

    if (success != 0) {
        SYSABORT (("Corrupted library file format: \"%s\"", pathname));
    }

    SystemCall ("cat %s.stt", modname);

    SystemCall ("rm -f %s.stt", modname);

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : GenLibStat
 *  arguments     :
 *  description   :
 *  global vars   : included_libs, required_libs, required_stdlibs
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

void
GenLibStat ()
{
    FILE *statusfile;
    strings *tmp;
    long int current;

    DBUG_ENTER ("GenLibStat");

    statusfile = WriteOpen ("%s%s.stt", build_dirname, modulename);

    current = time (NULL);

    fprintf (statusfile, "\n***  Status Report - %s.lib  ***\n\n", modulename);
    fprintf (statusfile, "Call : %s\n", commandline);
    fprintf (statusfile, "From : %s\n", getenv ("PWD"));
    fprintf (statusfile, "On   : %s\n", getenv ("HOST"));
    fprintf (statusfile, "By   : %s\n", getenv ("USER"));
    fprintf (statusfile, "Date : %s", ctime (&current));

    tmp = imported_decs;

    fprintf (statusfile, "\nDependencies from imported modules and classes :  %s\n",
             tmp == NULL ? "none" : "");

    while (tmp != NULL) {
        fprintf (statusfile, "%s\n", STRINGS_STRING (tmp));
        tmp = STRINGS_NEXT (tmp);
    }

    tmp = included_libs;

    fprintf (statusfile, "\nIncluded libraries :  %s\n", tmp == NULL ? "none" : "");

    while (tmp != NULL) {
        fprintf (statusfile, "%s\n", STRINGS_STRING (tmp));
        tmp = STRINGS_NEXT (tmp);
    }

    tmp = required_libs;

    fprintf (statusfile, "\nRequired libraries :  %s\n", tmp == NULL ? "none" : "");

    while (tmp != NULL) {
        fprintf (statusfile, "%s\n", STRINGS_STRING (tmp));
        tmp = STRINGS_NEXT (tmp);
    }

    tmp = required_stdlibs;

    fprintf (statusfile, "\nRequired standard libraries :  %s\n",
             tmp == NULL ? "none" : "");

    while (tmp != NULL) {
        fprintf (statusfile, "%s\n", STRINGS_STRING (tmp));
        tmp = STRINGS_NEXT (tmp);
    }

    fprintf (statusfile, "\n***  Status Report - %s.lib  ***\n\n", modulename);

    fclose (statusfile);

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : IsStandardMod
 *  arguments     : 1) module name
 *  description   : tests whether the given module is a standard one or not
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : strcmp
 *  macros        :
 *
 *  remarks       : The standard modules are hard coded as a list which
 *                  has to be kept up to date.
 *
 */

int
IsStandardMod (char *name)
{
    static char *standard_modules[]
      = {/* standard modules */

         /* structures */
         "String", "StringC", "StringSAC", "Char", "List", "Complex",

         /* numerical */
         "Math", "MathC", "MathSAC",

         /* standard classes */

         /* world */
         "World",

         /* stdio */
         "File", "TermFile", "ScalarIO", "ArrayIO", "PrintArray", "FibreIO", "FibrePrint",
         "FibreScan", "ComplexIO", "ListIO", "StdIO",

         /* system */
         "CommandLine", "Env", "EnvVar", "Rand", "Random", "Rand48", "SysErr", "Time",

         ""};

    int i, res;

    DBUG_ENTER ("IsStandardMod");

    i = 0;

    while ((standard_modules[i][0] != 0) && (strcmp (standard_modules[i], name) != 0)) {
        i += 1;
    }

    if (standard_modules[i][0] == 0) {
        res = 0;
    } else {
        res = 1;
    }

    DBUG_RETURN (res);
}

/*
 *
 *  functionname  : CreateLibrary
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

void
CreateLibrary (node *syntax_tree)
{
    DBUG_ENTER ("CreateLibrary");

    NOTE (("Creating SAC library \"%s%s.lib\"", targetdir, modulename));

    SystemCall ("ar cr %s%s.a %s*.o", build_dirname, modulename, store_dirname);

    if (useranlib) {
        SystemCall ("ranlib %s%s.a", build_dirname, modulename);
    }

    GenLibStat ();

    if (linkstyle == 3) {
        SystemCall ("cd %s; tar cf %s.lib *.a %s.sib %s.stt", build_dirname, modulename,
                    modulename, modulename);
    } else {
        SystemCall ("cd %s; tar cf %s.lib %s.a %s.sib %s.stt", build_dirname, modulename,
                    modulename, modulename, modulename);
    }

    SystemCall ("mv %s%s.lib %s", build_dirname, modulename, targetdir);

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : InvokeCC
 *  arguments     : 1) syntax tree
 *  description   : starts the gcc for final compilation
 *  global vars   : ccflagsstr, outfilename, cfilename, build_dirname,
 *                  targetdir
 *  internal funs : SystemCall
 *  external funs : ---
 *  macros        :
 *
 *  remarks       :
 *
 */

void
InvokeCC (node *syntax_tree)
{
    int i;

    DBUG_ENTER ("InvokeCC");

    if (MODUL_FILETYPE (syntax_tree) == F_prog) {
        if (link_archives) {
            SystemCall ("gcc %s -Wall -Wno-unused "
                        "-I$RCSROOT/src/compile/ -L$RCSROOT/src/compile/ "
                        "-fno-builtin "
                        "-o %s %s %s*.a -lsac -lm -ly -ll",
                        ccflagsstr, outfilename, cfilename, build_dirname);
        } else {
            SystemCall ("gcc %s -Wall -Wno-unused "
                        "-I$RCSROOT/src/compile/ -L$RCSROOT/src/compile/ "
                        "-fno-builtin "
                        "-o %s %s -lsac -lm -ly -ll",
                        ccflagsstr, outfilename, cfilename);
        }
    } else {
        if (linkstyle == 1) {
            SystemCall ("gcc %s -Wall -Wno-unused -I$RCSROOT/src/compile/ "
                        "-fno-builtin "
                        "-o %s%s.o -c %s%s.c",
                        ccflagsstr, store_dirname, modulename, targetdir, modulename);
        } else {
            SystemCall ("gcc %s -Wall -Wno-unused -I$RCSROOT/src/compile/ "
                        "-fno-builtin "
                        "-o %sglobals.o -c %sglobals.c",
                        ccflagsstr, store_dirname, store_dirname);
            NOTE ((" **\n"));

            for (i = 1; i < function_counter; i++) {
                SystemCall ("gcc %s -Wall -Wno-unused -I$RCSROOT/src/compile/ "
                            "-fno-builtin "
                            "-o %sfun%d.o -c %sfun%d.c",
                            ccflagsstr, store_dirname, i, store_dirname, i);
                NOTE ((" **\n"));
            }
        }
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : SearchLinkFile
 *  arguments     : ---
 *  description   :
 *
 *
 *
 *
 *  global vars   :
 *  internal funs : ---
 *  external funs :
 *  macros        : MAX_PATH_LEN
 *
 *  remarks       :
 *
 */

void
SearchLinkFile (char *name)
{
    char buffer[MAX_FILE_NAME];
    char *pathname, *abspathname, *standard;
    int stdmod;

    DBUG_ENTER ("SearchLinkFile");

    stdmod = IsStandardMod (name);

    if (stdmod) {
        standard = "standard ";
    } else {
        standard = "";
    }

    NOTE (("Required for linking: SAC %slibrary '%s` ...", standard, name));

    strcpy (buffer, name);
    strcat (buffer, ".lib");

    pathname = FindFile (MODIMP_PATH, buffer);

    if (pathname == NULL) {
        if (SystemTest ("-f %s%s.a", store_dirname, name)) {
            NOTE (("  Found implicit version !"));
            if ((linkstyle == 0) || ((linkstyle == 3) && !stdmod)) {
                SystemCall ("cd %s; ln -s %s%s.a", build_dirname, store_dirname, name);
            }
        } else {
            SYSERROR (("Unable to find SAC %slibrary '%s`", standard, name));
        }
    } else {
        abspathname = AbsolutePathname (pathname);

        NOTE (("  Found \"%s\" !", abspathname));

        if ((linkstyle == 0) || ((linkstyle == 3) && !stdmod)) {
            SystemCall ("cd %s; tar xf %s %s.a", build_dirname, abspathname, name);
        }
    }

    StoreRequiredMod (name, pathname, stdmod);

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : SearchExternalLinkFile
 *  arguments     : ---
 *  description   :
 *
 *
 *
 *
 *  global vars   :
 *  internal funs : ---
 *  external funs :
 *  macros        : MAX_PATH_LEN
 *
 *  remarks       :
 *
 */

void
SearchExternalLinkFile (char *name)
{
    char buffer[MAX_FILE_NAME];
    char *pathname, *standard;
    int stdmod;

    DBUG_ENTER ("SearchExternalLinkFile");

    stdmod = IsStandardMod (name);

    if (stdmod) {
        standard = "standard ";
    } else {
        standard = "";
    }

    NOTE (("Required for linking: external %slibrary '%s` ...", standard, name));

    strcpy (buffer, name);
    strcat (buffer, ".a");

    pathname = FindFile (MODIMP_PATH, buffer);

    if (pathname == NULL) {
        strcpy (buffer, name);
        strcat (buffer, ".o");

        pathname = FindFile (MODIMP_PATH, buffer);

        if (pathname == NULL) {
            if (SystemTest ("-f %s%s.a", store_dirname, name)) {
                NOTE (("  Found implicit version !"));
                if ((linkstyle == 0) || ((linkstyle == 3) && !stdmod)) {
                    SystemCall ("cd %s; ln -s %s%s.a", build_dirname, store_dirname,
                                name);
                }
            } else {
                SYSERROR (("Unable to find external %slibrary '%s`", standard, name));
            }
        } else {
            NOTE (("  Found \"%s\" !", pathname));
            if ((linkstyle == 0) || ((linkstyle == 3) && !stdmod)) {
                SystemCall ("ar cr %s%s.a %s", build_dirname, name, pathname);
            }
        }
    } else {
        NOTE (("  Found \"%s\" !", pathname));
        if ((linkstyle == 0) || ((linkstyle == 3) && !stdmod)) {
            SystemCall ("cd %s; ln -s %s", build_dirname, AbsolutePathname (pathname));
        }
    }

    StoreRequiredMod (name, pathname, stdmod);

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : AddToLinklist
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

int
AddToLinklist (char *mod)
{
    strings *tmp;
    int result;

    DBUG_ENTER ("AddToLinklist");

    tmp = linklist;

    while ((tmp != NULL) && (strcmp (STRINGS_STRING (tmp), mod) != 0)) {
        tmp = STRINGS_NEXT (tmp);
    }

    if (tmp == NULL) {
        linklist = MakeStrings (mod, linklist);
        result = 1;

        DBUG_PRINT ("LINK", ("Added %s to linklist", mod));
    } else {
        result = 0;

        DBUG_PRINT ("LINK", ("module %s already in linklist", mod));
    }

    DBUG_RETURN (result);
}

/*
 *
 *  functionname  : LINKfundef
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
LINKfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("LINKfundef");

    DBUG_PRINT ("LINK", ("Checking function %s", ItemName (arg_node)));

    if (FUNDEF_LINKMOD (arg_node) != NULL) {
        if (AddToLinklist (FUNDEF_LINKMOD (arg_node))) {
            if (FUNDEF_MOD (arg_node) == NULL) {
                SearchExternalLinkFile (FUNDEF_LINKMOD (arg_node));
            } else {
                SearchLinkFile (FUNDEF_LINKMOD (arg_node));
            }
        }

        link_archives = 1;
        /*
         * Flag is set at least one archive must be linked.
         */
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : LINKobjdef
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
LINKobjdef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("LINKobjdef");

    DBUG_PRINT ("LINK", ("Checking object %s", ItemName (arg_node)));

    if (OBJDEF_LINKMOD (arg_node) != NULL) {
        if (AddToLinklist (OBJDEF_LINKMOD (arg_node))) {
            if (OBJDEF_MOD (arg_node) == NULL) {
                SearchExternalLinkFile (OBJDEF_LINKMOD (arg_node));
            } else {
                SearchLinkFile (OBJDEF_LINKMOD (arg_node));
            }
        }
    }

    if (OBJDEF_NEXT (arg_node) != NULL) {
        Trav (OBJDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : LINKmodul
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
LINKmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("LINKmodul");

    if (MODUL_OBJS (arg_node) != NULL) {
        Trav (MODUL_OBJS (arg_node), arg_info);
    }

    if (MODUL_FUNS (arg_node) != NULL) {
        Trav (MODUL_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PrepareLinking
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
PrepareLinking (node *syntax_tree)
{
    DBUG_ENTER ("PrepareLinking");

    act_tab = link_tab;

    DBUG_RETURN (Trav (syntax_tree, NULL));
}

/*
 *
 *  functionname  :
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */
