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

OBJS := objinit.o resolve_reference_args.o \
        uniquecheck.o objanalysis.o restore_reference_args.o \
        extend_wrapper_types.o resolve_objects.o \
        restore_objects.o

all: $(OBJS)

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
