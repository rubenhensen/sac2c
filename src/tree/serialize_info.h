/*
 * $Log$
 * Revision 1.1  2004/09/20 19:52:38  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _SERIALIZE_INFO_H
#define _SERIALIZE_INFO_H

#include <stdio.h>

/*
 * INFO structure
 */

struct INFO {
    FILE *file;
};

/*
 * INFO macros
 */
#define INFO_SER_FILE(n) n->file

/*
 * INFO stack
 */
#define PUSH(x)
#define POP(x)
#define FINDPOS(x) 0

#endif /* _SERIALIZE_INFO_H */
