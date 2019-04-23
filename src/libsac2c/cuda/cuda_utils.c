/**
 * @file
 * @defgroup cutil CUDA utils
 * @ingroup cuda
 *
 * @{
 */
#include "cuda_utils.h"

#include "type_utils.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"
#include "globals.h"

#define DBUG_PREFIX "CUTIL"
#include "debug.h"

#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "new_types.h"

node *
CUnthApArg (node *args, int n)
{
    int i = 0;
    node *tmp = args;

    DBUG_ENTER ();

    while (i < n) {
        tmp = EXPRS_NEXT (tmp);
        i++;
    }

    tmp = EXPRS_EXPR (tmp);
    DBUG_RETURN (tmp);
}

bool
CUisSupportedHostSimpletype (simpletype st)
{
    DBUG_ENTER ();
    DBUG_RETURN ((st == T_bool) || (st == T_int) || (st == T_long) || (st == T_longlong)
                 || (st == T_float) || (st == T_double));
}

simpletype
CUh2dSimpleTypeConversion (simpletype sty)
{
    simpletype res = T_unknown;

    DBUG_ENTER ();

    switch (sty) {
    case T_bool:
        res = T_bool_dev;
        break;
    case T_int:
        res = T_int_dev;
        break;
    case T_long:
        res = T_long_dev;
        break;
    case T_longlong:
        res = T_longlong_dev;
        break;
    case T_float:
        res = T_float_dev;
        break;
    case T_double:
        res = T_double_dev;
        break;
    default:
        DBUG_UNREACHABLE ("Host to Device type conversion encountered not yet supported "
                          "host element type: %s!",
                          global.type_string[sty]);
    }
    DBUG_RETURN (res);
}

simpletype
CUd2hSimpleTypeConversion (simpletype sty)
{
    simpletype res = T_unknown;

    DBUG_ENTER ();

    switch (sty) {
    case T_bool_dev:
        res = T_bool;
        break;
    case T_int_dev:
        res = T_int;
        break;
    case T_long_dev:
        res = T_long;
        break;
    case T_longlong_dev:
        res = T_longlong;
        break;
    case T_float_dev:
        res = T_float;
        break;
    case T_double_dev:
        res = T_double;
        break;
    default:
        DBUG_UNREACHABLE ("Simple type conversion found undefined device simple type!");
    }
    DBUG_RETURN (res);
}

simpletype
CUd2shSimpleTypeConversion (simpletype sty)
{
    simpletype res = T_unknown;

    DBUG_ENTER ();

    switch (sty) {
    case T_int_dev:
    case T_int_dist:
        res = T_int_shmem;
        break;
    case T_long_dev:
    case T_long_dist:
        res = T_long_shmem;
        break;
    case T_longlong_dev:
    case T_longlong_dist:
        res = T_longlong_shmem;
        break;
    case T_float_dev:
    case T_float_dist:
        res = T_float_shmem;
        break;
    case T_double_dev:
    case T_double_dist:
        res = T_double_shmem;
        break;
    default:
        DBUG_UNREACHABLE ("Simple type conversion found undefined device simple type!");
    }
    DBUG_RETURN (res);
}

simpletype
CUh2shSimpleTypeConversion (simpletype sty)
{
    simpletype res = T_unknown;

    DBUG_ENTER ();

    switch (sty) {
    case T_int:
        res = T_int_shmem;
        break;
    case T_long:
        res = T_long_shmem;
        break;
    case T_longlong:
        res = T_longlong_shmem;
        break;
    case T_float:
        res = T_float_shmem;
        break;
    case T_double:
        res = T_double_shmem;
        break;
    default:
        DBUG_UNREACHABLE ("Simple type conversion found undefined host simple type!");
    }
    DBUG_RETURN (res);
}

bool
CUisDeviceTypeNew (ntype *ty)
{
    bool res;

    DBUG_ENTER ();

    switch (TYgetSimpleType (TYgetScalar (ty))) {
    case T_bool_dev:
    case T_int_dev:
    case T_long_dev:
    case T_longlong_dev:
    case T_float_dev:
    case T_double_dev:
        res = TRUE;
        break;
    default:
        res = FALSE;
    }

    DBUG_RETURN (res);
}

bool
CUisShmemTypeNew (ntype *ty)
{
    bool res;

    DBUG_ENTER ();

    switch (TYgetSimpleType (TYgetScalar (ty))) {
    case T_int_shmem:
    case T_long_shmem:
    case T_longlong_shmem:
    case T_float_shmem:
    case T_double_shmem:
        res = TRUE;
        break;
    default:
        res = FALSE;
    }

    DBUG_RETURN (res);
}

bool
CUisShmemTypeOld (types *ty)
{
    bool res;

    DBUG_ENTER ();

    switch (TCgetBasetype (ty)) {
    case T_int_shmem:
    case T_long_shmem:
    case T_longlong_shmem:
    case T_float_shmem:
    case T_double_shmem:
        res = TRUE;
        break;
    default:
        res = FALSE;
    }

    DBUG_RETURN (res);
}

bool
CUisDeviceTypeOld (types *ty)
{
    bool res;

    DBUG_ENTER ();

    switch (TCgetBasetype (ty)) {
    case T_bool_dev:
    case T_int_dev:
    case T_long_dev:
    case T_longlong_dev:
    case T_float_dev:
    case T_double_dev:
        res = TRUE;
        break;
    default:
        res = FALSE;
    }

    DBUG_RETURN (res);
}

bool
CUisDeviceArrayTypeNew (ntype *ty)
{
    bool res;

    DBUG_ENTER ();

    res = CUisDeviceTypeNew (ty) && TYisArray (ty);

    DBUG_RETURN (res);
}

/**
 * @brief Convert from a host ntype to a device ntype, while preserving shape information.
 *
 * @param host_type The host ntype
 * @return A device ntype struct, or NULL if the host_type *does not* have a simpletype
 */
ntype *
CUconvertHostToDeviceType (ntype *host_type)
{
    ntype *scalar_type, *dev_type = NULL;
    simpletype sty;

    DBUG_ENTER ();

    /* If the host_type is of known dimension */
    if (!TUdimKnown (host_type))
        CTIerrorInternal ("AUD type found!");

    /* If the scalar type is simple, e.g. int, float ... */
    if (TYgetDim (host_type) > 0
        && TYisSimple (TYgetScalar (host_type))) {
        dev_type = TYcopyType (host_type);
        scalar_type = TYgetScalar (dev_type);
        /* Get the corresponding device simple type e.g. int_dev, float_dev...*/
        sty = CUh2dSimpleTypeConversion (TYgetSimpleType (scalar_type));
        /* Set the device simple type */
        scalar_type = TYsetSimpleType (scalar_type, sty);
    }

    DBUG_RETURN (dev_type);
}

#undef DBUG_PREFIX
