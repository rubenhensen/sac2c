/*
 *
 * $Log$
 * Revision 2.3  1999/04/08 17:14:31  jhs
 * Changed value for KNOWN_DIM_OFFSET from -2 to -3.
 * Added EMPTY_ARRAZY with value -2.
 *
 * Revision 2.2  1999/02/28 21:07:06  srs
 * moved types from WithloopFolding.h to this file.
 *
 * Revision 2.1  1999/02/23 12:40:15  sacbase
 * new release made
 *
 * Revision 1.46  1999/02/11 09:18:45  bs
 * contextflag CT_array added.
 *
 * Revision 1.45  1999/02/06 12:50:45  srs
 * expanded type NODELIST
 *
 * Revision 1.44  1999/02/02 18:47:14  dkr
 * definition of type 'feature_t' changed, to fool cc
 *
 * Revision 1.43  1999/01/19 20:32:10  srs
 * added references_folded to struct WL_INFO
 *
 * Revision 1.42  1999/01/15 15:21:24  cg
 * added new types access_t, accessclass_t, and feature_t.
 *
 * Revision 1.41  1998/06/18 13:40:07  cg
 * macro NIF used in node_info.mac enlarged,
 * new traversal function tables added.
 *
 * Revision 1.40  1998/05/28 23:53:44  dkr
 * added ST_foldfun
 *
 * Revision 1.39  1998/05/28 07:48:53  cg
 * added entry ST_spmd_inout for enum statustype.
 *
 * Revision 1.38  1998/05/20 20:14:07  sbs
 * contextflag for INFO_FLTN inserted.
 *
 * Revision 1.37  1998/05/14 12:49:24  sbs
 * contextflag added
 *
 * Revision 1.36  1998/05/06 14:30:07  dkr
 * added DataFlowMasks to NODE type
 *
 * Revision 1.35  1998/04/29 17:10:30  dkr
 * changed usage of NIF
 *
 * Revision 1.34  1998/04/23 18:57:01  dkr
 * added usage of NIF
 *
 * Revision 1.33  1998/04/17 17:26:13  dkr
 * 'concurrent regions' are now called 'SPMD regions'
 *
 * Revision 1.32  1998/04/03 11:28:06  dkr
 * concregs renamed to concregions
 *
 * Revision 1.31  1998/04/02 18:17:38  dkr
 * ST_WLfun renamed to ST_concfun
 *
 * Revision 1.30  1998/04/02 16:06:11  dkr
 * added new compiler phase PH_concregs
 *
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

/*
 * The type faeture_t is used as a bit mask for tile size inference.
 * It stores information about features found within an operator of a
 * with-loop.
 */

typedef /* unsigned */ int feature_t;
/*
 * we better use 'int' here, to fool the cc compiler.
 * (a cast to 'unsigned int' is not a l-value !!)
 */

#define FEATURE_NONE 0  /* no special features at all */
#define FEATURE_WL 1    /* with-loop containing array accesses */
#define FEATURE_LOOP 2  /* while/do/for-loop containing array accesses */
#define FEATURE_TAKE 4  /* primitive function take */
#define FEATURE_DROP 8  /* primitive function drop */
#define FEATURE_AP 16   /* function application */
#define FEATURE_APSI 32 /* primitive function psi with array return value */
#define FEATURE_MODA 64 /* primitive function modarray */
#define FEATURE_CAT 128 /* primitive function cat */
#define FEATURE_ROT 256 /* primitive function rotate */
#define FEATURE_AARI                                                                     \
    512 /* primitive arithmetic operation on arrays (not index vectors) */
#define FEATURE_COND 1024 /* conditional containing array accesses */

#define TYP_IF(n, s, p, f, sz) n

typedef enum {
#include "type_info.mac"
} simpletype;

#undef TYP_IF

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
    PH_wltrans,
    PH_spmdregions,
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
    ST_spmd_inout,         /* for comp. of spmd-fun inout params     */
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
    ST_own,                /* own declaration of module impl.        */
    ST_foldfun,            /* dummy function containing the fold-op  */
    ST_spmdfun             /* function generated from a spmd-region  */
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

typedef enum { CT_normal, CT_ap, CT_array, CT_return, CT_wl } contextflag;

typedef enum { ACL_irregular, ACL_unknown, ACL_offset, ACL_const } accessclass_t;

/*
 * new nodes for yacc and the syntax tree
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
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
    void *attrib2;
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

typedef struct SHPSEG {
    int shp[SHP_SEG_SIZE];
    struct SHPSEG *next;
} shpseg;

typedef struct ACCESS_T {
    struct NODE *array_vardec;
    struct NODE *iv_vardec;
    accessclass_t accessclass;
    shpseg *offset;
    struct ACCESS_T *next;
} access_t;

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
 * - shape of type is known:               dim > SCALAR
 * - dimension is only known:              dim < KNOWN_DIM_OFFSET
 * - shape and dimension are not known:    dim == UNKNOWN_SHAPE
 * - type of on array or a scalar:         dim == ARRAY_OR_SCALAR
 * - type of an scalar:                    dim == SCALAR
 * - empty array, so shape is known as []: dim == EMPTY_ARRAY
 */
#define SCALAR 0
#define UNKNOWN_SHAPE -1
#define ARRAY_OR_SCALAR -2
#define EMPTY_ARRAY -3
#define KNOWN_DIM_OFFSET -3

typedef types shapes; /* this definition is primarily needed for
                       * the vinfo nodes; there we need the shape
                       * only( including the dim)...
                       */

typedef struct WL_INFO {
    int referenced;        /* number of references in function */
    int referenced_fold;   /* number of foldable references */
    int references_folded; /* number of refs eliminated by WLF */
    int parts;             /* number of N_part nodes */
    int complex;           /* indicator of fold complexity */
    int foldable;          /* has constant generator */
    int no_chance;         /* 1 if WL is defined within loop/cond */
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
    int varno;                   /* number of variables */
    long *mask[MAX_MASK];        /* special information about variables */
                                 /* mainly used for optimizations       */
    void *dfmask[MAX_MASK];      /* dataflow masks */
    int lineno;                  /* line number in source code */
    struct NODE *node[MAX_SONS]; /* pointers to child nodes */
} node;

/******************************************************************************
 *
 * the following types are needed for Withloopp Folding
 *
 ******************************************************************************/

/* The following struct is only annotated to N_assign nodes which are
   inside a WL body and which have ASSIGN_INSTRs N_let and N_prf(F_psi). */
typedef struct INDEX_INFO {
    int vector;               /* this is an index vector (>0) or a scalar (0)
                                 in case of a vector this number is the
                                 shape of the vector. */
    int *permutation;         /* Describes the permutation of index vars in
                                 this vector (if this is a vector) or stores
                                 the base scalar index in permutation[0].
                                 The index scarales are counted starting
                                 with 1. E.g. in [i,j,k] j has the no 2.
                                 If one elements is not based on an index
                                 (ONLY a constant, else the vector is not
                                 valid) this value is 0 and the constant
                                 can be found in const_arg. */
    struct INDEX_INFO **last; /* Vector of predecessores (in case of vector)
                                 or one predecessor (last[0] in case of scalar).
                                 Point to last transformations */

    /* the next 3 components describe the executed transformation */
    prf prf;        /* prf +,-,* or / */
    int *const_arg; /* the constant arg has to be an integer.
                       For every element of a vector there is an
                       own constant arg.
                       If this struct is an annotation for a scalar,
                       only const_arg[0] is valid.
                       If the corresponding permutation is 0, the
                       vector's element is a constant which is
                       stored here (not a prf arg). */
    int arg_no;     /* const_arg is the first (1) or second (2)
                       argument of prf. arg_no may be 0 which
                       means that no prf is given. Can only be in:
                        tmp = [i,j,c];
                        val = psi(...,tmp);
                       Well, can also happen when CF is deacivated.
                       If arg_no is 0, prf is undefined */
} index_info;

/* internal representation of WL generators on which the intersection
   creation is based. */
typedef struct INTERN_GEN {
    int shape;
    int *l, *u;
    int *step, *width;
    node *code;
    struct INTERN_GEN *next;
} intern_gen;

#endif /* _sac_types_h */
