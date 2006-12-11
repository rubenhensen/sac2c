/*
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "globals.h"
#include "build.h"
#include "internal_lib.h"
#include "dbug.h"
#include "usage.h"
#include "phase.h"

#define PRINT_BREAK_SPEC(ph, spec, comment)                                              \
    {                                                                                    \
        int _i;                                                                          \
        printf ("    -b %2i:%s", ph, spec);                                              \
        DBUG_ASSERT (strlen (spec) < 18, " you need to increase"                         \
                                         " the space in PRINT_BREAK_SPEC");              \
        for (_i = 0; (size_t)_i < (18 - strlen (spec)); _i++) {                          \
            printf (" ");                                                                \
        }                                                                                \
        printf ("%s\n", comment);                                                        \
    }

#define CONT_BREAK_SPEC(comment)                                                         \
    {                                                                                    \
        printf ("                            %s\n", comment);                            \
    }

void
USGprintUsage ()
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
      "    -VV             Display verbose version identification.\n"
      "\n"
      "    -libstat        Print status information of the given SAC library file.\n"
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
            "                      0: error messages only,\n"
            "                      1: error messages and warnings,\n"
            "                      2: basic compile time information,\n"
            "                      3: full compile time information.\n"
            "                    (default: %d)\n",
            global.verbose_level);

    printf ("\n\nBREAK OPTIONS:\n\n"

            "    Break options allow you to stop the compilation process\n"
            "    after a particular phase.\n"
            "    By default the programm will then be printed out, but this behaviour\n"
            "    may be influenced by the following compiler options:\n"
            "\n"
            "    -noPAB          Deactivates printing after break.\n"
            "    -doPAB          Activates printing after break.\n\n");

    for (ph = 1; ph <= PH_genccode; ph++) {
        printf ("    -b %2i           Stop after: %s.\n", ph, PHphaseName (ph));
    }

    printf ("\n\nBREAK SPECIFIERS:\n\n"

            "    Break specifiers allow you to stop the compilation process\n"
            "    within a particular phase.\n\n"

            "    Currently supported break specifiers are as follows:\n\n");

    PRINT_BREAK_SPEC (PH_scanparse, "cpp", "Stop after running the C preprocessor.");
    PRINT_BREAK_SPEC (PH_scanparse, "sp", "Stop after parsing (yacc).");
    PRINT_BREAK_SPEC (PH_scanparse, "hzgwl",
                      "Stop after transforming zero-generator with-loops");
    PRINT_BREAK_SPEC (PH_scanparse, "hwlg",
                      "Stop after transforming multi-generator with-loops");
    PRINT_BREAK_SPEC (PH_scanparse, "hwlo",
                      "Stop after transforming multi-operator with-loops");
    PRINT_BREAK_SPEC (PH_scanparse, "acn",
                      "Stop after resolving the axis control notation.");
    PRINT_BREAK_SPEC (PH_scanparse, "pragma", "Stop after resolving pragmas");
    PRINT_BREAK_SPEC (PH_scanparse, "objinit",
                      "Stop after generating generic types and functions");

    printf ("\n");

    PRINT_BREAK_SPEC (PH_module, "rsa", "Stop after resolving all flag.");
    PRINT_BREAK_SPEC (PH_module, "ans", "Stop after annotating namespaces.");
    PRINT_BREAK_SPEC (PH_module, "gdp", "Stop after gathering dependencies.");
    PRINT_BREAK_SPEC (PH_module, "imp", "Stop after importing instances.");
    PRINT_BREAK_SPEC (PH_module, "uss", "Stop after fetching used symbols.");

    printf ("\n");

    PRINT_BREAK_SPEC (PH_simplify, "w2d",
                      "Stop after transforming while-loops into do-loops");
    PRINT_BREAK_SPEC (PH_simplify, "hce",
                      "Stop after eliminating conditional expressions");
    PRINT_BREAK_SPEC (PH_simplify, "hm",
                      "Stop after resolving (multiple) applications of infix");
    CONT_BREAK_SPEC ("operations.");
    PRINT_BREAK_SPEC (PH_simplify, "flat", "Stop after flattening nested expressions");

    printf ("\n");

    PRINT_BREAK_SPEC (PH_pretypecheck, "rst", "Stop after resolving symbol types.");
    PRINT_BREAK_SPEC (PH_pretypecheck, "ivd", "Stop after inserting vardecs.");
    PRINT_BREAK_SPEC (PH_pretypecheck, "itc", "Stop after inserting type conversions.");
    PRINT_BREAK_SPEC (PH_pretypecheck, "ses",
                      "Stop after stripping external signatures.");
    PRINT_BREAK_SPEC (PH_pretypecheck, "cwr", "Stop after creating wrappers.");
    PRINT_BREAK_SPEC (PH_pretypecheck, "oan", "Stop after analysing objects.");
    PRINT_BREAK_SPEC (PH_pretypecheck, "goi", "Stop after creating object initialisers.");
    PRINT_BREAK_SPEC (PH_pretypecheck, "rso", "Stop after resolving global objects.");
    PRINT_BREAK_SPEC (PH_pretypecheck, "rra",
                      "Stop after resolving reference arguments.");
    PRINT_BREAK_SPEC (PH_pretypecheck, "ewt", "Stop after extending wrapper types.");
    PRINT_BREAK_SPEC (PH_pretypecheck, "l2f",
                      "Stop after converting loops and conditionals into");
    CONT_BREAK_SPEC ("functions.");
    PRINT_BREAK_SPEC (PH_pretypecheck, "ssa", "Stop after converting into SSA form.");

    printf ("\n");

    PRINT_BREAK_SPEC (PH_typecheck, "ntc", "Stop after infering all types.");
    PRINT_BREAK_SPEC (PH_typecheck, "ds", "Stop after deserializing code.");
    PRINT_BREAK_SPEC (PH_typecheck, "eat", "Stop after eliminating type variables.");
    PRINT_BREAK_SPEC (PH_typecheck, "ebt", "Stop after eliminating bottom types.");
    PRINT_BREAK_SPEC (PH_typecheck, "swr", "Stop after splitting wrappers.");

    printf ("\n");

    PRINT_BREAK_SPEC (PH_elimudt, "cwc",
                      "Stop after creating SAC code for wrapper functions.");
    PRINT_BREAK_SPEC (PH_elimudt, "l2f",
                      "Stop after converting loops and conditionals into");
    CONT_BREAK_SPEC ("functions.");
    PRINT_BREAK_SPEC (PH_elimudt, "ssa", "Stop after converting into SSA form.");
    PRINT_BREAK_SPEC (PH_elimudt, "dfc",
                      "Stop after trying to dispatch function calls statically.");
    PRINT_BREAK_SPEC (PH_elimudt, "eudt", "Stop after eliminating user-defined types.");

    printf ("\n");

    PRINT_BREAK_SPEC (PH_wlenhance, "accu",
                      "Stop after inserting explicit accumulation.");
    PRINT_BREAK_SPEC (PH_wlenhance, "wldp", "Stop after adding default partitions.");
    PRINT_BREAK_SPEC (PH_wlenhance, "wlpg", "Stop after with-loop partition generation.");

    printf ("\n");

    PRINT_BREAK_SPEC (PH_sacopt, "inl", "Stop after function inlining.");
    PRINT_BREAK_SPEC (PH_sacopt, "dfr", "Stop after initial dead function removal.");
    PRINT_BREAK_SPEC (PH_sacopt, "ae", "Stop after array elimination.");
    PRINT_BREAK_SPEC (PH_sacopt, "dcr", "Stop after dead code removal.");
    PRINT_BREAK_SPEC (PH_sacopt, "lir", "Stop after (with-)loop invariant removal.");

    printf ("\n");

    PRINT_BREAK_SPEC (PH_sacopt, "cyc:<n>:cse",
                      "Stop in cycle <n> after common subexpression elimination.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc:<n>:ntc",
                      "Stop in cycle <n> after type upgrade inference.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc:<n>:eat",
                      "Stop in cycle <n> after type variable elimination.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc:<n>:ebt",
                      "Stop in cycle <n> after bottom type elimination.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc:<n>:dfc",
                      "Stop in cycle <n> after dispatch function calls.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc:<n>:inl", "Stop in cycle <n> after inlining.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc:<n>:cf",
                      "Stop in cycle <n> after constant folding.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc:<n>:cvp",
                      "Stop in cycle <n> after constant and variable propagation.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc:<n>:wlpg",
                      "Stop in cycle <n> after with-loop partition generation.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc:<n>:wlsimp",
                      "Stop in cycle <n> after with-loop simplification.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc:<n>:cwle",
                      "Stop in cycle <n> after copy with-loop elimination.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc:<n>:wlt",
                      "Stop in cycle <n> after with-loop transformation.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc:<n>:wli",
                      "Stop in cycle <n> after with-loop information gathering.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc:<n>:wlf",
                      "Stop in cycle <n> after with-loop folding.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc:<n>:ssawlf",
                      "Stop in cycle <n> after restoring SSA form after with-loop "
                      "folding.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc:<n>:swlf",
                      "Stop in cycle <n> after symbolic with-loop folding.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc:<n>:cf2",
                      "Stop in cycle <n> after second constant folding.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc:<n>:dcr",
                      "Stop in cycle <n> after dead code removal.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc:<n>:wls",
                      "Stop in cycle <n> after with-loop scalarization.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc:<n>:lur",
                      "Stop in cycle <n> after (with-)loop unrolling.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc:<n>:ssalur",
                      "Stop in cycle <n> after restoring SSA form after (with-)loop "
                      "unrolling.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc:<n>:cf3",
                      "Stop in cycle <n> after third constant folding.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc:<n>:lus",
                      "Stop in cycle <n> after loop unswitching.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc:<n>:wlir",
                      "Stop in cycle <n> after with-loop invariant removal.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc:<n>:al",
                      "Stop in cycle <n> after associative law.");
    PRINT_BREAK_SPEC (PH_sacopt, "cyc:<n>:dl",
                      "Stop in cycle <n> after distributive law.");

    printf ("\n");

    PRINT_BREAK_SPEC (PH_sacopt, "cyc", "Stop after fundef optimization cycle.");
    PRINT_BREAK_SPEC (PH_sacopt, "lir2", "Stop after (with-)loop invariant removal.");
    PRINT_BREAK_SPEC (PH_sacopt, "uesd",
                      "Stop after reintroducing subtraction and division operators.");
    PRINT_BREAK_SPEC (PH_sacopt, "dfr2", "Stop after dead function removal.");
    PRINT_BREAK_SPEC (PH_sacopt, "dcr2", "Stop after dead code removal.");
    PRINT_BREAK_SPEC (PH_sacopt, "wlfs", "Stop after with loop fusion.");
    PRINT_BREAK_SPEC (PH_sacopt, "cse2", "Stop after common subexpression elimination.");
    PRINT_BREAK_SPEC (PH_sacopt, "dcr3", "Stop after dead code removal.");
    PRINT_BREAK_SPEC (PH_sacopt, "rtc", "Stop after final type inference.");
    PRINT_BREAK_SPEC (PH_sacopt, "fineat", "Stop after final type variable elimination.");
    PRINT_BREAK_SPEC (PH_sacopt, "finebt", "Stop after final bottom type elimination.");
    PRINT_BREAK_SPEC (PH_sacopt, "wlpg2",
                      "Stop after with-loop default partition generation.");
    PRINT_BREAK_SPEC (PH_sacopt, "wrci",
                      "Stop after with-loop reuse candidates inference.");
    PRINT_BREAK_SPEC (PH_sacopt, "wlidx",
                      "Stop after annotating offset variables at with-loops.");
    PRINT_BREAK_SPEC (PH_sacopt, "ivei",
                      "Stop after index vector elimination inference.");
    PRINT_BREAK_SPEC (PH_sacopt, "ive", "Stop after index vector elimination.");
    PRINT_BREAK_SPEC (PH_sacopt, "iveo",
                      "Stop after index vector elimination optimisation.");
    PRINT_BREAK_SPEC (PH_sacopt, "cvpive",
                      "Stop after constant and variable propagation after ive.");
    PRINT_BREAK_SPEC (PH_sacopt, "lirive",
                      "Stop after loop invariant removal after ive.");
    PRINT_BREAK_SPEC (PH_sacopt, "cseive",
                      "Stop after common subexpression elimination after ive.");
    PRINT_BREAK_SPEC (PH_sacopt, "dcrive", "Stop after dead code removal after ive.");
    PRINT_BREAK_SPEC (PH_sacopt, "fdi", "Stop after freeing dispatch information.");

    printf ("\n");

    PRINT_BREAK_SPEC (PH_wltrans, "ussa", "Stop after undo ssa transform.");
    PRINT_BREAK_SPEC (PH_wltrans, "fun2lac",
                      "Stop after reintroducing loops and conditionals");
    PRINT_BREAK_SPEC (PH_wltrans, "lacinl", "Stop after inlining LaC functions");
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
    PRINT_BREAK_SPEC (PH_wltrans, "l2f", "Stop after Lac To Fun conversion.");
    PRINT_BREAK_SPEC (PH_wltrans, "ssa", "Stop after conversion into SSA form.");
    PRINT_BREAK_SPEC (PH_wltrans, "cvp", "Stop after Constant and variable Propagation.");
    PRINT_BREAK_SPEC (PH_wltrans, "dcr", "Stop after Dead Code Removal.");

    printf ("\n");
    printf ("    with \"-mt [-mtmode 2]\"\n");

    PRINT_BREAK_SPEC (PH_multithread, "spmdinit", "Stop after building SPMD blocks.");
    PRINT_BREAK_SPEC (PH_multithread, "spmdopt", "Stop after optimizing SPMD blocks.");
    PRINT_BREAK_SPEC (PH_multithread, "createmtfuns",
                      "Stop after creating MT/ST functions.");
    PRINT_BREAK_SPEC (PH_multithread, "spmdlift", "Stop after lifting SPMD blocks.");
    PRINT_BREAK_SPEC (PH_multithread, "sched",
                      "Stop after creating scheduling annotations.");
    PRINT_BREAK_SPEC (PH_multithread, "rmspmd", "Stop after eliminating SPMD blocks.");
    PRINT_BREAK_SPEC (PH_multithread, "l2f", "Stop after lifting SPMD conditionals.");
    PRINT_BREAK_SPEC (PH_multithread, "ssa",
                      "Stop after restoring SSA form in SPMD conditionals.");

    printf ("\n");
    printf ("    with \"-mt -mtmode 3\" (UNDER CONSTRUCTION!!!)\n");

    PRINT_BREAK_SPEC (PH_multithread, "init", "Stop after internal initialization.");
    PRINT_BREAK_SPEC (PH_multithread, "tem", "Stop after tagging the executionmode.");
    PRINT_BREAK_SPEC (PH_multithread, "crwiw",
                      "Stop after create replications within with-loops.");
    PRINT_BREAK_SPEC (PH_multithread, "pem", "Stop after propagating the executionmode.");
    PRINT_BREAK_SPEC (PH_multithread, "cdfg", "Stop after creating the dataflowgraph.");
    PRINT_BREAK_SPEC (PH_multithread, "asmra", "Stop after assignments rearranging.");
    PRINT_BREAK_SPEC (PH_multithread, "crece", "Stop after creating the cells.");
    PRINT_BREAK_SPEC (PH_multithread, "cegro", "Stop after cell growing.");
    PRINT_BREAK_SPEC (PH_multithread, "repfun", "Stop after replicating functions.");
    PRINT_BREAK_SPEC (PH_multithread, "concel", "Stop after consolidating the cells.");

    printf ("\n");

    PRINT_BREAK_SPEC (PH_memory, "copy", "Stop after Explicit Copy Inference.");
    PRINT_BREAK_SPEC (PH_memory, "alloc", "Stop after Explicit Allocation Inference.");
    PRINT_BREAK_SPEC (PH_memory, "dcr", "Stop after Dead Code Removal.");
    PRINT_BREAK_SPEC (PH_memory, "ri", "Stop after Reuse Inference.");
    PRINT_BREAK_SPEC (PH_memory, "ia", "Stop after Interface Analysis.");
    PRINT_BREAK_SPEC (PH_memory, "lro", "Stop after Loop Reuse Optimization.");
    PRINT_BREAK_SPEC (PH_memory, "frc", "Stop after Filtering Reuse Candidates.");
    PRINT_BREAK_SPEC (PH_memory, "sr", "Stop after Static Reuse.");
    PRINT_BREAK_SPEC (PH_memory, "rb", "Stop after Reuse Branching..");
    PRINT_BREAK_SPEC (PH_memory, "ipc", "Stop after Inplace Computation.");
    PRINT_BREAK_SPEC (PH_memory, "dr", "Stop after Data Reuse.");
    PRINT_BREAK_SPEC (PH_memory, "dcr2", "Stop after Dead Code Removal (2).");
    PRINT_BREAK_SPEC (PH_memory, "rc", "Stop after Reference Counting Inference.");
    PRINT_BREAK_SPEC (PH_memory, "rco", "Stop after Reference Counting Optimizations.");
    PRINT_BREAK_SPEC (PH_memory, "re", "Stop after Reuse Elimination.");

    printf ("\n");

    PRINT_BREAK_SPEC (PH_precompile, "prec1", "Stop after first traversal.");
    PRINT_BREAK_SPEC (PH_precompile, "prec2", "Stop after second traversal.");
    PRINT_BREAK_SPEC (PH_precompile, "prec3", "Stop after third traversal.");

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
      "This is a performance measurement option to be removed.\n"
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
      "    \"-no opt -do inl\" disables all optimizations except for function inlining.\n"
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

    printf ("\n\nBACKEND OPTIONS:\n\n"

            "    -minarrayrep <class>\n"
            "                    Specify the minimum array representation class used:\n"
            "                      s: use all (SCL, AKS, AKD, AUD) representations,\n"
            "                      d: use SCL, AKD, AUD representations only,\n"
            "                      +: use SCL, AUD representations only,\n"
            "                      *: use AUD representation only.\n"
            "                    (default: s)\n");

    printf (
      "\n\nGENERAL DEBUG OPTIONS:\n\n"

      "    -d nocleanup    Do not remove temporary files and directories.\n"
      "    -d syscall      Show all system calls during compilation.\n"
      "    -d cccall       Generate shell script \".sac2c\" that contains C compiler\n"
      "                    invocation.\n"
      "                    This implies option \"-d nocleanup\".\n");

    printf (
      "\n\nINTERNAL DEBUG OPTIONS:\n\n"
      "    -d treecheck    Check syntax tree for consistency with xml specification. \n"
      "    -d memcheck     Check syntax tree for memory consistency.\n"
      "    -d nolacinline  Do not inline loop and conditional functions.\n"
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

            "    -check [atbmeh]+\n"
            "                    Incorporate runtime checks into executable program.\n"
            "                    The following flags are supported:\n"
            "                      a: Incorporate all available runtime checks.\n"
            "                      t: Check assignments for type violations.\n"
            "                      b: Check array accesses for boundary violations.\n"
            "                      m: Check success of memory allocations.\n"
            "                      e: Check errno variable upon applications of\n"
            "                         external functions.\n"
            "                      h: Use diagnostic heap manager.\n");

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
            ((NULL != env) && (env[strlen (env) - 1] != '/')) ? "/" : "",
            global.version_id);

    printf ("\n\nINTRINSIC ARRAY OPERATIONS OPTIONS:\n\n"

            "    For compatibility reasons with older versions of sac2c intrinsic\n"
            "    implementations still exist for some of the basic array operations\n"
            "    which today are imported from the array module of the SAC standard\n"
            "    library. These intrinsic implementations can be activated by the\n"
            "    following compiler option.\n"
            "\n"
            "    -intrinsic [a+-x/so]+\n"
            "                    Use intrinsic implementations for array operations.\n"
            "                      a: Use all intrinsic operations  available\n"
            "                         (same as +-x/so).\n"
            "                      +: Use intrinsic add.\n"
            "                      -: Use intrinsic sub.\n"
            "                      x: Use intrinsic mul.\n"
            "                      /: Use intrinsic div.\n"
            "                      s: Use intrinsic sel.\n"
            "                      o: Use intrinsic type conversion.\n");

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
            "                     Multithreading is not yet available for C libraries.\n",
            global.linksetsize == INT_MAX ? 0 : global.linksetsize);

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

    printf ("\n\nCUSTOMIZATION:\n\n"

            "    -target <name>  Specify a particular compilation target.\n"
            "                    Compilation targets are used to customize sac2c for\n"
            "                    various target architectures, operating systems, and C\n"
            "                    compilers.\n"
            "                    The target description is either read from the\n"
            "                    installation specific file $SACBASE/runtime/sac2crc or\n"
            "                    from a file named .sac2crc within the user's home\n"
            "                    directory.\n");

    printf ("\n\nENVIRONMENT VARIABLES:\n\n"

            "    The following environment variables are used by sac2c:\n"
            "\n"
            "    SACBASE                  Base directory of SAC installation.\n"
            "    SAC_PATH                 Search path for SAC source code files.\n"
            "    SAC_LIBRARY_PATH         Search path for SAC library files.\n"
            "    SAC_IMPLEMENTATION_PATH  Search path for SAC library source files.\n"
            "\n");

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
            "      Stephan Herhut\n"
            "      Karsten Hinckfuss\n"
            "      Steffen Kuthe\n"
            "      Jan-Henrik Baumgarten\n"
            "      Robert Bernecky\n"
            "      Theo van Klaveren\n");

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
USGprintVersion ()
{
    DBUG_ENTER ("USGprintVersion");

    printf ("sac2c %s rev %s %s  (%s %s)\n",
            (global.version_id[0] == '\0') ? "???" : global.version_id,
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

    printf ("\n          SAC - Single Assignment C\n"
            "    ---------------------------------------------\n\n"

            "NAME:          sac2c\n"
            "VERSION:       %s\n"
            "PLATFORM:      %s\n"
            "\n"

            "BUILD:         %s\n"
            "BY USER:       %s\n"
            "ON HOST:       %s\n"
            "FOR OS:        %s\n"
            "\n"

            "\n",
            (global.version_id[0] == '\0') ? "???" : global.version_id,
            (global.target_platform[0] == '\0') ? "???" : global.target_platform,
            (build_date[0] == '\0') ? "???" : build_date,
            (build_user[0] == '\0') ? "???" : build_user,
            (build_host[0] == '\0') ? "???" : build_host,
            (build_os[0] == '\0') ? "???" : build_os);

    printf ("(c) Copyright 1994 - 2005 by\n\n"

            "  SaC Development Team\n\n"

            "  http://www.sac-home.org\n"
            "  email:info@sac-home.org\n");

    DBUG_VOID_RETURN;
}

void
USGprintCopyright ()
{
    DBUG_ENTER ("USGprintCopyright");

    printf ("\n          SAC - Single Assignment C\n"
            "    ---------------------------------------------\n\n"

            "    SAC COPYRIGHT NOTICE, LICENSE, AND DISCLAIMER\n\n"

            "(c) Copyright 1994 - 2005 by\n\n"

            "  SaC Development Team\n\n"

            "  http://www.sac-home.org\n"
            "  email:info@sac-home.org\n\n");

    printf (
      "The SAC compiler, the SAC standard library, and all accompanying\n"
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
