/*
 *
 * $Log$
 * Revision 1.3  1995/10/05 16:05:39  cg
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

#include <malloc.h>
#include <string.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"

#include "free.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"

#include "scnprs.h"
#include "traverse.h"

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
LoadDeclaration (char *name, file_type modtype)
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

    NOTE (("\n  Loading %s ...\n", buffer));

    linenum = 1;
    start_token = PARSE_DEC;
    yyparse ();
    decl = decl_tree;

    if (strcmp (MODDEC_NAME (decl), name) != 0) {
        ERROR2 (
          1, ("%s :ERROR: File does not provide module/class %s, but module/class %s!\n",
              buffer, name, MODDEC_NAME (decl)));
    }

    if ((NODE_TYPE (decl) == N_classdec) && (modtype == F_modimp)) {
        ERROR2 (1, ("%s :ERROR: implementation of module but declaration of class",
                    filename, name));
    }

    if ((NODE_TYPE (decl) == N_moddec) && (modtype == F_classimp)) {
        ERROR2 (1, ("%s :ERROR: implementation of class but declaration of module",
                    filename, name));
    }

    if (NODE_TYPE (decl) == N_classdec) {
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
    node *tmp, *tdef;

    DBUG_ENTER ("HandleImplicitTypes");

    tmp = declarations;
    while (tmp != NULL) {
        tdef = SearchTypedef (TYPEDEF_NAME (tmp), TYPEDEF_MOD (tmp), implementations);
        if (tdef == NULL) {
            ERROR2 (1, ("%s :ERROR: Implementation of implicit type %s missing", filename,
                        TYPEDEF_NAME (tmp)));
        }

        fprintf (sibfile, "typedef %s ", Type2String (TYPEDEF_TYPE (tdef), 0));
        fprintf (sibfile, "%s;\n", TYPEDEF_NAME (tdef));

        tmp = TYPEDEF_NEXT (tmp);
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
        def = SearchTypedef (TYPEDEF_NAME (tmp), TYPEDEF_MOD (tmp), implementations);
        if (def == NULL) {
            WARN1 (("%s :WARNING: implementation of explicit type %s missing", filename,
                    TYPEDEF_NAME (tmp)));
        } else {
            if (CheckExplicitType (tmp, def) == 0) {
                ERROR2 (1, ("%s :ERROR: implementation of explicit type %s different "
                            "from declaration",
                            filename, TYPEDEF_NAME (tmp)));
            }
        }

        tmp = TYPEDEF_NEXT (tmp);
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : HandleFunctions
 *  arguments     : 1) list of function declarations
 *                  2) list of function definitions
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
void HandleFunctions(node* declarations, node* implementations)
{
  node *tmp, *fundef;

  DBUG_ENTER("HandleFunctions");

  tmp=declarations;
  while (tmp!=NULL)
  {
    fundef=SearchFundef(tmp, implementations);

    if (fundef==NULL)
    {
      ERROR2(1,("%s :ERROR: Implementation of function %s missing or wrong in arguments",
                filename,
                FUNDEF_NAME(tmp)));
    }







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

    if (MODUL_FILETYPE (arg_node) != F_prog)
    /* it's a module/class implementation */
    {
        decl = LoadDeclaration (MODUL_NAME (arg_node), MODUL_FILETYPE (arg_node));

        sibfile = OpenSibFile (MODUL_NAME (arg_node));

        fprintf (sibfile, "<%s>\n\n", MODUL_NAME (arg_node));

        HandleImplicitTypes (MODDEC_ITYPES (decl), MODUL_TYPES (arg_node));
        HandleExplicitTypes (MODDEC_ETYPES (decl), MODUL_TYPES (arg_node));
        /*
            HandleFunctions(MODDEC_FUNS(decl), MODUL_FUNS(arg_node));
        */

        fprintf (sibfile, "\n");

        fclose (sibfile);
    }

    DBUG_RETURN (arg_node);
}
