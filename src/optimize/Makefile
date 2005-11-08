#
#
# $Log$
# Revision 3.47  2005/09/12 13:56:38  ktr
# added wlsimplification.o
#
# Revision 3.46  2005/09/02 14:25:45  ktr
# added liftoptflags.o
#
# Revision 3.45  2005/08/20 12:06:11  ktr
# added elimtypeconv.o
#
# Revision 3.44  2005/08/19 18:16:38  ktr
# added inferneededcounters.o, associativity.o
#
# Revision 3.43  2005/08/02 14:24:37  ktr
# a seperate dead code inference traversal is now employed
#
# Revision 3.42  2005/07/19 16:57:14  ktr
# replaced SSADeadCodeRemoval with deadcoderemoval.
#
# Revision 3.41  2005/04/20 19:13:23  ktr
# removed Inline.o
#
# Revision 3.40  2005/02/14 11:20:52  cg
# Added compilation of inlining.c
#
# Revision 3.39  2005/02/02 18:08:14  mwe
# signature_simplification added
#
# Revision 3.38  2004/12/08 12:16:14  mwe
# type_upgrade added
#
# Revision 3.37  2004/11/26 21:08:51  sbs
# SelectionPropagation.o eliminated from OBJS
#
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

OBJS := optimize.o DeadFunctionRemoval.o deadcoderemoval.o \
        SSACSE.o SSAConstantFolding.o SSALIR.o SSALUR.o \
        SSAWLUnroll.o SSAInferLI.o AssociativeLaw.o rmcasts.o \
        UndoElimSubDiv.o ElimSubDiv.o deadcodeinference.o \
        ConstVarPropagation.o DistributiveLaw.o type_upgrade.o \
        signature_simplification.o inlining.o associativity.o \
        inferneedcounters.o elimtypeconv.o liftoptflags.o \
        wlsimplification.o prfunroll.o

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
