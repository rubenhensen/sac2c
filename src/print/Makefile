#
# $Log$
# Revision 2.2  1999/02/26 14:17:37  dkr
# make deps done
#
# Revision 2.1  1999/02/23 12:40:19  sacbase
# new release made
#
# Revision 1.31  1999/02/09 20:04:19  dkr
# make deps done
#
# Revision 1.29  1998/06/18 13:28:22  cg
# added ../concurrent as include directory
#
# Revision 1.28  1998/05/21 09:51:51  dkr
# make deps done
#
# Revision 1.23  1998/03/04 00:10:33  dkr
# 'clean' now deletes the subdir .sb, too
#
# Revision 1.22  1997/11/20 15:02:04  dkr
# make deps done
#
# Revision 1.21  1997/11/12 15:59:49  sbs
# CC+CCFLAGS handling changed.
# Now, it suffices to preset CC=cc if the SUN-compiler isdesired
# Otherwise, gcc is set with -Wall and -ansi
#
# Revision 1.20  1997/11/11 16:49:23  sbs
# make deps done
#
# Revision 1.17  1996/07/18  12:22:08  sbs
# convert.o included in clean-line
#
# Revision 1.16  1996/02/16  09:46:46  sbs
# make deps done
#
# Revision 1.15  1996/01/05  12:31:29  cg
# Now, we look even in ../modules for include files
#
# Revision 1.14  1996/01/04  18:11:06  cg
# make deps done
#
# Revision 1.8  1995/04/03  14:02:12  sbs
# ../compile inserted for icm.data-access
#
# Revision 1.7  1995/02/28  18:27:34  asi
# added optimize-dir to include-path
#
# Revision 1.6  1995/01/11  17:45:12  hw
# make deps done
#
# Revision 1.5  1994/12/05  11:16:46  hw
# added rule for conver.o
#
# Revision 1.4  1994/11/10  15:35:50  sbs
# RCS-header inserted
#
#

CC        := gcc
CCFLAGS   := -ansi -Wall -g
CFLAGS    := -D__EXTENSIONS__


INCS=-I../global -I../../inc -I../optimize -I../psi-opt -I../compile -I../modules -I../concurrent

all: print.o convert.o

print.o: print.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o print.o -c print.c $(INCS)

convert.o: convert.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o convert.o -c convert.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) print.o convert.o *.bak
	$(RM) -r .sb

# DO NOT DELETE THIS LINE -- make depend depends on it.

convert.o: /usr/include/string.h /usr/include/sys/feature_tests.h
convert.o: /usr/include/stdlib.h /usr/include/math.h ../global/internal_lib.h
convert.o: ../../inc/dbug.h /usr/include/stdio.h /usr/include/sys/va_list.h
convert.o: ../global/tree_basic.h ../global/types.h ../global/tree_compound.h
convert.o: ../global/Error.h ../global/globals.h ../global/type_info.mac
print.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
print.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
print.o: /usr/include/string.h ../global/tree.h ../global/types.h
print.o: ../global/tree_basic.h ../global/tree_compound.h ../global/Error.h
print.o: ../global/globals.h ../global/internal_lib.h print.h
print.o: ../global/my_debug.h ../../inc/dbug.h ../global/traverse.h convert.h
print.o: ../optimize/DataFlowMask.h ../optimize/optimize.h ../global/free.h
print.o: /usr/include/malloc.h ../optimize/generatemasks.h
print.o: ../modules/filemgr.h ../compile/gen_startup_code.h
print.o: ../psi-opt/WithloopFolding.h ../concurrent/scheduling.h
print.o: ../compile/icm.data ../global/prf_node_info.mac
