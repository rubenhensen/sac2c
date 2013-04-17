/*
 * CAUTION:
 *
 * This construction of including actual C code seems rather ugly,
 * however it serves its purpose well: generating two different object
 * files from a single C source without setting a burden on the makefile
 * mechanism.
 */

#define SAC_DO_MT_PTHREAD 1
#define TRACE 1

#include "mt_pth.c"
