#
# $Log$
# Revision 2.24  2000/05/29 14:35:09  dkr
# make deps done
#
# Revision 2.23  2000/03/31 09:33:40  jhs
# make deps done
#
# Revision 2.22  2000/03/31 09:22:12  cg
# make deps done
#
# Revision 2.21  2000/03/23 14:00:04  jhs
# Added build_os.
#
# Revision 2.20  2000/03/16 14:32:41  dkr
# make deps done
#
# Revision 2.19  2000/03/16 14:16:43  dkr
# target types.h updated
#
# Revision 2.18  2000/03/01 14:52:32  dkr
# make deps done
#
# Revision 2.17  2000/01/23 15:01:10  dkr
# make deps done
#
# Revision 2.16  2000/01/21 16:46:12  dkr
# tree related stuff moved from directory 'global' into 'tree'
#
# Revision 2.15  2000/01/21 14:27:04  jhs
# Makedeps ...
#
# Revision 2.14  2000/01/21 13:19:53  jhs
# Added new mt ... infrastructure expanded ...
#
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
     -I../psi-opt -I../objects -I../concurrent -I../Old2NewWith \
     -I../multithread

all: main.o Error.o usage.o my_debug.o internal_lib.o globals.o resource.o types.h \
     build.o interrupt.o options.o

#
# the types.h object is to fool makedepend which does not
# recognize the dependency between types.h and the .mac files!
#
types.h: ../global/nt_info.mac ../global/phase_info.mac ../global/status_info.mac ../tree/node_info.mac ../tree/prf_node_info.mac ../tree/type_info.mac
	touch types.h

interrupt.o: interrupt.c 
	$(CC) $(CCFLAGS) $(CFLAGS) -o interrupt.o -c interrupt.c $(INCS)

main.o: main.c 
	$(CC) $(CCFLAGS) $(CFLAGS) -o main.o -c main.c $(INCS)

Error.o: Error.c 
	$(CC) $(CCFLAGS) $(CFLAGS) -o Error.o -c Error.c $(INCS) 

usage.o: usage.c usage.h 
	$(CC) $(CCFLAGS) $(CFLAGS) -o usage.o -c usage.c $(INCS)

my_debug.o: my_debug.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o my_debug.o -c my_debug.c $(INCS)

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
	echo "char build_os[]   = \"$(OS)\";"   >> build.c
	cat build.c

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak *~
	$(RM) -r .sb SunWS_cache

# DO NOT DELETE THIS LINE -- make depend depends on it.

Error.o: /usr/include/stdarg.h /usr/include/varargs.h
Error.o: /usr/include/sys/va_list.h /usr/include/stdio.h
Error.o: /usr/include/sys/feature_tests.h /usr/include/string.h Error.h
Error.o: /usr/include/stdlib.h types.h globals.h ../tree/tree_basic.h
Error.o: ../../inc/dbug.h internal_lib.h ../modules/filemgr.h resource.h
globals.o: ../modules/filemgr.h /usr/include/stdio.h
globals.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
globals.o: types.h globals.h Error.h /usr/include/string.h
globals.o: /usr/include/stdlib.h ../tree/type_info.mac
internal_lib.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
internal_lib.o: /usr/include/math.h /usr/include/string.h
internal_lib.o: /usr/include/stdarg.h /usr/include/varargs.h
internal_lib.o: /usr/include/sys/va_list.h /usr/include/ctype.h Error.h
internal_lib.o: /usr/include/stdio.h types.h globals.h ../tree/free.h
internal_lib.o: ../../inc/dbug.h my_debug.h /usr/include/malloc.h
internal_lib.o: internal_lib.h ../tree/traverse.h ../tree/tree_basic.h
internal_lib.o: ../tree/tree_compound.h
interrupt.o: /usr/include/signal.h /usr/include/sys/feature_tests.h
interrupt.o: /usr/include/sys/signal.h /usr/include/sys/unistd.h
interrupt.o: /usr/include/stdio.h /usr/include/sys/va_list.h Error.h
interrupt.o: /usr/include/string.h /usr/include/stdlib.h types.h globals.h
interrupt.o: interrupt.h internal_lib.h build.h
main.o: ../tree/tree.h types.h ../tree/tree_basic.h ../tree/tree_compound.h
main.o: Error.h /usr/include/stdio.h /usr/include/sys/feature_tests.h
main.o: /usr/include/sys/va_list.h /usr/include/string.h
main.o: /usr/include/stdlib.h globals.h internal_lib.h ../tree/free.h
main.o: ../../inc/dbug.h my_debug.h /usr/include/malloc.h usage.h
main.o: ../flatten/lac2fun.h ../flatten/fun2lac.h ../flatten/flatten.h
main.o: ../print/print.h ../typecheck/typecheck.h ../optimize/optimize.h
main.o: ../modules/filemgr.h ../modules/import.h ../refcount/refcount.h
main.o: ../scanparse/scnprs.h resource.h ../scanparse/y.tab.h
main.o: ../psi-opt/psi-opt.h ../modules/writesib.h ../modules/readsib.h
main.o: ../modules/implicittypes.h ../objects/objinit.h ../modules/analysis.h
main.o: ../modules/checkdec.h ../objects/objects.h ../objects/uniquecheck.h
main.o: ../objects/rmvoidfun.h ../compile/wltransform.h
main.o: ../concurrent/concurrent.h ../compile/precompile.h
main.o: ../compile/compile.h ../modules/cccall.h ../compile/Old2NewWith.h
main.o: interrupt.h options.h ../multithread/multithread.h
options.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
options.o: ../../inc/dbug.h /usr/include/stdio.h /usr/include/sys/va_list.h
options.o: ../../inc/main_args.h /usr/include/string.h usage.h
options.o: ../modules/filemgr.h globals.h types.h internal_lib.h
options.o: ../tree/free.h my_debug.h /usr/include/malloc.h Error.h
resource.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
resource.o: /usr/include/stdio.h /usr/include/sys/va_list.h
resource.o: /usr/include/string.h types.h ../tree/tree_basic.h
resource.o: ../../inc/dbug.h resource.h ../scanparse/scnprs.h globals.h
resource.o: ../scanparse/y.tab.h ../modules/filemgr.h ../tree/free.h
resource.o: my_debug.h /usr/include/malloc.h internal_lib.h Error.h
usage.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
usage.o: /usr/include/sys/va_list.h /usr/include/stdlib.h usage.h globals.h
usage.o: types.h build.h ../../inc/dbug.h
