
#include "cuda_utils.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"
#include "globals.h"

#define DBUG_PREFIX "UNDEFINED"
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
    case T_int:
        res = T_int_dev;
        break;
    case T_float:
        res = T_float_dev;
        break;
    case T_double:
        res = T_double_dev;
        break;
    default:
        DBUG_UNREACHABLE ("Simple type conversion found undefined host simple type!");
    }
    DBUG_RETURN (res);
}

simpletype
CUd2hSimpleTypeConversion (simpletype sty)
{
    simpletype res = T_unknown;

    DBUG_ENTER ();

    switch (sty) {
    case T_int_dev:
        res = T_int;
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

    res = TYgetSimpleType (TYgetScalar (ty)) == T_float_dev
          || TYgetSimpleType (TYgetScalar (ty)) == T_int_dev
          || TYgetSimpleType (TYgetScalar (ty)) == T_double_dev;

    DBUG_RETURN (res);
}

bool
CUisShmemTypeNew (ntype *ty)
{
    bool res;

    DBUG_ENTER ();

    res = TYgetSimpleType (TYgetScalar (ty)) == T_float_shmem
          || TYgetSimpleType (TYgetScalar (ty)) == T_int_shmem
          || TYgetSimpleType (TYgetScalar (ty)) == T_double_shmem;

    DBUG_RETURN (res);
}

bool
CUisShmemTypeOld (types *ty)
{
    bool res;

    DBUG_ENTER ();

    res = TCgetBasetype (ty) == T_float_shmem || TCgetBasetype (ty) == T_int_shmem
          || TCgetBasetype (ty) == T_double_shmem;

    DBUG_RETURN (res);
}

bool
CUisDeviceTypeOld (types *ty)
{
    bool res;

    DBUG_ENTER ();

    res = TCgetBasetype (ty) == T_float_dev || TCgetBasetype (ty) == T_int_dev
          || TCgetBasetype (ty) == T_double_dev;

    DBUG_RETURN (res);
}

bool
CUisDeviceArrayTypeNew (ntype *ty)
{
    bool res;

    DBUG_ENTER ();

    res = (TYgetSimpleType (TYgetScalar (ty)) == T_float_dev
           || TYgetSimpleType (TYgetScalar (ty)) == T_int_dev
           || TYgetSimpleType (TYgetScalar (ty)) == T_double_dev)
          && TYisArray (ty);

    DBUG_RETURN (res);
}

#undef DBUG_PREFIX
