#
#
# $Log$
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
# Revision 3.9  2001/03/02 16:13:48  dkr
# make deps done
#
# Revision 3.8  2001/03/02 14:45:55  sbs
# make deps done
#
# Revision 3.7  2001/01/17 10:13:00  dkr
# make deps done
#
# Revision 3.6  2000/12/15 18:17:54  dkr
# make deps done
#
# Revision 3.5  2000/12/15 18:08:20  dkr
# adjust_ids.* cleanup_decls.* infer_dfms.* moved to tree
#
# Revision 3.4  2000/12/12 15:09:02  dkr
# make deps done
#
# Revision 3.3  2000/12/06 20:08:59  dkr
# make deps done
#
# Revision 3.2  2000/12/06 19:50:39  dkr
# infer_dfms.o added
#
# Revision 3.1  2000/11/20 17:59:15  sacbase
# new release made
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

INCS = -I../global -I../../inc -I../tree -I../print -I../optimize

all: flatten.o lac2fun.o fun2lac.o while2do.o insert_vardec.o

%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o $@ -c $< $(INCS)
	$(CLOCK_SKEW_ELIMINATION) $@

clean:
	$(RM) *.o *.bak *~ .*.d
	$(RM) -r .sb SunWS_cache

#######################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include ../../Makefile.Deps



