CC=gcc -ansi -g 
LEX=lex
YACC=yacc -dv
LIBS=-ly -ll

INCS=-I../global -I../../inc

all: y.tab.o lex.yy.o

y.tab.o: y.tab.c
	$(CC) -o y.tab.o -c y.tab.c $(INCS)

lex.yy.o: lex.yy.c y.tab.h
	$(CC) -o lex.yy.o -c lex.yy.c $(INCS)

lex.yy.c: sac.l
	$(LEX) sac.l

y.tab.c y.tab.h: sac.y 
	$(YACC)  sac.y

deps: y.tab.c lex.yy.c
	makedepend $(INCS) *.c


clean:
	$(RM) y.tab.h y.tab.c lex.yy.c *.o *.bak y.output

# DO NOT DELETE THIS LINE -- make depend depends on it.

lex.yy.o: /usr/include/stdio.h /usr/include/stdlib.h
lex.yy.o: /usr/include/sys/stdtypes.h ../global/tree.h
lex.yy.o: ../global/node_info.mac ../global/prf_node_info.mac y.tab.h
y.tab.o: /usr/include/stdio.h /usr/include/stdlib.h
y.tab.o: /usr/include/sys/stdtypes.h /usr/include/memory.h ../../inc/dbug.h
y.tab.o: ../global/tree.h ../global/node_info.mac ../global/prf_node_info.mac
y.tab.o: ../global/my_debug.h
