#
#
# $Log$
# Revision 1.10  2004/10/25 09:45:03  sah
# added serialize_link.[ch}]
#
# Revision 1.9  2004/10/11 15:48:43  sah
# added BST traversal xsl files
#
# Revision 1.8  2004/09/20 19:57:30  sah
# added serialize_helper.c
#
# Revision 1.7  2004/09/20 16:16:15  sah
# added serialize_attribs.h generation
#
# Revision 1.6  2004/09/01 14:13:07  sah
# ongoing work
#
# Revision 1.5  2004/08/07 17:59:36  sah
# added ast2check_node_h.xsl and
# ast2check_node_c.xsl
#
# Revision 1.4  2004/08/07 16:23:07  sah
# added some files
#
# Revision 1.3  2004/07/31 16:16:57  sah
# added support for flags and moved to memory saving attribute
# structure.
#
# Revision 1.2  2004/07/15 00:08:34  sah
# modularized the xslt scripts
#
# Revision 1.1  2004/07/03 15:13:54  sah
# Initial revision
#
#
#


#######################################################################################
#
# include general setup:
#

PROJECT_ROOT := ../../..
include $(PROJECT_ROOT)/Makefile.Config


#######################################################################################
#
# local settings:
#

TARGETS = ../attribs.h ../node_basic.h ../node_basic.c ../free_node.c \
          ../free_node.h ../free_attribs.h \
          ../check_node.h ../check_node.c \
          ../serialize_node.h ../serialize_node.c \
          ../serialize_attribs.h ../serialize_helper.c \
          ../serialize_buildstack.c ../serialize_buildstack.h \
          ../serialize_link.h ../serialize_link.c

all: $(TARGETS)

#
# these rules generate the [ch] files using the xml/xslt decriptions
#

../attribs.h: ast.xml ast2attribs_h.xsl
	$(XSLTENGINE) ast2attribs_h.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../node_basic.h: ast.xml ast2node_basic_h.xsl common-c-code.xsl \
                 common-make-head.xsl common-flag-defines.xsl \
                 common-accessor-macros.xsl
	$(XSLTENGINE) ast2node_basic_h.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@
                                                                                
../node_basic.c: ast.xml ast2node_basic_c.xsl common-c-code.xsl \
                 common-make-head.xsl common-make-body.xsl \
                 common-make-assertion.xsl
	$(XSLTENGINE) ast2node_basic_c.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../free_node.c: ast.xml ast2free_node_c.xsl common-c-code.xsl \
                 common-travfun.xsl
	$(XSLTENGINE) ast2free_node_c.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@


../free_node.h: ast.xml ast2free_node_h.xsl common-c-code.xsl \
                 common-travfun.xsl
	$(XSLTENGINE) ast2free_node_h.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../free_attribs.h: ast.xml ast2free_attribs_h.xsl
	$(XSLTENGINE) ast2free_attribs_h.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../check_node.h: ast.xml ast2check_node_h.xsl
	$(XSLTENGINE) ast2check_node_h.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../check_node.c: ast.xml ast2check_node_c.xsl
	$(XSLTENGINE) ast2check_node_c.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../serialize_node.h: ast.xml ast2serialize_node_h.xsl
	$(XSLTENGINE) ast2serialize_node_h.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../serialize_node.c: ast.xml ast2serialize_node_c.xsl
	$(XSLTENGINE) ast2serialize_node_c.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../serialize_link.h: ast.xml ast2serialize_link_h.xsl
	$(XSLTENGINE) ast2serialize_link_h.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../serialize_link.c: ast.xml ast2serialize_link_c.xsl
	$(XSLTENGINE) ast2serialize_link_c.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../serialize_attribs.h: ast.xml ast2serialize_attribs_h.xsl
	$(XSLTENGINE) ast2serialize_attribs_h.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../serialize_helper.c: ast.xml ast2serialize_helper_c.xsl
	$(XSLTENGINE) ast2serialize_helper_c.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../serialize_buildstack.c: ast.xml ast2serialize_buildstack_c.xsl
	$(XSLTENGINE) ast2serialize_buildstack_c.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../serialize_buildstack.h: ast.xml ast2serialize_buildstack_h.xsl
	$(XSLTENGINE) ast2serialize_buildstack_h.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

clean:
	$(RM) $(TARGETS)


