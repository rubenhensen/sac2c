#include "gtest/gtest.h"
#include "config.h"

#include <cstdlib>

#if ENABLE_HWLOC

#include <hwloc.h>

extern "C" {
#include "libsac/hwloc/cpubind.h"
}

TEST (HWLOCRuntime, HWLOCPrintStr)
{
    char test_string[1024] = "\0";
    int ret = 0;
    hwloc_obj_t obj1, obj2;

    hwloc_topology_init (&SAC_HWLOC_topology);

    // read data from XML file to global var exposed by runtime library
    ret = hwloc_topology_set_xml (SAC_HWLOC_topology, CMAKE_TESTS_PATH "/hwloc-test-topology.xml");
    ASSERT_NE (ret, -1);

    hwloc_topology_load (SAC_HWLOC_topology);

#if HWLOC_API_VERSION < 0x00010b00
    obj1 = hwloc_get_obj_by_type(SAC_HWLOC_topology, HWLOC_OBJ_SOCKET, 0);
    obj2 = hwloc_get_obj_by_type(SAC_HWLOC_topology, HWLOC_OBJ_CORE, 0);
#else
    obj1 = hwloc_get_obj_by_type(SAC_HWLOC_topology, HWLOC_OBJ_PACKAGE, 0);
    obj2 = hwloc_get_obj_by_type(SAC_HWLOC_topology, HWLOC_OBJ_CORE, 0);
#endif

    ret = SAC_HWLOC_info_snprintf (test_string, 1023, SAC_HWLOC_topology, obj1->cpuset, obj2->cpuset);
    EXPECT_NE (ret, EXIT_FAILURE);

    ASSERT_STREQ (test_string, "Core #0 in Package #0");

    hwloc_topology_destroy (SAC_HWLOC_topology);
}

TEST (HWLOCRuntime, HWLOCGetCore)
{
    int ret = 0;
    hwloc_obj_t obj;
    hwloc_cpuset_t * cpuset;

    hwloc_topology_init (&SAC_HWLOC_topology);

    // read data from XML file to global var exposed by runtime library
    ret = hwloc_topology_set_xml (SAC_HWLOC_topology, CMAKE_TESTS_PATH "/hwloc-test-topology.xml");
    ASSERT_NE (ret, -1);

    hwloc_topology_load (SAC_HWLOC_topology);

#if HWLOC_API_VERSION < 0x00010b00
    obj = hwloc_get_obj_by_type(SAC_HWLOC_topology, HWLOC_OBJ_SOCKET, 0);
#else
    obj = hwloc_get_obj_by_type(SAC_HWLOC_topology, HWLOC_OBJ_PACKAGE, 0);
#endif

    cpuset = SAC_HWLOC_get_core (obj->cpuset);
    ASSERT_TRUE (cpuset);

    hwloc_bitmap_free (*cpuset);
    hwloc_topology_destroy (SAC_HWLOC_topology);
}

TEST (HWLOCRuntime, HWLOCBind)
{
    int ret = 0;
    hwloc_obj_t obj;
    hwloc_bitmap_t cpuset;

    hwloc_topology_init (&SAC_HWLOC_topology);

    // read data from XML file to global var exposed by runtime library
    ret = hwloc_topology_set_xml (SAC_HWLOC_topology, CMAKE_TESTS_PATH "/hwloc-test-topology.xml");
    ASSERT_NE (ret, -1);

    hwloc_topology_load (SAC_HWLOC_topology);

    obj = hwloc_get_obj_by_type(SAC_HWLOC_topology, HWLOC_OBJ_CORE, 0);
    cpuset = hwloc_bitmap_dup (obj->cpuset);
    hwloc_bitmap_singlify (cpuset);

    // no further check needed as this happens inside
    SAC_HWLOC_bind_on_cpuset (cpuset);

    hwloc_bitmap_free (cpuset);
    hwloc_topology_destroy (SAC_HWLOC_topology);
}

TEST (HWLOCRuntime, HWLOCInitAndCleanUp)
{
    SAC_HWLOC_init ();

    // should be allocated and set
    ASSERT_TRUE (SAC_HWLOC_topology);
    ASSERT_TRUE (SAC_HWLOC_topo_data);
    ASSERT_GT (SAC_HWLOC_topo_data->num_sockets_available, 0);

    SAC_HWLOC_cleanup ();

    // should have been freed (be NULL)
    ASSERT_FALSE (SAC_HWLOC_topology);
    ASSERT_FALSE (SAC_HWLOC_cpu_sets);
    ASSERT_FALSE (SAC_HWLOC_topo_data);
}

#else /* ENABLE_HWLOC */

static const int empty_unit = 0xdeadbeef;

#endif /* ENABLE_HWLOC */
