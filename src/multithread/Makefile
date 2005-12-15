# 
# $Log$
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



OBJS := multithread_lib.o assignments_rearrange.o \
        tag_executionmode.o propagate_executionmode.o create_cells.o \
        create_dataflowgraph.o cell_growth.o create_withinwith.o \
        replicate_functions.o consolidate_cells.o 

all: $(OBJS) 


%.o: %.c 
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $< $(INCS)
	$(CLOCK_SKEW_ELIMINATION) $@


clean:
	$(RM) *.o *.a *.bak *~ .*.d
	$(RM) -r .sb


#######################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include ../../Makefile.Deps
