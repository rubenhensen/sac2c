/*
 *
 * $Log$
 * Revision 1.4  1994/12/11 17:28:52  sbs
 * -I, -L + enivironment vars inserted
 *
 * Revision 1.3  1994/12/02  12:37:34  sbs
 * Options -pfts inserted
 *
 * Revision 1.2  1994/11/10  15:44:34  sbs
 * RCS-header inserted
 *
 *
 */

#include "usage.h"
#include <stdio.h>
#include "dbug.h"

void
usage (char *prg_name)
{
    DBUG_ENTER ("usage");

    printf ("\n%s: usage: %s [options] [filename]\n\n", prg_name, prg_name);
    printf ("options: -h\t\t\tthis helptext\n");
    printf ("\t -#string\t\toptions (string) for DBUG information\n");
    printf ("\t -I path\t\tspecify additional declaration path\n");
    printf ("\t -L path\t\tspecify additional library path\n");
    printf ("\t -o outfilename\t\tset output to outfilename\n");
    printf ("\t -p \t\t\tstop after scan/parse\n");
    printf ("\t -f \t\t\tstop after flatten\n");
    printf ("\t -t \t\t\tstop after typecheck\n");
    printf ("\t -s \t\t\tcompile silently\n");

    printf ("\nenvironment variables:\n");
    printf ("\t SAC_PATH\t\tsearch paths for program source\n");
    printf ("\t SAC_DEC_PATH\t\tsearch paths for declarations\n");
    printf ("\t SAC_LIBRARY_PATH\tsearch paths for libraries\n");

    DBUG_VOID_RETURN;
}
