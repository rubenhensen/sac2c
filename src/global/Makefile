#
# $Log$
# Revision 2.13  2000/01/21 12:03:07  dkr
# directory tree added
#
# Revision 2.12  2000/01/17 18:40:52  cg
# make deps done
#
# Revision 2.11  2000/01/17 18:37:49  cg
# Handling of target platform unified: One of the following
# is always defined:
# SAC_FOR_SOLARIS_SPARC, SAC_FOR_LINUX_X86, OR SAC_FOR_OSF_ALPHA.
#
# Revision 2.9  1999/10/08 08:34:05  sbs
# in build.c :
# changed more into cat since some systems (eg alpha) do request an
# acknowledge of more!
#
# Revision 2.8  1999/08/05 13:32:04  jhs
# build.c is displayed (more) after creation
# (I like to see the values ...).
#
# Revision 2.7  1999/07/09 11:38:40  cg
# make deps done
#
# Revision 2.6  1999/06/16 14:36:17  dkr
# make deps done
#
# Revision 2.5  1999/05/12 15:13:22  cg
# make deps done
#
# Revision 2.4  1999/05/12 14:29:25  cg
# added compilation of options.c
# improved generation of build.c
#
# Revision 2.3  1999/03/11 18:18:19  dkr
# make deps done
#
# Revision 2.2  1999/02/26 14:17:37  dkr
# make deps done
#
# Revision 2.1  1999/02/23 12:39:14  sacbase
# new release made
#
# Revision 1.70  1999/02/09 20:04:18  dkr
# make deps done
#
# Revision 1.69  1999/01/25 16:26:58  sbs
# interrupt.[cho] added
#
# Revision 1.68  1998/11/04 18:22:30  dkr
# make deps done
#
# Revision 1.67  1998/10/26 12:32:59  cg
# new mechanism implemented that stores sac2c build information
# which may later be retrieved by the -h or -i option, repsectively.
#
# [...]
#
# Revision 1.2  1994/11/10  15:44:34  sbs
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

ECHO      := echo -n

INCS=-I../../inc -I../tree -I../print -I../flatten -I../scanparse -I../typecheck \
     -I../optimize -I../modules -I../global -I../refcount -I../compile \
     -I../psi-opt -I../objects -I../concurrent -I../Old2NewWith

all: main.o Error.o traverse.o usage.o my_debug.o tree.o free.o  \
     internal_lib.o tree_basic.o tree_compound.o globals.o \
     resource.o types.h build.o interrupt.o options.o

#
# the types.h object is to fool makedepend which does not
# recognize the dependency between types.h and the .mac files!
#

types.h: type_info.mac node_info.mac prf_node_info.mac
	touch types.h

interrupt.o: interrupt.c 
	$(CC) $(CCFLAGS) $(CFLAGS) -o interrupt.o -c interrupt.c $(INCS)

main.o: main.c 
	$(CC) $(CCFLAGS) $(CFLAGS) -o main.o -c main.c $(INCS)

Error.o: Error.c 
	$(CC) $(CCFLAGS) $(CFLAGS) -o Error.o -c Error.c $(INCS) 

traverse.o: traverse.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o traverse.o -c traverse.c $(INCS)

usage.o: usage.c usage.h 
	$(CC) $(CCFLAGS) $(CFLAGS) -o usage.o -c usage.c $(INCS)

my_debug.o: my_debug.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o my_debug.o -c my_debug.c $(INCS)

tree.o: tree.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o tree.o -c tree.c $(INCS)

tree_basic.o: tree_basic.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o tree_basic.o -c tree_basic.c $(INCS)

tree_compound.o: tree_compound.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o tree_compound.o -c tree_compound.c $(INCS)

free.o: free.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o free.o -c free.c $(INCS)

internal_lib.o: internal_lib.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o internal_lib.o -c internal_lib.c  $(INCS)

globals.o: globals.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o globals.o -c globals.c $(INCS)

resource.o: resource.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o resource.o -c resource.c $(INCS)

options.o: options.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o options.o -c options.c $(INCS)

build.o: build.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o build.o -c build.c $(INCS)
	$(RM) build.c

build.c: 
	echo "char build_date[] = \"`date`\";"  >  build.c
	echo "char build_user[] = \"$(USER)\";" >> build.c
	echo "char build_host[] = \"$(HOST)\";" >> build.c
	cat build.c

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak *~
	$(RM) -r .sb SunWS_cache

# DO NOT DELETE THIS LINE -- make depend depends on it.

Error.o: /usr/include/stdarg.h /usr/include/varargs.h
Error.o: /usr/include/sys/va_list.h /usr/include/stdio.h
Error.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
Error.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
Error.o: /usr/include/string.h Error.h /usr/include/stdlib.h types.h
Error.o: globals.h tree_basic.h ../../inc/dbug.h internal_lib.h
Error.o: ../modules/filemgr.h resource.h
free.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
free.o: /usr/include/sys/isa_defs.h /usr/include/stdio.h
free.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
free.o: /usr/include/stdio_impl.h ../../inc/dbug.h my_debug.h types.h tree.h
free.o: tree_basic.h tree_compound.h Error.h /usr/include/string.h globals.h
free.o: internal_lib.h traverse.h ../tree/DataFlowMask.h
free.o: ../concurrent/scheduling.h free.h /usr/include/malloc.h
free.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
free.o: /usr/include/sys/int_types.h
globals.o: ../modules/filemgr.h /usr/include/stdio.h
globals.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
globals.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
globals.o: /usr/include/stdio_impl.h types.h globals.h Error.h
globals.o: /usr/include/string.h /usr/include/stdlib.h type_info.mac
globals.o: nt_info.mac
internal_lib.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
internal_lib.o: /usr/include/sys/isa_defs.h /usr/include/math.h
internal_lib.o: /usr/include/string.h /usr/include/stdarg.h
internal_lib.o: /usr/include/varargs.h /usr/include/sys/va_list.h
internal_lib.o: /usr/include/ctype.h Error.h /usr/include/stdio.h
internal_lib.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h types.h
internal_lib.o: globals.h free.h ../../inc/dbug.h my_debug.h
internal_lib.o: /usr/include/malloc.h /usr/include/sys/types.h
internal_lib.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
internal_lib.o: internal_lib.h traverse.h tree_basic.h tree_compound.h
interrupt.o: /usr/include/signal.h /usr/include/sys/feature_tests.h
interrupt.o: /usr/include/sys/isa_defs.h /usr/include/sys/signal.h
interrupt.o: /usr/include/sys/unistd.h /usr/include/stdio.h
interrupt.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
interrupt.o: /usr/include/stdio_impl.h Error.h /usr/include/string.h
interrupt.o: /usr/include/stdlib.h types.h globals.h interrupt.h
interrupt.o: internal_lib.h build.h
main.o: tree.h types.h tree_basic.h tree_compound.h Error.h
main.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
main.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
main.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
main.o: /usr/include/string.h /usr/include/stdlib.h globals.h internal_lib.h
main.o: free.h ../../inc/dbug.h my_debug.h /usr/include/malloc.h
main.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
main.o: /usr/include/sys/int_types.h usage.h ../flatten/flatten.h
main.o: ../print/print.h ../typecheck/typecheck.h ../optimize/optimize.h
main.o: ../modules/filemgr.h ../modules/import.h ../refcount/refcount.h
main.o: ../scanparse/scnprs.h resource.h ../scanparse/y.tab.h
main.o: ../psi-opt/psi-opt.h ../modules/writesib.h ../modules/readsib.h
main.o: ../modules/implicittypes.h ../objects/objinit.h ../modules/analysis.h
main.o: ../modules/checkdec.h ../objects/objects.h ../objects/uniquecheck.h
main.o: ../objects/rmvoidfun.h ../compile/wltransform.h
main.o: ../concurrent/concurrent.h ../compile/precompile.h
main.o: ../compile/compile.h ../modules/cccall.h ../compile/Old2NewWith.h
main.o: interrupt.h options.h
my_debug.o: node_info.mac prf_node_info.mac type_info.mac
options.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
options.o: /usr/include/sys/isa_defs.h ../../inc/dbug.h /usr/include/stdio.h
options.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
options.o: /usr/include/stdio_impl.h ../../inc/main_args.h
options.o: /usr/include/string.h usage.h ../modules/filemgr.h globals.h
options.o: types.h internal_lib.h free.h my_debug.h /usr/include/malloc.h
options.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
options.o: /usr/include/sys/int_types.h Error.h
resource.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
resource.o: /usr/include/sys/isa_defs.h /usr/include/stdio.h
resource.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
resource.o: /usr/include/stdio_impl.h /usr/include/string.h types.h
resource.o: tree_basic.h ../../inc/dbug.h resource.h ../scanparse/scnprs.h
resource.o: globals.h ../scanparse/y.tab.h ../modules/filemgr.h free.h
resource.o: my_debug.h /usr/include/malloc.h /usr/include/sys/types.h
resource.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
resource.o: internal_lib.h Error.h
traverse.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
traverse.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
traverse.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
traverse.o: /usr/include/stdlib.h tree.h types.h tree_basic.h tree_compound.h
traverse.o: Error.h /usr/include/string.h globals.h internal_lib.h
traverse.o: ../../inc/dbug.h my_debug.h ../flatten/flatten.h ../print/print.h
traverse.o: ../typecheck/typecheck.h ../typecheck/typecheck_WL.h
traverse.o: ../optimize/optimize.h free.h /usr/include/malloc.h
traverse.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
traverse.o: /usr/include/sys/int_types.h ../optimize/generatemasks.h
traverse.o: ../optimize/freemasks.h ../optimize/ConstantFolding.h
traverse.o: ../optimize/DeadCodeRemoval.h ../optimize/DeadFunctionRemoval.h
traverse.o: ../optimize/LoopInvariantRemoval.h ../optimize/CSE.h
traverse.o: ../modules/import.h ../tree/DupTree.h ../optimize/Inline.h
traverse.o: ../optimize/Unroll.h ../optimize/Unswitch.h
traverse.o: ../psi-opt/ArrayElimination.h ../psi-opt/index.h
traverse.o: ../modules/writesib.h ../modules/readsib.h
traverse.o: ../modules/implicittypes.h ../objects/objinit.h
traverse.o: ../modules/analysis.h ../modules/checkdec.h ../objects/objects.h
traverse.o: ../objects/uniquecheck.h ../objects/rmvoidfun.h
traverse.o: ../refcount/refcount.h ../compile/wltransform.h
traverse.o: ../compile/precompile.h ../compile/compile.h
traverse.o: ../compile/ReuseWithArrays.h ../modules/cccall.h
traverse.o: ../compile/Old2NewWith.h ../psi-opt/WithloopFolding.h
traverse.o: ../psi-opt/WLT.h ../psi-opt/WLI.h ../psi-opt/WLF.h
traverse.o: ../compile/gen_startup_code.h ../concurrent/scheduling.h
traverse.o: ../concurrent/concurrent.h ../concurrent/spmd_init.h
traverse.o: ../concurrent/spmd_opt.h ../concurrent/spmd_lift.h
traverse.o: ../concurrent/sync_init.h ../concurrent/sync_opt.h
traverse.o: ../concurrent/spmd_trav.h ../tree/DataFlowMask.h
traverse.o: ../concurrent/spmd_cons.h ../concurrent/schedule.h
traverse.o: ../psi-opt/wl_access_analyze.h ../psi-opt/tile_size_inference.h
traverse.o: ../typecheck/new_typecheck.h traverse.h node_info.mac
tree.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
tree.o: /usr/include/sys/isa_defs.h tree.h types.h tree_basic.h
tree.o: tree_compound.h Error.h /usr/include/stdio.h
tree.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
tree.o: /usr/include/stdio_impl.h /usr/include/string.h globals.h
tree.o: internal_lib.h ../../inc/dbug.h my_debug.h ../scanparse/scnprs.h
tree.o: resource.h ../scanparse/y.tab.h ../optimize/optimize.h free.h
tree.o: /usr/include/malloc.h /usr/include/sys/types.h
tree.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
tree_basic.o: types.h tree_basic.h Error.h /usr/include/stdio.h
tree_basic.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
tree_basic.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
tree_basic.o: /usr/include/stdio_impl.h /usr/include/string.h
tree_basic.o: /usr/include/stdlib.h globals.h ../../inc/dbug.h my_debug.h
tree_basic.o: ../scanparse/scnprs.h resource.h ../scanparse/y.tab.h free.h
tree_basic.o: /usr/include/malloc.h /usr/include/sys/types.h
tree_basic.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
tree_basic.o: internal_lib.h prf_node_info.mac
tree_compound.o: types.h tree_basic.h tree_compound.h Error.h
tree_compound.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
tree_compound.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
tree_compound.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
tree_compound.o: /usr/include/string.h /usr/include/stdlib.h globals.h
tree_compound.o: ../../inc/dbug.h my_debug.h free.h /usr/include/malloc.h
tree_compound.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
tree_compound.o: /usr/include/sys/int_types.h internal_lib.h
usage.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
usage.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
usage.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
usage.o: /usr/include/stdlib.h usage.h globals.h types.h build.h
usage.o: ../../inc/dbug.h
