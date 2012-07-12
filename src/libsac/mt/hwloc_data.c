#if ENABLE_HWLOC
#include "../runtime/mt_h/hwloc_data.h"
#include <sac.h>
hwloc_cpuset_t *cpu_sets;
hwloc_topology_t topology;

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
SAC_HWLOC_init (unsigned char sockets, unsigned char cores, unsigned char PUs)
{
    hwloc_topology_init (&SAC_HWLOC_topology);
    hwloc_topology_load (SAC_HWLOC_topology);

    int num_sockets_available;
    num_sockets_available
      = hwloc_get_nbobjs_by_type (SAC_HWLOC_topology, HWLOC_OBJ_SOCKET);

    if (num_sockets_available < sockets)
        SAC_RuntimeError ("Tried to use more sockets than available on the system");
    if (hwloc_get_nbobjs_by_type (SAC_HWLOC_topology, HWLOC_OBJ_CORE) < cores * sockets)
        SAC_RuntimeError ("Tried to use more cores than available on the system");
    if (hwloc_get_nbobjs_by_type (SAC_HWLOC_topology, HWLOC_OBJ_PU)
        < cores * sockets * PUs)
        SAC_RuntimeError ("Tried to use more PUs than available on the system");

    SAC_HWLOC_cpu_sets
      = (hwloc_cpuset_t *)SAC_MALLOC (sockets * cores * PUs * sizeof (hwloc_cpuset_t));
    unsigned int counter, i;
    counter = 0;
    for (i = 0; i < sockets; ++i) {
        hwloc_obj_t socket;
        socket = hwloc_get_obj_by_type (SAC_HWLOC_topology, HWLOC_OBJ_SOCKET, i);
        traverse_topology_tree (socket, &counter, cores, PUs);
    }
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
    SAC_TR_PRINT (
      "sockets, cores or pus env variables not set or set to 0, not binding threads");
    topology = 0;
}

#endif // ENABLE_HWLOC
