#
#
# $Log$
# Revision 3.36  2004/11/26 21:05:23  sah
# *** empty log message ***
#
# Revision 3.35  2004/11/21 11:22:03  sah
# removed some old ast infos
#
# ... [eliminated]
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

OBJS := optimize.o Inline.o DeadFunctionRemoval.o \
        SSADeadCodeRemoval.o SSACSE.o SSAConstantFolding.o SSALIR.o SSALUR.o \
        SSAWLUnroll.o SSAInferLI.o AssociativeLaw.o rmcasts.o \
        UndoElimSubDiv.o ElimSubDiv.o SelectionPropagation.o \
        ConstVarPropagation.o DistributiveLaw.o 

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
