#
#
# $Id$
#
#


#######################################################################################
#
# include general setup:
#

PROJECT_ROOT := ../..
include $(PROJECT_ROOT)/Makefile.Config


#######################################################################################
#
# local settings:
#
all: wl_access_analyze.o tile_size_inference.o \
     SSAWithloopFolding.o SSAWLT.o SSAWLI.o SSAWLF.o \
     wls.o wlscheck.o wlsbuild.o wlswithloopification.o \
     pad.o pad_info.o pad_collect.o pad_infer.o pad_transform.o \
     WithloopFusion.o detectdependencies.o tagdependencies.o \
     resolvedependencies.o index_infer.o index_eliminate.o \
     index_optimize.o


#
# the *.[ch] objects are to fool makedepend which does not
# recognize the dependency between *.[ch] and the .mac files!
#
pad_infer.c: ../tree/type_info.mac
	touch $@
	$(CLOCK_SKEW_ELIMINATION) $@


%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o $@ -c $< $(INCS)
	$(CLOCK_SKEW_ELIMINATION) $@


clean:
	$(RM) *.o *.a *.bak *~ .*.d
	$(RM) -r .sb SunWS_cache


#######################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include ../../Makefile.Deps
