/*
 *
 * $Log$
 * Revision 1.1  1994/12/11 17:33:27  sbs
 * Initial revision
 *
 *
 */

#include <stdlib.h>
#include <string.h>

#include "dbug.h"

#include "filemgr.h"

static char path_bufs[3][MAX_PATH_LEN];
static int bufsize[3];

/*
 *
 *  functionname  : FindFile
 *  arguments     :
 *  description   :
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : ---
 *
 *  remarks       :
 *
 */

FILE *
FindFile (pathkind p, char *name)
{
    FILE *file = NULL;
    static char buffer[MAX_FILE_NAME];
    static char buffer2[MAX_PATH_LEN];
    char *path;

    DBUG_ENTER ("FindFile");

    strcpy (buffer2, path_bufs[p]);
    path = strtok (buffer2, ":");
    while ((file == NULL) && (path != NULL)) {
        strcpy (buffer, path);
        strcat (buffer, "/");
        strcat (buffer, name);
        DBUG_PRINT ("FILE", ("trying file %s\n", buffer));
        file = fopen (buffer, "r");
        if (file == NULL) {
            path = strtok (NULL, ":");
        }
    }
    DBUG_RETURN (file);
}

/*
 *
 *  functionname  : InitPaths
 *  arguments     :
 *  description   :
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : ---
 *
 *  remarks       :
 *
 */

void
InitPaths ()
{
    int i;

    DBUG_ENTER ("InitPaths");
    for (i = 0; i < 3; i++) {
        bufsize[i] = 1;
        strcat (path_bufs[i], ".");
    }
    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : AppendPath
 *  arguments     : pathkind, path to be appended
 *  description   :
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : ---
 *
 *  remarks       :
 *
 */

int
AppendPath (pathkind p, char *path)
{
    int v, len;

    DBUG_ENTER ("AppendPath");
    len = (strlen (path) + 1);
    if (len + bufsize[p] >= MAX_PATH_LEN)
        v = 0;
    else {
        strcat (path_bufs[p], ":");
        strcat (path_bufs[p], path);
        bufsize[p] += len;
        v = 1;
    }
    DBUG_RETURN (v);
}

/*
 *
 *  functionname  : AppendEnvVar
 *  arguments     : pathkind, path to be appended
 *  description   :
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : ---
 *
 *  remarks       :
 *
 */

int
AppendEnvVar (pathkind p, char *var)
{
    int v = 1, len;
    char *buffer;

    DBUG_ENTER ("AppendEnvVar");

    buffer = getenv (var);
    if (buffer != NULL) {
        len = (strlen (buffer) + 1);
        if (len + bufsize[p] >= MAX_PATH_LEN)
            v = 0;
        else {
            strcat (path_bufs[p], ":");
            strcat (path_bufs[p], buffer);
            bufsize[p] += len;
        }
    }
    DBUG_RETURN (v);
}
