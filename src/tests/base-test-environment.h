/**
 * @file base-test-environment.h
 * Including this file in a test suite registers the base environment
 * to the GoogleTest framework and makes it so the SetUp() and TearDown()
 * functions are run before and after every test.
 */

#ifndef _SAC_BASE_TEST_ENVIRONMENT_H_
#define _SAC_BASE_TEST_ENVIRONMENT_H_

#include "gtest/gtest.h"

extern "C" {
#include "ctformatting.h"
#include "ctinfo.h"
}

class BaseEnvironment : public ::testing::Environment {
    public:

#if __cplusplus >= 201103L
    void SetUp() override
#else
    void SetUp()
#endif
    {
        // Compile time formatting must be initialized to properly throw errors.
        CTIset_stderr (stderr);
        CTFinitialize ();
    }

#if __cplusplus >= 201103L
    void TearDown() override
#else
    void TearDown()
#endif
    {
        // Empty for now
    }
};

// A nice hack provided by the gtest library to always register the environment:
// https://google.github.io/googletest/advanced.html#global-set-up-and-tear-down
// base_env is not used for anything other than allowing us to run the function.
// The gtest library deallocates the environment.
testing::Environment* const base_env =
    testing::AddGlobalTestEnvironment(new BaseEnvironment);

#endif /* _SAC_BASE_TEST_ENVIRONMENT_H_ */
