#
# $Log$
# Revision 1.4  1994/12/01 17:35:42  hw
# added -I../typecheck
#
# Revision 1.3  1994/11/22  13:48:29  sbs
# scnprs.h included in main.c
#
# Revision 1.2  1994/11/10  15:44:34  sbs
# RCS-header inserted
#
#

MAKE=make
CC=gcc -ansi -g 

INCS=-I../../inc -I../print -I../flatten -I../scanparse -I../typecheck

all: main.o Error.o traverse.o usage.o my_debug.o

main.o: main.c 
	$(CC) -c main.c $(INCS)

Error.o: Error.c 
	$(CC) -o Error.o -c Error.c $(INCS)

traverse.o: traverse.c
	$(CC) -o traverse.o -c traverse.c $(INCS)

usage.o: usage.c usage.h 
	$(CC) -o usage.o -c usage.c $(INCS)
	
my_debug.o: my_debug.c
	$(CC) -o my_debug.o -c my_debug.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak
# DO NOT DELETE THIS LINE -- make depend depends on it.

Error.o: Error.h /usr/include/stdio.h
main.o: ../../inc/main.h ../../inc/dbug.h /usr/include/stdio.h tree.h
main.o: node_info.mac prf_node_info.mac Error.h usage.h ../flatten/flatten.h
main.o: ../print/print.h ../typecheck/typecheck.h ../scanparse/scnprs.h
main.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h
main.o: /usr/include/string.h
my_debug.o: node_info.mac prf_node_info.mac
traverse.o: /usr/include/stdio.h /usr/include/stdlib.h
traverse.o: /usr/include/sys/stdtypes.h tree.h node_info.mac
traverse.o: prf_node_info.mac Error.h ../../inc/dbug.h my_debug.h
traverse.o: ../flatten/flatten.h ../print/print.h ../typecheck/typecheck.h
traverse.o: traverse.h
usage.o: usage.h /usr/include/stdio.h ../../inc/dbug.h
