#include "gtest/gtest.h"
#include <cstdlib>

extern "C" {
#include "system.h"
#include "memory.h"
#define DBUG_PREFIX "SYS-TEST"
#include "debug.h"
}

TEST (SystemCalls, SanitizePath)
{
    char *spath;

    const char *path1 = "/tmp/sac/24gga23";
    spath = SYSsanitizePath (path1);

    ASSERT_STREQ (spath, path1);
    spath = MEMfree (spath);

    const char *path2 = "   /tmp/sac/24g ga23    ";
    spath = SYSsanitizePath (path2);

    ASSERT_STREQ (spath, "/tmp/sac/24g\\ ga23");
    spath = MEMfree (spath);

    const char *path3 = "    some new path/sac/24g ga23    ";
    spath = SYSsanitizePath (path3);

    ASSERT_STREQ (spath, "some\\ new\\ path/sac/24g\\ ga23");
    spath = MEMfree (spath);
}

#undef DBUG_PREFIX
