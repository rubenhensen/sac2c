/*
 *
 * $Log$
 * Revision 1.6  1998/06/05 19:41:43  dkr
 * added some INDENTs
 *
 * Revision 1.5  1998/06/04 17:03:29  cg
 * switched to new variable names.
 *
 * Revision 1.4  1998/05/12 09:03:42  cg
 * The new macro SAC_MT_CURRENT_FUN() is now set before each function definition.
 *
 * Revision 1.3  1998/05/07 10:13:53  dkr
 * some routines are moved to icm2c_basic.h
 *
 * Revision 1.2  1998/05/07 08:10:02  cg
 * C implemented ICMs converted to new naming conventions.
 *
 * Revision 1.1  1998/04/25 16:19:58  sbs
 * Initial revision
 *
 *
 */

#include <malloc.h>
#include <stdio.h>
#include <ctype.h>

#include "icm2c_basic.h"

#include "dbug.h"
#include "my_debug.h"
#include "main.h"
#include "convert.h"
#include "globals.h"
#include "print.h"
#include "gen_startup_code.h"

#define RetWithScal(res, val)                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "SAC_ND_A_FIELD(%s)[%s__destptr++]=%s;\n", res, res, val[0]);

#define RetWithArray(res, a)                                                             \
    INDENT;                                                                              \
    fprintf (outfile, "{ int __i;\n\n");                                                 \
    indent++;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "for(SAC_i=0; SAC_i<SAC_ND_A_SIZE(%s); SAC_i++)\n", a);            \
    indent++;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "SAC_ND_A_FIELD(%s)[%s__destptr++]=SAC_ND_A_FIELD(%s)[SAC_i];\n",  \
             res, res, a);                                                               \
    indent--;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "SAC_ND_DEC_RC_FREE_ARRAY( %s, 1);\n", a);                         \
    indent--;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "}\n")

#define BeginWith(res, dimres, from, to, idx, idxlen, fillstr, withkind)                 \
    INDENT;                                                                              \
    fprintf (outfile, "{\n");                                                            \
    indent++;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "int %s__destptr=0;\n", res);                                      \
                                                                                         \
    {                                                                                    \
        int i, j;                                                                        \
        for (i = 0; i < idxlen; i++) {                                                   \
            INDENT;                                                                      \
            fprintf (outfile, "int SAC_lim_%d;\n", i);                                   \
            INDENT;                                                                      \
            fprintf (outfile, "int %s__offset%d_left=", res, i);                         \
            fprintf (outfile, "SAC_ND_A_FIELD(%s)[%d]", from, i);                        \
            for (j = (i + 1); j < dimres; j++)                                           \
                fprintf (outfile, " *SAC_ND_KD_A_SHAPE(%s, %d)", res, j);                \
            fprintf (outfile, ";\n");                                                    \
            INDENT;                                                                      \
            fprintf (outfile, "int %s__offset%d_right=", res, i);                        \
            fprintf (outfile, "(SAC_ND_KD_A_SHAPE(%s, %d)-SAC_ND_A_FIELD(%s)[%d]-1)",    \
                     res, i, to, i);                                                     \
            for (j = (i + 1); j < dimres; j++)                                           \
                fprintf (outfile, " *SAC_ND_KD_A_SHAPE(%s, %d)", res, j);                \
            fprintf (outfile, ";\n");                                                    \
        }                                                                                \
        fprintf (outfile, "\n");                                                         \
        INDENT;                                                                          \
        fprintf (outfile, "PROFILE_BEGIN_WITH( " withkind " );\n");                      \
        {                                                                                \
            int i;                                                                       \
            for (i = 0; i < idxlen; i++) {                                               \
                INDENT;                                                                  \
                fprintf (outfile, "SAC_lim_%d=%s__destptr+%s__offset%d_left;\n", i, res, \
                         res, i);                                                        \
                INDENT;                                                                  \
                fprintf (outfile, "for(; %s__destptr<SAC_lim_%d; %s__destptr++) {\n",    \
                         res, i, res);                                                   \
                indent++;                                                                \
                INDENT;                                                                  \
                fprintf (outfile, "SAC_ND_A_FIELD(%s)[%s__destptr]=", res, res);         \
                fillstr;                                                                 \
                fprintf (outfile, ";\n");                                                \
                indent--;                                                                \
                INDENT;                                                                  \
                fprintf (outfile, "}\n");                                                \
                                                                                         \
                INDENT;                                                                  \
                fprintf (outfile,                                                        \
                         "for( SAC_ND_A_FIELD(%s)[%d]=SAC_ND_A_FIELD(%s)[%d]; "          \
                         "SAC_ND_A_FIELD(%s)[%d]<=SAC_ND_A_FIELD(%s)[%d]; "              \
                         "SAC_ND_A_FIELD(%s)[%d]++) {\n",                                \
                         idx, i, from, i, idx, i, to, i, idx, i);                        \
                indent++;                                                                \
            }                                                                            \
        }                                                                                \
        fprintf (outfile, "\n");                                                         \
    }

#define BeginFoldWith(res, sizeres, form, to, idx, idixlen, n_neutral, neutral)          \
    INDENT;                                                                              \
    fprintf (outfile, "{ PROFILE_BEGIN_WITH( fold );\n");                                \
    indent++;                                                                            \
    INDENT;                                                                              \
    if (0 < sizeres) {                                                                   \
        indent++;                                                                        \
        INDENT;                                                                          \
        {                                                                                \
            int i, j;                                                                    \
            if (sizeres == n_neutral)                                                    \
                for (i = 0; i < n_neutral; i += 1)                                       \
                    if (1 == isdigit ((neutral[i])[0]))                                  \
                        fprintf (outfile, "SAC_ND_A_FIELD(%s)[%d]=%s;\n", res, i,        \
                                 neutral[i]);                                            \
                    else                                                                 \
                        fprintf (outfile,                                                \
                                 "SAC_ND_A_FIELD(%s)[%d]=SAC_ND_A_FIELD(%s);\n", res, i, \
                                 neutral[i]);                                            \
            else                                                                         \
                for (i = 0, j = 0; i < n_neutral; i += 1, j = i / n_neutral)             \
                    fprintf (outfile,                                                    \
                             " SAC_ND_A_FIELD(%s)[%d]=SAC_ND_A_FIELD(%s)[%d];\n", res,   \
                             i, neutral[j], i - n_neutral * j);                          \
        }                                                                                \
    } else                                                                               \
        fprintf (outfile, " %s=%s;\n", res, neutral[0]);                                 \
    {                                                                                    \
        int i;                                                                           \
        for (i = 0; i < idxlen; i++) {                                                   \
            INDENT;                                                                      \
            fprintf (outfile,                                                            \
                     "for( SAC_ND_A_FIELD(%s)[%d]=SAC_ND_A_FIELD(%s)[%d]; "              \
                     "SAC_ND_A_FIELD(%s)[%d]<=SAC_ND_A_FIELD(%s)[%d]; "                  \
                     "SAC_ND_A_FIELD(%s)[%d]++) {\n",                                    \
                     idx, i, from, i, idx, i, to, i, idx, i);                            \
            indent++;                                                                    \
        }                                                                                \
    }                                                                                    \
    fprintf (outfile, "\n")

#define EndWith(res, dimres, idxlen, fillstr, withkind)                                  \
    {                                                                                    \
        int i;                                                                           \
        for (i = idxlen - 1; i >= 0; i--) {                                              \
            indent--;                                                                    \
            INDENT;                                                                      \
            fprintf (outfile, "}\n");                                                    \
            INDENT;                                                                      \
            fprintf (outfile, "SAC_lim_%d=%s__destptr+%s__offset%d_right;\n", i, res,    \
                     res, i);                                                            \
            INDENT;                                                                      \
            fprintf (outfile, "for(; %s__destptr<SAC_lim_%d; %s__destptr++) {\n", res,   \
                     i, res);                                                            \
            indent++;                                                                    \
            INDENT;                                                                      \
            fprintf (outfile, "SAC_ND_A_FIELD(%s)[%s__destptr]=", res, res);             \
            fillstr;                                                                     \
            fprintf (outfile, ";\n");                                                    \
            indent--;                                                                    \
            INDENT;                                                                      \
            fprintf (outfile, "}\n");                                                    \
        }                                                                                \
    }                                                                                    \
    INDENT;                                                                              \
    fprintf (outfile, "PROFILE_END_WITH( " withkind " );\n");                            \
    indent--;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "}\n\n")

#define EndFoldWith(idxlen)                                                              \
    {                                                                                    \
        int i;                                                                           \
        for (i = 0; i < idxlen; i++) {                                                   \
            indent--;                                                                    \
            INDENT;                                                                      \
            fprintf (outfile, "}\n");                                                    \
        }                                                                                \
    }                                                                                    \
    INDENT;                                                                              \
    fprintf (outfile, "PROFILE_END_WITH( fold );\n");                                    \
    indent--;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "}\n\n")

#define ScanArglist(arg, n, bin, bout, binout, bupd, bupdbox, binrc, boutrc, binoutrc,   \
                    sepstr)                                                              \
    {                                                                                    \
        int i = 0;                                                                       \
        int sep = 0;                                                                     \
        while (i < n) {                                                                  \
            if (sep)                                                                     \
                fprintf (outfile, "%s", sepstr);                                         \
            DBUG_PRINT ("PRINT", ("arg-index: %d, arg-tag : %s", i, arg[i]));            \
            if (strcmp (arg[i], "in") == 0) {                                            \
                i++;                                                                     \
                bin;                                                                     \
            } else if (strcmp (arg[i], "out") == 0) {                                    \
                i++;                                                                     \
                bout;                                                                    \
            } else if (strcmp (arg[i], "inout") == 0) {                                  \
                i++;                                                                     \
                binout;                                                                  \
            } else if (strcmp (arg[i], "upd") == 0) {                                    \
                i++;                                                                     \
                bupd;                                                                    \
            } else if (strcmp (arg[i], "upd_bx") == 0) {                                 \
                i++;                                                                     \
                bupdbox;                                                                 \
            } else if (strcmp (arg[i], "in_rc") == 0) {                                  \
                i++;                                                                     \
                binrc;                                                                   \
            } else if (strcmp (arg[i], "out_rc") == 0) {                                 \
                i++;                                                                     \
                boutrc;                                                                  \
            } else {                                                                     \
                i++;                                                                     \
                binoutrc;                                                                \
            }                                                                            \
        }                                                                                \
    }

#define NewBlock(init, body)                                                             \
    fprintf (outfile, "{\n");                                                            \
    indent++;                                                                            \
    init;                                                                                \
    body;                                                                                \
    indent--;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "}\n")

#define InitPtr(src, dest)                                                               \
    INDENT;                                                                              \
    fprintf (outfile, "int SAC_isrc=");                                                  \
    src;                                                                                 \
    fprintf (outfile, ";\n");                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "int SAC_idest=");                                                 \
    dest;                                                                                \
    fprintf (outfile, ";\n\n")

#define InitVecs(from, to, vn, v_i_str)                                                  \
    {                                                                                    \
        int i;                                                                           \
        for (i = from; i < to; i++) {                                                    \
            INDENT;                                                                      \
            fprintf (outfile, "int %s%d=", vn, i);                                       \
            v_i_str;                                                                     \
            fprintf (outfile, ";\n");                                                    \
        }                                                                                \
    }

#define InitIMaxs(from, to, v_i_str) InitVecs (from, to, "SAC_imax", v_i_str)

#define InitSrcOffs(from, to, v_i_str) InitVecs (from, to, "SAC_srcoff", v_i_str)

#define FillRes(res, body)                                                               \
    INDENT;                                                                              \
    fprintf (outfile, "do {\n");                                                         \
    indent++;                                                                            \
    body;                                                                                \
    indent--;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "}\n");                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "while( SAC_idest<SAC_ND_A_SIZE(%s));\n", res)

#define AccessSeg(dim, body)                                                             \
    {                                                                                    \
        int i;                                                                           \
        INDENT;                                                                          \
        fprintf (outfile, "{\n");                                                        \
        indent++;                                                                        \
        for (i = 1; i < dim; i++) {                                                      \
            INDENT;                                                                      \
            fprintf (outfile, "int SAC_i%d;\n", i);                                      \
        }                                                                                \
        for (i = 1; i < dim; i++) {                                                      \
            INDENT;                                                                      \
            fprintf (outfile, "for( SAC_i%d=0; SAC_i%d<SAC_imax%d; SAC_i%d++) {\n", i,   \
                     i, i, i);                                                           \
            indent++;                                                                    \
        }                                                                                \
        body;                                                                            \
        for (i = dim - 1; i > 0; i--) {                                                  \
            indent--;                                                                    \
            INDENT;                                                                      \
            fprintf (outfile, "}\n");                                                    \
            INDENT;                                                                      \
            fprintf (outfile, "SAC_isrc += SAC_srcoff%d;\n", i);                         \
        }                                                                                \
        indent--;                                                                        \
        INDENT;                                                                          \
        fprintf (outfile, "}\n");                                                        \
    }

#define CopyBlock(a, offset, res, line)                                                  \
    NewBlock (InitPtr (offset, fprintf (outfile, "0")),                                  \
              FillRes (                                                                  \
                res, INDENT; if (check_boundary) {                                       \
                    fprintf (outfile,                                                    \
                             "if((0 <= SAC_isrc)&&( SAC_isrc < SAC_ND_A_SIZE(%s)))\n",   \
                             a);                                                         \
                    indent++;                                                            \
                    INDENT;                                                              \
                } fprintf (outfile,                                                      \
                           "SAC_ND_A_FIELD(%s)[SAC_idest++]=SAC_ND_A_FIELD(%s)[SAC_"     \
                           "isrc++];\n",                                                 \
                           res, a);                                                      \
                if (check_boundary) {                                                    \
                    indent--;                                                            \
                    INDENT;                                                              \
                    fprintf (outfile, "else\n");                                         \
                    indent++;                                                            \
                    INDENT;                                                              \
                    fprintf (outfile,                                                    \
                             "SAC_OUT_OF_BOUND(%d, \"psi\", SAC_ND_A_SIZE(%s), "         \
                             "SAC_isrc);\n",                                             \
                             line, a);                                                   \
                    indent--;                                                            \
                    INDENT;                                                              \
                }))

/*
 * TakeSeg(a, dima, offset, dimi, sz_i_str, off_i_str, res)
 *   a        : src-array
 *   dima     : dimension of "a"
 *   offset   : beginning of the segment in the unrolling of "a"
 *   dimi     : length of "sz_i_str" and "off_i_str"
 *   sz_i_str : number of elements to take from the i'th axis
 *   off_i_str: number of elements to skip in the i'th axis
 *   res      : resulting array
 *
 */

#define TakeSeg(a, dima, offset, dimi, sz_i_str, off_i_str, res)                         \
    NewBlock (InitPtr (offset, fprintf (outfile, "0")); InitIMaxs (0, dimi, sz_i_str);   \
              InitIMaxs (dimi, dima,                                                     \
                         fprintf (outfile, "SAC_ND_KD_A_SHAPE(%s, %d)", a, i));          \
              InitSrcOffs (                                                              \
                0, dimi, off_i_str; {                                                    \
                    int j;                                                               \
                    for (j = i + 1; j < dima; j++)                                       \
                        fprintf (outfile, "*SAC_ND_KD_A_SHAPE(%s, %d)", a, j);           \
                }) InitSrcOffs (dimi, dima, fprintf (outfile, "0")),                     \
              FillRes (res, AccessSeg (dima, INDENT;                                     \
                                       fprintf (outfile,                                 \
                                                "SAC_ND_A_FIELD(%s)[SAC_idest++]=SAC_"   \
                                                "ND_A_FIELD(%s)[SAC_isrc++];\n",         \
                                                res, a);)))

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_FUN_DEC( char *name, char *rettype, int narg, char **tyarg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_FUN_DEC( name, rettype, narg, [ TAG, type, arg ]* )
 *
 *   where TAG is element in {in, in_rc, out, out_rc, inout, inout_rc}.
 *
 ******************************************************************************/

void
ICMCompileND_FUN_DEC (char *name, char *rettype, int narg, char **tyarg)
{
    DBUG_ENTER ("ICMCompileND_FUN_DEC");

#define ND_FUN_DEC
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_FUN_DEC

    fprintf (outfile, "#undef SAC_MT_CURRENT_FUN()\n");
    fprintf (outfile, "#define SAC_MT_CURRENT_FUN() %s\n", name);

    INDENT;
    fprintf (outfile, "%s ", rettype);
    if (strcmp (name, "main") == 0) {
        fprintf (outfile, "%s( int __argc, char *__argv[])", name);
    } else if (strcmp (name, "create_TheCommandLine") == 0) {
        fprintf (outfile, "%s( int __argc, char **__argv)", name);
    } else {
        fprintf (outfile, "%s( ", name);
        ScanArglist (tyarg, 3 * narg, fprintf (outfile, " %s %s", tyarg[i], tyarg[i + 1]);
                     i += 2; sep = 1,

                             if (0 != (tyarg[i + 1])[0]) {
                                 fprintf (outfile, " %s *%s__p", tyarg[i], tyarg[i + 1]);
                                 i += 2;
                                 sep = 1;
                             } else {
                                 fprintf (outfile, " %s *%s", tyarg[i], tyarg[i + 1]);
                                 i += 2;
                                 sep = 1;
                             },

                             if (0 != (tyarg[i + 1])[0]) {
                                 fprintf (outfile, " %s *%s__p", tyarg[i], tyarg[i + 1]);
                                 i += 2;
                                 sep = 1;
                             } else {
                                 fprintf (outfile, " %s *%s", tyarg[i], tyarg[i + 1]);
                                 i += 2;
                                 sep = 1;
                             },

                             fprintf (outfile, " %s *%s", tyarg[i], tyarg[i + 1]);
                     i += 2; sep = 1,

                             fprintf (outfile, " %s %s", tyarg[i], tyarg[i + 1]);
                     i += 2; sep = 1,

                             if (0 != (tyarg[i + 1])[0])
                               fprintf (outfile, " SAC_ND_KS_DEC_IN_RC(%s, %s)", tyarg[i],
                                        tyarg[i + 1]);
                     else fprintf (outfile, "SAC_ND_KS_DEC_IMPORT_IN_RC(%s)", tyarg[i]);
                     i += 2; sep = 1,

                             if (0 != (tyarg[i + 1])[0])
                               fprintf (outfile, " SAC_ND_KS_DEC_OUT_RC(%s, %s)",
                                        tyarg[i], tyarg[i + 1]);
                     else fprintf (outfile, "SAC_ND_KS_DEC_IMPORT_OUT_RC(%s)", tyarg[i]);
                     i += 2; sep = 1,

                             if (0 != (tyarg[i + 1])[0])
                               fprintf (outfile, " SAC_ND_KS_DEC_INOUT_RC(%s, %s)",
                                        tyarg[i], tyarg[i + 1]);
                     else fprintf (outfile, "SAC_ND_KS_DEC_IMPORT_INOUT_RC(%s)",
                                   tyarg[i]);
                     i += 2; sep = 1,

                             ",");
        fprintf (outfile, ")");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_FUN_AP( char *name, char *retname, int narg, char **arg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_FUN_DEC( name, retname, narg, [ TAG, arg ]* )
 *
 *   where TAG is element in {in, in_rc, out, out_rc, inout, inout_rc}.
 *
 ******************************************************************************/

void
ICMCompileND_FUN_AP (char *name, char *retname, int narg, char **arg)
{
    DBUG_ENTER ("ICMCompileND_FUN_AP");

#define ND_FUN_AP
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_FUN_AP

    INDENT;
    if (0 != strcmp (retname, ""))
        fprintf (outfile, "%s =", retname);
    if (strcmp (name, "create_TheCommandLine") == 0) {
        fprintf (outfile, "%s( __argc, __argv);", name);
    } else {
        fprintf (outfile, "%s( ", name);
        ScanArglist (arg, 2 * narg, fprintf (outfile, " %s", arg[i]); i++;
                     sep = 1, fprintf (outfile, " &%s", arg[i]); i++;
                     sep = 1, fprintf (outfile, " &%s", arg[i]); i++;
                     sep = 1, fprintf (outfile, " &%s", arg[i]); i++;
                     sep = 1, fprintf (outfile, " %s", arg[i]); i++;
                     sep = 1, fprintf (outfile, " SAC_ND_KS_AP_IN_RC(%s)", arg[i]); i++;
                     sep = 1, fprintf (outfile, " SAC_ND_KS_AP_OUT_RC(%s)", arg[i]); i++;
                     sep = 1, fprintf (outfile, " SAC_ND_KS_AP_INOUT_RC(%s)", arg[i]);
                     i++; sep = 1, ",");
        fprintf (outfile, ");");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_FUN_RET( char *retname, int narg, char **arg, node *arg_info)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_FUN_RET( retname, narg, [ TAG, arg ]* )
 *
 *   where TAG is element in { out, out_rc, inout, inout_rc}.
 *
 *   ATTENTION (!!!) this function gets an extra parameter "arg_info"
 *   in case of a usage from within PrintND_FUN_RET this argument contains
 *     a pointer to the fundef-node from which it was originally created;
 *   in case of a usage from BEtest this argument will be NULL !!
 *
 ******************************************************************************/

void
ICMCompileND_FUN_RET (char *retname, int narg, char **arg, node *arg_info)
{
    DBUG_ENTER ("ICMCompileND_FUN_RET");

#define ND_FUN_RET
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_FUN_RET

    INDENT;
    ScanArglist (arg, 2 * narg, i++;
                 sep = 0, fprintf (outfile, "*%s__p = %s;\n", arg[i], arg[i]); i++;
                 INDENT; sep = 1, fprintf (outfile, "*%s__p = %s;\n", arg[i], arg[i]);
                 i++; INDENT; sep = 1, i++; sep = 0, i++; sep = 0, i++;
                 sep = 0, fprintf (outfile, "SAC_ND_KS_RET_OUT_RC(%s);\n", arg[i]); i++;
                 INDENT;
                 sep = 1, fprintf (outfile, "SAC_ND_KS_RET_INOUT_RC(%s);\n", arg[i]); i++;
                 INDENT; sep = 1, "");

    if (arg_info != NULL) {
        if (strcmp (FUNDEF_NAME (INFO_FUNDEF (arg_info)), "main") == 0) {
            GSCPrintMainEnd ();
            INDENT;
        }
    }
    if (0 != strcmp (retname, ""))
        fprintf (outfile, "return(%s);", retname);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_CREATE_CONST_ARRAY_S( char *name, int dim, char **s)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_CREATE_CONST_ARRAY_S( name, dim, s0,..., sn)
 *
 ******************************************************************************/

void
ICMCompileND_CREATE_CONST_ARRAY_S (char *name, int dim, char **s)
{
    DBUG_ENTER ("ICMCompileND_CREATE_CONST_ARRAY_S");

#define ND_CREATE_CONST_ARRAY_S
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_CREATE_CONST_ARRAY_S

    {
        int i;
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile, "SAC_ND_A_FIELD(%s)[%d]=%s;\n", name, i, s[i]);
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_CREATE_CONST_ARRAY_H( char *name, char * copyfun, int dim, char
 ***A)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_CREATE_CONST_ARRAY_H( name, copyfun, dim, s0,..., sn)
 *   generates a constant array of refcounted hidden values
 *
 ******************************************************************************/

void
ICMCompileND_CREATE_CONST_ARRAY_H (char *name, char *copyfun, int dim, char **A)
{
    DBUG_ENTER ("ICMCompileND_CREATE_CONST_ARRAY_H");

#define ND_CREATE_CONST_ARRAY_H
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_CREATE_CONST_ARRAY_H

    {
        int i;
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile, "SAC_ND_NO_RC_MAKE_UNIQUE_HIDDEN(%s, %s[%d], %s);\n", A[i],
                     name, i, copyfun);
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_CREATE_CONST_ARRAY_A( char *name, int length, int dim, char **A)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_CREATE_CONST_ARRAY_A( name, length, dim, A0,..., An)
 *   generates a constant array out of arrays
 *   where length is the number of elements of the argument array A
 *
 ******************************************************************************/

void
ICMCompileND_CREATE_CONST_ARRAY_A (char *name, int length, int dim, char **A)
{
    DBUG_ENTER ("ICMCompileND_CREATE_CONST_ARRAY_A");

#define ND_CREATE_CONST_ARRAY_A
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_CREATE_CONST_ARRAY_A

    {
        int i;
        for (i = 0; i < dim * length; i++) {
            INDENT;
            fprintf (outfile, "%s[%d]=%s[%d];\n", name, i, A[i / length], i % length);
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KS_DECL_ARRAY( char *type, char *name, int dim, char **s)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KS_DECL_ARRAY( basetype, name, dim, s0,..., sn)
 *
 ******************************************************************************/

void
ICMCompileND_KS_DECL_ARRAY (char *type, char *name, int dim, char **s)
{
    DBUG_ENTER ("ICMCompileND_KS_DECL_ARRAY");

#define ND_KS_DECL_ARRAY
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KS_DECL_ARRAY

    INDENT;
    fprintf (outfile, "%s *%s;\n", type, name);
    INDENT;
    fprintf (outfile, "int SAC_ND_A_RC(%s);\n", name);
    INDENT;
    fprintf (outfile, "int SAC_ND_A_SIZE(%s)=", name);
    fprintf (outfile, "%s", s[0]);
    {
        int i;
        for (i = 1; i < dim; i++)
            fprintf (outfile, "*%s", s[i]);
        fprintf (outfile, ";\n");
        INDENT;
        fprintf (outfile, "int SAC_ND_A_DIM(%s)=%d;\n", name, dim);
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile, "int SAC_ND_KD_A_SHAPE(%s, %d)=%s;\n", name, i, s[i]);
        }
    }
    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KS_DECL_GLOBAL_ARRAY( char *type, char *name, int dim, char **s)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KS_DECL_GLOBAL_ARRAY( basetype, name, dim, s0,..., sn)
 *
 ******************************************************************************/

void
ICMCompileND_KS_DECL_GLOBAL_ARRAY (char *type, char *name, int dim, char **s)
{
    DBUG_ENTER ("ICMCompileND_KS_DECL_GLOBAL_ARRAY");

#define ND_KS_DECL_GLOBAL_ARRAY
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KS_DECL_GLOBAL_ARRAY

    if (print_objdef_for_header_file) {
        INDENT;
        fprintf (outfile, "extern %s *%s;\n", type, name);
        INDENT;
        fprintf (outfile, "extern int SAC_ND_A_RC(%s);\n", name);
        INDENT;
        fprintf (outfile, "extern int SAC_ND_A_SIZE(%s);\n", name);
        {
            int i;
            INDENT;
            fprintf (outfile, "extern int SAC_ND_A_DIM(%s);\n", name);
            for (i = 0; i < dim; i++) {
                INDENT;
                fprintf (outfile, "extern int SAC_ND_KD_A_SHAPE(%s, %d);\n", name, i);
            }
        }
        fprintf (outfile, "\n");
    } else {
        INDENT;
        fprintf (outfile, "%s *%s;\n", type, name);
        INDENT;
        fprintf (outfile, "int SAC_ND_A_RC(%s);\n", name);
        INDENT;
        fprintf (outfile, "int SAC_ND_A_SIZE(%s)=", name);
        fprintf (outfile, "%s", s[0]);
        {
            int i;
            for (i = 1; i < dim; i++)
                fprintf (outfile, "*%s", s[i]);
            fprintf (outfile, ";\n");
            INDENT;
            fprintf (outfile, "int SAC_ND_A_DIM(%s)=%d;\n", name, dim);
            for (i = 0; i < dim; i++) {
                INDENT;
                fprintf (outfile, "int SAC_ND_KD_A_SHAPE(%s, %d)=%s;\n", name, i, s[i]);
            }
        }
        fprintf (outfile, "\n");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KD_DECL_EXTERN_ARRAY( char *type, char *name, int dim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KD_DECL_EXTERN_ARRAY( basetype, name, dim)
 *   declares an array which is an imported global object
 *
 ******************************************************************************/

void
ICMCompileND_KD_DECL_EXTERN_ARRAY (char *type, char *name, int dim)
{
    DBUG_ENTER ("ICMCompileND_KD_DECL_EXTERN_ARRAY");

#define ND_KD_DECL_EXTERN_ARRAY
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KD_DECL_EXTERN_ARRAY

    INDENT;
    fprintf (outfile, "extern %s *%s;\n", type, name);
    INDENT;
    fprintf (outfile, "extern int SAC_ND_A_RC(%s);\n", name);
    INDENT;
    fprintf (outfile, "extern int SAC_ND_A_SIZE(%s);\n", name);
    {
        int i;
        INDENT;
        fprintf (outfile, "extern int SAC_ND_A_DIM(%s);\n", name);
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile, "extern int SAC_ND_KD_A_SHAPE(%s, %d);\n", name, i);
        }
    }
    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KS_DECL_ARRAY_ARG( char *name, int dim, char **s)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KS_DECL_ARRAY_ARG( name, dim, s0,..., sn)
 *   declares an array given as arg
 *
 ******************************************************************************/

void
ICMCompileND_KS_DECL_ARRAY_ARG (char *name, int dim, char **s)
{
    DBUG_ENTER ("ICMCompileND_KS_DECL_ARRAY_ARG");

#define ND_KS_DECL_ARRAY_ARG
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KS_DECL_ARRAY_ARG

    INDENT;
    fprintf (outfile, "int SAC_ND_A_SIZE(%s)=", name);
    fprintf (outfile, "%s", s[0]);
    {
        int i;
        for (i = 1; i < dim; i++)
            fprintf (outfile, "*%s", s[i]);
        fprintf (outfile, ";\n");
        INDENT;
        fprintf (outfile, "int SAC_ND_A_DIM(%s)=%d;\n", name, dim);
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile, "int SAC_ND_KD_A_SHAPE(%s, %d)=%s;\n", name, i, s[i]);
        }
    }
    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KD_SET_SHAPE( char *name, int dim, char **s)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KD_SET_SHAPE( name, dim, s0,..., sn)
 *   sets all shape components of an array
 *
 ******************************************************************************/

void
ICMCompileND_KD_SET_SHAPE (char *name, int dim, char **s)
{
    DBUG_ENTER ("ICMCompileND_KD_SET_SHAPE");

#define ND_KD_SET_SHAPE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KD_SET_SHAPE

    {
        int i;
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile, "SAC_ND_KD_A_SHAPE(%s, %d)=%s;\n", name, i, s[i]);
        }
    }
    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KD_PSI_CxA_S( int line, char *a, char *res, int dim, char **vi)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KD_PSI_CxA_S( line, a, res, dim, v0,..., vn)
 *   selects a single element of the array
 *
 ******************************************************************************/

void
ICMCompileND_KD_PSI_CxA_S (int line, char *a, char *res, int dim, char **vi)
{
    DBUG_ENTER ("ICMCompileND_KD_PSI_CxA_S");

#define ND_KD_PSI_CxA_S
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KD_PSI_CxA_S

    INDENT;
    if (check_boundary) {
        fprintf (outfile, "{ int SAC_idx=");
        VectToOffset (dim, AccessConst (vi, i), dim, a);
        fprintf (outfile, ";\n");
        INDENT;
        fprintf (outfile, "if((0 <= SAC_idx) && (SAC_idx < SAC_ND_A_SIZE(%s)) )\n", a);
        indent++;
        INDENT;
        fprintf (outfile, "%s=SAC_ND_A_FIELD(%s)[SAC_idx];\n", res, a);
        indent--;
        INDENT;
        fprintf (outfile, "else\n");
        indent++;
        INDENT;
        fprintf (outfile, "SAC_OUT_OF_BOUND(%d, \"psi\", SAC_ND_A_SIZE(%s), SAC_idx);\n",
                 line, a);
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
    } else {
        fprintf (outfile, "%s=SAC_ND_A_FIELD(%s)[", res, a);
        VectToOffset (dim, AccessConst (vi, i), dim, a);
        fprintf (outfile, "];\n\n");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KD_PSI_VxA_S( int line, char *a, char *res, int dim, char *v)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KD_PSI_VxA_S(line, a, res, dim, v )
 *   selects a single element of the array
 *
 ******************************************************************************/

void
ICMCompileND_KD_PSI_VxA_S (int line, char *a, char *res, int dim, char *v)
{
    DBUG_ENTER ("ICMCompileND_KD_PSI_VxA_S");

#define ND_KD_PSI_VxA_S
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KD_PSI_VxA_S

    INDENT;
    if (check_boundary) {
        fprintf (outfile, "{ int SAC_idx=");
        VectToOffset (dim, AccessVect (v, i), dim, a);
        fprintf (outfile, ";\n");
        INDENT;
        fprintf (outfile, "if ((0 <= SAC_idx) && (SAC_idx < SAC_ND_A_SIZE(%s)) )\n", a);
        indent++;
        INDENT;
        fprintf (outfile, "%s=SAC_ND_A_FIELD(%s)[SAC_idx];\n", res, a);
        indent--;
        INDENT;
        fprintf (outfile, "else\n");
        indent++;
        INDENT;
        fprintf (outfile, "SAC_OUT_OF_BOUND(%d, \"psi\", SAC_ND_A_SIZE(%s), SAC_idx);\n",
                 line, a);
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
    } else {
        fprintf (outfile, "%s=SAC_ND_A_FIELD(%s)[", res, a);
        VectToOffset (dim, AccessVect (v, i), dim, a);
        fprintf (outfile, "];\n\n");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KD_PSI_CxA_A( int line, int dima, char *a,
 *                                   char *res, int dimv, char **vi)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KD_PSI_CxA_A(line, dima, a, res, dimv, v0,..., vn)
 *   selects a sub-array
 *
 ******************************************************************************/

void
ICMCompileND_KD_PSI_CxA_A (int line, int dima, char *a, char *res, int dimv, char **vi)
{
    DBUG_ENTER ("ICMCompileND_KD_PSI_CxA_A");

#define ND_KD_PSI_CxA_A
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KD_PSI_CxA_A

    INDENT;
    CopyBlock (a, VectToOffset (dimv, AccessConst (vi, i), dima, a), res, line);
    fprintf (outfile, "\n\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KD_PSI_VxA_A( int line, int dima, char *a,
 *                                   char *res, int dimv, char *v)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KD_PSI_VxA_A(line, dima, a, res, dimv, v )
 *   selects a sub-array
 *
 ******************************************************************************/

void
ICMCompileND_KD_PSI_VxA_A (int line, int dima, char *a, char *res, int dimv, char *v)
{
    DBUG_ENTER ("ICMCompileND_KD_PSI_VxA_A");

#define ND_KD_PSI_VxA_A
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KD_PSI_VxA_A

    INDENT;
    CopyBlock (a, VectToOffset (dimv, AccessVect (v, i), dima, a), res, line);
    fprintf (outfile, "\n\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KD_TAKE_CxA_A( int dima, char *a, char *res, int dimv, char **vi)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KD_TAKE_CxA_A( dima, a, res, dimv, v0,..., vn)
 *
 ******************************************************************************/

void
ICMCompileND_KD_TAKE_CxA_A (int dima, char *a, char *res, int dimv, char **vi)
{
    DBUG_ENTER ("ICMCompileND_KD_TAKE_CxA_A");

#define ND_KD_TAKE_CxA_A
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KD_TAKE_CxA_A

    INDENT;
    TakeSeg (a, dima, fprintf (outfile, "0"), /* offset */
             dimv,                            /* dim of sizes & offsets */
             AccessConst (vi, i),             /* sizes */
             fprintf (outfile, "(SAC_ND_KD_A_SHAPE(%s, %d) - ", a, i);
             AccessConst (vi, i); fprintf (outfile, ")"), /* offsets */
                                  res);

    fprintf (outfile, "\n\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KD_DROP_CxA_A( int dima, char *a, char *res, int dimv, char **vi)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KD_DROP_CxA_A( dima, a, res, dimv, v0,..., vn)
 *
 ******************************************************************************/

void
ICMCompileND_KD_DROP_CxA_A (int dima, char *a, char *res, int dimv, char **vi)
{
    DBUG_ENTER ("ICMCompileND_KD_DROP_CxA_A");

#define ND_KD_DROP_CxA_A
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KD_DROP_CxA_A

    INDENT;
    TakeSeg (a, dima, VectToOffset (dimv, AccessConst (vi, i), dima, a), /* offset */
             dimv, /* dim of sizes & offsets */
             fprintf (outfile, "SAC_ND_KD_A_SHAPE(%s, %d) - ", a, i);
             AccessConst (vi, i), /* sizes */
             AccessConst (vi, i), /* offsets */
             res);

    fprintf (outfile, "\n\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KD_CAT_SxAxA_A( int dima, char **ar, char *res, int catdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KD_CAT_SxAxA_A( dima, ar0, ar1, res, catdim)
 *
 ******************************************************************************/

void
ICMCompileND_KD_CAT_SxAxA_A (int dima, char **ar, char *res, int catdim)
{
    DBUG_ENTER ("ICMCompileND_KD_CAT_SxAxA_A");

#define ND_KD_CAT_SxAxA_A
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KD_CAT_SxAxA_A

    INDENT;
    NewBlock (InitPtr (fprintf (outfile, "0"), fprintf (outfile, "0"));
              InitVecs (1, 2, "SAC_isrc", fprintf (outfile, "0"));
              InitVecs (0, 2, "SAC_i", fprintf (outfile, "0"));
              InitVecs (0, 2, "SAC_bl",
                        {
                            int j;
                            for (j = catdim; j < dima; j++)
                                fprintf (outfile, "SAC_ND_KD_A_SHAPE(%s, %d)*", ar[i], j);
                            fprintf (outfile, "1");
                        }),
              FillRes (res, INDENT;
                       fprintf (outfile, "for(SAC_i0=0; SAC_i0<SAC_bl0; SAC_i0++)\n");
                       indent++; INDENT; fprintf (outfile,
                                                  "SAC_ND_A_FIELD(%s)[SAC_idest++] = "
                                                  "SAC_ND_A_FIELD(%s)[SAC_isrc++];\n",
                                                  res, ar[0]);
                       indent--; INDENT;
                       fprintf (outfile, "for(SAC_i1=0; SAC_i1<SAC_bl1; SAC_i1++)\n");
                       indent++; INDENT; fprintf (outfile,
                                                  "SAC_ND_A_FIELD(%s)[SAC_idest++] = "
                                                  "SAC_ND_A_FIELD(%s)[SAC_isrc1++];\n",
                                                  res, ar[1]);
                       indent--; INDENT));
    fprintf (outfile, "\n\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KD_ROT_CxSxA_A( int rotdim, char **numstr, int dima,
 *                                     char *a, char *res)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KD_ROT_CxSxA_A( rotdim, numstr, dima, a, res)
 *
 ******************************************************************************/

void
ICMCompileND_KD_ROT_CxSxA_A (int rotdim, char **numstr, int dima, char *a, char *res)
{
    DBUG_ENTER ("ICMCompileND_KD_ROT_CxSxA_A");

#define ND_KD_ROT_CxSxA_A
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KD_ROT_CxSxA_A

    INDENT;
    NewBlock (InitVecs (0, 1, "SAC_shift",
                        {
                            int j;
                            for (j = rotdim + 1; j < dima; j++)
                                fprintf (outfile, "SAC_ND_KD_A_SHAPE(%s, %d)*", a, j);
                            fprintf (outfile, "(%s<0 ? ", numstr[0]);
                            fprintf (outfile,
                                     "SAC_ND_KD_A_SHAPE(%s, %d)+ (%s %% "
                                     "SAC_ND_KD_A_SHAPE(%s, %d)) : ",
                                     a, rotdim, numstr[0], a, rotdim);
                            fprintf (outfile, "%s %% SAC_ND_KD_A_SHAPE(%s, %d))",
                                     numstr[0], a, rotdim);
                        });
              InitVecs (0, 1, "SAC_bl",
                        {
                            int j;
                            for (j = rotdim + 1; j < dima; j++)
                                fprintf (outfile, "SAC_ND_KD_A_SHAPE(%s, %d)*", a, j);
                            fprintf (outfile, "SAC_ND_KD_A_SHAPE(%s, %d)", a, rotdim);
                        });
              InitVecs (0, 1, "SAC_i", fprintf (outfile, "0"));
              InitPtr (fprintf (outfile, "-SAC_shift0"), fprintf (outfile, "0")),
              FillRes (res, INDENT; fprintf (outfile, "SAC_isrc+=SAC_bl0;\n"); INDENT;
                       fprintf (outfile, "for(SAC_i0=0; SAC_i0<SAC_shift0; SAC_i0++)\n");
                       indent++; INDENT; fprintf (outfile,
                                                  "SAC_ND_A_FIELD(%s)[SAC_idest++] = "
                                                  "SAC_ND_A_FIELD(%s)[SAC_isrc++];\n",
                                                  res, a);
                       indent--; INDENT; fprintf (outfile, "SAC_isrc-=SAC_bl0;\n");
                       INDENT; fprintf (outfile, "for(; SAC_i0<SAC_bl0; SAC_i0++)\n");
                       indent++; INDENT; fprintf (outfile,
                                                  "SAC_ND_A_FIELD(%s)[SAC_idest++] = "
                                                  "SAC_ND_A_FIELD(%s)[SAC_isrc++];\n",
                                                  res, a);
                       indent--));
    fprintf (outfile, "\n\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_MODARRAY_AxCxS_CHECK_REUSE( int line, char *res_type,
 *                                                     int dimres, char *res,
 *                                                     char *old, char **value,
 *                                                     int dimv, char **vi)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_MODARRAY_AxCxS_CHECK_REUSE( line, res_type, dimres, res, old, value,
 *                                      dimv, v0,..., vn)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_MODARRAY_AxCxS_CHECK_REUSE (int line, char *res_type, int dimres,
                                             char *res, char *old, char **value, int dimv,
                                             char **vi)
{
    DBUG_ENTER ("ICMCompileND_PRF_MODARRAY_AxCxS_CHECK_REUSE");

#define ND_PRF_MODARRAY_AxCxS_CHECK_REUSE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_MODARRAY_AxCxS_CHECK_REUSE

    fprintf (outfile, " SAC_ND_CHECK_REUSE_ARRAY(%s,%s)\n", old, res);
    INDENT;
    fprintf (outfile, "{ int SAC_i;\n");
    fprintf (outfile, "  SAC_ND_ALLOC_ARRAY(%s, %s, 0);\n", res_type, res);
    INDENT;
    fprintf (outfile, "  for(SAC_i=0; SAC_i<SAC_ND_A_SIZE(%s); SAC_i++)", res);
    fprintf (outfile, " SAC_ND_A_FIELD(%s)[SAC_i]=SAC_ND_A_FIELD(%s)[SAC_i];\n", res,
             old);
    INDENT;
    fprintf (outfile, "}\n");
    INDENT;
    if (check_boundary) {
        fprintf (outfile, "{  int SAC_idx=");
        VectToOffset (dimv, AccessConst (vi, i), dimres, res);
        fprintf (outfile, ";\n");
        indent++;
        INDENT;
        fprintf (outfile, "if( (SAC_ND_A_SIZE(%s) > SAC_idx) && (0 <= SAC_idx))\n", res);
        indent++;
        INDENT;
    }
    fprintf (outfile, "SAC_ND_A_FIELD(%s)[", res);
    VectToOffset (dimv, AccessConst (vi, i), dimres, res);
    fprintf (outfile, "]=%s;\n", value[0]);
    INDENT;
    if (check_boundary) {
        fprintf (outfile, "else\n");
        indent++;
        INDENT;
        fprintf (outfile,
                 "SAC_OUT_OF_BOUND(%d, \"modarray\", SAC_ND_A_SIZE(%s), SAC_idx);\n",
                 line, res);
        indent -= 2;
        INDENT;
        fprintf (outfile, "}\n");
    }
    fprintf (outfile, "\n\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_MODARRAY_AxCxS( int line, char *res_type, int dimres,
 *                                         char *res, char *old, char **value,
 *                                         int dimv, char **vi)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_MODARRAY_AxCxS( line, res_type, dimres, res, old, value,
 *                                      dimv, v0,..., vn)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_MODARRAY_AxCxS (int line, char *res_type, int dimres, char *res,
                                 char *old, char **value, int dimv, char **vi)
{
    DBUG_ENTER ("ICMCompileND_PRF_MODARRAY_AxCxS");

#define ND_PRF_MODARRAY_AxCxS
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_MODARRAY_AxCxS

    fprintf (outfile, "{ int SAC_i;\n");
    fprintf (outfile, "  SAC_ND_ALLOC_ARRAY(%s, %s, 0);\n", res_type, res);
    INDENT;
    fprintf (outfile, "  for(SAC_i=0; SAC_i<SAC_ND_A_SIZE(%s); SAC_i++)", res);
    fprintf (outfile, " SAC_ND_A_FIELD(%s)[SAC_i]=SAC_ND_A_FIELD(%s)[SAC_i];\n", res,
             old);
    INDENT;
    fprintf (outfile, "}\n");
    INDENT;
    if (check_boundary) {
        fprintf (outfile, "{  int SAC_idx=");
        VectToOffset (dimv, AccessConst (vi, i), dimres, res);
        fprintf (outfile, ";\n");
        indent++;
        INDENT;
        fprintf (outfile, "if( (SAC_ND_A_SIZE(%s) > SAC_idx) && (0 <= SAC_idx))\n", res);
        indent++;
        INDENT;
    }
    fprintf (outfile, "SAC_ND_A_FIELD(%s)[", res);
    VectToOffset (dimv, AccessConst (vi, i), dimres, res);
    fprintf (outfile, "]=%s;\n", value[0]);
    INDENT;
    if (check_boundary) {
        fprintf (outfile, "else\n");
        indent++;
        INDENT;
        fprintf (outfile,
                 "SAC_OUT_OF_BOUND(%d, \"modarray\", SAC_ND_A_SIZE(%s), SAC_idx);\n",
                 line, res);
        indent -= 2;
        INDENT;
        fprintf (outfile, "}\n");
    }
    fprintf (outfile, "\n\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_MODARRAY_AxVxS_CHECK_REUSE( int line, char *res_type,
 *                                                     int dimres, char *res,
 *                                                     char *old, char **value,
 *                                                     int dim, char *v)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_MODARRAY_AxVxS_CHECK_REUSE( line, res_type, dimres, res, old, value,
 *                                      dimv, v)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_MODARRAY_AxVxS_CHECK_REUSE (int line, char *res_type, int dimres,
                                             char *res, char *old, char **value, int dim,
                                             char *v)
{
    DBUG_ENTER ("ICMCompileND_PRF_MODARRAY_AxVxS_CHECK_REUSE");

#define ND_PRF_MODARRAY_AxVxS_CHECK_REUSE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_MODARRAY_AxVxS_CHECK_REUSE

    fprintf (outfile, " SAC_ND_CHECK_REUSE_ARRAY(%s,%s)\n", old, res);
    INDENT;
    fprintf (outfile, "{ int SAC_i;\n");
    fprintf (outfile, "  SAC_ND_ALLOC_ARRAY(%s, %s, 0);\n", res_type, res);
    INDENT;
    fprintf (outfile, "  for(SAC_i=0; SAC_i<SAC_ND_A_SIZE(%s); SAC_i++)", res);
    fprintf (outfile, " SAC_ND_A_FIELD(%s)[SAC_i]=SAC_ND_A_FIELD(%s)[SAC_i];\n", res,
             old);
    INDENT;
    fprintf (outfile, "}\n");
    INDENT;
    if (check_boundary) {
        fprintf (outfile, "{  int SAC_idx=");
        VectToOffset (dim, AccessVect (v, i), dimres, res);
        fprintf (outfile, ";\n");
        indent++;
        INDENT;
        fprintf (outfile, "if( (SAC_ND_A_SIZE(%s) > SAC_idx) && (0 <= SAC_idx))\n", res);
        indent++;
        INDENT;
    }
    fprintf (outfile, "SAC_ND_A_FIELD(%s)[", res);
    VectToOffset (dim, AccessVect (v, i), dimres, res);
    fprintf (outfile, "]=%s;\n", value[0]);
    INDENT;
    if (check_boundary) {
        fprintf (outfile, "else\n");
        indent++;
        INDENT;
        fprintf (outfile,
                 "SAC_OUT_OF_BOUND(%d, \"modarray\", SAC_ND_A_SIZE(%s), SAC_idx);\n",
                 line, res);
        indent -= 2;
        INDENT;
        fprintf (outfile, "}\n");
    }
    fprintf (outfile, "\n\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_MODARRAY_AxVxS( int line, char *res_type, int dimres,
 *                                         char *res, char *old, char **value,
 *                                         int dim, char *v)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_MODARRAY_AxVxS( line, res_type, dimres, res, old, value, dim, v)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_MODARRAY_AxVxS (int line, char *res_type, int dimres, char *res,
                                 char *old, char **value, int dim, char *v)
{
    DBUG_ENTER ("ICMCompileND_PRF_MODARRAY_AxVxS");

#define ND_PRF_MODARRAY_AxVxS
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_MODARRAY_AxVxS

    fprintf (outfile, "{ int SAC_i;\n");
    fprintf (outfile, "  SAC_ND_ALLOC_ARRAY(%s, %s, 0);\n", res_type, res);
    INDENT;
    fprintf (outfile, "  for(SAC_i=0; SAC_i<SAC_ND_A_SIZE(%s); SAC_i++)", res);
    fprintf (outfile, " SAC_ND_A_FIELD(%s)[SAC_i]=SAC_ND_A_FIELD(%s)[SAC_i];\n", res,
             old);
    INDENT;
    fprintf (outfile, "}\n");
    INDENT;
    if (check_boundary) {
        fprintf (outfile, "{  int SAC_idx=");
        VectToOffset (dim, AccessVect (v, i), dimres, res);
        fprintf (outfile, ";\n");
        indent++;
        INDENT;
        fprintf (outfile, "if( (SAC_ND_A_SIZE(%s) > SAC_idx) && (0 <= SAC_idx))\n", res);
        indent++;
        INDENT;
    }
    fprintf (outfile, "SAC_ND_A_FIELD(%s)[", res);
    VectToOffset (dim, AccessVect (v, i), dimres, res);
    fprintf (outfile, "]=%s;\n", value[0]);
    INDENT;
    if (check_boundary) {
        fprintf (outfile, "else\n");
        indent++;
        INDENT;
        fprintf (outfile,
                 "SAC_OUT_OF_BOUND(%d, \"modarray\", SAC_ND_A_SIZE(%s), SAC_idx);\n",
                 line, res);
        indent -= 2;
        INDENT;
        fprintf (outfile, "}\n");
    }
    fprintf (outfile, "\n\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_MODARRAY_AxCxA( int line, char *res_type, int dimres,
 *                                         char *res, char *old, char *val,
 *                                         int dimv, char **vi)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_MODARRAY_AxCxA( line, res_type, dimres, res, old, val,
 *                                      dimv, v0,..., vn)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_MODARRAY_AxCxA (int line, char *res_type, int dimres, char *res,
                                 char *old, char *val, int dimv, char **vi)
{
    DBUG_ENTER ("ICMCompileND_PRF_MODARRAY_AxCxA");

#define ND_PRF_MODARRAY_AxCxA
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_MODARRAY_AxCxA

    fprintf (outfile, "{ int SAC_i, SAC_j;\n");
    fprintf (outfile, "  int SAC_idx=");
    VectToOffset (dimv, AccessConst (vi, i), dimres, res);
    fprintf (outfile, ";\n");
    INDENT;
    if (check_boundary) {
        fprintf (outfile,
                 "if( (SAC_ND_A_SIZE(%s) > (SAC_idx+SAC_ND_A_SIZE(%s)))"
                 "&& (0 <= (SAC_idx+SAC_ND_A_SIZE(%s))) ){\n",
                 res, val, val);
        indent++;
        INDENT;
    }
    fprintf (outfile, "  SAC_ND_ALLOC_ARRAY(%s, %s, 0);\n", res_type, res);
    INDENT;
    fprintf (outfile, "  for(SAC_i=0; SAC_i<SAC_idx-1; SAC_i++)");
    fprintf (outfile, " SAC_ND_A_FIELD(%s)[SAC_i]=SAC_ND_A_FIELD(%s)[SAC_i];\n", res,
             old);
    INDENT;
    fprintf (outfile,
             "  for(SAC_i=SAC_idx,SAC_j=0; SAC_j<SAC_ND_A_SIZE(%s); SAC_i++,SAC_j++)",
             val);
    fprintf (outfile, " SAC_ND_A_FIELD(%s)[SAC_i]=SAC_ND_A_FIELD(%s)[SAC_j];\n", res,
             val);
    INDENT;
    fprintf (outfile, " for(; SAC_i<SAC_ND_A_SIZE(%s); SAC_i++)\n", res);
    fprintf (outfile, " SAC_ND_A_FIELD(%s)[SAC_i]=SAC_ND_A_FIELD(%s)[SAC_i];\n", res,
             old);

    if (check_boundary) {
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
        INDENT;
        fprintf (outfile, "else\n");
        indent++;
        INDENT;
        fprintf (outfile,
                 "SAC_OUT_OF_BOUND(%d, \"modarray\", SAC_ND_A_SIZE(%s), SAC_idx);\n",
                 line, res);
        indent -= 2;
        INDENT;
        fprintf (outfile, "}\n");
    } else {
        INDENT;
        fprintf (outfile, "}\n");
    }
    fprintf (outfile, "\n\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_MODARRAY_AxCxA_CHECK_REUSE( int line, char *res_type,
 *                                                     int dimres, char *res,
 *                                                     char *old, char *val,
 *                                                     int dimv, char **vi)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_MODARRAY_AxCxA_CHECK_REUSE( line, res_type, dimres, res, old, val,
 *                                      dimv, v0,..., vn)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_MODARRAY_AxCxA_CHECK_REUSE (int line, char *res_type, int dimres,
                                             char *res, char *old, char *val, int dimv,
                                             char **vi)
{
    DBUG_ENTER ("ICMCompileND_PRF_MODARRAY_AxCxA_CHECK_REUSE");

#define ND_PRF_MODARRAY_AxCxA_CHECK_REUSE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_MODARRAY_AxCxA_CHECK_REUSE

    fprintf (outfile, "{  int SAC_i, SAC_j;\n");
    INDENT;
    fprintf (outfile, "  int SAC_idx=");
    VectToOffset (dimv, AccessConst (vi, i), dimres, res);
    fprintf (outfile, ";\n");
    INDENT;
    if (check_boundary) {
        fprintf (outfile,
                 "if( (SAC_ND_A_SIZE(%s) > (SAC_idx+SAC_ND_A_SIZE(%s)))"
                 "&& (0 <= (SAC_idx+SAC_ND_A_SIZE(%s))) ){\n",
                 res, val, val);
        indent++;
        INDENT;
    }
    fprintf (outfile, "if(SAC_ND_A_RC(%s)==1){\n", old);
    indent++;
    INDENT;
    fprintf (outfile, "SAC_ND_KS_ASSIGN_ARRAY(%s,%s)\n", old, res);
    INDENT;
    fprintf (outfile,
             "for(SAC_i=SAC_idx,SAC_j=0; SAC_j<SAC_ND_A_SIZE(%s); SAC_i++,SAC_j++)\n",
             val);
    indent++;
    INDENT;
    fprintf (outfile, " SAC_ND_A_FIELD(%s)[SAC_i]=SAC_ND_A_FIELD(%s)[SAC_j];\n", res,
             val);
    indent -= 2;
    INDENT;
    fprintf (outfile, "}\n");
    INDENT;
    fprintf (outfile, "else{\n");
    indent++;
    INDENT;
    fprintf (outfile, "SAC_ND_ALLOC_ARRAY(%s, %s, 0);\n", res_type, res);
    INDENT;
    fprintf (outfile, "for(SAC_i=0; SAC_i<SAC_idx-1; SAC_i++)\n");
    indent++;
    INDENT;
    fprintf (outfile, " SAC_ND_A_FIELD(%s)[SAC_i]=SAC_ND_A_FIELD(%s)[SAC_i];\n", res,
             old);
    indent--;
    INDENT;
    fprintf (outfile,
             "for(SAC_i=SAC_idx,SAC_j=0; SAC_j<SAC_ND_A_SIZE(%s); SAC_i++,SAC_j++)\n",
             val);
    indent++;
    INDENT;
    fprintf (outfile, "SAC_ND_A_FIELD(%s)[SAC_i]=SAC_ND_A_FIELD(%s)[SAC_j];\n", res, val);
    indent--;
    INDENT;
    fprintf (outfile, "for(; SAC_i<SAC_ND_A_SIZE(%s); SAC_i++)\n", res);
    indent++;
    INDENT;
    fprintf (outfile, "SAC_ND_A_FIELD(%s)[SAC_i]=SAC_ND_A_FIELD(%s)[SAC_i];\n", res, old);
    indent -= 2;
    INDENT;
    fprintf (outfile, "}\n");
    INDENT;

    if (check_boundary) {
        fprintf (outfile, "}\n");
        INDENT;
        fprintf (outfile, "else\n");
        indent++;
        INDENT;
        fprintf (outfile,
                 "SAC_OUT_OF_BOUND(%d, \"modarray\", SAC_ND_A_SIZE(%s), SAC_idx);\n",
                 line, res);
        indent -= 2;
        INDENT;
    }
    fprintf (outfile, "}\n");
    fprintf (outfile, "\n\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_BEGIN_GENARRAY( char *res, int dimres, char *from, char *to,
 *                                     char *idx, int idxlen)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_BEGIN_GENARRAY( res, dimres, from, to, idx, idxlen)
 *
 ******************************************************************************/

void
ICMCompileND_BEGIN_GENARRAY (char *res, int dimres, char *from, char *to, char *idx,
                             int idxlen)
{
    DBUG_ENTER ("ICMCompileND_BEGIN_GENARRAY");

#define ND_BEGIN_GENARRAY
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_BEGIN_GENARRAY

    BeginWith (res, dimres, from, to, idx, idxlen, fprintf (outfile, "0"), "genarray");

#ifdef TEST_BACKEND
    indent -= idxlen + 1;
#endif /* TEST_BACKEND */

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_BEGIN_MODARRAY( char *res, int dimres, char *a,
 *                                     char *from, char *to,
 *                                     char *idx, int idxlen)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_BEGIN_MODARRAY( res, dimres, a, from, to, idx, idxlen)
 *
 ******************************************************************************/

void
ICMCompileND_BEGIN_MODARRAY (char *res, int dimres, char *a, char *from, char *to,
                             char *idx, int idxlen)
{
    DBUG_ENTER ("ICMCompileND_BEGIN_MODARRAY");

#define ND_BEGIN_MODARRAY
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_BEGIN_MODARRAY

    BeginWith (res, dimres, from, to, idx, idxlen,
               fprintf (outfile, "SAC_ND_A_FIELD(%s)[%s__destptr]", a, res), "modarray");

#ifdef TEST_BACKEND
    indent -= idxlen + 1;
#endif /* TEST_BACKEND */

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_BEGIN_FOLDPRF( char *res, int dimres, char *from, char *to,
 *                                    char *idx, int idxlen, int n_neutral, char
 ***neutral)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_BEGIN_FOLDPRF( res, dimres, a, from, to, idx, idxlen, n_neutral, neutral)
 *
 ******************************************************************************/

void
ICMCompileND_BEGIN_FOLDPRF (char *res, int dimres, char *from, char *to, char *idx,
                            int idxlen, int n_neutral, char **neutral)
{
    DBUG_ENTER ("ICMCompileND_BEGIN_FOLDPRF");

#define ND_BEGIN_FOLDPRF
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_BEGIN_FOLDPRF

#ifdef OLD_FOLD
    BeginFoldWith (res, dimres, neutral, from, to, idx, idxlen, 1);
#else
    BeginFoldWith (res, dimres, from, to, idx, idxlen, n_neutral, neutral);
#endif

#ifdef TEST_BACKEND
    indent -= idxlen + 1;
#endif /* TEST_BACKEND */

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_BEGIN_FOLDFUN( char *res, int dimres, char *from, char *to,
 *                                    char *idx, int idxlen, int n_neutral, char
 ***neutral)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_BEGIN_FOLDFUN( res, dimres, a, from, to, idx, idxlen, n_neutral, neutral)
 *
 ******************************************************************************/

void
ICMCompileND_BEGIN_FOLDFUN (char *res, int dimres, char *from, char *to, char *idx,
                            int idxlen, int n_neutral, char **neutral)
{
    DBUG_ENTER ("ICMCompileND_BEGIN_FOLDFUN");

#define ND_BEGIN_FOLDFUN
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_BEGIN_FOLDFUN

#ifdef OLD_FOLF
    BeginFoldWith (res, dimres, neutral, from, to, idx, idxlen, 0);
#else
    BeginFoldWith (res, dimres, from, to, idx, idxlen, n_neutral, neutral);
#endif

#ifdef TEST_BACKEND
    indent -= idxlen + 1;
#endif /* TEST_BACKEND */

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_END_GENARRAY_S( char *res, int dimres, char **valstr)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_END_GENARRAY_S( res, dimres, valstr)
 *
 ******************************************************************************/

void
ICMCompileND_END_GENARRAY_S (char *res, int dimres, char **valstr)
{
    DBUG_ENTER ("ICMCompileND_END_GENARRAY_S");

#define ND_END_GENARRAY_S
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_END_GENARRAY_S

#ifdef TEST_BACKEND
    indent += dimres + 1;
#endif /* TEST_BACKEND */

    RetWithScal (res, valstr);
    EndWith (res, dimres, dimres, fprintf (outfile, "0"), "genarray");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_END_GENARRAY_A( char *res, int dimres, char *reta, int idxlen)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_END_GENARRAY_A( res, dimres, reta, idxlen)
 *
 ******************************************************************************/

void
ICMCompileND_END_GENARRAY_A (char *res, int dimres, char *reta, int idxlen)
{
    DBUG_ENTER ("ICMCompileND_END_GENARRAY_A");

#define ND_END_GENARRAY_A
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_END_GENARRAY_A

#ifdef TEST_BACKEND
    indent += idxlen + 1;
#endif /* TEST_BACKEND */

    RetWithArray (res, reta);
    EndWith (res, dimres, idxlen, fprintf (outfile, "0"), "genarray");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_END_MODARRAY_S( char *res, int dimres, char *a, char **valstr)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_END_MODARRAY_S( res, dimres, a, valstr)
 *
 ******************************************************************************/

void
ICMCompileND_END_MODARRAY_S (char *res, int dimres, char *a, char **valstr)
{
    DBUG_ENTER ("ICMCompileND_END_MODARRAY_S");

#define ND_END_MODARRAY_S
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_END_MODARRAY_S

#ifdef TEST_BACKEND
    indent += dimres + 1;
#endif /* TEST_BACKEND */

    RetWithScal (res, valstr);
    EndWith (res, dimres, dimres,
             fprintf (outfile, "SAC_ND_A_FIELD(%s)[%s__destptr]", a, res), "modarray");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_END_FOLD( int idxlen)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_END_FOLD( idxlen )
 *
 ******************************************************************************/

void
ICMCompileND_END_FOLD (int idxlen)
{
    DBUG_ENTER ("ICMCompileND_END_FOLD");

#define ND_END_FOLD
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_END_FOLD

#ifdef TEST_BACKEND
    indent += idxlen + 1;
#endif /* TEST_BACKEND */

    EndFoldWith (idxlen);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KS_VECT2OFFSET( char *name, int dim, int dims, char **s)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KS_VECT2OFFSET( name, dim, s )
 *
 ******************************************************************************/

void
ICMCompileND_KS_VECT2OFFSET (char *name, int dim, int dims, char **s)
{
    DBUG_ENTER ("ICMCompileND_KS_VECT2OFFSET");

#define ND_KS_VECT2OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KS_VECT2OFFSET

#ifdef TEST_BACKEND
    indent += idxlen + 1;
#endif /* TEST_BACKEND */

    INDENT;
    fprintf (outfile, "SAC_%s", name);
    {
        int i;
        for (i = 0; i < dims; i++) {
            fprintf (outfile, "_%s", s[i]);
        }
        fprintf (outfile, "= ");
        VectToOffset2 (dim, AccessVect (name, i), dims, AccessConst (s, i));
        fprintf (outfile, ";\n");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_END_MODARRAY_A( char *res, int dimres, char *a, char *reta,
 *                                     int idxlen)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_END_MODARRAY_A( res, dimres, a, reta, idxlen)
 *
 ******************************************************************************/

void
ICMCompileND_END_MODARRAY_A (char *res, int dimres, char *a, char *reta, int idxlen)
{
    DBUG_ENTER ("ICMCompileND_END_MODARRAY_A");

#define ND_END_MODARRAY_A
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_END_MODARRAY_A

#ifdef TEST_BACKEND
    indent += idxlen + 1;
#endif /* TEST_BACKEND */

    RetWithArray (res, reta);
    EndWith (res, dimres, idxlen,
             fprintf (outfile, "SAC_ND_A_FIELD(%s)[%s__destptr]", a, res), "modarray");

    DBUG_VOID_RETURN;
}
