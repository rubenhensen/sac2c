#
#
# $Log$
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
# Revision 3.14  2001/03/22 20:20:00  dkr
# make deps done
#
# Revision 3.13  2001/03/22 20:19:00  dkr
# INCS corrected
#
# Revision 3.12  2001/03/21 15:51:38  nmw
# include ../constants added
#
# Revision 3.11  2001/03/21 11:19:55  dkr
# make deps done
#
# Revision 3.10  2001/03/20 16:17:00  nmw
# SSAConstantFolding.o added
#
# Revision 3.9  2001/03/09 11:53:40  sbs
# make deps done
#
# Revision 3.8  2001/03/05 16:03:23  nmw
# SSACSE added
#
# Revision 3.7  2001/03/02 16:13:52  dkr
# make deps done
#
# Revision 3.6  2001/02/23 13:38:39  nmw
# SSADeadCodeRemoval added
#
# Revision 3.5  2001/02/13 15:54:04  nmw
# include for lac2fun.h added
#
# Revision 3.4  2001/02/02 10:39:01  dkr
# make deps done
#
# Revision 3.3  2001/01/17 10:13:03  dkr
# make deps done
#
# Revision 3.2  2000/12/06 20:09:02  dkr
# make deps done
#
# Revision 3.1  2000/11/20 18:00:34  sacbase
# new release made
#
#
# ... [eliminated]
#
# Revision 1.1  1994/12/09  10:48:23  sbs
# Initial revision
#
#



#######################################################################################
#
# include general setup:
#

include ../../Makefile.Config


#######################################################################################
#
# local settings:
#

INCS=-I../global -I../../inc -I../typecheck -I../print -I../psi-opt -I../modules \
     -I../compile -I../concurrent -I../tree -I../flatten -I../constants

all: optimize.o generatemasks.o ConstantFolding.o DeadCodeRemoval.o \
     LoopInvariantRemoval.o Inline.o Unroll.o Unswitch.o CSE.o \
     WLUnroll.o DeadFunctionRemoval.o freemasks.o \
     SSADeadCodeRemoval.o SSACSE.o SSAConstantFolding.o SSALIR.o SSALUR.o \
     rmcasts.o SSAInferLI.o

%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o $@ -c $< $(INCS)

clean:
	$(RM) *.o *.bak .*.d
	$(RM) -r .sb SunWS_cache

#######################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include ../../Makefile.Deps



