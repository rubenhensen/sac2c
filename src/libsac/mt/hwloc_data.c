#include "sac.h"

#if ENABLE_HWLOC

hwloc_cpuset_t *SAC_HWLOC_cpu_sets;
hwloc_topology_t SAC_HWLOC_topology;

#if SAC_MT_MODE > 0

/*
Traverses the topology tree to create a flat array of PUs (cpu_sets).
First called with an object of type socket.
@return the number of cores still to be put in cpu_sets to match the requested ammount.
*/
static unsigned int
traverse_topology_tree (hwloc_obj_t object, unsigned int *cpu_set_counter,
                        unsigned int cores_remaining, unsigned int PUs_required)
{
    unsigned int i;
    switch (object->type) {
    case HWLOC_OBJ_SOCKET:
    case HWLOC_OBJ_CACHE:
        for (i = 0; i < object->arity; ++i) {
            cores_remaining
              = traverse_topology_tree (object->children[i], cpu_set_counter,
                                        cores_remaining, PUs_required);
            if (cores_remaining == 0)
                return 0;
        }
        break;
    case HWLOC_OBJ_CORE:
        for (i = 0; i < PUs_required; ++i) {
            ++(*cpu_set_counter);
            SAC_HWLOC_cpu_sets[*cpu_set_counter]
              = hwloc_bitmap_dup (object->children[i]->cpuset);
        }
        return cores_remaining - 1;
    default:
        SAC_RuntimeError ("Unexpected system topology");
    }
    return cores_remaining;
}

void
SAC_HWLOC_init (int threads, int sockets, int cores, int PUs)
{
    hwloc_topology_init (&SAC_HWLOC_topology);
    hwloc_topology_load (SAC_HWLOC_topology);

    int num_sockets_available;
    num_sockets_available
      = hwloc_get_nbobjs_by_type (SAC_HWLOC_topology, HWLOC_OBJ_SOCKET);

    int num_cores_available;
    num_cores_available = hwloc_get_nbobjs_by_type (SAC_HWLOC_topology, HWLOC_OBJ_CORE);

    int num_pus_available;
    num_pus_available = hwloc_get_nbobjs_by_type (SAC_HWLOC_topology, HWLOC_OBJ_PU);

    if (sockets == 0) {
        SAC_TR_LIBSAC_PRINT (
          ("No number of sockets specified; presetting to number of sockets"
           " available: %d",
           num_sockets_available));
        sockets = num_sockets_available;
    } else if (num_sockets_available < sockets) {
        SAC_RuntimeError ("Tried to use more sockets than available on the system");
    };

    if (cores == 0) {
        SAC_TR_LIBSAC_PRINT (
          ("No number of cores per socket specified; presetting to number of cores"
           " per socket available: %d",
           num_cores_available / num_sockets_available));
        cores = num_cores_available / num_sockets_available;
    } else if (num_cores_available / num_sockets_available < cores) {
        SAC_RuntimeError (
          "Tried to use more cores per socket than available on the system");
    };

    if (PUs == 0) {
        SAC_TR_LIBSAC_PRINT (
          ("No number of processing units per socket specified; presetting to number of"
           " processing units per core available: %d",
           num_pus_available / num_cores_available));
        PUs = num_pus_available / num_cores_available;
    } else if (num_pus_available / num_cores_available < PUs) {
        SAC_RuntimeError (
          "Tried to use more processing units per core than available on the system");
    };

    if (threads > sockets * cores * PUs) {
        SAC_RuntimeError (
          "sockets*cores*PUs (%d) is less than the number of threads desired %d",
          sockets * cores * PUs, threads);
    }
    SAC_TR_LIBSAC_PRINT (
      ("Pinning on %u sockets, %u cores and %u processing units. Maximum number"
       " of processing units potentially used: %u",
       sockets, cores, PUs, sockets * cores * PUs));

    SAC_HWLOC_cpu_sets
      = (hwloc_cpuset_t *)SAC_MALLOC (sockets * cores * PUs * sizeof (hwloc_cpuset_t));
    unsigned int counter, i;
    counter = 0;
    for (i = 0; i < sockets; ++i) {
        SAC_TR_LIBSAC_PRINT (("traversing socket %d", i));
        hwloc_obj_t socket;
        socket = hwloc_get_obj_by_type (SAC_HWLOC_topology, HWLOC_OBJ_SOCKET, i);
        traverse_topology_tree (socket, &counter, cores, PUs);
        SAC_TR_LIBSAC_PRINT (("traversing socket %d done", i));
    }
    SAC_TR_LIBSAC_PRINT (("Pinning done"));
}

void
SAC_HWLOC_cleanup ()
{
    if (SAC_HWLOC_topology) {
        SAC_FREE (SAC_HWLOC_cpu_sets);
        hwloc_topology_destroy (SAC_HWLOC_topology);
    }
}

void
SAC_HWLOC_dont_bind ()
{
    SAC_TR_LIBSAC_PRINT (("binding disabled"));
    SAC_HWLOC_topology = 0;
}

#endif /* SAC_MT_MODE > 0 */

#else
static int this_translation_unit = 0xdead;
#endif // ENABLE_HWLOC
