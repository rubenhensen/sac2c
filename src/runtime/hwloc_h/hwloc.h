/*****************************************************************************
 *
 * file:   hwloc.h
 *
 * description:
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides the nessicary definitions of functions to bind CPUs by
 *   using the HWLOC library.
 *
 * notes:
 *
 *   Initialization expects a number of things:
 *    - The system is homogeneous (e.g. same number of cores on each socket
 *      and same number of PUs on each core). If it is not, the check to see
 *      if enough sockets/cores/PUs are available is useless and workload
 *      will still be distributed homogeneously.
 *    - Cores don't have caches below them, or if they do each cache has a
 *      different PU for it (NB. L1 is generally considered by hwloc to be
 *      the parent of a core, run hwloc's `lstopo' command to get the topology
 *      hierarchy when in doubt).
 *
 * variables:
 *  - `SAC_HWLOC_cpu_sets' is an array of length sockets*cores*PUs created in
 *    `SAC_HWLOC_init'. It contains the data necessary to bind to a specific PU.
 *  - `SAC_HWLOC_topology' contains information about the system's topology and
 *    is also required for binding to PUs.
 *
 *  Both can be removed with `SAC_HWLOC_cleanup' after threads have been bound.
 *
 *  Either `SAC_HWLOC_dont_bind' or `SAC_HWLOC_init' must be called when
 *  ENABLE_HWLOC is true or binding threads will result in undefined behaviour.
 *
 *****************************************************************************/
#ifndef _SAC_HWLOC_H
#define _SAC_HWLOC_H

#ifdef __cplusplus
extern "C" {
#endif

#if ENABLE_HWLOC

#include <hwloc.h>

typedef struct hwloc_topo_data {
    int num_sockets_available;
    int num_numa_nodes_avail;
    int num_cores_available;
    int num_pus_available;
} hwloc_topo_data_t;

hwloc_topo_data_t *SAC_HWLOC_topo_data;
extern hwloc_cpuset_t *SAC_HWLOC_cpu_sets;
extern hwloc_topology_t SAC_HWLOC_topology;

extern int SAC_HWLOC_info_snprintf (char *, size_t, hwloc_topology_t, hwloc_cpuset_t,
                                    hwloc_cpuset_t);
extern void SAC_HWLOC_bind_on_cpuset (hwloc_cpuset_t);
extern hwloc_cpuset_t *SAC_HWLOC_get_core (hwloc_cpuset_t);
extern void SAC_HWLOC_init (void);
extern void SAC_HWLOC_cleanup (void);

/*****************************************************************************
 *  These ICMs are used as part of the startup code for the MAIN method
 ****************************************************************************/

#define SAC_HWLOC_SETUP() SAC_HWLOC_init ();

#define SAC_HWLOC_FINALIZE() SAC_HWLOC_cleanup ();

#else /* ENABLE_HWLOC */

#define SAC_HWLOC_SETUP()
#define SAC_HWLOC_FINALIZE()

#endif /* ENABLE_HWLOC */

#ifdef __cplusplus
}
#endif

#endif /* _SAC_HWLOC_H */
