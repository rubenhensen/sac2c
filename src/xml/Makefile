#
# $Id$
#



PROJECT_ROOT := ../..

include $(PROJECT_ROOT)/Makefile.Config

.PHONY: all check clean


all: ast.html

check:
	$(XSLTENGINE) check_integrity.xsl ast.xml

ast.html: ast2html.xsl ast.xml 
	$(XSLTENGINE) ast2html.xsl ast.xml > ast.html

clean:
	$(RM) ast.html


