/*
 *
 * $Log$
 * Revision 3.2  2001/03/15 15:24:30  dkr
 * signature of Type2String modified
 *
 * Revision 3.1  2000/11/20 18:00:58  sacbase
 * new release made
 *
 * Revision 2.6  2000/11/14 13:19:24  dkr
 * no '... might be used uninitialized' warnings anymore
 *
 * Revision 2.5  2000/08/07 14:09:29  dkr
 * ST_independent replaced by ST_shp_indep and ST_dim_indep
 *
 * Revision 2.4  2000/02/23 20:24:53  cg
 * SIB format slightly changed in order to distinguish SIBs
 * of modules from SIBs of classes when being read in again.
 *
 * Revision 2.3  2000/02/23 17:27:01  cg
 * The entry TYPES_TDEF of the TYPES data structure now contains a
 * reference to the corresponding N_typedef node for all user-defined
 * types.
 * Therefore, most calls to LookupType() are eliminated.
 * Please, keep the back references up to date!!
 *
 * Revision 2.2  1999/06/08 08:32:42  cg
 * The print phase now always carries an N_info node with it in order
 * to distinguish between different layouts. The distinction between
 * arg_info ==NULL and arg_info !=NULL is no longer used.
 *
 * Revision 2.1  1999/02/23 12:42:15  sacbase
 * new release made
 *
 * Revision 1.19  1999/02/05 16:45:13  dkr
 * Added FreeTree(MODUL_DECL) in WSIBmodul().
 * This node will later be used by MODUL_FOLDFUN !!
 *
 * Revision 1.18  1999/01/07 10:50:49  cg
 * Fold functions are always written to the SIB including there bodies.
 *
 * Revision 1.17  1998/06/05 15:27:49  cg
 * global variable mod_name_con and macros MOD_NAME_CON MOD MOD_NAME MOD_CON removed
 * Now, module name and symbol name are combined correctly by ':'.
 * Only when it really comes to the generation of C code, the ':' is
 * replaced by '__'. This is done by the renaming of all identifiers
 * during the precompilation phase.
 *
 * Revision 1.16  1997/11/12 10:38:24  sbs
 * break in default of switch constructs added (as required by cc)
 *
 * Revision 1.15  1997/10/29 14:38:00  srs
 * free -> FREE
 *
 * Revision 1.14  1997/05/05 11:53:18  cg
 * SIB syntax slightly modified
 *
 * Revision 1.13  1997/04/28  12:00:25  cg
 * SIB syntax slightly changed:
 * key word classtype used instead of Class.
 *
 * Revision 1.12  1997/03/19  13:53:22  cg
 * The global dependency tree is written as special pragma linkwith to the SIB
 *
 * Revision 1.11  1996/09/11  06:26:47  cg
 * SIB is now written directly to build_dirname.
 *
 * Revision 1.10  1996/02/06  18:58:19  cg
 * added special print functions WSIBfloat and WSIBdouble for printing
 * constants followed by F or D to mark their types
 *
 * Revision 1.9  1996/01/26  15:32:21  cg
 * function status ST_classfun now supported
 *
 * Revision 1.8  1996/01/22  18:35:31  cg
 * added new pragmas for global objects: effect, initfun
 *
 * Revision 1.7  1996/01/07  17:01:26  cg
 * pragmas linkname, copyfun, and freefun are now generated
 * for sib from internal information
 *
 * Revision 1.6  1996/01/05  12:41:18  cg
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

#define PRINTMODNAME(mod, name)                                                          \
    if (mod == NULL) {                                                                   \
        fprintf (sibfile, "%s", name);                                                   \
    } else {                                                                             \
        fprintf (sibfile, "%s:%s", mod, name);                                           \
    }

/*
 *
 *  functionname  : WriteSib
 *  arguments     : 1) syntax tree
 *  description   : writes SAC-Information-Blocks by starting the
 *                  traversal mechanism
 *  global vars   : act_tab, writesib_tab
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
 *  functionname  : SIBPrintDependencies
 *  arguments     : 1) output stream to SIB file
 *                  2) dependency tree
 *                  3) recursion level
 *  description   : prints the dependencies to the sibfile
 *  global vars   : dependencies
 *
 */

void
SIBPrintDependencies (FILE *sibfile, deps *depends, int level)
{
    deps *tmp;
    int i;

    DBUG_ENTER ("SIBPrintDependencies");

    if ((dependencies != NULL) && (level == 1)) {
        fprintf (sibfile, "linkwith\n");
    }

    tmp = depends;

    while (tmp != NULL) {
        if (DEPS_STATUS (tmp) == ST_own) {
            tmp = DEPS_NEXT (tmp);
        } else {
            for (i = 0; i < level; i++) {
                fprintf (sibfile, " ");
            }

            switch (DEPS_STATUS (tmp)) {
            case ST_sac:
                fprintf (sibfile, "\"%s\"", DEPS_NAME (tmp));
                break;
            case ST_external:
                fprintf (sibfile, "external \"%s\"", DEPS_NAME (tmp));
                break;
            case ST_system:
                fprintf (sibfile, "linkwith \"%s\"", DEPS_NAME (tmp));
                break;
            default:
                break;
            }

            if (DEPS_SUB (tmp) != NULL) {
                fprintf (sibfile, "\n{\n");
                SIBPrintDependencies (sibfile, DEPS_SUB (tmp), level + 1);
                fprintf (sibfile, "}\n");
            }

            tmp = DEPS_NEXT (tmp);

            if (tmp != NULL) {
                fprintf (sibfile, ",\n");
            }
        }
    }

    fprintf (sibfile, "\n\n");

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : StoreExportNode
 *  arguments     : 1) node which has to be inserted
 *                  2) N_info node where 1) has to be inserted
 *  description   : inserts the given node at the end of the correct
 *                  nodelist (funlist, objlist, or typelist) of the
 *                  given N_info node if it's not already included.
 *
 */

static void
StoreExportNode (node *insert, node *info)
{
    nodelist *act, *last, *list;
    node *obj_tdef;

    DBUG_ENTER ("StoreExportNode");

    DBUG_PRINT ("WSIB", ("Must export '%s` (%s)", ItemName (insert),
                         mdb_nodetype[NODE_TYPE (insert)]));

    switch (NODE_TYPE (insert)) {
    case N_fundef:
        list = INFO_WSIB_EXPORTFUNS (info);
        break;

    case N_objdef:
        list = INFO_WSIB_EXPORTOBJS (info);
        break;

    case N_typedef:
        list = INFO_WSIB_EXPORTTYPES (info);
        break;

    default:
        list = NULL;
        DBUG_ASSERT (0, "Wrong insert node in call to function 'StoreExportNode`");
    }

    if (list == NULL) {
        switch (NODE_TYPE (insert)) {
        case N_fundef:
            INFO_WSIB_EXPORTFUNS (info) = MakeNodelist (insert, ST_regular, NULL);
            break;

        case N_objdef:
            INFO_WSIB_EXPORTOBJS (info) = MakeNodelist (insert, ST_regular, NULL);
            obj_tdef = TYPES_TDEF (OBJDEF_TYPE (insert));
            DBUG_ASSERT ((obj_tdef != NULL), "Failed attempt to look up typedef");

            StoreExportNode (obj_tdef, info);
            break;

        case N_typedef:
            INFO_WSIB_EXPORTTYPES (info) = MakeNodelist (insert, ST_regular, NULL);
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
                obj_tdef = TYPES_TDEF (OBJDEF_TYPE (insert));
                DBUG_ASSERT ((obj_tdef != NULL), "Failed attempt to look up typedef");
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
 *
 */

static void
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
 *  functionname  : WSIBfloat
 *  arguments     : 1) N_float node
 *                  2) arg_info unused
 *  description   : prints float constant to outfile
 *
 *  remarks       : Main difference to PrintFloat from print.c is that the
 *                  constant is always followed by an "F" to mark it as
 *                  a float constant. This is necessary for a later handling
 *                  by the typechecker.
 *
 */

node *
WSIBfloat (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("WSIBfloat");

    fprintf (outfile, "%.256gF", arg_node->info.cfloat);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : WSIBdouble
 *  arguments     : 1) N_dbl node
 *                  2) arg_info unused
 *  description   : prints double constant to outfile
 *
 *  remarks       : Main difference to PrintDouble from print.c is that the
 *                  constant is always followed by a "D" to mark it as
 *                  a double constant. The is necessary for a later handling
 *                  by the typechecker.
 *
 */

node *
WSIBdouble (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("WSIBdouble");

    fprintf (outfile, "%.256gD", arg_node->info.cdbl);

    DBUG_RETURN (arg_node);
}

static node *
AddImplicitItems (node *info, node *all_types)
{
    nodelist *tmp;
    node *fundef, *tdef, *impl_tdef, *objdef;

    DBUG_ENTER ("AddImplicitItems");

    tmp = INFO_WSIB_EXPORTFUNS (info);

    while (tmp != NULL) {
        fundef = NODELIST_NODE (tmp);
        StoreExportNodes (FUNDEF_NEEDOBJS (fundef), info);

        if ((FUNDEF_BODY (fundef) != NULL)
            && ((FUNDEF_INLINE (fundef)) || (FUNDEF_ATTRIB (fundef) == ST_shp_indep)
                || (FUNDEF_ATTRIB (fundef) == ST_dim_indep)
                || (FUNDEF_STATUS (fundef) == ST_foldfun))) {
            StoreExportNodes (FUNDEF_NEEDTYPES (NODELIST_NODE (tmp)), info);
            StoreExportNodes (FUNDEF_NEEDFUNS (NODELIST_NODE (tmp)), info);
        }

        tmp = NODELIST_NEXT (tmp);
    }

    tmp = INFO_WSIB_EXPORTTYPES (info);

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

    tmp = INFO_WSIB_EXPORTOBJS (info);

    while (tmp != NULL) {
        objdef = NODELIST_NODE (tmp);
        StoreExportNodes (OBJDEF_NEEDOBJS (objdef), info);
        StoreExportNodes (FUNDEF_NEEDOBJS (AP_FUNDEF (OBJDEF_EXPR (objdef))), info);

        tmp = NODELIST_NEXT (tmp);
    }

    DBUG_RETURN (info);
}

static void
PrintNeededTypes (FILE *sibfile, nodelist *types, node *arg_info)
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

static void
PrintNeededFunctions (FILE *sibfile, nodelist *funs, node *arg_info)
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
            INFO_PRINT_OMIT_FORMAL_PARAMS (arg_info) = 1;
            Trav (FUNDEF_ARGS (fun), arg_info);
            INFO_PRINT_OMIT_FORMAL_PARAMS (arg_info) = 0;
        }

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

static void
PrintNeededObjects (FILE *sibfile, nodelist *objs, node *arg_info)
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
 *  functionname  : PrintNeededObjectsOfObject
 *  arguments     : 1) file handle for printing
 *                  2) node list of objdefs
 *  description   : writes an effect pragma with the given objects
 *                  to the sib file.
 *
 *  remarks       : The only difference to PrintNeededObjects is that
 *                  this function always writes a pragma effect regardless
 *                  of the node list attribute.
 *
 */

static void
PrintNeededObjectsOfObject (FILE *sibfile, nodelist *objs, node *arg_info)
{
    node *obj;
    int first;
    nodelist *tmp;

    DBUG_ENTER ("PrintNeededObjectsOfObject");

    tmp = objs;
    first = 1;

    while (tmp != NULL) {
        obj = NODELIST_NODE (tmp);

        if (first) {
            first = 0;
            fprintf (sibfile, "#pragma effect ");
        } else {
            fprintf (sibfile, ", ");
        }

        PRINTMODNAME (OBJDEF_MOD (obj), OBJDEF_NAME (obj));

        tmp = NODELIST_NEXT (tmp);
    }

    if (!first) {
        fprintf (sibfile, "\n");
    }

    DBUG_VOID_RETURN;
}

static void
PrintSibTypes (FILE *sibfile, nodelist *tdeflist, char *modname, node *arg_info)
{
    node *tdef;
    nodelist *tmp;
    char *type_str;

    DBUG_ENTER ("PrintSibTypes");

    tmp = tdeflist;

    while (tmp != NULL) {
        tdef = NODELIST_NODE (tmp);

        if (TYPEDEF_ATTRIB (tdef) == ST_unique) {
            fprintf (sibfile, "classtype ");
        }

        if ((TYPEDEF_BASETYPE (tdef) == T_hidden) && (TYPEDEF_TNAME (tdef) == NULL)) {
            fprintf (sibfile, "typedef implicit ");
            PRINTMODNAME (TYPEDEF_MOD (tdef), TYPEDEF_NAME (tdef));
            fprintf (sibfile, ";\n");

            if (TYPEDEF_ATTRIB (tdef) != ST_unique) {
                fprintf (sibfile, "#pragma copyfun \"%s\"\n", TYPEDEF_COPYFUN (tdef));
                fprintf (sibfile, "#pragma freefun \"%s\"\n", TYPEDEF_FREEFUN (tdef));
            }
        } else {
            type_str = Type2String (TYPEDEF_TYPE (tdef), 0, TRUE);
            fprintf (sibfile, "typedef %s ", type_str);
            FREE (type_str);

            PRINTMODNAME (TYPEDEF_MOD (tdef), TYPEDEF_NAME (tdef));
            fprintf (sibfile, ";\n");
        }

        tmp = NODELIST_NEXT (tmp);
    }

    if (tdeflist != NULL) {
        fprintf (sibfile, "\n");
    }

    DBUG_VOID_RETURN;
}

static void
PrintSibObjs (FILE *sibfile, nodelist *objdeflist, char *modname, node *arg_info)
{
    node *objdef;
    nodelist *tmp;
    char *type_str;

    DBUG_ENTER ("PrintSibObjs");

    tmp = objdeflist;

    while (tmp != NULL) {
        objdef = NODELIST_NODE (tmp);

        type_str = Type2String (OBJDEF_TYPE (objdef), 0, TRUE);
        fprintf (sibfile, "objdef %s ", type_str);
        FREE (type_str);

        PRINTMODNAME (OBJDEF_MOD (objdef), OBJDEF_NAME (objdef));
        fprintf (sibfile, ";\n");

        if (OBJDEF_LINKNAME (objdef) != NULL) {
            fprintf (sibfile, "#pragma linkname \"%s\"\n", OBJDEF_LINKNAME (objdef));
        }

        if (OBJDEF_INITFUN (objdef) != NULL) {
            fprintf (sibfile, "#pragma initfun \"%s\"\n", OBJDEF_INITFUN (objdef));
        }

        PrintNeededObjectsOfObject (sibfile, OBJDEF_NEEDOBJS (objdef), arg_info);
        PrintNeededObjectsOfObject (sibfile,
                                    FUNDEF_NEEDOBJS (AP_FUNDEF (OBJDEF_EXPR (objdef))),
                                    arg_info);
        /*
         * The structure of the syntax tree guarantees that at most one of
         * the above function calls actually writes a pragma to sibfile.
         */

#if 0
    if (OBJDEF_LINKMOD(objdef)!=NULL)
    {
      fprintf(sibfile, "#pragma external %s\n", OBJDEF_LINKMOD(objdef));
    }
#endif
        tmp = NODELIST_NEXT (tmp);
    }

    if (objdeflist != NULL) {
        fprintf (sibfile, "\n");
    }

    DBUG_VOID_RETURN;
}

static void
PrintSibFuns (FILE *sibfile, nodelist *fundeflist, char *modname, node *arg_info)
{
    node *fundef;

    DBUG_ENTER ("PrintSibFuns");

    while (fundeflist != NULL) {
        fundef = NODELIST_NODE (fundeflist);
        INFO_PRINT_FUNDEF (arg_info) = fundef;

        if (FUNDEF_STATUS (fundef) == ST_classfun) {
            fprintf (outfile, "classtype ");
        }

        PrintFunctionHeader (fundef, arg_info);

        if ((FUNDEF_INLINE (fundef)) || (FUNDEF_ATTRIB (fundef) == ST_shp_indep)
            || (FUNDEF_ATTRIB (fundef) == ST_dim_indep)
            || (FUNDEF_STATUS (fundef) == ST_foldfun)) {
            fprintf (sibfile, "\n");
            Trav (FUNDEF_BODY (fundef), arg_info);
        } else {
            fprintf (sibfile, ";\n");
        }

        if (FUNDEF_PRAGMA (fundef) != NULL) {
            PrintPragma (FUNDEF_PRAGMA (fundef), arg_info);
        }
#if 0
    if (FUNDEF_LINKMOD(fundef)!=NULL) {
      fprintf(sibfile, "#pragma external %s\n", FUNDEF_LINKMOD(fundef));
    }
#endif
        PrintNeededObjects (sibfile, FUNDEF_NEEDOBJS (fundef), arg_info);

        if ((FUNDEF_INLINE (fundef)) || (FUNDEF_ATTRIB (fundef) == ST_shp_indep)
            || (FUNDEF_ATTRIB (fundef) == ST_dim_indep)
            || (FUNDEF_STATUS (fundef) == ST_foldfun)) {
            PrintNeededTypes (sibfile, FUNDEF_NEEDTYPES (fundef), arg_info);
            PrintNeededFunctions (sibfile, FUNDEF_NEEDFUNS (fundef), arg_info);
        }

        fprintf (sibfile, "\n");
        INFO_PRINT_FUNDEF (arg_info) = NULL;

        fundeflist = NODELIST_NEXT (fundeflist);
    }

    DBUG_VOID_RETURN;
}

node *
WSIBobjdef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WSIBobjdef");

    if (FUNDEF_NEEDOBJS (AP_FUNDEF (OBJDEF_EXPR (OBJDEC_DEF (arg_node))))) {
        StoreExportNode (OBJDEC_DEF (arg_node), arg_info);
    }

    if (OBJDEF_NEXT (arg_node) != NULL) {
        arg_info = Trav (OBJDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_info);
}

/*
 *
 *  functionname  : WSIBfundef
 *
 *  remarks       : arg_info == NULL
 *                  specifies a second traversal of the fundef chain in order to
 *                  remove obsolete function definitions.
 *
 */

node *
WSIBfundef (node *arg_node, node *arg_info)
{
    node *ret;

    DBUG_ENTER ("WSIBfundef");

    if (arg_info != NULL) {
        if ((FUNDEF_INLINE (FUNDEC_DEF (arg_node)))
            || (FUNDEF_ATTRIB (FUNDEC_DEF (arg_node)) == ST_shp_indep)
            || (FUNDEF_ATTRIB (FUNDEC_DEF (arg_node)) == ST_dim_indep)
            || (FUNDEF_NEEDOBJS (FUNDEC_DEF (arg_node)) != NULL)) {
            StoreExportNode (FUNDEC_DEF (arg_node), arg_info);
        }

        if (FUNDEF_NEXT (arg_node) != NULL) {
            ret = Trav (FUNDEF_NEXT (arg_node), arg_info);
        } else {
            ret = arg_info;
        }
    } else {
        /*
         * Here follows the definition of a second traversal of the N_fundef
         * chain. During this traversal, all function definitions with incomplete
         * argument or return value shape specifications will be removed.
         * Additionally, all information about needed functions and types will
         * also be removed.
         */
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
        }

        if (FUNDEF_ATTRIB (arg_node) == ST_shp_indep) {
            DBUG_PRINT ("WSIB",
                        ("Removing shape independent function %s", ItemName (arg_node)));

            ret = FreeNode (arg_node);
        } else if (FUNDEF_ATTRIB (arg_node) == ST_dim_indep) {
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

    if (EXPLIST_OBJS (arg_node) != NULL) {
        arg_info = Trav (EXPLIST_OBJS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_info);
}

node *
WSIBmodul (node *arg_node, node *arg_info)
{
    FILE *sibfile, *store_outfile;

    DBUG_ENTER ("WSIBmodul");

    arg_info = MakeInfo ();
    INFO_PRINT_SIB (arg_info) = 1;

    sibfile = WriteOpen ("%s/%s.sib", tmp_dirname, MODUL_NAME (arg_node));

    if (MODUL_FILETYPE (arg_node) == F_classimp) {
        fprintf (sibfile, "<Class %s>\n\n", MODUL_NAME (arg_node));
    } else {
        fprintf (sibfile, "<Module %s>\n\n", MODUL_NAME (arg_node));
    }

    SIBPrintDependencies (sibfile, dependencies, 1);

    if (MODDEC_OWN (MODUL_DECL (arg_node)) != NULL) {
        /*
         *  First, lists of functions, objects, and types which have to be
         *  printed to the SIB are generated.
         */

        arg_info = Trav (MODDEC_OWN (MODUL_DECL (arg_node)), arg_info);
        arg_info = AddImplicitItems (arg_info, MODUL_TYPES (arg_node));

        /*
         *  Second, the infered functions, objects, and types are printed.
         *  The temporary modification of some global variables allows the
         *  reuse of functions from print.c and convert.c
         */

        store_outfile = outfile;
        outfile = sibfile;

        PrintSibTypes (sibfile, INFO_WSIB_EXPORTTYPES (arg_info), MODUL_NAME (arg_node),
                       arg_info);
        PrintSibObjs (sibfile, INFO_WSIB_EXPORTOBJS (arg_info), MODUL_NAME (arg_node),
                      arg_info);
        PrintSibFuns (sibfile, INFO_WSIB_EXPORTFUNS (arg_info), MODUL_NAME (arg_node),
                      arg_info);

        outfile = store_outfile;

        /*
         *  Third, the lists infered at the beginning are removed again.
         */

        FreeNodelist (INFO_WSIB_EXPORTTYPES (arg_info));
        FreeNodelist (INFO_WSIB_EXPORTOBJS (arg_info));
        FreeNodelist (INFO_WSIB_EXPORTFUNS (arg_info));

        /*
         * MODUL_DECL is no longer needed.
         */
        MODUL_DECL (arg_node) = FreeTree (MODUL_DECL (arg_node));
    }

    fclose (sibfile);

    FreeNode (arg_info);

    /*
     *  Finally, the syntax tree can be tidied up:
     *  Lists of needed types and functions are removed as well as
     *  shape or dimension independent functions.
     */

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), NULL);
    }

    DBUG_RETURN (arg_node);
}
