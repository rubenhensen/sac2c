/** <!--*******************************************************************-->
 *
 * @file icm2c_rtspec.c
 *
 * @brief Contains the implementation of the ICMCompile functions for runtime
 * specialization.
 *
 * @author tvd
 *
 ****************************************************************************/

#include "icm2c_basic.h"
#include "icm2c_utils.h"
#include "icm2c_std.h"
#include "globals.h"
#include "print.h"
#include "str.h"

#include "dbug.h"

#define ScanArglist(cnt, inc, sep_str, sep_code, code)                                   \
    {                                                                                    \
        int i;                                                                           \
        for (i = 0; i < cnt * inc; i += inc) {                                           \
            if (i > 0) {                                                                 \
                fprintf (global.outfile, "%s", sep_str);                                 \
                sep_code;                                                                \
            }                                                                            \
            code;                                                                        \
        }                                                                                \
    }

/******************************************************************************
 *
 * function:
 *   void ICMCompileFP_FUN_AP( char *name, char *retname,
 *                             int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   FP_FUN_AP( name, retname, vararg_cnt, [ TAG, basetype, arg_NT ]* )
 *
 *   where TAG is element in { in, in_..., out, out_..., inout, inout_... }.
 *
 ******************************************************************************/

void
ICMCompileFP_FUN_AP (char *name, char *retname, int vararg_cnt, char **vararg)
{
    DBUG_ENTER ("ICMCompileFP_FUN_AP");

#define FP_FUN_AP
#include "icm_comment.c"
#include "icm_trace.c"
#undef FP_FUN_AP

    INDENT;
    if (!STReq (retname, "")) {
        fprintf (global.outfile, "%s = ", retname);
        fprintf (global.outfile, "%s(", name);
    } else {
        fprintf (global.outfile, "SAC_ND_FUNAP2(");
        fprintf (global.outfile, "%s, ", name);
    }

    ScanArglist (vararg_cnt, 3, ",", ,
                 fprintf (global.outfile, " SAC_ND_ARG_%s( %s, %s)", vararg[i],
                          vararg[i + 2], vararg[i + 1]));

    if (!STReq (retname, "")) {
        fprintf (global.outfile, ");");
    } else {
        fprintf (global.outfile, ")");
    }

    fprintf (global.outfile, "\n");

    DBUG_VOID_RETURN;
}
