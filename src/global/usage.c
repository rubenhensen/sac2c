/*
 *
 * $Log$
 * Revision 1.39  1996/01/17 16:49:21  asi
 * added common subexpression elimination
 *
 * Revision 1.38  1996/01/16  16:44:04  cg
 * added/modified debug options -dnocleanup, -dcheck_malloc
 * and -dcheck_boundary
 *
 * Revision 1.37  1996/01/05  12:30:23  cg
 * new command line option -noclean and -o may specify target
 * directory for compiling module/class implementations
 *
 * Revision 1.36  1996/01/04  16:58:04  asi
 * includes now globals.h instead of optimize.h
 *
 * Revision 1.35  1995/12/21  16:08:56  cg
 * added option -flink_module
 *
 * Revision 1.34  1995/12/01  20:26:49  cg
 * changed compilation sequence: objinit.c now after import.c
 *
 * Revision 1.33  1995/12/01  17:12:23  cg
 * added break parameter -bl to stop after precompiler
 *
 * Revision 1.32  1995/11/16  19:43:21  cg
 * added new break parameter -bv to stop after removing void-functions
 *
 * Revision 1.31  1995/11/10  15:01:59  cg
 * new command line option -v<n> to set verbose level
 *
 * Revision 1.30  1995/11/06  14:16:14  cg
 * added new break option -bq to break after uniqueness check
 *
 * Revision 1.29  1995/11/01  08:02:34  cg
 * added new break paramter -be to break after object handling.
 *
 * Revision 1.28  1995/10/26  18:20:43  cg
 * modified sequence of compiler phases considered
 * (checkdec moved behind typechecker)
 *
 * Revision 1.27  1995/10/22  17:27:04  cg
 * added new break option -bd to stop after checking
 * module/class declaration.
 *
 * Revision 1.26  1995/10/20  09:23:32  cg
 * added new break parameter to -by to break after analysis
 *
 * Revision 1.25  1995/10/16  17:56:25  cg
 * new break option '-bj' to stop after object init transformation
 *
 * Revision 1.24  1995/10/05  16:04:03  cg
 * break option -bm added.
 *
 * Revision 1.23  1995/09/01  07:48:46  cg
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
#include "globals.h"
#include "dbug.h"
#include "tree.h"
#include "optimize.h"

void
usage (char *prg_name)
{
    DBUG_ENTER ("usage");

    printf ("\n\nNAME:\n\n");

    printf ("\t%s\n", prgname);

    printf ("\n\nSYNOPSIS:\n\n");

    printf ("\t%s [options] [filename]\n", prg_name);

    printf ("\n\nOVERALL OPTIONS:\n\n");

    printf ("\t -h\t\t\tthis helptext\n");
    printf ("\t -#string\t\toptions (string) for DBUG information\n");
    printf ("\t -I path\t\tspecify additional declaration path\n");
    printf ("\t -L path\t\tspecify additional library path\n");
    printf ("\t -o name\t\tfor compilation of programs:\n");
    printf ("\t\t\t\t  write executable to specified file\n");
    printf ("\t\t\t\tfor compilation of module/class implementations:\n");
    printf ("\t\t\t\t  write library to specified directory\n");
    printf ("\t -c \t\t\tgenerate C-file only\n");
    printf ("\t -v<n> \t\t\tverbose level\n");
    printf ("\t\t\t\t\t0: error messages only\n");
    printf ("\t\t\t\t\t1: error messages and warnings\n");
    printf ("\t\t\t\t\t2: basic compile time information\n");
    printf ("\t\t\t\t\t3: full compile time information (default)\n");

    printf ("\n\nSTOP OPTIONS:\n\n");

    printf ("\t -bp \t\t\tstop after scan/parse\n");
    printf ("\t -bi \t\t\tstop after module imports\n");
    printf ("\t -bj \t\t\tstop after object init transformation\n");
    printf ("\t -bf \t\t\tstop after flatten\n");
    printf ("\t -bt \t\t\tstop after typecheck\n");
    printf ("\t -bd \t\t\tstop after checking module/class declaration\n");
    printf ("\t -bm \t\t\tstop after resolving implicit types\n");
    printf ("\t -by \t\t\tstop after analysing functions\n");
    printf ("\t -bb \t\t\tstop after writing SIB-file\n");
    printf ("\t -be \t\t\tstop after handling objects\n");
    printf ("\t -bq \t\t\tstop after checking uniqueness\n");
    printf ("\t -bv \t\t\tstop after generating purely functional code\n");
    printf ("\t -bo \t\t\tstop after sac-optimizations\n");
    printf ("\t -ba \t\t\tstop after array-elimination\n");
    printf ("\t -bs \t\t\tstop after psi-optimizations\n");
    printf ("\t -br \t\t\tstop after refcount inference\n");
    printf ("\t -bl \t\t\tstop after preparing code generation\n");
    printf ("\t -bc \t\t\tstop after generating ICM code\n");

    printf ("\n\nOPTIMIZATION OPTIONS:\n\n");

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
    printf ("\t -nocse or -noCSE \t\t\t  no common subexpression elimination \n");
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

    printf ("\n\nDEBUG OPTIONS:\n\n");

    printf ("\t -t [arupwm] \t\ttrace program execution\n");
    printf ("\t\t\t\ta: trace all (same as rupwm)\n");
    printf ("\t\t\t\tm: trace memory operations\n");
    printf ("\t\t\t\tr: trace refcount operations\n");
    printf ("\t\t\t\tu: trace user defined function calls\n");
    printf ("\t\t\t\tp: trace primitive function calls\n");
    printf ("\t\t\t\tw: trace with loop execution\n");

    printf ("\t -dcheck_boundary\tcheck boundary of arrays upon access\n");
    printf ("\t -dnocleanup\t\tdon't remove temporary files and directories\n");
    printf ("\t -dcheck_malloc\t\tcheck success of memory allocations\n");

    printf ("\n\nC-COMPILER OPTIONS:\t(handed to the C-compiler)\n\n");

    /*   printf("\t  (These options are handed to the C-compiler)\n");*/
    printf ("\t -g \t\t\tinclude debug information\n");
    printf ("\t -O[123] \t\tC-compiler level of optimization\n");

    printf ("\n\nENVIRONMENT VARIABLES:\n\n");

    printf ("\t SAC_PATH\t\tsearch paths for program source\n");
    printf ("\t SAC_DEC_PATH\t\tsearch paths for declarations\n");
    printf ("\t SAC_LIBRARY_PATH\tsearch paths for libraries\n");

    printf ("\n");

    DBUG_VOID_RETURN;
}
