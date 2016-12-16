#include "sac.h"

#if ENABLE_HWLOC

hwloc_cpuset_t *SAC_HWLOC_cpu_sets;
hwloc_topology_t SAC_HWLOC_topology;

#define SAC_HWLOC_NUM_SOCKETS_VAR_NAME "SAC_NUM_SOCKETS"
#define SAC_HWLOC_NUM_CORES_VAR_NAME "SAC_NUM_CORES"
#define SAC_HWLOC_NUM_PUS_VAR_NAME "SAC_NUM_PUS"

#define SAC_PULIST_EMPTY_CHAR '-'
#define SAC_PULIST_FULL_CHAR '*'

#if SAC_MT_MODE > 0

static char *
strategySimple (int threads, int sockets_avail, int cores_avail, int pus_avail)
{
    int i;
    char *res;

    if (threads > sockets_avail * cores_avail * pus_avail) {
        SAC_RuntimeError ("Asking for %d threads on a machine with %d processing units; "
                          "Either decrease the number of threads or turn -mt_bind off",
                          threads, sockets_avail * cores_avail * pus_avail);
    }
    res = (char *)SAC_MALLOC (sizeof (bool)
                              * (sockets_avail * cores_avail * pus_avail + 1));
    for (i = 0; i < sockets_avail * cores_avail * pus_avail; i++) {
        res[i] = SAC_PULIST_EMPTY_CHAR;
    }
    res[i] = '\0';
    for (i = 0; i < threads; i++) {
        res[i] = SAC_PULIST_FULL_CHAR;
    }
    return res;
}

static char *
strategyEnv (int threads, int sockets_avail, int cores_avail, int pus_avail)
{
    int i, j, k, idx;
    char *res;
    char *char_dump;
    int sockets;
    int cores;
    int pus;

    char_dump = getenv (SAC_HWLOC_NUM_SOCKETS_VAR_NAME);
    sockets = (char_dump == NULL ? 0 : atoi (char_dump));

    char_dump = getenv (SAC_HWLOC_NUM_CORES_VAR_NAME);
    cores = (char_dump == NULL ? 0 : atoi (char_dump));

    char_dump = getenv (SAC_HWLOC_NUM_PUS_VAR_NAME);
    pus = (char_dump == NULL ? 0 : atoi (char_dump));

    if (sockets == 0) {
        SAC_TR_LIBSAC_PRINT (
          ("No number of sockets specified; presetting to number of sockets"
           " available: %d",
           sockets_avail));
        sockets = sockets_avail;
    } else if (sockets_avail < sockets) {
        SAC_RuntimeError ("Tried to use %d sockets on a system with %d sockets", sockets);
    };

    if (cores == 0) {
        SAC_TR_LIBSAC_PRINT (
          ("No number of cores per socket specified; presetting to number of cores"
           " per socket available: %d",
           cores_avail));
        cores = cores_avail;
    } else if (cores_avail < cores) {
        SAC_RuntimeError (
          "Tried to use %d cores per socket on a system with %d cores per socket", cores,
          cores_avail);
    };

    if (pus == 0) {
        SAC_TR_LIBSAC_PRINT (
          ("No number of processing units per socket specified; presetting to number of"
           " processing units per core available: %d",
           pus_avail));
        pus = pus_avail;
    } else if (pus_avail < pus) {
        SAC_RuntimeError (
          "Tried to use %d processing units per core on the system with %d "
          "processing units per core",
          pus, pus_avail);
    };

    if (threads > sockets * cores * pus) {
        SAC_RuntimeError (
          "Specified %d threads which exceeds socket/core/PU-constraintss (%d); "
          "reduce the thread # or set fewer constraints/increase their values.",
          threads, sockets * cores * pus);
    }

    res = (char *)SAC_MALLOC (sizeof (bool)
                              * (sockets_avail * cores_avail * pus_avail + 1));
    for (i = 0; i < sockets_avail * cores_avail * pus_avail; i++) {
        res[i] = SAC_PULIST_EMPTY_CHAR;
    }
    res[i] = '\0';

    // Now, for the actual selection of PUs:
    idx = 0;
    for (i = 0; i < sockets; i++) {
        for (j = 0; j < cores; j++) {
            for (k = 0; k < pus; k++) {
                if (idx < threads) {
                    res[idx] = SAC_PULIST_FULL_CHAR;
                    idx++;
                }
            }
            idx += pus_avail - pus;
        }
        idx += cores_avail - cores;
    }

    return (res);
}

static hwloc_cpuset_t *
pusString2cpuSets (char *pus_string, int num_pus)
{
    int i, idx;
    hwloc_cpuset_t *res;
    hwloc_obj_t pu;

    res = (hwloc_cpuset_t *)SAC_MALLOC (SAC_MT_global_threads * sizeof (hwloc_cpuset_t));

    idx = 0;
    for (i = 0; i < num_pus; i++) {
        if (pus_string[i] == SAC_PULIST_FULL_CHAR) {
            pu = hwloc_get_obj_by_type (SAC_HWLOC_topology, HWLOC_OBJ_PU, i);
            if (pu == NULL) {
                SAC_RuntimeError ("hwloc not behaving as expected; turn -mt_bind off.");
            }
            res[idx] = hwloc_bitmap_dup (pu->cpuset);
            idx++;
        }
    }

    SAC_FREE (pus_string);

    return res;
}

void
SAC_HWLOC_init (int threads)
{
    char *pus_string;
    hwloc_obj_type_t socket_obj;
    hwloc_topology_init (&SAC_HWLOC_topology);
    hwloc_topology_load (SAC_HWLOC_topology);

    int num_sockets_available;
    num_sockets_available
      = hwloc_get_nbobjs_by_type (SAC_HWLOC_topology, HWLOC_OBJ_SOCKET);
    if (num_sockets_available < 1) {
#if HWLOC_API_VERSION >= 0x00010b00
        num_sockets_available
          = hwloc_get_nbobjs_by_type (SAC_HWLOC_topology, HWLOC_OBJ_PACKAGE);
        if (num_sockets_available < 1) {
            num_sockets_available
              = hwloc_get_nbobjs_by_type (SAC_HWLOC_topology, HWLOC_OBJ_NUMANODE);
            if (num_sockets_available < 1) {
#endif
                SAC_RuntimeError (
                  "hwloc returned %d sockets, packages and NUMAnodes available. "
                  "Set cpu bind strategy to \"off\".",
                  num_sockets_available);
#if HWLOC_API_VERSION >= 0x00010b00
            } else {
                socket_obj = HWLOC_OBJ_NUMANODE;
            }
        } else {
            socket_obj = HWLOC_OBJ_PACKAGE;
        }
#endif
    } else {
        socket_obj = HWLOC_OBJ_SOCKET;
    }

    int num_cores_available;
    num_cores_available = hwloc_get_nbobjs_by_type (SAC_HWLOC_topology, HWLOC_OBJ_CORE);
    if (num_cores_available < 1) {
        SAC_RuntimeError ("hwloc returned %d cores available. Turn -mt_bind off",
                          num_cores_available);
    }

    int num_pus_available;
    num_pus_available = hwloc_get_nbobjs_by_type (SAC_HWLOC_topology, HWLOC_OBJ_PU);
    if (num_pus_available < 1) {
        SAC_RuntimeError ("hwloc returned %d pus available. Turn -mt_bind off",
                          num_pus_available);
    }

    if (SAC_MT_cpu_bind_strategy == 1) {
        pus_string = strategySimple (threads, num_sockets_available,
                                     num_cores_available / num_sockets_available,
                                     num_pus_available
                                       / (num_sockets_available * num_cores_available));
    } else if (SAC_MT_cpu_bind_strategy == 2) {
        pus_string = strategyEnv (threads, num_sockets_available,
                                  num_cores_available / num_sockets_available,
                                  num_pus_available
                                    / (num_sockets_available * num_cores_available));
    } else {
        SAC_RuntimeError (
          "chosen cpubindstrategy is not yet implemented in the runtime system");
    }

    SAC_TR_LIBSAC_PRINT (
      ("pinning strategy lead to PU pinning string \"%s\"", pus_string));

    SAC_HWLOC_cpu_sets = pusString2cpuSets (pus_string, num_pus_available);

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

#endif /* SAC_MT_MODE > 0 */

#else
static int this_translation_unit = 0xdead;
#endif // ENABLE_HWLOC
