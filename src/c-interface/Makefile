#
#
# $Log$
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


INCS=-I../global -I../tree -I../../inc -I../compile -I../modules \
     -I../constants -I../print -I../scanparse

all: print_interface.o map_cwrapper.o import_specialization.o \
     print_interface_header.o print_interface_wrapper.o


#
# the *.[ch] objects are to fool makedepend which does not
# recognize the dependency between *.[ch] and the .mac files!
#
print_interface_wrapper.c: ../tree/type_info.mac
	touch $@


%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o $@ -c $< $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak
	$(RM) -r .sb SunWS_cache

# DO NOT DELETE THIS LINE -- make depend depends on it.

import_specialization.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
import_specialization.o: /usr/include/sys/feature_tests.h
import_specialization.o: /usr/include/sys/isa_defs.h
import_specialization.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
import_specialization.o: /usr/include/stdio_impl.h /usr/include/stdlib.h
import_specialization.o: /usr/include/iso/stdlib_iso.h /usr/include/string.h
import_specialization.o: /usr/include/iso/string_iso.h ../global/types.h
import_specialization.o: ../tree/tree_basic.h ../tree/tree_compound.h
import_specialization.o: ../global/Error.h ../global/globals.h
import_specialization.o: ../global/internal_lib.h ../global/my_debug.h
import_specialization.o: ../../inc/dbug.h ../tree/traverse.h
import_specialization.o: ../print/convert.h ../modules/filemgr.h
import_specialization.o: ../tree/free.h ../tree/DupTree.h
import_specialization.o: ../tree/LookUpTable.h ../global/resource.h
import_specialization.o: ../scanparse/scnprs.h ../scanparse/y.tab.h
import_specialization.o: import_specialization.h
map_cwrapper.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
map_cwrapper.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
map_cwrapper.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
map_cwrapper.o: /usr/include/stdio_impl.h /usr/include/stdlib.h
map_cwrapper.o: /usr/include/iso/stdlib_iso.h /usr/include/string.h
map_cwrapper.o: /usr/include/iso/string_iso.h ../global/types.h
map_cwrapper.o: ../tree/tree_basic.h ../tree/tree_compound.h
map_cwrapper.o: ../global/Error.h ../global/globals.h
map_cwrapper.o: ../global/internal_lib.h map_cwrapper.h ../global/my_debug.h
map_cwrapper.o: ../../inc/dbug.h ../tree/traverse.h ../print/convert.h
map_cwrapper.o: ../modules/filemgr.h ../tree/free.h ../global/resource.h
map_cwrapper.o: ../constants/shape.h
print_interface.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
print_interface.o: /usr/include/sys/feature_tests.h
print_interface.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
print_interface.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
print_interface.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
print_interface.o: /usr/include/string.h /usr/include/iso/string_iso.h
print_interface.o: ../global/types.h ../tree/tree_basic.h
print_interface.o: ../tree/tree_compound.h ../global/Error.h
print_interface.o: ../global/globals.h ../global/internal_lib.h
print_interface.o: print_interface.h ../print/print.h ../global/my_debug.h
print_interface.o: ../../inc/dbug.h ../tree/traverse.h ../print/convert.h
print_interface.o: ../modules/filemgr.h ../tree/free.h ../global/resource.h
print_interface.o: ../compile/gen_startup_code.h
print_interface_header.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
print_interface_header.o: /usr/include/sys/feature_tests.h
print_interface_header.o: /usr/include/sys/isa_defs.h
print_interface_header.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
print_interface_header.o: /usr/include/stdio_impl.h /usr/include/stdlib.h
print_interface_header.o: /usr/include/iso/stdlib_iso.h /usr/include/string.h
print_interface_header.o: /usr/include/iso/string_iso.h ../global/types.h
print_interface_header.o: ../tree/tree_basic.h ../tree/tree_compound.h
print_interface_header.o: ../global/Error.h ../global/globals.h
print_interface_header.o: ../global/internal_lib.h print_interface_header.h
print_interface_header.o: ../print/print.h ../global/my_debug.h
print_interface_header.o: ../../inc/dbug.h ../tree/traverse.h
print_interface_header.o: ../print/convert.h ../modules/filemgr.h
print_interface_header.o: ../tree/free.h ../global/resource.h
print_interface_header.o: ../compile/gen_startup_code.h
print_interface_wrapper.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
print_interface_wrapper.o: /usr/include/sys/feature_tests.h
print_interface_wrapper.o: /usr/include/sys/isa_defs.h
print_interface_wrapper.o: /usr/include/sys/va_list.h
print_interface_wrapper.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
print_interface_wrapper.o: /usr/include/stdlib.h
print_interface_wrapper.o: /usr/include/iso/stdlib_iso.h
print_interface_wrapper.o: /usr/include/string.h
print_interface_wrapper.o: /usr/include/iso/string_iso.h ../global/types.h
print_interface_wrapper.o: ../tree/tree_basic.h ../tree/tree_compound.h
print_interface_wrapper.o: ../global/Error.h ../global/globals.h
print_interface_wrapper.o: ../global/internal_lib.h print_interface_header.h
print_interface_wrapper.o: print_interface_wrapper.h ../print/print.h
print_interface_wrapper.o: ../global/my_debug.h ../../inc/dbug.h
print_interface_wrapper.o: ../tree/traverse.h ../print/convert.h
print_interface_wrapper.o: ../modules/filemgr.h ../tree/free.h
print_interface_wrapper.o: ../global/resource.h ../compile/gen_startup_code.h
print_interface_wrapper.o: ../tree/type_info.mac
