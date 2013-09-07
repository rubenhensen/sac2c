POLYINC = /usr/local/include/polylib

polyhedral:
	gcc -o polyhedral -I $(POLYINC) polyhedral.c -DLINEAR_VALUE_IS_LONGLONG -lpolylib64

deviceQuery:
	nvcc -o deviceQuery deviceQuery.cpp
