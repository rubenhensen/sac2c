/*
 *
 * $Log$
 * Revision 1.3  1997/05/28 12:35:25  sbs
 * Profiling integrated
 *
 * Revision 1.2  1997/05/16  09:52:19  sbs
 * ANALSE-TOOL extended to function-application specific timing
 *
 * Revision 1.1  1997/05/14  08:26:39  sbs
 * Initial revision
 *
 *
 *
 */

#include "dbug.h"
#include "globals.h"
#include "analyse.h"

int ATfuncntr = 1;
char *ATfunnme[AT_MAXFUN] = {"main"};
int ATfunapcntr[AT_MAXFUN];
int ATfunapline[AT_MAXFUN][AT_MAXFUNAP];

/*
 *
 *  functionname  : ATprintInitGlobals
 *  arguments     : 1) argument node
 *                  2) info_node
 *  description   : manages traversing of children nodes of N_assign node
 *                  - if arg_info->node[0] is N_stop, traversing will be
 *                    stopped
 *
 *  global vars   : outfile
 *  internal funs :
 *  external funs : Trav
 *  macros        : DBUG...,
 *
 *  remarks       : --
 *                  arg_node->node[0] , if arg_node->node[1] != NULL
 *
 */

void
ATprintInitGlobals ()
{
    int i, j;
    DBUG_ENTER ("ATprintInitGlobals");

    fprintf (outfile, "\n#ifdef ANALYSE_TIME\n\n");
    fprintf (outfile, "int            __AT_maxfun;\n");
    fprintf (outfile, "struct timeval *__AT_act_timer;\n");
    fprintf (outfile, "int            __AT_act_funno;\n");
    fprintf (outfile, "int            __AT_act_funapno;\n");
    fprintf (outfile, "int            __AT_with_level=0;\n");
    fprintf (outfile, "struct rusage  __AT_rusage_start;\n");
    fprintf (outfile, "struct rusage  __AT_rusage_stop;\n");
    fprintf (outfile, "struct timeval __AT_fw_fun_timer[%d][%d];\n", ATfuncntr,
             AT_MAXFUNAP);
    fprintf (outfile, "struct timeval __AT_fw_with_genarray_timer[%d][%d];\n", ATfuncntr,
             AT_MAXFUNAP);
    fprintf (outfile, "struct timeval __AT_fw_with_modarray_timer[%d][%d];\n", ATfuncntr,
             AT_MAXFUNAP);
    fprintf (outfile, "struct timeval __AT_fw_with_fold_timer[%d][%d];\n\n", ATfuncntr,
             AT_MAXFUNAP);
    fprintf (outfile, "struct timeval __AT_fun_timer[%d][%d];\n", ATfuncntr, AT_MAXFUNAP);
    fprintf (outfile, "struct timeval __AT_with_genarray_timer[%d][%d];\n", ATfuncntr,
             AT_MAXFUNAP);
    fprintf (outfile, "struct timeval __AT_with_modarray_timer[%d][%d];\n", ATfuncntr,
             AT_MAXFUNAP);
    fprintf (outfile, "struct timeval __AT_with_fold_timer[%d][%d];\n\n", ATfuncntr,
             AT_MAXFUNAP);

    fprintf (outfile, "char       *__AT_fun_name[]={ \"%s\"", ATfunnme[0]);
    for (i = 1; i < ATfuncntr; i++) {
        fprintf (outfile, ",\n                              \"%s\"", ATfunnme[i]);
    };
    fprintf (outfile, " };\n");

    fprintf (outfile, "int         __AT_maxfunap[]={ %d", ATfunapcntr[0]);
    for (i = 1; i < ATfuncntr; i++) {
        fprintf (outfile, ",\n                              %d", ATfunapcntr[i]);
    }
    fprintf (outfile, " };\n");

    fprintf (outfile, "int         __AT_funapline[%d][%d]={ { %d", ATfuncntr, AT_MAXFUNAP,
             ATfunapline[0][0]);
    for (j = 1; j < ATfunapcntr[0]; j++) {
        fprintf (outfile, ", %d", ATfunapline[0][j]);
    }
    fprintf (outfile, " }");
    for (i = 1; i < ATfuncntr; i++) {
        fprintf (outfile, ",\n                                     { %d",
                 ATfunapline[i][0]);
        for (j = 1; j < ATfunapcntr[i]; j++) {
            fprintf (outfile, ", %d", ATfunapline[i][j]);
        }
        fprintf (outfile, " }");
    }
    fprintf (outfile, " };\n\n");

    fprintf (outfile, "#endif /* ANALYSE_TIME */\n");
    DBUG_VOID_RETURN;
}
