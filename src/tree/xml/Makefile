#
#
# $Log$
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

TARGETS = ../attribs.h ../node_basic.h ../node_basic.c

all: $(TARGETS)

#
# these rules generate the [ch] files using the xml/xslt decriptions
#

../attribs.h: ast.xml ast2attribs_h.xsl
	$(XSLTENGINE) ast2attribs_h.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@

../node_basic.h: ast.xml ast2node_basic_h.xsl
	$(XSLTENGINE) ast2node_basic_h.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@
                                                                                
../node_basic.c: ast.xml ast2node_basic_c.xsl
	$(XSLTENGINE) ast2node_basic_c.xsl ast.xml \
	| $(CODE_BEAUTIFIER) >$@


clean:
	$(RM) $(TARGETS)


