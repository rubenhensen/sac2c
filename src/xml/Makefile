#
#
# $Log$
# Revision 1.8  2005/02/10 12:58:44  jhb
# added ast2check_c.xsl and ast2check_h.xsl
#
# Revision 1.7  2004/12/07 14:12:35  sah
# added generation of static html
#
# Revision 1.6  2004/11/29 10:29:32  sah
# added target check
#
# Revision 1.5  2004/11/24 17:47:01  sah
# added some files
#
# Revision 1.4  2004/11/24 16:46:06  sah
# changed node representation
#
# Revision 1.3  2004/11/23 22:22:29  sah
# added traverse*
#
# Revision 1.2  2004/11/23 12:27:00  sah
# added node_info.mac
#
# Revision 1.1  2004/11/23 11:38:05  sah
# Initial revision
#
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

TARGETS = ../tree/attribs.h ../tree/node_basic.h ../tree/node_basic.c \
          ../tree/free_node.c ../tree/free_node.h ../tree/free_attribs.h \
          ../serialize/serialize_node.h ../serialize/serialize_node.c \
          ../serialize/serialize_attribs.h ../serialize/serialize_helper.c \
          ../serialize/serialize_buildstack.c \
          ../serialize/serialize_buildstack.h \
          ../serialize/serialize_link.h ../serialize/serialize_link.c \
          ../types/types_trav.h ../types/types_nodetype.h \
          ../global/node_info.mac ../tree/traverse_tables.c \
          ../tree/traverse_tables.h ../tree/sons.h \
          ../tree/traverse_helper.c \
		  ../tree/check.h ../tree/check.c

all: $(TARGETS) html

check:
	$(XSLTENGINE) check-integrity.xsl ast.xml

html: ast.html

ast.html: ast.xml ast2html.xsl
	$(XSLTENGINE) ast2html.xsl ast.xml > ast.html

#
# these rules generate the [ch] files using the xml/xslt decriptions
#

../tree/attribs.h: ast.xml ast2attribs_h.xsl
	$(XSLTENGINE) ast2attribs_h.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../tree/sons.h: ast.xml ast2sons_h.xsl
	$(XSLTENGINE) ast2sons_h.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../tree/node_basic.h: ast.xml ast2node_basic_h.xsl common-c-code.xsl \
                 common-make-head.xsl common-accessor-macros.xsl
	$(XSLTENGINE) ast2node_basic_h.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@
                                                                                
../tree/node_basic.c: ast.xml ast2node_basic_c.xsl common-c-code.xsl \
                 common-make-head.xsl common-make-body.xsl \
                 common-make-assertion.xsl
	$(XSLTENGINE) ast2node_basic_c.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../tree/free_node.c: ast.xml ast2free_node_c.xsl common-c-code.xsl \
                 common-travfun.xsl
	$(XSLTENGINE) ast2free_node_c.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@


../tree/free_node.h: ast.xml ast2free_node_h.xsl common-c-code.xsl \
                 common-travfun.xsl
	$(XSLTENGINE) ast2free_node_h.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../tree/free_attribs.h: ast.xml ast2free_attribs_h.xsl
	$(XSLTENGINE) ast2free_attribs_h.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../serialize/serialize_node.h: ast.xml ast2serialize_node_h.xsl
	$(XSLTENGINE) ast2serialize_node_h.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../serialize/serialize_node.c: ast.xml ast2serialize_node_c.xsl
	$(XSLTENGINE) ast2serialize_node_c.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../serialize/serialize_link.h: ast.xml ast2serialize_link_h.xsl
	$(XSLTENGINE) ast2serialize_link_h.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../serialize/serialize_link.c: ast.xml ast2serialize_link_c.xsl
	$(XSLTENGINE) ast2serialize_link_c.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../serialize/serialize_attribs.h: ast.xml ast2serialize_attribs_h.xsl
	$(XSLTENGINE) ast2serialize_attribs_h.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../serialize/serialize_helper.c: ast.xml ast2serialize_helper_c.xsl
	$(XSLTENGINE) ast2serialize_helper_c.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../serialize/serialize_buildstack.c: ast.xml ast2serialize_buildstack_c.xsl
	$(XSLTENGINE) ast2serialize_buildstack_c.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../serialize/serialize_buildstack.h: ast.xml ast2serialize_buildstack_h.xsl
	$(XSLTENGINE) ast2serialize_buildstack_h.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../types/types_trav.h: ast.xml ast2types_trav_h.xsl
	$(XSLTENGINE) ast2types_trav_h.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../types/types_nodetype.h: ast.xml ast2types_nodetype_h.xsl
	$(XSLTENGINE) ast2types_nodetype_h.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../global/node_info.mac: ast.xml ast2node_info_mac.xsl
	$(XSLTENGINE) ast2node_info_mac.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../tree/traverse_tables.c: ast.xml ast2traverse_tables_c.xsl
	$(XSLTENGINE) ast2traverse_tables_c.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../tree/traverse_tables.h: ast.xml ast2traverse_tables_h.xsl
	$(XSLTENGINE) ast2traverse_tables_h.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../tree/traverse_helper.c: ast.xml ast2traverse_helper_c.xsl
	$(XSLTENGINE) ast2traverse_helper_c.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../tree/check.h: ast.xml ast2check_h.xsl
	$(XSLTENGINE) ast2check_h.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../tree/check.c: ast.xml ast2check_c.xsl
	$(XSLTENGINE) ast2check_c.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@


clean:
	$(RM) $(TARGETS) ast.html


