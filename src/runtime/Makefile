#
#
# $Log$
# Revision 3.16  2003/06/17 19:02:50  dkr
# target icm_test extended
#
# Revision 3.15  2003/06/17 18:49:20  dkr
# target test_icm modified
#
# Revision 3.14  2003/06/17 18:18:40  dkr
# target 'icm_test' added
#
# Revision 3.13  2003/03/25 12:06:41  sbs
# config.h added
#
# Revision 3.12  2003/03/21 16:30:25  sbs
# rule for sac2crc changed
#
# Revision 3.11  2003/03/20 23:18:22  sah
# updated link sac2crc link creation
# to confirm with new configure
#
# Revision 3.10  2002/06/06 19:56:03  dkr
# target clean modified
#
# Revision 3.9  2002/06/06 19:35:17  dkr
# target clean: *.old removed
#
# Revision 3.8  2001/11/15 15:36:48  sacbase
# CLOCK_SKEW_ELIMINATION and PROJECT_ROOT added.
#
# Revision 3.7  2001/11/14 12:59:48  sbs
# deps line replaced by inclusion of Makefile.Deps
# ( .xxx.d mechanism)
# global setup included via Makefile.Config
#
# Revision 3.6  2001/03/12 12:25:49  dkr
# make deps done
#
# Revision 3.5  2001/03/09 11:53:43  sbs
# make deps done
#
# Revision 3.4  2001/03/02 16:13:59  dkr
# make deps done
#
# Revision 3.3  2001/03/02 14:46:01  sbs
# make deps done
#
# Revision 3.2  2000/12/06 20:09:07  dkr
# make deps done
#
# Revision 3.1  2000/11/20 18:02:06  sacbase
# new release made
#
# ... [eliminated] ...
#
# Revision 1.1  1998/03/19 16:32:50  cg
# Initial revision
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


INCS := 
LIBS := 


.PHONY: all tar deps clean


all:  libsac.a libsac_mt.a libsac_heapmgr.a  \
      libsac_heapmgr_diag.a libsac_heapmgr_mt.a libsac_heapmgr_mt_diag.a  \
      CacheSimAnalyser sac2crc config.h


tar: runtime.tar


libsac_heap%.a : 
	@ if [ ! -f $@ ]; \
	    then ln -s ../heapmgr/$@;  \
                 echo ln -s ../heapmgr/$@;  \
	  fi
	$(CLOCK_SKEW_ELIMINATION) $@

lib%.a : 
	@ if [ ! -f $@ ]; \
	    then ln -s ../libsac/$@;  \
                 echo ln -s ../libsac/$@;  \
	  fi
	$(CLOCK_SKEW_ELIMINATION) $@

config.h:
	@ if [ ! -f $@ ]; \
	    then ln -s ../global/$@;  \
                 echo ln -s ../global/$@;  \
	  fi
	$(CLOCK_SKEW_ELIMINATION) $@
	


sac2crc: sac2crc.in
	(cd $(PROJECT_ROOT); ./configure)
	$(CLOCK_SKEW_ELIMINATION) $@


CacheSimAnalyser: 
	@ if [ ! -f $@ ]; \
	    then ln -s ../tools/$@; \
                 echo ln -s ../tools/$@; \
	  fi
	$(CLOCK_SKEW_ELIMINATION) $@


runtime.tar: *.h sac2crc.README
	tar cvf $@ $^
	$(CLOCK_SKEW_ELIMINATION) $@


#
# call the CC preprocessor on 'icm_test.C' to test the expansion of H-ICMs
#
icm_test: icm_test.C
	$(CC) -D_SHP_=SCL -D_HID_=NHD -D_UNQ_=NUQ -I$$SACBASE/runtime -E $^ >  icm_test.log
	$(CC) -D_SHP_=SCL -D_HID_=NHD -D_UNQ_=UNQ -I$$SACBASE/runtime -E $^ >> icm_test.log
	$(CC) -D_SHP_=SCL -D_HID_=HID -D_UNQ_=NUQ -I$$SACBASE/runtime -E $^ >> icm_test.log
	$(CC) -D_SHP_=SCL -D_HID_=HID -D_UNQ_=UNQ -I$$SACBASE/runtime -E $^ >> icm_test.log
	$(CC) -D_SHP_=AKS -D_HID_=NHD -D_UNQ_=NUQ -I$$SACBASE/runtime -E $^ >> icm_test.log
	$(CC) -D_SHP_=AKS -D_HID_=NHD -D_UNQ_=UNQ -I$$SACBASE/runtime -E $^ >> icm_test.log
	$(CC) -D_SHP_=AKS -D_HID_=HID -D_UNQ_=NUQ -I$$SACBASE/runtime -E $^ >> icm_test.log
	$(CC) -D_SHP_=AKS -D_HID_=HID -D_UNQ_=UNQ -I$$SACBASE/runtime -E $^ >> icm_test.log
	$(CC) -D_SHP_=AKD -D_HID_=NHD -D_UNQ_=NUQ -I$$SACBASE/runtime -E $^ >> icm_test.log
	$(CC) -D_SHP_=AKD -D_HID_=NHD -D_UNQ_=UNQ -I$$SACBASE/runtime -E $^ >> icm_test.log
	$(CC) -D_SHP_=AKD -D_HID_=HID -D_UNQ_=NUQ -I$$SACBASE/runtime -E $^ >> icm_test.log
	$(CC) -D_SHP_=AKD -D_HID_=HID -D_UNQ_=UNQ -I$$SACBASE/runtime -E $^ >> icm_test.log
	$(CC) -D_SHP_=AUD -D_HID_=NHD -D_UNQ_=NUQ -I$$SACBASE/runtime -E $^ >> icm_test.log
	$(CC) -D_SHP_=AUD -D_HID_=NHD -D_UNQ_=UNQ -I$$SACBASE/runtime -E $^ >> icm_test.log
	$(CC) -D_SHP_=AUD -D_HID_=HID -D_UNQ_=NUQ -I$$SACBASE/runtime -E $^ >> icm_test.log
	$(CC) -D_SHP_=AUD -D_HID_=HID -D_UNQ_=UNQ -I$$SACBASE/runtime -E $^ >> icm_test.log
	-grep CAT icm_test.log
	-grep NT icm_test.log
	@echo '!!! CPP output can be found in file >>icm_test.log<< !!!'

clean:
	$(RM) *.o *.a *.bak *~ .*.d
	$(RM) -r .sb  SunWS_cache
	$(RM) runtime.tar
	$(RM) sac2crc CacheSimAnalyser
	$(RM) icm_test.log


#######################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

include ../../Makefile.Deps
