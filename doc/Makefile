DOC	  := sac
SOURCES	  := sac.texi macros.texi breaks.texi optimizations.texi

TEXI2PDF  := texi2pdf
TEXI2POD  := perl texi2pod.pl
POD2MAN	  := pod2man --center "SaC" \
		     --release="sac-1.0"\
		     --date=$(shell date +%F)
TEXI2HTML := makeinfo --html --no-split \
		      --no-number-section \
		      --css-ref=sac.css
CPP	  := cpp -P -I${SAC2CBASE}/src/libsac2c/global \
		    -I${SAC2CBASE}/src/libsac2c/stdopt



all:
	cp *.texi *.texi.in *.js texi2pod.pl sac.css  Makefile ./build
	cd ./build && make doc

doc: pdf-docs man-docs html-docs

pdf-docs: $(SOURCES)
	$(TEXI2PDF) $(DOC).texi

man-docs: $(SOURCES)
	$(TEXI2POD) $< > $(DOC).pod
	$(POD2MAN) sac.pod > $(DOC).1

html-docs: $(SOURCES)
	$(TEXI2HTML) $(DOC).texi


%.texi: %.texi.in
	$(CPP) $< \
	| sed -e 's/"\([^"]*\)"/\1/g'\
	| sed -e s'/<br>/\n/g' > $@

clean:
	if [ -d build ]; then  \
		rm -rf build;  \
		mkdir build;   \
	fi;
