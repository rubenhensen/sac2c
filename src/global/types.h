/*
 *
 * $Log$
 * Revision 1.29  1998/04/02 12:49:18  dkr
 * added ST_WLfun
 *
 * Revision 1.28  1998/04/01 08:01:03  srs
 * removed NEWTREE
 *
 * Revision 1.27  1998/03/26 18:05:10  srs
 * modified struct WL_INFO
 *
 * Revision 1.26  1998/03/17 11:23:11  srs
 * added comments to WL_INFO
 *
 * Revision 1.25  1998/03/06 13:28:35  srs
 * added new type WL_INFO
 *
 * Revision 1.24  1998/02/25 09:13:12  cg
 * added type compiler_phase_t
 *
 * Revision 1.23  1997/11/18 18:05:04  srs
 * expanded infotype by GeneratorRel
 * new enum WithIdType
 *
 * Revision 1.22  1997/11/13 16:25:14  srs
 * added enum WithOpType
 * removed struct with_ids
 * added new component info2 (universal infoslot)
 *  to struct NODE
 *
 * Revision 1.21  1997/11/07 15:43:23  srs
 * added alternative with_ids to infotype.
 * with_ids is used in N_withid
 *
 * Revision 1.20  1997/11/05 09:40:49  dkr
 * usage of NIF-macro has changed
 *
 * Revision 1.19  1997/10/29 17:16:02  dkr
 * with defined NEWTREE the component node.nnode does not longer exist
 *
 * Revision 1.18  1997/10/07 16:04:41  srs
 * removed some unused typedefs
 *
 * Revision 1.17  1997/05/14 08:16:43  sbs
 * node->counter inserted.
 *
 * Revision 1.16  1997/03/19  13:39:07  cg
 * Added new data type 'deps'
 *
 * Revision 1.15  1996/01/26  15:29:22  cg
 * added statustype entry ST_classfun
 *
 * Revision 1.14  1996/01/25  16:35:06  hw
 * added macros to use with types->dim to examine whether
 * it is a scalar, an array with known dimension and unknown shape,
 * an array with known shape or an array with unknown shape
 *
 * Revision 1.13  1996/01/16  16:45:45  cg
 * extended macro TYP_IF to 5 positions
 *
 * Revision 1.12  1996/01/02  15:52:26  cg
 * macro NIF extended
 *
 * Revision 1.11  1995/12/29  10:28:16  cg
 * added entries tdef and id_cmod in struct types
 *
 * Revision 1.10  1995/12/20  08:17:37  cg
 * added new entry cchar in info union to store char values.
 *
 * Revision 1.9  1995/12/11  14:01:09  cg
 * added statustype entry ST_Cfun
 *
 * Revision 1.8  1995/12/01  17:11:45  cg
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

#define TYP_IF(n, s, p, f, sz) n

typedef enum {
#include "type_info.mac"
} simpletype;

#undef TYP_IF

#if 0 /* 10.02.97 unused types */
typedef enum { 
               A_let, A_sel, A_for, A_ret
             } 
             assigntype;

typedef enum {
               E_int, E_float, E_bool, E_prf,
               E_id, E_ap, E_with, E_sel
             }
             exprtype;

typedef enum {
               L_for, L_do, L_while
             }
             looptype;
#endif

typedef enum { ARG_int, ARG_float, ARG_id } argtype;

typedef enum {
    /* operation of withloop (N_Nwithop) */
    WO_genarray,
    WO_modarray,
    WO_foldfun,
    WO_foldprf
} WithOpType;

typedef enum {
    /* type of Ids within generator parts of a withloops (N_Nwithid).
     * WI_vector : e.g. [0,0] <= i < [5,5]
     * WI_scalars: e.g. [0,0] <= [i,j] <= [5,5]
     */
    WI_vector,
    WI_scalars
} WithIdType;

/*
 * type for representing compiler phases.
 * NOTE, that the sequence in this type definition is important because
 *  - the compiler phase is simply incremented in main.c
 *  - the compiler phase is used as an index into an array of phase
 *    descriptions defined in Error.c
 *
 * PH_initial and PH_final are required to be the first and last phase,
 * respectively.
 */

typedef enum {
    PH_initial = 0,
    PH_setup,
    PH_scanparse,
    PH_import,
    PH_readsib,
    PH_objinit,
    PH_flatten,
    PH_typecheck,
    PH_checkdec,
    PH_impltype,
    PH_analysis,
    PH_writesib,
    PH_objects,
    PH_uniquecheck,
    PH_rmvoidfun,
    PH_sacopt,
    PH_psiopt,
    PH_refcnt,
    PH_precompile,
    PH_compile,
    PH_genccode,
    PH_invokecc,
    PH_createlib,
    PH_writedeps,
    PH_final
} compiler_phase_t;

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
    ST_global,             /* identifier is global object            */
    ST_imported,           /* function, type, or object imported     */
                           /* from other module                      */
    ST_Cfun,               /* function implemented in C              */
    ST_used,               /* var declaration is used in body        */
    ST_objinitfun,         /* function is automatically generated    */
                           /* to contain global object init expr     */
    ST_classfun,           /* generic function for class conversion  */
    ST_sac,                /* SAC module/class                       */
    ST_external,           /* external module/class                  */
    ST_system,             /* external system library                */
    ST_own                 /* own declaration of module impl.        */
      ST_WLfun             /* function with parallelized with-loop   */
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

/*
 * new nodes for yacc and the syntax tree
 *
 */

#define NIF(n, s, i, f, p, t, o, x, y, z, a, b, c, d, e, g, h, j, k, l, m, aa, ab, ac,   \
            ad, ae, af, ag, ah, ai, aj, ak, al, am, an, ap, aq, ar, nn)                  \
    n

typedef enum {
#include "node_info.mac"
} nodetype; /* Type of nodes of syntax tree */

#undef NIF

#define PRF_IF(n, s, x, y) n

typedef enum {
#include "prf_node_info.mac"
} prf;

#undef PRF_IF

/*
 * structs
 */

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

typedef struct DEPS {
    char *name;
    char *decname;
    char *libname;
    statustype status;
    struct DEPS *sub;
    struct DEPS *next;
} deps;

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
    char *name;        /* only used for T_user !! */
    char *name_mod;    /* name of modul belonging to 'name' */
    struct NODE *tdef; /* typedef of user-defined type */
    int dim;           /* if (dim == 0) => simpletype */
    shpseg *shpseg;
    struct TYPES *next; /* only needed for fun-results  */
                        /* and implementation of implicit types */
    char *id;           /* identifier of function, object, ...  */
    char *id_mod;       /* SAC module where id is defined */
    char *id_cmod;      /* C module where id is defined */
    statustype attrib;  /* uniqueness attribute */
    statustype status;  /* regular or artificial */
} types;

typedef struct {
    prf op1; /* <= or < */
    prf op2; /* <= or < */
} GeneratorRel;

/* and now some macros for the use with 'types->dim'
 * They are used to classify :
 * -shape of type is known:             dim > SCALAR
 * -dimension is only known:            dim < KNOWN_DIM_OFFSET
 * -shape and dimension are not known:  dim == UNKNOWN_SHAPE
 * - type of on array or a scalar:      dim == ARRAY_OR_SCALAR
 * - type of an scalar:                 dim == SCALAR
 */
#define SCALAR 0
#define UNKNOWN_SHAPE -1
#define KNOWN_DIM_OFFSET -2
#define ARRAY_OR_SCALAR -2

typedef types shapes; /* this definition is primarily needed for
                       * the vinfo nodes; there we need the shape
                       * only( including the dim)...
                       */

typedef struct WL_INFO {
    int referenced;      /* number of references in function */
    int referenced_fold; /* number of foldable references */
    int parts;           /* number of N_part nodes */
    int complex;         /* indicator of fold complexity */
    int foldable;        /* has constant generator */
    int no_chance;       /* 1 if WL is defined within loop/cond */
} wl_info;

typedef struct FUN_NAME {
    char *id;     /* name of function */
    char *id_mod; /* name of modul belonging to 'id' */
} fun_name;

typedef struct {
    int tag; /* tag for return type */
    int tc;  /* type class */
} prf_tag;

typedef union {
    ids *ids;            /* list  of identifiers               */
    char *id;            /* identifier                         */
    types *types;        /* type information                   */
    int cint;            /* integer value                      */
    float cfloat;        /* float value                        */
    double cdbl;         /* double value                       */
    char cchar;          /* char value                         */
    prf prf;             /* tag for primitive functions        */
    fun_name fun_name;   /* used in N_ap nodes                 */
    useflag use;         /* used in N_vect_info nodes          */
    prf_tag prf_dec;     /* used for declaration of primitive functions
                          * this declarations are used to look for argument
                          * and result type of primitive functions
                          */
    WithOpType withop;   /* used in N_Nwith node */
    WithIdType withid;   /* used in N_Nwithid node */
    GeneratorRel genrel; /* used in N_Ngenerator node */
} infotype;

/*
 *   The node structure of the SAC syntax tree
 */

typedef struct NODE {
    nodetype nodetype;           /* type of node */
    infotype info;               /* node dependent information */
    void *info2;                 /* any node dependent information */
    int refcnt;                  /* reference count information */
    int flag;                    /* the flag is used for node-status */
                                 /* (loop invariant/not loop invariant,...) */
    int counter;                 /* needed for the enumeration of fundefs!  */
    int varno;                   /* number of variables - 1 */
    long *mask[MAX_MASK];        /* special information about variables */
                                 /* mainly used for optimizations       */
    int lineno;                  /* line number in source code */
    struct NODE *node[MAX_SONS]; /* pointers to child nodes */
} node;

#endif /* _sac_types_h */
