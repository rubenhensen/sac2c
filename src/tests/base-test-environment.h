/**
 * @file base-test-environment.h
 * Including this file in a test suite registers the base environment
 * to the GoogleTest framework and makes it so the SetUp() and TearDown()
 * functions are run before and after every test.
 */

#ifndef _SAC_BASE_TEST_ENVIRONMENT_H_
#define _SAC_BASE_TEST_ENVIRONMENT_H_

#include "gtest/gtest.h"

class BaseEnvironment : public ::testing::Environment {
    public:

    void SetUp();
    void TearDown();
};

#endif /* _SAC_BASE_TEST_ENVIRONMENT_H_ */
