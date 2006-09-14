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


INCS := 
LIBS := 


.PHONY: all tar deps clean


all:  sac_std_gen.h sac_mt_gen.h libsac.a libsac_mt.a libsac_heapmgr.a  \
      libsac_heapmgr_diag.a libsac_heapmgr_mt.a libsac_heapmgr_mt_diag.a  \
      CacheSimAnalyser config.h libsac2c.so libsac2c.a


tar: runtime.tar


libsac_heap%.a : 
	@ if [ ! -f $@ ]; \
	    then ln -s ../heapmgr/$@;  \
                 echo ln -s ../heapmgr/$@;  \
	  fi
	$(CLOCK_SKEW_ELIMINATION) $@

lib%: 
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


# sac2crc: sac2crc.in
# 	(cd $(PROJECT_ROOT); ./configure)
# 	$(CLOCK_SKEW_ELIMINATION) $@


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
	$(RM) CacheSimAnalyser
	$(RM) icm_test.log

%.h: %.h.m4 $(PROJECT_ROOT)/src/m4/icm.m4
	@$(ECHO) "   Generating header file from M4 macro..."
	$(HIDE) $(M4PROC) -I $(PROJECT_ROOT)/src/m4 $< > $@	


#######################################################################################
#
# include automatic dependency mechanism
#
# - relies on INCS to be set appropriately
#

# include ../../Makefile.Deps
