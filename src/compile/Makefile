#
# $Log$
# Revision 1.1  1995/03/29 12:12:52  hw
# Initial revision
#
#
# 


MAKE=make
CC=gcc -ansi -g

INCS=-I../global -I../../inc -I../print -I../typecheck -I../refcount

all: compile.o

compile.o: compile.c
	$(CC) -o compile.o -c compile.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) compile.o *.bak

# DO NOT DELETE THIS LINE -- make depend depends on it.

compile.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h ../print/print.h
compile.o: ../global/tree.h ../global/type_info.mac ../global/node_info.mac
compile.o: ../global/prf_node_info.mac ../../inc/dbug.h /usr/include/stdio.h
compile.o: ../global/internal_lib.h ../global/access_macros.h
compile.o: ../global/traverse.h compile.h ../print/convert.h
compile.o: ../refcount/refcount.h ../typecheck/typecheck.h
icm2c.o: /usr/include/stdio.h ../print/print.h ../global/tree.h
icm2c.o: ../global/type_info.mac ../global/node_info.mac
icm2c.o: ../global/prf_node_info.mac ../../inc/dbug.h icm_comment.c icm.data
icm2c.o: icm_decl.c
icm_args.o: icm.data
icm_comment.o: icm.data
icm_decl.o: icm.data
