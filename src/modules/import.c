/*
 *
 * $Log$
 * Revision 1.2  1994/12/21 12:21:48  sbs
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

#define IMPORTED 0
#define NOT_IMPORTED 1

typedef struct SYMS {
    char *id; /* symbol */
    int flag; /* IMPORTED / NOT_IMPORTED */
    struct SYMS *next;
} syms;

typedef struct SYMB {
    char *name;    /* modul name */
    int flag;      /* flag for recursion protection */
    node *moddec;  /* pointer to the respective N_moddec node */
    syms *syms[3]; /* pointer to implicit type symbols */
                   /* explicit type symbols */
                   /* pointer to function symbols */
    struct SYMB *next;
} symb;

extern void DoImport (symb *symbtab, node *modul, node *implist);

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

symb *
GenSymb (char *name)

{
    symb *symbtab;
    static char buffer[MAX_FILE_NAME];

    DBUG_ENTER ("GenSymb");

    symbtab = (symb *)malloc (sizeof (symb));
    symbtab->name = name;
    symbtab->flag = 0;

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

    symbtab->moddec = decl_tree;
    symbtab->next = NULL;

    DBUG_RETURN (symbtab);
}

#if 0

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

symb *FindOrInsert(symb *symbtab, node *implist)

{
  symb *start, *last;
  int tmp;

  DBUG_ENTER("FindOrInsert");
  DBUG_ASSERT((implist),"FindOrInsert called with NULL-import-list!");
    
  if( symbtab == NULL ) {
    symbtab=GenSymb(implist->info.id);
    implist=implist->node[0];
  }
  start=symbtab;
  DBUG_ASSERT((start),"Empty symbtab!");

  while(implist != NULL) {
    tmp=strcmp(symbtab->name, implist->info.id);
    if(tmp >0) {
      start=GenSymb(implist->info.id);
      implist=implist->node[0];
    }
    else {
      if(tmp < 0) {
        last=symbtab;
        symbtab=symbtab->next;
        while(( symbtab != NULL) && (tmp=strcmp(symbtab->name, implist->info.id)<0)) {
            last=symbtab;
            symbtab=symbtab->next;
        }
        if(tmp != 0) {
          symbtab=GenSymb(implist->info.id);
          symbtab->next=last->next;
          last->next=symbtab;
        }
        symbtab=start;
      }
      implist=implist->node[0];
    }
  }
  DBUG_RETURN(start);
}

#endif

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

symb *
FindOrAppend (symb *symbtab, node *implist)

{
    symb *start, *last;
    int tmp;

    DBUG_ENTER ("FindOrAppend");
    DBUG_ASSERT ((implist), "FindOrAppend called with NULL-import-list!");

    if (symbtab == NULL) {
        symbtab = GenSymb (implist->info.id);
        implist = implist->node[0];
    }
    start = symbtab;
    DBUG_ASSERT ((symbtab), "Empty symbtab!");

    while (implist != NULL) {
        do {
            tmp = strcmp (symbtab->name, implist->info.id);
            last = symbtab;
            symbtab = symbtab->next;
        } while ((symbtab != NULL) && (tmp != 0));
        if (tmp != 0) {
            symbtab = GenSymb (implist->info.id);
            last->next = symbtab;
        }
        implist = implist->node[0];
        symbtab = start;
    }
    DBUG_RETURN (start);
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

void
GenSyms (symb *symbtab)

{
    node *explist, *expty, *funs;
    ids *impty;
    syms *new;

    DBUG_ENTER ("GenSyms");

    explist = symbtab->moddec->node[0];
    if (explist != NULL) {
        expty = explist->node[0];
        funs = explist->node[1];
        impty = explist->info.ids;
        while (impty != NULL) {
            DBUG_PRINT ("MODUL", ("inserting implicit type %s", impty->id));
            new = (syms *)malloc (sizeof (syms));
            new->id = impty->id;
            new->flag = NOT_IMPORTED;
            new->next = symbtab->syms[0];
            symbtab->syms[0] = new;
            impty = impty->next; /* next implicit type declaration! */
        }
        while (expty != NULL) {
            DBUG_PRINT ("MODUL", ("inserting explicit type %s", expty->info.types->name));
            new = (syms *)malloc (sizeof (syms));
            new->id = expty->info.types->name;
            new->flag = NOT_IMPORTED;
            new->next = symbtab->syms[1];
            symbtab->syms[1] = new;
            expty = expty->node[0]; /* next explicit type declaration! */
        }
        while (funs != NULL) {
            DBUG_PRINT ("MODUL", ("inserting fundec %s", funs->info.types->id));
            new = (syms *)malloc (sizeof (syms));
            new->id = funs->info.types->id;
            new->flag = NOT_IMPORTED;
            new->next = symbtab->syms[2];
            symbtab->syms[2] = new;
            funs = funs->node[1]; /* next function declaration! */
        }
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
GenTypedefFromId (char *name)

{
    node *type_def;
    types *tmp;

    DBUG_ENTER ("GenTypedefFromId");

    type_def = MakeNode (N_typedef);
    type_def->info.types = tmp = GEN_NODE (types);
    tmp->simpletype = T_hidden;
    tmp->dim = 0;
    tmp->id = NULL;
    tmp->name = name;
    tmp->next = NULL;

    DBUG_RETURN (type_def);
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

void
ImportAll (symb *symbtab, symb *symb, node *modul)

{
    syms *symptr;
    node *explist, *tmp, *tmp2;
    ids *tmpids;
    int i;

    DBUG_ENTER ("ImportAll");

    if (symb->flag < INT_MAX) {
        explist = symb->moddec->node[0];

        /* First, typedefs are generated for each implicit type */
        tmpids = explist->info.ids;
        tmp = NULL;
        while (tmpids != NULL) {
            tmp2 = GenTypedefFromId (tmpids->id);
            tmp2->node[0] = tmp;
            if (tmp != NULL)
                tmp2->nnode = 1;
            tmp = tmp2;
            tmpids = tmpids->next;
        }

        /* Now, we insert all typedefs and fundecl's in the N_modul and free the ids
         * from the N_moddec !
         */
        tmp = AppendNodeChain (0, tmp, explist->node[0]);
        modul->node[1] = AppendNodeChain (0, tmp, modul->node[1]);
        FreeIdsOnly (explist->info.ids);
        explist->info.ids = NULL;
        explist->node[0] = NULL;
        modul->node[2] = AppendNodeChain (1, explist->node[1], modul->node[2]);
        explist->node[1] = NULL;

        /* We mark all syms entries as imported! */
        for (i = 0; i < 3; i++) {
            symptr = symb->syms[i];
            while (symptr != NULL) {
                symptr->flag = IMPORTED;
                symptr = symptr->next;
            }
        }
        symb->flag = INT_MAX; /* mark symb as beeing imported completely */

        /* Last, but not least, we recursively import the imported modules imports! */
        DoImport (symbtab, modul, modul->node[0]);
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

symb *
FindModul (symb *symbtab, char *name)
{

    DBUG_ENTER ("FindModul");

    while (symbtab->name != name)
        symbtab = symbtab->next;

    DBUG_RETURN (symbtab);
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

void
DoImport (symb *symbtab, node *modul, node *implist)

{
    symb *symb;

    DBUG_ENTER ("DoImport");

    if ((implist->node[1] == NULL) && (implist->node[2] == NULL)
        && (implist->node[3] == NULL)) {
        /* this is an import all!! */
        symb = FindModul (symbtab, implist->info.id);
        ImportAll (symbtab, symb, modul);
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
    symb *symbtab, *tmp_symbtab;

    DBUG_ENTER ("IMmodul");
    if (arg_node->node[0] != NULL) { /* there are any imports! */
        symbtab = tmp_symbtab = FindOrAppend (NULL, arg_node->node[0]);
        while (tmp_symbtab != NULL) {
            DBUG_PRINT ("MODUL", ("analyzing modul %s", tmp_symbtab->moddec->info.id));
            if (tmp_symbtab->moddec->node[1] != NULL) {
                FindOrAppend (symbtab, tmp_symbtab->moddec->node[1]);
            }
            GenSyms (tmp_symbtab);
            tmp_symbtab = tmp_symbtab->next;
        }

        DoImport (symbtab, arg_node, arg_node->node[0]);

        NOTE (("\n"));
    }
    DBUG_RETURN (arg_node);
}
