#
# $Log$
# Revision 1.9  1995/07/07 15:47:48  asi
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

CC=gcc -ansi -g
RM=rm -f
INCS=-I../global -I../../inc -I../print -I../modules -I../optimize

all: typecheck.o prim_fun.o

typecheck.o: typecheck.c
	$(CC) -o typecheck.o -c typecheck.c $(INCS)

prim_fun.o: prim_fun.c
	$(CC) -o prim_fun.o -c prim_fun.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) typecheck.o *.bak
# DO NOT DELETE THIS LINE -- make depend depends on it.

prim_fun.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h
prim_fun.o: ../global/tree.h ../global/type_info.mac ../global/node_info.mac
prim_fun.o: ../global/prf_node_info.mac prim_fun.h ../../inc/dbug.h
prim_fun.o: /usr/include/stdio.h ../global/Error.h ../global/my_debug.h
prim_fun.o: ../global/internal_lib.h typecheck.h ../global/access_macros.h
prim_fun.o: ../print/convert.h prim_fun_tt.mac prim_fun_ft.mac
typecheck.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h
typecheck.o: /usr/include/stddef.h /usr/include/string.h ../global/tree.h
typecheck.o: ../global/type_info.mac ../global/node_info.mac
typecheck.o: ../global/prf_node_info.mac ../global/my_debug.h
typecheck.o: ../../inc/dbug.h /usr/include/stdio.h ../global/traverse.h
typecheck.o: typecheck.h ../global/Error.h ../print/convert.h
typecheck.o: ../modules/import.h prim_fun.h ../global/free.h
typecheck.o: ../global/internal_lib.h ../global/access_macros.h
typecheck.o: ../optimize/DupTree.h prim_fun_tt.mac
