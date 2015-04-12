# This makefile is NOT controlled by dependencies (and I don't know to make it do so...)!!

POLYINC = /usr/local/include/polylib
ISLINC = /usr/local/include/isl
ISLPRIVATEINC = /home/rbe/barvinok/barvinok-0.36
ISLLIB = /usr/local/lib

all: polyhedral sacislinterface 

clean:
	rm -f polyhedral sacislinterface 

polyhedral: polyhedral.c
	gcc -o polyhedral -I $(POLYINC) polyhedral.c -DLINEAR_VALUE_IS_LONGLONG -lpolylib64

deviceQuery:
	nvcc -o deviceQuery deviceQuery.cpp

#sacpolylibinterface: sacpolylibinterface.c
#	gcc -o ../../../bin/sacpolylibinterface -I $(POLYINC) sacpolylibinterface.c -DLINEAR_VALUE_IS_LONGLONG -lpolylib64 -I../../include -I../../libsac2c/arrayopt -ggdb -g 

sacislinterface: sacislinterface.c
	gcc -I /usr/include -o ../../../bin/sacislinterface -I $(ISLINC) -I $(ISLPRIVATEINC) sacislinterface.c -I ../../include -I ../../libsac2c/arrayopt -ggdb -g  -std=c99 -lisl

