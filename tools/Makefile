#
#
# $Log$
# Revision 1.2  2001/12/06 14:39:14  dkr
# target 'clean' corrected
#
# Revision 1.1  2001/11/20 10:39:53  sacbase
# Initial revision
#
#

SUBDIRS = $(wildcard src/*)

all: 
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $${d}; \
	done

clean:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $${d} clean; \
	done
