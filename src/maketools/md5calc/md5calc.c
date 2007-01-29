/*******************************************************************************
 * This file defines a very simple MD5 sum calculator.
 *
 * It reads bytes from stdin until it recieves an EOF and outputs the MD5
 * sum of the data read. It is meant to compute a digest over the ast.xml
 * file and should not be used for anything else!
 */

#include <stdio.h>
#include "md5.h"

#define BUFSIZE 1024

int
main ()
{
    md5_state_t state;
    md5_byte_t buffer[BUFSIZE];
    md5_byte_t digest[16];
    int cnt;
    int numread;

    md5_init (&state);

    while (!feof (stdin)) {
        numread = fread (buffer, 1, BUFSIZE, stdin);

        md5_append (&state, buffer, numread);
    }

    md5_finish (&state, digest);

    for (cnt = 0; cnt < 16; cnt++) {
        printf ("%02x", digest[cnt]);
    }

    return (0);
}
