/*
 *
 * $Log$
 * Revision 1.53  1998/05/27 11:19:44  cg
 * global variable 'filename' which contains the current file name in order
 * to provide better error messages is now handled correctly.
 *
 * Revision 1.52  1998/04/03 20:45:08  dkr
 * removed a wrong cast in AddClasstypeOnSelectiveImport
 *   (type of IMPLIST_ITYPES is not *node but *ids)
 *
 * Revision 1.51  1998/02/27 16:32:58  cg
 * added correct setting of file names for diagnostic output
 * while parsing (global variable 'filename')
 *
 * Revision 1.50  1997/11/07 14:41:54  dkr
 * eliminated another nnode
 *
 * Revision 1.49  1997/11/07 12:28:12  srs
 * NEWTREE: nnode is ignored
 *
 * Revision 1.48  1997/10/29 14:30:44  srs
 * free -> FREE
 *
 * Revision 1.47  1997/08/04 17:13:17  dkr
 * removed strcpy/strlen-bug in GenSyms
 *
 * Revision 1.46  1997/04/30  11:53:36  cg
 * Bug fixed in InsertClassType()
 *
 * Revision 1.45  1997/04/24  10:02:15  cg
 * improved PrintDependencies for -Mlib option
 * bug fixed concerning class types upon selective import when checking
 * a module's own declaration file
 *
 * Revision 1.44  1997/03/19  13:48:59  cg
 * Now, all imported modules are stored in the global dependency tree for
 * later use (checking libraries, link list)
 *
 * Revision 1.43  1997/03/11  16:27:10  cg
 * new function PrintDependencies corresponding to compiler option -M
 * old compiler option -deps (updating makefile) no longer supported
 * absolute pathnams used for all dependencies
 *
 * Revision 1.42  1996/09/11  06:25:15  cg
 * Imported modules are stored in special structure to create libstat
 * information later.
 *
 * Revision 1.41  1996/03/05  10:02:06  cg
 * implemented a better consistency check for pragmas
 * linksign, refcounting, and readonly
 *
 * Revision 1.40  1996/01/23  09:58:05  cg
 * Now, the name length of imported modules is checked
 *
 * Revision 1.39  1996/01/23  09:01:39  cg
 * bug fixed in function ImportOwnDeclaration
 *
 * Revision 1.38  1996/01/22  18:34:18  cg
 * added new pragmas for global objects: effect, initfun
 *
 * Revision 1.37  1996/01/07  16:59:29  cg
 * pragmas copyfun, freefun, linkname, effect, touch and readonly
 * are now immediately resolved
 *
 * Revision 1.36  1996/01/02  16:03:09  cg
 * handling of global variable filename simplified
 *
 * Revision 1.35  1995/12/29  10:39:37  cg
 * All functions concerning SIBs extracted and moved to readsib.c
 *
 * Revision 1.34  1995/12/21  15:05:28  cg
 * pragmas will be imported and checked for plausibility.
 *
 * Revision 1.33  1995/12/18  18:26:19  cg
 * Now, we look in MODIMP-Path for SIBs instead of MODDEC-Path.
 *
 * Revision 1.32  1995/12/06  09:49:36  cg
 * Name of class and its respective type are no longer shared.
 *
 * Revision 1.31  1995/11/10  15:04:53  cg
 * converted to new error macros
 *
 * Revision 1.30  1995/11/01  16:31:36  cg
 * bug fixed in usage of attributes of global objects derived from
 * sib file as implicitly needed. Now, the specific attribute is removed.
 * Later it can be used to distinguish between read-objects and
 * read-write-objects.
 *
 * Revision 1.29  1995/10/31  15:37:46  sbs
 * error in mod init: sib not set to NULL for external imports!
 *
 * Revision 1.28  1995/10/31  14:47:21  sbs
 * GenLinkerList modifyed : ERROR->WARNING
 *
 * Revision 1.27  1995/10/31  09:41:02  cg
 * Now, SIB information will be retrieved for functions which themselves
 * are only needed by other imported inline functions.
 *
 * Revision 1.26  1995/10/26  16:13:25  cg
 *  new function ImportOwnDeclaration used to check the declaration file
 * when compiling a module/class implementation.
 * error messages improved.
 *
 * Revision 1.25  1995/10/24  13:14:30  cg
 *  Now, all file names in error messages are written with "
 *
 * Revision 1.24  1995/10/18  16:48:19  cg
 * converted to new error macros.
 * improved error messages.
 *
 * Revision 1.23  1995/10/16  12:41:21  cg
 * new function 'ModulePrefix' added for use in typechecker.
 *
 * Revision 1.22  1995/10/12  13:58:31  cg
 * analysis structures for implicitly used items are now generated in
 * the form of
 * nodelists.
 *
 * Revision 1.21  1995/10/06  17:12:17  cg
 * calls to MakeIds adjusted to new signature (3 parameters)
 *
 * Revision 1.20  1995/09/29  12:21:30  cg
 * charlist renamed to strings.
 *
 * Revision 1.19  1995/08/30  14:05:12  cg
 * compare-macros extracted to new header-file.
 *
 * Revision 1.18  1995/08/28  16:15:00  cg
 * Bugs in GenLinkerList fixed.
 *
 * Revision 1.17  1995/08/24  14:17:09  cg
 * GenLinkerlist modified.
 * GenExtmodlist added.
 *
 * Revision 1.16  1995/08/15  09:29:24  cg
 * SIB information retrieved about:
 * -inline function bodies
 * -implicitly used objects
 * -implicitly used functions and types in inline functions
 * -implicitly imported modules/classes
 * GenLinkerList extended with respect to implicitly imported modules
 *
 * Revision 1.15  1995/08/08  09:57:28  cg
 * SIB information about hidden type implementations are now retrieved.
 *
 * Revision 1.14  1995/07/31  07:10:12  cg
 * sibs will be read and stored in modtab.
 *
 * Revision 1.13  1995/07/25  07:37:10  cg
 * global objects may be imported now.
 * class types are automatically imported.
 *
 * Revision 1.12  1995/04/05  17:24:16  sbs
 * GenLinkList inserted
 *
 * Revision 1.11  1995/04/05  15:23:20  sbs
 * GenLinkerList added
 *
 * Revision 1.10  1995/01/16  15:34:06  hw
 * changed description of FindSymbolInModul
 *
 * Revision 1.9  1995/01/06  19:30:30  sbs
 * bug fixed in AppendModsToSymbol
 *
 * Revision 1.8  1995/01/06  17:50:43  sbs
 * no_mod_ext pragma inserted
 *
 * Revision 1.7  1995/01/04  18:19:36  sbs
 * bug fixed in GenSyms (multiple implicit types/funs do work as well now!)
 *
 * Revision 1.6  1995/01/04  13:44:59  sbs
 * bug in AppendModnameToSymbol fixed:
 * done resetted to 0 after each types-node!
 *
 * Revision 1.5  1995/01/04  12:35:56  sbs
 * import mechanism including renaming (of imported symbols)
 * done. FindSymbolInModul & FreeMods provided for external use
 * (needed from the typechecker.
 *
 * Revision 1.4  1995/01/02  19:24:34  sbs
 * first release with new N_explist nodes!
 * does not (yet?) automaticly introduce the modul names for applied
 * user defined types!
 *
 * Revision 1.3  1994/12/31  14:31:32  sbs
 * preliminary (working) version
 *
 * Revision 1.2  1994/12/21  12:21:48  sbs
 * preliminary version
 *
 * Revision 1.1  1994/12/16  14:39:17  sbs
 * Initial revision
 *
 *
 */

/*
 * This file contains .....
 */

#include <string.h>
#include <limits.h>

#include "tree.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"

#include "free.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "internal_lib.h"
#include "globals.h"

#include "scnprs.h"
#include "traverse.h"

#undef TYPES /* These macros are defined in scnprs.h as well as   */
#undef ID    /* in access_macros.h. The latter definition is used */
#undef DIM   /* in this file.                                     */

#include "access_macros.h"

#include "filemgr.h"
#include "import.h"

extern void DoImport (node *modul, node *implist, char *mastermod);

static mod *mod_tab = NULL;

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
 *  functionname  : Import
 *  arguments     : 1) syntax tree
 *  description   : Recursively scans and parses modul.dec's
 *  global vars   : syntax_tree, act_tab, imp_tab
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : ---
 *
 *  remarks       :
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
 *  internal funs : ---
 *  external funs : ---
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */

mod *
FindModul (char *name)
{
    mod *tmp;

    DBUG_ENTER ("FindModul");

    DBUG_PRINT ("IMPORT", ("searching for modul/class: %s", name));
    tmp = mod_tab;
    while ((tmp != NULL) && (strcmp (tmp->name, name) != 0))
        tmp = tmp->next;

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
        FREE (tmp);
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
 *  internal funs : FindModul
 *  external funs : Malloc
 *  macros        :
 *
 *  remarks       :
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

        DBUG_PRINT ("MEMIMPORT", ("Allocating mod at" P_FORMAT " name: %s(" P_FORMAT
                                  " prefix %s (" P_FORMAT,
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
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : MakeNode, MakeTypes
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */

void
InsertClassType (node *classdec)
{
    node *explist, *tmp;

    DBUG_ENTER ("InsertClassType");

    tmp = MakeNode (N_typedef);
    tmp->info.types = MakeTypes (T_hidden);
    tmp->info.types->id = StringCopy (classdec->info.fun_name.id);
    tmp->info.types->id_mod
      = CLASSDEC_ISEXTERNAL (classdec) ? NULL : StringCopy (classdec->info.fun_name.id);

    tmp->info.types->attrib = ST_unique;
    tmp->info.types->status = ST_imported;
    tmp->lineno = 0;

    explist = classdec->node[0];

    if (explist->node[0] == NULL) /* There are no other implicit types */
    {
        explist->node[0] = tmp;
    } else {
        tmp->node[0] = explist->node[0];
#ifndef NEWTREE
        tmp->nnode++;
#endif
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
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : SearchObjdef, MakeNodelist, ModName, FreeOneIds
 *  macros        : DBUG, TREE, ERROR
 *
 *  remarks       :
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
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Malloc, FreeAllNums
 *  macros        : DBUG, TREE, ERROR
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
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Malloc, FreeAllNums
 *  macros        : DBUG, TREE, ERROR
 *
 *  remarks       :
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

/*
 *
 *  functionname  : InitGenericFuns
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

            FREE (TYPEDEF_PRAGMA (arg_node));
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : ResolvePragmaReadonly
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs : Nums2BoolArray
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

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

    FREE (PRAGMA_READONLY (pragma));

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
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Trav, FreeAllNums, FreeAllIds, FreeTree, FreeNode
 *  macros        : DBUG, FREE, ERROR, TREE
 *
 *  remarks       :
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
                FREE (PRAGMA_LINKNAME (pragma));
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
                FREE (PRAGMA_INITFUN (pragma));
            }

            if (TYPEDEF_BASETYPE (arg_node) != T_hidden) {
                if (PRAGMA_COPYFUN (pragma) != NULL) {
                    WARN (NODE_LINE (arg_node),
                          ("Pragma 'copyfun` has no effect on explicit type"));
                    FREE (PRAGMA_COPYFUN (pragma));
                }

                if (PRAGMA_FREEFUN (pragma) != NULL) {
                    WARN (NODE_LINE (arg_node),
                          ("Pragma 'freefun` has no effect on explicit type"));
                    FREE (PRAGMA_FREEFUN (pragma));
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
 *  global vars   : ---
 *  internal funs : Nums2IntArray, Nums2BoolArray, CheckExistObjects
 *  external funs : FreeAllIds, FreeTree, FreeNode, ConcatNodelist
 *  macros        : DBUG, TREE, ERROR, FREE
 *
 *  remarks       :
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
                FREE (PRAGMA_COPYFUN (pragma));
            }

            if (PRAGMA_FREEFUN (pragma) != NULL) {
                WARN (NODE_LINE (arg_node),
                      ("Pragma 'freefun` has no effect on function"));
                FREE (PRAGMA_FREEFUN (pragma));
            }

            if (PRAGMA_INITFUN (pragma) != NULL) {
                WARN (NODE_LINE (arg_node),
                      ("Pragma 'initfun` has no effect on function"));
                FREE (PRAGMA_INITFUN (pragma));
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
                FREE (PRAGMA_COPYFUN (pragma));
            }

            if (PRAGMA_FREEFUN (pragma) != NULL) {
                WARN (NODE_LINE (arg_node),
                      ("Pragma 'freefun` has no effect on global object"));
                FREE (PRAGMA_FREEFUN (pragma));
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
 *  global vars   : ---
 *  internal funs : CheckPragmaTypedef, CheckPragmaObjdef, CheckPragmaFundef
 *  external funs : ---
 *  macros        : DBUG, TREE
 *
 *  remarks       : 2) is used to distinguish between external and SAC
 *                     modules and classes.
 *
 */

node *CheckPragmas(node *moddec, char *prefix)
{
  DBUG_ENTER("CheckPragmas");
  
  if (MODDEC_OWN(moddec)!=NULL)
  {
    if (MODDEC_ITYPES(moddec)!=NULL)
    {
      MODDEC_ITYPES(moddec)
        =CheckPragmaTypedef(MODDEC_ITYPES(moddec), (node*)prefix);
    }
    
    if (MODDEC_ETYPES(moddec)!=NULL)
    {
      MODDEC_ETYPES(moddec)
        =CheckPragmaTypedef(MODDEC_ETYPES(moddec), (node*)prefix);
    }
    
    if (MODDEC_OBJS(moddec)!=NULL)
    {
      MODDEC_OBJS(moddec)
        =CheckPragmaObjdef(MODDEC_OBJS(moddec), (node*)prefix);
    }
    
    if (MODDEC_FUNS(moddec)!=NULL)
    {
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
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Malloc, strcpy, strcat
 *  macros        : NOTE, ERROR, DBUG...
 *
 *  remarks       :
 *
 */

mod *
GenMod (char *name, int checkdec)
{
    int i;
    mod *tmp;
    static char buffer[MAX_FILE_NAME];
    char *pathname, *abspathname;

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
    yyin = fopen (pathname, "r");

    if (yyin == NULL) {
        SYSABORT (("Unable to open file \"%s\"", buffer));
    } else {
        abspathname = AbsolutePathname (pathname);

        NOTE (("  Parsing file \"%s\" ...", abspathname));

        filename = buffer;

        linenum = 1;
        start_token = PARSE_DEC;
        yyparse ();

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
                                  NULL, dependencies);
                }
            } else {
                dependencies
                  = MakeDeps (StringCopy (name), StringCopy (abspathname), NULL,
                              ST_external, MODDEC_LINKWITH (decl_tree), dependencies);
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

        for (i = 0; i < 4; i++)
            tmp->syms[i] = NULL;

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
 *  internal funs : GenMod
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */

void
FindOrAppend (node *implist, int checkdec)

{
    mod *current, *last;
    int tmp;

    DBUG_ENTER ("FindOrAppend");

    DBUG_ASSERT ((implist), "FindOrAppend called with NULL-import-list!");

    if (mod_tab == NULL) /* the first modul has to be inserted anyway! */
    {
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
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Malloc
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */

void
GenSyms (mod *mod)

{
    node *explist, *ptr;
    syms *new;
    int i, next;

    DBUG_ENTER ("GenSyms");

    explist = mod->moddec->node[0];

    if (explist != NULL) {
        for (i = 0; i < 4; i++) {
            if (i == 2) /* fundefs ! */
                next = 1;
            else
                next = 0;
            ptr = explist->node[i];

            while (ptr != NULL) {
                DBUG_PRINT ("IMPORT",
                            ("inserting symbol %s of kind %d", ptr->info.types->id, i));

                new = (syms *)Malloc (sizeof (syms));
                new->id = (char *)Malloc (strlen (ptr->info.types->id) + 1);
                strcpy (new->id, ptr->info.types->id);
                new->flag = NOT_IMPORTED;
                new->next = mod->syms[i];
                mod->syms[i] = new;

                ptr = ptr->node[next]; /* next declaration! */
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
 *  global vars   : ---
 *  internal funs : FindModul
 *  external funs : Malloc
 *  macros        : DBUG...
 *
 *  remarks       :
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

            if (syms != NULL) /* name is declared in this modul; */
                              /* insert it in result!            */
            {

                DBUG_PRINT ("IMPORT",
                            ("symbol %s found in module/class %s...", name, modname));

                new = (mods *)Malloc (sizeof (mods));
                new->mod = tmpmod;
                new->syms = syms;
                new->next = found;
                found = new;
            }

            if (recursive == 1) {
                /* Now, we recursively investigate all imports! */

                imports = tmpmod->moddec->node[1]; /* pointer to imports ! */

                while (imports != NULL) {
                    if ((imports->node[1] == NULL) && (imports->node[2] == NULL)
                        && (imports->node[3] == NULL) && (imports->node[4] == NULL))

                    {
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

                        if (tmpids != NULL) /* Symbol found! */
                        {
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
 *                  and inserts the respective modul-name in types->name_mod.
 *  global vars   : ---
 *  internal funs : FindSymbolInModul, FreeMods
 *  external funs : ---
 *  macros        : DBUG...
 *
 */

void
AppendModnameToSymbol (node *symbol, char *modname)

{
    node *arg = symbol->node[2];
    mods *mods, *mods2;
    types *types;
    int done;
    char decfilename[MAX_FILE_NAME];

    DBUG_ENTER ("AppendModnameToSymbol");

    strcpy (decfilename, modname);
    strcat (decfilename, ".dec");
    filename = decfilename;
    /* only for error messages */

    types = symbol->info.types;
    while (types != NULL) {
        if (types->simpletype == T_user) {
            done = 0;
            if (types->name_mod != NULL) {
                modname = types->name_mod;
                mods = FindSymbolInModul (modname, types->name, 0, NULL, 0);
                mods2 = FindSymbolInModul (modname, types->name, 1, NULL, 0);

                if ((mods == NULL) && (mods2 != NULL)) {
                    types->name_mod = mods2->mod->prefix;
                    done = 1;
                }

                if ((mods != NULL) && (mods2 == NULL)) {
                    types->name_mod = mods->mod->prefix;
                    done = 1;
                }

                FreeMods (mods);
                FreeMods (mods2);
            };

            if (done != 1) {
                mods = FindSymbolInModul (modname, types->name, 0, NULL, 1);
                mods2 = FindSymbolInModul (modname, types->name, 1, NULL, 1);

                if (mods != NULL)
                    if (mods2 != NULL) {
                        if ((strcmp (mods->mod->name, types->name) == 0)
                            && (mods->mod->moddec->nodetype == N_classdec)) {
                            ERROR (symbol->lineno,
                                   ("Explicit type '%s:%s` "
                                    "conflicts with class '%s`"
                                    "in module/class '%s`",
                                    mods2->mod->name, types->name, types->name, modname));
                        } else {
                            ERROR (symbol->lineno,
                                   ("Implicit type '%s:%s` "
                                    "and explicit type '%s:%s` available "
                                    "in module/class '%s`",
                                    mods->mod->name, types->name, mods2->mod->name,
                                    types->name, modname));
                        }
                    }

                    else /* mods2 == NULL */
                      if (mods->next != NULL) {
                        if ((strcmp (mods->mod->name, types->name) == 0)
                            && (mods->mod->moddec->nodetype == N_classdec)) {
                            ERROR (symbol->lineno, ("Implicit type '%s:%s` "
                                                    "conflicts with class '%s` "
                                                    "in module/class '%s`",
                                                    mods->next->mod->name, types->name,
                                                    types->name, modname));
                        } else {
                            if ((strcmp (mods->next->mod->name, types->name) == 0)
                                && (mods->next->mod->moddec->nodetype == N_classdec)) {
                                ERROR (symbol->lineno, ("Implicit type '%s:%s` "
                                                        "conflicts with class '%s` "
                                                        "in module/class '%s`",
                                                        mods->mod->name, types->name,
                                                        types->name, modname));
                            } else {
                                ERROR (symbol->lineno,
                                       ("Implicit types '%s:%s` and '%s:%s` available ",
                                        "in module/class '%s`", mods->mod->name,
                                        types->name, mods->next->mod->name, types->name,
                                        modname));
                            }
                        }
                    } else /* mods->next == NULL */
                        types->name_mod = mods->mod->prefix;

                else /* mods == NULL */
                  if (mods2 == NULL) {
                    ERROR (symbol->lineno, ("No type '%s` available "
                                            "in module/class '%s`",
                                            types->name, modname));
                }

                else /* mods2 != NULL */
                  if (mods2->next != NULL) {
                    ERROR (symbol->lineno,
                           ("Explicit types '%s:%s` and '%s:%s` available ",
                            "in module/class '%s`", mods2->mod->name, types->name,
                            mods2->next->mod->name, types->name, modname));
                } else
                    types->name_mod = mods2->mod->prefix;

                FreeMods (mods);
                FreeMods (mods2);
            }
        }
        types = types->next;
        if ((types == NULL) && (symbol->nodetype == N_fundef) && (arg != NULL)) {
            types = arg->info.types;
            arg = arg->node[0];
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
 *  global vars   : ---
 *  internal funs : DoImport, AppendModnameToSymbol
 *  external funs : AppendNodeChain
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */

void
ImportAll (mod *mod, node *modul)

{
    syms *symptr;
    node *explist, *tmpnode;
    int i, next, son;

    DBUG_ENTER ("ImportAll");

    DBUG_PRINT ("IMPORT", ("importing all from module/class %s", mod->name));

    if (mod->allflag != 1) {
        explist = mod->moddec->node[0];

        for (i = 0; i < 4; i++) {
            if (i == 2)
                next = 1;
            else
                next = 0;

            tmpnode = explist->node[i];

            while (tmpnode != NULL) {
                AppendModnameToSymbol (tmpnode, mod->name);
                tmpnode = tmpnode->node[next];
            }

            if (i == 0)
                son = 1;
            else
                son = i;

            modul->node[son] = AppendNodeChain (next, explist->node[i], modul->node[son]);
        }

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

        if (mod->moddec->node[1] != NULL)
            DoImport (modul, mod->moddec->node[1], mod->name);
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
    node *tmpdef, *tmp2def;
    int next, son;

    DBUG_ENTER ("ImportSymbol");
    DBUG_PRINT ("IMPORT", ("importing symbol %s of kind %d (0=imp/1=exp/2=fun/3=obj)"
                           "from modul %s",
                           name, symbtype, mod->name));

    explist = mod->moddec->node[0];

    if (symbtype == 2)
        next = 1; /* next pointer in N_fundef nodes */
    else
        next = 0; /* next pointer in N_typedef and N_objdef nodes */

    tmpdef = explist->node[symbtype];

    if (strcmp (tmpdef->info.types->id, name) == 0)
    /* The first entry has to be moved ! */
    {
        explist->node[symbtype] = tmpdef->node[next];
        /* eliminating tmpdef from the chain */
    } else {
        tmp2def = tmpdef;
        while (strcmp (tmp2def->node[next]->info.types->id, name) != 0)
            tmp2def = tmp2def->node[next];

        tmpdef = tmp2def->node[next]; /* neu neu, war node[0] */
        tmp2def->node[next] = tmpdef->node[next];

#ifndef NEWTREE
        if (tmp2def->node[next] == NULL)
            tmp2def->nnode--;
#endif
    }

    /* tmpdef points on the def which is to be inserted */

    AppendModnameToSymbol (tmpdef, mod->name);

#ifndef NEWTREE
    if (tmpdef->node[next] == NULL)
        tmpdef->nnode++;
#endif

    /* Now, we do know, that nnode in tmpdef is set for having a successor! */

    if (symbtype == 0)
        son = 1;
    else
        son = symbtype;

    tmpdef->node[next] = modul->node[son];
    modul->node[son] = tmpdef;
#ifndef NEWTREE
    if (tmpdef->node[next] == NULL)
        tmpdef->nnode--;
#endif

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
        } else /* selective import! */
        {

#if 0
The whole stuff is moved to function AddClasstypeOnSelectiveImport

      if (mod->moddec->nodetype==N_classdec)
      {
        /*
         *  If the imported module/class actually is a class then the
         *  respective class type is added to the import list in the
         *  case of a selective import.
         */
        tmp=MakeIds(implist->info.id, NULL, ST_regular);
        tmp->next= (ids*) implist->node[1];
        implist->node[1]= (node*) tmp;
      }

#endif

            for (i = 0; i < 4; i++) {
                tmp = (ids *)implist->node[i + 1];

                while (tmp != NULL) { /* importing some symbol! */
                    mods = FindSymbolInModul (implist->info.id, tmp->id, i, NULL, 1);

                    if (mods == NULL)
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

                    else {
                        do {
                            tmpmods = mods;
                            if (mods->syms->flag == NOT_IMPORTED) {
                                mods->syms->flag = IMPORTED; /* mark symbol as imported */
                                ImportSymbol (i, tmp->id, mods->mod, modul);
                            }
                            mods = mods->next;
                            FREE (tmpmods);
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
 *  global vars   : ---
 *  internal funs : FindModul
 *  external funs : MakeIds
 *  macros        :
 *
 *  remarks       :
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
 *  internal funs : FindOrAppend, GenSyms, DoImport,
 *                  AddClasstypeOnSelectiveImport
 *  external funs : Trav
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

node *
IMmodul (node *arg_node, node *arg_info)
{
    mod *modptr;

    DBUG_ENTER ("IMmodul");

    if (arg_node->node[0] != NULL) /* there are any imports! */
    {
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
 *  internal funs : InsertClassType, AppendModnameToSymbol,
 *                  AddClasstypeOnSelectiveImport
 *  external funs : strcmp, fopen, fclose, yyparse,
 *                  Malloc, FindFile
 *  macros        : DBUG, ERROR
 *
 *  remarks       :
 *
 */

node *
ImportOwnDeclaration (char *name, file_type modtype)
{
    mod *modptr, *old_mod_tab;
    int i;
    char buffer[MAX_FILE_NAME];
    node *decl = NULL, *symbol;
    char *pathname, *abspathname;

    DBUG_ENTER ("ImportOwnDeclaration");

    old_mod_tab = mod_tab;

    mod_tab = (mod *)Malloc (sizeof (mod));
    mod_tab->name = name;
    mod_tab->flag = 0;
    mod_tab->allflag = 0;

    strcpy (buffer, name);
    strcat (buffer, ".dec");

    pathname = FindFile (MODDEC_PATH, buffer);
    yyin = fopen (pathname, "r");

    if (yyin == NULL) {
        FREE (mod_tab);
    } else {
        abspathname = AbsolutePathname (pathname);

        NOTE (("Loading own declaration !"));
        NOTE (("  Parsing file \"%s\" ...", abspathname));

        linenum = 1;
        filename = buffer;
        start_token = PARSE_DEC;
        yyparse ();
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
    }

    mod_tab = old_mod_tab;

    filename = puresacfilename;

    DBUG_RETURN (decl);
}

/*
 *
 *  functionname  : PrintDependencies
 *  arguments     : 1) list of dependencies
 *                  2) mode, corresponds to compiler option -M (1) or
 *                     -Mlib (2).
 *  description   : prints a list of dependencies in a Makefile-like style
 *                  to stdout. All declaration files of imported modules
 *                  and classes are printed including the own declaration
 *                  when compiling a module/class implementation.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : printf
 *  macros        :
 *
 *  remarks       : This function corresponds to the -M and -Mlib
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

    if (tmp != NULL) {
        tmp = depends;

        while (tmp != NULL) {
            printf ("  \\\n  %s", DEPS_DECNAME (tmp));
            tmp = DEPS_NEXT (tmp);
        }

        if (mode == 1) {
            printf ("\n");
        } else {
            tmp = depends;

            while (tmp != NULL) {
                strcpy (buffer, DEPS_NAME (tmp));

                if (DEPS_STATUS (tmp) == ST_sac) {
                    strcat (buffer, ".lib");
                } else {
                    strcat (buffer, ".a");
                }

                printf ("  \\\n  %s", buffer);
                tmp = DEPS_NEXT (tmp);
            }

            strcpy (buffer, sacfilename);
            strcpy (buffer + strlen (sacfilename) - 4, ".d");

            printf ("\n\n%s:", buffer);

            tmp = depends;

            while (tmp != NULL) {
                printf ("  \\\n  %s", DEPS_DECNAME (tmp));
                tmp = DEPS_NEXT (tmp);
            }

            printf ("\n\nalldeps:");

            tmp = depends;

            while (tmp != NULL) {
                printf ("  \\\n  %s", DEPS_NAME (tmp));
                tmp = DEPS_NEXT (tmp);
            }

            printf ("\n\n.PHONY:");

            tmp = depends;

            while (tmp != NULL) {
                printf ("  \\\n  %s", DEPS_NAME (tmp));
                tmp = DEPS_NEXT (tmp);
            }

            printf ("\n\n");

            tmp = depends;

            while (tmp != NULL) {
                printf ("%s:\n\t", DEPS_NAME (tmp));

                strcpy (buffer, DEPS_DECNAME (tmp));
                filename = strrchr (buffer, '/');
                *filename = 0;

                printf ("(cd %s; $(MAKE) %s)\n\n", buffer, DEPS_NAME (tmp));

                tmp = DEPS_NEXT (tmp);
            }
        }
    }

    DBUG_VOID_RETURN;
}
