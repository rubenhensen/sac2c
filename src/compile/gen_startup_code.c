/*
 *
 * $Log$
 * Revision 3.30  2003/09/13 13:43:56  dkr
 * GSCicm(): NT-tags added for TAGGED_ARRAYS
 *
 * Revision 3.29  2003/08/05 16:16:18  dkr
 * fixed a bug in  GSCPrintMain()
 *
 * Revision 3.28  2003/08/04 18:03:59  dkr
 * GSCPrintMain(): tags for SAC_MT_mythread added
 *
 * Revision 3.27  2003/08/04 14:30:52  dkr
 * GSCicm(): error messages for MT_SPMD_SETUP corrected
 *
 * Revision 3.26  2003/04/14 14:55:54  sbs
 * num_threads casted to unsigned int for correct comparison.
 *
 * Revision 3.25  2003/03/21 18:05:29  sbs
 * PrintTargetPlatform eliminated.
 *
 * Revision 3.24  2003/03/13 17:10:49  dkr
 * support for the -minarrayrep flag added to GSCPrintMain()
 *
 * Revision 3.23  2003/03/09 19:15:59  dkr
 * TRACE_AA added
 *
 * Revision 3.22  2002/11/08 13:29:45  cg
 * Removed TRACE_OWL macro since old with-loops left sac2c several
 * years ago.  :-))))
 *
 * Revision 3.21  2002/10/04 14:12:15  cg
 * Non-existent or unspecified caches are now specified by cache size -1
 * instead of 0. This avoids nasty warnings on the Alpha system.
 *
 * Revision 3.20  2002/09/11 23:14:17  dkr
 * renaming of function names modified
 *
 * Revision 3.19  2002/08/07 10:07:25  dkr
 * bug in GSCPrintMain() fixed: SAC_argc/v renamed into __argc/v
 *
 * Revision 3.18  2002/08/06 08:52:31  dkr
 * cc warning eliminated
 *
 * Revision 3.17  2002/07/30 16:09:34  dkr
 * GSCPrintMain() modified:
 * CreateNtTag() used to create the tag for the argument of SACf_main_()
 *
 * Revision 3.16  2002/07/03 15:34:38  dkr
 * some spaces added :)
 *
 * Revision 3.15  2002/07/03 15:28:01  dkr
 * RUNTIMECHECK_TYPE added (for TAGGED_ARRAYS)
 *
 * Revision 3.14  2002/06/02 21:39:47  dkr
 * support for TAGGED_ARRAYS added
 *
 * Revision 3.13  2002/04/16 21:12:48  dkr
 * GSCPrintMain() added
 *
 * Revision 3.12  2002/04/16 18:39:21  dkr
 * signature of ObjInitFunctionName() modified
 *
 * Revision 3.11  2002/03/07 18:52:43  dkr
 * GSCPrintMainEnd(): a '\n' removed
 *
 * Revision 3.10  2001/05/21 12:44:03  ben
 * SAC_MT_SET_MAX_SCHEDULERS renamed to SAC_MR_SET_NUM_SCHEDULERS
 *
 * Revision 3.9  2001/05/17 12:08:44  dkr
 * FREE, MALLOC eliminated
 *
 * Revision 3.8  2001/05/14 10:21:20  cg
 * Added new setting SAC_SET_MAX_SCHEDULERS to make maximum number
 * of schedulings within single SPMD function available to macro
 * implementations of schedulers
 *
 * Revision 3.7  2001/04/24 09:39:58  dkr
 * CHECK_NULL renamed into STR_OR_EMPTY
 *
 * Revision 3.6  2001/04/03 12:06:43  dkr
 * GSCPrintDefines added
 *
 * Revision 3.5  2001/04/02 15:24:17  dkr
 * no changes done
 *
 * Revision 3.4  2001/03/29 01:40:11  dkr
 * CHECK_NULL used
 *
 * Revision 3.3  2001/02/12 21:50:33  dkr
 * GSCfundef: comment added
 *
 * Revision 3.2  2000/12/08 10:11:42  nmw
 * no more warnings on alpha
 *
 * [...]
 *
 * Revision 1.1  1998/03/24 14:33:35  cg
 * Initial revision
 *
 */

/*****************************************************************************
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

#include "dbug.h"
#include "globals.h"
#include "resource.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "NameTuplesUtils.h"
#include "precompile.h"
#include "icm2c_std.h"
#include "gen_startup_code.h"

/******************************************************************************
 *
 * global variable: static int spmd_block_counter
 *
 * description:
 *   This variable is used to detect whether a given function definition
 *   contains any SPMD blocks. If not, a dummy entry has to be inserted at
 *   the respective position of the global SPMD frame.
 *
 ******************************************************************************/

static int spmd_block_counter;

/******************************************************************************
 *
 * function:
 *   int GSCCalcMasterclass(int num_threads)
 *
 * description:
 *   For a given number of thread used, this function calculates the worker
 *   class of the master thread, i.e. the number of worker threads the master
 *   thread has to synchronize itslef with.
 *
 ******************************************************************************/

int
GSCCalcMasterclass (int num_threads)
{
    unsigned int res;

    DBUG_ENTER ("GSCCalcMasterclass");

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

    fprintf (outfile, "\n\n"
                      "/*\n"
                      " *  Global Switches\n */\n\n");

    fprintf (outfile, "#define SAC_DO_CHECK           %d\n", runtimecheck ? 1 : 0);
#ifdef TAGGED_ARRAYS
    fprintf (outfile, "#define SAC_DO_CHECK_TYPE      %d\n",
             (runtimecheck & RUNTIMECHECK_TYPE) ? 1 : 0);
#endif
    fprintf (outfile, "#define SAC_DO_CHECK_BOUNDARY  %d\n",
             (runtimecheck & RUNTIMECHECK_BOUNDARY) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_CHECK_MALLOC    %d\n",
             (runtimecheck & RUNTIMECHECK_MALLOC) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_CHECK_ERRNO     %d\n",
             (runtimecheck & RUNTIMECHECK_ERRNO) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_CHECK_HEAP      %d\n",
             (runtimecheck & RUNTIMECHECK_HEAP) ? 1 : 0);
    fprintf (outfile, "\n");

    fprintf (outfile, "#define SAC_DO_PHM             %d\n",
             (optimize & OPT_PHM) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_APS             %d\n",
             (optimize & OPT_APS) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_RCAO            %d\n",
             (optimize & OPT_RCAO) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_MSCA            %d\n",
             (optimize & OPT_MSCA) ? 1 : 0);
    fprintf (outfile, "\n");

    fprintf (outfile, "#define SAC_DO_PROFILE         %d\n", profileflag ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_PROFILE_WITH    %d\n",
             (profileflag & PROFILE_WITH) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_PROFILE_FUN     %d\n",
             (profileflag & PROFILE_FUN) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_PROFILE_INL     %d\n",
             (profileflag & PROFILE_INL) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_PROFILE_LIB     %d\n",
             (profileflag & PROFILE_LIB) ? 1 : 0);
    fprintf (outfile, "\n");

    fprintf (outfile, "#define SAC_DO_TRACE           %d\n", traceflag ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_TRACE_REF       %d\n",
             (traceflag & TRACE_REF) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_TRACE_MEM       %d\n",
             (traceflag & TRACE_MEM) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_TRACE_PRF       %d\n",
             (traceflag & TRACE_PRF) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_TRACE_FUN       %d\n",
             (traceflag & TRACE_FUN) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_TRACE_WL        %d\n",
             (traceflag & TRACE_WL) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_TRACE_AA        %d\n",
             (traceflag & TRACE_AA) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_TRACE_MT        %d\n",
             (traceflag & TRACE_MT) ? 1 : 0);
    fprintf (outfile, "\n");

    fprintf (outfile, "#define SAC_DO_CACHESIM        %d\n",
             (cachesim & CACHESIM_YES) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_CACHESIM_ADV    %d\n",
             (cachesim & CACHESIM_ADVANCED) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_CACHESIM_GLOBAL %d\n",
             (cachesim & CACHESIM_BLOCK) ? 0 : 1);
    fprintf (outfile, "#define SAC_DO_CACHESIM_FILE   %d\n",
             (cachesim & CACHESIM_FILE) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_CACHESIM_PIPE   %d\n",
             (cachesim & CACHESIM_PIPE) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_CACHESIM_IMDT   %d\n",
             (cachesim & CACHESIM_IMMEDIATE) ? 1 : 0);
    fprintf (outfile, "\n");

    fprintf (outfile, "#define SAC_DO_MULTITHREAD     %d\n", (num_threads == 1) ? 0 : 1);
    fprintf (outfile, "#define SAC_DO_THREADS_STATIC  %d\n", (num_threads == 0) ? 0 : 1);
    fprintf (outfile, "\n");

    fprintf (outfile, "#define SAC_DO_COMPILE_MODULE  %d\n",
             ((filetype == F_modimp) || (filetype == F_classimp)) ? 1 : 0);
    if (generatelibrary & GENERATELIBRARY_C) {
        fprintf (outfile, "#define SAC_GENERATE_CLIBRARY\n");
    }
    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void PrintSpmdData( node *syntax_tree)
 *
 * description:
 *   This function initializes an additional traversal of the syntax tree
 *   during which the so-called SPMD-frame is infered and printed. This
 *   SPMD-frame is required as temporary storage for arguments in the
 *   specialized parameter passing mechanism of SPMD functions. Actually,
 *   a union type is built over all SPMD functions. For each SPMD function
 *   it contains a struct of all parameters. The SPMD frame itself is nothing
 *   but a global variable of this type. This simple solution exploits the
 *   fact that SPMD functions may never be called in a nested way.
 *
 ******************************************************************************/

static void
PrintSpmdData (node *syntax_tree)
{
    funtab *old_tab;

    DBUG_ENTER ("PrintSpmdData");

    old_tab = act_tab;
    act_tab = gsc_tab;

    fprintf (outfile, "#define SAC_SET_SPMD_FRAME    \\\n");
    fprintf (outfile, "  {    \\\n");

    if (MODUL_FUNS (syntax_tree) != NULL) {
        Trav (MODUL_FUNS (syntax_tree), NULL);
    }

    fprintf (outfile, "  }\n\n");

    act_tab = old_tab;

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

    fprintf (outfile, "#define SAC_SET_FUN_NAMES    \\\n");
    fprintf (outfile, "  {    \\\n");
    fprintf (outfile, "    \"%s\"", PFfunnme[0]);
    for (i = 1; i < PFfuncntr; i++) {
        fprintf (outfile,
                 ",   \\\n"
                 "    \"%s\"",
                 PFfunnme[i]);
    };
    fprintf (outfile, "   \\\n"
                      "  }\n");

    fprintf (outfile, "\n");

    fprintf (outfile, "#define SAC_SET_FUN_APPS    \\\n");
    fprintf (outfile, "  {    \\\n");
    fprintf (outfile, "    %d", PFfunapcntr[0]);
    for (i = 1; i < PFfuncntr; i++) {
        fprintf (outfile,
                 ",   \\\n"
                 "    %d",
                 PFfunapcntr[i]);
    }
    fprintf (outfile, "   \\\n"
                      "  }\n");

    fprintf (outfile, "\n");

    fprintf (outfile, "#define SAC_SET_FUN_AP_LINES    \\\n");
    fprintf (outfile, "  {    \\\n");
    fprintf (outfile, "    {    \\\n");
    fprintf (outfile, "      %d", PFfunapline[0][0]);
    for (j = 1; j < PFfunapcntr[0]; j++) {
        fprintf (outfile, ", %d", PFfunapline[0][j]);
    }
    fprintf (outfile, "   \\\n"
                      "    }");
    for (i = 1; i < PFfuncntr; i++) {
        fprintf (outfile, ",   \\\n"
                          "    {     \\\n");
        fprintf (outfile, "      %d", PFfunapline[i][0]);
        for (j = 1; j < PFfunapcntr[i]; j++) {
            fprintf (outfile, ", %d", PFfunapline[i][j]);
        }
        fprintf (outfile, "   \\\n"
                          "    }");
    }
    fprintf (outfile, "   \\\n"
                      "  }");

    fprintf (outfile, "\n");

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

    fprintf (outfile, "\n\n/*\n *  Global Settings\n */\n\n");

    fprintf (outfile, "#ifndef NULL\n"
                      "#define NULL                      (void*) 0\n"
                      "#endif\n\n");

    fprintf (outfile, "#define SAC_SET_INITIAL_MASTER_HEAPSIZE      %d\n",
             initial_master_heapsize * 1024);
    fprintf (outfile, "#define SAC_SET_INITIAL_WORKER_HEAPSIZE      %d\n",
             initial_worker_heapsize * 1024);
    fprintf (outfile, "#define SAC_SET_INITIAL_UNIFIED_HEAPSIZE     %d\n\n",
             initial_unified_heapsize * 1024);

    fprintf (outfile, "#ifndef SAC_SET_THREADS_MAX\n");
    fprintf (outfile, "#define SAC_SET_THREADS_MAX          %d\n", max_threads);
    fprintf (outfile, "#endif\n\n");

    fprintf (outfile, "#ifndef SAC_SET_THREADS\n");
    fprintf (outfile, "#define SAC_SET_THREADS              %d\n", num_threads);
    fprintf (outfile, "#endif\n\n");

    fprintf (outfile, "#ifndef SAC_SET_MASTERCLASS\n");
    fprintf (outfile, "#define SAC_SET_MASTERCLASS          %d\n",
             GSCCalcMasterclass (num_threads));
    fprintf (outfile, "#endif\n\n");

    if (max_sync_fold == -1) {
        fprintf (outfile, "#define SAC_SET_MAX_SYNC_FOLD        %d\n", needed_sync_fold);
    } else {
        fprintf (outfile, "#define SAC_SET_MAX_SYNC_FOLD        %d\n", max_sync_fold);
    }

    fprintf (outfile, "#define SAC_SET_NUM_SCHEDULERS       %d\n\n", max_schedulers);

    fprintf (outfile, "#define SAC_SET_CACHE_1_SIZE         %d\n",
             config.cache1_size == 0 ? -1 : config.cache1_size);
    fprintf (outfile, "#define SAC_SET_CACHE_1_LINE         %d\n",
             config.cache1_line == 0 ? 4 : config.cache1_line);
    fprintf (outfile, "#define SAC_SET_CACHE_1_ASSOC        %d\n",
             config.cache1_assoc == 0 ? 1 : config.cache1_assoc);
    fprintf (outfile, "#define SAC_SET_CACHE_1_WRITEPOL     SAC_CS_%s\n",
             config.cache1_writepol);
    fprintf (outfile, "#define SAC_SET_CACHE_1_MSCA_FACTOR  %.2f\n\n",
             ((float)config.cache1_msca_factor) / 100);

    fprintf (outfile, "#define SAC_SET_CACHE_2_SIZE         %d\n",
             config.cache2_size == 0 ? -1 : config.cache2_size);
    fprintf (outfile, "#define SAC_SET_CACHE_2_LINE         %d\n",
             config.cache2_line == 0 ? 4 : config.cache2_line);
    fprintf (outfile, "#define SAC_SET_CACHE_2_ASSOC        %d\n",
             config.cache2_assoc == 0 ? 1 : config.cache2_assoc);
    fprintf (outfile, "#define SAC_SET_CACHE_2_WRITEPOL     SAC_CS_%s\n",
             config.cache2_writepol);
    fprintf (outfile, "#define SAC_SET_CACHE_2_MSCA_FACTOR  %.2f\n\n",
             ((float)config.cache2_msca_factor) / 100);

    fprintf (outfile, "#define SAC_SET_CACHE_3_SIZE         %d\n",
             config.cache3_size == 0 ? -1 : config.cache3_size);
    fprintf (outfile, "#define SAC_SET_CACHE_3_LINE         %d\n",
             config.cache3_line == 0 ? 4 : config.cache3_line);
    fprintf (outfile, "#define SAC_SET_CACHE_3_ASSOC        %d\n",
             config.cache3_assoc == 0 ? 1 : config.cache3_assoc);
    fprintf (outfile, "#define SAC_SET_CACHE_3_WRITEPOL     SAC_CS_%s\n",
             config.cache3_writepol);
    fprintf (outfile, "#define SAC_SET_CACHE_3_MSCA_FACTOR  %.2f\n\n",
             ((float)config.cache3_msca_factor) / 100);

    fprintf (outfile, "#define SAC_SET_CACHESIM_HOST        \"%s\"\n",
             STR_OR_EMPTY (cachesim_host));

    if (cachesim_file[0] == '\0') {
        fprintf (outfile, "#define SAC_SET_CACHESIM_FILE        \"%s.cs\"\n",
                 outfilename);
    } else {
        fprintf (outfile, "#define SAC_SET_CACHESIM_FILE        \"%s\"\n", cachesim_file);
    }

    if (cachesim_dir[0] == '\0') {
        fprintf (outfile, "#define SAC_SET_CACHESIM_DIR         \"%s\"\n", config.tmpdir);
    } else {
        fprintf (outfile, "#define SAC_SET_CACHESIM_DIR         \"%s\"\n", cachesim_dir);
    }

    fprintf (outfile, "#define SAC_SET_MAXFUN               %d\n", PFfuncntr);
    fprintf (outfile, "#define SAC_SET_MAXFUNAP             %d\n", PFfunapmax);

    fprintf (outfile, "\n");

    PrintProfileData ();

    if ((gen_mt_code == GEN_MT_OLD) || (gen_mt_code == GEN_MT_NEW)) {
        PrintSpmdData (syntax_tree);
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

    fprintf (outfile, "\n\n"
                      "/*\n"
                      " *  Includes\n */\n\n");

    fprintf (outfile, "\n"
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

    fprintf (outfile, "\n\n"
                      "/*\n"
                      " *  Global Definitions\n"
                      " */\n\n");

    fprintf (outfile, "SAC_MT_DEFINE()\n");
    fprintf (outfile, "SAC_PF_DEFINE()\n");
    fprintf (outfile, "SAC_HM_DEFINE()\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   node *GSCicm( node *arg_node, node *arg_info)
 *
 * description:
 *   filters the variables needed for the spmd-frame of a specific function
 *   from an MT_SPMD_SETUP-icm.
 *
 ******************************************************************************/

node *
GSCicm (node *arg_node, node *arg_info)
{
    node *icm_arg;
    char *tag, *type, *name;

    DBUG_ENTER ("GSCicm");

    if (!strcmp (ICM_NAME (arg_node), "MT_SPMD_SETUP")) {

        DBUG_ASSERT ((ICM_EXPRS1 (arg_node) != NULL),
                     "ICM MT_SPMD_SETUP has wrong format (args missing)");
        DBUG_ASSERT ((ICM_EXPRS2 (arg_node) != NULL),
                     "ICM MT_SPMD_SETUP has wrong format (name missing)");
        DBUG_ASSERT ((ICM_EXPRS3 (arg_node) != NULL),
                     "ICM MT_SPMD_SETUP has wrong format (numargs missing)");

        icm_arg = ICM_EXPRS3 (arg_node);
        while (icm_arg != NULL) {
            DBUG_ASSERT ((EXPRS_EXPR1 (icm_arg) != NULL),
                         "ICM MT_SPMD_SETUP has wrong format (tag missing)");
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR1 (icm_arg)) == N_id),
                         "ICM MT_SPMD_SETUP has wrong format (tag no N_id)");

            tag = ID_NAME (EXPRS_EXPR1 (icm_arg));

            DBUG_ASSERT ((EXPRS_EXPRS2 (icm_arg) != NULL),
                         "ICM MT_SPMD_SETUP has wrong format (type missing)");
            DBUG_ASSERT ((EXPRS_EXPR2 (icm_arg) != NULL),
                         "ICM MT_SPMD_SETUP has wrong format (type missing)");
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR2 (icm_arg)) == N_id),
                         "ICM MT_SPMD_SETUP has wrong format (type no N_id)");

            type = ID_NAME (EXPRS_EXPR2 (icm_arg));

            DBUG_ASSERT ((EXPRS_EXPRS3 (icm_arg) != NULL),
                         "ICM MT_SPMD_SETUP has wrong format (parameter missing)");
            DBUG_ASSERT ((EXPRS_EXPR3 (icm_arg) != NULL),
                         "ICM MT_SPMD_SETUP has wrong format (parameter missing)");
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR3 (icm_arg)) == N_id),
                         "ICM MT_SPMD_SETUP has wrong format (parameter no N_id)");
#ifdef TAGGED_ARRAYS
            DBUG_ASSERT ((ID_NT_TAG (EXPRS_EXPR3 (icm_arg)) != NULL),
                         "ICM MT_SPMD_SETUP has wrong format (parameter has no NT-tag)");

            name = ID_NT_TAG (EXPRS_EXPR3 (icm_arg));
#else
            name = ID_NAME (EXPRS_EXPR3 (icm_arg));
#endif

            fprintf (outfile, "        SAC_MT_SPMD_ARG_%s( %s, %s)    \\\n", tag, type,
                     name);

            icm_arg = EXPRS_EXPRS4 (icm_arg);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *GSCspmd( node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
GSCspmd (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("GSCspmd");

    spmd_block_counter++;

    fprintf (outfile, "      SAC_MT_BLOCK_FRAME( %s,  {  \\\n",
             FUNDEF_NAME (SPMD_FUNDEF (arg_node)));

    /*
     * The layout of the SPMD-frame is derived from the MT_SPMD_SETUP ICM.
     * Therefore, we have to traverse the respective block.
     */
    if (SPMD_ICM_PARALLEL (arg_node) != NULL) {
        Trav (SPMD_ICM_PARALLEL (arg_node), arg_info);
    }

    fprintf (outfile, "      })     \\\n");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *GSCfundef( node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
GSCfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("GSCfundef");

    if ((FUNDEF_STATUS (arg_node) == ST_regular) ||
#if 0
      (FUNDEF_IS_LACFUN( arg_node)) ||
#endif
        (FUNDEF_STATUS (arg_node) == ST_exported)
        || (((FUNDEF_STATUS (arg_node) == ST_imported_mod)
             || (FUNDEF_STATUS (arg_node) == ST_imported_class))
            && (FUNDEF_BODY (arg_node) != NULL))) {
        /*
         * Here, we want to check all functions which may contain an SPMD-block.
         */
        fprintf (outfile, "    SAC_MT_FUN_FRAME( %s, {   \\\n", FUNDEF_NAME (arg_node));
        spmd_block_counter = 0;

        Trav (FUNDEF_BODY (arg_node), arg_info);

        /*
         *  if there is no dummy frame, one is inserted in front here
         */
        if (spmd_block_counter == 0) {
            fprintf (outfile, "      SAC_MT_BLOCK_FRAME_DUMMY()    \\\n");
        }

        fprintf (outfile, "    })    \\\n");
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   void GSCPrintFileHeader( node *syntax_tree)
 *
 * description:
 *
 *
 ******************************************************************************/

void
GSCPrintFileHeader (node *syntax_tree)
{
    DBUG_ENTER ("GSCPrintFileHeader");

    PrintGlobalSwitches ();
    PrintGlobalSettings (syntax_tree);

    PrintIncludes ();

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void GSCPrintInternalInitFileHeader( node *syntax_tree)
 *
 * description:
 *   generates header part of internal_runtime_init.c
 *   used when compiling a c library
 *   code contains part of the startup code from a "real" SAC-executeable
 *
 ******************************************************************************/

void
GSCPrintInternalInitFileHeader (node *syntax_tree)
{
    DBUG_ENTER ("GSCPrintCWrapperFileHeader");

    PrintGlobalSwitches ();
    PrintGlobalSettings (syntax_tree);

    fprintf (outfile, "#undef SAC_DO_COMPILE_MODULE\n");
    fprintf (outfile, "#define SAC_DO_COMPILE_MODULE 0\n");

    PrintIncludes ();

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void GSCPrintDefines()
 *
 * description:
 *
 *
 * remark:
 *   This function should be called *after* the typedefs have been printed!
 *
 ******************************************************************************/

void
GSCPrintDefines ()
{
    DBUG_ENTER ("GSCPrintDefines");

    PrintDefines ();

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void GSCPrintMainBegin()
 *
 * Description:
 *
 *
 ******************************************************************************/

void
GSCPrintMainBegin ()
{
    char *funname;

    DBUG_ENTER ("GSCPrintMainBegin");

    funname = ObjInitFunctionName (FALSE);

    /* call init function for a c library - no command line available */
    if (generatelibrary & GENERATELIBRARY_C) {
        /* only call obj init function - runtimesystem already initialized */
        fprintf (outfile, "  %s( 0 , NULL);\n\n", funname);
    } else {
        fprintf (outfile,
                 "  SAC_MT_SETUP_INITIAL();\n"
                 "  SAC_PF_SETUP();\n"
                 "  SAC_HM_SETUP();\n"
                 "  SAC_MT_SETUP();\n"
                 "  SAC_CS_SETUP();\n"
                 "  %s( __argc, __argv);\n\n",
                 funname);
    }
    funname = Free (funname);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void GSCPrintMainEnd()
 *
 * Description:
 *
 *
 ******************************************************************************/

void
GSCPrintMainEnd ()
{
    DBUG_ENTER ("GSCPrintMainEnd");

    /*
     * outfile is already indented by 2
     */
    fprintf (outfile, "  SAC_PF_PRINT();\n");
    fprintf (outfile, "  SAC_CS_FINALIZE();\n");
    fprintf (outfile, "  SAC_HM_PRINT();\n\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void GSCPrintMain()
 *
 * Description:
 *
 *
 ******************************************************************************/

void
GSCPrintMain ()
{
#ifdef TAGGED_ARRAYS
    char *res_nt, *mythread_nt;
    types *tmp_type;
#endif
    bool print_thread_id = ((gen_mt_code == GEN_MT_OLD) && (optimize & OPT_PHM));

    DBUG_ENTER ("GSCPrintMain");

    fprintf (outfile, "int main( int __argc, char *__argv[])\n");
    fprintf (outfile, "{\n");
    if (print_thread_id) {
        fprintf (outfile, "  SAC_MT_DECL_MYTHREAD()\n");
    }
#ifdef TAGGED_ARRAYS
    tmp_type = MakeTypes1 (T_int);
    res_nt = CreateNtTag ("SAC_res", tmp_type);
    mythread_nt = CreateNtTag ("SAC_MT_mythread", tmp_type);
    tmp_type = FreeAllTypes (tmp_type);
    ICMCompileND_DECL (res_nt, "int", 0, NULL); /* create ND_DECL icm */
#else
    fprintf (outfile, "  int SAC_res;\n\n");
#endif
    GSCPrintMainBegin ();

#ifdef TAGGED_ARRAYS
    fprintf (outfile, "  SACf_main( ");
    if (print_thread_id) {
        fprintf (outfile, "SAC_ND_ARG_in( %s), ", mythread_nt);
    }
    fprintf (outfile, "SAC_ND_ARG_out( %s)", res_nt);
#else
    fprintf (outfile, "  SAC_res = SACf_main(");
    if (print_thread_id) {
        fprintf (outfile, " SAC_ND_ARG_in( SAC_MT_mythread)");
    }
#endif
    fprintf (outfile, ");\n\n");
    GSCPrintMainEnd ();
#ifdef TAGGED_ARRAYS
    fprintf (outfile, "  return( SAC_ND_READ( %s, 0));\n", res_nt);
    res_nt = Free (res_nt);
    mythread_nt = Free (mythread_nt);
#else
    fprintf (outfile, "  return( SAC_res);\n");
#endif
    fprintf (outfile, "}\n");

    DBUG_VOID_RETURN;
}
