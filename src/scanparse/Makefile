#
#
# $Log$
# Revision 3.10  2001/11/15 15:12:46  sacbase
# PROJECT_ROOT added
#
# Revision 3.9  2001/11/14 12:59:48  sbs
# deps line replaced by inclusion of Makefile.Deps
# ( .xxx.d mechanism)
# global setup included via Makefile.Config
#
# Revision 3.8  2001/11/13 20:32:42  dkr
# make deps done
#
# Revision 3.7  2001/07/18 13:08:16  cg
# make deps done
#
# Revision 3.6  2001/04/26 21:09:43  dkr
# make deps done
#
# Revision 3.5  2001/03/22 20:19:58  dkr
# make deps done
#
# Revision 3.4  2001/01/17 10:13:02  dkr
# make deps done
#
# Revision 3.3  2000/12/15 15:08:42  sbs
# YYDEBUG disabled for non-solaris machines
# => avoids warning on alpha.
#
# Revision 3.2  2000/12/06 20:09:01  dkr
# make deps done
#
# Revision 3.1  2000/11/20 17:59:47  sacbase
# new release made
#
#  ... [eliminated] ...
#
# Revision 1.2  1994/11/10  15:22:47  sbs
# RCS-header inserted
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

LIBS      := -ly -ll

INCS = -I../global -I../tree -I../../inc -I../modules -I../typecheck


all: y.tab.o lex.yy.o scnprs.o


y.tab.o: y.tab.c
	$(CC) $(CCL_FLAGS) $(CCFLAGS) $(CFLAGS) $(YYFLAGS) -o y.tab.o -c y.tab.c $(INCS)

lex.yy.o: lex.yy.c y.tab.h
	$(CC) $(CCL_FLAGS) $(CCFLAGS) $(CFLAGS) -DYYLMAX=8192 -o lex.yy.o -c lex.yy.c $(INCS)

lex.yy.c: sac.l
	$(LEX) sac.l

y.tab.c y.tab.h: sac.y 
	$(YACC)  sac.y

%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o $@ -c $< $(INCS)


clean:
	$(RM) y.tab.h y.tab.c lex.yy.c *.o *.bak y.output .*.d
	$(RM) -r .sb SunWS_cache



#######################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include ../../Makefile.Deps



