#
#
# $Log$
# Revision 1.1  2001/11/20 10:39:53  sacbase
# Initial revision
#
#
#

SUBDIRS = $(wildcard src/*)

all: 
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $${d}; \
	done
