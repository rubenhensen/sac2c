/*
 *
 * $Log$
 * Revision 1.5  1996/01/07 16:57:30  cg
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
static strings *ofilelist = NULL;
static strings *afilelist = NULL;

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

#if 0

/*
 *
 *  functionname  : CheckExternalImplementation
 *  arguments     : 1) name of external module/class
 *  description   : looks for implementation of module/class
 *                  and copies it to store_dirname
 *  global vars   : store_dirname, ext_mods, sibs
 *  internal funs : ---
 *  external funs : SystemCall, strcpy, strcat, FindFile
 *  macros        : MODIMP_PATH
 *
 *  remarks       : 
 *
 */

void CheckExternalImplementation(char *name)
{
  strings *tmp;
  static char buffer[MAX_FILE_NAME];
  char *pathname;
  node *tmpsibs;
  
  DBUG_ENTER("CheckExternalImplementation");
  
  DBUG_ASSERT(name!=NULL,
              "called CheckExternalImplementation with name==NULL");
  
  /*
   *  First, we look among sibs because even external items can be imported
   *  through sibs rather than directly.
   */

  strcpy(buffer, name);
  strcat(buffer, ".a");
  
  if (!SystemTest("-f %s%s", store_dirname, buffer))
  {
    strcpy(buffer, name);
    strcat(buffer, ".o");

    if (!SystemTest("-f %s%s", store_dirname, buffer))
    {
      NOTE(("Searching for implementation of external module/class '%s` ..."
            , name));

      pathname=FindFile(MODIMP_PATH, buffer);

      if (pathname==NULL)
      {
        strcpy(buffer, name);
        strcat(buffer, ".a");
    
        pathname=FindFile(MODIMP_PATH, buffer);

        if (pathname==NULL)
        {
          SYSABORT(("Unable to find implementation of external "
                    "module/class '%s`",
                    name));
        }
        else
        {
          NOTE(("  Found archive file \"%s\" !", pathname));
          SystemCall("cp %s %s", pathname, store_dirname);
        }
      }
      else
      {
        NOTE(("  Found object file \"%s\"", pathname));
        SystemCall("cp %s %s", pathname, store_dirname);
      }
    }
  }
  
  DBUG_VOID_RETURN;
}
#endif

#if 0
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

void MakeObjectFile(char *name)
{
  char *systemcall;
  int exit_code;
  
  DBUG_ENTER("MakeObjectFile");
  
  systemcall=(char*)Malloc((strlen(ccflagsstr)
                            + 2*strlen(name)
                            + MAX_PATH_LEN)
                           *sizeof(char));
  
  sprintf(systemcall,
          "gcc %s -Wall -Wno-unused -I $RCSROOT/src/compile/"
          " -o %s%s -c %s%s",
          ccflagsstr, build_dirname, outfilename, targetdir, cfilename);

  NEWLINE(3);
  
  NOTE(("%s", systemcall));
  
  exit_code=system(systemcall);
  
  if (exit_code>0)
  {
    NEWLINE(0);
    SYSABORT(("Compilation to object file failed (%d)", exit_code/256));
  }
  
  FREE(systemcall);
  
  DBUG_VOID_RETURN;
}
#endif

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
    char *name;
    strings *tmp;

    DBUG_ENTER ("CreateLibrary");

    name = MODUL_NAME (syntax_tree);

    NOTE (("Creating SAC library \"%s%s.lib\"", targetdir, name));

    tmp = afilelist;

    while (tmp != NULL) {
        SystemCall ("cd %s; ar x %s", build_dirname, STRINGS_STRING (tmp));
        tmp = STRINGS_NEXT (tmp);
    }

    SystemCall ("ar rc %s%s.a %s*.o", build_dirname, name, build_dirname);
    SystemCall ("ranlib %s%s.a", build_dirname, name);

    SystemCall ("cat %s%s.sib %s%s.a > %s%s.lib", store_dirname, name, build_dirname,
                name, targetdir, name);

    DBUG_VOID_RETURN;
}

#if 0

/*
 *
 *  functionname  : MakeExecutable
 *  arguments     : ---
 *  description   : generates string to call gcc including all object and
 *                  archive files to link with and executes it
 *  global vars   : afilelist, ofilelist
 *  internal funs : ---
 *  external funs : Malloc, sprintf, strcat, FreeOneStrings, system,
 *                  StringsLength
 *  macros        : MAX_PATH_LEN, FREE
 *
 *  remarks       : 
 *
 */

void MakeExecutable()
{
  strings *tmp;
  int exit_code;
  
  DBUG_ENTER("MakeExecutable");
  
  SystemCall("gcc %s -Wall -Wno-unused -I $RCSROOT/src/compile/ "
             "-o %s %s %s*.o %s*.a",
             ccflagsstr, outfilename, cfilename,
             build_dirname, build_dirname);
 
  tmp=ofilelist;
  
  while (tmp!=NULL)
  {
    strcat(systemcall, " ");
    strcat(systemcall, STRINGS_STRING(tmp));

    tmp=FreeOneStrings(tmp);
  }
 
  tmp=afilelist;
  
  while (tmp!=NULL)
  {
    strcat(systemcall, " ");
    strcat(systemcall, STRINGS_STRING(tmp));

    tmp=FreeOneStrings(tmp);
  }

  NEWLINE(3);
  
  NOTE(("%s", systemcall));
  
  exit_code=system(systemcall);
  
  if (exit_code>0)
  {
    NEWLINE(0);
    SYSABORT(("Compilation to executable failed (%d)", exit_code/256));
  }
  
  FREE(systemcall);
  
  DBUG_VOID_RETURN;
}
#endif

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
    DBUG_ENTER ("InvokeCC");

    if (MODUL_FILETYPE (syntax_tree) == F_prog) {
        char ofiles[MAX_FILE_NAME] = "";
        char afiles[MAX_FILE_NAME] = "";

        if (ofilelist != NULL) {
            sprintf (ofiles, "%s*.o", build_dirname);
        }

        if (afilelist != NULL) {
            sprintf (afiles, "%s*.a", build_dirname);
        }

        SystemCall ("gcc %s -Wall -Wno-unused -I $RCSROOT/src/compile/ "
                    "-o %s %s %s %s",
                    ccflagsstr, outfilename, cfilename, ofiles, afiles);
    } else {
        SystemCall ("gcc %s -Wall -Wno-unused -I $RCSROOT/src/compile/"
                    " -o %s%s -c %s%s",
                    ccflagsstr, build_dirname, outfilename, targetdir, cfilename);
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : SearchLinkFiles
 *  arguments     : ---
 *  description   : traverses the linklist and searches for the
 *                  implementations in store_dirname. The complete pathname
 *                  of archive files is written to afilelist. Object files
 *                  are moved to build_dirname and that new complete
 *                  pathname is written to ofilelist.
 *  global vars   : linklist, afilelist, ofilelist
 *  internal funs : ---
 *  external funs : StringCopy, sprintf, MakeStrings, SystemCall, SystemTest
 *  macros        : MAX_PATH_LEN
 *
 *  remarks       :
 *
 */

void
SearchLinkFiles ()
{
    strings *tmp, *old;
    char buffer[MAX_FILE_NAME];
    char *pathname;

    DBUG_ENTER ("SearchLinkFiles");

    tmp = linklist;

    while (tmp != NULL) {
        if (SystemTest ("-f %s%s.a", store_dirname, STRINGS_STRING (tmp))) {
            SystemCall ("mv %s%s.a %s", store_dirname, STRINGS_STRING (tmp),
                        build_dirname);

            strcpy (buffer, STRINGS_STRING (tmp));
            strcat (buffer, ".a");
            afilelist = MakeStrings (StringCopy (buffer), afilelist);
        } else {
            NOTE (("Searching for implementation of external module/class '%s` ...",
                   STRINGS_STRING (tmp)));

            strcpy (buffer, STRINGS_STRING (tmp));
            strcat (buffer, ".o");

            pathname = FindFile (MODIMP_PATH, buffer);

            if (pathname == NULL) {
                strcpy (buffer, STRINGS_STRING (tmp));
                strcat (buffer, ".a");

                pathname = FindFile (MODIMP_PATH, buffer);

                if (pathname == NULL) {
                    SYSERROR (("Unable to find implementation of external "
                               "module/class '%s`",
                               STRINGS_STRING (tmp)));
                } else {
                    NOTE (("  Found archive file \"%s\" !", pathname));
                    SystemCall ("cp %s %s", pathname, build_dirname);
                    afilelist = MakeStrings (StringCopy (buffer), afilelist);
                }
            } else {
                NOTE (("  Found object file \"%s\"", pathname));
                SystemCall ("cp %s %s", pathname, build_dirname);
                ofilelist = MakeStrings (StringCopy (buffer), ofilelist);
            }
        }

        old = tmp;
        tmp = STRINGS_NEXT (tmp);
        FREE (old);
        /* The strings are not freed because they are shared. */
    }

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

void
AddToLinklist (char *mod)
{
    strings *tmp;

    DBUG_ENTER ("AddToLinklist");

    tmp = linklist;

    while ((tmp != NULL) && (strcmp (STRINGS_STRING (tmp), mod) != 0)) {
        tmp = STRINGS_NEXT (tmp);
    }

    if (tmp == NULL) {
        linklist = MakeStrings (mod, linklist);

        DBUG_PRINT ("LINK", ("Added %s to linklist", mod));
    } else {
        DBUG_PRINT ("LINK", ("module %s already in linklist", mod));
    }

    DBUG_VOID_RETURN;
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
        AddToLinklist (FUNDEF_LINKMOD (arg_node));
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_info);
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
        AddToLinklist (OBJDEF_LINKMOD (arg_node));
    }

    if (OBJDEF_NEXT (arg_node) != NULL) {
        Trav (OBJDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_info);
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

    SearchLinkFiles ();

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
