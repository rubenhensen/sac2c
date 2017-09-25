#include "sac.h"

#if ENABLE_CUDA && ENABLE_HWLOC

#include <hwloc/cudart.h>

// FIXME(hans) we currently use the same cpuset point as in the MT case, meaning that
//            the cudahybrid backend could go boom!
void
SAC_CUDA_HWLOC_init (int cuda_ordinal)
{
    hwloc_cpuset_t tmp_hw_cpuset;
    char str[128];

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

    SAC_HWLOC_info_snprintf (str, sizeof (str), SAC_HWLOC_topology, tmp_hw_cpuset,
                             *SAC_HWLOC_cpu_sets);
    SAC_TR_PRINT (("-> bound to %s", str));

    hwloc_bitmap_free (tmp_hw_cpuset);
}

#else
void
SAC_CUDA_HWLOC_init (int cuda_ordinal)
{
    SAC_NOOP ();
}
static int nothing_is_here = 0xdead;
#endif /* ENABLE_HWLOC && ENABLE_CUDA */
