The Single-Assignment C Compiler
================================

[![ISC License](https://img.shields.io/badge/license-ISC-green.svg)](https://gitlab.sac-home.org/sac-group/sac2c/-/blob/develop/LICENSE.md) [![Latest release](https://img.shields.io/gitlab/v/release/sac-group/sac-packages?gitlab_url=https%3A%2F%2Fgitlab.sac-home.org&include_prereleases&sort=date)](https://gitlab.sac-home.org/sac-group/sac-packages/-/releases)

The project encompasses the Single-Assignment C Compiler (called _sac2c_) sources.

Building
--------

To get you started, you will need a recent version of [CMake][cmake] (at least version
3.19), a C99-compatible C compiler (`gcc` or `clang`), and the libUUID library (either
packaged as `libuuid-dev` *or* as part of util-linux).

To build (_quick and dirty_):

```sh
$ make
```

This will create two directories, `build_r` and `build_p`, in which the _sac2c_ compiler
will be compiled using the default configuration. Once this is done, you can run _sac2c_
from each directory directly, no install is needed.

### The long way

To build with custom options, or with a different C compiler other than the system
compiler:

```sh
$ mkdir build && cd build
$ cmake -DCMAKE_BUILD_TYPE=<type> -DCMAKE_C_COMPILER=/some/other/cc -DCMAKE_CXX_COMPILER=/some/other/c++ -DCUDA=OFF ..
$ make -j4
```
where `<type>` is either `DEBUG` or `RELEASE` (for compiling the debug and production
versions).

Further supported flags are stored in `cmake/options.cmake`.

Testing
-------

In order to test different parts of _sac2c_ we use [CTest][ctest] together with a custom
testing framework.  We test two parts of the compiler, its internals and its operations.

By default we have tests configured to tests its operations, you can call `make check` to
run these tests. (Running `make test` is *discouraged*!)

### Testing the internals

In order to test internals, you will need to additionally install [googletest][googt] and
activate testing of internals by re-configuring your build:

```sh
$ cmake -DFUNCTESTS=ON <OTHER-FLAGS> ..
```
where `<OTHER-FLAGS>` are the flags you initial used to configure the compiler.

After this, you can call `make check` to run the tests (which runs the internal tests
_together_ with the operational tests).

### Advanced testing (debugging)

If tests pass, that's great, but sometimes they do not 😞. The `make check` target wraps
around CTest, as such you can call `ctest` in the build directory for more fine-grained
control:

```sh
# to list all tests (without running)
$ ctest -N
# to run a specific test
$ ctest -R <test-name>
# to run a specific test with more verbosity
$ ctest -V --output-on-failure -R <test-name>
```

Through this you can see how test binaries are called, and what flags are passed to
_sac2c_.

For further details on test setup, see the documentation in `tests/` and `src/tests`.

License
-------

The compiler is OSS, please have a look at [LICENSE.md][license] and [THIRDPARTY.md][trdprty]
for more details.

[cmake]: https://cmake.org/
[ctest]: https://cmake.org/cmake/help/latest/manual/ctest.1.html
[googt]: https://github.com/google/googletest
[license]: https://gitlab.sac-home.org/sac-group/sac2c/-/blob/develop/LICENSE.md
[trdprty]: https://gitlab.sac-home.org/sac-group/sac2c/-/blob/develop/THIRDPARTY.md
