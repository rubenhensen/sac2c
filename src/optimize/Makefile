#
# $Log$
# Revision 1.4  1995/03/06 11:09:01  asi
# added -I../typecheck and make deps
#
# Revision 1.3  1995/02/13  17:17:59  asi
# ConstantFolding.o and DeadCodeRemoval.o added
#
# Revision 1.2  1995/01/18  17:40:09  asi
# make deps
#
# Revision 1.1  1994/12/09  10:48:23  sbs
# Initial revision
#
#

MAKE=make
CC=gcc -ansi -g

INCS=-I../global -I../../inc -I../typecheck

all: optimize.o ConstantFolding.o DeadCodeRemoval.o

optimize.o: optimize.c
	$(CC) -o optimize.o -c optimize.c $(INCS)

ConstantFolding.o: ConstantFolding.c
	$(CC) -o ConstantFolding.o -c ConstantFolding.c $(INCS)

DeadCodeRemoval.o: DeadCodeRemoval.c
	$(CC) -o DeadCodeRemoval.o -c DeadCodeRemoval.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak

# DO NOT DELETE THIS LINE -- make depend depends on it.

ConstantFolding.o: /usr/include/stdio.h /usr/include/stdlib.h
ConstantFolding.o: /usr/include/sys/stdtypes.h ../global/tree.h
ConstantFolding.o: ../global/type_info.mac ../global/node_info.mac
ConstantFolding.o: ../global/prf_node_info.mac ../global/free.h
ConstantFolding.o: ../global/Error.h ../../inc/dbug.h ../global/my_debug.h
ConstantFolding.o: ../global/traverse.h ../typecheck/typecheck.h optimize.h
ConstantFolding.o: ConstantFolding.h
DeadCodeRemoval.o: /usr/include/stdio.h /usr/include/stdlib.h
DeadCodeRemoval.o: /usr/include/sys/stdtypes.h ../global/tree.h
DeadCodeRemoval.o: ../global/type_info.mac ../global/node_info.mac
DeadCodeRemoval.o: ../global/prf_node_info.mac ../global/free.h
DeadCodeRemoval.o: ../global/Error.h ../../inc/dbug.h ../global/my_debug.h
DeadCodeRemoval.o: ../global/traverse.h optimize.h DeadCodeRemoval.h
optimize.o: /usr/include/stdio.h /usr/include/stdlib.h
optimize.o: /usr/include/sys/stdtypes.h ../global/tree.h
optimize.o: ../global/type_info.mac ../global/node_info.mac
optimize.o: ../global/prf_node_info.mac ../global/free.h ../global/Error.h
optimize.o: ../../inc/dbug.h ../global/my_debug.h ../global/traverse.h
optimize.o: optimize.h ConstantFolding.h DeadCodeRemoval.h
