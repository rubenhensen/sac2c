/******************************************************************************
 *
 * cygwinhelpers.h
 *
 * A collection of functions to be used under cygwin.
 * Mostly used to get the linker to resolve undefined symbols at compile time.
 *
 * Prefix: CYGH
 *
 *****************************************************************************/
#include "config.h"

#if IS_CYGWIN

#ifndef _SAC_CYGWINHELPERS_H_
#define _SAC_CYGWINHELPERS_H_

#include "types.h"

typedef enum { CYGH_sac, CYGH_sac2c } ldvariant_t;

typedef union {
    void *v;
    cyg_fun_table_t *(*f) (cyg_fun_table_t *);
} pass_fun_table_u;

extern void *CYGHbuildLibSearchString (const char *lib, strstype_t kind, void *rest);
extern char *CYGHbuildLibDirectoryString (void);
extern void CYGHaddToDeps (const char *name);
extern char *CYGHgetCompleteLibString (ldvariant_t type);
extern const char *CYGHcygToWinPath (const char *name);

const char *GetSacLibString (void);
const char *GetSac2cLibString (void);

extern void CYGHinitFunTable (void);
extern void CYGHpassToMod (dynlib_t *lib);

#endif /* _SAC_CYGWINHELPERS_H_ */
#endif /* IS_CYGWIN */
