/*
 *
 * $Log$
 * Revision 1.1  1995/10/22 14:23:26  cg
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
#include "my_debug.h"
#include "traverse.h"

#include "import.h"

/*
 *
 *  functionname  : LoadDeclaration
 *  arguments     : 1) name of module/class
 *                  2) file type, module or class ?
 *  description   : scans and parses the respective
 *                  declaration when compiling a module or class
 *                  implementation
 *  global vars   : decl_tree, linenum, start_token, yyin
 *  internal funs : ---
 *  external funs : strcmp, fopen, fclose, yyparse,
 *                  InsertClassType, FindFile
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
LoadDeclaration (char *name, file_type modtype)
{
    node *decl;

    DBUG_ENTER ("LoadDeclaration");

    yyin = fopen (FindFile (MODDEC_PATH, filename), "r");

    if (yyin == NULL) {
        SYSABORT (("Unable to open file \"%s\"", filename));
    }

    NOTE (("  Loading declaration file \"%s\" ...", filename));

    linenum = 1;
    start_token = PARSE_DEC;
    yyparse ();
    fclose (yyin);

    decl = decl_tree;

    if ((strcmp (MODDEC_NAME (decl), name) != 0)
        || ((NODE_TYPE (decl) == N_classdec) && (modtype == F_modimp))
        || ((NODE_TYPE (decl) == N_moddec) && (modtype == F_classimp))) {
        SYSERROR (("File \"%s\" provides wrong declaration", filename));

        if (modtype == F_modimp) {
            NOTE (("\tRequired: ModuleDec %s", name));
        } else {
            NOTE (("\tRequired: ClassDec %s", name));
        }

        if (NODE_TYPE (decl) == N_moddec) {
            NOTE (("\tProvided: ModuleDec %s", MODDEC_NAME (decl)));
        } else {
            NOTE (("\tProvided: ClassDec %s", MODDEC_NAME (decl)));
        }

        ABORT_ON_ERROR;
    }

    if (NODE_TYPE (decl) == N_classdec) {
        InsertClassType (decl);
    }

    DBUG_RETURN (decl);
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
 *  macros        : CMP_TYPE_USER
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

    if (TYPEDEF_BASETYPE (decl) == TYPEDEF_BASETYPE (impl)) {
        if (TYPEDEF_BASETYPE (decl) == T_user) {
            if (!CMP_TYPE_USER (TYPEDEF_TYPE (decl), TYPEDEF_TYPE (impl))) {
                result = 0;
            }
        }
        if ((result == 1) && (TYPEDEF_DIM (decl) == TYPEDEF_DIM (impl))) {
            for (i = 0; i < TYPEDEF_DIM (decl); i++) {
                if (TYPEDEF_SHAPE (decl, i) != TYPEDEF_SHAPE (impl, i)) {
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
 *  functionname  : CheckDec
 *  arguments     : 1) syntax tree
 *  description   : When compiling a module/class implementation, the
 *                  respective declaration file is parsed and its contents
 *                  checked against the implementation.
 *                  The declaration is stored in the syntax tree as son
 *                  of N_modul node.
 *                  References to defines of functions and implicit types
 *                  are stored in the repsective declaration nodes.
 *                  These are used in writesib.
 *  global vars   : act_tab, checkdec_tab
 *  internal funs : LoadDeclaration
 *  external funs : strcpy, strcat, Trav
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
CheckDec (node *syntax_tree)
{
    char store_filename[MAX_FILE_NAME];

    DBUG_ENTER ("CheckDec");

    strcpy (store_filename, filename);
    strcpy (filename, MODUL_NAME (syntax_tree));
    strcat (filename, ".dec");

    MODUL_DECL (syntax_tree)
      = LoadDeclaration (MODUL_NAME (syntax_tree), MODUL_FILETYPE (syntax_tree));

    act_tab = checkdec_tab;

    MODUL_DECL (syntax_tree) = Trav (MODUL_DECL (syntax_tree), syntax_tree);

    strcpy (filename, store_filename);

    DBUG_RETURN (syntax_tree);
}

/*
 *
 *  functionname  : CDECmoddec
 *  arguments     : 1) moddec or classdec node
 *                  2) pointer to N_modul node of module/class implementation
 *  description   : traverses the export list of a module/class declaration
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Trav
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
CDECmoddec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CDECmoddec");

    MODDEC_OWN (arg_node) = Trav (MODDEC_OWN (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CDECexplist
 *  arguments     : 1) explist node of module/class declaration
 *                  2) N_modul node of module/class implementation
 *  description   : Traverses the export declarations of implicit types,
 *                  explicit types, global objects and functions
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : DBUG, ERROR
 *
 *  remarks       :
 *
 */

node *
CDECexplist (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CDECexplist");

    if (EXPLIST_ITYPES (arg_node) != NULL)
        EXPLIST_ITYPES (arg_node)
          = Trav (EXPLIST_ITYPES (arg_node), MODUL_TYPES (arg_info));

    if (EXPLIST_ETYPES (arg_node) != NULL)
        EXPLIST_ETYPES (arg_node)
          = Trav (EXPLIST_ETYPES (arg_node), MODUL_TYPES (arg_info));

    if (EXPLIST_OBJS (arg_node) != NULL)
        EXPLIST_OBJS (arg_node) = Trav (EXPLIST_OBJS (arg_node), MODUL_OBJS (arg_info));

    if (EXPLIST_FUNS (arg_node) != NULL)
        EXPLIST_FUNS (arg_node) = Trav (EXPLIST_FUNS (arg_node), MODUL_FUNS (arg_info));

    ABORT_ON_ERROR;

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CDECtypedef
 *  arguments     : 1) type declaration (implicit or explicit)
 *                  2) entry point to type definitions of implementation
 *  description   : checks type declaration against type definitions
 *  global vars   : ---
 *  internal funs : CheckExplicitType
 *  external funs : SearchTypedef, Trav
 *  macros        : DBUG, ERROR, TREE
 *
 *  remarks       :
 *
 */

node *
CDECtypedef (node *arg_node, node *arg_info)
{
    node *tdef;

    DBUG_ENTER ("CDECtypedef");

    if (TYPEDEF_BASETYPE (arg_node) == T_hidden) {
        tdef = SearchTypedef (TYPEDEF_NAME (arg_node), TYPEDEF_MOD (arg_node), arg_info);
        if (tdef == NULL) {
            ERROR (NODE_LINE (arg_node), ("Implementation of implicit type '%s` missing",
                                          TYPEDEF_NAME (arg_node)));
        } else {
            if (TYPEDEF_STATUS (tdef) == ST_imported) {
                ERROR (NODE_LINE (arg_node),
                       ("Implementation of implicit type '%s` missing",
                        TYPEDEF_NAME (arg_node)));
            }
        }

        TYPEDEC_DEF (arg_node) = tdef;
    } else {
        tdef = SearchTypedef (TYPEDEF_NAME (arg_node), TYPEDEF_MOD (arg_node), arg_info);

        if (tdef == NULL) {
            WARN (NODE_LINE (arg_node), ("Implementation of explicit type '%s` missing",
                                         TYPEDEF_NAME (arg_node)));
        } else {
            if (TYPEDEF_STATUS (tdef) == ST_imported) {
                ERROR (NODE_LINE (arg_node),
                       ("Implementation of explicit type '%s` missing",
                        TYPEDEF_NAME (arg_node)));
            } else if (CheckExplicitType (arg_node, tdef) == 0) {
                ERROR (NODE_LINE (arg_node), ("Implementation of explicit type '%s`\n\t"
                                              "different from declaration",
                                              TYPEDEF_NAME (arg_node)));
            }
        }
    }

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        TYPEDEF_NEXT (arg_node) = Trav (TYPEDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CDECobjdef
 *  arguments     : 1) declaration of global object
 *                  2) entry point to object definitions of implementation
 *  description   : checks object declaration against object definitions
 *  global vars   : ---
 *  internal funs :
 *  external funs : SearchObjdef, Trav
 *  macros        : DBUG, ERROR, CMP_TYPE_USER
 *
 *  remarks       :
 *
 */

node *
CDECobjdef (node *arg_node, node *arg_info)
{
    node *odef;

    DBUG_ENTER ("CDECobjdef");

    odef = SearchObjdef (OBJDEF_NAME (arg_node), OBJDEF_MOD (arg_node), arg_info);

    if (odef == NULL) {
        ERROR (NODE_LINE (arg_node),
               ("Implementation of global object '%s` missing", OBJDEF_NAME (arg_node)));
    } else {
        if (TYPEDEF_STATUS (odef) == ST_imported) {
            ERROR (NODE_LINE (arg_node), ("Implementation of global object '%s` missing",
                                          OBJDEF_NAME (arg_node)));
        } else if (CMP_TYPE_USER (OBJDEF_TYPE (arg_node), OBJDEF_TYPE (tdef)) == 0) {
            ERROR (NODE_LINE (arg_node),
                   ("Type mismatch in declaration of global object '%s`" OBJDEF_NAME (
                     arg_node)));
        }
    }
}

if (OBJDEF_NEXT (arg_node) != NULL) {
    OBJDEF_NEXT (arg_node) = Trav (OBJDEF_NEXT (arg_node), arg_info);
}

DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CDECfundef
 *  arguments     : 1) function declaration
 *                  2) entry point to function definitions of implementation
 *  description   : checks funtion declaration against function definitions
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : SearchFundef, Trav
 *  macros        : DBUG, ERROR, TREE
 *
 *  remarks       :
 *
 */

node *
CDECfundef (node *arg_node, node *arg_info)
{
    node *fundef;

    DBUG_ENTER ("CDECfundef");

    fundef = SearchFundef (arg_node, arg_info);

    if (fundef == NULL) {
        ERROR (NODE_LINE (arg_node), ("Implementation of function '%s` missing\n\t"
                                      "or type mismatch in arguments",
                                      FUNDEF_NAME (arg_node)));
    } else {
        if (FUNDEF_STATUS (fundef) == ST_imported) {
            ERROR (NODE_LINE (arg_node), ("Implementation of function '%s` missing\n\t"
                                          "or type mismatch in arguments",
                                          FUNDEF_NAME (arg_node)));
        }
    }

    FUNDEC_DEF (arg_node) = fundef;

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
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
