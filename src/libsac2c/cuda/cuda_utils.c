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

simpletype
CUh2dSimpleTypeConversion (simpletype sty)
{
    simpletype res = T_unknown;

    DBUG_ENTER ();

    switch (sty) {
#define CUTypeMap(host, device, shmem, dist) \
    case host:                               \
        res = device;                        \
        break;
#include "cuda_types.mac"
#undef CUTypeMap
    default:
        CTIerrorInternal ("Simple type conversion found undefined host simple type!");
    }

    DBUG_RETURN (res);
}

simpletype
CUd2hSimpleTypeConversion (simpletype sty)
{
    simpletype res = T_unknown;

    DBUG_ENTER ();

    switch (sty) {
#define CUTypeMap(host, device, shmem, dist) \
    case device:                             \
        res = host;                          \
        break;
#include "cuda_types.mac"
#undef CUTypeMap
    default:
        CTIerrorInternal ("Simple type conversion found undefined device simple type!");
    }
    DBUG_RETURN (res);
}

simpletype
CUd2shSimpleTypeConversion (simpletype sty)
{
    simpletype res = T_unknown;

    DBUG_ENTER ();

    switch (sty) {
#define CUTypeMap(host, device, shmem, dist) \
    case device:                             \
    case dist:                               \
        res = shmem;                         \
        break;
#include "cuda_types.mac"
#undef CUTypeMap
    default:
        CTIerrorInternal ("Simple type conversion found undefined device simple type!");
    }
    DBUG_RETURN (res);
}

simpletype
CUh2shSimpleTypeConversion (simpletype sty)
{
    simpletype res = T_unknown;

    DBUG_ENTER ();

    switch (sty) {
#define CUTypeMap(host, device, shmem, dist) \
    case host:                               \
        res = shmem;                         \
        break;
#include "cuda_types.mac"
#undef CUTypeMap
    default:
        CTIerrorInternal ("Simple type conversion found undefined host simple type!");
    }
    DBUG_RETURN (res);
}

static bool
CUisDeviceType (simpletype sty)
{
    bool res;

    DBUG_ENTER ();

    switch (sty) {
#define CUTypeMap(host, device, shmem, dist) \
    case device:
#include "cuda_types.mac"
#undef CUTypeMap
        res = true;
        break;
    default:
        res = false;
    }
    DBUG_RETURN (res);
}

static bool
CUisShmemType (simpletype sty)
{
    bool res;

    DBUG_ENTER ();

    switch (sty) {
#define CUTypeMap(host, device, shmem, dist) \
    case shmem:
#include "cuda_types.mac"
#undef CUTypeMap
        res = true;
        break;
    default:
        res = false;
    }
    DBUG_RETURN (res);
}

bool
CUisSupportedHostSimpletype (simpletype st)
{
    bool ret;

    DBUG_ENTER ();

    switch (st)
    {
#define CUTypeMap(host, device, shmem, dist) case host:
#include "cuda_types.mac"
#undef CUTypeMap
        ret = true;
        break;
    default:
        ret = false;
    }
    DBUG_RETURN (ret);
}

bool
CUisDeviceTypeNew (ntype *ty)
{
    DBUG_ENTER ();
    DBUG_RETURN (CUisDeviceType (TYgetSimpleType (TYgetScalar (ty))));
}

bool
CUisShmemTypeNew (ntype *ty)
{
    DBUG_ENTER ();
    DBUG_RETURN (CUisShmemType (TYgetSimpleType (TYgetScalar (ty))));
}

bool
CUisDeviceArrayTypeNew (ntype *ty)
{
    DBUG_ENTER ();
    DBUG_RETURN (CUisDeviceTypeNew (ty) && TYisArray (ty));
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

    dev_type = TYcopyType (host_type);
    /* If the scalar type is simple, e.g. int, float ... */
    if (TYgetDim (host_type) >0
        && TYisSimple (TYgetScalar (host_type))) {
        scalar_type = TYgetScalar (dev_type);
        /* Get the corresponding device simple type e.g. int_dev, float_dev...*/
        sty = CUh2dSimpleTypeConversion (TYgetSimpleType (scalar_type));
        /* Set the device simple type */
        scalar_type = TYsetSimpleType (scalar_type, sty);
    }

    DBUG_RETURN (dev_type);
}

/**
 * @brief Translates device type (from simpletype) to host type, while
 *        preseving shape information.
 *
 * @param type The new type that *is* a device type
 * @return an updated instances of type
 */
ntype *
CUconvertDeviceToHostType (ntype *device_type)
{
    ntype *scalar_type, *h_type = NULL;
    simpletype sty;

    DBUG_ENTER ();

    if (!TUdimKnown (device_type))
        CTIerrorInternal ("AUD type found!");

    h_type = TYcopyType (device_type);
    /* If the scalar type is simple, e.g. int, float ... */
    if (TYgetDim (device_type) >0
        && TYisSimple (TYgetScalar (device_type))) {
        scalar_type = TYgetScalar (h_type);
        /* Get the corresponding device simple type e.g. int_dev, float_dev...*/
        sty = CUd2hSimpleTypeConversion (TYgetSimpleType (scalar_type));
        /* Set the device simple type */
        scalar_type = TYsetSimpleType (scalar_type, sty);
    }

    DBUG_RETURN (h_type);
}

 /** @} */
#undef DBUG_PREFIX
