#
#
# $Log$
# Revision 3.12  2001/11/13 20:32:42  dkr
# make deps done
#
# Revision 3.11  2001/07/18 13:08:17  cg
# make deps done
#
# Revision 3.10  2001/04/26 21:09:43  dkr
# make deps done
#
# Revision 3.9  2001/04/12 15:40:33  sbs
# force -O0 when compiling prim_fun.c on DEC_ALPHA...
# Otherwise, the product version yields strange type errors
# for arrays.sac from the demos.....
#
# Revision 3.8  2001/03/02 14:37:41  sbs
# constants move to ../constants
#
# Revision 3.7  2001/03/02 10:36:44  dkr
# /home/dkr/sac/BASE/sac2c/src/typecheck/RCS/Makefile,v used in touch-rules
#
# Revision 3.6  2001/02/23 18:04:22  sbs
# extended for negative take's and drop's in genarray
#
# Revision 3.5  2001/02/02 10:39:01  dkr
# make deps done
#
# Revision 3.4  2001/01/17 10:13:02  dkr
# make deps done
#
# Revision 3.3  2000/12/12 15:09:05  dkr
# make deps done
#
# Revision 3.2  2000/12/06 20:09:01  dkr
# make deps done
#
# Revision 3.1  2000/11/20 18:00:00  sacbase
# new release made
#
# ... [ eliminated ]
#
# Revision 1.1  1994/12/02  10:13:42  hw
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

RM=rm -f
INCS=-I../global -I../tree -I../../inc -I../print -I../modules -I../optimize \
     -I../compile -I../c-interface -I../constants

all: typecheck.o prim_fun.o typecheck_WL.o gen_pseudo_fun.o \
     new_typecheck.o new_types.o user_types.o


#
# the *.[ch] objects are to fool makedepend which does not
# recognize the dependency between *.[ch] and the .mac files!
#
typecheck.h: cmp_type.mac
	touch $@

typecheck.c: prim_fun_tt.mac cmp_type.mac
	touch $@

prim_fun.c: prim_fun_ft.mac
	touch $@

new_types.c: type_constructor_info.mac
	touch $@


%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o $@ -c $< $(INCS)

ifeq ($(OS),OSF_ALPHA)

prim_fun.o: prim_fun.c
	$(CC) $(CCFLAGS) $(CFLAGS) -O0 -o $@ -c $< $(INCS)

endif


deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak
	$(RM) -r .sb SunWS_cache

# DO NOT DELETE THIS LINE -- make depend depends on it.

gen_pseudo_fun.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
gen_pseudo_fun.o: /usr/include/sys/feature_tests.h
gen_pseudo_fun.o: /usr/include/sys/isa_defs.h ../global/types.h
gen_pseudo_fun.o: ../tree/tree_basic.h ../tree/tree_compound.h
gen_pseudo_fun.o: ../global/Error.h /usr/include/stdio.h
gen_pseudo_fun.o: /usr/include/iso/stdio_iso.h /usr/include/sys/va_list.h
gen_pseudo_fun.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
gen_pseudo_fun.o: /usr/include/string.h /usr/include/iso/string_iso.h
gen_pseudo_fun.o: ../global/globals.h ../global/internal_lib.h
gen_pseudo_fun.o: ../../inc/dbug.h ../tree/prf.h ../tree/free.h
gen_pseudo_fun.o: ../global/my_debug.h ../tree/DupTree.h
gen_pseudo_fun.o: ../tree/LookUpTable.h gen_pseudo_fun.h typecheck.h
new_typecheck.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
new_typecheck.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
new_typecheck.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
new_typecheck.o: /usr/include/stdio_impl.h /usr/include/string.h
new_typecheck.o: /usr/include/iso/string_iso.h ../../inc/dbug.h
new_typecheck.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
new_typecheck.o: ../global/types.h ../tree/tree_basic.h
new_typecheck.o: ../tree/tree_compound.h ../global/Error.h
new_typecheck.o: ../global/globals.h ../global/internal_lib.h
new_typecheck.o: ../tree/traverse.h user_types.h new_types.h ../tree/free.h
new_typecheck.o: ../global/my_debug.h ../constants/shape.h new_typecheck.h
new_types.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
new_types.o: ../global/Error.h /usr/include/stdio.h
new_types.o: /usr/include/iso/stdio_iso.h /usr/include/sys/feature_tests.h
new_types.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
new_types.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
new_types.o: /usr/include/string.h /usr/include/iso/string_iso.h
new_types.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
new_types.o: ../global/globals.h ../global/internal_lib.h ../../inc/dbug.h
new_types.o: ../tree/free.h ../global/my_debug.h ../print/convert.h
new_types.o: ../constants/shape.h type_constructor_info.mac new_types.h
new_types.o: user_types.h
prim_fun.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
prim_fun.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
prim_fun.o: ../global/types.h ../tree/tree_basic.h ../tree/tree_compound.h
prim_fun.o: ../global/Error.h /usr/include/stdio.h
prim_fun.o: /usr/include/iso/stdio_iso.h /usr/include/sys/va_list.h
prim_fun.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
prim_fun.o: /usr/include/string.h /usr/include/iso/string_iso.h
prim_fun.o: ../global/globals.h ../global/internal_lib.h prim_fun.h
prim_fun.o: ../../inc/dbug.h ../global/my_debug.h ../tree/DupTree.h
prim_fun.o: ../tree/LookUpTable.h ../print/convert.h ../tree/free.h
prim_fun.o: typecheck.h prim_fun_tt.mac prim_fun_ft.mac
typecheck.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
typecheck.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
typecheck.o: /usr/include/stddef.h /usr/include/iso/stddef_iso.h
typecheck.o: /usr/include/string.h /usr/include/iso/string_iso.h
typecheck.o: /usr/include/limits.h /usr/include/iso/limits_iso.h
typecheck.o: /usr/include/float.h ../global/types.h ../tree/tree_basic.h
typecheck.o: ../tree/tree_compound.h ../global/Error.h /usr/include/stdio.h
typecheck.o: /usr/include/iso/stdio_iso.h /usr/include/sys/va_list.h
typecheck.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
typecheck.o: ../global/globals.h ../global/internal_lib.h ../print/print.h
typecheck.o: ../global/my_debug.h ../../inc/dbug.h ../tree/traverse.h
typecheck.o: typecheck.h ../optimize/LoopInvariantRemoval.h
typecheck.o: ../print/convert.h ../modules/import.h prim_fun.h ../tree/free.h
typecheck.o: ../tree/DupTree.h ../tree/LookUpTable.h typecheck_WL.h
typecheck.o: gen_pseudo_fun.h ../c-interface/import_specialization.h
typecheck.o: new_typecheck.h prim_fun_tt.mac
typecheck_WL.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
typecheck_WL.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
typecheck_WL.o: /usr/include/stddef.h /usr/include/iso/stddef_iso.h
typecheck_WL.o: /usr/include/string.h /usr/include/iso/string_iso.h
typecheck_WL.o: ../global/types.h ../tree/tree_basic.h
typecheck_WL.o: ../tree/tree_compound.h ../global/Error.h
typecheck_WL.o: /usr/include/stdio.h /usr/include/iso/stdio_iso.h
typecheck_WL.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
typecheck_WL.o: /usr/include/stdio_impl.h ../global/globals.h
typecheck_WL.o: ../global/internal_lib.h ../print/print.h
typecheck_WL.o: ../global/my_debug.h ../../inc/dbug.h ../tree/traverse.h
typecheck_WL.o: ../tree/free.h ../optimize/ConstantFolding.h
typecheck_WL.o: ../constants/constants.h ../constants/shape.h typecheck.h
user_types.o: /usr/include/string.h /usr/include/iso/string_iso.h
user_types.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
user_types.o: ../global/Error.h /usr/include/stdio.h
user_types.o: /usr/include/iso/stdio_iso.h /usr/include/sys/va_list.h
user_types.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
user_types.o: /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h
user_types.o: ../global/types.h ../global/globals.h ../../inc/dbug.h
user_types.o: ../tree/free.h ../global/my_debug.h ../global/internal_lib.h
user_types.o: user_types.h new_types.h ../constants/shape.h
