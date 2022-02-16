#ifndef _MACOS_PLATFORM_H_
#define _MACOS_PLATFORM_H_

#include <stdbool.h>

extern char *MACOSgetSysroot (void);
extern char *MACOSgetSDKVer (void);
extern bool MACOScheckSDKVer (const char *);

#endif // _MACOS_PLATFORM_H_
