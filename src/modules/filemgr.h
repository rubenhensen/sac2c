#ifndef _filemgr_h

/*
 *
 * $Log$
 * Revision 1.2  1995/04/05 17:24:16  sbs
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

extern char *FindFile (pathkind p, char *name);
extern void InitPaths ();
extern int AppendPath (pathkind p, char *path);
extern int AppendEnvVar (pathkind p, char *var);

#endif /* _filemgr_h */
