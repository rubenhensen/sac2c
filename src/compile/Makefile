#
# $Log$
# Revision 2.25  2000/06/28 09:10:00  nmw
# makedeps
#
# Revision 2.24  2000/06/13 13:46:50  dkr
# deps done
#
# Revision 2.23  2000/06/13 13:21:02  dkr
# module Old2NewWith.[ch] renamed into PatchWith.[ch]
#
# Revision 2.22  2000/05/29 14:36:40  dkr
# make deps done
#
# Revision 2.21  2000/05/29 14:35:16  dkr
# make deps done
#
# Revision 2.20  2000/05/25 18:31:49  dkr
# -I../flatten added
#
# Revision 2.19  2000/03/31 09:33:44  jhs
# make deps done
#
# Revision 2.18  2000/03/31 09:22:22  cg
# make deps done
#
# Revision 2.17  2000/03/17 15:50:59  dkr
# *.o targets simplified
#
# Revision 2.16  2000/03/16 14:22:54  dkr
# target wltransform.c added
#
# Revision 2.15  2000/03/01 14:52:36  dkr
# make deps done
#
# Revision 2.14  2000/01/25 13:52:31  dkr
# make deps done
#
# Revision 2.13  2000/01/24 17:58:33  dkr
# make deps done
#
# Revision 2.12  2000/01/21 17:02:08  dkr
# make deps done
#
# Revision 2.11  2000/01/21 12:03:44  dkr
# directory tree added
#
# Revision 2.10  2000/01/17 18:41:03  cg
# make deps done
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
# Revision 2.4  1999/05/12 15:13:26  cg
# make deps done
#
# Revision 2.3  1999/03/11 18:18:24  dkr
# make deps done
#
# Revision 2.2  1999/02/26 14:17:41  dkr
# make deps done
#
# Revision 2.1  1999/02/23 12:42:20  sacbase
# new release made
#
# Revision 1.65  1998/11/04 18:22:36  dkr
# make deps done
#
# [...]
#
# Revision 1.1  1995/03/29  12:12:52  hw
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

INCS=-I../global -I../tree -I../../inc -I../print -I../typecheck -I../refcount \
     -I../optimize -I../psi-opt -I../scanparse -I../concurrent -I../flatten

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
          icm2c_std.BEtest.o icm2c_wl.BEtest.o icm2c_utils.BEtest.o icm2c_mt.BEtest.o \
          ../global/globals.o ../../lib/dbug.o
	$(CC) $(CCFLAGS) $(CFLAGS) -o BEtest BEtest.c $(INCS) \
          ../../lib/dbug.o icm2c_std.BEtest.o icm2c_wl.BEtest.o \
          icm2c_utils.BEtest.o icm2c_mt.BEtest.o ../global/globals.o 

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
BEtest.o: /usr/include/sys/select.h /usr/include/sys/time.h
BEtest.o: /usr/include/sys/time.h ../../inc/dbug.h ../global/globals.h
BEtest.o: ../global/types.h icm2c_utils.h icm2c_std.h icm2c_wl.h icm2c_mt.h
BEtest.o: icm_vars.c icm.data icm_betest.c
Old2NewWith.o: ../tree/tree.h ../global/types.h ../tree/tree_basic.h
Old2NewWith.o: ../tree/tree_compound.h ../global/Error.h /usr/include/stdio.h
Old2NewWith.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
Old2NewWith.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
Old2NewWith.o: /usr/include/stdio_impl.h /usr/include/string.h
Old2NewWith.o: /usr/include/stdlib.h ../global/globals.h
Old2NewWith.o: ../global/internal_lib.h ../tree/traverse.h
Old2NewWith.o: ../typecheck/typecheck.h ../tree/free.h ../../inc/dbug.h
Old2NewWith.o: ../global/my_debug.h /usr/include/malloc.h
Old2NewWith.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
Old2NewWith.o: /usr/include/sys/int_types.h /usr/include/sys/select.h
Old2NewWith.o: /usr/include/sys/time.h /usr/include/sys/time.h
Old2NewWith.o: ../tree/DupTree.h ../tree/LookUpTable.h
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
PatchWith.o: /usr/include/sys/int_types.h /usr/include/sys/select.h
PatchWith.o: /usr/include/sys/time.h /usr/include/sys/time.h
PatchWith.o: ../tree/DupTree.h ../tree/LookUpTable.h
ReuseWithArrays.o: ../../inc/dbug.h /usr/include/stdio.h
ReuseWithArrays.o: /usr/include/sys/feature_tests.h
ReuseWithArrays.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
ReuseWithArrays.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
ReuseWithArrays.o: ../global/types.h ../tree/free.h ../global/my_debug.h
ReuseWithArrays.o: /usr/include/malloc.h /usr/include/sys/types.h
ReuseWithArrays.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
ReuseWithArrays.o: /usr/include/sys/select.h /usr/include/sys/time.h
ReuseWithArrays.o: /usr/include/sys/time.h ../global/globals.h
ReuseWithArrays.o: ../global/internal_lib.h ../tree/tree_basic.h
ReuseWithArrays.o: ../tree/tree_compound.h ../global/Error.h
ReuseWithArrays.o: /usr/include/string.h /usr/include/stdlib.h
ReuseWithArrays.o: ../tree/traverse.h ../tree/DataFlowMask.h
ReuseWithArrays.o: ../psi-opt/index.h
compile.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
compile.o: /usr/include/sys/isa_defs.h /usr/include/string.h ../tree/tree.h
compile.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
compile.o: ../global/Error.h /usr/include/stdio.h /usr/include/sys/va_list.h
compile.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
compile.o: ../global/globals.h ../global/internal_lib.h ../print/print.h
compile.o: ../../inc/dbug.h ../global/my_debug.h ../tree/traverse.h
compile.o: ../tree/DataFlowMask.h compile.h ../print/convert.h
compile.o: ../tree/DupTree.h ../tree/LookUpTable.h ReuseWithArrays.h
compile.o: ../tree/free.h /usr/include/malloc.h /usr/include/sys/types.h
compile.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
compile.o: /usr/include/sys/select.h /usr/include/sys/time.h
compile.o: /usr/include/sys/time.h ../tree/scheduling.h precompile.h
compile.o: ../typecheck/typecheck.h
gen_startup_code.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
gen_startup_code.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
gen_startup_code.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
gen_startup_code.o: /usr/include/string.h ../../inc/dbug.h
gen_startup_code.o: ../global/globals.h ../global/types.h
gen_startup_code.o: ../global/resource.h ../tree/tree_basic.h
gen_startup_code.o: ../tree/traverse.h
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
icm2c_mt.o: /usr/include/sys/select.h /usr/include/sys/time.h
icm2c_mt.o: /usr/include/sys/time.h /usr/include/stdio.h
icm2c_mt.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
icm2c_mt.o: /usr/include/stdio_impl.h /usr/include/string.h icm2c_basic.h
icm2c_mt.o: ../../inc/dbug.h ../global/my_debug.h ../print/convert.h
icm2c_mt.o: ../global/types.h ../global/globals.h ../print/print.h
icm2c_mt.o: ../tree/tree.h ../tree/tree_basic.h ../tree/tree_compound.h
icm2c_mt.o: ../global/Error.h /usr/include/stdlib.h ../global/internal_lib.h
icm2c_mt.o: ../scanparse/scnprs.h ../global/resource.h ../scanparse/y.tab.h
icm2c_mt.o: ../tree/traverse.h compile.h ../tree/free.h icm_comment.c
icm2c_mt.o: icm.data icm_trace.c
icm2c_std.o: /usr/include/malloc.h /usr/include/sys/types.h
icm2c_std.o: /usr/include/sys/isa_defs.h /usr/include/sys/feature_tests.h
icm2c_std.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
icm2c_std.o: /usr/include/sys/select.h /usr/include/sys/time.h
icm2c_std.o: /usr/include/sys/time.h /usr/include/stdio.h
icm2c_std.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
icm2c_std.o: /usr/include/stdio_impl.h /usr/include/ctype.h icm2c_basic.h
icm2c_std.o: icm2c_std.h ../global/types.h ../../inc/dbug.h
icm2c_std.o: ../global/my_debug.h ../print/convert.h ../global/globals.h
icm2c_std.o: ../print/print.h ../tree/tree.h ../tree/tree_basic.h
icm2c_std.o: ../tree/tree_compound.h ../global/Error.h /usr/include/string.h
icm2c_std.o: /usr/include/stdlib.h ../global/internal_lib.h
icm2c_std.o: gen_startup_code.h icm_comment.c icm.data icm_trace.c
icm2c_utils.o: /usr/include/string.h /usr/include/sys/feature_tests.h
icm2c_utils.o: /usr/include/sys/isa_defs.h ../global/globals.h
icm2c_utils.o: ../global/types.h /usr/include/stdio.h
icm2c_utils.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
icm2c_utils.o: /usr/include/stdio_impl.h icm2c_utils.h ../../inc/dbug.h
icm2c_wl.o: /usr/include/malloc.h /usr/include/sys/types.h
icm2c_wl.o: /usr/include/sys/isa_defs.h /usr/include/sys/feature_tests.h
icm2c_wl.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
icm2c_wl.o: /usr/include/sys/select.h /usr/include/sys/time.h
icm2c_wl.o: /usr/include/sys/time.h /usr/include/stdio.h
icm2c_wl.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
icm2c_wl.o: /usr/include/stdio_impl.h icm2c_basic.h ../../inc/dbug.h
icm2c_wl.o: ../global/my_debug.h ../print/convert.h ../global/types.h
icm2c_wl.o: ../global/globals.h ../print/print.h ../tree/tree.h
icm2c_wl.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
icm2c_wl.o: /usr/include/string.h /usr/include/stdlib.h
icm2c_wl.o: ../global/internal_lib.h gen_startup_code.h icm_comment.c
icm2c_wl.o: icm.data icm_trace.c
icm_betest.o: icm.data
icm_comment.o: icm.data
icm_icm2c.o: icm.data
icm_trace.o: icm.data
icm_vars.o: icm.data
precompile.o: ../../inc/dbug.h /usr/include/stdio.h
precompile.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
precompile.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
precompile.o: /usr/include/stdio_impl.h ../global/types.h ../tree/tree.h
precompile.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
precompile.o: /usr/include/string.h /usr/include/stdlib.h ../global/globals.h
precompile.o: ../global/internal_lib.h ../tree/DupTree.h
precompile.o: ../tree/LookUpTable.h ../tree/free.h ../global/my_debug.h
precompile.o: /usr/include/malloc.h /usr/include/sys/types.h
precompile.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
precompile.o: /usr/include/sys/select.h /usr/include/sys/time.h
precompile.o: /usr/include/sys/time.h ../tree/traverse.h ../print/convert.h
precompile.o: ../tree/DataFlowMask.h ../typecheck/typecheck.h
precompile.o: ../tree/scheduling.h ../flatten/adjust_ids.h precompile.h
precompile.o: ../print/map_cwrapper.h
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
wlpragma_funs.o: /usr/include/sys/select.h /usr/include/sys/time.h
wlpragma_funs.o: /usr/include/sys/time.h ../tree/DupTree.h
wlpragma_funs.o: ../tree/LookUpTable.h wltransform.h wlpragma_funs.h
wltransform.o: ../tree/tree.h ../global/types.h ../tree/tree_basic.h
wltransform.o: ../tree/tree_compound.h ../global/Error.h /usr/include/stdio.h
wltransform.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
wltransform.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
wltransform.o: /usr/include/stdio_impl.h /usr/include/string.h
wltransform.o: /usr/include/stdlib.h ../global/globals.h
wltransform.o: ../global/internal_lib.h ../tree/traverse.h ../tree/free.h
wltransform.o: ../../inc/dbug.h ../global/my_debug.h /usr/include/malloc.h
wltransform.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
wltransform.o: /usr/include/sys/int_types.h /usr/include/sys/select.h
wltransform.o: /usr/include/sys/time.h /usr/include/sys/time.h
wltransform.o: ../tree/DupTree.h ../tree/LookUpTable.h ../tree/DataFlowMask.h
wltransform.o: ../print/print.h wlpragma_funs.h /usr/include/limits.h
