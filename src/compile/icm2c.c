/*
 *
 * $Log$
 * Revision 1.45  1997/05/14 08:09:33  sbs
 * ANALYSE_BEGIN/END_WITH... inserted.
 *
 * Revision 1.44  1997/05/06  15:41:52  sbs
 * ANALYSE_TIME_BEGIN / END for with-loops inserted
 *
 * Revision 1.43  1997/04/28  08:09:06  cg
 * Global world object Parameters renamed to TheCommandLine.
 * Special treatment in function definition and application of
 * create function adjusted.
 *
 * Revision 1.42  1997/04/25  09:29:32  sbs
 * NULL -> 0 in compaison with tyarg[0]
 *
 * Revision 1.41  1996/09/11  16:00:43  cg
 * bug fixed in FUN_DEC
 *
 * Revision 1.40  1996/09/11  06:28:20  cg
 * Modified ICMs FUNDEC and FUNAP in order to support analysis
 * of command line parameters via standard class CommandLine.
 *
 * Revision 1.39  1996/09/09  12:32:05  cg
 * ICMs for global objects modified in order allow construction of header files.
 *
 * Revision 1.38  1996/08/04  14:40:12  sbs
 * modarray_AxVxA with and without reuse check inserted.
 *
 * Revision 1.37  1996/04/28  12:45:49  sbs
 * chnaged RetWithArray-Macro; inserted
 * fprintf(outfile,"ND_DEC_RC_FREE_ARRAY( %s, 1);\n", a);
 * at the end of each with-instance, since otherwise
 * the "return"-arrays of the with-body were not deleted at all!
 *
 * Revision 1.36  1996/02/06  16:11:37  sbs
 * using Double2String and Float2String in icm_args.c
 *
 * Revision 1.35  1996/01/25  15:44:20  cg
 * renamed last forgotten ND_REUSE_ARRAY to ND_KS_ASSIGN_ARRAY
 *
 * Revision 1.34  1996/01/25  15:03:03  cg
 * renamed some icm macros
 *
 * Revision 1.33  1996/01/21  14:52:31  cg
 * bug fixed in creating arrays of type void*
 *
 * Revision 1.32  1996/01/21  14:11:49  cg
 * added new icms for refcounting external implicit types
 *
 * Revision 1.31  1995/12/18  16:27:17  cg
 * ICMs ND_FUN_DEC, ND_FUN_AP and ND_FUN_RET modified
 * new ICMs ND_KS_DECL_GLOBAL_ARRAY and ND_KD_DECL_EXTERN_ARRAY for
 * arrays as global objects.
 *
 * Revision 1.30  1995/10/08  11:22:36  sbs
 * some bugs fixed
 *
 * Revision 1.29  1995/09/05  11:36:50  hw
 * changed condition of boundary check of PRF_MODARRAY...
 * ( ">=" -> ">" )
 *
 * Revision 1.28  1995/08/16  10:03:58  sbs
 * bug fixed in ND_PRF_MODARRAY_AxCxS_CHECK_REUSE
 * ( CHECK_REUSE ( old <-> new ) )
 *
 * Revision 1.27  1995/08/15  14:38:22  hw
 * added ICMs ND_PRF_MODARRAY_AxCxS_CHECK_REUSE, ND_PRF_MODARRAY_AxCxS,
 * ND_PRF_MODARRAY_AxCxA and  ND_PRF_MODARRAY_AxCxA_CHECK_REUSE
 *
 * Revision 1.26  1995/07/13  16:42:39  hw
 * - changed N_icm ND_BEGIN_FOLDPRF & ND_BEGIN_FOLDFUN
 * - changed ND_FUN_DEC to handel declaration of imported functions
 *   ( imported function do not have formal paramter names anymore !!!)
 *
 * Revision 1.25  1995/06/09  15:34:30  hw
 * - changed N_icms ND_KD_PSI_... (linenumber inserted)
 * - boundery check for ND_KD_PSI_... inserted
 *
 * Revision 1.24  1995/06/08  18:30:13  hw
 * - changed names in call of ND_BEGIN_FOLDPRF & ND_BEGIN_FOLDFUN for BEtest
 *
 * Revision 1.23  1995/06/07  13:36:53  hw
 * - exchanges N_icm ND_CREATE_CONST_ARRAY with ND_CREATE_CONST_ARRAY_S
 * - N_icm ND_CREATE_CONST_ARRAY_A (array out of arrays) inserted
 *
 * Revision 1.22  1995/06/02  11:28:16  hw
 * - added macros BeginFoldWith & EndFoldWith
 * - implementation of N_icm's ND_BEGIN_FOLDPRF, ND_BEGIN_FOLDFUN & ND_END_FOLD
 *   inserted
 *
 * Revision 1.21  1995/05/24  15:31:22  sbs
 * trace.h included
 *
 * Revision 1.20  1995/05/24  13:58:31  sbs
 * ND_KS_DECL_ARRAY_ARG inserted
 *
 * Revision 1.19  1995/05/04  11:43:51  sbs
 * icm_trace.c inserted
 *
 * Revision 1.18  1995/05/02  07:11:31  sbs
 * with-constructs debugged
 *
 * Revision 1.17  1995/04/27  12:57:05  sbs
 * WITH-LOOP ICM s inserted.
 *
 * Revision 1.16  1995/04/18  14:45:55  sbs
 * ND_CREATE_CONST_ARRAY modified; initial basic_type parameter eliminated.
 *
 * Revision 1.15  1995/04/18  11:20:34  sbs
 * ND_KD_ROT_SxSxA_A eliminated & ND_KD_ROT_CxSxA_A modified
 *
 * Revision 1.14  1995/04/13  09:12:59  sbs
 * ND_KD_ROT_SxSxA_A inserted
 *
 * Revision 1.13  1995/04/12  15:14:00  sbs
 * cat & rot inserted.
 *
 * Revision 1.12  1995/04/12  07:02:53  sbs
 * separation of IN and OUT arrays added
 *
 * Revision 1.11  1995/04/11  16:08:48  sbs
 * some minor bugs fixed
 *
 * Revision 1.10  1995/04/11  15:06:35  sbs
 * arg->tyarg in ND_FUN_DEC
 *
 * Revision 1.9  1995/04/11  15:03:13  sbs
 * ND_FUN_[DEC/AP/RET] inserted
 *
 * Revision 1.8  1995/04/07  12:19:48  sbs
 * psi, take & drop fixed for args of different dimensionality
 *
 * Revision 1.7  1995/04/06  14:23:25  sbs
 * some bugs fixed
 *
 * Revision 1.6  1995/04/03  13:58:57  sbs
 * first "complete" version
 *
 * Revision 1.5  1995/03/31  13:57:34  sbs
 * ND_CREATE_CONST_ARRAY,ND_KS_ARG_ARRAY & ND_KS_RET_ARRAY inserted
 *
 * Revision 1.4  1995/03/15  07:55:11  sbs
 * bug in VectToOffset fixed
 *
 * Revision 1.3  1995/03/10  17:24:37  sbs
 * New macros develloped; psi, take & drop integrated
 *
 * Revision 1.2  1995/03/08  17:13:08  sbs
 * first version with test embedded
 *
 * Revision 1.1  1995/03/07  18:21:02  sbs
 * Initial revision
 *
 *
 */

#include <malloc.h>
#include "stdio.h"
#include "print.h"
#include "dbug.h"
#include "my_debug.h"
#include "main.h"
#include "trace.h"
#include "convert.h"
#include "globals.h"

#define RetWithScal(res, val)                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "ND_A_FIELD(%s)[%s__destptr++]=%s;\n", res, res, val[0]);

#define RetWithArray(res, a)                                                             \
    INDENT;                                                                              \
    fprintf (outfile, "{ int __i;\n\n");                                                 \
    indent++;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "for(__i=0; __i<ND_A_SIZE(%s); __i++)\n", a);                      \
    indent++;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "ND_A_FIELD(%s)[%s__destptr++]=ND_A_FIELD(%s)[__i];\n", res, res,  \
             a);                                                                         \
    indent--;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "ND_DEC_RC_FREE_ARRAY( %s, 1);\n", a);                             \
    indent--;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "}\n")

#define BeginWith(res, dimres, from, to, idx, idxlen, fillstr, withkind)                 \
    INDENT;                                                                              \
    fprintf (outfile, "{ int __i;\n");                                                   \
    indent++;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "int %s__destptr=0;\n", res);                                      \
    INDENT;                                                                              \
    fprintf (outfile, "int __start=");                                                   \
    VectToOffset (idxlen, AccessVect (from, i), dimres, res);                            \
    fprintf (outfile, ";\n");                                                            \
    {                                                                                    \
        int i, j;                                                                        \
        for (i = 1; i < idxlen; i++) {                                                   \
            INDENT;                                                                      \
            fprintf (outfile, "int %s__offset%d=", res, i);                              \
            fprintf (outfile, "(ND_KD_A_SHAPE(%s, %d)-ND_A_FIELD(%s)[%d]", res, i, to,   \
                     i);                                                                 \
            fprintf (outfile, "+ND_A_FIELD(%s)[%d]-1)", from, i);                        \
            for (j = (i + 1); j < dimres; j++)                                           \
                fprintf (outfile, " *ND_KD_A_SHAPE(%s, %d)", res, j);                    \
            fprintf (outfile, ";\n");                                                    \
        }                                                                                \
        fprintf (outfile, "\n");                                                         \
        INDENT;                                                                          \
        fprintf (outfile, "ANALYSE_BEGIN_WITH( " withkind " );\n");                      \
        INDENT;                                                                          \
        fprintf (outfile, "while( %s__destptr < __start) {\n", res);                     \
        indent++;                                                                        \
        INDENT;                                                                          \
        fprintf (outfile, "ND_A_FIELD(%s)[%s__destptr]=", res, res);                     \
        fillstr;                                                                         \
        fprintf (outfile, ";\n");                                                        \
        INDENT;                                                                          \
        fprintf (outfile, "%s__destptr++;\n", res);                                      \
        indent--;                                                                        \
        INDENT;                                                                          \
        fprintf (outfile, "}\n");                                                        \
        {                                                                                \
            int i;                                                                       \
            for (i = 0; i < idxlen; i++) {                                               \
                INDENT;                                                                  \
                fprintf (outfile,                                                        \
                         "for( ND_A_FIELD(%s)[%d]=ND_A_FIELD(%s)[%d]; "                  \
                         "ND_A_FIELD(%s)[%d]<=ND_A_FIELD(%s)[%d]; "                      \
                         "ND_A_FIELD(%s)[%d]++) {\n",                                    \
                         idx, i, from, i, idx, i, to, i, idx, i);                        \
                indent++;                                                                \
            }                                                                            \
        }                                                                                \
        fprintf (outfile, "\n");                                                         \
    }

#ifdef OLD_FOLD
#define BeginFoldWith(res, dimres, neutral, form, to, idx, idixlen, fun_tag)             \
    INDENT;                                                                              \
    fprintf (outfile, "{ int __i;\n");                                                   \
    indent++;                                                                            \
    INDENT;                                                                              \
    if (0 < dimres) {                                                                    \
        fprintf (outfile, "for(__i=0; __i < ND_A_SIZE(%s); __i++)\n", res);              \
        indent++;                                                                        \
        INDENT;                                                                          \
        if (1 == fun_tag)                                                                \
            fprintf (outfile, " %s[__i]=%s;\n", res, neutral[0]);                        \
        else                                                                             \
            fprintf (outfile, " %s[__i]=ND_A_FIELD(%s)[__i];\n", res, neutral[0]);       \
        indent--;                                                                        \
    } else                                                                               \
        fprintf (outfile, " %s=%s;\n", res, neutral[0]);                                 \
    {                                                                                    \
        int i;                                                                           \
        for (i = 0; i < idxlen; i++) {                                                   \
            INDENT;                                                                      \
            fprintf (outfile,                                                            \
                     "for( ND_A_FIELD(%s)[%d]=ND_A_FIELD(%s)[%d]; "                      \
                     "ND_A_FIELD(%s)[%d]<=ND_A_FIELD(%s)[%d]; ND_A_FIELD(%s)[%d]++) "    \
                     "{\n",                                                              \
                     idx, i, from, i, idx, i, to, i, idx, i);                            \
            indent++;                                                                    \
        }                                                                                \
    }                                                                                    \
    fprintf (outfile, "\n")
#else
#define BeginFoldWith(res, sizeres, form, to, idx, idixlen, n_neutral, neutral)          \
    INDENT;                                                                              \
    fprintf (outfile, "{ ANALYSE_BEGIN_WITH( fold );\n");                                \
    indent++;                                                                            \
    INDENT;                                                                              \
    if (0 < sizeres) {                                                                   \
        indent++;                                                                        \
        INDENT;                                                                          \
        {                                                                                \
            int i, j;                                                                    \
            if (sizeres == n_neutral)                                                    \
                for (i = 0; i < n_neutral; i += 1)                                       \
                    if (1 == IsDigit ((neutral[i])[0]))                                  \
                        fprintf (outfile, "ND_A_FIELD(%s)[%d]=%s;\n", res, i,            \
                                 neutral[i]);                                            \
                    else                                                                 \
                        fprintf (outfile, "ND_A_FIELD(%s)[%d]=ND_A_FIELD(%s);\n", res,   \
                                 i, neutral[i]);                                         \
            else                                                                         \
                for (i = 0, j = 0; i < n_neutral; i += 1, j = i / n_neutral)             \
                    fprintf (outfile, " ND_A_FIELD(%s)[%d]=ND_A_FIELD(%s)[%d];\n", res,  \
                             i, neutral[j], i - n_neutral * j);                          \
        }                                                                                \
    } else                                                                               \
        fprintf (outfile, " %s=%s;\n", res, neutral[0]);                                 \
    {                                                                                    \
        int i;                                                                           \
        for (i = 0; i < idxlen; i++) {                                                   \
            INDENT;                                                                      \
            fprintf (outfile,                                                            \
                     "for( ND_A_FIELD(%s)[%d]=ND_A_FIELD(%s)[%d]; "                      \
                     "ND_A_FIELD(%s)[%d]<=ND_A_FIELD(%s)[%d]; ND_A_FIELD(%s)[%d]++) "    \
                     "{\n",                                                              \
                     idx, i, from, i, idx, i, to, i, idx, i);                            \
            indent++;                                                                    \
        }                                                                                \
    }                                                                                    \
    fprintf (outfile, "\n")

#endif

#define EndWith(res, dimres, idxlen, fillstr, withkind)                                  \
    indent--;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "}\n");                                                            \
    {                                                                                    \
        int i;                                                                           \
        for (i = idxlen - 1; i > 0; i--) {                                               \
            INDENT;                                                                      \
            fprintf (outfile, "for(__i=0; __i<%s__offset%d; __i++) {\n", res, i);        \
            indent++;                                                                    \
            INDENT;                                                                      \
            fprintf (outfile, "ND_A_FIELD(%s)[%s__destptr]=", res, res);                 \
            fillstr;                                                                     \
            fprintf (outfile, ";\n");                                                    \
            INDENT;                                                                      \
            fprintf (outfile, "%s__destptr++;\n", res);                                  \
            indent--;                                                                    \
            INDENT;                                                                      \
            fprintf (outfile, "}\n");                                                    \
            indent--;                                                                    \
            INDENT;                                                                      \
            fprintf (outfile, "}\n");                                                    \
        }                                                                                \
    }                                                                                    \
    INDENT;                                                                              \
    fprintf (outfile, "while( %s__destptr< ND_A_SIZE(%s)) {\n", res, res);               \
    indent++;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "ND_A_FIELD(%s)[%s__destptr]=", res, res);                         \
    fillstr;                                                                             \
    fprintf (outfile, ";\n");                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "%s__destptr++;\n", res);                                          \
    indent--;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "}\n");                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "ANALYSE_END_WITH( " withkind " );\n");                            \
    indent--;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "}\n\n")

#ifdef OLD_FOLD
#define EndFoldWith(idxlen)                                                              \
    {                                                                                    \
        int i;                                                                           \
        for (i = 0; i <= idxlen; i++) {                                                  \
            indent--;                                                                    \
            INDENT;                                                                      \
            fprintf (outfile, "}\n");                                                    \
        }                                                                                \
    }
#else
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
    fprintf (outfile, "ANALYSE_END_WITH( fold );\n");                                    \
    indent--;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "}\n\n")
#endif

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

#define AccessVect(v, i) fprintf (outfile, "ND_A_FIELD(%s)[%i]", v, i)

#define AccessConst(v, i) fprintf (outfile, "%s", v[i])

#define AccessShape(v, i) fprintf (outfile, "ND_KD_A_SHAPE(%s, %d)", v, i)

#define VectToOffset2(dim, v_i_str, dima, a_i_str)                                       \
    {                                                                                    \
        int i;                                                                           \
        for (i = dim - 1; i > 0; i--) {                                                  \
            fprintf (outfile, "( ");                                                     \
            a_i_str;                                                                     \
            fprintf (outfile, "* ");                                                     \
        }                                                                                \
        v_i_str;                                                                         \
        for (i = 1; i < dim; i++) {                                                      \
            fprintf (outfile, "+");                                                      \
            v_i_str;                                                                     \
            fprintf (outfile, ") ");                                                     \
        }                                                                                \
        while (i < dima) {                                                               \
            fprintf (outfile, "*");                                                      \
            a_i_str;                                                                     \
            fprintf (outfile, " ");                                                      \
            i++;                                                                         \
        }                                                                                \
    }

#define VectToOffset(dim, v_i_str, dima, a)                                              \
    VectToOffset2 (dim, v_i_str, dima, AccessShape (a, i))

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
    fprintf (outfile, "int __isrc=");                                                    \
    src;                                                                                 \
    fprintf (outfile, ";\n");                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "int __idest=");                                                   \
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

#define InitIMaxs(from, to, v_i_str) InitVecs (from, to, "__imax", v_i_str)

#define InitSrcOffs(from, to, v_i_str) InitVecs (from, to, "__srcoff", v_i_str)

#define FillRes(res, body)                                                               \
    INDENT;                                                                              \
    fprintf (outfile, "do {\n");                                                         \
    indent++;                                                                            \
    body;                                                                                \
    indent--;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "}\n");                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "while( __idest<ND_A_SIZE(%s));\n", res)

#define AccessSeg(dim, body)                                                             \
    {                                                                                    \
        int i;                                                                           \
        INDENT;                                                                          \
        fprintf (outfile, "{\n");                                                        \
        indent++;                                                                        \
        for (i = 1; i < dim; i++) {                                                      \
            INDENT;                                                                      \
            fprintf (outfile, "int __i%d;\n", i);                                        \
        }                                                                                \
        for (i = 1; i < dim; i++) {                                                      \
            INDENT;                                                                      \
            fprintf (outfile, "for( __i%d=0; __i%d<__imax%d; __i%d++) {\n", i, i, i, i); \
            indent++;                                                                    \
        }                                                                                \
        body;                                                                            \
        for (i = dim - 1; i > 0; i--) {                                                  \
            indent--;                                                                    \
            INDENT;                                                                      \
            fprintf (outfile, "}\n");                                                    \
            INDENT;                                                                      \
            fprintf (outfile, "__isrc += __srcoff%d;\n", i);                             \
        }                                                                                \
        indent--;                                                                        \
        INDENT;                                                                          \
        fprintf (outfile, "}\n");                                                        \
    }

#define CopyBlock(a, offset, res, line)                                                  \
    NewBlock (InitPtr (offset, fprintf (outfile, "0")),                                  \
              FillRes (                                                                  \
                res, INDENT; if (check_boundary) {                                       \
                    fprintf (outfile, "if((0 <= __isrc)&&( __isrc < ND_A_SIZE(%s)))\n",  \
                             a);                                                         \
                    indent++;                                                            \
                    INDENT;                                                              \
                } fprintf (outfile,                                                      \
                           "ND_A_FIELD(%s)[__idest++]=ND_A_FIELD(%s)[__isrc++];\n", res, \
                           a);                                                           \
                if (check_boundary) {                                                    \
                    indent--;                                                            \
                    INDENT;                                                              \
                    fprintf (outfile, "else\n");                                         \
                    indent++;                                                            \
                    INDENT;                                                              \
                    fprintf (outfile,                                                    \
                             "OUT_OF_BOUND(%d, \"psi\", ND_A_SIZE(%s), __isrc);\n",      \
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
              InitIMaxs (dimi, dima, fprintf (outfile, "ND_KD_A_SHAPE(%s, %d)", a, i));  \
              InitSrcOffs (                                                              \
                0, dimi, off_i_str; {                                                    \
                    int j;                                                               \
                    for (j = i + 1; j < dima; j++)                                       \
                        fprintf (outfile, "*ND_KD_A_SHAPE(%s, %d)", a, j);               \
                }) InitSrcOffs (dimi, dima, fprintf (outfile, "0")),                     \
              FillRes (res,                                                              \
                       AccessSeg (dima, INDENT; fprintf (outfile,                        \
                                                         "ND_A_FIELD(%s)[__idest++]=ND_" \
                                                         "A_FIELD(%s)[__isrc++];\n",     \
                                                         res, a);)))

int
IsDigit (char alpha)
{
    int ret;

    DBUG_ENTER ("IsDigit");
    switch (alpha) {
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '0':
        ret = 1;
        break;
    default:
        ret = 0;
        break;
    }
    DBUG_RETURN (ret);
}

#ifdef TEST_BACKEND

MAIN
{

    typedef char *string;

    FILE *outfile = stdout;
    char type[] = "double";
    char name[] = "array_name";
    char rettype[] = "int";
    char retname[] = "result";
    int dim = 3, dims = 3, dima = 4, dimv = 3, dimres = 4;
    char *s[] = {"40", "50", "60"};
    char *vi[] = {"10", "20", "30"};
    char *ar[] = {"firstarray", "secondarray"};
    char *arg[] = {"in", "i", "in_rc", "ia", "out", "o1", "out", "o2", "out_rc", "oa"};
    char *tyarg[] = {"in", "int", "i",   "in_rc", "int *",  "ia",    "out", "int",
                     "o1", "out", "int", "o2",    "out_rc", "int *", "oa"};
    char *rotdimstr[] = {"rotindim"};
    char *numstr[] = {"rotnum"};
    char *valstr[] = {"ret-val"};
    char *neutral[] = {"neutral"};
    char *value[] = {"value"};
    char v[] = "vector";
    char a[] = "arg";
    char reta[] = "ret-array";
    char a1[] = "arg1";
    char from[] = "fromvar";
    char to[] = "tovar";
    char idx[] = "idxvar";
    char a2[] = "arg2";
    char res[] = "result";
    char old[] = "old";
    char res_type[] = "type";
    char a_value[] = "array";
    char *A[] = {"B", "C", "D"};
    char copyfun[] = "copy_xyz";

    int idxlen = 3;
    int i;
    int narg = 5;
    int indent = 1;
    int catdim = 2;
    int rotdim = 1;
    int length = 3;
    int traceflag = 0xffff;
    int line = 777;
    int check_boundary = 0;
    int n_neutral = 1;

    OPT ARG 'b':
    {
        check_boundary = 1;
    }
    OTHER
    {
        fprintf (stderr, "unknown option \"%c\"\n", **argv);
    }
    ENDOPT

#else  /* TEST_BACKEND */

extern FILE *outfile;      /* outputfile for PrintTree defined in main.c*/
extern int check_boundary; /* defined in main.c */
#endif /* TEST_BACKEND */

    /*
     * ND_FUN_DEC( name, rettype, narg, [ TAG, type, arg ]* )
     *
     * {in, in_rc, out, out_rc, inout, inout_rc} TAG
     *
     * char *name;
     * char *rettype;
     * int narg;
     * char **tyarg;
     */

#define ND_FUN_DEC

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

    INDENT;
    fprintf (outfile, "%s ", rettype);
    if (strcmp (name, "main") == 0) {
        fprintf (outfile, "%s( int __argc, char *__argv[])", name);
    } else if (strcmp (name, "create_TheCommandLine") == 0) {
        fprintf (outfile, "%s( int __argc, char **__argv)", name);
    } else {
        fprintf (outfile, "%s( ", name);
        ScanArglist (
          tyarg, 3 * narg, fprintf (outfile, " %s %s", tyarg[i++], tyarg[i++]);
          sep = 1,

          if (0 != (tyarg[i + 1])[0]) {
              fprintf (outfile, " %s *%s__p", tyarg[i++], tyarg[i++]);
              sep = 1;
          } else {
              fprintf (outfile, " %s *%s", tyarg[i++], tyarg[i++]);
              sep = 1;
          },

          if (0 != (tyarg[i + 1])[0]) {
              fprintf (outfile, " %s *%s__p", tyarg[i++], tyarg[i++]);
              sep = 1;
          } else {
              fprintf (outfile, " %s *%s", tyarg[i++], tyarg[i++]);
              sep = 1;
          },

          fprintf (outfile, " %s *%s", tyarg[i++], tyarg[i++]);
          sep = 1, fprintf (outfile, " %s %s", tyarg[i++], tyarg[i++]);
          sep = 1,

          if (0 != (tyarg[i + 1])[0])
            fprintf (outfile, " ND_KS_DEC_IN_RC(%s, %s)", tyarg[i++], tyarg[i++]);
          else {
              fprintf (outfile, "ND_KS_DEC_IMPORT_IN_RC(%s)", tyarg[i++]);
              i++;
          };
          sep = 1,

          if (0 != (tyarg[i + 1])[0])
            fprintf (outfile, " ND_KS_DEC_OUT_RC(%s, %s)", tyarg[i++], tyarg[i++]);
          else {
              fprintf (outfile, "ND_KS_DEC_IMPORT_OUT_RC(%s)", tyarg[i++]);
              i++;
          };
          sep = 1,

          if (0 != (tyarg[i + 1])[0])
            fprintf (outfile, " ND_KS_DEC_INOUT_RC(%s, %s)", tyarg[i++], tyarg[i++]);
          else {
              fprintf (outfile, "ND_KS_DEC_IMPORT_INOUT_RC(%s)", tyarg[i++]);
              i++;
          };
          sep = 1,

          ",");
        fprintf (outfile, ")");
    }

#undef ND_FUN_DEC

#ifndef TEST_BACKEND
    DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_FUN_AP( name, retname, n,
 *            [ {in, in_rc, out, out_rc, inout, inout_rc}, arg ] )
 *
 * char *name;
 * char *retname;
 * int narg;
 * char **arg;
 */

#define ND_FUN_AP

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

INDENT;
if (0 != strcmp (retname, ""))
    fprintf (outfile, "%s =", retname);
if (strcmp (name, "create_TheCommandLine") == 0) {
    fprintf (outfile, "%s( __argc, __argv);", name);
} else {
    fprintf (outfile, "%s( ", name);
    ScanArglist (arg, 2 * narg, fprintf (outfile, " %s", arg[i++]);
                 sep = 1, fprintf (outfile, " &%s", arg[i++]);
                 sep = 1, fprintf (outfile, " &%s", arg[i++]);
                 sep = 1, fprintf (outfile, " &%s", arg[i++]);
                 sep = 1, fprintf (outfile, " %s", arg[i++]);
                 sep = 1, fprintf (outfile, " ND_KS_AP_IN_RC(%s)", arg[i++]);
                 sep = 1, fprintf (outfile, " ND_KS_AP_OUT_RC(%s)", arg[i++]);
                 sep = 1, fprintf (outfile, " ND_KS_AP_INOUT_RC(%s)", arg[i++]);
                 sep = 1, ",");
    fprintf (outfile, ");");
}

#undef ND_FUN_AP

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_FUN_RET( retname, n, [ { out, out_rc, inout, inout_rc}, arg ] )
 *
 * char *retname;
 * int narg;
 * char **arg;
 */

#define ND_FUN_RET

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

INDENT;
ScanArglist (arg, 2 * narg, i++;
             sep = 0, fprintf (outfile, "*%s__p = %s;\n", arg[i], arg[i++]); INDENT;
             sep = 1, fprintf (outfile, "*%s__p = %s;\n", arg[i], arg[i++]); INDENT;
             sep = 1, i++; sep = 0, i++; sep = 0, i++;
             sep = 0, fprintf (outfile, "ND_KS_RET_OUT_RC(%s);\n", arg[i++]); INDENT;
             sep = 1, fprintf (outfile, "ND_KS_RET_INOUT_RC(%s);\n", arg[i++]); INDENT;
             sep = 1, "");
if ((strcmp (FUNDEF_NAME (INFO_FUNDEF (arg_info)), "main") == 0)) {
    fprintf (outfile, "ANALYSE_PRINT();\n");
    INDENT;
}
if (0 != strcmp (retname, ""))
    fprintf (outfile, "return(%s);", retname);

#undef ND_FUN_RET

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_CREATE_CONST_ARRAY_S( name, dim, s0,..., sn)
 *   generates a constant array
 *
 * char *name;
 * int dim;
 * char **s;
 */

#define ND_CREATE_CONST_ARRAY_S

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

{
    int i;
    for (i = 0; i < dim; i++) {
        INDENT;
        fprintf (outfile, "%s[%d]=%s;\n", name, i, s[i]);
    }
}

#undef ND_CREATE_CONST_ARRAY_S

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_CREATE_CONST_ARRAY_H( name, copyfun, dim, s0,..., sn)
 *   generates a constant array of refcounted hidden values
 *
 * char *name;
 * char *copyfun;
 * int dim;
 * char **A;
 */

#define ND_CREATE_CONST_ARRAY_H

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

{
    int i;
    for (i = 0; i < dim; i++) {
        INDENT;
        fprintf (outfile, "ND_NO_RC_MAKE_UNIQUE_HIDDEN(%s, %s[%d], %s);\n", A[i], name, i,
                 copyfun);
    }
}

#undef ND_CREATE_CONST_ARRAY_H

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_CREATE_CONST_ARRAY_A( name, length, dim, s0,..., sn)
 *                        : generates a constant array out of arrays
 *
 * char *name;
 * int length; // number of elements of the argument array (s0)
 * int dim;
 * char **s;
 */

#define ND_CREATE_CONST_ARRAY_A

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

{
    int i;
    for (i = 0; i < dim * length; i++) {
        INDENT;
        fprintf (outfile, "%s[%d]=%s[%d];\n", name, i, A[i / length], i % length);
    }
}

#undef ND_CREATE_CONST_ARRAY_A

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_KS_DECL_ARRAY( basetype, name, dim, s0,..., sn)
 *
 * declares an array
 *
 * char *basetype;      e.g. "int" for an integer array
 * char *name;
 * int dim;
 * char **s;
 */

#define ND_KS_DECL_ARRAY

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

INDENT;
fprintf (outfile, "%s *%s;\n", type, name);
INDENT;
fprintf (outfile, "int *__%s_rc;\n", name);
INDENT;
fprintf (outfile, "int __%s_sz=", name);
fprintf (outfile, "%s", s[0]);
{
    int i;
    for (i = 1; i < dim; i++)
        fprintf (outfile, "*%s", s[i]);
    fprintf (outfile, ";\n");
    INDENT;
    fprintf (outfile, "int __%s_d=%d;\n", name, dim);
    for (i = 0; i < dim; i++) {
        INDENT;
        fprintf (outfile, "int __%s_s%d=%s;\n", name, i, s[i]);
    }
}
fprintf (outfile, "\n");

#undef ND_KS_DECL_ARRAY

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_KS_DECL_GLOBAL_ARRAY( basetype, name, dim, s0,..., sn)   :
 *
 * declares an array which is a global object
 *
 * char *type, *name;
 * int dim;
 * char **s;
 */

#define ND_KS_DECL_GLOBAL_ARRAY

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

if (print_objdef_for_header_file) {
    INDENT;
    fprintf (outfile, "extern %s *%s;\n", type, name);
    INDENT;
    fprintf (outfile, "extern int *__%s_rc;\n", name);
    INDENT;
    fprintf (outfile, "extern int __%s_sz;\n", name);
    {
        int i;
        INDENT;
        fprintf (outfile, "extern int __%s_d;\n", name);
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile, "extern int __%s_s%d;\n", name, i);
        }
    }
    fprintf (outfile, "\n");
} else {
    INDENT;
    fprintf (outfile, "%s *%s;\n", type, name);
    INDENT;
    fprintf (outfile, "int *__%s_rc;\n", name);
    INDENT;
    fprintf (outfile, "int __%s_sz=", name);
    fprintf (outfile, "%s", s[0]);
    {
        int i;
        for (i = 1; i < dim; i++)
            fprintf (outfile, "*%s", s[i]);
        fprintf (outfile, ";\n");
        INDENT;
        fprintf (outfile, "int __%s_d=%d;\n", name, dim);
        for (i = 0; i < dim; i++) {
            INDENT;
            fprintf (outfile, "int __%s_s%d=%s;\n", name, i, s[i]);
        }
    }
    fprintf (outfile, "\n");
}

#undef ND_KS_DECL_GLOBAL_ARRAY

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_KD_DECL_EXTERN_ARRAY( basetype, name, dim)
 *
 * declares an array which is an imported global object
 *
 * char *basetype, *name;
 * int dim;
 */

#define ND_KD_DECL_EXTERN_ARRAY

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

INDENT;
fprintf (outfile, "extern %s *%s;\n", type, name);
INDENT;
fprintf (outfile, "extern int *__%s_rc;\n", name);
INDENT;
fprintf (outfile, "extern int __%s_sz;\n", name);
{
    int i;
    INDENT;
    fprintf (outfile, "extern int __%s_d;\n", name);
    for (i = 0; i < dim; i++) {
        INDENT;
        fprintf (outfile, "extern int __%s_s%d;\n", name, i);
    }
}
fprintf (outfile, "\n");

#undef ND_KD_DECL_EXTERN_ARRAY

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_KS_DECL_ARRAY_ARG( name, dim, s0,..., sn)   : declares an array given as arg
 *
 * char *name;
 * int dim;
 * char **s;
 */

#define ND_KS_DECL_ARRAY_ARG

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

INDENT;
fprintf (outfile, "int __%s_sz=", name);
fprintf (outfile, "%s", s[0]);
{
    int i;
    for (i = 1; i < dim; i++)
        fprintf (outfile, "*%s", s[i]);
    fprintf (outfile, ";\n");
    INDENT;
    fprintf (outfile, "int __%s_d=%d;\n", name, dim);
    for (i = 0; i < dim; i++) {
        INDENT;
        fprintf (outfile, "int __%s_s%d=%s;\n", name, i, s[i]);
    }
}
fprintf (outfile, "\n");

#undef ND_KS_DECL_ARRAY_ARG

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_KD_SET_SHAPE( name, dim, s0,..., sn)        : sets all shape components of an array
 *
 * char *name;
 * int dim;
 * char **s;
 */

#define ND_KD_SET_SHAPE

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

{
    int i;
    for (i = 0; i < dim; i++) {
        INDENT;
        fprintf (outfile, "ND_KD_A_SHAPE(%s, %d)=%s;\n", name, i, s[i]);
    }
}
fprintf (outfile, "\n");

#undef ND_KD_SET_SHAPE

#ifndef TEST_BACKEND
free (s);
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_KD_PSI_CxA_S(line, a, res, dim, v0,..., vn): selects a single element of the array
 *
 *
 * char *type, *a, *res;
 * int dim;
 * char **vi;
 */

#define ND_KD_PSI_CxA_S

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

INDENT;
if (check_boundary) {
    fprintf (outfile, "{ int idx=");
    VectToOffset (dim, AccessConst (vi, i), dim, a);
    fprintf (outfile, ";\n");
    INDENT;
    fprintf (outfile, "if((0 <= idx) && (idx < ND_A_SIZE(%s)) )\n", a);
    indent++;
    INDENT;
    fprintf (outfile, "%s=ND_A_FIELD(%s)[idx];\n", res, a);
    indent--;
    INDENT;
    fprintf (outfile, "else\n");
    indent++;
    INDENT;
    fprintf (outfile, "OUT_OF_BOUND(%d, \"psi\", ND_A_SIZE(%s), idx);\n", line, a);
    indent--;
    INDENT;
    fprintf (outfile, "}\n");
} else {
    fprintf (outfile, "%s=ND_A_FIELD(%s)[", res, a);
    VectToOffset (dim, AccessConst (vi, i), dim, a);
    fprintf (outfile, "];\n\n");
}

#undef ND_KD_PSI_CxA_S

#ifndef TEST_BACKEND
free (vi);
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_KD_PSI_VxA_S(line, a, res, dim, v ) : selects a single element of the array
 *
 * char *a, *res, *v;
 * int dim;
 */

#define ND_KD_PSI_VxA_S

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

INDENT;
if (check_boundary) {
    fprintf (outfile, "{ int idx=");
    VectToOffset (dim, AccessVect (v, i), dim, a);
    fprintf (outfile, ";\n");
    INDENT;
    fprintf (outfile, "if((0 <= idx)&&(idx < ND_A_SIZE(%s)) )\n", a);
    indent++;
    INDENT;
    fprintf (outfile, "%s=ND_A_FIELD(%s)[idx];\n", res, a);
    indent--;
    INDENT;
    fprintf (outfile, "else\n");
    indent++;
    INDENT;
    fprintf (outfile, "OUT_OF_BOUND(%d, \"psi\", ND_A_SIZE(%s), idx);\n", line, a);
    indent--;
    INDENT;
    fprintf (outfile, "}\n");
} else {
    fprintf (outfile, "%s=ND_A_FIELD(%s)[", res, a);
    VectToOffset (dim, AccessVect (v, i), dim, a);
    fprintf (outfile, "];\n\n");
}

#undef ND_KD_PSI_VxA_S

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_KD_PSI_CxA_A(line, dima, a, res, dimv, v0,..., vn): selects a sub-array
 *
 * int dima;
 * char *a, *res;
 * int dimv;
 * char **vi;
 */

#define ND_KD_PSI_CxA_A

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

INDENT;
CopyBlock (a, VectToOffset (dimv, AccessConst (vi, i), dima, a), res, line);
fprintf (outfile, "\n\n");

#undef ND_KD_PSI_CxA_A

#ifndef TEST_BACKEND
free (vi);
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_KD_PSI_VxA_A(line, dima, a, res, dimv, v )       : selects a sub-array
 *
 * int dima;
 * char *a, *res, *v;
 * int dimv;
 */

#define ND_KD_PSI_VxA_A

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

INDENT;
CopyBlock (a, VectToOffset (dimv, AccessVect (v, i), dima, a), res, line);
fprintf (outfile, "\n\n");

#undef ND_KD_PSI_VxA_A

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_KD_TAKE_CxA_A( dima, a, res, dimv, v0,..., vn):
 */

#define ND_KD_TAKE_CxA_A

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

INDENT;
TakeSeg (a, dima, fprintf (outfile, "0"), /* offset */
         dimv,                            /* dim of sizes & offsets */
         AccessConst (vi, i),             /* sizes */
         fprintf (outfile, "(ND_KD_A_SHAPE(%s, %d) - ", a, i);
         AccessConst (vi, i); fprintf (outfile, ")"), /* offsets */
                              res);

fprintf (outfile, "\n\n");

#undef ND_KD_TAKE_CxA_A

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_KD_DROP_CxA_A( dima, a, res, dimv, v0,..., vn):
 */

#define ND_KD_DROP_CxA_A

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

INDENT;
TakeSeg (a, dima, VectToOffset (dimv, AccessConst (vi, i), dima, a), /* offset */
         dimv, /* dim of sizes & offsets */
         fprintf (outfile, "ND_KD_A_SHAPE(%s, %d) - ", a, i);
         AccessConst (vi, i), /* sizes */
         AccessConst (vi, i), /* offsets */
         res);

fprintf (outfile, "\n\n");

#undef ND_KD_DROP_CxA_A

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_KD_CAT_SxAxA_A( dima, ar0, ar1, res, catdim):
 */

#define ND_KD_CAT_SxAxA_A

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

INDENT;
NewBlock (
  InitPtr (fprintf (outfile, "0"), fprintf (outfile, "0"));
  InitVecs (1, 2, "__isrc", fprintf (outfile, "0"));
  InitVecs (0, 2, "__i", fprintf (outfile, "0"));
  InitVecs (0, 2, "__bl",
            {
                int j;
                for (j = catdim; j < dima; j++)
                    fprintf (outfile, "ND_KD_A_SHAPE(%s, %d)*", ar[i], j);
                fprintf (outfile, "1");
            }),
  FillRes (res, INDENT; fprintf (outfile, "for(__i0=0; __i0<__bl0; __i0++)\n"); indent++;
           INDENT;
           fprintf (outfile, "ND_A_FIELD(%s)[__idest++] = ND_A_FIELD(%s)[__isrc++];\n",
                    res, ar[0]);
           indent--; INDENT; fprintf (outfile, "for(__i1=0; __i1<__bl1; __i1++)\n");
           indent++; INDENT;
           fprintf (outfile, "ND_A_FIELD(%s)[__idest++] = ND_A_FIELD(%s)[__isrc1++];\n",
                    res, ar[1]);
           indent--; INDENT));
fprintf (outfile, "\n\n");

#undef ND_KD_CAT_SxAxA_A

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_KD_ROT_CxSxA_A( rotdim, numstr, dima, a, res):
 */

#define ND_KD_ROT_CxSxA_A

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

INDENT;
NewBlock (
  InitVecs (0, 1, "__shift",
            {
                int j;
                for (j = rotdim + 1; j < dima; j++)
                    fprintf (outfile, "ND_KD_A_SHAPE(%s, %d)*", a, j);
                fprintf (outfile, "(%s<0 ? ", numstr[0]);
                fprintf (outfile,
                         "ND_KD_A_SHAPE(%s, %d)+ (%s %% ND_KD_A_SHAPE(%s, %d)) : ", a,
                         rotdim, numstr[0], a, rotdim);
                fprintf (outfile, "%s %% ND_KD_A_SHAPE(%s, %d))", numstr[0], a, rotdim);
            });
  InitVecs (0, 1, "__bl",
            {
                int j;
                for (j = rotdim + 1; j < dima; j++)
                    fprintf (outfile, "ND_KD_A_SHAPE(%s, %d)*", a, j);
                fprintf (outfile, "ND_KD_A_SHAPE(%s, %d)", a, rotdim);
            });
  InitVecs (0, 1, "__i", fprintf (outfile, "0"));
  InitPtr (fprintf (outfile, "-__shift0"), fprintf (outfile, "0")),
  FillRes (
    res, INDENT; fprintf (outfile, "__isrc+=__bl0;\n"); INDENT;
    fprintf (outfile, "for(__i0=0; __i0<__shift0; __i0++)\n"); indent++; INDENT;
    fprintf (outfile, "ND_A_FIELD(%s)[__idest++] = ND_A_FIELD(%s)[__isrc++];\n", res, a);
    indent--; INDENT; fprintf (outfile, "__isrc-=__bl0;\n"); INDENT;
    fprintf (outfile, "for(; __i0<__bl0; __i0++)\n"); indent++; INDENT;
    fprintf (outfile, "ND_A_FIELD(%s)[__idest++] = ND_A_FIELD(%s)[__isrc++];\n", res, a);
    indent--));
fprintf (outfile, "\n\n");

#undef ND_KD_ROT_CxSxA_A

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_PRF_MODARRAY_AxCxS_CHECK_REUSE( res_type, dimres, res, old, value, dimv,
 *                                    v0,..., vn):
 */

#define ND_PRF_MODARRAY_AxCxS_CHECK_REUSE

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

fprintf (outfile, " ND_CHECK_REUSE_ARRAY(%s,%s)\n", old, res);
INDENT;
fprintf (outfile, "{ int __i;\n");
fprintf (outfile, "  ND_ALLOC_ARRAY(%s, %s, 0);\n", res_type, res);
INDENT;
fprintf (outfile, "  for(__i=0; __i<ND_A_SIZE(%s); __i++)", res);
fprintf (outfile, " ND_A_FIELD(%s)[__i]=ND_A_FIELD(%s)[__i];\n", res, old);
INDENT;
fprintf (outfile, "}\n");
INDENT;
if (check_boundary) {
    fprintf (outfile, "{  int __idx=");
    VectToOffset (dimv, AccessConst (vi, i), dimres, res);
    fprintf (outfile, ";\n");
    indent++;
    INDENT;
    fprintf (outfile, "if( (ND_A_SIZE(%s) > __idx) && (0 <= __idx))\n", res);
    indent++;
    INDENT;
}
fprintf (outfile, "ND_A_FIELD(%s)[", res);
VectToOffset (dimv, AccessConst (vi, i), dimres, res);
fprintf (outfile, "]=%s;\n", value[0]);
INDENT;
if (check_boundary) {
    fprintf (outfile, "else\n");
    indent++;
    INDENT;
    fprintf (outfile, "OUT_OF_BOUND(%d, \"modarray\", ND_A_SIZE(%s), __idx);\n", line,
             res);
    indent -= 2;
    INDENT;
    fprintf (outfile, "}\n");
}
fprintf (outfile, "\n\n");

#undef ND_PRF_MODARRAY_AxCxS_CHECK_REUSE

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_PRF_MODARRAY_AxCxS( res_type, dimres, res, old, value, dimv,
 *                                    v0,..., vn):
 */

#define ND_PRF_MODARRAY_AxCxS

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

fprintf (outfile, "{ int __i;\n");
fprintf (outfile, "  ND_ALLOC_ARRAY(%s, %s, 0);\n", res_type, res);
INDENT;
fprintf (outfile, "  for(__i=0; __i<ND_A_SIZE(%s); __i++)", res);
fprintf (outfile, " ND_A_FIELD(%s)[__i]=ND_A_FIELD(%s)[__i];\n", res, old);
INDENT;
fprintf (outfile, "}\n");
INDENT;
if (check_boundary) {
    fprintf (outfile, "{  int __idx=");
    VectToOffset (dimv, AccessConst (vi, i), dimres, res);
    fprintf (outfile, ";\n");
    indent++;
    INDENT;
    fprintf (outfile, "if( (ND_A_SIZE(%s) > __idx) && (0 <= __idx))\n", res);
    indent++;
    INDENT;
}
fprintf (outfile, "ND_A_FIELD(%s)[", res);
VectToOffset (dimv, AccessConst (vi, i), dimres, res);
fprintf (outfile, "]=%s;\n", value[0]);
INDENT;
if (check_boundary) {
    fprintf (outfile, "else\n");
    indent++;
    INDENT;
    fprintf (outfile, "OUT_OF_BOUND(%d, \"modarray\", ND_A_SIZE(%s), __idx);\n", line,
             res);
    indent -= 2;
    INDENT;
    fprintf (outfile, "}\n");
}
fprintf (outfile, "\n\n");

#undef ND_PRF_MODARRAY_AxCxS

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_PRF_MODARRAY_AxVxS_CHECK_REUSE( res_type, dimres, res, old, value, dim, v):
 */

#define ND_PRF_MODARRAY_AxVxS_CHECK_REUSE

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

fprintf (outfile, " ND_CHECK_REUSE_ARRAY(%s,%s)\n", old, res);
INDENT;
fprintf (outfile, "{ int __i;\n");
fprintf (outfile, "  ND_ALLOC_ARRAY(%s, %s, 0);\n", res_type, res);
INDENT;
fprintf (outfile, "  for(__i=0; __i<ND_A_SIZE(%s); __i++)", res);
fprintf (outfile, " ND_A_FIELD(%s)[__i]=ND_A_FIELD(%s)[__i];\n", res, old);
INDENT;
fprintf (outfile, "}\n");
INDENT;
if (check_boundary) {
    fprintf (outfile, "{  int __idx=");
    VectToOffset (dim, AccessVect (v, i), dimres, res);
    fprintf (outfile, ";\n");
    indent++;
    INDENT;
    fprintf (outfile, "if( (ND_A_SIZE(%s) > __idx) && (0 <= __idx))\n", res);
    indent++;
    INDENT;
}
fprintf (outfile, "ND_A_FIELD(%s)[", res);
VectToOffset (dim, AccessVect (v, i), dimres, res);
fprintf (outfile, "]=%s;\n", value[0]);
INDENT;
if (check_boundary) {
    fprintf (outfile, "else\n");
    indent++;
    INDENT;
    fprintf (outfile, "OUT_OF_BOUND(%d, \"modarray\", ND_A_SIZE(%s), __idx);\n", line,
             res);
    indent -= 2;
    INDENT;
    fprintf (outfile, "}\n");
}
fprintf (outfile, "\n\n");

#undef ND_PRF_MODARRAY_AxVxS_CHECK_REUSE

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_PRF_MODARRAY_AxVxS( res_type, dimres, res, old, value, dim, v):
 */

#define ND_PRF_MODARRAY_AxVxS

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

fprintf (outfile, "{ int __i;\n");
fprintf (outfile, "  ND_ALLOC_ARRAY(%s, %s, 0);\n", res_type, res);
INDENT;
fprintf (outfile, "  for(__i=0; __i<ND_A_SIZE(%s); __i++)", res);
fprintf (outfile, " ND_A_FIELD(%s)[__i]=ND_A_FIELD(%s)[__i];\n", res, old);
INDENT;
fprintf (outfile, "}\n");
INDENT;
if (check_boundary) {
    fprintf (outfile, "{  int __idx=");
    VectToOffset (dim, AccessVect (v, i), dimres, res);
    fprintf (outfile, ";\n");
    indent++;
    INDENT;
    fprintf (outfile, "if( (ND_A_SIZE(%s) > __idx) && (0 <= __idx))\n", res);
    indent++;
    INDENT;
}
fprintf (outfile, "ND_A_FIELD(%s)[", res);
VectToOffset (dim, AccessVect (v, i), dimres, res);
fprintf (outfile, "]=%s;\n", value[0]);
INDENT;
if (check_boundary) {
    fprintf (outfile, "else\n");
    indent++;
    INDENT;
    fprintf (outfile, "OUT_OF_BOUND(%d, \"modarray\", ND_A_SIZE(%s), __idx);\n", line,
             res);
    indent -= 2;
    INDENT;
    fprintf (outfile, "}\n");
}
fprintf (outfile, "\n\n");

#undef ND_PRF_MODARRAY_AxVxS

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_PRF_MODARRAY_AxCxA( res_type, dimres, res, old, a_value, dimv,
 *                                    v0,..., vn):
 */

#define ND_PRF_MODARRAY_AxCxA

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

fprintf (outfile, "{ int __i, __j;\n");
fprintf (outfile, "  int __idx=");
VectToOffset (dimv, AccessConst (vi, i), dimres, res);
fprintf (outfile, ";\n");
INDENT;
if (check_boundary) {
    fprintf (outfile,
             "if( (ND_A_SIZE(%s) > (__idx+ND_A_SIZE(%s)))"
             "&& (0 <= (__idx+ND_A_SIZE(%s))) ){\n",
             res, value, value);
    indent++;
    INDENT;
}
fprintf (outfile, "  ND_ALLOC_ARRAY(%s, %s, 0);\n", res_type, res);
INDENT;
fprintf (outfile, "  for(__i=0; __i<__idx-1; __i++)");
fprintf (outfile, " ND_A_FIELD(%s)[__i]=ND_A_FIELD(%s)[__i];\n", res, old);
INDENT;
fprintf (outfile, "  for(__i=__idx,__j=0; __j<ND_A_SIZE(%s); __i++,__j++)", value);
fprintf (outfile, " ND_A_FIELD(%s)[__i]=ND_A_FIELD(%s)[__j];\n", res, value);
INDENT;
fprintf (outfile, " for(; __i<ND_A_SIZE(%s); __i++)", res);
fprintf (outfile, " ND_A_FIELD(%s)[__i]=ND_A_FIELD(%s)[__i];\n", res, old);

if (check_boundary) {
    indent--;
    INDENT;
    fprintf (outfile, "}\n");
    INDENT;
    fprintf (outfile, "else\n");
    indent++;
    INDENT;
    fprintf (outfile, "OUT_OF_BOUND(%d, \"modarray\", ND_A_SIZE(%s), __idx);\n", line,
             res);
    indent -= 2;
    INDENT;
    fprintf (outfile, "}\n");
} else {
    INDENT;
    fprintf (outfile, "}\n");
}
fprintf (outfile, "\n\n");

#undef ND_PRF_MODARRAY_AxCxA

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_PRF_MODARRAY_AxCxA_CHECK_REUSE( res_type, dimres, res, old, a_value, dimv,
 *                                    v0,..., vn):
 */

#define ND_PRF_MODARRAY_AxCxA_CHECK_REUSE

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"
fprintf (outfile, "{  int __i, __j;\n");
INDENT;
fprintf (outfile, "  int __idx=");
VectToOffset (dimv, AccessConst (vi, i), dimres, res);
fprintf (outfile, ";\n");
INDENT;
if (check_boundary) {
    fprintf (outfile,
             "if( (ND_A_SIZE(%s) > (__idx+ND_A_SIZE(%s)))"
             "&& (0 <= (__idx+ND_A_SIZE(%s))) ){\n",
             res, value, value);
    indent++;
    INDENT;
}
fprintf (outfile, "if(ND_A_RC(%s)==1){\n", old);
indent++;
INDENT;
fprintf (outfile, "ND_KS_ASSIGN_ARRAY(%s,%s)\n", old, res);
INDENT;
fprintf (outfile, "for(__i=__idx,__j=0; __j<ND_A_SIZE(%s); __i++,__j++)\n", value);
indent++;
INDENT;
fprintf (outfile, " ND_A_FIELD(%s)[__i]=ND_A_FIELD(%s)[__j];\n", res, value);
indent -= 2;
INDENT;
fprintf (outfile, "}\n");
INDENT;
fprintf (outfile, "else{\n");
indent++;
INDENT;
fprintf (outfile, "ND_ALLOC_ARRAY(%s, %s, 0);\n", res_type, res);
INDENT;
fprintf (outfile, "for(__i=0; __i<__idx-1; __i++)\n");
indent++;
INDENT;
fprintf (outfile, " ND_A_FIELD(%s)[__i]=ND_A_FIELD(%s)[__i];\n", res, old);
indent--;
INDENT;
fprintf (outfile, "for(__i=__idx,__j=0; __j<ND_A_SIZE(%s); __i++,__j++)\n", value);
indent++;
INDENT;
fprintf (outfile, "ND_A_FIELD(%s)[__i]=ND_A_FIELD(%s)[__j];\n", res, value);
indent--;
INDENT;
fprintf (outfile, "for(; __i<ND_A_SIZE(%s); __i++)\n", res);
indent++;
INDENT;
fprintf (outfile, "ND_A_FIELD(%s)[__i]=ND_A_FIELD(%s)[__i];\n", res, old);
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
    fprintf (outfile, "OUT_OF_BOUND(%d, \"modarray\", ND_A_SIZE(%s), __idx);\n", line,
             res);
    indent -= 2;
    INDENT;
}
fprintf (outfile, "}\n");
fprintf (outfile, "\n\n");

#undef ND_PRF_MODARRAY_AxCxA_CHECK_REUSE

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_BEGIN_GENARRAY( res, dimres, from, to, idx, idxlen)
 */

#define ND_BEGIN_GENARRAY

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

BeginWith (res, dimres, from, to, idx, idxlen, fprintf (outfile, "0"), "genarray");

#ifdef TEST_BACKEND
indent -= idxlen + 1;
#endif /* TEST_BACKEND */

#undef ND_BEGIN_GENARRAY

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_BEGIN_MODARRAY( res, dimres, a, from, to, idx, idxlen)
 */

#define ND_BEGIN_MODARRAY

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

BeginWith (res, dimres, from, to, idx, idxlen,
           fprintf (outfile, "ND_A_FIELD(%s)[%s__destptr]", a, res), "modarray");

#ifdef TEST_BACKEND
indent -= idxlen + 1;
#endif /* TEST_BACKEND */

#undef ND_BEGIN_MODARRAY

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * OLD_FOLD: ND_BEGIN_FOLDPRF( res, dimres, neutral, from, to, idx, idxlen)
 *
 * ND_BEGIN_FOLDPRF( res, dimres, from, to, idx, idxlen, n_neutral, neutral)
 */

#define ND_BEGIN_FOLDPRF

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

#ifdef OLD_FOLD
BeginFoldWith (res, dimres, neutral, from, to, idx, idxlen, 1);
#else
BeginFoldWith (res, dimres, from, to, idx, idxlen, n_neutral, neutral);
#endif

#ifdef TEST_BACKEND
indent -= idxlen + 1;
#endif /* TEST_BACKEND */

#undef ND_BEGIN_FOLDPRF

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_BEGIN_FOLDFUN( res, dimres, neutral, from, to, idx, idxlen)
 */

#define ND_BEGIN_FOLDFUN

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

#ifdef OLD_FOLF
BeginFoldWith (res, dimres, neutral, from, to, idx, idxlen, 0);
#else
BeginFoldWith (res, dimres, from, to, idx, idxlen, n_neutral, neutral);
#endif

#ifdef TEST_BACKEND
indent -= idxlen + 1;
#endif /* TEST_BACKEND */

#undef ND_BEGIN_FOLDFUN

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_END_GENARRAY_S( res, dimres, valstr):
 */

#define ND_END_GENARRAY_S

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

#ifdef TEST_BACKEND
indent += dimres + 1;
#endif /* TEST_BACKEND */

RetWithScal (res, valstr);
EndWith (res, dimres, dimres, fprintf (outfile, "0"), "genarray");

#undef ND_END_GENARRAY_S

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_END_GENARRAY_A( res, dimres, reta, idxlen):
 */

#define ND_END_GENARRAY_A

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

#ifdef TEST_BACKEND
indent += idxlen + 1;
#endif /* TEST_BACKEND */

RetWithArray (res, reta);
EndWith (res, dimres, idxlen, fprintf (outfile, "0"), "genarray");

#undef ND_END_GENARRAY_A

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_END_MODARRAY_S( res, dimres, a, valstr):
 */

#define ND_END_MODARRAY_S

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

#ifdef TEST_BACKEND
indent += dimres + 1;
#endif /* TEST_BACKEND */

RetWithScal (res, valstr);
EndWith (res, dimres, dimres, fprintf (outfile, "ND_A_FIELD(%s)[%s__destptr]", a, res),
         "modarray");

#undef ND_END_MODARRAY_S

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_END_FOLD( idxlen )
 */

#define ND_END_FOLD

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

#ifdef TEST_BACKEND
indent += idxlen + 1;
#endif /* TEST_BACKEND */

EndFoldWith (idxlen);

#undef ND_END_FOLD

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*
 * ND_KS_VECT2OFFSET( name, dim, s )
 *      char * name;
 *      int    dim;
 *      int    dims;
 *      char **s;
 */

#define ND_KS_VECT2OFFSET

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

#ifdef TEST_BACKEND
indent += idxlen + 1;
#endif /* TEST_BACKEND */

fprintf (outfile, "__%s", name);
{
    int i;
    for (i = 0; i < dims; i++) {
        fprintf (outfile, "_%s", s[i]);
    }
    fprintf (outfile, "= ");
    VectToOffset2 (dim, AccessVect (name, i), dims, AccessConst (s, i));
    fprintf (outfile, ";\n");
}

#undef ND_KS_VECT2OFFSET

#ifndef TEST_BACKEND
DBUG_VOID_RETURN;
}
#endif /* no TEST_BACKEND */

/*      !!!!!! NOTE !!!!!!!!!
 *   ND_END_MODARRAY_A should be the last N_icm implementation
 */

/*
 * ND_END_MODARRAY_A( res, dimres, a, reta, idxlen):
 */

#define ND_END_MODARRAY_A

#ifndef TEST_BACKEND
#include "icm_decl.c"
#include "icm_args.c"
#endif /* no TEST_BACKEND */

#include "icm_comment.c"
#include "icm_trace.c"

#ifdef TEST_BACKEND
indent += idxlen + 1;
#endif /* TEST_BACKEND */

RetWithArray (res, reta);
EndWith (res, dimres, idxlen, fprintf (outfile, "ND_A_FIELD(%s)[%s__destptr]", a, res),
         "modarray");

#undef ND_END_MODARRAY_A

#ifdef TEST_BACKEND
return (0);
}
#else  /* TEST_BACKEND */
DBUG_VOID_RETURN;
}
#endif /* TEST_BACKEND */
