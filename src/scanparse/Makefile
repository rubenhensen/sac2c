#
# $Log$
# Revision 2.4  1999/05/10 13:28:23  sbs
# adjusted for bison usage...
#
# Revision 2.3  1999/03/11 18:18:20  dkr
# make deps done
#
# Revision 2.2  1999/02/26 14:17:38  dkr
# make deps done
#
# Revision 2.1  1999/02/23 12:40:29  sacbase
# new release made
#
# Revision 1.25  1998/04/09 21:27:01  dkr
# make deps done
#
# Revision 1.24  1998/04/09 21:25:47  dkr
# added 'optimize' to INCS
#
# Revision 1.23  1998/03/04 00:09:54  dkr
# 'clean' now deletes the subdir .sb, too
#
# Revision 1.22  1998/03/03 22:08:56  dkr
# make deps done
#
# Revision 1.21  1998/02/16 21:43:57  dkr
# make deps done
#
# Revision 1.20  1997/12/16 16:01:16  sbs
# typecheck included into incs
#
# Revision 1.19  1997/11/20 15:02:05  dkr
# make deps done
#
# Revision 1.18  1997/11/12 15:59:49  sbs
# CC+CCFLAGS handling changed.
# Now, it suffices to preset CC=cc if the SUN-compiler isdesired
# Otherwise, gcc is set with -Wall and -ansi
#
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

CC        := gcc
CCFLAGS   := -ansi -Wall -g
CFLAGS    := -D__EXTENSIONS__

gcc_FLAGS := -ansi
cc_FLAGS  := -xsb

CCL_FLAGS := $($(CC)_FLAGS) -g

LEX       :=flex -n
YACC      :=bison -y -dv
LIBS      :=-ly -ll

INCS=-I../global -I../../inc -I../modules -I../typecheck -I../optimize

all: y.tab.o lex.yy.o scnprs.o

y.tab.o: y.tab.c
	$(CC) $(CCL_FLAGS) $(CFLAGS) -o y.tab.o -DYYDEBUG -c y.tab.c $(INCS)

lex.yy.o: lex.yy.c y.tab.h
	$(CC) $(CCL_FLAGS) $(CFLAGS) -o lex.yy.o -c lex.yy.c $(INCS)

lex.yy.c: sac.l
	$(LEX) sac.l

y.tab.c y.tab.h: sac.y 
	$(YACC)  sac.y

scnprs.o: scnprs.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o scnprs.o -c scnprs.c $(INCS)



deps: y.tab.c lex.yy.c
	makedepend $(INCS) *.c


clean:
	$(RM) y.tab.h y.tab.c lex.yy.c *.o *.bak y.output
	$(RM) -r .sb

# DO NOT DELETE THIS LINE -- make depend depends on it.

lex.yy.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
lex.yy.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
lex.yy.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
lex.yy.o: /usr/include/stdlib.h /usr/include/inttypes.h
lex.yy.o: /usr/include/sys/inttypes.h /usr/include/sys/int_types.h
lex.yy.o: ../global/tree.h ../global/types.h ../global/tree_basic.h
lex.yy.o: ../global/tree_compound.h ../global/Error.h /usr/include/string.h
lex.yy.o: ../global/globals.h ../global/internal_lib.h ../global/resource.h
lex.yy.o: y.tab.h
scnprs.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
scnprs.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
scnprs.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
scnprs.o: /usr/include/string.h ../global/types.h ../global/tree_basic.h
scnprs.o: ../global/Error.h /usr/include/stdlib.h ../global/globals.h
scnprs.o: ../../inc/dbug.h ../modules/filemgr.h scnprs.h ../global/resource.h
scnprs.o: y.tab.h
y.tab.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
y.tab.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
y.tab.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
y.tab.o: /usr/include/stdlib.h /usr/include/memory.h /usr/include/sys/types.h
y.tab.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
y.tab.o: ../../inc/dbug.h ../global/tree.h ../global/types.h
y.tab.o: ../global/tree_basic.h ../global/tree_compound.h ../global/Error.h
y.tab.o: /usr/include/string.h ../global/globals.h ../global/internal_lib.h
y.tab.o: ../typecheck/typecheck.h ../optimize/DupTree.h ../global/my_debug.h
y.tab.o: ../global/free.h /usr/include/malloc.h ../modules/readsib.h
y.tab.o: ../global/resource.h /usr/include/inttypes.h
y.tab.o: /usr/include/sys/inttypes.h /usr/include/values.h
