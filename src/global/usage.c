/*
 *
 * $Log$
 * Revision 1.66  1998/05/15 11:33:22  srs
 * added -no:wli
 *
 * Revision 1.65  1998/05/13 14:04:36  srs
 * added -noWLUNR and -maxwlunroll
 *
 * Revision 1.64  1998/05/05 12:44:28  srs
 * changed text for -noWLT
 *
 * Revision 1.63  1998/05/05 12:36:04  srs
 * added -noWLT
 *
 * Revision 1.62  1998/04/29 17:09:44  dkr
 * changed phase order
 *
 * Revision 1.61  1998/04/02 16:05:24  dkr
 * new compiler phase:
 *   generating concurrent regions (phase 18)
 *
 * Revision 1.60  1998/04/01 19:07:51  dkr
 * renamed break specifiers for precompile
 *
 * Revision 1.59  1998/03/31 13:56:11  dkr
 * new break specifiers for precompile
 *
 * Revision 1.58  1998/03/02 13:59:02  cg
 * added new option -target
 *
 * Revision 1.57  1998/02/25 09:13:39  cg
 * usage.c streamlined
 * break specifiers added
 *
 * Revision 1.56  1998/02/06 13:32:58  srs
 * new switch -noWLF
 *
 * Revision 1.55  1997/10/02 13:21:50  cg
 * option -Mlib explained again.
 *
 * Revision 1.54  1997/09/13 15:15:24  dkr
 * fixed an error in the desription of the flag -M
 *
 * Revision 1.53  1997/08/07 15:59:36  dkr
 * eleminated spelling mistake
 *
 * Revision 1.52  1997/08/07 15:28:33  dkr
 * eliminated typerror
 *
 * Revision 1.50  1997/08/07 11:13:38  dkr
 * added option -_DBUG<from>/<to>/<string>
 *
 * Revision 1.49  1997/06/03 08:40:21  sbs
 * -D option added
 *
 * Revision 1.48  1997/05/28  12:36:51  sbs
 * Profiling integrated
 *
 * Revision 1.46  1997/05/06  13:54:18  sbs
 * -a[at] option included
 *
 * Revision 1.45  1997/03/19  13:42:02  cg
 * compiler option -l3 no longer supported
 *
 * Revision 1.44  1997/03/11  16:32:44  cg
 * compiler option -deps replaced by -M
 *
 * Revision 1.43  1996/09/11  06:15:08  cg
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

#include <stdio.h>

#include "globals.h"
#include "dbug.h"

void
usage ()
{
    DBUG_ENTER ("usage");

    printf ("\n\n\tsac2c  --  The ultimate SAC compiler\n\n"

            "\nNAME:\n\n"

            "\tsac2c\n"

            "\n\nSYNOPSIS:\n\n"

            "\tsac2c [options] [filename]\n"

            "\n\nOVERALL OPTIONS:\n\n"

            "\t -h\t\t\t\tthis helptext\n"
            "\t -libstat\t\t\tprint status information about a SAC library file\n"
            "\t -D <cpp-var><=value>\t\tset <cpp-var> (to <value>) when running\n"
            "\t\t\t\t\t C-preprocessor\n"
            "\t -M\t\t\t\tonly detect dependencies from imported\n"
            "\t\t\t\t\t  modules/classes and write them to stdout.\n"
            "\t\t\t\t\t  Dependences from declaration files are\n"
            "\t\t\t\t\t  considered.\n"
            "\t -Mlib\t\t\t\tonly detect dependencies from imported\n"
            "\t\t\t\t\t  modules/classes and write them to stdout.\n"
            "\t\t\t\t\t  Dependences from declaration files as well as\n"
            "\t\t\t\t\t  library files are (recursively) considered.\n"
            "\t -# <string>\t\t\toptions (string) for DBUG information\n"
            "\t\t\t\t\t  (\"-#<string>\" is equivalent to \"-_DBUG//<string>\")\n"
            "\t -_DBUG<from>/<to>/<string>\tDBUG information only in compiler phases\n"
            "\t\t\t\t\t <from>..<to>\n"
            "\t\t\t\t\t  Default: <from> = 1, <to> = last compiler phase\n"
            "\t -I <path>\t\t\tspecify additional declaration path\n"
            "\t -L <path>\t\t\tspecify additional library path\n"
            "\t -o <name>\t\t\tfor compilation of programs:\n"
            "\t\t\t\t\t  write executable to specified file\n"
            "\t\t\t\t\tfor compilation of module/class implementations:\n"
            "\t\t\t\t\t  write library to specified directory\n"
            "\t -c \t\t\t\tgenerate C-file only\n"
            "\t -v <n> \t\t\tverbose level\n"
            "\t\t\t\t\t  0: error messages only\n"
            "\t\t\t\t\t  1: error messages and warnings\n"
            "\t\t\t\t\t  2: basic compile time information\n"
            "\t\t\t\t\t  3: full compile time information (default)\n");

    printf ("\n\nBREAK OPTIONS:\n\n");

    printf ("\t -bu -b1\tstop after: %s\n", compiler_phase_name[1]);
    printf ("\t -bp -b2\tstop after: %s\n", compiler_phase_name[2]);
    printf ("\t -bi -b3\tstop after: %s\n", compiler_phase_name[3]);
    printf ("\t -bb -b4\tstop after: %s\n", compiler_phase_name[4]);
    printf ("\t -bj -b5\tstop after: %s\n", compiler_phase_name[5]);
    printf ("\t -bf -b6\tstop after: %s\n", compiler_phase_name[6]);
    printf ("\t -bt -b7\tstop after: %s\n", compiler_phase_name[7]);
    printf ("\t -bd -b8\tstop after: %s\n", compiler_phase_name[8]);
    printf ("\t -bm -b9\tstop after: %s\n", compiler_phase_name[9]);
    printf ("\t -ba -b10\tstop after: %s\n", compiler_phase_name[10]);
    printf ("\t -bw -b11\tstop after: %s\n", compiler_phase_name[11]);
    printf ("\t -be -b12\tstop after: %s\n", compiler_phase_name[12]);
    printf ("\t -bq -b13\tstop after: %s\n", compiler_phase_name[13]);
    printf ("\t -bv -b14\tstop after: %s\n", compiler_phase_name[14]);
    printf ("\t -bo -b15\tstop after: %s\n", compiler_phase_name[15]);
    printf ("\t -bs -b16\tstop after: %s\n", compiler_phase_name[16]);
    printf ("\t -br -b17\tstop after: %s\n", compiler_phase_name[17]);
    printf ("\t -bn -b18\tstop after: %s\n", compiler_phase_name[18]);
    printf ("\t -by -b19\tstop after: %s\n", compiler_phase_name[19]);
    printf ("\t -bl -b20\tstop after: %s\n", compiler_phase_name[20]);
    printf ("\t -bc -b21\tstop after: %s\n", compiler_phase_name[21]);

    printf ("\n\nBREAK SPECIFIERS:\n\n"
            "\tBreak specifiers allow you to stop the compilation process\n"
            "\twithin a particular phase.\n\n"
            "\tCurrently supported:\n\n"

            "\t-bo:inl   \t-b15:inl   \tstop after function inlining\n"
            "\t-bo:ae    \t-b15:ae    \tstop after array elimination\n"
            "\t-bo:wli   \t-b15:wli   \tstop after withloop information gathering\n"
            "\t-bo:cyc   \t-b15:cyc   \tstop after one complete optimization cycle\n\n"

            "\t-bn:cubes \t-b18:cubes \tstop after cube-building\n"
            "\t-bn:segs  \t-b18:segs  \tstop after choice of segments\n"
            "\t-bn:split \t-b18:split \tstop after splitting\n"
            "\t-bn:block \t-b18:block \tstop after hierarchical blocking\n"
            "\t-bn:ublock\t-b18:ublock\tstop after unrolling-blocking\n"
            "\t-bn:merge \t-b18:merge \tstop after merging\n"
            "\t-bn:opt   \t-b18:opt   \tstop after optimization\n"
            "\t-bn:fit   \t-b18:fit   \tstop after fitting\n"
            "\t-bn:norm  \t-b18:norm  \tstop after normalization\n");

    printf ("\n\nOPTIMIZATION OPTIONS:\n\n"

            "\t -noOPT\t\tno optimizations at all\n"
            "\n\t -noCF \t\tno constant folding \n"
            "\t -noINL\t\tno function inlining \n"
            "\t -noLUNR\tno loop unrolling \n"
            "\t -noWLUNR\tno withloop unrolling \n"
            "\t -noUNS\t\tno loop unswitching \n"
            "\t -noDCR\t\tno dead code removal \n"
            "\t -noDFR\t\tno dead function removal \n"
            "\t -noLIR\t\tno loop invariant removal \n"
            "\t -noCSE\t\tno common subexpression elimination \n"
            "\t -noWLT\t\tno withloop transformations (implies -noWLF) \n"
            "\t -noWLF\t\tno withloop folding \n"
            "\t -noIVE\t\tno index vector elimination \n"
            "\t -noAE \t\tno array elimination \n"
            "\t -noRCO\t\tno refcount optimization \n"
            "\n\tLower case letters may be used instead!\n");

    printf ("\n\t -maxoptvar <no>\treserve <no> variables for optimization\n"
            "\t\t\t\t  Default: -maxoptvar %d\n",
            optvar);
    printf ("\t -maxinline <no>\tinline recursive functions <no> times\n"
            "\t\t\t\t  Default: -maxinline %d\n",
            inlnum);
    printf ("\t -maxunroll <no>\tunroll loops having no more than <no> iterations\n"
            "\t\t\t\t  Default: -maxunroll %d\n",
            unrnum);
    printf ("\t -maxwlunroll <no>\tunroll withloops having no more than <no> elements\n"
            "\t\t\t\t  Default: -maxwlunroll %d\n",
            wlunrnum);
    printf ("\t -minarray <no>\t\ttry array elimination for arrays with length <= <no>\n"
            "\t\t\t\t  Default: -minarray %d\n",
            minarray);
    printf (
      "\t -maxoverload <no>\tfunctions with unknown shape will <no> times overloaded\n"
      "\t\t\t\t  Default: -maxoverload %d\n",
      max_overload);

    printf ("\n\nDEBUG OPTIONS:\n\n"

            "\t -t [arupwm] \t\ttrace program execution\n"
            "\t\t\t\t  a: trace all (same as rupwm)\n"
            "\t\t\t\t  m: trace memory operations\n"
            "\t\t\t\t  r: trace refcount operations\n"
            "\t\t\t\t  u: trace user defined function calls\n"
            "\t\t\t\t  p: trace primitive function calls\n"
            "\t\t\t\t  w: trace with loop execution\n"

            "\t -dcheck_boundary\tcheck boundary of arrays upon access\n"
            "\t -dnocleanup\t\tdon't remove temporary files and directories\n"
            "\t -dcheck_malloc\t\tcheck success of memory allocations\n"

            "\n\nPROFILING OPTIONS:\n\n"

            "\t -p [afilw] \t\tinclude runtime analysis\n"
            "\t\t\t\t  a: analyse all (same as filw)\n"
            "\t\t\t\t  f: analyse time spend in non-inline functions\n"
            "\t\t\t\t  i: analyse time spend in inline functions\n"
            "\t\t\t\t  l: analyse time spend in library functions\n"
            "\t\t\t\t  w: analyse time spent in with-loops\n"

            "\n\nLINK OPTIONS:\n\n"

            "\t -l <n>\t\t\tlink level for generating SAC library\n"
            "\t\t\t\t  1: compile to one large object file\n"
            "\t\t\t\t  2: compile to archive of object files (default)\n"

            "\n\nC-COMPILER OPTIONS:\n\n"

            "\t -g \t\t\tinclude debug information into object code\n"
            "\t -O [0123] \t\tC compiler level of optimization\n"
            "\t\t\t\tdefault: 0\n"
            "\n\tThe actual effects of the above options are C compiler specific!\n"

            "\n\nCUSTOMIZATION\n\n"

            "\t-target <name>\tspecify a particular compilation target.\n"
            "\t\t\tCompilation targets are used to customize sac2c for various\n"
            "\t\t\ttarget architectures, operating systems, and C compilers.\n"
            "\t\t\tThe target description is read either from the installation\n"
            "\t\t\tspecific file $SACBASE/runtime/sac2crc or from a file named\n"
            "\t\t\t.sac2crc within the user's home directory.\n"

            "\n\nENVIRONMENT VARIABLES:\n\n"

            "\t SAC_PATH\t\tsearch paths for program source\n"
            "\t SAC_DEC_PATH\t\tsearch paths for declarations\n"
            "\t SAC_LIBRARY_PATH\tsearch paths for libraries\n"

            "\n\nAUTHORS:\n\n"

            "\t Sven-Bodo Scholz\n"
            "\t Henning Wolf\n"
            "\t Arne Sievers\n"
            "\t Clemens Grelck\n"
            "\t Dietmar Kreye\n"
            "\t Soeren Schwartz\n"

            "\n\nBUGS:\n\n"

            "\t Bugs ??  We ????\n"

            "\n");

    DBUG_VOID_RETURN;
}
