#include "gtest/gtest.h"
#include "base-test-environment.h" // All unit test files need to import this!
testing::Environment* base_test_env = testing::AddGlobalTestEnvironment(new BaseEnvironment);

#include "config.h"

extern "C" {
#include "cuda_utils.h"
#define DBUG_PREFIX "TEST-CUTIL"
#include "debug.h"
#include "new_types.h"
#include "shape.h"
#include "ctinfo.h"
}

TEST (CUDA_UTILS, SupportedHostType)
{
    /* it would be nice if we could derive these from type_info.mac */
    ASSERT_TRUE (CUisSupportedHostSimpletype (T_int));
    ASSERT_TRUE (CUisSupportedHostSimpletype (T_float));
    ASSERT_TRUE (CUisSupportedHostSimpletype (T_double));
    ASSERT_TRUE (CUisSupportedHostSimpletype (T_bool));
    ASSERT_TRUE (CUisSupportedHostSimpletype (T_long));
    ASSERT_TRUE (CUisSupportedHostSimpletype (T_longlong));

    /* these are missing implementation, are they even needed? */
    ASSERT_FALSE (CUisSupportedHostSimpletype (T_byte));
    ASSERT_FALSE (CUisSupportedHostSimpletype (T_ubyte));
    ASSERT_FALSE (CUisSupportedHostSimpletype (T_uint));
    ASSERT_FALSE (CUisSupportedHostSimpletype (T_ulong));
    ASSERT_FALSE (CUisSupportedHostSimpletype (T_ulonglong));
}

TEST (CUDA_UTILS, IsSupportedType)
{
    ASSERT_TRUE (CUisSupportedHostSimpletype (T_int));
    ASSERT_TRUE (CUisSupportedHostSimpletype (T_float));
    ASSERT_FALSE (CUisSupportedHostSimpletype (T_byte));
}

TEST (CUDA_UTILS, IsDeviceType)
{
    ntype *type = NULL, *wtype = NULL;
    type = TYmakeAKD (TYmakeSimpleType (T_int_dev), 1, SHcreateShape (0));
    wtype = TYmakeAKD (TYmakeSimpleType (T_int), 1, SHcreateShape (0));
    ASSERT_FALSE (type == NULL);
    ASSERT_FALSE (wtype == NULL);

    ASSERT_TRUE (CUisDeviceTypeNew (type));
    ASSERT_FALSE (CUisDeviceTypeNew (wtype));

    type = TYfreeType (type);
    wtype = TYfreeType (wtype);
}

TEST (CUDA_UTILS, IsDeviceArrayType)
{
    ntype *type = NULL;
    type = TYmakeAKD (TYmakeSimpleType (T_int_dev), 1, SHcreateShape (0));
    ASSERT_FALSE (type == NULL);

    ASSERT_TRUE (CUisDeviceArrayTypeNew (type));

    type = TYfreeType (type);
}

TEST (CUDA_UTILS, IsShmemType)
{
    ntype *type = NULL, *wtype = NULL;
    type = TYmakeAKD (TYmakeSimpleType (T_int_shmem), 1, SHcreateShape (0));
    wtype = TYmakeAKD (TYmakeSimpleType (T_int), 1, SHcreateShape (0));
    ASSERT_FALSE (type == NULL);
    ASSERT_FALSE (wtype == NULL);

    ASSERT_TRUE (CUisShmemTypeNew (type));
    ASSERT_FALSE (CUisShmemTypeNew (wtype));

    type = TYfreeType (type);
    wtype = TYfreeType (wtype);
}

TEST (CUDA_UTILS, ConvertH2DSimpleType)
{
    CTIset_stderr (stderr);
    ASSERT_TRUE (CUh2dSimpleTypeConversion (T_int) == T_int_dev);
    ASSERT_TRUE (CUh2dSimpleTypeConversion (T_float) == T_float_dev);

    /* this should result in CTIerrorInternal being called */
    EXPECT_TRUE (CUh2dSimpleTypeConversion (T_byte) == T_unknown);
}

TEST (CUDA_UTILS, ConvertD2HSimpleType)
{
    CTIset_stderr (stderr);
    ASSERT_TRUE (CUd2hSimpleTypeConversion (T_int_dev) == T_int);
    ASSERT_TRUE (CUd2hSimpleTypeConversion (T_float_dev) == T_float);

    /* this should result in CTIerrorInternal being called */
    EXPECT_TRUE (CUd2hSimpleTypeConversion (T_byte) == T_unknown);
}

TEST (CUDA_UTILS, ConvertH2SHSimpleType)
{
    CTIset_stderr (stderr);
    ASSERT_TRUE (CUh2shSimpleTypeConversion (T_int) == T_int_shmem);
    ASSERT_TRUE (CUh2shSimpleTypeConversion (T_float) == T_float_shmem);

    /* this should result in CTIerrorInternal being called */
    EXPECT_TRUE (CUh2shSimpleTypeConversion (T_byte) == T_unknown);
}

TEST (CUDA_UTILS, ConvertD2SHSimpleType)
{
    CTIset_stderr (stderr);
    ASSERT_TRUE (CUd2shSimpleTypeConversion (T_int_dev) == T_int_shmem);
    ASSERT_TRUE (CUd2shSimpleTypeConversion (T_float_dev) == T_float_shmem);

    /* this should result in CTIerrorInternal being called */
    EXPECT_TRUE (CUd2shSimpleTypeConversion (T_byte) == T_unknown);
}

TEST (CUDA_UTILS, ConvertH2DType)
{
    ntype *type = NULL, *dtype = NULL;

    type = TYmakeAKD (TYmakeSimpleType (T_int), 1, SHcreateShape (0));

    ASSERT_FALSE (type == NULL);

    dtype = CUconvertHostToDeviceType (type);

    /* convery copies the type, so it should still persist */
    ASSERT_FALSE (type == NULL);
    ASSERT_FALSE (dtype == NULL);

    /* check that we have the correct type */
    ASSERT_TRUE (TYgetSimpleType (TYgetScalar (dtype)) == T_int_dev);

    type = TYfreeType (type);
    dtype = TYfreeType (dtype);
}

TEST (CUDA_UTILS, ConvertD2HType)
{
    ntype *type = NULL, *dtype = NULL;

    dtype = TYmakeAKD (TYmakeSimpleType (T_int_dev), 1, SHcreateShape (0));

    ASSERT_FALSE (dtype == NULL);

    type = CUconvertDeviceToHostType (dtype);

    /* convery copies the type, so it should still persist */
    ASSERT_FALSE (type == NULL);
    ASSERT_FALSE (dtype == NULL);

    /* check that we have the correct type */
    ASSERT_TRUE (TYgetSimpleType (TYgetScalar (type)) == T_int);

    type = TYfreeType (type);
    dtype = TYfreeType (dtype);
}

#undef DBUG_PREFIX
