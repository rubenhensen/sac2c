/*****************************************************************************
 *
 * $Id$
 *
 * file:   gen_startup_code.c
 *
 * prefix: GSC
 *
 * description:
 *
 *   This file provides routines that write C code that is rather independent
 *   from the specific SAC source file. It is essentially required to specify
 *   and initialize the SAC runtime system. For this reason, additional code
 *   is produced at 3 different positions: at the beginning of each C source,
 *   at the beginning of the statement sequence of the main() function, and
 *   right before the return() statement of the main() function.
 *
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "free.h"
#include "dbug.h"
#include "globals.h"
#include "resource.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "NameTuplesUtils.h"
#include "icm2c_std.h"
#include "print.h"
#include "gen_startup_code.h"
#include "internal_lib.h"
#include "renameidentifiers.h"
#include "namespaces.h"

/******************************************************************************
 *
 * function:
 *   int CalcMasterclass(int num_threads)
 *
 * description:
 *   For a given number of thread used, this function calculates the worker
 *   class of the master thread, i.e. the number of worker threads the master
 *   thread has to synchronize itslef with.
 *
 ******************************************************************************/

static int
CalcMasterclass (int num_threads)
{
    unsigned int res;

    DBUG_ENTER ("GSCcalcMasterclass");

    for (res = 1; res < (unsigned int)num_threads; res <<= 1)
        ;

    res >>= 1;

    DBUG_RETURN ((int)res);
}

/******************************************************************************
 *
 * function:
 *   void PrintGlobalSwitches()
 *
 * description:
 *   This function prints macro definitions representing global switches, i.e.
 *   their value always is either 0 (disabled) or 1 (enabled). The value of
 *   any switch triggers the selection of custom code during the inclusion
 *   of the standard header file sac.h.
 *
 ******************************************************************************/

static void
PrintGlobalSwitches ()
{
    DBUG_ENTER ("PrintGlobalSwitches");

    fprintf (global.outfile, "\n\n"
                             "/*\n"
                             " *  Global Switches\n */\n\n");

    fprintf (global.outfile, "#define SAC_DO_CHECK           %d\n",
             (global.doruntimecheck) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_CHECK_TYPE      %d\n",
             (global.runtimecheck.type) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_CHECK_BOUNDARY  %d\n",
             (global.runtimecheck.boundary) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_CHECK_MALLOC    %d\n",
             (global.runtimecheck.malloc) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_CHECK_ERRNO     %d\n",
             (global.runtimecheck.checkerrno) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_CHECK_HEAP      %d\n",
             (global.runtimecheck.heap) ? 1 : 0);
    fprintf (global.outfile, "\n");

    fprintf (global.outfile, "#define SAC_DO_PHM             %d\n",
             (global.optimize.dophm) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_APS             %d\n",
             (global.optimize.doaps) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_DAO             %d\n",
             (global.optimize.dodao) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_MSCA            %d\n",
             (global.optimize.domsca) ? 1 : 0);
    fprintf (global.outfile, "\n");

    fprintf (global.outfile, "#define SAC_DO_PROFILE         %d\n",
             (global.doprofile) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_PROFILE_WITH    %d\n",
             (global.profile.with) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_PROFILE_FUN     %d\n",
             (global.profile.fun) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_PROFILE_INL     %d\n",
             (global.profile.inl) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_PROFILE_LIB     %d\n",
             (global.profile.lib) ? 1 : 0);
    fprintf (global.outfile, "\n");

    fprintf (global.outfile, "#define SAC_DO_TRACE           %d\n",
             (global.dotrace) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_TRACE_REF       %d\n",
             (global.trace.ref) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_TRACE_MEM       %d\n",
             (global.trace.mem) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_TRACE_PRF       %d\n",
             (global.trace.prf) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_TRACE_FUN       %d\n",
             (global.trace.fun) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_TRACE_WL        %d\n",
             (global.trace.wl) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_TRACE_AA        %d\n",
             (global.trace.aa) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_TRACE_MT        %d\n",
             (global.trace.mt) ? 1 : 0);
    fprintf (global.outfile, "\n");

    fprintf (global.outfile, "#define SAC_DO_CACHESIM        %d\n",
             (global.docachesim) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_CACHESIM_ADV    %d\n",
             (global.cachesim.advanced) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_CACHESIM_GLOBAL %d\n",
             (global.cachesim.block) ? 0 : 1);
    fprintf (global.outfile, "#define SAC_DO_CACHESIM_FILE   %d\n",
             (global.cachesim.file) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_CACHESIM_PIPE   %d\n",
             (global.cachesim.pipe) ? 1 : 0);
    fprintf (global.outfile, "#define SAC_DO_CACHESIM_IMDT   %d\n",
             (global.cachesim.immediate) ? 1 : 0);
    fprintf (global.outfile, "\n");

    fprintf (global.outfile, "#define SAC_DO_MULTITHREAD     %d\n",
             (global.num_threads == 1) ? 0 : 1);
    fprintf (global.outfile, "#define SAC_DO_THREADS_STATIC  %d\n",
             (global.num_threads == 0) ? 0 : 1);
    fprintf (global.outfile, "\n");

    fprintf (global.outfile, "#define SAC_DO_COMPILE_MODULE  %d\n",
             ((global.filetype == F_modimp) || (global.filetype == F_classimp)) ? 1 : 0);
    if (global.genlib.c) {
        fprintf (global.outfile, "#define SAC_GENERATE_CLIBRARY\n");
    }
    fprintf (global.outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void PrintProfileData()
 *
 * description:
 *   This function prints macro definitions related to the profiling feature.
 *
 ******************************************************************************/

static void
PrintProfileData ()
{
    int i, j;

    DBUG_ENTER ("PrintProfileData");

    fprintf (global.outfile, "#define SAC_SET_FUN_NAMES    \\\n");
    fprintf (global.outfile, "  {    \\\n");
    fprintf (global.outfile, "    \"%s\"", (global.profile_funnme[0]));
    for (i = 1; i < global.profile_funcntr; i++) {
        fprintf (global.outfile,
                 ",   \\\n"
                 "    \"%s\"",
                 global.profile_funnme[i]);
    };
    fprintf (global.outfile, "   \\\n"
                             "  }\n");

    fprintf (global.outfile, "\n");

    fprintf (global.outfile, "#define SAC_SET_FUN_APPS    \\\n");
    fprintf (global.outfile, "  {    \\\n");
    fprintf (global.outfile, "    %d", global.profile_funapcntr[0]);
    for (i = 1; i < global.profile_funcntr; i++) {
        fprintf (global.outfile,
                 ",   \\\n"
                 "    %d",
                 global.profile_funapcntr[i]);
    }
    fprintf (global.outfile, "   \\\n"
                             "  }\n");

    fprintf (global.outfile, "\n");

    fprintf (global.outfile, "#define SAC_SET_FUN_AP_LINES    \\\n");
    fprintf (global.outfile, "  {    \\\n");
    fprintf (global.outfile, "    {    \\\n");
    fprintf (global.outfile, "      %d", global.profile_funapline[0][0]);
    for (j = 1; j < global.profile_funapcntr[0]; j++) {
        fprintf (global.outfile, ", %d", global.profile_funapline[0][j]);
    }
    fprintf (global.outfile, "   \\\n"
                             "    }");
    for (i = 1; i < global.profile_funcntr; i++) {
        fprintf (global.outfile, ",   \\\n"
                                 "    {     \\\n");
        fprintf (global.outfile, "      %d", global.profile_funapline[i][0]);
        for (j = 1; j < global.profile_funapcntr[i]; j++) {
            fprintf (global.outfile, ", %d", global.profile_funapline[i][j]);
        }
        fprintf (global.outfile, "   \\\n"
                                 "    }");
    }
    fprintf (global.outfile, "   \\\n"
                             "  }");

    fprintf (global.outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void PrintGlobalSettings( node *syntax_tree)
 *
 * description:
 *
 *
 ******************************************************************************/

static void
PrintGlobalSettings (node *syntax_tree)
{
    DBUG_ENTER ("PrintGlobalSettings");

    fprintf (global.outfile, "\n\n/*\n *  Global Settings\n */\n\n");

    fprintf (global.outfile, "#ifndef NULL\n"
                             "#define NULL                      (void*) 0\n"
                             "#endif\n\n");

    fprintf (global.outfile, "#define SAC_SET_INITIAL_MASTER_HEAPSIZE      %d\n",
             global.initial_master_heapsize * 1024);
    fprintf (global.outfile, "#define SAC_SET_INITIAL_WORKER_HEAPSIZE      %d\n",
             global.initial_worker_heapsize * 1024);
    fprintf (global.outfile, "#define SAC_SET_INITIAL_UNIFIED_HEAPSIZE     %d\n\n",
             global.initial_unified_heapsize * 1024);

    fprintf (global.outfile, "#ifndef SAC_SET_MTMODE\n");
    fprintf (global.outfile, "#define SAC_SET_MTMODE               %d\n",
             (int)global.mtmode);
    fprintf (global.outfile, "#endif\n\n");

    fprintf (global.outfile, "#ifndef SAC_SET_THREADS_MAX\n");
    fprintf (global.outfile, "#define SAC_SET_THREADS_MAX          %d\n",
             global.max_threads);
    fprintf (global.outfile, "#endif\n\n");

    fprintf (global.outfile, "#ifndef SAC_SET_THREADS\n");
    fprintf (global.outfile, "#define SAC_SET_THREADS              %d\n",
             global.num_threads);
    fprintf (global.outfile, "#endif\n\n");

    fprintf (global.outfile, "#ifndef SAC_SET_MASTERCLASS\n");
    fprintf (global.outfile, "#define SAC_SET_MASTERCLASS          %d\n",
             CalcMasterclass (global.num_threads));
    fprintf (global.outfile, "#endif\n\n");

    if (global.max_sync_fold == -1) {
        fprintf (global.outfile, "#define SAC_SET_MAX_SYNC_FOLD        %d\n",
                 global.needed_sync_fold);
    } else {
        fprintf (global.outfile, "#define SAC_SET_MAX_SYNC_FOLD        %d\n",
                 global.max_sync_fold);
    }

    fprintf (global.outfile, "#define SAC_SET_NUM_SCHEDULERS       %d\n\n",
             global.max_schedulers);

    fprintf (global.outfile, "#define SAC_SET_CACHE_1_SIZE         %d\n",
             global.config.cache1_size == 0 ? -1 : global.config.cache1_size);
    fprintf (global.outfile, "#define SAC_SET_CACHE_1_LINE         %d\n",
             global.config.cache1_line == 0 ? 4 : global.config.cache1_line);
    fprintf (global.outfile, "#define SAC_SET_CACHE_1_ASSOC        %d\n",
             global.config.cache1_assoc == 0 ? 1 : global.config.cache1_assoc);
    fprintf (global.outfile, "#define SAC_SET_CACHE_1_WRITEPOL     SAC_CS_%s\n",
             global.config.cache1_writepol);
    fprintf (global.outfile, "#define SAC_SET_CACHE_1_MSCA_FACTOR  %.2f\n\n",
             ((float)global.config.cache1_msca_factor) / 100);

    fprintf (global.outfile, "#define SAC_SET_CACHE_2_SIZE         %d\n",
             global.config.cache2_size == 0 ? -1 : global.config.cache2_size);
    fprintf (global.outfile, "#define SAC_SET_CACHE_2_LINE         %d\n",
             global.config.cache2_line == 0 ? 4 : global.config.cache2_line);
    fprintf (global.outfile, "#define SAC_SET_CACHE_2_ASSOC        %d\n",
             global.config.cache2_assoc == 0 ? 1 : global.config.cache2_assoc);
    fprintf (global.outfile, "#define SAC_SET_CACHE_2_WRITEPOL     SAC_CS_%s\n",
             global.config.cache2_writepol);
    fprintf (global.outfile, "#define SAC_SET_CACHE_2_MSCA_FACTOR  %.2f\n\n",
             ((float)global.config.cache2_msca_factor) / 100);

    fprintf (global.outfile, "#define SAC_SET_CACHE_3_SIZE         %d\n",
             global.config.cache3_size == 0 ? -1 : global.config.cache3_size);
    fprintf (global.outfile, "#define SAC_SET_CACHE_3_LINE         %d\n",
             global.config.cache3_line == 0 ? 4 : global.config.cache3_line);
    fprintf (global.outfile, "#define SAC_SET_CACHE_3_ASSOC        %d\n",
             global.config.cache3_assoc == 0 ? 1 : global.config.cache3_assoc);
    fprintf (global.outfile, "#define SAC_SET_CACHE_3_WRITEPOL     SAC_CS_%s\n",
             global.config.cache3_writepol);
    fprintf (global.outfile, "#define SAC_SET_CACHE_3_MSCA_FACTOR  %.2f\n\n",
             ((float)global.config.cache3_msca_factor) / 100);

    fprintf (global.outfile, "#define SAC_SET_CACHESIM_HOST        \"%s\"\n",
             STR_OR_EMPTY (global.cachesim_host));

    if (global.cachesim_file[0] == '\0') {
        fprintf (global.outfile, "#define SAC_SET_CACHESIM_FILE        \"%s.cs\"\n",
                 global.outfilename);
    } else {
        fprintf (global.outfile, "#define SAC_SET_CACHESIM_FILE        \"%s\"\n",
                 global.cachesim_file);
    }

    if (global.cachesim_dir[0] == '\0') {
        fprintf (global.outfile, "#define SAC_SET_CACHESIM_DIR         \"%s\"\n",
                 global.config.tmpdir);
    } else {
        fprintf (global.outfile, "#define SAC_SET_CACHESIM_DIR         \"%s\"\n",
                 global.cachesim_dir);
    }

    fprintf (global.outfile, "#define SAC_SET_MAXFUN               %d\n",
             (global.profile_funcntr));
    fprintf (global.outfile, "#define SAC_SET_MAXFUNAP             %d\n",
             global.profile_funapmax);

    fprintf (global.outfile, "\n");

    if (global.doprofile) {
        PrintProfileData ();
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *
 *
 * description:
 *
 *
 ******************************************************************************/

static void
PrintIncludes ()
{
    DBUG_ENTER ("PrintIncludes");

    fprintf (global.outfile, "\n\n"
                             "/*\n"
                             " *  Includes\n */\n\n");

    fprintf (global.outfile, "\n"
                             "#include \"sac.h\"\n\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *
 *
 * description:
 *
 *
 ******************************************************************************/

static void
PrintDefines ()
{
    DBUG_ENTER ("PrintDefines");

    fprintf (global.outfile, "\n\n"
                             "/*\n"
                             " *  Global Definitions\n"
                             " */\n\n");

    fprintf (global.outfile, "SAC_MT_DEFINE()\n");
    fprintf (global.outfile, "SAC_PF_DEFINE()\n");
    fprintf (global.outfile, "SAC_HM_DEFINE()\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void GSCprintFileHeader( node *syntax_tree)
 *
 * description:
 *
 *
 ******************************************************************************/

void
GSCprintFileHeader (node *syntax_tree)
{
    DBUG_ENTER ("GSCprintFileHeader");

    PrintGlobalSwitches ();
    PrintGlobalSettings (syntax_tree);
    PrintIncludes ();

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void GSCprintInternalInitFileHeader( node *syntax_tree)
 *
 * description:
 *   generates header part of internal_runtime_init.c
 *   used when compiling a c library
 *   code contains part of the startup code from a "real" SAC-executeable
 *
 ******************************************************************************/

void
GSCprintInternalInitFileHeader (node *syntax_tree)
{
    DBUG_ENTER ("GSCprintCWrapperFileHeader");

    PrintGlobalSwitches ();
    PrintGlobalSettings (syntax_tree);

    fprintf (global.outfile, "#undef SAC_DO_COMPILE_MODULE\n");
    fprintf (global.outfile, "#define SAC_DO_COMPILE_MODULE 0\n");

    PrintIncludes ();

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void GSCprintDefines()
 *
 * description:
 *
 *
 * remark:
 *   This function should be called *after* the typedefs have been printed!
 *
 ******************************************************************************/

void
GSCprintDefines ()
{
    DBUG_ENTER ("GSCprintDefines");

    PrintDefines ();

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void GSCprintMainBegin()
 *
 * Description:
 *
 *
 ******************************************************************************/

void
GSCprintMainBegin ()
{
    char *funname;

    DBUG_ENTER ("GSCprintMainBegin");

    funname = RIDobjInitFunctionName (FALSE);
    /* TODO: move the entire creation of objinitfun to gen startupcode ... */

    /* call init function for a c library - no command line available */
    if (global.genlib.c) {
        /* only call obj init function - runtimesystem already initialized */
        INDENT;
        fprintf (global.outfile, "%s( 0 , NULL);\n\n", funname);
    } else {
        INDENT;
        fprintf (global.outfile, "SAC_MT_SETUP_INITIAL();\n");
        INDENT;
        fprintf (global.outfile, "SAC_PF_SETUP();\n");
        INDENT;
        fprintf (global.outfile, "SAC_HM_SETUP();\n");
        INDENT;
        fprintf (global.outfile, "SAC_MT_SETUP();\n");
        INDENT;
        fprintf (global.outfile, "SAC_CS_SETUP();\n");
#ifndef OBJR_DEACTIVATED
        INDENT;
        fprintf (global.outfile, "%s( __argc, __argv);\n\n", funname);
#endif
    }
    funname = ILIBfree (funname);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void GSCprintMainEnd()
 *
 * Description:
 *
 *
 ******************************************************************************/

void
GSCprintMainEnd ()
{
    DBUG_ENTER ("GSCprintMainEnd");

    /*
     * global.outfile is already indented by 2
     */
    INDENT;
    fprintf (global.outfile, "SAC_PF_PRINT();\n");
    INDENT;
    fprintf (global.outfile, "SAC_CS_FINALIZE();\n");
    INDENT;
    fprintf (global.outfile, "SAC_HM_PRINT();\n\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void GSCprintMain()
 *
 * Description:
 *
 *
 ******************************************************************************/

void
GSCprintMain ()
{
    char *res_NT, *mythread_NT;
    types *tmp_type;
    bool print_thread_id
      = (((global.mtmode == MT_createjoin) || (global.mtmode == MT_startstop))
         && (global.optimize.dophm));

    DBUG_ENTER ("GSCprintMain");

    INDENT;
    fprintf (global.outfile, "int main( int __argc, char *__argv[])\n");
    INDENT;
    fprintf (global.outfile, "{\n");
    global.indent++;
    if (print_thread_id) {
        INDENT;
        fprintf (global.outfile, "SAC_MT_DECL_MYTHREAD()\n");
    }
    tmp_type = TBmakeTypes1 (T_int);
    res_NT = NTUcreateNtTag ("SAC_res", tmp_type);
    mythread_NT = NTUcreateNtTag ("SAC_MT_mythread", tmp_type);
    tmp_type = FREEfreeAllTypes (tmp_type);
    ICMCompileND_DECL (res_NT, "int", 0, NULL); /* create ND_DECL icm */
    GSCprintMainBegin ();

    INDENT;
    fprintf (global.outfile, "SACf_%s__main( ", NSgetName (NSgetRootNamespace ()));

    if (print_thread_id) {
        fprintf (global.outfile, "SAC_ND_ARG_in( %s), ", mythread_NT);
    }
    fprintf (global.outfile, "SAC_ND_ARG_out( %s)", res_NT);
    fprintf (global.outfile, ");\n\n");
    GSCprintMainEnd ();
    INDENT;
    fprintf (global.outfile, "return( SAC_ND_READ( %s, 0));\n", res_NT);
    res_NT = ILIBfree (res_NT);
    mythread_NT = ILIBfree (mythread_NT);
    global.indent--;
    INDENT;
    fprintf (global.outfile, "}\n");

    DBUG_VOID_RETURN;
}
