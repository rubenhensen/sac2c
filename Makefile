#
# $Log$
# Revision 1.6  1994/12/05 15:07:25  hw
# added path tp convert.o to PRINT
#
# Revision 1.5  1994/12/01  17:32:02  hw
# added TYPCHECK ect.
#
# Revision 1.4  1994/11/17  11:11:37  sbs
# -Wall -pedantic forced for all makes
#
# Revision 1.3  1994/11/10  15:45:55  sbs
# RCS-header inserted
#
#

CC=gcc -ansi -Wall -pedantic -g 
MAKE=make CC="$(CC)"
LEX=lex
YACC=yacc -dv
LIBS=-ly -ll
RM=rm -f

LIB=lib/dbug.o
GLOBAL= src/global/main.o src/global/Error.o src/global/usage.o \
        src/global/my_debug.o src/global/traverse.o
SCANP= src/scanparse/y.tab.o src/scanparse/lex.yy.o
PRINT= src/print/print.o src/print/convert.o
FLATTEN= src/flatten/flatten.o
TYPECHECK= src/typecheck/typecheck.o 
OBJ=$(GLOBAL) $(SCANP) $(PRINT) $(FLATTEN) $(TYPECHECK)

all: dummy sac2c

dummy:
	(cd src/global; $(MAKE) )
	(cd src/scanparse; $(MAKE) )
	(cd src/print; $(MAKE) )
	(cd src/flatten; $(MAKE) )
	(cd src/typecheck; $(MAKE) )

sac2c: $(OBJ) $(LIB)
	$(CC) -o sac2c $(OBJ) $(LIB) $(LIBS)

deps:
	(cd src/global; $(MAKE) deps)
	(cd src/scanparse; $(MAKE) deps)
	(cd src/print; $(MAKE) deps)
	(cd src/flatten; $(MAKE) deps)
	(cd src/typecheck; $(MAKE) deps)
        
clean:
	(cd src/global; $(MAKE) clean)
	(cd src/scanparse; $(MAKE) clean)
	(cd src/print; $(MAKE) clean)
	(cd src/flatten; $(MAKE) clean)
	(cd src/typecheck; $(MAKE) clean)
	$(RM) sac2c
