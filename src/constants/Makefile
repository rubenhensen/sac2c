#
#
# $Log$
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


#
# OS setup:
#
# legal values for OS currently are:
#    SOLARIS_SPARC
#    LINUX_X86
#    OSF_ALPHA
#
OS        := SOLARIS_SPARC


CC        := gcc
CCFLAGS   := -ansi -Wall -g -DSAC_FOR_$(OS)
CFLAGS    := -D__EXTENSIONS__ 

RM=rm -f
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

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak
	$(RM) -r .sb SunWS_cache

# DO NOT DELETE THIS LINE -- make depend depends on it.

constants_ari_ops.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
constants_ari_ops.o: /usr/include/sys/isa_defs.h ../../inc/dbug.h
constants_ari_ops.o: /usr/include/stdio.h /usr/include/sys/va_list.h
constants_ari_ops.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
constants_ari_ops.o: ../global/internal_lib.h ../global/types.h constants.h
constants_ari_ops.o: shape.h constants_internal.h ../global/globals.h zipcv.h
constants_basic.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
constants_basic.o: /usr/include/sys/isa_defs.h ../global/globals.h
constants_basic.o: /usr/include/stdio.h /usr/include/sys/va_list.h
constants_basic.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
constants_basic.o: ../global/types.h ../tree/free.h ../../inc/dbug.h
constants_basic.o: ../global/my_debug.h ../global/internal_lib.h shape.h
constants_basic.o: cv2scalar.h cv2cv.h cv2str.h ../tree/tree_compound.h
constants_basic.o: ../tree/tree_basic.h ../global/Error.h
constants_basic.o: /usr/include/string.h constants.h constants_internal.h
constants_struc_ops.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
constants_struc_ops.o: /usr/include/sys/isa_defs.h ../global/internal_lib.h
constants_struc_ops.o: ../global/types.h ../../inc/dbug.h
constants_struc_ops.o: /usr/include/stdio.h /usr/include/sys/va_list.h
constants_struc_ops.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
constants_struc_ops.o: constants.h shape.h constants_internal.h
cv2cv.o: ../../inc/dbug.h /usr/include/stdio.h
cv2cv.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
cv2cv.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
cv2cv.o: /usr/include/stdio_impl.h cv2cv.h ../tree/type_info.mac
cv2scalar.o: ../tree/tree_basic.h ../global/types.h ../../inc/dbug.h
cv2scalar.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
cv2scalar.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
cv2scalar.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h cv2scalar.h
cv2scalar.o: ../tree/type_info.mac
cv2str.o: /usr/include/strings.h /usr/include/sys/types.h
cv2str.o: /usr/include/sys/isa_defs.h /usr/include/sys/feature_tests.h
cv2str.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
cv2str.o: /usr/include/stdlib.h ../../inc/dbug.h /usr/include/stdio.h
cv2str.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
cv2str.o: /usr/include/stdio_impl.h cv2str.h
shape.o: /usr/include/stdarg.h /usr/include/varargs.h
shape.o: /usr/include/sys/va_list.h ../../inc/dbug.h /usr/include/stdio.h
shape.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
shape.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
shape.o: ../tree/tree_basic.h ../global/types.h ../tree/free.h
shape.o: /usr/include/stdlib.h ../global/my_debug.h ../global/globals.h
shape.o: ../global/internal_lib.h shape.h
zipcv.o: /usr/include/strings.h /usr/include/sys/types.h
zipcv.o: /usr/include/sys/isa_defs.h /usr/include/sys/feature_tests.h
zipcv.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
zipcv.o: /usr/include/stdlib.h ../../inc/dbug.h /usr/include/stdio.h
zipcv.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
zipcv.o: /usr/include/stdio_impl.h ../global/types.h zipcv.h
