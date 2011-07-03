/*
 * $Log$
 * Revision 1.2  2005/07/21 15:13:32  sah
 * moved var decl to beginning of block
 *
 * Revision 1.1  2005/07/17 11:42:41  sah
 * Initial revision
 *
 *
 */

#include "serialize_filenames.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "str.h"
#include "memory.h"
#include "filemgr.h"
#include "globals.h"

struct FILENAMEDB {
    const char *name;
    struct FILENAMEDB *next;
};

typedef struct FILENAMEDB filenamedb_t;

static filenamedb_t *database = NULL;

int
SFNgetId (const char *filename)
{
    filenamedb_t *names;
    int result;

    DBUG_ENTER ();

    if (database == NULL) {
        database = (filenamedb_t *)MEMmalloc (sizeof (filenamedb_t));

        database->name = filename;
        database->next = NULL;

        result = 0;
    } else {
        bool found = FALSE;
        result = 0;
        names = database;

        while ((names->next != NULL) && (!found)) {
            if (names->name == filename) {
                found = TRUE;
            } else {
                names = names->next;
                result++;
            }
        }

        if (!found) {
            filenamedb_t *new = (filenamedb_t *)MEMmalloc (sizeof (filenamedb_t));

            new->name = filename;
            new->next = NULL;

            names->next = new;
            result++;
        }
    }

    DBUG_RETURN (result);
}

void
SFNgenerateFilenameTable ()
{
    FILE *filec;
    FILE *fileh;
    int cnt;
    filenamedb_t *names;

    DBUG_ENTER ();

    filec = FMGRwriteOpen ("%s/filenames.c", global.tmp_dirname);
    fileh = FMGRwriteOpen ("%s/filenames.h", global.tmp_dirname);

    fprintf (filec, "#include \"filenames.h\"\n\n");

    fprintf (fileh, "#ifndef _FILENAMES_H_\n#define _FILENAMES_H_\n\n");
    fprintf (fileh, "#define FILENAME( x) __%s__filename##x\n\n", global.modulename);

    names = database;
    cnt = 0;

    while (names != NULL) {
        fprintf (filec, "char *FILENAME( %d) = \"%s\";\n", cnt, names->name);

        fprintf (fileh, "extern char *FILENAME( %d);\n", cnt);

        cnt++;
        names = names->next;
    }

    fprintf (fileh, "\n#endif\n");

    fclose (filec);
    fclose (fileh);

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
