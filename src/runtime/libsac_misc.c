/*
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:43:39  sacbase
 * new release made
 *
 * Revision 1.3  1998/06/29 08:50:14  cg
 * added '#define _POSIX_C_SOURCE 199506L' for multi-threaded execution.
 *
 * Revision 1.2  1998/05/07 08:13:24  cg
 * SAC runtime library implementation converted to new naming conventions.
 *
 * Revision 1.1  1998/03/19 16:35:59  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   libsac_misc.c
 *
 * prefix: SAC_
 *
 * description:
 *
 *   This file is part of the implementation of the SAC runtime library.
 *
 *   It provides definitions of miscellaneous library functions.
 *
 *****************************************************************************/

#define _POSIX_C_SOURCE 199506L

/******************************************************************************
 *
 * function:
 *   void SAC_String2Array(char *array, const char *string)
 *
 * description:
 *   This function converts string representations of character arrays
 *   into real character arrays. It's used in conjunction with the ICM
 *   ND_CREATE_CONST_ARRAY.
 *
 ******************************************************************************/

void
SAC_String2Array (char *array, const char *string)
{
    int i = 0, j = 0;

    while (string[j] != '\0') {
        if (string[j] == '\\') {
            switch (string[j + 1]) {
            case 'n':
                array[i++] = '\n';
                j += 2;
                break;
            case 't':
                array[i++] = '\t';
                j += 2;
                break;
            case 'v':
                array[i++] = '\v';
                j += 2;
                break;
            case 'b':
                array[i++] = '\b';
                j += 2;
                break;
            case 'r':
                array[i++] = '\r';
                j += 2;
                break;
            case 'f':
                array[i++] = '\f';
                j += 2;
                break;
            case 'a':
                array[i++] = '\a';
                j += 2;
                break;
            case '"':
                array[i++] = '"';
                j += 2;
                break;
            default:
                array[i++] = '\\';
                j += 1;
            }
        } else {
            array[i++] = string[j++];
        }
    }
}
