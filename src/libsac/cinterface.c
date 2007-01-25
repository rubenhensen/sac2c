/*
 * $Log$
 * Revision 3.4  2000/12/08 10:03:46  nmw
 * no more warning on type mismatch on alpha
 *
 * Revision 3.3  2000/12/05 14:29:35  nmw
 * warning when setting refcounter of not refcounted variable
 * added. handling of T_hidden fixed
 *
 * Revision 3.2  2000/11/29 16:19:50  nmw
 * trace output of init/cleanup runtime enviroment added
 *
 * Revision 3.1  2000/11/20 18:02:43  sacbase
 * new release made
 *
 * Revision 1.13  2000/11/02 14:52:01  dkr
 * explicit path for #include removed
 *
 * Revision 1.12  2000/08/02 09:19:44  nmw
 * order of init function calls for interface and module changed
 *
 * Revision 1.11  2000/08/01 13:26:11  nmw
 * calling for internal init function for HM
 *
 * Revision 1.10  2000/07/28 14:43:42  nmw
 * Refcounting bug for simple types fixed
 * changes to handle T_user types internally
 *
 * Revision 1.9  2000/07/20 11:38:43  nmw
 * SAC_FreeRuntimesystem now frees all used sac-modules
 *
 * Revision 1.8  2000/07/13 09:46:07  nmw
 * convert functions for double, float, long, char added
 *
 * Revision 1.7  2000/07/12 10:03:13  nmw
 * RCS-Header added
 * memory leak fixed
 *
 * Revision 1.6  2000/07/07 15:34:11  nmw
 * utility functions added
 *
 * Revision 1.5  2000/07/06 15:54:33  nmw
 * convert functions changed to var_Args parameters
 *
 * Revision 1.4  2000/07/06 09:24:20  nme
 * simpletype from type_info.mac only used here, no more in sac_arg.h
 *
 * Revision 1.3  2000/07/05 19:48:12  nmw
 * stdlib.h added for linux compatibility
 *
 * Revision 1.2  2000/07/05 15:35:00  nmw
 * minor debugging, filenames adapted
 *
 * Revision 1.1  2000/07/05 12:43:13  nmw
 * Initial revision
 *
 * implementation of SAC <-> C interface functions,
 * implements prototypes from
 *     SAC_interface.h (external usage)
 */

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sac.h"
#include "sac_cinterface.h"

/* SAC_InitRuntimeSystem() and SAC_FreeRuntimeSystem() are not part
 * of the CInterface. These functions are implemented in each c-library
 * generated from a sac module, with runtime inits according to the
 * specified sac2c compiler switches concerning PIH and MT.
 * The functions call the init functions this cinterface.
 * This way of calling is necessary do avoid a circle in linking
 * dependencies
 */

#define CHAR_BUFFER_SIZE 256

/* Typenames used internally */
typedef enum {
#define TYP_IFname(name) name
#include "type_info.mac"
} SAC_ARG_simpletype_names;

/* global vars */
static bool SAC_CI_runtime_system_active = 0;

#define CHECK_FOR_ACTIVE_RUNTIMESYSTEM()                                                 \
    if (!SAC_CI_runtime_system_active) {                                                 \
        SAC_InitRuntimeSystem ();                                                        \
        SAC_InitCInterface ();                                                           \
    }

/******************************************************************************
 *
 * function:
 *   void SAC_InitCInterface()
 *
 * description:
 *   do some init procedures for the SAC runtime system (called be the
 *   SAC_InitRuntimeSystem Prozedure)
 *
 ******************************************************************************/

void
SAC_InitCInterface ()
{
    if (!SAC_CI_runtime_system_active) {
        /* init directory of used SAC_args */
        SAC_CI_InitSACArgDirectory ();
        SAC_CI_runtime_system_active = true;
    }
}

/******************************************************************************
 *
 * function:
 *   void SAC_FreeRuntimeSystem()
 *
 * description:
 *   do some cleanup after running the SAC Module
 *
 ******************************************************************************/

void
SAC_ExitCInterface ()
{
    if (SAC_CI_runtime_system_active) {
        /* free all used SAC_args */
        SAC_CI_FreeSACArgDirectory ();
    }
}

/******************************************************************************
 *
 * function:
 *   int SAC_isValid(SAC_arg sa)
 *
 * description:
 *   checks, if SAC_arg contains valid data (local refcount>0)
 *
 ******************************************************************************/

int
SAC_isValid (SAC_arg sa)
{
    CHECK_FOR_ACTIVE_RUNTIMESYSTEM ();
    return (SAC_ARG_LRC (sa) > 0 ? 1 : 0);
}

/******************************************************************************
 *
 * function:
 *   int SAC_GetDim(SAC_arg sa)
 *
 * description:
 *   returns the dimension of the SAC_arg
 *
 ******************************************************************************/

int
SAC_GetDim (SAC_arg sa)
{
    CHECK_FOR_ACTIVE_RUNTIMESYSTEM ();
    return (SAC_ARG_DIM (sa));
}

/******************************************************************************
 *
 * function:
 *   int SAC_GetShapeElement(SAC_arg sa, int pos)
 *
 * description:
 *   returns the Shapevector element at position 1..dim of SAC_arg
 *
 ******************************************************************************/

int
SAC_GetShapeElement (SAC_arg sa, int pos)
{
    CHECK_FOR_ACTIVE_RUNTIMESYSTEM ();
    if (pos >= SAC_ARG_DIM (sa)) {
        SAC_RuntimeError ("Access to illegal position in shape vector!\n");
    }
    return ((SAC_ARG_SHPVEC (sa))[pos - 1]);
}

/******************************************************************************
 *
 * function:
 *   void SAC_IncRC(SAC_arg sa, int no)
 *
 * description:
 *   increments the refcounter by no.
 *   does not change the refcounter if SAC_arg is invalid,
 *   that means refcounter has already reached 0
 *
 ******************************************************************************/

void
SAC_IncRC (SAC_arg sa, int no)
{
    char charbuffer[CHAR_BUFFER_SIZE];

    CHECK_FOR_ACTIVE_RUNTIMESYSTEM ();

    if (no < 0) {
        SAC_RuntimeError ("IncRC called with negative argument!\n");
    }

    if ((SAC_ARG_DIM (sa) > 0) || (SAC_ARG_TYPE (sa) == T_hidden)) {
        if (SAC_ARG_LRC (sa) > 0) {
            *(SAC_ARG_RC (sa)) += no;
            SAC_ARG_LRC (sa) += no;
        }
    } else {
        SAC_CI_SACArg2string (sa, charbuffer);
        SAC_Print ("*** warning: SAC_IncRC() called on variable not refcounted"
                   " (type: %s)\n\n",
                   charbuffer);
    }
}

/******************************************************************************
 *
 * function:
 *   void SAC_DecRC(SAC_arg sa, int no)
 *
 * description:
 *   decrements the refcounter by no.
 *   does not change the refcounter if SAC_arg is invalid,
 *   that means refcounter has already reached 0
 *
 ******************************************************************************/

void
SAC_DecRC (SAC_arg sa, int no)
{
    char charbuffer[CHAR_BUFFER_SIZE];

    CHECK_FOR_ACTIVE_RUNTIMESYSTEM ();

    if (no < 0) {
        SAC_RuntimeError ("DecRC called with negative argument!\n");
    }

    if ((SAC_ARG_DIM (sa) > 0) || (SAC_ARG_TYPE (sa) == T_hidden)) {
        if (SAC_ARG_LRC (sa) > 0) {
            *(SAC_ARG_RC (sa)) -= no;
            SAC_ARG_LRC (sa) -= no;
            if (SAC_ARG_LRC (sa) < 0) {
                SAC_RuntimeError ("RC dropped below zero.\n");
            }
        }
    } else {
        SAC_CI_SACArg2string (sa, charbuffer);
        SAC_Print ("*** warning: SAC_IncRC() called on variable not refcounted"
                   " (type: %s)\n\n",
                   charbuffer);
    }
}

/*
 * functions converting c-datatypes to SAC_arg and SAC_arg to c-datatype
 * here: macros used for all types
 *
 */

/* c-array -> SAC_arg */
#define SAC_ARRAY2SAC(c_type, SAC_type)                                                  \
    va_list Argp;                                                                        \
    SAC_arg result;                                                                      \
    int elemscount;                                                                      \
    int i;                                                                               \
    int *shpvec;                                                                         \
    c_type *elems;                                                                       \
                                                                                         \
    CHECK_FOR_ACTIVE_RUNTIMESYSTEM ();                                                   \
    /* create shapevector */                                                             \
    if (dim > 0) {                                                                       \
        shpvec = (int *)SAC_MALLOC (dim * sizeof (int));                                 \
        va_start (Argp, dim);                                                            \
        for (i = 0; i < dim; i++) {                                                      \
            shpvec[i] = va_arg (Argp, int);                                              \
        }                                                                                \
    } else {                                                                             \
        shpvec = NULL;                                                                   \
    }                                                                                    \
                                                                                         \
    if (reuseflag == SAC_COPY_ARG) {                                                     \
        /*                                                                               \
         * create copy of args for internal usage                                        \
         * calculate number of data elements                                             \
         */                                                                              \
        elemscount = 1;                                                                  \
        for (i = 0; i < dim; i++)                                                        \
            elemscount *= shpvec[i];                                                     \
                                                                                         \
        if (elemscount <= 0) {                                                           \
            SAC_RuntimeError ("Illegal shape vector!");                                  \
        }                                                                                \
                                                                                         \
        elems = (c_type *)SAC_MALLOC (elemscount * sizeof (c_type));                     \
        elems = memcpy (elems, array, elemscount * sizeof (c_type));                     \
    } else {                                                                             \
        /* args cans be reused internally */                                             \
        elems = array;                                                                   \
    }                                                                                    \
                                                                                         \
    result = SAC_CI_NewSACArg (SAC_type, NULL, dim, shpvec);                             \
                                                                                         \
    SAC_ARG_ELEMS (result) = elems;                                                      \
                                                                                         \
    result = SAC_CI_InitRefcounter (result, 1);                                          \
    return (result)

/* c-simple type -> SAC_arg */

#define SAC_SIMPLE2SAC(c_type, SAC_type)                                                 \
    SAC_arg result;                                                                      \
                                                                                         \
    CHECK_FOR_ACTIVE_RUNTIMESYSTEM ();                                                   \
    result = SAC_CI_NewSACArg (SAC_type, NULL, 0, NULL);                                 \
    SAC_ARG_ELEMS (result) = (c_type *)SAC_MALLOC (sizeof (c_type));                     \
    *((c_type *)SAC_ARG_ELEMS (result)) = value;                                         \
                                                                                         \
    result = SAC_CI_InitRefcounter (result, 1);                                          \
    return (result)

/* SAC_arg -> c-array */

#define SAC_SAC2ARRAY(c_type, SAC_type)                                                  \
    c_type *result;                                                                      \
    int i;                                                                               \
    int elemscount;                                                                      \
                                                                                         \
    CHECK_FOR_ACTIVE_RUNTIMESYSTEM ();                                                   \
    /* check for valid data */                                                           \
    SAC_CI_ExitOnInvalidArg (sa, SAC_type, NULL, SAC_CI_ARRAYTYPE);                      \
    if (reuseflag == SAC_COPY_ARG) {                                                     \
        elemscount = 1;                                                                  \
        for (i = 0; i < SAC_ARG_DIM (sa); i++)                                           \
            elemscount *= (SAC_ARG_SHPVEC (sa))[i];                                      \
                                                                                         \
        if (elemscount <= 0) {                                                           \
            SAC_RuntimeError ("Illegal shape vector!");                                  \
        }                                                                                \
                                                                                         \
        result = (c_type *)SAC_MALLOC (elemscount * sizeof (c_type));                    \
        result = memcpy (result, SAC_ARG_ELEMS (sa), elemscount * sizeof (c_type));      \
    } else {                                                                             \
        /* SAC_CONSUME_ARG - now sa is invalid */                                        \
        result = (c_type *)SAC_ARG_ELEMS (sa);                                           \
        SAC_ARG_LRC (sa) = 0;                                                            \
        SAC_FREE (SAC_ARG_RC (sa));                                                      \
    }                                                                                    \
    /* return value */                                                                   \
    return (result)

/* SAC_arg -> c-simpletype */

#define SAC_SAC2SIMPLE(c_type, SAC_type)                                                 \
                                                                                         \
    CHECK_FOR_ACTIVE_RUNTIMESYSTEM ();                                                   \
    /* check for valid data */                                                           \
    SAC_CI_ExitOnInvalidArg (sa, SAC_type, NULL, SAC_CI_SIMPLETYPE);                     \
    /* return value */                                                                   \
    return (*((c_type *)SAC_ARG_ELEMS (sa)))

/*
 * see headerfile ../runtime/sac_cinterface.h
 * for details in usage this functions
 *
 */

/* int */
SAC_arg
SAC_IntArray2Sac (SAC_reusetype reuseflag, int *array, int dim, ...)
{
    SAC_ARRAY2SAC (int, T_int);
}

SAC_arg
SAC_Int2Sac (int value)
{
    SAC_SIMPLE2SAC (int, T_int);
}

int *
SAC_Sac2IntArray (SAC_reusetype reuseflag, SAC_arg sa)
{
    SAC_SAC2ARRAY (int, T_int);
}

int
SAC_Sac2Int (SAC_arg sa)
{
    SAC_SAC2SIMPLE (int, T_int);
}

/* long */
SAC_arg
SAC_LongArray2Sac (SAC_reusetype reuseflag, long *array, int dim, ...)
{
    SAC_ARRAY2SAC (long, T_long);
}

SAC_arg
SAC_Long2Sac (long value)
{
    SAC_SIMPLE2SAC (long, T_long);
}

long *
SAC_Sac2LongArray (SAC_reusetype reuseflag, SAC_arg sa)
{
    SAC_SAC2ARRAY (long, T_long);
}

long
SAC_Sac2Long (SAC_arg sa)
{
    SAC_SAC2SIMPLE (long, T_long);
}

/* double */
SAC_arg
SAC_DoubleArray2Sac (SAC_reusetype reuseflag, double *array, int dim, ...)
{
    SAC_ARRAY2SAC (double, T_double);
}

SAC_arg
SAC_Double2Sac (double value)
{
    SAC_SIMPLE2SAC (double, T_double);
}

double *
SAC_Sac2DoubleArray (SAC_reusetype reuseflag, SAC_arg sa)
{
    SAC_SAC2ARRAY (double, T_double);
}

double
SAC_Sac2Double (SAC_arg sa)
{
    SAC_SAC2SIMPLE (double, T_double);
}

/* float */
SAC_arg
SAC_FloatArray2Sac (SAC_reusetype reuseflag, float *array, int dim, ...)
{
    SAC_ARRAY2SAC (float, T_float);
}

SAC_arg
SAC_Float2Sac (float value)
{
    SAC_SIMPLE2SAC (float, T_float);
}

float *
SAC_Sac2FloatArray (SAC_reusetype reuseflag, SAC_arg sa)
{
    SAC_SAC2ARRAY (float, T_float);
}

float
SAC_Sac2Float (SAC_arg sa)
{
    SAC_SAC2SIMPLE (float, T_float);
}

/* char */
SAC_arg
SAC_CharArray2Sac (SAC_reusetype reuseflag, char *array, int dim, ...)
{
    SAC_ARRAY2SAC (char, T_char);
}

SAC_arg
SAC_Char2Sac (char value)
{
    SAC_SIMPLE2SAC (char, T_char);
}

char *
SAC_Sac2CharArray (SAC_reusetype reuseflag, SAC_arg sa)
{
    SAC_SAC2ARRAY (char, T_char);
}

char
SAC_Sac2Char (SAC_arg sa)
{
    SAC_SAC2SIMPLE (char, T_char);
}
