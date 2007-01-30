/*
 * $Id$
 */

/*
 * CAUTION:
 *
 * This construction of including actual C code seems rather ugly,
 * however it serves its purpose well: generating two different object
 * files from a single C source without setting a burden on the makefile
 * mechanism.
 */

#define DETAILED 0
#define TYPE S

#include "access.c"
