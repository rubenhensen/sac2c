

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
     -I../typecheck -I../print -I../scanparse

all: print_interface.o map_cwrapper.o import_specialization.o

print_interface.o: print_interface.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o print_interface.o -c print_interface.c $(INCS)

map_cwrapper.o: map_cwrapper.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o map_cwrapper.o -c map_cwrapper.c $(INCS)

import_specialization.o: import_specialization.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o import_specialization.o \
        -c import_specialization.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) print_interface.o map_cwrapper.o *.bak
	$(RM) -r .sb SunWS_cache

# DO NOT DELETE THIS LINE -- make depend depends on it.

import_specialization.o: /usr/include/stdio.h
import_specialization.o: /usr/include/sys/feature_tests.h
import_specialization.o: /usr/include/sys/isa_defs.h
import_specialization.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
import_specialization.o: /usr/include/stdio_impl.h /usr/include/stdlib.h
import_specialization.o: /usr/include/string.h ../tree/tree.h
import_specialization.o: ../global/types.h ../tree/tree_basic.h
import_specialization.o: ../tree/tree_compound.h ../global/Error.h
import_specialization.o: ../global/globals.h ../global/internal_lib.h
import_specialization.o: ../global/my_debug.h ../../inc/dbug.h
import_specialization.o: ../tree/traverse.h ../print/convert.h
import_specialization.o: ../modules/filemgr.h ../tree/free.h
import_specialization.o: /usr/include/malloc.h /usr/include/sys/types.h
import_specialization.o: /usr/include/sys/machtypes.h
import_specialization.o: /usr/include/sys/int_types.h
import_specialization.o: /usr/include/sys/select.h /usr/include/sys/time.h
import_specialization.o: /usr/include/sys/time.h ../global/resource.h
import_specialization.o: ../scanparse/scnprs.h ../scanparse/y.tab.h
import_specialization.o: import_specialization.h
map_cwrapper.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
map_cwrapper.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
map_cwrapper.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
map_cwrapper.o: /usr/include/stdlib.h /usr/include/string.h ../tree/tree.h
map_cwrapper.o: ../global/types.h ../tree/tree_basic.h
map_cwrapper.o: ../tree/tree_compound.h ../global/Error.h ../global/globals.h
map_cwrapper.o: ../global/internal_lib.h map_cwrapper.h ../global/my_debug.h
map_cwrapper.o: ../../inc/dbug.h ../tree/traverse.h ../print/convert.h
map_cwrapper.o: ../modules/filemgr.h ../tree/free.h /usr/include/malloc.h
map_cwrapper.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
map_cwrapper.o: /usr/include/sys/int_types.h /usr/include/sys/select.h
map_cwrapper.o: /usr/include/sys/time.h /usr/include/sys/time.h
map_cwrapper.o: ../global/resource.h ../typecheck/shape.h
print_interface.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
print_interface.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
print_interface.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
print_interface.o: /usr/include/stdlib.h /usr/include/string.h ../tree/tree.h
print_interface.o: ../global/types.h ../tree/tree_basic.h
print_interface.o: ../tree/tree_compound.h ../global/Error.h
print_interface.o: ../global/globals.h ../global/internal_lib.h
print_interface.o: print_interface.h ../print/print.h ../global/my_debug.h
print_interface.o: ../../inc/dbug.h ../tree/traverse.h ../print/convert.h
print_interface.o: ../modules/filemgr.h ../tree/free.h /usr/include/malloc.h
print_interface.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
print_interface.o: /usr/include/sys/int_types.h /usr/include/sys/select.h
print_interface.o: /usr/include/sys/time.h /usr/include/sys/time.h
print_interface.o: ../global/resource.h ../compile/gen_startup_code.h
print_interface.o: ../tree/type_info.mac
