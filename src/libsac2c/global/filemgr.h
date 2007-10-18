/*
 * $Id$
 *
 */

/*
 * Revision 1.1  1994/12/11  17:33:27  sbs
 * Initial revision
 *
 */

/******************************************************************************
 *
 * Filemanager
 *
 * Prefix: FMGR
 *
 *****************************************************************************/

#ifndef _SAC_FILEMGR_H_
#define _SAC_FILEMGR_H_

#include <stdio.h>
#include "types.h"

extern const char *FMGRfindFile (pathkind_t p, const char *name);
extern const char *FMGRfindFilePath (pathkind_t p, const char *name);
extern void *FMGRmapPath (pathkind_t p, void *(*mapfun) (const char *, void *),
                          void *neutral);
extern void FMGRappendPath (pathkind_t p, const char *path);
extern void FMGRsetupPaths ();
extern const char *FMGRabsolutePathname (const char *path);
extern FILE *FMGRwriteOpen (const char *format, ...);
extern FILE *FMGRappendOpen (const char *format, ...);
extern FILE *FMGRclose (FILE *file);
extern bool FMGRcheckExistFile (const char *dir, const char *name);
extern bool FMGRcheckSystemLibrary (const char *name);
extern void FMGRsetFileNames (node *module);
extern void FMGRdeleteTmpDir ();
extern void FMGRcreateTmpDir ();
extern void FMGRforEach (const char *path, const char *fileexpr, void *funargs,
                         void(const char *path, const char *file, void *params));

#endif /* _SAC_FILEMGR_H_ */
