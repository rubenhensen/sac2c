/*
 *
 * $Log$
 * Revision 3.3  2004/06/08 14:39:11  skt
 * MUTHGetLastExpression added
 *
 * Revision 3.2  2001/05/08 12:27:57  dkr
 * new macros for RC used
 *
 * Revision 3.1  2000/11/20 18:03:12  sacbase
 * new release made
 *
 * Revision 1.6  2000/07/13 08:24:24  jhs
 * Moved DupMask_ InsertBlock, InsertMT and InsertST from blocks_init.[ch]
 * Renamed InsertXX to MUTHInsertXX.
 *
 * Revision 1.5  2000/04/10 15:45:08  jhs
 * Added Reduce
 *
 * Revision 1.4  2000/03/28 09:48:44  jhs
 * Added refcunt-macros
 *
 * Revision 1.3  2000/03/09 18:34:47  jhs
 * Additional features.
 *
 * Revision 1.2  2000/03/02 13:01:02  jhs
 * Added MUTHExchangeApplication,
 * added MUTHExpandFundefName,
 * added DBUG_PRINTS and DBUG_ASSERTS.
 *
 * Revision 1.1  2000/02/21 17:48:22  jhs
 * Initial revision
 *
 */

#ifndef _SAC_MULTITHREAD_LIB_H_
#define _SAC_MULTITHREAD_LIB_H_

#include "refcount.h"

/*
 *  returns 0 for refcounting-objects and -1 otherwise
 */
#define GET_ZERO_REFCNT(prefix, node)                                                    \
    (RC_IS_ACTIVE (prefix##_REFCNT (node)) ? 0 : RC_INACTIVE)

/*
 *  returns 1 for refcounting-objects and -1 otherwise
 */
#define GET_STD_REFCNT(prefix, node)                                                     \
    (RC_IS_ACTIVE (prefix##_REFCNT (node)) ? 1 : RC_INACTIVE)

extern node *MUTHBlocksLastInstruction (node *block);
extern node *MUTHMeltBlocks (node *first_block, node *second_block);
extern node *MUTHMeltBlocksOnCopies (node *first_block, node *second_block);
extern node *MUTHExchangeApplication (node *arg_node, node *new_fundef);
extern node *MUTHExpandFundefName (node *fundef, char *prefix);
extern node *MUTHReduceFundefName (node *fundef, int count);
extern node *MUTHInsertST (node *assign, node *arg_info);
extern node *MUTHInsertMT (node *assign, node *arg_info);
extern node *MUTHGetLastExpression (node *expression);

#endif /* _SAC_CONCURRENT_LIB_H_ */
