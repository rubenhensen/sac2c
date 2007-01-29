/*
 * $Log$
 * Revision 1.3  2005/02/16 22:29:13  sah
 * changed link handling
 *
 * Revision 1.2  2004/11/26 21:18:50  sah
 * pour Bodo *<8-)
 *
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
    sttable_t *table;
    node *ast;
    node *current;
    bool argavisdirect;
};

/*
 * INFO macros
 */
#define INFO_SER_FILE(n) ((n)->file)
#define INFO_SER_STACK(n) ((n)->stack)
#define INFO_SER_TABLE(n) ((n)->table)
#define INFO_SER_AST(n) ((n)->ast)
#define INFO_SER_CURRENT(n) ((n)->current)
#define INFO_SER_ARGAVISDIRECT(n) ((n)->argavisdirect)

#endif /* _SAC_SERIALIZE_INFO_H_ */
