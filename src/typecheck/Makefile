#
# $Log$
# Revision 1.22  1998/02/16 21:43:58  dkr
# make deps done
#
# Revision 1.21  1997/11/20 15:02:06  dkr
# make deps done
#
# Revision 1.20  1997/11/12 15:59:49  sbs
# CC+CCFLAGS handling changed.
# Now, it suffices to preset CC=cc if the SUN-compiler isdesired
# Otherwise, gcc is set with -Wall and -ansi
#
# Revision 1.19  1997/11/11 16:49:25  sbs
# make deps done
#
# Revision 1.18  1997/08/05 11:32:26  sbs
# make deps done
#
# Revision 1.17  1997/05/28 12:48:29  sbs
# make deps done
#
# Revision 1.16  1997/05/14  12:20:00  sbs
# make deps
#
# Revision 1.15  1997/05/14  06:51:54  sbs
# -I../compile included
#
# Revision 1.14  1996/07/18  12:22:58  sbs
# prim_fun.o included in clean-line
#
# Revision 1.13  1996/02/16  09:46:49  sbs
# make deps done
#
# Revision 1.12  1996/01/04  18:11:08  cg
# make deps done
#
# Revision 1.11  1995/12/15  10:41:09  cg
# make deps done
#
# Revision 1.10  1995/10/08  11:29:50  sbs
# make deps done
#
# Revision 1.9  1995/07/07  15:47:48  asi
# make deps done
#
# Revision 1.8  1995/06/23  13:06:58  hw
# -I../optimize added
#
# Revision 1.7  1995/03/28  12:08:53  hw
# make deps done
#
# Revision 1.6  1995/03/17  15:50:48  hw
# make deps done
#
# Revision 1.5  1995/02/02  14:59:05  hw
# added rule prim_fun.o
# make deps done
#
# Revision 1.4  1995/01/04  16:04:37  hw
# added -I../modules to INCS
# make deps done
#
# Revision 1.3  1994/12/09  10:44:24  sbs
# make deps done
#
# Revision 1.2  1994/12/06  16:31:12  hw
# added dependencies
#
# Revision 1.1  1994/12/02  10:13:42  hw
# Initial revision
#
# 
#

CC        := gcc
CCFLAGS   := -ansi -Wall -g
CFLAGS    := -D__EXTENSIONS__

RM=rm -f
INCS=-I../global -I../../inc -I../print -I../modules -I../optimize -I../compile

all: typecheck.o prim_fun.o

typecheck.o: typecheck.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o typecheck.o -c typecheck.c $(INCS)

prim_fun.o: prim_fun.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o prim_fun.o -c prim_fun.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) typecheck.o prim_fun.o *.bak
# DO NOT DELETE THIS LINE -- make depend depends on it.

prim_fun.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
prim_fun.o: ../global/tree.h ../global/types.h ../global/tree_basic.h
prim_fun.o: ../global/tree_compound.h ../global/Error.h /usr/include/stdio.h
prim_fun.o: /usr/include/sys/va_list.h /usr/include/string.h
prim_fun.o: ../global/globals.h ../global/internal_lib.h prim_fun.h
prim_fun.o: ../../inc/dbug.h ../global/my_debug.h typecheck.h
prim_fun.o: ../global/access_macros.h ../print/convert.h ../global/free.h
prim_fun.o: /usr/include/malloc.h prim_fun_tt.mac prim_fun_ft.mac
typecheck.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
typecheck.o: /usr/include/stddef.h /usr/include/string.h ../global/tree.h
typecheck.o: ../global/types.h ../global/tree_basic.h
typecheck.o: ../global/tree_compound.h ../global/Error.h /usr/include/stdio.h
typecheck.o: /usr/include/sys/va_list.h ../global/globals.h
typecheck.o: ../global/internal_lib.h ../print/print.h ../global/my_debug.h
typecheck.o: ../../inc/dbug.h ../global/traverse.h typecheck.h
typecheck.o: ../print/convert.h ../modules/import.h prim_fun.h
typecheck.o: ../global/free.h /usr/include/malloc.h ../global/access_macros.h
typecheck.o: ../optimize/DupTree.h ../compile/profile.h prim_fun_tt.mac
