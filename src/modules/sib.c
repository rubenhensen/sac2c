/*
 *
 * $Log$
 * Revision 1.2  1995/09/01 07:51:33  cg
 * first working revision.
 * writes implementation of implicit types to SIB-file end checks
 * implementation of explicit types against their declaration
 *
 * Revision 1.1  1995/08/31  08:37:41  cg
 * Initial revision
 *
 *
 */

#include <malloc.h>
#include <string.h>
#include <limits.h>

#include "dbug.h"
#include "tree.h"
#include "free.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"

#include "scnprs.h"
#include "traverse.h"

#undef TYPES /* These macros are defined in scnprs.h as well as   */
#undef ID    /* in access_macros.h. The latter definition is used */
#undef DIM   /* in this file.                                     */

#include "access_macros.h"
#include "compare_macros.h"

#include "filemgr.h"
#include "import.h"
#include "print.h"
#include "convert.h"

#include "sib.h"

FILE *sibfile;

extern char filename[]; /* is set in main.c */

/*
 *
 *  functionname  : WriteSib
 *  arguments     : 1) syntax tree
 *  description   : writes SAC-Information-Blocks by starting the
 *                  traversal mechanism
 *  global vars   : act_tab, sib_tab
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

    act_tab = sib_tab;

    DBUG_RETURN (Trav (arg_node, NULL));
}

/*
 *
 *  functionname  : LoadDeclaration
 *  arguments     : 1) name of module/class
 *                  2) module(==NULL) or class (!=NULL)
 *  description   : scans and parses the respective
 *                  declaration when compiling a module or class
 *                  implementation
 *  global vars   : decl_tree, linenum, start_token, yyin
 *  internal funs : ---
 *  external funs : strcpy, strcat, strcmp, fopen, yyparse,
 *                  InsertClassType
 *  macros        : ERROR2, NOTE
 *
 *  remarks       :
 *
 */

node *
LoadDeclaration (char *name, node *modtype)
{
    char buffer[MAX_FILE_NAME];
    node *decl;

    DBUG_ENTER ("LoadDeclaration");

    strcpy (buffer, name);
    strcat (buffer, ".dec");
    yyin = fopen (FindFile (MODDEC_PATH, buffer), "r");

    if (yyin == NULL) {
        ERROR2 (1, ("ERROR: Unable to open file \"%s\"", buffer));
    }

    NOTE (("\n  Loading %s ...", buffer));

    linenum = 1;
    start_token = PARSE_DEC;
    yyparse ();
    decl = decl_tree;

    if (strcmp (decl->info.fun_name.id, name) != 0) {
        ERROR2 (
          1, ("%s :ERROR: File does not provide module/class %s, but module/class %s!\n",
              buffer, name, decl->info.fun_name.id));
    }

    if ((decl->nodetype == N_classdec) && (modtype == NULL)) {
        ERROR2 (1, ("%s :ERROR: implementation of module but declaration of class",
                    filename, name));
    }

    if ((decl->nodetype == N_moddec) && (modtype != NULL)) {
        ERROR2 (1, ("%s :ERROR: implementation of class but declaration of module",
                    filename, name));
    }

    if (decl->nodetype == N_classdec) {
        InsertClassType (decl);
    }

    DBUG_RETURN (decl);
}

/*
 *
 *  functionname  : OpenSibFile
 *  arguments     : 1) name of module/class
 *  description   : opens the respective SIB-file
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : strcpy, strcat, fopen
 *  macros        : ERROR2
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
        ERROR2 (1, ("ERROR: Unable to open file \"%s\" for writing!\n", buffer));
    }

    DBUG_RETURN (tmp);
}

/*
 *
 *  functionname  : RetrieveImplTypeInfo
 *  arguments     : 1) N_modul node of program or module/class implementation
 *  description   : From previous SIB-Files information is stored about
 *                  the implementation of implicit types in "second"
 *                  types-structure of typedef-nodes. This information is
 *                  now retrieved.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : FREE
 *
 *  remarks       :
 *
 */

node *
RetrieveImplTypeInfo (node *modul)
{
    node *tmp;
    types *tobefreed;

    DBUG_ENTER ("RetrieveImplTypeInfo");

    tmp = modul->node[1];
    while (tmp != NULL) {
        if (tmp->info.types->next != NULL) {
            tobefreed = tmp->info.types;
            tmp->info.types = tmp->info.types->next;
            /* FREE(tobefreed); */

            DBUG_PRINT ("WRITESIB",
                        ("use SIB-info for implicit type %s:%s", tmp->ID_MOD, tmp->ID));
        }

        tmp = tmp->node[0];
    }

    DBUG_RETURN (modul);
}

/*
 *
 *  functionname  : SearchType
 *  arguments     : 1) typedef to be searched
 *                  2) list of type implementations
 *  description   : looks for a certain typedef in list of typedefs
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : CMP_TYPEDEF
 *
 *  remarks       :
 *
 */

node *
SearchType (node *type, node *implementations)
{
    node *tmp;

    DBUG_ENTER ("SearchType");

    tmp = implementations;
    while ((tmp != NULL) && (CMP_TYPEDEF (type, tmp) == 0)) {
        tmp = tmp->node[0];
    }

    DBUG_RETURN (tmp);
}

/*
 *
 *  functionname  : CheckExplicitType
 *  arguments     : 1) declaration of explicit type (N_typedef)
 *                  2) implementation of explicit type (N_typedef)
 *  description   : compares the declaration of an explicit type with its
 *                  implementation
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : access_macros, CMP_TYPE_USER
 *
 *  remarks       : result==1 -> compare positive
 *                  result==0 -> compare negative
 *
 */

int
CheckExplicitType (node *decl, node *impl)
{
    int i, result = 1;

    DBUG_ENTER ("CheckExplicitType");

    if (decl->SIMPLETYPE == impl->SIMPLETYPE) {
        if (decl->SIMPLETYPE == T_user) {
            if (!CMP_TYPE_USER (decl->TYPES, impl->TYPES)) {
                result = 0;
            }
        }
        if ((result == 1) && (decl->DIM == impl->DIM)) {
            for (i = 0; i < decl->DIM; i++) {
                if (decl->SHP[i] != impl->SHP[i]) {
                    result = 0;
                    break;
                }
            }
        } else
            result = 0;
    } else
        result = 0;

    DBUG_RETURN (result);
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
 *  functionname  : HandleImplicitTypes
 *  arguments     : 1) list of implicit type declarations
 *                  2) list of type definitions
 *  description   : for each exported implicit type its definition is
 *                  searched and printed to SIB-file
 *  global vars   : ---
 *  internal funs : SearchType
 *  external funs : fprintf
 *  macros        : ERROR2
 *
 *  remarks       :
 *
 */

void
HandleImplicitTypes (node *declarations, node *implementations)
{
    node *tmp, *def;

    DBUG_ENTER ("HandleImplicitTypes");

    tmp = declarations;
    while (tmp != NULL) {
        def = SearchType (tmp, implementations);
        if (def == NULL) {
            ERROR2 (1, ("%s :ERROR: Implementation of implicit type %s missing", filename,
                        tmp->ID));
        }

        fprintf (sibfile, "typedef %s ", Type2String (def->TYPES, 0));
        fprintf (sibfile, "%s;\n", def->ID);

        tmp = tmp->node[0];

        /*
         * preliminary version
         * Now, the defining type may be user-defined as well and therefore
         * unknown in the importing program. A final version must derive the
         * type completely from primitive types.
         */
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : HandleExplicitTypes
 *  arguments     : 1) list of explicit type declarations
 *                  2) list of type definitions
 *  description   : for each explicit type declaration the respective
 *                  implementation is searched and compared with the
 *                  declaration
 *  global vars   : ---
 *  internal funs : SearchType, CheckExplicitType
 *  external funs : ---
 *  macros        : ERROR2
 *
 *  remarks       :
 *
 */

void
HandleExplicitTypes (node *declarations, node *implementations)
{
    node *tmp, *def;

    DBUG_ENTER ("HandleExplicitTypes");

    tmp = declarations;
    while (tmp != NULL) {
        def = SearchType (tmp, implementations);
        if (def == NULL) {
            WARN1 (("%s :WARNING: implementation of explicit type %s missing", filename,
                    tmp->ID));
        } else {
            if (CheckExplicitType (tmp, def) == 0) {
                ERROR2 (1, ("%s :ERROR: implementation of explicit type %s different "
                            "from declaration",
                            filename, tmp->ID));
            }
        }

        tmp = tmp->node[0];
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : HandleFunctions
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
    node *decl;

    DBUG_ENTER ("SIBmodul");

    arg_node = RetrieveImplTypeInfo (arg_node);

    if (arg_node->info.id != NULL) /* it's a module/class implementation */
    {
        decl = LoadDeclaration (arg_node->info.id, arg_node->node[4]);

        sibfile = OpenSibFile (arg_node->info.id);

        fprintf (sibfile, "<%s>\n\n", arg_node->info.id);

        HandleImplicitTypes (decl->node[0]->node[0], arg_node->node[1]);
        HandleExplicitTypes (decl->node[0]->node[1], arg_node->node[1]);
        /*
            HandleFunctions(decl->node[0]->node[2], arg_node->node[2]);
        */

        fprintf (sibfile, "\n");

        fclose (sibfile);
    }

    DBUG_RETURN (arg_node);
}
