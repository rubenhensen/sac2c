#
# $Log$
# Revision 2.20  2000/07/11 15:56:22  dkr
# make deps done
#
# Revision 2.19  2000/07/05 14:23:42  dkr
# make deps done
#
# Revision 2.18  2000/06/23 09:51:19  nmw
# make deps
#
# Revision 2.17  2000/05/29 14:36:37  dkr
# make deps done
#
# Revision 2.16  2000/05/29 14:35:12  dkr
# make deps done
#
# Revision 2.15  2000/03/31 09:33:43  jhs
# make deps done
#
# Revision 2.14  2000/03/16 14:16:28  dkr
# target typecheck.h added
#
# Revision 2.13  2000/03/01 14:52:34  dkr
# make deps done
#
# Revision 2.12  2000/01/21 17:02:05  dkr
# make deps done
#
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

#
# the typecheck.h object is to fool makedepend which does not
# recognize the dependency between typecheck.h and the .mac files!
#
typecheck.h: cmp_type.mac
	touch typecheck.h

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
constants.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
constants.o: ../tree/free.h ../../inc/dbug.h ../global/my_debug.h
constants.o: /usr/include/malloc.h ../global/internal_lib.h shape.h
constants.o: cv2scalar.h cv2cv.h ../tree/tree_compound.h ../tree/tree_basic.h
constants.o: ../global/Error.h /usr/include/string.h /usr/include/stdlib.h
constants.o: constants.h
cv2cv.o: ../../inc/dbug.h /usr/include/stdio.h
cv2cv.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h cv2cv.h
cv2cv.o: ../tree/type_info.mac
cv2scalar.o: ../tree/tree_basic.h ../global/types.h ../../inc/dbug.h
cv2scalar.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
cv2scalar.o: /usr/include/sys/va_list.h cv2scalar.h ../tree/type_info.mac
gen_pseudo_fun.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
gen_pseudo_fun.o: ../../inc/dbug.h /usr/include/stdio.h
gen_pseudo_fun.o: /usr/include/sys/va_list.h ../global/types.h
gen_pseudo_fun.o: ../tree/tree_basic.h ../tree/tree.h ../tree/tree_compound.h
gen_pseudo_fun.o: ../global/Error.h /usr/include/string.h ../global/globals.h
gen_pseudo_fun.o: ../global/internal_lib.h ../tree/free.h
gen_pseudo_fun.o: ../global/my_debug.h /usr/include/malloc.h
gen_pseudo_fun.o: ../tree/DupTree.h ../tree/LookUpTable.h gen_pseudo_fun.h
gen_pseudo_fun.o: typecheck.h
new_typecheck.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
new_typecheck.o: /usr/include/sys/va_list.h /usr/include/string.h
new_typecheck.o: ../../inc/dbug.h ../tree/tree.h ../global/types.h
new_typecheck.o: ../tree/tree_basic.h ../tree/tree_compound.h
new_typecheck.o: ../global/Error.h /usr/include/stdlib.h ../global/globals.h
new_typecheck.o: ../global/internal_lib.h ../tree/traverse.h user_types.h
new_typecheck.o: new_types.h ../tree/free.h ../global/my_debug.h
new_typecheck.o: /usr/include/malloc.h shape.h new_typecheck.h
new_types.o: ../../inc/dbug.h /usr/include/stdio.h
new_types.o: /usr/include/sys/feature_tests.h /usr/include/sys/va_list.h
new_types.o: ../tree/free.h ../global/my_debug.h /usr/include/malloc.h
new_types.o: ../global/globals.h ../global/types.h ../global/internal_lib.h
new_types.o: ../tree/tree.h ../tree/tree_basic.h ../tree/tree_compound.h
new_types.o: ../global/Error.h /usr/include/string.h /usr/include/stdlib.h
new_types.o: ../print/convert.h shape.h type_constructor_info.mac new_types.h
new_types.o: user_types.h
prim_fun.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
prim_fun.o: ../global/globals.h ../global/types.h /usr/include/stdio.h
prim_fun.o: /usr/include/sys/va_list.h ../tree/tree.h ../tree/tree_basic.h
prim_fun.o: ../tree/tree_compound.h ../global/Error.h /usr/include/string.h
prim_fun.o: ../global/internal_lib.h prim_fun.h ../../inc/dbug.h
prim_fun.o: ../global/my_debug.h ../tree/DupTree.h ../tree/LookUpTable.h
prim_fun.o: ../tree/access_macros.h ../print/convert.h ../tree/free.h
prim_fun.o: /usr/include/malloc.h typecheck.h prim_fun_tt.mac prim_fun_ft.mac
shape.o: /usr/include/stdarg.h /usr/include/varargs.h
shape.o: /usr/include/sys/va_list.h ../../inc/dbug.h /usr/include/stdio.h
shape.o: /usr/include/sys/feature_tests.h ../tree/tree_basic.h
shape.o: ../global/types.h ../tree/free.h ../global/my_debug.h
shape.o: /usr/include/malloc.h ../global/globals.h ../global/internal_lib.h
shape.o: shape.h
typecheck.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
typecheck.o: /usr/include/stddef.h /usr/include/string.h
typecheck.o: /usr/include/limits.h /usr/include/sys/isa_defs.h
typecheck.o: /usr/include/float.h ../tree/tree.h ../global/types.h
typecheck.o: ../tree/tree_basic.h ../tree/tree_compound.h ../global/Error.h
typecheck.o: /usr/include/stdio.h /usr/include/sys/va_list.h
typecheck.o: ../global/globals.h ../global/internal_lib.h ../print/print.h
typecheck.o: ../global/my_debug.h ../../inc/dbug.h ../tree/traverse.h
typecheck.o: typecheck.h ../print/convert.h ../modules/import.h prim_fun.h
typecheck.o: ../tree/free.h /usr/include/malloc.h ../tree/access_macros.h
typecheck.o: ../tree/DupTree.h ../tree/LookUpTable.h typecheck_WL.h
typecheck.o: gen_pseudo_fun.h new_typecheck.h prim_fun_tt.mac
typecheck_WL.o: /usr/include/stdlib.h /usr/include/sys/feature_tests.h
typecheck_WL.o: /usr/include/stddef.h /usr/include/string.h ../tree/tree.h
typecheck_WL.o: ../global/types.h ../tree/tree_basic.h
typecheck_WL.o: ../tree/tree_compound.h ../global/Error.h
typecheck_WL.o: /usr/include/stdio.h /usr/include/sys/va_list.h
typecheck_WL.o: ../global/globals.h ../global/internal_lib.h ../print/print.h
typecheck_WL.o: ../global/my_debug.h ../../inc/dbug.h ../tree/traverse.h
typecheck_WL.o: ../tree/free.h /usr/include/malloc.h
typecheck_WL.o: ../optimize/ConstantFolding.h constants.h shape.h typecheck.h
user_types.o: /usr/include/string.h /usr/include/sys/feature_tests.h
user_types.o: ../global/Error.h /usr/include/stdio.h
user_types.o: /usr/include/sys/va_list.h /usr/include/stdlib.h
user_types.o: ../global/types.h ../global/globals.h ../../inc/dbug.h
user_types.o: ../tree/free.h ../global/my_debug.h /usr/include/malloc.h
user_types.o: ../global/internal_lib.h user_types.h new_types.h shape.h
