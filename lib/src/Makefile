#
# $Log$
# Revision 3.9  2001/11/15 16:13:12  sacbase
# CLOCK_SKEW_ELIMINATION and dpendency machanism eliminated
# due to circular reference with cse!!
#
# Revision 3.8  2001/11/15 15:52:39  sacbase
# PROJECT_ROOT and CLOCK_SKEW_ELIMINATION added.
#
# Revision 3.7  2001/11/14 12:59:48  sbs
# deps line replaced by inclusion of Makefile.Deps
# ( .xxx.d mechanism)
# global setup included via Makefile.Config
#
# Revision 3.6  2001/11/13 20:32:46  dkr
# make deps done
#
# Revision 3.5  2001/07/18 13:08:19  cg
# make deps done
#
# Revision 3.4  2001/03/02 14:46:00  sbs
# make deps done
#
# Revision 3.3  2001/02/27 14:03:37  nmw
# platform dependen enviroment variables added
#
# Revision 3.2  2000/12/06 20:09:07  dkr
# make deps done
#
# Revision 3.1  2000/11/20 18:02:04  sacbase
# new release made
#
# .... [eliminated] ....
#
# Revision 1.1  1997/11/25 13:44:09  dkr
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

INCS= -I../../inc

all: ../dbug.o ../main_args.o 

../dbug.o: dbug.c
	$(CC) $(CFLAGS) $(MYFLAGS) -o ../dbug.o -c dbug.c $(INCS)

../main_args.o: main_args.c
	$(CC) $(CFLAGS) $(MYFLAGS) -o ../main_args.o -c main_args.c $(INCS)

clean:
	$(RM) ../dbug.o ../main_args.o *.bak *~ .*.d





