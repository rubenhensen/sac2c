#
# $Log$
# Revision 1.5  1994/12/05 11:16:46  hw
# added rule for conver.o
#
# Revision 1.4  1994/11/10  15:35:50  sbs
# RCS-header inserted
#
#

CC=gcc -ansi -g

INCS=-I../global -I../../inc

all: print.o convert.o

print.o: print.c
	$(CC) -o print.o -c print.c $(INCS)

convert.o: convert.c
	$(CC) -o convert.o -c convert.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) print.o *.bak


# DO NOT DELETE THIS LINE -- make depend depends on it.

convert.o: /usr/include/string.h /usr/include/sys/stdtypes.h
convert.o: /usr/include/stdlib.h ../global/tree.h ../global/node_info.mac
convert.o: ../global/prf_node_info.mac ../../inc/dbug.h /usr/include/stdio.h
convert.o: convert.h
print.o: /usr/include/stdio.h /usr/include/stdlib.h
print.o: /usr/include/sys/stdtypes.h /usr/include/string.h ../global/tree.h
print.o: ../global/node_info.mac ../global/prf_node_info.mac
print.o: ../global/my_debug.h ../../inc/dbug.h ../global/traverse.h print.h
print.o: ../global/Error.h convert.h
