#
# $Log$
# Revision 1.3  1994/12/09 10:44:24  sbs
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
INCS=-I../global -I../../inc -I../print

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
typecheck.o: ../global/node_info.mac ../global/prf_node_info.mac
typecheck.o: ../global/my_debug.h ../../inc/dbug.h /usr/include/stdio.h
typecheck.o: ../global/traverse.h typecheck.h ../global/Error.h
typecheck.o: ../print/convert.h
