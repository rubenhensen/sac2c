#
# $Log$
# Revision 1.4  1995/01/04 16:04:37  hw
# added -I../modules to INCS
# make deps done
#
# Revision 1.3  1994/12/09  10:44:24  sbs
# make deps done
#
# Revision 1.2  1994/12/06  16:31:12  hw
# added dependencies
#
# Revision 1.1  1994/12/02  10:13:42  hw
# Initial revision
#
# 
#

CC=gcc -ansi -g
RM=rm -f
INCS=-I../global -I../../inc -I../print -I../modules

all: typecheck.o

typecheck.o: typecheck.c
	$(CC) -o typecheck.o -c typecheck.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) typecheck.o *.bak
# DO NOT DELETE THIS LINE -- make depend depends on it.

typecheck.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h
typecheck.o: /usr/include/stddef.h /usr/include/string.h ../global/tree.h
typecheck.o: ../global/type_info.mac ../global/node_info.mac
typecheck.o: ../global/prf_node_info.mac ../global/my_debug.h
typecheck.o: ../../inc/dbug.h /usr/include/stdio.h ../global/traverse.h
typecheck.o: typecheck.h ../global/Error.h ../print/convert.h
typecheck.o: ../modules/import.h
