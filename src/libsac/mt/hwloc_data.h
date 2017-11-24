/**
 * Initialization expects a number of things:
 * - The system is homogeneous (e.g. same number of cores on each socket and same number of
 * PUs on each core). If it is not, the check to see if enough sockets/cores/PUs are
 * available is useless and workload will still be distributed homogeneously.
 * - Cores don't have caches below them, or if they do each cache has a different PU for it
 * (NB. L1 is generally considered by hwloc to be the parent of a core, run hwloc's lstopo
 * command to get the toplogy hierarchy when in doubt).
 * 
 * 
 * SAC_HWLOC_cpu_sets is an array of length sockets*cores*PUs created in SAC_HWLOC_init. It
 * contains the data necessary to bind to a specific PU. SAC_HWLOC_topology contains
 * information about the system's topology and is also required for binding to PUs. Both can
 * be removed with SAC_HWLOC_cleanup after threads have been bound.
 * 
 * Either SAC_HWLOC_dont_bind or SAC_HWLOC_init must be called when ENABLE_HWLOC is true or
 * binding threads will result in undefined behaviour
 */

#ifndef _SAC_HWLOC_H_
#define _SAC_HWLOC_H_

#if ENABLE_HWLOC

#include <hwloc.h>

extern hwloc_cpuset_t *SAC_HWLOC_cpu_sets;
extern hwloc_topology_t SAC_HWLOC_topology;

void SAC_HWLOC_init (int threads);
void SAC_HWLOC_cleanup (void);

#endif // ENABLE_HWLOC

#endif // _SAC_HWLOC_H_
