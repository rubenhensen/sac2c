/*
 *
 * $Log$
 * Revision 1.4  1995/01/02 19:24:34  sbs
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

#include <malloc.h>
#include <string.h>
#include <limits.h>

#include "dbug.h"
#include "tree.h"
#include "free.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"

#include "scnprs.h"
#include "traverse.h"

#include "filemgr.h"
#include "import.h"

extern void DoImport (node *modul, node *implist, char *mastermod);

static mod *mod_tab = NULL;
static int cnt = 1;

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

    DBUG_PRINT ("MODUL", ("searching for modul: %s", name));
    tmp = mod_tab;
    while ((tmp != NULL) && (strcmp (tmp->name, name) != 0)) {
        tmp = tmp->next;
    }

    DBUG_RETURN (tmp);
}

/*
 *
 *  functionname  : GenMod
 *  arguments     : 1) name of modul to be read
 *  description   : Scans and parses the respective declaration
 *                  file and generates/initilizes a new mod-node
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : malloc
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

    tmp = (mod *)malloc (sizeof (mod));
    tmp->name = name;
    tmp->flag = 0;

    strcpy (buffer, name);
    strcat (buffer, ".dec");
    yyin = FindFile (MODDEC_PATH, buffer);
    if (yyin == NULL) {
        ERROR2 (1, ("Couldn't open file \"%s\"!\n", buffer));
    }
    NOTE (("\n  Loading %s ...", buffer));
    linenum = 1;
    start_token = PARSE_DEC;
    yyparse ();

    tmp->moddec = decl_tree;
    tmp->next = NULL;
    for (i = 0; i < 3; i++)
        tmp->syms[i] = NULL;

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

    DBUG_ENTER ("FindOrAppend");
    DBUG_ASSERT ((implist), "FindOrAppend called with NULL-import-list!");

    if (mod_tab == NULL) { /* the first modul has to be inserted anyway! */
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
            current = GenMod (implist->info.id);
            last->next = current;
        }
        implist = implist->node[0];
    }
    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : GenSyms
 *  arguments     : 1) mod-node
 *  description   : generates the symbol tables for the given mod-node.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : malloc
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
    int i;

    DBUG_ENTER ("GenSyms");

    explist = mod->moddec->node[0];
    if (explist != NULL) {
        for (i = 0; i < 3; i++) {
            ptr = explist->node[i];
            while (ptr != NULL) {
                DBUG_PRINT ("MODUL", ("inserting symbol %s", ptr->info.types->id));
                new = (syms *)malloc (sizeof (syms));
                new->id = (char *)malloc (strlen (ptr->info.types->id));
                strcpy (new->id, ptr->info.types->id);
                new->flag = NOT_IMPORTED;
                new->next = mod->syms[1];
                mod->syms[1] = new;
                if (i < 2)              /*typedefs ! */
                    ptr = ptr->node[0]; /* next type declaration! */
                else
                    ptr = ptr->node[1]; /* next function declaration! */
            }
        }
    }

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
    node *explist;
    int i, next;

    DBUG_ENTER ("ImportAll");

    DBUG_PRINT ("MODUL", ("importing all from %s", mod->name));
    if (mod->allflag != 1) {
        explist = mod->moddec->node[0];

        for (i = 0; i < 3; i++) {
            if (i < 2)
                next = 0;
            else
                next = 1;
            modul->node[next + 1]
              = AppendNodeChain (next, explist->node[i], modul->node[next + 1]);
        }

        /* We mark all syms entries as imported! */
        for (i = 0; i < 3; i++) {
            symptr = mod->syms[i];
            while (symptr != NULL) {
                symptr->flag = IMPORTED;
                symptr = symptr->next;
            }
        }
        mod->allflag = 1; /* mark mod as beeing imported completely */

        /* Last, but not least, we recursively import the imported modules imports! */
        if (mod->moddec->node[1] != NULL)
            DoImport (modul, mod->moddec->node[1], mod->name);
    } else {
        DBUG_PRINT ("MODUL", ("import of %s skipped", mod->name));
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
 *                  4) mods found previously; initially a NULL is given here.
 *  description   : searches for a symbol of given kind in a prespecified
 *                  modul; returns a pointer linked list of modul-entries
 *                  for all those modules which own a definition of the
 *                  given symbol and are imported into the given modul.
 *  global vars   : ---
 *  internal funs : FindModul
 *  external funs : malloc
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */

mods *
FindSymbolInModul (char *modname, char *name, int symbkind, mods *found)

{
    node *imports;
    mods *new;
    mod *tmpmod;
    syms *syms;
    ids *tmpids;
    int done = 0;

    DBUG_ENTER ("FindSymbolInModul");

    DBUG_PRINT ("MODUL", ("searching for symbol %s in modul %s...", name, modname));
    tmpmod = FindModul (modname);
    if (tmpmod != NULL) {
        if (tmpmod->flag != cnt) {
            tmpmod->flag = cnt;

            /* Find syms entry name */
            syms = tmpmod->syms[symbkind];
            while ((syms != NULL) && (strcmp (syms->id, name) != 0))
                syms = syms->next;

            if (syms != NULL) { /* name is declared in this modul; insert it in result! */
                DBUG_PRINT ("MODUL", ("symbol %s found in modul %s...", name, modname));

                new = (mods *)malloc (sizeof (mods));
                new->mod = tmpmod;
                new->flag = syms->flag;
                syms->flag = IMPORTED; /* mark symbol as imported */
                new->next = found;
                found = new;
            }

            /* Now, we recursively investigate all imports! */
            imports = tmpmod->moddec->node[1]; /* pointer to imports ! */

            while ((imports != NULL) && (done != 1)) {
                if ((imports->node[1] == NULL) && (imports->node[2] == NULL)
                    && (imports->node[3] == NULL)) {
                    /* import all ! */
                    found = FindSymbolInModul (imports->info.id, name, symbkind, found);
                } else {
                    /* selective import! */
                    /* Find name in ids */
                    tmpids = (ids *)imports->node[symbkind + 1];
                    while ((tmpids != NULL) && (strcmp (tmpids->id, name) != 0))
                        tmpids = tmpids->next;

                    if (tmpids != NULL) { /* Symbol found! */
                        found
                          = FindSymbolInModul (imports->info.id, name, symbkind, found);
                    }
                }
                imports = imports->node[0];
            }
        }
    }

    DBUG_RETURN (found);
}

/*
 *
 *  functionname  : ImportSymbol
 *  arguments     : 1) symbtype indicates the kind of symbol to be imported
 *                     0 - implicit type / 1 - explicit type / 2 - function
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
    int next;

    DBUG_ENTER ("ImportSymbol");
    DBUG_PRINT ("MODUL",
                ("importing symbol %s of kind %d (0=imp/1=exp/2=fun) from modul %s", name,
                 symbtype, mod->name));

    explist = mod->moddec->node[0];
    if (symbtype < 2)
        next = 0; /* next pointer in N_typedef nodes */
    else
        next = 1; /* next pointer in N_fundef nodes */

    tmpdef = explist->node[symbtype];
    if (strcmp (tmpdef->info.types->id, name)
        == 0) { /* The first entry has to be moved ! */
        explist->node[symbtype]
          = tmpdef->node[next]; /* eliminating tmpdef from the chain */
    } else {
        tmp2def = tmpdef;
        while (strcmp (tmp2def->node[next]->info.types->id, name) != 0) {
            tmp2def = tmp2def->node[next];
        }
        tmpdef = tmp2def->node[0];
        tmp2def->node[next] = tmpdef->node[next];
        if (tmp2def->node[next] == NULL)
            tmp2def->nnode--;
    }

    /* tmpdef points on the def which is to be inserted */
    if (tmpdef->node[next] == NULL)
        tmpdef->nnode++;

    /* Now, we do know, that nnode in tmpdef is set for having a successor! */
    tmpdef->node[next] = modul->node[next + 1];
    modul->node[next + 1] = tmpdef;
    if (tmpdef->node[next] == NULL) {
        tmpdef->nnode--;
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
 *  internal funs : FindModul, FindSymbolInModul
 *  external funs : ---
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */

void
DoImport (node *modul, node *implist, char *mastermod)

{
    int i;
    mods *mods, *tmpmods;
    mod *mod;
    ids *tmp;

    DBUG_ENTER ("DoImport");

    while (implist != NULL) {
        if ((implist->node[1] == NULL) && (implist->node[2] == NULL)
            && (implist->node[3] == NULL)) {
            /* this is an import all!! */
            mod = FindModul (implist->info.id);
            ImportAll (mod, modul);
        } else { /* selective import! */
            for (i = 0; i < 3; i++) {
                tmp = (ids *)implist->node[i + 1];
                while (tmp != NULL) { /* importing some symbol! */
                    mods = FindSymbolInModul (implist->info.id, tmp->id, i, NULL);
                    if (mods == NULL)
                        switch (i) {
                        case 0:
                            ERROR1 (
                              ("declaration error in %s: no implicit type %s in %s!",
                               mastermod, tmp->id, implist->info.id));
                            break;
                        case 1:
                            ERROR1 (
                              ("declaration error in %s: no explicit type %s in %s!",
                               mastermod, tmp->id, implist->info.id));
                            break;
                        case 2:
                            ERROR1 (
                              ("declaration error in %s: no function %s declared in %s!",
                               mastermod, tmp->id, implist->info.id));
                            break;
                        }
                    else {
                        do {
                            tmpmods = mods;
                            if (mods->flag == NOT_IMPORTED)
                                ImportSymbol (i, tmp->id, mods->mod, modul);
                            mods = mods->next;
                            free (tmpmods);
                        } while (mods != NULL);
                    }

                    tmp = tmp->next;
                    cnt++;
                }
            }
        }
        implist = implist->node[0];
    }

    DBUG_VOID_RETURN;
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

node *
IMmodul (node *arg_node, node *arg_info)
{
    mod *modptr;

    DBUG_ENTER ("IMmodul");
    if (arg_node->node[0] != NULL) { /* there are any imports! */
        FindOrAppend (arg_node->node[0]);
        modptr = mod_tab;
        while (modptr != NULL) {
            DBUG_PRINT ("MODUL", ("analyzing modul %s", modptr->moddec->info.id));
            if (modptr->moddec->node[1] != NULL) {
                FindOrAppend (modptr->moddec->node[1]);
            }
            GenSyms (modptr);
            modptr = modptr->next;
        }

        DoImport (arg_node, arg_node->node[0], arg_node->info.id);
        FreeImplist (arg_node->node[0]);
        arg_node->node[0] = NULL;

        NOTE (("\n"));
    }
    DBUG_RETURN (arg_node);
}
