#
#
# $Log$
# Revision 3.28  2003/02/08 16:00:01  mwe
# DistributiveLaw.o added
#
# Revision 3.27  2002/10/08 22:10:47  dkr
# SSAWLUnroll.o added
#
# Revision 3.26  2002/06/07 17:16:42  mwe
# AssociativeLaw.o added
#
# Revision 3.25  2002/06/06 19:56:03  dkr
# target clean modified
#
# Revision 3.24  2001/11/15 15:36:48  sacbase
# CLOCK_SKEW_ELIMINATION and PROJECT_ROOT added.
#
# Revision 3.23  2001/11/14 12:59:48  sbs
# deps line replaced by inclusion of Makefile.Deps
# ( .xxx.d mechanism)
# global setup included via Makefile.Config
#
# Revision 3.22  2001/11/13 20:32:43  dkr
# make deps done
#
# Revision 3.21  2001/07/18 13:08:17  cg
# make deps done
#
# Revision 3.20  2001/05/30 14:00:40  nmw
# SSAInferLI.o added
#
# Revision 3.19  2001/05/22 14:52:18  nmw
# rmcasts.o added
#
# Revision 3.18  2001/05/08 13:29:33  dkr
# make deps done
#
# Revision 3.17  2001/04/26 21:09:44  dkr
# make deps done
#
# Revision 3.16  2001/04/20 11:17:07  nmw
# SSALUR.o added
#
# Revision 3.15  2001/03/26 15:35:35  nmw
# SSALIR added
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

INCS = -I../global -I../../inc -I../typecheck -I../print -I../psi-opt -I../modules \
       -I../compile -I../concurrent -I../tree -I../flatten -I../constants

all: optimize.o generatemasks.o ConstantFolding.o DeadCodeRemoval.o \
     LoopInvariantRemoval.o Inline.o Unroll.o Unswitch.o CSE.o \
     WLUnroll.o DeadFunctionRemoval.o freemasks.o \
     SSADeadCodeRemoval.o SSACSE.o SSAConstantFolding.o SSALIR.o SSALUR.o \
     SSAWLUnroll.o SSAInferLI.o AssociativeLaw.o rmcasts.o DistributiveLaw.o


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
