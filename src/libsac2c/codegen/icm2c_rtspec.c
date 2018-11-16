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
#include "str_buffer.h"
#include "string.h"
#include "rtspec_modes.h"
#include "new_types.h"
#include "types.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_FUN_DEF_BEGIN( char *name, char *rettype_NT,
 *                                    unsigned int vararg_cnt, char **vararg)
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
ICMCompileWE_FUN_DEF_BEGIN (char *name, char *rettype_NT, unsigned int vararg_cnt, char **vararg)
{
    DBUG_ENTER ();

#define WE_FUN_DEF_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef WE_FUN_DEF_BEGIN

    INDENT;

    fprintf (global.outfile, "SAC_ND_DEF_FUN_BEGIN2(");

    fprintf (global.outfile, "%s, ", name);

    if (rettype_NT[0] != '\0') {
        fprintf (global.outfile, "SAC_ND_TYPE_NT( %s), ", rettype_NT);
    } else {
        fprintf (global.outfile, "void, ");
    }

    if (vararg_cnt > 0) {
        SCAN_ARG_LIST (vararg_cnt, 3, ",", ,
                       fprintf (global.outfile, " SAC_ND_PARAM_%s( %s, %s)", vararg[i],
                                vararg[i + 2], vararg[i + 1]));
    } else {
        fprintf (global.outfile, "void");
    }
    fprintf (global.outfile, ")\n");

    INDENT;
    fprintf (global.outfile, "{\n");
    global.indent++;

    INDENT;
    fprintf (global.outfile, "SAC_HM_DEFINE_THREAD_STATUS( SAC_HM_single_threaded)\n");

    DBUG_RETURN ();
}

/** <!--*******************************************************************-->
 *
 * @fn ICMCompileRTSPEC_FUN_AP( char *modname, char *name, char *srcname, char *uuid
 *                              char *rettype_NT, char *retname, unsigned int vararg_cnt,
 *                              char **vararg)
 *
 * @brief Implements the ICM for the function application with runtime
 *        specialization wrapper code.
 *
 ****************************************************************************/
void
ICMCompileRTSPEC_FUN_AP (char *modname, char *name, char *srcname, char *uuid,
                         char *rettype_NT, char *retname, unsigned int vararg_cnt, char **vararg)
{
    DBUG_ENTER ();

#define RTSPEC_FUN_AP
#include "icm_comment.c"
#include "icm_trace.c"
#undef RTSPEC_FUN_AP

    if (!STReq (retname, "")) {
        indout ("%s = %s(", retname, name);
    } else {

        indout ("SAC_RTSPEC_BLOCK_START()\n");

        indout ("SAC_WE_DECL_REG_FLAG()\n");

        indout ("SAC_WE_DECL_REG_OBJ()\n");

        indout ("SAC_WE_DECL_MOD( \"%s\")\n", modname);

        indout ("SAC_WE_DECL_FUN( \"%s\")\n", srcname);

        indout ("SAC_RTSPEC_DECL_UUID( \"%s\")\n", uuid);

        indout ("SAC_WE_DECL_SHAPE_ARRAY()\n");

        indout ("SAC_WE_DECL_I_J()\n");

        /*
         * Encode the base type information.
         */
        size_t type_string_size = 1;

        unsigned int i = 0;
        int arg_cnt = 0;

        i = 0;
        for (; i < vararg_cnt * 5; i += 5) {
            if (STReq (vararg[i], "in")) {
                type_string_size += STRlen (vararg[i + 1]) + 1;
                arg_cnt++;
            }
        }

        indout ("SAC_WE_CALC_SIZE( %d", arg_cnt);

        char *types = (char *)malloc (type_string_size * sizeof (char));

        types[0] = '\0';

        i = 0;
        for (; i < vararg_cnt * 5; i += 5) {
            if (STReq (vararg[i], "in")) {
                if (types[0] != 0) {
                    strcat (types, "-");
                }
                strcat (types, vararg[i + 1]);
                out (" + SAC_WE_GET_DIM( %s)", vararg[i + 4]);
            }
        }

        out (")\n");

        indout ("SAC_WE_ALLOC_SHAPE_ARRAY()\n");

        indout ("SAC_WE_SET_NUM_ARGS( %d)\n", arg_cnt);

        shape_class_t shape_class;
        int dim;
        int j = 0;

        i = 0;
        for (; i < vararg_cnt * 5; i += 5) {
            if (STReq (vararg[i], "in")) {
                // FIXME: check the result of strol for validity!
                shape_class = (shape_class_t)(int)strtol (vararg[i + 3], NULL, 10);
                if (shape_class == C_akd || shape_class == C_aks) {
                    dim = (int)strtol (vararg[i + 2], NULL, 10);
                    indout ("SAC_RTSPEC_GET_DIM_FOR_SHAPE( %s)\n", vararg[i + 4]);
                    for (j = 0; j < dim; j++) {
                        indout ("SAC_RTSPEC_GET_SHAPE_KNOWN_DIM( %s, %i)\n",
                                vararg[i + 4], j);
                    }
                } else {
                    indout ("SAC_RTSPEC_GET_SHAPE_UNKNOWN_DIM( %s)\n", vararg[i + 4]);
                }
            }
        }

        out ("#if defined (__clang__) || ((__GNUC__)  && ((__GNUC__ > 4) || ((__GNUC__ "
             "== 4) && (__GNUC_MINOR__ > 5))))\n");
        out ("#pragma GCC diagnostic push\n");
        out ("#pragma GCC diagnostic ignored \"-Wpedantic\"\n");
        out ("#endif\n");
        indout ("SAC_RTSPEC_PTR_CAST( %s, %s, ", name, types);

        if (rettype_NT[0] != '\0') {
            out ("SAC_ND_TYPE_NT( %s), ", rettype_NT);
        } else {
            out ("void, ");
        }

        SCAN_ARG_LIST (vararg_cnt, 5, ",", ,
                       out (" SAC_ND_PARAM_%s( %s, %s)", vararg[i], vararg[i + 4],
                            vararg[i + 1]));

        out (")(");
    }

    SCAN_ARG_LIST (vararg_cnt, 5, ",", ,
                   out (" SAC_ND_ARG_%s( %s, %s)", vararg[i], vararg[i + 4],
                        vararg[i + 1]));

    out (");\n");

    if (STReq (retname, "")) {
        out ("#if defined (__clang__) || ((__GNUC__)  && ((__GNUC__ > 4) || ((__GNUC__ "
             "== 4) && (__GNUC_MINOR__ > 5))))\n");
        out ("#pragma GCC diagnostic pop\n");
        out ("#endif\n");
        indout ("SAC_RTSPEC_BLOCK_END()\n");
    }

    DBUG_RETURN ();
}

/** <!--*******************************************************************-->
 *
 * @fn ICMCompileWE_FUN_AP( char *name, char *retname, unsigned int vararg_cnt,
 *                          char **vararg)
 *
 * @brief Implements the ICM for the function application of an indirect wrapper
 * function. Such a function application can only occur in a wrapper entry
 * function.
 *
 ****************************************************************************/
void
ICMCompileWE_FUN_AP (char *name, char *rettype_NT, char *retname, unsigned int vararg_cnt,
                     char **vararg)
{
    DBUG_ENTER ();

#define WE_FUN_AP
#include "icm_comment.c"
#include "icm_trace.c"
#undef WE_FUN_AP

    if (!STReq (retname, "")) {
        INDENT;
        fprintf (global.outfile, "%s = ", retname);
        fprintf (global.outfile, "%s(", name);
    } else {

        /*
         * Encode the base type information.
         */
        size_t type_string_size = 1;

        unsigned int i = 0;
        for (; i < vararg_cnt * 3; i += 3) {
            if (STReq (vararg[i], "in")) {
                type_string_size += STRlen (vararg[i + 1]) + 1;
            }
        }

        char *types = (char *)malloc (type_string_size * sizeof (char));

        types[0] = '\0';

        i = 0;
        for (; i < vararg_cnt * 3; i += 3) {
            if (STReq (vararg[i], "in")) {
                strcat (types, vararg[i + 1]);
                strcat (types, "-");
            }
        }

        fprintf (global.outfile, "#if defined (__clang__) || ((__GNUC__)  && ((__GNUC__ "
                                 "> 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 5))))\n");
        fprintf (global.outfile, "#pragma GCC diagnostic push\n");
        fprintf (global.outfile, "#pragma GCC diagnostic ignored \"-Wpedantic\"\n");
        fprintf (global.outfile, "#endif\n");
        INDENT;
        fprintf (global.outfile, "SAC_WE_FUNAP2(%s, %s)\n", types, name);
        INDENT;
        fprintf (global.outfile, "SAC_WE_PTR_CAST( ");

        if (rettype_NT[0] != '\0') {
            fprintf (global.outfile, "SAC_ND_TYPE_NT( %s), ", rettype_NT);
        } else {
            fprintf (global.outfile, "void, ");
        }

        SCAN_ARG_LIST (vararg_cnt, 3, ",", ,
                       fprintf (global.outfile, " SAC_ND_PARAM_%s( %s, %s)", vararg[i],
                                vararg[i + 2], vararg[i + 1]));

        fprintf (global.outfile, ")(");
    }

    SCAN_ARG_LIST (vararg_cnt, 3, ",", ,
                   fprintf (global.outfile, " SAC_ND_ARG_%s( %s, %s)", vararg[i],
                            vararg[i + 2], vararg[i + 1]));

    fprintf (global.outfile, ");\n");

    fprintf (global.outfile, "#if defined (__clang__) || ((__GNUC__)  && ((__GNUC__ > 4) "
                             "|| ((__GNUC__ == 4) && (__GNUC_MINOR__ > 5))))\n");
    fprintf (global.outfile, "#pragma GCC diagnostic pop\n");
    fprintf (global.outfile, "#endif\n");

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (global.outfile, "SAC_WE_DECL_REG_FLAG()\n");

    INDENT;
    fprintf (global.outfile, "SAC_WE_DECL_REG_OBJ()\n");

    INDENT;
    fprintf (global.outfile, "SAC_WE_DECL_MOD( %s)\n", modname);

    INDENT;
    fprintf (global.outfile, "SAC_WE_DECL_FUN( %s)\n", name);

    INDENT;
    fprintf (global.outfile, "SAC_WE_DECL_SHAPE_ARRAY()\n");

    DBUG_RETURN ();
}

/** <!--*******************************************************************-->
 *
 * @fn ICMCompileWE_SHAPE_ENCODE( unsigned int arg_cnt, char **arg)
 *
 * @brief Implements the ICM that prints the necessary code for the wrapper
 * entry function to encode its arguments shape information at runtime.
 *
 ****************************************************************************/
void
ICMCompileWE_SHAPE_ENCODE (unsigned int arg_cnt, char **arg)
{
    DBUG_ENTER ();

    fprintf (global.outfile, "SAC_WE_DECL_I_J()\n");

    INDENT;
    fprintf (global.outfile, "SAC_WE_CALC_SIZE( %u", arg_cnt);

    unsigned int i = 0;
    for (; i < arg_cnt; i++) {
        fprintf (global.outfile, " + SAC_WE_GET_DIM( %s)", arg[i]);
    }

    fprintf (global.outfile, ")\n");

    INDENT;
    fprintf (global.outfile, "SAC_WE_ALLOC_SHAPE_ARRAY()\n");

    INDENT;
    fprintf (global.outfile, "SAC_WE_SET_NUM_ARGS( %u)\n", arg_cnt);

    i = 0;
    for (; i < arg_cnt; i++) {
        INDENT;
        fprintf (global.outfile, "SAC_WE_GET_SHAPE( %s)\n", arg[i]);
    }

    DBUG_RETURN ();
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
ICMCompileWE_NO_SHAPE_ENCODE (unsigned int arg_cnt)
{
    DBUG_ENTER ();

    fprintf (global.outfile, "/* No shape information available!!! */");

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWE_FUN_DEF_END( char *name, char *rettype_NT,
 *                                  unsigned int vararg_cnt, char **vararg)
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
ICMCompileWE_FUN_DEF_END (char *name, char *rettype_NT, unsigned int vararg_cnt, char **vararg)
{
    DBUG_ENTER ();

#define WE_FUN_DEF_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef WE_FUN_DEF_END

    global.indent--;
    INDENT;
    fprintf (global.outfile, "}\n");
    INDENT;
    fprintf (global.outfile, "SAC_WE_FUN_DEF_END2()\n");

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
