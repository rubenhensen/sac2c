/**
 * @file cudabind.c
 * @brief Runtime functions for performing HWLOC operations when using CUDA
 */
 #include "config.h"

#include <stddef.h>

#include "libsac/essentials/message.h"
#include "cudabind.h"

#if ENABLE_CUDA && ENABLE_HWLOC

#include <hwloc.h>
#include <hwloc/cudart.h>

#include "libsac/hwloc/cpubind.h"

/**
 * Function initialises the HWLOC sub-system for use with CUDA-backend.
 *
 * @FIXME we currently use the same cpuset point as in the MT case, meaning that
 *        the cudahybrid backend could go boom!
 *
 * @param cuda_ordinal The CUDA device ordinal number (>= 0)
 * @param str On successful initialisation of HWLOC sub-system, this variable
 *            contains a human-readable description of the HWLOC topology
 * @param  str_size Size of the allocated string str
 */
void
SAC_CUDA_HWLOC_init (int cuda_ordinal, char *str, size_t str_size)
{
    hwloc_cpuset_t tmp_hw_cpuset;

    // FIXME might be good to make this a separate function?
    // cprintf(stderr, "-> Printing overall tree\n");
    // hwloc_print_topology_tree(hwloc_get_root_obj(SAC_HWLOC_topology), 0);
    // hwloc_print_topology_tree(hwloc_get_first_largest_obj_inside_cpuset(SAC_HWLOC_topology,
    // tmp_hw_cpuset), 0);

    tmp_hw_cpuset = hwloc_bitmap_alloc (); // empty map on cpuset
    if (hwloc_cudart_get_device_cpuset (SAC_HWLOC_topology, cuda_ordinal, tmp_hw_cpuset))
        SAC_RuntimeError ("Unable to locate HWLOC cpuset nearest to CUDA device %d!",
                          cuda_ordinal);

    // get nearest CPU core
    SAC_HWLOC_cpu_sets = SAC_HWLOC_get_core (tmp_hw_cpuset);

    // bind current process to cpuset
    SAC_HWLOC_bind_on_cpuset (*SAC_HWLOC_cpu_sets);

    // bind all further memory allocations to the current NODE/NUMANODE
    if (hwloc_set_membind (SAC_HWLOC_topology, *SAC_HWLOC_cpu_sets, HWLOC_MEMBIND_BIND,
                           HWLOC_MEMBIND_STRICT))
        SAC_RuntimeError (
          "HWLOC is unable to bind all memory allocations to current NODE!");

    SAC_HWLOC_info_snprintf (str, str_size, SAC_HWLOC_topology, tmp_hw_cpuset,
                             *SAC_HWLOC_cpu_sets);

    hwloc_bitmap_free (tmp_hw_cpuset);
}

#else /* ENABLE_HWLOC && ENABLE_CUDA */

/**
 * This is a dummy function that does no initialisation of the HWLOC sub-system.
 * Calling this function will cause a Runtime Error.
 *
 * @param cuda_ordinal The CUDA device ordinal number (>= 0)
 * @param str On successful initialisation of HWLOC sub-system, this variable
 *            contains a human-readable description of the HWLOC topology
 * @param  str_size Size of the allocated string str
 */
void
SAC_CUDA_HWLOC_init (int cuda_ordinal, char *str, size_t str_size)
{
    SAC_RuntimeError ("CUDA HWLOC binding is disabled! This function should not be called!");
}
#endif /* ENABLE_HWLOC && ENABLE_CUDA */
