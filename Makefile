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
#  Parameters:
#    DEPS="no"    de-activate dependency checking meachanism 
#    HIDE=""      show important commands issued by make (debugging)
#
#  Refactor parameters:
#    PATTERN=""   pattern to look for in all source files
#    OUTPUT=""    text to replace matched pattern
#
###############################################################################


#######################################################################################
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
	@$(ECHO) ""
	@$(ECHO) "************************************************************"
	@$(ECHO) "* Building $(PROJECT_NAME)"
	@$(ECHO) "************************************************************"
	$(HIDE) $(MAKE) -C src/maketools DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/maketools/" PREFIX_ROOT="" $@
	$(HIDE) $(MAKE) -C src/libsac2c  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/libsac2c/"  PREFIX_ROOT="" $@
	$(HIDE) $(MAKE) -C src/runtime  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/runtime/"  PREFIX_ROOT="" $@
	$(HIDE) $(MAKE) -C src/libsac  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/libsac/"  PREFIX_ROOT="" $@
	$(HIDE) if [ "$(ENABLE_PHM)" = "yes" ]; then   \
                  $(MAKE) -C src/libsacphm  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                          PREFIX_LOCAL="src/libsacphm/"  PREFIX_ROOT="" $@ ; \
                fi
	$(HIDE) $(MAKE) -C src/tools     DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/tools/"     PREFIX_ROOT="" $@
	$(HIDE) $(MAKE) -C src/libsacprelude  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/libsacprelude/"  PREFIX_ROOT="" $@
	@$(ECHO) ""
	@$(ECHO) "************************************************************"
	@$(ECHO) "* Building $(PROJECT_NAME) completed"
	@$(ECHO) "************************************************************"
	@$(ECHO) ""


 
###############################################################################
#
# Cleaning rules
#

clean cleandevel cleanprod: checks
	@$(ECHO) ""
	@$(ECHO) "************************************************************"
	@$(ECHO) "* Cleaning $(PROJECT_NAME)"
	@$(ECHO) "************************************************************"
	$(HIDE) if [ "$(CLEAN_MAKE_TOOLS)" = "yes" ]; then  \
                  $(MAKE) -C src/maketools DEPS="$(DEPS)" HIDE="$(HIDE)" \
                          PREFIX_LOCAL="src/maketools/" PREFIX_ROOT="" $@ ; \
                fi
	$(HIDE) $(MAKE) -C src/libsac2c  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/libsac2c/"  PREFIX_ROOT="" $@
	$(HIDE) $(MAKE) -C src/runtime  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/runtime/"  PREFIX_ROOT="" $@
	$(HIDE) $(MAKE) -C src/libsacprelude  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/libsacprelude/"  PREFIX_ROOT="" $@
	$(HIDE) $(MAKE) -C src/libsac  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/libsac/"  PREFIX_ROOT="" $@
	$(HIDE) $(MAKE) -C src/libsacphm  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/libsacphm/"  PREFIX_ROOT="" $@
	$(HIDE) $(MAKE) -C src/tools     DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/tools/"     PREFIX_ROOT="" $@
	$(HIDE) $(RM) -rf lib/* bin/*
	@$(ECHO) ""
	@$(ECHO) "************************************************************"
	@$(ECHO) "* Cleaning $(PROJECT_NAME) completed"
	@$(ECHO) "************************************************************"
	@$(ECHO) ""


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
	@$(ECHO) ""
	@$(ECHO) "************************************************************"
	@$(ECHO) "* Refactoring $(PROJECT_NAME)"
	@$(ECHO) "************************************************************"
	$(HIDE) $(MAKE) -C src/maketools DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/maketools/" PREFIX_ROOT="" $@
	$(HIDE) $(MAKE) -C src/libsac2c  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/libsac2c/"  PREFIX_ROOT="" $@
	$(HIDE) $(MAKE) -C src/runtime  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/runtime/"  PREFIX_ROOT="" $@
	$(HIDE) $(MAKE) -C src/libsacprelude  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/libsacprelude/"  PREFIX_ROOT="" $@
	$(HIDE) $(MAKE) -C src/libsac  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/libsac/"  PREFIX_ROOT="" $@
	$(HIDE) $(MAKE) -C src/libsacphm  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/libsacphm/"  PREFIX_ROOT="" $@
	$(HIDE) $(MAKE) -C src/tools     DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/tools/" PREFIX_ROOT="" $@
	@$(ECHO) ""
	@$(ECHO) "************************************************************"
	@$(ECHO) "* Refactoring $(PROJECT_NAME) completed"
	@$(ECHO) "************************************************************"
	@$(ECHO) ""


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

libsacprelude: checks
	$(HIDE) $(MAKE) -C src/libsacprelude  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/libsacprelude/"  PREFIX_ROOT="" default

libsac: checks
	$(HIDE) $(MAKE) -C src/libsac  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/libsac/"  PREFIX_ROOT="" default

libsacphm: checks
	$(HIDE) $(MAKE) -C src/libsacphm  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/libsacphm/"  PREFIX_ROOT="" default

tools: checks
	$(HIDE) $(MAKE) -C src/tools     DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/tools/"     PREFIX_ROOT="" default


###############################################################################
#
# Rules to create configure the script. Put it here as a first target.
#

config:
	@ cd setup && \
          aclocal -I config && \
          autoreconf -v -f -i


###############################################################################
#
# Includes for consistency checking mechanism
#

include $(MAKEFILE_DIR)/check.mkf



listinstfiles = $(shell for f in $$(ls -1 $(1)); do echo "\"$(PREFIX)/$(1)/$$f\"" ; done)
listfiles = $(shell for f in $$(ls -1 $(1)); do echo "\"$(1)/$$f\"" ; done)
install:
	echo $(call listinstfiles,include) > .uninstall
	echo $(call listinstfiles,lib) >> .uninstall
	echo $(call listinstfiles,bin) >> .uninstall
	echo "$(PREFIX)/share/sac2crc" >> .uninstall
	install -d -m 0755 "$(PREFIX)/bin"
	install -d -m 0755 "$(PREFIX)/lib"
	install -d -m 0755 "$(PREFIX)/include"
	install -d -m 0755 "$(PREFIX)/share"
	install -m 0755 $(call listfiles,include) "$(PREFIX)/include"
	install -m 0755 $(call listfiles,lib) "$(PREFIX)/lib"
	install -m 0755 $(call listfiles,bin) "$(PREFIX)/bin"
	install -m 0755 sac2crc "$(PREFIX)/share"

uninstall:
	if [ -f .uninstall ]; then \
	    for f in $$(cat .uninstall); do \
	        $(RM) $$f ; \
	    done \
	fi
