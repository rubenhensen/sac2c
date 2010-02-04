/*
 *
 * $Id$
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
#include <inttypes.h>

#include "config.h"
#include "types_nodetype.h"
#include "types_trav.h"

/*
 * bool values
 */

#ifdef __bool_true_false_are_defined

#undef bool
#undef true
#undef false

#endif /* __bool_true_false_are_defined */

typedef int bool;

#define FALSE 0
#define TRUE 1

/*
 * The NEW node structure of the SAC syntax tree
 * The type is abstract, as there is _no_ way to access a node other
 * than using tree_basic.h. Thus the structure is defined in
 * tree_basic.h. This as well solves dependency problems.
 */
typedef struct NODE node;

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

/*
 * information structure for the lac-function traversal
 */
typedef struct LAC_INFO lac_info_t;

/*
 * the namespace_t structure is used for namespaces (formerly represented
 * by module name strings). See namespace.[ch] for details
 */
typedef struct NAMESPACE namespace_t;
typedef struct VIEW view_t;

/*
 * The type feature_t is used as a bit mask for tile size inference.
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

#define PHASE(name, text, cond) PH_##name,

#define SUBPHASE(name, text, fun, cond, phase) PH_##phase##_##name,

#define CYCLE(name, text, cond, phase, setup) PH_##phase##_##name,

#define CYCLEPHASE(name, text, fun, cond, phase, cycle) PH_##phase##_##cycle##_##name,

#define FUNBEGIN(name, phase, cycle) PH_##phase##_##cycle##_##name,

#define CYCLEPHASEFUN(name, text, fun, cond, phase, cycle) PH_##phase##_##cycle##_##name,

typedef enum {
    PH_initial = 0,
#include "phase_sac2c.mac"
    PH_final,
#include "phase_sac4c.mac"
    PH_final_sac4c,
#include "phase_sac2tex.mac"
    PH_final_sac2tex,
    PH_undefined
} compiler_phase_t;

#undef PHASE
#undef SUBPHASE
#undef CYCLE
#undef CYCLEPHASE
#undef FUNBEGIN
#undef CYCLEPHASEFUN

typedef enum { TOOL_sac2c, TOOL_sac4c, TOOL_sac2tex } tool_t;

typedef enum { F_prog, F_modimp, F_classimp, F_cmod, F_unknown } file_type;

typedef enum { CT_normal, CT_ap, CT_array, CT_return, CT_wl } contextflag;

typedef enum { ACL_irregular, ACL_unknown, ACL_offset, ACL_const } accessclass_t;

typedef enum { ADIR_read, ADIR_write } accessdir_t;

typedef enum { NO_UNQCONV, TO_UNQ, FROM_UNQ } unqconv_t;

typedef enum { LOC_usr, LOC_stdlib } locationtype;

typedef enum { CMPT_EQ, CMPT_NEQ, CMPT_UKNWN } cmptree_t;

typedef enum { SS_aks, SS_akd, SS_aud } spec_mode_t;

typedef enum { IVE_all = 3, IVE_aks = 1, IVE_akd = 2 } ivetype_t;

typedef enum {
    IVEO_all = 15,
    IVEO_wlidx = 1,
    IVEO_idxs = 2,
    IVEO_copt = 4,
    IVEO_share = 8
} iveotype_t;

typedef enum { SSP_akv, SSP_aks, SSP_akd, SSP_aud } sigspec_mode_t;

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
#define PRFname(name) F_##name
#include "prf_info.mac"
} prf;

typedef enum { PA_x = 0, PA_S = 1, PA_V = 2, PA_A = 3 } arg_encoding_t;

typedef node *(*cf_fun_t) (node *arg_node);

/*
 * minimum array representation class
 */
typedef enum {
    MAR_scl_aks = 0,
    MAR_scl_akd = 1,
    MAR_scl_aud = 2,
    MAR_aud = 3
} min_array_rep_t;

/*
 * enums used by the mutc backend
 */

typedef enum { MUTC_SC_INT, MUTC_SC_FLOAT } mutcStorageClass;

typedef enum {
    MUTC_GLOBAL,
    MUTC_SHARED,
    MUTC_LOCAL,
} mutcScope;

typedef enum {
    MUTC_US_DEFAULT,
    MUTC_US_FUNPARAMIO,
    MUTC_US_FUNPARAM,
    MUTC_US_THREADPARAMIO,
    MUTC_US_THREADPARAM,
    MUTC_US_FUNARG
} mutcUsage;

typedef enum {
    MUTC_DMODE_default = 0,
    MUTC_DMODE_toplevel = 1,
    MUTC_DMODE_bounded = 2
} mutc_distribution_mode_t;

/*
 * structs
 */

typedef struct NODELIST {
    struct NODE *node;
    int status;
    int num;
    struct NODE *attrib2;
    struct NODELIST *next;
} nodelist;

#define SHP_SEG_SIZE 16

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

/*
 * This structure is used by wl_access_analyse to anotate
 * some information to N_CODE nodes. See NCODE_WLAA macros
 */
typedef struct ACCESS_INFO_T {
    access_t *access;
    int accesscnt;
    int feature;
    struct NODE *indexvar;
    struct NODE *wlarray;
} access_info_t;

typedef struct TYPES {
    simpletype simpletype;
    char *name;         /* only used for T_user !! */
    char *name_mod;     /* name of modul belonging to 'name' */
    struct NODE *tdef;  /* typedef of user-defined type */
    int dim;            /* if (dim == 0) => simpletype */
    bool poly;          /* only needed for type templates (newTC !) */
    shpseg *shpseg;     /* pointer to shape specification */
    struct TYPES *next; /* only needed for fun-results  */
                        /* and implementation of implicit types */
    /* mutc backend */
    mutcScope scope; /* the scope of the value of this var */
    mutcUsage usage; /* where is this var used */

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

/******************************************************************************
 *
 * the following types are needed for Withloop Folding
 *
 ******************************************************************************/

/* The following struct is only annotated to N_assign nodes which are
   inside a WL body and which have ASSIGN_INSTRs N_let and N_prf(F_sel_VxA). */
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
    node **ptr_out; /* N_ret or N_ids node */
    argtag_t *tag;
} argtab_t;

/*
 * The following defines indicate the position of tags within name tuples.
 * They should be kept in synch with the NT_NAME, NT_SHP, NT_HID and NT_UNQ
 * macros in sac_std.h
 */
#define NT_NAME_INDEX 0
#define NT_SHAPE_INDEX 1
#define NT_HIDDEN_INDEX 2
#define NT_UNIQUE_INDEX 3
#define NT_MUTC_STORAGE_CLASS_INDEX 4
#define NT_MUTC_SCOPE_INDEX 5
#define NT_MUTC_USAGE_INDEX 6

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

typedef enum {
#define ATTRIB NT_MUTC_STORAGE_CLASS_INDEX
#define NTIFtype(it_type) it_type
#include "nt_info.mac"
#undef NTIFtype
#undef ATTRIB
} mutc_storage_class_class_t;

typedef enum {
#define ATTRIB NT_MUTC_SCOPE_INDEX
#define NTIFtype(it_type) it_type
#include "nt_info.mac"
#undef NTIFtype
#undef ATTRIB
} mutc_scope_class_t;

typedef enum {
#define ATTRIB NT_MUTC_USAGE_INDEX
#define NTIFtype(it_type) it_type
#include "nt_info.mac"
#undef NTIFtype
#undef ATTRIB
} mutc_usage_class_t;

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
 * moved from cv2internal_lib.h
 */

typedef char *(*cv2strfunptr) (void *, int, int, int);

/*
 * moved from zipcv.h
 */

typedef void (*zipcvfunptr) (void *, int, void *, int, void *, int);

/*******************************************************************************
 *
 * moved from new_types.h
 */

typedef struct NTYPE ntype;

/*******************************************************************************
 *
 * moved from new_typecheck.c
 */

typedef enum { NTC_not_checked, NTC_checking, NTC_checked } NTC_stat;

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

typedef enum {
    TE_udf,
    TE_prf,
    TE_cond,
    TE_funcond,
    TE_generator,
    TE_with,
    TE_foldf
} te_kind_t;

typedef struct TE_INFO te_info;
typedef int (*te_funptr) (ntype *args);

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
 * type: inheritence_list_t
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

typedef struct INHERITENCE_LIST_T {
    char *name;
    struct INHERITENCE_LIST_T *next;
} inheritence_list_t;

typedef struct TARGET_LIST_T {
    char *name;
    inheritence_list_t *super_targets;
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

    char *backend;

    char *tree_cc;
    char *tree_ld;
    char *tree_ld_path;
    char *lib_variant;

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
    char *genpic;
    char *ld_path;
    char *ranlib;
    char *mkdir;
    char *rmdir;
    char *chdir;
    char *cat;
    char *move;
    char *rsh;
    char *dump_output;

    char *libpath;
    char *imppath;
    char *extlibpath;
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

/**
 * types from PHP
 */

typedef struct HEAP heap;
typedef bool (*php_cmp_fun) (void *elem1, void *elem2);

/*
 * moved from stringset.h
 */

typedef enum { STRS_unknown, STRS_saclib, STRS_extlib, STRS_objfile } strstype_t;
typedef void *(*strsfoldfun_p) (const char *elem, strstype_t kind, void *rest);

/*
 * moved from filemgr.h
 */

typedef enum {
    PK_path = 0,
    PK_lib_path = 1,
    PK_imp_path = 2,
    PK_extlib_path = 3
} pathkind_t;

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
 * Read in optimization counters from optimize.mac
 */

typedef struct OPTIMIZE_COUNTER_T {
#define OPTCOUNTERid(id) int id;
#include "optimize.mac"
} optimize_counter_t;

/*
 * Read in optimization flag type from optimize.mac
 */

typedef struct OPTIMIZE_FLAGS_T {
#define OPTIMIZEabbr(abbr) unsigned int do##abbr : 1;
#include "optimize.mac"
} optimize_flags_t;

/*
 * Read in configuration flag types from flags.mac
 */

typedef struct TRACE_FLAGS_T {
#define TRACEflag(flag) unsigned int flag : 1;
#include "flags.mac"
} trace_flags_t;

typedef struct PRINT_FLAGS_T {
#define PRINTflag(flag) unsigned int flag : 1;
#include "flags.mac"
} print_flags_t;

typedef struct PROFILE_FLAGS_T {
#define PROFILEflag(flag) unsigned int flag : 1;
#include "flags.mac"
} profile_flags_t;

typedef struct CACHESIM_FLAGS_T {
#define CSflag(flag) unsigned int flag : 1;
#include "flags.mac"
} cachesim_flags_t;

typedef struct RUNTIMECHECK_FLAGS_T {
#define RTCflag(flag) unsigned int flag : 1;
#include "flags.mac"
} runtimecheck_flags_t;

typedef struct GENLIB_FLAGS_T {
#define GENLIBflag(flag) unsigned int flag : 1;
#include "flags.mac"
} genlib_flags_t;

typedef enum {
#define BACKENDtype(type) type,
#include "backends.mac"
    BE_unknown
} backend_t;

/*
 * type of traversal functions
 */
typedef node *(*travfun_p) (node *, info *);

/*
 * type for anonymous traversal definitions
 */
typedef struct {
    nodetype node;
    travfun_p travfun;
} anontrav_t;

/*
 * Read in global variables from globals.mac
 */

typedef struct GLOBAL_T {
#define GLOBALtype(it_type) it_type
#define GLOBALname(it_name) it_name;
#include "globals.mac"
} global_t;

/******************************************************************************
 * moved from SSAConstantFolding.h:
 *
 *
 * structural constant (SCO) should be integrated in constants.[ch] in future
 */
typedef struct STRUCT_CONSTANT struct_constant;

/******************************************************************************
 * moved from DataFlowMask.h:
 */
typedef struct MASK_BASE_T dfmask_base_t;
typedef struct MASK_T dfmask_t;

/******************************************************************************
 * moved from DataFlowMaskUtils.h:
 */
typedef struct STACK_T dfmstack_t;

/******************************************************************************
 * moved from LookUpTable.h
 */

typedef struct LUT_T lut_t;

/******************************************************************************
 * moved from NumLookUpTable.h
 */

typedef struct NLUT_T nlut_t;

/******************************************************************************
 * moved from scheduling.h
 */

typedef struct SCHED_T sched_t;

typedef struct TASKSEL_T tasksel_t;

/******************************************************************************
 * moved from serialize_stack.h
 */

typedef struct SERSTACK_T serstack_t;

/******************************************************************************
 * tyedef for specialization_oracle_static_shape_knowledge
 */

typedef constant *(*shape_oracle_funptr) (int n);

/******************************************************************************
 * N_avis attributes
 */

#define SHAPECLIQUEIDNONE(AVIS) (AVIS)

#endif /* _SAC_TYPES_H_ */
