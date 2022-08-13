libsac2c and libsac Tests
=========================

This directory defines a collection of tests for libsac2c (the compiler
functions) and libsac (the runtime system).

Under the hood we use [gtest](https://github.com/google/googletest) together
with CMake's `ctest` to compile and run the tests. It is important to have
these installed and correctly setup on your system.

Adding a test
-------------

Each test file can contain one or more tests case, coupled together into what gtest
calls a _test suite_. Tests in general are constructed as minimally as possible,
with each test case doing only one kind of test (e.g. does some output match
some expected result)!

Below is a skeleton example of a test file; gtest is written in C++ and so it is
required that tests are defined using C++ (C code can still be included with `extern "C"`).

**NOTICE**: We can test against *two* distinct parts of the compiler, `libsac2c` which
defines compiler logic and `libsac` which defines the runtime system. These two systems
don't share any code. Importantly though, `libsac2c` expects some upfront configuration.
This configuration is defined in `base-test-environment.h` and **must** be includes in
all test files related to `libsac2c`. See the example below for how to do this:

```cpp
#include "gtest/gtest.h"

// XXX *only* include these two lines iff you are compiling against libsac2c!
#include "base-test-environment.h"
testing::Environment* base_test_env = testing::AddGlobalTestEnvironment(new BaseEnvironment);
// XXX

extern "C" {
// includes or other C-only code
}

TEST (TestSuiteName, TestName)
{
    // test code, see gtest manual for testing primitives
}

// more test cases....
```

Your file should be named `test-<test suite name>.cpp` and you must include it in the
`CMakeLists.txt` file using the add test macro, see:

For `libsac2c` test:
```cmake
ADD_FUNC_TEST (test-<test suite name> test-<test suite name>.cpp)
```

and for the runtime system, you need to additionally specify which runtime library
you are testing:
```cmake
ADD_RUNTIME_FUNC_TEST (test-<test suite name> test-<test suite name>.cpp "<TARGET, e.g. seq>" "<RT library name>")
```

With that, calling `make rebuild_cache` or cmake directly should add the test as a target to
the build system.
