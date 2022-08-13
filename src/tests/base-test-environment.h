/**
 * @file base-test-environment.h
 * Including this file in a test suite registers the base environment
 * to the GoogleTest framework and makes it so the SetUp() and TearDown()
 * functions are run before and after every test.
 */

#ifndef _SAC_BASE_TEST_ENVIRONMENT_H_
#define _SAC_BASE_TEST_ENVIRONMENT_H_
#include <cstdio>

#include "gtest/gtest.h"

extern "C" {
/*
 * We avoid including the ctinfo.h and ctformating.h files
 * here as theses have XSL dependencies which cannot be
 * resolved at the level of tests. Using object linking
 * is fine *but* if function names/signatures should change,
 * these must be updated!
 */
extern void CTIset_stderr (FILE * new_stderr);
extern void CTFinitialize (void);
}

class BaseEnvironment : public ::testing::Environment {
    public:

    void SetUp();
    void TearDown();
};

#endif /* _SAC_BASE_TEST_ENVIRONMENT_H_ */
