
#include "cuda_utils.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"
#include "globals.h"
#include "dbug.h"
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

    DBUG_ENTER ("CUnthApArg");

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

    DBUG_ENTER ("CUh2dSimpleTypeConversion");

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
        DBUG_ASSERT ((0), "Simple type conversion found undefined host simple type!");
    }
    DBUG_RETURN (res);
}

bool
CUisDeviceTypeNew (ntype *ty)
{
    bool res;

    DBUG_ENTER ("CUisDeviceTypeNew");

    res = TYgetSimpleType (TYgetScalar (ty)) == T_float_dev
          || TYgetSimpleType (TYgetScalar (ty)) == T_int_dev
          || TYgetSimpleType (TYgetScalar (ty)) == T_double_dev;

    DBUG_RETURN (res);
}

bool
CUisDeviceTypeOld (types *ty)
{
    bool res;

    DBUG_ENTER ("CUisDeviceTypeOld");

    res = TCgetBasetype (ty) == T_float_dev || TCgetBasetype (ty) == T_int_dev
          || TCgetBasetype (ty) == T_double_dev;

    DBUG_RETURN (res);
}

bool
CUisDeviceArrayTypeNew (ntype *ty)
{
    bool res;

    DBUG_ENTER ("CUisDeviceArrayTypeNew");

    res = (TYgetSimpleType (TYgetScalar (ty)) == T_float_dev
           || TYgetSimpleType (TYgetScalar (ty)) == T_int_dev
           || TYgetSimpleType (TYgetScalar (ty)) == T_double_dev)
          && TYisArray (ty);

    DBUG_RETURN (res);
}
