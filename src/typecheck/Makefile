#
#
# $Log$
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
# Revision 2.25  2000/11/02 14:37:04  dkr
# explicit .mac dependencies added
#
# Revision 2.22  2000/07/24 14:46:00  dkr
# suffix rule for .c.o added
#
# Revision 2.21  2000/07/21 14:44:16  nmw
# include path c-interface added
#
# Revision 2.14  2000/03/16 14:16:28  dkr
# target typecheck.h added
#
# Revision 2.11  2000/01/21 12:03:53  dkr
# directory tree added
#
# Revision 2.9  2000/01/17 18:37:49  cg
# Handling of target platform unified: One of the following
# is always defined:
# SAC_FOR_SOLARIS_SPARC, SAC_FOR_LINUX_X86, OR SAC_FOR_OSF_ALPHA.
#
# Revision 2.7  1999/10/20 13:02:06  sbs
# new lines for constants / new-Types added.
#
# Revision 2.6  1999/10/04 08:35:14  sbs
# ilne for new_typecheck.o added.
#
# Revision 2.5  1999/09/28 14:44:55  sbs
# line for user_types.o added.
#
# Revision 2.4  1999/05/12 08:57:55  jhs
# -DNEWTREE eliminated from CFLAGS.
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
     -I../compile -I../c-interface

all: typecheck.o prim_fun.o typecheck_WL.o gen_pseudo_fun.o \
     new_typecheck.o new_types.o shape.o user_types.o \
     constants.o cv2scalar.o cv2cv.o


#
# the *.[ch] objects are to fool makedepend which does not
# recognize the dependency between *.[ch] and the .mac files!
#
typecheck.h: cmp_type.mac
	touch typecheck.h

typecheck.c: prim_fun_tt.mac cmp_type.mac
	touch typecheck.c

prim_fun.c: prim_fun_ft.mac
	touch prim_fun.c

cv2cv.c: ../tree/type_info.mac
	touch cv2cv.c

cv2scalar.c: ../tree/type_info.mac
	touch cv2scalar.c

new_types.c: type_constructor_info.mac
	touch new_types.c


%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o $@ -c $< $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak
	$(RM) -r .sb SunWS_cache

# DO NOT DELETE THIS LINE -- make depend depends on it.

constants.o: ../global/globals.h /usr/include/stdio.h
constants.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
constants.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
constants.o: /usr/include/stdio_impl.h ../global/types.h ../tree/free.h
constants.o: /usr/include/stdlib.h ../../inc/dbug.h ../global/my_debug.h
constants.o: ../global/internal_lib.h shape.h cv2scalar.h cv2cv.h
constants.o: ../tree/tree_compound.h ../tree/tree_basic.h ../global/Error.h
constants.o: /usr/include/string.h constants.h
cv2cv.o: ../../inc/dbug.h /usr/include/stdio.h
cv2cv.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
cv2cv.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
cv2cv.o: /usr/include/stdio_impl.h cv2cv.h ../tree/type_info.mac
cv2scalar.o: ../tree/tree_basic.h ../global/types.h ../../inc/dbug.h
cv2scalar.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
cv2scalar.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
cv2scalar.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h cv2scalar.h
cv2scalar.o: ../tree/type_info.mac
gen_pseudo_fun.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
gen_pseudo_fun.o: /usr/include/sys/isa_defs.h ../../inc/dbug.h
gen_pseudo_fun.o: /usr/include/stdio.h /usr/include/sys/va_list.h
gen_pseudo_fun.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
gen_pseudo_fun.o: ../global/types.h ../tree/prf.h ../tree/tree_basic.h
gen_pseudo_fun.o: ../tree/tree.h ../tree/tree_compound.h ../global/Error.h
gen_pseudo_fun.o: /usr/include/string.h ../global/globals.h
gen_pseudo_fun.o: ../global/internal_lib.h ../tree/free.h
gen_pseudo_fun.o: ../global/my_debug.h ../tree/DupTree.h
gen_pseudo_fun.o: ../tree/LookUpTable.h gen_pseudo_fun.h typecheck.h
new_typecheck.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
new_typecheck.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
new_typecheck.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
new_typecheck.o: /usr/include/string.h ../../inc/dbug.h ../tree/tree.h
new_typecheck.o: ../global/types.h ../tree/tree_basic.h
new_typecheck.o: ../tree/tree_compound.h ../global/Error.h
new_typecheck.o: /usr/include/stdlib.h ../global/globals.h
new_typecheck.o: ../global/internal_lib.h ../tree/traverse.h user_types.h
new_typecheck.o: new_types.h ../tree/free.h ../global/my_debug.h shape.h
new_typecheck.o: new_typecheck.h
new_types.o: ../../inc/dbug.h /usr/include/stdio.h
new_types.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
new_types.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
new_types.o: /usr/include/stdio_impl.h ../tree/free.h /usr/include/stdlib.h
new_types.o: ../global/my_debug.h ../global/globals.h ../global/types.h
new_types.o: ../global/internal_lib.h ../tree/tree.h ../tree/tree_basic.h
new_types.o: ../tree/tree_compound.h ../global/Error.h /usr/include/string.h
new_types.o: ../print/convert.h shape.h type_constructor_info.mac new_types.h
new_types.o: user_types.h
prim_fun.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
prim_fun.o: /usr/include/sys/isa_defs.h ../global/globals.h
prim_fun.o: /usr/include/stdio.h /usr/include/sys/va_list.h
prim_fun.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
prim_fun.o: ../global/types.h ../tree/tree.h ../tree/tree_basic.h
prim_fun.o: ../tree/tree_compound.h ../global/Error.h /usr/include/string.h
prim_fun.o: ../global/internal_lib.h prim_fun.h ../../inc/dbug.h
prim_fun.o: ../global/my_debug.h ../tree/DupTree.h ../tree/LookUpTable.h
prim_fun.o: ../tree/access_macros.h ../print/convert.h ../tree/free.h
prim_fun.o: typecheck.h prim_fun_tt.mac prim_fun_ft.mac
shape.o: /usr/include/stdarg.h /usr/include/varargs.h
shape.o: /usr/include/sys/va_list.h ../../inc/dbug.h /usr/include/stdio.h
shape.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
shape.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
shape.o: ../tree/tree_basic.h ../global/types.h ../tree/free.h
shape.o: /usr/include/stdlib.h ../global/my_debug.h ../global/globals.h
shape.o: ../global/internal_lib.h shape.h
typecheck.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
typecheck.o: /usr/include/sys/isa_defs.h /usr/include/stddef.h
typecheck.o: /usr/include/string.h /usr/include/limits.h /usr/include/float.h
typecheck.o: ../tree/tree.h ../global/types.h ../tree/tree_basic.h
typecheck.o: ../tree/tree_compound.h ../global/Error.h /usr/include/stdio.h
typecheck.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
typecheck.o: /usr/include/stdio_impl.h ../global/globals.h
typecheck.o: ../global/internal_lib.h ../print/print.h ../global/my_debug.h
typecheck.o: ../../inc/dbug.h ../tree/traverse.h typecheck.h
typecheck.o: ../print/convert.h ../modules/import.h prim_fun.h ../tree/free.h
typecheck.o: ../tree/access_macros.h ../tree/DupTree.h ../tree/LookUpTable.h
typecheck.o: typecheck_WL.h gen_pseudo_fun.h
typecheck.o: ../c-interface/import_specialization.h new_typecheck.h
typecheck.o: prim_fun_tt.mac
typecheck_WL.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
typecheck_WL.o: /usr/include/sys/isa_defs.h /usr/include/stddef.h
typecheck_WL.o: /usr/include/string.h ../tree/tree.h ../global/types.h
typecheck_WL.o: ../tree/tree_basic.h ../tree/tree_compound.h
typecheck_WL.o: ../global/Error.h /usr/include/stdio.h
typecheck_WL.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
typecheck_WL.o: /usr/include/stdio_impl.h ../global/globals.h
typecheck_WL.o: ../global/internal_lib.h ../print/print.h
typecheck_WL.o: ../global/my_debug.h ../../inc/dbug.h ../tree/traverse.h
typecheck_WL.o: ../tree/free.h ../optimize/ConstantFolding.h constants.h
typecheck_WL.o: shape.h typecheck.h
user_types.o: /usr/include/string.h /usr/include/sys/feature_tests.h
user_types.o: /usr/include/sys/isa_defs.h ../global/Error.h
user_types.o: /usr/include/stdio.h /usr/include/sys/va_list.h
user_types.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
user_types.o: /usr/include/stdlib.h ../global/types.h ../global/globals.h
user_types.o: ../../inc/dbug.h ../tree/free.h ../global/my_debug.h
user_types.o: ../global/internal_lib.h user_types.h new_types.h shape.h
