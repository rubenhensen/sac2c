#
#
# $Log$
# Revision 3.18  2002/06/06 19:56:03  dkr
# target clean modified
#
# Revision 3.17  2002/03/12 15:12:59  sbs
# ssi.o added
#
# Revision 3.16  2002/03/05 15:52:56  sbs
# create_wrappers.o added
#
# Revision 3.15  2002/02/21 12:30:21  sbs
# flatten added as include path
#
# Revision 3.14  2001/11/15 15:36:48  sacbase
# CLOCK_SKEW_ELIMINATION and PROJECT_ROOT added.
#
# Revision 3.13  2001/11/14 12:59:48  sbs
# deps line replaced by inclusion of Makefile.Deps
# ( .xxx.d mechanism)
# global setup included via Makefile.Config
#
# Revision 3.12  2001/11/13 20:32:42  dkr
# make deps done
#
# Revision 3.11  2001/07/18 13:08:17  cg
# make deps done
#
# Revision 3.10  2001/04/26 21:09:43  dkr
# make deps done
#
# ... [ eliminated ]
#
# Revision 1.1  1994/12/02  10:13:42  hw
# Initial revision
# 
#


#######################################################################################
#
# include general setup:
#

PROJECT_ROOT := ../..
include $(PROJECT_ROOT)/Makefile.Config


#######################################################################################
#
# local settings:
#


INCS=-I../global -I../tree -I../../inc -I../print -I../modules -I../optimize \
     -I../compile -I../c-interface -I../constants -I../flatten

all: typecheck.o prim_fun.o typecheck_WL.o gen_pseudo_fun.o \
     new_typecheck.o new_types.o user_types.o create_wrappers.o \
     ssi.o


#
# the *.[ch] objects are to fool makedepend which does not
# recognize the dependency between *.[ch] and the .mac files!
#
typecheck.h: cmp_type.mac
	touch $@

typecheck.c: prim_fun_tt.mac cmp_type.mac
	touch $@
	$(CLOCK_SKEW_ELIMINATION) $@

prim_fun.c: prim_fun_ft.mac
	touch $@
	$(CLOCK_SKEW_ELIMINATION) $@

new_types.c: type_constructor_info.mac
	touch $@
	$(CLOCK_SKEW_ELIMINATION) $@


%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o $@ -c $< $(INCS)
	$(CLOCK_SKEW_ELIMINATION) $@


ifeq ($(OS),OSF_ALPHA)

prim_fun.o: prim_fun.c
	$(CC) $(CCFLAGS) $(CFLAGS) -O0 -o $@ -c $< $(INCS)
	$(CLOCK_SKEW_ELIMINATION) $@

endif


clean:
	$(RM) *.o *.a *.bak *~ .*.d
	$(RM) -r .sb SunWS_cache


#######################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include ../../Makefile.Deps
