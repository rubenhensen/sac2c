#
#
# $Log$
# Revision 1.1  2001/03/09 11:08:05  sbs
# Initial revision
#
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

RM=rm -f
INCS=-I../../inc -I../tree -I../global -I../print

all: annotate_fun_calls.o



%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o $@ -c $< $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak
	$(RM) -r .sb SunWS_cache

# DO NOT DELETE THIS LINE -- make depend depends on it.

annotate_fun_calls.o: ../../inc/dbug.h /usr/include/stdio.h
annotate_fun_calls.o: /usr/include/sys/feature_tests.h
annotate_fun_calls.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
annotate_fun_calls.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
annotate_fun_calls.o: ../tree/tree.h ../global/types.h ../tree/tree_basic.h
annotate_fun_calls.o: ../tree/tree_compound.h ../global/Error.h
annotate_fun_calls.o: /usr/include/string.h /usr/include/stdlib.h
annotate_fun_calls.o: ../global/globals.h ../global/internal_lib.h
annotate_fun_calls.o: ../tree/traverse.h ../tree/free.h ../global/my_debug.h
annotate_fun_calls.o: ../print/convert.h
