/*
 *
 * $Log$
 * Revision 1.8  1995/04/05 15:50:24  sbs
 * -c inserted
 *
 * Revision 1.7  1995/04/03  06:19:49  sbs
 * options converted to -b[piftorc] and show_icm inserted
 *
 * Revision 1.6  1995/03/17  16:00:35  hw
 * options -noRC , -r inserted
 *
 * Revision 1.5  1995/02/13  17:21:03  asi
 * parmeters noOPT, noCF and noDCR added
 *
 * Revision 1.4  1994/12/11  17:28:52  sbs
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
    printf ("\t -c \t\t\tproduce C-file only");
    printf ("\t -s \t\t\tcompile silently\n");

    printf ("\n");
    printf ("\t -bp \t\t\tstop after scan/parse\n");
    printf ("\t -bi \t\t\tstop after module imports\n");
    printf ("\t -bf \t\t\tstop after flatten\n");
    printf ("\t -bt \t\t\tstop after typecheck\n");
    printf ("\t -bo \t\t\tstop after sac-optimizations\n");
    printf ("\t -br \t\t\tstop after refcount inference\n");
    printf ("\t -bc \t\t\tstop unresolved ICM code\n");

    printf ("\n");
    printf ("\t -noOPT \t\tno optimizations\n");
    printf ("\t -noCF  \t\tno constant folding\n");
    printf ("\t -noDCR \t\tno dead code removal\n");

    printf ("\nenvironment variables:\n");
    printf ("\t SAC_PATH\t\tsearch paths for program source\n");
    printf ("\t SAC_DEC_PATH\t\tsearch paths for declarations\n");
    printf ("\t SAC_LIBRARY_PATH\tsearch paths for libraries\n");

    DBUG_VOID_RETURN;
}
