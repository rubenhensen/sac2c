#ifndef _import_h

/*
 *
 * $Log$
 * Revision 1.4  1995/01/02 17:05:03  sbs
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

#define _import_h

#define IMPORTED 1
#define NOT_IMPORTED 0

typedef struct SYMS {
    char *id; /* symbol */
    int flag; /* IMPORTED / NOT_IMPORTED */
    struct SYMS *next;
} syms;

typedef struct MOD {
    char *name;    /* modul name */
    int flag;      /* flag for recursion protection */
    int allflag;   /* flag for recursion protection */
    node *moddec;  /* pointer to the respective N_moddec node */
    syms *syms[3]; /* pointer to implicit type symbols */
                   /* explicit type symbols */
                   /* pointer to function symbols */
    struct MOD *next;
} mod;

typedef struct MODS {
    mod *mod;
    int flag; /* IMPORTED / NOT_IMPORTED */
    struct MODS *next;
} mods;

extern mods *FindSymbolInModul (char *modname, char *name, int symbkind, mods *found);

extern node *IMmodul (node *, node *);
extern node *Import (node *);

#endif /* _import_h */
