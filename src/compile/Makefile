#
# $Log$
# Revision 1.34  1998/03/04 00:09:36  dkr
# 'clean' now deletes the subdir .sb, too
#
# Revision 1.33  1998/02/16 21:44:01  dkr
# make deps done
#
# Revision 1.32  1998/02/12 16:58:29  dkr
# added ../optimize in INCS
#
# Revision 1.31  1997/11/20 19:19:17  dkr
# make clean now removes Old2NewWith.o
#
# Revision 1.30  1997/11/20 15:02:09  dkr
# make deps done
#
# Revision 1.29  1997/11/12 15:59:49  sbs
# CC+CCFLAGS handling changed.
# Now, it suffices to preset CC=cc if the SUN-compiler isdesired
# Otherwise, gcc is set with -Wall and -ansi
#
# Revision 1.28  1997/11/11 16:49:30  sbs
# make deps done
#
# Revision 1.27  1997/11/03 15:13:15  dkr
# make deps done
#
# Revision 1.26  1997/08/05 11:31:00  sbs
# profile.o added in clean-line
#
# Revision 1.25  1997/05/28 12:48:31  sbs
# make deps done
#
# Revision 1.24  1997/05/28  12:35:25  sbs
# Profiling integrated
#
# Revision 1.23  1997/05/14  08:54:07  sbs
# deps made
#
# Revision 1.22  1997/05/14  06:44:41  sbs
# analyse.o inserted
#
# Revision 1.21  1997/04/25  12:04:10  sbs
# make deps done
#
# Revision 1.20  1996/02/16  09:46:54  sbs
# make deps done
#
# Revision 1.19  1996/01/18  13:05:43  asi
# make deps done
#
# Revision 1.18  1996/01/09  09:19:14  cg
# added compilation of libsac.c
#
# Revision 1.17  1996/01/04  18:11:10  cg
# make deps done
#
# Revision 1.16  1995/12/15  10:41:12  cg
# make deps done
#
# Revision 1.15  1995/12/01  17:25:42  cg
# added compilation of precompile.c
#
# Revision 1.14  1995/10/08  11:29:56  sbs
# make deps done
#
# Revision 1.13  1995/07/07  15:47:53  asi
# make deps done
#
# Revision 1.12  1995/05/26  12:12:30  asi
# make deps done
#
# Revision 1.11  1995/05/15  13:38:54  asi
# make deps done
#
# Revision 1.10  1995/05/04  11:46:29  sbs
# make clean adjusted
#
# Revision 1.9  1995/05/04  11:44:35  sbs
# new deps made
#
# Revision 1.8  1995/04/20  10:21:23  asi
# make deps done
#
# Revision 1.7  1995/04/19  13:15:55  sbs
# make deps done
#
# Revision 1.6  1995/04/11  15:02:06  sbs
# dbug.o added in BEtest-rule
#
# Revision 1.5  1995/04/05  15:55:01  asi
# make deps done
#
# Revision 1.4  1995/04/04  11:03:54  sbs
# make deps done
#
# Revision 1.3  1995/04/03  13:59:44  sbs
# icm2c.o line included
#
# Revision 1.2  1995/03/31  14:05:38  sbs
# BEtest inserted
#
# Revision 1.1  1995/03/29  12:12:52  hw
# Initial revision
#
#
# 


CC        := gcc
CCFLAGS   := -ansi -Wall -g
CFLAGS    := -D__EXTENSIONS__

INCS=-I../global -I../../inc -I../print -I../typecheck -I../refcount -I../optimize
MACS=icm.data icm_decl.c icm_args.c icm_comment.c icm_trace.c

all: compile.o icm2c.o precompile.o libsac.a profile.o Old2NewWith.o

BEtest: icm2c.c $(MACS)
	$(CC) $(CCFLAGS) $(CFLAGS) -o BEtest icm2c.c -DTEST_BACKEND $(INCS) ../../lib/dbug.o

compile.o: compile.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o compile.o -c compile.c $(INCS)

icm2c.o: icm2c.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o icm2c.o -c icm2c.c $(INCS)

profile.o: profile.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o profile.o -c profile.c $(INCS)

precompile.o: precompile.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o precompile.o -c precompile.c $(INCS)

libsac.a: libsac.o
	ar rc libsac.a libsac.o
	ranlib libsac.a

libsac.o: libsac.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o libsac.o -c libsac.c $(INCS)

Old2NewWith.o: Old2NewWith.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o Old2NewWith.o -c Old2NewWith.c $(INCS)


deps:
	makedepend $(INCS) *.c

clean:
	$(RM) profile.o compile.o icm2c.o precompile.o libsac.a libsac.o Old2NewWith.o \
              *.bak BEtest *.sib *.sac
	$(RM) -r .sb

# DO NOT DELETE THIS LINE -- make depend depends on it.

Old2NewWith.o: ../global/tree.h ../global/types.h ../global/tree_basic.h
Old2NewWith.o: ../global/tree_compound.h ../global/Error.h
Old2NewWith.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
Old2NewWith.o: /usr/include/sys/va_list.h /usr/include/string.h
Old2NewWith.o: /usr/include/stdlib.h ../global/globals.h
Old2NewWith.o: ../global/internal_lib.h ../global/traverse.h
Old2NewWith.o: ../typecheck/typecheck.h ../global/free.h ../../inc/dbug.h
Old2NewWith.o: ../global/my_debug.h /usr/include/malloc.h
Old2NewWith.o: ../optimize/DupTree.h
compile.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
compile.o: /usr/include/string.h ../global/types.h ../global/tree_basic.h
compile.o: ../global/tree_compound.h ../global/Error.h /usr/include/stdio.h
compile.o: /usr/include/sys/va_list.h ../global/globals.h ../print/print.h
compile.o: ../global/tree.h ../global/internal_lib.h ../../inc/dbug.h
compile.o: ../global/my_debug.h ../global/traverse.h compile.h
compile.o: ../print/convert.h ../optimize/DupTree.h ../refcount/refcount.h
compile.o: ../typecheck/typecheck.h ../global/free.h /usr/include/malloc.h
compile.o: ../global/type_info.mac
icm2c.o: /usr/include/malloc.h /usr/include/stdio.h
icm2c.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
icm2c.o: ../print/print.h ../global/tree.h ../global/types.h
icm2c.o: ../global/tree_basic.h ../global/tree_compound.h ../global/Error.h
icm2c.o: /usr/include/string.h /usr/include/stdlib.h ../global/globals.h
icm2c.o: ../global/internal_lib.h ../../inc/dbug.h ../global/my_debug.h
icm2c.o: ../../inc/main.h trace.h ../print/convert.h icm2c.h libsac.h
icm2c.o: icm_decl.c icm.data icm_args.c icm_comment.c icm_trace.c
icm_args.o: icm.data
icm_comment.o: icm.data
icm_decl.o: icm.data
icm_trace.o: icm.data
libsac.o: /usr/include/sys/time.h /usr/include/sys/feature_tests.h
libsac.o: /usr/include/sys/types.h /usr/include/sys/isa_defs.h
libsac.o: /usr/include/sys/machtypes.h /usr/include/sys/resource.h
libsac.o: /usr/include/stdio.h /usr/include/sys/va_list.h
libsac.o: /usr/include/stdarg.h /usr/include/varargs.h /usr/include/malloc.h
libsac.o: profile.h
precompile.o: ../global/types.h ../global/tree_basic.h
precompile.o: ../global/tree_compound.h ../global/Error.h
precompile.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
precompile.o: /usr/include/sys/va_list.h /usr/include/string.h
precompile.o: /usr/include/stdlib.h ../global/globals.h ../global/free.h
precompile.o: ../../inc/dbug.h ../global/my_debug.h /usr/include/malloc.h
precompile.o: ../global/internal_lib.h ../print/convert.h
precompile.o: ../global/traverse.h ../refcount/refcount.h
profile.o: ../../inc/dbug.h /usr/include/stdio.h
profile.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
profile.o: ../global/globals.h ../global/types.h profile.h
