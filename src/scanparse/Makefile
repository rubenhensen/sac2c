#
# $Log$
# Revision 1.17  1997/11/11 16:49:24  sbs
# make deps done
#
# Revision 1.16  1997/10/29 14:36:13  srs
# added CFLAGS to compilerarguments for CCL
#
# Revision 1.15  1997/08/05 11:32:25  sbs
# make deps done
#
# Revision 1.14  1996/01/04 18:11:07  cg
# make deps done
#
# Revision 1.13  1996/01/02  15:55:15  cg
# added compilation of scnprs.c
#
# Revision 1.12  1995/12/15  10:41:08  cg
# make deps done
#
# Revision 1.11  1995/10/08  11:29:48  sbs
# make deps done
#
# Revision 1.10  1995/06/30  11:28:05  hw
# parameter( -n) to call of lex inserted
#
# Revision 1.9  1995/05/15  13:38:46  asi
# make deps done
#
# Revision 1.8  1995/04/04  11:03:48  sbs
# make deps done
#
# Revision 1.7  1994/12/30  17:37:07  sbs
# *** empty log message ***
#
# Revision 1.6  1994/12/30  17:28:08  sbs
# *** empty log message ***
#
# Revision 1.5  1994/12/14  13:56:58  sbs
# YYDEBUG set for y.tab.c
#
# Revision 1.4  1994/11/23  12:42:42  sbs
# compilation of y.tab.c without -Wall -pedantic !!!
#
# Revision 1.3  1994/11/17  13:18:05  sbs
# local gcc call for lex output!!
#
# Revision 1.2  1994/11/10  15:22:47  sbs
# RCS-header inserted
#
#

CC=gcc -ansi -g
CCL=gcc -ansi -g $(CFLAGS)
LEX=lex -n
YACC=yacc -dv
LIBS=-ly -ll

INCS=-I../global -I../../inc -I../modules

all: y.tab.o lex.yy.o scnprs.o

y.tab.o: y.tab.c
	$(CCL) -o y.tab.o -DYYDEBUG -c y.tab.c $(INCS)

lex.yy.o: lex.yy.c y.tab.h
	$(CCL) -o lex.yy.o -c lex.yy.c $(INCS)

lex.yy.c: sac.l
	$(LEX) sac.l

y.tab.c y.tab.h: sac.y 
	$(YACC)  sac.y

scnprs.o: scnprs.c
	$(CC) -o scnprs.o -c scnprs.c $(INCS)



deps: y.tab.c lex.yy.c
	makedepend $(INCS) *.c


clean:
	$(RM) y.tab.h y.tab.c lex.yy.c *.o *.bak y.output



# DO NOT DELETE THIS LINE -- make depend depends on it.

lex.yy.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
lex.yy.o: /usr/include/stdlib.h ../global/tree.h ../global/types.h
lex.yy.o: ../global/tree_basic.h ../global/tree_compound.h ../global/Error.h
lex.yy.o: /usr/include/string.h ../global/globals.h ../global/internal_lib.h
lex.yy.o: y.tab.h
scnprs.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
scnprs.o: ../global/types.h ../global/tree_basic.h ../global/Error.h
scnprs.o: /usr/include/string.h /usr/include/stdlib.h ../global/globals.h
scnprs.o: ../../inc/dbug.h ../modules/filemgr.h scnprs.h y.tab.h
y.tab.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
y.tab.o: /usr/include/stdlib.h /usr/include/memory.h ../../inc/dbug.h
y.tab.o: ../global/tree.h ../global/types.h ../global/tree_basic.h
y.tab.o: ../global/tree_compound.h ../global/Error.h /usr/include/string.h
y.tab.o: ../global/globals.h ../global/internal_lib.h ../global/my_debug.h
y.tab.o: ../global/free.h /usr/include/malloc.h ../modules/readsib.h
y.tab.o: /usr/include/values.h
