#
#
# $Log$
# Revision 2.26  2000/11/02 14:24:14  dkr
# header compressed
#
# Revision 2.22  2000/07/11 15:51:50  dkr
# psi-opt.o removed
#
# Revision 2.19  2000/06/08 11:06:47  mab
# added pad_info.o
#
# Revision 2.18  2000/05/31 14:03:20  mab
# lines for pad*.o completed
#
# Revision 2.15  2000/05/26 14:23:59  sbs
# lines for pad*.o added
# pad.o added as target
#
# Revision 2.11  2000/01/21 12:03:22  dkr
# directory tree added
#
# Revision 2.9  2000/01/17 18:37:49  cg
# Handling of target platform unified: One of the following
# is always defined:
# SAC_FOR_SOLARIS_SPARC, SAC_FOR_LINUX_X86, OR SAC_FOR_OSF_ALPHA.
#
# Revision 2.5  1999/05/10 10:48:37  bs
# added compilation of wl_access_analyze.c
#
# Revision 2.3  1999/02/26 14:24:48  dkr
# include path changed
#
# Revision 1.20  1999/01/15 15:29:01  cg
# added compilation of file tile_size_inference.c
#
# Revision 1.19  1998/03/04 00:11:05  dkr
# 'clean' now deletes the subdir .sb, too
#
# Revision 1.16  1997/11/12 15:59:49  sbs
# CC+CCFLAGS handling changed.
# Now, it suffices to preset CC=cc if the SUN-compiler isdesired
# Otherwise, gcc is set with -Wall and -ansi
#
# Revision 1.11  1996/07/18  12:23:42  sbs
# ArrayElimination.o included in clean-line
#
# Revision 1.8  1996/01/17  14:19:52  asi
# added ../modules to INCS
#
# Revision 1.4  1995/07/24  11:54:09  asi
# added array elimination
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


INCS=-I../global -I../../inc -I../tree -I../compile -I../typecheck -I../modules \
     -I../print -I../optimize

all: index.o ArrayElimination.o wl_access_analyze.o tile_size_inference.o \
     WithloopFolding.o WLT.o WLI.o WLF.o \
     pad.o pad_info.o pad_collect.o pad_infer.o pad_transform.o

%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o $@ -c $< $(INCS)

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
ArrayElimination.o: ../../inc/dbug.h ../global/my_debug.h ../tree/traverse.h
ArrayElimination.o: ../typecheck/typecheck.h ../tree/free.h
ArrayElimination.o: /usr/include/malloc.h /usr/include/sys/types.h
ArrayElimination.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
ArrayElimination.o: ../global/internal_lib.h ../tree/DupTree.h
ArrayElimination.o: ../tree/LookUpTable.h ../tree/tree.h ../tree/tree_basic.h
ArrayElimination.o: ../tree/tree_compound.h ../optimize/optimize.h
ArrayElimination.o: ../optimize/generatemasks.h
ArrayElimination.o: ../optimize/LoopInvariantRemoval.h ArrayElimination.h
WLF.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
WLF.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
WLF.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
WLF.o: /usr/include/stdlib.h /usr/include/limits.h ../tree/tree.h
WLF.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
WLF.o: ../global/Error.h /usr/include/string.h ../global/globals.h
WLF.o: ../global/internal_lib.h ../tree/free.h ../../inc/dbug.h
WLF.o: ../global/my_debug.h /usr/include/malloc.h /usr/include/sys/types.h
WLF.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
WLF.o: ../print/print.h ../tree/DupTree.h ../tree/LookUpTable.h
WLF.o: ../tree/traverse.h ../optimize/optimize.h ../optimize/generatemasks.h
WLF.o: ../optimize/ConstantFolding.h WithloopFolding.h WLF.h
WLI.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
WLI.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
WLI.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
WLI.o: /usr/include/stdlib.h ../tree/tree.h ../global/types.h
WLI.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
WLI.o: /usr/include/string.h ../global/globals.h ../global/internal_lib.h
WLI.o: ../tree/free.h ../../inc/dbug.h ../global/my_debug.h
WLI.o: /usr/include/malloc.h /usr/include/sys/types.h
WLI.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
WLI.o: ../print/print.h ../tree/traverse.h ../optimize/optimize.h
WLI.o: ../optimize/generatemasks.h ../optimize/ConstantFolding.h
WLI.o: WithloopFolding.h WLI.h
WLT.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
WLT.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
WLT.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
WLT.o: /usr/include/stdlib.h ../tree/tree.h ../global/types.h
WLT.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
WLT.o: /usr/include/string.h ../global/globals.h ../global/internal_lib.h
WLT.o: ../tree/free.h ../../inc/dbug.h ../global/my_debug.h
WLT.o: /usr/include/malloc.h /usr/include/sys/types.h
WLT.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
WLT.o: ../print/print.h ../tree/DupTree.h ../tree/LookUpTable.h
WLT.o: ../tree/traverse.h ../optimize/optimize.h ../optimize/generatemasks.h
WLT.o: WithloopFolding.h WLT.h
WithloopFolding.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
WithloopFolding.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
WithloopFolding.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
WithloopFolding.o: /usr/include/stdlib.h ../tree/tree.h ../global/types.h
WithloopFolding.o: ../tree/tree_basic.h ../tree/tree_compound.h
WithloopFolding.o: ../global/Error.h /usr/include/string.h
WithloopFolding.o: ../global/globals.h ../global/internal_lib.h
WithloopFolding.o: ../tree/free.h ../../inc/dbug.h ../global/my_debug.h
WithloopFolding.o: /usr/include/malloc.h /usr/include/sys/types.h
WithloopFolding.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
WithloopFolding.o: ../print/print.h ../tree/DupTree.h ../tree/LookUpTable.h
WithloopFolding.o: ../tree/traverse.h ../optimize/optimize.h
WithloopFolding.o: ../optimize/generatemasks.h ../optimize/ConstantFolding.h
WithloopFolding.o: WithloopFolding.h WLT.h WLI.h WLF.h
index.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
index.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
index.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
index.o: /usr/include/string.h ../../inc/dbug.h ../tree/tree.h
index.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
index.o: ../global/Error.h /usr/include/stdlib.h ../global/globals.h
index.o: ../global/internal_lib.h ../print/print.h ../tree/traverse.h
index.o: ../tree/DupTree.h ../tree/LookUpTable.h index.h ../tree/free.h
index.o: ../global/my_debug.h /usr/include/malloc.h /usr/include/sys/types.h
index.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
index.o: ../tree/access_macros.h ../compile/compile.h
pad.o: /usr/include/stdarg.h /usr/include/varargs.h
pad.o: /usr/include/sys/va_list.h ../../inc/dbug.h /usr/include/stdio.h
pad.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
pad.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h ../global/types.h
pad.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
pad.o: /usr/include/string.h /usr/include/stdlib.h ../global/globals.h
pad.o: ../tree/traverse.h ../tree/free.h ../global/my_debug.h
pad.o: /usr/include/malloc.h /usr/include/sys/types.h
pad.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
pad.o: ../global/internal_lib.h ../modules/filemgr.h ../global/resource.h
pad.o: pad.h pad_info.h pad_collect.h pad_infer.h pad_transform.h
pad_collect.o: ../../inc/dbug.h /usr/include/stdio.h
pad_collect.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
pad_collect.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
pad_collect.o: /usr/include/stdio_impl.h ../global/types.h
pad_collect.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
pad_collect.o: /usr/include/string.h /usr/include/stdlib.h
pad_collect.o: ../global/globals.h ../tree/traverse.h ../tree/free.h
pad_collect.o: ../global/my_debug.h /usr/include/malloc.h
pad_collect.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
pad_collect.o: /usr/include/sys/int_types.h ../global/internal_lib.h
pad_collect.o: ../tree/DupTree.h ../tree/LookUpTable.h pad_info.h
pad_collect.o: pad_collect.h ../tree/prf_node_info.mac
pad_infer.o: ../../inc/dbug.h /usr/include/stdio.h
pad_infer.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
pad_infer.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
pad_infer.o: /usr/include/stdio_impl.h ../global/types.h ../tree/tree_basic.h
pad_infer.o: ../tree/tree_compound.h ../global/Error.h /usr/include/string.h
pad_infer.o: /usr/include/stdlib.h ../global/globals.h ../tree/free.h
pad_infer.o: ../global/my_debug.h /usr/include/malloc.h
pad_infer.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
pad_infer.o: /usr/include/sys/int_types.h ../global/internal_lib.h
pad_infer.o: ../tree/DupTree.h ../tree/LookUpTable.h ../global/resource.h
pad_infer.o: ../print/convert.h pad_infer.h pad_info.h pad.h
pad_infer.o: ../tree/type_info.mac
pad_info.o: ../../inc/dbug.h /usr/include/stdio.h
pad_info.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
pad_info.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
pad_info.o: /usr/include/stdio_impl.h ../global/types.h ../tree/tree_basic.h
pad_info.o: ../tree/tree_compound.h ../global/Error.h /usr/include/string.h
pad_info.o: /usr/include/stdlib.h ../global/globals.h ../tree/traverse.h
pad_info.o: ../tree/free.h ../global/my_debug.h /usr/include/malloc.h
pad_info.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
pad_info.o: /usr/include/sys/int_types.h ../global/internal_lib.h
pad_info.o: ../tree/DupTree.h ../tree/LookUpTable.h ../optimize/optimize.h
pad_info.o: ../print/convert.h pad_info.h pad.h
pad_transform.o: ../../inc/dbug.h /usr/include/stdio.h
pad_transform.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
pad_transform.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
pad_transform.o: /usr/include/stdio_impl.h ../global/types.h
pad_transform.o: ../tree/tree_basic.h ../tree/tree_compound.h
pad_transform.o: ../global/Error.h /usr/include/string.h
pad_transform.o: /usr/include/stdlib.h ../global/globals.h ../tree/traverse.h
pad_transform.o: ../tree/free.h ../global/my_debug.h /usr/include/malloc.h
pad_transform.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
pad_transform.o: /usr/include/sys/int_types.h ../global/internal_lib.h
pad_transform.o: ../tree/DupTree.h ../tree/LookUpTable.h pad_info.h
pad_transform.o: pad_transform.h ../tree/prf_node_info.mac
tile_size_inference.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
tile_size_inference.o: /usr/include/sys/isa_defs.h /usr/include/limits.h
tile_size_inference.o: /usr/include/string.h ../../inc/dbug.h
tile_size_inference.o: /usr/include/stdio.h /usr/include/sys/va_list.h
tile_size_inference.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
tile_size_inference.o: ../global/types.h ../tree/tree_basic.h
tile_size_inference.o: ../tree/tree_compound.h ../global/Error.h
tile_size_inference.o: ../global/globals.h ../tree/traverse.h ../tree/free.h
tile_size_inference.o: ../global/my_debug.h /usr/include/malloc.h
tile_size_inference.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
tile_size_inference.o: /usr/include/sys/int_types.h ../global/internal_lib.h
tile_size_inference.o: ../global/resource.h ../tree/DupTree.h
tile_size_inference.o: ../tree/LookUpTable.h ../print/print.h ../tree/tree.h
tile_size_inference.o: tile_size_inference.h
wl_access_analyze.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
wl_access_analyze.o: /usr/include/sys/isa_defs.h ../../inc/dbug.h
wl_access_analyze.o: /usr/include/stdio.h /usr/include/sys/va_list.h
wl_access_analyze.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
wl_access_analyze.o: ../global/types.h ../tree/tree_basic.h
wl_access_analyze.o: ../tree/tree_compound.h ../global/Error.h
wl_access_analyze.o: /usr/include/string.h ../global/globals.h
wl_access_analyze.o: ../tree/traverse.h ../tree/free.h ../global/my_debug.h
wl_access_analyze.o: /usr/include/malloc.h /usr/include/sys/types.h
wl_access_analyze.o: /usr/include/sys/machtypes.h
wl_access_analyze.o: /usr/include/sys/int_types.h ../global/internal_lib.h
wl_access_analyze.o: ../print/print.h ../tree/tree.h wl_access_analyze.h
