/* implementation of SAC <-> C interface functions,
 * implements prototypes from
 *     SAC_interface.h (external usage)
 */

#include <stdarg.h>
#include <stdlib.h>
#include "sac.h"
#include "sac_cinterface.h"

/* Typenames used internally */
typedef enum {
#define TYP_IFname(name) name
#include "../tree/type_info.mac"
} SAC_ARG_simpletype_names;

/* constants */
#define SAC_CI_SIMPLETYPE 1
#define SAC_CI_ARRAYTYPE 2

/* global vars */
static bool SAC_CI_runtime_system_active;

/* functions with only local use */
static void SAC_CI_ExitOnInvalidArg (SAC_arg sa, SAC_ARG_simpletype basetype,
                                     int arg_mode);

/******************************************************************************
 *
 * function:
 *   void SAC_InitRuntimeSystem()
 *
 * description:
 *   do some init procedures for the SAC runtime system
 *
 ******************************************************************************/

void
SAC_InitRuntimeSystem ()
{
    if (!SAC_CI_runtime_system_active) {
        /* do some inits */

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
SAC_FreeRuntimeSystem ()
{
    if (!SAC_CI_runtime_system_active) {
        /* do some cleanup */
    }
}

/*
 *
 * functions converting c-datatypes to SAC_arg and SAC_arg to c-datatype
 *
 */

/* macros used for all types */
#define SAC_ARRAY2SAC(c_type, SAC_type)                                                  \
    va_list Argp;                                                                        \
    SAC_arg result;                                                                      \
    int elemscount;                                                                      \
    int i;                                                                               \
    int *shpvec;                                                                         \
    c_type *elems;                                                                       \
                                                                                         \
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
    if (reuseflag == SAC_COPY_ARGS) {                                                    \
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
    result = SAC_CI_NewSACArg (SAC_type, dim, shpvec);                                   \
                                                                                         \
    SAC_ARG_ELEMS (result) = elems;                                                      \
                                                                                         \
    result = SAC_CI_InitRefcounter (result, 1);                                          \
    return (result)

#define SAC_SIMPLE2SAC(c_type, SAC_type)                                                 \
    SAC_arg result;                                                                      \
    result = SAC_CI_NewSACArg (SAC_type, 0, NULL);                                       \
    SAC_ARG_ELEMS (result) = (c_type *)SAC_MALLOC (sizeof (c_type));                     \
    *((c_type *)SAC_ARG_ELEMS (result)) = value;                                         \
                                                                                         \
    result = SAC_CI_InitRefcounter (result, 1);                                          \
    return (result)

#define SAC_SAC2ARRAY(c_type, SAC_type)                                                  \
    /* check for valid data */                                                           \
    SAC_CI_ExitOnInvalidArg (sa, SAC_type, SAC_CI_ARRAYTYPE);                            \
    /* return value */                                                                   \
    return ((c_type *)SAC_ARG_ELEMS (sa))

#define SAC_SAC2SIMPLE(c_type, SAC_type)                                                 \
    /* check for valid data */                                                           \
    SAC_CI_ExitOnInvalidArg (sa, SAC_type, SAC_CI_SIMPLETYPE);                           \
    /* return value */                                                                   \
    return (*((c_type *)SAC_ARG_ELEMS (sa)))

/******************************************************************************
 *
 * functions: here for type ???
 *   SAC_arg  SAC_???Array2Sac(SAC_reusetype reuseflag, ??? *array, int dim, ...)
 *   SAC_arg  SAC_Int2Sac(??? value)
 *   ???     *SAC_Sac2IntArray(SAC_arg sa)
 *   ???      SAC_Sac2Int(SAC_arg sa)
 *
 * description:
 *   converts c datatype to SAC_arg datatype and vice versa.
 *   for every basic type there are 4 functions:
 *         simpletype -> SAC_arg
 *         arraytype  -> SAC_arg
 *         SAC_arg    -> simpletype
 *         SAC_arg    -> arraytype
 *
 *   for the arraytype to SAC_arg you have to specify the dimension and its
 *   shape (as varargs after the dimension).
 *   When reuseflag is set to SAC_CONSUME_ARGS, the array is reused internally,
 *   and you are not allowed to access this array any more!!! Do not free this
 *   array!!!
 *   When reuseflag is set to SAC_COPY_ARGS, then the argument array is not
 *   touched and you might use it in the future.
 *
 ******************************************************************************/

/* functions for the usual basetypes */
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
SAC_Sac2IntArray (SAC_arg sa)
{
    SAC_SAC2ARRAY (int, T_int);
}

int
SAC_Sac2Int (SAC_arg sa)
{
    SAC_SAC2SIMPLE (int, T_int);
}

/******************************************************************************
 *
 * function:
 *   void SAC_CI_ExitOnInvalidArg(SAC_arg sa,
 *                                SAC_ARG_simpletype basetype, int arg_mode)
 *
 * description:
 *   checks SAC_arg for valid content
 *   calls SAC_RuntimeError on error
 *
 ******************************************************************************/

static void
SAC_CI_ExitOnInvalidArg (SAC_arg sa, SAC_ARG_simpletype basetype, int flag)
{
    if (SAC_ARG_LRC (sa) > 0) {
        /* check basetype */
        if (SAC_ARG_TYPE (sa) == basetype) {
            if (flag == SAC_CI_SIMPLETYPE && SAC_ARG_DIM (sa) != 0) {
                SAC_RuntimeError ("SAC_Sac2XXX: access to array as simple type!\n");
            }
            if (flag == SAC_CI_ARRAYTYPE && SAC_ARG_DIM (sa) < 1) {
                SAC_RuntimeError ("SAC_Sac2XXX: access to simple type as array!\n");
            }
        } else {
            SAC_RuntimeError ("SAC_Sac2XXX: access to wrong basetype!\n");
        }
    } else {
        SAC_RuntimeError ("SAC_Sac2XXX: access to invalid SAC_arg data,\n"
                          "maybe increase the reference counter!\n");
    }
}
