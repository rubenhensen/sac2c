#
#
# $Log$
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
# ;.
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

INCS=-I../../inc -I../tree -I../global

all: shape.o \
     constants_basic.o constants_struc_ops.o constants_ari_ops.o \
     cv2scalar.o cv2cv.o cv2str.o zipcv.o basecv.o


#
# the *.[ch] objects are to fool makedepend which does not
# recognize the dependency between *.[ch] and the .mac files!
#

cv2cv.c: ../tree/type_info.mac
	touch $@

cv2scalar.c: ../tree/type_info.mac
	touch $@

cv2str.c: ../tree/type_info.mac
	touch $@

zipcv.c: ../tree/type_info.mac
	touch $@


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


