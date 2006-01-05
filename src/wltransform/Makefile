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

SAC2C_OBJS := wlpragma_funs.o wltransform.o wlidxs.o


all: $(SAC2C_OBJS)


#
# the wltransform.c object is to fool makedepend which does not
# recognize the dependency between wltransform.c and the .mac files!
#
wltransform.c: wlpragma_funs.mac
	touch $@


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
