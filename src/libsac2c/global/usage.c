/*
 * $Id$
 */

#include <stdio.h>
#include <limits.h>

#include "globals.h"
#include "build.h"
#include "dbug.h"
#include "usage.h"
#include "phase_options.h"

static void
PrintToolName (void)
{
    DBUG_ENTER ("PrintToolName");

    printf ("\n"
            "---------------------------------------------------------------------------"
            "\n"
            " SAC - Single Assignment C\n"
            "---------------------------------------------------------------------------"
            "\n"
            "\n"
            "\n"
            "NAME:          %s\n"
            "VERSION:       %s\n"
            "PLATFORM:      %s\n",
            global.toolname, (global.version_id[0] == '\0') ? "???" : global.version_id,
            (global.target_platform[0] == '\0') ? "???" : global.target_platform);

    DBUG_VOID_RETURN;
}

static void
PrintDescriptionSac2c (void)
{
    DBUG_ENTER ("PrintDescriptionSac2c");

    printf (
      "\n\nDESCRIPTION:\n\n"

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
      "    resulting SAC library is stored in the files <mod/class name>.a\n"
      "    and  <mod/class name>.so in the current directory.\n"
      "    In this case, the -o option may be used to specify a\n"
      "    different directory but not a different file name.\n");

    DBUG_VOID_RETURN;
}

static void
PrintDescriptionSac4c (void)
{
    DBUG_ENTER ("PrintDescriptionSac4c");

    printf (
      "\n\nDESCRIPTION:\n\n"

      "    The sac4c tools generates C headers and a corresponding wrapper library\n"
      "    for a given set of SAC libraries.\n"
      "    When generating the wrapper, sac4c will default to the two files\n"
      "    sacwrapper.h, libsacwrapper.so  and libsacwrapper.a, where the former\n"
      "    contains the C external declarations, and the later two files contain\n"
      "    the corresponding wrapper code. This default name can be overidden by\n"
      "    using the -o option as described below.\n\n"

      "    To ease linking of the final application, sac4c provides the flag\n"
      "    -ldflags, which prints appropriate linker flags that need to be used for\n"
      "    linking the wrapper library. To compiler code that uses the wrapper,\n"
      "    add the flags printed by using the -ccflags option to the c compiler\n"
      "    call.\n\n");

    DBUG_VOID_RETURN;
}

static void
PrintDescriptionSac2tex (void)
{
    DBUG_ENTER ("PrintDescriptionSac2tex");

    printf ("\n\nDESCRIPTION:\n\n"

            "    The sac2tex tool generates tex code from sac code\n"
            "    .\n\n");

    DBUG_VOID_RETURN;
}

static void
PrintSpecialOptions (void)
{
    DBUG_ENTER ("PrintSpecialOptions");

    printf (
      "\n\nSPECIAL OPTIONS:\n\n"

      "    -h              Display this helptext.\n"
      "    -help           Display this helptext.\n"
      "    -copyright      Display copyright/disclaimer.\n"
      "    -V              Display version identification.\n"
      "    -VV             Display verbose version identification.\n"
      "\n"
      "    -libstat        Print status information of the given SAC library file.\n"
      "    -prsc           Print resource settings.\n"
      "\n"
      "    -M              Detect dependencies from imported modules/classes and\n"
      "                    write them to stdout in a way suitable for the make\n"
      "                    utility.\n"
      "    -Mlib           Same as -M except that the output format is suitable\n"
      "                    for makefiles used by the standard library building\n"
      "                    process.\n"
      "\n"
      "    NOTE:\n"
      "    When called with one of these options, sac2c does not perform\n"
      "    any compilation steps.\n");

    DBUG_VOID_RETURN;
}

static void
PrintOptionsSac4c (void)
{
    DBUG_ENTER ("PrintOptionsSac4c");

    printf ("\n\nGENERAL OPTIONS:\n\n"

            "    -L <path>       Specify additional SAC library file path.\n"
            "    -I <path>       Specify additional SAC library source file path.\n"
            "    -E <path>       Specify additional C library file path.\n"
            "\n"
            "    -o <name>       Write external declarations to file <name>.h and\n"
            "                    wrapper library to file lib<name>.a/lib<name>/so.\n"
            "\n"
            "    -ldflags        Print linker flags to stdout\n"
            "    -ccflags        Print C compiler flags to stdout\n"
            "\n"
            "    -incdir         Specify the directory where the include file is "
            "written.\n"
            "    -libdir         Specify the directory where the library files are "
            "written.\n"
            "\n"
            "    -v <n>          Specify verbose level:\n"
            "                      0: error messages only\n"
            "                      1: error messages and warnings\n"
            "                      2: basic compile time information\n"
            "                      3: full compile time information\n"
            "                      4: even more compile time information\n"
            "                    (default: %d)\n",
            global.verbose_level);

    DBUG_VOID_RETURN;
}

static void
PrintGeneralOptions (void)
{
    DBUG_ENTER ("PrintGeneralOptions");

    printf ("\n\nGENERAL OPTIONS:\n\n"

            "    -D <var>        Set preprocessor variable <var>.\n"
            "    -D <var>=<val>  Set preprocessor variable <var> to <val>.\n"
            "    -cppI <path>    Specify path for preprocessor includes.\n"
            "\n"
            "    -L <path>       Specify additional SAC library file path.\n"
            "    -I <path>       Specify additional SAC library source file path.\n"
            "    -E <path>       Specify additional C library file path.\n"
            "\n"
            "    -o <name>       For compilation of programs:\n"
            "                      Write executable to specified file.\n"
            "                    For compilation of module/class implementations:\n"
            "                      Write library to specified directory.\n"
            "\n"
            "    -c              Generate C-file only; do not invoke C compiler.\n"
            "\n"
            "    -v <n>          Specify verbose level:\n"
            "                      0: error messages only\n"
            "                      1: error messages and warnings\n"
            "                      2: basic compile time information\n"
            "                      3: full compile time information\n"
            "                      4: even more compile time information\n"
            "                    (default: %d)\n",
            global.verbose_level);

    DBUG_VOID_RETURN;
}

static void
PrintBreakOptions (void)
{
    DBUG_ENTER ("PrintBreakOptions");

    printf ("\n\nBREAK OPTIONS:\n\n"

            "    Break options allow you to stop the compilation process\n"
            "    after a particular phase, subphase or cycle optimisation.\n"
            "    By default the intermediate programm will be printed, \n"
            "    but this behaviour may be influenced by the following\n"
            "    compiler options:\n"
            "\n"
            "    -noPAB          Deactivates printing after break.\n"
            "    -doPAB          Activates printing after break.\n"
            "\n"
            "    -b<spec>        Break after the compilation stage given\n"
            "                    by <spec>, where <spec> follows the pattern\n"
            "                    <phase>:<subphase>:<cyclephase>:<pass>.\n"
            "                    The first three are from the list of\n"
            "                    encodings below. The last one is a natural\n"
            "                    number. Alternatively, a number can be used\n"
            "                    for the phase, as well.\n");

    DBUG_VOID_RETURN;
}

static void
PrintBreakoptionSpecifierSac2c (void)
{
    DBUG_ENTER ("PrintBreakoptionSpecifierSac2c");

    printf ("\n\nBREAK OPTION SPECIFIERS:\n");

    PHOprintPhasesSac2c ();

    DBUG_VOID_RETURN;
}

static void
PrintBreakoptionSpecifierSac4c (void)
{
    DBUG_ENTER ("PrintBreakoptionSpecifierSac4c");

    printf ("\n\nBREAK OPTION SPECIFIERS:\n");

    PHOprintPhasesSac4c ();

    DBUG_VOID_RETURN;
}

static void
PrintTypeInferenceOptions (void)
{
    DBUG_ENTER ("PrintTypeInferenceOptions");

    printf ("\n\nTYPE INFERENCE OPTIONS:\n\n");
    printf ("    -specmode <strat>  Specify function specialization strategy:\n"
            "                         aks: try to infer all shapes statically,\n"
            "                         akd: try to infer all ranks statically,\n"
            "                         aud: do not specialize at all.\n"
            "                         (default: %s)\n\n",
            global.spec_mode_str[global.spec_mode]);

    printf ("    -maxspec <n>       Individual functions will be specialized at most <n> "
            "times.\n"
            "                         (default: %d)\n",
            global.maxspec);

    DBUG_VOID_RETURN;
}

static void
PrintOptimisationOptions (void)
{
    DBUG_ENTER ("PrintOptimisationOptions");
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
      "    -noreuse        Disable reuse inference in emm.\n"
      "\n"
      "    -iveo <n>       Enable or disable certain index vector optimisations\n"
      "                    <n> is a bitmask consisting of the following bits:\n"
      "                      1: enable the usage of withloop offsets where possible\n"
      "                      2: scalarise vect2offset operations where possible\n"
      "                      3: try to optimise computations on index vectors\n"
      "                      4: try to reuse offsets once computed\n"
      "                    The iveo option to for testing, and is to be removed.\n"
      "\n"
      "    -ssaiv          This option, if enabled, forces all with-loop generator\n"
      "                    variables to be unique (SSA form). This is\n"
      "                    a prerequisite for MINVAL/MAXVAL work.\n"
      "\n"
      "                    If disabled (the default setting), all with-loop\n"
      "                    generators use the same index vector variables.\n"
      "\n"
      "    -no <opt>       Disable optimization technique <opt>.\n"
      "\n"
      "    -do <opt>       Enable optimization technique <opt>.\n"
      "\n\n"
      "    The following optimization techniques are currently supported:\n"
      "\n"
      "     (A leading * identifies optimization enabled by default.)\n"
      "\n");

#ifdef PRODUCTION
#define OPTIMIZE(str, abbr, devl, prod, name)                                            \
    printf ("      %s %-8s%s\n", prod ? "*" : " ", str, name);
#else
#define OPTIMIZE(str, abbr, devl, prod, name)                                            \
    printf ("      %s %-8s%s\n", devl ? "*" : " ", str, name);
#endif
#include "optimize.mac"

    printf (
      "\n"
      "    NOTE:\n"
      "     -no opt     disables all optimizations at once.\n"
      "     -do opt     enables all optimizations at once.\n"
      "\n"
      "    NOTE:\n"
      "     Upper case letters may be used to indicate optimization techniques.\n"
      "\n"
      "    NOTE:\n"
      "     Command line arguments are evaluated from left to right, i.e.,\n"
      "     \"-no opt -do inl\" disables all optimizations except for function "
      "inlining.\n"
      "\n"
      "    NOTE:\n"
      "     Some of the optimization techniques are parameterized by additional side\n"
      "     conditions. They are controlled by the following options:\n"
      "\n");

    printf (
      "    -maxoptcyc <n>  Repeat optimization cycle max <n> times. After <n> cycles\n"
      "                    all optimisations except for type upgrade and function "
      "dispatch\n"
      "                    are disabled.\n"
      "                      (default: %d)\n\n",
      global.max_optcycles);

    printf (
      "    -maxrecinl <n>  Inline recursive function applications at most <n> times.\n"
      "                      (default: %d)\n\n",
      global.max_recursive_inlining);

    printf ("    -maxlur <n>     Unroll loops having at most <n> iterations.\n"
            "                      (default: %d)\n\n",
            global.unrnum);

    printf (
      "    -maxwlur <n>    Unroll with-loops with at most <n> elements generator set\n"
      "                    size.\n"
      "                      (default: %d)\n\n",
      global.wlunrnum);

    printf ("    -maxae <n>      Try to eliminate arrays with at most <n> elements.\n"
            "                      (default: %d)\n\n",
            global.minarray);

    printf (
      "    -initmheap <n>  At program startup initially request <n> KB of heap memory\n"
      "                    for master thread.\n"
      "                      (default: %d)\n\n",
      global.initial_master_heapsize);

    printf (
      "    -initwheap <n>  At program startup initially request <n> KB of heap memory\n"
      "                    for each worker thread.\n"
      "                      (default: %d)\n\n",
      global.initial_worker_heapsize);

    printf (
      "    -inituheap <n>  At program startup initially request <n> KB of heap memory\n"
      "                    for usage by all threads.\n"
      "                      (default: %d)\n\n",
      global.initial_unified_heapsize);

    printf (
      "    -aplimit <n>    Set the array padding resource allocation overhead limit\n"
      "                    to <n> %%.\n"
      "                      (default: %d)\n\n",
      global.padding_overhead_limit);

    printf (
      "    -apdiag         Print additional information for array padding to file\n"
      "                    \"<outfile>.ap\", where <outfile> is the name specified via\n"
      "                    the \"-o\" option.\n\n");

    printf (
      "    -apdiagsize <n> Limit the amount of information written to the diagnostic\n"
      "                    output file created via the -apdiag option to approximately\n"
      "                    <n> lines.\n"
      "                      (default: %d)\n\n",
      global.apdiag_limit);

    printf ("    -wls_aggressive Set WLS optimization level to aggressive.\n"
            "                    WARNING:\n"
            "                    Aggressive with-loop scalarization may have the "
            "opposite\n"
            "                    effect as with-loop invariant removal and cause "
            "duplication\n"
            "                    of code execution.\n\n"
            "    -maxwls         Set the maximum number of inner with-loop elements for "
            "which\n"
            "                    aggressive behaviour will be used even if "
            "-wls_aggressive is\n"
            "                    not given. (default: %d)\n\n",
            global.maxwls);

    printf ("    -nofoldfusion   Eliminate fusion of with-loops with fold operator.\n\n"
            "    -maxnewgens <n> Set the maximum number of new created generators while\n"
            "                    intersection of generatorsets from two with-loops in\n"
            "                    with-loop fusion to <n>.\n"
            "                      (default: %d)\n\n",
            global.max_newgens);

    printf ("    -sigspec <strat>   Specify strategy for specialization of function "
            "sigantures:\n"
            "                          akv: try to infer all values statically,\n"
            "                          aks: try to infer all shapes statically,\n"
            "                          akd: try to infer all ranks statically,\n"
            "                          aud: do not specialize at all.\n"
            "                          (default: %s)\n",
            global.sigspec_mode_str[global.sigspec_mode]);

    DBUG_VOID_RETURN;
}

static void
PrintMultithreadOptions (void)
{
    DBUG_ENTER ("PrintMultithreadOptions");

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
            "    -mtmode <n>     Enable a explicit organization scheme for "
            "multi-threaded program\n"
            "                    execution.\n"
            "                    Legal values:\n"
            "                      1: with thread creation/termination\n"
            "                      2: with start/stop barriers\n"
            "                      3: with magical new techniques, WARNING: UNDER "
            "CONSTRUCTION!!!\n"
            "                      (default: %d)\n"
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
            "    -nofoldparallel Disable parallelization of fold with-loops.\n"
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
            (int)MT_startstop, global.max_threads, global.max_sync_fold,
            global.min_parallel_size, global.max_replication_size);

    DBUG_VOID_RETURN;
}

static void
PrintBackendOptions (void)
{
    DBUG_ENTER ("PrintBackendOptions");

    printf ("\n\nBACKEND OPTIONS:\n\n"

            "    -minarrayrep <class>\n"
            "                    Specify the minimum array representation class used:\n"
            "                      s: use all (SCL, AKS, AKD, AUD) representations,\n"
            "                      d: use SCL, AKD, AUD representations only,\n"
            "                      +: use SCL, AUD representations only,\n"
            "                      *: use AUD representation only.\n"
            "                    (default: s)\n");

    DBUG_VOID_RETURN;
}

static void
PrintGeneralDebugOptions (void)
{
    DBUG_ENTER ("PrintGeneralDebugOptions");

    printf (
      "\n\nGENERAL DEBUG OPTIONS:\n\n"

      "    -d nocleanup    Do not remove temporary files and directories.\n"
      "    -d syscall      Show all system calls during compilation.\n"
      "    -d cccall       Generate shell script \".sac2c\" that contains C compiler\n"
      "                    invocation.\n"
      "                    This implies option \"-d nocleanup\".\n");

    DBUG_VOID_RETURN;
}

#ifndef DBUG_OFF

static void
PrintInternalDBUGOptions (void)
{
    DBUG_ENTER ("PrintInternalDBUGOptions");

    printf (
      "\n\nINTERNAL OPTIONS FOR FRED FISH'S DBUG:\n\n"
      "    -# t            Display trace information.\n"
      "                    Each function entry and exit during program execution is\n"
      "                    printed on the screen.\n"
      "\n"
      "    -# d            Display debug output information.\n"
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
      "                    All kinds of phases can be specified using a syntax\n"
      "                    analogous to that of the -b option.\n");

    DBUG_VOID_RETURN;
}

static void
PrintInternalDebugOptions (void)
{
    DBUG_ENTER ("PrintInternalDebugOptions");

    printf (
      "\n\nINTERNAL DEBUG OPTIONS:\n\n"
      "    -d treecheck    Check syntax tree for consistency with xml specification. \n"
      "    -d memcheck     Check syntax tree for memory consistency.\n"
      "    -d sancheck     Check syntax tree for structural consistency. \n"
      "    -d nolacinline  Do not inline loop and conditional functions.\n"
      "    -d efence       Link executable with ElectricFence (malloc debugger).\n");

    DBUG_VOID_RETURN;
}

#endif /* DBUG_OFF */

static void
PrintRuntimeCheckOptions (void)
{
    DBUG_ENTER ("PrintRuntimeCheckOptions");

    printf ("\n\nRUNTIME CHECK OPTIONS:\n\n"

            "    -ecc            Insert explicit conformity checks at compile time.\n"
            "\n"
            "    -check [atbmeh]+\n"
            "                    Incorporate runtime checks into executable program.\n"
            "                    The following flags are supported:\n"
            "                      a: Incorporate all available runtime checks.\n"
            "                      c: Perform conformity checks.\n"
            "                      t: Check assignments for type violations.\n"
            "                      b: Check array accesses for boundary violations.\n"
            "                      m: Check success of memory allocations.\n"
            "                      e: Check errno variable upon applications of\n"
            "                         external functions.\n"
            "                      h: Use diagnostic heap manager.\n");

    DBUG_VOID_RETURN;
}

static void
PrintRuntimeTraceOptions (void)
{
    DBUG_ENTER ("PrintRuntimeTraceOptions");

    printf (
      "\n\nRUNTIME TRACE OPTIONS:\n\n"

      "    -trace [amrfpwstc]+\n"
      "                    Incorporate trace output generation into executable program.\n"
      "                    The following flags are supported:\n"
      "                      a: Trace all (same as mrfpowt).\n"
      "                      m: Trace memory operations.\n"
      "                      r: Trace reference counting operations.\n"
      "                      f: Trace user-defined function calls.\n"
      "                      p: Trace primitive function calls.\n"
      "                      w: Trace with-loop execution.\n"
      "                      s: Trace array accesses.\n"
      "                      t: Trace multi-threading specific operations.\n"
      "                      c: Trace runtime enviroment init/exit when\n"
      "                         using SAC libraries in C programs.\n"
      "\n"
      "    -utrace\n"
      "                    Introduce user tracing calls.");

    DBUG_VOID_RETURN;
}

static void
PrintPrintingOptions (void)
{
    DBUG_ENTER ("PrintPrintingOptions");

    printf (
      "\n\nPRINTING OPTIONS:\n\n"

      "    -print [adv]+\n"
      "               Add internal AST information as comments to the program output.\n"
      "               The following flags are supported:\n"
      "                 a: Print all (same as dv).\n"
      "                 d: Print specialization demand.\n"
      "                 v: Print avis information.\n");

    DBUG_VOID_RETURN;
}

static void
PrintRuntimeProfilingOptions (void)
{
    DBUG_ENTER ("PrintRuntimeProfilingOptions");

    printf (
      "\n\nRUNTIME PROFILING OPTIONS:\n\n"

      "    -profile [afilw]+\n"
      "                    Incorporate profiling analysis into executable program.\n"
      "                      a: Analyse all (same as filw).\n"
      "                      f: Analyse time spent in non-inline functions.\n"
      "                      i: Analyse time spent in inline functions.\n"
      "                      l: Analyse time spent in library functions.\n"
      "                      w: Analyse time spent in with-loops.\n");

    DBUG_VOID_RETURN;
}

static void
PrintCacheSimulationOptions (void)
{
    DBUG_ENTER ("PrintCacheSimulationOptions");

    printf (
      "\n\nCACHE SIMULATION OPTIONS:\n\n"

      "    -cs             Enable runtime cache simulation.\n"
      "\n"
      "    -csdefaults [sagbifp]+\n"
      "                    This option sets default parameters for cache simulation.\n"
      "                    These settings may be overridden when starting the analysis\n"
      "                    of an application program:\n"
      "                      s: simple cache simulation,\n"
      "                      a: advanced cache simulation,\n"
      "                      g: global cache simulation,\n"
      "                      b: cache simulation on selected blocks,\n"
      "                      i: immediate analysis of memory access data,\n"
      "                      f: storage of memory access data in file,\n"
      "                      p: piping of memory access data to concurrently running\n"
      "                         analyser process.\n"
      "                    The default simulation parameters are \"sgp\".\n"
      "\n"
      "    -cshost <name>  This option specifies the host machine to run the additional\n"
      "                    analyser process on when doing piped cache simulation.\n"
      "                    This is very useful for single processor machines because\n"
      "                    the rather limited buffer size of the pipe determines the\n"
      "                    synchronisation distance of the two processes, i.e. the\n"
      "                    application process and the analysis process. This results\n"
      "                    in very frequent context switches when both processes are\n"
      "                    run on the same processor, and consequently, degrades the\n"
      "                    performance by orders of magnitude. So, when doing piped\n"
      "                    cache simulation always be sure to do so either on a\n"
      "                    multiprocessor or specify a different machine to run the\n"
      "                    analyser process on. However, this only defines a default\n"
      "                    which may be overridden by using this option when starting\n"
      "                    the compiled application program.\n"
      "\n"
      "    -csfile <name>  This option specifies a default file where to write the\n"
      "                    memory access trace when performing cache simulation via\n"
      "                    a file. This default may be overridden by using this option\n"
      "                    when starting the compiled application program.\n"
      "                    The general default name is \"<executable_name>.cs\".\n"
      "\n"
      "    -csdir <name>   This option specifies a default directory where to write\n"
      "                    the memory access trace file when performing cache\n"
      "                    simulation via a file. This default may be overridden by\n"
      "                    using this option when starting the compiled application\n"
      "                    program.\n"
      "                    The general default directory is the tmp directory specified\n"
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
      "    to write memory accesses to a file, beware that even for small programs\n"
      "    to be analysed the amount of data may be quite large. However, once a\n"
      "    memory trace file exists, it can be used to simulate different cache\n"
      "    configurations without repeatedly running the application program\n"
      "    itself. The simulation tool for memory access trace files is called\n"
      "    'csima' and resides in the bin directory of your SAC installation.\n"
      "\n"
      "    These default cache simulation parameters may be overridden when\n"
      "    invoking the application program to be analysed by using the generic\n"
      "    command line option\n"
      "        -cs [sagbifp]+\n"
      "    where the various flags have the same meaning as described for the\n"
      "    \"-csdefaults\" compiler option.\n"
      "\n"
      "    Cache parameters for up to 3 levels of caches may be provided as target\n"
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
      "        a: write around\n");

    DBUG_VOID_RETURN;
}

static void
PrintLibraryOptions (void)
{
    DBUG_ENTER ("PrintLibraryOptions");

    printf ("\n\nLIBRARY OPTIONS:\n\n"

            "    -linksetsize <n> Specify how many compiled C functions are stored "
            "within\n"
            "                     a single C source file for further compilation and "
            "linking.\n"
            "                     A large number here means that potentially many "
            "functions\n"
            "                     need to be linked to an executable that are actually "
            "never\n"
            "                     called. However, setting the linksetsize to 1 "
            "considerably\n"
            "                     slows down the compilation of large SAC "
            "modules/classes\n"
            "                     (default: %d)\n"
            "\n"
            "                     NOTE:\n"
            "                     A linksetsize of 0 means all functions are stored in "
            "a\n"
            "                     a single file.\n"
            "\n"
            "    -genlib <lang>   Specify library format when compiling SAC "
            "module/class\n"
            "                     implementations.\n"
            "                     Supported values for <lang> are:\n"
            "                       sac: Generate SAC library file (default).\n"
            "                         c: Generate C object and header files.\n"
            "\n"
            "                     NOTE:\n"
            "                     Be careful to use same options for privat heap "
            "management\n"
            "                     (PHM) and profiling for compilation of all "
            "modules/classes\n"
            "                     you are going to link together to a single "
            "executable.\n"
            "\n"
            "                     NOTE:\n"
            "                     Multithreading is not yet available for C libraries.\n"
            "\n"
            "    -noprelude       Do not load the standard prelude library `%s'.\n",
            global.linksetsize == INT_MAX ? 0 : global.linksetsize, global.preludename);

    DBUG_VOID_RETURN;
}

static void
PrintCCompilerOptions (void)
{
    DBUG_ENTER ("PrintCCompilerOptions");

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
            global.cc_optimize);

    DBUG_VOID_RETURN;
}

static void
PrintCustomisationOptions (void)
{
    DBUG_ENTER ("PrintCustomisationOptions");

    printf ("\n\nCUSTOMIZATION OPTIONS:\n\n"

            "    -target <name>  Specify a particular compilation target.\n"
            "                    Compilation targets are used to customize sac2c for\n"
            "                    various target architectures, operating systems, and C\n"
            "                    compilers.\n"
            "                    The target description is either read from the\n"
            "                    installation specific file $SACBASE/runtime/sac2crc or\n"
            "                    from a file named .sac2crc within the user's home\n"
            "                    directory.\n");

    DBUG_VOID_RETURN;
}

static void
PrintEnvironmentVariables (void)
{
    DBUG_ENTER ("PrintEnvironmentVariables");

    printf (
      "\n\nENVIRONMENT VARIABLES:\n\n"

      "    The following environment variables are used by the SAC compiler suite:\n"
      "\n"
      "    SACBASE         Base directory of SAC standard lib installation.\n"
      "    SAC2CBASE       Base directory of SAC installation.\n"
      "\n");

    DBUG_VOID_RETURN;
}

static void
PrintAuthors (void)
{
    DBUG_ENTER ("PrintAuthors");

    printf ("\n\nAUTHORS:\n\n"

            "    The following people contributed their time and mind to create the\n"
            "    SAC compiler suite (roughly in order of entering the project):\n"
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
            "      Stephan Herhut\n"
            "      Karsten Hinckfuss\n"
            "      Steffen Kuthe\n"
            "      Jan-Henrik Baumgarten\n"
            "      Robert Bernecky\n"
            "      Theo van Klaveren\n"
            "      Florian Buether\n"
            "      Torben Gerhards\n");

    DBUG_VOID_RETURN;
}

static void
PrintContact (void)
{
    DBUG_ENTER ("PrintContact");

    printf ("\n\nCONTACT:\n\n"

            "    WWW:    http://www.sac-home.org/\n"
            "    E-Mail: info@sac-home.org\n");

    printf ("\n\nBUGS:\n\n"

            "    Bugs??  We????\n"
            "\n"
            "    SAC is a research project!\n"
            "\n"
            "    SAC tools are platforms for scientific research rather than\n"
            "    \"products\" for end users. Although we try to do our very best,\n"
            "    you may well run into a compiler bug. So, we are happy to receive\n"
            "    your bug reports (Well, not really \"happy\", but ...).\n");

    DBUG_VOID_RETURN;
}

void
USGprintUsage ()
{
    DBUG_ENTER ("USGprintUsage");

    PrintToolName ();

    switch (global.tool) {
    case TOOL_sac2c:
        PrintDescriptionSac2c ();
        PrintSpecialOptions ();
        PrintGeneralOptions ();
        PrintBreakOptions ();
        PrintBreakoptionSpecifierSac2c ();
        PrintPrintingOptions ();
        PrintTypeInferenceOptions ();
        PrintOptimisationOptions ();
        PrintMultithreadOptions ();
        PrintBackendOptions ();
        PrintGeneralDebugOptions ();
#ifndef DBUG_OFF
        PrintInternalDebugOptions ();
        PrintInternalDBUGOptions ();
#endif /* DBUG_OFF */
        PrintRuntimeCheckOptions ();
        PrintRuntimeTraceOptions ();
        PrintRuntimeProfilingOptions ();
        PrintCacheSimulationOptions ();
        PrintLibraryOptions ();
        PrintCCompilerOptions ();
        PrintCustomisationOptions ();
        break;
    case TOOL_sac4c:
        PrintDescriptionSac4c ();
        PrintOptionsSac4c ();
        PrintGeneralDebugOptions ();
#ifndef DBUG_OFF
        PrintInternalDBUGOptions ();
#endif /* DBUG_OFF */
        PrintBreakoptionSpecifierSac4c ();
        PrintCCompilerOptions ();
        break;
    case TOOL_sac2tex:
        PrintDescriptionSac2tex ();
        PrintGeneralOptions ();
        PrintGeneralDebugOptions ();
#ifndef DBUG_OFF
        PrintInternalDBUGOptions ();
#endif /* DBUG_OFF */
        break;
    }

    PrintEnvironmentVariables ();
    PrintAuthors ();
    PrintContact ();

    printf ("\n\n");

    DBUG_VOID_RETURN;
}

void
USGprintVersion ()
{
    DBUG_ENTER ("USGprintVersion");

    printf ("%s %s\n %s rev %s %s\n (%s by %s)\n", global.toolname,
            (global.version_id[0] == '\0') ? "???" : global.version_id,
            (build_style[0] == '\0') ? "" : build_style,
            (build_rev[0] == '\0') ? "???" : build_rev,
            (global.target_platform[0] == '\0') ? "???" : global.target_platform,
            (build_date[0] == '\0') ? "???" : build_date,
            (build_user[0] == '\0') ? "???" : build_user);

    DBUG_VOID_RETURN;
}

void
USGprintVersionVerbose ()
{
    DBUG_ENTER ("USGprintVerboseVersion");

    PrintToolName ();

    printf ("\n"
            "BUILD:         %s (%s)\n"
            "AT DATE:       %s\n"
            "BY USER:       %s\n"
            "ON HOST:       %s\n"
            "\n"

            "\n",
            (build_rev[0] == '\0') ? "???" : build_rev,
            (build_style[0] == '\0') ? "" : build_style,
            (build_date[0] == '\0') ? "???" : build_date,
            (build_user[0] == '\0') ? "???" : build_user,
            (build_host[0] == '\0') ? "???" : build_host);

    printf ("(c) Copyright 1994-2007 by\n\n"

            "  SAC Development Team\n\n"

            "  http://www.sac-home.org\n"
            "  email:info@sac-home.org\n\n");

    DBUG_VOID_RETURN;
}

void
USGprintCopyright ()
{
    DBUG_ENTER ("USGprintCopyright");

    printf (
      "\n"
      "---------------------------------------------------------------------------\n"
      " SAC - Single Assignment C\n"
      "---------------------------------------------------------------------------\n"
      "\n"
      "COPYRIGHT NOTICE, LICENSE AND DISCLAIMER\n"
      "\n"
      "(c) Copyright 1994 - 2007 by\n"
      "\n"
      "  SAC Development Team\n"
      "\n"
      "  http://www.sac-home.org\n"
      "  email:info@sac-home.org\n"
      "\n"
      "---------------------------------------------------------------------------\n"
      "\n");

    printf (
      "The SAC compiler, the SAC standard library and all accompanying\n"
      "software and documentation (in the following named this software)\n"
      "is developed by the SAC Development Team (in the following named\n"
      "the developer) which reserves all rights on this software.\n"
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
      "The developer disclaims all warranties with regard to this software,\n"
      "including all implied warranties of merchantability and fitness.  In no\n"
      "event shall the developer be liable for any special, indirect or\n"
      "consequential damages or any damages whatsoever resulting from loss of\n"
      "use, data, or profits, whether in an action of contract, negligence, or\n"
      "other tortuous action, arising out of or in connection with the use or\n"
      "performance of this software. The entire risk as to the quality and\n"
      "performance of this software is with you. Should this software prove\n"
      "defective, you assume the cost of all servicing, repair, or correction.\n"
      "\n"
      "---------------------------------------------------------------------------\n"
      "\n");

    DBUG_VOID_RETURN;
}
