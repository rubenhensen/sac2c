#
# 
# $Log$
# Revision 3.24  2004/09/02 16:01:35  skt
# support for consolidate_cells added
#
# Revision 3.23  2004/08/31 16:56:53  skt
# replicate_functions.o added
#
# Revision 3.22  2004/08/24 16:50:24  skt
# creation of specialized functions within parallel withloops enabled
#
# Revision 3.21  2004/08/17 09:09:51  skt
# cell_growth.o added
#
# Revision 3.20  2004/08/13 18:02:34  skt
# several mtmode 3 traversals removed
#
# Revision 3.19  2004/07/29 00:40:54  skt
# added support for creation of dataflowgraph (mtmode 3)
#
# Revision 3.18  2004/07/26 16:13:35  skt
# support for create_cells added
#
# Revision 3.17  2004/07/06 12:40:54  skt
# propagate_executionmode added
#
# Revision 3.16  2004/06/08 14:21:07  skt
# tag_executionmode.o added
#
# Revision 3.15  2004/04/27 10:01:17  skt
# target assignments_rearrange added
#
# Revision 3.14  2003/06/11 21:54:11  ktr
# included ../constants
#
# Revision 3.13  2002/06/06 19:56:03  dkr
# target clean modified
#
# Revision 3.12  2001/11/15 15:36:48  sacbase
# CLOCK_SKEW_ELIMINATION and PROJECT_ROOT added.
#
# Revision 3.11  2001/11/14 12:59:48  sbs
# deps line replaced by inclusion of Makefile.Deps
# ( .xxx.d mechanism)
# global setup included via Makefile.Config
#
# Revision 3.10  2001/11/13 20:32:50  dkr
# make deps done
#
# Revision 3.9  2001/07/18 13:08:21  cg
# make deps done
#
# Revision 3.8  2001/05/08 13:29:44  dkr
# make deps done
#
# Revision 3.7  2001/01/17 10:13:11  dkr
# make deps done
#
# Revision 3.6  2000/12/15 18:18:08  dkr
# make deps done
#
# Revision 3.5  2000/12/12 15:09:27  dkr
# make deps done
#
# Revision 3.4  2000/12/08 10:05:27  dkr
# include path extended
#
# Revision 3.3  2000/12/07 14:19:55  dkr
# include path modified
#
# ... [eliminated] ...
#
# Revision 1.1  2000/01/21 13:11:16  jhs
# Initial revision
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


INCS = -I../global -I../tree -I../../inc -I../optimize -I../typecheck -I../flatten \
       -I../print -I../compile -I../refcount -I../constants


all: multithread.o multithread_lib.o assignments_rearrange.o \
    tag_executionmode.o propagate_executionmode.o create_cells.o \
    create_dataflowgraph.o cell_growth.o create_withinwith.o \
    replicate_functions.o consolidate_cells.o

 


%.o: %.c 
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<
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
