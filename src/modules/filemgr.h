#ifndef _filemgr_h

/*
 *
 * $Log$
 * Revision 1.4  1996/01/07 16:58:23  cg
 * handling of temporary directories modified.
 * cccall.c now only needs 2 of these
 *
 * Revision 1.3  1996/01/05  12:36:29  cg
 * added global variables tmp_dirname, store_dirname, build_dirname
 * and functions WriteOpen, CreateTmpDirectories, RemoveDirectory
 *
 * Revision 1.2  1995/04/05  17:24:16  sbs
 * GenLinkList inserted
 *
 * Revision 1.1  1994/12/11  17:33:27  sbs
 * Initial revision
 *
 *
 */

#include <stdio.h>

#define _filemgr_h

#define MAX_PATH_LEN 1024
#define MAX_FILE_NAME 256

typedef enum { PATH, MODDEC_PATH, MODIMP_PATH } pathkind;

extern char store_dirname[];
extern char build_dirname[];

extern char *FindFile (pathkind p, char *name);
extern void InitPaths ();
extern int AppendPath (pathkind p, char *path);
extern int AppendEnvVar (pathkind p, char *var);

extern FILE *WriteOpen (char *format, ...);
extern void CreateTmpDirectories ();
extern void RemoveDirectory (char *name);

#endif /* _filemgr_h */
