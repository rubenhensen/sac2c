#
#
# $Log$
# Revision 3.13  2001/11/13 20:32:45  dkr
# make deps done
#
# Revision 3.12  2001/07/18 13:08:19  cg
# make deps done
#
# Revision 3.11  2001/05/15 16:38:38  nmw
# SSAWLI.o and SSAWLF.o added
#
# Revision 3.10  2001/05/14 15:58:50  nmw
# SSAWLT.o and SSAWithloopFolding.o added
#
# Revision 3.9  2001/04/26 21:09:48  dkr
# make deps done
#
# Revision 3.8  2001/03/27 11:12:49  nmw
# include of directory ../constants added
#
# Revision 3.7  2001/03/22 20:20:03  dkr
# make deps done
#
# Revision 3.6  2001/03/02 10:36:22  dkr
# /home/dkr/sac/BASE/sac2c/src/psi-opt/RCS/Makefile,v used in touch-rules
#
# Revision 3.5  2001/02/06 01:52:44  dkr
# make deps done
#
# Revision 3.4  2001/02/02 10:39:04  dkr
# make deps done
#
# Revision 3.3  2001/01/17 10:13:06  dkr
# make deps done
#
# Revision 3.2  2000/12/06 20:09:05  dkr
# make deps done
#
# Revision 3.1  2000/11/20 18:01:38  sacbase
# new release made
#
# Revision 2.27  2000/11/02 14:46:31  dkr
# explicit .mac dependencies added
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
     -I../print -I../optimize -I../constants
all: index.o ArrayElimination.o wl_access_analyze.o tile_size_inference.o \
     WithloopFolding.o WLT.o WLI.o WLF.o \
     SSAWithloopFolding.o SSAWLT.o SSAWLI.o SSAWLF.o \
     pad.o pad_info.o pad_collect.o pad_infer.o pad_transform.o


#
# the *.[ch] objects are to fool makedepend which does not
# recognize the dependency between *.[ch] and the .mac files!
#
pad_collect.c: ../tree/prf_node_info.mac
	touch $@

pad_infer.c: ../tree/type_info.mac
	touch $@

pad_transform.c: ../tree/prf_node_info.mac
	touch $@


%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o $@ -c $< $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak *~
	$(RM) -r .sb SunWS_cache

# DO NOT DELETE THIS LINE -- make depend depends on it.

ArrayElimination.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
ArrayElimination.o: /usr/include/sys/feature_tests.h
ArrayElimination.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
ArrayElimination.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
ArrayElimination.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
ArrayElimination.o: /usr/include/string.h /usr/include/iso/string_iso.h
ArrayElimination.o: ../global/types.h ../tree/tree_basic.h
ArrayElimination.o: ../tree/tree_compound.h ../global/Error.h
ArrayElimination.o: ../global/globals.h ../global/internal_lib.h
ArrayElimination.o: ../../inc/dbug.h ../global/my_debug.h ../tree/traverse.h
ArrayElimination.o: ../tree/free.h ../tree/DupTree.h ../tree/LookUpTable.h
ArrayElimination.o: ../optimize/optimize.h ../optimize/generatemasks.h
ArrayElimination.o: ../optimize/LoopInvariantRemoval.h ArrayElimination.h
SSAWLF.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
SSAWLF.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
SSAWLF.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
SSAWLF.o: /usr/include/stdio_impl.h /usr/include/stdlib.h
SSAWLF.o: /usr/include/iso/stdlib_iso.h /usr/include/limits.h
SSAWLF.o: /usr/include/iso/limits_iso.h ../global/types.h
SSAWLF.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
SSAWLF.o: /usr/include/string.h /usr/include/iso/string_iso.h
SSAWLF.o: ../global/globals.h ../global/internal_lib.h ../tree/free.h
SSAWLF.o: ../../inc/dbug.h ../global/my_debug.h ../tree/DupTree.h
SSAWLF.o: ../tree/LookUpTable.h ../tree/traverse.h ../optimize/optimize.h
SSAWLF.o: SSAWithloopFolding.h SSAWLF.h ../constants/shape.h
SSAWLI.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
SSAWLI.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
SSAWLI.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
SSAWLI.o: /usr/include/stdio_impl.h /usr/include/stdlib.h
SSAWLI.o: /usr/include/iso/stdlib_iso.h ../global/types.h
SSAWLI.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
SSAWLI.o: /usr/include/string.h /usr/include/iso/string_iso.h
SSAWLI.o: ../global/globals.h ../global/internal_lib.h ../tree/free.h
SSAWLI.o: ../../inc/dbug.h ../global/my_debug.h ../tree/traverse.h
SSAWLI.o: ../optimize/optimize.h ../optimize/SSAConstantFolding.h
SSAWLI.o: SSAWithloopFolding.h SSAWLI.h
SSAWLT.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
SSAWLT.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
SSAWLT.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
SSAWLT.o: /usr/include/stdio_impl.h /usr/include/stdlib.h
SSAWLT.o: /usr/include/iso/stdlib_iso.h ../global/types.h
SSAWLT.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
SSAWLT.o: /usr/include/string.h /usr/include/iso/string_iso.h
SSAWLT.o: ../global/globals.h ../global/internal_lib.h ../tree/free.h
SSAWLT.o: ../../inc/dbug.h ../global/my_debug.h ../tree/DupTree.h
SSAWLT.o: ../tree/LookUpTable.h ../tree/traverse.h ../optimize/optimize.h
SSAWLT.o: SSAWithloopFolding.h SSAWLT.h
SSAWithloopFolding.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
SSAWithloopFolding.o: /usr/include/sys/feature_tests.h
SSAWithloopFolding.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
SSAWithloopFolding.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
SSAWithloopFolding.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
SSAWithloopFolding.o: ../global/types.h ../tree/tree_basic.h
SSAWithloopFolding.o: ../tree/tree_compound.h ../global/Error.h
SSAWithloopFolding.o: /usr/include/string.h /usr/include/iso/string_iso.h
SSAWithloopFolding.o: ../global/globals.h ../global/internal_lib.h
SSAWithloopFolding.o: ../tree/free.h ../../inc/dbug.h ../global/my_debug.h
SSAWithloopFolding.o: ../print/print.h ../tree/DupTree.h
SSAWithloopFolding.o: ../tree/LookUpTable.h ../tree/traverse.h
SSAWithloopFolding.o: ../optimize/optimize.h ../tree/CheckAvis.h
SSAWithloopFolding.o: ../tree/SSATransform.h SSAWithloopFolding.h SSAWLT.h
SSAWithloopFolding.o: SSAWLI.h
WLF.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
WLF.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
WLF.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
WLF.o: /usr/include/stdio_impl.h /usr/include/stdlib.h
WLF.o: /usr/include/iso/stdlib_iso.h /usr/include/limits.h
WLF.o: /usr/include/iso/limits_iso.h ../global/types.h ../tree/tree_basic.h
WLF.o: ../tree/tree_compound.h ../global/Error.h /usr/include/string.h
WLF.o: /usr/include/iso/string_iso.h ../global/globals.h
WLF.o: ../global/internal_lib.h ../tree/free.h ../../inc/dbug.h
WLF.o: ../global/my_debug.h ../tree/DupTree.h ../tree/LookUpTable.h
WLF.o: ../tree/traverse.h ../optimize/optimize.h ../optimize/generatemasks.h
WLF.o: ../optimize/ConstantFolding.h WithloopFolding.h WLF.h
WLF.o: ../constants/shape.h
WLI.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
WLI.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
WLI.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
WLI.o: /usr/include/stdio_impl.h /usr/include/stdlib.h
WLI.o: /usr/include/iso/stdlib_iso.h ../global/types.h ../tree/tree_basic.h
WLI.o: ../tree/tree_compound.h ../global/Error.h /usr/include/string.h
WLI.o: /usr/include/iso/string_iso.h ../global/globals.h
WLI.o: ../global/internal_lib.h ../tree/free.h ../../inc/dbug.h
WLI.o: ../global/my_debug.h ../tree/traverse.h ../optimize/optimize.h
WLI.o: ../optimize/generatemasks.h ../optimize/ConstantFolding.h
WLI.o: WithloopFolding.h WLI.h
WLT.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
WLT.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
WLT.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
WLT.o: /usr/include/stdio_impl.h /usr/include/stdlib.h
WLT.o: /usr/include/iso/stdlib_iso.h ../global/types.h ../tree/tree_basic.h
WLT.o: ../tree/tree_compound.h ../global/Error.h /usr/include/string.h
WLT.o: /usr/include/iso/string_iso.h ../global/globals.h
WLT.o: ../global/internal_lib.h ../tree/free.h ../../inc/dbug.h
WLT.o: ../global/my_debug.h ../tree/DupTree.h ../tree/LookUpTable.h
WLT.o: ../tree/traverse.h ../optimize/optimize.h ../optimize/generatemasks.h
WLT.o: WithloopFolding.h WLT.h
WithloopFolding.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
WithloopFolding.o: /usr/include/sys/feature_tests.h
WithloopFolding.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
WithloopFolding.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
WithloopFolding.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
WithloopFolding.o: ../global/types.h ../tree/tree_basic.h
WithloopFolding.o: ../tree/tree_compound.h ../global/Error.h
WithloopFolding.o: /usr/include/string.h /usr/include/iso/string_iso.h
WithloopFolding.o: ../global/globals.h ../global/internal_lib.h
WithloopFolding.o: ../tree/free.h ../../inc/dbug.h ../global/my_debug.h
WithloopFolding.o: ../print/print.h ../tree/DupTree.h ../tree/LookUpTable.h
WithloopFolding.o: ../tree/traverse.h ../optimize/optimize.h
WithloopFolding.o: ../optimize/generatemasks.h ../optimize/ConstantFolding.h
WithloopFolding.o: WithloopFolding.h WLT.h WLI.h WLF.h
index.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
index.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
index.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
index.o: /usr/include/stdio_impl.h /usr/include/string.h
index.o: /usr/include/iso/string_iso.h ../global/types.h ../tree/tree_basic.h
index.o: ../tree/tree_compound.h ../global/Error.h /usr/include/stdlib.h
index.o: /usr/include/iso/stdlib_iso.h ../global/globals.h
index.o: ../global/internal_lib.h ../../inc/dbug.h ../print/print.h
index.o: ../tree/traverse.h ../tree/DupTree.h ../tree/LookUpTable.h
index.o: ../tree/free.h ../global/my_debug.h ../print/convert.h index.h
pad.o: /usr/include/stdarg.h /usr/include/varargs.h
pad.o: /usr/include/sys/va_list.h ../../inc/dbug.h /usr/include/stdio.h
pad.o: /usr/include/iso/stdio_iso.h /usr/include/sys/feature_tests.h
pad.o: /usr/include/sys/isa_defs.h /usr/include/stdio_tag.h
pad.o: /usr/include/stdio_impl.h /usr/include/stdlib.h
pad.o: /usr/include/iso/stdlib_iso.h ../global/types.h ../tree/tree_basic.h
pad.o: ../tree/tree_compound.h ../global/Error.h /usr/include/string.h
pad.o: /usr/include/iso/string_iso.h ../global/globals.h ../tree/traverse.h
pad.o: ../tree/free.h ../global/my_debug.h ../global/internal_lib.h
pad.o: ../modules/filemgr.h ../global/resource.h pad.h pad_info.h
pad.o: pad_collect.h pad_infer.h pad_transform.h
pad_collect.o: ../../inc/dbug.h /usr/include/stdio.h
pad_collect.o: /usr/include/iso/stdio_iso.h /usr/include/sys/feature_tests.h
pad_collect.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
pad_collect.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
pad_collect.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
pad_collect.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
pad_collect.o: ../global/Error.h /usr/include/string.h
pad_collect.o: /usr/include/iso/string_iso.h ../global/globals.h
pad_collect.o: ../tree/traverse.h ../tree/free.h ../global/my_debug.h
pad_collect.o: ../global/internal_lib.h ../tree/DupTree.h
pad_collect.o: ../tree/LookUpTable.h pad_info.h pad_collect.h
pad_collect.o: ../tree/prf_node_info.mac
pad_infer.o: ../../inc/dbug.h /usr/include/stdio.h
pad_infer.o: /usr/include/iso/stdio_iso.h /usr/include/sys/feature_tests.h
pad_infer.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
pad_infer.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
pad_infer.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
pad_infer.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
pad_infer.o: ../global/Error.h /usr/include/string.h
pad_infer.o: /usr/include/iso/string_iso.h ../global/globals.h ../tree/free.h
pad_infer.o: ../global/my_debug.h ../global/internal_lib.h ../tree/DupTree.h
pad_infer.o: ../tree/LookUpTable.h ../global/resource.h ../print/convert.h
pad_infer.o: pad_infer.h pad_info.h pad.h ../tree/type_info.mac
pad_info.o: ../../inc/dbug.h /usr/include/stdio.h
pad_info.o: /usr/include/iso/stdio_iso.h /usr/include/sys/feature_tests.h
pad_info.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
pad_info.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
pad_info.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
pad_info.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
pad_info.o: ../global/Error.h /usr/include/string.h
pad_info.o: /usr/include/iso/string_iso.h ../global/globals.h
pad_info.o: ../tree/traverse.h ../tree/free.h ../global/my_debug.h
pad_info.o: ../global/internal_lib.h ../tree/DupTree.h ../tree/LookUpTable.h
pad_info.o: ../optimize/optimize.h ../print/convert.h pad_info.h pad.h
pad_transform.o: ../../inc/dbug.h /usr/include/stdio.h
pad_transform.o: /usr/include/iso/stdio_iso.h
pad_transform.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
pad_transform.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
pad_transform.o: /usr/include/stdio_impl.h /usr/include/stdlib.h
pad_transform.o: /usr/include/iso/stdlib_iso.h ../global/types.h
pad_transform.o: ../tree/tree_basic.h ../tree/tree_compound.h
pad_transform.o: ../global/Error.h /usr/include/string.h
pad_transform.o: /usr/include/iso/string_iso.h ../global/globals.h
pad_transform.o: ../tree/traverse.h ../tree/free.h ../global/my_debug.h
pad_transform.o: ../global/internal_lib.h ../tree/DupTree.h
pad_transform.o: ../tree/LookUpTable.h pad_info.h pad_transform.h
pad_transform.o: ../tree/prf_node_info.mac
tile_size_inference.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
tile_size_inference.o: /usr/include/sys/feature_tests.h
tile_size_inference.o: /usr/include/sys/isa_defs.h /usr/include/limits.h
tile_size_inference.o: /usr/include/iso/limits_iso.h /usr/include/string.h
tile_size_inference.o: /usr/include/iso/string_iso.h ../../inc/dbug.h
tile_size_inference.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
tile_size_inference.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
tile_size_inference.o: /usr/include/stdio_impl.h ../global/types.h
tile_size_inference.o: ../tree/tree_basic.h ../tree/tree_compound.h
tile_size_inference.o: ../global/Error.h ../global/globals.h
tile_size_inference.o: ../tree/traverse.h ../tree/free.h ../global/my_debug.h
tile_size_inference.o: ../global/internal_lib.h ../global/resource.h
tile_size_inference.o: ../tree/DupTree.h ../tree/LookUpTable.h
tile_size_inference.o: ../print/print.h tile_size_inference.h
wl_access_analyze.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
wl_access_analyze.o: /usr/include/sys/feature_tests.h
wl_access_analyze.o: /usr/include/sys/isa_defs.h ../../inc/dbug.h
wl_access_analyze.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
wl_access_analyze.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
wl_access_analyze.o: /usr/include/stdio_impl.h ../global/types.h
wl_access_analyze.o: ../tree/tree_basic.h ../tree/tree_compound.h
wl_access_analyze.o: ../global/Error.h /usr/include/string.h
wl_access_analyze.o: /usr/include/iso/string_iso.h ../global/globals.h
wl_access_analyze.o: ../tree/traverse.h ../tree/free.h ../global/my_debug.h
wl_access_analyze.o: ../global/internal_lib.h wl_access_analyze.h
