#
# $Log$
# Revision 1.1  2000/01/21 12:46:00  dkr
# Initial revision
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


INCS=-I../global -I../../inc -I../optimize -I../typecheck -I../concurrent

all: DupTree.o DataFlowMask.o


DupTree.o: DupTree.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o DupTree.o -c DupTree.c $(INCS)

DataFlowMask.o: DataFlowMask.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o DataFlowMask.o -c DataFlowMask.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) DupTree.o DataFlowMask.o *.bak
	$(RM) -r .sb SunWS_cache

# DO NOT DELETE THIS LINE -- make depend depends on it.

DataFlowMask.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
DataFlowMask.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
DataFlowMask.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
DataFlowMask.o: /usr/include/string.h ../../inc/dbug.h ../global/types.h
DataFlowMask.o: ../global/tree_basic.h ../global/tree_compound.h
DataFlowMask.o: ../global/Error.h /usr/include/stdlib.h ../global/globals.h
DataFlowMask.o: ../global/internal_lib.h ../global/free.h
DataFlowMask.o: ../global/my_debug.h /usr/include/malloc.h
DataFlowMask.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
DataFlowMask.o: /usr/include/sys/int_types.h
DupTree.o: /usr/include/string.h /usr/include/sys/feature_tests.h
DupTree.o: /usr/include/sys/isa_defs.h ../global/tree.h ../global/types.h
DupTree.o: ../global/tree_basic.h ../global/tree_compound.h ../global/Error.h
DupTree.o: /usr/include/stdio.h /usr/include/sys/va_list.h
DupTree.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
DupTree.o: /usr/include/stdlib.h ../global/globals.h ../global/internal_lib.h
DupTree.o: ../global/traverse.h ../global/free.h ../../inc/dbug.h
DupTree.o: ../global/my_debug.h /usr/include/malloc.h
DupTree.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
DupTree.o: /usr/include/sys/int_types.h ../typecheck/typecheck.h DupTree.h
DupTree.o: DataFlowMask.h ../optimize/optimize.h ../optimize/Inline.h
DupTree.o: ../concurrent/scheduling.h ../optimize/generatemasks.h
