/**
 * @file
 * @group platform
 *
 * @brief Functions for MacOS-related operations
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "system.h"
#include "ctinfo.h"
#include "str.h"
#define DBUG_PREFIX "MACOS"
#include "debug.h"

#include "config.h"

#include "macos.h"

/**
 * @brief on macos get sysroot to currently activated XCode
 *
 * @return path to MacOSX.sdk, or NULL on failure.
 */
char *
MACOSgetSysroot (void)
{
#ifdef __APPLE__
    int ret;
#endif // __APPLE__
    char *a;

    DBUG_ENTER ();

#ifdef __APPLE__
    ret = SYSexec_and_read_output ("/usr/bin/xcrun --show-sdk-path", &a);
    if (ret != 0)
    {
        CTIabort (EMPTY_LOC, "xcode-select failed with: %s", a);
    }
    STRstrip (a);
#else
    a = NULL;
#endif // __APPLE__

    DBUG_RETURN (a);
}

/**
 * @brief get MacOS SDK version
 *
 * @return string of version number, or NULL if not on APPLE system
 */
char *
MACOSgetSDKVer (void)
{
    char *v;

    DBUG_ENTER ();

#ifdef __APPLE__
    int ret = SYSexec_and_read_output ("/usr/bin/xcrun --show-sdk-version 2>/dev/null", &v);
    if (ret != 0)
    {
        CTIabort (EMPTY_LOC, "xcrun failed with: %s", v);
    }
    STRstrip (v);
#else
    v = NULL;
#endif // __APPLE__

    DBUG_RETURN (v);
}

/**
 * @brief check MacOS SDK version string against version used to build sac2c
 *
 * @param v version string to check
 * @return TRUE, if versions match, otherwise FALSE
 */
bool
MACOScheckSDKVer (const char *v)
{
    bool match = false;

    DBUG_ENTER ();

#ifdef MACOSSDKVER
    if (STReq (MACOSSDKVER, v))
        match = true;
#else
    CTIwarn (EMPTY_LOC, "MacOS SDK version used to build sac2c is not set!");
#endif // MACOSSDKVER

    DBUG_RETURN (match);
}
