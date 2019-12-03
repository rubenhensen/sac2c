/*****************************************************************************
 *
 * file:   sac_std.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides definitions of Intermediate Code Macros (ICM)
 *   implemented as real macros.
 *
 *   This file contains all those ICMs that deal with refcounted data
 *   objects in any way
 *
 *****************************************************************************/

#ifndef _SAC_STD_H_
#define _SAC_STD_H_
#include <stdint.h> /* intptr_t */
#include <limits.h> /* INT_MAX */

#ifdef SAC_BACKEND_CUDA
#include <cuda_runtime.h>
#endif

#include "rc_methods.h" // SAC_RCM_local, ...
#include "icm.h" // Item0, ...
#include "cuda_transfer_methods.h" // SAC_DO_CUDA_ALLOC
#include "runtime/extras_h/runtimecheck.h" // SAC_ASSURE_TYPE, ...
#include "runtime/phm_h/phm.h" // SAC_HM_MALLOC, ...
#include "runtime/extras_h/rt_trace.h" // SAC_TR_MEM_PRINT, ...
#include "runtime/extras_h/rt_profile.h" // SAC_PF_MEM_*, ...
#include "runtime/extras_h/rt_cachesim.h" // SAC_CS_UNREGISTER_ARRAY, ...


#define TRUE 1
#define FALSE 0

#ifndef SAC_RC_METHOD /* required for includes from libsac */
#define SAC_RC_METHOD -1
#endif

/*
 * Positional parameters for name tuples (nt):
 *   name, SHP, HID, UNQ, REG, SCO, USG, BIT, DIS, CBT
 *
 * Values are:
 *   name:   data object name
 *
 *   SHP shape: SCL: scalar
 *              AKS: array of known shape
 *              AKD: array of known dimension but unknown shape
 *              AUD: array of unknown dimension
 *
 *   HID hidden:  NHD: data object is non-hidden
 *                HID: data object is hidden
 *                HNS: TODO hidden nested shape?
 *
 *   UNQ unique: NUQ: data object is non-unique
 *               UNQ: data object is unique
 *
 *   REG storage: Used by the Micrgrid backend?
 *                INT: TODO describe
 *                FLO: TODO describe
 *
 *   SCO: Used by the Micrgrid backend?
 *        TODO describe
 *
 *   USG: Used by the Micrgrid backend?
 *        TODO describe
 *
 *   BIT: Bitarray?
 *        TODO describe
 *
 *   DIS distribution: Used by the distmem backend. Can the variable be
 * distributed/allocated in DSM memory? DIS: Distributable, i.e. potentially distributed
 * (and allocated in DSM memory) DSM: Not distributed, but may be allocated in DSM memory
 *                          Used for sub-allocated arrays (with a distributable outer
 * array) DCA: Distributable, i.e., potentially distributed; writing into the local cache
 * is allowed Used for write accesses to individual array elements in modarray primitive
 * functions DLO: Distributable, i.e. potentially distributed; but access is known to be
 * local Used for read accesses to individual array elements in modarray-with-loops NDI:
 * Not distributed, not allocated in DSM memory Only legal value if distributed memory
 * backend not used
 *
 *   CBT c-basetype:  Basetype as declared in generated C-code
 *                    Mainly used for the distributed memory backend, but always available
 * and in some cases used to improve debug output INT: int FLO: float DOU: double
 *                    ... (see SAC_NT_CBASETYPE macro for all values)
 *                    OTH: other
 */

#define NT_NAME(var_NT) Item0 var_NT
#define NT_SHP(var_NT) Item1 var_NT
#define NT_HID(var_NT) Item2 var_NT
#define NT_UNQ(var_NT) Item3 var_NT
#define NT_REG(var_NT) Item4 var_NT
#define NT_SCO(var_NT) Item5 var_NT
#define NT_USG(var_NT) Item6 var_NT
#define NT_BIT(var_NT) Item7 var_NT
#define NT_DIS(var_NT) Item8 var_NT
#define NT_CBT(var_NT) Item9 var_NT

#define NT_STR(var_NT) TO_STR (NT_NAME (var_NT))

/*
 * implementation type for hidden objects
 */

typedef void *SAC_hidden;

/* This type is used to store
 * inforamtiona about a saved reference count
 * [0]       -> rc
 * [1]       -> rc_mode
 */
typedef intptr_t SAC_referencecount_t;
/*
 *  array descriptor parent:
 *  [0]      -> number of child descriptors
 */
typedef intptr_t *SAC_array_descriptor_parent_t;
/*
 * array descriptor:
 *   [0]     -> reference count
 *   [1]     -> parent descriptor ( pointer)
 *   [2]     -> reference count mode
 *   [3]     -> # of dimensions
 *   [4]     -> # of elements
 *   [5]     -> hidden type
 *   [6,...] -> shape vector
 */
/*
 * Use intptr_t so that we can store ints and pointers in the same
 * array
 */
typedef intptr_t *SAC_array_descriptor_t;

#define SAC_RC_T_MODE(rc) (rc)

#define SAC_DESC_PARENT_T_SIZE (1 * sizeof (intptr_t))

// #define SAC_DESC_PARENT_T_CHILDREN( desc_parent)
//   ((desc_parent) [0])

/* size of dimension-independent parts of the descriptor */
#if SAC_DO_DISTMEM
#define FIXED_SIZE_OF_DESC 9
#elif SAC_DO_CUDA_ALLOC == SAC_CA_cureg || SAC_DO_CUDA_ALLOC == SAC_CA_cualloc
#define FIXED_SIZE_OF_DESC 7
#else
/* default */
#define FIXED_SIZE_OF_DESC 6
#endif
/* size of dimension-dependent parts of the descriptor */
#define VAR_SIZE_OF_DESC 1

#if SAC_DO_DISTMEM_PTR_DESC
/*
 * Strictly speaking, we only need to add SAC_DISTMEM_size
 * to the descriptor size for arrays that are distributed
 * (SAC_ND_A_IS_DIST). Unfortunately, we do not have this information
 * here. I thought that I replaced all relevant occurences of
 * SIZE_OF_DESC by SIZE_OF_DIS_DESC, but still ran into strange bugs
 * that suggest I did not.
 * Maybe the assumption that descriptors have the same size
 * if arrays have the same shape is also a problem.
 * Anyways, for the time being, we always add SAC_DISTMEM_size to
 * the descriptor size if SAC_DO_DISTMEM_PTR_DESC is active
 * although that wastes some memory.
 */
#define SIZE_OF_DESC(dim) (FIXED_SIZE_OF_DESC + (dim)*VAR_SIZE_OF_DESC + SAC_DISTMEM_size)
#else /* SAC_DO_DISTMEM_PTR_DESC  */
/* size of the descriptor = (FIXED_SIZE_OF_DESC + dim * VAR_SIZE_OF_DESC) */
#define SIZE_OF_DESC(dim) (FIXED_SIZE_OF_DESC + (dim)*VAR_SIZE_OF_DESC)
#endif /* SAC_DO_DISTMEM_PTR_DESC  */

#ifndef SAC_FORCE_DESC_SIZE
#define SAC_FORCE_DESC_SIZE -1
#endif

#if SAC_FORCE_DESC_SIZE >= 0
#define BYTE_SIZE_OF_DESC(dim) SAC_FORCE_DESC_SIZE
#else /* SAC_FORCE_DESC_SIZE >= 0 */
#define BYTE_SIZE_OF_DESC(dim) (SIZE_OF_DESC (dim) * sizeof (intptr_t))

#if SAC_DO_DISTMEM
/* Size of the descriptor for distributed arrays */
#if SAC_DO_DISTMEM_PTR_DESC
#define SIZE_OF_DIST_DESC(dim) (SIZE_OF_DESC (dim) + SAC_DISTMEM_size)
#else /* SAC_DO_DISTMEM_PTR_DESC */
#define SIZE_OF_DIST_DESC(dim) SIZE_OF_DESC (dim)
#endif /* SAC_DO_DISTMEM_PTR_DESC */

/* Size of the descriptor in bytes for distributable arrays */
#define BYTE_SIZE_OF_DIS_DESC(is_dist, dim)                                              \
    ((is_dist) ? (SIZE_OF_DIST_DESC (dim) * sizeof (intptr_t))                           \
               : (SIZE_OF_DESC (dim) * sizeof (intptr_t)))
#endif /* SAC_DO_DISTMEM */
#endif /* SAC_FORCE_DESC_SIZE >= 0 */

/** reference counter (RC) modes */
/* LOCAL: count locally without any lock */
#define SAC_DESC_RC_MODE_LOCAL 0
/* ASYNC: the async parent, has to use atomic operations */
#define SAC_DESC_RC_MODE_ASYNC 1
/* NORC: rc disabled; e.g. used for the relatively free vars in SPMD execution */
#define SAC_DESC_RC_MODE_NORC 2
/* OTHER: LOCAL or ASYNC */
#define SAC_DESC_RC_MODE_OTHER 3

/* bitmask for storing the RC mode in the descriptor pointer LSB */
#define SAC_RC_PTR_MASK (3)
#define SAC_RC_PTR_NEG_MASK (-1 ^ SAC_RC_PTR_MASK)

#define SAC_DESC_HIDDEN_NONE 0
#define SAC_DESC_HIDDEN_HIDDEN 1
#define SAC_DESC_HIDDEN_NESTED 2

/* offsets inside the descriptor */
#define DESC_OFFSET_RC 0
#define DESC_OFFSET_PARENT 1
#define DESC_OFFSET_RC_MODE 2
#define DESC_OFFSET_DIM 3
#define DESC_OFFSET_SIZE 4
#define DESC_OFFSET_HIDDEN_TYPE 5

#if SAC_DO_CUDA_ALLOC == SAC_CA_cureg || SAC_DO_CUDA_ALLOC == SAC_CA_cualloc
#define DESC_OFFSET_CUDA_PINNED 6
#endif /* SAC_DO_CUDA_ALLOC */

#if SAC_DO_DISTMEM
#define DESC_OFFSET_IS_DIST 6
#define DESC_OFFSET_OFFS 7
#define DESC_OFFSET_FIRST_ELEMS 8
#endif /* SAC_DO_DISTMEM */

#define DESC_OFFSET_SHAPE FIXED_SIZE_OF_DESC

/* offsets in the parent descriptor */
#define PARDESC_OFFSET_NCHILD 0

/*
 * Access hidden info in desc pointer.
 * The hidden data is stored in the lowest bits of the descriptor pointer.
 * They are typically used to store the RC mode, hence this is enabled only
 * for a subset of rc modes.
 */
#if (SAC_RC_METHOD == SAC_RCM_local_norc_ptr)                                            \
  || (SAC_RC_METHOD == SAC_RCM_async_norc_ptr)                                           \
  || (SAC_RC_METHOD == SAC_RCM_local_async_norc_ptr)                                     \
  || (SAC_RC_METHOD == SAC_RCM_SAC_local_async_norc_ptr)
#if 0 /* __alpha__*/

#define SAC_READ_DESC_POINTER(desc, offset)                                              \
    ({                                                                                   \
        intptr_t result;                                                                 \
        asm("ldq_u %0, %2(%1)" : "=r"(result) : "r"(desc), "I"(offset * sizeof (*b)));   \
        result;                                                                          \
    })
#define SAC_WRITE_DESC_POINTER(desc, offset, value)                                      \
    ({ asm("ldq_u %0, %2(%1)" : : "=r"(value), "r"(desc), "I"(offset * sizeof (*b))); })
#define SAC_DESC_HIDDEN_DATA(desc) (((intptr_t)desc) & SAC_RC_PTR_MASK)
#define SAC_DESC_SET_HIDDEN_DATA(desc, val)                                              \
    desc = (SAC_array_descriptor_t *)(((intptr_t)desc) & val)

#else /* __alpha__ */

#define SAC_REAL_DESC(desc)                                                              \
    ((SAC_array_descriptor_t) (((intptr_t)desc) & SAC_RC_PTR_NEG_MASK))
#define SAC_DESC_HIDDEN_DATA(desc) (((intptr_t)desc) & SAC_RC_PTR_MASK)
#define SAC_DESC_SET_HIDDEN_DATA(desc, val)                                              \
    desc = (SAC_array_descriptor_t) ((((intptr_t)desc) & SAC_RC_PTR_NEG_MASK) | val)

#endif

#else

#define SAC_REAL_DESC(desc) (desc)
#define SAC_DESC_HIDDEN_DATA(desc) 0

#endif

#define SAC_REAL_DESC_POINTER(desc, offset) (SAC_REAL_DESC (desc)[offset])

#define DESC_RC(desc) SAC_REAL_DESC_POINTER (desc, DESC_OFFSET_RC)
#define DESC_PARENT(desc) SAC_REAL_DESC_POINTER (desc, DESC_OFFSET_PARENT)
#define DESC_RC_MODE(desc) SAC_REAL_DESC_POINTER (desc, DESC_OFFSET_RC_MODE)
#define DESC_DIM(desc) SAC_REAL_DESC_POINTER (desc, DESC_OFFSET_DIM)
#define DESC_SIZE(desc) SAC_REAL_DESC_POINTER (desc, DESC_OFFSET_SIZE)
#define DESC_HIDDEN_TYPE(desc) SAC_REAL_DESC_POINTER (desc, DESC_OFFSET_HIDDEN_TYPE)
#define DESC_SHAPE(desc, pos) SAC_REAL_DESC_POINTER (desc, (DESC_OFFSET_SHAPE + (pos)))

#if SAC_DO_CUDA_ALLOC == SAC_CA_cureg || SAC_DO_CUDA_ALLOC == SAC_CA_cualloc
#define DESC_CUDA_PINNED(desc) SAC_REAL_DESC_POINTER (desc, DESC_OFFSET_CUDA_PINNED)
#endif /* SAC_DO_CUDA_ALLOC */

#if SAC_DO_DISTMEM
#define DESC_IS_DIST(desc) SAC_REAL_DESC_POINTER (desc, DESC_OFFSET_IS_DIST)
#define DESC_OFFS(desc) SAC_REAL_DESC_POINTER (desc, DESC_OFFSET_OFFS)
#define DESC_FIRST_ELEMS(desc) SAC_REAL_DESC_POINTER (desc, DESC_OFFSET_FIRST_ELEMS)
#define DESC_PTR(desc, dims, rank)                                                       \
    SAC_REAL_DESC_POINTER (desc, (DESC_OFFSET_SHAPE + (dims) + (rank)))
#endif /* SAC_DO_DISTMEM */

/* parent descriptor */
#define PARENT_DESC_NCHILD(desc)                                                         \
    (((SAC_ND_DESC_PARENT_TYPE) (desc))[PARDESC_OFFSET_NCHILD])

/*
 * The following macros are overloaded in mutc_gen.h:
 */
#ifndef SAC_BACKEND_MUTC

#define SAC_ND_GETVAR(nt, name) name
#define SAC_ND_GETVAR_INOUT(nt, name) SAC_ND_GETVAR (nt, name)

#endif

/*
 * The following macros are overloaded in mutc.h:
 */
#ifndef SAC_BACKEND_MUTC

#define SAC_ND_REAL_PARAM(type, name, var_NT) type name
#define SAC_ND_REAL_ARG(name, var_NT, type) name
#define SAC_ND_REAL_ARG_out(name, var_NT, type) name
#define SAC_ND_FUNAP2(name, ...) name (__VA_ARGS__);
#define SAC_ND_DECL_FUN2(name, type, ...) type name (__VA_ARGS__)
#define SAC_ND_DEF_FUN_BEGIN2(name, type, ...)                                           \
    SAC_ND_DECL_FUN2 (name, type, __VA_ARGS__)                                           \
    {
#define SAC_ND_FUN_DEF_END2(...) }
#define SAC_INIT_LOCAL_MEM()
#define SAC_CLEANUP_LOCAL_MEM()

#endif

/*
 * The following macros are replaced by tag-dispatched versions in mutc_rc_gen.h:
 * I (sbs) leave them here in case someone wants to strip out the mutc stuff...
 */
#if 0

#define SAC_ND_PRF_RESTORERC(array, desc)
#define SAC_ND_PRF_2NORC(desc, array)

#endif

#define SAC_NOP(...)

/**********************************
 **********************************
 ***
 *** CAT2, CAT3, CAT4, CAT5 (ND_A_DESC)
 *** CAT6, CAT7, CAT8       (ND_A_RC, ND_A_DIM, ND_A_SIZE, ND_A_SHAPE, ...)
 *** CAT9, CAT10, CAT11, CAT12    (ND_A_DESC_*, ND_A_MIRROR_*)
 ***
 ***/

#define SAC_ND_A_RC_T_MODE(var_NT) SAC_RC_T_MODE (SAC_ND_A_FIELD (var_NT))

#define SAC_ND_A_RC_T_RC(var_NT) SAC_RC_T_RC (SAC_ND_A_FIELD (var_NT))

/******************************************************************************
 *
 * ICMs for retrieving the
 * declared C type
 * ==========================
 *
 * SAC_NT_CBASETYPE( var_NT)
 *
 ******************************************************************************/

#define SAC_NT_CBASETYPE__INT(var_NT) int

#define SAC_NT_CBASETYPE__FLO(var_NT) float

#define SAC_NT_CBASETYPE__DOU(var_NT) double

#define SAC_NT_CBASETYPE__UCH(var_NT) unsigned char

#define SAC_NT_CBASETYPE__BOO(var_NT) bool

#define SAC_NT_CBASETYPE__BYT(var_NT) byte

#define SAC_NT_CBASETYPE__SHO(var_NT) short

#define SAC_NT_CBASETYPE__LON(var_NT) long

#define SAC_NT_CBASETYPE__LLO(var_NT) longlong

#define SAC_NT_CBASETYPE__UBY(var_NT) ubyte

#define SAC_NT_CBASETYPE__USH(var_NT) ushort

#define SAC_NT_CBASETYPE__UIN(var_NT) uint

#define SAC_NT_CBASETYPE__ULO(var_NT) ulong

#define SAC_NT_CBASETYPE__ULL(var_NT) ulonglong

#define SAC_NT_CBASETYPE__UNDEF(var_NT) SAC_ICM_UNDEF ()

/******************************************************************************
 *
 * ICMs to get the format specifier for the
 * declared C type
 * ==========================
 *
 * SAC_NT_PRINT_CBASETYPE( var_NT)
 *
 ******************************************************************************/

#define SAC_NT_PRINT_CBASETYPE__INT(var_NT) "%d"

#define SAC_NT_PRINT_CBASETYPE__FLO(var_NT) "%f"

#define SAC_NT_PRINT_CBASETYPE__DOU(var_NT) "%f"

#define SAC_NT_PRINT_CBASETYPE__UCH(var_NT) "%hhu"

#define SAC_NT_PRINT_CBASETYPE__BOO(var_NT) "%d"

#define SAC_NT_PRINT_CBASETYPE__BYT(var_NT) "%hhd"

#define SAC_NT_PRINT_CBASETYPE__SHO(var_NT) "%hd"

#define SAC_NT_PRINT_CBASETYPE__LON(var_NT) "%ld"

#define SAC_NT_PRINT_CBASETYPE__LLO(var_NT) "%lld"

#define SAC_NT_PRINT_CBASETYPE__UBY(var_NT) "%hhu"

#define SAC_NT_PRINT_CBASETYPE__USH(var_NT) "%hu"

#define SAC_NT_PRINT_CBASETYPE__UIN(var_NT) "%u"

#define SAC_NT_PRINT_CBASETYPE__ULO(var_NT) "%lu"

#define SAC_NT_PRINT_CBASETYPE__ULL(var_NT) "%llu"

#define SAC_NT_PRINT_CBASETYPE__UNDEF(var_NT)

/******************************************************************************
 *
 * ICMs for descriptor access
 * ==========================
 *
 * ND_A_DESC( var_NT) :
 *   accesses the descriptor of the data object
 *
 * ND_A_DESC_DIM( var_NT) :
 *   accesses the dimension of the data object via descriptor(!)
 * ND_A_DESC_SIZE( var_NT) :
 *   accesses the size (number of elements) of the data object via descriptor(!)
 * ND_A_DESC_SHAPE( var_NT, dim) :
 *   accesses a shape component of the data object via descriptor(!)
 *
 * === Additional ICMs for CUDA backend (for alloctors) ===
 * ND_A_DESC_CUDA_PINNED (var_NT)
 *   flag indicating if this memory is CUDA pinned (using either cudaHostRegister or
 *   cudaHostAlloc().
 * ===  Additional ICMs for distributed memory backend ===
 * ND_A_DESC_IS_DIST( var_NT, dim) :
 *   accesses the is-distributed property of the data object via descriptor(!)
 * ND_A_DESC_OFFS( var_NT, dim) :
 *   accesses the offset within the dsm shared segment of the data object via
 *descriptor(!) ND_A_DESC_FIRST_ELEMS( var_NT, dim) : accesses the maximum number of
 *elements of the data object that are owned by each node via descriptor(!)
 *
 * ND_A_MIRROR_DIM( var_NT) :
 *   accesses the dimension of the data object via mirror(!)
 * ND_A_MIRROR_SIZE( var_NT) :
 *   accesses the size of the data object via mirror(!)
 * ND_A_MIRROR_SHAPE( var_NT, dim) :
 *   accesses a shape component of the data object via mirror(!)
 *
 * === Additional ICMs for CUDA backend (for alloctors) ===
 * ND_A_MIRROR_CUDA_PINNED (var_NT)
 *   accesses the CUDA pinned flag of the data object via mirror(!)
 * ===  Additional ICMs for distributed memory backend ===
 * ND_A_MIRROR_IS_DIST( var_NT, dim) :
 *   accesses the is-distributed property of the data object via mirror(!)
 * ND_A_MIRROR_OFFS( var_NT, dim) :
 *   accesses the offset within the dsm shared segment of the data object via mirror(!)
 * ND_A_MIRROR_FIRST_ELEMS( var_NT, dim) :
 *   accesses the maximum number of elements of the data object
 *   that are owned by each node via mirror(!)
 *
 * ND_A_FIELD( var_NT) :
 *   accesses the pointer to the data object (array)
 *   or the data object (scalar) respectively
 *
 * ND_A_RC( var_NT) :
 *   accesses the reference counter
 * ND_A_DIM( var_NT) :
 *   accesses the dimension (via mirror if exists, via descriptor otherwise)
 * ND_A_SIZE( var_NT) :
 *   accesses the size (via mirror if exists, via descriptor otherwise)
 * ND_A_SHAPE( var_NT, dim) :
 *   accesses a shape component (via mirror if exists, via descriptor otherwise)
 *
 * === Additional ICMs for CUDA backend (for alloctors) ===
 * ND_A_CUDA_PINNED (var_NT)
 *   accesses the CUDA pinned flag of the data object
 * ===  Additional ICMs for distributed memory backend ===
 * ND_A_IS_DIST( var_NT, dim) :
 *   accesses the is-distributed property of the data object
 *   (via mirror if exists, via descriptor otherwise)
 * ND_A_OFFS( var_NT, dim) :
 *   accesses the offset within the dsm shared segment of the data object
 *   (via mirror if exists, via descriptor otherwise)
 * ND_A_FIRST_ELEMS( var_NT, dim) :
 *   accesses the maximum number of elements of the data object that are owned by each
 *node (via mirror if exists, via descriptor otherwise)
 *
 ******************************************************************************/

/*
 * SAC_ND_A_DESC implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_A_DESC__UNDEF(var_NT) SAC_ICM_UNDEF ()

#define SAC_ND_A_DESC__NULL(var_NT) NULL

#define SAC_ND_A_DESC__DEFAULT(var_NT) SAC_ND_GETVAR (var_NT, SAC_ND_A_DESC_NAME (var_NT))

#define SAC_ND_A_DESC_NAME__UNDEF(var_NT) SAC_ICM_UNDEF ()

#define SAC_ND_A_DESC_NAME__DEFAULT(var_NT) CAT5 (NT_NAME (var_NT), __desc)

#define SAC_ND_A_DESC_RC_MODE(var_NT) DESC_RC_MODE (SAC_ND_A_DESC (var_NT))

/*
 * Cast as stored as intptr_t
 */
#define SAC_ND_A_DESC_PARENT(var_NT)                                                     \
    ((SAC_ND_DESC_PARENT_TYPE)DESC_PARENT (SAC_ND_A_DESC (var_NT)))

/*
 * SAC_ND_A_DESC_DIM implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_A_DESC_DIM__DEFAULT(var_NT) DESC_DIM (SAC_ND_A_DESC (var_NT))

#define SAC_ND_A_DESC_DIM__UNDEF(var_NT) SAC_ICM_UNDEF ()

/*
 * SAC_ND_A_DESC_SIZE implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_A_DESC_SIZE__DEFAULT(var_NT) DESC_SIZE (SAC_ND_A_DESC (var_NT))

#define SAC_ND_A_DESC_SIZE__UNDEF(var_NT) SAC_ICM_UNDEF ()

/*
 * SAC_ND_A_DESC_SHAPE implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_A_DESC_SHAPE__DEFAULT(var_NT, dim) DESC_SHAPE (SAC_ND_A_DESC (var_NT), dim)

#define SAC_ND_A_DESC_SHAPE__UNDEF(var_NT, dim) SAC_ICM_UNDEF ()

#if SAC_DO_CUDA_ALLOC == SAC_CA_cureg || SAC_DO_CUDA_ALLOC == SAC_CA_cualloc
#define SAC_ND_A_DESC_CUDA_PINNED__DEFAULT(var_NT) DESC_CUDA_PINNED (SAC_ND_A_DESC (var_NT))
#define SAC_ND_A_DESC_CUDA_PINNED__UNDEF(var_NT) SAC_ICM_UNDEF ()
#endif /* SAC_DO_CUDA_ALLOC */

#if SAC_DO_DISTMEM
/*
 * SAC_ND_A_DESC_IS_DIST implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_A_DESC_IS_DIST__DEFAULT(var_NT) DESC_IS_DIST (SAC_ND_A_DESC (var_NT))

#define SAC_ND_A_DESC_IS_DIST__UNDEF(var_NT) SAC_ICM_UNDEF ()

/*
 * SAC_ND_A_DESC_OFFS implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_A_DESC_OFFS__DEFAULT(var_NT) DESC_OFFS (SAC_ND_A_DESC (var_NT))

#define SAC_ND_A_DESC_OFFS__UNDEF(var_NT) SAC_ICM_UNDEF ()

/*
 * SAC_ND_A_DESC_FIRST_ELEMS implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_A_DESC_FIRST_ELEMS__DEFAULT(var_NT)                                       \
    DESC_FIRST_ELEMS (SAC_ND_A_DESC (var_NT))

#define SAC_ND_A_DESC_FIRST_ELEMS__UNDEF(var_NT) SAC_ICM_UNDEF ()

/*
 * SAC_ND_A_DESC_PTR implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_A_DESC_PTR__DEFAULT(var_NT, rank)                                         \
    DESC_PTR (SAC_ND_A_DESC (var_NT), SAC_ND_A_DIM (var_NT), rank)

#define SAC_ND_A_DESC_PTR__UNDEF(var_NT, rank) SAC_ICM_UNDEF ()

#endif /* SAC_DO_DISTMEM */

/*
 * SAC_ND_A_MIRROR_DIM implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_A_MIRROR_DIM__SCL(var_NT) 0

#define SAC_ND_A_MIRROR_DIM__DEFAULT(var_NT) CAT12 (NT_NAME (var_NT), __dim)

/*
 * SAC_ND_A_MIRROR_SIZE implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_A_MIRROR_SIZE__SCL(var_NT) 1

#define SAC_ND_A_MIRROR_SIZE__DEFAULT(var_NT) CAT12 (NT_NAME (var_NT), __sz)

/*
 * SAC_ND_A_MIRROR_SHAPE implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_A_MIRROR_SHAPE__DEFAULT(var_NT, dim)                                      \
    CAT12 (NT_NAME (var_NT), CAT20 (__shp, dim))

#define SAC_ND_A_MIRROR_SHAPE__UNDEF(var_NT, dim) SAC_ICM_UNDEF ()

#if SAC_DO_CUDA_ALLOC == SAC_CA_cureg || SAC_DO_CUDA_ALLOC == SAC_CA_cualloc
#define SAC_ND_A_MIRROR_CUDA_PINNED__SCL(var_NT) 0
#define SAC_ND_A_MIRROR_CUDA_PINNED__DEFAULT(var_NT) CAT12 (NT_NAME (var_NT), __cuda_pinned)
#endif /* SAC_DO_CUDA_ALLOC */

#if SAC_DO_DISTMEM

/*
 * SAC_ND_A_MIRROR_IS_DIST implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_A_MIRROR_IS_DIST__DEFAULT(var_NT) CAT12 (NT_NAME (var_NT), __dm_isDist)

#define SAC_ND_A_MIRROR_IS_DIST__UNDEF(var_NT) SAC_ICM_UNDEF ()

/*
 * SAC_ND_A_MIRROR_OFFS implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_A_MIRROR_OFFS__DEFAULT(var_NT) CAT12 (NT_NAME (var_NT), __dm_offs)

#define SAC_ND_A_MIRROR_OFFS__UNDEF(var_NT) SAC_ICM_UNDEF ()

/*
 * SAC_ND_A_MIRROR_FIRST_ELEMS implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_A_MIRROR_FIRST_ELEMS__DEFAULT(var_NT)                                     \
    CAT12 (NT_NAME (var_NT), __dm_firstElems)

#define SAC_ND_A_MIRROR_FIRST_ELEMS__UNDEF(var_NT) SAC_ICM_UNDEF ()

/*
 * SAC_ND_A_MIRROR_LOCAL_FROM implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_A_MIRROR_LOCAL_FROM__DEFAULT(var_NT)                                      \
    CAT12 (NT_NAME (var_NT), __dm_localFrom)

#define SAC_ND_A_MIRROR_LOCAL_FROM__UNDEF(var_NT) SAC_ICM_UNDEF ()

/*
 * SAC_ND_A_MIRROR_LOCAL_COUNT implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_A_MIRROR_LOCAL_COUNT__DEFAULT(var_NT)                                     \
    CAT12 (NT_NAME (var_NT), __dm_localCount)

#define SAC_ND_A_MIRROR_LOCAL_COUNT__UNDEF(var_NT) SAC_ICM_UNDEF ()

/*
 * SAC_ND_A_MIRROR_PTR_CACHE implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_A_MIRROR_PTR_CACHE__DEFAULT(var_NT) CAT12 (NT_NAME (var_NT), __dm_ptrCache)

#define SAC_ND_A_MIRROR_PTR_CACHE__UNDEF(var_NT) SAC_ICM_UNDEF ()

/*
 * SAC_ND_A_MIRROR_PTR_CACHE_FROM implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_A_MIRROR_PTR_CACHE_FROM__DEFAULT(var_NT)                                  \
    CAT12 (NT_NAME (var_NT), __dm_ptrCacheFrom)

#define SAC_ND_A_MIRROR_PTR_CACHE_FROM__UNDEF(var_NT) SAC_ICM_UNDEF ()

/*
 * SAC_ND_A_MIRROR_PTR_CACHE_COUNT implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_A_MIRROR_PTR_CACHE_COUNT__DEFAULT(var_NT)                                 \
    CAT12 (NT_NAME (var_NT), __dm_ptrCacheCount)

#define SAC_ND_A_MIRROR_PTR_CACHE_COUNT__UNDEF(var_NT) SAC_ICM_UNDEF ()

#endif /* SAC_DO_DISTMEM */

/*
 * SAC_ND_A_FIELD implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_A_FIELD__DEFAULT(var_NT) NT_NAME (var_NT)

#define SAC_ND_A_FIELD__BOXED(var_NT) *SAC_ND_A_FIELD (var_NT)

/*
 * SAC_ND_A_DIM implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_A_DIM__DEFAULT(var_NT) SAC_ND_A_MIRROR_DIM (var_NT)

/*
 * SAC_ND_A_SIZE implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_A_SIZE__DEFAULT(var_NT) SAC_ND_A_MIRROR_SIZE (var_NT)

/*
 * SAC_ND_A_SHAPE implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_A_SHAPE__SCL(var_NT, dim) (-1) /* SAC_ICM_UNDEF() */
                                              /*
                                               * For convenience reasons, SAC_ND_A_SHAPE__SCL is *not* undefined but
                                               * a dummy value. Now, the following code fragment works even for scalars:
                                               *   for (i = 0; i < SAC_ND_A_DIM( var_NT); i++) {
                                               *     ... SAC_ND_A_SHAPE( var_NT, i) ...
                                               *   }
                                               * Unfortunately, however, things will go wrong if SAC_ND_A_SHAPE is used
                                               * on scalars in non-dead code...
                                               */

#define SAC_ND_A_SHAPE__AKS_AKD(var_NT, dim) SAC_ND_A_MIRROR_SHAPE (var_NT, dim)

#define SAC_ND_A_SHAPE__AUD(var_NT, dim) SAC_ND_A_DESC_SHAPE (var_NT, dim)

#if SAC_DO_CUDA_ALLOC == SAC_CA_cureg || SAC_DO_CUDA_ALLOC == SAC_CA_cualloc
#define SAC_ND_A_CUDA_PINNED__DEFAULT(var_NT) SAC_ND_A_MIRROR_CUDA_PINNED (var_NT)
#endif /* SAC_DO_CUDA_ALLOC */

#if SAC_DO_DISTMEM

/*
 * SAC_ND_A_IS_DIST implementations (referenced by sac_std_gen.h)
 * Returns whether the array is allocated in DSM memory.
 */

#define SAC_ND_A_IS_DIST__DIS_DSM(var_NT) SAC_ND_A_MIRROR_IS_DIST (var_NT)

#define SAC_ND_A_IS_DIST__DEFAULT(var_NT) FALSE

/*
 * SAC_ND_A_IS_DSM implementations (referenced by sac_std_gen.h)
 * Returns whether the array is allocated in DSM memory but not distributed.
 */

#define SAC_ND_A_IS_DSM__DSM(var_NT) SAC_ND_A_IS_DIST (var_NT)

#define SAC_ND_A_IS_DSM__DEFAULT(var_NT) FALSE

/*
 * SAC_ND_A_OFFS implementations (referenced by sac_std_gen.h)
 * The default variant returns 0 for use by the _offs_A_ primitive function.
 */

#define SAC_ND_A_OFFS__DIS_DSM(var_NT) SAC_ND_A_MIRROR_OFFS (var_NT)

#define SAC_ND_A_OFFS__DEFAULT(var_NT) 0

/*
 * SAC_ND_A_FIRST_ELEMS implementations (referenced by sac_std_gen.h)
 * The default variant returns 0 for use by the _firstElems_A_ primitive function.
 */

#define SAC_ND_A_FIRST_ELEMS__DIS(var_NT) SAC_ND_A_MIRROR_FIRST_ELEMS (var_NT)

#define SAC_ND_A_FIRST_ELEMS__DEFAULT(var_NT) 0

/*
 * SAC_ND_A_LOCAL_FROM implementations (referenced by sac_std_gen.h)
 * The default variant returns 0 for use by the _firstElems_A_ primitive function.
 */

#define SAC_ND_A_LOCAL_FROM__DIS(var_NT) SAC_ND_A_MIRROR_LOCAL_FROM (var_NT)

#define SAC_ND_A_LOCAL_FROM__DEFAULT(var_NT) 0

/*
 * SAC_ND_A_LOCAL_COUNT implementations (referenced by sac_std_gen.h)
 * The default variant returns 0 for use by the _firstElems_A_ primitive function.
 */

#define SAC_ND_A_LOCAL_COUNT__DIS(var_NT) SAC_ND_A_MIRROR_LOCAL_COUNT (var_NT)

#define SAC_ND_A_LOCAL_COUNT__DEFAULT(var_NT) SAC_ND_A_SIZE (var_NT)

#endif /* SAC_DO_DISTMEM */

/****************
 ****************
 ***
 *** CAT13, CAT14, CAT15 (ND_READ, ND_WRITE)
 *** CAT16, CAT17, CAT18 (ND_WRITE_COPY, ND_WRITE_READ_COPY)
 ***
 ***/

/******************************************************************************
 *
 * ICMs for read/write access
 * ==========================
 *
 * Only these two ICMs should be used to access the elements of an array as
 * they selectively enable boundary checking and cache simulation!
 *
 * ND_READ( from_NT, from_pos) :
 *   read access at specified index position
 * ND_WRITE( to_NT, to_pos) :
 *   write access at specified index position
 * ND_WRITE_READ( to_NT, to_pos, from_NT, from_pos) :
 *   write/read access at specified index positions
 *
 * ND_WRITE_COPY( to_NT, to_pos, expr, copyfun) :
 *   write/copy access at specified index position
 * ND_WRITE_READ_COPY( to_NT, to_pos, from_NT, from_pos, copyfun) :
 *   write/read/copy access at specified index positions
 *
 ******************************************************************************/

#define SAC_ND_WRITE_READ(to_NT, to_pos, from_NT, from_pos)                              \
    {                                                                                    \
        SAC_ND_WRITE (to_NT, to_pos) = SAC_ND_READ (from_NT, from_pos);                  \
    }

#define SAC_ND_WRITE_READ_COPY__DEFAULT(to_NT, to_pos, from_NT, from_pos, copyfun)       \
    SAC_ND_WRITE_COPY (to_NT, to_pos, SAC_ND_READ (from_NT, from_pos), copyfun)

/*
 * SAC_ND_READ implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_READ__SCL(from_NT, from_pos)                                              \
    SAC_ND_GETVAR (from_NT, SAC_ND_A_FIELD (from_NT))

/* This mirrors __DEFAULT for now (step-by-step introduction of bitarrays). */
#define SAC_ND_READ__BITARRAY(from_NT, from_pos)                                         \
    (SAC_TR_AA_PRINT ("read_bit", from_NT, from_pos) SAC_BC_READ (from_NT, from_pos)     \
       SAC_CS_READ_ARRAY (from_NT, from_pos)                                             \
         SAC_ND_GETVAR (from_NT, SAC_ND_A_FIELD (from_NT))[from_pos])

#define SAC_ND_READ__DEFAULT(from_NT, from_pos)                                          \
    (SAC_TR_AA_PRINT ("read", from_NT, from_pos) SAC_BC_READ (from_NT, from_pos)         \
       SAC_DISTMEM_CHECK_IS_NON_DIST_ARR (from_NT) SAC_CS_READ_ARRAY (from_NT, from_pos) \
         SAC_ND_GETVAR (from_NT, SAC_ND_A_FIELD (from_NT))[from_pos])

#define SAC_ND_READ__DSM(from_NT, from_pos)                                              \
    (SAC_TR_AA_FPRINT ("%s read", from_NT, from_pos,                                     \
                       SAC_ND_A_IS_DIST (from_NT) ? "DSM" : "Non-DSM")                   \
       SAC_BC_READ (from_NT, from_pos) SAC_DISTMEM_CHECK_IS_DSM_ARR (from_NT)            \
         SAC_CS_READ_ARRAY (from_NT, from_pos)                                           \
           SAC_ND_GETVAR (from_NT, SAC_ND_A_FIELD (from_NT))[from_pos])

/*
 * Read from a possibly distributed variable, but the read index is local
 * for sure.
 */
#define SAC_ND_READ__DLO(from_NT, from_pos)                                              \
    (SAC_TR_AA_FPRINT ("%s read", from_NT, from_pos,                                     \
                       SAC_ND_A_IS_DIST (from_NT) ? "DLO" : "Non-DLO")                   \
       SAC_BC_READ (from_NT, from_pos) SAC_CS_READ_ARRAY (from_NT, from_pos)             \
         SAC_DISTMEM_CHECK_LOCAL_READ_ALLOWED (&(SAC_ND_GETVAR (from_NT,                 \
                                                                SAC_ND_A_FIELD (         \
                                                                  from_NT))[from_pos]),  \
                                               from_NT, from_pos)                        \
           SAC_DISTMEM_AVOIDED_PTR_CALC_KNOWN_LOCAL_READ_EXPR (from_NT)                  \
             SAC_ND_GETVAR (from_NT, SAC_ND_A_FIELD (from_NT))[from_pos])

/******************************************************************************
 * How does the range check optimization in the SAC_ND_READ__DIS macros work?
 *
 * use a < for an inclusive lower bound and exclusive upper bound
 * use <= for an inclusive lower bound and inclusive upper bound
 * alternatively, if the upper bound is inclusive and you can pre-calculate
 *   upper-lower, simply add + 1 to upper-lower and use the < operator.
 *     if ((unsigned)(number-lower) <= (upper-lower))
 *         in_range(number);
 *
 ******************************************************************************/

#if SAC_DO_DISTMEM_PTR_CACHE

#define SAC_ND_READ__DIS_REMOTE(from_NT, from_pos)                                       \
    ((unsigned)(from_pos - SAC_ND_A_MIRROR_PTR_CACHE_FROM (from_NT))                     \
     >= SAC_ND_A_MIRROR_PTR_CACHE_COUNT (from_NT))                                       \
      ? (SAC_DISTMEM_UPDATE_PTR_CACHE_EXPR (from_NT, from_pos))                          \
      : (SAC_DISTMEM_AVOIDED_PTR_CALC_REMOTE_READ_EXPR () 0),                            \
      SAC_DISTMEM_CHECK_PTR_FROM_CACHE (from_NT, from_pos,                               \
                                        &(SAC_ND_A_MIRROR_PTR_CACHE (                    \
                                          from_NT)[from_pos]))                           \
        SAC_DISTMEM_CHECK_READ_ALLOWED (&(SAC_ND_A_MIRROR_PTR_CACHE (                    \
                                          from_NT)[from_pos]),                           \
                                        from_NT, from_pos)                               \
          SAC_ND_A_MIRROR_PTR_CACHE (from_NT)[from_pos]

#else /* SAC_DO_DISTMEM_PTR_CACHE */

#if SAC_DO_DISTMEM_PTR_DESC

#define SAC_ND_READ__DIS_REMOTE(from_NT, from_pos)                                       \
    SAC_DISTMEM_CHECK_PTR_FROM_DESC (from_NT, from_pos,                                  \
                                     SAC_DISTMEM_ELEM_DESC_POINTER (from_NT, from_pos))  \
    SAC_DISTMEM_CHECK_READ_ALLOWED (SAC_DISTMEM_ELEM_DESC_POINTER (from_NT, from_pos),   \
                                    from_NT, from_pos)                                   \
    *SAC_DISTMEM_ELEM_DESC_POINTER (from_NT, from_pos)

#else /* SAC_DO_DISTMEM_PTR_DESC */

#define SAC_ND_READ__DIS_REMOTE(from_NT, from_pos)                                       \
    SAC_DISTMEM_CHECK_READ_ALLOWED (_SAC_DISTMEM_ELEM_POINTER (SAC_ND_A_OFFS (from_NT),  \
                                                               SAC_NT_CBASETYPE (        \
                                                                 from_NT),               \
                                                               SAC_ND_A_FIRST_ELEMS (    \
                                                                 from_NT),               \
                                                               from_pos),                \
                                    from_NT, from_pos)                                   \
  *SAC_DISTMEM_ELEM_POINTER( SAC_ND_A_OFFS( from_NT),           \
                             SAC_NT_CBASETYPE( from_NT),        \
                             SAC_ND_A_FIRST_ELEMS( from_NT),    \
                             from_pos)                          \
    to_NT, to_pos)

#endif /* SAC_DO_DISTMEM_PTR_DESC */

#endif /* SAC_DO_DISTMEM_PTR_CACHE */

#define SAC_ND_READ__DIS(from_NT, from_pos)                                              \
    (SAC_TR_AA_FPRINT ("%s read", from_NT, from_pos,                                     \
                       SAC_ND_A_IS_DIST (from_NT) ? "DIS" : "Non-DIS")                   \
       SAC_BC_READ (from_NT, from_pos) SAC_CS_READ_ARRAY (from_NT, from_pos) (           \
         ((unsigned)(from_pos - SAC_ND_A_LOCAL_FROM (from_NT))                           \
          >= SAC_ND_A_LOCAL_COUNT (from_NT))                                             \
           ? (SAC_ND_READ__DIS_REMOTE (from_NT, from_pos))                               \
           : (SAC_DISTMEM_CHECK_READ_ALLOWED (&(SAC_ND_GETVAR (from_NT,                  \
                                                               SAC_ND_A_FIELD (          \
                                                                 from_NT))[from_pos]),   \
                                              from_NT, from_pos)                         \
                SAC_DISTMEM_AVOIDED_PTR_CALC_LOCAL_READ_EXPR (from_NT)                   \
                  SAC_ND_GETVAR (from_NT, SAC_ND_A_FIELD (from_NT))[from_pos])))

/*
 * SAC_ND_WRITE implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_WRITE__SCL(to_NT, to_pos) SAC_ND_GETVAR (to_NT, SAC_ND_A_FIELD (to_NT))

/* This mirrors __DEFAULT for now (step-by-step introduction of bitarrays). */
#define SAC_ND_WRITE__BITARRAY(to_NT, to_pos)                                            \
    SAC_TR_AA_PRINT ("write_bit", to_NT, to_pos)                                         \
    SAC_BC_WRITE (to_NT, to_pos)                                                         \
    SAC_CS_WRITE_ARRAY (to_NT, to_pos)                                                   \
    SAC_ND_GETVAR (to_NT, SAC_ND_A_FIELD (to_NT))[to_pos]

#define SAC_ND_WRITE__DEFAULT(to_NT, to_pos)                                             \
    SAC_TR_AA_PRINT ("write", to_NT, to_pos)                                             \
    SAC_BC_WRITE (to_NT, to_pos)                                                         \
    SAC_DISTMEM_CHECK_IS_NON_DIST_ARR (to_NT)                                            \
    SAC_CS_WRITE_ARRAY (to_NT, to_pos)                                                   \
    SAC_ND_GETVAR (to_NT, SAC_ND_A_FIELD (to_NT))[to_pos]

#define SAC_ND_WRITE__DSM(to_NT, to_pos)                                                 \
    SAC_TR_AA_FPRINT ("%s write", to_NT, to_pos,                                         \
                      SAC_ND_A_IS_DIST (to_NT) ? "DSM" : "Non-DSM")                      \
    SAC_BC_WRITE (to_NT, to_pos)                                                         \
    SAC_DISTMEM_CHECK_IS_DSM_ARR (to_NT)                                                 \
    SAC_CS_WRITE_ARRAY (to_NT, to_pos)                                                   \
    SAC_ND_GETVAR (to_NT, SAC_ND_A_FIELD (to_NT))[to_pos]

/*
 * This ICM should be used whenever cache writes are not allowed
 * for performance reasons. That is always the case except for
 * the modarray primitive functions.
 */
#define SAC_ND_WRITE__DIS(to_NT, to_pos)                                                 \
    SAC_TR_AA_FPRINT ("%s write", to_NT, to_pos,                                         \
                      SAC_ND_A_IS_DIST (to_NT) ? "DIS" : "Non-DIS")                      \
    SAC_BC_WRITE (to_NT, to_pos)                                                         \
    SAC_CS_WRITE_ARRAY (to_NT, to_pos)                                                   \
    SAC_DISTMEM_AVOIDED_PTR_CALC_LOCAL_WRITE_EXPR ()                                     \
    SAC_DISTMEM_CHECK_WRITE_ALLOWED (_SAC_DISTMEM_ELEM_POINTER (SAC_ND_A_OFFS (to_NT),   \
                                                                SAC_NT_CBASETYPE (       \
                                                                  to_NT),                \
                                                                SAC_ND_A_FIRST_ELEMS (   \
                                                                  to_NT),                \
                                                                to_pos),                 \
                                     to_NT, to_pos)                                      \
    SAC_ND_GETVAR (to_NT, SAC_ND_A_FIELD (to_NT))[to_pos]

/*
 * This ICM is for distributed arrays and should be used only when cache
 * writes are allowed which is the case in the modarray primitive functions.
 */
#define SAC_ND_WRITE__DCA(to_NT, to_pos)                                                 \
    SAC_TR_AA_PRINT ("DCA write", to_NT, to_pos)                                         \
    SAC_BC_WRITE (to_NT, to_pos)                                                         \
    SAC_CS_WRITE_ARRAY (to_NT, to_pos)                                                   \
    SAC_DISTMEM_CHECK_WRITE_ALLOWED (_SAC_DISTMEM_ELEM_POINTER (SAC_ND_A_OFFS (to_NT),   \
                                                                SAC_NT_CBASETYPE (       \
                                                                  to_NT),                \
                                                                SAC_ND_A_FIRST_ELEMS (   \
                                                                  to_NT),                \
                                                                to_pos),                 \
                                     to_NT, to_pos)                                      \
    *SAC_DISTMEM_ELEM_POINTER (SAC_ND_A_OFFS (to_NT), SAC_NT_CBASETYPE (to_NT),          \
                               SAC_ND_A_FIRST_ELEMS (to_NT), to_pos)

/*
 * SAC_ND_WRITE_COPY implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_WRITE_COPY__NHD(to_NT, to_pos, expr, copyfun)                             \
    {                                                                                    \
        SAC_ND_WRITE (to_NT, to_pos) = expr;                                             \
    }

#define SAC_ND_WRITE_COPY__HID(to_NT, to_pos, expr, copyfun)                             \
    {                                                                                    \
        SAC_ND_WRITE (to_NT, to_pos) = copyfun (expr);                                   \
        SAC_TR_MEM_PRINT (                                                               \
          ("new hidden object at addr: %p", SAC_ND_READ (to_NT, to_pos)))                \
        SAC_TR_INC_HIDDEN_MEMCNT (0)                                                     \
    }

/******************************************************************************
 *
 * ICMs for types
 * =================
 *
 * ND_DESC_TYPE( var_NT)           : type of descriptor
 *
 * ND_TYPE_NT( basetype_NT)        : type implementation
 *     (basetype must be tagged)
 * ND_TYPE( var_NT, basetype)      : type implementation
 *     (basetype not tagged, separate tagged identifier at hand instead)
 * ND_TYPE__<CLASS>( basetype)     : type implementation
 *
 ******************************************************************************/

#define SAC_ND_TYPE_NT(basetype_NT) SAC_ND_TYPE (basetype_NT, NT_NAME (basetype_NT))

/*
 * SAC_ND_DESC_TYPE implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_DESC_TYPE__DEFAULT(var_NT) SAC_array_descriptor_t

#define SAC_ND_DESC_BASETYPE(var_NT) intptr_t

#define SAC_ND_DESC_PARENT_TYPE SAC_array_descriptor_parent_t

#define SAC_ND_DESC_PARENT_BASETYPE intptr_t

/*
 * SAC_ND_TYPE_TAG implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_TYPE_TAG__SCL(var_NT, basetype) SAC_ND_TYPE__SCL (basetype)

#define SAC_ND_TYPE_TAG__DEFAULT(var_NT, basetype) SAC_ND_TYPE__AKS (basetype)

/*
 * SAC_ND_TYPE implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_TYPE__SCL(basetype) basetype

#define SAC_ND_TYPE__AKS(basetype) basetype *

#define SAC_ND_TYPE__AKD(basetype) SAC_ND_TYPE__AKS (basetype)

#define SAC_ND_TYPE__AUD(basetype) SAC_ND_TYPE__AKS (basetype)

/******************************************************************************
 *
 * ICMs for typedefs
 * =================
 *
 * ND_TYPEDEF( var_NT, basetype) : type definition
 *
 ******************************************************************************/

#define SAC_ND_TYPEDEF__HID(var_NT, basetype)                                            \
    typedef SAC_ND_TYPE (var_NT, SAC_hidden) NT_NAME (var_NT);

#define SAC_ND_TYPEDEF__DEFAULT(var_NT, basetype)                                        \
    typedef SAC_ND_TYPE (var_NT, basetype) NT_NAME (var_NT);

/************************
 ************************
 ***
 *** CAT19, CAT20, CAT21
 ***
 ***/

/******************************************************************************
 *
 * ICMs for objdefs
 * ================
 *
 * ND_OBJDEF( var_NT, basetype, sdim, ...shp...) :
 *   declaration of an internal object
 * ND_OBJDEF_EXTERN( var_NT, basetype, sdim) :
 *   declaration of an external object
 *
 ******************************************************************************/

/* ND_OBJDEF( ...)  is a C-ICM */

/* ND_OBJDEF_EXTERN( ...)  is a C-ICM */

/******************************************************************************
 *
 * ICMs for declaration of data objects
 * ====================================
 *
 * ND_DECL( var_NT, basetype, sdim, ...shp...) :
 *   declares a data object
 * ND_DECL_EXTERN( var_NT, basetype, sdim) :
 *   declares an external data object
 *
 * ND_DECL__DESC( var_NT, decoration) :
 *   declares descriptor of a data object
 * ND_DECL__DATA( var_NT, basetype, decoration) :
 *   declares a data object (without mirror and descriptor)
 *
 * ND_DECL__MIRROR( var_NT, sdim, ...shp...) :
 *   declares mirror of a data object
 * ND_DECL__MIRROR_PARAM( var_NT, sdim, ...shp...) :
 *   declares mirror of a function parameter
 * ND_DECL__MIRROR_EXTERN( var_NT, sdim) :
 *   declares mirror of an external data object
 *
 ******************************************************************************/

/* ND_DECL( ...)  is a C-ICM */

/* ND_DECL_EXTERN( ...)  is a C-ICM */

/*
 * Init to 0 to prevent C compiler complaining that the variable
 * may be uninit as it is used to receive the return value
 */

#define SAC_ND_DECL__DATA(var_NT, basetype, decoration)                                  \
    SAC_ND_DECL__DATA_##decoration (var_NT, basetype, decoration)

#define SAC_ND_DECL__DATA_extern(var_NT, basetype, decoration)                           \
    decoration SAC_ND_TYPE (var_NT, basetype) SAC_ND_A_FIELD (var_NT);

#define SAC_ND_DECL__DATA_(var_NT, basetype, decoration)                                 \
    decoration SAC_ND_TYPE (var_NT, basetype) SAC_ND_A_FIELD (var_NT); /*                \
FIXME Do not initialize for the time being, as value 0                                   \
      does not work for SIMD types.                                                      \
= ( SAC_ND_TYPE( var_NT, basetype)) 0;*/

#define SAC_ND_DECL_CONST__DATA(var_NT, basetype, val)                                   \
    const SAC_ND_TYPE (var_NT, basetype) SAC_ND_A_FIELD (var_NT) = val;
/* ND_DECL__MIRROR( ...)  is a C-ICM */

/* ND_DECL__MIRROR_PARAM( ...)  is a C-ICM */

/* ND_DECL__MIRROR_EXTERN( ...)  is a C-ICM */

/*
 * SAC_ND_DECL__DESC implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_DECL__DESC__NONE(var_NT, decoration) SAC_NOTHING ()

/*
 * Init to 0 to prevent C compiler complaining that the variable
 * may be uninit as it is used to receive the return value
 */

#define SAC_ND_DECL__DESC__DEFAULT(var_NT, decoration)                                   \
    SAC_ND_DECL__DESC__DEFAULT_##decoration (var_NT, decoration)

#define SAC_ND_DECL__DESC__DEFAULT_extern(var_NT, decoration)                            \
    decoration SAC_ND_DESC_TYPE (var_NT) SAC_ND_A_DESC (var_NT);

#define SAC_ND_DECL__DESC__DEFAULT_(var_NT, decoration)                                  \
    decoration SAC_ND_DESC_TYPE (var_NT) SAC_ND_A_DESC (var_NT) = NULL;

/******************************************************************************
 *
 * ICMs for descriptor initializing
 * ================================
 *
 * ND_SET__RC( var_NT, rc) :
 *   sets the refcount (in the descriptor) of a data object
 *
 * ND_SET__SHAPE_id( to_NT, to_sdim, shp_NT) :
 * ND_SET__SHAPE_arr( to_NT, dim, ...shp...) :
 *   sets the shape information (descriptor *and* mirror) of a data object
 *
 ******************************************************************************/

/* ND_SET__RC( ...) is defined in one of the other sections below */

/* ND_SET__SHAPE( ...) is a C-ICM */

/******************************************************************************
 *
 * ICMs for passing data objects to functions
 * ==========================================
 *
 * ND_PARAM_( dummy, basetype) :
 *   macro for ... parameters
 *
 * ND_PARAM_in( var_NT, basetype) :
 *   macro for prototyping data as "in" parameter
 *
 * ND_PARAM_in_nodesc( var_NT, basetype) :
 *   macro for prototyping data as "in" parameter without descriptor
 *
 * ND_PARAM_outpw( var_NT, basetype) :
 *   macro for prototyping data as "out" parameter
 *
 * ND_PARAM_out_nodesc( var_NT, basetype) :
 *   macro for prototyping data as "out" parameter without descriptor
 *
 * ND_PARAM_inout( var_NT, basetype) :
 *   macro for prototyping data as "inout" parameter
 *
 * ND_PARAM_inout_nodesc( var_NT, basetype) :
 *   macro for prototyping data as "inout" parameter without descriptor
 *
 * ND_PARAM_inout_nodesc_bx( var_NT, basetype) :
 *   macro for prototyping boxed data as "inout" parameter without descriptor
 *
 * ND_ARG_in( var_NT) :
 *   macro for giving data as "in" parameter
 *
 * ND_ARG_in_nodesc( var_NT) :
 *   macro for giving data as "in" parameter without descriptor
 *
 * ND_ARG_out( var_NT) :
 *   macro for getting data as "out" parameter
 *
 * ND_ARG_out_nodesc( var_NT) :
 *   macro for getting data as "out" parameter without descriptor
 *
 * ND_ARG_inout( var_NT) :
 *   macro for giving data as "inout" parameter
 *
 * ND_ARG_inout_nodesc( var_NT) :
 *   macro for giving data as "inout" argument without descriptor
 *
 * ND_ARG_inout_nodesc_bx( var_NT) :
 *   macro for giving boxed data as "inout" argument without descriptor
 *
 * ND_RET_out( retvar_NT, var_NT) :
 *   macro for returning data
 *
 * ND_RET_inout( retvar_NT, var_NT) :
 *   macro for returning "inout" data
 *
 * ND_DECL_PARAM_inout( var_NT, basetype) :
 *   macro for declaration of "inout" parameter
 *
 ******************************************************************************/

/* creates name for formal function parameter */
#define SAC_NAMEP(name) CAT0 (name, __p)

/* needed for T_dots parameters (ATG_notag) */
#define SAC_ND_PARAM_(dummy, basetype) basetype

#define SAC_ND_PARAM_in_nodesc(var_NT, basetype)                                         \
    SAC_ND_REAL_PARAM (SAC_ND_TYPE (var_NT, basetype), SAC_ND_A_FIELD (var_NT), var_NT)

#define SAC_ND_PARAM_in_justdesc(var_NT, type)                                           \
    SAC_ND_REAL_PARAM (SAC_ND_DESC_TYPE (var_NT), SAC_ND_A_DESC_NAME (var_NT), var_NT)

#define SAC_ND_PARAM_out_nodesc(var_NT, basetype)                                        \
    SAC_ND_REAL_PARAM (SAC_ND_TYPE (var_NT, basetype) *,                                 \
                       SAC_NAMEP (SAC_ND_A_FIELD (var_NT)), SET_NT_REG (INT, var_NT))

#define SAC_ND_PARAM_inout(var_NT, basetype) SAC_ND_PARAM_out (var_NT, basetype)

#define SAC_ND_PARAM_inout_nodesc(var_NT, basetype)                                      \
    SAC_ND_PARAM_out_nodesc (var_NT, basetype)

#define SAC_ND_PARAM_inout_nodesc_bx(var_NT, basetype)                                   \
    SAC_ND_PARAM_in_nodesc (var_NT, basetype)

#define SAC_ND_ARG_in_nodesc(var_NT, type)                                               \
    SAC_ND_REAL_ARG (SAC_ND_GETVAR (var_NT, SAC_ND_A_FIELD (var_NT)), var_NT,            \
                     SAC_ND_TYPE (var_NT, type))

#define SAC_ND_ARG_in_justdesc(var_NT, type)                                             \
    SAC_ND_REAL_ARG (SAC_ND_A_DESC (var_NT), var_NT, SAC_ND_DESC_TYPE (var_NT))

#define SAC_ND_ARG_out_nodesc(var_NT, type)                                              \
    SAC_ND_REAL_ARG_out (&SAC_ND_GETVAR (var_NT, SAC_ND_A_FIELD (var_NT)), var_NT,       \
                         SAC_ND_TYPE (var_NT, type))

#define SAC_ND_ARG_inout(var_NT, type) SAC_ND_ARG_out (var_NT, type)

#define SAC_ND_ARG_inout_nodesc(var_NT, type) SAC_ND_ARG_out_nodesc (var_NT, type)

#define SAC_ND_ARG_inout_nodesc_bx(var_NT, type) SAC_ND_ARG_in_nodesc (var_NT, type)

#define SAC_ND_RET_inout(retvar_NT, var_NT) SAC_ND_RET_out (retvar_NT, var_NT)

/*
 * SAC_ND_PARAM_in implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_PARAM_in__NODESC(var_NT, basetype)                                        \
    SAC_ND_PARAM_in_nodesc (var_NT, basetype)

#define SAC_ND_PARAM_in__DESC(var_NT, basetype)                                          \
    SAC_ND_PARAM_in_nodesc (var_NT, basetype),                                           \
      SAC_ND_REAL_PARAM (SAC_ND_DESC_TYPE (var_NT), SAC_ND_A_DESC_NAME (var_NT),         \
                         SET_NT_REG (INT, var_NT))

/*
 * SAC_ND_PARAM_out implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_PARAM_out__NODESC(var_NT, basetype)                                       \
    SAC_ND_PARAM_out_nodesc (var_NT, basetype)

#define SAC_ND_PARAM_out__DESC(var_NT, basetype)                                         \
    SAC_ND_PARAM_out_nodesc (var_NT, basetype),                                          \
      SAC_ND_REAL_PARAM (SAC_ND_DESC_TYPE (var_NT) *,                                    \
                         SAC_NAMEP (SAC_ND_A_DESC_NAME (var_NT)),                        \
                         SET_NT_REG (INT, var_NT))

/*
 * SAC_ND_ARG_in implementations (referenced by sac_std_gen.h)
 */
#define SAC_ND_ARG_in__NODESC(var_NT, type) SAC_ND_ARG_in_nodesc (var_NT, type)

#define SAC_ND_ARG_in__DESC(var_NT, type)                                                \
    SAC_ND_ARG_in_nodesc (var_NT, type),                                                 \
      SAC_ND_REAL_ARG (SAC_ND_A_DESC (var_NT), SET_NT_REG (INT, var_NT),                 \
                       SAC_ND_DESC_TYPE (var_NT))

/*
 * SAC_ND_ARG_out implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_ARG_out__NODESC(var_NT, type) SAC_ND_ARG_out_nodesc (var_NT, type *)

#define SAC_ND_ARG_out__DESC(var_NT, type)                                               \
    SAC_ND_ARG_out_nodesc (var_NT, type *),                                              \
      SAC_ND_REAL_ARG_out (&SAC_ND_A_DESC (var_NT), SET_NT_REG (INT, var_NT),            \
                           SAC_ND_DESC_TYPE (var_NT) *)
/*
 * SAC_ND_RET_out implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_RET_out__NODESC(retvar_NT, var_NT)                                        \
    {                                                                                    \
        *SAC_ND_GETVAR_INOUT (retvar_NT, SAC_NAMEP (SAC_ND_A_FIELD (retvar_NT)))         \
          = SAC_ND_GETVAR (var_NT, SAC_ND_A_FIELD (var_NT));                             \
    }

#define SAC_ND_RET_out__DESC(retvar_NT, var_NT)                                          \
    {                                                                                    \
        *SAC_ND_GETVAR_INOUT (retvar_NT, SAC_NAMEP (SAC_ND_A_FIELD (retvar_NT)))         \
          = SAC_ND_GETVAR (var_NT, SAC_ND_A_FIELD (var_NT));                             \
        *SAC_ND_GETVAR_INOUT (retvar_NT, SAC_NAMEP (SAC_ND_A_DESC_NAME (retvar_NT)))     \
          = SAC_ND_A_DESC (var_NT);                                                      \
    }

/*
 * SAC_ND_DECL_PARAM_inout implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_DECL_PARAM_inout__NODESC(var_NT, basetype)                                \
    SAC_ND_TYPE (var_NT, basetype)                                                       \
    SAC_ND_A_FIELD (var_NT)                                                              \
      = *SAC_ND_GETVAR_INOUT (var_NT, SAC_NAMEP (SAC_ND_A_FIELD (var_NT)));

#define SAC_ND_DECL_PARAM_inout__DESC(var_NT, basetype)                                  \
    SAC_ND_TYPE (var_NT, basetype)                                                       \
    SAC_ND_A_FIELD (var_NT)                                                              \
      = *SAC_ND_GETVAR_INOUT (var_NT, SAC_NAMEP (SAC_ND_A_FIELD (var_NT)));              \
    SAC_ND_DESC_TYPE (var_NT)                                                            \
    SAC_ND_A_DESC (var_NT)                                                               \
      = *SAC_ND_GETVAR_INOUT (var_NT, SAC_NAMEP (SAC_ND_A_DESC (var_NT)));

/******************************************************************************
 *
 * ICMs for allocation of data objects
 * ===================================
 *
 * ND_ALLOC( var_NT, rc, dim, set_shape_icm) :
 *   allocates a data object (no initialization apart from descriptor!)
 *
 * ND_ALLOC__DESC( var_NT, dim) :
 *   allocates memory for descriptor (no initialization apart from dimension!)
 *
 * ND_ALLOC__DATA( var_NT) :
 *   allocates memory for a data object (without descriptor, no initialization)
 *
 * ND_ALLOC__DESC_AND_DATA( var_NT, dim) :
 *   allocates memory for descriptor and data object (no init. apart from dim.!)
 *   This ICM is needed for decriptor allocation optimization (DAO)
 *
 * ND_CHECK_REUSE( to_NT, to_sdim, from_NT, copyfun) :
 *   tries to reuse old data object for new, copies if impossible
 *
 ******************************************************************************/

/* ND_CHECK_REUSE( ...)  is a C-ICM */

/* ND_CHECK_RESIZE( ...) is a C-ICM */

/*
 * DAO: descriptor and data vector are allocated together.
 *
 * DAO is possible for SCL and AKS only!  In order to allocate the data vector,
 * its size / the shape of the array is needed. If the shape is not statically,
 * known it must be computed and stored in the decriptor first!
 */

/*
 * For the time being the ND_ALLOC ICM is not used :-( because 'set_shape_icm'
 * is usually a C-ICM but this macro support H-ICMs only.  SAC_ND_ALLOC_BEGIN
 * and SAC_ND_ALLOC_END are used instead.
 */

/*
 * CUDA allocator specifc ICM
 */
#if SAC_DO_CUDA_ALLOC == SAC_CA_cureg || SAC_DO_CUDA_ALLOC == SAC_CA_cualloc
#define SAC_ND_CUDA_PIN(var_NT, basetype)                                                \
     if (SAC_ND_A_MIRROR_CUDA_PINNED (var_NT) == 0) {                                    \
        SAC_TR_GPU_PRINT ("Pinning " NT_STR (var_NT));                                   \
        cudaHostRegister (SAC_ND_A_FIELD (var_NT),                                       \
                          SAC_ND_A_MIRROR_SIZE (var_NT) * sizeof (basetype),             \
                          cudaHostRegisterPortable);                                     \
        SAC_CUDA_GET_LAST_ERROR_COND("GPU MALLOC", cudaErrorHostMemoryAlreadyRegistered);\
        SAC_ND_A_MIRROR_CUDA_PINNED (var_NT) = 1;                                        \
     }
#define SAC_ND_CUDA_UNPIN(var_NT)                                                        \
     if (SAC_ND_A_MIRROR_CUDA_PINNED (var_NT) != 0) {                                    \
        SAC_TR_GPU_PRINT ("Unpinning " NT_STR (var_NT));                                 \
        cudaHostUnregister (SAC_ND_A_FIELD (var_NT));                                    \
        SAC_CUDA_GET_LAST_ERROR_COND("GPU FREE", cudaErrorHostMemoryNotRegistered);      \
        SAC_ND_A_MIRROR_CUDA_PINNED (var_NT) = 0;                                        \
     }
#else
#define SAC_ND_CUDA_PIN(var_NT, basetype) SAC_NOOP ();
#define SAC_ND_CUDA_UNPIN(var_NT) SAC_NOOP ();
#endif /* SAC_DO_CUDA_ALLOC */

/*
 * SAC_ND_ALLOC implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_ALLOC__DAO(var_NT, rc, dim, set_shape_icm)                                \
    {                                                                                    \
        SAC_ND_ALLOC__DESC_AND_DATA (var_NT, dim)                                        \
        SAC_ND_INIT__RC (var_NT, rc)                                                     \
        set_shape_icm                                                                    \
    }

#define SAC_ND_ALLOC__NO_DAO(var_NT, rc, dim, set_shape_icm)                             \
    {                                                                                    \
        SAC_ND_ALLOC__DESC (var_NT, dim)                                                 \
        SAC_ND_INIT__RC (var_NT, rc)                                                     \
        set_shape_icm SAC_ND_ALLOC__DATA (var_NT)                                        \
    }

/*
 * SAC_ND_ALLOC_BEGIN implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_ALLOC_BEGIN__DAO(var_NT, rc, dim, basetype)                               \
    {                                                                                    \
        SAC_ND_ALLOC__DESC_AND_DATA (var_NT, dim, basetype)                              \
        SAC_ND_INIT__RC (var_NT, rc)                                                     \

#define SAC_ND_ALLOC_BEGIN__NO_DAO(var_NT, rc, dim, basetype)                            \
    {                                                                                    \
        SAC_ND_ALLOC__DESC (var_NT, dim)

/*
 * SAC_ND_ALLOC_END implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_ALLOC_END__DAO(var_NT, rc, dim, basetype) }

#define SAC_ND_ALLOC_END__NO_DAO__DEFAULT(var_NT, rc, dim, basetype)                     \
    SAC_ND_ALLOC__DATA_BASETYPE (var_NT, basetype)                                       \
    SAC_ND_INIT__RC (var_NT, rc)                                                         \
    }

/*
 * SAC_ND_REALLOC_BEGIN implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_REALLOC_BEGIN__DAO(dest_NT, src_NT, rc, dim, basetype)                    \
    {                                                                                    \
        SAC_ND_A_DESC (dest_NT) = SAC_ND_A_DESC (src_NT);                                \
        SAC_ND_A_FIELD (src_NT) = SAC_ND_A_FIELD (dest_NT)                               \
          = realloc (SAC_ND_A_FIELD (src_NT),                                            \
                     (SAC_ND_A_SIZE (dest_NT) * sizeof (*SAC_ND_A_FIELD (dest_NT))));    \
        SAC_ND_SET__RC (dest_NT, rc)

#define SAC_ND_REALLOC_BEGIN__NO_DAO(dest_NT, src_NT, rc, dim, basetype)                 \
    {                                                                                    \
        SAC_ND_A_DESC (dest_NT) = SAC_ND_A_DESC (src_NT);

/*
 * SAC_ND_REALLOC_END implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_REALLOC_END__DAO(dest_NT, src_NT, rc, dim, basetype) }

#define SAC_ND_REALLOC_END__NO_DAO(dest_NT, src_NT, rc, dim, basetype)                   \
    SAC_ND_A_FIELD (src_NT) = SAC_ND_A_FIELD (dest_NT)                                   \
      = realloc (SAC_ND_A_FIELD (src_NT),                                                \
                 (SAC_ND_A_SIZE (dest_NT) * sizeof (*SAC_ND_A_FIELD (dest_NT))));        \
    SAC_ND_SET__RC (dest_NT, rc)                                                         \
    }

/*
 * SAC_ND_ALLOC_DESC implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_ALLOC__DESC__NOOP(var_NT, dim) SAC_NOOP ()

/* Overloaded in mutc */
#define SAC_ND_ALLOC__DESC__FIXED(var_NT, dim) SAC_ND_ALLOC__DESC__FIXED_C99 (var_NT, dim)

/*
 * SAC_ND_ALLOC__DESC__FIXED_C99 implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_ALLOC__DESC__FIXED_C99__DEFAULT(var_NT, dim)                              \
    {                                                                                    \
        SAC_ASSURE_TYPE ((dim == SAC_ND_A_MIRROR_DIM (var_NT)),                          \
                         ("Inconsistant dimension for array %s found!",                  \
                          NT_STR (var_NT)));                                             \
        SAC_HM_MALLOC_FIXED_SIZE (SAC_ND_A_DESC (var_NT),                                \
                                  BYTE_SIZE_OF_DESC (SAC_ND_A_MIRROR_DIM (var_NT)),      \
                                  SAC_ND_DESC_BASETYPE (var_NT))                         \
        DESC_RC (SAC_ND_A_DESC (var_NT)) = 0;                                            \
        DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) = SAC_DESC_RC_MODE_LOCAL;                  \
        DESC_PARENT (SAC_ND_A_DESC (var_NT)) = 0;                                        \
        SAC_TR_MEM_PRINT (("ND_ALLOC__DESC( %s, %s) at addr: %p", NT_STR (var_NT), #dim, \
                           SAC_ND_A_DESC (var_NT)))                                      \
        SAC_PF_MEM_INC_ALLOC_DESC (BYTE_SIZE_OF_DESC (SAC_ND_A_MIRROR_DIM (var_NT)))     \
    }

#define SAC_ND_ALLOC__DESC__FIXED_C99__DIS(var_NT, dim)                                  \
    {                                                                                    \
        SAC_ASSURE_TYPE ((dim == SAC_ND_A_MIRROR_DIM (var_NT)),                          \
                         ("Inconsistant dimension for array %s found!",                  \
                          NT_STR (var_NT)));                                             \
        SAC_HM_MALLOC_FIXED_SIZE (SAC_ND_A_DESC (var_NT),                                \
                                  BYTE_SIZE_OF_DIS_DESC (SAC_ND_A_IS_DIST (var_NT),      \
                                                         SAC_ND_A_MIRROR_DIM (var_NT)),  \
                                  SAC_ND_DESC_BASETYPE (var_NT))                         \
        DESC_RC (SAC_ND_A_DESC (var_NT)) = 0;                                            \
        DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) = SAC_DESC_RC_MODE_LOCAL;                  \
        DESC_PARENT (SAC_ND_A_DESC (var_NT)) = 0;                                        \
        SAC_TR_MEM_PRINT (("ND_ALLOC__DESC( %s, %s) at addr: %p", NT_STR (var_NT), #dim, \
                           SAC_ND_A_DESC (var_NT)))                                      \
        SAC_PF_MEM_INC_ALLOC_DESC (BYTE_SIZE_OF_DIS_DESC (SAC_ND_A_IS_DIST (var_NT),     \
                                                          SAC_ND_A_MIRROR_DIM (var_NT))) \
    }

/* Overloaded in mutc */
#define SAC_ND_ALLOC__DESC__AUD(var_NT, dim) SAC_ND_ALLOC__DESC__AUD_C99 (var_NT, dim)

/*
 * SAC_ND_ALLOC__DESC__AUD_C99 implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_ALLOC__DESC__AUD_C99__DEFAULT(var_NT, dim)                                \
    {                                                                                    \
        SAC_ASSURE_TYPE ((dim >= 0),                                                     \
                         ("Illegal dimension for array %s found!", NT_STR (var_NT)));    \
        SAC_HM_MALLOC (SAC_ND_A_DESC (var_NT), BYTE_SIZE_OF_DESC (dim),                  \
                       SAC_ND_DESC_BASETYPE (var_NT))                                    \
        DESC_RC (SAC_ND_A_DESC (var_NT)) = 0;                                            \
        DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) = SAC_DESC_RC_MODE_LOCAL;                  \
        DESC_PARENT (SAC_ND_A_DESC (var_NT)) = 0;                                        \
        SAC_TR_MEM_PRINT (("ND_ALLOC__DESC( %s, %s) at addr: %p", NT_STR (var_NT), #dim, \
                           SAC_ND_A_DESC (var_NT)))                                      \
        SAC_PF_MEM_INC_ALLOC_DESC (BYTE_SIZE_OF_DESC (dim))                              \
        SAC_ND_A_DESC_DIM (var_NT) = SAC_ND_A_MIRROR_DIM (var_NT) = dim;                 \
    }

#define SAC_ND_ALLOC__DESC__AUD_C99__DIS(var_NT, dim)                                    \
    {                                                                                    \
        SAC_ASSURE_TYPE ((dim >= 0),                                                     \
                         ("Illegal dimension for array %s found!", NT_STR (var_NT)));    \
        SAC_HM_MALLOC (SAC_ND_A_DESC (var_NT),                                           \
                       BYTE_SIZE_OF_DIS_DESC (SAC_ND_A_IS_DIST (var_NT), dim),           \
                       SAC_ND_DESC_BASETYPE (var_NT))                                    \
        DESC_RC (SAC_ND_A_DESC (var_NT)) = 0;                                            \
        DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) = SAC_DESC_RC_MODE_LOCAL;                  \
        DESC_PARENT (SAC_ND_A_DESC (var_NT)) = 0;                                        \
        SAC_TR_MEM_PRINT (("ND_ALLOC__DESC( %s, %s) at addr: %p", NT_STR (var_NT), #dim, \
                           SAC_ND_A_DESC (var_NT)))                                      \
        SAC_PF_MEM_INC_ALLOC_DESC (                                                      \
          BYTE_SIZE_OF_DIS_DESC (SAC_ND_A_IS_DIST (var_NT), dim))                        \
        SAC_ND_A_DESC_DIM (var_NT) = SAC_ND_A_MIRROR_DIM (var_NT) = dim;                 \
    }

/*
 * SAC_DESC_ALLOC implementations (referenced by sac_std_gen.h)
 */

#define SAC_DESC_ALLOC__DEFAULT(ndesc, dim)                                              \
    {                                                                                    \
        SAC_ASSURE_TYPE ((dim >= 0),                                                     \
                         ("Illegal dimension for array %s found!", NT_STR (var_NT)));    \
        SAC_HM_MALLOC (ndesc, BYTE_SIZE_OF_DESC (dim), SAC_ND_DESC_BASETYPE (ndesc))     \
        DESC_RC (ndesc) = 0;                                                             \
        DESC_RC_MODE (ndesc) = SAC_DESC_RC_MODE_LOCAL;                                   \
        DESC_PARENT (ndesc) = 0;                                                         \
        DESC_DIM (ndesc) = dim;                                                          \
    }

#define SAC_DESC_ALLOC__DIS(ndesc, dim)                                                  \
    {                                                                                    \
        SAC_ASSURE_TYPE ((dim >= 0),                                                     \
                         ("Illegal dimension for array %s found!", NT_STR (var_NT)));    \
        SAC_HM_MALLOC (ndesc, BYTE_SIZE_OF_DIS_DESC (SAC_ND_A_IS_DIST (var_NT), dim),    \
                       SAC_ND_DESC_BASETYPE (ndesc))                                     \
        DESC_RC (ndesc) = 0;                                                             \
        DESC_RC_MODE (ndesc) = SAC_DESC_RC_MODE_LOCAL;                                   \
        DESC_PARENT (ndesc) = 0;                                                         \
        DESC_DIM (ndesc) = dim;                                                          \
    }

#define SAC_ND_ALLOC__DESC__PARENT(var_NT, dim)                                          \
    SAC_HM_MALLOC_AS_SCALAR (DESC_PARENT (SAC_ND_A_DESC (var_NT)),                       \
                             SAC_DESC_PARENT_T_SIZE, SAC_ND_DESC_PARENT_BASETYPE)

#define SAC_DESC_ALLOC_PARENT(ndesc, dim)                                                \
    SAC_HM_MALLOC_AS_SCALAR (DESC_PARENT (ndesc), SAC_DESC_PARENT_T_SIZE,                \
                             SAC_ND_DESC_PARENT_BASETYPE)

#define SAC_ND_ALLOC__DESC__NOOP_BASETYPE(var_NT, dim, basetype)                         \
    SAC_ND_ALLOC__DESC__NOOP (var_NT, dim)

#define SAC_ND_ALLOC__DESC__FIXED_BASETYPE(var_NT, dim, basetype)                        \
    SAC_ND_ALLOC__DESC__FIXED (var_NT, dim)

#define SAC_ND_ALLOC__DESC__AUD_BASETYPE(var_NT, dim, basetype)                          \
    SAC_ND_ALLOC__DESC__AUD_C99 (var_NT, dim)

#define SAC_ND_A_COPY_DESC(new, old)                                                     \
    SAC_ND_ALLOC__DESC (new, SAC_ND_A_DIM (old));                                        \
    SAC_ND_A_COPY_OVER_DESC (new, old);

#define SAC_DESC_COPY_DESC(newd, oldd)                                                   \
    SAC_DESC_ALLOC (newd, DESC_DIM (oldd));                                              \
    SAC_DESC_COPY_OVER_DESC (newd, oldd);

#define SAC_ND_A_COPY_OVER_DESC(new, old)                                                \
    memcpy (SAC_ND_A_DESC (new), SAC_ND_A_DESC (old),                                    \
            BYTE_SIZE_OF_DESC (SAC_ND_A_DIM (old)));

#define SAC_DESC_COPY_OVER_DESC(newd, oldd)                                              \
    memcpy (newd, oldd, BYTE_SIZE_OF_DESC (DESC_DIM (oldd)));

#define SAC_ND_ALLOC__DATA__NOOP(var_NT) SAC_NOOP ()

#define SAC_ND_ALLOC__DATA__AKS(var_NT)                                                  \
    {                                                                                    \
        SAC_HM_MALLOC_FIXED_SIZE (SAC_ND_A_FIELD (var_NT),                               \
                                  SAC_ND_A_SIZE (var_NT)                                 \
                                    * sizeof (*SAC_ND_A_FIELD (var_NT)),                 \
                                  void)                                                  \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_ALLOC__DATA( %s) at addr: %p", NT_STR (var_NT), SAC_ND_A_FIELD (var_NT))) \
        SAC_TR_INC_ARRAY_MEMCNT (SAC_ND_A_SIZE (var_NT))                                 \
        SAC_PF_MEM_INC_ALLOC (SAC_ND_A_SIZE (var_NT)                                     \
                              * sizeof (*SAC_ND_A_FIELD (var_NT)))                       \
        SAC_CS_REGISTER_ARRAY (var_NT)                                                   \
    }

#define SAC_ND_ALLOC__DATA__AKD_AUD(var_NT)                                              \
    {                                                                                    \
        SAC_HM_MALLOC (SAC_ND_A_FIELD (var_NT),                                          \
                       SAC_ND_A_SIZE (var_NT) * sizeof (*SAC_ND_A_FIELD (var_NT)), void) \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_ALLOC__DATA( %s) at addr: %p", NT_STR (var_NT), SAC_ND_A_FIELD (var_NT))) \
        SAC_TR_INC_ARRAY_MEMCNT (SAC_ND_A_SIZE (var_NT))                                 \
        SAC_PF_MEM_INC_ALLOC (SAC_ND_A_SIZE (var_NT)                                     \
                              * sizeof (*SAC_ND_A_FIELD (var_NT)))                       \
        SAC_CS_REGISTER_ARRAY (var_NT)                                                   \
    }

#define SAC_ND_ALLOC__DATA__AKS_DIS(var_NT)                                              \
    SAC_ND_ALLOC__DATA_BASETYPE__AKS_DIS (var_NT, SAC_NT_CBASETYPE (var_NT))

#define SAC_ND_ALLOC__DATA__AKD_AUD_DIS(var_NT)                                          \
    SAC_ND_ALLOC__DATA_BASETYPE__AKD_AUD_DIS (var_NT, SAC_NT_CBASETYPE (var_NT))

#define SAC_ND_ALLOC__DATA_BASETYPE__NOOP(var_NT, basetype) SAC_NOOP ()

#define SAC_ND_ALLOC__DATA_BASETYPE__AKS(var_NT, basetype)                               \
    {                                                                                    \
        SAC_HM_MALLOC_FIXED_SIZE (SAC_ND_A_FIELD (var_NT),                               \
                                  SAC_ND_A_SIZE (var_NT)                                 \
                                    * sizeof (*SAC_ND_A_FIELD (var_NT)),                 \
                                  basetype)                                              \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_ALLOC__DATA( %s) at addr: %p", NT_STR (var_NT), SAC_ND_A_FIELD (var_NT))) \
        SAC_TR_INC_ARRAY_MEMCNT (SAC_ND_A_SIZE (var_NT))                                 \
        SAC_PF_MEM_INC_ALLOC (SAC_ND_A_SIZE (var_NT)                                     \
                              * sizeof (*SAC_ND_A_FIELD (var_NT)))                       \
        SAC_CS_REGISTER_ARRAY (var_NT)                                                   \
    }

#define SAC_ND_ALLOC__DATA_BASETYPE__AKD_AUD(var_NT, basetype)                           \
    {                                                                                    \
        SAC_HM_MALLOC (SAC_ND_A_FIELD (var_NT),                                          \
                       SAC_ND_A_SIZE (var_NT) * sizeof (*SAC_ND_A_FIELD (var_NT)),       \
                       basetype)                                                         \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_ALLOC__DATA( %s) at addr: %p", NT_STR (var_NT), SAC_ND_A_FIELD (var_NT))) \
        SAC_TR_INC_ARRAY_MEMCNT (SAC_ND_A_SIZE (var_NT))                                 \
        SAC_PF_MEM_INC_ALLOC (SAC_ND_A_SIZE (var_NT)                                     \
                              * sizeof (*SAC_ND_A_FIELD (var_NT)))                       \
        SAC_CS_REGISTER_ARRAY (var_NT)                                                   \
    }

#define SAC_ND_ALLOC__DATA_BASETYPE__AKS_DIS(var_NT, basetype)                           \
    {                                                                                    \
        SAC_DISTMEM_RECHECK_IS_DIST (var_NT)                                             \
        if (SAC_ND_A_IS_DIST (var_NT)) {                                                 \
            SAC_ND_A_DESC_FIRST_ELEMS (var_NT) = SAC_ND_A_MIRROR_FIRST_ELEMS (var_NT)    \
              = SAC_DISTMEM_DET_MAX_ELEMS_PER_NODE (SAC_ND_A_SIZE (var_NT),              \
                                                    SAC_ND_A_SHAPE (var_NT, 0));         \
            SAC_DISTMEM_HM_MALLOC_FIXED_SIZE (SAC_ND_A_FIELD (var_NT),                   \
                                              SAC_ND_A_MIRROR_FIRST_ELEMS (var_NT)       \
                                                * sizeof (basetype),                     \
                                              basetype);                                 \
            SAC_TR_MEM_PRINT (("ND_ALLOC__DATA_DIS( %s) at addr: %p", NT_STR (var_NT),   \
                               SAC_ND_A_FIELD (var_NT)))                                 \
            SAC_TR_INC_ARRAY_MEMCNT (SAC_ND_A_FIRST_ELEMS (var_NT))                      \
            SAC_PF_MEM_INC_ALLOC (SAC_ND_A_FIRST_ELEMS (var_NT) * sizeof (basetype))     \
            SAC_ND_A_DESC_OFFS (var_NT) = SAC_ND_A_MIRROR_OFFS (var_NT)                  \
              = SAC_DISTMEM_DET_OFFS (SAC_ND_A_FIELD (var_NT));                          \
            SAC_CS_REGISTER_ARRAY (var_NT)                                               \
            SAC_DISTMEM_MOVE_PTR_FOR_WRITES (SAC_ND_A_FIELD (var_NT),                    \
                                             SAC_ND_A_FIRST_ELEMS (var_NT))              \
            SAC_DISTMEM_INIT_PTR_DESC (var_NT)                                           \
        } else {                                                                         \
            SAC_ND_ALLOC__DATA_BASETYPE__AKS (var_NT, basetype)                          \
        }                                                                                \
        SAC_ND_A_MIRROR_LOCAL_FROM (var_NT) = SAC_DISTMEM_DET_LOCAL_FROM (var_NT);       \
        SAC_ND_A_MIRROR_LOCAL_COUNT (var_NT) = SAC_DISTMEM_DET_LOCAL_COUNT (var_NT);     \
        SAC_ND_A_MIRROR_PTR_CACHE_FROM (var_NT) = 0;                                     \
        SAC_ND_A_MIRROR_PTR_CACHE_COUNT (var_NT) = 0;                                    \
    }

#define SAC_ND_ALLOC__DATA_BASETYPE__AKD_AUD_DIS(var_NT, basetype)                       \
    {                                                                                    \
        if (SAC_ND_A_IS_DIST (var_NT)) {                                                 \
            SAC_ND_A_DESC_FIRST_ELEMS (var_NT) = SAC_ND_A_MIRROR_FIRST_ELEMS (var_NT)    \
              = SAC_DISTMEM_DET_MAX_ELEMS_PER_NODE (SAC_ND_A_SIZE (var_NT),              \
                                                    SAC_ND_A_SHAPE (var_NT, 0));         \
            SAC_DISTMEM_HM_MALLOC (SAC_ND_A_FIELD (var_NT),                              \
                                   SAC_ND_A_MIRROR_FIRST_ELEMS (var_NT)                  \
                                     * sizeof (basetype),                                \
                                   basetype);                                            \
            SAC_TR_MEM_PRINT (("ND_ALLOC__DATA_DIS( %s) at addr: %p", NT_STR (var_NT),   \
                               SAC_ND_A_FIELD (var_NT)))                                 \
            SAC_TR_INC_ARRAY_MEMCNT (SAC_ND_A_FIRST_ELEMS (var_NT))                      \
            SAC_PF_MEM_INC_ALLOC (SAC_ND_A_FIRST_ELEMS (var_NT) * sizeof (basetype))     \
            SAC_ND_A_DESC_OFFS (var_NT) = SAC_ND_A_MIRROR_OFFS (var_NT)                  \
              = SAC_DISTMEM_DET_OFFS (SAC_ND_A_FIELD (var_NT));                          \
            SAC_CS_REGISTER_ARRAY (var_NT)                                               \
            SAC_DISTMEM_MOVE_PTR_FOR_WRITES (SAC_ND_A_FIELD (var_NT),                    \
                                             SAC_ND_A_FIRST_ELEMS (var_NT))              \
            SAC_DISTMEM_INIT_PTR_DESC (var_NT)                                           \
        } else {                                                                         \
            SAC_ND_ALLOC__DATA_BASETYPE__AKD_AUD (var_NT, basetype)                      \
        }                                                                                \
        SAC_ND_A_MIRROR_LOCAL_FROM (var_NT) = SAC_DISTMEM_DET_LOCAL_FROM (var_NT);       \
        SAC_ND_A_MIRROR_LOCAL_COUNT (var_NT) = SAC_DISTMEM_DET_LOCAL_COUNT (var_NT);     \
        SAC_ND_A_MIRROR_PTR_CACHE_FROM (var_NT) = 0;                                     \
        SAC_ND_A_MIRROR_PTR_CACHE_COUNT (var_NT) = 0;                                    \
    }

/*
 * SAC_ND_ALLOC_DESC_AND_DATA implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_ALLOC__DESC_AND_DATA__AKS(var_NT, dim, basetype)                          \
    {                                                                                    \
        SAC_ASSURE_TYPE ((dim == SAC_ND_A_MIRROR_DIM (var_NT)),                          \
                         ("Inconsistant dimension for array %s found!",                  \
                          NT_STR (var_NT)));                                             \
        SAC_HM_MALLOC_FIXED_SIZE_WITH_DESC (SAC_ND_A_FIELD (var_NT),                     \
                                            SAC_ND_A_DESC (var_NT),                      \
                                            SAC_ND_A_MIRROR_SIZE (var_NT)                \
                                              * sizeof (*SAC_ND_A_FIELD (var_NT)),       \
                                            SAC_ND_A_MIRROR_DIM (var_NT), basetype,      \
                                            SAC_ND_DESC_BASETYPE (var_NT))               \
        SAC_TR_MEM_PRINT (("ND_ALLOC__DESC( %s, %s) at addr: %p", NT_STR (var_NT), #dim, \
                           SAC_ND_A_DESC (var_NT)))                                      \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_ALLOC__DATA( %s) at addr: %p", NT_STR (var_NT), SAC_ND_A_FIELD (var_NT))) \
        SAC_TR_INC_ARRAY_MEMCNT (SAC_ND_A_SIZE (var_NT))                                 \
        SAC_PF_MEM_INC_ALLOC (SAC_ND_A_SIZE (var_NT)                                     \
                              * sizeof (*SAC_ND_A_FIELD (var_NT)))                       \
        SAC_PF_MEM_INC_ALLOC_DESC (BYTE_SIZE_OF_DESC (SAC_ND_A_MIRROR_SIZE (var_NT)))    \
        SAC_CS_REGISTER_ARRAY (var_NT)                                                   \
    }

#define SAC_ND_ALLOC__DESC_AND_DATA__AKS_DIS(var_NT, dim, basetype)                      \
    {                                                                                    \
        SAC_DISTMEM_RECHECK_IS_DIST (var_NT)                                             \
        if (SAC_ND_A_IS_DIST (var_NT)) {                                                 \
            SAC_ASSURE_TYPE ((dim == SAC_ND_A_MIRROR_DIM (var_NT)),                      \
                             ("Inconsistant dimension for array %s found!",              \
                              NT_STR (var_NT)));                                         \
            SAC_ND_A_MIRROR_FIRST_ELEMS (var_NT)                                         \
              = SAC_DISTMEM_DET_MAX_ELEMS_PER_NODE (SAC_ND_A_SIZE (var_NT),              \
                                                    SAC_ND_A_SHAPE (var_NT, 0));         \
            SAC_DISTMEM_HM_MALLOC_FIXED_SIZE (SAC_ND_A_FIELD (var_NT),                   \
                                              SAC_ND_A_MIRROR_FIRST_ELEMS (var_NT)       \
                                                * sizeof (basetype),                     \
                                              basetype);                                 \
            SAC_HM_MALLOC_FIXED_SIZE (SAC_ND_A_DESC (var_NT),                            \
                                      BYTE_SIZE_OF_DIS_DESC (TRUE, dim),                 \
                                      SAC_ND_DESC_BASETYPE (var_NT))                     \
            SAC_ND_A_DESC_FIRST_ELEMS (var_NT) = SAC_ND_A_MIRROR_FIRST_ELEMS (var_NT);   \
            SAC_TR_MEM_PRINT (("ND_ALLOC__DESC( %s, %s) at addr: %p", NT_STR (var_NT),   \
                               #dim, SAC_ND_A_DESC (var_NT)))                            \
            SAC_TR_MEM_PRINT (("ND_ALLOC__DATA( %s) at addr: %p", NT_STR (var_NT),       \
                               SAC_ND_A_FIELD (var_NT)))                                 \
            SAC_TR_INC_ARRAY_MEMCNT (SAC_ND_A_FIRST_ELEMS (var_NT))                      \
            SAC_PF_MEM_INC_ALLOC (SAC_ND_A_FIRST_ELEMS (var_NT) * sizeof (basetype))     \
            SAC_PF_MEM_INC_ALLOC_DESC (BYTE_SIZE_OF_DIS_DESC (TRUE, dim))                \
            SAC_ND_A_DESC_OFFS (var_NT) = SAC_ND_A_MIRROR_OFFS (var_NT)                  \
              = SAC_DISTMEM_DET_OFFS (SAC_ND_A_FIELD (var_NT));                          \
            SAC_DISTMEM_MOVE_PTR_FOR_WRITES (SAC_ND_A_FIELD (var_NT),                    \
                                             SAC_ND_A_FIRST_ELEMS (var_NT))              \
            SAC_DISTMEM_INIT_PTR_DESC (var_NT)                                           \
            SAC_CS_REGISTER_ARRAY (var_NT)                                               \
        } else {                                                                         \
            SAC_ND_ALLOC__DESC_AND_DATA__AKS (var_NT, dim, basetype)                     \
        }                                                                                \
        SAC_ND_A_DESC_IS_DIST (var_NT) = SAC_ND_A_MIRROR_IS_DIST (var_NT);               \
        SAC_ND_A_MIRROR_LOCAL_FROM (var_NT) = SAC_DISTMEM_DET_LOCAL_FROM (var_NT);       \
        SAC_ND_A_MIRROR_LOCAL_COUNT (var_NT) = SAC_DISTMEM_DET_LOCAL_COUNT (var_NT);     \
        SAC_ND_A_MIRROR_PTR_CACHE_FROM (var_NT) = 0;                                     \
        SAC_ND_A_MIRROR_PTR_CACHE_COUNT (var_NT) = 0;                                    \
    }

#define SAC_ND_ALLOC__DESC_AND_DATA__UNDEF(var_NT, dim, basetype) SAC_ICM_UNDEF ();

/******************************************************************************
 *
 * ICMs for freeing data objects
 * =============================
 *
 * ND_FREE( var_NT, freefun) :
 *   frees a data object
 *
 * ND_FREE__DESC( var_NT) :
 *   frees memory for descriptor
 *
 * ND_FREE__DATA( var_NT, freefun) :
 *   frees memory for a data object (without descriptor)
 *
 ******************************************************************************/

#define SAC_ND_FREE(var_NT, freefun)                                                     \
    {                                                                                    \
        SAC_ND_FREE__DATA (var_NT, freefun)                                              \
        SAC_ND_FREE__DESC (var_NT)                                                       \
    }

/*
 * SAC_ND_FREE__DESC implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_FREE__DESC__NOOP(var_NT) SAC_NOOP ()

#define SAC_ND_FREE__DESC__DEFAULT(var_NT)                                               \
    {                                                                                    \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_FREE__DESC( %s) at addr: %p", NT_STR (var_NT), SAC_ND_A_DESC (var_NT)))   \
        SAC_PF_MEM_INC_FREE_DESC (BYTE_SIZE_OF_DESC (SAC_ND_A_MIRROR_SIZE (var_NT)))     \
        SAC_HM_FREE_DESC (SAC_REAL_DESC (SAC_ND_A_DESC (var_NT)))                        \
    }

/*
 * SAC_ND_FREE__DATA implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_FREE__DATA__SCL_NHD(var_NT, freefun) SAC_NOOP ()

#define SAC_ND_FREE__DATA__SCL_HID(var_NT, freefun)                                      \
    {                                                                                    \
        SAC_TR_MEM_PRINT (("ND_FREE__DATA( %s, %s) at addr: %p", NT_STR (var_NT),        \
                           #freefun, SAC_ND_A_FIELD (var_NT)))                           \
        freefun (SAC_ND_GETVAR (var_NT, SAC_ND_A_FIELD (var_NT)));                       \
        SAC_TR_DEC_HIDDEN_MEMCNT (1)                                                     \
    }

#define SAC_ND_FREE__DATA__AKS_NHD(var_NT, freefun)                                      \
    {                                                                                    \
        SAC_TR_MEM_PRINT (("ND_FREE__DATA( %s, %s) at addr: %p", NT_STR (var_NT),        \
                           #freefun, SAC_ND_A_FIELD (var_NT)))                           \
        SAC_ND_CUDA_UNPIN (var_NT)                                                       \
        SAC_HM_FREE_FIXED_SIZE (SAC_ND_GETVAR (var_NT, SAC_ND_A_FIELD (var_NT)),         \
                                SAC_ND_A_SIZE (var_NT)                                   \
                                  * sizeof (*SAC_ND_A_FIELD (var_NT)))                   \
        SAC_TR_DEC_ARRAY_MEMCNT (SAC_ND_A_SIZE (var_NT))                                 \
        SAC_PF_MEM_INC_FREE (SAC_ND_A_SIZE (var_NT) * sizeof (*SAC_ND_A_FIELD (var_NT))) \
        SAC_CS_UNREGISTER_ARRAY (var_NT)                                                 \
    }

/* TODO: Is the descriptor freed? */
#define SAC_ND_FREE__DATA__AKS_NHD_DIS(var_NT, freefun)                                  \
    {                                                                                    \
        if (SAC_ND_A_IS_DIST (var_NT)) {                                                 \
            SAC_DISTMEM_BARRIER ();                                                      \
            SAC_TR_MEM_PRINT (("ND_FREE__DATA_DIS( %s, %s) at addr: %p",                 \
                               NT_STR (var_NT), #freefun, SAC_ND_A_FIELD (var_NT)))      \
            SAC_DISTMEM_HM_FREE_FIXED_SIZE (                                             \
              SAC_DISTMEM_GET_PTR_FOR_FREE (SAC_ND_GETVAR (var_NT,                       \
                                                           SAC_ND_A_FIELD (var_NT)),     \
                                            SAC_ND_A_FIRST_ELEMS (var_NT)),              \
              SAC_ND_A_FIRST_ELEMS (var_NT) * sizeof (*SAC_ND_A_FIELD (var_NT)))         \
            SAC_TR_DEC_ARRAY_MEMCNT (SAC_ND_A_FIRST_ELEMS (var_NT))                      \
            SAC_PF_MEM_INC_FREE (SAC_ND_A_FIRST_ELEMS (var_NT)                           \
                                 * sizeof (*SAC_ND_A_FIELD (var_NT)))                    \
            SAC_CS_UNREGISTER_ARRAY (var_NT)                                             \
        } else {                                                                         \
            SAC_ND_FREE__DATA__AKS_NHD (var_NT, freefun)                                 \
        }                                                                                \
    }

#define SAC_ND_FREE__DATA__AKS_HID(var_NT, freefun)                                      \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (var_NT); SAC_i++) {                       \
            freefun (SAC_ND_READ (var_NT, SAC_i));                                       \
        }                                                                                \
        SAC_ND_FREE__DATA__AKS_NHD (var_NT, freefun)                                     \
    }

#define SAC_ND_FREE__DATA__AKD_NHD(var_NT, freefun)                                      \
    {                                                                                    \
        SAC_TR_MEM_PRINT (("ND_FREE__DATA( %s, %s) at addr: %p", NT_STR (var_NT),        \
                           #freefun, SAC_ND_A_FIELD (var_NT)))                           \
        SAC_ND_CUDA_UNPIN (var_NT)                                                       \
        SAC_HM_FREE (SAC_ND_GETVAR (var_NT, SAC_ND_A_FIELD (var_NT)))                    \
        SAC_TR_DEC_ARRAY_MEMCNT (SAC_ND_A_SIZE (var_NT))                                 \
        SAC_PF_MEM_INC_FREE (SAC_ND_A_SIZE (var_NT) * sizeof (*SAC_ND_A_FIELD (var_NT))) \
        SAC_CS_UNREGISTER_ARRAY (var_NT)                                                 \
    }

#define SAC_ND_FREE__DATA__AKD_AUD_NHD_DIS(var_NT, freefun)                              \
    {                                                                                    \
        if (SAC_ND_A_IS_DIST (var_NT)) {                                                 \
            SAC_DISTMEM_BARRIER ();                                                      \
            SAC_TR_MEM_PRINT (("ND_FREE__DATA_DIS( %s, %s) at addr: %p",                 \
                               NT_STR (var_NT), #freefun, SAC_ND_A_FIELD (var_NT)))      \
            SAC_DISTMEM_HM_FREE (                                                        \
              SAC_DISTMEM_GET_PTR_FOR_FREE (SAC_ND_GETVAR (var_NT,                       \
                                                           SAC_ND_A_FIELD (var_NT)),     \
                                            SAC_ND_A_FIRST_ELEMS (var_NT)))              \
            SAC_TR_DEC_ARRAY_MEMCNT (SAC_ND_A_FIRST_ELEMS (var_NT))                      \
            SAC_PF_MEM_INC_FREE (SAC_ND_A_FIRST_ELEMS (var_NT)                           \
                                 * sizeof (*SAC_ND_A_FIELD (var_NT)))                    \
            SAC_CS_UNREGISTER_ARRAY (var_NT)                                             \
        } else {                                                                         \
            SAC_ND_FREE__DATA__AKD_NHD (var_NT, freefun)                                 \
        }                                                                                \
    }

#define SAC_ND_PRINT_SHAPE__SCL(nt)                                                      \
    {                                                                                    \
        SAC_Print ("[]\n");                                                              \
    }

#define SAC_ND_PRINT_SHAPE__SHP(nt)                                                      \
    {                                                                                    \
        SAC_Print ((char *)SAC_PrintShape (SAC_ND_A_DESC (nt)));                         \
    }

/************************
 ************************
 ***
 *** CAT22, CAT23, CAT24
 ***
 ***/

/******************************************************************************
 *
 * ICMs for assigning data objects
 * ===============================
 *
 * ND_ASSIGN( to_NT, to_sdim, from_NT, from_sdim, copyfun) :
 *   assigns a data object to another one
 * ND_ASSIGN__DESC( to_NT, from_NT) :
 *   assigns a descriptor to another one
 * ND_ASSIGN__SHAPE( to_NT, to_sdim, from_NT, from_sdim) :
 *   assigns a shape information (descriptor *and* mirror) to another one
 * ND_ASSIGN__DATA( to_NT, to_sdim, from_NT, from_sdim, copyfun) :
 *   assigns a data object to another one (without mirror)
 *
 * ND_COPY( to_NT, to_sdim, from_NT, from_sdim, copyfun) :
 *   copies a data object to another one
 * ND_COPY__SHAPE( to_NT, to_sdim, from_NT, from_sdim) :
 *   copies a shape information (descriptor *and* mirror) to another one
 * ND_COPY__DATA( to_NT, from_NT, copyfun) :
 *   copies a data object to another one (without mirror)
 *
 * ND_MAKE_UNIQUE( to_NT, to_sdim, from_NT, copyfun) :
 *   assigns a data object to another one iff RC is zero, copies it otherwise.
 *
 ******************************************************************************/

/* ND_ASSIGN( ...)  is a C-ICM */

/* ND_ASSIGN__DESC( ...)  is a C-ICM */

/* ND_ASSIGN__SHAPE( ...)  is a C-ICM */

/* ND_COPY( ...)  is a C-ICM */

/* ND_COPY__SHAPE( ...)  is a C-ICM */

/*
 * SAC_ND_ASSIGN__DATA implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_ASSIGN__DATA__UNDEF(to_NT, from_NT, copyfun) SAC_ICM_UNDEF ()

#define SAC_ND_ASSIGN__DATA__SCL_AUD(to_NT, from_NT, copyfun)                            \
    {                                                                                    \
        SAC_ND_WRITE_READ_COPY (to_NT, 0, from_NT, 0, copyfun)                           \
    }

#define SAC_ND_ASSIGN__DATA__SCL_AUD_UNQ(to_NT, from_NT, copyfun)                        \
    {                                                                                    \
        SAC_ND_WRITE_READ (to_NT, 0, from_NT, 0)                                         \
        /* free data vector but keep its content (i.e. the hidden object itself)! */     \
        SAC_ND_FREE__DATA__AUD_NHD (from_NT, freefun)                                    \
    }

#define SAC_ND_ASSIGN__DATA__AKS_AKS(to_NT, from_NT, copyfun)                            \
    {                                                                                    \
        SAC_ND_GETVAR (to_NT, SAC_ND_A_FIELD (to_NT))                                    \
          = SAC_ND_GETVAR (from_NT, SAC_ND_A_FIELD (from_NT));                           \
    }

#define SAC_ND_ASSIGN__DATA__AUD_SCL_NHD(to_NT, from_NT, copyfun)                        \
    {                                                                                    \
        SAC_ND_ALLOC__DATA_BASETYPE (to_NT, SAC_NT_CBASETYPE (to_NT))                    \
        SAC_ND_WRITE_READ_COPY (to_NT, 0, from_NT, 0, copyfun)                           \
    }

#define SAC_ND_ASSIGN__DATA__AUD_SCL_UNQ(to_NT, from_NT, copyfun)                        \
    {                                                                                    \
        SAC_ND_ALLOC__DATA_BASETYPE (to_NT, SAC_NT_CBASETYPE (to_NT))                    \
        SAC_ND_WRITE_READ (to_NT, 0, from_NT, 0);                                        \
    }

#define SAC_ND_ASSIGN__DATA__AUD_AKS(to_NT, from_NT, copyfun)                            \
    {                                                                                    \
        SAC_ND_GETVAR (to_NT, SAC_ND_A_FIELD (to_NT))                                    \
          = SAC_ND_GETVAR (from_NT, SAC_ND_A_FIELD (from_NT));                           \
    }

/*
 * The lhs is distributable and the rhs is either distributable
 * or non-distributable but may be assigned in DSM memory.
 *
 * We have to check at runtime whether the lhs and rhs are
 * actually distributed. There is three cases:
 *  1) The lhs and rhs are both distributed.
 *     Do a direct assign including the DSM mirror/descriptor variables.
 *  2) Only one of them is distributed.
 *     Allocate a new array and copy the data.
 *  3) None of them is distributed.
 *     Do a direct assign.
 */
#define SAC_ND_ASSIGN__DATA__TO_DIS(to_NT, from_NT, copyfun)                             \
    {                                                                                    \
        if (SAC_ND_A_IS_DIST (to_NT) && SAC_ND_A_IS_DIST (from_NT)) {                    \
            SAC_ND_GETVAR (to_NT, SAC_ND_A_FIELD (to_NT))                                \
              = SAC_ND_GETVAR (from_NT, SAC_ND_A_FIELD (from_NT));                       \
            SAC_ND_A_DESC_OFFS (to_NT) = SAC_ND_A_MIRROR_OFFS (to_NT)                    \
              = SAC_ND_A_OFFS (from_NT);                                                 \
            SAC_ND_A_DESC_FIRST_ELEMS (to_NT) = SAC_ND_A_MIRROR_FIRST_ELEMS (to_NT)      \
              = SAC_ND_A_FIRST_ELEMS (from_NT);                                          \
            SAC_DISTMEM_INIT_PTR_DESC (to_NT)                                            \
        } else if (SAC_ND_A_IS_DIST (to_NT) || SAC_ND_A_IS_DIST (from_NT)) {             \
            SAC_ND_ALLOC__DATA (to_NT)                                                   \
            SAC_ND_COPY__DATA (to_NT, from_NT, copyfun)                                  \
        } else {                                                                         \
            SAC_ND_GETVAR (to_NT, SAC_ND_A_FIELD (to_NT))                                \
              = SAC_ND_GETVAR (from_NT, SAC_ND_A_FIELD (from_NT));                       \
        }                                                                                \
        SAC_ND_A_DESC_IS_DIST (to_NT) = SAC_ND_A_MIRROR_IS_DIST (to_NT);                 \
        SAC_ND_A_MIRROR_LOCAL_FROM (to_NT) = SAC_DISTMEM_DET_LOCAL_FROM (to_NT);         \
        SAC_ND_A_MIRROR_LOCAL_COUNT (to_NT) = SAC_DISTMEM_DET_LOCAL_COUNT (to_NT);       \
        SAC_ND_A_MIRROR_PTR_CACHE_FROM (to_NT) = 0;                                      \
        SAC_ND_A_MIRROR_PTR_CACHE_COUNT (to_NT) = 0;                                     \
    }

/*
 * The lhs is non-distributable but may be assigned in DSM memory and the rhs
 * is either distributable or non-distributable but may be assigned in DSM memory.
 *
 * We have to check at runtime whether the lhs and rhs are
 * actually distributed. There is three cases:
 *  1) The lhs and rhs are both distributed.
 *     Do a direct assign including the DSM mirror/descriptor variables.
 *  2) Only one of them is distributed.
 *     Allocate a new array and copy the data.
 *  3) None of them is distributed.
 *     Do a direct assign.
 */
#define SAC_ND_ASSIGN__DATA__TO_DSM(to_NT, from_NT, copyfun)                             \
    {                                                                                    \
        if (SAC_ND_A_IS_DIST (to_NT) && SAC_ND_A_IS_DIST (from_NT)) {                    \
            SAC_ND_GETVAR (to_NT, SAC_ND_A_FIELD (to_NT))                                \
              = SAC_ND_GETVAR (from_NT, SAC_ND_A_FIELD (from_NT));                       \
            SAC_ND_A_DESC_OFFS (to_NT) = SAC_ND_A_MIRROR_OFFS (to_NT)                    \
              = SAC_ND_A_OFFS (from_NT);                                                 \
        } else if (SAC_ND_A_IS_DIST (to_NT) || SAC_ND_A_IS_DIST (from_NT)) {             \
            SAC_ND_ALLOC__DATA (to_NT)                                                   \
            SAC_ND_COPY__DATA (to_NT, from_NT, copyfun)                                  \
        } else {                                                                         \
            SAC_ND_GETVAR (to_NT, SAC_ND_A_FIELD (to_NT))                                \
              = SAC_ND_GETVAR (from_NT, SAC_ND_A_FIELD (from_NT));                       \
        }                                                                                \
    }

/*
 * The lhs is non-distributable and the rhs
 * is either distributable or non-distributable but may be assigned in DSM memory.
 *
 * We have to check at runtime whether the rhs is
 * actually distributed. There is two cases:
 *  1) The rhs is distributed.
 *     Allocate a new array and copy the data..
 *  2) The rhs is not distributed.
 *     Do a direct assign.
 */
#define SAC_ND_ASSIGN__DATA__TO_NDI(to_NT, from_NT, copyfun)                             \
    {                                                                                    \
        if (SAC_ND_A_IS_DIST (from_NT)) {                                                \
            SAC_ND_ALLOC__DATA (to_NT)                                                   \
            SAC_ND_COPY__DATA (to_NT, from_NT, copyfun)                                  \
        } else {                                                                         \
            SAC_ND_GETVAR (to_NT, SAC_ND_A_FIELD (to_NT))                                \
              = SAC_ND_GETVAR (from_NT, SAC_ND_A_FIELD (from_NT));                       \
        }                                                                                \
    }

/* ND_MAKE_UNIQUE( ...)  is a C-ICM */

/*
 * SAC_ND_COPY__DATA implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_COPY__DATA__SCL_SCL(to_NT, from_NT, copyfun)                              \
    {                                                                                    \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_COPY__DATA( %s, %s, %s)", NT_STR (to_NT), #from_NT, #copyfun))            \
        SAC_ND_WRITE_READ_COPY (to_NT, 0, from_NT, 0, copyfun)                           \
    }

#define SAC_ND_COPY__DATA__SCL_ANY(to_NT, from_NT, copyfun)                              \
    {                                                                                    \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_COPY__DATA( %s, %s, %s)", NT_STR (to_NT), #from_NT, #copyfun))            \
        SAC_ASSURE_TYPE ((SAC_ND_A_DIM (from_NT) == 0),                                  \
                         ("Assignment with incompatible types found!"));                 \
        SAC_ND_WRITE_READ_COPY (to_NT, 0, from_NT, 0, copyfun)                           \
    }

#define SAC_ND_COPY__DATA__ANY_SCL(to_NT, from_NT, copyfun)                              \
    {                                                                                    \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_COPY__DATA( %s, %s, %s)", NT_STR (to_NT), #from_NT, #copyfun))            \
        SAC_ASSURE_TYPE ((SAC_ND_A_DIM (to_NT) == 0),                                    \
                         ("Assignment with incompatible types found!"));                 \
        SAC_ND_WRITE_READ_COPY (to_NT, 0, from_NT, 0, copyfun)                           \
    }

#define SAC_ND_COPY__DATA__ANY_ANY(to_NT, from_NT, copyfun)                              \
    {                                                                                    \
        int SAC_i;                                                                       \
        SAC_TR_MEM_PRINT (("ND_COPY__DATA( %s, %s, %s) at addr: %p", NT_STR (from_NT),   \
                           #to_NT, #copyfun, SAC_ND_A_FIELD (to_NT)))                    \
        SAC_ASSURE_TYPE ((SAC_ND_A_SIZE (to_NT) == SAC_ND_A_SIZE (from_NT)),             \
                         ("Assignment with incompatible types found!"));                 \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (from_NT); SAC_i++) {                      \
            SAC_ND_WRITE_READ_COPY (to_NT, SAC_i, from_NT, SAC_i, copyfun)               \
        }                                                                                \
    }

/*
 * If the target is distributed, we only copy the part that is local
 * to this node. In that case we need a cache invalidation and barrier at the end.
 */
#define SAC_ND_COPY__DATA__DIS_ANY(to_NT, from_NT, copyfun)                              \
    {                                                                                    \
        int SAC_i;                                                                       \
        SAC_TR_MEM_PRINT (("ND_COPY__DATA%s( %s, %s, %s) at addr: %p",                   \
                           SAC_ND_A_IS_DIST (to_NT) ? "_DIS" : "_NDI", NT_STR (from_NT), \
                           #to_NT, #copyfun, SAC_ND_A_FIELD (to_NT)))                    \
        SAC_ASSURE_TYPE ((SAC_ND_A_SIZE (to_NT) == SAC_ND_A_SIZE (from_NT)),             \
                         ("Assignment with incompatible types found!"));                 \
        if (SAC_ND_A_IS_DIST (to_NT)) {                                                  \
            SAC_DISTMEM_ALLOW_DIST_WRITES ();                                            \
        }                                                                                \
        for (SAC_i = SAC_DISTMEM_DET_LOCAL_FROM (to_NT);                                 \
             SAC_i <= SAC_DISTMEM_DET_LOCAL_TO (to_NT); SAC_i++) {                       \
            SAC_ND_WRITE_READ_COPY (to_NT, SAC_i, from_NT, SAC_i, copyfun)               \
        }                                                                                \
        if (SAC_ND_A_IS_DIST (to_NT)) {                                                  \
            SAC_DISTMEM_FORBID_DIST_WRITES ();                                           \
            SAC_DISTMEM_INVAL_CACHE (SAC_ND_A_OFFS (to_NT),                              \
                                     SAC_ND_A_FIRST_ELEMS (to_NT)                        \
                                       * sizeof (SAC_NT_CBASETYPE (to_NT)));             \
            SAC_DISTMEM_BARRIER ();                                                      \
        }                                                                                \
    }

#define SAC_ND_COPY__DESC_SCL(to_NT, from_NT)

#define SAC_ND_COPY__DESC_AKS(to_NT, from_NT)                                            \
    SAC_ND_A_COPY__DESC_SCL                                                              \
    SAC_ND_A_DESC_RC (to_NT) = SAC_ND_A_DESC_RC (from_NT);

#define SAC_ND_COPY__DESC_AKD(to_NT, from_NT)                                            \
    SAC_ND_A_COPY__DESC_AKS (to_NT, from_NT)                                             \
    SAC_ND_A_DESC_SIZE (to_NT) = SAC_ND_A_DESC_SIZE (from_NT);                           \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_DIM (from_NT); SAC_i++) {                       \
            SAC_ND_A_DESC_SHAPE (to_NT) = SAC_ND_A_DESC_SHAPE (from_NT);                 \
        }                                                                                \
    }

#define SAC_ND_COPY__DESC_AUD(to_NT, from_NT)                                            \
    SAC_ND_A_COPY__DESC_AKD (to_NT, from_NT)                                             \
    SAC_ND_A_DESC_DIM (to_NT) = SAC_ND_A_DESC_DIM (from_NT);

/******************************************************************************
 *
 * ICMs for creating refcounted objects:
 * =====================================
 *
 * ND_CREATE__SCALAR__DATA( var_NT, val) :
 *   sets data of a constant scalar
 *
 * ND_CREATE__STRING__DATA( var_NT, str) :
 *   sets data of a constant character array (string)
 *
 * ND_CREATE__ARRAY__SHAPE( var_NT, sdim, val_size, ...val...) :
 *   sets shape of a constant non-scalar array
 * ND_CREATE__ARRAY__DATA( var_NT, sdim, val_size, ...val..., copyfun) :
 *   sets data of a constant non-scalar array
 *
 ******************************************************************************/

#define SAC_ND_CREATE__SCALAR__DATA(var_NT, val)                                         \
    {                                                                                    \
        SAC_ND_WRITE_COPY (var_NT, 0, val, );                                            \
    }

#define SAC_ND_CREATE__STRING__DATA(var_NT, str)                                         \
    {                                                                                    \
        SAC_String2Array (SAC_ND_A_FIELD (var_NT), str);                                 \
    }

/* ND_CREATE__ARRAY__DIM( ...) is a C-ICM */
/* ND_CREATE__ARRAY__SHAPE( ...) is a C-ICM */
/* ND_CREATE__ARRAY__DATA( ...) is a C-ICM */

/************************
 ************************
 ***
 *** CAT28, CAT29, CAT30
 ***
 ***/

/******************************************************************************
 *
 * ICMs for refcounting data objects
 * =================================
 *
 * ND_SET__RC( var_NT, rc) :
 *   sets the refcount (in the descriptor) of a data object
 * ND_INC_RC( var_NT, rc) :
 *   increments the refcount of a data object
 * ND_DEC_RC( var_NT, rc) :
 *   decrements the refcount of a data object
 * ND_DEC_RC_FREE( var_NT, rc, freefun) :
 *   decrements the refcount and frees the data object if refcount becomes 0
 *
 ******************************************************************************/

/*
 * SAC_ND_SET__RC implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_SET__RC__NOOP(var_NT, rc) SAC_NOOP ()

#define SAC_ND_SET__RC__C99(var_NT, rc)                                                  \
    {                                                                                    \
        SAC_TR_REF_PRINT (("ND_SET__RC( %s, %d)", NT_STR (var_NT), rc))                  \
        SAC_ND_A_RC__C99 (var_NT) = rc;                                                  \
        SAC_TR_REF_PRINT_RC (var_NT)                                                     \
    }

#define SAC_DESC_SET__RC__C99(desc, rc)                                                  \
    {                                                                                    \
        DESC_RC (var_NT) = rc;                                                           \
    }

/*
 * SAC_ND_INIT__RC implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_INIT__RC__NOOP(var_NT, rc) SAC_NOOP ()

#define SAC_ND_INIT__RC__C99(var_NT, rc)                                                 \
    {                                                                                    \
        SAC_TR_REF_PRINT (("ND_INIT__RC( %s, %d)", NT_STR (var_NT), rc))                 \
        DESC_RC (SAC_ND_A_DESC (var_NT)) = rc;                                           \
        DESC_RC_MODE (SAC_ND_A_DESC (var_NT)) = SAC_DESC_RC_MODE_LOCAL;                  \
        DESC_PARENT (SAC_ND_A_DESC (var_NT)) = 0;                                        \
        SAC_TR_REF_PRINT_RC (var_NT)                                                     \
    }

#define SAC_DESC_INIT__RC__C99(desc, rc)                                                 \
    {                                                                                    \
        DESC_RC (desc) = rc;                                                             \
        DESC_RC_MODE (desc) = SAC_DESC_RC_MODE_LOCAL;                                    \
        DESC_PARENT (desc) = 0;                                                          \
    }

/*
 * SAC_ND_INC_RC implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_INC_RC__NOOP(var_NT, rc) SAC_NOOP ()

#define SAC_ND_INC_RC__C99(var_NT, rc)                                                   \
    {                                                                                    \
        SAC_TR_REF_PRINT (("ND_INC_RC( %s, %d)", NT_STR (var_NT), rc))                   \
        SAC_ND_A_RC__C99 (var_NT) += rc;                                                 \
        SAC_TR_REF_PRINT_RC (var_NT)                                                     \
    }

#define SAC_DESC_INC_RC__C99(desc, rc)                                                   \
    {                                                                                    \
        DESC_RC (desc) += rc;                                                            \
    }

/*
 * SAC_ND_DEC_RC implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_DEC_RC__NOOP(var_NT, rc) SAC_NOOP ()

#define SAC_ND_DEC_RC__C99(var_NT, rc)                                                   \
    {                                                                                    \
        SAC_TR_REF_PRINT (("ND_DEC_RC( %s, %d)", NT_STR (var_NT), rc))                   \
        SAC_ND_A_RC__C99 (var_NT) -= rc;                                                 \
        SAC_TR_REF_PRINT_RC (var_NT)                                                     \
    }

#define SAC_DESC_DEC_RC__C99(desc, rc)                                                   \
    {                                                                                    \
        DESC_RC (desc) -= rc;                                                            \
    }

/*
 * SAC_ND_DEC_RC_FREE implementations (referenced by sac_std_gen.h)
 */

/* 'nt' is unique -> 'nt' has been consumed -> free 'nt' */
#define SAC_ND_DEC_RC_FREE__UNQ(var_NT, rc, freefun) SAC_ND_FREE (var_NT, freefun)

#define SAC_ND_DEC_RC_FREE__NOOP(var_NT, rc, freefun) SAC_NOOP ()

#if SAC_DO_CUDA_ALLOC == SAC_CA_cuman || SAC_DO_CUDA_ALLOC == SAC_CA_cumanp
#define SAC_ND_DEC_RC_FREE__C99(var_NT, rc, freefun)                                     \
    {                                                                                    \
        cudaDeviceSynchronize (); /* FIXME This is not good! */                          \
        SAC_TR_REF_PRINT (                                                               \
          ("ND_DEC_RC_FREE( %s, %d, %s)", NT_STR (var_NT), rc, #freefun))                \
        if ((SAC_ND_A_RC__C99 (var_NT) -= rc) == 0) {                                    \
            SAC_TR_REF_PRINT_RC (var_NT)                                                 \
            SAC_ND_FREE (var_NT, freefun)                                                \
        } else {                                                                         \
            SAC_TR_REF_PRINT_RC (var_NT)                                                 \
        }                                                                                \
    }
#else
#define SAC_ND_DEC_RC_FREE__C99(var_NT, rc, freefun)                                     \
    {                                                                                    \
        SAC_TR_REF_PRINT (                                                               \
          ("ND_DEC_RC_FREE( %s, %d, %s)", NT_STR (var_NT), rc, #freefun))                \
        if ((SAC_ND_A_RC__C99 (var_NT) -= rc) == 0) {                                    \
            SAC_TR_REF_PRINT_RC (var_NT)                                                 \
            SAC_ND_FREE (var_NT, freefun)                                                \
        } else {                                                                         \
            SAC_TR_REF_PRINT_RC (var_NT)                                                 \
        }                                                                                \
    }
#endif

#if SAC_DO_CUDA_ALLOC == SAC_CA_cuman || SAC_DO_CUDA_ALLOC == SAC_CA_cumanp
#define SAC_DESC_DEC_RC_FREE__C99(desc, rc, q_waslast)                                   \
    {                                                                                    \
        cudaDeviceSynchronize (); /* FIXME This is not good! */                          \
        q_waslast = ((DESC_RC (desc) -= rc) == 0);                                       \
        if (q_waslast) {                                                                 \
            SAC_FREE (desc);                                                             \
        }                                                                                \
    }
#else
#define SAC_DESC_DEC_RC_FREE__C99(desc, rc, q_waslast)                                   \
    {                                                                                    \
        q_waslast = ((DESC_RC (desc) -= rc) == 0);                                       \
        if (q_waslast) {                                                                 \
            SAC_FREE (desc);                                                             \
        }                                                                                \
    }
#endif

/*
 * SAC_ND_A_RC implementations (referenced by sac_std_gen.h)
 */

#define SAC_ND_A_RC__UNDEF(var_NT) SAC_ICM_UNDEF ()

#define SAC_ND_A_RC__C99(var_NT) DESC_RC (SAC_ND_A_DESC (var_NT))

#define SAC_DESC_A_RC__C99(desc) DESC_RC (desc)

#if SAC_RC_METHOD == SAC_RCM_local
/* Default mode */

#define SAC_ND_INIT__RC__DEFAULT(var_NT, rc) SAC_ND_INIT__RC__C99 (var_NT, rc)

#define SAC_ND_DEC_RC_FREE__DEFAULT(var_NT, rc, freefun)                                 \
    SAC_ND_DEC_RC_FREE__C99 (var_NT, rc, freefun)

#define SAC_ND_SET__RC__DEFAULT(var_NT, rc) SAC_ND_SET__RC__C99 (var_NT, rc)

#define SAC_ND_DEC_RC__DEFAULT(var_NT, rc) SAC_ND_DEC_RC__C99 (var_NT, rc)

#define SAC_ND_INC_RC__DEFAULT(var_NT, rc) SAC_ND_INC_RC__C99 (var_NT, rc)

#define SAC_ND_A_RC__DEFAULT(var_NT) SAC_ND_A_RC__C99 (var_NT)

/* The following SAC_DESC_* macros are for the sacarg interface. */

#define SAC_DESC_INIT_RC(desc, rc) SAC_DESC_INIT__RC__C99 (desc, rc)

#define SAC_DESC_A_RC(desc) SAC_DESC_A_RC__C99 (desc)

#define SAC_DESC_INC_RC(desc, rc) SAC_DESC_INC_RC__C99 (desc, rc)

#define SAC_DESC_DEC_RC(desc, rc) SAC_DESC_DEC_RC__C99 (desc, rc)

#define SAC_DESC_DEC_RC_FREE(desc, rc, q_waslast)                                        \
    SAC_DESC_DEC_RC_FREE__C99 (desc, rc, q_waslast)

#define SAC_DESC_RC_GIVE_ASYNC(newd, arrayd) newd = arrayd;

#endif

#include <stdint.h>
#define SAC_ND_INIT__RC__NORC(var_NT, rc)
#define SAC_ND_DEC_RC_FREE__NORC(var_NT, rc, freefun)
#define SAC_ND_SET__RC__NORC(var_NT, rc)
#define SAC_ND_DEC_RC__NORC(var_NT, rc)
#define SAC_ND_INC_RC__NORC(var_NT, rc)
#define SAC_ND_A_RC__NORC(var_NT) INT_MAX

#define SAC_DESC_INIT__RC__NORC(desc, rc)
#define SAC_DESC_DEC_RC_FREE__NORC(desc, rc, q_waslast)                                  \
    {                                                                                    \
        q_waslast = 0;                                                                   \
    }
#define SAC_DESC_SET__RC__NORC(desc, rc)
#define SAC_DESC_DEC_RC__NORC(desc, rc)
#define SAC_DESC_INC_RC__NORC(desc, rc)
#define SAC_DESC_A_RC__NORC(desc) INT_MAX

#if SAC_RC_METHOD == SAC_RCM_norc
/* No rc */
#define SAC_ND_INIT__RC__DEFAULT(var_NT, rc) SAC_ND_INIT__RC__NORC (var_NT, rc)
#define SAC_ND_DEC_RC_FREE__DEFAULT(var_NT, rc, freefun)                                 \
    SAC_ND_DEC_RC_FREE__NORC (var_NT, rc, freefun)
#define SAC_ND_SET__RC__DEFAULT(var_NT, rc) SAC_ND_SET__RC__NORC (var_NT, rc)
#define SAC_ND_DEC_RC__DEFAULT(var_NT, rc) SAC_ND_DEC_RC__NORC (var_NT, rc)
#define SAC_ND_INC_RC__DEFAULT(var_NT, rc) SAC_ND_INC_RC__NORC (var_NT, rc)
#define SAC_ND_A_RC__DEFAULT(var_NT) SAC_ND_A_RC__NORC (var_NT)
#endif

/******************************************************************************
 *
 * ICMs for predicate-sensitive operations:
 * ========================================
 *
 * IS_LASTREF__BLOCK_BEGIN( var_NT)
 * IS_LASTREF__BLOCK_ELSE( var_NT)
 * IS_LASTREF__BLOCK_END( var_NT)
 *
 * IS_REUSED__BLOCK_BEGIN( to_NT, from_NT)
 * IS_REUSED__BLOCK_ELSE( to_NT, from_NT)
 * IS_REUSED__BLOCK_END( to_NT, from_NT)
 *
 ******************************************************************************/

#define SAC_IS_LASTREF__BLOCK_ELSE(var_NT)                                               \
    }                                                                                    \
    else                                                                                 \
    {
#define SAC_IS_LASTREF__BLOCK_END(var_NT) }

#define SAC_IS_REUSED__BLOCK_ELSE(to_NT, from_NT)                                        \
    }                                                                                    \
    else                                                                                 \
    {
#define SAC_IS_REUSED__BLOCK_END(to_NT, from_NT) }

/*
 * SAC_IS_LASTREF__BLOCK_BEGIN implementations (referenced by sac_std_gen.h)
 */

#define SAC_IS_LASTREF__BLOCK_BEGIN__UNQ(var_NT) if (1) {

#define SAC_IS_LASTREF__BLOCK_BEGIN__SCL_NHD_NUQ(var_NT) if (1) {

#define SAC_IS_LASTREF__BLOCK_BEGIN__DEFAULT(var_NT) if (SAC_ND_A_RC (var_NT) == 1) {

/*
 * SAC_IS_REUSED__BLOCK_BEGIN implementations (referenced by sac_std_gen.h)
 */

#define SAC_IS_REUSED__BLOCK_BEGIN__SCL(to_NT, from_NT) if (0) {

#ifdef DBUG_REUSE
#define SAC_IS_REUSED__BLOCK_BEGIN__DEFAULT(to_NT, from_NT)                              \
    if (SAC_ND_A_FIELD (to_NT) == SAC_ND_A_FIELD (from_NT)) {                            \
        fprintf (stderr, "REUSE DYNAMIC\n");
#else
#ifdef DBUG_NOREUSE
#define SAC_IS_REUSED__BLOCK_BEGIN__DEFAULT(to_NT, from_NT) if (0) {
#else
#define SAC_IS_REUSED__BLOCK_BEGIN__DEFAULT(to_NT, from_NT)                              \
    if (SAC_ND_A_FIELD (to_NT) == SAC_ND_A_FIELD (from_NT)) {
#endif
#endif

/******************************************************************************
 *
 * ICMs for initializing global objects
 * ====================================
 *
 * INITGLOBALOBJECT_BEGIN( varname)
 * INITGLOBALOBJECT_END()
 *
 ******************************************************************************/

#ifdef SAC_GENERATE_CLIBRARY
/*
 * If used in a c library it has to check if the global object has been
 * initialized by another module (uses varname as flag).
 */
#define SAC_INITGLOBALOBJECT_BEGIN(varname)                                              \
    if (!CAT1 (SAC_INIT_FLAG_, varname)) {                                               \
        CAT1 (SAC_INIT_FLAG_, varname) = true;
#define SAC_INITGLOBALOBJECT_END() }
#else
/*
 * without check -> nothing
 */
#define SAC_INITGLOBALOBJECT_BEGIN(varname) SAC_NOTHING ()
#define SAC_INITGLOBALOBJECT_END() SAC_NOTHING ()
#endif

/******************************************************************************
 *
 * ICMs for the F_unshare primitive fun
 * ====================================
 *
 * SAC_IS_SHARED__BLOCK_BEGIN( va_NT, va_sdim, viv_NT, viv_sdim)
 * SAC_IS_SHARED__BLOCK_END( ...)
 *
 * Note: ND_UNSHARE(...) is a C-ICM.
 ******************************************************************************/

#define SAC_IS_SHARED__BLOCK_BEGIN(va_NT, va_sdim, viv_NT, viv_sdim)                     \
    if ((SAC_ND_A_DESC_NULL (va_NT) != NULL) && (SAC_ND_A_DESC_NULL (viv_NT) != NULL)    \
        && (SAC_ND_A_DESC_NULL (va_NT) == SAC_ND_A_DESC_NULL (viv_NT))) {

#define SAC_IS_SHARED__BLOCK_END(va_NT, va_sdim, viv_NT, viv_sdim) }

/******************************************************************************
 *
 * ICMs for broadcasts by the distributed
 * memory backend
 * ====================================
 *
 * SAC_UPDATE_A_MIRROR_DIM( var_NT, val)
 * SAC_A_DIM_BEFORE_UPDATE_MIRROR( var_NT)
 *
 ******************************************************************************/

/*
 * SAC_UPDATE_A_MIRROR_DIM implementations (referenced by sac_std_gen.h)
 */

#define SAC_UPDATE_A_MIRROR_DIM__AUD(var_NT, val) SAC_ND_A_MIRROR_DIM (var_NT) = val;

#define SAC_UPDATE_A_MIRROR_DIM__AKD(var_NT, val)

#define SAC_UPDATE_A_MIRROR_DIM__UNDEF(var_NT, val) SAC_ICM_UNDEF ()

/*
 * SAC_A_DIM_BEFORE_UPDATE_MIRROR implementations (referenced by sac_std_gen.h)
 */

#define SAC_A_DIM_BEFORE_UPDATE_MIRROR__AUD(var_NT) SAC_ND_A_DESC_DIM (var_NT)

#define SAC_A_DIM_BEFORE_UPDATE_MIRROR__AKD(var_NT) SAC_ND_A_DIM (var_NT)

#define SAC_A_DIM_BEFORE_UPDATE_MIRROR__UNDEF(var_NT) SAC_ICM_UNDEF ()

#endif /* _SAC_STD_H_ */
