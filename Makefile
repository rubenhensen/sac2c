#
# $Log$
# Revision 1.12  1995/02/03 07:57:47  hw
# added  TYPECHECK
#
# Revision 1.11  1994/12/20  17:33:45  hw
# added free.o & tree.o
#
# Revision 1.10  1994/12/16  14:41:26  sbs
# Put the invokation of make in scanparse in front, since
# some other files include y.tab.h (via scnprs.h)
#
# Revision 1.9  1994/12/16  14:33:25  sbs
# import.o inserted
#
# Revision 1.8  1994/12/11  17:30:35  sbs
# modules inserted
#
# Revision 1.7  1994/12/09  10:42:25  sbs
# optimize inserted
#
# Revision 1.6  1994/12/05  15:07:25  hw
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
        src/global/my_debug.o src/global/traverse.o  src/global/tree.o \
         src/global/free.o
SCANP= src/scanparse/y.tab.o src/scanparse/lex.yy.o
PRINT= src/print/print.o src/print/convert.o
FLATTEN= src/flatten/flatten.o
TYPECHECK= src/typecheck/typecheck.o src/typecheck/prim_fun.o
OPTIMIZE= src/optimize/optimize.o
MODULES= src/modules/filemgr.o src/modules/import.o
OBJ=$(GLOBAL) $(SCANP) $(PRINT) $(FLATTEN) $(TYPECHECK) $(OPTIMIZE) $(MODULES)

all: dummy sac2c

dummy:
	(cd src/scanparse; $(MAKE) )
	(cd src/global; $(MAKE) )
	(cd src/print; $(MAKE) )
	(cd src/flatten; $(MAKE) )
	(cd src/typecheck; $(MAKE) )
	(cd src/optimize; $(MAKE) )
	(cd src/modules; $(MAKE) )

sac2c: $(OBJ) $(LIB)
	$(CC) -o sac2c $(OBJ) $(LIB) $(LIBS)

deps:
	(cd src/scanparse; $(MAKE) deps)
	(cd src/global; $(MAKE) deps)
	(cd src/print; $(MAKE) deps)
	(cd src/flatten; $(MAKE) deps)
	(cd src/typecheck; $(MAKE) deps)
	(cd src/optimize; $(MAKE) deps)
	(cd src/modules; $(MAKE) deps)
        
clean:
	(cd src/scanparse; $(MAKE) clean)
	(cd src/global; $(MAKE) clean)
	(cd src/print; $(MAKE) clean)
	(cd src/flatten; $(MAKE) clean)
	(cd src/typecheck; $(MAKE) clean)
	(cd src/optimize; $(MAKE) clean)
	(cd src/modules; $(MAKE) clean)
	$(RM) sac2c
