/*
 *
 * $Log$
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
 *   This file provides routines that write C code that is independent from
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

static int spmd_block_counter;

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

static unsigned int
CalcMasterclass (int num_threads)
{
    unsigned int res;

    DBUG_ENTER ("CalcMasterclass");

    for (res = 1; res < num_threads; res <<= 1)
        ;

    DBUG_RETURN (res >> 1);
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
PrintGlobalSwitches ()
{
    DBUG_ENTER ("PrintGlobalSwitches");

    fprintf (outfile, "\n\n/*\n *  Global Switches\n */\n\n");

    fprintf (outfile, "#define SAC_DO_MALLOCCHECK     %d\n", check_malloc ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_BOUNDCHECK      %d\n", check_boundary ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_PROFILE         %d\n", profileflag ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_PROFILE_WITH    %d\n",
             (profileflag & PROFILE_WITH) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_PROFILE_FUN     %d\n",
             (profileflag & PROFILE_FUN) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_PROFILE_INL     %d\n",
             (profileflag & PROFILE_INL) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_PROFILE_LIB     %d\n",
             (profileflag & PROFILE_LIB) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_TRACE           %d\n", traceflag ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_TRACE_REF       %d\n",
             (traceflag & TRACE_REF) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_TRACE_MEM       %d\n",
             (traceflag & TRACE_MEM) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_TRACE_PRF       %d\n",
             (traceflag & TRACE_PRF) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_TRACE_UDF       %d\n",
             (traceflag & TRACE_UDF) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_TRACE_WST       %d\n",
             (traceflag & TRACE_WST) ? 1 : 0);
    fprintf (outfile, "#define SAC_DO_MULTITHREAD     %d\n", (num_threads == 1) ? 0 : 1);
    fprintf (outfile, "#define SAC_DO_THREADS_STATIC  %d\n", (num_threads == 0) ? 0 : 1);

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
PrintSpmdData (node *syntax_tree)
{
    funptr *old_tab;

    DBUG_ENTER ("PrintSpmdData");

    old_tab = act_tab;
    act_tab = gsc_tab;

    fprintf (outfile, "#define SAC_SET_SPMD_FRAME    \\\n");
    fprintf (outfile, "  {    \\\n");

    Trav (MODUL_FUNS (syntax_tree), NULL);

    fprintf (outfile, "  }\n\n");

    act_tab = old_tab;

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
PrintGlobalSettings (node *syntax_tree)
{
    DBUG_ENTER ("PrintGlobalSettings");

    fprintf (outfile, "\n\n/*\n *  Global Settings\n */\n\n");

    fprintf (outfile, "#define SAC_SET_MAX_SYNC_FOLD     %d\n", max_sync_fold);
    fprintf (outfile, "#define SAC_SET_THREADS_MAX       %d\n", max_threads);
    fprintf (outfile, "#define SAC_SET_THREADS           %d\n", num_threads);
    fprintf (outfile, "#define SAC_SET_MASTERCLASS       %d\n",
             CalcMasterclass (num_threads));

    fprintf (outfile, "#define SAC_SET_CACHE_1_SIZE      %d\n", config.cache1_size);
    fprintf (outfile, "#define SAC_SET_CACHE_1_LINE      %d\n", config.cache1_line);
    fprintf (outfile, "#define SAC_SET_CACHE_1_ASSOC     %d\n", config.cache1_assoc);
    fprintf (outfile, "#define SAC_SET_CACHE_2_SIZE      %d\n", config.cache2_size);
    fprintf (outfile, "#define SAC_SET_CACHE_2_LINE      %d\n", config.cache2_line);
    fprintf (outfile, "#define SAC_SET_CACHE_2_ASSOC     %d\n", config.cache2_assoc);
    fprintf (outfile, "#define SAC_SET_CACHE_3_SIZE      %d\n", config.cache3_size);
    fprintf (outfile, "#define SAC_SET_CACHE_3_LINE      %d\n", config.cache3_line);
    fprintf (outfile, "#define SAC_SET_CACHE_3_ASSOC     %d\n", config.cache3_assoc);

    fprintf (outfile, "#define SAC_SET_MAXFUN            %d\n", PFfuncntr);
    fprintf (outfile, "#define SAC_SET_MAXFUNAP          %d\n", PFfunapmax);

    fprintf (outfile, "\n");

    PrintProfileData ();

    PrintSpmdData (syntax_tree);

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
GSCicm (node *arg_node, node *arg_info)
{
    node *icm_arg;
    char *tag, *type, *name;

    DBUG_ENTER ("GSCicm");

    if (0 == strcmp (ICM_NAME (arg_node), "MT_SPMD_BLOCK")) {

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

            if (0 == strncmp (tag, "inout", 5)) {
                tag = "inout";
            }

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
GSCspmd (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("GSCspmd");

    spmd_block_counter++;

    fprintf (outfile, "      SAC_MT_BLOCK_FRAME( %s,  {  \\\n", SPMD_FUNNAME (arg_node));

    if (SPMD_ICM (arg_node) != NULL) {
        Trav (SPMD_ICM (arg_node), arg_info);
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

    if (FUNDEF_STATUS (arg_node) == ST_regular) {
        fprintf (outfile, "    SAC_MT_FUN_FRAME( %s, {   \\\n", FUNDEF_NAME (arg_node));
        spmd_block_counter = 0;

        Trav (FUNDEF_BODY (arg_node), arg_info);

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

    fprintf (outfile, "  SAC_PF_SETUP();\n");
    fprintf (outfile, "  SAC_MT_SETUP();\n");

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
    fprintf (outfile, "SAC_PF_PRINT();\n");

    DBUG_VOID_RETURN;
}
