/*
 *
 * $Log$
 * Revision 1.5  1995/10/22 17:38:26  cg
 * Totally modified revision:
 * A lot of code moved to checkdec.c
 * Making a new start with writing SIBs in the context of new
 * compiler modules such as analysis or checkdec.
 *
 * Revision 1.4  1995/10/12  13:57:03  cg
 * now imported items cannot be exported again (->Error message)
 *
 * Revision 1.3  1995/10/05  16:05:39  cg
 * implicit type resolution completely renewed.
 * and afterwards extracted to new file implicittype.c
 *
 * Revision 1.2  1995/09/01  07:51:33  cg
 * first working revision.
 * writes implementation of implicit types to SIB-file end checks
 * implementation of explicit types against their declaration
 *
 * Revision 1.1  1995/08/31  08:37:41  cg
 * Initial revision
 *
 *
 */

#include <string.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"

#include "free.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"

#include "traverse.h"

#include "convert.h"
#include "filemgr.h"

#include "sib.h"

static FILE *sibfile;

/*
 *
 *  functionname  : WriteSib
 *  arguments     : 1) syntax tree
 *  description   : writes SAC-Information-Blocks by starting the
 *                  traversal mechanism
 *  global vars   : act_tab, writesib_tab
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : ---
 *
 *  remarks       :
 *
 */

node *
WriteSib (node *arg_node)
{
    DBUG_ENTER ("WriteSib");

    act_tab = writesib_tab;

    DBUG_RETURN (Trav (arg_node, NULL));
}

/*
 *
 *  functionname  : OpenSibFile
 *  arguments     : 1) name of module/class
 *  description   : opens the respective SIB-file
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : strcpy, strcat, fopen
 *  macros        : ERROR, DBUG
 *
 *  remarks       :
 *
 */

FILE *
OpenSibFile (char *name)
{
    char buffer[MAX_FILE_NAME];
    FILE *tmp;

    DBUG_ENTER ("OpenSibFile");

    strcpy (buffer, name);
    strcat (buffer, ".sib");
    tmp = fopen (buffer, "w");

    if (tmp == NULL) {
        SYSABORT (("Unable to open file \"%s\" for writing!\n", buffer));
    }

    DBUG_RETURN (tmp);
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
 *  functionname  : SIBtypedef
 *  arguments     : 1)
 *                  2)
 *  description   :
 *
 *  global vars   : sibfile
 *  internal funs : ---
 *  external funs : fprintf, Type2String, Trav
 *  macros        : TREE, DBUG
 *
 *  remarks       :
 *
 */

node *
SIBtypedef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SIBtypedef");

    fprintf (sibfile, "typedef %s ", Type2String (TYPEDEC_TYPE (arg_node), 0));
    fprintf (sibfile, "%s;\n", TYPEDEF_NAME (arg_node));

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        Trav (TYPEDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : SIBmodul
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
SIBmodul (node *arg_node, node *arg_info)
{
    node *export;

    DBUG_ENTER ("SIBmodul");

    export = MODDEC_OWN (MODUL_DECL (arg_node));
    sibfile = OpenSibFile (MODUL_NAME (arg_node));

    fprintf (sibfile, "<%s>\n\n", MODUL_NAME (arg_node));

    if (EXPLIST_ITYPES (export) != NULL) {
        Trav (EXPLIST_ITYPES (export), NULL);
    }
    /*
      if (EXPLIST_FUNS(export)!=NULL)
      {
        Trav(EXPLIST_FUNS(export), NULL);
      }
    */
    fprintf (sibfile, "\n");

    fclose (sibfile);

    DBUG_RETURN (arg_node);
}
