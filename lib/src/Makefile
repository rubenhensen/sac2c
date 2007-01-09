#
# $Id$
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

INCS= -I../../inc -I../../src/global

all: ../dbug.o ../getoptions.o ../md5.o

../dbug.o: dbug.c
	$(CC) $(CFLAGS) $(CCFLAGS) -o ../dbug.o -c dbug.c $(INCS)

../getoptions.o: getoptions.c
	$(CC) $(CFLAGS) $(CCFLAGS) -o ../getoptions.o -c getoptions.c $(INCS)

../md5.o: md5.c
	 $(CC) $(CFLAGS) $(CCFLAGS) -o ../md5.o -c md5.c $(INCS)

clean:
	$(RM) ../dbug.o ../getoptions.o *.bak *~ .*.d





