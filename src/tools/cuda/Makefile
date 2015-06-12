# This makefile is NOT controlled by dependencies (and I don't know to make it do so...)!!

POLYINC = /usr/local/include/polylib
ISLINC = /usr/local/include/isl

all: polyhedral sacislinterface 

clean:
	rm -f polyhedral sacislinterface 

polyhedral: polyhedral.c
	gcc -o polyhedral -I $(POLYINC) polyhedral.c -DLINEAR_VALUE_IS_LONGLONG -lpolylib64

deviceQuery:
	nvcc -o deviceQuery deviceQuery.cpp

sacislinterface: sacislinterface.c
	gcc -I /usr/include -o sacislinterface -I $(ISLINC) sacislinterface.c -I ../../include -I ../../libsac2c/arrayopt -ggdb -g  -std=c99 -lisl
	cp sacislinterface ../../../bin/sacislinterface
