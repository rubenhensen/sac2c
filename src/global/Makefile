#
# $Log$
# Revision 3.11  2001/11/14 12:59:48  sbs
# deps line replaced by inclusion of Makefile.Deps
# ( .xxx.d mechanism)
# global setup included via Makefile.Config
#
# Revision 3.10  2001/11/13 20:32:41  dkr
# make deps done
#
# Revision 3.9  2001/07/18 13:08:15  cg
# make deps done
#
# Revision 3.8  2001/04/26 21:09:41  dkr
# make deps done
#
# Revision 3.7  2001/03/22 20:19:57  dkr
# make deps done
#
# Revision 3.6  2001/03/09 11:53:38  sbs
# make deps done
#
# Revision 3.5  2001/03/09 11:15:11  sbs
# -I ../profile/ added
#
# Revision 3.4  2001/03/02 10:36:02  dkr
# /home/dkr/sac/BASE/sac2c/src/global/RCS/Makefile,v used in touch-rules
#
# Revision 3.3  2001/01/17 10:13:00  dkr
# make deps done
#
# Revision 3.2  2000/12/06 20:08:59  dkr
# make deps done
#
# Revision 3.1  2000/11/20 17:59:26  sacbase
# new release made
#
#
# [...]
#
# Revision 1.2  1994/11/10  15:44:34  sbs
# RCS-header inserted
#
#

#######################################################################################
#
# include general setup:
#

include ../../Makefile.Config


#######################################################################################
#
# local settings:
#

INCS=-I../../inc -I../tree -I../print -I../flatten -I../scanparse -I../typecheck \
     -I../optimize -I../modules -I../global -I../refcount -I../compile \
     -I../psi-opt -I../objects -I../concurrent -I../multithread -I../profile

all: main.o Error.o usage.o my_debug.o internal_lib.o globals.o resource.o types.h \
     build.o interrupt.o options.o NameTuples.o

#
# the *.[ch] objects are to fool makedepend which does not
# recognize the dependency between *.[ch] and the .mac files!
#
types.h: ../tree/type_info.mac ../global/phase_info.mac ../global/status_info.mac \
         ../tree/node_info.mac ../tree/prf_node_info.mac
	touch $@

NameTuples.h: ../global/nt_info.mac
	touch $@

NameTuples.c: ../global/nt_info.mac
	touch $@

globals.c: ../tree/type_info.mac ../global/phase_info.mac
	touch $@

my_debug.c:  ../tree/node_info.mac ../tree/prf_node_info.mac ../tree/type_info.mac \
             ../global/status_info.mac
	touch $@


build.c: 
	echo "char build_date[] = \"`date`\";"  >  build.c
	echo "char build_user[] = \"$(USER)\";" >> build.c
	echo "char build_host[] = \"$(HOST)\";" >> build.c
	echo "char build_os[]   = \"$(OS)\";"   >> build.c
	cat build.c

build.o: build.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o build.o -c build.c $(INCS)
	$(RM) build.c

%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -o $@ -c $< $(INCS)

clean:
	$(RM) *.o *.bak *~ .*.d
	$(RM) -r .sb SunWS_cache


#######################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include ../../Makefile.Deps



