/*
 * implementation of abstract datatype SAC_arg
 */

#include <stdarg.h>
#include "sac.h"

/******************************************************************************
 *
 * function:
 *   SAC_arg SAC_NewSACArg(SAC_ARG_simpletype basetype, int dim, int *shpvec)
 *
 * description:
 *   creates a SAC_arg data type of basetype and dimension with the specified
 *   shapevec, it DOES reuse the shpvec argument internally!
 *   it DOES NOT allocate memory for refcount and data!
 *
 ******************************************************************************/

SAC_arg
SAC_CI_NewSACArg (SAC_ARG_simpletype basetype, int dim, int *shpvec)
{
    SAC_arg result;

    result = (SAC_arg)SAC_MALLOC (sizeof (SAC_ARG_STRUCT));

    /* init structure */
    SAC_ARG_LRC (result) = 0;
    SAC_ARG_RC (result) = NULL;
    SAC_ARG_ELEMS (result) = NULL;
    SAC_ARG_TYPE (result) = basetype;
    SAC_ARG_DIM (result) = dim;

    if (dim == 0) {
        /* create simple type */
        SAC_ARG_SHPVEC (result) = NULL; /* no shape needed */
    } else {
        /* create array tyoe */
        SAC_ARG_SHPVEC (result) = shpvec;
    }

    return (result);
}

/******************************************************************************
 *
 * function:
 *   SAC_arg SAC_CreateSACArg(simpletype basetype, int dim, ...)
 *
 * description:
 *   creates a SAC_arg data type of basetype and dimension with the specified
 *   shape by varargs, it does NOT allocate memory for refcount and data!
 *
 * remark:
 *   uses var_args and calls SAC_CI_NewSACArg()
 *
 ******************************************************************************/

SAC_arg
SAC_CI_CreateSACArg (SAC_ARG_simpletype basetype, int dim, ...)
{
    va_list Argp;
    int *shpvec;
    int i;

    if (dim == 0) {
        /* create simple type */
        shpvec = NULL; /* no shape needed */
    } else {
        /* create array tyoe */
        shpvec = (int *)SAC_MALLOC (dim * sizeof (int));
        va_start (Argp, dim);
        for (i = 0; i < dim; i++) {
            shpvec[i] = va_arg (Argp, int);
        }
    }

    return (SAC_CI_NewSACArg (basetype, dim, shpvec));
}

/******************************************************************************
 *
 * function:
 *   bool SAC_CmpSACArgType(SAC_arg sa, SAC_ARG_simpletype basetype, int dim, ...)
 *
 * description:
 *   compares SAC_arg argument with given basetype, dimension and shape
 *
 ******************************************************************************/

bool
SAC_CI_CmpSACArgType (SAC_arg sa, SAC_ARG_simpletype basetype, int dim, ...)
{
    va_list Argp;
    bool res = true;
    int i;

    if ((SAC_ARG_TYPE (sa) == basetype) && (SAC_ARG_DIM (sa) == dim)) {
        /* check shape */
        va_start (Argp, dim);
        for (i = 0; i < dim; i++) {
            if ((SAC_ARG_SHPVEC (sa))[i] != va_arg (Argp, int))
                res = false;
        }
    } else {
        res = false;
    }
    return (res);
}

/******************************************************************************
 *
 * function:
 *   void SAC_FreeSACArg(SAC_arg sa)
 *
 * description:
 *   frees SAC_arg data structure and used resources, if they have not been
 *   released yet.
 *
 ******************************************************************************/

void
SAC_CI_FreeSACArg (SAC_arg sa)
{
    if (sa != NULL) {
        /* free all allocated resources and the data structure */
        if (SAC_ARG_LRC (sa) > 0) {
            /* free data and refcount */
            if (SAC_ARG_RC (sa) != NULL)
                SAC_FREE (SAC_ARG_RC (sa));
            if (SAC_ARG_ELEMS (sa) != NULL)
                SAC_FREE (SAC_ARG_ELEMS (sa));
        }
        /* if arraytype free shapevector */
        if (SAC_ARG_DIM (sa) > 0)
            SAC_FREE (SAC_ARG_SHPVEC (sa));

        /*free SAC_arg datastructure */
        SAC_FREE (sa);
    }
}
