/*
 *
 * $Log$
 * Revision 1.25  1995/10/24 13:14:30  cg
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
#include <malloc.h>

#include "tree.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"

#include "free.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "internal_lib.h"

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

/* static strings *linker_tab=NULL; */

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
        free (tmp);
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
    tmp->info.types->id = classdec->info.fun_name.id;
    tmp->info.types->id_mod = classdec->info.fun_name.id_mod;
    tmp->info.types->attrib = ST_unique;
    tmp->lineno = 0;

    explist = classdec->node[0];

    if (explist->node[0] == NULL) /* There are no other implicit types */
    {
        explist->node[0] = tmp;
    } else {
        tmp->node[0] = explist->node[0];
        tmp->nnode++;
        explist->node[0] = tmp;
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : GenMod
 *  arguments     : 1) name of modul to be read
 *  description   : Scans and parses the respective declaration
 *                  file and SIB-file and generates/initilizes a
 *                  new mod-node
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Malloc
 *  macros        : NOTE, ERROR, DBUG...
 *
 *  remarks       :
 *
 */

mod *
GenMod (char *name)

{
    int i;
    mod *tmp;
    static char buffer[MAX_FILE_NAME];

    DBUG_ENTER ("GenMod");

    tmp = (mod *)Malloc (sizeof (mod));
    tmp->name = name;
    tmp->flag = 0;
    tmp->allflag = 0;

    NOTE (("\n  Loading module/class '%s`", name));

    strcpy (buffer, name);
    strcat (buffer, ".dec");
    yyin = fopen (FindFile (MODDEC_PATH, buffer), "r");

    if (yyin == NULL) {
        SYSERROR (("Unable to open file \"%s\"", buffer));
    }

    linenum = 1;
    start_token = PARSE_DEC;
    yyparse ();

    tmp->moddec = decl_tree;
    if (strcmp (decl_tree->info.fun_name.id, name) != 0)
        SYSERROR (("File \"%s\" does not provide module/class '%s`,\n\t"
                   "but module/class '%s`",
                   buffer, name, decl_tree->info.fun_name.id));

    tmp->prefix = decl_tree->info.fun_name.id_mod;
    tmp->next = NULL;
    for (i = 0; i < 4; i++)
        tmp->syms[i] = NULL;

    if (tmp->moddec->nodetype == N_classdec) {
        InsertClassType (tmp->moddec);
    }

    if (tmp->prefix != NULL) /* module is not external */
    {
        strcpy (buffer, name);
        strcat (buffer, ".sib");
        yyin = fopen (FindFile (MODDEC_PATH, buffer), "r");

        if (yyin == NULL) {
            tmp->sib = NULL;
            DBUG_PRINT ("IMPORT", ("Module %s has no SIB-file", name));
            SYSWARN (("SAC-module/class '%s` has no SIB-file", name));
            /* SYSWARN only preliminary, later it must be a SYSERROR */
        } else {
            DBUG_PRINT ("READSIB", ("...parsing %s.sib", name));
            linenum = 1;
            start_token = PARSE_SIB;
            yyparse ();
            tmp->sib = sib_tree;
        }
    }

    DBUG_RETURN (tmp);
}

/*
 *
 *  functionname  : FindOrAppend
 *  arguments     : 1) importlist
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
FindOrAppend (node *implist)

{
    mod *current, *last;
    int tmp;
    static char store_filename[MAX_FILE_NAME];

    DBUG_ENTER ("FindOrAppend");

    DBUG_ASSERT ((implist), "FindOrAppend called with NULL-import-list!");

    strcpy (store_filename, filename);

    if (mod_tab == NULL) /* the first modul has to be inserted anyway! */
    {
        strcpy (filename, implist->info.id);
        /* only for correct error messages while parsing decs */

        mod_tab = GenMod (implist->info.id);
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
            strcpy (filename, implist->info.id);
            /* only for correct error messages while parsing decs */

            current = GenMod (implist->info.id);
            last->next = current;
        }
        implist = implist->node[0];
    }

    strcpy (filename, store_filename);

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
                new->id = (char *)Malloc (strlen (ptr->info.types->id));
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
 *  functionname  : ModulePrefix
 *  arguments     : 1) name of module
 *  description   : returns the prefix of the given module
 *  global vars   : ---
 *  internal funs : FindModul
 *  external funs : ---
 *  macros        : ---
 *
 *  remarks       :
 *
 */

char *
ModulePrefix (char *name)
{
    mod *module;
    char *prefix;

    DBUG_ENTER ("ModulePrefix");

    module = FindModul (name);
    if (module == NULL) {
        prefix = NULL;
    } else {
        prefix = module->prefix;
    }

    DBUG_RETURN (prefix);
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
    static char store_filename[MAX_FILE_NAME];

    DBUG_ENTER ("AppendModnameToSymbol");

    strcpy (store_filename, filename);
    strcpy (filename, modname);
    strcat (filename, ".dec");
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
                        ERROR (symbol->lineno, ("Implicit type '%s:%s` and "
                                                "explicit type '%s:%s` available",
                                                mods->mod->name, types->name,
                                                mods2->mod->name, types->name));

                    }

                    else /* mods2 == NULL */
                      if (mods->next != NULL) {
                        ERROR (symbol->lineno,
                               ("Implicit types '%s:%s` and '%s:%s` available",
                                mods->mod->name, types->name, mods->next->mod->name,
                                types->name));
                    }

                    else /* mods->next == NULL */
                        types->name_mod = mods->mod->prefix;

                else /* mods == NULL */
                  if (mods2 == NULL) {
                    ERROR (symbol->lineno, ("No type '%s` available", types->name));
                }

                else /* mods2 != NULL */
                  if (mods2->next != NULL) {
                    ERROR (symbol->lineno,
                           ("Explicit types '%s:%s` and '%s:%s` available",
                            mods2->mod->name, types->name, mods2->next->mod->name,
                            types->name));
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

    strcpy (filename, store_filename);

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : ImportAll
 *  arguments     : 1) modtab of modul to be inserted
 *                  2) N_modul node where the imports have to be inserted
 *  description   : inserts all symbols from a given mod * into a given modul.
 *  global vars   : ---
 *  internal funs : DoImport
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
    DBUG_PRINT ("IMPORT",
                ("importing symbol %s of kind %d (0=imp/1=exp/2=fun/3=obj) from modul %s",
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

        if (tmp2def->node[next] == NULL)
            tmp2def->nnode--;
    }

    /* tmpdef points on the def which is to be inserted */

    AppendModnameToSymbol (tmpdef, mod->name);

    if (tmpdef->node[next] == NULL)
        tmpdef->nnode++;

    /* Now, we do know, that nnode in tmpdef is set for having a successor! */

    if (symbtype == 0)
        son = 1;
    else
        son = symbtype;

    tmpdef->node[next] = modul->node[son];
    modul->node[son] = tmpdef;
    if (tmpdef->node[next] == NULL)
        tmpdef->nnode--;

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
 *  internal funs : FindModul, FindSymbolInModul
 *  external funs : ---
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
            if (mod->moddec->nodetype == N_classdec) {
                tmp = MakeIds (implist->info.id, NULL, ST_regular);
                tmp->next = (ids *)implist->node[1];
                implist->node[1] = (node *)tmp;
            }

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
                            free (tmpmods);
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
 *  functionname  : IMmodul
 *  arguments     : 1) pointer to N_modul node
 *                  2) NULL-pointer
 *  description   : Main function for import of classes/objects,
 *                  which imports all items needed. Further traversals
 *                  of typedefs and fundefs necessary to retrieve
 *                  information from SIBs.
 *  global vars   : mod_tab
 *  internal funs :
 *  external funs :
 *  macros        :
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
        FindOrAppend (arg_node->node[0]);
        modptr = mod_tab;

        while (modptr != NULL) {
            DBUG_PRINT ("IMPORT", ("analyzing module/class %s", modptr->moddec->info.id));
            if (modptr->moddec->node[1] != NULL) {
                FindOrAppend (modptr->moddec->node[1]);
            }
            GenSyms (modptr);
            modptr = modptr->next;
        }

        DoImport (arg_node, arg_node->node[0], arg_node->info.id);
        FreeImplist (arg_node->node[0]);
        arg_node->node[0] = NULL;

        /*
         *  searching SIB-information about types
         */

        if (arg_node->node[1] != NULL) {
            Trav (arg_node->node[1], NULL);
        }

        /*
         *  searching SIB-information about functions
         */

        if (arg_node->node[2] != NULL) {
            Trav (arg_node->node[2], arg_node);
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : FindSibEntry
 *  arguments     : 1) pointer to N_fundef or N_typedef node
 *  description   : finds the respective N_fundef or N_typedef node in the
 *                  sib-tree that provides additional information about
 *                  the given node.
 *  global vars   : ---
 *  internal funs : FindModul
 *  external funs : ---
 *  macros        : CMP_FUNDEF, CMP_TYPEDEF
 *
 *  remarks       :
 *
 */

node *
FindSibEntry (node *orig)
{
    char *mod_name;
    node *sib_entry = NULL;
    mod *mod;

    DBUG_ENTER ("FindSibEntry");

    mod_name = orig->ID_MOD;
    if (mod_name != NULL) {
        mod = FindModul (mod_name);
        if (mod != NULL) {
            sib_entry = mod->sib;
            if (sib_entry != NULL) {
                if (orig->nodetype == N_typedef) {
                    sib_entry = sib_entry->node[0];
                    while ((sib_entry != NULL) && (!CMP_TYPEDEF (sib_entry, orig))) {
                        sib_entry = sib_entry->node[0];
                    }
                } else {
                    sib_entry = sib_entry->node[1];
                    while ((sib_entry != NULL) && (!CMP_FUNDEF (sib_entry, orig))) {
                        sib_entry = sib_entry->node[1];
                    }
                }
            }
        }
    }

    if (sib_entry == NULL) {
        DBUG_PRINT ("READSIB", ("NO SIB-entry found for %s:%s", orig->ID_MOD, orig->ID));
    } else {
        DBUG_PRINT ("READSIB", ("SIB-entry found for %s:%s", orig->ID_MOD, orig->ID));
    }

    DBUG_RETURN (sib_entry);
}

/*
 *
 *  functionname  : AddToLinkerTab
 *  arguments     : 1) module name to be added
 *  description   : adds a module name to the global linker_tab. This
 *                  module is implicitly imported and must therefore be
 *                  included in the link list for the C compiler
 *  global vars   : linker_tab
 *  internal funs : ---
 *  external funs : strcmp, Malloc
 *  macros        :
 *
 *  remarks       :
 *
 */

/*
void AddToLinkerTab(char *module)
{
  strings *tmp=linker_tab;

  DBUG_ENTER("AddToLinkerTab");

  while ((tmp!=NULL) && strcmp(tmp->name, module)!=0)
  {
    tmp=tmp->next;
  }

  if (tmp==NULL)
  {
    tmp=(strings*)Malloc(sizeof(strings));
    tmp->next=linker_tab;
    tmp->name=module;
    linker_tab=tmp;

      DBUG_PRINT("READSIB",
                 ("Implicitly imported module %s added to linker list.",
                  tmp->name));
  }

  DBUG_VOID_RETURN;
}
*/

/*
 *
 *  functionname  : AddSymbol
 *  arguments     : 1) name of symbol
 *                  2) module of symbol
 *                  3) type of symbol
 *  description   : adds implicitly imported symbol to global mod_tab if
 *                  it is yet unknown
 *  global vars   : ---
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
        tmp->name = name;
        tmp->prefix = name;
        tmp->flag = 0;
        tmp->allflag = 0;
        tmp->moddec = NULL;
        tmp->sib = NULL;
        for (i = 0; i < 4; i++) {
            tmp->syms[i] = NULL;
        }

        tmp->syms[symbkind] = (syms *)Malloc (sizeof (syms));
        tmp->syms[symbkind]->id = name;
        tmp->syms[symbkind]->next = NULL;
        tmp->syms[symbkind]->flag = 0;
    } else {
        sym = tmp->syms[symbkind];
        while ((sym != NULL) && (strcmp (sym->id, name) != 0)) {
            sym = sym->next;
        }
        if (sym == NULL) {
            sym = (syms *)Malloc (sizeof (syms));
            sym->id = name;
            sym->flag = 0;
            sym->next = tmp->syms[symbkind];
            tmp->syms[symbkind] = sym;
        }
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : EnsureExistObjects
 *  arguments     : 1) objdef node from sib, contains an object that is
 *                     implicitly used by one of the sib-functions
 *                  2) modul node of program
 *  description   : ensures that the object is declared in the syntax tree.
 *                  If the object is yet unknown, the objdef node
 *                  is inserted. A nodelist of all objects needed by the
 *                  particular function is generated and returned.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : MakeNodelist
 *  macros        : CMP_OBJDEF
 *
 *  remarks       :
 *
 */

nodelist *
EnsureExistObjects (node *object, node *modul)
{
    node *find_obj, *next;
    nodelist *objlist = NULL;

    DBUG_ENTER ("EnsureExistObjects");

    while (object != NULL) {
        find_obj = modul->node[3];

        while ((find_obj != NULL) && (!CMP_OBJDEF (object, find_obj))) {
            find_obj = find_obj->node[0];
        }

        next = object->node[0];

        if (find_obj == NULL) /* the object does not yet exist */
        {
            find_obj = object;
            find_obj->node[0] = modul->node[3];
            find_obj->nnode = (modul->node[3] == NULL) ? 0 : 1;

            modul->node[3] = find_obj;

            /*
                  if (FindModul(object->ID_MOD) == NULL)
                  {
                    AddToLinkerTab(object->ID_MOD);
                  }
            */
            if (object->ID_MOD != NULL) {
                AddSymbol (object->ID, object->ID_MOD, 3);
            }

            /* new symbol is added to mod_tab if it's a sac-symbol */

            DBUG_PRINT ("READSIB", ("Implicitly used object %s:%s inserted.",
                                    find_obj->ID_MOD, find_obj->ID));

        } else {
            free (object);

            DBUG_PRINT ("READSIB", ("Implicitly used object %s:%s already exists.",
                                    find_obj->ID_MOD, find_obj->ID));
        }

        objlist = MakeNodelist (find_obj, ST_regular, objlist);

        object = next;
    }

    DBUG_RETURN (objlist);
}

/*
 *
 *  functionname  : EnsureExistTypes
 *  arguments     : 1) typedef node from sib, contains a type that is
 *                     implicitly used by one of the sib-functions
 *                  2) modul node of program
 *  description   : ensures that the type is declared in the syntax tree.
 *                  If the type is yet unknown, the typedef node
 *                  is inserted. A nodelist of all types needed by the
 *                  particular function is generated and returned.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : MakeNodelist
 *  macros        : CMP_TYPEDEF
 *
 *  remarks       :
 *
 */

nodelist *
EnsureExistTypes (node *type, node *modul)
{
    node *find_type, *next;
    nodelist *typelist = NULL;

    DBUG_ENTER ("EnsureExistTypes");

    while (type != NULL) {
        find_type = modul->node[1];
        while ((find_type != NULL) && (!CMP_TYPEDEF (type, find_type))) {
            find_type = find_type->node[0];
        }

        next = type->node[0];

        if (find_type == NULL) /* the type does not yet exist */
        {
            find_type = type;

            find_type->node[0] = modul->node[1];
            find_type->nnode = (modul->node[1] == NULL) ? 0 : 1;
            modul->node[1] = find_type;

            if (type->ID_MOD != NULL) {
                AddSymbol (type->ID, type->ID_MOD, 1);
            }

            /* new symbol is added to mod_tab if it's a sac-symbol */

            DBUG_PRINT ("READSIB",
                        ("Implicitly used type %s:%s inserted.", type->ID_MOD, type->ID));

        } else {
            DBUG_PRINT ("READSIB", ("Implicitly used type %s:%s already exists.",
                                    type->ID_MOD, type->ID));

            free (type);
        }

        typelist = MakeNodelist (find_type, ST_regular, typelist);

        type = next;
    }

    DBUG_RETURN (typelist);
}

/*
 *
 *  functionname  : EnsureExistFuns
 *  arguments     : 1) fundef node from sib, contains a function that is
 *                     implicitly used by one of the sib-functions
 *                  2) modul node of program
 *  description   : ensures that the function is declared in the syntax tree.
 *                  If the function is yet unknown, the fundef node
 *                  is inserted. A nodelist of all functions needed by the
 *                  particular function is generated and returned.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : MakeNodelist
 *  macros        : CMP_FUNDEF
 *
 *  remarks       :
 *
 */

nodelist *
EnsureExistFuns (node *fundef, node *modul)
{
    node *find_fun, *next;
    nodelist *funlist = NULL;

    DBUG_ENTER ("EnsureExistFuns");

    while (fundef != NULL) /* search function */
    {
        find_fun = modul->node[2];
        while ((find_fun != NULL) && (!CMP_FUNDEF (fundef, find_fun))) {
            find_fun = find_fun->node[1];
        }

        next = fundef->node[1];

        if (find_fun == NULL) /* the function does not yet exist */
        {
            find_fun = fundef;

            find_fun->node[1] = modul->node[2];

            FUNDEF_NEEDOBJS (find_fun) = EnsureExistObjects (find_fun->node[4], modul);

            find_fun->nnode = (modul->node[2] == NULL) ? 1 : 2;
            modul->node[2] = find_fun;

            /*
                  if (FindModul(fundef->ID_MOD) == NULL)
                  {
                    AddToLinkerTab(fundef->ID_MOD);
                  }
            */
            if (fundef->ID_MOD != NULL) {
                AddSymbol (fundef->ID, fundef->ID_MOD, 2);
            }

            /* new symbol is added to mod_tab if it's a sac-symbol */

            DBUG_PRINT ("READSIB", ("Implicitly used function %s:%s inserted.",
                                    fundef->ID_MOD, fundef->ID));

        } else {
            DBUG_PRINT ("READSIB", ("Implicitly used function %s:%s already exists.",
                                    fundef->ID_MOD, fundef->ID));

            free (fundef);
        }

        funlist = MakeNodelist (find_fun, ST_regular, funlist);

        fundef = next;
    }

    DBUG_RETURN (funlist);
}

/*
 *
 *  functionname  : IMfundef
 *  arguments     : 1) pointer to N_fundef node
 *                  2) pointer to N_modul node of respective program
 *  description   : retrieves information from SIB for respective function.
 *                  Implicitly used types, objects, and other functions
 *                  are imported if necessary. Inline information is
 *                  stored as regular function body.
 *  global vars   : ---
 *  internal funs : FindSibEntry, MakeArgList, EnsureExistObjects,
 *                  EnsureExistTypes, EnsureExistFuns
 *  external funs : Trav
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
IMfundef (node *arg_node, node *arg_info)
{
    node *sib_entry;

    DBUG_ENTER ("IMfundef");

    if (arg_node->ID_MOD != NULL) {
        sib_entry = FindSibEntry (arg_node);

        if (sib_entry != NULL) /* SIB information available */
        {
            if (sib_entry->node[0] == NULL) /* only implicit object information */
            {
                FUNDEF_NEEDOBJS (arg_node)
                  = EnsureExistObjects (sib_entry->node[4], arg_info);

                DBUG_PRINT ("READSIB",
                            ("Adding implicit object information to function %s:%s",
                             arg_node->info.types->id_mod, arg_node->info.types->id));

            } else /* function inlining information */
            {
                arg_node->node[0] = sib_entry->node[0]; /* take body from SIB */
                arg_node->node[2] = sib_entry->node[2]; /* take args from SIB */

                FUNDEF_NEEDOBJS (arg_node)
                  = EnsureExistObjects (sib_entry->node[4], arg_info);

                FUNDEF_NEEDTYPES (arg_node)
                  = EnsureExistTypes (sib_entry->node[3], arg_info);

                FUNDEF_NEEDFUNS (arg_node)
                  = EnsureExistFuns (sib_entry->node[5], arg_info);

                arg_node->flag = 1; /* inline flag */

                DBUG_PRINT ("READSIB", ("Adding inline information to function %s:%s",
                                        arg_node->ID_MOD, arg_node->ID));
            }
        }
    }

    if (arg_node->node[1] != NULL) {
        Trav (arg_node->node[1], arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IMtypedef
 *  arguments     : 1) pointer to N_typedef node
 *                  2) unused, necessary for traversal mechanism
 *  description   : retrieves information from sib for specific type
 *                  definition. The additional types-structure containing
 *                  the implementation of a T_hidden type is added as a
 *                  second types-structure reusing the "next" entry.
 *  global vars   : ---
 *  internal funs : FindSibEntry
 *  external funs : Trav
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
IMtypedef (node *arg_node, node *arg_info)
{
    node *sib_entry;

    DBUG_ENTER ("IMtypedef");

    if (arg_node->info.types->simpletype == T_hidden) {
        sib_entry = FindSibEntry (arg_node);
        if (sib_entry != NULL) {
            arg_node->info.types->next = sib_entry->info.types;

            DBUG_PRINT ("READSIB",
                        ("adding implementation of hidden type %s:%s",
                         MOD (arg_node->info.types->id_mod), arg_node->info.types->id));
        }
    }

    if (arg_node->node[0] != NULL) {
        Trav (arg_node->node[0], NULL);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : AddToLinkList
 *  arguments     : ---
 *  description   : generates list of modules for C linker.
 *  global vars   : mod_tab, linker_tab
 *  internal funs :
 *  external funs : strcpy, strcat, FindFile
 *  macros        :
 *
 *  remarks       :
 *
 */

strings *
AddToLinkList (strings *list, char *name)
{
    strings *tmp = list;

    DBUG_ENTER ("AddToLinkList");

    while ((tmp != NULL) && (strcmp (tmp->name, name) != 0)) {
        tmp = tmp->next;
    }

    if (tmp == NULL) {
        tmp = (strings *)Malloc (sizeof (strings));
        tmp->name = name;
        tmp->next = list;
    } else {
        tmp = list;
    }

    DBUG_RETURN (tmp);
}

/*
 *
 *  functionname  : GenExtmodlistList
 *  arguments     : ---
 *  description   : generates list of imported external modules/classes.
 *                  This list is needed as additional linker list for
 *                  implicitly imported external symbols
 *  global vars   : mod_tab
 *  internal funs :
 *  external funs : Malloc
 *  macros        :
 *
 *  remarks       :
 *
 */

strings *
GenExtmodlistList ()
{
    mod *modp = mod_tab;
    strings *tmp, *linklist = NULL;

    DBUG_ENTER ("GenExtmodlistList");

    while (modp != NULL) {
        if (modp->prefix == NULL) /* external module/class */
        {
            tmp = (strings *)Malloc (sizeof (strings));
            tmp->name = modp->name;
            tmp->next = linklist;
            linklist = tmp;
        }
    }

    DBUG_RETURN (linklist);
}

/*
 *
 *  functionname  : GenLinkerList
 *  arguments     : ---
 *  description   : generates list of modules for C linker.
 *  global vars   : mod_tab
 *  internal funs : AddToLinkList
 *  external funs : strcpy, strcat, FindFile
 *  macros        :
 *
 *  remarks       :
 *
 */

char *
GenLinkerList ()
{
    mod *modp = mod_tab;
    strings *linklist = NULL, *tmp;
    static char buffer[MAX_FILE_NAME];
    static char list[MAX_PATH_LEN];
    char *file;

    DBUG_ENTER ("GenLinkerList");

    while (modp != NULL) {
        linklist = AddToLinkList (linklist, modp->name);
        if (modp->sib != NULL) {
            tmp = (strings *)modp->sib->node[2];
            while (tmp != NULL) {
                linklist = AddToLinkList (linklist, tmp->name);
                tmp = tmp->next;
            }
        }
        modp = modp->next;
    }

    while (linklist != NULL) {
        strcpy (buffer, linklist->name);
        strcat (buffer, ".o");
        file = FindFile (MODIMP_PATH, buffer);

        if (file) {
            strcat (list, " ");
            strcat (list, file);
        } else {
            SYSERROR (("Unable to find file \"%s\"", buffer));
        }
        linklist = linklist->next;
    }

    DBUG_RETURN (list);
}
