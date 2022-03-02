#include <stdio.h>
#include <limits.h>

#include "globals.h"
#include "build.h"

#define DBUG_PREFIX "USG"
#include "debug.h"

#include "usage.h"
#include "phase_options.h"
#include "config.h"
#include "sacdirs.h"

static void
PrintToolName (void)
{
    DBUG_ENTER ();

    printf ("\n"
            "---------------------------------------------------------------------------"
            "\n"
            " SAC - Single Assignment C\n"
            "---------------------------------------------------------------------------"
            "\n"
            "\n"
            "Name:                    %s\n"
            "Version:                 %s\n",
            global.toolname, (global.version_id[0] == '\0') ? "???" : global.version_id);

    DBUG_RETURN ();
}

static void
PrintDescriptionSac2c (void)
{
    DBUG_ENTER ();

    if (global.verbose_help) {
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
    } else {
        printf ( "\n\nDESCRIPTION: sac2c transforms SAC source code into executable programs.\n");
    }

    DBUG_RETURN ();
}

static void
PrintDescriptionSac4c (void)
{
    DBUG_ENTER ();

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
      "    call.\n\n"

      "    With the -fortran option, sac4c will generate a Fortran module file\n"
      "    in addition to the wrapper library to call SAC functions from a Fortran\n"
      "    context.\n\n");

    DBUG_RETURN ();
}

static void
PrintDescriptionSac2tex (void)
{
    DBUG_ENTER ();

    printf ("\n\nDESCRIPTION:\n\n"

            "    The sac2tex tool generates tex code from sac code\n"
            "    .\n\n");

    DBUG_RETURN ();
}

static void
PrintFeatureSet (void)
{
    DBUG_ENTER ();

    printf ("\n\nINSTALLATION-SPECIFIC FEATURE SET:\n\n"
            " - default path to sac2crc: %s\n"
            " - default path to libsac2c: %s\n\n",
            global.global_sac2crc_location == NULL ? "???"
                                                   : global.global_sac2crc_location,
            global.config.tree_outputdir == NULL ? "???" : global.config.tree_outputdir);

    DBUG_RETURN ();
}

static void
PrintSpecialOptions (void)
{
    DBUG_ENTER ();

    printf (
      "\n\nSPECIAL OPTIONS:\n\n");

    if (global.verbose_help) {
        printf (
      "    -h              Display very condensed helptext.\n"
      "    -help           Display this helptext.\n");
    } else {
        printf (
      "    -h              Display this helptext.\n"
      "    -help           Display much more helptext.\n");
    }

    printf (
      "    -copyright      Display copyright/disclaimer.\n"
      "    -V              Display version identification.\n"
      "    -VV             Display verbose version identification.\n"
      "\n"
      "    -libstat        Print status information of the given SAC library file.\n"
      "    -prsc           Print resource settings.\n"
      "\n"
      "    -M              Makes sac2c print a list of dependencies of the given\n"
      "                    file and exit.  Dependencies are objects and libraries that\n"
      "                    will be needed to compile the given file: use/imports,\n"
      "                    arguments of linkobj and linkwith pragmas.\n"
      "\n"
      "    -C <name>       Print out a configuration parameter\n");
    if (global.verbose_help) {
        printf (
      "\n"
      "    NOTE:\n"
      "    When called with one of these options, sac2c does not perform\n"
      "    any compilation steps.\n");
    }

    DBUG_RETURN ();
}

static void
PrintOptionsSac4c (void)
{
    DBUG_ENTER ();

    printf ("\n\nGENERAL OPTIONS:\n\n"

            "    -I <path>       Specify additional SAC library source file path.\n"
            "    -L <path>       Specify additional SAC library module path.\n"
            "    -T <path>       Specify additional SAC library tree path.\n"
            "    -E <path>       Specify additional C library file path.\n"
            "    -Xp <flags>     Extra flags to pass to the C preprocessor for "
            "program/module code.\n"
            "    -Xc <flags>     Extra flags to pass to the C compiler for "
            "program/module code.\n"
            "    -Xl <flags>     Extra flags to pass to the C linker for program/module "
            "code.\n"
            "    -Xtc <flags>    Extra flags to pass to the C compiler for tree code.\n"
            "    -Xtl <flags>    Extra flags to pass to the C linker for tree code.\n"
            "\n"
            "    -o <name>       Write external declarations to file <name>.h and\n"
            "                    wrapper library to file lib<name>.EXT.\n"
            "    -t <target>     Specify the target of the C library. Can also be "
            "specified with\n"
            "                    -target <target>.\n"
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
            "                    (default: %d)\n"
            "\n"
            "    -fortran        generate Fortran module\n",
            global.verbose_level);

    DBUG_RETURN ();
}

static void
PrintGeneralOptions (void)
{
    DBUG_ENTER ();

    printf ("\n\nGENERAL OPTIONS:\n\n"

            "    -D <var>        Set preprocessor variable <var>.\n"
            "    -D <var>=<val>  Set preprocessor variable <var> to <val>.\n"
            "    -cppI <path>    Specify path for preprocessor includes.\n"
            "\n"
            "    -I <path>       Specify additional SAC library source file path.\n"
            "    -E <path>       Specify additional C library file path.\n"
            "    -L <path>       Specify additional SAC library module path.\n"
            "    -T <path>       Specify additional SAC library tree path.\n"
            "    -Xp <flags>     Extra flags to pass to the C preprocessor for "
            "program/module code.\n"
            "    -Xc <flags>     Extra flags to pass to the C compiler for "
            "program/module code.\n"
            "    -Xl <flags>     Extra flags to pass to the C linker for program/module "
            "code.\n"
            "    -Xtc <flags>    Extra flags to pass to the C compiler for tree code.\n"
            "    -Xtl <flags>    Extra flags to pass to the C linker for tree code.\n"
            "\n"
            "    -o <name>       For compilation of programs:\n"
            "                      Place executable code in the specified file.\n"
            "                    For compilation of module/class implementations:\n"
            "                      Write module tree to the specified directory.\n"
            "    -olib <name>    For compilation of module/class implementations:\n"
            "                      Write module code to the specified directory.\n"
            "                      (if not specified, defaults to same as -o)\n"
            "    -install        Exclusively for compilation of module/class\n"
            "                    implementations:\n"
            "                      Write to the central repository.\n"
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

    DBUG_RETURN ();
}

static void
PrintBreakOptions (void)
{
    DBUG_ENTER ();

    printf ("\n\nBREAK OPTIONS:\n\n");

    if (global.verbose_help) {
        printf (
            "    Break options allow you to stop the compilation process\n"
            "    after a particular phase, subphase or cycle optimisation.\n"
            "    By default the intermediate programm will be printed. You\n"
            "    can visualize the syntax tree as well, and a new png file \n"
            "    will be created. But this behaviour may be influenced by \n"
            "    the following compiler options:\n "
            "\n"
            "    -noPAB          Deactivates printing after break.\n"
            "    -doPAB          Activates printing after break.\n"
            "    -doVAB          Activates visualization of program after break.\n"
            "                    Select different categories of funcion to print.\n"
            "    -fVAB <format>  Output visualization in <format>. Default is PNG.\n"
            "                    <format> must be supported by your dot installation,\n"
            "                    run \"dot -Tv\" for a list of available formats.\n"

            "\n");
    }
    printf (
            "    -b<spec>        Break after the compilation stage given\n"
            "                    by <spec>, where <spec> follows the pattern\n"
            "                    <phase>:<subphase>:<cyclephase>:<pass>.\n"
            "                    The first three are from the list of\n"
            "                    encodings below. The last one is a natural\n"
            "                    number. Alternatively, a number can be used\n"
            "                    for the phase, as well.\n");

    DBUG_RETURN ();
}

static void
PrintBreakoptionSpecifierSac2c (void)
{
    DBUG_ENTER ();

    printf ("\n\nBREAK OPTION SPECIFIERS: %s\n", (global.verbose_help ?
                                                  "(all, * indicates enabled phases)" :
                                                  "(only enabled phases)"));

    PHOprintPhasesSac2c ();

    DBUG_RETURN ();
}

static void
PrintBreakoptionSpecifierSac4c (void)
{
    DBUG_ENTER ();

    printf ("\n\nBREAK OPTION SPECIFIERS:\n");

    PHOprintPhasesSac4c ();

    DBUG_RETURN ();
}

static void
PrintTypeInferenceOptions (void)
{
    DBUG_ENTER ();

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

    DBUG_RETURN ();
}

static void
PrintOptimisationOptions (void)
{
    DBUG_ENTER ();
    printf (
      "\n\nOPTIMIZATION OPTIONS:\n\n"
      "    -enforceIEEE    Treat floating point arithmetic as defined in the IEEE-754\n"
      "                    standard. In particular, this means\n"
      "                      - disable some algebraic optimizations,\n"
      "                      - disable segmentation and tiling of fold-with-loops,\n"
      "                      - disable parallel execution of fold-with-loops.\n"
      "                    Currently implemented for:\n"
      "                      - associative law optimization,\n"
      "                      - distributive law optimization,\n"
      "                      - comparison to zero conversion,\n"
      "                      - segmentation and tiling of fold-with-loops.\n"
      "                    This option also inhibits these optimisations for all\n"
      "                    integer operations. The finiteness of the integer\n"
      "                    numbers can lead to changes in the semantics due to\n"
      "                    overflows or underflows that may occur in one version\n"
      "                    but not the other\n" );
    if (global.verbose_help) {
        printf (
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
      "    -dossawl        This option, if enabled, forces all with-loop generator\n"
      "                    and with-loop partition variables, to be unique (SSA form)."
      "\n"
      "                    If disabled (the default setting), all with-loop\n"
      "                    generators use the same index vector variables,\n"
      "                    and no attempt is made to enforce SSA form."
      "\n"
      "    -extrema        This option, if enabled, allows the compiler to\n"
      "                    use optimizations based on index variable extrema;\n"
      "                    i.e., the minimum and maximum value that index variables\n"
      "                    may take on. This option is deprecated.\n"
      "\n");
    }
    printf (
      "    -no <opt>       Disable optimization technique <opt>.\n"
      "\n"
      "    -do <opt>       Enable optimization technique <opt>.\n"
      "\n\n"
      "    The following optimization techniques are currently supported:\n"
      "\n"
      "     (A leading * identifies enabled optimizations, |* indicates enabled by default.)\n"
      "\n");

#define DELAYPHM global.config.use_phm_api
#ifdef PRODUCTION
#define OPTIMIZE(str, abbr, devl, prod, name)                                            \
    printf ("      %s%s %-8s%s\n", global.optimize.do ## abbr ? "*" : " ",               \
                                   prod ? "|*" : "  ", str, name);
#else
#define OPTIMIZE(str, abbr, devl, prod, name)                                            \
    printf ("      %s%s %-8s%s\n", global.optimize.do ## abbr ? "*" : " ",               \
                                   devl ? "|*" : "  ", str, name);
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

    printf (
      "    -maxprfur <n>   Unroll built-in vector operations with at most <n> elements\n"
      "                    generator set size.\n"
      "                      (default: %d)\n\n",
      global.prfunrnum);

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

    printf ("    -dowls_aggressive Set WLS optimization level to aggressive.\n"
            "                    WARNING:\n"
            "                    Aggressive with-loop scalarization may have the "
            "opposite\n"
            "                    effect as with-loop invariant removal and cause "
            "duplication\n"
            "                    of code execution.\n\n"
            "    -maxwls         Set the maximum number of inner with-loop elements for "
            "which\n"
            "                    aggressive behaviour will be used even if "
            "-dowls_aggressive is\n"
            "                    not given. (default: %d)\n\n",
            global.maxwls);

    printf (
      "    -dofoldfusion   Enable fusion of with-loops with fold operator (default).\n"
      "    -nofoldfusion   Disable fusion of with-loops with fold operator.\n\n"
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

    printf ("    -force_naive    Make wlt always run in naive mode\n\n");

    DBUG_RETURN ();
}

static void
PrintMutcOptions (void)
{
    DBUG_ENTER ();
    /*      12345678901234567890123456789012345678901234567890123456789012345678901234567890*/
    printf (
      "\n\nMUTC OPTIONS:\n\n"
      "    -mutc_fun_threads                Convert all functions to thread functions\n"
      "                                     and use singleton creates\n\n"
      "    -mutc_thread_mem                 Use thread local memory every where not\n"
      "                                     global memory\n\n"
      "    -mutc_disable_thread_mem         Disable creation of thread local memory\n"
      "    -mutc_benchmark                  Enable mutc benchmarking support\n\n"
      "    -mutc_static_resource_management Staticly manage resources\n\n"
      "    -mutc_force_block_size <n>       Force the block size to <n> for all\n"
      "                                     creates\n"
      "    -mutc_force_spawn_flags <s>      Force the flags to <s> for all spawns\n"
      "    -mutc_distribute <mode>          Select a mode for distributing threads\n"
      "                                     across cores. Possible modes are:\n\n"
      "                                     toplevel : only distribute the top-evel\n"
      "                                                create of a with3 nesting\n"
      "                                     bounded :  distribute threads globally\n"
      "                                                until at least <n> threads\n"
      "                                                have been distributed. <n> is\n"
      "                                                specified using the\n"
      "                                                -mutc_distribute_arg option\n\n"
      "    -mutc_distribute_arg             numerical argument for distribution "
      "modes.\n\n"
      "    -mutc_unroll <n>                 Maximum with3 size to unroll(1)\n\n"
      "    -mutc_suballoc_desc_one_level_up Allocate discs one level higher than they\n"
      "                                     are used\n\n"
      "    -mutc_rc_places <n>              Number of exclusive places to be used for\n"
      "                                     reference counting operations(1)\n\n"
      "    -mutc_rc_indirect                Perform reference counting operations using\n"
      "                                     wrapper functions\n\n"
      "    -mutc_seq_data_parallel          sequentialised data parallel code\n\n");

    DBUG_RETURN ();
}

static void
PrintDistMemOptions (void)
{
    DBUG_ENTER ();

    printf ("\n\nDISTRIBUTED MEMORY BACKEND OPTIONS (experimental):\n\n"

            "    -dsm_maxmem_mb <n>     Maximum amount of memory to use for the DSM "
            "system in MB.\n"
            "                           (default: %d MB)\n"
            "\n"
            "    -distmem_min_elems <n> Minimum number of array elements per node such "
            "that an array\n"
            "                           gets distributed (default: %d elements)\n"
            "\n"
            "    -distmem_tr_pf_node <r> Only produce trace and profile output at the "
            "node with this rank (-1 = all nodes).\n"
            "                            (default: %d elements)\n"
            "\n"
            "    -dmgs_min_selects <n> Minimum number of selects that have not been "
            "marked as local in the DMMLS\n"
            "                          optimisation so that the DMGS optimisation is "
            "applied.\n"
            "                          The value has to be at least 3 because otherwise "
            "it cannot pay off.\n"
            "                          (default: %d elements)\n"
            "\n"
            "    -dmgs_max_selects <n> Maximum number of selects that have not been "
            "marked as local in the DMMLS\n"
            "                          optimisation so that the DMGS optimisation is "
            "applied.\n"
            "                          (default: %d elements, 0 = unbounded)\n"
            "\n"
            "    -dsm_cache_outside_seg Allocate the caches outside of the DSM memory "
            "segment when GASNet\n"
            "                           is used. This requires Firehose to be disabled.\n"
            "                           (By default, the caches are allocated within the "
            "DSM segment when GASNet is used.)\n"
            "\n"
            "    -distmem_ptrs_desc   Keep pointers to the start of a distributed array "
            "at\n"
            "                         every node in the array descriptor.\n"
            "\n"
            "    -distmem_no_ptr_cache  Disables the pointer cache.\n"
            "\n",
            global.distmem_max_memory_mb, global.distmem_min_elems_per_node,
            global.distmem_tr_pf_node, global.dmgs_min_selects, global.dmgs_max_selects);

    DBUG_RETURN ();
}

static void
PrintMultithreadOptions (void)
{
    DBUG_ENTER ();

    printf ("\n\nMULTI-THREAD OPTIONS:\n\n"

            "    -mt_barrier_type <mode>\n"
            "                    Select barrier type for synchronization of threads:\n"
            "                      spin:    (default) uses a spinlock technique to "
            "implement the \n"
            "                               barrier\n"
            "                      mutex:   a barrier implementation solely based on "
            "mutex locks \n"
            "                               and counters\n"
            "                      cond:    uses pthread_cond_wait to implement the "
            "barrier\n"
            "                      pthread: uses the built-in POSIX thread barrier "
            "implementation\n"
            "                      futex:   a barrier that uses the low level futex lock "
            "technique,\n"
            "                               NOTE: FUTEX BARRIER IS ONLY AVAILABLE ON "
            "LINUX!\n"
            "\n"
            "    -mt_bind <mode>\n"
            "                    Select cpu binding strategy:\n"
            "                      off:    (default) OS decides the mapping of threads "
            "to cores.\n"
            "\n"
            "                      simple: bind to consecutive (wrt. hwloc idxs) cores\n"
            "\n"
            "                      env:    use environment variables to define\n"
            "                              SAC_NUM_SOCKETS the # of sockets to use\n"
            "                              SAC_NUM_CORES the # of cores per socket to "
            "use\n"
            "                              SAC_NUM_PUS the # of PUs per core to use\n"
            "\n"
            "                      numa:   bind to PUs, evenly distributed over all NUMA "
            "nodes. \n"
            "\n"
            "                    socket:   bind to PUs, evenly distributed over all "
            "sockets.\n"
            "\n"
            "    -mt_smart_mode <mode>\n"
            "                    Select smart decision mode:\n"
            "                      off:     (default) do not use the smart decision "
            "technique\n"
            "                      train:   toggles the smart decision tool into "
            "training mode\n"
            "                      on:      use smart decisions to optimize the number "
            "of threads\n"
            "                               being used to compute spmd functions\n"
            "\n"
            "    -mt_smart_filename <str>\n"
            "                    Filename for smart decision database file.\n"
            "\n"
            "    -mt_smart_arch <str>\n"
            "                    The name of the computer architecture you are using.\n"
            "\n"
            "    -mt_smart_period <n>\n"
            "                    To be able to make smart decisions, a performance "
            "profile is being\n"
            "                    created by performing a number of time measurements. "
            "This value is\n"
            "                    an indication for the time (ms) a single measurement "
            "should take.\n"
            "                    The real time can be a bit shorter or a bit longer.\n"
            "\n"
            "    -mt_smart_gradient <n>\n"
            "                    To be able to make smart decisions, a performance "
            "profile is being\n"
            "                    created. This profile is used to create a performance "
            "graph for\n"
            "                    each spmd function. The smart decision tool tries to "
            "find the point\n"
            "                    on the performance graph where the gradient matches "
            "the\n"
            "                    'mt_smart_gradient' value (degrees). This point on the "
            "performance\n"
            "                    graph will be used to determine the optimal number of "
            "threads to\n"
            "                    be used to parallelize the spmd function.\n"
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
            "    -dofoldparallel Enable parallelization of fold with-loops (default).\n"
            "    -nofoldparallel Disable parallelization of fold with-loops.\n"
            "\n"
            "    -minmtsize <n>  Specify minimum generator set size for parallel "
            "execution\n"
            "                    of with-loops.\n"
            "                      (default: %d)\n"
            "\n"
            "    -maxrepsize <n> Specify maximum size for arrays to be replicated as\n"
            "                    private data of multiple threads.\n"
            "                      (default: %d)\n",
            global.max_threads, global.min_parallel_size, global.max_replication_size);

    DBUG_RETURN ();
}

static void
PrintCudaOptions (void)
{
    DBUG_ENTER ();

    printf ("\n\nCUDA OPTIONS:\n"
            "\n"
            "    -cuda_async_mode <mode>\n"
            "                    Specify synchronisation mode between host thread and CUDA:\n"
            "                        nosync: do not synchronise at all\n"
            "                        device: synchronise the entire device\n"
            "                        stream: synchronise the stream\n"
            "                      callback: synchronise the stream using callback\n"
            "                    (default: device)\n"
            "\n"
            "    -cuda_gpu_branching\n"
            "                    Switch to branching implementation of the CUDA kernel mappings.\n"
            "                    Branchless implementations should be faster in 99%% of the cases,\n"
            "                    but in some complex cases where many kernels can be discarded\n"
            "                    early on in the process, the branching implementation may be faster. \n"
            "                    (default: branchless implementation)\n"
            "\n"
            "    -gpu_mapping_strategy <strategy>\n"
            "                    Choose the strategy with which index spaces are mapped onto the GPU \n"
            "                    kernels. Options are: \n"
            "                      jings_method         : Use the original heuristics Jing used in his\n"
            "                                             implementation of mappings with-loops to the\n"
            "                                             GPU. Note that -gpu_mapping_nocompress should\n"
            "                                             be set manually to have exactly Jing's method.\n"
            "                      jings_method_ext     : Similar to Jing's method, but also works for\n"
            "                                             higher dimensionalities. \n"
            "                      foldall              : First fold all dimensions together, and then\n"
            "                                             unfold them all again onto a pre-determined \n"
            "                                             shape and a \"rest\" dimension. \n"
            "                    (default: foldall)\n"
            "\n"
            "    -gpu_mapping_nocompress\n"
            "                    Disable index space compression for mapping with-loops onto the GPU. \n"
            "\n"
            "    -gpu_measure_kernel_time\n"
            "                    Measure the time spent in cuda kernels. Every kernel gets measured\n"
            "                    separately, and the time is printed in microseconds on it's own line. \n"
            "\n");

    DBUG_RETURN ();
}

static void
PrintBackendOptions (void)
{
    DBUG_ENTER ();

    printf ("\n\nBACKEND OPTIONS:\n"
            "\n"
            "    -minarrayrep <class>\n"
            "                    Specify the minimum array representation class used:\n"
            "                      s: use all (SCL, AKS, AKD, AUD) representations,\n"
            "                      d: use SCL, AKD, AUD representations only,\n"
            "                      +: use SCL, AUD representations only,\n"
            "                      *: use AUD representation only.\n"
            "                    (default: s)\n"
            "\n"
            "    -force_desc_size <n>\n"
            "                    Force the size of the descriptor to n bytes\n"
            "\n");

    DBUG_RETURN ();
}

#ifndef DBUG_OFF

static void
PrintFredFishOptions (void)
{
    DBUG_ENTER ();

    printf (
      "\n\nOPTIONS FOR FRED FISH'S DBUG:\n\n"
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

    DBUG_RETURN ();
}

static void
PrintDebugOptions (void)
{
    DBUG_ENTER ();

    printf ("\n\nDEBUG OPTIONS:\n\n"
            "    -debug_rc       Enable reference counting debuging features\n"
            "    -d treecheck    Check syntax tree for consistency with xml "
            "specification.\n"
            "    -d memcheck     Check syntax tree for memory consistency.\n"
            "    -d gpukernel    Check CUDA index-space to thread-space mapping "
            "and back.\n"
            "    -d nofree       Don't free any memory.\n"
            "    -d noclean      Don't initialize or clean memory before freeing.\n"
            "    -d sancheck     Check syntax tree for structural consistency.\n"
            "    -d lacfuncheck  Check syntax tree for single call property of LaC "
            "functions.\n"
            "    -d nolacinline  Do not inline loop and conditional functions.\n"
            "    -d nocleanup    Do not remove temporary files and directories.\n"
            "    -d syscall      Show all system calls during compilation.\n"
            "    -d cccall       Generate shell script \".sac2c\" that contains C "
            "compiler\n"
            "                    invocation.\n"
            "                    This implies option \"-d nocleanup\".\n"
            "\n"
            "    -stop           issue a SIGSTOP at the very beginning (usefull for OSX "
            "debugging).\n"
            "\n"
            "    -chkfreq <n>    Frequency of treecheck and lacfuncheck:\n"
            "                       0: no checks\n"
            "                       1: after each phase\n"
            "                       2: after each subphase\n"
            "                       3: after each optimisation\n"
            "                       4: after each function-based optimisation\n"
            "                    Default: %d\n",
            global.check_frequency);

    DBUG_RETURN ();
}

#endif /* DBUG_OFF */

static void
PrintRuntimeCheckOptions (void)
{
    DBUG_ENTER ();

    printf (
      "\n\nRUNTIME CHECK OPTIONS:\n\n"

      "    -ecc            Insert explicit conformity checks at compile time.\n"
      "\n"
      "    -check [acgtbmehdi]+\n"
      "                    Incorporate runtime checks into executable program.\n"
      "                    The following flags are supported:\n"
      "                      a: Incorporate all available runtime checks.\n"
      "                      c: Perform conformity checks.\n"
      "                      g: Perform GPU error checks.\n"
      "                      t: Check assignments for type violations.\n"
      "                      b: Check array accesses for boundary violations.\n"
      "                      m: Check success of memory allocations.\n"
      "                      e: Check errno variable upon applications of\n"
      "                         external functions.\n"
      "                      h: Use diagnostic heap manager.\n"
      "                      d: Perform checks for the distributed memory backend.\n"
      "                         (Check that there are no illegal accesses to "
      "distributed\n"
      "                         arrays.)\n"
      "                      i: Use diagnostic heap manager for distributed memory "
      "backend.\n");

    DBUG_RETURN ();
}

static void
PrintRuntimeTraceOptions (void)
{
    DBUG_ENTER ();

    printf (
      "\n\nRUNTIME TRACE OPTIONS:\n\n"

      "    -trace [amrfpwstcd]+\n"
      "                    Incorporate trace output generation into executable program.\n"
      "                    The following flags are supported:\n"
      "                      a: Trace all (same as mrfpwstcd).\n"
      "                      m: Trace memory operations.\n"
      "                      r: Trace reference counting operations.\n"
      "                      f: Trace user-defined function calls.\n"
      "                      p: Trace primitive function calls.\n"
      "                      w: Trace with-loop execution.\n"
      "                      s: Trace array accesses.\n"
      "                      g: Trace CUDA runtime.\n"
      "                      t: Trace multi-threading specific operations.\n"
      "                      c: Trace runtime enviroment init/exit when\n"
      "                         using SAC libraries in C programs.\n"
      "                      d: Trace distributed memory run time.\n"
      "                         (per element tracing only in combination with s)\n"
      "\n"
      "    -utrace\n"
      "                    Introduce user tracing calls.");

    DBUG_RETURN ();
}

static void
PrintPrintingOptions (void)
{
    DBUG_ENTER ();

    printf (
      "\n\nPRINTING OPTIONS:\n\n"
      "    -print [adv]+\n"
      "               Add internal AST information as comments to the program output.\n"
      "               The following flags are supported:\n"
      "                 a: Print all (same as dv).\n"
      "                 d: Print specialization demand.\n"
      "                 v: Print avis information.\n"
      "\n"
      "    -printfun <fun_name>\n"
      "               Uses <fun_name> as a compare token to selectively print only\n"
      "               functions that match the <fun_name>. \n"
      "               Use in conjunction with -b \n"
      "                 Example usage: -b 5:goi -printfun foo\n"
      "\n"
      "    -printfunsets [aiudwp]+\n"
      "                Select different categories of funcion to print or visualize\n"
      "                (if using -doVAB when break).\n"
      "               Use in conjunction with -b \n"
      "               The following flags are supported:\n"
      "                 a: Print/visualize all function(same as iudwp).\n"
      "                 i: Print/visualize functions introduced by import statement.\n"
      "                 u: Print/visualize functions introduced by use statement.\n"
      "                 d: Print/visualize local-defined functions.\n"
      "                 w: Print/visualize wrapper functions.\n"
      "                 p: Print/visualize prelude functions.\n"
      "\n"
      "    -printstart <phase_id> [-printstop <phase_id>]\n"
      "               Will report end of phase output to files in the format\n"
      "               a.out.<phase_id> unless -o <outfilename> option is specified\n"
      "               in which case the output file format will be\n"
      "               <outfilename>.<phase_id>\n"
      "\n"
      "    -printstop <phase_id> \n"
      "               if this option is used printing will stop at this\n"
      "               phase whilst compilation will continue until completion\n"
      "               unless -b is used.\n"
      "\n"
      "               NOTE: Local Funs associated with Fundefs will be printed\n"
      "               after every fundef node.\n"
      "\n"
      "    <phase_id>\n"
      "               In the context of -printstart/-printstop options this describes\n"
      "               the level of granularity that the output will appear in.\n"
      "               For Example:\n"
      "               -printstart 2 -printstop 4\n"
      "                 Will report the state after the end of each phase in the range.\n"
      "               -printstart 2:hs -printstop 2 || -printstop 2:csgd\n"
      "                 Will report the state after the end of each sub phase in the "
      "range\n"
      "                 2:hs -> 2:csgd.\n"
      "               -printstart 11:cyc -printstop 11:cyc\n"
      "                 Will report the state after each pass of the cyc cycle\n"
      "               -printstart 11:cyc:cse -printstop 11:cyc:cse\n"
      "                 Will report the state of each fundef node after a pass of\n"
      "                 the given cyclephase. NOTE: it is recommended to use this in\n"
      "                 conjunction with -printfun <fun_name> as the output could get "
      "BIG.\n"
      "\n");

    DBUG_RETURN ();
}

static void
PrintCompiletimeAnalysesOptions (void)
{
    DBUG_ENTER ();

    printf (
      "\n\nCOMPILETIME FEEDBACK OPTIONS:\n\n"

      "    -feedback [atcor]+\n"
      "                    Perform some compile time analyses and output some user feedback.\n"
      "                    The following flags are supported:\n"
      "                      a: Analyse all (same as tcor).\n"
      "                      t: Analyse the effectiveness of function specialisation.\n"
      "                      c: Analyse the effectveness of static constraint resolution.\n"
      "                      o: Analyse the effectiveness of the optimisation cycle.\n"
      "                      r: Analyse the effectiveness of reference counting.\n");

    DBUG_RETURN ();
}

static void
PrintCompiletimeMessagingOptions (void)
{
    DBUG_ENTER ();

    printf (
      "\n\nCOMPILETIME MESSAGING OPTIONS:\n\n"

      "    -cti-message-length <n>\n"
      "                    Insert extra line breaks for messages with lines that are longer\n"
      "                    than <n> characters. The value 0 implies no extra line breaks.\n"
      "                    (default: max(40, terminal-width) = %d)\n"
      "    -cti-no-color   Disables all color output.\n"
      "    -cti-single-line\n"
      "                    Omits all line breaks within messages and sets -cti-message-length 0.\n"
      "    -cti-header-format <string>\n"
      "                    Customize the header of the first line of all message types. The string\n"
      "                    must contain exactly one '%%s` which will be replaced by a possible\n"
      "                    location (GNU format), followed by the message type (error, warning or note)."
      "                    The string may also contain '@` to indicate desired line breaks.\n"
      "                    (default: \"%s\")\n"
      "    -cti-multi-line-format <string>\n"
      "                    Customize the header of continuation lines in multi-line messages.\n"
      "                    The string must contain exactly one '%%s` which will be replaced by a\n"
      "                    possible location (GNU format), followed by the message type (error,\n"
      "                    warning or note).\n"
      "                    The '@' character is not converted into line breaks.\n" 
      "                    Note that \"%%.s\" or \"%%.0s\" allows the location and message type\n"
      "                    to be omitted. This is done with the default value: \"%s\".\n",
      global.cti_message_length, global.cti_header_format, global.cti_multi_line_format);

    DBUG_RETURN ();
}

static void
PrintRuntimeProfilingOptions (void)
{
    DBUG_ENTER ();

    printf (
      "\n\nRUNTIME PROFILING OPTIONS:\n\n"

      "    -profile [afilwmocd]+\n"
      "                    Incorporate profiling analysis into executable program.\n"
      "                    The following flags are supported:\n"
      "                      a: Analyse all (same as filwmod).\n"
      "                      f: Analyse time spent in non-inline functions.\n"
      "                      i: Analyse time spent in inline functions.\n"
      "                      l: Analyse time spent in library functions.\n"
      "                      w: Analyse time spent in with-loops.\n"
      "                      m: Analyse number of memory operations.\n"
      "                      o: Analyse number of primitive operations (FLOPS,INTOPS).\n"
      "                      c: Analyse time spent in CUDA kernels.\n"
      "                      d: Analyse performance of distributed memory backend.\n");

    DBUG_RETURN ();
}

static void
PrintCacheSimulationOptions (void)
{
    DBUG_ENTER ();

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
      "\n\n");
    printf (
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

    DBUG_RETURN ();
}

static void
PrintLibraryOptions (void)
{
    DBUG_ENTER ();

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
            "    -noprelude       Do not load the standard prelude library `%s'.\n"
            "\n"
            "    -on_demand_lib   Do prepare the library provided as source for on "
            "demand\n"
            "                     compilation when needed by other source files.\n"
            "\n"
            "    -notree          Avoid generating tree file when compiling a module or "
            "a class.\n",
            global.linksetsize == INT_MAX ? 0 : global.linksetsize, global.preludename);

    DBUG_RETURN ();
}

static void
PrintCCompilerOptions (void)
{
    DBUG_ENTER ();

    printf ("\n\nC-COMPILER OPTIONS:\n\n"

            "    -g              Include debug information into object code.\n"
            "    -gg             Same as `-g', with extra debug information included\n"
            "                    into object code. (e.g. source line # mapping from\n"
            "                    SAC source code, etc.)\n"
            "\n"
            "    -O <n>          Specify the C compiler level of optimization.\n"
            "                      0: no C compiler optimizations.\n"
            "                      1: minor C compiler optimizations.\n"
            "                      2: medium C compiler optimizations.\n"
            "                      3: full C compiler optimizations.\n"
            "                    (default: %d)\n"
            "\n"
            "    -generic        Specify that the generated C code should be compiled\n"
            "                    without any architecture specific optimisations. This\n"
            "                    is useful when compiling for other systems.\n"
            "                    NOTE: if this flag is not given, the C code will be\n"
            "                          aggresively optimised to take advantage of the\n"
            "                          current architecture.\n"
            "\n"
            "                    NOTE:\n"
            "                    The actual effects of these options are specific to\n"
            "                    the C compiler used for code generation. Both the\n"
            "                    choice of a C compiler as well as the mapping of these\n"
            "                    generic options to compiler-specific optimization\n"
            "                    options are determined via the sac2crc configuration\n"
            "                    file. For details concerning sac2crc files see below\n"
            "                    under \"customization\".\n",
            global.cc_optimize);

    DBUG_RETURN ();
}

static void
PrintCustomisationOptions (void)
{
    DBUG_ENTER ();

    printf (
      "\n\nCUSTOMIZATION OPTIONS:\n\n"

      "    -target <name>  Specify a particular compilation target.\n"
      "    -t <name>       Compilation targets are used to customize sac2c for\n"
      "                    various target architectures, operating systems, and C\n"
      "                    compilers.\n"
      "\n"
      "                    The target description is either read from the\n"
      "                    installation specific file $PREFIX/share/sac2crc or\n"
      "                    from a file named .sac2crc within the user's home\n"
      "                    directory.\n"

      "\n                  target list can be printed with -list-targets\n"

      "\n");
    DBUG_RETURN ();
}



static void
PrintEnvironmentVariables (void)
{
    DBUG_ENTER ();

    printf (
      "\n\nENVIRONMENT VARIABLES:\n\n"

      "    The following environment variables are used by the SAC compiler suite:\n"
      "\n"
      "    SAC2CRC         Invoke compiler with a specific configuration file.\n"
      "    SAC_PARALLEL    (optional) specifies the number of threads to use.\n"
      "    SAC_NUM_SOCKETS (optional) specifies the number of sockets to use.\n"
      "    SAC_NUM_CORES   (optional) specifies the number of cores per socket to use.\n"
      "    SAC_NUM_PUS     (optional) specifies the number of processing units (e.g\n"
      "                    hardware threads) per core to use.\n"
      "                    If SAC_NUM_SOCKETS, SAC_NUM_CORES or SAC_NUM_PUS are not\n"
      "                    set or set to 0, threads will not be bound."
      "\n");

    DBUG_RETURN ();
}

static void
PrintAuthors (void)
{
    DBUG_ENTER ();

    printf ("\n%s\n", build_author);

    DBUG_RETURN ();
}

static void
PrintContact (void)
{
    DBUG_ENTER ();

    printf ("\n\nCONTACT:\n\n"

            "    WWW:    https://www.sac-home.org/\n"
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

    DBUG_RETURN ();
}

void
USGprintUsage ()
{
    DBUG_ENTER ();

    PrintToolName ();

    switch (global.tool) {
    case TOOL_sac2c:
        PrintDescriptionSac2c ();
        if (global.verbose_help)
            PrintFeatureSet ();
        PrintSpecialOptions ();
        PrintGeneralOptions ();
#ifndef DBUG_OFF
        PrintBreakOptions ();
        PrintBreakoptionSpecifierSac2c ();
#else
        if (global.verbose_help) {
            global.verbose_help = FALSE;
            PrintBreakOptions ();
            PrintBreakoptionSpecifierSac2c ();
            global.verbose_help = TRUE;
        }
#endif
        if (global.verbose_help)
            PrintPrintingOptions ();
        if (global.verbose_help)
            PrintTypeInferenceOptions ();
        PrintOptimisationOptions ();
        if (global.verbose_help || (global.mtmode == MT_createjoin) || (global.mtmode == MT_startstop))
            PrintMultithreadOptions ();
        if (global.verbose_help || (global.backend == BE_cuda) || (global.backend == BE_cudahybrid))
            PrintCudaOptions ();
        if (global.verbose_help||(global.backend == BE_distmem))
            PrintDistMemOptions ();
        if (global.verbose_help||(global.backend == BE_mutc))
            PrintMutcOptions ();
        if (global.verbose_help)
            PrintBackendOptions ();
#ifndef DBUG_OFF
        PrintDebugOptions ();
        PrintFredFishOptions ();
#endif /* DBUG_OFF */
        PrintCompiletimeAnalysesOptions ();
        if (global.verbose_help)
            PrintCompiletimeMessagingOptions ();
        PrintRuntimeCheckOptions ();
        PrintRuntimeTraceOptions ();
        PrintRuntimeProfilingOptions ();
        if (global.verbose_help)
            PrintCacheSimulationOptions ();
        if (global.verbose_help)
            PrintLibraryOptions ();
        PrintCCompilerOptions ();
        PrintCustomisationOptions ();
        break;
    case TOOL_sac4c:
        PrintDescriptionSac4c ();
        PrintOptionsSac4c ();
#ifndef DBUG_OFF
        PrintFredFishOptions ();
#endif /* DBUG_OFF */
        PrintBreakoptionSpecifierSac4c ();
        PrintCCompilerOptions ();
        break;
    case TOOL_sac2tex:
        PrintDescriptionSac2tex ();
        PrintGeneralOptions ();
#ifndef DBUG_OFF
        PrintFredFishOptions ();
#endif /* DBUG_OFF */
        break;
    }

    PrintEnvironmentVariables ();
    if (global.verbose_help)
        PrintAuthors ();
    if (global.verbose_help)
        PrintContact ();

    printf ("\n\n");

    DBUG_RETURN ();
}

void
USGprintVersion ()
{
    DBUG_ENTER ();

    printf ("%s %s\n"
            "build-type: %s\n"
            "built-by: \"%s\" at %s\n",
            global.toolname, (global.version_id[0] == '\0') ? "???" : global.version_id,
            (build_style[0] == '\0') ? "" : build_style,
            (build_user[0] == '\0') ? "???" : build_user,
            (build_date[0] == '\0') ? "???" : build_date);

    DBUG_RETURN ();
}

void
USGprintVersionVerbose ()
{
    DBUG_ENTER ();

    PrintToolName ();

    printf ("Build-type:              %s\n"
            "Built-by:                %s\n"
            "Host:                    %s\n"
            "Date:                    %s\n",
            (build_style[0] == '\0') ? "" : build_style,
            (build_user[0] == '\0') ? "???" : build_user,
            (build_host[0] == '\0') ? "???" : build_host,
            (build_date[0] == '\0') ? "???" : build_date);

    printf ("%s\n", BUILD_STATUS);

    printf ("Homepage: https://www.sac-home.org\n"
            "Email: info@sac-home.org\n\n"
            "Copyright (c) 1994-2022 SAC Development Team\n\n");

    DBUG_RETURN ();
}

void
USGprintCopyright ()
{
    DBUG_ENTER ();

    printf ("\n%s\n", build_lic);

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
