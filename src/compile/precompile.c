/*
 *
 * $Log$
 * Revision 1.2  1995/12/01 17:23:26  cg
 * first working revision
 *
 * Revision 1.1  1995/11/28  12:23:34  cg
 * Initial revision
 *
 *
 *
 */

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"

#include "internal_lib.h"
#include "convert.h"
#include "traverse.h"

#include "dbug.h"

#include <string.h>

/*
 *
 *  functionname  : precompile
 *  arguments     : 1) syntax tree
 *  description   : prepares syntax tree for code generation.
 *                  - renames functions and global objects
 *                  - inserts extern declarations for function definitions
 *                  - removes all artificial parameters and return values
 *                  - marks reference parameters in function applications
 *  global vars   : act_tab, precomp_tab
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : DBUG
 *
 *  remarks       :
 *
 */

node *
precompile (node *syntax_tree)
{
    DBUG_ENTER ("precompile");

    act_tab = precomp_tab;

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

/*
 *
 *  functionname  : InsertObjInits
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
InsertObjInits (node *fundef, node *objdef)
{
    node *block;

    DBUG_ENTER ("InsertObjInits");

    block = FUNDEF_BODY (fundef);

    while (objdef != NULL) {
        if (OBJDEF_INIT (objdef) != NULL) {
            BLOCK_INSTR (block) = MakeAssign (OBJDEF_INIT (objdef), BLOCK_INSTR (block));
        }

        objdef = OBJDEF_NEXT (objdef);
    }

    DBUG_RETURN (fundef);
}

/*
 *
 *  functionname  : RenameFun
 *  arguments     : 1) N_fundef node
 *  description   : renames the given function.
 *                  For SAC-functions, a new name is created from the module
 *                  name, the original name and the argument's types.
 *                  For C-functions, a new name is taken from the pragma
 *                  'linkname' if present.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Malloc, strlen, strcat, sprintf, Type2String
 *  macros        : DBUG, TREE, FREE
 *
 *  remarks       :
 *
 */

node *
RenameFun (node *fun)
{
    node *args;
    char *new_name;
    int length = 0;

    DBUG_ENTER ("RenameFun");

    if (FUNDEF_MOD (fun) != NULL) {
        /*
         *  These are SAC-functions which may be overloaded.
         */

        args = FUNDEF_ARGS (fun);

        while (args != NULL) {
            ARG_TYPESTRING (args) = Type2String (ARG_TYPE (args), 2);
            length += strlen (ARG_TYPESTRING (args));
            args = ARG_NEXT (args);
        }

        length += (strlen (FUNDEF_NAME (fun)) + strlen (FUNDEF_MOD (fun))
                   + strlen (MOD_NAME_CON) + 3);

        new_name = (char *)Malloc (sizeof (char) * length);

        sprintf (new_name, "%s%s%s_", FUNDEF_MOD (fun), MOD_NAME_CON, FUNDEF_NAME (fun));

        args = FUNDEF_ARGS (fun);

        while (args != NULL) {
            strcat (new_name, ARG_TYPESTRING (args));
            args = ARG_NEXT (args);
        }

        FREE (FUNDEF_NAME (fun));

        /* don't free FUNDEF_MOD(fun) because it is shared !! */

        FUNDEF_NAME (fun) = new_name;
        FUNDEF_MOD (fun) = NULL;
    } else {
        if ((FUNDEF_PRAGMA (fun) != NULL) && (FUNDEF_LINKNAME (fun) != NULL)) {
            /*
             *  These are C-functions with additional pragma 'linkname'.
             */

            FREE (FUNDEF_NAME (fun));

            /* don't free FUNDEF_MOD(fun) because it is shared !! */

            FUNDEF_NAME (fun) = FUNDEF_LINKNAME (fun);
        }
    }

    DBUG_RETURN (fun);
}

/*
 *
 *  functionname  : PRECmodul
 *  arguments     : 1) N_modul node
 *                  2) arg_info unused
 *  description   : starts traversal mechanism for objdef and fundef nodes
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

node *
PRECmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECmodul");

    if (MODUL_OBJS (arg_node) != NULL) {
        MODUL_OBJS (arg_node) = Trav (MODUL_OBJS (arg_node), arg_info);
    }

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_node);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PRECobjdef
 *  arguments     : 1) N_objdef node
 *                  2) arg_info unused
 *  description   : renames global objects.
 *                  For SAC-functions the VARNAME, a combination of module
 *                  name and object name is used, for C-functions the
 *                  optional 'linkname' is used if present.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : DBUG, TREE, FREE
 *
 *  remarks       :
 *
 */

node *
PRECobjdef (node *arg_node, node *arg_info)
{
    char *init_fun_name;
    node *new_node;
    ids *new_ids;

    DBUG_ENTER ("PRECobjdef");

    if (OBJDEF_MOD (arg_node) == NULL) {
        if ((OBJDEF_PRAGMA (arg_node) != NULL) && (OBJDEF_LINKNAME (arg_node) != NULL)) {
            FREE (OBJDEF_NAME (arg_node));
            OBJDEF_NAME (arg_node) = OBJDEF_LINKNAME (arg_node);
        }

        OBJDEF_INIT (arg_node) = NULL;
    } else {
        init_fun_name = (char *)Malloc (strlen (OBJDEF_NAME (arg_node)) + 10);

        init_fun_name = strcpy (init_fun_name, "_CREATE_");
        init_fun_name = strcat (init_fun_name, OBJDEF_NAME (arg_node));

        new_node = MakeAp (init_fun_name, OBJDEF_MOD (arg_node), NULL);
        new_ids = MakeIds (StringCopy (OBJDEF_VARNAME (arg_node)), NULL, ST_regular);
        OBJDEF_INIT (arg_node) = MakeLet (new_node, new_ids);

        FREE (OBJDEF_NAME (arg_node));
        OBJDEF_NAME (arg_node) = OBJDEF_VARNAME (arg_node);
        OBJDEF_MOD (arg_node) = NULL;
    }

    if (OBJDEF_NEXT (arg_node) != NULL) {
        OBJDEF_NEXT (arg_node) = Trav (OBJDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PRECfundef
 *  arguments     : 1) N_fundef node
 *                  2) arg_info unused
 *  description   : triggers the precompilation
 *  global vars   : ---
 *  internal funs : RenameFun
 *  external funs : Trav
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

node *
PRECfundef (node *arg_node, node *arg_info)
{
    int cnt_artificial = 0, i;
    char *keep_name, *keep_mod;
    statustype keep_status, keep_attrib;

    DBUG_ENTER ("PRECfundef");

    /*
     *  The body of an imported inline function is removed.
     */

    if ((FUNDEF_STATUS (arg_node) == ST_imported)
        && (FUNDEF_ATTRIB (arg_node) != ST_generic) && (FUNDEF_BODY (arg_node) != NULL)) {
        FUNDEF_BODY (arg_node) = FreeTree (FUNDEF_BODY (arg_node));
    }

    /*
     *  The function body is traversed in order to remove artificial return
     *  values and parameters of function applications.
     */

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    /*
     *  The function arguments are traversed, artificial arguments are removed
     *  and the number of reference parameters (including global objects)
     *  is counted and stored in 'cnt_artificial'
     */

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), (node *)&cnt_artificial);
    }

    /*
     *  All artificial return types are removed.
     *  It is necessary to keep name, module name, status, and attrib
     *  because in the real syntax tree these are stored within the types
     *  structure and not as part of the fundef node as in the virtual
     *  syntax tree.
     */

    if (cnt_artificial > 0) {

        /********************************************************************/
        keep_name = FUNDEF_NAME (arg_node);
        keep_mod = FUNDEF_MOD (arg_node);
        keep_status = FUNDEF_STATUS (arg_node);
        keep_attrib = FUNDEF_ATTRIB (arg_node);
        /********************************************************************/

        for (i = 1; i < cnt_artificial; i++) {
            FUNDEF_TYPES (arg_node) = FreeOneTypes (FUNDEF_TYPES (arg_node));
        }

        if (TYPES_NEXT (FUNDEF_TYPES (arg_node)) == NULL) {
            FUNDEF_BASETYPE (arg_node) = T_void;
            FREE (FUNDEF_TNAME (arg_node));
            FUNDEF_TNAME (arg_node) = NULL;
            FUNDEF_TMOD (arg_node) = NULL;
        } else {
            FUNDEF_TYPES (arg_node) = FreeOneTypes (FUNDEF_TYPES (arg_node));
        }

        /********************************************************************/
        FUNDEF_NAME (arg_node) = keep_name;
        FUNDEF_MOD (arg_node) = keep_mod;
        FUNDEF_STATUS (arg_node) = keep_status;
        FUNDEF_ATTRIB (arg_node) = keep_attrib;
        /********************************************************************/
    }

    /*
     *  The main function is extended by applications of the init functions
     *  of all global objects.
     *  All other functions are renamed.
     */

    if (strcmp (FUNDEF_NAME (arg_node), "main") == 0) {
        arg_node = InsertObjInits (arg_node, MODUL_OBJS (arg_info));
    } else {
        arg_node = RenameFun (arg_node);
    }

    /*
     *  The inline flag is set to 0
     */

    FUNDEF_INLINE (arg_node) = 0;

    /*
     *  Now, traverse the following functions.
     */

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PRECarg
 *  arguments     : 1) N_arg node
 *                  2) casted pointer to an integer, which is used to
 *                     count the number of artificial return values.
 *  description   : An artificial argument is removed,
 *                  the attribs are switched:
 *                    ST_readonly_reference -> ST_regular
 *                    ST_was_reference -> ST_inout
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : FreeNode, Trav
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

node *
PRECarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECarg");

    if (ARG_ATTRIB (arg_node) == ST_was_reference) {
        *((int *)arg_info) += 1;
    }

    if (ARG_STATUS (arg_node) == ST_artificial) {
        arg_node = FreeNode (arg_node);

        if (arg_node != NULL) {
            arg_node = Trav (arg_node, arg_info);
        }
    } else {
        if (ARG_ATTRIB (arg_node) == ST_readonly_reference) {
            ARG_ATTRIB (arg_node) = ST_regular;
        } else {
            if (ARG_ATTRIB (arg_node) == ST_was_reference) {
                ARG_ATTRIB (arg_node) = ST_inout;
            }
        }

        if (ARG_NEXT (arg_node) != NULL) {
            ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);

            /**************************************************************/
            if (ARG_NEXT (arg_node) == NULL) {
                arg_node->nnode = 0;
            }
            /**************************************************************/
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PREClet
 *  arguments     : 1) N_let node
 *                  2) arg_info unused
 *  description   : removes all artificial identifiers on the left hand
 *                  side of a let.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : FreeOneIds, Trav
 *  macros        : TREE, DBUG
 *
 *  remarks       :
 *
 */

node *
PREClet (node *arg_node, node *arg_info)
{
    ids *let_ids;

    DBUG_ENTER ("PREClet");

    let_ids = LET_IDS (arg_node);

    while ((let_ids != NULL) && (IDS_STATUS (let_ids) == ST_artificial)) {
        let_ids = FreeOneIds (let_ids);
    }

    LET_IDS (arg_node) = let_ids;

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PRECap
 *  arguments     : 1) N_ap node
 *                  2) arg_info unused
 *  description   : traverses the current arguments and sets arg_info
 *                  to the first fromal parameter of the applied function.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

node *
PRECap (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECap");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node)
          = Trav (AP_ARGS (arg_node), FUNDEF_ARGS (AP_FUNDEF (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PRECreturn
 *  arguments     : 1) N_return node
 *                  2) arg_info unused
 *  description   : traverses the return values and sets arg_info to
 *                  this N_return node.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

node *
PRECreturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECreturn");

    if (RETURN_EXPRS (arg_node) != NULL) {
        RETURN_EXPRS (arg_node) = Trav (RETURN_EXPRS (arg_node), arg_node);
    }

    if (RETURN_EXPRS (arg_node) == NULL) {
        arg_node->nnode = 0;
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PRECexprs
 *  arguments     : 1) N_exprs node
 *                  2) If below an N_return node:
 *                       pointer to this N_return node
 *                     If below an N_ap node:
 *                       pointer to corresponding formal parameter of
 *                       applied function
 *                     Otherwise:
 *                       unused (NULL)
 *  description   : removes artificial expressions.
 *                  The exact behaviour relies on the second parameter.
 *
 *                  N_return:
 *                  If the return value corresponds to an original reference
 *                  parameter and not a global object, it is stored within
 *                  the return-node to be used for compilation. Otherwise
 *                  it is freed.
 *
 *                  N_arg:
 *                  Additionally to removing artificial expressions, an
 *                  identifier that corresponds to a reference parameter
 *                  of the applied function is marked as attrib ST_inout
 *                  for later usage by compile.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : FreeNode, Trav
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

node *
PRECexprs (node *arg_node, node *arg_info)
{
    node *next;

    DBUG_ENTER ("PRECexprs");

    if (arg_info == NULL) {
        /*
         *  This N_exprs node is neither below an N_ap nor below an N_return.
         *  So, traverse it as usual.
         */

        EXPRS_EXPR (arg_node) = Trav (EXPRS_EXPR (arg_node), arg_info);

        if (EXPRS_NEXT (arg_node) != NULL) {
            EXPRS_NEXT (arg_node) = Trav (EXPRS_NEXT (arg_node), arg_info);
        }
    } else {
        if (NODE_TYPE (arg_info) == N_arg) {
            if (NODE_TYPE (EXPRS_EXPR (arg_node)) == N_id) {
                /*
                 *  This N_exprs node contains an idenitifier and is situated
                 *  below an N_ap node.
                 */

                if (ID_STATUS (EXPRS_EXPR (arg_node)) == ST_artificial) {
                    arg_node = FreeNode (arg_node);

                    if (arg_node != NULL) {
                        arg_node = Trav (arg_node, ARG_NEXT (arg_info));
                    }
                } else {
                    EXPRS_EXPR (arg_node) = Trav (EXPRS_EXPR (arg_node), arg_info);

                    if (EXPRS_NEXT (arg_node) != NULL) {
                        EXPRS_NEXT (arg_node)
                          = Trav (EXPRS_NEXT (arg_node), ARG_NEXT (arg_info));
                    }

                    /****************************************************************/
                    if (EXPRS_NEXT (arg_node) == NULL) {
                        arg_node->nnode = 1;
                    }
                    /****************************************************************/
                }
            } else {
                EXPRS_EXPR (arg_node) = Trav (EXPRS_EXPR (arg_node), arg_info);

                if (EXPRS_NEXT (arg_node) != NULL) {
                    EXPRS_NEXT (arg_node)
                      = Trav (EXPRS_NEXT (arg_node), ARG_NEXT (arg_info));
                }

                /****************************************************************/
                if (EXPRS_NEXT (arg_node) == NULL) {
                    arg_node->nnode = 1;
                }
                /****************************************************************/
            }
        } else {
            if ((NODE_TYPE (EXPRS_EXPR (arg_node)) == N_id)
                && (ID_STATUS (EXPRS_EXPR (arg_node)) == ST_artificial)) {
                /*
                 *  This N_exprs node contains an idenitifier, is situated
                 *  below an N_return node, and is artificially inserted.
                 */

                if ((NODE_TYPE (ID_VARDEC (EXPRS_EXPR (arg_node))) == N_arg)
                    && (ARG_STATUS (ID_VARDEC (EXPRS_EXPR (arg_node))) == ST_regular)) {
                    /*
                     *  The current return value corresponds to an original
                     *  call-by-reference parameter. So, it is stored within
                     *  the N_return node for compilation.
                     */

                    next = EXPRS_NEXT (arg_node);
                    EXPRS_NEXT (arg_node) = RETURN_REFERENCE (arg_info);
                    RETURN_REFERENCE (arg_info) = arg_node;

                    if (next != NULL) {
                        arg_node = Trav (next, arg_info);
                    } else {
                        arg_node = NULL;
                    }
                } else {
                    arg_node = FreeNode (arg_node);

                    if (arg_node != NULL) {
                        arg_node = Trav (arg_node, arg_info);
                    }
                }
            } else {
                EXPRS_EXPR (arg_node) = Trav (EXPRS_EXPR (arg_node), arg_info);

                if (EXPRS_NEXT (arg_node) != NULL) {
                    EXPRS_NEXT (arg_node) = Trav (EXPRS_NEXT (arg_node), arg_info);
                }

                /****************************************************************/
                if (EXPRS_NEXT (arg_node) == NULL) {
                    arg_node->nnode = 1;
                }
                /****************************************************************/
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PRECid
 *  arguments     : 1) N_id node;
 *                  2) same as in PRECexprs
 *  description   : 1) Applied occurrences of global objects may be renamed,
 *                     if the global object was renamed.
 *                  2) Identifiers in function applications are marked as
 *                     attrib 'ST_inout', if the corresponding formal
 *                     parameter of the applied function is a reference
 *                     parameter.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

node *
PRECid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECid");

    if ((NODE_TYPE (ID_VARDEC (arg_node)) == N_arg)
        && (ARG_STATUS (ID_VARDEC (arg_node)) == ST_artificial)) {
        ID_VARDEC (arg_node) = ARG_OBJDEF (ID_VARDEC (arg_node));
        ID_NAME (arg_node) = OBJDEF_NAME (ID_VARDEC (arg_node));
    }

    if ((arg_info != NULL) && (NODE_TYPE (arg_info) == N_arg)
        && (ARG_ATTRIB (arg_info) == ST_was_reference)) {
        ID_ATTRIB (EXPRS_EXPR (arg_node)) = ST_inout;
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
