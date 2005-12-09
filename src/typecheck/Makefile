#
# $Id$
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


OBJS:=gen_pseudo_fun.o new_typecheck.o new_types.o user_types.o \
      create_wrappers.o ssi.o sig_deps.o ct_basic.o ct_fun.o ct_prf.o \
      ct_with.o type_errors.o specialize.o new2old.o create_wrapper_code.o \
      type_statistics.o type_utils.o resolvesymboltypes.o \
      dispatchfuncalls.o split_wrappers.o insert_type_conv.o \
      update_wrapper_type.o free_dispatch_information.o \
      pre_typecheck.o

all: $(OBJS)

#
# the *.[ch] objects are to fool makedepend which does not
# recognize the dependency between *.[ch] and the .mac files!
#
typecheck.h: cmp_type.mac
	touch $@
	$(CLOCK_SKEW_ELIMINATION) $@

typecheck.c: prim_fun_tt.mac cmp_type.mac
	touch $@
	$(CLOCK_SKEW_ELIMINATION) $@

prim_fun.c: prim_fun_ft.mac prim_fun_tt.mac
	touch $@
	$(CLOCK_SKEW_ELIMINATION) $@

new_types.c: type_constructor_info.mac
	touch $@
	$(CLOCK_SKEW_ELIMINATION) $@

ct_prf.c: ../tree/prf_node_info.mac
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
