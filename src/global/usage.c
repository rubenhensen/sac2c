/*
 *
 * $Log$
 * Revision 1.43  1996/09/11 06:15:08  cg
 * Added options -libstat, -deps, -noranlib, -l1, l2, l3
 *
 * Revision 1.42  1996/08/09  16:44:12  asi
 * dead function removal added
 *
 * Revision 1.41  1996/05/29  14:18:57  sbs
 * inserted noRCO opt_rco!
 *
 * Revision 1.40  1996/01/25  18:39:25  cg
 * added new stop options  using compiler phase numbers
 *
 * Revision 1.39  1996/01/17  16:49:21  asi
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
#include "Error.h"

void
usage (char *prg_name)
{
    DBUG_ENTER ("usage");

    printf ("\n\n\tsac2c  --  The ultimate SAC compiler\n\n");

    printf ("\nNAME:\n\n");

    printf ("\t%s\n", prgname);

    printf ("\n\nSYNOPSIS:\n\n");

    printf ("\t%s [options] [filename]\n", prg_name);

    printf ("\n\nOVERALL OPTIONS:\n\n");

    printf ("\t -h\t\t\tthis helptext\n");
    printf ("\t -libstat\t\tprint status information about a SAC library file\n");
    printf ("\t -deps\t\t\tupdate Makefile in current directory with \n");
    printf ("\t\t\t\tdependencies from imported modules/classes.\n");

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

    printf ("\t -bp -b2\tstop after: %s\n", compiler_phase_name[2]);
    printf ("\t -bi -b3\tstop after: %s\n", compiler_phase_name[3]);
    printf ("\t -bb -b4\tstop after: %s\n", compiler_phase_name[4]);
    printf ("\t -bj -b5\tstop after: %s\n", compiler_phase_name[5]);
    printf ("\t -bf -b6\tstop after: %s\n", compiler_phase_name[6]);
    printf ("\t -bt -b7\tstop after: %s\n", compiler_phase_name[7]);
    printf ("\t -bd -b8\tstop after: %s\n", compiler_phase_name[8]);
    printf ("\t -bm -b9\tstop after: %s\n", compiler_phase_name[9]);
    printf ("\t -by -b10\tstop after: %s\n", compiler_phase_name[10]);
    printf ("\t -bw -b11\tstop after: %s\n", compiler_phase_name[11]);
    printf ("\t -be -b12\tstop after: %s\n", compiler_phase_name[12]);
    printf ("\t -bq -b13\tstop after: %s\n", compiler_phase_name[13]);
    printf ("\t -bv -b14\tstop after: %s\n", compiler_phase_name[14]);
    printf ("\t -bo -b15\tstop after: %s\n", compiler_phase_name[15]);
    printf ("\t -ba \t\tstop after: Array elimination\n");
    printf ("\t -bs -b16\tstop after: %s\n", compiler_phase_name[16]);
    printf ("\t -br -b17\tstop after: %s\n", compiler_phase_name[17]);
    printf ("\t -bl -b18\tstop after: %s\n", compiler_phase_name[18]);
    printf ("\t -bc -b19\tstop after: %s\n", compiler_phase_name[19]);

    printf ("\n\nOPTIMIZATION OPTIONS:\n\n");

    printf ("\t -noopt \t\t\t\t  no optimizations\n");
    printf ("\n\t -nosacopt \t\t\t\t  no sac optimizations\n");
    printf ("\t -noconstant_folding or -noCF \t\t  no constant folding \n");
    printf ("\t -noinline_functions or -noINL\t\t  no function inlineing \n");
    printf ("\t -nounroll_loops or -noUNR \t\t  no loop unrolling \n");
    printf ("\t -nounswitch_loops or -noUNS \t\t  no loop unswitching \n");
    printf ("\t -nodead_code_removal or -noDCR \t  no dead code removal \n");
    printf ("\t -nodead_function_removal or -noDFR  no dead function removal \n");
    printf ("\t -noloop_invariant_removal or -noLIR \t  no loop invariant removal \n");
    printf ("\t -nocse or -noCSE \t\t\t  no common subexpression elimination \n");
    printf ("\n\t -nopsiopt\t\t\t\t  no psi optimisations\n");
    printf ("\t -noindex_vect_elimination or -noIVE \t  no index vector elimination \n");
    printf ("\t -noarray_elimination or -noAE \t\t  no array elimination \n");
    printf ("\n\t -norefcount_opt or -noRCO \t\t  no refcount optimization \n");

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

    printf ("\n\nLINK OPTIONS:\n\n");

    printf ("\t -noranlib\t\tdon't use ranlib (for systems without ranlib)\n");
    printf ("\t -l<n>\t\t\tlink level for generating SAC library\n");
    printf ("\t\t\t\t1: compile to one large object file\n");
    printf ("\t\t\t\t2: compile to archive of object files (default)\n");
    printf ("\t\t\t\t3: compile to archive of object files\n");
    printf ("\t\t\t\t   and include required non-standard modules\n");

    printf ("\n\nC-COMPILER OPTIONS:\t(handed to the C-compiler)\n\n");

    /*   printf("\t  (These options are handed to the C-compiler)\n");*/
    printf ("\t -g \t\t\tinclude debug information\n");
    printf ("\t -O[123] \t\tC-compiler level of optimization\n");

    printf ("\n\nENVIRONMENT VARIABLES:\n\n");

    printf ("\t SAC_PATH\t\tsearch paths for program source\n");
    printf ("\t SAC_DEC_PATH\t\tsearch paths for declarations\n");
    printf ("\t SAC_LIBRARY_PATH\tsearch paths for libraries\n");

    printf ("\n\nAUTHORS:\n\n");

    printf ("\t Sven-Bodo Scholz\n");
    printf ("\t Henning Wolf\n");
    printf ("\t Arne Sievers\n");
    printf ("\t Clemens Grelck\n");

    printf ("\n\nBUGS:\n\n");

    printf ("\t Bugs ??  We ????\n");

    printf ("\n");

    DBUG_VOID_RETURN;
}
