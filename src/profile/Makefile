#
#
# $Log$
# Revision 1.3  2001/11/13 20:32:53  dkr
# make deps done
#
# Revision 1.2  2001/03/22 20:20:23  dkr
# make deps done
#
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

annotate_fun_calls.o: ../global/types.h ../tree/tree_basic.h
annotate_fun_calls.o: ../tree/tree_compound.h ../global/Error.h
annotate_fun_calls.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
annotate_fun_calls.o: /usr/include/sys/feature_tests.h
annotate_fun_calls.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
annotate_fun_calls.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
annotate_fun_calls.o: /usr/include/string.h /usr/include/iso/string_iso.h
annotate_fun_calls.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
annotate_fun_calls.o: ../global/globals.h ../global/internal_lib.h
annotate_fun_calls.o: ../../inc/dbug.h ../tree/traverse.h ../tree/free.h
annotate_fun_calls.o: ../global/my_debug.h ../print/convert.h
