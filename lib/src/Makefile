#
# $Log$
# Revision 2.4  1999/05/12 14:36:50  cg
# added compilation of main_args.c
#
# Revision 2.3  1999/03/11 18:18:26  dkr
# make deps done
#
# Revision 2.2  1999/02/26 14:17:44  dkr
# make deps done
#
# Revision 2.1  1999/02/23 12:43:30  sacbase
# new release made
#
# Revision 1.5  1998/02/05 17:08:45  dkr
# make deps done
#
# Revision 1.4  1997/12/04 15:28:30  srs
# removed generation of debug information for dbug.c
#
# Revision 1.3  1997/12/02 18:47:53  dkr
# now uses his own CFLAGS!!
#
# Revision 1.2  1997/11/28 13:11:07  dkr
# changed CCFLAGS
#
# Revision 1.1  1997/11/25 13:44:09  dkr
# Initial revision
#
#
#

CC        := gcc
#MYFLAGS   := -g
MYFLAGS   := 


INCS=

all: ../dbug.o ../main_args.o 

../dbug.o: dbug.c
	$(CC) $(MYFLAGS) -o ../dbug.o -c dbug.c $(INCS)

../main_args.o: main_args.c
	$(CC) $(MYFLAGS) -o ../main_args.o -c main_args.c $(INCS)

deps:
	makedepend $(INCS) *.c

clean:
	$(RM) ../dbug.o ../main_args.o *.bak *~

# DO NOT DELETE THIS LINE -- make depend depends on it.

dbug.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
dbug.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
dbug.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
main_args.o: /usr/include/stdio.h /usr/include/sys/feature_tests.h
main_args.o: /usr/include/sys/isa_defs.h /usr/include/sys/va_list.h
main_args.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
