/*
 *
 * $Log$
 * Revision 1.6  1996/01/05 12:41:18  cg
 * removed function OpenSibFile, WriteOpen from internal_lib.c
 * is used instead.
 *
 * Revision 1.5  1996/01/02  17:49:05  cg
 * Typedefs in SIBs which are again based on user-defined types are now resolved.
 *
 * Revision 1.4  1996/01/02  16:10:01  cg
 * types of implicitly used global objects are now written to the SIB
 *
 * Revision 1.3  1995/12/29  10:43:13  cg
 * All preparations for several link styles removed
 *
 * Revision 1.2  1995/12/23  17:02:41  cg
 * first running version for SIB grammar v0.5
 *
 * Revision 1.1  1995/12/21  16:17:49  cg
 * Initial revision
 *
 * First version after renaming sib.c to writesib.c
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
#include "globals.h"

#include "traverse.h"

#include "convert.h"
#include "filemgr.h"
#include "print.h"
#include "typecheck.h" /* for LookupType  */

#define PRINTMODNAME(mod, name)                                                          \
    if (mod == NULL) {                                                                   \
        fprintf (sibfile, "%s", name);                                                   \
    } else {                                                                             \
        fprintf (sibfile, "%s:%s", mod, name);                                           \
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
WriteSib (node *syntax_tree)
{
    DBUG_ENTER ("WriteSib");

    act_tab = writesib_tab;

    DBUG_RETURN (Trav (syntax_tree, NULL));
}

/*
 *
 *  functionname  : StoreExportNode
 *  arguments     : 1) node which has to be inserted
 *                  2) N_info node where 1) has to be inserted
 *  description   : inserts the given node at the end of the correct
 *                  nodelist (funlist, objlist, or typelist) of the
 *                  given N_info node if it's not already included.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : MakeNodelist
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

void
StoreExportNode (node *insert, node *info)
{
    nodelist *act, *last, *list;
    node *obj_tdef;

    DBUG_ENTER ("StoreExportNode");

    DBUG_PRINT ("WSIB", ("Must export '%s` (%s)", ItemName (insert),
                         mdb_nodetype[NODE_TYPE (insert)]));

    switch (NODE_TYPE (insert)) {
    case N_fundef:
        list = INFO_EXPORTFUNS (info);
        break;

    case N_objdef:
        list = INFO_EXPORTOBJS (info);
        break;

    case N_typedef:
        list = INFO_EXPORTTYPES (info);
        break;

    default:
        DBUG_ASSERT (0, "Wrong insert node in call to function 'StoreExportNode`");
    }

    if (list == NULL) {
        switch (NODE_TYPE (insert)) {
        case N_fundef:
            INFO_EXPORTFUNS (info) = MakeNodelist (insert, ST_regular, NULL);
            break;

        case N_objdef:
            INFO_EXPORTOBJS (info) = MakeNodelist (insert, ST_regular, NULL);
            obj_tdef = LookupType (OBJDEF_TNAME (insert), OBJDEF_TMOD (insert), 042);
            StoreExportNode (obj_tdef, info);
            break;

        case N_typedef:
            INFO_EXPORTTYPES (info) = MakeNodelist (insert, ST_regular, NULL);
            break;

        default:
            DBUG_ASSERT (0, "Wrong insert node in call to function 'StoreExportNode`");
        }
    } else {
        act = list;
        last = list;

        while ((act != NULL) && (NODELIST_NODE (act) != insert)) {
            last = act;
            act = NODELIST_NEXT (act);
        }

        if (act == NULL) {
            NODELIST_NEXT (last) = MakeNodelist (insert, ST_regular, NULL);

            if (NODE_TYPE (insert) == N_objdef) {
                obj_tdef = LookupType (OBJDEF_TNAME (insert), OBJDEF_TMOD (insert), 042);
                StoreExportNode (obj_tdef, info);
            }
        }
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : StoreExportNodes
 *  arguments     : 1) list of nodes
 *                  2) N_info node where inserts are to be done
 *  description   : inserts each node of the nodelist into the correct
 *                  nodelist of the N_info node
 *  global vars   : ---
 *  internal funs : StoreExportNode
 *  external funs : ---
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

void
StoreExportNodes (nodelist *inserts, node *info)
{
    DBUG_ENTER ("StoreExportNodes");

    while (inserts != NULL) {
        StoreExportNode (NODELIST_NODE (inserts), info);
        inserts = NODELIST_NEXT (inserts);
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

/*
 *
 *  functionname  : AddImplicitItems
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
AddImplicitItems (node *info, node *all_types)
{
    nodelist *tmp;
    node *fundef, *tdef, *impl_tdef;

    DBUG_ENTER ("AddImplicitItems");

    tmp = INFO_EXPORTFUNS (info);

    while (tmp != NULL) {
        fundef = NODELIST_NODE (tmp);
        StoreExportNodes (FUNDEF_NEEDOBJS (fundef), info);

        if ((FUNDEF_BODY (fundef) != NULL)
            && (FUNDEF_INLINE (fundef) || (FUNDEF_ATTRIB (fundef) == ST_independent))) {
            StoreExportNodes (FUNDEF_NEEDTYPES (NODELIST_NODE (tmp)), info);
            StoreExportNodes (FUNDEF_NEEDFUNS (NODELIST_NODE (tmp)), info);
        }

        tmp = NODELIST_NEXT (tmp);
    }

    tmp = INFO_EXPORTTYPES (info);

    while (tmp != NULL) {
        tdef = NODELIST_NODE (tmp);

        if (((TYPEDEF_BASETYPE (tdef) == T_user) || (TYPEDEF_BASETYPE (tdef) == T_hidden))
            && (TYPEDEF_TNAME (tdef) != NULL)) {
            impl_tdef
              = SearchTypedef (TYPEDEF_TNAME (tdef), TYPEDEF_TMOD (tdef), all_types);
            StoreExportNode (impl_tdef, info);
        }

        tmp = NODELIST_NEXT (tmp);
    }

    DBUG_RETURN (info);
}

/*
 *
 *  functionname  : PrintNeededTypes
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
PrintNeededTypes (FILE *sibfile, nodelist *types)
{
    node *type;
    nodelist *tmp;

    DBUG_ENTER ("PrintNeededTypes");

    if (types != NULL) {
        fprintf (sibfile, "#pragma types ");
    }

    tmp = types;

    while (tmp != NULL) {
        type = NODELIST_NODE (tmp);

        PRINTMODNAME (TYPEDEF_MOD (type), TYPEDEF_NAME (type));

        if (NODELIST_NEXT (tmp) != NULL) {
            fprintf (sibfile, ", ");
        }

        tmp = NODELIST_NEXT (tmp);
    }

    if (types != NULL) {
        fprintf (sibfile, "\n");
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : PrintNeededFunctions
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
PrintNeededFunctions (FILE *sibfile, nodelist *funs)
{
    node *fun;

    DBUG_ENTER ("PrintNeededFunctions");

    if (funs != NULL) {
        fprintf (sibfile, "#pragma functions ");
    }

    while (funs != NULL) {
        fun = NODELIST_NODE (funs);

        PRINTMODNAME (FUNDEF_MOD (fun), FUNDEF_NAME (fun));

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

        if (NODELIST_NEXT (funs) == NULL) {
            fprintf (sibfile, "\n");
        } else {
            fprintf (sibfile, ",\n                  ");
        }

        funs = NODELIST_NEXT (funs);
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : PrintNeededObjects
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
PrintNeededObjects (FILE *sibfile, nodelist *objs)
{
    node *obj;
    int first;
    nodelist *tmp;

    DBUG_ENTER ("PrintNeededObjects");

    tmp = objs;
    first = 1;

    while (tmp != NULL) {
        if (NODELIST_ATTRIB (tmp) == ST_reference) {
            obj = NODELIST_NODE (tmp);

            if (first) {
                first = 0;
                fprintf (sibfile, "#pragma effect ");
            } else {
                fprintf (sibfile, ", ");
            }

            PRINTMODNAME (OBJDEF_MOD (obj), OBJDEF_NAME (obj));
        }

        tmp = NODELIST_NEXT (tmp);
    }

    if (!first) {
        fprintf (sibfile, "\n");
    }

    tmp = objs;
    first = 1;

    while (tmp != NULL) {
        if (NODELIST_ATTRIB (tmp) == ST_readonly_reference) {
            obj = NODELIST_NODE (tmp);

            if (first) {
                first = 0;
                fprintf (sibfile, "#pragma touch ");
            } else {
                fprintf (sibfile, ", ");
            }

            PRINTMODNAME (OBJDEF_MOD (obj), OBJDEF_NAME (obj));
        }

        tmp = NODELIST_NEXT (tmp);
    }

    if (!first) {
        fprintf (sibfile, "\n");
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
 *  functionname  : PrintSibTypes
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
PrintSibTypes (FILE *sibfile, nodelist *tdeflist, char *modname)
{
    node *tdef;
    nodelist *tmp;

    DBUG_ENTER ("PrintSibTypes");

    tmp = tdeflist;

    while (tmp != NULL) {
        tdef = NODELIST_NODE (tmp);

        if (TYPEDEF_ATTRIB (tdef) == ST_unique) {
            fprintf (sibfile, "class ");
        }

        if ((TYPEDEF_BASETYPE (tdef) == T_hidden) && (TYPEDEF_TNAME (tdef) == NULL)) {
            fprintf (sibfile, "typedef implicit ");
        } else {
            fprintf (sibfile, "typedef %s ", Type2String (TYPEDEF_TYPE (tdef), 0));
        }

        PRINTMODNAME (TYPEDEF_MOD (tdef), TYPEDEF_NAME (tdef));
        fprintf (sibfile, ";\n");

        if (TYPEDEF_PRAGMA (tdef) != NULL) {
            PrintPragma (TYPEDEF_PRAGMA (tdef), tdef);
        }

        tmp = NODELIST_NEXT (tmp);
    }

    if (tdeflist != NULL) {
        fprintf (sibfile, "\n");
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : PrintSibObjs
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
PrintSibObjs (FILE *sibfile, nodelist *objdeflist, char *modname)
{
    node *objdef;
    nodelist *tmp;

    DBUG_ENTER ("PrintSibObjs");

    tmp = objdeflist;

    while (tmp != NULL) {
        objdef = NODELIST_NODE (tmp);

        fprintf (sibfile, "objdef %s ", Type2String (OBJDEF_TYPE (objdef), 0));
        PRINTMODNAME (OBJDEF_MOD (objdef), OBJDEF_NAME (objdef));
        fprintf (sibfile, ";\n");

        if (OBJDEF_PRAGMA (objdef) != NULL) {
            PrintPragma (OBJDEF_PRAGMA (objdef), objdef);
        }
        /*
            if (OBJDEF_LINKMOD(objdef)!=NULL)
            {
              fprintf(sibfile, "#pragma external %s\n", OBJDEF_LINKMOD(objdef));
            }
        */
        tmp = NODELIST_NEXT (tmp);
    }

    if (objdeflist != NULL) {
        fprintf (sibfile, "\n");
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : PrintSibFuns
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
PrintSibFuns (FILE *sibfile, nodelist *fundeflist, char *modname)
{
    node *fundef;

    DBUG_ENTER ("PrintSibFuns");

    while (fundeflist != NULL) {
        fundef = NODELIST_NODE (fundeflist);

        PrintFunctionHeader (fundef, fundef);

        if ((FUNDEF_INLINE (fundef)) || (FUNDEF_ATTRIB (fundef) == ST_independent)) {
            fprintf (sibfile, "\n");
            Trav (FUNDEF_BODY (fundef), NULL);
        } else {
            fprintf (sibfile, ";\n");
        }

        if (FUNDEF_PRAGMA (fundef) != NULL) {
            PrintPragma (FUNDEF_PRAGMA (fundef), fundef);
        }
        /*
            if (FUNDEF_LINKMOD(fundef)!=NULL)
            {
              fprintf(sibfile, "#pragma external %s\n", FUNDEF_LINKMOD(fundef));
            }
        */
        PrintNeededObjects (sibfile, FUNDEF_NEEDOBJS (fundef));

        if ((FUNDEF_INLINE (fundef)) || (FUNDEF_ATTRIB (fundef) == ST_independent)) {
            PrintNeededTypes (sibfile, FUNDEF_NEEDTYPES (fundef));
            PrintNeededFunctions (sibfile, FUNDEF_NEEDFUNS (fundef));
        }

        fprintf (sibfile, "\n");
        fundeflist = NODELIST_NEXT (fundeflist);
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : WSIBfundef
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
WSIBfundef (node *arg_node, node *arg_info)
{
    node *ret;

    DBUG_ENTER ("WSIBfundef");

    if (arg_info != NULL) {
        if (FUNDEF_INLINE (FUNDEC_DEF (arg_node))
            || (FUNDEF_ATTRIB (FUNDEC_DEF (arg_node)) == ST_independent)
            || (FUNDEF_NEEDOBJS (FUNDEC_DEF (arg_node)) != NULL)) {
            StoreExportNode (FUNDEC_DEF (arg_node), arg_info);
        }

        if (FUNDEF_NEXT (arg_node) != NULL) {
            ret = Trav (FUNDEF_NEXT (arg_node), arg_info);
        } else {
            ret = arg_info;
        }
    } else {
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
        }

        if (FUNDEF_ATTRIB (arg_node) == ST_independent) {
            DBUG_PRINT ("WSIB", ("Removing dimension independent function %s",
                                 ItemName (arg_node)));

            ret = FreeNode (arg_node);
        } else {
            if (FUNDEF_BODY (arg_node) != NULL) {
                DBUG_PRINT ("WSIB", ("Removing list of functions needed by %s",
                                     ItemName (arg_node)));

                FUNDEF_NEEDFUNS (arg_node) = FreeNodelist (FUNDEF_NEEDFUNS (arg_node));

                DBUG_PRINT ("WSIB",
                            ("Removing list of types needed by %s", ItemName (arg_node)));

                FUNDEF_NEEDTYPES (arg_node) = FreeNodelist (FUNDEF_NEEDTYPES (arg_node));
            }

            ret = arg_node;
        }
    }

    DBUG_RETURN (ret);
}

/*
 *
 *  functionname  : WSIBtypedef
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
WSIBtypedef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WSIBtypedef");

    StoreExportNode (TYPEDEC_DEF (arg_node), arg_info);

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        arg_info = Trav (TYPEDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_info);
}

/*
 *
 *  functionname  : WSIBexplist
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
WSIBexplist (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WSIBexplist");

    if (EXPLIST_ITYPES (arg_node) != NULL) {
        arg_info = Trav (EXPLIST_ITYPES (arg_node), arg_info);
    }

    if (EXPLIST_FUNS (arg_node) != NULL) {
        arg_info = Trav (EXPLIST_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_info);
}

/*
 *
 *  functionname  : WSIBmodul
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
WSIBmodul (node *arg_node, node *arg_info)
{
    node *export;
    FILE *sibfile, *store_outfile;

    DBUG_ENTER ("WSIBmodul");

    sibfile = WriteOpen ("%s%s.sib", store_dirname, MODUL_NAME (arg_node));

    fprintf (sibfile, "<%s>\n\n", MODUL_NAME (arg_node));

    if (MODDEC_OWN (MODUL_DECL (arg_node)) != NULL) {
        /*
         *  First, lists of functions, objects, and types which have to be
         *  printed to the SIB are generated.
         */

        export = MakeInfo ();
        export = Trav (MODDEC_OWN (MODUL_DECL (arg_node)), export);
        export = AddImplicitItems (export, MODUL_TYPES (arg_node));

        /*
         *  Second, the infered functions, objects, and types are printed.
         *  The temporary modification of some global variables allows the
         *  reuse of functions from print.c and convert.c
         */

        store_outfile = outfile;
        outfile = sibfile;
        mod_name_con = mod_name_con_2;

        PrintSibTypes (sibfile, INFO_EXPORTTYPES (export), MODUL_NAME (arg_node));
        PrintSibObjs (sibfile, INFO_EXPORTOBJS (export), MODUL_NAME (arg_node));
        PrintSibFuns (sibfile, INFO_EXPORTFUNS (export), MODUL_NAME (arg_node));

        mod_name_con = mod_name_con_1;
        outfile = store_outfile;

        /*
         *  Third, the lists infered at the beginning are removed again.
         */

        FreeNodelist (INFO_EXPORTTYPES (export));
        FreeNodelist (INFO_EXPORTOBJS (export));
        FreeNodelist (INFO_EXPORTFUNS (export));
        FreeNode (export);
    }

    fprintf (sibfile, "\n<###>");

    fclose (sibfile);

    /*
     *  Finally, the syntax tree can be tidied up:
     *  Lists of needed types and functions are removed as well as
     *  dimension independent functions.
     */

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), NULL);
    }

    DBUG_RETURN (arg_node);
}
