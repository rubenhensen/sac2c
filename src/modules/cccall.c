/*
 *
 * $Log$
 * Revision 1.3  1996/01/02 16:01:35  cg
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

#define MAKESYSCALL                                                                      \
    NOTE (("%s", systemcall));                                                           \
    system (systemcall);                                                                 \
    DBUG_EXECUTE ("SYSSTEP", printf ("\nHit key to continue\n");                         \
                  scanf ("%s", systemcall););

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
 *  functionname  : MakeArchive
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
MakeArchive (char *name)
{
    char *systemcall;
    strings *tmp;

    DBUG_ENTER ("MakeArchive");

    systemcall = (char *)Malloc ((strlen (ccflagsstr) + 2 * strlen (name) + MAX_PATH_LEN)
                                 * sizeof (char));

    sprintf (systemcall,
             "gcc %s -Wall -Wno-unused -I $RCSROOT/src/compile/"
             " -o %s.o -c %s.c",
             ccflagsstr, name, name);

    NOTE2 (("*** Invoking C-compiler:"));

    MAKESYSCALL;

    NEWLINE (2);

    NOTE2 (("*** Creating archive %s.a", name));

    sprintf (systemcall, "mkdir __tmp_sac");
    MAKESYSCALL;

    sprintf (systemcall, "mkdir __tmp_store_o");
    MAKESYSCALL;

    sprintf (systemcall, "mv %s.o __tmp_sac", name);
    MAKESYSCALL;

    sprintf (systemcall, "touch __tmp_store.o");
    MAKESYSCALL;

    /*
     *  The last system call assures that at least one .o file exists in
     *  the current directory, otherwise 'mv *.o ...' causes error messages.
     */

    sprintf (systemcall, "mv *.o __tmp_store_o");
    MAKESYSCALL;

    tmp = afilelist;

    while (tmp != NULL) {
        sprintf (systemcall, "ar x %s", STRINGS_STRING (tmp));
        MAKESYSCALL;

        sprintf (systemcall, "mv *.o __tmp_sac");
        MAKESYSCALL;

        sprintf (systemcall, "rm -f __.SYMDEF");
        MAKESYSCALL;

        tmp = FreeOneStrings (tmp);
    }

    tmp = ofilelist;

    while (tmp != NULL) {
        sprintf (systemcall, "cp %s __tmp_sac", STRINGS_STRING (tmp));
        MAKESYSCALL;

        tmp = FreeOneStrings (tmp);
    }

    sprintf (systemcall, "ar rc %s.a __tmp_sac/*.o", name);
    MAKESYSCALL;

    sprintf (systemcall, "ranlib %s.a", name);
    MAKESYSCALL;

    sprintf (systemcall, "mv  __tmp_store_o/*.o .");
    MAKESYSCALL;

    sprintf (systemcall, "rm __tmp_store.o");
    MAKESYSCALL;

    sprintf (systemcall, "rm __tmp_sac/*.o");
    MAKESYSCALL;

    sprintf (systemcall, "rmdir __tmp_sac");
    MAKESYSCALL;

    sprintf (systemcall, "rmdir __tmp_store_o");
    MAKESYSCALL;

    NEWLINE (2);

    FREE (systemcall);

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : MakeExecutable
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
MakeExecutable ()
{
    char *systemcall;
    strings *tmp;

    DBUG_ENTER ("MakeExecutable");

    systemcall = (char *)Malloc ((StringsLength (afilelist) + StringsLength (ofilelist)
                                  + strlen (ccflagsstr) + strlen (outfilename)
                                  + strlen (cfilename) + MAX_PATH_LEN)
                                 * sizeof (char));

    sprintf (systemcall, "gcc %s -Wall -Wno-unused -I $RCSROOT/src/compile/ -o %s %s",
             ccflagsstr, outfilename, cfilename);

    tmp = ofilelist;

    while (tmp != NULL) {
        strcat (systemcall, " ");
        strcat (systemcall, STRINGS_STRING (tmp));

        tmp = FreeOneStrings (tmp);
    }

    tmp = afilelist;

    while (tmp != NULL) {
        strcat (systemcall, " ");
        strcat (systemcall, STRINGS_STRING (tmp));

        tmp = FreeOneStrings (tmp);
    }

    NOTE2 (("*** Invoking C-compiler:"));

    MAKESYSCALL;

    NEWLINE (2);

    FREE (systemcall);

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

void
InvokeCC (node *modul)
{
    DBUG_ENTER ("InvokeCC");

    if (MODUL_FILETYPE (modul) == F_prog) {
        MakeExecutable ();
    } else {
        MakeArchive (MODUL_NAME (modul));
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : SearchLinkFiles
 *  arguments     :
 *  description   :
 *  global vars   : linklist, afilelist, ofilelist
 *  internal funs : ---
 *  external funs : FindFile, StringCopy, strcpy, strcat, MakeStrings
 *  macros        : MAX_FILE_NAME
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
        strcpy (buffer, STRINGS_STRING (tmp));
        strcat (buffer, ".a");

        pathname = FindFile (MODIMP_PATH, buffer);

        if (pathname == NULL) {
            strcpy (buffer, STRINGS_STRING (tmp));
            strcat (buffer, ".o");

            pathname = FindFile (MODIMP_PATH, buffer);

            if (pathname == NULL) {
                SYSERROR (("Unable to find implementation of module/class '%s`",
                           STRINGS_STRING (tmp)));
            } else {
                ofilelist = MakeStrings (StringCopy (pathname), ofilelist);

                DBUG_PRINT ("LINK", ("Added %s to ofilelist", pathname));
            }
        } else {
            afilelist = MakeStrings (StringCopy (pathname), afilelist);

            DBUG_PRINT ("LINK", ("Added %s to afilelist", pathname));
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
