

/*
 *  Global Switches
 */

#ifndef SAC_DO_CHECK
#define SAC_DO_CHECK                             0
#endif
#ifndef SAC_DO_CHECK_TYPE
#define SAC_DO_CHECK_TYPE                        0
#endif
#ifndef SAC_DO_CHECK_GPU
#define SAC_DO_CHECK_GPU                         0
#endif
#ifndef SAC_DO_CHECK_BOUNDARY
#define SAC_DO_CHECK_BOUNDARY                    0
#endif
#ifndef SAC_DO_CHECK_MALLOC
#define SAC_DO_CHECK_MALLOC                      0
#endif
#ifndef SAC_DO_CHECK_ERRNO
#define SAC_DO_CHECK_ERRNO                       0
#endif
#ifndef SAC_DO_CHECK_HEAP
#define SAC_DO_CHECK_HEAP                        0
#endif
#ifndef SAC_DO_CHECK_DISTMEM
#define SAC_DO_CHECK_DISTMEM                     0
#endif
#ifndef SAC_DO_CHECK_DISTMEMPHM
#define SAC_DO_CHECK_DISTMEMPHM                  0
#endif

#define SAC_DO_PHM                               0
#define SAC_DO_APS                               0
#define SAC_DO_DAO                               0
#define SAC_DO_MSCA                              0
#define SAC_DO_COMPILE_MODULE                    0

#ifndef SAC_DO_PROFILE
#define SAC_DO_PROFILE                           0
#endif
#ifndef SAC_DO_PROFILE_WITH
#define SAC_DO_PROFILE_WITH                      0
#endif
#ifndef SAC_DO_PROFILE_FUN
#define SAC_DO_PROFILE_FUN                       0
#endif
#ifndef SAC_DO_PROFILE_INL
#define SAC_DO_PROFILE_INL                       0
#endif
#ifndef SAC_DO_PROFILE_LIB
#define SAC_DO_PROFILE_LIB                       0
#endif
#ifndef SAC_DO_PROFILE_MEM
#define SAC_DO_PROFILE_MEM                       0
#endif
#ifndef SAC_DO_PROFILE_OPS
#define SAC_DO_PROFILE_OPS                       0
#endif
#ifndef SAC_DO_PROFILE_CUDA
#define SAC_DO_PROFILE_CUDA                      0
#endif
#ifndef SAC_DO_PROFILE_DISTMEM
#define SAC_DO_PROFILE_DISTMEM                   0
#endif

#ifndef SAC_DO_TRACE
#define SAC_DO_TRACE                             0
#endif
#ifndef SAC_DO_TRACE_REF
#define SAC_DO_TRACE_REF                         0
#endif
#ifndef SAC_DO_TRACE_MEM
#define SAC_DO_TRACE_MEM                         0
#endif
#ifndef SAC_DO_TRACE_PRF
#define SAC_DO_TRACE_PRF                         0
#endif
#ifndef SAC_DO_TRACE_FUN
#define SAC_DO_TRACE_FUN                         0
#endif
#ifndef SAC_DO_TRACE_WL
#define SAC_DO_TRACE_WL                          0
#endif
#ifndef SAC_DO_TRACE_AA
#define SAC_DO_TRACE_AA                          0
#endif
#ifndef SAC_DO_TRACE_MT
#define SAC_DO_TRACE_MT                          0
#endif
#ifndef SAC_DO_TRACE_GPU
#define SAC_DO_TRACE_GPU                         0
#endif
#ifndef SAC_DO_TRACE_RTSPEC
#define SAC_DO_TRACE_RTSPEC                      0
#endif
#ifndef SAC_DO_TRACE_DISTMEM
#define SAC_DO_TRACE_DISTMEM                     0
#endif

#ifndef SAC_DO_CACHESIM
#define SAC_DO_CACHESIM                          0
#endif
#ifndef SAC_DO_CACHESIM_ADV
#define SAC_DO_CACHESIM_ADV                      0
#endif
#ifndef SAC_DO_CACHESIM_GLOBAL
#define SAC_DO_CACHESIM_GLOBAL                   1
#endif
#ifndef SAC_DO_CACHESIM_FILE
#define SAC_DO_CACHESIM_FILE                     0
#endif
#ifndef SAC_DO_CACHESIM_PIPE
#define SAC_DO_CACHESIM_PIPE                     0
#endif
#ifndef SAC_DO_CACHESIM_IMDT
#define SAC_DO_CACHESIM_IMDT                     1
#endif

/*
 * Setup for Multi Threaded Data Parallelism
 */
#define SAC_DO_MULTITHREAD                       0
#define SAC_DO_THREADS_STATIC                    1
#define SAC_DO_MT_CREATE_JOIN                    0
#define SAC_DO_MT_PTHREAD                        0
#define SAC_DO_MT_LPEL                           0

/*
 * Setup for OMP Data Parallelism
 */
#define SAC_DO_MT_OMP                            0
#define SAC_DO_OMP_MACROS                        0

/*
 * Setup for MUTC
 */
#define SAC_MUTC_FUNAP_AS_CREATE                 0
#define SAC_MUTC_THREAD_MALLOC                   0
#define SAC_MUTC_DISABLE_THREAD_MEM              0
#define SAC_MUTC_BENCH                           0
#define SAC_MUTC_MACROS                          0
#define SAC_MUTC_RC_INDIRECT                     0
#define SAC_MUTC_SEQ_DATA_PARALLEL               0

/*
 * Setup for GPU Data Parallelism
 */
#define SAC_CUDA_MACROS                          1

/*
 * Setup for Distributed Memory Data Parallelism
 */
#define SAC_DO_DISTMEM                           0
#define SAC_DO_DISTMEM_GASNET                    0
#define SAC_DO_DISTMEM_GPI                       0
#define SAC_DO_DISTMEM_MPI                       0
#define SAC_DO_DISTMEM_ARMCI                     0
#define SAC_DO_DISTMEM_ALLOC_CACHE_OUTSIDE_DSM   0
#define SAC_DO_DISTMEM_PTR_DESC                  0
#define SAC_DO_DISTMEM_PTR_CACHE                 1

/*
 * Setup for Task Parallelism
 */
#define SAC_DO_FP                                0

/*
 * Debugging Support
 */
#ifndef SAC_DEBUG_RC
#define SAC_DEBUG_RC                             0
#endif

#define SAC_DO_CUDA_FORCE_INIT 1


/*
 *  Global Settings
 */

#define SAC_FORCE_DESC_SIZE -1

/*
 *  MUTC Backend Specific Settings
 */
#define SAC_MUTC_RC_PLACES  1
#define SAC_MUTC_FORCE_SPAWN_FLAGS

#define SAC_C_EXTERN           extern "C"



/*
 *  Global Settings
 */

#ifndef NULL
#  ifdef __cplusplus
#    define NULL         0
#  else
#    define NULL         (void*) 0
#  endif
#endif

#define SAC_SET_TMPDIR              "/tmp"
#define SAC_SET_INITIAL_MASTER_HEAPSIZE      1048576
#define SAC_SET_INITIAL_WORKER_HEAPSIZE      65536
#define SAC_SET_INITIAL_UNIFIED_HEAPSIZE     0

#ifndef SAC_SET_RTSPEC_THREADS
#define SAC_SET_RTSPEC_THREADS              1
#endif

#ifndef SAC_SET_MTMODE
#define SAC_SET_MTMODE               0
#endif

#define SAC_SET_CPU_BIND_STRATEGY 0
#define SAC_SET_BARRIER_TYPE               0
#define SAC_SET_SMART_DECISIONS            0
#define SAC_SET_SMART_FILENAME           "default"
#define SAC_SET_SMART_ARCH               "(null)"
#define SAC_SET_SMART_PERIOD               500
#ifndef SAC_SET_THREADS_MAX
#define SAC_SET_THREADS_MAX          128
#endif

#ifndef SAC_SET_THREADS
#define SAC_SET_THREADS              1
#endif

#ifndef SAC_OMP_ACTIVE_LEVEL
#define SAC_OMP_ACTIVE_LEVEL          1
#endif

#ifndef SAC_SET_MASTERCLASS
#define SAC_SET_MASTERCLASS          0
#endif

#define SAC_SET_NUM_SCHEDULERS       0

#define SAC_SET_CACHE_1_SIZE         -1
#define SAC_SET_CACHE_1_LINE         4
#define SAC_SET_CACHE_1_ASSOC        1
#define SAC_SET_CACHE_1_WRITEPOL     SAC_CS_default
#define SAC_SET_CACHE_1_MSCA_FACTOR  0.00

#define SAC_SET_CACHE_2_SIZE         -1
#define SAC_SET_CACHE_2_LINE         4
#define SAC_SET_CACHE_2_ASSOC        1
#define SAC_SET_CACHE_2_WRITEPOL     SAC_CS_default
#define SAC_SET_CACHE_2_MSCA_FACTOR  0.00

#define SAC_SET_CACHE_3_SIZE         -1
#define SAC_SET_CACHE_3_LINE         4
#define SAC_SET_CACHE_3_ASSOC        1
#define SAC_SET_CACHE_3_WRITEPOL     SAC_CS_default
#define SAC_SET_CACHE_3_MSCA_FACTOR  0.00

#define SAC_SET_CACHESIM_HOST        ""
#define SAC_SET_CACHESIM_FILE        "a.out.cs"
#define SAC_SET_CACHESIM_DIR         "/tmp"
#define SAC_SET_MAXFUN               0
#define SAC_SET_MAXFUNAP             1
#define SBLOCKSZ               16
#define LBLOCKSZ               256



/*
 *  Includes
 */


#include "sac.h"


#if SAC_OMP_MACROS

#include "omp.h"

#endif

#if SAC_CUDA_MACROS

#include <stdio.h>


#include <cuda.h>


#include <cuda_runtime.h>


#include <algorithm>

#endif

/*
 *  SAC-Program cuda.sac :
 */


/*
 *  type definitions
 */

SAC_ND_TYPEDEF ((SACt_sacprelude_d__SACarg, (SCL, (HID, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, )))))))))), SAC_hidden)

SAC_C_EXTERN SACt_sacprelude_d__SACarg SACARGcopy ( SACt_sacprelude_d__SACarg);
SAC_C_EXTERN void SACARGfree ( SACt_sacprelude_d__SACarg);



/*
 *  Global Definitions
 */

SAC_PF_DEFINE()
SAC_HM_DEFINE()


/*
 *  prototypes for locals (FUNDEFS)
 */

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__partitionSlicer__i_S__i_S__i_S__i_S__i_S, , 9, out, int, (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), out, int, (SAC_arg_2, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), out, int, (SAC_arg_3, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), out, int, (SAC_arg_4, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_min, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_max, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_axis, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_lb, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_ub, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__partitionSlicer__i_S__i_S__i_S__i_S__i_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_out ( (SAC_arg_2, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_out ( (SAC_arg_3, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_out ( (SAC_arg_4, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_min, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_max, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_axis, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_lb, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_ub, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__partitionIntersectMax__i_S__i_S, , 3, out, int, (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_PWLbound1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_ivmin, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__partitionIntersectMax__i_S__i_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_PWLbound1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_ivmin, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__isPartitionIntersectNull__i_S__i_S__i_S__i_S, , 5, out, bool, (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, int, (SACl_idxmin, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_idxmax, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_bound1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_bound2, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__isPartitionIntersectNull__i_S__i_S__i_S__i_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_idxmin, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_idxmax, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_bound1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_bound2, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___selSxADistmemLocal__i_S__i_S, , 3, out, int, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___selSxADistmemLocal__i_S__i_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___selSxADistmemLocal__i_S__f_S, , 3, out, float, (SAC_arg_1, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, float, (SACl_array, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___selSxADistmemLocal__i_S__f_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), float), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), float));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___selSxADistmemLocal__i_S__d_S, , 3, out, double, (SAC_arg_1, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, double, (SACl_array, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___selSxADistmemLocal__i_S__d_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), double), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), double));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___selSxADistmemLocal__i_S__bl_S, , 3, out, bool, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, bool, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___selSxADistmemLocal__i_S__bl_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___selSxADistmemLocal__i_S__c_S, , 3, out, unsigned char, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, unsigned char, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___selSxADistmemLocal__i_S__c_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), unsigned char), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), unsigned char));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___selSxADistmemLocal__i_S__b_S, , 3, out, byte, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, byte, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___selSxADistmemLocal__i_S__b_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), byte), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), byte));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___selSxADistmemLocal__i_S__s_S, , 3, out, short, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, short, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___selSxADistmemLocal__i_S__s_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), short), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), short));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___selSxADistmemLocal__i_S__l_S, , 3, out, long, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, long, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___selSxADistmemLocal__i_S__l_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), long), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), long));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___selSxADistmemLocal__i_S__ll_S, , 3, out, longlong, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, longlong, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___selSxADistmemLocal__i_S__ll_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), longlong), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), longlong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___selSxADistmemLocal__i_S__ub_S, , 3, out, ubyte, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, ubyte, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___selSxADistmemLocal__i_S__ub_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), ubyte), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), ubyte));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___selSxADistmemLocal__i_S__us_S, , 3, out, ushort, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, ushort, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___selSxADistmemLocal__i_S__us_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), ushort), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), ushort));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___selSxADistmemLocal__i_S__ui_S, , 3, out, uint, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, uint, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___selSxADistmemLocal__i_S__ui_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), uint), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), uint));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___selSxADistmemLocal__i_S__ul_S, , 3, out, ulong, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, ulong, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___selSxADistmemLocal__i_S__ul_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), ulong), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), ulong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___selSxADistmemLocal__i_S__ull_S, , 3, out, ulonglong, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, ulonglong, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___selSxADistmemLocal__i_S__ull_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), ulonglong), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), ulonglong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__partitionIntersectMin__i_S__i_S, , 3, out, int, (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_PWLbound2, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_ivmax, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__partitionIntersectMin__i_S__i_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_PWLbound2, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_ivmax, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__adjustLacFunParams__bl_S__i_S__i_S, , 4, out, int, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, bool, (SACl_p, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, int, (SACl_i, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_iv, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__adjustLacFunParams__bl_S__i_S__i_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_p, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_i, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_iv, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf__MAIN__main, , 1, out, int, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf__MAIN__main, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__prod__i_S, , 2, out, int, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_v, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__prod__i_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_v, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__adjustLacFunParamsReshape__bl_S__i_S__i_S__i_S, , 5, out, int, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, bool, (SACl_p, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, int, (SACl_i, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_iv, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_shp, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__adjustLacFunParamsReshape__bl_S__i_S__i_S__i_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_p, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_i, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_iv, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_shp, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__eq__i_S__i_S, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, int, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__eq__i_S__i_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__eq__f_S__f_S, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, float, (SACl_A, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), in, float, (SACl_B, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__eq__f_S__f_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), float), SAC_ND_PARAM_in ( (SACl_B, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), float));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__eq__d_S__d_S, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, double, (SACl_A, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), in, double, (SACl_B, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__eq__d_S__d_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), double), SAC_ND_PARAM_in ( (SACl_B, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), double));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__eq__bl_S__bl_S, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, bool, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, bool, (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__eq__bl_S__bl_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__eq__c_S__c_S, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, unsigned char, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), in, unsigned char, (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__eq__c_S__c_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), unsigned char), SAC_ND_PARAM_in ( (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), unsigned char));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__eq__b_S__b_S, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, byte, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), in, byte, (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__eq__b_S__b_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), byte), SAC_ND_PARAM_in ( (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), byte));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__eq__s_S__s_S, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, short, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), in, short, (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__eq__s_S__s_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), short), SAC_ND_PARAM_in ( (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), short));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__eq__l_S__l_S, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, long, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), in, long, (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__eq__l_S__l_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), long), SAC_ND_PARAM_in ( (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), long));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__eq__ll_S__ll_S, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, longlong, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), in, longlong, (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__eq__ll_S__ll_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), longlong), SAC_ND_PARAM_in ( (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), longlong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__eq__ub_S__ub_S, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, ubyte, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), in, ubyte, (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__eq__ub_S__ub_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), ubyte), SAC_ND_PARAM_in ( (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), ubyte));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__eq__us_S__us_S, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, ushort, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), in, ushort, (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__eq__us_S__us_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), ushort), SAC_ND_PARAM_in ( (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), ushort));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__eq__ui_S__ui_S, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, uint, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), in, uint, (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__eq__ui_S__ui_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), uint), SAC_ND_PARAM_in ( (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), uint));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__eq__ul_S__ul_S, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, ulong, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), in, ulong, (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__eq__ul_S__ul_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), ulong), SAC_ND_PARAM_in ( (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), ulong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__eq__ull_S__ull_S, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, ulonglong, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), in, ulonglong, (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__eq__ull_S__ull_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), ulonglong), SAC_ND_PARAM_in ( (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), ulonglong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__gridFiller__i_S__i_S__i_S__i_S__i_S, , 8, out, int, (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), out, int, (SAC_arg_2, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), out, int, (SAC_arg_3, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_lb, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_ub, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_wdth, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_dim, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_maxwidth, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__gridFiller__i_S__i_S__i_S__i_S__i_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_out ( (SAC_arg_2, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_out ( (SAC_arg_3, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_lb, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_ub, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_wdth, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_dim, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_maxwidth, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__isPartitionIntersect1Part__i_S__i_S__i_S__i_S, , 5, out, bool, (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, int, (SACl_idxmin, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_idxmax, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_bound1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_bound2, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__isPartitionIntersect1Part__i_S__i_S__i_S__i_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_idxmin, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_idxmax, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_bound1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_bound2, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__zero__i_S, , 2, out, int, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__zero__i_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__zero__d_S, , 2, out, double, (SAC_arg_1, (SCL, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), in, double, (SACl_A, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__zero__d_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), double), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), double));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__zero__f_S, , 2, out, float, (SAC_arg_1, (SCL, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), in, float, (SACl_A, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__zero__f_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), float), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), float));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__zero__c_S, , 2, out, unsigned char, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), in, unsigned char, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__zero__c_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), unsigned char), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), unsigned char));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__zero__bl_S, , 2, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, bool, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__zero__bl_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__zero__b_S, , 2, out, byte, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), in, byte, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__zero__b_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), byte), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), byte));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__zero__s_S, , 2, out, short, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), in, short, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__zero__s_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), short), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), short));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__zero__l_S, , 2, out, long, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), in, long, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__zero__l_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), long), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), long));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__zero__ll_S, , 2, out, longlong, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), in, longlong, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__zero__ll_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), longlong), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), longlong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__zero__ub_S, , 2, out, ubyte, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), in, ubyte, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__zero__ub_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), ubyte), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), ubyte));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__zero__us_S, , 2, out, ushort, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), in, ushort, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__zero__us_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), ushort), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), ushort));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__zero__ui_S, , 2, out, uint, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), in, uint, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__zero__ui_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), uint), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), uint));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__zero__ul_S, , 2, out, ulong, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), in, ulong, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__zero__ul_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), ulong), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), ulong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__zero__ull_S, , 2, out, ulonglong, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), in, ulonglong, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__zero__ull_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), ulonglong), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), ulonglong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__partitionMax__i_S__i_S, , 3, out, int, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_x, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_y, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__partitionMax__i_S__i_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_x, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_y, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__partitionMin__i_S__i_S, , 3, out, int, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_x, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_y, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__partitionMin__i_S__i_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_x, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_y, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___PL_PL__i_S, , 2, out, int, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_a, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___PL_PL__i_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_a, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__sel__i_S__i_S, , 3, out, int, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__sel__i_S__i_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__sel__i_S__f_S, , 3, out, float, (SAC_arg_1, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, float, (SACl_array, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__sel__i_S__f_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), float), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), float));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__sel__i_S__d_S, , 3, out, double, (SAC_arg_1, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, double, (SACl_array, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__sel__i_S__d_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), double), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), double));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__sel__i_S__bl_S, , 3, out, bool, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, bool, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__sel__i_S__bl_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__sel__i_S__c_S, , 3, out, unsigned char, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, unsigned char, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__sel__i_S__c_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), unsigned char), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), unsigned char));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__sel__i_S__b_S, , 3, out, byte, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, byte, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__sel__i_S__b_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), byte), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), byte));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__sel__i_S__s_S, , 3, out, short, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, short, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__sel__i_S__s_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), short), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), short));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__sel__i_S__l_S, , 3, out, long, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, long, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__sel__i_S__l_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), long), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), long));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__sel__i_S__ll_S, , 3, out, longlong, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, longlong, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__sel__i_S__ll_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), longlong), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), longlong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__sel__i_S__ub_S, , 3, out, ubyte, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, ubyte, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__sel__i_S__ub_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), ubyte), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), ubyte));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__sel__i_S__us_S, , 3, out, ushort, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, ushort, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__sel__i_S__us_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), ushort), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), ushort));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__sel__i_S__ui_S, , 3, out, uint, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, uint, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__sel__i_S__ui_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), uint), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), uint));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__sel__i_S__ul_S, , 3, out, ulong, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, ulong, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__sel__i_S__ul_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), ulong), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), ulong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d__sel__i_S__ull_S, , 3, out, ulonglong, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, ulonglong, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d__sel__i_S__ull_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), ulonglong), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), ulonglong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___selVxADistmemLocal__i_S__i_S, , 3, out, int, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___selVxADistmemLocal__i_S__i_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___selVxADistmemLocal__i_S__f_S, , 3, out, float, (SAC_arg_1, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, float, (SACl_array, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___selVxADistmemLocal__i_S__f_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), float), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), float));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___selVxADistmemLocal__i_S__d_S, , 3, out, double, (SAC_arg_1, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, double, (SACl_array, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___selVxADistmemLocal__i_S__d_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), double), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), double));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___selVxADistmemLocal__i_S__bl_S, , 3, out, bool, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, bool, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___selVxADistmemLocal__i_S__bl_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___selVxADistmemLocal__i_S__c_S, , 3, out, unsigned char, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, unsigned char, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___selVxADistmemLocal__i_S__c_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), unsigned char), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), unsigned char));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___selVxADistmemLocal__i_S__b_S, , 3, out, byte, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, byte, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___selVxADistmemLocal__i_S__b_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), byte), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), byte));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___selVxADistmemLocal__i_S__s_S, , 3, out, short, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, short, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___selVxADistmemLocal__i_S__s_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), short), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), short));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___selVxADistmemLocal__i_S__l_S, , 3, out, long, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, long, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___selVxADistmemLocal__i_S__l_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), long), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), long));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___selVxADistmemLocal__i_S__ll_S, , 3, out, longlong, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, longlong, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___selVxADistmemLocal__i_S__ll_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), longlong), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), longlong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___selVxADistmemLocal__i_S__ub_S, , 3, out, ubyte, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, ubyte, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___selVxADistmemLocal__i_S__ub_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), ubyte), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), ubyte));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___selVxADistmemLocal__i_S__us_S, , 3, out, ushort, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, ushort, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___selVxADistmemLocal__i_S__us_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), ushort), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), ushort));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___selVxADistmemLocal__i_S__ui_S, , 3, out, uint, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, uint, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___selVxADistmemLocal__i_S__ui_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), uint), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), uint));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___selVxADistmemLocal__i_S__ul_S, , 3, out, ulong, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, ulong, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___selVxADistmemLocal__i_S__ul_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), ulong), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), ulong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACwf_sacprelude_d___selVxADistmemLocal__i_S__ull_S, , 3, out, ulonglong, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), in, int, (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, ulonglong, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACwf_sacprelude_d___selVxADistmemLocal__i_S__ull_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), ulonglong), SAC_ND_PARAM_in ( (SACl_idx, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), ulonglong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf__MAIN__hiding, , 1, out, int, (SAC_arg_1, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf__MAIN__hiding, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf__MAIN__main, , 1, out, int, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf__MAIN__main, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__sel__i_X__ull_S, , 3, out, ulonglong, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), in, int, (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, ulonglong, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__sel__i_X__ull_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), ulonglong), SAC_ND_PARAM_in ( (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), ulonglong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__sel__i_X__ul_S, , 3, out, ulong, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), in, int, (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, ulong, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__sel__i_X__ul_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), ulong), SAC_ND_PARAM_in ( (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), ulong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__sel__i_X__ui_S, , 3, out, uint, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), in, int, (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, uint, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__sel__i_X__ui_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), uint), SAC_ND_PARAM_in ( (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), uint));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__sel__i_X__us_S, , 3, out, ushort, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), in, int, (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, ushort, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__sel__i_X__us_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), ushort), SAC_ND_PARAM_in ( (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), ushort));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__sel__i_X__ub_S, , 3, out, ubyte, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), in, int, (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, ubyte, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__sel__i_X__ub_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), ubyte), SAC_ND_PARAM_in ( (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), ubyte));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__sel__i_X__ll_S, , 3, out, longlong, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), in, int, (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, longlong, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__sel__i_X__ll_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), longlong), SAC_ND_PARAM_in ( (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), longlong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__sel__i_X__l_S, , 3, out, long, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), in, int, (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, long, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__sel__i_X__l_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), long), SAC_ND_PARAM_in ( (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), long));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__sel__i_X__s_S, , 3, out, short, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), in, int, (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, short, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__sel__i_X__s_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), short), SAC_ND_PARAM_in ( (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), short));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__sel__i_X__b_S, , 3, out, byte, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), in, int, (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, byte, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__sel__i_X__b_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), byte), SAC_ND_PARAM_in ( (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), byte));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__sel__i_X__c_S, , 3, out, unsigned char, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), in, int, (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, unsigned char, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__sel__i_X__c_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), unsigned char), SAC_ND_PARAM_in ( (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), unsigned char));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__sel__i_X__bl_S, , 3, out, bool, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, int, (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, bool, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__sel__i_X__bl_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__sel__i_X__d_S, , 3, out, double, (SAC_arg_1, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), in, int, (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, double, (SACl_array, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__sel__i_X__d_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), double), SAC_ND_PARAM_in ( (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), double));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__sel__i_X__f_S, , 3, out, float, (SAC_arg_1, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), in, int, (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, float, (SACl_array, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__sel__i_X__f_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), float), SAC_ND_PARAM_in ( (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), float));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__sel__i_X__i_S, , 3, out, int, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__sel__i_X__i_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selVxADistmemLocal__i_1__ull_S, , 3, out, ulonglong, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), in, int, (SACl_idx, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, ulonglong, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selVxADistmemLocal__i_1__ull_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), ulonglong), SAC_ND_PARAM_in ( (SACl_idx, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), ulonglong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selVxADistmemLocal__i_X__ull_S, , 3, out, ulonglong, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), in, int, (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, ulonglong, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selVxADistmemLocal__i_X__ull_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), ulonglong), SAC_ND_PARAM_in ( (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), ulonglong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selVxADistmemLocal__i_1__ul_S, , 3, out, ulong, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), in, int, (SACl_idx, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, ulong, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selVxADistmemLocal__i_1__ul_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), ulong), SAC_ND_PARAM_in ( (SACl_idx, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), ulong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selVxADistmemLocal__i_X__ul_S, , 3, out, ulong, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), in, int, (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, ulong, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selVxADistmemLocal__i_X__ul_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), ulong), SAC_ND_PARAM_in ( (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), ulong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selVxADistmemLocal__i_1__ui_S, , 3, out, uint, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), in, int, (SACl_idx, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, uint, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selVxADistmemLocal__i_1__ui_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), uint), SAC_ND_PARAM_in ( (SACl_idx, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), uint));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selVxADistmemLocal__i_X__ui_S, , 3, out, uint, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), in, int, (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, uint, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selVxADistmemLocal__i_X__ui_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), uint), SAC_ND_PARAM_in ( (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), uint));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selVxADistmemLocal__i_1__us_S, , 3, out, ushort, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), in, int, (SACl_idx, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, ushort, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selVxADistmemLocal__i_1__us_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), ushort), SAC_ND_PARAM_in ( (SACl_idx, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), ushort));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selVxADistmemLocal__i_X__us_S, , 3, out, ushort, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), in, int, (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, ushort, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selVxADistmemLocal__i_X__us_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), ushort), SAC_ND_PARAM_in ( (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), ushort));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selVxADistmemLocal__i_1__ub_S, , 3, out, ubyte, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), in, int, (SACl_idx, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, ubyte, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selVxADistmemLocal__i_1__ub_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), ubyte), SAC_ND_PARAM_in ( (SACl_idx, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), ubyte));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selVxADistmemLocal__i_X__ub_S, , 3, out, ubyte, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), in, int, (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, ubyte, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selVxADistmemLocal__i_X__ub_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), ubyte), SAC_ND_PARAM_in ( (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), ubyte));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selVxADistmemLocal__i_1__ll_S, , 3, out, longlong, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), in, int, (SACl_idx, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, longlong, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selVxADistmemLocal__i_1__ll_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), longlong), SAC_ND_PARAM_in ( (SACl_idx, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), longlong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selVxADistmemLocal__i_X__ll_S, , 3, out, longlong, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), in, int, (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, longlong, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selVxADistmemLocal__i_X__ll_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), longlong), SAC_ND_PARAM_in ( (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), longlong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selVxADistmemLocal__i_1__l_S, , 3, out, long, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), in, int, (SACl_idx, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, long, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selVxADistmemLocal__i_1__l_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), long), SAC_ND_PARAM_in ( (SACl_idx, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), long));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selVxADistmemLocal__i_X__l_S, , 3, out, long, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), in, int, (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, long, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selVxADistmemLocal__i_X__l_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), long), SAC_ND_PARAM_in ( (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), long));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selVxADistmemLocal__i_1__s_S, , 3, out, short, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), in, int, (SACl_idx, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, short, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selVxADistmemLocal__i_1__s_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), short), SAC_ND_PARAM_in ( (SACl_idx, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), short));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selVxADistmemLocal__i_X__s_S, , 3, out, short, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), in, int, (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, short, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selVxADistmemLocal__i_X__s_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), short), SAC_ND_PARAM_in ( (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), short));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selVxADistmemLocal__i_1__b_S, , 3, out, byte, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), in, int, (SACl_idx, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, byte, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selVxADistmemLocal__i_1__b_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), byte), SAC_ND_PARAM_in ( (SACl_idx, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), byte));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selVxADistmemLocal__i_X__b_S, , 3, out, byte, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), in, int, (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, byte, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selVxADistmemLocal__i_X__b_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), byte), SAC_ND_PARAM_in ( (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), byte));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selVxADistmemLocal__i_1__c_S, , 3, out, unsigned char, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), in, int, (SACl_idx, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, unsigned char, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selVxADistmemLocal__i_1__c_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), unsigned char), SAC_ND_PARAM_in ( (SACl_idx, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), unsigned char));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selVxADistmemLocal__i_X__c_S, , 3, out, unsigned char, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), in, int, (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, unsigned char, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selVxADistmemLocal__i_X__c_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), unsigned char), SAC_ND_PARAM_in ( (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), unsigned char));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selVxADistmemLocal__i_1__bl_S, , 3, out, bool, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, int, (SACl_idx, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, bool, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selVxADistmemLocal__i_1__bl_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_idx, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selVxADistmemLocal__i_X__bl_S, , 3, out, bool, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, int, (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, bool, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selVxADistmemLocal__i_X__bl_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selVxADistmemLocal__i_1__d_S, , 3, out, double, (SAC_arg_1, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), in, int, (SACl_idx, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, double, (SACl_array, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selVxADistmemLocal__i_1__d_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), double), SAC_ND_PARAM_in ( (SACl_idx, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), double));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selVxADistmemLocal__i_X__d_S, , 3, out, double, (SAC_arg_1, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), in, int, (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, double, (SACl_array, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selVxADistmemLocal__i_X__d_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), double), SAC_ND_PARAM_in ( (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), double));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selVxADistmemLocal__i_1__f_S, , 3, out, float, (SAC_arg_1, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), in, int, (SACl_idx, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, float, (SACl_array, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selVxADistmemLocal__i_1__f_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), float), SAC_ND_PARAM_in ( (SACl_idx, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), float));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selVxADistmemLocal__i_X__f_S, , 3, out, float, (SAC_arg_1, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), in, int, (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, float, (SACl_array, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selVxADistmemLocal__i_X__f_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), float), SAC_ND_PARAM_in ( (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), float));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selVxADistmemLocal__i_1__i_S, , 3, out, int, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_idx, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selVxADistmemLocal__i_1__i_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_idx, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selVxADistmemLocal__i_X__i_S, , 3, out, int, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selVxADistmemLocal__i_X__i_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_idx, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selSxADistmemLocal__i__ull_S, , 3, out, ulonglong, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), in, int, (SACl_idx, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, ulonglong, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selSxADistmemLocal__i__ull_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), ulonglong), SAC_ND_PARAM_in ( (SACl_idx, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), ulonglong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selSxADistmemLocal__i__ul_S, , 3, out, ulong, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), in, int, (SACl_idx, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, ulong, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selSxADistmemLocal__i__ul_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), ulong), SAC_ND_PARAM_in ( (SACl_idx, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), ulong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selSxADistmemLocal__i__ui_S, , 3, out, uint, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), in, int, (SACl_idx, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, uint, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selSxADistmemLocal__i__ui_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), uint), SAC_ND_PARAM_in ( (SACl_idx, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), uint));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selSxADistmemLocal__i__us_S, , 3, out, ushort, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), in, int, (SACl_idx, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, ushort, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selSxADistmemLocal__i__us_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), ushort), SAC_ND_PARAM_in ( (SACl_idx, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), ushort));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selSxADistmemLocal__i__ub_S, , 3, out, ubyte, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), in, int, (SACl_idx, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, ubyte, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selSxADistmemLocal__i__ub_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), ubyte), SAC_ND_PARAM_in ( (SACl_idx, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), ubyte));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selSxADistmemLocal__i__ll_S, , 3, out, longlong, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), in, int, (SACl_idx, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, longlong, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selSxADistmemLocal__i__ll_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), longlong), SAC_ND_PARAM_in ( (SACl_idx, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), longlong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selSxADistmemLocal__i__l_S, , 3, out, long, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), in, int, (SACl_idx, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, long, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selSxADistmemLocal__i__l_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), long), SAC_ND_PARAM_in ( (SACl_idx, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), long));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selSxADistmemLocal__i__s_S, , 3, out, short, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), in, int, (SACl_idx, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, short, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selSxADistmemLocal__i__s_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), short), SAC_ND_PARAM_in ( (SACl_idx, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), short));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selSxADistmemLocal__i__b_S, , 3, out, byte, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), in, int, (SACl_idx, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, byte, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selSxADistmemLocal__i__b_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), byte), SAC_ND_PARAM_in ( (SACl_idx, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), byte));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selSxADistmemLocal__i__c_S, , 3, out, unsigned char, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), in, int, (SACl_idx, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, unsigned char, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selSxADistmemLocal__i__c_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), unsigned char), SAC_ND_PARAM_in ( (SACl_idx, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), unsigned char));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selSxADistmemLocal__i__bl_S, , 3, out, bool, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, int, (SACl_idx, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, bool, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selSxADistmemLocal__i__bl_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_idx, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selSxADistmemLocal__i__d_S, , 3, out, double, (SAC_arg_1, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), in, int, (SACl_idx, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, double, (SACl_array, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selSxADistmemLocal__i__d_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), double), SAC_ND_PARAM_in ( (SACl_idx, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), double));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selSxADistmemLocal__i__f_S, , 3, out, float, (SAC_arg_1, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), in, int, (SACl_idx, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, float, (SACl_array, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selSxADistmemLocal__i__f_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), float), SAC_ND_PARAM_in ( (SACl_idx, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), float));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___selSxADistmemLocal__i__i_S, , 3, out, int, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_idx, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___selSxADistmemLocal__i__i_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_idx, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_array, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__zero__ull_S, , 2, out, ulonglong, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), in, ulonglong, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__zero__ull_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), ulonglong), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), ulonglong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__zero__ul_S, , 2, out, ulong, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), in, ulong, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__zero__ul_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), ulong), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), ulong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__zero__ui_S, , 2, out, uint, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), in, uint, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__zero__ui_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), uint), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), uint));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__zero__us_S, , 2, out, ushort, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), in, ushort, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__zero__us_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), ushort), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), ushort));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__zero__ub_S, , 2, out, ubyte, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), in, ubyte, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__zero__ub_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), ubyte), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), ubyte));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__zero__ll_S, , 2, out, longlong, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), in, longlong, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__zero__ll_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), longlong), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), longlong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__zero__l_S, , 2, out, long, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), in, long, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__zero__l_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), long), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), long));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__zero__s_S, , 2, out, short, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), in, short, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__zero__s_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), short), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), short));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__zero__b_S, , 2, out, byte, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), in, byte, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__zero__b_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), byte), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), byte));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__zero__bl_S, , 2, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, bool, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__zero__bl_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__zero__c_S, , 2, out, unsigned char, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), in, unsigned char, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__zero__c_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), unsigned char), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), unsigned char));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__zero__f_S, , 2, out, float, (SAC_arg_1, (SCL, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), in, float, (SACl_A, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__zero__f_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), float), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), float));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__zero__d_S, , 2, out, double, (SAC_arg_1, (SCL, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), in, double, (SACl_A, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__zero__d_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), double), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), double));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__zero__i_S, , 2, out, int, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__zero__i_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__eq__ull_S__ull_S, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, ulonglong, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), in, ulonglong, (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__eq__ull_S__ull_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), ulonglong), SAC_ND_PARAM_in ( (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULL, )))))))))), ulonglong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__eq__ul_S__ul_S, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, ulong, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), in, ulong, (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__eq__ul_S__ul_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), ulong), SAC_ND_PARAM_in ( (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (ULO, )))))))))), ulong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__eq__ui_S__ui_S, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, uint, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), in, uint, (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__eq__ui_S__ui_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), uint), SAC_ND_PARAM_in ( (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UIN, )))))))))), uint));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__eq__us_S__us_S, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, ushort, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), in, ushort, (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__eq__us_S__us_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), ushort), SAC_ND_PARAM_in ( (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (USH, )))))))))), ushort));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__eq__ub_S__ub_S, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, ubyte, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), in, ubyte, (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__eq__ub_S__ub_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), ubyte), SAC_ND_PARAM_in ( (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UBY, )))))))))), ubyte));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__eq__ll_S__ll_S, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, longlong, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), in, longlong, (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__eq__ll_S__ll_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), longlong), SAC_ND_PARAM_in ( (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LLO, )))))))))), longlong));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__eq__l_S__l_S, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, long, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), in, long, (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__eq__l_S__l_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), long), SAC_ND_PARAM_in ( (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (LON, )))))))))), long));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__eq__s_S__s_S, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, short, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), in, short, (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__eq__s_S__s_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), short), SAC_ND_PARAM_in ( (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (SHO, )))))))))), short));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__eq__b_S__b_S, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, byte, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), in, byte, (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__eq__b_S__b_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), byte), SAC_ND_PARAM_in ( (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BYT, )))))))))), byte));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__eq__c_S__c_S, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, unsigned char, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), in, unsigned char, (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__eq__c_S__c_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), unsigned char), SAC_ND_PARAM_in ( (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (UCH, )))))))))), unsigned char));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__eq__bl_S__bl_S, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, bool, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, bool, (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__eq__bl_S__bl_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__eq__d_S__d_S, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, double, (SACl_A, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), in, double, (SACl_B, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__eq__d_S__d_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), double), SAC_ND_PARAM_in ( (SACl_B, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (DOU, )))))))))), double));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__eq__f_S__f_S, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, float, (SACl_A, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), in, float, (SACl_B, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__eq__f_S__f_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), float), SAC_ND_PARAM_in ( (SACl_B, (AUD, (NHD, (NUQ, (FLO, (GLO, (FPM, (NOT, (NDI, (FLO, )))))))))), float));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__eq__i_S__i_S, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, int, (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__eq__i_S__i_S, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_A, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_B, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__adjustLacFunParams__bl_S__i_S__i_X, , 4, out, int, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, bool, (SACl_p, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, int, (SACl_i, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_iv, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__adjustLacFunParams__bl_S__i_S__i_X, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_p, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_i, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_iv, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__adjustLacFunParamsReshape__bl_S__i_S__i_X__i_X, , 5, out, int, (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, bool, (SACl_p, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, int, (SACl_i, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_iv, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_shp, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__adjustLacFunParamsReshape__bl_S__i_S__i_X__i_X, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_p, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_i, (AUD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_iv, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_shp, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__isPartitionIntersectNull__i_X__i_X__i_X__i_X, , 5, out, bool, (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, int, (SACl_idxmin, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_idxmax, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_bound1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_bound2, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__isPartitionIntersectNull__i_X__i_X__i_X__i_X, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_idxmin, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_idxmax, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_bound1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_bound2, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__isPartitionIntersectNull__i__i__i_X__i_X, , 5, out, bool, (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, int, (SACl_idxmin, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_idxmax, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_bound1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_bound2, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__isPartitionIntersectNull__i__i__i_X__i_X, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_idxmin, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_idxmax, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_bound1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_bound2, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__partitionMin__i__i, , 3, out, int, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_x, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_y, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__partitionMin__i__i, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_x, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_y, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__partitionMax__i__i, , 3, out, int, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_x, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_y, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__partitionMax__i__i, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_x, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_y, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__isPartitionIntersect1Part__i_X__i_X__i_X__i_X, , 5, out, bool, (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, int, (SACl_idxmin, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_idxmax, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_bound1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_bound2, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__isPartitionIntersect1Part__i_X__i_X__i_X__i_X, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_idxmin, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_idxmax, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_bound1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_bound2, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__isPartitionIntersect1Part__i__i__i_X__i_X, , 5, out, bool, (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, int, (SACl_idxmin, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_idxmax, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_bound1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_bound2, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__isPartitionIntersect1Part__i__i__i_X__i_X, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_idxmin, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_idxmax, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_bound1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_bound2, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__partitionIntersectMax__i_X__i_X, , 3, out, int, (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_PWLbound1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_ivmin, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__partitionIntersectMax__i_X__i_X, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_PWLbound1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_ivmin, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__partitionIntersectMax__i_X__i, , 3, out, int, (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_PWLbound1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_ivmin, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__partitionIntersectMax__i_X__i, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_PWLbound1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_ivmin, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__partitionIntersectMin__i_X__i_X, , 3, out, int, (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_PWLbound2, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_ivmax, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__partitionIntersectMin__i_X__i_X, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_PWLbound2, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_ivmax, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__partitionIntersectMin__i_X__i, , 3, out, int, (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_PWLbound2, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_ivmax, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__partitionIntersectMin__i_X__i, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_PWLbound2, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_ivmax, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__prod__i_X, , 2, out, int, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_v, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__prod__i_X, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_v, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__partitionSlicer__i_X__i_X__i__i_X__i_X, , 9, out, int, (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), out, int, (SAC_arg_2, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), out, int, (SAC_arg_3, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), out, int, (SAC_arg_4, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_min, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_max, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_axis, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_lb, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_ub, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__partitionSlicer__i_X__i_X__i__i_X__i_X, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_out ( (SAC_arg_2, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_out ( (SAC_arg_3, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_out ( (SAC_arg_4, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_min, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_max, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_axis, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_lb, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_ub, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__gridFiller__i_X__i_X__i_X__i__i_X, , 8, out, int, (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), out, int, (SAC_arg_2, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), out, int, (SAC_arg_3, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_lb, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_ub, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_wdth, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_dim, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_maxwidth, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__gridFiller__i_X__i_X__i_X__i__i_X, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_out ( (SAC_arg_2, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_out ( (SAC_arg_3, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_lb, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_ub, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_wdth, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_dim, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_maxwidth, (AKD, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___PL_PL__i, , 2, out, int, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_a, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___PL_PL__i, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_a, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d___ST__i__i, , 3, out, int, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_a, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), in, int, (SACl_b, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d___ST__i__i, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_a, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_ND_PARAM_in ( (SACl_b, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));

SAC_C_EXTERN
/*
 * ND_FUN_DECL( SACf_sacprelude_d__and__bl__bl, , 3, out, bool, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, bool, (SACl_a, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), in, bool, (SACl_b, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))))
 */
SAC_ND_DECL_FUN2 ( SACf_sacprelude_d__and__bl__bl, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_a, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool), SAC_ND_PARAM_in ( (SACl_b, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (BOO, )))))))))), bool));

SAC_C_EXTERN
/*
 * CUDA_GLOBALFUN_DECL( SACf__MAIN___cuknl_5648_CUDA__i__i__i__i__i__i__i__id_100_100_100, 8, inout, int, (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (FPO, (NOT, (NDI, (OTH, )))))))))), 3, in, int, (SACp_ub_2, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), 0, in, int, (SACp_ub_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), 0, in, int, (SACp_ub_0, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), 0, in, int, (SACp_lb_2, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), 0, in, int, (SACp_lb_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), 0, in, int, (SACp_lb_0, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), 0, in, int, (SACp_emal_4727__flat_13, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), 0)
 */
__global__ void SACf__MAIN___cuknl_5648_CUDA__i__i__i__i__i__i__i__id_100_100_100 (SAC_CUDA_PARAM_inout ( (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (FPO, (NOT, (NDI, (OTH, )))))))))), int), int SAC_ND_A_MIRROR_SHAPE ((SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (FPO, (NOT, (NDI, (OTH, )))))))))), 0), int SAC_ND_A_MIRROR_SHAPE ((SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (FPO, (NOT, (NDI, (OTH, )))))))))), 1), int SAC_ND_A_MIRROR_SHAPE ((SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (FPO, (NOT, (NDI, (OTH, )))))))))), 2), int SAC_ND_A_MIRROR_SIZE ((SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (FPO, (NOT, (NDI, (OTH, ))))))))))), int SAC_ND_A_MIRROR_DIM ((SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (FPO, (NOT, (NDI, (OTH, ))))))))))), SAC_CUDA_PARAM_in ( (SACp_ub_2, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_CUDA_PARAM_in ( (SACp_ub_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_CUDA_PARAM_in ( (SACp_ub_0, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_CUDA_PARAM_in ( (SACp_lb_2, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_CUDA_PARAM_in ( (SACp_lb_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_CUDA_PARAM_in ( (SACp_lb_0, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_CUDA_PARAM_in ( (SACp_emal_4727__flat_13, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int));
;



/*
 *  function definitions (FUNDEFS)
 */



/****************************************************************************
 * Wrapper function:
 * WITH-loop Count: 0
 * _MAIN::SACwf__MAIN__main(...) [ wrapper ]
 ****************************************************************************/
/*
 * ND_FUN_DEF_BEGIN( SACwf__MAIN__main, , 1, out, int, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DEF_FUN_BEGIN2 ( SACwf__MAIN__main, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int)) {
	SAC_HM_DEFINE_THREAD_STATUS ( SAC_HM_single_threaded)
	SAC_MT_DEFINE_ST_SELF()

	{
		/*
		 * ND_DECL( (SACp_cwc_25, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, 0)
		 */
		SAC_ND_DECL__DATA ( (SACp_cwc_25, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, )
		SAC_ND_DECL__DESC ( (SACp_cwc_25, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
		SAC_NOTHING()


		SAC_INIT_LOCAL_MEM()
		/*
		 * ND_FUN_AP( SACf__MAIN__main, , 1, out, int, SAC_SET_NT_USG( FAG, (SACp_cwc_25, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))))
		 */
		SAC_ND_FUNAP2 ( SACf__MAIN__main,  SAC_ND_ARG_out ( SAC_SET_NT_USG ( FAG, (SACp_cwc_25, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))), int))

		/*
		 * ND_REFRESH__MIRROR( (SACp_cwc_25, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0)
		 */
		SAC_NOOP()

		/*
		 * ND_FUN_RET( , 1, out, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), (SACp_cwc_25, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))))
		 */
		SAC_ND_RET_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), (SACp_cwc_25, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))))
		return;
		SAC_CLEANUP_LOCAL_MEM()
	}
	/*
	   * ND_FUN_DEF_END( SACwf__MAIN__main, , 1, out, int, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
	   */
}
SAC_ND_FUN_DEF_END2()



/****************************************************************************
 * WITH-loop Count: 0
 * _MAIN::SACf__MAIN__hiding(...) [ body ]
 ****************************************************************************/
/*
 * ND_FUN_DEF_BEGIN( SACf__MAIN__hiding, , 1, out, int, (SAC_arg_1, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DEF_FUN_BEGIN2 ( SACf__MAIN__hiding, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int)) {
	SAC_HM_DEFINE_THREAD_STATUS ( SAC_HM_single_threaded)
	SAC_MT_DEFINE_ST_SELF()

	{
		SAC_ND_DECL_CONST__DATA ((SACp_emal_4727__flat_13, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, 42)
		SAC_ND_DECL_CONST__DATA ((SACp_emal_4726__cnstass_4710_lb, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, 0)
		SAC_ND_DECL_CONST__DATA ((SACp_emal_4725__cnstass_4709_lb, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, 0)
		SAC_ND_DECL_CONST__DATA ((SACp_emal_4724__cnstass_4708_lb, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, 0)
		SAC_ND_DECL_CONST__DATA ((SACp_emal_4723__cnstass_4707_ub, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, 100)
		SAC_ND_DECL_CONST__DATA ((SACp_emal_4722__cnstass_4706_ub, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, 100)
		SAC_ND_DECL_CONST__DATA ((SACp_emal_4721__cnstass_4705_ub, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, 100)
		/*
		 * ND_DECL( (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, )))))))))), int, 3, 100, 100, 100)
		 */
		SAC_ND_DECL__DATA ( (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, )))))))))), int, )
		SAC_ND_DECL__DESC ( (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, )))))))))), )
		const int SAC_ND_A_MIRROR_SHAPE ( (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, )))))))))), 0) = 100;
		const int SAC_ND_A_MIRROR_SHAPE ( (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, )))))))))), 1) = 100;
		const int SAC_ND_A_MIRROR_SHAPE ( (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, )))))))))), 2) = 100;
		const int SAC_ND_A_MIRROR_SIZE ( (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, ))))))))))) = 1000000;
		const int SAC_ND_A_MIRROR_DIM ( (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, ))))))))))) = 3;

		/*
		 * ND_DECL( (SACp_emal_4718_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, 3, 100, 100, 100)
		 */
		SAC_ND_DECL__DATA ( (SACp_emal_4718_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, )
		SAC_ND_DECL__DESC ( (SACp_emal_4718_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
		const int SAC_ND_A_MIRROR_SHAPE ( (SACp_emal_4718_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0) = 100;
		const int SAC_ND_A_MIRROR_SHAPE ( (SACp_emal_4718_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 1) = 100;
		const int SAC_ND_A_MIRROR_SHAPE ( (SACp_emal_4718_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 2) = 100;
		const int SAC_ND_A_MIRROR_SIZE ( (SACp_emal_4718_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))) = 1000000;
		const int SAC_ND_A_MIRROR_DIM ( (SACp_emal_4718_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))) = 3;

		/*
		 * ND_DECL( (SACp_wlidx_4480_a, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, 0)
		 */
		SAC_ND_DECL__DATA ( (SACp_wlidx_4480_a, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, )
		SAC_ND_DECL__DESC ( (SACp_wlidx_4480_a, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
		SAC_NOTHING()

		/*
		 * ND_DECL( (SACp_eat_21, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, 0)
		 */
		SAC_ND_DECL__DATA ( (SACp_eat_21, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, )
		SAC_ND_DECL__DESC ( (SACp_eat_21, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
		SAC_NOTHING()

		/*
		 * ND_DECL( (SACp_eat_20, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, 0)
		 */
		SAC_ND_DECL__DATA ( (SACp_eat_20, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, )
		SAC_ND_DECL__DESC ( (SACp_eat_20, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
		SAC_NOTHING()

		/*
		 * ND_DECL( (SACp_eat_19, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, 0)
		 */
		SAC_ND_DECL__DATA ( (SACp_eat_19, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, )
		SAC_ND_DECL__DESC ( (SACp_eat_19, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
		SAC_NOTHING()


		SAC_INIT_LOCAL_MEM()
		SAC_CUDA_ALLOC_BEGIN ((SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, )))))))))), 1, 3, int)
		/*
		 * ND_SET__SHAPE_arr( (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, )))))))))), 3, 100, 100, 100)
		 */
		SAC_ASSURE_TYPE_LINE ((SAC_ND_A_DIM ( (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, ))))))))))) == 3), 4, "Assignment with incompatible types found!");
		SAC_ASSURE_TYPE_LINE ((SAC_ND_A_SHAPE ( (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, )))))))))), 0) == 100), 4, "Assignment with incompatible types found!");
		SAC_ASSURE_TYPE_LINE ((SAC_ND_A_SHAPE ( (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, )))))))))), 1) == 100), 4, "Assignment with incompatible types found!");
		SAC_ASSURE_TYPE_LINE ((SAC_ND_A_SHAPE ( (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, )))))))))), 2) == 100), 4, "Assignment with incompatible types found!");
		SAC_NOOP()

		SAC_CUDA_ALLOC_END ((SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, )))))))))), 1, 3, int)
		SAC_ND_ALLOC_BEGIN ((SACp_eat_21, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 1, 0, int)
		/*
		 * ND_SET__SHAPE_arr( (SACp_eat_21, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0)
		 */
		SAC_ASSURE_TYPE_LINE ((SAC_ND_A_DIM ( (SACp_eat_21, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))) == 0), 4, "Assignment with incompatible types found!");
		SAC_NOOP()

		SAC_ND_ALLOC_END ((SACp_eat_21, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 1, 0, int)
		SAC_ND_ALLOC_BEGIN ((SACp_eat_20, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 1, 0, int)
		/*
		 * ND_SET__SHAPE_arr( (SACp_eat_20, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0)
		 */
		SAC_ASSURE_TYPE_LINE ((SAC_ND_A_DIM ( (SACp_eat_20, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))) == 0), 4, "Assignment with incompatible types found!");
		SAC_NOOP()

		SAC_ND_ALLOC_END ((SACp_eat_20, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 1, 0, int)
		SAC_ND_ALLOC_BEGIN ((SACp_eat_19, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 1, 0, int)
		/*
		 * ND_SET__SHAPE_arr( (SACp_eat_19, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0)
		 */
		SAC_ASSURE_TYPE_LINE ((SAC_ND_A_DIM ( (SACp_eat_19, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))) == 0), 4, "Assignment with incompatible types found!");
		SAC_NOOP()

		SAC_ND_ALLOC_END ((SACp_eat_19, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 1, 0, int)
		SAC_ND_ALLOC_BEGIN ((SACp_wlidx_4480_a, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 1, 0, int)
		/*
		 * ND_SET__SHAPE_arr( (SACp_wlidx_4480_a, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0)
		 */
		SAC_ASSURE_TYPE_LINE ((SAC_ND_A_DIM ( (SACp_wlidx_4480_a, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))) == 0), 4, "Assignment with incompatible types found!");
		SAC_NOOP()

		SAC_ND_ALLOC_END ((SACp_wlidx_4480_a, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 1, 0, int)

		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5649_tmp)
		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5650_ub)
		SAC_GKCO_OPD_REDEFINE (SAC_ND_READ ( (SACp_emal_4722__cnstass_4706_ub, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0), SAC_gkco_prt_5650_ub)


		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5651_tmp)
		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5652_lb)
		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5653_ub)
		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5654_st)
		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5655_wi)
		SAC_GKCO_OPD_REDEFINE (SAC_gkco_prt_5653_ub, SAC_gkco_prt_5653_ub)

		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5656_lb)
		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5657_ub)
		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5658_st)
		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5659_wi)
		SAC_GKCO_OPD_REDEFINE (SAC_gkco_prt_5657_ub, SAC_gkco_prt_5657_ub)

		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5660_lb)
		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5661_ub)
		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5662_st)
		SAC_GKCO_OPD_DECLARE (SAC_gkco_prt_5663_wi)
		SAC_GKCO_OPD_REDEFINE (SAC_gkco_prt_5661_ub, SAC_gkco_prt_5661_ub)


		SAC_GKCO_HOST_OPD_COMPRESS_SW (SAC_gkco_prt_5653_ub, SAC_gkco_prt_5653_ub, SAC_gkco_prt_5654_st, SAC_gkco_prt_5655_wi, SAC_gkco_prt_5651_tmp)

		SAC_GKCO_HOST_OPD_COMPRESS_SW (SAC_gkco_prt_5661_ub, SAC_gkco_prt_5661_ub, SAC_gkco_prt_5662_st, SAC_gkco_prt_5663_wi, SAC_gkco_prt_5651_tmp)


		SAC_GKCO_HOST_OPM_SET_GRID (2147483647, 65535, 65535, 0, SAC_gkco_prt_5653_ub, SAC_gkco_prt_5657_ub)

		SAC_GKCO_HOST_OPM_SET_BLOCK (1024, 1024, 64, 1024, SAC_gkco_prt_5661_ub)



		/*
		 * CUDA_GLOBALFUN_AP( SACf__MAIN___cuknl_5648_CUDA__i__i__i__i__i__i__i__id_100_100_100, 8, inout, int, 3, SAC_SET_NT_USG( FAG, (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, ))))))))))), in, int, 0, SAC_SET_NT_USG( FAG, (SACp_emal_4723__cnstass_4707_ub, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))), in, int, 0, SAC_SET_NT_USG( FAG, (SACp_emal_4722__cnstass_4706_ub, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))), in, int, 0, SAC_SET_NT_USG( FAG, (SACp_emal_4721__cnstass_4705_ub, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))), in, int, 0, SAC_SET_NT_USG( FAG, (SACp_emal_4726__cnstass_4710_lb, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))), in, int, 0, SAC_SET_NT_USG( FAG, (SACp_emal_4725__cnstass_4709_lb, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))), in, int, 0, SAC_SET_NT_USG( FAG, (SACp_emal_4724__cnstass_4708_lb, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))), in, int, 0, SAC_SET_NT_USG( FAG, (SACp_emal_4727__flat_13, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))))
		 */
		SAC_TR_GPU_PRINT ("   kernel name \"SACf__MAIN___cuknl_5648_CUDA__i__i__i__i__i__i__i__id_100_100_100\"\n");
		SAC_PF_BEGIN_CUDA_KNL ();
		SACf__MAIN___cuknl_5648_CUDA__i__i__i__i__i__i__i__id_100_100_100 <<< grid, block>>> (    SAC_CUDA_ARG_inout ( SAC_SET_NT_USG ( FAG, (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, ))))))))))), int), SAC_ND_A_MIRROR_SHAPE (SAC_SET_NT_USG ( FAG, (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, ))))))))))), 0), SAC_ND_A_MIRROR_SHAPE (SAC_SET_NT_USG ( FAG, (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, ))))))))))), 1), SAC_ND_A_MIRROR_SHAPE (SAC_SET_NT_USG ( FAG, (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, ))))))))))), 2), SAC_ND_A_MIRROR_SIZE (SAC_SET_NT_USG ( FAG, (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, )))))))))))), SAC_ND_A_MIRROR_DIM (SAC_SET_NT_USG ( FAG, (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, )))))))))))),     SAC_CUDA_ARG_in ( SAC_SET_NT_USG ( FAG, (SACp_emal_4723__cnstass_4707_ub, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))), int),     SAC_CUDA_ARG_in ( SAC_SET_NT_USG ( FAG, (SACp_emal_4722__cnstass_4706_ub, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))), int),     SAC_CUDA_ARG_in ( SAC_SET_NT_USG ( FAG, (SACp_emal_4721__cnstass_4705_ub, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))), int),     SAC_CUDA_ARG_in ( SAC_SET_NT_USG ( FAG, (SACp_emal_4726__cnstass_4710_lb, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))), int),     SAC_CUDA_ARG_in ( SAC_SET_NT_USG ( FAG, (SACp_emal_4725__cnstass_4709_lb, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))), int),     SAC_CUDA_ARG_in ( SAC_SET_NT_USG ( FAG, (SACp_emal_4724__cnstass_4708_lb, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))), int),     SAC_CUDA_ARG_in ( SAC_SET_NT_USG ( FAG, (SACp_emal_4727__flat_13, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))), int));
		SAC_PF_END_CUDA_KNL ();
		SAC_CUDA_GET_LAST_KERNEL_ERROR();
	}

	/*
	 * ND_REFRESH__MIRROR( (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, )))))))))), 3)
	 */
	SAC_NOOP()

	SAC_ND_FREE ((SACp_emal_4723__cnstass_4707_ub, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
	SAC_ND_FREE ((SACp_emal_4722__cnstass_4706_ub, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
	SAC_ND_FREE ((SACp_emal_4721__cnstass_4705_ub, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
	SAC_ND_FREE ((SACp_emal_4726__cnstass_4710_lb, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
	SAC_ND_FREE ((SACp_emal_4725__cnstass_4709_lb, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
	SAC_ND_FREE ((SACp_emal_4724__cnstass_4708_lb, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
	SAC_ND_FREE ((SACp_eat_19, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
	SAC_ND_FREE ((SACp_eat_20, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
	SAC_ND_FREE ((SACp_eat_21, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
	SAC_ND_FREE ((SACp_emal_4727__flat_13, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
	SAC_ND_FREE ((SACp_wlidx_4480_a, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
	SAC_ND_ALLOC_BEGIN ((SACp_emal_4718_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 1, 3, int)
	/*
	 * ND_SET__SHAPE_arr( (SACp_emal_4718_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 3, 100, 100, 100)
	 */
	SAC_ASSURE_TYPE_LINE ((SAC_ND_A_DIM ( (SACp_emal_4718_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))) == 3), 4, "Assignment with incompatible types found!");
	SAC_ASSURE_TYPE_LINE ((SAC_ND_A_SHAPE ( (SACp_emal_4718_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0) == 100), 4, "Assignment with incompatible types found!");
	SAC_ASSURE_TYPE_LINE ((SAC_ND_A_SHAPE ( (SACp_emal_4718_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 1) == 100), 4, "Assignment with incompatible types found!");
	SAC_ASSURE_TYPE_LINE ((SAC_ND_A_SHAPE ( (SACp_emal_4718_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 2) == 100), 4, "Assignment with incompatible types found!");
	SAC_NOOP()

	SAC_ND_ALLOC_END ((SACp_emal_4718_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 1, 3, int)
	/*
	 * CUDA_MEM_TRANSFER( (SACp_emal_4718_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, )))))))))), int, cudaMemcpyDeviceToHost)
	 */
	SAC_ASSURE_TYPE_LINE ((SAC_ND_A_SIZE ( (SACp_emal_4718_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))) == SAC_ND_A_SIZE ( (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, )))))))))))), 4, "cudaMemcpy: Destionation and source arrays should have equal sizes!");
	SAC_TR_GPU_PRINT ("cudaMemcpyDeviceToHost size %d (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, )))))))))) -> (SACp_emal_4718_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))\n", SAC_ND_A_SIZE ( (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, ))))))))))));
	SAC_PF_BEGIN_CUDA_DtoH()
	SAC_CUDA_MEM_TRANSFER ((SACp_emal_4718_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, )))))))))), int, cudaMemcpyDeviceToHost)
	SAC_PF_END_CUDA_DtoH()

	SAC_CUDA_FREE ((SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, )))))))))), )
	/*
	 * ND_FUN_RET( , 1, out, (SAC_arg_1, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), (SACp_emal_4718_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))))
	 */
	SAC_ND_RET_out ( (SAC_arg_1, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), (SACp_emal_4718_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))))
	return;
	SAC_CLEANUP_LOCAL_MEM()
}
/*
   * ND_FUN_DEF_END( SACf__MAIN__hiding, , 1, out, int, (SAC_arg_1, (AKS, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
   */
}
SAC_ND_FUN_DEF_END2()



/****************************************************************************
 * WITH-loop Count: 0
 * _MAIN::SACf__MAIN__main(...) [ body ]
 ****************************************************************************/
/*
 * ND_FUN_DEF_BEGIN( SACf__MAIN__main, , 1, out, int, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
 */
SAC_ND_DEF_FUN_BEGIN2 ( SACf__MAIN__main, void,  SAC_ND_PARAM_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int)) {
	SAC_HM_DEFINE_THREAD_STATUS ( SAC_HM_single_threaded)
	SAC_MT_DEFINE_ST_SELF()

	{
		SAC_ND_DECL_CONST__DATA ((SACp_emal_4729__ivesplit_373, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, 232221)
		/*
		 * ND_DECL( (SACp_emal_4728__flat_14, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, 0)
		 */
		SAC_ND_DECL__DATA ( (SACp_emal_4728__flat_14, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, )
		SAC_ND_DECL__DESC ( (SACp_emal_4728__flat_14, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
		SAC_NOTHING()

		/*
		 * ND_DECL( (SACl_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, 3, 100, 100, 100)
		 */
		SAC_ND_DECL__DATA ( (SACl_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, )
		SAC_ND_DECL__DESC ( (SACl_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
		const int SAC_ND_A_MIRROR_SHAPE ( (SACl_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0) = 100;
		const int SAC_ND_A_MIRROR_SHAPE ( (SACl_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 1) = 100;
		const int SAC_ND_A_MIRROR_SHAPE ( (SACl_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 2) = 100;
		const int SAC_ND_A_MIRROR_SIZE ( (SACl_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))) = 1000000;
		const int SAC_ND_A_MIRROR_DIM ( (SACl_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))) = 3;


		SAC_INIT_LOCAL_MEM()
		/*
		 * ND_FUN_AP( SACf__MAIN__hiding, , 1, out, int, SAC_SET_NT_USG( FAG, (SACl_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))))
		 */
		SAC_ND_FUNAP2 ( SACf__MAIN__hiding,  SAC_ND_ARG_out ( SAC_SET_NT_USG ( FAG, (SACl_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))), int))

		/*
		 * ND_REFRESH__MIRROR( (SACl_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 3)
		 */
		SAC_NOOP()

		/*
		 * ND_PRF_IDX_SEL__DATA( (SACp_emal_4728__flat_14, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0, (SACl_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 3, (SACp_emal_4729__ivesplit_373, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
		 */
		SAC_TR_PRF_PRINT ( ("ND_PRF_IDX_SEL__DATA( (SACp_emal_4728__flat_14, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0, (SACl_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 3, (SACp_emal_4729__ivesplit_373, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))))"))
		SAC_ASSURE_TYPE_LINE ((SAC_ND_A_DIM ( (SACp_emal_4729__ivesplit_373, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))) == 0), 16, "1st argument of _idx_sel_ is not a scalar!");
		SAC_ND_WRITE_READ_COPY ( (SACp_emal_4728__flat_14, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0, (SACl_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), SAC_ND_READ ( (SACp_emal_4729__ivesplit_373, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0), )

		SAC_ND_FREE ((SACl_a, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
		SAC_ND_FREE ((SACp_emal_4729__ivesplit_373, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
		/*
		 * ND_FUN_RET( , 1, out, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), (SACp_emal_4728__flat_14, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))))
		 */
		SAC_ND_RET_out ( (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), (SACp_emal_4728__flat_14, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))))
		return;
		SAC_CLEANUP_LOCAL_MEM()
	}
	/*
	   * ND_FUN_DEF_END( SACf__MAIN__main, , 1, out, int, (SAC_arg_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))))
	   */
}
SAC_ND_FUN_DEF_END2()



/****************************************************************************
 * WITH-loop Count: 0
 * _MAIN::SACf__MAIN___cuknl_5648_CUDA__i__i__i__i__i__i__i__id_100_100_100(...) [ body ]
 ****************************************************************************/
/*
 * CUDA_GLOBALFUN_DEF_BEGIN( SACf__MAIN___cuknl_5648_CUDA__i__i__i__i__i__i__i__id_100_100_100, 8, inout, int, (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (FPO, (NOT, (NDI, (OTH, )))))))))), 3, in, int, (SACp_ub_2, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), 0, in, int, (SACp_ub_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), 0, in, int, (SACp_ub_0, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), 0, in, int, (SACp_lb_2, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), 0, in, int, (SACp_lb_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), 0, in, int, (SACp_lb_0, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), 0, in, int, (SACp_emal_4727__flat_13, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), 0)
 */
__global__ void SACf__MAIN___cuknl_5648_CUDA__i__i__i__i__i__i__i__id_100_100_100 (SAC_CUDA_PARAM_inout ( (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (FPO, (NOT, (NDI, (OTH, )))))))))), int), int SAC_ND_A_MIRROR_SHAPE ((SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (FPO, (NOT, (NDI, (OTH, )))))))))), 0), int SAC_ND_A_MIRROR_SHAPE ((SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (FPO, (NOT, (NDI, (OTH, )))))))))), 1), int SAC_ND_A_MIRROR_SHAPE ((SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (FPO, (NOT, (NDI, (OTH, )))))))))), 2), int SAC_ND_A_MIRROR_SIZE ((SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (FPO, (NOT, (NDI, (OTH, ))))))))))), int SAC_ND_A_MIRROR_DIM ((SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (FPO, (NOT, (NDI, (OTH, ))))))))))), SAC_CUDA_PARAM_in ( (SACp_ub_2, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_CUDA_PARAM_in ( (SACp_ub_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_CUDA_PARAM_in ( (SACp_ub_0, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_CUDA_PARAM_in ( (SACp_lb_2, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_CUDA_PARAM_in ( (SACp_lb_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_CUDA_PARAM_in ( (SACp_lb_0, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int), SAC_CUDA_PARAM_in ( (SACp_emal_4727__flat_13, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), int)) {

	{
		/*
		 * ND_DECL( (SACp_wlidx_4480_a, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, 0)
		 */
		SAC_ND_DECL__DATA ( (SACp_wlidx_4480_a, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, )
		SAC_ND_DECL__DESC ( (SACp_wlidx_4480_a, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
		SAC_NOTHING()

		/*
		 * ND_DECL( (SACp_eat_19, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, 0)
		 */
		SAC_ND_DECL__DATA ( (SACp_eat_19, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, )
		SAC_ND_DECL__DESC ( (SACp_eat_19, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
		SAC_NOTHING()

		/*
		 * ND_DECL( (SACp_eat_20, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, 0)
		 */
		SAC_ND_DECL__DATA ( (SACp_eat_20, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, )
		SAC_ND_DECL__DESC ( (SACp_eat_20, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
		SAC_NOTHING()

		/*
		 * ND_DECL( (SACp_eat_21, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, 0)
		 */
		SAC_ND_DECL__DATA ( (SACp_eat_21, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, )
		SAC_ND_DECL__DESC ( (SACp_eat_21, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
		SAC_NOTHING()

		/*
		 * CUDA_DECL_KERNEL_ARRAY( (SACl_iv, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, 1, 3)
		 */
		int SAC_ND_A_FIELD ( (SACl_iv, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))))[3];
		SAC_ND_DECL__DESC ( (SACl_iv, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
		const int SAC_ND_A_MIRROR_SHAPE ( (SACl_iv, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0) = 3;
		const int SAC_ND_A_MIRROR_SIZE ( (SACl_iv, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))) = 3;
		const int SAC_ND_A_MIRROR_DIM ( (SACl_iv, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, ))))))))))) = 1;


		SAC_INIT_LOCAL_MEM()
		/*
		 * CUDA_WLIDS( (SACp_eat_19, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0, 3, 2, (SACl_iv, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), false)
		 */
		SAC_CUDA_WLIDS_HD ( (SACp_eat_19, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0, THREADIDX_X, SACp_step_2, SACp_width_2, SACp_lb_2, SACp_ub_2)
		SAC_ND_WRITE ( (SACl_iv, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 2) = SAC_ND_READ ( (SACp_eat_19, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0);

		/*
		 * CUDA_WLIDS( (SACp_eat_20, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0, 3, 1, (SACl_iv, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), false)
		 */
		SAC_CUDA_WLIDS_HD ( (SACp_eat_20, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0, BLOCKIDX_X, SACp_step_1, SACp_width_1, SACp_lb_1, SACp_ub_1)
		SAC_ND_WRITE ( (SACl_iv, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 1) = SAC_ND_READ ( (SACp_eat_20, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0);

		/*
		 * CUDA_WLIDS( (SACp_eat_21, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0, 3, 0, (SACl_iv, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), false)
		 */
		SAC_CUDA_WLIDS_HD ( (SACp_eat_21, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0, BLOCKIDX_Y, SACp_step_0, SACp_width_0, SACp_lb_0, SACp_ub_0)
		SAC_ND_WRITE ( (SACl_iv, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0) = SAC_ND_READ ( (SACp_eat_21, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0);

		/*
		 * ND_IDXS2OFFSET_arr( (SACp_wlidx_4480_a, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 3, (SACp_eat_21, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), (SACp_eat_20, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), (SACp_eat_19, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 3, 100, 100, 100)
		 */
		SAC_ND_WRITE ( (SACp_wlidx_4480_a, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0) = ( 100 * ( 100 * SAC_ND_READ ( (SACp_eat_21, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0) + SAC_ND_READ ( (SACp_eat_20, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0) ) + SAC_ND_READ ( (SACp_eat_19, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0) );

		/*
		 * CUDA_WL_ASSIGN( (SACp_emal_4727__flat_13, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0, (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, )))))))))), 3, (SACp_wlidx_4480_a, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))))
		 */
		SAC_ND_WRITE_READ_COPY ( (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (OTH, )))))))))), SAC_ND_READ ( (SACp_wlidx_4480_a, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0), (SACp_emal_4727__flat_13, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0, );

		SAC_ND_FREE ((SACp_wlidx_4480_a, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
		SAC_ND_FREE ((SACp_eat_19, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
		SAC_ND_FREE ((SACp_eat_20, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
		SAC_ND_FREE ((SACp_eat_21, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
		SAC_NOOP()

		SAC_CLEANUP_LOCAL_MEM()
	}
	/*
	 * CUDA_GLOBALFUN_DEF_END( SACf__MAIN___cuknl_5648_CUDA__i__i__i__i__i__i__i__id_100_100_100, 8, inout, int, (SACp_emal_4719__iwlmem_4704_dev, (AKS, (NHD, (NUQ, (INT, (GLO, (FPO, (NOT, (NDI, (OTH, )))))))))), 3, in, int, (SACp_ub_2, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), 0, in, int, (SACp_ub_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), 0, in, int, (SACp_ub_0, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), 0, in, int, (SACp_lb_2, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), 0, in, int, (SACp_lb_1, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), 0, in, int, (SACp_lb_0, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), 0, in, int, (SACp_emal_4727__flat_13, (SCL, (NHD, (NUQ, (INT, (GLO, (FPM, (NOT, (NDI, (INT, )))))))))), 0)
	 */
}

/*
 * stubs for SACARGfreeDataUdt and SACARGcopyDataUdt
 */
extern "C" void SACARGfreeDataUdt ( int, void *);
extern "C" void *SACARGcopyDataUdt ( int, int, void *);
void SACARGfreeDataUdt ( int size, void *data) {}
void *SACARGcopyDataUdt ( int type, int size, void *data) {
	return ((void *) 0x0);
}

int main ( int __argc, char *__argv[]) {
	SAC_ND_DECL__DATA ( (SAC_res, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int, )
	SAC_ND_DECL__DESC ( (SAC_res, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), )
	SAC_NOTHING()
	SAC_HWLOC_SETUP();
	SAC_MT_SETUP_INITIAL();
	SAC_PF_SETUP();
	SAC_HM_SETUP();
	SAC_MT_SETUP();
	SAC_CUDA_SETUP();
	SAC_CS_SETUP();
	SAC_COMMANDLINE_SET ( __argc, __argv);

	SAC_INVOKE_MAIN_FUN ( SACf__MAIN__main, SAC_ND_ARG_out ( (SAC_res, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), int));

	SAC_DISTMEM_BARRIER();
	SAC_PF_PRINT();
	SAC_CS_FINALIZE();
	SAC_MT_FINALIZE();
	SAC_CUDA_FINALIZE();
	SAC_HWLOC_FINALIZE();
	SAC_HM_PRINT();

	return ( SAC_ND_READ ( (SAC_res, (SCL, (NHD, (NUQ, (INT, (GLO, (NON, (NOT, (NDI, (INT, )))))))))), 0));
}
