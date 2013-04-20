/***********************************************************************
 *                                                                     *
 *                      Copyright (c) 1994-2007                        *
 *         SAC Research Foundation (http://www.sac-home.org/)          *
 *                                                                     *
 *                        All Rights Reserved                          *
 *                                                                     *
 *   The copyright holder makes no warranty of any kind with respect   *
 *   to this product and explicitly disclaims any implied warranties   *
 *   of merchantability or fitness for any particular purpose.         *
 *                                                                     *
 ***********************************************************************/

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
