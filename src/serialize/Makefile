#
#
# $Log$
# Revision 1.5  2005/07/22 13:10:52  sah
# extracted some functionality from
# deserialize into add_function_body
#
# Revision 1.4  2005/07/17 11:46:54  sah
# added serialize_filenames.o
#
# Revision 1.3  2005/07/16 18:33:36  sah
# cleanup
#
# Revision 1.2  2004/11/26 21:20:30  sah
# *** empty log message ***
#
# Revision 1.1  2004/11/23 22:39:46  sah
# Initial revision
#
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

OBJS := serialize_node.o serialize_attribs.o \
        deserialize.o serialize_buildstack.o \
        serialize_helper.o serialize_link.o \
        serialize.o serialize_stack.o \
        serialize_symboltable.o \
        serialize_filenames.o add_function_body.o

all: $(OBJS)

%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<
	$(CLOCK_SKEW_ELIMINATION) $@

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
