/*
 *
 * $Log$
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

#include "usage.h"
#include "globals.h"
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

            "NAME:     \tsac2c\n\n"

            "BUILD:    \t%s\n\n"

            "VERSION:  \t%s for %s\n\n"

            "SYNOPSIS: \tsac2c [options] [filename]\n",
            build_date_time, version_id, target_platform);

    printf ("\n\nDESCRIPTION:\n\n"

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

            "\t -h\t\tdisplay this helptext\n"
            "\t -i\t\tdisplay copyright/disclaimer\n\n"
            "\t -libstat\tprint status information about a SAC library file\n"
            "\t -M\t\tonly detect dependencies from imported\n"
            "\t\t\t  modules/classes and write them to stdout\n"
            "\t\t\t  in a way suitable for the make utility.\n"
            "\t\t\t  Dependences from declaration files are\n"
            "\t\t\t  considered exclusively.\n"
            "\t -Mlib\t\tonly detect dependencies from imported\n"
            "\t\t\t  modules/classes and write them to stdout\n"
            "\t\t\t  in a way suitable for the make utility.\n"
            "\t\t\t  Dependences from declaration files as well as\n"
            "\t\t\t  library files are (recursively) considered.\n"

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
            "\t\t\t  3: full compile time information (default)\n");

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

            "\t-b15:inl      \tstop after function inlining\n"
            "\t-b15:dfr      \tstop after dead function removal\n"
            "\t-b15:ae       \tstop after array elimination\n"
            "\t-b15:dcr      \tstop after dead code removal\n"
            "\t-b15:cycN:cse \tstop after common subexpression elimination in cycle N\n"
            "\t-b15:cycN:cf  \tstop after constant folding in cycle N\n"
            "\t-b15:cycN:wlt \tstop after withloop transformation in cycle N\n"
            "\t-b15:cycN:wli \tstop after withloop information gathering in cycle N\n"
            "\t-b15:cycN:wlf \tstop after withloop folding in cycle N\n"
            "\t-b15:cycN:cf2 \tstop after second constant folding in cycle N\n"
            "\t-b15:cycN:dcr \tstop after dead code removal in cycle N\n"
            "\t-b15:cycN:unr \tstop after (with-) loop unrolling in cycle N\n"
            "\t-b15:cycN:uns \tstop after loop unswitching in cycle N\n"
            "\t-b15:cycN:lir \tstop after loop invariant removal in cycle N\n"
            "\n"
            "\t-b18:conv     \tstop after converting\n"
            "\t-b18:cubes    \tstop after cube-building\n"
            "\t-b18:segs     \tstop after choice of segments\n"
            "\t-b18:split    \tstop after splitting\n"
            "\t-b18:block    \tstop after hierarchical blocking\n"
            "\t-b18:ublock   \tstop after unrolling-blocking\n"
            "\t-b18:merge    \tstop after merging\n"
            "\t-b18:opt      \tstop after optimization\n"
            "\t-b18:fit      \tstop after fitting\n"
            "\t-b18:norm     \tstop after normalization\n"
            "\n"
            "\t-b19:spmdinit \tstop after building SPMD blocks\n"
            "\t-b19:spmdopt  \tstop after optimizing SPMD blocks\n"
            "\t-b19:spmdlift \tstop after lifting SPMD blocks\n"
            "\t-b19:syncinit \tstop after building synchronisation blocks\n"
            "\t-b19:syncopt  \tstop after optimizing synchronisation blocks\n");

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
            "\t -noDLAW\tno applications of the distributive law\n"
            "\t -noIVE\t\tno index vector elimination \n"
            "\t -noAE \t\tno array elimination \n"
            "\t -noRCO\t\tno refcount optimization \n"
            "\t -noUIP\t\tno update-in-place \n"
            /*    "\t -noTILE\t\tno tiling (blocking) \n" */
            "\n\tLower case letters may be used instead!\n");

    printf ("\n\t -maxoptcycles <no>\trepeat optimization phase <no> times\n"
            "\t\t\t\t  Default: -maxoptcycles %d\n",
            max_optcycles);
    printf ("\t -maxoptvar <no>\treserve <no> variables for optimization\n"
            "\t\t\t\t  Default: -maxoptvar %d\n",
            optvar);
    printf ("\t -maxinline <no>\tinline recursive functions <no> times\n"
            "\t\t\t\t  Default: -maxinline %d\n",
            inlnum);
    printf ("\t -maxunroll <no>\tunroll loops having no more than <no>\n"
            "\t\t\t\titerations\n"
            "\t\t\t\t  Default: -maxunroll %d\n",
            unrnum);
    printf ("\t -maxwlunroll <no>\tunroll withloops having no more than <no>\n"
            "\t\t\t\telements\n"
            "\t\t\t\t  Default: -maxwlunroll %d\n",
            wlunrnum);
    printf ("\t -minarray <no>\t\ttry array elimination for arrays with length\n"
            "\t\t\t\t<= <no>\n"
            "\t\t\t\t  Default: -minarray %d\n",
            minarray);
    printf ("\t -maxoverload <no>\tfunctions with unknown shape will <no> times\n"
            "\t\t\t\toverloaded\n"
            "\t\t\t\t  Default: -maxoverload %d\n",
            max_overload);

    printf ("\n\nMULTI-THREAD OPTIONS:\n\n"

            "\t -mtstatic <no>\t\tcompile program for multi-threaded execution\n"
            "\t\t\t\twith exact number of threads specified.\n"
            "\t -mtdynamic <no>\tcompile program for multi-threaded execution\n"
            "\t\t\t\twith upper bound for the number of threads\n"
            "\t\t\t\tspecified. The exact number of threads must be\n"
            "\t\t\t\tgiven upon application startup using the\n"
            "\t\t\t\tgeneric command line option '-mt <no>`.\n"
            "\t -mtall <no>\t\tcompile program for multi-threaded execution\n"
            "\t\t\t\tand derive <no>+1 executables, one that is\n"
            "\t\t\t\tcompiled for the dynamic specification of the\n"
            "\t\t\t\tnumber of threads with <no> as upper limit\n"
            "\t\t\t\tand <no> ones using the respective number of\n"
            "\t\t\t\tthreads statically.\n"
            "\t -maxsyncfold <no>\tmaximum number of fold with-loops in a single\n"
            "\t\t\t\tsynchronisation block.\n"
            "\t\t\t\t  Default: -maxsyncfold %d.\n"
            "\t -minparsize <no>\tminimum generator size for parallel execution\n"
            "\t\t\t\tof with-loops.\n"
            "\t\t\t\t  Default: -minparsize %d.\n",
            max_sync_fold, min_parallel_size);

    printf ("\n\nDEBUG OPTIONS:\n\n"

            "\t -dcheck_boundary\tcheck boundary of arrays upon access\n"
            "\t -dnocleanup\t\tdon't remove temporary files and directories\n"
            "\t -dshow_syscall\t\tshow all system calls during compilation\n"
            "\t -dcheck_malloc\t\tcheck success of memory allocations\n"
            "\t -dcccall\t\tgenerate shell script '.sac2c' that contains C\n"
            "\t\t\t\tcompiler call. This implies option -dnocleanup.\n"
            "\n\t -# <str>\t\toptions (string) for DBUG information\n"
            "\t\t\t\t(\"-#<str>\" is equivalent to \"-_DBUG//<str>\")\n"
            "\t -_DBUG<from>/<to>/<str>\n"
            "\t\t\t\tDBUG information only in compiler phases\n"
            "\t\t\t\t<from>..<to>\n"
            "\t\t\t\t  Default: <from> = 1,\n"
            "\t\t\t\t           <to> = last compiler phase\n"
            "\n\t -efence\t\tfor compilation of programs:\n"
            "\t\t\t\t  link executable with ElectricFence\n"
            "\t\t\t\t  (Malloc Debugger)\n"

            "\n\nTRACE OPTIONS:\n\n"

            "\t -trace [amrfpowt]+ \ttrace program execution\n"
            "\t\t\t\t  a: trace all (same as mrfpowt)\n"
            "\t\t\t\t  m: trace memory operations\n"
            "\t\t\t\t  r: trace refcount operations\n"
            "\t\t\t\t  f: trace user defined function calls\n"
            "\t\t\t\t  p: trace primitive function calls\n"
            "\t\t\t\t  o: trace old with-loop execution\n"
            "\t\t\t\t  w: trace new with-loop execution\n"
            "\t\t\t\t  t: trace multi-threading specific operations\n"

            "\n\nPROFILING OPTIONS:\n\n"

            "\t -profile [afilw]+ \tinclude runtime analysis\n"
            "\t\t\t\t  a: analyse all (same as filw)\n"
            "\t\t\t\t  f: analyse time spent in non-inline functions\n"
            "\t\t\t\t  i: analyse time spent in inline functions\n"
            "\t\t\t\t  l: analyse time spent in library functions\n"
            "\t\t\t\t  w: analyse time spent in with-loops\n"

            "\n\nINTRINSIC ARRAY OPERATIONS OPTIONS:\n\n"

            "\t -intrinsic [a+-x/tdcrpo]+ \tuse intrinsic array operations\n"
            "\t\t\t\t\t  a: use all intrinsic operations\n"
            "\t\t\t\t\t     available (same as +-x/tdcrpo)\n"
            "\t\t\t\t\t  +: use intrinsic add\n"
            "\t\t\t\t\t  -: use intrinsic sub\n"
            "\t\t\t\t\t  x: use intrinsic mul\n"
            "\t\t\t\t\t  /: use intrinsic div\n"
            "\t\t\t\t\t  t: use intrinsic take\n"
            "\t\t\t\t\t  d: use intrinsic drop\n"
            "\t\t\t\t\t  c: use intrinsic cat\n"
            "\t\t\t\t\t  r: use intrinsic rotate\n"
            "\t\t\t\t\t  p: use intrinsic psi\n"
            "\t\t\t\t\t  o: use intrinsic type conversion\n"

            "\n\nLINK OPTIONS:\n\n"

            "\t -l <n>\t\t\tlink level for generating SAC library\n"
            "\t\t\t\t  1: compile to one large object file\n"
            "\t\t\t\t  2: compile to archive of object files\n"
            "\t\t\t\t     (default)\n"

            "\n\nC-COMPILER OPTIONS:\n\n"

            "\t -g \t\t\tinclude debug information into object code\n"
            "\t -O [0123] \t\tC compiler level of optimization\n"
            "\t\t\t\t  default: 0\n"
            "\n\tThe actual effects of the above options are C compiler specific!\n"

            "\n\nCUSTOMIZATION\n\n"

            "\t-target <name>\tspecify a particular compilation target.\n"
            "\t\t\tCompilation targets are used to customize sac2c for\n"
            "\t\t\tvarious target architectures, operating systems, and C\n"
            "\t\t\tcompilers.\n"
            "\t\t\tThe target description is read either from the\n"
            "\t\t\tinstallation specific file $SACBASE/runtime/sac2crc or\n"
            "\t\t\tfrom a file named .sac2crc within the user's home\n"
            "\t\t\tdirectory.\n"

            "\n\nENVIRONMENT VARIABLES:\n\n"

            "\t SACBASE\t\tbase directory of SAC installation\n"
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

            "\n\nCONTACT:\n\n"

            "\t World Wide Web: http://www.informatik.uni-kiel.de/~sacbase/\n"
            "\t E-Mail: sacbase@informatik.uni-kiel.de\n"

            "\n\nBUGS:\n\n"

            "\t Bugs ??  We ????\n"
            "\n"
            "\t Do not annotate functions \"inline\" which contain fold-with-loops!\n"
            "\t It leads to the creation of C-code which does not compile properly!\n"
            "\n"
            "\t Unfortunately, two of our optimizations are quite buggy 8-(\n"
            "\t Therefore, we decided to preset -noLIR and -noDLAW in the current\n"
            "\t compiler release!\n"

            "\n");

    DBUG_VOID_RETURN;
}

void
copyright ()
{
    DBUG_ENTER ("copyright");

    printf ("\n\t\tSAC - Single Assignment C\n"
            "\t--------------------------------------------\n"
            "\n"
            "BUILD:   \t%s\n\n"
            "VERSION: \t%s for %s\n"
            "\n"
            "\n"

            "\tSAC COPYRIGHT NOTICE, LICENSE, AND DISCLAIMER\n\n"
            "(c) Copyright 1994 - 1999 by\n\n"

            "  Christian-Albrechts-Universitaet zu Kiel\n"
            "  Institut fuer Informatik und Praktische Mathematik\n"
            "  Preusserstrasse 1 - 9\n"
            "  D-24105 Kiel\n"
            "  Germany\n\n",
            build_date_time, version_id, target_platform);

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
