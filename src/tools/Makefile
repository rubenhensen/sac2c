#
# $Log$
# Revision 1.1  1999/07/08 12:02:34  cg
# Initial revision
#
#


CC        := gcc
CCFLAGS   := -ansi -Wall -D__EXTENSIONS__ -g -O3

LIBDIR    := ../runtime

INCS      := -I../runtime -I../libsac -I../../inc
LIBS      := -L$(LIBDIR)


.PHONY: all tar


all:  CacheSimAnalyser CacheSimTest


CacheSimAnalyser: CacheSimAnalyser.o $(LIBDIR)/libsac.a
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) $(LIBS) -o $@ $< -lsac -lm

CacheSimTest: CacheSimTest.o $(LIBDIR)/libsac.a
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) $(LIBS) -o $@ $< -lsac -lm

%.o: %.c 
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<


deps:
	makedepend $(INCS) *.c


clean:
	$(RM) *.o *.a *.bak *.old *~
	$(RM) -r .sb
	$(RM) CacheSimTest CacheSimAnalyser

