#
# $Log$
# Revision 1.13  1995/01/18 17:28:00  asi
# make deps
#
# Revision 1.12  1994/12/20  15:44:22  sbs
# tree.c and free.c added.
#
# Revision 1.11  1994/12/20  10:26:03  hw
# changed  -I/usr/local/lib/gcc-include to
# -I/usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include
# make deps done
#
# Revision 1.10  1994/12/16  13:19:47  sbs
# make deps...
#
# Revision 1.9  1994/12/13  15:15:04  hw
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
     -I../optimize -I../modules \
     -I/usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include

all: main.o Error.o traverse.o usage.o my_debug.o tree.o free.o

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

tree.o: tree.c
	$(CC) -o tree.o -c tree.c $(INCS)

free.o: free.c
	$(CC) -o free.o -c free.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak
# DO NOT DELETE THIS LINE -- make depend depends on it.

Error.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdarg.h
Error.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdio.h
Error.o: Error.h ../../inc/dbug.h
free.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdlib.h
free.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/sys/stdtypes.h
free.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdio.h
free.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdarg.h
free.o: ../../inc/dbug.h tree.h type_info.mac node_info.mac prf_node_info.mac
free.o: traverse.h
main.o: ../../inc/main.h ../../inc/dbug.h
main.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdio.h
main.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdarg.h
main.o: tree.h type_info.mac node_info.mac prf_node_info.mac Error.h usage.h
main.o: ../flatten/flatten.h ../print/print.h ../typecheck/typecheck.h
main.o: ../optimize/optimize.h ../modules/filemgr.h ../modules/import.h
main.o: ../scanparse/scnprs.h ../scanparse/y.tab.h
main.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdlib.h
main.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/sys/stdtypes.h
main.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/string.h
my_debug.o: tree.h type_info.mac node_info.mac prf_node_info.mac
my_debug.o: ../print/convert.h
traverse.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdio.h
traverse.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdarg.h
traverse.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdlib.h
traverse.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/sys/stdtypes.h
traverse.o: tree.h type_info.mac node_info.mac prf_node_info.mac Error.h
traverse.o: ../../inc/dbug.h my_debug.h ../flatten/flatten.h ../print/print.h
traverse.o: ../typecheck/typecheck.h ../optimize/optimize.h
traverse.o: ../modules/import.h free.h traverse.h
tree.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdlib.h
tree.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/sys/stdtypes.h
tree.o: tree.h type_info.mac node_info.mac prf_node_info.mac ../../inc/dbug.h
tree.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdio.h
tree.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdarg.h
tree.o: my_debug.h ../scanparse/scnprs.h ../scanparse/y.tab.h
tree.o: ../optimize/optimize.h
usage.o: usage.h
usage.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdio.h
usage.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdarg.h
usage.o: ../../inc/dbug.h
