/*
 *
 * $Log$
 * Revision 1.35  1998/03/27 18:39:56  dkr
 * added split phase
 * N_WLproj renamed in N_WLstride
 *
 * Revision 1.34  1998/03/26 15:38:58  dkr
 * improved sort order in CompareWLnode.
 * fixed a few bugs in PRECnwith ...
 * new usage of MakeWLgrid.
 *
 * Revision 1.33  1998/03/25 19:45:12  dkr
 * PRECnwith:
 *   added break specifiers
 *   new phase order: split - block - merge !!
 *
 * Revision 1.32  1998/03/24 21:45:41  dkr
 * changed IntersectOutline
 *
 * Revision 1.31  1998/03/24 21:31:41  dkr
 * removed a bug with WLPROJ_PART
 *
 * Revision 1.30  1998/03/24 21:09:53  dkr
 * removed a bug in IntersectOutline
 *
 * Revision 1.29  1998/03/22 23:43:35  dkr
 * N_WLgrid: OFFSET, WIDTH -> BOUND1, BOUND2
 *
 * Revision 1.28  1998/03/21 23:45:24  dkr
 * fixed a few bugs in PRECnwith
 * added parts of phase 5 (split-merge)
 *
 * Revision 1.27  1998/03/21 16:19:10  dkr
 * fixed a few bugs in PRECnwith
 *
 * Revision 1.26  1998/03/20 21:57:48  dkr
 * first version of PRECnwith:
 *   cube-building implemented
 *   choice of segments implemented (rudimental)
 *   hierarchical blocking implemented
 *
 * Revision 1.25  1998/03/20 20:52:29  dkr
 * changed usage of MakeWLseg
 *
 * Revision 1.24  1998/03/20 17:25:54  dkr
 * in N_WL... nodes: INNER is now called CONTENTS
 *
 * Revision 1.22  1998/03/19 20:57:42  dkr
 * added computation of the cubes
 *
 * Revision 1.20  1998/03/15 23:22:39  dkr
 * changed PRECnwith()
 *
 * Revision 1.18  1998/03/03 23:00:11  dkr
 * added PRECncode()
 *
 * Revision 1.17  1998/03/02 22:26:36  dkr
 * added PRECnwith()
 *
 * Revision 1.16  1997/11/24 15:45:45  dkr
 * removed a bug in PREClet(), PRECassign():
 * - LET_EXPR, ASSIGN_INSTR now only freed once!
 *
 * Revision 1.15  1997/11/22 15:41:31  dkr
 * problem with shared linknames fixed:
 * RenameFun() now copies the linkname to FUNDEF_NAME
 *
 * Revision 1.14  1997/09/05 13:46:04  cg
 * All cast expressions are now removed by rmvoidfun.c. Therefore,
 * the respective attempts in precompile.c and ConstantFolding.c
 * are removed. Cast expressions are only used by the type checker.
 * Afterwards, they are useless, and they are not supported by
 * Constant Folding as well as code generation.
 *
 * Revision 1.13  1997/04/30 11:55:34  cg
 * Artificial return values and arguments are removed even in the case
 * of function inlining.
 *
 * Revision 1.12  1997/04/25  09:37:40  sbs
 * DBUG_ASSERT in PRECfundef adjusted (no varargs)
 *
 * Revision 1.11  1996/03/05  15:32:04  cg
 * bug fixed in handling of functions with variable argument list
 *
 * Revision 1.10  1996/01/26  15:33:01  cg
 * applications of class conversion functions are removed
 * where necessary we mark where to copy
 *
 * Revision 1.9  1996/01/22  18:36:37  cg
 * new implementation of object initializations
 *
 * Revision 1.8  1996/01/09  09:19:43  cg
 * implemented pragma linkname for global objects
 *
 * Revision 1.7  1995/12/29  10:44:10  cg
 * some DBUG_PRINTs added.
 *
 * Revision 1.6  1995/12/18  16:30:55  cg
 * many bugs fixed, function PRECexprs replaced by PRECexprs_ap and
 * PRECexprs_return
 *
 * Revision 1.5  1995/12/04  17:00:04  cg
 * added function PRECcast
 * All casts are now eliminated by the precompiler
 *
 * Revision 1.4  1995/12/04  13:38:16  cg
 * bug fixed in string handling of function PRECtypedef
 *
 * Revision 1.3  1995/12/01  20:29:24  cg
 * now the prefix "SAC__" is added to all SAC-identifiers
 * in order to avoid name clashes with C indentifiers.
 *
 * Revision 1.2  1995/12/01  17:23:26  cg
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

#include "refcount.h"
#include "DupTree.h"
#include "dbug.h"

#include <string.h>

/* these macros are used in 'CompareWlnode' for compare purpose */
#define COMP_BEGIN(a, b, result, inc)                                                    \
    if (a > b) {                                                                         \
        result = inc;                                                                    \
    } else {                                                                             \
        if (a < b) {                                                                     \
            result = -inc;                                                               \
        } else {

#define COMP_END                                                                         \
    }                                                                                    \
    }

/*
 *
 *  functionname  : precompile
 *  arguments     : 1) syntax tree
 *  description   : prepares syntax tree for code generation.
 *                  - renames functions and global objects
 *                  - removes all casts
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
 *  functionname  : RenameTypes
 *  arguments     : 1) types structure
 *  description   : renames the given type if it is a user-defined
 *                  SAC-type. The new name is constructed from the
 *                  prefix "SAC_", the module name and the type name.
 *                  Chains of types structures are considered.
 *  global vars   : mod_name_con
 *  internal funs : ---
 *  external funs : Malloc, sprintf, strlen
 *  macros        : TREE, DBUG, FREE
 *
 *  remarks       : The complete new name is stored in NAME while
 *                  MOD is set to NULL.
 *
 */

types *
RenameTypes (types *type)
{
    char *tmp;

    DBUG_ENTER ("RenameTypes");

    if (TYPES_MOD (type) != NULL) {
        tmp = (char *)Malloc (sizeof (char)
                              * (strlen (TYPES_NAME (type))
                                 + strlen (MOD (TYPES_MOD (type)))
                                 + 2 * strlen (mod_name_con) + 4));
        sprintf (tmp, "SAC%s%s%s%s", mod_name_con, MOD (TYPES_MOD (type)), mod_name_con,
                 TYPES_NAME (type));

        DBUG_PRINT ("PREC", ("renaming type %s:%s to %s", MOD (TYPES_MOD (type)),
                             TYPES_NAME (type), tmp));

        FREE (TYPES_NAME (type));
        TYPES_NAME (type) = tmp;
        TYPES_MOD (type) = NULL;
    }

    if (TYPES_NEXT (type) != NULL) {
        TYPES_NEXT (type) = RenameTypes (TYPES_NEXT (type));
    }

    DBUG_RETURN (type);
}

/*
 *
 *  functionname  : InsertObjInits
 *  arguments     : 1) fundef node of main function
 *                  2) chain of objdef nodes
 *  description   : For each global object defined in SAC an application
 *                  of its generic initialization function is inserted
 *                  at the beginning of main.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : MakeAssign
 *  macros        : DBUG, TREE
 *
 *  remarks       : The Let_nodes are generated by PRECobjdef
 *
 */

node *
InsertObjInits (node *block, node *objects)
{
    ids *new_ids;
    node *new_assigns, *first;

    DBUG_ENTER ("InsertObjInits");

    new_assigns = MakeAssign (NULL, NULL);
    first = new_assigns;

    while (objects != NULL) {
        new_ids = MakeIds (StringCopy (OBJDEF_NAME (objects)), NULL, ST_regular);
        IDS_VARDEC (new_ids) = objects;
        IDS_ATTRIB (new_ids) = ST_global;

        if (IsArray (OBJDEF_TYPE (objects))) {
            IDS_REFCNT (new_ids) = 1;
        } else {
            IDS_REFCNT (new_ids) = -1;
        }

        ASSIGN_NEXT (first) = MakeAssign (MakeLet (OBJDEF_EXPR (objects), new_ids), NULL);

        first = ASSIGN_NEXT (first);

        OBJDEF_EXPR (objects) = NULL;

        objects = OBJDEF_NEXT (objects);
    }

    if (ASSIGN_NEXT (new_assigns) != NULL) {
        ASSIGN_NEXT (first) = BLOCK_INSTR (block);
        BLOCK_INSTR (block) = ASSIGN_NEXT (new_assigns);
    }

    FREE (new_assigns);

    DBUG_RETURN (block);
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
            length += strlen (ARG_TYPESTRING (args)) + 1;
            args = ARG_NEXT (args);
        }

        length += (strlen (FUNDEF_NAME (fun)) + strlen (MOD (FUNDEF_MOD (fun)))
                   + 2 * strlen (mod_name_con) + 6);

        new_name = (char *)Malloc (sizeof (char) * length);

        sprintf (new_name, "SAC%s%s%s%s", mod_name_con, MOD (FUNDEF_MOD (fun)),
                 mod_name_con, FUNDEF_NAME (fun));

        args = FUNDEF_ARGS (fun);

        while (args != NULL) {
            strcat (new_name, "_");
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

            FUNDEF_NAME (fun) = StringCopy (FUNDEF_LINKNAME (fun));
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

    if (MODUL_TYPES (arg_node) != NULL) {
        MODUL_TYPES (arg_node) = Trav (MODUL_TYPES (arg_node), arg_info);
    }

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
 *  functionname  : PRECtypedef
 *  arguments     : 1) N_typedef node
 *                  2) arg_info unused
 *  description   : renames types,
 *                  All types defined in SAC get the prefix "SAC_"
 *                  to avoid name clashes with C identifiers.
 *  global vars   : mod_name_con
 *  internal funs : ---
 *  external funs : Malloc, sprintf, strlen
 *  macros        : DBUG, FREE, TREE
 *
 *  remarks       :
 *
 */

node *
PRECtypedef (node *arg_node, node *arg_info)
{
    char *tmp;

    DBUG_ENTER ("PRECtypedef");

    if (TYPEDEF_MOD (arg_node) != NULL) {
        tmp = (char *)Malloc (sizeof (char)
                              * (strlen (TYPEDEF_NAME (arg_node))
                                 + strlen (MOD (TYPEDEF_MOD (arg_node)))
                                 + 2 * strlen (mod_name_con) + 5));
        sprintf (tmp, "SAC%s%s%s%s", mod_name_con, MOD (TYPEDEF_MOD (arg_node)),
                 mod_name_con, TYPEDEF_NAME (arg_node));

        FREE (TYPEDEF_NAME (arg_node));
        TYPEDEF_NAME (arg_node) = tmp;
        TYPEDEF_MOD (arg_node) = NULL;

        TYPEDEF_TYPE (arg_node) = RenameTypes (TYPEDEF_TYPE (arg_node));
    }

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        TYPEDEF_NEXT (arg_node) = Trav (TYPEDEF_NEXT (arg_node), arg_info);
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
 *                  Additionally, the object's type is renamed as well.
 *  global vars   : ---
 *  internal funs : RenameTypes
 *  external funs : Trav, Malloc, strcpy, strcat, MakeIds, MakeAp,
 *                  MakeLet, sprintf
 *  macros        : DBUG, TREE, FREE
 *
 *  remarks       :
 *
 */

node *
PRECobjdef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECobjdef");

    DBUG_PRINT ("PREC", ("precompiling object %s", ItemName (arg_node)));

    if (OBJDEF_MOD (arg_node) == NULL) {
        if (OBJDEF_LINKNAME (arg_node) != NULL) {
            FREE (OBJDEF_NAME (arg_node));
            OBJDEF_NAME (arg_node) = OBJDEF_LINKNAME (arg_node);
            FREE (OBJDEF_PRAGMA (arg_node));
        }
    } else {
        OBJDEF_TYPE (arg_node) = RenameTypes (OBJDEF_TYPE (arg_node));

        FREE (OBJDEF_NAME (arg_node));
        OBJDEF_NAME (arg_node) = (char *)Malloc (
          sizeof (char)
          * (strlen (OBJDEF_VARNAME (arg_node)) + strlen (mod_name_con) + 5));

        sprintf (OBJDEF_NAME (arg_node), "SAC%s%s", mod_name_con,
                 OBJDEF_VARNAME (arg_node));

        FREE (OBJDEF_VARNAME (arg_node));

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
        FUNDEF_RETURN (arg_node) = NULL;
    }

    /*
     *  The inline flag is set to 0
     */

    FUNDEF_INLINE (arg_node) = 0;

    /*
     *  The function body is traversed in order to remove artificial return
     *  values and parameters of function applications.
     */

    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_ASSERT ((FUNDEF_RETURN (arg_node) != NULL)
                       && (NODE_TYPE (FUNDEF_RETURN (arg_node)) == N_return),
                     ("N_fundef node has no reference to N_return node "));

        /*
         * The reference checked above is actually not needed by the
         * precompiler. This is done to check consistency of the syntax
         * tree for further compilation steps.
         */

        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    /*
     *  Now, traverse the following functions.
     *  All function bodies must be traversed before arguments and
     *  return values of functions are modified.
     */

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
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
        keep_name = FUNDEF_NAME (arg_node);
        keep_mod = FUNDEF_MOD (arg_node);
        keep_status = FUNDEF_STATUS (arg_node);
        keep_attrib = FUNDEF_ATTRIB (arg_node);

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

        FUNDEF_NAME (arg_node) = keep_name;
        FUNDEF_MOD (arg_node) = keep_mod;
        FUNDEF_STATUS (arg_node) = keep_status;
        FUNDEF_ATTRIB (arg_node) = keep_attrib;
    }

    /*
     *  The main function is extended by applications of the init functions
     *  of all global objects.
     *  All other functions are renamed in order to have separate name
     *  spaces for different modules.
     */

    if (strcmp (FUNDEF_NAME (arg_node), "main") == 0) {
        FUNDEF_BODY (arg_node)
          = InsertObjInits (FUNDEF_BODY (arg_node), MODUL_OBJS (arg_info));
    } else {
        if (FUNDEF_MOD (arg_node) == NULL) {
            FUNDEF_STATUS (arg_node) = ST_Cfun;
        }

        arg_node = RenameFun (arg_node);
        FUNDEF_TYPES (arg_node) = RenameTypes (FUNDEF_TYPES (arg_node));
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
        ARG_TYPESTRING (arg_node) = Type2String (ARG_TYPE (arg_node), 2);
        ARG_TYPE (arg_node) = RenameTypes (ARG_TYPE (arg_node));
        /*
         *  ARG_TYPESTRING is only used for renaming functions, so the
         *  type's actual name may be changed afterwards.
         */

        if (ARG_ATTRIB (arg_node) == ST_readonly_reference) {
            ARG_ATTRIB (arg_node) = ST_regular;
        } else {
            if (ARG_ATTRIB (arg_node) == ST_was_reference) {
                ARG_ATTRIB (arg_node) = ST_inout;
            }
        }

        if (ARG_NEXT (arg_node) != NULL) {
            ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PRECvardec
 *  arguments     : 1) N_vardec node
 *                  2) arg_info unused
 *  description   : renames types of declared variables
 *                  remove artificial variable declarations
 *  global vars   : ---
 *  internal funs : RenameTypes
 *  external funs : Trav
 *  macros        : TREE, DBUG
 *
 *  remarks       :
 *
 */

node *
PRECvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECvardec");

    if (VARDEC_STATUS (arg_node) == ST_artificial) {
        arg_node = FreeNode (arg_node);

        if (arg_node != NULL) {
            arg_node = Trav (arg_node, arg_info);
        }
    } else {
        VARDEC_TYPE (arg_node) = RenameTypes (VARDEC_TYPE (arg_node));

        if (VARDEC_NEXT (arg_node) != NULL) {
            VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PRECassign
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
PRECassign (node *arg_node, node *arg_info)
{
    node *instrs;

    DBUG_ENTER ("PRECassign");

    instrs = Trav (ASSIGN_INSTR (arg_node), NULL);

    ASSIGN_INSTR (arg_node) = instrs;
    if (instrs == NULL) {
        arg_node = FreeNode (arg_node);
        if (arg_node != NULL) {
            arg_node = Trav (arg_node, arg_info);
        }
    } else {
        if (ASSIGN_NEXT (arg_node) != NULL) {
            ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
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
    node *expr;

    DBUG_ENTER ("PREClet");

    let_ids = LET_IDS (arg_node);

    while ((let_ids != NULL) && (IDS_STATUS (let_ids) == ST_artificial)) {
        let_ids = FreeOneIds (let_ids);
    }

    LET_IDS (arg_node) = let_ids;

    expr = Trav (LET_EXPR (arg_node), arg_info);

    LET_EXPR (arg_node) = expr;
    if (expr == NULL) {
        arg_node = FreeTree (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PRECexprs_ap
 *  arguments     : 1) N_exprs chain of current parameters of a
 *                     function application
 *                  2) N_arg chain of formal parameters of the
 *                     respective function definition
 *  description   : removes all artificial parameters.
 *                  The status of those current parameters which belong
 *                  to formal reference parameters is modified to ST_inout.
 *                  Global objects given as parameters to the applied
 *                  function get a reference to the object definition
 *                  and are renamed with the new name of the global object.
 *  global vars   : ---
 *  internal funs : PRECexprs_ap
 *  external funs : ---
 *  macros        : TREE, DBUG
 *
 *  remarks       :
 *
 */

node *
PRECexprs_ap (node *current, node *formal)
{
    node *expr;

    DBUG_ENTER ("PRECexprs_ap");

    if (EXPRS_NEXT (current) != NULL) {
        EXPRS_NEXT (current)
          = PRECexprs_ap (EXPRS_NEXT (current),
                          ARG_BASETYPE (formal) == T_dots ? formal : ARG_NEXT (formal));
    }

    expr = EXPRS_EXPR (current);

    if (NODE_TYPE (expr) == N_id) {
        if (ID_STATUS (expr) == ST_artificial) {
            current = FreeNode (current);
        } else {
            if (ARG_ATTRIB (formal) == ST_was_reference) {
                ID_ATTRIB (expr) = ST_inout;
            }

            if ((NODE_TYPE (ID_VARDEC (expr)) == N_arg)
                && (ARG_STATUS (ID_VARDEC (expr)) == ST_artificial)) {
                ID_VARDEC (expr) = ARG_OBJDEF (ID_VARDEC (expr));
                ID_NAME (expr) = OBJDEF_NAME (ID_VARDEC (expr));
            }

            if ((NODE_TYPE (ID_VARDEC (expr)) == N_vardec)
                && (VARDEC_STATUS (ID_VARDEC (expr)) == ST_artificial)) {
                ID_VARDEC (expr) = VARDEC_OBJDEF (ID_VARDEC (expr));
                ID_NAME (expr) = OBJDEF_NAME (ID_VARDEC (expr));
            }
        }
    }

    DBUG_RETURN (current);
}

/*
 *
 *  functionname  : PRECexprs_return
 *  arguments     : 1) N_exprs chain of an N_return node
 *                  2) respective N_return node
 *  description   : removes all artificial return values from the chain.
 *                  A new chain is built up for all those return values
 *                  which belong to original reference parameters.
 *                  These are stored in RETURN_REFERENCE for later use
 *                  in compile.c
 *  global vars   : ---
 *  internal funs : PRECexprs_return
 *  external funs : ---
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

node *
PRECexprs_return (node *ret_exprs, node *ret_node)
{
    node *tmp;

    DBUG_ENTER ("PRECexprs_return");

    if (EXPRS_NEXT (ret_exprs) != NULL) {
        EXPRS_NEXT (ret_exprs) = PRECexprs_return (EXPRS_NEXT (ret_exprs), ret_node);
    }

    if (ID_STATUS (EXPRS_EXPR (ret_exprs)) == ST_artificial) {
        if (ARG_STATUS (ID_VARDEC (EXPRS_EXPR (ret_exprs))) == ST_artificial) {
            /*
             *  This artificial return value belongs to a global object,
             *  so it can be removed.
             */

            ret_exprs = FreeNode (ret_exprs);
        } else {
            /*
             *  This artificial return value belongs to an original reference
             *  parameter, so it is stored in RETURN_REFERENCE to be compiled
             *  to an "inout" parameter.
             */

            tmp = ret_exprs;
            ret_exprs = EXPRS_NEXT (ret_exprs);
            EXPRS_NEXT (tmp) = RETURN_REFERENCE (ret_node);
            RETURN_REFERENCE (ret_node) = tmp;
        }
    }

    DBUG_RETURN (ret_exprs);
}

/*
 *
 *  functionname  : PRECap
 *  arguments     : 1) N_ap node
 *                  2) arg_info unused
 *  description   : traverses the current arguments using function
 *                  PRECexprs_ap that is given a pointer
 *                  to the first formal parameter of the applied function.
 *  global vars   : ---
 *  internal funs : PRECexprs_ap
 *  external funs : ---
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

node *
PRECap (node *arg_node, node *arg_info)
{
    node *ap;

    DBUG_ENTER ("PRECap");

    if (FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_classfun) {
        ap = arg_node;
        arg_node = EXPRS_EXPR (AP_ARGS (arg_node));

        if (0 == strncmp (AP_NAME (ap), "to_", 3)) {
            if (NODE_TYPE (arg_node) == N_id) {
                if ((ID_REFCNT (arg_node) != -1)
                    && (!IsUnique (VARDEC_TYPE (ID_VARDEC (arg_node))))) {
                    /*
                     *  The base type of the class is refcounted,
                     *  so we have to make the boxed value unique.
                     */

                    ID_MAKEUNIQUE (arg_node) = 1;
                } else {
                    ID_MAKEUNIQUE (arg_node) = 0;
                }
            }
        } else {
            /*
             *  This must be a "from" function. So, the argument is of a class
             *  type which implies that it is am identifier.
             */

            ID_MAKEUNIQUE (arg_node) = 0;
        }

        FREE (AP_NAME (ap));
        FREE (ap);
    } else {
        if (AP_ARGS (arg_node) != NULL) {
            AP_ARGS (arg_node)
              = PRECexprs_ap (AP_ARGS (arg_node), FUNDEF_ARGS (AP_FUNDEF (arg_node)));
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PRECreturn
 *  arguments     : 1) N_return node
 *                  2) arg_info unused
 *  description   : traverses the return values using function
 *                  PRECexprs_return.
 *  global vars   : ---
 *  internal funs : PRECexprs_return
 *  external funs : ---
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
        RETURN_EXPRS (arg_node) = PRECexprs_return (RETURN_EXPRS (arg_node), arg_node);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : PRECid
 *  arguments     : 1) N_id node;
 *                  2) arg_info unused
 *  description   : Applied occurrences of global objects may be renamed,
 *                  if the global object was renamed.
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

    if (ID_STATUS (arg_node) == ST_artificial) {
        arg_node = FreeTree (arg_node);
    } else {
        if ((NODE_TYPE (ID_VARDEC (arg_node)) == N_arg)
            && (ARG_STATUS (ID_VARDEC (arg_node)) == ST_artificial)) {
            ID_VARDEC (arg_node) = ARG_OBJDEF (ID_VARDEC (arg_node));
            ID_NAME (arg_node) = OBJDEF_NAME (ID_VARDEC (arg_node));
        }

        if ((NODE_TYPE (ID_VARDEC (arg_node)) == N_vardec)
            && (VARDEC_STATUS (ID_VARDEC (arg_node)) == ST_artificial)) {
            ID_VARDEC (arg_node) = VARDEC_OBJDEF (ID_VARDEC (arg_node));
            ID_NAME (arg_node) = OBJDEF_NAME (ID_VARDEC (arg_node));
        }

        ID_MAKEUNIQUE (arg_node) = 0;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * precompilaton of new with-loop
 *
 */

/******************************************************************************
 *
 * function:
 *   int CompareWLnode(node *node1, node *node2, int outline)
 *
 * description:
 *   compares the N_WL...-nodes 'node1' and 'node2' IN ALL DIMS.
 *   eventually present next nodes in 'node1' or 'node2' are ignored.
 *
 *   if (outline > 0) ALL GRID DATA IS IGNORED!!!
 *   (this feature is used by 'ComputeCubes', to determine whether two strides
 *    lie in the same cube or not)
 *
 *   this function definies the sort order for InsertWLnodes.
 *
 *   return: -2 => outline('node1') < outline('node2')
 *           -1 => outline('node1') = outline('node2'), 'node1' < 'node2'
 *            0 => 'node1' = 'node2'
 *            1 => outline('node1') = outline('node2'), 'node1' > 'node2'
 *            2 => outline('node1') > outline('node2')
 *
 ******************************************************************************/

int
CompareWLnode (node *node1, node *node2, int outline)
{
    node *grid1, *grid2;
    int result, grid_result;

    DBUG_ENTER ("CompareWLnode");

    if ((node1 != NULL) && (node2 != NULL)) {

        DBUG_ASSERT ((NODE_TYPE (node1) == NODE_TYPE (node2)),
                     "can not compare object of different type");

        /* compare the bounds first */
        COMP_BEGIN (WLNODE_BOUND1 (node1), WLNODE_BOUND1 (node2), result, 2)
        COMP_BEGIN (WLNODE_BOUND2 (node1), WLNODE_BOUND2 (node2), result, 2)

        switch (NODE_TYPE (node1)) {

        case N_WLblock:

            result
              = CompareWLnode (WLBLOCK_NEXTDIM (node1), WLBLOCK_NEXTDIM (node2), outline);
            break;

        case N_WLublock:

            result = CompareWLnode (WLUBLOCK_NEXTDIM (node1), WLUBLOCK_NEXTDIM (node2),
                                    outline);
            break;

        case N_WLstride:

            grid1 = WLSTRIDE_CONTENTS (node1);
            DBUG_ASSERT ((grid1 != NULL), "no grid found");
            grid2 = WLSTRIDE_CONTENTS (node2);
            DBUG_ASSERT ((grid2 != NULL), "no grid found");

            if (outline == 0) {
                /* compare grid, but leave 'result' untouched until later dimensions are
                 * checked! */
                COMP_BEGIN (WLGRID_BOUND1 (grid1), WLGRID_BOUND1 (grid2), grid_result, 1)
                COMP_BEGIN (WLGRID_BOUND2 (grid1), WLGRID_BOUND2 (grid2), grid_result, 1)
                grid_result = 0;
                COMP_END
                COMP_END

                /* compare later dimensions */
                result = CompareWLnode (WLGRID_NEXTDIM (grid1), WLGRID_NEXTDIM (grid2),
                                        outline);

                /* the 'grid_result' value is important only if the outlines are equal */
                if (abs (result) != 2) {
                    result = grid_result;
                }
            } else {
                /* compare outlines only -> skip grid */
                result = CompareWLnode (WLGRID_NEXTDIM (grid1), WLGRID_NEXTDIM (grid2),
                                        outline);
            }

            break;

        default:

            DBUG_ASSERT ((0), "wrong node type");
        }

        COMP_END
        COMP_END

    } else {

        if ((node1 == NULL) && (node2 == NULL)) {
            result = 0;
        } else {
            if (node2 == NULL) {
                result = 2;
            } else {
                result = -2;
            }
        }
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *InsertWLnodes(node *nodes, node *insert_nodes)
 *
 * description:
 *   inserts all elements of the chain 'insert_nodes' into the sorted chain
 *     'nodes'.
 *   all elements of 'insert_nodes' that exist already in 'nodes' are freed.
 *   uses function 'CompareWLnode' to sort the elements.
 *
 *   insert_nodes: (unsorted) chain of N_WL...-nodes
 *   nodes:        sorted chain of N_WL...-nodes
 *   return:       sorted chain of N_WL...-nodes containing all the data of
 *                   'nodes' and 'insert_nodes'
 *
 ******************************************************************************/

node *
InsertWLnodes (node *nodes, node *insert_nodes)
{
    node *tmp, *insert_here;
    int compare;

    DBUG_ENTER ("InsertWLnodes");

    /*
     * insert all elements of 'insert_nodes' in 'nodes'
     */
    while (insert_nodes != NULL) {

        /* compare the first element to insert with the first element in 'nodes' */
        compare = CompareWLnode (insert_nodes, nodes, 0);

        if ((nodes == NULL) || (compare < 0)) {
            /* insert current element of 'insert_nodes' at head of 'nodes' */
            tmp = insert_nodes;
            insert_nodes = WLNODE_NEXT (insert_nodes);
            WLNODE_NEXT (tmp) = nodes;
            nodes = tmp;
        } else {

            /* search for insert-position in 'nodes' */
            insert_here = nodes;
            while ((compare > 0) && (WLNODE_NEXT (insert_here) != NULL)) {
                compare = CompareWLnode (insert_nodes, WLNODE_NEXT (insert_here), 0);

                if (compare > 0) {
                    insert_here = WLNODE_NEXT (insert_here);
                }
            }

            if (compare == 0) {
                /* current element of 'insert_nodes' exists already -> free it */
                insert_nodes = FreeNode (insert_nodes);
            } else {
                /* insert current element of 'insert_nodes' after the found position */
                tmp = insert_nodes;
                insert_nodes = WLNODE_NEXT (insert_nodes);
                WLNODE_NEXT (tmp) = WLNODE_NEXT (insert_here);
                WLNODE_NEXT (insert_here) = tmp;
            }
        }
    }

    DBUG_RETURN (nodes);
}

/******************************************************************************
 *
 * function:
 *   node *NormalizeStride_1(node *stride)
 *
 * description:
 *   returns the IN THE FIRST DIMENSION normalized N_WLstride-node 'stride'.
 *   a eventually present next node in 'stride' is ignored.
 *
 *   this normalization has two major goals:
 *     - every stride has a unambiguous form
 *        -> two strides represent the same index set
 *           if and only if there attribute values are equal.
 *     - maximize the outline of strides
 *        -> two strides of the same cube do not split each other in several
 *           parts when intersected
 *
 ******************************************************************************/

node *
NormalizeStride_1 (node *stride)
{
    node *grid;
    int bound1, bound2, step, grid_b1, grid_b2, new_bound1, new_bound2;

    DBUG_ENTER ("NormalizeStride_1");

    grid = WLSTRIDE_CONTENTS (stride);
    DBUG_ASSERT ((grid != NULL), "grid not found");
    DBUG_ASSERT ((WLGRID_NEXT (grid) == NULL), "more than one grid found");

    bound1 = WLSTRIDE_BOUND1 (stride);
    bound2 = WLSTRIDE_BOUND2 (stride);
    step = WLSTRIDE_STEP (stride);
    grid_b1 = WLGRID_BOUND1 (grid);
    grid_b2 = WLGRID_BOUND2 (grid);

    /*
     * assure: ([grid_b1; grid_b2] < [0; step]) or (grid_b2 = step = 1);
     * in other terms: (width < step) or (width = step = 1)
     */

    if (grid_b2 > step) {
        grid_b2 = step;
    }
    if ((step > 1) && (grid_b1 == 0) && (grid_b2 == step)) {
        grid_b2 = step = 1;
    }

    /*
     * maximize the outline
     */

    /* calculate minimum for 'bound1' */
    new_bound1 = bound1 - (step - grid_b2);
    new_bound1 = MAX (0, new_bound1);

    /* calculate maximum for 'bound2' */
    if ((bound2 - bound1 - grid_b1) % step >= grid_b2 - grid_b1) {
        new_bound2 = bound2 + step - ((bound2 - bound1 - grid_b1) % step);
    } else {
        new_bound2 = bound2;
    }

    WLSTRIDE_BOUND1 (stride) = new_bound1;
    WLSTRIDE_BOUND2 (stride) = new_bound2;
    WLSTRIDE_STEP (stride) = step;
    WLGRID_BOUND1 (grid) = grid_b1 + (bound1 - new_bound1);
    WLGRID_BOUND2 (grid) = grid_b2 + (bound1 - new_bound1);

    DBUG_RETURN (stride);
}

/******************************************************************************
 *
 * function:
 *   node* Parts2Strides(node *parts)
 *
 * description:
 *   converts a N_Npart-chain ('parts') into a N_WLstride-chain (return).
 *
 ******************************************************************************/

node *
Parts2Strides (node *parts)
{
    node *parts_stride, *strides, *new_stride, *last_grid, *gen, *bound1, *bound2, *step,
      *width;
    int dim;

    DBUG_ENTER ("Parts2Strides");

    parts_stride = NULL;

    while (parts != NULL) {
        strides = NULL;

        gen = NPART_GEN (parts);
        DBUG_ASSERT ((NGEN_OP1 (gen) == F_le), "op1 in generator is not <=");
        DBUG_ASSERT ((NGEN_OP2 (gen) == F_lt), "op2 in generator is not <");

        /* get components of current generator */
        bound1 = ARRAY_AELEMS (NGEN_BOUND1 (gen));
        bound2 = ARRAY_AELEMS (NGEN_BOUND2 (gen));
        step = ARRAY_AELEMS (NGEN_STEP (gen));
        width = ARRAY_AELEMS (NGEN_WIDTH (gen));

        dim = 0;
        while (bound1 != NULL) {
            DBUG_ASSERT ((bound2 != NULL), "bound2 of generator not complete");
            DBUG_ASSERT ((step != NULL), "step of generator not complete");
            DBUG_ASSERT ((width != NULL), "width of generator not complete");

            /* build N_WLstride-node of current dimension */
            new_stride = MakeWLstride (0, dim, NUM_VAL (EXPRS_EXPR (bound1)),
                                       NUM_VAL (EXPRS_EXPR (bound2)),
                                       NUM_VAL (EXPRS_EXPR (step)), 0,
                                       MakeWLgrid (dim, 0, NUM_VAL (EXPRS_EXPR (width)),
                                                   0, NULL, NULL, NULL),
                                       NULL);
            WLSTRIDE_PART (new_stride)
              = parts; /* this information is needed by 'IntersectOutline' */
            new_stride = NormalizeStride_1 (new_stride);

            /* append 'new_stride' to 'strides'-chain */
            if (strides == NULL) {
                strides = new_stride;
            } else {
                WLGRID_NEXTDIM (last_grid) = new_stride;
            }
            last_grid = WLSTRIDE_CONTENTS (new_stride);

            /* go to next dim */
            bound1 = EXPRS_NEXT (bound1);
            bound2 = EXPRS_NEXT (bound2);
            step = EXPRS_NEXT (step);
            width = EXPRS_NEXT (width);
            dim++;
        }

        WLGRID_CODE (last_grid) = NPART_CODE (parts);
        parts_stride = InsertWLnodes (parts_stride, strides);

        parts = NPART_NEXT (parts);
    }

    DBUG_RETURN (parts_stride);
}

/******************************************************************************
 *
 * function:
 *   int IndexHead(int bound1, int grid_b1)
 *
 * description:
 *   computes the maximum for 'bound1':
 *     returns the index position of the first element of the grid
 *
 ******************************************************************************/

int
IndexHead (int bound1, int grid_b1)
{
    int result;

    DBUG_ENTER ("IndexHead");

    result = bound1 + grid_b1;

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   int IndexRear(int bound1, int bound2, int step, int grid_b1, int grid_b2)
 *
 * description:
 *   computes the minimum for 'bound2':
 *     returns the index position '+ 1' of the last element of the grid
 *
 ******************************************************************************/

int
IndexRear (int bound1, int bound2, int step, int grid_b1, int grid_b2)
{
    int result;

    DBUG_ENTER ("IndexRear");

    result
      = bound2
        - MAX (0, ((bound2 - bound1 - grid_b1 - 1) % step) + 1 - (grid_b2 - grid_b1));

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   int GridOffset(int new_bound1,
 *                  int bound1, int step, int grid_b1, int grid_b2)
 *
 * description:
 *   computes a offset for a grid relating to 'new_bound1':
 *     what happens to the bounds of a grid if 'new_bound1' is the new
 *     upper bound for the related stride?
 *     the new bounds of the grid are:
 *       grid_b1 - offset, grid_b2 - offset
 *   CAUTION: if (offset > grid_b1) the grid must be devided in two parts!!
 *
 ******************************************************************************/

int
GridOffset (int new_bound1, int bound1, int step, int grid_b2)
{
    int offset;

    DBUG_ENTER ("GridOffset");

    offset = (new_bound1 - bound1) % step;

    if (offset >= grid_b2) {
        offset -= step;
    }

    DBUG_RETURN (offset);
}

/******************************************************************************
 *
 * function:
 *   void IntersectOutline(node *stride1, node *stride2,
 *                         node **i_stride1, node **i_stride2)
 *
 * description:
 *   returns in 'i_stride1' and 'i_stride2' the part of 'stride1', 'stride2'
 *     respectively that lies in a common cube.
 *   eventually present next nodes in 'stride1' or 'stride2' are ignored.
 *
 ******************************************************************************/

void
IntersectOutline (node *stride1, node *stride2, node **i_stride1, node **i_stride2)
{
    node *grid1, *grid2, *new_i_stride1, *new_i_stride2;
    int bound11, bound21, step1, grid1_b1, grid1_b2, bound12, bound22, step2, grid2_b1,
      grid2_b2, head1, rear1, head2, rear2, i_bound1, i_bound2, i_offset1, i_offset2;
    int flag = 0;

    DBUG_ENTER ("IntersectOutline");

    new_i_stride1 = *i_stride1 = DupNode (stride1);
    new_i_stride2 = *i_stride2 = DupNode (stride2);

    while (stride1 != NULL) {
        DBUG_ASSERT ((stride2 != NULL), "dim not found");

        DBUG_ASSERT ((WLSTRIDE_PART (stride1) != NULL), "no part found");
        DBUG_ASSERT ((WLSTRIDE_PART (stride2) != NULL), "no part found");

        grid1 = WLSTRIDE_CONTENTS (stride1);
        DBUG_ASSERT ((grid1 != NULL), "grid not found");
        DBUG_ASSERT ((WLGRID_NEXT (grid1) == NULL), "more than one grid found");
        grid2 = WLSTRIDE_CONTENTS (stride2);
        DBUG_ASSERT ((grid2 != NULL), "grid not found");
        DBUG_ASSERT ((WLGRID_NEXT (grid2) == NULL), "more than one grid found");

        bound11 = WLSTRIDE_BOUND1 (stride1);
        bound21 = WLSTRIDE_BOUND2 (stride1);
        step1 = WLSTRIDE_STEP (stride1);
        grid1_b1 = WLGRID_BOUND1 (grid1);
        grid1_b2 = WLGRID_BOUND2 (grid1);

        bound12 = WLSTRIDE_BOUND1 (stride2);
        bound22 = WLSTRIDE_BOUND2 (stride2);
        step2 = WLSTRIDE_STEP (stride2);
        grid2_b1 = WLGRID_BOUND1 (grid2);
        grid2_b2 = WLGRID_BOUND2 (grid2);

        head1 = IndexHead (bound11, grid1_b1);
        rear1 = IndexRear (bound11, bound21, step1, grid1_b1, grid1_b2);
        head2 = IndexHead (bound12, grid2_b1);
        rear2 = IndexRear (bound12, bound22, step2, grid2_b1, grid2_b2);

        i_bound1 = MAX (bound11, bound12);
        i_bound2 = MIN (bound21, bound22);

        i_offset1 = GridOffset (i_bound1, bound11, step1, grid1_b2);
        i_offset2 = GridOffset (i_bound1, bound12, step2, grid2_b2);

        if ((head1 < rear2) && (head2 < rear1) &&
            /* are the outlines of 'stride1' and 'stride2' not disjunkt? */
            (i_offset1 <= grid1_b1) && (i_offset2 <= grid2_b1)) {
            /* are the grids compatible? */

            if ((WLSTRIDE_PART (stride1) == WLSTRIDE_PART (stride2)) &&
                /* are 'stride1' and 'stride2' descended from the same Npart? */
                (!flag)) {
                /* we should deal with this exception only once !! */

                /*
                 * example: 0->6  step 3, 0->1: op1
                 *          0->16 step 3, 1->3: op2
                 *          4->20 step 3, 2->3: op3
                 *         ------------------------- after first round with
                 * IntersectOutline: 0->7  step 3, 1->3: op2  <- intersection of 'op2' and
                 * outline('op1') 3->16 step 3, 1->3: op2  <- intersection of 'op2' and
                 * outline('op3')
                 *
                 *    these two strides are NOT DISJUNCT !!! but they are part of the same
                 * Npart !!
                 */

                flag = 1; /* skip this exception handling in later dimensions! */

                /* modify the bounds of the first stride, so that the new outlines are
                 * disjunct */
                if (WLSTRIDE_BOUND2 (stride1) < WLSTRIDE_BOUND2 (stride2)) {
                    WLSTRIDE_BOUND2 (new_i_stride1) = i_bound1;
                    new_i_stride1 = NormalizeStride_1 (new_i_stride1);
                } else {
                    WLSTRIDE_BOUND2 (new_i_stride2) = i_bound1;
                    new_i_stride2 = NormalizeStride_1 (new_i_stride2);
                }

            } else {

                /* intersect 'stride1' with the outline of 'stride2' */
                WLSTRIDE_BOUND1 (new_i_stride1) = i_bound1;
                WLSTRIDE_BOUND2 (new_i_stride1) = i_bound2;
                WLGRID_BOUND1 (WLSTRIDE_CONTENTS (new_i_stride1)) = grid1_b1 - i_offset1;
                WLGRID_BOUND2 (WLSTRIDE_CONTENTS (new_i_stride1)) = grid1_b2 - i_offset1;
                new_i_stride1 = NormalizeStride_1 (new_i_stride1);

                /* intersect 'stride2' with the outline of 'stride1' */
                WLSTRIDE_BOUND1 (new_i_stride2) = i_bound1;
                WLSTRIDE_BOUND2 (new_i_stride2) = i_bound2;
                WLGRID_BOUND1 (WLSTRIDE_CONTENTS (new_i_stride2)) = grid2_b1 - i_offset2;
                WLGRID_BOUND2 (WLSTRIDE_CONTENTS (new_i_stride2)) = grid2_b2 - i_offset2;
                new_i_stride2 = NormalizeStride_1 (new_i_stride2);
            }

        } else {
            /*
             * intersection is empty
             *  -> free the useless data in 'i_stride1', 'i_stride2'
             */
            if (*i_stride1 != NULL) {
                *i_stride1 = FreeTree (*i_stride1);
            }
            if (*i_stride2 != NULL) {
                *i_stride2 = FreeTree (*i_stride2);
            }

            /* we can give up here */
            break;
        }

        /* next dim */
        stride1 = WLGRID_NEXTDIM (grid1);
        stride2 = WLGRID_NEXTDIM (grid2);
        new_i_stride1 = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (new_i_stride1));
        new_i_stride2 = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (new_i_stride2));
    }
}

/******************************************************************************
 *
 * function:
 *   node *MergeCube(node *stride1, node *stride2)
 *
 * description:
 *   merges 'stride2' into 'stride1'
 *     (copies needed parts of 'stride2' into 'stride1').
 *   eventually present next nodes in 'stride1' or 'stride2' are ignored.
 *
 ******************************************************************************/

node *
MergeCube (node *stride1, node *stride2)
{
    node *grid1, *grid2, *new_grid, *tmp;
    int bound11, bound12, offset;

    DBUG_ENTER ("MergeCube");

    if (stride1 != NULL) {

        grid1 = WLSTRIDE_CONTENTS (stride1);
        grid2 = WLSTRIDE_CONTENTS (stride2);

        bound11 = WLSTRIDE_BOUND1 (stride1);
        bound12 = WLSTRIDE_BOUND1 (stride2);

        WLSTRIDE_BOUND1 (stride1) = MAX (bound11, bound12);
        WLSTRIDE_BOUND2 (stride1)
          = MIN (WLSTRIDE_BOUND2 (stride1), WLSTRIDE_BOUND2 (stride2));

        /*
         * compute new offset for 'grid2'
         */
        offset = GridOffset (WLSTRIDE_BOUND1 (stride1), bound12, WLSTRIDE_STEP (stride2),
                             WLGRID_BOUND2 (grid2));

        WLSTRIDE_BOUND1 (stride1) -= offset;
        WLSTRIDE_BOUND2 (stride1) -= offset;

        /*
         * compute new offsets for all grids in 'stride1'
         */
        tmp = grid1;
        do {
            offset = GridOffset (WLSTRIDE_BOUND1 (stride1), bound11,
                                 WLSTRIDE_STEP (stride1), WLGRID_BOUND2 (tmp));
            WLGRID_BOUND1 (tmp) -= offset;
            WLGRID_BOUND2 (tmp) -= offset;

            tmp = WLGRID_NEXT (tmp);
        } while (tmp != NULL);

        /*
         * insert 'grid2'
         */
        if (WLGRID_BOUND1 (grid1) > WLGRID_BOUND1 (grid2)) {
            /* insert 'grid2' at head of grid list */
            DBUG_ASSERT ((WLGRID_BOUND1 (grid1) >= WLGRID_BOUND2 (grid2)),
                         "wrong offset");
            new_grid = DupTree (grid2, NULL);
            WLGRID_NEXT (new_grid) = grid1;
            WLSTRIDE_CONTENTS (stride1) = new_grid;
        } else {
            /* search for correct position for insertion */
            tmp = grid1;
            while ((WLGRID_NEXT (tmp) != NULL)
                   && (WLGRID_BOUND1 (tmp) != WLGRID_BOUND1 (grid2))
                   && (WLGRID_BOUND1 (WLGRID_NEXT (tmp)) <= WLGRID_BOUND1 (grid2))) {
                tmp = WLGRID_NEXT (tmp);
            }

            if (WLGRID_BOUND1 (tmp) == WLGRID_BOUND1 (grid2)) {
                /* range of 'grid2' is already in grid list -> merge next dim */
                WLGRID_NEXTDIM (tmp)
                  = MergeCube (WLGRID_NEXTDIM (tmp), WLGRID_NEXTDIM (grid2));
            } else {
                /* insert 'grid2' after 'tmp' */
                if (WLGRID_NEXT (tmp) != NULL) {
                    DBUG_ASSERT ((WLGRID_BOUND1 (WLGRID_NEXT (tmp))
                                  >= WLGRID_BOUND2 (grid2)),
                                 "wrong offset");
                }
                new_grid = DupTree (grid2, NULL);
                WLGRID_NEXT (new_grid) = WLGRID_NEXT (tmp);
                WLGRID_NEXT (tmp) = new_grid;
            }
        }
    }

    DBUG_RETURN (stride1);
}

#if 0 

    /* equalize steps */
    while (tmp != NULL) {
      offset = NUM_VAL(INDEX_BOUND1(tmp));
      step = NUM_VAL(INDEX_STEP(tmp));
      width = NUM_VAL(INDEX_WIDTH(tmp));

      count_new_stride = new_step / NUM_VAL(INDEX_STEP(tmp));
      NUM_VAL(INDEX_STEP(tmp)) = new_step;
      for (i = 1; i < count_new_stride; i++) {
        append = INDEX_NEXT(append)
          = MakeIndex(MakeNum(bound1), MakeNum(bound2),
                      MakeNum(offset + i * step),
                      MakeNum(new_step), MakeNum(width),
                      NULL);
      }

      tmp = INDEX_NEXT(tmp);
    }


        /* must 'grid1' be split into two parts? */
        if (i_offset1 > grid1_b1) {
          new_i_stride1 = MakeWLstride(WLSTRIDE_LEVEL(stride1),
                                  WLSTRIDE_DIM(stride1),
                                  i_bound1,
                                  i_bound2,
                                  step1,
                                  WLSTRIDE_UNROLLING(stride1),
                                  MakeWLgrid(WLGRID_DIM(grid1),
                                             0,
                                             grid1_b2 - i_offset1,
                                             WLGRID_UNROLLING(grid1),
                                             NULL,
                                             NULL,
                                             NULL),
                                  NULL);
          WLSTRIDE_PART(new_i_stride1) = WLSTRIDE_PART(stride1);
          new_i_stride1 = NormalizeStride_1(new_i_stride1);

          new_i_stride1 = MakeWLstride(WLSTRIDE_LEVEL(stride1),
                                  WLSTRIDE_DIM(stride1),
                                  i_bound1,
                                  i_bound2,
                                  step1,
                                  WLSTRIDE_UNROLLING(stride1),
                                  MakeWLgrid(WLGRID_DIM(grid1),
                                             grid1_b1 - (i_offset1 - step1),
                                             step1,
                                             WLGRID_UNROLLING(grid1),
                                             NULL,
                                             NULL,
                                             NULL),
                                  new_i_stride1);
          WLSTRIDE_PART(new_i_stride1) = WLSTRIDE_PART(stride1);
          new_i_stride1 = NormalizeStride_1(new_i_stride1);
        }
#endif

/******************************************************************************
 *
 * function:
 *   node *ComputeCubes(node *strides)
 *
 * description:
 *   returns the set of cubes as a N_WLstride-chain
 *
 ******************************************************************************/

node *
ComputeCubes (node *strides)
{
    node *tmp, *new_strides, *new_stride1, *last_stride2, *i_stride1, *i_stride2,
      *stride1, *stride2;
    int fixpoint;

    DBUG_ENTER ("ComputeCubes");

    /*
     * step 1: create disjunct outlines
     *          -> every stride lies in one and only one cube
     */
    do {
        fixpoint = 1;
        new_strides = NULL;

        /* check WLSTRIDE_MODIFIED */
        stride1 = strides;
        while (stride1 != NULL) {
            DBUG_ASSERT ((WLSTRIDE_MODIFIED (stride1) == 0), "stride was modified");
            stride1 = WLSTRIDE_NEXT (stride1);
        }

        /* intersect the elements of 'strides' in pairs */
        stride1 = strides;
        while (stride1 != NULL) {

            stride2 = WLSTRIDE_NEXT (stride1);
            while (stride2 != NULL) {

                /* intersect outlines of 'stride1' and 'stride2' */
                IntersectOutline (stride1, stride2, &i_stride1, &i_stride2);
                if ((i_stride1 != NULL) && (CompareWLnode (stride1, i_stride1, 1) != 0)) {
                    fixpoint = 0;
                    WLSTRIDE_MODIFIED (stride1) = 1;
                    new_strides = InsertWLnodes (new_strides, i_stride1);
                }
                if ((i_stride2 != NULL) && (CompareWLnode (stride2, i_stride2, 1) != 0)) {
                    fixpoint = 0;
                    WLSTRIDE_MODIFIED (stride2) = 1;
                    new_strides = InsertWLnodes (new_strides, i_stride2);
                }

                stride2 = WLSTRIDE_NEXT (stride2);
            }

            /* have 'stride1' only empty intersections with the others? */
            if (WLSTRIDE_MODIFIED (stride1) == 0) {
                /* insert 'stride1' in 'new_strides' */
                tmp = stride1;
                stride1 = WLSTRIDE_NEXT (stride1);
                WLSTRIDE_NEXT (tmp) = NULL;
                new_strides = InsertWLnodes (new_strides, tmp);
            } else {
                /* 'stride1' is no longer needed */
                stride1 = FreeNode (stride1);
                /* 'stride1' points now to his successor!! */
            }
        }

        strides = new_strides;
    } while (!fixpoint);

    /*
     * step 2: merge all strides that lie in the same cube -> set of cubes
     */
    new_strides = NULL;

    stride1 = strides;
    while (stride1 != NULL) {

        last_stride2 = stride1;
        stride2 = WLSTRIDE_NEXT (stride1);

        /* duplicate first node of 'stride1' */
        new_stride1 = DupNode (stride1);

#if 0
    /*
     * collect all strides, that lie in the same cube as 'stride1', in 'new_stride1'
     */
    while (stride2 != NULL) {
      /* intersect outlines of 'stride1' and 'stride2' */
      IntersectOutline(stride1, stride2, &i_stride1, &i_stride2);
      if (CompareWLnode(stride1, i_stride1, 1) == 0) {
        DBUG_ASSERT((CompareWLnode(stride2, i_stride2, 1) == 0), "wrong outline found");

        /* remove 'stride2' from chain */
        WLSTRIDE_NEXT(last_stride2) = WLSTRIDE_NEXT(stride2);

        /* insert first stride of 'stride2' into 'new_stride1' */
        WLSTRIDE_NEXT(stride2) = NULL;
        new_stride1 = InsertWLnodes(new_stride1, stride2);
      }
      else {
        last_stride2 = stride2;              /* save last stride */
      }
      stride2 = WLSTRIDE_NEXT(last_stride2);
    }

    /*
     * compute step value for cube 'new_stride1'
     */

    (...)
#endif

        while (stride2 != NULL) {
            /* intersect outlines of 'stride1' and 'stride2' */
            IntersectOutline (stride1, stride2, &i_stride1, &i_stride2);
            if (CompareWLnode (stride1, i_stride1, 1) == 0) {
                DBUG_ASSERT ((CompareWLnode (stride2, i_stride2, 1) == 0),
                             "wrong outline found");

                /* remove 'stride2' from chain */
                WLSTRIDE_NEXT (last_stride2) = WLSTRIDE_NEXT (stride2);
                /* merge 'stride2' with 'stride1' */
                new_stride1 = MergeCube (new_stride1, stride2);

                /* data of 'stride2' is no longer needed */
                stride2 = FreeNode (stride2);
                /* 'stride2' points now to his successor!! */
            } else {
                last_stride2 = stride2;            /* save last stride */
                stride2 = WLSTRIDE_NEXT (stride2); /* go to next stride */
            }
        }

        new_strides = InsertWLnodes (new_strides, new_stride1);

        /* data of 'stride1' no longer needed */
        stride1 = FreeNode (stride1);
        /* 'stride1' points now to his successor!! */
    }
    strides = new_strides;

    DBUG_RETURN (strides);
}

/******************************************************************************
 *
 * function:
 *   long *CalcSV(node *stride, long *sv)
 *
 * description:
 *   calculates the lcm of all grid-steps in N_WLstride-N_WLgrid-chain 'stride'
 *
 ******************************************************************************/

long *
CalcSV (node *stride, long *sv)
{
    node *grid;

    DBUG_ENTER ("CalcSV");

    if (stride != NULL) {

        /* initialize sv */
        if (WLSTRIDE_DIM (stride) == 0) {
            sv[0] = 1;
        }
        if (WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (stride)) != NULL) {
            sv[WLSTRIDE_DIM (stride) + 1] = 1;
        }

        do {
            sv[WLSTRIDE_DIM (stride)]
              = lcm (sv[WLSTRIDE_DIM (stride)], WLSTRIDE_STEP (stride));

            grid = WLSTRIDE_CONTENTS (stride);
            DBUG_ASSERT ((grid != NULL), "no grid found");
            do {
                sv = CalcSV (WLGRID_NEXTDIM (grid), sv);
                grid = WLGRID_NEXT (grid);
            } while (grid != NULL);

            stride = WLSTRIDE_NEXT (stride);
        } while (stride != NULL);
    }

    DBUG_RETURN (sv);
}

/******************************************************************************
 *
 * function:
 *   node *SetSegAttribs(node *seg)
 *
 * description:
 *   sets the attributes UBV, BV for segment 'seg' (N_WLseg node):
 *   - calulates the lcm of all grid-steps in segment (sv)
 *   - based on this, UBV and BV are set
 *
 ******************************************************************************/

node *
SetSegAttribs (node *seg)
{
    long *sv;
    int b, d;
    int dims = WLSEG_DIMS (seg);

    DBUG_ENTER ("SetSegAttribs");

    /* calculate sv */
    sv = (long *)MALLOC (sizeof (long) * dims);
    sv = CalcSV (WLSEG_CONTENTS (seg), sv);

    /*
     * set ubv (must be a multiple of sv,
     *          except that the first components could have value 1)
     */
    WLSEG_UBV (seg) = (long *)MALLOC (sizeof (long) * dims);
    for (d = 0; d < dims; d++) {
        (WLSEG_UBV (seg))[d] = 1; /* ??? */
    }

    /*
     * set bv[] (bv[b] must be a multiple of bv[b+1],
     *           bv[BLOCKS-1] must be a multiple of ubv,
     *           except that the first components could have value 1
     *           --- but in any case: (bv[b] >= bv[b+1]), (bv[BLOCK-1] >= ubv))
     */
    WLSEG_BLOCKS (seg) = 2; /* ??? */
    for (b = 0; b < WLSEG_BLOCKS (seg); b++) {
        WLSEG_BV (seg, b) = (long *)MALLOC (sizeof (long) * dims);
        for (d = 0; d < dims; d++) {
            (WLSEG_BV (seg, b))[d] = 1; /* ??? */
        }
    }

#if 0
  WLSEG_BLOCKS(seg) = 0;

  (WLSEG_BV(seg, 0))[0] = 200;
  (WLSEG_BV(seg, 0))[1] = 100;
  (WLSEG_BV(seg, 0))[2] = 50;

  (WLSEG_BV(seg, 1))[0] = 300;
  (WLSEG_BV(seg, 1))[1] = 150;
  (WLSEG_BV(seg, 1))[2] = 75;

  (WLSEG_UBV(seg))[0] = 1;
  (WLSEG_UBV(seg))[1] = 1;
  (WLSEG_UBV(seg))[2] = 1;
#else
    WLSEG_BLOCKS (seg) = 1;

    (WLSEG_BV (seg, 0))[0] = 180; /* 180 or 1 */
    (WLSEG_BV (seg, 0))[1] = 156; /* 156 */

    (WLSEG_UBV (seg))[0] = 1; /* 1 */
    (WLSEG_UBV (seg))[1] = 6; /* 6 */
#endif

    FREE (sv);

    DBUG_RETURN (seg);
}

/******************************************************************************
 *
 * function:
 *   node *SetSegs(node *cubes, int dims)
 *
 * description:
 *   returns chain of segments (based on the calculated cubes 'cubes')
 *
 ******************************************************************************/

node *
SetSegs (node *cubes, int dims)
{
    node *segs;
#if 0
  node *rect, *last_seg, *new_seg;
#endif

    DBUG_ENTER ("SetSegs");

    /* ??? */

#if 1
    /*
     * choose the hole array as the only segment
     *
     */

    segs = MakeWLseg (dims, cubes, NULL);
    segs = SetSegAttribs (segs);
#else
    /*
     * choose every cube as a segment
     *
     */

    segs = NULL;
    while (cubes != NULL) {
        /*
         * extract next cube
         */
        rect = cubes;
        cubes = WLSTRIDE_NEXT (cubes);
        WLSTRIDE_NEXT (rect) = NULL;
        /*
         * build new segment
         */
        new_seg = MakeWLseg (dims, rect, NULL);
        new_seg = SetSegAttribs (new_seg);
        /*
         * append 'new_seg' at 'segs'
         */
        if (segs == NULL) {
            segs = new_seg;
        } else {
            WLSEG_NEXT (last_seg) = new_seg;
        }
        last_seg = new_seg;
    }
#endif

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * function:
 *   node *NewBoundsStride(node *stride, int dim, int bound1, int bound2)
 *
 * description:
 *   returns modified 'stride':
 *     all strides in dimension "current dimension"+'dim' are new bounds
 *     given ('bound1', 'bound2').
 *
 ******************************************************************************/

node *
NewBoundsStride (node *stride, int dim, int bound1, int bound2)
{
    node *grid;

    DBUG_ENTER ("NewBoundsStride");

    if (dim == 0) {

        /*
         * arrivied at the correct dimension
         *  -> set new bounds
         */
        WLSTRIDE_BOUND1 (stride) = bound1;
        WLSTRIDE_BOUND2 (stride) = bound2;

    } else {

        /*
         * involve all grids of current dimension
         */

        grid = WLSTRIDE_CONTENTS (stride);
        do {
            WLGRID_NEXTDIM (grid)
              = NewBoundsStride (WLGRID_NEXTDIM (grid), dim - 1, bound1, bound2);
            grid = WLGRID_NEXT (grid);
        } while (grid != NULL);
    }

    DBUG_RETURN (stride);
}

/******************************************************************************
 *
 * function:
 *   void SplitStride(node *stride1, node *stride2
 *                    node **s_stride1, node **s_stride2)
 *
 * description:
 *   returns in 's_stride1', 's_stride2' the splitted stride 'stride1',
 *     'stride2' respectively.
 *   returns NULL if there is nothing to split.
 *
 ******************************************************************************/

void
SplitStride (node *stride1, node *stride2, node **s_stride1, node **s_stride2)
{
    node *tmp1, *tmp2;
    int i_bound1, i_bound2, dim;

    DBUG_ENTER ("SplitStride");

    tmp1 = stride1;
    tmp2 = stride2;

    *s_stride1 = *s_stride2 = NULL;

    /*
     * in which dimension is splitting needed?
     *
     * search for the first dim,
     * in which the bounds of 'stride1' and 'stride2' are not equal
     */
    dim = 0;
    while ((tmp1 != NULL) && (tmp2 != NULL)
           && (WLSTRIDE_BOUND1 (tmp1) == WLSTRIDE_BOUND1 (tmp2))
           && (WLSTRIDE_BOUND2 (tmp1) == WLSTRIDE_BOUND2 (tmp2))) {

        /*
         * we can take the first grid only,
         * because the stride-bounds are equal in all grids!!
         */
        tmp1 = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (tmp1));
        tmp2 = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (tmp2));
        dim++;
    }

    if ((tmp1 != NULL) && (tmp2 != NULL)) { /* is there anything to split? */

        /* compute bounds of intersection */
        i_bound1 = MAX (WLSTRIDE_BOUND1 (tmp1), WLSTRIDE_BOUND1 (tmp2));
        i_bound2 = MIN (WLSTRIDE_BOUND2 (tmp1), WLSTRIDE_BOUND2 (tmp2));

        if (i_bound1 < i_bound2) { /* is intersection non-empty? */

            *s_stride1 = DupNode (stride1);
            *s_stride2 = DupNode (stride2);

            /*
             * propagate the new bounds in dimension 'dim'
             * over the whole tree of 'stride1', 'stride2' respectively
             */
            *s_stride1 = NewBoundsStride (*s_stride1, dim, i_bound1, i_bound2);
            *s_stride2 = NewBoundsStride (*s_stride2, dim, i_bound1, i_bound2);
        }
    }
}

/******************************************************************************
 *
 * function:
 *   node *SplitWL(node *strides)
 *
 * description:
 *   returns the splitted stride-tree 'strides'.
 *
 ******************************************************************************/

node *
SplitWL (node *strides)
{
    node *stride1, *stride2, *split_stride1, *split_stride2, *new_strides, *tmp;
    int fixpoint;

    DBUG_ENTER ("SplitWL");

    do {
        fixpoint = 1;
        new_strides = NULL;

        /* check WLSTRIDE_MODIFIED */
        stride1 = strides;
        while (stride1 != NULL) {
            DBUG_ASSERT ((WLSTRIDE_MODIFIED (stride1) == 0), "stride was modified");
            stride1 = WLSTRIDE_NEXT (stride1);
        }

        /*
         * split in pairs
         */
        stride1 = strides;
        while (stride1 != NULL) {

            stride2 = WLSTRIDE_NEXT (stride1);
            while (stride2 != NULL) {

                SplitStride (stride1, stride2, &split_stride1, &split_stride2);
                if (split_stride1 != NULL) {
                    DBUG_ASSERT ((split_stride2 != NULL), "wrong splitting");
                    fixpoint = 0;
                    WLSTRIDE_MODIFIED (stride1) = WLSTRIDE_MODIFIED (stride2) = 1;
                    new_strides = InsertWLnodes (new_strides, split_stride1);
                    new_strides = InsertWLnodes (new_strides, split_stride2);
                } else {
                    DBUG_ASSERT ((split_stride2 == NULL), "wrong splitting");
                }

                stride2 = WLSTRIDE_NEXT (stride2);
            }

            /* have 'stride1' only empty intersections with the others? */
            if (WLSTRIDE_MODIFIED (stride1) == 0) {
                /* insert 'stride1' in 'new_strides' */
                tmp = stride1;
                stride1 = WLSTRIDE_NEXT (stride1);
                WLSTRIDE_NEXT (tmp) = NULL;
                new_strides = InsertWLnodes (new_strides, tmp);
            } else {
                /* 'stride1' is no longer needed */
                stride1 = FreeNode (stride1);
                /* 'stride1' points now to his successor!! */
            }
        }

        strides = new_strides;
    } while (!fixpoint);

    DBUG_RETURN (strides);
}

/******************************************************************************
 *
 * function:
 *   node *BlockStride(node *stride, long *bv)
 *
 * description:
 *   returns 'stride' with corrected bounds and blocking levels
 *     (needed after a blocking)
 *
 ******************************************************************************/

node *
BlockStride (node *stride, long *bv)
{
    node *curr_stride, *curr_grid;

    DBUG_ENTER ("BlockStride");

    if (stride != NULL) {

        DBUG_ASSERT ((NODE_TYPE (stride)), "no N_WLstride node found");

        curr_stride = stride;
        do {

            /* correct blocking level */
            WLSTRIDE_LEVEL (curr_stride)++;

            /* fit bounds of stride to blocking step */
            DBUG_ASSERT ((bv[WLSTRIDE_DIM (curr_stride)] > 1), "wrong bv value found");
            DBUG_ASSERT ((bv[WLSTRIDE_DIM (curr_stride)] >= WLSTRIDE_STEP (curr_stride)),
                         "blocking step (>1) is smaller than stride step");
            WLSTRIDE_BOUND1 (curr_stride) = 0;
            WLSTRIDE_BOUND2 (curr_stride) = bv[WLSTRIDE_DIM (curr_stride)];

            /*
             * involve all grids of current dimension
             */
            curr_grid = WLSTRIDE_CONTENTS (curr_stride);
            do {
                WLGRID_NEXTDIM (curr_grid) = BlockStride (WLGRID_NEXTDIM (curr_grid), bv);
                curr_grid = WLGRID_NEXT (curr_grid);
            } while (curr_grid != NULL);

            curr_stride = WLSTRIDE_NEXT (curr_stride);
        } while (curr_stride != NULL);
    }

    DBUG_RETURN (stride);
}

/******************************************************************************
 *
 * function:
 *   node *BlockWL(node *stride, int dims, long *bv, int unroll)
 *
 * description:
 *   returns with blocking-vector 'bv' blocked 'stride'.
 *
 *   when called multiple times in a row, this function even realizes
 *     hierarchical blocking!! (top-down: coarse blocking first!)
 *
 *   ('unroll' > 0) means unrolling-blocking --- allowed only once after all
 *     convential blocking!
 *
 ******************************************************************************/

node *
BlockWL (node *stride, int dims, long *bv, int unroll)
{
    node *curr_block, *curr_dim, *curr_stride, *curr_grid, *contents, *lastdim,
      *last_block, *block;
    int level, d;

    DBUG_ENTER ("BlockWL");

    if (stride != NULL) {

        switch (NODE_TYPE (stride)) {

        case N_WLblock:
            /*
             * block found -> hierarchical blocking
             */

            curr_block = stride;
            while (curr_block != NULL) {

                /* go to contents of block -> skip all found block nodes */
                curr_dim = curr_block;
                DBUG_ASSERT ((NODE_TYPE (curr_dim) == N_WLblock), "no block found");
                while (WLBLOCK_NEXTDIM (curr_dim) != NULL) {
                    curr_dim = WLBLOCK_NEXTDIM (curr_dim);
                    DBUG_ASSERT ((NODE_TYPE (curr_dim) == N_WLblock), "no block found");
                }

                /* block contents of found block */
                WLBLOCK_CONTENTS (curr_dim)
                  = BlockWL (WLBLOCK_CONTENTS (curr_dim), dims, bv, unroll);

                curr_block = WLBLOCK_NEXT (curr_block);
            }
            break;

        case N_WLublock:
            /*
             * ublock found ?!?!
             */

            /*
             * unrolling-blocking is allowed only once
             * after all conventional blocking!!
             */
            DBUG_ASSERT ((0), "data of unrolling-blocking found while blocking");
            break;

        case N_WLstride:
            /*
             * unblocked stride found
             */

            level = WLSTRIDE_LEVEL (stride);

            last_block = NULL;
            curr_stride = stride;
            while (curr_stride != NULL) {

                if (bv[WLSTRIDE_DIM (curr_stride)] == 1) {
                    /* no blocking -> go to next dim */
                    curr_grid = WLSTRIDE_CONTENTS (curr_stride);
                    do {
                        WLGRID_NEXTDIM (curr_grid)
                          = BlockWL (WLGRID_NEXTDIM (curr_grid), dims, bv, unroll);

                        curr_grid = WLGRID_NEXT (curr_grid);
                    } while (curr_grid != NULL);

                    curr_stride = WLSTRIDE_NEXT (curr_stride);
                } else {
                    /* blocking -> create a N_WLblock (N_WLublock respectively) node for
                     * each following dim */
                    contents = curr_stride;
                    lastdim = NULL;
                    for (d = WLSTRIDE_DIM (curr_stride); d < dims; d++) {
                        DBUG_ASSERT ((NODE_TYPE (contents) == N_WLstride),
                                     "wrong hierarchical blocking");

                        block
                          = MakeWLblock (level, WLSTRIDE_DIM (contents),
                                         WLSTRIDE_BOUND1 (contents),
                                         WLSTRIDE_BOUND2 (contents),
                                         bv[WLSTRIDE_DIM (contents)], NULL, NULL, NULL);

                        if (unroll > 0) { /* unrolling-blocking wanted? */
                            NODE_TYPE (block) = N_WLublock;
                        }

                        if (lastdim != NULL) {
                            /*
                             * not first blocking dim
                             *  -> append at block node of last dim
                             */
                            WLBLOCK_NEXTDIM (lastdim) = block;
                        } else {
                            /* current dim is first blocking dim */
                            if (last_block != NULL) {
                                /* append to last block */
                                WLBLOCK_NEXT (last_block) = block;
                            } else {
                                /* this is the first block */
                                stride = block;
                            }
                            last_block = block;
                        }
                        lastdim = block;

                        DBUG_ASSERT ((WLSTRIDE_CONTENTS (contents) != NULL),
                                     "no grid found");
                        contents = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (contents));
                    }

                    /*
                     * now the block nodes are complete
                     *  -> append contents of block
                     */
                    DBUG_ASSERT ((lastdim != NULL), "block node of last dim not found");
                    WLBLOCK_CONTENTS (lastdim) = curr_stride;
                    curr_stride = WLSTRIDE_NEXT (curr_stride);
                    /* successor is in next block -> no 'next' anymore! */
                    WLSTRIDE_NEXT (WLBLOCK_CONTENTS (lastdim)) = NULL;
                    /* correct the bounds and blocking level in contents of block */
                    WLBLOCK_CONTENTS (lastdim)
                      = BlockStride (WLBLOCK_CONTENTS (lastdim), bv);
                }
            }
            break;

        default:

            DBUG_ASSERT ((0), "wrong node type");
        }
    }

    DBUG_RETURN (stride);
}

/******************************************************************************
 *
 * function:
 *   node *MergeWL(node *nodes)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
MergeWL (node *nodes)
{
    DBUG_ENTER ("MergeWL");

#if 0
  /*
   * collect all projs with the same bounds
   */
  proj1 = projs;
  while (proj1 != NULL) {
 
    proj2 = proj1;
    while ((WLSTRIDE_NEXT(proj2) != NULL) && 
           (WLSTRIDE_BOUND1(proj1) == WLSTRIDE_BOUND1(WLSTRIDE_NEXT(proj2)))) {
      DBUG_ASSERT((WLSTRIDE_BOUND2(proj1) == WLSTRIDE_BOUND2(WLSTRIDE_NEXT(proj2))),
                  "wrong bounds found");
      proj2 = WLSTRIDE_NEXT(proj2);
    }

    tmp = proj1;
    proj1 = WLSTRIDE_NEXT(proj2);
    WLSTRIDE_NEXT(proj2) = NULL;


  }

#endif

#if 0
  if (node1 != NULL) {

    grid1 = WLSTRIDE_CONTENTS(proj1);
    grid2 = WLSTRIDE_CONTENTS(proj2);

    bound11 = WLSTRIDE_BOUND1(proj1);
    bound12 = WLSTRIDE_BOUND1(proj2);

    WLSTRIDE_BOUND1(proj1) = MAX(bound11, bound12);
    WLSTRIDE_BOUND2(proj1) = MIN(WLSTRIDE_BOUND2(proj1), WLSTRIDE_BOUND2(proj2));

    /*
     * compute new offset for 'grid2'
     */
    WLGRID_BOUND1(grid2) -= GridBound1(WLSTRIDE_BOUND1(proj1),
                                       bound12, WLSTRIDE_STEP(proj2),
                                       WLGRID_BOUND1(grid2), WLGRID_BOUND2(grid2));

    /*
     * compute new offsets for all grids in 'proj1'
     */
    tmp = grid1;
    do {
      WLGRID_BOUND1(tmp) -= GridBound1(WLSTRIDE_BOUND1(proj1),
                                       bound11, WLSTRIDE_STEP(proj1),
                                       WLGRID_BOUND1(tmp), WLGRID_BOUND2(tmp));
      tmp = WLGRID_NEXT(tmp);
    }
    while (tmp != NULL);

    /*
     * insert 'grid2'
     */
    if (WLGRID_BOUND1(grid1) > WLGRID_BOUND1(grid2)) {
      /* insert 'grid2' at head of grid list */
      DBUG_ASSERT((WLGRID_BOUND1(grid1) >= WLGRID_BOUND2(grid2)),
                  "wrong offset");
      new_grid = DupTree(grid2, NULL);
      WLGRID_NEXT(new_grid) = grid1;
      WLSTRIDE_CONTENTS(proj1) = new_grid;
    }
    else {
      /* search for correct position for insertion */
      tmp = grid1;
      while ((WLGRID_NEXT(tmp) != NULL) &&
             (WLGRID_BOUND1(tmp) != WLGRID_BOUND1(grid2)) &&
             (WLGRID_BOUND1(WLGRID_NEXT(tmp)) <= WLGRID_BOUND1(grid2))) {
        tmp = WLGRID_NEXT(tmp);
      }

      if (WLGRID_BOUND1(tmp) == WLGRID_BOUND1(grid2)) {
        /* range of 'grid2' is already in grid list -> merge next dim */
        WLGRID_NEXTDIM(tmp) = MergeCube(WLGRID_NEXTDIM(tmp), WLGRID_NEXTDIM(grid2));
      }
      else {
        /* insert 'grid2' after 'tmp' */
        if (WLGRID_NEXT(tmp) != NULL) {
          DBUG_ASSERT((WLGRID_BOUND1(WLGRID_NEXT(tmp)) >= WLGRID_BOUND2(grid2)),
                      "wrong offset");
	}
        new_grid = DupTree(grid2, NULL);
        WLGRID_NEXT(new_grid) = WLGRID_NEXT(tmp);
        WLGRID_NEXT(tmp) = new_grid;
      }
    }
  }
#endif

    DBUG_RETURN (nodes);
}

/******************************************************************************
 *
 * function:
 *   node *OptimizeWL(node *nodes)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
OptimizeWL (node *nodes)
{
    DBUG_ENTER ("OptimizeWL");

    DBUG_RETURN (nodes);
}

/******************************************************************************
 *
 * function:
 *   node *NormalizeWL(node *nodes)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NormalizeWL (node *nodes)
{
    DBUG_ENTER ("NormalizeWL");

    DBUG_RETURN (nodes);
}

/******************************************************************************
 *
 * function:
 *   node *FitWL(node *nodes)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
FitWL (node *nodes)
{
    DBUG_ENTER ("FitWL");

    DBUG_RETURN (nodes);
}

/******************************************************************************
 *
 * function:
 *   node *PRECnwith(node *arg_node, node *arg_info)
 *
 * description:
 *   precompilation of new with-loop (N_Nwith-node)
 *
 ******************************************************************************/

node *
PRECnwith (node *arg_node, node *arg_info)
{
    node *new_node, *cubes, *segs, *seg, *shape;
    int dims, b;
    enum {
        PREC_PH_cube,
        PREC_PH_seg,
        PREC_PH_split,
        PREC_PH_block,
        PREC_PH_ublock,
        PREC_PH_merge,
        PREC_PH_opt,
        PREC_PH_norm,
        PREC_PH_fit
    } PREC_break_after;

    DBUG_ENTER ("PRECnwith");

    /* analyse 'break_specifier' */
    PREC_break_after = PREC_PH_fit;
    if (break_after == PH_precompile) {
        if (strcmp (break_specifier, "cube") == 0) {
            PREC_break_after = PREC_PH_cube;
        } else {
            if (strcmp (break_specifier, "seg") == 0) {
                PREC_break_after = PREC_PH_seg;
            } else {
                if (strcmp (break_specifier, "split") == 0) {
                    PREC_break_after = PREC_PH_split;
                } else {
                    if (strcmp (break_specifier, "block") == 0) {
                        PREC_break_after = PREC_PH_block;
                    } else {
                        if (strcmp (break_specifier, "ublock") == 0) {
                            PREC_break_after = PREC_PH_ublock;
                        } else {
                            if (strcmp (break_specifier, "merge") == 0) {
                                PREC_break_after = PREC_PH_merge;
                            } else {
                                if (strcmp (break_specifier, "opt") == 0) {
                                    PREC_break_after = PREC_PH_opt;
                                } else {
                                    if (strcmp (break_specifier, "norm") == 0) {
                                        PREC_break_after = PREC_PH_norm;
                                    } else {
                                        if (strcmp (break_specifier, "fit") == 0) {
                                            PREC_break_after = PREC_PH_fit;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    DBUG_ASSERT ((NWITHOP_TYPE (NWITH_WITHOP (arg_node)) == WO_genarray),
                 "type of with-loop is not WO_genarray");

    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

    new_node = MakeNWith2 (NPART_WITHID (NWITH_PART (arg_node)), NULL,
                           NWITH_CODE (arg_node), NWITH_WITHOP (arg_node));

    /*
     * withid, code and withop are overtaken to the nwith2-tree without a change.
     * because of that, these parts are cut off from the old nwith-tree, before freeing
     * it.
     *
     */
    NPART_WITHID (NWITH_PART (arg_node)) = NULL;
    NWITH_CODE (arg_node) = NULL;
    NWITH_WITHOP (arg_node) = NULL;

    DBUG_ASSERT ((NWITHOP_TYPE (NWITH2_WITHOP (new_node)) == WO_genarray),
                 "type of with-loop is not genarray");

    /* get shape of genarray: */
    shape = NWITHOP_SHAPE (NWITH2_WITHOP (new_node));
    DBUG_ASSERT ((NODE_TYPE (shape) == N_array), "shape of with-genarray is unknown");

    /* get number of dims of genarray: */
    dims = SHPSEG_SHAPE (TYPES_SHPSEG (ARRAY_TYPE (shape)), 0);

    NOTE (("step 1: cube-building\n"))
    cubes = ComputeCubes (Parts2Strides (NWITH_PART (arg_node)));
    if (PREC_break_after == PREC_PH_cube) {
        /* build one segment containing all the cubes */
        segs = MakeWLseg (dims, cubes, NULL);
    }

    if (PREC_break_after >= PREC_PH_seg) {
        NOTE (("step 2: choice of segments\n"))
        segs = SetSegs (cubes, dims);

        seg = segs;
        while (seg != NULL) {
            /* splitting */
            if (PREC_break_after >= PREC_PH_split) {
                NOTE (("step 3: splitting\n"))
                WLSEG_CONTENTS (seg) = SplitWL (WLSEG_CONTENTS (seg));
            }

            /* hierarchical blocking */
            if (PREC_break_after >= PREC_PH_block) {
                NOTE (("step 4: hierarchical blocking\n"))
                for (b = 0; b < WLSEG_BLOCKS (seg); b++) {
                    NOTE (("step 4.%d: hierarchical blocking (level %d)\n", b + 1, b))
                    WLSEG_CONTENTS (seg)
                      = BlockWL (WLSEG_CONTENTS (seg), dims, WLSEG_BV (seg, b), 0);
                    FREE (WLSEG_BV (seg, b));
                }
            }

            /* unrolling-blocking */
            if (PREC_break_after >= PREC_PH_ublock) {
                NOTE (("step 5: unrolling-blocking\n"))
                WLSEG_CONTENTS (seg)
                  = BlockWL (WLSEG_CONTENTS (seg), dims, WLSEG_UBV (seg), 1);
                FREE (WLSEG_UBV (seg));
            }

            /* splitting - merging */
            if (PREC_break_after >= PREC_PH_merge) {
                NOTE (("step 6: splitting-merging\n"))
                WLSEG_CONTENTS (seg) = MergeWL (WLSEG_CONTENTS (seg));
            }

            /* optimization */
            if (PREC_break_after >= PREC_PH_opt) {
                NOTE (("step 7: optimization\n"))
                WLSEG_CONTENTS (seg) = OptimizeWL (WLSEG_CONTENTS (seg));
            }

            /* normalization */
            if (PREC_break_after >= PREC_PH_norm) {
                NOTE (("step 8: normalization\n"))
                WLSEG_CONTENTS (seg) = NormalizeWL (WLSEG_CONTENTS (seg));
            }

            /* fitting */
            if (PREC_break_after >= PREC_PH_fit) {
                NOTE (("step 9: fitting\n"))
                WLSEG_CONTENTS (seg) = FitWL (WLSEG_CONTENTS (seg));
            }

            seg = WLSEG_NEXT (seg);
        }
    }

    NWITH2_SEG (new_node) = segs;

    arg_node = FreeTree (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRECncode(node *arg_node, node *arg_info)
 *
 * description:
 *   precompilation of Ncode-nodes:
 *     CODE_NO is set in the whole Ncode-chain
 *
 ******************************************************************************/

node *
PRECncode (node *arg_node, node *arg_info)
{
    node *code;
    int no = 0;

    DBUG_ENTER ("PRECncode");

    code = arg_node;
    while (code != NULL) {
        NCODE_NO (code) = no;
        no++;
        code = NCODE_NEXT (code);
    }

    DBUG_RETURN (arg_node);
}
