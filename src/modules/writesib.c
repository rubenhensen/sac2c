/*
 *
 * $Log$
 * Revision 1.1  1995/12/21 16:17:49  cg
 * Initial revision
 *
 * Revision 1.11  1995/12/21  15:06:04  cg
 * bug fixed in printing of arguments.
 *
 * Revision 1.10  1995/12/01  17:21:06  cg
 * bug in freeing nodelists fixed
 * storage of alternative link names converted to new pragma node
 *
 * Revision 1.9  1995/11/10  15:05:04  cg
 * converted to new error macros
 *
 * Revision 1.8  1995/11/01  09:34:26  cg
 * Bug fixed in SibPrintFunctions.
 * PrintArg now called with NULL as second argument instead of N_info
 * node as usual when used for printing the syntax tree.
 *
 * Revision 1.7  1995/10/31  09:02:21  cg
 * Parameters of implicitly needed functions are now printed without
 * identifier name.
 *
 * Revision 1.6  1995/10/30  10:22:01  cg
 * now, SIB information about functions (inline info and needed
 * global objects) is printed to SIB-file.
 *
 * Revision 1.5  1995/10/22  17:38:26  cg
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
#include <malloc.h>

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

extern FILE *outfile; /* is set in main.c */

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
    FILE *tmp;

    DBUG_ENTER ("OpenSibFile");

    strcpy (sibfilename, name);
    strcat (sibfilename, ".sib");
    tmp = fopen (sibfilename, "w");

    if (tmp == NULL) {
        SYSABORT (("Unable to open file \"%s\" for writing", sibfilename));
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
 *  functionname  : SibPrintTypes
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs : free
 *  macros        :
 *
 *  remarks       :
 *
 */

void
SibPrintTypes (nodelist *types)
{
    node *type;

    DBUG_ENTER ("SibPrintTypes");

    while (types != NULL) {
        type = NODELIST_NODE (types);

        if (TYPEDEF_ATTRIB (type) == ST_unique) {
            fprintf (sibfile, "class ");
        }

        if (TYPEDEF_MOD (type) != NULL) {
            fprintf (sibfile, "%s:", TYPEDEF_MOD (type));
        }

        fprintf (sibfile, "%s=%s;\n", TYPEDEF_NAME (type),
                 Type2String (TYPEDEF_TYPE (type), 0));

        types = NODELIST_NEXT (types);
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : SibPrintFunctions
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
SibPrintFunctions (nodelist *funs)
{
    node *fun, *obj;
    nodelist *objs;

    DBUG_ENTER ("SibPrintFunctions");

    while (funs != NULL) {
        fun = NODELIST_NODE (funs);

        fprintf (sibfile, "%s ", Type2String (FUNDEF_TYPES (fun), 0));

        if (FUNDEF_MOD (fun) != NULL) {
            fprintf (sibfile, "%s:", FUNDEF_MOD (fun));
        }

        fprintf (sibfile, "%s ", FUNDEF_NAME (fun));

        if ((FUNDEF_PRAGMA (fun) != NULL) && (FUNDEF_LINKNAME (fun) != NULL)) {
            fprintf (sibfile, "{%s} ", FUNDEF_LINKNAME (fun));
        }

        fprintf (sibfile, "(");

        if (FUNDEF_ARGS (fun) != NULL) {
            Trav (FUNDEF_ARGS (fun), NULL);
        }

        /*
         *  The second argument in the above start of the traversal mechanism
         *  causes PrintArg to omit the name of the formal parameter.
         *  Normally, when called by the Print function, PrintArg gets an
         *  N_info node as second argument which is generated by PrintFundef.
         */

        fprintf (sibfile, ")");

        objs = FUNDEF_NEEDOBJS (fun);
        while (objs != NULL) {
            obj = NODELIST_NODE (objs);

            fprintf (sibfile, "\n  %s ", Type2String (OBJDEF_TYPE (obj), 0));
            fprintf (sibfile, "& ");

            if (OBJDEF_MOD (obj) != NULL) {
                fprintf (sibfile, "%s:", OBJDEF_MOD (obj));
            }

            fprintf (sibfile, "%s", OBJDEF_NAME (obj));

            objs = NODELIST_NEXT (objs);

            if (objs != NULL) {
                fprintf (sibfile, ",");
            }
        }

        fprintf (sibfile, ";\n");

        funs = NODELIST_NEXT (funs);
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : SibPrintObjects
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
SibPrintObjects (nodelist *objs)
{
    node *obj;

    DBUG_ENTER ("SibPrintObjects");

    while (objs != NULL) {
        obj = NODELIST_NODE (objs);

        fprintf (sibfile, "%s ", Type2String (OBJDEF_TYPE (obj), 0));
        fprintf (sibfile, "& ");
        if (OBJDEF_MOD (obj) != NULL) {
            fprintf (sibfile, "%s:", OBJDEF_MOD (obj));
        }

        fprintf (sibfile, "%s;\n", OBJDEF_NAME (obj));

        objs = NODELIST_NEXT (objs);
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : SIBfundef
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
SIBfundef (node *arg_node, node *arg_info)
{
    FILE *store_outfile;
    node *fundef;

    DBUG_ENTER ("SIBfundef");

    store_outfile = outfile;
    outfile = sibfile;
    mod_name_con = mod_name_con_2;

    fundef = FUNDEC_DEF (arg_node);

    if (FUNDEF_INLINE (fundef) || (FUNDEF_NEEDOBJS (fundef) != NULL)) {
        fprintf (sibfile, "\n");

        fprintf (sibfile, "%s(", FUNDEF_NAME (fundef));

        if (FUNDEF_ARGS (fundef) != NULL) {
            Trav (FUNDEF_ARGS (fundef), fundef);
        }

        fprintf (sibfile, ")");

        if (FUNDEF_INLINE (fundef)) {
            fprintf (sibfile, "\n");
            Trav (FUNDEF_BODY (fundef), arg_info);
        } else {
            fprintf (sibfile, ";\n");
        }

        fprintf (sibfile, "implicit:\n{\n");

        if (FUNDEF_NEEDTYPES (fundef) != NULL) {
            fprintf (sibfile, "types:\n");
            SibPrintTypes (FUNDEF_NEEDTYPES (fundef));
        }

        if (FUNDEF_NEEDOBJS (fundef) != NULL) {
            fprintf (sibfile, "objects:\n");
            SibPrintObjects (FUNDEF_NEEDOBJS (fundef));
        }

        if (FUNDEF_NEEDFUNS (fundef) != NULL) {
            fprintf (sibfile, "functions:\n");
            SibPrintFunctions (FUNDEF_NEEDFUNS (fundef));
        }

        fprintf (sibfile, "}\n");
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    mod_name_con = mod_name_con_1;
    outfile = store_outfile;

    DBUG_RETURN (arg_node);
}

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

    if (EXPLIST_FUNS (export) != NULL) {
        Trav (EXPLIST_FUNS (export), NULL);
    }

    fprintf (sibfile, "\n");

    fclose (sibfile);

    DBUG_RETURN (arg_node);
}
