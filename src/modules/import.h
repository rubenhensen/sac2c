/*
 *
 * $Log$
 * Revision 1.14  1995/09/26 16:52:52  cg
 * declaration of CmpDomain moved to tree_compound.h
 *
 * Revision 1.13  1995/09/01  08:44:29  cg
 * function CmpDomain added
 *
 * Revision 1.12  1995/08/30  14:04:17  cg
 * GenExtmodList and InsertClassType added.
 *
 * Revision 1.11  1995/08/15  09:28:15  cg
 * include of typecheck.h deleted.
 *
 * Revision 1.10  1995/08/08  09:52:45  cg
 * include of typecheck.h for CmpFunParams added.
 *
 * Revision 1.9  1995/07/31  07:11:02  cg
 * pointer to sib-tree inserted in modtab.
 *
 * Revision 1.8  1995/07/07  14:52:08  cg
 * struct MOD changed: pointer to global object symbols added.
 *
 * Revision 1.7  1995/04/05  15:23:20  sbs
 * GenLinkerList added
 *
 * Revision 1.6  1995/01/06  17:50:43  sbs
 * no_mod_ext pragma inserted
 *
 * Revision 1.5  1995/01/04  12:35:56  sbs
 * import mechanism including renaming (of imported symbols)
 * done. FindSymbolInModul & FreeMods provided for external use
 * (needed from the typechecker.
 *
 * Revision 1.4  1995/01/02  17:05:03  sbs
 * flag inserted in mods structure...
 *
 * Revision 1.3  1995/01/02  16:32:06  sbs
 * found arg added for FindSymbolInModul
 *
 * Revision 1.2  1995/01/02  16:08:59  sbs
 * FindSymbolInModul inserted
 *
 * Revision 1.1  1994/12/16  14:39:17  sbs
 * Initial revision
 *
 * Revision 1.1  1994/12/11  17:33:27  sbs
 * Initial revision
 *
 *
 */

#ifndef _sac_import_h

#define _sac_import_h

#define IMPORTED 1
#define NOT_IMPORTED 0

typedef struct SYMS {
    char *id; /* symbol */
    int flag; /* IMPORTED / NOT_IMPORTED */
    struct SYMS *next;
} syms;

typedef struct MOD {
    char *name;    /* modul name */
    char *prefix;  /* modul prefix */
    int flag;      /* flag for recursion protection */
    int allflag;   /* flag for recursion protection */
    node *moddec;  /* pointer to the respective N_moddec node */
    node *sib;     /* pointer to the respective N_sib node */
    syms *syms[4]; /* pointer to implicit type symbols */
                   /* pointer to explicit type symbols */
                   /* pointer to function symbols */
                   /* pointer to global object symbols */
    struct MOD *next;
} mod;

typedef struct MODS {
    mod *mod;
    syms *syms;
    struct MODS *next;
} mods;

extern void FreeMods (mods *mods);
extern mods *FindSymbolInModul (char *modname, char *name, int symbkind, mods *found,
                                int recursive);

extern node *IMmodul (node *, node *);
extern node *IMtypedef (node *, node *);
extern node *IMfundef (node *, node *);
extern node *Import (node *);
extern char *GenLinkerList ();
extern charlist *GenExtmodlistList ();
extern void InsertClassType (node *);

#endif /* _sac_import_h */
