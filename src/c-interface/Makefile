

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
     -I../typecheck -I../print

all: print_interface.o map_cwrapper.o

print_interface.o: print_interface.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o print_interface.o -c print_interface.c $(INCS)

map_cwrapper.o: map_cwrapper.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o map_cwrapper.o -c map_cwrapper.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) print_interface.o map_cwrapper.o *.bak
	$(RM) -r .sb SunWS_cache

# DO NOT DELETE THIS LINE -- make depend depends on it.

map_cwrapper.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
map_cwrapper.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
map_cwrapper.o: /usr/include/string.h ../tree/tree.h ../global/types.h
map_cwrapper.o: ../tree/tree_basic.h ../tree/tree_compound.h
map_cwrapper.o: ../global/Error.h ../global/globals.h
map_cwrapper.o: ../global/internal_lib.h map_cwrapper.h ../global/my_debug.h
map_cwrapper.o: ../../inc/dbug.h ../tree/traverse.h ../print/convert.h
map_cwrapper.o: ../modules/filemgr.h ../tree/free.h /usr/include/malloc.h
map_cwrapper.o: ../global/resource.h ../typecheck/shape.h
print_interface.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
print_interface.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
print_interface.o: /usr/include/string.h ../tree/tree.h ../global/types.h
print_interface.o: ../tree/tree_basic.h ../tree/tree_compound.h
print_interface.o: ../global/Error.h ../global/globals.h
print_interface.o: ../global/internal_lib.h print_interface.h
print_interface.o: ../print/print.h ../global/my_debug.h ../../inc/dbug.h
print_interface.o: ../tree/traverse.h ../print/convert.h ../modules/filemgr.h
print_interface.o: ../tree/free.h /usr/include/malloc.h ../global/resource.h
print_interface.o: ../tree/type_info.mac
