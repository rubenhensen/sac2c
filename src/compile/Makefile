#
# $Log$
# Revision 1.4  1995/04/04 11:03:54  sbs
# make deps done
#
# Revision 1.3  1995/04/03  13:59:44  sbs
# icm2c.o line included
#
# Revision 1.2  1995/03/31  14:05:38  sbs
# BEtest inserted
#
# Revision 1.1  1995/03/29  12:12:52  hw
# Initial revision
#
#
# 


MAKE=make
CC=gcc -ansi -g

INCS=-I../global -I../../inc -I../print -I../typecheck -I../refcount
MACS=icm.data icm_decl.c icm_args.c icm_comment.c

all: compile.o icm2c.o

BEtest: icm2c.c $(MACS)
	$(CC) -o BEtest icm2c.c -DTEST_BACKEND $(INCS)

compile.o: compile.c
	$(CC) -o compile.o -c compile.c $(INCS)

icm2c.o: icm2c.c
	$(CC) -o icm2c.o -c icm2c.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) compile.o *.bak BEtest

# DO NOT DELETE THIS LINE -- make depend depends on it.

compile.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h ../print/print.h
compile.o: ../global/tree.h ../global/type_info.mac ../global/node_info.mac
compile.o: ../global/prf_node_info.mac ../../inc/dbug.h /usr/include/stdio.h
compile.o: ../global/my_debug.h ../global/internal_lib.h
compile.o: ../global/access_macros.h ../global/traverse.h compile.h
compile.o: ../print/convert.h ../refcount/refcount.h ../typecheck/typecheck.h
compile.o: ../global/free.h
icm2c.o: /usr/include/malloc.h /usr/include/stdio.h ../print/print.h
icm2c.o: ../global/tree.h ../global/type_info.mac ../global/node_info.mac
icm2c.o: ../global/prf_node_info.mac ../../inc/dbug.h ../global/my_debug.h
icm2c.o: icm_decl.c icm.data icm_args.c icm_comment.c
icm_args.o: icm.data
icm_comment.o: icm.data
icm_decl.o: icm.data
tutu.o: icm2c.h
