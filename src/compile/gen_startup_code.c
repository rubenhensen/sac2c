/*
 *
 * $Log$
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

    fprintf (outfile, "#define MALLOCCHECK   %d\n", check_malloc ? 1 : 0);
    fprintf (outfile, "#define BOUNDCHECK    %d\n", check_boundary ? 1 : 0);
    fprintf (outfile, "#define PROFILE       %d\n", profileflag ? 1 : 0);
    fprintf (outfile, "#define PROFILE_WITH  %d\n", (profileflag & PROFILE_WITH) ? 1 : 0);
    fprintf (outfile, "#define PROFILE_FUN   %d\n", (profileflag & PROFILE_FUN) ? 1 : 0);
    fprintf (outfile, "#define PROFILE_INL   %d\n", (profileflag & PROFILE_INL) ? 1 : 0);
    fprintf (outfile, "#define PROFILE_LIB   %d\n", (profileflag & PROFILE_LIB) ? 1 : 0);
    fprintf (outfile, "#define TRACE         %d\n", traceflag ? 1 : 0);
    fprintf (outfile, "#define TRACE_REF     %d\n", (traceflag & TRACE_REF) ? 1 : 0);
    fprintf (outfile, "#define TRACE_MEM     %d\n", (traceflag & TRACE_MEM) ? 1 : 0);
    fprintf (outfile, "#define TRACE_PRF     %d\n", (traceflag & TRACE_PRF) ? 1 : 0);
    fprintf (outfile, "#define TRACE_UDF     %d\n", (traceflag & TRACE_UDF) ? 1 : 0);
    fprintf (outfile, "#define TRACE_WST     %d\n", (traceflag & TRACE_WST) ? 1 : 0);
    fprintf (outfile, "#define MODULE        %d\n", (filetype == F_prog) ? 0 : 1);
    fprintf (outfile, "#define MULTITHREAD   %d\n", (num_threads == 1) ? 0 : 1);

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
    DBUG_ENTER ("PrintGlobalSettings");

    fprintf (outfile, "\n\n/*\n *  Global Settings\n */\n\n");

    fprintf (outfile, "#define __PROFILE_MAXFUN    %d\n", PFfuncntr);
    fprintf (outfile, "#define __PROFILE_MAXFUNAP  %d\n", PFfunapmax);

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
PrintProfileInit ()
{
    int i, j;

    DBUG_ENTER ("PrintProfileInit");

    fprintf (outfile,
             "\n\n/*\n *  Application Specific Profiling Runtime System\n */\n\n");

    fprintf (outfile, "\n#if PROFILE\n\n");

    fprintf (outfile,
             "__PF_TIMER  __PF_fw_fun_timer[__PROFILE_MAXFUN][__PROFILE_MAXFUNAP];\n");

    fprintf (outfile, "__PF_TIMER  "
                      "__PF_fw_with_genarray_timer[__PROFILE_MAXFUN][__PROFILE_MAXFUNAP];"
                      "\n");

    fprintf (outfile, "__PF_TIMER  "
                      "__PF_fw_with_modarray_timer[__PROFILE_MAXFUN][__PROFILE_MAXFUNAP];"
                      "\n");

    fprintf (outfile, "__PF_TIMER  "
                      "__PF_fw_with_fold_timer[__PROFILE_MAXFUN][__PROFILE_MAXFUNAP];\n");

    fprintf (outfile,
             "__PF_TIMER  __PF_fun_timer[__PROFILE_MAXFUN][__PROFILE_MAXFUNAP];\n");

    fprintf (outfile, "__PF_TIMER  "
                      "__PF_with_genarray_timer[__PROFILE_MAXFUN][__PROFILE_MAXFUNAP];"
                      "\n");

    fprintf (outfile, "__PF_TIMER  "
                      "__PF_with_modarray_timer[__PROFILE_MAXFUN][__PROFILE_MAXFUNAP];"
                      "\n");

    fprintf (outfile,
             "__PF_TIMER  __PF_with_fold_timer[__PROFILE_MAXFUN][__PROFILE_MAXFUNAP];\n");

    fprintf (outfile, "\n");

    fprintf (outfile, "__PF_TIMER    *__PF_act_timer=&__PF_fun_timer[0][0];\n");
    fprintf (outfile, "int            __PF_act_funno=0;\n");
    fprintf (outfile, "int            __PF_act_funapno=0;\n");
    fprintf (outfile, "int            __PF_with_level=0;\n");
    fprintf (outfile, "struct rusage  __PF_start_timer;\n");
    fprintf (outfile, "struct rusage  __PF_stop_timer;\n");

    fprintf (outfile, "\n");

    fprintf (outfile, "char  *__PF_fun_name[]={ \"%s\"", PFfunnme[0]);
    for (i = 1; i < PFfuncntr; i++) {
        fprintf (outfile, ",\n                         \"%s\"", PFfunnme[i]);
    };
    fprintf (outfile, " };\n");

    fprintf (outfile, "int    __PF_maxfunap[]={ %d", PFfunapcntr[0]);
    for (i = 1; i < PFfuncntr; i++) {
        fprintf (outfile, ",\n                         %d", PFfunapcntr[i]);
    }
    fprintf (outfile, " };\n");

    fprintf (outfile,
             "int    __PF_funapline[__PROFILE_MAXFUN][__PROFILE_MAXFUNAP]\n"
             "         = { { %d",
             PFfunapline[0][0]);
    for (j = 1; j < PFfunapcntr[0]; j++) {
        fprintf (outfile, ", %d", PFfunapline[0][j]);
    }
    fprintf (outfile, " }");
    for (i = 1; i < PFfuncntr; i++) {
        fprintf (outfile, ",\n             { %d", PFfunapline[i][0]);
        for (j = 1; j < PFfunapcntr[i]; j++) {
            fprintf (outfile, ", %d", PFfunapline[i][j]);
        }
        fprintf (outfile, " }");
    }
    fprintf (outfile, " };\n\n");

    fprintf (outfile, "#endif /* PROFILE */\n");

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
GSCPrintFileHeader ()
{
    DBUG_ENTER ("GSCPrintDeclarations");

    PrintGlobalSwitches ();
    PrintGlobalSettings ();
    PrintIncludes ();
    PrintProfileInit ();

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

    fprintf (outfile, "  __PROFILE_SETUP();\n");

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
    fprintf (outfile, "__PROFILE_PRINT();\n");

    DBUG_VOID_RETURN;
}
