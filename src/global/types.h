/*
 *
 * $Log$
 * Revision 3.22  2003/09/16 16:09:51  sbs
 * spec_mode_t added.
 *
 * Revision 3.21  2003/04/04 17:00:09  sbs
 * prf_node_info.mac extended
 *
 * Revision 3.20  2002/09/11 23:16:09  dkr
 * prf_node_info.mac modified
 *
 * Revision 3.19  2002/09/06 16:18:45  sbs
 * TYPES_POLY for type vars added.
 *
 * Revision 3.18  2002/07/29 12:12:53  sbs
 * PRF_IF macro extended by z.
 *
 * Revision 3.17  2002/07/12 18:38:30  dkr
 * CT_prf removed (okay, that was a bullshit idea ... #$%@&)
 *
 * Revision 3.16  2002/07/08 20:14:00  dkr
 * CT_prf added
 *
 * Revision 3.15  2002/07/03 17:12:29  dkr
 * unqconv_t added again ...
 *
 * Revision 3.14  2002/07/03 16:55:51  dkr
 * unqconv_t removed for TAGGED_ARRAYS
 *
 * Revision 3.13  2002/04/12 13:57:47  sbs
 * info3 added as N_fundef was too crowded for a pointer to n-type to be added.
 *
 * Revision 3.12  2002/03/01 02:33:36  dkr
 * types argtag_t and argtab_t added
 * type clsconv_t renamed into unqconv_t
 *
 * Revision 3.11  2002/02/22 12:55:08  dkr
 * argtab_info.mac deactivated
 *
 * Revision 3.10  2002/02/22 11:49:00  dkr
 * TYPES structure modified
 *
 * Revision 3.9  2001/06/28 07:46:51  cg
 * Primitive function psi() renamed to sel().
 *
 * Revision 3.8  2001/04/24 13:27:28  dkr
 * - macro FUNDEF_UNUSED renamed into USED_INACTIVE
 * - macros FALSE, TRUE moved from internal_lib.h to types.h
 *
 * Revision 3.7  2001/04/02 11:07:30  nmw
 * symbol FUN_UNUSED added
 *
 * Revision 3.5  2001/03/16 11:58:44  nmw
 * ssaphit_t enum type added
 *
 * Revision 3.4  2001/03/06 13:19:10  nmw
 * type cmptree_t added
 *
 * Revision 3.3  2001/02/12 17:07:33  nmw
 * avis attribute in ids structure added
 *
 * Revision 3.2  2001/01/24 23:32:08  dkr
 * macro DIM_NO_OFFSET added
 *
 * Revision 3.1  2000/11/20 17:59:39  sacbase
 * new release made
 *
 * Revision 2.37  2000/11/17 16:20:14  sbs
 * DEPS structure extended by location field;
 * location type added.
 *
 * Revision 2.36  2000/09/20 18:20:00  dkr
 * enum type clsconv_t added
 *
 * Revision 2.35  2000/08/17 10:09:32  dkr
 * all the NT stuff is now in a separate modul (NameTuples.[ch])
 *
 * Revision 2.34  2000/07/21 08:22:54  nmw
 * F_modspec filetype added
 *
 * Revision 2.33  2000/07/11 15:38:51  jhs
 * changed DFMfoldmask.name => DFMfoldmask.vardec
 *
 * Revision 2.32  2000/07/10 14:19:49  cg
 * Added new field type_status in types struct as a dedicated status field
 * for the type itself.
 *
 * Revision 2.31  2000/06/30 13:22:43  nmw
 * conditional define for typedef bool added
 *
 * Revision 2.30  2000/06/23 14:01:56  dkr
 * type of some components of struct WL_INFO changed from 'int' into
 * 'bool'
 *
 * Revision 2.29  2000/03/23 14:01:19  jhs
 * Brushing around includes of nt_info.mac.
 * Added DFMfoldmask_t.
 *
 * Revision 2.28  2000/03/16 14:30:02  dkr
 * phase_info.mac used
 *
 * Revision 2.27  2000/03/02 14:08:39  jhs
 * Added statustype_info.mac for statustype and mdb_statustype.
 *
 * Revision 2.26  2000/03/02 13:09:18  jhs
 * Added ST_call_rep. ST_call_st, ST_call_mt.
 *
 * Revision 2.25  2000/02/23 20:24:53  cg
 * Node status ST_imported replaced by ST_imported_mod and
 * ST_imported_class in order to allow distinction between enteties
 * that are imported from a module and those that are imported from a
 * class.
 *
 * Revision 2.24  2000/02/21 17:59:04  jhs
 * Improved spelling.
 *
 * Revision 2.23  2000/02/11 16:20:29  jhs
 * Added St_repfun
 *
 * Revision 2.22  2000/01/21 12:43:18  dkr
 * new statustypes ST_condfun, ST_dofun and ST_whilefun added
 *
 * Revision 2.20  1999/11/18 12:51:57  bs
 * FEATURE_UNKNOWN added.
 *
 * [...]
 *
 */

/*
 *  This source code file does not contain any tabs any more. This is
 *  important to guarantee its unique appearance with all kinds of editors.
 *  So, please try to keep it this way !
 */

#ifndef _sac_types_h
#define _sac_types_h

/*
 * bool values
 */

typedef int bool;

#define FALSE 0
#define TRUE 1

#define SHP_SEG_SIZE 16
#define MAX_MASK 7
#define MAX_SONS 6

/* value for FUNDEF_USED, if reference counting is inactive */
#define USED_INACTIVE -99

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
#define FEATURE_ASEL 32 /* primitive function psi with array return value */
#define FEATURE_MODA 64 /* primitive function modarray */
#define FEATURE_CAT 128 /* primitive function cat */
#define FEATURE_ROT 256 /* primitive function rotate */
#define FEATURE_AARI                                                                     \
    512                      /* primitive arithmetic operation on arrays                 \
                                (not index vectors) */
#define FEATURE_COND 1024    /* conditional containing array accesses */
#define FEATURE_UNKNOWN 2048 /* no special features but offset not iferable */

typedef enum {
#define TYP_IFname(name) name
#include "type_info.mac"
#undef TYP_IFname
} simpletype;

/*
 * usertype is used in the new type representation only!!!
 * The association between usertype and its definition is
 * encapsulated in the module "user_types"
 * (cf. src/typecheck/user_types.[ch]).
 */
typedef int usertype;

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

typedef enum { M_uses_only, M_uses_and_transform } ive_mode;

typedef enum {
#define PH_SELelement(it_element) it_element
#include "phase_info.mac"
#undef PH_SELelement
} compiler_phase_t;

/* use mdb_statustype to get corresponding char* to print etc. */
typedef enum {
#define SELECTelement(it_element) it_element
#include "status_info.mac"
#undef SELECTelement
} statustype;

typedef enum { DOLLAR, VECT, IDX } useflag;

typedef enum {
    F_prog,
    F_modimp,
    F_classimp,
    F_moddec,
    F_extmoddec,
    F_classdec,
    F_extclassdec,
    F_sib,
    F_modspec
} file_type;

typedef enum { CT_normal, CT_ap, CT_array, CT_return, CT_wl } contextflag;

typedef enum { ACL_irregular, ACL_unknown, ACL_offset, ACL_const } accessclass_t;

typedef enum { ADIR_read, ADIR_write } accessdir_t;

typedef enum { NO_UNQCONV, TO_UNQ, FROM_UNQ } unqconv_t;

typedef enum { LOC_usr, LOC_stdlib } locationtype;

typedef enum { CMPT_EQ, CMPT_NEQ, CMPT_UKNWN } cmptree_t;

typedef enum { PHIT_NONE, PHIT_COND, PHIT_DO, PHIT_WHILE } ssaphit_t;

typedef enum { SS_aks, SS_akd, SS_aud } spec_mode_t;

/*
 * new nodes for yacc and the syntax tree
 */

typedef enum {
#define NIFNode_name(Node_name) Node_name
#include "node_info.mac"
#undef NIFNode_name
} nodetype; /* Type of nodes of syntax tree */

typedef enum {
#define PRF_IF(a, b, c, d, e, f, g, h) a
#include "prf_node_info.mac"
#undef PRF_IF
} prf;

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
    locationtype location;
    struct DEPS *sub;
    struct DEPS *next;
} deps;

typedef struct SHPSEG {
    int shp[SHP_SEG_SIZE];
    struct SHPSEG *next;
} shpseg;

typedef struct ACCESS_T {
    struct NODE *array_vardec; /* */
    struct NODE *iv_vardec;    /* index vector */
    accessclass_t accessclass; /* */
    shpseg *offset;            /* */
    accessdir_t direction;     /* 0 == ADIR_read,  1 == ADIR_write */
    struct ACCESS_T *next;     /* */
} access_t;

typedef struct IDS {
    char *id;
    char *mod;
    int refcnt;
    int naive_refcnt;
    int flag;          /* the flag is used for ids-status */
                       /* (loop invariant/not loop invariant , ...) */
    struct NODE *node; /* ptr. to declaration */
    struct NODE *avis; /* ptr. to common used attribs of vardec/arg */
    struct NODE *def;  /* ptr. to definition(s) resp. usage(s) */
    struct NODE *use;  /* ptr. to usage chain (used only if the var */
                       /* is a one dimensional array! */
    statustype attrib; /* ref/deref attribute */
    statustype status; /* regular or artificial */
    struct IDS *next;
} ids;

typedef struct TYPESS {
    simpletype simpletype;
    char *name;             /* only used for T_user !! */
    char *name_mod;         /* name of modul belonging to 'name' */
    struct NODE *tdef;      /* typedef of user-defined type */
    int dim;                /* if (dim == 0) => simpletype */
    bool poly;              /* only needed for type templates (newTC !) */
    shpseg *shpseg;         /* pointer to shape specification */
    statustype type_status; /* regular/artificial/crettype */
    struct TYPESS *next;    /* only needed for fun-results  */
                            /* and implementation of implicit types */
} types;

/*
 *  Used to store the relations of with-generators.
 *  Should be normalized in flatten and typecheck to op1: <= and op2: <
 *  Other values than <= or < are never allowed!
 *
 *  For empty arrays, a special treatment is necessary, so original
 *  Values before normalization are stored also.
 *  The xxx_orig values will be set on creation and are not to be changed
 *  from that on.
 */

typedef struct {
    prf op1;      /* only <= and < allowed, later normalized to <= */
    prf op2;      /* only <= and < allowed, later normalized to <= */
    prf op1_orig; /* only <= and < allowed, not to be changed      */
    prf op2_orig; /* only <= and < allowed, not to be changed      */
} GeneratorRel;

/*
 *  And now some constants (macros) for the use with TYPES_DIM
 *  (resp. 'types->dim').
 *
 *  They are used to classify, whether the type is an array or (not: xor!)
 *  a scalar.
 *  If we know it's a scalar we know shape == [] and dimension == 0.
 *  If it's an array some cases for shape and dimension are possible.
 *
 *  TYPES_DIM can have the following values (and meanings):
 *  - array, shape and dimension are know                 dim > SCALAR
 *  - array, shape is not known, dimension is known       dim < KNOWN_DIM_OFFSET
 *  - array, shape and dimension is not known             dim == UNKNOWN_SHAPE
 *  - array or scalar, shape and dimension are not known  dim == ARRAY_OR_SCALAR
 *  - scalar, shape and dimension are known               dim == SCALAR
 *
 *  The known-macros test for knowledge about the properties of TYPES_DIM.
 *  So one can easily test, wheter shape or dimension are known or not.
 */
#define SCALAR 0
#define UNKNOWN_SHAPE -1
#define ARRAY_OR_SCALAR -2
#define KNOWN_DIM_OFFSET -2

#define KNOWN_SHAPE(x) (((x) == SCALAR) || ((x) > SCALAR))
#define KNOWN_DIMENSION(x) (((x) == SCALAR) || ((x) > SCALAR) || ((x) < KNOWN_DIM_OFFSET))

#define DIM_NO_OFFSET(dim) (((dim) < KNOWN_DIM_OFFSET) ? KNOWN_DIM_OFFSET - (dim) : (dim))

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
    bool foldable;         /* has constant generator */
    bool no_chance;        /* 1 if WL is defined within loop/cond */
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
 *  The node structure of the SAC syntax tree
 */

typedef struct NODE {
    nodetype nodetype;           /* type of node */
    infotype info;               /* node dependent information */
    void *info2;                 /* any node dependent information */
    void *info3;                 /* any node dependent information */
    int refcnt;                  /* reference count information */
    int flag;                    /* the flag is used for node-status        */
                                 /* (loop invariant/not loop invariant,...) */
    int counter;                 /* needed for the enumeration of fundefs! */
    int varno;                   /* number of variables */
    int int_data;                /* additional int-entry */
    long *mask[MAX_MASK];        /* special information about variables */
                                 /* mainly used for optimizations       */
    void *dfmask[MAX_MASK];      /* dataflow masks */
    int lineno;                  /* line number in source code */
    char *src_file;              /* pointer to filename or source code */
    struct NODE *node[MAX_SONS]; /* pointers to child nodes */
} node;

/******************************************************************************
 *
 * the following types are needed for Withloopp Folding
 *
 ******************************************************************************/

/* The following struct is only annotated to N_assign nodes which are
   inside a WL body and which have ASSIGN_INSTRs N_let and N_prf(F_sel). */
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
                         val = sel(...,tmp);
                       Well, can also happen when CF is deacivated.
                       If arg_no is 0, prf is undefined */
} index_info;

/*
 * internal representation of WL generators on which the intersection
 * creation is based.
 */
typedef struct INTERN_GEN {
    int shape;
    int *l, *u;
    int *step, *width;
    node *code;
    struct INTERN_GEN *next;
} intern_gen;

/*
 *  Structure to store information during multithread.
 *  It is needed to store by whcih fold-operation a variable is set.
 */

typedef struct FM {
    node *vardec;
    node *foldop;
    struct FM *next;
} DFMfoldmask_t;

/*
 * used in precompile/compile
 */

typedef enum {
#define SELECTelement(it_element) it_element
#include "argtag_info.mac"
} argtag_t;

typedef struct {
    int size;
    node **ptr_in;  /* N_arg or N_exprs node */
    void **ptr_out; /* 'types*' or 'ids*' */
    argtag_t *tag;
} argtab_t;

#endif /* _sac_types_h */
