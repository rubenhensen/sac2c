#
# $Log$
# Revision 1.9  1994/12/13 15:15:04  hw
# moved -I/usr/local/lib/gcc-include from Error.o to INCS
# `make deps` done
#
# Revision 1.8  1994/12/13  14:57:18  hw
# added -I/usr/local/lib/gcc-include/ to rule for Error.o
#
# Revision 1.7  1994/12/11  17:30:14  sbs
# -I, -L + environment vars inserted
#
# Revision 1.6  1994/12/09  10:43:42  sbs
# optimize inserted
#
# Revision 1.5  1994/12/05  13:00:58  hw
# called  'make deps'
# ,.
#
# Revision 1.4  1994/12/01  17:35:42  hw
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

INCS=-I../../inc -I../print -I../flatten -I../scanparse -I../typecheck \
     -I../optimize -I../modules -I/usr/local/lib/gcc-include

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

Error.o: /usr/local/lib/gcc-include/stdarg.h /usr/include/stdio.h Error.h
Error.o: ../../inc/dbug.h
main.o: ../../inc/main.h ../../inc/dbug.h /usr/include/stdio.h tree.h
main.o: node_info.mac prf_node_info.mac Error.h usage.h ../flatten/flatten.h
main.o: ../print/print.h ../typecheck/typecheck.h ../optimize/optimize.h
main.o: ../modules/filemgr.h ../scanparse/scnprs.h /usr/include/stdlib.h
main.o: /usr/include/sys/stdtypes.h /usr/include/string.h
my_debug.o: tree.h node_info.mac prf_node_info.mac ../print/convert.h
traverse.o: /usr/include/stdio.h /usr/include/stdlib.h
traverse.o: /usr/include/sys/stdtypes.h tree.h node_info.mac
traverse.o: prf_node_info.mac Error.h ../../inc/dbug.h my_debug.h
traverse.o: ../flatten/flatten.h ../print/print.h ../typecheck/typecheck.h
traverse.o: ../optimize/optimize.h traverse.h
usage.o: usage.h /usr/include/stdio.h ../../inc/dbug.h
