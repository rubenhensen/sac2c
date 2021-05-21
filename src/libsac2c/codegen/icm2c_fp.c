/*****************************************************************************
 *
 * file:   icm2c_fp.c
 *
 * prefix: ICMCompile
 *
 * description:
 *
 *   This file contains the definitions of C implemented ICMs.
 *
 *****************************************************************************/

#include <stdio.h>

#include "icm2c_basic.h"
#include "icm2c_utils.h"
#include "icm2c_std.h"
#include "icm2c_fp.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "convert.h"
#include "globals.h"
#include "print.h"
#include "tree_basic.h"
#include "str.h"
#include "memory.h"

#ifndef BEtest
#include "scnprs.h"   /* for big magic access to syntax tree      */
#include "traverse.h" /* for traversal of fold operation function */
#include "compile.h"  /* for GetFoldCode()                        */
#include "free.h"
#endif /* BEtest */

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SPMDFUN_DECL( char *name, unsigned int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SPMDFUN_DECL( name, vararg_cnt, [ TAG, type, param_NT ]* )
 *
 *   This ICM implements the declaration of an spmd function. The first parameter
 *   specifies the name of this function.
 *   TAG may be from the set { in, out, inout }.
 *
 ******************************************************************************/

void
ICMCompileFP_SLOWCLONE_DECL (char *name, char *rettype_NT, unsigned int vararg_cnt, char **vararg)
{
    DBUG_ENTER ();

#define FP_SLOWCLONE_DECL
#include "icm_comment.c"
#include "icm_trace.c"
#undef FP_SLOWCLONE_DECL

    INDENT;

    fprintf (global.outfile, "SAC_ND_DECL_FUN2(%s, void, SAC_fp_frame *_fp_frame)", name);

    DBUG_RETURN ();
}

void
ICMCompileFP_SLOWCLONE_DEF_BEGIN (char *name, char *rettype_NT, unsigned int vararg_cnt,
                                  char **vararg)
{
    DBUG_ENTER ();

#define FP_SLOWCLONE_DEF_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef FP_SLOWCLONE_DEF_BEGIN

    INDENT;

    fprintf (global.outfile, "SAC_ND_DEF_FUN_BEGIN2(%s, void, SAC_fp_frame *_fp_frame)\n",
             name);

    INDENT;
    fprintf (global.outfile, "{\n");
    global.indent++;

    // TODO: change hm thread status
    INDENT;
    fprintf (global.outfile, "SAC_HM_DEFINE_THREAD_STATUS( SAC_HM_single_threaded)\n");

    DBUG_RETURN ();
}

void
ICMCompileFP_FUN_RET (char *framename, char *retname, unsigned int vararg_cnt, char **vararg)
{
    DBUG_ENTER ();

#define FP_FUN_RET
#include "icm_comment.c"
#include "icm_trace.c"
#undef FP_FUN_RET

    INDENT;
    SCAN_ARG_LIST (vararg_cnt, 3, "\n", INDENT,
                   fprintf (global.outfile, "SAC_FP_SAVE_RESULT( %s, %zu, %s)", framename,
                            i / 3, vararg[i + 2]));

    if (vararg_cnt > 0) {
        fprintf (global.outfile, "\n");
        INDENT;
    }

    fprintf (global.outfile, "return;");

    DBUG_RETURN ();
}

void
ICMCompileFP_FUN_AP (char *framename, char *name, char *retname, unsigned int vararg_cnt,
                     char **vararg)
{
    char *tmp;

    DBUG_ENTER ();

#define FP_FUN_AP
#include "icm_comment.c"
#include "icm_trace.c"
#undef FP_FUN_AP

    ICMCompileND_FUN_AP (name, retname, vararg_cnt, vararg);

    tmp = TRAVtmpVar ();

    INDENT;
    fprintf (global.outfile, "SAC_FP_AP_CHECK_START(%s)\n", tmp);

    INDENT;
    SCAN_ARG_LIST (vararg_cnt, 3, "\n", INDENT,
                   fprintf (global.outfile, "SAC_FP_GET_RESULT( %s, %s, %zu, %s)", tmp,
                            framename, i / 3, vararg[i + 2]));

    if (vararg_cnt > 0) {
        fprintf (global.outfile, "\n");
        INDENT;
    }

    fprintf (global.outfile, "SAC_FP_AP_CHECK_END(%s)\n", tmp);

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
