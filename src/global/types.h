/*
 *
 * $Log$
 * Revision 1.8  1995/12/01 17:11:45  cg
 * removed statustype entries ST_ref and ST_deref
 * added statustype entry ST_inout
 *
 * Revision 1.7  1995/11/16  19:42:54  cg
 * NIF macro extended by 4 additional parameters
 *
 * Revision 1.6  1995/11/01  09:39:36  cg
 * new member of enum statustype: ST_was_reference
 *
 * Revision 1.5  1995/10/20  09:27:00  cg
 * added 4 new items to macro NIF
 *
 * Revision 1.4  1995/10/19  10:08:41  cg
 * new entry in enum statustype: ST_used
 *
 * Revision 1.3  1995/10/16  17:57:00  cg
 * added new member 'ST_objinitfun' of enum statustype
 *
 * Revision 1.2  1995/10/06  17:15:46  cg
 * new type nodelist added.
 * some new statuses inserted into statustype.
 *
 * Revision 1.1  1995/09/27  15:13:26  cg
 * Initial revision
 *
 *
 */

/*
 *   This source code file does not contain any tabs any more. This is
 *   important to guarantee its unique appearance with all kinds of editors.
 *   So, please try to keep it this way !
 */

#ifndef _sac_types_h

#define _sac_types_h

#define SHP_SEG_SIZE 16
#define MAX_MASK 7
#define MAX_SONS 6

typedef char id; /* kept for compatibility reasons with old version only */

#define TYP_IF(n, s, p, f) n

typedef enum {
#include "type_info.mac"
} simpletype;

#undef TYP_IF

typedef enum { A_let, A_sel, A_for, A_ret } assigntype;

typedef enum { E_int, E_float, E_bool, E_prf, E_id, E_ap, E_with, E_sel } exprtype;

typedef enum { L_for, L_do, L_while } looptype;

typedef enum { ARG_int, ARG_float, ARG_id } argtype;

typedef enum { C_gen, C_mod } contype;

typedef enum {
    ST_regular,            /* normal types */
    ST_unique,             /* unique types */
    ST_reference,          /* reference parameter (unique)           */
    ST_was_reference,      /* for eliminated reference parameter     */
    ST_readonly_reference, /* readonly reference param (unique)  */
    ST_inout,              /* for compilation of reference params    */
    ST_artificial,         /* unique type inserted during            */
                           /* signature expansion                    */
    ST_independent,        /* dimension-independent function         */
    ST_generic,            /* generic function derived from          */
                           /* dimension-independent function         */
    ST_resolved,           /* objects from called function are       */
                           /* analysed.                              */
    ST_unresolved,         /* objects from called function are not   */
                           /* yet analysed.                          */
    ST_local,              /* identifier is local                    */
    ST_global,             /* identifier is global object            */
    ST_imported,           /* function, type, or object imported     */
                           /* from other module                      */
    ST_used,               /* var declaration is used in body        */
    ST_objinitfun          /* function is automatically generated    */
                           /* to contain global object init expr     */

} statustype;

typedef enum { VECT, IDX } useflag;

typedef enum {
    F_prog,
    F_modimp,
    F_classimp,
    F_moddec,
    F_extmoddec,
    F_classdec,
    F_extclassdec,
    F_sib
} file_type;

typedef struct STRINGS {
    char *name;
    struct STRINGS *next;
} strings;

typedef struct NUMS {
    int num;
    struct NUMS *next;
} nums;

typedef struct NODELIST {
    struct NODE *node;
    statustype attrib;
    statustype status;
    struct NODELIST *next;
} nodelist;

typedef struct IDS {
    char *id;
    char *mod;
    int refcnt;
    int flag;          /* the flag is used for ids-status */
                       /* (loop invariant/not loop invariant , ...) */
    struct NODE *node; /* ptr. to declaration */
    struct NODE *def;  /* ptr. to definition(s) resp. usage(s) */
    struct NODE *use;  /* ptr. to usage chain (used only if the var */
                       /* is a one dimensional array! */
    statustype attrib; /* ref/deref attribute */
    statustype status; /* regular or artificial */
    struct IDS *next;
} ids;

typedef struct SHPSEG {
    int shp[SHP_SEG_SIZE];
    struct SHPSEG *next;
} shpseg;

typedef struct TYPES {
    simpletype simpletype;
    char *name;     /* only used for T_user !! */
    char *name_mod; /* name of modul belonging to 'name' */
    int dim;        /* if (dim == 0) => simpletype */
    shpseg *shpseg;
    struct TYPES *next; /* only needed for fun-results  */
                        /* and implementation of implicit types */
    char *id;           /* identifier of function, object, ...  */
    char *id_mod;       /* module where id is defined */
    statustype attrib;  /* uniqueness attribute */
    statustype status;  /* regular or artificial */
} types;

typedef types shapes; /* this definition is primarily needed for
                       * the vinfo nodes; there we need the shape
                       * only( including the dim)...
                       */

typedef struct FUN_NAME {
    char *id;     /* name of function */
    char *id_mod; /* name of modul belonging to 'id' */
} fun_name;

/*
 * Neue Knoten fu"r yacc und den Syntaxbaum
 *
 */

#define NIF(n, s, i, f, p, t, o, x, y, z, a, b, c, d, e, g, h, j, k, l, m, aa, ab, ac,   \
            ad, ae, af, ag, ah, ai, aj, ak, al, am)                                      \
    n

typedef enum {
#include "node_info.mac"
} nodetype; /* Typ fu"r die node Knoten des Syntaxbaums */

#undef NIF

#define PRF_IF(n, s, x, y) n

typedef enum {
#include "prf_node_info.mac"
} prf;

#undef PRF_IF

typedef struct {
    int tag; /* tag for return type */
    int tc;  /* type class */
} prf_tag;

typedef union {
    ids *ids;          /* list  of identifiers               */
    char *id;          /* identifier                         */
    types *types;      /* type information                   */
    int cint;          /* integer value                      */
    float cfloat;      /* float value                        */
    double cdbl;       /* double value                       */
    prf prf;           /* tag for primitive functions        */
    fun_name fun_name; /* used in N_ap nodes                 */
    useflag use;       /* used in N_vect_info nodes          */
    prf_tag prf_dec;   /* used for declaration of primitive functions
                        * this declarations are used to look for argument
                        * and result type of primitive functions
                        */
} infotype;

/*
 *   The node structure of the SAC syntax tree
 */

typedef struct NODE {
    nodetype nodetype;           /* type of node */
    infotype info;               /* node dependent information */
    int refcnt;                  /* reference count information */
    int flag;                    /* the flag is used for node-status */
                                 /* (loop invariant/not loop invariant,...) */
    int varno;                   /* number of variables - 1 */
    long *mask[MAX_MASK];        /* special information about variables */
                                 /* mainly used for optimizations       */
    int nnode;                   /* number of used child nodes */
                                 /* the traversal mechanism depends on it */
    int lineno;                  /* line number in source code */
    struct NODE *node[MAX_SONS]; /* pointers to child nodes */
} node;

#endif /* _sac_types_h */
