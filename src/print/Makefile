#
# $Log$
# Revision 2.7  2000/01/21 12:04:08  dkr
# *** empty log message ***
#
# Revision 2.6  2000/01/17 18:37:49  cg
# Handling of target platform unified: One of the following
# is always defined:
# SAC_FOR_SOLARIS_SPARC, SAC_FOR_LINUX_X86, OR SAC_FOR_OSF_ALPHA.
#
# Revision 2.4  1999/05/12 15:13:23  cg
# make deps done
#
# Revision 2.3  1999/03/11 18:18:20  dkr
# make deps done
#
# Revision 2.2  1999/02/26 14:17:37  dkr
# make deps done
#
# Revision 2.1  1999/02/23 12:40:19  sacbase
# new release made
#
# Revision 1.31  1999/02/09 20:04:19  dkr
# make deps done
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
# Revision 1.22  1997/11/20 15:02:04  dkr
# make deps done
#
# Revision 1.21  1997/11/12 15:59:49  sbs
# CC+CCFLAGS handling changed.
# Now, it suffices to preset CC=cc if the SUN-compiler isdesired
# Otherwise, gcc is set with -Wall and -ansi
#
# Revision 1.20  1997/11/11 16:49:23  sbs
# make deps done
#
# Revision 1.17  1996/07/18  12:22:08  sbs
# convert.o included in clean-line
#
# Revision 1.16  1996/02/16  09:46:46  sbs
# make deps done
#
# Revision 1.15  1996/01/05  12:31:29  cg
# Now, we look even in ../modules for include files
#
# Revision 1.14  1996/01/04  18:11:06  cg
# make deps done
#
# Revision 1.8  1995/04/03  14:02:12  sbs
# ../compile inserted for icm.data-access
#
# Revision 1.7  1995/02/28  18:27:34  asi
# added optimize-dir to include-path
#
# Revision 1.6  1995/01/11  17:45:12  hw
# make deps done
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
     -I../psi-opt -I../optimize

all: print.o convert.o

print.o: print.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o print.o -c print.c $(INCS)

convert.o: convert.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o convert.o -c convert.c $(INCS)

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
convert.o: ../global/tree_basic.h ../global/tree_compound.h ../global/Error.h
convert.o: ../global/globals.h ../global/type_info.mac
print.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
print.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
print.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
print.o: /usr/include/stdlib.h /usr/include/string.h ../global/tree.h
print.o: ../global/types.h ../global/tree_basic.h ../global/tree_compound.h
print.o: ../global/Error.h ../global/globals.h ../global/internal_lib.h
print.o: print.h ../global/my_debug.h ../../inc/dbug.h ../global/traverse.h
print.o: convert.h ../tree/DataFlowMask.h ../optimize/optimize.h
print.o: ../global/free.h /usr/include/malloc.h /usr/include/sys/types.h
print.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
print.o: ../optimize/generatemasks.h ../modules/filemgr.h
print.o: ../compile/gen_startup_code.h ../psi-opt/WithloopFolding.h
print.o: ../concurrent/scheduling.h ../psi-opt/wl_access_analyze.h
print.o: ../psi-opt/tile_size_inference.h ../compile/icm.data
print.o: ../global/prf_node_info.mac
