/*
 *
 * $Log$
 * Revision 1.23  1995/09/01 07:48:46  cg
 * new options -bb (break after writing SIB-file) and
 * -noSIB (don't write SIB-file) explained
 *
 * Revision 1.22  1995/07/24  13:38:56  asi
 * bug fixed
 *
 * Revision 1.21  1995/07/24  13:35:22  asi
 * added -minarray and -noarray_elimination (-noAE)
 *
 * Revision 1.20  1995/07/14  13:28:11  sbs
 * nosacopt inserted.
 *
 * Revision 1.19  1995/07/07  14:58:38  asi
 * added loop unswitching - basic version
 *
 * Revision 1.18  1995/06/26  15:10:50  asi
 * added shortcuts -noCF -noINL -noUNR -noDCR -noPDCR -noLIR and -noIVE
 *
 * Revision 1.17  1995/06/23  14:05:49  hw
 * added -maxoverload
 *
 * Revision 1.16  1995/06/13  08:30:27  asi
 * added -maxoptvar, -maxinline und -maxunroll
 *
 * Revision 1.15  1995/06/09  15:28:46  hw
 *  new option '-fcheck_boundary'  inserted
 *
 * Revision 1.14  1995/06/02  09:54:53  sbs
 * -bs, -nopsiopt, and -noidex_vect_elimination inserted
 *
 * Revision 1.13  1995/05/26  14:26:02  asi
 * function inlineing and loop unrolling added
 *
 * Revision 1.12  1995/05/22  12:06:24  sbs
 * tr option inserted
 *
 * Revision 1.11  1995/05/04  11:40:51  sbs
 * trace option added
 *
 * Revision 1.10  1995/04/10  11:19:36  sbs
 * options I,L,O & g included
 *
 * Revision 1.9  1995/04/05  15:52:05  sbs
 * bug fixed
 *
 * Revision 1.8  1995/04/05  15:50:24  sbs
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
#include "tree.h"
#include "optimize.h"

extern int max_overload; /* defined in main.c */

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
    printf ("\t -c \t\t\tgenerate C-file only\n");
    printf ("\t -s \t\t\tcompile silently\n");

    printf ("\nSTOP OPTIONS:\n");
    printf ("\t -bp \t\t\tstop after scan/parse\n");
    printf ("\t -bi \t\t\tstop after module imports\n");
    printf ("\t -bf \t\t\tstop after flatten\n");
    printf ("\t -bt \t\t\tstop after typecheck\n");
    printf ("\t -bb \t\t\tstop after writing SIB-file\n");
    printf ("\t -bo \t\t\tstop after sac-optimizations\n");
    printf ("\t -bs \t\t\tstop after psi-optimizations\n");
    printf ("\t -br \t\t\tstop after refcount inference\n");
    printf ("\t -bc \t\t\tstop unresolved ICM code\n");

    printf ("\nOPTIMIZATION OPTIONS:\n");
    printf ("\t -noopt \t\t\t\t  no optimizations\n");
    printf ("\n\t -nosacopt \t\t\t\t  no sac optimizations\n");
    printf ("\t -noconstant_folding or -noCF \t\t  no constant folding \n");
    printf ("\t -noinline_functions or -noINL\t\t  no function inlineing \n");
    printf ("\t -nounroll_loops or -noUNR \t\t  no loop unrolling \n");
    printf ("\t -nounswitch_loops or -noUNS \t\t  no loop unswitching \n");
    printf ("\t -nodead_code_removal or -noDCR \t  no dead code removal \n");
    printf (
      "\t -nopartial_dead_code_removal or -noPDCR  no partial_dead code removal \n");
    printf ("\t -noloop_invariant_removal or -noLIR \t  no loop invariant removal \n");
    printf ("\n\t -nopsiopt\t\t\t\t  no psi optimisations\n");
    printf ("\t -noindex_vect_elimination or -noIVE \t  no index vector elimination \n");
    printf ("\t -noarray_elimination or -noAE \t\t  no array elimination \n");

    printf ("\n\t -maxoptvar <no>\treserve <no> variables for optimization\n"
            "\t\t\t\tDefault: -maxoptvar %d\n",
            optvar);
    printf ("\t -maxinline <no>\tinline recursive functions <no> times\n"
            "\t\t\t\tDefault: -maxinline %d\n",
            inlnum);
    printf ("\t -maxunroll <no>\tunroll loops having no more than <no> iterations\n"
            "\t\t\t\tDefault: -maxunroll %d\n",
            unrnum);
    printf ("\t -minarray <no>\t\ttry array elimination for arrays with length <= <no>\n"
            "\t\t\t\tDefault: -minarray %d\n",
            minarray);
    printf ("\t -maxoverload <no>\tfunctions with unknown shape will <no> times"
            " overloaded\n"
            "\t\t\t\tDefault: -maxoverload %d\n",
            max_overload);

    printf ("\nTRACE OPTIONS:\n");
    printf ("\t -t [arupw] \t\ttrace array-opts\n");
    printf ("\t\t\t\ta trace all(same as rupw)\n");
    printf ("\t\t\t\tm memory ops\n");
    printf ("\t\t\t\tr refcount ops\n");
    printf ("\t\t\t\tu user defined function calls\n");
    printf ("\t\t\t\tp primitive function calls\n");
    printf ("\t\t\t\tw with loop execution\n");

    printf ("\nCOMPILER OPTIONS:\n");
    printf ("\t -fcheck_boundary\tcheck boundary of arrays while access\n");
    printf ("\t -noSIB\t\t\tdon't write SIB-file\n");

    printf ("\nC-COMPILER OPTIONS:\n");
    printf ("\t  (these options handed to the C-compiler)\n");
    printf ("\t -g \t\t\tinclude debug information\n");
    printf ("\t -O[123] \t\tC-compiler level of optimization\n");

    printf ("\nENVIROMENT VARIABLES:\n");
    printf ("\t SAC_PATH\t\tsearch paths for program source\n");
    printf ("\t SAC_DEC_PATH\t\tsearch paths for declarations\n");
    printf ("\t SAC_LIBRARY_PATH\tsearch paths for libraries\n");

    DBUG_VOID_RETURN;
}
