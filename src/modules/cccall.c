/*
 *
 * $Log$
 * Revision 1.1  1995/12/29 17:19:26  cg
 * Initial revision
 *
 *
 *
 */

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"

#include "Error.h"
#include "dbug.h"

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
NOTE_COMPILER_PHASE;
if (MODUL_FILETYPE (syntax_tree) == F_prog) {
    sprintf (cccallstr,
             "gcc %s-Wall -Wno-unused -I $RCSROOT/src/compile/"
             " -o %s %s %s",
             ccflagsstr, outfilename, cfilename, GenLinkerList ());
} else {
    if ((MODUL_FILETYPE (syntax_tree) == F_modimp)
        || (MODUL_FILETYPE (syntax_tree) == F_classimp)) {
        sprintf (cccallstr,
                 "gcc %s-Wall -Wno-unused -I $RCSROOT/src/compile/"
                 " -o %s.o -c %s",
                 ccflagsstr, (NULL != module_name) ? module_name : outfilename,
                 cfilename);
    } else {
        DBUG_ASSERT (0, "wrong value of kind_of_file ");
    }
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
NOTE2 (("*** Invoking C-compiler:"));
NOTE2 (("%s", cccallstr));
NEWLINE (2);

system (cccallstr);

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

void
SearchLinkFiles ()
{
    strings *tmp;
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

    DBUG_VOID_RETURN ();
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

    if (0 != strcmp (mod, MODUL_NAME (syntax_tree))) {
        tmp = linklist;

        while ((tmp != NULL) && (strcmp (STRINGS_STRING (tmp), mod) != 0)) {
            tmp = STRINGS_NEXT (tmp);
        }

        if (tmp == NULL) {
            linklist = MakeStrings (mod, linklist);

            DBUG_PRINT ("LINK", ("Added %s to linklist", mod));
        }
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

    if (FUNDEF_LINKMOD (arg_node) != NULL) {
        AddToLinklist (FUNDEF_LINKMOD (arg_node));
    } else {
        if (FUNDEF_MOD (arg_node) != NULL) {
            AddToLinklist (FUNDEF_MOD (arg_node));
        }
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

    if (OBJDEF_LINKMOD (arg_node) != NULL) {
        AddToLinklist (OBJDEF_LINKMOD (arg_node));
    } else {
        if (OBJDEF_MOD (arg_node) != NULL) {
            AddToLinklist (OBJDEF_MOD (arg_node));
        }
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
