#
#
# $Log$
# Revision 3.5  2001/03/22 20:19:58  dkr
# make deps done
#
# Revision 3.4  2001/01/17 10:13:02  dkr
# make deps done
#
# Revision 3.3  2000/12/15 15:08:42  sbs
# YYDEBUG disabled for non-solaris machines
# => avoids warning on alpha.
#
# Revision 3.2  2000/12/06 20:09:01  dkr
# make deps done
#
# Revision 3.1  2000/11/20 17:59:47  sacbase
# new release made
#
#  ... [eliminated] ...
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

ifeq ($(OS),SOLARIS_SPARC)
  YYFLAGS   := -DYYDEBUG
else
  YYFLAGS   :=
endif

all: y.tab.o lex.yy.o scnprs.o


y.tab.o: y.tab.c
	$(CC) $(CCL_FLAGS) $(CCFLAGS) $(CFLAGS) $(YYFLAGS) -o y.tab.o -c y.tab.c $(INCS)

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
lex.yy.o: /usr/include/stdlib.h ../global/types.h ../tree/tree_basic.h
lex.yy.o: ../tree/tree_compound.h ../global/Error.h /usr/include/string.h
lex.yy.o: ../global/globals.h ../global/internal_lib.h ../tree/free.h
lex.yy.o: ../../inc/dbug.h ../global/my_debug.h ../global/resource.h y.tab.h
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
y.tab.o: ../tree/tree.h ../global/types.h ../tree/tree_basic.h
y.tab.o: ../tree/tree_compound.h ../global/Error.h /usr/include/string.h
y.tab.o: ../global/globals.h ../global/internal_lib.h ../../inc/dbug.h
y.tab.o: ../typecheck/typecheck.h ../tree/DupTree.h ../tree/LookUpTable.h
y.tab.o: ../global/my_debug.h ../tree/free.h ../modules/readsib.h
y.tab.o: ../global/resource.h
