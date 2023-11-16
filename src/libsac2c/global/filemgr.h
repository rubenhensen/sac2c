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
#include "fun-attrs.h"

extern char *FMGRabsName (const char *name);
extern char *FMGRfindFile (pathkind_t p, const char *name);
extern char *FMGRfindFilePath (pathkind_t p, const char *name);
extern void *FMGRmapPath (pathkind_t p, void *(*mapfun) (const char *, void *),
                          void *neutral);
extern void FMGRappendPath (pathkind_t p, const char *path);
extern void FMGRprependPath (pathkind_t p, const char *path);
extern void FMGRsetupPaths (void);
extern FILE *FMGRwriteOpen (const char *format, ...) PRINTF_FORMAT (1, 2);
extern FILE *FMGRreadWriteOpen (const char *format, ...) PRINTF_FORMAT (1, 2);
extern FILE *FMGRreadOpen (const char *format, ...) PRINTF_FORMAT (1, 2);
extern FILE *FMGRwriteOpenExecutable (const char *format, ...) PRINTF_FORMAT (1, 2);
extern FILE *FMGRappendOpen (const char *format, ...) PRINTF_FORMAT (1, 2);
extern FILE *FMGRclose (FILE *file);
extern bool FMGRcheckExistDir (const char *dir);
extern bool FMGRcheckExistFile (const char *file);
extern void FMGRsetFileNames (node *module);
extern void FMGRdeleteTmpDir (void);
extern void FMGRcreateTmpDir (void);
extern void FMGRforEach (const char *path, const char *fileexpr, void *funargs,
                         void(const char *path, const char *file, void *params));
extern char *FMGRdirname (const char *path);
extern char *FMGRbasename (const char *path);
extern char *FMGRstripExt (const char *path);
extern char *FMGRfile2id (const char *path);

#endif /* _SAC_FILEMGR_H_ */
