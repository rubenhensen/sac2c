/* implementation of SAC <-> C interface functions,
 * implements prototypes from
 *     SAC_interface.h (external usage)
 */

#include <stdarg.h>
#include <stdlib.h>
#include "sac.h"
#include "sac_cinterface.h"

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

SAC_arg
SAC_IntArray2Sac (int *array, int dim, int *shape, SAC_reusetype reuseflag)
{
    SAC_arg result;
    int elemscount;
    int i;
    int *shpvec;
    int *elems;

    if (reuseflag == SAC_COPY_ARGS) {
        /* create copy of args for internal usage */
        shpvec = (int *)SAC_MALLOC (dim * sizeof (int));
        shpvec = memcpy (shpvec, shape, dim * sizeof (int));

        /* calculate number of dataelements */
        elemscount = 1;
        for (i = 0; i < dim; i++)
            elemscount *= shape[i];

        if (elemscount <= 0) {
            SAC_RuntimeError ("Illegal shape vector!");
        }

        elems = (int *)SAC_MALLOC (elemscount * sizeof (int));
        elems = memcpy (elems, array, elemscount * sizeof (int));
    } else {
        /* args cans be reused internally */
        shpvec = shape;
        elems = array;
    }

    result = SAC_CI_NewSACArg (T_int, dim, shpvec);
    SAC_ARG_ELEMS (result) = elems;
    SAC_ARG_RC (result) = (int *)SAC_MALLOC (sizeof (int));
    *SAC_ARG_RC (result) = 1; /* init refcounter */
    return (result);
}

SAC_arg
SAC_Int2Sac (int value)
{
    SAC_arg result;
    result = SAC_CI_NewSACArg (T_int, 0, NULL);
    SAC_ARG_ELEMS (result) = (int *)SAC_MALLOC (sizeof (int));
    SAC_ARG_RC (result) = (int *)SAC_MALLOC (sizeof (int));
    *SAC_ARG_RC (result) = 1; /* init refcounter */
    return (result);
}

int *
SAC_Sac2IntArray (SAC_arg sa)
{
    /* check for valid data */
    SAC_CI_ExitOnInvalidArg (sa, T_int, SAC_CI_ARRAYTYPE);
    /* return value */
    return ((int *)SAC_ARG_ELEMS (sa));
}

int
SAC_Sac2Int (SAC_arg sa)
{
    /* check for valid data */
    SAC_CI_ExitOnInvalidArg (sa, T_int, SAC_CI_SIMPLETYPE);
    /* return value */
    return (*((int *)SAC_ARG_ELEMS (sa)));
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
