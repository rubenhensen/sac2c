#
#
# $Log$
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

INCS = -I../global -I../../inc -I../tree -I../print -I../optimize -I../constants


all: flatten.o lac2fun.o fun2lac.o while2do.o insert_vardec.o handle_mops.o


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
