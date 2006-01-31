#
# $Id$
#


###############################################################################
#
# include general setup:
#

PROJECT_ROOT := ../..

include $(PROJECT_ROOT)/Makefile.Source



###############################################################################
#
# subdirectory specific aspects
#


REV = $(shell svn info | grep Revision: | sed -e 's/Revision: //g')

.PHONY: build.c

build.c: 
	@echo "Creating file build.c"
	@echo "char build_date[] = \"`date`\";"  >  build.c
	@echo "char build_user[] = \"$(USER)\";" >> build.c
	@echo "char build_host[] = \"`hostname`\";" >> build.c
	@echo "char build_os[]   = \"$(OS)\";"   >> build.c
	@echo "char build_rev[]  = \"$(REV)\";"  >> build.c
	@$(CLOCK_SKEW_ELIMINATION) $@

