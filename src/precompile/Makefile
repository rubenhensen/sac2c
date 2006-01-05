#
#
# $Id: Makefile 14341 2005-10-25 14:34:36Z sah $
#
# 



################################################################################
#
# include general setup:
#

PROJECT_ROOT := ../..
include $(PROJECT_ROOT)/Makefile.Config


################################################################################
#
# local settings:
#

SAC2C_OBJS := precompile.o markmemvals.o \
              typeconv_precompile.o functionprecompile.o renameidentifiers.o \
              setlinksign.o remove_external_code.o mark_noop_grids.o


all: $(SAC2C_OBJS)


%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<
	$(CLOCK_SKEW_ELIMINATION) $@


clean:
	$(RM) *.o *.a *.bak *~ .*.d
	$(RM) -r .sb SunWS_cache


################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include ../../Makefile.Deps
