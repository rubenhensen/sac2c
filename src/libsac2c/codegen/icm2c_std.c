#include <stdio.h>
#include <ctype.h>

#include "icm2c_basic.h"
#include "icm2c_utils.h"
#include "icm2c_std.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "convert.h"
#include "globals.h"
#include "print.h"
#include "gen_startup_code.h"
#include "free.h"
#include "str.h"
#include "memory.h"
#include "ctinfo.h"

#ifdef BEtest
#define MEMfree(x)                                                                       \
    do {                                                                                 \
        x;                                                                               \
        free (x);                                                                        \
    } while (0)
#define MEMmalloc(x) malloc (x)
#endif /* BEtest */

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_FUN_DECL( char *name, char *rettype_NT,
 *                              unsigned int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_FUN_DECL( name, rettype_NT, vararg_cnt, [ TAG, basetype, arg_NT ]* )
 *
 *   where TAG is element in { in, in_..., out, out_..., inout, inout_... }.
 *
 ******************************************************************************/

void
ICMCompileND_FUN_DECL (char *name, char *rettype_NT, unsigned int vararg_cnt, char **vararg)
{
    DBUG_ENTER ();

#define ND_FUN_DECL
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_FUN_DECL

    indout ("SAC_ND_DECL_FUN2( %s, ", name);

    if (rettype_NT[0] != '\0') {
        out ("SAC_ND_TYPE_NT( %s), ", rettype_NT);
    } else {
        out ("void, ");
    }

    if (vararg_cnt > 0) {
        SCAN_ARG_LIST (vararg_cnt, 3, ",", ,
                       out (" SAC_ND_PARAM_%s( %s, %s)", vararg[i], vararg[i + 2],
                            vararg[i + 1]));
    } else {
        out ("void");
    }
    out (")");

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_DISTMEM_FUN_DECL_WITH_SIDE_EFFECTS ( char *name, char *rettype_NT,
 *                                                          unsigned int vararg_cnt, char **vararg)
 *
 * description:
 *   Currently, this ICM only serves debugging purposes in the C code by showing which
 *   functions have side effects when the distributed memory backend is used.
 *   All calls are redirected to ICMCompileND_FUN_DECL.
 *
 ******************************************************************************/

void
ICMCompileND_DISTMEM_FUN_DECL_WITH_SIDE_EFFECTS (char *name, char *rettype_NT,
                                                 unsigned int vararg_cnt, char **vararg)
{
    DBUG_ENTER ();

#define ND_DISTMEM_FUN_DECL_WITH_SIDE_EFFECTS
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_DISTMEM_FUN_DECL_WITH_SIDE_EFFECTS

    ICMCompileND_FUN_DECL (name, rettype_NT, vararg_cnt, vararg);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_FUN_DEF_BEGIN( char *name, char *rettype_NT,
 *                                    unsigned int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_FUN_DEF_BEGIN( name, rettype_NT, vararg_cnt, [ TAG, basetype, arg_NT ]* )
 *
 *   where TAG is element in { in, in_..., out, out_..., inout, inout_... }.
 *
 ******************************************************************************/

void
ICMCompileND_FUN_DEF_BEGIN (char *name, char *rettype_NT, unsigned vararg_cnt, char **vararg)
{
    DBUG_ENTER ();

#define ND_FUN_DEF_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_FUN_DEF_BEGIN

    indout ("SAC_ND_DEF_FUN_BEGIN2( %s, ", name);

    if (rettype_NT[0] != '\0') {
        out ("SAC_ND_TYPE_NT( %s), ", rettype_NT);
    } else {
        out ("void, ");
    }

    if (vararg_cnt > 0) {
        SCAN_ARG_LIST (vararg_cnt, 3, ",", ,
                       out (" SAC_ND_PARAM_%s( %s, %s)", vararg[i], vararg[i + 2],
                            vararg[i + 1]));
    } else {
        out ("void");
    }
    out (")\n");

    indout ("{\n");
    global.indent++;

    /* The ND_FUN_DEF ICM is used for SEQ and ST functions.
     * The SEQ functions are assumed always running in the single-threaded context (hence
     * the HM annotation), and they do not contain any SPMD invocations. The ST functions
     * are also assumed always running in the single-threaded context, but they may
     * contain an SPMD invocation. They are used only in stand-alone programs, i.e. not
     * from external calls. The SAC_MT_DEFINE_ST_SELF() defines the SAC_MT_self local
     * variable. In SEQ it will be ultimately NULL, while in ST it shall point to the
     * global singleton queen-bee.
     */
    indout ("SAC_HM_DEFINE_THREAD_STATUS( SAC_HM_single_threaded)\n");
    indout ("SAC_MT_DEFINE_ST_SELF()\n");

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_FUN_DEF_END( char *name, char *rettype_NT,
 *                                  unsigned int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_FUN_DEF_END( name, rettype, vararg_cnt, [ TAG, type, param_NT ]* )
 *
 *   This ICM implements end of a standard function. The first parameter
 *   specifies the name of this function.
 *   TAG may be from the set { in, out, inout }.
 *
 ******************************************************************************/

void
ICMCompileND_FUN_DEF_END (char *name, char *rettype_NT, unsigned int vararg_cnt, char **vararg)
{
    DBUG_ENTER ();

#define ND_FUN_DEF_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_FUN_DEF_END

    global.indent--;
    indout ("}\n");
    indout ("SAC_ND_FUN_DEF_END2()\n");

    DBUG_RETURN ();
}

/** <!--*********************************************************************-->
 *
 * @fn void ICMCompileMUTC_THREADFUN_DECL( char *name, char *rettype_NT,
 *                                         unsigned int vararg_cnt, char **vararg)
 *
 * @brief
 *
 ******************************************************************************/

void
ICMCompileMUTC_THREADFUN_DECL (char *name, char *rettype_NT, unsigned int vararg_cnt,
                               char **vararg)
{
    DBUG_ENTER ();

#define MUTC_THREADFUN_DECL
#include "icm_comment.c"
#include "icm_trace.c"
#undef MUTC_THREADFUN_DECL

    DBUG_ASSERT (rettype_NT[0] == '\0', "Thread funs must have a return type of void");

    indout ("SAC_MUTC_DECL_THREADFUN2( %s, , ", name);
    if (vararg_cnt > 0) {
        SCAN_ARG_LIST (vararg_cnt, 3, ",", ,
                       out (" SAC_ND_PARAM_%s( %s, %s)", vararg[i], vararg[i + 2],
                            vararg[i + 1]));
    } else {
        out ("void");
    }
    out (")");

    DBUG_RETURN ();
}

void
ICMCompileMUTC_THREADFUN_DEF_BEGIN (char *name, char *rettype_NT, unsigned int vararg_cnt,
                                    char **vararg)
{
    DBUG_ENTER ();

#define MUTC_THREADFUN_DEF_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef MUTC_THREADFUN_DEF_BEGIN

    DBUG_ASSERT (rettype_NT[0] == '\0', "Thread funs must have a return type of void");

    indout ("SAC_MUTC_DEF_THREADFUN_BEGIN2 ( %s, , ", name);
    if (vararg_cnt > 0) {
        SCAN_ARG_LIST (vararg_cnt, 3, ",", ,
                       out (" SAC_ND_PARAM_%s( %s, %s)", vararg[i], vararg[i + 2],
                            vararg[i + 1]));
    } else {
        out ("void");
    }
    out (")");

    DBUG_RETURN ();
}

/** <!--*********************************************************************-->
 *
 * @fn void ICMCompileMUTC_SPAWNFUN_DECL( char *name, char *rettype_NT,
 *                                         unsigned int vararg_cnt, char **vararg)
 *
 * @brief
 *
 ******************************************************************************/

void
ICMCompileMUTC_SPAWNFUN_DECL (char *name, char *rettype_NT, unsigned int vararg_cnt, char **vararg)
{
    DBUG_ENTER ();

#define MUTC_SPAWNFUN_DECL
#include "icm_comment.c"
#include "icm_trace.c"
#undef MUTC_SPAWNFUN_DECL

    DBUG_ASSERT (rettype_NT[0] == '\0', "Spawn funs must have a return type of void");

    indout ("SAC_MUTC_SPAWNFUN_DECL2( %s, , ", name);
    if (vararg_cnt > 0) {
        SCAN_ARG_LIST (vararg_cnt, 3, ",", ,
                       out (" SAC_ND_PARAM_%s( %s, %s)", vararg[i], vararg[i + 2],
                            vararg[i + 1]));
    } else {
        out ("void");
    }
    out (")");

    DBUG_RETURN ();
}

void
ICMCompileMUTC_SPAWNFUN_DEF_BEGIN (char *name, char *rettype_NT, unsigned int vararg_cnt,
                                   char **vararg)
{
    DBUG_ENTER ();

#define MUTC_SPAWNFUN_DEF_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef MUTC_SPAWNFUN_DEF_BEGIN

    DBUG_ASSERT (rettype_NT[0] == '\0', "Spawn funs must have a return type of void");

    indout ("SAC_MUTC_DEF_SPAWNFUN_BEGIN2( %s, , ", name);
    if (vararg_cnt > 0) {
        SCAN_ARG_LIST (vararg_cnt, 3, ",", ,
                       out (" SAC_ND_PARAM_%s( %s, %s)", vararg[i], vararg[i + 2],
                            vararg[i + 1]));
    } else {
        out ("void");
    }
    out (")");

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_FUN_AP( char *name, char *retname,
 *                             unsigned int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_FUN_DEC( name, retname, vararg_cnt, [ TAG, basetype, arg_NT ]* )
 *
 *   where TAG is element in { in, in_..., out, out_..., inout, inout_... }.
 *
 ******************************************************************************/

void
ICMCompileND_FUN_AP (char *name, char *retname, unsigned int vararg_cnt, char **vararg)
{
    DBUG_ENTER ();

#define ND_FUN_AP
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_FUN_AP

    if (global.backend == BE_distmem) {
        BLOCK_NOVAR_BEGIN ()
            ;
            indout ("bool SAC_FUN_AP_was_side_effects_outer = FALSE;\n");
            IF_BEGIN ("SAC_DISTMEM_exec_mode == SAC_DISTMEM_exec_mode_side_effects_outer")
                ;
                /* We are in side effects outer execution mode.
                 * Switch to side effects execution mode. */
                indout ("SAC_DISTMEM_SWITCH_TO_SIDE_EFFECTS_EXEC();\n");
                indout ("SAC_FUN_AP_was_side_effects_outer = TRUE;\n");
            IF_END ();
    }

    if (!STReq (retname, "")) {
        indout ("%s = %s(", retname, name);
    } else {
        indout ("SAC_ND_FUNAP2( %s, ", name);
    }

    SCAN_ARG_LIST (vararg_cnt, 3, ",", ,
                   out (" SAC_ND_ARG_%s( %s, %s)", vararg[i], vararg[i + 2],
                        vararg[i + 1]));

    if (!STReq (retname, "")) {
        out (");\n");
    } else {
        out (")\n");
    }

    if (global.backend == BE_distmem) {
        IF_BEGIN ("SAC_FUN_AP_was_side_effects_outer")
            ;
            /* We were in side effects outer execution mode before the function
             * application. Switch back. */
            indout ("SAC_DISTMEM_SWITCH_TO_SIDE_EFFECTS_OUTER_EXEC();\n");
        IF_END ();
BLOCK_END
();
}

DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS_HID_UNQ_CHECK( char *name, char
 **retname, char *ret_NT, unsigned int vararg_cnt, char **vararg_NT, char **vararg)
 *
 * description:
 *
 *   For hidden out arguments and hidden return values, we raises a compile time warning.
 *   The distributed memory backend does not support hidden out arguments and hidden
 *return values in function applications with side effects since this would require C
 *functions for the (de-)serialisation when the out arguments and return values are
 *broadcasted from the master to the worker nodes after the function application.
 *
 ******************************************************************************/

static void
ICMCompileND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS_HID_UNQ_CHECK (char *name, char *retname,
                                                             char *ret_NT, unsigned int vararg_cnt,
                                                             char **vararg_NT,
                                                             char **vararg)
{
    DBUG_ENTER ();

    /* FIXME: We have to detect whether the return value is unique
     * so that we can raise a compile time error if there is a hidden non-unique return
     * value. For now we assume that the return value is unique (and therefore only used
     * by the master node), initialize it to NULL at the worker nodes and raise a compile
     * time warning. */
    if (!STReq (retname, "") && ICUGetHiddenClass (ret_NT) == C_hid) {
        CTIwarn (EMPTY_LOC, 
          "The distributed memory backend does not support hidden non-unique return "
          "values in function applications with side-effects (application of function: "
          "%s). "
          "We assume that the return value is unique and initialize it to NULL. If the "
          "program works, don't worry about this. "
          "However, if you encounter a segfault, the reason may be that the return value "
          "is not unique after all.",
          name);
    }

    /* Raise compile time warning if there is a hidden out argument. */
    for (unsigned int i = 0; i < vararg_cnt * 3; i += 3) {
#define SEPargtag
#define SELECTtextoutinout(it_text, it_out, it_inout)                                    \
    if (STReq (it_text, vararg[i]) && it_out                                             \
        && ICUGetHiddenClass (vararg_NT[i / 3]) == C_hid) {                              \
        CTIwarn (EMPTY_LOC, "The distributed memory backend does not support hidden non-unique "    \
                 "out arguments in function applications with side-effects (argument "   \
                 "%s of function %s). "                                                  \
                 "We assume that the out argument is unique and initialize it to NULL. " \
                 "If the program "                                                       \
                 "works, don't worry about this. However, if you encounter a segfault, " \
                 "the reason may "                                                       \
                 "be that the out argument is not unique after all.",                    \
                 vararg[i + 2], name);                                                   \
    }
#include "argtag_info.mac"
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void  ICMCompileND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS_HID_UNQ_INIT( char *name, char
 **retname, char *ret_NT, unsigned int vararg_cnt, char **vararg_NT, char **vararg)
 *
 * description:
 *
 *   Initializes hidden out arguments and hidden return values to NULL (assuming they are
 *unique). This function also handles inout arguments which are always unique. We can
 *safely initialise them to NULL at all worker nodes as they are only used by the master.
 *
 ******************************************************************************/

static void
ICMCompileND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS_HID_UNQ_INIT (char *name, char *retname,
                                                            char *ret_NT, unsigned int vararg_cnt,
                                                            char **vararg_NT,
                                                            char **vararg)
{
    DBUG_ENTER ();

    /* FIXME: We have to detect whether the return value is unique
     * so that we can raise a runtime error if there is a hidden non-unique return value.
     * For now we assume that the return value is unique (and therefore only used by the
     * master node) and initialize it to NULL at the worker nodes. */
    if (!STReq (retname, "") && ICUGetHiddenClass (ret_NT) == C_hid) {
        indout ("%s = NULL;\n", retname);
    }

    /* FIXME: We have to detect whether the out argument is unique
     * so that we can raise a runtime error if there is a hidden non-unique out argument.
     * For now we assume that the out argument value is unique (and therefore only used by
     * the master node) and initialize it to NULL at the worker nodes. Inout arguments
     * should always be unique. We initialize them to NULL if they are hidden. */
    for (unsigned int i = 0; i < vararg_cnt * 3; i += 3) {
#define SEPargtag
#define SELECTtextoutinout(it_text, it_out, it_inout)                                    \
    if (STReq (it_text, vararg[i]) && (it_out || it_inout)                               \
        && ICUGetHiddenClass (vararg_NT[i / 3]) == C_hid) {                              \
        indout ("SAC_ND_A_FIELD( %s) = NULL;\n", vararg[i + 2]);                         \
    }
#include "argtag_info.mac"
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS_INCRC( unsigned int vararg_cnt, char
 ***vararg_NT, char **vararg)
 *
 * description:
 *   Create code to increment the reference counter for distributed in arguments.
 *   We have to ensure that all nodes allocate and free variables in the DSM segment in
 *the same order. By increasing the reference counter before the function application, we
 *prevent that the variable is freed within the function. After the function application,
 *we broadcast the updated reference counter from the master to the workers, decrement it
 *and, if necessary, free the variable at all nodes.
 *
 ******************************************************************************/

static void
ICMCompileND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS_INCRC (unsigned int vararg_cnt, char **vararg_NT,
                                                     char **vararg)
{
    DBUG_ENTER ();

    /*
     * Create code to increment the reference counter for distributed in arguments.
     * We have to ensure that all nodes allocate and free variables in the DSM segment in
     * the same order. By increasing the reference counter before the function
     * application, we prevent that the variable is freed within the function.
     */
    for (unsigned int i = 0; i < vararg_cnt * 3; i += 3) {
#define SEPargtag
#define SELECTtextin(it_text, it_in)                                                     \
    if (STReq (it_text, vararg[i]) && it_in                                              \
        && ICUGetDistributedClass (vararg_NT[i / 3]) == C_distr) {                       \
        IF_BEGIN ("SAC_ND_A_IS_DIST( %s)", vararg_NT[i / 3])                             \
            ;                                                                            \
            indout ("SAC_ND_INC_RC( %s, 1);\n", vararg_NT[i / 3]);                       \
            indout (                                                                     \
              "SAC_TR_DISTMEM_PRINT( \"DISTMEM_FUN_AP_WITH_SIDE_EFFECTS_INCRC( %%s)"     \
              ": new value = %%\"PRIdPTR,"                                               \
              "NT_STR( %s), SAC_ND_A_RC( %s));\n",                                       \
              vararg_NT[i / 3], vararg_NT[i / 3]);                                       \
        IF_END ();                                                                       \
    }
#include "argtag_info.mac"
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS_BROADCAST( char *retname, char
 **rettype, char *ret_NT, unsigned int vararg_cnt, char **vararg_NT, char **vararg, char
 **broadcast_operation)
 *
 * description:
 *   Creates code for the broadcast operations for non-hidden out arguments and non-hidden
 *return values of function applications with side effects when the distributed memory
 *backend is used.
 *
 ******************************************************************************/

static void
ICMCompileND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS_BROADCAST (char *retname, char *rettype,
                                                         char *ret_NT, unsigned int vararg_cnt,
                                                         char **vararg_NT, char **vararg,
                                                         char *broadcast_operation)
{
    DBUG_ENTER ();

    if (!STReq (retname, "") && ICUGetHiddenClass (ret_NT) != C_hid) {
        /* Create code for broadcast operation for non-hidden return value. */
        shape_class_t sc = ICUGetShapeClass (ret_NT);
        char *prefix = "";
        if (sc == C_akd || sc == C_aud) {
            prefix = "WITH_DESC_";
        } else if (sc == C_scl) {
            prefix = "SCL_";
        }
        indout ("SAC_DISTMEM_BROADCAST_%s%s( %s, %s);\n", prefix, broadcast_operation,
                rettype, ret_NT);
    }

    for (unsigned int i = 0; i < vararg_cnt * 3; i += 3) {
/*
 * Create code for broadcast operations for non-hidden out and in-out arguments.
 * Inout arguments should always be unique but for the time being we broadcast them
 * anyways so that free operations and passing them as arguments to other functions don't
 * fail.
 */
#define SEPargtag
#define SELECTtextoutinout(it_text, it_out, it_inout)                                    \
    if (STReq (it_text, vararg[i]) && (it_out || it_inout)                               \
        && ICUGetHiddenClass (vararg_NT[i / 3]) != C_hid) {                              \
        shape_class_t sc = ICUGetShapeClass (vararg_NT[i / 3]);                          \
        char *prefix = "";                                                               \
        if (sc == C_akd || sc == C_aud) {                                                \
            prefix = "WITH_DESC_";                                                       \
        } else if (sc == C_scl) {                                                        \
            prefix = "SCL_";                                                             \
        }                                                                                \
        indout ("SAC_DISTMEM_BROADCAST_%s%s( %s, %s);\n", prefix, broadcast_operation,   \
                vararg[i + 1], vararg[i + 2]);                                           \
    }
#include "argtag_info.mac"

/*
 * Create code for reference counter broadcasts for distributable in arguments.
 */
#define SEPargtag
#define SELECTtextin(it_text, it_in)                                                     \
    if (STReq (it_text, vararg[i]) && it_in                                              \
        && ICUGetDistributedClass (vararg_NT[i / 3]) == C_distr) {                       \
        indout ("SAC_DISTMEM_BROADCAST_RC_%s( %s);\n", broadcast_operation,              \
                vararg[i + 2]);                                                          \
    }
#include "argtag_info.mac"
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS_BROADCAST_BARRIER( char *retname,
 *char *ret_NT, unsigned int vararg_cnt, char **vararg_NT, char **vararg)
 *
 * description:
 *   Creates code for a barrier operation to be used between broadcast operations for
 *function applications with side effects when the distributed memory backend is used. The
 *barrier is only created if there was at least one broadcast operation.
 *
 ******************************************************************************/

static void
ICMCompileND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS_BROADCAST_BARRIER (
  char *retname, char *ret_NT, unsigned int vararg_cnt, char **vararg_NT, char **vararg)
{
    DBUG_ENTER ();

    bool do_barrier = FALSE;

    if (!STReq (retname, "") && ICUGetHiddenClass (ret_NT) != C_hid) {
        /* There was a broadcast operation for a non-hidden return value. */
        do_barrier = TRUE;
    }

    if (!do_barrier) {
        for (unsigned int i = 0; i < vararg_cnt * 3; i += 3) {
/* Check whether there was a broadcast operation for a non-hidden out or inout argument.
 */
#define SEPargtag
#define SELECTtextoutinout(it_text, it_out, it_inout)                                    \
    if (STReq (it_text, vararg[i]) && (it_out || it_inout)                               \
        && ICUGetHiddenClass (vararg_NT[i / 3]) != C_hid) {                              \
        do_barrier = TRUE;                                                               \
        break;                                                                           \
    }
#include "argtag_info.mac"

/* Check whether there was (possibly) a broadcast operation for a distributable in
 * argument. */
#define SEPargtag
#define SELECTtextin(it_text, it_in)                                                     \
    if (STReq (it_text, vararg[i]) && it_in                                              \
        && ICUGetDistributedClass (vararg_NT[i / 3]) == C_distr) {                       \
        do_barrier = TRUE;                                                               \
        break;                                                                           \
    }
#include "argtag_info.mac"
        }
    }

    if (do_barrier) {
        indout ("SAC_DISTMEM_BARRIER();\n");
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS( int vararg_NT_cnt, char
 ***vararg_NT, char *rettype, char *ret_NT, char *name, char *retname, unsigned int vararg_cnt,
 *char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS( vararg_NT_cnt, vararg_NT*, rettype, ret_NT,
 *                                        name, retname, vararg_cnt, [ TAG, basetype,
 *arg_NT ]* )
 *
 *   where TAG is element in { in, in_..., out, out_..., inout, inout_... }.
 *
 ******************************************************************************/

void
ICMCompileND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS (unsigned int vararg_NT_cnt, char **vararg_NT,
                                               char *rettype, char *ret_NT, char *name,
                                               char *retname, unsigned int vararg_cnt,
                                               char **vararg)
{
    DBUG_ENTER ();

#define ND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS

    /*
     * We have to check whether we are already in side effects execution mode at runtime
     * because the following may happen:
     * fun0 declared as ND_FUN_DECL
     * fun0 calls fun1 ND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS
     * fun1 declared as ND_DISTMEM_FUN_DECL_WITH_SIDE_EFFECTS
     * fun1 calls fun2 ND_FUN_AP
     * fun2 declared as ND_FUN_DECL
     * fun2 calls fun3 ND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS --> Error: already in side
     * effects execution mode fun3 declared as ND_DISTMEM_FUN_DECL_WITH_SIDE_EFFECTS
     *
     * In a function declared with ND_DISTMEM_FUN_DECL_WITH_SIDE_EFFECTS we know
     * for sure at compile time that it will already be in side effects execution mode
     * when it is called but for a function declared with ND_FUN_DECL that depends on
     * where it is called from. It may be possible to recognize this in some cases at
     * compile time but not across module borders.
     */
    IF_BEGIN ("SAC_DISTMEM_exec_mode == SAC_DISTMEM_exec_mode_side_effects"
              "|| SAC_DISTMEM_exec_mode == SAC_DISTMEM_exec_mode_side_effects_outer")
        ;
        /* We are already in side effects (outer) execution mode. Only the master will
         * execute this so no special treatment is necessary. */
        indout ("SAC_TR_DISTMEM_PRINT( \"Already in side effects (outer) execution mode "
                "when calling function: %s. "
                "Redirecting to sequential function application.\");\n\n",
                name);
        ICMCompileND_FUN_AP (name, retname, vararg_cnt, vararg);
    IF_END ();
    ELSE_BEGIN ()
        ;
        /* It is important that we are in a block at this point because the variables
         * declared by the broadcast operations are only needed within the block. */

        indout ("SAC_DISTMEM_SWITCH_TO_SIDE_EFFECTS_OUTER_EXEC();\n");
        indout ("SAC_DISTMEM_BARRIER();\n");

        /* Handle non-unique hidden arguments and non-unique hidden return values. */
        ICMCompileND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS_HID_UNQ_CHECK (name, retname,
                                                                     ret_NT, vararg_cnt,
                                                                     vararg_NT, vararg);

        IF_BEGIN ("SAC_DISTMEM_rank == SAC_DISTMEM_RANK_MASTER")
            ;
            indout ("SAC_TR_DISTMEM_PRINT( \"Master node is executing function "
                    "application with side effects: %s\");\n",
                    name);
            indout ("SAC_DISTMEM_FORBID_DSM_ALLOCS();\n");

            /*
             * Increment reference counters of distributed in arguments so that they are
             * not freed within the function. The updated reference counters will be
             * broadcast from the master to the workers after the function application.
             * Then, all nodes will decrement the reference counters and if they become
             * zero, free the variables. (The latter happens in
             * SAC_DISTMEM_BROADCAST_RC_FREE_MEM_COMMON.)
             */
            ICMCompileND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS_INCRC (vararg_cnt, vararg_NT,
                                                                 vararg);

            if (!STReq (retname, "")) {
                indout ("%s = %s(", retname, name);
            } else {
                indout ("SAC_ND_FUNAP2( %s, ", name);
            }

            SCAN_ARG_LIST (vararg_cnt, 3, ",", ,
                           out (" SAC_ND_ARG_%s( %s, %s)", vararg[i], vararg[i + 2],
                                vararg[i + 1]));

            if (!STReq (retname, "")) {
                out (");\n");
            } else {
                out (")\n");
            }

            /* Allow allocations in the DSM segment for the broadcast operations. */
            indout ("SAC_DISTMEM_ALLOW_DSM_ALLOCS();\n");

            /* Required broadcast operations */
            ICMCompileND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS_BROADCAST (retname, rettype,
                                                                     ret_NT, vararg_cnt,
                                                                     vararg_NT, vararg,
                                                                     "INIT_MASTER");
            ICMCompileND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS_BROADCAST_BARRIER (retname,
                                                                             ret_NT,
                                                                             vararg_cnt,
                                                                             vararg_NT,
                                                                             vararg);

            /* There is no finalize operation for the master. */
            ICMCompileND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS_BROADCAST_BARRIER (retname,
                                                                             ret_NT,
                                                                             vararg_cnt,
                                                                             vararg_NT,
                                                                             vararg);

            ICMCompileND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS_BROADCAST (retname, rettype,
                                                                     ret_NT, vararg_cnt,
                                                                     vararg_NT, vararg,
                                                                     "FREE_MEM_COMMON");
        IF_END ();
        ELSE_BEGIN ()
            ;
            indout ("SAC_TR_DISTMEM_PRINT( \"Non-master node is skipping function "
                    "application with side effects: %s\");\n",
                    name);

            /* Handle hidden arguments, hidden return values and unique (inout) arguments.
             */
            ICMCompileND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS_HID_UNQ_INIT (name, retname,
                                                                        ret_NT,
                                                                        vararg_cnt,
                                                                        vararg_NT,
                                                                        vararg);

            /* Required broadcast operations */
            ICMCompileND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS_BROADCAST (retname, rettype,
                                                                     ret_NT, vararg_cnt,
                                                                     vararg_NT, vararg,
                                                                     "INIT_WORKER");
            ICMCompileND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS_BROADCAST_BARRIER (retname,
                                                                             ret_NT,
                                                                             vararg_cnt,
                                                                             vararg_NT,
                                                                             vararg);

            ICMCompileND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS_BROADCAST (retname, rettype,
                                                                     ret_NT, vararg_cnt,
                                                                     vararg_NT, vararg,
                                                                     "STEP1_WORKER");
            ICMCompileND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS_BROADCAST (retname, rettype,
                                                                     ret_NT, vararg_cnt,
                                                                     vararg_NT, vararg,
                                                                     "STEP2_WORKER");
            ICMCompileND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS_BROADCAST_BARRIER (retname,
                                                                             ret_NT,
                                                                             vararg_cnt,
                                                                             vararg_NT,
                                                                             vararg);

            ICMCompileND_DISTMEM_FUN_AP_WITH_SIDE_EFFECTS_BROADCAST (retname, rettype,
                                                                     ret_NT, vararg_cnt,
                                                                     vararg_NT, vararg,
                                                                     "FREE_MEM_COMMON");
        ELSE_END ();

        if (global.profile.distmem) {
            /* If profiling for the distributed memory backend is active, insert a
             * barrier. This way, we get a more accurate estimate of how much time is
             * spent in side effects execution mode. */
            indout ("SAC_DISTMEM_BARRIER();\n");
        }
        indout ("SAC_DISTMEM_SWITCH_TO_SYNC_EXEC();\n");

    ELSE_END ();

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMUTC_THREADFUN_AP( char *name,
 *                                     char *retname,
 *                                     unsigned int vararg_cnt,
 *                                     char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MUTC_THREADFUN_AP( name, vararg_cnt, [ TAG, basetype, arg_NT ]* )
 *
 *   where TAG is element in { in, in_..., out, out_..., inout, inout_... }.
 *
 ******************************************************************************/

void
ICMCompileMUTC_THREADFUN_AP (char *name, char *retname, unsigned int vararg_cnt, char **vararg)
{
    DBUG_ENTER ();

#define MUTC_THREADFUN_AP
#include "icm_comment.c"
#include "icm_trace.c"
#undef MUTC_THREADFUN_AP

    indout ("SAC_MUTC_THREAD_AP2( %s, ", name);

    SCAN_ARG_LIST (vararg_cnt, 3, ",", ,
                   out (" SAC_ND_ARG_%s( %s, %s)", vararg[i], vararg[i + 2],
                        vararg[i + 1]));
    out (")\n");

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMUTC_FUNTHREADFUN_AP( char *name,
 *                                     char *retname,
 *                                     unsigned int vararg_cnt,
 *                                     char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MUTC_FUNTHREADFUN_AP( name, vararg_cnt, [ TAG, basetype, arg_NT ]* )
 *
 *   where TAG is element in { in, in_..., out, out_..., inout, inout_... }.
 *
 ******************************************************************************/

void
ICMCompileMUTC_FUNTHREADFUN_AP (char *name, char *retname, unsigned int vararg_cnt, char **vararg)
{
    DBUG_ENTER ();

#define MUTC_FUNTHREADFUN_AP
#include "icm_comment.c"
#include "icm_trace.c"
#undef MUTC_FUNTHREADFUN_AP

    indout ("SAC_MUTC_THREAD_FUNAP( %s, ", name);

    SCAN_ARG_LIST (vararg_cnt, 3, ",", ,
                   out (" SAC_ND_ARG_%s( %s, %s)", vararg[i], vararg[i + 2],
                        vararg[i + 1]));
    out (")\n");

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMUTC_SPAWNFUN_AP( char *var_NT,
 *                                    char *place,
 *                                    char *name,
 *                                    char *retname,
 *                                    unsigned int vararg_cnt,
 *                                    char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MUTC_SPAWNFUN_AP( var_NT, place, name, vararg_cnt,
 *                     [ TAG, basetype, arg_NT ]* )
 *
 *   where TAG is element in { in, in_..., out, out_..., inout, inout_... }.
 *
 ******************************************************************************/

void
ICMCompileMUTC_SPAWNFUN_AP (char *var_NT, char *place, char *name, char *retname,
                            unsigned int vararg_cnt, char **vararg)
{
    DBUG_ENTER ();

#define MUTC_SPAWNFUN_AP
#include "icm_comment.c"
#include "icm_trace.c"
#undef MUTC_SPAWNFUN_AP

    indout ("SAC_MUTC_SPAWN_AP( %s, %s, %s, ", var_NT, place, name);
    SCAN_ARG_LIST (vararg_cnt, 3, ",", ,
                   out (" SAC_ND_ARG_%s( %s, %s)", vararg[i], vararg[i + 2],
                        vararg[i + 1]));
    out (")\n");

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_FUN_RET( char *retname, unsigned int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_FUN_RET( retname, vararg_cnt, [ TAG, arg_NT, decl_arg_NT ]* )
 *
 *   where TAG is element in { out, inout }.
 *
 ******************************************************************************/

void
ICMCompileND_FUN_RET (char *retname, unsigned int vararg_cnt, char **vararg)
{
    DBUG_ENTER ();

#define ND_FUN_RET
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_FUN_RET

    if (vararg_cnt > 0) {
        INDENT;
        SCAN_ARG_LIST (vararg_cnt, 3, "\n", INDENT,
                       out ("SAC_ND_RET_%s( %s, %s)", vararg[i], vararg[i + 1],
                            vararg[i + 2]));
        out ("\n");
    }

    if (!STReq (retname, "")) {
        indout ("return( %s);", retname);
    } else {
        indout ("return;");
    }

    DBUG_RETURN ();
}

/** <!--*********************************************************************-->
 *
 * function:
 *   void ICMCompileMUTC_THREADFUN_RET( char *retname,
 *                                      unsigned int vararg_cnt,
 *                                      char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MUTC_THREADFUN_RET( retname, vararg_cnt, [ TAG, arg_NT, decl_arg_NT ]* )
 *
 *   where TAG is element in { out, inout }.
 *
 *****************************************************************************/

void
ICMCompileMUTC_THREADFUN_RET (char *retname, unsigned int vararg_cnt, char **vararg)
{
    DBUG_ENTER ();

#define MUTC_THREADFUN_RET
#include "icm_comment.c"
#include "icm_trace.c"
#undef MUTC_THREADFUN_RET

    if (vararg_cnt > 0) {
        INDENT;
        SCAN_ARG_LIST (vararg_cnt, 3, "\n", INDENT,
                       out ("SAC_ND_RET_%s( %s, %s)", vararg[i], vararg[i + 1],
                            vararg[i + 2]));
        out ("\n");
    }

    if (!STReq (retname, "")) {
        indout ("return( %s);", retname);
    } else {
        indout ("return;");
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_OBJDEF( char *var_NT, char *basetype, int sdim, int *shp)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_OBJDEF( var_NT, basetype, sdim, [ shp ]* )
 *
 ******************************************************************************/

void
ICMCompileND_OBJDEF (char *var_NT, char *basetype, int sdim, int *shp)
{
    DBUG_ENTER ();

#define ND_OBJDEF
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_OBJDEF

    if (global.print_objdef_for_header_file) {
        ICMCompileND_DECL_EXTERN (var_NT, basetype, sdim);
    } else if (global.filetype == FT_prog){
        indout ("SAC_ND_DECL__DATA( %s, %s, static)\n", var_NT, basetype);
        indout ("SAC_ND_DECL__DESC( %s, static)\n", var_NT);
        ICMCompileND_DECL__MIRROR (var_NT, sdim, shp);
    } else {
        ICMCompileND_DECL (var_NT, basetype, sdim, shp);
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_OBJDEF_EXTERN( char *var_NT, char *basetype, int sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_OBJDEF_EXTERN( var_NT, basetype, sdim)
 *
 ******************************************************************************/

void
ICMCompileND_OBJDEF_EXTERN (char *var_NT, char *basetype, int sdim)
{
    DBUG_ENTER ();

#define ND_OBJDEF_EXTERN
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_OBJDEF_EXTERN

    ICMCompileND_DECL_EXTERN (var_NT, basetype, sdim);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_DECL( char *var_NT, char *basetype, int sdim, int *shp)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_DECL( var_NT, basetype, sdim, [ shp ]* )
 *
 ******************************************************************************/

void
ICMCompileND_DECL (char *var_NT, char *basetype, int sdim, int *shp)
{
    DBUG_ENTER ();

#define ND_DECL
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_DECL

    indout ("SAC_ND_DECL__DATA( %s, %s, )\n", var_NT, basetype);
    indout ("SAC_ND_DECL__DESC( %s, )\n", var_NT);
    ICMCompileND_DECL__MIRROR (var_NT, sdim, shp);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_DSM_DECL( char *var_NT, char *basetype, int sdim, int *shp)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_DSM_DECL( var_NT, basetype, sdim, [ shp ]* )
 *
 ******************************************************************************/

void
ICMCompileND_DSM_DECL (char *var_NT, char *basetype, int sdim, int *shp)
{
    DBUG_ENTER ();

#define ND_DSM_DECL
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_DSM_DECL

    indout ("SAC_ND_DECL__DATA( %s, %s, )\n", var_NT, basetype);
    indout ("SAC_ND_DECL__DESC( %s, )\n", var_NT);
    ICMCompileND_DECL__MIRROR (var_NT, sdim, shp);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_DECL_EXTERN( char *var_NT, char *basetype, int sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_DECL_EXTERN( var_NT, basetype, sdim)
 *
 ******************************************************************************/

void
ICMCompileND_DECL_EXTERN (char *var_NT, char *basetype, int sdim)
{
    DBUG_ENTER ();

#define ND_DECL_EXTERN
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_DECL_EXTERN

    indout ("SAC_ND_DECL__DATA( %s, %s, extern)\n", var_NT, basetype);
    indout ("SAC_ND_DECL__DESC( %s, extern)\n", var_NT);
    ICMCompileND_DECL__MIRROR_EXTERN (var_NT, sdim);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_DECL__MIRROR( char *var_NT, int sdim, int *shp)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_DECL__MIRROR( var_NT, sdim, [ shp ]* )
 *
 ******************************************************************************/

void
ICMCompileND_DECL__MIRROR (char *var_NT, int sdim, int *shp)
{
    int size, i;
    shape_class_t sc = ICUGetShapeClass (var_NT);
    int dim = DIM_NO_OFFSET (sdim);
    distributed_class_t dc = ICUGetDistributedClass (var_NT);

    DBUG_ENTER ();

#define ND_DECL__MIRROR
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_DECL__MIRROR

    switch (sc) {
    case C_scl:
        indout ("SAC_NOTHING()\n");
        break;

    case C_aks:
        size = 1;
        DBUG_ASSERT (dim >= 0, "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            indout ("const int SAC_ND_A_MIRROR_SHAPE( %s, %d) = %d;\n", var_NT, i,
                    shp[i]);
            size *= shp[i];
            DBUG_ASSERT (size >= 0, "array with size <0 found!");
        }

        indout ("const int SAC_ND_A_MIRROR_SIZE( %s) = %d;\n", var_NT, size);
        indout ("const int SAC_ND_A_MIRROR_DIM( %s) = %d;\n", var_NT, dim);

        if (global.backend == BE_distmem) {
            if (dc == C_distr) {
                /* Array is potentially distributed. */

                /*
                 * Postpone the decisison whether the array is actually distributed to
                 * runtime since this depends on the number of nodes as well.
                 * SAC_ND_A_MIRROR_IS_DIST cannot be declared constant because the array
                 * may be treated as non-distributed later on.
                 */
                indout ("bool SAC_ND_A_MIRROR_IS_DIST( %s) = "
                        "SAC_DISTMEM_DET_DO_DISTR_ARR( %d, %d);\n",
                        var_NT, size, shp[0]);

                /*
                 * Initialize these variables to avoid warnings.
                 * If the array is actually distributed, they will be set to the correct
                 * values when the data is allocated.
                 */
                indout ("size_t SAC_ND_A_MIRROR_FIRST_ELEMS( %s) = 0;\n", var_NT);
                indout ("int SAC_ND_A_MIRROR_LOCAL_FROM( %s) = 0;\n", var_NT);
                indout ("int SAC_ND_A_MIRROR_LOCAL_COUNT( %s) = 0;\n", var_NT);
                indout ("SAC_NT_CBASETYPE( %s) *SAC_ND_A_MIRROR_PTR_CACHE( %s) = NULL;\n",
                        var_NT, var_NT);
                indout ("int SAC_ND_A_MIRROR_PTR_CACHE_FROM( %s) = 0;\n", var_NT);
                indout ("int SAC_ND_A_MIRROR_PTR_CACHE_COUNT( %s) = 0;\n", var_NT);
            } else if (dc == C_distmem) {
                /* Array is potentially allocated in DSM memory. */
                indout ("bool SAC_ND_A_MIRROR_IS_DIST( %s) = "
                        "SAC_DISTMEM_DET_ALLOC_DSM_IN_DSM();\n",
                        var_NT);
            }

            if (dc == C_distr || dc == C_distmem) {
                /* Array is potentially distributed or allocated in DSM memory. */
                indout ("uintptr_t SAC_ND_A_MIRROR_OFFS( %s) = 0;\n", var_NT);
            }
        }

        break;

    case C_akd:
        DBUG_ASSERT (dim >= 0, "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            indout ("int SAC_ND_A_MIRROR_SHAPE( %s, %d);\n", var_NT, i);
        }

        indout ("int SAC_ND_A_MIRROR_SIZE( %s);\n", var_NT);
        indout ("const int SAC_ND_A_MIRROR_DIM( %s) = %d;\n", var_NT, dim);

        if (global.backend == BE_distmem) {
            if (dc == C_distr) {
                /* Array is potentially distributed. */

                /*
                 * Initialize these variables to avoid warnings.
                 * If the array is actually distributed, they will be set to the correct
                 * values when the data is allocated.
                 */
                indout ("bool SAC_ND_A_MIRROR_IS_DIST( %s) = FALSE;\n", var_NT);
                indout ("size_t SAC_ND_A_MIRROR_FIRST_ELEMS( %s) = 0;\n", var_NT);
                indout ("int SAC_ND_A_MIRROR_LOCAL_FROM( %s) = 0;\n", var_NT);
                indout ("int SAC_ND_A_MIRROR_LOCAL_COUNT( %s) = 0;\n", var_NT);
                indout ("SAC_NT_CBASETYPE( %s) *SAC_ND_A_MIRROR_PTR_CACHE( %s) = NULL;\n",
                        var_NT, var_NT);
                indout ("int SAC_ND_A_MIRROR_PTR_CACHE_FROM( %s) = 0;\n", var_NT);
                indout ("int SAC_ND_A_MIRROR_PTR_CACHE_COUNT( %s) = 0;\n", var_NT);
            } else if (dc == C_distmem) {
                /* Array is potentially allocated in DSM memory. */
                indout ("bool SAC_ND_A_MIRROR_IS_DIST( %s) = "
                        "SAC_DISTMEM_DET_ALLOC_DSM_IN_DSM();\n",
                        var_NT);
            }

            if (dc == C_distr || dc == C_distmem) {
                /* Array is potentially distributed or allocated in DSM memory. */
                indout ("uintptr_t SAC_ND_A_MIRROR_OFFS( %s) = 0;\n", var_NT);
            }
        }

        break;

    case C_aud:
        indout ("int SAC_ND_A_MIRROR_SIZE( %s) = 0;\n", var_NT);
        indout ("int SAC_ND_A_MIRROR_DIM( %s) = 0;\n", var_NT);

        if (global.backend == BE_distmem) {
            if (dc == C_distr) {
                /* Array is potentially distributed. */

                /*
                 * Initialize these variables to avoid warnings.
                 * If the array is actually distributed, they will be set to the correct
                 * values when the data is allocated.
                 */
                indout ("bool SAC_ND_A_MIRROR_IS_DIST( %s) = FALSE;\n", var_NT);
                indout ("size_t SAC_ND_A_MIRROR_FIRST_ELEMS( %s) = 0;\n", var_NT);
                indout ("int SAC_ND_A_MIRROR_LOCAL_FROM( %s) = 0;\n", var_NT);
                indout ("int SAC_ND_A_MIRROR_LOCAL_COUNT( %s) = 0;\n", var_NT);
                indout ("SAC_NT_CBASETYPE( %s) *SAC_ND_A_MIRROR_PTR_CACHE( %s) = NULL;\n",
                        var_NT, var_NT);
                indout ("int SAC_ND_A_MIRROR_PTR_CACHE_FROM( %s) = 0;\n", var_NT);
                indout ("int SAC_ND_A_MIRROR_PTR_CACHE_COUNT( %s) = 0;\n", var_NT);
            } else if (dc == C_distmem) {
                /* Array is potentially allocated in DSM memory. */
                indout ("bool SAC_ND_A_MIRROR_IS_DIST( %s) = "
                        "SAC_DISTMEM_DET_ALLOC_DSM_IN_DSM();\n",
                        var_NT);
            }

            if (dc == C_distr || dc == C_distmem) {
                /* Array is potentially distributed or allocated in DSM memory. */
                indout ("uintptr_t SAC_ND_A_MIRROR_OFFS( %s) = 0;\n", var_NT);
            }
        }

        break;

    default:
        DBUG_UNREACHABLE ("Unknown shape class found!");
        break;
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_DECL__MIRROR_PARAM( char *var_NT, int sdim, int *shp)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_DECL__MIRROR_PARAM( var_NT, sdim, [ shp ]* )
 *
 ******************************************************************************/

void
ICMCompileND_DECL__MIRROR_PARAM (char *var_NT, int sdim, int *shp)
{
    int size, i;
    shape_class_t sc = ICUGetShapeClass (var_NT);
    int dim = DIM_NO_OFFSET (sdim);
    distributed_class_t dc = ICUGetDistributedClass (var_NT);

    DBUG_ENTER ();

#define ND_DECL__MIRROR_PARAM
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_DECL__MIRROR_PARAM

    switch (sc) {
    case C_scl:
        indout ("SAC_NOTHING()\n");
        break;

    case C_aks:
        size = 1;
        DBUG_ASSERT (dim >= 0, "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            indout ("const int SAC_ND_A_MIRROR_SHAPE( %s, %d) = %d;\n", var_NT, i,
                    shp[i]);
            size *= shp[i];
            DBUG_ASSERT (size >= 0, "array with size <0 found!");
        }

        indout ("const int SAC_ND_A_MIRROR_SIZE( %s) = %d;\n", var_NT, size);
        indout ("const int SAC_ND_A_MIRROR_DIM( %s) = %d;\n", var_NT, dim);

        if (global.backend == BE_distmem) {
            if (dc == C_distr) {
                /* Array is potentially distributed.
                 * SAC_ND_A_MIRROR_IS_DIST cannot be declared constant because the
                 * array may be treated as non-distributed later on. */
                indout (
                  "bool SAC_ND_A_MIRROR_IS_DIST( %s) = SAC_ND_A_DESC_IS_DIST( %s);\n",
                  var_NT, var_NT);
                indout ("size_t SAC_ND_A_MIRROR_FIRST_ELEMS( %s) = "
                        "SAC_ND_A_DESC_FIRST_ELEMS( %s);\n",
                        var_NT, var_NT);
                indout ("int SAC_ND_A_MIRROR_LOCAL_FROM( %s) = "
                        "SAC_DISTMEM_DET_LOCAL_FROM( %s);\n",
                        var_NT, var_NT);
                indout ("int SAC_ND_A_MIRROR_LOCAL_COUNT( %s) = "
                        "SAC_DISTMEM_DET_LOCAL_COUNT( %s);\n",
                        var_NT, var_NT);
                indout ("SAC_NT_CBASETYPE( %s) *SAC_ND_A_MIRROR_PTR_CACHE( %s) = NULL;\n",
                        var_NT, var_NT);
                indout ("int SAC_ND_A_MIRROR_PTR_CACHE_FROM( %s) = 0;\n", var_NT);
                indout ("int SAC_ND_A_MIRROR_PTR_CACHE_COUNT( %s) = 0;\n", var_NT);
                indout (
                  "uintptr_t SAC_ND_A_MIRROR_OFFS( %s) = SAC_ND_A_DESC_OFFS( %s);\n",
                  var_NT, var_NT);
            } else if (dc == C_distmem) {
                /* Array is potentially allocated in DSM memory. */
                indout ("bool SAC_ND_A_MIRROR_IS_DIST( %s) = "
                        "SAC_DISTMEM_DET_ALLOC_DSM_IN_DSM();\n",
                        var_NT);
                indout (
                  "uintptr_t SAC_ND_A_MIRROR_OFFS( %s) = SAC_ND_A_DESC_OFFS( %s);\n",
                  var_NT, var_NT);
            }
        }

        break;

    case C_akd:
        DBUG_ASSERT (dim >= 0, "illegal dimension found!");

        for (i = 0; i < dim; i++) {
            indout ("int SAC_ND_A_MIRROR_SHAPE( %s, %d) "
                    "= SAC_ND_A_DESC_SHAPE( %s, %d);\n",
                    var_NT, i, var_NT, i);
        }

        indout ("int SAC_ND_A_MIRROR_SIZE( %s)"
                " = SAC_ND_A_DESC_SIZE( %s);\n",
                var_NT, var_NT);
        indout ("const int SAC_ND_A_MIRROR_DIM( %s) = %d;\n", var_NT, dim);

        if (global.backend == BE_distmem) {
            if (dc == C_distr) {
                /* Array is potentially distributed. */
                indout (
                  "bool SAC_ND_A_MIRROR_IS_DIST( %s) = SAC_ND_A_DESC_IS_DIST( %s);\n",
                  var_NT, var_NT);
                indout ("size_t SAC_ND_A_MIRROR_FIRST_ELEMS( %s) = "
                        "SAC_ND_A_DESC_FIRST_ELEMS( %s);\n",
                        var_NT, var_NT);
                indout ("int SAC_ND_A_MIRROR_LOCAL_FROM( %s) = "
                        "SAC_DISTMEM_DET_LOCAL_FROM( %s);\n",
                        var_NT, var_NT);
                indout ("int SAC_ND_A_MIRROR_LOCAL_COUNT( %s) = "
                        "SAC_DISTMEM_DET_LOCAL_COUNT( %s);\n",
                        var_NT, var_NT);
                indout ("SAC_NT_CBASETYPE( %s) *SAC_ND_A_MIRROR_PTR_CACHE( %s) = NULL;\n",
                        var_NT, var_NT);
                indout ("int SAC_ND_A_MIRROR_PTR_CACHE_FROM( %s) = 0;\n", var_NT);
                indout ("int SAC_ND_A_MIRROR_PTR_CACHE_COUNT( %s) = 0;\n", var_NT);
                indout (
                  "uintptr_t SAC_ND_A_MIRROR_OFFS( %s) = SAC_ND_A_DESC_OFFS( %s);\n",
                  var_NT, var_NT);
            } else if (dc == C_distmem) {
                /* Array is potentially allocated in DSM memory. */
                indout ("bool SAC_ND_A_MIRROR_IS_DIST( %s) = "
                        "SAC_DISTMEM_DET_ALLOC_DSM_IN_DSM();\n",
                        var_NT);
                indout (
                  "uintptr_t SAC_ND_A_MIRROR_OFFS( %s) = SAC_ND_A_DESC_OFFS( %s);\n",
                  var_NT, var_NT);
            }
        }

        break;

    case C_aud:
        indout ("int SAC_ND_A_MIRROR_SIZE( %s)"
                " = SAC_ND_A_DESC_SIZE( %s);\n",
                var_NT, var_NT);
        indout ("int SAC_ND_A_MIRROR_DIM( %s)"
                " = SAC_ND_A_DESC_DIM( %s);\n",
                var_NT, var_NT);

        if (global.backend == BE_distmem) {
            if (dc == C_distr) {
                /* Array is potentially distributed. */
                indout (
                  "bool SAC_ND_A_MIRROR_IS_DIST( %s) = SAC_ND_A_DESC_IS_DIST( %s);\n",
                  var_NT, var_NT);
                indout ("size_t SAC_ND_A_MIRROR_FIRST_ELEMS( %s) = "
                        "SAC_ND_A_DESC_FIRST_ELEMS( %s);\n",
                        var_NT, var_NT);
                indout ("int SAC_ND_A_MIRROR_LOCAL_FROM( %s) = "
                        "SAC_DISTMEM_DET_LOCAL_FROM( %s);\n",
                        var_NT, var_NT);
                indout ("int SAC_ND_A_MIRROR_LOCAL_COUNT( %s) = "
                        "SAC_DISTMEM_DET_LOCAL_COUNT( %s);\n",
                        var_NT, var_NT);
                indout ("SAC_NT_CBASETYPE( %s) *SAC_ND_A_MIRROR_PTR_CACHE( %s) = NULL;\n",
                        var_NT, var_NT);
                indout ("int SAC_ND_A_MIRROR_PTR_CACHE_FROM( %s) = 0;\n", var_NT);
                indout ("int SAC_ND_A_MIRROR_PTR_CACHE_COUNT( %s) = 0;\n", var_NT);
                indout (
                  "uintptr_t SAC_ND_A_MIRROR_OFFS( %s) = SAC_ND_A_DESC_OFFS( %s);\n",
                  var_NT, var_NT);
            } else if (dc == C_distmem) {
                /* Array is potentially allocated in DSM memory. */
                indout (
                  "uintptr_t SAC_ND_A_MIRROR_OFFS( %s) = SAC_ND_A_DESC_OFFS( %s);\n",
                  var_NT, var_NT);
                indout ("bool SAC_ND_A_MIRROR_IS_DIST( %s) = "
                        "SAC_DISTMEM_DET_ALLOC_DSM_IN_DSM();\n",
                        var_NT);
            }
        }

        break;

    default:
        DBUG_UNREACHABLE ("Unknown shape class found!");
        break;
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_DECL__MIRROR_EXTERN( char *var_NT, int sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_DECL__MIRROR_EXTERN( var_NT, sdim)
 *
 ******************************************************************************/

/* TODO: Does this require changes for distributed memory backend? */
void
ICMCompileND_DECL__MIRROR_EXTERN (char *var_NT, int sdim)
{
    int i;
    shape_class_t sc = ICUGetShapeClass (var_NT);
    int dim = DIM_NO_OFFSET (sdim);

    DBUG_ENTER ();

#define ND_DECL__MIRROR_EXTERN
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_DECL__MIRROR_EXTERN

    switch (sc) {
    case C_scl:
        indout ("SAC_NOTHING()\n");
        break;

    case C_aks:
        DBUG_ASSERT (dim >= 0, "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            indout ("extern const int SAC_ND_A_MIRROR_SHAPE( %s, %d);\n", var_NT, i);
        }

        indout ("extern const int SAC_ND_A_MIRROR_SIZE( %s);\n", var_NT);
        indout ("extern const int SAC_ND_A_MIRROR_DIM( %s);\n", var_NT);
        break;

    case C_akd:
        DBUG_ASSERT (dim >= 0, "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            indout ("extern int SAC_ND_A_MIRROR_SHAPE( %s, %d);\n", var_NT, i);
        }

        indout ("extern int SAC_ND_A_MIRROR_SIZE( %s);\n", var_NT);
        indout ("extern const int SAC_ND_A_MIRROR_DIM( %s);\n", var_NT);
        break;

    case C_aud:
        indout ("extern int SAC_ND_A_MIRROR_SIZE( %s);\n", var_NT);
        indout ("extern int SAC_ND_A_MIRROR_DIM( %s);\n", var_NT);
        break;

    default:
        DBUG_UNREACHABLE ("Unknown shape class found!");
        break;
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_CHECK_REUSE( char *to_NT, int to_sdim,
 *                                  char *from_NT, int from_sdim,
 *                                  char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_CHECK_REUSE( to_NT, to_sdim, from_NT, from_sdim, copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_CHECK_REUSE (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                          char *copyfun)
{
    shape_class_t to_sc = ICUGetShapeClass (to_NT);

    DBUG_ENTER ();

#define ND_CHECK_REUSE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_CHECK_REUSE

    if (to_sc == C_scl) {
        indout ("SAC_NOOP()\n");
    } else {
        if (global.backend == BE_distmem
            && ICUGetDistributedClass (to_NT) != ICUGetDistributedClass (from_NT)) {

            indout ("SAC_TR_DISTMEM_PRINT( \"Cannot reuse memory, distributability does "
                    "not match (from %%s: %s, to %%s: %s).\","
                    "%s, %s);\n",
                    global.nt_distributed_string[ICUGetDistributedClass (from_NT)],
                    global.nt_distributed_string[ICUGetDistributedClass (to_NT)], from_NT,
                    to_NT);
        } else {
            indout ("SAC_IS_LASTREF__BLOCK_BEGIN( %s)\n", from_NT);

            global.indent++;
            ICMCompileND_ASSIGN (to_NT, to_sdim, from_NT, from_sdim, copyfun);
            indout ("SAC_TR_MEM_PRINT("
                    " (\"reuse memory of %s at %%p for %s\","
                    " SAC_ND_GETVAR( %s, SAC_ND_A_FIELD( %s))))\n",
                    from_NT, to_NT, from_NT, from_NT);
            indout ("SAC_PF_MEM_INC_REUSE()");
            global.indent--;

            indout ("SAC_IS_LASTREF__BLOCK_END( %s)\n", from_NT);
            indout ("else\n");
        }
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_SET__SHAPE_id( char *to_NT, int to_sdim, char *shp_NT)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_SET__SHAPE_id( to_NT, to_sdim, shp_NT )
 *
 ******************************************************************************/

void
ICMCompileND_SET__SHAPE_id (char *to_NT, int to_sdim, char *shp_NT)
{
    DBUG_ENTER ();

#define ND_SET__SHAPE_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_SET__SHAPE_id

    Set_Shape (to_NT, to_sdim, shp_NT, -1, SizeId, NULL, ReadId, NULL, 0, NULL, NULL,
               NULL);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_SET__SHAPE_arr( char *to_NT, int dim, char **shp_ANY)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_SET__SHAPE_arr( to_NT, dim, [ shp_ANY ]* )
 *
 ******************************************************************************/

void
ICMCompileND_SET__SHAPE_arr (char *to_NT, int dim, char **shp_ANY)
{
    DBUG_ENTER ();

#define ND_SET__SHAPE_arr
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_SET__SHAPE_arr

    /*
     * CAUTION:
     * 'shp_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */
    Set_Shape (to_NT, dim, shp_ANY, dim, NULL, NULL, ReadConstArray_Str, NULL, 0, NULL,
               NULL, NULL);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_REFRESH__MIRROR( char *var_NT, int sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_REFRESH__MIRROR( var_NT, sdim)
 *
 *
 *   This is used for the results of a function call.
 *
 ******************************************************************************/

void
ICMCompileND_REFRESH__MIRROR (char *var_NT, int sdim)
{
    shape_class_t sc = ICUGetShapeClass (var_NT);
    distributed_class_t dc = ICUGetDistributedClass (var_NT);
    int dim = DIM_NO_OFFSET (sdim);

    DBUG_ENTER ();

#define ND_REFRESH__MIRROR
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_REFRESH__MIRROR

    switch (sc) {
    case C_scl:
        indout ("SAC_NOOP()\n");
        break;

    case C_aks:
        if (global.backend == BE_distmem) {
            if (dc == C_distr) {
                /* Array is potentially distributed. */
                /* The array is AKS, so we know the size. Still, it is possibly not
                 * distributed for another reason (allocated in distributed execution
                 * mode). */
                indout ("SAC_ND_A_MIRROR_IS_DIST( %s) = SAC_ND_A_DESC_IS_DIST( %s);\n",
                        var_NT, var_NT);
                indout (
                  "SAC_ND_A_MIRROR_FIRST_ELEMS( %s) = SAC_ND_A_DESC_FIRST_ELEMS( %s);\n",
                  var_NT, var_NT);
                indout (
                  "SAC_ND_A_MIRROR_LOCAL_FROM( %s) = SAC_DISTMEM_DET_LOCAL_FROM( %s);\n",
                  var_NT, var_NT);
                indout ("SAC_ND_A_MIRROR_LOCAL_COUNT( %s) = SAC_DISTMEM_DET_LOCAL_COUNT( "
                        "%s);\n",
                        var_NT, var_NT);
                indout ("SAC_ND_A_MIRROR_PTR_CACHE_FROM( %s) = 0;\n", var_NT);
                indout ("SAC_ND_A_MIRROR_PTR_CACHE_COUNT( %s) = 0;\n", var_NT);
                indout ("SAC_ND_A_MIRROR_OFFS( %s) = SAC_ND_A_DESC_OFFS( %s);\n", var_NT,
                        var_NT);
            } else if (dc == C_distmem) {
                /* Array is potentially allocated in DSM memory. */
                indout ("SAC_ND_A_MIRROR_OFFS( %s) = SAC_ND_A_DESC_OFFS( %s);\n", var_NT,
                        var_NT);
                indout (
                  "SAC_ND_A_MIRROR_IS_DIST( %s) = SAC_DISTMEM_DET_ALLOC_DSM_IN_DSM();\n",
                  var_NT);
            }
        } else {
            indout ("SAC_NOOP()\n");
        }
        break;

    case C_akd:
        DBUG_ASSERT (dim >= 0, "illegal dimension found!");
        for (int i = 0; i < dim; i++) {
            indout ("SAC_ND_A_MIRROR_SHAPE( %s, %d) "
                    "= SAC_ND_A_DESC_SHAPE( %s, %d);\n",
                    var_NT, i, var_NT, i);
        }

        indout ("SAC_ND_A_MIRROR_SIZE( %s)"
                " = SAC_ND_A_DESC_SIZE( %s);\n",
                var_NT, var_NT);

        if (global.backend == BE_distmem) {
            if (dc == C_distr) {
                /* Array is potentially distributed. */
                indout ("SAC_ND_A_MIRROR_IS_DIST( %s) = SAC_ND_A_DESC_IS_DIST( %s);\n",
                        var_NT, var_NT);
                indout (
                  "SAC_ND_A_MIRROR_FIRST_ELEMS( %s) = SAC_ND_A_DESC_FIRST_ELEMS( %s);\n",
                  var_NT, var_NT);
                indout (
                  "SAC_ND_A_MIRROR_LOCAL_FROM( %s) = SAC_DISTMEM_DET_LOCAL_FROM( %s);\n",
                  var_NT, var_NT);
                indout ("SAC_ND_A_MIRROR_LOCAL_COUNT( %s) = SAC_DISTMEM_DET_LOCAL_COUNT( "
                        "%s);\n",
                        var_NT, var_NT);
                indout ("SAC_ND_A_MIRROR_PTR_CACHE_FROM( %s) = 0;\n", var_NT);
                indout ("SAC_ND_A_MIRROR_PTR_CACHE_COUNT( %s) = 0;\n", var_NT);
                indout ("SAC_ND_A_MIRROR_OFFS( %s) = SAC_ND_A_DESC_OFFS( %s);\n", var_NT,
                        var_NT);
            } else if (dc == C_distmem) {
                /* Array is potentially allocated in DSM memory. */
                indout ("SAC_ND_A_MIRROR_OFFS( %s) = SAC_ND_A_DESC_OFFS( %s);\n", var_NT,
                        var_NT);
                indout (
                  "SAC_ND_A_MIRROR_IS_DIST( %s) = SAC_DISTMEM_DET_ALLOC_DSM_IN_DSM();\n",
                  var_NT);
            }
        }
        break;

    case C_aud:
        indout ("SAC_ND_A_MIRROR_SIZE( %s)"
                " = SAC_ND_A_DESC_SIZE( %s);\n",
                var_NT, var_NT);
        indout ("SAC_ND_A_MIRROR_DIM( %s)"
                " = SAC_ND_A_DESC_DIM( %s);\n",
                var_NT, var_NT);

        if (global.backend == BE_distmem) {
            if (dc == C_distr) {
                /* Array is potentially distributed. */
                indout ("SAC_ND_A_MIRROR_IS_DIST( %s) = SAC_ND_A_DESC_IS_DIST( %s);\n",
                        var_NT, var_NT);
                indout (
                  "SAC_ND_A_MIRROR_FIRST_ELEMS( %s) = SAC_ND_A_DESC_FIRST_ELEMS( %s);\n",
                  var_NT, var_NT);
                indout (
                  "SAC_ND_A_MIRROR_LOCAL_FROM( %s) = SAC_DISTMEM_DET_LOCAL_FROM( %s);\n",
                  var_NT, var_NT);
                indout ("SAC_ND_A_MIRROR_LOCAL_COUNT( %s) = SAC_DISTMEM_DET_LOCAL_COUNT( "
                        "%s);\n",
                        var_NT, var_NT);
                indout ("SAC_ND_A_MIRROR_PTR_CACHE_FROM( %s) = 0;\n", var_NT);
                indout ("SAC_ND_A_MIRROR_PTR_CACHE_COUNT( %s) = 0;\n", var_NT);
                indout ("SAC_ND_A_MIRROR_OFFS( %s) = SAC_ND_A_DESC_OFFS( %s);\n", var_NT,
                        var_NT);
            } else if (dc == C_distmem) {
                /* Array is potentially allocated in DSM memory. */
                indout ("SAC_ND_A_MIRROR_OFFS( %s) = SAC_ND_A_DESC_OFFS( %s);\n", var_NT,
                        var_NT);
                indout (
                  "SAC_ND_A_MIRROR_IS_DIST( %s) = SAC_DISTMEM_DET_ALLOC_DSM_IN_DSM();\n",
                  var_NT);
            }
        }
        break;

    default:
        DBUG_UNREACHABLE ("Unknown shape class found!");
        break;
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_ASSIGN( char *to_NT, int to_sdim,
 *                             char *from_NT, int from_sdim,
 *                             char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_ASSIGN( to_NT, to_sdim, from_NT, from_sdim, copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_ASSIGN (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                     char *copyfun)
{
    DBUG_ENTER ();

#define ND_ASSIGN
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_ASSIGN

    ICMCompileND_ASSIGN__SHAPE (to_NT, to_sdim, from_NT, from_sdim);
    indout ("SAC_ND_ASSIGN__DATA( %s, %s, %s)\n", to_NT, from_NT, copyfun);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_COPY__DESC_DIS_FIELDS( char *to_NT, char *from_NT)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_ND_COPY__DESC_DIS_FIELDS( to_NT, from_NT)
 *
 *   This ICM is used by reshape.
 *   When the new shape has more dimensions than the old one, a new descriptor is
 *assigned. When the distributed memory backend is used, the descriptor also contains
 *information to locate the data. We have to make sure that that does not get lost!
 *   from_NT is the array with the new descriptor, to_NT is the array with the old
 *descriptor
 *
 ******************************************************************************/

void
ICMCompileND_COPY__DESC_DIS_FIELDS (char *to_NT, char *from_NT)
{
    distributed_class_t to_dc = ICUGetDistributedClass (to_NT);
    distributed_class_t from_dc = ICUGetDistributedClass (from_NT);

    DBUG_ENTER ();

    DBUG_ASSERT (to_dc == from_dc, "Illegal assignment found!");

#define ND_COPY__DESC_DIS_FIELDS
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_COPY__DESC_DIS_FIELDS

    if (to_dc == C_distr) {
        indout ("SAC_ND_A_DESC_IS_DIST( %s) = SAC_ND_A_MIRROR_IS_DIST( %s) = "
                "SAC_ND_A_IS_DIST( %s);\n",
                from_NT, from_NT, to_NT);
        indout ("SAC_ND_A_DESC_FIRST_ELEMS( %s) = SAC_ND_A_MIRROR_FIRST_ELEMS( %s) = "
                "SAC_ND_A_FIRST_ELEMS( %s);\n",
                from_NT, from_NT, to_NT);
        indout (
          "SAC_ND_A_DESC_OFFS( %s) = SAC_ND_A_MIRROR_OFFS( %s) = SAC_ND_A_OFFS( %s);\n",
          from_NT, from_NT, to_NT);

        FOR_LOOP_BEGIN ("int SAC_r = 0; SAC_r < SAC_DISTMEM_size; SAC_r++")
            ;
            indout ("SAC_ND_A_DESC_PTR( %s, SAC_r) = SAC_ND_A_DESC_PTR( %s, SAC_r);\n",
                    from_NT, to_NT);
        FOR_LOOP_END ();
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_ASSIGN__DESC( char *to_NT, char *from_NT)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_ASSIGN__DESC( to_NT, from_NT)
 *
 ******************************************************************************/

void
ICMCompileND_ASSIGN__DESC (char *to_NT, char *from_NT)
{
    shape_class_t to_sc = ICUGetShapeClass (to_NT);
    hidden_class_t to_hc = ICUGetHiddenClass (to_NT);
    unique_class_t to_uc = ICUGetUniqueClass (to_NT);
    shape_class_t from_sc = ICUGetShapeClass (from_NT);
    hidden_class_t from_hc = ICUGetHiddenClass (from_NT);
    unique_class_t from_uc = ICUGetUniqueClass (from_NT);

    bool to_has_desc, from_has_desc;

    DBUG_ENTER ();

#define ND_ASSIGN__DESC
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_ASSIGN__DESC

    DBUG_ASSERT (to_hc == from_hc, "Illegal assignment found!");

    to_has_desc = to_sc != C_scl || (to_hc == C_hid && to_uc != C_unq);

    from_has_desc = from_sc != C_scl || (from_hc == C_hid && from_uc != C_unq);

    /* FIXME This is hard to read, and it may be simplified.  */
    if (!to_has_desc && !from_has_desc) {
        /* 'to_NT' has no desc, 'from_NT' has no desc */
        indout ("SAC_NOOP()\n");
    } else if (!to_has_desc && from_has_desc) {

        /* 'to_NT' has no desc, 'from_NT' has a desc */
        if (to_hc != C_hid) {
            /*
             * -> 'to_NT' is a non-hidden scalar
             * -> 'from_NT' is a non-hidden array
             *     -> descriptor / data vector of 'from_NT' are not reused by 'to_NT'
             *         -> ND_DEC_RC_FREE( from_NT) in ND_ASSIGN__DATA
             */
            indout ("SAC_NOOP()\n");
        } else {
            /*
             * -> 'to_NT' is a unique hidden scalar
             * -> 'from_NT' is hidden
             * -> RC of 'from_NT' is 1 (otherwise ND_COPY is used)
             *     -> descriptor of 'from_NT' is not reused by 'to_NT'
             *     -> content of data vector of 'from_NT' is reused by 'to_NT'
             *         -> data vector of 'from_NT' (without content) is removed
             *            in ND_ASSIGN_DATA
             *         -> descriptor is removed here
             */
            indout ("SAC_ND_FREE__DESC( %s)\n", from_NT);
        }
    } else if (to_has_desc && (!from_has_desc)) {

        /* 'to_NT' has a desc, 'from_NT' has no desc */
        indout ("SAC_ND_ALLOC__DESC( %s, 0)\n", to_NT);
        indout ("SAC_ND_SET__RC( %s, 1)\n", to_NT);
    } else {

        /* 'to_NT' has a desc, 'from_NT' has a desc */
        if (((to_sc == C_scl && from_sc != C_scl) || (to_sc != C_scl && from_sc == C_scl))
            && from_uc == C_nuq) {
            /*
             * -> 'to_NT' and 'from_NT' are neither both scalars nor both arrays
             * -> 'from_NT' is a non-unique hidden
             *     -> descriptor / data vector of 'from_NT' cannot be reused by 'to_NT'
             *         -> ND_ALLOC__DESC( to_NT, 0)
             *         -> ND_DEC_RC_FREE( from_NT) in ND_ASSIGN__DATA
             */
            indout ("SAC_ND_ALLOC__DESC( %s, 0)\n", to_NT);
        } else {
            indout ("SAC_ND_A_DESC( %s) = SAC_ND_A_DESC( %s);\n", to_NT, from_NT);
        }
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_ASSIGN__SHAPE( char *to_NT, int to_sdim,
 *                                    char *from_NT, int from_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_ASSIGN__SHAPE( to_NT, to_sdim, from_NT, from_sdim)
 *
 ******************************************************************************/

void
ICMCompileND_ASSIGN__SHAPE (char *to_NT, int to_sdim, char *from_NT, int from_sdim)
{
    int from_dim = DIM_NO_OFFSET (from_sdim);

    DBUG_ENTER ();

    DBUG_PRINT ("create shape assign of %s to %s", to_NT, from_NT);

#define ND_ASSIGN__SHAPE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_ASSIGN__SHAPE

    /* Check constant part of mirror.  */
    Check_Mirror (to_NT, to_sdim, from_NT, from_dim, DimId, ShapeId, NULL, 0, NULL, NULL);

    ICMCompileND_ASSIGN__DESC (to_NT, from_NT);

    /* Assign non-constant part of mirror.  */
    ICMCompileND_UPDATE__MIRROR (to_NT, to_sdim, from_NT, from_sdim);

    /* Assign missing descriptor entries.  */
    ICMCompileND_UPDATE__DESC (to_NT, to_sdim, from_NT, from_sdim);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_UPDATE__DESC( char *to_NT, int to_sdim,
 *                                   char *from_NT, int from_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_UPDATE__DESC( to_NT, to_sdim, from_NT, from_sdim)
 *
 *   This is used for assignments.
 *
 ******************************************************************************/

void
ICMCompileND_UPDATE__DESC (char *to_NT, int to_sdim, char *from_NT, int from_sdim)
{
    int i;
    shape_class_t to_sc = ICUGetShapeClass (to_NT);
    shape_class_t from_sc = ICUGetShapeClass (from_NT);
    int from_dim = DIM_NO_OFFSET (from_sdim);

    DBUG_ENTER ();

#define ND_UPDATE__DESC
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_UPDATE__DESC

    /* Assign missing descriptor entries.  */
    switch (to_sc) {
    case C_scl:
    case C_aks:
        indout ("SAC_NOOP()\n");
        break;

    case C_akd:
        switch (from_sc) {
        case C_aks:
            DBUG_ASSERT (from_dim >= 0, "illegal dimension found!");
            for (i = 0; i < from_dim; i++) {
                indout ("SAC_ND_A_DESC_SHAPE( %s, %d)"
                        " = SAC_ND_A_SHAPE( %s, %d);\n",
                        to_NT, i, from_NT, i);
            }

            indout ("SAC_ND_A_DESC_SIZE( %s) = SAC_ND_A_SIZE( %s);\n", to_NT, from_NT);
            break;

        case C_akd:
        case C_aud:
            indout ("SAC_NOOP()\n");
            break;

        default:
            DBUG_UNREACHABLE ("Illegal assignment found!");
            break;
        }
        break;

    case C_aud:
        switch (from_sc) {
        case C_scl:
        case C_aks:
            DBUG_ASSERT (from_dim >= 0, "illegal dimension found!");
            for (i = 0; i < from_dim; i++) {
                indout ("SAC_ND_A_DESC_SHAPE( %s, %d)"
                        " = SAC_ND_A_SHAPE( %s, %d);\n",
                        to_NT, i, from_NT, i);
            }

            indout ("SAC_ND_A_DESC_SIZE( %s) = SAC_ND_A_SIZE( %s);\n", to_NT, from_NT);
            indout ("SAC_ND_A_DESC_DIM( %s) = SAC_ND_A_DIM( %s);\n", to_NT, from_NT);
            break;

        case C_akd:
            indout ("SAC_ND_A_DESC_DIM( %s) = SAC_ND_A_DIM( %s);\n", to_NT, from_NT);
            break;

        case C_aud:
            indout ("SAC_NOOP()\n");
            break;

        default:
            DBUG_UNREACHABLE ("Illegal assignment found!");
            break;
        }
        break;

    default:
        DBUG_UNREACHABLE ("Unknown shape class found!");
        break;
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_UPDATE__MIRROR( char *to_NT, int to_sdim,
 *                                     char *from_NT, int from_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_UPDATE__MIRROR( to_NT, to_sdim, from_NT, from_sdim)
 *
 *
 *   This is used for assignments.
 *
 ******************************************************************************/

void
ICMCompileND_UPDATE__MIRROR (char *to_NT, int to_sdim, char *from_NT, int from_sdim)
{
    shape_class_t to_sc = ICUGetShapeClass (to_NT);
    int to_dim = DIM_NO_OFFSET (to_sdim);
    distributed_class_t to_dc = ICUGetDistributedClass (to_NT);

    DBUG_ENTER ();

#define ND_UPDATE__MIRROR
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_UPDATE__MIRROR

    /* Initialize non-constant mirror variables.  */
    switch (to_sc) {
    case C_scl:
    case C_aks:
        indout ("SAC_NOOP()\n");

        break;

    case C_akd:
        DBUG_ASSERT (to_dim >= 0, "illegal dimension found!");
        for (int i = 0; i < to_dim; i++) {
            indout ("SAC_ND_A_MIRROR_SHAPE( %s, %d) = "
                    "SAC_ND_A_SHAPE( %s, %d);\n",
                    to_NT, i, from_NT, i);
        }

        indout ("SAC_ND_A_MIRROR_SIZE( %s) = SAC_ND_A_SIZE( %s);\n", to_NT, from_NT);

        if (global.backend == BE_distmem && to_dc == C_distr) {
            /* Target is potentially distributed. */
            indout ("SAC_ND_A_DESC_IS_DIST( %s) = SAC_ND_A_MIRROR_IS_DIST( %s) = "
                    "SAC_ND_A_IS_DIST( %s);\n",
                    to_NT, to_NT, from_NT);
        }

        break;

    case C_aud:
        indout ("SAC_ND_A_MIRROR_SIZE( %s) = SAC_ND_A_SIZE( %s);\n", to_NT, from_NT);
        indout ("SAC_ND_A_MIRROR_DIM( %s) = SAC_ND_A_DIM( %s);\n", to_NT, from_NT);

        if (global.backend == BE_distmem && to_dc == C_distr) {
            /* Target is potentially distributed. */
            indout ("SAC_ND_A_DESC_IS_DIST( %s) = SAC_ND_A_MIRROR_IS_DIST( %s) = "
                    "SAC_ND_A_IS_DIST( %s);\n",
                    to_NT, to_NT, from_NT);
        }

        break;

    default:
        DBUG_UNREACHABLE ("Unknown shape class found!");
        break;
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_COPY( char *to_NT, int to_sdim,
 *                           char *from_NT, int from_sdim,
 *                           char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_COPY( to_NT, to_sdim, from_NT, from_sdim, copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_COPY (char *to_NT, int to_sdim, char *from_NT, int from_sdim, char *basetype,
                   char *copyfun)
{
    DBUG_ENTER ();

#define ND_COPY
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_COPY

    /* Allocate descriptor.  */
    indout ("SAC_ND_ALLOC_BEGIN( %s, 1, SAC_ND_A_DIM( %s), %s)\n", to_NT, from_NT,
            basetype);

    /* Copy descriptor entries and non-constant part of mirror.  */
    ICMCompileND_COPY__SHAPE (to_NT, to_sdim, from_NT, from_sdim);

    indout ("SAC_ND_ALLOC_END( %s, 1, SAC_ND_A_DIM( %s), %s)\n", to_NT, from_NT,
            basetype);
    indout ("SAC_ND_COPY__DATA( %s, %s, %s)\n", to_NT, from_NT, copyfun);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_COPY__SHAPE( char *to_NT, int to_sdim,
 *                                  char *from_NT, int from_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_COPY__SHAPE( to_NT, to_sdim, from_NT, from_sdim)
 *
 ******************************************************************************/

void
ICMCompileND_COPY__SHAPE (char *to_NT, int to_sdim, char *from_NT, int from_sdim)
{
    int from_dim = DIM_NO_OFFSET (from_sdim);

    DBUG_ENTER ();

#define ND_COPY__SHAPE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_COPY__SHAPE

    Set_Shape (to_NT, to_sdim, from_NT, from_dim, DimId, SizeId, ShapeId, NULL, 0, NULL,
               NULL, NULL);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_MAKE_UNIQUE( char *to_NT, int to_sdim,
 *                                  char *from_NT, int from_sdim,
 *                                  char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_MAKE_UNIQUE( to_NT, to_sdim, from_NT, from_sdim, copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_MAKE_UNIQUE (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                          char *basetype, char *copyfun)
{
    DBUG_ENTER ();

#define ND_MAKE_UNIQUE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_MAKE_UNIQUE

    indout ("SAC_TR_MEM_PRINT( (\"ND_MAKE_UNIQUE( %s, %d, %s, %d, %s)\"))\n", to_NT,
            to_sdim, from_NT, from_sdim, copyfun);

    indout ("SAC_TR_REF_PRINT_RC( %s)\n", from_NT);
    indout ("SAC_IS_LASTREF__BLOCK_BEGIN( %s)\n", from_NT);
    global.indent++;
    indout ("SAC_TR_MEM_PRINT( (\"%s is already unique.\"))\n", from_NT);
    ICMCompileND_ASSIGN (to_NT, to_sdim, from_NT, from_sdim, copyfun);
    global.indent--;

    indout ("SAC_IS_LASTREF__BLOCK_ELSE( %s)\n", from_NT);
    global.indent++;
    ICMCompileND_COPY (to_NT, to_sdim, from_NT, from_sdim, basetype, copyfun);
    indout ("SAC_ND_DEC_RC( %s, 1)\n", from_NT);
    global.indent--;

    indout ("SAC_IS_LASTREF__BLOCK_END( %s)\n", from_NT);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_CREATE__ARRAY__SHAPE( char *to_NT, int to_sdim,
 *                                           int dim, int *shp,
 *                                           int val_size, char **vals_ANY,
 *                                           int val0_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_CREATE__ARRAY__SHAPE( to_NT, to_sdim, dim, [ shp ]* ,
 *                            val_size, [ vals_ANY ]* , val0_sdim )
 *
 *   dim: top-level dimension
 *   shp: top-level shape
 *           [a,b,c,d]       ->   dim == 1, shp == [4]
 *           [[a,b],[c,d]]   ->   dim == 2, shp == [2,2]
 *   val_size: size of data vector
 *   vals_ANY: data vector
 *   val0_sdim: shape-encoded dimension of the (first) data vector element(s)
 *
 ******************************************************************************/

void
ICMCompileND_CREATE__ARRAY__SHAPE (char *to_NT, int to_sdim, int dim, int *shp,
                                   int val_size, char **vals_ANY, int val0_sdim)
{
    bool entries_are_scalars;
    int i;
    shape_class_t to_sc = ICUGetShapeClass (to_NT);

    int val0_dim = DIM_NO_OFFSET (val0_sdim);

    DBUG_ENTER ();

#define ND_CREATE__ARRAY__SHAPE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_CREATE__ARRAY__SHAPE

    /*
     * CAUTION:
     * 'vals_ANY[i]' is either a tagged identifier (scalar or non-scalar)
     * or a constant scalar!
     */
    entries_are_scalars = FALSE;
    for (i = 0; i < val_size; i++) {
        /* Not a tagged id -> is a const scalar! */
        if (vals_ANY[i][0] != '(' || ICUGetShapeClass (vals_ANY[i]) == C_scl) {
            entries_are_scalars = TRUE;
        }
    }

    if (val_size <= 0) {
        /* 'A = []' works only for arrays with known dimension/shape!  */
        DBUG_ASSERT (to_sc == C_aks, "[] with unknown shape found!");
    } else if (entries_are_scalars) {

        /* FIXME Consider using `asprintf` here.  */
        char **shp_str = (char **)MEMmalloc (dim * sizeof (char *));
        for (i = 0; i < dim; i++) {
            shp_str[i] = (char *)MEMmalloc (20 * sizeof (char));
            sprintf (shp_str[i], "%d", shp[i]);
        }

        ICMCompileND_SET__SHAPE_arr (to_NT, dim, shp_str);
        for (i = 0; i < dim; i++) {
            shp_str[i] = MEMfree (shp_str[i]);
        }
        shp_str = MEMfree (shp_str);
    } else {

        /* 'vals_ANY[i]' is a tagged identifier */

        /* Check whether all entries have identical shape.  */
        for (i = 1; i < val_size; i++) {
            ASSURE_TYPE (ASSURE_COND ("SAC_ND_A_DIM( %s) == SAC_ND_A_DIM( %s)",
                                      vals_ANY[i], vals_ANY[0]),
                         ASSURE_TEXT ("Inconsistent vector found:"
                                      " First entry and entry at position %d have"
                                      " different dimension!",
                                      i));

            ASSURE_TYPE (ASSURE_COND ("SAC_ND_A_SIZE( %s) == SAC_ND_A_SIZE( %s)",
                                      vals_ANY[i], vals_ANY[0]),
                         ASSURE_TEXT ("Inconsistent vector found:"
                                      " First entry and entry at position %d have"
                                      " different size!",
                                      i));

            if (val0_dim >= 0) {
                for (int d = 0; d < val0_dim; d++) {
                    ASSURE_TYPE (ASSURE_COND ("SAC_ND_A_SHAPE( %s, %d) "
                                              "== SAC_ND_A_SHAPE( %s, %d)",
                                              vals_ANY[i], d, vals_ANY[0], d),
                                 ASSURE_TEXT ("Inconsistent vector found: First entry "
                                              "and entry at position %d have different "
                                              "shape component %d!",
                                              i, d));
                }
            } else {
                FOR_LOOP_BEGIN ("int SAC_d = 0; SAC_d < SAC_ND_A_DIM( %s);"
                                " SAC_d++",
                                vals_ANY[0])
                    ;
                    ASSURE_TYPE (ASSURE_COND ("SAC_ND_A_SHAPE( %s, SAC_d) "
                                              "== SAC_ND_A_SHAPE( %s, SAC_d)",
                                              vals_ANY[i], vals_ANY[0]),
                                 ASSURE_TEXT ("Inconsistent vector found: First entry "
                                              "and entry at position %d have different "
                                              "shape!",
                                              i));
                FOR_LOOP_END ();
            }
        }

        Set_Shape (to_NT, to_sdim, shp, dim, NULL, NULL, ReadConstArray_Num, vals_ANY[0],
                   val0_dim, DimId, SizeId, ShapeId);
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_CREATE__ARRAY__DATA( char *to_NT, int to_sdim,
 *                                          int val_size, char **vals_ANY,
 *                                          char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_CREATE__ARRAY__DATA( to_NT, to_sdim, val_size, [ vals_ANY ]* , copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_CREATE__ARRAY__DATA (char *to_NT, int to_sdim, int val_size, char **vals_ANY,
                                  char *copyfun)
{
    bool entries_are_scalars;
    int i;

    DBUG_ENTER ();

#define ND_CREATE__ARRAY__DATA
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_CREATE__ARRAY__DATA

    /*
     * CAUTION:
     * 'vals_ANY[i]' is either a tagged identifier (scalar or non-scalar)
     * or a constant scalar!
     */
    entries_are_scalars = FALSE;
    for (i = 0; i < val_size; i++) {
        /* not a tagged id -> is a const scalar! */
        if (vals_ANY[i][0] != '(' || ICUGetShapeClass (vals_ANY[i]) == C_scl) {
            entries_are_scalars = TRUE;
        }
    }

    if (global.backend == BE_distmem && ICUGetDistributedClass (to_NT) == C_distr) {
        /* The target array is distributable. */
        IF_BEGIN ("SAC_ND_A_IS_DIST( %s)", to_NT)
            ;
            /*
             * The target array is actually distributed.
             * Make sure that the value array is fully loaded into the cache
             * before the writing starts.
             */
            indout ("SAC_DISTMEM_ASSURE_IN_CACHE ( SAC_ND_A_OFFS( %s), SAC_NT_CBASETYPE( "
                    "%s), SAC_ND_A_FIRST_ELEMS( %s), 0, SAC_ND_A_SIZE( %s));\n",
                    to_NT, to_NT, to_NT, to_NT);
            indout ("SAC_DISTMEM_BARRIER();\n");

            /*
             * This write is performed by every node.
             * If a node is not the owner, it writes directly
             * into its local cache.
             * Temporarily allow writing
             * distributed arrays (local and local cache).
             */
            indout ("SAC_DISTMEM_ALLOW_CACHE_WRITES();\n");
        IF_END ();
    }

    if (entries_are_scalars) {

        /* ensure all entries are scalar */
        for (i = 0; i < val_size; i++) {
            if (vals_ANY[i][0] == '(' && ICUGetShapeClass (vals_ANY[i]) != C_scl) {
                DBUG_ASSERT (ICUGetShapeClass (vals_ANY[i]) == C_aud,
                             "tagged id is no scalar!");

                ASSURE_TYPE (ASSURE_COND ("SAC_ND_A_DIM( %s) == 0", vals_ANY[i]),
                             ASSURE_TEXT ("Scalar expected but array "
                                          "with (dim > 0) found!"));
            }
        }

        for (i = 0; i < val_size; i++) {

            if (global.backend == BE_distmem
                && ICUGetDistributedClass (to_NT) == C_distr) {
                /* The target array is distributable. */
                IF_BEGIN ("SAC_ND_A_IS_DIST( %s)", to_NT)
                    ;
                    /* The target array is actually distributed. Allow cache writes. */
                    indout ("SAC_ND_WRITE_COPY( SAC_SET_NT_DIS( DCA, %s), %d, ", to_NT,
                            i);
                    ReadScalar (vals_ANY[i], NULL, 0);
                    out (", %s)\n", copyfun);
                IF_END ();
                ELSE_BEGIN ()
                    ;
                    /* The target array is not distributed. Do nothing special. */
                    indout ("SAC_ND_WRITE_COPY( %s, %d, ", to_NT, i);
                    ReadScalar (vals_ANY[i], NULL, 0);
                    out (", %s)\n", copyfun);
                ELSE_END ();
            } else {
                /* The target array is not distributable. Do nothing special. */
                indout ("SAC_ND_WRITE_COPY( %s, %d, ", to_NT, i);
                ReadScalar (vals_ANY[i], NULL, 0);
                out (", %s)\n", copyfun);
            }
        }
    } else {

        /* 'vals_ANY[i]' is a tagged identifier */
        if (val_size > 0) {

            BLOCK_BEGIN ("int SAC_j, SAC_i = 0;")
                ;
                for (i = 0; i < val_size; i++) {
                    /* check whether all entries have identical size */
                    if (i > 0) {
                        ASSURE_TYPE (
                          ASSURE_COND ("SAC_ND_A_SIZE( %s) == SAC_ND_A_SIZE( %s)",
                                       vals_ANY[i], vals_ANY[0]),
                          ASSURE_TEXT ("Inconsistent vector found: First entry "
                                       "and entry at position %d have different "
                                       "size!",
                                       i));
                    }

                    FOR_LOOP_BEGIN ("SAC_j = 0; SAC_j < SAC_ND_A_SIZE( %s); SAC_j++",
                                    vals_ANY[i])
                        ;

                        if (global.backend == BE_distmem
                            && ICUGetDistributedClass (to_NT) == C_distr) {
                            /* The target array is distributable. */
                            IF_BEGIN ("SAC_ND_A_IS_DIST( %s)", to_NT)
                                ;
                                /* The target array is actually distributed. Allow cache
                                 * writes. */
                                indout ("SAC_ND_WRITE_READ_COPY( SAC_SET_NT_DIS( DCA, "
                                        "%s), SAC_i, %s, SAC_j, %s)\n",
                                        to_NT, vals_ANY[i], copyfun);
                            IF_END ();
                            ELSE_BEGIN ()
                                ;
                                /* The target array is not distributed. Do nothing
                                 * special. */
                                indout (
                                  "SAC_ND_WRITE_READ_COPY( %s, SAC_i, %s, SAC_j, %s)\n",
                                  to_NT, vals_ANY[i], copyfun);
                            ELSE_END ();
                        } else {
                            /* The target array is not distributed. Do nothing special. */
                            indout ("SAC_ND_WRITE_READ_COPY( %s, SAC_i, %s, SAC_j, %s)\n",
                                    to_NT, vals_ANY[i], copyfun);
                        }

                        indout ("SAC_i++;\n");
                    FOR_LOOP_END ();
                }

                ASSURE_TYPE (ASSURE_COND ("SAC_ND_A_SIZE( %s) == SAC_i", to_NT),
                             ASSURE_TEXT ("Assignment with incompatible types found!"));

            BLOCK_END ();
        }
    }

    if (global.backend == BE_distmem && ICUGetDistributedClass (to_NT) == C_distr) {
        /* The target array is distributable. */
        IF_BEGIN ("SAC_ND_A_IS_DIST( %s)", to_NT)
            ;
            /* The target array is actually distributed. Forbid cache writes again. */
            indout ("SAC_DISTMEM_FORBID_CACHE_WRITES();\n");
        IF_END ();
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_VECT2OFFSET_arr( char *off_NT, int from_size,
 *                                      char *from_NT,
 *                                      int shp_size, char **shp_ANY)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_VECT2OFFSET_arr( off_NT, from_size, from_NT, shp_size, shp_ANY)
 *
 ******************************************************************************/

void
ICMCompileND_VECT2OFFSET_arr (char *off_NT, int from_size, char *from_NT, int shp_size,
                              char **shp_ANY)
{
    DBUG_ENTER ();

#define ND_VECT2OFFSET_arr
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_VECT2OFFSET_arr

    /*
     * CAUTION:
     * 'shp_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar.
     */

    DBUG_ASSERT (shp_size >= 0, "invalid size found!");

    Vect2Offset2 (off_NT, from_NT, from_size, SizeId, ReadId, shp_ANY, shp_size, NULL,
                  ReadConstArray_Str);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_VECT2OFFSET_id( char *off_NT, int from_size,
 *                                     char *from_NT,
 *                                     int shp_size, char *shp_NT)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_VECT2OFFSET_id( off_NT, from_size, from_NT, shp_size, shp_NT)
 *
 ******************************************************************************/

void
ICMCompileND_VECT2OFFSET_id (char *off_NT, int from_size, char *from_NT, int shp_size,
                             char *shp_NT)
{
    DBUG_ENTER ();

#define ND_VECT2OFFSET_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_VECT2OFFSET_id

    /*
     * CAUTION:
     * shp_NT is a tagged identifier (representing a vector).
     */
    Vect2Offset2 (off_NT, from_NT, from_size, SizeId, ReadId, shp_NT, shp_size, SizeId,
                  ReadId);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_IDXS2OFFSET_arr( char *off_NT, int idxs_size,
 *                                      char **idxs_ANY,
 *                                      int shp_size, char **shp_ANY)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_IDXS2OFFSET_arr( off_NT, idxs_size, idxs_ANY, shp_size, shp_ANY)
 *
 ******************************************************************************/

void
ICMCompileND_IDXS2OFFSET_arr (char *off_NT, int idxs_size, char **idxs_ANY, int shp_size,
                              char **shp_ANY)
{
    DBUG_ENTER ();

#define ND_IDXS2OFFSET_arr
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_IDXS2OFFSET_arr

    /*
     * CAUTION:
     * 'shp_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar.
     *
     * 'idxs_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    DBUG_ASSERT (idxs_size >= 0, "illegal index size");
    DBUG_ASSERT (shp_size >= 0, "illegal shape size");

    Vect2Offset2 (off_NT, idxs_ANY, idxs_size, NULL, ReadConstArray_Str, shp_ANY,
                  shp_size, NULL, ReadConstArray_Str);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_IDXS2OFFSET_id( char *off_NT, int idxs_size,
 *                                     char **idxs_ANY,
 *                                     int shp_size, char *shp_NT)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_IDXS2OFFSET_id( off_NT, idxs_size, idxs_ANY, shp_size, shp_NT)
 *
 ******************************************************************************/

void
ICMCompileND_IDXS2OFFSET_id (char *off_NT, int idxs_size, char **idxs_ANY, int shp_size,
                             char *shp_NT)
{
    DBUG_ENTER ();

#define ND_IDXS2OFFSET_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_IDXS2OFFSET_id

    /*
     * CAUTION:
     * shp_NT is a tagged identifier (representing a vector).
     *
     * 'idxs_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    DBUG_ASSERT (idxs_size >= 0, "illegal index size");

    Vect2Offset2 (off_NT, idxs_ANY, idxs_size, NULL, ReadConstArray_Str, shp_NT, shp_size,
                  SizeId, ReadId);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_ARRAY_IDXS2OFFSET_id( char *off_NT, int idxs_size,
 *                                           char **idxs_ANY,
 *                                           int arr_dim, char *arr_NT)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_ARRAY_IDXS2OFFSET_id( off_NT, idxs_size, idxs_ANY, arr_size, arr_NT)
 *
 ******************************************************************************/

void
ICMCompileND_ARRAY_IDXS2OFFSET_id (char *off_NT, int idxs_size, char **idxs_ANY,
                                   int arr_dim, char *arr_NT)
{
    DBUG_ENTER ();

#define ND_ARRAY_IDXS2OFFSET_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_ARRAY_IDXS2OFFSET_id

    /*
     * CAUTION:
     * arr_NT is a tagged identifier (representing an array).
     *
     * 'idxs_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    DBUG_ASSERT (idxs_size >= 0, "illegal index size");

    Vect2Offset2 (off_NT, idxs_ANY, idxs_size, NULL, ReadConstArray_Str, arr_NT, arr_dim,
                  NULL, ShapeId);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_ARRAY_VECT2OFFSET_id( char *off_NT, int from_size,
 *                                           char *from_NT,
 *                                           int shp_size, char *shp_NT)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_VECT2OFFSET_id( off_NT, from_size, from_NT, shp_size, shp_NT)
 *
 ******************************************************************************/

void
ICMCompileND_ARRAY_VECT2OFFSET_id (char *off_NT, int from_size, char *from_NT,
                                   int arr_dim, char *arr_NT)
{
    DBUG_ENTER ();

#define ND_ARRAY_VECT2OFFSET_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_ARRAY_VECT2OFFSET_id

    /*
     * CAUTION:
     * shp_NT is a tagged identifier (representing a vector).
     */

    Vect2Offset2 (off_NT, from_NT, from_size, SizeId, ReadId, arr_NT, arr_dim, NULL,
                  ShapeId);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_UNSHARE( char *to_NT, int to_sdim,
 *                                  char *from_NT, int from_sdim,
 *                                  char *basetype,
 *                                  char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_UNSHARE( va_NT, va_sdim, viv_NT, viv_sdim, basetype, copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_UNSHARE (char *va_NT, int va_sdim, char *viv_NT, int viv_sdim,
                      char *basetype, char *copyfun)
{
    DBUG_ENTER ();

#define ND_UNSHARE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_UNSHARE

    if (va_sdim == viv_sdim) {
        indout ("SAC_IS_SHARED__BLOCK_BEGIN( %s, %d, %s, %d)\n", va_NT, va_sdim, viv_NT,
                viv_sdim);

        global.indent++;
        ICMCompileND_COPY (viv_NT, viv_sdim, va_NT, va_sdim, basetype, copyfun);
        indout ("SAC_ND_DEC_RC( %s, 1)\n", va_NT);
        global.indent--;

        indout ("SAC_IS_SHARED__BLOCK_END( %s, %d, %s, %d)\n", va_NT, va_sdim, viv_NT,
                viv_sdim);
    } else {
        /* va_sdim != viv_sdim, hence cannot ever share */
        indout ("SAC_NOOP()\n");
    }

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
