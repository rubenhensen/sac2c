#
# $Log$
# Revision 1.19  1997/08/05 11:32:24  sbs
# make deps done
#
# Revision 1.18  1997/05/28 12:48:27  sbs
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
# Revision 1.13  1995/12/15  10:41:07  cg
# make deps done
#
# Revision 1.12  1995/10/08  11:29:46  sbs
# make deps done
#
# Revision 1.11  1995/05/26  12:12:23  asi
# make deps done
#
# Revision 1.10  1995/05/15  13:38:44  asi
# make deps done
#
# Revision 1.9  1995/04/04  09:30:39  sbs
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

CC=gcc -ansi -g

INCS=-I../global -I../../inc -I../optimize -I../compile -I../modules

all: print.o convert.o

print.o: print.c
	$(CC) -o print.o -c print.c $(INCS)

convert.o: convert.c
	$(CC) -o convert.o -c convert.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) print.o convert.o *.bak


# DO NOT DELETE THIS LINE -- make depend depends on it.

convert.o: /usr/include/string.h /usr/include/sys/feature_tests.h
convert.o: /usr/include/stdlib.h /usr/include/math.h ../global/tree.h
convert.o: ../global/types.h ../global/tree_basic.h ../global/tree_compound.h
convert.o: ../global/Error.h /usr/include/stdio.h /usr/include/sys/va_list.h
convert.o: ../global/globals.h ../global/internal_lib.h ../../inc/dbug.h
convert.o: convert.h ../global/type_info.mac
print.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
print.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
print.o: /usr/include/string.h ../global/types.h ../global/tree_basic.h
print.o: ../global/tree_compound.h ../global/Error.h ../global/globals.h
print.o: print.h ../global/tree.h ../global/internal_lib.h
print.o: ../global/my_debug.h ../../inc/dbug.h ../global/traverse.h convert.h
print.o: ../optimize/optimize.h ../global/free.h /usr/include/malloc.h
print.o: ../compile/trace.h ../compile/profile.h ../modules/filemgr.h
print.o: ../compile/icm.data ../global/prf_node_info.mac
