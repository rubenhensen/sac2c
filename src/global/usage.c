/*
 *
 * $Log$
 * Revision 3.35  2002/11/14 13:33:12  dkr
 * minor changes done
 *
 * Revision 3.34  2002/11/08 13:29:45  cg
 * Added infos concerning Fred Fish DBUG package options.
 * Beautified entire layout of usage screen.
 * Modified contact information to new domain sac-home.org.
 * Added Stephan Herhut as new developer to sac2c.
 *
 * Revision 3.33  2002/10/30 14:19:42  dkr
 * -enforceIEEE for with-loops implemented now
 *
 * Revision 3.32  2002/10/25 16:01:55  mwe
 * option enforce_ieee added
 * rename DLAW to DL
 *
 * Revision 3.31  2002/10/24 13:12:32  ktr
 * level of WLS aggressiveness now controlled by flag -wls_aggressive
 *
 * Revision 3.30  2002/10/19 13:16:25  dkr
 * some \n added
 *
 * Revision 3.29  2002/10/18 14:16:50  ktr
 * changed option -wlsx to -wls <level>
 *
 * Revision 3.28  2002/10/17 17:53:08  ktr
 * added option -wlsx for aggressive WLS
 *
 * Revision 3.27  2002/09/05 12:05:08  dkr
 * -b7:n2o added
 *
 * Revision 3.26  2002/09/03 22:27:21  dkr
 * CONT_BREAK_SPEC added
 *
 * Revision 3.25  2002/08/13 10:51:44  sbs
 * break specifiers for flatten and typecheck (new type checker)
 * added.
 *
 * Revision 3.24  2002/07/15 19:05:07  dkr
 * -intrinsic flag modified for TAGGED_ARRAYS
 *
 * Revision 3.23  2002/07/10 16:33:38  dkr
 * -b2:yacc added
 *
 * Revision 3.22  2002/07/03 15:28:18  dkr
 * -checkt added (for TAGGED_ARRAYS)
 *
 * Revision 3.21  2002/06/24 14:35:34  dkr
 * -intrinsic flag removed for TAGGED_ARRAYS
 *
 * Revision 3.20  2002/06/07 17:04:03  mwe
 * help information for AssociativeLaw added.
 *
 * Revision 3.19  2002/04/09 16:38:30  dkr
 * break specifier for -b18 added
 *
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
        printf ("    -b %2i:%s", ph, spec);                                              \
        for (_i = 0; _i < (12 - strlen (spec)); _i++) {                                  \
            printf (" ");                                                                \
        }                                                                                \
        printf ("%s\n", comment);                                                        \
    }

#define CONT_BREAK_SPEC(comment)                                                         \
    {                                                                                    \
        printf ("                      %s\n", comment);                                  \
    }

void
usage ()
{
    int ph;
    char *env;

    DBUG_ENTER ("usage");

    printf (
      "\n\n"
      "      sac2c  --  The Ultimate SAC Compiler\n"
      "    ----------------------------------------\n\n\n"

      "NAME:         sac2c\n\n\n"

      "DESCRIPTION:\n\n"

      "    The sac2c compiler transforms SAC source code into executable programs\n"
      "    (SAC programs) or into a SAC specific library format (SAC module and\n"
      "    class implementations), respectively.\n"
      "    \n"
      "    The compilation process is performed in 4 separate stages:\n"
      "      1. sac2c uses any C preprocessor to preprocess the given SAC source;\n"
      "      2. sac2c itself transforms preprocessed SAC source code into C code;\n"
      "      3. sac2c uses any C compiler to generate target machine code;\n"
      "      4. sac2c uses any C linker to create an executable program\n"
      "         or sac2c itself creates a SAC library file.\n"
      "    \n"
      "    When compiling a SAC program, sac2c stores the corresponding\n"
      "    intermediate C code either in the file a.out.c in the current directory\n"
      "    (default) or in the file <file>.c if <file> is specified using the -o\n"
      "    option. Here, any absolute or relative path name may be used.\n"
      "    The executable program is either written to the file a.out or to any\n"
      "    file specified using the -o option.\n"
      "    \n"
      "    However, when compiling a SAC module/class implementation, the\n"
      "    resulting SAC library is stored in the file <mod/class name>.lib in the\n"
      "    current directory. In this case, the -o option may be used to specify a\n"
      "    different directory but not a different file name.\n");

    printf (
      "\n\nSPECIAL OPTIONS:\n\n"

      "    -h              Display this helptext.\n"
      "    -help           Display this helptext.\n"
      "    -copyright      Display copyright/disclaimer.\n"
      "    -V              Display version identification.\n"
      "\n"
      "    -libstat        Print status information of the given SAC library file.\n"
      "                    This option requires the environment variables PWD,\n"
      "                    USER, and HOST to be set when compiling the module/\n"
      "                    class implementation in order to work correctly.\n"
      "\n"
      "    -M              Detect dependencies from imported modules/classes and\n"
      "                    write them to stdout in a way suitable for the make\n"
      "                    utility. Only dependencies from declaration files are\n"
      "                    considered.\n"
      "\n"
      "    -MM             Like `-M' but the output mentions only non-standard\n"
      "                    library dependencies.\n"
      "\n"
      "    -Mlib           Detect dependencies from imported modules/classes and\n"
      "                    write them to stdout in a way suitable for the make\n"
      "                    utility. Dependencies from declaration files as well\n"
      "                    as library files are (recursively) considered.\n"
      "\n"
      "    -MMlib          Like `-Mlib' but the output mentions only non standard\n"
      "                    library dependencies.\n"
      "\n"
      "    NOTE:\n"
      "    When called with one of these options, sac2c does not perform\n"
      "    any compilation steps.\n");

    printf ("\n\nGENERAL OPTIONS:\n\n"

            "    -D <var>        Set preprocessor variable <var>.\n"
            "    -D <var>=<val>  Set preprocessor variable <var> to <val>.\n"
            "\n"
            "    -I <path>       Specify additional module/class declaration file path.\n"
            "    -L <path>       Specify additional SAC library file path.\n"
            "\n"
            "    -o <name>       For compilation of programs:\n"
            "                      Write executable to specified file.\n"
            "                    For compilation of module/class implementations:\n"
            "                      Write library to specified directory.\n"
            "\n"
            "    -c              Generate C-file only; do not invoke C compiler.\n"
            "\n"
            "    -v <n>          Specify verbose level:\n"
            "                      0: error messages only,\n"
            "                      1: error messages and warnings,\n"
            "                      2: basic compile time information,\n"
            "                      3: full compile time information.\n"
            "                    (default: %d)\n",
            verbose_level);

    printf ("\n\nBREAK OPTIONS:\n\n"

            "    Break options allow you to stop the compilation process\n"
            "    after a particular phase.\n"
            "    By default the programm will then be printed out, but this behaviour\n"
            "    may be influenced by the following compiler options:\n"
            "\n"
            "    -noPAB          Deactivates printing after break.\n"
            "    -doPAB          Activates printing after break.\n\n");

    for (ph = 1; ph <= PH_genccode; ph++) {
        printf ("    -b %2i           Stop after: %s.\n", ph, compiler_phase_name[ph]);
    }

    printf ("\n\nBREAK SPECIFIERS:\n\n"

            "    Break specifiers allow you to stop the compilation process\n"
            "    within a particular phase.\n\n"

            "    Currently supported break specifiers are as follows:\n\n");

    PRINT_BREAK_SPEC (PH_scanparse, "yacc", "Stop after parsing (yacc).");

    printf ("\n");

    PRINT_BREAK_SPEC (PH_flatten, "mop",
                      "Stop after resolving (multiple) applications of infix");
    CONT_BREAK_SPEC ("operations.");

    printf ("\n");

    PRINT_BREAK_SPEC (PH_typecheck, "ivd", "Stop after inserting vardecs.");
    PRINT_BREAK_SPEC (PH_typecheck, "cwr", "Stop after creating wrappers.");
    PRINT_BREAK_SPEC (PH_typecheck, "l2f",
                      "Stop after converting loops and conditionals into");
    CONT_BREAK_SPEC ("functions.");
    PRINT_BREAK_SPEC (PH_typecheck, "cha", "Stop after checking avis consistency.");
    PRINT_BREAK_SPEC (PH_typecheck, "ssa", "Stop after converting into SSA form.");
    PRINT_BREAK_SPEC (PH_typecheck, "ntc", "Stop after infering all types.");
    PRINT_BREAK_SPEC (PH_typecheck, "cwc",
                      "Stop after creating SAC code for wrapper functions.");
    PRINT_BREAK_SPEC (PH_typecheck, "n2o",
                      "Stop after computing old type representation.");

    printf ("\n");

    PRINT_BREAK_SPEC (PH_sacopt, "inl", "Stop after function inlining.");
    PRINT_BREAK_SPEC (PH_sacopt, "dfr", "Stop after initial dead function removal.");
    PRINT_BREAK_SPEC (PH_sacopt, "w2d",
                      "Stop after transf. of while into do loops (ssa only).");
    PRINT_BREAK_SPEC (PH_sacopt, "l2f",
                      "Stop after transf. into fun representation (ssa only).");
    PRINT_BREAK_SPEC (PH_sacopt, "ssa",
                      "Stop after initial ssa transformation (ssa only).");
    PRINT_BREAK_SPEC (PH_sacopt, "ae", "Stop after array elimination.");
    PRINT_BREAK_SPEC (PH_sacopt, "dcr", "Stop after dead code removal.");

    printf ("\n");

    PRINT_BREAK_SPEC (PH_sacopt, "cyc<n>:cse",
                      "Stop in cycle <n> after common subexpression elimination.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc<n>:cf",
                      "Stop in cycle <n> after constant folding.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc<n>:wlt",
                      "Stop in cycle <n> after with-loop transformation.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc<n>:wli",
                      "Stop in cycle <n> after with-loop information gathering.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc<n>:wlf",
                      "Stop in cycle <n> after with-loop folding.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc<n>:wls",
                      "Stop in cycle <n> after with-loop scalarization.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc<n>:al", "Stop in cycle <n> after associative law.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc<n>:cf2",
                      "Stop in cycle <n> after second constant folding.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc<n>:dcr",
                      "Stop in cycle <n> after dead code removal.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc<n>:lur",
                      "Stop in cycle <n> after (with-)loop unrolling.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc<n>:cf3",
                      "Stop in cycle <n> after third constant folding.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc<n>:lus",
                      "Stop in cycle <n> after loop unswitching.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc<n>:lir",
                      "Stop in cycle <n> after (with-)loop invariant removal.");

    printf ("\n");

    PRINT_BREAK_SPEC (PH_sacopt, "funopt", "Stop after fundef optimization cycle.");
    PRINT_BREAK_SPEC (PH_sacopt, "ussa",
                      "Stop after undo ssa transformation (ssa only).");
    PRINT_BREAK_SPEC (PH_sacopt, "f2l",
                      "Stop after transf. into lac representation (ssa only).");
    PRINT_BREAK_SPEC (PH_sacopt, "wlaa", "Stop after with loop array access inference.");
    PRINT_BREAK_SPEC (PH_sacopt, "ap", "Stop after array padding.");
    PRINT_BREAK_SPEC (PH_sacopt, "tsi", "Stop after tile size inference.");
    PRINT_BREAK_SPEC (PH_sacopt, "dfr2", "Stop after final dead function removal.");
    PRINT_BREAK_SPEC (PH_sacopt, "ive", "Stop after index vector elimination.");

    printf ("\n");

    PRINT_BREAK_SPEC (PH_wltrans, "conv", "Stop after converting.");
    PRINT_BREAK_SPEC (PH_wltrans, "cubes", "Stop after cube-building.");
    PRINT_BREAK_SPEC (PH_wltrans, "fill1", "Stop after gap filling (grids only).");
    PRINT_BREAK_SPEC (PH_wltrans, "segs", "Stop after choice of segments.");
    PRINT_BREAK_SPEC (PH_wltrans, "split", "Stop after splitting.");
    PRINT_BREAK_SPEC (PH_wltrans, "block", "Stop after hierarchical blocking.");
    PRINT_BREAK_SPEC (PH_wltrans, "ublock", "Stop after unrolling-blocking.");
    PRINT_BREAK_SPEC (PH_wltrans, "merge", "Stop after merging.");
    PRINT_BREAK_SPEC (PH_wltrans, "opt", "Stop after optimization.");
    PRINT_BREAK_SPEC (PH_wltrans, "fit", "Stop after fitting.");
    PRINT_BREAK_SPEC (PH_wltrans, "norm", "Stop after normalization.");
    PRINT_BREAK_SPEC (PH_wltrans, "fill2", "Stop after gap filling (all nodes).");

    printf ("\n");
    printf ("    with \"-mt\"\n");

    PRINT_BREAK_SPEC (PH_multithread, "spmdinit", "Stop after building SPMD blocks.");
    PRINT_BREAK_SPEC (PH_multithread, "spmdopt", "Stop after optimizing SPMD blocks.");
    PRINT_BREAK_SPEC (PH_multithread, "spmdlift", "Stop after lifting SPMD blocks.");
    PRINT_BREAK_SPEC (PH_multithread, "syncinit", "Stop after building SYNC blocks.");
    PRINT_BREAK_SPEC (PH_multithread, "syncopt", "Stop after optimizing SYNC blocks.");
    PRINT_BREAK_SPEC (PH_multithread, "scheduling",
                      "Stop after scheduling SYNC blocks and with-loop segments.");
    PRINT_BREAK_SPEC (PH_multithread, "spmdcons", "Stop after constraining SPMD blocks.");

    printf ("\n");
    printf ("    with \"-mtn\" (UNDER CONSTRUCTION!!!)\n");

    PRINT_BREAK_SPEC (PH_multithread, "init", "Stop after internal initialization.");
    PRINT_BREAK_SPEC (PH_multithread, "schin", "Stop after schedulings initialized.");
    PRINT_BREAK_SPEC (PH_multithread, "rfin", "Stop after replicated functions built.");
    PRINT_BREAK_SPEC (PH_multithread, "blkin", "Stop after ST- and MT-blocks built.");
    PRINT_BREAK_SPEC (PH_multithread, "blkpp", "Stop after blocks propagated.");
    PRINT_BREAK_SPEC (PH_multithread, "blkex", "Stop after blocks expanded.");
    PRINT_BREAK_SPEC (PH_multithread, "mtfin", "Stop after multithread functions built.");
    PRINT_BREAK_SPEC (PH_multithread, "blkco", "Stop after blocks consolidated.");
    PRINT_BREAK_SPEC (PH_multithread, "dfa", "Stop after dataflow-analysis.");
    PRINT_BREAK_SPEC (PH_multithread, "barin", "Stop after barriers initialized.");
    PRINT_BREAK_SPEC (PH_multithread, "blkli", "Stop after blocks lifted.");
    PRINT_BREAK_SPEC (PH_multithread, "adjca", "Stop after adjusted calls.");

    printf ("\n");

    PRINT_BREAK_SPEC (PH_precompile, "prec1", "Stop after first traversal.");
    PRINT_BREAK_SPEC (PH_precompile, "prec2", "Stop after second traversal.");
    PRINT_BREAK_SPEC (PH_precompile, "prec3", "Stop after third traversal.");

    printf (
      "\n\nOPTIMIZATION OPTIONS:\n\n"
      "    -enforceIEEE    Treat floating point arithmetic as defined in the IEEE-754\n"
      "                    standard. In particular, this means\n"
      "                      - disable some algebraic optimizations,\n"
      "                      - disable segmentation and tiling of fold-with-loops,\n"
      "                      - disable parallel execution of fold-with-loops.\n"
      "                    Currently implemented for:\n"
      "                      - associative law optimization,\n"
      "                      - segmentation and tiling of fold-with-loops.\n"
      "\n"
      "    -ssa            Apply optimizations based on ssa-form, instead of using the\n"
      "                    old non-ssa based implementations.\n"
      "                    NOTE:\n"
      "                    Some optimizations are exclusively implemented in ssa style.\n"
      "                    Support for non-ssa based implementations will be removed in\n"
      "                    future releases.\n"
      "\n"
      "    -no <opt>       Disable optimization technique <opt>.\n"
      "\n"
      "    -do <opt>       Enable optimization technique <opt>.\n"
      "\n\n"
      "    The following optimization techniques are currently supported:\n\n"

      "        CF      constant folding\n"
      "        INL     function inlining\n"
      "        LUR     loop unrolling\n"
      "        WLUR    with-loop unrolling\n"
      "        LUS     loop unswitching\n"
      "        DCR     dead code removal\n"
      "        DFR     dead function removal\n"
      "        LIR     loop invariant removal\n"
      "        CSE     common subexpression elimination\n"
      "        WLT     with-loop transformation\n"
      "        WLF     with-loop folding\n"
      "        WLS     with-loop scalarization\n"
      "        AL      application of associative law\n"
      "        DL      application of distributive law\n"
      "        IVE     index vector elimination\n"
      "        AE      array elimination\n"
      "        RCO     refcount optimization\n"
      "        UIP     update-in-place\n"
      "        AP      array padding\n"
      "        APL     array placement\n"
      "        TSI     tile size inference (blocking)\n"
      "        TSP     tile size pragmas (blocking)\n"
      "        MTO     multi-thread optimization\n"
      "        SBE     syncronisation barrier elimination\n"
      "        PHM     private heap management\n"
      "        APS     arena preselection           (in conjunction with PHM)\n"
      "        RCAO    refcount allocation optimiz. (in conjunction with PHM)\n"
      "        MSCA    memory size cache adjustment (in conjunction with PHM)\n"
      "\n"
      "        OPT     enables/disables all optimizations at once.\n"
      "\n"
      "    NOTE:\n"
      "    Lower case letters may be used to indicate optimization techniques.\n"
      "\n"
      "    NOTE:\n"
      "    Command line arguments are evaluated from left to right, i.e.,\n"
      "    \"-no OPT -do INL\" disables all optimizations except for function "
      "inlining.\n\n");

    printf (
      "    Some of the optimization techniques are parameterized by additional side\n"
      "    conditions. They are controlled by the following options:\n"
      "\n"
      "    -maxoptcyc <n>  Repeat optimization cycle <n> times.\n"
      "                      (default: %d)\n\n",
      max_optcycles);

    printf ("    -maxoptvar <n>  Reserve <n> variables for optimization.\n"
            "                      (default: %d)\n\n",
            optvar);

    printf ("    -maxinl <n>     Inline recursive functions at most <n> times.\n"
            "                      (default: %d)\n\n",
            inlnum);

    printf ("    -maxlur <n>     Unroll loops having at most <n> iterations.\n"
            "                      (default: %d)\n\n",
            unrnum);

    printf (
      "    -maxwlur <n>    Unroll with-loops with at most <n> elements generator set\n"
      "                    size.\n"
      "                      (default: %d)\n\n",
      wlunrnum);

    printf ("    -maxae <n>      Try to eliminate arrays with at most <n> elements.\n"
            "                      (default: %d)\n\n",
            minarray);

    printf (
      "    -maxspec <n>    Individual functions will be specialized at most <n> times.\n"
      "                      (default: %d)\n\n",
      max_overload);

    printf (
      "    -initmheap <n>  At program startup initially request <n> KB of heap memory\n"
      "                    for master thread.\n"
      "                      (default: %d)\n\n",
      initial_master_heapsize);

    printf (
      "    -initwheap <n>  At program startup initially request <n> KB of heap memory\n"
      "                    for each worker thread.\n"
      "                      (default: %d)\n\n",
      initial_worker_heapsize);

    printf (
      "    -inituheap <n>  At program startup initially request <n> KB of heap memory\n"
      "                    for usage by all threads.\n"
      "                      (default: %d)\n\n",
      initial_unified_heapsize);

    printf (
      "    -aplimit <n>    Set the array padding resource allocation overhead limit\n"
      "                    to <n> %%.\n"
      "                      (default: %d)\n\n",
      padding_overhead_limit);

    printf (
      "    -apdiag         Print additional information for array padding to file\n"
      "                    \"<outfile>.ap\", where <outfile> is the name specified via\n"
      "                    the \"-o\" option.\n\n");

    printf (
      "    -apdiagsize <n> Limit the amount of information written to the diagnostic\n"
      "                    output file created via the -apdiag option to approximately\n"
      "                    <n> lines.\n"
      "                      (default: %d)\n\n",
      apdiag_limit);

    printf (
      "    -wls_aggressive Set WLS optimization level to aggressive.\n"
      "                    WARNING:\n"
      "                    Aggressive with-loop scalarization may have the opposite\n"
      "                    effect as with-loop invariant removal and cause duplication\n"
      "                    of code execution.\n");

    printf ("\n\nMULTI-THREAD OPTIONS:\n\n"

            "    -mt             Compile program for multi-threaded execution,\n"
            "                    e.g. implicitly parallelize the code for "
            "non-sequential\n"
            "                    execution on shared memory multiprocessors.\n"
            "\n"
            "                    NOTE:\n"
            "                    The number of threads to be used can either be "
            "specified\n"
            "                    statically using the option \"-numthreads\" or "
            "dynamically\n"
            "                    upon application startup using the generic command "
            "line\n"
            "                    option \"-mt <n>\".\n"
            "\n"
            "    -mtn            Enable a new organization scheme for multi-threaded "
            "program\n"
            "                    execution.\n"
            "                    WARNING: UNDER CONSTRUCTION!!!\n"
            "\n"
            "    -numthreads <n> Specify at compile time the exact number of threads to "
            "be\n"
            "                    used for parallel execution.\n"
            "\n"
            "    -maxthreads <n> Specify at compile time only an upper bound on the "
            "number\n"
            "                    of threads to be used  for parallel execution when "
            "exact\n"
            "                    number is determined at runtime.\n"
            "                      (default: %d)\n"
            "\n"
            "    -maxsync <n>    Specify maximum number of fold with-loops to be "
            "combined\n"
            "                    into a single synchronisation block.\n"
            "                    Legal values:\n"
            "                      -1: maximum number needed (mechanically infered).\n"
            "                       0: no fold-with-loops are allowed.\n"
            "                          (This implies that fold-with-loops are not "
            "executed\n"
            "                           in parallel.)\n"
            "                      >0: maximum number set to <n>.\n"
            "                      (default: %d)\n"
            "\n"
            "    -minmtsize <n>  Specify minimum generator set size for parallel "
            "execution\n"
            "                    of with-loops.\n"
            "                      (default: %d)\n"
            "\n"
            "    -maxrepsize <n> Specify maximum size for arrays to be replicated as\n"
            "                    private data of multiple threads.\n"
            "                      (default: %d)\n"
            "                    Option applies to \"-mtn\" style parallelization "
            "only.\n",
            max_threads, max_sync_fold, min_parallel_size, max_replication_size);

    printf (
      "\n\nGENERAL DEBUG OPTIONS:\n\n"

      "    -d nocleanup    Do not remove temporary files and directories.\n"
      "    -d syscall      Show all system calls during compilation.\n"
      "    -d cccall       Generate shell script \".sac2c\" that contains C compiler\n"
      "                    invocation.\n"
      "                    This implies option \"-d nocleanup\".\n");

    printf (
      "\n\nINTERNAL DEBUG OPTIONS:\n\n"

      "    -d efence       Link executable with ElectricFence (malloc debugger).\n"
      "\n"
      "    -# t            Display trace information generated by Fred Fish DBUG\n"
      "                    package.\n"
      "                    Each function entry and exit during program execution is\n"
      "                    printed on the screen.\n"
      "\n"
      "    -# d            Display debug output information generated by Fred Fish\n"
      "                    DBUG package.\n"
      "                    Each DBUG_PRINT macro in the code will be executed.\n"
      "                    Each DBUG_EXECUTE macro in the code will be executed.\n"
      "\n"
      "    -# d,<str>      Restrict \"-# d\" option to DBUG_PRINT / DBUG_EXECUTE macros\n"
      "                    which are tagged with the string <str> (no quotes).\n"
      "\n"
      "    -# <f>/<t>/<o>  Restrict the effect of any Fred Fish DBUG package option <o>\n"
      "                    to the range <f> to <t> of sac2c compiler phases.\n"
      "                      (default: <f> = first compiler phase,\n"
      "                                <t> = last compiler phase.)\n"
      "\n"
      "    -lac2fun <ph>[:<ph>]*\n"
      "                    Transform loops and conditionals into functions before\n"
      "                    compiler phases <ph>.\n"
      "                    NOTE:\n"
      "                    \"-b <ph>\" stops the compiler *after* the lac2fun\n"
      "                    transformation of phase <ph+1>!\n"
      "\n"
      "    -fun2lac <ph>[:<ph>]*\n"
      "                    Transform specific functions back into loops and\n"
      "                    conditionals after compiler phases <ph>.\n");

    printf ("\n\nRUNTIME CHECK OPTIONS:\n\n"

#ifdef TAGGED_ARRAYS
            "    -check [atbmeh]+\n"
#else
            "    -check [abmeh]+\n"
#endif
            "                    Incorporate runtime checks into executable program.\n"
            "                    The following flags are supported:\n"
            "                      a: Incorporate all available runtime checks.\n"
#ifdef TAGGED_ARRAYS
            "                      t: Check assignments for type violations.\n"
#endif
            "                      b: Check array accesses for boundary violations.\n"
            "                      m: Check success of memory allocations.\n"
            "                      e: Check errno variable upon applications of\n"
            "                         external functions.\n"
            "                      h: Use diagnostic heap manager.\n");

    printf (
      "\n\nRUNTIME TRACE OPTIONS:\n\n"

      "    -trace [amrfpwt]+\n"
      "                    Incorporate trace output generation into executable program.\n"
      "                    The following flags are supported:\n"
      "                      a: Trace all (same as mrfpowt).\n"
      "                      m: Trace memory operations.\n"
      "                      r: Trace reference counting operations.\n"
      "                      f: Trace user-defined function calls.\n"
      "                      p: Trace primitive function calls.\n"
      "                      w: Trace with-loop execution.\n"
      "                      t: Trace multi-threading specific operations.\n"
      "                      c: Trace runtime enviroment init/exit when\n"
      "                         using SAC libraries in C programs.\n");

    printf (
      "\n\nRUNTIME PROFILING OPTIONS:\n\n"

      "    -profile [afilw]+\n"
      "                    Incorporate profiling analysis into executable program.\n"
      "                      a: Analyse all (same as filw).\n"
      "                      f: Analyse time spent in non-inline functions.\n"
      "                      i: Analyse time spent in inline functions.\n"
      "                      l: Analyse time spent in library functions.\n"
      "                      w: Analyse time spent in with-loops.\n");

    env = getenv ("SACBASE");
    printf ("\n\nCACHE SIMULATION OPTIONS:\n\n"

            "    -cs             Enable runtime cache simulation.\n"
            "\n"
            "    -csdefaults [sagbifp]+\n"
            "                    This option sets default parameters for cache "
            "simulation.\n"
            "                    These settings may be overridden when starting the "
            "analysis\n"
            "                    of an application program:\n"
            "                      s: simple cache simulation,\n"
            "                      a: advanced cache simulation,\n"
            "                      g: global cache simulation,\n"
            "                      b: cache simulation on selected blocks,\n"
            "                      i: immediate analysis of memory access data,\n"
            "                      f: storage of memory access data in file,\n"
            "                      p: piping of memory access data to concurrently "
            "running\n"
            "                         analyser process.\n"
            "                    The default simulation parameters are \"sgp\".\n"
            "\n"
            "    -cshost <name>  This option specifies the host machine to run the "
            "additional\n"
            "                    analyser process on when doing piped cache simulation.\n"
            "                    This is very useful for single processor machines "
            "because\n"
            "                    the rather limited buffer size of the pipe determines "
            "the\n"
            "                    synchronisation distance of the two processes, i.e. "
            "the\n"
            "                    application process and the analysis process. This "
            "results\n"
            "                    in very frequent context switches when both processes "
            "are\n"
            "                    run on the same processor, and consequently, degrades "
            "the\n"
            "                    performance by orders of magnitude. So, when doing "
            "piped\n"
            "                    cache simulation always be sure to do so either on a\n"
            "                    multiprocessor or specify a different machine to run "
            "the\n"
            "                    analyser process on. However, this only defines a "
            "default\n"
            "                    which may be overridden by using this option when "
            "starting\n"
            "                    the compiled application program.\n"
            "\n"
            "    -csfile <name>  This option specifies a default file where to write "
            "the\n"
            "                    memory access trace when performing cache simulation "
            "via\n"
            "                    a file. This default may be overridden by using this "
            "option\n"
            "                    when starting the compiled application program.\n"
            "                    The general default name is \"<executable_name>.cs\".\n"
            "\n"
            "    -csdir <name>   This option specifies a default directory where to "
            "write\n"
            "                    the memory access trace file when performing cache\n"
            "                    simulation via a file. This default may be overridden "
            "by\n"
            "                    using this option when starting the compiled "
            "application\n"
            "                    program.\n"
            "                    The general default directory is the tmp directory "
            "specified\n"
            "                    in your sac2crc file.\n"
            "\n\n"
            "CACHE SIMULATION FEATURES:\n"
            "\n"
            "    Simple cache simulation only counts cache hits and cache misses while\n"
            "    advanced cache simulation additionally classifies cache misses into\n"
            "    cold start, cross interference, self interference, and invalidation\n"
            "    misses.\n"
            "\n"
            "    Simulation results may be presented for the entire program run or more\n"
            "    specifically for any code block marked by the following pragma:\n"
            "        #pragma cachesim [tag]\n"
            "    The optional tag allows to distinguish between the simulation results\n"
            "    for various code blocks. The tag must be a string.\n"
            "\n"
            "    Memory accesses may be evaluated with respect to their cache behaviour\n"
            "    either immediately within the application process, stored in a file,\n"
            "    or they may be piped to a concurrently running analyser process.\n"
            "    Whereas immediate analysis usually is the fastest alternative,\n"
            "    results, in particular for advanced analysis, are often inaccurate due\n"
            "    to changes in the memory layout caused by the analyser. If you choose\n"
            "    to write memory accesses to a file, beware that even for small "
            "programs\n"
            "    to be analysed the amount of data may be quite large. However, once a\n"
            "    memory trace file exists, it can be used to simulate different cache\n"
            "    configurations without repeatedly running the application program\n"
            "    itself. The simulation tool for memory access trace files is called\n"
            "        CacheSimAnalyser\n"
            "    and may be found in the directory\n"
            "        %s%sruntime\n"
            "    as part of your SAC %s installation.\n"
            "\n"
            "    These default cache simulation parameters may be overridden when\n"
            "    invoking the application program to be analysed by using the generic\n"
            "    command line option\n"
            "        -cs [sagbifp]+\n"
            "    where the various flags have the same meaning as described for the\n"
            "    \"-csdefaults\" compiler option.\n"
            "\n"
            "    Cache parameters for up to 3 levels of caches may be provided as "
            "target\n"
            "    specification in the sac2crc file. However, these only serve as a\n"
            "    default cache specification which may well be altered when running the\n"
            "    compiled SAC program with cache simulation enabled. This can be done\n"
            "    using the following command line options:\n"
            "        -cs[123] <size>[/<line size>[/<assoc>[/<write miss policy>]]].\n"
            "    The cache size must be given in KBytes, the cache line size in\n"
            "    Bytes. A cache size of 0 KB disables the corresponding cache level\n"
            "    completely regardless of any other setting.\n"
            "    Write miss policies are specified by a single letter:\n"
            "        d: default (fetch on write)\n"
            "        f: fetch on write\n"
            "        v: write validate\n"
            "        a: write around\n",
            STR_OR_EMPTY (env),
            ((NULL != env) && (env[strlen (env) - 1] != '/')) ? "/" : "", version_id);

    printf ("\n\nINTRINSIC ARRAY OPERATIONS OPTIONS:\n\n"

            "    For compatibility reasons with older versions of sac2c intrinsic\n"
            "    implementations still exist for some of the basic array operations\n"
            "    which today are imported from the array module of the SAC standard\n"
            "    library. These intrinsic implementations can be activated by the\n"
            "    following compiler option.\n"
            "\n"
            "    -intrinsic [a+-x/tdcrpo]+\n"
            "                    Use intrinsic implementations for array operations.\n"
            "                      a: Use all intrinsic operations  available\n"
#ifndef TAGGED_ARRAYS
            "                         (same as +-x/tdcrso).\n"
#else
            "                         (same as +-x/so).\n"
#endif
            "                      +: Use intrinsic add.\n"
            "                      -: Use intrinsic sub.\n"
            "                      x: Use intrinsic mul.\n"
            "                      /: Use intrinsic div.\n"
#ifndef TAGGED_ARRAYS
            "                      t: Use intrinsic take.\n"
            "                      d: Use intrinsic drop.\n"
            "                      c: Use intrinsic cat.\n"
            "                      r: Use intrinsic rotate.\n"
#endif
            "                      s: Use intrinsic sel.\n"
            "                      o: Use intrinsic type conversion.\n");

    printf ("\n\nLIBRARY OPTIONS:\n\n"

            "    -genlib <lang>  Specify library format when compiling SAC module/class\n"
            "                    implementations.\n"
            "                    Supported values for <lang> are:\n"
            "                      sac: Generate SAC library file (default).\n"
            "                        c: Generate C object and header files.\n"
            "\n"
            "                    NOTE:\n"
            "                    Be careful to use same options for privat heap "
            "management\n"
            "                    (PHM) and profiling for compilation of all "
            "modules/classes\n"
            "                    you are going to link together to a single executable.\n"
            "\n"
            "                    NOTE:\n"
            "                    Multithreading is not yet available for C libraries.\n"

            "\n"
            "    -l <style>      Specify the link style for generating SAC library "
            "files.\n"
            "                    Supported values for <style> are:\n"
            "                      1: Compile to one large object file.\n"
            "                      2: Compile to archive of object files.\n"
            "                    (default: %d)\n",
            linkstyle);

    printf ("\n\nC-COMPILER OPTIONS:\n\n"

            "    -g              Include debug information into object code.\n"
            "\n"
            "    -O <n>          Specify  the C compiler level of optimization.\n"
            "                      0: no C compiler optimizations.\n"
            "                      1: minor C compiler optimizations.\n"
            "                      2: medium C compiler optimizations.\n"
            "                      3: full C compiler optimizations.\n"
            "                    (default: %d)\n"
            "\n"
            "                    NOTE:\n"
            "                    The actual effects of these options are specific to "
            "the\n"
            "                    C compiler used for code generation. Both the choice "
            "of\n"
            "                    a C compiler as well as the mapping of these generic\n"
            "                    options to compiler-specific optimization options are\n"
            "                    are determined via the sac2crc configuration file.\n"
            "                    For details concerning sac2crc files see below under\n"
            "                    \"customization\".\n",
            cc_optimize);

    printf ("\n\nCUSTOMIZATION:\n\n"

            "    -target <name>  Specify a particular compilation target.\n"
            "                    Compilation targets are used to customize sac2c for\n"
            "                    various target architectures, operating systems, and C\n"
            "                    compilers.\n"
            "                    The target description is either read from the\n"
            "                    installation specific file $SACBASE/runtime/sac2crc or\n"
            "                    from a file named .sac2crc within the user's home\n"
            "                    directory.\n");

    printf (
      "\n\nENVIRONMENT VARIABLES:\n\n"

      "    The following environment variables are used by sac2c:\n"
      "\n"
      "    SACBASE           Base directory of SAC installation.\n"
      "    SAC_PATH          Search path for SAC source code files.\n"
      "    SAC_DEC_PATH      Search path for module/class declaration files.\n"
      "    SAC_LIBRARY_PATH  Search path for SAC library files.\n"
      "\n"
      "    The following environment variables must be set correctly when compiling\n"
      "    a SAC module/class implementation in order to enable full usability of\n"
      "    sac2c command line option \"-libstat\": PWD, USER, and HOST.\n");

    printf ("\n\nAUTHORS:\n\n"

            "    The following people contributed their time and mind to create sac2c\n"
            "    (roughly in order of entering the project):\n"
            "\n"
            "      Sven-Bodo Scholz\n"
            "      Henning Wolf\n"
            "      Arne Sievers\n"
            "      Clemens Grelck\n"
            "      Dietmar Kreye\n"
            "      Soeren Schwartz\n"
            "      Bjoern Schierau\n"
            "      Helge Ernst\n"
            "      Jan-Hendrik Schoeler\n"
            "      Nico Marcussen-Wulff\n"
            "      Markus Bradtke\n"
            "      Borg Enders\n"
            "      Kai Trojahner\n"
            "      Michael Werner\n"
            "      Stephan Herhut\n");

    printf ("\n\nCONTACT:\n\n"

            "    WWW:    http://www.sac-home.org/\n"
            "    E-Mail: info@sac-home.org\n");

    printf (
      "\n\nBUGS:\n\n"

      "    Bugs??  We????\n"
      "\n"
      "    Sac2c is a research compiler!\n"
      "\n"
      "    It is intended as a platform for scientific research rather than a\n"
      "    \"product\" for end users. Although we try to do our very best,\n"
      "    you may well run into a compiler bug. So, we are happy to receive\n"
      "    your bug reports (Well, not really \"happy\", but ...)\n"
      "\n"
      "    Unfortunately, two of our optimizations are quite buggy 8-(\n"
      "    Therefore, we decided to preset \"-noLIR\" (non-ssa version) and \"-noDL\"\n"
      "    in the current compiler release.\n");

    printf ("\n\n");

    DBUG_VOID_RETURN;
}

void
version ()
{
    DBUG_ENTER ("version");

    printf ("\n          SAC - Single Assignment C\n"
            "    ---------------------------------------------\n\n"

            "NAME:      sac2c\n"
            "VERSION:   %s\n"
            "PLATFORM:  %s\n"
            "\n"

            "BUILD:     %s\n"
            "BY USER:   %s\n"
            "ON HOST:   %s\n"
            "FOR OS:    %s\n"
            "\n\n",
            (version_id[0] == '\0') ? "???" : version_id,
            (target_platform[0] == '\0') ? "???" : target_platform,
            (build_date[0] == '\0') ? "???" : build_date,
            (build_user[0] == '\0') ? "???" : build_user,
            (build_host[0] == '\0') ? "???" : build_host,
            (build_os[0] == '\0') ? "???" : build_os);

    printf ("(c) Copyright 1994 - 2002 by\n\n"

            "  Christian-Albrechts-Universitaet zu Kiel\n"
            "  Institut fuer Informatik und Praktische Mathematik\n"
            "  Herman-Rodewald-Str.3\n"
            "  D-24118 Kiel\n"
            "  Germany\n\n");

    DBUG_VOID_RETURN;
}

void
copyright ()
{
    DBUG_ENTER ("copyright");

    printf ("\n          SAC - Single Assignment C\n"
            "    ---------------------------------------------\n\n"

            "    SAC COPYRIGHT NOTICE, LICENSE, AND DISCLAIMER\n\n"

            "(c) Copyright 1994 - 2000 by\n\n"

            "  Christian-Albrechts-Universitaet zu Kiel\n"
            "  Institut fuer Informatik und Praktische Mathematik\n"
            "  Herman-Rodewald-Str.3\n"
            "  D-24118 Kiel\n"
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
