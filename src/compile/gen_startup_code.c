/*
 *
 * $Log$
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

#include "dbug.h"
#include "globals.h"
#include "resource.h"

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
PrintGlobalSettings ()
{
    int i, j;

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

#if 0
static
void PrintProfileInit()
{
  int i,j;

  DBUG_ENTER("PrintProfileInit");

  fprintf(outfile, "\n\n/*\n *  Application Specific Profiling Runtime System\n */\n\n");
  
  fprintf(outfile, "\n#if PROFILE\n\n");

  fprintf(outfile,
	  "__PF_TIMER  __PF_fw_fun_timer[__PROFILE_MAXFUN][__PROFILE_MAXFUNAP];\n");

  fprintf(outfile,
	  "__PF_TIMER  __PF_fw_with_genarray_timer[__PROFILE_MAXFUN][__PROFILE_MAXFUNAP];\n");

  fprintf(outfile,
	  "__PF_TIMER  __PF_fw_with_modarray_timer[__PROFILE_MAXFUN][__PROFILE_MAXFUNAP];\n");

  fprintf(outfile,
	  "__PF_TIMER  __PF_fw_with_fold_timer[__PROFILE_MAXFUN][__PROFILE_MAXFUNAP];\n");
  
  fprintf(outfile,
	  "__PF_TIMER  __PF_fun_timer[__PROFILE_MAXFUN][__PROFILE_MAXFUNAP];\n");

  fprintf(outfile,
	  "__PF_TIMER  __PF_with_genarray_timer[__PROFILE_MAXFUN][__PROFILE_MAXFUNAP];\n");

  fprintf(outfile,
	  "__PF_TIMER  __PF_with_modarray_timer[__PROFILE_MAXFUN][__PROFILE_MAXFUNAP];\n");

  fprintf(outfile,
	  "__PF_TIMER  __PF_with_fold_timer[__PROFILE_MAXFUN][__PROFILE_MAXFUNAP];\n");
  

  fprintf(outfile, "\n");
  
  fprintf(outfile, "__PF_TIMER    *__PF_act_timer=&__PF_fun_timer[0][0];\n");
  fprintf(outfile, "int            __PF_act_funno=0;\n");
  fprintf(outfile, "int            __PF_act_funapno=0;\n");
  fprintf(outfile, "int            __PF_with_level=0;\n");
  fprintf(outfile, "struct rusage  __PF_start_timer;\n");
  fprintf(outfile, "struct rusage  __PF_stop_timer;\n");

  fprintf(outfile, "\n");

  fprintf(outfile, "char  *__PF_fun_name[]={ \"%s\"", PFfunnme[0]);
  for( i=1; i<PFfuncntr; i++) {
    fprintf(outfile, ",\n                         \"%s\"", PFfunnme[i]);
  };
  fprintf(outfile, " };\n");


  fprintf(outfile, "int    __PF_maxfunap[]={ %d", PFfunapcntr[0]);
  for( i=1; i<PFfuncntr; i++) {
    fprintf(outfile, ",\n                         %d", PFfunapcntr[i]);
  }
  fprintf(outfile, " };\n");


  fprintf(outfile, "int    __PF_funapline[__PROFILE_MAXFUN][__PROFILE_MAXFUNAP]\n"
	  "         = { { %d",
	  PFfunapline[0][0]);
  for( j=1; j<PFfunapcntr[0]; j++) {
    fprintf(outfile,", %d", PFfunapline[0][j]);
  }
  fprintf(outfile, " }");
  for( i=1; i<PFfuncntr; i++) {
    fprintf(outfile, ",\n             { %d",
                PFfunapline[i][0]);
    for( j=1; j<PFfunapcntr[i]; j++) {
      fprintf(outfile,", %d", PFfunapline[i][j]);
    }
    fprintf(outfile, " }");
  }
  fprintf(outfile, " };\n\n");

  fprintf(outfile, "#endif /* PROFILE */\n");

  DBUG_VOID_RETURN;
}
#endif

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
GSCPrintFileHeader ()
{
    DBUG_ENTER ("GSCPrintDeclarations");

    PrintGlobalSwitches ();
    PrintGlobalSettings ();
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
