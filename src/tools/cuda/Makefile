POLYINC = /usr/local/include/polylib

all: polyhedral sacpolylibintersect

polyhedral:
	gcc -o polyhedral -I $(POLYINC) polyhedral.c -DLINEAR_VALUE_IS_LONGLONG -lpolylib64

deviceQuery:
	nvcc -o deviceQuery deviceQuery.cpp

sacpolylibintersect:
	gcc -o sacpolylibintersect -I $(POLYINC) polylibintersect.c -DLINEAR_VALUE_IS_LONGLONG -lpolylib64
