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
 *   void ICMCompileND_FUN_DEF_BEGIN( char *name, char *rettype_NT,
 *                                    int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WE_FUN_DEF_BEGIN( name, rettype_NT, vararg_cnt, [ TAG, basetype, arg_NT ]* )
 *
 *   where TAG is element in { in, in_..., out, out_..., inout, inout_... }.
 *
 ******************************************************************************/
void
ICMCompileWE_FUN_DEF_BEGIN (char *name, char *rettype_NT, int vararg_cnt, char **vararg)
{
    DBUG_ENTER ("ICMCompileWE_FUN_DEF_BEGIN");

#define ND_FUN_DEF_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_FUN_DEF_BEGIN

    INDENT;

    fprintf (global.outfile, "SAC_ND_DEF_FUN_BEGIN2(");

    fprintf (global.outfile, "%s, ", name);

    if (rettype_NT[0] != '\0') {
        fprintf (global.outfile, "SAC_ND_TYPE_NT( %s), ", rettype_NT);
    } else {
        fprintf (global.outfile, "void, ");
    }

    ScanArglist (vararg_cnt, 3, ",", ,
                 fprintf (global.outfile, " SAC_ND_PARAM_%s( %s, %s)", vararg[i],
                          vararg[i + 2], vararg[i + 1]));
    fprintf (global.outfile, ")\n");

    INDENT;
    fprintf (global.outfile, "{\n");
    global.indent++;

    INDENT;
    fprintf (global.outfile, "SAC_HM_DEFINE_THREAD_STATUS( SAC_HM_single_threaded)\n");

    /*
     * Add a macro do declare the function pointer needed by the wrapper entry
     * function.
     */
    INDENT;
    fprintf (global.outfile, "SAC_WE_DECL_FN_POINTER(");

    if (rettype_NT[0] != '\0') {
        fprintf (global.outfile, "SAC_ND_TYPE_NT( %s), ", rettype_NT);
    } else {
        fprintf (global.outfile, "void, ");
    }

    ScanArglist (vararg_cnt, 3, ",", ,
                 fprintf (global.outfile, " SAC_ND_PARAM_%s( %s, %s)", vararg[i],
                          vararg[i + 2], vararg[i + 1]));
    fprintf (global.outfile, ");\n");

    DBUG_VOID_RETURN;
}

/** <!--*******************************************************************-->
 *
 * @fn ICMCompileWE_FUN_AP( char *name, char *retname, int vararg_cnt,
 *                          char **vararg)
 *
 * @brief Implements the ICM for the function application of an indirect wrapper
 * function. Such a function application can only occur in a wrapper entry
 * function.
 *
 ****************************************************************************/
void
ICMCompileWE_FUN_AP (char *name, char *retname, int vararg_cnt, char **vararg)
{
    DBUG_ENTER ("ICMCompileND_FUN_AP");

#define ND_FUN_AP
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_FUN_AP

    /*
     * Encode the base type information.
     */
    char types[256] = "";
    char current[10] = "";

    int i = 0;
    for (; i < vararg_cnt * 3; i += 3) {
        if (STReq (vararg[i], "in")) {
            sprintf (current, "%s-", vararg[i + 1]);
            sprintf (types, "%s", STRcat (types, current));
        }
    }

    INDENT;
    if (!STReq (retname, "")) {
        fprintf (global.outfile, "%s = ", retname);
        fprintf (global.outfile, "%s(", name);
    } else {
        fprintf (global.outfile, "SAC_WE_FUNAP2(");
        fprintf (global.outfile, "%s, %s, ", types, name);
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

/** <!--*******************************************************************-->
 *
 * @fn ICMCompileWE_MODFUN_INFO( char *name, char *modname)
 *
 * @brief Implements the ICM that passes the module name and the function name
 * to the wrapper entry function.
 *
 ****************************************************************************/
void
ICMCompileWE_MODFUN_INFO (char *name, char *modname)
{
    DBUG_ENTER ("ICMCompileWE_MODFUN_INFO");

    fprintf (global.outfile, "SAC_WE_DECL_REG_FLAG\n");

    INDENT;
    fprintf (global.outfile, "SAC_WE_DECL_REG_OBJ\n");

    INDENT;
    fprintf (global.outfile, "SAC_WE_DECL_MOD( %s)\n", modname);

    INDENT;
    fprintf (global.outfile, "SAC_WE_DECL_FUN( %s)\n", name);

    INDENT;
    fprintf (global.outfile, "SAC_WE_DECL_SHAPE_ARRAY\n");

    DBUG_VOID_RETURN;
}

/** <!--*******************************************************************-->
 *
 * @fn ICMCompileWE_SHAPE_ENCODE( int arg_cnt, char **arg)
 *
 * @brief Implements the ICM that prints the necessary code for the wrapper
 * entry function to encode its arguments shape information at runtime.
 *
 ****************************************************************************/
void
ICMCompileWE_SHAPE_ENCODE (int arg_cnt, char **arg)
{
    DBUG_ENTER ("ICMCompileWE_SHAPE_ENCODE");

    fprintf (global.outfile, "SAC_WE_DECL_I_J\n");

    INDENT;
    fprintf (global.outfile, "SAC_WE_CALC_SIZE( %d", arg_cnt);

    int i = 0;
    for (; i < arg_cnt; i++) {
        fprintf (global.outfile, " + SAC_WE_GET_DIM( %s)", arg[i]);
    }

    fprintf (global.outfile, ")\n");

    INDENT;
    fprintf (global.outfile, "SAC_WE_ALLOC_SHAPE_ARRAY\n");

    INDENT;
    fprintf (global.outfile, "SAC_WE_SET_NUM_ARGS( %d)\n", arg_cnt);

    i = 0;
    for (; i < arg_cnt; i++) {
        INDENT;
        fprintf (global.outfile, "SAC_WE_GET_SHAPE( %s)\n", arg[i]);
    }

    DBUG_VOID_RETURN;
}

/** <!--*******************************************************************-->
 *
 * @fn ICMCompileWE_NO_SHAPE_ENCODE( int arg_cnt)
 *
 * @brief Dummy function as a solution to a bug that arises when for some reason
 * the arguments to the SHAPE_ENCODE PRF become NULL.
 *
 * @TODO: Fix the bug that necessitates this function!
 *
 ****************************************************************************/
void
ICMCompileWE_NO_SHAPE_ENCODE (int arg_cnt)
{
    DBUG_ENTER ("ICMCompileWE_NO_SHAPE_ENCODE");

    fprintf (global.outfile, "/* No shape information available!!! */");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWE_FUN_DEF_END( char *name, char *rettype_NT,
 *                                  int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WE_FUN_DEF_END( name, rettype, vararg_cnt, [ TAG, type, param_NT ]* )
 *
 *   This ICM implements end of a standard function. The first parameter
 *   specifies the name of this function.
 *   TAG may be from the set { in, out, inout }.
 *
 ******************************************************************************/

void
ICMCompileWE_FUN_DEF_END (char *name, char *rettype_NT, int vararg_cnt, char **vararg)
{
    DBUG_ENTER ("ICMCompileWE_FUN_DEF_END");

#define ND_FUN_DEF_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_FUN_DEF_END

    global.indent--;
    INDENT;
    fprintf (global.outfile, "}\n");
    INDENT;
    fprintf (global.outfile, "SAC_WE_FUN_DEF_END2();\n");

    DBUG_VOID_RETURN;
}
