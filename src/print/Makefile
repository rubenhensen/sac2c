#
# $Log$
# Revision 1.10  1995/05/15 13:38:44  asi
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

INCS=-I../global -I../../inc -I../optimize -I../compile

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
convert.o: /usr/include/stdlib.h ../global/tree.h ../global/type_info.mac
convert.o: ../global/node_info.mac ../global/prf_node_info.mac
convert.o: ../../inc/dbug.h /usr/include/stdio.h convert.h
print.o: /usr/include/stdio.h /usr/include/stdlib.h
print.o: /usr/include/sys/stdtypes.h /usr/include/string.h print.h
print.o: ../global/tree.h ../global/type_info.mac ../global/node_info.mac
print.o: ../global/prf_node_info.mac ../global/my_debug.h ../../inc/dbug.h
print.o: ../global/traverse.h ../global/Error.h convert.h
print.o: ../optimize/optimize.h ../compile/icm.data ../compile/trace.h
