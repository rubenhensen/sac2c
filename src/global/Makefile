#
# $Log$
# Revision 1.53  1998/02/27 09:04:31  cg
# added compilation of resource.h
#
# Revision 1.52  1998/02/25 10:08:23  srs
# make deps done
#
# Revision 1.51  1998/02/25 09:06:27  cg
# added file globals.c for compilation
#
# Revision 1.50  1998/02/06 18:09:47  dkr
# make deps done
#
# Revision 1.49  1998/02/05 17:08:36  dkr
# make deps done
#
# Revision 1.48  1997/11/20 15:02:03  dkr
# make deps done
#
# Revision 1.47  1997/11/19 19:41:19  dkr
# added include-path ../Old2NewWith
#
# Revision 1.46  1997/11/12 19:17:59  dkr
# build main.o now with -g!!
#
# Revision 1.45  1997/11/12 15:59:49  sbs
# CC+CCFLAGS handling changed.
# Now, it suffices to preset CC=cc if the SUN-compiler isdesired
# Otherwise, gcc is set with -Wall and -ansi
#
# Revision 1.44  1997/11/11 16:49:22  sbs
# make deps done
#
# Revision 1.43  1997/10/29 14:40:44  srs
# make deps done
#
# Revision 1.42  1997/10/28 18:28:00  srs
# removed unused dir from INCS
#
# Revision 1.41  1997/09/23 13:23:50  dkr
# forced make to recognize the dependency between types.h and
# type_info.mac, node_info.mac, prf_node_info.mac
#
# Revision 1.40  1997/09/23 12:59:31  dkr
# make deps done
#
# Revision 1.39  1997/08/05 11:32:23  sbs
# make deps done
#
# Revision 1.38  1997/05/28 12:48:26  sbs
# make deps done
#
# Revision 1.37  1997/04/25  12:04:05  sbs
# make deps done
#
# Revision 1.36  1996/01/18  13:05:38  asi
# make deps done
#
# Revision 1.35  1996/01/04  18:11:05  cg
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

CC        := gcc
CCFLAGS   := -ansi -Wall -g
CFLAGS    := -D__EXTENSIONS__


INCS=-I../../inc -I../print -I../flatten -I../scanparse -I../typecheck \
     -I../optimize -I../modules -I../global -I../refcount -I../compile \
     -I../psi-opt -I../objects -I../Old2NewWith

all: main.o Error.o traverse.o usage.o my_debug.o tree.o free.o  \
     internal_lib.o tree_basic.o tree_compound.o globals.o \
     resource.o types.h

#
# the types.h object is to fool makedepend which does not
# recognize the dependency between types.h and the .mac files!
#

types.h: type_info.mac node_info.mac prf_node_info.mac
	touch types.h

main.o: main.c 
	$(CC) $(CCFLAGS) $(CFLAGS) -o main.o -c main.c $(INCS)

Error.o: Error.c 
	$(CC) $(CCFLAGS) $(CFLAGS) -o Error.o -c Error.c $(INCS) 

traverse.o: traverse.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o traverse.o -c traverse.c $(INCS)

usage.o: usage.c usage.h 
	$(CC) $(CCFLAGS) $(CFLAGS) -o usage.o -c usage.c $(INCS)

my_debug.o: my_debug.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o my_debug.o -c my_debug.c $(INCS)

tree.o: tree.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o tree.o -c tree.c $(INCS)

tree_basic.o: tree_basic.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o tree_basic.o -c tree_basic.c $(INCS)

tree_compound.o: tree_compound.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o tree_compound.o -c tree_compound.c $(INCS)

free.o: free.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o free.o -c free.c $(INCS)

internal_lib.o: internal_lib.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o internal_lib.o -c internal_lib.c  $(INCS)

globals.o: globals.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o globals.o -c globals.c $(INCS)

resource.o: resource.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o resource.o -c resource.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak
# DO NOT DELETE THIS LINE -- make depend depends on it.

Error.o: /usr/include/stdarg.h /usr/include/varargs.h
Error.o: /usr/include/sys/va_list.h /usr/include/stdio.h
Error.o: /usr/include/sys/feature_tests.h /usr/include/string.h Error.h
Error.o: /usr/include/stdlib.h types.h globals.h tree_basic.h
Error.o: ../../inc/dbug.h internal_lib.h ../modules/filemgr.h
free.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
free.o: /usr/include/stdio.h /usr/include/sys/va_list.h ../../inc/dbug.h
free.o: my_debug.h types.h tree_basic.h tree_compound.h Error.h
free.o: /usr/include/string.h globals.h traverse.h free.h
free.o: /usr/include/malloc.h internal_lib.h
globals.o: ../modules/filemgr.h /usr/include/stdio.h
globals.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
globals.o: types.h globals.h ../scanparse/scnprs.h ../scanparse/y.tab.h
globals.o: Error.h /usr/include/string.h /usr/include/stdlib.h
internal_lib.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
internal_lib.o: /usr/include/math.h /usr/include/string.h
internal_lib.o: /usr/include/stdarg.h /usr/include/varargs.h
internal_lib.o: /usr/include/sys/va_list.h Error.h /usr/include/stdio.h
internal_lib.o: types.h globals.h free.h ../../inc/dbug.h my_debug.h
internal_lib.o: /usr/include/malloc.h internal_lib.h traverse.h
main.o: ../../inc/main.h ../../inc/dbug.h /usr/include/stdio.h
main.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h tree.h
main.o: types.h tree_basic.h tree_compound.h Error.h /usr/include/string.h
main.o: /usr/include/stdlib.h globals.h internal_lib.h free.h my_debug.h
main.o: /usr/include/malloc.h usage.h ../flatten/flatten.h ../print/print.h
main.o: ../typecheck/typecheck.h ../optimize/optimize.h ../modules/filemgr.h
main.o: ../modules/import.h ../refcount/refcount.h ../scanparse/scnprs.h
main.o: ../scanparse/y.tab.h ../compile/trace.h ../compile/profile.h
main.o: ../compile/compile.h ../psi-opt/psi-opt.h ../modules/writesib.h
main.o: ../modules/readsib.h ../modules/implicittypes.h ../objects/objinit.h
main.o: ../modules/analysis.h ../modules/checkdec.h ../objects/objects.h
main.o: ../objects/uniquecheck.h ../objects/rmvoidfun.h
main.o: ../compile/precompile.h ../modules/cccall.h ../compile/Old2NewWith.h
my_debug.o: tree.h types.h tree_basic.h tree_compound.h Error.h
my_debug.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
my_debug.o: /usr/include/sys/va_list.h /usr/include/string.h
my_debug.o: /usr/include/stdlib.h globals.h internal_lib.h ../print/convert.h
my_debug.o: node_info.mac prf_node_info.mac type_info.mac
traverse.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
traverse.o: /usr/include/sys/va_list.h /usr/include/stdlib.h tree.h types.h
traverse.o: tree_basic.h tree_compound.h Error.h /usr/include/string.h
traverse.o: globals.h internal_lib.h ../../inc/dbug.h my_debug.h
traverse.o: ../flatten/flatten.h ../print/print.h ../typecheck/typecheck.h
traverse.o: ../optimize/optimize.h free.h /usr/include/malloc.h
traverse.o: ../optimize/ConstantFolding.h ../optimize/DeadCodeRemoval.h
traverse.o: ../optimize/LoopInvariantRemoval.h ../optimize/CSE.h
traverse.o: ../modules/import.h ../refcount/refcount.h ../compile/compile.h
traverse.o: ../optimize/DupTree.h ../optimize/Inline.h ../optimize/Unroll.h
traverse.o: ../optimize/Unswitch.h ../psi-opt/ArrayElimination.h
traverse.o: ../psi-opt/index.h ../modules/writesib.h ../modules/readsib.h
traverse.o: ../modules/implicittypes.h ../objects/objinit.h
traverse.o: ../modules/analysis.h ../modules/checkdec.h ../objects/objects.h
traverse.o: ../objects/uniquecheck.h ../objects/rmvoidfun.h
traverse.o: ../compile/precompile.h ../modules/cccall.h
traverse.o: ../compile/Old2NewWith.h ../optimize/WithloopFolding.h traverse.h
traverse.o: node_info.mac
tree.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h tree.h types.h
tree.o: tree_basic.h tree_compound.h Error.h /usr/include/stdio.h
tree.o: /usr/include/sys/va_list.h /usr/include/string.h globals.h
tree.o: internal_lib.h ../../inc/dbug.h my_debug.h ../scanparse/scnprs.h
tree.o: ../scanparse/y.tab.h ../optimize/optimize.h free.h
tree.o: /usr/include/malloc.h
tree_basic.o: types.h tree_basic.h Error.h /usr/include/stdio.h
tree_basic.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
tree_basic.o: /usr/include/string.h /usr/include/stdlib.h globals.h
tree_basic.o: ../../inc/dbug.h my_debug.h ../scanparse/scnprs.h
tree_basic.o: ../scanparse/y.tab.h free.h /usr/include/malloc.h
tree_basic.o: internal_lib.h prf_node_info.mac
tree_compound.o: types.h tree_basic.h tree_compound.h Error.h
tree_compound.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
tree_compound.o: /usr/include/sys/va_list.h /usr/include/string.h
tree_compound.o: /usr/include/stdlib.h globals.h ../../inc/dbug.h my_debug.h
tree_compound.o: free.h /usr/include/malloc.h internal_lib.h
usage.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
usage.o: /usr/include/sys/va_list.h globals.h types.h ../../inc/dbug.h
