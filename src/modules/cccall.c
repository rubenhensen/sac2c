/*
 *
 * $Log$
 * Revision 1.11  1997/03/19 13:46:23  cg
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

#if 0

==========================================================================

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

void AddToLinklist(ids *new)
{
  ids *tmp, *rest;
  
  DBUG_ENTER("AddToLinklist");
  
  while (new!=NULL)
  {
    tmp=linklist;
    
    while ((tmp!=NULL) && (0!=strcmp(IDS_NAME(new), IDS_NAME(tmp))))
    {
      tmp=IDS_NEXT(tmp);
    }
    
    rest=IDS_NEXT(new);

    if (tmp==NULL)
    {
      IDS_NEXT(new)=linklist;
      linklist=new;
    }
    else
    {
      FreeOneIds(new);
    }
    
    new=rest;
  }
  
  DBUG_VOID_RETURN;
}



/*
 *
 *  functionname  : PrintLinklist
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

void PrintLinklist(FILE *sib)
{
  ids *tmp;
  
  DBUG_ENTER("PrintLinklist");
  
  tmp=linklist;
  
  if (tmp!=NULL)
  {
    fprintf(sib, "#pragma linkwith %s\"%s\"",
            IDS_ISEXTERNAL(tmp)?"external ":"", IDS_NAME(tmp));
    
    tmp=IDS_NEXT(tmp);
    
    while (tmp!=NULL)
    {
      fprintf(sib, ",\n                 %s\"%s\"",
            IDS_ISEXTERNAL(tmp)?"external ":"", IDS_NAME(tmp));
      tmp=IDS_NEXT(tmp);
    }
    fprintf(sib, "\n\n");
  }
  
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

int IsStandardMod(char *name)
{
  static char *standard_modules[]=
  {
  /* standard modules */

    /* structures */
    "String", "StringC", "StringSAC", "Char", "List", "Complex",
    
    /* numerical */
    "Math", "MathC", "MathSAC",

  /* standard classes */

    /* world */
    "World",

    /* stdio */
    "File", "TermFile", "ScalarIO", "ArrayIO", "PrintArray",
    "FibreIO", "FibrePrint", "FibreScan", "ComplexIO",
    "ListIO", "StdIO",

    /* system */
    "CommandLine", "Env", "EnvVar", "Rand", "Random", "Rand48",
    "SysErr", "Time",

    ""
  };

  int i, res;
    
  DBUG_ENTER("IsStandardMod");
  
  i=0;
  
  while ((standard_modules[i][0]!=0)
         && (strcmp(standard_modules[i], name) != 0))
  {
    i+=1;
  }
  
  if (standard_modules[i][0]==0)
  {
    res=0;
  }
  else
  {
    res=1;
  }
  
  DBUG_RETURN(res);
}
  



/*
 *
 *  functionname  : GenLibStat
 *  arguments     : 
 *  description   : 
 *  global vars   : dependencies
 *  internal funs : 
 *  external funs : 
 *  macros        : 
 *
 *  remarks       : 
 *
 */

void GenLibStat()
{
  FILE *statusfile;
  ids *tmp;
  long int current;
  strings *str;
  
  DBUG_ENTER("GenLibStat");
  
  statusfile=WriteOpen("%s%s.stt", build_dirname, modulename);
  
  current=time(NULL);
    
  fprintf(statusfile, "\n***  Status Report - %s.lib  ***\n\n", modulename);
  fprintf(statusfile, "Call : %s\n", commandline);
  fprintf(statusfile, "From : %s\n", getenv("PWD"));
  fprintf(statusfile, "On   : %s\n", getenv("HOST"));
  fprintf(statusfile, "By   : %s\n", getenv("USER"));
  fprintf(statusfile, "Date : %s", ctime(&current));
   
  fprintf(statusfile, 
          "\nDependencies from imported modules and classes :\n");

  str=imported_decs;

  while (str!=NULL)
  {
    fprintf(statusfile, "%s\n", STRINGS_STRING(str));
    str=STRINGS_NEXT(str);
  }
  

  if (linkstyle==3)
  {
    fprintf(statusfile, "\nIncluded non-standard libraries :\n");
  }
  else
  {
    fprintf(statusfile, "\nRequired non-standard libraries :\n");
  }
  
  tmp=linklist;

  while (tmp!=NULL)
  {
    if (!IsStandardMod(IDS_NAME(tmp))
        && (!IDS_ISEXTERNAL(tmp)
            || (0!=strncmp("lib", IDS_NAME(tmp), 3))))
    {
      fprintf(statusfile, "  %-15s found : %s\n", 
              IDS_NAME(tmp), IDS_MOD(tmp));
    }
    
    tmp=IDS_NEXT(tmp);
  }


  fprintf(statusfile, "\nRequired standard libraries :\n");

  tmp=linklist;

  while (tmp!=NULL)
  {
    if (IsStandardMod(IDS_NAME(tmp)))
    {
      fprintf(statusfile, "  %-15s found : %s\n",
              IDS_NAME(tmp), IDS_MOD(tmp));
    }
    
    tmp=IDS_NEXT(tmp);
  }


  fprintf(statusfile, "\nRequired system libraries :\n");

  tmp=linklist;

  while (tmp!=NULL)
  {
    if (!IsStandardMod(IDS_NAME(tmp))
        && IDS_ISEXTERNAL(tmp)
        && (0==strncmp("lib", IDS_NAME(tmp), 3)))
    {
      fprintf(statusfile, "  %-15s found : not checked\n", IDS_NAME(tmp));
    }
    
    tmp=IDS_NEXT(tmp);
  }


  fprintf(statusfile, "\n***  Status Report - %s.lib  ***\n\n", modulename);
  
  fclose(statusfile);
    
  DBUG_VOID_RETURN;
}


/*
 *
 *  functionname  : SearchLinkFiles
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

void SearchLinkFiles()
{
  ids *tmp;
  static char buffer[MAX_FILE_NAME];
  char *pathname, *abspathname, *standard, *external, *systemlib;
  int stdmod, issystemlib;
  
  DBUG_ENTER("SearchLinkFiles");
  
  tmp=linklist;
  
  while (tmp!=NULL)
  {
    if (IsStandardMod(IDS_NAME(tmp)))
    {
      standard="standard ";
      stdmod=1;
    }
    else
    {
      standard="";
      stdmod=0;
    }

    if (IDS_ISEXTERNAL(tmp))
    {
      external="external";
      
      if ((0==strncmp(IDS_NAME(tmp), "lib", 3)) && !stdmod)
      {
        systemlib="system ";
        issystemlib=1;
      }
      else
      {
        systemlib="";
        issystemlib=0;
        link_archives=1;
      }
      
    }
    else
    {
      external="SAC";
      link_archives=1;
    }
    
        
    NOTE(("Required for linking: %s %s%slibrary '%s` ...", 
          external, standard, systemlib, IDS_NAME(tmp)));
  
    if (IDS_ISEXTERNAL(tmp))
    {
      if (issystemlib)
      {
        NOTE(("  Not checked !"));
        sprintf(buffer, " -l%s.a", IDS_NAME(tmp)+3);
        strcat(systemlibs, buffer);
      }
      else
      {
        strcpy(buffer, IDS_NAME(tmp));
        strcat(buffer, ".a");

        pathname=FindFile(MODIMP_PATH, buffer);

        if (pathname==NULL)
        {
          strcpy(buffer, IDS_NAME(tmp));
          strcat(buffer, ".o");

          pathname=FindFile(MODIMP_PATH, buffer);

          if (pathname==NULL)
          {
            if (SystemTest("-f %s%s.a", tmp_dirname, IDS_NAME(tmp)))
            {
              NOTE(("  Found implicit version !"));

              IDS_MOD(tmp)=StringCopy("implicit version");
          
              if ((linkstyle==0) || ((linkstyle==3) && !stdmod))
              {
                SystemCall("cd %s; ln -s %s%s.a", build_dirname,
                           tmp_dirname, IDS_NAME(tmp));
              }
            }
            else
            {
              SYSERROR(("Unable to find %s %s%slibrary '%s`",
                        external, standard, systemlib, IDS_NAME(tmp)));
            }
          }
          else
          {
            abspathname=AbsolutePathname(pathname);
      
            NOTE(("  Found \"%s\" !", abspathname));

            IDS_MOD(tmp)=StringCopy(abspathname);
          
            if ((linkstyle==0) || ((linkstyle==3) && !stdmod))
            {
              SystemCall("ar cr %s%s.a %s",
                         build_dirname, IDS_NAME(tmp), abspathname);
              if (useranlib)
              {
                SystemCall("ranlib %s%s.a", build_dirname, IDS_NAME(tmp));
              }
            }
          }
        }
        else
        {
          abspathname=AbsolutePathname(pathname);
      
          NOTE(("  Found \"%s\" !", abspathname));

          IDS_MOD(tmp)=StringCopy(abspathname);
          
          if ((linkstyle==0) || ((linkstyle==3) && !stdmod))
          {
            SystemCall("cd %s; ln -s %s", build_dirname, abspathname);
          }
        }
      }
      
    }
    else
    {
      strcpy(buffer, IDS_NAME(tmp));
      strcat(buffer, ".lib");

      pathname=FindFile(MODIMP_PATH, buffer);

      if (pathname==NULL)
      {
        if (SystemTest("-f %s%s.a", tmp_dirname, IDS_NAME(tmp)))
        {
          NOTE(("  Found implicit version !"));

          IDS_MOD(tmp)=StringCopy("implicit version");
          
          if ((linkstyle==0) || ((linkstyle==3) && !stdmod))
          {
            SystemCall("cd %s; ln -s %s%s.a", build_dirname,
                       tmp_dirname, IDS_NAME(tmp));
          }
        }
        else
        {
          SYSERROR(("Unable to find %s %s%slibrary '%s`",
                    external, standard, systemlib, IDS_NAME(tmp)));
        }
      }
      else
      {
        abspathname=AbsolutePathname(pathname);
    
        NOTE(("  Found \"%s\" !", abspathname));

        IDS_MOD(tmp)=StringCopy(abspathname);
          
        if ((linkstyle==0) || ((linkstyle==3) && !stdmod))
        {
          SystemCall("cd %s; tar xf %s %s.a", 
                     build_dirname, abspathname, IDS_NAME(tmp));
        }
      }

    }

    tmp=IDS_NEXT(tmp);
  }
    
  DBUG_VOID_RETURN;
}



============================================================================
#endif

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

strings *
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

void
GenLibStat ()
{
    FILE *statusfile;
    deps *tmp;
    long int current;

    DBUG_ENTER ("GenLibStat");

    statusfile = WriteOpen ("%s/%s.stt", tmp_dirname, modulename);

    current = time (NULL);

    fprintf (statusfile, "\n***  Status Report - %s.lib  ***\n\n", modulename);
    fprintf (statusfile, "Call : %s\n", commandline);
    fprintf (statusfile, "From : %s\n", getenv ("PWD"));
    fprintf (statusfile, "On   : %s\n", getenv ("HOST"));
    fprintf (statusfile, "By   : %s\n", getenv ("USER"));
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
    char *pathname, *abspathname, *modname, *nopath;
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

    success = SystemCall2 ("cd %s; tar xf %s %s.stt >/dev/null 2>&1", tmp_dirname,
                           abspathname, modname);

    if (success != 0) {
        SYSABORT (("Corrupted library file format: \"%s\"", abspathname));
    }

    SystemCall ("cat %s/%s.stt", tmp_dirname, modname);

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : CountLinklistLength
 *  arguments     : 1) dependency tree
 *  description   : counts the length of the link list which is used to
 *                  allocate the appropriate amount of memory before
 *                  generating the link list.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : strlen
 *  macros        :
 *
 *  remarks       : For SAC libraries the pure module/class name is appended
 *                  by '.a' because all archives extracted from SAC libraries
 *                  reside in a temporary directory.
 *                  For external and system libraries the respective pathname
 *                  is used.
 *
 */

int
CountLinklistLength (deps *depends)
{
    deps *tmp;
    int cnt = 0;

    DBUG_ENTER ("CountLinklistLength");

    tmp = depends;

    while (tmp != NULL) {
        if (DEPS_STATUS (tmp) == ST_sac) {
            cnt += strlen (DEPS_NAME (tmp)) + 3;
        } else {
            cnt += strlen (DEPS_LIBNAME (tmp)) + 1;
        }

        if (DEPS_SUB (tmp) != NULL) {
            cnt += CountLinklistLength (DEPS_SUB (tmp));
        }

        tmp = DEPS_NEXT (tmp);
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
 *  remarks       : For SAC libraries the pure module/class name is appended
 *                  by '.a' because all archives extracted from SAC libraries
 *                  reside in a temporary directory.
 *                  For external and system libraries the respective pathname
 *                  is used.
 *
 */

void
FillLinklist (deps *depends, char *linklist)
{
    deps *tmp;

    DBUG_ENTER ("FillLinklist");

    tmp = depends;

    while (tmp != NULL) {
        if (DEPS_STATUS (tmp) == ST_sac) {
            strcat (linklist, DEPS_NAME (tmp));
            strcat (linklist, ".a ");
        } else {
            strcat (linklist, DEPS_LIBNAME (tmp));
            strcat (linklist, " ");
        }

        if (DEPS_SUB (tmp) != NULL) {
            FillLinklist (DEPS_SUB (tmp), linklist);
        }

        tmp = DEPS_NEXT (tmp);
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : GenLinklist
 *  arguments     : 1) dependency tree
 *  description   : allocates memory for the link list and fills it with
 *                  the dependent libraries extracted from the dependency tree
 *  global vars   : ---
 *  internal funs : CountLinklistLength, FillLinklist
 *  external funs : Malloc, strcpy
 *  macros        :
 *
 *  remarks       :
 *
 */

char *
GenLinklist (deps *depends)
{
    char *linklist;
    int size;

    DBUG_ENTER ("GenLinklist");

    size = CountLinklistLength (depends) + 5;

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

    NOTE (("Creating SAC library \"%s%s.lib\"", targetdir, modulename));

    SystemCall ("ar cr %s/%s.a %s/*.o", tmp_dirname, modulename, tmp_dirname);

    if (useranlib) {
        SystemCall ("ranlib %s/%s.a", tmp_dirname, modulename);
    }

    GenLibStat ();

    SystemCall ("cd %s; tar cf %s.lib %s.a %s.sib %s.stt", tmp_dirname, modulename,
                modulename, modulename, modulename);

    SystemCall ("mv %s/%s.lib %s", tmp_dirname, modulename, targetdir);

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
 *  The gcc compiler options:
 *  -Wall                    enable all warnings
 *  -Wno-unused              don't warn upon unused local variables
 *  -fno-builtin             don't use builtin functions not starting
 *                           with two underscores
 *  -I$RCSROOT/src/compile/  additional include path:
 *                             all files include icm2c.h and libsac.h
 *  -L$RCSROOT/src/compile/  additional link path:
 *                             all programs are linked with libsac.a
 *
 */

void
InvokeCC ()
{
    int i;

    DBUG_ENTER ("InvokeCC");

    if (filetype == F_prog) {
        SystemCall ("gcc %s -Wall -Wno-unused -fno-builtin "
                    "-I$RCSROOT/src/compile/ -L$RCSROOT/src/compile/ "
                    "-L%s "
                    "-o %s %s %s -lsac",
                    ccflagsstr, tmp_dirname, outfilename, cfilename,
                    GenLinklist (dependencies));
    } else {
        if (linkstyle == 1) {
            SystemCall ("gcc %s -Wall -Wno-unused -fno-builtin "
                        "-I$RCSROOT/src/compile/ -L$RCSROOT/src/compile/ "
                        "-o %s%s.o -c %s/%s.c",
                        ccflagsstr, tmp_dirname, modulename, tmp_dirname, modulename);
        } else {
            SystemCall ("gcc %s -Wall -Wno-unused -fno-builtin "
                        "-I$RCSROOT/src/compile/ -L$RCSROOT/src/compile/ "
                        "-o %s/globals.o -c %s/globals.c",
                        ccflagsstr, tmp_dirname, tmp_dirname);
            NOTE ((" **\n"));

            for (i = 1; i < function_counter; i++) {
                SystemCall ("gcc %s -Wall -Wno-unused -fno-builtin "
                            "-I$RCSROOT/src/compile/ -L$RCSROOT/src/compile/ "
                            "-o %s/fun%d.o -c %s/fun%d.c",
                            ccflagsstr, tmp_dirname, i, tmp_dirname, i);
                NOTE ((" ** %d\n", i));
            }
        }
    }

    DBUG_VOID_RETURN;
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
