End-to-end tests for sac2c
===================

The end-to-end tests in this directory are supposed to ensure correct operation of
a specific functionality of `sac2c`.  Each test file encodes how
it should be run and what its criteria are for success.

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

## Running end-to-end tests

Running end-to-end tests is achieved via

```sh
make check
```
after the sac2c compiler has been built.   To run tests in parallel, which
can become important if the number of tests increases, use
```sh
ctest -j5
```

For each test, cmake generates a call to a script
`<cmake-build-dir>/tests/scripts/run.sh <sac-test-file> <cmake-build-dir>/tests`.
For debugging purposes, this process can be done manually:
```sh
cd <cmake-build-dir>/tests
# Running a test in the test directory
./scripts/run.sh test-trivial.sac $PWD
# Running a test in a subdirectory
./scripts/run.sh errors/test-generic-error.sac $PWD
```

Additionally, the script can be used to run all tests in a folder using the -r flag:
```sh
cd <cmake-build-dir>/tests
./scripts/run.sh -r errors $PWD
```

To inspect the makefile generated for the test, see the `<sac-filename>.mk`
file, e.g. for one of the above example that would be `test-trivial.sac.mk`.

### Adding new end-to-end tests

To add a new end-to-end test, add a file that is named `test-\*.sac` in tests
directory. The test should include build/run instructions using the `SAC_TEST`
keyword. Tests can depend on specific features being available in sac2c, you
can use the `REQFEATURES` keyword to marks these. See in the next section for
further details.

Once you've added your test, make sure you run the following command before triggering new checks!
```sh
make rebuild_cache
```

## Structure of an end-to-end test

When writing an end-to-end test we rely on a few conventions.  First, we keep
the size of each end-to-end test as small as possible.  For that reason, end-to-end tests
do not use the standard library. Common functionality, like basic wrappers around
builtin functions are defined in `mini-stdlib.sac` that should be used via
`#include "mini-stdlib.sac"`.

Secondly, we want to abstract generic functionality. Therefore, we provide
a file called `common.mk` that contains convenient makefile definitions that
are often used.  It is very likely that every end-to-end test should start with 
```
// SAC_TEST|include common.mk
```
definition.  Further, common functionality that is relevant to a set of end-to-end
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

Tests sometimes depend on certain sac2c features; if a feature isn't compiled in,
we shouldn't run the test. Feature dependencies are given using the `REQFEATURES`
keyword, which contains a list of (space seperated) `ENABLE_*` terms relating to
a specfic feature. For example, if you have some test that depends on CUDA being
available, you would place this into your test file:

```c
// REQFEATURES|ENABLE_CUDA
// SAC_TEST|include common.mk
// ... more build/run specification here ...
```

At times, it is desirable to run multiple checks on the sac output. In such scenarios, the output can be stored and reused as such:
```c
// SAC_TEST|all: <file-name>
// SAC_TEST|<tab>OUTPUT=`$(CALL_SAC2C) $<`; <nlslash>
// Ensure we get exactly three lines as an output:
// SAC_TEST|<tab>echo "$$OUTPUT" | $(GREP_COMMAND_OUTPUT) "" 3; <nlslash>
// Ensure one of those lines contains "TEST"
// SAC_TEST|<tab>echo "$$OUTPUT" | $(GREP_COMMAND_OUTPUT) "test" 1; <nlslash>
```

This leads to the makefile
```
all: <example-file-name.sac>
  OUTPUT=`$(CALL_SAC2C) $<; \
  echo "$$OUTPUT" | $(GREP_COMMAND_OUTPUT) "" 3; \
  echo "$$OUTPUT" | $(GREP_COMMAND_OUTPUT) "test" 1; \

```  

The `<nlslash>` meta tag gets transformed into a backslash. The backslash, in combination with either `;` or `&&` allows for the subsequent commands to happen in the same shell session. If the commands were executed in different shell sessions, the variable `OUTPUT` would be lost.  
While debugging or creating the tests, your first instinct should be to look for a missing `&& <nlslash>`.

## Description of common.mk and scripts

The `common.mk` file gets correct path to `sac2c` via CMake and keeps it in
`SAC2C` variable.

The following variables are defined: `SAC2C_FLAGS`, `GREP_COMMAND_OUTPUT`,
`CHECK_RETURN_STATUS`, `ISALLOPTIONSON`, and a generic rule to build sac programs
from SaC sources. For explanation see comments in `common.mk`.

Several shell scripts are provided in the `scripts/` directory, some of these
can be used as part of the test operation:

  * `check-return-status.sh` --- runs the binary passed as a first argument
    and checks that its return status is equal to the second argument.

  * `grep-command-output.sh` --- a wrapper around grep that counts the number
    of occurrences of the first argument is equal to the second argument.

  * `isalloptionson.sh` --- checks the output of `sac2c -VV` for feature flags,
    and returns TRUE iff all arguments are marked as ON. This script is useful
    for changinge behaviour of a test at runtime.

The tests themselves are defined inline with the test code, using the `TEST_SAC`
tag. The following script scans the test file for this tags, and converts it into
a Makefile which is ultimately used to run the test(s):

  * `run.sh` --- the script which generates the Makefile and runs the
    specified tests. Can be called directly to run individual tests for debugging.

## Meta tags

  * `<file-name>` --- current file name **without the path**.  This gets useful
    for keeping test specification agnostic to file renaming.

  * `<file-name-we>` --- current file name **without the path** without
    extension, e.g. for /tmp/test-feature.sac this will be `test-feature`.

  * `<file-name-strip>` --- current file name **without the path** without
    extension, and without non-identifier-friendly characters, e.g. for
    /tmp/test-feature.sac this will be `testfeature`.

  * `<tab>` --- a tab symbol.  This is usefule for generating makefile rule.

  * `<nlslash>` --- expands to `\` followed by a newline.  This is needed if
     you want to encode makefile rule with linebreaks.
