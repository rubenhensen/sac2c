#
# $Log$
# Revision 2.23  2000/07/11 15:56:28  dkr
# make deps done
#
# Revision 2.22  2000/07/11 15:51:50  dkr
# psi-opt.o removed
#
# Revision 2.21  2000/07/05 14:23:47  dkr
# make deps done
#
# Revision 2.20  2000/06/13 13:46:21  dkr
# deps done
#
# Revision 2.19  2000/06/08 11:06:47  mab
# added pad_info.o
#
# Revision 2.18  2000/05/31 14:03:20  mab
# lines for pad*.o completed
#
# Revision 2.17  2000/05/29 14:36:41  dkr
# make deps done
#
# Revision 2.16  2000/05/29 14:35:18  dkr
# make deps done
#
# Revision 2.15  2000/05/26 14:23:59  sbs
# lines for pad*.o added
# pad.o added as target
#
# Revision 2.14  2000/03/31 09:33:45  jhs
# make deps done
#
# Revision 2.13  2000/03/01 14:52:38  dkr
# make deps done
#
# Revision 2.12  2000/01/21 17:02:09  dkr
# make deps done
#
# Revision 2.11  2000/01/21 12:03:22  dkr
# directory tree added
#
# Revision 2.10  2000/01/17 18:41:06  cg
# make deps done
#
# Revision 2.9  2000/01/17 18:37:49  cg
# Handling of target platform unified: One of the following
# is always defined:
# SAC_FOR_SOLARIS_SPARC, SAC_FOR_LINUX_X86, OR SAC_FOR_OSF_ALPHA.
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
ArrayElimination.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
ArrayElimination.o: /usr/include/string.h ../global/globals.h
ArrayElimination.o: ../global/types.h ../global/Error.h ../../inc/dbug.h
ArrayElimination.o: ../global/my_debug.h ../tree/traverse.h
ArrayElimination.o: ../typecheck/typecheck.h ../tree/free.h
ArrayElimination.o: /usr/include/malloc.h ../global/internal_lib.h
ArrayElimination.o: ../tree/DupTree.h ../tree/LookUpTable.h ../tree/tree.h
ArrayElimination.o: ../tree/tree_basic.h ../tree/tree_compound.h
ArrayElimination.o: ../optimize/optimize.h ../optimize/generatemasks.h
ArrayElimination.o: ../optimize/Inline.h ../optimize/LoopInvariantRemoval.h
ArrayElimination.o: ArrayElimination.h
WLF.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
WLF.o: /usr/include/sys/va_list.h /usr/include/stdlib.h /usr/include/limits.h
WLF.o: /usr/include/sys/isa_defs.h ../tree/tree.h ../global/types.h
WLF.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
WLF.o: /usr/include/string.h ../global/globals.h ../global/internal_lib.h
WLF.o: ../tree/free.h ../../inc/dbug.h ../global/my_debug.h
WLF.o: /usr/include/malloc.h ../print/print.h ../tree/DupTree.h
WLF.o: ../tree/LookUpTable.h ../tree/traverse.h ../optimize/optimize.h
WLF.o: ../optimize/generatemasks.h ../optimize/ConstantFolding.h
WLF.o: WithloopFolding.h WLF.h
WLI.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
WLI.o: /usr/include/sys/va_list.h /usr/include/stdlib.h ../tree/tree.h
WLI.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
WLI.o: ../global/Error.h /usr/include/string.h ../global/globals.h
WLI.o: ../global/internal_lib.h ../tree/free.h ../../inc/dbug.h
WLI.o: ../global/my_debug.h /usr/include/malloc.h ../print/print.h
WLI.o: ../tree/traverse.h ../optimize/optimize.h ../optimize/generatemasks.h
WLI.o: ../optimize/ConstantFolding.h WithloopFolding.h WLI.h
WLT.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
WLT.o: /usr/include/sys/va_list.h /usr/include/stdlib.h ../tree/tree.h
WLT.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
WLT.o: ../global/Error.h /usr/include/string.h ../global/globals.h
WLT.o: ../global/internal_lib.h ../tree/free.h ../../inc/dbug.h
WLT.o: ../global/my_debug.h /usr/include/malloc.h ../print/print.h
WLT.o: ../tree/DupTree.h ../tree/LookUpTable.h ../tree/traverse.h
WLT.o: ../optimize/optimize.h ../optimize/generatemasks.h WithloopFolding.h
WLT.o: WLT.h
WithloopFolding.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
WithloopFolding.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
WithloopFolding.o: ../tree/tree.h ../global/types.h ../tree/tree_basic.h
WithloopFolding.o: ../tree/tree_compound.h ../global/Error.h
WithloopFolding.o: /usr/include/string.h ../global/globals.h
WithloopFolding.o: ../global/internal_lib.h ../tree/free.h ../../inc/dbug.h
WithloopFolding.o: ../global/my_debug.h /usr/include/malloc.h
WithloopFolding.o: ../print/print.h ../tree/DupTree.h ../tree/LookUpTable.h
WithloopFolding.o: ../tree/traverse.h ../optimize/optimize.h
WithloopFolding.o: ../optimize/generatemasks.h ../optimize/Inline.h
WithloopFolding.o: ../typecheck/typecheck.h ../optimize/ConstantFolding.h
WithloopFolding.o: WithloopFolding.h WLT.h WLI.h WLF.h
index.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
index.o: /usr/include/sys/va_list.h /usr/include/string.h ../../inc/dbug.h
index.o: ../tree/tree.h ../global/types.h ../tree/tree_basic.h
index.o: ../tree/tree_compound.h ../global/Error.h /usr/include/stdlib.h
index.o: ../global/globals.h ../global/internal_lib.h ../print/print.h
index.o: ../tree/traverse.h ../tree/DupTree.h ../tree/LookUpTable.h index.h
index.o: ../tree/free.h ../global/my_debug.h /usr/include/malloc.h
index.o: ../tree/access_macros.h ../compile/compile.h
pad.o: ../../inc/dbug.h /usr/include/stdio.h /usr/include/sys/feature_tests.h
pad.o: /usr/include/sys/va_list.h ../global/types.h ../tree/tree_basic.h
pad.o: ../tree/tree_compound.h ../global/Error.h /usr/include/string.h
pad.o: /usr/include/stdlib.h ../global/globals.h ../tree/traverse.h
pad.o: ../tree/free.h ../global/my_debug.h /usr/include/malloc.h
pad.o: ../global/internal_lib.h pad.h pad_info.h pad_collect.h pad_infer.h
pad.o: pad_transform.h
pad_collect.o: ../../inc/dbug.h /usr/include/stdio.h
pad_collect.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
pad_collect.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
pad_collect.o: ../global/Error.h /usr/include/string.h /usr/include/stdlib.h
pad_collect.o: ../global/globals.h ../tree/traverse.h ../tree/free.h
pad_collect.o: ../global/my_debug.h /usr/include/malloc.h
pad_collect.o: ../global/internal_lib.h pad_info.h pad_collect.h
pad_infer.o: pad_infer.h
pad_info.o: ../../inc/dbug.h /usr/include/stdio.h
pad_info.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
pad_info.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
pad_info.o: ../global/Error.h /usr/include/string.h /usr/include/stdlib.h
pad_info.o: ../global/globals.h ../tree/traverse.h ../tree/free.h
pad_info.o: ../global/my_debug.h /usr/include/malloc.h
pad_info.o: ../global/internal_lib.h ../tree/DupTree.h ../tree/LookUpTable.h
pad_info.o: pad_info.h
pad_transform.o: ../../inc/dbug.h /usr/include/stdio.h
pad_transform.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
pad_transform.o: ../global/types.h ../tree/tree_basic.h
pad_transform.o: ../tree/tree_compound.h ../global/Error.h
pad_transform.o: /usr/include/string.h /usr/include/stdlib.h
pad_transform.o: ../global/globals.h ../tree/traverse.h ../tree/free.h
pad_transform.o: ../global/my_debug.h /usr/include/malloc.h
pad_transform.o: ../global/internal_lib.h ../tree/DupTree.h
pad_transform.o: ../tree/LookUpTable.h pad_info.h pad_transform.h
pad_transform.o: ../tree/prf_node_info.mac
tile_size_inference.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
tile_size_inference.o: /usr/include/limits.h /usr/include/sys/isa_defs.h
tile_size_inference.o: /usr/include/string.h ../../inc/dbug.h
tile_size_inference.o: /usr/include/stdio.h /usr/include/sys/va_list.h
tile_size_inference.o: ../global/types.h ../tree/tree_basic.h
tile_size_inference.o: ../tree/tree_compound.h ../global/Error.h
tile_size_inference.o: ../global/globals.h ../tree/traverse.h ../tree/free.h
tile_size_inference.o: ../global/my_debug.h /usr/include/malloc.h
tile_size_inference.o: ../global/internal_lib.h ../global/resource.h
tile_size_inference.o: ../tree/DupTree.h ../tree/LookUpTable.h
tile_size_inference.o: ../print/print.h ../tree/tree.h tile_size_inference.h
tile_size_inference.o: ../tree/type_info.mac
wl_access_analyze.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
wl_access_analyze.o: ../../inc/dbug.h /usr/include/stdio.h
wl_access_analyze.o: /usr/include/sys/va_list.h ../global/types.h
wl_access_analyze.o: ../tree/tree_basic.h ../tree/tree_compound.h
wl_access_analyze.o: ../global/Error.h /usr/include/string.h
wl_access_analyze.o: ../global/globals.h ../tree/traverse.h ../tree/free.h
wl_access_analyze.o: ../global/my_debug.h /usr/include/malloc.h
wl_access_analyze.o: ../global/internal_lib.h ../print/print.h ../tree/tree.h
wl_access_analyze.o: wl_access_analyze.h
