#
# $Log$
# Revision 1.3  1999/07/09 11:39:08  cg
# make deps done
#
# Revision 1.2  1999/07/08 15:39:55  cg
# Dependency from libsac now uses libsac/libsac.a instead of
# runtime/libsac.a
#
# Revision 1.1  1999/07/08 12:02:34  cg
# Initial revision
#
#


CC        := gcc
CCFLAGS   := -ansi -Wall -D__EXTENSIONS__ -g -O3

LIBDIR    := ../libsac
  # Here, we cannot use the links in ../runtime because after a gmake clean
  # they are not yet existing.

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

# DO NOT DELETE THIS LINE -- make depend depends on it.

CacheSimAnalyser.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
CacheSimAnalyser.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
CacheSimAnalyser.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
CacheSimAnalyser.o: ../runtime/sac_cachesim.h ../runtime/sac_message.h
CacheSimAnalyser.o: ../libsac/cachesim.h
CacheSimTest.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
CacheSimTest.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
CacheSimTest.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
CacheSimTest.o: ../runtime/sac_cachesim.h
