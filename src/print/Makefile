#
#
# $Log$
# Revision 2.23  2000/11/02 14:24:44  dkr
# header compressed
#
# Revision 2.20  2000/07/24 14:46:21  dkr
# suffix rule for .c.o added
#
# Revision 2.17  2000/07/05 11:41:49  nmw
# moved cinterface files from print to c-interface
#
# Revision 2.16  2000/06/23 16:38:42  nmw
# map_cwrapper inserted
#
# Revision 2.15  2000/06/21 12:12:24  nmw
# make deps, includes modified
#
# Revision 2.13  2000/06/08 12:47:38  nmw
# print_interface added
#
# Revision 2.6  2000/01/17 18:37:49  cg
# Handling of target platform unified: One of the following
# is always defined:
# SAC_FOR_SOLARIS_SPARC, SAC_FOR_LINUX_X86, OR SAC_FOR_OSF_ALPHA.
#
# Revision 1.29  1998/06/18 13:28:22  cg
# added ../concurrent as include directory
#
# Revision 1.28  1998/05/21 09:51:51  dkr
# make deps done
#
# Revision 1.23  1998/03/04 00:10:33  dkr
# 'clean' now deletes the subdir .sb, too
#
# Revision 1.21  1997/11/12 15:59:49  sbs
# CC+CCFLAGS handling changed.
# Now, it suffices to preset CC=cc if the SUN-compiler isdesired
# Otherwise, gcc is set with -Wall and -ansi
#
# Revision 1.17  1996/07/18  12:22:08  sbs
# convert.o included in clean-line
#
# Revision 1.15  1996/01/05  12:31:29  cg
# Now, we look even in ../modules for include files
#
# Revision 1.8  1995/04/03  14:02:12  sbs
# ../compile inserted for icm.data-access
#
# Revision 1.7  1995/02/28  18:27:34  asi
# added optimize-dir to include-path
#
# Revision 1.5  1994/12/05  11:16:46  hw
# added rule for conver.o
#
# Revision 1.4  1994/11/10  15:35:50  sbs
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


INCS=-I../global -I../tree -I../../inc -I../compile -I../modules -I../concurrent \
     -I../psi-opt -I../optimize -I../typecheck -I../c-interface

all: print.o convert.o

%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o $@ -c $< $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) print.o convert.o *.bak
	$(RM) -r .sb SunWS_cache

# DO NOT DELETE THIS LINE -- make depend depends on it.

convert.o: /usr/include/string.h /usr/include/sys/feature_tests.h
convert.o: /usr/include/sys/isa_defs.h /usr/include/stdlib.h
convert.o: /usr/include/math.h ../global/internal_lib.h ../global/types.h
convert.o: ../../inc/dbug.h /usr/include/stdio.h /usr/include/sys/va_list.h
convert.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
convert.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
convert.o: ../global/globals.h ../tree/type_info.mac
print.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
print.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
print.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
print.o: /usr/include/stdlib.h /usr/include/string.h ../tree/tree.h
print.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
print.o: ../global/Error.h ../global/globals.h ../global/internal_lib.h
print.o: print.h ../c-interface/print_interface.h ../global/my_debug.h
print.o: ../../inc/dbug.h ../tree/traverse.h convert.h ../tree/DataFlowMask.h
print.o: ../optimize/optimize.h ../tree/free.h /usr/include/malloc.h
print.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
print.o: /usr/include/sys/int_types.h ../optimize/generatemasks.h
print.o: ../modules/filemgr.h ../compile/gen_startup_code.h
print.o: ../psi-opt/WithloopFolding.h ../tree/scheduling.h
print.o: ../psi-opt/wl_access_analyze.h ../psi-opt/tile_size_inference.h
print.o: ../global/NameTuples.h ../compile/icm.data ../tree/prf_node_info.mac
