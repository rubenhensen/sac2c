#
# $Log$
# Revision 1.18  1995/04/05 13:54:01  sbs
# make deps done
#
# Revision 1.17  1995/03/29  11:50:51  hw
# added -I../compile
# make deps done
#
# Revision 1.16  1995/03/28  12:08:16  hw
#  added internal_lib.o
# make deps done
#
# Revision 1.15  1995/03/10  10:48:11  hw
# -I../refcount.c inserted
# make deps done
#
# Revision 1.14  1995/03/08  17:04:59  asi
# added -I../global
#
# Revision 1.13  1995/01/18  17:28:00  asi
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
# 
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
     -I../optimize -I../modules -I../global -I../refcount -I../compile \
     -I/usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include

all: main.o Error.o traverse.o usage.o my_debug.o tree.o free.o internal_lib.o

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

internal_lib.o: internal_lib.c
	$(CC) -o internal_lib.o -c internal_lib.c  $(INCS)

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
free.o: traverse.h ../optimize/optimize.h
internal_lib.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdlib.h
internal_lib.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/sys/stdtypes.h
internal_lib.o: Error.h
internal_lib.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdio.h
internal_lib.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdarg.h
internal_lib.o: ../../inc/dbug.h my_debug.h internal_lib.h
main.o: ../../inc/main.h ../../inc/dbug.h
main.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdio.h
main.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdarg.h
main.o: tree.h type_info.mac node_info.mac prf_node_info.mac free.h Error.h
main.o: usage.h ../flatten/flatten.h ../print/print.h
main.o: ../typecheck/typecheck.h ../optimize/optimize.h
main.o: ../optimize/ConstantFolding.h ../optimize/DeadCodeRemoval.h
main.o: ../modules/filemgr.h ../modules/import.h ../refcount/refcount.h
main.o: ../scanparse/scnprs.h ../scanparse/y.tab.h ../compile/compile.h
main.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdlib.h
main.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/sys/stdtypes.h
main.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/string.h
my_debug.o: tree.h type_info.mac node_info.mac prf_node_info.mac
my_debug.o: ../print/convert.h
traverse.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdio.h
traverse.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdarg.h
traverse.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdlib.h
traverse.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/sys/stdtypes.h
traverse.o: tree.h type_info.mac node_info.mac prf_node_info.mac free.h
traverse.o: Error.h ../../inc/dbug.h my_debug.h ../flatten/flatten.h
traverse.o: ../print/print.h ../typecheck/typecheck.h ../optimize/optimize.h
traverse.o: ../optimize/ConstantFolding.h ../optimize/DeadCodeRemoval.h
traverse.o: ../optimize/WorkReduction.h ../modules/import.h
traverse.o: ../refcount/refcount.h ../compile/compile.h traverse.h
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
