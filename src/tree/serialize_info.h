/*
 * $Log$
 * Revision 1.2  2004/09/21 16:34:27  sah
 * ongoing implementation of
 * serialize traversal
 *
 * Revision 1.1  2004/09/20 19:52:38  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _SERIALIZE_INFO_H
#define _SERIALIZE_INFO_H

#include <stdio.h>
#include "serialize_stack.h"

/*
 * INFO structure
 */

struct INFO {
    FILE *file;
    serstack_t *stack;
};

/*
 * INFO macros
 */
#define INFO_SER_FILE(n) n->file
#define INFO_SER_STACK(n) n->stack

#endif /* _SERIALIZE_INFO_H */
