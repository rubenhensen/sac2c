#
#
# $Log$
# Revision 1.12  2002/06/06 19:56:03  dkr
# target clean modified
#
# Revision 1.11  2001/11/15 15:36:48  sacbase
# CLOCK_SKEW_ELIMINATION and PROJECT_ROOT added.
#
# Revision 1.10  2001/11/14 12:59:48  sbs
# deps line replaced by inclusion of Makefile.Deps
# ( .xxx.d mechanism)
# global setup included via Makefile.Config
#
# Revision 1.9  2001/11/13 20:32:52  dkr
# make deps done
#
# Revision 1.8  2001/07/18 13:08:22  cg
# make deps done
#
# Revision 1.7  2001/05/08 13:29:47  dkr
# make deps done
#
# Revision 1.6  2001/05/02 07:58:49  nmw
# basecv.o added
#
# Revision 1.5  2001/04/26 21:09:59  dkr
# make deps done
#
# Revision 1.4  2001/03/22 20:20:22  dkr
# make deps done
#
# Revision 1.3  2001/03/05 16:57:04  sbs
# zipcv.o added
#
# Revision 1.2  2001/03/02 14:46:05  sbs
# make deps done
#
# Revision 1.1  2001/03/02 14:32:50  sbs
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

INCS = -I../../inc -I../tree -I../global

all: shape.o \
     constants_basic.o constants_struc_ops.o constants_ari_ops.o \
     cv2scalar.o cv2cv.o cv2str.o zipcv.o basecv.o


#
# the *.[ch] objects are to fool makedepend which does not
# recognize the dependency between *.[ch] and the .mac files!
#

cv2cv.c: ../tree/type_info.mac
	touch $@
	$(CLOCK_SKEW_ELIMINATION) $@

cv2scalar.c: ../tree/type_info.mac
	touch $@
	$(CLOCK_SKEW_ELIMINATION) $@

cv2str.c: ../tree/type_info.mac
	touch $@
	$(CLOCK_SKEW_ELIMINATION) $@

zipcv.c: ../tree/type_info.mac
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
