/*
 *
 * $Log$
 * Revision 1.6  1995/11/06 14:18:35  cg
 * bug fixed in generating correct references of identifiers
 * to their respective vardec or arg nodes
 *
 * Revision 1.5  1995/10/26  16:15:26  cg
 * Loading of declaration file moved to function ImportOwnDeclaration
 * from import.c
 * Many minor bugs fixed.
 *
 * Revision 1.4  1995/10/24  13:15:02  cg
 * Some errors corrected, first working revision.
 *
 * Revision 1.3  1995/10/22  17:37:59  cg
 * first compilable revision
 *
 * Revision 1.2  1995/10/22  15:56:31  cg
 * Now, declaration files will be generated automatically if not
 * present at compile time of module/class implementation.
 *
 * Revision 1.1  1995/10/22  14:23:26  cg
 * Initial revision
 *
 *
 *
 */

#include <string.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"

#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"
#include "free.h"

#include "import.h"
#include "convert.h"
#include "filemgr.h"
#include "scnprs.h"
#include "implicittypes.h"
#include "print.h"

extern FILE *outfile; /* set in main.c */

static FILE *decfile;

/*
 *
 *  functionname  : CheckTypes
 *  arguments     : 1) first type
 *                  2) second type
 *  description   : compares 2 types
 *
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : DBUG, TREE, ERROR, CMP_TYPE_USER
 *
 *  remarks       : result==1 -> compare positive
 *                  result==0 -> compare negative
 *
 */

int
CheckTypes (types *decl, types *impl)
{
    int i, result = 1;

    DBUG_ENTER ("CheckTypes");

    if (TYPES_BASETYPE (decl) == TYPES_BASETYPE (impl)) {
        if (TYPES_BASETYPE (decl) == T_user) {
            result = CMP_TYPE_USER (decl, impl);
        }
        if (result && (TYPES_DIM (decl) == TYPES_DIM (impl))) {
            for (i = 0; i < TYPES_DIM (decl); i++) {
                if (TYPES_SHAPE (decl, i) != TYPES_SHAPE (impl, i)) {
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
 *  functionname  : PrintDecTypes
 *  arguments     : 1) type to be printed
 *                  2) name of module for which a default declaration
 *                     file is generated.
 *  description   : The given types are printed to 'decfile`, but all
 *                  module names which are identical to the given one are
 *                  omitted.
 *  global vars   : decfile
 *  internal funs : ---
 *  external funs : Type2String
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

void
PrintDecTypes (types *type, char *modname)
{
    DBUG_ENTER ("PrintDecTypes");

    do {
        if (strcmp (MOD (TYPES_MOD (type)), modname) == 0) {
            fprintf (decfile, "%s", Type2String (type, 3));
        } else {
            fprintf (decfile, "%s", Type2String (type, 0));
        }

        if (TYPES_NEXT (type) != NULL) {
            fprintf (decfile, ", ");
        }

        type = TYPES_NEXT (type);
    } while (type != NULL);

    DBUG_VOID_RETURN;
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
 *  global vars   : act_tab, checkdec_tab, writedec_tab
 *  internal funs : LoadDeclaration
 *  external funs : strcpy, strcat, Trav
 *  macros        : DBUG, TREE, ERROR
 *
 *  remarks       :
 *
 */

node *
CheckDec (node *syntax_tree)
{
    char store_filename[MAX_FILE_NAME];
    node *decl;

    DBUG_ENTER ("CheckDec");

    strcpy (store_filename, filename);
    strcpy (filename, MODUL_NAME (syntax_tree));
    strcat (filename, ".dec");

    decl = ImportOwnDeclaration (MODUL_NAME (syntax_tree), MODUL_FILETYPE (syntax_tree));

    if (decl == NULL) {
        SYSWARN (("Unable to open file \"%s\"", filename));

        if (MODUL_FILETYPE (syntax_tree) == F_modimp) {
            SYSWARN (("Declaration of module '%s` missing", MODUL_NAME (syntax_tree)));
        } else {
            SYSWARN (("Declaration of class '%s` missing", MODUL_NAME (syntax_tree)));
        }

        NOTE (("\n  Generating default declaration file \"%s\" ...", filename));
        SYSWARN (("File \"%s\" should be edited", filename));
        NOTE ((""));

        act_tab = writedec_tab;

        mod_name_con = mod_name_con_2;

        Trav (syntax_tree, NULL);

        mod_name_con = mod_name_con_1;

        ABORT_ON_ERROR;

        decl
          = ImportOwnDeclaration (MODUL_NAME (syntax_tree), MODUL_FILETYPE (syntax_tree));
    }

    act_tab = checkdec_tab;

    if (MODUL_STORE_IMPORTS (syntax_tree) != NULL)
        FreeImplist (MODUL_STORE_IMPORTS (syntax_tree));

    MODUL_DECL (syntax_tree) = Trav (decl, syntax_tree);

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

    if (MODDEC_OWN (arg_node) != NULL) {
        MODDEC_OWN (arg_node) = Trav (MODDEC_OWN (arg_node), arg_info);
    }

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
 *  internal funs : CheckTypes
 *  external funs : SearchTypedef, Trav, SearchImplementation
 *  macros        : DBUG, ERROR, TREE
 *
 *  remarks       :
 *
 */

node *
CDECtypedef (node *arg_node, node *arg_info)
{
    node *tdef;
    types *type_impl;

    DBUG_ENTER ("CDECtypedef");

    if (TYPEDEF_BASETYPE (arg_node) == T_hidden) {
        tdef = SearchTypedef (TYPEDEF_NAME (arg_node), TYPEDEF_MOD (arg_node), arg_info);
        if (tdef == NULL) {
            if (NODE_LINE (arg_node) == 0) {

                ERROR (NODE_LINE (arg_node), ("Implementation of class type '%s` missing",
                                              ItemName (arg_node)));
            } else {
                ERROR (NODE_LINE (arg_node),
                       ("Implementation of implicit type '%s` missing",
                        ItemName (arg_node)));
            }

        } else {
            if (TYPEDEF_STATUS (tdef) == ST_imported) {
                ERROR (NODE_LINE (arg_node),
                       ("Implementation of implicit type '%s` missing",
                        ItemName (arg_node)));
            }
            if ((TYPEDEF_ATTRIB (tdef) == ST_unique) && (NODE_LINE (arg_node) != 0)) {
                ERROR (NODE_LINE (arg_node),
                       ("Illegal declaration of class type '%s`", ItemName (arg_node)));
            }
        }

        TYPEDEC_DEF (arg_node) = tdef;
    } else {
        tdef = SearchTypedef (TYPEDEF_NAME (arg_node), TYPEDEF_MOD (arg_node), arg_info);

        if (tdef == NULL) {
            WARN (NODE_LINE (arg_node),
                  ("Implementation of explicit type '%s` missing", ItemName (arg_node)));
        } else {
            if (TYPEDEF_STATUS (tdef) == ST_imported) {
                ERROR (NODE_LINE (arg_node),
                       ("Implementation of explicit type '%s` missing",
                        ItemName (arg_node)));
            } else {
                if (TYPEDEF_BASETYPE (arg_node) == T_user) {
                    type_impl = SearchImplementation (TYPEDEF_TYPE (arg_node), arg_info);
                } else {
                    type_impl = TYPEDEF_TYPE (arg_node);
                }

                if (type_impl == NULL) {
                    ERROR (NODE_LINE (arg_node), ("Declaration of explicit type '%s`\n\t"
                                                  "different from implementation",
                                                  ItemName (arg_node)));
                } else {
                    if (CheckTypes (type_impl, TYPEDEF_TYPE (tdef)) == 0) {
                        ERROR (NODE_LINE (arg_node),
                               ("Declaration of explicit type '%s`\n\t"
                                "different from implementation",
                                ItemName (arg_node)));
                    }
                    if (TYPEDEF_ATTRIB (tdef) == ST_unique) {
                        ERROR (NODE_LINE (arg_node),
                               ("Illegal declaration of class type '%s`",
                                ItemName (arg_node)));
                    }
                }
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
 *  internal funs : CheckTypes
 *  external funs : SearchObjdef, Trav
 *  macros        : DBUG, ERROR
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
               ("Implementation of global object '%s` missing", ItemName (arg_node)));
    } else {
        if (TYPEDEF_STATUS (odef) == ST_imported) {
            ERROR (NODE_LINE (arg_node),
                   ("Implementation of global object '%s` missing", ItemName (arg_node)));
        } else {
            if (!CheckTypes (OBJDEF_TYPE (arg_node), OBJDEF_TYPE (odef))) {
                ERROR (NODE_LINE (arg_node),
                       ("Type mismatch in declaration of\n\tglobal object '%s`",
                        ItemName (arg_node)));
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
 *  internal funs : CheckTypes
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
    types *tmpdec, *tmpdef;
    int counter;

    DBUG_ENTER ("CDECfundef");

    fundef = SearchFundef (arg_node, arg_info);

    if (fundef == NULL) {
        ERROR (NODE_LINE (arg_node), ("Implementation of function '%s` missing\n\t"
                                      "or type mismatch in arguments",
                                      ItemName (arg_node)));
    } else {
        if (FUNDEF_STATUS (fundef) == ST_imported) {
            ERROR (NODE_LINE (arg_node), ("Implementation of function '%s` missing\n\t"
                                          "or type mismatch in arguments",
                                          ItemName (arg_node)));
        } else {
            tmpdef = FUNDEF_TYPES (fundef);
            tmpdec = FUNDEF_TYPES (arg_node);
            counter = 1;

            while ((tmpdef != NULL) && (tmpdec != NULL)) {
                if (!CheckTypes (tmpdef, tmpdec)) {
                    ERROR (NODE_LINE (arg_node),
                           ("Type mismatch in %d. return value in declaration\n\t"
                            "of function '%s`",
                            counter, ItemName (arg_node)));
                }

                counter++;
                tmpdef = TYPES_NEXT (tmpdef);
                tmpdec = TYPES_NEXT (tmpdec);
            }

            if (tmpdef != tmpdec) {
                ERROR (NODE_LINE (arg_node),
                       ("Wrong number of return values in declaration\n\t"
                        "of function '%s`",
                        ItemName (arg_node)));
            }
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
 *  functionname  : WDECmodul
 *  arguments     : 1) N_modul node of module/class implementation
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
WDECmodul (node *arg_node, node *arg_info)
{
    FILE *store_outfile;

    DBUG_ENTER ("WDECmodul");

    decfile = fopen (filename, "w");

    if (decfile == NULL) {
        SYSABORT (("Unable to open file \"%s\" for writing", filename));
    }

    if (MODUL_FILETYPE (arg_node) == F_modimp) {
        fprintf (decfile, "ModuleDec %s :\n\n", MODUL_NAME (arg_node));
    } else {
        fprintf (decfile, "ClassDec %s :\n\n", MODUL_NAME (arg_node));
    }

    if (MODUL_STORE_IMPORTS (arg_node) != NULL) {
        store_outfile = outfile;
        outfile = decfile;

        PrintImplist (MODUL_STORE_IMPORTS (arg_node), NULL);

        outfile = store_outfile;
    }

    fprintf (decfile, "own:\n{\n");

    fprintf (decfile, "implicit types:\n");
    if (MODUL_TYPES (arg_node) != NULL) {
        if (MODUL_FILETYPE (arg_node) == F_classimp) {
            Trav (MODUL_TYPES (arg_node), (node *)MODUL_NAME (arg_node));
        } else {
            Trav (MODUL_TYPES (arg_node), NULL);
        }
    }

    fprintf (decfile, "\nexplicit types:\n");

    fprintf (decfile, "\nglobal objects:\n");
    if (MODUL_OBJS (arg_node) != NULL)
        Trav (MODUL_OBJS (arg_node), (node *)MODUL_NAME (arg_node));

    fprintf (decfile, "\nfunctions:\n");
    if (MODUL_FUNS (arg_node) != NULL)
        Trav (MODUL_FUNS (arg_node), (node *)MODUL_NAME (arg_node));

    fprintf (decfile, "}\n");

    fclose (decfile);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : WDECtypedef
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
WDECtypedef (node *arg_node, node *arg_info)
{
    char *classname;

    DBUG_ENTER ("WDECtypedef");

    classname = (char *)arg_info;

    if (TYPEDEF_STATUS (arg_node) == ST_regular) {
        if (classname == NULL) {
            fprintf (decfile, "  %s;\n", TYPEDEF_NAME (arg_node));
        } else {
            if ((strcmp (TYPEDEF_NAME (arg_node), classname) != 0)
                || (strcmp (MOD (TYPEDEF_MOD (arg_node)), classname) != 0)) {
                fprintf (decfile, "  %s;\n", TYPEDEF_NAME (arg_node));
            }
        }
    }

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        Trav (TYPEDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : WDECobjdef
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
WDECobjdef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WDECobjdef");

    if (OBJDEF_STATUS (arg_node) == ST_regular) {
        fprintf (decfile, "  ");

        PrintDecTypes (OBJDEF_TYPE (arg_node), (char *)arg_info);

        fprintf (decfile, " %s;\n", OBJDEF_NAME (arg_node));
    }

    if (OBJDEF_NEXT (arg_node) != NULL) {
        Trav (OBJDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : WDECfundef
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
WDECfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WDECfundef");

    if (FUNDEF_STATUS (arg_node) == ST_regular) {
        fprintf (decfile, "  ");

        PrintDecTypes (FUNDEF_TYPES (arg_node), (char *)arg_info);

        fprintf (decfile, " %s(", FUNDEF_NAME (arg_node));

        if (FUNDEF_ARGS (arg_node) != NULL) {
            Trav (FUNDEF_ARGS (arg_node), arg_info);
        }

        fprintf (decfile, ");\n");
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : WDECarg
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
WDECarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WDECarg");

    PrintDecTypes (ARG_TYPE (arg_node), (char *)arg_info);

    fprintf (decfile, " ");

    if ((ARG_ATTRIB (arg_node) == ST_reference)
        || (ARG_ATTRIB (arg_node) == ST_readonly_reference)) {
        fprintf (decfile, "&");
    }

    fprintf (decfile, "%s", ARG_NAME (arg_node));

    if (ARG_NEXT (arg_node) != NULL) {
        fprintf (decfile, ", ");
        Trav (ARG_NEXT (arg_node), arg_info);
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
