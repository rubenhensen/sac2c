/*
 *
 * $Log$
 * Revision 1.7  2004/11/23 20:00:49  cg
 * *** empty log message ***
 *
 * Revision 1.6  2004/11/23 15:52:58  cg
 * *** empty log message ***
 *
 * Revision 1.5  2004/11/23 15:07:30  cg
 * Macro renamed.
 *
 * Revision 1.4  2004/11/23 13:39:59  ktr
 * adjusted structure identifiers according to the CODING GUIDELINES
 *
 * Revision 1.3  2004/11/23 13:05:43  ktr
 * removed funtab\
 *
 * Revision 1.2  2004/11/23 11:48:19  cg
 * added comments.
 *
 * Revision 1.1  2004/11/23 11:45:25  cg
 * Initial revision
 */

/*
 * File moved from src/gobal to src/types.
 *
 *
 * Revision 3.62  2004/11/23 11:36:42  cg
 * Switched mac-file based declaration of global variables.
 *
 * Revision 3.61  2004/11/23 11:21:53  sbs
 * node_info.mac excluded
 *
 * Revision 3.60  2004/11/23 09:38:55  cg
 * Fixed various typos.
 * Remove DFMfoldmask_t.
 *
 * Revision 3.59  2004/11/22 21:07:39  skt
 * moved 'typedef struct SERSTACK_T serstack_t' from serialize_stack.h
 *
 * Revision 3.58  2004/11/22 21:05:30  ktr
 * ISMOP
 *
 * Revision 3.57  2004/11/22 19:09:54  ktr
 * SACDECCAMP 04
 *
 * Revision 3.56  2004/11/22 18:50:07  ktr
 * typedefs moved from LookUpTables.h SACDevCamp 04
 *
 * Revision 3.55  2004/11/22 18:48:41  sah
 * *<8-D
 *
 * Revision 3.54  2004/11/22 18:08:33  sbs
 * stuff from SSAConstantFolding.h added.
 *
 * Revision 3.53  2004/11/22 18:03:33  cg
 * Added optimize_t and global_t.
 *
 * Revision 3.52  2004/11/22 16:44:40  ktr
 * typedefs moved from symboltable SacDevCamp 04
 *
 * Revision 3.51  2004/11/22 16:31:12  ktr
 * typedefss moved from modulemanager SacDevCamp04
 *
 * Revision 3.50  2004/11/22 16:26:45  sbs
 * imported from pad_info.h
 *
 * Revision 3.49  2004/11/22 16:23:58  ktr
 * moved typedefs from libmanager.c
 *
 * Revision 3.48  2004/11/22 16:18:29  ktr
 * ISMOP SacDevCamp 04
 *
 * Revision 3.47  2004/11/22 14:48:43  ktr
 * deleted all IDS-Structure SacDevCamp 04
 *
 * Revision 3.46  2004/11/22 14:42:16  sbs
 * moved from type_errors.h
 *
 * Revision 3.45  2004/11/22 14:39:10  sbs
 * stuff from ssi.h
 *
 * Revision 3.44  2004/11/22 14:37:51  ktr
 * Ismop SacDevCamp 04
 *
 * Revision 3.43  2004/11/22 14:35:06  sbs
 * types from sig_deps.h added.
 *
 * Revision 3.42  2004/11/22 14:19:00  sbs
 * type_conversion_flag added
 *
 * Revision 3.41  2004/11/22 14:15:46  sbs
 * ct_res added
 *
 * Revision 3.40  2004/11/22 14:12:51  sbs
 * renamed DFT_res into dft_res (thanks Kai!)
 *
 * Revision 3.39  2004/11/22 14:09:22  sbs
 * some further stuff from new_types.h
 *
 * Revision 3.38  2004/11/22 14:05:54  sbs
 * stuff from new_types.h added
 *
 * Revision 3.37  2004/11/22 14:01:39  ktr
 * Ismop SacDevCamp 04
 *
 * Revision 3.36  2004/11/21 21:30:33  ktr
 * moved some typedefs from NameTuples.h to types.h
 *
 * Revision 3.35  2004/10/28 17:16:55  sah
 * added stringset_t
 *
 * Revision 3.34  2004/10/20 08:04:09  khf
 * modified wl_info
 *
 * Revision 3.33  2004/09/30 19:53:49  sah
 * made rc_counter visible
 *
 * Revision 3.32  2004/08/26 17:03:54  skt
 * value MUTH_MULTI_SPECIALIZED added to mtexecmode_t
 *
 * Revision 3.31  2004/08/26 15:02:08  khf
 * extend wl_info for WithloopFusion
 *
 * Revision 3.30  2004/08/18 11:50:08  skt
 * added enumeration-type mtexecmode_t
 *
 * Revision 3.29  2004/07/29 15:05:03  sah
 * added access_info_t structure to annotate acces information
 * to the syntax tree (instead of using a N_info node). This
 * fixes the bug introduced by the new INFO structure.
 *
 * Revision 3.28  2004/07/16 14:41:34  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 3.27  2004/07/11 17:54:01  sah
 * added F_unknown filetype
 *
 * Revision 3.26  2004/07/11 17:41:39  sah
 * added WO_unknown WithOp to model the unset state of
 * NWithOp node
 *
 * Revision 3.25  2004/07/03 15:09:35  sah
 * added the new node representation into the source code.
 * the new ast can be enabled by make newast int the
 * project root
 *
 * Revision 3.24  2004/03/10 00:10:17  dkrHH
 * old backend removed
 *
 * Revision 3.23  2004/02/05 10:37:14  cg
 * Re-factorized handling of different modes in multithreaded code
 * generation:
 * - Added enumeration type for representation of modes
 * - Renamed mode identifiers to more descriptive names.
 *
 * Revision 3.22  2003/09/16 16:09:51  sbs
 * spec_mode_t added.
 *
 * Revision 3.21  2003/04/04 17:00:09  sbs
 * prf_node_info.mac extended
 *
 *
 * [...]
 *
 */

/*
 *  This source code file does not contain any tabs any more. This is
 *  important to guarantee its unique appearance with all kinds of editors.
 *  So, please try to keep it this way !
 */

#ifndef _SAC_TYPES_H_
#define _SAC_TYPES_H_

#include <stdio.h>

#include "config.h"
#include "types_nodetype.h"
#include "types_trav.h"

/*
 * bool values
 */

typedef int bool;

#define FALSE 0
#define TRUE 1

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
#define FEATURE_UNKNOWN 2048 /* no special features but offset not inferable */

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
    /* type of Ids within generator parts of a withloops (N_Nwithid).
     * WI_vector : e.g. [0,0] <= i < [5,5]
     * WI_scalars: e.g. [0,0] <= [i,j] <= [5,5]
     */
    WI_vector,
    WI_scalars
} withid_t;

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
    F_modspec,
    F_unknown
} file_type;

typedef enum { CT_normal, CT_ap, CT_array, CT_return, CT_wl } contextflag;

typedef enum { ACL_irregular, ACL_unknown, ACL_offset, ACL_const } accessclass_t;

typedef enum { ADIR_read, ADIR_write } accessdir_t;

typedef enum { NO_UNQCONV, TO_UNQ, FROM_UNQ } unqconv_t;

typedef enum { LOC_usr, LOC_stdlib } locationtype;

typedef enum { CMPT_EQ, CMPT_NEQ, CMPT_UKNWN } cmptree_t;

typedef enum { PHIT_NONE, PHIT_COND, PHIT_DO, PHIT_WHILE } ssaphit_t;

typedef enum { SS_aks, SS_akd, SS_aud } spec_mode_t;

typedef enum { MT_none = 0, MT_createjoin, MT_startstop, MT_mtstblock } mtmode_t;

/* the possible executiomodes of mtmode 3 (mtstblock) */
typedef enum {
    MUTH_ANY,
    MUTH_EXCLUSIVE,
    MUTH_SINGLE,
    MUTH_MULTI,
    MUTH_MULTI_SPECIALIZED
} mtexecmode_t;

/* string sets, see stringset.h */
typedef struct STRINGSET_T stringset_t;

typedef enum {
#define PRF_IF(a, b, c, d, e, f, g, h) a
#include "prf_node_info.mac"
#undef PRF_IF
} prf;

/*
 * structs
 */

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
    struct node *array_vardec; /* */
    struct node *iv_vardec;    /* index vector */
    accessclass_t accessclass; /* */
    shpseg *offset;            /* */
    accessdir_t direction;     /* 0 == ADIR_read,  1 == ADIR_write */
    struct ACCESS_T *next;     /* */
} access_t;

/*
 * This structure is used by wl_access_analyse to anotate
 * some information to N_CODE nodes. See NCODE_WLAA macros
 */
typedef struct ACCESS_INFO_T {
    access_t *access;
    int accesscnt;
    int feature;
    struct node *indexvar;
    struct node *wlarray;
} access_info_t;

typedef struct TYPES {
    simpletype simpletype;
    char *name;             /* only used for T_user !! */
    char *name_mod;         /* name of modul belonging to 'name' */
    struct node *tdef;      /* typedef of user-defined type */
    int dim;                /* if (dim == 0) => simpletype */
    bool poly;              /* only needed for type templates (newTC !) */
    shpseg *shpseg;         /* pointer to shape specification */
    statustype type_status; /* regular/artificial/crettype */
    struct TYPES *next;     /* only needed for fun-results  */
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

typedef struct GENERATOR_REL {
    prf op1;      /* only <= and < allowed, later normalized to <= */
    prf op2;      /* only <= and < allowed, later normalized to <= */
    prf op1_orig; /* only <= and < allowed, not to be changed      */
    prf op2_orig; /* only <= and < allowed, not to be changed      */
} generator_rel;

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
    int referenced;         /* number of references in function */
    int referenced_fold;    /* number of foldable references */
    int references_folded;  /* number of refs eliminated by WLF */
    int parts;              /* number of N_part nodes */
    int complex;            /* indicator of fold complexity */
    bool foldable;          /* has constant generator */
    bool no_chance;         /* 1 if WL is defined within loop/cond */
    bool dependent;         /* depends on fusionable withloop */
    bool mto_offset_needed; /* more than one offset needed */
} wl_info;

typedef struct FUN_NAME {
    char *id;     /* name of function */
    char *id_mod; /* name of modul belonging to 'id' */
} fun_name;

typedef struct {
    int tag; /* tag for return type */
    int tc;  /* type class */
} prf_tag;

typedef union INFOTYPE {
    struct node *ids;     /* list  of identifiers               */
    char *id;             /* identifier                         */
    types *types;         /* type information                   */
    int cint;             /* integer value                      */
    float cfloat;         /* float value                        */
    double cdbl;          /* double value                       */
    char cchar;           /* char value                         */
    prf prf;              /* tag for primitive functions        */
    fun_name fun_name;    /* used in N_ap nodes                 */
    useflag use;          /* used in N_vect_info nodes          */
    prf_tag prf_dec;      /* used for declaration of primitive functions
                           * this declarations are used to look for argument
                           * and result type of primitive functions
                           */
    withid_t withid;      /* used in N_Nwithid node */
    generator_rel genrel; /* used in N_Ngenerator node */
} infotype;

#ifdef NEW_AST
/*
 * The NEW node structure of the SAC syntax tree
 * The type is abstract, as there is _no_ way to access a node other
 * than using tree_basic.h. Thus the structure is defined in
 * tree_basic.h. This as well solves dependency problems.
 */
typedef struct NODE node;

#else
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
    struct node *node[MAX_SONS]; /* pointers to child nodes */
} node;
#endif /* NEW_AST */

/*****************************************************************************
 * The info structure is used during traversal to store some stateful
 * information. It replaces the old N_info node. The structure is defined
 * as an abstract type here, so it can be definied by the different
 * traversals to suit the local needs. To do so, define a structure INFO
 * within the .c file of your traversal or create a specific .h/.c file
 * included by all .c files of your traversal. You as well have to create
 * a static MakeInfo/FreeInfo function.
 *****************************************************************************/

typedef struct INFO info;

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
 * used in precompile/compile
 */

typedef enum {
#define SELECTelement(it_element) it_element
#include "argtag_info.mac"
} argtag_t;

typedef struct ARGTAB_T {
    int size;
    node **ptr_in;  /* N_arg or N_exprs node */
    void **ptr_out; /* 'types*' or 'ids*' */
    argtag_t *tag;
} argtab_t;

/*
 * used in emm
 */

typedef struct RC_COUNTER rc_counter;

/*
 * The following defines indicate the position of tags within name tuples.
 * They should be kept in synch with the NT_NAME, NT_SHP, NT_HID and NT_UNQ
 * macros in sac_std.h
 */
#define NT_NAME_INDEX 0
#define NT_SHAPE_INDEX 1
#define NT_HIDDEN_INDEX 2
#define NT_UNIQUE_INDEX 3

/*
 * Enumerated types for data class and uniqueness class
 */

typedef enum {
#define ATTRIB NT_SHAPE_INDEX
#define NTIFtype(it_type) it_type
#include "nt_info.mac"
#undef NTIFtype
#undef ATTRIB
} shape_class_t;

typedef enum {
#define ATTRIB NT_HIDDEN_INDEX
#define NTIFtype(it_type) it_type
#include "nt_info.mac"
#undef NTIFtype
#undef ATTRIB
} hidden_class_t;

typedef enum {
#define ATTRIB NT_UNIQUE_INDEX
#define NTIFtype(it_type) it_type
#include "nt_info.mac"
#undef NTIFtype
#undef ATTRIB
} unique_class_t;

/*
 * moved from shape.h
 */

typedef struct SHAPE shape;

/*
 * moved from constant.h
 */

typedef struct CONSTANT constant;
typedef constant *(*monCF) (constant *);
typedef constant *(*binCF) (constant *, constant *);
typedef constant *(*triCF) (constant *, constant *, constant *);

/*
 * moved from basecv.h
 */

typedef constant *(*basecvfunptr) (shape *shp);

/*
 * moved from cv2cv.h
 */

typedef void (*cv2cvfunptr) (void *, int, int, void *, int);

/*
 * moved from cv2scalar.h
 */

typedef node *(*cv2scalarfunptr) (void *, int);

/*
 * moved from cv2str.h
 */

typedef char *(*cv2strfunptr) (void *, int, int, int);

/*
 * moved from zipcv.h
 */

typedef void (*zipcvfunptr) (void *, int, void *, int, void *, int);

/*
 * moved from internal_lib.h
 */
typedef struct STR_BUF str_buf;
typedef struct PTR_BUF ptr_buf;

/*******************************************************************************
 *
 * moved from new_types.h
 */

typedef struct NTYPE ntype;

/*
 * basic stuff which should only be used if essential from a performance
 * point of view, as it unleashes part of the implementation...
 */

typedef enum {
#define TCITypeConstr(a) a
#include "type_constructor_info.mac"
#undef TCITypeConstr
} typeconstr;

typedef struct DFT {
    ntype *type;
    node *def;
    node *deriveable;
    int num_partials;
    node **partials;
    int num_deriveable_partials;
    node **deriveable_partials;
} dft_res;

/*
 * Type-Comparison
 */
typedef enum {
    TY_eq,  /* types are identical */
    TY_lt,  /* first type is subtype of second one */
    TY_gt,  /* first type is supertype of second one */
    TY_hcs, /* types are unrelated but do have a common supertype */
    TY_dis  /* types are disjoint */
} ct_res;

/*
 * Functions for converting from old to new types and back
 */
typedef enum { TY_symb, TY_user } type_conversion_flag;

/*******************************************************************************
 *
 * moved from type_errors.h
 */

typedef struct TE_INFO te_info;

/*******************************************************************************
 *
 * moved from sig_deps.h
 */

typedef ntype *(*ct_funptr) (te_info *, ntype *);
typedef struct SIG_DEP sig_dep;

/*
 * moved from resource.h
 */

/*****************************************************************************
 *
 * type: resource_list_t
 * type: target_list_t
 *
 * description:
 *
 *  These types are used to build up a tree-like structure for temporaily
 *  storing all information read in from sac2crc files.
 *
 ******************************************************************************/

typedef struct RESOURCE_LIST_T {
    char *name;
    char *value_str;
    int value_num;
    int add_flag;
    struct RESOURCE_LIST_T *next;
} resource_list_t;

typedef struct TARGET_LIST_T {
    char *name;
    node *super_targets;
    resource_list_t *resource_list;
    struct TARGET_LIST_T *next;
} target_list_t;

/*****************************************************************************
 *
 * type: resource_t
 *
 * description:
 *
 *  This structure is used to permanently store all relevant resource
 *  information for the selected target.
 *
 ******************************************************************************/

typedef struct {
    char *cc;
    char *ccflags;
    char *ccdir;
    char *ldflags;
    char *cclink;
    char *ccmtlink;
    char *opt_O0;
    char *opt_O1;
    char *opt_O2;
    char *opt_O3;
    char *opt_g;
    char *opt_D;
    char *opt_I;

    char *cpp_stdin;
    char *cpp_file;
    char *tar_create;
    char *tar_extract;
    char *ar_create;
    char *ld_dynamic;
    char *ranlib;
    char *mkdir;
    char *rmdir;
    char *chdir;
    char *cat;
    char *move;
    char *rsh;
    char *dump_output;

    char *stdlib_decpath;
    char *stdlib_libpath;
    char *system_libpath;
    char *tmpdir;

    int cache1_size;
    int cache1_line;
    int cache1_assoc;
    char *cache1_writepol;
    int cache1_msca_factor;

    int cache2_size;
    int cache2_line;
    int cache2_assoc;
    char *cache2_writepol;
    int cache2_msca_factor;

    int cache3_size;
    int cache3_line;
    int cache3_assoc;
    char *cache3_writepol;
    int cache3_msca_factor;
} configuration_t;

/*******************************************************************************
 *
 * moved from ssi.h
 */

typedef struct TVAR tvar;
typedef bool (*tvar_ass_handle_fun) (sig_dep *handle);

/*
 * moved from stringset.h
 */

typedef enum { SS_saclib, SS_extlib, SS_objfile } SStype_t;
typedef void *(*SSfoldfun_p) (const char *elem, SStype_t kind, void *rest);

/*
 * moved from filemgr.h
 */

typedef enum { PK_path, PK_moddec_path, PK_modimp_path, PK_systemlib_path } pathkind;

/*
 * moved from libmanager.h
 */
typedef void *dynlib_t;
typedef void *dynfun_t;

/*******************************************************************************
 *
 * moved from pad_info.h
 */

/* structure for storing access patterns */
typedef struct PATTERN_T {
    shpseg *pattern;
    struct PATTERN_T *next;
} pattern_t;

/* structure for grouping access patterns by conflict groups */
typedef struct CONFLICT_GROUP_T {
    shpseg *group;
    accessdir_t direction;
    pattern_t *patterns;
    struct CONFLICT_GROUP_T *next;
} conflict_group_t;

/* strcture for grouping conflict groups by array types */
typedef struct ARRAY_TYPE_T {
    simpletype type;
    int dim;
    shpseg *shape;
    conflict_group_t *groups;
    struct ARRAY_TYPE_T *next;
} array_type_t;

/* structure containing shapes of unsupported operations */
typedef struct UNSUPPORTED_SHAPE_T {
    simpletype type;
    int dim;
    shpseg *shape;
    struct UNSUPPORTED_SHAPE_T *next;
} unsupported_shape_t;

/* structure containing old and inferred array shape */
typedef struct PAD_INFO_T {
    simpletype type;
    int dim;
    shpseg *old_shape;
    shpseg *new_shape;
    shpseg *padding;
    node *fundef_pad;
    node *fundef_unpad;
    struct PAD_INFO_T *next;
} pad_info_t;

/*
 * moved from modulemanager.h
 */

typedef struct MODULE_T module_t;
typedef node *(*serfun_p) ();

/*
 * moved from symboltable.h
 */

typedef enum {
    SET_funbody,
    SET_funhead,
    SET_typedef,
    SET_objdef,
    SET_wrapperbody,
    SET_wrapperhead,
    SET_namespace
} stentrytype_t;

typedef enum { SVT_local, SVT_provided, SVT_exported } stvisibility_t;

typedef struct ST_ENTRY_T stentry_t;
typedef struct ST_SYMBOLITERATOR_T stsymboliterator_t;
typedef struct ST_ENTRYITERATOR_T stentryiterator_t;
typedef struct ST_SYMBOLTABLE_T sttable_t;
typedef struct ST_SYMBOL_T stsymbol_t;

/*
 * New types for global
 */

/*
 * Read in optimization flag type from optimize.mac
 */

typedef struct OPTIMIZE_FLAGS_T {
#define OPTabbr(abbr) unsigned int do##abbr : 1;
#include "optimize.mac"
#undef OPTabbr
} optimize_flags_t;

/*
 * Read in configuration flag types from flags.mac
 */

typedef struct TRACE_FLAGS_T {
#define TRACEflag(flag) unsigned int do##flag : 1;
#include "flags.mac"
} trace_flags_t;

typedef struct CACHESIM_FLAGS_T {
#define CSflag(flag) unsigned int do##flag : 1;
#include "flags.mac"
} cachesim_flags_t;

typedef struct RUNTIMECHECK_FLAGS_T {
#define RTCflag(flag) unsigned int do##flag : 1;
#include "flags.mac"
} runtimecheck_flags_t;

/*
 * Read in global variables from globals.mac
 */

typedef struct GLOBAL_T {
#define GLOBALtype(it_type) it_type
#define GLOBALname(it_name) it_name
#define GLOBALdelim ;
#include "globals.mac"
#undef GLOBALdelim
#undef GLOBALname
#undef GLOBALtype
} global_t;

/*******************************************************************************
 * moved from SSAConstantFolding.h:
 *
 *
 * structural constant (SCO) should be integrated in constants.[ch] in future
 */
typedef struct struct_constant struct_constant;

/*******************************************************************************
 * moved from DataFlowMask.h:
 */
typedef struct MASK_BASE_T dfmask_base_t;
typedef struct MASK_T dfmask_t;

/*******************************************************************************
 * moved from DataFlowMaskUtils.h:
 */
typedef struct STACK_T dfmstack_t;

/*******************************************************************************
 * moved from LookUpTable.h
 */

typedef struct LUT_T lut_t;

/*******************************************************************************
 * moved from scheduling.h
 */

typedef struct SCHED_T sched_t;

typedef struct TASKSEL_T tasksel_t;

/*******************************************************************************
 * moved from serialize_stack.h
 */

typedef struct SERSTACK_T serstack_t;

#endif /* _SAC_TYPES_H_ */
