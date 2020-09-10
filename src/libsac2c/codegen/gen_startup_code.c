/*****************************************************************************
 *
 * file:   gen_startup_code.c
 *
 * prefix: GSC
 *
 * description:
 *
 *   This file provides routines that write C code that is rather independent
 *   from the specific SAC source file. It is essentially required to specify
 *   and initialize the SAC runtime system. For this reason, additional code
 *   is produced at 3 different positions: at the beginning of each C source,
 *   at the beginning of the statement sequence of the main() function, and
 *   right before the return() statement of the main() function.
 *
 *****************************************************************************/

#include <stdio.h>

#include <stdlib.h>

#include "free.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "globals.h"
#include "resource.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "NameTuplesUtils.h"
#include "icm2c_std.h"
#include "print.h"
#include "gen_startup_code.h"
#include "str.h"
#include "memory.h"
#include "renameidentifiers.h"
#include "namespaces.h"
#include "rtspec_modes.h"

/******************************************************************************
 *
 * function:
 *   int CalcMasterclass(int num_threads)
 *
 * description:
 *   For a given number of thread used, this function calculates the worker
 *   class of the master thread, i.e. the number of worker threads the master
 *   thread has to synchronize itslef with.
 *
 ******************************************************************************/

static int
CalcMasterclass (int num_threads)
{
    unsigned int res;

    DBUG_ENTER ();

    for (res = 1; res < (unsigned int)num_threads; res <<= 1)
        ;

    res >>= 1;

    DBUG_RETURN ((int)res);
}

/******************************************************************************
 *
 * function:
 *   void PrintGlobalSwitches()
 *
 * description:
 *   This function prints macro definitions representing global switches, i.e.
 *   their value always is either 0 (disabled) or 1 (enabled). The value of
 *   any switch triggers the selection of custom code during the inclusion
 *   of the standard header file sac.h.
 *
 ******************************************************************************/

static void
PrintGlobalSwitches (void)
{
    DBUG_ENTER ();

    fprintf (global.outfile, "\n\n"
                             "/*\n"
                             " *  Global Switches\n */\n\n");

    fprintf (global.outfile, "#define SAC_DO_CHECK           %d\n",
             (global.doruntimecheck) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_CHECK_TYPE      %d\n",
             (global.runtimecheck.type) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_CHECK_GPU       %d\n",
             (global.runtimecheck.gpu) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_CHECK_BOUNDARY   %d\n",
             (global.runtimecheck.boundary) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_CHECK_MALLOC     %d\n",
             (global.runtimecheck.malloc) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_CHECK_ERRNO      %d\n",
             (global.runtimecheck.checkerrno) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_CHECK_HEAP       %d\n",
             (global.runtimecheck.heap) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_CHECK_DISTMEM    %d\n",
             (global.runtimecheck.distmem) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_CHECK_DISTMEMPHM %d\n",
             (global.runtimecheck.distmemphm) ? 1 : 0);
    fprintf (global.outfile, "\n");

    fprintf (global.outfile, "#define SAC_DO_PHM             %d\n",
             (global.optimize.dophm) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_APS             %d\n",
             (global.optimize.doaps) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_DAO             %d\n",
             (global.optimize.dodpa) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_MSCA            %d\n",
             (global.optimize.domsca) ? 1 : 0);
    fprintf (global.outfile, "\n");

    fprintf (global.outfile, "#define SAC_DO_PROFILE         %d\n",
             (global.doprofile) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_PROFILE_WITH    %d\n",
             (global.profile.with) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_PROFILE_FUN     %d\n",
             (global.profile.fun) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_PROFILE_INL     %d\n",
             (global.profile.inl) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_PROFILE_LIB     %d\n",
             (global.profile.lib) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_PROFILE_MEM     %d\n",
             (global.profile.mem) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_PROFILE_DISTMEM %d\n",
             (global.profile.distmem) ? 1 : 0);
    fprintf (global.outfile, "\n");

    fprintf (global.outfile, "#ifndef SAC_DO_TRACE\n"
                             "#define SAC_DO_TRACE           %d\n"
                             "#endif\n",
             (global.dotrace) ? 1 : 0);
    fprintf (global.outfile, "#ifndef SAC_DO_TRACE_REF\n"
                             "#define SAC_DO_TRACE_REF       %d\n"
                             "#endif\n",
             (global.trace.ref) ? 1 : 0);
    fprintf (global.outfile, "#ifndef SAC_DO_TRACE_MEM\n"
                             "#define SAC_DO_TRACE_MEM       %d\n"
                             "#endif\n",
             (global.trace.mem) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_TRACE_PRF       %d\n",
             (global.trace.prf) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_TRACE_FUN       %d\n",
             (global.trace.fun) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_TRACE_WL        %d\n",
             (global.trace.wl) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_TRACE_AA        %d\n",
             (global.trace.aa) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_TRACE_MT        %d\n",
             (global.trace.mt) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_TRACE_GPU       %d\n",
             (global.trace.gpu) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_TRACE_RTSPEC    %d\n",
             (global.trace.rtspec) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_TRACE_DISTMEM   %d\n",
             (global.trace.distmem) ? 1 : 0);

    fprintf (global.outfile, "#define SAC_DO_CACHESIM        %d\n",
             (global.docachesim) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_CACHESIM_ADV    %d\n",
             (global.cachesim.advanced) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_CACHESIM_GLOBAL %d\n",
             (global.cachesim.block) ? 0 : 1);
    fprintf (global.outfile, "#define SAC_DO_CACHESIM_FILE   %d\n",
             (global.cachesim.file) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_CACHESIM_PIPE   %d\n",
             (global.cachesim.pipe) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_CACHESIM_IMDT   %d\n",
             (global.cachesim.immediate) ? 1 : 0);
    fprintf (global.outfile, "\n");

    fprintf (global.outfile, "#define SAC_DO_MULTITHREAD     %d\n",
             (global.num_threads == 1) ? 0 : 1);

    fprintf (global.outfile, "#define SAC_DO_MT_PTHREAD      %d\n",
             ((global.mtmode != MT_none) && STReq (global.config.mt_lib, "pthread")) ? 1
                                                                                     : 0);

    fprintf (global.outfile, "#define SAC_DO_MT_LPEL         %d\n",
             ((global.mtmode != MT_none) && STReq (global.config.mt_lib, "lpel")) ? 1
                                                                                  : 0);

    fprintf (global.outfile, "#define SAC_DO_MT_OMP          %d\n",
             (global.backend == BE_omp) ? 1 : 0);

    fprintf (global.outfile, "#define SAC_DO_DISTMEM         %d\n",
             (global.backend == BE_distmem) ? 1 : 0);

    fprintf (global.outfile, "#define SAC_DO_DISTMEM_GASNET  %d\n",
             (global.backend == BE_distmem
              && global.distmem_commlib == DISTMEM_COMMLIB_GASNET)
               ? 1
               : 0);

    fprintf (global.outfile, "#define SAC_DO_DISTMEM_GPI     %d\n",
             (global.backend == BE_distmem
              && global.distmem_commlib == DISTMEM_COMMLIB_GPI)
               ? 1
               : 0);

    fprintf (global.outfile, "#define SAC_DO_DISTMEM_MPI     %d\n",
             (global.backend == BE_distmem
              && global.distmem_commlib == DISTMEM_COMMLIB_MPI)
               ? 1
               : 0);

    fprintf (global.outfile, "#define SAC_DO_DISTMEM_ARMCI   %d\n",
             (global.backend == BE_distmem
              && global.distmem_commlib == DISTMEM_COMMLIB_ARMCI)
               ? 1
               : 0);

    fprintf (global.outfile, "#define SAC_DO_DISTMEM_ALLOC_CACHE_OUTSIDE_DSM %d\n",
             global.distmem_cache_outside_dsm);

    fprintf (global.outfile, "#define SAC_DO_DISTMEM_PTR_DESC %d\n",
             (global.backend == BE_distmem && global.distmem_ptrs_desc) ? 1 : 0);

    fprintf (global.outfile, "#define SAC_DO_DISTMEM_PTR_CACHE %d\n",
             (global.backend == BE_distmem && global.distmem_ptr_cache) ? 1 : 0);

    fprintf (global.outfile, "#define SAC_DO_THREADS_STATIC  %d\n",
             (global.num_threads == 0) ? 0 : 1);

    fprintf (global.outfile, "#define SAC_DO_FP              %d\n",
             (global.fp == 0) ? 0 : 1);

    fprintf (global.outfile, "#define SAC_DO_MT_CREATE_JOIN  %d\n",
             (global.mtmode == MT_createjoin) ? 1 : 0);

    fprintf (global.outfile, "#define SAC_DEBUG_RC           %d\n",
             global.debug_rc ? 1 : 0);

    fprintf (global.outfile, "\n\n"
                             "/*\n"
                             " *  Global Settings\n */\n\n");

    fprintf (global.outfile, "#define SAC_FORCE_DESC_SIZE %d\n", global.force_desc_size);

    /* MUTC Switches */
    fprintf (global.outfile, "\n\n"
                             "/*\n"
                             " *  MUTC Backend Specific Switches\n */\n\n");

    fprintf (global.outfile, "#define SAC_MUTC_FUNAP_AS_CREATE  %d\n",
             (global.mutc_fun_as_threads) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_MUTC_THREAD_MALLOC %d\n",
             (global.mutc_thread_mem) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_MUTC_DISABLE_THREAD_MEM %d\n",
             (global.mutc_disable_thread_mem) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_MUTC_BENCH %d\n",
             (global.mutc_benchmark) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_MUTC_MACROS  %d\n",
             (global.backend == BE_mutc) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_MUTC_RC_PLACES  %d\n", global.mutc_rc_places);
    fprintf (global.outfile, "#define SAC_MUTC_RC_INDIRECT  %d\n",
             (global.mutc_rc_indirect) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_MUTC_SEQ_DATA_PARALLEL  %d\n",
             (global.mutc_seq_data_parallel) ? 1 : 0);
    if (global.mutc_force_spawn_flags != NULL) {
        fprintf (global.outfile, "#define SAC_MUTC_FORCE_SPAWN_FLAGS  %s\n",
                 global.mutc_force_spawn_flags);
    } else {
        fprintf (global.outfile, "#define SAC_MUTC_FORCE_SPAWN_FLAGS\n");
    }

    fprintf (global.outfile, "\n");

    fprintf (global.outfile, "#define SAC_CUDA_MACROS  %d\n",
             (global.backend == BE_cuda || global.backend == BE_cudahybrid) ? 1 : 0);

    fprintf (global.outfile, "#define SAC_OMP_MACROS  %d\n",
             (global.backend == BE_omp) ? 1 : 0);

    fprintf (global.outfile, "\n");

    fprintf (global.outfile, "#define SAC_DO_COMPILE_MODULE  %d\n",
             ((global.filetype == FT_modimp) || (global.filetype == FT_classimp)
              || (global.filetype == FT_cmod))
               ? 1
               : 0);

    fprintf (global.outfile, "#define SAC_C_EXTERN           %s\n",
             (global.backend == BE_mutc)
               ? ""
               : (global.backend == BE_cuda || global.backend == BE_cudahybrid)
                   ? "extern \"C\""
                   : "extern");
    fprintf (global.outfile, "\n");

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void PrintProfileData()
 *
 * description:
 *   This function prints macro definitions related to the profiling feature.
 *
 ******************************************************************************/

static void
PrintProfileData (void)
{
    size_t i;
    int j;

    DBUG_ENTER ();

    fprintf (global.outfile, "#define SAC_SET_FUN_NAMES    \\\n");
    fprintf (global.outfile, "  {    \\\n");
    fprintf (global.outfile, "    \"%s\"", (global.profile_funnme[0]));
    for (i = 1; i < global.profile_funcntr; i++) {
        fprintf (global.outfile,
                 ",   \\\n"
                 "    \"%s\"",
                 global.profile_funnme[i]);
    };
    fprintf (global.outfile, "   \\\n"
                             "  }\n");

    fprintf (global.outfile, "\n");

    fprintf (global.outfile, "#define SAC_SET_FUN_APPS    \\\n");
    fprintf (global.outfile, "  {    \\\n");
    fprintf (global.outfile, "    %d", global.profile_funapcntr[0]);
    for (i = 1; i < global.profile_funcntr; i++) {
        fprintf (global.outfile,
                 ",   \\\n"
                 "    %d",
                 global.profile_funapcntr[i]);
    }
    fprintf (global.outfile, "   \\\n"
                             "  }\n");

    fprintf (global.outfile, "\n");

    fprintf (global.outfile, "#define SAC_SET_FUN_AP_LINES    \\\n");
    fprintf (global.outfile, "  {    \\\n");
    fprintf (global.outfile, "    {    \\\n");
    fprintf (global.outfile, "      %zu", global.profile_funapline[0][0]);
    for (j = 1; j < global.profile_funapcntr[0]; j++) {
        fprintf (global.outfile, ", %zu", global.profile_funapline[0][j]);
    }
    fprintf (global.outfile, "   \\\n"
                             "    }");
    for (i = 1; i < global.profile_funcntr; i++) {
        fprintf (global.outfile, ",   \\\n"
                                 "    {     \\\n");
        fprintf (global.outfile, "      %zu", global.profile_funapline[i][0]);
        for (j = 1; j < global.profile_funapcntr[i]; j++) {
            fprintf (global.outfile, ", %zu", global.profile_funapline[i][j]);
        }
        fprintf (global.outfile, "   \\\n"
                                 "    }");
    }
    fprintf (global.outfile, "   \\\n"
                             "  }");

    fprintf (global.outfile, "\n");

    fprintf (global.outfile, "#define SAC_SET_FUN_PARENTS    \\\n");
    fprintf (global.outfile, "  {    \\\n");
    fprintf (global.outfile, "    {    \\\n");
    fprintf (global.outfile, "      %zu", global.profile_parentfunno[0][0]);
    for (j = 1; j < global.profile_funapcntr[0]; j++) {
        fprintf (global.outfile, ", %zu", global.profile_parentfunno[0][j]);
    }
    fprintf (global.outfile, "   \\\n"
                             "    }");
    for (i = 1; i < global.profile_funcntr; i++) {
        fprintf (global.outfile, ",   \\\n"
                                 "    {     \\\n");
        fprintf (global.outfile, "      %zu", global.profile_parentfunno[i][0]);
        for (j = 1; j < global.profile_funapcntr[i]; j++) {
            fprintf (global.outfile, ", %zu", global.profile_parentfunno[i][j]);
        }
        fprintf (global.outfile, "   \\\n"
                                 "    }");
    }
    fprintf (global.outfile, "   \\\n"
                             "  }");

    fprintf (global.outfile, "\n");

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void PrintGlobalSettings( node *syntax_tree)
 *
 * description:
 *
 *
 ******************************************************************************/

static void
PrintGlobalSettings (node *syntax_tree)
{
    DBUG_ENTER ();

    fprintf (global.outfile, "\n\n/*\n *  Global Settings\n */\n\n");

    fprintf (global.outfile, "#ifndef NULL\n"
                             "#  ifdef __cplusplus\n"
                             "#    define NULL         0\n"
                             "#  else\n"
                             "#    define NULL         (void*) 0\n"
                             "#  endif\n"
                             "#endif\n\n");

    fprintf (global.outfile, "#define SAC_SET_TMPDIR              \"%s\"\n",
             global.config.tmpdir);

    fprintf (global.outfile, "#define SAC_SET_INITIAL_MASTER_HEAPSIZE      %d\n",
             global.initial_master_heapsize * 1024);
    fprintf (global.outfile, "#define SAC_SET_INITIAL_WORKER_HEAPSIZE      %d\n",
             global.initial_worker_heapsize * 1024);
    fprintf (global.outfile, "#define SAC_SET_INITIAL_UNIFIED_HEAPSIZE     %d\n\n",
             global.initial_unified_heapsize * 1024);

    fprintf (global.outfile, "#ifndef SAC_SET_RTSPEC_THREADS\n");
    fprintf (global.outfile, "#define SAC_SET_RTSPEC_THREADS              %d\n",
             global.num_rtspec_threads);
    fprintf (global.outfile, "#endif\n\n");

    fprintf (global.outfile, "#ifndef SAC_SET_MTMODE\n");
    fprintf (global.outfile, "#define SAC_SET_MTMODE               %d\n",
             (int)global.mtmode);
    fprintf (global.outfile, "#endif\n\n");

    switch (global.cpubindstrategy) {
    case HWLOC_off:
        fprintf (global.outfile, "#define SAC_SET_CPU_BIND_STRATEGY 0\n");
        break;
    case HWLOC_simple:
        fprintf (global.outfile, "#define SAC_SET_CPU_BIND_STRATEGY 1\n");
        break;
    case HWLOC_env:
        fprintf (global.outfile, "#define SAC_SET_CPU_BIND_STRATEGY 2\n");
        break;
    case HWLOC_numa:
        fprintf (global.outfile, "#define SAC_SET_CPU_BIND_STRATEGY 3\n");
        break;
    case HWLOC_socket:
        fprintf (global.outfile, "#define SAC_SET_CPU_BIND_STRATEGY 4\n");
        break;
    case HWLOC_envString:
        fprintf (global.outfile, "#define SAC_SET_CPU_BIND_STRATEGY 5\n");
        break;

    default:
        CTIerror ("internal error: missing strategy in gen_startup_code");
    }

    fprintf (global.outfile, "#define SAC_SET_BARRIER_TYPE               %d\n",
             global.mt_barrier_type);

    fprintf (global.outfile, "#define SAC_SET_SMART_DECISIONS            %d\n",
             global.mt_smart_mode);

    fprintf (global.outfile, "#define SAC_SET_SMART_FILENAME           \"%s\"\n",
             global.mt_smart_filename);

    fprintf (global.outfile, "#define SAC_SET_SMART_ARCH               \"%s\"\n",
             global.mt_smart_arch);

    fprintf (global.outfile, "#define SAC_SET_SMART_PERIOD               %d\n",
             global.mt_smart_period);

    fprintf (global.outfile, "#ifndef SAC_SET_THREADS_MAX\n");
    fprintf (global.outfile, "#define SAC_SET_THREADS_MAX          %d\n",
             global.max_threads);
    fprintf (global.outfile, "#endif\n\n");

    fprintf (global.outfile, "#ifndef SAC_SET_THREADS\n");
    fprintf (global.outfile, "#define SAC_SET_THREADS              %d\n",
             global.num_threads);
    fprintf (global.outfile, "#endif\n\n");

    fprintf (global.outfile, "#ifndef SAC_OMP_ACTIVE_LEVEL\n");
    fprintf (global.outfile, "#define SAC_OMP_ACTIVE_LEVEL          %d\n",
             global.ompnestlevel);
    fprintf (global.outfile, "#endif\n\n");

    fprintf (global.outfile, "#ifndef SAC_SET_MASTERCLASS\n");
    fprintf (global.outfile, "#define SAC_SET_MASTERCLASS          %d\n",
             CalcMasterclass (global.num_threads));
    fprintf (global.outfile, "#endif\n\n");

    fprintf (global.outfile, "#define SAC_SET_NUM_SCHEDULERS       %d\n\n",
             global.max_schedulers);

    fprintf (global.outfile, "#define SAC_SET_CACHE_1_SIZE         %d\n",
             global.config.cache1_size == 0 ? -1 : global.config.cache1_size);
    fprintf (global.outfile, "#define SAC_SET_CACHE_1_LINE         %d\n",
             global.config.cache1_line == 0 ? 4 : global.config.cache1_line);
    fprintf (global.outfile, "#define SAC_SET_CACHE_1_ASSOC        %d\n",
             global.config.cache1_assoc == 0 ? 1 : global.config.cache1_assoc);
    fprintf (global.outfile, "#define SAC_SET_CACHE_1_WRITEPOL     SAC_CS_%s\n",
             global.config.cache1_writepol);
    fprintf (global.outfile, "#define SAC_SET_CACHE_1_MSCA_FACTOR  %.2f\n\n",
             ((double)global.config.cache1_msca_factor) / 100.0);

    fprintf (global.outfile, "#define SAC_SET_CACHE_2_SIZE         %d\n",
             global.config.cache2_size == 0 ? -1 : global.config.cache2_size);
    fprintf (global.outfile, "#define SAC_SET_CACHE_2_LINE         %d\n",
             global.config.cache2_line == 0 ? 4 : global.config.cache2_line);
    fprintf (global.outfile, "#define SAC_SET_CACHE_2_ASSOC        %d\n",
             global.config.cache2_assoc == 0 ? 1 : global.config.cache2_assoc);
    fprintf (global.outfile, "#define SAC_SET_CACHE_2_WRITEPOL     SAC_CS_%s\n",
             global.config.cache2_writepol);
    fprintf (global.outfile, "#define SAC_SET_CACHE_2_MSCA_FACTOR  %.2f\n\n",
             ((double)global.config.cache2_msca_factor) / 100.0);

    fprintf (global.outfile, "#define SAC_SET_CACHE_3_SIZE         %d\n",
             global.config.cache3_size == 0 ? -1 : global.config.cache3_size);
    fprintf (global.outfile, "#define SAC_SET_CACHE_3_LINE         %d\n",
             global.config.cache3_line == 0 ? 4 : global.config.cache3_line);
    fprintf (global.outfile, "#define SAC_SET_CACHE_3_ASSOC        %d\n",
             global.config.cache3_assoc == 0 ? 1 : global.config.cache3_assoc);
    fprintf (global.outfile, "#define SAC_SET_CACHE_3_WRITEPOL     SAC_CS_%s\n",
             global.config.cache3_writepol);
    fprintf (global.outfile, "#define SAC_SET_CACHE_3_MSCA_FACTOR  %.2f\n\n",
             ((double)global.config.cache3_msca_factor) / 100.0);

    fprintf (global.outfile, "#define SAC_SET_CACHESIM_HOST        \"%s\"\n",
             STRonNull ("", global.cachesim_host));

    if (global.cachesim_file[0] == '\0') {
        fprintf (global.outfile, "#define SAC_SET_CACHESIM_FILE        \"%s.cs\"\n",
                 global.outfilename);
    } else {
        fprintf (global.outfile, "#define SAC_SET_CACHESIM_FILE        \"%s\"\n",
                 global.cachesim_file);
    }

    if (global.cachesim_dir[0] == '\0') {
        fprintf (global.outfile, "#define SAC_SET_CACHESIM_DIR         \"%s\"\n",
                 global.config.tmpdir);
    } else {
        fprintf (global.outfile, "#define SAC_SET_CACHESIM_DIR         \"%s\"\n",
                 global.cachesim_dir);
    }

    fprintf (global.outfile, "#define SAC_SET_MAXFUN               %zu\n",
             (global.profile_funcntr));
    fprintf (global.outfile, "#define SAC_SET_MAXFUNAP             %d\n",
             global.profile_funapmax);

    fprintf (global.outfile, "#define SBLOCKSZ               %d\n", 16);

    fprintf (global.outfile, "#define LBLOCKSZ               %d\n", 256);

    fprintf (global.outfile, "\n");

    if (global.doprofile) {
        PrintProfileData ();
    }

    /* Distributed memory backend specific settings */
    if (global.backend == BE_distmem) {
        fprintf (global.outfile,
                 "#define SAC_SET_DISTMEM_MAX_MEMORY_MB              %d\n",
                 global.distmem_max_memory_mb);
        fprintf (global.outfile,
                 "#define SAC_SET_DISTMEM_MIN_ELEMS_PER_NODE         %d\n",
                 global.distmem_min_elems_per_node);
        fprintf (global.outfile,
                 "#define SAC_SET_DISTMEM_TRACE_PROFILE_NODE         %d\n",
                 global.distmem_tr_pf_node);
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *
 *
 * description:
 *
 *
 ******************************************************************************/

static void
PrintIncludes (void)
{
    DBUG_ENTER ();

    fprintf (global.outfile, "\n\n"
                             "/*\n"
                             " *  Includes\n */\n\n");

    fprintf (global.outfile, "\n"
                             "#include \"sac.h\"\n\n");

    fprintf (global.outfile, "\n"
                             "#if SAC_OMP_MACROS\n");
    fprintf (global.outfile, "\n"
                             "#include \"omp.h\"\n\n");
    fprintf (global.outfile, "#endif\n");

    fprintf (global.outfile, "\n"
                             "#if SAC_CUDA_MACROS\n");
    fprintf (global.outfile, "\n"
                             "#include <stdio.h>\n\n");
    fprintf (global.outfile, "\n"
                             "#include <cuda.h>\n\n");
    fprintf (global.outfile, "\n"
                             "#include <cuda_runtime.h>\n\n");
    fprintf (global.outfile, "#endif\n");

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *
 *
 * description:
 *
 *
 ******************************************************************************/

static void
PrintDefines (void)
{
    DBUG_ENTER ();

    fprintf (global.outfile, "\n\n"
                             "/*\n"
                             " *  Global Definitions\n"
                             " */\n\n");

    fprintf (global.outfile, "SAC_PF_DEFINE()\n");
    fprintf (global.outfile, "SAC_HM_DEFINE()\n");

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void GSCprintFileHeader( node *syntax_tree)
 *
 * description:
 *
 *
 ******************************************************************************/

void
GSCprintFileHeader (node *syntax_tree)
{
    DBUG_ENTER ();

    PrintGlobalSwitches ();
    PrintGlobalSettings (syntax_tree);
    PrintIncludes ();

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void GSCprintDefines()
 *
 * description:
 *
 *
 * remark:
 *   This function should be called *after* the typedefs have been printed!
 *
 ******************************************************************************/

void
GSCprintDefines (void)
{
    DBUG_ENTER ();

    PrintDefines ();

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void GSCprintMainBegin()
 *
 * Description:
 *
 *
 ******************************************************************************/

void
GSCprintMainBegin (void)
{
    DBUG_ENTER ();

    if (global.backend == BE_distmem) {
        INDENT;
        fprintf (global.outfile, "SAC_DISTMEM_SETUP();\n");
    }

    INDENT;
    fprintf (global.outfile, "SAC_HWLOC_SETUP();\n");

    INDENT;
    fprintf (global.outfile, "SAC_MT_SETUP_INITIAL();\n");

    if (global.backend != BE_cuda) {
        INDENT;
        fprintf (global.outfile, "SAC_RTSPEC_SETUP_INITIAL(%i, \"%s\", \"%s\");\n",
                 global.rtspec_mode, global.command_line, global.argv[0]);
    }

    INDENT;
    fprintf (global.outfile, "SAC_PF_SETUP();\n");
    INDENT;
    fprintf (global.outfile, "SAC_HM_SETUP();\n");
    INDENT;
    fprintf (global.outfile, "SAC_MT_SETUP();\n");

    if (global.backend == BE_cuda) {
        INDENT;
        fprintf (global.outfile, "SAC_CUDA_SETUP();\n");
    }

    if (global.backend == BE_cudahybrid) {
        INDENT;
        fprintf (global.outfile, "SAC_DIST_SETUP();\n");
    }

    INDENT;
    fprintf (global.outfile, "SAC_CS_SETUP();\n");

    if (global.backend != BE_cuda) {
        INDENT;
        fprintf (global.outfile, "SAC_RTSPEC_SETUP();\n");
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void GSCprintMainEnd()
 *
 * Description:
 *
 *
 ******************************************************************************/

void
GSCprintMainEnd (void)
{
    DBUG_ENTER ();

    /*
     * global.outfile is already indented by 2
     */
    INDENT;
    /* We put the barrier here because the dsm memory is not
     * used anymore and otherwise the profiling may be incorrect. */
    fprintf (global.outfile, "SAC_DISTMEM_BARRIER();\n");
    INDENT;
    fprintf (global.outfile, "SAC_PF_PRINT();\n");
    INDENT;
    fprintf (global.outfile, "SAC_CS_FINALIZE();\n");
    INDENT;
    fprintf (global.outfile, "SAC_MT_FINALIZE();\n");

    if (global.backend == BE_cuda) {
        INDENT;
        fprintf (global.outfile, "SAC_CUDA_FINALIZE();\n");
    }

    INDENT;
    fprintf (global.outfile, "SAC_HWLOC_FINALIZE();\n");

    INDENT;
    fprintf (global.outfile, "SAC_HM_PRINT();\n\n");

    if (global.backend != BE_cuda) {
        INDENT;
        fprintf (global.outfile, "SAC_RTSPEC_FINALIZE();\n\n");
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void GSCprintMain()
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
GSCprintMainC99 (void)
{
    char *res_NT;
    types *tmp_type;
    bool print_thread_id, run_mt, run_mt_pthread, run_mt_lpel, run_mt_omp;

    DBUG_ENTER ();

    run_mt_pthread
      = (global.mtmode != MT_none) && STReq (global.config.mt_lib, "pthread");
    run_mt_lpel = (global.mtmode != MT_none) && STReq (global.config.mt_lib, "lpel");
    run_mt_omp = (global.backend == BE_omp);
    run_mt = run_mt_pthread || run_mt_omp || run_mt_lpel;

    print_thread_id = (run_mt_pthread || run_mt_lpel) && global.optimize.dophm;

    INDENT;
    fprintf (global.outfile, "int main( int __argc, char *__argv[])\n");
    INDENT;
    fprintf (global.outfile, "{\n");
    global.indent++;

    if (global.backend == BE_distmem) {
        /*
         * The distributed memory communication library needs to be initialized
         * before inspecting the arguments and before any output is produced.
         */
        INDENT;
        fprintf (global.outfile, "SAC_DISTMEM_INIT();\n");
    }

    if (print_thread_id) {
        INDENT;
        fprintf (global.outfile, "SAC_MT_DECL_MYTHREAD()\n");
    }
    tmp_type = TBmakeTypes1 (T_int);
    res_NT = NTUcreateNtTag ("SAC_res", tmp_type);
    tmp_type = FREEfreeAllTypes (tmp_type);
    ICMCompileND_DECL (res_NT, "int", 0, NULL); /* create ND_DECL icm */
    GSCprintMainBegin ();

    /*
     * set the number of OpenMP threads according to the parameter numthreads
     * <n> in the   command line
     * set the max active nested parallel according to the parameter ompnestlevel
     * <n> in the command line
     */
    if (global.backend == BE_omp) {
        INDENT;
        fprintf (global.outfile, "SAC_OMP_SET_NUM_THREADS();\n\n");
        INDENT;
        fprintf (global.outfile, "SAC_OMP_SET_MAX_ACTIVE_LEVEL();\n\n");
        INDENT;
    }

    INDENT;
    fprintf (global.outfile, "SAC_COMMANDLINE_SET( __argc, __argv);\n\n");

    INDENT;
    fprintf (global.outfile, "SAC_INVOKE_MAIN_FUN( SACf_%s_%s_main, ",
             NSgetName (NSgetRootNamespace ()), run_mt ? "CL_ST_" : "");

    fprintf (global.outfile, "SAC_ND_ARG_out( %s, int)", res_NT);
    fprintf (global.outfile, ");\n\n");

    GSCprintMainEnd ();

    if (global.backend == BE_distmem) {
        INDENT;
        fprintf (global.outfile, "SAC_DISTMEM_EXIT( SAC_ND_READ( %s, 0));\n", res_NT);
    }

    INDENT;
    fprintf (global.outfile, "return( SAC_ND_READ( %s, 0));\n", res_NT);
    res_NT = MEMfree (res_NT);
    global.indent--;
    INDENT;
    fprintf (global.outfile, "}\n");

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void GSCprintMainDistMem()
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
GSCprintMainDistMem (void)
{

    DBUG_ENTER ();

    GSCprintMainC99 ();

    DBUG_RETURN ();
}

static void
GSCprintMainMuTC (void)
{
#if 0
  char *res_NT;
  types *tmp_type;
#endif

    DBUG_ENTER ();
#if 0
  INDENT;
  fprintf( global.outfile, "thread main()\n");
  INDENT;
  fprintf( global.outfile, "{\n");
  global.indent++;
  tmp_type = TBmakeTypes1( T_int);
  res_NT = NTUcreateNtTag( "SAC_res", tmp_type);
  tmp_type = FREEfreeAllTypes( tmp_type);
  ICMCompileND_DECL( res_NT, "int", 0, NULL);   /* create ND_DECL icm */
  GSCprintMainBegin();

  INDENT;
  fprintf( global.outfile, "SAC_COMMANDLINE_SET( 0, ((char **) 0));\n\n");
  INDENT;
  fprintf( global.outfile, "SACf_%s__main( ",
      NSgetName( NSgetRootNamespace()));

  fprintf( global.outfile, "SAC_ND_ARG_out( %s)", res_NT);
  fprintf( global.outfile, ");\n\n");
  GSCprintMainEnd();
#if 0
  INDENT;
  fprintf( global.outfile, "return( SAC_ND_READ( %s, 0));\n", res_NT);
#endif
  res_NT = MEMfree( res_NT);
  global.indent--;
  INDENT;
  fprintf( global.outfile, "}\n");
#endif
    fprintf (global.outfile, "SAC_MUTC_MAIN\n");
    DBUG_RETURN ();
}

void
GSCprintMain (void)
{
    DBUG_ENTER ();

    switch (global.backend) {
    case BE_c99:
        GSCprintMainC99 ();
        break;
    case BE_mutc:
        GSCprintMainMuTC ();
        break;
    case BE_cuda:
    case BE_cudahybrid:
        GSCprintMainC99 ();
        break;
    case BE_omp:
        GSCprintMainC99 ();
        break;
    case BE_distmem:
        GSCprintMainDistMem ();
        break;
    default:
        DBUG_UNREACHABLE ("unknown backend");
    }

    DBUG_RETURN ();
}

/** <!-- ****************************************************************** -->
 * @brief Used to print stubs for SACARGfreeDataUdt and SACARGcopyDataUdt
 *        when compiling programs to circumvent linker errors.
 ******************************************************************************/
void
GSCprintSACargCopyFreeStubs (void)
{
    DBUG_ENTER ();

    if (global.backend != BE_cuda && global.backend != BE_cudahybrid) {
        fprintf (global.outfile, "/*\n"
                                 " * stubs for SACARGfreeDataUdt and SACARGcopyDataUdt\n"
                                 " */\n"
                                 "extern void SACARGfreeDataUdt( int, void *);\n"
                                 "extern void *SACARGcopyDataUdt( int, int, void *);\n"
                                 "void SACARGfreeDataUdt( int size, void *data) {}\n"
                                 "void *SACARGcopyDataUdt( int type, int size, void "
                                 "*data) { return ((void *) 0x0); } \n"
                                 "\n");
    } else {
        fprintf (global.outfile,
                 "/*\n"
                 " * stubs for SACARGfreeDataUdt and SACARGcopyDataUdt\n"
                 " */\n"
                 "extern \"C\" void SACARGfreeDataUdt( int, void *);\n"
                 "extern \"C\" void *SACARGcopyDataUdt( int, int, void *);\n"
                 "void SACARGfreeDataUdt( int size, void *data) {}\n"
                 "void *SACARGcopyDataUdt( int type, int size, void *data) { return "
                 "((void *) 0x0); } \n"
                 "\n");
    }

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
