/*
 *
 * $Log$
 * Revision 1.5  1995/04/10 11:08:02  sbs
 * some DBUG_PRINTs inserted
 * FindFile only looks for the explicit name if preceeded by "/"
 * i.e. an absolute path is given
 *
 * Revision 1.4  1995/04/07  09:36:35  sbs
 * bug resulting from change FILE * => char * eliminated
 *
 * Revision 1.3  1995/04/05  17:24:16  sbs
 * GenLinkList inserted
 *
 * Revision 1.2  1995/02/22  14:14:36  hw
 * changed FindFile (now look first for name and later have a look at path )
 *
 * Revision 1.1  1994/12/11  17:33:27  sbs
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

char *
FindFile (pathkind p, char *name)
{
    FILE *file = NULL;
    static char buffer[MAX_FILE_NAME];
    static char buffer2[MAX_PATH_LEN];
    char *path;
    char *result = NULL;

    DBUG_ENTER ("FindFile");

    if (name[0] == '/') { /* absolut path specified! */
        strcpy (buffer, name);
        file = fopen (buffer, "r");
    } else {
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
    }
    if (file) {
        fclose (file);
        result = buffer;
    }

    DBUG_RETURN (result);
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
        DBUG_PRINT ("FILE", ("appending \":%s\" to path %d", path, p));
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
            DBUG_PRINT ("FILE", ("appending \":%s\" to path %d", buffer, p));
            bufsize[p] += len;
        }
    }
    DBUG_RETURN (v);
}
