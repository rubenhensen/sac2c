#
#
# $Log$
# Revision 3.22  2003/10/20 14:35:53  dkr
# target icm_test modified
#
# Revision 3.21  2003/09/29 23:38:54  dkr
# target icm_test modified: more GREPs added
#
# Revision 3.20  2003/09/22 16:03:06  dkr
# target icm_tester modified
#
# Revision 3.19  2003/09/22 15:22:22  dkr
# new target icm_tester added
#
# Revision 3.18  2003/09/22 11:17:23  dkr
# target icm_test modified
#
# Revision 3.17  2003/09/19 15:05:13  dkr
# target icm_test extended: some formating done with sed, code beautifier used
#
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
	$(CC) -D_SHP_=SCL -D_HID_=NHD -D_UNQ_=NUQ -I$$SACBASE/runtime -E $^ | sed -f icm_test.sed | $(CODE_BEAUTIFIER) -r >  icm_test.log
	$(CC) -D_SHP_=SCL -D_HID_=NHD -D_UNQ_=UNQ -I$$SACBASE/runtime -E $^ | sed -f icm_test.sed | $(CODE_BEAUTIFIER) -r >> icm_test.log
	$(CC) -D_SHP_=SCL -D_HID_=HID -D_UNQ_=NUQ -I$$SACBASE/runtime -E $^ | sed -f icm_test.sed | $(CODE_BEAUTIFIER) -r >> icm_test.log
	$(CC) -D_SHP_=SCL -D_HID_=HID -D_UNQ_=UNQ -I$$SACBASE/runtime -E $^ | sed -f icm_test.sed | $(CODE_BEAUTIFIER) -r >> icm_test.log
	$(CC) -D_SHP_=AKS -D_HID_=NHD -D_UNQ_=NUQ -I$$SACBASE/runtime -E $^ | sed -f icm_test.sed | $(CODE_BEAUTIFIER) -r >> icm_test.log
	$(CC) -D_SHP_=AKS -D_HID_=NHD -D_UNQ_=UNQ -I$$SACBASE/runtime -E $^ | sed -f icm_test.sed | $(CODE_BEAUTIFIER) -r >> icm_test.log
	$(CC) -D_SHP_=AKS -D_HID_=HID -D_UNQ_=NUQ -I$$SACBASE/runtime -E $^ | sed -f icm_test.sed | $(CODE_BEAUTIFIER) -r >> icm_test.log
	$(CC) -D_SHP_=AKS -D_HID_=HID -D_UNQ_=UNQ -I$$SACBASE/runtime -E $^ | sed -f icm_test.sed | $(CODE_BEAUTIFIER) -r >> icm_test.log
	$(CC) -D_SHP_=AKD -D_HID_=NHD -D_UNQ_=NUQ -I$$SACBASE/runtime -E $^ | sed -f icm_test.sed | $(CODE_BEAUTIFIER) -r >> icm_test.log
	$(CC) -D_SHP_=AKD -D_HID_=NHD -D_UNQ_=UNQ -I$$SACBASE/runtime -E $^ | sed -f icm_test.sed | $(CODE_BEAUTIFIER) -r >> icm_test.log
	$(CC) -D_SHP_=AKD -D_HID_=HID -D_UNQ_=NUQ -I$$SACBASE/runtime -E $^ | sed -f icm_test.sed | $(CODE_BEAUTIFIER) -r >> icm_test.log
	$(CC) -D_SHP_=AKD -D_HID_=HID -D_UNQ_=UNQ -I$$SACBASE/runtime -E $^ | sed -f icm_test.sed | $(CODE_BEAUTIFIER) -r >> icm_test.log
	$(CC) -D_SHP_=AUD -D_HID_=NHD -D_UNQ_=NUQ -I$$SACBASE/runtime -E $^ | sed -f icm_test.sed | $(CODE_BEAUTIFIER) -r >> icm_test.log
	$(CC) -D_SHP_=AUD -D_HID_=NHD -D_UNQ_=UNQ -I$$SACBASE/runtime -E $^ | sed -f icm_test.sed | $(CODE_BEAUTIFIER) -r >> icm_test.log
	$(CC) -D_SHP_=AUD -D_HID_=HID -D_UNQ_=NUQ -I$$SACBASE/runtime -E $^ | sed -f icm_test.sed | $(CODE_BEAUTIFIER) -r >> icm_test.log
	$(CC) -D_SHP_=AUD -D_HID_=HID -D_UNQ_=UNQ -I$$SACBASE/runtime -E $^ | sed -f icm_test.sed | $(CODE_BEAUTIFIER) -r >> icm_test.log
	-grep CAT[0-9] icm_test.log
	-grep "[^A-Z]NT_" icm_test.log
	-grep Item icm_test.log
	-grep __SCL icm_test.log
	-grep __AKS icm_test.log
	-grep __AKD icm_test.log
	-grep __AUD icm_test.log
	-grep __HID icm_test.log
	-grep __NHD icm_test.log
	-grep __UNQ icm_test.log
	-grep __NUQ icm_test.log
	@echo '!!! CPP output can be found in file >>icm_test.log<< !!!'

icm_tester:
	@ if [ ! -f $@ ];                                 \
	    then ln -s $(PROJECT_ROOT)/tools/bin/$@;      \
	         echo ln -s $(PROJECT_ROOT)/tools/bin/$@; \
	  fi
	$(CLOCK_SKEW_ELIMINATION) $@

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
