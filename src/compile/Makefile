#
#
# $Log$
# Revision 3.3  2000/12/12 15:09:09  dkr
# make deps done
#
# Revision 3.2  2000/12/06 20:09:04  dkr
# make deps done
#
# Revision 3.1  2000/11/20 18:01:00  sacbase
# new release made
#
# Revision 2.36  2000/11/02 14:43:01  dkr
# header compressed
#
# Revision 2.28  2000/07/06 14:51:48  dkr
# target BEtest modified
#
# Revision 2.26  2000/07/05 11:42:09  nmw
# moved cinterface files from print to c-interface
#
# Revision 2.23  2000/06/13 13:21:02  dkr
# module Old2NewWith.[ch] renamed into PatchWith.[ch]
#
# Revision 2.20  2000/05/25 18:31:49  dkr
# -I../flatten added
#
# Revision 2.17  2000/03/17 15:50:59  dkr
# *.o targets simplified
#
# Revision 2.16  2000/03/16 14:22:54  dkr
# target wltransform.c added
#
# Revision 2.11  2000/01/21 12:03:44  dkr
# directory tree added
#
# Revision 2.9  2000/01/17 18:37:49  cg
# Handling of target platform unified: One of the following
# is always defined:
# SAC_FOR_SOLARIS_SPARC, SAC_FOR_LINUX_X86, OR SAC_FOR_OSF_ALPHA.
#
# Revision 2.7  1999/06/25 14:54:35  rob
# Introduce definitions and utility infrastructure for tagged array support.
#
# Revision 2.6  1999/06/16 17:21:37  rob
# Add support for TAGGED_ARRAYS.
#
# [...]
#
# Revision 1.1  1995/03/29  12:12:52  hw
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

INCS=-I../global -I../tree -I../../inc -I../print -I../typecheck -I../refcount \
     -I../optimize -I../psi-opt -I../scanparse -I../concurrent -I../flatten \
     -I../c-interface

MACS=icm.data icm_comment.c icm_trace.c

all: wltransform.o wlpragma_funs.o precompile.o compile.o ReuseWithArrays.o \
     icm2c.o icm2c_std.o icm2c_wl.o icm2c_mt.o icm2c_utils.o gen_startup_code.o \
     PatchWith.o


#
# the wltransform.c object is to fool makedepend which does not
# recognize the dependency between wltransform.c and the .mac files!
#
wltransform.c: wlpragma_funs.mac
	touch wltransform.c

%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o $@ -c $< $(INCS)


BEtest: BEtest.c icm_vars.c icm_betest.c icm.data \
          ../global/globals.o ../../lib/dbug.o \
          icm2c_std.BEtest.o icm2c_wl.BEtest.o icm2c_utils.BEtest.o \
          icm2c_mt.BEtest.o
	$(CC) $(CCFLAGS) $(CFLAGS) -o BEtest BEtest.c $(INCS) \
          ../global/globals.o  ../../lib/dbug.o \
          icm2c_std.BEtest.o icm2c_wl.BEtest.o icm2c_utils.BEtest.o \
          icm2c_mt.BEtest.o

icm2c_std.BEtest.o: icm2c_std.c $(MACS)
	$(CC) $(CCFLAGS) $(CFLAGS) -DBEtest -o icm2c_std.BEtest.o -c icm2c_std.c $(INCS)

icm2c_wl.BEtest.o: icm2c_wl.c $(MACS)
	$(CC) $(CCFLAGS) $(CFLAGS) -DBEtest -o icm2c_wl.BEtest.o -c icm2c_wl.c $(INCS)

icm2c_mt.BEtest.o: icm2c_mt.c $(MACS)
	$(CC) $(CCFLAGS) $(CFLAGS) -DBEtest -o icm2c_mt.BEtest.o -c icm2c_mt.c $(INCS)

icm2c_utils.BEtest.o: icm2c_utils.c $(MACS)
	$(CC) $(CCFLAGS) $(CFLAGS) -DBEtest -o icm2c_utils.BEtest.o -c icm2c_utils.c $(INCS)


deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak BEtest *.sib *.sac
	$(RM) -r .sb SunWS_cache

# DO NOT DELETE THIS LINE -- make depend depends on it.

BEtest.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
BEtest.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
BEtest.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
BEtest.o: /usr/include/malloc.h /usr/include/sys/types.h
BEtest.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
BEtest.o: ../../inc/dbug.h ../global/globals.h ../global/types.h icm2c_std.h
BEtest.o: icm2c_wl.h icm2c_mt.h icm_vars.c icm.data icm_betest.c
PatchWith.o: ../tree/tree.h ../global/types.h ../tree/tree_basic.h
PatchWith.o: ../tree/tree_compound.h ../global/Error.h /usr/include/stdio.h
PatchWith.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
PatchWith.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
PatchWith.o: /usr/include/stdio_impl.h /usr/include/string.h
PatchWith.o: /usr/include/stdlib.h ../global/globals.h
PatchWith.o: ../global/internal_lib.h ../tree/traverse.h
PatchWith.o: ../typecheck/typecheck.h ../tree/free.h ../../inc/dbug.h
PatchWith.o: ../global/my_debug.h /usr/include/malloc.h
PatchWith.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
PatchWith.o: /usr/include/sys/int_types.h ../tree/DupTree.h
PatchWith.o: ../tree/LookUpTable.h
ReuseWithArrays.o: ../../inc/dbug.h /usr/include/stdio.h
ReuseWithArrays.o: /usr/include/sys/feature_tests.h
ReuseWithArrays.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
ReuseWithArrays.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
ReuseWithArrays.o: ../global/types.h ../tree/free.h ../global/my_debug.h
ReuseWithArrays.o: /usr/include/malloc.h /usr/include/sys/types.h
ReuseWithArrays.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
ReuseWithArrays.o: ../global/globals.h ../global/internal_lib.h
ReuseWithArrays.o: ../tree/tree_basic.h ../tree/tree_compound.h
ReuseWithArrays.o: ../global/Error.h /usr/include/string.h
ReuseWithArrays.o: /usr/include/stdlib.h ../tree/traverse.h
ReuseWithArrays.o: ../tree/DataFlowMask.h ../psi-opt/index.h
compile.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
compile.o: /usr/include/sys/isa_defs.h /usr/include/string.h ../tree/tree.h
compile.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
compile.o: ../global/Error.h /usr/include/stdio.h /usr/include/sys/va_list.h
compile.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
compile.o: ../global/globals.h ../global/internal_lib.h ../tree/prf.h
compile.o: ../print/print.h ../../inc/dbug.h ../global/my_debug.h
compile.o: ../tree/traverse.h ../tree/DataFlowMask.h compile.h
compile.o: ../print/convert.h ../tree/DupTree.h ../tree/LookUpTable.h
compile.o: ReuseWithArrays.h ../tree/free.h /usr/include/malloc.h
compile.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
compile.o: /usr/include/sys/int_types.h ../tree/scheduling.h
compile.o: ../refcount/refcount.h precompile.h ../typecheck/typecheck.h
gen_startup_code.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
gen_startup_code.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
gen_startup_code.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
gen_startup_code.o: /usr/include/string.h /usr/include/stdlib.h
gen_startup_code.o: ../../inc/dbug.h ../global/globals.h ../global/types.h
gen_startup_code.o: ../global/resource.h ../tree/tree_basic.h
gen_startup_code.o: ../tree/traverse.h ../tree/free.h ../global/my_debug.h
gen_startup_code.o: /usr/include/malloc.h /usr/include/sys/types.h
gen_startup_code.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
gen_startup_code.o: ../global/internal_lib.h precompile.h
icm2c.o: ../tree/tree.h ../global/types.h ../tree/tree_basic.h
icm2c.o: ../tree/tree_compound.h ../global/Error.h /usr/include/stdio.h
icm2c.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
icm2c.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
icm2c.o: /usr/include/stdio_impl.h /usr/include/string.h
icm2c.o: /usr/include/stdlib.h ../global/globals.h ../global/internal_lib.h
icm2c.o: ../../inc/dbug.h ../print/convert.h ../global/my_debug.h icm2c_std.h
icm2c.o: icm2c_wl.h icm2c_mt.h icm_vars.c icm.data icm_icm2c.c
icm2c_mt.o: /usr/include/malloc.h /usr/include/sys/types.h
icm2c_mt.o: /usr/include/sys/isa_defs.h /usr/include/sys/feature_tests.h
icm2c_mt.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
icm2c_mt.o: /usr/include/stdio.h /usr/include/sys/va_list.h
icm2c_mt.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
icm2c_mt.o: /usr/include/string.h icm2c_basic.h ../../inc/dbug.h
icm2c_mt.o: ../global/my_debug.h ../print/convert.h ../global/types.h
icm2c_mt.o: ../global/globals.h ../print/print.h ../tree/tree.h
icm2c_mt.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
icm2c_mt.o: /usr/include/stdlib.h ../global/internal_lib.h
icm2c_mt.o: ../scanparse/scnprs.h ../global/resource.h ../scanparse/y.tab.h
icm2c_mt.o: ../tree/traverse.h compile.h ../tree/free.h icm_comment.c
icm2c_mt.o: icm.data icm_trace.c
icm2c_std.o: /usr/include/malloc.h /usr/include/sys/types.h
icm2c_std.o: /usr/include/sys/isa_defs.h /usr/include/sys/feature_tests.h
icm2c_std.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
icm2c_std.o: /usr/include/stdio.h /usr/include/sys/va_list.h
icm2c_std.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
icm2c_std.o: /usr/include/ctype.h icm2c_basic.h icm2c_std.h ../global/types.h
icm2c_std.o: ../../inc/dbug.h ../global/my_debug.h ../print/convert.h
icm2c_std.o: ../global/globals.h ../print/print.h ../tree/tree.h
icm2c_std.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
icm2c_std.o: /usr/include/string.h /usr/include/stdlib.h
icm2c_std.o: ../global/internal_lib.h gen_startup_code.h icm_comment.c
icm2c_std.o: icm.data icm_trace.c
icm2c_utils.o: /usr/include/string.h /usr/include/sys/feature_tests.h
icm2c_utils.o: /usr/include/sys/isa_defs.h ../global/globals.h
icm2c_utils.o: /usr/include/stdio.h /usr/include/sys/va_list.h
icm2c_utils.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
icm2c_utils.o: ../global/types.h icm2c_utils.h ../global/NameTuples.h
icm2c_utils.o: ../../inc/dbug.h
icm2c_wl.o: /usr/include/malloc.h /usr/include/sys/types.h
icm2c_wl.o: /usr/include/sys/isa_defs.h /usr/include/sys/feature_tests.h
icm2c_wl.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
icm2c_wl.o: /usr/include/stdio.h /usr/include/sys/va_list.h
icm2c_wl.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h icm2c_basic.h
icm2c_wl.o: ../../inc/dbug.h ../global/my_debug.h ../print/convert.h
icm2c_wl.o: ../global/types.h ../global/globals.h ../print/print.h
icm2c_wl.o: ../tree/tree.h ../tree/tree_basic.h ../tree/tree_compound.h
icm2c_wl.o: ../global/Error.h /usr/include/string.h /usr/include/stdlib.h
icm2c_wl.o: ../global/internal_lib.h gen_startup_code.h icm_comment.c
icm2c_wl.o: icm.data icm_trace.c
icm_betest.o: icm.data
icm_comment.o: icm.data
icm_icm2c.o: icm.data
icm_trace.o: icm.data
icm_vars.o: icm.data
precompile.o: /usr/include/string.h /usr/include/sys/feature_tests.h
precompile.o: /usr/include/sys/isa_defs.h ../../inc/dbug.h
precompile.o: /usr/include/stdio.h /usr/include/sys/va_list.h
precompile.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
precompile.o: ../global/types.h ../tree/tree.h ../tree/tree_basic.h
precompile.o: ../tree/tree_compound.h ../global/Error.h /usr/include/stdlib.h
precompile.o: ../global/globals.h ../global/internal_lib.h ../tree/DupTree.h
precompile.o: ../tree/LookUpTable.h ../tree/free.h ../global/my_debug.h
precompile.o: /usr/include/malloc.h /usr/include/sys/types.h
precompile.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
precompile.o: ../tree/prf.h ../tree/traverse.h ../print/convert.h
precompile.o: ../tree/DataFlowMask.h ../typecheck/typecheck.h
precompile.o: ../tree/scheduling.h ../flatten/adjust_ids.h precompile.h
precompile.o: ../c-interface/map_cwrapper.h ../refcount/refcount.h compile.h
wlpragma_funs.o: ../../inc/dbug.h /usr/include/stdio.h
wlpragma_funs.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
wlpragma_funs.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
wlpragma_funs.o: /usr/include/stdio_impl.h ../tree/tree.h ../global/types.h
wlpragma_funs.o: ../tree/tree_basic.h ../tree/tree_compound.h
wlpragma_funs.o: ../global/Error.h /usr/include/string.h
wlpragma_funs.o: /usr/include/stdlib.h ../global/globals.h
wlpragma_funs.o: ../global/internal_lib.h ../tree/free.h ../global/my_debug.h
wlpragma_funs.o: /usr/include/malloc.h /usr/include/sys/types.h
wlpragma_funs.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
wlpragma_funs.o: ../tree/DupTree.h ../tree/LookUpTable.h wltransform.h
wlpragma_funs.o: wlpragma_funs.h
wltransform.o: ../tree/tree.h ../global/types.h ../tree/tree_basic.h
wltransform.o: ../tree/tree_compound.h ../global/Error.h /usr/include/stdio.h
wltransform.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
wltransform.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
wltransform.o: /usr/include/stdio_impl.h /usr/include/string.h
wltransform.o: /usr/include/stdlib.h ../global/globals.h
wltransform.o: ../global/internal_lib.h ../tree/traverse.h ../tree/free.h
wltransform.o: ../../inc/dbug.h ../global/my_debug.h /usr/include/malloc.h
wltransform.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
wltransform.o: /usr/include/sys/int_types.h ../tree/DupTree.h
wltransform.o: ../tree/LookUpTable.h ../tree/DataFlowMask.h ../print/print.h
wltransform.o: ../global/resource.h wlpragma_funs.h /usr/include/limits.h
