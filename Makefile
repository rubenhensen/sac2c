#
# $Log$
# Revision 1.26  1995/07/24 11:40:53  asi
# added ArrayElimination.o
#
# Revision 1.25  1995/07/07  15:00:40  asi
# added loop unswitching
#
# Revision 1.24  1995/06/02  09:53:02  sbs
# psi-opt inserted.
#
# Revision 1.23  1995/05/26  14:23:42  asi
# function inlineing and loop unrolling added
#
# Revision 1.22  1995/05/01  15:36:19  asi
# src/optimize/DupTree.o inserted
#
# Revision 1.21  1995/04/05  15:52:38  asi
# loop invariant removal added
#
# Revision 1.20  1995/04/03  10:11:30  sbs
# src/compile/icm2c.o inserted
#
# Revision 1.19  1995/03/29  12:10:37  hw
# *** empty log message ***
#
# Revision 1.18  1995/03/29  11:49:43  hw
# compile inserted
#
# Revision 1.17  1995/03/28  12:07:46  hw
# added internal_lib.o
#
# Revision 1.16  1995/03/22  11:19:57  asi
# added debuging tools for malloc, free etc. - malloc.o
# should be removed in final version
#
# Revision 1.15  1995/03/17  17:39:11  asi
# WorkReduction.o added
#
# Revision 1.14  1995/03/10  10:52:39  hw
# refcount inserted
#
# Revision 1.13  1995/02/13  17:25:14  asi
# added ConstantFolding.o and DeadCodeRemoval.o
#
# Revision 1.12  1995/02/03  07:57:47  hw
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

LIB=lib/dbug.o /usr/lib/debug/malloc.o /usr/lib/libm.a
GLOBAL= src/global/main.o src/global/Error.o src/global/usage.o \
        src/global/my_debug.o src/global/traverse.o  src/global/tree.o \
        src/global/free.o src/global/internal_lib.o
SCANP= src/scanparse/y.tab.o src/scanparse/lex.yy.o
PRINT= src/print/print.o src/print/convert.o
FLATTEN= src/flatten/flatten.o
TYPECHECK= src/typecheck/typecheck.o src/typecheck/prim_fun.o
OPTIMIZE= src/optimize/optimize.o src/optimize/ConstantFolding.o \
          src/optimize/DeadCodeRemoval.o src/optimize/WorkReduction.o \
	  src/optimize/LoopInvariantRemoval.o src/optimize/DupTree.o \
	  src/optimize/Inline.o src/optimize/Unroll.o src/optimize/Unswitch.o \
	  src/psi-opt/ArrayElimination.o
PSIOPT= src/psi-opt/index.o src/psi-opt/psi-opt.o
MODULES= src/modules/filemgr.o src/modules/import.o
REFCOUNT= src/refcount/refcount.o
COMPILE= src/compile/compile.o src/compile/icm2c.o

OBJ=$(GLOBAL) $(SCANP) $(PRINT) $(FLATTEN) $(TYPECHECK) $(OPTIMIZE) $(MODULES) \
    $(REFCOUNT) $(COMPILE) $(PSIOPT)

all: dummy sac2c

dummy:
	(cd src/scanparse; $(MAKE) )
	(cd src/global; $(MAKE) )
	(cd src/print; $(MAKE) )
	(cd src/flatten; $(MAKE) )
	(cd src/typecheck; $(MAKE) )
	(cd src/optimize; $(MAKE) )
	(cd src/modules; $(MAKE) )
	(cd src/refcount; $(MAKE) )       
	(cd src/compile; $(MAKE) )
	(cd src/psi-opt; $(MAKE) )

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
	(cd src/refcount; $(MAKE) deps )
	(cd src/compile; $(MAKE) deps )
	(cd src/psi-opt; $(MAKE) deps)

clean:
	(cd src/scanparse; $(MAKE) clean)
	(cd src/global; $(MAKE) clean)
	(cd src/print; $(MAKE) clean)
	(cd src/flatten; $(MAKE) clean)
	(cd src/typecheck; $(MAKE) clean)
	(cd src/optimize; $(MAKE) clean)
	(cd src/modules; $(MAKE) clean)
	(cd src/refcount; $(MAKE) clean)
	(cd src/compile; $(MAKE) clean )
	(cd src/psi-opt; $(MAKE) clean)
	$(RM) sac2c
