#
#
# $Log$
# Revision 3.2  2000/12/06 20:09:01  dkr
# make deps done
#
# Revision 3.1  2000/11/20 17:59:47  sacbase
# new release made
#
# Revision 2.20  2000/11/15 14:30:50  dkr
# default CC flags used now
#
# Revision 2.19  2000/11/13 15:53:08  sbs
# definition of YACC made dependent on OS-version.
#
# Revision 2.18  2000/11/02 14:22:37  dkr
# header compressed
#
# Revision 2.9  2000/01/21 12:04:23  dkr
# directory tree added
#
# Revision 2.8  2000/01/17 18:37:49  cg
# Handling of target platform unified: One of the following
# is always defined:
# SAC_FOR_SOLARIS_SPARC, SAC_FOR_LINUX_X86, OR SAC_FOR_OSF_ALPHA.
#
# Revision 2.7  1999/12/13 10:41:15  dkr
# flag -erroff=E_CAST_DOESNT_YIELD_LVALUE for CC added
#
# Revision 2.5  1999/05/11 09:57:41  sbs
# maximum line for input set to -DYYLMAX=8192 .
#
# Revision 2.4  1999/05/10 13:28:23  sbs
# adjusted for bison usage...
#
# Revision 1.24  1998/04/09 21:25:47  dkr
# added 'optimize' to INCS
#
# Revision 1.23  1998/03/04 00:09:54  dkr
# 'clean' now deletes the subdir .sb, too
#
# Revision 1.20  1997/12/16 16:01:16  sbs
# typecheck included into incs
#
# Revision 1.18  1997/11/12 15:59:49  sbs
# CC+CCFLAGS handling changed.
# Now, it suffices to preset CC=cc if the SUN-compiler isdesired
# Otherwise, gcc is set with -Wall and -ansi
#
# Revision 1.16  1997/10/29 14:36:13  srs
# added CFLAGS to compilerarguments for CCL
#
# Revision 1.13  1996/01/02  15:55:15  cg
# added compilation of scnprs.c
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


#
# OS setup:
#
# legal values for OS currently are:
#    SOLARIS_SPARC
#    LINUX_X86
#    OSF_ALPHA
#
OS        := SOLARIS_SPARC


CC        := gcc
CCFLAGS   := -ansi -Wall -g -DSAC_FOR_$(OS)
CFLAGS    := -D__EXTENSIONS__

ifeq ($(OS),OSF_ALPHA)
  LEX     := flex -n
  YACC    := yacc -dv
else
  LEX     := flex -n
  YACC    := bison -y -dv
endif

LIBS      := -ly -ll

INCS = -I../global -I../tree -I../../inc -I../modules -I../typecheck


all: y.tab.o lex.yy.o scnprs.o


y.tab.o: y.tab.c
	$(CC) $(CCL_FLAGS) $(CCFLAGS) $(CFLAGS) -o y.tab.o -DYYDEBUG -c y.tab.c $(INCS)

lex.yy.o: lex.yy.c y.tab.h
	$(CC) $(CCL_FLAGS) $(CCFLAGS) $(CFLAGS) -DYYLMAX=8192 -o lex.yy.o -c lex.yy.c $(INCS)

lex.yy.c: sac.l
	$(LEX) sac.l

y.tab.c y.tab.h: sac.y 
	$(YACC)  sac.y

%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o $@ -c $< $(INCS)


deps: y.tab.c lex.yy.c
	makedepend $(INCS) *.c


clean:
	$(RM) y.tab.h y.tab.c lex.yy.c *.o *.bak y.output
	$(RM) -r .sb SunWS_cache

# DO NOT DELETE THIS LINE -- make depend depends on it.

lex.yy.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
lex.yy.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
lex.yy.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
lex.yy.o: /usr/include/stdlib.h ../tree/tree.h ../global/types.h
lex.yy.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
lex.yy.o: /usr/include/string.h ../global/globals.h ../global/internal_lib.h
lex.yy.o: ../tree/free.h ../../inc/dbug.h ../global/my_debug.h
lex.yy.o: /usr/include/malloc.h /usr/include/sys/types.h
lex.yy.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
lex.yy.o: ../global/resource.h y.tab.h
scnprs.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
scnprs.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
scnprs.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
scnprs.o: /usr/include/string.h ../global/types.h ../tree/tree_basic.h
scnprs.o: ../global/Error.h /usr/include/stdlib.h ../global/globals.h
scnprs.o: ../../inc/dbug.h ../modules/filemgr.h scnprs.h ../global/resource.h
scnprs.o: y.tab.h
y.tab.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
y.tab.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
y.tab.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
y.tab.o: /usr/include/stdlib.h /usr/include/memory.h /usr/include/sys/types.h
y.tab.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
y.tab.o: ../../inc/dbug.h ../tree/tree.h ../global/types.h
y.tab.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
y.tab.o: /usr/include/string.h ../global/globals.h ../global/internal_lib.h
y.tab.o: ../typecheck/typecheck.h ../tree/DupTree.h ../tree/LookUpTable.h
y.tab.o: ../global/my_debug.h ../tree/free.h /usr/include/malloc.h
y.tab.o: ../modules/readsib.h ../global/resource.h
