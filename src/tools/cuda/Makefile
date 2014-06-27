
all: sacprapolyhedral sacpolylibintersect 

clean:
	rm -f sacpolylibintersect sacprapolyhedral

sacprapolyhedral: sacprapolyhedral.c
	echo $(POLYLIBINCLUDE) is polylibinclude. LIBS is $(LIBS)
	gcc -o sacprapolyhedral $(POLYLIBINCLUDE) sacprapolyhedral.c -DLINEAR_VALUE_IS_LONGLONG $(LIBS)

deviceQuery:
	nvcc -o deviceQuery deviceQuery.cpp

sacpolylibintersect: sacpolylibintersect.c
	gcc -o sacpolylibintersect $(POLYLIBINCLUDE) sacpolylibintersect.c -DLINEAR_VALUE_IS_LONGLONG $(LIBS)
