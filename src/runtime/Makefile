#
# $Log$
# Revision 1.2  1998/03/24 13:51:04  cg
# additional include directory specified
#
# Revision 1.1  1998/03/19 16:32:50  cg
# Initial revision
#
#
# 


CC        := gcc
CCFLAGS   := -ansi -Wall -D__EXTENSIONS__

INCS := -I../global -I../../inc
OBJS := $(patsubst %.c,%.o,$(wildcard libsac_*.c))

.PHONY: all


all:  libsac.a

libsac.a: $(OBJS)
	ar rc libsac.a $(OBJS)
	ranlib libsac.a

%.o: %.c 
	$(CC) $(CCFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<


deps:
	makedepend $(INCS) *.c

clean:
	$(RM) *.o libsac.a *.bak *.old *~
	$(RM) -r .sb

