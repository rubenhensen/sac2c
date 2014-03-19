POLYINC = /usr/local/include/polylib

all: polyhedral sacpolylibintersect 

clean:
	rm -f sacpolylibintersect polyhedral

polyhedral: polyhedral.c
	gcc -o polyhedral -I $(POLYINC) polyhedral.c -DLINEAR_VALUE_IS_LONGLONG -lpolylib64

deviceQuery:
	nvcc -o deviceQuery deviceQuery.cpp

sacpolylibintersect: sacpolylibintersect.c
	gcc -o sacpolylibintersect -I $(POLYINC) sacpolylibintersect.c -DLINEAR_VALUE_IS_LONGLONG -lpolylib64
