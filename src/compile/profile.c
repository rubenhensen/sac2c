/*
 *
 * $Log$
 * Revision 1.1  1997/05/28 12:41:06  sbs
 * Initial revision
 *
 *
 *
 */

#include "dbug.h"
#include "globals.h"
#include "profile.h"

int PFfuncntr = 1;
char *PFfunnme[PF_MAXFUN] = {"main"};
int PFfunapcntr[PF_MAXFUN];
int PFfunapline[PF_MAXFUN][PF_MAXFUNAP];

/*
 *
 *  functionname  : PFprintInitGlobals
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
PFprintInitGlobals ()
{
    int i, j;
    DBUG_ENTER ("PFprintInitGlobals");

    fprintf (outfile, "\n#ifdef PROFILE\n\n");
    fprintf (outfile, "int            __PF_maxfun;\n");
    fprintf (outfile, "__PF_TIMER    *__PF_act_timer;\n");
    fprintf (outfile, "int            __PF_act_funno;\n");
    fprintf (outfile, "int            __PF_act_funapno;\n");
    fprintf (outfile, "int            __PF_with_level=0;\n");
    fprintf (outfile, "struct rusage  __PF_rusage_start;\n");
    fprintf (outfile, "struct rusage  __PF_rusage_stop;\n");
    fprintf (outfile, "__PF_TIMER     __PF_fw_fun_timer[%d][%d];\n", PFfuncntr,
             PF_MAXFUNAP);
    fprintf (outfile, "__PF_TIMER     __PF_fw_with_genarray_timer[%d][%d];\n", PFfuncntr,
             PF_MAXFUNAP);
    fprintf (outfile, "__PF_TIMER     __PF_fw_with_modarray_timer[%d][%d];\n", PFfuncntr,
             PF_MAXFUNAP);
    fprintf (outfile, "__PF_TIMER     __PF_fw_with_fold_timer[%d][%d];\n\n", PFfuncntr,
             PF_MAXFUNAP);
    fprintf (outfile, "__PF_TIMER     __PF_fun_timer[%d][%d];\n", PFfuncntr, PF_MAXFUNAP);
    fprintf (outfile, "__PF_TIMER     __PF_with_genarray_timer[%d][%d];\n", PFfuncntr,
             PF_MAXFUNAP);
    fprintf (outfile, "__PF_TIMER     __PF_with_modarray_timer[%d][%d];\n", PFfuncntr,
             PF_MAXFUNAP);
    fprintf (outfile, "__PF_TIMER     __PF_with_fold_timer[%d][%d];\n\n", PFfuncntr,
             PF_MAXFUNAP);

    fprintf (outfile, "char       *__PF_fun_name[]={ \"%s\"", PFfunnme[0]);
    for (i = 1; i < PFfuncntr; i++) {
        fprintf (outfile, ",\n                              \"%s\"", PFfunnme[i]);
    };
    fprintf (outfile, " };\n");

    fprintf (outfile, "int         __PF_maxfunap[]={ %d", PFfunapcntr[0]);
    for (i = 1; i < PFfuncntr; i++) {
        fprintf (outfile, ",\n                              %d", PFfunapcntr[i]);
    }
    fprintf (outfile, " };\n");

    fprintf (outfile, "int         __PF_funapline[%d][%d]={ { %d", PFfuncntr, PF_MAXFUNAP,
             PFfunapline[0][0]);
    for (j = 1; j < PFfunapcntr[0]; j++) {
        fprintf (outfile, ", %d", PFfunapline[0][j]);
    }
    fprintf (outfile, " }");
    for (i = 1; i < PFfuncntr; i++) {
        fprintf (outfile, ",\n                                     { %d",
                 PFfunapline[i][0]);
        for (j = 1; j < PFfunapcntr[i]; j++) {
            fprintf (outfile, ", %d", PFfunapline[i][j]);
        }
        fprintf (outfile, " }");
    }
    fprintf (outfile, " };\n\n");

    fprintf (outfile, "#endif /* PROFILE */\n");
    DBUG_VOID_RETURN;
}
