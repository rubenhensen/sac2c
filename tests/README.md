Unittests for sac2c
===================

The unit tests in this directory are supposed to ensure correct operation of
a specific functionality of `sac2c`.  Each unit test in its code encodes how
it should be run and what are its criteria for success.

The encoding is done using a series of line comments that start with a tag
`// SAC_TEST|` from the first column of any line.  `// SAC_TEST|...` lines
are read in order they appear in the file, and are combined in a makefile
by cutting `// SAC_TEST|` prefix.  Consider an example:

```
// SAC_TEST|all:
// SAC_TEST|<tab>echo "Hello"

int main ()
{
    return 0;
}
```
Will generate a Makefile:

```
all: test-trivial
	echo "Hello"
```

Note the `<tab>` tag in the second line of the sac program.  Even though
a tab symbol can be inserted directly in the text, it is often tedious to
convince your text editor to keep it unexpanded.  For more meta tags see
Meta tags section below.

## Running unit tests

Running unit tests is achieved via

```sh
make test
```
after the sac2c compiler has been built.   To run tests in parallel, which
can become important if the number of tests increases, use
```sh
ctests -j5
```

For each test, cmake generates a call to a script
`<cmake-build-dir>/tests/scripts/run.sh <sac-test-file>`.  For debugging
purposes, this process can be done manually:
```
cd build/tests
./scripts/run.sh test-trivial.sac
```
To inspect the makefile generated for the test, see `<sac-filename>.mk`
file, e.g. for the above example that would be `test-trivial.sac.mk`.

### Adding new unit tests

To add a new unit test add a file that is named `test-\*.sac` in tests
directory.

## A structure of a unit test

When writing a unit test we rely on a few conventions.  First, we keep
the size of each unit test as small as possible.  For that reason, unit tests
do not use standard library.  Common functionality, like basic wrappers around
builtin functions are defined in `mini-stdlib.sac` that should be used via
`#include "mini-stdlib.sac"`.

Secondly, we want to abstract generic functionality.  Therefore, we provide
a file called `common.mk` that contains convenient makefile definitions that
are often used.  It is very likely that every unit test should start with 
```
// SAC_TEST|include common.mk
```
definition.  Further, common functionality that is relevant to a set of unit
tests can be abstracted in the header files and sed via includes.  For example:

```sh
$ cat set-size-1024.h
// SAC_TEST|SAC2C_FLAGS += -DSIZE=1024
$ cat test-size.sac
// SAC_TEST|include common.mk
#include "set-size-1024.h"
// SAC_TEST|all: <file-name-we>
// SAC_TEST|<tab>./$<

int main ()
{
    return 1024 - SIZE;
}
```
This works, because when generating a makefile, we run a C preprocessor with
`-C` command, that preserves comments.

## Description of common.mk and scripts

The `common.mk` file gets correct path to `sac2c` via CMake and keeps it in
`SAC2C` variable.

The following variables are defined: `SAC2C_FLAGS`, `GREP_COMMAND_OUTPUT`,
`CHECK_RETURN_STATUS` and a generic rule to build sac programs from sac sources.
For explanation see comments in `common.mk`.

Several shell scripts are defined in scripts, to be used in `common.mk`:

  * `check-return-status.sh` --- runs the binary passed as a first argument
    and checks that its return status is equal to the second argument.
    
  * `grep-command-output.sh` --- a wrapper around grep that counts the number
    of occurrences of the first argument is equal to the second argument.

  * `runttest:


## Meta tags

  * `<file-name>` --- current file name **without the path**.  This gets useful
    for keeping test specification agnostic to file renaming.

  * `<file-name-we>` --- current file name **without the path** without
    extension, e.g. for /tmp/test-feature.sac this will be `test-feature`.

  * `<tab>` --- a tab symbol.  This is usefule for generating makefile rule.

  * `<nlslash>` --- expands to `\` followed by a newline.  This is needed if
     you want to encode makefile rule with linebreaks.

## Existing tests

One of the first goals would be to port the relevant tests from Bob's testsuite
in the sac2c repository.  The `testsuite-porting.txt` file describes the
progress.

Unfortunately a lot of tests there fail when you run them.  Currently it is very
difficult to understand whether this indicates a problem in the compiler or a
problem in a particular unit test.
