#
# $Log$
# Revision 1.18  1996/01/09 09:19:14  cg
# added compilation of libsac.c
#
# Revision 1.17  1996/01/04  18:11:10  cg
# make deps done
#
# Revision 1.16  1995/12/15  10:41:12  cg
# make deps done
#
# Revision 1.15  1995/12/01  17:25:42  cg
# added compilation of precompile.c
#
# Revision 1.14  1995/10/08  11:29:56  sbs
# make deps done
#
# Revision 1.13  1995/07/07  15:47:53  asi
# make deps done
#
# Revision 1.12  1995/05/26  12:12:30  asi
# make deps done
#
# Revision 1.11  1995/05/15  13:38:54  asi
# make deps done
#
# Revision 1.10  1995/05/04  11:46:29  sbs
# make clean adjusted
#
# Revision 1.9  1995/05/04  11:44:35  sbs
# new deps made
#
# Revision 1.8  1995/04/20  10:21:23  asi
# make deps done
#
# Revision 1.7  1995/04/19  13:15:55  sbs
# make deps done
#
# Revision 1.6  1995/04/11  15:02:06  sbs
# dbug.o added in BEtest-rule
#
# Revision 1.5  1995/04/05  15:55:01  asi
# make deps done
#
# Revision 1.4  1995/04/04  11:03:54  sbs
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
MACS=icm.data icm_decl.c icm_args.c icm_comment.c icm_trace.c

all: compile.o icm2c.o precompile.o libsac.a

BEtest: icm2c.c $(MACS)
	$(CC) -o BEtest icm2c.c -DTEST_BACKEND $(INCS) ../../lib/dbug.o

compile.o: compile.c
	$(CC) -o compile.o -c compile.c $(INCS)

icm2c.o: icm2c.c
	$(CC) -o icm2c.o -c icm2c.c $(INCS)

precompile.o: precompile.c
	$(CC) -o precompile.o -c precompile.c $(INCS)

libsac.a: libsac.o
	ar rc libsac.a libsac.o
	ranlib libsac.a

libsac.o: libsac.c
	$(CC) -o libsac.o -c libsac.c $(INCS)


deps:
	makedepend $(INCS) *.c

clean:
	$(RM) compile.o icm2c.o precompile.o libsac.a libsac.o  \
              *.bak BEtest *.sib *.sac



# DO NOT DELETE THIS LINE -- make depend depends on it.

compile.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h
compile.o: /usr/include/string.h ../print/print.h ../global/tree.h
compile.o: ../global/types.h ../global/tree_basic.h ../global/tree_compound.h
compile.o: ../global/Error.h /usr/include/stdio.h ../global/globals.h
compile.o: ../global/internal_lib.h ../../inc/dbug.h ../global/my_debug.h
compile.o: ../global/access_macros.h ../global/traverse.h compile.h
compile.o: ../print/convert.h ../refcount/refcount.h ../typecheck/typecheck.h
compile.o: ../global/free.h /usr/include/malloc.h
icm2c.o: /usr/include/malloc.h /usr/include/stdio.h ../print/print.h
icm2c.o: ../global/tree.h ../global/types.h ../global/tree_basic.h
icm2c.o: ../global/tree_compound.h ../global/Error.h /usr/include/string.h
icm2c.o: /usr/include/sys/stdtypes.h /usr/include/stdlib.h
icm2c.o: ../global/globals.h ../global/internal_lib.h ../../inc/dbug.h
icm2c.o: ../global/my_debug.h ../../inc/main.h trace.h icm_decl.c icm.data
icm2c.o: icm_args.c icm_comment.c icm_trace.c
icm_args.o: icm.data
icm_comment.o: icm.data
icm_decl.o: icm.data
icm_trace.o: icm.data
precompile.o: ../global/types.h ../global/tree_basic.h
precompile.o: ../global/tree_compound.h ../global/Error.h
precompile.o: /usr/include/stdio.h /usr/include/string.h
precompile.o: /usr/include/sys/stdtypes.h /usr/include/stdlib.h
precompile.o: ../global/globals.h ../global/free.h ../../inc/dbug.h
precompile.o: ../global/my_debug.h /usr/include/malloc.h
precompile.o: ../global/internal_lib.h ../print/convert.h
precompile.o: ../global/traverse.h
