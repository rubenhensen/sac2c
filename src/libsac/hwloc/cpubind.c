/**
 * @file cpubind.c
 * @brief Primary functions to initialise and interact with the HWLOC sub-system
 */
#include "config.h"

#if ENABLE_HWLOC

#include <stddef.h>
#include <hwloc.h>

#include "cpubind.h"
#include "libsac/essentials/message.h"

#define SAC_HWLOC_NUM_SOCKETS_VAR_NAME "SAC_NUM_SOCKETS"
#define SAC_HWLOC_NUM_CORES_VAR_NAME "SAC_NUM_CORES"
#define SAC_HWLOC_NUM_PUS_VAR_NAME "SAC_NUM_PUS"

#define SAC_PULIST_EMPTY_CHAR '-'
#define SAC_PULIST_FULL_CHAR '*'

/**
 * Topology information for CPU-binding is stored here.
 * This var is also accessed in mt/hwloc_data.c
 */
hwloc_topo_data_t *SAC_HWLOC_topo_data;

/**
 * @brief Check that we are using the same major version of HWLOC as we used
 *        to compile the runtime libraries. Issue an error if this is not the
 *        case.
 *
 * In practice we should never reach this point as system-level installs of HWLOC will
 * indicate to ldconfig/ldd their version, meaning that at runtime, as we lookup functions from
 * shared libraries, the linker should error out with the wrong version.
 *
 * We use the function for the case where a user chooses to locally install hwloc, and then use
 * LD_LIBRARY_PATH (or even LD_PRELOAD) to point to this shared library.
 */
static void
hwloc_check_version (void)
{
    unsigned version = hwloc_get_api_version();

    if ((version >> 16) != (HWLOC_API_VERSION >> 16)) {
        SAC_RuntimeError ("SaC compiled for hwloc API 0x%x but running on library API 0x%x. "
                          "You may need to point LD_LIBRARY_PATH to the right hwloc library. "
                          "Aborting since the new ABI is not backward compatible.",
                          HWLOC_API_VERSION, version);
    }
}

/**
 * Print out a topological tree of the system using HWLOC-derived information
 *
 * @param obj HWLOC system information structure
 * @param depth How many levels of the tree we want to traverse through
 */
static void
hwloc_print_topology_tree (hwloc_obj_t obj, int depth)
{
    char st[128];
    unsigned i;

    hwloc_obj_type_snprintf (st, sizeof (st), obj, 0);
    printf ("%*s%s", 2 * depth, "", st);
    if (obj->type != HWLOC_OBJ_CORE && obj->type != HWLOC_OBJ_PU) {
        hwloc_obj_attr_snprintf (st, sizeof (st), obj, "#", 0);
        printf (" (%s) #%d\n", st, obj->logical_index);
    } else { // no attr information is avaible for COREs or PUs
        printf (" #%d\n", obj->logical_index);
    }
    for (i = 0; i < obj->arity; i++) {
        hwloc_print_topology_tree (obj->children[i], depth + 1);
    }
}

/**
 * Compare two HWLOC objects for equality
 *
 * @param a Object one
 * @param b Object two
 * @return Boolean integer value
 */
static int
hwloc_compare_objs (hwloc_obj_t a, hwloc_obj_t b)
{
    return a->logical_index == b->logical_index
           && hwloc_compare_types (a->type, b->type) == 0;
}

/**
 * Print HWLOC object information
 *
 * @param obj The object to print
 * @param length Print length
 */
static void
hwloc_print_obj_info (hwloc_obj_t obj, unsigned length)
{
    char st[128];

    hwloc_obj_type_snprintf (st, sizeof (st), obj, 0);
    printf ("## %*s%s", 2 * (length + 2), "", st);

    switch (obj->type) {
    case HWLOC_OBJ_CORE:
    case HWLOC_OBJ_PU:
    case HWLOC_OBJ_OS_DEVICE:
        printf (" #%d", obj->logical_index);
        break;
    default:
        hwloc_obj_attr_snprintf (st, sizeof (st), obj, ", ", 1);
        printf (" (%s) #%d", st, obj->logical_index);
        break;
    }
}

/**
 * snprintf equivalent function for printing HWLOC object information.
 *
 * @param str An allocated string pointer
 * @param size Length of string, not include `\0`
 * @param topo HWLOC topology structure
 * @param cpuset HWLOC cpuset structure
 * @param bind HWLOC bind structure
 * @returns EXIT_SUCCESS or EXIT_FAILURE
 */
int
SAC_HWLOC_info_snprintf (char *str, size_t size, hwloc_topology_t topo,
                         hwloc_cpuset_t cpuset, hwloc_cpuset_t bind)
{
    char st1[128];
    char st2[128];
    hwloc_obj_t obj1 = NULL;
    hwloc_obj_t obj2 = NULL;
    int exits = EXIT_SUCCESS;

    // There should only be one obj - as this cpuset was singlefied before...
    // obj1 = hwloc_get_next_obj_inside_cpuset_by_type(topo, bind, HWLOC_OBJ_CORE, obj1);
    obj1 = hwloc_get_obj_inside_cpuset_by_type (topo, bind, HWLOC_OBJ_CORE, 0);
    if (!obj1) {
        obj1 = hwloc_get_obj_inside_cpuset_by_type (topo, bind, HWLOC_OBJ_PU, 0);
        if (!obj1)
            fprintf (stderr, "-! Unable to find bind object!\n");
    }
    // this should return the package - cpusocket
    // obj2 = hwloc_get_next_obj_inside_cpuset_by_type(topo, cpuset, HWLOC_OBJ_NODE,
    // obj2);
    obj2 = hwloc_get_obj_inside_cpuset_by_type (topo, cpuset, HWLOC_OBJ_NODE, 0);
    if (!obj2) {
        obj2 = hwloc_get_obj_inside_cpuset_by_type (topo, cpuset, HWLOC_OBJ_SOCKET, 0);
        if (!obj2)
            fprintf (stderr, "-! Unable to find cpuset object!\n");
    }

    if (!obj1 || !obj2) {
        exits = EXIT_FAILURE;
    } else {
        hwloc_obj_type_snprintf (st1, sizeof (st1), obj1, 0);
        hwloc_obj_type_snprintf (st2, sizeof (st2), obj2, 0);
        snprintf (str, size, "%s #%d in %s #%d", st1, obj1->logical_index, st2,
                  obj2->logical_index);
    }

    return exits;
}

/**
 * Print human-readable list of HWLOC objects from topology information, and annotate
 * those which match the selection
 *
 * @param sel HWLOC object to search for
 * @param obj HWLOC object to search in and annotate output
 * @param depth How many levels to traverse of the topology tree
 */
static void
hwloc_print_topology_selection (hwloc_obj_t sel_obj, hwloc_obj_t obj, unsigned depth)
{
    unsigned i;

    hwloc_print_obj_info (obj, depth);

    if (hwloc_compare_objs (sel_obj, obj)) {
        printf (" <USING>");
    }

    printf ("\n");

    for (i = 0; i < obj->arity; i++) {
        hwloc_print_topology_selection (sel_obj, obj->children[i], depth + 1);
    }
}

/**
 * Bind current process to given HWLOC object
 *
 * @FIXME We assume that we are in a single-threaded mode!
 * @param cpuset HWLOC object to bind to
 */
void
SAC_HWLOC_bind_on_cpuset (hwloc_cpuset_t cpuset)
{
    char *str;

#if HWLOC_API_VERSION < 0x00010800
    // bind current process to cpuset
    if (hwloc_set_cpubind (SAC_HWLOC_topology, cpuset,
                           HWLOC_CPUBIND_STRICT)) // assumes single-threaded execution
#else
    hwloc_topology_t dup;

    // we need to duplicate the topology because cpubind consumes it
    hwloc_topology_dup (&dup, SAC_HWLOC_topology);

    // bind current process to cpuset
    if (hwloc_set_cpubind (dup, cpuset,
                           HWLOC_CPUBIND_STRICT)) // assumes single-threaded execution
#endif
    {
        SAC_RuntimeError ("Unable to bind to given HWLOC cpuset!");
    }
}

/**
 * From a HWLOC object representing a CPU, find the first listed real core (not hyperthreaded)
 * and return a copy of its HWLOC object representation.
 *
 * @param cpuset HWLOC CPU object to search in
 * @returns A HWLOC cpuset representing a single real core
 */
hwloc_cpuset_t *
SAC_HWLOC_get_core (hwloc_cpuset_t cpuset)
{
    hwloc_bitmap_t *res;
    hwloc_obj_t obj_core = NULL;

    res = (hwloc_cpuset_t *)SAC_MALLOC (sizeof (hwloc_cpuset_t));
    // this will get the first core...
    obj_core = hwloc_get_next_obj_inside_cpuset_by_type (SAC_HWLOC_topology, cpuset,
                                                         HWLOC_OBJ_CORE, obj_core);

    if (obj_core) {
        // get a copy of its cpuset that we may modify
        *res = hwloc_bitmap_dup (obj_core->cpuset);
        // get only one logical processor (in case the core is SMT/hyperthreaded)
        hwloc_bitmap_singlify (*res);
    } else {
        SAC_RuntimeError ("Unable to find a core within the given HWLOC cpuset!");
    }

    return res;
}

/**
 * Initialise the HWLOC sub-system
 */
void
SAC_HWLOC_init ()
{
    hwloc_obj_type_t socket_obj; // FIXME(hans) What the hell is this for?

    /*
     * Check that we are using the same version of HWLOC as we compiled with.
     */
    hwloc_check_version ();

    /*
     * Next we grab the host's topology and store it in the global variable:
     */
    hwloc_topology_init (&SAC_HWLOC_topology);
#if HWLOC_API_VERSION < 0x00020000
    hwloc_topology_set_flags (SAC_HWLOC_topology,
                              HWLOC_TOPOLOGY_FLAG_IO_DEVICES); // include access to IO
                                                               // devices
#else
    hwloc_topology_set_io_types_filter (SAC_HWLOC_topology,
                                        HWLOC_TYPE_FILTER_KEEP_ALL);
#endif
    hwloc_topology_load (SAC_HWLOC_topology);

    /*
     * Allocate our struct to contain all the topology information for later
     */
    SAC_HWLOC_topo_data = (hwloc_topo_data_t *)SAC_MALLOC (sizeof (hwloc_topo_data_t));

    /*
     * Now, we derive how many sockets, cores and pus we have.
     * We store those values in the variables
     * - num_sockets_available
     * - num_cores_available
     * - num_pus_available
     * Note here, that these values are the *total* number of the
     * corresponding entities, not the relative numbers!
     */
#if HWLOC_API_VERSION < 0x00010b00
    SAC_HWLOC_topo_data->num_sockets_available
      = hwloc_get_nbobjs_by_type (SAC_HWLOC_topology, HWLOC_OBJ_SOCKET);
#else
    SAC_HWLOC_topo_data->num_sockets_available
      = hwloc_get_nbobjs_by_type (SAC_HWLOC_topology, HWLOC_OBJ_PACKAGE);
#endif
    if (SAC_HWLOC_topo_data->num_sockets_available < 1) {
#if HWLOC_API_VERSION < 0x00010b00
        SAC_HWLOC_topo_data->num_sockets_available
          = hwloc_get_nbobjs_by_type (SAC_HWLOC_topology, HWLOC_OBJ_NODE);
#else
        SAC_HWLOC_topo_data->num_sockets_available
          = hwloc_get_nbobjs_by_type (SAC_HWLOC_topology, HWLOC_OBJ_NUMANODE);
#endif
        if (SAC_HWLOC_topo_data->num_sockets_available < 1) {
            SAC_RuntimeError (
              "hwloc returned %d sockets, packages and NUMAnodes available. "
              "Set cpu bind strategy to \"off\".",
              SAC_HWLOC_topo_data->num_sockets_available);
        } else {
#if HWLOC_API_VERSION < 0x00010b00
            socket_obj = HWLOC_OBJ_NODE;
#else
            socket_obj = HWLOC_OBJ_NUMANODE;
#endif
        }
    } else {
#if HWLOC_API_VERSION < 0x00010b00
        socket_obj = HWLOC_OBJ_SOCKET;
#else
        socket_obj = HWLOC_OBJ_PACKAGE;
#endif
    }

#if HWLOC_API_VERSION < 0x00010b00
    SAC_HWLOC_topo_data->num_numa_nodes_avail
      = hwloc_get_nbobjs_by_type (SAC_HWLOC_topology, HWLOC_OBJ_NODE);
#else
    SAC_HWLOC_topo_data->num_numa_nodes_avail
      = hwloc_get_nbobjs_by_type (SAC_HWLOC_topology, HWLOC_OBJ_NUMANODE);
#endif
    if (SAC_HWLOC_topo_data->num_numa_nodes_avail < 1) {
        SAC_RuntimeWarning ("hwloc returned %d numa nodes available. Memory binding will "
                            "not work, perhaps you"
                            " should turn -mt_bind off",
                            SAC_HWLOC_topo_data->num_numa_nodes_avail);
    }

    SAC_HWLOC_topo_data->num_cores_available
      = hwloc_get_nbobjs_by_type (SAC_HWLOC_topology, HWLOC_OBJ_CORE);
    if (SAC_HWLOC_topo_data->num_cores_available < 1) {
        SAC_RuntimeError ("hwloc returned %d cores available. Turn -mt_bind off",
                          SAC_HWLOC_topo_data->num_cores_available);
    }

    SAC_HWLOC_topo_data->num_pus_available
      = hwloc_get_nbobjs_by_type (SAC_HWLOC_topology, HWLOC_OBJ_PU);
    if (SAC_HWLOC_topo_data->num_pus_available < 1) {
        SAC_RuntimeError ("hwloc returned %d pus available. Turn -mt_bind off",
                          SAC_HWLOC_topo_data->num_pus_available);
    }
}

/**
 * De-initialise the HWLOC sub-system
 */
void
SAC_HWLOC_cleanup ()
{
    if (SAC_HWLOC_topology) {
        hwloc_topology_destroy (SAC_HWLOC_topology);
        SAC_HWLOC_topology = NULL;
    }
    if (SAC_HWLOC_cpu_sets) {
        SAC_FREE (SAC_HWLOC_cpu_sets);
        SAC_HWLOC_cpu_sets = NULL;
    }
    if (SAC_HWLOC_topo_data) {
        SAC_FREE (SAC_HWLOC_topo_data);
        SAC_HWLOC_topo_data = NULL;
    }
}

#else /* ENABLE_HWLOC */

// compiling an empty file causes errors to be issued, this resolves it.
static int this_translation_unit = 0xdead;

#endif /* ENABLE_HWLOC */
