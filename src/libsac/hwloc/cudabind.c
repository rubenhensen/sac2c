/**
 * @file
 * @brief Runtime functions for performing HWLOC operations when using CUDA
 *
 * This module builds on top of the existing HWLOC stuff in cpubind.c. When a user
 * specifies `-mt_bind` when using the CUDA backend, this activates the HWLOC binding
 * to setup the topology and perform an initial binding based upon the parameter passed
 * through the flag. In the CUDA related functions below, we override this by making use
 * of HWLOC functionality to find the nearest CPU/PU associated with the PCI bus connected
 * to the CUDA device, and we bind to this instead.
 *
 * We do not specify a _finalize_ functions as this is already handled though cpubind.c. See
 * gen_startup_code.c for the macros that are added to the generated code that affect this.
 */
#include "config.h"

#include <stddef.h>
#include <stdbool.h>

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
bool
SAC_CUDA_HWLOC_init (int cuda_ordinal, char *str, size_t str_size)
{
    bool ret = true;
    hwloc_cpuset_t tmp_hw_cpuset;

    // TODO might be good to make this a separate function?
    // cprintf(stderr, "-> Printing overall tree\n");
    // hwloc_print_topology_tree(hwloc_get_root_obj(SAC_HWLOC_topology), 0);
    // hwloc_print_topology_tree(hwloc_get_first_largest_obj_inside_cpuset(SAC_HWLOC_topology,
    // tmp_hw_cpuset), 0);

    tmp_hw_cpuset = hwloc_bitmap_alloc (); // empty map on cpuset
    if (hwloc_cudart_get_device_cpuset (SAC_HWLOC_topology, cuda_ordinal, tmp_hw_cpuset)) {
        ret = false;
        SAC_RuntimeWarning ("Unable to locate HWLOC cpuset nearest to CUDA device %d!",
                            cuda_ordinal);
        goto error;
    }

    // get nearest CPU core
    SAC_HWLOC_cpu_sets = SAC_HWLOC_get_core (tmp_hw_cpuset);

    // bind current process to cpuset
    SAC_HWLOC_bind_on_cpuset (*SAC_HWLOC_cpu_sets);

    // bind all further memory allocations to the current NODE/NUMANODE
    if (hwloc_set_membind (SAC_HWLOC_topology, *SAC_HWLOC_cpu_sets, HWLOC_MEMBIND_BIND,
                           HWLOC_MEMBIND_STRICT)) {
        ret = false;
        SAC_RuntimeWarning (
          "HWLOC is unable to bind all memory allocations to current NODE!");
        goto error;
    }

    SAC_HWLOC_info_snprintf (str, str_size, SAC_HWLOC_topology, tmp_hw_cpuset,
                             *SAC_HWLOC_cpu_sets);

error:
    hwloc_bitmap_free (tmp_hw_cpuset);
    return ret;
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
