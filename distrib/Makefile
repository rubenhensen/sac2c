
BASICS  = sac2c runtime.tar stdlib.tar
OTHERS  = install_sac
SYSTEM  = solaris
DATE    = $(shell date +%d_%b_%y)

distrib: src/sac2c src/runtime.tar src/stdlib.tar
	mkdir -p ../binary
	( cd src; tar cvf ../binary/sac-$(SYSTEM)-$(DATE).tar $(OTHERS) $(BASICS))
	( cd binary; gzip sac-$(SYSTEM)-$(DATE).tar)
	
update: 
	( cd ..; update .)
	( cd $(SACBASE)/stdlib; update .)

src/sac2c:
	( cd ..; $(MAKE) product)
	cp ../sac2c src/sac2c

src/runtime.tar:
	( cd ../src/runtime; $(MAKE) runtime.tar)
	cp ../src/runtime/runtime.tar src/runtime.tar

src/stdlib.tar:
	( cd $(SACBASE)/stdlib; $(MAKE) tar)
	cp $(SACBASE)/stdlib/stdlib.tar src/stdlib.tar
