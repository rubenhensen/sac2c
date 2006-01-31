#
# $Id$
#


###############################################################################
#
# include general setup:
#

PROJECT_ROOT := ../..

include $(PROJECT_ROOT)/Makefile.Source



###############################################################################
#
# subdirectory specific aspects
#

lex.yy.c: sac.l
	@echo "Lexing $<"
	@$(LEX) $<
	@$(CLOCK_SKEW_ELIMINATION) $@

y.tab.c y.tab.h: sac.y
	@echo "Yaccing $<"
	@$(YACC) $<
	@$(CLOCK_SKEW_ELIMINATION) y.tab.c y.tab.h

