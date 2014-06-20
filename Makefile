###############################################################################
#
# Calling conventions:
#
#  Start rules:
#    default      compile executables as developer code and libraries as product
#                 code
#    devel        compile everything as developer code
#    prod         compile everything as product code
#
#    clean        cleanup all derived files
#    cleandevel   cleanup only developer compiled files
#    cleanprod    cleanup only product compiled files
#
#    tidy         cleanup all derived files excluding make tools
#    tidydevel    cleanup only developer compiled files excluding make tools
#    tidyprod     cleanup only product compiled files excluding make tools
#
#    refactor     refactor source code (requires parameters below)
#
#    config       rebuild autoconf-gnerated scripts, e.g. configure,
#                 src/include/config.h.in, etc.
#
#    install      install sac2c under the $PREFIX directory.  Please note
#                 that $PREFIX can be set when you run ./configure via
#                 ./configure --prefix=/path/to/some/location
#                 Also, make sure that your $PREFIX/bin is on the PATH.
#
#    uninstall    Remove the files created by the install target.  Please
#                 note that it does not remove a directory structure, only
#                 the files.
#
#  Parameters:
#    DEPS="no"    de-activate dependency checking meachanism
#    HIDE=""      show important commands issued by make (debugging)
#
#  Refactor parameters:
#    PATTERN=""   pattern to look for in all source files
#    OUTPUT=""    text to replace matched pattern
#
###############################################################################


###############################################################################
#
# general setup:
#

PROJECT_ROOT := .

HIDE := @
DEPS := yes

CLEAN_MAKE_TOOLS := "yes"

MAKEFILE_DIR := $(PROJECT_ROOT)/src/makefiles

-include $(MAKEFILE_DIR)/config.mkf   # config.mkf may not yet exist
include $(MAKEFILE_DIR)/settings.mkf

# A function that outputs an argument surrounded by the box of stars.
# Like this:
#    ********
#    * arg
#    ********
# The first and second line are expansions of $(frame_stars) variable.
frame_stars := "************************************************************"
framed_text = @$(ECHO) -e "$(frame_stars)\n* $(1)\n$(frame_stars)"


###############################################################################
#
# Dummy rules
#

.PHONY: default devel prod clean cleandevel cleanprod config \
        tidy tidydevel tidyprod


###############################################################################
#
# Start rules
#
# The definition of these rules deliberately enforces a sequence in compilation
# rather than expressing the dependencies properly by makefile rules.
#
# The rationale is that commonlib and maketools rather seldomly require
# recompilation. With proper dependencies, however, they would require a
# dependency from every compilation rule. This would lead to extensive
# rechecking at compile time that is absolutely superfluous.
#
# The runtime system may require the compiler and, likewise, the tools may
# need the runtime system or parts thereof. Expressing all this by dependencies
# would lead to a system in which the tools form the main target. This seems
# unnatural.
#
# Furthermore, the current solution allows us to rebuild locally without
# enforcing dependency checks.
#

default devel prod: checks
	$(call framed_text,Building $(PROJECT_NAME))
	$(HIDE) $(MAKE) -C src/maketools DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/maketools/" PREFIX_ROOT="" $@
	$(HIDE) $(MAKE) -C src/libsac2c  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/libsac2c/"  PREFIX_ROOT="" $@
	$(HIDE) $(MAKE) -C src/runtime  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/runtime/"  PREFIX_ROOT="" $@
	$(HIDE) $(MAKE) -C src/tools     DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/tools/"     PREFIX_ROOT="" $@
	$(HIDE) for t in $(BUILD_TARGETS); do \
	  $(MAKE) -f $(MAKEFILE_DIR)/rtlibs.mkf HIDE="$(HIDE)" TARGET=$$t all || exit 1; \
	done
	$(call framed_text,Building $(PROJECT_NAME) completed)


###############################################################################
#
# Cleaning rules
#

clean cleandevel cleanprod: checks
	$(call framed_text,Cleaning $(PROJECT_NAME))
	$(HIDE) if [ "$(CLEAN_MAKE_TOOLS)" = "yes" ]; then  \
                  $(MAKE) -C src/maketools DEPS="$(DEPS)" HIDE="$(HIDE)" \
                          PREFIX_LOCAL="src/maketools/" PREFIX_ROOT="" $@ ; \
                fi
	$(HIDE) $(MAKE) -C src/libsac2c  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/libsac2c/"  PREFIX_ROOT="" $@
	$(HIDE) $(MAKE) -C src/runtime  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/runtime/"  PREFIX_ROOT="" $@
	$(HIDE) $(MAKE) -C src/tools     DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/tools/"     PREFIX_ROOT="" $@
	$(HIDE) for t in $(BUILD_TARGETS); do \
	    $(MAKE) -f $(MAKEFILE_DIR)/rtlibs.mkf HIDE="$(HIDE)" TARGET=$$t clean || exit 1; \
	done
	$(HIDE) $(RM) -rf lib/* bin/*
	$(call framed_text,Cleaning $(PROJECT_NAME) completed)


tidy:
	$(MAKE) CLEAN_MAKE_TOOLS="no" clean

tidydevel:
	$(MAKE) CLEAN_MAKE_TOOLS="no" cleandevel

tidyprod:
	$(MAKE) CLEAN_MAKE_TOOLS="no" cleanprod


###############################################################################
#
# Refactoring rules
#

refactor: checks
	$(call framed_text,Refactoring $(PROJECT_NAME))
	$(HIDE) $(MAKE) -C src/maketools DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/maketools/" PREFIX_ROOT="" $@
	$(HIDE) $(MAKE) -C src/libsac2c  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/libsac2c/"  PREFIX_ROOT="" $@
	$(HIDE) $(MAKE) -C src/runtime  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/runtime/"  PREFIX_ROOT="" $@
	$(HIDE) $(MAKE) -C src/tools     DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/tools/" PREFIX_ROOT="" $@
	$(call framed_text,Refactoring $(PROJECT_NAME) completed)


###############################################################################
#
# Rules for making selected subprojects in default mode
#

maketools: checks
	$(HIDE) $(MAKE) -C src/maketools DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/maketools/" PREFIX_ROOT="" default

libsac2c: checks
	$(HIDE) $(MAKE) -C src/libsac2c  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/libsac2c/"  PREFIX_ROOT="" default

runtime: checks
	$(HIDE) $(MAKE) -C src/runtime  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/runtime/"  PREFIX_ROOT="" default

tools: checks
	$(HIDE) $(MAKE) -C src/tools     DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/tools/"     PREFIX_ROOT="" default

rtlibs: checks
	$(HIDE) for t in $(BUILD_TARGETS); do \
	   $(MAKE) -f $(MAKEFILE_DIR)/rtlibs.mkf HIDE="$(HIDE)" TARGET=$$t || exit 1; \
	done

###############################################################################
#
# Rules to create configure the script. Put it here as a first target.
#


config:
	autoreconf -v -f -i -I setup/config

###############################################################################
#
# Includes for consistency checking mechanism
#

include $(MAKEFILE_DIR)/check.mkf


install: checks
	$(INSTALL) -d "$(INCPATH_CONF)"
	$(INSTALL) -d "$(SAC2CRC_DIR)"
	$(INSTALL) -d "$(bindir)"
	$(INSTALL) -d $(DLL_DIR)
	$(RM) .uninstall
	for i in include/*.h; do \
	  dst="$(INCPATH_CONF)"/`basename $$i`; \
	  $(ECHO) "$$dst" >>.uninstall; \
	  $(INSTALL_DATA) $$i "$$dst"; \
	done
	$(ECHO) "$(SAC2CRC_CONF)" >>.uninstall
	$(INSTALL_DATA) setup/sac2crc "$(SAC2CRC_CONF)"
	for i in bin/*; do \
	  dst1="$(DLL_DIR)"/`basename $$i`; \
	  $(ECHO) "$$dst1" >>.uninstall; \
	  $(INSTALL_PROGRAM) $$i "$$dst1"; \
	  dst2="$(bindir)"/`basename $$i`; \
	  $(ECHO) "$$dst2" >>.uninstall; \
	  $(RM) "$$dst2"; $(LN) "$$dst1" "$$dst2"; \
	done
	for i in lib/*.*; do \
	  dst="$(DLL_DIR)"/`basename $$i`; \
	  $(ECHO) "$$dst" >>.uninstall; \
	  $(INSTALL_PROGRAM) $$i "$$dst"; \
	done
	for t in $(BUILD_TARGETS); do \
	  $(MAKE) -f $(MAKEFILE_DIR)/rtlibs.mkf HIDE="$(HIDE)" TARGET=$$t install; \
	done


uninstall:
	if test -r .uninstall; then \
	  cat .uninstall | while read f; do $(RM) "$$f"; done; \
	fi
