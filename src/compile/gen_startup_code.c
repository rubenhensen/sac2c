/*
 *
 * $Log$
 * Revision 2.13  1999/07/21 16:29:37  jhs
 * needed_sync_fold introduced, max_sync_fold_adjusted.
 *
 * Revision 2.12  1999/07/20 16:52:32  jhs
 * Added one or two comments.
 *
 * Revision 2.11  1999/07/16 09:34:12  cg
 * Added facilities for heap management diagnostics.
 *
 * Revision 2.10  1999/07/09 12:45:32  cg
 * Basic prerequisites for diagnostic heap management introduced.
 *
 * Revision 2.9  1999/07/09 07:34:16  cg
 * SAC heap manager integrated into sac2c.
 *
 * Revision 2.8  1999/06/11 12:56:07  cg
 * Default settings for options -csfile, -csdir, and -cshost
 * made accessible to cache simulator
 *
 * Revision 2.7  1999/06/04 14:33:15  cg
 * added global setting CACHESIM_HOST.
 *
 * Revision 2.6  1999/05/26 13:21:44  cg
 * Bug fixed in activating cache simulation.
 *
 * Revision 2.5  1999/05/20 14:12:49  cg
 * Now, reasonable default values are set for unspecified cache parameters.
 *
 * Revision 2.4  1999/05/12 14:39:40  cg
 * added new flags to be defined for extended capabilities of
 * cache simulator.
 *
 * Revision 2.3  1999/04/14 09:20:40  cg
 * Settings for cache simulation improved.
 *
 * Revision 2.2  1999/04/06 13:36:09  cg
 * added startup code for cache simulation
 *
 * Revision 2.1  1999/02/23 12:42:32  sacbase
 * new release made
 *
 * Revision 1.17  1999/02/22 12:54:34  cg
 * SPMD frame is now printed iff multi-threaded code is desired.
 *
 * Revision 1.16  1999/01/08 17:23:21  cg
 * Bug fixed in generation of SPMD-frames: now imported functions
 * with body are also checked for containing SPMD-blocks.
 *
 * Revision 1.15  1998/12/07 17:32:24  cg
 * Now, the platform identification is taken from the global
 * variable target_platform.
 *
 * Revision 1.14  1998/12/07 09:59:00  cg
 * added switch for target platform for multi-platform sac2c
 *
 * Revision 1.13  1998/10/29 16:55:03  cg
 * Bug fixed in PrintSpmdData():
 * works now also if no functions are present at all,
 * e.g. in the case of module implementations
 *
 * Revision 1.12  1998/07/07 13:43:12  cg
 * Global flags SAC_DO_MULTITHREAD, SAC_DO_THREADS_STATC and
 * settings for multithreaded execution may now be set by the C
 * compiler instead of being fixed in the C source.
 * This was necessary to implement the -mt-all command line option.
 *
 * Revision 1.11  1998/07/03 10:18:15  cg
 * Super ICM MT_SPMD_BLOCK replaced by combinations of new ICMs
 * MT_SPMD_[STATIC|DYNAMIC]_MODE_[BEGIN|ALTSEQ|END]
 * MT_SPMD_SETUP and MT_SPMD_EXECUTE
 *
 * Revision 1.10  1998/06/29 08:52:19  cg
 * streamlined tracing facilities
 * tracing on new with-loop and multi-threading operations implemented
 *
 * Revision 1.9  1998/06/25 08:08:37  cg
 * definition of NULL-pointer added.
 *
 * Revision 1.8  1998/06/23 12:47:21  cg
 * Now, the correct spmd-function name is used for the specification
 * of the spmd frame.
 *
 * Revision 1.7  1998/05/15 09:21:35  cg
 * tag inout renamed to inout_rc since these arguments are alway
 * refcounted.
 *
 * Revision 1.6  1998/05/12 12:35:58  dkr
 * SPMD_ICM can now be NULL (temporary ?!? needed for non-MT-version)
 *
 * Revision 1.5  1998/05/12 12:27:04  dkr
 * assert in GSCicm changed into if-statement:
 *     if (0==strcmp(ICM_NAME(arg_node), MT_SPMD_BLOCK)) {
 * (GSCicm is called not only for MT_SPMD_BLOCK ICMs !!!)
 *
 * Revision 1.4  1998/05/11 09:51:22  cg
 * added definition of SPMD frame
 *
 * Revision 1.3  1998/05/08 09:04:34  cg
 * The syntax tree is now given as an argument to function GSCPrintFileHeader()
 *
 * Revision 1.2  1998/05/07 08:08:26  cg
 * revised version
 *
 * Revision 1.1  1998/03/24 14:33:35  cg
 * Initial revision
 *
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
 *   This file provides routines that write C code that is rather independent from
 *   the specific SAC source file. It is essentially required to specify
 *   and initialize the SAC runtime system. For this reason, additional code
 *   is produced at 3 different positions: at the beginning of each C source,
 *   at the beginning of the statement sequence of the main() function, and
 *   right before the return() statement of the main() function.
 *
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "dbug.h"
#include "globals.h"
#include "resource.h"
#include "types.h"
#include "tree_basic.h"
#include "traverse.h"

/******************************************************************************
 *
 * global variable: static int spmd_block_counter
 *
 * description:
 *
 *   This variable is used to detect whether a given function definition contains
 *   any SPMD blocks. If not, a dummy entry has to be inserted at the respective
 *   position of the global SPMD frame.
 *
 ******************************************************************************/

static int spmd_block_counter;

/******************************************************************************
 *
 * function:
 *   int GSCCalcMasterclass(int num_threads)
 *
 * description:
 *
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

    for (res = 1; res < num_threads; res <<= 1)
        ;

    res >>= 1;

    DBUG_RETURN ((int)res);
}

/******************************************************************************
 *
 * function:
 *   void PrintTargetPlatform()
 *
 * description:
 *
 *   This function prints macro definitions concerning the respective
 *   target platform.
 *
 *
 ******************************************************************************/

static void
PrintTargetPlatform ()
{
    DBUG_ENTER ("PrintTargetPlatform");

    fprintf (outfile, "/*\n *  Target Platform\n */\n\n");

    fprintf (outfile, "#define SAC_FOR_%s\n", target_platform);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void PrintGlobalSwitches()
 *
 * description:
 *
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

    fprintf (outfile, "\n\n/*\n *  Global Switches\n */\n\n");

    fprintf (outfile, "#define SAC_DO_CHECK           %d\n", runtimecheck ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_CHECK_MALLOC    %d\n",
             (runtimecheck & RUNTIMECHECK_MALLOC) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_CHECK_BOUNDARY  %d\n",
             (runtimecheck & RUNTIMECHECK_BOUNDARY) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_CHECK_ERRNO     %d\n",
             (runtimecheck & RUNTIMECHECK_ERRNO) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_CHECK_HEAP      %d\n",
             (runtimecheck & RUNTIMECHECK_HEAP) ? 1 : 0);
    fprintf (outfile, "\n");

    fprintf (outfile, "#define SAC_DO_PHM             %d\n",
             (optimize & OPT_PHM) ? 1 : 0);
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
    fprintf (outfile, "#define SAC_DO_TRACE_OWL       %d\n",
             (traceflag & TRACE_OWL) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_TRACE_WL        %d\n",
             (traceflag & TRACE_WL) ? 1 : 0);
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

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void PrintSpmdData(node *syntax_tree)
 *
 * description:
 *
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
    funptr *old_tab;

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
 *
 *   This function prints macro definitions related to the profiling feature.
 *
 *
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
        fprintf (outfile, ",   \\\n    \"%s\"", PFfunnme[i]);
    };
    fprintf (outfile, "   \\\n  }\n");

    fprintf (outfile, "\n");

    fprintf (outfile, "#define SAC_SET_FUN_APPS    \\\n");
    fprintf (outfile, "  {    \\\n");
    fprintf (outfile, "    %d", PFfunapcntr[0]);
    for (i = 1; i < PFfuncntr; i++) {
        fprintf (outfile, ",   \\\n    %d", PFfunapcntr[i]);
    }
    fprintf (outfile, "   \\\n  }\n");

    fprintf (outfile, "\n");

    fprintf (outfile, "#define SAC_SET_FUN_AP_LINES    \\\n");
    fprintf (outfile, "  {    \\\n");
    fprintf (outfile, "    {    \\\n");
    fprintf (outfile, "      %d", PFfunapline[0][0]);
    for (j = 1; j < PFfunapcntr[0]; j++) {
        fprintf (outfile, ", %d", PFfunapline[0][j]);
    }
    fprintf (outfile, "   \\\n    }");
    for (i = 1; i < PFfuncntr; i++) {
        fprintf (outfile, ",   \\\n    {     \\\n");
        fprintf (outfile, "      %d", PFfunapline[i][0]);
        for (j = 1; j < PFfunapcntr[i]; j++) {
            fprintf (outfile, ", %d", PFfunapline[i][j]);
        }
        fprintf (outfile, "   \\\n    }");
    }
    fprintf (outfile, "   \\\n  }");

    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void PrintGlobalSettings(node *syntax_tree)
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

static void
PrintGlobalSettings (node *syntax_tree)
{
    DBUG_ENTER ("PrintGlobalSettings");

    fprintf (outfile, "\n\n/*\n *  Global Settings\n */\n\n");

    fprintf (outfile, "#define NULL                      (void*) 0\n\n");

    fprintf (outfile, "#define SAC_SET_INITIAL_HEAPSIZE  %d\n\n", initial_heapsize);

    fprintf (outfile, "#ifndef SAC_SET_THREADS_MAX\n");
    fprintf (outfile, "#define SAC_SET_THREADS_MAX       %d\n", max_threads);
    fprintf (outfile, "#endif\n\n");

    fprintf (outfile, "#ifndef SAC_SET_THREADS\n");
    fprintf (outfile, "#define SAC_SET_THREADS           %d\n", num_threads);
    fprintf (outfile, "#endif\n\n");

    fprintf (outfile, "#ifndef SAC_SET_MASTERCLASS\n");
    fprintf (outfile, "#define SAC_SET_MASTERCLASS       %d\n",
             GSCCalcMasterclass (num_threads));
    fprintf (outfile, "#endif\n\n");

    if (max_sync_fold == -1) {
        fprintf (outfile, "#define SAC_SET_MAX_SYNC_FOLD     %d\n\n", needed_sync_fold);
    } else {
        fprintf (outfile, "#define SAC_SET_MAX_SYNC_FOLD     %d\n\n", max_sync_fold);
    }

    fprintf (outfile, "#define SAC_SET_CACHE_1_SIZE      %d\n", config.cache1_size);
    fprintf (outfile, "#define SAC_SET_CACHE_1_LINE      %d\n",
             config.cache1_line == 0 ? 4 : config.cache1_line);
    fprintf (outfile, "#define SAC_SET_CACHE_1_ASSOC     %d\n",
             config.cache1_assoc == 0 ? 1 : config.cache1_assoc);
    fprintf (outfile, "#define SAC_SET_CACHE_1_WRITEPOL  SAC_CS_%s\n\n",
             config.cache1_writepol);

    fprintf (outfile, "#define SAC_SET_CACHE_2_SIZE      %d\n", config.cache2_size);
    fprintf (outfile, "#define SAC_SET_CACHE_2_LINE      %d\n",
             config.cache2_line == 0 ? 4 : config.cache2_line);
    fprintf (outfile, "#define SAC_SET_CACHE_2_ASSOC     %d\n",
             config.cache2_assoc == 0 ? 1 : config.cache2_assoc);
    fprintf (outfile, "#define SAC_SET_CACHE_2_WRITEPOL  SAC_CS_%s\n\n",
             config.cache2_writepol);

    fprintf (outfile, "#define SAC_SET_CACHE_3_SIZE      %d\n", config.cache3_size);
    fprintf (outfile, "#define SAC_SET_CACHE_3_LINE      %d\n",
             config.cache3_line == 0 ? 4 : config.cache3_line);
    fprintf (outfile, "#define SAC_SET_CACHE_3_ASSOC     %d\n",
             config.cache3_assoc == 0 ? 1 : config.cache3_assoc);
    fprintf (outfile, "#define SAC_SET_CACHE_3_WRITEPOL  SAC_CS_%s\n\n",
             config.cache3_writepol);

    fprintf (outfile, "#define SAC_SET_CACHESIM_HOST     \"%s\"\n",
             cachesim_host[0] == '\0' ? "" : cachesim_host);

    if (cachesim_file[0] == '\0') {
        fprintf (outfile, "#define SAC_SET_CACHESIM_FILE     \"%s.cs\"\n", outfilename);
    } else {
        fprintf (outfile, "#define SAC_SET_CACHESIM_FILE     \"%s\"\n", cachesim_file);
    }

    if (cachesim_dir[0] == '\0') {
        fprintf (outfile, "#define SAC_SET_CACHESIM_DIR      \"%s\"\n", config.tmpdir);
    } else {
        fprintf (outfile, "#define SAC_SET_CACHESIM_DIR      \"%s\"\n", cachesim_dir);
    }

    fprintf (outfile, "#define SAC_SET_MAXFUN            %d\n", PFfuncntr);
    fprintf (outfile, "#define SAC_SET_MAXFUNAP          %d\n", PFfunapmax);

    fprintf (outfile, "\n");

    PrintProfileData ();

    if (gen_mt_code) {
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
 *
 *
 *
 ******************************************************************************/

static void
PrintIncludes ()
{
    DBUG_ENTER ("PrintIncludes");

    fprintf (outfile, "\n\n/*\n *  Includes\n */\n\n");

    fprintf (outfile, "\n#include \"sac.h\"\n\n");

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
 *
 *
 *
 ******************************************************************************/

static void
PrintDefines ()
{
    DBUG_ENTER ("PrintDefines");

    fprintf (outfile, "\n\n/*\n *  Global Definitions\n */\n\n");

    fprintf (outfile, "SAC_MT_DEFINE()\n");
    fprintf (outfile, "SAC_PF_DEFINE()\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   node *GSCicm(node *arg_node, node *arg_info)
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

    if (0 == strcmp (ICM_NAME (arg_node), "MT_SPMD_SETUP")) {

        DBUG_ASSERT ((ICM_ARGS (arg_node) != NULL),
                     "ICM MT_SPMD_BLOCK has wrong format (args missing)");
        DBUG_ASSERT ((EXPRS_NEXT (ICM_ARGS (arg_node)) != NULL),
                     "ICM MT_SPMD_BLOCK has wrong format (name missing)");
        DBUG_ASSERT ((EXPRS_NEXT (EXPRS_NEXT (ICM_ARGS (arg_node))) != NULL),
                     "ICM MT_SPMD_BLOCK has wrong format (numargs missing)");

        icm_arg = EXPRS_NEXT (EXPRS_NEXT (ICM_ARGS (arg_node)));

        while (icm_arg != NULL) {

            DBUG_ASSERT ((EXPRS_EXPR (icm_arg) != NULL),
                         "ICM MT_SPMD_BLOCK has wrong format (tag missing)");
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (icm_arg)) == N_id),
                         "ICM MT_SPMD_BLOCK has wrong format (tag missing)");

            tag = ID_NAME (EXPRS_EXPR (icm_arg));

            DBUG_ASSERT ((EXPRS_NEXT (icm_arg) != NULL),
                         "ICM MT_SPMD_BLOCK has wrong format (type missing)");
            DBUG_ASSERT ((EXPRS_EXPR (EXPRS_NEXT (icm_arg)) != NULL),
                         "ICM MT_SPMD_BLOCK has wrong format (type missing)");
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (EXPRS_NEXT (icm_arg))) == N_id),
                         "ICM MT_SPMD_BLOCK has wrong format (type missing)");

            type = ID_NAME (EXPRS_EXPR (EXPRS_NEXT (icm_arg)));

            DBUG_ASSERT ((EXPRS_NEXT (EXPRS_NEXT (icm_arg)) != NULL),
                         "ICM MT_SPMD_BLOCK has wrong format (parameter missing)");
            DBUG_ASSERT ((EXPRS_EXPR (EXPRS_NEXT (EXPRS_NEXT (icm_arg))) != NULL),
                         "ICM MT_SPMD_BLOCK has wrong format (parameter missing)");
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (EXPRS_NEXT (EXPRS_NEXT (icm_arg))))
                          == N_id),
                         "ICM MT_SPMD_BLOCK has wrong format (parameter missing)");

            name = ID_NAME (EXPRS_EXPR (EXPRS_NEXT (EXPRS_NEXT (icm_arg))));

            fprintf (outfile, "        SAC_MT_SPMD_ARG_%s( %s, %s)    \\\n", tag, type,
                     name);

            icm_arg = EXPRS_NEXT (EXPRS_NEXT (EXPRS_NEXT (icm_arg)));
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *GSCspmd(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
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
 *
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

node *
GSCfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("GSCfundef");

    if ((FUNDEF_STATUS (arg_node) == ST_regular)
        /*      || (FUNDEF_STATUS(arg_node) == ST_foldfun) */
        || ((FUNDEF_STATUS (arg_node) == ST_imported)
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
 *
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

void
GSCPrintFileHeader (node *syntax_tree)
{
    DBUG_ENTER ("GSCPrintFileHeader");

    PrintTargetPlatform ();
    PrintGlobalSwitches ();
    PrintGlobalSettings (syntax_tree);
    PrintIncludes ();
    PrintDefines ();

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
 *
 *
 *
 ******************************************************************************/

void
GSCPrintMainBegin ()
{
    DBUG_ENTER ("GSCPrintMainBegin");

    fprintf (outfile, "  SAC_HM_SETUP();\n");
    fprintf (outfile, "  SAC_PF_SETUP();\n");
    fprintf (outfile, "  SAC_MT_SETUP();\n");
    fprintf (outfile, "  SAC_CS_SETUP();\n\n");

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
 *
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
    fprintf (outfile, "\n  SAC_PF_PRINT();\n");
    fprintf (outfile, "  SAC_CS_FINALIZE();\n");
    fprintf (outfile, "  SAC_HM_PRINT();\n\n");

    DBUG_VOID_RETURN;
}
