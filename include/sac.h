#ifndef __SAC_H__
#define __SAC_H__

#include "runtime/essentials_h/rc_impl_gen.h"
#include "runtime/essentials_h/std_gen.h"

#if SAC_DO_MULTITHREAD
#    include "runtime/mt_h/mt_gen.h"
#    include "runtime/mt_h/fp_gen.h"
#endif

// FIXME for all practical purposes this doesn't work
// so maybe we want to introduce a flag to disable NESTED
#include "runtime/nested_h/nested_gen.h"
#include "runtime/nested_h/nested.h"

#ifdef SAC_BACKEND_CUDA
#    include "runtime/cuda_h/cuda_gen.h"
#    include "runtime/cuda_h/cuda.h"
#endif

#ifdef SAC_BACKEND_CudaHybrid
#    include "runtime/cudahybrid_h/alloc_cache.h"
#    include "runtime/cudahybrid_h/basic.h"
#    include "runtime/cudahybrid_h/cudahybrid.h"
#    include "runtime/cudahybrid_h/dist_var_memsave.h"
#    include "runtime/cudahybrid_h/dist_var_nomemsave.h"
#endif

#include "runtime/distmem_h/distmem.h"

#if SAC_DO_DISTMEM
#    include "runtime/distmemphm_h/distmemphm.h"
#endif

#include "runtime/essentials_h/audwl.h"
#include "runtime/essentials_h/bool.h"
#include "runtime/essentials_h/errors.h"
#include "runtime/essentials_h/icm.h"
#include "runtime/essentials_h/idx.h"
#include "runtime/essentials_h/prf.h"
#include "runtime/essentials_h/rc_impl.h"
#include "runtime/essentials_h/rc_methods.h"
#include "runtime/essentials_h/rc_stubs.h"
#include "runtime/essentials_h/rcm_local_async_norc_ptr.h"
#include "runtime/essentials_h/rcm_local_pasync.h"
#include "runtime/essentials_h/rcm_local_pasync_norc_desc.h"
#include "runtime/essentials_h/rt_commandline.h"
#include "runtime/essentials_h/rt_misc.h"
#include "runtime/essentials_h/std.h"
#include "runtime/essentials_h/tuple.h"
#include "runtime/essentials_h/types.h"
#include "runtime/essentials_h/wl.h"

#if SAC_DO_CACHESIM
#   include "runtime/extras_h/rt_cachesim.h"
#endif
#include "runtime/extras_h/rt_profile.h"
#if SAC_DO_TRACE
#    include "runtime/extras_h/rt_trace.h"
#endif
#if SAC_DO_CHECK
#    include "runtime/extras_h/runtimecheck.h"
#endif

#include "runtime/hwloc_h/cpubind.h"

#if SAC_DO_MULTITHREAD
#    include "runtime/mt_h/fp.h"
#    include "runtime/mt_h/mt_lpel.h"
#    include "runtime/mt_h/rt_mt.h"
#    include "runtime/mt_h/rt_mt_barriers.h"
#    include "runtime/mt_h/rt_mt_beehive.h"
#    include "runtime/mt_h/rt_mt_omp.h"
#    include "runtime/mt_h/rt_mt_pth.h"
#    include "runtime/mt_h/rt_mt_smart.h"
#    include "runtime/mt_h/schedule.h"
#endif

#ifdef SAC_BACKEND_MUTC
#    include "runtime/mutc_h/mutc_gen.h"
#    include "runtime/mutc_h/mutc_rc_gen.h"
#    include "runtime/mutc_h/fun.h"
#    include "runtime/mutc_h/mutc.h"
#    include "runtime/mutc_h/mutc_bench.h"
#    include "runtime/mutc_h/mutc_rc.h"
#    include "runtime/mutc_h/mutc_startup.h"
#    include "runtime/mutc_h/mutc_tostring.h"
#    include "runtime/mutc_h/mutc_world.h"
#    include "runtime/mutc_h/tls_malloc.h"
#endif

#if SAC_DO_PHM
#   include "runtime/phm_h/phm.h"
#endif

#include "runtime/rtspec_h/rtspec.h"

#if SAC_DO_CACHESIM
#    include "libsac/cachesim/basic.h"
#    include "libsac/cachesim/cachesim.h"
#endif

#include "libsac/essentials/commandline.h"
#include "libsac/essentials/message.h"
#include "libsac/essentials/misc.h"
#include "libsac/essentials/trace.h"
#include "libsac/profile/profile.h"
#include "libsac/profile/profile_memory.h"
#include "libsac/profile/profile_ops.h"
#include "libsac/profile/profile_print.h"
#include "libsac/hwloc/cpubind.h"
#include "libsac/hwloc/cudabind.h"

// This is not needed, as this is internal
//#include "libsac/interface/sacarg.h"

#include "libsac/mt/mt_beehive.h"

#if SAC_DO_MULTITHREAD
#    include "libsac/mt/hwloc_data.h"
#    include "libsac/mt/mt.h"
#    include "libsac/mt/mt_barriers.h"
#    include "libsac/mt/mt_omp.h"
#    include "libsac/mt/mt_pth.h"
#    include "libsac/mt/mt_smart.h"
#endif

// This file is needed when compiling with -t mt_pth
// which relies on a certain global variable defined here.
#include "libsac/rtspec/empty.h"
#if SAC_DO_RTSPEC
#    include "libsac/rtspec/meldqueue.h"
#    include "libsac/rtspec/persistence.h"
#    include "libsac/rtspec/registry.h"
#    include "libsac/rtspec/simple_controller.h"
#    include "libsac/rtspec/simple_reqqueue.h"
#    include "libsac/rtspec/supervisor.h"
#    include "libsac/rtspec/trace.h"
#    include "libsac/rtspec/uuid_controller.h"
#    include "libsac/rtspec/uuid_reqqueue.h"
#endif

#endif /* __SAC_H__  */
