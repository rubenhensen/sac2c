/*
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:43:23  sacbase
 * new release made
 *
 * Revision 1.11  1998/12/02 16:30:37  cg
 * Now, generic object creation functions for imported global objects
 * have status ST_objinitfun rather than ST_imported. This forces the
 * typechecker to typecheck them and thereby to qualify them as being
 * used.
 *
 * Revision 1.10  1997/11/07 14:44:48  dkr
 * eliminated another nnode
 *
 * Revision 1.9  1997/04/25 14:23:48  sbs
 * OIobjdef: pragma is deleted (FreeNode) iff it is != NULL !
 *
 * Revision 1.8  1996/01/26  15:32:21  cg
 * function status ST_classfun now supported
 *
 * Revision 1.7  1996/01/25  18:46:09  cg
 * In class implementations, the typedef node for the class type
 * is generated here as well as the fundef nodes of the generic
 * conversion functions.
 *
 * Revision 1.6  1996/01/22  18:38:56  cg
 * Now, an object initialization expression is always generated
 * with respect to pragma initfun.
 *
 * Revision 1.5  1995/12/01  20:30:51  cg
 * extern declarations are now generated for generic object init
 * functions if it is a SAC-program (not module implementation)
 *
 * Revision 1.4  1995/12/01  17:25:00  cg
 * now shape segments and strings are always copied when generated
 * from existing nodes.
 *
 * Revision 1.3  1995/10/18  13:35:36  cg
 * now Malloc is used instead of malloc,
 * so error messages are no longer needed.
 *
 * Revision 1.2  1995/10/17  08:28:15  cg
 * all automatically generated functions now have status ST_objinitfun.
 * This tag is used by the typechecker.
 *
 * Revision 1.1  1995/10/16  12:22:44  cg
 * Initial revision
 *
 *
 *
 */

#include <string.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"

#include "traverse.h"
#include "dbug.h"
#include "internal_lib.h"
#include "free.h"

#include "typecheck.h"
#include "filemgr.h"

/*
 *
 *  functionname  : OImodul
 *  arguments     : 1) pointer to N_modul node
 *                  2) arg_info unused
 *  description   : traverses all global object definitions
 *                  In class implementations, a typedef node for the
 *                  class type is generated as well as generic conversion
 *                  functions.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Trav, strcpy, strcat, MakeTypedef, MakeFundef,
 *                  Malloc
 *  macros        : ---
 *
 *  remarks       :
 *
 */

node *
OImodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("OImodul");

    if (MODUL_OBJS (arg_node) != NULL) {
        MODUL_OBJS (arg_node) = Trav (MODUL_OBJS (arg_node), arg_node);
    }

    if (MODUL_FILETYPE (arg_node) == F_classimp) {
        char *toclass;
        char *fromclass;

        MODUL_TYPES (arg_node)
          = MakeTypedef (StringCopy (MODUL_NAME (arg_node)), MODUL_NAME (arg_node),
                         DuplicateTypes (MODUL_CLASSTYPE (arg_node), 1), ST_unique,
                         MODUL_TYPES (arg_node));

        toclass = (char *)Malloc (MAX_FILE_NAME);
        fromclass = (char *)Malloc (MAX_FILE_NAME);

        strcpy (toclass, "to_");
        strcpy (fromclass, "from_");

        strcat (toclass, MODUL_NAME (arg_node));
        strcat (fromclass, MODUL_NAME (arg_node));

        MODUL_FUNS (arg_node)
          = MakeFundef (toclass, MODUL_NAME (arg_node),
                        MakeType (T_user, 0, NULL, StringCopy (MODUL_NAME (arg_node)),
                                  MODUL_NAME (arg_node)),
                        MakeArg (NULL, DuplicateTypes (MODUL_CLASSTYPE (arg_node), 1),
                                 ST_regular, ST_regular, NULL),
                        NULL, MODUL_FUNS (arg_node));

        FUNDEF_STATUS (MODUL_FUNS (arg_node)) = ST_classfun;

        MODUL_FUNS (arg_node)
          = MakeFundef (fromclass, MODUL_NAME (arg_node), MODUL_CLASSTYPE (arg_node),
                        MakeArg (NULL,
                                 MakeType (T_user, 0, NULL,
                                           StringCopy (MODUL_NAME (arg_node)),
                                           MODUL_NAME (arg_node)),
                                 ST_regular, ST_regular, NULL),
                        NULL, MODUL_FUNS (arg_node));

        FUNDEF_STATUS (MODUL_FUNS (arg_node)) = ST_classfun;

        MODUL_CLASSTYPE (arg_node) = NULL;

        /*************************************************************/
#ifndef NEWTREE
        if (TYPEDEF_NEXT (MODUL_TYPES (arg_node)) == NULL) {
            MODUL_TYPES (arg_node)->nnode -= 1;
        }

        if (FUNDEF_NEXT (FUNDEF_NEXT (MODUL_FUNS (arg_node))) == NULL) {
            FUNDEF_NEXT (MODUL_FUNS (arg_node))->nnode = 0;
        }

        FUNDEF_ARGS (MODUL_FUNS (arg_node))->nnode -= 1;
        FUNDEF_ARGS (FUNDEF_NEXT (MODUL_FUNS (arg_node)))->nnode -= 1;
#endif
        /*************************************************************/
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : OIobjdef
 *  arguments     : 1) pointer to objdef node
 *                  2) pointer to modul node (head of syntax_tree)
 *  description   : A new function definition is generated that has no
 *                  formal parameters, the return type is identical to
 *                  the type of the global object and the initialization
 *                  expression is moved to the return-statement of the
 *                  new function. In the objdef node it's replaced by
 *                  an application of the new function.
 *
 *                  This mechanism allows for arbitrary expression to
 *                  initialize global objects.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Malloc, strlen, strcpy, strcat,
 *                  MakeType, MakeExprs, MakeReturn, MakeAssign, MakeBlock,
 *                  MakeFundef, MakeAp, Trav
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
OIobjdef (node *arg_node, node *arg_info)
{
    node *new_node;
    types *new_fun_type;
    char *new_fun_name;

    DBUG_ENTER ("OIobjdef");

    if (OBJDEF_EXPR (arg_node) != NULL) {
        new_fun_type
          = MakeType (OBJDEF_BASETYPE (arg_node), OBJDEF_DIM (arg_node),
                      CopyShpseg (OBJDEF_SHPSEG (arg_node)),
                      StringCopy (OBJDEF_TNAME (arg_node)), OBJDEF_TMOD (arg_node));

        new_fun_name = (char *)Malloc (strlen (OBJDEF_NAME (arg_node)) + 10);

        new_fun_name = strcpy (new_fun_name, "CREATE__");
        new_fun_name = strcat (new_fun_name, OBJDEF_NAME (arg_node));

        new_node = MakeExprs (OBJDEF_EXPR (arg_node), NULL);
        NODE_LINE (new_node) = NODE_LINE (OBJDEF_EXPR (arg_node));

#ifndef NEWTREE
        new_node->nnode = 1;
#endif

        new_node = MakeReturn (new_node);
        NODE_LINE (new_node) = NODE_LINE (OBJDEF_EXPR (arg_node));

        new_node = MakeAssign (new_node, NULL);
        NODE_LINE (new_node) = NODE_LINE (OBJDEF_EXPR (arg_node));

#ifndef NEWTREE
        new_node->nnode = 1;
#endif

        new_node = MakeBlock (new_node, NULL);
        NODE_LINE (new_node) = NODE_LINE (OBJDEF_EXPR (arg_node));

#ifndef NEWTREE
        new_node->nnode = 1;
#endif

        new_node = MakeFundef (new_fun_name, OBJDEF_MOD (arg_node), new_fun_type, NULL,
                               new_node, MODUL_FUNS (arg_info));
        NODE_LINE (new_node) = NODE_LINE (OBJDEF_EXPR (arg_node));

#ifndef NEWTREE
        new_node->nnode = (MODUL_FUNS (arg_info) == NULL) ? 1 : 2;
#endif

        FUNDEF_STATUS (new_node) = ST_objinitfun;

        /*
         * The new functions have status ST_objinitfun.
         * This tag is needed by the typechecker to ensure that these functions
         * are actually typechecked even if they are exclusively applied in
         * a global object initialization of a SAC-program.
         */

        MODUL_FUNS (arg_info) = new_node;

        new_node = MakeAp (StringCopy (new_fun_name), OBJDEF_MOD (arg_node), NULL);
        OBJDEF_EXPR (arg_node) = new_node;

        AP_FUNDEF (OBJDEF_EXPR (arg_node)) = MODUL_FUNS (arg_info);

    } else {
        new_fun_type
          = MakeType (OBJDEF_BASETYPE (arg_node), OBJDEF_DIM (arg_node),
                      CopyShpseg (OBJDEF_SHPSEG (arg_node)),
                      StringCopy (OBJDEF_TNAME (arg_node)), OBJDEF_TMOD (arg_node));

        if (OBJDEF_INITFUN (arg_node) != NULL) {
            new_fun_name = OBJDEF_INITFUN (arg_node);
            PRAGMA_INITFUN (OBJDEF_PRAGMA (arg_node)) = NULL;
        } else {
            new_fun_name = (char *)Malloc (strlen (OBJDEF_NAME (arg_node)) + 10);

            if (OBJDEF_MOD (arg_node) == NULL) {
                strcpy (new_fun_name, "create_");
            } else {
                strcpy (new_fun_name, "CREATE__");
            }

            strcat (new_fun_name, OBJDEF_NAME (arg_node));
        }

        new_node = MakeFundef (new_fun_name, OBJDEF_MOD (arg_node), new_fun_type, NULL,
                               NULL, MODUL_FUNS (arg_info));

#ifndef NEWTREE
        new_node->nnode = (MODUL_FUNS (arg_info) == NULL) ? 1 : 2;
#endif

        FUNDEF_STATUS (new_node) = ST_objinitfun;

        MODUL_FUNS (arg_info) = new_node;

        new_node = MakeAp (StringCopy (new_fun_name), OBJDEF_MOD (arg_node), NULL);
        OBJDEF_EXPR (arg_node) = new_node;

        AP_FUNDEF (OBJDEF_EXPR (arg_node)) = MODUL_FUNS (arg_info);
    }

    if ((OBJDEF_PRAGMA (arg_node) != NULL)
        && (PRAGMA_LINKNAME (OBJDEF_PRAGMA (arg_node)) == NULL)) {
        OBJDEF_PRAGMA (arg_node) = FreeNode (OBJDEF_PRAGMA (arg_node));
    }

    if (OBJDEF_NEXT (arg_node) != NULL) {
        OBJDEF_NEXT (arg_node) = Trav (OBJDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : objinit
 *  arguments     : 1) syntax_tree (pointer to N_modul node
 *  description   : starts the traversal mechanism for the objinit
 *                  compilation phase. Here, all initialization expressions
 *                  of global objects are moved to new generated functions
 *                  and replaced by applications of the respective functions.
 *  global vars   : act_tab, objinit_tab
 *  internal funs : ---
 *  external funs : Trav
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
objinit (node *syntax_tree)
{
    DBUG_ENTER ("objinit");

    act_tab = objinit_tab;

    DBUG_RETURN (Trav (syntax_tree, NULL));
}
