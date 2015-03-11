# This makefile is NOT controlled by dependencies (and I don't know to make it do so...)!!

POLYINC = /usr/local/include/polylib

all: polyhedral sacpolylibintersect sacpolylibisnullintersect

clean:
	rm -f sacpolylibintersect polyhedral sacpolylibisnullintersect

polyhedral: polyhedral.c
	gcc -o polyhedral -I $(POLYINC) polyhedral.c -DLINEAR_VALUE_IS_LONGLONG -lpolylib64

deviceQuery:
	nvcc -o deviceQuery deviceQuery.cpp

sacpolylibintersect: sacpolylibintersect.c
	gcc -o sacpolylibintersect -I $(POLYINC) sacpolylibintersect.c -DLINEAR_VALUE_IS_LONGLONG -lpolylib64

sacpolylibisnullintersect: sacpolylibisnullintersect.c
	gcc -o ../../../bin/sacpolylibisnullintersect -I $(POLYINC) sacpolylibisnullintersect.c -DLINEAR_VALUE_IS_LONGLONG -lpolylib64 -I  ../../include -I  ../../libsac2c/arrayopt  -ggdb -g 
