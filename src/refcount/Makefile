#
# $Id$
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
all: allocation.o alloc.o referencecounting.o reuse.o rcopt.o rcphase.o \
     filterrc.o aliasanalysis.o staticreuse.o reuseelimination.o \
     interfaceanalysis.o loopreuseopt.o datareuse.o explicitcopy.o \
     reusebranching.o ReuseWithArrays.o inplacecomp.o audscldist.o \
     NumLookUpTable.o rcminimize.o wrci.o reusewithoffset.o


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
