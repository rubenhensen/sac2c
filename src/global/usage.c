/*
 *
 * $Log$
 * Revision 3.18  2002/03/13 16:03:20  ktr
 * Help information for Withloop-Scalarization added
 *
 * Revision 3.17  2002/01/18 11:39:02  sbs
 * updated the copyright notice.
 *
 * Revision 3.16  2001/06/28 07:46:51  cg
 * Primitive function psi() renamed to sel().
 *
 * Revision 3.15  2001/05/30 15:50:25  nmw
 * break specifier for cf3 added
 *
 * Revision 3.14  2001/05/25 09:21:56  nmw
 * ssa form related break specifier in optimizations added
 *
 * Revision 3.13  2001/05/07 15:13:28  dkr
 * minor changes done
 *
 * Revision 3.12  2001/05/07 14:21:24  dkr
 * all output lines contain <= 80 characters now
 *
 * Revision 3.11  2001/04/26 17:10:55  dkr
 * PRINT_BREAK_SPEC added
 *
 * Revision 3.10  2001/04/24 09:39:35  dkr
 * CHECK_NULL renamed into STR_OR_EMPTY
 *
 * Revision 3.9  2001/03/28 14:50:02  dkr
 * CHECK_NULL used
 *
 * Revision 3.8  2001/02/09 14:40:09  nmw
 * ssa switch documentation added
 *
 * Revision 3.7  2001/02/05 15:55:42  dkr
 * break specifier for -b16 updated
 *
 * Revision 3.6  2001/01/25 10:17:29  dkr
 * -b21 added
 *
 * Revision 3.5  2001/01/08 12:03:52  dkr
 * compiler phases renumbered
 *
 * Revision 3.3  2000/11/27 21:04:38  cg
 * Added general support for new optimization APL,
 * "array placement"
 *
 * Revision 3.2  2000/11/24 16:31:06  nmw
 * trace option -trace c added
 *
 * Revision 3.1  2000/11/20 17:59:41  sacbase
 * new release made
 *
 * Revision 2.43  2000/11/17 16:21:06  sbs
 * -MM and -MMlib added.
 *
 * Revision 2.42  2000/11/14 16:47:47  sbs
 * string.h included which is needed by strlen
 *
 * Revision 2.41  2000/10/31 18:06:27  cg
 * Added additional break specifier -b15:dfr2.
 *
 * Revision 2.40  2000/10/27 13:23:13  cg
 * Added new command line options -aplimit and -apdiaglimit.
 *
 * Revision 2.39  2000/08/04 13:26:10  mab
 * added description for flag -apdiag
 *
 * Revision 2.38  2000/08/04 09:54:30  nmw
 * added hint for additional docu of the cinterface
 *
 * Revision 2.37  2000/08/02 11:13:44  nmw
 * genlib c comment for profiling and MT changed
 *
 * Revision 2.36  2000/08/01 13:44:40  nmw
 * comment to -genlib c switch adjusted
 *
 * Revision 2.35  2000/07/11 16:13:58  dkr
 * psi-opt phase removed
 *
 * Revision 2.34  2000/06/14 10:56:57  jhs
 * Added information about "out-of-order" phase 17 while doing -mtn
 *
 * [...]
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "build.h"
#include "internal_lib.h"
#include "dbug.h"
#include "usage.h"

#define PRINT_BREAK_SPEC(ph, spec, comment)                                              \
    {                                                                                    \
        int _i;                                                                          \
        printf ("\t -b%i:%s", ph, spec);                                                 \
        for (_i = 0; _i < (13 - strlen (spec)); _i++) {                                  \
            printf (" ");                                                                \
        }                                                                                \
        printf ("%s\n", comment);                                                        \
    }

void
usage ()
{
    int ph;
    char *env;

    DBUG_ENTER ("usage");

    printf ("\n\n\t  sac2c  --  The ultimate SAC compiler\n"
            "\t----------------------------------------\n\n"

            "NAME:     \tsac2c\n\n\n"

            "DESCRIPTION:\n\n"

            "\tThe sac2c compiler transforms SAC source code into executable programs\n"
            "\t(SAC programs) or into a SAC specific library format (SAC module and\n"
            "\tclass implementations), respectively.\n"
            "\t\n"
            "\tThe compilation process is performed in 4 separate stages:\n"
            "\t  1. sac2c uses any C preprocessor to preprocess the given SAC source;\n"
            "\t  2. sac2c itself transforms preprocessed SAC source code into C code;\n"
            "\t  3. sac2c uses any C compiler to generate target machine code;\n"
            "\t  4. sac2c uses any C linker to create an executable program\n"
            "\t     or sac2c itself creates a SAC library file.\n"
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
            "\tdifferent directory but not a different file name.\n");

    printf ("\n\nSPECIAL OPTIONS:\n\n"

            "\t -h\t\tdisplay this helptext.\n"
            "\t -help\t\tdisplay this helptext.\n"
            "\t -copyright\tdisplay copyright/disclaimer.\n"
            "\t -V\t\tdisplay version identification.\n"
            "\n"
            "\t -libstat \tprint status information of the given SAC library file.\n"
            "\t\t\tThis option requires the environment variables PWD,\n"
            "\t\t\tUSER, and HOST to be set when compiling the module/\n"
            "\t\t\tclass implementation in order to work correctly.\n"
            "\n"
            "\t -M\t\tdetect dependencies from imported modules/classes and\n"
            "\t\t\twrite them to stdout in a way suitable for the make\n"
            "\t\t\tutility. Only dependencies from declaration files are\n"
            "\t\t\tconsidered.\n"
            "\n"
            "\t -MM\t\tlike `-M' but the output mentions only non standard lib\n"
            "\t\t\tdependencies.\n"
            "\n"
            "\t -Mlib\t\tdetect dependencies from imported modules/classes and\n"
            "\t\t\twrite them to stdout in a way suitable for the make\n"
            "\t\t\tutility. Dependencies from declaration files as well\n"
            "\t\t\tas library files are (recursively) considered.\n"
            "\n"
            "\t -MMlib\t\tlike `-Mlib' but the output mentions only non standard\n"
            "\t\t\tlib dependencies.\n"
            "\n"
            "\tWhen called with one of these options, sac2c does not perform\n"
            "\tany compilation steps.\n");

    printf ("\n\nGENERAL OPTIONS:\n\n"

            "\t -D <cpp-var>[=<value>]?\n"
            "\t\t\tset <cpp-var> (to <value>) when running C preprocessor\n"
            "\n"
            "\t -I <path>\tspecify additional declaration path\n"
            "\t -L <path>\tspecify additional library path\n"
            "\n"
            "\t -o <name>\tfor compilation of programs:\n"
            "\t\t\t  write executable to specified file\n"
            "\t\t\tfor compilation of module/class implementations:\n"
            "\t\t\t  write library to specified directory\n"
            "\n"
            "\t -c \t\tgenerate C-file only\n"
            "\n"
            "\t -v <n> \tverbose level\n"
            "\t\t\t  0: error messages only\n"
            "\t\t\t  1: error messages and warnings\n"
            "\t\t\t  2: basic compile time information\n"
            "\t\t\t  3: full compile time information\n"
            "\t\t\tdefault: -v %d\n",
            verbose_level);

    printf ("\n\nBREAK OPTIONS:\n\n"

            "\tBreak options allow you to stop the compilation process\n"
            "\tafter a particular phase.\n"
            "\tPer default the programm will then be printed out, but\n"
            "\t\t-noPAB\tdeactivates print\n"
            "\t\t-doPAB\tactivates print\n\n");

    for (ph = 1; ph <= PH_genccode; ph++) {
        printf ("\t -b%i\tstop after: %s\n", ph, compiler_phase_name[ph]);
    }

    printf ("\n\nBREAK SPECIFIERS:\n\n"

            "\tBreak specifiers allow you to stop the compilation process\n"
            "\twithin a particular phase.\n\n"

            "\tCurrently supported:\n\n");

    PRINT_BREAK_SPEC (PH_sacopt, "inl", "stop after function inlining");
    PRINT_BREAK_SPEC (PH_sacopt, "dfr", "stop after initial dead function removal");
    PRINT_BREAK_SPEC (PH_sacopt, "w2d",
                      "stop after transf. of while into do loops (ssa only)");
    PRINT_BREAK_SPEC (PH_sacopt, "l2f",
                      "stop after transf. into fun representation (ssa only)");
    PRINT_BREAK_SPEC (PH_sacopt, "ssa",
                      "stop after initial ssa transformation (ssa only)");
    PRINT_BREAK_SPEC (PH_sacopt, "ae", "stop after array elimination");
    PRINT_BREAK_SPEC (PH_sacopt, "dcr", "stop after dead code removal");

    PRINT_BREAK_SPEC (PH_sacopt, "cyc<N>:cse",
                      "stop after common subexpression elimination ...");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc<N>:cf", "stop after constant folding ...");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc<N>:wlt", "stop after with-loop transformation ...");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc<N>:wli",
                      "stop after with-loop information gathering ...");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc<N>:wlf", "stop after with-loop folding ...");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc<N>:wls", "stop after with-loop scalarization ...");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc<N>:cf2", "stop after second constant folding ...");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc<N>:dcr", "stop after dead code removal ...");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc<N>:lur", "stop after (with-)loop unrolling ...");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc<N>:cf3", "stop after third constant folding ...");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc<N>:lus", "stop after loop unswitching ...");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc<N>:lir",
                      "stop after (with-)loop invariant removal ...");

    printf ("\t                     ... in cycle <N>\n");

    PRINT_BREAK_SPEC (PH_sacopt, "funopt", "stop after fundef optimization cycle");
    PRINT_BREAK_SPEC (PH_sacopt, "ussa", "stop after undo ssa transformation (ssa only)");
    PRINT_BREAK_SPEC (PH_sacopt, "f2l",
                      "stop after transf. into lac representation (ssa only)");
    PRINT_BREAK_SPEC (PH_sacopt, "wlaa", "stop after with loop array access inference");
    PRINT_BREAK_SPEC (PH_sacopt, "ap", "stop after array padding");
    PRINT_BREAK_SPEC (PH_sacopt, "tsi", "stop after tile size inference");
    PRINT_BREAK_SPEC (PH_sacopt, "dfr2", "stop after final dead function removal");
    PRINT_BREAK_SPEC (PH_sacopt, "ive", "stop after index vector elimination");

    printf ("\n");

    PRINT_BREAK_SPEC (PH_wltrans, "conv", "stop after converting");
    PRINT_BREAK_SPEC (PH_wltrans, "cubes", "stop after cube-building");
    PRINT_BREAK_SPEC (PH_wltrans, "fill1", "stop after gap filling (grids only)");
    PRINT_BREAK_SPEC (PH_wltrans, "segs", "stop after choice of segments");
    PRINT_BREAK_SPEC (PH_wltrans, "split", "stop after splitting");
    PRINT_BREAK_SPEC (PH_wltrans, "block", "stop after hierarchical blocking");
    PRINT_BREAK_SPEC (PH_wltrans, "ublock", "stop after unrolling-blocking");
    PRINT_BREAK_SPEC (PH_wltrans, "merge", "stop after merging");
    PRINT_BREAK_SPEC (PH_wltrans, "opt", "stop after optimization");
    PRINT_BREAK_SPEC (PH_wltrans, "fit", "stop after fitting");
    PRINT_BREAK_SPEC (PH_wltrans, "norm", "stop after normalization");
    PRINT_BREAK_SPEC (PH_wltrans, "fill2", "stop after gap filling (all nodes)");

    printf ("\n");
    printf ("\twith -mt\n");

    PRINT_BREAK_SPEC (PH_multithread, "spmdinit", "stop after building SPMD blocks");
    PRINT_BREAK_SPEC (PH_multithread, "spmdopt", "stop after optimizing SPMD blocks");
    PRINT_BREAK_SPEC (PH_multithread, "spmdlift", "stop after lifting SPMD blocks");
    PRINT_BREAK_SPEC (PH_multithread, "syncinit", "stop after building SYNC blocks");
    PRINT_BREAK_SPEC (PH_multithread, "syncopt", "stop after optimizing SYNC blocks");
    PRINT_BREAK_SPEC (PH_multithread, "scheduling",
                      "stop after scheduling SYNC blocks and with-loop");
    printf ("\t                     segments\n");

    PRINT_BREAK_SPEC (PH_multithread, "spmdcons", "stop after constrainig SPMD blocks");

    printf ("\t                     (same as [-mt] -b16 only)\n");
    printf ("\n");
    printf ("\twith -mtn (UNDER CONSTRUCTION!!!)\n");

    PRINT_BREAK_SPEC (PH_multithread, "init", "stop after internal initialization");
    PRINT_BREAK_SPEC (PH_multithread, "schin", "stop after schedulings initialized");
    PRINT_BREAK_SPEC (PH_multithread, "rfin", "stop after replicated functions builded");
    PRINT_BREAK_SPEC (PH_multithread, "blkin", "stop after ST- and MT-blocks builded");
    PRINT_BREAK_SPEC (PH_multithread, "blkpp", "stop after blocks propagated");
    PRINT_BREAK_SPEC (PH_multithread, "blkex", "stop after blocks expanded");
    PRINT_BREAK_SPEC (PH_multithread, "mtfin",
                      "stop after multithread functions builded");
    PRINT_BREAK_SPEC (PH_multithread, "blkco", "stop after blocks consolidated");
    PRINT_BREAK_SPEC (PH_multithread, "dfa", "stop after dataflow-analysis");
    PRINT_BREAK_SPEC (PH_multithread, "barin", "stop after barriers initialized");
    PRINT_BREAK_SPEC (PH_multithread, "blkli", "stop after blocks lifted");
    PRINT_BREAK_SPEC (PH_multithread, "adjca", "stop after adjusted calls");

    printf ("\n\nOPTIMIZATION OPTIONS:\n\n"
            "\t -ssa\t\tuse optimizations based on ssa-form.\n"
            "\t -no <opt>\tdisable optimization technique <opt>\n"
            "\t -do <opt>\tenable optimization technique <opt>\n"
            "\n"
            "\tThe following optimization techniques are currently supported:\n\n"

            "\t\tCF  \tconstant folding\n"
            "\t\tINL \tfunction inlining\n"
            "\t\tLUR \tloop unrolling\n"
            "\t\tWLUR\twith-loop unrolling\n"
            "\t\tLUS \tloop unswitching\n"
            "\t\tDCR \tdead code removal\n"
            "\t\tDFR \tdead function removal\n"
            "\t\tLIR \tloop invariant removal\n"
            "\t\tCSE \tcommon subexpression elimination\n"
            "\t\tWLT \twith-loop transformation\n"
            "\t\tWLF \twith-loop folding\n"
            "\t\tWLS \twith-loop scalarization\n"
            "\t\tDLAW\tapplication of the distributive law\n"
            "\t\tIVE \tindex vector elimination\n"
            "\t\tAE  \tarray elimination\n"
            "\t\tRCO \trefcount optimization\n"
            "\t\tUIP \tupdate-in-place\n"
            "\t\tAP  \tarray padding\n"
            "\t\tAPL \tarray placement\n"
            "\t\tTSI \ttile size inference (blocking)\n"
            "\t\tTSP \ttile size pragmas (blocking)\n"
            "\t\tMTO \tmulti-thread optimization\n"
            "\t\tSBE \tsyncronisation barrier elimination\n"
            "\t\tPHM \tprivate heap management\n"
            "\t\tAPS \tarena preselection           (in conjunction with PHM)\n"
            "\t\tRCAO\trefcount allocation optimiz. (in conjunction with PHM)\n"
            "\t\tMSCA\tmemory size cache adjustment (in conjunction with PHM)\n"
            "\n"
            "\t\tOPT \tenables/disables all optimizations at once.\n"
            "\n"
            "\tLower case letters may be used to indicate optimization techniques.\n"
            "\n"
            "\tCommand line arguments are evaluated from left to right, i.e.,\n"
            "\t\"-no OPT -do INL\" disables all optimizations except for\n"
            "\tfunction inlining.\n\n");

    printf ("\tOptimization side conditions:\n\n"

            "\t -maxoptcyc <no>    \trepeat optimization phase <no> times.\n"
            "\t\t\t\tdefault: -maxoptcyc %d\n\n",
            max_optcycles);

    printf ("\t -maxoptvar <no>    \treserve <no> variables for optimization.\n"
            "\t\t\t\tdefault: -maxoptvar %d\n\n",
            optvar);

    printf ("\t -maxinl <no>       \tinline recursive functions <no> times.\n"
            "\t\t\t\tdefault: -maxinl %d\n\n",
            inlnum);

    printf ("\t -maxlur <no>       \tunroll loops having no more than <no>\n"
            "\t\t\t\t  iterations.\n"
            "\t\t\t\tdefault: -maxlur %d\n\n",
            unrnum);

    printf ("\t -maxwlur <no>      \tunroll with-loops having no more than <no>\n"
            "\t\t\t\t  elements.\n"
            "\t\t\t\tdefault: -maxwlur %d\n\n",
            wlunrnum);

    printf ("\t -maxae <no>        \ttry array elimination for arrays with length\n"
            "\t\t\t\t  less than or equal <no>.\n"
            "\t\t\t\tdefault: -maxae %d\n\n",
            minarray);

    printf ("\t -maxspecialize <no>\tfunctions with unknown shape will at most\n"
            "\t\t\t\t  <no> times be specialized.\n"
            "\t\t\t\tdefault: -maxspecialize %d\n\n",
            max_overload);

    printf ("\t -initmheap <size>\tat program startup initially request <size> KB\n"
            "\t\t\t\t  of heap memory for usage by the master thread.\n"
            "\t\t\t\tdefault: -initmheap %d\n\n",
            initial_master_heapsize);

    printf ("\t -initwheap <size>\tat program startup initially request <size> KB\n"
            "\t\t\t\t  of heap memory for usage by each worker\n"
            "\t\t\t\t  thread.\n"
            "\t\t\t\tdefault: -initwheap %d\n\n",
            initial_worker_heapsize);

    printf ("\t -inituheap <size>\tat program startup initially request <size> KB\n"
            "\t\t\t\t  of heap memory for usage by all threads\n"
            "\t\t\t\t  (unified heap).\n"
            "\t\t\t\tdefault: -inituheap %d\n\n",
            initial_unified_heapsize);

    printf ("\t -aplimit <limit> \tset the array padding resource allocation\n"
            "\t\t\t\t  overhead limit to <limit> %%.\n"
            "\t\t\t\tdefault: -aplimit %d\n\n",
            padding_overhead_limit);

    printf ("\t -apdiag          \tprint additional information for array padding\n"
            "\t\t\t\t  to file 'outfile.ap', where 'outfile' is the\n"
            "\t\t\t\t  name specified with option -o.\n\n");

    printf ("\t -apdiaglimit <n> \tlimits the amount of information written to\n"
            "\t\t\t\t  the diagnostic output file created by the\n"
            "\t\t\t\t  -apdiag option to approximately <n> lines.\n"
            "\t\t\t\tdefault: -apdiaglimit %d\n\n",
            apdiag_limit);

    printf ("\n\nMULTI-THREAD OPTIONS:\n\n"

            "\t -mt \t\t\tcompile program for multi-threaded execution.\n"
            "\t\t\t\tThe number of threads to be used can either\n"
            "\t\t\t\t  be specified statically using the option\n"
            "\t\t\t\t  \"-numthreads\" or dynamically upon application\n"
            "\t\t\t\t  startup using the generic command line option\n"
            "\t\t\t\t  \"-mt <no>\".\n"
            "\n"
            "\t -mtn \t\t\tnew support for multi-threading\n"
            "\t\t\t\t  UNDER CONSTRUCTION!!!\n"
            "\n"
            "\t -numthreads <no>\tstatically specify exact number of threads\n"
            "\t\t\t\t  to be used.\n"
            "\n"
            "\t -maxthreads <no>\tmaximum number of threads to be used when exact\n"
            "\t\t\t\t  number is specified dynamically.\n"
            "\t\t\t\tdefault: -maxthreads %d.\n"
            "\n"
            "\t -maxsyncfold <no>\tmaximum number of fold with-loops in a single\n"
            "\t\t\t\t  synchronisation block.\n"
            "\t\t\t\t  -1: maximum of needed (mechanical infered)\n"
            "\t\t\t\t   0: no fold-with-loops are allowed\n"
            "\t\t\t\t        (implies fold-with-loop will not be\n"
            "\t\t\t\t        executed concurrently)\n"
            "\t\t\t\t  >0: number is limited by value of maxsyncfold\n"
            "\t\t\t\tdefault: -maxsyncfold %d.\n"
            "\n"
            "\t -minmtsize <no>\tminimum generator size for parallel execution\n"
            "\t\t\t\t  of with-loops.\n"
            "\t\t\t\tdefault: -minmtsize %d.\n"
            "\n"
            "\t -maxrepsize <no>\t(-mtn) maximum size for arrays to be replicated\n"
            "\t\t\t\tdefault: -maxrepsize %d.\n",
            max_threads, max_sync_fold, min_parallel_size, max_replication_size);

    printf ("\n\nGENERAL DEBUG OPTIONS:\n\n"

            "\t -d nocleanup\t\tdon't remove temporary files and directories.\n"
            "\t -d syscall\t\tshow all system calls during compilation.\n"
            "\t -d cccall\t\tgenerate shell script \".sac2c\" that contains C\n"
            "\t\t\t\t  compiler call.\n"
            "\t\t\t\t  This implies option \"-d nocleanup\".\n");

    printf ("\n\nINTERNAL DEBUG OPTIONS:\n\n"

            "\t -d efence\t\tfor compilation of programs:\n"
            "\t\t\t\t  link executable with ElectricFence\n"
            "\t\t\t\t  (malloc debugger).\n\n"

            "\t -# <str>\t\toptions (string) for DBUG information\n"
            "\t -# <from>/<to>/<str>\tDBUG information only in compiler phases\n"
            "\t\t\t\t  <from>..<to>\n"
            "\t\t\t\tdefault: <from> = first compiler phase,\n"
            "\t\t\t\t         <to>   = last compiler phase\n\n"

            "\t -lac2fun <ph>[:<ph>]*\ttransformation of loops and conditions into\n"
            "\t\t\t\t  functions before the compiler phases <ph>.\n"
            "\t -fun2lac <ph>[:<ph>]*\ttransformation vice versa after the compiler\n"
            "\t\t\t\t  phases <ph>.\n"
            "\t\t\t\tnote: -b<ph> stops the compiler *after* the\n"
            "\t\t\t\t  lac2fun transformation of phase <ph+1>!\n");

    printf ("\n\nRUNTIME CHECK OPTIONS:\n\n"

            "\t -check [abmeh]+ \tinclude runtime checks into executable program.\n"
            "\t\t\t\t  a: include all checks available.\n"
            "\t\t\t\t  b: check array accesses for boundary\n"
            "\t\t\t\t     violations.\n"
            "\t\t\t\t  m: check success of memory allocations.\n"
            "\t\t\t\t  e: check errno variable upon applications of\n"
            "\t\t\t\t     external functions.\n"
            "\t\t\t\t  h: use diagnostic heap manager.\n");

    printf ("\n\nRUNTIME TRACE OPTIONS:\n\n"

            "\t -trace [amrfpowt]+ \tinclude runtime program tracing.\n"
            "\t\t\t\t  a: trace all (same as mrfpowt).\n"
            "\t\t\t\t  m: trace memory operations.\n"
            "\t\t\t\t  r: trace reference counting operations.\n"
            "\t\t\t\t  f: trace user-defined function calls.\n"
            "\t\t\t\t  p: trace primitive function calls.\n"
            "\t\t\t\t  o: trace old with-loop execution.\n"
            "\t\t\t\t  w: trace new with-loop execution.\n"
            "\t\t\t\t  t: trace multi-threading specific operations.\n"
            "\t\t\t\t  c: trace runtime enviroment init/exit when\n"
            "\t\t\t\t     using sac-libraries in c programms.\n");

    printf ("\n\nRUNTIME PROFILING OPTIONS:\n\n"

            "\t -profile [afilw]+ \tinclude runtime profiling analysis.\n"
            "\t\t\t\t  a: analyse all (same as filw).\n"
            "\t\t\t\t  f: analyse time spent in non-inline functions.\n"
            "\t\t\t\t  i: analyse time spent in inline functions.\n"
            "\t\t\t\t  l: analyse time spent in library functions.\n"
            "\t\t\t\t  w: analyse time spent in with-loops.\n");

    env = getenv ("SACBASE");
    printf ("\n\nCACHE SIMULATION OPTIONS:\n\n"

            "\t -cs\t\tenable runtime cache simulation\n"
            "\n"
            "\t -csdefaults [sagbifp]+\n\n"

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
            "\t\tThe default simulation parameters are \"sgp\".\n"
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
            "\tresults, in particular for advanced analysis, are often inaccurate due\n"
            "\tto changes in the memory layout caused by the analyser. If you choose\n"
            "\tto write memory accesses to a file, beware that even for small programs\n"
            "\tto be analysed the amount of data may be quite large. However, once a\n"
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
            "\t-cshost <name> \tallows the specification of a specific host to run the\n"
            "\t\t\tadditional analyser process on when doing piped cache\n"
            "\t\t\tsimulation. This is very useful for single processor\n"
            "\t\t\tmachines because the rather limited buffer size of the\n"
            "\t\t\tpipe determines the synchronisation distance of the two\n"
            "\t\t\tprocesses, i.e. the application process and the analysis\n"
            "\t\t\tprocess. This results in very frequent context switches\n"
            "\t\t\twhen both processes are run on the same processor, and\n"
            "\t\t\tconsequently, degrades the performance by orders of\n"
            "\t\t\tmagnitude. So, when doing piped cache simulation always\n"
            "\t\t\tbe sure to do so either on a multiprocessor or specify a\n"
            "\t\t\tdifferent machine to run the analyser process on.\n"
            "\t\t\tHowever, this only defines a default which may be\n"
            "\t\t\toverridden by using this option when starting the\n"
            "\t\t\tcompiled application program.\n"
            "\n"
            "\t-csfile <name> \tallows the specification of a default file where to\n"
            "\t\t\twrite the memory access trace when performing cache\n"
            "\t\t\tsimulation via a file. This default may be overridden by\n"
            "\t\t\tusing this option when starting the compiled application\n"
            "\t\t\tprogram.\n"
            "\t\t\tThe general default name is <exec file name>.cs.\n"
            "\n"
            "\t-csdir <name> \tallows the specification of a default directory where to\n"
            "\t\t\twrite the memory access trace file when performing cache\n"
            "\t\t\tsimulation via a file. This default may be overridden by\n"
            "\t\t\tusing this option when starting the compiled application\n"
            "\t\t\tprogram.\n"
            "\t\t\tThe general default directory is the tmp directory\n"
            "\t\t\tspecified in your sac2crc file.\n",
            STR_OR_EMPTY (env),
            ((NULL != env) && (env[strlen (env) - 1] != '/')) ? "/" : "", version_id);

    printf ("\n\nINTRINSIC ARRAY OPERATIONS OPTIONS:\n\n"

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
            "\t\t\t\t\t  s: use intrinsic sel.\n"
            "\t\t\t\t\t  o: use intrinsic type conversion.\n");

    printf ("\n\nLIBRARY OPTIONS:\n\n"
            "\t -genlib <lang>\tlanguage interface to generate from module.\n"
            "\t\t\tsac: generate SAC library.\n"
            "\t\t\t  c: generate C library and headerfile. Be careful to\n"
            "\t\t\t     use same switches for PHM and profiling in all\n"
            "\t\t\t     modules you link to one c executeable! Multi-\n"
            "\t\t\t     threading is not yet available for c libraries!\n"
            "\t\t\t     (see also documentation in sac_cinterface.h)\n"
            "\t\t\tdefault: -genlib sac\n"

            "\n\t -l <n>\t\tlink level for generating SAC library.\n"
            "\t\t\t  1: compile to one large object file.\n"
            "\t\t\t  2: compile to archive of object files.\n"
            "\t\t\tdefault: -l %d\n",
            linkstyle);

    printf ("\n\nC-COMPILER OPTIONS:\n\n"

            "\t -g     \tinclude debug information into object code.\n"
            "\n"
            "\t -O <n> \tC compiler level of optimization.\n"
            "\t\t\t  0: no C compiler optimizations.\n"
            "\t\t\t  1: minor C compiler optimizations.\n"
            "\t\t\t  2: medium C compiler optimizations.\n"
            "\t\t\t  3: full C compiler optimizations.\n"
            "\t\t\tdefault: -O %d\n"
            "\n"
            "\tThe actual effects of these options are C compiler specific!\n",
            cc_optimize);

    printf ("\n\nCUSTOMIZATION:\n\n"

            "\t-target <name>\tspecify a particular compilation target.\n"
            "\t\t\tCompilation targets are used to customize sac2c for\n"
            "\t\t\tvarious target architectures, operating systems, and C\n"
            "\t\t\tcompilers.\n"
            "\t\t\tThe target description is either read from the\n"
            "\t\t\tinstallation specific file $SACBASE/runtime/sac2crc or\n"
            "\t\t\tfrom a file named .sac2crc within the user's home\n"
            "\t\t\tdirectory.\n");

    printf ("\n\nENVIRONMENT VARIABLES:\n\n"

            "\tSACBASE\t\t\tbase directory of SAC installation\n"
            "\tSAC_PATH\t\tsearch paths for program source\n"
            "\tSAC_DEC_PATH\t\tsearch paths for declarations\n"
            "\tSAC_LIBRARY_PATH\tsearch paths for libraries\n"
            "\n"
            "\tThe following environment variables must be set correctly when compiling\n"
            "\ta SAC module/class implementation in order to enable full usability of\n"
            "\tsac2c command line option \"-libstat\": PWD, USER, and HOST.\n");

    printf ("\n\nAUTHORS:\n\n"

            "\tSven-Bodo Scholz\n"
            "\tHenning Wolf\n"
            "\tArne Sievers\n"
            "\tClemens Grelck\n"
            "\tDietmar Kreye\n"
            "\tSoeren Schwartz\n"
            "\tBjoern Schierau\n"
            "\tHelge Ernst\n"
            "\tJan-Hendrik Schoeler\n"
            "\tNico Marcussen-Wulff\n"
            "\tMarkus Bradtke\n"
            "\tBorg Enders\n"
            "\tKai Trojahner\n");

    printf ("\n\nCONTACT:\n\n"

            "\tWorld Wide Web: http://www.informatik.uni-kiel.de/~sacbase/\n"
            "\tE-Mail: sacbase@informatik.uni-kiel.de\n");

    printf ("\n\nBUGS:\n\n"

            "\tBugs??  We????\n"
            "\n"
            "\tUnfortunately, two of our optimizations are quite buggy 8-(\n"
            "\tTherefore, we decided to preset -noLIR and -noDLAW in the current\n"
            "\tcompiler release!\n\n");

    DBUG_VOID_RETURN;
}

void
version ()
{
    DBUG_ENTER ("version");

    printf ("\n\t\t  SAC - Single Assignment C\n"
            "\t---------------------------------------------\n\n"

            "NAME:      sac2c\n"
            "VERSION:   %s\n"
            "PLATFORM:  %s\n"
            "\n"

            "BUILD:     %s\n"
            "BY USER:   %s\n"
            "ON HOST:   %s\n"
            "FOR OS:    %s\n"
            "\n\n"

            "(c) Copyright 1994 - 2002 by\n\n"

            "  Christian-Albrechts-Universitaet zu Kiel\n"
            "  Institut fuer Informatik und Praktische Mathematik\n"
            "  Herman-Rodewald-Str.3\n"
            "  D-24118 Kiel\n"
            "  Germany\n\n",
            (version_id[0] == '\0') ? "???" : version_id,
            (target_platform[0] == '\0') ? "???" : target_platform,
            (build_date[0] == '\0') ? "???" : build_date,
            (build_user[0] == '\0') ? "???" : build_user,
            (build_host[0] == '\0') ? "???" : build_host,
            (build_os[0] == '\0') ? "???" : build_os);

    DBUG_VOID_RETURN;
}

void
copyright ()
{
    DBUG_ENTER ("copyright");

    printf ("\n\t\t  SAC - Single Assignment C\n"
            "\t---------------------------------------------\n\n"

            "\tSAC COPYRIGHT NOTICE, LICENSE, AND DISCLAIMER\n\n"

            "(c) Copyright 1994 - 2000 by\n\n"

            "  Christian-Albrechts-Universitaet zu Kiel\n"
            "  Institut fuer Informatik und Praktische Mathematik\n"
            "  Preusserstrasse 1 - 9\n"
            "  D-24105 Kiel\n"
            "  Germany\n\n");

    printf (
      "The SAC compiler, the SAC standard library, and all accompanying\n"
      "software and documentation (in the following named this software)\n"
      "is developed by the SAC group as part of the Chair of Computer\n"
      "Organization within the Department of Computer Science and Applied\n"
      "Mathematics of the University of Kiel (in the following named CAU Kiel)\n"
      "which reserves all rights on this software.\n"
      "\n"
      "Permission to use this software is hereby granted free of charge\n"
      "for any non-profit purpose in a non-commercial environment, i.e. for\n"
      "educational or research purposes in a non-profit institute or for\n"
      "personal, non-commercial use. For this kind of use it is allowed to\n"
      "copy or redistribute this software under the condition that the\n"
      "complete distribution for a certain platform is copied or\n"
      "redistributed and this copyright notice, license agreement, and\n"
      "warranty disclaimer appears in each copy. ANY use of this software with\n"
      "a commercial purpose or in a commercial environment is not granted by\n"
      "this license.\n"
      "\n"
      "CAU Kiel disclaims all warranties with regard to this software, including\n"
      "all implied warranties of merchantability and fitness.  In no event\n"
      "shall CAU Kiel be liable for any special, indirect or consequential\n"
      "damages or any damages whatsoever resulting from loss of use, data, or\n"
      "profits, whether in an action of contract, negligence, or other\n"
      "tortuous action, arising out of or in connection with the use or\n"
      "performance of this software. The entire risk as to the quality and\n"
      "performance of this software is with you. Should this software prove\n"
      "defective, you assume the cost of all servicing, repair, or correction.\n\n");

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
