/*
 * $Log$
 * Revision 1.4  2004/10/25 11:58:47  sah
 * major code cleanup
 *
 * Revision 1.3  2004/09/23 21:12:25  sah
 * ongoing implementation
 *
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
#include "symboltable.h"

/*
 * INFO structure
 */

struct INFO {
    FILE *file;
    serstack_t *stack;
    STtable_t *table;
};

/*
 * INFO macros
 */
#define INFO_SER_FILE(n) n->file
#define INFO_SER_STACK(n) n->stack
#define INFO_SER_TABLE(n) n->table

#endif /* _SERIALIZE_INFO_H */
