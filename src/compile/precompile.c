/*
 *
 * $Log$
 * Revision 1.50  1998/04/16 11:55:56  dkr
 * removed unused vars
 *
 * Revision 1.49  1998/04/10 03:13:20  dkr
 * fixed a bug in FitWL
 *
 * Revision 1.48  1998/04/10 02:25:58  dkr
 * added support for wlcomp-pragmas
 *
 * Revision 1.47  1998/04/09 14:00:07  dkr
 * attributes for N_conc nodes are now build in 'concregions.[ch]'
 *
 * Revision 1.46  1998/04/07 17:33:19  dkr
 * removed a bug in PRECnwith
 *
 * Revision 1.45  1998/04/04 21:07:29  dkr
 * changed PRECconc
 *
 * Revision 1.44  1998/04/03 21:07:54  dkr
 * changed usage of arg_info
 * changed PRECconc
 *
 * Revision 1.43  1998/04/02 18:47:31  dkr
 * added PRECconc
 *
 * Revision 1.42  1998/04/01 23:57:14  dkr
 * removed a few bugs
 *
 * Revision 1.41  1998/03/31 18:34:33  dkr
 * the UNROLLING flag in N_WLstride, N_WLgrid is now set correctly
 *
 * Revision 1.40  1998/03/31 00:03:52  dkr
 * removed unused vars
 *
 * Revision 1.39  1998/03/30 23:57:51  dkr
 * fixed a bug in PRECnwith:
 *   default value for 'PREC_break_after' is now correct
 *
 * Revision 1.38  1998/03/30 23:43:05  dkr
 * PRECnwith is completed!!!
 *
 * Revision 1.37  1998/03/29 23:28:57  dkr
 * first release with complete support for WL-PREC-phases 1-6
 *
 * Revision 1.36  1998/03/28 20:29:10  dkr
 * removed a bug in splitting phase
 * added merging phase (for blocks only)
 *
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
#include "wlpragma_funs.h"

#include "DupTree.h"
#include "refcount.h"
#include "dbug.h"

#include <string.h>
#include <limits.h> /* INT_MAX */

/*
 * these macros are used in 'Parts2Strides' to manage
 *   non-constant generator params
 */
#define TO_FIRST_COMP(node)                                                              \
    if (NODE_TYPE (node) == N_array) {                                                   \
        node = ARRAY_AELEMS (node);                                                      \
    }

#define GET_CURRENT_COMP(node, comp)                                                     \
    if (node != NULL) {                                                                  \
        if (NODE_TYPE (node) == N_id) {                                                  \
            comp = DupTree (node, NULL);                                                 \
        } else {                                                                         \
            DBUG_ASSERT ((NODE_TYPE (node) == N_exprs), "wrong node type found");        \
            comp = MakeNum (NUM_VAL (EXPRS_EXPR (node)));                                \
            node = EXPRS_EXPR (node);                                                    \
        }                                                                                \
    } else {                                                                             \
        comp = MakeNum (1);                                                              \
    }

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

/******************************************************************************
 *
 * function:
 *   node *precompile(node *syntax_tree)
 *
 * description:
 *   prepares syntax tree for code generation.
 *     - renames functions and global objects
 *     - removes all casts
 *     - inserts extern declarations for function definitions
 *     - removes all artificial parameters and return values
 *     - marks reference parameters in function applications
 *     - transforms new with-loops
 *
 ******************************************************************************/

node *
precompile (node *syntax_tree)
{
    node *info;

    DBUG_ENTER ("precompile");

    info = MakeInfo ();
    act_tab = precomp_tab;

    syntax_tree = Trav (syntax_tree, info);

    FREE (info);

    DBUG_RETURN (syntax_tree);
}

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

/******************************************************************************
 *
 * function:
 *   node *PRECmodul(node *arg_node, node *arg_info)
 *
 * description:
 *   starts traversal mechanism for objdef and fundef nodes.
 *   appends new fundefs from 'arg_info' at fundef chain.
 *
 ******************************************************************************/

node *
PRECmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECmodul");

    INFO_PREC_MODUL (arg_info) = arg_node;

    if (MODUL_TYPES (arg_node) != NULL) {
        MODUL_TYPES (arg_node) = Trav (MODUL_TYPES (arg_node), arg_info);
    }

    if (MODUL_OBJS (arg_node) != NULL) {
        MODUL_OBJS (arg_node) = Trav (MODUL_OBJS (arg_node), arg_info);
    }

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
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

/******************************************************************************
 *
 * function:
 *   node *PRECfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   triggers the precompilation
 *
 ******************************************************************************/

node *
PRECfundef (node *arg_node, node *arg_info)
{
    char *keep_name, *keep_mod;
    statustype keep_status, keep_attrib;
    int i;

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

    INFO_PREC_CNT_ARTIFICIAL (arg_info) = 0;
    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    /*
     *  All artificial return types are removed.
     *  It is necessary to keep name, module name, status, and attrib
     *  because in the real syntax tree these are stored within the types
     *  structure and not as part of the fundef node as in the virtual
     *  syntax tree.
     */

    if (INFO_PREC_CNT_ARTIFICIAL (arg_info) > 0) {
        keep_name = FUNDEF_NAME (arg_node);
        keep_mod = FUNDEF_MOD (arg_node);
        keep_status = FUNDEF_STATUS (arg_node);
        keep_attrib = FUNDEF_ATTRIB (arg_node);

        for (i = 1; i < INFO_PREC_CNT_ARTIFICIAL (arg_info); i++) {
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
        FUNDEF_BODY (arg_node) = InsertObjInits (FUNDEF_BODY (arg_node),
                                                 MODUL_OBJS (INFO_PREC_MODUL (arg_info)));
    } else {
        if (FUNDEF_MOD (arg_node) == NULL) {
            FUNDEF_STATUS (arg_node) = ST_Cfun;
        }

        arg_node = RenameFun (arg_node);
        FUNDEF_TYPES (arg_node) = RenameTypes (FUNDEF_TYPES (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRECarg(node *arg_node, node *arg_info)
 *
 * description:
 *  An artificial argument is removed, the attribs are switched:
 *       ST_readonly_reference -> ST_regular
 *       ST_was_reference -> ST_inout
 *
 *  INFO_PREC_CNT_ARTIFICIAL is used to count the number of artificial
 *   return values.
 *
 ******************************************************************************/

node *
PRECarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECarg");

    if (ARG_ATTRIB (arg_node) == ST_was_reference) {
        INFO_PREC_CNT_ARTIFICIAL (arg_info)++;
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

node *
PRECassign (node *arg_node, node *arg_info)
{
    node *instrs;

    DBUG_ENTER ("PRECassign");

    instrs = Trav (ASSIGN_INSTR (arg_node), arg_info);

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
 * function:
 *   node *PRECconc(node *arg_node, node *arg_info)
 *
 * description:
 *   precompile of a N_conc node: just traverses the son
 *
 ******************************************************************************/

node *
PRECconc (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PRECconc");

    CONC_REGION (arg_node) = Trav (CONC_REGION (arg_node), arg_info);

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
 *   possibly present next nodes in 'node1' or 'node2' are ignored.
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
                     "can not compare objects of different type");

        /* compare the bounds first */
        COMP_BEGIN (WLNODE_BOUND1 (node1), WLNODE_BOUND1 (node2), result, 2)
        COMP_BEGIN (WLNODE_BOUND2 (node1), WLNODE_BOUND2 (node2), result, 2)

        switch (NODE_TYPE (node1)) {

        case N_WLblock:
            /* here is no break missing! */
        case N_WLublock:

            /* compare next dim */
            result
              = CompareWLnode (WLNODE_NEXTDIM (node1), WLNODE_NEXTDIM (node2), outline);
            break;

        case N_WLstride:

            grid1 = WLSTRIDE_CONTENTS (node1);
            DBUG_ASSERT ((grid1 != NULL), "no grid found");
            grid2 = WLSTRIDE_CONTENTS (node2);
            DBUG_ASSERT ((grid2 != NULL), "no grid found");

            if (outline) {
                /* compare outlines only -> skip grid */
                result = CompareWLnode (WLGRID_NEXTDIM (grid1), WLGRID_NEXTDIM (grid2),
                                        outline);
            } else {
                /*
                 * compare grid, but leave 'result' untouched
                 *   until later dimensions are checked!
                 */
                COMP_BEGIN (WLGRID_BOUND1 (grid1), WLGRID_BOUND1 (grid2), grid_result, 1)
                COMP_BEGIN (WLGRID_BOUND2 (grid1), WLGRID_BOUND2 (grid2), grid_result, 1)
                grid_result = 0;
                COMP_END
                COMP_END

                /* compare later dimensions */
                result = CompareWLnode (WLGRID_NEXTDIM (grid1), WLGRID_NEXTDIM (grid2),
                                        outline);

                /*
                 * the 'grid_result' value is important
                 *   only if the outlines are equal
                 */
                if (abs (result) != 2) {
                    result = grid_result;
                }
            }

            break;

        case N_WLgrid:

            result
              = CompareWLnode (WLGRID_NEXTDIM (node1), WLGRID_NEXTDIM (node2), outline);
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
            result = (node2 == NULL) ? (2) : (-2);
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
 *   a possibly present next node in 'stride' is ignored.
 *
 *   this normalization has two major goals:
 *     * every stride has a unambiguous form
 *        -> two strides represent the same index set
 *           if and only if there attribute values are equal.
 *     * maximize the outline of strides
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
    new_bound2 = ((bound2 - bound1 - grid_b1) % step >= grid_b2 - grid_b1)
                   ? (bound2 + step - ((bound2 - bound1 - grid_b1) % step))
                   : (bound2);

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
 *   node* Parts2Strides(node *parts, int dims)
 *
 * description:
 *   converts a N_Npart-chain ('parts') into a N_WLstride-chain (return).
 *   'dims' is the number of dimensions.
 *
 ******************************************************************************/

node *
Parts2Strides (node *parts, int dims)
{
    node *parts_stride, *stride, *new_stride, *last_grid, *gen, *bound1, *bound2, *step,
      *width, *curr_bound1, *curr_bound2, *curr_step, *curr_width;
    int dim, curr_step_, curr_width_;

    DBUG_ENTER ("Parts2Strides");

    parts_stride = NULL;

    gen = NPART_GEN (parts);
    bound1 = NGEN_BOUND1 (gen);
    bound2 = NGEN_BOUND2 (gen);
    step = NGEN_STEP (gen);
    width = NGEN_WIDTH (gen);

    /*
     * check, if params of generator are constant
     */
    if ((NODE_TYPE (bound1) == N_array) && (NODE_TYPE (bound2) == N_array)
        && ((step == NULL) || (NODE_TYPE (step) == N_array))
        && ((width == NULL) || (NODE_TYPE (width) == N_array))) {

        /*
         * the generator parameters are constant
         */

        while (parts != NULL) {
            stride = NULL;

            gen = NPART_GEN (parts);
            DBUG_ASSERT ((NGEN_OP1 (gen) == F_le), "op1 in generator is not <=");
            DBUG_ASSERT ((NGEN_OP2 (gen) == F_lt), "op2 in generator is not <");

            /* get components of current generator */
            bound1 = ARRAY_AELEMS (NGEN_BOUND1 (gen));
            bound2 = ARRAY_AELEMS (NGEN_BOUND2 (gen));
            step = (NGEN_STEP (gen) != NULL) ? ARRAY_AELEMS (NGEN_STEP (gen)) : NULL;
            width = (NGEN_WIDTH (gen) != NULL) ? ARRAY_AELEMS (NGEN_WIDTH (gen)) : NULL;

            for (dim = 0; dim < dims; dim++) {
                DBUG_ASSERT ((bound1 != NULL), "bound1 not complete");
                DBUG_ASSERT ((bound2 != NULL), "bound2 not complete");

                curr_step_ = (step != NULL) ? NUM_VAL (EXPRS_EXPR (step)) : 1;
                curr_width_ = (width != NULL) ? NUM_VAL (EXPRS_EXPR (width)) : 1;

                /* build N_WLstride-node of current dimension */
                new_stride = MakeWLstride (0, dim, NUM_VAL (EXPRS_EXPR (bound1)),
                                           NUM_VAL (EXPRS_EXPR (bound2)), curr_step_, 0,
                                           MakeWLgrid (0, dim, 0, curr_width_, 0, NULL,
                                                       NULL, NULL),
                                           NULL);

                /* the PART-information is needed by 'IntersectOutline' */
                WLSTRIDE_PART (new_stride) = parts;

                new_stride = NormalizeStride_1 (new_stride);

                /* append 'new_stride' to 'stride' */
                if (dim == 0) {
                    stride = new_stride;
                } else {
                    WLGRID_NEXTDIM (last_grid) = new_stride;
                }
                last_grid = WLSTRIDE_CONTENTS (new_stride);

                /* go to next dim */
                bound1 = EXPRS_NEXT (bound1);
                bound2 = EXPRS_NEXT (bound2);
                if (step != NULL) {
                    step = EXPRS_NEXT (step);
                }
                if (width != NULL) {
                    width = EXPRS_NEXT (width);
                }
            }

            WLGRID_CODE (last_grid) = NPART_CODE (parts);
            NCODE_USED (NPART_CODE (parts))++;
            parts_stride = InsertWLnodes (parts_stride, stride);

            parts = NPART_NEXT (parts);
        }
        while (parts != NULL)
            ;

    } else {

        /*
         * not all generator parameters are constant
         */

        DBUG_ASSERT ((NPART_NEXT (parts) == NULL), "more than one part found");

        TO_FIRST_COMP (bound1)
        TO_FIRST_COMP (bound2)
        if (step != NULL) {
            TO_FIRST_COMP (step)
        }
        if (width != NULL) {
            TO_FIRST_COMP (width)
        }

        for (dim = 0; dim < dims; dim++) {
            /*
             * components of current dim
             */
            GET_CURRENT_COMP (bound1, curr_bound1)
            GET_CURRENT_COMP (bound2, curr_bound2)
            GET_CURRENT_COMP (step, curr_step)
            GET_CURRENT_COMP (width, curr_width)

            /* build N_WLstriVar-node of current dimension */
            new_stride = MakeWLstriVar (dim, curr_bound1, curr_bound2, curr_step,
                                        MakeWLgridVar (dim, MakeNum (0), curr_width, NULL,
                                                       NULL, NULL),
                                        NULL);

            /* append 'new_stride' to 'parts_stride' */
            if (dim == 0) {
                parts_stride = new_stride;
            } else {
                WLGRIDVAR_NEXTDIM (last_grid) = new_stride;
            }
            last_grid = WLSTRIVAR_CONTENTS (new_stride);
        }

        WLGRIDVAR_CODE (last_grid) = NPART_CODE (parts);
        NCODE_USED (NPART_CODE (parts))++;
    }

    DBUG_RETURN (parts_stride);
}

/******************************************************************************
 *
 * function:
 *   int IndexHeadStride(node *stride)
 *
 * description:
 *   returns the index position of the first element of 'stride'
 *
 ******************************************************************************/

int
IndexHeadStride (node *stride)
{
    int result;

    DBUG_ENTER ("IndexHeadStride");

    result = WLSTRIDE_BOUND1 (stride) + WLGRID_BOUND1 (WLSTRIDE_CONTENTS (stride));

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   int IndexRearStride(node *stride)
 *
 * description:
 *   returns the index position '+1' of the last element of 'stride'
 *
 ******************************************************************************/

int
IndexRearStride (node *stride)
{
    node *grid = WLSTRIDE_CONTENTS (stride);
    int bound2 = WLSTRIDE_BOUND2 (stride);
    int grid_b1 = WLGRID_BOUND1 (grid);
    int result;

    DBUG_ENTER ("IndexRearStride");

    /* search last grid (there will we find the last element!) */
    while (WLGRID_NEXT (grid) != NULL) {
        grid = WLGRID_NEXT (grid);
    }

    result = bound2
             - MAX (0, ((bound2 - WLSTRIDE_BOUND1 (stride) - grid_b1 - 1)
                        % WLSTRIDE_STEP (stride))
                         + 1 - (WLGRID_BOUND2 (grid) - grid_b1));

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   int GridOffset(int new_bound1,
 *                  int bound1, int step, int grid_b2)
 *
 * description:
 *   computes a offset for a grid relating to 'new_bound1':
 *     what happens to the bounds of a grid if 'new_bound1' is the new
 *     upper bound for the accessory stride?
 *     the new bounds of the grid are:
 *         grid_b1 - offset, grid_b2 - offset
 *
 *   CAUTION: if (offset > grid_b1) the grid must be devided in two parts:
 *              "(grid_b1 - offset + step) -> step" and
 *              "0 -> (grid_b2 - offset)" !!
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
 *   int IntersectOutline(node *stride1, node *stride2,
 *                        node **i_stride1, node **i_stride2)
 *
 * description:
 *   returns in 'i_stride1' and 'i_stride2' the part of 'stride1', 'stride2'
 *     respectively that lies in a common cube.
 *   possibly present next nodes in 'stride1' or 'stride2' are ignored.
 *   the return value is 1 if and only if the intersection is non-empty.
 *
 *   if we are interested in the return value only, we can call this
 *     function with ('stride1' == NULL), ('stride2' == NULL).
 *
 ******************************************************************************/

int
IntersectOutline (node *stride1, node *stride2, node **i_stride1, node **i_stride2)
{
    node *grid1, *grid2, *new_i_stride1, *new_i_stride2;
    int bound11, bound21, grid1_b1, grid1_b2, bound12, bound22, grid2_b1, grid2_b2, head1,
      rear1, head2, rear2, i_bound1, i_bound2, i_offset1, i_offset2;
    int flag = 0;
    int result = 1;

    DBUG_ENTER ("IntersectOutline");

    if (i_stride1 != NULL) {
        new_i_stride1 = *i_stride1 = DupNode (stride1);
    }
    if (i_stride2 != NULL) {
        new_i_stride2 = *i_stride2 = DupNode (stride2);
    }

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
        grid1_b1 = WLGRID_BOUND1 (grid1);
        grid1_b2 = WLGRID_BOUND2 (grid1);

        bound12 = WLSTRIDE_BOUND1 (stride2);
        bound22 = WLSTRIDE_BOUND2 (stride2);
        grid2_b1 = WLGRID_BOUND1 (grid2);
        grid2_b2 = WLGRID_BOUND2 (grid2);

        head1 = IndexHeadStride (stride1);
        rear1 = IndexRearStride (stride1);
        head2 = IndexHeadStride (stride2);
        rear2 = IndexRearStride (stride2);

        i_bound1 = MAX (bound11, bound12);
        i_bound2 = MIN (bound21, bound22);

        i_offset1 = GridOffset (i_bound1, bound11, WLSTRIDE_STEP (stride1), grid1_b2);
        i_offset2 = GridOffset (i_bound1, bound12, WLSTRIDE_STEP (stride2), grid2_b2);

        if ((head1 < rear2) && (head2 < rear1) &&
            /* are the outlines of 'stride1' and 'stride2' not disjunkt? */
            (i_offset1 <= grid1_b1) && (i_offset2 <= grid2_b1)) {
            /* are the grids compatible? */

            if ((WLSTRIDE_PART (stride1) == WLSTRIDE_PART (stride2)) &&
                /* are 'stride1' and 'stride2' descended from the same Npart? */
                (!flag)) {
                /* we should deal with this exception only once !! */

                /*
                 * example:
                 *
                 *  0->6  step 3, 0->1: op1
                 *  0->16 step 3, 1->3: op2
                 *  4->20 step 3, 2->3: op3
                 * ------------------------- after first round with IntersectOutline:
                 *  0->7  step 3, 1->3: op2  <- intersection of 'op2' and outline('op1')
                 *  3->16 step 3, 1->3: op2  <- intersection of 'op2' and outline('op3')
                 *
                 *  these two strides are **not** disjunkt!!!
                 *  but they are part of the same Npart!!
                 */

                flag = 1; /* skip this exception handling in later dimensions! */

                /*
                 * modify the bounds of the first stride,
                 * so that the new outlines are disjunct
                 */
                if (WLSTRIDE_BOUND2 (stride1) < WLSTRIDE_BOUND2 (stride2)) {
                    if (i_stride1 != NULL) {
                        WLSTRIDE_BOUND2 (new_i_stride1) = i_bound1;
                        new_i_stride1 = NormalizeStride_1 (new_i_stride1);
                    }
                } else {
                    if (i_stride2 != NULL) {
                        WLSTRIDE_BOUND2 (new_i_stride2) = i_bound1;
                        new_i_stride2 = NormalizeStride_1 (new_i_stride2);
                    }
                }

            } else {

                if (i_stride1 != NULL) {
                    /* intersect 'stride1' with the outline of 'stride2' */
                    WLSTRIDE_BOUND1 (new_i_stride1) = i_bound1;
                    WLSTRIDE_BOUND2 (new_i_stride1) = i_bound2;
                    WLGRID_BOUND1 (WLSTRIDE_CONTENTS (new_i_stride1))
                      = grid1_b1 - i_offset1;
                    WLGRID_BOUND2 (WLSTRIDE_CONTENTS (new_i_stride1))
                      = grid1_b2 - i_offset1;
                    new_i_stride1 = NormalizeStride_1 (new_i_stride1);
                }

                if (i_stride2 != NULL) {
                    /* intersect 'stride2' with the outline of 'stride1' */
                    WLSTRIDE_BOUND1 (new_i_stride2) = i_bound1;
                    WLSTRIDE_BOUND2 (new_i_stride2) = i_bound2;
                    WLGRID_BOUND1 (WLSTRIDE_CONTENTS (new_i_stride2))
                      = grid2_b1 - i_offset2;
                    WLGRID_BOUND2 (WLSTRIDE_CONTENTS (new_i_stride2))
                      = grid2_b2 - i_offset2;
                    new_i_stride2 = NormalizeStride_1 (new_i_stride2);
                }
            }

        } else {
            /*
             * intersection is empty
             *  -> free the useless data in 'i_stride1', 'i_stride2'
             */
            if (i_stride1 != NULL) {
                if (*i_stride1 != NULL) {
                    *i_stride1 = FreeTree (*i_stride1);
                }
            }
            if (i_stride2 != NULL) {
                if (*i_stride2 != NULL) {
                    *i_stride2 = FreeTree (*i_stride2);
                }
            }
            result = 0;

            /* we can give up here */
            break;
        }

        /* next dim */
        stride1 = WLGRID_NEXTDIM (grid1);
        stride2 = WLGRID_NEXTDIM (grid2);
        if (i_stride1 != NULL) {
            new_i_stride1 = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (new_i_stride1));
        }
        if (i_stride2 != NULL) {
            new_i_stride2 = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (new_i_stride2));
        }
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *SetSegs(node *pragma, node *cubes, int dims)
 *
 * description:
 *   returns chain of segments (based on the calculated cubes 'cubes')
 *
 ******************************************************************************/

node *
SetSegs (node *pragma, node *cubes, int dims)
{
    node *aps;
    node *segs = NULL;

    DBUG_ENTER ("SetSegs");

    /*
     * create default configuration
     */
#if 1 /* -> sac2c flag! */
    segs = All (segs, NULL, cubes, dims);
#else
    segs = Cubes (segs, NULL, cubes, dims);
#endif

    /*
     * create pragma-dependent configuration
     */
    if (pragma != NULL) {
        aps = PRAGMA_WLCOMP_APS (pragma);
        while (aps != NULL) {

#define WLP(fun, str)                                                                    \
    if (strcmp (AP_NAME (EXPRS_EXPR (aps)), str) == 0) {                                 \
        segs = fun (segs, AP_ARGS (EXPRS_EXPR (aps)), cubes, dims);                      \
    } else

#include "wlpragma_funs.mac"
            DBUG_ASSERT ((0), "wrong function name in wlcomp-pragma");

#undef WLP

            aps = EXPRS_NEXT (aps);
        }
    }

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * function:
 *   node *NewBoundsStride(node *stride, int dim,
 *                         int new_bound1, int new_bound2)
 *
 * description:
 *   returns modified 'stride':
 *     all strides in dimension "current dimension"+'dim' are new bounds
 *     given ('bound1', 'bound2').
 *
 ******************************************************************************/

node *
NewBoundsStride (node *stride, int dim, int new_bound1, int new_bound2)
{
    node *grids, *new_grids, *tmp, *tmp2;
    int bound1, step, grid_b1, grid_b2, offset;

    DBUG_ENTER ("NewBoundsStride");

    grids = WLSTRIDE_CONTENTS (stride);

    if (dim == 0) {
        /*
         * arrived at the correct dimension
         *  -> set new bounds
         *  -> correct the grids if necessary
         */

        bound1 = WLSTRIDE_BOUND1 (stride);
        if (bound1 != new_bound1) {
            /*
             * correct the grids
             */

            step = WLSTRIDE_STEP (stride);
            new_grids = NULL;
            do {
                grid_b1 = WLGRID_BOUND1 (grids);
                grid_b2 = WLGRID_BOUND2 (grids);

                offset = GridOffset (new_bound1, bound1, step, grid_b2);

                /* extract current grid from chain -> single grid in 'tmp' */
                tmp = grids;
                grids = WLGRID_NEXT (grids);
                WLGRID_NEXT (tmp) = NULL;

                if (offset <= grid_b1) {
                    /*
                     * grid is still in one pice :)
                     */

                    WLGRID_BOUND1 (tmp) = grid_b1 - offset;
                    WLGRID_BOUND2 (tmp) = grid_b2 - offset;

                    /* insert changed grid into 'new_grids' */
                    new_grids = InsertWLnodes (new_grids, tmp);
                } else {
                    /*
                     * the grid is split into two parts :(
                     */

                    /* first part: recycle old grid */
                    WLGRID_BOUND1 (tmp) = grid_b1 - offset + step;
                    WLGRID_BOUND2 (tmp) = step;
                    /* second part: duplicate old grid first */
                    tmp2 = DupNode (tmp);
                    WLGRID_BOUND1 (tmp2) = 0;
                    WLGRID_BOUND2 (tmp2) = grid_b2 - offset;
                    /* concate the two grids */
                    WLGRID_NEXT (tmp2) = tmp;

                    /* insert them into 'new_grids' */
                    new_grids = InsertWLnodes (new_grids, tmp2);
                }
            } while (grids != NULL);

            WLSTRIDE_CONTENTS (stride) = new_grids;
            WLSTRIDE_BOUND1 (stride) = new_bound1;
        }

        WLSTRIDE_BOUND2 (stride) = new_bound2;

    } else {
        /*
         * involve all grids of current dimension
         */

        do {
            WLGRID_NEXTDIM (grids)
              = NewBoundsStride (WLGRID_NEXTDIM (grids), dim - 1, new_bound1, new_bound2);
            grids = WLGRID_NEXT (grids);
        } while (grids != NULL);
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

    DBUG_VOID_RETURN
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

    DBUG_ASSERT ((NODE_TYPE (strides) == N_WLstride), "wrong node type found");

    /*
     * the outline of each stride is intersected with all the other ones.
     * this is done until no new intersections are generated (fixpoint).
     */
    do {
        fixpoint = 1;       /* initialize 'fixpoint' */
        new_strides = NULL; /* here we collect the new stride-set */

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
                    fixpoint = 0; /* no, not a fixpoint yet :( */
                    WLSTRIDE_MODIFIED (stride1) = WLSTRIDE_MODIFIED (stride2) = 1;
                    new_strides = InsertWLnodes (new_strides, split_stride1);
                    new_strides = InsertWLnodes (new_strides, split_stride2);
                } else {
                    DBUG_ASSERT ((split_stride2 == NULL), "wrong splitting");
                }

                stride2 = WLSTRIDE_NEXT (stride2);
            }

            /*
             * was 'stride1' not modified?
             *  -> it is a part of the result
             */
            if (WLSTRIDE_MODIFIED (stride1) == 0) {
                /* insert 'stride1' in 'new_strides' */
                tmp = stride1;
                stride1 = WLSTRIDE_NEXT (stride1);
                WLSTRIDE_NEXT (tmp) = NULL;
                new_strides = InsertWLnodes (new_strides, tmp);
            } else {
                /*
                 * 'stride1' was modified, hence no part of the result.
                 *  -> is no longer needed
                 */
                stride1 = FreeNode (stride1);
                /* 'stride1' points to his successor now! */
            }
        }

        strides = new_strides;
    } while (!fixpoint); /* fixpoint found? */

    DBUG_RETURN (strides);
}

/******************************************************************************
 *
 * function:
 *   node *BlockStride(node *stride, long *bv)
 *
 * description:
 *   returns 'stride' with corrected bounds, blocking levels and
 *     unrolling-flag.
 *   this function is needed after a blocking.
 *
 ******************************************************************************/

node *
BlockStride (node *stride, long *bv)
{
    node *curr_stride, *curr_grid, *grids;

    DBUG_ENTER ("BlockStride");

    if (stride != NULL) {

        DBUG_ASSERT ((NODE_TYPE (stride)), "no N_WLstride node found");

        curr_stride = stride;
        do {

            /* correct blocking level and unrolling-flag */
            WLSTRIDE_LEVEL (curr_stride)++;
            WLSTRIDE_UNROLLING (curr_stride) = 1; /* we want to unroll this stride */
            grids = WLSTRIDE_CONTENTS (curr_stride);
            do {
                WLGRID_LEVEL (grids)++;
                WLGRID_UNROLLING (grids) = 1; /* we want to unroll this grid */
                grids = WLGRID_NEXT (grids);
            } while (grids != NULL);

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
 *   'dims' is the number of dimensions in 'stride'.
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
             * ublock found ?!?! -> error
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
                    /*
                     * no blocking -> go to next dim
                     */
                    curr_grid = WLSTRIDE_CONTENTS (curr_stride);
                    do {
                        WLGRID_NEXTDIM (curr_grid)
                          = BlockWL (WLGRID_NEXTDIM (curr_grid), dims, bv, unroll);

                        curr_grid = WLGRID_NEXT (curr_grid);
                    } while (curr_grid != NULL);

                    curr_stride = WLSTRIDE_NEXT (curr_stride);
                } else {
                    /*
                     * blocking -> create a N_WLblock (N_WLublock respectively) node
                     *   for each following dim
                     */
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
 *   node *NewStepGrids(node *grids, int step, int new_step, int offset)
 *
 * description:
 *   returns the modified 'grids' chain:
 *     * the bounds of the grids are modified (relating to 'offset')
 *     * the step of the grids is now 'step'
 *        -> possibly the grids must be duplicated
 *
 ******************************************************************************/

node *
NewStepGrids (node *grids, int step, int new_step, int offset)
{
    node *last_old, *last, *new_grid, *tmp;
    int i, div;

    DBUG_ENTER ("NewStepGrids");

    DBUG_ASSERT ((new_step % step == 0), "wrong new step");

    if (step == 1) {
        DBUG_ASSERT ((WLGRID_BOUND1 (grids) == 0), "grid has wrong lower bound");
        DBUG_ASSERT ((WLGRID_NEXT (grids) == NULL), "grid has wrong bounds");
        WLGRID_BOUND2 (grids) = new_step;
    } else {
        div = new_step / step;

        /*
         * adjust bounds (relating to 'offset')
         *
         * search for last grid -> save it in 'last_old'
         */
        WLGRID_BOUND1 (grids) -= offset;
        WLGRID_BOUND2 (grids) -= offset;
        last_old = grids;
        while (WLGRID_NEXT (last_old) != NULL) {
            last_old = WLGRID_NEXT (last_old);
            WLGRID_BOUND1 (last_old) -= offset;
            WLGRID_BOUND2 (last_old) -= offset;
        }

        if (div > 1) {
            /*
             * duplicate all grids ('div' -1) times
             */
            last = last_old;
            for (i = 1; i < div; i++) {
                tmp = grids;
                do {
                    /* duplicate current grid */
                    new_grid = DupNode (tmp);
                    WLGRID_BOUND1 (new_grid) = WLGRID_BOUND1 (new_grid) + i * step;
                    WLGRID_BOUND2 (new_grid) = WLGRID_BOUND2 (new_grid) + i * step;

                    last = WLGRID_NEXT (last) = new_grid;
                } while (tmp != last_old);
            }
        }
    }

    DBUG_RETURN (grids);
}

/******************************************************************************
 *
 * function:
 *   node *IntersectGrid(node *grid1, node *grid2, int step,
 *                       node **i_grid1, node **i_grid2)
 *
 * description:
 *   returns in 'i_grid1', 'i_grid2' the intersection of 'grid1' and 'grid2'.
 *   both grids must have the same step ('step').
 *
 *   returns NULL if the intersection is equal to the original grid!!
 *
 ******************************************************************************/

void
IntersectGrid (node *grid1, node *grid2, int step, node **i_grid1, node **i_grid2)
{
    int bound11, bound21, bound12, bound22, i_bound1, i_bound2;

    DBUG_ENTER ("IntersectGrid");

    *i_grid1 = *i_grid2 = NULL;

    bound11 = WLGRID_BOUND1 (grid1);
    bound21 = WLGRID_BOUND2 (grid1);

    bound12 = WLGRID_BOUND1 (grid2);
    bound22 = WLGRID_BOUND2 (grid2);

    /* compute bounds of intersection */
    i_bound1 = MAX (bound11, bound12);
    i_bound2 = MIN (bound21, bound22);

    if (i_bound1 < i_bound2) { /* is intersection non-empty? */

        if ((i_bound1 != bound11) || (i_bound2 != bound21)) {
            *i_grid1 = DupNode (grid1);
            WLGRID_BOUND1 ((*i_grid1)) = i_bound1;
            WLGRID_BOUND2 ((*i_grid1)) = i_bound2;
        }

        if ((i_bound1 != bound12) || (i_bound2 != bound22)) {
            *i_grid2 = DupNode (grid2);
            WLGRID_BOUND1 ((*i_grid2)) = i_bound1;
            WLGRID_BOUND2 ((*i_grid2)) = i_bound2;
        }
    }

    DBUG_VOID_RETURN
}

/******************************************************************************
 *
 * function:
 *   node *MergeWL(node *nodes)
 *
 * description:
 *   returns the merged chain 'nodes'.
 *   if necessary (e.g. if called from 'ComputeCubes') the bounds of the
 *     chain-elements are adjusted.
 *
 ******************************************************************************/

node *
MergeWL (node *nodes)
{
    node *node1, *grids, *new_grids, *grid1, *grid2, *i_grid1, *i_grid2, *tmp;
    int bound1, bound2, step, rear1, count, fixpoint, i;

    DBUG_ENTER ("MergeWL");

    node1 = nodes;
    while (node1 != NULL) {

        /*
         * get all nodes with same bounds as 'node1'
         *
         * (because of the sort order these nodes are
         * located directly after 'node1' in the chain)
         */

        switch (NODE_TYPE (node1)) {

        case N_WLblock:
            /* here is no break missing! */
        case N_WLublock:
            /* here is no break missing! */
        case N_WLgrid:

            while ((WLNODE_NEXT (node1) != NULL)
                   && (WLNODE_BOUND1 (node1) == WLNODE_BOUND1 (WLNODE_NEXT (node1)))) {

                DBUG_ASSERT ((WLNODE_BOUND2 (node1)
                              == WLNODE_BOUND2 (WLNODE_NEXT (node1))),
                             "wrong bounds found");
                DBUG_ASSERT ((WLNODE_NEXTDIM (node1) != NULL), "dim not found");
                DBUG_ASSERT ((WLNODE_NEXTDIM (WLNODE_NEXT (node1)) != NULL),
                             "dim not found");

                /*
                 * merge 'node1' with his successor
                 */
                WLNODE_NEXTDIM (node1)
                  = InsertWLnodes (WLNODE_NEXTDIM (node1),
                                   WLNODE_NEXTDIM (WLNODE_NEXT (node1)));

                /* the remaining block node is useless now */
                WLNODE_NEXTDIM (WLNODE_NEXT (node1)) = NULL;
                WLNODE_NEXT (node1) = FreeNode (WLNODE_NEXT (node1));
                /* 'WLNODE_NEXT(node1)' points to his successor now */

                /* merge next dimension */
                WLNODE_NEXTDIM (node1) = MergeWL (WLNODE_NEXTDIM (node1));
            }
            break;

        case N_WLstride:

            /*
             * compute new bounds and step
             *             ^^^^^^
             * CAUTION: when called by 'ComputeCubes' the bounds are not equal!!
             */
            rear1 = IndexRearStride (node1);
            bound1 = WLSTRIDE_BOUND1 (node1);
            bound2 = WLSTRIDE_BOUND2 (node1);
            step = WLSTRIDE_STEP (node1);
            count = 0;
            tmp = WLSTRIDE_NEXT (node1);
            while ((tmp != NULL) && (IndexHeadStride (tmp) < rear1)) {
                /* compute new bounds */
                bound1 = MAX (bound1, WLSTRIDE_BOUND1 (tmp));
                bound2 = MIN (bound2, WLSTRIDE_BOUND2 (tmp));
                /* compute new step */
                step = lcm (step, WLSTRIDE_STEP (tmp));
                /* count the number of found dimensions for next traversal */
                count++;
                tmp = WLSTRIDE_NEXT (tmp);
            }

            /*
             * fit all grids to new step and collect them in 'grids'
             */
            grids = NewStepGrids (WLSTRIDE_CONTENTS (node1), WLSTRIDE_STEP (node1), step,
                                  bound1 - WLSTRIDE_BOUND1 (node1));
            for (i = 0; i < count; i++) {
                grids
                  = InsertWLnodes (grids,
                                   NewStepGrids (WLSTRIDE_CONTENTS (
                                                   WLSTRIDE_NEXT (node1)),
                                                 WLSTRIDE_STEP (WLSTRIDE_NEXT (node1)),
                                                 step,
                                                 bound1
                                                   - WLSTRIDE_BOUND1 (
                                                       WLSTRIDE_NEXT (node1))));

                /* the remaining block node is useless now */
                WLSTRIDE_CONTENTS (WLSTRIDE_NEXT (node1)) = NULL;
                WLSTRIDE_NEXT (node1) = FreeNode (WLSTRIDE_NEXT (node1));
                /* 'WLSTRIDE_NEXT(node1)' points to his successor now */
            }

            /*
             * intersect all grids with each other
             *   until fixpoint is reached.
             */
            do {
                fixpoint = 1;
                new_grids = NULL;

                /* check WLGRID_MODIFIED */
                grid1 = grids;
                while (grid1 != NULL) {
                    DBUG_ASSERT ((WLGRID_MODIFIED (grid1) == 0), "grid was modified");
                    grid1 = WLGRID_NEXT (grid1);
                }

                grid1 = grids;
                while (grid1 != NULL) {

                    grid2 = WLGRID_NEXT (grid1);
                    while (grid2 != NULL) {
                        IntersectGrid (grid1, grid2, step, &i_grid1, &i_grid2);
                        if (i_grid1 != NULL) {
                            new_grids = InsertWLnodes (new_grids, i_grid1);
                            WLGRID_MODIFIED (grid1) = 1;
                            fixpoint = 0;
                        }
                        if (i_grid2 != NULL) {
                            new_grids = InsertWLnodes (new_grids, i_grid2);
                            WLGRID_MODIFIED (grid2) = 1;
                            fixpoint = 0;
                        }

                        grid2 = WLGRID_NEXT (grid2);
                    }

                    /* was 'grid1' not modified? */
                    if (WLGRID_MODIFIED (grid1) == 0) {
                        /* insert 'grid1' in 'new_grids' */
                        tmp = grid1;
                        grid1 = WLGRID_NEXT (grid1);
                        WLGRID_NEXT (tmp) = NULL;
                        new_grids = InsertWLnodes (new_grids, tmp);
                    } else {
                        /* 'grid1' is no longer needed */
                        grid1 = FreeNode (grid1);
                        /* 'grid1' points to his successor now! */
                    }
                }

                grids = new_grids;
            } while (!fixpoint);

            /*
             * merge the grids
             */
            WLSTRIDE_BOUND1 (node1) = bound1;
            WLSTRIDE_BOUND2 (node1) = bound2;
            WLSTRIDE_STEP (node1) = step;
            WLSTRIDE_CONTENTS (node1) = MergeWL (grids);
            break;

        default:

            DBUG_ASSERT ((0), "wrong node type");
        }

        node1 = WLNODE_NEXT (node1);
    }

    DBUG_RETURN (nodes);
}

/******************************************************************************
 *
 * function:
 *   int IsEqualWLnodes(node *tree1, node *tree2)
 *
 * description:
 *   returns 1 if the N_WL...-trees 'tree1' and 'tree2' are equal.
 *   returns 0 otherwise.
 *
 *   remark: we can not use 'CompareWlnodes' here, because this function only
 *           compares the first level of dimensions.
 *           but here we must compare the *whole* trees --- all block levels,
 *           and even the code ...
 *
 ******************************************************************************/

int
IsEqualWLnodes (node *tree1, node *tree2)
{
    node *tmp1, *tmp2;
    int equal = 1;

    DBUG_ENTER ("IsEqualWLnodes");

    if ((tree1 != NULL) && (tree2 != NULL)) {

        DBUG_ASSERT ((NODE_TYPE (tree1) == NODE_TYPE (tree2)),
                     "can not compare objects of different type");

        /*
         * compare the whole chains
         */
        tmp1 = tree1;
        tmp2 = tree2;
        do {

            /*
             * compare type-independent data
             */
            if ((WLNODE_BOUND1 (tmp1) == WLNODE_BOUND1 (tmp2))
                && (WLNODE_BOUND2 (tmp1) == WLNODE_BOUND2 (tmp2))
                && (WLNODE_STEP (tmp1) == WLNODE_STEP (tmp2))) {

                /*
                 * compare type-specific data
                 */
                switch (NODE_TYPE (tmp1)) {

                case N_WLblock:
                    /* here is no break missing! */
                case N_WLublock:

                    /*
                     * CAUTION: to prevent nice ;-) bugs in this code fragment
                     *          WLBLOCK_CONTENTS and WLUBLOCK_CONTENTS must be
                     *          equivalent (as currently realized in tree_basic.h)
                     */
                    if (WLNODE_NEXTDIM (tmp1) != NULL) {
                        /*
                         * compare NEXTDIM (CONTENTS is NULL)
                         */
                        DBUG_ASSERT ((WLBLOCK_CONTENTS (tmp1) == NULL),
                                     "data in NEXTDIM *and* CONTENTS found");
                        DBUG_ASSERT ((WLBLOCK_CONTENTS (tmp2) == NULL),
                                     "data in NEXTDIM *and* CONTENTS found");
                        equal
                          = IsEqualWLnodes (WLNODE_NEXTDIM (tmp1), WLNODE_NEXTDIM (tmp2));
                    } else {
                        /*
                         * compare CONTENTS (NEXTDIM is NULL)
                         */
                        equal = IsEqualWLnodes (WLBLOCK_CONTENTS (tmp1),
                                                WLBLOCK_CONTENTS (tmp2));
                    }
                    break;

                case N_WLstride:

                    equal = IsEqualWLnodes (WLSTRIDE_CONTENTS (tmp1),
                                            WLSTRIDE_CONTENTS (tmp2));
                    break;

                case N_WLgrid:

                    if (WLGRID_NEXTDIM (tmp1) != NULL) {
                        /*
                         * compare NEXTDIM (CODE is NULL)
                         */
                        DBUG_ASSERT ((WLGRID_CODE (tmp1) == NULL),
                                     "data in NEXTDIM *and* CODE found");
                        DBUG_ASSERT ((WLGRID_CODE (tmp2) == NULL),
                                     "data in NEXTDIM *and* CODE found");
                        equal
                          = IsEqualWLnodes (WLGRID_NEXTDIM (tmp1), WLGRID_NEXTDIM (tmp2));
                    } else {
                        /*
                         * compare CODE (NEXTDIM is NULL)
                         */
                        equal = (WLGRID_CODE (tmp1) == WLGRID_CODE (tmp2));
                    }
                    break;

                default:

                    DBUG_ASSERT ((0), "wrong node type");
                }

            } else {
                equal = 0;
            }

            tmp1 = WLNODE_NEXT (tmp1);
            tmp2 = WLNODE_NEXT (tmp2);
        } while (equal && (tmp1 != NULL));

    } else {
        DBUG_ASSERT (((tree1 == NULL) && (tree2 == NULL)),
                     "trees differ in their depths");
        equal = 1;
    }

    DBUG_RETURN (equal);
}

/******************************************************************************
 *
 * function:
 *   node *OptimizeWL(node *nodes)
 *
 * description:
 *   returns the optimized N_WL...-tree 'nodes'.
 *
 ******************************************************************************/

node *
OptimizeWL (node *nodes)
{
    node *next, *grids, *comp1, *comp2;
    int offset;

    DBUG_ENTER ("OptimizeWL");

    if (nodes != NULL) {

        /*
         * optimize the next node
         */
        next = WLNODE_NEXT (nodes) = OptimizeWL (WLNODE_NEXT (nodes));

        /*
         * optimize the type-specific sons
         *
         * save in 'comp1', 'comp2' the son of 'nodes', 'next' respectively.
         */
        switch (NODE_TYPE (nodes)) {

        case N_WLblock:
            /* here is no break missing! */
        case N_WLublock:

            if (WLBLOCK_NEXTDIM (nodes) != NULL) {
                /*
                 * compare NEXTDIM (CONTENTS is NULL)
                 */
                DBUG_ASSERT ((WLBLOCK_CONTENTS (nodes) == NULL),
                             "data in NEXTDIM *and* CONTENTS found");
                comp1 = WLBLOCK_NEXTDIM (nodes) = OptimizeWL (WLBLOCK_NEXTDIM (nodes));
                if (next != NULL) {
                    comp2 = WLBLOCK_NEXTDIM (next);
                }
            } else {
                /*
                 * compare CONTENTS (NEXTDIM is NULL)
                 */
                comp1 = WLBLOCK_CONTENTS (nodes) = OptimizeWL (WLBLOCK_CONTENTS (nodes));
                if (next != NULL) {
                    comp2 = WLBLOCK_CONTENTS (next);
                }
            }
            break;

        case N_WLstride:

            comp1 = WLSTRIDE_CONTENTS (nodes) = OptimizeWL (WLSTRIDE_CONTENTS (nodes));
            if (next != NULL) {
                comp2 = WLSTRIDE_CONTENTS (next);
            }

            /*
             * if the grids contained in the stride have an offset
             * (the first grid does not begin at index 0), remove this offset.
             */
            grids = comp1;
            DBUG_ASSERT ((grids != NULL), "no grid found");
            offset = WLGRID_BOUND1 (grids);
            WLSTRIDE_BOUND1 (nodes) += offset;
            if (offset > 0) {
                do {
                    WLGRID_BOUND1 (grids) -= offset;
                    WLGRID_BOUND2 (grids) -= offset;
                    grids = WLGRID_NEXT (grids);
                } while (grids != NULL);
            }

            /*
             * if the first (and only) grid fills the whole step range
             *   set upper bound of this grid and step to 1
             */
            DBUG_ASSERT ((comp1 != NULL), "no grid found");
            if ((WLGRID_BOUND1 (comp1) == 0)
                && (WLGRID_BOUND2 (comp1) == WLSTRIDE_STEP (nodes))) {
                WLGRID_BOUND2 (comp1) = WLSTRIDE_STEP (nodes) = 1;
            }
            break;

        case N_WLgrid:

            if (WLGRID_NEXTDIM (nodes) != NULL) {
                /*
                 * compare NEXTDIM (CODE is NULL)
                 */
                DBUG_ASSERT ((WLGRID_CODE (nodes) == NULL),
                             "data in NEXTDIM *and* CODE found");
                comp1 = WLGRID_NEXTDIM (nodes) = OptimizeWL (WLGRID_NEXTDIM (nodes));
                if (next != NULL) {
                    comp2 = WLGRID_NEXTDIM (next);
                }
            } else {
                /*
                 * compare CODE (NEXTDIM is NULL)
                 */
                comp1 = WLGRID_CODE (nodes);
                if (next != NULL) {
                    comp2 = WLGRID_CODE (next);
                }
            }
            break;

        default:

            DBUG_ASSERT ((0), "wrong node type");
        }

        /*
         * if 'comp1' and 'comp2' are equal subtrees
         *   we can concate 'nodes' and 'next'
         */
        if (next != NULL) {
            if ((WLNODE_STEP (nodes) == WLNODE_STEP (next))
                && (WLNODE_BOUND2 (nodes) == WLNODE_BOUND1 (next))) {
                if ((NODE_TYPE (comp1) != N_Ncode) ? IsEqualWLnodes (comp1, comp2)
                                                   : (comp1 == comp2)) {
                    /* concate 'nodes' and 'next' */
                    WLNODE_BOUND2 (nodes) = WLNODE_BOUND2 (next);
                    /* free useless data in 'next' */
                    WLNODE_NEXT (nodes) = FreeNode (WLNODE_NEXT (nodes));
                }
            }
        }
    }

    DBUG_RETURN (nodes);
}

/******************************************************************************
 *
 * function:
 *   int GetMaxUnroll(node *nodes, int unroll, int dim)
 *
 * description:
 *   returns the maximally number elements that must be unrolled
 *     in dimension 'dim' of N_WL...-tree 'nodes'.
 *   'unroll' is the initial value for the computation (normally 1).
 *
 *   we must search for the first N_WLublock- or N_WLstride-node in each
 *     leaf of the 'nodes'-tree and get the step of this node.
 *
 ******************************************************************************/

int
GetMaxUnroll (node *nodes, int unroll, int dim)
{
    DBUG_ENTER ("GetMaxUnroll");

    if (nodes != NULL) {

        unroll = GetMaxUnroll (WLNODE_NEXT (nodes), unroll, dim);

        if ((WLNODE_DIM (nodes) == dim)
            && ((NODE_TYPE (nodes) == N_WLublock) || (NODE_TYPE (nodes) == N_WLstride))) {

            /*
             * we have found a node with unrolling information
             */
            unroll = MAX (unroll, WLNODE_STEP (nodes));

        } else {

            /*
             * search in whole tree for nodes with unrolling information
             */
            switch (NODE_TYPE (nodes)) {

            case N_WLblock:
                /* here is no break missing! */
            case N_WLublock:

                unroll = GetMaxUnroll (WLBLOCK_NEXTDIM (nodes), unroll, dim);
                unroll = GetMaxUnroll (WLBLOCK_CONTENTS (nodes), unroll, dim);
                break;

            case N_WLstride:

                unroll = GetMaxUnroll (WLSTRIDE_CONTENTS (nodes), unroll, dim);
                break;

            case N_WLgrid:

                unroll = GetMaxUnroll (WLBLOCK_NEXTDIM (nodes), unroll, dim);
                break;

            default:

                DBUG_ASSERT ((0), "wrong node type");
            }
        }
    }

    DBUG_RETURN (unroll);
}

/******************************************************************************
 *
 * function:
 *   node *FitWL(node *nodes, int curr_dim, int dims)
 *
 * description:
 *   returns the fitted N_WL...-tree 'nodes'.
 *   the tree is fitted in the dimension from 'curr_dim' till ('dims'-1).
 *
 ******************************************************************************/

node *
FitWL (node *nodes, int curr_dim, int dims)
{
    node *new_node, *grids, *tmp;
    int unroll, remain, width;

    DBUG_ENTER ("FitWL");

    if (curr_dim < dims) {

        /*
         * traverse the whole chain
         */
        tmp = nodes;
        while (tmp != NULL) {

            switch (NODE_TYPE (tmp)) {

            case N_WLblock:

                if (curr_dim < dims - 1) {
                    /*
                     * fit in next dimension;
                     *   compute unrolling information
                     */
                    WLBLOCK_NEXTDIM (tmp)
                      = FitWL (WLBLOCK_NEXTDIM (tmp), curr_dim + 1, dims);
                    unroll = GetMaxUnroll (WLBLOCK_NEXTDIM (tmp), 1, curr_dim);
                } else {
                    unroll = GetMaxUnroll (WLBLOCK_CONTENTS (tmp), 1, curr_dim);
                }
                break;

            case N_WLublock:

                if (curr_dim < dims - 1) {
                    /*
                     * fit in next dimension;
                     *   get unrolling information
                     */
                    WLBLOCK_NEXTDIM (tmp)
                      = FitWL (WLBLOCK_NEXTDIM (tmp), curr_dim + 1, dims);
                }
                unroll = WLUBLOCK_STEP (tmp);
                break;

            case N_WLstride:

                grids = WLSTRIDE_CONTENTS (tmp);
                if (curr_dim < dims - 1) {
                    /*
                     * fit for all grids in next dimension;
                     *   get unrolling information
                     */
                    while (grids != NULL) {
                        WLGRID_NEXTDIM (grids)
                          = FitWL (WLGRID_NEXTDIM (grids), curr_dim + 1, dims);
                        grids = WLGRID_NEXT (grids);
                    }
                }
                unroll = WLSTRIDE_STEP (tmp);
                break;

            default:

                DBUG_ASSERT ((0), "wrong node type");
            }

            /*
             * fit current dimension:
             *   split a uncompleted periode at the end of index range
             */
            width = WLNODE_BOUND2 (tmp) - WLNODE_BOUND1 (tmp);
            remain = width % unroll;
            if ((remain > 0) && (width > remain)) {
                /*
                 *  uncompleted periode found -> split
                 */
                new_node = DupNode (tmp);
                WLNODE_BOUND2 (new_node) = WLNODE_BOUND2 (tmp);
                WLNODE_BOUND2 (tmp) = WLNODE_BOUND1 (new_node)
                  = WLNODE_BOUND2 (tmp) - remain;
                WLNODE_NEXT (new_node) = WLNODE_NEXT (tmp);
                WLNODE_NEXT (tmp) = new_node;
                tmp = new_node;
            }

            tmp = WLNODE_NEXT (tmp);
        }
    }

    DBUG_RETURN (nodes);
}

/******************************************************************************
 *
 * function:
 *   node *NormalizeWLnodes(node *nodes, int *width)
 *
 * description:
 *   returns the normalized N_WL...-tree 'nodes'.
 *   'width' is an array with one component for each dimension;
 *     here we save the width of the index ranges
 *
 ******************************************************************************/

node *
NormalizeWLnodes (node *nodes, int *width)
{
    node *tmp;
    int curr_width;

    DBUG_ENTER ("NormalizeWLnodes");

    if (nodes != NULL) {

        /*
         * backup width of current dim
         */
        curr_width = width[WLNODE_DIM (nodes)];

        tmp = nodes;
        do {

            /*
             * adjust upper bound
             */
            DBUG_ASSERT ((WLNODE_BOUND1 (tmp) < curr_width), "wrong bounds found");
            WLNODE_BOUND2 (tmp) = MIN (WLNODE_BOUND2 (tmp), curr_width);

            /*
             * remove nodes whose index ranges lies outside the current block
             */
            while ((WLNODE_NEXT (tmp) != NULL)
                   && (WLNODE_BOUND1 (WLNODE_NEXT (tmp)) >= curr_width)) {
                WLNODE_NEXT (tmp) = FreeNode (WLNODE_NEXT (tmp));
            }

            /* take next node */
            tmp = WLNODE_NEXT (tmp);
        } while (tmp != NULL);

        tmp = nodes;
        do {

            /*
             * save width of current index range; adjust step
             */
            width[WLNODE_DIM (tmp)] = WLNODE_BOUND2 (tmp) - WLNODE_BOUND1 (tmp);
            WLNODE_STEP (tmp) = MIN (WLNODE_STEP (tmp), width[WLNODE_DIM (tmp)]);

            /*
             * normalize the type-specific sons
             */
            switch (NODE_TYPE (tmp)) {

            case N_WLblock:
                /* here is no break missing! */
            case N_WLublock:

                WLBLOCK_NEXTDIM (tmp) = NormalizeWLnodes (WLBLOCK_NEXTDIM (tmp), width);
                WLBLOCK_CONTENTS (tmp) = NormalizeWLnodes (WLBLOCK_CONTENTS (tmp), width);
                break;

            case N_WLstride:

                WLSTRIDE_CONTENTS (tmp)
                  = NormalizeWLnodes (WLSTRIDE_CONTENTS (tmp), width);
                break;

            case N_WLgrid:

                WLGRID_NEXTDIM (tmp) = NormalizeWLnodes (WLGRID_NEXTDIM (tmp), width);
                break;

            default:

                DBUG_ASSERT ((0), "wrong node type");
            }

            /* take next node */
            tmp = WLNODE_NEXT (tmp);
        } while (tmp != NULL);

        /*
         * restore width of current dim
         */
        width[WLNODE_DIM (nodes)] = curr_width;
    }

    DBUG_RETURN (nodes);
}

/******************************************************************************
 *
 * function:
 *   node *NormalizeWL(node *nodes, int dims)
 *
 * description:
 *   returns the normalized N_WL...-tree 'nodes'.
 *   'dims' is the number of dimension in 'nodes'.
 *
 ******************************************************************************/

node *
NormalizeWL (node *nodes, int dims)
{
    int *width;
    int d;

    DBUG_ENTER ("NormalizeWL");

    /*
     * initialize 'width' with the maximum value for int
     */
    width = (int *)MALLOC (dims * sizeof (int));
    for (d = 0; d < dims; d++) {
        width[d] = INT_MAX;
    }

    nodes = NormalizeWLnodes (nodes, width);

    FREE (width);

    DBUG_RETURN (nodes);
}

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
    node *new_strides, *stride1, *stride2, *i_stride1, *i_stride2, *remain, *last_remain,
      *last_stride1, *tmp;
    int fixpoint;

    DBUG_ENTER ("ComputeCubes");

    DBUG_ASSERT ((NODE_TYPE (strides) == N_WLstride), "wrong node type found");

    /*
     * create disjunct outlines
     *  -> every stride lies in one and only one cube
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

                if (i_stride1 != NULL) {
                    if (CompareWLnode (stride1, i_stride1, 1) != 0) {
                        fixpoint = 0;
                        WLSTRIDE_MODIFIED (stride1) = 1;
                        new_strides = InsertWLnodes (new_strides, i_stride1);
                    } else {
                        /*
                         * 'stride1' and 'i_stride1' are equal
                         *  -> free 'i_stride1'
                         */
                        i_stride1 = FreeTree (i_stride1);
                    }
                }

                if (i_stride2 != NULL) {
                    if (CompareWLnode (stride2, i_stride2, 1) != 0) {
                        fixpoint = 0;
                        WLSTRIDE_MODIFIED (stride2) = 1;
                        new_strides = InsertWLnodes (new_strides, i_stride2);
                    } else {
                        /*
                         * 'stride2' and 'i_stride2' are equal
                         *  -> free 'i_stride2'
                         */
                        i_stride2 = FreeTree (i_stride2);
                    }
                }

                stride2 = WLSTRIDE_NEXT (stride2);
            }

            /* was 'stride1' not modified? */
            if (WLSTRIDE_MODIFIED (stride1) == 0) {
                /* insert 'stride1' in 'new_strides' */
                tmp = stride1;
                stride1 = WLSTRIDE_NEXT (stride1);
                WLSTRIDE_NEXT (tmp) = NULL;
                new_strides = InsertWLnodes (new_strides, tmp);
            } else {
                /* 'stride1' is no longer needed */
                stride1 = FreeNode (stride1);
                /* 'stride1' points to his successor now! */
            }
        }

        strides = new_strides;
    } while (!fixpoint);

    /*
     * merge the strides of each cube
     */
    stride1 = strides;
    while (stride1 != NULL) {

        /*
         * collect all strides, that lie in the same cube as 'stride1'.
         * 'remain' collects the remaining strides.
         */
        stride2 = WLSTRIDE_NEXT (stride1);
        last_stride1 = NULL;
        remain = last_remain = NULL;
        while (stride2 != NULL) {

            /* lie 'stride1' and 'stride2' in the same cube? */
            if (IntersectOutline (stride1, stride2, NULL, NULL)) {
                /*
                 * 'stride1' and 'stride2' lie in the same cube
                 *  -> append 'stride2' to the 'stride1'-chain
                 */
                if (last_stride1 == NULL) {
                    WLSTRIDE_NEXT (stride1) = stride2;
                } else {
                    WLSTRIDE_NEXT (last_stride1) = stride2;
                }
                last_stride1 = stride2;
            } else {
                /*
                 * 'stride2' lies not in the same cube as 'stride1'
                 *  -> append 'stride2' to to 'remain'-chain
                 */
                if (remain == NULL) {
                    remain = stride2;
                } else {
                    WLSTRIDE_NEXT (last_remain) = stride2;
                }
                last_remain = stride2;
            }

            stride2 = WLSTRIDE_NEXT (stride2);
        }

        /*
         * merge the 'stride1'-chain
         */
        if (last_stride1 != NULL) {
            WLSTRIDE_NEXT (last_stride1) = NULL;
            stride1 = MergeWL (stride1);
        }

        if (strides == NULL) {
            strides = stride1;
        }

        WLSTRIDE_NEXT (stride1) = remain;
        if (last_remain != NULL) {
            WLSTRIDE_NEXT (last_remain) = NULL;
        }
        stride1 = remain;
    }
    strides = new_strides;

    DBUG_RETURN (strides);
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
    node *new_node, *strides, *cubes, *segs, *seg;
    int dims, b;
    enum {
        PREC_PH_cube,
        PREC_PH_seg,
        PREC_PH_split,
        PREC_PH_block,
        PREC_PH_ublock,
        PREC_PH_merge,
        PREC_PH_opt,
        PREC_PH_fit,
        PREC_PH_norm
    } PREC_break_after;

    DBUG_ENTER ("PRECnwith");

    /* analyse 'break_specifier' */
    PREC_break_after = PREC_PH_norm;
    if (break_after == PH_precompile) {
        if (strcmp (break_specifier, "cubes") == 0) {
            PREC_break_after = PREC_PH_cube;
        } else {
            if (strcmp (break_specifier, "segs") == 0) {
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
                                    if (strcmp (break_specifier, "fit") == 0) {
                                        PREC_break_after = PREC_PH_fit;
                                    } else {
                                        if (strcmp (break_specifier, "norm") == 0) {
                                            PREC_break_after = PREC_PH_norm;
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

    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

    new_node = MakeNWith2 (NPART_WITHID (NWITH_PART (arg_node)), NULL,
                           NWITH_CODE (arg_node), NWITH_WITHOP (arg_node));

    /*
     * withid, code and withop are overtaken to the nwith2-tree without a change.
     * because of that, these parts are cut off from the old nwith-tree,
     * before freeing it.
     */
    NPART_WITHID (NWITH_PART (arg_node)) = NULL;
    NWITH_CODE (arg_node) = NULL;
    NWITH_WITHOP (arg_node) = NULL;

    /*
     * get number of dims of with-loop index range
     */
    dims
      = SHPSEG_SHAPE (VARDEC_SHPSEG (IDS_VARDEC (NWITHID_VEC (NWITH2_WITHID (new_node)))),
                      0);

    DBUG_EXECUTE ("WLprec", NOTE (("step 0: converting parts to strides\n")));
    strides = Parts2Strides (NWITH_PART (arg_node), dims);

    if (NODE_TYPE (strides) == N_WLstride) {

        /*
         * the generator params are constant
         */

        DBUG_EXECUTE ("WLprec", NOTE (("step 1: cube-building\n")));
        cubes = ComputeCubes (strides);
        if (PREC_break_after == PREC_PH_cube) {
            /*
             * build one segment containing all the cubes
             */
            segs = MakeWLseg (dims, cubes, NULL);
        }

        if (PREC_break_after >= PREC_PH_seg) {
            DBUG_EXECUTE ("WLprec", NOTE (("step 2: choice of segments\n")));
            segs = SetSegs (NWITH_PRAGMA (arg_node), cubes, dims);
            /* free temporary data */
            if (NWITH_PRAGMA (arg_node) != NULL) {
                NWITH_PRAGMA (arg_node) = FreeTree (NWITH_PRAGMA (arg_node));
            }
            if (cubes != NULL) {
                cubes = FreeTree (cubes);
            }

            seg = segs;
            while (seg != NULL) {
                /* splitting */
                if (PREC_break_after >= PREC_PH_split) {
                    DBUG_EXECUTE ("WLprec", NOTE (("step 3: splitting\n")));
                    WLSEG_CONTENTS (seg) = SplitWL (WLSEG_CONTENTS (seg));
                }

                /* hierarchical blocking */
                if (PREC_break_after >= PREC_PH_block) {
                    DBUG_EXECUTE ("WLprec", NOTE (("step 4: hierarchical blocking\n")));
                    for (b = 0; b < WLSEG_BLOCKS (seg); b++) {
                        DBUG_EXECUTE ("WLprec",
                                      NOTE (
                                        ("step 4.%d: hierarchical blocking (level %d)\n",
                                         b + 1, b)));
                        WLSEG_CONTENTS (seg)
                          = BlockWL (WLSEG_CONTENTS (seg), dims, WLSEG_BV (seg, b), 0);
                        FREE (WLSEG_BV (seg, b));
                    }
                }

                /* unrolling-blocking */
                if (PREC_break_after >= PREC_PH_ublock) {
                    DBUG_EXECUTE ("WLprec", NOTE (("step 5: unrolling-blocking\n")));
                    WLSEG_CONTENTS (seg)
                      = BlockWL (WLSEG_CONTENTS (seg), dims, WLSEG_UBV (seg), 1);
                    FREE (WLSEG_UBV (seg));
                    FREE (WLSEG_SV (seg));
                }

                /* merging */
                if (PREC_break_after >= PREC_PH_merge) {
                    DBUG_EXECUTE ("WLprec", NOTE (("step 6: merging\n")));
                    WLSEG_CONTENTS (seg) = MergeWL (WLSEG_CONTENTS (seg));
                }

                /* optimization */
                if (PREC_break_after >= PREC_PH_opt) {
                    DBUG_EXECUTE ("WLprec", NOTE (("step 7: optimization\n")));
                    WLSEG_CONTENTS (seg) = OptimizeWL (WLSEG_CONTENTS (seg));
                }

                /* fitting */
                if (PREC_break_after >= PREC_PH_fit) {
                    DBUG_EXECUTE ("WLprec", NOTE (("step 8: fitting\n")));
                    WLSEG_CONTENTS (seg) = FitWL (WLSEG_CONTENTS (seg), 0, dims);
                }

                /* normalization */
                if (PREC_break_after >= PREC_PH_norm) {
                    DBUG_EXECUTE ("WLprec", NOTE (("step 9: normalization\n")));
                    WLSEG_CONTENTS (seg) = NormalizeWL (WLSEG_CONTENTS (seg), dims);
                }

                seg = WLSEG_NEXT (seg);
            }
        }

    } else {

        /*
         * not all generator params are constant
         */

        /*
         * build one segment only
         */
        segs = MakeWLseg (dims, strides, NULL);
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

/*
 * precompilation of new with-loop
 *
 ******************************************************************************/
