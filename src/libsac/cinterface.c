/*
 * implementation of SAC <-> C interface functions,
 * implements prototypes from
 *     SAC_interface.h (external usage)
 */

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "sac.h"
#include "sac_cinterface.h"

/* Typenames used internally */
typedef enum {
#define TYP_IFname(name) name
#include "../tree/type_info.mac"
} SAC_ARG_simpletype_names;

/* global vars */
static bool SAC_CI_runtime_system_active = 0;

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

        SAC_CI_InitSACArgDirectory ();

        printf ("SAC-runtimesystem ready...\n");

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
    if (SAC_CI_runtime_system_active) {
        /* do some cleanup */
        SAC_CI_FreeSACArgDirectory ();
        printf ("SAC-Runtime-System cleaned up!\n");
    }
}

/******************************************************************************
 *
 * function:
 *   int SAC_isValid(SAC_arg sa)
 *
 * description:
 *   checks, if SAC_arg contains valid data (refcount>0)
 *
 ******************************************************************************/

int
SAC_isValid (SAC_arg sa)
{
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
    if (pos >= SAC_ARG_DIM (sa)) {
        SAC_RuntimeError ("Access to illegal position in shape vector!\n");
    }
    return ((SAC_ARG_SHPVEC (sa))[pos - 1]);
}

/******************************************************************************
 *
 * function:
 *   int SAC_GetRefcounter(SAC_arg sa)
 *
 * description:
 *   returns the current refcounter value
 *
 ******************************************************************************/

int
SAC_GetRefcounter (SAC_arg sa)
{
    return (SAC_ARG_LRC (sa));
}

/******************************************************************************
 *
 * function:
 *   int SAC_SetRefcounter(SAC_arg sa, int newrc)
 *
 * description:
 *   sets the refcounter to newrc, returns the new refcounter
 *   does not change the refcounter is SAC_arg is invalid,
 *   that means refcounter has already reached 0
 *
 ******************************************************************************/

int
SAC_SetRefcounter (SAC_arg sa, int newrc)
{
    if (newrc < 0)
        SAC_RuntimeError ("Illegal refcounter value specified!\n");
    if (SAC_ARG_LRC (sa) > 0) {
        return ((SAC_ARG_LRC (sa)) = newrc);
    } else {
        return (SAC_ARG_LRC (sa));
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
    c_type *result;                                                                      \
    int i;                                                                               \
    int elemscount;                                                                      \
    /* check for valid data */                                                           \
    SAC_CI_ExitOnInvalidArg (sa, SAC_type, SAC_CI_ARRAYTYPE);                            \
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
        /* SAC_CONSUME_ARG */                                                            \
        result = (c_type *)SAC_ARG_ELEMS (sa);                                           \
        SAC_ARG_LRC (sa) = 0;                                                            \
        SAC_FREE (SAC_ARG_RC (sa));                                                      \
        printf ("*************** FREE **************************\n");                    \
    }                                                                                    \
    /* return value */                                                                   \
    return (result)

#define SAC_SAC2SIMPLE(c_type, SAC_type)                                                 \
    /* check for valid data */                                                           \
    SAC_CI_ExitOnInvalidArg (sa, SAC_type, SAC_CI_SIMPLETYPE);                           \
    /* return value */                                                                   \
    return (*((c_type *)SAC_ARG_ELEMS (sa)))

/******************************************************************************
 *
 * functions: here for type ???
 *   SAC_arg  SAC_???Array2Sac(SAC_reusetype reuseflag,??? *array,int dim,...)
 *   SAC_arg  SAC_???2Sac(??? value)
 *   ???     *SAC_Sac2IntArray(SAC_reusetype reuseflag, SAC_arg sa)
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
 *
 * remarks:
 *   When reuseflag is set to SAC_CONSUME_ARG, the array is shared internally,
 *   between c-code and SAC. Therefore you are not allowed to access a c array
 *   any more when convert it to SAC_arg!!! DO NOT FREE such a array!!!
 *   On the other hand you are not allowed to access the SAC_arg any more,
 *   when you have converted it to a c-array using SAC_CONSUME_ARG. And you
 *   HAVE TO FREE this array.
 *   When reuseflag is set to SAC_COPY_ARG, then the argument array is not
 *   touched and you might use it in the future. This is valid for both
 *   directions. A c-array you have to free on your own.
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
SAC_Sac2IntArray (SAC_reusetype reuseflag, SAC_arg sa)
{
    SAC_SAC2ARRAY (int, T_int);
}

int
SAC_Sac2Int (SAC_arg sa)
{
    SAC_SAC2SIMPLE (int, T_int);
}
