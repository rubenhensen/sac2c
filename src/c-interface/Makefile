#
#
# $Log$
# Revision 3.12  2002/06/06 19:56:03  dkr
# target clean modified
#
# Revision 3.11  2001/11/15 15:36:48  sacbase
# CLOCK_SKEW_ELIMINATION and PROJECT_ROOT added.
#
# Revision 3.10  2001/11/14 12:59:48  sbs
# deps line replaced by inclusion of Makefile.Deps
# ( .xxx.d mechanism)
# global setup included via Makefile.Config
#
# Revision 3.9  2001/11/13 20:32:51  dkr
# make deps done
#
# Revision 3.8  2001/03/22 20:20:21  dkr
# make deps done
#
# Revision 3.7  2001/03/02 16:14:07  dkr
# make deps done
#
# Revision 3.6  2001/03/02 14:55:22  sbs
# constants included rather than typecheck now.
# (shape has moved there)
#
# Revision 3.5  2001/03/02 14:46:04  sbs
# make deps done
#
# Revision 3.4  2001/03/02 10:35:53  dkr
# /home/dkr/sac/BASE/sac2c/src/c-interface/RCS/Makefile,v used in touch-rules
#
# Revision 3.3  2001/01/17 10:13:13  dkr
# make deps done
#
# Revision 3.2  2000/12/06 20:09:13  dkr
# make deps done
#
# Revision 3.1  2000/11/20 18:03:38  sacbase
# new release made
#
# Revision 1.12  2000/11/02 14:54:48  dkr
# deps made
#
# Revision 1.11  2000/11/02 14:54:34  dkr
# RCS header added
# explicit .mac dependencies added
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

INCS = -I../global -I../tree -I../../inc -I../compile -I../modules \
       -I../constants -I../print -I../scanparse


all: print_interface.o map_cwrapper.o import_specialization.o \
     print_interface_header.o print_interface_wrapper.o


#
# the *.[ch] objects are to fool makedepend which does not
# recognize the dependency between *.[ch] and the .mac files!
#
print_interface_wrapper.c: ../tree/type_info.mac
	touch $@
	$(CLOCK_SKEW_ELIMINATION) $@


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
