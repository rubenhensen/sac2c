#
#
# $Log$
# Revision 1.3  2003/04/01 10:53:58  sbs
# Now, Makefile.Config is included and (if nec.) made here!
#
# Revision 1.2  2001/12/06 14:39:14  dkr
# target 'clean' corrected
#
# Revision 1.1  2001/11/20 10:39:53  sacbase
# Initial revision
#
#

PROJECT_ROOT := ../
include $(PROJECT_ROOT)/Makefile.Config

SUBDIRS = $(wildcard src/*)

all: 
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $${d}; \
	done

$(PROJECT_ROOT)/Makefile.Config: $(PROJECT_ROOT)/Makefile.Config.in
	(cd $(PROJECT_ROOT); ./configure)

clean:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $${d} clean; \
	done
