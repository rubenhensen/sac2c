/*****************************************************************************
 *
 * file:   misc.c
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

/*****************************************************************************
 *
 * function:
 *   void SAC_String2Array( unsigned char *array, const char *string)
 *
 * description:
 *   This function converts string representations of character arrays
 *   into real character arrays. It's used in conjunction with the ICM
 *   ND_CREATE__STRING__DATA.
 *
 *****************************************************************************/

void
SAC_String2Array (unsigned char *array, const char *string)
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

    array[i] = '\0';
}
