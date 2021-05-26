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
#include "argcount-cpp.h"
#include "types_nodetype.h"
#include "types_trav.h"

#if IS_CYGWIN
#include <stdarg.h>
#endif

#include <stdbool.h>
#define TRUE true
#define FALSE false

/*
 * moved from shape.h
 */

typedef struct SHAPE shape;

/* Structcure to store where a token came from.  */
struct location {
    const char *fname;
    size_t line, col;
};

#if HAVE_GCC_SIMD_OPERATIONS
typedef float __attribute__ ((vector_size (4 * sizeof (float)))) floatvec;
#define FLOATVEC_IDX(vec, idx) (vec)[(idx)]
#else
/* You cannot assign static arrays, so we go with struct.  Stupid C!  */
typedef struct {
    float a, b, c, d;
} __attribute__ ((packed)) floatvec;
#define FLOATVEC_IDX(vec, idx) ((float *)&(vec))[(idx)]
#endif

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

enum simpletype_t {
#define TYP_IFname(name) name
#include "type_info.mac"
#undef TYP_IFname
};
typedef enum simpletype_t simpletype;

/*
 * usertype is used in the new type representation only!!!
 * The association between usertype and its definition is
 * encapsulated in the module "user_types"
 * (cf. src/typecheck/user_types.[ch]).
 */
typedef int usertype;

typedef enum distmem_dis_t {
    distmem_dis_dis,
    distmem_dis_ndi,
    distmem_dis_dsm
} distmem_dis;

/**
 * @brief Create PH_* based phase name, with different structures depending on
 *        number of passed arguments.
 *
 * This macro is used in several places in the libsac2c sources.
 *
 * @see phase_drivers.c
 * @see phase_info.c
 */
#define PHASENAME(...) \
    MACRO_GLUE(__PNARG, MACRO_ARGCOUNT(__VA_ARGS__))(__VA_ARGS__)
#define __PNARG1(phase) PH_##phase
#define __PNARG2(phase, name) PH_##phase##_##name
#define __PNARG3(phase, cycle, name) PH_##phase##_##cycle##_##name

#define PHASE(name, text, cond) PHASENAME(name),

#define SUBPHASE(name, text, fun, cond, phase) PHASENAME(phase, name),

#define CYCLE(name, text, cond, phase, setup) PHASENAME(phase, name),

#define CYCLEPHASE(name, text, fun, cond, phase, cycle) PHASENAME(phase, cycle, name),

#define FUNBEGIN(name, phase, cycle) PHASENAME(phase, cycle, name),

#define CYCLEPHASEFUN(name, text, fun, cond, phase, cycle) PHASENAME(phase, cycle, name),

#define CYCLEPHASEFUNOLD(name, text, fun, cond, phase, cycle)                            \
    PHASENAME(phase, cycle, name),

typedef enum {
    PH_dummy = -1, /* Prevent comparison with >= PH_initial from producing a warning */
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
#undef CYCLEPHASEFUNOLD

typedef enum { TOOL_sac2c, TOOL_sac4c, TOOL_sac2tex } tool_t;

typedef enum {
    HWLOC_off,
    HWLOC_simple,
    HWLOC_env,
    HWLOC_numa,
    HWLOC_socket,
    HWLOC_envString
} cpubindstrategy_t;

typedef enum { FT_prog, FT_modimp, FT_classimp, FT_cmod, FT_unknown } file_type;

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

typedef enum {
    MT_none = 0,
    MT_createjoin = 1, /* PThreads */
    MT_startstop = 2,  /* PThreads, default */
    MT_mtstblock = 3   /* ?? */
} mtmode_t;

/* the possible executiomodes of mtmode 3 (mtstblock) */
typedef enum {
    MUTH_ANY,
    MUTH_EXCLUSIVE,
    MUTH_SINGLE,
    MUTH_MULTI,
    MUTH_MULTI_SPECIALIZED
} mtexecmode_t;

typedef enum { CUDA_HOST_SINGLE, CUDA_DEVICE_SINGLE, CUDA_DEVICE_MULTI } cudaexecmode_t;

typedef enum {
#define PRFname(name) F_##name
#include "prf_info.mac"
} prf;

typedef enum {
    PA_x = 0, /* No argument */
    PA_S = 1, /* Scalar argument */
    PA_V = 2, /* Vector argument */
    PA_A = 3  /* Array argument */
} arg_encoding_t;

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
 * fold operation OpenMP supports limited
 * operators
 */
typedef enum {
    OMP_REDUCTION_NONE = 0,
    OMP_REDUCTION_SCL_ADD = 1,
    OMP_REDUCTION_SCL_MUL = 2,
    OMP_REDUCTION_BOOL_AND = 3,
    OMP_REDUCTION_BOOL_OR = 4
} omp_reduction_op;

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

typedef struct ACCESS_T {
    struct NODE *array_vardec; /* */
    struct NODE *iv_vardec;    /* index vector */
    accessclass_t accessclass; /* */
    shape *offset;            /* */
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
   inside a WL body and which have ASSIGN_STMTs N_let and N_prf(F_sel_VxA). */
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
    prf mprf;       /* prf +,-,* or / */
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
    size_t size;
    node **ptr_in;  /* N_arg or N_exprs node */
    node **ptr_out; /* N_ret or N_ids node */
    argtag_t *tag;
} argtab_t;

/*
 * The following defines indicate the position of tags within name tuples.
 * They should be kept in synch with the macros in std.h.
 */
#define NT_NAME_INDEX 0
#define NT_SHAPE_INDEX 1
#define NT_HIDDEN_INDEX 2
#define NT_UNIQUE_INDEX 3
#define NT_MUTC_STORAGE_CLASS_INDEX 4
#define NT_MUTC_SCOPE_INDEX 5
#define NT_MUTC_USAGE_INDEX 6
#define NT_BITARRAY_INDEX 7
#define NT_DISTRIBUTED_INDEX 8
#define NT_CBASETYPE_INDEX 9

/*
 * Enumerated types for classes
 */

typedef enum {
#define ATTRIB NT_SHAPE_INDEX
#define NTIFtype(it_type) it_type
#include "nt_info.mac"
} shape_class_t;

typedef enum {
#define ATTRIB NT_HIDDEN_INDEX
#define NTIFtype(it_type) it_type
#include "nt_info.mac"
} hidden_class_t;

typedef enum {
#define ATTRIB NT_UNIQUE_INDEX
#define NTIFtype(it_type) it_type
#include "nt_info.mac"
} unique_class_t;

typedef enum {
#define ATTRIB NT_MUTC_STORAGE_CLASS_INDEX
#define NTIFtype(it_type) it_type
#include "nt_info.mac"
} mutc_storage_class_class_t;

typedef enum {
#define ATTRIB NT_MUTC_SCOPE_INDEX
#define NTIFtype(it_type) it_type
#include "nt_info.mac"
} mutc_scope_class_t;

typedef enum {
#define ATTRIB NT_MUTC_USAGE_INDEX
#define NTIFtype(it_type) it_type
#include "nt_info.mac"
} mutc_usage_class_t;

typedef enum {
#define ATTRIB NT_BITARRAY_INDEX
#define NTIFtype(it_type) it_type
#include "nt_info.mac"
} bitarray_class_t;

typedef enum {
#define ATTRIB NT_DISTRIBUTED_INDEX
#define NTIFtype(it_type) it_type
#include "nt_info.mac"
} distributed_class_t;

typedef enum {
#define ATTRIB NT_CBASETYPE_INDEX
#define NTIFtype(it_type) it_type
#include "nt_info.mac"
} cbasetype_class_t;

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

typedef void (*cv2cvfunptr) (void *, size_t, size_t, void *, size_t);

/*
 * moved from cv2scalar.h
 */

typedef node *(*cv2scalarfunptr) (void *, size_t);

/*
 * moved from cv2internal_lib.h
 */

typedef char *(*cv2strfunptr) (void *, size_t, size_t, size_t);

/*
 * moved from zipcv.h
 */

typedef void (*zipcvfunptr) (void *, size_t, void *, size_t, void *, size_t);

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
 * for set_expression_utils.h:
 */

typedef struct IDTABLE idtable;

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
typedef size_t (*te_funptr) (ntype *args);

/*******************************************************************************
 *
 * moved from sig_deps.h
 */

typedef ntype *(*ct_funptr) (te_info *, ntype *);
typedef struct SIG_DEP sig_dep;

/*
 * built to facilitate printable_target_functions.h
 */

/**
 * These types are used in target printing
 */

typedef struct PRINTABLE_TARGET printable_target;

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

#define DEF_RESOURCES_ALL                                                                \
    DEF_RESOURCE (CPP_STDIN, cpp_stdin, char *, str)                                     \
    DEF_RESOURCE (CPP_FILE, cpp_file, char *, str)                                       \
    DEF_RESOURCE (TMPDIR, tmpdir, char *, str)                                           \
    DEF_RESOURCE (LIBPATH, libpath, char *, str)                                         \
    DEF_RESOURCE (IMPPATH, imppath, char *, str)                                         \
    DEF_RESOURCE (EXTLIBPATH, extlibpath, char *, str)                                   \
    DEF_RESOURCE (INCPATH, incpath, char *, str)                                         \
    DEF_RESOURCE (TREEPATH, treepath, char *, str)                                       \
    DEF_RESOURCE (RMDIR, rmdir, char *, str)                                             \
    DEF_RESOURCE (MKDIR, mkdir, char *, str)                                             \
    DEF_RESOURCE (TREE_OUTPUTDIR, tree_outputdir, char *, str)                           \
    DEF_RESOURCE (LIB_OUTPUTDIR, lib_outputdir, char *, str)                             \
    DEF_RESOURCE (SACINCLUDES, sacincludes, char *, str)                                 \
    DEF_RESOURCE (TREE_CEXT, tree_cext, char *, str)                                     \
    DEF_RESOURCE (TREE_OBJEXT, tree_objext, char *, str)                                 \
    DEF_RESOURCE (TREE_DLLEXT, tree_dllext, char *, str)                                 \
    DEF_RESOURCE (COMPILE_TREE, compile_tree, char *, str)                               \
    DEF_RESOURCE (LINK_TREE, link_tree, char *, str)                                     \
    DEF_RESOURCE (TARGET_ENV, target_env, char *, str)                                   \
    DEF_RESOURCE (SBI, sbi, char *, str)                                                 \
    DEF_RESOURCE (VARIANT, variant, char *, str)                                         \
    DEF_RESOURCE (BACKEND, backend, char *, str)                                         \
    DEF_RESOURCE (DISTMEM_COMMLIB, distmem_commlib, char *, str)                         \
    DEF_RESOURCE (COMMLIB_CONDUIT, commlib_conduit, char *, str)                         \
    DEF_RESOURCE (RC_METHOD, rc_method, char *, str)                                     \
    DEF_RESOURCE (CUDA_ALLOC, cuda_alloc, char *, str)                                   \
    DEF_RESOURCE (CUDA_ARCH, cuda_arch, char *, str)                                     \
    DEF_RESOURCE (USE_PHM_API, use_phm_api, int, num)                                    \
    DEF_RESOURCE (RTSPEC, rtspec, int, num)                                              \
    DEF_RESOURCE (MT_LIB, mt_lib, char *, str)                                           \
    DEF_RESOURCE (MT_MODE, mt_mode, int, num)                                            \
    DEF_RESOURCE (CACHE1_SIZE, cache1_size, int, num)                                    \
    DEF_RESOURCE (CACHE1_LINE, cache1_line, int, num)                                    \
    DEF_RESOURCE (CACHE1_ASSOC, cache1_assoc, int, num)                                  \
    DEF_RESOURCE (CACHE1_WRITEPOL, cache1_writepol, char *, str)                         \
    DEF_RESOURCE (CACHE1_MSCA, cache1_msca_factor, int, num)                             \
    DEF_RESOURCE (CACHE2_SIZE, cache2_size, int, num)                                    \
    DEF_RESOURCE (CACHE2_LINE, cache2_line, int, num)                                    \
    DEF_RESOURCE (CACHE2_ASSOC, cache2_assoc, int, num)                                  \
    DEF_RESOURCE (CACHE2_WRITEPOL, cache2_writepol, char *, str)                         \
    DEF_RESOURCE (CACHE2_MSCA, cache2_msca_factor, int, num)                             \
    DEF_RESOURCE (CACHE3_SIZE, cache3_size, int, num)                                    \
    DEF_RESOURCE (CACHE3_LINE, cache3_line, int, num)                                    \
    DEF_RESOURCE (CACHE3_ASSOC, cache3_assoc, int, num)                                  \
    DEF_RESOURCE (CACHE3_WRITEPOL, cache3_writepol, char *, str)                         \
    DEF_RESOURCE (CACHE3_MSCA, cache3_msca_factor, int, num)                             \
    DEF_RESOURCE (CCP_CEXT, ccp_cext, char *, str)                                       \
    DEF_RESOURCE (CCP_OBJEXT, ccp_objext, char *, str)                                     \
    DEF_RESOURCE (CEXT, cext, char *, str)                                               \
    DEF_RESOURCE (OBJEXT, objext, char *, str)                                           \
    DEF_RESOURCE (MODEXT, modext, char *, str)                                           \
    DEF_RESOURCE (EXEEXT, exeext, char *, str)                                           \
    DEF_RESOURCE (CC, cc, char *, str)                                                   \
    DEF_RESOURCE (CFLAGS, cflags, char *, str)                                           \
    DEF_RESOURCE (LD, ld, char *, str)                                                   \
    DEF_RESOURCE (LIBS, libs, char *, str)                                               \
    DEF_RESOURCE (LDPATH, ldpath, char *, str)                                           \
    DEF_RESOURCE (LDFLAGS, ldflags, char *, str)                                         \
    DEF_RESOURCE (OPT_O0, opt_o0, char *, str)                                           \
    DEF_RESOURCE (OPT_O1, opt_o1, char *, str)                                           \
    DEF_RESOURCE (OPT_O2, opt_o2, char *, str)                                           \
    DEF_RESOURCE (OPT_O3, opt_o3, char *, str)                                           \
    DEF_RESOURCE (OPT_g, opt_g, char *, str)                                             \
    DEF_RESOURCE (TUNE_native, tune_native, char *, str)                                 \
    DEF_RESOURCE (TUNE_generic, tune_generic, char *, str)                               \
    DEF_RESOURCE (CCP_MOD, ccp_mod, char *, str)                                         \
    DEF_RESOURCE (CCP_PROG, ccp_prog, char *, str)                                       \
    DEF_RESOURCE (COMPILE_MOD, compile_mod, char *, str)                                 \
    DEF_RESOURCE (COMPILE_PROG, compile_prog, char *, str)                               \
    DEF_RESOURCE (LINK_MOD, link_mod, char *, str)                                       \
    DEF_RESOURCE (LINK_PROG, link_prog, char *, str)                                     \
    DEF_RESOURCE (COMPILE_RMOD, compile_rmod, char *, str)                               \
    DEF_RESOURCE (LINK_RMOD, link_rmod, char *, str)

typedef struct {
#define DEF_RESOURCE(Name, Attr, Type1, Type2) Type1 Attr;
    DEF_RESOURCES_ALL
#undef DEF_RESOURCE
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

typedef enum {
    STRS_unknown,
    STRS_saclib,
    STRS_extlib,
    STRS_objfile,
    STRS_headers
} strstype_t;
typedef void *(*strsfoldfun_p) (const char *elem, strstype_t kind, void *rest);

/* string sets, see stringset.h */
typedef struct STRINGSET_T {
    char *val;
    strstype_t kind;
    struct STRINGSET_T *next;
} stringset_t;

/*
 * moved from filemgr.h
 */

typedef enum {
    PK_path = 0,
    PK_lib_path = 1,
    PK_imp_path = 2,
    PK_extlib_path = 3,
    PK_tree_path = 4,
    PK_inc_path = 5,
    PK_LAST = 6
} pathkind_t;

/*
 * moved from libmanager.h
 */
typedef void *dynlib_t;
typedef void *dynfun_t;

/*
 * moved from pattern_match_modes.h
 */
typedef struct PAT pattern;
typedef node *skip_fun_t (intptr_t, node *);
typedef bool prf_match_fun_t (prf);
typedef struct {
    skip_fun_t *fun;
    intptr_t param;
} pm_mode_t;

/*
 * moved from pattern_match_attribs.h
 */
typedef struct PATTR attrib;

/*******************************************************************************
 *
 * moved from pad_info.h
 */

/* structure for storing access patterns */
typedef struct PATTERN_T {
    shape *pattern;
    struct PATTERN_T *next;
} pattern_t;

/* structure for grouping access patterns by conflict groups */
typedef struct CONFLICT_GROUP_T {
    shape *group;
    accessdir_t direction;
    pattern_t *patterns;
    struct CONFLICT_GROUP_T *next;
} conflict_group_t;

/* strcture for grouping conflict groups by array types */
typedef struct ARRAY_TYPE_T {
    simpletype type;
    int dim;
    shape *shp;
    conflict_group_t *groups;
    struct ARRAY_TYPE_T *next;
} array_type_t;

/* structure containing shapes of unsupported operations */
typedef struct UNSUPPORTED_SHAPE_T {
    simpletype type;
    int dim;
    shape *shp;
    struct UNSUPPORTED_SHAPE_T *next;
} unsupported_shape_t;

/* structure containing old and inferred array shape */
typedef struct PAD_INFO_T {
    simpletype type;
    int dim;
    shape *old_shape;
    shape *new_shape;
    shape *padding;
    node *fundef_pad;
    node *fundef_unpad;
    struct PAD_INFO_T *next;
} pad_info_t;

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

typedef enum { DO_C_none = 0, DO_C_prog, DO_C_mod, DO_C_rmod } do_c_type_t;

typedef enum {
    CCT_doall,        // regular compile/link for SAC programs/modules
    CCT_linkflags,    // just return %linkflags%
    CCT_compileflags, // just return %compileflags%
    CCT_clinkonly,    // just link C objects
    CCT_ccompileonly  // just compile one C source
} ccm_task_t;

/*
 * moved from modulemanager.h
 */
/* typedef struct MODULE_T module_t;*/

typedef struct MODULE_T {
    char *name;
    char *sofile;
    dynlib_t lib;
    stringset_t *headers;
    sttable_t *stable;
    struct MODULE_T *next;
    int usecount;
} module_t;

/*
 * The serfun_* types are used by modulemanager and the serialise/de-serialise
 * traversals to access serialised functions. The functions are *never* defined
 * as having an argument (i.e. void *(*)(void)).
 */
typedef node *(*serfun_p) (void);

typedef union {
    void *v;
    node *(*f) (void);
} serfun_u;

/*
 * New types for global
 */

/* Communication libraries for the DistMem (distributed memory) backend. */
typedef enum {
#define DISTMEM_COMMLIBtype(type) type,
#include "distmem_commlibs.mac"
#undef DISTMEM_COMMLIBtype
    DISTMEM_COMMLIB_UNKNOWN
} distmem_commlib_t;

/*
 * Read in optimization counters from optimize.mac
 */

typedef struct OPTIMIZE_COUNTER_T {
#define OPTCOUNTERid(id) size_t id;
#include "optimize.mac"
} optimize_counter_t;

/*
 * Read in optimization flag type from optimize.mac
 */

typedef struct OPTIMIZE_FLAGS_T {
/* this uses 3-valued logic, to support delaying the configuration until sac2crc is
 * loaded. */
#define OPTIMIZEabbr(abbr) unsigned int do##abbr : 2;
#include "optimize.mac"
} optimize_flags_t;

/*
 * Read in configuration flag types from flags.mac
 */

typedef struct TRACE_FLAGS_T {
#define TRACEflag(flag) unsigned int flag : 1;
#include "flags.mac"
} trace_flags_t;

typedef struct VISUALIZEFUNSETS_FLAGS_T {
#define VISUALIZEFUNSETSflag(flag) unsigned int flag : 1;
#include "flags.mac"
} visualizefunsets_flags_t;

typedef struct PRINTFUNSETS_FLAGS_T {
#define PRINTFUNSETSflag(flag) unsigned int flag : 1;
#include "flags.mac"
} printfunsets_flags_t;

typedef struct PRINT_FLAGS_T {
#define PRINTflag(flag) unsigned int flag : 1;
#include "flags.mac"
} print_flags_t;

typedef struct PROFILE_FLAGS_T {
#define PROFILEflag(flag) unsigned int flag : 1;
#include "flags.mac"
} profile_flags_t;

typedef struct FEEDBACK_FLAGS_T {
#define FEEDBACKflag(flag) unsigned int flag : 1;
#include "flags.mac"
} feedback_flags_t;

typedef struct CACHESIM_FLAGS_T {
#define CSflag(flag) unsigned int flag : 1;
#include "flags.mac"
} cachesim_flags_t;

typedef struct RUNTIMECHECK_FLAGS_T {
#define RTCflag(flag) unsigned int flag : 1;
#include "flags.mac"
} runtimecheck_flags_t;

/*
typedef struct GENLIB_FLAGS_T {
  #define GENLIBflag( flag) unsigned int flag : 1;
  #include "flags.mac"
} genlib_flags_t;
*/

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
 * struct for list of pre/post traversal functions
 */
typedef struct TRAVFUNLIST {
    travfun_p fun;
    struct TRAVFUNLIST *next;
} travfunlist_t;

/*
 * type for anonymous traversal definitions
 */
typedef struct {
    nodetype node;
    travfun_p travfun;
} anontrav_t;

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
 * typedef for cross edge reachability analysis
 */

struct ELEM {
    int idx;
    void *data;
};

struct DYNARRAY {
    struct ELEM **elems;
    int totalelems;
    int allocelems;
};

struct MATRIX {
    struct DYNARRAY **array2d;
    int totalrows;
    int totalcols;
};

struct ELEMSTACK {
    struct ELEM *curr;
    struct ELEMSTACK *next;
};

struct ELEMLIST {
    struct ELEM *curr;
    struct ELEMLIST *prev;
    struct ELEMLIST *next;
};

struct ELEMQUEUE {
    struct ELEMLIST *head;
    struct ELEMLIST *tail;
};

struct LUBINFO {
    int numintra;
    int blocksize;
    struct DYNARRAY *blockmin;
    struct MATRIX *intermat;
    struct MATRIX **intramats;
    struct MATRIX *pcptmat;
    struct MATRIX *pcpcmat;
};

struct COMPINFO {
    struct DYNARRAY *csrc;
    struct DYNARRAY *ctar;
    struct DYNARRAY *tltable;
    struct DYNARRAY *eulertour;
    struct DYNARRAY *prearr;
    struct MATRIX *crossclos;
    struct MATRIX *tlc;
    struct LUBINFO *lub;
    struct MATRIX *dist;
    struct NODELIST *topolist;
};

typedef enum { edgetree, edgecross, edgeforward, edgeback } graph_edgetype;
typedef enum { tree_labeling, nontree_labeling, edge_labeling } graph_label_mode;
typedef enum { vertices, edges } dot_output_mode;
typedef struct ELEM elem;
typedef struct DYNARRAY dynarray;
typedef struct MATRIX matrix;
typedef struct ELEMSTACK elemstack;
typedef struct ELEMLIST elemlist;
typedef struct ELEMQUEUE elemqueue;
typedef struct LUBINFO lubinfo;
typedef struct COMPINFO compinfo;

/* typdef for CUDA block sizes and other parameters */
#define CUDA_OPTIONS_ALL                                                                 \
    CUDA_OPTION (optimal_threads, int, num)                                              \
    CUDA_OPTION (optimal_blocks, int, num)                                               \
    CUDA_OPTION (cuda_1d_block_x, int, num)                                              \
    CUDA_OPTION (cuda_1d_block_large, int, num)                                          \
    CUDA_OPTION (cuda_1d_block_small, int, num)                                          \
    CUDA_OPTION (cuda_2d_block_x, int, num)                                              \
    CUDA_OPTION (cuda_2d_block_y, int, num)                                              \
    CUDA_OPTION (cuda_max_x_grid, unsigned int, num)                                     \
    CUDA_OPTION (cuda_max_yz_grid, unsigned int, num)                                    \
    CUDA_OPTION (cuda_max_xy_block, unsigned int, num)                                   \
    CUDA_OPTION (cuda_max_z_block, unsigned int, num)                                    \
    CUDA_OPTION (cuda_max_threads_block, unsigned int, num)

typedef struct {
#define CUDA_OPTION(name, type, attr) type name;
    CUDA_OPTIONS_ALL
#undef CUDA_OPTION
} cuda_options_t;

#define CUDA_ARCHS_ALL                                                                   \
    CUDA_ARCH (SM10, "sm_10")                                                            \
    CUDA_ARCH (SM11, "sm_11")                                                            \
    CUDA_ARCH (SM12, "sm_12")                                                            \
    CUDA_ARCH (SM13, "sm_13")                                                            \
    CUDA_ARCH (SM20, "sm_20")                                                            \
    CUDA_ARCH (SM35, "sm_35")                                                            \
    CUDA_ARCH (SM50, "sm_50")                                                            \
    CUDA_ARCH (SM60, "sm_60")                                                            \
    CUDA_ARCH (SM61, "sm_61")                                                            \
    CUDA_ARCH (SM70, "sm_70")                                                            \
    CUDA_ARCH (SM75, "sm_75")

/* typdef enum for CUDA architecture setting */
typedef enum cuda_arch_e {
#define CUDA_ARCH(name, flagopt) CUDA_##name,
    CUDA_ARCHS_ALL
#undef CUDA_ARCH
} cuda_arch_t;

/* typedef enum for CUDA Async Modes */
typedef enum cuda_async_mode_e {
    CUDA_SYNC_NONE,
    CUDA_SYNC_DEVICE,
    CUDA_SYNC_STREAM,
    CUDA_SYNC_CALLBACK
} cuda_async_mode_t;

/******************************************************************************
 *
 * The following types are for cygwin compatibility only
 *
 ******************************************************************************/
#if IS_CYGWIN
/*
 * This struct is used to collect function pointers to allow compiled modules
 * to callback to the correct libsac2c.x.dll
 * It is passed to any sac module that is opened dlopened.
 *
 * See cygcompat.c for more information.
 */

typedef struct CYG_FUN_TABLE {
    char *(*STRcpy_fp) (const char *);
    shape *(*SHcreateShapeVa_fp) (int, va_list);
    ntype *(*TYdeserializeTypeVa_fp) (int, va_list);
    namespace_t *(*NSdeserializeNamespace_fp) (int);
    int (*NSaddMapping_fp) (const char *, void *);
    void *(*NSdeserializeView_fp) (const char *, int, void *);
    node *(*SHLPmakeNodeVa_fp) (int, char *, size_t, size_t, va_list);
    void (*SHLPfixLink_fp) (serstack_t *, int, int, int);
    serstack_t *(*SERbuildSerStack_fp) (node *);
    constant *(*COdeserializeConstant_fp) (simpletype, shape *, size_t, char *);
    node *(*DSlookupFunction_fp) (const char *, const char *);
    node *(*DSlookupObject_fp) (const char *, const char *);
    node *(*DSfetchArgAvis_fp) (int);
    double (*DShex2Double_fp) (const char *);
    float (*DShex2Float_fp) (const char *);
    sttable_t *(*STinit_fp) (void);
    void (*STadd_fp) (const char *, int, const char *, int, void *, unsigned);
    stringset_t *(*STRSadd_fp) (const char *, strstype_t, stringset_t *);
} cyg_fun_table_t;

#endif /* IS_CYGWIN */

/*****************************************************************************/

/*
 * Read in global variables from globals.mac
 *
 * GLOBAL_NOINIT is a special case whereby the global variable is not
 *               initialised, this is used in the case where it must be set
 *               *before* the initialisation of all global variables occur.
 */

typedef struct GLOBAL_T {
#define GLOBAL(type, name, val, ...) type name;
#define GLOBAL_NOINIT(type, name, val, ...) type name;
#include "globals.mac"
} global_t;

/******************************************************************************/

/* typedef for CUDA data reuse analysis
 */

typedef struct sMtx {
    unsigned int dim_x, dim_y;
    int *m_stor;
    int **mtx;
} * IntMatrix, sMatrix;


#define SHP_SEG_SIZE 16

/* These two structs are used to annotate reusable arrays
 * in a wl. The info will be attached to N_code node */
typedef struct RC_T {
    node *array;
    node *arrayshp;
    node *sharray;
    node *sharrayshp;
    size_t dim;
    bool selfref;
    int posoffset[SHP_SEG_SIZE];
    int negoffset[SHP_SEG_SIZE];
    bool reusable;
    struct RC_T *next;
} rc_t;

typedef struct REUSE_INFO_T {
    int count;
    rc_t *rcs;
} reuse_info_t;

/* For the moment, we only look at potential reuse opportunities
 * for 1 or 2 dimensional arrays */
#define MAX_REUSE_DIM 2

#define IDX_CONSTANT 0
#define IDX_THREADIDX_X 1
#define IDX_THREADIDX_Y 2
#define IDX_THREADIDX 3
#define IDX_LOOPIDX 4
#define IDX_EXTID 5
#define IDX_WITHIDS 6 /* This type is only used in reusewithregion.c */

#define ACCTY_REUSE 0
#define ACCTY_COALESCE 1

typedef struct CUDA_INDEX_T {
    unsigned int type;
    int coefficient;
    node *id;
    size_t looplevel; /* This attribute is only meaningful if type is LOOPIDX */
    struct CUDA_INDEX_T *next;
} cuda_index_t;

typedef struct CUDA_ACCESS_INFO_T {
    IntMatrix coe_mtx;
    unsigned int type; /* Type of this access: either reuse or coalescing */
    int dim;
    size_t nestlevel;
    node *array;
    node *arrayshp;
    node *sharray;
    node *sharrayshp_phy; /* Physical shape of the shared memory */
    node
      *sharrayshp_log; /* Logical shape of the sahred memory(smaller than the physical
                          one) */
    size_t cuwldim;       /* Dimesion of the containing cuda withloop */
    node *tbshp;       /* shape of the thread block */
    cuda_index_t *indices[MAX_REUSE_DIM];
    bool isconstant[MAX_REUSE_DIM]; /* whether a dimension is constant, i.e. consists of
                                       only IDX_CONSTANT and IDX_EXTID indeices*/
} cuda_access_info_t;

struct DAG {
    node *gnode;
};

struct VERTEX {
    node *vnode;
    void *annotation;
};

typedef struct DAG dag;
typedef struct VERTEX vertex;
typedef bool (*idag_fun_t) (void *, void *);

/******************************************************************************
 * N_avis attributes
 */

#define SHAPECLIQUEIDNONE(AVIS) (AVIS)

#endif /* _SAC_TYPES_H_ */
