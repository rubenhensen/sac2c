/*
 *
 * $Log$
 * Revision 3.11  2002/02/21 15:45:40  dkr
 * TOFDEF_..., ..._TOF access macros added
 *
 * Revision 3.10  2001/11/16 14:02:41  cg
 * sac2c now supports preprocessor directives in module/class
 * declaration files.
 *
 * Revision 3.9  2001/07/19 08:54:27  cg
 * Warnings eliminated.
 *
 * Revision 3.8  2001/07/18 12:57:45  cg
 * Applications of old tree construction function
 * AppendNodeChain eliminated.
 *
 * Revision 3.7  2001/05/17 13:08:53  nmw
 * MALLOC/FREE replaced by Malloc/Free, using result of Free()
 *
 * Revision 3.6  2001/04/24 09:15:45  dkr
 * P_FORMAT replaced by F_PTR
 *
 * Revision 3.5  2001/03/22 19:33:41  dkr
 * no changes done
 *
 * Revision 3.4  2001/02/14 10:16:42  dkr
 * MakeNode eliminated
 *
 * Revision 3.3  2001/02/02 10:13:44  dkr
 * superfluous import of access_macros.h removed
 *
 * Revision 3.2  2001/01/26 15:43:01  cg
 * Bug fixed in FindSymbolInModule.
 * Internal module representations arising from SIB interpretation
 * are now handled correctly.
 *
 * Revision 3.1  2000/11/20 18:00:54  sacbase
 * new release made
 *
 * Revision 2.7  2000/11/17 16:16:02  sbs
 * locationtype field added in DEPS structure.
 * This is used for implementing -MM and -MMlib options!
 * => some extensions of PrintDependencies
 * => some adjustments of MakeDeps calls done.
 *
 * Revision 2.6  2000/11/15 14:32:44  sbs
 * {}'s added in order to please gcc.
 *
 * Revision 2.5  2000/10/24 11:48:02  dkr
 * MakeTypes renamed into MakeTypes1
 *
 * Revision 2.4  2000/08/04 17:19:32  dkr
 * NEWTREE removed
 *
 * Revision 2.3  2000/02/23 20:16:34  cg
 * Node status ST_imported replaced by ST_imported_mod and
 * ST_imported_class in order to allow distinction between enteties
 * that are imported from a module and those that are imported from a
 * class.
 *
 * Revision 2.2  1999/05/06 15:38:46  sbs
 * call of yyparse changed to My_yyparse.
 *
 * [...]
 *
 */

#include <string.h>
#include <limits.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"

#include "free.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "globals.h"

#include "scnprs.h"
#include "traverse.h"

#include "filemgr.h"
#include "import.h"

#define MODUL_TOF(n, id)                                                                 \
    (id == 0)                                                                            \
      ? (MODUL_TYPES (n))                                                                \
      : ((id == 1)                                                                       \
           ? (MODUL_TYPES (n))                                                           \
           : ((id == 2) ? (MODUL_FUNS (n)) : ((id == 3) ? (MODUL_OBJS (n)) : NULL)))

#define MODUL_TOF_L(n, id, rhs)                                                          \
    if (id == 0) {                                                                       \
        MODUL_TYPES (n) = rhs;                                                           \
    } else if (id == 1) {                                                                \
        MODUL_TYPES (n) = rhs;                                                           \
    } else if (id == 2) {                                                                \
        MODUL_FUNS (n) = rhs;                                                            \
    } else if (id == 3) {                                                                \
        MODUL_OBJS (n) = rhs;                                                            \
    } else {                                                                             \
        DBUG_ASSERT ((0), "illegal symbol type found");                                  \
    }

#define EXPLIST_TOF(n, id)                                                               \
    (id == 0)                                                                            \
      ? (EXPLIST_ITYPES (n))                                                             \
      : ((id == 1)                                                                       \
           ? (EXPLIST_ETYPES (n))                                                        \
           : ((id == 2) ? (EXPLIST_FUNS (n)) : ((id == 3) ? (EXPLIST_OBJS (n)) : NULL)))

#define EXPLIST_TOF_L(n, id, rhs)                                                        \
    if (id == 0) {                                                                       \
        EXPLIST_ITYPES (n) = rhs;                                                        \
    } else if (id == 1) {                                                                \
        EXPLIST_ETYPES (n) = rhs;                                                        \
    } else if (id == 2) {                                                                \
        EXPLIST_FUNS (n) = rhs;                                                          \
    } else if (id == 3) {                                                                \
        EXPLIST_OBJS (n) = rhs;                                                          \
    } else {                                                                             \
        DBUG_ASSERT ((0), "illegal symbol type found");                                  \
    }

#define TOFDEF_NAME(n)                                                                   \
    (NODE_TYPE (n) == N_typedef)                                                         \
      ? (TYPEDEF_NAME (n))                                                               \
      : ((NODE_TYPE (n) == N_fundef)                                                     \
           ? (FUNDEF_NAME (n))                                                           \
           : ((NODE_TYPE (n) == N_objdef) ? (OBJDEF_NAME (n)) : NULL))

#define TOFDEF_TYPES(n)                                                                  \
    (NODE_TYPE (n) == N_typedef)                                                         \
      ? (TYPEDEF_TYPE (n))                                                               \
      : ((NODE_TYPE (n) == N_fundef)                                                     \
           ? (FUNDEF_TYPES (n))                                                          \
           : ((NODE_TYPE (n) == N_objdef) ? (OBJDEF_TYPE (n)) : NULL))

#define TOFDEF_NEXT(n)                                                                   \
    (NODE_TYPE (n) == N_typedef)                                                         \
      ? (TYPEDEF_NEXT (n))                                                               \
      : ((NODE_TYPE (n) == N_fundef)                                                     \
           ? (FUNDEF_NEXT (n))                                                           \
           : ((NODE_TYPE (n) == N_objdef) ? (OBJDEF_NEXT (n)) : NULL))

#define TOFDEF_NEXT_L(n, rhs)                                                            \
    if (NODE_TYPE (n) == N_typedef) {                                                    \
        TYPEDEF_NEXT (n) = rhs;                                                          \
    } else if (NODE_TYPE (n) == N_fundef) {                                              \
        FUNDEF_NEXT (n) = rhs;                                                           \
    } else if (NODE_TYPE (n) == N_objdef) {                                              \
        OBJDEF_NEXT (n) = rhs;                                                           \
    } else {                                                                             \
        DBUG_ASSERT ((0), "illegal symbol nodetype found");                              \
    }

extern void DoImport (node *modul, node *implist, char *mastermod);

static mod *mod_tab = NULL;

/*
 *
 *  functionname  : Import
 *  arguments     : 1) syntax tree
 *  description   : Recursively scans and parses modul.dec's
 *  global vars   : syntax_tree, act_tab, imp_tab
 *
 */

node *
Import (node *arg_node)
{
    DBUG_ENTER ("Import");

    act_tab = imp_tab;

    filename = puresacfilename;
    /*
     * The global variable filename is used for generating error messages.
     * After all imports have been done, it is reset to the original file
     * to be compiled.
     */

    DBUG_RETURN (Trav (arg_node, NULL));
}

/*
 *
 *  functionname  : FindModul
 *  arguments     : 1) name of modul to be looked for
 *  description   : finds the entry of given modul in the global mod_tab
 *  global vars   : mod_tab
 *
 */

mod *
FindModul (char *name)
{
    mod *tmp;

    DBUG_ENTER ("FindModul");

    DBUG_PRINT ("IMPORT", ("searching for modul/class: %s", name));

    tmp = mod_tab;
    while ((tmp != NULL) && (strcmp (tmp->name, name) != 0)) {
        tmp = tmp->next;
    }

    DBUG_RETURN (tmp);
}

/*
 *
 *  functionname  : FreeMods
 *  arguments     : 1) mods-list to be free'd
 *  description   : frees the mods-list given
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : free
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */

void
FreeMods (mods *mod)
{
    mods *tmp;

    DBUG_ENTER ("FreeMods");

    while (mod != NULL) {
        tmp = mod;
        mod = mod->next;
        tmp = Free (tmp);
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : AddSymbol
 *  arguments     : 1) name of symbol
 *                  2) module of symbol
 *                  3) type of symbol
 *  description   : adds implicitly imported symbol to global mod_tab if
 *                  it is yet unknown
 *  global vars   : mod_tab
 *
 */

void
AddSymbol (char *name, char *module, int symbkind)
{
    mod *tmp;
    int i;
    syms *sym;

    DBUG_ENTER ("AddSymbol");

    tmp = FindModul (module);
    if (tmp == NULL) {
        tmp = (mod *)Malloc (sizeof (mod));
        tmp->name = StringCopy (module);
        tmp->prefix = StringCopy (module);

        DBUG_PRINT ("MEMIMPORT",
                    ("Allocating mod at" F_PTR " name: %s(" F_PTR " prefix %s (" F_PTR,
                     tmp, tmp->name, tmp->name, tmp->prefix, tmp->prefix));

        tmp->flag = 0;
        tmp->allflag = 0;
        tmp->moddec = NULL;

        tmp->next = mod_tab;
        mod_tab = tmp;

        for (i = 0; i < 4; i++) {
            tmp->syms[i] = NULL;
        }

        tmp->syms[symbkind] = (syms *)Malloc (sizeof (syms));
        tmp->syms[symbkind]->id = StringCopy (name);
        tmp->syms[symbkind]->next = NULL;
        tmp->syms[symbkind]->flag = 0;
    } else {
        sym = tmp->syms[symbkind];
        while ((sym != NULL) && (strcmp (sym->id, name) != 0)) {
            sym = sym->next;
        }
        if (sym == NULL) {
            sym = (syms *)Malloc (sizeof (syms));
            sym->id = StringCopy (name);
            sym->flag = 0;
            sym->next = tmp->syms[symbkind];
            tmp->syms[symbkind] = sym;
        }
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : InsertClassType
 *  arguments     : 1) pointer to respective classdec-node
 *  description   : Inserts a new implicit type with uniqueness attribute
 *                  and the same name as the class itself to the class
 *                  declaration.
 *
 */

void
InsertClassType (node *classdec)
{
    node *explist, *tmp;

    DBUG_ENTER ("InsertClassType");

    tmp = MakeTypedef (StringCopy (CLASSDEC_NAME (classdec)),
                       CLASSDEC_ISEXTERNAL (classdec)
                         ? NULL
                         : StringCopy (CLASSDEC_NAME (classdec)),
                       MakeTypes1 (T_hidden), ST_unique, NULL);
    TYPEDEF_STATUS (tmp) = ST_imported_class;
    NODE_LINE (tmp) = 0;

    explist = classdec->node[0];

    if (explist->node[0] == NULL) { /* There are no other implicit types */
        explist->node[0] = tmp;
    } else {
        tmp->node[0] = explist->node[0];
        explist->node[0] = tmp;
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : CheckExistObjects
 *  arguments     : 1) global object from pragma touch or effect
 *                  2) N_modul node of current program
 *                  3) attrib of global object
 *                     (effect->ST_reference, touch->ST_readonly_reference)
 *                  4) line number of function for error message
 *  description   : checks if the global objects mentioned in touch or
 *                  effect pragmas of external modules/classes actually
 *                  exist in the current context.
 *                  A node list of needed objects for this particular
 *                  function is returned.
 *
 */

nodelist *
CheckExistObjects (ids *object, node *modul, statustype attrib, int line)
{
    node *find;
    nodelist *objlist = NULL;

    DBUG_ENTER ("CheckExistObjects");

    while (object != NULL) {
        find = SearchObjdef (IDS_NAME (object), IDS_MOD (object), MODUL_OBJS (modul));

        if (find == NULL) {
            if (attrib == ST_reference) {
                ERROR (line, ("Effected global object '%s` unknown",
                              ModName (IDS_MOD (object), IDS_NAME (object))));
            } else {
                ERROR (line, ("Touched global object '%s` unknown",
                              ModName (IDS_MOD (object), IDS_NAME (object))));
            }
        } else {
            objlist = MakeNodelist (find, ST_regular, objlist);
            NODELIST_ATTRIB (objlist) = attrib;
        }

        object = FreeOneIds (object);
    }

    DBUG_RETURN (objlist);
}

/*
 *
 *  functionname  : Nums2IntArray
 *  arguments     : 1) line number for error messages
 *                  2) number of parameters of function == array size
 *                  3) start of nums chain
 *  description   : traverses the nums chain and initializes the
 *                  returned array with these numbers.
 *
 *  remarks       : used for converting linksign pragmas
 *
 */

int *
Nums2IntArray (int line, int size, nums *numsp)
{
    int *ret, i;
    nums *tmp;

    DBUG_ENTER ("Nums2IntArray");

    ret = (int *)Malloc (size * sizeof (int));

    DBUG_PRINT ("PRAGMA", ("Converting nums to int[%d]", size));

    for (i = 0, tmp = numsp; (i < size) && (tmp != NULL); i++, tmp = NUMS_NEXT (tmp)) {
        DBUG_PRINT ("PRAGMA", ("Nums value is %d", NUMS_NUM (tmp)));

        if ((NUMS_NUM (tmp) < 0) || (NUMS_NUM (tmp) > size)) {
            ERROR (line, ("Invalid argument of pragma 'linksign`"));
            CONT_ERROR (
              ("Entry no.%d does not match a valid parameter position !", i + 1));
        }

        ret[i] = NUMS_NUM (tmp);
    }

    if (i < size) {
        ERROR (line, ("Invalid argument of pragma 'linksign`"));
        CONT_ERROR (("Less entries (%d) than parameters of function (%d) !", i, size));
    }

    if (tmp != NULL) {
        do {
            i++;

            DBUG_PRINT ("PRAGMA", ("Nums value is %d", NUMS_NUM (tmp)));

            tmp = NUMS_NEXT (tmp);
        } while (tmp != NULL);

        ERROR (line, ("Invalid argument of pragma 'linksign`:"));
        CONT_ERROR (("More entries (%d) than function parameters (%d) !", i, size));
    }

    FreeAllNums (numsp);

    DBUG_RETURN (ret);
}

/*
 *
 *  functionname  : Nums2BoolArray
 *  arguments     : 1) line number for error messages
 *                  2) number of parameters of function == array size
 *                  3) start of nums chain
 *  description   : allocates an int array of given size and initializes
 *                  all elements with 0. Afterwards, those positions which
 *                  are in the nums chain are set to 1.
 *
 */

int *
Nums2BoolArray (int line, int size, nums *numsp)
{
    int *ret, i;
    nums *tmp;

    DBUG_ENTER ("Nums2BoolArray");

    ret = (int *)Malloc (size * sizeof (int));

    DBUG_PRINT ("PRAGMA", ("Converting nums to bool[%d]", size));

    for (i = 0; i < size; i++) {
        ret[i] = 0;
    }

    tmp = numsp;
    i = 1;

    while (tmp != NULL) {
        DBUG_PRINT ("PRAGMA", ("Nums value is %d", NUMS_NUM (tmp)));

        if ((NUMS_NUM (tmp) < 0) || (NUMS_NUM (tmp) >= size)) {
            ERROR (line, ("Invalid argument of pragma 'readonly` or 'refcounting`:"));
            CONT_ERROR (
              ("Entry no.%d does not match a function parameter !", i, NUMS_NUM (tmp)));
        } else {
            ret[NUMS_NUM (tmp)] = 1;
        }

        tmp = NUMS_NEXT (tmp);
        i++;
    }

    FreeAllNums (numsp);

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * Function:
 *   node *InitGenericFuns(node *arg_node, node *pragma)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
InitGenericFuns (node *arg_node, node *pragma)
{
    DBUG_ENTER ("InitGenericFuns");

    DBUG_PRINT ("PRAGMA",
                ("Initializing generic functions of type %s", ItemName (arg_node)));

    if (TYPEDEF_ATTRIB (arg_node) != ST_unique) {
        if (pragma == NULL) {
            TYPEDEF_COPYFUN (arg_node)
              = (char *)Malloc (sizeof (char) * (strlen (TYPEDEF_NAME (arg_node)) + 7));
            strcpy (TYPEDEF_COPYFUN (arg_node), "copy_");
            strcat (TYPEDEF_COPYFUN (arg_node), TYPEDEF_NAME (arg_node));

            TYPEDEF_FREEFUN (arg_node)
              = (char *)Malloc (sizeof (char) * (strlen (TYPEDEF_NAME (arg_node)) + 7));
            strcpy (TYPEDEF_FREEFUN (arg_node), "free_");
            strcat (TYPEDEF_FREEFUN (arg_node), TYPEDEF_NAME (arg_node));
        } else {
            if (PRAGMA_COPYFUN (pragma) != NULL) {
                TYPEDEF_COPYFUN (arg_node) = PRAGMA_COPYFUN (pragma);
            } else {
                TYPEDEF_COPYFUN (arg_node) = (char *)Malloc (
                  sizeof (char) * (strlen (TYPEDEF_NAME (arg_node)) + 7));
                strcpy (TYPEDEF_COPYFUN (arg_node), "copy_");
                strcat (TYPEDEF_COPYFUN (arg_node), TYPEDEF_NAME (arg_node));
            }

            if (PRAGMA_FREEFUN (pragma) != NULL) {
                TYPEDEF_FREEFUN (arg_node) = PRAGMA_FREEFUN (pragma);
            } else {
                TYPEDEF_FREEFUN (arg_node) = (char *)Malloc (
                  sizeof (char) * (strlen (TYPEDEF_NAME (arg_node)) + 7));
                strcpy (TYPEDEF_FREEFUN (arg_node), "free_");
                strcat (TYPEDEF_FREEFUN (arg_node), TYPEDEF_NAME (arg_node));
            }

            TYPEDEF_PRAGMA (arg_node) = Free (TYPEDEF_PRAGMA (arg_node));
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *ResolvePragmaReadonly(node *arg_node, node *pragma, int count_params)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
ResolvePragmaReadonly (node *arg_node, node *pragma, int count_params)
{
    int cnt;
    node *tmp_args;
    types *tmp_types;

    DBUG_ENTER ("ResolvePragmaReadonly");

    DBUG_PRINT ("PRAGMA",
                ("Resolving pragma readonly for function %s", ItemName (arg_node)));

    PRAGMA_READONLY (pragma)
      = Nums2BoolArray (NODE_LINE (arg_node), count_params, PRAGMA_READONLYNUMS (pragma));

    cnt = 0;
    tmp_types = FUNDEF_TYPES (arg_node);

    while (tmp_types != NULL) {
        cnt++;
        tmp_types = TYPES_NEXT (tmp_types);
    }

    tmp_args = FUNDEF_ARGS (arg_node);

    while (tmp_args != NULL) {
        if (PRAGMA_READONLY (pragma)[cnt]) {
            if (ARG_ATTRIB (tmp_args) == ST_reference) {
                ARG_ATTRIB (tmp_args) = ST_readonly_reference;
            } else {
                WARN (NODE_LINE (arg_node),
                      ("Parameter no. %d of function '%s` is not a reference "
                       "parameter, so pragma 'readonly` has no effect on it",
                       cnt, ItemName (arg_node)));
            }
        }

        tmp_args = ARG_NEXT (tmp_args);
        cnt++;
    }

    PRAGMA_READONLY (pragma) = Free (PRAGMA_READONLY (pragma));

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IMtypedef
 *  arguments     : 1) N_typedef node
 *                  2) arg_info unused
 *  description   : checks the correctness of pragmas for a typedef.
 *                  The whole pragma node is removed afterwards.
 *                  The slots FREEFUN and COPYFUN are initialized.
 *
 */

node *
IMtypedef (node *arg_node, node *arg_info)
{
    node *pragma = TYPEDEF_PRAGMA (arg_node);

    DBUG_ENTER ("IMtypedef");

    DBUG_PRINT ("PRAGMA", ("Checking pragmas of type %s", ItemName (arg_node)));

    if (pragma != NULL) {
        if (TYPEDEF_MOD (arg_node) != NULL) {
            /*
             *  typedef from SAC-module/class
             */

            WARN (NODE_LINE (arg_node),
                  ("Pragmas have no effect on SAC-modules/classes"));
            TYPEDEF_PRAGMA (arg_node) = FreeNode (pragma);
        } else {
            /*
             *  typedef from  external module/class
             */

            if (PRAGMA_LINKNAME (pragma) != NULL) {
                WARN (NODE_LINE (arg_node), ("Pragma 'linkname` has no effect on type"));
                PRAGMA_LINKNAME (pragma) = Free (PRAGMA_LINKNAME (pragma));
            }

            if (PRAGMA_LINKSIGNNUMS (pragma) != NULL) {
                WARN (NODE_LINE (arg_node), ("Pragma 'linksign` has no effect on type"));
                PRAGMA_LINKSIGNNUMS (pragma) = FreeAllNums (PRAGMA_LINKSIGNNUMS (pragma));
            }

            if (PRAGMA_REFCOUNTINGNUMS (pragma) != NULL) {
                WARN (NODE_LINE (arg_node),
                      ("Pragma 'refcounting` has no effect on type"));
                PRAGMA_REFCOUNTINGNUMS (pragma)
                  = FreeAllNums (PRAGMA_REFCOUNTINGNUMS (pragma));
            }

            if (PRAGMA_READONLYNUMS (pragma) != NULL) {
                WARN (NODE_LINE (arg_node), ("Pragma 'readonly` has no effect on type"));
                PRAGMA_READONLYNUMS (pragma) = FreeAllNums (PRAGMA_READONLYNUMS (pragma));
            }

            if (PRAGMA_EFFECT (pragma) != NULL) {
                WARN (NODE_LINE (arg_node), ("Pragma 'effect` has no effect on type"));
                PRAGMA_EFFECT (pragma) = FreeAllIds (PRAGMA_EFFECT (pragma));
            }

            if (PRAGMA_TOUCH (pragma) != NULL) {
                WARN (NODE_LINE (arg_node), ("Pragma 'touch` has no effect on type"));
                PRAGMA_TOUCH (pragma) = FreeAllIds (PRAGMA_TOUCH (pragma));
            }

            if (PRAGMA_INITFUN (pragma) != NULL) {
                WARN (NODE_LINE (arg_node), ("Pragma 'initfun` has no effect on type"));
                PRAGMA_INITFUN (pragma) = Free (PRAGMA_INITFUN (pragma));
            }

            if (TYPEDEF_BASETYPE (arg_node) != T_hidden) {
                if (PRAGMA_COPYFUN (pragma) != NULL) {
                    WARN (NODE_LINE (arg_node),
                          ("Pragma 'copyfun` has no effect on explicit type"));
                    PRAGMA_COPYFUN (pragma) = Free (PRAGMA_COPYFUN (pragma));
                }

                if (PRAGMA_FREEFUN (pragma) != NULL) {
                    WARN (NODE_LINE (arg_node),
                          ("Pragma 'freefun` has no effect on explicit type"));
                    PRAGMA_FREEFUN (pragma) = Free (PRAGMA_FREEFUN (pragma));
                }
            }
        }
    }

    if ((TYPEDEF_BASETYPE (arg_node) == T_hidden) && (TYPEDEF_MOD (arg_node) == NULL)) {
        InitGenericFuns (arg_node, pragma);
    }

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        TYPEDEF_NEXT (arg_node) = Trav (TYPEDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IMfundef
 *  arguments     : 1) N_fundef node
 *                  2) arg_info points to N_modul node
 *  description   : checks pragmas of functions.
 *                  For SAC-modules/classes the whole pragma node is
 *                  removed. For external modules/classes, all those pragmas
 *                  are removed which do not belong to functions.
 *                  The pragmas linksign and refcounting are
 *                  converted from their first representation as nums chain
 *                  to an array representation.
 *                  The existence of all objects referenced in effect or
 *                  touch pragmas is checked and node lists of the defining
 *                  N_objdef nodes are generated.
 *                  The readonly pragma is resolved by traversing the
 *                  function's arguments and switching from attribute
 *                  ST_reference to ST_readonly_reference.
 *
 */

node *
IMfundef (node *arg_node, node *arg_info)
{
    int count_params = 0;
    node *pragma = FUNDEF_PRAGMA (arg_node);

    DBUG_ENTER ("IMfundef");

    DBUG_PRINT ("PRAGMA", ("Checking pragmas of function %s", ItemName (arg_node)));

    if (pragma != NULL) {
        if (FUNDEF_MOD (arg_node) != NULL) {
            /*
             *  fundef from SAC-module/class
             */

            WARN (NODE_LINE (arg_node),
                  ("Pragmas have no effect on SAC-modules/classes"));
            FUNDEF_PRAGMA (arg_node) = FreeNode (pragma);
        } else {
            /*
             *  fundef from  external module/class
             */

            count_params = CountFunctionParams (arg_node);

            if (PRAGMA_COPYFUN (pragma) != NULL) {
                WARN (NODE_LINE (arg_node),
                      ("Pragma 'copyfun` has no effect on function"));
                PRAGMA_COPYFUN (pragma) = Free (PRAGMA_COPYFUN (pragma));
            }

            if (PRAGMA_FREEFUN (pragma) != NULL) {
                WARN (NODE_LINE (arg_node),
                      ("Pragma 'freefun` has no effect on function"));
                PRAGMA_FREEFUN (pragma) = Free (PRAGMA_FREEFUN (pragma));
            }

            if (PRAGMA_INITFUN (pragma) != NULL) {
                WARN (NODE_LINE (arg_node),
                      ("Pragma 'initfun` has no effect on function"));
                PRAGMA_INITFUN (pragma) = Free (PRAGMA_INITFUN (pragma));
            }

            if (PRAGMA_LINKSIGNNUMS (pragma) != NULL) {
                PRAGMA_LINKSIGN (pragma)
                  = Nums2IntArray (NODE_LINE (arg_node), count_params,
                                   PRAGMA_LINKSIGNNUMS (pragma));
            }

            if (PRAGMA_REFCOUNTINGNUMS (pragma) != NULL) {
                PRAGMA_REFCOUNTING (pragma)
                  = Nums2BoolArray (NODE_LINE (arg_node), count_params,
                                    PRAGMA_REFCOUNTINGNUMS (pragma));
            }

            PRAGMA_NUMPARAMS (pragma) = count_params;

            if (PRAGMA_READONLYNUMS (pragma) != NULL) {
                arg_node = ResolvePragmaReadonly (arg_node, pragma, count_params);
            }

            if (PRAGMA_EFFECT (pragma) != NULL) {
                FUNDEF_NEEDOBJS (arg_node)
                  = CheckExistObjects (PRAGMA_EFFECT (pragma), arg_info, ST_reference,
                                       NODE_LINE (arg_node));
                PRAGMA_EFFECT (pragma) = NULL;
            }

            if (PRAGMA_TOUCH (pragma) != NULL) {
                FUNDEF_NEEDOBJS (arg_node)
                  = ConcatNodelist (FUNDEF_NEEDOBJS (arg_node),
                                    CheckExistObjects (PRAGMA_TOUCH (pragma), arg_info,
                                                       ST_readonly_reference,
                                                       NODE_LINE (arg_node)));
                PRAGMA_TOUCH (pragma) = NULL;
            }

            if ((PRAGMA_LINKNAME (pragma) == NULL) && (PRAGMA_LINKSIGN (pragma) == NULL)
                && (PRAGMA_REFCOUNTING (pragma) == NULL)) {
                FUNDEF_PRAGMA (arg_node) = FreeNode (pragma);
            }
        }
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IMobjdef
 *  arguments     : 1) N_objdef node
 *                  2) arg_info points to N_modul node
 *  description   : checks pragmas of global objects.
 *                  Afterwards, the whole pragma node is removed.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Trav, FreeAllNums, FreeAllIds, FreeTree, FreeNode
 *  macros        : DBUG, TREE, FREE, ERROR
 *
 *  remarks       :
 *
 */

node *
IMobjdef (node *arg_node, node *arg_info)
{
    node *pragma = OBJDEF_PRAGMA (arg_node);

    DBUG_ENTER ("IMobjdef");

    DBUG_PRINT ("PRAGMA", ("Checking pragmas of object %s", ItemName (arg_node)));

    if (pragma != NULL) {
        if (OBJDEF_MOD (arg_node) != NULL) {
            /*
             *  fundef from SAC-module/class
             */

            WARN (NODE_LINE (arg_node),
                  ("Pragmas have no effect on SAC-modules/classes"));
            OBJDEF_PRAGMA (arg_node) = FreeNode (pragma);
        } else {
            /*
             *  fundef from  external module/class
             */

            if (PRAGMA_LINKSIGNNUMS (pragma) != NULL) {
                WARN (NODE_LINE (arg_node),
                      ("Pragma 'linksign` has no effect on global object"));
                PRAGMA_LINKSIGNNUMS (pragma) = FreeAllNums (PRAGMA_LINKSIGNNUMS (pragma));
            }

            if (PRAGMA_REFCOUNTINGNUMS (pragma) != NULL) {
                WARN (NODE_LINE (arg_node),
                      ("Pragma 'refcounting` has no effect on global object"));
                PRAGMA_REFCOUNTINGNUMS (pragma)
                  = FreeAllNums (PRAGMA_REFCOUNTINGNUMS (pragma));
            }

            if (PRAGMA_READONLYNUMS (pragma) != NULL) {
                WARN (NODE_LINE (arg_node),
                      ("Pragma 'readonly` has no effect on global object"));
                PRAGMA_READONLYNUMS (pragma) = FreeAllNums (PRAGMA_READONLYNUMS (pragma));
            }

            if (PRAGMA_TOUCH (pragma) != NULL) {
                WARN (NODE_LINE (arg_node),
                      ("Pragma 'touch` has no effect on global object"));
                PRAGMA_TOUCH (pragma) = FreeAllIds (PRAGMA_TOUCH (pragma));
            }

            if (PRAGMA_EFFECT (pragma) != NULL) {
                OBJDEF_NEEDOBJS (arg_node)
                  = CheckExistObjects (PRAGMA_EFFECT (pragma), arg_info, ST_reference,
                                       NODE_LINE (arg_node));
                PRAGMA_EFFECT (pragma) = NULL;
            }

            if (PRAGMA_COPYFUN (pragma) != NULL) {
                WARN (NODE_LINE (arg_node),
                      ("Pragma 'copyfun` has no effect on global object"));
                PRAGMA_COPYFUN (pragma) = Free (PRAGMA_COPYFUN (pragma));
            }

            if (PRAGMA_FREEFUN (pragma) != NULL) {
                WARN (NODE_LINE (arg_node),
                      ("Pragma 'freefun` has no effect on global object"));
                PRAGMA_FREEFUN (pragma) = Free (PRAGMA_FREEFUN (pragma));
            }

            if (PRAGMA_INITFUN (pragma) != NULL) {
                DBUG_PRINT ("PRAGMA", ("object %s has initfun %s", ItemName (arg_node),
                                       PRAGMA_INITFUN (pragma)));
            }

            if (PRAGMA_LINKNAME (pragma) != NULL) {
                DBUG_PRINT ("PRAGMA", ("object %s has linkname %s", ItemName (arg_node),
                                       OBJDEF_LINKNAME (arg_node)));
            }
        }
    }

    if (OBJDEF_NEXT (arg_node) != NULL) {
        OBJDEF_NEXT (arg_node) = Trav (OBJDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

#if 0

/*
 *
 *  functionname  : CheckPragmas
 *  arguments     : 1) N_moddec node
 *                  2) prefix of respective module
 *  description   : checks pragmas of the given module
 *
 *  remarks       : 2) is used to distinguish between external and SAC
 *                     modules and classes.
 *
 */

node *CheckPragmas(node *moddec, char *prefix)
{
  DBUG_ENTER("CheckPragmas");
  
  if (MODDEC_OWN(moddec)!=NULL) {
    if (MODDEC_ITYPES(moddec)!=NULL) {
      MODDEC_ITYPES(moddec)
        =CheckPragmaTypedef(MODDEC_ITYPES(moddec), (node*)prefix);
    }
    
    if (MODDEC_ETYPES(moddec)!=NULL) {
      MODDEC_ETYPES(moddec)
        =CheckPragmaTypedef(MODDEC_ETYPES(moddec), (node*)prefix);
    }
    
    if (MODDEC_OBJS(moddec)!=NULL) {
      MODDEC_OBJS(moddec)
        =CheckPragmaObjdef(MODDEC_OBJS(moddec), (node*)prefix);
    }
    
    if (MODDEC_FUNS(moddec)!=NULL) {
      MODDEC_FUNS(moddec)
        =CheckPragmaFundef(MODDEC_FUNS(moddec), (node*)prefix);
    }
  }
  
  DBUG_RETURN(moddec);
}

#endif

/*
 *
 *  functionname  : GenMod
 *  arguments     : 1) name of modul to be read
 *                  2) flag whether the module is initially generated(0)
 *                     or for checking the declaration of a module/class
 *                     implementation(1).
 *  description   : Scans and parses the respective declaration
 *                  file and generates/initilizes a new mod-node
 *
 */

mod *
GenMod (char *name, int checkdec)
{
    int i;
    mod *tmp;
    static char buffer[MAX_FILE_NAME];
    char *pathname, *abspathname;
    char cccallstr[MAX_PATH_LEN];

    DBUG_ENTER ("GenMod");

    tmp = (mod *)Malloc (sizeof (mod));

    tmp->flag = 0;
    tmp->allflag = 0;

    strcpy (buffer, name);
    strcat (buffer, ".dec");

    if (checkdec) {
        NOTE (("Verifying module/class '%s` !", name));
    } else {
        NOTE (("Loading module/class '%s` !", name));
    }

    pathname = FindFile (MODDEC_PATH, buffer);

    if (pathname == NULL) {
        SYSABORT (("Unable to open file \"%s\"", buffer));
    }

    strcpy (cccallstr, config.cpp_file);

    strcat (cccallstr, " ");
    strcat (cccallstr, config.opt_D);
    strcat (cccallstr, "SAC_FOR_");
    strcat (cccallstr, target_platform);

    for (i = 0; i < num_cpp_vars; i++) {
        strcat (cccallstr, " ");
        strcat (cccallstr, config.opt_D);
        strcat (cccallstr, cppvars[i]);
    }

    strcat (cccallstr, " ");
    strcat (cccallstr, pathname);

    if (show_syscall)
        NOTE (("yyin = popen( %s)", cccallstr));

    yyin = popen (cccallstr, "r");

    if (yyin == NULL) {
        SYSABORT (("Unable to start C preprocessor", buffer));
    } else {
        abspathname = AbsolutePathname (pathname);

        NOTE (("  Parsing file \"%s\" ...", abspathname));

        filename = buffer;

        linenum = 1;
        start_token = PARSE_DEC;
        My_yyparse ();

        tmp->moddec = decl_tree;

        if (strcmp (decl_tree->info.fun_name.id, name) != 0) {
            SYSERROR (("File \"%s\" does not provide module/class '%s`, "
                       "but module/class '%s`",
                       buffer, name, decl_tree->info.fun_name.id));
        }

        if ((MODDEC_LINKWITH (decl_tree) != NULL) && (!MODDEC_ISEXTERNAL (decl_tree))) {
            WARN (NODE_LINE (decl_tree),
                  ("Pragma 'linkwith` has no effect on SAC module/class"));
            MODDEC_LINKWITH (decl_tree) = FreeAllDeps (MODDEC_LINKWITH (decl_tree));
        }

        /*
        Restriction no longer needed due to new library format using tar
        instead of ar.

        if (strlen(name)>13)
        {
          SYSERROR(("Module/class name '%s` too long (maximum: 13 characters)",
                    name));
        }
        */

        tmp->prefix = MODDEC_ISEXTERNAL (decl_tree) ? NULL : decl_tree->info.fun_name.id;
        tmp->name = decl_tree->info.fun_name.id;
        tmp->next = NULL;

        if (!checkdec) {
            if (MODDEC_LINKWITH (decl_tree) == NULL) {
                if ((EXPLIST_ITYPES (MODDEC_OWN (decl_tree)) != NULL)
                    || (EXPLIST_ETYPES (MODDEC_OWN (decl_tree)) != NULL)
                    || (EXPLIST_FUNS (MODDEC_OWN (decl_tree)) != NULL)
                    || (EXPLIST_OBJS (MODDEC_OWN (decl_tree)) != NULL)) {
                    dependencies
                      = MakeDeps (StringCopy (name), StringCopy (abspathname), NULL,
                                  MODDEC_ISEXTERNAL (decl_tree) ? ST_external : ST_sac,
                                  FindLocationOfFile (abspathname), NULL, dependencies);
                }
            } else {
                dependencies
                  = MakeDeps (StringCopy (name), StringCopy (abspathname), NULL,
                              ST_external, FindLocationOfFile (abspathname),
                              MODDEC_LINKWITH (decl_tree), dependencies);
                MODDEC_LINKWITH (decl_tree) = NULL;
            }

            /*
             * If checkdec is set, GenMod() is called from checkdec.c in order
             * to compare defined and declared items. Here, declarations are read
             * which are imported by the module's own declaration. These are
             * not necessarily required for linking.
             *
             * All imported modules/classes are put to the dependencies list
             * which is stored in the global variable dependencies.
             *
             * For external modules and classes the contents of the optional pragma
             * linkwith is added to the list of dependencies as a sub tree of the
             * respective module or class.
             *
             * The list of dependencies is used in readsib for searching for
             * required libraries, updated and then again used in cccall for
             * generating a link list.
             */
        }

        for (i = 0; i < 4; i++) {
            tmp->syms[i] = NULL;
        }

        if (tmp->moddec->nodetype == N_classdec) {
            InsertClassType (tmp->moddec);
        }
    }

    DBUG_RETURN (tmp);
}

/*
 *
 *  functionname  : FindOrAppend
 *  arguments     : 1) importlist
 *                  2) flag whether the module is initially generated(0)
 *                     or for checking the declaration of a module/class
 *                     implementation(1).
 *  description   : searches for all modules from the given importlist
 *                  in the global mod_tab. For all modules, which can
 *                  not be found, a new mod_tab entry is generated
 *                  and appended to mod_tab.
 *  global vars   : mod_tab
 *
 */

void
FindOrAppend (node *implist, int checkdec)
{
    mod *current, *last;
    int tmp;

    DBUG_ENTER ("FindOrAppend");

    DBUG_ASSERT ((implist), "FindOrAppend called with NULL-import-list!");

    if (mod_tab == NULL) { /* the first modul has to be inserted anyway! */
        mod_tab = GenMod (implist->info.id, checkdec);
        implist = implist->node[0];
    }

    /* mod_tab contains at least one entry! */
    DBUG_ASSERT ((mod_tab), "Empty mod_tab!");

    while (implist != NULL) {
        current = mod_tab;
        do {
            tmp = strcmp (current->name, implist->info.id);
            last = current;
            current = current->next;
        } while ((current != NULL) && (tmp != 0));

        if (tmp != 0) {
            current = GenMod (implist->info.id, checkdec);
            last->next = current;
        }
        implist = implist->node[0];
    }

    filename = sacfilename;

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : GenSyms
 *  arguments     : 1) mod-node
 *  description   : generates the symbol tables for the given mod-node.
 *
 */

void
GenSyms (mod *mod)
{
    node *explist, *ptr;
    syms *new;
    char *pname;
    int i;

    DBUG_ENTER ("GenSyms");

    explist = mod->moddec->node[0];

    if (explist != NULL) {
        for (i = 0; i < 4; i++) {
            ptr = EXPLIST_TOF (explist, i);

            while (ptr != NULL) {
                pname = TOFDEF_NAME (ptr);

                DBUG_PRINT ("IMPORT", ("inserting symbol %s of kind %d", pname, i));

                new = (syms *)Malloc (sizeof (syms));
                new->id = (char *)Malloc (strlen (pname) + 1);
                strcpy (new->id, pname);
                new->flag = NOT_IMPORTED;
                new->next = mod->syms[i];
                mod->syms[i] = new;

                ptr = TOFDEF_NEXT (ptr); /* next declaration! */
            }
        }
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : FindSymbolInModul
 *  arguments     : 1) name of modul
 *                  2) name of the symbol
 *                  3) symbol-kind: 0 : implicit type
 *                                  1 : explicit type
 *                                  2 : fun-declaration
 *                                  3 : global object
 *                  4) mods found previously; initially a NULL is given here.
 *                  5) search strategy: 0 : search in the given modul only
 *                                      1 : search in imported modules as well
 *  description   : searches for a symbol of given kind in a prespecified
 *                  modul; returns a pointer linked list of modul-entries
 *                  for all those modules which own a definition of the
 *                  given symbol and are imported into the given modul.
 *
 */

mods *
FindSymbolInModul (char *modname, char *name, int symbkind, mods *found, int recursive)
{
    node *imports;
    mods *new;
    mod *tmpmod;
    syms *syms;
    ids *tmpids;
    static int cnt = 1;

    DBUG_ENTER ("FindSymbolInModul");

    DBUG_PRINT ("IMPORT",
                ("searching for symbol %s in module/class %s...", name, modname));

    cnt++;
    tmpmod = FindModul (modname);

    if (tmpmod != NULL) {
        if (tmpmod->flag != cnt) {
            tmpmod->flag = cnt;

            /* Find syms entry name */
            syms = tmpmod->syms[symbkind];
            while ((syms != NULL) && (strcmp (syms->id, name) != 0))
                syms = syms->next;

            if (syms != NULL) { /* name is declared in this modul; */
                                /* insert it in result!            */
                DBUG_PRINT ("IMPORT",
                            ("symbol %s found in module/class %s...", name, modname));

                new = (mods *)Malloc (sizeof (mods));
                new->mod = tmpmod;
                new->syms = syms;
                new->next = found;
                found = new;
            }

            if ((recursive == 1) && (tmpmod->moddec != NULL)) {
                /*
                 * Now, we recursively investigate all recursive imports!
                 *
                 * However, we do so only if the module has explicitly been
                 * imported rather than implicitly via a SIB.
                 */

                imports = tmpmod->moddec->node[1]; /* pointer to imports ! */

                while (imports != NULL) {
                    if ((imports->node[1] == NULL) && (imports->node[2] == NULL)
                        && (imports->node[3] == NULL) && (imports->node[4] == NULL)) {
                        /* import all ! */
                        cnt--;
                        found = FindSymbolInModul (imports->info.id, name, symbkind,
                                                   found, 1);
                    } else {
                        /* selective import! */
                        /* Find name in ids */

                        tmpids = (ids *)imports->node[symbkind + 1];
                        while ((tmpids != NULL) && (strcmp (tmpids->id, name) != 0))
                            tmpids = tmpids->next;

                        if (tmpids != NULL) { /* Symbol found! */
                            cnt--;
                            found = FindSymbolInModul (imports->info.id, name, symbkind,
                                                       found, 1);
                        }
                    }
                    imports = imports->node[0];
                }
            }
        }
    }

    DBUG_RETURN (found);
}

/*
 *
 *  functionname  : AppendModnameToSymbol
 *  arguments     : 1) N_typedef, N_objdef or N_fundef
 *                     node of symbol
 *                  2) name of the modul which owns the symbol
 *  description   : runs for all user defined types FindSymbolInModul
 *                  and inserts the respective modul-name in TYPES_MOD( types).
 *
 */

void
AppendModnameToSymbol (node *symbol, char *modname)
{
    node *arg;
    mods *mods, *mods2;
    types *types;
    int done;
    char decfilename[MAX_FILE_NAME];

    DBUG_ENTER ("AppendModnameToSymbol");

    strcpy (decfilename, modname);
    strcat (decfilename, ".dec");
    filename = decfilename;
    /* only for error messages */

    if (NODE_TYPE (symbol) == N_fundef) {
        arg = FUNDEF_ARGS (symbol);
    } else {
        arg = NULL;
    }

    types = TOFDEF_TYPES (symbol);
    while (types != NULL) {
        if (TYPES_BASETYPE (types) == T_user) {
            done = 0;
            if (TYPES_MOD (types) != NULL) {
                modname = TYPES_MOD (types);
                mods = FindSymbolInModul (modname, TYPES_NAME (types), 0, NULL, 0);
                mods2 = FindSymbolInModul (modname, TYPES_NAME (types), 1, NULL, 0);

                if ((mods == NULL) && (mods2 != NULL)) {
                    TYPES_MOD (types) = mods2->mod->prefix;
                    done = 1;
                }

                if ((mods != NULL) && (mods2 == NULL)) {
                    TYPES_MOD (types) = mods->mod->prefix;
                    done = 1;
                }

                FreeMods (mods);
                FreeMods (mods2);
            };

            if (done != 1) {
                mods = FindSymbolInModul (modname, TYPES_NAME (types), 0, NULL, 1);
                mods2 = FindSymbolInModul (modname, TYPES_NAME (types), 1, NULL, 1);

                if (mods != NULL) {
                    if (mods2 != NULL) {
                        if ((strcmp (mods->mod->name, TYPES_NAME (types)) == 0)
                            && (mods->mod->moddec->nodetype == N_classdec)) {
                            ERROR (symbol->lineno, ("Explicit type '%s:%s` "
                                                    "conflicts with class '%s`"
                                                    "in module/class '%s`",
                                                    mods2->mod->name, TYPES_NAME (types),
                                                    TYPES_NAME (types), modname));
                        } else {
                            ERROR (symbol->lineno,
                                   ("Implicit type '%s:%s` "
                                    "and explicit type '%s:%s` available "
                                    "in module/class '%s`",
                                    mods->mod->name, TYPES_NAME (types), mods2->mod->name,
                                    TYPES_NAME (types), modname));
                        }
                    } else { /* mods2 == NULL */
                        if (mods->next != NULL) {
                            if ((strcmp (mods->mod->name, TYPES_NAME (types)) == 0)
                                && (mods->mod->moddec->nodetype == N_classdec)) {
                                ERROR (symbol->lineno,
                                       ("Implicit type '%s:%s` "
                                        "conflicts with class '%s` "
                                        "in module/class '%s`",
                                        mods->next->mod->name, TYPES_NAME (types),
                                        TYPES_NAME (types), modname));
                            } else {
                                if ((strcmp (mods->next->mod->name, TYPES_NAME (types))
                                     == 0)
                                    && (mods->next->mod->moddec->nodetype
                                        == N_classdec)) {
                                    ERROR (symbol->lineno,
                                           ("Implicit type '%s:%s` "
                                            "conflicts with class '%s` "
                                            "in module/class '%s`",
                                            mods->mod->name, TYPES_NAME (types),
                                            TYPES_NAME (types), modname));
                                } else {
                                    ERROR (
                                      symbol->lineno,
                                      ("Implicit types '%s:%s` and '%s:%s` available ",
                                       "in module/class '%s`", mods->mod->name,
                                       TYPES_NAME (types), mods->next->mod->name,
                                       TYPES_NAME (types), modname));
                                }
                            }
                        } else { /* mods->next == NULL */
                            TYPES_MOD (types) = mods->mod->prefix;
                        }
                    }
                } else { /* mods == NULL */
                    if (mods2 == NULL) {
                        ERROR (symbol->lineno, ("No type '%s` available "
                                                "in module/class '%s`",
                                                TYPES_NAME (types), modname));
                    } else { /* mods2 != NULL */
                        if (mods2->next != NULL) {
                            ERROR (symbol->lineno,
                                   ("Explicit types '%s:%s` and '%s:%s` available ",
                                    "in module/class '%s`", mods2->mod->name,
                                    TYPES_NAME (types), mods2->next->mod->name,
                                    TYPES_NAME (types), modname));
                        } else {
                            TYPES_MOD (types) = mods2->mod->prefix;
                        }
                    }
                }

                FreeMods (mods);
                FreeMods (mods2);
            }
        }
        types = TYPES_NEXT (types);

        if ((types == NULL) && (arg != NULL)) {
            types = ARG_TYPE (arg);
            arg = ARG_NEXT (arg);
        }
    }

    filename = puresacfilename;

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : ImportAll
 *  arguments     : 1) modtab of modul to be inserted
 *                  2) N_modul node where the imports have to be inserted
 *  description   : inserts all symbols from a given mod * into a given modul.
 *
 */

void
ImportAll (mod *mod, node *modul)
{
    syms *symptr;
    node *explist, *tmpnode;
    int i;

    DBUG_ENTER ("ImportAll");

    DBUG_PRINT ("IMPORT", ("importing all from module/class %s", mod->name));

    if (mod->allflag != 1) {
        explist = mod->moddec->node[0];

        tmpnode = EXPLIST_ITYPES (explist);

        while (tmpnode != NULL) {
            AppendModnameToSymbol (tmpnode, mod->name);
            tmpnode = TYPEDEF_NEXT (tmpnode);
        }

        MODUL_TYPES (modul)
          = AppendTypedef (EXPLIST_ITYPES (explist), MODUL_TYPES (modul));

        tmpnode = EXPLIST_ETYPES (explist);

        while (tmpnode != NULL) {
            AppendModnameToSymbol (tmpnode, mod->name);
            tmpnode = TYPEDEF_NEXT (tmpnode);
        }

        MODUL_TYPES (modul)
          = AppendTypedef (EXPLIST_ETYPES (explist), MODUL_TYPES (modul));

        tmpnode = EXPLIST_OBJS (explist);

        while (tmpnode != NULL) {
            AppendModnameToSymbol (tmpnode, mod->name);
            tmpnode = OBJDEF_NEXT (tmpnode);
        }

        MODUL_OBJS (modul) = AppendObjdef (EXPLIST_OBJS (explist), MODUL_OBJS (modul));

        tmpnode = EXPLIST_FUNS (explist);

        while (tmpnode != NULL) {
            AppendModnameToSymbol (tmpnode, mod->name);
            tmpnode = FUNDEF_NEXT (tmpnode);
        }

        MODUL_FUNS (modul) = AppendFundef (EXPLIST_FUNS (explist), MODUL_FUNS (modul));

        /* We mark all syms entries as imported! */
        for (i = 0; i < 4; i++) {
            symptr = mod->syms[i];
            while (symptr != NULL) {
                symptr->flag = IMPORTED;
                symptr = symptr->next;
            }
        }

        mod->allflag = 1; /* mark mod as beeing imported completely */

        /* Last, but not least,
           we recursively import the imported modules imports! */

        if (mod->moddec->node[1] != NULL) {
            DoImport (modul, mod->moddec->node[1], mod->name);
        }
    } else {
        DBUG_PRINT ("IMPORT", ("import of modul/class %s skipped", mod->name));
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : ImportSymbol
 *  arguments     : 1) symbtype indicates the kind of symbol to be imported
 *                     0 - implicit type
 *                     1 - explicit type
 *                     2 - function
 *                     3 - global object
 *                  2) name of the symbol to be imported
 *                  3) pointer to the mod_tab entry where the import is from
 *                  4) pointer to the modul node where the import is
 *                     to be inserted
 *  description   : moves a particular symbol from the parsed modul declaration
 *                  into the syntax tree for the whole program.
 *                  In the case of functions, all functions with the given name
 *                  are moved in the presence of function overloading.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */

void
ImportSymbol (int symbtype, char *name, mod *mod, node *modul)
{
    node *explist;
    node *tmpdef, *last;

    DBUG_ENTER ("ImportSymbol");
    DBUG_PRINT ("IMPORT", ("importing symbol %s of kind %d (0=imp/1=exp/2=fun/3=obj)"
                           "from modul %s",
                           name, symbtype, mod->name));

    explist = mod->moddec->node[0];
    tmpdef = EXPLIST_TOF (explist, symbtype);

    while ((tmpdef != NULL) && (strcmp (TOFDEF_NAME (tmpdef), name) == 0)) {
        /* The first entry has to be moved ! */

        EXPLIST_TOF_L (explist, symbtype, TOFDEF_NEXT (tmpdef));
        /* eliminating tmpdef from the chain */
        /* tmpdef points on the def which is to be inserted */

        AppendModnameToSymbol (tmpdef, mod->name);

        TOFDEF_NEXT_L (tmpdef, MODUL_TOF (modul, symbtype));
        MODUL_TOF_L (modul, symbtype, tmpdef);

        tmpdef = EXPLIST_TOF (explist, symbtype);
    }

    if (tmpdef != NULL) {
        last = tmpdef;
        tmpdef = TOFDEF_NEXT (tmpdef);

        while (tmpdef != NULL) {
            if (strcmp (TOFDEF_NAME (tmpdef), name) == 0) {
                TOFDEF_NEXT_L (last, TYPEDEF_NEXT (tmpdef));

                AppendModnameToSymbol (tmpdef, mod->name);

                TOFDEF_NEXT_L (tmpdef, MODUL_TYPES (modul));
                MODUL_TYPES (modul) = tmpdef;

                tmpdef = TOFDEF_NEXT (last);
            } else {
                last = tmpdef;
                tmpdef = TOFDEF_NEXT (tmpdef);
            }
        }
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : DoImport
 *  arguments     : 1) N_modul where the imports have to be done
 *                  2) implist, containing the imports to be resolved
 *                  3) name of the modul, that initially tried to
 *                     import a specific symbol
 *  description   : imports all symbols from the given implist into the
 *                  given modul. If a specified symbol can not be found
 *                  an Error message is made.
 *  global vars   : ---
 *  internal funs : FindModul, FindSymbolInModul, ImportAll, ImportSymbol
 *  external funs : MakeIds
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */

void
DoImport (node *modul, node *implist, char *filename)
{
    int i;
    mods *mods, *tmpmods;
    mod *mod;
    ids *tmp;

    DBUG_ENTER ("DoImport");

    while (implist != NULL) {
        mod = FindModul (implist->info.id);

        if ((implist->node[1] == NULL) && (implist->node[2] == NULL)
            && (implist->node[3] == NULL) && (implist->node[4] == NULL)) {
            /* this is an import all!! */
            ImportAll (mod, modul);
        } else { /* selective import! */
            for (i = 0; i < 4; i++) {
                tmp = (ids *)implist->node[i + 1];

                while (tmp != NULL) { /* importing some symbol! */
                    mods = FindSymbolInModul (implist->info.id, tmp->id, i, NULL, 1);

                    if (mods == NULL) {
                        switch (i) {
                        case 0:
                            ERROR (implist->lineno,
                                   ("No implicit type '%s` in module/class '%s`", tmp->id,
                                    implist->info.id));
                            break;

                        case 1:
                            ERROR (implist->lineno,
                                   ("No explicit type '%s` in module/class '%s`", tmp->id,
                                    implist->info.id));
                            break;

                        case 2:
                            ERROR (implist->lineno,
                                   ("No function '%s` in module/class '%s`", tmp->id,
                                    implist->info.id));
                            break;

                        case 3:
                            ERROR (implist->lineno,
                                   ("No global object '%s` in module/class '%s`", tmp->id,
                                    implist->info.id));
                            break;
                        }
                    } else {
                        do {
                            tmpmods = mods;
                            if (mods->syms->flag == NOT_IMPORTED) {
                                mods->syms->flag = IMPORTED; /* mark symbol as imported */
                                ImportSymbol (i, tmp->id, mods->mod, modul);
                            }
                            mods = mods->next;
                            tmpmods = Free (tmpmods);
                        } while (mods != NULL);
                    }
                    tmp = tmp->next;
                }
            }
        }
        implist = implist->node[0];
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : AddClasstypeOnSelectiveImport
 *  arguments     : 1) pointer to mod_tab entry
 *  description   : The entire import list of the given module/class
 *                  declaration is traversed and wherever a selective
 *                  import from another class occurs, the classtype
 *                  is added to the list of imported implicit types.
 *                  So, a programmer may or may not include the class type
 *                  when importing selectively from a class.
 *
 */

void
AddClasstypeOnSelectiveImport (mod *modptr)
{
    mod *moddec;
    ids *tmp;
    node *implist;

    DBUG_ENTER ("AddClasstypeOnSelectiveImport");

    implist = MODDEC_IMPORTS (modptr->moddec);

    while (implist != NULL) {
        moddec = FindModul (IMPLIST_NAME (implist));

        if ((NODE_TYPE (moddec->moddec) == N_classdec)
            && ((IMPLIST_ITYPES (implist) != NULL) || (IMPLIST_ETYPES (implist) != NULL)
                || (IMPLIST_FUNS (implist) != NULL)
                || (IMPLIST_OBJS (implist) != NULL))) {
            /*
             *  If the imported module/class actually is a class then the
             *  respective class type is added to the import list in the
             *  case of a selective import.
             */

            tmp = MakeIds (IMPLIST_NAME (implist), NULL, ST_regular);
            tmp->next = IMPLIST_ITYPES (implist);
            IMPLIST_ITYPES (implist) = tmp;
        }

        implist = IMPLIST_NEXT (implist);
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : IMmodul
 *  arguments     : 1) pointer to N_modul node
 *                  2) NULL-pointer
 *  description   : Main function for import of classes/objects,
 *                  which imports all items needed. Further traversals
 *                  of typedefs and fundefs necessary to retrieve
 *                  information from SIBs.
 *  global vars   : mod_tab
 *
 */

node *
IMmodul (node *arg_node, node *arg_info)
{
    mod *modptr;

    DBUG_ENTER ("IMmodul");

    if (arg_node->node[0] != NULL) { /* there are any imports! */
        FindOrAppend (arg_node->node[0], 0);
        modptr = mod_tab;

        while (modptr != NULL) {
            DBUG_PRINT ("IMPORT", ("analyzing module/class %s", modptr->moddec->info.id));
            if (modptr->moddec->node[1] != NULL) {
                FindOrAppend (modptr->moddec->node[1], 0);
            }
            GenSyms (modptr);
            AddClasstypeOnSelectiveImport (modptr);
            modptr = modptr->next;
        }

        ABORT_ON_ERROR;

        DoImport (arg_node, arg_node->node[0], arg_node->info.id);

        /*
         *  The imports are reused in checkdec.c for writing automatic
         *  declaration files, but they will not be printed any more.
         */

        MODUL_STORE_IMPORTS (arg_node) = MODUL_IMPORTS (arg_node);
        MODUL_IMPORTS (arg_node) = NULL;

        /*
         *  In the following, types, functions, and objects are traversed
         *  in order to check and resolve pragmas.
         */

        if (MODUL_TYPES (arg_node) != NULL) {
            MODUL_TYPES (arg_node) = Trav (MODUL_TYPES (arg_node), arg_info);
        }

        if (MODUL_OBJS (arg_node) != NULL) {
            MODUL_OBJS (arg_node) = Trav (MODUL_OBJS (arg_node), arg_node);
        }

        if (MODUL_FUNS (arg_node) != NULL) {
            MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_node);
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : ImportOwnDeclaration
 *  arguments     : 1) name of module/class
 *                  2) file type, module or class ?
 *  description   : scans and parses the respective
 *                  declaration when compiling a module or class
 *                  implementation
 *  global vars   : decl_tree, linenum, start_token, yyin
 *
 */

node *
ImportOwnDeclaration (char *name, file_type modtype)
{
    mod *modptr, *old_mod_tab;
    int i;
    char buffer[MAX_FILE_NAME];
    node *decl = NULL, *symbol;
    char *pathname, *abspathname, *old_filename;
    char cccallstr[MAX_PATH_LEN];

    DBUG_ENTER ("ImportOwnDeclaration");

    old_mod_tab = mod_tab;

    mod_tab = (mod *)Malloc (sizeof (mod));
    mod_tab->name = name;
    mod_tab->flag = 0;
    mod_tab->allflag = 0;

    strcpy (buffer, name);
    strcat (buffer, ".dec");

    pathname = FindFile (MODDEC_PATH, buffer);

    if (pathname == NULL) {
        mod_tab = Free (mod_tab);
    } else {
        strcpy (cccallstr, config.cpp_file);

        strcat (cccallstr, " ");
        strcat (cccallstr, config.opt_D);
        strcat (cccallstr, "SAC_FOR_");
        strcat (cccallstr, target_platform);

        for (i = 0; i < num_cpp_vars; i++) {
            strcat (cccallstr, " ");
            strcat (cccallstr, config.opt_D);
            strcat (cccallstr, cppvars[i]);
        }

        strcat (cccallstr, " ");
        strcat (cccallstr, pathname);

        if (show_syscall)
            NOTE (("yyin = popen( %s)", cccallstr));

        yyin = popen (cccallstr, "r");

        if (yyin == NULL) {
            SYSABORT (("Unable to start C preprocessor"));
        }

        abspathname = AbsolutePathname (pathname);

        NOTE (("Loading own declaration !"));
        NOTE (("  Parsing file \"%s\" ...", abspathname));

        linenum = 1;
        old_filename = filename; /* required for restauration */
        filename = buffer;
        start_token = PARSE_DEC;
        My_yyparse ();
        fclose (yyin);

        if ((strcmp (MODDEC_NAME (decl_tree), name) != 0)
            || ((NODE_TYPE (decl_tree) == N_classdec) && (modtype == F_modimp))
            || ((NODE_TYPE (decl_tree) == N_moddec) && (modtype == F_classimp))) {
            SYSERROR (("File \"%s\" provides wrong declaration", filename));

            if (modtype == F_modimp) {
                CONT_ERROR (("  Required: ModuleDec %s", name));
            } else {
                CONT_ERROR (("  Required: ClassDec %s", name));
            }

            if (NODE_TYPE (decl_tree) == N_moddec) {
                CONT_ERROR (("  Provided: ModuleDec %s", MODDEC_NAME (decl_tree)));
            } else {
                CONT_ERROR (("  Provided: ClassDec %s", MODDEC_NAME (decl_tree)));
            }

            ABORT_ON_ERROR;
        }

        decl = decl_tree;

        mod_tab->moddec = decl_tree;
        mod_tab->prefix
          = MODDEC_ISEXTERNAL (decl_tree) ? NULL : decl_tree->info.fun_name.id;
        mod_tab->next = NULL;

        for (i = 0; i < 4; i++) {
            mod_tab->syms[i] = NULL;
        }

        if (mod_tab->moddec->nodetype == N_classdec) {
            InsertClassType (mod_tab->moddec);
        }

        modptr = mod_tab;

        while (modptr != NULL) {
            if (modptr->moddec->node[1] != NULL) {
                FindOrAppend (modptr->moddec->node[1], 1);
            }
            GenSyms (modptr);
            AddClasstypeOnSelectiveImport (modptr);

            modptr = modptr->next;
        }

        for (i = 1; i < 4; i++) {
            symbol = mod_tab->moddec->node[0]->node[i];
            while (symbol != NULL) {
                AppendModnameToSymbol (symbol, name);
                symbol = NODE_NEXT (symbol);
            }
        }

        filename = old_filename;
    }

    mod_tab = old_mod_tab;

    DBUG_RETURN (decl);
}

/*
 *
 *  functionname  : PrintDependencies
 *  arguments     : 1) list of dependencies
 *                  2) mode, corresponds to compiler option -M (1),
 *                     -Mlib (2), -MM (3), or -MMlib (4)
 *  description   : prints a list of dependencies in a Makefile-like style
 *                  to stdout. All declaration files of imported modules
 *                  and classes are printed including the own declaration
 *                  when compiling a module/class implementation.
 *
 *  remarks       : This function corresponds to the -M, -Mlib, -MM, and -MMlib
 *                  compiler options.
 *
 */

void
PrintDependencies (deps *depends, int mode)
{
    deps *tmp;
    char buffer[MAX_FILE_NAME];

    DBUG_ENTER ("PrintDependencies");

    printf ("%s: ", outfilename);

    tmp = depends;

    while (tmp != NULL) {
        if ((mode <= 2) || (DEPS_LOC (tmp) == LOC_usr)) {
            printf ("  \\\n  %s", DEPS_DECNAME (tmp));
        }
        tmp = DEPS_NEXT (tmp);
    }

    if ((mode == 1) || (mode == 3)) {
        printf ("\n");
    } else {
        tmp = depends;

        while (tmp != NULL) {
            if ((mode == 2) || (DEPS_LOC (tmp) == LOC_usr)) {
                strcpy (buffer, DEPS_NAME (tmp));

                if (DEPS_STATUS (tmp) == ST_sac) {
                    strcat (buffer, ".lib");
                } else {
                    strcat (buffer, ".a");
                }

                printf ("  \\\n  %s", buffer);
            }
            tmp = DEPS_NEXT (tmp);
        }

        strcpy (buffer, sacfilename);
        strcpy (buffer + strlen (sacfilename) - 4, ".d");

        printf ("\n\n%s:", buffer);

        tmp = depends;

        while (tmp != NULL) {
            if ((mode == 2) || (DEPS_LOC (tmp) == LOC_usr)) {
                printf ("  \\\n  %s", DEPS_DECNAME (tmp));
            }
            tmp = DEPS_NEXT (tmp);
        }

        printf ("\n\nalldeps:");

        tmp = depends;

        while (tmp != NULL) {
            if ((mode == 2) || (DEPS_LOC (tmp) == LOC_usr)) {
                printf ("  \\\n  %s", DEPS_NAME (tmp));
            }
            tmp = DEPS_NEXT (tmp);
        }

        printf ("\n\n.PHONY:");

        tmp = depends;

        while (tmp != NULL) {
            if ((mode == 2) || (DEPS_LOC (tmp) == LOC_usr)) {
                printf ("  \\\n  %s", DEPS_NAME (tmp));
            }
            tmp = DEPS_NEXT (tmp);
        }

        printf ("\n\n");

        tmp = depends;

        while (tmp != NULL) {
            if ((mode == 2) || (DEPS_LOC (tmp) == LOC_usr)) {
                printf ("%s:\n\t", DEPS_NAME (tmp));

                strcpy (buffer, DEPS_DECNAME (tmp));
                filename = strrchr (buffer, '/');
                *filename = 0;

                printf ("(cd %s; $(MAKE) %s)\n\n", buffer, DEPS_NAME (tmp));
            }

            tmp = DEPS_NEXT (tmp);
        }
    }

    DBUG_VOID_RETURN;
}
