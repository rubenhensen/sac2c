#
#
# $Log$
# Revision 3.37  2005/07/03 17:02:38  ktr
# added code simplification
#
# Revision 3.36  2005/06/28 20:57:09  cg
# Added while2do.[ch] and handle_condexpr.[ch]
#
# Revision 3.35  2005/06/14 08:52:04  khf
# added wldefaultpartition.o
#
# Revision 3.34  2005/05/13 16:36:57  ktr
# added lacinlining.o
#
# Revision 3.33  2005/04/29 20:31:11  khf
# added wlanalysis.o
#
# Revision 3.32  2004/11/27 03:08:15  sbs
# *** empty log message ***
#
# Revision 3.31  2004/11/26 16:18:58  sbs
# INCS eliminated
#
# Revision 3.30  2004/11/18 14:34:31  mwe
# changed CheckAvis and chkavis to ToNewTypes and to tonewtypes
#
# Revision 3.29  2004/07/21 12:40:38  khf
# WLEnhancement.o and ExplizitAccumulate.o added
#
# Revision 3.28  2004/02/26 13:04:21  khf
# WLPartitionGeneration.o added
#
# Revision 3.27  2004/02/25 08:17:44  cg
# Elimination of while-loops by conversion into do-loops with
# leading conditional integrated into flatten.
# Separate compiler phase while2do eliminated.
# NO while-loops may occur after flatten.
# While-loop specific code eliminated.
#
# Revision 3.26  2004/02/02 15:49:35  skt
# ssa.o added
#
# Revision 3.25  2004/01/28 18:02:41  skt
# some adaptions, because of moving of CheckAvis.[hc],
# SSATransform.[hc] and UndoSSATransform.[hs] from
# src/tree to src/flatten
#
# Revision 3.24  2003/09/09 12:53:33  sbs
# -I../typecheck added
#
# Revision 3.23  2003/06/11 21:40:03  ktr
# included ../constants
#
# Revision 3.22  2002/08/13 10:22:18  sbs
# handle_mops.o added
#
# Revision 3.21  2002/06/06 19:56:03  dkr
# target clean modified
#
# Revision 3.20  2002/02/21 14:21:21  sbs
# insert_vardec.o added as target
#
# Revision 3.19  2001/11/15 15:36:48  sacbase
# CLOCK_SKEW_ELIMINATION and PROJECT_ROOT added.
#
# Revision 3.18  2001/11/14 12:59:48  sbs
# deps line replaced by inclusion of Makefile.Deps
# ( .xxx.d mechanism)
# global setup included via Makefile.Config
#
# Revision 3.17  2001/11/13 20:32:40  dkr
# make deps done
#
# Revision 3.16  2001/07/18 13:08:15  cg
# make deps done
#
# Revision 3.15  2001/04/26 21:09:40  dkr
# make deps done
#
# Revision 3.14  2001/04/18 15:39:09  nmw
# while2to.o added
#
# Revision 3.13  2001/03/27 17:45:09  dkr
# INCS updated
#
# Revision 3.12  2001/03/22 20:19:56  dkr
# make deps done
#
# Revision 3.11  2001/03/12 12:25:39  dkr
# make deps done
#
# Revision 3.10  2001/03/09 11:53:38  sbs
# make deps done
#
# ... [eliminated] ...
#
# Revision 1.3  1994/11/10  15:39:42  sbs
# RCS-header inserted
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


all: flatten.o lac2fun.o fun2lac.o insert_vardec.o handle_mops.o \
     ToNewTypes.o SSATransform.o UndoSSATransform.o ssa.o \
     WLPartitionGeneration.o WLEnhancement.o ExplicitAccumulate.o \
     ToOldTypes.o wlanalysis.o lacinlining.o wldefaultpartition.o \
     while2do.o handle_condexpr.o codesimplification.o

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
