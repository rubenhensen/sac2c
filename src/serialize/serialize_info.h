/*
 * $Log$
 * Revision 1.1  2004/11/23 22:41:02  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _SAC_SERIALIZE_INFO_H_
#define _SAC_SERIALIZE_INFO_H_

#include "types.h"

/*
 * INFO structure
 */

struct INFO {
    FILE *file;
    serstack_t *stack;
    STtable_t *table;
    node *ast;
};

/*
 * INFO macros
 */
#define INFO_SER_FILE(n) (n->file)
#define INFO_SER_STACK(n) (n->stack)
#define INFO_SER_TABLE(n) (n->table)
#define INFO_SER_AST(n) (n->ast)

#endif /* _SAC_SERIALIZE_INFO_H_ */
