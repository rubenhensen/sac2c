#
# $Log$
# Revision 2.11  2000/01/21 12:03:53  dkr
# directory tree added
#
# Revision 2.10  2000/01/17 18:40:57  cg
# make deps done
#
# Revision 2.9  2000/01/17 18:37:49  cg
# Handling of target platform unified: One of the following
# is always defined:
# SAC_FOR_SOLARIS_SPARC, SAC_FOR_LINUX_X86, OR SAC_FOR_OSF_ALPHA.
#
# Revision 2.8  1999/12/13 11:27:53  dkr
# *** empty log message ***
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
# Revision 2.3  1999/03/11 18:18:21  dkr
# make deps done
#
# Revision 2.2  1999/02/26 14:17:39  dkr
# make deps done
#
# Revision 2.1  1999/02/23 12:40:41  sacbase
# new release made
#
# ... [ eliminated ]
#
# Revision 1.1  1994/12/02  10:13:42  hw
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

RM=rm -f
INCS=-I../global -I../tree -I../../inc -I../print -I../modules -I../optimize -I../compile

all: typecheck.o prim_fun.o typecheck_WL.o gen_pseudo_fun.o \
     new_typecheck.o new_types.o shape.o user_types.o \
     constants.o cv2scalar.o cv2cv.o

new_typecheck.o: new_typecheck.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o new_typecheck.o -c new_typecheck.c $(INCS)

new_types.o: new_types.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o new_types.o -c new_types.c $(INCS)

constants.o: constants.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o constants.o -c constants.c $(INCS)

cv2cv.o: cv2cv.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o cv2cv.o -c cv2cv.c $(INCS)

cv2scalar.o: cv2scalar.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o cv2scalar.o -c cv2scalar.c $(INCS)

shape.o: shape.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o shape.o -c shape.c $(INCS)

user_types.o: user_types.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o user_types.o -c user_types.c $(INCS)

typecheck.o: typecheck.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o typecheck.o -c typecheck.c $(INCS)

prim_fun.o: prim_fun.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o prim_fun.o -c prim_fun.c $(INCS)

gen_pseudo_fun.o: gen_pseudo_fun.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o gen_pseudo_fun.o -c gen_pseudo_fun.c $(INCS)

typecheck_WL.o: typecheck_WL.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o typecheck_WL.o -c typecheck_WL.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o *.bak
	$(RM) -r .sb SunWS_cache

# DO NOT DELETE THIS LINE -- make depend depends on it.

constants.o: ../global/globals.h ../global/types.h /usr/include/stdio.h
constants.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
constants.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
constants.o: /usr/include/stdio_impl.h ../global/free.h ../../inc/dbug.h
constants.o: ../global/my_debug.h /usr/include/malloc.h
constants.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
constants.o: /usr/include/sys/int_types.h ../global/internal_lib.h shape.h
constants.o: cv2scalar.h cv2cv.h ../global/tree_compound.h
constants.o: ../global/tree_basic.h ../global/Error.h /usr/include/string.h
constants.o: /usr/include/stdlib.h constants.h
cv2cv.o: ../../inc/dbug.h /usr/include/stdio.h
cv2cv.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
cv2cv.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
cv2cv.o: /usr/include/stdio_impl.h cv2cv.h ../global/type_info.mac
cv2scalar.o: ../global/tree_basic.h ../global/types.h ../../inc/dbug.h
cv2scalar.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
cv2scalar.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
cv2scalar.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h cv2scalar.h
cv2scalar.o: ../global/type_info.mac
gen_pseudo_fun.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
gen_pseudo_fun.o: /usr/include/sys/isa_defs.h ../../inc/dbug.h
gen_pseudo_fun.o: /usr/include/stdio.h /usr/include/sys/va_list.h
gen_pseudo_fun.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
gen_pseudo_fun.o: ../global/types.h ../global/tree_basic.h ../global/tree.h
gen_pseudo_fun.o: ../global/tree_compound.h ../global/Error.h
gen_pseudo_fun.o: /usr/include/string.h ../global/globals.h
gen_pseudo_fun.o: ../global/internal_lib.h ../global/free.h
gen_pseudo_fun.o: ../global/my_debug.h /usr/include/malloc.h
gen_pseudo_fun.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
gen_pseudo_fun.o: /usr/include/sys/int_types.h typecheck.h gen_pseudo_fun.h
new_typecheck.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
new_typecheck.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
new_typecheck.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
new_typecheck.o: /usr/include/string.h ../../inc/dbug.h ../global/tree.h
new_typecheck.o: ../global/types.h ../global/tree_basic.h
new_typecheck.o: ../global/tree_compound.h ../global/Error.h
new_typecheck.o: /usr/include/stdlib.h ../global/globals.h
new_typecheck.o: ../global/internal_lib.h ../global/traverse.h user_types.h
new_typecheck.o: new_types.h ../global/free.h ../global/my_debug.h
new_typecheck.o: /usr/include/malloc.h /usr/include/sys/types.h
new_typecheck.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
new_typecheck.o: shape.h new_typecheck.h
new_types.o: ../../inc/dbug.h /usr/include/stdio.h
new_types.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
new_types.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
new_types.o: /usr/include/stdio_impl.h ../global/free.h ../global/my_debug.h
new_types.o: /usr/include/malloc.h /usr/include/sys/types.h
new_types.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
new_types.o: ../global/globals.h ../global/types.h ../global/internal_lib.h
new_types.o: ../global/tree.h ../global/tree_basic.h
new_types.o: ../global/tree_compound.h ../global/Error.h
new_types.o: /usr/include/string.h /usr/include/stdlib.h ../print/convert.h
new_types.o: shape.h type_constructor_info.mac new_types.h user_types.h
prim_fun.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
prim_fun.o: /usr/include/sys/isa_defs.h ../global/globals.h ../global/types.h
prim_fun.o: /usr/include/stdio.h /usr/include/sys/va_list.h
prim_fun.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
prim_fun.o: ../global/tree.h ../global/tree_basic.h ../global/tree_compound.h
prim_fun.o: ../global/Error.h /usr/include/string.h ../global/internal_lib.h
prim_fun.o: prim_fun.h ../../inc/dbug.h ../global/my_debug.h typecheck.h
prim_fun.o: ../global/access_macros.h ../print/convert.h ../global/free.h
prim_fun.o: /usr/include/malloc.h /usr/include/sys/types.h
prim_fun.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
prim_fun.o: prim_fun_tt.mac prim_fun_ft.mac
shape.o: ../../inc/dbug.h /usr/include/stdio.h
shape.o: /usr/include/sys/feature_tests.h /usr/include/sys/isa_defs.h
shape.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
shape.o: /usr/include/stdio_impl.h ../global/tree_basic.h ../global/types.h
shape.o: ../global/free.h ../global/my_debug.h /usr/include/malloc.h
shape.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
shape.o: /usr/include/sys/int_types.h ../global/globals.h
shape.o: ../global/internal_lib.h shape.h
typecheck.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
typecheck.o: /usr/include/sys/isa_defs.h /usr/include/stddef.h
typecheck.o: /usr/include/string.h /usr/include/limits.h
typecheck.o: /usr/include/values.h ../global/tree.h ../global/types.h
typecheck.o: ../global/tree_basic.h ../global/tree_compound.h
typecheck.o: ../global/Error.h /usr/include/stdio.h
typecheck.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
typecheck.o: /usr/include/stdio_impl.h ../global/globals.h
typecheck.o: ../global/internal_lib.h ../print/print.h ../global/my_debug.h
typecheck.o: ../../inc/dbug.h ../global/traverse.h typecheck.h
typecheck.o: ../print/convert.h ../modules/import.h prim_fun.h
typecheck.o: ../global/free.h /usr/include/malloc.h /usr/include/sys/types.h
typecheck.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
typecheck.o: ../global/access_macros.h ../tree/DupTree.h typecheck_WL.h
typecheck.o: gen_pseudo_fun.h new_typecheck.h prim_fun_tt.mac
typecheck_WL.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
typecheck_WL.o: /usr/include/sys/isa_defs.h /usr/include/stddef.h
typecheck_WL.o: /usr/include/string.h ../global/tree.h ../global/types.h
typecheck_WL.o: ../global/tree_basic.h ../global/tree_compound.h
typecheck_WL.o: ../global/Error.h /usr/include/stdio.h
typecheck_WL.o: /usr/include/sys/va_list.h /usr/include/stdio_tag.h
typecheck_WL.o: /usr/include/stdio_impl.h ../global/globals.h
typecheck_WL.o: ../global/internal_lib.h ../print/print.h
typecheck_WL.o: ../global/my_debug.h ../../inc/dbug.h ../global/traverse.h
typecheck_WL.o: ../global/free.h /usr/include/malloc.h
typecheck_WL.o: /usr/include/sys/types.h /usr/include/sys/machtypes.h
typecheck_WL.o: /usr/include/sys/int_types.h ../optimize/ConstantFolding.h
typecheck_WL.o: constants.h shape.h
user_types.o: /usr/include/string.h /usr/include/sys/feature_tests.h
user_types.o: /usr/include/sys/isa_defs.h ../global/Error.h
user_types.o: /usr/include/stdio.h /usr/include/sys/va_list.h
user_types.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
user_types.o: /usr/include/stdlib.h ../global/types.h ../global/globals.h
user_types.o: ../../inc/dbug.h ../global/free.h ../global/my_debug.h
user_types.o: /usr/include/malloc.h /usr/include/sys/types.h
user_types.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
user_types.o: ../global/internal_lib.h user_types.h new_types.h shape.h
