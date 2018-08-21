#include "config.h"

#if ENABLE_HWLOC

#include <hwloc.h>

#include "libsac/essentials/trace.h"
#include "libsac/mt/mt.h"
#include "runtime/hwloc_h/cpubind.h"
#include "libsac/essentials/message.h"
#include "hwloc_data.h"

hwloc_cpuset_t *SAC_HWLOC_cpu_sets;
hwloc_topology_t SAC_HWLOC_topology;

#define SAC_HWLOC_NUM_SOCKETS_VAR_NAME "SAC_NUM_SOCKETS"
#define SAC_HWLOC_NUM_CORES_VAR_NAME "SAC_NUM_CORES"
#define SAC_HWLOC_NUM_PUS_VAR_NAME "SAC_NUM_PUS"

#define SAC_PULIST_EMPTY_CHAR '-'
#define SAC_PULIST_FULL_CHAR '*'

#if SAC_MT_MODE > 0

static char *
strategySocket (int threads, int sockets_avail, int cores_avail, int pus_avail)
{

    int i, j, idx, rem, socket;
    char *res;

    if (threads > sockets_avail * cores_avail * pus_avail) {
        SAC_RuntimeError ("Asking for %d threads on a machine with %d processing units; "
                          "Either decrease the number of threads or turn -mt_bind off",
                          threads, sockets_avail * cores_avail * pus_avail);
    }
    res = (char *)SAC_MALLOC (sizeof (char)
                              * (sockets_avail * cores_avail * pus_avail + 1));
    for (i = 0; i < sockets_avail * cores_avail * pus_avail; i++) {
        res[i] = SAC_PULIST_EMPTY_CHAR;
    }
    res[i] = '\0';

    if (threads > sockets_avail) {

        rem = 0;
        while (threads % sockets_avail != 0) {
            threads--;
            rem++;
        }
        idx = (int)((double)threads / (double)sockets_avail);

        socket = 0;

        for (i = 0; i < sockets_avail; i++) {
            for (j = 0; j < idx; j++) {
                res[i * ((cores_avail * pus_avail * sockets_avail) / sockets_avail) + j]
                  = SAC_PULIST_FULL_CHAR;
                if (rem > 0) {
                    res[socket + idx] = SAC_PULIST_FULL_CHAR;
                    rem--;
                    socket += (cores_avail * pus_avail * sockets_avail) / sockets_avail;
                }
            }
        }
    } else {
        for (i = 0; i < threads; i++) {
            res[i * ((cores_avail * pus_avail * sockets_avail) / sockets_avail)]
              = SAC_PULIST_FULL_CHAR;
        }
    }
    return res;
}

static char *
strategyNuma (int threads, int numa_nodes_avail, int sockets_avail, int cores_avail,
              int pus_avail)
{

    int i, j, idx, numa_node;
    char *res;

    if (threads > sockets_avail * cores_avail * pus_avail) {
        SAC_RuntimeError ("Asking for %d threads on a machine with %d processing units; "
                          "Either decrease the number of threads or turn -mt_bind off",
                          threads, sockets_avail * cores_avail * pus_avail);
    }
    res = (char *)SAC_MALLOC (sizeof (char)
                              * (sockets_avail * cores_avail * pus_avail + 1));
    for (i = 0; i < sockets_avail * cores_avail * pus_avail; i++) {
        res[i] = SAC_PULIST_EMPTY_CHAR;
    }
    res[i] = '\0';

    if (threads < numa_nodes_avail) {

        for (i = 0; i < threads; i++) {
            res[i * ((sockets_avail * cores_avail * pus_avail) / numa_nodes_avail)]
              = SAC_PULIST_FULL_CHAR;
        }
        return res;
    }

    int rem = 0;
    while (threads % ((sockets_avail * cores_avail * pus_avail) / numa_nodes_avail)
           != 0) {
        threads--;
        rem++;
    }
    idx = (int)((double)threads / (double)numa_nodes_avail);
    numa_node = 0;

    for (i = 0; i < numa_nodes_avail; i++) {
        for (j = 0; j < idx; j++) {
            res[i * ((sockets_avail * cores_avail * pus_avail) / numa_nodes_avail) + j]
              = SAC_PULIST_FULL_CHAR;

            if (rem > 0) {
                res[numa_node + idx] = SAC_PULIST_FULL_CHAR;
                rem--;
                numa_node += (sockets_avail * cores_avail * pus_avail) / numa_nodes_avail;
            }
        }
    }
    return res;
}

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
    res = (char *)SAC_MALLOC (sizeof (char)
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

    res = (char *)SAC_MALLOC (sizeof (char)
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
                if (threads > 0) {
                    res[idx] = SAC_PULIST_FULL_CHAR;
                    threads--;
                }
                idx++;
            }
            idx += pus_avail - pus;
        }
        idx += cores_avail - cores;
    }

    return (res);
}

static char *
strategyExtString (int threads, int sockets_avail, int cores_avail, int pus_avail)
{
    int i, len, pus_pinned;
    char *pus_string;
    char *res;

    pus_string = getenv ("SAC_HWLOC_PUS_STRING");

    if (pus_string == NULL) {
        SAC_RuntimeError ("Set environment variable SAC_HWLOC_PUS_STRING");
    }

    len = strlen (pus_string);
    SAC_TR_LIBSAC_PRINT (("pus_string_length=%d | pus avail=%d", len,
                          sockets_avail * cores_avail * pus_avail));

    if (len != sockets_avail * cores_avail * pus_avail) {
        SAC_RuntimeError ("SAC_HWLOC_PUS_STRING incorrect length");
    }

    pus_pinned = 0;
    for (i = 0; i < len; i++) {
        SAC_TR_LIBSAC_PRINT (("pus_string=%c (idx:%d)", pus_string[i], i));
        if (pus_string[i] == SAC_PULIST_EMPTY_CHAR
            || pus_string[i] == SAC_PULIST_FULL_CHAR) {
            if (pus_string[i] == SAC_PULIST_FULL_CHAR)
                pus_pinned++;
            continue;
        }
        SAC_RuntimeError ("SAC_HWLOC_PUS_STRING is not constructed correctly");
    }
    if (pus_pinned != threads) {
        SAC_RuntimeError (
          "SAC_HWLOC_PUS_STRING, pinning config does not match thread num");
    }

    res = (char *)SAC_MALLOC (sizeof (char)
                              * (sockets_avail * cores_avail * pus_avail + 1));

    for (i = 0; i < sockets_avail * cores_avail * pus_avail; i++) {
        res[i] = SAC_PULIST_EMPTY_CHAR;
    }
    res[i] = '\0';

    strcpy (res, pus_string);

    return res;
}

static hwloc_cpuset_t *
pusString2cpuSets (char *pus_string, int num_pus)
{
    int i, idx;
    hwloc_cpuset_t *res = NULL;
    hwloc_obj_t pu;

    if (pus_string != NULL) {
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
    } else {
        SAC_RuntimeError ("No PUS string given, unable to create hwloc cpuset.");
    }

    return res;
}

void
SAC_MT_HWLOC_init (int threads)
{
    char *pus_string = NULL;

    if (!SAC_HWLOC_topology) {
        SAC_RuntimeError ("HWLOC topology is missing, was it allocated?");
    }

    /*
     * Now we compute the intended PU usage from the available sockets,
     * cores and pus (assuming a symmetric architecture), the number
     * of threads we want to create and the strategy provided in -mt_bind
     *
     * The strategy functions return a string of the length of all PUS
     * available. All individual characters in the string either indicate
     * a to-be-used PU ('*') or a not-to -be-used PU ('-').
     */
    if (SAC_MT_cpu_bind_strategy == 1) {
        pus_string = strategySimple (threads, SAC_HWLOC_topo_data->num_sockets_available,
                                     SAC_HWLOC_topo_data->num_cores_available
                                       / SAC_HWLOC_topo_data->num_sockets_available,
                                     SAC_HWLOC_topo_data->num_pus_available
                                       / SAC_HWLOC_topo_data->num_cores_available);
    } else if (SAC_MT_cpu_bind_strategy == 2) {
        pus_string = strategyEnv (threads, SAC_HWLOC_topo_data->num_sockets_available,
                                  SAC_HWLOC_topo_data->num_cores_available
                                    / SAC_HWLOC_topo_data->num_sockets_available,
                                  SAC_HWLOC_topo_data->num_pus_available
                                    / SAC_HWLOC_topo_data->num_cores_available);
    } else if (SAC_MT_cpu_bind_strategy == 3) {
        pus_string = strategyNuma (threads, SAC_HWLOC_topo_data->num_numa_nodes_avail,
                                   SAC_HWLOC_topo_data->num_sockets_available,
                                   SAC_HWLOC_topo_data->num_cores_available
                                     / SAC_HWLOC_topo_data->num_sockets_available,
                                   SAC_HWLOC_topo_data->num_pus_available
                                     / SAC_HWLOC_topo_data->num_cores_available);
    } else if (SAC_MT_cpu_bind_strategy == 4) {
        pus_string = strategySocket (threads, SAC_HWLOC_topo_data->num_sockets_available,
                                     SAC_HWLOC_topo_data->num_cores_available
                                       / SAC_HWLOC_topo_data->num_sockets_available,
                                     SAC_HWLOC_topo_data->num_pus_available
                                       / SAC_HWLOC_topo_data->num_cores_available);

    } else if (SAC_MT_cpu_bind_strategy == 5) {
        pus_string
          = strategyExtString (threads, SAC_HWLOC_topo_data->num_sockets_available,
                               SAC_HWLOC_topo_data->num_cores_available
                                 / SAC_HWLOC_topo_data->num_sockets_available,
                               SAC_HWLOC_topo_data->num_pus_available
                                 / SAC_HWLOC_topo_data->num_cores_available);
    } else {
        SAC_RuntimeError (
          "chosen cpubindstrategy is not yet implemented in the runtime system");
    }

    SAC_TR_LIBSAC_PRINT (
      ("pinning strategy lead to PU pinning string \"%s\"", pus_string));

    /*
     * Eventually, the pus-string is being used to preset the global structure
     * used for the binding process through hwloc:
     */
    SAC_HWLOC_cpu_sets
      = pusString2cpuSets (pus_string, SAC_HWLOC_topo_data->num_pus_available);

    SAC_TR_LIBSAC_PRINT (("Pinning done"));
}

#endif /* SAC_MT_MODE > 0 */

#else
static int this_translation_unit = 0xdead;
#endif // ENABLE_HWLOC
