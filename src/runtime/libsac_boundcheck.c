/*
 *
 * $Log$
 * Revision 1.1  1999/04/09 09:50:19  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   sac_boundcheck.c
 *
 * description:
 *
 *   This file is part of the SAC runtime library
 *
 *   Currently, it only provides the format strings for generating runtime
 *   error message in the case of memory violations.
 *
 *****************************************************************************/

char SAC_BC_format_write[] = "Memory access violation on writing into array %s\n"
                             "*** with size %d at index position %d !\n";

char SAC_BC_format_read[] = "Memory access violation on reading from array %s\n"
                            "*** with size %d at index position %d !\n";
