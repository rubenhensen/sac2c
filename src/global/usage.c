/*
 *
 * $Log$
 * Revision 2.13  1999/07/28 13:01:14  jhs
 * Added information about spmdcons.
 *
 * Revision 2.12  1999/07/21 16:28:19  jhs
 * needed_sync_fold introduced, max_sync_fold adjusted, command-line and usage
 * updated.
 *
 * Revision 2.11  1999/07/09 07:31:24  cg
 * SAC heap manager integrated into sac2c.
 *
 * Revision 2.10  1999/06/11 12:55:17  cg
 * Added options -cshost, -csfile, -csdir.
 * Explanation of cache simulation improved in general.
 *
 * Revision 2.9  1999/06/10 09:49:03  cg
 * Added usage of option -cshost.
 *
 * Revision 2.8  1999/05/26 14:32:23  jhs
 * Added options MTO and SBE for multi-thread optimsation and
 * synchronisation barrier elimination, both options are by
 * default disabled.
 *
 * Revision 2.7  1999/05/20 14:40:10  dkr
 * output adjusted to 80 columns
 *
 * Revision 2.6  1999/05/20 14:09:34  cg
 * typos fixed.
 *
 * Revision 2.5  1999/05/18 12:54:13  cg
 * Option -minae renamed to -maxae.
 * No default value printed for option -numthreads
 *
 * Revision 2.4  1999/05/12 14:28:54  cg
 * command line options streamlined.
 *
 * Revision 2.3  1999/04/14 09:22:00  cg
 * Cache simulation is now explained in detail.
 *
 * Revision 2.2  1999/03/31 11:30:27  cg
 * added command line parameter -cachesim
 *
 * Revision 2.1  1999/02/23 12:40:16  sacbase
 * new release made
 *
 * Revision 1.90  1999/02/19 17:26:03  dkr
 * changed -efence to -defence
 *
 * Revision 1.89  1999/02/19 17:06:13  dkr
 * flag -efence added
 *
 * Revision 1.88  1999/02/15 13:34:09  sbs
 * added -noDLAW opt_dlaw;
 *
 * Revision 1.87  1999/02/15 10:33:12  sbs
 * added some .... bugs
 *
 * Revision 1.86  1999/02/06 16:38:04  dkr
 * output of 'sac2c -h' fitted to terminal size of 80 cols
 *
 * Revision 1.85  1999/02/01 19:46:19  srs
 * fixed typo
 *
 * Revision 1.84  1999/01/26 14:25:08  cg
 * Added option -intrinsic p for intrinsic array-valued psi().
 *
 * Revision 1.83  1999/01/18 12:53:29  sbs
 * -b15:cycN:wlt, -b15:cycN:wlf, -b15:cycN:cf2 inserted
 *
 * Revision 1.82  1999/01/15 15:21:58  cg
 * modified intrinsic option.
 *
 * Revision 1.81  1999/01/07 14:01:01  sbs
 * more sophisticated breaking facilities inserted;
 * Now, a break in a specific cycle can be triggered!
 *
 * Revision 1.80  1998/12/07 17:31:04  cg
 * Now, the version identifier and the target platform are
 * printed as well with the -h and -i options.
 * The information is taken from global vars version_id and
 * target_platform
 *
 * Revision 1.79  1998/12/01 09:41:24  cg
 * bug fixed in description of -trace, -profile, and -intrinsic options.
 * added new section DESCRIPTION with general information on sac2c.
 * some more polishing on the help text
 * copyright/license notice updated.
 *
 * Revision 1.78  1998/10/26 12:50:18  cg
 * Mechanism to store sac2c build information now works correctly.
 * Compiler options renamed:
 * -t -> -trace
 * -p -> -profile
 * new compiler option -intrinsic to select intrinsic implementations
 * of array operations instead of with-loop based ones.
 *
 * Revision 1.77  1998/10/23 14:29:46  cg
 * added the new command line option -inparsize <no> which allows to
 * specify a minimum generator size for with-loops to be executed in
 * parallel if such execution is enabled.
 * The information stored by the global variable min_parallel_size.
 *
 * Revision 1.76  1998/07/23 10:08:06  cg
 * sac2c option -mt-static -mt-dynamic -mt-all renamed to
 * -mtstatic, -mtdynamic, -mtall resepctively
 *
 * Revision 1.75  1998/07/10 15:20:04  cg
 * included option -i to display copyright/disclaimer
 *
 * Revision 1.74  1998/07/07 13:41:08  cg
 * implemented the command line option -mt-all
 *
 * Revision 1.73  1998/06/30 12:41:43  cg
 * dynamic command line option -threads replaced by -mt.
 *
 * Revision 1.72  1998/06/29 08:52:19  cg
 * streamlined tracing facilities
 * tracing on new with-loop and multi-threading operations implemented
 *
 * Revision 1.71  1998/06/23 15:05:58  cg
 * added command line options -dcccall and -dshow_syscall
 *
 * Revision 1.70  1998/06/19 16:53:51  dkr
 * added -noUIP
 *
 * Revision 1.69  1998/06/09 09:46:14  cg
 * added command line options -mt-static, -mt-dynamic, and -maxsyncfold.
 *
 * Revision 1.68  1998/05/15 15:44:31  srs
 * added -maxoptcycles
 *
 * Revision 1.67  1998/05/15 13:48:22  dkr
 * added flag -bn:conv
 *
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
 *
 * Revision 1.2  1994/11/10  15:44:34  sbs
 * RCS-header inserted
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "usage.h"
#include "globals.h"
#include "build.h"

#include "dbug.h"

/*
 * This variable contains the build information of sac2c, i.e. build date
 * and time as well as user and host data.
 * The corresponding variable definition is generated by the make
 * utility each time sac2c is recompiled using a temporary file build.c
 * which then is compiled to build.o and removed afterwards.
 */

void
usage ()
{
    DBUG_ENTER ("usage");

    printf ("\n\n\t  sac2c  --  The ultimate SAC compiler\n"
            "\t----------------------------------------\n\n"

            "NAME:     \tsac2c\n"

            "\n\nDESCRIPTION:\n\n"

            "\tThe sac2c compiler transforms SAC source code into executable programs\n"
            "\t(SAC programs) or into a SAC specific library format (SAC module and\n"
            "\tclass implementations), respectively.\n"
            "\t\n"
            "\tThe compilation process is performed in 4 separate stages:\n"
            "\t1. sac2c uses any C preprocessor to preprocess the given SAC source;\n"
            "\t2. sac2c itself transforms preprocessed SAC source code into C code;\n"
            "\t3. sac2c uses any C compiler to generate target machine code;\n"
            "\t4. sac2c uses any C linker to create an executable program\n"
            "\t   or sac2c itself creates a SAC library file.\n"
            "\t\n"
            "\tWhen compiling a SAC program, sac2c stores the corresponding\n"
            "\tintermediate C code either in the file a.out.c in the current directory\n"
            "\t(default) or in the file <file>.c if <file> is specified using the -o\n"
            "\toption. Here, any absolute or relative path name may be used.\n"
            "\tThe executable program is either written to the file a.out or to any\n"
            "\tfile specified using the -o option.\n"
            "\t\n"
            "\tHowever, when compiling a SAC module/class implementation, the\n"
            "\tresulting SAC library is stored in the file <mod/class name>.lib in the\n"
            "\tcurrent directory. In this case, the -o option may be used to specify a\n"
            "\tdifferent directory but not a different file name.\n"

            "\n\nSPECIAL OPTIONS:\n\n"

            "\t -h\t\tdisplay this helptext.\n"
            "\t -help\t\tdisplay this helptext.\n"
            "\t -copyright\tdisplay copyright/disclaimer.\n"
            "\t -V\t\tdisplay version identification.\n"
            "\n"
            "\t -libstat \tprint status information of the given SAC library file.\n"
            "\t\t\t  This option requires the environment variables PWD,\n"
            "\t\t\t  USER, and HOST to be set when compiling the module/\n"
            "\t\t\t  class implementation in order to work correctly.\n"
            "\t -M\t\tdetect dependencies from imported modules/classes and\n"
            "\t\t\t  write them to stdout in a way suitable for the make\n"
            "\t\t\t  utility. Only dependencies from declaration files are\n"
            "\t\t\t  considered.\n"
            "\t -Mlib\t\tdetect dependencies from imported modules/classes and\n"
            "\t\t\t  write them to stdout in a way suitable for the make\n"
            "\t\t\t  utility. Dependencies from declaration files as well\n"
            "\t\t\t  as library files are (recursively) considered.\n"
            "\n"
            "\tWhen called with one of these options, sac2c does not perform\n"
            "\tany compilation steps.\n"

            "\n\nGENERAL OPTIONS:\n\n"

            "\t -D <cpp-var>[=<value>]?\n"
            "\t\t\tset <cpp-var> (to <value>) when \n"
            "\t\t\t  running C preprocessor\n"
            "\t -I <path>\tspecify additional declaration path\n"
            "\t -L <path>\tspecify additional library path\n"
            "\t -o <name>\tfor compilation of programs:\n"
            "\t\t\t  write executable to specified file\n"
            "\t\t\tfor compilation of module/class implementations:\n"
            "\t\t\t  write library to specified directory\n"
            "\t -c \t\tgenerate C-file only\n"
            "\t -v <n> \tverbose level\n"
            "\t\t\t  0: error messages only\n"
            "\t\t\t  1: error messages and warnings\n"
            "\t\t\t  2: basic compile time information\n"
            "\t\t\t  3: full compile time information\n"
            "\t\t\t  default: -v %d\n",
            verbose_level);

    printf ("\n\nBREAK OPTIONS:\n\n");

    printf ("\t -b1\tstop after: %s\n", compiler_phase_name[1]);
    printf ("\t -b2\tstop after: %s\n", compiler_phase_name[2]);
    printf ("\t -b3\tstop after: %s\n", compiler_phase_name[3]);
    printf ("\t -b4\tstop after: %s\n", compiler_phase_name[4]);
    printf ("\t -b5\tstop after: %s\n", compiler_phase_name[5]);
    printf ("\t -b6\tstop after: %s\n", compiler_phase_name[6]);
    printf ("\t -b7\tstop after: %s\n", compiler_phase_name[7]);
    printf ("\t -b8\tstop after: %s\n", compiler_phase_name[8]);
    printf ("\t -b9\tstop after: %s\n", compiler_phase_name[9]);
    printf ("\t -b10\tstop after: %s\n", compiler_phase_name[10]);
    printf ("\t -b11\tstop after: %s\n", compiler_phase_name[11]);
    printf ("\t -b12\tstop after: %s\n", compiler_phase_name[12]);
    printf ("\t -b13\tstop after: %s\n", compiler_phase_name[13]);
    printf ("\t -b14\tstop after: %s\n", compiler_phase_name[14]);
    printf ("\t -b15\tstop after: %s\n", compiler_phase_name[15]);
    printf ("\t -b16\tstop after: %s\n", compiler_phase_name[16]);
    printf ("\t -b17\tstop after: %s\n", compiler_phase_name[17]);
    printf ("\t -b18\tstop after: %s\n", compiler_phase_name[18]);
    printf ("\t -b19\tstop after: %s\n", compiler_phase_name[19]);
    printf ("\t -b20\tstop after: %s\n", compiler_phase_name[20]);
    printf ("\t -b21\tstop after: %s\n", compiler_phase_name[21]);

    printf ("\n\nBREAK SPECIFIERS:\n\n"
            "\tBreak specifiers allow you to stop the compilation process\n"
            "\twithin a particular phase.\n\n"
            "\tCurrently supported:\n\n"

            "\t-b15:inl       \tstop after function inlining\n"
            "\t-b15:dfr       \tstop after dead function removal\n"
            "\t-b15:ae        \tstop after array elimination\n"
            "\t-b15:dcr       \tstop after dead code removal\n"
            "\t-b15:cycN:cse  \tstop after common subexpression elimination in cycle N\n"
            "\t-b15:cycN:cf   \tstop after constant folding in cycle N\n"
            "\t-b15:cycN:wlt  \tstop after with-loop transformation in cycle N\n"
            "\t-b15:cycN:wli  \tstop after with-loop information gathering in cycle N\n"
            "\t-b15:cycN:wlf  \tstop after with-loop folding in cycle N\n"
            "\t-b15:cycN:cf2  \tstop after second constant folding in cycle N\n"
            "\t-b15:cycN:dcr  \tstop after dead code removal in cycle N\n"
            "\t-b15:cycN:lur  \tstop after (with-)loop unrolling in cycle N\n"
            "\t-b15:cycN:lus  \tstop after loop unswitching in cycle N\n"
            "\t-b15:cycN:lir  \tstop after loop invariant removal in cycle N\n"
            "\n"
            "\t-b18:conv      \tstop after converting\n"
            "\t-b18:cubes     \tstop after cube-building\n"
            "\t-b18:segs      \tstop after choice of segments\n"
            "\t-b18:split     \tstop after splitting\n"
            "\t-b18:block     \tstop after hierarchical blocking\n"
            "\t-b18:ublock    \tstop after unrolling-blocking\n"
            "\t-b18:merge     \tstop after merging\n"
            "\t-b18:opt       \tstop after optimization\n"
            "\t-b18:fit       \tstop after fitting\n"
            "\t-b18:norm      \tstop after normalization\n"
            "\n"
            "\t-b19:spmdinit  \tstop after building SPMD blocks\n"
            "\t-b19:spmdopt   \tstop after optimizing SPMD blocks\n"
            "\t-b19:spmdlift  \tstop after lifting SPMD blocks\n"
            "\t-b19:syncinit  \tstop after building SYNC blocks\n"
            "\t-b19:syncopt   \tstop after optimizing SYNC blocks\n"
            "\t-b19:scheduling\tstop after scheduling SYNC blocks and with-loop segments"
            "\t-b19:spmdcons  \tstop after constrainig SPMD blocks\n"
            "\t               \t(same as -b19 only)");

    printf ("\n\nOPTIMIZATION OPTIONS:\n\n"

            "\t -no <opt>\tdisable optimization technique <opt>\n"
            "\t -do <opt>\tenable optimization technique <opt>\n"
            "\n"
            "\t The following optimization techniques are currently supported:\n\n"
            "\t\tCF  \t constant folding \n"
            "\t\tINL \t function inlining \n"
            "\t\tLUR \t loop unrolling \n"
            "\t\tWLUR\t with-loop unrolling \n"
            "\t\tLUS \t loop unswitching \n"
            "\t\tDCR \t dead code removal \n"
            "\t\tDFR \t dead function removal \n"
            "\t\tLIR \t loop invariant removal \n"
            "\t\tCSE \t common subexpression elimination \n"
            "\t\tWLT \t with-loop transformation \n"
            "\t\tWLF \t with-loop folding \n"
            "\t\tDLAW\t application of the distributive law\n"
            "\t\tIVE \t index vector elimination \n"
            "\t\tAE  \t array elimination \n"
            "\t\tRCO \t refcount optimization \n"
            "\t\tUIP \t update-in-place \n"
            "\t\tTSI \t tile size inference (blocking) \n"
            "\t\tTSP \t tile size pragmas (blocking) \n"
            "\t\tMTO \t multi-thread optimization \n"
            "\t\tSBE \t syncronisation barrier elimination \n"
            "\t\tPHM \t private heap management \n"
            "\n"
            "\t\tOPT  \t enables/disables all optimizations at once.\n"
            "\n"
            "\tLower case letters may be used to indicate optimization techniques.\n"
            "\n"
            "\tCommand line arguments are evaluated from left to right, i.e.,\n"
            "\t\"-no OPT -do INL\" disables all optimizations except for \n"
            "\tfunction inlining.\n\n");

    printf ("\tOptimization side conditions:\n\n");

    printf ("\t -maxoptcyc <no>    \trepeat optimization phase <no> times.\n"
            "\t\t\t\t  Default: -maxoptcyc %d\n",
            max_optcycles);
    printf ("\t -maxoptvar <no>    \treserve <no> variables for optimization.\n"
            "\t\t\t\t  Default: -maxoptvar %d\n",
            optvar);
    printf ("\t -maxinl <no>       \tinline recursive functions <no> times.\n"
            "\t\t\t\t  Default: -maxinl %d\n",
            inlnum);
    printf ("\t -maxlur <no>       \tunroll loops having no more than <no>\n"
            "\t\t\t\titerations.\n"
            "\t\t\t\t  Default: -maxlur %d\n",
            unrnum);
    printf ("\t -maxwlur <no>      \tunroll with-loops having no more than <no>\n"
            "\t\t\t\telements.\n"
            "\t\t\t\t  Default: -maxwlur %d\n",
            wlunrnum);
    printf ("\t -maxae <no>        \ttry array elimination for arrays with length\n"
            "\t\t\t\tless than or equal <no>.\n"
            "\t\t\t\t  Default: -maxae %d\n",
            minarray);
    printf ("\t -maxspecialize <no>\tfunctions with unknown shape will at most\n"
            "\t\t\t\t<no> times be specialized.\n"
            "\t\t\t\t  Default: -maxspecialize %d\n",
            max_overload);
    printf ("\t -initheap <size>\tat program startup initially request <size> MB\n"
            "\t\t\t\tof heap memory from operating system.\n"
            "\t\t\t\t  Default: -heapsize %d\n",
            initial_heapsize);

    printf ("\n\nMULTI-THREAD OPTIONS:\n\n"

            "\t -mt \t\t\tcompile program for multi-threaded execution.\n"
            "\t\t\t\t  The number of threads to be used can either\n"
            "\t\t\t\t  be specified statically using the option\n"
            "\t\t\t\t  \"-numthreads\" or dynamically upon application\n"
            "\t\t\t\t  startup using the generic command line option\n"
            "\t\t\t\t  \"-mt <no>\".\n\n"
            "\t -numthreads <no>\tstatically specify exact number of threads\n"
            "\t\t\t\tto be used.\n"
            "\t -maxthreads <no>\tmaximum number of threads to be used when exact\n"
            "\t\t\t\tnumber is specified dynamically.\n"
            "\t\t\t\t  Default: -maxthreads %d.\n"
            "\t -maxsyncfold <no>\tmaximum number of fold with-loops in a single\n"
            "\t\t\t\tsynchronisation block.\n"
            "\t\t\t\t  -1: maximum of needed (mechanical infered)\n"
            "\t\t\t\t   0: no fold-with-loops are allowed\n"
            "\t\t\t\t      (implies fold-with-loop will not be executed concurrently)\n"
            "\t\t\t\t  >0: number is limited by value of maxsyncfold\n"
            "\t\t\t\t  Default: -maxsyncfold %d.\n"
            "\t -minmtsize <no>\tminimum generator size for parallel execution\n"
            "\t\t\t\tof with-loops.\n"
            "\t\t\t\t  Default: -minmtsize %d.\n",
            max_threads, max_sync_fold, min_parallel_size);

    printf (

      "\n\nDEBUG OPTIONS:\n\n"

      "\t -d efence\t\tfor compilation of programs:\n"
      "\t\t\t\t  link executable with ElectricFence\n"
      "\t\t\t\t  (malloc debugger).\n"
      "\t -d nocleanup\t\tdon't remove temporary files and directories.\n"
      "\t -d syscall\t\tshow all system calls during compilation.\n"
      "\t -d cccall\t\tgenerate shell script '.sac2c' that contains C\n"
      "\t\t\t\tcompiler call.\n"
      "\t\t\t\tThis implies option \"-d nocleanup\".\n"
      "\n\t -# <str>\t\toptions (string) for DBUG information\n"
      "\t -# <from>/<to>/<str>\tDBUG information only in compiler phases\n"
      "\t\t\t\t<from>..<to>\n"
      "\t\t\t\t  Default: <from> = first compiler phase,\n"
      "\t\t\t\t           <to>   = last compiler phase\n"

      "\n\nRUNTIME CHECK OPTIONS:\n\n"

      "\t -check [abme]+ \tinclude runtime checks into executable program.\n"
      "\t\t\t\t  a: include all checks available.\n"
      "\t\t\t\t  b: check array accesses for boundary\n"
      "\t\t\t\t     violations.\n"
      "\t\t\t\t  m: check success of memory allocations.\n"
      "\t\t\t\t  e: check errno variable upon applications of\n"
      "\t\t\t\t     external functions.\n"
      "\t\t\t\t  h: use diagnostic heap manager.\n"

      "\n\nRUNTIME TRACE OPTIONS:\n\n"

      "\t -trace [amrfpowt]+ \tinclude runtime program tracing.\n"
      "\t\t\t\t  a: trace all (same as mrfpowt).\n"
      "\t\t\t\t  m: trace memory operations.\n"
      "\t\t\t\t  r: trace reference counting operations.\n"
      "\t\t\t\t  f: trace user-defined function calls.\n"
      "\t\t\t\t  p: trace primitive function calls.\n"
      "\t\t\t\t  o: trace old with-loop execution.\n"
      "\t\t\t\t  w: trace new with-loop execution.\n"
      "\t\t\t\t  t: trace multi-threading specific operations.\n"

      "\n\nRUNTIME PROFILING OPTIONS:\n\n"

      "\t -profile [afilw]+ \tinclude runtime profiling analysis.\n"
      "\t\t\t\t  a: analyse all (same as filw).\n"
      "\t\t\t\t  f: analyse time spent in non-inline functions.\n"
      "\t\t\t\t  i: analyse time spent in inline functions.\n"
      "\t\t\t\t  l: analyse time spent in library functions.\n"
      "\t\t\t\t  w: analyse time spent in with-loops.\n"

      "\n\nCACHE SIMULATION OPTIONS:\n\n"

      "\t -cs\t\tenable runtime cache simulation\n"
      "\n"
      "\t -csdefaults [sagbifp]+ \n\n"
      "\t\tThis option sets default parameters for cache simulation.\n"
      "\t\tThese settings may be overridden when starting the analysis of\n"
      "\t\tan application program.\n"
      "\t\t  s: simple cache simulation.\n"
      "\t\t  a: advanced cache simulation.\n"
      "\t\t  g: global cache simulation.\n"
      "\t\t  b: cache simulation on selected blocks.\n"
      "\t\t  i: immediate analysis of memory accesses.\n"
      "\t\t  f: storage of memory accesses in file.\n"
      "\t\t  p: piping of memory accesses to concurrently running analyser.\n"
      "\n"
      "\t\tThe default simulation parameters are 'sgp'.\n"
      "\n"
      "\tSimple cache simulation only counts cache hits and cache misses while\n"
      "\tadvanced cache simulation additionally classifies cache misses into\n"
      "\tcold start, cross interference, self interference, and invalidation\n"
      "\tmisses.\n"
      "\n"
      "\tSimulation results may be presented for the entire program run or more\n"
      "\tspecifically for any code block marked by the following pragma:\n"
      "\t\t#pragma cachesim [tag]\n"
      "\tThe optional tag allows to distinguish between the simulation results\n"
      "\tfor various code blocks. The tag must be a string.\n"
      "\n"
      "\tMemory accesses may be evaluated with respect to their cache behavior\n"
      "\teither immediately within the application process, stored in a file,\n"
      "\tor they may be piped to a concurrently running analyser process.\n"
      "\tWhile immediate analysis usually is the fastest alternative,\n"
      "\tresults, in particular for advanced analysis, are often inaccurate due to\n"
      "\tchanges in the memory layout caused by the analyser. If you choose to\n"
      "\twrite memory accesses to a file, beware that even for small programs to\n"
      "\tbe analysed the amount of data may be quite large. However, once a\n"
      "\tmemory trace file exists, it can be used to simulate different cache\n"
      "\tconfigurations without repeatedly running the application program\n"
      "\titself. The simulation tool for memory access trace files is called\n"
      "\t\tCacheSimAnalyser\n"
      "\tand may be found in the directory\n"
      "\t\t%s%sruntime\n"
      "\tas part of your SAC %s installation.\n"
      "\n"
      "\tThese default cache simulation parameters may be overridden when\n"
      "\tinvoking the application program to be analysed by using the generic\n"
      "\tcommand line option\n"
      "\t\t-cs [sagbifp]+\n"
      "\n"
      "\tCache parameters for up to 3 levels of caches may be provided as target\n"
      "\tspecification in the sac2crc file. However, these only serve as a\n"
      "\tdefault cache specification which may well be altered when running the\n"
      "\tcompiled SAC program with cache simulation enabled. This can be done\n"
      "\tusing the following command line options:\n"
      "\t\t-cs[123] <size>[/<line size>[/<assoc>[/<write miss policy>]]].\n"
      "\tThe cache size must be given in KBytes, the cache line size in\n"
      "\tBytes. A cache size of 0 KB disables the corresponding cache level\n"
      "\tcompletely regardless of any other setting.\n"
      "\tWrite miss policies are specified by a single letter:\n"
      "\t\td: default (fetch on write)\n"
      "\t\tf: fetch on write\n"
      "\t\tv: write validate\n"
      "\t\ta: write around\n"
      "\n"
      "\t-cshost <name> \tallows the specification of a specific host to\n"
      "\t\t\trun the additional analyser process on when doing piped cache\n"
      "\t\t\tsimulation. This is very useful for single processor machines\n"
      "\t\t\tbecause the rather limited buffer size of the pipe determines\n"
      "\t\t\tthe synchronisation distance of the two processes, i.e. the\n"
      "\t\t\tapplication process and the analysis process. This results in\n"
      "\t\t\tvery frequent context switches when both processes are run on the\n"
      "\t\t\tsame processor, and consequently, degrades the performance by\n"
      "\t\t\torders of magnitude. So, when doing piped cache simulation always\n"
      "\t\t\tbe sure to do so either on a multiprocessor or specify a different\n"
      "\t\t\tmachine to run the analyser process on.\n"
      "\t\t\tHowever, this only defines a default which may be overridden\n"
      "\t\t\tby using this option when starting the compiled application\n"
      "\t\t\tprogram.\n"
      "\n"
      "\t-csfile <name> \tallows the specification of a default file where to\n"
      "\t\t\twrite the memory access trace when performing cache simulation\n"
      "\t\t\tvia a file. This default may be overridden by using this option\n"
      "\t\t\twhen starting the compiled application program.\n"
      "\t\t\tThe general default name is <exec file name>.cs.\n"
      "\n"
      "\t-csdir <name> \tallows the specification of a default directory where to\n"
      "\t\t\twrite the memory access trace file when performing cache simulation\n"
      "\t\t\tvia a file. This default may be overridden by using this option\n"
      "\t\t\twhen starting the compiled application program.\n"
      "\t\t\tThe general default directory is the tmp directory specified in\n"
      "\t\t\tyour sac2crc file.\n"

      "\n\nINTRINSIC ARRAY OPERATIONS OPTIONS:\n\n"

      "\t -intrinsic [a+-x/tdcrpo]+ \tuse intrinsic array operations.\n"
      "\t\t\t\t\t  a: use all intrinsic operations\n"
      "\t\t\t\t\t     available (same as +-x/tdcrpo).\n"
      "\t\t\t\t\t  +: use intrinsic add.\n"
      "\t\t\t\t\t  -: use intrinsic sub.\n"
      "\t\t\t\t\t  x: use intrinsic mul.\n"
      "\t\t\t\t\t  /: use intrinsic div.\n"
      "\t\t\t\t\t  t: use intrinsic take.\n"
      "\t\t\t\t\t  d: use intrinsic drop.\n"
      "\t\t\t\t\t  c: use intrinsic cat.\n"
      "\t\t\t\t\t  r: use intrinsic rotate.\n"
      "\t\t\t\t\t  p: use intrinsic psi.\n"
      "\t\t\t\t\t  o: use intrinsic type conversion.\n"

      "\n\nLINK OPTIONS:\n\n"

      "\t -l <n>\t\tlink level for generating SAC library.\n"
      "\t\t\t  1: compile to one large object file.\n"
      "\t\t\t  2: compile to archive of object files.\n"
      "\t\t\t  Default: -l %d\n"

      "\n\nC-COMPILER OPTIONS:\n\n"

      "\t -g     \tinclude debug information into object code.\n"
      "\n"
      "\t -O <n> \tC compiler level of optimization.\n"
      "\t\t\t  0: no C compiler optimizations.\n"
      "\t\t\t  1: minor C compiler optimizations.\n"
      "\t\t\t  2: medium C compiler optimizations.\n"
      "\t\t\t  3: full C compiler optimizations.\n"
      "\t\t\t  Default: -O %d\n"
      "\n"
      "\tThe actual effects of these options are C compiler specific!\n"

      "\n\nCUSTOMIZATION\n\n"

      "\t-target <name>\tspecify a particular compilation target.\n"
      "\t\t\tCompilation targets are used to customize sac2c for\n"
      "\t\t\tvarious target architectures, operating systems, and C\n"
      "\t\t\tcompilers.\n"
      "\t\t\tThe target description is either read from the\n"
      "\t\t\tinstallation specific file $SACBASE/runtime/sac2crc or\n"
      "\t\t\tfrom a file named .sac2crc within the user's home\n"
      "\t\t\tdirectory.\n"

      "\n\nENVIRONMENT VARIABLES:\n\n"

      "\tSACBASE\t\t\tbase directory of SAC installation\n"
      "\tSAC_PATH\t\tsearch paths for program source\n"
      "\tSAC_DEC_PATH\t\tsearch paths for declarations\n"
      "\tSAC_LIBRARY_PATH\tsearch paths for libraries\n"
      "\n"
      "\tThe following environment variables must be set correctly when compiling\n"
      "\ta SAC module/class implementation in order to enable full usability of\n"
      "\tsac2c command line option \"-libstat\": PWD, USER, and HOST.\n"

      "\n\nAUTHORS:\n\n"

      "\tSven-Bodo Scholz\n"
      "\tHenning Wolf\n"
      "\tArne Sievers\n"
      "\tClemens Grelck\n"
      "\tDietmar Kreye\n"
      "\tSoeren Schwartz\n"
      "\tBjoern Schierau\n"
      "\tHelge Ernst\n"
      "\tJan-Hendrik Schoeler\n"

      "\n\nCONTACT:\n\n"

      "\tWorld Wide Web: http://www.informatik.uni-kiel.de/~sacbase/\n"
      "\tE-Mail: sacbase@informatik.uni-kiel.de\n"

      "\n\nBUGS:\n\n"

      "\tBugs ??  We ????\n"
      "\n"
      "\tDo not annotate functions \"inline\" which contain fold-with-loops!\n"
      "\tIt leads to the creation of C-code which does not compile properly!\n"
      "\n"
      "\tUnfortunately, two of our optimizations are quite buggy 8-(\n"
      "\tTherefore, we decided to preset -noLIR and -noDLAW in the current\n"
      "\tcompiler release!\n"

      "\n",
      NULL == getenv ("SACBASE") ? "" : getenv ("SACBASE"),
      (NULL != getenv ("SACBASE")
       && getenv ("SACBASE")[strlen (getenv ("SACBASE")) - 1] != '/')
        ? "/"
        : "",
      version_id, linkstyle, cc_optimize);

    DBUG_VOID_RETURN;
}

void
version ()
{
    DBUG_ENTER ("version");

    printf ("\n\t\tSAC - Single Assignment C\n"
            "\t--------------------------------------------\n"
            "\n"
            "NAME:      sac2c\n"
            "VERSION:   %s\n"
            "PLATFORM:  %s\n\n"
            "BUILD:     %s\n"
            "BY USER:   %s\n"
            "ON HOST:   %s\n"
            "\n"
            "\n"

            "(c) Copyright 1994 - 1999 by\n\n"

            "  Christian-Albrechts-Universitaet zu Kiel\n"
            "  Institut fuer Informatik und Praktische Mathematik\n"
            "  Preusserstrasse 1 - 9\n"
            "  D-24105 Kiel\n"
            "  Germany\n\n",
            version_id[0] == '\0' ? "???" : version_id,
            target_platform[0] == '\0' ? "???" : target_platform,
            build_date[0] == '\0' ? "???" : build_date,
            build_user[0] == '\0' ? "???" : build_user,
            build_host[0] == '\0' ? "???" : build_host);

    DBUG_VOID_RETURN;
}

void
copyright ()
{
    DBUG_ENTER ("copyright");

    printf ("\n\t\tSAC - Single Assignment C\n"
            "\t--------------------------------------------\n"
            "\n"

            "\tSAC COPYRIGHT NOTICE, LICENSE, AND DISCLAIMER\n\n"
            "(c) Copyright 1994 - 1999 by\n\n"

            "  Christian-Albrechts-Universitaet zu Kiel\n"
            "  Institut fuer Informatik und Praktische Mathematik\n"
            "  Preusserstrasse 1 - 9\n"
            "  D-24105 Kiel\n"
            "  Germany\n\n");

    printf ("The SAC compiler, the SAC standard library, and all accompanying\n"
            "software and documentation (in the following named this software)\n"
            "is developed by the SAC group as part of the Chair of Computer \n"
            "Organization within the Department of Computer Science and Applied\n"
            "Mathematics of the University of Kiel (in the following named CAU Kiel)\n"
            "which reserves all rights on this software.\n"
            " \n"
            "Permission to use this software is hereby granted free of charge\n"
            "for any non-profit purpose in a non-commercial environment, i.e. for\n"
            "educational or research purposes in a non-profit institute or for\n"
            "personal, non-commercial use. For this kind of use it is allowed to\n"
            "copy or redistribute this software under the condition that the \n"
            "complete distribution for a certain platform is copied or \n"
            "redistributed and this copyright notice, license agreement, and \n"
            "warranty disclaimer appears in each copy. ANY use of this software with \n"
            "a commercial purpose or in a commercial environment is not granted by \n"
            "this license. \n"
            "\n"
            "CAU Kiel disclaims all warranties with regard to this software, including \n"
            "all implied warranties of merchantability and fitness.  In no event\n"
            "shall CAU Kiel be liable for any special, indirect or consequential\n"
            "damages or any damages whatsoever resulting from loss of use, data, or\n"
            "profits, whether in an action of contract, negligence, or other\n"
            "tortuous action, arising out of or in connection with the use or\n"
            "performance of this software. The entire risk as to the quality and\n"
            "performance of this software is with you. Should this software prove\n"
            "defective, you assume the cost of all servicing, repair, or correction.\n"
            " \n");

#if 0
  printf("Permission to use, copy, modify, and distribute this software and its\n"
         "documentation for any purpose and without fee is hereby granted,\n"
         "provided that the above copyright notice appear in all copies and that\n"
         "both the copyright notice and this permission notice and warranty\n"
         "disclaimer appear in supporting documentation, and that the name of\n"
         "CAU Kiel or any CAU Kiel entity not be used in advertising\n"
         "or publicity pertaining to distribution of the software without\n"
         "specific, written prior permission.\n\n"

         "CAU Kiel disclaims all warranties with regard to this software, including\n"
         "all implied warranties of merchantability and fitness.  In no event\n"
         "shall CAU Kiel be liable for any special, indirect or consequential\n"
         "damages or any damages whatsoever resulting from loss of use, data or\n"
         "profits, whether in an action of contract, negligence or other\n"
         "tortious action, arising out of or in connection with the use or\n"
         "performance of this software.\n\n");
#endif

    DBUG_VOID_RETURN;
}
