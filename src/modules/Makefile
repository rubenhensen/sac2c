#
#
# $Log$
# Revision 3.9  2001/11/14 12:59:48  sbs
# deps line replaced by inclusion of Makefile.Deps
# ( .xxx.d mechanism)
# global setup included via Makefile.Config
#
# Revision 3.8  2001/11/13 20:32:44  dkr
# make deps done
#
# Revision 3.7  2001/07/18 13:08:17  cg
# make deps done
#
# Revision 3.6  2001/03/22 20:20:00  dkr
# make deps done
#
# Revision 3.5  2001/03/21 11:19:56  dkr
# make deps done
#
# Revision 3.4  2001/02/02 10:39:02  dkr
# make deps done
#
# Revision 3.3  2001/01/17 10:13:04  dkr
# make deps done
#
# Revision 3.2  2000/12/06 20:09:03  dkr
# make deps done
#
# Revision 3.1  2000/11/20 18:00:46  sacbase
# new release made
#
# ... [eliminated] ...
#
# Revision 1.1  1994/12/11  17:33:27  sbs
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


INCS=-I../global -I../tree -I../../inc -I../scanparse -I../print -I../typecheck \
     -I../compile

all: filemgr.o import.o readsib.o writesib.o implicittypes.o analysis.o \
     checkdec.o cccall.o

%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o $@ -c $< $(INCS)

clean:
	$(RM) *.o *.bak *.sac *.sib *.dec .*.d
	$(RM) -r .sb SunWS_cache



#######################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include ../../Makefile.Deps



