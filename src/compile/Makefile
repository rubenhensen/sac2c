#
#
# $Log$
# Revision 3.15  2001/11/13 20:32:44  dkr
# make deps done
#
# Revision 3.14  2001/07/18 13:08:18  cg
# make deps done
#
# Revision 3.13  2001/05/17 13:17:19  dkr
# minor changes done
#
# Revision 3.12  2001/05/11 14:36:56  cg
# Added compilation of new source file icm2c_sched.c
#
# Revision 3.11  2001/05/08 13:29:35  dkr
# make deps done
#
# Revision 3.10  2001/04/26 21:09:47  dkr
# make deps done
#
# Revision 3.9  2001/03/22 20:20:01  dkr
# make deps done
#
# Revision 3.8  2001/03/21 11:19:57  dkr
# make deps done
#
# Revision 3.7  2001/03/02 10:36:30  dkr
# /home/dkr/sac/BASE/sac2c/src/compile/RCS/Makefile,v used in touch-rules
#
# Revision 3.6  2001/02/02 10:39:03  dkr
# make deps done
#
# Revision 3.5  2001/01/17 10:13:04  dkr
# make deps done
#
# Revision 3.4  2000/12/15 18:18:00  dkr
# make deps done
#
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

BETEST_OBJS = icm2c_std.BEtest.o icm2c_wl.BEtest.o \
              icm2c_mt.BEtest.o icm2c_sched.BEtest.o \
              ../global/globals.o ../../lib/dbug.o


all: wltransform.o wlpragma_funs.o precompile.o compile.o ReuseWithArrays.o \
     icm2c.o icm2c_std.o icm2c_wl.o icm2c_mt.o icm2c_sched.o icm2c_utils.o \
     gen_startup_code.o PatchWith.o


#
# the wltransform.c object is to fool makedepend which does not
# recognize the dependency between wltransform.c and the .mac files!
#
wltransform.c: wlpragma_funs.mac
	touch $@

%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<


BEtest: BEtest.c icm_vars.c icm_betest.c icm.data $(BETEST_OBJS)
	$(CC) $(CCFLAGS) $(CFLAGS) -o BEtest $(INCS) BEtest.c $(BETEST_OBJS)

icm2c_std.BEtest.o: icm2c_std.c $(MACS)
	$(CC) $(CCFLAGS) $(CFLAGS) -DBEtest -o icm2c_std.BEtest.o -c icm2c_std.c $(INCS)

icm2c_wl.BEtest.o: icm2c_wl.c $(MACS)
	$(CC) $(CCFLAGS) $(CFLAGS) -DBEtest -o icm2c_wl.BEtest.o -c icm2c_wl.c $(INCS)

icm2c_mt.BEtest.o: icm2c_mt.c $(MACS)
	$(CC) $(CCFLAGS) $(CFLAGS) -DBEtest -o icm2c_mt.BEtest.o -c icm2c_mt.c $(INCS)

icm2c_sched.BEtest.o: icm2c_sched.c $(MACS)
	$(CC) $(CCFLAGS) $(CFLAGS) -DBEtest -o icm2c_sched.BEtest.o -c icm2c_sched.c $(INCS)

icm2c_utils.BEtest.o: icm2c_utils.c $(MACS)
	$(CC) $(CCFLAGS) $(CFLAGS) -DBEtest -o icm2c_utils.BEtest.o -c icm2c_utils.c $(INCS)


deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak *~ BEtest *.sib *.sac
	$(RM) -r .sb SunWS_cache

# DO NOT DELETE THIS LINE -- make depend depends on it.

BEtest.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
BEtest.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
BEtest.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
BEtest.o: /usr/include/stdio_impl.h /usr/include/malloc.h
BEtest.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
BEtest.o: /usr/include/sys/int_types.h ../../inc/dbug.h /usr/include/stdlib.h
BEtest.o: /usr/include/iso/stdlib_iso.h ../global/globals.h ../global/types.h
BEtest.o: icm2c_std.h icm2c_wl.h icm2c_mt.h icm_vars.c icm.data icm_betest.c
PatchWith.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
PatchWith.o: ../global/Error.h /usr/include/stdio.h
PatchWith.o: /usr/include/iso/stdio_iso.h /usr/include/sys/feature_tests.h
PatchWith.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
PatchWith.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
PatchWith.o: /usr/include/string.h /usr/include/iso/string_iso.h
PatchWith.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
PatchWith.o: ../global/globals.h ../global/internal_lib.h ../tree/traverse.h
PatchWith.o: ../tree/free.h ../../inc/dbug.h ../global/my_debug.h
PatchWith.o: ../tree/DupTree.h ../tree/LookUpTable.h
ReuseWithArrays.o: ../../inc/dbug.h /usr/include/stdio.h
ReuseWithArrays.o: /usr/include/iso/stdio_iso.h
ReuseWithArrays.o: /usr/include/sys/feature_tests.h
ReuseWithArrays.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
ReuseWithArrays.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
ReuseWithArrays.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
ReuseWithArrays.o: ../global/types.h ../tree/free.h ../global/my_debug.h
ReuseWithArrays.o: ../global/globals.h ../global/internal_lib.h
ReuseWithArrays.o: ../tree/tree_basic.h ../tree/tree_compound.h
ReuseWithArrays.o: ../global/Error.h /usr/include/string.h
ReuseWithArrays.o: /usr/include/iso/string_iso.h ../tree/traverse.h
ReuseWithArrays.o: ../tree/DataFlowMask.h ../psi-opt/index.h
compile.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
compile.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
compile.o: /usr/include/string.h /usr/include/iso/string_iso.h
compile.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
compile.o: ../global/Error.h /usr/include/stdio.h
compile.o: /usr/include/iso/stdio_iso.h /usr/include/sys/va_list.h
compile.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
compile.o: ../global/globals.h ../global/internal_lib.h ../tree/prf.h
compile.o: ../../inc/dbug.h ../global/my_debug.h ../tree/traverse.h
compile.o: ../tree/free.h ../tree/DupTree.h ../tree/LookUpTable.h
compile.o: ../print/convert.h ../print/print.h ../tree/DataFlowMask.h
compile.o: ReuseWithArrays.h ../typecheck/typecheck.h ../tree/scheduling.h
compile.o: ../tree/wl_bounds.h ../refcount/refcount.h compile.h
gen_startup_code.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
gen_startup_code.o: /usr/include/sys/feature_tests.h
gen_startup_code.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
gen_startup_code.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
gen_startup_code.o: /usr/include/string.h /usr/include/iso/string_iso.h
gen_startup_code.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
gen_startup_code.o: ../../inc/dbug.h ../global/globals.h ../global/types.h
gen_startup_code.o: ../global/resource.h ../tree/tree_basic.h
gen_startup_code.o: ../tree/tree_compound.h ../global/Error.h
gen_startup_code.o: ../tree/traverse.h ../tree/free.h ../global/my_debug.h
gen_startup_code.o: ../global/internal_lib.h precompile.h
icm2c.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
icm2c.o: ../global/Error.h /usr/include/stdio.h /usr/include/iso/stdio_iso.h
icm2c.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
icm2c.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
icm2c.o: /usr/include/stdio_impl.h /usr/include/string.h
icm2c.o: /usr/include/iso/string_iso.h /usr/include/stdlib.h
icm2c.o: /usr/include/iso/stdlib_iso.h ../global/globals.h
icm2c.o: ../global/internal_lib.h ../../inc/dbug.h ../print/convert.h
icm2c.o: ../global/my_debug.h icm2c_std.h icm2c_wl.h icm2c_mt.h icm2c_sched.h
icm2c.o: icm_vars.c icm.data icm_icm2c.c
icm2c_mt.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
icm2c_mt.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
icm2c_mt.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
icm2c_mt.o: /usr/include/stdio_impl.h /usr/include/string.h
icm2c_mt.o: /usr/include/iso/string_iso.h icm2c_basic.h ../../inc/dbug.h
icm2c_mt.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
icm2c_mt.o: ../global/my_debug.h ../print/convert.h ../global/types.h
icm2c_mt.o: ../global/globals.h ../print/print.h ../tree/tree_basic.h
icm2c_mt.o: ../tree/tree_compound.h ../global/Error.h
icm2c_mt.o: ../global/internal_lib.h ../scanparse/scnprs.h
icm2c_mt.o: ../global/resource.h ../scanparse/y.tab.h ../tree/traverse.h
icm2c_mt.o: compile.h ../tree/free.h icm_comment.c icm.data icm_trace.c
icm2c_sched.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
icm2c_sched.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
icm2c_sched.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
icm2c_sched.o: /usr/include/stdio_impl.h /usr/include/string.h
icm2c_sched.o: /usr/include/iso/string_iso.h icm2c_basic.h ../../inc/dbug.h
icm2c_sched.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
icm2c_sched.o: ../global/my_debug.h ../print/convert.h ../global/types.h
icm2c_sched.o: ../global/globals.h ../print/print.h ../tree/tree_basic.h
icm2c_sched.o: ../tree/tree_compound.h ../global/Error.h
icm2c_sched.o: ../global/internal_lib.h ../scanparse/scnprs.h
icm2c_sched.o: ../global/resource.h ../scanparse/y.tab.h ../tree/traverse.h
icm2c_sched.o: compile.h ../tree/free.h icm_comment.c icm.data icm_trace.c
icm2c_std.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
icm2c_std.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
icm2c_std.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
icm2c_std.o: /usr/include/stdio_impl.h /usr/include/ctype.h
icm2c_std.o: /usr/include/iso/ctype_iso.h icm2c_basic.h icm2c_std.h
icm2c_std.o: ../global/types.h ../../inc/dbug.h /usr/include/stdlib.h
icm2c_std.o: /usr/include/iso/stdlib_iso.h ../global/my_debug.h
icm2c_std.o: ../print/convert.h ../global/globals.h ../print/print.h
icm2c_std.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
icm2c_std.o: /usr/include/string.h /usr/include/iso/string_iso.h
icm2c_std.o: ../global/internal_lib.h gen_startup_code.h icm_comment.c
icm2c_std.o: icm.data icm_trace.c
icm2c_utils.o: /usr/include/string.h /usr/include/iso/string_iso.h
icm2c_utils.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
icm2c_utils.o: ../global/globals.h /usr/include/stdio.h
icm2c_utils.o: /usr/include/iso/stdio_iso.h /usr/include/sys/va_list.h
icm2c_utils.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
icm2c_utils.o: ../global/types.h icm2c_utils.h ../global/NameTuples.h
icm2c_utils.o: ../../inc/dbug.h /usr/include/stdlib.h
icm2c_utils.o: /usr/include/iso/stdlib_iso.h
icm2c_wl.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
icm2c_wl.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
icm2c_wl.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
icm2c_wl.o: /usr/include/stdio_impl.h icm2c_basic.h ../../inc/dbug.h
icm2c_wl.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
icm2c_wl.o: ../global/my_debug.h ../print/convert.h ../global/types.h
icm2c_wl.o: ../global/globals.h ../print/print.h ../tree/tree_basic.h
icm2c_wl.o: ../tree/tree_compound.h ../global/Error.h /usr/include/string.h
icm2c_wl.o: /usr/include/iso/string_iso.h ../global/internal_lib.h
icm2c_wl.o: gen_startup_code.h icm_comment.c icm.data icm_trace.c
icm_betest.o: icm.data
icm_comment.o: icm.data
icm_icm2c.o: icm.data
icm_trace.o: icm.data
icm_vars.o: icm.data
precompile.o: /usr/include/string.h /usr/include/iso/string_iso.h
precompile.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
precompile.o: ../../inc/dbug.h /usr/include/stdio.h
precompile.o: /usr/include/iso/stdio_iso.h /usr/include/sys/va_list.h
precompile.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
precompile.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
precompile.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
precompile.o: ../global/Error.h ../global/globals.h ../global/internal_lib.h
precompile.o: ../tree/prf.h ../tree/free.h ../global/my_debug.h
precompile.o: ../tree/DupTree.h ../tree/LookUpTable.h ../tree/traverse.h
precompile.o: ../print/convert.h ../tree/DataFlowMask.h ../tree/adjust_ids.h
precompile.o: ../refcount/refcount.h ../c-interface/map_cwrapper.h
precompile.o: ../tree/scheduling.h compile.h precompile.h
wlpragma_funs.o: ../global/types.h ../tree/tree_basic.h
wlpragma_funs.o: ../tree/tree_compound.h ../global/Error.h
wlpragma_funs.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
wlpragma_funs.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
wlpragma_funs.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
wlpragma_funs.o: /usr/include/stdio_impl.h /usr/include/string.h
wlpragma_funs.o: /usr/include/iso/string_iso.h /usr/include/stdlib.h
wlpragma_funs.o: /usr/include/iso/stdlib_iso.h ../global/globals.h
wlpragma_funs.o: ../global/internal_lib.h ../../inc/dbug.h ../tree/free.h
wlpragma_funs.o: ../global/my_debug.h ../tree/DupTree.h ../tree/LookUpTable.h
wlpragma_funs.o: ../global/resource.h wltransform.h ../tree/scheduling.h
wlpragma_funs.o: wlpragma_funs.h wlpragma_funs.mac
wltransform.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
wltransform.o: ../global/Error.h /usr/include/stdio.h
wltransform.o: /usr/include/iso/stdio_iso.h /usr/include/sys/feature_tests.h
wltransform.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
wltransform.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
wltransform.o: /usr/include/string.h /usr/include/iso/string_iso.h
wltransform.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
wltransform.o: ../global/globals.h ../global/internal_lib.h
wltransform.o: ../tree/traverse.h ../tree/free.h ../../inc/dbug.h
wltransform.o: ../global/my_debug.h ../tree/DupTree.h ../tree/LookUpTable.h
wltransform.o: ../tree/DataFlowMask.h ../print/print.h ../tree/wl_bounds.h
wltransform.o: wlpragma_funs.h wlpragma_funs.mac wltransform.h
