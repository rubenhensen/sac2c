
#
# $Id$
#


###############################################################################
#
# Calling conventions:
#
#  Start rules: 
#    default    compile executables as developer code and libraries as product 
#               code
#    devel      compile everything as developer code
#    prod       compile everything as product code
#    clean      cleanup all derived files
#    cleandevel cleanup only compiled files (developer code)
#    cleanprod  cleanup only compiled files (product code)
#
#  Parameters:
#    DEPS="no"  de-activate dependency checking meachanism 
#    HIDE=""    show important commands issued by make (debugging)
#
###############################################################################


#######################################################################################
#
# general setup:
#

PROJECT_ROOT := .

HIDE := @
DEPS := yes

MAKEFILE_DIR := $(PROJECT_ROOT)/src/makefiles

-include $(MAKEFILE_DIR)/config.mkf   # config.mkf may not yet exist
include $(MAKEFILE_DIR)/settings.mkf


###############################################################################
#
# Dummy rules
#

.PHONY: default devel prod clean cleandevel cleanprod efence config


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
	$(HIDE) $(MAKE) -C src/libsacprelude  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/libsacprelude/"  PREFIX_ROOT="" $@
	$(HIDE) $(MAKE) -C src/libsac  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/libsac/"  PREFIX_ROOT="" $@
	$(HIDE) $(MAKE) -C src/libsacphm  DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/libsacphm/"  PREFIX_ROOT="" $@
	$(HIDE) $(MAKE) -C src/tools     DEPS="$(DEPS)" HIDE="$(HIDE)" \
                        PREFIX_LOCAL="src/tools/"     PREFIX_ROOT="" $@
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
                        PREFIX_LOCAL="src/tools/"     PREFIX_ROOT="" $@
	$(HIDE) $(RM) lib/* bin/*
	@$(ECHO) ""
	@$(ECHO) "************************************************************"
	@$(ECHO) "* Cleaning $(PROJECT_NAME) completed"
	@$(ECHO) "************************************************************"
	@$(ECHO) ""

efence:
	$(HIDE) $(MAKE) -C src/compiler  DEPS="$(DEPS)" HIDE="$(HIDE)" $@


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
# Rules for configure mechanism
#

config: 
	svn lock configure $(AUTOHEADERED)
	autoconf
	autoheader
	svn commit configure $(AUTOHEADERED)


###############################################################################
#
# Includes for consistency checking mechanism
#

include $(MAKEFILE_DIR)/check.mkf



