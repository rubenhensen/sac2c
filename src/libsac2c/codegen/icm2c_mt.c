/*****************************************************************************
 *
 * $Id$
 *
 * file:   icm2c_mt.c
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
#include "icm2c_mt.h"

#include "dbug.h"
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
 *   void ICMCompileMT_SPMDFUN_DECL( char *name, int vararg_cnt, char **vararg)
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
ICMCompileMT_SPMDFUN_DECL (char *funname, int vararg_cnt, char **vararg)
{
    DBUG_ENTER ("ICMCompileMT_SPMDFUN_DECL");

#define MT_SPMDFUN_DECL
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMDFUN_DECL

    INDENT;
    fprintf (global.outfile,
             "SAC_MT_SPMDFUN_REAL_RETTYPE()"
             " %s( SAC_MT_SPMDFUN_REAL_PARAM_LIST());\n",
             funname);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SPMDFUN_DEF_BEGIN( char *name, int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SPMDFUN_DEF_BEGIN( name, vararg_cnt, [ TAG, type, param_NT ]* )
 *
 *   This ICM implements the protoype of an spmd function. The first parameter
 *   specifies the name of this function.
 *   TAG may be from the set { in, out, inout }.
 *
 ******************************************************************************/

void
ICMCompileMT_SPMDFUN_DEF_BEGIN (char *funname, int vararg_cnt, char **vararg)
{
    int i;
    int cnt;

    DBUG_ENTER ("ICMCompileMT_SPMDFUN_DEF_BEGIN");

#define MT_SPMDFUN_DEF_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMDFUN_DEF_BEGIN

    INDENT;
    fprintf (global.outfile,
             "SAC_MT_SPMDFUN_REAL_RETTYPE()"
             " %s( SAC_MT_SPMDFUN_REAL_PARAM_LIST())\n",
             funname);

    INDENT;
    fprintf (global.outfile, "{\n");

    global.indent++;

    INDENT;
    fprintf (global.outfile, "SAC_HM_DEFINE_THREAD_STATUS( SAC_HM_multi_threaded)\n");

    cnt = 0;
    for (i = 0; i < 3 * vararg_cnt; i += 3) {
        INDENT;
        fprintf (global.outfile, "SAC_MT_RECEIVE_PARAM_%s( %s, %d, %s, %s)\n", vararg[i],
                 funname, cnt++, vararg[i + 1], vararg[i + 2]);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SPMDFUN_DEF_END( char *name, int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SPMDFUN_DEF_END( name, vararg_cnt, [ TAG, type, param_NT ]* )
 *
 *   This ICM implements end of an spmd function. The first parameter
 *   specifies the name of this function.
 *   TAG may be from the set { in, out, inout }.
 *
 ******************************************************************************/

void
ICMCompileMT_SPMDFUN_DEF_END (char *funname, int vararg_cnt, char **vararg)
{
    DBUG_ENTER ("ICMCompileMT_SPMDFUN_DEF_END");

#define MT_SPMDFUN_DEF_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMDFUN_DEF_END

    global.indent--;
    INDENT;
    fprintf (global.outfile, "}\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SPMDFUN_AP( char *name, int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SPMDFUN_AP( name, vararg_cnt, [ TAG, basetype, param_NT ]* )
 *
 *   This ICM implements the application of an spmd function. The first
 *   parameter specifies the name of this function.
 *   TAG may be from the set { in, out, inout }.
 *
 ******************************************************************************/

void
ICMCompileMT_SPMDFUN_AP (char *funname, int vararg_cnt, char **vararg)
{
    int i;
    int cnt;

    DBUG_ENTER ("ICMCompileMT_SPMDFUN_AP");

#define MT_SPMDFUN_AP
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMDFUN_AP

    cnt = 0;
    for (i = 0; i < 3 * vararg_cnt; i += 3) {
        INDENT;
        fprintf (global.outfile, "SAC_MT_SEND_PARAM_%s( %s, %d, %s)\n", vararg[i],
                 funname, cnt++, vararg[i + 2]);
    }

    INDENT;
    fprintf (global.outfile, "\nSAC_MT_SPMD_EXECUTE( %s);\n", funname);

    cnt = 0;
    for (i = 0; i < 3 * vararg_cnt; i += 3) {
        INDENT;
        fprintf (global.outfile, "SAC_MT_RECEIVE_RESULT_%s( %s, 0, %d, %s)\n", vararg[i],
                 funname, cnt++, vararg[i + 2]);
    }
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SPMDFUN_RET( char *funname,
 *                                  int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SPMDFUN_RET( funname, vararg_cnt, [ tag, accu_NT, val_NT, basetype, tag, foldfun
 *]* )
 *
 *   This ICM implements the return statement of an spmd-function,
 *   i.e. it has to realize the return of several out parameters.
 *
 *   <tag> tags the foldfun as either following MT or ND calling convention.
 *
 ******************************************************************************/

void
ICMCompileMT_SPMDFUN_RET (char *funname, int vararg_cnt, char **vararg)
{
    int i, cnt;

    DBUG_ENTER ("ICMCompileMT_SPMDFUN_RET");

#define MT_SPMDFUN_RET
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMDFUN_RET

    INDENT;
    fprintf (global.outfile, "SAC_MT_SYNC_BEGIN( %s);\n", funname);

    global.indent++;
    cnt = 0;
    for (i = 0; i < 6 * vararg_cnt; i += 6) {
        INDENT;
        fprintf (global.outfile, "SAC_MT_SYNC_FOLD_%s( %s, %d, %s, %s, %s, %s, %s);\n",
                 vararg[i], funname, cnt, vararg[i + 1], vararg[i + 2], vararg[i + 3],
                 vararg[i + 4], vararg[i + 5]);
        cnt += 1;
    }
    global.indent--;

    INDENT;
    fprintf (global.outfile, "SAC_MT_SYNC_CONT( %s);\n", funname);

    global.indent++;
    cnt = 0;
    for (i = 0; i < 5 * vararg_cnt; i += 5) {
        INDENT;
        fprintf (global.outfile,
                 "SAC_MT_SEND_RESULT_%s( %s, SAC_MT_MYTHREAD(), %d, %s);\n", vararg[i],
                 funname, cnt, vararg[i + 1]);
        cnt += 1;
    }
    global.indent--;

    INDENT;
    fprintf (global.outfile, "SAC_MT_SYNC_END( %s);\n", funname);

    INDENT;
    fprintf (global.outfile, "SAC_MT_SPMDFUN_REAL_RETURN();\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_MTFUN_DECL( char *funname, char *rettype_NT,
 *                                 int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_MTFUN_DECL( name, rettype_NT, vararg_cnt, [ TAG, type, param_NT ]* )
 *
 *   This ICM implements the declaration of an spmd function. The first parameter
 *   specifies the name of this function.
 *   TAG may be from the set { in, out, inout }.
 *
 ******************************************************************************/

void
ICMCompileMT_MTFUN_DECL (char *funname, char *rettype_NT, int vararg_cnt, char **vararg)
{
    DBUG_ENTER ("ICMCompileMT_MTFUN_DECL");

#define MT_MTFUN_DECL
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_MTFUN_DECL

    INDENT;
    if (rettype_NT[0] != '\0') {
        fprintf (global.outfile, "SAC_ND_TYPE_NT( %s) ", rettype_NT);
    } else {
        fprintf (global.outfile, "void ");
    }

    fprintf (global.outfile, "%s(", funname);
    fprintf (global.outfile, " SAC_MT_MYTHREAD_PARAM()");

    if (vararg_cnt > 0) {
        fprintf (global.outfile, ", ");
    }

    ScanArglist (vararg_cnt, 3, ",", ,
                 fprintf (global.outfile, " SAC_ND_PARAM_%s( %s, %s)", vararg[i],
                          vararg[i + 2], vararg[i + 1]));
    fprintf (global.outfile, ")");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_MTFUN_DEF_BEGIN( char *funname, char *rettype_NT,
 *                                      int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_MTFUN_DEF_BEGIN( name, rettype_NT, vararg_cnt, [ TAG, type, param_NT ]* )
 *
 *   This ICM implements the protoype of an spmd function. The first parameter
 *   specifies the name of this function.
 *   TAG may be from the set { in, out, inout }.
 *
 ******************************************************************************/

void
ICMCompileMT_MTFUN_DEF_BEGIN (char *funname, char *rettype_NT, int vararg_cnt,
                              char **vararg)
{
    DBUG_ENTER ("ICMCompileMT_MTFUN_DEF_BEGIN");

#define MT_MTFUN_DEF_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_MTFUN_DEF_BEGIN

    INDENT;
    if (rettype_NT[0] != '\0') {
        fprintf (global.outfile, "SAC_ND_TYPE_NT( %s) ", rettype_NT);
    } else {
        fprintf (global.outfile, "void ");
    }

    fprintf (global.outfile, "%s( SAC_MT_MYTHREAD_PARAM()", funname);

    if (vararg_cnt > 0) {
        fprintf (global.outfile, ", ");
    }

    ScanArglist (vararg_cnt, 3, ",", ,
                 fprintf (global.outfile, " SAC_ND_PARAM_%s( %s, %s)", vararg[i],
                          vararg[i + 2], vararg[i + 1]));
    fprintf (global.outfile, ")\n");

    INDENT;
    fprintf (global.outfile, "{");
    global.indent++;

    INDENT;
    fprintf (global.outfile, "SAC_HM_DEFINE_THREAD_STATUS( SAC_HM_multi_threaded)\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_MTFUN_DEF_END( char *name, char *rettype_NT,
 *                                    int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_MTFUN_DEF_END( name, rettype_NT, vararg_cnt, [ TAG, type, param_NT ]* )
 *
 *   This ICM implements end of an spmd function. The first parameter
 *   specifies the name of this function.
 *   TAG may be from the set { in, out, inout }.
 *
 ******************************************************************************/

void
ICMCompileMT_MTFUN_DEF_END (char *funname, char *rettype_NT, int vararg_cnt,
                            char **vararg)
{
    DBUG_ENTER ("ICMCompileMT_MTFUN_DEF_END");

#define MT_MTFUN_DEF_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_MTFUN_DEF_END

    global.indent--;
    INDENT;
    fprintf (global.outfile, "}\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_MTFUN_AP( char *name, char *retname_NT,
 *                               int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_MTFUN_AP( name, retname_NT, vararg_cnt, [ TAG, param_NT ]* )
 *
 *   This ICM implements the application of an spmd function. The first
 *   parameter specifies the name of this function.
 *   TAG may be from the set { in, out, inout }.
 *
 ******************************************************************************/

void
ICMCompileMT_MTFUN_AP (char *funname, char *retname_NT, int vararg_cnt, char **vararg)
{
    DBUG_ENTER ("ICMCompileMT_MTFUN_AP");

#define MT_MTFUN_AP
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_MTFUN_AP

    INDENT;
    if (retname_NT[0] != '\0') {
        fprintf (global.outfile, "%s = ", retname_NT);
    }

    fprintf (global.outfile, "%s( SAC_MT_MYTHREAD()", funname);

    if (vararg_cnt > 0) {
        fprintf (global.outfile, ", ");
    }

    ScanArglist (vararg_cnt, 3, ",", ,
                 fprintf (global.outfile, " SAC_ND_ARG_%s( %s, %s)", vararg[i],
                          vararg[i + 2], vararg[i + 1]));
    fprintf (global.outfile, ");\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_MTFUN_RET( char *retname_NT, int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_MTFUN_RET( retname_NT, vararg_cnt, [ TAG, arg_NT, decl_arg_NT ]* )
 *
 *   where TAG is element in { out, inout }.
 *
 ******************************************************************************/

void
ICMCompileMT_MTFUN_RET (char *retname_NT, int vararg_cnt, char **vararg)
{
    DBUG_ENTER ("ICMCompileMT_MTFUN_RET");

#define MT_MTFUN_RET
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_MTFUN_RET

    INDENT;
    ScanArglist (vararg_cnt, 3, "\n", INDENT,
                 fprintf (global.outfile, "SAC_ND_RET_%s( %s, %s)", vararg[i],
                          vararg[i + 1], vararg[i + 2]));
    if (vararg_cnt > 0) {
        fprintf (global.outfile, "\n");
        INDENT;
    }

    if (retname_NT[0] != '\0') {
        fprintf (global.outfile, "return( %s);", retname_NT);
    } else {
        fprintf (global.outfile, "return;");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SPMD_FRAME_ELEMENT( char *name, int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SPMD_FRAME_ELEMENT( name, vararg_cnt, [ TAG, type, param_NT ]* )
 *
 *   This ICM implements the protoype of an spmd function. The first parameter
 *   specifies the name of this function.
 *   TAG may be from the set { in, out, inout }.
 *
 ******************************************************************************/

void
ICMCompileMT_SPMD_FRAME_ELEMENT (char *funname, int vararg_cnt, char **vararg)
{
    int i;
    int cnt;

    DBUG_ENTER ("ICMCompileMT_SPMD_FRAME_ELEMENT");

#define MT_SPMD_FRAME_ELEMENT
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMD_FRAME_ELEMENT

    INDENT;
    fprintf (global.outfile, "SAC_MT_SPMD_FRAME_ELEMENT_BEGIN( %s)\n", funname);

    cnt = 0;
    for (i = 0; i < 3 * vararg_cnt; i += 3) {
        INDENT;
        fprintf (global.outfile, "SAC_MT_FRAME_ELEMENT_%s( %s, %d, %s, %s)\n", vararg[i],
                 funname, cnt++, vararg[i + 1], vararg[i + 2]);
    }

    INDENT;
    fprintf (global.outfile, "SAC_MT_SPMD_FRAME_ELEMENT_END( %s)\n", funname);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SPMD_BARRIER_ELEMENT( char *name, int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SPMD_BARRIER_ELEMENT( name, vararg_cnt, [ TAG, type, param_NT ]* )
 *
 *   This ICM implements the protoype of an spmd function. The first parameter
 *   specifies the name of this function.
 *   TAG may be from the set { in, out, inout }.
 *
 ******************************************************************************/

void
ICMCompileMT_SPMD_BARRIER_ELEMENT (char *funname, int vararg_cnt, char **vararg)
{
    int i;
    int cnt;

    DBUG_ENTER ("ICMCompileMT_SPMD_BARRIER_ELEMENT");

#define MT_SPMD_BARRIER_ELEMENT
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMD_BARRIER_ELEMENT

    INDENT;
    fprintf (global.outfile, "SAC_MT_SPMD_BARRIER_ELEMENT_BEGIN( %s)\n", funname);

    cnt = 0;
    for (i = 0; i < 3 * vararg_cnt; i += 3) {
        INDENT;
        fprintf (global.outfile, "SAC_MT_BARRIER_ELEMENT_%s( %s, %d, %s, %s)\n",
                 vararg[i], funname, cnt++, vararg[i + 1], vararg[i + 2]);
    }

    INDENT;
    fprintf (global.outfile, "SAC_MT_SPMD_BARRIER_ELEMENT_END( %s)\n", funname);

    DBUG_VOID_RETURN;
}
