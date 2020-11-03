#ifndef _SAC_HWLOC_LIB_H_
#define _SAC_HWLOC_LIB_H_

#include <stddef.h>
#include <hwloc.h>

typedef struct hwloc_topo_data {
    int num_sockets_available;
    int num_numa_nodes_avail;
    int num_cores_available;
    int num_pus_available;
} hwloc_topo_data_t;

extern hwloc_topo_data_t *SAC_HWLOC_topo_data;
extern hwloc_cpuset_t *SAC_HWLOC_cpu_sets;
extern hwloc_topology_t SAC_HWLOC_topology;

extern int SAC_HWLOC_info_snprintf (char *, size_t, hwloc_topology_t, hwloc_cpuset_t,
                                    hwloc_cpuset_t);
extern void SAC_HWLOC_bind_on_cpuset (hwloc_cpuset_t);
extern hwloc_cpuset_t *SAC_HWLOC_get_core (hwloc_cpuset_t);
extern void SAC_HWLOC_init (void);
extern void SAC_HWLOC_cleanup (void);

#endif /* _SAC_HWLOC_LIB_H_ */
