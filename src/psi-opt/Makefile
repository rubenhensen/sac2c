#
# $Log$
# Revision 2.9  2000/01/17 18:37:49  cg
# Handling of target platform unified: One of the following
# is always defined:
# SAC_FOR_SOLARIS_SPARC, SAC_FOR_LINUX_X86, OR SAC_FOR_OSF_ALPHA.
#
# Revision 2.8  1999/12/13 11:27:35  dkr
# *** empty log message ***
#
# Revision 2.7  1999/06/01 15:17:07  dkr
# make deps done
#
# Revision 2.6  1999/05/12 15:13:27  cg
# make deps done
#
# Revision 2.5  1999/05/10 10:48:37  bs
# added compilation of wl_access_analyze.c
#
# Revision 2.4  1999/03/11 18:18:25  dkr
# make deps done
#
# Revision 2.3  1999/02/26 14:24:48  dkr
# include path changed
#
# Revision 2.2  1999/02/26 14:17:43  dkr
# make deps done
#
# Revision 2.1  1999/02/23 12:43:10  sacbase
# new release made
#
# Revision 1.21  1999/02/09 20:04:24  dkr
# make deps done
#
# Revision 1.20  1999/01/15 15:29:01  cg
# added compilation of file tile_size_inference.c
#
# Revision 1.19  1998/03/04 00:11:05  dkr
# 'clean' now deletes the subdir .sb, too
#
# Revision 1.17  1997/11/20 15:02:11  dkr
# make deps done
#
# Revision 1.16  1997/11/12 15:59:49  sbs
# CC+CCFLAGS handling changed.
# Now, it suffices to preset CC=cc if the SUN-compiler isdesired
# Otherwise, gcc is set with -Wall and -ansi
#
# Revision 1.15  1997/11/11 16:49:32  sbs
# make deps done
#
# Revision 1.11  1996/07/18  12:23:42  sbs
# ArrayElimination.o included in clean-line
#
# Revision 1.10  1996/02/16  09:46:57  sbs
# make deps done
#
# Revision 1.8  1996/01/17  14:19:52  asi
# added ../modules to INCS
#
# Revision 1.7  1995/12/15  10:41:13  cg
# make deps done
#
# Revision 1.4  1995/07/24  11:54:09  asi
# added array elimination
#
# Revision 1.3  1995/07/14  08:33:01  sbs
# make deps done
#
# Revision 1.1  1995/06/02  10:09:27  sbs
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


INCS=-I../global -I../../inc -I../optimize -I../compile -I../typecheck -I../modules -I../print

all: index.o psi-opt.o ArrayElimination.o wl_access_analyze.o tile_size_inference.o WithloopFolding.o WLT.o WLI.o WLF.o

index.o: index.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o index.o -c index.c $(INCS)

psi-opt.o: psi-opt.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o psi-opt.o -c psi-opt.c $(INCS)

ArrayElimination.o: ArrayElimination.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o ArrayElimination.o -c ArrayElimination.c $(INCS)

wl_access_analyze.o: wl_access_analyze.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o wl_access_analyze.o -c wl_access_analyze.c $(INCS)

tile_size_inference.o: tile_size_inference.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o tile_size_inference.o -c tile_size_inference.c $(INCS)

WithloopFolding.o: WithloopFolding.c WithloopFolding.h
	$(CC) $(CCFLAGS) $(CFLAGS) -o WithloopFolding.o -c WithloopFolding.c $(INCS)

WLT.o: WLT.c WLT.h
	$(CC) $(CCFLAGS) $(CFLAGS) -o WLT.o -c WLT.c $(INCS)

WLI.o: WLI.c WLI.h
	$(CC) $(CCFLAGS) $(CFLAGS) -o WLI.o -c WLI.c $(INCS)

WLF.o: WLF.c WLF.h
	$(CC) $(CCFLAGS) $(CFLAGS) -o WLF.o -c WLF.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak *~
	$(RM) -r .sb SunWS_cache


# DO NOT DELETE THIS LINE -- make depend depends on it.

ArrayElimination.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
ArrayElimination.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
ArrayElimination.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
ArrayElimination.o: /usr/include/stdlib.h /usr/include/string.h
ArrayElimination.o: ../global/globals.h ../global/types.h ../global/Error.h
ArrayElimination.o: ../../inc/dbug.h ../global/my_debug.h
ArrayElimination.o: ../global/traverse.h ../typecheck/typecheck.h
ArrayElimination.o: ../global/free.h /usr/include/malloc.h
ArrayElimination.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
ArrayElimination.o: /usr/include/sys/int_types.h ../global/internal_lib.h
ArrayElimination.o: ../optimize/DupTree.h ../global/tree.h
ArrayElimination.o: ../global/tree_basic.h ../global/tree_compound.h
ArrayElimination.o: ../optimize/optimize.h ../optimize/generatemasks.h
ArrayElimination.o: ../optimize/Inline.h ../optimize/LoopInvariantRemoval.h
ArrayElimination.o: ArrayElimination.h
WLF.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
WLF.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
WLF.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
WLF.o: /usr/include/stdlib.h /usr/include/limits.h ../global/tree.h
WLF.o: ../global/types.h ../global/tree_basic.h ../global/tree_compound.h
WLF.o: ../global/Error.h /usr/include/string.h ../global/globals.h
WLF.o: ../global/internal_lib.h ../global/free.h ../../inc/dbug.h
WLF.o: ../global/my_debug.h /usr/include/malloc.h /usr/include/sys/types.h
WLF.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
WLF.o: ../print/print.h ../optimize/DupTree.h ../global/traverse.h
WLF.o: ../optimize/optimize.h ../optimize/generatemasks.h
WLF.o: ../optimize/ConstantFolding.h WithloopFolding.h WLF.h
WLI.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
WLI.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
WLI.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
WLI.o: /usr/include/stdlib.h ../global/tree.h ../global/types.h
WLI.o: ../global/tree_basic.h ../global/tree_compound.h ../global/Error.h
WLI.o: /usr/include/string.h ../global/globals.h ../global/internal_lib.h
WLI.o: ../global/free.h ../../inc/dbug.h ../global/my_debug.h
WLI.o: /usr/include/malloc.h /usr/include/sys/types.h
WLI.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
WLI.o: ../print/print.h ../global/traverse.h ../optimize/optimize.h
WLI.o: ../optimize/generatemasks.h ../optimize/ConstantFolding.h
WLI.o: WithloopFolding.h WLI.h
WLT.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
WLT.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
WLT.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
WLT.o: /usr/include/stdlib.h ../global/tree.h ../global/types.h
WLT.o: ../global/tree_basic.h ../global/tree_compound.h ../global/Error.h
WLT.o: /usr/include/string.h ../global/globals.h ../global/internal_lib.h
WLT.o: ../global/free.h ../../inc/dbug.h ../global/my_debug.h
WLT.o: /usr/include/malloc.h /usr/include/sys/types.h
WLT.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
WLT.o: ../print/print.h ../optimize/DupTree.h ../global/traverse.h
WLT.o: ../optimize/optimize.h ../optimize/generatemasks.h WithloopFolding.h
WLT.o: WLT.h
WithloopFolding.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
WithloopFolding.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
WithloopFolding.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
WithloopFolding.o: /usr/include/stdlib.h ../global/tree.h ../global/types.h
WithloopFolding.o: ../global/tree_basic.h ../global/tree_compound.h
WithloopFolding.o: ../global/Error.h /usr/include/string.h
WithloopFolding.o: ../global/globals.h ../global/internal_lib.h
WithloopFolding.o: ../global/free.h ../../inc/dbug.h ../global/my_debug.h
WithloopFolding.o: /usr/include/malloc.h /usr/include/sys/types.h
WithloopFolding.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
WithloopFolding.o: ../print/print.h ../optimize/DupTree.h
WithloopFolding.o: ../global/traverse.h ../optimize/optimize.h
WithloopFolding.o: ../optimize/generatemasks.h ../optimize/Inline.h
WithloopFolding.o: ../typecheck/typecheck.h ../optimize/ConstantFolding.h
WithloopFolding.o: WithloopFolding.h WLT.h WLI.h WLF.h
index.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
index.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
index.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
index.o: /usr/include/string.h ../../inc/dbug.h ../global/tree.h
index.o: ../global/types.h ../global/tree_basic.h ../global/tree_compound.h
index.o: ../global/Error.h /usr/include/stdlib.h ../global/globals.h
index.o: ../global/internal_lib.h ../global/traverse.h ../optimize/DupTree.h
index.o: index.h psi-opt.h ../global/free.h ../global/my_debug.h
index.o: /usr/include/malloc.h /usr/include/sys/types.h
index.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
index.o: ../global/access_macros.h ../compile/compile.h
psi-opt.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
psi-opt.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
psi-opt.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
psi-opt.o: /usr/include/stdlib.h ../global/tree.h ../global/types.h
psi-opt.o: ../global/tree_basic.h ../global/tree_compound.h ../global/Error.h
psi-opt.o: /usr/include/string.h ../global/globals.h ../global/internal_lib.h
psi-opt.o: ../../inc/dbug.h ../global/traverse.h psi-opt.h
tile_size_inference.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
tile_size_inference.o: /usr/include/sys/isa_defs.h ../../inc/dbug.h
tile_size_inference.o: /usr/include/stdio.h /usr/include/sys/va_list.h
tile_size_inference.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
tile_size_inference.o: ../global/types.h ../global/tree_basic.h
tile_size_inference.o: ../global/tree_compound.h ../global/Error.h
tile_size_inference.o: /usr/include/string.h ../global/globals.h
tile_size_inference.o: ../global/traverse.h ../global/free.h
tile_size_inference.o: ../global/my_debug.h /usr/include/malloc.h
tile_size_inference.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
tile_size_inference.o: /usr/include/sys/int_types.h ../global/internal_lib.h
tile_size_inference.o: ../print/print.h ../global/tree.h
tile_size_inference.o: tile_size_inference.h
wl_access_analyze.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
wl_access_analyze.o: /usr/include/sys/isa_defs.h ../../inc/dbug.h
wl_access_analyze.o: /usr/include/stdio.h /usr/include/sys/va_list.h
wl_access_analyze.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
wl_access_analyze.o: ../global/types.h ../global/tree_basic.h
wl_access_analyze.o: ../global/tree_compound.h ../global/Error.h
wl_access_analyze.o: /usr/include/string.h ../global/globals.h
wl_access_analyze.o: ../global/traverse.h ../global/free.h
wl_access_analyze.o: ../global/my_debug.h /usr/include/malloc.h
wl_access_analyze.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
wl_access_analyze.o: /usr/include/sys/int_types.h ../global/internal_lib.h
wl_access_analyze.o: ../print/print.h ../global/tree.h wl_access_analyze.h
