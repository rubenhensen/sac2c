#
# $Log$
# Revision 1.35  1996/01/04 18:11:05  cg
# make deps done
#
# Revision 1.34  1995/12/15  10:41:06  cg
# make deps done
#
# Revision 1.33  1995/11/01  08:01:27  cg
# make deps done
#
# Revision 1.32  1995/10/22  17:28:57  cg
# make deps done
#
# Revision 1.31  1995/10/20  09:22:29  cg
# make deps done
#
# Revision 1.30  1995/10/16  12:00:32  cg
# path to new directory objects added
#
# Revision 1.29  1995/10/08  11:29:45  sbs
# make deps done
#
# Revision 1.28  1995/09/27  13:04:32  cg
# included tree_basic.c and tree_compound.c
#
# Revision 1.27  1995/07/24  11:56:01  asi
# make deps done
#
# Revision 1.26  1995/07/07  15:47:43  asi
# make deps done
#
# Revision 1.25  1995/06/02  11:58:17  asi
# make deps done
#
# Revision 1.24  1995/06/02  10:08:21  sbs
# make deps done
#
# Revision 1.23  1995/06/02  09:27:06  sbs
# psi-opt instead of psi_opt !!!
#
# Revision 1.22  1995/06/02  09:25:48  sbs
# psi_opt inserted in LIBS
#
# Revision 1.21  1995/05/26  14:28:03  asi
# make deps done
#
# Revision 1.20  1995/05/15  13:38:42  asi
# make deps done
#
# Revision 1.19  1995/04/05  15:54:51  asi
# make deps done
#
# Revision 1.18  1995/04/05  13:54:01  sbs
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
     -I../psi-opt -I../objects \
     -I/usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include

all: main.o Error.o traverse.o usage.o my_debug.o tree.o free.o  \
     internal_lib.o tree_basic.o tree_compound.o

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

tree_basic.o: tree_basic.c
	$(CC) -o tree_basic.o -c tree_basic.c $(INCS)

tree_compound.o: tree_compound.c
	$(CC) -o tree_compound.o -c tree_compound.c $(INCS)

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
Error.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/string.h
Error.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/sys/stdtypes.h
Error.o: Error.h
Error.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdlib.h
Error.o: types.h globals.h tree_basic.h ../../inc/dbug.h internal_lib.h
Error.o: ../modules/filemgr.h
a.out.o: ../compile/icm2c.h
a.out.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/malloc.h
a.out.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdio.h
a.out.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdarg.h
free.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdlib.h
free.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/sys/stdtypes.h
free.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdio.h
free.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdarg.h
free.o: ../../inc/dbug.h my_debug.h types.h tree_basic.h tree_compound.h
free.o: Error.h
free.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/string.h
free.o: globals.h traverse.h free.h
free.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/malloc.h
internal_lib.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdlib.h
internal_lib.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/sys/stdtypes.h
internal_lib.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/math.h
internal_lib.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/floatingpoint.h
internal_lib.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/sys/ieeefp.h
internal_lib.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/string.h
internal_lib.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdarg.h
internal_lib.o: Error.h
internal_lib.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdio.h
internal_lib.o: types.h globals.h ../../inc/dbug.h my_debug.h internal_lib.h
main.o: ../../inc/main.h ../../inc/dbug.h
main.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdio.h
main.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdarg.h
main.o: tree.h types.h tree_basic.h tree_compound.h Error.h
main.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/string.h
main.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/sys/stdtypes.h
main.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdlib.h
main.o: globals.h internal_lib.h free.h my_debug.h
main.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/malloc.h
main.o: usage.h ../flatten/flatten.h ../print/print.h
main.o: ../typecheck/typecheck.h ../optimize/optimize.h ../modules/filemgr.h
main.o: ../modules/import.h ../refcount/refcount.h ../scanparse/scnprs.h
main.o: ../scanparse/y.tab.h ../compile/trace.h ../compile/compile.h
main.o: ../psi-opt/psi-opt.h ../modules/writesib.h ../modules/readsib.h
main.o: ../modules/implicittypes.h ../objects/objinit.h ../modules/analysis.h
main.o: ../modules/checkdec.h ../objects/objects.h ../objects/uniquecheck.h
main.o: ../objects/rmvoidfun.h ../compile/precompile.h ../modules/cccall.h
my_debug.o: tree.h types.h tree_basic.h tree_compound.h Error.h
my_debug.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdio.h
my_debug.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdarg.h
my_debug.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/string.h
my_debug.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/sys/stdtypes.h
my_debug.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdlib.h
my_debug.o: globals.h internal_lib.h ../print/convert.h node_info.mac
my_debug.o: prf_node_info.mac type_info.mac
traverse.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdio.h
traverse.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdarg.h
traverse.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdlib.h
traverse.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/sys/stdtypes.h
traverse.o: tree.h types.h tree_basic.h tree_compound.h Error.h
traverse.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/string.h
traverse.o: globals.h internal_lib.h ../../inc/dbug.h my_debug.h
traverse.o: ../flatten/flatten.h ../print/print.h ../typecheck/typecheck.h
traverse.o: ../optimize/optimize.h free.h
traverse.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/malloc.h
traverse.o: ../optimize/ConstantFolding.h ../optimize/DeadCodeRemoval.h
traverse.o: ../optimize/WorkReduction.h ../optimize/LoopInvariantRemoval.h
traverse.o: ../modules/import.h ../refcount/refcount.h ../compile/compile.h
traverse.o: ../optimize/DupTree.h ../optimize/Inline.h ../optimize/Unroll.h
traverse.o: ../optimize/Unswitch.h ../psi-opt/ArrayElimination.h
traverse.o: ../psi-opt/index.h ../modules/writesib.h ../modules/readsib.h
traverse.o: ../modules/implicittypes.h ../objects/objinit.h
traverse.o: ../modules/analysis.h ../modules/checkdec.h ../objects/objects.h
traverse.o: ../objects/uniquecheck.h ../objects/rmvoidfun.h
traverse.o: ../compile/precompile.h ../modules/cccall.h traverse.h
traverse.o: node_info.mac
tree.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdlib.h
tree.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/sys/stdtypes.h
tree.o: tree.h types.h tree_basic.h tree_compound.h Error.h
tree.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdio.h
tree.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdarg.h
tree.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/string.h
tree.o: globals.h internal_lib.h ../../inc/dbug.h my_debug.h
tree.o: ../scanparse/scnprs.h ../scanparse/y.tab.h ../optimize/optimize.h
tree.o: free.h
tree.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/malloc.h
tree_basic.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/malloc.h
tree_basic.o: types.h tree_basic.h Error.h
tree_basic.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdio.h
tree_basic.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdarg.h
tree_basic.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/string.h
tree_basic.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/sys/stdtypes.h
tree_basic.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdlib.h
tree_basic.o: globals.h ../../inc/dbug.h my_debug.h ../scanparse/scnprs.h
tree_basic.o: ../scanparse/y.tab.h free.h internal_lib.h prf_node_info.mac
tree_compound.o: types.h tree_basic.h tree_compound.h Error.h
tree_compound.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdio.h
tree_compound.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdarg.h
tree_compound.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/string.h
tree_compound.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/sys/stdtypes.h
tree_compound.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdlib.h
tree_compound.o: globals.h ../../inc/dbug.h my_debug.h
tree_compound.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/malloc.h
usage.o: usage.h
usage.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdio.h
usage.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdarg.h
usage.o: globals.h ../../inc/dbug.h tree.h types.h tree_basic.h
usage.o: tree_compound.h Error.h
usage.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/string.h
usage.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/sys/stdtypes.h
usage.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/stdlib.h
usage.o: internal_lib.h ../optimize/optimize.h free.h my_debug.h
usage.o: /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/include/malloc.h
