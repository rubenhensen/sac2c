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

#include "libsac/hwloc/cpubind.h"

#if SAC_SET_CPU_BIND_STRATEGY > 0

/*****************************************************************************
 *  These ICMs are used as part of the startup code for the MAIN method
 ****************************************************************************/

#define SAC_HWLOC_SETUP() SAC_HWLOC_init ();

#define SAC_HWLOC_FINALIZE() SAC_HWLOC_cleanup ();

#else /* SAC_SET_CPU_BIND_STRATEGY */

#define SAC_HWLOC_SETUP()
#define SAC_HWLOC_FINALIZE()

#endif /* SAC_SET_CPU_BIND_STRATEGY */

#ifdef __cplusplus
}
#endif

#endif /* _SAC_HWLOC_H */
