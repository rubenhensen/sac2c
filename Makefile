MAKE=make
CC=gcc -ansi -g 
LEX=lex
YACC=yacc -dv
LIBS=-ly -ll
RM=rm -f

LIB=lib/dbug.o
GLOBAL= src/global/main.o src/global/Error.o src/global/usage.o \
        src/global/my_debug.o src/global/traverse.o
SCANP= src/scanparse/y.tab.o src/scanparse/lex.yy.o
PRINT= src/print/print.o
FLATTEN= src/flatten/flatten.o
OBJ=$(GLOBAL) $(SCANP) $(PRINT) $(FLATTEN)

all: dummy sac2c

dummy:
	(cd src/global; $(MAKE) )
	(cd src/scanparse; $(MAKE) )
	(cd src/print; $(MAKE) )
	(cd src/flatten; $(MAKE) )

sac2c: $(OBJ) $(LIB)
	$(CC) -o sac2c $(OBJ) $(LIB) $(LIBS)

deps:
	(cd src/global; $(MAKE) deps)
	(cd src/scanparse; $(MAKE) deps)
	(cd src/print; $(MAKE) deps)
	(cd src/flatten; $(MAKE) deps)

clean:
	(cd src/global; $(MAKE) clean)
	(cd src/scanparse; $(MAKE) clean)
	(cd src/print; $(MAKE) clean)
	(cd src/flatten; $(MAKE) clean)
	$(RM) sac2c
