#
# $Log$
# Revision 3.5  2001/03/09 11:15:11  sbs
# -I ../profile/ added
#
# Revision 3.4  2001/03/02 10:36:02  dkr
# /home/dkr/sac/BASE/sac2c/src/global/RCS/Makefile,v used in touch-rules
#
# Revision 3.3  2001/01/17 10:13:00  dkr
# make deps done
#
# Revision 3.2  2000/12/06 20:08:59  dkr
# make deps done
#
# Revision 3.1  2000/11/20 17:59:26  sacbase
# new release made
#
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
     -I../psi-opt -I../objects -I../concurrent -I../multithread -I../profile

all: main.o Error.o usage.o my_debug.o internal_lib.o globals.o resource.o types.h \
     build.o interrupt.o options.o NameTuples.o

#
# the *.[ch] objects are to fool makedepend which does not
# recognize the dependency between *.[ch] and the .mac files!
#
types.h: ../tree/type_info.mac ../global/phase_info.mac ../global/status_info.mac \
         ../tree/node_info.mac ../tree/prf_node_info.mac
	touch $@

NameTuples.h: ../global/nt_info.mac
	touch $@

NameTuples.c: ../global/nt_info.mac
	touch $@

globals.c: ../tree/type_info.mac ../global/phase_info.mac
	touch $@

my_debug.c:  ../tree/node_info.mac ../tree/prf_node_info.mac ../tree/type_info.mac \
             ../global/status_info.mac
	touch $@


build.c: 
	echo "char build_date[] = \"`date`\";"  >  build.c
	echo "char build_user[] = \"$(USER)\";" >> build.c
	echo "char build_host[] = \"$(HOST)\";" >> build.c
	echo "char build_os[]   = \"$(OS)\";"   >> build.c
	cat build.c

build.o: build.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o build.o -c build.c $(INCS)
	$(RM) build.c

%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o $@ -c $< $(INCS)

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
Error.o: globals.h ../tree/tree_basic.h ../../inc/dbug.h internal_lib.h
Error.o: ../modules/filemgr.h resource.h
NameTuples.o: ../../inc/dbug.h /usr/include/stdio.h
NameTuples.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
NameTuples.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
NameTuples.o: /usr/include/stdio_impl.h types.h ../tree/tree_basic.h
NameTuples.o: ../tree/tree_compound.h Error.h /usr/include/string.h
NameTuples.o: /usr/include/stdlib.h globals.h NameTuples.h
globals.o: ../modules/filemgr.h /usr/include/stdio.h
globals.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
globals.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
globals.o: /usr/include/stdio_impl.h types.h globals.h Error.h
globals.o: /usr/include/string.h /usr/include/stdlib.h internal_lib.h
internal_lib.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
internal_lib.o: /usr/include/sys/isa_defs.h /usr/include/math.h
internal_lib.o: /usr/include/string.h /usr/include/stdarg.h
internal_lib.o: /usr/include/varargs.h /usr/include/sys/va_list.h
internal_lib.o: /usr/include/ctype.h Error.h /usr/include/stdio.h
internal_lib.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h types.h
internal_lib.o: globals.h ../tree/free.h ../../inc/dbug.h my_debug.h
internal_lib.o: internal_lib.h ../tree/traverse.h ../tree/tree_basic.h
internal_lib.o: ../tree/tree_compound.h
interrupt.o: /usr/include/signal.h /usr/include/sys/feature_tests.h
interrupt.o: /usr/include/sys/isa_defs.h /usr/include/sys/signal.h
interrupt.o: /usr/include/sys/unistd.h /usr/include/stdio.h
interrupt.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
interrupt.o: /usr/include/stdio_impl.h Error.h /usr/include/string.h
interrupt.o: /usr/include/stdlib.h types.h globals.h interrupt.h
interrupt.o: internal_lib.h build.h
main.o: ../tree/tree.h types.h ../tree/tree_basic.h ../tree/tree_compound.h
main.o: Error.h /usr/include/stdio.h /usr/include/sys/feature_tests.h
main.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
main.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
main.o: /usr/include/string.h /usr/include/stdlib.h globals.h internal_lib.h
main.o: ../tree/free.h ../../inc/dbug.h my_debug.h usage.h
main.o: ../flatten/lac2fun.h ../flatten/fun2lac.h ../flatten/flatten.h
main.o: ../print/print.h ../typecheck/typecheck.h ../optimize/optimize.h
main.o: ../modules/filemgr.h ../modules/import.h ../refcount/refcount.h
main.o: ../scanparse/scnprs.h resource.h ../scanparse/y.tab.h
main.o: ../modules/writesib.h ../modules/readsib.h ../modules/implicittypes.h
main.o: ../objects/objinit.h ../modules/analysis.h ../modules/checkdec.h
main.o: ../objects/objects.h ../objects/uniquecheck.h ../objects/rmvoidfun.h
main.o: ../compile/wltransform.h ../concurrent/concurrent.h
main.o: ../compile/precompile.h ../compile/compile.h ../modules/cccall.h
main.o: ../compile/PatchWith.h interrupt.h options.h
main.o: ../multithread/multithread.h
options.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
options.o: /usr/include/sys/isa_defs.h ../../inc/dbug.h /usr/include/stdio.h
options.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
options.o: /usr/include/stdio_impl.h ../../inc/main_args.h
options.o: /usr/include/string.h usage.h ../modules/filemgr.h types.h
options.o: globals.h internal_lib.h ../tree/free.h my_debug.h Error.h
resource.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
resource.o: /usr/include/sys/isa_defs.h /usr/include/stdio.h
resource.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
resource.o: /usr/include/stdio_impl.h /usr/include/string.h types.h
resource.o: ../tree/tree_basic.h ../../inc/dbug.h resource.h
resource.o: ../scanparse/scnprs.h globals.h ../scanparse/y.tab.h
resource.o: ../modules/filemgr.h ../tree/free.h my_debug.h internal_lib.h
resource.o: Error.h
usage.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
usage.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
usage.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
usage.o: /usr/include/stdlib.h /usr/include/string.h usage.h globals.h
usage.o: types.h build.h ../../inc/dbug.h
