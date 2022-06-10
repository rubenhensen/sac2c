/**
 * @file base-test-environment.h
 * Including this file in a test suite registers the base environment
 * to the GoogleTest framework and makes it so the SetUp() and TearDown()
 * functions are run before and after every test.
 */

#include "base-test-environment.h"

extern "C" {
#include "ctformatting.h"
#include "ctinfo.h"
}

void BaseEnvironment::SetUp()
{
    // Compile time formatting must be initialized to properly throw errors.
    CTIset_stderr (stderr);
    CTFinitialize ();
}

void BaseEnvironment::TearDown()
{
    // Empty for now
}
